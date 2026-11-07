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


#ifndef GUCEF_DRN_CDRNDATAGRIDTABLE_H
#define GUCEF_DRN_CDRNDATAGRIDTABLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CIDATATABLE_H
#include "gucefCORE_CIDataTable.h"
#define GUCEF_CORE_CIDATATABLE_H
#endif /* GUCEF_CORE_CIDATATABLE_H ? */

#ifndef GUCEF_CORE_CMEMORYDATATABLE_H
#include "gucefCORE_CMemoryDataTable.h"
#define GUCEF_CORE_CMEMORYDATATABLE_H
#endif /* GUCEF_CORE_CMEMORYDATATABLE_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef GUCEF_CORE_CDATATABLEASYNCOPERATION_H
#include "gucefCORE_CDataTableAsyncOperation.h"
#define GUCEF_CORE_CDATATABLEASYNCOPERATION_H
#endif /* GUCEF_CORE_CDATATABLEASYNCOPERATION_H ? */

#ifndef GUCEF_DRN_CDRNDATAGRID_H
#include "gucefDRN_CDRNDataGrid.h"
#define GUCEF_DRN_CDRNDATAGRID_H
#endif /* GUCEF_DRN_CDRNDATAGRID_H ? */

#ifndef GUCEF_DRN_CDRNDATAGRIDTABLESETTINGS_H
#include "gucefDRN_CDRNDataGridTableSettings.h"
#define GUCEF_DRN_CDRNDATAGRIDTABLESETTINGS_H
#endif /* GUCEF_DRN_CDRNDATAGRIDTABLESETTINGS_H ? */

#ifndef GUCEF_DRN_CDRNDATAGRIDPENDINGWRITE_H
#include "gucefDRN_CDRNDataGridPendingWrite.h"
#define GUCEF_DRN_CDRNDATAGRIDPENDINGWRITE_H
#endif /* GUCEF_DRN_CDRNDATAGRIDPENDINGWRITE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDRNSession;

/**
 *  A data grid as a data table: one row per key, so that browsers, exports and tools that work on CORE::CIDataTable
 *  reach replicated state without knowing DRN. An optional second way in: it only uses the public interface of the
 *  grid, a grid that no table is opened on is not touched by it, and the grid stays reachable directly at the same time.
 *
 *  Columns: key (primary key), value, revision (UInt32), lastWriter (peer id, UInt32), lastChange (UInt64, the session
 *  time in microseconds at which the change was sent) and, when the table includes removed keys, removed (Bool). With
 *  declared key and value types (grid settings, or the table settings) the key and value columns have these types, a
 *  value that does not fit its column is shown as null and counted, a key that does not fit hides its row (counted);
 *  without them both are the text form of the variants and the table is read only.
 *
 *  Scans read a copy of the grid taken in chunks of settings.scanChunkKeyCount keys, one hold of the lock of the node
 *  per chunk, then run the shared scan over the copy (a CMemoryDataTable): rows come in key index order. Within a chunk
 *  the state is that of one update tick, and when the tick did not advance between the chunks the scan is one
 *  consistent state (GetLastScanTicks). A predicate that names key with equals or in is answered by key lookups.
 *
 *  Writes: UPSERT and INSERT (REPLACE_PARTITIONS is UNSUPPORTED). On the publisher they apply at once; on a replica
 *  that may write they are proposals: Write reports them as sent, WriteAsync ends when the publisher (and the vote)
 *  answered for each, with the reason of a refusal per row. A replica of an OWNER grid is read only. WriteAsync
 *  counts the answers of the grid to this member as its own: do not write the grid through other means from the same
 *  member while one is pending.
 *
 *  RowsChangedEvent follows the grid updates (per packet or per change notification of the grid; a grid in POLL mode
 *  tells nothing). The table listens to the grid only while an observer is subscribed to RowsChangedEvent (or to all
 *  events) of the table, or a write waits for answers; otherwise the grid has no observer of the table.
 *
 *  A table ends (every call answers NOT_FOUND) when the item is removed or the session ends. The session and node
 *  must outlive calls in progress, as for the grid itself. Thread safe as CIDataTable requires.
 */
class GUCEF_DRN_EXPORT_CPP CDRNDataGridTable : public CORE::CIDataTable
{
    public:

    static const CORE::CString KeyColumnName;
    static const CORE::CString ValueColumnName;
    static const CORE::CString RevisionColumnName;
    static const CORE::CString LastWriterColumnName;
    static const CORE::CString LastChangeColumnName;
    static const CORE::CString RemovedColumnName;

    /** A table over a grid of the session; events reach observers on the thread of the pulse generator, the global one when NULL */
    CDRNDataGridTable( CDRNSession& session                                     ,
                       CDRNDataGrid& grid                                       ,
                       const CDRNDataGridTableSettings& settings = CDRNDataGridTableSettings() ,
                       CORE::PulseGeneratorPtr pulseGenerator = CORE::PulseGeneratorPtr() );

    virtual ~CDRNDataGridTable() GUCEF_VIRTUAL_OVERRIDE;

    const CDRNDataGridTableSettings& GetSettings( void ) const;

    /** Key and value have a type: rows can be written */
    bool IsTyped( void ) const;

    /** The item was removed or the session ended */
    bool IsClosed( void ) const;

    /** Values shown as null because they do not fit the type of the value column, over all scans */
    UInt64 GetMismatchedValueCount( void ) const;

    /** Keys whose row is hidden because the key does not fit the type of the key column, over all scans */
    UInt64 GetMismatchedKeyCount( void ) const;

    /** Scans that took a copy of the grid */
    UInt64 GetSnapshotCount( void ) const;

    /** The update ticks of the grid at the first and the last chunk of the last scan this thread made; false when it made none */
    bool GetLastScanTicks( UInt32& firstTick, UInt32& lastTick ) const;

    /** How many subscriptions the table has at the grid: none while nobody listens to RowsChangedEvent and no write waits */
    UInt32 GetGridSubscriptionCount( void );

    virtual const CORE::CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CDataTableDefinition& GetDefinition( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CDataTableSchemaPtr GetSchema( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsReadOnly( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus Scan( const CORE::CDataTableQuery& query, CORE::CDataTableScanPtr& scan ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult ScanAsync( const CORE::CDataTableQuery& query, CORE::CDataTableScanPtr& scan, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus Write( const CORE::CDataTableBatch& rows, CORE::TDataTableWriteMode mode, CORE::CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult WriteAsync( const CORE::CDataTableBatchPtr& rows, CORE::TDataTableWriteMode mode, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus DeleteRows( const CORE::CDataTableQuery& predicates, CORE::CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult DeleteRowsAsync( const CORE::CDataTableQuery& predicates, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    /** A grid has no partitions: UNSUPPORTED */
    virtual CORE::TDataTableStatus DeletePartitions( const CORE::CDataTableQuery& partitionFilter, CORE::CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult DeletePartitionsAsync( const CORE::CDataTableQuery& partitionFilter, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    /** No partitions: an empty list */
    virtual CORE::TDataTableStatus ListPartitions( const CORE::CDataTableQuery& partitionFilter, bool withFileStats, CORE::CDataTablePartitionInfo::TPartitionInfoVector& partitions ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult ListPartitionsAsync( const CORE::CDataTableQuery& partitionFilter, bool withFileStats, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    /** Nothing to compact: UNSUPPORTED */
    virtual CORE::TDataTableStatus Compact( const CORE::CDataTableQuery& partitionFilter, bool force, CORE::CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult CompactAsync( const CORE::CDataTableQuery& partitionFilter, bool force, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    /** Drops the cached copy of the grid (settings.snapshotCacheMs) */
    virtual void Refresh( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    // the grid listens only while this table has observers: these keep count

    virtual bool Subscribe( CORE::CObserver* observer ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Subscribe( CORE::CObserver* observer, const CORE::CEvent& eventid, CORE::CIEventHandlerFunctorBase* callback = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Unsubscribe( CORE::CObserver* observer ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Unsubscribe( CORE::CObserver* observer, const CORE::CEvent& eventid ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void UnsubscribeAllFromNotifier( void ) GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnObserverDestruction( CORE::CObserver* observer ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    class CSessionWatcher : public CORE::CObserver
    {
        public:
        CDRNDataGridTable* m_table;
        CORE::CNotifier* m_session;
        CSessionWatcher( void ) : CORE::CObserver(), m_table( GUCEF_NULL ), m_session( GUCEF_NULL ) {}
        virtual ~CSessionWatcher() GUCEF_VIRTUAL_OVERRIDE { SignalUpcomingObserverDestruction(); }
        protected:
        virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;
    };
    friend class CSessionWatcher;

    typedef CORE::CTEventHandlerFunctor< CDRNDataGridTable >    TEventCallback;
    typedef GUCEF::map< CORE::CObserver*, UInt32 >              TObserverMap;
    typedef GUCEF::map< UInt32, CORE::CString >                 TLastErrorMap;
    typedef GUCEF::vector< CDRNDataGridPendingWritePtr >        TPendingWriteVector;

    class CScanTicks
    {
        public:
        UInt32 firstTick;
        UInt32 lastTick;
        CScanTicks( void ) : firstTick( 0 ), lastTick( 0 ) {}
    };
    typedef GUCEF::map< UInt32, CScanTicks >                    TScanTicksMap;

    CDRNDataGridTable( const CDRNDataGridTable& src );                  /**< not implemented */
    CDRNDataGridTable& operator=( const CDRNDataGridTable& src );       /**< not implemented */

    void BuildDefinition( void );
    CORE::TDataTableStatus Fail( CORE::TDataTableStatus status, const CORE::CString& message ) const;
    CORE::TDataTableStatus Succeed( void ) const;
    CDRNDataGrid* AcquireGrid( CDRNSession*& session ) const;
    CORE::TDataTableStatus CheckWritable( CORE::TDataTableWriteMode mode, CDRNDataGrid*& grid, CDRNSession*& session ) const;
    CORE::CFutureResult QueueOperation( CORE::CDataTableAsyncOperationData& data, CORE::ThreadPoolPtr threadPool );

    // scans
    bool GetKeyLookup( const CORE::CDataTableQuery& query, CORE::CVariant::VariantVector& keys ) const;
    bool AppendRow( CORE::CDataTableBatchBuilder& builder, const CORE::CVariant& key, bool hasValue, const CORE::CVariant& value, UInt64& mismatchedValues, UInt64& mismatchedKeys ) const;
    CORE::TDataTableStatus FlushBatch( CORE::CDataTableBatchBuilder& builder, CORE::CMemoryDataTable& snapshot );
    CORE::TDataTableStatus TakeSnapshot( const CORE::CDataTableQuery& query, CORE::CMemoryDataTablePtr& snapshot );

    // writes
    CORE::TDataTableStatus ExtractChanges( const CORE::CDataTableBatch& rows, TDRNRowChangeVector& changes );
    CORE::TDataTableStatus ApplyChanges( CDRNDataGrid& grid, const TDRNRowChangeVector& changes, CORE::TDataTableWriteMode mode, CORE::CDataTableWriteResult& result, GUCEF::vector< UInt32 >& proposedRows );
    CORE::TDataTableStatus ProposeDeleteRows( const CORE::CDataTableQuery& predicates, CDRNDataGridPendingWritePtr& pending );
    CORE::CFutureResult EnqueueWrite( const CDRNDataGridPendingWritePtr& pending );
    void StartPendingWrites( void );
    void EndPendingWrites( const CORE::CString& statusText );
    void OnProposeResult( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventData );
    void OnWriteTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventData );

    // following the grid
    void NoteInterest( CORE::CObserver* observer, UInt32 interest );
    void DropInterest( CORE::CObserver* observer, UInt32 interest );
    void UpdateGridSubscriptions( void );
    void OnGridUpdated( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventData );
    void OnGridKeyChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventData );
    void OnSessionDestroyed( void );

    CDRNSession* m_session;
    CDRNDataGrid* m_grid;
    CDRNDataGridTableSettings m_settings;
    CORE::CDataTableDefinition m_definition;
    CORE::CDataTableSchemaPtr m_schema;
    CORE::CDataTableDefinition m_scanDefinition;        /**< the same columns without primary key: what the copy of the grid is stored as */
    CORE::CDataTableSchemaPtr m_scanSchema;
    CORE::CDataTableBufferPool* m_pool;
    bool m_typed;
    CSessionWatcher m_sessionWatcher;
    CORE::CDataTableAsyncLinkPtr m_asyncLink;

    mutable MT::CMutex m_lock;                          /**< guards the state below up to the counters; never held while calling the grid */
    bool m_closed;
    mutable TLastErrorMap m_lastErrors;                 /**< per thread */
    mutable TScanTicksMap m_scanTicks;                  /**< per thread */
    UInt64 m_mismatchedValues;
    UInt64 m_mismatchedKeys;
    UInt64 m_snapshotCount;
    CORE::CMemoryDataTablePtr m_cachedSnapshot;
    UInt64 m_cachedSnapshotTime;                        /**< microseconds, CDRNClock */

    MT::CMutex m_subscriptionLock;                      /**< guards the observer count and the subscriptions at the grid */
    TObserverMap m_observers;
    bool m_followingRows;
    bool m_followingAnswers;
    bool m_subscribed;

    MT::CMutex m_writeLock;                             /**< serializes the synchronous changes */
    MT::CMutex m_pendingLock;                           /**< guards the pending writes, never held while calling the grid */
    TPendingWriteVector m_pendingWrites;
    CORE::CTimer* m_writeTimer;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CDRNDataGridTable, MT::CMutex >  CDRNDataGridTablePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNDATAGRIDTABLE_H ? */
