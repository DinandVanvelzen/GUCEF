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

#ifndef GUCEF_DRNUTILS_CITASKMANAGERSOURCE_H
#define GUCEF_DRNUTILS_CITASKMANAGERSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

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
 *  The seam between the remote layer and the task manager: CTaskManagerHost, the sampler and the publishers work
 *  against this interface (CTaskManagerSource for the process task manager, fakes in tests). Every call returns copies;
 *  task data is never part of them (GetAllTaskInfo without task data copies).
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CITaskManagerSource
{
    public:

    CITaskManagerSource( void );

    virtual ~CITaskManagerSource();

    /** Name of the node in views and grid names */
    virtual const CORE::CString& GetHostName( void ) const = 0;

    virtual UInt64 GetHostInstanceId( void ) const = 0;

    virtual Int64 GetUtcTimeInMs( void ) const = 0;

    virtual UInt64 GetMonotonicTimeInMs( void ) const = 0;

    virtual bool GetPoolCounters( CORE::CTaskManager::TThreadPoolCounterMap& counters ) const = 0;

    virtual bool GetAllThreadPoolInfo( CORE::CTaskManager::TThreadPoolInfoMap& info ) const = 0;

    virtual bool GetAllThreadInfo( CORE::CTaskManager::TThreadInfoMap& info ) const = 0;

    virtual bool GetAllTaskInfo( CORE::CTaskManager::TTaskInfoMap& info ) const = 0;

    virtual bool GetAllScheduledJobInfo( CORE::TScheduledJobInfoVector& info ) const = 0;

    virtual bool ResizePool( const CORE::CString& poolName, UInt32 minWorkers, Int32 maxThreads, CORE::CString& error ) = 0;

    virtual bool CancelTask( UInt32 taskId, CORE::CString& error ) = 0;

    virtual bool StopTask( UInt32 taskId, CORE::CString& error ) = 0;

    /** Pool and type of a task, for the capability checks of control actions */
    virtual bool GetTaskPoolAndType( UInt32 taskId, CORE::CString& poolName, CORE::CString& taskTypeName ) const = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CITASKMANAGERSOURCE_H ? */
