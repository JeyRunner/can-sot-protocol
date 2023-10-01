#pragma once

#ifdef __linux__
#include "optional"
#include "variant"
using namespace std;
#else
#include "etl/optional.h"
#include "etl/variant.h"
using namespace etl;
#endif

#include "objectTree/DataConversion.h"


struct RemoteCallDataWritable {
    virtual const uint8_t getRequiredDataSizeBytes() = 0;

    /**
     * Write value to data of a can frame.
     */
    virtual inline void _writeToData(uint8_t *data) = 0;
};

struct RemoteCallDataReadable {
    virtual const uint8_t getRequiredDataSizeBytes() = 0;

    /**
     * Read value from data of a can frame.
     * This will overwrite the current value of the node.
     * This will also set the wasChangedEvent to true.
     */
    virtual inline void _readFromData(const uint8_t *data) = 0;
};




template<class RETURN_DATA, class ERROR_ENUM>
struct RemoteCallReturn {
    bool isError = false;
    RETURN_DATA returnData;
    ERROR_ENUM returnError;

    RemoteCallReturn() {}
};


class RemoteCallCallerAbstract {
public:
    uint8_t id;

    /// is set when the callee answered to the remote call with return data or an error.
    EventFlag remoteCallReturned;

    RemoteCallDataReadable *__returnData = nullptr;
    TYPE_UINT8 *__returnError = nullptr;
    bool* __returnIsError;
};

template<uint8_t ID, class ARGS_DATA, class RETURN_DATA, class ERROR_ENUM, class COMMUNICATION_CLASS>
class RemoteCallCaller: public RemoteCallCallerAbstract {
protected:
    COMMUNICATION_CLASS *sotCommunication = nullptr;

public:
    /// This needs to be called before doing calling send or sendReadReq
    void __setProtocolRef(COMMUNICATION_CLASS *protocol) {
        this->sotCommunication = protocol;
    }

public:
    RemoteCallCaller() {
      id = ID;
      __returnData = &this->callReturnData.returnData;
      __returnError = reinterpret_cast<TYPE_UINT8*>(&this->callReturnData.returnError);
      __returnIsError = &this->callReturnData.isError;
    }


    /// contains the return data when the remote call returned
    RemoteCallReturn<RETURN_DATA, ERROR_ENUM> callReturnData;

    void sendCall(ARGS_DATA argumentData) {
        sotCommunication->sendRemoteCallRequest(*this, argumentData);
    }
};



class RemoteCallCallableAbstract {
public:
    uint8_t id;

    /// is set when the call has been called.
    EventFlag remoteCallCalled;

    RemoteCallDataReadable *__argData = nullptr;
};

template<uint8_t ID, class ARGS_DATA, class RETURN_DATA, class ERROR_ENUM, class COMMUNICATION_CLASS>
class RemoteCallCallable: public RemoteCallCallableAbstract {
protected:
    COMMUNICATION_CLASS *sotCommunication = nullptr;

public:
    /// This needs to be called before doing calling send or sendReadReq
    void __setProtocolRef(COMMUNICATION_CLASS *protocol) {
        this->sotCommunication = protocol;
    }

public:
    RemoteCallCallable() {
      id = ID;
      __argData = &this->argumentsData;
    }

    /// contains the argument data when the remote call was called
    ARGS_DATA argumentsData{};


    void sendReturnOk(RETURN_DATA returnData) {
        sotCommunication->sendRemoteCallResponseOk(*this, returnData);
    }

    void sendReturnError(ERROR_ENUM errorCode) {
        sotCommunication->sendRemoteCallResponseError(*this, (uint8_t) errorCode);
    }
};