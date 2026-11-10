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

#ifndef GUCEF_CORE_CDATATABLEASYNCOPERATION_H
#define GUCEF_CORE_CDATATABLEASYNCOPERATION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_CIDATATABLE_H
#include "gucefCORE_CIDataTable.h"
#define GUCEF_CORE_CIDATATABLE_H
#endif /* GUCEF_CORE_CIDATATABLE_H ? */

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

/**
 *  What the async operations of a table run against. The table makes one, hands it to every operation it queues
 *  and detaches it in its destructor: Detach waits for an operation that is running, and operations that start
 *  later find no table and fail. An operation holds the link while it runs, so the operations of one table run one
 *  at a time, as its writes are serialized anyway.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableAsyncLink
{
    public:

    explicit CDataTableAsyncLink( CIDataTable* table );

    ~CDataTableAsyncLink();

    /** The table goes: waits for a running operation, later ones find no table */
    void Detach( void );

    /** The table for one operation, GUCEF_NULL once it is detached; every call is paired with Unlock */
    CIDataTable* Lock( void );

    void Unlock( void );

    private:

    CDataTableAsyncLink( const CDataTableAsyncLink& src );              /**< not implemented */
    CDataTableAsyncLink& operator=( const CDataTableAsyncLink& src );   /**< not implemented */

    MT::CMutex m_lock;
    CIDataTable* m_table;
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CDataTableAsyncLink, MT::CMutex >   CDataTableAsyncLinkPtr;

/*-------------------------------------------------------------------------*/

/** The task data of an async table operation: the operation, its arguments and the future it completes */
class GUCEF_CORE_PUBLIC_CPP CDataTableAsyncOperationData : public CICloneable
{
    public:

    enum EOperation
    {
        OPERATION_WRITE = 0             ,
        OPERATION_DELETE_ROWS           ,
        OPERATION_DELETE_PARTITIONS     ,
        OPERATION_LIST_PARTITIONS       ,
        OPERATION_COMPACT
    };

    CDataTableAsyncLinkPtr table;
    EOperation operation;
    CDataTableBatchPtr rows;                /**< OPERATION_WRITE */
    TDataTableWriteMode writeMode;          /**< OPERATION_WRITE */
    CDataTableQuery query;                  /**< the predicates or the partition filter */
    bool flag;                              /**< withFileStats of OPERATION_LIST_PARTITIONS, force of OPERATION_COMPACT */
    CFutureResultSource result;             /**< completed with a CDataTableWriteResult or a CDataTablePartitionInfoList */

    CDataTableAsyncOperationData( void );

    CDataTableAsyncOperationData( const CDataTableAsyncOperationData& src );

    virtual ~CDataTableAsyncOperationData() GUCEF_VIRTUAL_OVERRIDE;

    CDataTableAsyncOperationData& operator=( const CDataTableAsyncOperationData& src );

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  The task that runs the sync call of a table operation on a pool thread for its async form (4.4). Success
 *  completes the future with the result data; failure ends it failed with "<STATUS>: <message>" as extra info,
 *  the message being the table's last error on the pool thread.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableAsyncOperation : public CTaskConsumer
{
    public:

    static const CString TaskType;

    /** The pool async data table work uses when none is given */
    static const CString DefaultThreadPoolName;

    /**
     *  Queues the operation on the pool, or on DefaultThreadPoolName from CTaskManager::GetOrCreateThreadPool when
     *  none is given, and returns the future of data.result; a future already failed when queueing fails.
     */
    static CFutureResult Queue( const CDataTableAsyncOperationData& data, ThreadPoolPtr threadPool );

    CDataTableAsyncOperation( void );

    virtual ~CDataTableAsyncOperation() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDataTableAsyncOperation( const CDataTableAsyncOperation& src );            /**< not implemented */
    CDataTableAsyncOperation& operator=( const CDataTableAsyncOperation& src ); /**< not implemented */
};

/*-------------------------------------------------------------------------*/

typedef CTFactory< CTaskConsumer, CDataTableAsyncOperation, MT::CMutex > TDataTableAsyncOperationTaskFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLEASYNCOPERATION_H ? */
