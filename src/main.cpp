#include "mcp3424/mcp3424.h"    // for MCP3424 interface
#include "ads1148/ads1148.h"    // for ADS1148 interface
#include "pump/dcmotor.h"       // for pump interface
#include "cosmos/packets.h"     // for packet definitions
#include "cosmos/cosmosQueue.h" // for cosmos tlm and cmd queues
#include <wiringPi.h>           // for PI_THREAD
#include <string>               // for strings
#include <fstream>              // for reading system files
#include <cstdio>               // for printf() and perror()
#include <sstream>              // for stringstream objects
#include <unistd.h>             // for getpid() and usleep()
#include <chrono>               // for timestamps and timing
#include <sys/sysinfo.h>        // for total memory used

//TODO: set the rpm pin
#define RPM_PIN     1

// this queue provides two concurrent threads: a tlm_queue and cmd_queue
// any packet pushed to the tlm_queue will be sent to COSMOS
// any packet received will be found in the cmd_queue
CosmosQueue queue(4810, 20000, 8);

// function prototype for the ISR (defined at the bottom) that increments
// count each time the Raspberry Pi detects a rising edge on the RPM line
void rpmISR();
volatile unsigned long count = 0;

// function prototype for getTimestamp() (defined at the bottom), which
// returns the time in microseconds since unix epoch
uint64_t getTimestamp();

// this thread handles everything to do with the ADS1148
// it collects and queues the TempPacket
PI_THREAD(ads1148_thread) {
    ADS1148* dev = nullptr;
    while (true) {
        // try opening the ADS1148 every ten seconds until it succeeds
        while (true) {
            try {
                //TODO: increase the SPI bus speed
                //TODO: change the DRDY pin
                //TODO: reset device at startup
                dev = new ADS1148(0, 100000, 4);
                printf("Successfully opened the ADS1148\n");
                break;
            } catch (int e) {
                printf("FAILED to open ADS1148. Trying again in 10 seconds...\n");
                sleep(10);
            }
        }

        //TODO: set ADS1148 settings (VREP, IEXT, etc.)
        try {
            dev->setVREFsource(VREF_REF1);
        } catch (int e) {
            printf("failed to write ADS1148 settings\n");
            delete dev;
            continue;
        }
        TempPacket* tPacket = nullptr;
        StampedConversion conv;

        try {
            // collect and send packets repeatedly
            while (true) {
                tPacket = new TempPacket;

                // get channel 1 conversion and switch to channel 2
                dev->getConversion(conv, AIN2, AIN3);
                tPacket->time1 = conv.timestamp;
                tPacket->temp1 = conv.code;

                // get channel 2 conversion and switch to channel 3
                dev->getConversion(conv, AIN4, AIN5);
                tPacket->time2 = conv.timestamp;
                tPacket->temp2 = conv.code;

                // get channel 3 conversion and switch to channel 4
                dev->getConversion(conv, AIN6, AIN7);
                tPacket->time3 = conv.timestamp;
                tPacket->temp3 = conv.code;

                // get channel 4 conversion and switch to channel 1
                dev->getConversion(conv, AIN0, AIN1);
                tPacket->time4 = conv.timestamp;
                tPacket->temp4 = conv.code;

                queue.push_tlm(tPacket);
            }
        // if at any point it fails, delete the device then jump back up to
        // the top and try to open/configure it again
        } catch (int e) {
            printf("Lost connection with ADS1148\n");
            delete dev;
            continue;
        }
    }
}

// this thread handles everything to do with the MCP3424
// it collects and queues the VenturiPacket, PumpPacket, and StaticPackets
PI_THREAD(mcp3424_thread) {
    MCP3424* dev = nullptr;
    while (true) {
        // try opening the MCP3424 every ten seconds until it succeeds
        while (true) {
            try {
                dev = new MCP3424(0x68, CHANNEL1 | ONESHOT | RES_16_BITS | PGAx2);
                printf("Successfully opened the MCP3424\n");
                break;
            } catch (int e) {
                printf("FAILED to open MCP3424. Trying again in 10 seconds...\n");
                sleep(10);
            }
        }

        PressurePacket* pPacket = nullptr;

        try {
            // get and queue the PressurePacket repeatedly
            while (true) {
                pPacket = new PressurePacket;
                //TODO: this assumes that venturi is channel 1, pump is 2, and static is 3
                dev->setConfig(CHANNEL1 | ONESHOT | RES_16_BITS | PGAx2);
                dev->startConversion();
                while (!dev->isReady()) usleep(1000);
                pPacket->venturiTime = getTimestamp();
                pPacket->venturiPressure = dev->getConversion();

                dev->setConfig(CHANNEL2 | ONESHOT | RES_16_BITS | PGAx2);
                dev->startConversion();
                while (!dev->isReady()) usleep(1000);
                pPacket->pumpTime = getTimestamp();
                pPacket->pumpPressure = dev->getConversion();

                dev->setConfig(CHANNEL3 | ONESHOT | RES_16_BITS | PGAx2);
                dev->startConversion();
                while (!dev->isReady()) usleep(1000);
                pPacket->staticTime = getTimestamp();
                pPacket->staticPressure = dev->getConversion();

                queue.push_tlm(pPacket);
            }
        } catch (int e) {
            printf("Lost connection with MCP3424\n");
            delete dev;
            continue;
        }
    }
}

// this thread watches the RPM interrupts through rpmISR(), then
// constructs the RpmPacket and queues it
PI_THREAD(rpm_thread) {
    RpmPacket* rPacket = nullptr;
    // start the ISR (Interrupt Sub-Routine)
    wiringPiSetup();
    wiringPiISR(RPM_PIN, INT_EDGE_RISING, rpmISR);
    unsigned long start = 0;
    unsigned long end = count;
    while (true) {
        rPacket = new RpmPacket;
        start = end;
        //TODO make the delay more accurate
        sleep(1);
        end = count;
        rPacket->value = 15 * (start - end);
        rPacket->timestamp = getTimestamp();
        queue.push_tlm(rPacket);
    }
}

// this thread puts together a housekeeping packet that it
// queues every second
PI_THREAD(housekeeping) {
    HKPacket* hkPacket = nullptr;
    std::ifstream tempfile("/sys/class/thermal/thermal_zone0/temp");
    std::ifstream loadfile("/proc/loadavg");
    std::string tmp;
    struct sysinfo memInfo;

    // get and send housekeeping packet repeatedly
    while (true) {
        hkPacket = new HKPacket;
        hkPacket->timestamp = getTimestamp();
        hkPacket->queue_size = queue.tlmSize();

        // look at the system file that has the cpu temperature
        tempfile.seekg(std::ios_base::beg);
        getline(tempfile, tmp);
        hkPacket->cpu_temp = stof(tmp) / 1000;

        // look at the system file that has the cpu load average
        loadfile.seekg(std::ios_base::beg);
        getline(loadfile, tmp);
        hkPacket->cpu_load = stof(tmp);

        // get currently used virtual memory
        sysinfo(&memInfo);
        hkPacket->mem_usage = memInfo.totalram - memInfo.freeram;
        hkPacket->mem_usage += memInfo.totalswap - memInfo.freeswap;
        hkPacket->mem_usage *= memInfo.mem_unit;

        queue.push_tlm(hkPacket);
        sleep(1);
    }
    tempfile.close();
    loadfile.close();
}

int main() {
    /*// launch the ads1148_thread
    if (piThreadCreate(ads1148_thread) != 0) {
        perror("ADS1148 control thread didn't start");
    }*/
    // launch the mcp3424_thread
    if (piThreadCreate(mcp3424_thread) != 0) {
        perror("MCP3424 control thread didn't start");
    }
    /*// launch the rpm_thread
    if (piThreadCreate(rpm_thread) != 0) {
        perror("RPM thread didn't start");
    }*/
    // launch the housekeeping thread
    if (piThreadCreate(housekeeping) != 0) {
        perror("Housekeeping thread didn't start");
    }
    sleep(500000);

    // This code changes the thread priority
    /*pid_t pid = getpid();
    std::stringstream cmd;
    cmd << "renice -n -2 -p " << pid;
    if (pid > 0) system(cmd.str().c_str());*/

    // TODO: gracefully handle disconnects? Or not connected?
    float speed;
    Packet* cmdPacket = nullptr;
    DCMotor* pump = nullptr;
    // try to connect to the motor hat every 10 seconds until it succeeds
    while (true) {
        try {
            // TODO: change PWM frequency?
            pump = new DCMotor(3, 0x60, 1500);
            printf("Successfully opened the motor hat\n");
            break;
        } catch (int e) {
            printf("FAILED to open the motor hat. Trying again in 10 seconds...\n");
            sleep(10);
        }
    }
    // loop over and over, waiting for a pump command
    while (true) {
        // if there's a pump command:
        if (queue.cmdSize() > 0 && queue.cmd_front_id() == 0x30) {
            queue.pop_cmd(cmdPacket);
            PumpCmd* pCmd = static_cast<PumpCmd*>(cmdPacket);
            pCmd->PumpCmd::convert();
            // convert the voltage into a PWM code
            speed = pCmd->voltage * 4095.0 / 12.0;
            printf("Received pump command: setting pump voltage to %f\n", pCmd->voltage);
            // set the speed
            pump->setGradSpeed((int) speed);
            delete cmdPacket;
            cmdPacket = nullptr;
        }
        usleep(500000);
    }
    return 0;
}

// increment the count every time the RPI detects a rising edge
void rpmISR() {
    count++;
}

// return the system time in microseconds since unix epoch
uint64_t getTimestamp() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
