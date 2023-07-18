#pragma once

#include <cinttypes>
#include "OTNodeValueTypes.h"
class SOTCanCommunication;


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


class ValueNodeAbstract {
  public:
    explicit ValueNodeAbstract(NodeId nodeId) : nodeId(nodeId) {};
  public: // protected
    NodeId nodeId;
    VALUE_NODE_DATA_TYPES dataType;

  public:
    /**
     * Write value to data of a can frame.
     */
    void writeToData(uint8_t *data);

    /**
     * Read value from data of a can frame.
     * This will overwrite the current value of the node.
     */
    void readFromData(const uint8_t *data);

    friend SOTCanCommunication;
};




// -- Value Types --------------------------------------------
template<class TYPE>
class ValueNodeTypeAbstract: public ValueNodeAbstract {
  public:
    explicit ValueNodeTypeAbstract(NodeId nodeId) : ValueNodeAbstract(nodeId) {
      dataType = getValueNoteDataType<TYPE>();
    };

  public: // protected
    TYPE value;
    friend ValueNodeAbstract;
};




// -- Access Types --------------------------------------------

template<class TYPE, NodeId NODE_ID>
class ValueNodeWritable: public ValueNodeTypeAbstract<TYPE> {
  public:
    explicit ValueNodeWritable(): ValueNodeTypeAbstract<TYPE>(NODE_ID) {
    };

    void write(TYPE value) {
      this->value = value;
    }
};

template<class TYPE, NodeId NODE_ID>
class ValueNodeReadable: public ValueNodeTypeAbstract<TYPE> {
  public:
    explicit ValueNodeReadable(): ValueNodeTypeAbstract<TYPE>(NODE_ID) {
    };


    TYPE read() {
      return ((ValueNodeTypeAbstract<TYPE>*) this)->value;
    }
};

template<class TYPE, NodeId NODE_ID>
class ValueNodeReadWriteable: public ValueNodeTypeAbstract<TYPE> {
  public:
    explicit ValueNodeReadWriteable(): ValueNodeTypeAbstract<TYPE>(NODE_ID) {
    };

    void write(TYPE value) {
      this->value = value;
    }

    TYPE read() {
      return ((ValueNodeTypeAbstract<TYPE>*) this)->value;
    }
};



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