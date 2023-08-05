// disable all debug messages of can sot so reduce binary size
#define SOT_DEBUG false

#include "cubeMxGenerated/Inc/stm32f1xx_it.h"
#include <stm32f1xx_hal_can.h>
#include <stm32_hal/Stm32HalCanInterface.hpp>
#include <communication/SOTClient.h>
#include "protocol_generated/SOTProtocol.hpp"

/// the handle to the used can, set in main.cpp
extern CAN_HandleTypeDef hcan;


/// just for some testing
void sendTestFrame(Stm32HalCanInterface &canInterface) {
    // test send frame
    uint8_t data[1] = {1};
    CanFrame frame{
            .canId = 0,
            .data = data,
            .dataLength = 1
    };
    canInterface.canSendFrame(frame);
}


/**
 * Gets called from cubeMxGenerated/Src/main.cpp
 * @return
 */
int runApp()
{
  uint8_t ownSOTDeviceId = 1;
  Stm32HalCanInterface canInterface(ownSOTDeviceId, hcan);
  SOTClient<TestProtocol, Stm32HalCanInterface> sotClient(canInterface, ownSOTDeviceId);
  canInterface.startCanInterface();


  // set some values
  sotClient.getProtocol().objectTree._meta.protocolVersion.write(11);
  sotClient.getProtocol().objectTree.settings.value1.write(5);
  sotClient.getProtocol().objectTree.settings.value2.write(10);
  sotClient.getProtocol().objectTree.settings.subSettings.value3.write(0.25);



  // main communication loop
  while (true) {
    // handle all received can frames
    sotClient.processCanFrames();

    // on first connected
    if (sotClient.gotConnectedEvent.checkAndReset()) {
    }


    // when received new value from remote client
    if (sotClient.getProtocol().objectTree.settings.subSettings.value3.receivedValueUpdate.checkAndReset()) {
    }

    // change some value
    sotClient.getProtocol().objectTree.settings.subSettings.value3.write(
            sotClient.getProtocol().objectTree.settings.subSettings.value3.read()+0.001
    );


    // wait
    HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
    //sendTestFrame(canInterface);
    //HAL_Delay(500);
  }

  return 0;
}
