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

#ifndef GUCEF_CORE_CFILEDATATABLE_H
#define GUCEF_CORE_CFILEDATATABLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTBASICSHAREDPTR_H
#include "CTBasicSharedPtr.h"
#define GUCEF_CORE_CTBASICSHAREDPTR_H
#endif /* GUCEF_CORE_CTBASICSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CITABLECODEC_H
#include "gucefCORE_CITableCodec.h"
#define GUCEF_CORE_CITABLECODEC_H
#endif /* GUCEF_CORE_CITABLECODEC_H ? */

#ifndef GUCEF_CORE_CIDATATABLE_H
#include "gucefCORE_CIDataTable.h"
#define GUCEF_CORE_CIDATATABLE_H
#endif /* GUCEF_CORE_CIDATATABLE_H ? */

#ifndef GUCEF_CORE_CDATATABLEASYNCOPERATION_H
#include "gucefCORE_CDataTableAsyncOperation.h"
#define GUCEF_CORE_CDATATABLEASYNCOPERATION_H
#endif /* GUCEF_CORE_CDATATABLEASYNCOPERATION_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDataTableBufferPool;
class CFileDataTableFile;

/**
 *  One file read through a table codec as a table of one partition (section 4.5 of the data tables design).
 *
 *  Made on an IOAccess it is read only and the table schema is the schema of the file: CSV imports use this form.
 *  Made on a native file path with a definition it can also be written: every change reads the rows of the file,
 *  combines them as 7.2 describes (CDataTableMerge), writes the result through the codec to a temporary file next to
 *  the file and moves it over the file, so a reader sees the old rows or the new ones and never a partial file.
 *  A file that does not exist is an empty table; a change that leaves no rows deletes the file.
 *
 *  Scans read the file as it was when they started, through one open input shared by the readers of the scan under
 *  a lock (sharedFileLock of 14.6). The metadata is read on first use and kept until the table changes the file or
 *  Refresh() is called. Thread safe; the last error message is kept per calling thread.
 */
class GUCEF_CORE_PUBLIC_CPP CFileDataTable : public CIDataTable
{
    public:

    /** The codec handle: the type CTableCodecRegistry hands out */
    typedef CTBasicSharedPtr< CITableCodec, MT::CMutex > TTableCodecPtr;

    /** Key of the writer's key value metadata holding the layout a change wrote, compared by Compact */
    static const CString LayoutMetaDataKey;

    /**
     *  The value of LayoutMetaDataKey for files written under the definition: its row group and page sizes, compression,
     *  dictionary mode and schema version. Backends that rewrite partitions write and compare the same text
     */
    static CString MakeLayoutText( const CDataTableDefinition& definition );

    /** A read only table on the input; the name is what GetName() gives */
    CFileDataTable( const IOAccessPtr& source                           ,
                    const TTableCodecPtr& codec                         ,
                    const CValueList& codecOptions                      ,
                    const CString& tableName = CString::Empty           ,
                    CDataTableBufferPool* bufferPool = GUCEF_NULL       ,
                    PulseGeneratorPtr pulseGenerator = PulseGeneratorPtr() );

    /**
     *  A table that can be written, on the file at the native path. The definition gives the name, the schema (the
     *  file's schema must be readable as it, see CDataTableSchema::IsReadableAs) and the writer options.
     */
    CFileDataTable( const CString& filePath                             ,
                    const TTableCodecPtr& codec                         ,
                    const CValueList& codecOptions                      ,
                    const CDataTableDefinition& definition              ,
                    CDataTableBufferPool* bufferPool = GUCEF_NULL       ,
                    PulseGeneratorPtr pulseGenerator = PulseGeneratorPtr() );

    /** Waits for an async operation that is running; scans and batches handed out stay valid */
    virtual ~CFileDataTable() GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the metadata of the file now instead of on first use: OK (also for a missing file of a writable table) or the status of the codec */
    TDataTableStatus Open( void );

    /** The native path, empty for a table on an IOAccess */
    const CString& GetFilePath( void ) const;

    virtual const CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** For a table on an IOAccess the schema is the file's, known once the metadata is read */
    virtual const CDataTableDefinition& GetDefinition( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CDataTableSchemaPtr GetSchema( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsReadOnly( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TDataTableStatus Scan( const CDataTableQuery& query, CDataTableScanPtr& scan ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CFutureResult ScanAsync( const CDataTableQuery& query, CDataTableScanPtr& scan, ThreadPoolPtr threadPool = ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDataTableStatus Write( const CDataTableBatch& rows, TDataTableWriteMode mode, CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CFutureResult WriteAsync( const CDataTableBatchPtr& rows, TDataTableWriteMode mode, ThreadPoolPtr threadPool = ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDataTableStatus DeleteRows( const CDataTableQuery& predicates, CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CFutureResult DeleteRowsAsync( const CDataTableQuery& predicates, ThreadPoolPtr threadPool = ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    /** Deletes the file unless a predicate is on a column that is not a partition source column: INVALID_ARGUMENT */
    virtual TDataTableStatus DeletePartitions( const CDataTableQuery& partitionFilter, CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CFutureResult DeletePartitionsAsync( const CDataTableQuery& partitionFilter, ThreadPoolPtr threadPool = ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDataTableStatus ListPartitions( const CDataTableQuery& partitionFilter, bool withFileStats, CDataTablePartitionInfo::TPartitionInfoVector& partitions ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CFutureResult ListPartitionsAsync( const CDataTableQuery& partitionFilter, bool withFileStats, ThreadPoolPtr threadPool = ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    /** Rewrites the file in the canonical form unless its layout metadata says it is in it already; always with force */
    virtual TDataTableStatus Compact( const CDataTableQuery& partitionFilter, bool force, CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CFutureResult CompactAsync( const CDataTableQuery& partitionFilter, bool force, ThreadPoolPtr threadPool = ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    /** Forgets the metadata: the next call reads the file again */
    virtual void Refresh( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CTSharedPtr< CFileDataTableFile, MT::CMutex > TFilePtr;
    typedef GUCEF::map< UInt32, CString > TLastErrorMap;
    typedef GUCEF::vector< CDataTableBatchPtr > TBatchVector;

    CFileDataTable( const CFileDataTable& src );                    /**< not implemented */
    CFileDataTable& operator=( const CFileDataTable& src );         /**< not implemented */

    TDataTableStatus Fail( TDataTableStatus status, const CString& message ) const;
    TDataTableStatus Succeed( void ) const;
    TDataTableStatus GetFile( TFilePtr& file ) const;
    TDataTableStatus CheckWritable( void ) const;
    TDataTableStatus ReadAllRows( const TFilePtr& file, TBatchVector& batches, CString& error ) const;
    TDataTableStatus WriteRows( const TBatchVector& batches, UInt64 rowsReplaced, UInt64 rowsDeleted, UInt64 rowsBefore, CDataTableWriteResult& result );
    TDataTableStatus MakeScan( const TFilePtr& file, CDataTableScanPtr& scan ) const;
    CString GetLayoutText( void ) const;
    CFutureResult QueueOperation( CDataTableAsyncOperationData& data, ThreadPoolPtr threadPool );

    IOAccessPtr m_source;                   /**< the input of a read only table */
    CString m_filePath;                     /**< the file of a writable table */
    TTableCodecPtr m_codec;
    CValueList m_codecOptions;
    mutable CDataTableDefinition m_definition;
    mutable CDataTableSchemaPtr m_schema;
    CDataTableBufferPool* m_pool;
    bool m_writable;
    UInt32 m_tempCounter;
    MT::CMutex m_writeLock;                 /**< serializes the changes */
    mutable MT::CMutex m_fileLock;          /**< guards m_file and the lazily read schema */
    mutable TFilePtr m_file;                /**< the file as last read, NULL until then or after a change */
    CDataTableAsyncLinkPtr m_asyncLink;
    mutable MT::CMutex m_errorLock;
    mutable TLastErrorMap m_lastErrors;     /**< per thread */
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CFileDataTable, MT::CMutex >    CFileDataTablePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CFILEDATATABLE_H ? */
