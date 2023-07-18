#pragma once

#include <cinttypes>


using TYPE_UINT8 = uint8_t;
using TYPE_UINT16 = uint16_t;
using TYPE_F32 = float;

enum class VALUE_NODE_DATA_TYPES {
    UINT8,
    UINT16,
    F32
};



template<class TYPE>
static VALUE_NODE_DATA_TYPES getValueNoteDataType() = delete;

template<>  VALUE_NODE_DATA_TYPES getValueNoteDataType<TYPE_UINT8>() {
  return VALUE_NODE_DATA_TYPES::UINT8;
}
template<>  VALUE_NODE_DATA_TYPES getValueNoteDataType<TYPE_UINT16>() {
  return VALUE_NODE_DATA_TYPES::UINT16;
}
template<>  VALUE_NODE_DATA_TYPES getValueNoteDataType<TYPE_F32>() {
  return VALUE_NODE_DATA_TYPES::F32;
}