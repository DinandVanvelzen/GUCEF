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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef GUCEF_CORE_CDATATABLEMERGE_H
#include "gucefCORE_CDataTableMerge.h"
#define GUCEF_CORE_CDATATABLEMERGE_H
#endif /* GUCEF_CORE_CDATATABLEMERGE_H ? */

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

#include "gucefDRN_CDRNDataGridTable.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The columns of the table, in order */
static const UInt32 KeyColumn = 0;
static const UInt32 ValueColumn = 1;
static const UInt32 RevisionColumn = 2;
static const UInt32 LastWriterColumn = 3;
static const UInt32 LastChangeColumn = 4;
static const UInt32 RemovedColumn = 5;

/** What a subscribed observer wants to hear */
static const UInt32 InterestAll = 1;
static const UInt32 InterestRows = 2;

/** A run of this many unused key indices ends a scan of the grid */
static const UInt32 ScanGapKeyCount = 4096;

/** How often the pending writes are checked for their timeout */
static const UInt32 WriteTimerIntervalMs = 100;

/*-------------------------------------------------------------------------*/

/** A key, a value and the bookkeeping of a key, copied from the grid while its lock is held */
class CRowCopy
{
    public:

    CORE::CVariant key;
    CORE::CVariant value;
    bool hasValue;
    UInt32 revision;
    UInt32 writer;
    UInt64 sendTime;

    CRowCopy( void ) : key(), value(), hasValue( false ), revision( 0 ), writer( 0 ), sendTime( 0 ) {}
};

typedef GUCEF::vector< CRowCopy >   TRowCopyVector;

/*-------------------------------------------------------------------------*/

/** Copies the key at the index with its value and bookkeeping; false for an index that has no key or a key that was never written */
static bool
CopyRow( const CDRNDataGrid& grid, UInt32 index, bool includeRemoved, CRowCopy& row )
{
    row.key = grid.GetKeyByIndex( index );
    if ( !row.key.IsInitialized() )
        return false;
    row.revision = grid.GetRevision( index );
    if ( 0 == row.revision )
        return false;
    row.hasValue = grid.GetValueByIndex( index, row.value );
    if ( !row.hasValue && !includeRemoved )
        return false;
    row.writer = grid.GetLastWriter( index );
    row.sendTime = grid.GetLastChangeSendTime( index );
    return true;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
RowLabel( const CORE::CVariant& key )
{
    return "key=" + key.AsString();
}

/*-------------------------------------------------------------------------*/

static void
AddRowResult( CORE::CDataTableWriteResult& result, const CORE::CVariant& key, CORE::TDataTableStatus status, UInt64 written, UInt64 deleted, const CORE::CString& message )
{
    CORE::CDataTableWriteResult::SPartitionResult row;
    row.path = RowLabel( key );
    row.status = status;
    row.rowsWritten = written;
    row.rowsDeleted = deleted;
    row.message = message;
    result.partitions.push_back( row );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNDataGridTable::KeyColumnName = "key";
const CORE::CString CDRNDataGridTable::ValueColumnName = "value";
const CORE::CString CDRNDataGridTable::RevisionColumnName = "revision";
const CORE::CString CDRNDataGridTable::LastWriterColumnName = "lastWriter";
const CORE::CString CDRNDataGridTable::LastChangeColumnName = "lastChange";
const CORE::CString CDRNDataGridTable::RemovedColumnName = "removed";

/*-------------------------------------------------------------------------*/

void
CDRNDataGridTable::CSessionWatcher::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* /* eventdata */ )
{GUCEF_TRACE;

    if ( CORE::CNotifier::DestructionEvent == eventid && notifier == m_session && GUCEF_NULL != m_table )
        m_table->OnSessionDestroyed();
}

/*-------------------------------------------------------------------------*/

CDRNDataGridTable::CDRNDataGridTable( CDRNSession& session                          ,
                                      CDRNDataGrid& grid                            ,
                                      const CDRNDataGridTableSettings& settings     ,
                                      CORE::PulseGeneratorPtr pulseGenerator        )
    : CORE::CIDataTable( pulseGenerator )
    , m_session( &session )
    , m_grid( &grid )
    , m_settings( settings )
    , m_definition()
    , m_schema()
    , m_scanDefinition()
    , m_scanSchema()
    , m_pool( &CORE::CCoreGlobal::Instance()->GetDataTableBufferPool() )
    , m_typed( false )
    , m_sessionWatcher()
    , m_asyncLink()
    , m_lock()
    , m_closed( false )
    , m_lastErrors()
    , m_scanTicks()
    , m_mismatchedValues( 0 )
    , m_mismatchedKeys( 0 )
    , m_snapshotCount( 0 )
    , m_cachedSnapshot()
    , m_cachedSnapshotTime( 0 )
    , m_subscriptionLock()
    , m_observers()
    , m_followingRows( false )
    , m_followingAnswers( false )
    , m_subscribed( false )
    , m_writeLock()
    , m_pendingLock()
    , m_pendingWrites()
    , m_writeTimer( GUCEF_NULL )
    , m_makingWriteTimer( false )
{GUCEF_TRACE;

    m_asyncLink = CORE::CDataTableAsyncLinkPtr( GUCEF_NEW CORE::CDataTableAsyncLink( this ) );
    BuildDefinition();
    m_sessionWatcher.m_table = this;
    m_sessionWatcher.m_session = &session;
    m_sessionWatcher.SubscribeTo( &session, CDRNSession::SessionClosedEvent );
}

/*-------------------------------------------------------------------------*/

CDRNDataGridTable::~CDRNDataGridTable()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    m_asyncLink->Detach();
    m_sessionWatcher.m_table = GUCEF_NULL;

    CDRNSession* session = GUCEF_NULL;
    CDRNDataGrid* grid = AcquireGrid( session );
    if ( GUCEF_NULL != grid )
        UnsubscribeFrom( grid );

    if ( GUCEF_NULL != m_writeTimer )
    {
        m_writeTimer->SetEnabled( false );
        UnsubscribeFrom( m_writeTimer );
        GUCEF_DELETE m_writeTimer;
        m_writeTimer = GUCEF_NULL;
    }
    EndPendingWrites( "NOT_FOUND: the table was destroyed before the write was answered" );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridTable::BuildDefinition( void )
{GUCEF_TRACE;

    CORE::TDataTableLogicalType keyType = CORE::DATATABLE_TYPE_UNSUPPORTED != m_settings.keyType ? m_settings.keyType : m_grid->GetSettings().keyType;
    CORE::TDataTableLogicalType valueType = CORE::DATATABLE_TYPE_UNSUPPORTED != m_settings.valueType ? m_settings.valueType : m_grid->GetSettings().valueType;
    m_typed = CORE::DATATABLE_TYPE_UNSUPPORTED != keyType && CORE::DATATABLE_TYPE_UNSUPPORTED != valueType;
    if ( !m_typed )
    {
        keyType = CORE::DATATABLE_TYPE_UTF8;
        valueType = CORE::DATATABLE_TYPE_UTF8;
    }

    CORE::CDataTableSchema schema;
    CORE::CDataTableColumnDef keyColumn( KeyColumnName, keyType, false );
    keyColumn.description = "The key of the grid";
    CORE::CDataTableColumnDef valueColumn( ValueColumnName, valueType, true );
    valueColumn.description = "The newest value of the key; null when it does not fit the type of the column";
    CORE::CDataTableColumnDef revisionColumn( RevisionColumnName, CORE::DATATABLE_TYPE_UINT32, true );
    revisionColumn.description = "How often the key changed";
    CORE::CDataTableColumnDef writerColumn( LastWriterColumnName, CORE::DATATABLE_TYPE_UINT32, true );
    writerColumn.description = "The peer id of the member that made the last change";
    CORE::CDataTableColumnDef changeColumn( LastChangeColumnName, CORE::DATATABLE_TYPE_UINT64, true );
    changeColumn.description = "The session time in microseconds at which the last change was sent";
    schema.AddColumn( keyColumn );
    schema.AddColumn( valueColumn );
    schema.AddColumn( revisionColumn );
    schema.AddColumn( writerColumn );
    schema.AddColumn( changeColumn );
    if ( m_settings.includeRemoved )
    {
        CORE::CDataTableColumnDef removedColumn( RemovedColumnName, CORE::DATATABLE_TYPE_BOOL, true );
        removedColumn.description = "The key was removed";
        schema.AddColumn( removedColumn );
    }

    // the scan machinery stores the copy of the grid without key: rows stay in the order of the key indices
    m_scanDefinition.name = "gridcopy";
    m_scanDefinition.schema = schema;
    m_scanSchema = m_scanDefinition.CreateSchemaPtr();

    CORE::CString::StringVector primaryKey;
    primaryKey.push_back( KeyColumnName );
    schema.SetPrimaryKey( primaryKey );
    m_definition.name = m_grid->GetName();
    m_definition.schema = schema;
    m_definition.codecType = "drn";
    m_definition.backendOptions.Set( "backend", "drn" );
    m_definition.backendOptions.Set( "session", m_session->GetName() );
    m_definition.backendOptions.Set( "owner", CORE::UInt32ToString( m_grid->GetOwnerPeerId() ) );
    m_definition.backendOptions.Set( "authority", CDRNDataGridSettings::AuthorityToString( m_grid->GetSettings().authority ) );
    m_schema = m_definition.CreateSchemaPtr();
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDataGridTable::Fail( CORE::TDataTableStatus status, const CORE::CString& message ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_lastErrors[ MT::GetCurrentTaskID() ] = message;
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDataGridTable::Succeed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TLastErrorMap::iterator i = m_lastErrors.find( MT::GetCurrentTaskID() );
    if ( i != m_lastErrors.end() )
        m_lastErrors.erase( i );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNDataGridTable::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TLastErrorMap::const_iterator i = m_lastErrors.find( MT::GetCurrentTaskID() );
    return i != m_lastErrors.end() ? (*i).second : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CDRNDataGrid*
CDRNDataGridTable::AcquireGrid( CDRNSession*& session ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_closed || GUCEF_NULL == m_grid || GUCEF_NULL == m_session || m_grid->IsRemoved() )
        return GUCEF_NULL;
    session = m_session;
    return m_grid;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridTable::OnSessionDestroyed( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_closed = true;
        m_grid = GUCEF_NULL;
        m_session = GUCEF_NULL;
        m_cachedSnapshot.Unlink();
    }
    {
        MT::CScopeMutex lock( m_subscriptionLock );
        m_followingRows = false;
        m_followingAnswers = false;
        m_subscribed = false;
    }
    EndPendingWrites( "NOT_FOUND: the session ended before the write was answered" );
}

/*-------------------------------------------------------------------------*/

const CDRNDataGridTableSettings&
CDRNDataGridTable::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGridTable::IsTyped( void ) const
{GUCEF_TRACE;

    return m_typed;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGridTable::IsClosed( void ) const
{GUCEF_TRACE;

    CDRNSession* session = GUCEF_NULL;
    return GUCEF_NULL == AcquireGrid( session );
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNDataGridTable::GetMismatchedValueCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_mismatchedValues;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNDataGridTable::GetMismatchedKeyCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_mismatchedKeys;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNDataGridTable::GetSnapshotCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_snapshotCount;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGridTable::GetLastScanTicks( UInt32& firstTick, UInt32& lastTick ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TScanTicksMap::const_iterator i = m_scanTicks.find( MT::GetCurrentTaskID() );
    if ( i == m_scanTicks.end() )
        return false;
    firstTick = (*i).second.firstTick;
    lastTick = (*i).second.lastTick;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataGridTable::GetGridSubscriptionCount( void )
{GUCEF_TRACE;

    CDRNSession* session = GUCEF_NULL;
    CDRNDataGrid* grid = AcquireGrid( session );
    return GUCEF_NULL == grid ? 0 : grid->GetSubscriptionCountForObserver( &AsObserver() );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNDataGridTable::GetName( void ) const
{GUCEF_TRACE;

    return m_definition.name;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataTableDefinition&
CDRNDataGridTable::GetDefinition( void ) const
{GUCEF_TRACE;

    return m_definition;
}

/*-------------------------------------------------------------------------*/

CORE::CDataTableSchemaPtr
CDRNDataGridTable::GetSchema( void ) const
{GUCEF_TRACE;

    return m_schema;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGridTable::IsReadOnly( void ) const
{GUCEF_TRACE;

    CDRNSession* session = GUCEF_NULL;
    CDRNDataGrid* grid = AcquireGrid( session );
    if ( GUCEF_NULL == grid || !m_typed || m_settings.readOnly )
        return true;
    return !grid->IsOrigin() && DRN_AUTHORITY_OWNER == grid->GetSettings().authority;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNDataGridTable::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNDataGridTable";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGridTable::GetKeyLookup( const CORE::CDataTableQuery& query, CORE::CVariant::VariantVector& keys ) const
{GUCEF_TRACE;

    // a predicate on the key that names the keys looked for: the other predicates are applied by the scan to what was found
    const CORE::CDataTableQuery::TPredicateVector& predicates = query.GetPredicates();
    for ( UInt32 i=0; i<(UInt32) predicates.size(); ++i )
    {
        const CORE::CDataTablePredicate& predicate = predicates[ i ];
        if ( predicate.columnName != KeyColumnName )
            continue;
        if ( CORE::DATATABLE_PREDICATE_EQUALS == predicate.op )
        {
            keys.clear();
            keys.push_back( predicate.low );
            return true;
        }
        if ( CORE::DATATABLE_PREDICATE_IN == predicate.op )
        {
            keys = predicate.values;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGridTable::AppendRow( CORE::CDataTableBatchBuilder& builder  ,
                              const CORE::CVariant& key              ,
                              bool hasValue                          ,
                              const CORE::CVariant& value            ,
                              UInt64& mismatchedValues               ,
                              UInt64& mismatchedKeys                 ) const
{GUCEF_TRACE;

    // a key that does not fit hides its row: the key is the primary key and cannot be null
    CORE::CDataTableColumnBuilder& keyBuilder = builder.GetColumn( KeyColumn );
    if ( m_typed ? !keyBuilder.Append( key ) : !keyBuilder.AppendString( key.AsString() ) )
    {
        ++mismatchedKeys;
        return false;
    }

    CORE::CDataTableColumnBuilder& valueBuilder = builder.GetColumn( ValueColumn );
    if ( !hasValue )
    {
        valueBuilder.AppendNull();
    }
    else
    if ( m_typed )
    {
        if ( !valueBuilder.Append( value ) )
        {
            valueBuilder.AppendNull();
            ++mismatchedValues;
        }
    }
    else
    {
        valueBuilder.AppendString( value.AsString() );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDataGridTable::FlushBatch( CORE::CDataTableBatchBuilder& builder, CORE::CMemoryDataTable& snapshot )
{GUCEF_TRACE;

    if ( 0 == builder.GetRowCount() )
        return CORE::DATATABLE_STATUS_OK;
    CORE::CDataTableBatchPtr batch;
    CORE::CString error;
    CORE::TDataTableStatus status = builder.Build( batch, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, "Building the rows of the copy of the grid failed: " + error );
    CORE::CDataTableWriteResult written;
    status = snapshot.Write( *batch, CORE::DATATABLE_WRITE_INSERT, written );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, "Storing the copy of the grid failed: " + snapshot.GetLastErrorMessage() );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDataGridTable::TakeSnapshot( const CORE::CDataTableQuery& query, CORE::CMemoryDataTablePtr& snapshot )
{GUCEF_TRACE;

    snapshot.Unlink();
    CDRNSession* session = GUCEF_NULL;
    CDRNDataGrid* grid = AcquireGrid( session );
    if ( GUCEF_NULL == grid )
        return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "The grid of table \"" + m_definition.name + "\" is gone: the item was removed or the session ended" );

    CORE::CVariant::VariantVector lookupKeys;
    const bool lookup = m_typed && GetKeyLookup( query, lookupKeys );
    if ( !lookup && 0 != m_settings.snapshotCacheMs )
    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_cachedSnapshot.IsNULL() && ( CDRNClock::NowInMicroseconds() - m_cachedSnapshotTime ) < (UInt64) m_settings.snapshotCacheMs * 1000 )
        {
            snapshot = m_cachedSnapshot;
            return CORE::DATATABLE_STATUS_OK;
        }
    }

    CORE::CMemoryDataTablePtr copy( GUCEF_NEW CORE::CMemoryDataTable( m_scanDefinition, m_pool, GetPulseGenerator() ) );
    if ( !copy->IsValid() )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "The copy of the grid cannot be stored: " + copy->GetLastErrorMessage() );

    CORE::CDataTableBatchBuilder builder( m_scanSchema, *m_pool );
    MT::CMutex& nodeLock = session->GetNode().GetLock();
    const UInt32 chunkSize = 0 == m_settings.scanChunkKeyCount ? GUCEF_UINT32MAX : m_settings.scanChunkKeyCount;
    UInt64 mismatchedValues = 0;
    UInt64 mismatchedKeys = 0;
    UInt32 firstTick = 0;
    UInt32 lastTick = 0;
    bool haveTick = false;
    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    TRowCopyVector rows;
    UInt32 index = 0;
    UInt32 emptyRun = 0;
    UInt32 lookupPosition = 0;
    bool more = true;
    while ( more && CORE::DATATABLE_STATUS_OK == status )
    {
        // the lock is held while the keys of one chunk are copied, and not longer
        UInt32 copied = 0;
        {
            MT::CScopeMutex lock( nodeLock );
            const UInt32 tick = grid->GetUpdateTick();
            if ( !haveTick )
            {
                firstTick = tick;
                haveTick = true;
            }
            lastTick = tick;

            if ( lookup )
            {
                for ( ; lookupPosition < (UInt32) lookupKeys.size() && copied < chunkSize; ++lookupPosition )
                {
                    const UInt32 keyIndex = grid->GetKeyIndex( lookupKeys[ lookupPosition ] );
                    if ( CDRNDataGrid::InvalidKeyIndex == keyIndex )
                        continue;
                    if ( copied == (UInt32) rows.size() )
                        rows.push_back( CRowCopy() );
                    if ( CopyRow( *grid, keyIndex, m_settings.includeRemoved, rows[ copied ] ) )
                        ++copied;
                }
                more = lookupPosition < (UInt32) lookupKeys.size();
            }
            else
            {
                UInt32 visited = 0;
                while ( visited < chunkSize && emptyRun < ScanGapKeyCount && GUCEF_UINT32MAX != index )
                {
                    if ( copied == (UInt32) rows.size() )
                        rows.push_back( CRowCopy() );
                    CRowCopy& row = rows[ copied ];
                    if ( CopyRow( *grid, index, m_settings.includeRemoved, row ) )
                    {
                        ++copied;
                        emptyRun = 0;
                    }
                    else
                    if ( row.key.IsInitialized() )
                        emptyRun = 0;           // a key that was never written or was removed is not a gap
                    else
                        ++emptyRun;
                    ++visited;
                    ++index;
                }
                more = emptyRun < ScanGapKeyCount && GUCEF_UINT32MAX != index;
            }
        }

        // the game thread gets its turn before the next chunk is taken
        if ( more )
            MT::ThreadDelay( 0 );

        // the rows become columns without the lock
        for ( UInt32 r=0; r<copied; ++r )
        {
            const CRowCopy& row = rows[ r ];
            if ( !AppendRow( builder, row.key, row.hasValue, row.value, mismatchedValues, mismatchedKeys ) )
                continue;
            builder.GetColumn( RevisionColumn ).AppendUInt64( row.revision );
            builder.GetColumn( LastWriterColumn ).AppendUInt64( row.writer );
            builder.GetColumn( LastChangeColumn ).AppendUInt64( row.sendTime );
            if ( m_settings.includeRemoved )
                builder.GetColumn( RemovedColumn ).AppendBool( !row.hasValue );
        }
        status = FlushBatch( builder, *copy );
    }
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    {
        MT::CScopeMutex lock( m_lock );
        m_mismatchedValues += mismatchedValues;
        m_mismatchedKeys += mismatchedKeys;
        ++m_snapshotCount;
        CScanTicks& ticks = m_scanTicks[ MT::GetCurrentTaskID() ];
        ticks.firstTick = firstTick;
        ticks.lastTick = lastTick;
        if ( !lookup && 0 != m_settings.snapshotCacheMs )
        {
            m_cachedSnapshot = copy;
            m_cachedSnapshotTime = CDRNClock::NowInMicroseconds();
        }
    }
    snapshot = copy;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDataGridTable::Scan( const CORE::CDataTableQuery& query, CORE::CDataTableScanPtr& scan )
{GUCEF_TRACE;

    scan.Unlink();
    CORE::CMemoryDataTablePtr snapshot;
    CORE::TDataTableStatus status = TakeSnapshot( query, snapshot );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    status = snapshot->Scan( query, scan );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, snapshot->GetLastErrorMessage() );
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNDataGridTable::ScanAsync( const CORE::CDataTableQuery& query, CORE::CDataTableScanPtr& scan, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    // the copy of the grid is taken now, so the scan has the state of the moment of the call; the rest runs on the pool
    scan.Unlink();
    CORE::CMemoryDataTablePtr snapshot;
    if ( CORE::DATATABLE_STATUS_OK != TakeSnapshot( query, snapshot ) )
        return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    return snapshot->ScanAsync( query, scan, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDataGridTable::CheckWritable( CORE::TDataTableWriteMode mode, CDRNDataGrid*& grid, CDRNSession*& session ) const
{GUCEF_TRACE;

    grid = AcquireGrid( session );
    if ( GUCEF_NULL == grid )
        return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "The grid of table \"" + m_definition.name + "\" is gone: the item was removed or the session ended" );
    if ( CORE::DATATABLE_WRITE_REPLACE_PARTITIONS == mode )
        return Fail( CORE::DATATABLE_STATUS_UNSUPPORTED, "A grid has no partitions to replace: use UPSERT or INSERT" );
    if ( m_settings.readOnly )
        return Fail( CORE::DATATABLE_STATUS_READ_ONLY, "Table \"" + m_definition.name + "\" is read only: it was opened read only" );
    if ( !m_typed )
        return Fail( CORE::DATATABLE_STATUS_READ_ONLY, "Table \"" + m_definition.name + "\" is read only: the grid declares no key and value type" );
    if ( !grid->IsOrigin() && DRN_AUTHORITY_OWNER == grid->GetSettings().authority )
        return Fail( CORE::DATATABLE_STATUS_READ_ONLY, "Table \"" + m_definition.name + "\" is read only: only the owner of the grid writes it" );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDataGridTable::ExtractChanges( const CORE::CDataTableBatch& rows, TDRNRowChangeVector& changes )
{GUCEF_TRACE;

    changes.clear();
    CORE::CString error;
    CORE::CDataTableBatchPtr normalized;
    CORE::TDataTableStatus status = CORE::CDataTableMerge::Normalize( m_schema, rows, *m_pool, normalized, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, error );

    const UInt32* selection = normalized->GetSelection();
    const UInt32 selected = normalized->GetSelectedRowCount();
    changes.reserve( selected );
    for ( UInt32 i=0; i<selected; ++i )
    {
        const UInt32 row = GUCEF_NULL == selection ? i : selection[ i ];
        CDRNRowChange change;
        normalized->GetValue( row, KeyColumn, change.key, false );
        normalized->GetValue( row, ValueColumn, change.value, false );
        changes.push_back( change );
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDataGridTable::ApplyChanges( CDRNDataGrid& grid                      ,
                                 const TDRNRowChangeVector& changes      ,
                                 CORE::TDataTableWriteMode mode          ,
                                 CORE::CDataTableWriteResult& result     ,
                                 GUCEF::vector< UInt32 >& proposedRows   )
{GUCEF_TRACE;

    // a replica that may write sends proposals: the rows are reported as sent, the answers come later
    const bool proposing = !grid.IsOrigin();
    CORE::TDataTableStatus overall = CORE::DATATABLE_STATUS_OK;
    result.partitions.reserve( changes.size() );
    for ( UInt32 i=0; i<(UInt32) changes.size(); ++i )
    {
        const CDRNRowChange& change = changes[ i ];
        CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
        CORE::CString message;
        if ( change.remove )
        {
            if ( !grid.RemoveKey( change.key ) )
            {
                status = CORE::DATATABLE_STATUS_FAILED;
                message = "the grid refused the removal";
            }
        }
        else
        if ( CORE::DATATABLE_WRITE_INSERT == mode && grid.HasKey( change.key ) )
        {
            status = CORE::DATATABLE_STATUS_CONFLICT;
            message = "the key exists";
        }
        else
        if ( change.value.IsNULL() || !change.value.IsInitialized() )
        {
            status = CORE::DATATABLE_STATUS_TYPE_MISMATCH;
            message = "a grid value cannot be null, delete the row instead";
        }
        else
        if ( !grid.SetValue( change.key, change.value ) )
        {
            status = CORE::DATATABLE_STATUS_FAILED;
            message = "the grid refused the write";
        }

        if ( CORE::DATATABLE_STATUS_OK == status )
        {
            if ( proposing )
            {
                proposedRows.push_back( i );
                message = "proposed";
            }
            else
            {
                message = "applied";
            }
            AddRowResult( result, change.key, status, change.remove ? 0 : 1, change.remove ? 1 : 0, message );
        }
        else
        {
            AddRowResult( result, change.key, status, 0, 0, message );
            if ( CORE::DATATABLE_STATUS_OK == overall )
                overall = status;
        }
    }
    return overall;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDataGridTable::Write( const CORE::CDataTableBatch& rows, CORE::TDataTableWriteMode mode, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    CDRNDataGrid* grid = GUCEF_NULL;
    CDRNSession* session = GUCEF_NULL;
    CORE::TDataTableStatus status = CheckWritable( mode, grid, session );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    TDRNRowChangeVector changes;
    status = ExtractChanges( rows, changes );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    MT::CScopeMutex writeLock( m_writeLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    GUCEF::vector< UInt32 > proposedRows;
    status = ApplyChanges( *grid, changes, mode, result, proposedRows );
    if ( CORE::DATATABLE_STATUS_OK != status )
    {
        CORE::CString message = "A row was not written";
        for ( UInt32 i=0; i<(UInt32) result.partitions.size(); ++i )
        {
            if ( CORE::DATATABLE_STATUS_OK != result.partitions[ i ].status )
            {
                message = "Row " + result.partitions[ i ].path + " was not written: " + result.partitions[ i ].message;
                break;
            }
        }
        return Fail( status, message );
    }
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNDataGridTable::QueueOperation( CORE::CDataTableAsyncOperationData& data, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    data.table = m_asyncLink;
    return CORE::CDataTableAsyncOperation::Queue( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNDataGridTable::WriteAsync( const CORE::CDataTableBatchPtr& rows, CORE::TDataTableWriteMode mode, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CDRNDataGrid* grid = GUCEF_NULL;
    CDRNSession* session = GUCEF_NULL;
    if ( CORE::DATATABLE_STATUS_OK != CheckWritable( mode, grid, session ) || rows.IsNULL() )
        return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_TASKDATA_INVALID );

    if ( grid->IsOrigin() )
    {
        CORE::CDataTableAsyncOperationData data;
        data.operation = CORE::CDataTableAsyncOperationData::OPERATION_WRITE;
        data.rows = rows;
        data.writeMode = mode;
        return QueueOperation( data, threadPool );
    }

    TDRNRowChangeVector changes;
    if ( CORE::DATATABLE_STATUS_OK != ExtractChanges( *rows, changes ) )
        return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    return EnqueueWrite( CDRNDataGridPendingWritePtr( GUCEF_NEW CDRNDataGridPendingWrite( changes, mode, m_settings.writeTimeoutMs ) ) );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDataGridTable::ProposeDeleteRows( const CORE::CDataTableQuery& predicates, CDRNDataGridPendingWritePtr& pending )
{GUCEF_TRACE;

    // the keys of the rows that match, as the table shows them now
    CORE::CDataTableQuery keysQuery;
    keysQuery.Select( KeyColumnName );
    const CORE::CDataTableQuery::TPredicateVector& predicateList = predicates.GetPredicates();
    for ( UInt32 i=0; i<(UInt32) predicateList.size(); ++i )
        keysQuery.Where( predicateList[ i ] );

    CORE::CDataTableScanPtr scan;
    CORE::TDataTableStatus status = Scan( keysQuery, scan );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    TDRNRowChangeVector changes;
    CORE::CDataTableBatchPtr batch;
    while ( CORE::DATATABLE_STATUS_OK == ( status = scan->NextBatch( batch ) ) )
    {
        const UInt32* selection = batch->GetSelection();
        const UInt32 selected = batch->GetSelectedRowCount();
        for ( UInt32 i=0; i<selected; ++i )
        {
            CDRNRowChange change;
            change.remove = true;
            batch->GetValue( GUCEF_NULL == selection ? i : selection[ i ], 0, change.key, false );
            changes.push_back( change );
        }
    }
    if ( CORE::DATATABLE_STATUS_END != status )
        return Fail( status, scan->GetLastErrorMessage() );
    pending = CDRNDataGridPendingWritePtr( GUCEF_NEW CDRNDataGridPendingWrite( changes, CORE::DATATABLE_WRITE_UPSERT, m_settings.writeTimeoutMs ) );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDataGridTable::DeleteRows( const CORE::CDataTableQuery& predicates, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    CDRNDataGrid* grid = GUCEF_NULL;
    CDRNSession* session = GUCEF_NULL;
    CORE::TDataTableStatus status = CheckWritable( CORE::DATATABLE_WRITE_UPSERT, grid, session );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    CDRNDataGridPendingWritePtr removal;
    status = ProposeDeleteRows( predicates, removal );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    MT::CScopeMutex writeLock( m_writeLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    GUCEF::vector< UInt32 > proposedRows;
    status = ApplyChanges( *grid, removal->GetChanges(), CORE::DATATABLE_WRITE_UPSERT, result, proposedRows );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, "A row was not deleted" );
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNDataGridTable::DeleteRowsAsync( const CORE::CDataTableQuery& predicates, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CDRNDataGrid* grid = GUCEF_NULL;
    CDRNSession* session = GUCEF_NULL;
    if ( CORE::DATATABLE_STATUS_OK != CheckWritable( CORE::DATATABLE_WRITE_UPSERT, grid, session ) )
        return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_TASKDATA_INVALID );

    if ( grid->IsOrigin() )
    {
        CORE::CDataTableAsyncOperationData data;
        data.operation = CORE::CDataTableAsyncOperationData::OPERATION_DELETE_ROWS;
        data.query = predicates;
        return QueueOperation( data, threadPool );
    }

    CDRNDataGridPendingWritePtr removal;
    if ( CORE::DATATABLE_STATUS_OK != ProposeDeleteRows( predicates, removal ) )
        return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    return EnqueueWrite( removal );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDataGridTable::DeletePartitions( const CORE::CDataTableQuery& /* partitionFilter */, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Fail( CORE::DATATABLE_STATUS_UNSUPPORTED, "A grid has no partitions" );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNDataGridTable::DeletePartitionsAsync( const CORE::CDataTableQuery& partitionFilter, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CORE::CDataTableAsyncOperationData data;
    data.operation = CORE::CDataTableAsyncOperationData::OPERATION_DELETE_PARTITIONS;
    data.query = partitionFilter;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDataGridTable::ListPartitions( const CORE::CDataTableQuery& /* partitionFilter */, bool /* withFileStats */, CORE::CDataTablePartitionInfo::TPartitionInfoVector& partitions )
{GUCEF_TRACE;

    partitions.clear();
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNDataGridTable::ListPartitionsAsync( const CORE::CDataTableQuery& partitionFilter, bool withFileStats, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CORE::CDataTableAsyncOperationData data;
    data.operation = CORE::CDataTableAsyncOperationData::OPERATION_LIST_PARTITIONS;
    data.query = partitionFilter;
    data.flag = withFileStats;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDataGridTable::Compact( const CORE::CDataTableQuery& /* partitionFilter */, bool /* force */, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Fail( CORE::DATATABLE_STATUS_UNSUPPORTED, "A grid has nothing to compact" );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNDataGridTable::CompactAsync( const CORE::CDataTableQuery& partitionFilter, bool force, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CORE::CDataTableAsyncOperationData data;
    data.operation = CORE::CDataTableAsyncOperationData::OPERATION_COMPACT;
    data.query = partitionFilter;
    data.flag = force;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridTable::Refresh( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_cachedSnapshot.Unlink();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNDataGridTable::EnqueueWrite( const CDRNDataGridPendingWritePtr& pending )
{GUCEF_TRACE;

    bool first = false;
    {
        MT::CScopeMutex lock( m_pendingLock );
        m_pendingWrites.push_back( pending );
        first = 1 == m_pendingWrites.size();
    }
    CORE::CFutureResult future = pending->GetFuture();
    EnsureWriteTimer();
    if ( first )
        StartPendingWrites();
    return future;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridTable::EnsureWriteTimer( void )
{GUCEF_TRACE;

    // Made once, by the first write that is enqueued, without a lock of the table, and enabled from then on: its pulses
    // take the pending lock (OnWriteTimer()), and the timer takes its notification lock when it is enabled, disabled or
    // subscribed to, so none of that happens under the pending lock, nor from within the events of the grid or of the
    // timer (gucefDRN design 9, lock order). A pulse without an overdue write finds nothing to do.
    if ( 0 == m_settings.writeTimeoutMs )
        return;
    {
        MT::CScopeMutex lock( m_pendingLock );
        if ( GUCEF_NULL != m_writeTimer || m_makingWriteTimer )
            return;
        m_makingWriteTimer = true;
    }
    CORE::CTimer* timer = GUCEF_NEW CORE::CTimer( GetPulseGenerator(), WriteTimerIntervalMs );
    TEventCallback callback( this, &CDRNDataGridTable::OnWriteTimer );
    SubscribeTo( timer, CORE::CTimer::TimerUpdateEvent, callback );
    timer->SetEnabled( true );

    MT::CScopeMutex lock( m_pendingLock );
    m_writeTimer = timer;
    m_makingWriteTimer = false;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridTable::StartPendingWrites( void )
{GUCEF_TRACE;

    // the write at the head sends its proposals; writes without a proposal in flight end at once and the next one starts
    for ( ;; )
    {
        CDRNDataGridPendingWritePtr head;
        {
            MT::CScopeMutex lock( m_pendingLock );
            if ( m_pendingWrites.empty() || m_pendingWrites.front()->HasStarted() )
                break;
            head = m_pendingWrites.front();
        }

        CDRNSession* session = GUCEF_NULL;
        CDRNDataGrid* grid = AcquireGrid( session );
        if ( GUCEF_NULL == grid )
        {
            EndPendingWrites( "NOT_FOUND: the grid is gone, the item was removed or the session ended" );
            break;
        }

        CORE::CDataTableWriteResult sent;
        GUCEF::vector< UInt32 > proposedRows;
        ApplyChanges( *grid, head->GetChanges(), head->GetWriteMode(), sent, proposedRows );
        head->SetSentResult( sent );
        head->SetProposedRows( proposedRows );
        head->Start( CDRNClock::NowInMicroseconds() );

        if ( head->IsComplete() )
        {
            {
                MT::CScopeMutex lock( m_pendingLock );
                if ( !m_pendingWrites.empty() && m_pendingWrites.front() == head )
                    m_pendingWrites.erase( m_pendingWrites.begin() );
            }
            head->Complete();
            continue;
        }

        // the write timer looks for overdue writes from now on (EnsureWriteTimer())
        break;
    }
    UpdateGridSubscriptions();
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridTable::EndPendingWrites( const CORE::CString& statusText )
{GUCEF_TRACE;

    TPendingWriteVector ended;
    {
        MT::CScopeMutex lock( m_pendingLock );
        ended = m_pendingWrites;
        m_pendingWrites.clear();
    }
    for ( UInt32 i=0; i<(UInt32) ended.size(); ++i )
        ended[ i ]->Fail( statusText );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridTable::OnProposeResult( CORE::CNotifier* /* notifier */, const CORE::CEvent& /* eventid */, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CDRNProposeResultEventData* data = static_cast< CDRNProposeResultEventData* >( eventData );
    if ( GUCEF_NULL == data )
        return;
    const CDRNProposeResult& answer = data->GetData();

    CDRNDataGridPendingWritePtr head;
    {
        MT::CScopeMutex lock( m_pendingLock );
        if ( m_pendingWrites.empty() || !m_pendingWrites.front()->HasStarted() )
            return;
        head = m_pendingWrites.front();
        head->AddAnswer( answer.proposalId, answer.accepted, answer.voteOutcome, answer.reason );
        if ( !head->IsComplete() )
            return;
        m_pendingWrites.erase( m_pendingWrites.begin() );
    }
    head->Complete();
    StartPendingWrites();
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridTable::OnWriteTimer( CORE::CNotifier* /* notifier */, const CORE::CEvent& /* eventid */, CORE::CICloneable* /* eventData */ )
{GUCEF_TRACE;

    CDRNDataGridPendingWritePtr head;
    {
        MT::CScopeMutex lock( m_pendingLock );
        if ( m_pendingWrites.empty() )
            return;
        if ( !m_pendingWrites.front()->IsOverdue( CDRNClock::NowInMicroseconds() ) )
            return;
        head = m_pendingWrites.front();
        m_pendingWrites.erase( m_pendingWrites.begin() );
    }
    head->Fail( "FAILED: the publisher did not answer for every proposal within " + CORE::UInt32ToString( m_settings.writeTimeoutMs ) + " ms" );
    StartPendingWrites();
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGridTable::Subscribe( CORE::CObserver* observer )
{GUCEF_TRACE;

    const bool subscribed = CORE::CIDataTable::Subscribe( observer );
    if ( subscribed )
        NoteInterest( observer, InterestAll );
    return subscribed;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGridTable::Subscribe( CORE::CObserver* observer, const CORE::CEvent& eventid, CORE::CIEventHandlerFunctorBase* callback )
{GUCEF_TRACE;

    const bool subscribed = CORE::CIDataTable::Subscribe( observer, eventid, callback );
    if ( subscribed && RowsChangedEvent == eventid )
        NoteInterest( observer, InterestRows );
    return subscribed;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridTable::Unsubscribe( CORE::CObserver* observer )
{GUCEF_TRACE;

    CORE::CIDataTable::Unsubscribe( observer );
    DropInterest( observer, InterestAll | InterestRows );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridTable::Unsubscribe( CORE::CObserver* observer, const CORE::CEvent& eventid )
{GUCEF_TRACE;

    CORE::CIDataTable::Unsubscribe( observer, eventid );
    if ( RowsChangedEvent == eventid )
        DropInterest( observer, InterestRows );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridTable::UnsubscribeAllFromNotifier( void )
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
CDRNDataGridTable::OnObserverDestruction( CORE::CObserver* observer )
{GUCEF_TRACE;

    CORE::CIDataTable::OnObserverDestruction( observer );
    DropInterest( observer, InterestAll | InterestRows );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridTable::NoteInterest( CORE::CObserver* observer, UInt32 interest )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_subscriptionLock );
        m_observers[ observer ] |= interest;
    }
    UpdateGridSubscriptions();
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridTable::DropInterest( CORE::CObserver* observer, UInt32 interest )
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
CDRNDataGridTable::UpdateGridSubscriptions( void )
{GUCEF_TRACE;

    // The subscriptions at the grid change under the notification lock of the grid, then the subscription lock: the grid
    // raises its events under its notification lock and their handlers take the locks of the table (OnProposeResult(),
    // which may come here again), so the order is the same everywhere (gucefDRN design 9, lock order)
    CDRNSession* session = GUCEF_NULL;
    CDRNDataGrid* grid = AcquireGrid( session );
    CORE::CNotifierScopeLock gridLock( grid );
    MT::CScopeMutex lock( m_subscriptionLock );
    bool wantAnswers = false;
    {
        MT::CScopeMutex pendingLock( m_pendingLock );
        wantAnswers = !m_pendingWrites.empty();
    }
    const bool wantRows = !m_observers.empty();

    if ( GUCEF_NULL == grid )
    {
        m_followingRows = false;
        m_followingAnswers = false;
        m_subscribed = false;
        return;
    }

    if ( !wantRows && !wantAnswers )
    {
        // all the way out: no observer of this table is left at the grid
        if ( m_subscribed )
            UnsubscribeFrom( grid );
        m_followingRows = false;
        m_followingAnswers = false;
        m_subscribed = false;
        return;
    }

    if ( wantRows && !m_followingRows )
    {
        TEventCallback updatedCallback( this, &CDRNDataGridTable::OnGridUpdated );
        TEventCallback changedCallback( this, &CDRNDataGridTable::OnGridKeyChanged );
        SubscribeTo( grid, CDRNDataGrid::GridUpdatedEvent, updatedCallback );
        SubscribeTo( grid, CDRNDataGrid::GridKeyChangedEvent, changedCallback );
        m_followingRows = true;
        m_subscribed = true;
    }
    else
    if ( !wantRows && m_followingRows )
    {
        UnsubscribeFrom( grid, CDRNDataGrid::GridUpdatedEvent );
        UnsubscribeFrom( grid, CDRNDataGrid::GridKeyChangedEvent );
        m_followingRows = false;
    }

    if ( wantAnswers && !m_followingAnswers )
    {
        TEventCallback answerCallback( this, &CDRNDataGridTable::OnProposeResult );
        SubscribeTo( grid, CDRNDataGrid::ProposeResultEvent, answerCallback );
        m_followingAnswers = true;
        m_subscribed = true;
    }
    else
    if ( !wantAnswers && m_followingAnswers )
    {
        UnsubscribeFrom( grid, CDRNDataGrid::ProposeResultEvent );
        m_followingAnswers = false;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridTable::OnGridUpdated( CORE::CNotifier* /* notifier */, const CORE::CEvent& /* eventid */, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CDRNGridUpdatedEventData* data = static_cast< CDRNGridUpdatedEventData* >( eventData );
    CDRNSession* session = GUCEF_NULL;
    CDRNDataGrid* grid = AcquireGrid( session );
    if ( GUCEF_NULL == data || GUCEF_NULL == grid )
        return;

    // the keys of the changed indices; a row appears or goes with the rows of the table that follow
    CORE::TDataTableRowsChangedEventData changed;
    const TDRNKeyIndexVector& indices = data->GetData();
    changed.GetData().reserve( indices.size() );
    for ( UInt32 i=0; i<(UInt32) indices.size(); ++i )
        changed.GetData().push_back( grid->GetKeyByIndex( indices[ i ] ) );
    NotifyObserversFromThread( RowsChangedEvent, &changed );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridTable::OnGridKeyChanged( CORE::CNotifier* /* notifier */, const CORE::CEvent& /* eventid */, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::TCloneableUInt32* data = static_cast< CORE::TCloneableUInt32* >( eventData );
    CDRNSession* session = GUCEF_NULL;
    CDRNDataGrid* grid = AcquireGrid( session );
    if ( GUCEF_NULL == data || GUCEF_NULL == grid )
        return;

    CORE::TDataTableRowsChangedEventData changed;
    changed.GetData().push_back( grid->GetKeyByIndex( data->GetData() ) );
    NotifyObserversFromThread( RowsChangedEvent, &changed );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
