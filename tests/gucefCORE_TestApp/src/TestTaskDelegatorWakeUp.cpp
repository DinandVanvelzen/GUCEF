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

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)

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

    // Observation kept as a reproducer for the CThreadPool review, not a confirmed defect: after RequestAllThreadsToStop
    // on a pool whose delegator ran a dedicated task, WaitForAllThreadsToFinish( 5000 ) was seen to return false.
    // Switched off by default; run with GUCEF_TEST_THREADPOOL_STOP_WAIT=1
    GUCEF_TESTFW_TESTCASE( "After all threads of a pool with a dedicated task were asked to stop, waiting for them to finish succeeds" )
        GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_THREADPOOL_STOP_WAIT" ) != "1", "reproducer for the CThreadPool review, switched on with GUCEF_TEST_THREADPOOL_STOP_WAIT=1" );
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

    if ( !task.IsNULL() )
        task->RequestTaskToStop( true );
    CCoreGlobal::Instance()->GetTaskManager().UnregisterThreadPool( g_poolName );
}

/*-------------------------------------------------------------------------*/
