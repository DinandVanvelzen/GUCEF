/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNTABLEPUBLISHER_H
#include "gucefDRN_CDRNTablePublisher.h"
#define GUCEF_DRN_CDRNTABLEPUBLISHER_H
#endif /* GUCEF_DRN_CDRNTABLEPUBLISHER_H ? */

#include "gucefDRN_CDRNPublishedTable.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CDRNPublishedTable::SchemaChangedEvent = "GUCEF::DRN::CDRNPublishedTable::SchemaChangedEvent";

/** Interest of an observer: everything, or RowsChangedEvent alone */
static const UInt32 InterestAll = 1;
static const UInt32 InterestRows = 2;

/** The keys of changed rows */
typedef GUCEF::vector< CORE::CVariant > TDRNKeyVector;

/** A scan stops after this many key indices in a row that never held a key */
static const UInt32 ScanGapKeyCount = 4096;

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTable::RegisterEvents( void )
{GUCEF_TRACE;

    SchemaChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTable::CSessionWatcher::OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventid, CORE::CICloneable* /* eventdata */ )
{GUCEF_TRACE;

    if ( CORE::CNotifier::DestructionEvent == eventid && GUCEF_NULL != m_table )
        m_table->OnSessionDestroyed();
}

/*-------------------------------------------------------------------------*/

CDRNPublishedTable::CDRNPublishedTable( CDRNDataGrid& grid, const CDRNPublishedTableSettings& settings, CORE::PulseGeneratorPtr pulseGenerator )
    : CORE::CIDataTable( pulseGenerator )
    , m_grid( &grid )
    , m_session( &grid.GetSession() )
    , m_settings( settings )
    , m_name( grid.GetName() )
    , m_sessionWatcher()
    , m_lock()
    , m_schemaRevision( 0 )
    , m_haveSchema( false )
    , m_unsupported( false )
    , m_published()
    , m_definition()
    , m_schema()
    , m_rows()
    , m_mismatchedValues( 0 )
    , m_undecodableRows( 0 )
    , m_lastCacheUse( 0 )
    , m_snapshot()
    , m_snapshotTime( 0 )
    , m_lastError()
    , m_subscriptionLock()
    , m_observers()
    , m_following( false )
    , m_deferredKeys()
    , m_deferredSince( 0 )
    , m_flushPending( false )
    , m_cacheTimer( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();
    m_definition.name = m_name;
    m_definition.codecType = "drn";
    m_sessionWatcher.m_table = this;
    m_sessionWatcher.SubscribeTo( m_session, CORE::CNotifier::DestructionEvent );
    if ( 0 != m_settings.decodeCacheIdleMs )
    {
        // runs only while there are decoded rows
        m_cacheTimer = GUCEF_NEW CORE::CTimer( GetEventPulseGenerator(), m_settings.decodeCacheIdleMs );
        TEventCallback timerCallback( this, &CDRNPublishedTable::OnCacheTimer );
        SubscribeTo( m_cacheTimer, CORE::CTimer::TimerUpdateEvent, timerCallback );
    }
    RefreshSchema();
}

/*-------------------------------------------------------------------------*/

CDRNPublishedTable::~CDRNPublishedTable()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_cacheTimer )
        m_cacheTimer->SetEnabled( false );
    m_sessionWatcher.m_table = GUCEF_NULL;
    m_sessionWatcher.UnsubscribeAllFromObserver();
    UnsubscribeAllFromObserver();
    GUCEF_DELETE m_cacheTimer;
    m_cacheTimer = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTable::OnSessionDestroyed( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_grid = GUCEF_NULL;
    m_session = GUCEF_NULL;
    m_following = false;
}

/*-------------------------------------------------------------------------*/

CDRNDataGrid*
CDRNPublishedTable::AcquireGrid( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_grid;
}

/*-------------------------------------------------------------------------*/

const CDRNPublishedTableSettings&
CDRNPublishedTable::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNPublishedTable::Fail( CORE::TDataTableStatus status, const CORE::CString& message ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_lastError = message;
    return status;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTable::RefreshSchema( void ) const
{GUCEF_TRACE;

    // #schema is read again only when its grid revision moved; returns whether it changed. The value is copied under
    // the node lock and decoded without it
    CDRNDataGrid* grid = AcquireGrid();
    if ( GUCEF_NULL == grid )
        return false;
    CORE::CVariant value;
    UInt32 revision = 0;
    {
        MT::CScopeMutex nodeLock( m_session->GetNode().GetLock() );
        UInt32 index = grid->GetKeyIndex( CORE::CVariant( CDRNTablePublisher::SchemaKey ) );
        if ( CDRNDataGrid::InvalidKeyIndex == index )
            return false;
        revision = grid->GetRevision( index );
        {
            MT::CScopeMutex lock( m_lock );
            if ( ( m_haveSchema || m_unsupported || 0 != m_schemaRevision ) && revision == m_schemaRevision )
                return false;
        }
        if ( !grid->GetValueByIndex( index, value ) )
            return false;
    }

    CORE::CVariantStream stream;
    stream.GetBuffer().CopyFrom( value.ByteSize( false ), value.AsVoidPtr() );
    stream.ResetReadPosition();
    CDRNPublishedTableSchema published;
    bool unsupported = false;
    CORE::CDataTableSchema schema;
    bool readable = published.Decode( stream, unsupported ) && published.ToDataTableSchema( schema );

    MT::CScopeMutex lock( m_lock );
    if ( revision == m_schemaRevision && ( m_haveSchema || m_unsupported ) )
        return false;   // another thread read the same revision meanwhile
    m_schemaRevision = revision;
    m_rows.clear();         // the rows decode against the new columns
    m_snapshot.Unlink();
    m_undecodableRows = 0;
    if ( !readable )
    {
        m_haveSchema = false;
        m_unsupported = unsupported;
        m_schema = CORE::CDataTableSchemaPtr();
        m_definition.schema = CORE::CDataTableSchema();
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNPublishedTable(" << m_name << "): " << ( unsupported ? "the schema is of a newer version" : "the schema cannot be read or is not valid" ) );
        return true;
    }
    m_published = published;
    m_unsupported = false;
    m_haveSchema = true;
    m_definition.schema = schema;
    m_schema = CORE::CreateDataTableSchemaPtr( schema );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTable::IsValid( void ) const
{GUCEF_TRACE;

    RefreshSchema();
    MT::CScopeMutex lock( m_lock );
    return m_haveSchema && !m_unsupported;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTable::IsUnsupported( void ) const
{GUCEF_TRACE;

    RefreshSchema();
    MT::CScopeMutex lock( m_lock );
    return m_unsupported;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTable::GetPublishedSchema( CDRNPublishedTableSchema& schema ) const
{GUCEF_TRACE;

    RefreshSchema();
    MT::CScopeMutex lock( m_lock );
    if ( !m_haveSchema )
        return false;
    schema = m_published;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTable::GetColumnHints( UInt32 columnIndex, CORE::CValueList& hints ) const
{GUCEF_TRACE;

    RefreshSchema();
    MT::CScopeMutex lock( m_lock );
    if ( !m_haveSchema || columnIndex >= m_published.columns.size() )
        return false;

    // the column of the data table schema: the key first, then the others in published order
    UInt32 published = m_published.keyColumn;
    if ( 0 != columnIndex )
        published = columnIndex <= m_published.keyColumn ? columnIndex - 1 : columnIndex;
    const CDRNPublishedColumn& column = m_published.columns[ published ];
    hints = column.hints;
    hints.Set( "id", CORE::CVariant( column.id ) );
    if ( !column.title.IsNULLOrEmpty() )
        hints.Set( "title", CORE::CVariant( column.title ) );
    if ( 0 != column.width )
        hints.Set( "width", CORE::CVariant( column.width ) );
    if ( column.hidden )
        hints.Set( "hidden", CORE::CVariant( true ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTable::GetMeta( CDRNPublishedTableMeta& meta ) const
{GUCEF_TRACE;

    CDRNDataGrid* grid = AcquireGrid();
    CORE::CVariant value;
    if ( GUCEF_NULL == grid || !grid->GetValue( CORE::CVariant( CDRNTablePublisher::MetaKey ), value ) )
        return false;
    CORE::CVariantStream stream;
    stream.GetBuffer().CopyFrom( value.ByteSize( false ), value.AsVoidPtr() );
    stream.ResetReadPosition();
    bool unsupported = false;
    return meta.Decode( stream, unsupported );
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNPublishedTable::GetPageRevision( void ) const
{GUCEF_TRACE;

    CDRNPublishedTableMeta meta;
    return GetMeta( meta ) ? meta.pageRevision : 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNPublishedTable::GetSourceVersion( void ) const
{GUCEF_TRACE;

    CDRNPublishedTableMeta meta;
    return GetMeta( meta ) ? meta.sourceVersion : 0;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTable::GetScalar( const CORE::CString& name, CORE::CVariant& value ) const
{GUCEF_TRACE;

    CDRNPublishedTableMeta meta;
    if ( !GetMeta( meta ) || !meta.scalars.HasKey( name ) )
        return false;
    value = meta.scalars.GetValue( name );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTable::DecodeRow( const CORE::CVariant& value, UInt32 cellCount, CRowCache& row )
{GUCEF_TRACE;

    // a row: version, the revision it last changed in, then per column but the key a presence flag and the value. The
    // decode stops at the first value that is not there: a row is never read further than its own bytes
    row.decoded = true;
    row.present = false;
    CORE::CVariantStream stream;
    stream.GetBuffer().CopyFrom( value.ByteSize( false ), value.AsVoidPtr() );
    stream.ResetReadPosition();
    UInt8 version = 0;
    stream >> version >> row.rowRevision;
    if ( !stream.IsValid() || version > CDRNTablePublisher::RowEncodingVersion )
        return false;
    row.cells.resize( cellCount );
    for ( UInt32 i=0; i<cellCount; ++i )
    {
        bool present = false;
        if ( !stream.HasUnreadData() )
            return false;
        stream >> present;
        row.cells[ i ] = CORE::CVariant();
        if ( present )
        {
            if ( !stream.HasUnreadData() )
                return false;
            stream >> row.cells[ i ];
        }
        if ( !stream.IsValid() )
            return false;
    }
    row.present = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTable::UpdateCache( void ) const
{GUCEF_TRACE;

    // decodes the rows whose grid revision moved. Called without the node lock and the table lock: per chunk of
    // scanChunkKeyCount keys the values of the changed rows are copied under the node lock, decoded without it, and
    // stored under the table lock
    CDRNDataGrid* grid = AcquireGrid();
    if ( GUCEF_NULL == grid )
        return false;
    const UInt32 chunk = 0 == m_settings.scanChunkKeyCount ? GUCEF_UINT32MAX : m_settings.scanChunkKeyCount;
    TCopiedRowVector copied;
    TRowCacheVector decoded;
    UInt32 schemaRevision = 0;
    UInt32 cellCount = 0;
    UInt32 newFailures = 0;
    UInt32 index = 0;
    UInt32 emptyRun = 0;
    bool more = true;
    while ( more )
    {
        copied.clear();
        {
            MT::CScopeMutex nodeLock( m_session->GetNode().GetLock() );
            MT::CScopeMutex lock( m_lock );
            if ( !m_haveSchema )
                return false;
            if ( 0 == index )
            {
                schemaRevision = m_schemaRevision;
                cellCount = (UInt32) m_published.columns.size() - 1;
            }
            else
            if ( schemaRevision != m_schemaRevision )
                return false;
            for ( UInt32 visited=0; visited < chunk && emptyRun < ScanGapKeyCount; ++visited, ++index )
            {
                CORE::CVariant key = grid->GetKeyByIndex( index );
                if ( !key.IsInitialized() )
                {
                    ++emptyRun;
                    ForgetRow( index );
                    continue;
                }
                emptyRun = 0;
                if ( CDRNTablePublisher::IsReservedKey( key ) )
                    continue;
                UInt32 revision = grid->GetRevision( index );
                if ( index < m_rows.size() && m_rows[ index ].decoded && m_rows[ index ].gridRevision == revision )
                    continue;
                copied.push_back( CCopiedRow() );
                CCopiedRow& row = copied.back();
                if ( !grid->GetValueByIndex( index, row.value ) )
                {
                    // removed: nothing to decode
                    copied.pop_back();
                    ForgetRow( index );
                    continue;
                }
                row.index = index;
                row.gridRevision = revision;
                row.key = key;
            }
            more = emptyRun < ScanGapKeyCount;
        }
        if ( copied.empty() )
            continue;

        // without the node lock
        decoded.resize( copied.size() );
        for ( UInt32 i=0; i<copied.size(); ++i )
        {
            CRowCache& row = decoded[ i ];
            row = CRowCache();
            row.gridRevision = copied[ i ].gridRevision;
            if ( DecodeRow( copied[ i ].value, cellCount, row ) )
                row.key = copied[ i ].key;
            else
                ++newFailures;
        }

        MT::CScopeMutex lock( m_lock );
        if ( !m_haveSchema || schemaRevision != m_schemaRevision )
            return false;
        for ( UInt32 i=0; i<copied.size(); ++i )
        {
            UInt32 rowIndex = copied[ i ].index;
            if ( m_rows.size() <= rowIndex )
                m_rows.resize( rowIndex + 1 );
            CRowCache& row = m_rows[ rowIndex ];
            if ( row.decoded && !row.present && m_undecodableRows > 0 )
                --m_undecodableRows;
            row = decoded[ i ];
            if ( !row.present )
                ++m_undecodableRows;
        }
    }
    UseCache();
    if ( newFailures > 0 )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNPublishedTable(" << m_name << "): " << newFailures <<
                           " rows do not decode against the schema of the table and are left out" );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTable::ForgetRow( UInt32 index ) const
{GUCEF_TRACE;

    // a key index without a row (never used, removed): the table lock is held by the caller
    if ( index >= m_rows.size() )
        return;
    CRowCache& row = m_rows[ index ];
    if ( row.decoded && !row.present && m_undecodableRows > 0 )
        --m_undecodableRows;
    row.decoded = false;
    row.present = false;
}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTable::UseCache( void ) const
{GUCEF_TRACE;

    // the decoded rows were used now: they stay for decodeCacheIdleMs from here
    {
        MT::CScopeMutex lock( m_lock );
        m_lastCacheUse = CDRNClock::NowInMicroseconds();
    }
    if ( GUCEF_NULL != m_cacheTimer && !m_cacheTimer->GetEnabled() )
        m_cacheTimer->SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTable::IsConsistent( void ) const
{GUCEF_TRACE;

    RefreshSchema();
    CDRNDataGrid* grid = AcquireGrid();
    if ( GUCEF_NULL == grid )
        return false;

    // #meta before and after the rows are brought up to date: a #meta that moved meanwhile is a page in transit
    CORE::CVariant value;
    UInt32 metaRevision = 0;
    {
        MT::CScopeMutex nodeLock( m_session->GetNode().GetLock() );
        UInt32 index = grid->GetKeyIndex( CORE::CVariant( CDRNTablePublisher::MetaKey ) );
        if ( CDRNDataGrid::InvalidKeyIndex == index || !grid->GetValueByIndex( index, value ) )
            return false;
        metaRevision = grid->GetRevision( index );
    }
    if ( !UpdateCache() )
        return false;
    {
        MT::CScopeMutex nodeLock( m_session->GetNode().GetLock() );
        if ( metaRevision != grid->GetRevision( grid->GetKeyIndex( CORE::CVariant( CDRNTablePublisher::MetaKey ) ) ) )
            return false;
    }

    CDRNPublishedTableMeta meta;
    CORE::CVariantStream stream;
    stream.GetBuffer().CopyFrom( value.ByteSize( false ), value.AsVoidPtr() );
    stream.ResetReadPosition();
    bool unsupported = false;
    if ( !meta.Decode( stream, unsupported ) )
        return false;
    UInt32 removed = 0;
    if ( stream.HasUnreadData() )
        stream >> removed;      // the removals of this revision, appended by the publisher
    if ( !stream.IsValid() )
        removed = 0;

    MT::CScopeMutex lock( m_lock );
    UInt32 rows = 0;
    UInt32 ofRevision = 0;
    for ( UInt32 i=0; i<m_rows.size(); ++i )
    {
        const CRowCache& row = m_rows[ i ];
        if ( !row.present )
            continue;
        if ( row.rowRevision > meta.pageRevision )
            return false;
        ++rows;
        if ( row.rowRevision == meta.pageRevision )
            ++ofRevision;
    }
    return rows == meta.rowCount && ofRevision + removed == meta.changedRowCount;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNPublishedTable::GetMismatchedValueCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_mismatchedValues;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNPublishedTable::GetUndecodableRowCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_undecodableRows;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNPublishedTable::GetCachedRowCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt32 count = 0;
    for ( UInt32 i=0; i<m_rows.size(); ++i )
        if ( m_rows[ i ].present )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNPublishedTable::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataTableDefinition&
CDRNPublishedTable::GetDefinition( void ) const
{GUCEF_TRACE;

    RefreshSchema();
    return m_definition;
}

/*-------------------------------------------------------------------------*/

CORE::CDataTableSchemaPtr
CDRNPublishedTable::GetSchema( void ) const
{GUCEF_TRACE;

    RefreshSchema();
    MT::CScopeMutex lock( m_lock );
    return m_schema;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTable::IsReadOnly( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNPublishedTable::Scan( const CORE::CDataTableQuery& query, CORE::CDataTableScanPtr& scan )
{GUCEF_TRACE;

    scan.Unlink();
    RefreshSchema();
    CDRNDataGrid* grid = AcquireGrid();
    if ( GUCEF_NULL == grid )
        return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "The grid of the published table \"" + m_name + "\" is gone" );

    CORE::CMemoryDataTablePtr copy;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_unsupported )
            return Fail( CORE::DATATABLE_STATUS_UNSUPPORTED, "The schema of the published table \"" + m_name + "\" is of a newer version" );
        if ( !m_haveSchema )
            return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "The published table \"" + m_name + "\" has no schema yet" );

        // a copy of the whole table younger than snapshotCacheMs serves the scan as it is
        if ( 0 != m_settings.snapshotCacheMs && !m_snapshot.IsNULL() &&
             CDRNClock::NowInMicroseconds() - m_snapshotTime < (UInt64) m_settings.snapshotCacheMs * 1000 )
            copy = m_snapshot;
    }
    if ( !copy.IsNULL() )
        UseCache();
    else
    {
        CORE::TDataTableStatus status = BuildCopy( copy );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
    }
    CORE::TDataTableStatus status = copy->Scan( query, scan );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, copy->GetLastErrorMessage() );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNPublishedTable::BuildCopy( CORE::CMemoryDataTablePtr& copy )
{GUCEF_TRACE;

    // the rows that changed are decoded (UpdateCache); the batches are built from a copy of the decoded rows, without locks
    if ( !UpdateCache() )
        return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "The published table \"" + m_name + "\" has no readable schema" );
    CORE::CDataTableBufferPool& pool = CORE::CCoreGlobal::Instance()->GetDataTableBufferPool();
    CORE::CDataTableSchemaPtr schema;
    CORE::CDataTableDefinition definition;
    CDRNPublishedTableSchema published;
    UInt32 schemaRevision = 0;
    TRowCacheVector rows;
    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_haveSchema )
            return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "The published table \"" + m_name + "\" has no schema yet" );
        schema = m_schema;
        definition = m_definition;
        published = m_published;
        schemaRevision = m_schemaRevision;
        rows.reserve( m_rows.size() );
        for ( UInt32 i=0; i<m_rows.size(); ++i )
            if ( m_rows[ i ].present )
                rows.push_back( m_rows[ i ] );
    }

    copy = CORE::CMemoryDataTablePtr( GUCEF_NEW CORE::CMemoryDataTable( definition, &pool, GetPulseGenerator() ) );
    if ( !copy->IsValid() )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "The rows cannot be stored: " + copy->GetLastErrorMessage() );
    CORE::CDataTableBatchBuilder builder( schema, pool );
    const UInt32 chunk = 0 == m_settings.scanChunkKeyCount ? GUCEF_UINT32MAX : m_settings.scanChunkKeyCount;
    UInt64 mismatched = 0;
    for ( UInt32 r=0; r<rows.size(); ++r )
    {
        const CRowCache& row = rows[ r ];
        if ( !builder.GetColumn( 0 ).Append( row.key ) )
            continue;
        UInt32 cell = 0;
        for ( UInt32 c=0; c<published.columns.size(); ++c )
        {
            if ( c == published.keyColumn )
                continue;
            CORE::CDataTableColumnBuilder& column = builder.GetColumn( published.ToDataTableColumnIndex( c ) );
            const CORE::CVariant& value = row.cells[ cell++ ];
            if ( !value.IsInitialized() )
            {
                column.AppendNull();
            }
            else
            if ( !column.Append( value ) )
            {
                // a value that does not fit its column shows as null and is counted
                column.AppendNull();
                ++mismatched;
            }
        }
        if ( builder.GetRowCount() >= chunk || r + 1 == rows.size() )
        {
            CORE::CDataTableBatchPtr batch;
            CORE::CString error;
            CORE::TDataTableStatus status = builder.Build( batch, error );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return Fail( status, "Building the rows failed: " + error );
            CORE::CDataTableWriteResult written;
            status = copy->Write( *batch, CORE::DATATABLE_WRITE_INSERT, written );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return Fail( status, "Storing the rows failed: " + copy->GetLastErrorMessage() );
        }
    }
    MT::CScopeMutex lock( m_lock );
    m_mismatchedValues += mismatched;
    if ( 0 != m_settings.snapshotCacheMs && schemaRevision == m_schemaRevision )
    {
        m_snapshot = copy;
        m_snapshotTime = CDRNClock::NowInMicroseconds();
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNPublishedTable::ScanAsync( const CORE::CDataTableQuery& query, CORE::CDataTableScanPtr& scan, CORE::ThreadPoolPtr /* threadPool */ )
{GUCEF_TRACE;

    // the copy is taken now, so the scan has the state of the moment of the call
    if ( CORE::DATATABLE_STATUS_OK != Scan( query, scan ) )
        return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_FINISHED );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNPublishedTable::Write( const CORE::CDataTableBatch& /* rows */, CORE::TDataTableWriteMode /* mode */, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Fail( CORE::DATATABLE_STATUS_READ_ONLY, "A published table is read only: changes go through the application" );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNPublishedTable::WriteAsync( const CORE::CDataTableBatchPtr& /* rows */, CORE::TDataTableWriteMode /* mode */, CORE::ThreadPoolPtr /* threadPool */ )
{GUCEF_TRACE;

    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNPublishedTable::DeleteRows( const CORE::CDataTableQuery& /* predicates */, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Fail( CORE::DATATABLE_STATUS_READ_ONLY, "A published table is read only" );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNPublishedTable::DeleteRowsAsync( const CORE::CDataTableQuery& /* predicates */, CORE::ThreadPoolPtr /* threadPool */ )
{GUCEF_TRACE;

    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNPublishedTable::DeletePartitions( const CORE::CDataTableQuery& /* partitionFilter */, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Fail( CORE::DATATABLE_STATUS_UNSUPPORTED, "A published table has no partitions" );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNPublishedTable::DeletePartitionsAsync( const CORE::CDataTableQuery& /* partitionFilter */, CORE::ThreadPoolPtr /* threadPool */ )
{GUCEF_TRACE;

    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNPublishedTable::ListPartitions( const CORE::CDataTableQuery& /* partitionFilter */, bool /* withFileStats */, CORE::CDataTablePartitionInfo::TPartitionInfoVector& partitions )
{GUCEF_TRACE;

    partitions.clear();
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNPublishedTable::ListPartitionsAsync( const CORE::CDataTableQuery& /* partitionFilter */, bool /* withFileStats */, CORE::ThreadPoolPtr /* threadPool */ )
{GUCEF_TRACE;

    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNPublishedTable::Compact( const CORE::CDataTableQuery& /* partitionFilter */, bool /* force */, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Fail( CORE::DATATABLE_STATUS_UNSUPPORTED, "A published table has nothing to compact" );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNPublishedTable::CompactAsync( const CORE::CDataTableQuery& /* partitionFilter */, bool /* force */, CORE::ThreadPoolPtr /* threadPool */ )
{GUCEF_TRACE;

    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTable::Refresh( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_rows.clear();
    m_snapshot.Unlink();
    m_undecodableRows = 0;
    m_haveSchema = false;
    m_unsupported = false;
    m_schemaRevision = 0;
}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTable::OnCacheTimer( CORE::CNotifier* /* notifier */, const CORE::CEvent& /* eventid */, CORE::CICloneable* /* eventData */ )
{GUCEF_TRACE;

    // the decoded rows and the copy of the table go when nothing used them for decodeCacheIdleMs
    bool idle = false;
    {
        MT::CScopeMutex lock( m_lock );
        if ( CDRNClock::NowInMicroseconds() - m_lastCacheUse >= (UInt64) m_settings.decodeCacheIdleMs * 1000 )
        {
            idle = true;
            m_rows.clear();
            m_snapshot.Unlink();
            m_undecodableRows = 0;
        }
    }
    if ( idle && GUCEF_NULL != m_cacheTimer )
        m_cacheTimer->SetEnabled( false );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNPublishedTable::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastError;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNPublishedTable::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNPublishedTable";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTable::Subscribe( CORE::CObserver* observer )
{GUCEF_TRACE;

    const bool subscribed = CORE::CIDataTable::Subscribe( observer );
    if ( subscribed )
        NoteInterest( observer, InterestAll );
    return subscribed;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTable::Subscribe( CORE::CObserver* observer, const CORE::CEvent& eventid, CORE::CIEventHandlerFunctorBase* callback )
{GUCEF_TRACE;

    const bool subscribed = CORE::CIDataTable::Subscribe( observer, eventid, callback );
    if ( subscribed && ( RowsChangedEvent == eventid || SchemaChangedEvent == eventid ) )
        NoteInterest( observer, InterestRows );
    return subscribed;
}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTable::Unsubscribe( CORE::CObserver* observer )
{GUCEF_TRACE;

    CORE::CIDataTable::Unsubscribe( observer );
    DropInterest( observer, InterestAll | InterestRows );
}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTable::Unsubscribe( CORE::CObserver* observer, const CORE::CEvent& eventid )
{GUCEF_TRACE;

    CORE::CIDataTable::Unsubscribe( observer, eventid );
    if ( RowsChangedEvent == eventid || SchemaChangedEvent == eventid )
        DropInterest( observer, InterestRows );
}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTable::UnsubscribeAllFromNotifier( void )
{GUCEF_TRACE;

    CORE::CIDataTable::UnsubscribeAllFromNotifier();
    {
        MT::CScopeMutex lock( m_subscriptionLock );
        m_observers.clear();
    }
    UpdateGridSubscriptions();
}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTable::OnObserverDestruction( CORE::CObserver* observer )
{GUCEF_TRACE;

    CORE::CIDataTable::OnObserverDestruction( observer );
    DropInterest( observer, InterestAll | InterestRows );
}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTable::NoteInterest( CORE::CObserver* observer, UInt32 interest )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_subscriptionLock );
        m_observers[ observer ] |= interest;
    }
    UpdateGridSubscriptions();
}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTable::DropInterest( CORE::CObserver* observer, UInt32 interest )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_subscriptionLock );
        TObserverMap::iterator i = m_observers.find( observer );
        if ( i == m_observers.end() )
            return;
        (*i).second &= ~interest;
        if ( 0 == (*i).second )
            m_observers.erase( i );
    }
    UpdateGridSubscriptions();
}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTable::UpdateGridSubscriptions( void )
{GUCEF_TRACE;

    // The grid knows of this table only while somebody listens to it. The change is made under the notification lock of
    // the grid, then the subscription lock: the grid raises its events under its notification lock and their handlers
    // take the subscription lock (OnChangedIndices()), so the order is the same, and a change made from within such a
    // handler takes the grid's lock again (gucefDRN design 9, lock order)
    CDRNDataGrid* grid = AcquireGrid();
    CORE::CNotifierScopeLock gridLock( grid );
    bool subscribe = false;
    bool unsubscribe = false;
    {
        MT::CScopeMutex lock( m_subscriptionLock );
        if ( GUCEF_NULL == grid )
        {
            m_following = false;
            return;
        }
        const bool wanted = !m_observers.empty();
        if ( wanted && !m_following )
        {
            subscribe = true;
            m_following = true;
        }
        else
        if ( !wanted && m_following )
        {
            unsubscribe = true;
            m_following = false;
        }
    }
    if ( subscribe )
    {
        TEventCallback updatedCallback( this, &CDRNPublishedTable::OnGridUpdated );
        TEventCallback changedCallback( this, &CDRNPublishedTable::OnGridKeyChanged );
        SubscribeTo( grid, CDRNDataGrid::GridUpdatedEvent, updatedCallback );
        SubscribeTo( grid, CDRNDataGrid::GridKeyChangedEvent, changedCallback );
    }
    else
    if ( unsubscribe )
    {
        UnsubscribeFrom( grid );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNPublishedTable::GetGridSubscriptionCount( void ) const
{GUCEF_TRACE;

    return m_following ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTable::OnGridUpdated( CORE::CNotifier* /* notifier */, const CORE::CEvent& /* eventid */, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CDRNGridUpdatedEventData* data = static_cast< CDRNGridUpdatedEventData* >( eventData );
    if ( GUCEF_NULL != data )
        OnChangedIndices( data->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTable::OnGridKeyChanged( CORE::CNotifier* /* notifier */, const CORE::CEvent& /* eventid */, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::TCloneableUInt32* data = static_cast< CORE::TCloneableUInt32* >( eventData );
    if ( GUCEF_NULL == data )
        return;
    TDRNKeyIndexVector indices;
    indices.push_back( data->GetData() );
    OnChangedIndices( indices );
}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTable::OnChangedIndices( const TDRNKeyIndexVector& indices )
{GUCEF_TRACE;

    CDRNDataGrid* grid = AcquireGrid();
    if ( GUCEF_NULL == grid )
        return;

    // the row keys that changed; #meta alone raises no row event, #schema raises SchemaChangedEvent
    bool schemaChanged = false;
    TDRNKeyVector keys;
    for ( UInt32 i=0; i<(UInt32) indices.size(); ++i )
    {
        CORE::CVariant key = grid->GetKeyByIndex( indices[ i ] );
        if ( !CDRNTablePublisher::IsReservedKey( key ) )
            keys.push_back( key );
        else
        if ( key.AsString() == CDRNTablePublisher::SchemaKey )
            schemaChanged = true;
    }
    if ( schemaChanged && RefreshSchema() )
        NotifyObserversFromThread( SchemaChangedEvent );

    // the changes of one tick (a grid with notify PER_CHANGE reports them one by one) go out together, on the next pulse.
    // The subscription to the pulse generator is made without the subscription lock, which its pulses take
    // (OnFlushPulse()); only the call that set m_flushPending makes it
    CORE::PulseGeneratorPtr pulseGenerator = GetEventPulseGenerator();
    {
        MT::CScopeMutex lock( m_subscriptionLock );
        m_deferredKeys.insert( keys.begin(), keys.end() );
        if ( m_deferredKeys.empty() || m_flushPending || pulseGenerator.IsNULL() )
            return;
        m_flushPending = true;
    }
    TEventCallback flushCallback( this, &CDRNPublishedTable::OnFlushPulse );
    SubscribeTo( pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, flushCallback );
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CDRNPublishedTable::GetEventPulseGenerator( void ) const
{GUCEF_TRACE;

    CORE::PulseGeneratorPtr pulseGenerator = GetPulseGenerator();
    if ( pulseGenerator.IsNULL() )
        pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    return pulseGenerator;
}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTable::OnFlushPulse( CORE::CNotifier* /* notifier */, const CORE::CEvent& /* eventid */, CORE::CICloneable* /* eventData */ )
{GUCEF_TRACE;

    // the node lock (in IsConsistent) is never taken under the subscription lock
    const bool consistent = !m_settings.deferUntilConsistent || IsConsistent();
    CORE::TDataTableRowsChangedEventData changed;
    {
        // held back while an update has not arrived whole, at most maxDeferMs
        MT::CScopeMutex lock( m_subscriptionLock );
        UInt64 now = CDRNClock::NowInMicroseconds();
        if ( 0 == m_deferredSince )
            m_deferredSince = now;
        if ( !consistent && now - m_deferredSince < (UInt64) m_settings.maxDeferMs * 1000 )
            return;

        changed.GetData().reserve( m_deferredKeys.size() );
        for ( GUCEF::set< CORE::CVariant >::const_iterator i=m_deferredKeys.begin(); i!=m_deferredKeys.end(); ++i )
            changed.GetData().push_back( *i );
        m_deferredKeys.clear();
        m_deferredSince = 0;
        m_flushPending = false;
        CORE::PulseGeneratorPtr pulseGenerator = GetEventPulseGenerator();
        if ( !pulseGenerator.IsNULL() )
            UnsubscribeFrom( pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent );
    }
    if ( !changed.GetData().empty() )
        NotifyObservers( RowsChangedEvent, &changed );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
