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

#ifndef GUCEF_DRNUTILS_CTASKMANAGERSOURCE_H
#define GUCEF_DRNUTILS_CTASKMANAGERSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRNUTILS_CITASKMANAGERSOURCE_H
#include "gucefDRNUTILS_CITaskManagerSource.h"
#define GUCEF_DRNUTILS_CITASKMANAGERSOURCE_H
#endif /* GUCEF_DRNUTILS_CITASKMANAGERSOURCE_H ? */

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

/** CITaskManagerSource over the process task manager (CCoreGlobal::Instance()->GetTaskManager()) */
class GUCEF_DRNUTILS_PUBLIC_CPP CTaskManagerSource : public CITaskManagerSource
{
    public:

    explicit CTaskManagerSource( const CORE::CString& hostName );

    virtual ~CTaskManagerSource() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetHostName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 GetHostInstanceId( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual Int64 GetUtcTimeInMs( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 GetMonotonicTimeInMs( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetPoolCounters( CORE::CTaskManager::TThreadPoolCounterMap& counters ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetAllThreadPoolInfo( CORE::CTaskManager::TThreadPoolInfoMap& info ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetAllThreadInfo( CORE::CTaskManager::TThreadInfoMap& info ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetAllTaskInfo( CORE::CTaskManager::TTaskInfoMap& info ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetAllScheduledJobInfo( CORE::TScheduledJobInfoVector& info ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool ResizePool( const CORE::CString& poolName, UInt32 minWorkers, Int32 maxThreads, CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool CancelTask( UInt32 taskId, CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool StopTask( UInt32 taskId, CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetTaskPoolAndType( UInt32 taskId, CORE::CString& poolName, CORE::CString& taskTypeName ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CORE::CString m_hostName;
    UInt64 m_hostInstanceId;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CTASKMANAGERSOURCE_H ? */
