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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef GUCEF_CORE_CDATATABLEMERGE_H
#include "gucefCORE_CDataTableMerge.h"
#define GUCEF_CORE_CDATATABLEMERGE_H
#endif /* GUCEF_CORE_CDATATABLEMERGE_H ? */

#ifndef GUCEF_CORE_CDATATABLEPRUNER_H
#include "gucefCORE_CDataTablePruner.h"
#define GUCEF_CORE_CDATATABLEPRUNER_H
#endif /* GUCEF_CORE_CDATATABLEPRUNER_H ? */

#ifndef GUCEF_CORE_CDATATABLESCAN_H
#include "gucefCORE_CDataTableScan.h"
#define GUCEF_CORE_CDATATABLESCAN_H
#endif /* GUCEF_CORE_CDATATABLESCAN_H ? */

#ifndef GUCEF_CORE_CITABLEWRITER_H
#include "gucefCORE_CITableWriter.h"
#define GUCEF_CORE_CITABLEWRITER_H
#endif /* GUCEF_CORE_CITABLEWRITER_H ? */

#include "gucefCORE_CFileDataTable.h"

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

const CString CFileDataTable::LayoutMetaDataKey = "gucef.table.layout";

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The file of a file table as one scan sees it: the open input, its metadata and the lock its readers share */
class CFileDataTableFile
{
    public:

    IOAccessPtr access;
    CITableFileMetaDataPtr metaData;        /**< NULL when the file does not exist */
    MT::CMutex readLock;
    CString path;
    UInt64 size;
    CDateTime modificationTime;

    CFileDataTableFile( void ) : access(), metaData(), readLock(), path(), size( GUCEF_UINT64MAX ), modificationTime() {}
};

/*-------------------------------------------------------------------------*/

namespace {

typedef CTSharedPtr< CFileDataTableFile, MT::CMutex > TFileDataTableFilePtr;

/** The one file of a file table, readers made by its codec on the shared input */
class CFileDataTableScanSource : public CIDataTableScanSource
{
    public:

    CFileDataTableScanSource( const TFileDataTableFilePtr& file, const CFileDataTable::TTableCodecPtr& codec, const CValueList& codecOptions )
        : CIDataTableScanSource()
        , m_file( file )
        , m_codec( codec )
        , m_codecOptions( codecOptions )
        , m_listed( false )
    {
    }

    virtual TDataTableStatus NextFile( CDataTableScanFile& file ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( m_listed || m_file->metaData.IsNULL() )
            return DATATABLE_STATUS_END;
        m_listed = true;
        file.Clear();
        file.path = m_file->path;
        file.metaData = m_file->metaData;
        file.sharedFileLock = &m_file->readLock;
        file.fileSize = m_file->size;
        file.modificationTime = m_file->modificationTime;
        return DATATABLE_STATUS_OK;
    }

    virtual TDataTableStatus OpenFile( const CDataTableScanFile& file, CITableReaderPtr& reader ) GUCEF_VIRTUAL_OVERRIDE
    {
        CString error;
        return m_codec->CreateReader( m_file->access, m_file->metaData, m_codecOptions, reader, error );
    }

    private:

    TFileDataTableFilePtr m_file;
    CFileDataTable::TTableCodecPtr m_codec;
    CValueList m_codecOptions;
    bool m_listed;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFileDataTable::CFileDataTable( const IOAccessPtr& source               ,
                                const TTableCodecPtr& codec             ,
                                const CValueList& codecOptions          ,
                                const CString& tableName                ,
                                CDataTableBufferPool* bufferPool        ,
                                PulseGeneratorPtr pulseGenerator        )
    : CIDataTable( pulseGenerator )
    , m_source( source )
    , m_filePath()
    , m_codec( codec )
    , m_codecOptions( codecOptions )
    , m_definition()
    , m_schema()
    , m_pool( GUCEF_NULL != bufferPool ? bufferPool : &CCoreGlobal::Instance()->GetDataTableBufferPool() )
    , m_writable( false )
    , m_tempCounter( 0 )
    , m_writeLock()
    , m_fileLock()
    , m_file()
    , m_asyncLink()
    , m_errorLock()
    , m_lastErrors()
{GUCEF_TRACE;

    m_asyncLink = CDataTableAsyncLinkPtr( GUCEF_NEW CDataTableAsyncLink( this ) );
    m_definition.name = tableName;
    if ( !m_codec.IsNULL() )
        m_definition.codecType = m_codec->GetTypeName();
}

/*-------------------------------------------------------------------------*/

CFileDataTable::CFileDataTable( const CString& filePath                 ,
                                const TTableCodecPtr& codec             ,
                                const CValueList& codecOptions          ,
                                const CDataTableDefinition& definition  ,
                                CDataTableBufferPool* bufferPool        ,
                                PulseGeneratorPtr pulseGenerator        )
    : CIDataTable( pulseGenerator )
    , m_source()
    , m_filePath( filePath )
    , m_codec( codec )
    , m_codecOptions( codecOptions )
    , m_definition( definition )
    , m_schema()
    , m_pool( GUCEF_NULL != bufferPool ? bufferPool : &CCoreGlobal::Instance()->GetDataTableBufferPool() )
    , m_writable( true )
    , m_tempCounter( 0 )
    , m_writeLock()
    , m_fileLock()
    , m_file()
    , m_asyncLink()
    , m_errorLock()
    , m_lastErrors()
{GUCEF_TRACE;

    m_asyncLink = CDataTableAsyncLinkPtr( GUCEF_NEW CDataTableAsyncLink( this ) );
    m_schema = m_definition.CreateSchemaPtr();
    if ( !m_codec.IsNULL() )
        m_definition.codecType = m_codec->GetTypeName();
}

/*-------------------------------------------------------------------------*/

CFileDataTable::~CFileDataTable()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    m_asyncLink->Detach();
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CFileDataTable::Fail( TDataTableStatus status, const CString& message ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_errorLock );
    m_lastErrors[ MT::GetCurrentTaskID() ] = message;
    return status;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CFileDataTable::Succeed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_errorLock );
    TLastErrorMap::iterator i = m_lastErrors.find( MT::GetCurrentTaskID() );
    if ( i != m_lastErrors.end() )
        m_lastErrors.erase( i );
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CString
CFileDataTable::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_errorLock );
    TLastErrorMap::const_iterator i = m_lastErrors.find( MT::GetCurrentTaskID() );
    return i != m_lastErrors.end() ? (*i).second : CString::Empty;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CFileDataTable::GetFile( TFilePtr& file ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_fileLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    if ( !m_file.IsNULL() )
    {
        file = m_file;
        return DATATABLE_STATUS_OK;
    }
    if ( m_codec.IsNULL() )
        return Fail( DATATABLE_STATUS_INVALID_ARGUMENT, "The table has no codec" );

    TFilePtr loaded( GUCEF_NEW CFileDataTableFile() );
    if ( m_writable )
    {
        loaded->path = m_filePath;
        if ( !FileExists( m_filePath ) )
        {
            // no file yet: an empty table
            m_file = loaded;
            file = loaded;
            return DATATABLE_STATUS_OK;
        }
        CFileAccess* access = GUCEF_NEW CFileAccess( m_filePath, "rb" );
        loaded->access = IOAccessPtr( access );
        if ( !access->IsValid() )
            return Fail( DATATABLE_STATUS_IO_ERROR, "File \"" + m_filePath + "\" cannot be opened for reading" );
        CResourceMetaData resourceMetaData;
        if ( GetFileMetaData( m_filePath, resourceMetaData ) && resourceMetaData.hasModifiedDateTime )
            loaded->modificationTime = resourceMetaData.modifiedDateTime;
        loaded->size = FileSize( m_filePath );
    }
    else
    {
        loaded->path = m_definition.name;
        loaded->access = m_source;
        if ( m_source.IsNULL() )
            return Fail( DATATABLE_STATUS_INVALID_ARGUMENT, "The table has no input" );
        loaded->size = m_source->GetSize();
    }

    CString error;
    CITableFileMetaDataPtr metaData;
    TDataTableStatus status = DATATABLE_STATUS_OK;
    {
        MT::CScopeMutex readLock( loaded->readLock );
        loaded->access->Setpos( 0 );
        status = m_codec->ReadMetaData( *loaded->access, m_codecOptions, metaData, error );
    }
    if ( DATATABLE_STATUS_OK != status )
        return Fail( status, "Reading the metadata of \"" + loaded->path + "\" failed: " + error );
    if ( metaData.IsNULL() || metaData->GetSchema().IsNULL() )
        return Fail( DATATABLE_STATUS_CORRUPT_DATA, "The codec gave no schema for \"" + loaded->path + "\"" );

    if ( m_writable )
    {
        CString mismatch;
        if ( !m_schema->IsReadableAs( *metaData->GetSchema(), mismatch ) )
            return Fail( DATATABLE_STATUS_SCHEMA_MISMATCH, "File \"" + loaded->path + "\" does not fit the table: " + mismatch );
    }
    else
    {
        // a read only table is what the file holds
        m_schema = metaData->GetSchema();
        m_definition.schema = *m_schema;
    }
    loaded->metaData = metaData;
    m_file = loaded;
    file = loaded;
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CFileDataTable::Open( void )
{GUCEF_TRACE;

    TFilePtr file;
    const TDataTableStatus status = GetFile( file );
    return DATATABLE_STATUS_OK == status ? Succeed() : status;
}

/*-------------------------------------------------------------------------*/

const CString&
CFileDataTable::GetFilePath( void ) const
{GUCEF_TRACE;

    return m_filePath;
}

/*-------------------------------------------------------------------------*/

const CString&
CFileDataTable::GetName( void ) const
{GUCEF_TRACE;

    return m_definition.name;
}

/*-------------------------------------------------------------------------*/

const CDataTableDefinition&
CFileDataTable::GetDefinition( void ) const
{GUCEF_TRACE;

    if ( !m_writable )
    {
        TFilePtr file;
        GetFile( file );
    }
    return m_definition;
}

/*-------------------------------------------------------------------------*/

CDataTableSchemaPtr
CFileDataTable::GetSchema( void ) const
{GUCEF_TRACE;

    if ( !m_writable )
    {
        TFilePtr file;
        GetFile( file );
    }
    MT::CScopeMutex lock( m_fileLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    return m_schema;
}

/*-------------------------------------------------------------------------*/

bool
CFileDataTable::IsReadOnly( void ) const
{GUCEF_TRACE;

    return !m_writable;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CFileDataTable::CheckWritable( void ) const
{GUCEF_TRACE;

    if ( !m_writable )
        return Fail( DATATABLE_STATUS_READ_ONLY, "A table on an input is read only" );
    if ( m_codec.IsNULL() )
        return Fail( DATATABLE_STATUS_INVALID_ARGUMENT, "The table has no codec" );
    if ( 0 == ( m_codec->GetCapabilities() & GUCEF_TABLECODEC_CAPABILITY_WRITE ) )
        return Fail( DATATABLE_STATUS_READ_ONLY, "Codec \"" + m_codec->GetTypeName() + "\" does not write files" );
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CFileDataTable::MakeScan( const TFilePtr& file, CDataTableScanPtr& scan ) const
{GUCEF_TRACE;

    CDataTableSchemaPtr schema;
    {
        MT::CScopeMutex lock( m_fileLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        schema = m_schema;
    }
    CDataTableScanSourcePtr source( GUCEF_NEW CFileDataTableScanSource( file, m_codec, m_codecOptions ) );
    scan = CDataTableScanPtr( GUCEF_NEW CDataTableScan( source, schema, *m_pool, GetPulseGenerator() ) );
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CFileDataTable::Scan( const CDataTableQuery& query, CDataTableScanPtr& scan )
{GUCEF_TRACE;

    scan.Unlink();
    TFilePtr file;
    TDataTableStatus status = GetFile( file );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    MakeScan( file, scan );
    status = scan->Start( query );
    if ( DATATABLE_STATUS_OK != status )
        return Fail( status, scan->GetLastErrorMessage() );
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CFutureResult
CFileDataTable::ScanAsync( const CDataTableQuery& query, CDataTableScanPtr& scan, ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    scan.Unlink();
    TFilePtr file;
    if ( DATATABLE_STATUS_OK != GetFile( file ) )
        return CFutureResult( TTaskStatus::TASKSTATUS_STARTUP_FAILED );
    MakeScan( file, scan );
    return scan->StartAsync( query, threadPool );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CFileDataTable::ReadAllRows( const TFilePtr& file, TBatchVector& batches, CString& error ) const
{GUCEF_TRACE;

    batches.clear();
    if ( file->metaData.IsNULL() )
        return DATATABLE_STATUS_OK;
    CDataTableScanPtr scan;
    MakeScan( file, scan );
    TDataTableStatus status = scan->Start( CDataTableQuery() );
    while ( DATATABLE_STATUS_OK == status )
    {
        CDataTableBatchPtr batch;
        status = scan->NextBatch( batch );
        if ( DATATABLE_STATUS_OK == status )
            batches.push_back( batch );
    }
    if ( DATATABLE_STATUS_END != status )
    {
        error = "Reading the rows of \"" + file->path + "\" failed: " + scan->GetLastErrorMessage();
        return status;
    }
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CString
CFileDataTable::MakeLayoutText( const CDataTableDefinition& definition )
{GUCEF_TRACE;

    const CDataTableWriterOptions& storage = definition.storage;
    return "rowGroupRowCount=" + ToString( storage.rowGroupRowCount ) + ";pageRowCount=" + ToString( storage.pageRowCount ) +
           ";compression=" + storage.compression + ";dictionary=" + DataTableDictionaryModeToString( storage.dictionary ) +
           ";schemaVersion=" + ToString( definition.schema.GetSchemaVersion() );
}

/*-------------------------------------------------------------------------*/

CString
CFileDataTable::GetLayoutText( void ) const
{GUCEF_TRACE;

    return MakeLayoutText( m_definition );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CFileDataTable::WriteRows( const TBatchVector& batches, UInt64 rowsReplaced, UInt64 rowsDeleted, UInt64 rowsBefore, CDataTableWriteResult& result )
{GUCEF_TRACE;

    UInt64 rowCount = 0;
    for ( UInt32 i=0; i<(UInt32) batches.size(); ++i )
        rowCount += batches[ i ]->GetRowCount();

    CDataTableWriteResult::SPartitionResult partition;
    partition.path = m_filePath;
    partition.rowsWritten = rowCount;
    partition.rowsReplaced = rowsReplaced;
    partition.rowsDeleted = rowsDeleted;

    CDataTablePartitionInfo info;
    info.path = m_filePath;
    info.hasFileStats = true;
    info.rowCount = rowCount;
    info.rowGroupCount = (UInt32) batches.size();

    if ( 0 == rowCount )
    {
        // a partition left empty is deleted
        if ( FileExists( m_filePath ) && !DeleteFile( m_filePath ) )
            return Fail( DATATABLE_STATUS_IO_ERROR, "File \"" + m_filePath + "\" could not be deleted" );
        {
            MT::CScopeMutex lock( m_fileLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
            m_file.Unlink();
        }
        partition.status = DATATABLE_STATUS_OK;
        result.partitions.push_back( partition );
        info.fileSize = 0;
        if ( rowsBefore > 0 )
            NotifyObserversFromThread( PartitionDeletedEvent, &info );
        return Succeed();
    }

    // the new file is written next to the old one and moved over it
    const CString tempPath = m_filePath + ".tmp-" + ToString( MT::GetProcessID() ) + "-" + ToString( ++m_tempCounter );
    CString error;
    TDataTableStatus status = DATATABLE_STATUS_OK;
    {
        CFileAccess* fileAccess = GUCEF_NEW CFileAccess( tempPath, "wb" );
        IOAccessPtr output( fileAccess );
        if ( !fileAccess->IsValid() )
            return Fail( DATATABLE_STATUS_IO_ERROR, "File \"" + tempPath + "\" cannot be created" );

        CDataTableWriterOptions options( m_definition.storage );
        options.keyValueMetaData.Set( LayoutMetaDataKey, GetLayoutText() );
        CITableWriterPtr writer;
        status = m_codec->CreateWriter( output, m_schema, options, writer, error );
        for ( UInt32 i=0; i<(UInt32) batches.size() && DATATABLE_STATUS_OK == status; ++i )
        {
            status = writer->WriteBatch( *batches[ i ] );
            if ( DATATABLE_STATUS_OK == status )
                status = writer->FlushRowGroup();
        }
        CDataTableFileSummary summary;
        if ( DATATABLE_STATUS_OK == status )
            status = writer->Finish( summary );
        if ( DATATABLE_STATUS_OK != status && !writer.IsNULL() )
        {
            error = writer->GetLastErrorMessage();
            writer->Abort();
        }
        writer.Unlink();
        output->Close();
    }
    if ( DATATABLE_STATUS_OK != status )
    {
        DeleteFile( tempPath );
        return Fail( status, "Writing \"" + m_filePath + "\" failed: " + error );
    }
    if ( !MoveFile( tempPath, m_filePath, true ) )
    {
        DeleteFile( tempPath );
        return Fail( DATATABLE_STATUS_IO_ERROR, "File \"" + tempPath + "\" could not replace \"" + m_filePath + "\"" );
    }
    {
        MT::CScopeMutex lock( m_fileLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        m_file.Unlink();
    }

    partition.status = DATATABLE_STATUS_OK;
    result.partitions.push_back( partition );
    info.fileSize = FileSize( m_filePath );
    NotifyObserversFromThread( PartitionWrittenEvent, &info );
    return Succeed();
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CFileDataTable::Write( const CDataTableBatch& rows, TDataTableWriteMode mode, CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    TDataTableStatus status = CheckWritable();
    if ( DATATABLE_STATUS_OK != status )
        return status;

    MT::CScopeMutex writeLock( m_writeLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    CString error;
    CDataTableBatchPtr normalized;
    status = CDataTableMerge::Normalize( m_schema, rows, *m_pool, normalized, error );
    if ( DATATABLE_STATUS_OK != status )
        return Fail( status, error );

    TFilePtr file;
    status = GetFile( file );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    TBatchVector existing;
    if ( DATATABLE_WRITE_REPLACE_PARTITIONS != mode )
    {
        status = ReadAllRows( file, existing, error );
        if ( DATATABLE_STATUS_OK != status )
            return Fail( status, error );
    }
    UInt64 rowsBefore = 0;
    for ( UInt32 i=0; i<(UInt32) existing.size(); ++i )
        rowsBefore += existing[ i ]->GetRowCount();
    if ( DATATABLE_WRITE_REPLACE_PARTITIONS == mode && !file->metaData.IsNULL() )
        rowsBefore = file->metaData->GetRowCount();

    TBatchVector incoming;
    incoming.push_back( normalized );
    TBatchVector merged;
    UInt64 rowsReplaced = 0;
    status = CDataTableMerge::Merge( m_schema, existing, incoming, mode, m_definition.storage.rowGroupRowCount, *m_pool, merged, rowsReplaced, error );
    if ( DATATABLE_STATUS_OK != status )
        return Fail( status, error );
    return WriteRows( merged, rowsReplaced, 0, rowsBefore, result );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CFileDataTable::DeleteRows( const CDataTableQuery& predicates, CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    TDataTableStatus status = CheckWritable();
    if ( DATATABLE_STATUS_OK != status )
        return status;

    MT::CScopeMutex writeLock( m_writeLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    CString error;
    CDataTableResolvedQuery resolved;
    resolved.schema = m_schema;
    if ( !predicates.Resolve( *m_schema, resolved, error ) )
        return Fail( resolved.status, error );

    TFilePtr file;
    status = GetFile( file );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    TBatchVector existing;
    status = ReadAllRows( file, existing, error );
    if ( DATATABLE_STATUS_OK != status )
        return Fail( status, error );
    UInt64 rowsBefore = 0;
    for ( UInt32 i=0; i<(UInt32) existing.size(); ++i )
        rowsBefore += existing[ i ]->GetRowCount();

    TBatchVector kept;
    UInt64 removed = 0;
    status = CDataTableMerge::RemoveMatchingRows( resolved, existing, *m_pool, kept, removed, error );
    if ( DATATABLE_STATUS_OK != status )
        return Fail( status, error );
    if ( 0 == removed )
    {
        CDataTableWriteResult::SPartitionResult partition;
        partition.path = m_filePath;
        partition.status = DATATABLE_STATUS_OK;
        partition.rowsWritten = rowsBefore;
        result.partitions.push_back( partition );
        return Succeed();
    }
    return WriteRows( kept, 0, removed, rowsBefore, result );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CFileDataTable::DeletePartitions( const CDataTableQuery& partitionFilter, CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    TDataTableStatus status = CheckWritable();
    if ( DATATABLE_STATUS_OK != status )
        return status;

    const CDataTablePartitionSpec::TFieldVector& fields = m_definition.partitioning.GetFields();
    const CDataTableQuery::TPredicateVector& predicates = partitionFilter.GetPredicates();
    for ( UInt32 p=0; p<(UInt32) predicates.size(); ++p )
    {
        bool isSource = false;
        for ( UInt32 f=0; f<(UInt32) fields.size() && !isSource; ++f )
            isSource = fields[ f ].sourceColumn == predicates[ p ].columnName;
        if ( !isSource )
            return Fail( DATATABLE_STATUS_INVALID_ARGUMENT, "Column \"" + predicates[ p ].columnName + "\" is not a partition source column" );
    }

    MT::CScopeMutex writeLock( m_writeLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    CString error;
    CDataTableResolvedQuery resolved;
    resolved.schema = m_schema;
    if ( !partitionFilter.Resolve( *m_schema, resolved, error ) )
        return Fail( resolved.status, error );

    TFilePtr file;
    status = GetFile( file );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    if ( file->metaData.IsNULL() || resolved.matchesNothing || !m_definition.partitioning.MayMatch( CValueList(), resolved ) )
        return Succeed();
    const UInt64 rowsBefore = file->metaData->GetRowCount();
    const TBatchVector none;
    return WriteRows( none, 0, GUCEF_UINT64MAX == rowsBefore ? 0 : rowsBefore, 1, result );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CFileDataTable::ListPartitions( const CDataTableQuery& partitionFilter, bool withFileStats, CDataTablePartitionInfo::TPartitionInfoVector& partitions )
{GUCEF_TRACE;

    partitions.clear();
    TFilePtr file;
    TDataTableStatus status = GetFile( file );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    if ( file->metaData.IsNULL() )
        return Succeed();

    CDataTableSchemaPtr schema = GetSchema();
    CString error;
    CDataTableResolvedQuery resolved;
    resolved.schema = schema;
    if ( !partitionFilter.Resolve( *schema, resolved, error ) )
        return Fail( resolved.status, error );

    // the file may hold a match when one of its row groups may
    const CITableFileMetaData& metaData = *file->metaData;
    const CDataTableSchema& fileSchema = *metaData.GetSchema();
    CDataTableSchema::TColumnIndexVector fileColumnOfTableColumn( schema->GetColumnCount(), GUCEF_UINT32MAX );
    for ( UInt32 t=0; t<schema->GetColumnCount(); ++t )
    {
        const Int32 fileColumn = fileSchema.FindColumn( schema->GetColumn( t ).name );
        if ( fileColumn >= 0 )
            fileColumnOfTableColumn[ t ] = (UInt32) fileColumn;
    }
    bool mayMatch = resolved.predicates.empty();
    CDataTableRowGroupInfo info;
    for ( UInt32 rg=0; rg<metaData.GetRowGroupCount() && !mayMatch; ++rg )
        mayMatch = !metaData.GetRowGroupInfo( rg, info ) || CDataTablePruner::RowGroupMayMatch( resolved, info, fileColumnOfTableColumn, &fileSchema );
    if ( !mayMatch )
        return Succeed();

    CDataTablePartitionInfo partition;
    partition.path = file->path;
    partition.hasFileStats = withFileStats;
    if ( withFileStats )
    {
        partition.fileSize = file->size;
        partition.modificationTime = file->modificationTime;
        partition.rowCount = metaData.GetRowCount();
        partition.rowGroupCount = metaData.GetRowGroupCount();
    }
    partitions.push_back( partition );
    return Succeed();
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CFileDataTable::Compact( const CDataTableQuery& partitionFilter, bool force, CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    TDataTableStatus status = CheckWritable();
    if ( DATATABLE_STATUS_OK != status )
        return status;

    MT::CScopeMutex writeLock( m_writeLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    CString error;
    CDataTableResolvedQuery resolved;
    resolved.schema = m_schema;
    if ( !partitionFilter.Resolve( *m_schema, resolved, error ) )
        return Fail( resolved.status, error );

    TFilePtr file;
    status = GetFile( file );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    if ( file->metaData.IsNULL() || resolved.matchesNothing )
        return Succeed();
    if ( !force )
    {
        CValueList keyValues;
        file->metaData->GetKeyValueMetaData( keyValues );
        if ( keyValues.GetValueAlways( LayoutMetaDataKey ).AsString() == GetLayoutText() )
            return Succeed();
    }

    TBatchVector existing;
    status = ReadAllRows( file, existing, error );
    if ( DATATABLE_STATUS_OK != status )
        return Fail( status, error );
    UInt64 rowsBefore = 0;
    for ( UInt32 i=0; i<(UInt32) existing.size(); ++i )
        rowsBefore += existing[ i ]->GetRowCount();
    TBatchVector canonical;
    UInt64 removed = 0;
    status = CDataTableMerge::Canonicalize( m_schema, existing, m_definition.storage.rowGroupRowCount, *m_pool, canonical, removed, error );
    if ( DATATABLE_STATUS_OK != status )
        return Fail( status, error );
    return WriteRows( canonical, removed, 0, rowsBefore, result );
}

/*-------------------------------------------------------------------------*/

void
CFileDataTable::Refresh( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_fileLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    m_file.Unlink();
}

/*-------------------------------------------------------------------------*/

CFutureResult
CFileDataTable::QueueOperation( CDataTableAsyncOperationData& data, ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    data.table = m_asyncLink;
    return CDataTableAsyncOperation::Queue( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CFileDataTable::WriteAsync( const CDataTableBatchPtr& rows, TDataTableWriteMode mode, ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CDataTableAsyncOperationData data;
    data.operation = CDataTableAsyncOperationData::OPERATION_WRITE;
    data.rows = rows;
    data.writeMode = mode;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CFileDataTable::DeleteRowsAsync( const CDataTableQuery& predicates, ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CDataTableAsyncOperationData data;
    data.operation = CDataTableAsyncOperationData::OPERATION_DELETE_ROWS;
    data.query = predicates;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CFileDataTable::DeletePartitionsAsync( const CDataTableQuery& partitionFilter, ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CDataTableAsyncOperationData data;
    data.operation = CDataTableAsyncOperationData::OPERATION_DELETE_PARTITIONS;
    data.query = partitionFilter;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CFileDataTable::ListPartitionsAsync( const CDataTableQuery& partitionFilter, bool withFileStats, ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CDataTableAsyncOperationData data;
    data.operation = CDataTableAsyncOperationData::OPERATION_LIST_PARTITIONS;
    data.query = partitionFilter;
    data.flag = withFileStats;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CFileDataTable::CompactAsync( const CDataTableQuery& partitionFilter, bool force, ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CDataTableAsyncOperationData data;
    data.operation = CDataTableAsyncOperationData::OPERATION_COMPACT;
    data.query = partitionFilter;
    data.flag = force;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

const CString&
CFileDataTable::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CFileDataTable";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
