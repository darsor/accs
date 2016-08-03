#include "mcp3424/mcp3424.h"    // for MCP3424 interface
#include "ads1148/ads1148.h"    // for ADS1148 interface
#include "cosmos/packets.h"     // for packet definitions
#include "cosmos/cosmosQueue.h" // for cosmos tlm and cmd queues
#include <wiringPi.h>           // for PI_THREAD
#include <cstdio>               // for printf() and perror()
#include <sstream>              // for stringstream objects
#include <unistd.h>             // for getpid() and usleep()

// this queue provides two concurrent threads: a tlm_queue and cmd_queue
// any packet pushed to the tlm_queue will be sent to COSMOS
// any packet received will be found in the cmd_queue
CosmosQueue queue(4810, 20000, 8);

// this thread handles everything to do with the ADS1148
// it collects and queues the TempPacket
PI_THREAD(ads1148_thread) {
    // try opening the ADS1148 every ten seconds until it succeeds
    ADS1148* dev = nullptr;
    while (true) {
        try {
            //TODO: increase the SPI bus speed
            dev = new ADS1148(0, 100000, 4);
            break;
        } catch (int e) {
            printf("FAILED to open ADS1148. Trying again every 10 seconds...\n");
            sleep(10);
        }
    }

    //TODO: set ADS1148 settings (VREP, IEXT, etc.)
    TempPacket* tPacket = nullptr;
    StampedConversion conv;

    //TODO: exit gracefully if sensor is disconnected,
    //and try to reconnect
    while (true) {
        // get and send TempPacket
        tPacket = new TempPacket;

        dev->getConversion(conv, AIN2, AIN3);
        tPacket->time1 = conv.timestamp;
        tPacket->temp1 = conv.code;

        dev->getConversion(conv, AIN4, AIN5);
        tPacket->time2 = conv.timestamp;
        tPacket->temp2 = conv.code;

        dev->getConversion(conv, AIN6, AIN7);
        tPacket->time3 = conv.timestamp;
        tPacket->temp3 = conv.code;

        dev->getConversion(conv, AIN0, AIN1);
        tPacket->time4 = conv.timestamp;
        tPacket->temp4 = conv.code;

        queue.push_tlm(tPacket);
    }
    delete dev;
}

// this thread handles everything to do with the MCP3424
// it collects and queues the VenturiPacket, PumpPacket, and StaticPackets
PI_THREAD(mcp3424_thread) {
    // try opening the MCP3424 every ten seconds until it succeeds
    MCP3424* dev = nullptr;
    while (true) {
        try {
            dev = new MCP3424(0x68, CHANNEL1 | ONESHOT | RES_16_BITS | PGAx2);
            break;
        } catch (int e) {
            printf("FAILED to open MCP3424. Trying again every 10 seconds...\n");
            sleep(10);
        }
    }

    PressurePacket* pPacket = nullptr;

    //TODO: exit gracefully if sensor is disconnected,
    //and try to reconnect
    while (true) {
        // get and queue the PressurePacket
        //TODO: this assumes that Venturi is channel 1, pump is 2, and static is 3
        dev->setConfig(CHANNEL1 | ONESHOT | RES_16_BITS | PGAx2);
        dev->startConversion();
        while (!dev->isReady()) usleep(1000);
        //TODO: get timestamp
        //pPacket->venturiTime = ...
        pPacket->venturiPressure = dev->getConversion();

        dev->setConfig(CHANNEL2 | ONESHOT | RES_16_BITS | PGAx2);
        dev->startConversion();
        while (!dev->isReady()) usleep(1000);
        //TODO: get timestamp
        //pPacket->pumpTime = ...
        pPacket->pumpPressure = dev->getConversion();

        dev->setConfig(CHANNEL3 | ONESHOT | RES_16_BITS | PGAx2);
        dev->startConversion();
        while (!dev->isReady()) usleep(1000);
        //TODO: get timestamp
        //pPacket->staticTime = ...
        pPacket->staticPressure = dev->getConversion();

        queue.push_tlm(pPacket);
    }
    delete dev;
}

int main() {
    // launch the ads1148_thread
    if (piThreadCreate(ads1148_thread) != 0) {
        perror("ADS1148 control thread didn't start");
    }
    // launch the mcp3424_thread
    if (piThreadCreate(mcp3424_thread) != 0) {
        perror("MCP3424 control thread didn't start");
    }
    /*pid_t pid = getpid();
    std::stringstream cmd;
    cmd << "renice -n -2 -p " << pid;
    if (pid > 0) system(cmd.str().c_str());*/

    return 0;
}
