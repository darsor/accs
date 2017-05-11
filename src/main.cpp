#include "mcp3424/mcp3424.h"    // for MCP3424 interface
#include "mcp4725/mcp4725.h"    // for MCP4725 interface
#include "ads1148/ads1148.h"    // for ADS1148 interface
#include "pump/dcmotor.h"       // for pump interface
#include "cosmos/packets.h"     // for packet definitions
#include "cosmos/cosmosQueue.h" // for cosmos tlm and cmd queues
#include <wiringPi.h>           // for PI_THREAD
#include <thread>               // for multiple threads
#include <string>               // for strings
#include <fstream>              // for reading system files
#include <cstdio>               // for printf() and perror()
#include <sstream>              // for stringstream objects
#include <unistd.h>             // for getpid() and usleep()
#include <chrono>               // for timestamps and timing
#include <sys/sysinfo.h>        // for total memory used
#include <math.h>               // for round()

// this queue provides two concurrent threads: a tlm_queue and cmd_queue
// any packet pushed to the tlm_queue will be sent to COSMOS
// any packet received will be found in the cmd_queue
CosmosQueue queue(4810, 20000, 8);

// function prototype for getTimestamp() (defined at the bottom), which
// returns the time in microseconds since unix epoch
uint64_t getTimestamp();

// this thread handles everything to do with the ADS1148
// it collects and queues the TempPacket
void ads1148_thread() {
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

        // set ADS1148 settings (VREP, IEXT, etc.)
        try {
            dev->setMux(AIN0, AIN1);
            dev->setVREFstate(VREF_ON);
            dev->setVREFsource(VREF_REF0);
            dev->setPGA(ADS1148_PGAx32);
            dev->setDataRate(DATA_20SPS);
            dev->setDACpins(DAC_AIN0, DAC_AIN1);
            dev->setDACmagnitude(DAC_500uA);
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
                dev->setDACpins(DAC_AIN2, DAC_AIN3);
                tPacket->time1 = conv.timestamp;
                tPacket->temp1 = conv.code;

                // get channel 2 conversion and switch to channel 3
                dev->getConversion(conv, AIN4, AIN5);
                dev->setDACpins(DAC_AIN4, DAC_AIN5);
                tPacket->time2 = conv.timestamp;
                tPacket->temp2 = conv.code;

                // get channel 3 conversion and switch to channel 1
                dev->getConversion(conv, AIN0, AIN1);
                dev->setDACpins(DAC_AIN0, DAC_AIN1);
                tPacket->time3 = conv.timestamp;
                tPacket->temp3 = conv.code;

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
// it collects and queues the pressure and power packets
void mcp3424_thread() {
    MCP3424* dev = nullptr;
    while (true) {
        // try opening the MCP3424 every ten seconds until it succeeds
        while (true) {
            try {
                dev = new MCP3424(0x68, CHANNEL1 | ONESHOT | RES_16_BITS | PGAx2);
                printf("Successfully opened the pressure MCP3424\n");
                break;
            } catch (int e) {
                printf("FAILED to open the pressure MCP3424. Trying again in 10 seconds...\n");
                sleep(10);
            }
        }

        PressurePacket* prsPacket = nullptr;
        PowerPacket* pwrPacket = nullptr;
        Packet* cmdPacket = nullptr;
        float vZero, sZero, pZero;
        vZero = sZero = pZero = 0.0;

        try {
            // get and queue the PressurePacket and PowerPacket repeatedly
            while (true) {
                prsPacket = new PressurePacket;
                dev->setConfig(CHANNEL1 | ONESHOT | RES_16_BITS | PGAx2);
                dev->startConversion();
                while (!dev->isReady()) usleep(1000);
                prsPacket->venturiTime = getTimestamp();
                prsPacket->venturiPressure = dev->getConversion();

                dev->setConfig(CHANNEL2 | ONESHOT | RES_16_BITS | PGAx2);
                dev->startConversion();
                while (!dev->isReady()) usleep(1000);
                prsPacket->staticTime = getTimestamp();
                prsPacket->staticPressure = dev->getConversion();

                dev->setConfig(CHANNEL4 | ONESHOT | RES_16_BITS | PGAx2);
                dev->startConversion();
                while (!dev->isReady()) usleep(1000);
                prsPacket->pumpTime = getTimestamp();
                prsPacket->pumpPressure = dev->getConversion();

                prsPacket->venturiZero = vZero;
                prsPacket->staticZero  = sZero;
                prsPacket->pumpZero    = pZero;
                queue.push_tlm(prsPacket);

                // pump current sensor on channel 3
                dev->setConfig(CHANNEL4 | ONESHOT | RES_16_BITS | PGAx8);
                dev->startConversion();
                pwrPacket = new PowerPacket;
                while (!dev->isReady()) usleep(1000);
                pwrPacket->amperageTime = getTimestamp();
                pwrPacket->amperage = dev->getConversion();
                pwrPacket->voltageTime = 0;
                pwrPacket->voltage = 0;

                queue.push_tlm(pwrPacket);

                if (queue.cmdSize() > 0 && queue.cmd_front_id() == ZERO_PRES_ID) {
                    printf("received zero cmd, new zeroes are:\n");
                    queue.pop_cmd(cmdPacket);
                    ZeroPressure* zCmd = static_cast<ZeroPressure*>(cmdPacket);
                    zCmd->ZeroPressure::convert();
                    vZero = zCmd->venturiZero;
                    sZero = zCmd->staticZero;
                    pZero = zCmd->pumpZero;
                    printf("venturi: %f\npump: %f\nstatic: %f\n", vZero, pZero, sZero);
                }
            }
        } catch (int e) {
            printf("Lost connection with pressure MCP3424\n");
            delete dev;
            continue;
        }
    }
}

// this thread puts together a housekeeping packet that it
// queues every second
void housekeeping() {
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
    // launch each thread
    std::thread(ads1148_thread).detach();
    std::thread(mcp3424_thread).detach();
    std::thread(housekeeping).detach();

    // This code changes the thread priority
    /*pid_t pid = getpid();
    std::stringstream cmd;
    cmd << "renice -n -2 -p " << pid;
    if (pid > 0) system(cmd.str().c_str());*/

    // TODO: gracefully handle disconnects? Or not connected?
    // (max code) / ((max dac output/max controller input)*(voltage supply))
    const float VOLTAGE_TO_DAC_CODE = 4095.0 / ((3.3/5.0)*(28.0));
    int code;
    Packet* cmdPacket = nullptr;
    MCP4725* dac = nullptr;
    // try to connect to the DAC every 10 seconds until it succeeds
    while (true) {
        try {
            dac = new MCP4725(0x60);
            printf("Successfully opened the DAC\n");
            break;
        } catch (int e) {
            printf("FAILED to open the DAC. Trying again in 10 seconds...\n");
            sleep(10);
        }
    }
    // loop over and over, waiting for a pump command
    while (true) {
        // if there's a pump command:
        if (queue.cmdSize() > 0 && queue.cmd_front_id() == PUMP_CMD_ID) {
            queue.pop_cmd(cmdPacket);
            PumpCmd* pCmd = static_cast<PumpCmd*>(cmdPacket);
            pCmd->PumpCmd::convert();
            // convert the voltage into a DAC code
            code = round(pCmd->voltage * VOLTAGE_TO_DAC_CODE);
            printf("Received pump command: setting pump voltage to %f (%d)\n", pCmd->voltage, code);
            // set the DAC
            dac->write(code);
            delete cmdPacket;
            cmdPacket = nullptr;
        }
        usleep(500000);
    }
    return 0;
}

// return the system time in microseconds since unix epoch
uint64_t getTimestamp() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
