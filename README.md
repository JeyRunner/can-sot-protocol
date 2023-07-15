# CAN sot protocol  [🚧 WIP 🚧]
This is the **CAN _simple object tree_ protocol** for the CAN bus.
Although it uses ideas from CANopen, it is not compatible with it but can be seen as a very simplified version.

## Concepts

In the network there are multiple clients and one master.
The whole communication structure (Object Tree and Real Time Packages) a client offers is specified in a yaml file (example: [example-can-sot.spec.yaml](example/example-can-sot.spec.yaml)).

### Object Tree

Clients offer an object tree, where each tree node represents a value that can be read or written from a master.
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
Each node in the object tree gets an unique id (2 bytes) that is then part of the can frame that transfers the value.

### Real Time Packages

Specific packages, that will directly be mapped to can frames, can be specified.
These packages are ment to for directly transfer multiple node values at real time without protocol overhead (like PDOs in CANopen).
Again the total size is limited by the maximum CAN frame data size (8bytes).
A RTP message can frame would look like this:

## Implementation

There is a python tool that generates C++ code for accessing the object tree from a configuration yaml file. The yaml file describes the object tree and contains all definitions for the Real Time Packages.

### General API
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
    //    acquire because can packages may be process by other thread/task
    ObjectTreeLocked ot = canSot.acquireObjectTree(); 
    ot.some_node.some_value.write(32f);
    int value2 = ot.some_node.value2.read();
    
    // e.g. react to incoming real time packages
    if (canSot.RPTsIncomming.SetValue3.recived()) {
      // ...
    }
    // reset all received flags of incoming RPTs
    canSot.RPTsIncomming.ReceivedFlagsReset();
    
    // will send all change values of OT to master (slower, with protocol overhead)
    ot.sendAllChangedValuesToMaster()
    
    // OR just send a specific RTP that will contain only some value(s) of the OT 
    //    (e.g. some_node.some_value that we wrote before)
    // this is faster and without overhead, will just send one can frame
    canSot.RTPsOutgoing.UpdateValue2RTP.sendToMaster();
    
    // unlock after access
    ot.unlock();
    
    // to other stuff e.g. with value2
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
    ot.some_node.some_valueY.write(32f);

    // e.g. react to incoming real time packages
    if (canSot.RPTsIncomming.SetValue4.recived()) {
    // ...
    }
    // reset all received flags of incoming RPTs
    canSot.RPTsIncomming.ReceivedFlagsReset();

    // will send all change values of OT to client (slower, with protocol overhead)
    ot.sendAllChangedValuesToClient();
    // OR send all values to client
    ot.sendAllValuesToClient();
    
    // OR just send a specific RTP that will contain only some value(s) of the OT 
    //    (e.g. some_node.some_value that we wrote before)
    // this is faster and without overhead, will just send one can frame
    canSot.RTPsOutgoing.some_valueY.sendToMaster();
}
```



#### Implementation Status

| Type   | Platform        | Status      |
| ------ | --------------- |-------------|
| master | linux/socketCAN | ✍🏻 planned |
| client | esp32           | ✍🏻 planned |
