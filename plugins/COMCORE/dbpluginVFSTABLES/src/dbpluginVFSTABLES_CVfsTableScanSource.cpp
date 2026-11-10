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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <algorithm>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#include "dbpluginVFSTABLES_CVfsTableScanSource.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The metadata of a file the scan skips: the table schema and no row groups */
class CSkippedFileMetaData : public CORE::CITableFileMetaData
{
    public:

    explicit CSkippedFileMetaData( const CORE::CDataTableSchemaPtr& schema ) : CORE::CITableFileMetaData(), m_schema( schema ) {}
    virtual const CORE::CDataTableSchemaPtr& GetSchema( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_schema; }
    virtual UInt64 GetRowCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return 0; }
    virtual UInt32 GetRowGroupCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return 0; }
    virtual bool GetRowGroupInfo( UInt32 rowGroup, CORE::CDataTableRowGroupInfo& info ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual UInt64 GetMemoryFootprint( void ) const GUCEF_VIRTUAL_OVERRIDE { return sizeof( *this ); }
    virtual void GetKeyValueMetaData( CORE::CValueList& keyValues ) const GUCEF_VIRTUAL_OVERRIDE { keyValues.Clear(); }

    private:

    CORE::CDataTableSchemaPtr m_schema;
};

/*-------------------------------------------------------------------------*/

/** The reader of a skipped file: there is nothing to read */
class CSkippedFileReader : public CORE::CITableReader
{
    public:

    explicit CSkippedFileReader( const CORE::CDataTableSchemaPtr& schema )
        : CORE::CITableReader()
        , m_metaData()
    {
        CORE::CTSharedPtr< CSkippedFileMetaData, MT::CMutex > metaData( GUCEF_NEW CSkippedFileMetaData( schema ) );
        m_metaData = metaData;
    }

    virtual const CORE::CITableFileMetaDataPtr& GetMetaData( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_metaData; }
    virtual CORE::TDataTableStatus OpenRowGroup( UInt32 rowGroup, const CORE::CDataTableReadRequest& request ) GUCEF_VIRTUAL_OVERRIDE { return CORE::DATATABLE_STATUS_INVALID_ARGUMENT; }
    virtual CORE::TDataTableStatus ReadNextBatch( CORE::CDataTableBatchPtr& batch ) GUCEF_VIRTUAL_OVERRIDE { return CORE::DATATABLE_STATUS_END; }
    virtual CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE { return "The file is skipped: the table cannot read its schema"; }

    private:

    CORE::CITableFileMetaDataPtr m_metaData;
};

/*-------------------------------------------------------------------------*/

/** A resolved literal (5.2) as a key value BuildSegment takes: dates and timestamps as their variants */
CORE::CVariant
NativeToKeyValue( const CORE::CDataTableColumnDef& column, const CORE::CVariant& native )
{
    if ( CORE::DATATABLE_TYPE_DATE == column.type )
        return CORE::CDataTableValues::Date( (Int32) native.AsInt64() );
    if ( CORE::DATATABLE_TYPE_TIMESTAMP == column.type )
        return CORE::CDataTableValues::Timestamp( native.AsInt64(), column.timeUnit );
    return native;
}

}

/*-------------------------------------------------------------------------*/

/** Orders the candidates of a level by their key values, through their indices */
struct CVfsTableScanSource::SCandidateOrder
{
    const CVfsTableScanSource* source;
    const TCandidateVector* candidates;
    UInt32 segment;

    bool operator()( UInt32 left, UInt32 right ) const
    {
        return source->CandidateLess( segment, (*candidates)[ left ], (*candidates)[ right ] );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsTableScanSource::CVfsTableScanSource( const TVfsTablesStorePtr& store                    ,
                                          const CORE::CDataTableDefinition& definition       ,
                                          const CORE::CDataTableSchemaPtr& tableSchema       ,
                                          const CORE::CString& unpartitionedFile             ,
                                          const TTableCodecPtr& codec                        ,
                                          const CORE::CValueList& codecOptions               ,
                                          const CORE::CDataTableResolvedQuery& query         )
    : CORE::CIDataTableScanSource()
    , m_store( store )
    , m_spec( definition.partitioning )
    , m_tableSchema( tableSchema )
    , m_unpartitionedFile( unpartitionedFile )
    , m_codec( codec )
    , m_codecOptions( codecOptions )
    , m_query( query )
    , m_hasQuery( true )
    , m_givenFiles()
    , m_useGivenFiles( false )
    , m_nextGivenFile( 0 )
    , m_started( false )
    , m_levels()
    , m_segmentCount( 0 )
    , m_segmentFields()
    , m_lock()
    , m_files()
    , m_pruned( 0 )
    , m_footerCacheHits( 0 )
    , m_skipped( 0 )
    , m_lastError()
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

CVfsTableScanSource::CVfsTableScanSource( const TVfsTablesStorePtr& store                    ,
                                          const CORE::CDataTableDefinition& definition       ,
                                          const CORE::CDataTableSchemaPtr& tableSchema       ,
                                          const TTableCodecPtr& codec                        ,
                                          const CORE::CValueList& codecOptions               ,
                                          const TPartitionFileVector& files                  )
    : CORE::CIDataTableScanSource()
    , m_store( store )
    , m_spec( definition.partitioning )
    , m_tableSchema( tableSchema )
    , m_unpartitionedFile()
    , m_codec( codec )
    , m_codecOptions( codecOptions )
    , m_query()
    , m_hasQuery( false )
    , m_givenFiles( files )
    , m_useGivenFiles( true )
    , m_nextGivenFile( 0 )
    , m_started( false )
    , m_levels()
    , m_segmentCount( 0 )
    , m_segmentFields()
    , m_lock()
    , m_files()
    , m_pruned( 0 )
    , m_footerCacheHits( 0 )
    , m_skipped( 0 )
    , m_lastError()
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

CVfsTableScanSource::~CVfsTableScanSource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CVfsTableScanSource::Initialize( void )
{GUCEF_TRACE;

    m_segmentCount = m_spec.IsPartitioned() ? m_spec.GetSegmentCount() : 0;
    m_segmentFields.assign( m_segmentCount, TSegmentFieldVector() );
    for ( UInt32 s=0; s<m_segmentCount; ++s )
    {
        CORE::CString::StringVector names;
        m_spec.GetSegmentFields( s, names );
        for ( UInt32 n=0; n<(UInt32) names.size(); ++n )
        {
            const CORE::CDataTablePartitionSpec::SField* field = m_spec.FindField( names[ n ] );
            if ( GUCEF_NULL == field )
                continue;
            SSegmentField segmentField;
            segmentField.name = field->name;
            segmentField.transform = field->transform;
            segmentField.sourceColumn = m_tableSchema.IsNULL() ? -1 : m_tableSchema->FindColumn( field->sourceColumn );
            segmentField.kind = KEYKIND_SIGNED;
            if ( CORE::DATATABLE_PARTITION_TRANSFORM_IDENTITY == field->transform && segmentField.sourceColumn >= 0 )
            {
                const CORE::CDataTableColumnDef& column = m_tableSchema->GetColumn( (UInt32) segmentField.sourceColumn );
                if ( CORE::DataTableLogicalTypeIsUnsigned( column.type ) )
                    segmentField.kind = KEYKIND_UNSIGNED;
                else
                if ( !CORE::DataTableLogicalTypeIsInteger( column.type ) && CORE::DATATABLE_TYPE_DATE != column.type )
                    segmentField.kind = KEYKIND_TEXT;
            }
            m_segmentFields[ s ].push_back( segmentField );
        }
    }
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsTableScanSource::Fail( CORE::TDataTableStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VfsTables: " << message );
    MT::CScopeMutex lock( m_lock );
    m_lastError = message;
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsTableScanSource::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastError;
}

/*-------------------------------------------------------------------------*/

bool
CVfsTableScanSource::CandidateLess( UInt32 segment, const SCandidate& left, const SCandidate& right ) const
{GUCEF_TRACE;

    const TSegmentFieldVector& fields = m_segmentFields[ segment ];
    for ( UInt32 f=0; f<(UInt32) fields.size(); ++f )
    {
        switch ( fields[ f ].kind )
        {
            case KEYKIND_SIGNED:
            {
                if ( left.signedKeys[ f ] != right.signedKeys[ f ] )
                    return left.signedKeys[ f ] < right.signedKeys[ f ];
                break;
            }
            case KEYKIND_UNSIGNED:
            {
                if ( left.unsignedKeys[ f ] != right.unsignedKeys[ f ] )
                    return left.unsignedKeys[ f ] < right.unsignedKeys[ f ];
                break;
            }
            default:
            {
                // bytewise, unsigned: the order of text in the table (5.1)
                const int order = strcmp( left.textKeys[ f ].C_String(), right.textKeys[ f ].C_String() );
                if ( 0 != order )
                    return order < 0;
                break;
            }
        }
    }
    return strcmp( left.name.C_String(), right.name.C_String() ) < 0;
}

/*-------------------------------------------------------------------------*/

bool
CVfsTableScanSource::MakeCandidate( UInt32 segment, const CORE::CString& name, const CORE::CString& relativeDir,
                                    const CORE::CValueList& parentKeys, SCandidate& candidate ) const
{GUCEF_TRACE;

    candidate.keyValues = parentKeys;
    candidate.keyValues.SetAllowMultipleValues( false );
    if ( !m_spec.ParseSegment( segment, name, candidate.keyValues ) )
        return false;
    candidate.name = name;
    candidate.relativePath = relativeDir.IsNULLOrEmpty() ? name : relativeDir + "/" + name;

    const TSegmentFieldVector& fields = m_segmentFields[ segment ];
    candidate.signedKeys.assign( fields.size(), 0 );
    candidate.unsignedKeys.assign( fields.size(), 0 );
    candidate.textKeys.assign( fields.size(), CORE::CString::Empty );
    for ( UInt32 f=0; f<(UInt32) fields.size(); ++f )
    {
        CORE::CVariant value;
        if ( !candidate.keyValues.TryGetValue( fields[ f ].name, value ) )
            continue;
        if ( CORE::DATATABLE_PARTITION_TRANSFORM_IDENTITY != fields[ f ].transform )
        {
            candidate.signedKeys[ f ] = (Int64) value.AsInt32();
            continue;
        }
        const CORE::CString text = value.AsString();
        switch ( fields[ f ].kind )
        {
            case KEYKIND_UNSIGNED: { candidate.unsignedKeys[ f ] = CORE::StringToUInt64( text, 0 ); break; }
            case KEYKIND_TEXT: { candidate.textKeys[ f ] = text; break; }
            default:
            {
                CORE::CUtf8String utf8 = CORE::ToUtf8String( text );
                Int32 days = 0;
                const CORE::CDataTableColumnDef& column = m_tableSchema->GetColumn( (UInt32) fields[ f ].sourceColumn );
                if ( CORE::DATATABLE_TYPE_DATE == column.type && CORE::CDataTableValues::ParseDate( utf8.C_String(), utf8.ByteSize() - 1, days ) )
                    candidate.signedKeys[ f ] = days;
                else
                    candidate.signedKeys[ f ] = CORE::StringToInt64( text, 0 );
                break;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVfsTableScanSource::BuildFixedNames( UInt32 segment, CORE::CString::StringSet& names ) const
{GUCEF_TRACE;

    names.clear();
    if ( !m_hasQuery || m_tableSchema.IsNULL() )
        return false;

    // the values each field of the segment is fixed to by an EQUALS or IN predicate on its source column
    const TSegmentFieldVector& fields = m_segmentFields[ segment ];
    GUCEF::vector< CORE::CVariant::VariantVector > valuesPerField( fields.size() );
    UInt64 combinations = 1;
    for ( UInt32 f=0; f<(UInt32) fields.size(); ++f )
    {
        if ( fields[ f ].sourceColumn < 0 )
            return false;
        const CORE::CDataTableColumnDef& column = m_tableSchema->GetColumn( (UInt32) fields[ f ].sourceColumn );
        bool fixed = false;
        for ( UInt32 p=0; p<(UInt32) m_query.predicates.size() && !fixed; ++p )
        {
            const CORE::CDataTableResolvedPredicate& predicate = m_query.predicates[ p ];
            if ( predicate.columnIndex != (UInt32) fields[ f ].sourceColumn || predicate.matchesNothing )
                continue;
            if ( CORE::DATATABLE_PREDICATE_EQUALS == predicate.op )
            {
                valuesPerField[ f ].push_back( NativeToKeyValue( column, predicate.low ) );
                fixed = true;
            }
            else
            if ( CORE::DATATABLE_PREDICATE_IN == predicate.op )
            {
                for ( UInt32 v=0; v<(UInt32) predicate.values.size(); ++v )
                    valuesPerField[ f ].push_back( NativeToKeyValue( column, predicate.values[ v ] ) );
                fixed = true;
            }
        }
        if ( !fixed )
            return false;
        combinations *= valuesPerField[ f ].size();
        if ( combinations > MaxBuiltNamesPerLevel )
            return false;
    }

    // every combination of the values, as names of the segment
    GUCEF::vector< UInt32 > position( fields.size(), 0 );
    for ( UInt64 c=0; c<combinations; ++c )
    {
        CORE::CValueList keys;
        keys.SetAllowMultipleValues( false );
        for ( UInt32 f=0; f<(UInt32) fields.size(); ++f )
            keys.Set( fields[ f ].name, valuesPerField[ f ][ position[ f ] ] );
        CORE::CString name;
        if ( m_spec.BuildSegment( segment, keys, name ) )
            names.insert( name );
        for ( UInt32 f=0; f<(UInt32) fields.size(); ++f )
        {
            if ( ++position[ f ] < (UInt32) valuesPerField[ f ].size() )
                break;
            position[ f ] = 0;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVfsTableScanSource::ExpandLevel( UInt32 segment, const CORE::CString& relativeDir, const CORE::CValueList& parentKeys )
{GUCEF_TRACE;

    SLevel level;
    level.segment = segment;
    level.relativeDir = relativeDir;
    level.dirPath = m_store->MakePath( relativeDir );
    level.next = 0;
    const bool lastSegment = segment + 1 == m_segmentCount;

    CORE::CString::StringSet builtNames;
    bool built = false;
    if ( m_segmentFields[ segment ].empty() )
    {
        // a literal segment
        CORE::CString name;
        if ( m_spec.BuildSegment( segment, CORE::CValueList(), name ) )
            builtNames.insert( name );
        built = true;
    }
    else
    {
        built = BuildFixedNames( segment, builtNames );
    }

    CORE::CString::StringVector names;
    if ( built )
        names.assign( builtNames.begin(), builtNames.end() );
    else
        m_store->ListDirectory( level.dirPath, !lastSegment, names );

    UInt64 pruned = 0;
    TCandidateVector candidates;
    candidates.reserve( names.size() );
    for ( UInt32 i=0; i<(UInt32) names.size(); ++i )
    {
        // the directory of the table definitions is not a partition of any table
        if ( 0 == segment && CVfsTablesStore::TablesDirName == names[ i ] )
            continue;
        SCandidate candidate;
        if ( !MakeCandidate( segment, names[ i ], relativeDir, parentKeys, candidate ) )
            continue;
        if ( m_hasQuery && !m_spec.MayMatch( candidate.keyValues, m_query ) )
        {
            ++pruned;
            continue;
        }
        if ( built && lastSegment && !m_store->FileExists( m_store->MakePath( candidate.relativePath ) ) )
            continue;   // sparse: no file, no rows
        candidates.push_back( candidate );
    }
    if ( pruned > 0 )
    {
        MT::CScopeMutex lock( m_lock );
        m_pruned += pruned;
    }

    // in path order of the key values, through indices: candidates are not cheap to copy
    GUCEF::vector< UInt32 > order( candidates.size() );
    for ( UInt32 i=0; i<(UInt32) order.size(); ++i )
        order[ i ] = i;
    SCandidateOrder less;
    less.source = this;
    less.candidates = &candidates;
    less.segment = segment;
    std::sort( order.begin(), order.end(), less );

    m_levels.push_back( level );
    TCandidateVector& sorted = m_levels.back().candidates;
    sorted.reserve( candidates.size() );
    for ( UInt32 i=0; i<(UInt32) order.size(); ++i )
        sorted.push_back( candidates[ order[ i ] ] );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsTableScanSource::NextPartition( SPartitionFile& partition )
{GUCEF_TRACE;

    if ( m_useGivenFiles )
    {
        while ( m_nextGivenFile < (UInt32) m_givenFiles.size() )
        {
            const SPartitionFile& given = m_givenFiles[ m_nextGivenFile++ ];
            if ( m_store->FileExists( m_store->MakePath( given.relativePath ) ) )
            {
                partition = given;
                return CORE::DATATABLE_STATUS_OK;
            }
        }
        return CORE::DATATABLE_STATUS_END;
    }

    if ( m_query.matchesNothing )
        return CORE::DATATABLE_STATUS_END;

    if ( !m_started )
    {
        m_started = true;
        if ( 0 == m_segmentCount )
        {
            // one file
            if ( m_unpartitionedFile.IsNULLOrEmpty() || !m_store->FileExists( m_store->MakePath( m_unpartitionedFile ) ) )
                return CORE::DATATABLE_STATUS_END;
            partition.relativePath = m_unpartitionedFile;
            partition.keyValues.Clear();
            return CORE::DATATABLE_STATUS_OK;
        }
        ExpandLevel( 0, CORE::CString::Empty, CORE::CValueList() );
    }

    while ( !m_levels.empty() )
    {
        SLevel& top = m_levels.back();
        if ( top.next >= (UInt32) top.candidates.size() )
        {
            m_levels.pop_back();
            continue;
        }
        const UInt32 segment = top.segment;
        SCandidate& candidate = top.candidates[ top.next++ ];
        if ( segment + 1 == m_segmentCount )
        {
            partition.relativePath = candidate.relativePath;
            partition.keyValues = candidate.keyValues;
            return CORE::DATATABLE_STATUS_OK;
        }
        const CORE::CString relativeDir = candidate.relativePath;
        const CORE::CValueList keys = candidate.keyValues;
        ExpandLevel( segment + 1, relativeDir, keys );
    }
    return CORE::DATATABLE_STATUS_END;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsTableScanSource::NextFile( CORE::CDataTableScanFile& file )
{GUCEF_TRACE;

    SPartitionFile partition;
    const CORE::TDataTableStatus status = NextPartition( partition );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    file.Clear();
    file.path = m_store->MakePath( partition.relativePath );
    file.partitionKeys = partition.keyValues;
    file.fileSize = GUCEF_UINT64MAX;

    SFileEntry entry;
    entry.path = file.path;
    entry.opened = false;
    entry.generation = 0;
    MT::CScopeMutex lock( m_lock );
    file.backendData = m_files.size();
    m_files.push_back( entry );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsTableScanSource::OpenFile( const CORE::CDataTableScanFile& file, CORE::CITableReaderPtr& reader )
{GUCEF_TRACE;

    reader.Unlink();
    TDataTableSharedFilePtr shared;
    CORE::CITableFileMetaDataPtr planned;
    CORE::CString path;
    bool reopen = false;
    UInt64 generation = GUCEF_UINT64MAX;
    {
        MT::CScopeMutex lock( m_lock );
        if ( file.backendData >= (UInt64) m_files.size() )
        {
            m_lastError = "The file \"" + file.path + "\" was not listed by this source";
            return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
        }
        SFileEntry& entry = m_files[ (size_t) file.backendData ];
        shared = entry.sharedFile.GetSharedPtr();
        planned = entry.plannedMetaData.GetSharedPtr();
        path = entry.path;
        reopen = entry.opened;
        generation = entry.generation;
    }

    if ( shared.IsNULL() )
    {
        // the first reader of the file opens it; a reader after the others went must find the version they read
        bool fromCache = false;
        CORE::CString error;
        const CORE::TDataTableStatus status = m_store->OpenSharedFile( path, m_codec, m_codecOptions, shared, fromCache, error,
                                                                      reopen ? generation : GUCEF_UINT64MAX,
                                                                      reopen ? planned : CORE::CITableFileMetaDataPtr() );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return Fail( status, error );

        MT::CScopeMutex lock( m_lock );
        SFileEntry& entry = m_files[ (size_t) file.backendData ];
        TDataTableSharedFilePtr existing = entry.sharedFile.GetSharedPtr();
        if ( !existing.IsNULL() )
        {
            shared = existing;
        }
        else
        {
            entry.sharedFile = shared;
            entry.opened = true;
            entry.generation = shared->generation;
            if ( fromCache )
                ++m_footerCacheHits;
        }
    }

    CORE::CString mismatch;
    if ( m_store->GetSettings().skipIncompatibleFiles && !m_tableSchema->IsReadableAs( *shared->metaData->GetSchema(), mismatch ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VfsTables: skipping \"" << path << "\": " << mismatch );
        reader = CORE::CITableReaderPtr( GUCEF_NEW CSkippedFileReader( m_tableSchema ) );
        MT::CScopeMutex lock( m_lock );
        ++m_skipped;
        return CORE::DATATABLE_STATUS_OK;
    }

    CORE::IOAccessPtr input( GUCEF_NEW CSharedFileReadAccess( shared ) );
    CORE::CString error;
    const CORE::TDataTableStatus status = m_codec->CreateReader( input, shared->metaData, m_codecOptions, reader, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, "A reader on \"" + path + "\" could not be made: " + error );

    // what the scan plans the file with: a file opened again must hold it (the scan keeps it while the file is in play)
    if ( planned.IsNULL() )
    {
        MT::CScopeMutex lock( m_lock );
        SFileEntry& entry = m_files[ (size_t) file.backendData ];
        if ( entry.plannedMetaData.GetSharedPtr().IsNULL() )
            entry.plannedMetaData = reader->GetMetaData();
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CVfsTableScanSource::AddSourceStats( CORE::CDataTableScanStats& stats ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    stats.filesPruned += m_pruned;
    stats.footerCacheHits += m_footerCacheHits;
    stats.filesSkipped += m_skipped;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFSTABLES */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
