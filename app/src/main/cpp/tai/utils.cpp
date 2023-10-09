#include "utils.h"

uint8_t checksum(uint8_t *buf, uint16_t length) {
  uint8_t crc8 = 0;
  while (length--) {
    crc8 = crc8 ^ (*buf++);
  }
  return crc8;
}
