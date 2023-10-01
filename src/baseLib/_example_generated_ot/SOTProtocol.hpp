#pragma once

//#include "objectTree/OTTemplate.h"
#include "objectTree/OTNodeValueTypeDefs.h"
#include "objectTree/OTDeclares.h"
#include "objectTree/ProtocolDef.h"
#include "remoteCalls/RemoteCalls.h"

#include "SOTProtocolRemoteCallArgs.hpp"

template<typename COMC>
struct TestProtocol: public ProtocolDef<COMC, 8, 1, 2, 2> {
    enum TEST_ENUM: TYPE_UINT8 {
        A,
        B
    };

    struct MyObjectTree : public Node {
        struct _Meta : Node {
            ValueNodeReadWriteable<TYPE_UINT8, 0, COMC> protocolVersion;
        } _meta;



        struct Settings : Node {
            ValueNodeReadWriteable<TYPE_UINT16, 1, COMC> value1;
            ValueNodeReadWriteable<TYPE_UINT16, 2, COMC> value2;

            //ValueNodeReadable<long, 0> testShouldNotCompile; /// should not compile

            struct SubSettings : Node {
                ValueNodeReadWriteable<TYPE_F32, 3, COMC> value3;
            } subSettings;

        } settings;


        struct Debug : Node {
            ValueNodeReadWriteable<TYPE_UINT16, 4, COMC> clientRxBufferNumPackages;
            ValueNodeReadWriteable<TYPE_UINT16, 5, COMC> clientTxBufferNumPackages;
            ValueNodeReadWriteable<TYPE_F32, 6, COMC> clientProcessPackagesDurationMs;
        } debug;

        ValueNodeReadWriteableEnum<TEST_ENUM, 7, COMC> valueEnum;
    } objectTree;

    struct RemoteCalls {
        struct Caller {
            RemoteCallCaller<0, TestFuncArgDataCaller, TestFuncReturnDataCaller, TEST_ENUM, COMC> testFunc;
        } caller;
        struct Callable {
            RemoteCallCallable<0, TestFuncArgDataCallable, TestFuncReturnDataCallable, TEST_ENUM, COMC> testFunc;
        } callable;
    } remoteCalls;


    /// the index is the node id belonging to the referenced node value
    OTNodeIDsTable<8> otNodeIDsTable = {
            valueNodeAsAbstract(objectTree._meta.protocolVersion),
            valueNodeAsAbstract(objectTree.settings.value1),
            valueNodeAsAbstract(objectTree.settings.value2),
            valueNodeAsAbstract(objectTree.settings.subSettings.value3),
            valueNodeAsAbstract(objectTree.debug.clientRxBufferNumPackages),
            valueNodeAsAbstract(objectTree.debug.clientTxBufferNumPackages),
            valueNodeAsAbstract(objectTree.debug.clientProcessPackagesDurationMs),
            valueNodeAsAbstract(objectTree.valueEnum),
    };


    RemoteCallCallerAbstract *rcCallerTable[2] = {
            valueRemoteCallCallerAsAbstract(remoteCalls.caller.testFunc)
    };

    RemoteCallCallableAbstract *rcCallableTable[2] = {
            valueRemoteCallCallableAsAbstract(remoteCalls.callable.testFunc)
    };




    /**
     * meta data objects that are send to master on communication initialization.
     * These are generally read only.
     */
    ValueNodeAbstract *metaNodeValuesToSendOnInit[1] = {
            valueNodeAsAbstract(objectTree._meta.protocolVersion),
    };


    explicit TestProtocol(COMC *sotCanCommunication)
    : ProtocolDef<COMC, 8, 1, 2, 2>(sotCanCommunication) {
        // setup all nodevalues
        //valueNodeTypeAbstractWithProt_setProtocolRef(objectTree.testNode, sotCanCommunication);
        //objectTree.testNode.__setProtocolRef(sotCanCommunication);
        objectTree._meta.protocolVersion.__setProtocolRef(sotCanCommunication);
        objectTree.settings.value1.__setProtocolRef(sotCanCommunication);
        objectTree.settings.value2.__setProtocolRef(sotCanCommunication);
        objectTree.settings.subSettings.value3.__setProtocolRef(sotCanCommunication);
        objectTree.debug.clientRxBufferNumPackages.__setProtocolRef(sotCanCommunication);
        objectTree.debug.clientTxBufferNumPackages.__setProtocolRef(sotCanCommunication);
        objectTree.debug.clientProcessPackagesDurationMs.__setProtocolRef(sotCanCommunication);
        objectTree.valueEnum.__setProtocolRef(sotCanCommunication);
        remoteCalls.caller.testFunc.__setProtocolRef(sotCanCommunication);
        remoteCalls.callable.testFunc.__setProtocolRef(sotCanCommunication);
    };
};
