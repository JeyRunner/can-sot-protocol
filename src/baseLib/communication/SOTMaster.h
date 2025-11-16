#pragma once

#include <map>
#include <stdexcept>
#include <string>
#include "objectTree/OTDeclares.h"
#include "can/CanPackages.h"
#include "SOTDefs.h"
#include "util/Logging.h"
#include "objectTree/ObjectTree.h"
#include "SOTCanCommunication.h"
#include "util/EventFlag.h"

/*
template<template <class T> class PROTOCOL_DEF, class CAN_INTERFACE_CLASS>
class SOTMaster;
*/





template<template <class T> class PROTOCOL_DEF, class SOT_MASTER, class CONNECTED_CLIENT>
struct ConnectedClientGeneric {
    using SOT_MASTER_TYPE = SOT_MASTER;

    SOT_COMMUNICATION_STATE communicationState = SOT_COMMUNICATION_STATE::UNINITIALIZED;
    /// contains object tree
    PROTOCOL_DEF<CONNECTED_CLIENT> protocol;

    /// event flag is set when client is successfully connected to this master
    EventFlag gotConnectedEvent;

    /// event flag is set when client has an overflow in the send packages queue (-> you should slow down the communication with the client)
    EventFlag onCommunicationErrorTxOverflow;
    /// event flag is set when client has an overflow in the received packages queue (the client will not be able to process all packages send by the master)
    /// (-> you should slow down the communication with the client, reduce frequency of send packages)
    EventFlag onCommunicationErrorRxOverflow;


    uint8_t deviceId;
    SOT_MASTER *sotMaster = nullptr;
    explicit ConnectedClientGeneric() : protocol((CONNECTED_CLIENT*)this) {}

    inline void sendValue(ValueNodeAbstract &vNode) {
        sotMaster->sendValue(vNode, deviceId);
    }

    inline void sendReadValueReq(ValueNodeAbstract &vNode) {
        sotMaster->sendReadValueReq(vNode, deviceId);
    }

    inline void sendRemoteCallRequest(RemoteCallCallerAbstract &call, RemoteCallDataWritable &args) {
        sotMaster->sendRemoteCallRequest(call, args, deviceId);
    }

    /**
     * Return true if currently connected to client.
     */
    bool isConnected() {
      return communicationState == SOT_COMMUNICATION_STATE::INITIALIZED;
    }

    /**
     * Send a disconnect request to client and then a init communication request.
     * Will therefor try to reconnect to client.
     * When connected to the client the gotConnectedEvent flag will be set.
     */
    void reconnectToClient() {
      sotMaster->reconnectToClient(deviceId);
    }
};

template<template <class T> class PROTOCOL_DEF, class SOT_MASTER>
struct ConnectedClient: public ConnectedClientGeneric<PROTOCOL_DEF, SOT_MASTER, ConnectedClient<PROTOCOL_DEF, SOT_MASTER>> {
};



#ifdef DEV_MODE
using PROTOCOL_DEF = ProtocolDef<1,1>; // just dummy value to get code autocompletion
#else
template<template <class T> class PROTOCOL_DEF /* = ProtocolDef<_DummpProtocl, 1,1>*/, class CAN_INTERFACE_CLASS,
        class _MASTER_CLASS, class _CONNECTED_CLIENT_CLASS> //SOTMaster<PROTOCOL_DEF, CAN_INTERFACE_CLASS, X, X>>
#endif
class SOTMasterGeneric: public SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>
{
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::checkPackageDataSize;
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::checkPackageDataSizeForNodeId;
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::checkPackageDataSizeForRemoteCall;
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::checkPackageDataSizeForNodeValue;
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::checkPackageDataSizeForRemoteCallReturn;
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::getRemoteCallReturnOkFromPackage;
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::getRemoteCallIdFromPackage;
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::sendInitCommunicationResponse;
    //using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::sendInitCommunicationRequest;
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::sendDisconnectCommunicationRequest;
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::sendReadNodeValueRequest;
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::sendReadNodeValueResponse;
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::sendWriteNodeValueRequest;
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::myDeviceId;

  private:
    //using M = ConnectedClient<PROTOCOL_DEF>;
    //using Client = ConnectedClient<PROTOCOL_DEF, PROTOCOL_DEF::COMMUNICATION_CLASS_TYPE>;
    using Client = _CONNECTED_CLIENT_CLASS; //ConnectedClient<PROTOCOL_DEF, _MASTER_CLASS>;

    /// id is the client deviceId
    std::map<uint8_t, Client> clients;

  public:
    explicit SOTMasterGeneric(CAN_INTERFACE_CLASS &canInterface): SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>(canInterface) {
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
          RETURN_IF_FALSE(checkPackageDataSize(frame, 1));
          auto responseType = static_cast<INIT_COMMUNICATION_RESPONSE_TYPES>(frame.data[0]);
          if (client.communicationState == SOT_COMMUNICATION_STATE::INITIALIZING) {
            if (responseType == INIT_COMMUNICATION_RESPONSE_TYPES::ACCEPT) {
              client.communicationState = SOT_COMMUNICATION_STATE::INITIALIZED;
              client.gotConnectedEvent._triggerEvent();
            }
            else if (responseType == INIT_COMMUNICATION_RESPONSE_TYPES::NOT_ACCEPT_NOT_IN_UNINITIALIZED_STATE) {
              reconnectToClient(client.deviceId);
              logWarn("INIT_COMMUNICATION_REQUEST was not accepted by client, try to disconnect client and try again");
            }
          }
          else {
            logWarn("Got INIT_COMMUNICATION_RESPONSE although currently not in INITIALIZING state");
          }
          break;
        }

        case COMMUNICATION_ERROR: {
          auto &client = getClient(devIdAndType);
          RETURN_IF_FALSE(checkPackageDataSize(frame, 1));
          logWarn("Got COMMUNICATION_ERROR from client");
          switch (static_cast<COMMUNICATION_ERROR_TYPES>(frame.data[0])) {
            case CAN_RECEIVE_OVERFLOW:
              client.onCommunicationErrorRxOverflow._triggerEvent();
              break;
            case CAN_SEND_OVERFLOW:
              client.onCommunicationErrorTxOverflow._triggerEvent();
              break;
            default:
              logWarn("Got COMMUNICATION_ERROR from client with unknown error type: %d", ((unsigned int)frame.data[0]));
          }
          break;
        }


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

        case REMOTE_CALL_RETURN: {
          RETURN_IF_FALSE(checkPackageDataSizeForRemoteCall(frame));
          uint8_t callId = getRemoteCallIdFromPackage(frame);
          auto call = getRemoteCallCallerById(callId, devIdAndType);
          RESULT_WHEN_ERR_RETURN(call);
          bool returnedOk = getRemoteCallReturnOkFromPackage(frame);
          if (returnedOk) {
            // check received frame data size depending on data type
            RETURN_IF_FALSE(checkPackageDataSizeForRemoteCallReturn(frame, call));
            call._->__returnData->_readFromData(&frame.data[1]);
          }
          else {
            RETURN_IF_FALSE(checkPackageDataSizeForRemoteCall(frame, 2)); //  second byte for error code
            *(call._->__returnError) = frame.data[1];
          }
          *(call._->__returnIsError) = !returnedOk;
          call._->remoteCallReturned._triggerEvent();
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
        logWarn("Accessed NodeId with id %d does not exist", nodeId);
        return Result<ValueNodeAbstract*>::Error();
      }
      else {
        return Result<ValueNodeAbstract*>::Ok(client.protocol.otNodeIDsTable[nodeId]);
      }
    }

    Result<RemoteCallCallerAbstract*> getRemoteCallCallerById(uint8_t callId, DeviceIdAndSOTMessageType &devIdAndType) {
      auto &client = getClient(devIdAndType);
      if (callId > client.protocol.rcCallerTableSize) {
        logWarn("Accessed RemoteCall with id %d does not exist", callId);
        return Result<RemoteCallCallerAbstract*>::Error();
      }
      else {
        return Result<RemoteCallCallerAbstract*>::Ok(client.protocol.rcCallerTable[callId]);
      }
    }


public:

    /**
     * Add a client and connect with it.
     * This is non-blocking (after returning from this function the connection initiation will not be finished).
     * When connected to the client the gotConnectedEvent flag will be set of the corresponding client object.
     * @param clientDeviceId
     * @return true when the client does not exist already in the client list, and thus was added successfully.
     */
    bool addAndConnectToClient(const uint8_t clientDeviceId) {
      if (clients.contains(clientDeviceId)) {
        logError("can't add client %d since its already in the client list", clientDeviceId);
        return false;
      }
      auto &client = clients[clientDeviceId]; //clients.emplace(clientDeviceId, ConnectedClient<PROTOCOL_DEF>{}); // @todo seems move values on access -> references are invalid
      client.sotMaster = (_MASTER_CLASS*) this;
      client.deviceId = clientDeviceId;
      sendInitCommunicationRequest(clientDeviceId);
      client.communicationState = SOT_COMMUNICATION_STATE::INITIALIZING;
      return true;
    }

    /**
     * Send a disconnect request to client and then a init communication request.
     * Will therefor try to reconnect to client.
     * When connected to the client the gotConnectedEvent flag will be set of the corresponding client object.
     */
    void reconnectToClient(const uint8_t clientDeviceId) {
      getClient(clientDeviceId).communicationState = SOT_COMMUNICATION_STATE::INITIALIZING;
      sendDisconnectCommunicationRequest(clientDeviceId);
      sendInitCommunicationRequest(clientDeviceId);
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
            throw std::runtime_error("client with id " + std::to_string(clientDeviceId) + " does not exist");
        }
        return c->second;
    }



public:
    inline void sendValue(ValueNodeAbstract &vNode, uint8_t clientDeviceId) {
        sendWriteNodeValueRequest(vNode, clientDeviceId);
    }

    inline void sendReadValueReq(ValueNodeAbstract &vNode, uint8_t clientDeviceId) {
        sendReadNodeValueRequest(vNode, clientDeviceId);
    }


  protected:
    using SOTCanCommunication<PROTOCOL_DEF, CAN_INTERFACE_CLASS>::sendInitCommunicationRequest;

    friend Client;
};



template<template <class T> class PROTOCOL_DEF, class CAN_INTERFACE_CLASS>
class SOTMaster: public SOTMasterGeneric<PROTOCOL_DEF, CAN_INTERFACE_CLASS, SOTMaster<PROTOCOL_DEF, CAN_INTERFACE_CLASS>,
                                         ConnectedClient<PROTOCOL_DEF, SOTMaster<PROTOCOL_DEF, CAN_INTERFACE_CLASS>>>
{
  public:
    explicit SOTMaster(CAN_INTERFACE_CLASS &canInterface)
    : SOTMasterGeneric<PROTOCOL_DEF, CAN_INTERFACE_CLASS, SOTMaster<PROTOCOL_DEF, CAN_INTERFACE_CLASS>,
            ConnectedClient<PROTOCOL_DEF, SOTMaster<PROTOCOL_DEF, CAN_INTERFACE_CLASS>>>(canInterface) {
    }
};