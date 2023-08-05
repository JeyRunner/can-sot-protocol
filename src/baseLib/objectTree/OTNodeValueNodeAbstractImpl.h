//#include "OTNode.h"

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
      *((uint8_t*) data) = v->value;
      break;
    }
    case VALUE_NODE_DATA_TYPES::UINT16: {
      auto *v = (ValueNodeTypeAbstract<TYPE_UINT16>*)this;
      *((uint16_t*) data) = v->value;
      break;
    }
    case VALUE_NODE_DATA_TYPES::F32:{
      auto *v = (ValueNodeTypeAbstract<TYPE_F32>*)this;
      *((float*) data) = v->value;
      break;
    }
  }
}


void ValueNodeAbstract::readFromData(const uint8_t *data) {
  // @todo: handle endianness
  switch (dataType) {
    case VALUE_NODE_DATA_TYPES::UINT8: {
      auto *v = (ValueNodeTypeAbstract<TYPE_UINT8>*)this;
       v->value = *((uint8_t*) data);
      break;
    }
    case VALUE_NODE_DATA_TYPES::UINT16: {
      auto *v = (ValueNodeTypeAbstract<TYPE_UINT16>*)this;
       v->value = *((uint16_t*) data);
      break;
    }
    case VALUE_NODE_DATA_TYPES::F32:{
      auto *v = (ValueNodeTypeAbstract<TYPE_F32>*)this;
      v->value = *((float*) data);
      break;
    }
  }
  receivedValueUpdate._triggerEvent();
}

