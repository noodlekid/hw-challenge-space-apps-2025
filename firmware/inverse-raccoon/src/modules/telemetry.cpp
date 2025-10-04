/**
 * @file telemetry.cpp
 * @brief Telemetry and diagnostics implementation with JSON output
 */

#include "modules/telemetry.h"
#include "modules/safety_manager.h"
#include "modules/sensor_manager.h"
#include "modules/servo_driver.h"
#include "config.h"
#include <Arduino.h>

// Module state
static bool g_led_state = false;
static uint32_t g_telemetry_counter = 0;

void telemetry_init() {
  pinMode(LED_HEARTBEAT_PIN, OUTPUT);
  Serial.begin(115200);  // Increased baud rate for faster data
  
  Serial.println(F("\n\n================================="));
  Serial.println(" ________  ___   __    __   __   ______   ______    ______   ______       ______    ________   ______   ______   ______   ______   ___   __      ");
  Serial.println("/_______/\\/__/\\ /__/\\ /_/\\ /_/\\ /_____/\\ /_____/\\  /_____/\\ /_____/\\     /_____/\\  /_______/\\ /_____/\\ /_____/\\ /_____/\\ /_____/\\ /__/\\ /__/\\    ");
  Serial.println("\\__.::._\\/\\::\\_\\\\  \\ \\\\:\\ \\\\ \\ \\\\::::_\\/_\\:::_ \\ \\ \\::::_\\/_\\::::_\\/_    \\:::_ \\ \\ \\::: _  \\ \\\\:::__\\/ \\:::__\\/ \\:::_ \\ \\\\:::_ \\ \\\\::\\_\\\\  \\ \\   ");
  Serial.println("   \\::\\ \\  \\:. `-\\  \\ \\\\:\\ \\\\ \\ \\\\:\\/___/\\\\:(_) ) )_\\:\\/___/\\\\:\\/___/\\    \\:(_) ) )_\\::(_)  \\ \\\\:\\ \\  __\\:\\ \\  __\\:\\ \\ \\ \\\\:\\ \\ \\ \\\\:. `-\\  \\ \\  ");
  Serial.println("   _\\::\\ \\__\\:. _    \\ \\\\:\\_/.:\\ \\\\::___\\/_\\: __ `\\ \\\\_::._\\:\\\\::___\\/_    \\: __ `\\ \\\\:: __  \\ \\\\:\\ \\/_/\\\\:\\ \\/_/\\\\:\\ \\ \\ \\\\:\\ \\ \\ \\\\:. _    \\ \\ ");
  Serial.println("  /__\\::\\__/\\\\. \\`-\\  \\ \\\\ ..::/ / \\:\\____/\\\\ \\ `\\ \\ \\ /____\\:\\\\:\\____/\\    \\ \\ `\\ \\ \\\\:.\\ \\  \\ \\\\:\\_\\ \\ \\\\:\\_\\ \\ \\\\:\\_\\ \\ \\\\:\\_\\ \\ \\\\. \\`-\\  \\ \\");
  Serial.println("  \\________\\/ \\__\\/ \\__\\/ \\___/_(   \\_____\\/ \\_\\/ \\_\\/ \\_____\\/ \\_____\\/     \\_\\/ \\_\\/ \\__\\/\\__\\/ \\_____\\/ \\_____\\/ \\_____\\/ \\_____\\/ \\__\\/ \\__\\/");
  Serial.println("");
  Serial.println(F("Space.Apps.Ottawa 2025"));
  Serial.println(F("================================="));
}

void telemetry_print_status() {
  Serial.println(F("\n--- SYSTEM STATUS ---"));
  Serial.print(F("Mode: "));
  
  switch (safety_get_mode()) {
    case MODE_NORMAL: Serial.println(F("NORMAL")); break;
    case MODE_DEGRADED_1: Serial.println(F("DEGRADED_1")); break;
    case MODE_DEGRADED_2: Serial.println(F("DEGRADED_2")); break;
    case MODE_SAFE: Serial.println(F("SAFE")); break;
    case MODE_EMERGENCY: Serial.println(F("EMERGENCY")); break;
  }
  
  Serial.print(F("Uptime: "));
  Serial.print(millis() / 1000);
  Serial.println(F(" sec"));
  
  // Print error counts
  uint32_t total_errors = safety_get_total_errors();
  if (total_errors > 0) {
    Serial.println(F("Errors:"));
    for (uint8_t i = 0; i < ERR_COUNT; i++) {
      uint16_t count = safety_get_error_count((ErrorCode_t)i);
      if (count > 0) {
        Serial.print(F("  ERR"));
        Serial.print(i);
        Serial.print(F(": "));
        Serial.println(count);
      }
    }
  } else {
    Serial.println(F("No errors detected"));
  }
}

void telemetry_print_sensors(const SensorReading_t* reading) {
  Serial.print(F("Sensors: TL="));
  Serial.print(reading->top_left);
  Serial.print(F(" TR="));
  Serial.print(reading->top_right);
  Serial.print(F(" BL="));
  Serial.print(reading->bottom_left);
  Serial.print(F(" BR="));
  Serial.print(reading->bottom_right);
  Serial.print(F(" ["));
  Serial.print(reading->valid ? F("VALID") : F("FAULT"));
  Serial.println(F("]"));
}

void telemetry_print_servos(const ServoCommand_t* cmd) {
  Serial.print(F("Position: Az="));
  Serial.print(cmd->azimuth);
  Serial.print(F("° El="));
  Serial.print(cmd->elevation);
  Serial.println(F("°"));
}

void telemetry_update_heartbeat() {
  g_led_state = !g_led_state;
  digitalWrite(LED_HEARTBEAT_PIN, g_led_state);
}

/**
 * @brief Print complete system state as JSON
 */
void telemetry_print_json(const SensorReading_t* sensor_data, 
                          const ServoCommand_t* servo_cmd) {
  const SunPosition_t* sun_pos = sensor_get_position();
  SystemMode_t mode = safety_get_mode();
  
  Serial.print(F("{"));
  
  // Metadata
  Serial.print(F("\"seq\":"));
  Serial.print(g_telemetry_counter++);
  Serial.print(F(",\"uptime\":"));
  Serial.print(millis() / 1000);
  Serial.print(F(",\"mode\":\""));
  switch (mode) {
    case MODE_NORMAL: Serial.print(F("NORMAL")); break;
    case MODE_DEGRADED_1: Serial.print(F("DEGRADED_1")); break;
    case MODE_DEGRADED_2: Serial.print(F("DEGRADED_2")); break;
    case MODE_SAFE: Serial.print(F("SAFE")); break;
    case MODE_EMERGENCY: Serial.print(F("EMERGENCY")); break;
  }
  Serial.print(F("\""));
  
  // Sensors
  Serial.print(F(",\"sensors\":{"));
  Serial.print(F("\"tl\":"));
  Serial.print(sensor_data->top_left);
  Serial.print(F(",\"tr\":"));
  Serial.print(sensor_data->top_right);
  Serial.print(F(",\"bl\":"));
  Serial.print(sensor_data->bottom_left);
  Serial.print(F(",\"br\":"));
  Serial.print(sensor_data->bottom_right);
  Serial.print(F(",\"valid\":"));
  Serial.print(sensor_data->valid ? F("true") : F("false"));
  Serial.print(F("}"));
  
  // Sun position
  Serial.print(F(",\"sun\":{"));
  Serial.print(F("\"detected\":"));
  Serial.print(sun_pos->sun_detected ? F("true") : F("false"));
  Serial.print(F(",\"az_error\":"));
  Serial.print(sun_pos->azimuth_error);
  Serial.print(F(",\"el_error\":"));
  Serial.print(sun_pos->elevation_error);
  Serial.print(F("}"));
  
  // Servos
  Serial.print(F(",\"servos\":{"));
  Serial.print(F("\"az\":"));
  Serial.print(servo_cmd->azimuth);
  Serial.print(F(",\"el\":"));
  Serial.print(servo_cmd->elevation);
  Serial.print(F("}"));
  
  // Errors
  Serial.print(F(",\"errors\":{"));
  Serial.print(F("\"total\":"));
  Serial.print(safety_get_total_errors());
  Serial.print(F(",\"sensor\":"));
  Serial.print(sensor_get_error_count());
  Serial.print(F(",\"servo\":"));
  Serial.print(servo_get_error_count());
  Serial.print(F("}"));
  
  Serial.println(F("}"));
}