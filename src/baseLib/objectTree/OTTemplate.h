#pragma once

#include "objectTree/OTNodeValueTypes.h"
#include "objectTree/OTDeclares.h"



/// the index is the node id belonging to the referenced node value
OTNodeIDsTable<0> __attribute__((weak)) otNodeIDsTable = {};


/**
 * meta data objects that are send to master on communication initialization.
 * These are generally read only.
 */
ValueNodeAbstract* metaNodeValuesToSendOnInit[0] = {};