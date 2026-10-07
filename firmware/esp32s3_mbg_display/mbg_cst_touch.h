#pragma once
#include <stdint.h>

// CST820/816 registers 0x02..0x06, rotated to the TFT's landscape rotation 1.
inline bool mbgCstLandscapeTouch(const uint8_t data[5], uint16_t &x, uint16_t &y) {
  if ((data[0] & 0x0F) != 1 || (data[1] >> 6) == 1) return false;
  const uint16_t rawX = ((data[1] & 0x0F) << 8) | data[2];
  const uint16_t rawY = ((data[3] & 0x0F) << 8) | data[4];
  if (rawX >= 240 || rawY >= 320) return false;
  x = rawY;
  y = 239 - rawX;
  return true;
}
