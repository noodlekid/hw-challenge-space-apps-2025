/**
 * @file telemetry.h
 * @brief Telemetry and diagnostics interface
 */

#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "types.h"

void telemetry_init();
void telemetry_print_status();
void telemetry_print_sensors(const SensorReading_t* reading);
void telemetry_print_servos(const ServoCommand_t* cmd);
void telemetry_update_heartbeat();
void telemetry_print_json(const SensorReading_t* sensor_data, 
                          const ServoCommand_t* servo_cmd);

#endif // TELEMETRY_H