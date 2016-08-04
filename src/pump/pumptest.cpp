#include "dcmotor.h"
#include <iostream>

int main() {
    float speed;
    DCMotor motor(3, 0x60, 1500);

    while (true) {
        std::cout << "Enter a voltage (0-12V): ";
        std::cin >> speed;
        if (speed == 0) break;
        if (speed < 0 || speed > 12) continue;
        speed *= 255.0 / 12.0;

        motor.setGradSpeed((int) speed);
    }
    motor.run(RELEASE);

    return 0;
}
