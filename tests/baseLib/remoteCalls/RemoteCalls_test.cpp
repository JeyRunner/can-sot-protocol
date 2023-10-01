#include "DocTestIncl.h"
#include "objectTree/OTNode.h"
#include "objectTree/OTNodeValueTypeDefs.h"
#include "communication/SOTClient.h"
#include "_example_generated_ot/SOTProtocol.hpp"
#include "_example_generated_ot/SOTProtocolRemoteCallArgs.hpp"

#include "TestUtil.h"


using SOTDummyClient = SOTClient<TestProtocol, CanInterface>;





TEST_CASE("test remote call arguments to binary and back") {
  TestFuncArgDataCaller argDataCaller(10.5f, 33);
  TestFuncArgDataCallable argDataCallable;

  uint8_t data[8];
  argDataCaller._writeToData(data);
  argDataCallable._readFromData(data);

  CHECK(argDataCaller.arg1 == argDataCallable.arg1);
  CHECK(argDataCaller.arg2 == argDataCallable.arg2);
}


TEST_CASE("test remote call return values to binary and back") {
  TestFuncReturnDataCaller argDataCaller(12, 33, 54681);
  TestFuncReturnDataCallable argDataCallable;

  uint8_t data[8];
  argDataCallable._writeToData(data);
  argDataCaller._readFromData(data);

  CHECK(argDataCaller.data1 == argDataCallable.data1);
  CHECK(argDataCaller.data2 == argDataCallable.data2);
  CHECK(argDataCaller.data3 == argDataCallable.data3);
}