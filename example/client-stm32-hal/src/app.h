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
            .canId = 0xFFFF,
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
    // set debug values
    sotClient.getProtocol().objectTree.debug.clientRxBufferNumPackages.write(canInterface.getRxBufferNumPackages());
    sotClient.getProtocol().objectTree.debug.clientTxBufferNumPackages.write(canInterface.getTxBufferNumPackages());

    // measure timing
    uint32_t startUs = canInterface.getCurrentMicros();
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);

    // handle all received can frames
    sotClient.processCanFrames();

    // measure timing
    uint32_t endUs = canInterface.getCurrentMicros();
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
    sotClient.getProtocol().objectTree.debug.clientProcessPackagesDurationMs.write((float)(endUs - startUs) / 1000.0);


    // on first connected
    if (sotClient.gotConnectedEvent.checkAndReset()) {
    }


    // when received new value from remote client
    if (sotClient.getProtocol().objectTree.settings.subSettings.value3.receivedValueUpdate.checkAndReset()) {
    }

    // change some value
    sotClient.getProtocol().objectTree.settings.subSettings.value3.write(
            sotClient.getProtocol().objectTree.settings.subSettings.value3.read()+0.1
    );

    // testing: send a lot of packages to test tx overflow
    if (sotClient.isConnected()) {
      /*
      for (int i = 0; i < 10; ++i) {
        sendTestFrame(canInterface);
        sotClient.getProtocol().objectTree.debug.clientTxBufferNumPackages.write(canInterface.getTxBufferNumPackages());
        sotClient.getProtocol().objectTree.debug.clientTxBufferNumPackages.sendValue();
      }
      */
    }


    // wait
    //HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
    //sendTestFrame(canInterface);
    HAL_Delay(1);
  }

  return 0;
}
