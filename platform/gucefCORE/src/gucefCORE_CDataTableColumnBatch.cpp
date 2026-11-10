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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CDATATABLECOLUMNDEF_H
#include "gucefCORE_CDataTableColumnDef.h"
#define GUCEF_CORE_CDATATABLECOLUMNDEF_H
#endif /* GUCEF_CORE_CDATATABLECOLUMNDEF_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCH_H
#include "gucefCORE_CDataTableBatch.h"
#define GUCEF_CORE_CDATATABLEBATCH_H
#endif /* GUCEF_CORE_CDATATABLEBATCH_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#include "gucefCORE_CDataTableColumnBatch.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Cleared bits among the first bitCount bits, LSB first */
UInt32
CountClearedBits( const UInt8* bits, UInt32 bitCount )
{
    UInt32 setBits = 0;
    const UInt32 fullBytes = bitCount >> 3;
    for ( UInt32 i=0; i<fullBytes; ++i )
    {
        UInt32 b = bits[ i ];
        b = b - ( ( b >> 1 ) & 0x55u );
        b = ( b & 0x33u ) + ( ( b >> 2 ) & 0x33u );
        setBits += ( b + ( b >> 4 ) ) & 0x0Fu;
    }
    for ( UInt32 i=fullBytes<<3; i<bitCount; ++i )
    {
        if ( 0 != ( bits[ i >> 3 ] & ( 1u << ( i & 7u ) ) ) )
            ++setBits;
    }
    return bitCount - setBits;
}

/** Copies bitCount bits starting at bit firstBit into a new pooled buffer, starting at bit 0 */
TDataTableStatus
CopyShiftedBits( const UInt8* bits, UInt32 firstBit, UInt32 bitCount, CDataTableBufferPool& pool, CDataTableBufferPtr& copy )
{
    const UInt32 byteCount = ( bitCount + 7 ) >> 3;
    TDataTableStatus status = pool.Acquire( byteCount, copy );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    UInt8* out = static_cast< UInt8* >( copy->GetData() );
    memset( out, 0, byteCount );
    for ( UInt32 i=0; i<bitCount; ++i )
    {
        const UInt32 bit = firstBit + i;
        if ( 0 != ( bits[ bit >> 3 ] & ( 1u << ( bit & 7u ) ) ) )
            out[ i >> 3 ] = (UInt8) ( out[ i >> 3 ] | ( 1u << ( i & 7u ) ) );
    }
    copy->SetDataSize( byteCount );
    return DATATABLE_STATUS_OK;
}

}

CDataTableColumnBatch::CDataTableColumnBatch( void )
    : m_type( DATATABLE_TYPE_UNSUPPORTED )
    , m_storage( DATATABLE_STORAGE_NONE )
    , m_physical( PHYSICAL_NONE )
    , m_rowCount( 0 )
    , m_nullCount( 0 )
    , m_decimalPrecision( DataTableMaxDecimalPrecision )
    , m_decimalScale( 0 )
    , m_timeUnit( DATATABLE_TIMEUNIT_MICROSECONDS )
    , m_utcAdjusted( true )
    , m_values( GUCEF_NULL )
    , m_offsets( GUCEF_NULL )
    , m_bytes( GUCEF_NULL )
    , m_indices( GUCEF_NULL )
    , m_validity( GUCEF_NULL )
    , m_dictionary( GUCEF_NULL )
    , m_valuesOwner()
    , m_bytesOwner()
    , m_validityOwner()
    , m_dictionaryOwner()
{

}

/*-------------------------------------------------------------------------*/

CDataTableColumnBatch::CDataTableColumnBatch( const CDataTableColumnBatch& src )
    : m_type( src.m_type )
    , m_storage( src.m_storage )
    , m_physical( src.m_physical )
    , m_rowCount( src.m_rowCount )
    , m_nullCount( src.m_nullCount )
    , m_decimalPrecision( src.m_decimalPrecision )
    , m_decimalScale( src.m_decimalScale )
    , m_timeUnit( src.m_timeUnit )
    , m_utcAdjusted( src.m_utcAdjusted )
    , m_values( src.m_values )
    , m_offsets( src.m_offsets )
    , m_bytes( src.m_bytes )
    , m_indices( src.m_indices )
    , m_validity( src.m_validity )
    , m_dictionary( src.m_dictionary )
    , m_valuesOwner( src.m_valuesOwner )
    , m_bytesOwner( src.m_bytesOwner )
    , m_validityOwner( src.m_validityOwner )
    , m_dictionaryOwner( src.m_dictionaryOwner )
{

}

/*-------------------------------------------------------------------------*/

CDataTableColumnBatch::~CDataTableColumnBatch()
{

}

/*-------------------------------------------------------------------------*/

CDataTableColumnBatch&
CDataTableColumnBatch::operator=( const CDataTableColumnBatch& src )
{

    if ( &src != this )
    {
        m_type = src.m_type;
        m_storage = src.m_storage;
        m_physical = src.m_physical;
        m_rowCount = src.m_rowCount;
        m_nullCount = src.m_nullCount;
        m_decimalPrecision = src.m_decimalPrecision;
        m_decimalScale = src.m_decimalScale;
        m_timeUnit = src.m_timeUnit;
        m_utcAdjusted = src.m_utcAdjusted;
        m_values = src.m_values;
        m_offsets = src.m_offsets;
        m_bytes = src.m_bytes;
        m_indices = src.m_indices;
        m_validity = src.m_validity;
        m_dictionary = src.m_dictionary;
        m_valuesOwner = src.m_valuesOwner;
        m_bytesOwner = src.m_bytesOwner;
        m_validityOwner = src.m_validityOwner;
        m_dictionaryOwner = src.m_dictionaryOwner;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CDataTableColumnBatch::UpdatePhysical( void )
{

    switch ( m_storage )
    {
        case DATATABLE_STORAGE_BITS:        { m_physical = PHYSICAL_BITS; return; }
        case DATATABLE_STORAGE_VARIABLE:    { m_physical = PHYSICAL_VARIABLE; return; }
        case DATATABLE_STORAGE_DICTIONARY:  { m_physical = PHYSICAL_DICTIONARY; return; }
        case DATATABLE_STORAGE_FIXED:       { break; }
        default:                            { m_physical = PHYSICAL_NONE; return; }
    }

    switch ( m_type )
    {
        case DATATABLE_TYPE_INT8:       { m_physical = PHYSICAL_INT8; break; }
        case DATATABLE_TYPE_INT16:      { m_physical = PHYSICAL_INT16; break; }
        case DATATABLE_TYPE_INT32:
        case DATATABLE_TYPE_DATE:       { m_physical = PHYSICAL_INT32; break; }
        case DATATABLE_TYPE_INT64:
        case DATATABLE_TYPE_DECIMAL:
        case DATATABLE_TYPE_TIMESTAMP:  { m_physical = PHYSICAL_INT64; break; }
        case DATATABLE_TYPE_UINT8:      { m_physical = PHYSICAL_UINT8; break; }
        case DATATABLE_TYPE_UINT16:     { m_physical = PHYSICAL_UINT16; break; }
        case DATATABLE_TYPE_UINT32:     { m_physical = PHYSICAL_UINT32; break; }
        case DATATABLE_TYPE_UINT64:     { m_physical = PHYSICAL_UINT64; break; }
        case DATATABLE_TYPE_FLOAT32:    { m_physical = PHYSICAL_FLOAT32; break; }
        case DATATABLE_TYPE_FLOAT64:    { m_physical = PHYSICAL_FLOAT64; break; }
        default:                        { m_physical = PHYSICAL_NONE; break; }
    }
}

/*-------------------------------------------------------------------------*/

void
CDataTableColumnBatch::Reset( void )
{

    m_storage = DATATABLE_STORAGE_NONE;
    m_physical = PHYSICAL_NONE;
    m_rowCount = 0;
    m_nullCount = 0;
    m_values = GUCEF_NULL;
    m_offsets = GUCEF_NULL;
    m_bytes = GUCEF_NULL;
    m_indices = GUCEF_NULL;
    m_validity = GUCEF_NULL;
    m_dictionary = GUCEF_NULL;
    m_valuesOwner.Reset();
    m_bytesOwner.Reset();
    m_validityOwner.Reset();
    m_dictionaryOwner.Reset();
}

/*-------------------------------------------------------------------------*/

void
CDataTableColumnBatch::SetType( TDataTableLogicalType type      ,
                                UInt8 decimalPrecision          ,
                                UInt8 decimalScale              ,
                                TDataTableTimeUnit timeUnit     ,
                                bool utcAdjusted                )
{GUCEF_TRACE;

    Reset();
    m_type = type;
    m_decimalPrecision = decimalPrecision;
    m_decimalScale = decimalScale;
    m_timeUnit = timeUnit;
    m_utcAdjusted = utcAdjusted;
}

/*-------------------------------------------------------------------------*/

void
CDataTableColumnBatch::SetType( const CDataTableColumnDef& column )
{GUCEF_TRACE;

    SetType( column.type, column.decimalPrecision, column.decimalScale, column.timeUnit, column.utcAdjusted );
}

/*-------------------------------------------------------------------------*/

void
CDataTableColumnBatch::SetFixedValues( UInt32 rowCount, const void* values, const CDataTableBufferPtr& owner )
{GUCEF_TRACE;

    m_storage = DataTableLogicalTypeStorage( m_type );
    if ( DATATABLE_STORAGE_FIXED != m_storage && DATATABLE_STORAGE_BITS != m_storage )
        m_storage = DATATABLE_STORAGE_NONE;
    m_rowCount = rowCount;
    m_values = values;
    m_valuesOwner = owner;
    m_offsets = GUCEF_NULL;
    m_bytes = GUCEF_NULL;
    m_bytesOwner.Reset();
    m_indices = GUCEF_NULL;
    m_dictionary = GUCEF_NULL;
    m_dictionaryOwner.Reset();
    m_validity = GUCEF_NULL;
    m_nullCount = 0;
    m_validityOwner.Reset();
    UpdatePhysical();
}

/*-------------------------------------------------------------------------*/

void
CDataTableColumnBatch::SetVariableValues( UInt32 rowCount                         ,
                                          const UInt32* offsets                   ,
                                          const CDataTableBufferPtr& offsetsOwner ,
                                          const char* bytes                       ,
                                          const CDataTableBufferPtr& bytesOwner   )
{GUCEF_TRACE;

    m_storage = DATATABLE_STORAGE_VARIABLE;
    m_rowCount = rowCount;
    m_values = GUCEF_NULL;
    m_offsets = offsets;
    m_valuesOwner = offsetsOwner;
    m_bytes = bytes;
    m_bytesOwner = bytesOwner;
    m_indices = GUCEF_NULL;
    m_dictionary = GUCEF_NULL;
    m_dictionaryOwner.Reset();
    m_validity = GUCEF_NULL;
    m_nullCount = 0;
    m_validityOwner.Reset();
    UpdatePhysical();
}

/*-------------------------------------------------------------------------*/

void
CDataTableColumnBatch::SetDictionaryValues( UInt32 rowCount                         ,
                                            const UInt32* indices                   ,
                                            const CDataTableBufferPtr& indicesOwner ,
                                            const CDataTableBatchPtr& dictionary    ,
                                            UInt32 dictionaryColumn                 )
{GUCEF_TRACE;

    m_storage = DATATABLE_STORAGE_DICTIONARY;
    m_rowCount = rowCount;
    m_values = GUCEF_NULL;
    m_offsets = GUCEF_NULL;
    m_bytes = GUCEF_NULL;
    m_bytesOwner.Reset();
    m_indices = indices;
    m_valuesOwner = indicesOwner;
    m_dictionaryOwner = dictionary;
    m_dictionary = dictionary.IsNULL() ? GUCEF_NULL : &dictionary->GetColumn( dictionaryColumn );
    m_validity = GUCEF_NULL;
    m_nullCount = 0;
    m_validityOwner.Reset();
    UpdatePhysical();
}

/*-------------------------------------------------------------------------*/

void
CDataTableColumnBatch::SetValidity( const UInt8* validity, UInt32 nullCount, const CDataTableBufferPtr& owner )
{GUCEF_TRACE;

    if ( GUCEF_NULL == validity || 0 == nullCount )
    {
        // no nulls: the fast path has no bitmap at all
        m_validity = GUCEF_NULL;
        m_nullCount = 0;
        m_validityOwner.Reset();
        return;
    }
    m_validity = validity;
    m_nullCount = nullCount;
    m_validityOwner = owner;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBatch::GetVariableCell( const char* bytes, UInt32 byteLength, CVariant& out, bool linkIfPossible ) const
{

    if ( DATATABLE_TYPE_UTF8 == m_type )
    {
        // the terminator is part of the value, as LinkTo( const CUtf8String& ) gives
        return out.Set( bytes, byteLength + 1, GUCEF_DATATYPE_UTF8_STRING, linkIfPossible );
    }
    return out.Set( bytes, byteLength, GUCEF_DATATYPE_BINARY_BLOB, linkIfPossible );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBatch::GetValue( UInt32 row, CVariant& out, bool linkIfPossible ) const
{

    if ( row >= m_rowCount )
    {
        out.Clear();
        return false;
    }
    if ( !IsValid( row ) )
    {
        out.Set( GUCEF_NULL, 0, GUCEF_DATATYPE_NULL, false );
        return true;
    }

    switch ( m_physical )
    {
        case PHYSICAL_BITS:
        {
            const UInt8* bits = static_cast< const UInt8* >( m_values );
            out = 0 != ( bits[ row >> 3 ] & ( 1u << ( row & 7u ) ) );
            return true;
        }
        case PHYSICAL_INT8:     { out = static_cast< const Int8* >( m_values )[ row ]; return true; }
        case PHYSICAL_INT16:    { out = static_cast< const Int16* >( m_values )[ row ]; return true; }
        case PHYSICAL_UINT8:    { out = static_cast< const UInt8* >( m_values )[ row ]; return true; }
        case PHYSICAL_UINT16:   { out = static_cast< const UInt16* >( m_values )[ row ]; return true; }
        case PHYSICAL_UINT32:   { out = static_cast< const UInt32* >( m_values )[ row ]; return true; }
        case PHYSICAL_UINT64:   { out = static_cast< const UInt64* >( m_values )[ row ]; return true; }
        case PHYSICAL_FLOAT32:  { out = static_cast< const Float32* >( m_values )[ row ]; return true; }
        case PHYSICAL_FLOAT64:  { out = static_cast< const Float64* >( m_values )[ row ]; return true; }
        case PHYSICAL_INT32:
        {
            const Int32 value = static_cast< const Int32* >( m_values )[ row ];
            if ( DATATABLE_TYPE_DATE == m_type )
                return out.Set( &value, sizeof( value ), GUCEF_DATATYPE_DATE_IN_DAYS_SINCE_UNIX_EPOCH, false );
            out = value;
            return true;
        }
        case PHYSICAL_INT64:
        {
            const Int64 value = static_cast< const Int64* >( m_values )[ row ];
            switch ( m_type )
            {
                case DATATABLE_TYPE_TIMESTAMP:
                {
                    return out.Set( &value, sizeof( value ), CDataTableValues::TimestampTypeId( m_timeUnit ), false );
                }
                case DATATABLE_TYPE_DECIMAL:
                {
                    CDecimal decimal;
                    if ( DecimalStatusIsError( CDecimal::FromUnscaled( value, m_decimalScale, decimal ) ) )
                    {
                        out.Clear();
                        return false;
                    }
                    out = decimal;
                    return true;
                }
                default:
                {
                    out = value;
                    return true;
                }
            }
        }
        case PHYSICAL_VARIABLE:
        {
            const UInt32 begin = m_offsets[ row ];
            return GetVariableCell( m_bytes + begin, m_offsets[ row + 1 ] - begin - 1, out, linkIfPossible );
        }
        case PHYSICAL_DICTIONARY:
        {
            UInt32 byteLength = 0;
            const char* bytes = m_dictionary->GetVariableValue( m_indices[ row ], byteLength );
            return GetVariableCell( bytes, byteLength, out, linkIfPossible );
        }
        default:
        {
            out.Clear();
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnBatch::Validate( CString& error ) const
{GUCEF_TRACE;

    switch ( m_storage )
    {
        case DATATABLE_STORAGE_NONE:
        {
            if ( m_rowCount > 0 )
            {
                error = "A column batch without storage holds rows";
                return false;
            }
            return true;
        }
        case DATATABLE_STORAGE_FIXED:
        case DATATABLE_STORAGE_BITS:
        {
            if ( m_storage != DataTableLogicalTypeStorage( m_type ) )
            {
                error = "The storage does not match the type " + CString( DataTableLogicalTypeToString( m_type ) );
                return false;
            }
            if ( m_rowCount > 0 && GUCEF_NULL == m_values )
            {
                error = "Fixed width values are missing";
                return false;
            }
            break;
        }
        case DATATABLE_STORAGE_VARIABLE:
        {
            if ( DATATABLE_TYPE_UTF8 != m_type && DATATABLE_TYPE_BINARY != m_type )
            {
                error = "Variable storage holds a type that is neither UTF8 nor BINARY";
                return false;
            }
            if ( GUCEF_NULL == m_offsets || ( m_rowCount > 0 && GUCEF_NULL == m_bytes ) )
            {
                error = "Offsets or bytes are missing";
                return false;
            }
            for ( UInt32 i=0; i<m_rowCount; ++i )
            {
                if ( m_offsets[ i + 1 ] <= m_offsets[ i ] )
                {
                    error = "Offset " + UInt32ToString( i + 1 ) + " leaves no room for the terminator of value " + UInt32ToString( i );
                    return false;
                }
                if ( '\0' != m_bytes[ m_offsets[ i + 1 ] - 1 ] )
                {
                    error = "Value " + UInt32ToString( i ) + " lacks its terminator";
                    return false;
                }
            }
            break;
        }
        case DATATABLE_STORAGE_DICTIONARY:
        {
            if ( GUCEF_NULL == m_dictionary || DATATABLE_STORAGE_VARIABLE != m_dictionary->GetStorage() || m_dictionary->GetLogicalType() != m_type )
            {
                error = "The dictionary is missing or is not a variable column of the same type";
                return false;
            }
            if ( !m_dictionary->Validate( error ) )
                return false;
            if ( m_rowCount > 0 && GUCEF_NULL == m_indices )
            {
                error = "Dictionary indices are missing";
                return false;
            }
            const UInt32 dictionaryRows = m_dictionary->GetRowCount();
            for ( UInt32 i=0; i<m_rowCount; ++i )
            {
                if ( m_indices[ i ] >= dictionaryRows )
                {
                    error = "Dictionary index " + UInt32ToString( m_indices[ i ] ) + " of row " + UInt32ToString( i ) + " is beyond the dictionary";
                    return false;
                }
            }
            break;
        }
    }

    if ( GUCEF_NULL == m_validity )
    {
        if ( 0 != m_nullCount )
        {
            error = "A null count without a validity bitmap";
            return false;
        }
        return true;
    }

    UInt32 nulls = 0;
    const UInt32 width = DataTableLogicalTypeByteWidth( m_type );
    for ( UInt32 i=0; i<m_rowCount; ++i )
    {
        if ( IsValid( i ) )
            continue;
        ++nulls;

        // null slots are zero
        bool slotIsZero = true;
        switch ( m_storage )
        {
            case DATATABLE_STORAGE_FIXED:
            {
                const UInt8* slot = static_cast< const UInt8* >( m_values ) + (UInt64) i * width;
                for ( UInt32 b=0; b<width; ++b )
                    slotIsZero = slotIsZero && 0 == slot[ b ];
                break;
            }
            case DATATABLE_STORAGE_BITS:
            {
                slotIsZero = 0 == ( static_cast< const UInt8* >( m_values )[ i >> 3 ] & ( 1u << ( i & 7u ) ) );
                break;
            }
            case DATATABLE_STORAGE_VARIABLE:
            {
                slotIsZero = 1 == m_offsets[ i + 1 ] - m_offsets[ i ];
                break;
            }
            case DATATABLE_STORAGE_DICTIONARY:
            {
                slotIsZero = 0 == m_indices[ i ];
                break;
            }
            default:
            {
                break;
            }
        }
        if ( !slotIsZero )
        {
            error = "The slot of null row " + UInt32ToString( i ) + " is not zero";
            return false;
        }
    }
    if ( nulls != m_nullCount )
    {
        error = "The null count is " + UInt32ToString( m_nullCount ) + " but the validity has " + UInt32ToString( nulls ) + " nulls";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDataTableColumnBatch::Truncate( UInt32 rowCount )
{GUCEF_TRACE;

    if ( rowCount >= m_rowCount )
        return;

    m_rowCount = rowCount;
    if ( GUCEF_NULL != m_validity )
    {
        m_nullCount = CountClearedBits( m_validity, rowCount );
        if ( 0 == m_nullCount )
        {
            m_validity = GUCEF_NULL;
            m_validityOwner.Reset();
        }
    }
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableColumnBatch::SetSlice( const CDataTableColumnBatch& source, UInt32 firstRow, UInt32 rowCount, CDataTableBufferPool& pool )
{GUCEF_TRACE;

    if ( firstRow > source.m_rowCount || rowCount > source.m_rowCount - firstRow )
        return DATATABLE_STATUS_INVALID_ARGUMENT;

    // a copy first: the source may be this column
    const CDataTableColumnBatch src( source );
    *this = src;
    m_rowCount = rowCount;

    switch ( src.m_storage )
    {
        case DATATABLE_STORAGE_FIXED:
        {
            m_values = static_cast< const char* >( src.m_values ) + (size_t) firstRow * DataTableLogicalTypeByteWidth( src.m_type );
            break;
        }
        case DATATABLE_STORAGE_BITS:
        {
            if ( 0 == ( firstRow & 7u ) )
            {
                m_values = static_cast< const UInt8* >( src.m_values ) + ( firstRow >> 3 );
            }
            else
            {
                CDataTableBufferPtr bits;
                TDataTableStatus status = CopyShiftedBits( static_cast< const UInt8* >( src.m_values ), firstRow, rowCount, pool, bits );
                if ( DATATABLE_STATUS_OK != status )
                {
                    Reset();
                    return status;
                }
                m_values = bits->GetData();
                m_valuesOwner = bits;
            }
            break;
        }
        case DATATABLE_STORAGE_VARIABLE:
        {
            // offsets stay absolute positions in the shared bytes
            m_offsets = src.m_offsets + firstRow;
            break;
        }
        case DATATABLE_STORAGE_DICTIONARY:
        {
            m_indices = src.m_indices + firstRow;
            break;
        }
        default:
        {
            break;
        }
    }

    if ( GUCEF_NULL != src.m_validity )
    {
        if ( 0 == ( firstRow & 7u ) )
        {
            m_validity = src.m_validity + ( firstRow >> 3 );
        }
        else
        {
            CDataTableBufferPtr validity;
            TDataTableStatus status = CopyShiftedBits( src.m_validity, firstRow, rowCount, pool, validity );
            if ( DATATABLE_STATUS_OK != status )
            {
                Reset();
                return status;
            }
            m_validity = static_cast< const UInt8* >( validity->GetData() );
            m_validityOwner = validity;
        }
        m_nullCount = CountClearedBits( m_validity, rowCount );
        if ( 0 == m_nullCount )
        {
            m_validity = GUCEF_NULL;
            m_validityOwner.Reset();
        }
    }
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
