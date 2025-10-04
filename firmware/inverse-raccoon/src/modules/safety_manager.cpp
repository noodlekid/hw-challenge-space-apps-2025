/**
 * @file safety_manager.cpp
 * @brief Safety monitoring and fault management implementation
 */

#include "modules/safety_manager.h"
#include "modules/sensor_manager.h"
#include "modules/servo_driver.h"
#include "config.h"
#include "utils/tmr.h"
#include <Arduino.h>

// Module state
static TMR<SystemMode_t> g_system_mode;
static uint16_t g_error_counts[ERR_COUNT];

void safety_manager_init() {
  g_system_mode.write(MODE_NORMAL);
  memset(g_error_counts, 0, sizeof(g_error_counts));
}

void safety_evaluate_mode() {
  SystemMode_t new_mode = MODE_NORMAL;
  
  uint16_t sensor_errors = sensor_get_error_count();
  uint16_t servo_errors = servo_get_error_count();
  
  // Check for critical errors
  if (g_error_counts[ERR_CONFIG_LOST] > 0 || 
      g_error_counts[ERR_CONTROL_FLOW] > MAX_ERROR_COUNT) {
    new_mode = MODE_EMERGENCY;
  }
  else if (servo_errors >= 1) {
    new_mode = MODE_DEGRADED_2;
  }
  else if (sensor_errors >= 1) {
    new_mode = MODE_DEGRADED_1;
  }
  else if (g_error_counts[ERR_MEMORY_CORRUPTION] > MAX_ERROR_COUNT) {
    new_mode = MODE_SAFE;
  }
  
  // Update mode with TMR
  g_system_mode.write(new_mode);
}

void safety_scrub_memory() {
  // Validate TMR variables
  if (!g_system_mode.validate()) {
    Serial.println(F("[SAFETY] TMR corruption in system_mode"));
    g_error_counts[ERR_MEMORY_CORRUPTION]++;
    g_system_mode.write(MODE_SAFE);
  }
}

bool safety_verify_control_flow(uint16_t signature) {
  if (signature != SIG_EXPECTED) {
    Serial.println(F("[CRITICAL] Control flow corruption!"));
    g_error_counts[ERR_CONTROL_FLOW]++;
    g_system_mode.write(MODE_SAFE);
    return false;
  }
  return true;
}

SystemMode_t safety_get_mode() {
  return g_system_mode.vote();
}

void safety_set_mode(SystemMode_t mode) {
  g_system_mode.write(mode);
}

void safety_log_error(ErrorCode_t error) {
  if (error >= 0 && error < ERR_COUNT) {
    g_error_counts[error]++;
  }
}

uint16_t safety_get_error_count(ErrorCode_t error) {
  if (error >= 0 && error < ERR_COUNT) {
    return g_error_counts[error];
  }
  return 0;
}

uint32_t safety_get_total_errors() {
  uint32_t total = 0;
  for (uint8_t i = 0; i < ERR_COUNT; i++) {
    total += g_error_counts[i];
  }
  return total;
}