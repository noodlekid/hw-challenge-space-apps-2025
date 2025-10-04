/**
 * @file crc.cpp
 * @brief CRC-16 error detection implementation
 */

#include "utils/crc.h"

uint16_t crc16(const void* data, size_t length) {
  uint16_t crc = 0xFFFF;
  const uint8_t* ptr = (const uint8_t*)data;
  
  for (size_t i = 0; i < length; i++) {
    crc ^= (uint16_t)ptr[i] << 8;
    
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x8000) {
        crc = (crc << 1) ^ 0x1021;
      } else {
        crc = crc << 1;
      }
    }
  }
  
  return crc;
}