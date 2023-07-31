#pragma once

#include "objectTree/OTDeclares.h"
#include "can/CanPackages.h"
#include "SOTDefs.h"
#include "util/Logging.h"
#include "objectTree/ObjectTree.h"
#include "objectTree/ProtocolDef.h"
#include "util/Result.h"
#include "driver_template/DriverTemplate.hpp"

#ifdef IS_MASTER_DEF
#include "set"
#endif




enum class SOT_COMMUNICATION_STATE {
    UNINITIALIZED,
    INITIALIZING,
    INITIALIZED,
};


#ifdef DEV_MODE
//template<class PROTOCOL_DEF> requires ProtocolDefType<PROTOCOL_DEF, 1>
template<class PROTOCOL_DEF = Protocol<1,1>>
#else
template<template <class T> class PROTOCOL_DEF /* = ProtocolDef<_DummpProtocl, 1,1>*/, class CAN_INTERFACE_CLASS>
#endif
class SOTCanCommunication {
  protected:
    CAN_INTERFACE_CLASS &canInterface;
    uint8_t myDeviceId = 0;

    /**
     * check received frame data size is >= 1, so that it can hold the nodeId in the first byte
     * @return true if ok
     */
    static bool checkPackageDataSizeForNodeId(const CanFrame &frame) {
      if (frame.dataLength < 1) {
        logError("received Can Frame data size %d bytes is smaller 1 byte (required for nodeId)", frame.dataLength);
        return false;
      }
      return true;
    }

    /**
     * check received frame data size depending on data type of the node
     * @return true if ok
     */
    static bool checkPackageDataSizeForNodeValue(const CanFrame &frame, const Result<ValueNodeAbstract*> &node) {
      if (frame.dataLength < node._->getRequiredDataSizeInBytes()+1) {
        logError("received Can Frame data size %d bytes is smaller then expected %d bytes", frame.dataLength, node._->getRequiredDataSizeInBytes()+1);
        return false;
      }
      return true;
    }



    void sendInitCommunicationRequest(uint8_t targetDeviceId) {
      sendMessage(SOT_MESSAGE_TYPE::INIT_COMMUNICATION_REQUEST, targetDeviceId, nullptr, 0);
    }

    void sendInitCommunicationResponse(uint8_t targetDeviceId) {
      sendMessage(SOT_MESSAGE_TYPE::INIT_COMMUNICATION_RESPONSE, targetDeviceId, nullptr, 0);
    }



    void sendWriteNodeValueRequest(ValueNodeAbstract &valueNode, uint8_t targetDeviceId) {
      uint8_t dataSize = valueNode.getRequiredDataSizeInBytes() + 1; // first byte for nodeId
      uint8_t data[dataSize];
      data[0] = valueNode.nodeId;
      valueNode.writeToData(&data[1]);
      sendMessage(SOT_MESSAGE_TYPE::WRITE_NODE_VALUE_REQEUST, targetDeviceId, data, dataSize);
    }



    void sendReadNodeValueRequest(ValueNodeAbstract &valueNode, uint8_t targetDeviceId) {
        uint8_t dataSize = 1; // first byte for nodeId
        uint8_t data[dataSize];
        data[0] = valueNode.nodeId;
        valueNode.writeToData(&data[1]);
        sendMessage(SOT_MESSAGE_TYPE::READ_NODE_VALUE_REQEUST, targetDeviceId, data, dataSize);
    }

    void sendReadNodeValueResponse(ValueNodeAbstract &valueNode, uint8_t targetDeviceId) {
      uint8_t dataSize = valueNode.getRequiredDataSizeInBytes() + 1; // first byte for nodeId
      uint8_t data[dataSize];
      data[0] = valueNode.nodeId;
      valueNode.writeToData(&data[1]);
      sendMessage(SOT_MESSAGE_TYPE::READ_NODE_VALUE_RESPONSE, targetDeviceId, data, dataSize);
    }



    void sendMessage(SOT_MESSAGE_TYPE type, uint8_t targetDeviceId, uint8_t *data, uint8_t dataSize) {
      CanFrame frame;
      packCanFrameId(frame, DeviceIdAndSOTMessageType{
          .sourceDeviceId = myDeviceId,
          .targetDeviceId = targetDeviceId,
          .messageType = type
      });
      frame.data = data;
      frame.dataLength = dataSize;
      //canSendFrame(frame);
      canInterface.canSendFrame(frame);
    }


    static NodeId nodeIdFromPackage(CanFrame frame) {
      return frame.data[0];
    }

    /**
     * Process a Can frame from the receive buffer.
     * Handle responses, writes value to OT.
     */
    virtual void processCanFrameReceived(CanFrame &frame) = 0;



public:
    explicit SOTCanCommunication(CAN_INTERFACE_CLASS &canInterface) : canInterface(canInterface) {}


    /**
     * Handle all received can frames in the can receive buffer.
     * Handle responses, writes values to OT.
     * This has to be called periodically, so that the can receive buffer does not flow over.
     */
    void processCanFrames() {
      uint8_t data[8] = {};
      CanFrame frame;
      frame.data = data;
      while (canInterface.getNextCanFrameReceived(frame)) {
          processCanFrameReceived(frame);
      }
    }


    /**
     * Allocates a new can from to send with the given size for the data.
     */
    //virtual CanFrame& allocNewCanTxFrame(uint8_t dataSize) {
    //}
};