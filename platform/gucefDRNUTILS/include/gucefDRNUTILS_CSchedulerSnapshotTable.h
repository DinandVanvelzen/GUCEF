/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CSCHEDULERSNAPSHOTTABLE_H
#define GUCEF_DRNUTILS_CSCHEDULERSNAPSHOTTABLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CIDATATABLE_H
#include "gucefCORE_CIDataTable.h"
#define GUCEF_CORE_CIDATATABLE_H
#endif /* GUCEF_CORE_CIDATATABLE_H ? */

#ifndef GUCEF_CORE_CMEMORYDATATABLE_H
#include "gucefCORE_CMemoryDataTable.h"
#define GUCEF_CORE_CMEMORYDATATABLE_H
#endif /* GUCEF_CORE_CMEMORYDATATABLE_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_DRNUTILS_CISCHEDULERSOURCE_H
#include "gucefDRNUTILS_CISchedulerSource.h"
#define GUCEF_DRNUTILS_CISCHEDULERSOURCE_H
#endif /* GUCEF_DRNUTILS_CISCHEDULERSOURCE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Module-private: a read-only CORE::CIDataTable over the jobs, lanes or runs of a CISchedulerSource, for a GUI in the
 *  same process without DRN (design 7.5). The rows are those of the published grids (same columns, hidden jobs left
 *  out, sensitive texts redacted, private item keys never shown). Zero cost while unobserved: the table follows the
 *  source only once an observer subscribed (then RowsChangedEvent carries the keys that changed); a scan reads a fresh
 *  copy when the source moved on. The source must outlive the table. Used on the thread of the given pulse generator.
 */
class CSchedulerSnapshotTable : public CORE::CIDataTable
{
    public:

    enum ETable { TABLE_JOBS = 0, TABLE_LANES = 1, TABLE_RUNS = 2 };

    CSchedulerSnapshotTable( CISchedulerSource& source, ETable table, UInt32 publishedRuns, const CORE::PulseGeneratorPtr& pulseGenerator );
    virtual ~CSchedulerSnapshotTable() GUCEF_VIRTUAL_OVERRIDE;

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

    virtual bool Subscribe( CORE::CObserver* observer ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Subscribe( CORE::CObserver* observer, const CORE::CEvent& eventid, CORE::CIEventHandlerFunctorBase* callback = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the table follows the source (an observer subscribed once) */
    bool IsFollowingSource( void ) const;

    /** How often the rows were read from the source */
    UInt32 GetFillCount( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CSchedulerSnapshotTable > TEventCallback;
    typedef GUCEF::map< CORE::CVariant, UInt64 > TRevisionMap;

    CSchedulerSnapshotTable( const CSchedulerSnapshotTable& src );
    CSchedulerSnapshotTable& operator=( const CSchedulerSnapshotTable& src );

    void FollowSource( void );
    void OnSourceChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void Fill( bool force );
    void TakeRows( const CORE::CJobSchedulerSnapshot& snapshot, GUCEF::vector< CORE::CVariant::VariantVector >& rows, TRevisionMap& revisions ) const;

    CISchedulerSource* m_source;
    ETable m_table;
    UInt32 m_publishedRuns;
    CORE::CDataTableDefinition m_definition;
    CORE::CDataTableSchemaPtr m_schema;
    CORE::CMemoryDataTable* m_rows;
    UInt64 m_filledRevision;
    UInt64 m_filledInstanceId;
    bool m_stale;
    bool m_following;
    UInt32 m_fillCount;
    TRevisionMap m_revisions;
    CORE::CString m_lastError;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CSCHEDULERSNAPSHOTTABLE_H ? */
