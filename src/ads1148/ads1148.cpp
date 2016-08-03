#include "ads1148.h"
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <cstdio>           // for printf() and perror()
#include <unistd.h>         // for close()
#include <string.h>         // for memcpy()
#include <mutex>
#include <condition_variable>

ADS1148* instancePtr = nullptr;
std::mutex isr_mutex;
bool drdy;
std::condition_variable isr_cv;

void ADS1148::drdyISR() {
    conversion.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    {
        std::lock_guard<std::mutex> lk(isr_mutex);
        drdy = true;
    }
    isr_cv.notify_one();
}

void adaptISR() {
    instancePtr->drdyISR();
}

ADS1148::ADS1148(int channel, int speed, int drdyPin):
    channel(channel), speed(speed), drdyPin(drdyPin) {
    instancePtr = this;
    if (wiringPiSetup() < 0) {
        perror("unable to set up WiringPi");
        throw 1;
    }
    fd = wiringPiSPISetupMode(channel, speed, 1);
    if (fd < 0) {
        perror("unable to open ADS1148");
        throw 1;
    }
    if (wiringPiISR(drdyPin, INT_EDGE_FALLING, adaptISR) < 0) {
        perror("unable to set up ADS1148 ISR");
        throw 1;
    }
}

ADS1148::~ADS1148() {
    close(fd);
}

void ADS1148::getConversion(StampedConversion &conv, char pos, char neg) {
    std::unique_lock<std::mutex> lk(isr_mutex);
    isr_cv.wait(lk, []{ return drdy; });
    drdy = false;
    char cmd[3];
    if (pos == 0 && neg == 0) {
        memset(cmd, SPI_NOP, 2);
        wiringPiSPIDataRW(channel, (unsigned char*) cmd, 2);
    } else {
        cmd[0] = SPI_WREG_MASK | REG_MUX0;
        cmd[1] = 0;
        cmd[2] = (pos << 3) | neg;
        wiringPiSPIDataRW(channel, (unsigned char*) cmd, 3);
    }
    conv.timestamp = conversion.timestamp;
    conv.code = ((int16_t) cmd[0]) << 8 | cmd[1];
    conversion.code = conv.code;
}

void ADS1148::setMux(char positive, char negative) {
    char bits = (positive << 3) | negative;
    writeReg(REG_MUX0, 1, &bits);
}

void ADS1148::setVREFstate(char state) {
    char bits;
    readReg(REG_MUX1, 1, &bits);
    bits = (bits & ~VREFCON_MASK) | state;
    writeReg(REG_MUX1, 1, &bits);
}

void ADS1148::setVREFsource(char source) {
    char bits;
    readReg(REG_MUX1, 1, &bits);
    bits = (bits & ~REFSELT_MASK) | source;
    writeReg(REG_MUX1, 1, &bits);
}

void ADS1148::setPGA(char pga) {
    char bits;
    readReg(REG_SYS0, 1, &bits);
    bits = (bits & ~ADS1148_PGA_MASK) | pga;
    writeReg(REG_SYS0, 1, &bits);
}

void ADS1148::setDataRate(char rate) {
    char bits;
    readReg(REG_SYS0, 1, &bits);
    bits = (bits & ~DATARATE_MASK) | rate;
    writeReg(REG_SYS0, 1, &bits);
}

void ADS1148::setDACmagnitude(char mag) {
    char bits;
    readReg(REG_IDAC0, 1, &bits);
    bits = (bits & ~DAC_MAG_MASK) | mag;
    writeReg(REG_IDAC0, 1, &bits);
}

void ADS1148::setDACpins(char dac1, char dac2) {
    char bits = (dac1 << 4) | dac2;
    writeReg(REG_IDAC1, 1, &bits);
}

char ADS1148::readReg(char reg) {
    char data;
    readReg(reg, 1, &data);
    return data;
}

void ADS1148::writeReg(char reg, int bytes, const char* data) {
    if (bytes < 0 || bytes > 15) {
        printf("INVALID write size\n");
        return;
    }
    char* cmd = new char[2+bytes];
    cmd[0] = SPI_WREG_MASK | reg;
    cmd[1] = bytes-1;
    memcpy(cmd+2, data, bytes);
    wiringPiSPIDataRW(channel, (unsigned char*) cmd, 2+bytes);
    usleep(100);
    delete[] cmd;
}

void ADS1148::readReg(char reg, int bytes, char* data) {
    if (bytes < 0 || bytes > 15) {
        printf("INVALID read size\n");
        return;
    }
    char* cmd = new char[2+bytes];
    cmd[0] = SPI_RREG_MASK | reg;
    cmd[1] = bytes - 1;
    memset(cmd+2, SPI_NOP, bytes);
    wiringPiSPIDataRW(channel, (unsigned char*) cmd, 2+bytes);
    memcpy(data, cmd+2, bytes);
    delete[] cmd;
}
