/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CRPCPROTOCOLTASK_H
#define GUCEF_COMCORE_CRPCPROTOCOLTASK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

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

class CRpcProtocolTask;
typedef CORE::CTSharedPtr< CRpcProtocolTask, MT::CMutex > CRpcProtocolTaskPtr;

/*-------------------------------------------------------------------------*/

/**
 *  A dedicated protocol thread obtained through the gucefCORE task manager (gucefCOMCORE RPC design 10.9): a
 *  singular, persistent task set up with CThreadPool::SetupSingularTask() and started with StartTaskWithConsumer()
 *  on a named pool. Transports that need a thread of their own run on its delegator's pulse generator. No raw threads.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcProtocolTask : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    /** Sets up and starts a protocol task on the named pool; NULL when the pool refused */
    static CRpcProtocolTaskPtr Start( const CORE::CString& threadPoolName );

    virtual ~CRpcProtocolTask() GUCEF_VIRTUAL_OVERRIDE;

    /** Asks the task to end and waits for it unless called on the task's own thread */
    void Stop( void );

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CRpcProtocolTask( void );
    CRpcProtocolTask( const CRpcProtocolTask& src );                /**< not implemented */
    CRpcProtocolTask& operator=( const CRpcProtocolTask& src );     /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCPROTOCOLTASK_H ? */
