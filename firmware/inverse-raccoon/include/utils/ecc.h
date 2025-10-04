/**
 * @file ecc.h
 * @brief Error Correcting Code (Hamming) functions
 */

#ifndef ECC_H
#define ECC_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Encode 4 data bits with Hamming(7,4) SECDED
 * @param data 4-bit data value (only lower 4 bits used)
 * @return 7-bit encoded value with parity bits
 */
uint8_t hamming_encode(uint8_t data);

/**
 * @brief Decode Hamming(7,4) with error correction
 * @param encoded 7-bit encoded value
 * @param corrected Output parameter - set to true if error was corrected
 * @return Decoded 4-bit data value
 */
uint8_t hamming_decode(uint8_t encoded, bool* corrected);

#endif // ECC_H