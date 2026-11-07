/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_COMCORE_CDATABASECLIENTBASE_H
#define GUCEF_COMCORE_CDATABASECLIENTBASE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_COMCORE_CIDATABASECLIENT_H
#include "gucefCOMCORE_CIDatabaseClient.h"
#define GUCEF_COMCORE_CIDATABASECLIENT_H
#endif /* GUCEF_COMCORE_CIDATABASECLIENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The async calls of a database client, made from its sync calls, for backends that have no async form of their own.
 *  A backend implements the sync calls; each async call queues a task on the thread pool of the settings (or the pool
 *  of the name in the settings, "DataTables" by default) which invokes the same sync call on a pool thread and completes
 *  the future with the result data. The sync calls never touch a thread pool and never pump a pulse generator.
 *
 *  A backend calls WaitForAsyncOperations() in Close() and at the start of its destructor: the queued and running
 *  operations call into the backend, which must still be whole when they do.
 */
class GUCEF_COMCORE_EXPORT_CPP CDatabaseClientBase : public CIDatabaseClient
{
    public:

    /** The sync calls that have an async form */
    enum EOperation
    {
        OPERATION_OPEN ,
        OPERATION_LIST_TABLES ,
        OPERATION_OPEN_TABLE ,
        OPERATION_CREATE_TABLE ,
        OPERATION_DROP_TABLE
    };

    CDatabaseClientBase( void );

    virtual ~CDatabaseClientBase() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CFutureResult OpenAsync( const CDatabaseClientSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CFutureResult ListTablesAsync( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CFutureResult OpenTableAsync( const CORE::CString& tableName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CFutureResult CreateTableAsync( const CORE::CDataTableDefinition& definition ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CFutureResult DropTableAsync( const CORE::CString& tableName, bool deleteData ) GUCEF_VIRTUAL_OVERRIDE;

    /** Async operations that are queued or running */
    UInt32 GetPendingAsyncOperationCount( void ) const;

    protected:

    /** Waits until no async operation of this client is queued or running; false after the timeout. Not to be called from a pool thread of this client */
    bool WaitForAsyncOperations( UInt32 timeoutInMs = 60000 );

    private:
    friend class CDatabaseClientOperation;

    /** Runs the sync call of the operation on the calling thread (a pool thread) and completes the future */
    void RunOperation( EOperation operation, const CDatabaseClientSettings& settings, const CORE::CString& tableName,
                       const CORE::CDataTableDefinition& definition, bool deleteData, CORE::CFutureResultSource& source );

    CORE::CFutureResult QueueOperation( EOperation operation, const CDatabaseClientSettings& settings, const CORE::CString& tableName,
                                        const CORE::CDataTableDefinition& definition, bool deleteData );

    void OperationEnded( void );

    CDatabaseClientBase( const CDatabaseClientBase& src );              /**< not implemented */
    CDatabaseClientBase& operator=( const CDatabaseClientBase& src );   /**< not implemented */

    private:

    MT::CTAtomic< UInt32 > m_pendingOperations;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CDATABASECLIENTBASE_H ? */
