#include "mcp3424.h"
#include <unistd.h>
#include <iostream>
#include <cstdio>

int main() {
    MCP3424 dev(0x68);
    usleep(100000);
    int32_t data;

    dev.setConfig(CHANNEL1 | CONTINUOUS | RES_16_BITS | PGAx2);
    usleep(100000);

    for (int i=0; i<500; ++i) {
        usleep(70000);
        data = dev.getConversion();
        printf("Sample #%-3u:      0x%04x      %-8d (%-.5fV)\n", i, data, data, MCP3424::toVoltage(data, 2, 16));
    }

    return 0;
}
