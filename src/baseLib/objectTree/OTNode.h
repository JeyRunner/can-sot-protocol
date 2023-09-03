#pragma once

#include <cinttypes>
#include "OTNodeValueTypes.h"
#include "OTUtil.h"
#include "communication/can/Can.h"
#include "util/EventFlag.h"

template<typename COMMUNICATION_CLASS, unsigned int OT_TABLE_SIZE, unsigned int INIT_NODES_SIZE>
struct ProtocolDef;


using NodeId = uint8_t;

enum class VALUE_NODE_ACCESS {
    CONST,
    READ,
    WRITE,
    READ_WRITE
};



class Node {
  protected:

};


//template<class PROTOCOL_DEF>
class ValueNodeAbstract {
  public:
    ValueNodeAbstract(NodeId nodeId/*, PROTOCOL_DEF &protocol*/) : nodeId(nodeId) /*, protocol(protocol)*/ {};
  public: // protected
    NodeId nodeId;
    VALUE_NODE_DATA_TYPES dataType;
    //PROTOCOL_DEF &protocol;

    /**
     * This event flag will be set to true after the value of this node was changed by the remote master or client.
     * This happens when a write-request or read-response package is received.
     */
    EventFlag receivedValueUpdate;

  public:
    inline const uint8_t getRequiredDataSizeInBytes() const;

    /**
     * Write value to data of a can frame.
     */
    inline void writeToData(uint8_t *data);

    /**
     * Write value to data of a can frame.
     */
    template<uint8_t SIZE>
    inline void writeToData(CanData<SIZE> data) {
      writeToData(data);
    }

    /**
     * Read value from data of a can frame.
     * This will overwrite the current value of the node.
     * This will also set the wasChangedEvent to true.
     */
    inline void readFromData(const uint8_t *data);

    /**
     * Read value from data of a can frame.
     * This will overwrite the current value of the node.
     */
    template<uint8_t SIZE>
    inline void readFromData(CanData<SIZE> data) {
      readFromData(data);
    }

    /**
     * Send the current value to the remote client or server.
     * This will put the corresponding can frame directly into the can send buffer.
     */
    //void sendValue() {
    //}

    /**
     * Send a request for reading the current value of this node from a remote client or server.
     * When the new read value arrives it will directly overwrite the current value of this node.
     * This will put the corresponding can frame directly into the can send buffer.
     * @note it will take time for the new value to arrive, so the new value will not be immediately available
     */
    //void sendReadRequest();
};




// -- Value Types --------------------------------------------
template<class TYPE/*, class PROTOCOL_DEF*/>
class ValueNodeTypeAbstract: public ValueNodeAbstract/*<PROTOCOL_DEF>*/ {
  public:
    ValueNodeTypeAbstract(NodeId nodeId/*, PROTOCOL_DEF protocol*/) : ValueNodeAbstract/*<PROTOCOL_DEF>*/(nodeId /*, protocol*/) {
      this->dataType = getValueNoteDataType<TYPE>();
    };

  public: // protected
    TYPE value;
    friend ValueNodeAbstract/*<PROTOCOL_DEF>*/;
};




template<class TYPE, class COMMUNICATION_CLASS>
class ValueNodeTypeAbstractWithProt: public ValueNodeTypeAbstract<TYPE> {
public:
    ValueNodeTypeAbstractWithProt(NodeId nodeId) : ValueNodeTypeAbstract<TYPE>(nodeId) {
        this->dataType = getValueNoteDataType<TYPE>();
    };

    friend void valueNodeTypeAbstractWithProt_setProtocolRef(
            ValueNodeTypeAbstractWithProt<TYPE, COMMUNICATION_CLASS> &vNode, COMMUNICATION_CLASS *protocol);

protected:
    COMMUNICATION_CLASS *sotCommunication = nullptr;

public:
    /// This needs to be called before doing calling send or sendReadReq
    void __setProtocolRef(COMMUNICATION_CLASS *protocol) {
        this->sotCommunication = protocol;
    }


public:
    /**
     * Send value of value node to the remote master or client.
     * This will add the corresponding send package into the can send buffer.
     */
    inline void sendValue() {
        sotCommunication->sendValue(*this);
    }

    /**
     * Send a request for reading the current value of this node from a remote client or server.
     * When the new read value arrives it will directly overwrite the current value of this node.
     * This will put the corresponding can frame directly into the can send buffer.
     * @note it will take time for the new value to arrive, so the new value will not be immediately available.
     */
    inline void sendReadValueReq() {
        sotCommunication->sendReadValueReq(*this);
    }
};





// -- Access Types --------------------------------------------

template<class TYPE, NodeId NODE_ID, class COMMUNICATION_CLASS>
class ValueNodeWritable: public ValueNodeTypeAbstractWithProt<TYPE, COMMUNICATION_CLASS> {
  public:
    ValueNodeWritable(): ValueNodeTypeAbstractWithProt<TYPE, COMMUNICATION_CLASS>(NODE_ID) {
    };

    void write(TYPE value) {
      this->value = value;
    }
};

template<class TYPE, NodeId NODE_ID, class COMMUNICATION_CLASS>
class ValueNodeReadable: public ValueNodeTypeAbstractWithProt<TYPE, COMMUNICATION_CLASS> {
  public:
    ValueNodeReadable(): ValueNodeTypeAbstractWithProt<TYPE, COMMUNICATION_CLASS>(NODE_ID) {
    };


    TYPE read() {
      return ((ValueNodeTypeAbstract<TYPE>*) this)->value;
    }
};

template<class TYPE, NodeId NODE_ID, class COMMUNICATION_CLASS>
class ValueNodeReadWriteable: public ValueNodeTypeAbstractWithProt<TYPE, COMMUNICATION_CLASS> {
  public:
    ValueNodeReadWriteable(): ValueNodeTypeAbstractWithProt<TYPE, COMMUNICATION_CLASS>(NODE_ID) {
    };

    void write(TYPE value) {
      this->value = value;
    }

    TYPE read() {
      return ((ValueNodeTypeAbstract<TYPE>*) this)->value;
    }
};


// -- Access Types for Enums ----------------------------------------
template<class TYPE, NodeId NODE_ID, class COMMUNICATION_CLASS>
class ValueNodeWritableEnum: public ValueNodeTypeAbstractWithProt<TYPE_UINT8, COMMUNICATION_CLASS> {
public:
    ValueNodeWritableEnum(): ValueNodeTypeAbstractWithProt<TYPE_UINT8, COMMUNICATION_CLASS>(NODE_ID) {
    };
    void write(TYPE value) {
        this->value = value;
    }
};

template<class TYPE, NodeId NODE_ID, class COMMUNICATION_CLASS>
class ValueNodeReadableEnum: public ValueNodeTypeAbstractWithProt<TYPE_UINT8, COMMUNICATION_CLASS> {
public:
    ValueNodeReadableEnum(): ValueNodeTypeAbstractWithProt<TYPE_UINT8, COMMUNICATION_CLASS>(NODE_ID) {
    };
    TYPE read() {
        return static_cast<TYPE>(((ValueNodeTypeAbstract<TYPE_UINT8>*) this)->value);
    }
};
template<class TYPE, NodeId NODE_ID, class COMMUNICATION_CLASS>
class ValueNodeReadWriteableEnum: public ValueNodeTypeAbstractWithProt<TYPE_UINT8, COMMUNICATION_CLASS> {
public:
    ValueNodeReadWriteableEnum(): ValueNodeTypeAbstractWithProt<TYPE_UINT8, COMMUNICATION_CLASS>(NODE_ID) {
    };

    void write(TYPE value) {
        this->value = value;
    }

    TYPE read() {
        return static_cast<TYPE>(((ValueNodeTypeAbstract<TYPE_UINT8>*) this)->value);
    }
};




// HAS TO BE INCLUDED AT THE END
#include "OTNodeValueNodeAbstractImpl.h"



/// Not working:
/*
template<class TYPE, VALUE_NODE_ACCESS ACCESS, NodeId NODE_ID>
class ValueNode: public ValueNodeTypeAbstract<TYPE> {
  public:
    explicit ValueNode(): ValueNodeAbstract(NODE_ID) {
    };

    void write(TYPE value) {
      this->value = value;
    }
};


template<class TYPE, NodeId NODE_ID>
class ValueNode<TYPE, VALUE_NODE_ACCESS::WRITE, NODE_ID> {
  public:
    void write(TYPE value) {
      ((ValueNodeTypeAbstract<TYPE>*) this)->value = value;
    }
};


template<class TYPE, NodeId NODE_ID>
class ValueNode<TYPE, VALUE_NODE_ACCESS::READ, NODE_ID> {
  public:
    TYPE read() {
      return ((ValueNodeTypeAbstract<TYPE>*) this)->value;
    }
};


template<class TYPE, NodeId NODE_ID>
class ValueNode<TYPE, VALUE_NODE_ACCESS::READ_WRITE, NODE_ID> {
  public:
    void write(TYPE value1) {
      ((ValueNodeTypeAbstract<TYPE>*) this)->value = value1;
    }

    TYPE read() {
      return ((ValueNodeTypeAbstract<TYPE>*) this)->value;
    }
};
*/