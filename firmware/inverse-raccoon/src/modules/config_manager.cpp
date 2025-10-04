/**
 * @file config_manager.cpp
 * @brief Configuration management implementation
 */

#include "modules/config_manager.h"
#include "config.h"
#include "utils/crc.h"
#include "utils/ecc.h"
#include <EEPROM.h>
#include <string.h>

// Module state
static Config_t g_config;
static uint16_t g_local_error_counts[ERR_COUNT];

/**
 * @brief Save configuration to EEPROM with ECC
 */
static void config_save(const Config_t* cfg, uint16_t addr) {
  const uint8_t* bytes = (const uint8_t*)cfg;
  
  for (size_t i = 0; i < sizeof(Config_t); i++) {
    uint8_t low_nibble = hamming_encode(bytes[i] & 0x0F);
    uint8_t high_nibble = hamming_encode(bytes[i] >> 4);
    
    EEPROM.update(addr + i * 2, low_nibble);
    EEPROM.update(addr + i * 2 + 1, high_nibble);
  }
}

/**
 * @brief Load configuration from EEPROM with ECC correction
 */
static bool config_load(Config_t* cfg, uint16_t addr) {
  uint8_t* bytes = (uint8_t*)cfg;
  bool any_corrected = false;
  
  for (size_t i = 0; i < sizeof(Config_t); i++) {
    bool corrected_low = false, corrected_high = false;
    
    uint8_t low_encoded = EEPROM.read(addr + i * 2);
    uint8_t high_encoded = EEPROM.read(addr + i * 2 + 1);
    
    uint8_t low_nibble = hamming_decode(low_encoded, &corrected_low);
    uint8_t high_nibble = hamming_decode(high_encoded, &corrected_high);
    
    bytes[i] = (low_nibble & 0x0F) | ((high_nibble & 0x0F) << 4);
    
    if (corrected_low || corrected_high) {
      any_corrected = true;
    }
  }
  
  if (any_corrected) {
    Serial.println(F("[CONFIG] ECC corrected bit errors"));
  }
  
  return config_validate(cfg);
}

bool config_validate(const Config_t* cfg) {
  if (cfg->magic != CONFIG_MAGIC) return false;
  if (cfg->version != CONFIG_VERSION) return false;
  
  uint16_t computed = crc16(cfg, offsetof(Config_t, crc16));
  return computed == cfg->crc16;
}

void config_load_defaults(Config_t* cfg) {
  memset(cfg, 0, sizeof(Config_t));
  cfg->magic = CONFIG_MAGIC;
  cfg->version = CONFIG_VERSION;
  cfg->servo_azimuth_offset = 0;
  cfg->servo_elevation_offset = 0;
  cfg->boot_count = 0;
  cfg->crc16 = crc16(cfg, offsetof(Config_t, crc16));
}

void config_manager_init() {
  Serial.println(F("\n=== SAFE BOOT SEQUENCE ==="));
  
  Config_t primary, backup;
  bool primary_valid = config_load(&primary, CONFIG_PRIMARY_ADDR);
  bool backup_valid = config_load(&backup, CONFIG_BACKUP_ADDR);
  
  if (primary_valid) {
    memcpy(&g_config, &primary, sizeof(Config_t));
    Serial.println(F("[BOOT] Primary config OK"));
  } 
  else if (backup_valid) {
    memcpy(&g_config, &backup, sizeof(Config_t));
    Serial.println(F("[BOOT] Primary corrupt, restored from backup"));
    config_save(&g_config, CONFIG_PRIMARY_ADDR);
    g_local_error_counts[ERR_PRIMARY_CONFIG_CORRUPT]++;
  } 
  else {
    Serial.println(F("[BOOT] Both configs corrupt, loading defaults"));
    config_load_defaults(&g_config);
    g_local_error_counts[ERR_CONFIG_LOST]++;
  }
  
  g_config.boot_count++;
  
  memcpy(g_local_error_counts, g_config.error_counts, sizeof(g_local_error_counts));
  
  Serial.print(F("[BOOT] Boot count: "));
  Serial.println(g_config.boot_count);
}

const Config_t* config_get() {
  return &g_config;
}

Config_t* config_get_mutable() {
  return &g_config;
}

void config_persist() {
  // Update error counts in config
  memcpy(g_config.error_counts, g_local_error_counts, sizeof(g_config.error_counts));
  
  // Recalculate CRC
  g_config.crc16 = crc16(&g_config, offsetof(Config_t, crc16));
  
  // Write to both locations
  config_save(&g_config, CONFIG_PRIMARY_ADDR);
  config_save(&g_config, CONFIG_BACKUP_ADDR);
}