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

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#include "gucefCORE_CCsvTableWriter.h"

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

CCsvTableWriter::CCsvTableWriter( const IOAccessPtr& destination    ,
                                  const CDataTableSchemaPtr& schema ,
                                  const SCsvWriteOptions& options   )
    : CITableWriter()
    , m_destination( destination )
    , m_schema( schema )
    , m_options( options )
    , m_out()
    , m_nullCounts()
    , m_rowCount( 0 )
    , m_byteCount( 0 )
    , m_started( false )
    , m_finished( false )
    , m_failed( false )
    , m_failedStatus( DATATABLE_STATUS_OK )
    , m_lastError()
{GUCEF_TRACE;

    m_out.reserve( FlushThreshold + 4096 );
    if ( !m_schema.IsNULL() )
        m_nullCounts.assign( m_schema->GetColumnCount(), 0 );
}

/*-------------------------------------------------------------------------*/

CCsvTableWriter::~CCsvTableWriter()
{GUCEF_TRACE;

    // Without Finish() the output is incomplete and the caller removes it
    if ( !m_finished )
        Abort();
}

/*-------------------------------------------------------------------------*/

CString
CCsvTableWriter::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    return m_lastError;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvTableWriter::Fail( TDataTableStatus status, const CString& message )
{GUCEF_TRACE;

    m_lastError = message;
    m_failed = true;
    m_failedStatus = status;
    return status;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvTableWriter::Flush( void )
{GUCEF_TRACE;

    if ( m_out.empty() )
        return DATATABLE_STATUS_OK;
    if ( m_destination.IsNULL() )
        return Fail( DATATABLE_STATUS_IO_ERROR, "No destination to write to" );

    const UInt32 count = (UInt32) m_out.size();
    const UInt32 written = m_destination->Write( &m_out[ 0 ], 1, count );
    m_out.clear();
    if ( written != count )
        return Fail( DATATABLE_STATUS_IO_ERROR, "The destination accepted " + UInt32ToString( written ) + " of " + UInt32ToString( count ) + " bytes" );
    m_byteCount += count;
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CCsvTableWriter::AppendBytes( const char* bytes, UInt32 count )
{GUCEF_TRACE;

    if ( count > 0 )
        m_out.insert( m_out.end(), bytes, bytes + count );
}

/*-------------------------------------------------------------------------*/

void
CCsvTableWriter::AppendField( const char* data, UInt32 length, bool quoteIfEmpty )
{GUCEF_TRACE;

    const char delimiter = m_options.delimiter;
    const char quote = m_options.quote;

    bool needsQuotes = quoteIfEmpty && 0 == length;
    if ( !needsQuotes && length > 0 )
    {
        if ( ' ' == data[ 0 ] || '\t' == data[ 0 ] || ' ' == data[ length - 1 ] || '\t' == data[ length - 1 ] )
        {
            needsQuotes = true;
        }
        else
        {
            for ( UInt32 i = 0; i < length; ++i )
            {
                const char c = data[ i ];
                if ( c == delimiter || c == quote || '\r' == c || '\n' == c )
                {
                    needsQuotes = true;
                    break;
                }
            }
        }
    }
    if ( !needsQuotes && length > 0 && quoteIfEmpty )
    {
        // text that reads as the null token is quoted to stay text
        const UInt32 nullLength = m_options.nullToken.ByteSize() > 0 ? m_options.nullToken.ByteSize() - 1 : 0;
        if ( nullLength == length && 0 == memcmp( m_options.nullToken.C_String(), data, length ) )
            needsQuotes = true;
    }

    if ( !needsQuotes )
    {
        AppendBytes( data, length );
        return;
    }

    m_out.push_back( quote );
    UInt32 runStart = 0;
    for ( UInt32 i = 0; i < length; ++i )
    {
        if ( data[ i ] == quote )
        {
            AppendBytes( data + runStart, i + 1 - runStart );       // up to and including the quote
            m_out.push_back( quote );                               // doubled
            runStart = i + 1;
        }
    }
    AppendBytes( data + runStart, length - runStart );
    m_out.push_back( quote );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvTableWriter::Start( void )
{GUCEF_TRACE;

    if ( m_started )
        return DATATABLE_STATUS_OK;
    if ( m_schema.IsNULL() || 0 == m_schema->GetColumnCount() )
        return Fail( DATATABLE_STATUS_INVALID_ARGUMENT, "A CSV file needs a schema with at least one column" );

    m_started = true;
    if ( m_options.bom )
        AppendBytes( "\xEF\xBB\xBF", 3 );

    if ( m_options.header )
    {
        for ( UInt32 i = 0; i < m_schema->GetColumnCount(); ++i )
        {
            if ( i > 0 )
                m_out.push_back( m_options.delimiter );
            const CString& name = m_schema->GetColumn( i ).name;
            AppendField( name.C_String(), name.ByteSize() > 0 ? name.ByteSize() - 1 : 0, true );
        }
        AppendBytes( m_options.lineEnd.C_String(), m_options.lineEnd.ByteSize() - 1 );
    }
    return Flush();
}

/*-------------------------------------------------------------------------*/

bool
CCsvTableWriter::CheckBatch( const CDataTableBatch& batch )
{GUCEF_TRACE;

    const UInt32 columnCount = m_schema->GetColumnCount();
    if ( batch.GetColumnCount() != columnCount )
    {
        Fail( DATATABLE_STATUS_SCHEMA_MISMATCH, "The batch has " + UInt32ToString( batch.GetColumnCount() ) + " columns where the schema has " + UInt32ToString( columnCount ) );
        return false;
    }

    for ( UInt32 c = 0; c < columnCount; ++c )
    {
        const CDataTableColumnDef& def = m_schema->GetColumn( c );
        const CDataTableColumnBatch& column = batch.GetColumn( c );
        bool same = ( column.GetLogicalType() == def.type );
        if ( same && DATATABLE_TYPE_DECIMAL == def.type )
            same = ( column.GetDecimalScale() == def.decimalScale );
        if ( same && DATATABLE_TYPE_TIMESTAMP == def.type )
            same = ( column.GetTimeUnit() == def.timeUnit && column.IsUtcAdjusted() == def.utcAdjusted );
        if ( !same || DATATABLE_TYPE_UNSUPPORTED == def.type )
        {
            Fail( DATATABLE_STATUS_SCHEMA_MISMATCH, "Column " + UInt32ToString( c ) + " of the batch is not " + def.GetTypeText() + " as column \"" + def.name + "\" of the schema" );
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvTableWriter::WriteBatch( const CDataTableBatch& batch )
{GUCEF_TRACE;

    if ( m_failed )
        return m_failedStatus;
    if ( m_finished )
        return Fail( DATATABLE_STATUS_INVALID_ARGUMENT, "WriteBatch after Finish" );
    TDataTableStatus status = Start();
    if ( DATATABLE_STATUS_OK != status )
        return status;
    if ( !CheckBatch( batch ) )
        return m_failedStatus;

    const UInt32 columnCount = m_schema->GetColumnCount();
    const UInt32* selection = batch.GetSelection();
    const UInt32 selected = batch.GetSelectedRowCount();

    for ( UInt32 s = 0; s < selected; ++s )
    {
        const UInt32 row = GUCEF_NULL == selection ? s : selection[ s ];
        for ( UInt32 c = 0; c < columnCount; ++c )
        {
            if ( c > 0 )
                m_out.push_back( m_options.delimiter );

            const CDataTableColumnBatch& column = batch.GetColumn( c );
            if ( !column.IsValid( row ) )
            {
                ++m_nullCounts[ c ];
                const CString& nullToken = m_options.nullToken;
                AppendField( nullToken.C_String(), nullToken.ByteSize() > 0 ? nullToken.ByteSize() - 1 : 0, false );
                continue;
            }

            char text[ CDataTableValues::MaxTimestampTextLength + 8 ];
            char number[ CCsvTableText::MaxFloatTextLength + CCsvTableText::MaxIntegerTextLength ];
            UInt32 length = 0;
            switch ( column.GetLogicalType() )
            {
                case DATATABLE_TYPE_BOOL:
                {
                    const bool value = 0 != ( column.GetBoolBits()[ row >> 3 ] & ( 1u << ( row & 7u ) ) );
                    AppendField( value ? "true" : "false", value ? 4 : 5, false );
                    break;
                }
                case DATATABLE_TYPE_INT8: length = CCsvTableText::FormatInt64( column.GetInt8Data()[ row ], number ); AppendField( number, length, false ); break;
                case DATATABLE_TYPE_INT16: length = CCsvTableText::FormatInt64( column.GetInt16Data()[ row ], number ); AppendField( number, length, false ); break;
                case DATATABLE_TYPE_INT32: length = CCsvTableText::FormatInt64( column.GetInt32Data()[ row ], number ); AppendField( number, length, false ); break;
                case DATATABLE_TYPE_INT64: length = CCsvTableText::FormatInt64( column.GetInt64Data()[ row ], number ); AppendField( number, length, false ); break;
                case DATATABLE_TYPE_UINT8: length = CCsvTableText::FormatUInt64( column.GetUInt8Data()[ row ], number ); AppendField( number, length, false ); break;
                case DATATABLE_TYPE_UINT16: length = CCsvTableText::FormatUInt64( column.GetUInt16Data()[ row ], number ); AppendField( number, length, false ); break;
                case DATATABLE_TYPE_UINT32: length = CCsvTableText::FormatUInt64( column.GetUInt32Data()[ row ], number ); AppendField( number, length, false ); break;
                case DATATABLE_TYPE_UINT64: length = CCsvTableText::FormatUInt64( column.GetUInt64Data()[ row ], number ); AppendField( number, length, false ); break;
                case DATATABLE_TYPE_FLOAT32: length = CCsvTableText::FormatFloat32( column.GetFloat32Data()[ row ], number ); AppendField( number, length, false ); break;
                case DATATABLE_TYPE_FLOAT64: length = CCsvTableText::FormatFloat64( column.GetFloat64Data()[ row ], number ); AppendField( number, length, false ); break;
                case DATATABLE_TYPE_DECIMAL: length = CCsvTableText::FormatDecimal( column.GetInt64Data()[ row ], column.GetDecimalScale(), number ); AppendField( number, length, false ); break;
                case DATATABLE_TYPE_DATE:
                {
                    length = CDataTableValues::FormatDate( column.GetInt32Data()[ row ], text, sizeof( text ) );
                    AppendField( text, length, false );
                    break;
                }
                case DATATABLE_TYPE_TIMESTAMP:
                {
                    length = CDataTableValues::FormatTimestamp( column.GetInt64Data()[ row ], column.GetTimeUnit(), column.IsUtcAdjusted(), text, sizeof( text ) );
                    AppendField( text, length, false );
                    break;
                }
                case DATATABLE_TYPE_UTF8:
                {
                    UInt32 byteLength = 0;
                    const char* value = column.GetVariableValue( row, byteLength );
                    AppendField( GUCEF_NULL != value ? value : "", byteLength, true );
                    break;
                }
                case DATATABLE_TYPE_BINARY:
                {
                    UInt32 byteLength = 0;
                    const char* value = column.GetVariableValue( row, byteLength );
                    const size_t encodedSize = ( ( (size_t) byteLength + 2 ) / 3 ) * 4;
                    GUCEF::vector< char > encoded( encodedSize + 1 );
                    const UInt32 encodedLength = CCsvTableText::Base64Encode( value, byteLength, &encoded[ 0 ] );
                    AppendField( &encoded[ 0 ], encodedLength, true );
                    break;
                }
                default:
                    return Fail( DATATABLE_STATUS_UNSUPPORTED, "A column of this type cannot be written" );
            }
        }
        AppendBytes( m_options.lineEnd.C_String(), m_options.lineEnd.ByteSize() - 1 );
        ++m_rowCount;

        if ( m_out.size() >= FlushThreshold )
        {
            status = Flush();
            if ( DATATABLE_STATUS_OK != status )
                return status;
        }
    }
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvTableWriter::FlushRowGroup( void )
{GUCEF_TRACE;

    // a CSV file has no row groups; the buffered text goes to the destination
    if ( m_failed )
        return m_failedStatus;
    return Flush();
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvTableWriter::Finish( CDataTableFileSummary& summary )
{GUCEF_TRACE;

    summary.Clear();
    if ( m_failed )
        return m_failedStatus;
    if ( m_finished )
        return Fail( DATATABLE_STATUS_INVALID_ARGUMENT, "Finish was called already" );

    TDataTableStatus status = Start();
    if ( DATATABLE_STATUS_OK != status )
        return status;
    status = Flush();
    if ( DATATABLE_STATUS_OK != status )
        return status;
    m_destination->Flush();
    m_finished = true;

    summary.rowCount = m_rowCount;
    summary.rowGroupCount = 1;
    summary.byteCount = m_byteCount;
    summary.columnStats.resize( m_nullCounts.size() );
    for ( size_t i = 0; i < m_nullCounts.size(); ++i )
    {
        summary.columnStats[ i ].hasNullCount = true;
        summary.columnStats[ i ].nullCount = m_nullCounts[ i ];
        summary.columnStats[ i ].valueCount = m_rowCount;
    }
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CCsvTableWriter::Abort( void )
{GUCEF_TRACE;

    // the output is incomplete: nothing more is written, the caller removes it
    m_out.clear();
    m_failed = true;
    if ( DATATABLE_STATUS_OK == m_failedStatus )
        m_failedStatus = DATATABLE_STATUS_CANCELLED;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
