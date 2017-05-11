#include "mcp4725.h"
#include <wiringPiI2C.h>    // for I2C functions
#include <cstdio>           // for perror() and printf()
#include <cstdlib>          // for exit()
#include <unistd.h>         // for close()

// device constructor
MCP4725::MCP4725(uint8_t address) {
    // if it failed to open, print a message and throw an exception
    if ((fd = wiringPiI2CSetup(address)) < 0) {
        perror("Failed to open MCP4725");
        throw 1;
    }

    // set default DAC output to 0V
    writeEEPROM(0);
}

// write DAC output
void MCP4725::write(uint16_t data) {
    if (data > 0xFFF) { data = 0xFFF; }
    data = data << 4;
    data = (data >> 8) | (data << 8);
    printf("writing %x\n", data);
    if (wiringPiI2CWriteReg16(fd, MCP4725_WRITE, data) < 0) {
        perror("Failed to communicate with MCP4725");
        throw 1;
    }
}

// write EEPROM and DAC output
void MCP4725::writeEEPROM(uint16_t data) {
    if (data > 0xFFF) { data = 0xFFF; }
    data = data << 4;
    if (wiringPiI2CWriteReg16(fd, MCP4725_WRITE_EEPROM, data) < 0) {
        perror("Failed to communicate with MCP4725");
        throw 1;
    }
}

// read DAC state
uint32_t MCP4725::readDAC() {
    uint8_t data[4];
    if (read(fd, data, 4) < 0) {
        perror("Failed to communicate with MCP4725");
        throw 1;
    }
    uint32_t output = (((uint16_t) data[1]) << 8 | data[2]) >> 4;
    
    return output;
}

