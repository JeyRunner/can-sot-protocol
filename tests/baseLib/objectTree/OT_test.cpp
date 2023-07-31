#include "DocTestIncl.h"
#include "objectTree/OTNode.h"
#include "objectTree/OTNodeValueTypeDefs.h"
#include "communication/SOTClient.h"
#include "_example_generated_ot/SOTProtocol.hpp"

#include "TestUtil.h"


using SOTDummyClient = SOTClient<TestProtocol, CanInterface>;


TEST_CASE("test ValueNodeTypeAbstract") {
  SUBCASE("UInt8") {
    ValueNodeTypeAbstract<uint8_t> valueNode(12);
    valueNode.value = 1;
    CHECK(valueNode.dataType == VALUE_NODE_DATA_TYPES::UINT8);
    CHECK(valueNode.value == 1);
  }
  SUBCASE("UInt16") {
    ValueNodeTypeAbstract<TYPE_UINT16> valueNode(12);
    valueNode.value = 1;
    CHECK(valueNode.dataType == VALUE_NODE_DATA_TYPES::UINT16);
    CHECK(valueNode.value == 1);
  }
  SUBCASE("F32") {
    ValueNodeTypeAbstract<TYPE_F32> valueNode(12);
    valueNode.value = 1.2;
    CHECK(valueNode.dataType == VALUE_NODE_DATA_TYPES::F32);
    CHECK(valueNode.value == doctest::Approx(1.2));
  }
}





template<class TYPE> static void testObjectValueWriteRead(TYPE value) {
  uint8_t data[8] = {0};
  ValueNodeReadWriteable<TYPE, 0, SOTDummyClient> valueNode;
  ValueNodeReadWriteable<TYPE, 0, SOTDummyClient> valueNode2;
  valueNode.write(value);
  CHECK(valueNode.read() == value);

  valueNode.writeToData(data);
  std::cout << "-- data array: " << toString(data) << std::endl;

  //CHECK((data[0] != 0 || data[1] != 0));

  valueNode2.readFromData(data);
  CHECK(valueNode2.read() == value);
}

TEST_CASE("test object value to binary and back") {
  SUBCASE("UInt8") {
    testObjectValueWriteRead<TYPE_UINT8>(55);
  }
  SUBCASE("UInt16") {
    testObjectValueWriteRead<TYPE_UINT16>(55);
  }
  SUBCASE("F32") {
    testObjectValueWriteRead<TYPE_F32>(112.33);
  }
}