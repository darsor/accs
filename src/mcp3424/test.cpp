#include "mcp3424.h"
#include <unistd.h>
#include <iostream>
#include <cstdio>

int main() {
    MCP3424 dev(0x68);
    usleep(100000);
    int32_t data;
    char config;

    printf("TEST 1: Testing getConfig(). Press enter to continue... ");
    getchar();

    config = dev.getConfig();
    printf("config is 0x%02x, expecting 0x%02x\n", config & ~RDY_MASK, CHANNEL1 | CONTINUOUS | RES_12_BITS | PGAx1);

    printf("\nTEST 2: Testing getConfig(). Press enter to continue... ");
    getchar();

    dev.setConfig(CHANNEL4 | ONESHOT | RES_16_BITS | PGAx2);
    usleep(200000);
    config = dev.getConfig();
    printf("config is 0x%02x, expecting 0x%02x\n", config & ~RDY_MASK, CHANNEL4 | ONESHOT | RES_16_BITS | PGAx2);

    printf("\nTEST 3: Testing getConfig(). Press enter to continue... ");
    getchar();

    dev.setConfig(CHANNEL1 | ONESHOT | RES_18_BITS | PGAx1);
    usleep(400000);
    config = dev.getConfig();
    printf("config is 0x%02x, expecting 0x%02x\n", config & ~RDY_MASK, CHANNEL1 | ONESHOT | RES_18_BITS | PGAx1);

    printf("\nTEST 4: Testing isReady(). Press enter to continue... ");
    getchar();
    usleep(400000);
    printf("conversion is %sready, expected to be ready\n", dev.isReady() ? "" : "not ");

    printf("\nTEST 5: Testing isReady(). Press enter to continue... ");
    getchar();
    dev.startConversion();
    usleep(50000);
    printf("conversion is %sready, expected to be not ready\n", dev.isReady() ? "" : "not ");

    printf("\nTEST 6: Testing isReady() and startConversion(). Press enter to continue... ");
    getchar();
    usleep(500000);
    printf("conversion is %sready, expected to be ready\n", dev.isReady() ? "" : "not ");

    printf("\nTEST 7: Testing isReady() and startConversion(). Press enter to continue... ");
    getchar();
    dev.startConversion();
    usleep(500000);
    printf("conversion is %sready, expected to be ready\n", dev.isReady() ? "" : "not ");

    printf("\nTEST 8: Reading 12-bit values from channel 1 (PGAx1). Press enter to continue... ");
    getchar();
    printf("\n");

    dev.setConfig(CHANNEL1 | CONTINUOUS | RES_12_BITS | PGAx1);
    usleep(100000);
    for (int i=0; i<10; ++i) {
        data = dev.getConversion();
        printf("Data point %d: 0x%03x\n", i, data);
        usleep(100000);
    }

    printf("\nTEST 9: Reading 14_bit values from channel 1 (PGAx1). Press enter to continue... ");
    getchar();
    printf("\n");

    dev.setConfig(CHANNEL1 | CONTINUOUS | RES_14_BITS | PGAx1);
    usleep(100000);
    for (int i=0; i<10; ++i) {
        data = dev.getConversion();
        printf("Data point %d: 0x%04x\n", i, data);
        usleep(100000);
    }

    printf("\nTEST 10: Reading 16_bit values from channel 1 (PGAx1). Press enter to continue... ");
    getchar();
    printf("\n");

    dev.setConfig(CHANNEL1 | CONTINUOUS | RES_16_BITS | PGAx1);
    usleep(200000);
    for (int i=0; i<10; ++i) {
        data = dev.getConversion();
        printf("Data point %d: 0x%04x\n", i, data);
        usleep(200000);
    }

    printf("\nTEST 11: Reading 18_bit values from channel 1 (PGAx1). Press enter to continue... ");
    getchar();
    printf("\n");

    dev.setConfig(CHANNEL1 | CONTINUOUS | RES_18_BITS | PGAx1);
    usleep(400000);
    for (int i=0; i<10; ++i) {
        data = dev.getConversion();
        printf("Data point %d: 0x%05x\n", i, data);
        usleep(400000);
    }

    printf("\nTEST 12: Reading 12-bit values from channel 2 (PGAx2). Press enter to continue... ");
    getchar();
    printf("\n");

    dev.setConfig(CHANNEL2 | CONTINUOUS | RES_12_BITS | PGAx2);
    for (int i=0; i<10; ++i) {
        data = dev.getConversion();
        printf("Data point %d: 0x%03x\n", i, data);
        usleep(100000);
    }

    printf("\nTEST 13: Reading 14_bit values from channel 2 (PGAx2). Press enter to continue... ");
    getchar();
    printf("\n");

    dev.setConfig(CHANNEL2 | CONTINUOUS | RES_14_BITS | PGAx2);
    usleep(100000);
    for (int i=0; i<10; ++i) {
        data = dev.getConversion();
        printf("Data point %d: 0x%04x\n", i, data);
        usleep(100000);
    }

    printf("\nTEST 14: Reading 16_bit values from channel 2 (PGAx2). Press enter to continue... ");
    getchar();
    printf("\n");

    dev.setConfig(CHANNEL2 | CONTINUOUS | RES_16_BITS | PGAx2);
    usleep(200000);
    for (int i=0; i<10; ++i) {
        data = dev.getConversion();
        printf("Data point %d: 0x%04x\n", i, data);
        usleep(200000);
    }

    printf("\nTEST 15: Reading 18_bit values from channel 2 (PGAx2). Press enter to continue... ");
    getchar();
    printf("\n");

    dev.setConfig(CHANNEL2 | CONTINUOUS | RES_18_BITS | PGAx2);
    usleep(400000);
    for (int i=0; i<10; ++i) {
        data = dev.getConversion();
        printf("Data point %d: 0x%05x\n", i, data);
        usleep(400000);
    }

    return 0;
}
