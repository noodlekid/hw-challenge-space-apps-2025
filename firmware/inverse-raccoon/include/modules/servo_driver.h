/**
 * @file servo_driver.h
 * @brief Servo control with write-verify
 */

#ifndef SERVO_DRIVER_H
#define SERVO_DRIVER_H

#include "types.h"

/**
 * @brief Initialize servo driver
 */
void servo_driver_init();

/**
 * @brief Execute servo command with verification
 * @param cmd Servo command structure with CRC
 * @return true if successful, false otherwise
 */
bool servo_execute_command(const ServoCommand_t* cmd);

/**
 * @brief Get error count for servo faults
 * @return Number of servo write failures
 */
uint16_t servo_get_error_count();

void servo_reset_error_count();

#endif // SERVO_DRIVER_H