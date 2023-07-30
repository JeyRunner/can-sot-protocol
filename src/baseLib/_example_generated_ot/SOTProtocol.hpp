#pragma once

//#include "objectTree/OTTemplate.h"
#include "objectTree/OTNodeValueTypeDefs.h"
#include "objectTree/OTDeclares.h"
#include "objectTree/ProtocolDef.h"

template<typename COMC>
struct TestProtocol: public ProtocolDef<COMC, 3,1> {
    struct MyObjectTree : public Node {
        struct _Meta : Node {
            ValueNodeReadable<TYPE_UINT8, 0, COMC> protocolVersion;
        } _meta;

        struct Settings : Node {
            ValueNodeReadable<TYPE_UINT16, 1, COMC> value1;
            ValueNodeReadable<TYPE_UINT16, 2, COMC> value2;

            //ValueNodeReadable<long, 0> testShouldNotCompile; /// should not compile

            struct SubSettings : Node {
                ValueNodeReadable<TYPE_F32, 0, COMC> value3;
            } subSettings;

        } settings;

        //ValueNodeTypeAbstractWithProt<TYPE_F32, COMC> testNode = 1;
    } objectTree;


    /// the index is the node id belonging to the referenced node value
    OTNodeIDsTable<3> otNodeIDsTable = {
            valueNodeAsAbstract(objectTree.settings.value1),
            valueNodeAsAbstract(objectTree.settings.value2),
            valueNodeAsAbstract(objectTree.settings.subSettings.value3),
    };


    /**
     * meta data objects that are send to master on communication initialization.
     * These are generally read only.
     */
    ValueNodeAbstract *metaNodeValuesToSendOnInit[1] = {
            valueNodeAsAbstract(objectTree._meta.protocolVersion),
    };


    explicit TestProtocol(COMC *sotCanCommunication)
    : ProtocolDef<COMC, 3,1>(sotCanCommunication) {
        // setup all nodevalues
        //valueNodeTypeAbstractWithProt_setProtocolRef(objectTree.testNode, sotCanCommunication);
        //objectTree.testNode.__setProtocolRef(sotCanCommunication);
        objectTree._meta.protocolVersion.__setProtocolRef(sotCanCommunication);
        objectTree.settings.value1.__setProtocolRef(sotCanCommunication);
        objectTree.settings.value2.__setProtocolRef(sotCanCommunication);
    };
};
