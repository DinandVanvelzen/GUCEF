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

#include <string.h>
#include <math.h>
#include <new>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATE_H
#include "gucefCORE_CDate.h"
#define GUCEF_CORE_CDATE_H
#endif /* GUCEF_CORE_CDATE_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#include "gucefCORE_CDataTableBatchBuilder.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The offsets of a variable column without rows */
const UInt32 g_emptyOffsets[ 2 ] = { 0, 0 };

/** 10^p - 1 for p = 0..18: the largest mantissa of each decimal precision */
const Int64 g_maxMantissa[ 19 ] =
{
    0LL, 9LL, 99LL, 999LL, 9999LL, 99999LL, 999999LL, 9999999LL, 99999999LL, 999999999LL,
    9999999999LL, 99999999999LL, 999999999999LL, 9999999999999LL, 99999999999999LL, 999999999999999LL,
    9999999999999999LL, 99999999999999999LL, 999999999999999999LL
};

inline void
SetBit( UInt8* bits, UInt32 index, bool value )
{
    const UInt8 mask = (UInt8) ( 1u << ( index & 7u ) );
    if ( value )
        bits[ index >> 3 ] |= mask;
    else
        bits[ index >> 3 ] &= (UInt8) ~mask;
}

inline bool
GetBit( const UInt8* bits, UInt32 index )
{
    return 0 != ( bits[ index >> 3 ] & ( 1u << ( index & 7u ) ) );
}

/** Clears the bits of the last byte beyond bitCount */
inline void
ClearTrailingBits( UInt8* bits, UInt32 bitCount )
{
    if ( 0 != ( bitCount & 7u ) )
        bits[ bitCount >> 3 ] &= (UInt8) ( ( 1u << ( bitCount & 7u ) ) - 1u );
}

inline bool
IsDateOrTimestampTypeId( UInt8 typeId )
{
    switch ( typeId )
    {
        case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
            return true;
        default:
            return false;
    }
}

/** A float that holds an integer exactly converted to Int64 */
inline bool
FloatToInt64Exactly( Float64 value, Int64& result )
{
    // 2^63 is the first double above the Int64 range
    if ( value != value || value < -9223372036854775808.0 || value >= 9223372036854775808.0 || floor( value ) != value )
        return false;
    result = (Int64) value;
    return true;
}

/** Text of either string class into a UTF8 column, without a copy when the string is UTF-8 or ASCII already */
inline bool
AppendTextAsUtf8( CDataTableColumnBuilder& builder, const CUtf8String& text )
{
    return builder.AppendUtf8( text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0 );
}

inline bool
AppendTextAsUtf8( CDataTableColumnBuilder& builder, const CAsciiString& text )
{
    return builder.AppendUtf8( text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0 );
}

template < class StringType >
inline bool
AppendTextAsUtf8( CDataTableColumnBuilder& builder, const StringType& text )
{
    CUtf8String utf8 = ToUtf8String( text );
    return builder.AppendUtf8( utf8.C_String(), utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0 );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataTableColumnBuilder::CDataTableColumnBuilder( void )
    : m_column()
    , m_pool( GUCEF_NULL )
    , m_storage( DATATABLE_STORAGE_NONE )
    , m_width( 0 )
    , m_rowCount( 0 )
    , m_nullCount( 0 )
    , m_byteCount( 0 )
    , m_lastStatus( DATATABLE_STATUS_OK )
    , m_values()
    , m_bytes()
    , m_validity()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableColumnBuilder::CDataTableColumnBuilder( const CDataTableColumnDef& column, CDataTableBufferPool* pool )
    : m_column( column )
    , m_pool( pool )
    , m_storage( DataTableLogicalTypeStorage( column.type ) )
    , m_width( DataTableLogicalTypeByteWidth( column.type ) )
    , m_rowCount( 0 )
    , m_nullCount( 0 )
    , m_byteCount( 0 )
    , m_lastStatus( DATATABLE_STATUS_OK )
    , m_values()
    , m_bytes()
    , m_validity()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableColumnBuilder::~CDataTableColumnBuilder()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::Fail( TDataTableStatus status )
{

    // a builder without a column (an index out of range) takes nothing
    m_lastStatus = DATATABLE_TYPE_UNSUPPORTED == m_column.type ? DATATABLE_STATUS_INVALID_ARGUMENT : status;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::PrepareRow( void )
{

    if ( DATATABLE_STORAGE_NONE == m_storage || GUCEF_NULL == m_pool )
        return Fail( DATATABLE_STATUS_INVALID_ARGUMENT );
    if ( m_rowCount >= GUCEF_UINT32MAX - 2 )
        return Fail( DATATABLE_STATUS_LIMIT_EXCEEDED );

    UInt32 usedBytes = 0;
    UInt32 requiredBytes = 0;
    switch ( m_storage )
    {
        case DATATABLE_STORAGE_FIXED:       { usedBytes = m_rowCount * m_width; requiredBytes = usedBytes + m_width; break; }
        case DATATABLE_STORAGE_BITS:        { usedBytes = ( m_rowCount + 7 ) >> 3; requiredBytes = ( m_rowCount >> 3 ) + 1; break; }
        case DATATABLE_STORAGE_VARIABLE:    { usedBytes = ( m_rowCount + 1 ) * sizeof( UInt32 ); requiredBytes = usedBytes + sizeof( UInt32 ); break; }
        default:                            { return Fail( DATATABLE_STATUS_INVALID_ARGUMENT ); }
    }
    if ( DATATABLE_STORAGE_FIXED == m_storage && (UInt64) m_rowCount * m_width + m_width > GUCEF_UINT32MAX - DataTableBufferPadding )
        return Fail( DATATABLE_STATUS_LIMIT_EXCEEDED );

    const bool first = m_values.IsNULL();
    if ( first || m_values->GetCapacity() < requiredBytes )
    {
        if ( !first )
            m_values->SetDataSize( usedBytes );
        TDataTableStatus status = m_pool->Reserve( m_values, requiredBytes );
        if ( DATATABLE_STATUS_OK != status )
            return Fail( status );
    }
    if ( DATATABLE_STORAGE_VARIABLE == m_storage && 0 == m_rowCount )
        static_cast< UInt32* >( m_values->GetData() )[ 0 ] = 0;

    if ( !m_validity.IsNULL() && m_validity->GetCapacity() < ( m_rowCount >> 3 ) + 1 )
    {
        m_validity->SetDataSize( ( m_rowCount + 7 ) >> 3 );
        TDataTableStatus status = m_pool->Reserve( m_validity, ( m_rowCount >> 3 ) + 1 );
        if ( DATATABLE_STATUS_OK != status )
            return Fail( status );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::MarkPresent( void )
{

    if ( !m_validity.IsNULL() )
        SetBit( static_cast< UInt8* >( m_validity->GetData() ), m_rowCount, true );
    ++m_rowCount;
    m_lastStatus = DATATABLE_STATUS_OK;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::CreateValidity( void )
{

    // every row so far is present
    TDataTableStatus status = m_pool->Acquire( ( m_rowCount >> 3 ) + 1, m_validity );
    if ( DATATABLE_STATUS_OK != status )
        return Fail( status );
    UInt8* bits = static_cast< UInt8* >( m_validity->GetData() );
    const UInt32 fullBytes = m_rowCount >> 3;
    memset( bits, 0xFF, fullBytes );
    bits[ fullBytes ] = (UInt8) ( ( 1u << ( m_rowCount & 7u ) ) - 1u );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendFixed( const void* value )
{

    if ( !PrepareRow() )
        return false;
    memcpy( static_cast< char* >( m_values->GetData() ) + (UInt64) m_rowCount * m_width, value, m_width );
    return MarkPresent();
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendVariable( const char* bytes, UInt32 byteLength )
{

    if ( DATATABLE_STORAGE_VARIABLE != m_storage )
        return Fail( DATATABLE_STATUS_TYPE_MISMATCH );
    if ( (UInt64) m_byteCount + byteLength + 1 > (UInt64) GUCEF_UINT32MAX - DataTableBufferPadding )
        return Fail( DATATABLE_STATUS_LIMIT_EXCEEDED );
    if ( !PrepareRow() )
        return false;

    const UInt32 required = m_byteCount + byteLength + 1;
    if ( m_bytes.IsNULL() || m_bytes->GetCapacity() < required )
    {
        if ( !m_bytes.IsNULL() )
            m_bytes->SetDataSize( m_byteCount );
        TDataTableStatus status = m_pool->Reserve( m_bytes, required );
        if ( DATATABLE_STATUS_OK != status )
            return Fail( status );
    }
    char* target = static_cast< char* >( m_bytes->GetData() ) + m_byteCount;
    if ( byteLength > 0 )
        memcpy( target, bytes, byteLength );
    target[ byteLength ] = '\0';
    m_byteCount = required;
    static_cast< UInt32* >( m_values->GetData() )[ m_rowCount + 1 ] = m_byteCount;
    return MarkPresent();
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendNull( void )
{

    if ( !m_column.nullable )
        return Fail( DATATABLE_STATUS_TYPE_MISMATCH );
    if ( !PrepareRow() )
        return false;
    if ( m_validity.IsNULL() && !CreateValidity() )
        return false;

    // null slots are zero
    switch ( m_storage )
    {
        case DATATABLE_STORAGE_FIXED:
        {
            memset( static_cast< char* >( m_values->GetData() ) + (UInt64) m_rowCount * m_width, 0, m_width );
            break;
        }
        case DATATABLE_STORAGE_BITS:
        {
            SetBit( static_cast< UInt8* >( m_values->GetData() ), m_rowCount, false );
            break;
        }
        case DATATABLE_STORAGE_VARIABLE:
        {
            const UInt32 required = m_byteCount + 1;
            if ( m_bytes.IsNULL() || m_bytes->GetCapacity() < required )
            {
                if ( !m_bytes.IsNULL() )
                    m_bytes->SetDataSize( m_byteCount );
                TDataTableStatus status = m_pool->Reserve( m_bytes, required );
                if ( DATATABLE_STATUS_OK != status )
                    return Fail( status );
            }
            static_cast< char* >( m_bytes->GetData() )[ m_byteCount ] = '\0';
            m_byteCount = required;
            static_cast< UInt32* >( m_values->GetData() )[ m_rowCount + 1 ] = m_byteCount;
            break;
        }
        default:
        {
            return Fail( DATATABLE_STATUS_INVALID_ARGUMENT );
        }
    }
    SetBit( static_cast< UInt8* >( m_validity->GetData() ), m_rowCount, false );
    ++m_nullCount;
    ++m_rowCount;
    m_lastStatus = DATATABLE_STATUS_OK;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendBool( bool value )
{

    if ( DATATABLE_TYPE_BOOL != m_column.type )
        return Fail( DATATABLE_STATUS_TYPE_MISMATCH );
    if ( !PrepareRow() )
        return false;
    SetBit( static_cast< UInt8* >( m_values->GetData() ), m_rowCount, value );
    return MarkPresent();
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendIntegerChecked( Int64 signedValue, UInt64 unsignedValue, bool isUnsigned )
{

    // the value as a signed and as an unsigned 64 bit number, with flags for whether each form is exact
    const bool fitsSigned = !isUnsigned || unsignedValue <= (UInt64) GUCEF_INT64MAX;
    const bool fitsUnsigned = isUnsigned || signedValue >= 0;
    const Int64 s = isUnsigned ? (Int64) unsignedValue : signedValue;
    const UInt64 u = isUnsigned ? unsignedValue : (UInt64) signedValue;

    switch ( m_column.type )
    {
        case DATATABLE_TYPE_INT8:   { if ( !fitsSigned || s < -128 || s > 127 ) break; Int8 v = (Int8) s; return AppendFixed( &v ); }
        case DATATABLE_TYPE_INT16:  { if ( !fitsSigned || s < -32768 || s > 32767 ) break; Int16 v = (Int16) s; return AppendFixed( &v ); }
        case DATATABLE_TYPE_INT32:  { if ( !fitsSigned || s < GUCEF_INT32MIN || s > GUCEF_INT32MAX ) break; Int32 v = (Int32) s; return AppendFixed( &v ); }
        case DATATABLE_TYPE_INT64:  { if ( !fitsSigned ) break; return AppendFixed( &s ); }
        case DATATABLE_TYPE_UINT8:  { if ( !fitsUnsigned || u > 255 ) break; UInt8 v = (UInt8) u; return AppendFixed( &v ); }
        case DATATABLE_TYPE_UINT16: { if ( !fitsUnsigned || u > 65535 ) break; UInt16 v = (UInt16) u; return AppendFixed( &v ); }
        case DATATABLE_TYPE_UINT32: { if ( !fitsUnsigned || u > GUCEF_UINT32MAX ) break; UInt32 v = (UInt32) u; return AppendFixed( &v ); }
        case DATATABLE_TYPE_UINT64: { if ( !fitsUnsigned ) break; return AppendFixed( &u ); }
        default:                    { break; }
    }
    return Fail( DATATABLE_STATUS_TYPE_MISMATCH );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendInt64( Int64 value )
{

    return AppendIntegerChecked( value, 0, false );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendUInt64( UInt64 value )
{

    return AppendIntegerChecked( 0, value, true );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendFloat64( Float64 value )
{

    if ( DATATABLE_TYPE_FLOAT64 == m_column.type )
        return AppendFixed( &value );
    if ( DATATABLE_TYPE_FLOAT32 == m_column.type )
    {
        const Float32 narrow = (Float32) value;
        if ( value == value && (Float64) narrow != value )
            return Fail( DATATABLE_STATUS_TYPE_MISMATCH );
        return AppendFixed( &narrow );
    }
    return Fail( DATATABLE_STATUS_TYPE_MISMATCH );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendDecimal( const CDecimal& value )
{

    if ( DATATABLE_TYPE_DECIMAL != m_column.type )
        return Fail( DATATABLE_STATUS_TYPE_MISMATCH );
    CDecimal atColumnScale;
    if ( DecimalStatusIsError( value.Rescale( m_column.decimalScale, ROUNDINGMODE_UNNECESSARY, atColumnScale ) ) )
        return Fail( DATATABLE_STATUS_TYPE_MISMATCH );
    return AppendDecimalMantissa( atColumnScale.GetUnscaledValue() );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendDecimalMantissa( Int64 mantissaAtColumnScale )
{

    if ( DATATABLE_TYPE_DECIMAL != m_column.type || m_column.decimalPrecision > DataTableMaxDecimalPrecision )
        return Fail( DATATABLE_STATUS_TYPE_MISMATCH );
    const Int64 limit = g_maxMantissa[ m_column.decimalPrecision ];
    if ( mantissaAtColumnScale > limit || mantissaAtColumnScale < -limit )
        return Fail( DATATABLE_STATUS_TYPE_MISMATCH );
    return AppendFixed( &mantissaAtColumnScale );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendDate( Int32 daysSinceEpoch )
{

    if ( DATATABLE_TYPE_DATE != m_column.type )
        return Fail( DATATABLE_STATUS_TYPE_MISMATCH );
    return AppendFixed( &daysSinceEpoch );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendDate( const CDate& date )
{

    if ( !date.IsValid() )
        return Fail( DATATABLE_STATUS_TYPE_MISMATCH );
    return AppendDate( date.ToDaysSinceUnixEpoch() );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendTimestamp( Int64 ticksInColumnUnit )
{

    if ( DATATABLE_TYPE_TIMESTAMP != m_column.type )
        return Fail( DATATABLE_STATUS_TYPE_MISMATCH );
    return AppendFixed( &ticksInColumnUnit );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendDateTime( const CDateTime& dateTime )
{

    Int64 ticks = 0;
    if ( DATATABLE_TYPE_TIMESTAMP != m_column.type || !CDataTableValues::TicksFromDateTime( dateTime, m_column.timeUnit, ticks ) )
        return Fail( DATATABLE_STATUS_TYPE_MISMATCH );
    return AppendFixed( &ticks );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendUtf8( const char* bytes, UInt32 byteLength )
{

    if ( DATATABLE_TYPE_UTF8 != m_column.type || ( GUCEF_NULL == bytes && byteLength > 0 ) )
        return Fail( DATATABLE_STATUS_TYPE_MISMATCH );
    return AppendVariable( bytes, byteLength );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendString( const CString& value )
{

    return AppendTextAsUtf8( *this, value );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendBinary( const void* bytes, UInt32 byteLength )
{

    if ( DATATABLE_TYPE_BINARY != m_column.type || ( GUCEF_NULL == bytes && byteLength > 0 ) )
        return Fail( DATATABLE_STATUS_TYPE_MISMATCH );
    return AppendVariable( static_cast< const char* >( bytes ), byteLength );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::Append( const CVariant& value )
{

    const UInt8 typeId = value.GetTypeId();
    if ( value.IsNULL() || GUCEF_DATATYPE_UNKNOWN == typeId )
        return AppendNull();

    const bool isDateOrTimestamp = IsDateOrTimestampTypeId( typeId );
    const bool isPlainInteger = value.IsInteger() && !isDateOrTimestamp;

    switch ( m_column.type )
    {
        case DATATABLE_TYPE_BOOL:
        {
            if ( value.IsBoolean() )
                return AppendBool( value.AsBool() );
            if ( isPlainInteger )
            {
                CVariant native( value );
                native.ToNativeEndian();
                if ( native.IsSignedInteger() ? ( 0 == native.AsInt64() || 1 == native.AsInt64() ) : native.AsUInt64() <= 1 )
                    return AppendBool( 0 != native.AsUInt64() );
            }
            break;
        }
        case DATATABLE_TYPE_INT8:
        case DATATABLE_TYPE_INT16:
        case DATATABLE_TYPE_INT32:
        case DATATABLE_TYPE_INT64:
        case DATATABLE_TYPE_UINT8:
        case DATATABLE_TYPE_UINT16:
        case DATATABLE_TYPE_UINT32:
        case DATATABLE_TYPE_UINT64:
        {
            if ( isPlainInteger )
            {
                CVariant native( value );
                native.ToNativeEndian();
                if ( native.IsUnsignedInteger() )
                    return AppendUInt64( native.AsUInt64() );
                return AppendInt64( native.AsInt64() );
            }
            if ( value.IsDecimal() )
            {
                CDecimal decimal;
                Int64 integral = 0;
                if ( DecimalStatusIsError( value.GetDecimal( decimal ) ) ||
                     DecimalStatusIsError( decimal.ToInt64( ROUNDINGMODE_UNNECESSARY, integral ) ) )
                    break;
                return AppendInt64( integral );
            }
            if ( value.IsFloat() )
            {
                Int64 integral = 0;
                if ( !FloatToInt64Exactly( value.AsFloat64(), integral ) )
                    break;
                return AppendInt64( integral );
            }
            break;
        }
        case DATATABLE_TYPE_FLOAT32:
        case DATATABLE_TYPE_FLOAT64:
        {
            if ( value.IsFloat() )
                return AppendFloat64( value.AsFloat64() );
            if ( isPlainInteger )
            {
                CVariant native( value );
                native.ToNativeEndian();
                if ( native.IsUnsignedInteger() )
                {
                    const UInt64 integer = native.AsUInt64();
                    const Float64 asFloat = (Float64) integer;
                    if ( asFloat < 18446744073709551616.0 && (UInt64) asFloat == integer )
                        return AppendFloat64( asFloat );
                }
                else
                {
                    const Int64 integer = native.AsInt64();
                    const Float64 asFloat = (Float64) integer;
                    Int64 back = 0;
                    if ( FloatToInt64Exactly( asFloat, back ) && back == integer )
                        return AppendFloat64( asFloat );
                }
                break;
            }
            if ( value.IsDecimal() )
            {
                CDecimal decimal;
                CDecimal check;
                if ( DecimalStatusIsError( value.GetDecimal( decimal ) ) )
                    break;
                const Float64 asFloat = decimal.ToFloat64();
                if ( DecimalStatusIsError( CDecimal::FromFloat64( asFloat, decimal.GetScale(), ROUNDINGMODE_UNNECESSARY, check ) ) || 0 != check.Compare( decimal ) )
                    break;
                return AppendFloat64( asFloat );
            }
            break;
        }
        case DATATABLE_TYPE_DECIMAL:
        {
            if ( value.IsDecimal() )
            {
                CDecimal decimal;
                if ( DecimalStatusIsError( value.GetDecimal( decimal ) ) )
                    break;
                return AppendDecimal( decimal );
            }
            if ( isPlainInteger )
            {
                CVariant native( value );
                native.ToNativeEndian();
                CDecimal decimal;
                if ( native.IsUnsignedInteger() && native.AsUInt64() > (UInt64) GUCEF_INT64MAX )
                    break;
                if ( DecimalStatusIsError( CDecimal::FromInt64( native.AsInt64(), 0, decimal ) ) )
                    break;
                return AppendDecimal( decimal );
            }
            break;
        }
        case DATATABLE_TYPE_DATE:
        {
            if ( GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == typeId || GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == typeId )
                return AppendDate( value.AsInt32() );
            break;
        }
        case DATATABLE_TYPE_TIMESTAMP:
        {
            Int64 ticks = 0;
            if ( CDataTableValues::TicksFromVariant( value, m_column.timeUnit, ticks ) )
                return AppendTimestamp( ticks );
            break;
        }
        case DATATABLE_TYPE_UTF8:
        {
            if ( GUCEF_DATATYPE_UTF8_STRING == typeId || GUCEF_DATATYPE_ASCII_STRING == typeId ||
                 GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING == typeId || GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING == typeId )
                return AppendUtf8( value.AsCharPtr(), value.ByteSize( false ) );
            if ( value.IsString() )
            {
                CUtf8String utf8 = value.AsUtf8String();
                return AppendUtf8( utf8.C_String(), utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0 );
            }
            break;
        }
        case DATATABLE_TYPE_BINARY:
        {
            if ( GUCEF_DATATYPE_BINARY_BLOB == typeId || GUCEF_DATATYPE_BINARY_BSOB == typeId )
                return AppendBinary( value.AsVoidPtr(), value.ByteSize() );
            if ( GUCEF_DATATYPE_UTF8_STRING == typeId || GUCEF_DATATYPE_ASCII_STRING == typeId )
                return AppendBinary( value.AsCharPtr(), value.ByteSize( false ) );
            break;
        }
        default:
        {
            return Fail( DATATABLE_STATUS_INVALID_ARGUMENT );
        }
    }
    return Fail( DATATABLE_STATUS_TYPE_MISMATCH );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendInt64s( const Int64* values, UInt32 count, const UInt8* validity )
{

    const UInt32 startRows = m_rowCount;
    const bool direct = GUCEF_NULL == validity && ( DATATABLE_TYPE_INT64 == m_column.type || DATATABLE_TYPE_TIMESTAMP == m_column.type );
    if ( direct && count > 0 )
    {
        // one copy of the whole block
        if ( (UInt64) ( m_rowCount + (UInt64) count ) * 8 > (UInt64) GUCEF_UINT32MAX - DataTableBufferPadding )
            return Fail( DATATABLE_STATUS_LIMIT_EXCEEDED );
        if ( !PrepareRow() )
            return false;
        const UInt32 required = ( m_rowCount + count ) * 8;
        if ( m_values->GetCapacity() < required )
        {
            m_values->SetDataSize( m_rowCount * 8 );
            TDataTableStatus status = m_pool->Reserve( m_values, required );
            if ( DATATABLE_STATUS_OK != status )
                return Fail( status );
        }
        if ( !m_validity.IsNULL() )
        {
            if ( m_validity->GetCapacity() < ( ( m_rowCount + count ) >> 3 ) + 1 )
            {
                m_validity->SetDataSize( ( m_rowCount + 7 ) >> 3 );
                TDataTableStatus status = m_pool->Reserve( m_validity, ( ( m_rowCount + count ) >> 3 ) + 1 );
                if ( DATATABLE_STATUS_OK != status )
                    return Fail( status );
            }
            UInt8* bits = static_cast< UInt8* >( m_validity->GetData() );
            for ( UInt32 i=0; i<count; ++i )
                SetBit( bits, m_rowCount + i, true );
        }
        memcpy( static_cast< char* >( m_values->GetData() ) + (UInt64) m_rowCount * 8, values, (size_t) count * 8 );
        m_rowCount += count;
        m_lastStatus = DATATABLE_STATUS_OK;
        return true;
    }

    for ( UInt32 i=0; i<count; ++i )
    {
        const bool ok = ( GUCEF_NULL != validity && !GetBit( validity, i ) ) ? AppendNull() :
                        ( DATATABLE_TYPE_TIMESTAMP == m_column.type ? AppendTimestamp( values[ i ] ) :
                        ( DATATABLE_TYPE_DECIMAL == m_column.type ? AppendDecimalMantissa( values[ i ] ) : AppendInt64( values[ i ] ) ) );
        if ( !ok )
        {
            TDataTableStatus status = m_lastStatus;
            Truncate( startRows );
            return Fail( status );
        }
    }
    m_lastStatus = DATATABLE_STATUS_OK;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendInt32s( const Int32* values, UInt32 count, const UInt8* validity )
{

    const UInt32 startRows = m_rowCount;
    for ( UInt32 i=0; i<count; ++i )
    {
        const bool ok = ( GUCEF_NULL != validity && !GetBit( validity, i ) ) ? AppendNull() :
                        ( DATATABLE_TYPE_DATE == m_column.type ? AppendDate( values[ i ] ) : AppendInt64( values[ i ] ) );
        if ( !ok )
        {
            TDataTableStatus status = m_lastStatus;
            Truncate( startRows );
            return Fail( status );
        }
    }
    m_lastStatus = DATATABLE_STATUS_OK;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBuilder::AppendFloat64s( const Float64* values, UInt32 count, const UInt8* validity )
{

    const UInt32 startRows = m_rowCount;
    for ( UInt32 i=0; i<count; ++i )
    {
        const bool ok = ( GUCEF_NULL != validity && !GetBit( validity, i ) ) ? AppendNull() : AppendFloat64( values[ i ] );
        if ( !ok )
        {
            TDataTableStatus status = m_lastStatus;
            Truncate( startRows );
            return Fail( status );
        }
    }
    m_lastStatus = DATATABLE_STATUS_OK;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableColumnBuilder::GetRowCount( void ) const
{

    return m_rowCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableColumnBuilder::GetNullCount( void ) const
{

    return m_nullCount;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableColumnBuilder::GetLastStatus( void ) const
{

    return m_lastStatus;
}

/*-------------------------------------------------------------------------*/

const CDataTableColumnDef&
CDataTableColumnBuilder::GetColumnDef( void ) const
{

    return m_column;
}

/*-------------------------------------------------------------------------*/

void
CDataTableColumnBuilder::Truncate( UInt32 rowCount )
{

    if ( rowCount >= m_rowCount )
        return;
    if ( !m_validity.IsNULL() )
    {
        const UInt8* bits = static_cast< const UInt8* >( m_validity->GetData() );
        for ( UInt32 i=rowCount; i<m_rowCount; ++i )
        {
            if ( !GetBit( bits, i ) )
                --m_nullCount;
        }
    }
    if ( DATATABLE_STORAGE_VARIABLE == m_storage )
        m_byteCount = static_cast< const UInt32* >( m_values->GetData() )[ rowCount ];
    m_rowCount = rowCount;
}

/*-------------------------------------------------------------------------*/

void
CDataTableColumnBuilder::Reset( void )
{

    Truncate( 0 );
    m_nullCount = 0;
    m_byteCount = 0;
    m_validity.Reset();
    m_lastStatus = DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableColumnBuilder::BuildInto( CDataTableColumnBatch& column )
{

    column.SetType( m_column );
    switch ( m_storage )
    {
        case DATATABLE_STORAGE_FIXED:
        case DATATABLE_STORAGE_BITS:
        {
            if ( 0 == m_rowCount )
            {
                column.SetFixedValues( 0, GUCEF_NULL, CDataTableBufferPtr() );
                break;
            }
            if ( DATATABLE_STORAGE_BITS == m_storage )
            {
                ClearTrailingBits( static_cast< UInt8* >( m_values->GetData() ), m_rowCount );
                m_values->SetDataSize( ( m_rowCount + 7 ) >> 3 );
            }
            else
            {
                m_values->SetDataSize( m_rowCount * m_width );
            }
            column.SetFixedValues( m_rowCount, m_values->GetData(), m_values );
            break;
        }
        case DATATABLE_STORAGE_VARIABLE:
        {
            if ( 0 == m_rowCount )
            {
                column.SetVariableValues( 0, g_emptyOffsets, CDataTableBufferPtr(), GUCEF_NULL, CDataTableBufferPtr() );
                break;
            }
            m_values->SetDataSize( ( m_rowCount + 1 ) * sizeof( UInt32 ) );
            m_bytes->SetDataSize( m_byteCount );
            column.SetVariableValues( m_rowCount, static_cast< const UInt32* >( m_values->GetData() ), m_values,
                                      static_cast< const char* >( m_bytes->GetData() ), m_bytes );
            break;
        }
        default:
        {
            return DATATABLE_STATUS_INVALID_ARGUMENT;
        }
    }

    if ( m_nullCount > 0 )
    {
        ClearTrailingBits( static_cast< UInt8* >( m_validity->GetData() ), m_rowCount );
        m_validity->SetDataSize( ( m_rowCount + 7 ) >> 3 );
        column.SetValidity( static_cast< const UInt8* >( m_validity->GetData() ), m_nullCount, m_validity );
    }

    // the buffers now belong to the column batch; the next rows go into fresh ones from the pool
    m_values.Reset();
    m_bytes.Reset();
    m_validity.Reset();
    m_rowCount = 0;
    m_nullCount = 0;
    m_byteCount = 0;
    m_lastStatus = DATATABLE_STATUS_OK;
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CDataTableBatchBuilder::CDataTableBatchBuilder( const CDataTableSchemaPtr& schema, CDataTableBufferPool& pool )
    : m_schema( schema )
    , m_pool( &pool )
    , m_columns()
    , m_invalidColumn( GUCEF_NULL )
    , m_lastStatus( DATATABLE_STATUS_OK )
{GUCEF_TRACE;

    m_invalidColumn = GUCEF_NEW CDataTableColumnBuilder();
    if ( !m_schema.IsNULL() )
    {
        const UInt32 columnCount = m_schema->GetColumnCount();
        m_columns.reserve( columnCount );
        for ( UInt32 i=0; i<columnCount; ++i )
            m_columns.push_back( GUCEF_NEW CDataTableColumnBuilder( m_schema->GetColumn( i ), m_pool ) );
    }
}

/*-------------------------------------------------------------------------*/

CDataTableBatchBuilder::~CDataTableBatchBuilder()
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) m_columns.size(); ++i )
        GUCEF_DELETE m_columns[ i ];
    m_columns.clear();
    GUCEF_DELETE m_invalidColumn;
    m_invalidColumn = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CDataTableColumnBuilder&
CDataTableBatchBuilder::GetColumn( UInt32 schemaColumnIndex )
{

    if ( schemaColumnIndex < (UInt32) m_columns.size() )
        return *m_columns[ schemaColumnIndex ];
    return *m_invalidColumn;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableBatchBuilder::GetColumnCount( void ) const
{

    return (UInt32) m_columns.size();
}

/*-------------------------------------------------------------------------*/

bool
CDataTableBatchBuilder::AppendRow( const CVariant* values, UInt32 valueCount )
{

    if ( valueCount != (UInt32) m_columns.size() || ( valueCount > 0 && GUCEF_NULL == values ) )
    {
        m_lastStatus = DATATABLE_STATUS_INVALID_ARGUMENT;
        return false;
    }
    for ( UInt32 i=0; i<valueCount; ++i )
    {
        const UInt32 rowsBefore = m_columns[ i ]->GetRowCount();
        if ( !m_columns[ i ]->Append( values[ i ] ) )
        {
            // take the row out of the columns that took it
            m_lastStatus = m_columns[ i ]->GetLastStatus();
            for ( UInt32 n=0; n<i; ++n )
                m_columns[ n ]->Truncate( m_columns[ n ]->GetRowCount() - 1 );
            m_columns[ i ]->Truncate( rowsBefore );
            return false;
        }
    }
    m_lastStatus = DATATABLE_STATUS_OK;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableBatchBuilder::GetRowCount( void ) const
{

    UInt32 rows = GUCEF_UINT32MAX;
    for ( UInt32 i=0; i<(UInt32) m_columns.size(); ++i )
    {
        if ( m_columns[ i ]->GetRowCount() < rows )
            rows = m_columns[ i ]->GetRowCount();
    }
    return GUCEF_UINT32MAX == rows ? 0 : rows;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableBatchBuilder::GetLastStatus( void ) const
{

    return m_lastStatus;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableBatchBuilder::Build( CDataTableBatchPtr& batch, CString& error )
{GUCEF_TRACE;

    batch.Reset();
    const UInt32 columnCount = (UInt32) m_columns.size();
    const UInt32 rowCount = columnCount > 0 ? m_columns[ 0 ]->GetRowCount() : 0;
    for ( UInt32 i=1; i<columnCount; ++i )
    {
        if ( m_columns[ i ]->GetRowCount() != rowCount )
        {
            error = "Column \"" + m_columns[ i ]->GetColumnDef().name + "\" has " + UInt32ToString( m_columns[ i ]->GetRowCount() ) +
                    " rows where column \"" + m_columns[ 0 ]->GetColumnDef().name + "\" has " + UInt32ToString( rowCount );
            m_lastStatus = DATATABLE_STATUS_INVALID_ARGUMENT;
            return m_lastStatus;
        }
    }

    CDataTableBatchPtr result;
    TDataTableStatus status = m_pool->AcquireBatch( result );
    if ( DATATABLE_STATUS_OK != status )
    {
        error = "No batch object could be acquired";
        m_lastStatus = status;
        return status;
    }

    CDataTableBatch* target = result.GetWritable();
    if ( !target->SetColumnCount( columnCount ) )
    {
        error = "The columns of the batch could not be allocated";
        m_lastStatus = DATATABLE_STATUS_OUT_OF_MEMORY;
        return m_lastStatus;
    }
    target->SetSchema( m_schema );
    target->SetRowCount( rowCount );
    for ( UInt32 i=0; i<columnCount; ++i )
    {
        target->SetSchemaColumnIndex( i, i );
        status = m_columns[ i ]->BuildInto( target->GetMutableColumn( i ) );
        if ( DATATABLE_STATUS_OK != status )
        {
            error = "Column \"" + m_columns[ i ]->GetColumnDef().name + "\" could not be built";
            m_lastStatus = status;
            return status;
        }
    }

    batch.Swap( result );
    m_lastStatus = DATATABLE_STATUS_OK;
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CDataTableBatchBuilder::Reset( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) m_columns.size(); ++i )
        m_columns[ i ]->Reset();
    m_lastStatus = DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

const CDataTableSchemaPtr&
CDataTableBatchBuilder::GetSchema( void ) const
{

    return m_schema;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
