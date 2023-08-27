#pragma once

#include <utility>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>


#include "driver_template/DriverTemplate.hpp"

using namespace std;

/**
 * Driver for the socketCan interface in linux.
 */
class SocketCanInterface : public CanInterface {

  private:
    std::string canInterfaceName;
    int8_t ownDeviceId = 0;
    int canSocket = -1;
    bool socketConnected = false;

  public:
    /**
     * Note multiple instances of this class can exists at time.
     * @param canInterfaceName the name of the linux can interface to use
     * @param ownDeviceId the device id of this master (is 0 by default)
     */
    explicit SocketCanInterface(std::string canInterfaceName, uint8_t ownDeviceId = 0)
    : canInterfaceName(std::move(canInterfaceName)), ownDeviceId(ownDeviceId) {}

    virtual ~SocketCanInterface() {
      closeCanInterface();
    }


    /**
     * Start the can interface. This has to be called before doing any communication over can.
     */
    bool startCanInterface() {
      if ((canSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        cout << "could not open can socket " << endl;
        return false;
      }

      // setup Interface request structure
      ifreq ifr{};
      strncpy(ifr.ifr_name, canInterfaceName.c_str(), IFNAMSIZ - 1);
      ifr.ifr_name[IFNAMSIZ - 1] = '\0';
      ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);
      if (!ifr.ifr_ifindex) {
        cout << "could not open can socket: if_nametoindex error for interface name '" << canInterfaceName << "'" << endl;
        return false;
      }

      // add filter for incoming packages for only packages with master dest address
      if (!setupRxFilter()) {
        cout << "could not open can socket: could not make socket non-blocking" << endl;
        closeCanInterface();
        return false;
      }

      // init can addr struct with zero
      sockaddr_can addr{};
      memset(&addr, 0, sizeof(addr));
      addr.can_family = AF_CAN;
      addr.can_ifindex = ifr.ifr_ifindex;

      timeval tv{};
      tv.tv_sec = 0;
      tv.tv_usec = 100; //1 * 1000;  // 1ms read timeout
      setsockopt(canSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));

      if (bind(canSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        cout << "could not open can socket: bind(...) error for interface name '" << canInterfaceName << "'" << endl;
        return false;
      }

      if (!makeSocketNonBlocking(canSocket)) {
        cout << "could not open can socket: could not make socket non-blocking" << endl;
        closeCanInterface();
        return false;
      }

      socketConnected = true;
      return true;
    }

    /// add filter for incoming packages for only packages with master dest address
    bool setupRxFilter() const {
      struct can_filter rxFilter[1];
      uint16_t ownCanID = ((ownDeviceId & 0b000'0000'0111) << 5); // own 11 bit id
      uint16_t ownCanFilterMaks = ((0b000'0000'0111) << 5); // 11 bit id filter
      rxFilter[0].can_id = ownCanID;
      rxFilter[0].can_mask = ownCanFilterMaks;
      int ok = setsockopt(canSocket, SOL_CAN_RAW, CAN_RAW_FILTER, &rxFilter, sizeof(rxFilter));
      if (ok != 0) {
        perror("setsockopt(CAN_RAW_FILTER)");
        return false;
      }
      return true;
    }

    static bool makeSocketNonBlocking(int socked) {
      int flags = fcntl(socked, F_GETFL, 0);
      if (flags < 0) {
        perror("fcntl(F_GETFL)");
        return false;
      }

      flags |= O_NONBLOCK;
      if (fcntl(socked, F_SETFL, flags) < 0) {
        perror("fcntl(F_SETFL)");
        return false;
      }
      return true;
    }


    /**
     * Stop the can interface.
     */
    void closeCanInterface() {
      close(canSocket);
      socketConnected = false;
    }


    /**
     * Put can frame into the send buffer.
     * @return true if was successfully
     */
    bool canSendFrame(CanFrame &frame, bool frameIsOverflowError = false) override {
      if (!socketConnected) {
        cout << "could not send can frame since can socket is not connected" << endl;
        return false;
      }
      struct can_frame socketCanFrame{};
      socketCanFrame.can_id = frame.canId;
      socketCanFrame.len = frame.dataLength;
      memcpy(socketCanFrame.data, frame.data, frame.dataLength);

      if (write(canSocket, &socketCanFrame, CAN_MTU) != CAN_MTU) {
        cout << "could not send can frame: write failed" << endl;
        perror("can write frame");
        return false;
      }
      return true;
    }


    /**
     * Get next can frame from the received buffer. This will also remove this frame from the received buffer.
     * @param receiveFrame the can id and data of this frame is set to the value of the last received frame.
     * @return true if there was still a frame in the buffer, otherwise false when the buffer is empty
     */
    bool getNextCanFrameReceived(CanFrame &receiveFrame) override {
      if (!socketConnected) {
        cout << "could not send can frame since can socket is not connected" << endl;
        return false;
      }
      struct can_frame socketCanFrame{};

      auto numBytes = read(canSocket, &socketCanFrame, CAN_MTU);
      if (numBytes < 0) {
        //cout << "could not get can frame from receive buffer since read(...) returned an error: " << numBytes << endl;
        //perror("can read frame");
        return false;
      }

      receiveFrame.canId = socketCanFrame.can_id;
      receiveFrame.dataLength = socketCanFrame.len;
      memcpy(receiveFrame.data, socketCanFrame.data, socketCanFrame.len);

      return true;
    }
};