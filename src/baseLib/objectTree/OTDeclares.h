#pragma once

#include <cinttypes>
#include "OTNode.h"
#include "remoteCalls/RemoteCalls.h"

/*
struct OTNodeIDsTableEntry {
    NodeId nodeId;
    ValueNodeAbstract *valueNode;
};
*/


/// The index in the table is the node id
template<unsigned int size>
using OTNodeIDsTable = ValueNodeAbstract*[size]; //OTNodeIDsTableEntry[size];


template<class T>
ValueNodeAbstract *valueNodeAsAbstract(T &valueNode) {
  return (ValueNodeAbstract*) &valueNode;
}

template<class T>
inline RemoteCallCallerAbstract *valueRemoteCallCallerAsAbstract(T &remoteCall) {
  return (RemoteCallCallerAbstract*) &remoteCall;
}

template<class T>
inline RemoteCallCallableAbstract *valueRemoteCallCallableAsAbstract(T &remoteCall) {
  return (RemoteCallCallableAbstract*) &remoteCall;
}