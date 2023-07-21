#pragma once

#include <map>
#include "objectTree/OTDeclares.h"
#include "can/CanPackages.h"
#include "SOTDefs.h"
#include "util/Logging.h"
#include "objectTree/ObjectTree.h"
#include "SOTCanCommunication.h"


template<class PROTOCOL_DEF>
struct ConnectedClient {
    SOT_COMMUNICATION_STATE communicationState = SOT_COMMUNICATION_STATE::UNINITIALIZED;
    /// contains object tree
    PROTOCOL_DEF protocol;
};


#ifdef DEV_MODE
using PROTOCOL_DEF = ProtocolDef<1,1>; // just dummy value to get code autocompletion
#else
template<class PROTOCOL_DEF>
#endif
class SOTMaster: public SOTCanCommunication<PROTOCOL_DEF> {
    using SOTCanCommunication<PROTOCOL_DEF>::checkPackageDataSizeForNodeId;
    using SOTCanCommunication<PROTOCOL_DEF>::checkPackageDataSizeForNodeValue;
    using SOTCanCommunication<PROTOCOL_DEF>::sendInitCommunicationResponse;
    using SOTCanCommunication<PROTOCOL_DEF>::sendInitCommunicationRequest;
    using SOTCanCommunication<PROTOCOL_DEF>::sendReadNodeValueResponse;
    using SOTCanCommunication<PROTOCOL_DEF>::myDeviceId;

  public:
    /// id is the client deviceId
    std::map<uint8_t, ConnectedClient<PROTOCOL_DEF>> clients;

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
          sendReadNodeValueResponse(node._, devIdAndType.sourceDeviceId);
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

    ConnectedClient<PROTOCOL_DEF> &getClient(DeviceIdAndSOTMessageType &devIdAndType) {
      return clients[devIdAndType.sourceDeviceId];
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


    void addAndConnectToClient(const uint8_t clientDeviceId) {
      auto client = clients.emplace(clientDeviceId, ConnectedClient<PROTOCOL_DEF>{});
      sendInitCommunicationRequest(clientDeviceId);
      client.first->second.communicationState = SOT_COMMUNICATION_STATE::INITIALIZING;
    }

};