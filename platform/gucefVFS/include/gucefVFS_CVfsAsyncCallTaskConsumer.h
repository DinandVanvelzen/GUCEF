/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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


#ifndef GUCEF_VFS_CVFSASYNCCALLTASKCONSUMER_H
#define GUCEF_VFS_CVFSASYNCCALLTASKCONSUMER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_VFS_CIVFSASYNCCALL_H
#include "gucefVFS_CIVfsAsyncCall.h"
#define GUCEF_VFS_CIVFSASYNCCALL_H
#endif /* GUCEF_VFS_CIVFSASYNCCALL_H ? */

#ifndef GUCEF_VFS_MACROS_H
#include "gucefVFS_macros.h"
#define GUCEF_VFS_MACROS_H
#endif /* GUCEF_VFS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Runs an asynchronous VFS call (CIVfsAsyncCall) on a thread of a pool and ends the future of the call afterwards.
 *  Registered by CVfsGlobal. The calls that wait for a remote place use the pool of the remote operations
 *  (see CVfsFetchScheduler::DefaultThreadPoolName), not the pool of the other asynchronous VFS operations.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsAsyncCallTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    /** The workers a pool is given when it is made for the first time by a call that finds it without any */
    static const UInt32 DefaultWorkerThreads;

    CVfsAsyncCallTaskConsumer( void );

    virtual ~CVfsAsyncCallTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Queues the call on the pool of that name (made when it is not there, with 'workerThreads' workers when it has none).
     *  A future without a task (HasNoFuture()) when it could not be queued: no pool, or a pool that takes no more work.
     */
    static CORE::CFutureResult Queue( const CORE::CString& threadPoolName, UInt32 workerThreads, const CIVfsAsyncCallPtr& call );

    private:

    CVfsAsyncCallTaskConsumer( const CVfsAsyncCallTaskConsumer& src );                  /**< not implemented */
    CVfsAsyncCallTaskConsumer& operator=( const CVfsAsyncCallTaskConsumer& src );       /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSASYNCCALLTASKCONSUMER_H ? */
