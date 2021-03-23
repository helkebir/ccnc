#include "src/ccnc.h"

int main(int argc, char **argv) {
    const char *chipname = "gpiochip0";
    struct gpiod_chip *chip;
    chip = gpiod_chip_open_by_name(chipname);
    
    struct MotorPins xMotorPins = {
        .stp = 21,
        .dir = 20,
        .ms1 = 8,
        .ms2 = 7,
        .ms3 = 1
    };
    struct MotorGPIOD xMotor;
    initMotor(&xMotorPins, &xMotor, chip);

    struct MotorPins yMotorPins = {
        .stp = 26,
        .dir = 19,
        .ms1 = 5,
        .ms2 = 6,
        .ms3 = 13
    };
    struct MotorGPIOD yMotor;
    initMotor(&yMotorPins, &yMotor, chip);

    struct MotorPins zMotorPins = {
        .stp = 11,
        .dir = 9,
        .ms1 = 22,
        .ms2 = 10,
        .ms3 = 9
    };
    struct MotorGPIOD zMotor;
    initMotor(&zMotorPins, &zMotor, chip);

    struct MotorGPIOD *motors[3] = {&xMotor, &yMotor, &zMotor};

    demoMotors(&xMotor, &yMotor, &zMotor);
    
    
    releaseMotor(&xMotor);
    releaseMotor(&yMotor);
    releaseMotor(&zMotor);
    gpiod_chip_close(chip);

    return 0;
}