/**
 * @file ecc.cpp
 * @brief Error Correcting Code (Hamming) implementation
 */

#include "utils/ecc.h"

uint8_t hamming_encode(uint8_t data) {
  data &= 0x0F;
  
  uint8_t d0 = (data >> 0) & 1;
  uint8_t d1 = (data >> 1) & 1;
  uint8_t d2 = (data >> 2) & 1;
  uint8_t d3 = (data >> 3) & 1;
  
  uint8_t p0 = d0 ^ d1 ^ d3;
  uint8_t p1 = d0 ^ d2 ^ d3;
  uint8_t p2 = d1 ^ d2 ^ d3;
  
  return (p0 << 0) | (p1 << 1) | (d0 << 2) | 
         (p2 << 3) | (d1 << 4) | (d2 << 5) | (d3 << 6);
}

uint8_t hamming_decode(uint8_t encoded, bool* corrected) {
  uint8_t p0 = (encoded >> 0) & 1;
  uint8_t p1 = (encoded >> 1) & 1;
  uint8_t d0 = (encoded >> 2) & 1;
  uint8_t p2 = (encoded >> 3) & 1;
  uint8_t d1 = (encoded >> 4) & 1;
  uint8_t d2 = (encoded >> 5) & 1;
  uint8_t d3 = (encoded >> 6) & 1;
  
  uint8_t s0 = p0 ^ d0 ^ d1 ^ d3;
  uint8_t s1 = p1 ^ d0 ^ d2 ^ d3;
  uint8_t s2 = p2 ^ d1 ^ d2 ^ d3;
  
  uint8_t syndrome = (s0 << 0) | (s1 << 1) | (s2 << 2);
  
  *corrected = false;
  
  if (syndrome != 0) {
    encoded ^= (1 << (syndrome - 1));
    *corrected = true;
    
    d0 = (encoded >> 2) & 1;
    d1 = (encoded >> 4) & 1;
    d2 = (encoded >> 5) & 1;
    d3 = (encoded >> 6) & 1;
  }
  
  return (d0 << 0) | (d1 << 1) | (d2 << 2) | (d3 << 3);
}