#include <doctest/doctest.h>
#include <cstring>
#include <utility>
#include "communication/SOTMaster.h"
#include "communication/SOTClient.h"
#include "vector"
#include "list"

#include "TestUtil.h"

using namespace std;

/// Mock protocol, here just for testing the same struct is used for client and master
template<typename COMMUNICATION_CLASS>
struct MockTestProtocol: public ProtocolDef<COMMUNICATION_CLASS, 2,2> {

    struct MyObjectTree: public Node {
        struct Settings: Node {
            ValueNodeReadWriteable<TYPE_UINT16, 0> value1;
            ValueNodeReadWriteable<TYPE_UINT16, 1>  value2;
        } settings;
        ValueNodeReadWriteable<TYPE_F32, 2> value3ThatIsSendOnInit;
    } objectTree;

    static const uint8_t OT_TABLE_SIZE = 3;
    uint8_t otTableSize = OT_TABLE_SIZE;

    /// the index is the node ID belonging to the referenced node value
    OTNodeIDsTable<OT_TABLE_SIZE>  otNodeIDsTable = {
            valueNodeAsAbstract(objectTree.settings.value1),
            valueNodeAsAbstract(objectTree.settings.value2),
            valueNodeAsAbstract(objectTree.value3ThatIsSendOnInit),
    };


    /**
     * meta data objects that are send to master on communication initialization.
     * These are generally read only.
     */
    ValueNodeAbstract* metaNodeValuesToSendOnInit[1] = {
            valueNodeAsAbstract(objectTree.value3ThatIsSendOnInit)
    };


    explicit MockTestProtocol(COMMUNICATION_CLASS *sotCanCommunication)
    : ProtocolDef<COMMUNICATION_CLASS, 2, 2>(sotCanCommunication) {};
};