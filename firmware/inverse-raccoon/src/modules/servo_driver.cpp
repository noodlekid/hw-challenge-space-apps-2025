/**
 * @file servo_driver.cpp
 * @brief Servo control implementation
 */

#include "modules/servo_driver.h"
#include "config.h"
#include "utils/crc.h"
#include <Servo.h>

// Module state
static Servo g_servo_azimuth;
static Servo g_servo_elevation;
static uint16_t g_error_count = 0;

void servo_driver_init() {
  g_servo_azimuth.attach(SERVO_AZIMUTH_PIN);
  g_servo_elevation.attach(SERVO_ELEVATION_PIN);
  
  // Move to default position
  g_servo_azimuth.write(DEFAULT_AZIMUTH_DEG);
  g_servo_elevation.write(DEFAULT_ELEVATION_DEG);
  
  g_error_count = 0;
  
  Serial.println(F("[SERVO] Initialized"));
}

bool servo_execute_command(const ServoCommand_t* cmd) {
  // Validate CRC
  uint16_t computed_crc = crc16(cmd, offsetof(ServoCommand_t, crc16));
  if (computed_crc != cmd->crc16) {
    Serial.println(F("[SERVO] CRC mismatch"));
    g_error_count++;
    return false;
  }
  
  // Validate range - use physical servo limits
  if (cmd->azimuth < SERVO_MIN_DEG || cmd->azimuth > SERVO_MAX_DEG) {
    Serial.print(F("[SERVO] Azimuth out of range: "));
    Serial.println(cmd->azimuth);
    g_error_count++;
    return false;
  }
  if (cmd->elevation < SERVO_MIN_DEG || cmd->elevation > SERVO_MAX_DEG) {
    Serial.print(F("[SERVO] Elevation out of range: "));
    Serial.println(cmd->elevation);
    g_error_count++;
    return false;
  }
  
  // Execute command
  g_servo_azimuth.write(cmd->azimuth);
  g_servo_elevation.write(cmd->elevation);
  
  return true;
}

uint16_t servo_get_error_count() {
  return g_error_count;
}

void servo_reset_error_count() {
  g_error_count = 0;
}