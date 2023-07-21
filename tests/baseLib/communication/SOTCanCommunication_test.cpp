#include <doctest/doctest.h>
#include <cstring>
#include "communication/SOTMaster.h"
#include "communication/SOTClient.h"
#include "vector"
#include "list"

#include "TestUtil.h"
#include "MockMasterClient.hpp"

using namespace std;


static SOT_MESSAGE_TYPE getMessageType(CanFrame &frame) {
    return unpackCanFrameId(frame).messageType;
}


TEST_CASE("MasterClient communication: init") {
    TestSOTMaster master;
    TestSOTClient client;

    master.addAndConnectToClient(1);
    CHECK(master.clients[1].communicationState == SOT_COMMUNICATION_STATE::INITIALIZING);
    CHECK(master.framesSend.size() == 1);
    CHECK(getMessageType(master.framesSend.back()) == INIT_COMMUNICATION_REQUEST);

    client.processCanFrameReceived(master.framesSend.back());
    CHECK(client.communicationState == SOT_COMMUNICATION_STATE::INITIALIZING);
    CHECK(client.framesSend.size() == 2);
    CHECK(getMessageType(client.getSendFrame(0)) == WRITE_NODE_VALUE_REQEUST);
    CHECK(client.getSendFrame(0).dataLength == 1+4);
    CHECK(client.getSendFrame(0).data[0] == client.protocolDef.metaNodeValuesToSendOnInit[0]->nodeId);
    CHECK(getMessageType(client.getSendFrame(1)) == INIT_COMMUNICATION_RESPONSE);

}
