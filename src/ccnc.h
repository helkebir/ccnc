/**
 * @file ccnc.h
 *
 * Main CCNC functions.
 */
#include<gpiod.h>

/** @page deprecated Deprecated objects
 *  @brief Deprecated objects.
 *
 *  These objects risk deletion in later releases. Use at your own discretion.
 */

/**
 * Character buffer size for stdin input.
 */
#define BUFFER_SIZE 5

/**
 * @brief Number of subcommands in a command.
 * 
 * @details Normally equals number of controlled motors.
 */
#define COMMAND_LENGTH 3

/** Shorthand for unsigned integer. */
typedef unsigned int uint;

/**
 * @brief A struct containing motor pin numbers.
 */
struct MotorPins {
    uint stp; /**< STP pin */
    uint dir; /**< DIR pin */
    uint ms1; /**< MS1 pin */
    uint ms2; /**< MS2 pin */
    uint ms3; /**< MS3 pin */
};

/**
 * @brief A struct containing motor GPIOD line pointers and state.
 */
struct MotorGPIOD {
    struct gpiod_line *stp; /**< STP gpiod_line */
    struct gpiod_line *dir; /**< DIR gpiod_line */
    struct gpiod_line *ms1; /**< MS1 gpiod_line */
    struct gpiod_line *ms2; /**< MS2 gpiod_line */
    struct gpiod_line *ms3; /**< MS3 gpiod_line */
    bool stpState; /**< STP state */
    bool dirState; /**< DIR state */
    bool ms1State; /**< MS1 state */
    bool ms2State; /**< MS2 state */
    bool ms3State; /**< MS3 state */
};

/**
 * @brief Initializes a A4988-style motor driver.
 * 
 * @details Calls GPIOD and request pin output access as the "cnccontroller"
 * consumer. Initial pin values are set to low.
 * 
 * @param motorPins Motor driver GPIO pins.
 * @param motorGPIOD Uninitialized pointer to a MotorGPIOD instance that is to
 *                   be populated.
 * @param chip Pointer to the GPIOD chip.
 */
void initMotor(struct MotorPins *motorPins, struct MotorGPIOD *motorGPIOD,
    struct gpiod_chip *chip);

/**
 * @brief Safety releases all GPIOD connections for a given motor.
 * 
 * @param motorGPIOD Pointer to the MotorGPIOD instance to be released.
 */ 
void releaseMotor(struct MotorGPIOD *motorGPIOD);

/**
 * @brief Prepares pin states for a subcommand.
 * 
 * @details A subcommand is a 5-bit value with the following build-up:
 * 
 * | Address (LE) | Abbreviation | Meaning     |
 * |:-------------|:-------------|:------------|
 * | 0            | MS3          | Microstep 3 |
 * | 1            | MS2          | Microstep 2 |
 * | 2            | MS1          | Microstep 1 |
 * | 3            | DIR          | Direction   |
 * | 4            | ENA          | Enable      |
 * 
 * @param scmd A 5-bit subcommand.
 * @param motorGPIOD Pointer to the MotorGPIOD instance.
 */ 
void prepareSubcommand(uint scmd, struct MotorGPIOD *motorGPIOD);

/**
 * @brief Executes a subcommand with a specified wait time between step pin
 * high and low.
 * 
 * @see prepareSubcommand
 * 
 * @param scmd A 5-bit subcommand.
 * @param motorGPIOD Pointer to the MotorGPIOD instance.
 * @param wait Wait time in microseconds.
 */ 
void executeSubcommand(uint scmd, struct MotorGPIOD *motorGPIOD, uint wait);

/**
 * @brief Executes multiple subcommands on multiple motors (one per motor).
 * 
 * @see executeSubcommand
 * 
 * @param scmds An array of 5-bit subcommands.
 * @param motorGPIODs An array of pointers to MotorGPIOD instances.
 * @param wait Wait time in microseconds.
 * @param number Number of motors to be commanded.
 */
void executeSubcommandsOnMotors(uint scmds[], struct MotorGPIOD *motorGPIODs[],
    uint wait, uint number);

/**
 * @deprecated Superceded by executeSubcommand().
 * @brief Execute a subcommand by manually supplying GPIOD lines.
 * 
 * @param scmd A 5-bit subcommand.
 * @param wait Wait time in microseconds.
 * @param stp Pointer to the STP gpiod_line.
 * @param dir Pointer to the DIR gpiod_line.
 * @param ms1 Pointer to the MS1 gpiod_line.
 * @param ms2 Pointer to the MS2 gpiod_line.
 * @param ms3 Pointer to the MS3 gpiod_line.
 */
void runScmd(uint scmd, uint wait, struct gpiod_line *stp,
    struct gpiod_line *dir, struct gpiod_line *ms1, struct gpiod_line *ms2,
    struct gpiod_line *ms3);

/**
 * @deprecated Superceded by executeSubcommandsOnMotors().
 * @brief Execute a subcommand from an array by manually supplying GPIOD lines.
 * 
 * @param scmd An array of five integers.
 * @param wait Wait time in microseconds.
 * @param stp Pointer to the STP gpiod_line.
 * @param dir Pointer to the DIR gpiod_line.
 * @param ms1 Pointer to the MS1 gpiod_line.
 * @param ms2 Pointer to the MS2 gpiod_line.
 * @param ms3 Pointer to the MS3 gpiod_line.
 */
void runScmdFromArray(uint scmd[5], uint wait, struct gpiod_line *stp,
    struct gpiod_line *dir, struct gpiod_line *ms1, struct gpiod_line *ms2,
    struct gpiod_line *ms3);

/**
 * @brief Computes integer parity.
 * 
 * @details If even returns 1, 0 otherwise.
 * 
 * @param i Integer.
 * 
 * @return Parity.
 */
uint parity(uint i);

/**
 * @brief Prepends parity bit to an integer.
 * 
 * @details Parity bit is added at address 0 (LE).
 * 
 * @param i Integer.
 * 
 * @return Integer with prepended parity bit.
 */
uint addParityBit(uint i);

/**
 * @brief Checks if parity bit is properly set.
 * 
 * @param i Integer to be checked.
 * @param parityBit Supposed parity bit.
 * 
 * @return True if parity matches parity bit, false otherwise.
 */
bool checkParity(uint i, uint parityBit);

/**
 * @brief Concatenates two subcommands.
 * 
 * @param scmd0 First subcommand.
 * @param scmd1 Second subcommand.
 * 
 * @return Concatenated subcommands.
 */
uint concatenateSubcommands(uint scmd0, uint scmd1);

/**
 * @brief Deconstructs command
 * 
 * @param cmd Command to be deconstructed.
 * 
 * @return Array of length ::COMMAND_LENGTH containing all consituent
 *  subcommands.
 */
uint* deconstructCommand(uint cmd);

/**
 * @brief A small motor demo.
 * 
 * @param xMotor A pointer to the x-axis MotorGPIOD instance.
 * @param yMotor A pointer to the y-axis MotorGPIOD instance.
 * @param zMotor A pointer to the z-axis MotorGPIOD instance.
 */
void demoMotors(struct MotorGPIOD *xMotor, struct MotorGPIOD *yMotor,
    struct MotorGPIOD *zMotor);