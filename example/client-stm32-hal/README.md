## CAN SOT 🌳 - example client-stm32-hal
This example has to be complied separate from the main repo since it requires the usage of a custom stm32 compiler toolchain.
This example is for the STM32F103C8 microcontroller aka `Blue Pill`.

Note that here all debug messages of CAN SOT where disabled to reduce the binary size.


### Compile
To compile this just use cmake:
```bash
mkdir build
cd build
cmake ..
make
```
Now the resulting `CanSotProtocol-example-client-stm32.bin` file in the `build` folder can be flashed to the microcontroller, e.g. by using openocd.