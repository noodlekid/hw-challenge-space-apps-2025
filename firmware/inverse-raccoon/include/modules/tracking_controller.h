/**
 * @file tracking_controller.h
 * @brief Proportional tracking controller with dead-band
 */

#ifndef TRACKING_CONTROLLER_H
#define TRACKING_CONTROLLER_H

#include "types.h"

/**
 * @brief Initialize tracking controller
 */
void tracking_controller_init();

/**
 * @brief Calculate servo command from sun position
 * @param position Input sun position error
 * @param cmd Output servo command with CRC
 */
void tracking_calculate_command(const SunPosition_t* position, ServoCommand_t* cmd);

/**
 * @brief Update last sun detection time
 * @param timestamp Time in milliseconds
 */
void tracking_update_sun_time(uint32_t timestamp);

/**
 * @brief Check if sun has been lost for too long
 * @return true if sun is lost and timeout exceeded
 */
bool tracking_is_sun_lost();

#endif // TRACKING_CONTROLLER_H