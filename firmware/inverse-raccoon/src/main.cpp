/**
 * @file main.cpp
 * @brief Main control loop with manual override capability
 */

#include <Arduino.h>
#include <avr/wdt.h>

#include "config.h"
#include "types.h"
#include "modules/config_manager.h"
#include "modules/sensor_manager.h"
#include "modules/tracking_controller.h"
#include "modules/servo_driver.h"
#include "modules/safety_manager.h"
#include "modules/telemetry.h"
#include "modules/command_handler.h"

// Global timing variables
static uint32_t g_loop_start_time;
static uint32_t g_last_scrub_time;
static uint32_t g_last_telemetry_time;
static uint32_t g_last_config_save_time;
static uint32_t g_last_error_reset_time;
static uint16_t g_flow_signature;

// Telemetry intervals
#define TELEMETRY_INTERVAL_MS  1000
#define CONFIG_SAVE_INTERVAL_MS 60000
#define ERROR_RESET_INTERVAL_MS 30000

void setup() {
  // Initialize telemetry first for debug output
  telemetry_init();
  
  // Initialize watchdog (8 second timeout)
  wdt_enable(WDTO_8S);
  
  // Safe boot and configuration
  config_manager_init();
  
  // Initialize all modules
  sensor_manager_init();
  tracking_controller_init();
  servo_driver_init();
  safety_manager_init();
  command_handler_init();
  
  // Initialize timing
  g_last_scrub_time = millis();
  g_last_telemetry_time = millis();
  g_last_config_save_time = millis();
  g_last_error_reset_time = millis();
  
  Serial.println(F("[INIT] System ready\n"));
  delay(1000);
}

void loop() {
  g_loop_start_time = millis();
  
  // Feed watchdog
  wdt_reset();
  
  // Process incoming commands
  command_handler_process();
  
  // Control flow check initialization
  g_flow_signature = SIG_INIT;
  
  // Declare command structure
  ServoCommand_t servo_cmd;
  
  // Check control mode
  ControlMode_t control_mode = command_get_mode();
  
  if (control_mode == CONTROL_MANUAL) {
    // ===== MANUAL MODE =====
    // Still read sensors for telemetry, but don't use for control
    SensorReading_t sensor_data;
    sensor_read_all(&sensor_data);
    
    g_flow_signature ^= SIG_SENSOR;
    g_flow_signature ^= SIG_TRACKING;
    
    // Check for pending manual command
    if (command_has_pending()) {
      command_get_pending(&servo_cmd);
      
      // Execute manual command
      if (safety_get_mode() != MODE_EMERGENCY) {
        servo_execute_command(&servo_cmd);
      }
    }
    // If no pending command, servos just hold their last position
    
    g_flow_signature ^= SIG_SERVO;
  } 
  else {
    // ===== AUTOMATIC MODE =====
    // Normal sun tracking operation
    SensorReading_t sensor_data;
    SunPosition_t sun_position;
    
    if (sensor_read_all(&sensor_data)) {
      sensor_calculate_position(&sensor_data, &sun_position);
      
      // Update sun detection time if sun is visible
      if (sun_position.sun_detected) {
        tracking_update_sun_time(millis());
      }
    }
    
    g_flow_signature ^= SIG_SENSOR;
    
    // Tracking algorithm
    tracking_calculate_command(&sun_position, &servo_cmd);
    
    g_flow_signature ^= SIG_TRACKING;
    
    // Servo control
    if (safety_get_mode() != MODE_EMERGENCY) {
      servo_execute_command(&servo_cmd);
    }
    
    g_flow_signature ^= SIG_SERVO;
  }
  
  // Check control flow integrity
  safety_verify_control_flow(g_flow_signature);
  
  // Memory scrubbing
  if (millis() - g_last_scrub_time >= SCRUB_INTERVAL_MS) {
    safety_scrub_memory();
    g_last_scrub_time = millis();
  }
  
  // Safety evaluation
  safety_evaluate_mode();
  
  // Heartbeat LED
  telemetry_update_heartbeat();
  
  // Telemetry output
  if (millis() - g_last_telemetry_time >= TELEMETRY_INTERVAL_MS) {
    // Read current sensor data for telemetry
    SensorReading_t sensor_data;
    sensor_read_all(&sensor_data);
    
    telemetry_print_json(&sensor_data, &servo_cmd);
    
    // Print control mode indicator
    if (control_mode == CONTROL_MANUAL) {
      Serial.println(F("[MODE] MANUAL"));
    }
    
    g_last_telemetry_time = millis();
  }
  
  // Periodic config save
  if (millis() - g_last_config_save_time >= CONFIG_SAVE_INTERVAL_MS) {
    config_persist();
    Serial.println(F("[CONFIG] Persisted to EEPROM"));
    g_last_config_save_time = millis();
  }
  
  // Error counter reset
  if (millis() - g_last_error_reset_time >= ERROR_RESET_INTERVAL_MS) {
    // Only reset if currently operating successfully
    if (safety_get_mode() == MODE_NORMAL) {
      sensor_reset_error_count();
      servo_reset_error_count();
      Serial.println(F("[SAFETY] Error counters cleared - recovery confirmed"));
    }
    g_last_error_reset_time = millis();
  }
  
  // Maintain control loop timing
  uint32_t elapsed = millis() - g_loop_start_time;
  if (elapsed < CONTROL_LOOP_PERIOD_MS) {
    delay(CONTROL_LOOP_PERIOD_MS - elapsed);
  } else {
    Serial.print(F("[WARNING] Control loop overrun: "));
    Serial.print(elapsed);
    Serial.println(F("ms"));
  }
}