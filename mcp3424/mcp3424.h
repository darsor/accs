#ifndef MCP3424_H
#define MCP3424_H

#include <cstdint>

// big masks and values for use with the constructor and setConfig()
#define CHANNEL_MASK    0b01100000
#define CHANNEL1        0b00000000
#define CHANNEL2        0b00100000
#define CHANNEL3        0b01000000
#define CHANNEL4        0b01100000

#define MODE_MASK       0b00010000
#define ONESHOT         0b00000000
#define CONTINUOUS      0b00010000

#define RES_MASK        0b00001100
#define RES_12_BITS     0b00000000
#define RES_14_BITS     0b00000100
#define RES_16_BITS     0b00001000
#define RES_18_BITS     0b00001100

#define PGA_MASK        0b00000011
#define PGAx1           0b00000000
#define PGAx2           0b00000001
#define PGAx3           0b00000010
#define PGAx4           0b00000011

#define RDY_MASK        0b10000000

class MCP3424 {
    public:
        MCP3424(int address, char bits = CHANNEL1 | CONTINUOUS | RES_12_BITS | PGAx1);
        ~MCP3424();
        void setConfig(char bits);
        char getConfig();
        void startConversion(); // starts conversion in oneshot mode
        int32_t getConversion();
        bool isReady();
        static float toVoltage(int32_t code, int pga, int resolution);

    private:
        int fd;
        char config;
};

#endif
