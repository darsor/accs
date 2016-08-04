#include "mcp3424.h"
#include <wiringPiI2C.h>    // for I2C functions
#include <cstdio>           // for perror() and printf()
#include <cstdlib>          // for exit()
#include <unistd.h>         // for close()

// set up and configure the device
MCP3424::MCP3424(int address, char bits) {
    // if it failed to open, print a message and throw an exception
    if ((fd = wiringPiI2CSetup(address)) < 0) {
        perror("Failed to open MCP3424");
        throw 1;
    }
    // set the inital configuration
    setConfig(bits);
}

// close the MCP3424 when we're done with it
MCP3424::~MCP3424() {
    close(fd);
}

// set the configuration byte
void MCP3424::setConfig(char bits) {
    config = bits;
    if (wiringPiI2CWrite(fd, bits) < 0) {
        perror("Failed to communicate with MCP3424");
        throw 1;
    }
}

// get the configuration byte
char MCP3424::getConfig() {
    static char temp[4];
    int bytes;
    // if it's set to 18-bit resolution, the config byte will be the 4th one returned
    if ((config & RES_MASK) == RES_18_BITS) {
        bytes = 4;
    // otherwise, it will be the 3rd
    } else {
        bytes = 3;
    }
    if (read(fd, temp, bytes) < 0) {
        perror("Failed to communicate with MCP3424");
        throw 1;
    }
    config = temp[bytes-1];
    return config;
}

// write the ready bit of the configuration byte
void MCP3424::startConversion() {
    if (wiringPiI2CWrite(fd, config | RDY_MASK) < 0) {
        perror("Failed to communicate with MCP3424");
        throw 1;
    }
}

// get the conversion result
int32_t MCP3424::getConversion() {
    static char temp[3];
    int32_t conversion = 0;
    int bytes;
    // if it's set to 18-bit resolution, the result will be 3 bytes (18 bits)
    if ((config & RES_MASK) == RES_18_BITS) {
        bytes = 3;
    // otherwise, it will be 2 bytes (12, 14, or 16 bits)
    } else {
        bytes = 2;
    }
    if (read(fd, temp, bytes) < 0) {
        perror("Failed to communicate with MCP3424");
        throw 1;
    }
    // mash the bytes together depending on the resolution
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

// tell if a conversion result is ready or not
bool MCP3424::isReady() {
    return !(getConfig() & RDY_MASK);
}

// simple equation to convert conversion codes into voltages
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
