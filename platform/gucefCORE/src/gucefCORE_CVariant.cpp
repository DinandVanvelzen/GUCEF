/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include <assert.h>
#include <cmath>

#ifndef GUCEF_ENDIAN_H
#include "gucef_endian.h"
#define GUCEF_ENDIAN_H
#endif /* GUCEF_ENDIAN_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef SHEREDOM_UTF8_H_INCLUDED
#include "gucefCORE_utf8.h"
#define SHEREDOM_UTF8_H_INCLUDED
#endif /* SHEREDOM_UTF8_H_INCLUDED ? */

#include "gucefCORE_CVariant.h"

#ifndef GUCEF_CORE_CUTF16STRING_H
#include "gucefCORE_CUtf16String.h"
#define GUCEF_CORE_CUTF16STRING_H
#endif /* GUCEF_CORE_CUTF16STRING_H ? */

#ifndef GUCEF_CORE_CUTF32STRING_H
#include "gucefCORE_CUtf32String.h"
#define GUCEF_CORE_CUTF32STRING_H
#endif /* GUCEF_CORE_CUTF32STRING_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_ESSENTIALS_H
#endif /* GUCEF_CORE_ESSENTIALS_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CVariant                  CVariant::Empty;
const CVariant::VariantVector   CVariant::EmptyVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The variant type to use for std::wstring content
 *  wchar_t is 2 bytes (UTF-16) on Windows but 4 bytes (UTF-32) on Linux and most other platforms
 */
static inline UInt8
WStringVariantType( void )
{
    return 2 == sizeof( wchar_t ) ? (UInt8) GUCEF_DATATYPE_UTF16_STRING : (UInt8) GUCEF_DATATYPE_UTF32_STRING;
}

/*-------------------------------------------------------------------------*/

/** The decimal type id that is not the native byte order */
#if ( GUCEF_PLATFORM_BYTEORDER_ENDIAN == GUCEF_BYTEORDER_LITTLE_ENDIAN )
  #define GUCEF_DATATYPE_NON_NATIVE_INT64_UINT8_DECIMAL     GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL
#else
  #define GUCEF_DATATYPE_NON_NATIVE_INT64_UINT8_DECIMAL     GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL
#endif

/*-------------------------------------------------------------------------*/

static inline bool
IsDecimalTypeId( UInt8 typeId )
{
    return GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL == typeId || GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL == typeId;
}

/*-------------------------------------------------------------------------*/

/** The date and microsecond timestamp type ids that are not the native byte order */
#if ( GUCEF_PLATFORM_BYTEORDER_ENDIAN == GUCEF_BYTEORDER_LITTLE_ENDIAN )
  #define GUCEF_DATATYPE_NON_NATIVE_DATE_IN_DAYS_SINCE_UNIX_EPOCH           GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH
  #define GUCEF_DATATYPE_NON_NATIVE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH    GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH
  #define GUCEF_DATATYPE_NON_NATIVE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH     GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH
  #define GUCEF_DATATYPE_NON_NATIVE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH      GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH
  #define GUCEF_DATATYPE_NON_NATIVE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH        GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH
#else
  #define GUCEF_DATATYPE_NON_NATIVE_DATE_IN_DAYS_SINCE_UNIX_EPOCH           GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH
  #define GUCEF_DATATYPE_NON_NATIVE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH    GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH
  #define GUCEF_DATATYPE_NON_NATIVE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH     GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH
  #define GUCEF_DATATYPE_NON_NATIVE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH      GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH
  #define GUCEF_DATATYPE_NON_NATIVE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH        GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH
#endif

/*-------------------------------------------------------------------------*/

static inline bool
IsDateTypeId( UInt8 typeId )
{
    return GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == typeId || GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == typeId;
}

/*-------------------------------------------------------------------------*/

/** The day count of a date variant payload in native byte order */
static inline Int32
DaysFromVariantPayload( const TVariantData& data )
{
    if ( GUCEF_DATATYPE_NON_NATIVE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == data.containedType )
        return SwapEndianInt32( data.union_data.int32_data );
    return data.union_data.int32_data;
}

/*-------------------------------------------------------------------------*/

/** The signed tick count of a nanosecond, second or millisecond timestamp variant payload in native byte order */
static inline Int64
SignedTicksFromVariantPayload( const TVariantData& data )
{
    if ( GUCEF_DATATYPE_NON_NATIVE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH == data.containedType ||
         GUCEF_DATATYPE_NON_NATIVE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH == data.containedType  ||
         GUCEF_DATATYPE_NON_NATIVE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH == data.containedType )
        return SwapEndianInt64( data.union_data.int64_data );
    return data.union_data.int64_data;
}

/*-------------------------------------------------------------------------*/

/** The tick count of a microsecond timestamp variant payload in native byte order */
static inline Int64
MicrosFromVariantPayload( const TVariantData& data )
{
    if ( GUCEF_DATATYPE_NON_NATIVE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH == data.containedType )
        return SwapEndianInt64( data.union_data.int64_data );
    return data.union_data.int64_data;
}

/*-------------------------------------------------------------------------*/

/**
 *  Validated decimal from the variant payload, swapping the unscaled value when it is not in native byte order
 *  A corrupt payload (scale byte above the maximum or INT64_MIN) gives DECIMALSTATUS_INVALID_OPERAND
 */
static TDecimalStatus
DecimalFromVariantPayload( const TVariantData& data, CDecimal& result )
{
    Int64 unscaled = data.union_data.decimal_data.unscaled;
    if ( GUCEF_DATATYPE_NON_NATIVE_INT64_UINT8_DECIMAL == data.containedType )
        unscaled = SwapEndianInt64( unscaled );
    if ( DecimalStatusIsError( CDecimal::FromUnscaled( unscaled, data.union_data.decimal_data.scale, result ) ) )
        return DECIMALSTATUS_INVALID_OPERAND;
    return DECIMALSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

/** Integer value of a decimal variant truncated toward zero, the same as the float to integer casts */
static bool
DecimalVariantAsTruncatedInt64( const CVariant& var, Int64& result )
{
    CDecimal value;
    if ( DecimalStatusIsError( var.GetDecimal( value ) ) )
        return false;
    return !DecimalStatusIsError( value.ToInt64( ROUNDINGMODE_TOWARD_ZERO, result ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  Exact decimal for numerator / denominator: only when the reduced denominator has no prime factors other than 2 and 5
 *  ROUNDING_REQUIRED for any other denominator, INVALID_SCALE when more than MaxScale digits would be needed
 */
static TDecimalStatus
DecimalFromFraction( bool negative, UInt64 numerator, UInt64 denominator, CDecimal& result )
{
    if ( 0 == denominator )
        return DECIMALSTATUS_DIVISION_BY_ZERO;

    // reduce by the greatest common divisor
    UInt64 a = numerator;
    UInt64 b = denominator;
    while ( 0 != b )
    {
        UInt64 t = a % b;
        a = b;
        b = t;
    }
    if ( 0 != a )
    {
        numerator /= a;
        denominator /= a;
    }

    UInt32 twos = 0;
    UInt32 fives = 0;
    UInt64 rest = denominator;
    while ( 0 == rest % 2 ) { rest /= 2; ++twos; }
    while ( 0 == rest % 5 ) { rest /= 5; ++fives; }
    if ( 1 != rest )
        return DECIMALSTATUS_ROUNDING_REQUIRED;

    UInt32 scale = twos > fives ? twos : fives;
    if ( scale > CDecimal::MaxScale )
        return DECIMALSTATUS_INVALID_SCALE;

    // numerator / denominator = numerator * ( 10^scale / denominator ) / 10^scale
    UInt64 multiplier = 1;
    for ( UInt32 i=twos; i<scale; ++i )
        multiplier *= 2;
    for ( UInt32 i=fives; i<scale; ++i )
        multiplier *= 5;

    if ( numerator > (UInt64) GUCEF_INT64MAX )
        return DECIMALSTATUS_OVERFLOW;
    CDecimal integral;
    TDecimalStatus status = CDecimal::FromUnscaled( negative ? -(Int64) numerator : (Int64) numerator, 0, integral );
    if ( DecimalStatusIsError( status ) )
        return status;
    CDecimal scaled;
    if ( multiplier > (UInt64) GUCEF_INT64MAX )
        return DECIMALSTATUS_OVERFLOW;
    status = integral.Multiply( (Int64) multiplier, scaled );
    if ( DecimalStatusIsError( status ) )
        return status;
    return scaled.MovePointLeft( (UInt8) scale, result );
}

/*-------------------------------------------------------------------------*/

/**
 *  Exact comparison of a decimal variant with another variant, as far as one is defined
 *  Returns 0 when not applicable, 1 when 'cmp' holds the exact comparison, 2 when the comparison has to use floats
 */
static Int32
CompareWithDecimal( const CVariant& a, const CVariant& b, Int32& cmp )
{
    const CVariant* decimalSide = a.IsDecimal() ? &a : &b;
    const CVariant* otherSide = a.IsDecimal() ? &b : &a;
    Int32 orientation = a.IsDecimal() ? 1 : -1;

    CDecimal value;
    if ( DecimalStatusIsError( decimalSide->GetDecimal( value ) ) )
        return 0;

    CDecimal other;
    if ( otherSide->IsDecimal() )
    {
        if ( DecimalStatusIsError( otherSide->GetDecimal( other ) ) )
            return 0;
    }
    else
    if ( otherSide->IsInteger() )
    {
        if ( otherSide->IsUnsignedInteger() )
        {
            UInt64 unsignedValue = otherSide->AsUInt64();
            if ( unsignedValue > (UInt64) GUCEF_INT64MAX )
            {
                // above every decimal
                cmp = -orientation;
                return 1;
            }
            CDecimal::FromInt64( (Int64) unsignedValue, 0, other );
        }
        else
        {
            Int64 signedValue = otherSide->AsInt64();
            if ( GUCEF_INT64MIN == signedValue )
            {
                // below every decimal
                cmp = orientation;
                return 1;
            }
            CDecimal::FromInt64( signedValue, 0, other );
        }
    }
    else
    if ( otherSide->IsString() )
    {
        if ( DecimalStatusIsError( CDecimal::FromString( otherSide->AsString(), other ) ) )
            return 0;
    }
    else
    if ( otherSide->IsFloat() )
    {
        return 2;
    }
    else
    {
        return 0;
    }

    cmp = orientation * value.Compare( other );
    return 1;
}

/*-------------------------------------------------------------------------*/

/**
 *  How the content of a text or binary variant is read for a comparison
 *  'unitSize' is 0 for every other type
 */
struct SComparisonContent
{
    const UInt8* data;
    UInt32 size;
    UInt32 unitSize;
    bool isText;
    bool isBigEndian;
};

/*-------------------------------------------------------------------------*/

static void
GetComparisonContent( const CVariant& var, SComparisonContent& content )
{
    content.data = GUCEF_NULL;
    content.size = 0;
    content.unitSize = 0;
    content.isText = true;
    content.isBigEndian = false;

    switch ( var.GetTypeId() )
    {
        case GUCEF_DATATYPE_ASCII_STRING:
        case GUCEF_DATATYPE_UTF8_STRING:
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING:
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING:
        case GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING:
        case GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING: { content.unitSize = 1; break; }
        case GUCEF_DATATYPE_UTF16_LE_STRING: { content.unitSize = 2; break; }
        case GUCEF_DATATYPE_UTF16_BE_STRING: { content.unitSize = 2; content.isBigEndian = true; break; }
        case GUCEF_DATATYPE_UTF32_LE_STRING: { content.unitSize = 4; break; }
        case GUCEF_DATATYPE_UTF32_BE_STRING: { content.unitSize = 4; content.isBigEndian = true; break; }
        case GUCEF_DATATYPE_BINARY_BLOB:
        case GUCEF_DATATYPE_BINARY_BSOB: { content.unitSize = 1; content.isText = false; break; }
        default: { return; }
    }

    content.data = static_cast< const UInt8* >( var.AsVoidPtr() );
    content.size = GUCEF_NULL != content.data ? var.ByteSize( true ) : 0;

    // Text ends at its first null code unit, if any: a terminator is not part of the text
    if ( content.isText )
    {
        UInt32 units = content.size / content.unitSize;
        UInt32 length = 0;
        while ( length < units )
        {
            const UInt8* unit = content.data + ( length * content.unitSize );
            bool isNull = true;
            for ( UInt32 b=0; b<content.unitSize; ++b )
            {
                if ( 0 != unit[ b ] )
                {
                    isNull = false;
                    break;
                }
            }
            if ( isNull )
                break;
            ++length;
        }
        content.size = length * content.unitSize;
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  Reads text or binary content as the bytes it is compared by: binary data as its bytes, text as its UTF-8 encoding.
 *  The UTF-8 bytes of two texts order the same way as their code points, so text in any of the encodings compares by
 *  code point. Unpaired surrogates and values which are not code points read as U+FFFD
 */
class CComparisonByteReader
{
    public:

    explicit CComparisonByteReader( const SComparisonContent& content )
        : m_content( content )
        , m_offset( 0 )
        , m_pendingCount( 0 )
        , m_pendingOffset( 0 )
    {
    }

    bool Next( UInt8& byte )
    {
        if ( m_pendingOffset < m_pendingCount )
        {
            byte = m_pending[ m_pendingOffset++ ];
            return true;
        }
        if ( 1 == m_content.unitSize )
        {
            if ( m_offset >= m_content.size )
                return false;
            byte = m_content.data[ m_offset++ ];
            return true;
        }

        UInt32 codePoint = 0;
        if ( !NextCodePoint( codePoint ) )
            return false;
        EncodeUtf8( codePoint );
        byte = m_pending[ m_pendingOffset++ ];
        return true;
    }

    private:

    UInt32 ReadUnit( void )
    {
        const UInt8* unit = m_content.data + m_offset;
        m_offset += m_content.unitSize;
        if ( 2 == m_content.unitSize )
        {
            return m_content.isBigEndian ? ( ( (UInt32) unit[ 0 ] << 8 ) | unit[ 1 ] ) : ( ( (UInt32) unit[ 1 ] << 8 ) | unit[ 0 ] );
        }
        return m_content.isBigEndian ? ( ( (UInt32) unit[ 0 ] << 24 ) | ( (UInt32) unit[ 1 ] << 16 ) | ( (UInt32) unit[ 2 ] << 8 ) | unit[ 3 ] )
                                     : ( ( (UInt32) unit[ 3 ] << 24 ) | ( (UInt32) unit[ 2 ] << 16 ) | ( (UInt32) unit[ 1 ] << 8 ) | unit[ 0 ] );
    }

    bool NextCodePoint( UInt32& codePoint )
    {
        if ( m_offset + m_content.unitSize > m_content.size )
            return false;

        codePoint = ReadUnit();
        if ( 2 == m_content.unitSize )
        {
            if ( codePoint >= 0xD800 && codePoint <= 0xDBFF && m_offset + 2 <= m_content.size )
            {
                const UInt8* next = m_content.data + m_offset;
                UInt32 low = m_content.isBigEndian ? ( ( (UInt32) next[ 0 ] << 8 ) | next[ 1 ] ) : ( ( (UInt32) next[ 1 ] << 8 ) | next[ 0 ] );
                if ( low >= 0xDC00 && low <= 0xDFFF )
                {
                    m_offset += 2;
                    codePoint = 0x10000 + ( ( codePoint - 0xD800 ) << 10 ) + ( low - 0xDC00 );
                    return true;
                }
            }
        }
        if ( ( codePoint >= 0xD800 && codePoint <= 0xDFFF ) || codePoint > 0x10FFFF )
            codePoint = 0xFFFD;
        return true;
    }

    void EncodeUtf8( UInt32 codePoint )
    {
        m_pendingOffset = 0;
        if ( codePoint < 0x80 )
        {
            m_pending[ 0 ] = (UInt8) codePoint;
            m_pendingCount = 1;
        }
        else
        if ( codePoint < 0x800 )
        {
            m_pending[ 0 ] = (UInt8) ( 0xC0 | ( codePoint >> 6 ) );
            m_pending[ 1 ] = (UInt8) ( 0x80 | ( codePoint & 0x3F ) );
            m_pendingCount = 2;
        }
        else
        if ( codePoint < 0x10000 )
        {
            m_pending[ 0 ] = (UInt8) ( 0xE0 | ( codePoint >> 12 ) );
            m_pending[ 1 ] = (UInt8) ( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) );
            m_pending[ 2 ] = (UInt8) ( 0x80 | ( codePoint & 0x3F ) );
            m_pendingCount = 3;
        }
        else
        {
            m_pending[ 0 ] = (UInt8) ( 0xF0 | ( codePoint >> 18 ) );
            m_pending[ 1 ] = (UInt8) ( 0x80 | ( ( codePoint >> 12 ) & 0x3F ) );
            m_pending[ 2 ] = (UInt8) ( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) );
            m_pending[ 3 ] = (UInt8) ( 0x80 | ( codePoint & 0x3F ) );
            m_pendingCount = 4;
        }
    }

    const SComparisonContent& m_content;
    UInt32 m_offset;
    UInt8 m_pending[ 4 ];
    UInt32 m_pendingCount;
    UInt32 m_pendingOffset;
};

/*-------------------------------------------------------------------------*/

/** Compares text and binary content fully: lexicographic, a shorter value which is a prefix of the other sorts first */
static Int32
CompareContent( const SComparisonContent& a, const SComparisonContent& b )
{
    if ( 1 == a.unitSize && 1 == b.unitSize )
    {
        UInt32 smallestSize = GUCEF_SMALLEST( a.size, b.size );
        int cmp = smallestSize > 0 ? memcmp( a.data, b.data, smallestSize ) : 0;
        if ( 0 != cmp )
            return cmp < 0 ? -1 : 1;
        return a.size == b.size ? 0 : ( a.size < b.size ? -1 : 1 );
    }

    CComparisonByteReader readerA( a );
    CComparisonByteReader readerB( b );
    while ( true )
    {
        UInt8 byteA = 0;
        UInt8 byteB = 0;
        bool hasA = readerA.Next( byteA );
        bool hasB = readerB.Next( byteB );
        if ( !hasA || !hasB )
            return hasA == hasB ? 0 : ( hasA ? 1 : -1 );
        if ( byteA != byteB )
            return byteA < byteB ? -1 : 1;
    }
}

/*-------------------------------------------------------------------------*/

/** Whether the value is compared as a number when the other side is a number too */
static bool
IsComparedAsNumber( const CVariant& var )
{
    return var.IsInteger() || var.IsFloat() || var.IsFraction() || var.IsDecimal();
}

/*-------------------------------------------------------------------------*/

/** The order of the kinds of values which have no comparison in common */
static Int32
ComparisonCategory( const CVariant& var, const SComparisonContent& content )
{
    if ( 0 != content.unitSize )
        return 2;
    if ( IsComparedAsNumber( var ) )
        return 1;
    switch ( var.GetTypeId() )
    {
        case GUCEF_DATATYPE_UNKNOWN:
        case GUCEF_DATATYPE_NIL:
        case GUCEF_DATATYPE_NULL:
        case GUCEF_DATATYPE_VOID: { return 0; }
        default: { return 3; }
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  The comparison of the values which the comparison operators do not compare as numbers or booleans
 *  Text compares to text by code point whatever the encoding, binary data compares by its bytes and text compares to
 *  binary data by its UTF-8 encoding. Other values of the same type compare by their bytes. Values of different
 *  kinds are never equal: they order by kind, then by type.
 *  This is a total order, so variants can be the keys of ordered containers
 */
static Int32
CompareRemaining( const CVariant& a, const CVariant& b )
{
    SComparisonContent contentA;
    SComparisonContent contentB;
    GetComparisonContent( a, contentA );
    GetComparisonContent( b, contentB );

    if ( 0 != contentA.unitSize && 0 != contentB.unitSize )
        return CompareContent( contentA, contentB );

    if ( a.GetTypeId() == b.GetTypeId() )
    {
        UInt32 sizeA = a.ByteSize();
        UInt32 sizeB = b.ByteSize();
        const void* dataA = a.AsVoidPtr();
        const void* dataB = b.AsVoidPtr();
        UInt32 smallestSize = GUCEF_SMALLEST( sizeA, sizeB );
        int cmp = ( smallestSize > 0 && GUCEF_NULL != dataA && GUCEF_NULL != dataB ) ? memcmp( dataA, dataB, smallestSize ) : 0;
        if ( 0 != cmp )
            return cmp < 0 ? -1 : 1;
        return sizeA == sizeB ? 0 : ( sizeA < sizeB ? -1 : 1 );
    }

    Int32 categoryA = ComparisonCategory( a, contentA );
    Int32 categoryB = ComparisonCategory( b, contentB );
    if ( categoryA != categoryB )
        return categoryA < categoryB ? -1 : 1;
    return a.GetTypeId() < b.GetTypeId() ? -1 : 1;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVariant::CVariant( void )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.containedType = GUCEF_DATATYPE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const TVariantData* variantData, bool linkIfPossible )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    Set( variantData, linkIfPossible );
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const TVariantData& variantData, bool linkIfPossible )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    assert( GUCEF_NULL != &variantData );
    Set( &variantData, linkIfPossible );
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( bool data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.containedType = GUCEF_DATATYPE_BOOLEAN_INT32;
    m_variantData.union_data.int32_data = data ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( Int8 data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.containedType = GUCEF_DATATYPE_INT8;
    m_variantData.union_data.int8_data = data;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( UInt8 data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.containedType = GUCEF_DATATYPE_UINT8;
    m_variantData.union_data.uint8_data = data;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( Int16 data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.containedType = GUCEF_DATATYPE_INT16;
    m_variantData.union_data.int16_data = data;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( UInt16 data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.containedType = GUCEF_DATATYPE_UINT16;
    m_variantData.union_data.uint16_data = data;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( Int32 data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.containedType = GUCEF_DATATYPE_INT32;
    m_variantData.union_data.int32_data = data;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( UInt32 data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.containedType = GUCEF_DATATYPE_UINT32;
    m_variantData.union_data.uint32_data = data;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( Int64 data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.containedType = GUCEF_DATATYPE_INT64;
    m_variantData.union_data.int64_data = data;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( UInt64 data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.containedType = GUCEF_DATATYPE_UINT64;
    m_variantData.union_data.uint64_data = data;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( Float32 data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.containedType = GUCEF_DATATYPE_FLOAT32;
    m_variantData.union_data.float32_data = data;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( Float64 data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.containedType = GUCEF_DATATYPE_FLOAT64;
    m_variantData.union_data.float64_data = data;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const CAsciiString& data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    *this = data;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const CUtf8String& data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    *this = data;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const CUtf16String& data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    LinkTo( data );
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const CUtf32String& data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    LinkTo( data );
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const char* utf8Str )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    *this = CUtf8String( utf8Str );
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const std::string& utf8Str )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    *this = utf8Str;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const CDateTime& datetime )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    *this = datetime;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const CTimestamp& timestamp )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    *this = timestamp;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const void* data, UInt32 dataSize, UInt8 varType )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    Set( data, dataSize, varType );
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( UInt8 varType, const CString& stringFormData, const CVariant& defaultValue )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    SetFromString( varType, stringFormData, defaultValue );
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const CDynamicBuffer& data, UInt8 varType )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    Set( data.GetConstBufferPtr(), data.GetDataSize(), varType );
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const CVariant& data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    Set( &data.m_variantData, false );
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const CVariant& data ,
                    bool linkIfPossible  )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    Set( &data.m_variantData, linkIfPossible );
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( void* data_memory_address )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.containedType = GUCEF_DATATYPE_DATA_MEMORY_ADDRESS;
    m_variantData.union_data.memory_address_data.objPtr = data_memory_address;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( TDefaultFuncPtr function_memory_address )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.containedType = GUCEF_DATATYPE_FUNCTION_MEMORY_ADDRESS;
    m_variantData.union_data.memory_address_data.funcPtr = function_memory_address;
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( TInt32Fraction data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.containedType = GUCEF_DATATYPE_INT32T2_FRACTION;
    m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator = data.GetNumerator();
    m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator = data.GetDenominator();
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( TUInt32Fraction data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.containedType = GUCEF_DATATYPE_UINT32T2_FRACTION;
    m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator = data.GetNumerator();
    m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator = data.GetDenominator();
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const TInt64Int32Fraction& data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.containedType = GUCEF_DATATYPE_INT64_INT32_FRACTION;
    m_variantData.union_data.fraction_data.union_data.int64_int32_data.numerator = data.GetNumerator();
    m_variantData.union_data.fraction_data.union_data.int64_int32_data.denominator = data.GetDenominator();
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const TUInt64UInt32Fraction& data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.containedType = GUCEF_DATATYPE_UINT64_UINT32_FRACTION;
    m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.numerator = data.GetNumerator();
    m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.denominator = data.GetDenominator();
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const TInt64Fraction& data )
    : m_variantData()
{GUCEF_TRACE;

    TInt64T2Fraction cData = { data.GetNumerator(), data.GetDenominator() };
    memset( &m_variantData, 0, sizeof( m_variantData ) );
    Set( &cData, sizeof( cData ), GUCEF_DATATYPE_INT64T2_FRACTION );
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const TUInt64Fraction& data )
    : m_variantData()
{GUCEF_TRACE;

    TUInt64T2Fraction cData = { data.GetNumerator(), data.GetDenominator() };
    memset( &m_variantData, 0, sizeof( m_variantData ) );
    Set( &cData, sizeof( cData ), GUCEF_DATATYPE_UINT64T2_FRACTION );
}

/*-------------------------------------------------------------------------*/

CVariant::CVariant( const CDecimal& data )
    : m_variantData()
{GUCEF_TRACE;

    memset( &m_variantData, 0, sizeof( m_variantData ) );
    m_variantData.union_data.decimal_data.unscaled = data.GetUnscaledValue();
    m_variantData.union_data.decimal_data.scale = data.GetScale();
    m_variantData.containedType = GUCEF_DATATYPE_INT64_UINT8_DECIMAL;
}

/*-------------------------------------------------------------------------*/

bool
CVariant::Set( const TVariantData* src, bool linkOnlyForDynMem )
{GUCEF_TRACE;

    Clear();

    if ( GUCEF_NULL != src )
    {
        // If dynamic memory is used we need to actually copy said memory
        // into a private copy
        if ( UsesDynamicMemory( src->containedType ) )
        {
            if ( !linkOnlyForDynMem )
            {
                if ( 0 < src->union_data.heap_data.heap_data_size && GUCEF_NULL != src->union_data.heap_data.union_data.void_heap_data )
                {
                    m_variantData.union_data.heap_data.union_data.void_heap_data = malloc( (size_t) src->union_data.heap_data.heap_data_size );
                    assert( GUCEF_NULL != m_variantData.union_data.heap_data.union_data.void_heap_data );
                    memcpy( m_variantData.union_data.heap_data.union_data.void_heap_data, src->union_data.heap_data.union_data.void_heap_data, (size_t) src->union_data.heap_data.heap_data_size );
                    m_variantData.union_data.heap_data.heap_data_size = src->union_data.heap_data.heap_data_size;
                }
                else
                {
                    m_variantData.union_data.heap_data.union_data.void_heap_data = GUCEF_NULL;
                    m_variantData.union_data.heap_data.heap_data_size = 0;
                }
                m_variantData.union_data.heap_data.heap_data_is_linked = 0;
                m_variantData.containedType = src->containedType;
            }
            else
            {
                memcpy( &m_variantData, src, sizeof( m_variantData ) );
                m_variantData.union_data.heap_data.heap_data_is_linked = 1;
            }
        }
        else
        {
            memcpy( &m_variantData, src, sizeof( m_variantData ) );
        }
        return true;
    }
    else
    {
        memset( &m_variantData, 0, sizeof( m_variantData ) );
        return true;
    }
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED
CVariant::CVariant( CVariant&& src ) GUCEF_NOEXCEPT
    : m_variantData( src.m_variantData )
{GUCEF_TRACE;

    memset( &src.m_variantData, 0, sizeof( src.m_variantData ) );
}
#endif

/*-------------------------------------------------------------------------*/

bool
CVariant::IsLittleEndian( void ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_LE_INT16:
        case GUCEF_DATATYPE_LE_UINT16:
        case GUCEF_DATATYPE_LE_INT32:
        case GUCEF_DATATYPE_LE_UINT32:
        case GUCEF_DATATYPE_LE_INT64:
        case GUCEF_DATATYPE_LE_UINT64:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_UTF16_LE_STRING:
        case GUCEF_DATATYPE_UTF32_LE_STRING:
        case GUCEF_DATATYPE_BOOLEAN_UTF16_LE_STRING:
        case GUCEF_DATATYPE_BOOLEAN_UTF32_LE_STRING:
        case GUCEF_DATATYPE_INT32T2_LE_FRACTION:
        case GUCEF_DATATYPE_UINT32T2_LE_FRACTION:
        case GUCEF_DATATYPE_INT64T2_LE_FRACTION:
        case GUCEF_DATATYPE_UINT64T2_LE_FRACTION:
        case GUCEF_DATATYPE_INT64_INT32_LE_FRACTION:
        case GUCEF_DATATYPE_UINT64_UINT32_LE_FRACTION:
        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
        case GUCEF_DATATYPE_LE_FLOAT32:
        case GUCEF_DATATYPE_LE_FLOAT64:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsBigEndian( void ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_BE_INT16:
        case GUCEF_DATATYPE_BE_UINT16:
        case GUCEF_DATATYPE_BE_INT32:
        case GUCEF_DATATYPE_BE_UINT32:
        case GUCEF_DATATYPE_BE_INT64:
        case GUCEF_DATATYPE_BE_UINT64:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_UTF16_BE_STRING:
        case GUCEF_DATATYPE_UTF32_BE_STRING:
        case GUCEF_DATATYPE_BOOLEAN_UTF16_BE_STRING:
        case GUCEF_DATATYPE_BOOLEAN_UTF32_BE_STRING:
        case GUCEF_DATATYPE_INT32T2_BE_FRACTION:
        case GUCEF_DATATYPE_UINT32T2_BE_FRACTION:
        case GUCEF_DATATYPE_INT64T2_BE_FRACTION:
        case GUCEF_DATATYPE_UINT64T2_BE_FRACTION:
        case GUCEF_DATATYPE_INT64_INT32_BE_FRACTION:
        case GUCEF_DATATYPE_UINT64_UINT32_BE_FRACTION:
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
        case GUCEF_DATATYPE_BE_FLOAT32:
        case GUCEF_DATATYPE_BE_FLOAT64:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsEndianAgnostic( void ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT8:
        case GUCEF_DATATYPE_UINT8:
        case GUCEF_DATATYPE_NUMERIC:
        case GUCEF_DATATYPE_ARRAY:
        case GUCEF_DATATYPE_OBJECT:
        case GUCEF_DATATYPE_NIL:
        case GUCEF_DATATYPE_NULL:
        case GUCEF_DATATYPE_SET:
        case GUCEF_DATATYPE_ASCII_STRING:
        case GUCEF_DATATYPE_UTF8_STRING:
        case GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING:
        case GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING:
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING:
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING:
        case GUCEF_DATATYPE_BINARY_BLOB:
        case GUCEF_DATATYPE_BINARY_BSOB:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::ToLittleEndian( void )
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_BE_INT16:
            { m_variantData.union_data.int16_data = SwapEndianInt16( m_variantData.union_data.int16_data ); m_variantData.containedType = GUCEF_DATATYPE_LE_INT16; return *this; }
        case GUCEF_DATATYPE_BE_UINT16:
            { m_variantData.union_data.uint16_data = SwapEndianUInt16( m_variantData.union_data.uint16_data ); m_variantData.containedType = GUCEF_DATATYPE_LE_UINT16; return *this; }
        case GUCEF_DATATYPE_BE_INT32:
            { m_variantData.union_data.int32_data = SwapEndianInt32( m_variantData.union_data.int32_data ); m_variantData.containedType = GUCEF_DATATYPE_LE_INT32; return *this; }
        case GUCEF_DATATYPE_BE_UINT32:
            { m_variantData.union_data.uint32_data = SwapEndianUInt32( m_variantData.union_data.uint32_data ); m_variantData.containedType = GUCEF_DATATYPE_LE_UINT32; return *this; }
        case GUCEF_DATATYPE_BE_INT64:
            { m_variantData.union_data.int64_data = SwapEndianInt64( m_variantData.union_data.int64_data ); m_variantData.containedType = GUCEF_DATATYPE_LE_INT64; return *this; }
        case GUCEF_DATATYPE_BE_UINT64:
            { m_variantData.union_data.uint64_data = SwapEndianUInt64( m_variantData.union_data.uint64_data ); m_variantData.containedType = GUCEF_DATATYPE_LE_UINT64; return *this; }
        case GUCEF_DATATYPE_BE_FLOAT32:
            { m_variantData.union_data.float32_data = SwapEndianFloat32( m_variantData.union_data.float32_data ); m_variantData.containedType = GUCEF_DATATYPE_LE_FLOAT32; return *this; }
        case GUCEF_DATATYPE_BE_FLOAT64:
            { m_variantData.union_data.float64_data = SwapEndianFloat64( m_variantData.union_data.float64_data ); m_variantData.containedType = GUCEF_DATATYPE_LE_FLOAT64; return *this; }

        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
            { m_variantData.union_data.uint64_data = SwapEndianUInt64( m_variantData.union_data.uint64_data ); m_variantData.containedType = GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH; return *this; }
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
            { m_variantData.union_data.uint64_data = SwapEndianUInt64( m_variantData.union_data.uint64_data ); m_variantData.containedType = GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH; return *this; }
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
            { m_variantData.union_data.uint64_data = SwapEndianUInt64( m_variantData.union_data.uint64_data ); m_variantData.containedType = GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH; return *this; }
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
            { m_variantData.union_data.int64_data = SwapEndianInt64( m_variantData.union_data.int64_data ); m_variantData.containedType = GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH; return *this; }
        case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
            { m_variantData.union_data.int32_data = SwapEndianInt32( m_variantData.union_data.int32_data ); m_variantData.containedType = GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH; return *this; }


        case GUCEF_DATATYPE_BOOLEAN_UTF16_BE_STRING:
        case GUCEF_DATATYPE_UTF16_BE_STRING:
            {
                UInt16* dataArray = static_cast< UInt16* >( m_variantData.union_data.heap_data.union_data.void_heap_data );
                UInt32 dataSize = m_variantData.union_data.heap_data.heap_data_size / sizeof( UInt16 );
                for ( UInt32 i=0; i<dataSize; ++i )
                {
                    dataArray[ i ] = SwapEndianUInt16( dataArray[ i ] );
                }
                m_variantData.containedType = m_variantData.containedType == GUCEF_DATATYPE_UTF16_BE_STRING ? GUCEF_DATATYPE_UTF16_LE_STRING : GUCEF_DATATYPE_BOOLEAN_UTF16_LE_STRING;
                return *this;
            }
        case GUCEF_DATATYPE_BOOLEAN_UTF32_BE_STRING:
        case GUCEF_DATATYPE_UTF32_BE_STRING:
            {
                UInt32* dataArray = static_cast< UInt32* >( m_variantData.union_data.heap_data.union_data.void_heap_data );
                UInt32 dataSize = m_variantData.union_data.heap_data.heap_data_size / sizeof( UInt32 );
                for ( UInt32 i=0; i<dataSize; ++i )
                {
                    dataArray[ i ] = SwapEndianUInt32( dataArray[ i ] );
                }
                m_variantData.containedType = m_variantData.containedType == GUCEF_DATATYPE_UTF32_BE_STRING ? GUCEF_DATATYPE_UTF32_LE_STRING : GUCEF_DATATYPE_BOOLEAN_UTF32_LE_STRING;
                return *this;
            }


        case GUCEF_DATATYPE_INT32T2_BE_FRACTION:
            {
                m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator = SwapEndianInt32( m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator );
                m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator = SwapEndianInt32( m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator );
                m_variantData.containedType = GUCEF_DATATYPE_INT32T2_LE_FRACTION;
                return *this;
            }
        case GUCEF_DATATYPE_UINT32T2_BE_FRACTION:
            {
                m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator = SwapEndianUInt32( m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator );
                m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator = SwapEndianUInt32( m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator );
                m_variantData.containedType = GUCEF_DATATYPE_UINT32T2_LE_FRACTION;
                return *this;
            }
        case GUCEF_DATATYPE_INT64_INT32_BE_FRACTION:
            {
                m_variantData.union_data.fraction_data.union_data.int64_int32_data.numerator = SwapEndianInt64( m_variantData.union_data.fraction_data.union_data.int64_int32_data.numerator );
                m_variantData.union_data.fraction_data.union_data.int64_int32_data.denominator = SwapEndianInt32( m_variantData.union_data.fraction_data.union_data.int64_int32_data.denominator );
                m_variantData.containedType = GUCEF_DATATYPE_INT64_INT32_LE_FRACTION;
                return *this;
            }
        case GUCEF_DATATYPE_UINT64_UINT32_BE_FRACTION:
        {
                m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.numerator = SwapEndianUInt64( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.numerator );
                m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.denominator = SwapEndianUInt32( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.denominator );
                m_variantData.containedType = GUCEF_DATATYPE_UINT64_UINT32_LE_FRACTION;
                return *this;
        }

        case GUCEF_DATATYPE_INT64T2_BE_FRACTION:
            {
                m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->numerator = SwapEndianInt64( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->numerator );
                m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->denominator = SwapEndianInt64( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->denominator );
                m_variantData.containedType = GUCEF_DATATYPE_INT64T2_LE_FRACTION;
                return *this;
            }
        case GUCEF_DATATYPE_UINT64T2_BE_FRACTION:
            {
                m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->numerator = SwapEndianInt64( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->numerator );
                m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->denominator = SwapEndianInt64( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->denominator );
                m_variantData.containedType = GUCEF_DATATYPE_UINT64T2_LE_FRACTION;
                return *this;
            }

        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
            {
                m_variantData.union_data.decimal_data.unscaled = SwapEndianInt64( m_variantData.union_data.decimal_data.unscaled );
                m_variantData.containedType = GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL;
                return *this;
            }

        default:
        {
            // No changes needed
            return *this;
        }
    }
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::ToBigEndian( void )
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_LE_INT16:
            { m_variantData.union_data.int16_data = SwapEndianInt16( m_variantData.union_data.int16_data ); m_variantData.containedType = GUCEF_DATATYPE_BE_INT16; return *this; }
        case GUCEF_DATATYPE_LE_UINT16:
            { m_variantData.union_data.uint16_data = SwapEndianUInt16( m_variantData.union_data.uint16_data ); m_variantData.containedType = GUCEF_DATATYPE_BE_UINT16; return *this; }
        case GUCEF_DATATYPE_LE_INT32:
            { m_variantData.union_data.int32_data = SwapEndianInt32( m_variantData.union_data.int32_data ); m_variantData.containedType = GUCEF_DATATYPE_BE_INT32; return *this; }
        case GUCEF_DATATYPE_LE_UINT32:
            { m_variantData.union_data.uint32_data = SwapEndianUInt32( m_variantData.union_data.uint32_data ); m_variantData.containedType = GUCEF_DATATYPE_BE_UINT32; return *this; }
        case GUCEF_DATATYPE_LE_INT64:
            { m_variantData.union_data.int64_data = SwapEndianInt64( m_variantData.union_data.int64_data ); m_variantData.containedType = GUCEF_DATATYPE_BE_INT64; return *this; }
        case GUCEF_DATATYPE_LE_UINT64:
            { m_variantData.union_data.uint64_data = SwapEndianUInt64( m_variantData.union_data.uint64_data ); m_variantData.containedType = GUCEF_DATATYPE_BE_UINT64; return *this; }
        case GUCEF_DATATYPE_LE_FLOAT32:
            { m_variantData.union_data.float32_data = SwapEndianFloat32( m_variantData.union_data.float32_data ); m_variantData.containedType = GUCEF_DATATYPE_BE_FLOAT32; return *this; }
        case GUCEF_DATATYPE_LE_FLOAT64:
            { m_variantData.union_data.float64_data = SwapEndianFloat64( m_variantData.union_data.float64_data ); m_variantData.containedType = GUCEF_DATATYPE_BE_FLOAT64; return *this; }


        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
            { m_variantData.union_data.uint64_data = SwapEndianUInt64( m_variantData.union_data.uint64_data ); m_variantData.containedType = GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH; return *this; }
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
            { m_variantData.union_data.uint64_data = SwapEndianUInt64( m_variantData.union_data.uint64_data ); m_variantData.containedType = GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH; return *this; }
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
            { m_variantData.union_data.uint64_data = SwapEndianUInt64( m_variantData.union_data.uint64_data ); m_variantData.containedType = GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH; return *this; }
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
            { m_variantData.union_data.int64_data = SwapEndianInt64( m_variantData.union_data.int64_data ); m_variantData.containedType = GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH; return *this; }
        case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
            { m_variantData.union_data.int32_data = SwapEndianInt32( m_variantData.union_data.int32_data ); m_variantData.containedType = GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH; return *this; }


        case GUCEF_DATATYPE_BOOLEAN_UTF16_LE_STRING:
        case GUCEF_DATATYPE_UTF16_LE_STRING:
            {
                UInt16* dataArray = static_cast< UInt16* >( m_variantData.union_data.heap_data.union_data.void_heap_data );
                UInt32 dataSize = m_variantData.union_data.heap_data.heap_data_size / sizeof( UInt16 );
                for ( UInt32 i=0; i<dataSize; ++i )
                {
                    dataArray[ i ] = SwapEndianUInt16( dataArray[ i ] );
                }
                m_variantData.containedType = m_variantData.containedType == GUCEF_DATATYPE_UTF16_LE_STRING ? GUCEF_DATATYPE_UTF16_BE_STRING : GUCEF_DATATYPE_BOOLEAN_UTF16_BE_STRING;
                return *this;
            }
        case GUCEF_DATATYPE_BOOLEAN_UTF32_LE_STRING:
        case GUCEF_DATATYPE_UTF32_LE_STRING:
            {
                UInt32* dataArray = static_cast< UInt32* >( m_variantData.union_data.heap_data.union_data.void_heap_data );
                UInt32 dataSize = m_variantData.union_data.heap_data.heap_data_size / sizeof( UInt32 );
                for ( UInt32 i=0; i<dataSize; ++i )
                {
                    dataArray[ i ] = SwapEndianUInt32( dataArray[ i ] );
                }
                m_variantData.containedType = m_variantData.containedType == GUCEF_DATATYPE_UTF32_LE_STRING ? GUCEF_DATATYPE_UTF32_BE_STRING : GUCEF_DATATYPE_BOOLEAN_UTF32_BE_STRING;
                return *this;
            }


        case GUCEF_DATATYPE_INT32T2_LE_FRACTION:
            {
                m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator = SwapEndianInt32( m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator );
                m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator = SwapEndianInt32( m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator );
                m_variantData.containedType = GUCEF_DATATYPE_INT32T2_BE_FRACTION;
                return *this;
            }
        case GUCEF_DATATYPE_UINT32T2_LE_FRACTION:
            {
                m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator = SwapEndianUInt32( m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator );
                m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator = SwapEndianUInt32( m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator );
                m_variantData.containedType = GUCEF_DATATYPE_UINT32T2_BE_FRACTION;
                return *this;
            }
        case GUCEF_DATATYPE_INT64_INT32_LE_FRACTION:
            {
                m_variantData.union_data.fraction_data.union_data.int64_int32_data.numerator = SwapEndianInt64( m_variantData.union_data.fraction_data.union_data.int64_int32_data.numerator );
                m_variantData.union_data.fraction_data.union_data.int64_int32_data.denominator = SwapEndianInt32( m_variantData.union_data.fraction_data.union_data.int64_int32_data.denominator );
                m_variantData.containedType = GUCEF_DATATYPE_INT64_INT32_BE_FRACTION;
                return *this;
            }
        case GUCEF_DATATYPE_UINT64_UINT32_LE_FRACTION:
        {
                m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.numerator = SwapEndianUInt64( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.numerator );
                m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.denominator = SwapEndianUInt32( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.denominator );
                m_variantData.containedType = GUCEF_DATATYPE_UINT64_UINT32_BE_FRACTION;
                return *this;
        }

        case GUCEF_DATATYPE_INT64T2_LE_FRACTION:
            {
                m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->numerator = SwapEndianInt64( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->numerator );
                m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->denominator = SwapEndianInt64( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->denominator );
                m_variantData.containedType = GUCEF_DATATYPE_INT64T2_BE_FRACTION;
                return *this;
            }
        case GUCEF_DATATYPE_UINT64T2_LE_FRACTION:
            {
                m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->numerator = SwapEndianInt64( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->numerator );
                m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->denominator = SwapEndianInt64( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->denominator );
                m_variantData.containedType = GUCEF_DATATYPE_UINT64T2_BE_FRACTION;
                return *this;
            }

        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
            {
                m_variantData.union_data.decimal_data.unscaled = SwapEndianInt64( m_variantData.union_data.decimal_data.unscaled );
                m_variantData.containedType = GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL;
                return *this;
            }

        default:
        {
            // No changes needed
            return *this;
        }
    }
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::ToNativeEndian( void )
{GUCEF_TRACE;

    // Most platforms are little endian so if we dont know we will guess little endian
    #if ( ( GUCEF_BYTEORDER_ENDIAN_COMPILE_TIME == GUCEF_BYTEORDER_LITTLE_ENDIAN ) || ( GUCEF_BYTEORDER_ENDIAN_COMPILE_TIME == GUCEF_BYTEORDER_UNKNOWN_ENDIAN ) )
    return ToLittleEndian();
    #else
    return ToBigEndian();
    #endif
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsMemoryAddress( void ) const
{GUCEF_TRACE;

    return m_variantData.containedType == GUCEF_DATATYPE_DATA_MEMORY_ADDRESS ||
           m_variantData.containedType == GUCEF_DATATYPE_FUNCTION_MEMORY_ADDRESS;
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsVoid( void ) const
{GUCEF_TRACE;

    return m_variantData.containedType == GUCEF_DATATYPE_VOID;
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsNIL( void ) const
{GUCEF_TRACE;

    return m_variantData.containedType == GUCEF_DATATYPE_NIL;
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsNULL( void ) const
{GUCEF_TRACE;

    return m_variantData.containedType == GUCEF_DATATYPE_NULL;
}

/*-------------------------------------------------------------------------*/

void
CVariant::SetVoidType( void )
{GUCEF_TRACE;

    Clear();
    m_variantData.containedType = GUCEF_DATATYPE_VOID;
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsInteger( void ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT8:
        case GUCEF_DATATYPE_UINT8:
        case GUCEF_DATATYPE_LE_INT16:
        case GUCEF_DATATYPE_BE_INT16:
        case GUCEF_DATATYPE_LE_UINT16:
        case GUCEF_DATATYPE_BE_UINT16:
        case GUCEF_DATATYPE_LE_INT32:
        case GUCEF_DATATYPE_BE_INT32:
        case GUCEF_DATATYPE_LE_UINT32:
        case GUCEF_DATATYPE_BE_UINT32:
        case GUCEF_DATATYPE_LE_INT64:
        case GUCEF_DATATYPE_BE_INT64:
        case GUCEF_DATATYPE_LE_UINT64:
        case GUCEF_DATATYPE_BE_UINT64:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsSignedInteger( void ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT8:
        case GUCEF_DATATYPE_LE_INT16:
        case GUCEF_DATATYPE_BE_INT16:
        case GUCEF_DATATYPE_LE_INT32:
        case GUCEF_DATATYPE_BE_INT32:
        case GUCEF_DATATYPE_LE_INT64:
        case GUCEF_DATATYPE_BE_INT64:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsUnsignedInteger( void ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_UINT8:
        case GUCEF_DATATYPE_LE_UINT16:
        case GUCEF_DATATYPE_BE_UINT16:
        case GUCEF_DATATYPE_LE_UINT32:
        case GUCEF_DATATYPE_BE_UINT32:
        case GUCEF_DATATYPE_LE_UINT64:
        case GUCEF_DATATYPE_BE_UINT64:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsFloat( void ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_FLOAT32:
        case GUCEF_DATATYPE_FLOAT64:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsNumber( void ) const
{GUCEF_TRACE;

    return IsInteger() || IsFloat() || IsFraction() || IsDecimal() ||
        m_variantData.containedType == GUCEF_DATATYPE_NUMERIC;
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsFraction( void ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT32T2_LE_FRACTION:
        case GUCEF_DATATYPE_INT32T2_BE_FRACTION:
        case GUCEF_DATATYPE_UINT32T2_LE_FRACTION:
        case GUCEF_DATATYPE_UINT32T2_BE_FRACTION:
        case GUCEF_DATATYPE_INT64T2_LE_FRACTION:
        case GUCEF_DATATYPE_INT64T2_BE_FRACTION:
        case GUCEF_DATATYPE_UINT64T2_LE_FRACTION:
        case GUCEF_DATATYPE_UINT64T2_BE_FRACTION:
        case GUCEF_DATATYPE_INT64_INT32_LE_FRACTION:
        case GUCEF_DATATYPE_INT64_INT32_BE_FRACTION:
        case GUCEF_DATATYPE_UINT64_UINT32_LE_FRACTION:
        case GUCEF_DATATYPE_UINT64_UINT32_BE_FRACTION:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsSignedFraction( void ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT32T2_LE_FRACTION:
        case GUCEF_DATATYPE_INT32T2_BE_FRACTION:
        case GUCEF_DATATYPE_INT64T2_LE_FRACTION:
        case GUCEF_DATATYPE_INT64T2_BE_FRACTION:
        case GUCEF_DATATYPE_INT64_INT32_LE_FRACTION:
        case GUCEF_DATATYPE_INT64_INT32_BE_FRACTION:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsUnsignedFraction( void ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_UINT32T2_LE_FRACTION:
        case GUCEF_DATATYPE_UINT32T2_BE_FRACTION:
        case GUCEF_DATATYPE_UINT64T2_LE_FRACTION:
        case GUCEF_DATATYPE_UINT64T2_BE_FRACTION:
        case GUCEF_DATATYPE_UINT64_UINT32_LE_FRACTION:
        case GUCEF_DATATYPE_UINT64_UINT32_BE_FRACTION:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsDecimal( void ) const
{GUCEF_TRACE;

    return IsDecimalTypeId( m_variantData.containedType );
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsString( void ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_ASCII_STRING:
        case GUCEF_DATATYPE_UTF8_STRING:
        case GUCEF_DATATYPE_UTF16_LE_STRING:
        case GUCEF_DATATYPE_UTF16_BE_STRING:
        case GUCEF_DATATYPE_UTF32_STRING:
        case GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING:
        case GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsBoolean( void ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_BOOLEAN_INT32:
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING:
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsBinary( void ) const
{GUCEF_TRACE;

    return m_variantData.containedType == GUCEF_DATATYPE_BINARY_BLOB || m_variantData.containedType == GUCEF_DATATYPE_BINARY_BSOB;
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsBlob( void ) const
{GUCEF_TRACE;

    return m_variantData.containedType == GUCEF_DATATYPE_BINARY_BLOB;
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsBsob( void ) const
{GUCEF_TRACE;

    return m_variantData.containedType == GUCEF_DATATYPE_BINARY_BSOB;
}

/*-------------------------------------------------------------------------*/

bool
CVariant::UsesDynamicMemory( UInt8 typeId )
{GUCEF_TRACE;

    switch ( typeId )
    {
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING:
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING:
        case GUCEF_DATATYPE_ASCII_STRING:
        case GUCEF_DATATYPE_UTF8_STRING:
        case GUCEF_DATATYPE_UTF16_LE_STRING:
        case GUCEF_DATATYPE_UTF16_BE_STRING:
        case GUCEF_DATATYPE_UTF32_LE_STRING:
        case GUCEF_DATATYPE_UTF32_BE_STRING:
        case GUCEF_DATATYPE_BINARY_BLOB:
        case GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING:
        case GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING:
        case GUCEF_DATATYPE_INT64T2_LE_FRACTION:
        case GUCEF_DATATYPE_INT64T2_BE_FRACTION:
        case GUCEF_DATATYPE_UINT64T2_LE_FRACTION:
        case GUCEF_DATATYPE_UINT64T2_BE_FRACTION:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CVariant::UsesDynamicMemory( void ) const
{GUCEF_TRACE;

    return UsesDynamicMemory( m_variantData.containedType );
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsDynamicMemoryLinked( void ) const
{GUCEF_TRACE;

    if ( UsesDynamicMemory() )
    {
        return 0 != m_variantData.union_data.heap_data.heap_data_is_linked;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVariant::OwnsDynamicMemory( void ) const
{GUCEF_TRACE;

    if ( UsesDynamicMemory() )
    {
        return 0 == m_variantData.union_data.heap_data.heap_data_is_linked;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void*
CVariant::PreAllocateDynamicMemory( UInt32 byteSize, UInt8 varType )
{GUCEF_TRACE;

    Clear();

    if ( UsesDynamicMemory( varType ) )
    {
        if ( GUCEF_NULL != HeapReserve( byteSize, true ) )
        {
            m_variantData.containedType = varType;
            memset( m_variantData.union_data.heap_data.union_data.void_heap_data, 0, byteSize );
            return m_variantData.union_data.heap_data.union_data.void_heap_data;
        }
    }

    // This should not be used in a context where the type is not one that requires dynamic memory
    // adjust your code flow accordingly
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsInitialized( void ) const
{GUCEF_TRACE;

    return GUCEF_DATATYPE_UNKNOWN != m_variantData.containedType;
}

/*-------------------------------------------------------------------------*/

bool
CVariant::IsNULLOrEmpty( void ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        // Specifically for strings we check for 0 length or null terminator as well
        case GUCEF_DATATYPE_ASCII_STRING:
        case GUCEF_DATATYPE_UTF8_STRING:
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING:
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING:
        case GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING:
        case GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING:
        {
            return 0 == m_variantData.union_data.heap_data.heap_data_size ||
                   ( 1 == m_variantData.union_data.heap_data.heap_data_size && '\0' == *( (const char*) m_variantData.union_data.heap_data.union_data.char_heap_data ) );
        }
        case GUCEF_DATATYPE_UTF16_LE_STRING:
        case GUCEF_DATATYPE_UTF16_BE_STRING:
        {
            return 0 == m_variantData.union_data.heap_data.heap_data_size ||
                   ( 2 == m_variantData.union_data.heap_data.heap_data_size && 0 == *( (const UInt16*) m_variantData.union_data.heap_data.union_data.char_heap_data ) );
        }
        case GUCEF_DATATYPE_UTF32_STRING:
        {
            return 0 == m_variantData.union_data.heap_data.heap_data_size ||
                   ( 4 == m_variantData.union_data.heap_data.heap_data_size && 0 == *( (const UInt32*) m_variantData.union_data.heap_data.union_data.char_heap_data ) );
        }
        case GUCEF_DATATYPE_INT64T2_LE_FRACTION:
        case GUCEF_DATATYPE_INT64T2_BE_FRACTION:
        case GUCEF_DATATYPE_UINT64T2_LE_FRACTION:
        case GUCEF_DATATYPE_UINT64T2_BE_FRACTION:
        case GUCEF_DATATYPE_BINARY_BLOB:
        {
            return 0 == m_variantData.union_data.heap_data.heap_data_size;
        }
        case GUCEF_DATATYPE_CONST_DATA_MEMORY_ADDRESS:
        case GUCEF_DATATYPE_DATA_MEMORY_ADDRESS:
        {
            return GUCEF_NULL == m_variantData.union_data.memory_address_data.const_objPtr;
        }
        case GUCEF_DATATYPE_FUNCTION_MEMORY_ADDRESS:
        {
            return GUCEF_NULL == m_variantData.union_data.memory_address_data.funcPtr;
        }
        case GUCEF_DATATYPE_NULL:
        case GUCEF_DATATYPE_NIL:
        {
            return true;
        }
        case GUCEF_DATATYPE_UNKNOWN: return true;
        default: return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CVariant::OverrideTypeId( UInt8 typeId )
{GUCEF_TRACE;

    m_variantData.containedType = typeId;
}

/*-------------------------------------------------------------------------*/

UInt8
CVariant::GetTypeId( void ) const
{GUCEF_TRACE;

    return m_variantData.containedType;
}

/*-------------------------------------------------------------------------*/

const char*
CVariant::GetTypeNameC( void ) const
{GUCEF_TRACE;

    return TypeNameForGucefTypeId( m_variantData.containedType );
}

/*-------------------------------------------------------------------------*/

CString
CVariant::GetTypeName( void ) const
{GUCEF_TRACE;

    return GetTypeNameC();
}

/*-------------------------------------------------------------------------*/

UInt8
CVariant::TypeIdForTypeName( const char* typeName )
{GUCEF_TRACE;

    return GucefTypeIdForTypeName( typeName );
}

/*-------------------------------------------------------------------------*/

UInt8
CVariant::TypeIdForTypeName( const CString& typeName )
{GUCEF_TRACE;

    return GucefTypeIdForTypeName( typeName.C_String() );
}
/*-------------------------------------------------------------------------*/

const char*
CVariant::TypeNameForTypeIdC( UInt8 typeId )
{GUCEF_TRACE;

    return TypeNameForGucefTypeId( typeId );
}

/*-------------------------------------------------------------------------*/

CString
CVariant::TypeNameForTypeId( UInt8 typeId )
{GUCEF_TRACE;

    return TypeNameForGucefTypeId( typeId );
}

/*-------------------------------------------------------------------------*/

bool
CVariant::AsBool( bool defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_BOOLEAN_INT32:
            return 0 != m_variantData.union_data.int32_data;

        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING:
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING:
        case GUCEF_DATATYPE_ASCII_STRING:
        case GUCEF_DATATYPE_UTF8_STRING:
        {
            if ( resolveVarsIfApplicable )
                return StringToBool( AsString( CString::Empty, resolveVarsIfApplicable ), defaultIfNeeded );
            else
                return StringToBool( AsString(), defaultIfNeeded );
        }

        case GUCEF_DATATYPE_CONST_DATA_MEMORY_ADDRESS:
        case GUCEF_DATATYPE_DATA_MEMORY_ADDRESS:
        {
            return GUCEF_NULL != m_variantData.union_data.memory_address_data.const_objPtr;
        }
        case GUCEF_DATATYPE_FUNCTION_MEMORY_ADDRESS:
        {
            return GUCEF_NULL != m_variantData.union_data.memory_address_data.funcPtr;
        }

        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
        {
            CDecimal value;
            if ( DecimalStatusIsError( GetDecimal( value ) ) )
                return defaultIfNeeded;
            return !value.IsZero();
        }

        default:
        {
            if ( IsInteger() )
                return 0 != AsUInt64();
            if ( IsFloat() || IsFraction() )
                return std::fabs( AsFloat64() - 0.0 ) < GUCEF_FLOAT64_COMPARISON_EPSILON;

            return defaultIfNeeded;
        }
    }
}

/*-------------------------------------------------------------------------*/

Int8
CVariant::AsInt8( Int8 defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT8: return m_variantData.union_data.int8_data;
        case GUCEF_DATATYPE_UINT8: return (Int8) m_variantData.union_data.uint8_data;
        case GUCEF_DATATYPE_INT16: return (Int8) m_variantData.union_data.int16_data;
        case GUCEF_DATATYPE_UINT16: return (Int8) m_variantData.union_data.uint16_data;
        case GUCEF_DATATYPE_INT32: return (Int8) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_UINT32: return (Int8) m_variantData.union_data.uint32_data;
        case GUCEF_DATATYPE_INT64: return (Int8) m_variantData.union_data.int64_data;
        case GUCEF_DATATYPE_UINT64: return (Int8) m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_FLOAT32: return (Int8) m_variantData.union_data.float32_data;
        case GUCEF_DATATYPE_FLOAT64: return (Int8) m_variantData.union_data.float64_data;
        case GUCEF_DATATYPE_BOOLEAN_INT32: return (Int8) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING: return (Int8) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1 : 0 ) : AsBool() ? 1 : 0 );
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING: return (Int8) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1 : 0 ) : AsBool() ? 1 : 0 );
        case GUCEF_DATATYPE_ASCII_STRING: return (Int8) ( resolveVarsIfApplicable ? StringToInt8( AsString(), defaultIfNeeded ) : StringToInt8( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_UTF8_STRING: return (Int8) ( resolveVarsIfApplicable ? StringToInt8( AsString(), defaultIfNeeded ) : StringToInt8( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_INT32T2_FRACTION: return (Int8) AsFloat64();
        case GUCEF_DATATYPE_UINT32T2_FRACTION: return (Int8) AsFloat64();
        case GUCEF_DATATYPE_INT64T2_FRACTION: return (Int8) AsFloat64();
        case GUCEF_DATATYPE_UINT64T2_FRACTION: return (Int8) AsFloat64();
        case GUCEF_DATATYPE_INT64_INT32_FRACTION: return (Int8) AsFloat64();
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION: return (Int8) AsFloat64();
        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
        {
            Int64 value = 0;
            if ( DecimalVariantAsTruncatedInt64( *this, value ) )
            {
                if ( (Int64) (Int8) value == value ) return (Int8) value;
            }
            return defaultIfNeeded;
        }
        default: return defaultIfNeeded;
    }
}

/*-------------------------------------------------------------------------*/

UInt8
CVariant::AsUInt8( UInt8 defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT8: return (UInt8) m_variantData.union_data.int8_data;
        case GUCEF_DATATYPE_UINT8: return m_variantData.union_data.uint8_data;
        case GUCEF_DATATYPE_INT16: return (UInt8) m_variantData.union_data.int16_data;
        case GUCEF_DATATYPE_UINT16: return (UInt8) m_variantData.union_data.uint16_data;
        case GUCEF_DATATYPE_INT32: return (UInt8) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_UINT32: return (UInt8) m_variantData.union_data.uint32_data;
        case GUCEF_DATATYPE_INT64: return (UInt8) m_variantData.union_data.int64_data;
        case GUCEF_DATATYPE_UINT64: return (UInt8) m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_FLOAT32: return (UInt8) m_variantData.union_data.float32_data;
        case GUCEF_DATATYPE_FLOAT64: return (UInt8) m_variantData.union_data.float64_data;
        case GUCEF_DATATYPE_BOOLEAN_INT32: return (UInt8) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING: return (UInt8) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1 : 0 ) : AsBool() ? 1 : 0 );
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING: return (UInt8) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1 : 0 ) : AsBool() ? 1 : 0 );
        case GUCEF_DATATYPE_ASCII_STRING: return (UInt8) ( resolveVarsIfApplicable ? StringToUInt8( AsString(), defaultIfNeeded ) : StringToUInt8( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_UTF8_STRING: return (UInt8) ( resolveVarsIfApplicable ? StringToUInt8( AsString(), defaultIfNeeded ) : StringToUInt8( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_INT32T2_FRACTION: return (UInt8) AsFloat64();
        case GUCEF_DATATYPE_UINT32T2_FRACTION: return (UInt8) AsFloat64();
        case GUCEF_DATATYPE_INT64T2_FRACTION: return (UInt8) AsFloat64();
        case GUCEF_DATATYPE_UINT64T2_FRACTION: return (UInt8) AsFloat64();
        case GUCEF_DATATYPE_INT64_INT32_FRACTION: return (UInt8) AsFloat64();
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION: return (UInt8) AsFloat64();
        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
        {
            Int64 value = 0;
            if ( DecimalVariantAsTruncatedInt64( *this, value ) )
            {
                if ( value >= 0 && (Int64) (UInt8) value == value ) return (UInt8) value;
            }
            return defaultIfNeeded;
        }
        default: return defaultIfNeeded;
    }
}

/*-------------------------------------------------------------------------*/

Int16
CVariant::AsInt16( Int16 defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT8: return (Int16) m_variantData.union_data.int8_data;
        case GUCEF_DATATYPE_UINT8: return (Int16) m_variantData.union_data.uint8_data;
        case GUCEF_DATATYPE_INT16: return m_variantData.union_data.int16_data;
        case GUCEF_DATATYPE_UINT16: return (Int16) m_variantData.union_data.uint16_data;
        case GUCEF_DATATYPE_INT32: return (Int16) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_UINT32: return (Int16) m_variantData.union_data.uint32_data;
        case GUCEF_DATATYPE_INT64: return (Int16) m_variantData.union_data.int64_data;
        case GUCEF_DATATYPE_UINT64: return (Int16) m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_FLOAT32: return (Int16) m_variantData.union_data.float32_data;
        case GUCEF_DATATYPE_FLOAT64: return (Int16) m_variantData.union_data.float64_data;
        case GUCEF_DATATYPE_BOOLEAN_INT32: return (Int16) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING: return (Int16) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1 : 0 ) : AsBool() ? 1 : 0 );
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING: return (Int16) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1 : 0 ) : AsBool() ? 1 : 0 );
        case GUCEF_DATATYPE_ASCII_STRING: return (Int16) ( resolveVarsIfApplicable ? StringToInt16( AsString(), defaultIfNeeded ) : StringToInt16( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_UTF8_STRING: return (Int16) ( resolveVarsIfApplicable ? StringToInt16( AsString(), defaultIfNeeded ) : StringToInt16( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_INT32T2_FRACTION: return (Int16) AsFloat64();
        case GUCEF_DATATYPE_UINT32T2_FRACTION: return (Int16) AsFloat64();
        case GUCEF_DATATYPE_INT64T2_FRACTION: return (Int16) AsFloat64();
        case GUCEF_DATATYPE_UINT64T2_FRACTION: return (Int16) AsFloat64();
        case GUCEF_DATATYPE_INT64_INT32_FRACTION: return (Int16) AsFloat64();
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION: return (Int16) AsFloat64();
        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
        {
            Int64 value = 0;
            if ( DecimalVariantAsTruncatedInt64( *this, value ) )
            {
                if ( (Int64) (Int16) value == value ) return (Int16) value;
            }
            return defaultIfNeeded;
        }
        default: return defaultIfNeeded;
    }
}

/*-------------------------------------------------------------------------*/

UInt16
CVariant::AsUInt16( UInt16 defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT8: return (UInt16) m_variantData.union_data.int8_data;
        case GUCEF_DATATYPE_UINT8: return (UInt16) m_variantData.union_data.uint8_data;
        case GUCEF_DATATYPE_INT16: return (UInt16) m_variantData.union_data.int16_data;
        case GUCEF_DATATYPE_UINT16: return m_variantData.union_data.uint16_data;
        case GUCEF_DATATYPE_INT32: return (UInt16) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_UINT32: return (UInt16) m_variantData.union_data.uint32_data;
        case GUCEF_DATATYPE_INT64: return (UInt16) m_variantData.union_data.int64_data;
        case GUCEF_DATATYPE_UINT64: return (UInt16) m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_FLOAT32: return (UInt16) m_variantData.union_data.float32_data;
        case GUCEF_DATATYPE_FLOAT64: return (UInt16) m_variantData.union_data.float64_data;
        case GUCEF_DATATYPE_BOOLEAN_INT32: return (UInt16) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING: return (UInt16) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1 : 0 ) : AsBool() ? 1 : 0 );
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING: return (UInt16) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1 : 0 ) : AsBool() ? 1 : 0 );
        case GUCEF_DATATYPE_ASCII_STRING: return (UInt16) ( resolveVarsIfApplicable ? StringToUInt16( AsString(), defaultIfNeeded ) : StringToUInt16( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_UTF8_STRING: return (UInt16) ( resolveVarsIfApplicable ? StringToUInt16( AsString(), defaultIfNeeded ) : StringToUInt16( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_INT32T2_FRACTION: return (UInt16) AsFloat64();
        case GUCEF_DATATYPE_UINT32T2_FRACTION: return (UInt16) AsFloat64();
        case GUCEF_DATATYPE_INT64T2_FRACTION: return (UInt16) AsFloat64();
        case GUCEF_DATATYPE_UINT64T2_FRACTION: return (UInt16) AsFloat64();
        case GUCEF_DATATYPE_INT64_INT32_FRACTION: return (UInt16) AsFloat64();
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION: return (UInt16) AsFloat64();
        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
        {
            Int64 value = 0;
            if ( DecimalVariantAsTruncatedInt64( *this, value ) )
            {
                if ( value >= 0 && (Int64) (UInt16) value == value ) return (UInt16) value;
            }
            return defaultIfNeeded;
        }
        default: return defaultIfNeeded;
    }
}

/*-------------------------------------------------------------------------*/

Int32
CVariant::AsInt32( Int32 defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT8: return (Int32) m_variantData.union_data.int8_data;
        case GUCEF_DATATYPE_UINT8: return (Int32) m_variantData.union_data.uint8_data;
        case GUCEF_DATATYPE_INT16: return (Int32) m_variantData.union_data.int16_data;
        case GUCEF_DATATYPE_UINT16: return (Int32) m_variantData.union_data.uint16_data;
        case GUCEF_DATATYPE_INT32: return m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_UINT32: return (Int32) m_variantData.union_data.uint32_data;
        case GUCEF_DATATYPE_INT64: return (Int32) m_variantData.union_data.int64_data;
        case GUCEF_DATATYPE_UINT64: return (Int32) m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_FLOAT32: return (Int32) m_variantData.union_data.float32_data;
        case GUCEF_DATATYPE_FLOAT64: return (Int32) m_variantData.union_data.float64_data;
        case GUCEF_DATATYPE_BOOLEAN_INT32: return (Int32) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH: return DaysFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING: return (Int32) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1 : 0 ) : AsBool() ? 1 : 0 );
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING: return (Int32) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1 : 0 ) : AsBool() ? 1 : 0 );
        case GUCEF_DATATYPE_ASCII_STRING: return (Int32) ( resolveVarsIfApplicable ? StringToInt32( AsString(), defaultIfNeeded ) : StringToInt32( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_UTF8_STRING: return (Int32) ( resolveVarsIfApplicable ? StringToInt32( AsString(), defaultIfNeeded ) : StringToInt32( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_INT32T2_FRACTION: return (Int32) AsFloat64();
        case GUCEF_DATATYPE_UINT32T2_FRACTION: return (Int32) AsFloat64();
        case GUCEF_DATATYPE_INT64T2_FRACTION: return (Int32) AsFloat64();
        case GUCEF_DATATYPE_UINT64T2_FRACTION: return (Int32) AsFloat64();
        case GUCEF_DATATYPE_INT64_INT32_FRACTION: return (Int32) AsFloat64();
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION: return (Int32) AsFloat64();
        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
        {
            Int64 value = 0;
            if ( DecimalVariantAsTruncatedInt64( *this, value ) )
            {
                if ( (Int64) (Int32) value == value ) return (Int32) value;
            }
            return defaultIfNeeded;
        }
        default: return defaultIfNeeded;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CVariant::AsUInt32( UInt32 defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT8: return (UInt32) m_variantData.union_data.int8_data;
        case GUCEF_DATATYPE_UINT8: return (UInt32) m_variantData.union_data.uint8_data;
        case GUCEF_DATATYPE_INT16: return (UInt32) m_variantData.union_data.int16_data;
        case GUCEF_DATATYPE_UINT16: return (UInt32) m_variantData.union_data.uint16_data;
        case GUCEF_DATATYPE_INT32: return (UInt32) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_UINT32: return m_variantData.union_data.uint32_data;
        case GUCEF_DATATYPE_INT64: return (UInt32) m_variantData.union_data.int64_data;
        case GUCEF_DATATYPE_UINT64: return (UInt32) m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_FLOAT32: return (UInt32) m_variantData.union_data.float32_data;
        case GUCEF_DATATYPE_FLOAT64: return (UInt32) m_variantData.union_data.float64_data;
        case GUCEF_DATATYPE_BOOLEAN_INT32: return (UInt32) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING: return (UInt32) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1 : 0 ) : AsBool() ? 1 : 0 );
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING: return (UInt32) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1 : 0 ) : AsBool() ? 1 : 0 );
        case GUCEF_DATATYPE_ASCII_STRING: return (UInt32) ( resolveVarsIfApplicable ? StringToUInt32( AsString(), defaultIfNeeded ) : StringToUInt32( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_UTF8_STRING: return (UInt32) ( resolveVarsIfApplicable ? StringToUInt32( AsString(), defaultIfNeeded ) : StringToUInt32( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_INT32T2_FRACTION: return (UInt32) AsFloat64();
        case GUCEF_DATATYPE_UINT32T2_FRACTION: return (UInt32) AsFloat64();
        case GUCEF_DATATYPE_INT64T2_FRACTION: return (UInt32) AsFloat64();
        case GUCEF_DATATYPE_UINT64T2_FRACTION: return (UInt32) AsFloat64();
        case GUCEF_DATATYPE_INT64_INT32_FRACTION: return (UInt32) AsFloat64();
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION: return (UInt32) AsFloat64();
        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
        {
            Int64 value = 0;
            if ( DecimalVariantAsTruncatedInt64( *this, value ) )
            {
                if ( value >= 0 && (Int64) (UInt32) value == value ) return (UInt32) value;
            }
            return defaultIfNeeded;
        }

        #if ( GUCEF_BITNESS == GUCEF_BITNESS_32 )
        case GUCEF_DATATYPE_DATA_MEMORY_ADDRESS: return (UInt32) m_variantData.union_data.memory_address_data.objPtr;
        case GUCEF_DATATYPE_FUNCTION_MEMORY_ADDRESS: return (UInt32) m_variantData.union_data.memory_address_data.funcPtr;
        #endif

        default: return defaultIfNeeded;
    }
}

/*-------------------------------------------------------------------------*/

Int64
CVariant::AsInt64( Int64 defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT8: return (Int64) m_variantData.union_data.int8_data;
        case GUCEF_DATATYPE_UINT8: return (Int64) m_variantData.union_data.uint8_data;
        case GUCEF_DATATYPE_INT16: return (Int64) m_variantData.union_data.int16_data;
        case GUCEF_DATATYPE_UINT16: return (Int64) m_variantData.union_data.uint16_data;
        case GUCEF_DATATYPE_INT32: return (Int64) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_UINT32: return (Int64) m_variantData.union_data.uint32_data;
        case GUCEF_DATATYPE_INT64: return m_variantData.union_data.int64_data;
        case GUCEF_DATATYPE_UINT64: return (Int64) m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_FLOAT32: return (Int64) m_variantData.union_data.float32_data;
        case GUCEF_DATATYPE_FLOAT64: return (Int64) m_variantData.union_data.float64_data;
        case GUCEF_DATATYPE_BOOLEAN_INT32: return (Int64) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH: return SignedTicksFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH: return SignedTicksFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH: return SignedTicksFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH: return SignedTicksFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH: return SignedTicksFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH: return SignedTicksFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH: return MicrosFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH: return (Int64) DaysFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING: return (Int64) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1 : 0 ) : AsBool() ? 1 : 0 );
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING: return (Int64) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1 : 0 ) : AsBool() ? 1 : 0 );
        case GUCEF_DATATYPE_ASCII_STRING: return (Int64) ( resolveVarsIfApplicable ? StringToInt64( AsString(), defaultIfNeeded ) : StringToInt64( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_UTF8_STRING: return (Int64) ( resolveVarsIfApplicable ? StringToInt64( AsString(), defaultIfNeeded ) : StringToInt64( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_INT32T2_FRACTION: return (Int64) AsFloat64();
        case GUCEF_DATATYPE_UINT32T2_FRACTION: return (Int64) AsFloat64();
        case GUCEF_DATATYPE_INT64T2_FRACTION: return (Int64) AsFloat64();
        case GUCEF_DATATYPE_UINT64T2_FRACTION: return (Int64) AsFloat64();
        case GUCEF_DATATYPE_INT64_INT32_FRACTION: return (Int64) AsFloat64();
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION: return (Int64) AsFloat64();
        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
        {
            Int64 value = 0;
            if ( DecimalVariantAsTruncatedInt64( *this, value ) )
            {
                return value;
            }
            return defaultIfNeeded;
        }
        default: return defaultIfNeeded;
    }
}

/*-------------------------------------------------------------------------*/

UInt64
CVariant::AsUInt64( UInt64 defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT8: return (UInt64) m_variantData.union_data.int8_data;
        case GUCEF_DATATYPE_UINT8: return (UInt64) m_variantData.union_data.uint8_data;
        case GUCEF_DATATYPE_INT16: return (UInt64) m_variantData.union_data.int16_data;
        case GUCEF_DATATYPE_UINT16: return (UInt64) m_variantData.union_data.uint16_data;
        case GUCEF_DATATYPE_INT32: return (UInt64) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_UINT32: return (UInt64) m_variantData.union_data.uint32_data;
        case GUCEF_DATATYPE_INT64: return (UInt64) m_variantData.union_data.int64_data;
        case GUCEF_DATATYPE_UINT64: return m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_FLOAT32: return (UInt64) m_variantData.union_data.float32_data;
        case GUCEF_DATATYPE_FLOAT64: return (UInt64) m_variantData.union_data.float64_data;
        case GUCEF_DATATYPE_BOOLEAN_INT32: return (UInt64) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH: return (UInt64) SignedTicksFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH: return (UInt64) SignedTicksFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH: return (UInt64) SignedTicksFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH: return (UInt64) SignedTicksFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH: return (UInt64) SignedTicksFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH: return (UInt64) SignedTicksFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH: return (UInt64) MicrosFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH: return (UInt64) (Int64) DaysFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING: return (UInt64) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1 : 0 ) : AsBool() ? 1 : 0 );
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING: return (UInt64) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1 : 0 ) : AsBool() ? 1 : 0 );
        case GUCEF_DATATYPE_ASCII_STRING: return (UInt64) ( resolveVarsIfApplicable ? StringToUInt64( AsString(), defaultIfNeeded ) : StringToUInt64( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_UTF8_STRING: return (UInt64) ( resolveVarsIfApplicable ? StringToUInt64( AsString(), defaultIfNeeded ) : StringToUInt64( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_INT32T2_FRACTION: return (UInt64) AsFloat64();
        case GUCEF_DATATYPE_UINT32T2_FRACTION: return (UInt64) AsFloat64();
        case GUCEF_DATATYPE_INT64T2_FRACTION: return (UInt64) AsFloat64();
        case GUCEF_DATATYPE_UINT64T2_FRACTION: return (UInt64) AsFloat64();
        case GUCEF_DATATYPE_INT64_INT32_FRACTION: return (UInt64) AsFloat64();
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION: return (UInt64) AsFloat64();
        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
        {
            Int64 value = 0;
            if ( DecimalVariantAsTruncatedInt64( *this, value ) )
            {
                if ( value >= 0 ) return (UInt64) value;
            }
            return defaultIfNeeded;
        }

        #if ( GUCEF_BITNESS == GUCEF_BITNESS_64 )
        case GUCEF_DATATYPE_DATA_MEMORY_ADDRESS: return (UInt64) m_variantData.union_data.memory_address_data.objPtr;
        case GUCEF_DATATYPE_FUNCTION_MEMORY_ADDRESS: return (UInt64) m_variantData.union_data.memory_address_data.funcPtr;
        #endif

        default: return defaultIfNeeded;
    }
}

/*-------------------------------------------------------------------------*/

size_t
CVariant::AsSizeT( size_t defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT8: return (size_t) m_variantData.union_data.int8_data;
        case GUCEF_DATATYPE_UINT8: return (size_t) m_variantData.union_data.uint8_data;
        case GUCEF_DATATYPE_INT16: return (size_t) m_variantData.union_data.int16_data;
        case GUCEF_DATATYPE_UINT16: return (size_t) m_variantData.union_data.uint16_data;
        case GUCEF_DATATYPE_INT32: return (size_t) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_UINT32: return (size_t) m_variantData.union_data.uint32_data;
        case GUCEF_DATATYPE_INT64: return (size_t) m_variantData.union_data.int64_data;
        case GUCEF_DATATYPE_UINT64: return (size_t) m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_FLOAT32: return (size_t) m_variantData.union_data.float32_data;
        case GUCEF_DATATYPE_FLOAT64: return (size_t) m_variantData.union_data.float64_data;
        case GUCEF_DATATYPE_BOOLEAN_INT32: return (size_t) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH: return (size_t) SignedTicksFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH: return (size_t) SignedTicksFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH: return (size_t) SignedTicksFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH: return (size_t) SignedTicksFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH: return (size_t) SignedTicksFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH: return (size_t) SignedTicksFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH: return (size_t) MicrosFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH: return (size_t) DaysFromVariantPayload( m_variantData );
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING: return (size_t) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1 : 0 ) : AsBool() ? 1 : 0 );
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING: return (size_t) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1 : 0 ) : AsBool() ? 1 : 0 );
        case GUCEF_DATATYPE_ASCII_STRING: return (size_t) ( resolveVarsIfApplicable ? StringToUInt64( AsString(), defaultIfNeeded ) : StringToUInt64( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_UTF8_STRING: return (size_t) ( resolveVarsIfApplicable ? StringToUInt64( AsString(), defaultIfNeeded ) : StringToUInt64( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_INT32T2_FRACTION: return (size_t) AsFloat64();
        case GUCEF_DATATYPE_UINT32T2_FRACTION: return (size_t) AsFloat64();
        case GUCEF_DATATYPE_INT64T2_FRACTION: return (size_t) AsFloat64();
        case GUCEF_DATATYPE_UINT64T2_FRACTION: return (size_t) AsFloat64();
        case GUCEF_DATATYPE_INT64_INT32_FRACTION: return (size_t) AsFloat64();
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION: return (size_t) AsFloat64();
        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
        {
            Int64 value = 0;
            if ( DecimalVariantAsTruncatedInt64( *this, value ) )
            {
                if ( value >= 0 && (UInt64) (size_t) value == (UInt64) value ) return (size_t) value;
            }
            return defaultIfNeeded;
        }
        default: return defaultIfNeeded;
    }
}

/*-------------------------------------------------------------------------*/

Float32
CVariant::AsFloat32( Float32 defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT8: return (Float32) m_variantData.union_data.int8_data;
        case GUCEF_DATATYPE_UINT8: return (Float32) m_variantData.union_data.uint8_data;
        case GUCEF_DATATYPE_INT16: return (Float32) m_variantData.union_data.int16_data;
        case GUCEF_DATATYPE_UINT16: return (Float32) m_variantData.union_data.uint16_data;
        case GUCEF_DATATYPE_INT32: return (Float32) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_UINT32: return (Float32) m_variantData.union_data.uint32_data;
        case GUCEF_DATATYPE_INT64: return  (Float32) m_variantData.union_data.int64_data;
        case GUCEF_DATATYPE_UINT64: return (Float32) m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_FLOAT32: return m_variantData.union_data.float32_data;
        case GUCEF_DATATYPE_FLOAT64: return (Float32) m_variantData.union_data.float64_data;
        case GUCEF_DATATYPE_BOOLEAN_INT32: return (Float32) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING: return (Float32) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1.0f : 0.0f ) : AsBool() ? 1.0f : 0.0f );
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING: return (Float32) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1.0f : 0.0f ) : AsBool() ? 1.0f : 0.0f );
        case GUCEF_DATATYPE_ASCII_STRING: return (Float32) ( resolveVarsIfApplicable ? StringToFloat( AsString(), defaultIfNeeded ) : StringToFloat( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_UTF8_STRING: return (Float32) ( resolveVarsIfApplicable ? StringToFloat( AsString(), defaultIfNeeded ) : StringToFloat( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_INT32T2_FRACTION: return ( (Float32) TInt32Fraction( m_variantData.union_data.fraction_data.union_data.int32t2_data ).ToFloat32() );
        case GUCEF_DATATYPE_UINT32T2_FRACTION: return ( (Float32) TUInt32Fraction( m_variantData.union_data.fraction_data.union_data.uint32t2_data ).ToFloat32() );
        case GUCEF_DATATYPE_INT64_INT32_FRACTION: return ( (Float32) TInt64Int32Fraction( m_variantData.union_data.fraction_data.union_data.int64_int32_data ).ToFloat32() );
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION: return ( (Float32) TUInt64UInt32Fraction( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data ).ToFloat32() );
        case GUCEF_DATATYPE_INT64T2_FRACTION:
        {
            if ( GUCEF_NULL != m_variantData.union_data.heap_data.union_data.void_heap_data    &&
                 sizeof(TInt64T2Fraction) <= m_variantData.union_data.heap_data.heap_data_size )
            {
                return TInt64Fraction( *m_variantData.union_data.heap_data.union_data.fraction_int64t2_data ).ToFloat32();
            }
            else
            {
                return defaultIfNeeded;
            }
        }
        case GUCEF_DATATYPE_UINT64T2_FRACTION:
        {
            if ( GUCEF_NULL != m_variantData.union_data.heap_data.union_data.void_heap_data    &&
                 sizeof(TUInt64T2Fraction) <= m_variantData.union_data.heap_data.heap_data_size )
            {
                return TUInt64Fraction( *m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data ).ToFloat32();
            }
            else
            {
                return defaultIfNeeded;
            }
        }
        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
        {
            CDecimal value;
            if ( DecimalStatusIsError( GetDecimal( value ) ) )
                return defaultIfNeeded;
            return (Float32) value.ToFloat64();
        }
        default:
        {
            return defaultIfNeeded;
        }
    }
}

/*-------------------------------------------------------------------------*/

Float64
CVariant::AsFloat64( Float64 defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT8: return (Float64) m_variantData.union_data.int8_data;
        case GUCEF_DATATYPE_UINT8: return (Float64) m_variantData.union_data.uint8_data;
        case GUCEF_DATATYPE_INT16: return (Float64) m_variantData.union_data.int16_data;
        case GUCEF_DATATYPE_UINT16: return (Float64) m_variantData.union_data.uint16_data;
        case GUCEF_DATATYPE_INT32: return (Float64) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_UINT32: return (Float64) m_variantData.union_data.uint32_data;
        case GUCEF_DATATYPE_INT64: return  (Float64) m_variantData.union_data.int64_data;
        case GUCEF_DATATYPE_UINT64: return (Float64) m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_FLOAT32: return (Float64) m_variantData.union_data.float32_data;
        case GUCEF_DATATYPE_FLOAT64: return m_variantData.union_data.float64_data;
        case GUCEF_DATATYPE_BOOLEAN_INT32: return (Float64) m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING: return (Float64) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1.0l : 0.0l ) : AsBool() ? 1.0l : 0.0l );
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING: return (Float64) ( resolveVarsIfApplicable ? ( StringToBool( ResolveVars( AsString() ) ) ? 1.0l : 0.0l ) : AsBool() ? 1.0l : 0.0l );
        case GUCEF_DATATYPE_ASCII_STRING: return (Float64) ( resolveVarsIfApplicable ? StringToDouble( AsString(), defaultIfNeeded ) : StringToDouble( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_UTF8_STRING: return (Float64) ( resolveVarsIfApplicable ? StringToDouble( AsString(), defaultIfNeeded ) : StringToDouble( AsString(), defaultIfNeeded ) );
        case GUCEF_DATATYPE_INT32T2_FRACTION: return ( (Float64) m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator ) / ( (Float64) m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator );
        case GUCEF_DATATYPE_UINT32T2_FRACTION: return ( (Float64) m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator ) / ( (Float64) m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator );
        case GUCEF_DATATYPE_INT64T2_FRACTION:
        {
            if ( GUCEF_NULL != m_variantData.union_data.heap_data.union_data.void_heap_data    &&
                 sizeof(TInt64T2Fraction) <= m_variantData.union_data.heap_data.heap_data_size )
            {
                return TInt64Fraction( *m_variantData.union_data.heap_data.union_data.fraction_int64t2_data ).ToFloat64();
            }
            else
            {
                return defaultIfNeeded;
            }
        }
        case GUCEF_DATATYPE_UINT64T2_FRACTION:
        {
            if ( GUCEF_NULL != m_variantData.union_data.heap_data.union_data.void_heap_data    &&
                 sizeof(TUInt64T2Fraction) <= m_variantData.union_data.heap_data.heap_data_size )
            {
                return TUInt64Fraction( *m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data ).ToFloat64();
            }
            else
            {
                return defaultIfNeeded;
            }
        }
        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
        {
            CDecimal value;
            if ( DecimalStatusIsError( GetDecimal( value ) ) )
                return defaultIfNeeded;
            return value.ToFloat64();
        }
        default: return defaultIfNeeded;
    }
}

/*-------------------------------------------------------------------------*/

const char*
CVariant::AsCharPtr( const char* defaultIfNeeded ) const
{GUCEF_TRACE;

    return static_cast< const char* >( AsVoidPtr(  defaultIfNeeded ) );
}

/*-------------------------------------------------------------------------*/

char
CVariant::AsChar( char defaultIfNeeded ) const
{GUCEF_TRACE;

    const char* charPtr = AsCharPtr( GUCEF_NULL );
    if ( GUCEF_NULL != charPtr )
        return *charPtr;
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

TDefaultFuncPtr
CVariant::AsDefaultFuncPtr( TDefaultFuncPtr defaultIfNeeded ) const
{GUCEF_TRACE;

    if ( m_variantData.containedType == GUCEF_DATATYPE_FUNCTION_MEMORY_ADDRESS )
        return m_variantData.union_data.memory_address_data.funcPtr;
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

const void*
CVariant::AsVoidPtr( const void* defaultIfNeeded ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_VOID: return GUCEF_NULL;
        case GUCEF_DATATYPE_INT8: return &m_variantData.union_data.int8_data;
        case GUCEF_DATATYPE_UINT8: return &m_variantData.union_data.uint8_data;
        case GUCEF_DATATYPE_INT16: return &m_variantData.union_data.int16_data;
        case GUCEF_DATATYPE_UINT16: return &m_variantData.union_data.uint16_data;
        case GUCEF_DATATYPE_INT32: return &m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_UINT32: return &m_variantData.union_data.uint32_data;
        case GUCEF_DATATYPE_INT64: return &m_variantData.union_data.int64_data;
        case GUCEF_DATATYPE_UINT64: return &m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_FLOAT32: return &m_variantData.union_data.float32_data;
        case GUCEF_DATATYPE_FLOAT64: return &m_variantData.union_data.float64_data;
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH: return &m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH: return &m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH: return &m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH: return &m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH: return &m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH: return &m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH: return &m_variantData.union_data.int64_data;
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH: return &m_variantData.union_data.int64_data;
        case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH: return &m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH: return &m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_BOOLEAN_INT32: return &m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_BINARY_BSOB: return m_variantData.union_data.bsob_data;
        case GUCEF_DATATYPE_INT32T2_FRACTION: return &m_variantData.union_data.fraction_data.union_data.int32t2_data;
        case GUCEF_DATATYPE_UINT32T2_FRACTION: return &m_variantData.union_data.fraction_data.union_data.uint32t2_data;
        case GUCEF_DATATYPE_INT64_INT32_FRACTION: return &m_variantData.union_data.fraction_data.union_data.int64_int32_data;
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION: return &m_variantData.union_data.fraction_data.union_data.uint64_uint32_data;
        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL: return &m_variantData.union_data.decimal_data;
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL: return &m_variantData.union_data.decimal_data;
        case GUCEF_DATATYPE_CONST_DATA_MEMORY_ADDRESS: return m_variantData.union_data.memory_address_data.objPtr;
        case GUCEF_DATATYPE_DATA_MEMORY_ADDRESS: return m_variantData.union_data.memory_address_data.objPtr;
        case GUCEF_DATATYPE_FUNCTION_MEMORY_ADDRESS: return sizeof( TDefaultFuncPtr ) == sizeof( void* ) ? reinterpret_cast< const void* >( m_variantData.union_data.memory_address_data.funcPtr ) : defaultIfNeeded;

        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING: return m_variantData.union_data.heap_data.union_data.char_heap_data;
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING: return m_variantData.union_data.heap_data.union_data.char_heap_data;
        case GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING: return m_variantData.union_data.heap_data.union_data.char_heap_data;
        case GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING: return m_variantData.union_data.heap_data.union_data.char_heap_data;
        case GUCEF_DATATYPE_ASCII_STRING: return m_variantData.union_data.heap_data.union_data.char_heap_data;
        case GUCEF_DATATYPE_UTF8_STRING: return m_variantData.union_data.heap_data.union_data.char_heap_data;
        case GUCEF_DATATYPE_UTF16_LE_STRING: return m_variantData.union_data.heap_data.union_data.void_heap_data;
        case GUCEF_DATATYPE_UTF16_BE_STRING: return m_variantData.union_data.heap_data.union_data.void_heap_data;
        case GUCEF_DATATYPE_UTF32_STRING: return m_variantData.union_data.heap_data.union_data.void_heap_data;
        case GUCEF_DATATYPE_BINARY_BLOB: return m_variantData.union_data.heap_data.union_data.void_heap_data;
        case GUCEF_DATATYPE_INT64T2_FRACTION: return m_variantData.union_data.heap_data.union_data.fraction_int64t2_data;
        case GUCEF_DATATYPE_UINT64T2_FRACTION: return m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data;

        default:
        {
            return defaultIfNeeded;
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CVariant::ByteSizeOfFixedSizeType( UInt8 varType )
{GUCEF_TRACE;

    return GucefByteSizeOfFixedSizeType( varType );
}

/*-------------------------------------------------------------------------*/

UInt32
CVariant::ByteSize( bool includeNullTerm ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT8: return sizeof m_variantData.union_data.int8_data;
        case GUCEF_DATATYPE_UINT8: return sizeof m_variantData.union_data.uint8_data;
        case GUCEF_DATATYPE_INT16: return sizeof m_variantData.union_data.int16_data;
        case GUCEF_DATATYPE_UINT16: return sizeof m_variantData.union_data.uint16_data;
        case GUCEF_DATATYPE_INT32: return sizeof m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_UINT32: return sizeof m_variantData.union_data.uint32_data;
        case GUCEF_DATATYPE_INT64: return sizeof m_variantData.union_data.int64_data;
        case GUCEF_DATATYPE_UINT64: return sizeof m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_FLOAT32: return sizeof m_variantData.union_data.float32_data;
        case GUCEF_DATATYPE_FLOAT64: return sizeof m_variantData.union_data.float64_data;
        case GUCEF_DATATYPE_BOOLEAN_INT32: return sizeof m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_BINARY_BSOB: return sizeof m_variantData.union_data.bsob_data;
        case GUCEF_DATATYPE_BINARY_BLOB: return m_variantData.union_data.heap_data.heap_data_size;
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH: return sizeof m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH: return sizeof m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH: return sizeof m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH: return sizeof m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH: return sizeof m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH: return sizeof m_variantData.union_data.uint64_data;
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH: return sizeof m_variantData.union_data.int64_data;
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH: return sizeof m_variantData.union_data.int64_data;
        case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH: return sizeof m_variantData.union_data.int32_data;
        case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH: return sizeof m_variantData.union_data.int32_data;

        case GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING:
        case GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING:
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING:
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING:
        case GUCEF_DATATYPE_ASCII_STRING:
        case GUCEF_DATATYPE_UTF8_STRING:
        {
            if ( includeNullTerm || 0 == m_variantData.union_data.heap_data.heap_data_size )
                return m_variantData.union_data.heap_data.heap_data_size;

            const char* heapPtr = static_cast< const char* >( m_variantData.union_data.heap_data.union_data.char_heap_data );
            if ( '\0' == heapPtr[ m_variantData.union_data.heap_data.heap_data_size-1 ] )
                return m_variantData.union_data.heap_data.heap_data_size-1;
            return m_variantData.union_data.heap_data.heap_data_size;
        }
        case GUCEF_DATATYPE_UTF16_LE_STRING:
        case GUCEF_DATATYPE_UTF16_BE_STRING:
        {
            if ( includeNullTerm || 1 > m_variantData.union_data.heap_data.heap_data_size )
                return m_variantData.union_data.heap_data.heap_data_size;

            const UInt16 lastCodePoint = *reinterpret_cast< const UInt16* >( m_variantData.union_data.heap_data.union_data.char_heap_data + m_variantData.union_data.heap_data.heap_data_size - 2 );
            if ( 0 == lastCodePoint )
                return m_variantData.union_data.heap_data.heap_data_size-2;
            return m_variantData.union_data.heap_data.heap_data_size;
        }
        case GUCEF_DATATYPE_UTF32_STRING:
        {
            if ( includeNullTerm || 4 > m_variantData.union_data.heap_data.heap_data_size )
                return m_variantData.union_data.heap_data.heap_data_size;

            const UInt32 lastCodePoint = *reinterpret_cast< const UInt32* >( m_variantData.union_data.heap_data.union_data.char_heap_data + m_variantData.union_data.heap_data.heap_data_size - 4 );
            if ( 0 == lastCodePoint )
                return m_variantData.union_data.heap_data.heap_data_size-4;
            return m_variantData.union_data.heap_data.heap_data_size;
        }

        case GUCEF_DATATYPE_INT32T2_FRACTION: return sizeof m_variantData.union_data.fraction_data.union_data.int32t2_data;
        case GUCEF_DATATYPE_UINT32T2_FRACTION: return sizeof m_variantData.union_data.fraction_data.union_data.uint32t2_data;
        case GUCEF_DATATYPE_INT64_INT32_FRACTION: return sizeof m_variantData.union_data.fraction_data.union_data.int64_int32_data;
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION: return sizeof m_variantData.union_data.fraction_data.union_data.uint64_uint32_data;
        case GUCEF_DATATYPE_INT64T2_FRACTION: return m_variantData.union_data.heap_data.heap_data_size;
        case GUCEF_DATATYPE_UINT64T2_FRACTION: return m_variantData.union_data.heap_data.heap_data_size;
        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL: return GUCEF_DECIMAL_PAYLOAD_SIZE;
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL: return GUCEF_DECIMAL_PAYLOAD_SIZE;

        case GUCEF_DATATYPE_CONST_DATA_MEMORY_ADDRESS: return sizeof m_variantData.union_data.memory_address_data.objPtr;
        case GUCEF_DATATYPE_DATA_MEMORY_ADDRESS: return sizeof m_variantData.union_data.memory_address_data.objPtr;
        case GUCEF_DATATYPE_FUNCTION_MEMORY_ADDRESS: return sizeof m_variantData.union_data.memory_address_data.funcPtr;

        case GUCEF_DATATYPE_NULL:
        case GUCEF_DATATYPE_NIL:
        default: return 0;
    }
}

/*-------------------------------------------------------------------------*/

CVariant::~CVariant()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( bool data )
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.int32_data = data ? 1 : 0;
    m_variantData.containedType = GUCEF_DATATYPE_BOOLEAN_INT32;
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( Int8 data )
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.int8_data = data;
    m_variantData.containedType = GUCEF_DATATYPE_INT8;
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( UInt8 data )
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.uint8_data = data;
    m_variantData.containedType = GUCEF_DATATYPE_UINT8;
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( Int16 data )
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.int16_data = data;
    m_variantData.containedType = GUCEF_DATATYPE_INT16;
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( UInt16 data )
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.uint16_data = data;
    m_variantData.containedType = GUCEF_DATATYPE_UINT16;
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( Int32 data )
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.int32_data = data;
    m_variantData.containedType = GUCEF_DATATYPE_INT32;
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( UInt32 data )
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.uint32_data = data;
    m_variantData.containedType = GUCEF_DATATYPE_UINT32;
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( Int64 data )
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.int64_data = data;
    m_variantData.containedType = GUCEF_DATATYPE_INT64;
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( UInt64 data )
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.uint64_data = data;
    m_variantData.containedType = GUCEF_DATATYPE_UINT64;
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( Float32 data )
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.float32_data = data;
    m_variantData.containedType = GUCEF_DATATYPE_FLOAT32;
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( Float64 data )
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.float64_data = data;
    m_variantData.containedType = GUCEF_DATATYPE_FLOAT64;
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( TInt32T2Fraction data )
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.fraction_data.union_data.int32t2_data = data;
    m_variantData.containedType = GUCEF_DATATYPE_INT32T2_FRACTION;
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( TUInt32T2Fraction data )
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.fraction_data.union_data.uint32t2_data = data;
    m_variantData.containedType = GUCEF_DATATYPE_UINT32T2_FRACTION;
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( const TInt64T2Fraction& data )
{GUCEF_TRACE;

    Clear();
    Set( &data, sizeof data, GUCEF_DATATYPE_INT64T2_FRACTION, false );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( const TUInt64T2Fraction& data )
{GUCEF_TRACE;

    Clear();
    Set( &data, sizeof data, GUCEF_DATATYPE_UINT64T2_FRACTION, false );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( const TInt64WInt32Fraction& data )
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.fraction_data.union_data.int64_int32_data = data;
    m_variantData.containedType = GUCEF_DATATYPE_INT64_INT32_FRACTION;
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( const TUInt64WUInt32Fraction& data )
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.fraction_data.union_data.uint64_uint32_data = data;
    m_variantData.containedType = GUCEF_DATATYPE_UINT64_UINT32_FRACTION;
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( const CDecimal& data )
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.decimal_data.unscaled = data.GetUnscaledValue();
    m_variantData.union_data.decimal_data.scale = data.GetScale();
    m_variantData.containedType = GUCEF_DATATYPE_INT64_UINT8_DECIMAL;
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( const CDynamicBuffer& data )
{GUCEF_TRACE;

    Set( data.GetConstBufferPtr(), data.GetDataSize(), GUCEF_DATATYPE_BINARY_BLOB, false );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( const CDateTime& data )
{GUCEF_TRACE;

    CString str = data.ToIso8601DateTimeString( false, true );
    Set( str.C_String(), str.ByteSize(), GUCEF_DATATYPE_DATETIME_ISO8601_STRING, false );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( const CTimestamp& data )
{GUCEF_TRACE;

    UInt64 value = data.ToNanosecondsSinceEpoch();
    Set( &value, sizeof( UInt64 ), GUCEF_DATATYPE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH, false );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( const CAsciiString& data )
{GUCEF_TRACE;

    Set( data.C_String(), data.ByteSize(), GUCEF_DATATYPE_ASCII_STRING, false );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( const CUtf8String& data )
{GUCEF_TRACE;

    Set( data.C_String(), data.ByteSize(), GUCEF_DATATYPE_UTF8_STRING, false );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( const CUtf16String& data )
{GUCEF_TRACE;

    Set( data.C_String(), data.ByteSize(), GUCEF_DATATYPE_UTF16_STRING, false );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( const CUtf32String& data )
{GUCEF_TRACE;

    Set( data.C_String(), data.ByteSize(), GUCEF_DATATYPE_UTF32_STRING, false );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( const std::string& data )
{GUCEF_TRACE;

    Set( data.c_str(), (UInt32) data.size(), GUCEF_DATATYPE_UTF8_STRING, false );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( const std::wstring& data )
{GUCEF_TRACE;

    Set( data.c_str(), (UInt32)( (data.size()+1) * sizeof(std::wstring::value_type) ), WStringVariantType(), false );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( const CVariant& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        Set( &src.m_variantData, false );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( const TVariantData& src )
{GUCEF_TRACE;

    if ( &m_variantData != &src )
    {
        Set( &src, false );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( const TDefaultFuncPtr& src )                                   
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.memory_address_data.funcPtr = src;
    m_variantData.containedType = GUCEF_DATATYPE_FUNCTION_MEMORY_ADDRESS;
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( const void*& src )                                   
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.memory_address_data.const_objPtr = src;
    m_variantData.containedType = GUCEF_DATATYPE_CONST_DATA_MEMORY_ADDRESS;
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::operator=( void*& src )                                   
{GUCEF_TRACE;

    Clear();
    m_variantData.union_data.memory_address_data.const_objPtr = src;
    m_variantData.containedType = GUCEF_DATATYPE_DATA_MEMORY_ADDRESS;
    return *this;
}

/*-------------------------------------------------------------------------*/

/**
 *  Whether data of the given type can be linked to at the given address. Heap data of the UTF-16 and UTF-32 string
 *  types is read as code units and that of the 64 bit fraction types as 64 bit numbers: at an address that is not
 *  aligned for them the reads are misaligned (undefined behaviour, a fault on some processors). Linked data comes
 *  from wherever the caller has it, for instance a value at any offset in a serialized buffer.
 */
static bool
IsAlignedForLinking( const void* data, UInt8 varType )
{GUCEF_TRACE;

    size_t alignment = 1;
    switch ( varType )
    {
        case GUCEF_DATATYPE_UTF16_LE_STRING:
        case GUCEF_DATATYPE_UTF16_BE_STRING:        { alignment = sizeof( UInt16 ); break; }
        case GUCEF_DATATYPE_UTF32_LE_STRING:
        case GUCEF_DATATYPE_UTF32_BE_STRING:        { alignment = sizeof( UInt32 ); break; }
        case GUCEF_DATATYPE_INT64T2_LE_FRACTION:
        case GUCEF_DATATYPE_INT64T2_BE_FRACTION:
        case GUCEF_DATATYPE_UINT64T2_LE_FRACTION:
        case GUCEF_DATATYPE_UINT64T2_BE_FRACTION:   { alignment = sizeof( UInt64 ); break; }
        default:                                    { break; }
    }
    return 0 == ( reinterpret_cast< size_t >( data ) % alignment );
}

/*-------------------------------------------------------------------------*/

bool
CVariant::Set( const void* data, UInt32 dataSize, UInt8 varType, bool linkOnlyForDynMem )
{GUCEF_TRACE;

    Clear();

    switch ( varType )
    {
        case GUCEF_DATATYPE_INT8:
        {
            if ( GUCEF_NULL == data || dataSize < sizeof(Int8) )
                return false;
            memcpy( &m_variantData.union_data.int8_data, data, sizeof( Int8 ) );     // the data need not be aligned for the type
            m_variantData.containedType = GUCEF_DATATYPE_INT8;
            return true;
        }
        case GUCEF_DATATYPE_UINT8:
        {
            if ( GUCEF_NULL == data || dataSize < sizeof(UInt8) )
                return false;
            memcpy( &m_variantData.union_data.uint8_data, data, sizeof( UInt8 ) );     // the data need not be aligned for the type
            m_variantData.containedType = GUCEF_DATATYPE_UINT8;
            return true;
        }
        case GUCEF_DATATYPE_LE_INT16:
        case GUCEF_DATATYPE_BE_INT16:
        {
            if ( GUCEF_NULL == data || dataSize < sizeof(Int16) )
                return false;
            memcpy( &m_variantData.union_data.int16_data, data, sizeof( Int16 ) );     // the data need not be aligned for the type
            m_variantData.containedType = GUCEF_DATATYPE_INT16;
            return true;
        }
        case GUCEF_DATATYPE_LE_UINT16:
        case GUCEF_DATATYPE_BE_UINT16:
        {
            if ( GUCEF_NULL == data || dataSize < sizeof(UInt16) )
                return false;
            memcpy( &m_variantData.union_data.uint16_data, data, sizeof( UInt16 ) );     // the data need not be aligned for the type
            m_variantData.containedType = GUCEF_DATATYPE_UINT16;
            return true;
        }
        case GUCEF_DATATYPE_LE_INT32:
        case GUCEF_DATATYPE_BE_INT32:
        {
            if ( GUCEF_NULL == data || dataSize < sizeof(Int32) )
                return false;
            memcpy( &m_variantData.union_data.int32_data, data, sizeof( Int32 ) );     // the data need not be aligned for the type
            m_variantData.containedType = GUCEF_DATATYPE_INT32;
            return true;
        }
        case GUCEF_DATATYPE_LE_UINT32:
        case GUCEF_DATATYPE_BE_UINT32:
        {
            if ( GUCEF_NULL == data || dataSize < sizeof(UInt32) )
                return false;
            memcpy( &m_variantData.union_data.uint32_data, data, sizeof( UInt32 ) );     // the data need not be aligned for the type
            m_variantData.containedType = GUCEF_DATATYPE_UINT32;
            return true;
        }
        case GUCEF_DATATYPE_LE_INT64:
        case GUCEF_DATATYPE_BE_INT64:
        {
            if ( GUCEF_NULL == data || dataSize < sizeof(Int64) )
                return false;
            memcpy( &m_variantData.union_data.int64_data, data, sizeof( Int64 ) );     // the data need not be aligned for the type
            m_variantData.containedType = GUCEF_DATATYPE_INT64;
            return true;
        }
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_UINT64:
        case GUCEF_DATATYPE_BE_UINT64:
        {
            if ( GUCEF_NULL == data || dataSize < sizeof(UInt64) )
                return false;
            memcpy( &m_variantData.union_data.uint64_data, data, sizeof( UInt64 ) );     // the data need not be aligned for the type
            m_variantData.containedType = varType;
            return true;
        }
        case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        {
            if ( GUCEF_NULL == data || dataSize < sizeof(Int32) )
                return false;
            memcpy( &m_variantData.union_data.int32_data, data, sizeof( Int32 ) );     // the data need not be aligned for the type
            m_variantData.containedType = varType;
            return true;
        }
        case GUCEF_DATATYPE_FLOAT32:
        {
            if ( GUCEF_NULL == data || dataSize < sizeof(Float32) )
                return false;
            memcpy( &m_variantData.union_data.float32_data, data, sizeof( Float32 ) );     // the data need not be aligned for the type
            m_variantData.containedType = GUCEF_DATATYPE_FLOAT32;
            return true;
        }
        case GUCEF_DATATYPE_FLOAT64:
        {
            if ( GUCEF_NULL == data || dataSize < sizeof(Float64) )
                return false;
            memcpy( &m_variantData.union_data.float64_data, data, sizeof( Float64 ) );     // the data need not be aligned for the type
            m_variantData.containedType = GUCEF_DATATYPE_FLOAT64;
            return true;
        }
        case GUCEF_DATATYPE_BOOLEAN_INT32:
        {
            if ( GUCEF_NULL == data || dataSize < sizeof(Int32) )
                return false;
            memcpy( &m_variantData.union_data.int32_data, data, sizeof( Int32 ) );     // the data need not be aligned for the type
            m_variantData.containedType = GUCEF_DATATYPE_BOOLEAN_INT32;
            return true;
        }
        case GUCEF_DATATYPE_BINARY_BSOB:
        {
            if ( GUCEF_NULL == data || dataSize > sizeof(m_variantData.union_data.bsob_data) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Variant(" + CORE::ToString( this ) +
                    "):Set: Refusing to set BSOB data with ptr " + ToString( data ) + " and size " + ToString( dataSize ) );
                return false;
            }
            memset( m_variantData.union_data.bsob_data, 0, sizeof( m_variantData.union_data.bsob_data ) );
            memcpy( m_variantData.union_data.bsob_data, data, dataSize );
            m_variantData.containedType = GUCEF_DATATYPE_BINARY_BSOB;
            return true;
        }
        case GUCEF_DATATYPE_BINARY_BLOB:
        case GUCEF_DATATYPE_ASCII_STRING:
        case GUCEF_DATATYPE_UTF8_STRING:
        case GUCEF_DATATYPE_UTF16_LE_STRING:
        case GUCEF_DATATYPE_UTF16_BE_STRING:
        case GUCEF_DATATYPE_UTF32_STRING:
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING:
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING:
        case GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING:
        case GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING:
        case GUCEF_DATATYPE_INT64T2_LE_FRACTION:
        case GUCEF_DATATYPE_INT64T2_BE_FRACTION:
        case GUCEF_DATATYPE_UINT64T2_LE_FRACTION:
        case GUCEF_DATATYPE_UINT64T2_BE_FRACTION:
        {
            // data that is not aligned for the type is copied instead, the copy is aligned
            if ( linkOnlyForDynMem && IsAlignedForLinking( data, varType ) )
            {
                m_variantData.union_data.heap_data.heap_data_is_linked = 1;
                if ( dataSize > 0 && GUCEF_NULL != data )
                {
                    m_variantData.union_data.heap_data.union_data.void_heap_data = const_cast< void* >( data );
                    m_variantData.union_data.heap_data.heap_data_size = dataSize;
                }
                else
                {
                    m_variantData.union_data.heap_data.union_data.void_heap_data = GUCEF_NULL;
                    m_variantData.union_data.heap_data.heap_data_size = 0;
                }
                m_variantData.containedType = varType;
                return true;
            }
            else
            {
                m_variantData.union_data.heap_data.heap_data_is_linked = 0;
                if ( dataSize > 0 && GUCEF_NULL != data )
                {
                    if ( GUCEF_NULL != HeapReserve( dataSize ) )
                    {
                        memcpy( m_variantData.union_data.heap_data.union_data.void_heap_data, data, dataSize );
                        m_variantData.containedType = varType;
                        return true;
                    }
                    return false;
                }
                else
                {
                    m_variantData.union_data.heap_data.union_data.void_heap_data = GUCEF_NULL;
                    m_variantData.union_data.heap_data.heap_data_size = 0;
                    m_variantData.containedType = varType;
                    return true;
                }
            }
        }
        case GUCEF_DATATYPE_INT32T2_FRACTION:
        {
            if ( GUCEF_NULL == data || dataSize < (2*sizeof( Int32 )) )
                return false;

            m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator = ((Int32*)data)[ 0 ];
            m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator = ((Int32*)data)[ 1 ];
            m_variantData.containedType = GUCEF_DATATYPE_INT32T2_FRACTION;
            return true;
        }
        case GUCEF_DATATYPE_UINT32T2_FRACTION:
        {
            if ( GUCEF_NULL == data || dataSize < (2*sizeof( UInt32 )) )
                return false;

            m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator = ((UInt32*)data)[ 0 ];
            m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator = ((UInt32*)data)[ 1 ];
            m_variantData.containedType = GUCEF_DATATYPE_UINT32T2_FRACTION;
            return true;
        }
        case GUCEF_DATATYPE_INT64_INT32_FRACTION:
        {
            if ( GUCEF_NULL == data || dataSize < ( sizeof( Int64 ) + sizeof( Int32 ) ) )
                return false;

            m_variantData.union_data.fraction_data.union_data.int64_int32_data.numerator = *(Int64*) data;
            m_variantData.union_data.fraction_data.union_data.int64_int32_data.denominator = *(Int32*) ( ((char*)data)+sizeof(Int64) );
            m_variantData.containedType = GUCEF_DATATYPE_INT64_INT32_FRACTION;
            return true;
        }
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION:
        {
            if ( GUCEF_NULL == data || dataSize < ( sizeof( UInt64 ) + sizeof( UInt32 ) ) )
                return false;

            m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.numerator = *(UInt64*) data;
            m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.denominator = *(UInt32*) ( ((char*)data)+sizeof(UInt64) );
            m_variantData.containedType = GUCEF_DATATYPE_UINT64_UINT32_FRACTION;
            return true;
        }

        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
        {
            if ( GUCEF_NULL == data || dataSize < GUCEF_DECIMAL_PAYLOAD_SIZE )
                return false;

            // the data need not be aligned for the type, the byte order is as named by the type id
            TVariantData decimalData;
            memset( &decimalData, 0, sizeof( decimalData ) );
            memcpy( &decimalData.union_data.decimal_data.unscaled, data, sizeof( Int64 ) );
            memcpy( &decimalData.union_data.decimal_data.scale, static_cast< const char* >( data ) + sizeof( Int64 ), sizeof( UInt8 ) );
            decimalData.containedType = varType;

            CDecimal validated;
            if ( DecimalStatusIsError( DecimalFromVariantPayload( decimalData, validated ) ) )
                return false;

            memcpy( &m_variantData, &decimalData, sizeof( m_variantData ) );
            return true;
        }
        case GUCEF_DATATYPE_CONST_DATA_MEMORY_ADDRESS:
        case GUCEF_DATATYPE_DATA_MEMORY_ADDRESS:
        case GUCEF_DATATYPE_FUNCTION_MEMORY_ADDRESS:
        {
            // The data holds the address itself (as written by the binary serializer)
            UInt32 addressSize = GucefByteSizeOfFixedSizeType( varType );
            if ( GUCEF_NULL == data || 0 == addressSize || dataSize < addressSize )
                return false;
            memcpy( &m_variantData.union_data.memory_address_data, data, addressSize );
            m_variantData.containedType = varType;
            return true;
        }
        case GUCEF_DATATYPE_NULL:
        case GUCEF_DATATYPE_NIL:
        case GUCEF_DATATYPE_VOID:
        {
            // These types have no payload, the type itself is the value
            m_variantData.containedType = varType;
            return true;
        }
        default:
        {
            m_variantData.containedType = GUCEF_DATATYPE_UNKNOWN;
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::LinkTo( const CDynamicBuffer& src, UInt32 bufferOffset, UInt8 varType, UInt32 bytesToLink )
{GUCEF_TRACE;

    if ( bufferOffset + bytesToLink > src.GetDataSize() )
    {
        Clear();
        return *this;
    }

    UInt32 bufferSize = src.GetDataSize() - bufferOffset;
    if ( 0 == bytesToLink || bytesToLink > bufferSize )
        bytesToLink = bufferSize;

    Set( src.GetConstBufferPtr( bufferOffset ), bytesToLink, varType, true );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::LinkTo( const void* externalBuffer, UInt32 bufferSize, UInt8 varType )
{GUCEF_TRACE;

    Set( externalBuffer, bufferSize, varType, true );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::LinkTo( const char* externalBuffer, UInt32 bufferSize, UInt8 varType )
{GUCEF_TRACE;

    // Same as the void* version of this member function
    // the difference is in the default varType param used
    Set( externalBuffer, bufferSize, varType, true );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::LinkTo( const char* externalBuffer, UInt8 varType )
{GUCEF_TRACE;

    if ( GUCEF_NULL == externalBuffer )
    {
        Set( GUCEF_NULL, 0, varType, true );
        return *this;
    }

    switch ( varType )
    {
        case GUCEF_DATATYPE_ASCII_STRING:
        {
            UInt32 bufferSize = static_cast< UInt32 >( strlen( externalBuffer ) );
            Set( externalBuffer, bufferSize, varType, true );
            return *this;
        }
        case GUCEF_DATATYPE_UTF8_STRING:
        {
            UInt32 bufferSize = static_cast< UInt32 >( utf8size( externalBuffer ) );
            Set( externalBuffer, bufferSize, varType, true );
            return *this;
        }
        default:
        {
            // Cannot determine length automatically
            Set( externalBuffer, 0, varType, true );
            return *this;
        }
    }
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::LinkTo( const TVariantData& src )
{GUCEF_TRACE;

    Set( &src, true );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::LinkTo( const TVariantData* src )
{GUCEF_TRACE;

    Set( src, true );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::LinkTo( const CVariant& src )
{GUCEF_TRACE;

    Set( src.AsVoidPtr(), src.ByteSize(), src.GetTypeId(), true );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::LinkTo( const CAsciiString& src )
{GUCEF_TRACE;

    Set( src.C_String(), src.ByteSize(), GUCEF_DATATYPE_ASCII_STRING, true );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::LinkTo( const CUtf8String& src )
{GUCEF_TRACE;

    Set( src.C_String(), src.ByteSize(), GUCEF_DATATYPE_UTF8_STRING, true );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::LinkTo( const CUtf16String& src )
{GUCEF_TRACE;

    Set( src.C_String(), src.ByteSize(), GUCEF_DATATYPE_UTF16_STRING, true );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::LinkTo( const CUtf32String& src )
{GUCEF_TRACE;

    Set( src.C_String(), src.ByteSize(), GUCEF_DATATYPE_UTF32_STRING, true );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::LinkTo( const std::string& src )
{GUCEF_TRACE;

    Set( src.c_str(), (UInt32) src.size(), GUCEF_DATATYPE_UTF8_STRING, true );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::LinkTo( const std::wstring& src )
{GUCEF_TRACE;

    Set( src.c_str(), (UInt32)( (src.size()+1) * sizeof(std::wstring::value_type) ), WStringVariantType(), true );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CVariant::TransferOwnershipTo( CVariant& newOwner )
{GUCEF_TRACE;

    newOwner.Clear();
    newOwner.m_variantData = m_variantData;
    memset( &m_variantData, 0, sizeof( m_variantData ) );
    return newOwner;
}

/*-------------------------------------------------------------------------*/

bool
CVariant::TransferOwnershipFrom( CDynamicBuffer& oldOwner )
{GUCEF_TRACE;

    // first we get the dynamic buffer to give up ownership
    void* data = GUCEF_NULL;
    UInt32 dataSize  = 0;
    oldOwner.RelinquishDataOwnership( data, dataSize );

    // then we link to this memory
    if ( Set( data, dataSize, GUCEF_DATATYPE_BINARY_BLOB, true ) )
    {
        // then we wipe out knowledge that it was supposedly a link
        m_variantData.union_data.heap_data.heap_data_is_linked = 0;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void*
CVariant::HeapReserve( UInt32 byteSize, bool allowReduction )
{GUCEF_TRACE;

    if ( !UsesDynamicMemory() || !OwnsDynamicMemory() )
    {
        memset( &m_variantData, 0, sizeof( m_variantData ) );
        m_variantData.containedType = GUCEF_DATATYPE_BINARY_BLOB;
    }

    if ( allowReduction || m_variantData.union_data.heap_data.heap_data_size < byteSize )
    {
        if ( 0 == byteSize )
        {
            if ( GUCEF_NULL != m_variantData.union_data.heap_data.union_data.void_heap_data )
                free( m_variantData.union_data.heap_data.union_data.void_heap_data );
            m_variantData.union_data.heap_data.union_data.void_heap_data = GUCEF_NULL;
            m_variantData.union_data.heap_data.heap_data_size = 0;
            m_variantData.union_data.heap_data.heap_data_is_linked = 0;
            return GUCEF_NULL;
        }

        void* newBuffer = realloc( m_variantData.union_data.heap_data.union_data.void_heap_data, byteSize );
        if ( GUCEF_NULL != newBuffer )
        {
            m_variantData.union_data.heap_data.union_data.void_heap_data = newBuffer;
            m_variantData.union_data.heap_data.heap_data_size = byteSize;
            return newBuffer;
        }
        else
        {
            return GUCEF_NULL;
        }
    }

    return m_variantData.union_data.heap_data.union_data.void_heap_data;
}

/*-------------------------------------------------------------------------*/

bool
CVariant::SetString( UInt8 varType, const CString& data, const CVariant& defaultValue )
{GUCEF_TRACE;

    switch ( varType )
    {
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING:  { *this = ToAsciiString( data ); if ( IsNULLOrEmpty() ) *this = defaultValue.AsAsciiString(); m_variantData.containedType = varType; return true; }
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING:   { *this = ToUtf8String( data ); if ( IsNULLOrEmpty() ) *this = defaultValue.AsUtf8String(); m_variantData.containedType = varType; return true; }
        case GUCEF_DATATYPE_ASCII_STRING:          { *this = ToAsciiString( data ); if ( IsNULLOrEmpty() ) *this = defaultValue.AsAsciiString(); return true; }
        case GUCEF_DATATYPE_UTF8_STRING:           { *this = ToUtf8String( data ); if ( IsNULLOrEmpty() ) *this = defaultValue.AsUtf8String(); return true; }
        default:                                   { return false; }
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  Stores native byte order ticks under a 64 bit timestamp type id of either byte order
 */
static bool
SetSignedTicksInByteOrderOfId( CVariant& variant, Int64 ticks, UInt8 varType )
{
    bool bigEndianId = GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH == varType  ||
                       GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH == varType   ||
                       GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH == varType     ||
                       GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH == varType;
    #if ( GUCEF_PLATFORM_BYTEORDER_ENDIAN == GUCEF_BYTEORDER_LITTLE_ENDIAN )
    if ( bigEndianId )
        ticks = SwapEndianInt64( ticks );
    #else
    if ( !bigEndianId )
        ticks = SwapEndianInt64( ticks );
    #endif
    return variant.Set( &ticks, sizeof( ticks ), varType, false );
}

/*-------------------------------------------------------------------------*/

bool
CVariant::SetFromString( UInt8 varType, const CString& data, const CVariant& defaultValue )
{GUCEF_TRACE;

    switch ( varType )
    {
        case GUCEF_DATATYPE_UINT8:
            { *this = StringToUInt8( data, defaultValue.AsUInt8() ); return true; }

        case GUCEF_DATATYPE_LE_UINT16:
        case GUCEF_DATATYPE_BE_UINT16:
            { *this = StringToUInt16( data, defaultValue.AsUInt16() ); return true; }

        case GUCEF_DATATYPE_LE_UINT32:
        case GUCEF_DATATYPE_BE_UINT32:
            { *this = StringToUInt32( data, defaultValue.AsUInt32() ); return true; }

        case GUCEF_DATATYPE_LE_UINT64:
        case GUCEF_DATATYPE_BE_UINT64:
            { *this = StringToUInt64( data, defaultValue.AsUInt64() ); return true; }

        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        {
            // signed tick counts in text, the type id is kept
            Int64 ticks = StringToInt64( data, defaultValue.AsInt64() );
            return SetSignedTicksInByteOrderOfId( *this, ticks, varType );
        }
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        {
            // ISO 8601 text, the type id is kept
            Int64 ticks = 0;
            CUtf8String utf8 = ToUtf8String( data );
            const UInt32 length = utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0;
            if ( !CDataTableValues::ParseTimestamp( utf8.C_String(), length, DATATABLE_TIMEUNIT_MICROSECONDS, ticks ) &&
                 !CDataTableValues::TicksFromVariant( defaultValue, DATATABLE_TIMEUNIT_MICROSECONDS, ticks ) )
                return false;
            return SetSignedTicksInByteOrderOfId( *this, ticks, varType );
        }
        case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        {
            // yyyy-mm-dd, the type id is kept
            Int32 days = 0;
            CUtf8String utf8 = ToUtf8String( data );
            const UInt32 length = utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0;
            if ( !CDataTableValues::ParseDate( utf8.C_String(), length, days ) )
            {
                if ( !IsDateTypeId( defaultValue.GetTypeId() ) )
                    return false;
                days = defaultValue.AsInt32();
            }
            if ( GUCEF_DATATYPE_NON_NATIVE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == varType )
                days = SwapEndianInt32( days );
            return Set( &days, sizeof( days ), varType, false );
        }

        case GUCEF_DATATYPE_INT8:
            { *this = StringToInt8( data, defaultValue.AsInt8() ); return true; }

        case GUCEF_DATATYPE_LE_INT16:
        case GUCEF_DATATYPE_BE_INT16:
            { *this = StringToInt16( data, defaultValue.AsInt16() ); return true; }

        case GUCEF_DATATYPE_LE_INT32:
        case GUCEF_DATATYPE_BE_INT32:
            { *this = StringToInt32( data, defaultValue.AsInt32() ); return true; }

        case GUCEF_DATATYPE_LE_INT64:
        case GUCEF_DATATYPE_BE_INT64:
            { *this = StringToInt64( data, defaultValue.AsInt64() ); return true; }

        case GUCEF_DATATYPE_FLOAT32:               { *this = StringToFloat( data, defaultValue.AsFloat32() ); return true; }
        case GUCEF_DATATYPE_FLOAT64:               { *this = StringToDouble( data, defaultValue.AsFloat64() ); return true; }
        case GUCEF_DATATYPE_BOOLEAN_INT32:         { *this = StringToBool( data, defaultValue.AsBool() ); return true; }
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING:  { return SetString( varType, data, defaultValue ); }
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING:   { return SetString( varType, data, defaultValue ); }
        case GUCEF_DATATYPE_ASCII_STRING:          { return SetString( varType, data, defaultValue ); }
        case GUCEF_DATATYPE_UTF8_STRING:           { return SetString( varType, data, defaultValue ); }
        case GUCEF_DATATYPE_BINARY_BLOB:           { return Base64Decode( data, HeapReserve( data.ByteSize()-1 ), data.ByteSize()-1, m_variantData.union_data.heap_data.heap_data_size ); }

        case GUCEF_DATATYPE_INT32T2_FRACTION:
        {
            TInt32Fraction fraction;
            if ( fraction.FromString( data ) )
            {
                *this = fraction;
                return true;
            }
            return false;
        }
        case GUCEF_DATATYPE_UINT32T2_FRACTION:
        {
            TUInt32Fraction fraction;
            if ( fraction.FromString( data ) )
            {
                *this = fraction;
                return true;
            }
            return false;
        }
        case GUCEF_DATATYPE_INT64_INT32_FRACTION:
        {
            TInt64Int32Fraction fraction;
            if ( fraction.FromString( data ) )
            {
                *this = fraction;
                return true;
            }
            return false;
        }
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION:
        {
            TUInt64UInt32Fraction fraction;
            if ( fraction.FromString( data ) )
            {
                *this = fraction;
                return true;
            }
            return false;
        }
        case GUCEF_DATATYPE_INT64T2_FRACTION:
        {
            TInt64Fraction fraction;
            if ( fraction.FromString( data ) )
            {
                *this = fraction;
                return true;
            }
            return false;
        }
        case GUCEF_DATATYPE_UINT64T2_FRACTION:
        {
            TUInt64Fraction fraction;
            if ( fraction.FromString( data ) )
            {
                *this = fraction;
                return true;
            }
            return false;
        }
        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
        {
            CDecimal value;
            if ( DecimalStatusIsError( CDecimal::FromString( data, value ) ) &&
                 DecimalStatusIsError( defaultValue.GetDecimal( value ) ) )
                return false;

            *this = value;
            if ( GUCEF_DATATYPE_NON_NATIVE_INT64_UINT8_DECIMAL == varType )
            {
                if ( GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL == varType )
                    ToBigEndian();
                else
                    ToLittleEndian();
            }
            return true;
        }
        case GUCEF_DATATYPE_UNKNOWN:               { return false; }
        default:                                   { return false; }
    }
}

/*-------------------------------------------------------------------------*/

bool
CVariant::operator==( const CVariant& other ) const
{GUCEF_TRACE;

    if ( IsDecimal() || other.IsDecimal() )
    {
        Int32 cmp = 0;
        Int32 how = CompareWithDecimal( *this, other, cmp );
        if ( 1 == how )
            return 0 == cmp;
        if ( 2 == how )
            return std::fabs( AsFloat64() - other.AsFloat64() ) < GUCEF_FLOAT64_COMPARISON_EPSILON;
    }

    if ( IsInteger() && other.IsInteger() )
    {
        bool isSignedInt = IsSignedInteger();
        bool otherSignedInt = other.IsSignedInteger();

        if ( isSignedInt && otherSignedInt )
        {
            return AsInt64() == other.AsInt64();
        }
        else
        if ( !isSignedInt && !otherSignedInt )
        {
            return AsUInt64() == other.AsUInt64();
        }
        else
        {
            // Comparing signed and unsigned integers
            return AsInt64() == other.AsInt64();
        }
    }
    else
    if ( IsFloat() && other.IsFloat() )
    {
        return std::fabs( AsFloat64() - other.AsFloat64() ) < GUCEF_FLOAT64_COMPARISON_EPSILON;
    }
    else
    if ( IsBoolean() || other.IsBoolean() )
    {
        return AsBool() == other.AsBool();
    }
    else
    if ( IsFraction() || other.IsFraction() )
    {
        return std::fabs( AsFloat64() - other.AsFloat64() ) < GUCEF_FLOAT64_COMPARISON_EPSILON;
    }
    else
    if ( IsComparedAsNumber( *this ) && IsComparedAsNumber( other ) )
    {
        // an integer and a float
        return std::fabs( AsFloat64() - other.AsFloat64() ) < GUCEF_FLOAT64_COMPARISON_EPSILON;
    }
    else
    {
        // text, binary data and the rest, see CompareRemaining()
        return 0 == CompareRemaining( *this, other );
    }
}

/*-------------------------------------------------------------------------*/

bool
CVariant::operator!=( const CVariant& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CVariant::operator<( const CVariant& other ) const
{GUCEF_TRACE;

    if ( IsDecimal() || other.IsDecimal() )
    {
        Int32 cmp = 0;
        Int32 how = CompareWithDecimal( *this, other, cmp );
        if ( 1 == how )
            return cmp < 0;
        if ( 2 == how )
            return AsFloat64() < other.AsFloat64();
    }

    if ( IsInteger() && other.IsInteger() )
    {
        bool isSignedInt = IsSignedInteger();
        bool otherSignedInt = other.IsSignedInteger();

        if ( isSignedInt && otherSignedInt )
        {
            return AsInt64() < other.AsInt64();
        }
        else
        if ( !isSignedInt && !otherSignedInt )
        {
            return AsUInt64() < other.AsUInt64();
        }
        else
        {
            // Comparing signed and unsigned integers
            return AsInt64() < other.AsInt64();
        }
    }
    else
    if ( IsFloat() && other.IsFloat() )
    {
        return AsFloat64() < other.AsFloat64();
    }
    else
    if ( IsBoolean() || other.IsBoolean() )
    {
        return AsBool() < other.AsBool();
    }
    else
    if ( IsComparedAsNumber( *this ) && IsComparedAsNumber( other ) )
    {
        // an integer, a float or a fraction against another kind of number
        return AsFloat64() < other.AsFloat64();
    }
    else
    {
        // text, binary data and the rest, see CompareRemaining()
        return 0 > CompareRemaining( *this, other );
    }
}

/*-------------------------------------------------------------------------*/

bool
CVariant::operator>( const CVariant& other ) const
{GUCEF_TRACE;

    return !( *this <= other );
}

/*-------------------------------------------------------------------------*/

bool
CVariant::operator<=( const CVariant& other ) const
{GUCEF_TRACE;

    if ( IsDecimal() || other.IsDecimal() )
    {
        Int32 cmp = 0;
        Int32 how = CompareWithDecimal( *this, other, cmp );
        if ( 1 == how )
            return cmp <= 0;
        if ( 2 == how )
            return AsFloat64() <= other.AsFloat64();
    }

    if ( IsInteger() && other.IsInteger() )
    {
        bool isSignedInt = IsSignedInteger();
        bool otherSignedInt = other.IsSignedInteger();

        if ( isSignedInt && otherSignedInt )
        {
            return AsInt64() <= other.AsInt64();
        }
        else
        if ( !isSignedInt && !otherSignedInt )
        {
            return AsUInt64() <= other.AsUInt64();
        }
        else
        {
            // Comparing signed and unsigned integers
            return AsInt64() <= other.AsInt64();
        }
    }
    else
    if ( IsFloat() && other.IsFloat() )
    {
        return AsFloat64() <= other.AsFloat64();
    }
    else
    if ( IsBoolean() || other.IsBoolean() )
    {
        return AsBool() <= other.AsBool();
    }
    else
    if ( IsComparedAsNumber( *this ) && IsComparedAsNumber( other ) )
    {
        // an integer, a float or a fraction against another kind of number
        return AsFloat64() <= other.AsFloat64();
    }
    else
    {
        // text, binary data and the rest, see CompareRemaining()
        return 0 >= CompareRemaining( *this, other );
    }
}

/*-------------------------------------------------------------------------*/

bool
CVariant::operator>=( const CVariant& other ) const
{GUCEF_TRACE;

    return !( *this < other );
}

/*-------------------------------------------------------------------------*/

void
CVariant::Clear( void )
{GUCEF_TRACE;

    if ( OwnsDynamicMemory() )
    {
        if ( GUCEF_NULL != m_variantData.union_data.heap_data.union_data.void_heap_data )
            free( m_variantData.union_data.heap_data.union_data.void_heap_data );
    }
    // else: No dynamic memory management needed

    memset( &m_variantData, 0, sizeof( m_variantData ) );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CVariant::AsAsciiString( const CAsciiString& defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    if ( GUCEF_DATATYPE_ASCII_STRING == m_variantData.containedType                 ||
         GUCEF_DATATYPE_BOOLEAN_ASCII_STRING == m_variantData.containedType         ||
         GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING == m_variantData.containedType )
    {
        if ( resolveVarsIfApplicable )
        {
            if ( 0 == m_variantData.union_data.heap_data.heap_data_size ||
                 ( 1 == m_variantData.union_data.heap_data.heap_data_size && '\0' == *( (const char*) m_variantData.union_data.heap_data.union_data.char_heap_data ) ) )
                return ResolveVars( ToString( defaultIfNeeded ) );

            return ResolveVars( ToString( CAsciiString( (const char*) m_variantData.union_data.heap_data.union_data.char_heap_data, m_variantData.union_data.heap_data.heap_data_size ) ) );
        }
        else
        {
            if ( 0 == m_variantData.union_data.heap_data.heap_data_size ||
                 ( 1 == m_variantData.union_data.heap_data.heap_data_size && '\0' == *( (const char*) m_variantData.union_data.heap_data.union_data.char_heap_data ) ) )
                return defaultIfNeeded;

            return CAsciiString( (const char*) m_variantData.union_data.heap_data.union_data.char_heap_data, m_variantData.union_data.heap_data.heap_data_size );
        }
    }
    return ToAsciiString( AsString( defaultIfNeeded ) );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CVariant::AsUtf8String( const CUtf8String& defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    if ( GUCEF_DATATYPE_UTF8_STRING == m_variantData.containedType                 ||
         GUCEF_DATATYPE_BOOLEAN_UTF8_STRING == m_variantData.containedType         ||
         GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING == m_variantData.containedType )
    {
        if ( resolveVarsIfApplicable )
        {
            if ( 0 == m_variantData.union_data.heap_data.heap_data_size ||
                 ( 1 == m_variantData.union_data.heap_data.heap_data_size && '\0' == *( (const char*) m_variantData.union_data.heap_data.union_data.char_heap_data ) ) )
                return ResolveVars( ToString( defaultIfNeeded ) );

            return ResolveVars( ToString( CUtf8String( (const char*) m_variantData.union_data.heap_data.union_data.char_heap_data, m_variantData.union_data.heap_data.heap_data_size ) ) );
        }
        else
        {
            if ( 0 == m_variantData.union_data.heap_data.heap_data_size ||
                 ( 1 == m_variantData.union_data.heap_data.heap_data_size && '\0' == *( (const char*) m_variantData.union_data.heap_data.union_data.char_heap_data ) ) )
                return defaultIfNeeded;

            return CUtf8String( (const char*) m_variantData.union_data.heap_data.union_data.char_heap_data, m_variantData.union_data.heap_data.heap_data_size );
        }
    }
    return ToUtf8String( AsString( defaultIfNeeded, resolveVarsIfApplicable ) );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CVariant::AsUtf16String( const CUtf16String& defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    if ( GUCEF_DATATYPE_UTF16_LE_STRING == m_variantData.containedType ||
         GUCEF_DATATYPE_UTF16_BE_STRING == m_variantData.containedType )
    {
        if ( m_variantData.union_data.heap_data.heap_data_size > sizeof(UInt16) )
        {
            const UInt16* ptr = static_cast< const UInt16* >( m_variantData.union_data.heap_data.union_data.void_heap_data );
            UInt32 codeUnits = ( m_variantData.union_data.heap_data.heap_data_size / sizeof(UInt16) ) - 1;
            CUtf16String tmp( ptr, codeUnits );
            if ( resolveVarsIfApplicable )
                return CUtf16String( ResolveVars( CString( tmp.STL_String() ) ) );
            return tmp;
        }
        return resolveVarsIfApplicable ? CUtf16String( ResolveVars( CString( defaultIfNeeded.STL_String() ) ) ) : defaultIfNeeded;
    }
    return CUtf16String( AsString( CString( defaultIfNeeded.STL_String() ), resolveVarsIfApplicable ) );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CVariant::AsUtf32String( const CUtf32String& defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    if ( GUCEF_DATATYPE_UTF32_LE_STRING == m_variantData.containedType ||
         GUCEF_DATATYPE_UTF32_BE_STRING == m_variantData.containedType )
    {
        if ( m_variantData.union_data.heap_data.heap_data_size > sizeof(UInt32) )
        {
            const UInt32* ptr = static_cast< const UInt32* >( m_variantData.union_data.heap_data.union_data.void_heap_data );
            UInt32 codePoints = ( m_variantData.union_data.heap_data.heap_data_size / sizeof(UInt32) ) - 1;
            CUtf32String tmp( ptr, codePoints );
            if ( resolveVarsIfApplicable )
                return CUtf32String( ResolveVars( CString( tmp.STL_String() ) ) );
            return tmp;
        }
        return resolveVarsIfApplicable ? CUtf32String( ResolveVars( CString( defaultIfNeeded.STL_String() ) ) ) : defaultIfNeeded;
    }
    return CUtf32String( AsString( CString( defaultIfNeeded.STL_String() ), resolveVarsIfApplicable ) );
}

/*-------------------------------------------------------------------------*/

CString
CVariant::AsString( const CString& defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    if ( resolveVarsIfApplicable )
    {
        switch ( m_variantData.containedType )
        {
            case GUCEF_DATATYPE_UINT8:
                { return ToString( AsUInt8() ); }

            case GUCEF_DATATYPE_LE_UINT16:
            case GUCEF_DATATYPE_BE_UINT16:
                { return ToString( AsUInt16() ); }

            case GUCEF_DATATYPE_LE_UINT32:
            case GUCEF_DATATYPE_BE_UINT32:
                { return ToString( AsUInt32() ); }

            case GUCEF_DATATYPE_LE_UINT64:
            case GUCEF_DATATYPE_BE_UINT64:
                { return ToString( AsUInt64() ); }

            case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
            case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
            case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
            case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
            case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
            case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
                { return ToString( AsInt64() ); }

            case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
            case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
                { return CDataTableValues::TimestampToString( MicrosFromVariantPayload( m_variantData ), DATATABLE_TIMEUNIT_MICROSECONDS, true ); }

            case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
            case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
                { return CDataTableValues::DateToString( DaysFromVariantPayload( m_variantData ) ); }

            case GUCEF_DATATYPE_INT8:
                { return ToString( AsInt8() ); }

            case GUCEF_DATATYPE_LE_INT16:
            case GUCEF_DATATYPE_BE_INT16:
                { return ToString( AsInt16() ); }

            case GUCEF_DATATYPE_LE_INT32:
            case GUCEF_DATATYPE_BE_INT32:
                { return ToString( AsInt32() ); }

            case GUCEF_DATATYPE_LE_INT64:
            case GUCEF_DATATYPE_BE_INT64:
                { return ToString( AsInt64() ); }

            case GUCEF_DATATYPE_FLOAT32:
                { return ToString( AsFloat32() ); }
            case GUCEF_DATATYPE_FLOAT64:
                { return ToString( AsFloat64() ); }

            case GUCEF_DATATYPE_BOOLEAN_INT32:
                { return ToString( AsBool() ); }

            case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING:  { return ResolveVars( ToString( AsAsciiString( ToAsciiString( ResolveVars( defaultIfNeeded ) ) ) ) ); }
            case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING:   { return ResolveVars( ToString( AsUtf8String( ToUtf8String( ResolveVars( defaultIfNeeded ) ) ) ) ); }
            case GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING:
            case GUCEF_DATATYPE_ASCII_STRING:          { return ResolveVars( ToString( AsAsciiString( ToAsciiString( ResolveVars( defaultIfNeeded ) ) ) ) ); }
            case GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING:
            case GUCEF_DATATYPE_UTF8_STRING:           { return ResolveVars( ToString( AsUtf8String( ToUtf8String( ResolveVars( defaultIfNeeded ) ) ) ) ); }

            case GUCEF_DATATYPE_INT32T2_FRACTION:      { return ToString( m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator ) + '/' + ToString( m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator ); }
            case GUCEF_DATATYPE_UINT32T2_FRACTION:     { return ToString( m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator ) + '/' + ToString( m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator ); }

            case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
            case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
            {
                CDecimal value;
                if ( DecimalStatusIsError( GetDecimal( value ) ) )
                    return ResolveVars( defaultIfNeeded );
                return value.ToString();
            }

            case GUCEF_DATATYPE_UTF16_LE_STRING:
            case GUCEF_DATATYPE_UTF16_BE_STRING:
            {
                if ( m_variantData.union_data.heap_data.heap_data_size > sizeof(UInt16) )
                {
                    const UInt16* ptr = static_cast< const UInt16* >( m_variantData.union_data.heap_data.union_data.void_heap_data );
                    UInt32 codeUnits = ( m_variantData.union_data.heap_data.heap_data_size / sizeof(UInt16) ) - 1;
                    CString result( CUtf16String( ptr, codeUnits ).STL_String() );
                    return resolveVarsIfApplicable ? ResolveVars( result ) : result;
                }
                return CString::Empty;
            }

            case GUCEF_DATATYPE_UTF32_LE_STRING:
            case GUCEF_DATATYPE_UTF32_BE_STRING:
            {
                if ( m_variantData.union_data.heap_data.heap_data_size > sizeof(UInt32) )
                {
                    const UInt32* ptr = static_cast< const UInt32* >( m_variantData.union_data.heap_data.union_data.void_heap_data );
                    UInt32 codePoints = ( m_variantData.union_data.heap_data.heap_data_size / sizeof(UInt32) ) - 1;
                    CString result( CUtf32String( ptr, codePoints ).STL_String() );
                    return resolveVarsIfApplicable ? ResolveVars( result ) : result;
                }
                return CString::Empty;
            }

            case GUCEF_DATATYPE_BINARY_BSOB:           { CString result = Base64Encode( m_variantData.union_data.bsob_data, sizeof( m_variantData.union_data.bsob_data ) ); return result.IsNULLOrEmpty() ? ResolveVars( defaultIfNeeded ) : result; }
            case GUCEF_DATATYPE_BINARY_BLOB:           { CString result = Base64Encode( m_variantData.union_data.heap_data.union_data.void_heap_data, m_variantData.union_data.heap_data.heap_data_size ); return result.IsNULLOrEmpty() ? ResolveVars( defaultIfNeeded ) : result; }
            case GUCEF_DATATYPE_UNKNOWN:               { return ResolveVars( defaultIfNeeded ); }
            default:                                   { return ResolveVars( defaultIfNeeded ); }
        }
    }
    else
    {
        switch ( m_variantData.containedType )
        {
            case GUCEF_DATATYPE_UINT8:                 { return ToString( AsUInt8( StringToUInt8( defaultIfNeeded, 0 ) ) ); }
            case GUCEF_DATATYPE_UINT16:                { return ToString( AsUInt16( StringToUInt16( defaultIfNeeded, 0 ) ) ); }
            case GUCEF_DATATYPE_UINT32:                { return ToString( AsUInt32( StringToUInt32( defaultIfNeeded, 0 ) ) ); }

            case GUCEF_DATATYPE_LE_UINT64:
            case GUCEF_DATATYPE_BE_UINT64:
                { return ToString( AsUInt64( StringToUInt64( defaultIfNeeded, 0 ) ) ); }

            case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
            case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
            case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
            case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
            case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
            case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
                { return ToString( AsInt64( StringToInt64( defaultIfNeeded, 0 ) ) ); }

            case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
            case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
                { return CDataTableValues::TimestampToString( MicrosFromVariantPayload( m_variantData ), DATATABLE_TIMEUNIT_MICROSECONDS, true ); }

            case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
            case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
                { return CDataTableValues::DateToString( DaysFromVariantPayload( m_variantData ) ); }

            case GUCEF_DATATYPE_INT8:                  { return ToString( AsInt8( StringToInt8( defaultIfNeeded, 0 ) ) ); }
            case GUCEF_DATATYPE_INT16:                 { return ToString( AsInt16( StringToInt16( defaultIfNeeded, 0 ) ) ); }
            case GUCEF_DATATYPE_INT32:                 { return ToString( AsInt32( StringToInt32( defaultIfNeeded, 0 ) ) ); }
            case GUCEF_DATATYPE_INT64:                 { return ToString( AsInt64( StringToInt64( defaultIfNeeded, 0 ) ) ); }
            case GUCEF_DATATYPE_FLOAT32:               { return ToString( AsFloat32( StringToFloat( defaultIfNeeded, 0.0f ) ) ); }
            case GUCEF_DATATYPE_FLOAT64:               { return ToString( AsFloat64( StringToDouble( defaultIfNeeded, 0.0 ) ) ); }
            case GUCEF_DATATYPE_BOOLEAN_INT32:         { return ToString( AsBool( StringToBool( defaultIfNeeded, false ) ) ); }
            case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING:  { return ToString( AsAsciiString( ToAsciiString( defaultIfNeeded ) ) ); }
            case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING:   { return ToString( AsUtf8String( ToUtf8String( defaultIfNeeded ) ) ); }
            case GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING:
            case GUCEF_DATATYPE_ASCII_STRING:          { return ToString( AsAsciiString( ToAsciiString( defaultIfNeeded ), false ) ); }
            case GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING:
            case GUCEF_DATATYPE_UTF8_STRING:           { return ToString( AsUtf8String( ToUtf8String( defaultIfNeeded ), false ) ); }

            case GUCEF_DATATYPE_INT32T2_FRACTION:      { return ToString( m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator ) + '/' + ToString( m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator ); }
            case GUCEF_DATATYPE_UINT32T2_FRACTION:     { return ToString( m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator ) + '/' + ToString( m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator ); }

            case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
            case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
            {
                CDecimal value;
                if ( DecimalStatusIsError( GetDecimal( value ) ) )
                    return defaultIfNeeded;
                return value.ToString();
            }

            case GUCEF_DATATYPE_UTF16_LE_STRING:
            case GUCEF_DATATYPE_UTF16_BE_STRING:
            {
                if ( m_variantData.union_data.heap_data.heap_data_size > sizeof(UInt16) )
                {
                    const UInt16* ptr = static_cast< const UInt16* >( m_variantData.union_data.heap_data.union_data.void_heap_data );
                    UInt32 codeUnits = ( m_variantData.union_data.heap_data.heap_data_size / sizeof(UInt16) ) - 1;
                    CString result( CUtf16String( ptr, codeUnits ).STL_String() );
                    return resolveVarsIfApplicable ? ResolveVars( result ) : result;
                }
                return CString::Empty;
            }

            case GUCEF_DATATYPE_UTF32_LE_STRING:
            case GUCEF_DATATYPE_UTF32_BE_STRING:
            {
                if ( m_variantData.union_data.heap_data.heap_data_size > sizeof(UInt32) )
                {
                    const UInt32* ptr = static_cast< const UInt32* >( m_variantData.union_data.heap_data.union_data.void_heap_data );
                    UInt32 codePoints = ( m_variantData.union_data.heap_data.heap_data_size / sizeof(UInt32) ) - 1;
                    CString result( CUtf32String( ptr, codePoints ).STL_String() );
                    return resolveVarsIfApplicable ? ResolveVars( result ) : result;
                }
                return CString::Empty;
            }

            case GUCEF_DATATYPE_BINARY_BSOB:           { CString result = Base64Encode( m_variantData.union_data.bsob_data, sizeof( m_variantData.union_data.bsob_data ) ); return result.IsNULLOrEmpty() ? defaultIfNeeded : result; }
            case GUCEF_DATATYPE_BINARY_BLOB:           { CString result = Base64Encode( m_variantData.union_data.heap_data.union_data.void_heap_data, m_variantData.union_data.heap_data.heap_data_size ); return result.IsNULLOrEmpty() ? defaultIfNeeded : result; }
            case GUCEF_DATATYPE_UNKNOWN:               { return defaultIfNeeded; }
            default:                                   { return defaultIfNeeded; }
        }
    }
}

/*-------------------------------------------------------------------------*/

const TVariantData*
CVariant::CStyleAccess( void ) const
{GUCEF_TRACE;

    return &m_variantData;
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer
CVariant::AsBuffer( void ) const
{GUCEF_TRACE;

    return CDynamicBuffer( static_cast< const char* >( AsVoidPtr() ), ByteSize() );
}

/*-------------------------------------------------------------------------*/

CDateTime
CVariant::AsDateTime( const CDateTime& defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_UTF8_STRING:
        case GUCEF_DATATYPE_ASCII_STRING:
        case GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING:
        case GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING:
        {
            CString str = AsString( CString::Empty, resolveVarsIfApplicable );
            if ( str.IsNULLOrEmpty() )
                return defaultIfNeeded;

            CDateTime dt;
            if ( dt.FromIso8601DateTimeString( str ) )
                return dt;
            return defaultIfNeeded;
        }

        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        {
            CTimestamp ts;
            ts.FromNanosecondsSinceEpoch( (UInt64) SignedTicksFromVariantPayload( m_variantData ) );
            return ts.ToDateTime();
        }
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        {
            CDateTime dt;
            dt.FromUnixEpochBasedTicksInMillisecs( (UInt64) SignedTicksFromVariantPayload( m_variantData ) * 1000 );
            return dt;
        }
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_UINT64:
        case GUCEF_DATATYPE_BE_UINT64:
        {
            CDateTime dt;
            dt.FromUnixEpochBasedTicksInMillisecs( (UInt64) SignedTicksFromVariantPayload( m_variantData ) );
            return dt;
        }
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        {
            CDateTime dt;
            if ( CDataTableValues::DateTimeFromTicks( MicrosFromVariantPayload( m_variantData ), DATATABLE_TIMEUNIT_MICROSECONDS, dt ) )
                return dt;
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        {
            CDateTime dt;
            if ( CDataTableValues::DateTimeFromTicks( (Int64) DaysFromVariantPayload( m_variantData ) * CDataTableValues::TicksPerDay( DATATABLE_TIMEUNIT_MILLISECONDS ), DATATABLE_TIMEUNIT_MILLISECONDS, dt ) )
                return dt;
            return defaultIfNeeded;
        }

        default:
            return defaultIfNeeded;
    }
}

/*-------------------------------------------------------------------------*/

CTimestamp
CVariant::AsTimestamp( const CTimestamp& defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_UTF8_STRING:
        case GUCEF_DATATYPE_ASCII_STRING:
        case GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING:
        case GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING:
        {
            CString str = AsString( CString::Empty, resolveVarsIfApplicable );
            if ( str.IsNULLOrEmpty() )
                return defaultIfNeeded;

            CDateTime dt;
            if ( dt.FromIso8601DateTimeString( str ) )
                return CTimestamp( dt );
            return defaultIfNeeded;
        }

        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        {
            CTimestamp ts;
            ts.FromNanosecondsSinceEpoch( (UInt64) SignedTicksFromVariantPayload( m_variantData ) );
            return ts;
        }
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        {
            CTimestamp ts;
            ts.FromMillisecondsSinceEpoch( (UInt64) SignedTicksFromVariantPayload( m_variantData ) * 1000 );
            return ts;
        }
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_UINT64:
        case GUCEF_DATATYPE_BE_UINT64:
        {
            CTimestamp ts;
            ts.FromMillisecondsSinceEpoch( (UInt64) SignedTicksFromVariantPayload( m_variantData ) );
            return ts;
        }
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        {
            // CTimestamp holds unsigned ticks: no instant before the epoch
            Int64 micros = MicrosFromVariantPayload( m_variantData );
            if ( micros < 0 )
                return defaultIfNeeded;
            CTimestamp ts;
            ts.FromMicrosecondsSinceEpoch( (UInt64) micros );
            return ts;
        }
        case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        {
            Int32 days = DaysFromVariantPayload( m_variantData );
            if ( days < 0 )
                return defaultIfNeeded;
            CTimestamp ts;
            ts.FromMillisecondsSinceEpoch( (UInt64) days * 86400000ULL );
            return ts;
        }

        default:
            return defaultIfNeeded;
    }
}

/*-------------------------------------------------------------------------*/

TInt8Fraction
CVariant::AsInt8Fraction( const TInt8Fraction& defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT32T2_FRACTION:
        {
            return TInt8Fraction( static_cast< Int8 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator ),
                                  static_cast< Int8 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT32T2_FRACTION:
        {
            return TInt8Fraction( static_cast< Int8 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator ),
                                  static_cast< Int8 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_INT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TInt64T2Fraction ) )
            {
                return TInt8Fraction( static_cast< Int8 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->numerator ),
                                      static_cast< Int8 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_UINT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TUInt64T2Fraction ) )
            {
                return TInt8Fraction( static_cast< Int8 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->numerator ),
                                      static_cast< Int8 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_INT64_INT32_FRACTION:
        {
            return TInt8Fraction( static_cast< Int8 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.numerator ),
                                  static_cast< Int8 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION:
        {
            return TInt8Fraction( static_cast< Int8 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.numerator ),
                                  static_cast< Int8 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.denominator ) );
        }
        case GUCEF_DATATYPE_FLOAT32:
        {
            return TInt8Fraction::ConvertFloat32ToFraction( AsFloat32() );
        }
        case GUCEF_DATATYPE_FLOAT64:
        {
            return TInt8Fraction::ConvertFloat64ToFraction( AsFloat64() );
        }
        default:
        {
            if ( IsInteger() )
                return TInt8Fraction( AsInt8(), 1 );
            return defaultIfNeeded;
        }
    }
}

/*-------------------------------------------------------------------------*/

TUInt8Fraction
CVariant::AsUInt8Fraction( const TUInt8Fraction& defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT32T2_FRACTION:
        {
            return TUInt8Fraction( static_cast< UInt8 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator ),
                                   static_cast< UInt8 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT32T2_FRACTION:
        {
            return TUInt8Fraction( static_cast< UInt8 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator ),
                                   static_cast< UInt8 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_INT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TInt64T2Fraction ) )
            {
                return TUInt8Fraction( static_cast< UInt8 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->numerator ),
                                       static_cast< UInt8 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_UINT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TUInt64T2Fraction ) )
            {
                return TUInt8Fraction( static_cast< UInt8 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->numerator ),
                                       static_cast< UInt8 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_INT64_INT32_FRACTION:
        {
            return TUInt8Fraction( static_cast< UInt8 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.numerator ),
                                   static_cast< UInt8 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION:
        {
            return TUInt8Fraction( static_cast< UInt8 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.numerator ),
                                   static_cast< UInt8 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.denominator ) );
        }
        case GUCEF_DATATYPE_FLOAT32:
        {
            return TUInt8Fraction::ConvertFloat32ToFraction( AsFloat32() );
        }
        case GUCEF_DATATYPE_FLOAT64:
        {
            return TUInt8Fraction::ConvertFloat64ToFraction( AsFloat64() );
        }
        default:
        {
            if ( IsInteger() )
                return TUInt8Fraction( AsUInt8(), 1 );
            return defaultIfNeeded;
        }
    }
}

/*-------------------------------------------------------------------------*/

TInt16Fraction
CVariant::AsInt16Fraction( const TInt16Fraction& defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT32T2_FRACTION:
        {
            return TInt16Fraction( static_cast< Int16 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator ),
                                  static_cast< Int16 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT32T2_FRACTION:
        {
            return TInt16Fraction( static_cast< Int16 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator ),
                                  static_cast< Int16 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_INT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TInt64T2Fraction ) )
            {
                return TInt16Fraction( static_cast< Int16 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->numerator ),
                                      static_cast< Int16 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_UINT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TUInt64T2Fraction ) )
            {
                return TInt16Fraction( static_cast< Int16 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->numerator ),
                                      static_cast< Int16 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_INT64_INT32_FRACTION:
        {
            return TInt16Fraction( static_cast< Int16 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.numerator ),
                                  static_cast< Int16 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION:
        {
            return TInt16Fraction( static_cast< Int16 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.numerator ),
                                  static_cast< Int16 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.denominator ) );
        }
        case GUCEF_DATATYPE_FLOAT32:
        {
            return TInt16Fraction::ConvertFloat32ToFraction( AsFloat32() );
        }
        case GUCEF_DATATYPE_FLOAT64:
        {
            return TInt16Fraction::ConvertFloat64ToFraction( AsFloat64() );
        }
        default:
        {
            if ( IsInteger() )
                return TInt16Fraction( AsInt16(), 1 );
            return defaultIfNeeded;
        }
    }
}

/*-------------------------------------------------------------------------*/

TUInt16Fraction
CVariant::AsUInt16Fraction( const TUInt16Fraction& defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT32T2_FRACTION:
        {
            return TUInt16Fraction( static_cast< UInt16 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator ),
                                   static_cast< UInt16 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT32T2_FRACTION:
        {
            return TUInt16Fraction( static_cast< UInt16 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator ),
                                   static_cast< UInt16 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_INT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TInt64T2Fraction ) )
            {
                return TUInt16Fraction( static_cast< UInt16 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->numerator ),
                                       static_cast< UInt16 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_UINT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TUInt64T2Fraction ) )
            {
                return TUInt16Fraction( static_cast< UInt16 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->numerator ),
                                       static_cast< UInt16 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_INT64_INT32_FRACTION:
        {
            return TUInt16Fraction( static_cast< UInt16 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.numerator ),
                                   static_cast< UInt16 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION:
        {
            return TUInt16Fraction( static_cast< UInt16 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.numerator ),
                                   static_cast< UInt16 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.denominator ) );
        }
        case GUCEF_DATATYPE_FLOAT32:
        {
            return TUInt16Fraction::ConvertFloat32ToFraction( AsFloat32() );
        }
        case GUCEF_DATATYPE_FLOAT64:
        {
            return TUInt16Fraction::ConvertFloat64ToFraction( AsFloat64() );
        }
        default:
        {
            if ( IsInteger() )
                return TUInt16Fraction( AsUInt16(), 1 );
            return defaultIfNeeded;
        }
    }
}

/*-------------------------------------------------------------------------*/

TInt32Fraction
CVariant::AsInt32Fraction( const TInt32Fraction& defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT32T2_FRACTION:
        {
            return TInt32Fraction( static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator ),
                                   static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT32T2_FRACTION:
        {
            return TInt32Fraction( static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator ),
                                   static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_INT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TInt64T2Fraction ) )
            {
                return TInt32Fraction( static_cast< Int32 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->numerator ),
                                       static_cast< Int32 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_UINT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TUInt64T2Fraction ) )
            {
                return TInt32Fraction( static_cast< Int32 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->numerator ),
                                       static_cast< Int32 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_INT64_INT32_FRACTION:
        {
            return TInt32Fraction( static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.numerator ),
                                   static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION:
        {
            return TInt32Fraction( static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.numerator ),
                                   static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.denominator ) );
        }
        case GUCEF_DATATYPE_FLOAT32:
        {
            return TInt32Fraction::ConvertFloat32ToFraction( AsFloat32() );
        }
        case GUCEF_DATATYPE_FLOAT64:
        {
            return TInt32Fraction::ConvertFloat64ToFraction( AsFloat64() );
        }
        default:
        {
            if ( IsInteger() )
                return TInt32Fraction( AsInt32(), 1 );
            return defaultIfNeeded;
        }
    }
}

/*-------------------------------------------------------------------------*/

TUInt32Fraction
CVariant::AsUInt32Fraction( const TUInt32Fraction& defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT32T2_FRACTION:
        {
            return TUInt32Fraction( static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator ),
                                    static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT32T2_FRACTION:
        {
            return TUInt32Fraction( static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator ),
                                    static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_INT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TInt64T2Fraction ) )
            {
                return TUInt32Fraction( static_cast< UInt32 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->numerator ),
                                        static_cast< UInt32 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_UINT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TUInt64T2Fraction ) )
            {
                return TUInt32Fraction( static_cast< UInt32 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->numerator ),
                                        static_cast< UInt32 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_INT64_INT32_FRACTION:
        {
            return TUInt32Fraction( static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.numerator ),
                                    static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION:
        {
            return TUInt32Fraction( static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.numerator ),
                                    static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.denominator ) );
        }
        case GUCEF_DATATYPE_FLOAT32:
        {
            return TUInt32Fraction::ConvertFloat32ToFraction( AsFloat32() );
        }
        case GUCEF_DATATYPE_FLOAT64:
        {
            return TUInt32Fraction::ConvertFloat64ToFraction( AsFloat64() );
        }
        default:
        {
            if ( IsInteger() )
                return TUInt32Fraction( AsUInt32(), 1 );
            return defaultIfNeeded;
        }
    }
}

/*-------------------------------------------------------------------------*/

TInt64Fraction
CVariant::AsInt64Fraction( const TInt64Fraction& defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT32T2_FRACTION:
        {
            return TInt64Fraction( static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator ),
                                   static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT32T2_FRACTION:
        {
            return TInt64Fraction( static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator ),
                                   static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_INT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TInt64T2Fraction ) )
            {
                return TInt64Fraction( static_cast< Int32 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->numerator ),
                                       static_cast< Int32 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_UINT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TUInt64T2Fraction ) )
            {
                return TInt64Fraction( static_cast< Int32 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->numerator ),
                                       static_cast< Int32 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_INT64_INT32_FRACTION:
        {
            return TInt64Fraction( static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.numerator ),
                                   static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION:
        {
            return TInt64Fraction( static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.numerator ),
                                   static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.denominator ) );
        }
        case GUCEF_DATATYPE_FLOAT32:
        {
            return TInt64Fraction::ConvertFloat32ToFraction( AsFloat32() );
        }
        case GUCEF_DATATYPE_FLOAT64:
        {
            return TInt64Fraction::ConvertFloat64ToFraction( AsFloat64() );
        }
        default:
        {
            if ( IsInteger() )
                return TInt64Fraction( AsInt64(), 1 );
            return defaultIfNeeded;
        }
    }
}

/*-------------------------------------------------------------------------*/

TUInt64Fraction
CVariant::AsUInt64Fraction( const TUInt64Fraction& defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT32T2_FRACTION:
        {
            return TUInt64Fraction( static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator ),
                                   static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT32T2_FRACTION:
        {
            return TUInt64Fraction( static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator ),
                                   static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_INT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TInt64T2Fraction ) )
            {
                return TUInt64Fraction( static_cast< UInt32 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->numerator ),
                                       static_cast< UInt32 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_UINT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TUInt64T2Fraction ) )
            {
                return TUInt64Fraction( static_cast< UInt32 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->numerator ),
                                       static_cast< UInt32 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_INT64_INT32_FRACTION:
        {
            return TUInt64Fraction( static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.numerator ),
                                   static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION:
        {
            return TUInt64Fraction( static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.numerator ),
                                   static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.denominator ) );
        }
        case GUCEF_DATATYPE_FLOAT32:
        {
            return TUInt64Fraction::ConvertFloat32ToFraction( AsFloat32() );
        }
        case GUCEF_DATATYPE_FLOAT64:
        {
            return TUInt64Fraction::ConvertFloat64ToFraction( AsFloat64() );
        }
        default:
        {
            if ( IsInteger() )
                return TUInt64Fraction( AsUInt64(), 1 );
            return defaultIfNeeded;
        }
    }
}

/*-------------------------------------------------------------------------*/

TInt64Int32Fraction
CVariant::AsInt64Int32Fraction( const TInt64Int32Fraction& defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT32T2_FRACTION:
        {
            return TInt64Int32Fraction( static_cast< Int64 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator ),
                                        static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT32T2_FRACTION:
        {
            return TInt64Int32Fraction( static_cast< Int64 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator ),
                                        static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_INT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TInt64T2Fraction ) )
            {
                return TInt64Int32Fraction( static_cast< Int64 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->numerator ),
                                            static_cast< Int32 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_UINT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TUInt64T2Fraction ) )
            {
                return TInt64Int32Fraction( static_cast< Int64 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->numerator ),
                                            static_cast< Int32 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_INT64_INT32_FRACTION:
        {
            return TInt64Int32Fraction( static_cast< Int64 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.numerator ),
                                        static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION:
        {
            return TInt64Int32Fraction( static_cast< Int64 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.numerator ),
                                        static_cast< Int32 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.denominator ) );
        }
        case GUCEF_DATATYPE_FLOAT32:
        {
            return TInt64Int32Fraction::ConvertFloat32ToFraction( AsFloat32() );
        }
        case GUCEF_DATATYPE_FLOAT64:
        {
            return TInt64Int32Fraction::ConvertFloat64ToFraction( AsFloat64() );
        }
        default:
        {
            if ( IsInteger() )
                return TInt64Int32Fraction( AsInt64(), 1 );
            return defaultIfNeeded;
        }
    }
}

/*-------------------------------------------------------------------------*/

TUInt64UInt32Fraction
CVariant::AsUInt64UInt32Fraction( const TUInt64UInt32Fraction& defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT32T2_FRACTION:
        {
            return TUInt64UInt32Fraction( static_cast< UInt64 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator ),
                                          static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT32T2_FRACTION:
        {
            return TUInt64UInt32Fraction( static_cast< UInt64 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator ),
                                          static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator ) );
        }
        case GUCEF_DATATYPE_INT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TInt64T2Fraction ) )
            {
                return TUInt64UInt32Fraction( static_cast< UInt64 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->numerator ),
                                              static_cast< UInt32 >( m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_UINT64T2_FRACTION:
        {
            if ( m_variantData.union_data.heap_data.union_data.void_heap_data != GUCEF_NULL   &&
                 m_variantData.union_data.heap_data.heap_data_size >= sizeof( TUInt64T2Fraction ) )
            {
                return TUInt64UInt32Fraction( static_cast< UInt64 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->numerator ),
                                              static_cast< UInt32 >( m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->denominator ) );
            }
            return defaultIfNeeded;
        }
        case GUCEF_DATATYPE_INT64_INT32_FRACTION:
        {
            return TUInt64UInt32Fraction( static_cast< UInt64 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.numerator ),
                                          static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.int64_int32_data.denominator ) );
        }
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION:
        {
            return TUInt64UInt32Fraction( static_cast< UInt64 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.numerator ),
                                          static_cast< UInt32 >( m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.denominator ) );
        }
        case GUCEF_DATATYPE_FLOAT32:
        {
            return TUInt64UInt32Fraction::ConvertFloat32ToFraction( AsFloat32() );
        }
        case GUCEF_DATATYPE_FLOAT64:
        {
            return TUInt64UInt32Fraction::ConvertFloat64ToFraction( AsFloat64() );
        }
        default:
        {
            if ( IsInteger() )
                return TUInt64UInt32Fraction( AsInt64(), 1 );
            return defaultIfNeeded;
        }
    }
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CVariant::GetDecimal( CDecimal& result, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    switch ( m_variantData.containedType )
    {
        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
            return DecimalFromVariantPayload( m_variantData, result );

        case GUCEF_DATATYPE_INT8:
        case GUCEF_DATATYPE_UINT8:
        case GUCEF_DATATYPE_INT16:
        case GUCEF_DATATYPE_UINT16:
        case GUCEF_DATATYPE_INT32:
        case GUCEF_DATATYPE_UINT32:
        case GUCEF_DATATYPE_INT64:
            return CDecimal::FromInt64( AsInt64(), 0, result );

        case GUCEF_DATATYPE_UINT64:
        {
            UInt64 value = m_variantData.union_data.uint64_data;
            if ( value > (UInt64) GUCEF_INT64MAX )
                return DECIMALSTATUS_OVERFLOW;
            return CDecimal::FromInt64( (Int64) value, 0, result );
        }

        case GUCEF_DATATYPE_BOOLEAN_INT32:
            return CDecimal::FromInt64( 0 != m_variantData.union_data.int32_data ? 1 : 0, 0, result );

        case GUCEF_DATATYPE_FLOAT32:
            return CDecimal::FromFloat32Shortest( m_variantData.union_data.float32_data, result );
        case GUCEF_DATATYPE_FLOAT64:
            return CDecimal::FromFloat64Shortest( m_variantData.union_data.float64_data, result );

        case GUCEF_DATATYPE_ASCII_STRING:
        case GUCEF_DATATYPE_UTF8_STRING:
        case GUCEF_DATATYPE_UTF16_LE_STRING:
        case GUCEF_DATATYPE_UTF16_BE_STRING:
        case GUCEF_DATATYPE_UTF32_LE_STRING:
        case GUCEF_DATATYPE_UTF32_BE_STRING:
            return CDecimal::FromString( AsString( CString::Empty, resolveVarsIfApplicable ), result );

        case GUCEF_DATATYPE_INT32T2_FRACTION:
        {
            Int64 numerator = m_variantData.union_data.fraction_data.union_data.int32t2_data.numerator;
            Int64 denominator = m_variantData.union_data.fraction_data.union_data.int32t2_data.denominator;
            return DecimalFromFraction( 0 != numerator && ( numerator < 0 ) != ( denominator < 0 ),
                                        numerator < 0 ? (UInt64) -numerator : (UInt64) numerator,
                                        denominator < 0 ? (UInt64) -denominator : (UInt64) denominator, result );
        }
        case GUCEF_DATATYPE_UINT32T2_FRACTION:
        {
            return DecimalFromFraction( false, m_variantData.union_data.fraction_data.union_data.uint32t2_data.numerator,
                                        m_variantData.union_data.fraction_data.union_data.uint32t2_data.denominator, result );
        }
        case GUCEF_DATATYPE_INT64_INT32_FRACTION:
        {
            Int64 numerator = m_variantData.union_data.fraction_data.union_data.int64_int32_data.numerator;
            Int64 denominator = m_variantData.union_data.fraction_data.union_data.int64_int32_data.denominator;
            return DecimalFromFraction( 0 != numerator && ( numerator < 0 ) != ( denominator < 0 ),
                                        numerator < 0 ? 0 - (UInt64) numerator : (UInt64) numerator,
                                        denominator < 0 ? (UInt64) -denominator : (UInt64) denominator, result );
        }
        case GUCEF_DATATYPE_UINT64_UINT32_FRACTION:
        {
            return DecimalFromFraction( false, m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.numerator,
                                        m_variantData.union_data.fraction_data.union_data.uint64_uint32_data.denominator, result );
        }
        case GUCEF_DATATYPE_INT64T2_FRACTION:
        {
            if ( GUCEF_NULL == m_variantData.union_data.heap_data.union_data.void_heap_data ||
                 sizeof( TInt64T2Fraction ) > m_variantData.union_data.heap_data.heap_data_size )
                return DECIMALSTATUS_INVALID_OPERAND;
            Int64 numerator = m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->numerator;
            Int64 denominator = m_variantData.union_data.heap_data.union_data.fraction_int64t2_data->denominator;
            return DecimalFromFraction( 0 != numerator && ( numerator < 0 ) != ( denominator < 0 ),
                                        numerator < 0 ? 0 - (UInt64) numerator : (UInt64) numerator,
                                        denominator < 0 ? 0 - (UInt64) denominator : (UInt64) denominator, result );
        }
        case GUCEF_DATATYPE_UINT64T2_FRACTION:
        {
            if ( GUCEF_NULL == m_variantData.union_data.heap_data.union_data.void_heap_data ||
                 sizeof( TUInt64T2Fraction ) > m_variantData.union_data.heap_data.heap_data_size )
                return DECIMALSTATUS_INVALID_OPERAND;
            return DecimalFromFraction( false, m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->numerator,
                                        m_variantData.union_data.heap_data.union_data.fraction_uint64t2_data->denominator, result );
        }

        default:
            return DECIMALSTATUS_INVALID_OPERAND;
    }
}

/*-------------------------------------------------------------------------*/

CDecimal
CVariant::AsDecimal( const CDecimal& defaultIfNeeded, bool resolveVarsIfApplicable ) const
{GUCEF_TRACE;

    CDecimal result;
    if ( DecimalStatusIsError( GetDecimal( result, resolveVarsIfApplicable ) ) )
        return defaultIfNeeded;
    return result;
}

/*-------------------------------------------------------------------------*/

CVariant::operator CAsciiString() const
{GUCEF_TRACE;

    return AsAsciiString();
}

/*-------------------------------------------------------------------------*/

CVariant::operator CUtf8String() const
{GUCEF_TRACE;

    return AsUtf8String();
}

/*-------------------------------------------------------------------------*/

CVariant::operator CUtf16String() const
{GUCEF_TRACE;

    return AsUtf16String();
}

/*-------------------------------------------------------------------------*/

CVariant::operator CUtf32String() const
{GUCEF_TRACE;

    return AsUtf32String();
}

/*-------------------------------------------------------------------------*/

CString::StringVector
ToStringVector( const CVariant::VariantVector& var )
{GUCEF_TRACE;

    CString::StringVector result;
    result.reserve( var.size() );
    CVariant::VariantVector::const_iterator i = var.begin();
    while ( i != var.end() )
    {
        result.push_back( (*i).AsString() );
        ++i;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CString::StringVector
ToStringVector( const CVariant::VariantSet& var )
{GUCEF_TRACE;

    CString::StringVector result;
    result.reserve( var.size() );
    CVariant::VariantSet::const_iterator i = var.begin();
    while ( i != var.end() )
    {
        result.push_back( (*i).AsString() );
        ++i;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CString::StringSet
ToStringSet( const CVariant::VariantVector& var )
{GUCEF_TRACE;

    CString::StringSet result;
    CVariant::VariantVector::const_iterator i = var.begin();
    while ( i != var.end() )
    {
        result.insert( (*i).AsString() );
        ++i;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CString::StringSet
ToStringSet( const CVariant::VariantSet& var )
{GUCEF_TRACE;

    CString::StringSet result;
    CVariant::VariantSet::const_iterator i = var.begin();
    while ( i != var.end() )
    {
        result.insert( (*i).AsString() );
        ++i;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CString::StringMap 
ToStringMap( const CVariant::VariantMap& var )
{GUCEF_TRACE;

    CString::StringMap result;
    CVariant::VariantMap::const_iterator i = var.begin();
    while ( i != var.end() )
    {
        result[ (*i).first.AsString() ] = (*i).second.AsString();
        ++i;
    }
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
