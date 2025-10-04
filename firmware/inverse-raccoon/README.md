# Lunar Solar Tracker Firmware
**Fault-tolerant solar tracking system for Arduino**
Built for the Space.Apps.Ottawa 2025 Hardware Challenge
## What This Does
Autonomous sun tracking using 4 photoresistors and servo motors, with fault-tolerance techniques borrowed from spaceflight systems. The tracker reads light sensors, calculates where the sun is, and moves servos to follow it. If things go wrong (sensors glitch, servos fail, memory corrupts), the system detects it and tries to keep going or fails safe.
## Architecture Overview
7 Modules, Clean Separation:

- Main Loop (main.cpp) - Orchestrates everything at ~100ms intervals, feeds the watchdog
- Sensor Manager - Reads 4 photoresistors with 3-sample median filtering, detects sun position
- Tracking Controller - Proportional control with dead-band to avoid jitter
- Servo Driver - PWM generation with CRC validation on commands
- Safety Manager - Error counting, mode management (Normal → Degraded → Safe → Emergency)
- Telemetry - JSON output over serial (115200 baud), LED heartbeat
- Config Manager - EEPROM storage with Software ECC (Hamming codes), dual-copy backup

## Fault-Tolerance Features (The Cool Stuff)

- Triple Modular Redundancy (TMR) - Critical variables stored 3x, majority vote corrects corruption
- Software ECC - Hamming(7,4) codes on EEPROM, auto-corrects single-bit errors
- Safe Boot - Validates both config copies on startup, falls back to defaults if both corrupt
- CRC Validation - All inter-module data transfers checksummed
- Control Flow Checking - XOR signatures verify code executes in correct sequence
- Memory Scrubbing - Background task validates TMR variables every 500ms
- Graceful Degradation - System drops to reduced-function modes instead of crashing
- Watchdog Timer - 8s timeout, fed every loop cycle

## What Needs Tuning
Check these TODOs in the code:

- Sensor threshold (sensor_manager.cpp:74) - Sun detection threshold is currently 200, might need adjustment based on ambient light
- Scaling factors (sensor_manager.cpp:83-84) - Error-to-degrees conversion factor (currently /10.0) needs calibration with actual hardware
- Magic numbers in main loop - Telemetry interval (1000ms), config save (60s), error reset (30s) - should be moved to config.h