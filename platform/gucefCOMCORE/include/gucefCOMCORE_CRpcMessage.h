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

#ifndef GUCEF_COMCORE_CRPCMESSAGE_H
#define GUCEF_COMCORE_CRPCMESSAGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

#ifndef GUCEF_COMCORE_CRPCSTATUS_H
#include "gucefCOMCORE_CRpcStatus.h"
#define GUCEF_COMCORE_CRPCSTATUS_H
#endif /* GUCEF_COMCORE_CRPCSTATUS_H ? */

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

class CRpcMessageCodec;

/*-------------------------------------------------------------------------*/

/**
 *  A schema-bound message (gucefCOMCORE RPC design 5.4): one slot per declared field, indexed by the dense slot
 *  index of the field, so access by number is O(1); access by name is one lookup in the type.
 *
 *  Values are kept as CVariant in the form of the field type: integers of their declared width, strings as UTF-8,
 *  decimals as CDecimal, dates as GUCEF_DATATYPE_DATE_IN_DAYS_SINCE_UNIX_EPOCH, timestamps as microseconds
 *  (GUCEF_DATATYPE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH), durations as Int64 microseconds, enums as Int32, date
 *  times as CDateTime. Setters convert where the conversion is exact and refuse otherwise. A single field that is
 *  set is encoded also when it holds its zero value (presence travels); an unset one reads as the zero value.
 *
 *  Unknown fields met while decoding are kept (preserveUnknownFields) and written again on encode, so a relay
 *  passes fields of a newer minor through. Not thread safe: one thread at a time.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcMessage
{
    public:

    typedef GUCEF::vector< CORE::CVariant >     TVariantVector;

    CRpcMessage( void );
    CRpcMessage( const CRpcContractPtr& contract, const CRpcTypeDescriptor* type );
    CRpcMessage( const CRpcMessage& src );
    ~CRpcMessage();
    CRpcMessage& operator=( const CRpcMessage& src );

    bool IsBound( void ) const;
    const CRpcTypeDescriptor* GetType( void ) const;
    const CRpcContractPtr& GetContract( void ) const;

    /** Binds to a type, dropping every value */
    void Bind( const CRpcContractPtr& contract, const CRpcTypeDescriptor* type );

    /** Unsets every field and drops unknown fields */
    void Reset( void );

    /** Single and optional fields */
    bool Set( UInt32 number, const CORE::CVariant& value );
    bool Get( UInt32 number, CORE::CVariant& value ) const;     /**< the zero value when unset; false for an unknown or non-singular field */
    bool Has( UInt32 number ) const;                            /**< set (singular) or not empty (repeated, map) */
    void Clear( UInt32 number );

    bool SetByName( const CORE::CString& name, const CORE::CVariant& value );
    bool GetByName( const CORE::CString& name, CORE::CVariant& value ) const;
    const CRpcFieldDescriptor* FindField( const CORE::CString& name ) const;

    bool SetBool( UInt32 number, bool value );
    bool SetInt32( UInt32 number, Int32 value );
    bool SetInt64( UInt32 number, Int64 value );
    bool SetUInt32( UInt32 number, UInt32 value );
    bool SetUInt64( UInt32 number, UInt64 value );
    bool SetFloat64( UInt32 number, Float64 value );
    bool SetString( UInt32 number, const CORE::CString& value );
    bool SetBytes( UInt32 number, const void* data, UInt32 size );
    bool SetBytes( UInt32 number, const CORE::CDynamicBuffer& data );
    bool SetDecimal( UInt32 number, const CORE::CDecimal& value );
    bool SetDate( UInt32 number, Int32 daysSinceEpoch );
    bool SetTimestamp( UInt32 number, Int64 microsSinceEpoch );
    bool SetDateTime( UInt32 number, const CORE::CDateTime& value );
    bool SetDuration( UInt32 number, Int64 micros );
    bool SetEnum( UInt32 number, Int32 value );
    bool SetDataNode( UInt32 number, const CORE::CDataNode& value );

    bool GetBool( UInt32 number ) const;
    Int32 GetInt32( UInt32 number ) const;
    Int64 GetInt64( UInt32 number ) const;
    UInt32 GetUInt32( UInt32 number ) const;
    UInt64 GetUInt64( UInt32 number ) const;
    Float64 GetFloat64( UInt32 number ) const;
    CORE::CString GetString( UInt32 number ) const;
    bool GetBytes( UInt32 number, CORE::CDynamicBuffer& data ) const;
    CORE::CDecimal GetDecimal( UInt32 number ) const;
    Int32 GetDate( UInt32 number ) const;
    Int64 GetTimestamp( UInt32 number ) const;
    CORE::CDateTime GetDateTime( UInt32 number ) const;
    Int64 GetDuration( UInt32 number ) const;
    Int32 GetEnum( UInt32 number ) const;
    bool GetDataNode( UInt32 number, CORE::CDataNode& value ) const;

    /** Message fields: the nested message, created (empty) when it was not set */
    CRpcMessage* GetMutableMessage( UInt32 number );
    /** NULL when not set */
    const CRpcMessage* GetMessage( UInt32 number ) const;
    bool SetMessage( UInt32 number, const CRpcMessage& value );

    /** Repeated fields */
    bool Add( UInt32 number, const CORE::CVariant& value );
    UInt32 GetCount( UInt32 number ) const;
    bool GetAt( UInt32 number, UInt32 index, CORE::CVariant& value ) const;
    CRpcMessage* AddMessage( UInt32 number );
    const CRpcMessage* GetMessageAt( UInt32 number, UInt32 index ) const;
    CRpcMessage* GetMutableMessageAt( UInt32 number, UInt32 index );

    /** Map fields; keys are integers, bool or strings */
    bool SetMapEntry( UInt32 number, const CORE::CVariant& key, const CORE::CVariant& value );
    bool GetMapEntry( UInt32 number, const CORE::CVariant& key, CORE::CVariant& value ) const;
    CRpcMessage* GetMutableMapMessage( UInt32 number, const CORE::CVariant& key );
    const CRpcMessage* GetMapMessage( UInt32 number, const CORE::CVariant& key ) const;
    bool GetMapKeys( UInt32 number, TVariantVector& keys ) const;   /**< in encoding order */

    /** Appends the encoding at offset of target (5.2). False when a value cannot be encoded */
    bool Encode( CORE::CDynamicBuffer& target, UInt32 offset, UInt32* bytesWritten = GUCEF_NULL ) const;
    bool Encode( CORE::CDynamicBuffer& target ) const;

    /**
     *  Decodes (5.3) into this bound message, replacing its values. With linkBinary, bytes fields link to the given
     *  memory (zero copy) and stay valid while it lives: Detach() makes them independent
     */
    bool Decode( const void* data, UInt32 size, CRpcStatus& status, bool linkBinary = false );

    /** Copies every value that links to outside memory */
    void Detach( void );

    UInt32 GetEncodedSize( void ) const;

    /** Unknown fields kept by the last Decode() */
    UInt32 GetUnknownFieldCount( void ) const;

    /** Whether Decode() keeps unknown fields (default true) */
    void SetPreserveUnknownFields( bool preserve );

    /**
     *  The JSON form (for debugging, CLI output and MCP): an object with the field names. Sensitive fields are
     *  replaced by the redaction marker unless asked otherwise
     */
    bool ToDataNode( CORE::CDataNode& target, bool redactSensitive = true ) const;
    bool FromDataNode( const CORE::CDataNode& source, CRpcStatus& status );

    /** Required fields set; unknown enum values give warnings only */
    bool Validate( CRpcStatus& status ) const;

    private:
    friend class CRpcMessageCodec;

    struct SSlot
    {
        bool isSet;
        CORE::CVariant value;
        CRpcMessage* message;
        TVariantVector values;
        GUCEF::vector< CRpcMessage* > messages;
        TVariantVector mapKeys;

        SSlot( void );
    };
    typedef GUCEF::vector< SSlot > TSlotVector;

    struct SUnknownField
    {
        UInt32 number;
        CORE::CDynamicBuffer bytes;      /**< key and value as they arrived */
    };
    typedef GUCEF::vector< SUnknownField > TUnknownFieldVector;

    SSlot* GetSlot( UInt32 number, const CRpcFieldDescriptor** field );
    const SSlot* GetSlot( UInt32 number, const CRpcFieldDescriptor** field ) const;
    void FreeSlots( void );
    void CopyFrom( const CRpcMessage& src );
    CRpcMessage* NewNested( const CRpcFieldDescriptor& field ) const;
    Int32 FindMapIndex( const SSlot& slot, const CORE::CVariant& key ) const;

    CRpcContractPtr m_contract;
    const CRpcTypeDescriptor* m_type;
    TSlotVector m_slots;
    TUnknownFieldVector m_unknown;
    bool m_preserveUnknown;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCMESSAGE_H ? */
