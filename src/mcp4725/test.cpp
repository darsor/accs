#include "mcp4725.h"
#include <stdio.h>

int main() {

    MCP4725 dac(0x60);
    float input = 0;
    int output = 0;
    while (1) {
        printf("Enter a voltage between 0-3.3 V: ");
        scanf("%f", &input);
        output = 4095 * (input/3.3);
        printf("sending %f (%x) to DAC\n", input, output);
        dac.write(output);
        printf("DAC state is %x\n", dac.readDAC());
        if (input == 0) { break; }
    }

    return 0;
}
