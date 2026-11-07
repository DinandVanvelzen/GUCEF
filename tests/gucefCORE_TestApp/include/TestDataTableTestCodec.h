/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_TESTAPP_TESTDATATABLETESTCODEC_H
#define GUCEF_CORE_TESTAPP_TESTDATATABLETESTCODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CITABLECODEC_H
#include "gucefCORE_CITableCodec.h"
#define GUCEF_CORE_CITABLECODEC_H
#endif /* GUCEF_CORE_CITABLECODEC_H ? */

#ifndef GUCEF_CORE_CITABLEREADER_H
#include "gucefCORE_CITableReader.h"
#define GUCEF_CORE_CITABLEREADER_H
#endif /* GUCEF_CORE_CITABLEREADER_H ? */

#ifndef GUCEF_CORE_CITABLEWRITER_H
#include "gucefCORE_CITableWriter.h"
#define GUCEF_CORE_CITABLEWRITER_H
#endif /* GUCEF_CORE_CITABLEWRITER_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef GUCEF_CORE_CDATATABLEMERGE_H
#include "gucefCORE_CDataTableMerge.h"
#define GUCEF_CORE_CDATATABLEMERGE_H
#endif /* GUCEF_CORE_CDATATABLEMERGE_H ? */

#ifndef GUCEF_CORE_CDATATABLESCANSTATS_H
#include "gucefCORE_CDataTableScanStats.h"
#define GUCEF_CORE_CDATATABLESCANSTATS_H
#endif /* GUCEF_CORE_CDATATABLESCANSTATS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace CORETEST {

/**
 *  A small binary row group format for the tests of the generic table code (CFileDataTable, CDataTableScan), the
 *  only table codec gucefCORE has a use for before the CSV and parquet codecs: type "dttest", extension "dtt".
 *
 *  Layout, native byte order: "DTT1"; the row groups, each at an offset that is a multiple of 8 and each column in
 *  it as a null count, the validity bits when there are nulls, then the values: fixed width values, the bits of a
 *  bool, or the offsets and bytes of text and binary in the terminated form of the batches, every part padded to 8
 *  bytes; then the footer: the schema (name, type text, nullable per column), per row group its offset, byte length
 *  and row count, the key value metadata, the footer offset and "DTTF".
 *
 *  A reader reads a row group with one Setpos and Read (under the shared file lock when there is one) into a pooled
 *  buffer and delivers spans pointing into it: the in place path. The metadata reader decodes every row group to
 *  give exact statistics, unless the option "statistics" is false.
 *
 *  Options for fault tests: "failOpenRowGroup" (row group index) makes OpenRowGroup of that row group fail with
 *  CORRUPT_DATA, "failReadBatch" (n) makes the n-th ReadNextBatch of every reader fail, "decodeDelayMs" sleeps in
 *  every ReadNextBatch, "pruneRowGroup" (index) makes OpenRowGroup of that row group return PRUNED.
 */
class CTestTableCodec;

/*-------------------------------------------------------------------------*/

/** Bytes of a value of a row group part, padded to 8 */
inline GUCEF::UInt64
TestCodecPadded( GUCEF::UInt64 bytes )
{
    return ( bytes + 7 ) & ~(GUCEF::UInt64) 7;
}

/*-------------------------------------------------------------------------*/

class CTestTableFileMetaData : public GUCEF::CORE::CITableFileMetaData
{
    public:

    struct SRowGroup
    {
        GUCEF::UInt64 offset;
        GUCEF::UInt64 byteLength;
        GUCEF::UInt32 rowCount;
    };
    typedef GUCEF::vector< SRowGroup > TRowGroupVector;
    typedef GUCEF::vector< GUCEF::CORE::CDataTableRowGroupInfo > TRowGroupInfoVector;

    GUCEF::CORE::CDataTableSchemaPtr schema;
    TRowGroupVector rowGroups;
    TRowGroupInfoVector infos;      /**< empty without statistics */
    GUCEF::UInt64 rowCount;
    GUCEF::CORE::CValueList keyValues;

    CTestTableFileMetaData( void ) : GUCEF::CORE::CITableFileMetaData(), schema(), rowGroups(), infos(), rowCount( 0 ), keyValues() {}

    virtual const GUCEF::CORE::CDataTableSchemaPtr& GetSchema( void ) const GUCEF_VIRTUAL_OVERRIDE { return schema; }
    virtual GUCEF::UInt64 GetRowCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return rowCount; }
    virtual GUCEF::UInt32 GetRowGroupCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return (GUCEF::UInt32) rowGroups.size(); }

    virtual bool GetRowGroupInfo( GUCEF::UInt32 rowGroup, GUCEF::CORE::CDataTableRowGroupInfo& info ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( rowGroup >= (GUCEF::UInt32) rowGroups.size() )
            return false;
        if ( rowGroup < (GUCEF::UInt32) infos.size() )
        {
            info = infos[ rowGroup ];
            return true;
        }
        // no statistics: the row count only
        info.Clear();
        info.rowCount = rowGroups[ rowGroup ].rowCount;
        info.columnStats.resize( schema->GetColumnCount() );
        return true;
    }

    virtual GUCEF::UInt64 GetMemoryFootprint( void ) const GUCEF_VIRTUAL_OVERRIDE { return sizeof( *this ) + rowGroups.size() * sizeof( SRowGroup ); }
    virtual void GetKeyValueMetaData( GUCEF::CORE::CValueList& out ) const GUCEF_VIRTUAL_OVERRIDE { out = keyValues; }
};

/*-------------------------------------------------------------------------*/

/** Decodes the columns of a row group held in a buffer into column batches pointing into it */
inline GUCEF::CORE::TDataTableStatus
TestCodecDecodeRowGroup( const GUCEF::CORE::CDataTableSchema& schema         ,
                         const GUCEF::CORE::CDataTableBufferPtr& chunk       ,
                         GUCEF::UInt32 rowCount                              ,
                         GUCEF::vector< GUCEF::CORE::CDataTableColumnBatch >& columns ,
                         GUCEF::CORE::CString& error                         )
{
    using namespace GUCEF;
    using namespace GUCEF::CORE;

    const char* base = static_cast< const char* >( chunk->GetData() );
    const UInt64 size = chunk->GetDataSize();
    UInt64 pos = 0;
    const UInt32 columnCount = schema.GetColumnCount();
    columns.resize( columnCount );
    for ( UInt32 c=0; c<columnCount; ++c )
    {
        const CDataTableColumnDef& def = schema.GetColumn( c );
        CDataTableColumnBatch& column = columns[ c ];
        column.SetType( def );
        if ( pos + 8 > size )
        {
            error = "Row group truncated";
            return DATATABLE_STATUS_CORRUPT_DATA;
        }
        UInt32 nullCount = 0;
        memcpy( &nullCount, base + pos, 4 );
        pos += 8;
        const UInt8* validity = GUCEF_NULL;
        const UInt64 bitmapBytes = ( rowCount + 7 ) / 8;
        if ( nullCount > 0 )
        {
            if ( pos + bitmapBytes > size )
            {
                error = "Validity truncated";
                return DATATABLE_STATUS_CORRUPT_DATA;
            }
            validity = reinterpret_cast< const UInt8* >( base + pos );
            pos += TestCodecPadded( bitmapBytes );
        }
        switch ( DataTableLogicalTypeStorage( def.type ) )
        {
            case DATATABLE_STORAGE_FIXED:
            case DATATABLE_STORAGE_BITS:
            {
                const UInt32 width = DataTableLogicalTypeByteWidth( def.type );
                const UInt64 bytes = 0 == width ? bitmapBytes : (UInt64) rowCount * width;
                if ( pos + bytes > size )
                {
                    error = "Values truncated";
                    return DATATABLE_STATUS_CORRUPT_DATA;
                }
                column.SetFixedValues( rowCount, base + pos, chunk );
                pos += TestCodecPadded( bytes );
                break;
            }
            case DATATABLE_STORAGE_VARIABLE:
            {
                const UInt64 offsetBytes = (UInt64) ( rowCount + 1 ) * 4;
                if ( pos + offsetBytes > size )
                {
                    error = "Offsets truncated";
                    return DATATABLE_STATUS_CORRUPT_DATA;
                }
                const UInt32* offsets = reinterpret_cast< const UInt32* >( base + pos );
                pos += TestCodecPadded( offsetBytes );
                const UInt64 bytes = offsets[ rowCount ];
                if ( pos + bytes > size )
                {
                    error = "Bytes truncated";
                    return DATATABLE_STATUS_CORRUPT_DATA;
                }
                column.SetVariableValues( rowCount, offsets, chunk, base + pos, chunk );
                pos += TestCodecPadded( bytes );
                break;
            }
            default:
            {
                error = "Unsupported column type";
                return DATATABLE_STATUS_UNSUPPORTED;
            }
        }
        if ( GUCEF_NULL != validity )
            column.SetValidity( validity, nullCount, chunk );
    }
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

/** Reads a row group into a pooled buffer, under the shared lock when there is one */
inline GUCEF::CORE::TDataTableStatus
TestCodecReadChunk( GUCEF::CORE::CIOAccess& input, GUCEF::MT::CMutex* sharedLock, GUCEF::UInt64 offset, GUCEF::UInt64 byteLength,
                    GUCEF::CORE::CDataTableBufferPool& pool, GUCEF::CORE::CDataTableBufferPtr& chunk )
{
    using namespace GUCEF;
    using namespace GUCEF::CORE;

    if ( byteLength > GUCEF_UINT32MAX - DataTableBufferPadding )
        return DATATABLE_STATUS_LIMIT_EXCEEDED;
    TDataTableStatus status = pool.Acquire( (UInt32) byteLength, chunk );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    UInt32 read = 0;
    if ( GUCEF_NULL != sharedLock )
        sharedLock->Lock();
    if ( 0 == input.Setpos( offset ) )
        read = input.Read( chunk->GetData(), 1, (UInt32) byteLength );
    if ( GUCEF_NULL != sharedLock )
        sharedLock->Unlock();
    if ( read != byteLength )
        return DATATABLE_STATUS_IO_ERROR;
    chunk->SetDataSize( (UInt32) byteLength );
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

class CTestTableReader : public GUCEF::CORE::CITableReader
{
    public:

    CTestTableReader( const GUCEF::CORE::IOAccessPtr& input, const GUCEF::CORE::CITableFileMetaDataPtr& metaData, const GUCEF::CORE::CValueList& options )
        : GUCEF::CORE::CITableReader()
        , m_input( input )
        , m_metaData( metaData )
        , m_request( GUCEF_NULL )
        , m_columns()
        , m_chunk()
        , m_rowCount( 0 )
        , m_position( 0 )
        , m_rowGroup( 0 )
        , m_readCalls( 0 )
        , m_failOpenRowGroup( options.GetValueAlways( "failOpenRowGroup", GUCEF::CORE::CVariant( (GUCEF::Int64) -1 ) ).AsInt64() )
        , m_pruneRowGroup( options.GetValueAlways( "pruneRowGroup", GUCEF::CORE::CVariant( (GUCEF::Int64) -1 ) ).AsInt64() )
        , m_failReadBatch( options.GetValueAlways( "failReadBatch", GUCEF::CORE::CVariant( (GUCEF::Int64) -1 ) ).AsInt64() )
        , m_decodeDelayMs( options.GetValueAlways( "decodeDelayMs", GUCEF::CORE::CVariant( (GUCEF::Int64) 0 ) ).AsUInt32() )
        , m_error()
    {
    }

    virtual const GUCEF::CORE::CITableFileMetaDataPtr& GetMetaData( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_metaData; }

    virtual GUCEF::CORE::TDataTableStatus OpenRowGroup( GUCEF::UInt32 rowGroup, const GUCEF::CORE::CDataTableReadRequest& request ) GUCEF_VIRTUAL_OVERRIDE
    {
        using namespace GUCEF::CORE;
        m_request = GUCEF_NULL;
        m_chunk.Reset();
        const CTestTableFileMetaData& metaData = static_cast< const CTestTableFileMetaData& >( *m_metaData );
        if ( rowGroup >= (GUCEF::UInt32) metaData.rowGroups.size() || GUCEF_NULL == request.bufferPool )
        {
            m_error = "Invalid row group or request";
            return DATATABLE_STATUS_INVALID_ARGUMENT;
        }
        if ( (GUCEF::Int64) rowGroup == m_pruneRowGroup )
            return DATATABLE_STATUS_PRUNED;
        if ( (GUCEF::Int64) rowGroup == m_failOpenRowGroup )
        {
            m_error = "Row group " + ToString( rowGroup ) + " is damaged (injected)";
            return DATATABLE_STATUS_CORRUPT_DATA;
        }
        const CTestTableFileMetaData::SRowGroup& info = metaData.rowGroups[ rowGroup ];
        TDataTableStatus status = TestCodecReadChunk( *m_input, request.sharedFileLock, info.offset, info.byteLength, *request.bufferPool, m_chunk );
        if ( DATATABLE_STATUS_OK != status )
        {
            m_error = "Reading row group " + ToString( rowGroup ) + " failed";
            return status;
        }
        if ( GUCEF_NULL != request.stats )
            request.stats->bytesRead += info.byteLength;
        status = TestCodecDecodeRowGroup( *metaData.schema, m_chunk, info.rowCount, m_columns, m_error );
        if ( DATATABLE_STATUS_OK != status )
            return status;
        m_request = &request;
        m_rowGroup = rowGroup;
        m_rowCount = info.rowCount;
        m_position = 0;
        return DATATABLE_STATUS_OK;
    }

    virtual GUCEF::CORE::TDataTableStatus ReadNextBatch( GUCEF::CORE::CDataTableBatchPtr& batch ) GUCEF_VIRTUAL_OVERRIDE
    {
        using namespace GUCEF;
        using namespace GUCEF::CORE;
        batch.Reset();
        if ( GUCEF_NULL == m_request )
        {
            m_error = "No row group is open";
            return DATATABLE_STATUS_INVALID_ARGUMENT;
        }
        if ( m_position >= m_rowCount )
        {
            // the chunk goes once the row group is done; batches handed out keep it
            m_chunk.Reset();
            return DATATABLE_STATUS_END;
        }
        ++m_readCalls;
        if ( m_failReadBatch >= 0 && (Int64) m_readCalls == m_failReadBatch )
        {
            m_error = "Batch " + ToString( m_readCalls ) + " cannot be decoded (injected)";
            return DATATABLE_STATUS_CORRUPT_DATA;
        }
        if ( m_decodeDelayMs > 0 )
            GUCEF::MT::PrecisionDelay( m_decodeDelayMs );

        UInt32 count = m_rowCount - m_position;
        if ( count > m_request->batchRowCount )
            count = m_request->batchRowCount;
        CDataTableBufferPool& pool = *m_request->bufferPool;
        TDataTableStatus status = pool.AcquireBatch( batch );
        if ( DATATABLE_STATUS_OK != status )
            return status;
        CDataTableBatch* writable = batch.GetWritable();
        const UInt32 columnCount = (UInt32) m_request->projection.size();
        if ( !writable->SetColumnCount( columnCount ) )
            return DATATABLE_STATUS_OUT_OF_MEMORY;
        writable->SetSchema( m_metaData->GetSchema() );
        writable->SetRowCount( count );
        for ( UInt32 i=0; i<columnCount; ++i )
        {
            const UInt32 column = m_request->projection[ i ];
            status = writable->GetMutableColumn( i ).SetSlice( m_columns[ column ], m_position, count, pool );
            if ( DATATABLE_STATUS_OK != status )
                return status;
            writable->SetSchemaColumnIndex( i, column );
        }
        CDataTableBatchOrigin origin;
        origin.fileOrdinal = m_request->fileOrdinal;
        origin.rowGroup = m_rowGroup;
        origin.firstRowInRowGroup = m_position;
        writable->SetOrigin( origin );
        if ( GUCEF_NULL != m_request->stats )
            m_request->stats->pagesInPlace += columnCount;
        m_position += count;
        return DATATABLE_STATUS_OK;
    }

    virtual GUCEF::CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_error; }

    private:

    GUCEF::CORE::IOAccessPtr m_input;
    GUCEF::CORE::CITableFileMetaDataPtr m_metaData;
    const GUCEF::CORE::CDataTableReadRequest* m_request;
    GUCEF::vector< GUCEF::CORE::CDataTableColumnBatch > m_columns;
    GUCEF::CORE::CDataTableBufferPtr m_chunk;
    GUCEF::UInt32 m_rowCount;
    GUCEF::UInt32 m_position;
    GUCEF::UInt32 m_rowGroup;
    GUCEF::UInt32 m_readCalls;
    GUCEF::Int64 m_failOpenRowGroup;
    GUCEF::Int64 m_pruneRowGroup;
    GUCEF::Int64 m_failReadBatch;
    GUCEF::UInt32 m_decodeDelayMs;
    GUCEF::CORE::CString m_error;
};

/*-------------------------------------------------------------------------*/

class CTestTableWriter : public GUCEF::CORE::CITableWriter
{
    public:

    CTestTableWriter( const GUCEF::CORE::IOAccessPtr& output, const GUCEF::CORE::CDataTableSchemaPtr& schema, const GUCEF::CORE::CDataTableWriterOptions& options )
        : GUCEF::CORE::CITableWriter()
        , m_output( output )
        , m_schema( schema )
        , m_options( options )
        , m_pending()
        , m_rowGroups()
        , m_position( 0 )
        , m_rowCount( 0 )
        , m_started( false )
        , m_finished( false )
        , m_error()
    {
    }

    virtual ~CTestTableWriter() GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !m_finished )
            Abort();
    }

    virtual GUCEF::CORE::TDataTableStatus WriteBatch( const GUCEF::CORE::CDataTableBatch& batch ) GUCEF_VIRTUAL_OVERRIDE
    {
        using namespace GUCEF::CORE;
        if ( m_finished )
            return DATATABLE_STATUS_INVALID_ARGUMENT;
        CDataTableBatchPtr normalized;
        TDataTableStatus status = CDataTableMerge::Normalize( m_schema, batch, GUCEF::CORE::CCoreGlobal::Instance()->GetDataTableBufferPool(), normalized, m_error );
        if ( DATATABLE_STATUS_OK != status )
            return status;
        m_pending.push_back( normalized );
        GUCEF::UInt64 pendingRows = 0;
        for ( GUCEF::UInt32 i=0; i<(GUCEF::UInt32) m_pending.size(); ++i )
            pendingRows += m_pending[ i ]->GetRowCount();
        if ( pendingRows >= m_options.rowGroupRowCount )
            return FlushRowGroup();
        return DATATABLE_STATUS_OK;
    }

    virtual GUCEF::CORE::TDataTableStatus FlushRowGroup( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        using namespace GUCEF;
        using namespace GUCEF::CORE;
        if ( m_pending.empty() )
            return DATATABLE_STATUS_OK;
        if ( !Start() )
            return DATATABLE_STATUS_IO_ERROR;

        UInt32 rows = 0;
        for ( UInt32 i=0; i<(UInt32) m_pending.size(); ++i )
            rows += m_pending[ i ]->GetRowCount();

        CTestTableFileMetaData::SRowGroup rowGroup;
        rowGroup.offset = m_position;
        rowGroup.rowCount = rows;
        const UInt32 columnCount = m_schema->GetColumnCount();
        const UInt32 bitmapBytes = ( rows + 7 ) / 8;
        GUCEF::vector< UInt8 > scratch;
        for ( UInt32 c=0; c<columnCount; ++c )
        {
            const CDataTableColumnDef& def = m_schema->GetColumn( c );
            UInt32 nullCount = 0;
            for ( UInt32 b=0; b<(UInt32) m_pending.size(); ++b )
                nullCount += m_pending[ b ]->GetColumn( c ).GetNullCount();
            UInt8 header[ 8 ] = { 0 };
            memcpy( header, &nullCount, 4 );
            if ( !Put( header, 8 ) )
                return DATATABLE_STATUS_IO_ERROR;
            if ( nullCount > 0 )
            {
                scratch.assign( bitmapBytes, 0 );
                UInt32 row = 0;
                for ( UInt32 b=0; b<(UInt32) m_pending.size(); ++b )
                {
                    const CDataTableColumnBatch& column = m_pending[ b ]->GetColumn( c );
                    for ( UInt32 r=0; r<column.GetRowCount(); ++r, ++row )
                        if ( column.IsValid( r ) )
                            scratch[ row >> 3 ] = (UInt8) ( scratch[ row >> 3 ] | ( 1u << ( row & 7u ) ) );
                }
                if ( !PutPadded( &scratch[ 0 ], bitmapBytes ) )
                    return DATATABLE_STATUS_IO_ERROR;
            }
            switch ( DataTableLogicalTypeStorage( def.type ) )
            {
                case DATATABLE_STORAGE_FIXED:
                {
                    const UInt32 width = DataTableLogicalTypeByteWidth( def.type );
                    scratch.resize( (size_t) rows * width + 1 );
                    UInt64 at = 0;
                    for ( UInt32 b=0; b<(UInt32) m_pending.size(); ++b )
                    {
                        const CDataTableColumnBatch& column = m_pending[ b ]->GetColumn( c );
                        const UInt32 bytes = column.GetRowCount() * width;
                        if ( bytes > 0 )
                            memcpy( &scratch[ (size_t) at ], column.GetFixedData(), bytes );
                        at += bytes;
                    }
                    if ( !PutPadded( &scratch[ 0 ], (UInt64) rows * width ) )
                        return DATATABLE_STATUS_IO_ERROR;
                    break;
                }
                case DATATABLE_STORAGE_BITS:
                {
                    scratch.assign( bitmapBytes + 1, 0 );
                    UInt32 row = 0;
                    for ( UInt32 b=0; b<(UInt32) m_pending.size(); ++b )
                    {
                        const CDataTableColumnBatch& column = m_pending[ b ]->GetColumn( c );
                        const UInt8* bits = column.GetBoolBits();
                        for ( UInt32 r=0; r<column.GetRowCount(); ++r, ++row )
                            if ( 0 != ( bits[ r >> 3 ] & ( 1u << ( r & 7u ) ) ) )
                                scratch[ row >> 3 ] = (UInt8) ( scratch[ row >> 3 ] | ( 1u << ( row & 7u ) ) );
                    }
                    if ( !PutPadded( &scratch[ 0 ], bitmapBytes ) )
                        return DATATABLE_STATUS_IO_ERROR;
                    break;
                }
                case DATATABLE_STORAGE_VARIABLE:
                {
                    GUCEF::vector< UInt32 > offsets;
                    GUCEF::vector< char > bytes;
                    offsets.push_back( 0 );
                    for ( UInt32 b=0; b<(UInt32) m_pending.size(); ++b )
                    {
                        const CDataTableColumnBatch& column = m_pending[ b ]->GetColumn( c );
                        for ( UInt32 r=0; r<column.GetRowCount(); ++r )
                        {
                            UInt32 length = 0;
                            const char* value = column.GetVariableValue( r, length );
                            if ( length > 0 )
                                bytes.insert( bytes.end(), value, value + length );
                            bytes.push_back( '\0' );
                            offsets.push_back( (UInt32) bytes.size() );
                        }
                    }
                    if ( !PutPadded( &offsets[ 0 ], (UInt64) offsets.size() * 4 ) ||
                         ( !bytes.empty() && !PutPadded( &bytes[ 0 ], bytes.size() ) ) )
                        return DATATABLE_STATUS_IO_ERROR;
                    break;
                }
                default:
                {
                    m_error = "Unsupported column type";
                    return DATATABLE_STATUS_UNSUPPORTED;
                }
            }
        }
        rowGroup.byteLength = m_position - rowGroup.offset;
        m_rowGroups.push_back( rowGroup );
        m_rowCount += rows;
        m_pending.clear();
        return DATATABLE_STATUS_OK;
    }

    virtual GUCEF::CORE::TDataTableStatus Finish( GUCEF::CORE::CDataTableFileSummary& summary ) GUCEF_VIRTUAL_OVERRIDE
    {
        using namespace GUCEF;
        using namespace GUCEF::CORE;
        TDataTableStatus status = FlushRowGroup();
        if ( DATATABLE_STATUS_OK != status )
            return status;
        if ( !Start() )
            return DATATABLE_STATUS_IO_ERROR;
        const UInt64 footerOffset = m_position;
        const UInt32 columnCount = m_schema->GetColumnCount();
        bool ok = PutUInt32( columnCount );
        for ( UInt32 c=0; c<columnCount && ok; ++c )
        {
            const CDataTableColumnDef& def = m_schema->GetColumn( c );
            ok = PutString( def.name ) && PutString( def.GetTypeText() );
            const UInt8 nullable = def.nullable ? 1 : 0;
            ok = ok && Put( &nullable, 1 );
        }
        ok = ok && PutUInt32( (UInt32) m_rowGroups.size() );
        for ( UInt32 i=0; i<(UInt32) m_rowGroups.size() && ok; ++i )
            ok = Put( &m_rowGroups[ i ].offset, 8 ) && Put( &m_rowGroups[ i ].byteLength, 8 ) && PutUInt32( m_rowGroups[ i ].rowCount );
        ok = ok && PutUInt32( m_options.keyValueMetaData.GetKeyCount() );
        CValueList::TValueMap::const_iterator i = m_options.keyValueMetaData.GetDataBeginIterator();
        while ( ok && i != m_options.keyValueMetaData.GetDataEndIterator() )
        {
            ok = PutString( (*i).first.AsString() ) && PutString( (*i).second.empty() ? CString::Empty : (*i).second.front().AsString() );
            ++i;
        }
        ok = ok && Put( &footerOffset, 8 ) && Put( "DTTF", 4 );
        if ( !ok )
            return DATATABLE_STATUS_IO_ERROR;
        m_finished = true;
        summary.Clear();
        summary.rowCount = m_rowCount;
        summary.rowGroupCount = (UInt32) m_rowGroups.size();
        summary.byteCount = m_position;
        return DATATABLE_STATUS_OK;
    }

    virtual void Abort( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_pending.clear();
        m_finished = true;
    }

    virtual GUCEF::CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_error; }

    private:

    bool Start( void )
    {
        if ( m_started )
            return true;
        m_started = true;
        return Put( "DTT1\0\0\0\0", 8 );
    }

    bool Put( const void* data, GUCEF::UInt64 bytes )
    {
        if ( 0 == bytes )
            return true;
        if ( m_output->Write( data, 1, (GUCEF::UInt32) bytes ) != bytes )
        {
            m_error = "Writing failed";
            return false;
        }
        m_position += bytes;
        return true;
    }

    bool PutPadded( const void* data, GUCEF::UInt64 bytes )
    {
        static const char zeros[ 8 ] = { 0 };
        return Put( data, bytes ) && Put( zeros, TestCodecPadded( bytes ) - bytes );
    }

    bool PutUInt32( GUCEF::UInt32 value ) { return Put( &value, 4 ); }

    bool PutString( const GUCEF::CORE::CString& text )
    {
        const GUCEF::CORE::CUtf8String utf8 = GUCEF::CORE::ToUtf8String( text );
        const GUCEF::UInt32 length = utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0;
        return PutUInt32( length ) && Put( utf8.C_String(), length );
    }

    GUCEF::CORE::IOAccessPtr m_output;
    GUCEF::CORE::CDataTableSchemaPtr m_schema;
    GUCEF::CORE::CDataTableWriterOptions m_options;
    GUCEF::vector< GUCEF::CORE::CDataTableBatchPtr > m_pending;
    CTestTableFileMetaData::TRowGroupVector m_rowGroups;
    GUCEF::UInt64 m_position;
    GUCEF::UInt64 m_rowCount;
    bool m_started;
    bool m_finished;
    GUCEF::CORE::CString m_error;
};

/*-------------------------------------------------------------------------*/

class CTestTableCodec : public GUCEF::CORE::CITableCodec
{
    public:

    CTestTableCodec( void ) : GUCEF::CORE::CITableCodec() {}

    virtual GUCEF::CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CTestTableCodec(); }

    virtual GUCEF::CORE::CString GetTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "dttest"; }
    virtual GUCEF::CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "Data table test codec"; }
    virtual GUCEF::CORE::CString GetCopyright( void ) const GUCEF_VIRTUAL_OVERRIDE { return "Copyright (C) Dinand Vanvelzen. 2002 - 2026"; }
    virtual GUCEF::CORE::TVersion GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE { GUCEF::CORE::TVersion version; version.major = 1; version.minor = 0; version.patch = 0; version.release = 0; return version; }

    virtual GUCEF::UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return GUCEF_TABLECODEC_CAPABILITY_READ | GUCEF_TABLECODEC_CAPABILITY_WRITE | GUCEF_TABLECODEC_CAPABILITY_ROW_GROUPS |
               GUCEF_TABLECODEC_CAPABILITY_STATISTICS | GUCEF_TABLECODEC_CAPABILITY_RANDOM_ACCESS | GUCEF_TABLECODEC_CAPABILITY_EMBEDDED_SCHEMA |
               GUCEF_TABLECODEC_CAPABILITY_IN_PLACE_PAGES;
    }

    virtual void GetFileExtensions( GUCEF::CORE::CString::StringSet& extensions ) const GUCEF_VIRTUAL_OVERRIDE { extensions.insert( "dtt" ); }

    virtual GUCEF::CORE::TDataTableStatus ReadMetaData( GUCEF::CORE::CIOAccess& source, const GUCEF::CORE::CValueList& options,
                                                        GUCEF::CORE::CITableFileMetaDataPtr& metaData, GUCEF::CORE::CString& error ) const GUCEF_VIRTUAL_OVERRIDE
    {
        using namespace GUCEF;
        using namespace GUCEF::CORE;

        const UInt64 size = source.GetSize();
        char magic[ 8 ] = { 0 };
        UInt64 footerOffset = 0;
        if ( size < 20 || 0 != source.Setpos( 0 ) || 8 != source.Read( magic, 1, 8 ) || 0 != memcmp( magic, "DTT1", 4 ) ||
             0 != source.Setpos( size - 12 ) || 8 != source.Read( &footerOffset, 1, 8 ) || 4 != source.Read( magic, 1, 4 ) ||
             0 != memcmp( magic, "DTTF", 4 ) || footerOffset >= size - 12 )
        {
            error = "Not a test table file";
            return DATATABLE_STATUS_CORRUPT_DATA;
        }

        const UInt32 footerBytes = (UInt32) ( size - 12 - footerOffset );
        GUCEF::vector< char > footer( footerBytes + 1 );
        if ( 0 != source.Setpos( footerOffset ) || footerBytes != source.Read( &footer[ 0 ], 1, footerBytes ) )
        {
            error = "Footer unreadable";
            return DATATABLE_STATUS_IO_ERROR;
        }
        UInt32 pos = 0;
        CTSharedPtr< CTestTableFileMetaData, GUCEF::MT::CMutex > result( GUCEF_NEW CTestTableFileMetaData() );
        CDataTableSchema schema;
        UInt32 columnCount = 0;
        if ( !Get( footer, pos, &columnCount, 4 ) )
            return Corrupt( error );
        for ( UInt32 c=0; c<columnCount; ++c )
        {
            CString name;
            CString typeText;
            UInt8 nullable = 0;
            if ( !GetString( footer, pos, name ) || !GetString( footer, pos, typeText ) || !Get( footer, pos, &nullable, 1 ) )
                return Corrupt( error );
            CDataTableColumnDef def( name, DATATABLE_TYPE_INT64, 0 != nullable );
            if ( !def.SetTypeFromText( typeText ) || !schema.AddColumn( def ) )
                return Corrupt( error );
        }
        result->schema = CreateDataTableSchemaPtr( schema );
        UInt32 rowGroupCount = 0;
        if ( !Get( footer, pos, &rowGroupCount, 4 ) )
            return Corrupt( error );
        for ( UInt32 i=0; i<rowGroupCount; ++i )
        {
            CTestTableFileMetaData::SRowGroup rowGroup;
            if ( !Get( footer, pos, &rowGroup.offset, 8 ) || !Get( footer, pos, &rowGroup.byteLength, 8 ) || !Get( footer, pos, &rowGroup.rowCount, 4 ) )
                return Corrupt( error );
            result->rowGroups.push_back( rowGroup );
            result->rowCount += rowGroup.rowCount;
        }
        UInt32 keyCount = 0;
        if ( !Get( footer, pos, &keyCount, 4 ) )
            return Corrupt( error );
        for ( UInt32 i=0; i<keyCount; ++i )
        {
            CString key;
            CString value;
            if ( !GetString( footer, pos, key ) || !GetString( footer, pos, value ) )
                return Corrupt( error );
            result->keyValues.Set( key, value );
        }

        if ( options.GetValueAlways( "statistics", CVariant( true ) ).AsBool( true ) )
        {
            // exact statistics: every row group decoded once
            CDataTableBufferPool& pool = CCoreGlobal::Instance()->GetDataTableBufferPool();
            for ( UInt32 i=0; i<rowGroupCount; ++i )
            {
                const CTestTableFileMetaData::SRowGroup& rowGroup = result->rowGroups[ i ];
                CDataTableBufferPtr chunk;
                TDataTableStatus status = TestCodecReadChunk( source, GUCEF_NULL, rowGroup.offset, rowGroup.byteLength, pool, chunk );
                GUCEF::vector< CDataTableColumnBatch > columns;
                if ( DATATABLE_STATUS_OK == status )
                    status = TestCodecDecodeRowGroup( schema, chunk, rowGroup.rowCount, columns, error );
                if ( DATATABLE_STATUS_OK != status )
                    return status;
                CDataTableBatchPtr batch;
                status = pool.AcquireBatch( batch );
                if ( DATATABLE_STATUS_OK != status )
                    return status;
                CDataTableBatch* writable = batch.GetWritable();
                writable->SetColumnCount( columnCount );
                writable->SetSchema( result->schema );
                writable->SetRowCount( rowGroup.rowCount );
                for ( UInt32 c=0; c<columnCount; ++c )
                {
                    writable->GetMutableColumn( c ) = columns[ c ];
                    writable->SetSchemaColumnIndex( c, c );
                }
                CDataTableRowGroupInfo info;
                CDataTableMerge::ComputeStats( *batch, info );
                result->infos.push_back( info );
            }
        }
        metaData = CITableFileMetaDataPtr( result );
        return DATATABLE_STATUS_OK;
    }

    virtual GUCEF::CORE::TDataTableStatus CreateReader( const GUCEF::CORE::IOAccessPtr& source, const GUCEF::CORE::CITableFileMetaDataPtr& metaData,
                                                        const GUCEF::CORE::CValueList& options, GUCEF::CORE::CITableReaderPtr& reader, GUCEF::CORE::CString& error ) const GUCEF_VIRTUAL_OVERRIDE
    {
        using namespace GUCEF::CORE;
        if ( source.IsNULL() || metaData.IsNULL() )
        {
            error = "No input or metadata";
            return DATATABLE_STATUS_INVALID_ARGUMENT;
        }
        reader = CITableReaderPtr( GUCEF_NEW CTestTableReader( source, metaData, options ) );
        return DATATABLE_STATUS_OK;
    }

    virtual GUCEF::CORE::TDataTableStatus CreateWriter( const GUCEF::CORE::IOAccessPtr& destination, const GUCEF::CORE::CDataTableSchemaPtr& schema,
                                                        const GUCEF::CORE::CDataTableWriterOptions& options, GUCEF::CORE::CITableWriterPtr& writer, GUCEF::CORE::CString& error ) const GUCEF_VIRTUAL_OVERRIDE
    {
        using namespace GUCEF::CORE;
        if ( destination.IsNULL() || schema.IsNULL() )
        {
            error = "No output or schema";
            return DATATABLE_STATUS_INVALID_ARGUMENT;
        }
        writer = CITableWriterPtr( GUCEF_NEW CTestTableWriter( destination, schema, options ) );
        return DATATABLE_STATUS_OK;
    }

    private:

    static GUCEF::CORE::TDataTableStatus Corrupt( GUCEF::CORE::CString& error )
    {
        error = "The footer is damaged";
        return GUCEF::CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }

    static bool Get( const GUCEF::vector< char >& data, GUCEF::UInt32& pos, void* out, GUCEF::UInt32 bytes )
    {
        if ( pos + bytes > (GUCEF::UInt32) data.size() )
            return false;
        memcpy( out, &data[ pos ], bytes );
        pos += bytes;
        return true;
    }

    static bool GetString( const GUCEF::vector< char >& data, GUCEF::UInt32& pos, GUCEF::CORE::CString& text )
    {
        GUCEF::UInt32 length = 0;
        if ( !Get( data, pos, &length, 4 ) || pos + length > (GUCEF::UInt32) data.size() )
            return false;
        GUCEF::CORE::CUtf8String utf8;
        if ( length > 0 )
            utf8.Append( &data[ pos ], length );
        pos += length;
        text = GUCEF::CORE::ToString( utf8 );
        return true;
    }
};

/*-------------------------------------------------------------------------*/

/** Writes full batches into an in memory test table file, one row group per batch; returns the input to read it */
inline GUCEF::CORE::TDataTableStatus
WriteTestTableFile( const GUCEF::vector< GUCEF::CORE::CDataTableBatchPtr >& batches ,
                    const GUCEF::CORE::CDataTableSchemaPtr& schema                ,
                    GUCEF::CORE::IOAccessPtr& file                                )
{
    using namespace GUCEF::CORE;
    CDynamicBufferAccess* access = GUCEF_NEW CDynamicBufferAccess( GUCEF_NEW CDynamicBuffer(), true );
    file = IOAccessPtr( access->CreateSharedPtr() );
    CTestTableCodec codec;
    CDataTableWriterOptions options;
    CITableWriterPtr writer;
    CString error;
    TDataTableStatus status = codec.CreateWriter( file, schema, options, writer, error );
    for ( GUCEF::UInt32 i=0; i<(GUCEF::UInt32) batches.size() && DATATABLE_STATUS_OK == status; ++i )
    {
        status = writer->WriteBatch( *batches[ i ] );
        if ( DATATABLE_STATUS_OK == status )
            status = writer->FlushRowGroup();
    }
    CDataTableFileSummary summary;
    if ( DATATABLE_STATUS_OK == status )
        status = writer->Finish( summary );
    file->Setpos( 0 );
    return status;
}

}; /* namespace CORETEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTDATATABLETESTCODEC_H ? */
