#pragma once

#include <map>
#include <thread>
#include <optional>
#include <stdexcept>
#include <mutex>
#include <iostream>

#include "objectTree/OTDeclares.h"
#include "communication/can/CanPackages.h"
#include "SOTDefs.h"
#include "util/Logging.h"
#include "objectTree/ObjectTree.h"
#include "communication/SOTCanCommunication.h"
#include "util/EventFlag.h"


template<class SOT_CAN_COMMUNICATION>
class CanMasterLock {
  private:
    std::unique_lock<mutex> lock;

  public:
    /// allays access the master or clients through this reference.
    SOT_CAN_COMMUNICATION &master;

    explicit CanMasterLock(mutex &mutex, SOT_CAN_COMMUNICATION &canMaster):
      lock(mutex), master(canMaster) {
      master.__setCanCommunicationMutexLock(&lock);
    }

    /**
     * Release the lock to the CanMaster so that the background thread can continue and
     * incoming packages can be progressed again.
     * After releasing the lock DO NOT access any functionality
     * of the CanMaster or clients (as reading writing values form/to the object tree).
     */
    void unlock() {
      lock.unlock();
      //std::condition_variable& cv = master.__canCommunicationConditionVariable();
      //cv.notify_one();
    }


    SOT_CAN_COMMUNICATION *operator->() const {
      return &master;
    }


    virtual ~CanMasterLock() {
      // remove lock again
      master.__setCanCommunicationMutexLock(nullptr);
    }
};


template<class SOT_CAN_COMMUNICATION, class CAN_INTERFACE_CLASS,
        std::enable_if_t<std::is_base_of_v<SOTMasterLockableType, SOT_CAN_COMMUNICATION>, bool> = true>
class SOTMasterThreaded {
  private:
    SOT_CAN_COMMUNICATION canMaster;

    unique_ptr<thread> mainThread = nullptr;
    atomic<bool> mainThreadRunning = false;
    atomic<float> mainThreadWaitMs = 1;


  public:
    explicit SOTMasterThreaded(CAN_INTERFACE_CLASS &canInterface): canMaster(canInterface) {
    }

    /**
     * Use just for testing.
     */
    explicit SOTMasterThreaded(): canMaster() {
    }


    virtual ~SOTMasterThreaded() {
      stopMainThread();
    }




    /**
     * Start the can communication thread (will react to received messages).
     * @param mainThreadLoopWaitMs will sleep this time in the communication loop at each iteration.
     */
    void startMainThread(float mainThreadLoopWaitMs = 1) {
      this->mainThreadWaitMs = mainThreadLoopWaitMs;
      mainThreadRunning = true;
      mainThread = make_unique<thread>(&SOTMasterThreaded::mainThreadFunc, this);
    }

    void stopMainThread() {
      if (mainThread and mainThreadRunning) {
        mainThreadRunning = false;
        mainThread->join(); // wait for the thread to exit
        std::cout << "stopped SOTMasterThreaded main thread" << endl;
      }
    }

    /**
     * Acquire the lock to the CanMaster class.
     * To access ANY functionality of the CanMaster or clients (as reading writing values form/to the object tree)
     * first acquire this lock.
     * While holding this lock the background thread for handling received can packages will be blocked.
     * So when you are finished with your operations, e.g. accessing the object tree of a client,
     * please release the lock again, so that incoming packages can be progressed again.
     * @note Holding the lock should only be for a short time (e.g. < 1ms)
     */
    CanMasterLock<SOT_CAN_COMMUNICATION> acquireLock() {
      return CanMasterLock<SOT_CAN_COMMUNICATION>(canMaster.__canCommunicationMutex(), canMaster);
    }


  private:
    void mainThreadFunc() {
      while (mainThreadRunning) {
        cout << "master thread waiting to acquire lock" << endl;
        unique_lock<mutex> lock(canMaster.__canCommunicationMutex());
        //std::condition_variable& cv = canMaster.__canCommunicationConditionVariable();
        //cv.wait(lock);

        cout << "master thread got lock -> will processCanFrames" << endl;
        canMaster.processCanFrames();
        this_thread::sleep_for(2ms);

        // unlock and wakeup waiting blocking calls (e.g. sendReadValueReqBlocking)
        condition_variable &blockingCallsConditionVariable = canMaster.__blockingCallsConditionVariable();
        lock.unlock();
        blockingCallsConditionVariable.notify_all();

        cout << "master thread released lock" << endl;
        this_thread::sleep_for(chrono::duration<float, milli>(mainThreadWaitMs));
      }
    }

};