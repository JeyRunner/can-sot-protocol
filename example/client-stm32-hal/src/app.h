#include <stm32f1xx_hal_can.h>
#include "cubeMxGenerated/Inc/stm32f1xx_it.h"
//#include "main.h"

//#include <stm32_hal/can_stm32.h>
#include <stm32_hal/Stm32HalCanInterface.hpp>
#include <communication/SOTClient.h>
#include "protocol_generated/SOTProtocol.hpp"

/// set in main.cpp
extern CAN_HandleTypeDef hcan;


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
        /*
{
  // main communication loop
  while (1) {
    // wait
    HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
    HAL_Delay(1000);
  }
  return 0;
}*/

{
  // main communication loop
  /*
  while (true) {
      // wait
      HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
      HAL_Delay(1000);
  }
  return 0;
  */


  Stm32HalCanInterface canInterface(1, hcan);
  SOTClient<TestProtocol, Stm32HalCanInterface> sotClient(canInterface, 1);
  canInterface.startCanInterface();


  // set some values
  sotClient.getProtocol().objectTree._meta.protocolVersion.write(11);
  sotClient.getProtocol().objectTree.settings.value1.write(5);
  sotClient.getProtocol().objectTree.settings.value2.write(10);
  sotClient.getProtocol().objectTree.settings.subSettings.value3.write(0.25);


  // test
  sendTestFrame(canInterface);
  TestProtocol<SOTClient<TestProtocol, Stm32HalCanInterface>> p(&sotClient);
  auto d = p.objectTree.settings.value1.getTestVal();
  uint8_t data[8];
  p.objectTree.settings.value1.writeToData(data);
    uint8_t bytesNum = p.objectTree.settings.value1.getRequiredDataSizeInBytes();
  //sotClient.getProtocol().objectTree.settings.value1.write(static_cast<unsigned short>(d));



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

    HAL_Delay(500);
  }

  return 0;
}
