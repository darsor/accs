#include "mcp3424.h"
#include <wiringPiI2C.h>
#include <cstdio>           // for perror() and printf()
#include <cstdlib>          // for exit()
#include <unistd.h>         // for close()

MCP3424::MCP3424(int address, char bits) {
    fd = wiringPiI2CSetup(address);
    if (fd < 0) {
        perror("Failed to open MCP3424");
        exit(1);
    }
    setConfig(bits);
}

MCP3424::~MCP3424() {
    close(fd);
}

void MCP3424::setConfig(char bits) {
    config = bits;
    wiringPiI2CWrite(fd, bits);
}

char MCP3424::getConfig() {
    static char temp[4];
    int bytes;
    if ((config & RES_MASK) == RES_18_BITS) {
        bytes = 4;
    } else {
        bytes = 3;
    }
    read(fd, temp, bytes);
    config = temp[bytes-1];
    return config;
}

void MCP3424::startConversion() {
    wiringPiI2CWrite(fd, config | RDY_MASK);
}

int32_t MCP3424::getConversion() {
    static char temp[3];
    int32_t conversion = 0;
    int bytes;
    if ((config & RES_MASK) == RES_18_BITS) {
        bytes = 3;
    } else {
        bytes = 2;
    }
    read(fd, temp, bytes);
    switch (config & RES_MASK) {
        case RES_12_BITS:
            conversion = ((int32_t)(temp[0] & 0x0F)) << 8 | temp[1];
            break;
        case RES_14_BITS:
            conversion = ((int32_t)(temp[0] & 0x3F)) << 8 | temp[1];
            break;
        case RES_16_BITS:
            conversion = ((int32_t) temp[0]) << 8 | temp[1];
            break;
        case RES_18_BITS:
            conversion = ((int32_t)(temp[0] & 0x03)) << 16 | ((int32_t) temp[1]) << 8 | temp[2];
            break;
    }
    return conversion;
}

bool MCP3424::isReady() {
    return !(getConfig() & RDY_MASK);
}

float MCP3424::toVoltage(int32_t code, int pga, int resolution) {
    float max = 0;
    switch (resolution) {
        case 12:
            max = 2047;
            break;
        case 14:
            max = 8192;
            break;
        case 16:
            max = 32767;
            break;
        case 18:
            max = 131071;
            break;
        default:
            printf("invalid resolution for voltage conversion)\n");
    }
    return (code / max) * (2.048 / pga) * 5.45454545;
}
