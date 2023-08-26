#pragma once

#include "communication/can/Can.h"
#include "util/EventFlag.h"


template<template<class T> class PROTOCOL_DEF, class CAN_INTERFACE_CLASS>
class SOTCanCommunication;


/// To create a driver, the functions of this class have to be implemented according to the specific microcontroller or os.
/// @note just one instance of a CanInterface should be created at any time.
//template<class COMMUNICATION_CLASS>
class CanInterface {
  private:
    //COMMUNICATION_CLASS *sotCanCommunication;

    /// needs to be set as first action by the SotCanCommunication class
    //void setSotCanCommunication(COMMUNICATION_CLASS *sotCanCommunication) {
    //    CanInterface::sotCanCommunication = sotCanCommunication;
    //}
    //friend COMMUNICATION_CLASS;

  public:

    /**
     * Put can frame into the send buffer.
     * @param frameIsOverflowError if set to true, the onTxOverflow will not be set although the buffer might overflow.
     * @return true if was successfully
     * @note the caller is expected to check onTxOverflow flag and add send an according package on overflow
     */
    virtual bool canSendFrame(CanFrame &frame, bool frameIsOverflowError = false) {
      return false;
    }


    /**
     * Get next can frame from the received buffer. This will also remove this frame from the received buffer.
     * @param receiveFrame the can id and data of this frame is set to the value of the last received frame.
     * @return true if there was still a frame in the buffer, otherwise false when the buffer is empty
     * @note the caller is expected to check onRxOverflow flag and add send an according package on overflow
     */
    virtual bool getNextCanFrameReceived(CanFrame &receiveFrame) {
      return false;
    }



    /// is set when the tx queue is full by maxsize-1  [just used at the client]
    static EventFlag onTxOverflow;
    /// is set when the rx queue is full or the incoming packages can't be progressed fast enough and some are lost [just used at the client]
    static EventFlag onRxOverflow;


    static void handleTxOverflow() {
      onTxOverflow._triggerEvent();
    }

    static void handleRxOverflow() {
      onRxOverflow._triggerEvent();
    }
};

