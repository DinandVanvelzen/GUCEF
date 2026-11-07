/*
 *  dbpluginVFSTABLES: GUCEF database backend over table files in the VFS
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

#ifndef DBPLUGIN_VFSTABLES_CVFSTABLESCANSOURCE_H
#define DBPLUGIN_VFSTABLES_CVFSTABLESCANSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTWEAKPTR_H
#include "gucefCORE_CTWeakPtr.h"
#define GUCEF_CORE_CTWEAKPTR_H
#endif /* GUCEF_CORE_CTWEAKPTR_H ? */

#ifndef GUCEF_CORE_CIDATATABLESCANSOURCE_H
#include "gucefCORE_CIDataTableScanSource.h"
#define GUCEF_CORE_CIDATATABLESCANSOURCE_H
#endif /* GUCEF_CORE_CIDATATABLESCANSOURCE_H ? */

#ifndef GUCEF_CORE_CDATATABLEDEFINITION_H
#include "gucefCORE_CDataTableDefinition.h"
#define GUCEF_CORE_CDATATABLEDEFINITION_H
#endif /* GUCEF_CORE_CDATATABLEDEFINITION_H ? */

#ifndef GUCEF_CORE_CDATATABLEQUERY_H
#include "gucefCORE_CDataTableQuery.h"
#define GUCEF_CORE_CDATATABLEQUERY_H
#endif /* GUCEF_CORE_CDATATABLEQUERY_H ? */

#ifndef DBPLUGIN_VFSTABLES_CVFSTABLESSTORE_H
#include "dbpluginVFSTABLES_CVfsTablesStore.h"
#define DBPLUGIN_VFSTABLES_CVFSTABLESSTORE_H
#endif /* DBPLUGIN_VFSTABLES_CVFSTABLESSTORE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DBPLUGIN {
namespace VFSTABLES {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The partition files of a vfstables table and readers on them (sections 14.4 and 14.6 of the data tables design).
 *
 *  Discovery walks the path template level by level. A level whose fields are all fixed by EQUALS or IN predicates on
 *  their source columns is not listed: its names are built from the values (and a file of the last level is probed for
 *  existence, a missing file is an empty partition). Other levels are listed once (through the listing cache), names
 *  that do not parse against the template (temporary files, foreign files) are ignored, and names whose key values
 *  cannot match the query are dropped there, with everything below them. Files come in path order of the key values,
 *  numbers and dates in numeric order. Discovery is lazy: memory stays bounded by the listings on the current path.
 *
 *  A file is opened once, on the first reader the scan asks for, and shared by all its readers: every reader reads
 *  through its own CSharedFileReadAccess, so CDataTableScanFile::sharedFileLock is not used. The metadata comes from
 *  the footer cache of the store when it holds the file as it is now. With onIncompatibleFile = skip a file whose schema
 *  the table cannot read is delivered as a file without row groups and counted in filesSkipped.
 *
 *  NextFile / NextPartition from one thread at a time; OpenFile from several at once.
 */
class DBPLUGIN_VFSTABLES_PUBLIC_CPP CVfsTableScanSource : public CORE::CIDataTableScanSource
{
    public:

    typedef CVfsTablesStore::TTableCodecPtr TTableCodecPtr;

    /** A partition file: its path relative to the database root and the key values of its path */
    struct DBPLUGIN_VFSTABLES_PUBLIC_CPP SPartitionFile
    {
        CORE::CString relativePath;
        CORE::CValueList keyValues;
    };
    typedef GUCEF::vector< SPartitionFile > TPartitionFileVector;

    /** At most this many names are built from the values of predicates for one level; more are listed instead */
    static const UInt32 MaxBuiltNamesPerLevel = 4096;

    /**
     *  The partitions of the table that may hold rows matching the resolved query (a query without predicates selects
     *  every partition). An unpartitioned table has one file: unpartitionedFile, relative to the root.
     */
    CVfsTableScanSource( const TVfsTablesStorePtr& store                    ,
                         const CORE::CDataTableDefinition& definition       ,
                         const CORE::CDataTableSchemaPtr& tableSchema       ,
                         const CORE::CString& unpartitionedFile             ,
                         const TTableCodecPtr& codec                        ,
                         const CORE::CValueList& codecOptions               ,
                         const CORE::CDataTableResolvedQuery& query         );

    /** Exactly the given files, without discovery: files that do not exist are left out */
    CVfsTableScanSource( const TVfsTablesStorePtr& store                    ,
                         const CORE::CDataTableDefinition& definition       ,
                         const CORE::CDataTableSchemaPtr& tableSchema       ,
                         const TTableCodecPtr& codec                        ,
                         const CORE::CValueList& codecOptions               ,
                         const TPartitionFileVector& files                  );

    virtual ~CVfsTableScanSource() GUCEF_VIRTUAL_OVERRIDE;

    /** The next partition file in path order, without opening it; END when there are no more */
    CORE::TDataTableStatus NextPartition( SPartitionFile& partition );

    virtual CORE::TDataTableStatus NextFile( CORE::CDataTableScanFile& file ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus OpenFile( const CORE::CDataTableScanFile& file, CORE::CITableReaderPtr& reader ) GUCEF_VIRTUAL_OVERRIDE;

    /** Adds the files and directories pruned on their key values, the footer cache hits and the skipped files */
    virtual void AddSourceStats( CORE::CDataTableScanStats& stats ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The message of the last failure of OpenFile */
    CORE::CString GetLastErrorMessage( void ) const;

    private:

    typedef CORE::CTWeakPtr< CDataTableSharedFile, MT::CMutex > TSharedFileWeakPtr;

    struct SCandidate
    {
        CORE::CString name;
        CORE::CString relativePath;
        CORE::CValueList keyValues;
        GUCEF::vector< Int64 > signedKeys;      /**< per field of the segment: numbers, dates and periods */
        GUCEF::vector< UInt64 > unsignedKeys;
        CORE::CString::StringVector textKeys;
    };
    typedef GUCEF::vector< SCandidate > TCandidateVector;

    struct SLevel
    {
        UInt32 segment;
        CORE::CString dirPath;                  /**< VFS path of the directory the names are in */
        CORE::CString relativeDir;              /**< the same, relative to the root */
        TCandidateVector candidates;
        UInt32 next;
    };
    typedef GUCEF::vector< SLevel > TLevelVector;

    typedef CORE::CTWeakPtr< const CORE::CITableFileMetaData, MT::CMutex > TMetaDataWeakPtr;

    struct SFileEntry
    {
        CORE::CString path;                     /**< VFS path */
        TSharedFileWeakPtr sharedFile;          /**< alive while a reader of the file is */
        TMetaDataWeakPtr plannedMetaData;       /**< the metadata of the first reader, alive while the scan plans with it */
        bool opened;
        UInt64 generation;
    };
    typedef GUCEF::vector< SFileEntry > TFileEntryVector;

    /** How the key of a field of a segment orders: as a signed number (integers, dates, periods), unsigned, or text */
    enum EKeyKind
    {
        KEYKIND_SIGNED = 0  ,
        KEYKIND_UNSIGNED    ,
        KEYKIND_TEXT
    };

    struct SSegmentField
    {
        CORE::CString name;
        Int32 sourceColumn;                     /**< table schema column, -1 when absent */
        CORE::TDataTablePartitionTransform transform;
        EKeyKind kind;
    };
    typedef GUCEF::vector< SSegmentField > TSegmentFieldVector;
    typedef GUCEF::vector< TSegmentFieldVector > TSegmentFieldsVector;

    struct SCandidateOrder;

    CVfsTableScanSource( const CVfsTableScanSource& src );              /**< not implemented */
    CVfsTableScanSource& operator=( const CVfsTableScanSource& src );   /**< not implemented */

    void Initialize( void );
    void ExpandLevel( UInt32 segment, const CORE::CString& relativeDir, const CORE::CValueList& parentKeys );
    bool BuildFixedNames( UInt32 segment, CORE::CString::StringSet& names ) const;
    bool MakeCandidate( UInt32 segment, const CORE::CString& name, const CORE::CString& relativeDir,
                        const CORE::CValueList& parentKeys, SCandidate& candidate ) const;
    bool CandidateLess( UInt32 segment, const SCandidate& left, const SCandidate& right ) const;
    CORE::TDataTableStatus Fail( CORE::TDataTableStatus status, const CORE::CString& message );

    TVfsTablesStorePtr m_store;
    CORE::CDataTablePartitionSpec m_spec;
    CORE::CDataTableSchemaPtr m_tableSchema;
    CORE::CString m_unpartitionedFile;
    TTableCodecPtr m_codec;
    CORE::CValueList m_codecOptions;
    CORE::CDataTableResolvedQuery m_query;
    bool m_hasQuery;
    TPartitionFileVector m_givenFiles;
    bool m_useGivenFiles;
    UInt32 m_nextGivenFile;
    bool m_started;
    TLevelVector m_levels;
    UInt32 m_segmentCount;
    TSegmentFieldsVector m_segmentFields;

    mutable MT::CMutex m_lock;                  /**< guards what OpenFile shares with NextFile */
    TFileEntryVector m_files;
    UInt64 m_pruned;
    UInt64 m_footerCacheHits;
    UInt64 m_skipped;
    CORE::CString m_lastError;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFSTABLES */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* DBPLUGIN_VFSTABLES_CVFSTABLESCANSOURCE_H ? */
