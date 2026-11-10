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

#ifndef GUCEF_CORE_CDATATABLESCAN_H
#define GUCEF_CORE_CDATATABLESCAN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef GUCEF_CORE_CDATATABLEQUERY_H
#include "gucefCORE_CDataTableQuery.h"
#define GUCEF_CORE_CDATATABLEQUERY_H
#endif /* GUCEF_CORE_CDATATABLEQUERY_H ? */

#ifndef GUCEF_CORE_CDATATABLESCANSTATS_H
#include "gucefCORE_CDataTableScanStats.h"
#define GUCEF_CORE_CDATATABLESCANSTATS_H
#endif /* GUCEF_CORE_CDATATABLESCANSTATS_H ? */

#ifndef GUCEF_CORE_CIDATATABLEBATCHSOURCE_H
#include "gucefCORE_CIDataTableBatchSource.h"
#define GUCEF_CORE_CIDATATABLEBATCHSOURCE_H
#endif /* GUCEF_CORE_CIDATATABLEBATCHSOURCE_H ? */

#ifndef GUCEF_CORE_CIDATATABLESCANSOURCE_H
#include "gucefCORE_CIDataTableScanSource.h"
#define GUCEF_CORE_CIDATATABLESCANSOURCE_H
#endif /* GUCEF_CORE_CIDATATABLESCANSOURCE_H ? */

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

class CDataTableScanState;

/**
 *  A scan over the files of a CIDataTableScanSource (section 6 of the data tables design). Concrete and shared by
 *  every backend: it owns scheduling, ordering, credits, cancellation, row group pruning and exact filtering; a
 *  backend only lists files and opens readers.
 *
 *  Sync scans (Start) decode on the calling thread inside NextBatch; nothing runs elsewhere and no pulse generator is
 *  pumped. Async scans (StartAsync) decode on a thread pool, at most maxParallelism row groups at a time and at most
 *  maxBatchesInFlight batches ahead of the consumer; the consumer pulls batches with NextBatch, TryNextBatch or
 *  WaitForNextBatch, and may wait for ScanBatchesAvailableEvent, which is coalesced: raised when the ring of ready
 *  batches goes from empty to not empty, not again until the consumer drained it.
 *
 *  Batches handed out stay valid while held, also after the scan or the table is gone. Destroying the scan cancels
 *  it; tasks hold a reference on the shared scan state, never on the scan object.
 *
 *  The consuming calls are made from one thread at a time; Cancel() from any thread.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableScan : public CTSGNotifier           ,
                                             public CIDataTableBatchSource
{
    public:

    static const CEvent ScanBatchesAvailableEvent;   /**< no data; at most one pending at a time */
    static const CEvent ScanFinishedEvent;           /**< CDataTableScanStats */
    static const CEvent ScanFailedEvent;             /**< TCloneableString: the error */

    static void RegisterEvents( void );

    /**
     *  A scan of the files the source lists, read as the table schema, with buffers and batches from the pool.
     *  Events are raised on the thread of the pulse generator, the global one when none is given.
     */
    CDataTableScan( const CDataTableScanSourcePtr& source       ,
                    const CDataTableSchemaPtr& tableSchema      ,
                    CDataTableBufferPool& bufferPool            ,
                    PulseGeneratorPtr pulseGenerator = PulseGeneratorPtr() );

    /** Cancels the scan when it still runs */
    virtual ~CDataTableScan() GUCEF_VIRTUAL_OVERRIDE;

    /** Resolves the query and prepares a sync scan; NextBatch then decodes on the calling thread */
    TDataTableStatus Start( const CDataTableQuery& query );

    /** Resolves the query and starts an async scan; the future ends when the whole scan ends (finished, failed or cancelled) */
    CFutureResult StartAsync( const CDataTableQuery& query, ThreadPoolPtr threadPool = ThreadPoolPtr() );

    /** Sync scans: decodes the next batch on the calling thread. Async scans: waits for the next batch. OK, END, CANCELLED or an error */
    virtual TDataTableStatus NextBatch( CDataTableBatchPtr& batch ) GUCEF_VIRTUAL_OVERRIDE;

    /** Async scans: DATATABLE_STATUS_PENDING when no batch is ready, never waits */
    TDataTableStatus TryNextBatch( CDataTableBatchPtr& batch );

    /** Async scans: waits at most the timeout for the next batch, PENDING when it passes */
    TDataTableStatus WaitForNextBatch( CDataTableBatchPtr& batch, UInt32 timeoutInMs );

    /** Any thread: stops scheduling; running tasks stop at their next batch boundary */
    void Cancel( void );

    bool IsAsync( void ) const;

    void GetStats( CDataTableScanStats& stats ) const;

    virtual CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CDataTableScanState;

    CDataTableScan( const CDataTableScan& src );                /**< not implemented */
    CDataTableScan& operator=( const CDataTableScan& src );     /**< not implemented */

    CDataTableScanState* m_state;                               /**< scheduler state, shared with the scan tasks */
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CDataTableScan, MT::CMutex >   CDataTableScanPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLESCAN_H ? */
