/**
 * @file tracking_controller.cpp
 * @brief Proportional tracking controller implementation
 */

#include "modules/tracking_controller.h"
#include "config.h"
#include "utils/crc.h"
#include "utils/tmr.h"
#include <Arduino.h>

// Module state
static float g_current_azimuth = DEFAULT_AZIMUTH_DEG;
static float g_current_elevation = DEFAULT_ELEVATION_DEG;
static TMR<uint32_t> g_last_sun_detect_time;

void tracking_controller_init() {
  g_current_azimuth = DEFAULT_AZIMUTH_DEG;
  g_current_elevation = DEFAULT_ELEVATION_DEG;
  g_last_sun_detect_time.write(millis());
}

void tracking_calculate_command(const SunPosition_t* position, ServoCommand_t* cmd) {
  // Check for sun loss timeout
  bool sun_lost = tracking_is_sun_lost();
  
  if (sun_lost || !position->sun_detected) {
    // Return to default position
    g_current_azimuth = DEFAULT_AZIMUTH_DEG;
    g_current_elevation = DEFAULT_ELEVATION_DEG;
  } else {
    // Apply proportional control with dead-band
    if (abs(position->azimuth_error) > DEADBAND_DEGREES) {
      g_current_azimuth += position->azimuth_error * PROPORTIONAL_GAIN;
    }
    
    if (abs(position->elevation_error) > DEADBAND_DEGREES) {
      g_current_elevation += position->elevation_error * PROPORTIONAL_GAIN;
    }
    
    // Enforce position limits
    g_current_azimuth = constrain(g_current_azimuth, MIN_AZIMUTH_DEG, MAX_AZIMUTH_DEG);
    g_current_elevation = constrain(g_current_elevation, MIN_ELEVATION_DEG, MAX_ELEVATION_DEG);
  }
  
  // Generate command
  cmd->azimuth = (uint16_t)g_current_azimuth;
  cmd->elevation = (uint16_t)g_current_elevation;
  cmd->crc16 = crc16(cmd, offsetof(ServoCommand_t, crc16));
}

void tracking_update_sun_time(uint32_t timestamp) {
  g_last_sun_detect_time.write(timestamp);
}

bool tracking_is_sun_lost() {
  uint32_t time_since_sun = millis() - g_last_sun_detect_time.vote();
  return (time_since_sun > SUN_LOSS_TIMEOUT_MS);
}