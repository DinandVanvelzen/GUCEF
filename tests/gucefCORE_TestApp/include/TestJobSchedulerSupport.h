/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_TESTAPP_TESTJOBSCHEDULERSUPPORT_H
#define GUCEF_CORE_TESTAPP_TESTJOBSCHEDULERSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULER_H
#include "gucefCORE_CJobScheduler.h"
#define GUCEF_CORE_CJOBSCHEDULER_H
#endif /* GUCEF_CORE_CJOBSCHEDULER_H ? */

#ifndef GUCEF_CORE_CIJOBSCHEDULERSTATESTORE_H
#include "gucefCORE_CIJobSchedulerStateStore.h"
#define GUCEF_CORE_CIJOBSCHEDULERSTATESTORE_H
#endif /* GUCEF_CORE_CIJOBSCHEDULERSTATESTORE_H ? */

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H
#include "gucefCORE_CIDataNodeSerializableTaskData.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H ? */

#include "TestManualClocks.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/**
 *  Test seam of CJobScheduler (a friend of it): dispatch cycles carried out on the test thread instead of by the
 *  dispatcher task, so tests on manual clocks see every cycle and the delay it plans; and the dispatcher's figures
 */
class CJobSchedulerTestAccess
{
    public:

    /** As Start(), without a dispatcher task: the test calls Cycle() */
    static bool StartManual( CJobScheduler& scheduler, CString& error ) { return scheduler.StartImpl( false, error ); }

    /** One dispatcher cycle on the calling thread; the delay it plans until the next one (ms), -1 when stopping */
    static Int64 Cycle( CJobScheduler& scheduler ) { return scheduler.DispatchCycle(); }

    static UInt64 GetCycleCount( CJobScheduler& scheduler ) { MT::CScopeMutex lock( scheduler.m_lock ); return scheduler.m_cycleCount; }
    static UInt64 GetClockJumpCount( CJobScheduler& scheduler ) { MT::CScopeMutex lock( scheduler.m_lock ); return scheduler.m_clockJumpCount; }
    static UInt64 GetDispatcherSystemThreadId( CJobScheduler& scheduler ) { MT::CScopeMutex lock( scheduler.m_lock ); return scheduler.m_dispatcherSystemThreadId; }
    static UInt64 GetLastPlannedDelayMs( CJobScheduler& scheduler ) { MT::CScopeMutex lock( scheduler.m_lock ); return scheduler.m_lastPlannedDelayMs; }
    static UInt32 GetEndedTasksWaiting( CJobScheduler& scheduler ) { MT::CScopeMutex lock( scheduler.m_lock ); return (UInt32) scheduler.m_endedTasks.size(); }
    static UInt32 GetPendingChangeCount( CJobScheduler& scheduler ) { MT::CScopeMutex lock( scheduler.m_lock ); return (UInt32) ( scheduler.m_pendingChanges.size() + scheduler.m_pendingRunChanges.size() ); }
    static UInt32 GetHistoryCount( CJobScheduler& scheduler ) { MT::CScopeMutex lock( scheduler.m_lock ); return scheduler.m_history.GetCount(); }
    static UInt32 GetJobHistoryCount( CJobScheduler& scheduler, const CString& jobId )
    {
        MT::CScopeMutex lock( scheduler.m_lock );
        CJobScheduler::TJobMap::iterator j = scheduler.m_jobs.find( jobId );
        return j == scheduler.m_jobs.end() ? 0 : (*j).second.recentRuns.GetCount();
    }
    static bool IsDispatcherActive( CJobScheduler& scheduler ) { MT::CScopeMutex lock( scheduler.m_lock ); return scheduler.m_dispatcherActive; }
};

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

namespace CORETEST {

/** What the test task data carries: a value to record and how the run behaves */
class CJsTestTaskData : public GUCEF::CORE::CIDataNodeSerializableTaskData
{
    public:

    static const GUCEF::CORE::CString TaskType;
    static const char* CloneableTypeName( void ) { return "CORETEST::CJsTestTaskData"; }

    GUCEF::CORE::CString value;         /**< recorded in the run log */
    GUCEF::UInt32 cycles;               /**< cycles before the task finishes; 0 = until cancelled */
    GUCEF::UInt32 blockMs;              /**< the first cycle keeps its thread this long */
    bool fail;                          /**< the start fails: the run FAILS */
    GUCEF::CORE::CString note;          /**< status extra info the task sets */

    CJsTestTaskData( void );
    CJsTestTaskData( const CJsTestTaskData& src );
    virtual ~CJsTestTaskData() GUCEF_VIRTUAL_OVERRIDE;

    virtual const GUCEF::CORE::CString& GetTaskTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const GUCEF::CORE::CString& GetThreadPoolName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetOnlyUseExistingThreadPool( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetTaskCanBeQueued( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::UInt32 GetTaskId( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Serialize( GUCEF::CORE::CDataNode& domRootNode, const GUCEF::CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Deserialize( const GUCEF::CORE::CDataNode& domRootNode, const GUCEF::CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** One run of the test task as it happened */
struct SJsRunLogEntry
{
    GUCEF::CORE::CString value;
    GUCEF::UInt32 taskId;
    GUCEF::UInt64 systemThreadId;
    GUCEF::UInt64 startedRealMs;        /**< CSystemMonotonicClock */
    GUCEF::UInt64 endedRealMs;
    bool hadTaskData;
};
typedef GUCEF::vector< SJsRunLogEntry > TJsRunLog;

/*-------------------------------------------------------------------------*/

/** A state store in memory: counts the saves, can be made to fail them */
class CMemoryJobStateStore : public GUCEF::CORE::CIJobSchedulerStateStore
{
    public:

    CMemoryJobStateStore( void );
    virtual ~CMemoryJobStateStore() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadState( GUCEF::CORE::CDataNode& state, GUCEF::CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SaveState( const GUCEF::CORE::CDataNode& state, GUCEF::CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE;

    GUCEF::UInt32 GetSaveCount( void ) const;
    GUCEF::UInt32 GetLoadCount( void ) const;
    void SetFailSaves( GUCEF::UInt32 failNextSaves );
    bool GetState( GUCEF::CORE::CDataNode& state ) const;

    private:

    mutable GUCEF::MT::CMutex m_lock;
    GUCEF::CORE::CDataNode m_state;
    bool m_hasState;
    GUCEF::UInt32 m_saves;
    GUCEF::UInt32 m_loads;
    GUCEF::UInt32 m_failNextSaves;
};

/*-------------------------------------------------------------------------*/

/** The test task type: registers the consumer and task data factories with the task manager */
void RegisterJsTestTaskType( void );
void UnregisterJsTestTaskType( void );

void ClearJsRunLog( void );
void GetJsRunLog( TJsRunLog& log );
GUCEF::UInt32 CountJsRuns( const GUCEF::CORE::CString& value );

/** 2026-01-01T00:00:00Z plus an offset */
GUCEF::Int64 JsUtc( GUCEF::Int64 hours, GUCEF::Int64 minutes = 0, GUCEF::Int64 seconds = 0 );

/** A job of the test task type with the value in its payload */
GUCEF::CORE::CJobDefinition MakeJsJob( const GUCEF::CORE::CString& id, const GUCEF::CORE::CJobTrigger& trigger, const GUCEF::CORE::CString& value,
                                       GUCEF::UInt32 cycles = 1, GUCEF::UInt32 blockMs = 0, bool fail = false );

/** Settings of a test scheduler: pools of its own (named after the host), short stop timeout */
GUCEF::CORE::CJobSchedulerSettings MakeJsSettings( const GUCEF::CORE::CString& hostName );

/** Through the task manager, with the given clocks (NULL = system) and a main pulse generator of the test */
GUCEF::CORE::JobSchedulerPtr MakeJsScheduler( const GUCEF::CORE::CJobSchedulerSettings& settings, CManualClocks* clocks,
                                             GUCEF::CORE::PulseGeneratorPtr mainPulse = GUCEF::CORE::PulseGeneratorPtr() );

/** Manual dispatch: cycles on this thread until no run is active (bounded); whether it got there */
bool CycleUntilIdle( GUCEF::CORE::CJobScheduler& scheduler, GUCEF::UInt32 timeoutMs = 10000 );

/** Real dispatcher: waits (bounded) until no run is active */
bool WaitUntilIdle( GUCEF::CORE::CJobScheduler& scheduler, GUCEF::UInt32 timeoutMs = 10000 );

/** The runs of a job in the snapshot, newest first */
void GetJobRuns( GUCEF::CORE::CJobScheduler& scheduler, const GUCEF::CORE::CString& jobId, GUCEF::CORE::TJobRunRecordVector& runs );

/** Runs of the job that ended in the state */
GUCEF::UInt32 CountJobRunsInState( GUCEF::CORE::CJobScheduler& scheduler, const GUCEF::CORE::CString& jobId, GUCEF::CORE::TJobRunState state );

GUCEF::CORE::SJobStatus GetJobStatus( GUCEF::CORE::CJobScheduler& scheduler, const GUCEF::CORE::CString& jobId );

/** Stops and unregisters the scheduler */
void DropJsScheduler( GUCEF::CORE::JobSchedulerPtr& scheduler );

}; /* namespace CORETEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTJOBSCHEDULERSUPPORT_H ? */
