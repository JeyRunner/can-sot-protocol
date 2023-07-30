#pragma once


template<class TYPE, class COMMUNICATION_CLASS>
class ValueNodeTypeAbstractWithProt;

//namespace _internal {
/// This needs to be called before doing calling send or sendReadReq
template<class TYPE, class COMMUNICATION_CLASS>
static void valueNodeTypeAbstractWithProt_setProtocolRef(
        ValueNodeTypeAbstractWithProt<TYPE, COMMUNICATION_CLASS> &vNode, COMMUNICATION_CLASS *protocol){
    vNode._setProtocolRef(protocol);
}
//}