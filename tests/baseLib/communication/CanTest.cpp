#include "DocTestIncl.h"
#include <cstring>
#include "communication/SOTMaster.h"
#include "communication/SOTClient.h"
#include "vector"
#include "list"

#include "TestUtil.h"
#include "MockMasterClient.hpp"

using namespace std;


TEST_CASE("Pack/Unpack Can frame ID") {
  uint8_t sourceDeviceId;
  uint8_t targetDeviceId;
  SOT_MESSAGE_TYPE messageType;

  // sub testcases with different params
  SUBCASE("a") { sourceDeviceId = 0;  targetDeviceId = 1;  messageType = READ_NODE_VALUE_RESPONSE; }
  SUBCASE("b") { sourceDeviceId = 5;  targetDeviceId = 7;  messageType = INIT_COMMUNICATION_REQUEST; }
  SUBCASE("c") { sourceDeviceId = 2;  targetDeviceId = 6;  messageType = WRITE_NODE_VALUE_REQEUST; }

  // capture for parameterized test
  CAPTURE(sourceDeviceId);
  CAPTURE(targetDeviceId);
  CAPTURE(messageType);



  CanFrame frame;
  DeviceIdAndSOTMessageType idAndSotMessageType{
          .sourceDeviceId = sourceDeviceId,
          .targetDeviceId = targetDeviceId,
          .messageType = messageType
  };

  CHECK(frame.canId == 0);
  packCanFrameId(frame, idAndSotMessageType);
  CHECK(frame.canId != 0);

  auto idUnpacked = unpackCanFrameId(frame);
  CHECK(idUnpacked.sourceDeviceId == idAndSotMessageType.sourceDeviceId);
  CHECK(idUnpacked.targetDeviceId == idAndSotMessageType.targetDeviceId);
  CHECK(idUnpacked.messageType == idAndSotMessageType.messageType);
}