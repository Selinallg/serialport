#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <vector>

// void SingleY2BGR24(uint8_t *src, const int32_t &width, const int32_t &height,
//                    std::vector<uint8_t> *dst);

// template <typename T>
// inline void OnUnpackNum(const uint8_t *data, T *num) {
//   *num = 0;
//   auto size = static_cast<int>(sizeof(T));
//   for (int i = (size - 1); i >= 0; --i) {
//     *num <<= 8;
//     *num |= static_cast<int8_t>(data[i]);
//   }
// }

static const int32_t OnMode = 2;  // 1 BE 2 LE

template <typename T>
inline void OnUnpackNum(const uint8_t *data, T *num) {
  *num = 0;
  auto size = static_cast<int>(sizeof(T));
  switch (OnMode) {
    case 1:
      for (int i = 0; i < size; ++i) {
        *num <<= 8;
        *num |= data[i];
      }
      break;
    case 2:
      for (int i = (size - 1); i >= 0; --i) {
        *num <<= 8;
        *num |= data[i];
      }
      break;
    default:
      break;
  }
}

template <typename T>
inline void OnPackNum(char *data, T num) {
  int size = static_cast<int>(sizeof(T));
  switch (OnMode) {
    case 1:
      for (int i = (size - 1); i >= 0; --i) {
        data[i] = num & 0xff;
        num >>= 8;
      }
      break;
    case 2:
      for (int i = 0; i < size; ++i) {
        data[i] = num & 0xff;
        num >>= 8;
      }
      break;
    default:
      break;
  }
}

uint8_t checksum(uint8_t *buf, uint16_t length);

#endif  // UTILS_H
