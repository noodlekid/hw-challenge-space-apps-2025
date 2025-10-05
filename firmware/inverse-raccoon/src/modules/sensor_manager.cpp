/**
 * @file sensor_manager.cpp
 * @brief Sensor reading and sun position calculation implementation
 */

#include "modules/sensor_manager.h"
#include "config.h"
#include <Arduino.h>

// Module state
static SunPosition_t g_current_position;
static uint16_t g_error_count = 0;

/**
 * @brief Median of 3 values
 */
static uint16_t median3(uint16_t a, uint16_t b, uint16_t c) {
  if (a > b) {
    if (b > c) return b;
    else if (a > c) return c;
    else return a;
  } else {
    if (a > c) return a;
    else if (b > c) return c;
    else return b;
  }
}

/**
 * @brief Read single sensor with median filtering
 */
static uint16_t sensor_read_filtered(uint8_t pin) {
  uint16_t samples[SENSOR_SAMPLE_COUNT];
  
  for (uint8_t i = 0; i < SENSOR_SAMPLE_COUNT; i++) {
    samples[i] = analogRead(pin);
    delayMicroseconds(100);
  }
  
  return median3(samples[0], samples[1], samples[2]);
}

void sensor_manager_init() {
  // pinMode(SENSOR_PIN_TOPLEFT, INPUT_PULLUP);
  // pinMode(SENSOR_PIN_TOPRIGHT, INPUT_PULLUP);
  // pinMode(SENSOR_PIN_BOTTOMLEFT, INPUT_PULLUP);
  // pinMode(SENSOR_PIN_BOTTOMRIGHT, INPUT_PULLUP);

  g_current_position.azimuth_error = 0;
  g_current_position.elevation_error = 0;
  g_current_position.sun_detected = false;
  g_error_count = 0;
}

bool sensor_read_all(SensorReading_t* reading) {
  reading->timestamp = millis();
  
  reading->top_left = sensor_read_filtered(SENSOR_PIN_TOPLEFT);
  reading->top_right = sensor_read_filtered(SENSOR_PIN_TOPRIGHT);
  reading->bottom_left = sensor_read_filtered(SENSOR_PIN_BOTTOMLEFT);
  reading->bottom_right = sensor_read_filtered(SENSOR_PIN_BOTTOMRIGHT);
  
  // Validate sensor readings
  uint8_t fault_count = 0;
  
  if (reading->top_left < SENSOR_MIN_VALUE || reading->top_left > SENSOR_MAX_VALUE) fault_count++;
  if (reading->top_right < SENSOR_MIN_VALUE || reading->top_right > SENSOR_MAX_VALUE) fault_count++;
  if (reading->bottom_left < SENSOR_MIN_VALUE || reading->bottom_left > SENSOR_MAX_VALUE) fault_count++;
  if (reading->bottom_right < SENSOR_MIN_VALUE || reading->bottom_right > SENSOR_MAX_VALUE) fault_count++;
  
  reading->valid = (fault_count < 2);  // Allow operation with 1 faulty sensor
  
  if (!reading->valid) {
    g_error_count++;
  }
  
  return reading->valid;
}

void sensor_calculate_position(const SensorReading_t* reading, SunPosition_t* position) {
  // Calculate average light intensity
  uint32_t total = reading->top_left + reading->top_right + 
                   reading->bottom_left + reading->bottom_right;
  uint16_t average = total / 4;
  
  // Detect if sun is visible 
  // TODO: the threshold may need tuning, idk what ambient light levels are like
  position->sun_detected = (average > SUN_TRESHOLD);
  
  if (!position->sun_detected) {
    position->azimuth_error = 0;
    position->elevation_error = 0;
    return;
  }
  
  // Calculate differential errors
  int16_t horizontal_diff = (reading->top_right + reading->bottom_right) - 
                            (reading->top_left + reading->bottom_left);
  int16_t vertical_diff = (reading->top_left + reading->top_right) - 
                          (reading->bottom_left + reading->bottom_right);
  
  // Normalize to degree,
  // TODO: the scaling factor may need tuning
  position->azimuth_error =  horizontal_diff / 10.0f;
  position->elevation_error = vertical_diff / 10.0f;
  
  // Cache current position
  g_current_position = *position;
}

const SunPosition_t* sensor_get_position() {
  return &g_current_position;
}

uint16_t sensor_get_error_count() {
  return g_error_count;
}

void sensor_reset_error_count() {
  g_error_count = 0;
}