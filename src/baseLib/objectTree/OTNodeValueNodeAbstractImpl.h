//#include "OTNode.h"

#include "DataConversion.h"

// THIS HAS TO BE A HEADER, OTHERWISE ON STM32 WE GET AN HARD FAULT WHEN CALLING ONE OF THESE FUNCTIONS

const uint8_t ValueNodeAbstract::getRequiredDataSizeInBytes() const {
  switch (dataType) {
    case VALUE_NODE_DATA_TYPES::UINT8:
      return 1;
    case VALUE_NODE_DATA_TYPES::UINT16:
      return 2;
    case VALUE_NODE_DATA_TYPES::F32:
      return 4;
    default:
     return 255;
  }
  return 255;
}

void ValueNodeAbstract::writeToData(uint8_t *data) {
  // @todo: handle endianness
  switch (dataType) {
    case VALUE_NODE_DATA_TYPES::UINT8: {
      auto *v = (ValueNodeTypeAbstract<TYPE_UINT8>*)this;
      writeToDataUINT8(*data, v->value);
      break;
    }
    case VALUE_NODE_DATA_TYPES::UINT16: {
      auto *v = (ValueNodeTypeAbstract<TYPE_UINT16>*)this;
      writeToDataUINT16(*data, v->value);
      break;
    }
    case VALUE_NODE_DATA_TYPES::F32:{
      auto *v = (ValueNodeTypeAbstract<TYPE_F32>*)this;
      writeToDataF32(*data, v->value);
      break;
    }
  }
}


void ValueNodeAbstract::readFromData(const uint8_t *data) {
  // @todo: handle endianness
  switch (dataType) {
    case VALUE_NODE_DATA_TYPES::UINT8: {
      auto *v = (ValueNodeTypeAbstract<TYPE_UINT8>*)this;
      readFromDataUINT8(*data, v->value);
      break;
    }
    case VALUE_NODE_DATA_TYPES::UINT16: {
      auto *v = (ValueNodeTypeAbstract<TYPE_UINT16>*)this;
      readFromDataUINT16(*data, v->value);
      break;
    }
    case VALUE_NODE_DATA_TYPES::F32:{
      auto *v = (ValueNodeTypeAbstract<TYPE_F32>*)this;
      readFromDataF32(*data, v->value);
      break;
    }
  }
  receivedValueUpdate._triggerEvent();
}

