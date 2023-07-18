#pragma once

//#include "objectTree/OTTemplate.h"
#include "objectTree/OTNodeValueTypeDefs.h"
#include "objectTree/OTDeclares.h"


struct MyObjectTree: public Node {
    struct _Meta: Node {
        ValueNodeReadable<TYPE_UINT8, 0> protocolVersion;
    } _meta;

    struct Settings: Node {
        ValueNodeReadable<TYPE_UINT16, 1> value1;
        ValueNodeReadable<TYPE_UINT16, 2>  value2;

        //ValueNodeReadable<long, 0> testShouldNotCompile; /// should not compile

        struct SubSettings: Node {
            ValueNodeReadable<TYPE_F32, 0> value3;
        } subSettings;

    } settings;

} myObjectTree;


/// the index is the node id belonging to the referenced node value
OTNodeIDsTable<3> otNodeIDsTable = {
    valueNodeAsAbstract(myObjectTree.settings.value1),
    valueNodeAsAbstract(myObjectTree.settings.value2),
    valueNodeAsAbstract(myObjectTree.settings.subSettings.value3),
};


/**
 * meta data objects that are send to master on communication initialization.
 * These are generally read only.
 */
ValueNodeAbstract* metaNodeValuesToSendOnInit[1] = {
    valueNodeAsAbstract(myObjectTree._meta.protocolVersion),
};