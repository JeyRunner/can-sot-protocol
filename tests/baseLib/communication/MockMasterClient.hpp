#include <doctest/doctest.h>
#include <cstring>
#include <utility>
#include "communication/SOTMaster.h"
#include "communication/SOTClient.h"
#include "driver_template/DriverTemplate.hpp"
#include "vector"
#include "array"
#include <algorithm>

#include "TestUtil.h"
#include "MockSOTProtocol.hpp"
#include "communication/threaded/SOTMasterLockable.h"

using namespace std;


static SOT_MESSAGE_TYPE getMessageType(CanFrame &frame) {
  return unpackCanFrameId(frame).messageType;
}

struct CanFrameWithData {
    CanFrame frame;
    uint8_t data[8];
};


constexpr size_t BUFFER_MAX_SIZE = 255;

class MockCanBuffer : public CanInterface {
  public:
    list<CanFrameWithData> framesSend;
    list<CanFrameWithData> framesReceived;

    explicit MockCanBuffer(string name) : name(std::move(name)) {
    }


    bool canSendFrame(CanFrame &frame, bool frameIsOverflowError = false) override {
      putFrameInSendBuffer(frame);
      return true;
    }

    bool getNextCanFrameReceived(CanFrame &receiveFrame) override {
      if (framesReceived.empty()) {
        return false;
      }
      receiveFrame = framesReceived.front().frame;
      framesReceived.pop_front();
      //cout << "can interface: get next frame rec" << endl;
      return true;
    }


    CanFrame &getSendFrame(int frameIndex) {
      return std::next(framesSend.begin(), frameIndex)->frame;
    }

    SOT_MESSAGE_TYPE getSendFrameType(int frameIndex) {
      return getMessageType(getSendFrame(frameIndex));
    }

    DeviceIdAndSOTMessageType getSendFrameTypeAndIDs(int frameIndex) {
      return unpackCanFrameId(getSendFrame(frameIndex));
    }

    CanFrame &getLastSendFrame() {
      return framesSend.back().frame;
    }

    SOT_MESSAGE_TYPE getLastSendFrameType() {
      return getMessageType(framesSend.back().frame);
    }

    DeviceIdAndSOTMessageType getLastSendFrameTypeAndIDs() {
      return unpackCanFrameId(framesSend.back().frame);
    }

    void clearFramesSend() {
      framesSend.clear();
    }

  protected:
    const string name;

    void putFrameInSendBuffer(CanFrame &frame) {
      cout << "[" << name << "]" << " send Frame with id: " << frame.canId << "  data(" << (int) frame.dataLength
           << "B)"
           << (frame.dataLength > 0 ? ": " : "")
           << byteArrayToString(frame.data, frame.dataLength) << endl;
      auto &newF = framesSend.emplace_back();
      if (frame.dataLength > 0) {
        memcpy(newF.data, frame.data, sizeof(uint8_t) * frame.dataLength);
      }
      newF.frame = CanFrame{
              .canId = frame.canId,
              .data = newF.data,
              .dataLength = frame.dataLength,
      };
    }
};


class TestSOTMaster : public SOTMaster<MockTestProtocol, TestSOTMaster>, public MockCanBuffer {
  public:
    using SOTMaster<MockTestProtocol, TestSOTMaster>::sendInitCommunicationRequest;

    explicit TestSOTMaster() : MockCanBuffer("Master"), SOTMaster<MockTestProtocol, TestSOTMaster>(*this) {}

    /*
    void canSendFrame(CanFrame &frame) override {
      putFrameInSendBuffer(frame);
    }
     */

    void processCanFramesReceived(list<CanFrameWithData> &frames) {
      framesReceived = frames;
      processCanFrames();
      /*
      for (auto &f : frames) {
        this->processCanFrameReceived(f.frame);
      }
      */
    }
};

class TestSOTMasterLockable : public SOTMasterLockableGeneric<MockTestProtocol, TestSOTMasterLockable, TestSOTMasterLockable>, public MockCanBuffer {
  public:
    using SOTMasterLockableGeneric<MockTestProtocol, TestSOTMasterLockable, TestSOTMasterLockable>::sendInitCommunicationRequest;

    explicit TestSOTMasterLockable():
    MockCanBuffer("Master"),
    SOTMasterLockableGeneric<MockTestProtocol, TestSOTMasterLockable, TestSOTMasterLockable>(*this) {}

    void processCanFramesReceived(list<CanFrameWithData> &frames) {
      framesReceived = frames;
      processCanFrames();
    }
};


class TestSOTClient : public SOTClient<MockTestProtocol, TestSOTClient>, public MockCanBuffer {
  public:
    using SOTClient<MockTestProtocol, TestSOTClient>::communicationState;

    explicit TestSOTClient() : MockCanBuffer("Client"), SOTClient<MockTestProtocol, TestSOTClient>(*this, 1) {}

    /*
    void canSendFrame(CanFrame &frame) override {
      putFrameInSendBuffer(frame);
    }
    */

    void processCanFramesReceived(list<CanFrameWithData> &frames) {
      framesReceived = frames;
      processCanFrames();
    }
};
