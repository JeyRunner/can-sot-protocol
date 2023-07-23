#include <doctest/doctest.h>
#include <cstring>
#include "communication/SOTMaster.h"
#include "communication/SOTClient.h"
#include "vector"
#include "list"

#include "TestUtil.h"
#include "MockMasterClient.hpp"

using namespace std;


TEST_CASE("MasterClient communication: init") {
  TestSOTMaster master;
  TestSOTClient client;
  CHECK(master.clients.size() == 0);

  master.addAndConnectToClient(1);
  CHECK(master.clients.size() == 1);
  CHECK(master.clients[1].communicationState == SOT_COMMUNICATION_STATE::INITIALIZING);
  REQUIRE(master.framesSend.size() == 1);
  CHECK(master.getLastSendFrameType() == INIT_COMMUNICATION_REQUEST);

  client.processCanFrameReceived(master.getLastSendFrame());
  CHECK(client.communicationState == SOT_COMMUNICATION_STATE::INITIALIZED);
  CHECK(client.framesSend.size() == 2);
  CHECK(client.getSendFrameType(0) == WRITE_NODE_VALUE_REQEUST);
  CHECK(client.getSendFrame(0).dataLength == 1 + 4);
  CHECK(client.getSendFrame(0).data[0] == client.protocolDef.metaNodeValuesToSendOnInit[0]->nodeId);
  CHECK(client.getSendFrameType(1) == INIT_COMMUNICATION_RESPONSE);

  master.clearFramesSend();
  master.processCanFramesReceived(client.framesSend);
  REQUIRE(master.clients.size() == 1);
  CHECK(master.clients[1].communicationState == SOT_COMMUNICATION_STATE::INITIALIZED);
  CHECK(master.framesSend.empty());
}


TEST_CASE("MasterClient communication: master send write to client") {
  TestSOTMaster master;
  TestSOTClient client;

  master.addAndConnectToClient(1);

  // set node value that is sent on init
  client.protocolDef.objectTree.value3ThatIsSendOnInit.write(55.2);
  CHECK(client.protocolDef.objectTree.value3ThatIsSendOnInit.read() == doctest::Approx(55.2));
  client.processCanFramesReceived(master.framesSend);

  master.clearFramesSend();
  master.processCanFramesReceived(client.framesSend);
  CHECK(master.clients[1].protocol.objectTree.value3ThatIsSendOnInit.read() == doctest::Approx(55.2));
}
