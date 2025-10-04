/**
 * @file config_manager.h
 * @brief Configuration management with ECC-protected EEPROM storage
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "types.h"

/**
 * @brief Initialize configuration manager and perform safe boot
 */
void config_manager_init();

/**
 * @brief Get current configuration
 * @return Pointer to configuration structure
 */
const Config_t* config_get();

/**
 * @brief Get mutable configuration for updates
 * @return Pointer to configuration structure
 */
Config_t* config_get_mutable();

/**
 * @brief Persist configuration to both EEPROM locations
 */
void config_persist();

/**
 * @brief Validate configuration structure
 * @param cfg Configuration to validate
 * @return true if valid, false otherwise
 */
bool config_validate(const Config_t* cfg);

/**
 * @brief Load factory defaults
 * @param cfg Configuration to initialize
 */
void config_load_defaults(Config_t* cfg);

#endif // CONFIG_MANAGER_H