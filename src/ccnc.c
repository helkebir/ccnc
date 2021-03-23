#include "ccnc.h"

void initMotor(struct MotorPins *motorPins, struct MotorGPIOD *motorGPIOD,
    struct gpiod_chip *chip)
{
    motorGPIOD->stp = gpiod_chip_get_line(chip, motorPins->stp);
    gpiod_line_request_output(motorGPIOD->stp, "cnccontroller", 0);
    motorGPIOD->stpState = 0;

    motorGPIOD->dir = gpiod_chip_get_line(chip, motorPins->dir);
    gpiod_line_request_output(motorGPIOD->dir, "cnccontroller", 0);
    motorGPIOD->dirState = 0;
    
    motorGPIOD->ms1 = gpiod_chip_get_line(chip, motorPins->ms1);
    gpiod_line_request_output(motorGPIOD->ms1, "cnccontroller", 0);
    motorGPIOD->ms1State = 0;

    motorGPIOD->ms2 = gpiod_chip_get_line(chip, motorPins->ms2);
    gpiod_line_request_output(motorGPIOD->ms2, "cnccontroller", 0);
    motorGPIOD->ms2State = 0;

    motorGPIOD->ms3 = gpiod_chip_get_line(chip, motorPins->ms3);
    gpiod_line_request_output(motorGPIOD->ms3, "cnccontroller", 0);
    motorGPIOD->ms3State = 0;
}

void releaseMotor(struct MotorGPIOD *motorGPIOD)
{
    gpiod_line_release(motorGPIOD->stp);
    gpiod_line_release(motorGPIOD->dir);
    gpiod_line_release(motorGPIOD->ms1);
    gpiod_line_release(motorGPIOD->ms2);
    gpiod_line_release(motorGPIOD->ms3);
}

void prepareSubcommand(uint scmd, struct MotorGPIOD *motorGPIOD)
{
    bool stateChanged = false;

    if (motorGPIOD->ms1State != (scmd&4)>>2) {
        motorGPIOD->ms1State = (scmd&4)>>2;
        stateChanged = true;
        gpiod_line_set_value(motorGPIOD->ms1, motorGPIOD->ms1State);
    }
    
    if (motorGPIOD->ms2State != (scmd&2)>>1) {
        motorGPIOD->ms2State = (scmd&2)>>1;
        stateChanged = true;
        gpiod_line_set_value(motorGPIOD->ms2, motorGPIOD->ms2State);
    }

    if (motorGPIOD->ms3State != scmd&1) {
        motorGPIOD->ms3State = scmd&1;
        stateChanged = true;
        gpiod_line_set_value(motorGPIOD->ms3, motorGPIOD->ms3State);
    }

    if ((motorGPIOD->dirState != (scmd&8)>>3) || stateChanged) {
        motorGPIOD->dirState = (scmd&8)>>3;
        gpiod_line_set_value(motorGPIOD->dir, motorGPIOD->dirState);
    }
}

void executeSubcommand(uint scmd, struct MotorGPIOD *motorGPIOD, uint wait)
{
    prepareSubcommand(scmd, motorGPIOD);

    if ((scmd&16)>>4) {
        gpiod_line_set_value(motorGPIOD->stp, 1);
        usleep(wait);
        gpiod_line_set_value(motorGPIOD->stp, 0);
        usleep(wait);
    } else {
        // nop
        usleep(2*wait);
    }
}

void executeSubcommandsOnMotors(uint scmds[], struct MotorGPIOD *motorGPIODs[],
    uint wait, uint number)
{
    for (int i=0; i < number; i++) {
        prepareSubcommand(scmds[i], motorGPIODs[i]);
    }

    struct gpiod_line *lines[GPIOD_LINE_BULK_MAX_LINES] = {NULL};
    int j = 0;
    for (int i=0; i < number; i++) {
        if ((scmds[i]&16)>>4) {
            lines[j] = motorGPIODs[i]->stp;
            j++;
        }
    }

    for (int i=0; i < j; i++) {
        gpiod_line_set_value(lines[i], 1);
    }
    usleep(wait);
    for (int i=0; i < j; i++) {
        gpiod_line_set_value(lines[i], 0);
    }
    usleep(wait);
}

void runScmd(uint scmd, uint wait, struct gpiod_line *stp,
    struct gpiod_line *dir, struct gpiod_line *ms1, struct gpiod_line *ms2,
    struct gpiod_line *ms3)
{
    if ((scmd&16)>>4) {
        gpiod_line_set_value(ms1, (scmd&4)>>2);
        gpiod_line_set_value(ms2, (scmd&2)>>1);
        gpiod_line_set_value(ms3, scmd&1);

        gpiod_line_set_value(dir, (scmd&8)>>3);

        gpiod_line_set_value(stp, 1);
        usleep(wait);
        gpiod_line_set_value(stp, 0);
        usleep(wait);
    }
}

void runScmdFromArray(uint scmd[5], uint wait, struct gpiod_line *stp,
    struct gpiod_line *dir, struct gpiod_line *ms1, struct gpiod_line *ms2,
    struct gpiod_line *ms3)
{
    if (scmd[0]) {
        gpiod_line_set_value(dir, scmd[1]);
        // gpiod_line_set_value(ms1, scmd[2]);
        // gpiod_line_set_value(ms2, scmd[3]);
        // gpiod_line_set_value(ms3, scmd[4]);

        gpiod_line_set_value(stp, 1);
        usleep(wait);
        gpiod_line_set_value(stp, 0);
        usleep(wait);
    }
}

uint parity(uint i)
{
    return (uint) (i%2 == 0);
}

uint addParityBit(uint i)
{
    return (i << 1) + parity(i);
}

bool checkParity(uint i, uint parityBit)
{
    return i%2 == parityBit;
}

uint concatenateSubcommands(uint scmd0, uint scmd1)
{
    return (scmd1 << 5) + scmd0;
}

uint* deconstructCommand(uint cmd)
{
    uint scmds[COMMAND_LENGTH] = {};

    if (checkParity(cmd>>1, cmd&1))
        for (uint i=0; i < COMMAND_LENGTH; i++)
            scmds[i] = (cmd >> 1 + i*5) & 0x1F;

    return scmds;
}

void demoMotors(struct MotorGPIOD *xMotor, struct MotorGPIOD *yMotor,
    struct MotorGPIOD *zMotor)
{
    int i = 0;
    int j = 0;
    bool fwd = true;
    while (i < 6) {
        j = 0;
        while (j < 200) {
            if (fwd)
                executeSubcommand(0b11000, xMotor, 1000);
            else
                executeSubcommand(0b10000, xMotor, 1000);
            j++;
        }
        fwd = !fwd;
        i++;
    }

    i = 0; j = 0; fwd = true;
    while (i < 6) {
        j = 0;
        while (j < 200) {
            if (fwd)
                executeSubcommand(0b11000, yMotor, 1000);
            else
                executeSubcommand(0b10000, yMotor, 1000);
            j++;
        }
        fwd = !fwd;
        i++;
    }

    i = 0; j = 0; fwd = true;
    while (i < 6) {
        j = 0;
        while (j < 200) {
            if (fwd)
                executeSubcommand(0b11000, zMotor, 1000);
            else
                executeSubcommand(0b10000, zMotor, 1000);
            j++;
        }
        fwd = !fwd;
        i++;
    }

    struct MotorGPIOD *motorsXYZ[3] = {xMotor, yMotor, zMotor};
    uint subcommandsFwd[3] = {0b11100, 0b11100, 0b11100};
    uint subcommandsBwd[3] = {0b10100, 0b10100, 0b10100};

    i = 0; j = 0; fwd = true;
    while (i < 6) {
        j = 0;
        while (j < 200) {
            if (fwd) {
                executeSubcommandsOnMotors(subcommandsFwd, motorsXYZ, 1000, 3);
            } else {
                executeSubcommandsOnMotors(subcommandsBwd, motorsXYZ, 1000, 3);
            }
            j++;
        }
        fwd = !fwd;
        i++;
    }

    // i = 0; j = 0; fwd = true;
    // while (i < 6) {
    //     j = 0;
    //     while (j < 200) {
    //         if (fwd) {
    //             executeSubcommand(0b11000, xMotor, 1000);
    //             executeSubcommand(0b11000, yMotor, 1000);
    //         } else {
    //             executeSubcommand(0b10000, xMotor, 1000);
    //             executeSubcommand(0b10000, yMotor, 1000);
    //         }
    //         j++;
    //     }
    //     fwd = !fwd;
    //     i++;
    // }
}