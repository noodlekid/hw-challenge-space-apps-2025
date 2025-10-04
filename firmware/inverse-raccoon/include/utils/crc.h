/**
 * @file crc.h
 * @brief CRC-16 error detection functions
 */

#ifndef CRC_H
#define CRC_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Calculate CRC-16-CCITT
 * @param data Pointer to data buffer
 * @param length Number of bytes to process
 * @return CRC-16 checksum
 */
uint16_t crc16(const void* data, size_t length);

#endif // CRC_H