#include <doctest/doctest.h>
#include <cstring>
#include <utility>
#include "communication/SOTMaster.h"
#include "communication/SOTClient.h"
#include "vector"
#include "array"
#include <algorithm>

#include "TestUtil.h"
#include "MockSOTProtocol.hpp"

using namespace std;

static SOT_MESSAGE_TYPE getMessageType(CanFrame &frame) {
  return unpackCanFrameId(frame).messageType;
}

struct CanFrameWithData {
    CanFrame frame;
    uint8_t data[8];
};


constexpr size_t BUFFER_MAX_SIZE = 255;
class MockCanBuffer {
  public:
    list<CanFrameWithData> framesSend;

    explicit MockCanBuffer(string name): name(std::move(name)) {
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
      cout << "[" << name << "]" <<" send Frame with id: " << frame.canId << "  data(" << (int)frame.dataLength << "B)"
           << (frame.dataLength > 0 ? ": " : "")
           << byteArrayToString(frame.data, frame.dataLength) << endl;
      auto &newF = framesSend.emplace_back();
      if (frame.dataLength > 0) {
        memcpy(newF.data, frame.data, sizeof(uint8_t)*frame.dataLength);
      }
      newF.frame = CanFrame{
          .canId = frame.canId,
          .data = newF.data,
          .dataLength = frame.dataLength,
      };
    }
};


class TestSOTMaster: public SOTMaster<TestProtocol>, public MockCanBuffer {
public:
    explicit TestSOTMaster(): MockCanBuffer("Master")
    {}

    void canSendFrame(CanFrame &frame) override {
      putFrameInSendBuffer(frame);
    }

    void processCanFramesReceived(list<CanFrameWithData> &frames) {
      for (auto &f : frames) {
        this->processCanFrameReceived(f.frame);
      }
    }
};


class TestSOTClient: public SOTClient<TestProtocol>, public MockCanBuffer {
public:
    explicit TestSOTClient(): MockCanBuffer("Client"), SOTClient<TestProtocol>(1)
    {}

    void canSendFrame(CanFrame &frame) override {
      putFrameInSendBuffer(frame);
    }

    void processCanFramesReceived(list<CanFrameWithData> &frames) {
      for (auto &f : frames) {
        this->processCanFrameReceived(f.frame);
      }
    }
};
