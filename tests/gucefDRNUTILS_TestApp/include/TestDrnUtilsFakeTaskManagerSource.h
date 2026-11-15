/*
 *  gucefDRNUTILS_TestApp: GUCEF test application for gucefDRNUTILS
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

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKETASKMANAGERSOURCE_H
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKETASKMANAGERSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

#ifndef GUCEF_DRNUTILS_CITASKMANAGERSOURCE_H
#include "gucefDRNUTILS_CITaskManagerSource.h"
#define GUCEF_DRNUTILS_CITASKMANAGERSOURCE_H
#endif /* GUCEF_DRNUTILS_CITASKMANAGERSOURCE_H ? */

namespace DRNUTILSTEST {

using namespace GUCEF;

/**
 *  A task manager as data: pools with counters, threads, tasks and scheduled jobs; the control calls are recorded;
 *  every call is counted so that a test can see whether (and how often) the sampler asks
 */
class CFakeTaskManagerSource : public DRNUTILS::CITaskManagerSource
{
    public:

    struct SResize
    {
        CORE::CString pool;
        UInt32 minWorkers;
        Int32 maxThreads;
    };

    CORE::CString hostName;
    UInt64 instanceId;
    Int64 utcMs;
    UInt64 monotonicMs;
    bool useRealTime;
    CORE::CTaskManager::TThreadPoolInfoMap pools;
    CORE::CTaskManager::TThreadPoolCounterMap counters;
    CORE::CTaskManager::TThreadInfoMap threads;
    CORE::CTaskManager::TTaskInfoMap tasks;
    CORE::TScheduledJobInfoVector scheduledJobs;
    GUCEF::vector< SResize > resizes;
    GUCEF::vector< UInt32 > cancelled;
    GUCEF::vector< UInt32 > stopped;
    UInt32 counterCalls;
    UInt32 taskCalls;
    mutable MT::CMutex lock;

    explicit CFakeTaskManagerSource( const CORE::CString& name = "node1" )
        : DRNUTILS::CITaskManagerSource(), hostName( name ), instanceId( 77 ), utcMs( 1767225600000LL ), monotonicMs( 500000 ), useRealTime( true )
        , pools(), counters(), threads(), tasks(), scheduledJobs(), resizes(), cancelled(), stopped(), counterCalls( 0 ), taskCalls( 0 ), lock()
    {}

    void AddPool( const CORE::CString& name, UInt32 workers )
    {
        MT::CScopeMutex scope( lock );
        CORE::CThreadPoolInfo info;
        info.SetThreadPoolName( name );
        info.SetActiveNrOfWorkerThreads( workers );
        info.SetDesiredMinNrOfWorkerThreads( workers );
        info.SetDesiredMaxNrOfThreads( (Int32) workers * 2 );
        pools[ name ] = info;
        CORE::CThreadPoolCounters counter;
        counter.SetThreadPoolName( name );
        counters[ name ] = counter;
    }

    void AddTask( UInt32 taskId, const CORE::CString& pool, const CORE::CString& taskType, UInt64 thread )
    {
        MT::CScopeMutex scope( lock );
        CORE::CTaskInfo info;
        info.SetTaskId( taskId );
        info.SetThreadPoolName( pool );
        info.SetTaskTypeName( taskType );
        info.SetTaskStatus( CORE::TASKSTATUS_RUNNING );
        info.SetThreadId( (TSystemThreadId) thread );
        info.SetQueuedAtMs( 1000 );
        info.SetStartedAtMs( 2000 );
        tasks[ taskId ] = info;
        CORE::CThreadInfo threadInfo;
        threadInfo.SetThreadId( (TSystemThreadId) thread );
        threadInfo.SetThreadName( pool + "#1" );
        threadInfo.SetThreadPoolName( pool );
        threadInfo.SetCurrentTaskId( taskId );
        threadInfo.SetCurrentTaskTypeName( taskType );
        threads[ (TSystemThreadId) thread ] = threadInfo;
    }

    void RemoveTask( UInt32 taskId )
    {
        MT::CScopeMutex scope( lock );
        tasks.erase( taskId );
    }

    void AddFinished( const CORE::CString& pool, UInt64 finished, UInt64 runMs )
    {
        MT::CScopeMutex scope( lock );
        CORE::CThreadPoolCounters& counter = counters[ pool ];
        counter.SetStartedTotal( counter.GetStartedTotal() + finished );
        counter.SetFinishedTotal( counter.GetFinishedTotal() + finished );
        counter.SetCumulativeRunMs( counter.GetCumulativeRunMs() + runMs );
    }

    UInt32 GetCounterCalls( void ) const { MT::CScopeMutex scope( lock ); return counterCalls; }
    UInt32 GetTaskCalls( void ) const { MT::CScopeMutex scope( lock ); return taskCalls; }

    virtual const CORE::CString& GetHostName( void ) const GUCEF_VIRTUAL_OVERRIDE { return hostName; }
    virtual UInt64 GetHostInstanceId( void ) const GUCEF_VIRTUAL_OVERRIDE { return instanceId; }
    virtual Int64 GetUtcTimeInMs( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( useRealTime )
            return (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
        MT::CScopeMutex scope( lock );
        return utcMs;
    }
    virtual UInt64 GetMonotonicTimeInMs( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( useRealTime )
            return CORE::CSystemMonotonicClock().GetMonotonicTimeInMs();
        MT::CScopeMutex scope( lock );
        return monotonicMs;
    }
    virtual bool GetPoolCounters( CORE::CTaskManager::TThreadPoolCounterMap& result ) const GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex scope( lock );
        const_cast< CFakeTaskManagerSource* >( this )->counterCalls++;
        result = counters;
        return true;
    }
    virtual bool GetAllThreadPoolInfo( CORE::CTaskManager::TThreadPoolInfoMap& result ) const GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex scope( lock );
        result = pools;
        return true;
    }
    virtual bool GetAllThreadInfo( CORE::CTaskManager::TThreadInfoMap& result ) const GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex scope( lock );
        result = threads;
        return true;
    }
    virtual bool GetAllTaskInfo( CORE::CTaskManager::TTaskInfoMap& result ) const GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex scope( lock );
        const_cast< CFakeTaskManagerSource* >( this )->taskCalls++;
        result = tasks;
        return true;
    }
    virtual bool GetAllScheduledJobInfo( CORE::TScheduledJobInfoVector& result ) const GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex scope( lock );
        result = scheduledJobs;
        return true;
    }
    virtual bool ResizePool( const CORE::CString& poolName, UInt32 minWorkers, Int32 maxThreads, CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex scope( lock );
        SResize resize;
        resize.pool = poolName;
        resize.minWorkers = minWorkers;
        resize.maxThreads = maxThreads;
        resizes.push_back( resize );
        return true;
    }
    virtual bool CancelTask( UInt32 taskId, CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex scope( lock );
        cancelled.push_back( taskId );
        return true;
    }
    virtual bool StopTask( UInt32 taskId, CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex scope( lock );
        stopped.push_back( taskId );
        return true;
    }
    virtual bool GetTaskPoolAndType( UInt32 taskId, CORE::CString& poolName, CORE::CString& taskTypeName ) const GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex scope( lock );
        CORE::CTaskManager::TTaskInfoMap::const_iterator i = tasks.find( taskId );
        if ( i == tasks.end() )
            return false;
        poolName = (*i).second.GetThreadPoolName();
        taskTypeName = (*i).second.GetTaskTypeName();
        return true;
    }
};

}; /* namespace DRNUTILSTEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKETASKMANAGERSOURCE_H ? */
