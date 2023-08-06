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
template<template <class T> class PROTOCOL_DEF /* = ProtocolDef<_DummpProtocl, 1,1>*/, class CAN_INTERFACE_CLASS>
#endif
class SOTClient: public SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS> {
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::checkPackageDataSizeForNodeId;
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::sendCommunicationError;
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::sendInitCommunicationResponse;
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::sendWriteNodeValueRequest;
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::checkPackageDataSizeForNodeValue;
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::sendReadNodeValueRequest;
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::sendReadNodeValueResponse;
    using PROTOCOL = PROTOCOL_DEF<SOTClient<PROTOCOL_DEF, CAN_INTERFACE_CLASS>>;

  protected:
    /// contains the object tree
    PROTOCOL protocolDef;

    uint8_t masterDeviceId = 0xFF;
    SOT_COMMUNICATION_STATE communicationState = SOT_COMMUNICATION_STATE::UNINITIALIZED;


public:
    SOTClient() = delete;
    explicit SOTClient(CAN_INTERFACE_CLASS &canInterface, uint8_t myDeviceId)
    : SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>(canInterface), protocolDef(this)
    {
      //canInterface.setSotCanCommunication(this);
      this->myDeviceId = myDeviceId;
    }


    /**
     * Process Can frame from the receive buffer.
     * Handle responses, writes value to OT.
     */
    void processCanFrameReceived(CanFrame &frame) final {
      DeviceIdAndSOTMessageType devIdAndType = unpackCanFrameId(frame);

      // do nothing when not connected
      if (communicationState != SOT_COMMUNICATION_STATE::INITIALIZED) {
        if (devIdAndType.messageType != INIT_COMMUNICATION_REQUEST) {
          logWarn("Ignore incoming package, since currently not in state INITIALIZED");
          return;
        }
      }

      switch (devIdAndType.messageType) {
        case INIT_COMMUNICATION_REQUEST: {
          if (communicationState != SOT_COMMUNICATION_STATE::UNINITIALIZED) {
            logWarn("Got INIT_COMMUNICATION_REQUEST although currently not in UNINITIALIZED state");
            sendInitCommunicationResponse(devIdAndType.sourceDeviceId, INIT_COMMUNICATION_RESPONSE_TYPES::NOT_ACCEPT_NOT_IN_UNINITIALIZED_STATE);
            return;
          }
          masterDeviceId = devIdAndType.sourceDeviceId;
          // send all meta init node values
          for (auto valueNode : protocolDef.metaNodeValuesToSendOnInit) {
            sendWriteNodeValueRequest(*valueNode, masterDeviceId);
          }
          communicationState = SOT_COMMUNICATION_STATE::INITIALIZED;
          sendInitCommunicationResponse(masterDeviceId, INIT_COMMUNICATION_RESPONSE_TYPES::ACCEPT);
          gotConnectedEvent._triggerEvent();
          break;
        }

        case DISCONNECT_COMMUNICATION_REQUEST: {
          communicationState = SOT_COMMUNICATION_STATE::UNINITIALIZED;
          gotConnectedEvent.clear();
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


    /// event flag is set when client is successfully connected to this master
    EventFlag gotConnectedEvent;


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


public:
    inline void sendValue(ValueNodeAbstract &vNode) {
        sendWriteNodeValueRequest(vNode, masterDeviceId);
    }

    inline void sendValueNodeRead(ValueNodeAbstract &vNode) {
        sendReadNodeValueRequest(&vNode, masterDeviceId);
    }


    /**
     * Return true if currently connected to master.
     */
    bool isConnected() {
      return communicationState == SOT_COMMUNICATION_STATE::INITIALIZED;
    }

};