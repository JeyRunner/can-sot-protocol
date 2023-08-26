#include "DocTestIncl.h"
#include <cstring>
#include "communication/SOTMaster.h"
#include "communication/SOTClient.h"
#include "vector"
#include "list"

#include "TestUtil.h"
#include "MockMasterClient.hpp"

using namespace std;



TEST_CASE("MasterClient communication: client has receive overflow") {
  TestSOTMaster master;
  TestSOTClient client;

  // connect
  master.addAndConnectToClient(1);
  auto &masterProtocol = master.getClient(1).protocol;
  client.processCanFramesReceived(master.framesSend);
  master.processCanFramesReceived(client.framesSend);
  client.clearFramesSend();
  master.clearFramesSend();

  // check
  CHECK_FALSE(master.getClient(1).onCommunicationErrorTxOverflow);
  CHECK_FALSE(master.getClient(1).onCommunicationErrorRxOverflow);

  // change value on client
  client.getProtocol().objectTree.settings.value2.write(27);

  // master reads value client master
  masterProtocol.objectTree.settings.value2.sendReadValueReq();
  REQUIRE(master.framesSend.size() == 1);

  // set rx overflow on client
  TestSOTClient::onRxOverflow._triggerEvent();
  // let client answer
  client.processCanFramesReceived(master.framesSend);
  REQUIRE(client.framesSend.size() == 2);
  CHECK(client.getLastSendFrameType() == READ_NODE_VALUE_RESPONSE);
  CHECK(client.getSendFrameType(0) == COMMUNICATION_ERROR);
  CHECK(client.getSendFrameTypeAndIDs(0).targetDeviceId == 0);
  CHECK(client.getSendFrame(0).dataLength == 1);
  CHECK(client.getSendFrame(0).data[0] == static_cast<uint8_t>(COMMUNICATION_ERROR_TYPES::CAN_RECEIVE_OVERFLOW));
  master.clearFramesSend();

  // master process packages
  master.processCanFramesReceived(client.framesSend);
  REQUIRE(master.framesSend.size() == 0);

  CHECK_FALSE(masterProtocol.objectTree.settings.value1.receivedValueUpdate);
  CHECK(masterProtocol.objectTree.settings.value2.receivedValueUpdate);
  CHECK(masterProtocol.objectTree.settings.value2.read() == 27);
  // check
  CHECK_FALSE(master.getClient(1).onCommunicationErrorTxOverflow);
  CHECK(master.getClient(1).onCommunicationErrorRxOverflow);
}




TEST_CASE("MasterClient communication: client has send overflow") {
  TestSOTMaster master;
  TestSOTClient client;

  // connect
  master.addAndConnectToClient(1);
  auto &masterProtocol = master.getClient(1).protocol;
  client.processCanFramesReceived(master.framesSend);
  master.processCanFramesReceived(client.framesSend);
  client.clearFramesSend();
  master.clearFramesSend();

  // check
  CHECK_FALSE(master.getClient(1).onCommunicationErrorTxOverflow);
  CHECK_FALSE(master.getClient(1).onCommunicationErrorRxOverflow);

  // change value on client
  client.getProtocol().objectTree.settings.value2.write(27);

  // master reads value client master
  masterProtocol.objectTree.settings.value2.sendReadValueReq();
  REQUIRE(master.framesSend.size() == 1);

  // set tx overflow on client
  TestSOTClient::onTxOverflow._triggerEvent();
  // let client answer
  client.processCanFramesReceived(master.framesSend);
  REQUIRE(client.framesSend.size() == 2); // second package is overflow error
  CHECK(client.getSendFrameType(0) == READ_NODE_VALUE_RESPONSE);
  CHECK(client.getSendFrameType(1) == COMMUNICATION_ERROR);
  CHECK(client.getSendFrameTypeAndIDs(1).targetDeviceId == 0);
  CHECK(client.getSendFrame(1).dataLength == 1);
  CHECK(client.getSendFrame(1).data[0] == static_cast<uint8_t>(COMMUNICATION_ERROR_TYPES::CAN_SEND_OVERFLOW));
  master.clearFramesSend();

  // master process packages
  master.processCanFramesReceived(client.framesSend);
  REQUIRE(master.framesSend.size() == 0);

  CHECK_FALSE(masterProtocol.objectTree.settings.value1.receivedValueUpdate);
  CHECK(masterProtocol.objectTree.settings.value2.receivedValueUpdate);
  CHECK(masterProtocol.objectTree.settings.value2.read() == 27);
  // check
  CHECK(master.getClient(1).onCommunicationErrorTxOverflow);
  CHECK_FALSE(master.getClient(1).onCommunicationErrorRxOverflow);
}