#pragma once

#include <map>
#include <thread>
#include <optional>
#include <stdexcept>
#include <mutex>
#include <condition_variable>
#include <tl/expected.hpp>

#include "communication/SOTMaster.h"
#include "SOTMasterLockableType.h"



template<template <class T> class PROTOCOL_DEF, class SOT_MASTER>
struct ConnectedClientLockable: public ConnectedClientGeneric<PROTOCOL_DEF, SOT_MASTER, ConnectedClientLockable<PROTOCOL_DEF, SOT_MASTER>> {
    using ConnectedClientGeneric<PROTOCOL_DEF, SOT_MASTER, ConnectedClientLockable<PROTOCOL_DEF, SOT_MASTER>>::sotMaster;
    using ConnectedClientGeneric<PROTOCOL_DEF, SOT_MASTER, ConnectedClientLockable<PROTOCOL_DEF, SOT_MASTER>>::deviceId;

  public:
    inline void sendReadValueReqBlocking(ValueNodeAbstract &vNode) {
      // check if already locked
      unique_lock<mutex> &lock = sotMaster->__getCanCommunicationMutexLock();
      sotMaster->sendReadValueReq(vNode, deviceId);
      // wait
      condition_variable &blockingCallsConditionVariable = sotMaster->__blockingCallsConditionVariable();
      cout << "sendReadValueReqBlocking wait for blockingCallsConditionVariable" << endl;
      blockingCallsConditionVariable.wait(lock, [&]{
        cout << "sendReadValueReqBlocking blockingCallsConditionVariable -> check for condition, is: " << vNode.receivedValueUpdate.operator bool() << endl;
        return vNode.receivedValueUpdate;
      });
      cout << "sendReadValueReqBlocking DONE waiting for blockingCallsConditionVariable" << endl;
    }
};



template<template <class T> class PROTOCOL_DEF, class CAN_INTERFACE_CLASS, class _MASTER_CLASS>
class SOTMasterLockableGeneric: public SOTMasterGeneric<PROTOCOL_DEF, CAN_INTERFACE_CLASS, _MASTER_CLASS,
                                                    ConnectedClientLockable<PROTOCOL_DEF, CAN_INTERFACE_CLASS>>,
                                SOTMasterLockableType {
  private:
    mutex canCommunicationMutex;
    unique_lock<mutex> *canCommunicationMutexLockPtr = nullptr;
    std::condition_variable blockingCallsConditionVariable;

  public:
    explicit SOTMasterLockableGeneric(CAN_INTERFACE_CLASS &canInterface):
      SOTMasterGeneric<PROTOCOL_DEF, CAN_INTERFACE_CLASS, _MASTER_CLASS,
              ConnectedClientLockable<PROTOCOL_DEF, CAN_INTERFACE_CLASS>>(canInterface) {
    }


    /**
     * Just for internal use!
     */
    mutex& __canCommunicationMutex() {
      return this->canCommunicationMutex;
    }

    /**
     * Just for internal use!
     */
    std::condition_variable& __blockingCallsConditionVariable() {
      return this->blockingCallsConditionVariable;
    }

    /**
     * Just for internal use!
     */
    void __setCanCommunicationMutexLock(unique_lock<mutex> *lock) {
      this->canCommunicationMutexLockPtr = lock;
    }

    /**
     * Just for internal use!
     */
    unique_lock<mutex>& __getCanCommunicationMutexLock() {
      if (this->canCommunicationMutexLockPtr == nullptr) {
        throw runtime_error("can't get CanCommunicationMutexLock because its empty currently!");
      }
      return *this->canCommunicationMutexLockPtr;
    }



    enum class CLIENT_CONNECTING_ERRORS {
        TIMEOUT,
        REJECTED
    };

    /**
     * After calling addAndConnectToClient(...) or reconnectToClient(...),
     * this function can be called to block until client gets connected or an error occurs.
     * @return true when client is successfully connected, otherwise the error.
     */
     tl::expected<void, CLIENT_CONNECTING_ERRORS> waitTillClientConnectedBlocking() {

    }
};


template<template <class T> class PROTOCOL_DEF, class CAN_INTERFACE_CLASS>
class SOTMasterLockable: public SOTMasterLockableGeneric<PROTOCOL_DEF, CAN_INTERFACE_CLASS, SOTMasterLockable<PROTOCOL_DEF, CAN_INTERFACE_CLASS>>,
SOTMasterLockableType {
};