/**
 * @file command_handler.cpp
 * @brief Serial command interface implementation
 */

#include "modules/command_handler.h"
#include "config.h"
#include "utils/crc.h"
#include <Arduino.h>
#include <string.h>

// Command buffer
static char g_cmd_buffer[CMD_BUFFER_SIZE];
static uint8_t g_cmd_index = 0;

// Module state
static ControlMode_t g_control_mode = CONTROL_AUTO;
static ServoCommand_t g_pending_command;
static bool g_has_pending = false;
static uint32_t g_demo_start_time = 0;
static const uint32_t DEMO_DURATION_MS = 45000;

/**
 * @brief Parse and execute a command string
 */
static void command_parse(const char* cmd) {
  // Input validation
  if (cmd == NULL) {
    return;
  }
  
  // Skip leading whitespace
  while (*cmd == ' ') cmd++;
  
  // MANUAL <azimuth> <elevation>
  if (strncmp(cmd, "MANUAL", 6) == 0) {
    const char* args = cmd + 6;
    
    // Validate input length
    if (strlen(args) > CMD_MAX_ARG_LENGTH) {
      Serial.println(F("[CMD] Error: Arguments too long"));
      return;
    }
    
    int az, el;
    if (sscanf(args, "%d %d", &az, &el) == 2) {
      // Validate ranges
      if (az >= MIN_AZIMUTH_DEG && az <= MAX_AZIMUTH_DEG &&
          el >= MIN_ELEVATION_DEG && el <= MAX_ELEVATION_DEG) {
        
        g_control_mode = CONTROL_MANUAL;
        g_pending_command.azimuth = (uint16_t)az;
        g_pending_command.elevation = (uint16_t)el;
        g_pending_command.crc16 = crc16(&g_pending_command, 
                                        offsetof(ServoCommand_t, crc16));
        g_has_pending = true;
        
        Serial.print(F("[CMD] Manual mode - Az: "));
        Serial.print(az);
        Serial.print(F("° El: "));
        Serial.print(el);
        Serial.println(F("°"));
      } else {
        Serial.println(F("[CMD] Error: Position out of range"));
        Serial.print(F("  Valid: Az["));
        Serial.print(MIN_AZIMUTH_DEG);
        Serial.print(F("-"));
        Serial.print(MAX_AZIMUTH_DEG);
        Serial.print(F("] El["));
        Serial.print(MIN_ELEVATION_DEG);
        Serial.print(F("-"));
        Serial.print(MAX_ELEVATION_DEG);
        Serial.println(F("]"));
      }
    } else {
      Serial.println(F("[CMD] Usage: MANUAL <azimuth> <elevation>"));
    }
  }
  
  // AUTO
  else if (strncmp(cmd, "AUTO", 4) == 0) {
    g_control_mode = CONTROL_AUTO;
    g_has_pending = false;
    Serial.println(F("[CMD] Automatic tracking mode"));
  }
  
  // HOME
  else if (strncmp(cmd, "HOME", 4) == 0) {
    g_control_mode = CONTROL_MANUAL;
    g_pending_command.azimuth = DEFAULT_AZIMUTH_DEG;
    g_pending_command.elevation = DEFAULT_ELEVATION_DEG;
    g_pending_command.crc16 = crc16(&g_pending_command, 
                                    offsetof(ServoCommand_t, crc16));
    g_has_pending = true;
    Serial.println(F("[CMD] Moving to home position"));
  }
  
  // HELP
  else if (strncmp(cmd, "HELP", 4) == 0 || *cmd == '?') {
    Serial.println(F("\n=== Command Reference ==="));
    Serial.println(F("MANUAL <az> <el> - Move to position (e.g. MANUAL 90 60)"));
    Serial.println(F("AUTO             - Return to sun tracking mode"));
    Serial.println(F("HOME             - Move to default position"));
    Serial.println(F("HELP or ?        - Show this help"));
    Serial.print(F("\nValid ranges: Az["));
    Serial.print(MIN_AZIMUTH_DEG);
    Serial.print(F("-"));
    Serial.print(MAX_AZIMUTH_DEG);
    Serial.print(F("] El["));
    Serial.print(MIN_ELEVATION_DEG);
    Serial.print(F("-"));
    Serial.print(MAX_ELEVATION_DEG);
    Serial.println(F("]"));
  }

  else if (strncmp(cmd, "DEMO", 4) == 0) {
    g_control_mode = CONTROL_DEMO;
    g_demo_start_time = millis();
    g_has_pending = false;
    Serial.println(F("[CMD] Ephemeris demo mode - simulating sun arc"));
    Serial.println(F("  Sunrise (East) -> Noon (Peak) -> Sunset (West)"));
  }
  
  // Unknown command
  else if (*cmd != '\0') {
    Serial.print(F("[CMD] Unknown command: "));
    Serial.println(cmd);
    Serial.println(F("Type HELP for command list"));
  }
}

void command_handler_init(void) {
  g_cmd_index = 0;
  g_control_mode = CONTROL_AUTO;
  g_has_pending = false;
  memset(g_cmd_buffer, 0, CMD_BUFFER_SIZE);
  
  Serial.println(F("[CMD] Command interface ready (type HELP)"));
}

uint32_t command_get_demo_start_time(void) {
  return g_demo_start_time;
}

void command_handler_process(void) {
  // Non-blocking serial read
  while (Serial.available() > 0) {
    char c = Serial.read();
    
    // Handle newline (command complete)
    if (c == '\n' || c == '\r') {
      if (g_cmd_index > 0) {
        g_cmd_buffer[g_cmd_index] = '\0';
        command_parse(g_cmd_buffer);
        g_cmd_index = 0;
      }
    }
    // Add to buffer
    else if (g_cmd_index < CMD_BUFFER_SIZE - 1) {
      g_cmd_buffer[g_cmd_index++] = c;
    }
    // Buffer overflow protection
    else {
      Serial.println(F("[CMD] Error: Command too long"));
      g_cmd_index = 0;
    }
  }
}

ControlMode_t command_get_mode(void) {
  return g_control_mode;
}

bool command_has_pending(void) {
  return g_has_pending;
}

void command_get_pending(ServoCommand_t* cmd) {
  if (cmd == NULL) {
    return;
  }
  
  if (g_has_pending) {
    memcpy(cmd, &g_pending_command, sizeof(ServoCommand_t));
    g_has_pending = false;
  }
}