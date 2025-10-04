/**
 * @file safety_manager.h
 * @brief Safety monitoring and fault management
 */

#ifndef SAFETY_MANAGER_H
#define SAFETY_MANAGER_H

#include "types.h"

/**
 * @brief Initialize safety manager
 */
void safety_manager_init();

/**
 * @brief Evaluate system mode based on error counts
 */
void safety_evaluate_mode();

/**
 * @brief Memory scrubbing routine (periodic call)
 */
void safety_scrub_memory();

/**
 * @brief Verify control flow signature
 * @param signature Current flow signature
 * @return true if valid, false if corrupted
 */
bool safety_verify_control_flow(uint16_t signature);

/**
 * @brief Get current system mode
 * @return Current operating mode
 */
SystemMode_t safety_get_mode();

/**
 * @brief Set system mode
 * @param mode New operating mode
 */
void safety_set_mode(SystemMode_t mode);

/**
 * @brief Increment error counter
 * @param error Error code
 */
void safety_log_error(ErrorCode_t error);

/**
 * @brief Get error count for specific error code
 * @param error Error code
 * @return Error count
 */
uint16_t safety_get_error_count(ErrorCode_t error);

/**
 * @brief Get total error count across all types
 * @return Total errors
 */
uint32_t safety_get_total_errors();

#endif // SAFETY_MANAGER_H