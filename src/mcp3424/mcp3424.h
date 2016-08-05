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
#define PGAx4           0b00000010
#define PGAx8           0b00000011

#define RDY_MASK        0b10000000

class MCP3424 {
    public:

        /*  MCP3424 constructor
         *  Parameters:
         *      int address - the I2C address of the device
         *      char bits - starting configuration for the device. Defaults have bene given.
         *          takes the same input as the setConfig() function
         *  Notes:
         *      Defaults are:
         *          Channel 1, continuous mode, 12-bit ADC resolution, PGA x1
         */
        MCP3424(int address, char bits = CHANNEL1 | CONTINUOUS | RES_12_BITS | PGAx1);
        ~MCP3424();

        /*  MCP3424 setConfig()
         *  Parameters:
         *      char bits - configuration for the device. Includes:
         *          Channel, operating mode, resolution, and pga.
         *          All four must be included each time this function is called.
         *          Valid arguments are:
         *              Channel: CHANNEL1, CHANNEL2, CHANNEL3, or CHANNEL4
         *              Mode: ONESHOT or CONTINUOUS
         *              Resolution: RES_12_BITS, RES_14_BITS, RES_16_BITS, or RES_18_BITS
         *              PGA: PGAx1, PGAx2, PGAx4, or PGAx8
         *  Notes:
         *      See the constructor for an example of how to call this function
         *      (it takes the same arguments for it's 'bits' parameter
         */
        void setConfig(char bits);

        /*  MCP3424 getConfig()
         *  Return values:
         *      char - the value of the configuration register set by the setConfig() function
         *  Notes:
         *      This can be used to check current configuration values (using the defined
         *      bit masks, if you're saavy).
         */
        char getConfig();

        /*  MCP3424 startConversion()
         *  Notes:
         *      This function writes the ready bit of the config register, signaling
         *      the device to start a conversion. This does not do anything in
         *      continuous mode.
         */
        void startConversion();

        /*  MCP3424 getConversion()
         *  Return values:
         *      int32_t - 32 bit integer that is the most recent conversion result
         *  Notes:
         *      The isReady() function can be used to determine if the conversion
         *      result is ready to be read
         */
        int32_t getConversion();


        /*  MCP3424 getConversion()
         *  Return values:
         *      bool - whether or not there is a new conversion result ready to be read
         *  Notes:
         *      This function can be called repeatedly until it returns true
         */
        bool isReady();

        /*  MCP3424 toVoltage()
         *  Parameters:
         *      int32_t code - the conversion result given by getConversion()
         *      int pga - the gain of the PGA (valid arguments are 1, 2, 4 or 8)
         *      int resolution - resolution of the ADC (valid arguments are 12, 14, 16, or 18)
         *  Return values:
         *      float - the input voltage specified by the conversion result
         *  Notes:
         *      If measuring current, you can get Amps instead of Volts by
         *      dividing the result by 249.
         */
        static float toVoltage(int32_t code, int pga, int resolution);

    private:
        int fd;
        char config;
};

#endif
