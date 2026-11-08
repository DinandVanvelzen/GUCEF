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

#ifndef GUCEF_CORE_CIDATATABLE_H
#define GUCEF_CORE_CIDATATABLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CDATATABLEDEFINITION_H
#include "gucefCORE_CDataTableDefinition.h"
#define GUCEF_CORE_CDATATABLEDEFINITION_H
#endif /* GUCEF_CORE_CDATATABLEDEFINITION_H ? */

#ifndef GUCEF_CORE_CDATATABLEQUERY_H
#include "gucefCORE_CDataTableQuery.h"
#define GUCEF_CORE_CDATATABLEQUERY_H
#endif /* GUCEF_CORE_CDATATABLEQUERY_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCH_H
#include "gucefCORE_CDataTableBatch.h"
#define GUCEF_CORE_CDATATABLEBATCH_H
#endif /* GUCEF_CORE_CDATATABLEBATCH_H ? */

#ifndef GUCEF_CORE_CDATATABLESCAN_H
#include "gucefCORE_CDataTableScan.h"
#define GUCEF_CORE_CDATATABLESCAN_H
#endif /* GUCEF_CORE_CDATATABLESCAN_H ? */

#ifndef GUCEF_CORE_CDATATABLEPARTITIONINFO_H
#include "gucefCORE_CDataTablePartitionInfo.h"
#define GUCEF_CORE_CDATATABLEPARTITIONINFO_H
#endif /* GUCEF_CORE_CDATATABLEPARTITIONINFO_H ? */

#ifndef GUCEF_CORE_CDATATABLEWRITERESULT_H
#include "gucefCORE_CDataTableWriteResult.h"
#define GUCEF_CORE_CDATATABLEWRITERESULT_H
#endif /* GUCEF_CORE_CDATATABLEWRITERESULT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The primary keys of the rows that changed, in the order the table lists them: the event data of CIDataTable::RowsChangedEvent */
typedef CTCloneableObj< GUCEF::vector< CVariant > >  TDataTableRowsChangedEventData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A typed, columnar, optionally partitioned set of rows (section 4.4 of the data tables design).
 *
 *  Sync and async pairs: the sync call does the work on the calling thread and never pumps a pulse generator; the
 *  async call queues a task on the given thread pool, or on "DataTables" from CTaskManager::GetOrCreateThreadPool when
 *  none is given, and returns a CFutureResult. Both run the same function. Result data (CDataTableWriteResult,
 *  CDataTablePartitionInfoList) comes with the completed future; failures end the future failed with the status text.
 *
 *  Events raised on pool threads reach observers on the thread of the pulse generator given at construction (the
 *  global one by default). Observers subscribe per event with CTEventHandlerFunctor.
 *
 *  Thread safe: scans run concurrently with each other and with writes; writes are serialized per table.
 *  The last error message is kept per calling thread.
 */
class GUCEF_CORE_PUBLIC_CPP CIDataTable : public CTSGNotifier
{
    public:

    static const CEvent PartitionWrittenEvent;     /**< CDataTablePartitionInfo */
    static const CEvent PartitionDeletedEvent;     /**< CDataTablePartitionInfo */
    static const CEvent RowsChangedEvent;          /**< TDataTableRowsChangedEventData: the keys of rows that changed behind the table's back (a replicated source); raised by tables that follow a changing source, and only while an observer is subscribed to it */

    static void RegisterEvents( void );

    /** Events reach observers on the thread of the pulse generator; the global one when it is NULL */
    explicit CIDataTable( PulseGeneratorPtr pulseGenerator );

    virtual ~CIDataTable() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetName( void ) const = 0;
    virtual const CDataTableDefinition& GetDefinition( void ) const = 0;
    virtual CDataTableSchemaPtr GetSchema( void ) const = 0;
    virtual bool IsReadOnly( void ) const = 0;

    virtual TDataTableStatus Scan( const CDataTableQuery& query, CDataTableScanPtr& scan ) = 0;
    virtual CFutureResult ScanAsync( const CDataTableQuery& query, CDataTableScanPtr& scan, ThreadPoolPtr threadPool = ThreadPoolPtr() ) = 0;

    virtual TDataTableStatus Write( const CDataTableBatch& rows, TDataTableWriteMode mode, CDataTableWriteResult& result ) = 0;
    virtual CFutureResult WriteAsync( const CDataTableBatchPtr& rows, TDataTableWriteMode mode, ThreadPoolPtr threadPool = ThreadPoolPtr() ) = 0;

    virtual TDataTableStatus DeleteRows( const CDataTableQuery& predicates, CDataTableWriteResult& result ) = 0;
    virtual CFutureResult DeleteRowsAsync( const CDataTableQuery& predicates, ThreadPoolPtr threadPool = ThreadPoolPtr() ) = 0;

    virtual TDataTableStatus DeletePartitions( const CDataTableQuery& partitionFilter, CDataTableWriteResult& result ) = 0;
    virtual CFutureResult DeletePartitionsAsync( const CDataTableQuery& partitionFilter, ThreadPoolPtr threadPool = ThreadPoolPtr() ) = 0;

    virtual TDataTableStatus ListPartitions( const CDataTableQuery& partitionFilter, bool withFileStats, CDataTablePartitionInfo::TPartitionInfoVector& partitions ) = 0;
    virtual CFutureResult ListPartitionsAsync( const CDataTableQuery& partitionFilter, bool withFileStats, ThreadPoolPtr threadPool = ThreadPoolPtr() ) = 0;

    virtual TDataTableStatus Compact( const CDataTableQuery& partitionFilter, bool force, CDataTableWriteResult& result ) = 0;
    virtual CFutureResult CompactAsync( const CDataTableQuery& partitionFilter, bool force, ThreadPoolPtr threadPool = ThreadPoolPtr() ) = 0;

    /** Forgets cached listings and file metadata, for files changed by another process */
    virtual void Refresh( void ) = 0;

    /** The message of the last failed call this thread made on the table */
    virtual CString GetLastErrorMessage( void ) const = 0;

    /**
     *  Presentation hints of a column (index into GetSchema()), for generic consumers such as a grid view: hint names
     *  are those of the presentation of a column (title, width, hidden, alignment, decimals, thousandsSeparator,
     *  prefix, suffix, dateFormat, booleanDisplay, nullText), values as text. Tables without hints return false
     *  (the default).
     */
    virtual bool GetColumnHints( UInt32 columnIndex, CValueList& hints ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CIDataTable( const CIDataTable& src );                  /**< not implemented */
    CIDataTable& operator=( const CIDataTable& src );       /**< not implemented */
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CIDataTable, MT::CMutex >  CIDataTablePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CIDATATABLE_H ? */
