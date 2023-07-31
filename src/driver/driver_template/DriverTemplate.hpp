#pragma once

#include "communication/can/Can.h"


template<template<class T> class PROTOCOL_DEF, class CAN_INTERFACE_CLASS>
class SOTCanCommunication;


/// To create a driver, the functions of this class have to be implemented according to the specific microcontroller or os.
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
     * @return true if was successfully
     */
    virtual bool canSendFrame(CanFrame &frame) {}


    /**
     * Get next can frame from the received buffer. This will also remove this frame from the received buffer.
     * @param receiveFrame the can id and data of this frame is set to the value of the last received frame.
     * @return true if there was still a frame in the buffer, otherwise false when the buffer is empty
     */
    virtual bool getNextCanFrameReceived(CanFrame &receiveFrame) {
      return false;
    }
};