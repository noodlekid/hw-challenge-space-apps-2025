/**
 * @file config.h
 * @brief System configuration and constants
 */

#ifndef CONFIG_H
#define CONFIG_H


// SYSTEM TIMING
#define CONTROL_LOOP_PERIOD_MS    100
#define WATCHDOG_TIMEOUT_MS       2000
#define SENSOR_SAMPLE_COUNT       3
#define SCRUB_INTERVAL_MS         500
#define SUN_LOSS_TIMEOUT_MS       5000

// HARDWARE PIN DEFINITIONS
#define SENSOR_PIN_TOPLEFT        A0
#define SENSOR_PIN_TOPRIGHT       A1
#define SENSOR_PIN_BOTTOMLEFT     A2
#define SENSOR_PIN_BOTTOMRIGHT    A3
#define BATTERY_VOLTAGE_PIN       A4

#define SERVO_AZIMUTH_PIN         9
#define SERVO_ELEVATION_PIN       10
#define LED_HEARTBEAT_PIN         13

// TRACKING PARAMETERS
#define DEADBAND_DEGREES          2.0f
#define PROPORTIONAL_GAIN         0.8f
#define MIN_ELEVATION_DEG         30
#define MAX_ELEVATION_DEG         150
#define MIN_AZIMUTH_DEG           0
#define MAX_AZIMUTH_DEG           180
#define DEFAULT_AZIMUTH_DEG       90
#define DEFAULT_ELEVATION_DEG     90

// FAULT THRESHOLDS
#define MAX_ERROR_COUNT           10
#define SENSOR_MIN_VALUE          50
#define SENSOR_MAX_VALUE          950

// EEPROM ADDRESSES
#define CONFIG_PRIMARY_ADDR       0x0000
#define CONFIG_BACKUP_ADDR        0x0100
#define CONFIG_MAGIC              0xA55A
#define CONFIG_VERSION            1

// CONTROL FLOW SIGNATURES
#define SIG_INIT      0xA5A5
#define SIG_SENSOR    0x3C3C
#define SIG_TRACKING  0x5A5A
#define SIG_SERVO     0xC3C3
#define SIG_EXPECTED  (SIG_INIT ^ SIG_SENSOR ^ SIG_TRACKING ^ SIG_SERVO)

#define CMD_BUFFER_SIZE           64
#define CMD_MAX_ARG_LENGTH        20

#endif // CONFIG_H