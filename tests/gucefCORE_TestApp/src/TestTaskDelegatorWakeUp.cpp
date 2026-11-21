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

#include <algorithm>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestTaskDelegatorWakeUp.h"

#include <exception>

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
#include <stdio.h>
#define GUCEF_TEST_HAVE_PROC_THREAD_STATUS 1
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define REQUIRE_TRUE(t)  do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw std::exception(); } } while ( 0 )

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/** The test seam of CTaskManager: the probe outcome of the wake-up signal of the task delegators made from now on */
class CTaskManagerTestAccess
{
    public:

    /** 1: they use the wake-up signal, -1: they wait with a plain delay as without a usable semaphore, 0: as probed */
    static void OverrideTaskDelegatorWakeUpAvailability( Int32 availability )
    {
        CTaskManager::OverrideTaskDelegatorWakeUpAvailability( availability );
    }
};

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

namespace {

const char* g_poolName = "gucefCORE_TestApp.TaskDelegatorWakeUp";
const char* g_fallbackPoolName = "gucefCORE_TestApp.TaskDelegatorWakeUp.Fallback";
const char* g_threadEndPoolName = "gucefCORE_TestApp.TaskDelegatorWakeUp.ThreadEnd";
const char* g_idlePoolName = "gucefCORE_TestApp.TaskDelegatorWakeUp.IdleWorkers";
const char* g_shrinkPoolName = "gucefCORE_TestApp.TaskDelegatorWakeUp.Shrink";
const char* g_backToBackPoolName = "gucefCORE_TestApp.TaskDelegatorWakeUp.BackToBack";
const char* g_startupPoolName = "gucefCORE_TestApp.TaskDelegatorWakeUp.Startup";
const char* g_producersPoolName = "gucefCORE_TestApp.TaskDelegatorWakeUp.Producers";
const char* g_busyShrinkPoolName = "gucefCORE_TestApp.TaskDelegatorWakeUp.BusyShrink";
const char* g_pausePoolName = "gucefCORE_TestApp.TaskDelegatorWakeUp.Pause";
const char* g_burstPoolName = "gucefCORE_TestApp.TaskDelegatorWakeUp.Burst";

/** the longest wait of a worker thread without a task for a task to be queued (CTaskDelegator::IdleTaskWaitMs) */
const UInt32 g_idleTaskWaitMs = 1000;

/** the minimal cycle delta of a task delegator: an idle delegator waits this long between its cycles */
const UInt32 g_minimalCycleDeltaInMs = 10;

#if defined( __SANITIZE_THREAD__ ) || defined( __SANITIZE_ADDRESS__ )
const Float64 g_medianWakeLimitInMs = 2.0;
#else
const Float64 g_medianWakeLimitInMs = 1.0;
#endif

/*-------------------------------------------------------------------------*/

/** A persistent dedicated task without work of its own, it counts its cycles */
class CWakeUpTestTask : public CTaskConsumer
{
    public:

    CWakeUpTestTask( void ) : CTaskConsumer(), m_cycles( 0 ) {}
    virtual ~CWakeUpTestTask() {}

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "GUCEF::CORE::TestTaskDelegatorWakeUp::Task"; }
    virtual bool OnTaskCycle( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE { ++m_cycles; return false; }
    virtual void OnTaskEnded( CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE { CTaskConsumer::OnTaskEnded( task, wasForced ); }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CString typeName = "CWakeUpTestTask";
        return typeName;
    }

    UInt32 GetCycles( void ) const { return m_cycles.Load(); }

    private:

    MT::CTAtomic< UInt32 > m_cycles;
};

typedef CTSharedPtr< CWakeUpTestTask, MT::CMutex > CWakeUpTestTaskPtr;

/*-------------------------------------------------------------------------*/

/** The precision tick count at which the last CQueuedTickTask began its cycle */
MT::CTAtomic< UInt64 > g_queuedTaskStartTicks( 0 );

/** A queued task that records when a worker thread began it, then ends */
class CQueuedTickTask : public CTaskConsumer
{
    public:

    static const CString TaskType;

    CQueuedTickTask( void ) : CTaskConsumer() {}
    virtual ~CQueuedTickTask() {}

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }
    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }
    virtual bool OnTaskCycle( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE { g_queuedTaskStartTicks.Store( MT::PrecisionTickCount() ); return true; }
    virtual void OnTaskEnded( CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE { CTaskConsumer::OnTaskEnded( task, wasForced ); }
};

const CString CQueuedTickTask::TaskType = "GUCEF::CORE::TestTaskDelegatorWakeUp::QueuedTickTask";

/*-------------------------------------------------------------------------*/

/** How many tasks of CTinyTask ran, how long each one holds its worker, and the precision tick count of the start of the last one */
MT::CTAtomic< UInt32 > g_tinyTaskRuns( 0 );
MT::CTAtomic< UInt32 > g_tinyTaskHoldMs( 0 );
MT::CTAtomic< UInt64 > g_tinyTaskLastStartTicks( 0 );

/** The precision tick count at which each task began, by the order in which they began (the first g_tinyTaskStartLogSize tasks after g_tinyTaskRuns was set to 0) */
const UInt32 g_tinyTaskStartLogSize = 1024;
UInt64 g_tinyTaskStartLog[ g_tinyTaskStartLogSize ];

/** A queued task of a pool that does next to nothing: it counts itself and holds its worker for g_tinyTaskHoldMs */
class CTinyTask : public CTaskConsumer
{
    public:

    static const CString TaskType;

    CTinyTask( void ) : CTaskConsumer() {}
    virtual ~CTinyTask() {}

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }
    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }
    virtual bool OnTaskCycle( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt64 startTicks = MT::PrecisionTickCount();
        g_tinyTaskLastStartTicks.Store( startTicks );
        const UInt32 index = g_tinyTaskRuns.FetchAdd( 1 );
        if ( index < g_tinyTaskStartLogSize )
            g_tinyTaskStartLog[ index ] = startTicks;
        const UInt32 holdMs = g_tinyTaskHoldMs.Load();
        if ( holdMs > 0 )
            MT::ThreadDelay( holdMs );
        return true;
    }
    virtual void OnTaskEnded( CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE { CTaskConsumer::OnTaskEnded( task, wasForced ); }
};

const CString CTinyTask::TaskType = "GUCEF::CORE::TestTaskDelegatorWakeUp::TinyTask";

typedef CTFactory< CTaskConsumer, CTinyTask, MT::CMutex > TTinyTaskFactory;

/*-------------------------------------------------------------------------*/

/** Queues a number of CTinyTask tasks to a pool from a thread of its own */
class CTinyTaskProducer : public MT::CActiveObject
{
    public:

    CTinyTaskProducer( ThreadPoolPtr pool, UInt32 count ) : MT::CActiveObject(), m_pool( pool ), m_count( count ), m_queued( 0 ), m_failed( 0 ) {}
    virtual ~CTinyTaskProducer() {}

    UInt32 GetQueued( void ) const { return m_queued.Load(); }
    UInt32 GetFailed( void ) const { return m_failed.Load(); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 i=0; i<m_count; ++i )
        {
            CFutureResult future = m_pool->QueueTask( CTinyTask::TaskType );
            if ( future.HasAFuture() )
                m_queued.FetchAdd( 1 );
            else
                m_failed.FetchAdd( 1 );
        }
        return true;
    }

    private:

    ThreadPoolPtr m_pool;
    UInt32 m_count;
    MT::CTAtomic< UInt32 > m_queued;
    MT::CTAtomic< UInt32 > m_failed;
};

/*-------------------------------------------------------------------------*/

/** The processor time in nanoseconds that the given threads of this process used so far, -1 where that cannot be read */
Int64
CountThreadCpuNs( const CThreadPool::TThreadIdVector& threadIds )
{
    #ifdef GUCEF_TEST_HAVE_PROC_THREAD_STATUS
    Int64 total = 0;
    for ( UInt32 i=0; i<threadIds.size(); ++i )
    {
        char path[ 64 ];
        snprintf( path, sizeof( path ), "/proc/self/task/%u/schedstat", (unsigned int) threadIds[ i ] );
        FILE* stat = fopen( path, "r" );
        if ( GUCEF_NULL == stat )
            return -1;
        unsigned long long runNs = 0;
        int fields = fscanf( stat, "%llu", &runNs );
        fclose( stat );
        if ( 1 != fields )
            return -1;
        total += (Int64) runNs;
    }
    return total;
    #else
    return -1;
    #endif
}

/*-------------------------------------------------------------------------*/

/** Waits until the future is ready, at most timeoutInMs; yields instead of sleeping so that the next call follows at once */
bool
WaitForReady( const CFutureResult& future, UInt32 timeoutInMs )
{
    const UInt64 startTicks = MT::PrecisionTickCount();
    while ( !future.IsReady() )
    {
        if ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks ) > (Float64) timeoutInMs )
            return false;
        MT::ThreadDelay( 0 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** A dedicated task that does its work in cycles a long delay apart and needs no periodic pulses between them */
class CLongDelayTask : public CTaskConsumer
{
    public:

    CLongDelayTask( void ) : CTaskConsumer(), m_cycles( 0 ) {}
    virtual ~CLongDelayTask() {}

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "GUCEF::CORE::TestTaskDelegatorWakeUp::LongDelayTask"; }
    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CString typeName = "CLongDelayTask";
        return typeName;
    }
    virtual bool OnTaskStart( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !CTaskConsumer::OnTaskStart( task ) )
            return false;
        RequestTaskCycleDelayInMs( 60000 );
        GetPulseGenerator()->ForceStopOfPeriodicPulses();
        return true;
    }
    virtual bool OnTaskCycle( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE { ++m_cycles; RequestTaskCycleDelayInMs( 60000 ); return false; }
    virtual void OnTaskEnded( CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE { CTaskConsumer::OnTaskEnded( task, wasForced ); }

    UInt32 GetCycles( void ) const { return m_cycles.Load(); }

    private:

    MT::CTAtomic< UInt32 > m_cycles;
};

typedef CTSharedPtr< CLongDelayTask, MT::CMutex > CLongDelayTaskPtr;

/*-------------------------------------------------------------------------*/

/**
 *  A dedicated task like CLongDelayTask (cycles 60 s apart, no periodic pulses) whose cycle can be held at a gate: the test
 *  makes a request from another thread at a known point inside the cycle, after the cycle took its input, as a work queue
 *  that is fed from other threads and woken with an immediate pulse request is
 */
class CGatedCycleTask : public CTaskConsumer
{
    public:

    CGatedCycleTask( void ) : CTaskConsumer(), m_cycles( 0 ), m_gateArmed( 0 ), m_inGate( 0 ), m_gateOpen( 0 ) {}
    virtual ~CGatedCycleTask() {}

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "GUCEF::CORE::TestTaskDelegatorWakeUp::GatedCycleTask"; }
    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CString typeName = "CGatedCycleTask";
        return typeName;
    }
    virtual bool OnTaskStart( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !CTaskConsumer::OnTaskStart( task ) )
            return false;
        RequestTaskCycleDelayInMs( 60000 );
        GetPulseGenerator()->ForceStopOfPeriodicPulses();
        return true;
    }
    virtual bool OnTaskCycle( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++m_cycles;
        UInt32 armed = 1;
        if ( m_gateArmed.CompareExchangeStrong( armed, 0 ) )
        {
            // the input of this cycle is taken: wait at the gate until the test opened it (at most 5 s)
            m_inGate.Store( 1 );
            for ( UInt32 waited=0; waited<5000 && 0 == m_gateOpen.Load(); ++waited )
                MT::ThreadDelay( 1 );
            m_gateOpen.Store( 0 );
            m_inGate.Store( 0 );
        }
        RequestTaskCycleDelayInMs( 60000 );
        return false;
    }
    virtual void OnTaskEnding( CTaskPtr task, bool willBeForced ) GUCEF_VIRTUAL_OVERRIDE
    {
        GetPulseGenerator()->AllowPeriodicPulses();
        CTaskConsumer::OnTaskEnding( task, willBeForced );
    }
    virtual void OnTaskEnded( CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE { CTaskConsumer::OnTaskEnded( task, wasForced ); }

    UInt32 GetCycles( void ) const { return m_cycles.Load(); }
    void ArmGate( void ) { m_gateArmed.Store( 1 ); }
    bool IsInGate( void ) const { return 0 != m_inGate.Load(); }
    void OpenGate( void ) { m_gateOpen.Store( 1 ); }

    private:

    MT::CTAtomic< UInt32 > m_cycles;
    MT::CTAtomic< UInt32 > m_gateArmed;
    MT::CTAtomic< UInt32 > m_inGate;
    MT::CTAtomic< UInt32 > m_gateOpen;
};

typedef CTSharedPtr< CGatedCycleTask, MT::CMutex > CGatedCycleTaskPtr;

/*-------------------------------------------------------------------------*/

/** Waits until the task cycled more than the given count, at most timeoutInMs; @return whether it did */
bool
WaitForCycleAfter( const CGatedCycleTask& task, UInt32 cycles, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs && task.GetCycles() <= cycles; ++waited )
        MT::ThreadDelay( 1 );
    return task.GetCycles() > cycles;
}

/*-------------------------------------------------------------------------*/

/** How many of the given threads of this process still run, -1 where that cannot be read */
Int32
CountLiveThreads( const CThreadPool::TThreadIdVector& threadIds )
{
    #ifdef GUCEF_TEST_HAVE_PROC_THREAD_STATUS
    Int32 live = 0;
    for ( UInt32 i=0; i<threadIds.size(); ++i )
    {
        char path[ 64 ];
        snprintf( path, sizeof( path ), "/proc/self/task/%u/status", (unsigned int) threadIds[ i ] );
        FILE* status = fopen( path, "r" );
        if ( GUCEF_NULL != status )
        {
            ++live;
            fclose( status );
        }
    }
    return live;
    #else
    return -1;
    #endif
}

/*-------------------------------------------------------------------------*/

/** The context switches (voluntary and not) of the given threads of this process so far, -1 where they cannot be read */
Int64
CountContextSwitches( const CThreadPool::TThreadIdVector& threadIds )
{
    #ifdef GUCEF_TEST_HAVE_PROC_THREAD_STATUS
    Int64 total = 0;
    for ( UInt32 i=0; i<threadIds.size(); ++i )
    {
        char path[ 64 ];
        snprintf( path, sizeof( path ), "/proc/self/task/%u/status", (unsigned int) threadIds[ i ] );
        FILE* status = fopen( path, "r" );
        if ( GUCEF_NULL == status )
            return -1;
        char line[ 256 ];
        while ( GUCEF_NULL != fgets( line, sizeof( line ), status ) )
        {
            unsigned long count = 0;
            if ( 1 == sscanf( line, "voluntary_ctxt_switches: %lu", &count ) || 1 == sscanf( line, "nonvoluntary_ctxt_switches: %lu", &count ) )
                total += (Int64) count;
        }
        fclose( status );
    }
    return total;
    #else
    return -1;
    #endif
}

/*-------------------------------------------------------------------------*/

/** Keeps the number of pulses of a pulse generator and the precision tick count of the last one */
class CPulseClock : public CObserver
{
    public:

    CPulseClock( void ) : CObserver(), m_pulses( 0 ), m_lastPulseTicks( 0 ) {}
    virtual ~CPulseClock() { UnsubscribeAllFromObserver(); }

    void Watch( PulseGeneratorPtr pulseGenerator )
    {
        CTEventHandlerFunctor< CPulseClock > callback( this, &CPulseClock::OnPulse );
        SubscribeTo( pulseGenerator.GetPointerAlways(), CPulseGenerator::PulseEvent, callback );
    }

    UInt32 GetPulses( void ) const { return m_pulses.Load(); }
    UInt64 GetLastPulseTicks( void ) const { return m_lastPulseTicks.Load(); }

    private:

    void OnPulse( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        m_lastPulseTicks.Store( MT::PrecisionTickCount() );
        ++m_pulses;
    }

    MT::CTAtomic< UInt32 > m_pulses;
    MT::CTAtomic< UInt64 > m_lastPulseTicks;
};

/*-------------------------------------------------------------------------*/

/** Waits for a pulse after the given count, @return the time from startTicks to that pulse in ms, or -1 */
Float64
WaitForPulseAfter( const CPulseClock& clock, UInt32 pulsesBefore, UInt64 startTicks )
{
    UInt32 waitedMs = 0;
    while ( clock.GetPulses() <= pulsesBefore )
    {
        if ( waitedMs >= 2000 )
            return -1.0;
        MT::ThreadDelay( 1 );
        ++waitedMs;
    }
    UInt64 pulseTicks = clock.GetLastPulseTicks();
    if ( pulseTicks < startTicks )
        return 0.0;
    return MT::PrecisionTimerTicksToMs( pulseTicks - startTicks );
}

/*-------------------------------------------------------------------------*/

/** @return the given fraction (0..1) of the sorted latencies */
Float64
LatencyAt( GUCEF::vector< Float64 >& latencies, Float64 fraction )
{
    if ( latencies.empty() )
        return -1.0;
    std::sort( latencies.begin(), latencies.end() );
    size_t index = (size_t) ( fraction * ( latencies.size() - 1 ) );
    return latencies[ index ];
}

/*-------------------------------------------------------------------------*/

/** Asks a pulse generator for an immediate pulse once, from a thread of its own */
class CPulseRequesterThread : public MT::CActiveObject
{
    public:

    explicit CPulseRequesterThread( PulseGeneratorPtr pulseGenerator ) : MT::CActiveObject(), m_pulseGenerator( pulseGenerator ), m_done( 0 ) {}
    virtual ~CPulseRequesterThread() {}

    bool IsDone( void ) const { return 0 != m_done.Load(); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_pulseGenerator->RequestImmediatePulse();
        m_done.Store( 1 );
        return true;
    }

    private:

    PulseGeneratorPtr m_pulseGenerator;
    MT::CTAtomic< UInt32 > m_done;
};

/*-------------------------------------------------------------------------*/

/**
 *  Counts the pulses of a pulse generator and, when armed, asks for one more immediate pulse from within the next pulse:
 *  on the thread of the pulse itself, or from another thread while the pulse is delivered
 */
class CPulseRequestingClock : public CObserver
{
    public:

    CPulseRequestingClock( void ) : CObserver(), m_pulses( 0 ), m_armed( 0 ), m_fromOtherThread( false ), m_requestFailures( 0 ) {}
    virtual ~CPulseRequestingClock() { UnsubscribeAllFromObserver(); }

    void Watch( PulseGeneratorPtr pulseGenerator )
    {
        m_pulseGenerator = pulseGenerator;
        CTEventHandlerFunctor< CPulseRequestingClock > callback( this, &CPulseRequestingClock::OnPulse );
        SubscribeTo( pulseGenerator.GetPointerAlways(), CPulseGenerator::PulseEvent, callback );
    }

    void Arm( bool fromOtherThread ) { m_fromOtherThread = fromOtherThread; m_armed.Store( 1 ); }

    UInt32 GetPulses( void ) const { return m_pulses.Load(); }
    UInt32 GetRequestFailures( void ) const { return m_requestFailures.Load(); }

    private:

    void OnPulse( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        ++m_pulses;
        UInt32 armed = 1;
        if ( !m_armed.CompareExchangeStrong( armed, 0 ) )
            return;

        if ( !m_fromOtherThread )
        {
            m_pulseGenerator->RequestImmediatePulse();
            return;
        }

        // the request is made and done while this pulse is still being delivered
        CPulseRequesterThread requester( m_pulseGenerator );
        if ( !requester.Activate( GUCEF_NULL, 1, 1 ) )
        {
            ++m_requestFailures;
            return;
        }
        for ( UInt32 i=0; i<2000 && !requester.IsDone(); ++i )
            MT::ThreadDelay( 1 );
        if ( !requester.IsDone() )
            ++m_requestFailures;
        requester.Deactivate( false, true );
    }

    PulseGeneratorPtr m_pulseGenerator;
    MT::CTAtomic< UInt32 > m_pulses;
    MT::CTAtomic< UInt32 > m_armed;
    bool m_fromOtherThread;
    MT::CTAtomic< UInt32 > m_requestFailures;
};

/*-------------------------------------------------------------------------*/

/**
 *  Runs rounds of: an immediate pulse request, during whose pulse one more is asked for (see CPulseRequestingClock).
 *  @return the number of rounds in which the request made during the pulse got no pulse after it within 300 ms
 */
UInt32
CountRequestsDuringPulseWithoutPulse( PulseGeneratorPtr pulseGenerator, bool fromOtherThread, UInt32 rounds, UInt32& requestFailures )
{
    CPulseRequestingClock clock;
    clock.Watch( pulseGenerator );

    UInt32 lost = 0;
    for ( UInt32 r=0; r<rounds; ++r )
    {
        MT::ThreadDelay( 2 + ( r % 5 ) );
        UInt32 before = clock.GetPulses();
        clock.Arm( fromOtherThread );
        pulseGenerator->RequestImmediatePulse();
        for ( UInt32 waited=0; waited<300 && clock.GetPulses() < before + 2; ++waited )
            MT::ThreadDelay( 1 );
        if ( clock.GetPulses() < before + 2 )
            ++lost;
    }
    requestFailures = clock.GetRequestFailures();
    clock.UnsubscribeAllFromObserver();
    return lost;
}

/*-------------------------------------------------------------------------*/

/** Sets up and starts a dedicated task, @return the pulse generator of its delegator or NULL */
PulseGeneratorPtr
StartDedicatedTask( ThreadPoolPtr pool, CWakeUpTestTaskPtr task )
{
    if ( !pool->SetupSingularTask( task ).HasAFuture() || !pool->StartTaskWithConsumer( task ).HasAFuture() )
        return PulseGeneratorPtr();

    // the consumer takes the delegator's pulse generator once it is linked to its thread
    PulseGeneratorPtr global = CCoreGlobal::Instance()->GetPulseGenerator();
    for ( UInt32 i=0; i<600; ++i )
    {
        if ( !task->GetTaskDelegator().IsNULL() && task->GetPulseGenerator() != global && 0 < task->GetCycles() )
            return task->GetPulseGenerator();
        MT::ThreadDelay( 5 );
    }
    return PulseGeneratorPtr();
}

/*-------------------------------------------------------------------------*/

/** What the thread that waits for all tasks of a pool shares with the test */
struct SWaitAllCase
{
    SWaitAllCase( ThreadPoolPtr threadPool ) : pool( threadPool ), stop( false ), waits( 0 ), failedWaits( 0 ), taskIdScans( 0 ) {}

    ThreadPoolPtr pool;
    MT::CTAtomic< bool > stop;
    MT::CTAtomic< UInt32 > waits;
    MT::CTAtomic< UInt32 > failedWaits;
    MT::CTAtomic< UInt32 > taskIdScans;
};

/** Waits for all tasks of the pool and asks which tasks have work, over and over, while the test queues tasks that end on the workers */
UInt32 GUCEF_CALLSPEC_PREFIX
WaitAllThreadMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SWaitAllCase& waitCase = *static_cast< SWaitAllCase* >( data );
    try
    {
        while ( !waitCase.stop.Load( MT::MEMORDER_ACQUIRE ) )
        {
            CThreadPool::TTaskIdVector taskIds;
            waitCase.pool->GetAllTaskIdsForTasksWithWork( taskIds, true );
            waitCase.taskIdScans.FetchAdd( 1 );
            if ( !waitCase.pool->WaitForAllTasksToFinish( 2000 ) )
                waitCase.failedWaits.FetchAdd( 1 );
            waitCase.waits.FetchAdd( 1 );
        }
    }
    catch ( ... )
    {
        waitCase.failedWaits.FetchAdd( 1 );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

/** What a polling thread shares with the test: the task it asks about and what it saw */
struct SConsumerPoll
{
    SConsumerPoll( CWakeUpTestTaskPtr consumer ) : task( consumer ), stop( false ), polls( 0 ), sawActive( false ) {}

    CWakeUpTestTaskPtr task;
    MT::CTAtomic< bool > stop;
    MT::CTAtomic< UInt32 > polls;
    MT::CTAtomic< bool > sawActive;
};

/** Asks a task consumer from a thread that is not its own whether it is active or asked to stop, as fast as it can */
UInt32 GUCEF_CALLSPEC_PREFIX
ConsumerPollThreadMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SConsumerPoll& poll = *static_cast< SConsumerPoll* >( data );
    while ( !poll.stop.Load( MT::MEMORDER_ACQUIRE ) )
    {
        if ( poll.task->IsActive() )
            poll.sawActive.Store( true );
        poll.task->IsDeactivationRequested();
        poll.polls.FetchAdd( 1 );
    }
    return 0;
}

}

/*-------------------------------------------------------------------------*/

void
PerformTaskDelegatorWakeUpTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "TaskDelegatorWakeUp" );

    ThreadPoolPtr pool = CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( g_poolName );
    CWakeUpTestTaskPtr task( GUCEF_NEW CWakeUpTestTask() );
    PulseGeneratorPtr pulseGenerator;
    if ( !pool.IsNULL() )
        pulseGenerator = StartDedicatedTask( pool, task );

    GUCEF_TESTFW_TESTCASE( "An immediate pulse requested on another thread wakes the delegator of a dedicated task that waits between its cycles" )
        try
        {
            ASSERT_TRUE( !pulseGenerator.IsNULL() );
            if ( !pulseGenerator.IsNULL() )
            {
                CPulseClock clock;
                clock.Watch( pulseGenerator );

                // the requests fall at every point of the delegator's wait between two cycles
                const UInt32 requests = 60;
                GUCEF::vector< Float64 > latencies;
                for ( UInt32 i=0; i<requests; ++i )
                {
                    MT::ThreadDelay( 2 + ( i % 7 ) );
                    UInt32 pulsesBefore = clock.GetPulses();
                    UInt64 startTicks = MT::PrecisionTickCount();
                    pulseGenerator->RequestImmediatePulse();
                    Float64 latency = WaitForPulseAfter( clock, pulsesBefore, startTicks );
                    ASSERT_TRUE( latency >= 0.0 );
                    if ( latency >= 0.0 )
                        latencies.push_back( latency );
                }

                Float64 median = LatencyAt( latencies, 0.5 );
                Float64 p90 = LatencyAt( latencies, 0.9 );
                Float64 worst = LatencyAt( latencies, 1.0 );
                GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: immediate pulse request to pulse, over " << latencies.size()
                           << " requests: median " << median << " ms, p90 " << p90 << " ms, worst " << worst << " ms" );
                ASSERT_TRUE( median >= 0.0 && median < g_medianWakeLimitInMs );
                ASSERT_TRUE( p90 < g_minimalCycleDeltaInMs / 2.0 );

                clock.UnsubscribeAllFromObserver();
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "A request for periodic pulses made on another thread wakes the delegator of a dedicated task that waits between its cycles" )
        try
        {
            ASSERT_TRUE( !pulseGenerator.IsNULL() );
            if ( !pulseGenerator.IsNULL() )
            {
                CPulseClock clock;
                clock.Watch( pulseGenerator );

                // as a transport does that has a frame for the task: periodic pulses while frames wait, stopped once
                // they are delivered
                const UInt32 requests = 40;
                GUCEF::vector< Float64 > latencies;
                for ( UInt32 i=0; i<requests; ++i )
                {
                    MT::ThreadDelay( 2 + ( i % 7 ) );
                    UInt32 pulsesBefore = clock.GetPulses();
                    UInt64 startTicks = MT::PrecisionTickCount();
                    pulseGenerator->RequestPeriodicPulses( &clock, 1 );
                    Float64 latency = WaitForPulseAfter( clock, pulsesBefore, startTicks );
                    pulseGenerator->RequestStopOfPeriodicUpdates( &clock );
                    ASSERT_TRUE( latency >= 0.0 );
                    if ( latency >= 0.0 )
                        latencies.push_back( latency );
                    MT::ThreadDelay( g_minimalCycleDeltaInMs * 2 );   // the delegator is back to its idle cycle
                }

                Float64 median = LatencyAt( latencies, 0.5 );
                Float64 p90 = LatencyAt( latencies, 0.9 );
                Float64 worst = LatencyAt( latencies, 1.0 );
                GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: periodic pulse request to first pulse, over " << latencies.size()
                           << " requests: median " << median << " ms, p90 " << p90 << " ms, worst " << worst << " ms" );
                ASSERT_TRUE( median >= 0.0 && median < g_medianWakeLimitInMs );
                ASSERT_TRUE( p90 < g_minimalCycleDeltaInMs / 2.0 );

                clock.UnsubscribeAllFromObserver();
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "After a flood of pulse requests from another thread an idle delegator cycles no faster than its minimal cycle delta" )
        try
        {
            ASSERT_TRUE( !pulseGenerator.IsNULL() );
            if ( !pulseGenerator.IsNULL() )
            {
                for ( UInt32 i=0; i<5000; ++i )
                {
                    pulseGenerator->RequestImmediatePulse();
                    if ( 0 == i % 3 )
                    {
                        pulseGenerator->RequestPeriodicPulses( &task, 1 );
                        pulseGenerator->RequestStopOfPeriodicUpdates( &task );
                    }
                }
                MT::ThreadDelay( g_minimalCycleDeltaInMs * 5 );

                // waiting does not turn into spinning: over the measured time the cycles are bound by the minimal cycle
                // delta, with a few more for the scheduling of the measurement itself
                const UInt32 measuredMs = 400;
                UInt32 cyclesBefore = task->GetCycles();
                UInt64 startTicks = MT::PrecisionTickCount();
                MT::ThreadDelay( measuredMs );
                UInt32 cycles = task->GetCycles() - cyclesBefore;
                Float64 elapsedMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks );
                UInt32 limit = (UInt32) ( elapsedMs / g_minimalCycleDeltaInMs ) + 5;
                GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: idle delegator after a flood of requests: " << cycles << " cycles in "
                           << elapsedMs << " ms (limit " << limit << ")" );
                ASSERT_TRUE( cycles > 0 );
                ASSERT_TRUE( cycles <= limit );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "With periodic pulses stopped, an immediate pulse asked for from within a pulse is followed by a pulse, every time" )
        try
        {
            ASSERT_TRUE( !pulseGenerator.IsNULL() );
            if ( !pulseGenerator.IsNULL() )
            {
                // only requests make the delegator pulse: a ticket handed out while it pulses must not be taken by that pulse
                pulseGenerator->ForceStopOfPeriodicPulses();

                const UInt32 rounds = 50;
                UInt32 requestFailures = 0;
                UInt32 lostOwnThread = CountRequestsDuringPulseWithoutPulse( pulseGenerator, false, rounds, requestFailures );
                UInt32 lostOtherThread = CountRequestsDuringPulseWithoutPulse( pulseGenerator, true, rounds, requestFailures );
                GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: requests made during a pulse without a pulse after them: " << lostOwnThread
                           << " of " << rounds << " on the delegator thread, " << lostOtherThread << " of " << rounds << " from another thread" );
                ASSERT_TRUE( 0 == requestFailures );
                ASSERT_TRUE( 0 == lostOwnThread );
                ASSERT_TRUE( 0 == lostOtherThread );

                pulseGenerator->AllowPeriodicPulses();
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Without a usable wake-up signal a delegator waits between its cycles without spinning and still answers pulse requests" )
        try
        {
            // as with a gucefMT whose semaphore does not work: the delegators made now fall back to a plain delay
            CTaskManagerTestAccess::OverrideTaskDelegatorWakeUpAvailability( -1 );
            ThreadPoolPtr fallbackPool = CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( g_fallbackPoolName );
            CWakeUpTestTaskPtr fallbackTask( GUCEF_NEW CWakeUpTestTask() );
            PulseGeneratorPtr fallbackPulseGenerator;
            if ( !fallbackPool.IsNULL() )
                fallbackPulseGenerator = StartDedicatedTask( fallbackPool, fallbackTask );
            CTaskManagerTestAccess::OverrideTaskDelegatorWakeUpAvailability( 0 );

            ASSERT_TRUE( !fallbackPulseGenerator.IsNULL() );
            if ( !fallbackPulseGenerator.IsNULL() )
            {
                const UInt32 measuredMs = 400;
                UInt32 cyclesBefore = fallbackTask->GetCycles();
                UInt64 startTicks = MT::PrecisionTickCount();
                MT::ThreadDelay( measuredMs );
                UInt32 cycles = fallbackTask->GetCycles() - cyclesBefore;
                Float64 elapsedMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks );
                UInt32 limit = (UInt32) ( elapsedMs / g_minimalCycleDeltaInMs ) + 5;
                GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: idle delegator without a wake-up signal: " << cycles << " cycles in "
                           << elapsedMs << " ms (limit " << limit << ")" );
                ASSERT_TRUE( cycles > 0 );
                ASSERT_TRUE( cycles <= limit );

                // a request is taken at the next cycle, within the minimal cycle delta and the time to pulse
                CPulseClock clock;
                clock.Watch( fallbackPulseGenerator );
                GUCEF::vector< Float64 > latencies;
                for ( UInt32 i=0; i<20; ++i )
                {
                    MT::ThreadDelay( 2 + ( i % 7 ) );
                    UInt32 pulsesBefore = clock.GetPulses();
                    UInt64 requestTicks = MT::PrecisionTickCount();
                    fallbackPulseGenerator->RequestImmediatePulse();
                    Float64 latency = WaitForPulseAfter( clock, pulsesBefore, requestTicks );
                    ASSERT_TRUE( latency >= 0.0 );
                    if ( latency >= 0.0 )
                        latencies.push_back( latency );
                }
                clock.UnsubscribeAllFromObserver();
                Float64 worst = LatencyAt( latencies, 1.0 );
                GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: without a wake-up signal the worst request to pulse time is " << worst << " ms" );
                ASSERT_TRUE( worst >= 0.0 && worst < g_minimalCycleDeltaInMs * 5.0 );

                fallbackPulseGenerator->ForceStopOfPeriodicPulses();
                UInt32 requestFailures = 0;
                ASSERT_TRUE( 0 == CountRequestsDuringPulseWithoutPulse( fallbackPulseGenerator, true, 20, requestFailures ) );
                ASSERT_TRUE( 0 == requestFailures );
                fallbackPulseGenerator->AllowPeriodicPulses();
            }

            if ( !fallbackTask.IsNULL() )
                fallbackTask->RequestTaskToStop( true );
            CCoreGlobal::Instance()->GetTaskManager().UnregisterThreadPool( g_fallbackPoolName );
        }
        catch ( ... )
        {
            CTaskManagerTestAccess::OverrideTaskDelegatorWakeUpAvailability( 0 );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Once the thread of a delegator ended its pulse generator no longer names that thread as its driver thread" )
        try
        {
            ThreadPoolPtr endPool = CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( g_threadEndPoolName );
            CWakeUpTestTaskPtr endTask( GUCEF_NEW CWakeUpTestTask() );
            PulseGeneratorPtr endPulseGenerator;
            if ( !endPool.IsNULL() )
                endPulseGenerator = StartDedicatedTask( endPool, endTask );
            ASSERT_TRUE( !endPulseGenerator.IsNULL() );
            if ( !endPulseGenerator.IsNULL() )
            {
                TThreadId driverThreadId = endPulseGenerator->GetPulseDriverThreadId();
                ASSERT_TRUE( 0 != driverThreadId );

                ASSERT_TRUE( endPool->RequestAllThreadsToStop( true, false, 5000 ) );

                // a thread created later may get the same id: it must not count as the driver of this pulse generator
                TThreadId driverThreadIdAfterEnd = endPulseGenerator->GetPulseDriverThreadId();
                for ( UInt32 waitedMs=0; 0 != driverThreadIdAfterEnd && waitedMs<5000; ++waitedMs )
                {
                    MT::ThreadDelay( 1 );
                    driverThreadIdAfterEnd = endPulseGenerator->GetPulseDriverThreadId();
                }
                GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: driver thread id " << driverThreadId << " while the delegator runs, "
                           << driverThreadIdAfterEnd << " after its thread ended" );
                ASSERT_TRUE( 0 == driverThreadIdAfterEnd );
            }
            CCoreGlobal::Instance()->GetTaskManager().UnregisterThreadPool( g_threadEndPoolName );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Idle worker threads of a pool wait for a task to be queued instead of looking every cycle delay; a queued task starts at once, also several queued at once, and the threads stop at once" )
        try
        {
            typedef CTFactory< CTaskConsumer, CQueuedTickTask, MT::CMutex > TQueuedTickTaskFactory;
            TQueuedTickTaskFactory factory;
            CTaskManager& taskManager = CCoreGlobal::Instance()->GetTaskManager();
            taskManager.RegisterTaskConsumerFactory( CQueuedTickTask::TaskType, &factory );

            const UInt32 workers = 4;
            ThreadPoolPtr idlePool = taskManager.GetOrCreateThreadPool( g_idlePoolName );
            REQUIRE_TRUE( !idlePool.IsNULL() );
            idlePool->SetDesiredMaxTotalNrOfThreads( (Int32) workers );
            idlePool->SetDesiredMinNrOfWorkerThreads( workers );
            for ( UInt32 i=0; i<500 && idlePool->GetActiveNrOfWorkerThreads() < workers; ++i )
                MT::ThreadDelay( 5 );
            REQUIRE_TRUE( workers == idlePool->GetActiveNrOfWorkerThreads() );
            CThreadPool::TThreadIdVector threadIds;
            idlePool->GetAllCurrentThreadIds( threadIds );
            REQUIRE_TRUE( workers == threadIds.size() );

            // idle: before, every worker looked at the queue after every cycle delay (25 ms), about 40 wake-ups per
            // second each; now each waits for a task, at most g_idleTaskWaitMs
            MT::ThreadDelay( 200 );
            Int64 switchesBefore = CountContextSwitches( threadIds );
            const UInt32 idleMs = 2000;
            MT::ThreadDelay( idleMs );
            Int64 switchesAfter = CountContextSwitches( threadIds );
            if ( switchesBefore >= 0 && switchesAfter >= 0 )
            {
                Int64 wakes = switchesAfter - switchesBefore;
                Int64 limit = (Int64) workers * ( idleMs / 1000 + 2 );
                GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: " << workers << " idle worker threads woke " << wakes << " times in " << idleMs << " ms (limit " << limit << ")" );
                ASSERT_TRUE( wakes <= limit );
            }

            // a task queued to idle workers starts at once, not at the next look at the queue
            GUCEF::vector< Float64 > latencies;
            for ( UInt32 i=0; i<40; ++i )
            {
                MT::ThreadDelay( 3 + ( i % 11 ) );
                g_queuedTaskStartTicks.Store( 0 );
                UInt64 queuedTicks = MT::PrecisionTickCount();
                CFutureResult future = idlePool->QueueTask( CQueuedTickTask::TaskType );
                ASSERT_TRUE( future.HasAFuture() );
                for ( UInt32 waited=0; 0 == g_queuedTaskStartTicks.Load() && waited<2000; ++waited )
                    MT::ThreadDelay( 1 );
                UInt64 startTicks = g_queuedTaskStartTicks.Load();
                ASSERT_TRUE( 0 != startTicks );
                if ( 0 != startTicks )
                    latencies.push_back( MT::PrecisionTimerTicksToMs( startTicks - queuedTicks ) );
            }
            Float64 median = LatencyAt( latencies, 0.5 );
            Float64 worst = LatencyAt( latencies, 1.0 );
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: queued task to its start on an idle worker, over " << latencies.size()
                       << " tasks: median " << median << " ms, worst " << worst << " ms" );
            ASSERT_TRUE( median >= 0.0 && median < g_medianWakeLimitInMs * 2.0 );
            ASSERT_TRUE( worst < (Float64) g_idleTaskWaitMs / 2.0 );

            // as many tasks as workers queued at once each wake a worker of their own: all have run within a few ms
            UInt64 burstTicks = MT::PrecisionTickCount();
            GUCEF::vector< CFutureResult > futures;
            for ( UInt32 i=0; i<workers * 4; ++i )
                futures.push_back( idlePool->QueueTask( CQueuedTickTask::TaskType ) );
            bool allDone = false;
            for ( UInt32 waited=0; !allDone && waited<2000; ++waited )
            {
                allDone = true;
                for ( UInt32 i=0; i<futures.size() && allDone; ++i )
                    allDone = futures[ i ].IsReady();
                if ( !allDone )
                    MT::ThreadDelay( 1 );
            }
            Float64 burstMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - burstTicks );
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: " << futures.size() << " tasks queued at once on " << workers << " idle workers done after " << burstMs << " ms" );
            ASSERT_TRUE( allDone );
            ASSERT_TRUE( burstMs < 250.0 );

            // the request to stop ends the wait for a task: the threads end at once, not after g_idleTaskWaitMs each
            MT::ThreadDelay( 100 );
            UInt64 stopTicks = MT::PrecisionTickCount();
            ASSERT_TRUE( idlePool->RequestAllThreadsToStop( true, false, 5000 ) );
            Float64 stopMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - stopTicks );
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: " << workers << " idle worker threads stopped in " << stopMs << " ms" );
            ASSERT_TRUE( stopMs < (Float64) g_idleTaskWaitMs );

            taskManager.UnregisterThreadPool( g_idlePoolName );
            taskManager.UnregisterTaskConsumerFactory( CQueuedTickTask::TaskType );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    // WaitForAllTasksToFinish() and GetAllTaskIdsForTasksWithWork() take the pool lock and the lock of the task queue while the
    // workers take tasks out of the queue, start them and end them: neither side may wait for the other for good
    GUCEF_TESTFW_TESTCASE( "Waiting for all tasks of a pool and asking for the tasks with work while tasks are queued, started and ended by the workers: no stall" )
        try
        {
            typedef CTFactory< CTaskConsumer, CQueuedTickTask, MT::CMutex > TQueuedTickTaskFactory;
            TQueuedTickTaskFactory factory;
            CTaskManager& taskManager = CCoreGlobal::Instance()->GetTaskManager();
            taskManager.RegisterTaskConsumerFactory( CQueuedTickTask::TaskType, &factory );

            const char* waitAllPoolName = "gucefCORE_TestApp.TaskDelegatorWakeUp.WaitAll";
            const UInt32 workers = 4;
            ThreadPoolPtr waitAllPool = taskManager.GetOrCreateThreadPool( waitAllPoolName );
            REQUIRE_TRUE( !waitAllPool.IsNULL() );
            waitAllPool->SetDesiredMaxTotalNrOfThreads( (Int32) workers );
            waitAllPool->SetDesiredMinNrOfWorkerThreads( workers );
            for ( UInt32 i=0; i<500 && waitAllPool->GetActiveNrOfWorkerThreads() < workers; ++i )
                MT::ThreadDelay( 5 );
            REQUIRE_TRUE( workers == waitAllPool->GetActiveNrOfWorkerThreads() );

            // the case stays when the thread did not end: that thread may still use it
            SWaitAllCase* waitCase = GUCEF_NEW SWaitAllCase( waitAllPool );
            MT::SThreadData* waitThread = MT::ThreadDataReserve();
            REQUIRE_TRUE( GUCEF_NULL != waitThread );
            REQUIRE_TRUE( 0 != MT::ThreadCreate( waitThread, &WaitAllThreadMain, waitCase ) );

            bool allDone = true;
            const UInt32 rounds = 150;
            for ( UInt32 r=0; r<rounds && allDone; ++r )
            {
                GUCEF::vector< CFutureResult > futures;
                for ( UInt32 i=0; i<workers * 2; ++i )
                    futures.push_back( waitAllPool->QueueTask( CQueuedTickTask::TaskType ) );
                bool roundDone = false;
                for ( UInt32 waited=0; !roundDone && waited<5000; ++waited )
                {
                    roundDone = true;
                    for ( UInt32 i=0; i<futures.size() && roundDone; ++i )
                        roundDone = futures[ i ].IsReady();
                    if ( !roundDone )
                        MT::ThreadDelay( 1 );
                }
                allDone = roundDone;
                MT::ThreadDelay( r % 3 );
            }
            ASSERT_TRUE( allDone );

            const UInt32 waitsBefore = waitCase->waits.Load();
            MT::ThreadDelay( 100 );
            ASSERT_TRUE( waitCase->waits.Load() > waitsBefore || 0 != waitCase->failedWaits.Load() );
            waitCase->stop.Store( true, MT::MEMORDER_RELEASE );
            const bool ended = GUCEF_THREAD_WAIT_OK == MT::ThreadWait( waitThread, 20000 );
            ASSERT_TRUE( ended );
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: waits for all tasks: " << waitCase->waits.Load() << ", scans for tasks with work: "
                       << waitCase->taskIdScans.Load() << ", waits that did not succeed: " << waitCase->failedWaits.Load() );
            ASSERT_TRUE( waitCase->waits.Load() > 0 );

            ASSERT_TRUE( waitAllPool->RequestAllThreadsToStop( true, false, 5000 ) );
            if ( ended )
            {
                MT::ThreadDataCleanup( waitThread );
                GUCEF_DELETE waitCase;
            }
            taskManager.UnregisterThreadPool( waitAllPoolName );
            taskManager.UnregisterTaskConsumerFactory( CQueuedTickTask::TaskType );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "A pool told its size twice in a row keeps that size when a worker it asked to stop has ended in between but is still listed: the ended worker counts as gone, no second worker is asked to stop" )
        try
        {
            CTaskManager& taskManager = CCoreGlobal::Instance()->GetTaskManager();
            const UInt32 workers = 4;
            ThreadPoolPtr shrinkPool = taskManager.GetOrCreateThreadPool( g_shrinkPoolName );
            REQUIRE_TRUE( !shrinkPool.IsNULL() );
            shrinkPool->SetDesiredMaxTotalNrOfThreads( (Int32) workers );
            shrinkPool->SetDesiredMinNrOfWorkerThreads( workers );
            for ( UInt32 i=0; i<500 && shrinkPool->GetActiveNrOfWorkerThreads() < workers; ++i )
                MT::ThreadDelay( 5 );
            REQUIRE_TRUE( workers == shrinkPool->GetActiveNrOfWorkerThreads() );
            CThreadPool::TThreadIdVector threadIds;
            shrinkPool->GetAllCurrentThreadIds( threadIds );
            REQUIRE_TRUE( workers == threadIds.size() );
            // where the threads of the process cannot be looked at (no /proc) there is nothing to measure
            const bool canCount = CountLiveThreads( threadIds ) >= 0;
            MT::ThreadDelay( 100 );

            // one worker too many: it is asked to stop and, waiting for a task, ends at once. Its end reaches the pool
            // as a notification on the pulse thread, which this test does not pump: the pool still lists it
            shrinkPool->SetDesiredMaxTotalNrOfThreads( (Int32) workers - 1 );
            for ( UInt32 i=0; canCount && i<400 && CountLiveThreads( threadIds ) > (Int32) workers - 1; ++i )
                MT::ThreadDelay( 5 );
            REQUIRE_TRUE( !canCount || (Int32) workers - 1 == CountLiveThreads( threadIds ) );

            // the same size again, as a caller that sets the maximum and then the minimum does
            shrinkPool->SetDesiredMinNrOfWorkerThreads( workers - 1 );
            MT::ThreadDelay( 300 );
            const Int32 live = CountLiveThreads( threadIds );
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: " << live << " of " << workers << " workers run after the pool was told " << ( workers - 1 ) << " twice" );
            ASSERT_TRUE( !canCount || (Int32) workers - 1 == live );

            ASSERT_TRUE( shrinkPool->RequestAllThreadsToStop( true, false, 5000 ) );
            taskManager.UnregisterThreadPool( g_shrinkPoolName );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "A dedicated task between cycles a long delay apart, without periodic pulses, sleeps the whole delay instead of waking at every pulse window; an immediate pulse request still wakes it, a stop ends it at once" )
        try
        {
            const char* delayPoolName = "gucefCORE_TestApp.TaskDelegatorWakeUp.LongDelay";
            ThreadPoolPtr delayPool = CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( delayPoolName );
            REQUIRE_TRUE( !delayPool.IsNULL() );
            CLongDelayTaskPtr delayTask( GUCEF_NEW CLongDelayTask() );
            REQUIRE_TRUE( delayPool->SetupSingularTask( delayTask ).HasAFuture() && delayPool->StartTaskWithConsumer( delayTask ).HasAFuture() );
            for ( UInt32 i=0; i<400 && 0 == delayTask->GetCycles(); ++i )
                MT::ThreadDelay( 5 );
            REQUIRE_TRUE( 1 <= delayTask->GetCycles() );
            CThreadPool::TThreadIdVector threadIds;
            TSystemThreadId threadId = 0;
            REQUIRE_TRUE( delayPool->GetThreadIdForTaskId( delayTask->GetCurrentTaskId(), threadId ) && 0 != threadId );
            threadIds.push_back( threadId );

            // before, the delegator woke at every pulse window of its pulse generator (10 ms) for the whole delay
            MT::ThreadDelay( 200 );
            Int64 before = CountContextSwitches( threadIds );
            MT::ThreadDelay( 2000 );
            Int64 after = CountContextSwitches( threadIds );
            if ( before >= 0 && after >= 0 )
            {
                GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: a task between cycles 60 s apart woke " << ( after - before ) << " times in 2000 ms" );
                ASSERT_TRUE( after - before <= 6 );
            }

            // an immediate pulse request ends the delay: the task is pulsed at once
            CPulseClock clock;
            clock.Watch( delayTask->GetPulseGenerator() );
            UInt32 pulsesBefore = clock.GetPulses();
            UInt64 requestTicks = MT::PrecisionTickCount();
            delayTask->GetPulseGenerator()->RequestImmediatePulse();
            Float64 latency = WaitForPulseAfter( clock, pulsesBefore, requestTicks );
            clock.UnsubscribeAllFromObserver();
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: an immediate pulse request during a long delay is pulsed after " << latency << " ms" );
            ASSERT_TRUE( latency >= 0.0 && latency < 50.0 );

            UInt64 stopTicks = MT::PrecisionTickCount();
            delayTask->GetPulseGenerator()->AllowPeriodicPulses();
            ASSERT_TRUE( delayTask->RequestTaskToStop( true ) );
            Float64 stopMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - stopTicks );
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: the task during a long delay stopped in " << stopMs << " ms" );
            ASSERT_TRUE( stopMs < 1000.0 );
            CCoreGlobal::Instance()->GetTaskManager().UnregisterThreadPool( delayPoolName );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "An immediate pulse request made on another thread while a task's cycle runs, after the cycle took its input, ends the cycle delay the task asked for: the next cycle follows at once, as for a request made during the delay; without a request the task sleeps its delay" )
        try
        {
            const char* gatedPoolName = "gucefCORE_TestApp.TaskDelegatorWakeUp.GatedCycle";
            ThreadPoolPtr gatedPool = CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( gatedPoolName );
            REQUIRE_TRUE( !gatedPool.IsNULL() );
            CGatedCycleTaskPtr gatedTask( GUCEF_NEW CGatedCycleTask() );
            REQUIRE_TRUE( gatedPool->SetupSingularTask( gatedTask ).HasAFuture() && gatedPool->StartTaskWithConsumer( gatedTask ).HasAFuture() );
            for ( UInt32 i=0; i<400 && 0 == gatedTask->GetCycles(); ++i )
                MT::ThreadDelay( 5 );
            REQUIRE_TRUE( 1 <= gatedTask->GetCycles() );
            PulseGeneratorPtr gatedPulseGenerator = gatedTask->GetPulseGenerator();
            REQUIRE_TRUE( !gatedPulseGenerator.IsNULL() );

            // the losing order, forced: the request comes while the cycle is held at its gate, after it took its input
            const UInt32 rounds = 20;
            UInt32 lost = 0;
            for ( UInt32 r=0; r<rounds; ++r )
            {
                gatedTask->ArmGate();
                UInt32 before = gatedTask->GetCycles();
                gatedPulseGenerator->RequestImmediatePulse();    // ends the delay: the next cycle stops at the gate
                for ( UInt32 waited=0; waited<2000 && !gatedTask->IsInGate(); ++waited )
                    MT::ThreadDelay( 1 );
                REQUIRE_TRUE( gatedTask->IsInGate() && gatedTask->GetCycles() == before + 1 );
                const UInt32 inCycle = gatedTask->GetCycles();
                gatedPulseGenerator->RequestImmediatePulse();    // new input for the task, from another thread, during its cycle
                gatedTask->OpenGate();
                if ( !WaitForCycleAfter( *gatedTask, inCycle, 1000 ) )
                    ++lost;
            }
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: requests made during a cycle that asks for a 60 s delay, without a cycle within 1000 ms after them: " << lost << " of " << rounds );
            ASSERT_TRUE( 0 == lost );

            // without a request during its cycle the task still sleeps the delay it asked for: no cycle follows
            MT::ThreadDelay( 50 );
            const UInt32 settled = gatedTask->GetCycles();
            MT::ThreadDelay( 500 );
            ASSERT_TRUE( settled == gatedTask->GetCycles() );

            ASSERT_TRUE( gatedTask->RequestTaskToStop( true ) );
            CCoreGlobal::Instance()->GetTaskManager().UnregisterThreadPool( gatedPoolName );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    // After RequestAllThreadsToStop( true, ... ) joined the threads of a pool whose delegator ran a dedicated task, waiting
    // for the threads to finish again must succeed: a thread that was joined has finished (ThreadWait() answered failure
    // for a joined thread, and the pool reported that as not finished)
    GUCEF_TESTFW_TESTCASE( "After all threads of a pool with a dedicated task were asked to stop, waiting for them to finish succeeds" )
        try
        {
            const char* stopWaitPoolName = "gucefCORE_TestApp.TaskDelegatorWakeUp.StopWait";
            ThreadPoolPtr stopPool = CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( stopWaitPoolName );
            CWakeUpTestTaskPtr stopTask( GUCEF_NEW CWakeUpTestTask() );
            PulseGeneratorPtr stopPulseGenerator;
            if ( !stopPool.IsNULL() )
                stopPulseGenerator = StartDedicatedTask( stopPool, stopTask );
            ASSERT_TRUE( !stopPulseGenerator.IsNULL() );
            if ( !stopPool.IsNULL() && !stopPulseGenerator.IsNULL() )
            {
                ASSERT_TRUE( stopPool->RequestAllThreadsToStop( true, false, 5000 ) );
                bool finished = stopPool->WaitForAllThreadsToFinish( 5000 );
                GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: WaitForAllThreadsToFinish( 5000 ) after RequestAllThreadsToStop returned "
                           << ( finished ? "true" : "false" ) );
                ASSERT_TRUE( finished );
            }
            CCoreGlobal::Instance()->GetTaskManager().UnregisterThreadPool( stopWaitPoolName );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    // The delegator thread clears the delegator of the consumer when its task ends (CTaskConsumer::SetTaskDelegator(), from
    // TaskCleanup()) while another thread asks the consumer: no data race (ThreadSanitizer), whatever the interleaving
    GUCEF_TESTFW_TESTCASE( "A task consumer asked whether it is active or asked to stop by another thread while its task ends: no data race" )
        try
        {
            const char* pollPoolName = "gucefCORE_TestApp.TaskDelegatorWakeUp.ConsumerPoll";
            ThreadPoolPtr pollPool = CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( pollPoolName );
            ASSERT_TRUE( !pollPool.IsNULL() );
            UInt32 roundsWithActive = 0;
            UInt32 totalPolls = 0;
            const UInt32 rounds = 60;
            for ( UInt32 r=0; r<rounds && !pollPool.IsNULL(); ++r )
            {
                CWakeUpTestTaskPtr pollTask( GUCEF_NEW CWakeUpTestTask() );
                PulseGeneratorPtr pollPulseGenerator = StartDedicatedTask( pollPool, pollTask );
                ASSERT_TRUE( !pollPulseGenerator.IsNULL() );
                if ( pollPulseGenerator.IsNULL() )
                    break;

                SConsumerPoll poll( pollTask );
                MT::SThreadData* pollThread = MT::ThreadDataReserve();
                ASSERT_TRUE( GUCEF_NULL != pollThread );
                ASSERT_TRUE( 0 != MT::ThreadCreate( pollThread, &ConsumerPollThreadMain, &poll ) );

                MT::ThreadDelay( 2 );
                ASSERT_TRUE( pollTask->RequestTaskToStop( true ) );
                MT::ThreadDelay( 2 );

                poll.stop.Store( true, MT::MEMORDER_RELEASE );
                ASSERT_TRUE( GUCEF_THREAD_WAIT_OK == MT::ThreadWait( pollThread, 10000 ) );
                MT::ThreadDataCleanup( pollThread );
                ASSERT_TRUE( !pollTask->IsActive() );
                if ( poll.sawActive.Load() )
                    ++roundsWithActive;
                totalPolls += poll.polls.Load();
            }
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: consumer polls: " << totalPolls << " polls, the task was seen active in "
                       << roundsWithActive << " rounds" );
            ASSERT_TRUE( totalPolls > 0 );
            ASSERT_TRUE( roundsWithActive > 0 );
            CCoreGlobal::Instance()->GetTaskManager().UnregisterThreadPool( pollPoolName );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    // A worker that has just finished a task must take the next queued task at once, and otherwise wait for the pool's
    // wake-up: it must not sleep the cycle delay of the active object (10 ms) after the task, as a task queued then waited
    GUCEF_TESTFW_TESTCASE( "A pool worker that finished a task picks up the next queued task at once: tasks queued one after the other, a backlog queued at once and tasks of several producers are not paced by the cycle delay, and each runs exactly once" )
        try
        {
            CTaskManager& taskManager = CCoreGlobal::Instance()->GetTaskManager();
            TTinyTaskFactory factory;
            taskManager.RegisterTaskConsumerFactory( CTinyTask::TaskType, &factory );
            g_tinyTaskHoldMs.Store( 0 );

            ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( g_backToBackPoolName );
            REQUIRE_TRUE( !pool.IsNULL() );
            pool->SetDesiredMaxTotalNrOfThreads( 1 );
            pool->SetDesiredMinNrOfWorkerThreads( 1 );
            for ( UInt32 i=0; i<500 && 1 != pool->GetActiveNrOfWorkerThreads(); ++i )
                MT::ThreadDelay( 5 );
            REQUIRE_TRUE( 1 == pool->GetActiveNrOfWorkerThreads() );
            MT::ThreadDelay( 50 );

            // one after the other, the worker the only one: half of the tasks are queued the moment the previous one ended
            // (the worker is still ending it), half a little later (the worker waits for a task again)
            GUCEF::vector< Float64 > latencies;
            const UInt32 sequentialTasks = 100;
            const UInt64 sequenceStartTicks = MT::PrecisionTickCount();
            for ( UInt32 i=0; i<sequentialTasks; ++i )
            {
                g_tinyTaskLastStartTicks.Store( 0 );
                const UInt64 queuedTicks = MT::PrecisionTickCount();
                CFutureResult future = pool->QueueTask( CTinyTask::TaskType );
                REQUIRE_TRUE( future.HasAFuture() );
                REQUIRE_TRUE( WaitForReady( future, 2000 ) );
                const UInt64 startTicks = g_tinyTaskLastStartTicks.Load();
                ASSERT_TRUE( 0 != startTicks );
                if ( 0 != startTicks && startTicks >= queuedTicks )
                    latencies.push_back( MT::PrecisionTimerTicksToMs( startTicks - queuedTicks ) );
                if ( 0 != ( i % 2 ) )
                    MT::ThreadDelay( 1 );
            }
            const Float64 sequenceMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - sequenceStartTicks );
            const Float64 median = LatencyAt( latencies, 0.5 );
            const Float64 worst = LatencyAt( latencies, 1.0 );
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: " << sequentialTasks << " tasks queued one after the other on a pool of one worker: queued to started median "
                       << median << " ms, worst " << worst << " ms, all done after " << sequenceMs << " ms" );
            ASSERT_TRUE( median >= 0.0 && median < g_medianWakeLimitInMs * 2.0 );
            ASSERT_TRUE( sequenceMs < (Float64) sequentialTasks * (Float64) g_minimalCycleDeltaInMs / 2.0 );

            // a backlog queued at once: the one worker takes the tasks without a pause between them
            g_tinyTaskRuns.Store( 0 );
            const UInt32 backlog = 200;
            GUCEF::vector< CFutureResult > futures;
            const UInt64 backlogStartTicks = MT::PrecisionTickCount();
            for ( UInt32 i=0; i<backlog; ++i )
                futures.push_back( pool->QueueTask( CTinyTask::TaskType ) );
            bool allDone = true;
            for ( UInt32 i=0; i<futures.size(); ++i )
                allDone = WaitForReady( futures[ i ], 5000 ) && allDone;
            const Float64 backlogMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - backlogStartTicks );
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: a backlog of " << backlog << " tasks on a pool of one worker done after " << backlogMs << " ms" );
            ASSERT_TRUE( allDone );
            ASSERT_TRUE( backlog == g_tinyTaskRuns.Load() );
            ASSERT_TRUE( backlogMs < (Float64) backlog * (Float64) g_minimalCycleDeltaInMs / 4.0 );

            // several producer threads queue to a pool of two workers: every task runs once, none is lost or run twice
            ThreadPoolPtr producersPool = taskManager.GetOrCreateThreadPool( g_producersPoolName );
            REQUIRE_TRUE( !producersPool.IsNULL() );
            producersPool->SetDesiredMaxTotalNrOfThreads( 2 );
            producersPool->SetDesiredMinNrOfWorkerThreads( 2 );
            for ( UInt32 i=0; i<500 && 2 != producersPool->GetActiveNrOfWorkerThreads(); ++i )
                MT::ThreadDelay( 5 );
            REQUIRE_TRUE( 2 == producersPool->GetActiveNrOfWorkerThreads() );
            g_tinyTaskRuns.Store( 0 );
            const UInt32 producerCount = 4;
            const UInt32 tasksPerProducer = 250;
            CTinyTaskProducer* producers[ producerCount ];
            const UInt64 producersStartTicks = MT::PrecisionTickCount();
            for ( UInt32 i=0; i<producerCount; ++i )
            {
                producers[ i ] = GUCEF_NEW CTinyTaskProducer( producersPool, tasksPerProducer );
                producers[ i ]->Activate();
            }
            UInt32 queuedByProducers = 0;
            for ( UInt32 i=0; i<producerCount; ++i )
            {
                producers[ i ]->WaitForThreadToFinish( 10000 );
                queuedByProducers += producers[ i ]->GetQueued();
                ASSERT_TRUE( 0 == producers[ i ]->GetFailed() );
            }
            for ( UInt32 waited=0; waited<10000 && g_tinyTaskRuns.Load() < queuedByProducers; ++waited )
                MT::ThreadDelay( 1 );
            MT::ThreadDelay( 50 );
            const Float64 producersMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - producersStartTicks );
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: " << producerCount << " producer threads queued " << queuedByProducers << " tasks, " << g_tinyTaskRuns.Load()
                       << " ran, done after " << producersMs << " ms" );
            ASSERT_TRUE( producerCount * tasksPerProducer == queuedByProducers );
            ASSERT_TRUE( queuedByProducers == g_tinyTaskRuns.Load() );
            ASSERT_TRUE( 0 == producersPool->GetNrOfQueuedTasks() );
            for ( UInt32 i=0; i<producerCount; ++i )
                GUCEF_DELETE producers[ i ];

            // bursts: as many tasks as workers queued at once to workers that have just ended a task or wait for one all begin at
            // once, each on a worker of its own: a wake that is spent on a worker that has its task already leaves one idle while
            // a task of the burst waits for the length of another task
            ThreadPoolPtr burstPool = taskManager.GetOrCreateThreadPool( g_burstPoolName );
            REQUIRE_TRUE( !burstPool.IsNULL() );
            const UInt32 burstWorkers = 4;
            burstPool->SetDesiredMaxTotalNrOfThreads( (Int32) burstWorkers );
            burstPool->SetDesiredMinNrOfWorkerThreads( burstWorkers );
            for ( UInt32 i=0; i<500 && burstWorkers != burstPool->GetActiveNrOfWorkerThreads(); ++i )
                MT::ThreadDelay( 5 );
            REQUIRE_TRUE( burstWorkers == burstPool->GetActiveNrOfWorkerThreads() );
            const UInt32 holdMs = 15;
            const UInt32 bursts = 80;
            UInt32 lateBursts = 0;
            Float64 worstSpreadMs = 0.0;
            for ( UInt32 b=0; b<bursts; ++b )
            {
                g_tinyTaskHoldMs.Store( holdMs );
                g_tinyTaskRuns.Store( 0 );
                GUCEF::vector< CFutureResult > burstFutures;
                for ( UInt32 i=0; i<burstWorkers; ++i )
                    burstFutures.push_back( burstPool->QueueTask( CTinyTask::TaskType ) );
                bool burstDone = true;
                for ( UInt32 i=0; i<burstFutures.size(); ++i )
                    burstDone = WaitForReady( burstFutures[ i ], 5000 ) && burstDone;
                ASSERT_TRUE( burstDone );
                UInt64 first = g_tinyTaskStartLog[ 0 ];
                UInt64 last = g_tinyTaskStartLog[ 0 ];
                for ( UInt32 i=1; i<burstWorkers; ++i )
                {
                    if ( g_tinyTaskStartLog[ i ] < first )
                        first = g_tinyTaskStartLog[ i ];
                    if ( g_tinyTaskStartLog[ i ] > last )
                        last = g_tinyTaskStartLog[ i ];
                }
                const Float64 spreadMs = MT::PrecisionTimerTicksToMs( last - first );
                if ( spreadMs > worstSpreadMs )
                    worstSpreadMs = spreadMs;
                if ( spreadMs > (Float64) holdMs / 2.0 )
                    ++lateBursts;
                MT::ThreadDelay( b % 5 );
            }
            g_tinyTaskHoldMs.Store( 0 );
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: " << bursts << " bursts of " << burstWorkers << " tasks to " << burstWorkers << " workers: " << lateBursts << " with a task that began late, worst spread " << worstSpreadMs << " ms" );
            ASSERT_TRUE( lateBursts <= 1 );
            ASSERT_TRUE( burstPool->RequestAllThreadsToStop( true, false, 5000 ) );
            taskManager.UnregisterThreadPool( g_burstPoolName );

            MT::ThreadDelay( 20 );
            const UInt64 stopTicks = MT::PrecisionTickCount();
            ASSERT_TRUE( pool->RequestAllThreadsToStop( true, false, 5000 ) );
            ASSERT_TRUE( producersPool->RequestAllThreadsToStop( true, false, 5000 ) );
            const Float64 stopMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - stopTicks );
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: two pools whose workers just ended their last tasks stopped in " << stopMs << " ms" );
            ASSERT_TRUE( stopMs < 250.0 );
            taskManager.UnregisterThreadPool( g_backToBackPoolName );
            taskManager.UnregisterThreadPool( g_producersPoolName );
            taskManager.UnregisterTaskConsumerFactory( CTinyTask::TaskType );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    // A worker thread that starts together with its pool begins in the wait for a task, not in the first cycle delay of
    // the active object (25 ms), during which a task that is queued is not seen
    GUCEF_TESTFW_TESTCASE( "A task queued to a pool whose worker has just started begins at once, not after the first cycle delay of the worker" )
        try
        {
            CTaskManager& taskManager = CCoreGlobal::Instance()->GetTaskManager();
            TTinyTaskFactory factory;
            taskManager.RegisterTaskConsumerFactory( CTinyTask::TaskType, &factory );
            g_tinyTaskHoldMs.Store( 0 );

            GUCEF::vector< Float64 > latencies;
            for ( UInt32 i=0; i<6; ++i )
            {
                ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( g_startupPoolName );
                REQUIRE_TRUE( !pool.IsNULL() );
                pool->SetDesiredMaxTotalNrOfThreads( 1 );
                pool->SetDesiredMinNrOfWorkerThreads( 1 );
                for ( UInt32 w=0; w<500 && 1 != pool->GetActiveNrOfWorkerThreads(); ++w )
                    MT::ThreadDelay( 1 );
                REQUIRE_TRUE( 1 == pool->GetActiveNrOfWorkerThreads() );

                // the worker had its first cycle by now
                MT::ThreadDelay( 3 );
                g_tinyTaskLastStartTicks.Store( 0 );
                const UInt64 queuedTicks = MT::PrecisionTickCount();
                CFutureResult future = pool->QueueTask( CTinyTask::TaskType );
                REQUIRE_TRUE( future.HasAFuture() );
                REQUIRE_TRUE( WaitForReady( future, 2000 ) );
                const UInt64 startTicks = g_tinyTaskLastStartTicks.Load();
                ASSERT_TRUE( 0 != startTicks );
                if ( 0 != startTicks && startTicks >= queuedTicks )
                    latencies.push_back( MT::PrecisionTimerTicksToMs( startTicks - queuedTicks ) );

                ASSERT_TRUE( pool->RequestAllThreadsToStop( true, false, 5000 ) );
                taskManager.UnregisterThreadPool( g_startupPoolName );
            }
            const Float64 median = LatencyAt( latencies, 0.5 );
            const Float64 worst = LatencyAt( latencies, 1.0 );
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: first task on a freshly started worker, over " << latencies.size() << " pools: median " << median << " ms, worst " << worst << " ms" );
            ASSERT_TRUE( 6 == latencies.size() );
            ASSERT_TRUE( median >= 0.0 && median < 10.0 );

            taskManager.UnregisterTaskConsumerFactory( CTinyTask::TaskType );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    // The worker that waits for a task after its last one must not spin, and not wake more often than the idle workers do
    GUCEF_TESTFW_TESTCASE( "Workers that just finished work wait for the next task without spinning: no processor time and no more than a wake-up per idle wait over a window" )
        try
        {
            CTaskManager& taskManager = CCoreGlobal::Instance()->GetTaskManager();
            TTinyTaskFactory factory;
            taskManager.RegisterTaskConsumerFactory( CTinyTask::TaskType, &factory );
            g_tinyTaskHoldMs.Store( 0 );

            const UInt32 workers = 2;
            ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( g_backToBackPoolName );
            REQUIRE_TRUE( !pool.IsNULL() );
            pool->SetDesiredMaxTotalNrOfThreads( (Int32) workers );
            pool->SetDesiredMinNrOfWorkerThreads( workers );
            for ( UInt32 i=0; i<500 && workers != pool->GetActiveNrOfWorkerThreads(); ++i )
                MT::ThreadDelay( 5 );
            REQUIRE_TRUE( workers == pool->GetActiveNrOfWorkerThreads() );
            CThreadPool::TThreadIdVector threadIds;
            pool->GetAllCurrentThreadIds( threadIds );
            REQUIRE_TRUE( workers == threadIds.size() );

            // work first, so that both workers end in the wait that follows a task
            GUCEF::vector< CFutureResult > futures;
            for ( UInt32 i=0; i<40; ++i )
                futures.push_back( pool->QueueTask( CTinyTask::TaskType ) );
            for ( UInt32 i=0; i<futures.size(); ++i )
                ASSERT_TRUE( WaitForReady( futures[ i ], 5000 ) );
            MT::ThreadDelay( 200 );

            const Int64 cpuBefore = CountThreadCpuNs( threadIds );
            const Int64 switchesBefore = CountContextSwitches( threadIds );
            const UInt32 windowMs = 2000;
            MT::ThreadDelay( windowMs );
            const Int64 cpuAfter = CountThreadCpuNs( threadIds );
            const Int64 switchesAfter = CountContextSwitches( threadIds );
            if ( cpuBefore >= 0 && cpuAfter >= 0 )
            {
                const Float64 cpuMs = (Float64) ( cpuAfter - cpuBefore ) / 1000000.0;
                GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: " << workers << " workers that ended a task used " << cpuMs << " ms of processor time in " << windowMs << " ms of waiting" );
                ASSERT_TRUE( cpuMs < 20.0 );
            }
            if ( switchesBefore >= 0 && switchesAfter >= 0 )
            {
                const Int64 wakes = switchesAfter - switchesBefore;
                const Int64 limit = (Int64) workers * ( windowMs / g_idleTaskWaitMs + 2 );
                GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: " << workers << " workers that ended a task woke " << wakes << " times in " << windowMs << " ms (limit " << limit << ")" );
                ASSERT_TRUE( wakes <= limit );
            }

            ASSERT_TRUE( pool->RequestAllThreadsToStop( true, false, 5000 ) );
            taskManager.UnregisterThreadPool( g_backToBackPoolName );
            taskManager.UnregisterTaskConsumerFactory( CTinyTask::TaskType );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    // A worker that is asked to stop while tasks are queued ends after the task it carries out, it does not first work through the queue
    GUCEF_TESTFW_TESTCASE( "Workers of a pool that is made smaller while a backlog is queued end after their current task: the queued tasks stay for the workers that remain" )
        try
        {
            CTaskManager& taskManager = CCoreGlobal::Instance()->GetTaskManager();
            TTinyTaskFactory factory;
            taskManager.RegisterTaskConsumerFactory( CTinyTask::TaskType, &factory );

            const UInt32 workers = 3;
            ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( g_busyShrinkPoolName );
            REQUIRE_TRUE( !pool.IsNULL() );
            pool->SetDesiredMaxTotalNrOfThreads( (Int32) workers );
            pool->SetDesiredMinNrOfWorkerThreads( workers );
            for ( UInt32 i=0; i<500 && workers != pool->GetActiveNrOfWorkerThreads(); ++i )
                MT::ThreadDelay( 5 );
            REQUIRE_TRUE( workers == pool->GetActiveNrOfWorkerThreads() );
            CThreadPool::TThreadIdVector threadIds;
            pool->GetAllCurrentThreadIds( threadIds );
            REQUIRE_TRUE( workers == threadIds.size() );
            const bool canCount = CountLiveThreads( threadIds ) >= 0;
            MT::ThreadDelay( 100 );

            // 90 tasks of 20 ms: three workers need 600 ms, one needs 1.8 s
            g_tinyTaskRuns.Store( 0 );
            g_tinyTaskHoldMs.Store( 20 );
            const UInt32 backlog = 90;
            GUCEF::vector< CFutureResult > futures;
            for ( UInt32 i=0; i<backlog; ++i )
                futures.push_back( pool->QueueTask( CTinyTask::TaskType ) );
            for ( UInt32 waited=0; waited<1000 && g_tinyTaskRuns.Load() < 6; ++waited )
                MT::ThreadDelay( 1 );

            pool->SetDesiredMaxTotalNrOfThreads( 1 );
            const UInt64 shrinkTicks = MT::PrecisionTickCount();
            for ( UInt32 i=0; canCount && i<400 && CountLiveThreads( threadIds ) > 1; ++i )
                MT::ThreadDelay( 1 );
            const Float64 shrinkMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - shrinkTicks );
            const UInt32 runsWhenShrunk = g_tinyTaskRuns.Load();
            const UInt32 queuedWhenShrunk = pool->GetNrOfQueuedTasks();
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: workers of a busy pool asked to stop ended after " << shrinkMs << " ms, " << runsWhenShrunk << " of " << backlog
                       << " tasks had run, " << queuedWhenShrunk << " still queued, " << CountLiveThreads( threadIds ) << " of " << workers << " threads live" );
            ASSERT_TRUE( !canCount || 1 == CountLiveThreads( threadIds ) );
            ASSERT_TRUE( !canCount || shrinkMs < 300.0 );
            ASSERT_TRUE( queuedWhenShrunk > 20 );

            // the worker that remains works the rest off, none of the tasks is lost
            g_tinyTaskHoldMs.Store( 0 );
            bool allDone = true;
            for ( UInt32 i=0; i<futures.size(); ++i )
                allDone = WaitForReady( futures[ i ], 10000 ) && allDone;
            ASSERT_TRUE( allDone );
            ASSERT_TRUE( backlog == g_tinyTaskRuns.Load() );

            ASSERT_TRUE( pool->RequestAllThreadsToStop( true, false, 5000 ) );
            taskManager.UnregisterThreadPool( g_busyShrinkPoolName );
            taskManager.UnregisterTaskConsumerFactory( CTinyTask::TaskType );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    #ifdef GUCEF_TEST_HAVE_PROC_THREAD_STATUS

    // A pause of a worker's task is taken between two tasks (where the system can suspend threads); the worker serves the queue afterwards
    GUCEF_TESTFW_TESTCASE( "A worker that was asked to pause while it carried out a task, and to resume, still serves the queued tasks, and the pool stops at once" )
        try
        {
            CTaskManager& taskManager = CCoreGlobal::Instance()->GetTaskManager();
            TTinyTaskFactory factory;
            taskManager.RegisterTaskConsumerFactory( CTinyTask::TaskType, &factory );

            ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( g_pausePoolName );
            REQUIRE_TRUE( !pool.IsNULL() );
            pool->SetDesiredMaxTotalNrOfThreads( 1 );
            pool->SetDesiredMinNrOfWorkerThreads( 1 );
            for ( UInt32 i=0; i<500 && 1 != pool->GetActiveNrOfWorkerThreads(); ++i )
                MT::ThreadDelay( 5 );
            REQUIRE_TRUE( 1 == pool->GetActiveNrOfWorkerThreads() );

            g_tinyTaskRuns.Store( 0 );
            g_tinyTaskHoldMs.Store( 200 );
            CFutureResult holding = pool->QueueTask( CTinyTask::TaskType );
            REQUIRE_TRUE( holding.HasAFuture() );
            for ( UInt32 waited=0; waited<1000 && 0 == g_tinyTaskRuns.Load(); ++waited )
                MT::ThreadDelay( 1 );
            REQUIRE_TRUE( 1 == g_tinyTaskRuns.Load() );

            ASSERT_TRUE( pool->PauseTask( holding.GetTaskId(), false ) );
            ASSERT_TRUE( pool->ResumeTask( holding.GetTaskId() ) );
            g_tinyTaskHoldMs.Store( 0 );

            GUCEF::vector< CFutureResult > futures;
            for ( UInt32 i=0; i<20; ++i )
                futures.push_back( pool->QueueTask( CTinyTask::TaskType ) );
            bool allDone = WaitForReady( holding, 5000 );
            const UInt64 pausedStartTicks = MT::PrecisionTickCount();
            for ( UInt32 i=0; i<futures.size(); ++i )
                allDone = WaitForReady( futures[ i ], 10000 ) && allDone;
            const Float64 pausedMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - pausedStartTicks );
            ASSERT_TRUE( allDone );
            ASSERT_TRUE( 21 == g_tinyTaskRuns.Load() );

            // the request to pause is taken between two tasks: the worker goes back to the active object, which sees it (where
            // threads cannot be suspended, as here, that is all it does, after its cycle delay), instead of taking the whole queue
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: 20 tasks behind a paused task of a worker took " << pausedMs << " ms" );
            ASSERT_TRUE( pausedMs >= 100.0 );

            UInt64 stopTicks = MT::PrecisionTickCount();
            ASSERT_TRUE( pool->RequestAllThreadsToStop( true, false, 5000 ) );
            const Float64 stopMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - stopTicks );
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "TaskDelegatorWakeUp: the worker of a pool whose task was paused and resumed stopped in " << stopMs << " ms" );
            // the worker had just ended its last task: the request to stop ends its wait at once, not when the wait would have ended
            ASSERT_TRUE( stopMs < 250.0 );
            taskManager.UnregisterThreadPool( g_pausePoolName );
            taskManager.UnregisterTaskConsumerFactory( CTinyTask::TaskType );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    #endif /* GUCEF_TEST_HAVE_PROC_THREAD_STATUS ? */

    if ( !task.IsNULL() )
        task->RequestTaskToStop( true );
    CCoreGlobal::Instance()->GetTaskManager().UnregisterThreadPool( g_poolName );
}

/*-------------------------------------------------------------------------*/
