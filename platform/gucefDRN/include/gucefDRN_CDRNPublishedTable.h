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

#ifndef GUCEF_DRN_CDRNPUBLISHEDTABLE_H
#define GUCEF_DRN_CDRNPUBLISHEDTABLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATATABLE_H
#include "gucefCORE_CIDataTable.h"
#define GUCEF_CORE_CIDATATABLE_H
#endif /* GUCEF_CORE_CIDATATABLE_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CMEMORYDATATABLE_H
#include "gucefCORE_CMemoryDataTable.h"
#define GUCEF_CORE_CMEMORYDATATABLE_H
#endif /* GUCEF_CORE_CMEMORYDATATABLE_H ? */

#ifndef GUCEF_DRN_CDRNDATAGRID_H
#include "gucefDRN_CDRNDataGrid.h"
#define GUCEF_DRN_CDRNDATAGRID_H
#endif /* GUCEF_DRN_CDRNDATAGRID_H ? */

#ifndef GUCEF_DRN_CDRNPUBLISHEDTABLESCHEMA_H
#include "gucefDRN_CDRNPublishedTableSchema.h"
#define GUCEF_DRN_CDRNPUBLISHEDTABLESCHEMA_H
#endif /* GUCEF_DRN_CDRNPUBLISHEDTABLESCHEMA_H ? */

#ifndef GUCEF_DRN_CDRNPUBLISHEDTABLEMETA_H
#include "gucefDRN_CDRNPublishedTableMeta.h"
#define GUCEF_DRN_CDRNPUBLISHEDTABLEMETA_H
#endif /* GUCEF_DRN_CDRNPUBLISHEDTABLEMETA_H ? */

#ifndef GUCEF_DRN_CDRNPUBLISHEDTABLESETTINGS_H
#include "gucefDRN_CDRNPublishedTableSettings.h"
#define GUCEF_DRN_CDRNPUBLISHEDTABLESETTINGS_H
#endif /* GUCEF_DRN_CDRNPUBLISHEDTABLESETTINGS_H ? */

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
 *  A read only CORE::CIDataTable over a grid that carries a published table (CDRNTablePublisher): the replica of it,
 *  or the publisher's own grid. The columns come from "#schema", the page revision and scalars from "#meta"; every
 *  other key is a row. Rows come in key index order.
 *
 *  Optional and without cost when unused: a client of the public API of the grid; it follows the grid's events only
 *  while an observer listens to RowsChangedEvent. RowsChangedEvent is held back while an update has not arrived whole
 *  (IsConsistent(), deferUntilConsistent), at most maxDeferMs. A change of "#schema" raises SchemaChangedEvent.
 *  Rows are decoded without the node lock: the values of rows that changed are copied in chunks of
 *  scanChunkKeyCount keys under the node lock, then decoded; the decoded rows are kept per key with their grid revision
 *  and dropped after decodeCacheIdleMs without use. A scan may reuse the copy of the whole table for snapshotCacheMs.
 *  A "#schema" of more than CDRNPublishedTableSchema::MaxColumnCount columns, or one that is not valid, is refused; a row
 *  that does not decode is left out, counted (GetUndecodableRowCount()) and logged once per pass.
 *  Lifetime: the table can outlive the grid; once the session that holds the grid is gone it reports NOT_FOUND.
 */
class GUCEF_DRN_EXPORT_CPP CDRNPublishedTable : public CORE::CIDataTable
{
    public:

    static const CORE::CEvent SchemaChangedEvent;  /**< no data: GetSchema() and GetPublishedSchema() changed */

    static void RegisterEvents( void );

    CDRNPublishedTable( CDRNDataGrid& grid, const CDRNPublishedTableSettings& settings, CORE::PulseGeneratorPtr pulseGenerator = CORE::PulseGeneratorPtr() );

    virtual ~CDRNPublishedTable() GUCEF_VIRTUAL_OVERRIDE;

    const CDRNPublishedTableSettings& GetSettings( void ) const;

    /** "#schema" arrived and is of a version this reader knows */
    bool IsValid( void ) const;

    /** "#schema" is of a newer version than this reader knows: no columns, no rows */
    bool IsUnsupported( void ) const;

    /** The whole published schema: titles, widths, hidden columns, hints */
    bool GetPublishedSchema( CDRNPublishedTableSchema& schema ) const;

    bool GetMeta( CDRNPublishedTableMeta& meta ) const;
    UInt64 GetPageRevision( void ) const;
    UInt64 GetSourceVersion( void ) const;
    bool GetScalar( const CORE::CString& name, CORE::CVariant& value ) const;

    /**
     *  Whether the rows are those of one page revision: no row newer than #meta, the rows of the newest revision
     *  and the removals of it add up to its changed row count, and the row count matches
     */
    bool IsConsistent( void ) const;

    /** Cells that did not fit their column, shown as null */
    UInt64 GetMismatchedValueCount( void ) const;

    /** Rows whose value does not decode against the schema (they are left out), as of the last decode */
    UInt32 GetUndecodableRowCount( void ) const;

    /** Decoded rows the table keeps (settings.decodeCacheIdleMs) */
    UInt32 GetCachedRowCount( void ) const;

    /** The observers of the table registered at the grid, 0 while nobody listens */
    UInt32 GetGridSubscriptionCount( void ) const;

    virtual bool GetColumnHints( UInt32 columnIndex, CORE::CValueList& hints ) const GUCEF_VIRTUAL_OVERRIDE;

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
    virtual CORE::TDataTableStatus DeletePartitions( const CORE::CDataTableQuery& partitionFilter, CORE::CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult DeletePartitionsAsync( const CORE::CDataTableQuery& partitionFilter, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TDataTableStatus ListPartitions( const CORE::CDataTableQuery& partitionFilter, bool withFileStats, CORE::CDataTablePartitionInfo::TPartitionInfoVector& partitions ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult ListPartitionsAsync( const CORE::CDataTableQuery& partitionFilter, bool withFileStats, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TDataTableStatus Compact( const CORE::CDataTableQuery& partitionFilter, bool force, CORE::CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult CompactAsync( const CORE::CDataTableQuery& partitionFilter, bool force, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Refresh( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

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
        CDRNPublishedTable* m_table;
        CSessionWatcher( void ) : CORE::CObserver(), m_table( GUCEF_NULL ) {}
        virtual ~CSessionWatcher() { SignalUpcomingObserverDestruction(); }
        protected:
        virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;
    };
    friend class CSessionWatcher;

    class CRowCache
    {
        public:
        UInt32 gridRevision;            /**< of the grid slot when decoded */
        UInt64 rowRevision;
        bool decoded;                   /**< the value of gridRevision was decoded (present tells whether it could be) */
        bool present;
        CORE::CVariant key;
        CORE::CVariant::VariantVector cells;
        CRowCache( void ) : gridRevision( 0 ), rowRevision( 0 ), decoded( false ), present( false ), key(), cells() {}
    };

    /** A row value copied out of the grid under the node lock, decoded after it */
    class CCopiedRow
    {
        public:
        UInt32 index;
        UInt32 gridRevision;
        CORE::CVariant key;
        CORE::CVariant value;
        CCopiedRow( void ) : index( 0 ), gridRevision( 0 ), key(), value() {}
    };

    typedef GUCEF::vector< CRowCache > TRowCacheVector;
    typedef GUCEF::vector< CCopiedRow > TCopiedRowVector;
    typedef GUCEF::map< CORE::CObserver*, UInt32 > TObserverMap;
    typedef CORE::CTEventHandlerFunctor< CDRNPublishedTable > TEventCallback;

    CDRNPublishedTable( const CDRNPublishedTable& src );
    CDRNPublishedTable& operator=( const CDRNPublishedTable& src );

    CDRNDataGrid* AcquireGrid( void ) const;
    bool RefreshSchema( void ) const;
    static bool DecodeRow( const CORE::CVariant& value, UInt32 cellCount, CRowCache& row );
    bool UpdateCache( void ) const;
    void UseCache( void ) const;
    void ForgetRow( UInt32 index ) const;
    CORE::TDataTableStatus BuildCopy( CORE::CMemoryDataTablePtr& copy );
    void OnCacheTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventData );
    CORE::TDataTableStatus Fail( CORE::TDataTableStatus status, const CORE::CString& message ) const;
    void NoteInterest( CORE::CObserver* observer, UInt32 interest );
    void DropInterest( CORE::CObserver* observer, UInt32 interest );
    void UpdateGridSubscriptions( void );
    void OnGridUpdated( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventData );
    void OnGridKeyChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventData );
    void OnChangedIndices( const TDRNKeyIndexVector& indices );
    void OnFlushPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventData );
    CORE::PulseGeneratorPtr GetEventPulseGenerator( void ) const;
    void OnSessionDestroyed( void );

    CDRNDataGrid* m_grid;
    CDRNSession* m_session;
    CDRNPublishedTableSettings m_settings;
    CORE::CString m_name;
    CSessionWatcher m_sessionWatcher;

    mutable MT::CMutex m_lock;                          /**< guards what follows, taken after the node lock */
    mutable UInt32 m_schemaRevision;
    mutable bool m_haveSchema;
    mutable bool m_unsupported;
    mutable CDRNPublishedTableSchema m_published;
    mutable CORE::CDataTableDefinition m_definition;
    mutable CORE::CDataTableSchemaPtr m_schema;
    mutable TRowCacheVector m_rows;
    mutable UInt64 m_mismatchedValues;
    mutable UInt32 m_undecodableRows;
    mutable UInt64 m_lastCacheUse;                      /**< CDRNClock microseconds of the last use of the decoded rows */
    mutable CORE::CMemoryDataTablePtr m_snapshot;       /**< the copy of the whole table a scan reuses (snapshotCacheMs) */
    mutable UInt64 m_snapshotTime;
    mutable CORE::CString m_lastError;

    MT::CMutex m_subscriptionLock;
    TObserverMap m_observers;
    bool m_following;
    GUCEF::set< CORE::CVariant > m_deferredKeys;        /**< guarded by m_subscriptionLock */
    UInt64 m_deferredSince;                             /**< CDRNClock microseconds, 0 = nothing held back */
    bool m_flushPending;                                /**< the changes of a tick are raised together on the next pulse */
    CORE::CTimer* m_cacheTimer;                         /**< drops the decoded rows after decodeCacheIdleMs, running while there are any */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CDRNPublishedTable, MT::CMutex > CDRNPublishedTablePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNPUBLISHEDTABLE_H ? */
