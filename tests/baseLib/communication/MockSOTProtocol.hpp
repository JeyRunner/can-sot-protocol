#include <doctest/doctest.h>
#include <cstring>
#include <utility>
#include "communication/SOTMaster.h"
#include "communication/SOTClient.h"
#include "vector"
#include "list"
#include "_example_generated_ot/SOTProtocolRemoteCallArgs.hpp"

#include "TestUtil.h"

using namespace std;


enum TEST_ENUM: TYPE_UINT8 {
    A,
    B
};


/// Mock protocol, here just for testing the same struct is used for client and master
template<typename COMC>
struct MockTestProtocol: public ProtocolDef<COMC, 2, 2, 2, 2> {

    struct MyObjectTree: public Node {
        struct Settings: Node {
            ValueNodeReadWriteable<TYPE_UINT16, 0, COMC> value1;
            ValueNodeReadWriteable<TYPE_UINT16, 1, COMC>  value2;
        } settings;
        ValueNodeReadWriteable<TYPE_F32, 2, COMC> value3ThatIsSendOnInit;
    } objectTree;

    struct RemoteCalls {
        struct Caller {
            RemoteCallCaller<0, TestFuncArgDataCaller, TestFuncReturnDataCaller, TEST_ENUM, COMC> testFunc;
        } caller;
        struct Callable {
            RemoteCallCallable<0, TestFuncArgDataCallable, TestFuncReturnDataCallable, TEST_ENUM, COMC> testFunc;
        } callable;
    } remoteCalls;

    static const uint8_t OT_TABLE_SIZE = 3;
    uint8_t otTableSize = OT_TABLE_SIZE;

    /// the index is the node ID belonging to the referenced node value
    OTNodeIDsTable<OT_TABLE_SIZE>  otNodeIDsTable = {
            valueNodeAsAbstract(objectTree.settings.value1),
            valueNodeAsAbstract(objectTree.settings.value2),
            valueNodeAsAbstract(objectTree.value3ThatIsSendOnInit),
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
    ValueNodeAbstract* metaNodeValuesToSendOnInit[1] = {
            valueNodeAsAbstract(objectTree.value3ThatIsSendOnInit)
    };


    explicit MockTestProtocol(COMC *sotCanCommunication)
    : ProtocolDef<COMC, 2, 2, 2, 2>(sotCanCommunication) {
        objectTree.settings.value1.__setProtocolRef(sotCanCommunication);
        objectTree.settings.value2.__setProtocolRef(sotCanCommunication);
        objectTree.value3ThatIsSendOnInit.__setProtocolRef(sotCanCommunication);
        remoteCalls.caller.testFunc.__setProtocolRef(sotCanCommunication);
        remoteCalls.callable.testFunc.__setProtocolRef(sotCanCommunication);
    };
};