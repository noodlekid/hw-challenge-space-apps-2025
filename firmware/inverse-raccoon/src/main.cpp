/**
 * @file main.cpp
 * @brief Main control loop and system orchestration
 */

//     ________  ___   __    __   __   ______   ______    ______   ______       ______    ________   ______   ______   ______   ______   ___   __      
//    /_______/\/__/\ /__/\ /_/\ /_/\ /_____/\ /_____/\  /_____/\ /_____/\     /_____/\  /_______/\ /_____/\ /_____/\ /_____/\ /_____/\ /__/\ /__/\    
//    \__.::._\/\::\_\\  \ \\:\ \\ \ \\::::_\/_\:::_ \ \ \::::_\/_\::::_\/_    \:::_ \ \ \::: _  \ \\:::__\/ \:::__\/ \:::_ \ \\:::_ \ \\::\_\\  \ \   
//       \::\ \  \:. `-\  \ \\:\ \\ \ \\:\/___/\\:(_) ) )_\:\/___/\\:\/___/\    \:(_) ) )_\::(_)  \ \\:\ \  __\:\ \  __\:\ \ \ \\:\ \ \ \\:. `-\  \ \  
//       _\::\ \__\:. _    \ \\:\_/.:\ \\::___\/_\: __ `\ \\_::._\:\\::___\/_    \: __ `\ \\:: __  \ \\:\ \/_/\\:\ \/_/\\:\ \ \ \\:\ \ \ \\:. _    \ \ 
//      /__\::\__/\\. \`-\  \ \\ ..::/ / \:\____/\\ \ `\ \ \ /____\:\\:\____/\    \ \ `\ \ \\:.\ \  \ \\:\_\ \ \\:\_\ \ \\:\_\ \ \\:\_\ \ \\. \`-\  \ \
//      \________\/ \__\/ \__\/ \___/_(   \_____\/ \_\/ \_\/ \_____\/ \_____\/     \_\/ \_\/ \__\/\__\/ \_____\/ \_____\/ \_____\/ \_____\/ \__\/ \__\/
//                                                                                                                                                     

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

// Global timing variables
static uint32_t g_loop_start_time;
static uint32_t g_last_scrub_time;
static uint32_t g_last_telemetry_time;
static uint32_t g_last_config_save_time;
static uint16_t g_flow_signature;

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
  
  // Initialize timing
  g_last_scrub_time = millis();
  g_last_telemetry_time = millis();
  g_last_config_save_time = millis();
  
  Serial.println(F("[INIT] System ready\n"));
  delay(1000);
}

void loop() {
  g_loop_start_time = millis();
  
  // Feed watchdog
  wdt_reset();
  
  // Control flow check initialization
  g_flow_signature = SIG_INIT;
  
  // ===== SENSOR READING =====
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
  
  // ===== TRACKING ALGORITHM =====
  ServoCommand_t servo_cmd;
  tracking_calculate_command(&sun_position, &servo_cmd);
  
  g_flow_signature ^= SIG_TRACKING;
  
  // ===== SERVO CONTROL =====
  if (safety_get_mode() != MODE_EMERGENCY) {
    servo_execute_command(&servo_cmd);
  }
  
  g_flow_signature ^= SIG_SERVO;
  

  // check control flow integrity
  safety_verify_control_flow(g_flow_signature);
  
  
  // memory scrubbing
  if (millis() - g_last_scrub_time >= SCRUB_INTERVAL_MS) {
    safety_scrub_memory();
    g_last_scrub_time = millis();
  }
  
  // Safety evaluation
  safety_evaluate_mode();
  
  // Heartbeat LED
  telemetry_update_heartbeat();
  
  // Telemetry output
  // Get rid of magic number
if (millis() - g_last_telemetry_time >= 1000) {
  telemetry_print_json(&sensor_data, &servo_cmd);
  
  g_last_telemetry_time = millis();
}
  
  // Periodic config save 
  // TODO: Get rid of magic number
  if (millis() - g_last_config_save_time >= 60000) {
    config_persist();
    Serial.println(F("[CONFIG] Persisted to EEPROM"));
    g_last_config_save_time = millis();
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


  // Error Counter Reset
  static uint32_t g_last_error_reset_time;


  // TOOD: Get rid of magic number
  if (millis() - g_last_error_reset_time >= 30000) {
  // Only reset if currently operating successfully
  if (safety_get_mode() == MODE_NORMAL) {
    sensor_reset_error_count();
    servo_reset_error_count();
    Serial.println(F("[SAFETY] Error counters cleared - recovery confirmed"));
  }
  g_last_error_reset_time = millis();
}
}