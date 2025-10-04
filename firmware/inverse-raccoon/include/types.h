/**
 * @file types.h
 * @brief Type definitions and data structures
 */

#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

// ============================================================================
// SYSTEM TYPES
// ============================================================================

/**
 * @brief System operating modes
 */
typedef enum {
  MODE_NORMAL = 0,      // All systems operational
  MODE_DEGRADED_1,      // 3/4 sensors working
  MODE_DEGRADED_2,      // 2/3 servos working
  MODE_SAFE,            // Return to default position
  MODE_EMERGENCY        // Critical failure
} SystemMode_t;

/**
 * @brief Error codes
 */
typedef enum {
  ERR_NONE = 0,
  ERR_SENSOR_FAULT,
  ERR_SERVO_FAULT,
  ERR_MEMORY_CORRUPTION,
  ERR_CONTROL_FLOW,
  ERR_PRIMARY_CONFIG_CORRUPT,
  ERR_CONFIG_LOST,
  ERR_WATCHDOG_RESET,
  ERR_COUNT  // Must be last
} ErrorCode_t;

/**
 * @brief Sensor reading structure
 */
typedef struct {
  uint16_t top_left;
  uint16_t top_right;
  uint16_t bottom_left;
  uint16_t bottom_right;
  uint32_t timestamp;
  bool valid;
} SensorReading_t;

/**
 * @brief Sun position error vector
 */
typedef struct {
  float azimuth_error;
  float elevation_error;
  bool sun_detected;
} SunPosition_t;

/**
 * @brief Servo command with CRC
 */
typedef struct __attribute__((packed)) {
  uint16_t azimuth;
  uint16_t elevation;
  uint16_t crc16;
} ServoCommand_t;

/**
 * @brief Configuration structure with ECC
 */
typedef struct __attribute__((packed)) {
  uint16_t magic;
  uint16_t version;
  uint16_t servo_azimuth_offset;
  uint16_t servo_elevation_offset;
  uint16_t error_counts[ERR_COUNT];
  uint32_t boot_count;
  uint16_t crc16;
} Config_t;

#endif // TYPES_H