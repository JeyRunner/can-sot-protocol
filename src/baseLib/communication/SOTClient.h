#pragma once

#include "objectTree/OTDeclares.h"
#include "can/CanPackages.h"
#include "SOTDefs.h"
#include "util/Logging.h"
#include "objectTree/ObjectTree.h"
#include "SOTCanCommunication.h"
#include "util/Result.h"



#ifdef DEV_MODE
using PROTOCOL_DEF = ProtocolDef<_DummpProtocl, 1,1>; // just dummy value to get code autocompletion
//template<class PROTOCOL_DEF> requires ProtocolDefType<PROTOCOL_DEF, 1>
#else
template<template <class T> class PROTOCOL_DEF /* = ProtocolDef<_DummpProtocl, 1,1>*/>
#endif
class SOTClient: public SOTCanCommunication<PROTOCOL_DEF> {
    using SOTCanCommunication<PROTOCOL_DEF>::checkPackageDataSizeForNodeId;
    using SOTCanCommunication<PROTOCOL_DEF>::sendInitCommunicationResponse;
    using SOTCanCommunication<PROTOCOL_DEF>::sendWriteNodeValueRequest;
    using SOTCanCommunication<PROTOCOL_DEF>::checkPackageDataSizeForNodeValue;
    using SOTCanCommunication<PROTOCOL_DEF>::sendReadNodeValueRequest;
    using SOTCanCommunication<PROTOCOL_DEF>::sendReadNodeValueResponse;
    using PROTOCOL = PROTOCOL_DEF<SOTClient<PROTOCOL_DEF>>;

  protected:
    /// contains the object tree
    PROTOCOL protocolDef;

    uint8_t masterDeviceId = 0xFF;
    SOT_COMMUNICATION_STATE communicationState = SOT_COMMUNICATION_STATE::UNINITIALIZED;


public:
    SOTClient() = delete;
    explicit SOTClient(uint8_t myDeviceId): protocolDef(this) {
      this->myDeviceId = myDeviceId;
    }


    /**
     * Process Can frame from the receive buffer.
     * Handle responses, writes value to OT.
     */
    void processCanFrameReceived(CanFrame &frame) final {
      DeviceIdAndSOTMessageType devIdAndType = unpackCanFrameId(frame);

      switch (devIdAndType.messageType) {
        case INIT_COMMUNICATION_REQUEST: {
          if (communicationState != SOT_COMMUNICATION_STATE::UNINITIALIZED) {
            logWarn("Got INIT_COMMUNICATION_REQUEST although currently not in UNINITIALIZED state");
            return;
          }
          masterDeviceId = devIdAndType.sourceDeviceId;
          // send all meta init node values
          for (auto valueNode : protocolDef.metaNodeValuesToSendOnInit) {
            sendWriteNodeValueRequest(*valueNode, masterDeviceId);
          }
          communicationState = SOT_COMMUNICATION_STATE::INITIALIZED;
          sendInitCommunicationResponse(masterDeviceId);
          break;
        }

        case COMMUNICATION_ERROR:
          logWarn("Got COMMUNICATION_ERROR");
          break;


        case WRITE_NODE_VALUE_REQEUST: {
          RETURN_IF_FALSE(checkPackageDataSizeForNodeId(frame));
          auto node = getValueNodeById(frame.data[0]);
          RESULT_WHEN_ERR_RETURN(node);
          // check received frame data size depending on data type
          RETURN_IF_FALSE(checkPackageDataSizeForNodeValue(frame, node));
          node._->readFromData(&frame.data[1]);
          break;
        }
        case READ_NODE_VALUE_REQEUST: {
          RETURN_IF_FALSE(checkPackageDataSizeForNodeId(frame));
          auto node = getValueNodeById(frame.data[0]);
          RESULT_WHEN_ERR_RETURN(node);
          sendReadNodeValueResponse(*node._, masterDeviceId);
          break;
        }
        case READ_NODE_VALUE_RESPONSE: {
          RETURN_IF_FALSE(checkPackageDataSizeForNodeId(frame));
          auto node = getValueNodeById(frame.data[0]);
          RESULT_WHEN_ERR_RETURN(node);
          // check received frame data size depending on data type
          RETURN_IF_FALSE(checkPackageDataSizeForNodeValue(frame, node));
          node._->readFromData(&frame.data[1]);
          break;
        }


        default: {
          if ((uint8_t)devIdAndType.messageType >= SOT_MESSAGE_ID_FIRST_SP_ID) {
            // it's a Stream Package
            // @todo handle stream package
          }
          else {
            logWarn("Got non supported package type %d for client", devIdAndType.messageType);
          }
          break;
        }
      }

    }


    /**
     * Use this function to access the object tree and SP of the protocol managed by this client.
     */
    PROTOCOL &getProtocol() {
      return protocolDef;
    }

protected:
    Result<ValueNodeAbstract*> getValueNodeById(uint8_t id) {
      if (id > protocolDef.otTableSize) {
        logWarn("Accessed NodeId %d does not exist", id);
        return Result<ValueNodeAbstract*>::Error();
      }
      else {
        return Result<ValueNodeAbstract*>::Ok(protocolDef.otNodeIDsTable[id]);
      }
    }


private:
    inline void sendValue(ValueNodeAbstract &vNode) {
        sendWriteNodeValueRequest(vNode, masterDeviceId);
    }

    inline void sendValueNodeRead(ValueNodeAbstract &vNode) {
        sendReadNodeValueRequest(&vNode, masterDeviceId);
    }
};