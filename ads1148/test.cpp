#include "ads1148.h"
#include <cstdio>       // for printf()
#include <unistd.h>     // for usleep()

int main() {
    StampedConversion conv;
    ADS1148 dev(1, 100000, 4);
    usleep(300000);

    printf("TEST 0: Reading MUX0 register\n");
    printf("register is 0x%02x, expecting 0x%02x\n", dev.readReg(REG_MUX0), 0);
    getchar();

    printf("TEST 1: Reading SYS0 register\n");
    printf("register is 0x%02x, expecting 0x%02x\n", dev.readReg(REG_SYS0), 0);
    getchar();

    printf("TEST 2: Writing SYS0 register\n");
    dev.setPGA(PGAx32);
    dev.setDataRate(DATA_40SPS);
    printf("register is 0x%02x, expecting 0x%02x\n", dev.readReg(REG_SYS0), 0b01010011);
    getchar();

    printf("TEST 3: Writing SYS0 register\n");
    dev.setPGA(PGAx4);
    dev.setDataRate(DATA_20SPS);
    printf("register is 0x%02x, expecting 0x%02x\n", dev.readReg(REG_SYS0), 0b00100010);
    getchar();

    printf("TEST 4: Getting conversions from channel 1 (AIN0 positive, AIN1 negative)");
    getchar();
    for (int i=0; i<5; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();

    printf("TEST 5: Getting conversions from channel 2 (AIN2 positive, AIN3 negative)");
    dev.setMux(AIN2, AIN3);
    usleep(60000);
    getchar();
    for (int i=0; i<5; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();

    printf("TEST 6: Getting alternating conversions from channels 1 and 2");
    dev.setMux(AIN0, AIN1);
    usleep(60000);
    getchar();
    for (int i=0; i<5; ++i) {
        dev.getConversion(conv, AIN2, AIN3);
        printf("channel 1 conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
        dev.getConversion(conv, AIN0, AIN1);
        printf("channel 2 conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();

    printf("TEST 7: Turning on internal voltage reference\n");
    printf("reference is turned off - test the voltage across the test points");
    getchar();
    dev.setVREFstate(VREF_ON);
    printf("reference is turned on - test the voltage across the test points");
    getchar();
    dev.setVREFstate(VREF_OFF);
    printf("reference is turned off again - measure the test points\n");
    getchar();

    printf("TEST 8: Turning on DAC currents to IOUT1 and IOUT2\n");
    dev.setVREFstate(VREF_ON);
    dev.setDACmagnitude(DAC_250uA);
    dev.setDACpins(DAC_IEXT1, DAC_IEXT2);
    printf("current output should be 250uA - test it now");
    getchar();
    dev.setDACmagnitude(DAC_1500uA);
    printf("current output should be 1.5mA - test it now");
    getchar();
    dev.setDACmagnitude(DAC_50uA);
    dev.setDACpins(DAC_AIN2, DAC_AIN6);
    printf("current output should be 50uA on AIN2 and AIN6 - test it now");
    getchar();
    dev.setDACpins(DAC_DISCONNECT, DAC_DISCONNECT);
    dev.setDACmagnitude(DAC_OFF);
    printf("DACs are now disabled\n");
    getchar();

    return 0;
}
