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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CDATANODEBINARYSERIALIZER_H
#include "gucefCORE_CDataNodeBinarySerializer.h"
#define GUCEF_CORE_CDATANODEBINARYSERIALIZER_H
#endif /* GUCEF_CORE_CDATANODEBINARYSERIALIZER_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_COMCORE_CRPCMESSAGECODEC_H
#include "gucefCOMCORE_CRpcMessageCodec.h"
#define GUCEF_COMCORE_CRPCMESSAGECODEC_H
#endif /* GUCEF_COMCORE_CRPCMESSAGECODEC_H ? */

#include "gucefCOMCORE_CRpcMessage.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRpcMessage::SSlot::SSlot( void )
    : isSet( false )
    , value()
    , message( GUCEF_NULL )
    , values()
    , messages()
    , mapKeys()
{

}

/*-------------------------------------------------------------------------*/

CRpcMessage::CRpcMessage( void )
    : m_contract()
    , m_type( GUCEF_NULL )
    , m_slots()
    , m_unknown()
    , m_preserveUnknown( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcMessage::CRpcMessage( const CRpcContractPtr& contract, const CRpcTypeDescriptor* type )
    : m_contract()
    , m_type( GUCEF_NULL )
    , m_slots()
    , m_unknown()
    , m_preserveUnknown( true )
{GUCEF_TRACE;

    Bind( contract, type );
}

/*-------------------------------------------------------------------------*/

CRpcMessage::CRpcMessage( const CRpcMessage& src )
    : m_contract()
    , m_type( GUCEF_NULL )
    , m_slots()
    , m_unknown()
    , m_preserveUnknown( src.m_preserveUnknown )
{GUCEF_TRACE;

    CopyFrom( src );
}

/*-------------------------------------------------------------------------*/

CRpcMessage::~CRpcMessage()
{GUCEF_TRACE;

    FreeSlots();
}

/*-------------------------------------------------------------------------*/

CRpcMessage&
CRpcMessage::operator=( const CRpcMessage& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        FreeSlots();
        m_preserveUnknown = src.m_preserveUnknown;
        CopyFrom( src );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CRpcMessage::FreeSlots( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_slots.size(); ++i )
    {
        SSlot& slot = m_slots[ i ];
        GUCEF_DELETE slot.message;
        slot.message = GUCEF_NULL;
        for ( UInt32 n=0; n<slot.messages.size(); ++n )
            GUCEF_DELETE slot.messages[ n ];
        slot.messages.clear();
    }
    m_slots.clear();
    m_unknown.clear();
}

/*-------------------------------------------------------------------------*/

void
CRpcMessage::CopyFrom( const CRpcMessage& src )
{GUCEF_TRACE;

    m_contract = src.m_contract;
    m_type = src.m_type;
    m_slots = src.m_slots;
    m_unknown = src.m_unknown;

    // the nested messages are owned per instance: deep copies
    for ( UInt32 i=0; i<m_slots.size(); ++i )
    {
        SSlot& slot = m_slots[ i ];
        if ( GUCEF_NULL != slot.message )
            slot.message = GUCEF_NEW CRpcMessage( *slot.message );
        for ( UInt32 n=0; n<slot.messages.size(); ++n )
            slot.messages[ n ] = GUCEF_NEW CRpcMessage( *slot.messages[ n ] );
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::IsBound( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_type;
}

/*-------------------------------------------------------------------------*/

const CRpcTypeDescriptor*
CRpcMessage::GetType( void ) const
{GUCEF_TRACE;

    return m_type;
}

/*-------------------------------------------------------------------------*/

const CRpcContractPtr&
CRpcMessage::GetContract( void ) const
{GUCEF_TRACE;

    return m_contract;
}

/*-------------------------------------------------------------------------*/

void
CRpcMessage::Bind( const CRpcContractPtr& contract, const CRpcTypeDescriptor* type )
{GUCEF_TRACE;

    FreeSlots();
    m_contract = contract;
    m_type = type;
    if ( GUCEF_NULL != type )
        m_slots.resize( type->GetFieldCount() );
}

/*-------------------------------------------------------------------------*/

void
CRpcMessage::Reset( void )
{GUCEF_TRACE;

    Bind( m_contract, m_type );
}

/*-------------------------------------------------------------------------*/

CRpcMessage::SSlot*
CRpcMessage::GetSlot( UInt32 number, const CRpcFieldDescriptor** field )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_type )
        return GUCEF_NULL;
    const CRpcFieldDescriptor* f = m_type->FindFieldByNumber( number );
    if ( GUCEF_NULL == f )
        return GUCEF_NULL;
    *field = f;
    return &m_slots[ f->GetSlotIndex() ];
}

/*-------------------------------------------------------------------------*/

const CRpcMessage::SSlot*
CRpcMessage::GetSlot( UInt32 number, const CRpcFieldDescriptor** field ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_type )
        return GUCEF_NULL;
    const CRpcFieldDescriptor* f = m_type->FindFieldByNumber( number );
    if ( GUCEF_NULL == f )
        return GUCEF_NULL;
    *field = f;
    return &m_slots[ f->GetSlotIndex() ];
}

/*-------------------------------------------------------------------------*/

CRpcMessage*
CRpcMessage::NewNested( const CRpcFieldDescriptor& field ) const
{GUCEF_TRACE;

    CRpcMessage* nested = GUCEF_NEW CRpcMessage( m_contract, field.GetMessageType() );
    nested->m_preserveUnknown = m_preserveUnknown;
    return nested;
}

/*-------------------------------------------------------------------------*/

Int32
CRpcMessage::FindMapIndex( const SSlot& slot, const CORE::CVariant& key ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<slot.mapKeys.size(); ++i )
        if ( slot.mapKeys[ i ] == key )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::Set( UInt32 number, const CORE::CVariant& value )
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = GUCEF_NULL;
    SSlot* slot = GetSlot( number, &field );
    if ( GUCEF_NULL == slot || !field->IsSingular() || RPC_FIELDTYPE_MESSAGE == field->GetType() )
        return false;
    CORE::CVariant converted;
    if ( !CRpcMessageCodec::ConvertValue( *field, field->GetType(), value, converted ) )
        return false;
    slot->value = converted;
    slot->isSet = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::Get( UInt32 number, CORE::CVariant& value ) const
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = GUCEF_NULL;
    const SSlot* slot = GetSlot( number, &field );
    if ( GUCEF_NULL == slot || !field->IsSingular() || RPC_FIELDTYPE_MESSAGE == field->GetType() )
        return false;
    if ( slot->isSet )
    {
        // a value that links to a decoded buffer is handed out as a link too (zero copy, see Detach())
        if ( slot->value.IsDynamicMemoryLinked() )
            value.LinkTo( slot->value );
        else
            value = slot->value;
    }
    else
        value = CRpcMessageCodec::ZeroValue( *field, field->GetType() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::Has( UInt32 number ) const
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = GUCEF_NULL;
    const SSlot* slot = GetSlot( number, &field );
    if ( GUCEF_NULL == slot )
        return false;
    if ( field->IsSingular() )
        return slot->isSet;
    return !slot->values.empty() || !slot->messages.empty() || !slot->mapKeys.empty();
}

/*-------------------------------------------------------------------------*/

void
CRpcMessage::Clear( UInt32 number )
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = GUCEF_NULL;
    SSlot* slot = GetSlot( number, &field );
    if ( GUCEF_NULL == slot )
        return;
    GUCEF_DELETE slot->message;
    for ( UInt32 n=0; n<slot->messages.size(); ++n )
        GUCEF_DELETE slot->messages[ n ];
    *slot = SSlot();
}

/*-------------------------------------------------------------------------*/

const CRpcFieldDescriptor*
CRpcMessage::FindField( const CORE::CString& name ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_type )
        return GUCEF_NULL;
    return m_type->FindFieldByName( name );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetByName( const CORE::CString& name, const CORE::CVariant& value )
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = FindField( name );
    return GUCEF_NULL != field && Set( field->GetNumber(), value );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::GetByName( const CORE::CString& name, CORE::CVariant& value ) const
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = FindField( name );
    return GUCEF_NULL != field && Get( field->GetNumber(), value );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetBool( UInt32 number, bool value )
{GUCEF_TRACE;

    return Set( number, CORE::CVariant( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetInt32( UInt32 number, Int32 value )
{GUCEF_TRACE;

    return Set( number, CORE::CVariant( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetInt64( UInt32 number, Int64 value )
{GUCEF_TRACE;

    return Set( number, CORE::CVariant( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetUInt32( UInt32 number, UInt32 value )
{GUCEF_TRACE;

    return Set( number, CORE::CVariant( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetUInt64( UInt32 number, UInt64 value )
{GUCEF_TRACE;

    return Set( number, CORE::CVariant( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetFloat64( UInt32 number, Float64 value )
{GUCEF_TRACE;

    return Set( number, CORE::CVariant( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetString( UInt32 number, const CORE::CString& value )
{GUCEF_TRACE;

    return Set( number, CORE::CVariant( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetBytes( UInt32 number, const void* data, UInt32 size )
{GUCEF_TRACE;

    return Set( number, CORE::CVariant( data, size, GUCEF_DATATYPE_BINARY_BLOB ) );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetBytes( UInt32 number, const CORE::CDynamicBuffer& data )
{GUCEF_TRACE;

    return Set( number, CORE::CVariant( data.GetConstBufferPtr(), data.GetDataSize(), GUCEF_DATATYPE_BINARY_BLOB ) );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetDecimal( UInt32 number, const CORE::CDecimal& value )
{GUCEF_TRACE;

    return Set( number, CORE::CVariant( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetDate( UInt32 number, Int32 daysSinceEpoch )
{GUCEF_TRACE;

    return Set( number, CORE::CDataTableValues::Date( daysSinceEpoch ) );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetTimestamp( UInt32 number, Int64 microsSinceEpoch )
{GUCEF_TRACE;

    return Set( number, CORE::CDataTableValues::Timestamp( microsSinceEpoch, CORE::DATATABLE_TIMEUNIT_MICROSECONDS ) );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetDateTime( UInt32 number, const CORE::CDateTime& value )
{GUCEF_TRACE;

    return Set( number, CRpcMessageCodec::DateTimeToVariant( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetDuration( UInt32 number, Int64 micros )
{GUCEF_TRACE;

    return Set( number, CORE::CVariant( micros ) );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetEnum( UInt32 number, Int32 value )
{GUCEF_TRACE;

    return Set( number, CORE::CVariant( value ) );
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetDataNode( UInt32 number, const CORE::CDataNode& value )
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer;
    UInt32 written = 0;
    CORE::CDataNodeBinarySerializerOptions options;
    if ( !CORE::CDataNodeBinarySerializer::Serialize( options, value, 0, buffer, written ) )
        return false;
    return Set( number, CORE::CVariant( buffer.GetConstBufferPtr(), written, GUCEF_DATATYPE_BINARY_BLOB ) );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::GetBool( UInt32 number ) const
{GUCEF_TRACE;

    CORE::CVariant v;
    return Get( number, v ) && v.AsBool();
}

/*-------------------------------------------------------------------------*/

Int32
CRpcMessage::GetInt32( UInt32 number ) const
{GUCEF_TRACE;

    CORE::CVariant v;
    return Get( number, v ) ? v.AsInt32() : 0;
}

/*-------------------------------------------------------------------------*/

Int64
CRpcMessage::GetInt64( UInt32 number ) const
{GUCEF_TRACE;

    CORE::CVariant v;
    return Get( number, v ) ? v.AsInt64() : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcMessage::GetUInt32( UInt32 number ) const
{GUCEF_TRACE;

    CORE::CVariant v;
    return Get( number, v ) ? v.AsUInt32() : 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CRpcMessage::GetUInt64( UInt32 number ) const
{GUCEF_TRACE;

    CORE::CVariant v;
    return Get( number, v ) ? v.AsUInt64() : 0;
}

/*-------------------------------------------------------------------------*/

Float64
CRpcMessage::GetFloat64( UInt32 number ) const
{GUCEF_TRACE;

    CORE::CVariant v;
    return Get( number, v ) ? v.AsFloat64() : 0.0;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRpcMessage::GetString( UInt32 number ) const
{GUCEF_TRACE;

    CORE::CVariant v;
    return Get( number, v ) ? v.AsString() : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::GetBytes( UInt32 number, CORE::CDynamicBuffer& data ) const
{GUCEF_TRACE;

    CORE::CVariant v;
    if ( !Get( number, v ) )
        return false;
    data.Clear( true );
    if ( v.IsBinary() && v.ByteSize( false ) > 0 )
        data.CopyFrom( v.ByteSize( false ), v.AsVoidPtr() );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CDecimal
CRpcMessage::GetDecimal( UInt32 number ) const
{GUCEF_TRACE;

    CORE::CVariant v;
    CORE::CDecimal decimal;
    if ( Get( number, v ) )
        v.GetDecimal( decimal );
    return decimal;
}

/*-------------------------------------------------------------------------*/

Int32
CRpcMessage::GetDate( UInt32 number ) const
{GUCEF_TRACE;

    CORE::CVariant v;
    return Get( number, v ) ? v.AsInt32() : 0;
}

/*-------------------------------------------------------------------------*/

Int64
CRpcMessage::GetTimestamp( UInt32 number ) const
{GUCEF_TRACE;

    CORE::CVariant v;
    Int64 micros = 0;
    if ( Get( number, v ) )
        CORE::CDataTableValues::TicksFromVariant( v, CORE::DATATABLE_TIMEUNIT_MICROSECONDS, micros );
    return micros;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
CRpcMessage::GetDateTime( UInt32 number ) const
{GUCEF_TRACE;

    CORE::CVariant v;
    if ( Get( number, v ) )
        return v.AsDateTime();
    return CORE::CDateTime();
}

/*-------------------------------------------------------------------------*/

Int64
CRpcMessage::GetDuration( UInt32 number ) const
{GUCEF_TRACE;

    return GetInt64( number );
}

/*-------------------------------------------------------------------------*/

Int32
CRpcMessage::GetEnum( UInt32 number ) const
{GUCEF_TRACE;

    return GetInt32( number );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::GetDataNode( UInt32 number, CORE::CDataNode& value ) const
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer;
    if ( !GetBytes( number, buffer ) )
        return false;
    value.Clear();
    if ( 0 == buffer.GetDataSize() )
        return true;
    UInt32 bytesRead = 0;
    CORE::CDataNodeBinarySerializerOptions options;
    return CORE::CDataNodeBinarySerializer::Deserialize( options, false, value, 0, buffer, bytesRead );
}

/*-------------------------------------------------------------------------*/

CRpcMessage*
CRpcMessage::GetMutableMessage( UInt32 number )
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = GUCEF_NULL;
    SSlot* slot = GetSlot( number, &field );
    if ( GUCEF_NULL == slot || !field->IsSingular() || RPC_FIELDTYPE_MESSAGE != field->GetType() )
        return GUCEF_NULL;
    if ( GUCEF_NULL == slot->message )
        slot->message = NewNested( *field );
    slot->isSet = true;
    return slot->message;
}

/*-------------------------------------------------------------------------*/

const CRpcMessage*
CRpcMessage::GetMessage( UInt32 number ) const
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = GUCEF_NULL;
    const SSlot* slot = GetSlot( number, &field );
    if ( GUCEF_NULL == slot || !field->IsSingular() || RPC_FIELDTYPE_MESSAGE != field->GetType() || !slot->isSet )
        return GUCEF_NULL;
    return slot->message;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetMessage( UInt32 number, const CRpcMessage& value )
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = GUCEF_NULL;
    SSlot* slot = GetSlot( number, &field );
    if ( GUCEF_NULL == slot || !field->IsSingular() || RPC_FIELDTYPE_MESSAGE != field->GetType() )
        return false;
    if ( value.m_type != field->GetMessageType() )
        return false;
    GUCEF_DELETE slot->message;
    slot->message = GUCEF_NEW CRpcMessage( value );
    slot->isSet = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::Add( UInt32 number, const CORE::CVariant& value )
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = GUCEF_NULL;
    SSlot* slot = GetSlot( number, &field );
    if ( GUCEF_NULL == slot || !field->IsRepeated() || RPC_FIELDTYPE_MESSAGE == field->GetType() )
        return false;
    CORE::CVariant converted;
    if ( !CRpcMessageCodec::ConvertValue( *field, field->GetType(), value, converted ) )
        return false;
    slot->values.push_back( converted );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcMessage::GetCount( UInt32 number ) const
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = GUCEF_NULL;
    const SSlot* slot = GetSlot( number, &field );
    if ( GUCEF_NULL == slot )
        return 0;
    if ( field->IsSingular() )
        return slot->isSet ? 1 : 0;
    if ( field->IsMap() )
        return (UInt32) slot->mapKeys.size();
    return RPC_FIELDTYPE_MESSAGE == field->GetType() ? (UInt32) slot->messages.size() : (UInt32) slot->values.size();
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::GetAt( UInt32 number, UInt32 index, CORE::CVariant& value ) const
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = GUCEF_NULL;
    const SSlot* slot = GetSlot( number, &field );
    if ( GUCEF_NULL == slot || !field->IsRepeated() || index >= slot->values.size() )
        return false;
    if ( slot->values[ index ].IsDynamicMemoryLinked() )
        value.LinkTo( slot->values[ index ] );
    else
        value = slot->values[ index ];
    return true;
}

/*-------------------------------------------------------------------------*/

CRpcMessage*
CRpcMessage::AddMessage( UInt32 number )
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = GUCEF_NULL;
    SSlot* slot = GetSlot( number, &field );
    if ( GUCEF_NULL == slot || !field->IsRepeated() || RPC_FIELDTYPE_MESSAGE != field->GetType() )
        return GUCEF_NULL;
    CRpcMessage* nested = NewNested( *field );
    slot->messages.push_back( nested );
    return nested;
}

/*-------------------------------------------------------------------------*/

const CRpcMessage*
CRpcMessage::GetMessageAt( UInt32 number, UInt32 index ) const
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = GUCEF_NULL;
    const SSlot* slot = GetSlot( number, &field );
    if ( GUCEF_NULL == slot || !field->IsRepeated() || index >= slot->messages.size() )
        return GUCEF_NULL;
    return slot->messages[ index ];
}

/*-------------------------------------------------------------------------*/

CRpcMessage*
CRpcMessage::GetMutableMessageAt( UInt32 number, UInt32 index )
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = GUCEF_NULL;
    SSlot* slot = GetSlot( number, &field );
    if ( GUCEF_NULL == slot || !field->IsRepeated() || index >= slot->messages.size() )
        return GUCEF_NULL;
    return slot->messages[ index ];
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::SetMapEntry( UInt32 number, const CORE::CVariant& key, const CORE::CVariant& value )
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = GUCEF_NULL;
    SSlot* slot = GetSlot( number, &field );
    if ( GUCEF_NULL == slot || !field->IsMap() || RPC_FIELDTYPE_MESSAGE == field->GetType() )
        return false;
    CORE::CVariant convertedKey;
    CORE::CVariant convertedValue;
    if ( !CRpcMessageCodec::ConvertValue( *field, field->GetMapKeyType(), key, convertedKey ) ||
         !CRpcMessageCodec::ConvertValue( *field, field->GetType(), value, convertedValue ) )
        return false;
    Int32 index = FindMapIndex( *slot, convertedKey );
    if ( index >= 0 )
        slot->values[ (UInt32) index ] = convertedValue;
    else
    {
        slot->mapKeys.push_back( convertedKey );
        slot->values.push_back( convertedValue );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::GetMapEntry( UInt32 number, const CORE::CVariant& key, CORE::CVariant& value ) const
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = GUCEF_NULL;
    const SSlot* slot = GetSlot( number, &field );
    if ( GUCEF_NULL == slot || !field->IsMap() || RPC_FIELDTYPE_MESSAGE == field->GetType() )
        return false;
    CORE::CVariant convertedKey;
    if ( !CRpcMessageCodec::ConvertValue( *field, field->GetMapKeyType(), key, convertedKey ) )
        return false;
    Int32 index = FindMapIndex( *slot, convertedKey );
    if ( index < 0 )
        return false;
    value = slot->values[ (UInt32) index ];
    return true;
}

/*-------------------------------------------------------------------------*/

CRpcMessage*
CRpcMessage::GetMutableMapMessage( UInt32 number, const CORE::CVariant& key )
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = GUCEF_NULL;
    SSlot* slot = GetSlot( number, &field );
    if ( GUCEF_NULL == slot || !field->IsMap() || RPC_FIELDTYPE_MESSAGE != field->GetType() )
        return GUCEF_NULL;
    CORE::CVariant convertedKey;
    if ( !CRpcMessageCodec::ConvertValue( *field, field->GetMapKeyType(), key, convertedKey ) )
        return GUCEF_NULL;
    Int32 index = FindMapIndex( *slot, convertedKey );
    if ( index >= 0 )
        return slot->messages[ (UInt32) index ];
    CRpcMessage* nested = NewNested( *field );
    slot->mapKeys.push_back( convertedKey );
    slot->messages.push_back( nested );
    return nested;
}

/*-------------------------------------------------------------------------*/

const CRpcMessage*
CRpcMessage::GetMapMessage( UInt32 number, const CORE::CVariant& key ) const
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = GUCEF_NULL;
    const SSlot* slot = GetSlot( number, &field );
    if ( GUCEF_NULL == slot || !field->IsMap() || RPC_FIELDTYPE_MESSAGE != field->GetType() )
        return GUCEF_NULL;
    CORE::CVariant convertedKey;
    if ( !CRpcMessageCodec::ConvertValue( *field, field->GetMapKeyType(), key, convertedKey ) )
        return GUCEF_NULL;
    Int32 index = FindMapIndex( *slot, convertedKey );
    return index >= 0 ? slot->messages[ (UInt32) index ] : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::GetMapKeys( UInt32 number, TVariantVector& keys ) const
{GUCEF_TRACE;

    const CRpcFieldDescriptor* field = GUCEF_NULL;
    const SSlot* slot = GetSlot( number, &field );
    if ( GUCEF_NULL == slot || !field->IsMap() )
        return false;
    // the order of the encoding: sorted by key
    GUCEF::vector< UInt32 > order;
    CRpcMessageCodec::SortMapKeyOrder( field->GetMapKeyType(), slot->mapKeys, order );
    keys.resize( order.size() );
    for ( UInt32 i=0; i<order.size(); ++i )
        keys[ i ] = slot->mapKeys[ order[ i ] ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::Encode( CORE::CDynamicBuffer& target, UInt32 offset, UInt32* bytesWritten ) const
{GUCEF_TRACE;

    CORE::CBinaryWireWriter writer( target, offset );
    CRpcStatus status;
    if ( !CRpcMessageCodec::Encode( *this, writer, &status ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcMessage: cannot encode a " << ( GUCEF_NULL != m_type ? m_type->GetFullName() : CORE::CString( "unbound message" ) ) << ": " << status.ToString() );
        return false;
    }
    if ( GUCEF_NULL != bytesWritten )
        *bytesWritten = writer.GetOffset() - offset;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::Encode( CORE::CDynamicBuffer& target ) const
{GUCEF_TRACE;

    target.SetDataSize( 0 );
    return Encode( target, 0 );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::Decode( const void* data, UInt32 size, CRpcStatus& status, bool linkBinary )
{GUCEF_TRACE;

    Reset();
    status = CRpcStatus::Ok;
    if ( GUCEF_NULL == m_type )
    {
        status.Set( RPC_STATUS_INTERNAL, "message is not bound to a type" );
        return false;
    }
    if ( !CRpcMessageCodec::Decode( *this, static_cast< const UInt8* >( data ), size, 1, linkBinary, status ) )
    {
        Reset();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcMessage::Detach( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_slots.size(); ++i )
    {
        SSlot& slot = m_slots[ i ];
        if ( slot.value.IsDynamicMemoryLinked() )
            slot.value = CORE::CVariant( slot.value, false );
        for ( UInt32 n=0; n<slot.values.size(); ++n )
            if ( slot.values[ n ].IsDynamicMemoryLinked() )
                slot.values[ n ] = CORE::CVariant( slot.values[ n ], false );
        if ( GUCEF_NULL != slot.message )
            slot.message->Detach();
        for ( UInt32 n=0; n<slot.messages.size(); ++n )
            slot.messages[ n ]->Detach();
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcMessage::GetEncodedSize( void ) const
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer;
    UInt32 written = 0;
    if ( !Encode( buffer, 0, &written ) )
        return 0;
    return written;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcMessage::GetUnknownFieldCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_unknown.size();
}

/*-------------------------------------------------------------------------*/

void
CRpcMessage::SetPreserveUnknownFields( bool preserve )
{GUCEF_TRACE;

    m_preserveUnknown = preserve;
}

/*-------------------------------------------------------------------------*/

namespace {

/** The JSON form of one value */
CORE::CVariant
ToJsonValue( const CRpcFieldDescriptor& field, TRpcFieldType type, const CORE::CVariant& value )
{
    switch ( type )
    {
        case RPC_FIELDTYPE_BYTES:
        case RPC_FIELDTYPE_DATANODE:
            return CORE::CVariant( CORE::Base64Encode( value.AsVoidPtr(), value.IsBinary() ? value.ByteSize( false ) : 0 ) );
        case RPC_FIELDTYPE_DECIMAL:
        {
            CORE::CDecimal decimal;
            value.GetDecimal( decimal );
            return CORE::CVariant( decimal.ToString() );
        }
        case RPC_FIELDTYPE_DATE:
            return CORE::CVariant( CORE::CDataTableValues::DateToString( value.AsInt32() ) );
        case RPC_FIELDTYPE_TIMESTAMP:
        {
            Int64 micros = 0;
            CORE::CDataTableValues::TicksFromVariant( value, CORE::DATATABLE_TIMEUNIT_MICROSECONDS, micros );
            return CORE::CVariant( CORE::CDataTableValues::TimestampToString( micros, CORE::DATATABLE_TIMEUNIT_MICROSECONDS, true ) );
        }
        case RPC_FIELDTYPE_DATETIME:
            return CORE::CVariant( CRpcMessageCodec::DateTimeToVariant( value.AsDateTime() ).AsString() );
        case RPC_FIELDTYPE_ENUM:
        {
            if ( GUCEF_NULL != field.GetEnumType() )
            {
                const CRpcEnumDescriptor::SValue* known = field.GetEnumType()->FindValueByNumber( value.AsInt32() );
                if ( GUCEF_NULL != known )
                    return CORE::CVariant( known->name );
            }
            return CORE::CVariant( value.AsInt64() );
        }
        case RPC_FIELDTYPE_INT8:
        case RPC_FIELDTYPE_INT16:
        case RPC_FIELDTYPE_INT32:
        case RPC_FIELDTYPE_INT64:
        case RPC_FIELDTYPE_DURATION:
            return CORE::CVariant( value.AsInt64() );
        case RPC_FIELDTYPE_UINT8:
        case RPC_FIELDTYPE_UINT16:
        case RPC_FIELDTYPE_UINT32:
        case RPC_FIELDTYPE_UINT64:
            return CORE::CVariant( value.AsUInt64() );
        case RPC_FIELDTYPE_STRING:
            return CORE::CVariant( value.AsString() );
        default:
            return value;
    }
}

/** A value from its JSON form */
bool
FromJsonValue( const CRpcFieldDescriptor& field, TRpcFieldType type, const CORE::CVariant& json, CORE::CVariant& value )
{
    if ( ( RPC_FIELDTYPE_BYTES == type || RPC_FIELDTYPE_DATANODE == type ) && json.IsString() )
    {
        CORE::CDynamicBuffer bytes;
        CORE::CString text = json.AsString();
        if ( !text.IsNULLOrEmpty() )
            bytes.CopyAndDecodeBase64From( text, 0 );
        value.Set( bytes.GetConstBufferPtr(), bytes.GetDataSize(), GUCEF_DATATYPE_BINARY_BLOB );
        return true;
    }
    if ( RPC_FIELDTYPE_DATETIME == type && json.IsString() )
    {
        CORE::CDateTime dateTime;
        if ( !dateTime.FromIso8601DateTimeString( json.AsString() ) )
            return false;
        return CRpcMessageCodec::ConvertValue( field, type, CORE::CVariant( dateTime ), value );
    }
    return CRpcMessageCodec::ConvertValue( field, type, json, value );
}

}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::ToDataNode( CORE::CDataNode& target, bool redactSensitive ) const
{GUCEF_TRACE;

    target.SetNodeType( GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL == m_type )
        return false;

    const CORE::CString marker = GUCEF_CORE_REDACTION_MARKER;
    for ( UInt32 i=0; i<m_type->GetFieldCount(); ++i )
    {
        const CRpcFieldDescriptor& field = m_type->GetFieldAt( i );
        const SSlot& slot = m_slots[ i ];
        const CORE::CString& name = field.GetName();
        const bool redact = redactSensitive && field.IsSensitive();
        const TRpcFieldType type = field.GetType();

        if ( field.IsSingular() )
        {
            if ( !slot.isSet )
                continue;
            if ( redact )
                target.SetAttribute( name, CORE::CVariant( marker ) );
            else if ( RPC_FIELDTYPE_MESSAGE == type )
            {
                CORE::CDataNode* child = target.AddChild( name, GUCEF_DATATYPE_OBJECT );
                if ( GUCEF_NULL != slot.message )
                    slot.message->ToDataNode( *child, redactSensitive );
            }
            else
                target.SetAttribute( name, ToJsonValue( field, type, slot.value ) );
        }
        else if ( field.IsRepeated() )
        {
            if ( slot.values.empty() && slot.messages.empty() )
                continue;
            if ( redact )
            {
                target.SetAttribute( name, CORE::CVariant( marker ) );
                continue;
            }
            CORE::CDataNode* array = target.AddChild( name, GUCEF_DATATYPE_ARRAY );
            if ( RPC_FIELDTYPE_MESSAGE == type )
            {
                for ( UInt32 n=0; n<slot.messages.size(); ++n )
                {
                    CORE::CDataNode* element = array->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                    slot.messages[ n ]->ToDataNode( *element, redactSensitive );
                }
            }
            else
            {
                for ( UInt32 n=0; n<slot.values.size(); ++n )
                    array->AddValueAsChild( ToJsonValue( field, type, slot.values[ n ] ) );
            }
        }
        else
        {
            if ( slot.mapKeys.empty() )
                continue;
            if ( redact )
            {
                target.SetAttribute( name, CORE::CVariant( marker ) );
                continue;
            }
            CORE::CDataNode* object = target.AddChild( name, GUCEF_DATATYPE_OBJECT );
            GUCEF::vector< UInt32 > order;
            CRpcMessageCodec::SortMapKeyOrder( field.GetMapKeyType(), slot.mapKeys, order );
            for ( UInt32 n=0; n<order.size(); ++n )
            {
                UInt32 index = order[ n ];
                CORE::CString keyText = slot.mapKeys[ index ].AsString();
                if ( RPC_FIELDTYPE_MESSAGE == type )
                {
                    CORE::CDataNode* child = object->AddChild( keyText, GUCEF_DATATYPE_OBJECT );
                    slot.messages[ (UInt32) index ]->ToDataNode( *child, redactSensitive );
                }
                else
                    object->SetAttribute( keyText, ToJsonValue( field, type, slot.values[ (UInt32) index ] ) );
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::FromDataNode( const CORE::CDataNode& source, CRpcStatus& status )
{GUCEF_TRACE;

    status = CRpcStatus::Ok;
    if ( GUCEF_NULL == m_type )
    {
        status.Set( RPC_STATUS_INTERNAL, "message is not bound to a type" );
        return false;
    }
    Reset();

    // scalar fields are attributes
    CORE::CDataNode::TAttributeMap::const_iterator a = source.AttributeBegin();
    while ( a != source.AttributeEnd() )
    {
        const CRpcFieldDescriptor* field = m_type->FindFieldByName( (*a).first );
        if ( GUCEF_NULL == field )
        {
            status.Set( RPC_STATUS_INVALID_ARGUMENT, "unknown field \"" + (*a).first + "\" for " + m_type->GetFullName() );
            return false;
        }
        if ( !field->IsSingular() || RPC_FIELDTYPE_MESSAGE == field->GetType() )
        {
            status.Set( RPC_STATUS_INVALID_ARGUMENT, "field \"" + field->GetName() + "\" needs an array or object" );
            return false;
        }
        SSlot& slot = m_slots[ field->GetSlotIndex() ];
        if ( !FromJsonValue( *field, field->GetType(), (*a).second, slot.value ) )
        {
            status.Set( RPC_STATUS_INVALID_ARGUMENT, "invalid value for field \"" + field->GetName() + "\"" );
            return false;
        }
        slot.isSet = true;
        ++a;
    }

    // nested messages, repeated and map fields are child nodes
    CORE::CDataNode::const_iterator c = source.ConstBegin();
    while ( c != source.ConstEnd() )
    {
        const CORE::CDataNode& child = *(*c);
        ++c;
        const CRpcFieldDescriptor* field = m_type->FindFieldByName( child.GetName() );
        if ( GUCEF_NULL == field )
        {
            status.Set( RPC_STATUS_INVALID_ARGUMENT, "unknown field \"" + child.GetName() + "\" for " + m_type->GetFullName() );
            return false;
        }
        SSlot& slot = m_slots[ field->GetSlotIndex() ];
        const TRpcFieldType type = field->GetType();
        if ( field->IsSingular() )
        {
            if ( RPC_FIELDTYPE_MESSAGE != type )
            {
                status.Set( RPC_STATUS_INVALID_ARGUMENT, "field \"" + field->GetName() + "\" is not a message" );
                return false;
            }
            CRpcMessage* nested = NewNested( *field );
            if ( !nested->FromDataNode( child, status ) )
            {
                GUCEF_DELETE nested;
                return false;
            }
            GUCEF_DELETE slot.message;
            slot.message = nested;
            slot.isSet = true;
        }
        else if ( field->IsRepeated() )
        {
            CORE::CDataNode::const_iterator e = child.ConstBegin();
            while ( e != child.ConstEnd() )
            {
                const CORE::CDataNode& element = *(*e);
                ++e;
                if ( RPC_FIELDTYPE_MESSAGE == type )
                {
                    CRpcMessage* nested = NewNested( *field );
                    if ( !nested->FromDataNode( element, status ) )
                    {
                        GUCEF_DELETE nested;
                        return false;
                    }
                    slot.messages.push_back( nested );
                }
                else
                {
                    CORE::CVariant value;
                    if ( !FromJsonValue( *field, type, element.GetValue(), value ) )
                    {
                        status.Set( RPC_STATUS_INVALID_ARGUMENT, "invalid element for field \"" + field->GetName() + "\"" );
                        return false;
                    }
                    slot.values.push_back( value );
                }
            }
        }
        else
        {
            if ( RPC_FIELDTYPE_MESSAGE == type )
            {
                CORE::CDataNode::const_iterator e = child.ConstBegin();
                while ( e != child.ConstEnd() )
                {
                    const CORE::CDataNode& entry = *(*e);
                    ++e;
                    CORE::CVariant key;
                    if ( !CRpcMessageCodec::ConvertValue( *field, field->GetMapKeyType(), CORE::CVariant( entry.GetName() ), key ) )
                    {
                        status.Set( RPC_STATUS_INVALID_ARGUMENT, "invalid map key for field \"" + field->GetName() + "\"" );
                        return false;
                    }
                    CRpcMessage* nested = NewNested( *field );
                    if ( !nested->FromDataNode( entry, status ) )
                    {
                        GUCEF_DELETE nested;
                        return false;
                    }
                    slot.mapKeys.push_back( key );
                    slot.messages.push_back( nested );
                }
            }
            else
            {
                CORE::CDataNode::TAttributeMap::const_iterator e = child.AttributeBegin();
                while ( e != child.AttributeEnd() )
                {
                    CORE::CVariant key;
                    CORE::CVariant value;
                    if ( !CRpcMessageCodec::ConvertValue( *field, field->GetMapKeyType(), CORE::CVariant( (*e).first ), key ) ||
                         !FromJsonValue( *field, type, (*e).second, value ) )
                    {
                        status.Set( RPC_STATUS_INVALID_ARGUMENT, "invalid map entry for field \"" + field->GetName() + "\"" );
                        return false;
                    }
                    slot.mapKeys.push_back( key );
                    slot.values.push_back( value );
                    ++e;
                }
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessage::Validate( CRpcStatus& status ) const
{GUCEF_TRACE;

    status = CRpcStatus::Ok;
    if ( GUCEF_NULL == m_type )
    {
        status.Set( RPC_STATUS_INTERNAL, "message is not bound to a type" );
        return false;
    }
    for ( UInt32 i=0; i<m_type->GetFieldCount(); ++i )
    {
        const CRpcFieldDescriptor& field = m_type->GetFieldAt( i );
        const SSlot& slot = m_slots[ i ];
        if ( field.IsRequired() && field.IsSingular() && !slot.isSet )
        {
            status.Set( RPC_STATUS_INVALID_ARGUMENT, "required field \"" + field.GetName() + "\" of " + m_type->GetFullName() + " is not set" );
            return false;
        }
        if ( RPC_FIELDTYPE_ENUM == field.GetType() && GUCEF_NULL != field.GetEnumType() && field.IsSingular() && slot.isSet &&
             GUCEF_NULL == field.GetEnumType()->FindValueByNumber( slot.value.AsInt32() ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "RpcMessage: field \"" << field.GetName() << "\" of " << m_type->GetFullName() <<
                               " holds enum value " << slot.value.AsInt32() << " which this side does not know" );
        }
        if ( GUCEF_NULL != slot.message && !slot.message->Validate( status ) )
            return false;
        for ( UInt32 n=0; n<slot.messages.size(); ++n )
            if ( !slot.messages[ n ]->Validate( status ) )
                return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
