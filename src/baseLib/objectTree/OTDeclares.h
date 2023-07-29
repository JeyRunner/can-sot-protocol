#pragma once

#include <cinttypes>
#include "OTNode.h"

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