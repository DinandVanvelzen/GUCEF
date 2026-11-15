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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "TestJobSchedulerSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace CORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static MT::CMutex g_jsRunLogLock;
static TJsRunLog g_jsRunLog;

const CORE::CString CJsTestTaskData::TaskType = "JsTestTask";

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The test task: records its run, cycles as its data says, cooperative (ends at a cancel between cycles) */
class CJsTestTask : public CORE::CTaskConsumer
{
    public:

    CJsTestTask( void ) : CORE::CTaskConsumer(), m_cyclesDone( 0 ), m_logIndex( 0 ) {}
    virtual ~CJsTestTask() GUCEF_VIRTUAL_OVERRIDE {}

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return CJsTestTaskData::TaskType; }
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return CJsTestTaskData::TaskType; }

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_cyclesDone = 0;
        const CJsTestTaskData* data = task.IsNULL() ? GUCEF_NULL : MT::CICloneable::CloneableAs< CJsTestTaskData >( task->GetTaskData() );
        SJsRunLogEntry entry;
        entry.value = GUCEF_NULL != data ? data->value : CORE::CString( "(no data)" );
        entry.taskId = task.IsNULL() ? 0 : task->GetTaskId();
        entry.systemThreadId = (UInt64) MT::GetCurrentThreadSystemID();
        entry.startedRealMs = CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
        entry.endedRealMs = 0;
        entry.hadTaskData = GUCEF_NULL != data;
        {
            MT::CScopeMutex lock( g_jsRunLogLock );
            m_logIndex = (UInt32) g_jsRunLog.size();
            g_jsRunLog.push_back( entry );
        }
        if ( GUCEF_NULL != data && !data->note.IsNULLOrEmpty() )
            task->SetTaskStatusExtraInfo( data->note );
        return GUCEF_NULL == data || !data->fail;
    }

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        const CJsTestTaskData* data = task.IsNULL() ? GUCEF_NULL : MT::CICloneable::CloneableAs< CJsTestTaskData >( task->GetTaskData() );
        if ( 0 == m_cyclesDone && GUCEF_NULL != data && data->blockMs > 0 )
            MT::ThreadDelay( data->blockMs );
        ++m_cyclesDone;
        UInt32 cycles = GUCEF_NULL != data ? data->cycles : 1;
        if ( 0 != cycles && m_cyclesDone >= cycles )
            return true;
        RequestTaskCycleDelayInMs( 2 );
        return false;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE
    {
        {
            MT::CScopeMutex lock( g_jsRunLogLock );
            if ( m_logIndex < g_jsRunLog.size() )
                g_jsRunLog[ m_logIndex ].endedRealMs = CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
        }
        CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
    }

    private:

    UInt32 m_cyclesDone;
    UInt32 m_logIndex;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CORE::CTaskConsumer, CJsTestTask, MT::CMutex > TJsTaskFactory;
typedef CORE::CTFactory< CORE::CIDataNodeSerializableTaskData, CJsTestTaskData, MT::CMutex > TJsTaskDataFactory;
static TJsTaskFactory g_jsTaskFactory;
static TJsTaskDataFactory g_jsTaskDataFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CJsTestTaskData::CJsTestTaskData( void )
    : CORE::CIDataNodeSerializableTaskData()
    , value()
    , cycles( 1 )
    , blockMs( 0 )
    , fail( false )
    , note()
{
}

CJsTestTaskData::CJsTestTaskData( const CJsTestTaskData& src )
    : CORE::CIDataNodeSerializableTaskData( src )
    , value( src.value )
    , cycles( src.cycles )
    , blockMs( src.blockMs )
    , fail( src.fail )
    , note( src.note )
{
}

CJsTestTaskData::~CJsTestTaskData() {}

const CORE::CString& CJsTestTaskData::GetTaskTypeName( void ) const { return TaskType; }
const CORE::CString& CJsTestTaskData::GetThreadPoolName( void ) const { return CORE::CString::Empty; }
bool CJsTestTaskData::GetOnlyUseExistingThreadPool( void ) const { return false; }
bool CJsTestTaskData::GetTaskCanBeQueued( void ) const { return true; }
UInt32 CJsTestTaskData::GetTaskId( void ) const { return 0; }
CORE::CICloneable* CJsTestTaskData::Clone( void ) const { return GUCEF_NEW CJsTestTaskData( *this ); }
const CORE::CString& CJsTestTaskData::GetClassTypeName( void ) const { static const CORE::CString name = CloneableTypeName(); return name; }
const char* CJsTestTaskData::GetCloneableTypeName( void ) const { return CloneableTypeName(); }

bool
CJsTestTaskData::Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const
{
    domRootNode.SetAttribute( "value", value );
    domRootNode.SetAttribute( "cycles", cycles );
    domRootNode.SetAttribute( "blockMs", blockMs );
    domRootNode.SetAttribute( "fail", fail );
    domRootNode.SetAttribute( "note", note );
    return true;
}

bool
CJsTestTaskData::Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings )
{
    // a payload without a value is not task data of this type
    CORE::CVariant valueVar = domRootNode.GetAttributeValueOrChildValueByName( "value" );
    if ( valueVar.IsNULLOrEmpty() )
        return false;
    value = valueVar.AsString( CORE::CString::Empty, true );
    cycles = domRootNode.GetAttributeValueOrChildValueByName( "cycles" ).AsUInt32( 1, true );
    blockMs = domRootNode.GetAttributeValueOrChildValueByName( "blockMs" ).AsUInt32( 0, true );
    fail = domRootNode.GetAttributeValueOrChildValueByName( "fail" ).AsBool( false, true );
    note = domRootNode.GetAttributeValueOrChildValueByName( "note" ).AsString( CORE::CString::Empty, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CMemoryJobStateStore::CMemoryJobStateStore( void )
    : CORE::CIJobSchedulerStateStore()
    , m_lock()
    , m_state()
    , m_hasState( false )
    , m_saves( 0 )
    , m_loads( 0 )
    , m_failNextSaves( 0 )
{
}

CMemoryJobStateStore::~CMemoryJobStateStore() {}

bool
CMemoryJobStateStore::LoadState( CORE::CDataNode& state, CORE::CString& error )
{
    MT::CScopeMutex lock( m_lock );
    ++m_loads;
    state.Clear();
    if ( m_hasState )
        state = m_state;
    return true;
}

bool
CMemoryJobStateStore::SaveState( const CORE::CDataNode& state, CORE::CString& error )
{
    MT::CScopeMutex lock( m_lock );
    if ( m_failNextSaves > 0 )
    {
        --m_failNextSaves;
        error = "the test store fails this save";
        return false;
    }
    ++m_saves;
    m_state = state;
    m_hasState = true;
    return true;
}

UInt32 CMemoryJobStateStore::GetSaveCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_saves; }
UInt32 CMemoryJobStateStore::GetLoadCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_loads; }
void CMemoryJobStateStore::SetFailSaves( UInt32 failNextSaves ) { MT::CScopeMutex lock( m_lock ); m_failNextSaves = failNextSaves; }
bool CMemoryJobStateStore::GetState( CORE::CDataNode& state ) const { MT::CScopeMutex lock( m_lock ); state = m_state; return m_hasState; }

/*-------------------------------------------------------------------------*/

void
RegisterJsTestTaskType( void )
{
    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    taskManager.RegisterTaskConsumerFactory( CJsTestTaskData::TaskType, &g_jsTaskFactory );
    taskManager.RegisterTaskDataFactory( CJsTestTaskData::TaskType, &g_jsTaskDataFactory );
}

void
UnregisterJsTestTaskType( void )
{
    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    taskManager.UnregisterTaskConsumerFactory( CJsTestTaskData::TaskType );
    taskManager.UnregisterTaskDataFactory( CJsTestTaskData::TaskType );
}

void
ClearJsRunLog( void )
{
    MT::CScopeMutex lock( g_jsRunLogLock );
    g_jsRunLog.clear();
}

void
GetJsRunLog( TJsRunLog& log )
{
    MT::CScopeMutex lock( g_jsRunLogLock );
    log = g_jsRunLog;
}

UInt32
CountJsRuns( const CORE::CString& value )
{
    MT::CScopeMutex lock( g_jsRunLogLock );
    UInt32 count = 0;
    for ( TJsRunLog::const_iterator i = g_jsRunLog.begin(); i != g_jsRunLog.end(); ++i )
        if ( (*i).value == value )
            ++count;
    return count;
}

Int64
JsUtc( Int64 hours, Int64 minutes, Int64 seconds )
{
    // 2026-01-01T00:00:00Z
    static const Int64 base = 1767225600000LL;
    return base + hours * 3600000LL + minutes * 60000LL + seconds * 1000LL;
}

CORE::CJobDefinition
MakeJsJob( const CORE::CString& id, const CORE::CJobTrigger& trigger, const CORE::CString& value, UInt32 cycles, UInt32 blockMs, bool fail )
{
    CORE::CJobDefinition job;
    job.SetId( id );
    job.SetTaskTypeName( CJsTestTaskData::TaskType );
    job.SetTrigger( trigger );
    CORE::CDataNode payload;
    payload.SetNodeType( GUCEF_DATATYPE_OBJECT );
    payload.SetAttribute( "value", value );
    payload.SetAttribute( "cycles", cycles );
    payload.SetAttribute( "blockMs", blockMs );
    payload.SetAttribute( "fail", fail );
    job.SetPayload( payload );
    return job;
}

CORE::CJobSchedulerSettings
MakeJsSettings( const CORE::CString& hostName )
{
    CORE::CJobSchedulerSettings settings;
    settings.SetHostName( hostName );
    settings.SetDispatcherPoolName( "test.js.dispatch" );
    settings.SetRunPoolName( "test.js.run." + hostName );
    settings.SetMainThreadPoolName( "test.js.main." + hostName );
    settings.SetStopTimeoutMs( 5000 );
    settings.SetStateSaveDebounceMs( 1000 );
    return settings;
}

CORE::JobSchedulerPtr
MakeJsScheduler( const CORE::CJobSchedulerSettings& settings, CManualClocks* clocks, CORE::PulseGeneratorPtr mainPulse )
{
    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    if ( mainPulse.IsNULL() )
        mainPulse = CORE::CPulseGenerator::CreateSharedObj();
    CORE::JobSchedulerPtr scheduler = taskManager.GetOrCreateJobScheduler( settings, mainPulse );
    if ( !scheduler.IsNULL() && GUCEF_NULL != clocks )
        scheduler->SetClocks( &clocks->utc, &clocks->monotonic );
    return scheduler;
}

bool
CycleUntilIdle( CORE::CJobScheduler& scheduler, UInt32 timeoutMs )
{
    UInt64 start = CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
    while ( CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs() - start < timeoutMs )
    {
        CORE::CJobSchedulerTestAccess::Cycle( scheduler );
        if ( scheduler.IsIdle() )
            return true;
        MT::ThreadDelay( 1 );
    }
    return scheduler.IsIdle();
}

bool
WaitUntilIdle( CORE::CJobScheduler& scheduler, UInt32 timeoutMs )
{
    UInt64 start = CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
    while ( CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs() - start < timeoutMs )
    {
        if ( scheduler.IsIdle() )
            return true;
        MT::ThreadDelay( 1 );
    }
    return scheduler.IsIdle();
}

void
GetJobRuns( CORE::CJobScheduler& scheduler, const CORE::CString& jobId, CORE::TJobRunRecordVector& runs )
{
    runs.clear();
    CORE::CJobSchedulerSnapshot snapshot;
    scheduler.GetSnapshot( snapshot );
    const CORE::TJobRunRecordVector& all = snapshot.GetRuns();
    for ( CORE::TJobRunRecordVector::const_iterator i = all.begin(); i != all.end(); ++i )
        if ( (*i).jobId == jobId )
            runs.push_back( (*i) );
}

UInt32
CountJobRunsInState( CORE::CJobScheduler& scheduler, const CORE::CString& jobId, CORE::TJobRunState state )
{
    CORE::TJobRunRecordVector runs;
    GetJobRuns( scheduler, jobId, runs );
    UInt32 count = 0;
    for ( CORE::TJobRunRecordVector::const_iterator i = runs.begin(); i != runs.end(); ++i )
        if ( (*i).state == state )
            ++count;
    return count;
}

CORE::SJobStatus
GetJobStatus( CORE::CJobScheduler& scheduler, const CORE::CString& jobId )
{
    CORE::CJobSchedulerSnapshot snapshot;
    scheduler.GetSnapshot( snapshot );
    const CORE::SJobStatus* status = snapshot.FindJob( jobId );
    return GUCEF_NULL != status ? *status : CORE::SJobStatus();
}

void
DropJsScheduler( CORE::JobSchedulerPtr& scheduler )
{
    if ( scheduler.IsNULL() )
        return;
    CORE::CString name = scheduler->GetName();
    CORE::CCoreGlobal::Instance()->GetTaskManager().UnregisterJobScheduler( name );
    scheduler.Unlink();
}

/*-------------------------------------------------------------------------*/

}; /* namespace CORETEST */
