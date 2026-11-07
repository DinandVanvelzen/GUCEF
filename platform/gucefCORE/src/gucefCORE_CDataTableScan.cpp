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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATATABLEPRUNER_H
#include "gucefCORE_CDataTablePruner.h"
#define GUCEF_CORE_CDATATABLEPRUNER_H
#endif /* GUCEF_CORE_CDATATABLEPRUNER_H ? */

#ifndef GUCEF_CORE_CDATATABLEFILTER_H
#include "gucefCORE_CDataTableFilter.h"
#define GUCEF_CORE_CDATATABLEFILTER_H
#endif /* GUCEF_CORE_CDATATABLEFILTER_H ? */

#include "gucefCORE_CDataTableScanTask.h"
#include "gucefCORE_CDataTableScan.h"

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

const CEvent CDataTableScan::ScanBatchesAvailableEvent = "GUCEF::CORE::CDataTableScan::ScanBatchesAvailableEvent";
const CEvent CDataTableScan::ScanFinishedEvent = "GUCEF::CORE::CDataTableScan::ScanFinishedEvent";
const CEvent CDataTableScan::ScanFailedEvent = "GUCEF::CORE::CDataTableScan::ScanFailedEvent";

const CString CDataTableScanTask::TaskType = "DataTableScanTask";

/** The pool async scans use when none is given */
static const char* DataTablesThreadPoolName = "DataTables";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 NoSlot = GUCEF_UINT32MAX;

/** How long a task without a credit waits for one before it parks its row group and ends */
const UInt32 CreditLingerMs = 1;

/** How long a task without work waits for the files other tasks list or open before it ends */
const UInt32 WorkLingerMs = 1;

/** How a delivered column is made from what the reader gave */
enum EColumnAdapt
{
    COLUMNADAPT_NONE = 0            ,   /**< the reader's column as it is */
    COLUMNADAPT_NULLS               ,   /**< the file lacks the column: all null */
    COLUMNADAPT_INT32_TO_INT64      ,
    COLUMNADAPT_FLOAT32_TO_FLOAT64  ,
    COLUMNADAPT_DECIMAL_RESCALE         /**< a file decimal of a lower scale */
};

struct SColumnPlan
{
    UInt32 readerColumn;                /**< the column of the reader's batch, GUCEF_UINT32MAX for COLUMNADAPT_NULLS */
    EColumnAdapt adapt;
    Int64 rescaleFactor;                /**< COLUMNADAPT_DECIMAL_RESCALE: 10 ^ ( table scale - file scale ) */

    SColumnPlan( void ) : readerColumn( GUCEF_UINT32MAX ), adapt( COLUMNADAPT_NONE ), rescaleFactor( 1 ) {}
};

enum EItemState
{
    ITEMSTATE_PENDING = 0   ,   /**< not started */
    ITEMSTATE_RUNNING       ,   /**< a task decodes it */
    ITEMSTATE_PARKED        ,   /**< started, waiting for a credit; its reader keeps the position */
    ITEMSTATE_DONE              /**< all batches decoded */
};

/** Async: a listed file is opened and its row groups are planned by a task outside the lock, then published in listing order */
enum EPlanState
{
    PLANSTATE_LISTED = 0    ,   /**< waiting for a task to open it */
    PLANSTATE_OPENING       ,   /**< a task opens it: the task owns it until it hands it back */
    PLANSTATE_PLANNED           /**< opened and planned (or failed), waiting for the files listed before it */
};

enum EDecodeOutcome
{
    DECODE_BATCH = 0    ,   /**< a batch with selected rows */
    DECODE_EMPTY        ,   /**< a batch without selected rows: nothing to deliver */
    DECODE_END          ,   /**< the row group is done */
    DECODE_ERROR
};

/** Waits a little for work of other threads: gives up the time slice for the first 5 ms of a wait, then sleeps in 1 ms steps */
void
BackOff( UInt32& round, UInt64 waitStartTicks )
{
    if ( round < 64 || MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - waitStartTicks ) < 5 )
        MT::ThreadDelay( 0 );
    else
        MT::ThreadDelay( 1 );
    ++round;
}

}

/*-------------------------------------------------------------------------*/

/** A file of a scan once it is planned: its metadata, how its columns become the delivered columns, its row groups */
class CDataTableScanPlannedFile
{
    public:

    typedef GUCEF::vector< SColumnPlan > TColumnPlanVector;
    typedef GUCEF::vector< UInt32 > TRowGroupVector;
    typedef GUCEF::vector< CITableReaderPtr > TReaderVector;

    CDataTableScanFile file;
    CITableFileMetaDataPtr metaData;
    CDataTableSchemaPtr fileSchema;
    CDataTableSchema::TColumnIndexVector fileColumnOfTableColumn;
    CDataTableSchema::TColumnIndexVector readerProjection;
    TColumnPlanVector columns;              /**< per read column of the scan */
    bool passThrough;                       /**< the reader's batches are the read columns as they are */
    TRowGroupVector rowGroups;              /**< the row groups left after pruning, in order */
    UInt32 nextRowGroup;                    /**< index into rowGroups of the next one to schedule */
    UInt32 itemsOutstanding;                /**< row groups scheduled and not done */
    TReaderVector idleReaders;              /**< readers on the file not in use */
    bool readerOpened;                      /**< a reader was opened on the file, for filesOpened */
    EPlanState planState;                   /**< async: where it is between listing and publishing */
    TDataTableStatus planStatus;            /**< async: how opening and planning it went, once it is planned */
    CString planError;

    CDataTableScanPlannedFile( void )
        : file()
        , metaData()
        , fileSchema()
        , fileColumnOfTableColumn()
        , readerProjection()
        , columns()
        , passThrough( false )
        , rowGroups()
        , nextRowGroup( 0 )
        , itemsOutstanding( 0 )
        , idleReaders()
        , readerOpened( false )
        , planState( PLANSTATE_LISTED )
        , planStatus( DATATABLE_STATUS_OK )
        , planError()
    {
    }
};

/*-------------------------------------------------------------------------*/

/** One row group of a planned file: the unit of work and of ordered delivery */
class CDataTableScanWorkItem
{
    public:

    UInt64 sequence;
    CDataTableScanPlannedFile* file;
    UInt32 rowGroup;
    EItemState state;
    CITableReaderPtr reader;
    bool opened;
    CDataTableReadRequest request;          /**< valid until the row group is done, readers may keep a reference */
    CDataTableScanStats stats;              /**< what the reader and the decode added, moved to the scan totals after each step */
    UInt32 readyHead;                       /**< ordered delivery: the decoded batches of this row group, a list of slots */
    UInt32 readyTail;
    UInt32 readyCount;

    CDataTableScanWorkItem( void )
        : sequence( 0 )
        , file( GUCEF_NULL )
        , rowGroup( 0 )
        , state( ITEMSTATE_PENDING )
        , reader()
        , opened( false )
        , request()
        , stats()
        , readyHead( NoSlot )
        , readyTail( NoSlot )
        , readyCount( 0 )
    {
    }

    void Recycle( void )
    {
        sequence = 0;
        file = GUCEF_NULL;
        rowGroup = 0;
        state = ITEMSTATE_PENDING;
        reader.Unlink();
        opened = false;
        request.query = GUCEF_NULL;
        request.stats = GUCEF_NULL;
        request.sharedFileLock = GUCEF_NULL;
        stats.Clear();
        readyHead = NoSlot;
        readyTail = NoSlot;
        readyCount = 0;
    }
};

/*-------------------------------------------------------------------------*/

/** The per task part of a scan: the filter compiled for the scan's columns */
class CDataTableScanWorkerContext
{
    public:

    CDataTableFilter filter;
};

/*-------------------------------------------------------------------------*/

/**
 *  The scheduler state of a CDataTableScan, shared with its tasks through a reference count: a task never touches the
 *  scan object, which may be gone. Everything below the lock is guarded by it; decoding happens outside it.
 */
class CDataTableScanState
{
    public:

    CDataTableScanState( const CDataTableScanSourcePtr& source, const CDataTableSchemaPtr& tableSchema, CDataTableBufferPool& pool, CDataTableScan* owner );

    void AddReference( void );
    void ReleaseReference( void );

    TDataTableStatus Start( const CDataTableQuery& query, bool async, ThreadPoolPtr threadPool, CFutureResult* future );
    TDataTableStatus NextBatchSync( CDataTableBatchPtr& batch );
    TDataTableStatus PopAsync( CDataTableBatchPtr& batch, UInt32 timeoutInMs, bool wait );
    void Cancel( void );
    void Detach( void );
    void WaitWhileBusy( void );
    void GetStats( CDataTableScanStats& stats ) const;
    CString GetLastErrorMessage( void ) const;
    bool IsAsync( void ) const;
    bool IsRunning( void ) const;

    /** The body of a scan task */
    void RunWorker( void );

    private:

    struct SPendingEvents
    {
        bool batchesAvailable;
        bool finished;
        bool failed;
        CDataTableScanStats stats;
        CString error;

        SPendingEvents( void ) : batchesAvailable( false ), finished( false ), failed( false ), stats(), error() {}
    };

    typedef CDataTableScanPlannedFile SPlannedFile;
    typedef CDataTableScanWorkItem SWorkItem;
    typedef CDataTableScanWorkerContext SWorkerContext;

    struct SSlot
    {
        CDataTableBatchPtr batch;
        UInt32 next;
    };

    typedef GUCEF::vector< SPlannedFile* > TPlannedFileVector;
    typedef GUCEF::vector< SWorkItem* > TWorkItemVector;
    typedef GUCEF::vector< SWorkerContext* > TWorkerContextVector;
    typedef GUCEF::vector< SSlot > TSlotVector;
    typedef CDataTableScanPlannedFile::TReaderVector TReaderVector;

    ~CDataTableScanState();

    CDataTableScanState( const CDataTableScanState& src );              /**< not implemented */
    CDataTableScanState& operator=( const CDataTableScanState& src );   /**< not implemented */

    bool IsStoppedLocked( void ) const;
    void EndLocked( TDataTableStatus status, const CString& message, SPendingEvents& events );
    void CheckCancellationLocked( SPendingEvents& events );
    void CheckCompletionLocked( SPendingEvents& events );
    void RaiseEvents( SPendingEvents& events );
    void FlushEvents( SPendingEvents& events );

    TDataTableStatus PlanNextFile( SPlannedFile*& planned, CString& error );
    TDataTableStatus ListNextFile( SPlannedFile*& listed, CDataTableScanStats& stats, CString& error );
    TDataTableStatus OpenListedFile( SPlannedFile& planned, CDataTableScanStats& stats, CString& error );
    void PublishPlannedFilesLocked( SPendingEvents& events );
    bool IsOpeningFilesLocked( void ) const;
    SPlannedFile* TakeListedFileLocked( void );
    UInt32 CountListedFilesLocked( void ) const;
    void DiscardPlanQueueLocked( void );
    TDataTableStatus PlanFileColumns( SPlannedFile& planned, CString& error );
    void PruneRowGroups( SPlannedFile& planned, CDataTableScanStats& stats );

    EDecodeOutcome DecodeStep( SWorkerContext& context, SWorkItem& item, CDataTableBatchPtr& batch, TDataTableStatus& status, CString& error );
    TDataTableStatus ComposeBatch( SPlannedFile& file, CDataTableBatchPtr& raw, CDataTableBatchPtr& batch, CString& error );
    TDataTableStatus MakeNullColumn( const CDataTableColumnDef& column, UInt32 rowCount, CDataTableColumnBatch& target );
    TDataTableStatus AdaptColumn( const SColumnPlan& plan, const CDataTableColumnDef& column, const CDataTableColumnBatch& source, UInt32 rowCount, CDataTableColumnBatch& target );
    TDataTableStatus FinishDelivery( CDataTableBatchPtr& batch );

    SWorkItem* NewItemLocked( SPlannedFile* file, UInt32 rowGroup );
    void RecycleItemLocked( SWorkItem* item );
    void FinishItemLocked( SWorkItem* item );
    SWorkItem* CreditHeadLocked( void ) const;
    bool MayTakeCreditLocked( const SWorkItem* item ) const;
    SWorkItem* PickItemLocked( void );
    bool CanListLocked( void ) const;
    bool HasRoomToListLocked( void ) const;
    UInt32 CountRunnableWorkLocked( bool afterCreditFreed ) const;
    UInt32 ScheduleWorkersLocked( bool afterCreditFreed );
    void QueueWorkers( UInt32 count );
    void UnlockForWork( SPendingEvents& events, UInt32& workersToQueue );
    UInt32 TakeSlotLocked( void );
    void FreeSlotLocked( UInt32 slot );
    void LinkReadyLocked( SWorkItem* item, UInt32 slot );
    bool HasDeliverableLocked( void ) const;
    bool PopReadyLocked( CDataTableBatchPtr& batch );
    void DeliverLocked( CDataTableBatchPtr& batch );
    TDataTableStatus TryPopLocked( CDataTableBatchPtr& batch, SPendingEvents& events, UInt32& workersToQueue );
    SWorkerContext* AcquireContextLocked( void );
    void ReleaseContextLocked( SWorkerContext* context );
    TDataTableStatus PrepareContext( SWorkerContext& context, CString& error );

    MT::CTAtomic< UInt32 > m_refCount;
    MT::CMutex m_lock;
    MT::CMutex m_ownerLock;
    CDataTableScan* m_owner;                        /**< guarded by m_ownerLock, NULL once the scan object goes */

    CDataTableScanSourcePtr m_source;
    CDataTableSchemaPtr m_tableSchema;
    CDataTableBufferPool* m_pool;

    CDataTableResolvedQuery m_query;
    CDataTableSchema::TColumnIndexVector m_readColumns;     /**< table columns read: the projection without duplicates, then predicate columns */
    UInt32 m_projectedReadColumns;                          /**< read columns that are delivered */
    bool m_identityDelivery;                                /**< the projection is the first read columns in order */
    CDataTableSchema::TColumnIndexVector m_deliveryMap;     /**< per projected column its read column, when not identity */
    bool m_exactFilter;

    bool m_started;
    bool m_async;
    bool m_ended;
    TDataTableStatus m_endStatus;
    CString m_errorMessage;
    bool m_limitReached;
    UInt64 m_rowsDelivered;
    bool m_listingDone;
    bool m_listing;                                 /**< a task lists the next file: one at a time */
    UInt32 m_nextFileOrdinal;
    UInt64 m_nextSequence;
    TPlannedFileVector m_files;
    TPlannedFileVector m_planQueue;                 /**< async: files listed and not yet planned or published, in listing order */
    TWorkItemVector m_items;                        /**< async: in sequence order, the oldest undelivered first */
    TWorkItemVector m_freeItems;

    TSlotVector m_slots;                            /**< async: one per credit */
    UInt32 m_freeSlotHead;
    UInt32 m_freeSlots;
    UInt32 m_readyHead;                             /**< unordered delivery: completed batches in completion order */
    UInt32 m_readyTail;
    UInt32 m_readyCount;
    UInt32 m_requeueThreshold;

    UInt32 m_parallelism;
    UInt32 m_scheduledWorkers;                      /**< tasks queued or running */
    UInt32 m_busyWorkers;                           /**< tasks working outside the lock: using the source, readers or the pool */
    ThreadPoolPtr m_threadPool;
    TWorkerContextVector m_idleContexts;
    TWorkerContextVector m_allContexts;
    bool m_availableEventPending;
    CFutureResultSource m_future;

    CDataTableScanStats m_stats;
    UInt64 m_poolMissesAtStart;

    SWorkerContext m_syncContext;
    SPlannedFile* m_syncFile;
    SWorkItem* m_syncItem;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataTableScanState::CDataTableScanState( const CDataTableScanSourcePtr& source, const CDataTableSchemaPtr& tableSchema, CDataTableBufferPool& pool, CDataTableScan* owner )
    : m_refCount( 1 )
    , m_lock()
    , m_ownerLock()
    , m_owner( owner )
    , m_source( source )
    , m_tableSchema( tableSchema )
    , m_pool( &pool )
    , m_query()
    , m_readColumns()
    , m_projectedReadColumns( 0 )
    , m_identityDelivery( true )
    , m_deliveryMap()
    , m_exactFilter( true )
    , m_started( false )
    , m_async( false )
    , m_ended( false )
    , m_endStatus( DATATABLE_STATUS_OK )
    , m_errorMessage()
    , m_limitReached( false )
    , m_rowsDelivered( 0 )
    , m_listingDone( false )
    , m_listing( false )
    , m_nextFileOrdinal( 0 )
    , m_nextSequence( 0 )
    , m_files()
    , m_planQueue()
    , m_items()
    , m_freeItems()
    , m_slots()
    , m_freeSlotHead( NoSlot )
    , m_freeSlots( 0 )
    , m_readyHead( NoSlot )
    , m_readyTail( NoSlot )
    , m_readyCount( 0 )
    , m_requeueThreshold( 1 )
    , m_parallelism( 1 )
    , m_scheduledWorkers( 0 )
    , m_busyWorkers( 0 )
    , m_threadPool()
    , m_idleContexts()
    , m_allContexts()
    , m_availableEventPending( false )
    , m_future( TTaskStatus::TASKSTATUS_QUEUED )
    , m_stats()
    , m_poolMissesAtStart( 0 )
    , m_syncContext()
    , m_syncFile( GUCEF_NULL )
    , m_syncItem( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableScanState::~CDataTableScanState()
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) m_slots.size(); ++i )
        m_slots[ i ].batch.Reset();
    for ( UInt32 i=0; i<(UInt32) m_items.size(); ++i )
        GUCEF_DELETE m_items[ i ];
    for ( UInt32 i=0; i<(UInt32) m_freeItems.size(); ++i )
        GUCEF_DELETE m_freeItems[ i ];
    GUCEF_DELETE m_syncItem;
    for ( UInt32 i=0; i<(UInt32) m_files.size(); ++i )
        GUCEF_DELETE m_files[ i ];
    for ( UInt32 i=0; i<(UInt32) m_planQueue.size(); ++i )
        GUCEF_DELETE m_planQueue[ i ];
    for ( UInt32 i=0; i<(UInt32) m_allContexts.size(); ++i )
        GUCEF_DELETE m_allContexts[ i ];
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::AddReference( void )
{

    m_refCount.FetchAdd( 1, MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::ReleaseReference( void )
{

    if ( 1 == m_refCount.FetchSub( 1, MT::MEMORDER_ACQ_REL ) )
        GUCEF_DELETE this;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableScanState::IsAsync( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_async;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableScanState::IsRunning( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_started && !m_ended;
}

/*-------------------------------------------------------------------------*/

CString
CDataTableScanState::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_errorMessage;
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::GetStats( CDataTableScanStats& stats ) const
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        stats = m_stats;
    }
    if ( !m_source.IsNULL() )
        m_source->AddSourceStats( stats );
    CDataTableBufferPoolStats poolStats;
    m_pool->GetStats( poolStats );
    stats.poolMisses = poolStats.misses >= m_poolMissesAtStart ? poolStats.misses - m_poolMissesAtStart : 0;
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::Detach( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_ownerLock );
    m_owner = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::WaitWhileBusy( void )
{GUCEF_TRACE;

    // tasks stop at their next batch boundary; none uses the source, a reader or the pool after this returns
    UInt32 round = 0;
    const UInt64 startTicks = MT::PrecisionTickCount();
    while ( true )
    {
        {
            MT::CScopeMutex lock( m_lock );
            if ( 0 == m_busyWorkers )
                return;
        }
        BackOff( round, startTicks );
    }
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::FlushEvents( SPendingEvents& events )
{GUCEF_TRACE;

    // raised while the task goes on working, so a consumer hears of batches as soon as they are there
    if ( !events.batchesAvailable && !events.finished && !events.failed )
        return;
    RaiseEvents( events );
    events.batchesAvailable = false;
    events.finished = false;
    events.failed = false;
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::RaiseEvents( SPendingEvents& events )
{GUCEF_TRACE;

    if ( !events.batchesAvailable && !events.finished && !events.failed )
        return;

    MT::CScopeMutex lock( m_ownerLock );
    if ( GUCEF_NULL == m_owner )
        return;
    if ( events.batchesAvailable )
        m_owner->NotifyObserversFromThread( CDataTableScan::ScanBatchesAvailableEvent );
    if ( events.finished )
        m_owner->NotifyObserversFromThread( CDataTableScan::ScanFinishedEvent, &events.stats );
    if ( events.failed )
    {
        TCloneableString error( events.error );
        m_owner->NotifyObserversFromThread( CDataTableScan::ScanFailedEvent, &error );
    }
}

/*-------------------------------------------------------------------------*/

bool
CDataTableScanState::IsStoppedLocked( void ) const
{

    return m_ended || m_limitReached;
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::EndLocked( TDataTableStatus status, const CString& message, SPendingEvents& events )
{GUCEF_TRACE;

    if ( m_ended )
        return;
    m_ended = true;
    m_endStatus = status;
    if ( DataTableStatusIsError( status ) )
        m_errorMessage = message;

    // batches decoded and not delivered go back to the pool
    for ( UInt32 i=0; i<(UInt32) m_items.size(); ++i )
    {
        SWorkItem* item = m_items[ i ];
        UInt32 slot = item->readyHead;
        while ( NoSlot != slot )
        {
            const UInt32 next = m_slots[ slot ].next;
            FreeSlotLocked( slot );
            slot = next;
        }
        item->readyHead = item->readyTail = NoSlot;
        item->readyCount = 0;
        if ( ITEMSTATE_RUNNING != item->state )
        {
            item->reader.Unlink();
            item->state = ITEMSTATE_DONE;
        }
    }
    UInt32 slot = m_readyHead;
    while ( NoSlot != slot )
    {
        const UInt32 next = m_slots[ slot ].next;
        FreeSlotLocked( slot );
        slot = next;
    }
    m_readyHead = m_readyTail = NoSlot;
    m_readyCount = 0;
    for ( UInt32 i=0; i<(UInt32) m_files.size(); ++i )
    {
        TReaderVector& idle = m_files[ i ]->idleReaders;
        idle.clear();
    }
    DiscardPlanQueueLocked();

    if ( DATATABLE_STATUS_END == status )
    {
        events.finished = true;
        events.stats = m_stats;
        if ( m_async )
            m_future.Complete( m_stats.Clone(), true );
    }
    else
    if ( DATATABLE_STATUS_CANCELLED == status )
    {
        if ( m_async )
            m_future.Cancel();
    }
    else
    {
        events.failed = true;
        events.error = message;
        if ( m_async )
        {
            const CString extraInfo = CString( DataTableStatusToString( status ) ) + ": " + message;
            m_future.Fail( TTaskStatus::TASKSTATUS_STARTUP_FAILED, extraInfo );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::CheckCancellationLocked( SPendingEvents& events )
{

    if ( !m_ended && m_async && m_future.IsCancellationRequested() )
        EndLocked( DATATABLE_STATUS_CANCELLED, CString::Empty, events );
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::CheckCompletionLocked( SPendingEvents& events )
{

    if ( m_ended )
        return;
    if ( m_limitReached )
    {
        EndLocked( DATATABLE_STATUS_END, CString::Empty, events );
        return;
    }
    if ( !m_listingDone || m_listing || !m_planQueue.empty() )
        return;
    for ( UInt32 i=0; i<(UInt32) m_items.size(); ++i )
    {
        if ( ITEMSTATE_DONE != m_items[ i ]->state || m_items[ i ]->readyCount > 0 )
            return;
    }
    if ( m_readyCount > 0 )
        return;
    EndLocked( DATATABLE_STATUS_END, CString::Empty, events );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableScanState::PrepareContext( SWorkerContext& context, CString& error )
{GUCEF_TRACE;

    if ( !m_exactFilter )
        return DATATABLE_STATUS_OK;
    return context.filter.Prepare( m_query, m_readColumns, error );
}

/*-------------------------------------------------------------------------*/

CDataTableScanState::SWorkerContext*
CDataTableScanState::AcquireContextLocked( void )
{

    if ( !m_idleContexts.empty() )
    {
        SWorkerContext* context = m_idleContexts.back();
        m_idleContexts.pop_back();
        return context;
    }
    SWorkerContext* context = GUCEF_NEW SWorkerContext();
    CString error;
    if ( DATATABLE_STATUS_OK != PrepareContext( *context, error ) )
    {
        // the query was compiled once at Start: this cannot fail for another context
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataTableScan: compiling the filter of a scan task failed: " << error );
    }
    m_allContexts.push_back( context );
    return context;
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::ReleaseContextLocked( SWorkerContext* context )
{

    if ( GUCEF_NULL != context )
        m_idleContexts.push_back( context );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableScanState::Start( const CDataTableQuery& query, bool async, ThreadPoolPtr threadPool, CFutureResult* future )
{GUCEF_TRACE;

    SPendingEvents events;
    UInt32 workersToQueue = 0;
    TDataTableStatus result = DATATABLE_STATUS_OK;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_started )
        {
            m_errorMessage = "A scan is started once";
            if ( GUCEF_NULL != future )
            {
                CFutureResultSource refused;
                refused.Fail( TTaskStatus::TASKSTATUS_TASKDATA_INVALID, m_errorMessage );
                *future = refused.GetFuture();
            }
            return DATATABLE_STATUS_INVALID_ARGUMENT;
        }
        m_started = true;
        m_async = async;
        m_freeItems.reserve( 16 );
        m_files.reserve( 4 );
        if ( GUCEF_NULL != future )
            *future = m_future.GetFuture();

        CDataTableBufferPoolStats poolStats;
        m_pool->GetStats( poolStats );
        m_poolMissesAtStart = poolStats.misses;

        CString error;
        if ( m_tableSchema.IsNULL() || m_source.IsNULL() )
        {
            EndLocked( DATATABLE_STATUS_INVALID_ARGUMENT, "A scan needs a source and a table schema", events );
            result = DATATABLE_STATUS_INVALID_ARGUMENT;
        }
        else
        {
            m_query.schema = m_tableSchema;
            if ( !query.Resolve( *m_tableSchema, m_query, error ) )
            {
                result = DATATABLE_STATUS_OK != m_query.status ? m_query.status : DATATABLE_STATUS_INVALID_ARGUMENT;
                EndLocked( result, error, events );
            }
        }

        if ( DATATABLE_STATUS_OK == result )
        {
            // the columns to read: the projection once each, then the columns only the filter needs
            m_exactFilter = DATATABLE_FILTER_EXACT == m_query.filterMode;
            m_readColumns.clear();
            m_deliveryMap.clear();
            m_identityDelivery = true;
            for ( UInt32 i=0; i<(UInt32) m_query.projection.size(); ++i )
            {
                const UInt32 column = m_query.projection[ i ];
                UInt32 readIndex = GUCEF_UINT32MAX;
                for ( UInt32 r=0; r<(UInt32) m_readColumns.size(); ++r )
                {
                    if ( m_readColumns[ r ] == column )
                    {
                        readIndex = r;
                        break;
                    }
                }
                if ( GUCEF_UINT32MAX == readIndex )
                {
                    readIndex = (UInt32) m_readColumns.size();
                    m_readColumns.push_back( column );
                }
                if ( readIndex != i )
                    m_identityDelivery = false;
                m_deliveryMap.push_back( readIndex );
            }
            m_projectedReadColumns = (UInt32) m_readColumns.size();
            if ( m_exactFilter )
            {
                for ( UInt32 p=0; p<(UInt32) m_query.predicates.size(); ++p )
                {
                    const UInt32 column = m_query.predicates[ p ].columnIndex;
                    bool present = false;
                    for ( UInt32 r=0; r<(UInt32) m_readColumns.size(); ++r )
                        present = present || m_readColumns[ r ] == column;
                    if ( !present )
                        m_readColumns.push_back( column );
                }
            }

            result = PrepareContext( m_syncContext, error );
            if ( DATATABLE_STATUS_OK != result )
                EndLocked( result, error, events );
        }

        if ( DATATABLE_STATUS_OK == result )
        {
            if ( m_query.matchesNothing || 0 == m_query.limit )
            {
                // nothing can be delivered: no file is listed
                EndLocked( DATATABLE_STATUS_END, CString::Empty, events );
            }
            else
            if ( async )
            {
                if ( threadPool.IsNULL() )
                {
                    CTaskManager& taskManager = CCoreGlobal::Instance()->GetTaskManager();
                    threadPool = taskManager.GetOrCreateThreadPool( DataTablesThreadPoolName );
                    if ( !threadPool.IsNULL() && 0 == threadPool->GetDesiredMinNrOfWorkerThreads() )
                        threadPool->SetNrOfWorkerThreadsToLogicalCPUs( 1 );
                }
                if ( threadPool.IsNULL() )
                {
                    result = DATATABLE_STATUS_FAILED;
                    EndLocked( result, "No thread pool is available for the async scan", events );
                }
                else
                {
                    m_threadPool = threadPool;
                    m_parallelism = m_query.maxParallelism;
                    if ( 0 == m_parallelism )
                    {
                        m_parallelism = threadPool->GetDesiredMinNrOfWorkerThreads();
                        const UInt32 active = threadPool->GetActiveNrOfWorkerThreads();
                        if ( active > m_parallelism )
                            m_parallelism = active;
                        if ( 0 == m_parallelism )
                            m_parallelism = 1;
                    }
                    UInt32 credits = m_query.maxBatchesInFlight;
                    if ( 0 == credits )
                        credits = 2 * m_parallelism;
                    m_slots.resize( credits );
                    for ( UInt32 i=0; i<credits; ++i )
                        m_slots[ i ].next = i + 1 < credits ? i + 1 : NoSlot;
                    m_freeSlotHead = 0;
                    m_freeSlots = credits;
                    m_requeueThreshold = credits / 2 > 0 ? credits / 2 : 1;
                    m_future.SetStatus( TTaskStatus::TASKSTATUS_RUNNING );
                    workersToQueue = ScheduleWorkersLocked( true );
                }
            }
        }
    }
    QueueWorkers( workersToQueue );
    RaiseEvents( events );
    return result;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableScanState::PlanFileColumns( SPlannedFile& planned, CString& error )
{GUCEF_TRACE;

    if ( planned.metaData.IsNULL() || planned.metaData->GetSchema().IsNULL() )
    {
        error = "File \"" + planned.file.path + "\" has no schema";
        return DATATABLE_STATUS_CORRUPT_DATA;
    }
    planned.fileSchema = planned.metaData->GetSchema();
    const CDataTableSchema& fileSchema = *planned.fileSchema;
    const CDataTableSchema& tableSchema = *m_tableSchema;

    CString mismatch;
    if ( !tableSchema.IsReadableAs( fileSchema, mismatch ) )
    {
        error = "File \"" + planned.file.path + "\" does not fit the table: " + mismatch;
        return DATATABLE_STATUS_SCHEMA_MISMATCH;
    }

    const UInt32 tableColumns = tableSchema.GetColumnCount();
    planned.fileColumnOfTableColumn.assign( tableColumns, GUCEF_UINT32MAX );
    for ( UInt32 t=0; t<tableColumns; ++t )
    {
        const Int32 fileColumn = fileSchema.FindColumn( tableSchema.GetColumn( t ).name );
        if ( fileColumn >= 0 )
            planned.fileColumnOfTableColumn[ t ] = (UInt32) fileColumn;
    }

    planned.readerProjection.clear();
    planned.columns.assign( m_readColumns.size(), SColumnPlan() );
    planned.passThrough = true;
    for ( UInt32 i=0; i<(UInt32) m_readColumns.size(); ++i )
    {
        const UInt32 tableColumn = m_readColumns[ i ];
        const UInt32 fileColumn = planned.fileColumnOfTableColumn[ tableColumn ];
        SColumnPlan& plan = planned.columns[ i ];
        if ( GUCEF_UINT32MAX == fileColumn )
        {
            plan.adapt = COLUMNADAPT_NULLS;
            planned.passThrough = false;
            continue;
        }
        plan.readerColumn = (UInt32) planned.readerProjection.size();
        planned.readerProjection.push_back( fileColumn );
        if ( plan.readerColumn != i )
            planned.passThrough = false;

        const CDataTableColumnDef& tableDef = tableSchema.GetColumn( tableColumn );
        const CDataTableColumnDef& fileDef = fileSchema.GetColumn( fileColumn );
        if ( tableDef.IsSameType( fileDef ) )
            continue;
        planned.passThrough = false;
        if ( DATATABLE_TYPE_INT64 == tableDef.type && DATATABLE_TYPE_INT32 == fileDef.type )
            plan.adapt = COLUMNADAPT_INT32_TO_INT64;
        else
        if ( DATATABLE_TYPE_FLOAT64 == tableDef.type && DATATABLE_TYPE_FLOAT32 == fileDef.type )
            plan.adapt = COLUMNADAPT_FLOAT32_TO_FLOAT64;
        else
        if ( DATATABLE_TYPE_DECIMAL == tableDef.type && DATATABLE_TYPE_DECIMAL == fileDef.type && fileDef.decimalScale < tableDef.decimalScale )
        {
            plan.adapt = COLUMNADAPT_DECIMAL_RESCALE;
            plan.rescaleFactor = 1;
            for ( UInt8 s=fileDef.decimalScale; s<tableDef.decimalScale; ++s )
                plan.rescaleFactor *= 10;
        }
        else
        {
            error = "File \"" + planned.file.path + "\": column \"" + tableDef.name + "\" is " + fileDef.GetTypeText() + " and cannot be read as " + tableDef.GetTypeText();
            return DATATABLE_STATUS_SCHEMA_MISMATCH;
        }
    }
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::PruneRowGroups( SPlannedFile& planned, CDataTableScanStats& stats )
{GUCEF_TRACE;

    const UInt32 rowGroupCount = planned.metaData->GetRowGroupCount();
    stats.rowGroupsTotal += rowGroupCount;
    planned.rowGroups.clear();
    planned.rowGroups.reserve( rowGroupCount );
    const bool prune = !m_query.predicates.empty();
    CDataTableRowGroupInfo info;
    for ( UInt32 rg=0; rg<rowGroupCount; ++rg )
    {
        if ( prune && planned.metaData->GetRowGroupInfo( rg, info ) &&
             !CDataTablePruner::RowGroupMayMatch( m_query, info, planned.fileColumnOfTableColumn, planned.fileSchema.GetPointerAlways() ) )
        {
            ++stats.rowGroupsPrunedByStatistics;
            continue;
        }
        planned.rowGroups.push_back( rg );
    }
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableScanState::ListNextFile( SPlannedFile*& listed, CDataTableScanStats& stats, CString& error )
{GUCEF_TRACE;

    // runs outside the lock, by one thread at a time: the source lists in delivery order
    listed = GUCEF_NULL;
    CDataTableScanFile file;
    TDataTableStatus status = m_source->NextFile( file );
    if ( DATATABLE_STATUS_END == status )
        return status;
    if ( DATATABLE_STATUS_OK != status )
    {
        error = "Listing the files of the scan failed: " + CString( DataTableStatusToString( status ) );
        return status;
    }

    SPlannedFile* plan = GUCEF_NEW SPlannedFile();
    plan->file = file;
    // room for the reader that returns after each row group, made while planning so decoding batches allocates nothing
    plan->idleReaders.reserve( 1 );
    ++stats.filesListed;
    listed = plan;
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableScanState::OpenListedFile( SPlannedFile& plan, CDataTableScanStats& stats, CString& error )
{GUCEF_TRACE;

    // runs outside the lock, by several threads at once for different files: the source opens files concurrently
    TDataTableStatus status = DATATABLE_STATUS_OK;
    if ( !plan.file.metaData.IsNULL() )
    {
        plan.metaData = plan.file.metaData;
        ++stats.footerCacheHits;
    }
    else
    {
        CITableReaderPtr reader;
        status = m_source->OpenFile( plan.file, reader );
        if ( DATATABLE_STATUS_OK != status || reader.IsNULL() )
        {
            error = "Opening file \"" + plan.file.path + "\" failed: " + CString( DataTableStatusToString( DATATABLE_STATUS_OK == status ? DATATABLE_STATUS_FAILED : status ) );
            return DATATABLE_STATUS_OK == status ? DATATABLE_STATUS_FAILED : status;
        }
        ++stats.filesOpened;
        plan.readerOpened = true;
        plan.metaData = reader->GetMetaData();
        plan.idleReaders.push_back( reader );
    }

    status = PlanFileColumns( plan, error );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    PruneRowGroups( plan, stats );
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableScanState::PlanNextFile( SPlannedFile*& planned, CString& error )
{GUCEF_TRACE;

    // the sync scan: lists, opens and plans on the calling thread, outside the lock
    planned = GUCEF_NULL;
    CDataTableScanStats stats;
    SPlannedFile* plan = GUCEF_NULL;
    TDataTableStatus status = ListNextFile( plan, stats, error );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    {
        MT::CScopeMutex lock( m_lock );
        plan->file.fileOrdinal = m_nextFileOrdinal++;
    }
    status = OpenListedFile( *plan, stats, error );
    if ( DATATABLE_STATUS_OK != status )
    {
        GUCEF_DELETE plan;
        return status;
    }

    MT::CScopeMutex lock( m_lock );
    m_stats += stats;
    planned = plan;
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::PublishPlannedFilesLocked( SPendingEvents& events )
{

    // planned files become row groups in listing order, so the delivery sequence stays that of the listing whichever
    // task finished opening first; the first file that failed ends the scan, as it would have planned in sequence
    while ( !m_planQueue.empty() && PLANSTATE_PLANNED == m_planQueue.front()->planState )
    {
        SPlannedFile* planned = m_planQueue.front();
        m_planQueue.erase( m_planQueue.begin() );
        if ( m_ended )
        {
            GUCEF_DELETE planned;
            continue;
        }
        if ( DATATABLE_STATUS_OK != planned->planStatus )
        {
            const TDataTableStatus status = planned->planStatus;
            const CString error = planned->planError;
            GUCEF_DELETE planned;
            EndLocked( status, error, events );
            continue;
        }
        m_files.push_back( planned );
        for ( UInt32 i=0; i<(UInt32) planned->rowGroups.size(); ++i )
            m_items.push_back( NewItemLocked( planned, planned->rowGroups[ i ] ) );
        planned->nextRowGroup = (UInt32) planned->rowGroups.size();
        if ( planned->rowGroups.empty() )
            planned->idleReaders.clear();
    }
    CheckCompletionLocked( events );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableScanState::IsOpeningFilesLocked( void ) const
{

    for ( UInt32 i=0; i<(UInt32) m_planQueue.size(); ++i )
    {
        if ( PLANSTATE_PLANNED != m_planQueue[ i ]->planState )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CDataTableScanState::SPlannedFile*
CDataTableScanState::TakeListedFileLocked( void )
{

    // the earliest listed file first, so files are published soon after they are opened
    for ( UInt32 i=0; i<(UInt32) m_planQueue.size(); ++i )
    {
        SPlannedFile* planned = m_planQueue[ i ];
        if ( PLANSTATE_LISTED == planned->planState )
        {
            planned->planState = PLANSTATE_OPENING;
            return planned;
        }
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableScanState::CountListedFilesLocked( void ) const
{

    UInt32 listed = 0;
    for ( UInt32 i=0; i<(UInt32) m_planQueue.size(); ++i )
    {
        if ( PLANSTATE_LISTED == m_planQueue[ i ]->planState )
            ++listed;
    }
    return listed;
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::DiscardPlanQueueLocked( void )
{

    // a file a task is opening belongs to that task until it hands it back; the others go
    UInt32 kept = 0;
    for ( UInt32 i=0; i<(UInt32) m_planQueue.size(); ++i )
    {
        if ( PLANSTATE_OPENING == m_planQueue[ i ]->planState )
            m_planQueue[ kept++ ] = m_planQueue[ i ];
        else
            GUCEF_DELETE m_planQueue[ i ];
    }
    m_planQueue.resize( kept );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableScanState::MakeNullColumn( const CDataTableColumnDef& column, UInt32 rowCount, CDataTableColumnBatch& target )
{GUCEF_TRACE;

    target.SetType( column );
    const UInt32 bitmapBytes = ( rowCount + 7 ) >> 3;
    CDataTableBufferPtr validity;
    TDataTableStatus status = m_pool->Acquire( bitmapBytes, validity );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    memset( validity->GetData(), 0, bitmapBytes );
    validity->SetDataSize( bitmapBytes );

    switch ( DataTableLogicalTypeStorage( column.type ) )
    {
        case DATATABLE_STORAGE_FIXED:
        case DATATABLE_STORAGE_BITS:
        {
            const UInt32 width = DataTableLogicalTypeByteWidth( column.type );
            const UInt64 bytes = 0 == width ? bitmapBytes : (UInt64) rowCount * width;
            if ( bytes > GUCEF_UINT32MAX - DataTableBufferPadding )
                return DATATABLE_STATUS_LIMIT_EXCEEDED;
            CDataTableBufferPtr values;
            status = m_pool->Acquire( (UInt32) bytes, values );
            if ( DATATABLE_STATUS_OK != status )
                return status;
            memset( values->GetData(), 0, (size_t) bytes );
            values->SetDataSize( (UInt32) bytes );
            target.SetFixedValues( rowCount, values->GetData(), values );
            break;
        }
        case DATATABLE_STORAGE_VARIABLE:
        {
            // every null is an empty value: its terminator only
            CDataTableBufferPtr offsets;
            CDataTableBufferPtr bytes;
            status = m_pool->Acquire( ( rowCount + 1 ) * (UInt32) sizeof( UInt32 ), offsets );
            if ( DATATABLE_STATUS_OK == status )
                status = m_pool->Acquire( rowCount, bytes );
            if ( DATATABLE_STATUS_OK != status )
                return status;
            UInt32* offsetValues = static_cast< UInt32* >( offsets->GetData() );
            for ( UInt32 i=0; i<=rowCount; ++i )
                offsetValues[ i ] = i;
            memset( bytes->GetData(), 0, rowCount );
            offsets->SetDataSize( ( rowCount + 1 ) * (UInt32) sizeof( UInt32 ) );
            bytes->SetDataSize( rowCount );
            target.SetVariableValues( rowCount, offsetValues, offsets, static_cast< const char* >( bytes->GetData() ), bytes );
            break;
        }
        default:
        {
            return DATATABLE_STATUS_UNSUPPORTED;
        }
    }
    target.SetValidity( static_cast< const UInt8* >( validity->GetData() ), rowCount, validity );
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableScanState::AdaptColumn( const SColumnPlan& plan, const CDataTableColumnDef& column, const CDataTableColumnBatch& source, UInt32 rowCount, CDataTableColumnBatch& target )
{GUCEF_TRACE;

    CDataTableBufferPtr values;
    TDataTableStatus status = m_pool->Acquire( rowCount * 8, values );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    switch ( plan.adapt )
    {
        case COLUMNADAPT_INT32_TO_INT64:
        {
            const Int32* in = source.GetInt32Data();
            if ( GUCEF_NULL == in && rowCount > 0 )
                return DATATABLE_STATUS_SCHEMA_MISMATCH;
            Int64* out = static_cast< Int64* >( values->GetData() );
            for ( UInt32 i=0; i<rowCount; ++i )
                out[ i ] = in[ i ];
            break;
        }
        case COLUMNADAPT_FLOAT32_TO_FLOAT64:
        {
            const Float32* in = source.GetFloat32Data();
            if ( GUCEF_NULL == in && rowCount > 0 )
                return DATATABLE_STATUS_SCHEMA_MISMATCH;
            Float64* out = static_cast< Float64* >( values->GetData() );
            for ( UInt32 i=0; i<rowCount; ++i )
                out[ i ] = in[ i ];
            break;
        }
        case COLUMNADAPT_DECIMAL_RESCALE:
        {
            const Int64* in = source.GetInt64Data();
            if ( GUCEF_NULL == in && rowCount > 0 )
                return DATATABLE_STATUS_SCHEMA_MISMATCH;
            Int64* out = static_cast< Int64* >( values->GetData() );
            for ( UInt32 i=0; i<rowCount; ++i )
                out[ i ] = in[ i ] * plan.rescaleFactor;
            break;
        }
        default:
        {
            return DATATABLE_STATUS_FAILED;
        }
    }
    values->SetDataSize( rowCount * 8 );
    target.SetType( column );
    target.SetFixedValues( rowCount, values->GetData(), values );

    if ( GUCEF_NULL != source.GetValidity() )
    {
        const UInt32 bitmapBytes = ( rowCount + 7 ) >> 3;
        CDataTableBufferPtr validity;
        status = m_pool->Acquire( bitmapBytes, validity );
        if ( DATATABLE_STATUS_OK != status )
            return status;
        memcpy( validity->GetData(), source.GetValidity(), bitmapBytes );
        validity->SetDataSize( bitmapBytes );
        target.SetValidity( static_cast< const UInt8* >( validity->GetData() ), source.GetNullCount(), validity );
    }
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableScanState::ComposeBatch( SPlannedFile& file, CDataTableBatchPtr& raw, CDataTableBatchPtr& batch, CString& error )
{GUCEF_TRACE;

    const CDataTableBatch& rawBatch = *raw;
    const UInt32 rowCount = rawBatch.GetRowCount();
    const UInt32 columnCount = (UInt32) m_readColumns.size();

    if ( file.passThrough && 1 == rawBatch.GetReferenceCount() && rawBatch.GetColumnCount() == columnCount )
    {
        // the common case: the reader's batch is delivered as the table's, nothing is copied
        CDataTableBatch* writable = raw.GetWritable();
        writable->SetSchema( m_tableSchema );
        for ( UInt32 i=0; i<columnCount; ++i )
        {
            if ( writable->GetColumn( i ).GetRowCount() != rowCount )
            {
                error = "File \"" + file.file.path + "\": the reader delivered columns of unequal row counts";
                return DATATABLE_STATUS_CORRUPT_DATA;
            }
            writable->SetSchemaColumnIndex( i, m_readColumns[ i ] );
        }
        batch.Swap( raw );
        raw.Reset();
        return DATATABLE_STATUS_OK;
    }

    TDataTableStatus status = m_pool->AcquireBatch( batch );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    CDataTableBatch* writable = batch.GetWritable();
    if ( !writable->SetColumnCount( columnCount ) )
        return DATATABLE_STATUS_OUT_OF_MEMORY;
    writable->SetSchema( m_tableSchema );
    writable->SetRowCount( rowCount );
    writable->SetOrigin( rawBatch.GetOrigin() );
    writable->SetGuaranteedPredicates( rawBatch.GetGuaranteedPredicates() );

    for ( UInt32 i=0; i<columnCount; ++i )
    {
        const SColumnPlan& plan = file.columns[ i ];
        const CDataTableColumnDef& column = m_tableSchema->GetColumn( m_readColumns[ i ] );
        CDataTableColumnBatch& target = writable->GetMutableColumn( i );
        writable->SetSchemaColumnIndex( i, m_readColumns[ i ] );
        if ( COLUMNADAPT_NULLS == plan.adapt )
        {
            status = MakeNullColumn( column, rowCount, target );
        }
        else
        {
            const CDataTableColumnBatch& source = rawBatch.GetColumn( plan.readerColumn );
            if ( source.GetRowCount() != rowCount )
            {
                error = "File \"" + file.file.path + "\": the reader delivered columns of unequal row counts";
                return DATATABLE_STATUS_CORRUPT_DATA;
            }
            if ( COLUMNADAPT_NONE == plan.adapt )
            {
                target = source;
                status = DATATABLE_STATUS_OK;
            }
            else
            {
                status = AdaptColumn( plan, column, source, rowCount, target );
            }
        }
        if ( DATATABLE_STATUS_OK != status )
        {
            error = "File \"" + file.file.path + "\": column \"" + column.name + "\" could not be made: " + DataTableStatusToString( status );
            return status;
        }
    }

    if ( GUCEF_NULL != rawBatch.GetSelection() )
    {
        // a selection the reader made: copied, its buffer belongs to the reader's batch
        const UInt32 selected = rawBatch.GetSelectedRowCount();
        CDataTableBufferPtr selection;
        status = m_pool->Acquire( selected * (UInt32) sizeof( UInt32 ), selection );
        if ( DATATABLE_STATUS_OK != status )
            return status;
        memcpy( selection->GetData(), rawBatch.GetSelection(), selected * sizeof( UInt32 ) );
        selection->SetDataSize( selected * (UInt32) sizeof( UInt32 ) );
        writable->SetSelection( static_cast< const UInt32* >( selection->GetData() ), selected, selection );
    }
    raw.Reset();
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableScanState::FinishDelivery( CDataTableBatchPtr& batch )
{GUCEF_TRACE;

    // the columns only the filter needed are dropped, the projection is put in its order
    CDataTableBatch* writable = batch.GetWritable();
    if ( m_identityDelivery )
    {
        if ( writable->GetColumnCount() > m_projectedReadColumns )
            writable->SetColumnCount( m_projectedReadColumns );
        return DATATABLE_STATUS_OK;
    }

    CDataTableBatchPtr delivered;
    TDataTableStatus status = m_pool->AcquireBatch( delivered );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    CDataTableBatch* target = delivered.GetWritable();
    const UInt32 columnCount = (UInt32) m_deliveryMap.size();
    if ( !target->SetColumnCount( columnCount ) )
        return DATATABLE_STATUS_OUT_OF_MEMORY;
    target->SetSchema( m_tableSchema );
    target->SetRowCount( writable->GetRowCount() );
    target->SetOrigin( writable->GetOrigin() );
    target->SetGuaranteedPredicates( writable->GetGuaranteedPredicates() );
    for ( UInt32 i=0; i<columnCount; ++i )
    {
        target->GetMutableColumn( i ) = writable->GetColumn( m_deliveryMap[ i ] );
        target->SetSchemaColumnIndex( i, m_query.projection[ i ] );
    }
    if ( GUCEF_NULL != writable->GetSelection() )
    {
        const UInt32 selected = writable->GetSelectedRowCount();
        CDataTableBufferPtr selection;
        status = m_pool->Acquire( selected * (UInt32) sizeof( UInt32 ), selection );
        if ( DATATABLE_STATUS_OK != status )
            return status;
        memcpy( selection->GetData(), writable->GetSelection(), selected * sizeof( UInt32 ) );
        selection->SetDataSize( selected * (UInt32) sizeof( UInt32 ) );
        target->SetSelection( static_cast< const UInt32* >( selection->GetData() ), selected, selection );
    }
    batch.Swap( delivered );
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

EDecodeOutcome
CDataTableScanState::DecodeStep( SWorkerContext& context, SWorkItem& item, CDataTableBatchPtr& batch, TDataTableStatus& status, CString& error )
{GUCEF_TRACE;

    // runs outside the lock; the item belongs to the calling task while it is RUNNING
    SPlannedFile& file = *item.file;
    status = DATATABLE_STATUS_OK;

    if ( item.reader.IsNULL() )
    {
        status = m_source->OpenFile( file.file, item.reader );
        if ( DATATABLE_STATUS_OK != status || item.reader.IsNULL() )
        {
            if ( DATATABLE_STATUS_OK == status )
                status = DATATABLE_STATUS_FAILED;
            error = "Opening file \"" + file.file.path + "\" failed: " + CString( DataTableStatusToString( status ) );
            return DECODE_ERROR;
        }
    }

    if ( !item.opened )
    {
        CDataTableReadRequest& request = item.request;
        request.query = &m_query;
        request.projection = file.readerProjection;
        request.fileColumnOfTableColumn = file.fileColumnOfTableColumn;
        request.batchRowCount = m_query.batchRowCount;
        request.filterMode = m_query.filterMode;
        request.materializeDictionaries = m_query.materializeDictionaries;
        request.bufferPool = m_pool;
        request.sharedFileLock = file.file.sharedFileLock;
        request.fileOrdinal = file.file.fileOrdinal;
        request.stats = &item.stats;

        item.opened = true;
        status = item.reader->OpenRowGroup( item.rowGroup, request );
        if ( DATATABLE_STATUS_PRUNED == status )
        {
            ++item.stats.rowGroupsPrunedByDictionary;
            status = DATATABLE_STATUS_OK;
            return DECODE_END;
        }
        if ( DATATABLE_STATUS_OK != status )
        {
            error = "File \"" + file.file.path + "\", row group " + ToString( item.rowGroup ) + ": " + item.reader->GetLastErrorMessage();
            return DECODE_ERROR;
        }
    }

    CDataTableBatchPtr raw;
    status = item.reader->ReadNextBatch( raw );
    if ( DATATABLE_STATUS_END == status )
    {
        status = DATATABLE_STATUS_OK;
        return DECODE_END;
    }
    if ( DATATABLE_STATUS_OK != status || raw.IsNULL() )
    {
        if ( DATATABLE_STATUS_OK == status )
            status = DATATABLE_STATUS_FAILED;
        error = "File \"" + file.file.path + "\", row group " + ToString( item.rowGroup ) + ": " + item.reader->GetLastErrorMessage();
        return DECODE_ERROR;
    }
    item.stats.rowsScanned += raw->GetRowCount();

    status = ComposeBatch( file, raw, batch, error );
    if ( DATATABLE_STATUS_OK != status )
        return DECODE_ERROR;

    if ( m_exactFilter )
    {
        UInt32 selected = 0;
        status = context.filter.Apply( *batch.GetWritable(), *m_pool, selected );
        if ( DATATABLE_STATUS_OK != status )
        {
            error = "File \"" + file.file.path + "\": filtering a batch failed: " + DataTableStatusToString( status );
            return DECODE_ERROR;
        }
        if ( 0 == selected )
        {
            batch.Reset();
            return DECODE_EMPTY;
        }
    }
    else
    if ( 0 == batch->GetSelectedRowCount() )
    {
        batch.Reset();
        return DECODE_EMPTY;
    }

    status = FinishDelivery( batch );
    if ( DATATABLE_STATUS_OK != status )
    {
        error = "Making the delivered columns failed: " + CString( DataTableStatusToString( status ) );
        return DECODE_ERROR;
    }
    return DECODE_BATCH;
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::DeliverLocked( CDataTableBatchPtr& batch )
{

    // the limit counts delivered selected rows; the batch that reaches it is cut to fit
    const UInt32 selected = batch->GetSelectedRowCount();
    UInt32 delivered = selected;
    if ( CDataTableQuery::NoLimit != m_query.limit )
    {
        const UInt64 remaining = m_query.limit > m_rowsDelivered ? m_query.limit - m_rowsDelivered : 0;
        if ( (UInt64) selected >= remaining )
        {
            delivered = (UInt32) remaining;
            if ( delivered < selected )
                batch.GetWritable()->TruncateSelection( delivered );
            m_limitReached = true;
        }
    }
    m_rowsDelivered += delivered;
    m_stats.rowsSelected += delivered;
    ++m_stats.batchesDelivered;
}

/*-------------------------------------------------------------------------*/

CDataTableScanState::SWorkItem*
CDataTableScanState::NewItemLocked( SPlannedFile* file, UInt32 rowGroup )
{

    SWorkItem* item = GUCEF_NULL;
    if ( !m_freeItems.empty() )
    {
        item = m_freeItems.back();
        m_freeItems.pop_back();
    }
    else
    {
        item = GUCEF_NEW SWorkItem();
    }
    item->sequence = m_nextSequence++;
    item->file = file;
    item->rowGroup = rowGroup;
    item->state = ITEMSTATE_PENDING;
    ++file->itemsOutstanding;
    return item;
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::RecycleItemLocked( SWorkItem* item )
{

    item->Recycle();
    m_freeItems.push_back( item );
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::FinishItemLocked( SWorkItem* item )
{

    item->state = ITEMSTATE_DONE;
    m_stats += item->stats;
    item->stats.Clear();
    SPlannedFile* file = item->file;
    if ( !item->reader.IsNULL() )
    {
        // the reader serves the next row group of the file
        if ( !m_ended )
            file->idleReaders.push_back( item->reader );
        item->reader.Unlink();
    }
    if ( file->itemsOutstanding > 0 )
        --file->itemsOutstanding;
    if ( 0 == file->itemsOutstanding && file->nextRowGroup >= (UInt32) file->rowGroups.size() )
    {
        // the file is done: its readers go, and so does the plan
        file->idleReaders.clear();
    }
}

/*-------------------------------------------------------------------------*/

CDataTableScanState::SWorkItem*
CDataTableScanState::CreditHeadLocked( void ) const
{

    for ( UInt32 i=0; i<(UInt32) m_items.size(); ++i )
    {
        if ( ITEMSTATE_DONE != m_items[ i ]->state )
            return m_items[ i ];
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableScanState::MayTakeCreditLocked( const SWorkItem* item ) const
{

    // ordered delivery keeps the last credit for the earliest unfinished row group, so it can always progress
    if ( 0 == m_freeSlots )
        return false;
    if ( !m_query.orderedDelivery || m_freeSlots > 1 )
        return true;
    return item == CreditHeadLocked();
}

/*-------------------------------------------------------------------------*/

CDataTableScanState::SWorkItem*
CDataTableScanState::PickItemLocked( void )
{

    // credits go out in sequence order: the earliest row group that waits, parked or not yet started
    for ( UInt32 i=0; i<(UInt32) m_items.size(); ++i )
    {
        SWorkItem* item = m_items[ i ];
        if ( ITEMSTATE_PENDING != item->state && ITEMSTATE_PARKED != item->state )
            continue;
        if ( !MayTakeCreditLocked( item ) )
            return GUCEF_NULL;
        item->state = ITEMSTATE_RUNNING;
        if ( item->reader.IsNULL() && !item->file->idleReaders.empty() )
        {
            item->reader = item->file->idleReaders.back();
            item->file->idleReaders.pop_back();
        }
        if ( item->reader.IsNULL() && !item->file->readerOpened )
        {
            // the task opens the first reader on a file whose metadata came with the listing
            item->file->readerOpened = true;
            ++item->stats.filesOpened;
        }
        return item;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableScanState::CanListLocked( void ) const
{

    return !m_listing && HasRoomToListLocked();
}

/*-------------------------------------------------------------------------*/

bool
CDataTableScanState::HasRoomToListLocked( void ) const
{

    if ( m_listingDone || IsStoppedLocked() )
        return false;
    UInt32 pending = (UInt32) m_planQueue.size();
    for ( UInt32 i=0; i<(UInt32) m_items.size(); ++i )
    {
        if ( ITEMSTATE_PENDING == m_items[ i ]->state )
            ++pending;
    }
    // files are listed and planned up to the parallelism ahead of decoding: the files being opened count
    return pending < m_parallelism;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableScanState::CountRunnableWorkLocked( bool afterCreditFreed ) const
{

    if ( IsStoppedLocked() || 0 == m_freeSlots )
        return 0;
    UInt32 runnable = ( CanListLocked() ? 1 : 0 ) + CountListedFilesLocked();
    const SWorkItem* head = CreditHeadLocked();
    for ( UInt32 i=0; i<(UInt32) m_items.size() && runnable < m_freeSlots; ++i )
    {
        const SWorkItem* item = m_items[ i ];
        if ( ITEMSTATE_PENDING != item->state && ITEMSTATE_PARKED != item->state )
            continue;
        if ( m_query.orderedDelivery && 1 == m_freeSlots && item != head )
            break;
        // a parked row group is queued again once half the credits are free, the head of the line at once
        if ( afterCreditFreed && ITEMSTATE_PARKED == item->state && m_freeSlots < m_requeueThreshold && item != head )
            continue;
        ++runnable;
    }
    return runnable;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableScanState::ScheduleWorkersLocked( bool afterCreditFreed )
{

    if ( !m_async || m_scheduledWorkers >= m_parallelism )
        return 0;
    UInt32 count = CountRunnableWorkLocked( afterCreditFreed );
    // tasks already queued or running pick up runnable work too
    const UInt32 idleScheduled = m_scheduledWorkers > m_busyWorkers ? m_scheduledWorkers - m_busyWorkers : 0;
    count = count > idleScheduled ? count - idleScheduled : 0;
    if ( count > m_parallelism - m_scheduledWorkers )
        count = m_parallelism - m_scheduledWorkers;
    m_scheduledWorkers += count;
    return count;
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::UnlockForWork( SPendingEvents& events, UInt32& workersToQueue )
{GUCEF_TRACE;

    // the tasks scheduled meanwhile are queued now, not when this task ends: a task that keeps finding work would
    // otherwise hold back the tasks counted as scheduled, and the scan would run on one thread
    m_lock.Unlock();
    const UInt32 count = workersToQueue;
    workersToQueue = 0;
    QueueWorkers( count );
    FlushEvents( events );
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::QueueWorkers( UInt32 count )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<count; ++i )
    {
        CDataTableScanTaskData taskData( this );
        CFutureResult queued = m_threadPool->QueueTask( CDataTableScanTask::TaskType, &taskData, GUCEF_NULL, false );
        if ( queued.HasNoFuture() )
        {
            SPendingEvents events;
            {
                MT::CScopeMutex lock( m_lock );
                --m_scheduledWorkers;
                if ( 0 == m_scheduledWorkers )
                    EndLocked( DATATABLE_STATUS_FAILED, "A task of the scan could not be queued on thread pool \"" + m_threadPool->GetThreadPoolName() + "\"", events );
            }
            RaiseEvents( events );
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableScanState::TakeSlotLocked( void )
{

    const UInt32 slot = m_freeSlotHead;
    if ( NoSlot != slot )
    {
        m_freeSlotHead = m_slots[ slot ].next;
        m_slots[ slot ].next = NoSlot;
        --m_freeSlots;
    }
    return slot;
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::FreeSlotLocked( UInt32 slot )
{

    m_slots[ slot ].batch.Reset();
    m_slots[ slot ].next = m_freeSlotHead;
    m_freeSlotHead = slot;
    ++m_freeSlots;
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::LinkReadyLocked( SWorkItem* item, UInt32 slot )
{

    m_slots[ slot ].next = NoSlot;
    if ( m_query.orderedDelivery )
    {
        if ( NoSlot == item->readyTail )
            item->readyHead = slot;
        else
            m_slots[ item->readyTail ].next = slot;
        item->readyTail = slot;
        ++item->readyCount;
    }
    else
    {
        if ( NoSlot == m_readyTail )
            m_readyHead = slot;
        else
            m_slots[ m_readyTail ].next = slot;
        m_readyTail = slot;
        ++m_readyCount;
    }
}

/*-------------------------------------------------------------------------*/

bool
CDataTableScanState::HasDeliverableLocked( void ) const
{

    if ( !m_query.orderedDelivery )
        return m_readyCount > 0;
    for ( UInt32 i=0; i<(UInt32) m_items.size(); ++i )
    {
        const SWorkItem* item = m_items[ i ];
        if ( item->readyCount > 0 )
            return true;
        if ( ITEMSTATE_DONE != item->state )
            return false;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableScanState::PopReadyLocked( CDataTableBatchPtr& batch )
{

    UInt32 slot = NoSlot;
    if ( m_query.orderedDelivery )
    {
        while ( !m_items.empty() )
        {
            SWorkItem* head = m_items.front();
            if ( head->readyCount > 0 )
            {
                slot = head->readyHead;
                head->readyHead = m_slots[ slot ].next;
                if ( NoSlot == head->readyHead )
                    head->readyTail = NoSlot;
                --head->readyCount;
                break;
            }
            if ( ITEMSTATE_DONE != head->state )
                return false;
            // the head is done and delivered: the next row group is the head now
            m_items.erase( m_items.begin() );
            RecycleItemLocked( head );
        }
    }
    else
    {
        slot = m_readyHead;
        if ( NoSlot != slot )
        {
            m_readyHead = m_slots[ slot ].next;
            if ( NoSlot == m_readyHead )
                m_readyTail = NoSlot;
            --m_readyCount;
        }
    }
    if ( NoSlot == slot )
        return false;

    batch.Swap( m_slots[ slot ].batch );
    FreeSlotLocked( slot );
    return true;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableScanState::TryPopLocked( CDataTableBatchPtr& batch, SPendingEvents& events, UInt32& workersToQueue )
{

    if ( !m_started )
    {
        m_errorMessage = "The scan was not started";
        return DATATABLE_STATUS_INVALID_ARGUMENT;
    }
    CheckCancellationLocked( events );
    if ( m_ended )
        return m_endStatus;

    if ( PopReadyLocked( batch ) )
    {
        DeliverLocked( batch );
        if ( !HasDeliverableLocked() )
            m_availableEventPending = false;
        CheckCompletionLocked( events );
        if ( !m_ended )
            workersToQueue = ScheduleWorkersLocked( true );
        return DATATABLE_STATUS_OK;
    }

    m_availableEventPending = false;
    CheckCompletionLocked( events );
    if ( m_ended )
        return m_endStatus;
    return DATATABLE_STATUS_PENDING;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableScanState::PopAsync( CDataTableBatchPtr& batch, UInt32 timeoutInMs, bool wait )
{GUCEF_TRACE;

    batch.Reset();
    const UInt64 startTicks = MT::PrecisionTickCount();
    UInt32 round = 0;
    while ( true )
    {
        SPendingEvents events;
        UInt32 workersToQueue = 0;
        TDataTableStatus status = DATATABLE_STATUS_PENDING;
        {
            MT::CScopeMutex lock( m_lock );
            status = TryPopLocked( batch, events, workersToQueue );
        }
        QueueWorkers( workersToQueue );
        RaiseEvents( events );

        if ( DATATABLE_STATUS_PENDING != status || !wait )
            return status;
        if ( GUCEF_UINT32MAX != timeoutInMs && MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks ) >= timeoutInMs )
            return DATATABLE_STATUS_PENDING;
        BackOff( round, startTicks );
    }
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::Cancel( void )
{GUCEF_TRACE;

    SPendingEvents events;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_started && !m_ended )
            EndLocked( DATATABLE_STATUS_CANCELLED, CString::Empty, events );
    }
    RaiseEvents( events );
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanState::RunWorker( void )
{GUCEF_TRACE;

    SPendingEvents events;
    UInt32 workersToQueue = 0;

    m_lock.Lock();
    SWorkerContext* context = AcquireContextLocked();
    SWorkItem* item = GUCEF_NULL;
    UInt64 lingerStartTicks = 0;
    UInt64 workLingerStartTicks = 0;
    while ( true )
    {
        CheckCancellationLocked( events );
        if ( IsStoppedLocked() )
            break;

        if ( GUCEF_NULL == item )
            item = PickItemLocked();
        if ( GUCEF_NULL == item )
        {
            // open a listed file and plan its row groups outside the lock: tasks open several files at once, so the
            // latency of opening small files overlaps
            SPlannedFile* planned = TakeListedFileLocked();
            if ( GUCEF_NULL != planned )
            {
                workLingerStartTicks = 0;
                ++m_busyWorkers;
                UnlockForWork( events, workersToQueue );
                CDataTableScanStats planStats;
                CString error;
                const TDataTableStatus status = OpenListedFile( *planned, planStats, error );
                m_lock.Lock();
                --m_busyWorkers;
                if ( DATATABLE_STATUS_OK == status )
                    m_stats += planStats;
                planned->planStatus = status;
                planned->planError = error;
                planned->planState = PLANSTATE_PLANNED;
                if ( m_ended )
                    planned->idleReaders.clear();
                PublishPlannedFilesLocked( events );
                if ( m_ended )
                    break;
                workersToQueue += ScheduleWorkersLocked( false );
                continue;
            }

            if ( !CanListLocked() )
            {
                // another task lists the next file or opens one: its row groups come within microseconds. Ending
                // the task and queueing a new one costs more than that (a pool thread picks up a queued task at its
                // next poll), so linger briefly with the lock released, as for a credit
                if ( IsStoppedLocked() || ( !m_listing && !IsOpeningFilesLocked() ) )
                    break;
                if ( 0 == workLingerStartTicks )
                    workLingerStartTicks = MT::PrecisionTickCount();
                if ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - workLingerStartTicks ) >= WorkLingerMs )
                    break;
                UnlockForWork( events, workersToQueue );
                MT::ThreadDelay( 0 );
                m_lock.Lock();
                continue;
            }
            workLingerStartTicks = 0;

            // list files, one task at a time and outside the lock, until the files ahead of decoding reach the
            // parallelism; the other tasks open them meanwhile
            m_listing = true;
            ++m_busyWorkers;
            bool listing = true;
            while ( listing )
            {
                UnlockForWork( events, workersToQueue );
                CDataTableScanStats listStats;
                CString error;
                const TDataTableStatus status = ListNextFile( planned, listStats, error );
                m_lock.Lock();
                listing = false;
                CheckCancellationLocked( events );
                if ( DATATABLE_STATUS_END == status )
                {
                    m_listingDone = true;
                    break;
                }
                if ( DATATABLE_STATUS_OK != status )
                {
                    EndLocked( status, error, events );
                    break;
                }
                if ( m_ended )
                {
                    // cancelled while listing
                    GUCEF_DELETE planned;
                    break;
                }
                m_stats += listStats;
                planned->file.fileOrdinal = m_nextFileOrdinal++;
                planned->planState = PLANSTATE_LISTED;
                m_planQueue.push_back( planned );
                workersToQueue += ScheduleWorkersLocked( false );

                // the role of lister is kept while there is room ahead, so no file waits for a task to take it up
                listing = HasRoomToListLocked();
            }
            --m_busyWorkers;
            m_listing = false;
            CheckCompletionLocked( events );
            if ( m_ended )
                break;
            continue;
        }

        workLingerStartTicks = 0;
        if ( !MayTakeCreditLocked( item ) )
        {
            // a consumer that keeps up frees a credit within microseconds: linger briefly before parking, since a
            // parked row group costs a task to queue and start again. The lock is free while lingering.
            if ( 0 == lingerStartTicks )
                lingerStartTicks = MT::PrecisionTickCount();
            if ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - lingerStartTicks ) < CreditLingerMs )
            {
                UnlockForWork( events, workersToQueue );
                MT::ThreadDelay( 0 );
                m_lock.Lock();
                continue;
            }
            item->state = ITEMSTATE_PARKED;
            item = GUCEF_NULL;
            break;
        }
        lingerStartTicks = 0;
        const UInt32 slot = TakeSlotLocked();

        ++m_busyWorkers;
        UnlockForWork( events, workersToQueue );
        CDataTableBatchPtr batch;
        TDataTableStatus status = DATATABLE_STATUS_OK;
        CString error;
        const EDecodeOutcome outcome = DecodeStep( *context, *item, batch, status, error );
        m_lock.Lock();
        --m_busyWorkers;
        m_stats += item->stats;
        item->stats.Clear();

        switch ( outcome )
        {
            case DECODE_BATCH:
            {
                if ( IsStoppedLocked() )
                {
                    batch.Reset();
                    FreeSlotLocked( slot );
                    break;
                }
                const bool wasDeliverable = HasDeliverableLocked();
                m_slots[ slot ].batch.Swap( batch );
                LinkReadyLocked( item, slot );
                if ( !wasDeliverable && !m_availableEventPending && HasDeliverableLocked() )
                {
                    // coalesced: raised when the ring goes from empty to not empty, not again until it was drained
                    m_availableEventPending = true;
                    events.batchesAvailable = true;
                }
                break;
            }
            case DECODE_EMPTY:
            {
                FreeSlotLocked( slot );
                break;
            }
            case DECODE_END:
            {
                FreeSlotLocked( slot );
                FinishItemLocked( item );
                item = GUCEF_NULL;
                if ( !m_query.orderedDelivery )
                {
                    // without ordering a finished row group is not needed for delivery
                    for ( UInt32 i=0; i<(UInt32) m_items.size(); ++i )
                    {
                        if ( ITEMSTATE_DONE == m_items[ i ]->state )
                        {
                            SWorkItem* done = m_items[ i ];
                            m_items.erase( m_items.begin() + i );
                            RecycleItemLocked( done );
                            break;
                        }
                    }
                }
                // a finished head of the line lets the next row group take the reserved credit
                workersToQueue += ScheduleWorkersLocked( false );
                if ( HasDeliverableLocked() && !m_availableEventPending )
                {
                    m_availableEventPending = true;
                    events.batchesAvailable = true;
                }
                CheckCompletionLocked( events );
                break;
            }
            default:
            {
                FreeSlotLocked( slot );
                FinishItemLocked( item );
                item = GUCEF_NULL;
                EndLocked( status, error, events );
                break;
            }
        }
        if ( m_ended )
            break;
    }

    if ( GUCEF_NULL != item && ITEMSTATE_RUNNING == item->state )
    {
        if ( m_ended )
        {
            FinishItemLocked( item );
        }
        else
        {
            item->state = ITEMSTATE_PARKED;
        }
    }
    ReleaseContextLocked( context );
    --m_scheduledWorkers;
    if ( !m_ended )
    {
        CheckCompletionLocked( events );
        if ( !m_ended )
            workersToQueue += ScheduleWorkersLocked( false );
    }
    m_lock.Unlock();

    QueueWorkers( workersToQueue );
    RaiseEvents( events );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableScanState::NextBatchSync( CDataTableBatchPtr& batch )
{GUCEF_TRACE;

    batch.Reset();
    while ( true )
    {
        SPendingEvents events;
        {
            MT::CScopeMutex lock( m_lock );
            if ( !m_started )
            {
                m_errorMessage = "The scan was not started";
                return DATATABLE_STATUS_INVALID_ARGUMENT;
            }
            if ( m_ended )
                return m_endStatus;
            if ( m_limitReached )
            {
                EndLocked( DATATABLE_STATUS_END, CString::Empty, events );
                lock.EarlyUnlock();
                RaiseEvents( events );
                return DATATABLE_STATUS_END;
            }
        }

        if ( GUCEF_NULL == m_syncItem )
        {
            if ( GUCEF_NULL != m_syncFile && m_syncFile->nextRowGroup < (UInt32) m_syncFile->rowGroups.size() )
            {
                MT::CScopeMutex lock( m_lock );
                m_syncItem = NewItemLocked( m_syncFile, m_syncFile->rowGroups[ m_syncFile->nextRowGroup++ ] );
                m_syncItem->state = ITEMSTATE_RUNNING;
                if ( !m_syncFile->idleReaders.empty() )
                {
                    m_syncItem->reader = m_syncFile->idleReaders.back();
                    m_syncFile->idleReaders.pop_back();
                }
                if ( m_syncItem->reader.IsNULL() && !m_syncFile->readerOpened )
                {
                    m_syncFile->readerOpened = true;
                    ++m_syncItem->stats.filesOpened;
                }
            }
            else
            {
                if ( GUCEF_NULL != m_syncFile )
                {
                    MT::CScopeMutex lock( m_lock );
                    m_syncFile->idleReaders.clear();
                    for ( UInt32 i=0; i<(UInt32) m_files.size(); ++i )
                    {
                        if ( m_files[ i ] == m_syncFile )
                        {
                            m_files.erase( m_files.begin() + i );
                            break;
                        }
                    }
                    GUCEF_DELETE m_syncFile;
                    m_syncFile = GUCEF_NULL;
                }

                SPlannedFile* planned = GUCEF_NULL;
                CString error;
                const TDataTableStatus status = PlanNextFile( planned, error );
                MT::CScopeMutex lock( m_lock );
                if ( DATATABLE_STATUS_OK == status )
                {
                    m_files.push_back( planned );
                    m_syncFile = planned;
                    continue;
                }
                EndLocked( status, error, events );
                lock.EarlyUnlock();
                RaiseEvents( events );
                return m_endStatus;
            }
        }

        TDataTableStatus status = DATATABLE_STATUS_OK;
        CString error;
        const EDecodeOutcome outcome = DecodeStep( m_syncContext, *m_syncItem, batch, status, error );

        MT::CScopeMutex lock( m_lock );
        m_stats += m_syncItem->stats;
        m_syncItem->stats.Clear();
        if ( m_ended )
        {
            // cancelled from another thread meanwhile
            batch.Reset();
            return m_endStatus;
        }
        switch ( outcome )
        {
            case DECODE_BATCH:
            {
                DeliverLocked( batch );
                return DATATABLE_STATUS_OK;
            }
            case DECODE_EMPTY:
            {
                break;
            }
            case DECODE_END:
            {
                FinishItemLocked( m_syncItem );
                RecycleItemLocked( m_syncItem );
                m_syncItem = GUCEF_NULL;
                break;
            }
            default:
            {
                FinishItemLocked( m_syncItem );
                RecycleItemLocked( m_syncItem );
                m_syncItem = GUCEF_NULL;
                EndLocked( status, error, events );
                lock.EarlyUnlock();
                RaiseEvents( events );
                return status;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

CDataTableScanTaskData::CDataTableScanTaskData( CDataTableScanState* state )
    : CICloneable()
    , m_state( state )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_state )
        m_state->AddReference();
}

/*-------------------------------------------------------------------------*/

CDataTableScanTaskData::CDataTableScanTaskData( const CDataTableScanTaskData& src )
    : CICloneable( src )
    , m_state( src.m_state )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_state )
        m_state->AddReference();
}

/*-------------------------------------------------------------------------*/

CDataTableScanTaskData::~CDataTableScanTaskData()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_state )
        m_state->ReleaseReference();
}

/*-------------------------------------------------------------------------*/

CICloneable*
CDataTableScanTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDataTableScanTaskData( *this );
}

/*-------------------------------------------------------------------------*/

CDataTableScanState*
CDataTableScanTaskData::GetState( void ) const
{GUCEF_TRACE;

    return m_state;
}

/*-------------------------------------------------------------------------*/

CDataTableScanTask::CDataTableScanTask( void )
    : CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableScanTask::~CDataTableScanTask()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDataTableScanTask::OnTaskCycle( CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CDataTableScanTaskData* taskData = static_cast< CDataTableScanTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL != taskData && GUCEF_NULL != taskData->GetState() )
        taskData->GetState()->RunWorker();

    // one run per task: a task without credit or work ends, the scan queues a new one when there is
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanTask::OnTaskEnded( CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    // the task data holds a reference on the scan state; it goes with the task
}

/*-------------------------------------------------------------------------*/

CString
CDataTableScanTask::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CString&
CDataTableScanTask::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CDataTableScanTask";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CDataTableScan::RegisterEvents( void )
{GUCEF_TRACE;

    ScanBatchesAvailableEvent.Initialize();
    ScanFinishedEvent.Initialize();
    ScanFailedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CDataTableScan::CDataTableScan( const CDataTableScanSourcePtr& source       ,
                                const CDataTableSchemaPtr& tableSchema      ,
                                CDataTableBufferPool& bufferPool            ,
                                PulseGeneratorPtr pulseGenerator            )
    : CTSGNotifier( pulseGenerator.IsNULL() ? CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator )
    , CIDataTableBatchSource()
    , m_state( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();
    m_state = GUCEF_NEW CDataTableScanState( source, tableSchema, bufferPool, this );
}

/*-------------------------------------------------------------------------*/

CDataTableScan::~CDataTableScan()
{GUCEF_TRACE;

    SignalUpcomingDestruction();

    // no event reaches this object any more; running tasks stop at their next batch and none touches the source,
    // a reader or the pool once the wait returns. Tasks still queued only see the shared state, which they hold.
    m_state->Detach();
    m_state->Cancel();
    m_state->WaitWhileBusy();
    m_state->ReleaseReference();
    m_state = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableScan::Start( const CDataTableQuery& query )
{GUCEF_TRACE;

    return m_state->Start( query, false, ThreadPoolPtr(), GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CDataTableScan::StartAsync( const CDataTableQuery& query, ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CFutureResult future( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    m_state->Start( query, true, threadPool, &future );
    return future;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableScan::NextBatch( CDataTableBatchPtr& batch )
{GUCEF_TRACE;

    if ( m_state->IsAsync() )
        return m_state->PopAsync( batch, GUCEF_UINT32MAX, true );
    return m_state->NextBatchSync( batch );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableScan::TryNextBatch( CDataTableBatchPtr& batch )
{GUCEF_TRACE;

    if ( m_state->IsAsync() )
        return m_state->PopAsync( batch, 0, false );
    return m_state->NextBatchSync( batch );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableScan::WaitForNextBatch( CDataTableBatchPtr& batch, UInt32 timeoutInMs )
{GUCEF_TRACE;

    if ( m_state->IsAsync() )
        return m_state->PopAsync( batch, timeoutInMs, true );
    return m_state->NextBatchSync( batch );
}

/*-------------------------------------------------------------------------*/

void
CDataTableScan::Cancel( void )
{GUCEF_TRACE;

    m_state->Cancel();
}

/*-------------------------------------------------------------------------*/

bool
CDataTableScan::IsAsync( void ) const
{GUCEF_TRACE;

    return m_state->IsAsync();
}

/*-------------------------------------------------------------------------*/

void
CDataTableScan::GetStats( CDataTableScanStats& stats ) const
{GUCEF_TRACE;

    m_state->GetStats( stats );
}

/*-------------------------------------------------------------------------*/

CString
CDataTableScan::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    return m_state->GetLastErrorMessage();
}

/*-------------------------------------------------------------------------*/

const CString&
CDataTableScan::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CDataTableScan";
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
