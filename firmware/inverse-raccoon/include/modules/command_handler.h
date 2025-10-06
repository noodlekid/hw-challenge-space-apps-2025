/**
 * @file command_handler.h
 * @brief Serial command interface for manual servo control
 */

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "types.h"

/**
 * @brief Control modes
 */
typedef enum {
  CONTROL_AUTO = 0,    // Normal sun tracking
  CONTROL_MANUAL,      // Manual servo positioning
  CONTROL_DEMO         // Ephemeris demo mode
} ControlMode_t;


/**
 * @brief Get demo mode start time
 */
uint32_t command_get_demo_start_time(void);
/**
 * @brief Initialize command handler
 */
void command_handler_init(void);

/**
 * @brief Process incoming serial commands (non-blocking)
 * 
 * Call this regularly in your main loop. It checks for incoming
 * serial data and processes commands when a complete line is received.
 */
void command_handler_process(void);

/**
 * @brief Get current control mode
 */
ControlMode_t command_get_mode(void);

/**
 * @brief Check if there's a pending manual command
 */
bool command_has_pending(void);

/**
 * @brief Get the pending manual command and clear it
 */
void command_get_pending(ServoCommand_t* cmd);

#endif // COMMAND_HANDLER_H