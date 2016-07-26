#include "mcp3424.h"
#include <unistd.h>
#include <iostream>
#include <cstdio>

int main() {
    MCP3424 dev(0x68);
    usleep(100000);
    int32_t data;
    char config;

    printf("TEST 1: Reading 12-bit values from channel 1. Press enter to continue... ");
    getchar();
    printf("\n");

    dev.setConfig(CHANNEL1 | CONTINUOUS | RES_12_BITS | PGAx1);
    for (int i=0; i<10; ++i) {
        data = dev.getConversion();
        printf("Data point %d: %03x\n", i+1, data);
        usleep(100000);
    }

    printf("\nTEST 2: Reading 14_bit values from channel 1. Press enter to continue... ");
    getchar();
    printf("\n");

    dev.setConfig(CHANNEL1 | CONTINUOUS | RES_14_BITS | PGAx1);
    usleep(100000);
    for (int i=0; i<10; ++i) {
        data = dev.getConversion();
        printf("Data point %d: %04x\n", i+1, data);
        usleep(100000);
    }

    printf("\nTEST 3: Reading 16_bit values from channel 1. Press enter to continue... ");
    getchar();
    printf("\n");

    dev.setConfig(CHANNEL1 | CONTINUOUS | RES_16_BITS | PGAx1);
    usleep(200000);
    for (int i=0; i<10; ++i) {
        data = dev.getConversion();
        printf("Data point %d: %04x\n", i+1, data);
        usleep(200000);
    }

    printf("\nTEST 4: Reading 18_bit values from channel 1. Press enter to continue... ");
    getchar();
    printf("\n");

    dev.setConfig(CHANNEL1 | CONTINUOUS | RES_18_BITS | PGAx1);
    usleep(400000);
    for (int i=0; i<10; ++i) {
        data = dev.getConversion();
        printf("Data point %d: %05x\n", i+1, data);
        usleep(400000);
    }

    printf("TEST 5: Reading 12-bit values from channel 2. Press enter to continue... ");
    getchar();
    printf("\n");

    dev.setConfig(CHANNEL2 | CONTINUOUS | RES_12_BITS | PGAx1);
    for (int i=0; i<10; ++i) {
        data = dev.getConversion();
        printf("Data point %d: %03x\n", i+1, data);
        usleep(100000);
    }

    printf("\nTEST 6: Reading 14_bit values from channel 2. Press enter to continue... ");
    getchar();
    printf("\n");

    dev.setConfig(CHANNEL2 | CONTINUOUS | RES_14_BITS | PGAx1);
    usleep(100000);
    for (int i=0; i<10; ++i) {
        data = dev.getConversion();
        printf("Data point %d: %04x\n", i+1, data);
        usleep(100000);
    }

    printf("\nTEST 7: Reading 16_bit values from channel 2. Press enter to continue... ");
    getchar();
    printf("\n");

    dev.setConfig(CHANNEL2 | CONTINUOUS | RES_16_BITS | PGAx1);
    usleep(200000);
    for (int i=0; i<10; ++i) {
        data = dev.getConversion();
        printf("Data point %d: %04x\n", i+1, data);
        usleep(200000);
    }

    printf("\nTEST 8: Reading 18_bit values from channel 2. Press enter to continue... ");
    getchar();
    printf("\n");

    dev.setConfig(CHANNEL2 | CONTINUOUS | RES_18_BITS | PGAx1);
    usleep(400000);
    for (int i=0; i<10; ++i) {
        data = dev.getConversion();
        printf("Data point %d: %05x\n", i+1, data);
        usleep(400000);
    }

    printf("\nTEST 9: Testing getConfig(). Press enter to continue... ");
    getchar();

    config = dev.getConfig();
    printf("config is %02x, expecting %02x\n", config & ~RDY_MASK, CHANNEL2 | CONTINUOUS | RES_18_BITS | PGAx1);

    printf("\nTEST 10: Testing getConfig(). Press enter to continue... ");
    getchar();

    dev.setConfig(CHANNEL4 | ONESHOT | RES_16_BITS | PGAx2);
    usleep(200000);
    config = dev.getConfig();
    printf("config is %02x, expecting %02x\n", config & ~RDY_MASK, CHANNEL4 | ONESHOT | RES_16_BITS | PGAx2);

    printf("\nTEST 11: Testing getConfig(). Press enter to continue... ");
    getchar();

    dev.setConfig(CHANNEL1 | ONESHOT | RES_18_BITS | PGAx1);
    usleep(400000);
    config = dev.getConfig();
    printf("config is %02x, expecting %02x\n", config & ~RDY_MASK, CHANNEL1 | ONESHOT | RES_18_BITS | PGAx1);

    printf("\nTEST 12: Testing isReady(). Press enter to continue... ");
    getchar();
    usleep(400000);
    printf("conversion is %s ready, expected to be ready\n", dev.isReady() ? "" : "not");

    printf("\nTEST 13: Testing isReady(). Press enter to continue... ");
    getchar();
    usleep(400000);
    printf("conversion is %s ready, expected to be not ready\n", dev.isReady() ? "" : "not");

    printf("\nTEST 14: Testing isReady() and startConversion(). Press enter to continue... ");
    getchar();
    dev.startConversion();
    usleep(500000);
    printf("conversion is %s ready, expected to be ready\n", dev.isReady() ? "" : "not");

    printf("\nTEST 15: Testing isReady() and startConversion(). Press enter to continue... ");
    getchar();
    usleep(400000);
    printf("conversion is %s ready, expected to be not ready\n", dev.isReady() ? "" : "not");

    return 0;
}
