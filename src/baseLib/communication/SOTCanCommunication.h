#pragma once

#include "objectTree/OTDeclares.h"
#include "can/CanPackages.h"
#include "SOTDefs.h"
#include "util/Logging.h"
#include "objectTree/ObjectTree.h"


enum class SOT_COMMUNICATION_STATE {
    UNINITIALIZED,
    SENT_INITIALIZATION_REQUEST,
    INITIALIZED,
};



class SOTCanCommunication {
  public:
    SOT_COMMUNICATION_STATE communicationState = SOT_COMMUNICATION_STATE::UNINITIALIZED;

    /**
     * Handle incoming can frame. Store the can frame in the receive buffer.
     */
    static void onCanFrameReceived(CanFrame &frame) {
    }

    /**
     * Process Can frame from the receive buffer.
     * Handle responses, writes value to OT.
     */
    void processCanFrameReceived(CanFrame &frame) {
      DeviceIdAndSOTMessageType devIdAndType = unpackCanFrameId(frame);

      switch (devIdAndType.packageType) {
        case INIT_COMMUNICATION_REQUEST: {
          if constexpr (IS_CLIENT) {
            // send all meta init node values
            for (auto valueNode : metaNodeValuesToSendOnInit) {
              sendWriteNodeValueRequest(*valueNode);
            }
          }
          break;
        }


        case INIT_COMMUNICATION_RESPONSE:
          if constexpr (IS_MASTER) {
            if (communicationState == SOT_COMMUNICATION_STATE::SENT_INITIALIZATION_REQUEST) {
              communicationState = SOT_COMMUNICATION_STATE::INITIALIZED;
            }
            else {
              logWarn("Got INIT_COMMUNICATION_RESPONSE although currently not in SENT_INITIALIZATION_REQUEST state");
            }

          }
          break;

        case COMMUNICATION_ERROR:
          logWarn("Got COMMUNICATION_ERROR");
          break;

        default: {
          if (devIdAndType.packageType >= SOT_MESSAGE_ID_FIRST_SP_ID) {
            // it's a Stream Package
          }
          break;
        }
      }
    }




    void sendWriteNodeValueRequest(ValueNodeAbstract &valueNode) {

    }


    /**
     * Put can frame into the send buffer.
     */
    static void canSendFrame(CanFrame &frame) {

    }
};