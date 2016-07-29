#ifndef ADS1148_H
#define ADS1148_H

#include <cstdint>      // for precise integer definitions
#include <chrono>       // for timestamp

// Register definitions
#define REG_MUX0            0x00
#define REG_VBIAS           0x01
#define REG_MUX1            0x02
#define REG_SYS0            0x03
#define REG_OFC0            0x04
#define REG_OFC1            0x05
#define REG_OFC2            0x06
#define REG_FSC0            0x07
#define REG_FSC1            0x08
#define REG_FSC2            0x09
#define REG_IDAC0           0x0A
#define REG_IDAC1           0x0B
#define REG_GPIOCFG         0x0C
#define REG_GPIODIR         0x0D
#define REG_GPIODAT         0x0E

// Command definitions
#define SPI_WAKEUP          0x00
#define SPI_SLEEP           0x02
#define SPI_RESET           0x06
#define SPI_NOP             0xFF
#define SPI_RDATA           0x12
#define SPI_RDATAC          0x14
#define SPI_SDATAC          0x16
#define SPI_RREG_MASK       0x20
#define SPI_WREG_MASK       0x40

// Mask and value definitions
#define MUX_SP_MASK         0b00111000
#define MUX_SN_MASK         0b00000111
#define AIN0                0b000
#define AIN1                0b001
#define AIN2                0b010
#define AIN3                0b011
#define AIN4                0b100
#define AIN5                0b101
#define AIN6                0b110
#define AIN7                0b111

#define VREFCON_MASK        0b01100000
#define VREF_OFF            0b00000000
#define VREF_ON             0b00100000
#define VREF_AUTO           0b01100000

#define REFSELT_MASK        0b00011000
#define VREF_REF0           0b00000000
#define VREF_REF1           0b00001000
#define VREF_ONBOARD        0b00010000
#define VREF_ONBOARD_REF0   0b00011000

#define PGA_MASK            0b01110000
#define PGAx1               0b00000000
#define PGAx2               0b00010000
#define PGAx4               0b00100000
#define PGAx8               0b00110000
#define PGAx16              0b01000000
#define PGAx32              0b01010000
#define PGAx64              0b01100000
#define PGAx128             0b01110000

#define DATARATE_MASK       0b00001111
#define DATA_5SPS           0b00000000
#define DATA_10SPS          0b00000001
#define DATA_20SPS          0b00000010
#define DATA_40SPS          0b00000011
#define DATA_80SPS          0b00000100
#define DATA_160SPS         0b00000101
#define DATA_320SPS         0b00000110
#define DATA_640SPS         0b00000111
#define DATA_1000SPS        0b00001000
#define DATA_2000SPS        0b00001111

#define DRDY_MODE_MASK      0b00001000
#define DRDY_MODE_DOUT      0b00000000
#define DRDY_MODE_DOUT_DRDY 0b00001000

#define DAC_MAG_MASK        0b00000111
#define DAC_OFF             0b00000000
#define DAC_50uA            0b00000001
#define DAC_100uA           0b00000010
#define DAC_250uA           0b00000011
#define DAC_500uA           0b00000100
#define DAC_750uA           0b00000101
#define DAC_1000uA          0b00000110
#define DAC_1500uA          0b00000111

#define DAC1_SRC_MASK       0b11110000
#define DAC2_SRC_MASK       0b00001111
#define DAC_AIN0            0b0000
#define DAC_AIN1            0b0001
#define DAC_AIN2            0b0010
#define DAC_AIN3            0b0011
#define DAC_AIN4            0b0100
#define DAC_AIN5            0b0101
#define DAC_AIN6            0b0110
#define DAC_AIN7            0b0111
#define DAC_IEXT1           0b1000
#define DAC_IEXT2           0b1001
#define DAC_DISCONNECT      0b1111

struct StampedConversion {
    int64_t timestamp;
    int16_t code;
};

class ADS1148 {
    public:

        /*  ADS1148 constructor
         *  Parameters:
         *      int channel - the SPI channel to open device on (chip select)
         *      int speed - SPI clock speed in Hz (maximum 2MHz)
         *      int drdyPin - the wiringPi pin that the ADS1148 drdy is connected to
         *  Notes:
         *      Defaults are:
         *          Burnout current source off
         *          AIN0 positive, AIN1 negative channel
         *          Bias voltage off
         *          Internal reference off
         *          REF0 selected for reference voltage
         *          PGAx1
         *          5SPS output data rate
         *          IDACs off and disconnected
         */
        ADS1148(int channel, int speed, int drdyPin);
        ~ADS1148();

        /*  ADS1148 getConversion
         *  Parameters:
         *      StampedConversion &conv - this is what the function returns (by reference)
         *      (optional) char pos - new positive analog input for next conversion
         *          Valid arguments are AIN0-AIN7
         *      (optional) char neg - new negative analog input for next conversion
         *          Valid arguments are AIN0-AIN7
         *  Notes:
         *      This will block until ADS1148 triggers its DRDY signal
         *      This function can be used repeatedly to get conversion outputs.
         *      The optional arguments allow the user to select which channels the next
         *      conversion will be done on
         */
        void getConversion(StampedConversion &conv, char pos = 0, char neg = 0);

        /*  ADS1148 setMux
         *  Parameters:
         *      char positive - new positive analog input
         *          Valid arguments are AIN0-AIN7
         *      char positive - new positive analog input
         *          Valid arguments are AIN0-AIN7
         *  Notes:
         *      Set which analog inputs will be used for the conversions
         */
        void setMux(char positive, char negative);

        /*  ADS1148 setVREFstate
         *  Parameters:
         *      char state - set state of internal voltage reference
         *          Valid arguments are VREF_OFF, VREF_ON, or VREF_AUTO
         *  Notes:
         *      Turn on/off the internal voltage reference
         */
        void setVREFstate(char state);

        /*  ADS1148 setVREFsource
         *  Parameters:
         *      char source - which source to use as voltage reference
         *          Valid arguments are VREF_REF0, VREF_REF1,
         *          VREF_ONBOARD, or VREF_ONBOARD_REF0
         *  Notes:
         *      Select the source that the ADS1148 will use as
         *      voltage reference for conversions
         */
        void setVREFsource(char source);

        /*  ADS1148 setPGA
         *  Parameters:
         *      char pga - programmable gain amplifier
         *          Valid arguments are PGAx1, PGAx2, PGAx4, PGAx8,
         *          PGAx16, PGAx32, PGAx64, or PGAx128
         *  Notes:
         *      Set the input gain on the analog pins (before conversion)
         */
        void setPGA(char pga);

        /*  ADS1148 setDataRate
         *  Parameters:
         *      char rate - data rate in Samples Per Second (SPS)
         *          Valid arguments are DATA_5SPS, DATA_10SPS, DATA_20SPS,
         *          DATA_40SPS, DATA_80SPS, DATA_160SPS, DATA_320SPS,
         *          DATA_640SPS, DATA_1000SPS, or DATA_2000SPS
         *  Notes:
         *      Set the data rate of the ADS1148
         */
        void setDataRate(char rate);

        /*  ADS1148 setDACmagnitude
         *  Parameters:
         *      char mag - magnitude of matched excitation currents
         *          Valid arguments are DAC_OFF, DAC_50uA, DAC_100uA,
         *          DAC_250uA, DAC_500uA, DAC_750uA, DAC1000uA, DAC1500uA
         *  Notes:
         *      Set the magnitude of the matched excitation currents
         */
        void setDACmagnitude(char mag);

        /*  ADS1148 setDACpins
         *  Parameters:
         *      char dac1 - output pin for the first current source DAC
         *          Valid arguments are DAC_AIN0, DAC_AIN1, DAC_AIN2,
         *          DAC_AIN3, DAC_AIN4, DAC_AIN5, DAC_AIN6, DAC_AIN7,
         *          DAC_IEXT1, DAC_IEXT2, or DAC_DISCONNECT
         *      char dac2 - output pin for the second current source DAC
         *          Valid arguments are DAC_AIN0, DAC_AIN1, DAC_AIN2,
         *          DAC_AIN3, DAC_AIN4, DAC_AIN5, DAC_AIN6, DAC_AIN7,
         *          DAC_IEXT1, DAC_IEXT2, or DAC_DISCONNECT
         *  Notes:
         *      Set the output pins for both current source DACs
         */
        void setDACpins(char dac1, char dac2);

        char readReg(char reg);

        /*  ADS1148 drdyISR
         *  Notes:
         *      For internal use only
         */
        void drdyISR();
        
    private:
        int fd;
        int channel, speed, drdyPin;
        void writeReg(char reg, int bytes, const char* data);
        void readReg(char reg, int bytes, char* data);
        struct StampedConversion conversion;
};

#endif
