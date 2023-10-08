#pragma once

#ifdef __linux__
#include "optional"
#include "variant"
#include "functional"
using namespace std;
#else
#include "etl/optional.h"
#include "etl/variant.h"
#include "etl/function.h"
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



/// no data
struct VoidRemoteCallDataWritable: RemoteCallDataWritable {
  const uint8_t getRequiredDataSizeBytes() override {
    return 0;
  }

  void _writeToData(uint8_t *data) override {}
};

/// no data
struct VoidRemoteCallDataReadable: RemoteCallDataReadable {
  const uint8_t getRequiredDataSizeBytes() override {
    return 0;
  }

  void _readFromData(const uint8_t *data) override {}
};

/// used when there is not error type
enum VOID_ENUM {};




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

    /**
     * Will call the given handleFunction when the remote called returned.
     * This will also clear the remoteCallReturned flag.
     */
#ifdef __linux__
    inline void handleCallReturned(function<void (RemoteCallReturn<RETURN_DATA, ERROR_ENUM> returnData)> handleFunction) {
#else
    inline void handleCallReturned(void (*handleFunction) (RemoteCallReturn<RETURN_DATA, ERROR_ENUM> returnData)) {
#endif
      // void (*handleFunction) (RemoteCallReturn<RETURN_DATA, ERROR_ENUM> returnData
      if (remoteCallReturned.checkAndReset()) {
        #ifdef __linux__
        handleFunction(callReturnData);
        #else
        (*handleFunction)(callReturnData);
        #endif
      }
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


    /**
     * Will call the given handleFunction when the remote was called.
     * The return value handleFunction will be send back as a response to the caller.
     * This will also clear the remoteCallReturned flag.
     */
#ifdef __linux__
    inline void handleCallCalled(function<variant<RETURN_DATA, ERROR_ENUM> (ARGS_DATA arguments)> handleFunction) {
#else
    inline void handleCallCalled(variant<RETURN_DATA, ERROR_ENUM> (*handleFunction) (ARGS_DATA arguments)) {
#endif
      // variant<RETURN_DATA, ERROR_ENUM> (*handleFunction) (ARGS_DATA arguments)
      if (remoteCallCalled.checkAndReset()) {
#ifdef __linux__
        variant<RETURN_DATA, ERROR_ENUM> retValue = handleFunction(argumentsData);
#else
        variant<RETURN_DATA, ERROR_ENUM> retValue = (*handleFunction)(argumentsData);
#endif
        if (holds_alternative<RETURN_DATA>(retValue)) {
          sendReturnOk(get<RETURN_DATA>(retValue));
        }
        else if (holds_alternative<ERROR_ENUM>(retValue)) {
          sendReturnError(get<ERROR_ENUM>(retValue));
        }
      }
    }
};



