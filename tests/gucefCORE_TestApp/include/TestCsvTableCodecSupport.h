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

#ifndef GUCEF_CORE_TESTAPP_TESTCSVTABLECODECSUPPORT_H
#define GUCEF_CORE_TESTAPP_TESTCSVTABLECODECSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTABLECODECREGISTRY_H
#include "gucefCORE_CTableCodecRegistry.h"
#define GUCEF_CORE_CTABLECODECREGISTRY_H
#endif /* GUCEF_CORE_CTABLECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CCSVTABLECODEC_H
#include "gucefCORE_CCsvTableCodec.h"
#define GUCEF_CORE_CCSVTABLECODEC_H
#endif /* GUCEF_CORE_CCSVTABLECODEC_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDATATABLESCANSTATS_H
#include "gucefCORE_CDataTableScanStats.h"
#define GUCEF_CORE_CDATATABLESCANSTATS_H
#endif /* GUCEF_CORE_CDATATABLESCANSTATS_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace CORETEST {

typedef GUCEF::vector< GUCEF::CORE::CVariant > TCsvTestRow;
typedef GUCEF::vector< TCsvTestRow > TCsvTestRows;

/** What reading a CSV file through the codec gave */
struct SCsvTestRead
{
    GUCEF::CORE::TDataTableStatus status;       /**< the first status that was not OK */
    GUCEF::CORE::CString error;
    GUCEF::CORE::CDataTableSchemaPtr schema;
    TCsvTestRows rows;                          /**< copies that own their memory */
    GUCEF::vector< GUCEF::UInt32 > batchRowCounts;
    GUCEF::vector< GUCEF::UInt64 > batchFirstRows;       /**< the origin of every batch */
    GUCEF::UInt64 peakPoolBytesInUse;                    /**< most bytes of the pool in use while a batch was held */
    GUCEF::CORE::CDataTableScanStats stats;
    GUCEF::CORE::CValueList keyValues;          /**< the key value metadata of the file */

    SCsvTestRead( void ) : status( GUCEF::CORE::DATATABLE_STATUS_OK ), error(), schema(), rows(), batchRowCounts(), batchFirstRows(), peakPoolBytesInUse( 0 ), stats(), keyValues() {}
};

/** A readable and writable source over the bytes; the bytes are copied */
inline GUCEF::CORE::IOAccessPtr
MakeCsvSource( const char* bytes, GUCEF::UInt32 length )
{
    using namespace GUCEF::CORE;
    CDynamicBuffer* buffer = GUCEF_NEW CDynamicBuffer( length > 0 ? length : 1, true );
    if ( length > 0 )
        buffer->CopyFrom( length, bytes );
    buffer->SetDataSize( length );
    CDynamicBufferAccess* access = GUCEF_NEW CDynamicBufferAccess( buffer, true );
    IOAccessPtr result = access->CreateSharedPtr();
    return result;
}

inline GUCEF::CORE::IOAccessPtr
MakeCsvSource( const char* text )
{
    return MakeCsvSource( text, (GUCEF::UInt32) strlen( text ) );
}

inline GUCEF::CORE::IOAccessPtr
MakeCsvSource( const GUCEF::vector< char >& bytes )
{
    return MakeCsvSource( bytes.empty() ? "" : &bytes[ 0 ], (GUCEF::UInt32) bytes.size() );
}

/** A file in memory that can be written and read; the bytes can be looked at */
class CCsvMemoryFile
{
    public:

    CCsvMemoryFile( void )
        : m_buffer( 1, true )
        , m_access()
    {
        using namespace GUCEF::CORE;
        m_buffer.SetDataSize( 0 );
        CDynamicBufferAccess* access = GUCEF_NEW CDynamicBufferAccess( &m_buffer, false );
        m_access = access->CreateSharedPtr();
    }

    CCsvMemoryFile( const char* bytes, GUCEF::UInt32 length )
        : m_buffer( length > 0 ? length : 1, true )
        , m_access()
    {
        using namespace GUCEF::CORE;
        m_buffer.SetDataSize( 0 );
        if ( length > 0 )
            m_buffer.CopyFrom( length, bytes );
        m_buffer.SetDataSize( length );
        CDynamicBufferAccess* access = GUCEF_NEW CDynamicBufferAccess( &m_buffer, false );
        m_access = access->CreateSharedPtr();
    }

    const GUCEF::CORE::IOAccessPtr& Access( void ) const { return m_access; }

    GUCEF::UInt32 Size( void ) const { return m_buffer.GetDataSize(); }

    const char* Data( void ) const { return static_cast< const char* >( m_buffer.GetConstBufferPtr() ); }

    /** The bytes as text */
    GUCEF::CORE::CString Text( void ) const
    {
        GUCEF::CORE::CString text;
        if ( m_buffer.GetDataSize() > 0 )
            text.Append( Data(), m_buffer.GetDataSize() );
        return text;
    }

    GUCEF::vector< char > Bytes( void ) const
    {
        return GUCEF::vector< char >( Data(), Data() + m_buffer.GetDataSize() );
    }

    private:

    CCsvMemoryFile( const CCsvMemoryFile& src );
    CCsvMemoryFile& operator=( const CCsvMemoryFile& src );

    GUCEF::CORE::CDynamicBuffer m_buffer;
    GUCEF::CORE::IOAccessPtr m_access;
};

/** The access behind a pointer to it */
inline GUCEF::CORE::CIOAccess&
CsvIo( const GUCEF::CORE::IOAccessPtr& access )
{
    return *const_cast< GUCEF::CORE::CIOAccess* >( access.GetPointerAlways() );
}

inline bool
GetCsvCodec( GUCEF::CORE::CTableCodecRegistry::TTableCodecPtr& codec )
{
    return GUCEF::CORE::CCoreGlobal::Instance()->GetTableCodecRegistry().TryGetCsvCodec( codec );
}

/** A value list that takes lists of values */
inline GUCEF::CORE::CValueList
CsvOptions( void )
{
    GUCEF::CORE::CValueList options;
    options.SetAllowMultipleValues( true );
    return options;
}

inline void
SetCsvOption( GUCEF::CORE::CValueList& options, const char* key, const char* value )
{
    options.SetAllowMultipleValues( true );
    options.Set( key, GUCEF::CORE::CVariant( GUCEF::CORE::CString( value ) ) );
}

/**
 *  Reads the source through the codec as a table does: metadata first (unless viaMetaData is false, then the reader
 *  works the schema out itself), a reader on the source, row group 0 opened with the given batch size and projection,
 *  and the batches read to the end.
 */
inline GUCEF::CORE::TDataTableStatus
ReadCsvSource( const GUCEF::CORE::IOAccessPtr& source, const GUCEF::CORE::CValueList& options, SCsvTestRead& result,
               GUCEF::UInt32 batchRowCount = 16384, bool viaMetaData = true, GUCEF::CORE::CDataTableBufferPool* pool = GUCEF_NULL,
               const GUCEF::CORE::CDataTableSchema::TColumnIndexVector* projection = GUCEF_NULL, GUCEF::MT::CMutex* sharedFileLock = GUCEF_NULL,
               GUCEF::UInt32 maxRows = GUCEF_UINT32MAX )
{
    using namespace GUCEF::CORE;
    result = SCsvTestRead();

    CTableCodecRegistry::TTableCodecPtr codec;
    if ( !GetCsvCodec( codec ) )
    {
        result.status = DATATABLE_STATUS_NOT_FOUND;
        result.error = "no csv codec";
        return result.status;
    }

    CITableFileMetaDataPtr metaData;
    if ( viaMetaData )
    {
        result.status = codec->ReadMetaData( CsvIo( source ), options, metaData, result.error );
        if ( DATATABLE_STATUS_OK != result.status )
            return result.status;
    }

    CITableReaderPtr reader;
    result.status = codec->CreateReader( source, metaData, options, reader, result.error );
    if ( DATATABLE_STATUS_OK != result.status )
        return result.status;

    CDataTableReadRequest request;
    request.batchRowCount = batchRowCount;
    request.bufferPool = pool;
    request.stats = &result.stats;
    request.sharedFileLock = sharedFileLock;
    if ( GUCEF_NULL != projection )
        request.projection = *projection;
    result.status = reader->OpenRowGroup( 0, request );
    if ( DATATABLE_STATUS_OK != result.status )
    {
        result.error = reader->GetLastErrorMessage();
        return result.status;
    }

    result.schema = reader->GetMetaData()->GetSchema();
    reader->GetMetaData()->GetKeyValueMetaData( result.keyValues );

    for ( ;; )
    {
        CDataTableBatchPtr batch;
        TDataTableStatus status = reader->ReadNextBatch( batch );
        if ( DATATABLE_STATUS_END == status )
            break;
        if ( DATATABLE_STATUS_OK != status )
        {
            result.status = status;
            result.error = reader->GetLastErrorMessage();
            return result.status;
        }

        result.batchRowCounts.push_back( batch->GetRowCount() );
        result.batchFirstRows.push_back( batch->GetOrigin().firstRowInRowGroup );
        {
            CDataTableBufferPoolStats poolStats;
            ( GUCEF_NULL != pool ? *pool : CCoreGlobal::Instance()->GetDataTableBufferPool() ).GetStats( poolStats );
            if ( poolStats.bytesInUse > result.peakPoolBytesInUse )
                result.peakPoolBytesInUse = poolStats.bytesInUse;
        }
        for ( UInt32 row = 0; row < batch->GetRowCount(); ++row )
        {
            TCsvTestRow cells;
            for ( UInt32 column = 0; column < batch->GetColumnCount(); ++column )
            {
                CVariant value;
                batch->GetValue( row, column, value, false );
                cells.push_back( value );
            }
            result.rows.push_back( cells );
        }
        if ( result.rows.size() >= maxRows )
            break;
    }
    return DATATABLE_STATUS_OK;
}

/** Reads the text */
inline GUCEF::CORE::TDataTableStatus
ReadCsvText( const char* text, const GUCEF::CORE::CValueList& options, SCsvTestRead& result, GUCEF::UInt32 batchRowCount = 16384, bool viaMetaData = true )
{
    return ReadCsvSource( MakeCsvSource( text ), options, result, batchRowCount, viaMetaData );
}

inline GUCEF::CORE::TDataTableStatus
ReadCsvText( const char* text, SCsvTestRead& result )
{
    return ReadCsvSource( MakeCsvSource( text ), CsvOptions(), result );
}

/** The element of a vector, 0 for an index that is not there */
template< typename T >
inline T
CsvAt( const GUCEF::vector< T >& items, size_t index )
{
    return index < items.size() ? items[ index ] : T();
}

/** The cell of the read, a null cell for a row or column that is not there, so that a test whose read failed fails instead of crashing */
inline const GUCEF::CORE::CVariant&
CsvCell( const SCsvTestRead& read, size_t row, size_t column )
{
    static const GUCEF::CORE::CVariant nothing;
    return row < read.rows.size() && column < read.rows[ row ].size() ? read.rows[ row ][ column ] : nothing;
}

/** The column of the schema of the read, a default column when there is none */
inline const GUCEF::CORE::CDataTableColumnDef&
CsvColumnOf( const SCsvTestRead& read, GUCEF::UInt32 index )
{
    static const GUCEF::CORE::CDataTableColumnDef nothing;
    return !read.schema.IsNULL() && index < read.schema->GetColumnCount() ? read.schema->GetColumn( index ) : nothing;
}

inline GUCEF::UInt32
CsvColumnCountOf( const SCsvTestRead& read )
{
    return read.schema.IsNULL() ? 0 : read.schema->GetColumnCount();
}

/** The type texts of the columns, "int64,decimal(18,2),utf8" */
inline GUCEF::CORE::CString
CsvTypeTexts( const SCsvTestRead& read )
{
    GUCEF::CORE::CString text;
    if ( read.schema.IsNULL() )
        return text;
    for ( GUCEF::UInt32 i = 0; i < read.schema->GetColumnCount(); ++i )
    {
        if ( i > 0 )
            text += ',';
        text += read.schema->GetColumn( i ).GetTypeText();
    }
    return text;
}

/** The names of the columns, "a,b,c" */
inline GUCEF::CORE::CString
CsvColumnNames( const SCsvTestRead& read )
{
    GUCEF::CORE::CString text;
    if ( read.schema.IsNULL() )
        return text;
    for ( GUCEF::UInt32 i = 0; i < read.schema->GetColumnCount(); ++i )
    {
        if ( i > 0 )
            text += ',';
        text += read.schema->GetColumn( i ).name;
    }
    return text;
}

/** The cell as text: "null" for null, the string form otherwise */
inline GUCEF::CORE::CString
CsvCellText( const GUCEF::CORE::CVariant& value )
{
    return value.IsNULL() ? GUCEF::CORE::CString( "null" ) : value.AsString();
}

/** Whether two cells hold the same value; floats are compared by their bits so that -0, NaN and the infinities count */
inline bool
CsvSameCell( const GUCEF::CORE::CVariant& a, const GUCEF::CORE::CVariant& b )
{
    using namespace GUCEF;
    if ( a.IsNULL() || b.IsNULL() )
        return a.IsNULL() && b.IsNULL();
    if ( a.GetTypeId() != b.GetTypeId() )
        return false;
    if ( GUCEF_DATATYPE_FLOAT64 == a.GetTypeId() )
    {
        const Float64 x = a.AsFloat64(), y = b.AsFloat64();
        return 0 == memcmp( &x, &y, sizeof( x ) ) || ( x != x && y != y );
    }
    if ( GUCEF_DATATYPE_FLOAT32 == a.GetTypeId() )
    {
        const Float32 x = a.AsFloat32(), y = b.AsFloat32();
        return 0 == memcmp( &x, &y, sizeof( x ) ) || ( x != x && y != y );
    }
    if ( GUCEF_DATATYPE_BINARY_BLOB == a.GetTypeId() || GUCEF_DATATYPE_BINARY_BSOB == a.GetTypeId() )
        return a.ByteSize() == b.ByteSize() && ( 0 == a.ByteSize() || 0 == memcmp( a.AsVoidPtr(), b.AsVoidPtr(), a.ByteSize() ) );
    return a.AsString() == b.AsString();
}

/** Builds a batch of the rows; false when a row does not fit the schema */
inline bool
BuildCsvTestBatch( const GUCEF::CORE::CDataTableSchemaPtr& schema, const TCsvTestRows& rows, GUCEF::CORE::CDataTableBatchPtr& batch )
{
    using namespace GUCEF::CORE;
    CDataTableBatchBuilder builder( schema, CCoreGlobal::Instance()->GetDataTableBufferPool() );
    for ( size_t i = 0; i < rows.size(); ++i )
        if ( !builder.AppendRow( &rows[ i ][ 0 ], (GUCEF::UInt32) rows[ i ].size() ) )
            return false;
    CString error;
    return DATATABLE_STATUS_OK == builder.Build( batch, error );
}

/** Writes the batch through the codec with the options and returns the bytes */
inline GUCEF::CORE::TDataTableStatus
WriteCsvBatch( const GUCEF::CORE::CDataTableSchemaPtr& schema, const GUCEF::CORE::CDataTableBatch& batch, const GUCEF::CORE::CDataTableWriterOptions& options,
               CCsvMemoryFile& destination, GUCEF::CORE::CString& error, GUCEF::CORE::CDataTableFileSummary* summary = GUCEF_NULL )
{
    using namespace GUCEF::CORE;
    CTableCodecRegistry::TTableCodecPtr codec;
    if ( !GetCsvCodec( codec ) )
        return DATATABLE_STATUS_NOT_FOUND;

    CITableWriterPtr writer;
    TDataTableStatus status = codec->CreateWriter( destination.Access(), schema, options, writer, error );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    status = writer->WriteBatch( batch );
    if ( DATATABLE_STATUS_OK != status )
    {
        error = writer->GetLastErrorMessage();
        return status;
    }
    CDataTableFileSummary localSummary;
    status = writer->Finish( GUCEF_NULL != summary ? *summary : localSummary );
    if ( DATATABLE_STATUS_OK != status )
        error = writer->GetLastErrorMessage();
    return status;
}

inline GUCEF::CORE::CDataTableWriterOptions
CsvWriterOptions( void )
{
    GUCEF::CORE::CDataTableWriterOptions options;
    options.codecOptions.SetAllowMultipleValues( true );
    return options;
}

inline void
SetCsvWriterOption( GUCEF::CORE::CDataTableWriterOptions& options, const char* key, const char* value )
{
    options.codecOptions.Set( key, GUCEF::CORE::CVariant( GUCEF::CORE::CString( value ) ) );
}

/** A small deterministic generator for the property tests */
class CCsvTestRandom
{
    public:

    explicit CCsvTestRandom( GUCEF::UInt64 seed ) : m_state( seed * 6364136223846793005ULL + 1442695040888963407ULL ) {}

    GUCEF::UInt64 Next( void )
    {
        m_state ^= m_state << 13;
        m_state ^= m_state >> 7;
        m_state ^= m_state << 17;
        return m_state;
    }

    /** 0 .. n - 1 */
    GUCEF::UInt32 Below( GUCEF::UInt32 n ) { return n > 0 ? (GUCEF::UInt32) ( ( Next() >> 11 ) % n ) : 0; }

    bool Coin( GUCEF::UInt32 oneIn ) { return 0 == Below( oneIn ); }

    private:

    GUCEF::UInt64 m_state;
};

}; /* namespace CORETEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTCSVTABLECODECSUPPORT_H ? */
