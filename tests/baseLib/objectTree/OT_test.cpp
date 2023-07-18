#include <doctest/doctest.h>
#include "objectTree/OTNode.h"
#include "objectTree/OTNodeValueTypeDefs.h"

#include "TestUtil.h"



TEST_CASE("test ValueNodeTypeAbstract") {
  SUBCASE("UInt16") {
    ValueNodeTypeAbstract<uint16_t> valueNode(12);
    valueNode.value = 1;
    CHECK(valueNode.dataType == VALUE_NODE_DATA_TYPES::UINT16);
    CHECK(valueNode.value == 1);
  }
}





template<class TYPE> static void testObjectValueWriteRead(TYPE value) {
  uint8_t data[8] = {0};
  ValueNodeReadWriteable<TYPE, 0> valueNode;
  ValueNodeReadWriteable<TYPE, 0> valueNode2;
  valueNode.write(55);
  CHECK(valueNode.read() == 55);

  valueNode.writeToData(data);
  std::cout << "-- data array: " << toString(data) << std::endl;

  //CHECK((data[0] != 0 || data[1] != 0));

  valueNode2.readFromData(data);
  CHECK(valueNode2.read() == 55);
}

TEST_CASE("test object value to binary and back") {
  SUBCASE("UInt8") {
    testObjectValueWriteRead<TYPE_UINT8>(55);
  }
  SUBCASE("UInt16") {
    testObjectValueWriteRead<TYPE_UINT16>(55);
  }
  SUBCASE("UF32") {
    testObjectValueWriteRead<TYPE_F32>(112.33);
  }
}