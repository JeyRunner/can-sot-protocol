[![pipeline status](https://gitlab.com/JeyRunner/can-sot-protocol/badges/main/pipeline.svg)](https://gitlab.com/JeyRunner/can-sot-protocol/-/commits/main)

# CAN SOT Protocol 🌳  &emsp;  🚧 WIP 🚧
This is the **CAN _simple object tree_ protocol** for the CAN bus.
Although it uses ideas from CANopen, it is not compatible with it but can be seen as a very simplified version.

## Concepts

In the network there are multiple clients and one master.
The whole communication structure (Object Tree and Real Time Packages) a client offers is specified in a yaml file (example: [example-can-sot.spec.yaml](example/example-can-sot.spec.yaml)).
This file is used to generate the required protocol code for master and client. 
See [cliTool/README.md](cliTool%2FREADME.md) for how to use spec.yaml files for protocol code generation.

### Object Tree

Clients offer an object tree, where each tree node represents a value that can be __read or written from a master__.
Tree nodes have a datatype (which can be at max 8 bytes long).
Below you can see an example of a simple object tree:

```yaml
- settings:
    - value1: uint32
    - other_settings:
        - some_flag: bool
    - value2: float64
```

In general nodes can be specified as readable, writable or both.
The master node can now read and write values of the nodes of the clients in the network.
This is done by sending one object node value at a time via a can data frame (similar to SDOs in CANopen, but here the size of value is limited to 7 bytes).
Each node in the object tree gets a unique id (2 bytes) that is then part of the can frame that transfers the value.
For details see [protocol_can_frames.md](doc/protocol_can_frames.md).

Note that the object tree will not be saved in persistent memory, therefore the master should send all settings on startup to the client.

### Stream Packages

Specific packages, that will directly be mapped to can frames, can be specified.
These packages are ment to for directly transfer multiple node values at real time without protocol overhead (like PDOs in CANopen).
Again the total size is limited by the maximum CAN frame data size (8bytes).
For details see [protocol_can_frames.md](doc/protocol_can_frames.md).

## Implementation

There is a python tool that generates C++ code for accessing the object tree from a configuration yaml file. The yaml file describes the object tree and contains all definitions for the Real Time Packages.

The implementation is independent of the used platform. To support a platform a corresponding driver has to be implemented in `src/drivers`.

### General API
See examples in the [example](example) folder. 
For the client:
```c++
// ... init of canSot ...
while (true) {
    // e.g. in the main loop on a microcontroller:
    
    // process rx can frames and write them into the object tree
    // also send can frames to the master (can be also done with separate function)
    // this can also be done in e.g. a separate freeRTOS task
    canSot.processRxTxCanPackages(); // this will lock the object tree
    
    // before writing or reading values lock the object tree
    //    acquire because can packages may be processed in other thread/task
    ObjectTreeLocked ot = canSot.acquireObjectTree(); 
    ot.some_node.some_value.write(32f);
    int value2 = ot.some_node.value2.read();
    
    // e.g. react to incoming stream packages
    if (canSot.SPsIncomming.SetValue3.received()) {
      // ...
    }
    // reset all received flags of incoming SPs
    canSot.SPsIncomming.ReceivedFlagsReset();
    

    // send a specific SP that will contain only some value(s) of the OT 
    //    (e.g. some_node.some_value that we wrote before)
    //    without overhead, will just send one can frame
    canSot.SPsOutgoing.UpdateValue2SP.sendToMaster();
    
    // unlock after access
    ot.unlock();
    
    // do other stuff e.g. with value2
    printf("value2: %d", value2)
}
```

For the master (works with socketCAN):
```c++
// ... init of canSot ...
while (true) {
    // process rx can frames and write them into the object tree
    // also send can frames
    canSot.processRxTxCanPackages();
    
    // here no locking is required of object tree since all is done in one thread
    ObjectTree ot = canSot.objectTree();
    ot.some_node.some_valueY.write(32f); // just set value (will not send anything)

    // e.g. react to incoming real time packages
    if (canSot.RPTsIncomming.SetValue4.received()) {
    // ...
    }
    // reset all received flags of incoming SPs
    canSot.SPsIncomming.ReceivedFlagsReset();
    
    // pull node values from a remote client
    ot.some_node.some_valueY.sendReadValueReq();
    // when value is received corresponding received flag will be set in the valueNode
    // note that this will happen with some delay 
    //    (earliest after next processRxTxCanPackages() call)
    if (ot.some_node.some_valueY.receivedValueUpdate.checkAndReset()) 
    { /*...*/ }

    // will send all change values of OT to client (slower, with protocol overhead)
    ot.sendAllChangedNodeValuesToClient();
    // OR send all values to client
    ot.sendAllNodeValuesToClient();
    // OR send just a specific value to client
    ot.some_node.some_valueY.sendValue();
    
    // OR just send a specific SP that will contain only some value(s) of the OT 
    //    (e.g. some_node.some_value that we wrote before)
    // this is faster and without overhead, will just send one can frame
    canSot.SPsOutgoing.some_valueY.sendToClient();
}
```



### Implementation Status

| Type   | Platform            | Status          | Example                                        |
|--------|---------------------|-----------------|:-----------------------------------------------|
| master | linux [`socketCAN`] | 🏗️ in progress | [client-linux](example%2Fclient-linux)         |
| client | linux [`socketCAN`] | 🏗️ in progress | [master-linux](example%2Fmaster-linux)         |
| client | stm32 [`STM32 HAL`] | 🏗️ in progress | [client-stm32-hal](example%2Fclient-stm32-hal) |
| client | esp32 [`ESP-IDF`]   | ✍🏻 planned     |                                                |


### Compile CAN Sot
To compile CAN Sot for the current host platform (including the cliTool):
```bash
mkdir build
cd build
cmake ..
make
```
This will also build the example master and client for the current platform.

### Include in your project
When want to use CAN Sot in your own project as dependency, you can include it via Cmake.
Adapt you CMakeLists.txt file as follows:
```cmake
# we use CPMAddPackage to download and include CAN Sot from gitlab
# you need to put CPM.cmake into the cmake/ folder to install CPM
# -> see https://github.com/cpm-cmake/CPM.cmake#adding-cpm
include(cmake/CPM.cmake)

CPMAddPackage("gl:JeyRunner/can-sot-protocol")

# add your sources
# ...
add_executable(canSot-client main.cpp)
# folder for the generated protocol header file
target_include_directories(canSot-client  PRIVATE protocol_generated) 
target_link_libraries(canSot-client  PRIVATE CanSotProtocol-baselib)
```
You also need to generate a header file for you protocol form your defined `MYPROTCOL.spec.ymal` file, to do this see [cliTool/README.md](cliTool%2FREADME.md).
This header file will be included form your source code (as e.g. in [example/client-linux](example%2Fclient-linux/src)).
