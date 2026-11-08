/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef GUCEF_COMCORE_CRPCFIELDDESCRIPTOR_H
#define GUCEF_COMCORE_CRPCFIELDDESCRIPTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COMCORE_RPCTYPES_H
#include "gucefCOMCORE_RpcTypes.h"
#define GUCEF_COMCORE_RPCTYPES_H
#endif /* GUCEF_COMCORE_RPCTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CRpcContract;
class CRpcTypeDescriptor;
class CRpcEnumDescriptor;

/*-------------------------------------------------------------------------*/

/**
 *  A field of a message type (gucefCOMCORE RPC design 3.2, 3.4). For a map the type is the value type and
 *  GetMapKeyType() the key type. Immutable once its contract is built.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcFieldDescriptor
{
    public:

    CRpcFieldDescriptor( void );
    ~CRpcFieldDescriptor();

    UInt32 GetNumber( void ) const;
    const CORE::CString& GetName( void ) const;
    TRpcFieldType GetType( void ) const;
    TRpcCardinality GetCardinality( void ) const;
    TRpcFieldType GetMapKeyType( void ) const;
    const CORE::CString& GetTypeRef( void ) const;              /**< full name of the message or enum type, empty otherwise */
    const CRpcTypeDescriptor* GetMessageType( void ) const;     /**< resolved at build for message fields */
    const CRpcEnumDescriptor* GetEnumType( void ) const;        /**< resolved at build for enum fields */
    Int32 GetDecimalScale( void ) const;                        /**< -1 for a free scale */
    UInt32 GetFlags( void ) const;                              /**< GUCEF_RPC_FIELDFLAG_* */
    UInt32 GetSince( void ) const;
    UInt32 GetSlotIndex( void ) const;                          /**< position in the type, by ascending number */
    const CORE::CString& GetDoc( void ) const;

    bool IsRepeated( void ) const;
    bool IsMap( void ) const;
    bool IsSingular( void ) const;                              /**< single or optional */
    bool IsRequired( void ) const;
    bool IsSensitive( void ) const;
    bool IsFixedScaleDecimal( void ) const;

    /** The protobuf wire type of one value of the field */
    UInt32 GetWireType( void ) const;

    /** Repeated scalars of VARINT and FIXED kinds are packed */
    bool IsPacked( void ) const;

    private:
    friend class CRpcContract;

    CRpcFieldDescriptor( const CRpcFieldDescriptor& src );              /**< not implemented */
    CRpcFieldDescriptor& operator=( const CRpcFieldDescriptor& src );   /**< not implemented */

    UInt32 m_number;
    CORE::CString m_name;
    TRpcFieldType m_type;
    TRpcCardinality m_cardinality;
    TRpcFieldType m_mapKeyType;
    CORE::CString m_typeRef;
    const CRpcTypeDescriptor* m_messageType;
    const CRpcEnumDescriptor* m_enumType;
    Int32 m_decimalScale;
    UInt32 m_flags;
    UInt32 m_since;
    UInt32 m_slotIndex;
    UInt32 m_wireType;
    CORE::CString m_doc;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCFIELDDESCRIPTOR_H ? */
