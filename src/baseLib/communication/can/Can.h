#pragma once

#include <cinttypes>

template<uint8_t DATA_SIZE>
struct CanData {
    uint8_t data[DATA_SIZE];
};

struct CanFrame {
    /// 11 bit can Id
    uint16_t canId;
    /// 0-8 bytes of data
    uint8_t *data = nullptr;
    /// number of bytes in data
    uint8_t dataLength = 0;
};