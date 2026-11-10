/*
 *  tablecodecpluginPARQUET: GUCEF table codec plugin for Apache Parquet files
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

#include <string.h>
#include <algorithm>

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef GUCEF_CORE_CDATATABLEQUERY_H
#include "gucefCORE_CDataTableQuery.h"
#define GUCEF_CORE_CDATATABLEQUERY_H
#endif /* GUCEF_CORE_CDATATABLEQUERY_H ? */

#ifndef GUCEF_CORE_CDATATABLEPRUNER_H
#include "gucefCORE_CDataTablePruner.h"
#define GUCEF_CORE_CDATATABLEPRUNER_H
#endif /* GUCEF_CORE_CDATATABLEPRUNER_H ? */

#ifndef GUCEF_CORE_CDATATABLEWRITEROPTIONS_H
#include "gucefCORE_CDataTableWriterOptions.h"
#define GUCEF_CORE_CDATATABLEWRITEROPTIONS_H
#endif /* GUCEF_CORE_CDATATABLEWRITEROPTIONS_H ? */

#ifndef GUCEF_CORE_CDATATABLESCANSTATS_H
#include "gucefCORE_CDataTableScanStats.h"
#define GUCEF_CORE_CDATATABLESCANSTATS_H
#endif /* GUCEF_CORE_CDATATABLESCANSTATS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_BYTEORDER_H
#include "tablecodecpluginPARQUET_ByteOrder.h"
#define TABLECODECPLUGIN_PARQUET_BYTEORDER_H
#endif /* TABLECODECPLUGIN_PARQUET_BYTEORDER_H ? */

#include "tablecodecpluginPARQUET_CParquetTableReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace PARQUET {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Variable values of one column batch stay below 4 GiB, the reach of the UInt32 offsets, with room for the padding */
const UInt64 MaxVariableBytes = 0xFFFFFFFFULL - 4096;

inline bool
BitIsSet( const UInt8* bitmap, UInt32 index )
{
    return 0 != ( bitmap[ index >> 3 ] & ( 1u << ( index & 7 ) ) );
}

inline void
SetBit( UInt8* bitmap, UInt32 index )
{
    bitmap[ index >> 3 ] |= (UInt8) ( 1u << ( index & 7 ) );
}

void
SetBitRange( UInt8* bitmap, UInt32 first, UInt32 count )
{
    UInt32 bit = first;
    const UInt32 end = first + count;
    while ( bit < end && 0 != ( bit & 7 ) )
        SetBit( bitmap, bit++ );
    if ( end - bit >= 8 )
    {
        const UInt32 bytes = ( end - bit ) >> 3;
        memset( bitmap + ( bit >> 3 ), 0xFF, bytes );
        bit += bytes << 3;
    }
    while ( bit < end )
        SetBit( bitmap, bit++ );
}

/** Copies count bits; the destination bits must be clear. Returns how many of them are set */
UInt32
CopyBits( UInt8* destination, UInt32 destinationBit, const UInt8* source, UInt32 sourceBit, UInt32 count )
{
    UInt32 ones = 0;
    UInt32 i = 0;
    if ( 0 == ( destinationBit & 7 ) && 0 == ( sourceBit & 7 ) )
    {
        const UInt32 bytes = count >> 3;
        const UInt8* s = source + ( sourceBit >> 3 );
        UInt8* d = destination + ( destinationBit >> 3 );
        for ( UInt32 b=0; b<bytes; ++b )
        {
            UInt8 v = s[ b ];
            d[ b ] = v;
            while ( 0 != v ) { v &= (UInt8) ( v - 1 ); ++ones; }
        }
        i = bytes << 3;
    }
    for ( ; i<count; ++i )
    {
        if ( BitIsSet( source, sourceBit + i ) )
        {
            SetBit( destination, destinationBit + i );
            ++ones;
        }
    }
    return ones;
}

/*
 *  Converters from the little endian PLAIN bytes of a physical type to the native batch value
 */
struct SConvertRaw32 { typedef UInt32 TOut; static inline UInt32 Get( const UInt8* p ) { return LoadLE32( p ); } };
struct SConvertRaw64 { typedef UInt64 TOut; static inline UInt64 Get( const UInt8* p ) { return LoadLE64( p ); } };
struct SConvertToInt8 { typedef Int8 TOut; static inline Int8 Get( const UInt8* p ) { return (Int8) (Int32) LoadLE32( p ); } };
struct SConvertToInt16 { typedef Int16 TOut; static inline Int16 Get( const UInt8* p ) { return (Int16) (Int32) LoadLE32( p ); } };
struct SConvertToUInt8 { typedef UInt8 TOut; static inline UInt8 Get( const UInt8* p ) { return (UInt8) LoadLE32( p ); } };
struct SConvertToUInt16 { typedef UInt16 TOut; static inline UInt16 Get( const UInt8* p ) { return (UInt16) LoadLE32( p ); } };
struct SConvertToInt64 { typedef Int64 TOut; static inline Int64 Get( const UInt8* p ) { return (Int64) (Int32) LoadLE32( p ); } };

/** Spreads dense values to their row slots; null slots are zero */
template < class TConvert >
void
SpreadFixed( typename TConvert::TOut* out, const UInt8* source, UInt32 sourceWidth, UInt32 rowCount, const UInt8* validity )
{
    if ( GUCEF_NULL == validity )
    {
        for ( UInt32 r=0; r<rowCount; ++r )
            out[ r ] = TConvert::Get( source + (size_t) r * sourceWidth );
        return;
    }
    UInt32 k = 0;
    for ( UInt32 r=0; r<rowCount; ++r )
    {
        if ( BitIsSet( validity, r ) )
            out[ r ] = TConvert::Get( source + (size_t) ( k++ ) * sourceWidth );
        else
            out[ r ] = 0;
    }
}

/** Gathers dictionary values of a width into their row slots; null slots are zero */
template < typename T >
void
GatherFixed( T* out, const T* dictionary, const UInt32* indices, UInt32 rowCount, const UInt8* validity )
{
    if ( GUCEF_NULL == validity )
    {
        for ( UInt32 r=0; r<rowCount; ++r )
            out[ r ] = dictionary[ indices[ r ] ];
        return;
    }
    UInt32 k = 0;
    for ( UInt32 r=0; r<rowCount; ++r )
    {
        if ( BitIsSet( validity, r ) )
            out[ r ] = dictionary[ indices[ k++ ] ];
        else
            out[ r ] = 0;
    }
}

/**
 *  Gathers the rows of page column batches into one column batch of pooled buffers, for a batch whose rows cross pages.
 *  Text and binary stay dictionary storage while every piece uses the one dictionary of the row group, and become
 *  variable storage otherwise.
 */
class CGather
{
    public:

    CGather( CORE::CDataTableBufferPool& pool, const CORE::CDataTableColumnDef& column, UInt32 rowCapacity )
        : m_pool( pool ), m_column( column ), m_capacity( rowCapacity ), m_rows( 0 ), m_nullCount( 0 ), m_started( false )
        , m_storage( CORE::DATATABLE_STORAGE_NONE ), m_width( CORE::DataTableLogicalTypeByteWidth( column.type ) ), m_bytesUsed( 0 )
        , m_dictionary( GUCEF_NULL )
    {
    }

    CORE::TDataTableStatus Append( const CORE::CDataTableColumnBatch& piece, UInt32 first, UInt32 count )
    {
        CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
        if ( !m_started )
        {
            status = Start( piece );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return status;
        }
        if ( count > m_capacity - m_rows )
            return CORE::DATATABLE_STATUS_FAILED;

        UInt8* validity = static_cast< UInt8* >( m_validity->GetData() );
        if ( GUCEF_NULL == piece.GetValidity() )
            SetBitRange( validity, m_rows, count );
        else
            m_nullCount += count - CopyBits( validity, m_rows, piece.GetValidity(), first, count );

        switch ( m_storage )
        {
            case CORE::DATATABLE_STORAGE_FIXED:
            {
                const UInt8* source = static_cast< const UInt8* >( piece.GetFixedData() );
                if ( GUCEF_NULL == source )
                    return CORE::DATATABLE_STATUS_FAILED;
                memcpy( static_cast< UInt8* >( m_values->GetData() ) + (size_t) m_rows * m_width, source + (size_t) first * m_width, (size_t) count * m_width );
                break;
            }
            case CORE::DATATABLE_STORAGE_BITS:
            {
                const UInt8* source = piece.GetBoolBits();
                if ( GUCEF_NULL == source )
                    return CORE::DATATABLE_STATUS_FAILED;
                CopyBits( static_cast< UInt8* >( m_values->GetData() ), m_rows, source, first, count );
                break;
            }
            case CORE::DATATABLE_STORAGE_DICTIONARY:
            {
                if ( CORE::DATATABLE_STORAGE_DICTIONARY != piece.GetStorage() || piece.GetDictionary() != m_dictionary )
                {
                    status = ConvertToVariable();
                    if ( CORE::DATATABLE_STATUS_OK != status )
                        return status;
                    return AppendVariable( piece, first, count );
                }
                memcpy( static_cast< UInt32* >( m_values->GetData() ) + m_rows, piece.GetDictionaryIndices() + first, (size_t) count * 4 );
                break;
            }
            case CORE::DATATABLE_STORAGE_VARIABLE:
            {
                return AppendVariable( piece, first, count );
            }
            default:
            {
                return CORE::DATATABLE_STATUS_FAILED;
            }
        }
        m_rows += count;
        return CORE::DATATABLE_STATUS_OK;
    }

    void Finish( CORE::CDataTableColumnBatch& out )
    {
        out.SetType( m_column );
        switch ( m_storage )
        {
            case CORE::DATATABLE_STORAGE_FIXED:
            case CORE::DATATABLE_STORAGE_BITS: { out.SetFixedValues( m_rows, m_values->GetData(), m_values ); break; }
            case CORE::DATATABLE_STORAGE_DICTIONARY: { out.SetDictionaryValues( m_rows, static_cast< const UInt32* >( m_values->GetData() ), m_values, m_dictionaryBatch, 0 ); break; }
            case CORE::DATATABLE_STORAGE_VARIABLE: { out.SetVariableValues( m_rows, static_cast< const UInt32* >( m_values->GetData() ), m_values, static_cast< const char* >( m_bytes->GetData() ), m_bytes ); break; }
            default: { break; }
        }
        if ( m_nullCount > 0 )
            out.SetValidity( static_cast< const UInt8* >( m_validity->GetData() ), m_nullCount, m_validity );
    }

    private:

    CORE::TDataTableStatus Start( const CORE::CDataTableColumnBatch& piece )
    {
        m_started = true;
        m_storage = piece.GetStorage();
        CORE::TDataTableStatus status = m_pool.Acquire( ( m_capacity + 7 ) / 8, m_validity );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        memset( m_validity->GetData(), 0, ( m_capacity + 7 ) / 8 );
        m_validity->SetDataSize( ( m_capacity + 7 ) / 8 );
        switch ( m_storage )
        {
            case CORE::DATATABLE_STORAGE_FIXED: { status = m_pool.Acquire( m_capacity * m_width, m_values ); if ( CORE::DATATABLE_STATUS_OK == status ) m_values->SetDataSize( m_capacity * m_width ); break; }
            case CORE::DATATABLE_STORAGE_BITS:
            {
                status = m_pool.Acquire( ( m_capacity + 7 ) / 8, m_values );
                if ( CORE::DATATABLE_STATUS_OK == status )
                {
                    memset( m_values->GetData(), 0, ( m_capacity + 7 ) / 8 );
                    m_values->SetDataSize( ( m_capacity + 7 ) / 8 );
                }
                break;
            }
            case CORE::DATATABLE_STORAGE_DICTIONARY:
            {
                m_dictionary = piece.GetDictionary();
                m_dictionaryBatch = piece.GetDictionaryBatch();
                status = m_pool.Acquire( m_capacity * 4, m_values );
                if ( CORE::DATATABLE_STATUS_OK == status )
                    m_values->SetDataSize( m_capacity * 4 );
                break;
            }
            case CORE::DATATABLE_STORAGE_VARIABLE: { status = StartVariable(); break; }
            default: { status = CORE::DATATABLE_STATUS_FAILED; break; }
        }
        return status;
    }

    CORE::TDataTableStatus StartVariable( void )
    {
        m_storage = CORE::DATATABLE_STORAGE_VARIABLE;
        CORE::TDataTableStatus status = m_pool.Acquire( ( m_capacity + 1 ) * 4, m_values );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        m_values->SetDataSize( ( m_capacity + 1 ) * 4 );
        static_cast< UInt32* >( m_values->GetData() )[ 0 ] = 0;
        m_bytesUsed = 0;
        return m_pool.Acquire( m_capacity + 1, m_bytes );
    }

    CORE::TDataTableStatus AppendBytes( const char* bytes, UInt32 length )
    {
        const UInt64 needed = (UInt64) m_bytesUsed + length;
        if ( needed > MaxVariableBytes )
            return CORE::DATATABLE_STATUS_LIMIT_EXCEEDED;
        if ( needed > m_bytes->GetCapacity() )
        {
            UInt64 grown = (UInt64) m_bytes->GetCapacity() * 2;
            if ( grown < needed )
                grown = needed;
            if ( grown > MaxVariableBytes )
                grown = MaxVariableBytes;
            m_bytes->SetDataSize( m_bytesUsed );
            CORE::TDataTableStatus status = m_pool.Reserve( m_bytes, (UInt32) grown );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return status;
        }
        if ( length > 0 )
            memcpy( static_cast< char* >( m_bytes->GetData() ) + m_bytesUsed, bytes, length );
        m_bytesUsed += length;
        m_bytes->SetDataSize( m_bytesUsed );
        return CORE::DATATABLE_STATUS_OK;
    }

    CORE::TDataTableStatus AppendVariable( const CORE::CDataTableColumnBatch& piece, UInt32 first, UInt32 count )
    {
        UInt32* offsets = static_cast< UInt32* >( m_values->GetData() );
        if ( CORE::DATATABLE_STORAGE_VARIABLE == piece.GetStorage() )
        {
            // the bytes of the rows are contiguous, terminators included: one copy and shifted offsets
            const UInt32* source = piece.GetOffsets();
            const UInt32 begin = source[ first ];
            const UInt32 end = source[ first + count ];
            const UInt32 base = m_bytesUsed;
            CORE::TDataTableStatus status = AppendBytes( piece.GetBytes() + begin, end - begin );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return status;
            offsets = static_cast< UInt32* >( m_values->GetData() );
            for ( UInt32 i=1; i<=count; ++i )
                offsets[ m_rows + i ] = base + ( source[ first + i ] - begin );
        }
        else
        {
            for ( UInt32 i=0; i<count; ++i )
            {
                UInt32 length = 0;
                const char* value = piece.IsValid( first + i ) ? piece.GetVariableValue( first + i, length ) : GUCEF_NULL;
                const char zero = '\0';
                CORE::TDataTableStatus status = AppendBytes( value, GUCEF_NULL != value ? length : 0 );
                if ( CORE::DATATABLE_STATUS_OK == status )
                    status = AppendBytes( &zero, 1 );
                if ( CORE::DATATABLE_STATUS_OK != status )
                    return status;
                offsets[ m_rows + i + 1 ] = m_bytesUsed;
            }
        }
        m_rows += count;
        return CORE::DATATABLE_STATUS_OK;
    }

    CORE::TDataTableStatus ConvertToVariable( void )
    {
        // the rows gathered so far as dictionary indices become values
        CORE::CDataTableBufferPtr indices = m_values;
        const UInt32 rows = m_rows;
        const UInt8* validity = static_cast< const UInt8* >( m_validity->GetData() );
        CORE::TDataTableStatus status = StartVariable();
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        const UInt32* index = static_cast< const UInt32* >( indices->GetData() );
        UInt32* offsets = static_cast< UInt32* >( m_values->GetData() );
        for ( UInt32 r=0; r<rows; ++r )
        {
            UInt32 length = 0;
            const char* value = BitIsSet( validity, r ) ? m_dictionary->GetVariableValue( index[ r ], length ) : GUCEF_NULL;
            const char zero = '\0';
            status = AppendBytes( value, GUCEF_NULL != value ? length : 0 );
            if ( CORE::DATATABLE_STATUS_OK == status )
                status = AppendBytes( &zero, 1 );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return status;
            offsets[ r + 1 ] = m_bytesUsed;
        }
        m_dictionary = GUCEF_NULL;
        m_dictionaryBatch.Reset();
        return CORE::DATATABLE_STATUS_OK;
    }

    CORE::CDataTableBufferPool& m_pool;
    const CORE::CDataTableColumnDef& m_column;
    UInt32 m_capacity;
    UInt32 m_rows;
    UInt32 m_nullCount;
    bool m_started;
    CORE::TDataTableStorage m_storage;
    UInt32 m_width;
    CORE::CDataTableBufferPtr m_values;     /**< fixed values, bits, dictionary indices or variable offsets */
    CORE::CDataTableBufferPtr m_bytes;
    UInt32 m_bytesUsed;
    CORE::CDataTableBufferPtr m_validity;
    const CORE::CDataTableColumnBatch* m_dictionary;
    CORE::CDataTableBatchPtr m_dictionaryBatch;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CParquetTableReader::SColumnState::SColumnState( void )
    : fileColumn( 0 ), info( GUCEF_NULL ), meta( GUCEF_NULL ), compression( GUCEF_NULL ), chunkBuffer(), chunkData( GUCEF_NULL )
    , chunkSize( 0 ), chunkPosition( 0 ), rowsLoaded( 0 ), hasDictionary( false ), dictionaryCount( 0 ), dictionary(), dictionaryBatch()
    , page(), pageFirstRow( 0 ), pageRows( 0 ), pageLoaded( false )
{
}

/*-------------------------------------------------------------------------*/

CParquetTableReader::CParquetTableReader( const CORE::IOAccessPtr& source                 ,
                                          const CORE::CITableFileMetaDataPtr& metaData    ,
                                          const CParquetFileMetaData& parquetMetaData     ,
                                          const SParquetReadOptions& options              )
    : CORE::CITableReader()
    , m_source( source )
    , m_metaDataPtr( metaData )
    , m_metaData( parquetMetaData )
    , m_options( options )
    , m_request()
    , m_pool( GUCEF_NULL )
    , m_rowGroup( 0 )
    , m_rowGroupRows( 0 )
    , m_nextRow( 0 )
    , m_batchRowCount( CORE::DataTableDefaultBatchRowCount )
    , m_open( false )
    , m_columns()
    , m_projection()
    , m_ranges()
    , m_decoder()
    , m_scratch()
    , m_failed( false )
    , m_failedStatus( CORE::DATATABLE_STATUS_OK )
    , m_lastError()
{GUCEF_TRACE;

    for ( UInt32 i=0; i<=PARQUET_COMPRESSION_LZ4_RAW; ++i )
        m_compressionReady[ i ] = false;
}

/*-------------------------------------------------------------------------*/

CParquetTableReader::~CParquetTableReader()
{GUCEF_TRACE;

    m_columns.clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CITableFileMetaDataPtr&
CParquetTableReader::GetMetaData( void ) const
{GUCEF_TRACE;

    return m_metaDataPtr;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CParquetTableReader::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    return m_lastError;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::Fail( CORE::TDataTableStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    m_failed = true;
    m_failedStatus = CORE::DataTableStatusIsError( status ) ? status : CORE::DATATABLE_STATUS_FAILED;
    m_lastError = message;
    m_open = false;
    return m_failedStatus;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::FailColumn( const SColumnState& column, CORE::TDataTableStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    return Fail( status, "Column \"" + column.info->path + "\", row group " + CORE::ToString( m_rowGroup ) + ": " + message );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::AcquireZeroed( UInt32 bytes, CORE::CDataTableBufferPtr& buffer )
{
    CORE::TDataTableStatus status = m_pool->Acquire( bytes, buffer );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    memset( buffer->GetData(), 0, bytes );
    buffer->SetDataSize( bytes );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::CheckDecodedSize( const SColumnState& column, UInt64 bytes )
{
    if ( bytes > m_options.maxPageBytes )
        return FailColumn( column, CORE::DATATABLE_STATUS_LIMIT_EXCEEDED, "a page decodes to " + CORE::ToString( bytes ) +
                           " bytes, more than maxPageBytes (" + CORE::ToString( m_options.maxPageBytes ) + ")" );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::OpenRowGroup( UInt32 rowGroup, const CORE::CDataTableReadRequest& request )
{GUCEF_TRACE;

    m_failed = false;
    m_failedStatus = CORE::DATATABLE_STATUS_OK;
    m_lastError.Clear();
    m_open = false;
    m_columns.clear();

    if ( m_source.IsNULL() )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "The reader has no source" );
    if ( rowGroup >= m_metaData.GetRowGroupCount() )
        return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "The file has " + CORE::ToString( m_metaData.GetRowGroupCount() ) + " row groups, not row group " + CORE::ToString( rowGroup ) );

    m_request = request;
    m_pool = GUCEF_NULL != request.bufferPool ? request.bufferPool : &CORE::CCoreGlobal::Instance()->GetDataTableBufferPool();
    m_rowGroup = rowGroup;
    m_rowGroupRows = (UInt64) m_metaData.GetParquetMetaData().rowGroups[ rowGroup ].numRows;
    m_nextRow = 0;
    m_batchRowCount = request.batchRowCount;
    if ( 0 == m_batchRowCount )
        m_batchRowCount = CORE::DataTableDefaultBatchRowCount;
    if ( m_batchRowCount > CORE::DataTableMaxBatchRowCount )
        m_batchRowCount = CORE::DataTableMaxBatchRowCount;

    // the projection: every supported column when none is asked for
    const TParquetColumnInfoVector& infos = m_metaData.GetColumnInfos();
    const UInt32 columnCount = (UInt32) infos.size();
    CORE::CDataTableSchema::TColumnIndexVector& projection = m_projection;
    projection = request.projection;
    if ( projection.empty() )
    {
        for ( UInt32 i=0; i<columnCount; ++i )
        {
            if ( infos[ i ].supported )
                projection.push_back( i );
        }
    }
    m_columns.resize( projection.size() );
    for ( size_t p=0; p<projection.size(); ++p )
    {
        const UInt32 fileColumn = projection[ p ];
        if ( fileColumn >= columnCount )
            return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "The projection names column " + CORE::ToString( fileColumn ) + " of a file with " + CORE::ToString( columnCount ) + " columns" );
        for ( size_t q=0; q<p; ++q )
        {
            if ( projection[ q ] == fileColumn )
                return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "The projection names column \"" + infos[ fileColumn ].path + "\" twice" );
        }
        if ( !infos[ fileColumn ].supported )
            return Fail( CORE::DATATABLE_STATUS_UNSUPPORTED, "Column \"" + infos[ fileColumn ].path + "\" cannot be read: " + infos[ fileColumn ].unsupportedReason );
        SColumnState& column = m_columns[ p ];
        column.fileColumn = fileColumn;
        column.info = &infos[ fileColumn ];
        column.meta = &m_metaData.GetParquetMetaData().rowGroups[ rowGroup ].columns[ fileColumn ].metaData;
        const SParquetChunkLocation* location = m_metaData.GetChunkLocation( rowGroup, fileColumn );
        if ( GUCEF_NULL == location )
            return Fail( CORE::DATATABLE_STATUS_FAILED, "No location for a column chunk" );
        if ( CORE::DATATABLE_STATUS_OK != location->status )
            return FailColumn( column, location->status, location->problem );
        if ( location->length > m_options.maxChunkBytes )
            return FailColumn( column, CORE::DATATABLE_STATUS_LIMIT_EXCEEDED, "the column chunk of " + CORE::ToString( location->length ) +
                               " bytes is larger than maxChunkBytes (" + CORE::ToString( m_options.maxChunkBytes ) + ")" );
        CORE::TDataTableStatus status = PrepareCompression( column );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
    }

    if ( 0 == m_rowGroupRows )
    {
        m_open = true;
        return CORE::DATATABLE_STATUS_OK;
    }

    CORE::TDataTableStatus status = ReadChunks();
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    // dictionaries first: they are decoded once per row group and let EQUALS / IN prune the row group
    for ( size_t p=0; p<m_columns.size(); ++p )
    {
        bool gotDataPage = false;
        status = ReadPage( m_columns[ p ], true, gotDataPage );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
    }
    if ( !DictionaryMayMatch() )
    {
        m_columns.clear();
        return CORE::DATATABLE_STATUS_PRUNED;
    }
    m_open = true;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::PrepareCompression( SColumnState& column )
{GUCEF_TRACE;

    const Int32 codec = column.meta->codec;
    if ( codec < 0 || codec > PARQUET_COMPRESSION_LZ4_RAW )
        return FailColumn( column, CORE::DATATABLE_STATUS_UNSUPPORTED, "the column chunk uses " + CParquetMetaDataSerializer::CompressionToString( codec ) + ", which is not known" );
    if ( !m_compressionReady[ codec ] )
    {
        CORE::CString error;
        CORE::TDataTableStatus status = m_compressions[ codec ].Init( codec, CORE::CDataTableWriterOptions::DefaultCompressionLevel, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return FailColumn( column, status, error );
        m_compressionReady[ codec ] = true;
    }
    column.compression = &m_compressions[ codec ];
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::ReadChunks( void )
{GUCEF_TRACE;

    // the chunks in file order, merged into reads when they lie close together
    GUCEF::vector< SChunkReadRange >& ranges = m_ranges;
    ranges.clear();
    for ( size_t p=0; p<m_columns.size(); ++p )
    {
        const SParquetChunkLocation* location = m_metaData.GetChunkLocation( m_rowGroup, m_columns[ p ].fileColumn );
        if ( 0 == location->length )
            continue;
        SChunkReadRange range;
        range.start = location->start;
        range.end = location->start + location->length;
        range.column = (UInt32) p;
        ranges.push_back( range );
    }
    std::sort( ranges.begin(), ranges.end() );

    size_t i = 0;
    while ( i < ranges.size() )
    {
        // reads start at a multiple of 8 in the file, so values aligned in the file are aligned in the buffer
        const UInt64 readStart = ranges[ i ].start & ~(UInt64) 7;
        UInt64 readEnd = ranges[ i ].end;
        size_t last = i;
        while ( last + 1 < ranges.size() && ranges[ last + 1 ].start <= readEnd + m_options.ioCoalesceGapBytes &&
                ( ranges[ last + 1 ].end > readEnd ? ranges[ last + 1 ].end : readEnd ) - readStart <= m_options.maxChunkBytes )
        {
            ++last;
            if ( ranges[ last ].end > readEnd )
                readEnd = ranges[ last ].end;
        }
        const UInt64 length = readEnd - readStart;
        if ( length > m_options.maxChunkBytes + 8 )
            return Fail( CORE::DATATABLE_STATUS_LIMIT_EXCEEDED, "A column chunk read is larger than maxChunkBytes" );

        CORE::CDataTableBufferPtr buffer;
        CORE::TDataTableStatus status = m_pool->Acquire( (UInt32) length, buffer );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return Fail( status, "No buffer for " + CORE::ToString( length ) + " bytes of column chunks" );
        {
            MT::CObjectScopeLock lock( m_request.sharedFileLock );
            if ( 0 != m_source->Setpos( readStart ) || (UInt32) length != m_source->Read( buffer->GetData(), 1, (UInt32) length ) )
                return Fail( CORE::DATATABLE_STATUS_IO_ERROR, "Column chunks of row group " + CORE::ToString( m_rowGroup ) + " at byte " + CORE::ToString( readStart ) + " cannot be read" );
        }
        buffer->SetDataSize( (UInt32) length );
        if ( GUCEF_NULL != m_request.stats )
            m_request.stats->bytesRead += length;

        for ( size_t r=i; r<=last; ++r )
        {
            SColumnState& column = m_columns[ ranges[ r ].column ];
            column.chunkBuffer = buffer;
            column.chunkData = static_cast< const UInt8* >( buffer->GetData() ) + ( ranges[ r ].start - readStart );
            column.chunkSize = (UInt32) ( ranges[ r ].end - ranges[ r ].start );
            column.chunkPosition = 0;
        }
        i = last + 1;
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::ReadPage( SColumnState& column, bool dictionaryOnly, bool& gotDataPage )
{GUCEF_TRACE;

    gotDataPage = false;
    for ( ;; )
    {
        if ( column.chunkPosition >= column.chunkSize )
        {
            if ( dictionaryOnly )
                return CORE::DATATABLE_STATUS_OK;
            return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "the column chunk ends after " + CORE::ToString( column.rowsLoaded ) +
                               " of " + CORE::ToString( m_rowGroupRows ) + " rows" );
        }
        const UInt32 remaining = column.chunkSize - column.chunkPosition;
        CThriftCompactReader thrift( column.chunkData + column.chunkPosition, remaining );
        SParquetPageHeader header;
        if ( !CParquetMetaDataSerializer::ReadPageHeader( thrift, header ) )
            return FailColumn( column, thrift.GetStatus(), "the page header at byte " + CORE::ToString( column.chunkPosition ) +
                               " of the column chunk is not valid: " + thrift.GetErrorMessage() );
        const UInt32 headerSize = thrift.GetPosition();
        if ( (UInt32) header.compressedPageSize > remaining - headerSize )
            return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "a page of " + CORE::ToString( header.compressedPageSize ) +
                               " bytes runs past the end of the column chunk" );
        const UInt8* pageData = column.chunkData + column.chunkPosition + headerSize;
        const UInt32 next = column.chunkPosition + headerSize + (UInt32) header.compressedPageSize;

        switch ( header.type )
        {
            case PARQUET_PAGE_DICTIONARY_PAGE:
            {
                if ( column.hasDictionary || column.rowsLoaded > 0 )
                    return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "a dictionary page follows the first page of the column chunk" );
                column.chunkPosition = next;
                CORE::TDataTableStatus status = DecodeDictionaryPage( column, header, pageData );
                if ( CORE::DATATABLE_STATUS_OK != status )
                    return status;
                if ( dictionaryOnly )
                    return CORE::DATATABLE_STATUS_OK;
                continue;
            }
            case PARQUET_PAGE_DATA_PAGE:
            case PARQUET_PAGE_DATA_PAGE_V2:
            {
                if ( dictionaryOnly )
                    return CORE::DATATABLE_STATUS_OK;
                column.chunkPosition = next;
                CORE::TDataTableStatus status = PARQUET_PAGE_DATA_PAGE == header.type ? DecodeDataPageV1( column, header, pageData )
                                                                                     : DecodeDataPageV2( column, header, pageData );
                if ( CORE::DATATABLE_STATUS_OK != status )
                    return status;
                gotDataPage = true;
                return CORE::DATATABLE_STATUS_OK;
            }
            default:
            {
                // index pages and page types this reader does not know carry nothing it needs
                column.chunkPosition = next;
                continue;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::GetPageBytes( SColumnState& column, const SParquetPageHeader& header, const UInt8* pageData, UInt32 levelBytes,
                                   bool compressed, const UInt8*& bytes, CORE::CDataTableBufferPtr& owner )
{GUCEF_TRACE;

    const UInt32 compressedSize = (UInt32) header.compressedPageSize;
    const UInt32 uncompressedSize = (UInt32) header.uncompressedPageSize;
    if ( (UInt64) uncompressedSize > m_options.maxPageBytes )
        return FailColumn( column, CORE::DATATABLE_STATUS_LIMIT_EXCEEDED, "a page of " + CORE::ToString( uncompressedSize ) +
                           " uncompressed bytes is larger than maxPageBytes (" + CORE::ToString( m_options.maxPageBytes ) + ")" );
    if ( !compressed )
    {
        if ( uncompressedSize != compressedSize )
            return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "an uncompressed page states " + CORE::ToString( uncompressedSize ) +
                               " uncompressed bytes and " + CORE::ToString( compressedSize ) + " stored bytes" );
        bytes = pageData;
        owner = column.chunkBuffer;
        return CORE::DATATABLE_STATUS_OK;
    }
    if ( levelBytes > compressedSize || levelBytes > uncompressedSize )
        return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "the levels of a page are larger than the page" );

    CORE::CDataTableBufferPtr buffer;
    CORE::TDataTableStatus status = m_pool->Acquire( uncompressedSize, buffer );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return FailColumn( column, status, "no buffer for a page of " + CORE::ToString( uncompressedSize ) + " bytes" );
    UInt8* target = static_cast< UInt8* >( buffer->GetData() );
    if ( levelBytes > 0 )
        memcpy( target, pageData, levelBytes );
    CORE::CString error;
    status = column.compression->Decompress( pageData + levelBytes, compressedSize - levelBytes, target + levelBytes, uncompressedSize - levelBytes, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return FailColumn( column, status, error );
    buffer->SetDataSize( uncompressedSize );
    bytes = target;
    owner = buffer;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::DecodeDictionaryPage( SColumnState& column, const SParquetPageHeader& header, const UInt8* pageData )
{GUCEF_TRACE;

    if ( !header.hasDictionaryPageHeader )
        return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "a dictionary page has no dictionary page header" );
    const SParquetDictionaryPageHeader& d = header.dictionaryPageHeader;
    if ( PARQUET_ENCODING_PLAIN != d.encoding && PARQUET_ENCODING_PLAIN_DICTIONARY != d.encoding )
        return FailColumn( column, CORE::DATATABLE_STATUS_UNSUPPORTED, "a dictionary page of encoding " + CParquetMetaDataSerializer::EncodingToString( d.encoding ) );
    if ( d.numValues < 0 )
        return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "a dictionary page has a negative value count" );

    const UInt8* bytes = GUCEF_NULL;
    CORE::CDataTableBufferPtr owner;
    CORE::TDataTableStatus status = GetPageBytes( column, header, pageData, 0, column.compression->IsCompressing(), bytes, owner );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    // the values are PLAIN, without nulls; decoded into the batch layout once for the row group
    status = DecodePlain( column, bytes, (UInt32) header.uncompressedPageSize, (UInt32) d.numValues, GUCEF_NULL, 0, owner, false, column.dictionary );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    column.hasDictionary = true;
    column.dictionaryCount = (UInt32) d.numValues;

    if ( CORE::DATATABLE_STORAGE_VARIABLE == column.dictionary.GetStorage() )
    {
        status = m_pool->AcquireBatch( column.dictionaryBatch );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return FailColumn( column, status, "no batch for a dictionary" );
        CORE::CDataTableBatch* batch = column.dictionaryBatch.GetWritable();
        if ( !batch->SetColumnCount( 1 ) )
            return FailColumn( column, CORE::DATATABLE_STATUS_OUT_OF_MEMORY, "no column for a dictionary" );
        batch->SetSchema( m_metaData.GetSchema() );
        batch->SetRowCount( column.dictionaryCount );
        batch->SetSchemaColumnIndex( 0, column.fileColumn );
        batch->GetMutableColumn( 0 ) = column.dictionary;
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::DecodeLevels( SColumnState& column, const UInt8* data, UInt32 size, UInt32 valueCount,
                                   CORE::CDataTableBufferPtr& validity, UInt32& nullCount )
{GUCEF_TRACE;

    validity.Reset();
    nullCount = 0;
    const UInt32 bytes = (UInt32) ( ( (UInt64) valueCount + 7 ) / 8 );
    CORE::TDataTableStatus status = CheckDecodedSize( column, bytes );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    status = AcquireZeroed( bytes > 0 ? bytes : 1, validity );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return FailColumn( column, status, "no buffer for definition levels" );
    m_decoder.Reset( data, size, 1 );
    UInt32 zeros = 0;
    const UInt32 decoded = m_decoder.GetBits( static_cast< UInt8* >( validity->GetData() ), 0, valueCount, zeros );
    if ( decoded < valueCount )
    {
        validity.Reset();
        return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "the definition levels hold " + CORE::ToString( decoded ) + " of " +
                           CORE::ToString( valueCount ) + " values" + ( m_decoder.HasFailed() ? CORE::CString( ": " ) + m_decoder.GetError() : CORE::CString::Empty ) );
    }
    nullCount = zeros;
    if ( 0 == nullCount )
        validity.Reset();
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::DecodeDataPageV1( SColumnState& column, const SParquetPageHeader& header, const UInt8* pageData )
{GUCEF_TRACE;

    if ( !header.hasDataPageHeader )
        return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "a data page has no data page header" );
    const SParquetDataPageHeader& d = header.dataPageHeader;
    if ( d.numValues < 0 || (UInt64) d.numValues > m_rowGroupRows - column.rowsLoaded )
        return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "a data page of " + CORE::ToString( d.numValues ) +
                           " values does not fit the rows left in the row group" );
    const UInt32 valueCount = (UInt32) d.numValues;

    const UInt8* bytes = GUCEF_NULL;
    CORE::CDataTableBufferPtr owner;
    CORE::TDataTableStatus status = GetPageBytes( column, header, pageData, 0, column.compression->IsCompressing(), bytes, owner );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    UInt32 size = (UInt32) header.uncompressedPageSize;

    CORE::CDataTableBufferPtr validity;
    UInt32 nullCount = 0;
    if ( column.info->maxDefinitionLevel > 0 )
    {
        if ( PARQUET_ENCODING_RLE != d.definitionLevelEncoding )
            return FailColumn( column, CORE::DATATABLE_STATUS_UNSUPPORTED, "definition levels of encoding " +
                               CParquetMetaDataSerializer::EncodingToString( d.definitionLevelEncoding ) );
        if ( size < 4 )
            return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "a data page ends before the length of its definition levels" );
        const UInt32 levelBytes = LoadLE32( bytes );
        if ( levelBytes > size - 4 )
            return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "the definition levels are longer than their page" );
        status = DecodeLevels( column, bytes + 4, levelBytes, valueCount, validity, nullCount );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        bytes += 4 + levelBytes;
        size -= 4 + levelBytes;
    }

    status = DecodeValues( column, d.encoding, bytes, size, valueCount, validity, nullCount, owner, column.page );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    column.pageFirstRow = column.rowsLoaded;
    column.pageRows = valueCount;
    column.rowsLoaded += valueCount;
    column.pageLoaded = true;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::DecodeDataPageV2( SColumnState& column, const SParquetPageHeader& header, const UInt8* pageData )
{GUCEF_TRACE;

    if ( !header.hasDataPageHeaderV2 )
        return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "a data page v2 has no data page v2 header" );
    const SParquetDataPageHeaderV2& d = header.dataPageHeaderV2;
    if ( d.numValues < 0 || d.numNulls < 0 || d.numNulls > d.numValues || d.numRows != d.numValues ||
         (UInt64) d.numValues > m_rowGroupRows - column.rowsLoaded )
        return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "a data page v2 has value, null or row counts that do not fit a flat column" );
    if ( d.definitionLevelsByteLength < 0 || d.repetitionLevelsByteLength != 0 ||
         ( 0 == column.info->maxDefinitionLevel && 0 != d.definitionLevelsByteLength ) ||
         (UInt32) d.definitionLevelsByteLength > (UInt32) header.compressedPageSize )
        return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "a data page v2 has level lengths that do not fit a flat column or its page" );
    const UInt32 valueCount = (UInt32) d.numValues;
    const UInt32 levelBytes = (UInt32) d.definitionLevelsByteLength;

    const UInt8* bytes = GUCEF_NULL;
    CORE::CDataTableBufferPtr owner;
    const bool compressed = d.isCompressed && column.compression->IsCompressing();
    CORE::TDataTableStatus status = GetPageBytes( column, header, pageData, levelBytes, compressed, bytes, owner );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    const UInt32 size = (UInt32) header.uncompressedPageSize;
    if ( levelBytes > size )
        return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "the definition levels are longer than their page" );

    CORE::CDataTableBufferPtr validity;
    UInt32 nullCount = 0;
    if ( column.info->maxDefinitionLevel > 0 )
    {
        status = DecodeLevels( column, bytes, levelBytes, valueCount, validity, nullCount );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
    }
    if ( nullCount != (UInt32) d.numNulls )
        return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "a data page v2 states " + CORE::ToString( d.numNulls ) +
                           " nulls where its definition levels hold " + CORE::ToString( nullCount ) );

    status = DecodeValues( column, d.encoding, bytes + levelBytes, size - levelBytes, valueCount, validity, nullCount, owner, column.page );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    column.pageFirstRow = column.rowsLoaded;
    column.pageRows = valueCount;
    column.rowsLoaded += valueCount;
    column.pageLoaded = true;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::DecodeValues( SColumnState& column, Int32 encoding, const UInt8* data, UInt32 size, UInt32 valueCount,
                                   const CORE::CDataTableBufferPtr& validity, UInt32 nullCount,
                                   const CORE::CDataTableBufferPtr& pageOwner, CORE::CDataTableColumnBatch& out )
{GUCEF_TRACE;

    const UInt8* validityBits = validity.IsNULL() ? GUCEF_NULL : static_cast< const UInt8* >( validity->GetData() );
    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    switch ( encoding )
    {
        case PARQUET_ENCODING_PLAIN:
        {
            status = DecodePlain( column, data, size, valueCount, validityBits, nullCount, pageOwner, true, out );
            break;
        }
        case PARQUET_ENCODING_PLAIN_DICTIONARY:
        case PARQUET_ENCODING_RLE_DICTIONARY:
        {
            status = DecodeDictionaryIndices( column, data, size, valueCount, validityBits, nullCount, out );
            break;
        }
        case PARQUET_ENCODING_RLE:
        {
            if ( PARQUET_CONVERT_BITS != column.info->conversion )
                return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "RLE encoded values in a column that is not BOOLEAN" );
            status = DecodeRleBooleans( column, data, size, valueCount, validityBits, nullCount, out );
            break;
        }
        default:
        {
            return FailColumn( column, CORE::DATATABLE_STATUS_UNSUPPORTED, "data pages of encoding " + CParquetMetaDataSerializer::EncodingToString( encoding ) + " are not supported" );
        }
    }
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    if ( nullCount > 0 )
        out.SetValidity( validityBits, nullCount, validity );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::DecodePlain( SColumnState& column, const UInt8* data, UInt32 size, UInt32 valueCount,
                                  const UInt8* validity, UInt32 nullCount, const CORE::CDataTableBufferPtr& pageOwner,
                                  bool countPage, CORE::CDataTableColumnBatch& out )
{GUCEF_TRACE;

    const SParquetColumnInfo& info = *column.info;
    const UInt32 denseCount = valueCount - nullCount;
    CORE::CDataTableScanStats* stats = countPage ? m_request.stats : GUCEF_NULL;
    out.SetType( info.column );

    switch ( info.conversion )
    {
        case PARQUET_CONVERT_BITS:
        {
            if ( (UInt64) ( denseCount + 7 ) / 8 > size )
                return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "the PLAIN booleans of a page are shorter than its values" );
            CORE::CDataTableBufferPtr bits;
            const UInt32 bytes = (UInt32) ( ( (UInt64) valueCount + 7 ) / 8 );
            CORE::TDataTableStatus status = CheckDecodedSize( column, bytes );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return status;
            status = AcquireZeroed( bytes > 0 ? bytes : 1, bits );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return FailColumn( column, status, "no buffer for booleans" );
            UInt8* target = static_cast< UInt8* >( bits->GetData() );
            if ( GUCEF_NULL == validity )
            {
                CopyBits( target, 0, data, 0, valueCount );
            }
            else
            {
                UInt32 k = 0;
                for ( UInt32 r=0; r<valueCount; ++r )
                {
                    if ( BitIsSet( validity, r ) && BitIsSet( data, k++ ) )
                        SetBit( target, r );
                }
            }
            out.SetFixedValues( valueCount, target, bits );
            if ( GUCEF_NULL != stats ) ++stats->pagesDecoded;
            return CORE::DATATABLE_STATUS_OK;
        }
        case PARQUET_CONVERT_FIXED:
        case PARQUET_CONVERT_INT32_TO_INT8:
        case PARQUET_CONVERT_INT32_TO_INT16:
        case PARQUET_CONVERT_INT32_TO_UINT8:
        case PARQUET_CONVERT_INT32_TO_UINT16:
        case PARQUET_CONVERT_INT32_TO_INT64:
        case PARQUET_CONVERT_INT96_TO_INT64:
        {
            const UInt32 sourceWidth = info.physicalWidth;
            const UInt32 width = info.storageWidth;
            if ( (UInt64) denseCount * sourceWidth > size )
                return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "the PLAIN values of a page are shorter than its " + CORE::ToString( denseCount ) + " values" );
            CORE::TDataTableStatus status = CheckDecodedSize( column, (UInt64) valueCount * width );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return status;

            #if TABLECODECPLUGIN_PARQUET_HOST_IS_LITTLE_ENDIAN
            if ( PARQUET_CONVERT_FIXED == info.conversion && GUCEF_NULL == validity && 0 == ( (size_t) data % width ) )
            {
                // the page itself is the span: no decode, no copy
                out.SetFixedValues( valueCount, data, pageOwner );
                if ( GUCEF_NULL != stats ) ++stats->pagesInPlace;
                return CORE::DATATABLE_STATUS_OK;
            }
            #endif

            CORE::CDataTableBufferPtr values;
            const UInt32 bytes = valueCount * width;
            status = m_pool->Acquire( bytes > 0 ? bytes : 1, values );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return FailColumn( column, status, "no buffer for the values of a page" );
            values->SetDataSize( bytes );
            void* target = values->GetData();
            switch ( info.conversion )
            {
                case PARQUET_CONVERT_FIXED:
                {
                    #if TABLECODECPLUGIN_PARQUET_HOST_IS_LITTLE_ENDIAN
                    if ( GUCEF_NULL == validity )
                    {
                        memcpy( target, data, bytes );
                        if ( GUCEF_NULL != stats ) ++stats->alignmentCopies;
                        break;
                    }
                    #endif
                    if ( 4 == width )
                        SpreadFixed< SConvertRaw32 >( static_cast< UInt32* >( target ), data, 4, valueCount, validity );
                    else
                        SpreadFixed< SConvertRaw64 >( static_cast< UInt64* >( target ), data, 8, valueCount, validity );
                    break;
                }
                case PARQUET_CONVERT_INT32_TO_INT8: { SpreadFixed< SConvertToInt8 >( static_cast< Int8* >( target ), data, 4, valueCount, validity ); break; }
                case PARQUET_CONVERT_INT32_TO_INT16: { SpreadFixed< SConvertToInt16 >( static_cast< Int16* >( target ), data, 4, valueCount, validity ); break; }
                case PARQUET_CONVERT_INT32_TO_UINT8: { SpreadFixed< SConvertToUInt8 >( static_cast< UInt8* >( target ), data, 4, valueCount, validity ); break; }
                case PARQUET_CONVERT_INT32_TO_UINT16: { SpreadFixed< SConvertToUInt16 >( static_cast< UInt16* >( target ), data, 4, valueCount, validity ); break; }
                case PARQUET_CONVERT_INT32_TO_INT64: { SpreadFixed< SConvertToInt64 >( static_cast< Int64* >( target ), data, 4, valueCount, validity ); break; }
                case PARQUET_CONVERT_INT96_TO_INT64:
                {
                    Int64* ticks = static_cast< Int64* >( target );
                    UInt32 k = 0;
                    for ( UInt32 r=0; r<valueCount; ++r )
                    {
                        if ( GUCEF_NULL != validity && !BitIsSet( validity, r ) )
                        {
                            ticks[ r ] = 0;
                            continue;
                        }
                        if ( !CParquetSchemaMapping::DecodeInt96Timestamp( data + (size_t) ( k++ ) * 12, ticks[ r ] ) )
                            return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "an INT96 timestamp is outside the nanosecond range" );
                    }
                    break;
                }
                default: { break; }
            }
            out.SetFixedValues( valueCount, target, values );
            if ( GUCEF_NULL != stats ) ++stats->pagesDecoded;
            return CORE::DATATABLE_STATUS_OK;
        }
        case PARQUET_CONVERT_BYTES_VARIABLE:
        case PARQUET_CONVERT_FIXED_BYTES_VARIABLE:
        {
            CORE::TDataTableStatus status = DecodeVariable( column, data, size, valueCount, validity, nullCount, out );
            if ( CORE::DATATABLE_STATUS_OK == status && GUCEF_NULL != stats )
                ++stats->pagesDecoded;
            return status;
        }
        case PARQUET_CONVERT_BYTES_DECIMAL:
        case PARQUET_CONVERT_FIXED_BYTES_DECIMAL:
        {
            CORE::TDataTableStatus status = CheckDecodedSize( column, (UInt64) valueCount * 8 );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return status;
            CORE::CDataTableBufferPtr values;
            const UInt32 bytes = valueCount * 8;
            status = m_pool->Acquire( bytes > 0 ? bytes : 1, values );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return FailColumn( column, status, "no buffer for the values of a page" );
            values->SetDataSize( bytes );
            Int64* mantissas = static_cast< Int64* >( values->GetData() );
            const bool fixedLength = PARQUET_CONVERT_FIXED_BYTES_DECIMAL == info.conversion;
            UInt32 position = 0;
            for ( UInt32 r=0; r<valueCount; ++r )
            {
                if ( GUCEF_NULL != validity && !BitIsSet( validity, r ) )
                {
                    mantissas[ r ] = 0;
                    continue;
                }
                UInt32 length = (UInt32) info.typeLength;
                if ( !fixedLength )
                {
                    if ( size - position < 4 )
                        return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "a BYTE_ARRAY decimal runs past the end of its page" );
                    length = LoadLE32( data + position );
                    position += 4;
                }
                if ( length > size - position )
                    return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "a decimal runs past the end of its page" );
                if ( !CParquetSchemaMapping::DecodeBigEndianDecimal( data + position, length, mantissas[ r ] ) )
                    return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "a decimal value does not fit 18 digits" );
                position += length;
            }
            out.SetFixedValues( valueCount, mantissas, values );
            if ( GUCEF_NULL != stats ) ++stats->pagesDecoded;
            return CORE::DATATABLE_STATUS_OK;
        }
        default:
        {
            return FailColumn( column, CORE::DATATABLE_STATUS_UNSUPPORTED, "the column cannot be decoded" );
        }
    }
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::DecodeVariable( SColumnState& column, const UInt8* data, UInt32 size, UInt32 valueCount,
                                     const UInt8* validity, UInt32 nullCount, CORE::CDataTableColumnBatch& out )
{GUCEF_TRACE;

    const SParquetColumnInfo& info = *column.info;
    const UInt32 denseCount = valueCount - nullCount;
    const bool fixedLength = PARQUET_CONVERT_FIXED_BYTES_VARIABLE == info.conversion;

    // a first pass checks every length against the page before anything is allocated
    UInt64 valueBytes = 0;
    if ( fixedLength )
    {
        valueBytes = (UInt64) denseCount * (UInt32) info.typeLength;
        if ( valueBytes > size )
            return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "the FIXED_LEN_BYTE_ARRAY values of a page are shorter than its values" );
    }
    else
    {
        UInt32 position = 0;
        for ( UInt32 k=0; k<denseCount; ++k )
        {
            if ( size - position < 4 )
                return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "the BYTE_ARRAY values of a page end after " + CORE::ToString( k ) + " of " + CORE::ToString( denseCount ) + " values" );
            const UInt32 length = LoadLE32( data + position );
            position += 4;
            if ( length > size - position )
                return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "a BYTE_ARRAY value runs past the end of its page" );
            position += length;
            valueBytes += length;
        }
    }
    const UInt64 totalBytes = valueBytes + valueCount;
    if ( totalBytes > MaxVariableBytes )
        return FailColumn( column, CORE::DATATABLE_STATUS_LIMIT_EXCEEDED, "the values of a page are more than 4 GiB" );
    CORE::TDataTableStatus status = CheckDecodedSize( column, totalBytes + (UInt64) ( valueCount + 1 ) * 4 );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    CORE::CDataTableBufferPtr offsetsBuffer;
    CORE::CDataTableBufferPtr bytesBuffer;
    status = m_pool->Acquire( ( valueCount + 1 ) * 4, offsetsBuffer );
    if ( CORE::DATATABLE_STATUS_OK == status )
        status = m_pool->Acquire( totalBytes > 0 ? (UInt32) totalBytes : 1, bytesBuffer );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return FailColumn( column, status, "no buffer for the values of a page" );
    offsetsBuffer->SetDataSize( ( valueCount + 1 ) * 4 );
    bytesBuffer->SetDataSize( (UInt32) totalBytes );
    UInt32* offsets = static_cast< UInt32* >( offsetsBuffer->GetData() );
    char* bytes = static_cast< char* >( bytesBuffer->GetData() );

    const bool checkUtf8 = m_options.validateUtf8 && CORE::DATATABLE_TYPE_UTF8 == info.column.type;
    UInt32 position = 0;
    UInt32 written = 0;
    offsets[ 0 ] = 0;
    for ( UInt32 r=0; r<valueCount; ++r )
    {
        if ( GUCEF_NULL == validity || BitIsSet( validity, r ) )
        {
            UInt32 length = (UInt32) info.typeLength;
            if ( !fixedLength )
            {
                length = LoadLE32( data + position );
                position += 4;
            }
            if ( checkUtf8 && !CParquetSchemaMapping::IsValidUtf8( data + position, length ) )
                return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "a text value is not valid UTF-8" );
            if ( length > 0 )
                memcpy( bytes + written, data + position, length );
            position += length;
            written += length;
        }
        bytes[ written++ ] = '\0';
        offsets[ r + 1 ] = written;
    }
    out.SetVariableValues( valueCount, offsets, offsetsBuffer, bytes, bytesBuffer );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::DecodeRleBooleans( SColumnState& column, const UInt8* data, UInt32 size, UInt32 valueCount,
                                        const UInt8* validity, UInt32 nullCount, CORE::CDataTableColumnBatch& out )
{GUCEF_TRACE;

    const UInt32 denseCount = valueCount - nullCount;
    if ( size < 4 )
        return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "RLE booleans end before their length" );
    const UInt32 length = LoadLE32( data );
    if ( length > size - 4 )
        return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "RLE booleans are longer than their page" );

    const UInt32 denseBytes = (UInt32) ( ( (UInt64) denseCount + 7 ) / 8 );
    CORE::TDataTableStatus status = CheckDecodedSize( column, 2ULL * ( ( (UInt64) valueCount + 7 ) / 8 ) );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    CORE::CDataTableBufferPtr dense;
    status = AcquireZeroed( denseBytes > 0 ? denseBytes : 1, dense );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return FailColumn( column, status, "no buffer for booleans" );
    m_decoder.Reset( data + 4, length, 1 );
    UInt32 zeros = 0;
    if ( m_decoder.GetBits( static_cast< UInt8* >( dense->GetData() ), 0, denseCount, zeros ) < denseCount )
        return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "the RLE booleans of a page are fewer than its values" );

    out.SetType( column.info->column );
    if ( GUCEF_NULL == validity )
    {
        out.SetFixedValues( valueCount, dense->GetData(), dense );
    }
    else
    {
        CORE::CDataTableBufferPtr bits;
        const UInt32 bytes = (UInt32) ( ( (UInt64) valueCount + 7 ) / 8 );
        status = AcquireZeroed( bytes > 0 ? bytes : 1, bits );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return FailColumn( column, status, "no buffer for booleans" );
        const UInt8* source = static_cast< const UInt8* >( dense->GetData() );
        UInt8* target = static_cast< UInt8* >( bits->GetData() );
        UInt32 k = 0;
        for ( UInt32 r=0; r<valueCount; ++r )
        {
            if ( !BitIsSet( validity, r ) )
                continue;
            if ( BitIsSet( source, k++ ) )
                SetBit( target, r );
        }
        out.SetFixedValues( valueCount, target, bits );
    }
    if ( GUCEF_NULL != m_request.stats ) ++m_request.stats->pagesDecoded;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::MaterializeDictionary( const SColumnState& column, const UInt32* indices, UInt32 rowCount,
                                            const UInt8* validity, CORE::CDataTableColumnBatch& out )
{GUCEF_TRACE;

    const CORE::CDataTableColumnBatch& dictionary = column.dictionary;
    UInt64 totalBytes = rowCount;
    UInt32 k = 0;
    for ( UInt32 r=0; r<rowCount; ++r )
    {
        if ( GUCEF_NULL != validity && !BitIsSet( validity, r ) )
            continue;
        UInt32 length = 0;
        dictionary.GetVariableValue( indices[ k++ ], length );
        totalBytes += length;
    }
    if ( totalBytes > MaxVariableBytes )
        return FailColumn( column, CORE::DATATABLE_STATUS_LIMIT_EXCEEDED, "the values of a page are more than 4 GiB" );
    CORE::TDataTableStatus status = CheckDecodedSize( column, totalBytes + (UInt64) ( rowCount + 1 ) * 4 );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    CORE::CDataTableBufferPtr offsetsBuffer;
    CORE::CDataTableBufferPtr bytesBuffer;
    status = m_pool->Acquire( ( rowCount + 1 ) * 4, offsetsBuffer );
    if ( CORE::DATATABLE_STATUS_OK == status )
        status = m_pool->Acquire( totalBytes > 0 ? (UInt32) totalBytes : 1, bytesBuffer );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return FailColumn( column, status, "no buffer for the values of a page" );
    offsetsBuffer->SetDataSize( ( rowCount + 1 ) * 4 );
    bytesBuffer->SetDataSize( (UInt32) totalBytes );
    UInt32* offsets = static_cast< UInt32* >( offsetsBuffer->GetData() );
    char* bytes = static_cast< char* >( bytesBuffer->GetData() );
    UInt32 written = 0;
    offsets[ 0 ] = 0;
    k = 0;
    for ( UInt32 r=0; r<rowCount; ++r )
    {
        if ( GUCEF_NULL == validity || BitIsSet( validity, r ) )
        {
            UInt32 length = 0;
            const char* value = dictionary.GetVariableValue( indices[ k++ ], length );
            if ( length > 0 )
                memcpy( bytes + written, value, length );
            written += length;
        }
        bytes[ written++ ] = '\0';
        offsets[ r + 1 ] = written;
    }
    out.SetType( column.info->column );
    out.SetVariableValues( rowCount, offsets, offsetsBuffer, bytes, bytesBuffer );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::DecodeDictionaryIndices( SColumnState& column, const UInt8* data, UInt32 size, UInt32 valueCount,
                                              const UInt8* validity, UInt32 nullCount, CORE::CDataTableColumnBatch& out )
{GUCEF_TRACE;

    if ( !column.hasDictionary )
        return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "a dictionary encoded page in a column chunk without a dictionary page" );
    const UInt32 denseCount = valueCount - nullCount;
    const SParquetColumnInfo& info = *column.info;

    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    const UInt32* dense = GUCEF_NULL;
    if ( denseCount > 0 )
    {
        if ( size < 1 )
            return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "a dictionary encoded page has no bit width" );
        const UInt32 bitWidth = data[ 0 ];
        if ( bitWidth > CRleBitPackedDecoder::MaxBitWidth )
            return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "dictionary indices of bit width " + CORE::ToString( bitWidth ) + ", above 32" );
        status = CheckDecodedSize( column, (UInt64) denseCount * 4 );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        status = m_pool->Reserve( m_scratch, denseCount * 4 );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return FailColumn( column, status, "no buffer for dictionary indices" );
        UInt32* indices = static_cast< UInt32* >( m_scratch->GetData() );
        m_decoder.Reset( data + 1, size - 1, bitWidth );
        const UInt32 decoded = m_decoder.GetValues( indices, denseCount );
        if ( decoded < denseCount )
            return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "the dictionary indices of a page hold " + CORE::ToString( decoded ) + " of " +
                               CORE::ToString( denseCount ) + " values" + ( m_decoder.HasFailed() ? CORE::CString( ": " ) + m_decoder.GetError() : CORE::CString::Empty ) );
        // one compare for the page instead of a branch per value
        UInt32 maxIndex = 0;
        for ( UInt32 k=0; k<denseCount; ++k )
            maxIndex = indices[ k ] > maxIndex ? indices[ k ] : maxIndex;
        if ( maxIndex >= column.dictionaryCount )
            return FailColumn( column, CORE::DATATABLE_STATUS_CORRUPT_DATA, "dictionary index " + CORE::ToString( maxIndex ) + " is outside the dictionary of " +
                               CORE::ToString( column.dictionaryCount ) + " values" );
        dense = indices;
    }

    // the output is bounded like every decoded page: indices or values of every row
    const UInt64 outputBytes = (UInt64) valueCount * ( 0 != info.storageWidth ? ( info.storageWidth > 4 ? info.storageWidth : 4 ) : 4 );
    status = CheckDecodedSize( column, outputBytes );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    out.SetType( info.column );
    switch ( column.dictionary.GetStorage() )
    {
        case CORE::DATATABLE_STORAGE_VARIABLE:
        {
            if ( m_request.materializeDictionaries || 0 == column.dictionaryCount )
            {
                status = MaterializeDictionary( column, dense, valueCount, validity, out );
                break;
            }
            CORE::CDataTableBufferPtr indexBuffer;
            const UInt32 bytes = valueCount * 4;
            status = m_pool->Acquire( bytes > 0 ? bytes : 1, indexBuffer );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return FailColumn( column, status, "no buffer for dictionary indices" );
            indexBuffer->SetDataSize( bytes );
            UInt32* target = static_cast< UInt32* >( indexBuffer->GetData() );
            if ( GUCEF_NULL == validity )
            {
                if ( valueCount > 0 )
                    memcpy( target, dense, bytes );
            }
            else
            {
                UInt32 k = 0;
                for ( UInt32 r=0; r<valueCount; ++r )
                    target[ r ] = BitIsSet( validity, r ) ? dense[ k++ ] : 0;
            }
            out.SetDictionaryValues( valueCount, target, indexBuffer, column.dictionaryBatch, 0 );
            break;
        }
        case CORE::DATATABLE_STORAGE_FIXED:
        {
            const UInt32 width = info.storageWidth;
            CORE::CDataTableBufferPtr values;
            const UInt32 bytes = valueCount * width;
            status = m_pool->Acquire( bytes > 0 ? bytes : 1, values );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return FailColumn( column, status, "no buffer for the values of a page" );
            values->SetDataSize( bytes );
            const void* dictionary = column.dictionary.GetFixedData();
            switch ( width )
            {
                case 1: { GatherFixed( static_cast< UInt8* >( values->GetData() ), static_cast< const UInt8* >( dictionary ), dense, valueCount, validity ); break; }
                case 2: { GatherFixed( static_cast< UInt16* >( values->GetData() ), static_cast< const UInt16* >( dictionary ), dense, valueCount, validity ); break; }
                case 4: { GatherFixed( static_cast< UInt32* >( values->GetData() ), static_cast< const UInt32* >( dictionary ), dense, valueCount, validity ); break; }
                default: { GatherFixed( static_cast< UInt64* >( values->GetData() ), static_cast< const UInt64* >( dictionary ), dense, valueCount, validity ); break; }
            }
            out.SetFixedValues( valueCount, values->GetData(), values );
            break;
        }
        case CORE::DATATABLE_STORAGE_BITS:
        {
            CORE::CDataTableBufferPtr bits;
            const UInt32 bytes = ( valueCount + 7 ) / 8;
            status = AcquireZeroed( bytes > 0 ? bytes : 1, bits );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return FailColumn( column, status, "no buffer for booleans" );
            const UInt8* dictionary = column.dictionary.GetBoolBits();
            UInt8* target = static_cast< UInt8* >( bits->GetData() );
            UInt32 k = 0;
            for ( UInt32 r=0; r<valueCount; ++r )
            {
                if ( GUCEF_NULL != validity && !BitIsSet( validity, r ) )
                    continue;
                if ( BitIsSet( dictionary, dense[ k++ ] ) )
                    SetBit( target, r );
            }
            out.SetFixedValues( valueCount, target, bits );
            break;
        }
        default:
        {
            // an empty dictionary of a fixed type: only nulls can refer to it
            CORE::CDataTableBufferPtr values;
            const UInt32 width = 0 != info.storageWidth ? info.storageWidth : 1;
            const UInt32 bytes = PARQUET_CONVERT_BITS == info.conversion ? ( valueCount + 7 ) / 8 : valueCount * width;
            status = AcquireZeroed( bytes > 0 ? bytes : 1, values );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return FailColumn( column, status, "no buffer for the values of a page" );
            out.SetFixedValues( valueCount, values->GetData(), values );
            break;
        }
    }
    if ( CORE::DATATABLE_STATUS_OK == status && GUCEF_NULL != m_request.stats )
        ++m_request.stats->pagesDecoded;
    return status;
}

/*-------------------------------------------------------------------------*/

bool
CParquetTableReader::DictionaryMayMatch( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_request.query || m_request.query->schema.IsNULL() )
        return true;
    const CORE::CDataTableResolvedQuery& query = *m_request.query;
    for ( size_t i=0; i<query.predicates.size(); ++i )
    {
        const CORE::CDataTableResolvedPredicate& predicate = query.predicates[ i ];
        if ( ( CORE::DATATABLE_PREDICATE_EQUALS != predicate.op && CORE::DATATABLE_PREDICATE_IN != predicate.op ) ||
             predicate.matchesNothing || predicate.matchesAllNonNull )
            continue;
        if ( predicate.columnIndex >= query.schema->GetColumnCount() )
            continue;
        UInt32 fileColumn = predicate.columnIndex;
        if ( !m_request.fileColumnOfTableColumn.empty() )
        {
            if ( predicate.columnIndex >= m_request.fileColumnOfTableColumn.size() )
                continue;
            fileColumn = m_request.fileColumnOfTableColumn[ predicate.columnIndex ];
        }
        const SColumnState* column = GUCEF_NULL;
        for ( size_t p=0; p<m_columns.size(); ++p )
        {
            if ( m_columns[ p ].fileColumn == fileColumn )
            {
                column = &m_columns[ p ];
                break;
            }
        }
        if ( GUCEF_NULL == column || !column->hasDictionary || !column->meta->hasEncodingStats )
            continue;
        if ( !column->info->column.IsSameType( query.schema->GetColumn( predicate.columnIndex ) ) )
            continue;

        // only a chunk whose data pages are all dictionary encoded has no value outside the dictionary
        bool allDictionary = true;
        bool anyDataPage = false;
        for ( size_t s=0; s<column->meta->encodingStats.size(); ++s )
        {
            const SParquetPageEncodingStats& stats = column->meta->encodingStats[ s ];
            if ( PARQUET_PAGE_DATA_PAGE != stats.pageType && PARQUET_PAGE_DATA_PAGE_V2 != stats.pageType )
                continue;
            if ( stats.count <= 0 )
                continue;
            anyDataPage = true;
            if ( PARQUET_ENCODING_PLAIN_DICTIONARY != stats.encoding && PARQUET_ENCODING_RLE_DICTIONARY != stats.encoding )
                allDictionary = false;
        }
        if ( !allDictionary || !anyDataPage )
            continue;

        // the literals: the operand of EQUALS, the members of IN
        const bool isEquals = CORE::DATATABLE_PREDICATE_EQUALS == predicate.op;
        const size_t literalCount = isEquals ? 1 : predicate.values.size();
        const CORE::CDataTableColumnBatch& dictionary = column->dictionary;
        const CORE::TDataTableLogicalType type = column->info->column.type;
        const UInt32 entries = column->dictionaryCount;
        if ( (UInt64) entries * literalCount > 16ULL * 1024 * 1024 )
            continue;   // too much work to be worth it, the filter does it per row

        bool found = false;
        if ( CORE::DATATABLE_STORAGE_VARIABLE == dictionary.GetStorage() )
        {
            for ( size_t l=0; !found && l<literalCount; ++l )
            {
                UInt32 literalLength = 0;
                const char* literal = CORE::CDataTablePruner::GetNativeBytes( isEquals ? predicate.low : predicate.values[ l ], literalLength );
                for ( UInt32 e=0; e<entries; ++e )
                {
                    UInt32 length = 0;
                    const char* value = dictionary.GetVariableValue( e, length );
                    if ( length == literalLength && ( 0 == length || 0 == memcmp( value, literal, length ) ) )
                    {
                        found = true;
                        break;
                    }
                }
            }
        }
        else
        if ( CORE::DATATABLE_STORAGE_FIXED == dictionary.GetStorage() && CORE::DATATABLE_TYPE_FLOAT32 != type && CORE::DATATABLE_TYPE_FLOAT64 != type )
        {
            const bool isUnsigned = CORE::DataTableLogicalTypeIsUnsigned( type );
            const UInt32 width = column->info->storageWidth;
            const UInt8* values = static_cast< const UInt8* >( dictionary.GetFixedData() );
            for ( size_t l=0; !found && l<literalCount; ++l )
            {
                const CORE::CVariant& literal = isEquals ? predicate.low : predicate.values[ l ];
                const Int64 signedLiteral = literal.AsInt64();
                const UInt64 unsignedLiteral = literal.AsUInt64();
                for ( UInt32 e=0; e<entries && !found; ++e )
                {
                    // the native values of the dictionary, read at their width
                    const UInt8* p = values + (size_t) e * width;
                    Int64 signedValue = 0;
                    UInt64 unsignedValue = 0;
                    switch ( width )
                    {
                        case 1: { signedValue = *reinterpret_cast< const Int8* >( p ); unsignedValue = *p; break; }
                        case 2: { Int16 v; memcpy( &v, p, 2 ); signedValue = v; UInt16 u; memcpy( &u, p, 2 ); unsignedValue = u; break; }
                        case 4: { Int32 v; memcpy( &v, p, 4 ); signedValue = v; UInt32 u; memcpy( &u, p, 4 ); unsignedValue = u; break; }
                        default: { memcpy( &signedValue, p, 8 ); memcpy( &unsignedValue, p, 8 ); break; }
                    }
                    found = isUnsigned ? unsignedValue == unsignedLiteral : signedValue == signedLiteral;
                }
            }
        }
        else
        {
            continue;
        }
        if ( !found )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::EnsurePage( SColumnState& column, UInt64 row )
{
    while ( !column.pageLoaded || row >= column.pageFirstRow + column.pageRows )
    {
        bool gotDataPage = false;
        CORE::TDataTableStatus status = ReadPage( column, false, gotDataPage );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::FillColumn( SColumnState& column, UInt64 firstRow, UInt32 rowCount, CORE::CDataTableColumnBatch& out )
{GUCEF_TRACE;

    CORE::TDataTableStatus status = EnsurePage( column, firstRow );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    const UInt64 pageEnd = column.pageFirstRow + column.pageRows;
    if ( firstRow + rowCount <= pageEnd )
    {
        // inside one page: a view of it, in place
        status = out.SetSlice( column.page, (UInt32) ( firstRow - column.pageFirstRow ), rowCount, *m_pool );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return FailColumn( column, status, "a page cannot be sliced" );
        return CORE::DATATABLE_STATUS_OK;
    }

    CGather gather( *m_pool, column.info->column, rowCount );
    UInt64 row = firstRow;
    UInt32 remaining = rowCount;
    for ( ;; )
    {
        const UInt64 end = column.pageFirstRow + column.pageRows;
        const UInt32 take = (UInt32) ( end - row < remaining ? end - row : remaining );
        if ( take > 0 )
        {
            status = gather.Append( column.page, (UInt32) ( row - column.pageFirstRow ), take );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return FailColumn( column, status, "the rows of several pages cannot be gathered" );
            row += take;
            remaining -= take;
        }
        if ( 0 == remaining )
            break;
        status = EnsurePage( column, row );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
    }
    gather.Finish( out );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableReader::ReadNextBatch( CORE::CDataTableBatchPtr& batch )
{GUCEF_TRACE;

    batch.Reset();
    if ( m_failed )
        return m_failedStatus;
    if ( !m_open )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "ReadNextBatch before OpenRowGroup" );
    if ( m_nextRow >= m_rowGroupRows )
        return CORE::DATATABLE_STATUS_END;

    const UInt64 left = m_rowGroupRows - m_nextRow;
    const UInt32 rowCount = (UInt32) ( left < m_batchRowCount ? left : m_batchRowCount );

    CORE::CDataTableBatchPtr result;
    CORE::TDataTableStatus status = m_pool->AcquireBatch( result );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, "No batch object" );
    CORE::CDataTableBatch* writable = result.GetWritable();
    if ( !writable->SetColumnCount( (UInt32) m_columns.size() ) )
        return Fail( CORE::DATATABLE_STATUS_OUT_OF_MEMORY, "No column objects for a batch" );
    writable->SetSchema( m_metaData.GetSchema() );
    writable->SetRowCount( rowCount );
    for ( size_t p=0; p<m_columns.size(); ++p )
    {
        writable->SetSchemaColumnIndex( (UInt32) p, m_columns[ p ].fileColumn );
        status = FillColumn( m_columns[ p ], m_nextRow, rowCount, writable->GetMutableColumn( (UInt32) p ) );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
    }
    CORE::CDataTableBatchOrigin origin;
    origin.fileOrdinal = m_request.fileOrdinal;
    origin.rowGroup = m_rowGroup;
    origin.firstRowInRowGroup = m_nextRow;
    writable->SetOrigin( origin );
    m_nextRow += rowCount;
    batch.Swap( result );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PARQUET */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
