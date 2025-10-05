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
static bool g_elevation_inverted = false;

static TMR<uint32_t> g_last_sun_detect_time;

void tracking_controller_init() {
  g_current_azimuth = DEFAULT_AZIMUTH_DEG;
  g_current_elevation = DEFAULT_ELEVATION_DEG;
  g_last_sun_detect_time.write(millis());
}

// Add at top with other static variables

void tracking_calculate_command(const SunPosition_t* position, ServoCommand_t* cmd) {
  bool sun_lost = tracking_is_sun_lost();
  
  if (sun_lost || !position->sun_detected) {
    g_current_azimuth = DEFAULT_AZIMUTH_DEG;
    g_current_elevation = DEFAULT_ELEVATION_DEG;
    g_elevation_inverted = false;
  } else {
    float azimuth_error = position->azimuth_error;
    float elevation_error = position->elevation_error;
    

    if (g_current_elevation > 100.0f) {
      g_elevation_inverted = true;
    } else if (g_current_elevation < 80.0f) {
      g_elevation_inverted = false;
    }
    
    if (g_elevation_inverted) {
      azimuth_error = -azimuth_error;
    }
    
    // Apply azimuth control
    if (abs(azimuth_error) > DEADBAND_DEGREES) {
      float correction = azimuth_error * PROPORTIONAL_GAIN;
      g_current_azimuth += correction;
      g_current_azimuth = constrain(g_current_azimuth, MIN_AZIMUTH_DEG, MAX_AZIMUTH_DEG);
    }
    
    // Apply elevation control  
    if (abs(elevation_error) > DEADBAND_DEGREES) {
      float correction = elevation_error * PROPORTIONAL_GAIN;
      g_current_elevation += correction;
      g_current_elevation = constrain(g_current_elevation, MIN_ELEVATION_DEG, MAX_ELEVATION_DEG);
    }
  }
  
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