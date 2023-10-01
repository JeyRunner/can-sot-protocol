#pragma once

#include "objectTree/OTNodeValueTypes.h"
#include "objectTree/OTDeclares.h"
#include "remoteCalls/RemoteCalls.h"

template<template <class T> class PROTOCOL_DEF /* = ProtocolDef<_DummpProtocl, 1,1>*/, class CAN_INTERFACE_CLASS>
class SOTCanCommunication;


// if C++20
#if __cplusplus > 201703L
#include <concepts> // std::same_as
/*
template <typename T, uint8_t OT_TABLE_SIZE>
concept ProtocolDefType = requires(T) {
  {T::objectTree} -> std::same_as<OTNodeIDsTable<OT_TABLE_SIZE>>;
  {T::metaNodeValuesToSendOnInit} -> std::same_as<ValueNodeAbstract*[]>;
};
 */
#endif


/**
 * Generated protocols will extend from this class,
 */
template<typename COMMUNICATION_CLASS, unsigned int OT_TABLE_SIZE, unsigned int INIT_NODES_SIZE, uint8_t RCCALLER_TABLE_SIZE, uint8_t RCCALLABLE_TABLE_SIZE>
struct ProtocolDef {
    uint8_t otTableSize = OT_TABLE_SIZE;
    uint8_t rcCallerTableSize = RCCALLER_TABLE_SIZE;
    uint8_t rcCallableTableSize = RCCALLABLE_TABLE_SIZE;

    /// the index is the node id belonging to the referenced node value
    OTNodeIDsTable<OT_TABLE_SIZE>  otNodeIDsTable = {};


    /**
     * meta data objects that are send to master on communication initialization.
     * These are generally read only.
     */
    ValueNodeAbstract* metaNodeValuesToSendOnInit[INIT_NODES_SIZE] = {};

    /// Remote calls that can be called from this device
    /// the index is the call id belonging to the referenced remote call
    RemoteCallCallerAbstract *rcCallerTable[RCCALLER_TABLE_SIZE] = {};

    /// Remote calls that are callable on this device. The caller will be another device.
    /// the index is the call id belonging to the referenced remote call
    RemoteCallCallerAbstract *rcCallableTable[RCCALLABLE_TABLE_SIZE] = {};


    COMMUNICATION_CLASS &sotCanCommunication;


    /**
     * Send value of value node to the remote master or client.
     * This will add the corresponding send package into the can send buffer.
     */
    inline void sendValue(ValueNodeAbstract &vNode) {
        sotCanCommunication.sendValue(vNode);
    }

    /**
     * Send a request for reading the current value of this node from a remote client or server.
     * When the new read value arrives it will directly overwrite the current value of this node.
     * This will put the corresponding can frame directly into the can send buffer.
     * @note it will take time for the new value to arrive, so the new value will not be immediately available.
     */
    inline void sendReadValueReq(ValueNodeAbstract &vNode) {
        sotCanCommunication.sendReadValueReq(vNode);
    }


    ProtocolDef() = delete;
    //ProtocolDef(int i, COMMUNICATION_CLASS *sotCanCommunication)
    //: sotCanCommunication(*sotCanCommunication) {};
    ProtocolDef(COMMUNICATION_CLASS *sotCanCommunication)
            : sotCanCommunication(*sotCanCommunication) {};
    ProtocolDef(const ProtocolDef&) = delete;
    ProtocolDef(ProtocolDef&&) = delete;
    ProtocolDef& operator=(const ProtocolDef&) = delete;
    ProtocolDef& operator=(ProtocolDef&&) = delete;
};


/// just used for testing
template<typename COMMUNICATION_CLASS>
struct _DummpProtocl: public ProtocolDef<COMMUNICATION_CLASS, 0, 0, 0, 0>
{};





/*
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
*/