/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#include <math.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRN_CDRNVariantCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static Float64
PowerOfTen( Int32 precision )
{GUCEF_TRACE;
    static const Float64 powers[] = { 1.0, 10.0, 100.0, 1000.0, 10000.0, 100000.0, 1000000.0, 10000000.0, 100000000.0, 1000000000.0 };
    if ( precision < 0 )
        return 1.0;
    if ( precision < 10 )
        return powers[ precision ];
    return pow( 10.0, (Float64) precision );
}

/*-------------------------------------------------------------------------*/

static Int64
RoundToInt64( Float64 value )
{GUCEF_TRACE;
    return (Int64) ( value < 0 ? value - 0.5 : value + 0.5 );
}

/*-------------------------------------------------------------------------*/

/** The tag of the native type for integer and float type ids, 0 for other types */
static UInt8
NativeNumericTag( UInt8 typeId )
{GUCEF_TRACE;
    switch ( typeId )
    {
        case GUCEF_DATATYPE_UINT8: return GUCEF_DATATYPE_UINT8;
        case GUCEF_DATATYPE_INT8: return GUCEF_DATATYPE_INT8;
        case GUCEF_DATATYPE_LE_UINT16: case GUCEF_DATATYPE_BE_UINT16: return GUCEF_DATATYPE_UINT16;
        case GUCEF_DATATYPE_LE_INT16: case GUCEF_DATATYPE_BE_INT16: return GUCEF_DATATYPE_INT16;
        case GUCEF_DATATYPE_LE_UINT32: case GUCEF_DATATYPE_BE_UINT32: return GUCEF_DATATYPE_UINT32;
        case GUCEF_DATATYPE_LE_INT32: case GUCEF_DATATYPE_BE_INT32: return GUCEF_DATATYPE_INT32;
        case GUCEF_DATATYPE_LE_UINT64: case GUCEF_DATATYPE_BE_UINT64: return GUCEF_DATATYPE_UINT64;
        case GUCEF_DATATYPE_LE_INT64: case GUCEF_DATATYPE_BE_INT64: return GUCEF_DATATYPE_INT64;
        case GUCEF_DATATYPE_LE_FLOAT32: case GUCEF_DATATYPE_BE_FLOAT32: return GUCEF_DATATYPE_FLOAT32;
        case GUCEF_DATATYPE_LE_FLOAT64: case GUCEF_DATATYPE_BE_FLOAT64: return GUCEF_DATATYPE_FLOAT64;
        default: return 0;
    }
}

/*-------------------------------------------------------------------------*/

static UInt64
MaskUnsigned( UInt64 value, UInt32 bits )
{GUCEF_TRACE;
    if ( 0 == bits || bits >= 64 )
        return value;
    return value & ( ( (UInt64) 1 << bits ) - 1 );
}

/*-------------------------------------------------------------------------*/

static Int64
ClampSigned( Int64 value, UInt32 bits )
{GUCEF_TRACE;
    if ( 0 == bits || bits >= 64 )
        return value;
    Int64 maxValue = ( (Int64) 1 << ( bits - 1 ) ) - 1;
    Int64 minValue = -maxValue - 1;
    return value > maxValue ? maxValue : ( value < minValue ? minValue : value );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNQuantisation::CDRNQuantisation( void )
    : floatPrecision( -1 )
    , intBits( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNQuantisation::CDRNQuantisation( Int32 precision, UInt32 bits )
    : floatPrecision( precision )
    , intBits( bits )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDRNQuantisation::IsFull( void ) const
{GUCEF_TRACE;

    return floatPrecision < 0 && 0 == intBits;
}

/*-------------------------------------------------------------------------*/

void
CDRNVariantCodec::WriteRemoved( CDRNWireWriter& writer )
{GUCEF_TRACE;

    writer.WriteUInt8( TagRemoved );
}

/*-------------------------------------------------------------------------*/

void
CDRNVariantCodec::Write( CDRNWireWriter& writer, const CORE::CVariant& value, const CDRNQuantisation* quantisation )
{GUCEF_TRACE;

    UInt8 typeId = value.GetTypeId();
    UInt8 numericTag = NativeNumericTag( typeId );
    UInt32 intBits = GUCEF_NULL != quantisation ? quantisation->intBits : 0;
    Int32 floatPrecision = GUCEF_NULL != quantisation ? quantisation->floatPrecision : -1;

    switch ( numericTag )
    {
        case GUCEF_DATATYPE_UINT8:
        case GUCEF_DATATYPE_UINT16:
        case GUCEF_DATATYPE_UINT32:
        case GUCEF_DATATYPE_UINT64:
        {
            writer.WriteUInt8( numericTag );
            writer.WriteVarUInt64( MaskUnsigned( value.AsUInt64(), intBits ) );
            return;
        }
        case GUCEF_DATATYPE_INT8:
        case GUCEF_DATATYPE_INT16:
        case GUCEF_DATATYPE_INT32:
        case GUCEF_DATATYPE_INT64:
        {
            writer.WriteUInt8( numericTag );
            writer.WriteVarInt64( ClampSigned( value.AsInt64(), intBits ) );
            return;
        }
        case GUCEF_DATATYPE_FLOAT32:
        {
            if ( floatPrecision >= 0 )
            {
                writer.WriteUInt8( TagQuantisedFloat32 );
                writer.WriteUInt8( (UInt8) floatPrecision );
                writer.WriteVarInt64( RoundToInt64( value.AsFloat32() * PowerOfTen( floatPrecision ) ) );
            }
            else
            {
                writer.WriteUInt8( numericTag );
                writer.WriteFloat32( value.AsFloat32() );
            }
            return;
        }
        case GUCEF_DATATYPE_FLOAT64:
        {
            if ( floatPrecision >= 0 )
            {
                writer.WriteUInt8( TagQuantisedFloat64 );
                writer.WriteUInt8( (UInt8) floatPrecision );
                writer.WriteVarInt64( RoundToInt64( value.AsFloat64() * PowerOfTen( floatPrecision ) ) );
            }
            else
            {
                writer.WriteUInt8( numericTag );
                writer.WriteFloat64( value.AsFloat64() );
            }
            return;
        }
        default:
            break;
    }

    switch ( typeId )
    {
        case GUCEF_DATATYPE_UNKNOWN:
        case GUCEF_DATATYPE_NIL:
        case GUCEF_DATATYPE_NULL:
        case GUCEF_DATATYPE_VOID:
        {
            writer.WriteUInt8( typeId );
            return;
        }
        case GUCEF_DATATYPE_BOOLEAN_INT32:
        {
            writer.WriteUInt8( typeId );
            writer.WriteUInt8( value.AsBool() ? 1 : 0 );
            return;
        }
        case GUCEF_DATATYPE_ASCII_STRING:
        case GUCEF_DATATYPE_UTF8_STRING:
        {
            // the bytes of the string as they are, without conversion
            writer.WriteUInt8( typeId );
            UInt32 size = value.ByteSize( false );
            writer.WriteSizedBytes( value.AsVoidPtr(), size );
            return;
        }
        case GUCEF_DATATYPE_BINARY_BLOB:
        case GUCEF_DATATYPE_BINARY_BSOB:
        {
            writer.WriteUInt8( GUCEF_DATATYPE_BINARY_BLOB );
            writer.WriteSizedBytes( value.AsVoidPtr(), value.ByteSize( false ) );
            return;
        }
        default:
        {
            if ( value.IsString() )
            {
                CORE::CUtf8String utf8 = value.AsUtf8String();
                writer.WriteUInt8( GUCEF_DATATYPE_UTF8_STRING );
                writer.WriteSizedBytes( utf8.C_String(), utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0 );
                return;
            }
            // anything else travels in its string form
            writer.WriteUInt8( TagOther );
            writer.WriteUInt8( typeId );
            writer.WriteString( value.AsString() );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNVariantCodec::Read( CDRNWireReader& reader, CORE::CVariant& value, bool* isRemoved )
{GUCEF_TRACE;

    if ( GUCEF_NULL != isRemoved )
        *isRemoved = false;

    UInt8 tag = 0;
    if ( !reader.ReadUInt8( tag ) )
        return false;

    switch ( tag )
    {
        case GUCEF_DATATYPE_UINT8:
        case GUCEF_DATATYPE_UINT16:
        case GUCEF_DATATYPE_UINT32:
        case GUCEF_DATATYPE_UINT64:
        {
            UInt64 v = 0;
            if ( !reader.ReadVarUInt64( v ) )
                return false;
            switch ( tag )
            {
                case GUCEF_DATATYPE_UINT8: value = (UInt8) v; break;
                case GUCEF_DATATYPE_UINT16: value = (UInt16) v; break;
                case GUCEF_DATATYPE_UINT32: value = (UInt32) v; break;
                default: value = v; break;
            }
            return true;
        }
        case GUCEF_DATATYPE_INT8:
        case GUCEF_DATATYPE_INT16:
        case GUCEF_DATATYPE_INT32:
        case GUCEF_DATATYPE_INT64:
        {
            Int64 v = 0;
            if ( !reader.ReadVarInt64( v ) )
                return false;
            switch ( tag )
            {
                case GUCEF_DATATYPE_INT8: value = (Int8) v; break;
                case GUCEF_DATATYPE_INT16: value = (Int16) v; break;
                case GUCEF_DATATYPE_INT32: value = (Int32) v; break;
                default: value = v; break;
            }
            return true;
        }
        case GUCEF_DATATYPE_FLOAT32:
        {
            Float32 v = 0;
            if ( !reader.ReadFloat32( v ) )
                return false;
            value = v;
            return true;
        }
        case GUCEF_DATATYPE_FLOAT64:
        {
            Float64 v = 0;
            if ( !reader.ReadFloat64( v ) )
                return false;
            value = v;
            return true;
        }
        case TagQuantisedFloat32:
        case TagQuantisedFloat64:
        {
            UInt8 precision = 0;
            Int64 scaled = 0;
            if ( !reader.ReadUInt8( precision ) || !reader.ReadVarInt64( scaled ) )
                return false;
            Float64 v = (Float64) scaled / PowerOfTen( precision );
            if ( TagQuantisedFloat32 == tag )
                value = (Float32) v;
            else
                value = v;
            return true;
        }
        case GUCEF_DATATYPE_UNKNOWN:
        case GUCEF_DATATYPE_NIL:
        case GUCEF_DATATYPE_NULL:
        case GUCEF_DATATYPE_VOID:
        {
            value.Clear();
            if ( GUCEF_DATATYPE_VOID == tag )
                value.SetVoidType();
            return true;
        }
        case GUCEF_DATATYPE_BOOLEAN_INT32:
        {
            UInt8 b = 0;
            if ( !reader.ReadUInt8( b ) )
                return false;
            value = ( 0 != b );
            return true;
        }
        case GUCEF_DATATYPE_ASCII_STRING:
        case GUCEF_DATATYPE_UTF8_STRING:
        {
            const UInt8* data = GUCEF_NULL;
            UInt32 size = 0;
            if ( !reader.ReadSizedBytes( data, size ) )
                return false;
            if ( GUCEF_DATATYPE_ASCII_STRING == tag )
                value = CORE::CAsciiString( reinterpret_cast< const char* >( data ), size );
            else
                value = CORE::CUtf8String( reinterpret_cast< const char* >( data ), size );
            return true;
        }
        case GUCEF_DATATYPE_BINARY_BLOB:
        {
            const UInt8* data = GUCEF_NULL;
            UInt32 size = 0;
            if ( !reader.ReadSizedBytes( data, size ) )
                return false;
            value.Set( data, size, GUCEF_DATATYPE_BINARY_BLOB );
            return true;
        }
        case TagOther:
        {
            UInt8 typeId = 0;
            CORE::CString text;
            if ( !reader.ReadUInt8( typeId ) || !reader.ReadString( text ) )
                return false;
            value.SetFromString( typeId, text );
            return true;
        }
        case TagRemoved:
        {
            value.Clear();
            if ( GUCEF_NULL != isRemoved )
                *isRemoved = true;
            return true;
        }
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNVariantCodec::Quantise( CORE::CVariant& value, const CDRNQuantisation& quantisation )
{GUCEF_TRACE;

    UInt8 tag = NativeNumericTag( value.GetTypeId() );
    if ( quantisation.floatPrecision >= 0 && ( GUCEF_DATATYPE_FLOAT32 == tag || GUCEF_DATATYPE_FLOAT64 == tag ) )
    {
        Float64 factor = PowerOfTen( quantisation.floatPrecision );
        Float64 v = (Float64) RoundToInt64( value.AsFloat64() * factor ) / factor;
        if ( GUCEF_DATATYPE_FLOAT32 == tag )
            value = (Float32) v;
        else
            value = v;
    }
    else
    if ( 0 != quantisation.intBits )
    {
        switch ( tag )
        {
            case GUCEF_DATATYPE_UINT8: value = (UInt8) MaskUnsigned( value.AsUInt64(), quantisation.intBits ); break;
            case GUCEF_DATATYPE_UINT16: value = (UInt16) MaskUnsigned( value.AsUInt64(), quantisation.intBits ); break;
            case GUCEF_DATATYPE_UINT32: value = (UInt32) MaskUnsigned( value.AsUInt64(), quantisation.intBits ); break;
            case GUCEF_DATATYPE_UINT64: value = MaskUnsigned( value.AsUInt64(), quantisation.intBits ); break;
            case GUCEF_DATATYPE_INT8: value = (Int8) ClampSigned( value.AsInt64(), quantisation.intBits ); break;
            case GUCEF_DATATYPE_INT16: value = (Int16) ClampSigned( value.AsInt64(), quantisation.intBits ); break;
            case GUCEF_DATATYPE_INT32: value = (Int32) ClampSigned( value.AsInt64(), quantisation.intBits ); break;
            case GUCEF_DATATYPE_INT64: value = ClampSigned( value.AsInt64(), quantisation.intBits ); break;
            default: break;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNVariantCodec::IsSame( const CORE::CVariant& a, const CORE::CVariant& b )
{GUCEF_TRACE;

    if ( a.GetTypeId() != b.GetTypeId() )
        return false;
    UInt8 tag = NativeNumericTag( a.GetTypeId() );
    switch ( tag )
    {
        case GUCEF_DATATYPE_FLOAT32: return a.AsFloat32() == b.AsFloat32();
        case GUCEF_DATATYPE_FLOAT64: return a.AsFloat64() == b.AsFloat64();
        case GUCEF_DATATYPE_INT8: case GUCEF_DATATYPE_INT16: case GUCEF_DATATYPE_INT32: case GUCEF_DATATYPE_INT64:
            return a.AsInt64() == b.AsInt64();
        case GUCEF_DATATYPE_UINT8: case GUCEF_DATATYPE_UINT16: case GUCEF_DATATYPE_UINT32: case GUCEF_DATATYPE_UINT64:
            return a.AsUInt64() == b.AsUInt64();
        default:
            return a == b;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
