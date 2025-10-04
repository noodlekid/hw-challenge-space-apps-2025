/**
 * @file sensor_manager.h
 * @brief Sensor reading and sun position calculation
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "types.h"

/**
 * @brief Initialize sensor manager
 */
void sensor_manager_init();

/**
 * @brief Read all sensors with median filtering and fault detection
 * @param reading Output structure for sensor readings
 * @return true if readings are valid, false otherwise
 */
bool sensor_read_all(SensorReading_t* reading);

/**
 * @brief Calculate sun position from sensor readings
 * @param reading Input sensor readings
 * @param position Output sun position structure
 */
void sensor_calculate_position(const SensorReading_t* reading, SunPosition_t* position);

/**
 * @brief Get current sun position (cached from last calculation)
 * @return Pointer to current sun position
 */
const SunPosition_t* sensor_get_position();

/**
 * @brief Get error count for sensor faults
 * @return Number of sensor faults detected
 */
uint16_t sensor_get_error_count();

void sensor_reset_error_count();

#endif // SENSOR_MANAGER_H