#include "ads1148.h"
#include <wiringPi.h>           // for wiringPiISR()
#include <wiringPiSPI.h>        // for SPI functions
#include <cstdio>               // for printf() and perror()
#include <unistd.h>             // for close()
#include <string.h>             // for memcpy()
#include <mutex>                // for mutexes
#include <condition_variable>   // for condition variables

// this pointer is a tricky way of helping a non-static member function
// be used as a callback for the DRDY ISR
ADS1148* instancePtr = nullptr;

// these variables are used to wait for the DRDY without polling the CPU
std::mutex isr_mutex;
bool drdy;
std::condition_variable isr_cv;

// this is what is called when the DRDY indicates the conversion is ready
void ADS1148::drdyISR() {
    // get a timestamp
    conversion.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    {
        std::lock_guard<std::mutex> lk(isr_mutex);
        drdy = true;
    }
    // notify anyone that's waiting for the DRDY pin that it has gone low
    isr_cv.notify_one();
}

// this function simply calls drdyISR() for the interrupt
void adaptISR() {
    instancePtr->drdyISR();
}

// set up the device
ADS1148::ADS1148(int channel, int speed, int drdyPin):
    channel(channel), speed(speed), drdyPin(drdyPin) {
    instancePtr = this;
    if (wiringPiSetup() < 0) {
        perror("unable to set up WiringPi");
        throw 1;
    }
    // open the device with SPI mode 1 (actions on falling edge of SCLK)
    fd = wiringPiSPISetupMode(channel, speed, 1);
    if (fd < 0) {
        perror("unable to open ADS1148");
        throw 1;
    }
    // set up the DRDY ISR
    if (wiringPiISR(drdyPin, INT_EDGE_FALLING, adaptISR) < 0) {
        perror("unable to set up ADS1148 ISR");
        throw 1;
    }
}

// close the device when we're done with it
ADS1148::~ADS1148() {
    close(fd);
}

// get the conversion result and optionally specify the next channel to be measured
void ADS1148::getConversion(StampedConversion &conv, char pos, char neg) {
    // wait for drdyISR() to indicate that a conversion is ready
    std::unique_lock<std::mutex> lk(isr_mutex);
    isr_cv.wait(lk, []{ return drdy; });
    drdy = false;

    char cmd[3];
    // if no new channel was specified, send two bytes of NOP to read the result
    if (pos == 0 && neg == 0) {
        memset(cmd, SPI_NOP, 2);
        wiringPiSPIDataRW(channel, (unsigned char*) cmd, 2);
    // if there were new channels specified, use that command to read the result
    // SPI is full-duplex, so we can read the result and send the command at the same time
    } else {
        cmd[0] = SPI_WREG_MASK | REG_MUX0;
        cmd[1] = 0;
        cmd[2] = (pos << 3) | neg;
        wiringPiSPIDataRW(channel, (unsigned char*) cmd, 3);
    }
    // grab the timestamp that drdyISR() saved
    conv.timestamp = conversion.timestamp;
    conv.code = ((int16_t) cmd[0]) << 8 | cmd[1];
    conversion.code = conv.code;
}

// set the channels used for conversion
void ADS1148::setMux(char positive, char negative) {
    char bits = (positive << 3) | negative;
    writeReg(REG_MUX0, 1, &bits);
}

// set the state of the internal voltage reference
void ADS1148::setVREFstate(char state) {
    char bits;
    // it's only a few bits of a register, so read the contents of the register
    readReg(REG_MUX1, 1, &bits);
    // then modify the couple of bits
    bits = (bits & ~VREFCON_MASK) | state;
    // then write it back
    writeReg(REG_MUX1, 1, &bits);
}

// set the source of the voltage reference used for conversions
void ADS1148::setVREFsource(char source) {
    char bits;
    // it's only a few bits of a register, so read the contents of the register
    readReg(REG_MUX1, 1, &bits);
    // then modify the couple of bits
    bits = (bits & ~REFSELT_MASK) | source;
    // then write it back
    writeReg(REG_MUX1, 1, &bits);
}

// set the gain
void ADS1148::setPGA(char pga) {
    char bits;
    // it's only a few bits of a register, so read the contents of the register
    readReg(REG_SYS0, 1, &bits);
    // then modify the couple of bits
    bits = (bits & ~ADS1148_PGA_MASK) | pga;
    // then write it back
    writeReg(REG_SYS0, 1, &bits);
}

// set the data rate
void ADS1148::setDataRate(char rate) {
    char bits;
    // it's only a few bits of a register, so read the contents of the register
    readReg(REG_SYS0, 1, &bits);
    // then modify the couple of bits
    bits = (bits & ~DATARATE_MASK) | rate;
    // then write it back
    writeReg(REG_SYS0, 1, &bits);
}

// set the amperage of the matched current DACs
void ADS1148::setDACmagnitude(char mag) {
    char bits;
    // it's only a few bits of a register, so read the contents of the register
    readReg(REG_IDAC0, 1, &bits);
    // then modify the couple of bits
    bits = (bits & ~DAC_MAG_MASK) | mag;
    // then write it back
    writeReg(REG_IDAC0, 1, &bits);
}

// set which pins the current DACs will be routed through
void ADS1148::setDACpins(char dac1, char dac2) {
    char bits = (dac1 << 4) | dac2;
    writeReg(REG_IDAC1, 1, &bits);
}

// return the contents of a single register
char ADS1148::readReg(char reg) {
    char data;
    readReg(reg, 1, &data);
    return data;
}

// write to one or more registers
void ADS1148::writeReg(char reg, int bytes, const char* data) {
    if (bytes < 0 || bytes > 16) {
        printf("INVALID write size\n");
        return;
    }
    // the size of the command will be the number of registers to be written to
    // plus 2 bytes for the command itself
    char* cmd = new char[2+bytes];
    // set up the 2 bytes of the command
    cmd[0] = SPI_WREG_MASK | reg;
    cmd[1] = bytes-1;
    // copy the values passed in by 'data' into the command
    memcpy(cmd+2, data, bytes);
    // send the command
    wiringPiSPIDataRW(channel, (unsigned char*) cmd, 2+bytes);
    usleep(100);
    delete[] cmd;
}

// read one or more registers
void ADS1148::readReg(char reg, int bytes, char* data) {
    if (bytes < 0 || bytes > 16) {
        printf("INVALID read size\n");
        return;
    }
    // the size of the command will be the number of registers to be read
    // plus 2 bytes for the command itself
    char* cmd = new char[2+bytes];
    // set up the 2 bytes of the command
    cmd[0] = SPI_RREG_MASK | reg;
    cmd[1] = bytes - 1;
    // set the rest of the command to NOP
    memset(cmd+2, SPI_NOP, bytes);
    // send the command
    wiringPiSPIDataRW(channel, (unsigned char*) cmd, 2+bytes);
    // copy the returned bytes in the the 'data' argument
    memcpy(data, cmd+2, bytes);
    delete[] cmd;
}
