#pragma once

#include "Can.h"
#include "CheckEndianness.h"


/**
 * Types for CAN SOT Package Frames.
 * The value of the type is also the SOT message ID (6 bit).
 */
enum SOT_MESSAGE_TYPE
{
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
     * To disconnect a client, the master can send a disconnect communication request to the client.
     * Afterward, client and master are not connected anymore.
     * data = 1 bytes
     */
    DISCONNECT_COMMUNICATION_REQUEST = 0b00'0011,


    /**
     * On device sends this to the other device its communicating with to indicate there is a general communication error/problem.
     * data = 1 bytes
     */
    COMMUNICATION_ERROR = 0b00'0010,

    /**
     * Send a node value to another device.
     * data = 1-8 bytes
     */
    WRITE_NODE_VALUE_REQEUST = 0b00'0100,

    /**
     * Response is sent after Write Node Value Request was received.
     * data = 1 bytes
     */
    READ_NODE_VALUE_REQEUST = 0b00'1000,

    /**
     * Request to read a node value from another device.
     * data = 1-8 bytes
     */
    READ_NODE_VALUE_RESPONSE = 0b00'1001,
};


struct DeviceIdAndSOTMessageType {
    /// 3 bit source device ID
    uint8_t sourceDeviceId;

    /// 3 bit target device ID
    uint8_t targetDeviceId;

    /// 5 bit
    SOT_MESSAGE_TYPE messageType;
};

/**
 * Get DeviceId and SOT message ID from can frame ID.
 */
static DeviceIdAndSOTMessageType unpackCanFrameId(CanFrame &frame) {
  return DeviceIdAndSOTMessageType{
    .sourceDeviceId = (uint8_t) ((frame.canId >> 8) & 0b000'0000'0111),
    .targetDeviceId = (uint8_t) ((frame.canId >> 5) & 0b000'0000'0111),
    .messageType = (SOT_MESSAGE_TYPE) ((frame.canId >> 0) & 0b000'0001'1111),
  };
}

/**
 * Write DeviceId and SOT message ID into CAN ID of given CAN frame.
 */
static void packCanFrameId(CanFrame &frame, DeviceIdAndSOTMessageType idAndType) {
  frame.canId =
      ((idAndType.sourceDeviceId & 0b000'0000'0111) << 8) |
      ((idAndType.targetDeviceId & 0b000'0000'0111) << 5) |
      ((idAndType.messageType & 0b000'0001'1111) << 0);
}


enum COMMUNICATION_ERROR_TYPES {
    /// microcontroller is not fast enough to respond to all incoming can requests.
    CAN_REQUESTS_TO_FAST,

    /// microcontroller is not fast enough to progress all incoming can requests.
    CAN_RECEIVE_OVERFLOW,
};

enum class INIT_COMMUNICATION_RESPONSE_TYPES {
    /// accept communication
    ACCEPT,

    /// reject communication request, since e.g. already connected
    NOT_ACCEPT_NOT_IN_UNINITIALIZED_STATE,
};


/// Stream Packages ID range is 16-32
const uint8_t SOT_MESSAGE_ID_FIRST_SP_ID = 16;


