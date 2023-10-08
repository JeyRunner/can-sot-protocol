#include "DocTestIncl.h"
#include <cstring>
#include "communication/SOTMaster.h"
#include "communication/SOTClient.h"
#include "vector"
#include "list"

#include "TestUtil.h"
#include "MockMasterClient.hpp"

using namespace std;




TEST_CASE("MasterClient communication: master does remote call to client") {
  TestSOTMaster master;
  TestSOTClient client;

  // connect
  master.addAndConnectToClient(1);
  auto &masterProtocol = master.getClient(1).protocol;
  client.processCanFramesReceived(master.framesSend);
  master.processCanFramesReceived(client.framesSend);
  client.clearFramesSend();
  master.clearFramesSend();

  // check precond
  CHECK_FALSE(client.getProtocol().remoteCalls.callable.testFunc.remoteCallCalled);
  CHECK(client.getProtocol().remoteCalls.callable.testFunc.argumentsData.arg1 == 0);
  CHECK(client.getProtocol().remoteCalls.callable.testFunc.argumentsData.arg2 == 0);

  // master send remote call
  masterProtocol.remoteCalls.caller.testFunc.sendCall({22.3, 68});
  //masterProtocol.remoteCalls.caller.testFunc.sendCall({.arg1=22.3, .arg2=68});

  REQUIRE(master.framesSend.size() == 1);
  CHECK(master.getLastSendFrameType() == REMOTE_CALL_REQUEST);
  CHECK(master.getLastSendFrame().dataLength == 5+1);

  // receive on client
  client.processCanFramesReceived(master.framesSend);
  REQUIRE(client.framesSend.size() == 0);
  CHECK(client.getProtocol().remoteCalls.callable.testFunc.remoteCallCalled);
  CHECK(client.getProtocol().remoteCalls.callable.testFunc.argumentsData.arg1 == doctest::Approx(22.3));
  CHECK(client.getProtocol().remoteCalls.callable.testFunc.argumentsData.arg2 == 68);
}




TEST_CASE("MasterClient communication: client does return from remote call [OK] -> send return data to master") {
  TestSOTMaster master;
  TestSOTClient client;

  // connect
  master.addAndConnectToClient(1);
  auto &masterProtocol = master.getClient(1).protocol;
  client.processCanFramesReceived(master.framesSend);
  master.processCanFramesReceived(client.framesSend);
  client.clearFramesSend();
  master.clearFramesSend();

  // check precond
  CHECK_FALSE(masterProtocol.remoteCalls.caller.testFunc.remoteCallReturned);
  CHECK(masterProtocol.remoteCalls.caller.testFunc.callReturnData.returnData.data1 == 0);
  CHECK(masterProtocol.remoteCalls.caller.testFunc.callReturnData.returnData.data2 == 0);
  CHECK(masterProtocol.remoteCalls.caller.testFunc.callReturnData.returnData.data3 == 0);
  CHECK(masterProtocol.remoteCalls.caller.testFunc.callReturnData.isError == false);

  // client returns: send return data to master
  client.getProtocol().remoteCalls.callable.testFunc.sendReturnOk({42, 34, 6541});

  REQUIRE(client.framesSend.size() == 1);
  CHECK(client.getLastSendFrameType() == REMOTE_CALL_RETURN);
  CHECK(client.getLastSendFrame().dataLength == 1 + 4);

  // receive on master
  master.processCanFramesReceived(client.framesSend);
  REQUIRE(master.framesSend.size() == 0);
  CHECK(masterProtocol.remoteCalls.caller.testFunc.remoteCallReturned);
  CHECK(masterProtocol.remoteCalls.caller.testFunc.callReturnData.returnData.data1 == 42);
  CHECK(masterProtocol.remoteCalls.caller.testFunc.callReturnData.returnData.data2 == 34);
  CHECK(masterProtocol.remoteCalls.caller.testFunc.callReturnData.returnData.data3 == 6541);
  CHECK(masterProtocol.remoteCalls.caller.testFunc.callReturnData.isError == false);
  bool handleFuncCalled = false;
  masterProtocol.remoteCalls.caller.testFunc.handleCallReturned([&](RemoteCallReturn<TestFuncReturnDataCaller, TEST_ENUM> agrs) {
    handleFuncCalled = true;
  });
  CHECK(handleFuncCalled);
}



TEST_CASE("MasterClient communication: client does return from remote call [Error] -> send return data to master") {
    TestSOTMaster master;
    TestSOTClient client;

    // connect
    master.addAndConnectToClient(1);
    auto &masterProtocol = master.getClient(1).protocol;
    client.processCanFramesReceived(master.framesSend);
    master.processCanFramesReceived(client.framesSend);
    client.clearFramesSend();
    master.clearFramesSend();
    masterProtocol.remoteCalls.caller.testFunc.callReturnData.returnData = {};

    // check precond
    CHECK_FALSE(masterProtocol.remoteCalls.caller.testFunc.remoteCallReturned);
    CHECK(masterProtocol.remoteCalls.caller.testFunc.callReturnData.returnData.data1 == 0);
    CHECK(masterProtocol.remoteCalls.caller.testFunc.callReturnData.returnData.data2 == 0);
    CHECK(masterProtocol.remoteCalls.caller.testFunc.callReturnData.returnData.data3 == 0);
    CHECK(masterProtocol.remoteCalls.caller.testFunc.callReturnData.returnError == 0);
    CHECK(masterProtocol.remoteCalls.caller.testFunc.callReturnData.isError == false);

    // client returns: send return data to master
    client.getProtocol().remoteCalls.callable.testFunc.sendReturnError(TEST_ENUM::B);

    REQUIRE(client.framesSend.size() == 1);
    CHECK(client.getLastSendFrameType() == REMOTE_CALL_RETURN);
    CHECK(client.getLastSendFrame().dataLength == 1 + 1);

    // receive on master
    master.processCanFramesReceived(client.framesSend);
    REQUIRE(master.framesSend.size() == 0);
    CHECK(masterProtocol.remoteCalls.caller.testFunc.remoteCallReturned);
    CHECK(masterProtocol.remoteCalls.caller.testFunc.callReturnData.returnData.data1 == 0);
    CHECK(masterProtocol.remoteCalls.caller.testFunc.callReturnData.returnData.data2 == 0);
    CHECK(masterProtocol.remoteCalls.caller.testFunc.callReturnData.returnData.data3 == 0);
    CHECK(masterProtocol.remoteCalls.caller.testFunc.callReturnData.returnError == TEST_ENUM::B);
    CHECK(masterProtocol.remoteCalls.caller.testFunc.callReturnData.isError == true);
}
