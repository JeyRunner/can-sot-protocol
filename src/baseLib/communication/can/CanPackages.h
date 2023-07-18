#pragma once

#include "Can.h"
#include "CheckEndianness.h"


/**
 * Types for CAN SOT Package Frames.
 * The value of the type is also the SOT message ID (6 bit).
 */
enum SOT_MESSAGE_TYPE {
  /**
   * This is the first exchanged message (from master to client).
   * data = 0 bytes
   */
  INIT_COMMUNICATION_REQUEST = 0b00'0000,

  /**
   * This is the response of the client to the masters Init Communication Request (from client to master).
   * data = 1 bytes
   */
  INIT_COMMUNICATION_RESPONSE = 0b00'0001,

  /**
   * On device sends this to the other device its communicating with to indicate there is a general communication error/problem.
   * data = 1 bytes
   */
  COMMUNICATION_ERROR = 0b00'0010,
};


struct DeviceIdAndSOTMessageType {
    /// 5 bit device id
    uint8_t deviceId;
    SOT_MESSAGE_TYPE packageType;
};

/**
 * Get DeviceId and SOT message ID from can frame ID.
 */
DeviceIdAndSOTMessageType unpackCanFrameId(CanFrame &frame) {
  return DeviceIdAndSOTMessageType{
    .deviceId = (uint8_t) ((frame.canId >> 6) & 0b000'0001'1111),
    .packageType = (SOT_MESSAGE_TYPE) ((frame.canId >> 0) & 0b000'0011'1111),
  };
}


enum CAN_PACKAGE_TYPE__COMMUNICATION_ERROR {
    /// microcontroller is not fast enough to respond to all incoming can requests.
    CAN_REQUESTS_TO_FAST,

    /// microcontroller is not fast enough to progress all incoming can requests.
    CAN_RECEIVE_OVERFLOW,
};


const uint8_t SOT_MESSAGE_ID_FIRST_SP_ID = 16;





/// the data is encoded as little endian
///  -> to also e.g. when int is in dataCarried it is also encoded as little endian
struct CanPackage {
    SOT_MESSAGE_TYPE canPackageType; // 4 bit

    /// the available fields depend on the canPackageType
    /// max 2^3=8 different field types
    uint8_t fieldType = 0; // 3 bit

    /// only relevant for packageTypes that can have different fieldTypes.
    ///  -> only when apply is true all previously send values (and this one) will be applied.
    /// e.g. useful for SET_PD_VALUES, to apply all different send PD values at once.
    bool apply = true; // 1 bit

    uint8_t *dataAll;
    uint8_t *dataCarried;
    uint8_t  dataCarriedLength;


    /// write and read contained data as long
    /// @todo: handle endianness
    bool &dataAsBool() {
      return *((bool *) dataCarried);
    }

    /// write and read contained data as unsigned int
    /// @todo: handle endianness
    uint16_t &dataAsUInt16() {
      return *((uint16_t *) dataCarried);
    }

    /// write and read contained data as long
    /// @todo: handle endianness
    int32_t &dataAsLong() {
      return *((int32_t *) dataCarried);
    }

    /// write and read contained data as float
    /// @todo: handle endianness
    float &dataAsFloat() {
      return *((float *) dataCarried);
    }

    /// write and read contained data as int32
    /// @todo: handle endianness
    int32_t &dataAsInt32() {
      return *((int32_t *) dataCarried);
    }





    /**
     * write first byte of dataAll according to canPackageType and fieldType.
     */
    void writeHeaderToDataAll() {
      this->dataAll[0] =
          ((static_cast<int>(canPackageType) << 4 ) & 0b11110000) |
          ((fieldType << 1) & 0b00001110) |
          (apply  & 0x0000'0001);
    }
};