#include "ads1148.h"
#include <cstdio>       // for printf()
#include <unistd.h>     // for usleep()

int main() {
    StampedConversion conv;
    ADS1148 dev(0, 400000, 4);
    usleep(300000);

    printf("TEST 0: Reading MUX0 register\n");
    printf("register is 0x%02x, expecting 0x%02x\n", dev.readReg(REG_MUX0), 1);
    getchar();

    printf("TEST 1: Reading SYS0 register\n");
    printf("register is 0x%02x, expecting 0x%02x\n", dev.readReg(REG_SYS0), 0);
    getchar();

    printf("TEST 2: Writing SYS0 register\n");
    dev.setPGA(ADS1148_PGAx32);
    dev.setDataRate(DATA_40SPS);
    printf("register is 0x%02x, expecting 0x%02x\n", dev.readReg(REG_SYS0), 0b01010011);
    getchar();

    printf("TEST 3: Writing SYS0 register\n");
    dev.setPGA(ADS1148_PGAx4);
    dev.setDataRate(DATA_20SPS);
    printf("register is 0x%02x, expecting 0x%02x\n", dev.readReg(REG_SYS0), 0b00100010);
    getchar();
    dev.setPGA(ADS1148_PGAx1);
    dev.setDataRate(DATA_5SPS);

    printf("TEST 4: Turning on internal voltage reference\n");
    dev.setVREFstate(VREF_OFF);
    printf("reference is turned off - test the voltage across the test points");
    getchar();
    dev.setVREFstate(VREF_ON);
    printf("reference is turned on - test the voltage across the test points");
    getchar();
    dev.setVREFsource(VREF_ONBOARD_REF0);
    printf("now using onboard reference --tied to ref0 pins. test them now");
    getchar();

    printf("TEST 5: Getting conversions from channel 1 (AIN0 positive, AIN1 negative)");
    getchar();
    for (int i=0; i<5; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();

    printf("TEST 6: Getting conversions from channel 2 (AIN2 positive, AIN3 negative)");
    dev.setMux(AIN2, AIN3);
    usleep(60000);
    getchar();
    for (int i=0; i<5; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();

    printf("TEST 7: Getting alternating conversions from channels 1 and 2");
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

    printf("TEST 8: PGA gains");
    dev.setPGA(ADS1148_PGAx1);
    printf("ADS1148_PGAx1\n");
    for (int i=0; i<5; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();
    dev.setPGA(ADS1148_PGAx2);
    printf("ADS1148_PGAx2\n");
    for (int i=0; i<5; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();
    dev.setPGA(ADS1148_PGAx4);
    printf("ADS1148_PGAx4\n");
    for (int i=0; i<5; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();
    dev.setPGA(ADS1148_PGAx8);
    printf("ADS1148_PGAx8\n");
    for (int i=0; i<5; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();
    dev.setPGA(ADS1148_PGAx16);
    printf("ADS1148_PGAx16\n");
    for (int i=0; i<5; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();
    dev.setPGA(ADS1148_PGAx32);
    printf("ADS1148_PGAx32\n");
    for (int i=0; i<5; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();
    dev.setPGA(ADS1148_PGAx64);
    printf("ADS1148_PGAx64\n");
    for (int i=0; i<5; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();
    dev.setPGA(ADS1148_PGAx128);
    printf("ADS1148_PGAx128\n");
    for (int i=0; i<5; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();
    dev.setPGA(ADS1148_PGAx1);

    printf("TEST 9: DAC currents to IOUT1 and IOUT2\n");
    dev.setVREFstate(VREF_OFF);
    dev.setDACmagnitude(DAC_OFF);
    dev.setDACpins(DAC_DISCONNECT, DAC_DISCONNECT);
    printf("current output should be 0A - test it now");
    getchar();
    dev.setVREFstate(VREF_ON);
    dev.setDACmagnitude(DAC_50uA);
    dev.setDACpins(DAC_IEXT1, DAC_IEXT2);
    printf("current output should be 50uA - test it now");
    getchar();
    dev.setDACmagnitude(DAC_100uA);
    printf("current output should be 100uA - test it now");
    getchar();
    dev.setDACmagnitude(DAC_250uA);
    printf("current output should be 250uA - test it now");
    getchar();
    dev.setDACmagnitude(DAC_500uA);
    printf("current output should be 500uA - test it now");
    getchar();
    dev.setDACmagnitude(DAC_750uA);
    printf("current output should be 750uA - test it now");
    getchar();
    dev.setDACmagnitude(DAC_1000uA);
    printf("current output should be 1000uA - test it now");
    getchar();
    dev.setDACmagnitude(DAC_1500uA);
    printf("current output should be 1500uA - test it now");
    getchar();
    dev.setDACmagnitude(DAC_50uA);
    dev.setDACpins(DAC_AIN2, DAC_AIN6);
    printf("current output should be 50uA on AIN2 and AIN6 - test it now");
    getchar();
    dev.setDACpins(DAC_DISCONNECT, DAC_DISCONNECT);
    dev.setDACmagnitude(DAC_OFF);
    printf("DACs are now disabled\n");
    getchar();

    printf("TEST 10: Switching to VREF1 (on EVM)");
    dev.setVREFstate(VREF_OFF);
    dev.setVREFsource(VREF_REF1);
    printf("getting conversions from channel 1 (AIN0 positive, AIN1 negative)");
    getchar();
    for (int i=0; i<5; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();

    printf("TEST 11: Data rate tests - ADS1148_PGAx2");
    dev.setPGA(ADS1148_PGAx2);
    dev.setDataRate(DATA_5SPS);
    usleep(10000);
    printf("data rate is 5 sps\n");
    for (int i=0; i<5; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();
    dev.setDataRate(DATA_10SPS);
    usleep(10000);
    printf("data rate is 10 sps\n");
    for (int i=0; i<5; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();
    dev.setDataRate(DATA_20SPS);
    usleep(10000);
    printf("data rate is 20 sps\n");
    for (int i=0; i<5; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();
    dev.setDataRate(DATA_40SPS);
    usleep(10000);
    printf("data rate is 40 sps\n");
    for (int i=0; i<10; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();
    dev.setDataRate(DATA_80SPS);
    usleep(10000);
    printf("data rate is 80 sps\n");
    for (int i=0; i<10; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();
    dev.setDataRate(DATA_160SPS);
    usleep(10000);
    printf("data rate is 160 sps\n");
    for (int i=0; i<10; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();
    dev.setDataRate(DATA_320SPS);
    usleep(10000);
    printf("data rate is 320 sps\n");
    for (int i=0; i<10; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();
    dev.setDataRate(DATA_640SPS);
    usleep(10000);
    printf("data rate is 640 sps\n");
    for (int i=0; i<10; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();
    dev.setDataRate(DATA_1000SPS);
    usleep(10000);
    printf("data rate is 1000 sps\n");
    for (int i=0; i<20; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();
    dev.setDataRate(DATA_2000SPS);
    usleep(10000);
    printf("data rate is 2000 sps\n");
    for (int i=0; i<20; ++i) {
        dev.getConversion(conv);
        printf("conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }
    getchar();

    printf("TEST 12: Multiplexer switching");
    dev.setDataRate(DATA_1000SPS);
    usleep(10000);
    printf("press enter to start gathering 1000Hz samples from channels 1 and 2");
    getchar();
    for (int i=0; i<50; ++i) {
        dev.getConversion(conv, AIN2, AIN3);
        printf("channel 1 conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
        dev.getConversion(conv, AIN0, AIN1);
        printf("channel 2 conversion code is %04x, timestamp is %lli\n", conv.code, conv.timestamp);
    }

    return 0;
}
