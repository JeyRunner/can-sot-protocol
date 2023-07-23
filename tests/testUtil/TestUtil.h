#pragma once

#include <iostream>
#include <sstream>
#include <string>

#include <iostream>
#include <sstream>
#include <string>
#include <bitset>

template <typename T, size_t N>
std::string toString(const T (&arr)[N]) {
  std::stringstream ss;
  ss << "[";

  for (size_t i = 0; i < N; ++i) {
    ss << std::bitset<8 * sizeof(T)>(arr[i]);
    if (i < N - 1) {
      ss << ", ";
    }
  }

  ss << "]";
  return ss.str();
}

template<typename T>
std::string byteArrayToString(const T bytes[], size_t length) {
  std::stringstream ss;
  ss << "";

  for (size_t i = 0; i < length; ++i) {
    ss << std::bitset<8 * sizeof(T)>(bytes[i]);
    if (i < length - 1) {
      ss << " ";
    }
  }

  ss << "";
  return ss.str();
}