#pragma once
#include <string>

namespace utf8 {
inline size_t char_byte_count(unsigned char c) {
  if ((c & 0x80) == 0) {
    return 1;
  }
  if ((c & 0xE0) == 0xC0) {
    return 2;
  }
  if ((c & 0xF0) == 0xE0) {
    return 3;
  }
  if ((c & 0xF8) == 0xF0) {
    return 4;
  }
  return 1;
}
inline size_t byte_position(const std::string& str, size_t char_pos) {
  size_t byte_pos = 0;
  size_t char_count = 0;
  while (byte_pos < str.size() && char_count < char_pos) {
    byte_pos += char_byte_count(str[byte_pos]);
    char_count++;
  }
  return byte_pos;
}
inline size_t length(const std::string& str) {
  size_t len = 0;
  for (size_t i = 0; i < str.size(); i += char_byte_count(str[i])) {
    len++;
  }
  return len;
}
}