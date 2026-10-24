/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#include "TestDedicatedTasks.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Counts its cycles and ends itself after a given number */
class CCountingConsumer : public CORE::CTaskConsumer
{
    public:

    explicit CCountingConsumer( UInt32 cyclesToRun ) : CORE::CTaskConsumer(), m_cyclesToRun( cyclesToRun ), cycles( 0 ), ended( false ) {}

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "TestDedicatedTasks::CCountingConsumer"; }

    virtual bool OnTaskCycle( CORE::CTaskPtr ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt32 done = cycles.FetchAdd( 1 ) + 1;
        RequestTaskCycleDelayInMs( 1 );
        return done >= m_cyclesToRun;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr, bool ) GUCEF_VIRTUAL_OVERRIDE { ended.Store( true ); }

    UInt32 m_cyclesToRun;
    MT::CTAtomic< UInt32 > cycles;
    MT::CTAtomic< bool > ended;
};

typedef CORE::CTSharedPtr< CCountingConsumer, MT::CMutex > TCountingConsumerPtr;

/** Task data which counts its live instances */
class CCountedTaskData : public CORE::CICloneable
{
    public:

    static MT::CTAtomic< Int32 > liveInstances;

    CCountedTaskData( void ) : CORE::CICloneable() { liveInstances.FetchAdd( 1 ); }
    CCountedTaskData( const CCountedTaskData& src ) : CORE::CICloneable( src ) { liveInstances.FetchAdd( 1 ); }
    virtual ~CCountedTaskData() { liveInstances.FetchAdd( -1 ); }

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CCountedTaskData( *this ); }
};

MT::CTAtomic< Int32 > CCountedTaskData::liveInstances( 0 );

/** Lets the main thread pulse, which is where the pool gets to hear about threads that finished */
void
PumpFor( UInt32 milliseconds )
{
    UInt32 waited = 0;
    while ( waited < milliseconds )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::PrecisionDelay( 5 );
        waited += 5;
    }
}

bool
WaitUntil( const MT::CTAtomic< bool >& flag, UInt32 timeoutInMs )
{
    UInt32 waited = 0;
    while ( !flag.Load() && waited < timeoutInMs )
    {
        MT::PrecisionDelay( 2 );
        waited += 2;
    }
    return flag.Load();
}

bool
WaitUntilCycled( const MT::CTAtomic< UInt32 >& cycles, UInt32 timeoutInMs )
{
    UInt32 waited = 0;
    while ( 0 == cycles.Load() && waited < timeoutInMs )
    {
        MT::PrecisionDelay( 2 );
        waited += 2;
    }
    return 0 != cycles.Load();
}

/** Counts the task events a pool passes on to its observers */
class CPoolTaskObserver : public CORE::CObserver
{
    public:

    CPoolTaskObserver( void ) : CORE::CObserver(), started( 0 ), killed( 0 ), finished( 0 ) {}

    UInt32 started;
    UInt32 killed;
    UInt32 finished;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( CORE::CThreadPool::TaskStartedEvent == eventid )
            ++started;
        else if ( CORE::CThreadPool::TaskKilledEvent == eventid )
            ++killed;
        else if ( CORE::CThreadPool::TaskFinishedEvent == eventid )
            ++finished;
    }
};

/** Notes the destruction of what it observes */
class CDestructionObserver : public CORE::CObserver
{
    public:

    CDestructionObserver( void ) : CORE::CObserver(), destroyed( false ) {}

    bool destroyed;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( CORE::CNotifier::DestructionEvent == eventid )
            destroyed = true;
    }
};

/** Ends after its first cycle, for tasks which are queued by type */
class CQuickConsumer : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;
    static MT::CTAtomic< Int32 > liveInstances;

    CQuickConsumer( void ) : CORE::CTaskConsumer() { liveInstances.FetchAdd( 1 ); }
    virtual ~CQuickConsumer() { liveInstances.FetchAdd( -1 ); }

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }

    virtual bool OnTaskCycle( CORE::CTaskPtr ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE { CORE::CTaskConsumer::OnTaskEnded( task, wasForced ); }
};

const CORE::CString CQuickConsumer::TaskType = "TestDedicatedTasks::CQuickConsumer";
MT::CTAtomic< Int32 > CQuickConsumer::liveInstances( 0 );

typedef CORE::CTFactory< CORE::CTaskConsumer, CQuickConsumer, MT::CMutex > TQuickConsumerFactory;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDedicatedTaskTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING DedicatedTasks TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "DedicatedTasks" );

    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();

    GUCEF_TESTFW_TESTCASE( "DedicatedTasks: After SetupSingularTask the task does not cycle until it is started, then it does" )
    try
    {
        CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( "DedicatedTasksSetup" );
        TCountingConsumerPtr consumer( GUCEF_NEW CCountingConsumer( 3 ) );

        CORE::CFutureResult setup = pool->SetupSingularTask( consumer );
        ASSERT_FALSE( setup.HasNoFuture() );
        ASSERT_TRUE( 0 != consumer->GetCurrentTaskId() );             // linked to its task from setup onwards
        MT::PrecisionDelay( 200 );                                   // the thread exists and waits
        ASSERT_TRUE( 0 == consumer->cycles.Load() );
        ASSERT_FALSE( consumer->ended.Load() );

        CORE::CFutureResult started = pool->StartTaskWithConsumer( consumer );
        ASSERT_FALSE( started.HasNoFuture() );
        ASSERT_TRUE( WaitUntil( consumer->ended, 5000 ) );           // the thread notices within its wait (25 ms windows)
        ASSERT_TRUE( 3 == consumer->cycles.Load() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DedicatedTasks: A dedicated task that finished is no longer counted as an active dedicated thread of its pool" )
    try
    {
        CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( "DedicatedTasksCleanup" );
        const UInt32 before = pool->GetActiveNrOfDedicatedThreads();

        TCountingConsumerPtr consumer( GUCEF_NEW CCountingConsumer( 2 ) );
        CORE::CFutureResult started = pool->StartTaskWithConsumer( consumer );
        ASSERT_FALSE( started.HasNoFuture() );
        ASSERT_TRUE( WaitUntil( consumer->ended, 5000 ) );
        ASSERT_TRUE( pool->WaitForAllThreadsToFinish( 5000 ) );

        // the pool hears of the finished thread through its mailbox, delivered on the pulses of the main thread
        UInt32 waited = 0;
        while ( pool->GetActiveNrOfDedicatedThreads() != before && waited < 3000 )
        {
            PumpFor( 50 );
            waited += 50;
        }
        ASSERT_TRUE( before == pool->GetActiveNrOfDedicatedThreads() );
        ASSERT_TRUE( before == pool->GetActiveNrOfThreads() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // The pool passes the events of its task consumers on to its own observers. It never heard them (subscriptions to
    // one event without a callback were not recorded by the notifier) and when it did, a started task was passed on as killed
    GUCEF_TESTFW_TESTCASE( "DedicatedTasks: Observers of a pool hear that a task started and finished, not that it was killed" )
    try
    {
        CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( "DedicatedTasksEvents" );
        CPoolTaskObserver observer;
        observer.SubscribeTo( pool.GetPointerAlways() );

        TCountingConsumerPtr consumer( GUCEF_NEW CCountingConsumer( 2 ) );
        CORE::CFutureResult started = pool->StartTaskWithConsumer( consumer );
        ASSERT_FALSE( started.HasNoFuture() );
        ASSERT_TRUE( WaitUntil( consumer->ended, 5000 ) );

        // the pool hears the events of its consumers on the pulses of the main thread
        UInt32 waited = 0;
        while ( 0 == observer.finished && waited < 3000 )
        {
            PumpFor( 50 );
            waited += 50;
        }
        ASSERT_TRUE( 1 == observer.started );
        ASSERT_TRUE( 1 == observer.finished );
        ASSERT_TRUE( 0 == observer.killed );
        observer.UnsubscribeAllFromObserver();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DedicatedTasks: The consumer of a running dedicated task can request it to stop" )
    try
    {
        CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( "DedicatedTasksConsumerStop" );
        TCountingConsumerPtr consumer( GUCEF_NEW CCountingConsumer( GUCEF_UINT32MAX ) );   // never ends by itself

        CORE::CFutureResult started = pool->StartTaskWithConsumer( consumer );
        ASSERT_FALSE( started.HasNoFuture() );
        ASSERT_TRUE( WaitUntilCycled( consumer->cycles, 5000 ) );
        ASSERT_TRUE( 0 != consumer->GetCurrentTaskId() );

        ASSERT_TRUE( consumer->RequestTaskToStop( false ) );
        ASSERT_TRUE( WaitUntil( consumer->ended, 5000 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DedicatedTasks: The pool can stop a running dedicated task via its consumer" )
    try
    {
        CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( "DedicatedTasksPoolStop" );
        TCountingConsumerPtr consumer( GUCEF_NEW CCountingConsumer( GUCEF_UINT32MAX ) );   // never ends by itself

        CORE::CFutureResult started = pool->StartTaskWithConsumer( consumer );
        ASSERT_FALSE( started.HasNoFuture() );
        ASSERT_TRUE( WaitUntilCycled( consumer->cycles, 5000 ) );

        CORE::CTaskConsumerPtr asTaskConsumer = consumer;
        ASSERT_TRUE( pool->RequestTaskToStop( asTaskConsumer, false ) );
        ASSERT_TRUE( WaitUntil( consumer->ended, 5000 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DedicatedTasks: The copy a task makes of task data it does not own is released with the task" )
    try
    {
        CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( "DedicatedTasksTaskData" );
        TCountingConsumerPtr consumer( GUCEF_NEW CCountingConsumer( 1 ) );
        CCountedTaskData data;
        const Int32 before = CCountedTaskData::liveInstances.Load();
        {
            CORE::CFutureResult started = pool->StartTaskWithConsumer( consumer, &data, false );     // the task keeps a copy
            ASSERT_FALSE( started.HasNoFuture() );
            ASSERT_TRUE( WaitUntil( consumer->ended, 5000 ) );
        }
        ASSERT_TRUE( pool->WaitForAllThreadsToFinish( 5000 ) );
        PumpFor( 50 );

        UInt32 waited = 0;
        while ( CCountedTaskData::liveInstances.Load() != before && waited < 3000 )
        {
            MT::PrecisionDelay( 10 );
            waited += 10;
        }
        ASSERT_TRUE( before == CCountedTaskData::liveInstances.Load() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DedicatedTasks: A pool which is no longer registered survives hearing that its dedicated thread finished" )
    try
    {
        // Once unregistered and let go of, the pool is only referenced by its delegator. Handling that delegator's
        // ThreadFinishedEvent must not release the last reference to the pool from within the pool's own event handling
        CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( "DedicatedTasksUnregistered" );
        TCountingConsumerPtr consumer( GUCEF_NEW CCountingConsumer( 2 ) );
        {
            // the future's task references the pool as well, it has to be gone before we let go of the pool
            CORE::CFutureResult started = pool->StartTaskWithConsumer( consumer );
            ASSERT_FALSE( started.HasNoFuture() );
            ASSERT_TRUE( WaitUntil( consumer->ended, 5000 ) );
        }
        ASSERT_TRUE( pool->WaitForAllThreadsToFinish( 5000 ) );

        ASSERT_TRUE( taskManager.UnregisterThreadPool( pool ) );
        consumer.Unlink();                                          // consumers reference their pool too
        pool.Unlink();

        PumpFor( 200 );                                             // delivers ThreadFinishedEvent to the pool
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DedicatedTasks: A pool which is no longer registered is destroyed once it is let go of and its threads stopped" )
    try
    {
        // the threads of a pool, and the tasks it carried out, must not keep the pool alive after its use
        TQuickConsumerFactory quickFactory;
        taskManager.RegisterTaskConsumerFactory( CQuickConsumer::TaskType, &quickFactory );

        CDestructionObserver poolObserver;
        {
            CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( "DedicatedTasksLetGo" );
            poolObserver.SubscribeTo( pool.GetPointerAlways() );
            TCountingConsumerPtr consumer( GUCEF_NEW CCountingConsumer( 2 ) );
            {
                CORE::CFutureResult started = pool->StartTaskWithConsumer( consumer );
                ASSERT_FALSE( started.HasNoFuture() );
                ASSERT_TRUE( WaitUntil( consumer->ended, 5000 ) );
                CORE::CFutureResult queued = pool->QueueTask( CQuickConsumer::TaskType );
                ASSERT_TRUE( queued.HasAFuture() );
                ASSERT_FALSE( queued.GetResult( 5000 ).IsNULL() );
            }
            ASSERT_TRUE( pool->RequestAllThreadsToStop( true, false, 5000 ) );
            ASSERT_TRUE( taskManager.UnregisterThreadPool( pool ) );
            consumer.Unlink();
        }
        PumpFor( 200 );
        ASSERT_TRUE( poolObserver.destroyed );

        taskManager.UnregisterTaskConsumerFactory( CQuickConsumer::TaskType );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DedicatedTasks: The consumers a pool keeps for reuse are released when their factory is unregistered" )
    try
    {
        // a consumer is destroyed through the factory that made it, so it cannot outlive that factory
        CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( "DedicatedTasksFactoryGone" );
        {
            TQuickConsumerFactory quickFactory;
            taskManager.RegisterTaskConsumerFactory( CQuickConsumer::TaskType, &quickFactory );
            {
                CORE::CFutureResult queued = pool->QueueTask( CQuickConsumer::TaskType );
                ASSERT_TRUE( queued.HasAFuture() );
                ASSERT_FALSE( queued.GetResult( 5000 ).IsNULL() );
            }
            PumpFor( 50 );
            ASSERT_TRUE( 1 == CQuickConsumer::liveInstances.Load() );

            taskManager.UnregisterTaskConsumerFactory( CQuickConsumer::TaskType );
            ASSERT_TRUE( 0 == CQuickConsumer::liveInstances.Load() );
        }
        ASSERT_TRUE( pool->RequestAllThreadsToStop( true, false, 5000 ) );
        ASSERT_TRUE( taskManager.UnregisterThreadPool( pool ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
