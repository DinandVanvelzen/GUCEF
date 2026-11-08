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

#include <stdio.h>
#include <string.h>
#include <algorithm>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCOMCORE_CRpcMessageCodec.h"

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

namespace {

/** Orders entry indices by their map keys (CRpcMessageCodec::CompareMapKeys), for std::stable_sort */
class CMapKeyIndexOrder
{
    public:

    CMapKeyIndexOrder( TRpcFieldType keyType, const GUCEF::vector< CORE::CVariant >& keys )
        : m_keyType( keyType )
        , m_keys( &keys )
    {}

    bool operator()( UInt32 a, UInt32 b ) const
    {
        return CRpcMessageCodec::CompareMapKeys( m_keyType, (*m_keys)[ a ], (*m_keys)[ b ] ) < 0;
    }

    private:

    TRpcFieldType m_keyType;
    const GUCEF::vector< CORE::CVariant >* m_keys;
};

/**
 *  A key that occurs more than once in the entries of a map keeps the place of its first entry and the value of its
 *  last one (the replacing rule of a decode). One sort instead of a lookup per entry.
 */
void
RemoveRepeatedMapKeys( TRpcFieldType keyType, GUCEF::vector< CORE::CVariant >& keys, GUCEF::vector< CORE::CVariant >& values,
                       GUCEF::vector< CRpcMessage* >& messages, bool messageValues )
{
    const UInt32 count = (UInt32) keys.size();
    if ( count < 2 )
        return;
    GUCEF::vector< UInt32 > order;
    CRpcMessageCodec::SortMapKeyOrder( keyType, keys, order );
    bool repeated = false;
    for ( UInt32 i=1; i<count && !repeated; ++i )
        repeated = 0 == CRpcMessageCodec::CompareMapKeys( keyType, keys[ order[ i-1 ] ], keys[ order[ i ] ] );
    if ( !repeated )
        return;

    GUCEF::vector< UInt8 > keep( count, 1 );
    UInt32 i = 0;
    while ( i < count )
    {
        UInt32 j = i;
        while ( j+1 < count && 0 == CRpcMessageCodec::CompareMapKeys( keyType, keys[ order[ i ] ], keys[ order[ j+1 ] ] ) )
            ++j;
        if ( j > i )
        {
            // stable: order[ i ] is the first entry of the key, order[ j ] the last
            UInt32 first = order[ i ];
            UInt32 last = order[ j ];
            if ( messageValues )
            {
                GUCEF_DELETE messages[ first ];
                messages[ first ] = messages[ last ];
                messages[ last ] = GUCEF_NULL;
                for ( UInt32 k=i+1; k<j; ++k )
                {
                    GUCEF_DELETE messages[ order[ k ] ];
                    messages[ order[ k ] ] = GUCEF_NULL;
                }
            }
            else
                values[ first ] = values[ last ];
            for ( UInt32 k=i+1; k<=j; ++k )
                keep[ order[ k ] ] = 0;
        }
        i = j+1;
    }

    UInt32 kept = 0;
    for ( UInt32 n=0; n<count; ++n )
    {
        if ( 0 == keep[ n ] )
            continue;
        if ( kept != n )
        {
            keys[ kept ] = keys[ n ];
            if ( messageValues )
                messages[ kept ] = messages[ n ];
            else
                values[ kept ] = values[ n ];
        }
        ++kept;
    }
    keys.resize( kept );
    if ( messageValues )
        messages.resize( kept );
    else
        values.resize( kept );
}

bool
Refuse( CRpcStatus* status, const CRpcFieldDescriptor& field, const char* why )
{
    if ( GUCEF_NULL != status )
    {
        CORE::CString message = "field " + CORE::ToString( field.GetNumber() ) + " (" + field.GetName() + "): " + why;
        status->Set( RPC_STATUS_INVALID_ARGUMENT, message );
    }
    return false;
}

bool
RefuseDecode( CRpcStatus& status, const CORE::CString& why )
{
    status.Set( RPC_STATUS_INVALID_ARGUMENT, why );
    return false;
}

bool
IntegerRange( TRpcFieldType type, Int64& minValue, UInt64& maxValue, bool& isSigned )
{
    switch ( type )
    {
        case RPC_FIELDTYPE_INT8: minValue = -128; maxValue = 127; isSigned = true; return true;
        case RPC_FIELDTYPE_INT16: minValue = -32768; maxValue = 32767; isSigned = true; return true;
        case RPC_FIELDTYPE_INT32:
        case RPC_FIELDTYPE_ENUM:
        case RPC_FIELDTYPE_DATE: minValue = GUCEF_INT32MIN; maxValue = GUCEF_INT32MAX; isSigned = true; return true;
        case RPC_FIELDTYPE_INT64:
        case RPC_FIELDTYPE_TIMESTAMP:
        case RPC_FIELDTYPE_DURATION: minValue = GUCEF_INT64MIN; maxValue = (UInt64) GUCEF_INT64MAX; isSigned = true; return true;
        case RPC_FIELDTYPE_UINT8: minValue = 0; maxValue = 0xFF; isSigned = false; return true;
        case RPC_FIELDTYPE_UINT16: minValue = 0; maxValue = 0xFFFF; isSigned = false; return true;
        case RPC_FIELDTYPE_UINT32: minValue = 0; maxValue = 0xFFFFFFFFULL; isSigned = false; return true;
        case RPC_FIELDTYPE_UINT64: minValue = 0; maxValue = 0xFFFFFFFFFFFFFFFFULL; isSigned = false; return true;
        default: return false;
    }
}

/** A variant holding the integer in the declared width */
CORE::CVariant
TypedInteger( TRpcFieldType type, Int64 signedValue, UInt64 unsignedValue )
{
    switch ( type )
    {
        case RPC_FIELDTYPE_INT8: return CORE::CVariant( (Int8) signedValue );
        case RPC_FIELDTYPE_INT16: return CORE::CVariant( (Int16) signedValue );
        case RPC_FIELDTYPE_INT32:
        case RPC_FIELDTYPE_ENUM: return CORE::CVariant( (Int32) signedValue );
        case RPC_FIELDTYPE_INT64:
        case RPC_FIELDTYPE_DURATION: return CORE::CVariant( (Int64) signedValue );
        case RPC_FIELDTYPE_UINT8: return CORE::CVariant( (UInt8) unsignedValue );
        case RPC_FIELDTYPE_UINT16: return CORE::CVariant( (UInt16) unsignedValue );
        case RPC_FIELDTYPE_UINT32: return CORE::CVariant( (UInt32) unsignedValue );
        case RPC_FIELDTYPE_UINT64: return CORE::CVariant( (UInt64) unsignedValue );
        case RPC_FIELDTYPE_DATE: return CORE::CDataTableValues::Date( (Int32) signedValue );
        case RPC_FIELDTYPE_TIMESTAMP: return CORE::CDataTableValues::Timestamp( signedValue, CORE::DATATABLE_TIMEUNIT_MICROSECONDS );
        default: return CORE::CVariant();
    }
}

/** Integer value of a variant: false when it is not an integer (or an integral float) */
bool
IntegerOf( const CORE::CVariant& in, bool& negative, Int64& signedValue, UInt64& unsignedValue )
{
    if ( in.IsUnsignedInteger() )
    {
        unsignedValue = in.AsUInt64();
        negative = false;
        signedValue = unsignedValue <= (UInt64) GUCEF_INT64MAX ? (Int64) unsignedValue : 0;
        return true;
    }
    if ( in.IsSignedInteger() || in.IsBoolean() )
    {
        signedValue = in.AsInt64();
        negative = signedValue < 0;
        unsignedValue = negative ? 0 : (UInt64) signedValue;
        return true;
    }
    if ( in.IsFloat() )
    {
        Float64 f = in.AsFloat64();
        if ( f != f || f < -9.2233720368547758e18 || f >= 1.8446744073709552e19 )
            return false;
        if ( f < 0 )
        {
            signedValue = (Int64) f;
            if ( (Float64) signedValue != f )
                return false;
            negative = true;
            unsignedValue = 0;
            return true;
        }
        unsignedValue = (UInt64) f;
        if ( (Float64) unsignedValue != f )
            return false;
        negative = false;
        signedValue = unsignedValue <= (UInt64) GUCEF_INT64MAX ? (Int64) unsignedValue : 0;
        return true;
    }
    if ( in.IsString() )
    {
        CORE::CString text = in.AsString().Trim( true ).Trim( false );
        if ( text.IsNULLOrEmpty() )
            return false;
        CORE::CAsciiString ascii = CORE::ToAsciiString( text );
        const char* c = ascii.C_String();
        negative = '-' == *c;
        if ( negative || '+' == *c )
            ++c;
        if ( '\0' == *c )
            return false;
        UInt64 magnitude = 0;
        while ( '\0' != *c )
        {
            if ( *c < '0' || *c > '9' )
                return false;
            UInt64 digit = (UInt64) ( *c - '0' );
            if ( magnitude > ( 0xFFFFFFFFFFFFFFFFULL - digit ) / 10 )
                return false;
            magnitude = magnitude * 10 + digit;
            ++c;
        }
        if ( negative )
        {
            if ( magnitude > (UInt64) GUCEF_INT64MAX + 1 )
                return false;
            signedValue = magnitude == (UInt64) GUCEF_INT64MAX + 1 ? GUCEF_INT64MIN : -(Int64) magnitude;
            unsignedValue = 0;
        }
        else
        {
            unsignedValue = magnitude;
            signedValue = magnitude <= (UInt64) GUCEF_INT64MAX ? (Int64) magnitude : 0;
        }
        return true;
    }
    return false;
}

bool
StringBytesLess( const CORE::CVariant& a, const CORE::CVariant& b, Int32& result )
{
    CORE::CUtf8String sa = a.AsUtf8String();
    CORE::CUtf8String sb = b.AsUtf8String();
    UInt32 la = sa.ByteSize() > 0 ? sa.ByteSize() - 1 : 0;
    UInt32 lb = sb.ByteSize() > 0 ? sb.ByteSize() - 1 : 0;
    UInt32 common = la < lb ? la : lb;
    int c = common > 0 ? memcmp( sa.C_String(), sb.C_String(), common ) : 0;
    if ( 0 != c )
        result = c < 0 ? -1 : 1;
    else
        result = la == lb ? 0 : ( la < lb ? -1 : 1 );
    return true;
}

}

/*-------------------------------------------------------------------------*/

CORE::CVariant
CRpcMessageCodec::DateTimeToVariant( const CORE::CDateTime& value )
{GUCEF_TRACE;

    Int32 offset = value.GetTimeZoneUTCOffsetInMins();
    UInt32 absOffset = (UInt32) ( offset < 0 ? -offset : offset );
    char text[ 64 ];
    int length = snprintf( text, sizeof( text ), "%04d-%02u-%02uT%02u:%02u:%02u.%03u%c%02u:%02u",
                           (int) value.GetYear(), (unsigned) value.GetMonth(), (unsigned) value.GetDay(),
                           (unsigned) value.GetHours(), (unsigned) value.GetMinutes(), (unsigned) value.GetSeconds(),
                           (unsigned) value.GetMilliseconds(), offset < 0 ? '-' : '+', absOffset / 60, absOffset % 60 );
    CORE::CVariant result;
    if ( length > 0 && length < (int) sizeof( text ) )
        result.Set( text, (UInt32) length + 1, GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING );
    return result;
}

/*-------------------------------------------------------------------------*/

void
CRpcMessageCodec::WriteKey( CORE::CBinaryWireWriter& writer, UInt32 fieldNumber, UInt32 wireType )
{GUCEF_TRACE;

    writer.WriteVarUInt32( ( fieldNumber << 3 ) | wireType );
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcMessageCodec::BeginLen( CORE::CBinaryWireWriter& writer )
{GUCEF_TRACE;

    // One byte holds lengths below 128, the common case; EndLen() moves the body when more is needed
    writer.WriteUInt8( 0 );
    return writer.GetOffset();
}

/*-------------------------------------------------------------------------*/

void
CRpcMessageCodec::EndLen( CORE::CBinaryWireWriter& writer, UInt32 bodyStart )
{GUCEF_TRACE;

    if ( writer.HasFailed() )
        return;
    UInt32 bodyEnd = writer.GetOffset();
    UInt32 length = bodyEnd - bodyStart;
    if ( length < 0x80 )
    {
        *static_cast< UInt8* >( writer.GetBuffer().GetBufferPtr( bodyStart - 1 ) ) = (UInt8) length;
        return;
    }
    UInt8 varint[ CORE::CBinaryWireWriter::MaxVarIntSize ];
    UInt32 varintSize = CORE::CBinaryWireWriter::EncodeVarUInt64( length, varint );
    UInt32 extra = varintSize - 1;
    if ( GUCEF_NULL == writer.Reserve( extra ) )
        return;
    UInt8* base = static_cast< UInt8* >( writer.GetBuffer().GetBufferPtr() );
    memmove( base + bodyStart + extra, base + bodyStart, length );
    memcpy( base + bodyStart - 1, varint, varintSize );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessageCodec::SkipValue( CORE::CBinaryWireReader& reader, UInt32 wireType )
{GUCEF_TRACE;

    switch ( wireType )
    {
        case GUCEF_RPC_WIRETYPE_VARINT: { UInt64 v = 0; return reader.ReadVarUInt64( v ); }
        case GUCEF_RPC_WIRETYPE_FIXED64: return reader.SkipBytes( 8 );
        case GUCEF_RPC_WIRETYPE_FIXED32: return reader.SkipBytes( 4 );
        case GUCEF_RPC_WIRETYPE_LEN: { const UInt8* d = GUCEF_NULL; UInt32 s = 0; return reader.ReadSizedBytes( d, s ); }
        default: return false;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CVariant
CRpcMessageCodec::ZeroValue( const CRpcFieldDescriptor& field, TRpcFieldType type )
{GUCEF_TRACE;

    switch ( type )
    {
        case RPC_FIELDTYPE_BOOL: return CORE::CVariant( false );
        case RPC_FIELDTYPE_FLOAT32: return CORE::CVariant( (Float32) 0 );
        case RPC_FIELDTYPE_FLOAT64: return CORE::CVariant( (Float64) 0 );
        case RPC_FIELDTYPE_STRING: return CORE::CVariant( CORE::CUtf8String::Empty );
        case RPC_FIELDTYPE_BYTES:
        case RPC_FIELDTYPE_DATANODE: return CORE::CVariant( (const void*) GUCEF_NULL, 0, GUCEF_DATATYPE_BINARY_BLOB );
        case RPC_FIELDTYPE_DECIMAL:
        {
            CORE::CDecimal zero;
            if ( field.IsFixedScaleDecimal() )
                CORE::CDecimal::FromUnscaled( 0, (UInt8) field.GetDecimalScale(), zero );
            return CORE::CVariant( zero );
        }
        case RPC_FIELDTYPE_DATETIME:
        {
            CORE::CDateTime epoch;
            CORE::CDataTableValues::DateTimeFromTicks( 0, CORE::DATATABLE_TIMEUNIT_MILLISECONDS, epoch );
            return DateTimeToVariant( epoch );
        }
        case RPC_FIELDTYPE_VARIANT: return CORE::CVariant();
        default: return TypedInteger( type, 0, 0 );
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessageCodec::ConvertValue( const CRpcFieldDescriptor& field, TRpcFieldType type, const CORE::CVariant& in, CORE::CVariant& out )
{GUCEF_TRACE;

    switch ( type )
    {
        case RPC_FIELDTYPE_BOOL:
        {
            if ( in.IsBoolean() || in.IsInteger() || in.IsString() )
            {
                out = in.AsBool();
                return true;
            }
            return false;
        }
        case RPC_FIELDTYPE_INT8:
        case RPC_FIELDTYPE_INT16:
        case RPC_FIELDTYPE_INT32:
        case RPC_FIELDTYPE_INT64:
        case RPC_FIELDTYPE_UINT8:
        case RPC_FIELDTYPE_UINT16:
        case RPC_FIELDTYPE_UINT32:
        case RPC_FIELDTYPE_UINT64:
        case RPC_FIELDTYPE_DURATION:
        {
            bool negative = false;
            Int64 s = 0;
            UInt64 u = 0;
            if ( !IntegerOf( in, negative, s, u ) )
                return false;
            Int64 minValue = 0;
            UInt64 maxValue = 0;
            bool isSigned = false;
            IntegerRange( type, minValue, maxValue, isSigned );
            if ( negative )
            {
                if ( !isSigned || s < minValue )
                    return false;
            }
            else if ( u > maxValue )
                return false;
            out = TypedInteger( type, negative ? s : (Int64) u, u );
            return true;
        }
        case RPC_FIELDTYPE_ENUM:
        {
            if ( in.IsString() && GUCEF_NULL != field.GetEnumType() )
            {
                const CRpcEnumDescriptor::SValue* value = field.GetEnumType()->FindValueByName( in.AsString() );
                if ( GUCEF_NULL != value )
                {
                    out = CORE::CVariant( value->number );
                    return true;
                }
            }
            bool negative = false;
            Int64 s = 0;
            UInt64 u = 0;
            if ( !IntegerOf( in, negative, s, u ) )
                return false;
            if ( negative ? s < GUCEF_INT32MIN : u > (UInt64) GUCEF_INT32MAX )
                return false;
            out = CORE::CVariant( (Int32) ( negative ? s : (Int64) u ) );
            return true;
        }
        case RPC_FIELDTYPE_FLOAT32:
        {
            if ( !in.IsNumber() && !in.IsString() )
                return false;
            out = in.AsFloat32();
            return true;
        }
        case RPC_FIELDTYPE_FLOAT64:
        {
            if ( !in.IsNumber() && !in.IsString() )
                return false;
            out = in.AsFloat64();
            return true;
        }
        case RPC_FIELDTYPE_STRING:
        {
            if ( in.IsBinary() || in.IsMemoryAddress() )
                return false;
            out = in.AsUtf8String();
            return true;
        }
        case RPC_FIELDTYPE_BYTES:
        case RPC_FIELDTYPE_DATANODE:
        {
            if ( in.IsBinary() )
            {
                out.Set( in.AsVoidPtr(), in.ByteSize( false ), GUCEF_DATATYPE_BINARY_BLOB );
                return true;
            }
            if ( RPC_FIELDTYPE_BYTES == type && in.IsString() )
            {
                CORE::CUtf8String utf8 = in.AsUtf8String();
                UInt32 size = utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0;
                out.Set( utf8.C_String(), size, GUCEF_DATATYPE_BINARY_BLOB );
                return true;
            }
            if ( in.IsNULLOrEmpty() )
            {
                out = ZeroValue( field, type );
                return true;
            }
            return false;
        }
        case RPC_FIELDTYPE_DECIMAL:
        {
            CORE::CDecimal decimal;
            if ( in.IsDecimal() || in.IsInteger() || in.IsString() )
            {
                if ( CORE::DECIMALSTATUS_OK != in.GetDecimal( decimal ) )
                    return false;
            }
            else if ( in.IsFloat() )
            {
                if ( CORE::DECIMALSTATUS_OK != CORE::CDecimal::FromFloat64Shortest( in.AsFloat64(), decimal ) )
                    return false;
            }
            else
                return false;
            if ( field.IsFixedScaleDecimal() && decimal.GetScale() != (UInt8) field.GetDecimalScale() )
            {
                CORE::CDecimal rescaled;
                if ( CORE::DECIMALSTATUS_OK != decimal.Rescale( (UInt8) field.GetDecimalScale(), CORE::ROUNDINGMODE_UNNECESSARY, rescaled ) )
                    return false;
                decimal = rescaled;
            }
            out = decimal;
            return true;
        }
        case RPC_FIELDTYPE_DATE:
        {
            UInt8 typeId = in.GetTypeId();
            if ( GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == typeId || GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == typeId )
            {
                out = CORE::CDataTableValues::Date( in.AsInt32() );
                return true;
            }
            if ( in.IsString() )
            {
                CORE::CAsciiString text = CORE::ToAsciiString( in.AsString() );
                Int32 days = 0;
                if ( !CORE::CDataTableValues::ParseDate( text.C_String(), text.Length(), days ) )
                    return false;
                out = CORE::CDataTableValues::Date( days );
                return true;
            }
            bool negative = false;
            Int64 s = 0;
            UInt64 u = 0;
            if ( !IntegerOf( in, negative, s, u ) )
                return false;
            if ( negative ? s < GUCEF_INT32MIN : u > (UInt64) GUCEF_INT32MAX )
                return false;
            out = CORE::CDataTableValues::Date( (Int32) ( negative ? s : (Int64) u ) );
            return true;
        }
        case RPC_FIELDTYPE_TIMESTAMP:
        {
            Int64 micros = 0;
            if ( in.IsString() )
            {
                CORE::CAsciiString text = CORE::ToAsciiString( in.AsString() );
                if ( !CORE::CDataTableValues::ParseTimestamp( text.C_String(), text.Length(), CORE::DATATABLE_TIMEUNIT_MICROSECONDS, micros ) )
                    return false;
            }
            else if ( in.IsInteger() && !in.IsBoolean() )
            {
                UInt8 typeId = in.GetTypeId();
                bool plainInteger = typeId <= GUCEF_DATATYPE_BE_INT64;
                if ( plainInteger )
                {
                    bool negative = false;
                    Int64 s = 0;
                    UInt64 u = 0;
                    if ( !IntegerOf( in, negative, s, u ) || ( !negative && u > (UInt64) GUCEF_INT64MAX ) )
                        return false;
                    micros = negative ? s : (Int64) u;
                }
                else if ( !CORE::CDataTableValues::TicksFromVariant( in, CORE::DATATABLE_TIMEUNIT_MICROSECONDS, micros ) )
                    return false;
            }
            else if ( !CORE::CDataTableValues::TicksFromVariant( in, CORE::DATATABLE_TIMEUNIT_MICROSECONDS, micros ) )
                return false;
            out = CORE::CDataTableValues::Timestamp( micros, CORE::DATATABLE_TIMEUNIT_MICROSECONDS );
            return true;
        }
        case RPC_FIELDTYPE_DATETIME:
        {
            CORE::CDateTime value = in.AsDateTime();
            Int64 ms = 0;
            if ( !CORE::CDataTableValues::TicksFromDateTime( value, CORE::DATATABLE_TIMEUNIT_MILLISECONDS, ms ) )
                return false;
            out = DateTimeToVariant( value );
            return true;
        }
        case RPC_FIELDTYPE_VARIANT:
        {
            out = in;
            return true;
        }
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

Int32
CRpcMessageCodec::CompareMapKeys( TRpcFieldType keyType, const CORE::CVariant& a, const CORE::CVariant& b )
{GUCEF_TRACE;

    switch ( keyType )
    {
        case RPC_FIELDTYPE_STRING:
        {
            Int32 result = 0;
            StringBytesLess( a, b, result );
            return result;
        }
        case RPC_FIELDTYPE_BOOL:
        {
            bool x = a.AsBool();
            bool y = b.AsBool();
            return x == y ? 0 : ( !x ? -1 : 1 );
        }
        case RPC_FIELDTYPE_UINT8:
        case RPC_FIELDTYPE_UINT16:
        case RPC_FIELDTYPE_UINT32:
        case RPC_FIELDTYPE_UINT64:
        {
            UInt64 x = a.AsUInt64();
            UInt64 y = b.AsUInt64();
            return x == y ? 0 : ( x < y ? -1 : 1 );
        }
        default:
        {
            Int64 x = a.AsInt64();
            Int64 y = b.AsInt64();
            return x == y ? 0 : ( x < y ? -1 : 1 );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessageCodec::EncodeValue( CORE::CBinaryWireWriter& writer, const CRpcFieldDescriptor& field, TRpcFieldType type,
                               const CORE::CVariant& value, CRpcStatus* status )
{GUCEF_TRACE;

    switch ( type )
    {
        case RPC_FIELDTYPE_BOOL: writer.WriteVarUInt64( value.AsBool() ? 1 : 0 ); return true;
        case RPC_FIELDTYPE_INT8:
        case RPC_FIELDTYPE_INT16:
        case RPC_FIELDTYPE_INT32:
        case RPC_FIELDTYPE_INT64:
        case RPC_FIELDTYPE_ENUM:
        case RPC_FIELDTYPE_DURATION: writer.WriteVarInt64( value.AsInt64() ); return true;
        case RPC_FIELDTYPE_DATE: writer.WriteVarInt64( value.AsInt32() ); return true;
        case RPC_FIELDTYPE_TIMESTAMP:
        {
            Int64 micros = 0;
            if ( !CORE::CDataTableValues::TicksFromVariant( value, CORE::DATATABLE_TIMEUNIT_MICROSECONDS, micros ) )
                micros = value.AsInt64();
            writer.WriteVarInt64( micros );
            return true;
        }
        case RPC_FIELDTYPE_UINT8:
        case RPC_FIELDTYPE_UINT16:
        case RPC_FIELDTYPE_UINT32:
        case RPC_FIELDTYPE_UINT64: writer.WriteVarUInt64( value.AsUInt64() ); return true;
        case RPC_FIELDTYPE_FLOAT32: writer.WriteFloat32( value.AsFloat32() ); return true;
        case RPC_FIELDTYPE_FLOAT64: writer.WriteFloat64( value.AsFloat64() ); return true;
        case RPC_FIELDTYPE_STRING:
        {
            CORE::CUtf8String utf8 = value.AsUtf8String();
            writer.WriteUtf8( utf8.C_String(), utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0 );
            return true;
        }
        case RPC_FIELDTYPE_BYTES:
        case RPC_FIELDTYPE_DATANODE:
        {
            writer.WriteSizedBytes( value.AsVoidPtr(), value.IsBinary() ? value.ByteSize( false ) : 0 );
            return true;
        }
        case RPC_FIELDTYPE_DECIMAL:
        {
            CORE::CDecimal decimal;
            if ( CORE::DECIMALSTATUS_OK != value.GetDecimal( decimal ) )
                return Refuse( status, field, "not a decimal" );
            if ( field.IsFixedScaleDecimal() )
            {
                if ( decimal.GetScale() != (UInt8) field.GetDecimalScale() )
                {
                    CORE::CDecimal rescaled;
                    if ( CORE::DECIMALSTATUS_OK != decimal.Rescale( (UInt8) field.GetDecimalScale(), CORE::ROUNDINGMODE_UNNECESSARY, rescaled ) )
                        return Refuse( status, field, "the decimal cannot be rescaled exactly to the declared scale" );
                    decimal = rescaled;
                }
                writer.WriteVarInt64( decimal.GetUnscaledValue() );
                return true;
            }
            UInt32 bodyStart = BeginLen( writer );
            writer.WriteDecimal( decimal );
            EndLen( writer, bodyStart );
            return true;
        }
        case RPC_FIELDTYPE_DATETIME:
        {
            UInt32 bodyStart = BeginLen( writer );
            if ( !writer.WriteDateTime( value.AsDateTime() ) )
                return Refuse( status, field, "the date time is out of range" );
            EndLen( writer, bodyStart );
            return true;
        }
        case RPC_FIELDTYPE_VARIANT:
        {
            UInt32 bodyStart = BeginLen( writer );
            if ( !writer.WriteVariant( value ) )
                return Refuse( status, field, "the variant cannot be encoded" );
            EndLen( writer, bodyStart );
            return true;
        }
        default:
            return Refuse( status, field, "unsupported type" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessageCodec::EncodeMessageField( CORE::CBinaryWireWriter& writer, const CRpcMessage& nested, CRpcStatus* status )
{GUCEF_TRACE;

    UInt32 bodyStart = BeginLen( writer );
    if ( !Encode( nested, writer, status ) )
        return false;
    EndLen( writer, bodyStart );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessageCodec::Encode( const CRpcMessage& message, CORE::CBinaryWireWriter& writer, CRpcStatus* status )
{GUCEF_TRACE;

    const CRpcTypeDescriptor* type = message.m_type;
    if ( GUCEF_NULL == type )
    {
        if ( GUCEF_NULL != status )
            status->Set( RPC_STATUS_INTERNAL, "message is not bound to a type" );
        return false;
    }

    UInt32 unknownIndex = 0;
    const UInt32 unknownCount = (UInt32) message.m_unknown.size();
    const UInt32 fieldCount = type->GetFieldCount();
    for ( UInt32 slotIndex=0; slotIndex<fieldCount; ++slotIndex )
    {
        const CRpcFieldDescriptor& field = type->GetFieldAt( slotIndex );
        const CRpcMessage::SSlot& slot = message.m_slots[ slotIndex ];

        // unknown fields kept from a decode go in number order between the known ones
        while ( unknownIndex < unknownCount && message.m_unknown[ unknownIndex ].number < field.GetNumber() )
        {
            const CORE::CDynamicBuffer& bytes = message.m_unknown[ unknownIndex ].bytes;
            writer.WriteBytes( bytes.GetConstBufferPtr(), bytes.GetDataSize() );
            ++unknownIndex;
        }

        const UInt32 number = field.GetNumber();
        const TRpcFieldType fieldType = field.GetType();
        switch ( field.GetCardinality() )
        {
            case RPC_CARDINALITY_SINGLE:
            case RPC_CARDINALITY_OPTIONAL:
            {
                if ( !slot.isSet )
                    break;
                if ( RPC_FIELDTYPE_MESSAGE == fieldType )
                {
                    WriteKey( writer, number, GUCEF_RPC_WIRETYPE_LEN );
                    if ( GUCEF_NULL != slot.message )
                    {
                        if ( !EncodeMessageField( writer, *slot.message, status ) )
                            return false;
                    }
                    else
                        writer.WriteVarUInt32( 0 );
                }
                else
                {
                    WriteKey( writer, number, field.GetWireType() );
                    if ( !EncodeValue( writer, field, fieldType, slot.value, status ) )
                        return false;
                }
                break;
            }
            case RPC_CARDINALITY_REPEATED:
            {
                if ( RPC_FIELDTYPE_MESSAGE == fieldType )
                {
                    for ( UInt32 i=0; i<slot.messages.size(); ++i )
                    {
                        WriteKey( writer, number, GUCEF_RPC_WIRETYPE_LEN );
                        if ( !EncodeMessageField( writer, *slot.messages[ i ], status ) )
                            return false;
                    }
                }
                else if ( field.IsPacked() )
                {
                    if ( slot.values.empty() )
                        break;
                    WriteKey( writer, number, GUCEF_RPC_WIRETYPE_LEN );
                    UInt32 bodyStart = BeginLen( writer );
                    for ( UInt32 i=0; i<slot.values.size(); ++i )
                        if ( !EncodeValue( writer, field, fieldType, slot.values[ i ], status ) )
                            return false;
                    EndLen( writer, bodyStart );
                }
                else
                {
                    for ( UInt32 i=0; i<slot.values.size(); ++i )
                    {
                        WriteKey( writer, number, field.GetWireType() );
                        if ( !EncodeValue( writer, field, fieldType, slot.values[ i ], status ) )
                            return false;
                    }
                }
                break;
            }
            case RPC_CARDINALITY_MAP:
            {
                const UInt32 entryCount = (UInt32) slot.mapKeys.size();
                if ( 0 == entryCount )
                    break;

                // deterministic: entries sorted by key
                GUCEF::vector< UInt32 > order;
                SortMapKeyOrder( field.GetMapKeyType(), slot.mapKeys, order );

                const UInt32 keyWireType = RpcWireTypeOfFieldType( field.GetMapKeyType(), false );
                for ( UInt32 i=0; i<entryCount; ++i )
                {
                    UInt32 entry = order[ i ];
                    WriteKey( writer, number, GUCEF_RPC_WIRETYPE_LEN );
                    UInt32 bodyStart = BeginLen( writer );
                    WriteKey( writer, 1, keyWireType );
                    if ( !EncodeValue( writer, field, field.GetMapKeyType(), slot.mapKeys[ entry ], status ) )
                        return false;
                    if ( RPC_FIELDTYPE_MESSAGE == fieldType )
                    {
                        WriteKey( writer, 2, GUCEF_RPC_WIRETYPE_LEN );
                        if ( !EncodeMessageField( writer, *slot.messages[ entry ], status ) )
                            return false;
                    }
                    else
                    {
                        WriteKey( writer, 2, field.GetWireType() );
                        if ( !EncodeValue( writer, field, fieldType, slot.values[ entry ], status ) )
                            return false;
                    }
                    EndLen( writer, bodyStart );
                }
                break;
            }
        }
    }
    while ( unknownIndex < unknownCount )
    {
        const CORE::CDynamicBuffer& bytes = message.m_unknown[ unknownIndex ].bytes;
        writer.WriteBytes( bytes.GetConstBufferPtr(), bytes.GetDataSize() );
        ++unknownIndex;
    }
    if ( writer.HasFailed() )
    {
        if ( GUCEF_NULL != status )
            status->Set( RPC_STATUS_RESOURCE_EXHAUSTED, "the encoding does not fit the buffer" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessageCodec::DecodeValue( CORE::CBinaryWireReader& reader, const CRpcFieldDescriptor& field, TRpcFieldType type,
                               bool linkBinary, CORE::CVariant& value, CRpcStatus& status )
{GUCEF_TRACE;

    switch ( type )
    {
        case RPC_FIELDTYPE_BOOL:
        {
            UInt64 v = 0;
            if ( !reader.ReadVarUInt64( v ) )
                return RefuseDecode( status, "truncated varint" );
            value = ( 0 != v );
            return true;
        }
        case RPC_FIELDTYPE_INT8:
        case RPC_FIELDTYPE_INT16:
        case RPC_FIELDTYPE_INT32:
        case RPC_FIELDTYPE_INT64:
        case RPC_FIELDTYPE_ENUM:
        case RPC_FIELDTYPE_DURATION:
        case RPC_FIELDTYPE_DATE:
        case RPC_FIELDTYPE_TIMESTAMP:
        {
            Int64 v = 0;
            if ( !reader.ReadVarInt64( v ) )
                return RefuseDecode( status, "malformed varint in field " + CORE::ToString( field.GetNumber() ) );
            Int64 minValue = 0;
            UInt64 maxValue = 0;
            bool isSigned = true;
            IntegerRange( type, minValue, maxValue, isSigned );
            if ( v < minValue || ( v > 0 && (UInt64) v > maxValue ) )
                return RefuseDecode( status, "value out of the declared width in field " + CORE::ToString( field.GetNumber() ) );
            value = TypedInteger( type, v, 0 );
            return true;
        }
        case RPC_FIELDTYPE_UINT8:
        case RPC_FIELDTYPE_UINT16:
        case RPC_FIELDTYPE_UINT32:
        case RPC_FIELDTYPE_UINT64:
        {
            UInt64 v = 0;
            if ( !reader.ReadVarUInt64( v ) )
                return RefuseDecode( status, "malformed varint in field " + CORE::ToString( field.GetNumber() ) );
            Int64 minValue = 0;
            UInt64 maxValue = 0;
            bool isSigned = false;
            IntegerRange( type, minValue, maxValue, isSigned );
            if ( v > maxValue )
                return RefuseDecode( status, "value out of the declared width in field " + CORE::ToString( field.GetNumber() ) );
            value = TypedInteger( type, 0, v );
            return true;
        }
        case RPC_FIELDTYPE_FLOAT32:
        {
            Float32 v = 0;
            if ( !reader.ReadFloat32( v ) )
                return RefuseDecode( status, "truncated fixed32" );
            value = v;
            return true;
        }
        case RPC_FIELDTYPE_FLOAT64:
        {
            Float64 v = 0;
            if ( !reader.ReadFloat64( v ) )
                return RefuseDecode( status, "truncated fixed64" );
            value = v;
            return true;
        }
        case RPC_FIELDTYPE_STRING:
        {
            const char* utf8 = GUCEF_NULL;
            UInt32 size = 0;
            if ( !reader.ReadUtf8( utf8, size ) )
                return RefuseDecode( status, "invalid UTF-8 or length in field " + CORE::ToString( field.GetNumber() ) );
            value = CORE::CUtf8String( utf8, size );
            return true;
        }
        case RPC_FIELDTYPE_BYTES:
        case RPC_FIELDTYPE_DATANODE:
        {
            const UInt8* data = GUCEF_NULL;
            UInt32 size = 0;
            if ( !reader.ReadSizedBytes( data, size ) )
                return RefuseDecode( status, "length beyond the message in field " + CORE::ToString( field.GetNumber() ) );
            if ( linkBinary && size > 0 )
                value.LinkTo( data, size, GUCEF_DATATYPE_BINARY_BLOB );
            else
                value.Set( data, size, GUCEF_DATATYPE_BINARY_BLOB );
            return true;
        }
        case RPC_FIELDTYPE_DECIMAL:
        {
            CORE::CDecimal decimal;
            if ( field.IsFixedScaleDecimal() )
            {
                Int64 unscaled = 0;
                if ( !reader.ReadVarInt64( unscaled ) )
                    return RefuseDecode( status, "malformed varint in field " + CORE::ToString( field.GetNumber() ) );
                if ( CORE::DECIMALSTATUS_OK != CORE::CDecimal::FromUnscaled( unscaled, (UInt8) field.GetDecimalScale(), decimal ) )
                    return RefuseDecode( status, "invalid decimal in field " + CORE::ToString( field.GetNumber() ) );
            }
            else
            {
                const UInt8* data = GUCEF_NULL;
                UInt32 size = 0;
                if ( !reader.ReadSizedBytes( data, size ) )
                    return RefuseDecode( status, "length beyond the message in field " + CORE::ToString( field.GetNumber() ) );
                CORE::CBinaryWireReader body( data, size );
                if ( !body.ReadDecimal( decimal ) || !body.IsAtEnd() )
                    return RefuseDecode( status, "invalid decimal in field " + CORE::ToString( field.GetNumber() ) );
            }
            value = decimal;
            return true;
        }
        case RPC_FIELDTYPE_DATETIME:
        {
            const UInt8* data = GUCEF_NULL;
            UInt32 size = 0;
            if ( !reader.ReadSizedBytes( data, size ) )
                return RefuseDecode( status, "length beyond the message in field " + CORE::ToString( field.GetNumber() ) );
            CORE::CBinaryWireReader body( data, size );
            CORE::CDateTime dateTime;
            if ( !body.ReadDateTime( dateTime ) || !body.IsAtEnd() )
                return RefuseDecode( status, "invalid date time in field " + CORE::ToString( field.GetNumber() ) );
            value = DateTimeToVariant( dateTime );
            return true;
        }
        case RPC_FIELDTYPE_VARIANT:
        {
            const UInt8* data = GUCEF_NULL;
            UInt32 size = 0;
            if ( !reader.ReadSizedBytes( data, size ) )
                return RefuseDecode( status, "length beyond the message in field " + CORE::ToString( field.GetNumber() ) );
            if ( 0 == size )
            {
                value.Clear();
                return true;
            }
            CORE::CBinaryWireReader body( data, size );
            if ( !body.ReadVariant( value, linkBinary ) || !body.IsAtEnd() )
                return RefuseDecode( status, "invalid variant in field " + CORE::ToString( field.GetNumber() ) );
            return true;
        }
        default:
            return RefuseDecode( status, "unsupported type in field " + CORE::ToString( field.GetNumber() ) );
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessageCodec::DecodeField( CRpcMessage& message, CORE::CBinaryWireReader& reader, const CRpcFieldDescriptor& field,
                               UInt32 wireType, UInt32 depth, bool linkBinary, CRpcStatus& status )
{GUCEF_TRACE;

    CRpcMessage::SSlot& slot = message.m_slots[ field.GetSlotIndex() ];
    const TRpcFieldType fieldType = field.GetType();
    const UInt32 expected = field.GetWireType();

    if ( field.IsMap() )
    {
        if ( GUCEF_RPC_WIRETYPE_LEN != wireType )
            return RefuseDecode( status, "map field " + CORE::ToString( field.GetNumber() ) + " with wire type " + CORE::ToString( wireType ) );
        const UInt8* data = GUCEF_NULL;
        UInt32 size = 0;
        if ( !reader.ReadSizedBytes( data, size ) )
            return RefuseDecode( status, "length beyond the message in field " + CORE::ToString( field.GetNumber() ) );

        CORE::CBinaryWireReader entry( data, size );
        CORE::CVariant key = ZeroValue( field, field.GetMapKeyType() );
        CORE::CVariant value = ZeroValue( field, fieldType );
        CRpcMessage* nested = GUCEF_NULL;
        const UInt32 keyWireType = RpcWireTypeOfFieldType( field.GetMapKeyType(), false );
        while ( !entry.IsAtEnd() )
        {
            UInt64 entryKey = 0;
            if ( !entry.ReadVarUInt64( entryKey ) || entryKey > 0xFFFFFFFFULL )
            {
                GUCEF_DELETE nested;
                return RefuseDecode( status, "malformed map entry in field " + CORE::ToString( field.GetNumber() ) );
            }
            UInt32 entryNumber = (UInt32) ( entryKey >> 3 );
            UInt32 entryWireType = (UInt32) ( entryKey & 7 );
            bool ok = true;
            if ( 1 == entryNumber && entryWireType == keyWireType )
                ok = DecodeValue( entry, field, field.GetMapKeyType(), false, key, status );
            else if ( 2 == entryNumber && RPC_FIELDTYPE_MESSAGE == fieldType && GUCEF_RPC_WIRETYPE_LEN == entryWireType )
            {
                const UInt8* nestedData = GUCEF_NULL;
                UInt32 nestedSize = 0;
                ok = entry.ReadSizedBytes( nestedData, nestedSize );
                if ( ok )
                {
                    GUCEF_DELETE nested;
                    nested = message.NewNested( field );
                    ok = Decode( *nested, nestedData, nestedSize, depth + 1, linkBinary, status );
                }
                else
                    RefuseDecode( status, "length beyond the message in field " + CORE::ToString( field.GetNumber() ) );
            }
            else if ( 2 == entryNumber && RPC_FIELDTYPE_MESSAGE != fieldType && entryWireType == expected )
                ok = DecodeValue( entry, field, fieldType, linkBinary, value, status );
            else if ( 1 == entryNumber || 2 == entryNumber )
                ok = RefuseDecode( status, "wire type mismatch in a map entry of field " + CORE::ToString( field.GetNumber() ) );
            else
            {
                ok = SkipValue( entry, entryWireType );
                if ( !ok )
                    RefuseDecode( status, "malformed map entry in field " + CORE::ToString( field.GetNumber() ) );
            }
            if ( !ok )
            {
                GUCEF_DELETE nested;
                return false;
            }
        }

        // appended: a repeated key replaces the earlier entry once the message is decoded (RemoveRepeatedMapKeys())
        if ( RPC_FIELDTYPE_MESSAGE == fieldType )
        {
            if ( GUCEF_NULL == nested )
                nested = message.NewNested( field );
            slot.mapKeys.push_back( key );
            slot.messages.push_back( nested );
        }
        else
        {
            slot.mapKeys.push_back( key );
            slot.values.push_back( value );
        }
        return true;
    }

    if ( RPC_FIELDTYPE_MESSAGE == fieldType )
    {
        if ( GUCEF_RPC_WIRETYPE_LEN != wireType )
            return RefuseDecode( status, "message field " + CORE::ToString( field.GetNumber() ) + " with wire type " + CORE::ToString( wireType ) );
        const UInt8* data = GUCEF_NULL;
        UInt32 size = 0;
        if ( !reader.ReadSizedBytes( data, size ) )
            return RefuseDecode( status, "length beyond the message in field " + CORE::ToString( field.GetNumber() ) );
        CRpcMessage* nested = message.NewNested( field );
        if ( !Decode( *nested, data, size, depth + 1, linkBinary, status ) )
        {
            GUCEF_DELETE nested;
            return false;
        }
        if ( field.IsRepeated() )
            slot.messages.push_back( nested );
        else
        {
            // last value wins
            GUCEF_DELETE slot.message;
            slot.message = nested;
            slot.isSet = true;
        }
        return true;
    }

    if ( field.IsRepeated() && GUCEF_RPC_WIRETYPE_LEN == wireType && GUCEF_RPC_WIRETYPE_LEN != expected )
    {
        // packed
        const UInt8* data = GUCEF_NULL;
        UInt32 size = 0;
        if ( !reader.ReadSizedBytes( data, size ) )
            return RefuseDecode( status, "length beyond the message in field " + CORE::ToString( field.GetNumber() ) );
        CORE::CBinaryWireReader packed( data, size );
        while ( !packed.IsAtEnd() )
        {
            slot.values.push_back( CORE::CVariant() );
            if ( !DecodeValue( packed, field, fieldType, linkBinary, slot.values.back(), status ) )
                return false;
        }
        return true;
    }

    if ( wireType != expected )
        return RefuseDecode( status, "field " + CORE::ToString( field.GetNumber() ) + " (" + field.GetName() + ") arrived with wire type " +
                                     CORE::ToString( wireType ) + ", declared " + CORE::ToString( expected ) );

    // decoded in place: a value that links to the buffer keeps linking (a copy would copy the bytes)
    if ( field.IsRepeated() )
    {
        slot.values.push_back( CORE::CVariant() );
        return DecodeValue( reader, field, fieldType, linkBinary, slot.values.back(), status );
    }
    if ( !DecodeValue( reader, field, fieldType, linkBinary, slot.value, status ) )
        return false;
    slot.isSet = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMessageCodec::Decode( CRpcMessage& message, const UInt8* data, UInt32 size, UInt32 depth, bool linkBinary, CRpcStatus& status )
{GUCEF_TRACE;

    const CRpcTypeDescriptor* type = message.m_type;
    if ( GUCEF_NULL == type )
        return RefuseDecode( status, "message is not bound to a type" );
    if ( depth > GUCEF_RPC_MAX_MESSAGE_DEPTH )
        return RefuseDecode( status, "messages nested deeper than " + CORE::ToString( (UInt32) GUCEF_RPC_MAX_MESSAGE_DEPTH ) );

    CORE::CBinaryWireReader reader( data, size );
    bool unknownInOrder = true;
    bool hasMapEntries = false;
    while ( !reader.IsAtEnd() )
    {
        UInt32 fieldStart = reader.GetPosition();
        UInt64 key = 0;
        if ( !reader.ReadVarUInt64( key ) || key > 0xFFFFFFFFULL )
            return RefuseDecode( status, "malformed field key" );
        UInt32 number = (UInt32) ( key >> 3 );
        UInt32 wireType = (UInt32) ( key & 7 );
        if ( 0 == number )
            return RefuseDecode( status, "field number 0" );
        if ( GUCEF_RPC_WIRETYPE_VARINT != wireType && GUCEF_RPC_WIRETYPE_FIXED64 != wireType &&
             GUCEF_RPC_WIRETYPE_LEN != wireType && GUCEF_RPC_WIRETYPE_FIXED32 != wireType )
            return RefuseDecode( status, "invalid wire type " + CORE::ToString( wireType ) + " for field " + CORE::ToString( number ) );

        const CRpcFieldDescriptor* field = type->FindFieldByNumber( number );
        if ( GUCEF_NULL == field )
        {
            if ( !SkipValue( reader, wireType ) )
                return RefuseDecode( status, "malformed value of unknown field " + CORE::ToString( number ) );
            if ( message.m_preserveUnknown )
            {
                // appended; sorted by number once at the end so a re-encode interleaves them correctly
                if ( !message.m_unknown.empty() && message.m_unknown.back().number > number )
                    unknownInOrder = false;
                message.m_unknown.push_back( CRpcMessage::SUnknownField() );
                CRpcMessage::SUnknownField& unknown = message.m_unknown.back();
                unknown.number = number;
                unknown.bytes.CopyFrom( reader.GetPosition() - fieldStart, data + fieldStart );
            }
            continue;
        }
        if ( field->IsMap() )
            hasMapEntries = true;
        if ( !DecodeField( message, reader, *field, wireType, depth, linkBinary, status ) )
            return false;
    }
    if ( !unknownInOrder )
    {
        // by number, those of one number in arrival order (the pair's index breaks ties)
        const UInt32 unknownCount = (UInt32) message.m_unknown.size();
        GUCEF::vector< std::pair< UInt32, UInt32 > > order( unknownCount );
        for ( UInt32 i=0; i<unknownCount; ++i )
            order[ i ] = std::pair< UInt32, UInt32 >( message.m_unknown[ i ].number, i );
        std::sort( order.begin(), order.end() );
        CRpcMessage::TUnknownFieldVector sorted( unknownCount );
        for ( UInt32 i=0; i<unknownCount; ++i )
        {
            CRpcMessage::SUnknownField& source = message.m_unknown[ order[ i ].second ];
            sorted[ i ].number = source.number;
            sorted[ i ].bytes.Swap( source.bytes );
        }
        message.m_unknown.swap( sorted );
    }
    if ( hasMapEntries )
    {
        const UInt32 fieldCount = type->GetFieldCount();
        for ( UInt32 slotIndex=0; slotIndex<fieldCount; ++slotIndex )
        {
            const CRpcFieldDescriptor& field = type->GetFieldAt( slotIndex );
            if ( !field.IsMap() )
                continue;
            CRpcMessage::SSlot& slot = message.m_slots[ slotIndex ];
            RemoveRepeatedMapKeys( field.GetMapKeyType(), slot.mapKeys, slot.values, slot.messages, RPC_FIELDTYPE_MESSAGE == field.GetType() );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcMessageCodec::SortMapKeyOrder( TRpcFieldType keyType, const GUCEF::vector< CORE::CVariant >& keys, GUCEF::vector< UInt32 >& order )
{GUCEF_TRACE;

    const UInt32 count = (UInt32) keys.size();
    order.resize( count );
    for ( UInt32 i=0; i<count; ++i )
        order[ i ] = i;
    std::stable_sort( order.begin(), order.end(), CMapKeyIndexOrder( keyType, keys ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
