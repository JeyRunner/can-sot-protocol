#pragma once

//#include "objectTree/OTTemplate.h"
#include "objectTree/OTNodeValueTypeDefs.h"
#include "objectTree/OTDeclares.h"
#include "objectTree/ProtocolDef.h"
#include "remoteCalls/RemoteCalls.h"


struct TestFuncArgDataCaller: public RemoteCallDataWritable {
    TYPE_F32 arg1;
    TYPE_UINT8 arg2;

    TestFuncArgDataCaller() = default;
    TestFuncArgDataCaller(TYPE_F32 arg1, TYPE_UINT8 arg2): RemoteCallDataWritable(), arg1(arg1), arg2(arg2) {}

    const uint8_t getRequiredDataSizeBytes() final {
      return 5;
    }

    /**
     * Write value to data of a can frame.
     */
    inline void _writeToData(uint8_t *data) final {
      writeToDataF32(data[0], arg1);
      writeToDataUINT8(data[4], arg2);
    }
};


struct TestFuncArgDataCallable: public RemoteCallDataReadable {
    TYPE_F32 arg1;
    TYPE_UINT8 arg2;

    TestFuncArgDataCallable() = default;
    TestFuncArgDataCallable(TYPE_F32 arg1, TYPE_UINT8 arg2): arg1(arg1), arg2(arg2) {}

    const uint8_t getRequiredDataSizeBytes() final {
      return 5;
    }

    /**
     * Write value to data of a can frame.
     */
    inline void _readFromData(const uint8_t *data) final {
      readFromDataF32(data[0], arg1);
      readFromDataUINT8(data[4], arg2);
    }
};


struct TestFuncReturnDataCaller: RemoteCallDataReadable {
    TYPE_UINT8 data1;
    TYPE_UINT8 data2;
    TYPE_UINT16 data3;

    TestFuncReturnDataCaller() = default;
    TestFuncReturnDataCaller(TYPE_UINT8 data1, TYPE_UINT8 data2, TYPE_UINT16 data3)
            : RemoteCallDataReadable(), data1(data1), data2(data2), data3(data3) {}

    const uint8_t getRequiredDataSizeBytes() override {
      return sizeof(TYPE_UINT8) + sizeof(TYPE_UINT8) + sizeof(TYPE_UINT16);
    }

    void _readFromData(const uint8_t *data) override {
      readFromDataUINT8(data[0], data1);
      readFromDataUINT8(data[1], data2);
      readFromDataUINT16(data[2], data3);
    }
};

struct TestFuncReturnDataCallable: RemoteCallDataWritable {
    TYPE_UINT8 data1;
    TYPE_UINT8 data2;
    TYPE_UINT16 data3;

    TestFuncReturnDataCallable() = default;
    TestFuncReturnDataCallable(TYPE_UINT8 data1, TYPE_UINT8 data2, TYPE_UINT16 data3)
            : RemoteCallDataWritable(), data1(data1), data2(data2), data3(data3) {}

    const uint8_t getRequiredDataSizeBytes() override {
        return sizeof(TYPE_UINT8) + sizeof(TYPE_UINT8) + sizeof(TYPE_UINT16);
    }

    inline void _writeToData(uint8_t *data) override {
      writeToDataUINT8(data[0], data1);
      writeToDataUINT8(data[1], data2);
      writeToDataUINT16(data[2], data3);
    }
};
