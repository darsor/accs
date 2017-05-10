#ifndef MCP4725_H
#define MCP4725_H

#include <stdint.h>

#define MCP4725_WRITE           0x40
#define MCP4725_WRITE_EEPROM    0x60

class MCP4725 {
    public:
        /*  MCP3424 constructor
         *  Parameters:
         *      uint8_t address - the I2C address of the device
         *  Notes:
         *      Sets default start-up condition to output 0V
         */
        MCP4725(uint8_t address);

        /*  Write DAC output
         *  Parameters:
         *      uint16_t data - 12 bit number (0 to 4095) controlling DAC output
         */
        void write(uint16_t data);

        /*  Write DAC EEPROM
         *  Parameters:
         *      uint16_t data - 12 bit number (0 to 4095) controlling DAC output
         *  Notes:
         *      the given value becomes the default DAC output at startup
         *      also writes the volatile DAC register
         */
        void writeEEPROM(uint16_t data);

        /*  Read DAC output
         *  Return values:
         *      uint32_t - current status of the DAC register
         */
        uint32_t readDAC();
        
    private:
        int fd;
};

#endif
