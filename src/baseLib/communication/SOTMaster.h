#pragma once

#include <map>
#include <stdexcept>
#include "objectTree/OTDeclares.h"
#include "can/CanPackages.h"
#include "SOTDefs.h"
#include "util/Logging.h"
#include "objectTree/ObjectTree.h"
#include "SOTCanCommunication.h"
#include "util/EventFlag.h"

template<template <class T> class PROTOCOL_DEF>
class SOTMaster;


template<template <class T> class PROTOCOL_DEF>
struct ConnectedClient {
    SOT_COMMUNICATION_STATE communicationState = SOT_COMMUNICATION_STATE::UNINITIALIZED;
    /// contains object tree
    PROTOCOL_DEF<ConnectedClient> protocol;

    /// event flag is set when client is successfully connected to this master
    EventFlag gotConnectedEvent;

    uint8_t deviceId;
    SOTMaster<PROTOCOL_DEF> *sotMaster = nullptr;
    explicit ConnectedClient() : protocol(this) {}

    inline void sendValue(ValueNodeAbstract &vNode) {
        sotMaster->sendValue(vNode, deviceId);
    }

    inline void sendReadValueReq(ValueNodeAbstract &vNode) {
        sotMaster->sendReadValueReq(vNode, deviceId);
    }
};


#ifdef DEV_MODE
using PROTOCOL_DEF = ProtocolDef<1,1>; // just dummy value to get code autocompletion
#else
template<template <class T> class PROTOCOL_DEF /* = ProtocolDef<_DummpProtocl, 1,1>*/>
#endif
class SOTMaster: public SOTCanCommunication<PROTOCOL_DEF> {
    using SOTCanCommunication<PROTOCOL_DEF>::checkPackageDataSizeForNodeId;
    using SOTCanCommunication<PROTOCOL_DEF>::checkPackageDataSizeForNodeValue;
    using SOTCanCommunication<PROTOCOL_DEF>::sendInitCommunicationResponse;
    using SOTCanCommunication<PROTOCOL_DEF>::sendInitCommunicationRequest;
    using SOTCanCommunication<PROTOCOL_DEF>::sendReadNodeValueRequest;
    using SOTCanCommunication<PROTOCOL_DEF>::sendReadNodeValueResponse;
    using SOTCanCommunication<PROTOCOL_DEF>::sendWriteNodeValueRequest;
    using SOTCanCommunication<PROTOCOL_DEF>::myDeviceId;

  private:
    //using M = ConnectedClient<PROTOCOL_DEF>;
    using Client = ConnectedClient<PROTOCOL_DEF>;

    /// id is the client deviceId
    std::map<uint8_t, Client> clients;

  public:
    explicit SOTMaster() {
      myDeviceId = 0;
    }


    /**
     * Process Can frame from the receive buffer.
     * Handle responses, writes value to OT.
     */
    void processCanFrameReceived(CanFrame &frame) final {
      DeviceIdAndSOTMessageType devIdAndType = unpackCanFrameId(frame);

      switch (devIdAndType.messageType) {
        case INIT_COMMUNICATION_RESPONSE: {
          auto &client = getClient(devIdAndType);
          if (client.communicationState == SOT_COMMUNICATION_STATE::INITIALIZING) {
            client.communicationState = SOT_COMMUNICATION_STATE::INITIALIZED;
            client.gotConnectedEvent._triggerEvent();
          }
          else {
            logWarn("Got INIT_COMMUNICATION_RESPONSE although currently not in SENT_INITIALIZATION_REQUEST state");
          }
          break;
        }

        case COMMUNICATION_ERROR:
          logWarn("Got COMMUNICATION_ERROR");
          break;


        case WRITE_NODE_VALUE_REQEUST: {
          RETURN_IF_FALSE(checkPackageDataSizeForNodeId(frame));
          auto node = getValueNodeById(frame.data[0], devIdAndType);
          RESULT_WHEN_ERR_RETURN(node);
          // check received frame data size depending on data type
          RETURN_IF_FALSE(checkPackageDataSizeForNodeValue(frame, node));
          node._->readFromData(&frame.data[1]);
          break;
        }
        case READ_NODE_VALUE_REQEUST: {
          RETURN_IF_FALSE(checkPackageDataSizeForNodeId(frame));
          auto node = getValueNodeById(frame.data[0], devIdAndType);
          RESULT_WHEN_ERR_RETURN(node);
          sendReadNodeValueResponse(*node._, devIdAndType.sourceDeviceId);
          break;
        }
        case READ_NODE_VALUE_RESPONSE: {
          RETURN_IF_FALSE(checkPackageDataSizeForNodeId(frame));
          auto node = getValueNodeById(frame.data[0], devIdAndType);
          RESULT_WHEN_ERR_RETURN(node);
          // check received frame data size depending on data type
          RETURN_IF_FALSE(checkPackageDataSizeForNodeValue(frame, node));
          node._->readFromData(&frame.data[1]);
          break;
        }


        default: {
          if ((uint8_t)devIdAndType.messageType >= SOT_MESSAGE_ID_FIRST_SP_ID) {
            // it's a Stream Package
          }
          else {
            logWarn("Got non supported package type %d for master", devIdAndType.messageType);
          }
          break;
        }
      }
    }

private:
    Client &getClient(DeviceIdAndSOTMessageType &devIdAndType) {
      return clients.find(devIdAndType.sourceDeviceId)->second;
    }


    Result<ValueNodeAbstract*> getValueNodeById(uint8_t nodeId, DeviceIdAndSOTMessageType &devIdAndType) {
      auto &client = getClient(devIdAndType);
      if (nodeId > client.protocol.otTableSize) {
        logWarn("Accessed NodeId %d does not exist", nodeId);
        return Result<ValueNodeAbstract*>::Error();
      }
      else {
        return Result<ValueNodeAbstract*>::Ok(client.protocol.otNodeIDsTable[nodeId]);
      }
    }


public:

    /**
     * Add a client and connect with it.
     * This is non-blocking (after returning from this function the connection initiation will not be finished)
     * @param clientDeviceId
     * @return true when the client does not exist already in the client list, and thus was added successfully.
     */
    bool addAndConnectToClient(const uint8_t clientDeviceId) {
      if (clients.contains(clientDeviceId)) {
        logError("can't add client %d since its already in the client list", clientDeviceId);
        return false;
      }
      auto &client = clients[clientDeviceId]; //clients.emplace(clientDeviceId, ConnectedClient<PROTOCOL_DEF>{}); // @todo seems move values on access -> references are invalid
      client.sotMaster = this;
      client.deviceId = clientDeviceId;
      sendInitCommunicationRequest(clientDeviceId);
      client.communicationState = SOT_COMMUNICATION_STATE::INITIALIZING;
      return true;
    }


    /**
     * Get list of all clients.
     * @note no elements should be added to the map manually!
     */
    const std::map<uint8_t, Client> &getClients() {
        return clients;
    }

    /**
     * Get a specific client by its device id. Throws when client with device id does not exist.
     * @param clientDeviceId
     */
    Client &getClient(uint8_t clientDeviceId) {
        auto c = clients.find(clientDeviceId);
        if (c == clients.end()) {
            throw std::runtime_error("client with does not exist");
        }
        return c->second;
    }



private:
    inline void sendValue(ValueNodeAbstract &vNode, uint8_t clientDeviceId) {
        sendWriteNodeValueRequest(vNode, clientDeviceId);
    }

    inline void sendReadValueReq(ValueNodeAbstract &vNode, uint8_t clientDeviceId) {
        sendReadNodeValueRequest(vNode, clientDeviceId);
    }

    friend Client;
};