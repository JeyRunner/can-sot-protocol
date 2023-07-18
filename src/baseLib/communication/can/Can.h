#pragma once


#include <cinttypes>


struct CanFrame {
    /// 11 bit can Id
    uint16_t canId;
    /// 0-8 bytes of data
    uint8_t *data;
    /// number of bytes in data
    uint8_t dataLength;
};