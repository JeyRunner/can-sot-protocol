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
//#include "DriverTemplate.hpp"
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

    static bool checkPackageDataSize(const CanFrame &frame, uint8_t expectedDataSize) {
      if (frame.dataLength < expectedDataSize) {  // @todo do more strict size check with ==
        logError("received Can Frame data size %d bytes has not expected size of %d", frame.dataLength, expectedDataSize);
        return false;
      }
      return true;
    }

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
     * check received frame data size is >= 1, so that it can hold the caller id in the first byte
     * @return true if ok
     */
    static bool checkPackageDataSizeForRemoteCall(const CanFrame &frame, uint8_t requiredSize=1) {
      if (frame.dataLength < requiredSize) {
        logError("received Can Frame data size %d bytes is smaller %d byte (first byte required for call id)", frame.dataLength, requiredSize);
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

    /**
     * check received frame data size for a remote call
     * @return true if ok
     */
    static bool checkPackageDataSizeForRemoteCallReturn(const CanFrame &frame, const Result<RemoteCallCallerAbstract*> &call) {
      if (frame.dataLength < call._->__returnData->getRequiredDataSizeBytes()+1) {
        logError("received Can Frame data size %d bytes is smaller then expected %d bytes", frame.dataLength, call._->__returnData->getRequiredDataSizeBytes()+1);
        return false;
      }
      return true;
    }



    void sendInitCommunicationRequest(uint8_t targetDeviceId) {
      sendMessage(SOT_MESSAGE_TYPE::INIT_COMMUNICATION_REQUEST, targetDeviceId, nullptr, 0);
    }

    void sendInitCommunicationResponse(uint8_t targetDeviceId, INIT_COMMUNICATION_RESPONSE_TYPES type) {
      uint8_t dataSize = 1;
      uint8_t data[dataSize];
      data[0] = static_cast<uint8_t>(type);
      sendMessage(SOT_MESSAGE_TYPE::INIT_COMMUNICATION_RESPONSE, targetDeviceId, data, dataSize);
    }

    void sendDisconnectCommunicationRequest(uint8_t targetDeviceId) {
      sendMessage(SOT_MESSAGE_TYPE::DISCONNECT_COMMUNICATION_REQUEST, targetDeviceId, nullptr, 0);
    }


    void sendCommunicationError(uint8_t targetDeviceId, COMMUNICATION_ERROR_TYPES error) {
      uint8_t dataSize = 1;
      uint8_t data[dataSize];
      data[0] = error;
      sendMessage(SOT_MESSAGE_TYPE::COMMUNICATION_ERROR, targetDeviceId, data, dataSize, true);
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


    void sendRemoteCallRequest(RemoteCallCallerAbstract &call, RemoteCallDataWritable &args, uint8_t targetDeviceId) {
        uint8_t dataSize = 1 + args.getRequiredDataSizeBytes(); // first byte for nodeId
        uint8_t data[dataSize];
        data[0] = remoteCallPackIdAndReturn(call.id, false); // returnOk bit unused in emoteCallRequest
        args._writeToData(&data[1]);
        sendMessage(SOT_MESSAGE_TYPE::REMOTE_CALL_REQUEST, targetDeviceId, data, dataSize);
    }

    void sendRemoteCallResponseOk(RemoteCallCallableAbstract &call, RemoteCallDataWritable &returnData, uint8_t targetDeviceId) {
        uint8_t dataSize = 1 + returnData.getRequiredDataSizeBytes(); // first byte for nodeId
        uint8_t data[dataSize];
        data[0] = remoteCallPackIdAndReturn(call.id, true);
        returnData._writeToData(&data[1]);
        sendMessage(SOT_MESSAGE_TYPE::REMOTE_CALL_RETURN, targetDeviceId, data, dataSize);
    }

    void sendRemoteCallResponseError(RemoteCallCallableAbstract &call, uint8_t errorCode, uint8_t targetDeviceId) {
        uint8_t dataSize = 1 + 1; // first byte for nodeId
        uint8_t data[dataSize];
        data[0] = remoteCallPackIdAndReturn(call.id, false);
        data[1] = errorCode;
        sendMessage(SOT_MESSAGE_TYPE::REMOTE_CALL_RETURN, targetDeviceId, data, dataSize);
    }


    void sendMessage(SOT_MESSAGE_TYPE type, uint8_t targetDeviceId, uint8_t *data, uint8_t dataSize, bool frameIsOverflowError = false) {
      CanFrame frame;
      packCanFrameId(frame, DeviceIdAndSOTMessageType{
          .sourceDeviceId = myDeviceId,
          .targetDeviceId = targetDeviceId,
          .messageType = type
      });
      frame.data = data;
      frame.dataLength = dataSize;
      //canSendFrame(frame);
      bool ok = canInterface.canSendFrame(frame);

      // if !ok and
      if (!frameIsOverflowError) {
        // handle overflow errors
        if (canInterface.onTxOverflow.checkAndReset()) {
          sendCommunicationError(targetDeviceId, COMMUNICATION_ERROR_TYPES::CAN_SEND_OVERFLOW);
        }
      }
    }


    static NodeId nodeIdFromPackage(CanFrame frame) {
      return frame.data[0];
    }

    static inline uint8_t getRemoteCallIdFromPackage(CanFrame frame) {
      return (uint8_t) (frame.data[0] & 0b1111'1110) >> 1;
    }

    static inline bool getRemoteCallReturnOkFromPackage(CanFrame frame) {
      return (bool) (frame.data[0] & 0b000'0001);
    }

    static inline uint8_t remoteCallPackIdAndReturn(uint8_t remoteCallId, bool returnOk) {
        return (uint8_t) ((remoteCallId << 1) & 0b1111'1110) | (returnOk & 0b0000'0001);
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
    virtual void processCanFrames() {
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