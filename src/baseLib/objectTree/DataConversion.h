#pragma once

inline void writeToDataUINT8(uint8_t &data, TYPE_UINT8 value) {
  *((uint8_t*) &data) = value;
}

template<class T>
inline void writeToDataENUM(uint8_t &data, T value) {
  *((uint8_t*) &data) = value;
}

inline void writeToDataUINT16(uint8_t &data, TYPE_UINT16 value) {
  *((TYPE_UINT16*) &data) = value;
}

inline void writeToDataF32(uint8_t &data, TYPE_F32 value) {
  *((TYPE_F32*) &data) = value;
}




inline void readFromDataUINT8(const uint8_t &data, TYPE_UINT8 &value) {
  value = *((uint8_t*) &data);
}

template<class T>
inline void readFromDataENUM(const uint8_t &data, T value) {
  return static_cast<T>(*((uint8_t*) &data));
}

inline void readFromDataUINT16(const uint8_t &data, TYPE_UINT16 &value) {
  value = *((TYPE_UINT16*) &data);
}

inline void readFromDataF32(const uint8_t &data, TYPE_F32 &value) {
  value = *((TYPE_F32*) &data);
}