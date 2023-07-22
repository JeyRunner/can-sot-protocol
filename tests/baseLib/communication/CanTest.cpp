#include <doctest/doctest.h>
#include <cstring>
#include "communication/SOTMaster.h"
#include "communication/SOTClient.h"
#include "vector"
#include "list"

#include "TestUtil.h"
#include "MockMasterClient.hpp"

using namespace std;


TEST_CASE("Pack/Unpack Can frame ID") {
  CanFrame frame;
  DeviceIdAndSOTMessageType idAndSotMessageType{
    .sourceDeviceId = 3,
    .targetDeviceId = 6,
    .messageType = READ_NODE_VALUE_RESPONSE
  };

  CHECK(frame.canId == 0);
  packCanFrameId(frame, idAndSotMessageType);
  CHECK(frame.canId != 0);

  auto idUnpacked = unpackCanFrameId(frame);
  CHECK(idUnpacked.sourceDeviceId == idAndSotMessageType.sourceDeviceId);
  CHECK(idUnpacked.targetDeviceId == idAndSotMessageType.targetDeviceId);
  CHECK(idUnpacked.messageType == idAndSotMessageType.messageType);
}