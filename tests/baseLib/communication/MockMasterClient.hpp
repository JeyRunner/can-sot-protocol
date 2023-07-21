#include <doctest/doctest.h>
#include <cstring>
#include "communication/SOTMaster.h"
#include "communication/SOTClient.h"
#include "vector"
#include "list"

#include "TestUtil.h"

using namespace std;


struct TestProtocol: public ProtocolDef<2,2> {

    struct MyObjectTree: public Node {
        struct Settings: Node {
            ValueNodeReadable<TYPE_UINT16, 1> value1;
            ValueNodeReadable<TYPE_UINT16, 2>  value2;
        } settings;
        ValueNodeReadable<TYPE_F32, 3> value3;
    } objectTree;

    static const uint8_t OT_TABLE_SIZE = 3;
    uint8_t otTableSize = OT_TABLE_SIZE;

    /// the index is the node id belonging to the referenced node value
    OTNodeIDsTable<OT_TABLE_SIZE>  otNodeIDsTable = {
            valueNodeAsAbstract(objectTree.settings.value1),
            valueNodeAsAbstract(objectTree.settings.value2),
            valueNodeAsAbstract(objectTree.value3),
    };


    /**
     * meta data objects that are send to master on communication initialization.
     * These are generally read only.
     */
    ValueNodeAbstract* metaNodeValuesToSendOnInit[1] = {
            valueNodeAsAbstract(objectTree.value3)
    };
};

/*
struct CanFrameWithData {
    CanFrame frame;
    uint8_t data[8];
    static CanFrameWithData createFromFrame(CanFrame &frame) {

    }
};
*/

class TestSOTMaster: public SOTMaster<TestProtocol> {
public:
    list<array<uint8_t, 8>> sendDataBuffer;
    vector<CanFrame> framesSend;

    void canSendFrame(CanFrame &frame) override {
        uint8_t data[frame.dataLength];
        auto &dataPtr = sendDataBuffer.emplace_back(array<uint8_t, 8>{});
        memcpy(data, dataPtr.data(), sizeof(uint8_t)*framesSend.size());
        framesSend.emplace_back(CanFrame{
                .canId = frame.canId,
                .data = dataPtr.data(),
                .dataLength = frame.dataLength,
        });
    }

    CanFrame &getSendFrame(int frameIndex) {
        return framesSend.at(frameIndex);
    }

    void clearFramesSend() {
        framesSend.clear();
    }
};


class TestSOTClient: public SOTClient<TestProtocol> {
public:
    list<array<uint8_t, 8>> sendDataBuffer;
    vector<CanFrame> framesSend;

    void canSendFrame(CanFrame &frame) override {
        cout << "client send Frame: id: " << frame.canId << " data[0]: " << (frame.dataLength > 0 ? frame.data[0] : 0) << endl;
        uint8_t data[frame.dataLength];
        auto &dataPtr = sendDataBuffer.emplace_back(array<uint8_t, 8>{});
        memcpy(data, dataPtr.data(), sizeof(uint8_t)*framesSend.size());
        framesSend.emplace_back(CanFrame{
                .canId = frame.canId,
                .data = dataPtr.data(),
                .dataLength = frame.dataLength,
        });
    }

    CanFrame &getSendFrame(int frameIndex) {
        return framesSend.at(frameIndex);
    }

    void clearFramesSend() {
        framesSend.clear();
    }
};
