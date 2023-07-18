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
