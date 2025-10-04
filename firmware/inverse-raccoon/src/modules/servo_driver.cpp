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

/**
 * @brief Write servo position with verification
 * 
 * 
 * Probably not gonna use this, I know we don't have encoders
 */
static bool servo_write_verified(Servo& servo, uint16_t position) {
  const uint8_t MAX_RETRIES = 3;
  const uint16_t TOLERANCE = 2;
  
  for (uint8_t attempt = 0; attempt < MAX_RETRIES; attempt++) {
    servo.write(position);
    delay(20);  // Reduced from 50ms - servos usually settle in 20ms
    
    uint16_t readback = servo.read();
    
    if (abs((int16_t)readback - (int16_t)position) <= TOLERANCE) {
      return true;
    }
  }
  
  g_error_count++;
  return false;
}

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
  
  // Validate range
  if (cmd->azimuth < MIN_AZIMUTH_DEG || cmd->azimuth > MAX_AZIMUTH_DEG) {
    g_error_count++;
    return false;
  }
  if (cmd->elevation < MIN_ELEVATION_DEG || cmd->elevation > MAX_ELEVATION_DEG) {
    g_error_count++;
    return false;
  }
  
  // Execute with verification, no encoder tho
  // bool az_ok = servo_write_verified(g_servo_azimuth, cmd->azimuth);
  // bool el_ok = servo_write_verified(g_servo_elevation, cmd->elevation);

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