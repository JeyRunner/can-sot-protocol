#pragma once

#include "objectTree/OTNodeValueTypes.h"
#include "objectTree/OTDeclares.h"

// if C++20
#if __cplusplus > 201703L
#include <concepts> // std::same_as

template <typename T, uint8_t OT_TABLE_SIZE>
concept ProtocolDefType = requires(T) {
  {T::objectTree} -> std::same_as<OTNodeIDsTable<OT_TABLE_SIZE>>;
  {T::metaNodeValuesToSendOnInit} -> std::same_as<ValueNodeAbstract*[]>;
};
#endif


template<unsigned int OT_TABLE_SIZE, unsigned int INIT_NODES_SIZE>
struct ProtocolDef {
    uint8_t otTableSize = OT_TABLE_SIZE;

    /// the index is the node id belonging to the referenced node value
    OTNodeIDsTable<OT_TABLE_SIZE>  otNodeIDsTable = {};


    /**
     * meta data objects that are send to master on communication initialization.
     * These are generally read only.
     */
    ValueNodeAbstract* metaNodeValuesToSendOnInit[INIT_NODES_SIZE] = {};
};


template<unsigned int OT_TABLE_SIZE, unsigned int INIT_NODES_SIZE>
struct Protocol {
    virtual ProtocolDef<OT_TABLE_SIZE,INIT_NODES_SIZE> getProtocolDef() {
    }
};




struct MyProtocol: public Protocol<1,1> {
    ValueNodeReadWriteable<TYPE_UINT8, 0> nodeVal1;


    ProtocolDef<1,1> getProtocolDef() override {
      return ProtocolDef<1,1> {
        .otNodeIDsTable = {
            valueNodeAsAbstract(nodeVal1)
        },
        .metaNodeValuesToSendOnInit = {
            valueNodeAsAbstract(nodeVal1)
        }
      };
    }
};
