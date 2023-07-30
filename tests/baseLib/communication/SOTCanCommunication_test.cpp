#include "DocTestIncl.h"
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
  CHECK(master.getClients().size() == 0);
  CHECK_THROWS(master.getClient(1));

  master.addAndConnectToClient(1);
  CHECK(master.getClients().size() == 1);
  CHECK(master.getClient(1).communicationState == SOT_COMMUNICATION_STATE::INITIALIZING);
  REQUIRE(master.framesSend.size() == 1);
  CHECK(master.getLastSendFrameType() == INIT_COMMUNICATION_REQUEST);
  CHECK(!master.getClient(1).gotConnectedEvent);


  client.processCanFrameReceived(master.getLastSendFrame());
  CHECK(client.communicationState == SOT_COMMUNICATION_STATE::INITIALIZED);
  CHECK(client.framesSend.size() == 2);
  CHECK(client.getSendFrameType(0) == WRITE_NODE_VALUE_REQEUST);
  CHECK(client.getSendFrame(0).dataLength == 1 + 4);
  CHECK(client.getSendFrame(0).data[0] == client.getProtocol().metaNodeValuesToSendOnInit[0]->nodeId);
  CHECK(client.getSendFrameType(1) == INIT_COMMUNICATION_RESPONSE);

  master.clearFramesSend();
  master.processCanFramesReceived(client.framesSend);
  REQUIRE(master.getClients().size() == 1);
  CHECK(master.getClient(1).communicationState == SOT_COMMUNICATION_STATE::INITIALIZED);
  CHECK(master.framesSend.empty());
  CHECK(master.getClient(1).gotConnectedEvent.checkAndReset());
  CHECK(!master.getClient(1).gotConnectedEvent);
}


TEST_CASE("MasterClient communication: client send initValues to master") {
  TestSOTMaster master;
  TestSOTClient client;

  master.addAndConnectToClient(1);

  // set node value that is sent on init
  client.getProtocol().objectTree.value3ThatIsSendOnInit.write(55.2);
  CHECK(client.getProtocol().objectTree.value3ThatIsSendOnInit.read() == doctest::Approx(55.2));
  client.processCanFramesReceived(master.framesSend);

  master.clearFramesSend();
  master.processCanFramesReceived(client.framesSend);
  CHECK(master.getClient(1).protocol.objectTree.value3ThatIsSendOnInit.read() == doctest::Approx(55.2));
}


TEST_CASE("MasterClient communication: master send read request to client") {
  TestSOTMaster master;
  TestSOTClient client;

  // connect
  master.addAndConnectToClient(1);
  auto &masterProtocol = master.getClient(1).protocol;
  client.processCanFramesReceived(master.framesSend);
  master.processCanFramesReceived(client.framesSend);
  client.clearFramesSend();
  master.clearFramesSend();

  // change value on client
  client.getProtocol().objectTree.settings.value2.write(27);

  // master reads value client master
  masterProtocol.objectTree.settings.value2.sendReadValueReq();
  // OR: masterProtocol.sendReadValueReq(masterProtocol.objectTree.settings.value2);
  REQUIRE(master.framesSend.size() == 1);
  CHECK(master.getLastSendFrameType() == READ_NODE_VALUE_REQEUST);
  CHECK_FALSE(masterProtocol.objectTree.settings.value1.receivedValueUpdate);
  CHECK_FALSE(masterProtocol.objectTree.settings.value2.receivedValueUpdate);

  // let client answer
  client.processCanFramesReceived(master.framesSend);
  REQUIRE(client.framesSend.size() == 1);
  CHECK(client.getLastSendFrameType() == READ_NODE_VALUE_RESPONSE);
  CHECK_FALSE(client.getProtocol().objectTree.settings.value1.receivedValueUpdate);
  CHECK_FALSE(client.getProtocol().objectTree.settings.value2.receivedValueUpdate);
  master.clearFramesSend();

  // master process packages
  master.processCanFramesReceived(client.framesSend);
  REQUIRE(master.framesSend.size() == 0);

  CHECK_FALSE(masterProtocol.objectTree.settings.value1.receivedValueUpdate);
  CHECK(masterProtocol.objectTree.settings.value2.receivedValueUpdate);
  CHECK(masterProtocol.objectTree.settings.value2.read() == 27);
}




TEST_CASE("MasterClient communication: master send write request to client") {
  TestSOTMaster master;
  TestSOTClient client;

  // connect
  master.addAndConnectToClient(1);
  auto &masterProtocol = master.getClient(1).protocol;
  client.processCanFramesReceived(master.framesSend);
  master.processCanFramesReceived(client.framesSend);
  client.clearFramesSend();
  master.clearFramesSend();

  CHECK_FALSE(client.getProtocol().objectTree.settings.value1.receivedValueUpdate);
  CHECK_FALSE(client.getProtocol().objectTree.settings.value2.receivedValueUpdate);

  // change value on master
  masterProtocol.objectTree.settings.value1.write(27);

  // master sends value to client
  masterProtocol.objectTree.settings.value1.sendValue();
  REQUIRE(master.framesSend.size() == 1);
  CHECK(master.getLastSendFrameType() == WRITE_NODE_VALUE_REQEUST);

  // let client answer
  client.processCanFramesReceived(master.framesSend);
  REQUIRE(client.framesSend.size() == 0);

  CHECK(client.getProtocol().objectTree.settings.value1.receivedValueUpdate);
  CHECK_FALSE(client.getProtocol().objectTree.settings.value2.receivedValueUpdate);
  CHECK(client.getProtocol().objectTree.settings.value1.read() == 27);
}
