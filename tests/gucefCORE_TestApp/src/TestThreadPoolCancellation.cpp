/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CASYNC_H
#include "gucefCORE_CASync.h"
#define GUCEF_CORE_CASYNC_H
#endif /* GUCEF_CORE_CASYNC_H ? */

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

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "TestThreadPoolCancellation.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static MT::CMutex g_cancelTestLock;
static bool g_releaseBlockers = false;
static UInt32 g_blockersStarted = 0;
static UInt32 g_blockersEnded = 0;
static UInt32 g_blockersEnding = 0;
static UInt32 g_quickTasksRun = 0;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** a task which keeps a thread busy until it is released or stopped */
class CCancelTestBlocker : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    CCancelTestBlocker( void ) : CORE::CTaskConsumer() {}
    virtual ~CCancelTestBlocker() GUCEF_VIRTUAL_OVERRIDE {}

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex lock( g_cancelTestLock );
        ++g_blockersStarted;
        return true;
    }

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        {
            MT::CScopeMutex lock( g_cancelTestLock );
            if ( g_releaseBlockers )
                return true;
        }
        RequestTaskCycleDelayInMs( 2 );
        return false;
    }

    virtual void OnTaskEnding( CORE::CTaskPtr task, bool willBeForced ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex lock( g_cancelTestLock );
        ++g_blockersEnding;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE
    {
        {
            MT::CScopeMutex lock( g_cancelTestLock );
            ++g_blockersEnded;
        }
        CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
    }
};

const CORE::CString CCancelTestBlocker::TaskType = "CancelTestBlocker";

/*-------------------------------------------------------------------------*/

/** a task which is done right away */
class CCancelTestQuick : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    CCancelTestQuick( void ) : CORE::CTaskConsumer() {}
    virtual ~CCancelTestQuick() GUCEF_VIRTUAL_OVERRIDE {}

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex lock( g_cancelTestLock );
        ++g_quickTasksRun;
        return true;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
    }
};

const CORE::CString CCancelTestQuick::TaskType = "CancelTestQuick";

/*-------------------------------------------------------------------------*/

/** gives access to the first task of a chain, which is protected in CASync */
class CChainAccess : public CORE::CASync
{
    public:

    CChainAccess( CORE::ThreadPoolPtr threadPool ) : CORE::CASync( threadPool ) {}
    ~CChainAccess() {}

    CORE::CTaskPtr GetFirstTaskPublic( void ) const { return GetFirstTask(); }
    CORE::CTaskPtr GetLastTaskPublic( void ) const { return GetLastTask(); }
};

static int
QuickChainCallback( void )
{
    MT::CScopeMutex lock( g_cancelTestLock );
    ++g_quickTasksRun;
    return 1;
}

/** the steps after the first one of a chain are given the task they are carried out as */
static int
QuickChainStepCallback( CORE::CTaskPtr task )
{
    MT::CScopeMutex lock( g_cancelTestLock );
    ++g_quickTasksRun;
    return 2;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
ResetCounters( void )
{
    MT::CScopeMutex lock( g_cancelTestLock );
    g_releaseBlockers = false;
    g_blockersStarted = 0;
    g_blockersEnded = 0;
    g_blockersEnding = 0;
    g_quickTasksRun = 0;
}

/*-------------------------------------------------------------------------*/

static void
ReleaseBlockers( void )
{
    MT::CScopeMutex lock( g_cancelTestLock );
    g_releaseBlockers = true;
}

/*-------------------------------------------------------------------------*/

static UInt32
GetCounter( const UInt32& counter )
{
    MT::CScopeMutex lock( g_cancelTestLock );
    return counter;
}

/*-------------------------------------------------------------------------*/

static bool
WaitForCounter( const UInt32& counter, UInt32 atLeast, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs; waited += 2 )
    {
        if ( GetCounter( counter ) >= atLeast )
            return true;
        MT::ThreadDelay( 2 );
    }
    return GetCounter( counter ) >= atLeast;
}

/*-------------------------------------------------------------------------*/

/** a pool with a single worker thread, so that a task which keeps it busy makes the next ones wait in the queue */
static CORE::ThreadPoolPtr
CreateSingleThreadPool( const CORE::CString& name )
{
    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( name );
    pool->SetDesiredMaxTotalNrOfThreads( 1 );
    return pool;
}

/*-------------------------------------------------------------------------*/

/** Gets (or makes) a thread pool on a thread of its own and notes when that returned */
class CPoolCreatorThread : public MT::CActiveObject
{
    public:

    explicit CPoolCreatorThread( const CORE::CString& poolName )
        : MT::CActiveObject()
        , m_poolName( poolName )
        , m_doneLock()
        , m_done( false )
    {GUCEF_TRACE;
    }

    // the thread may still be ending (OnThreadEnding(), OnThreadEnded()) when the test sees it done: it is waited for
    // before this part of the object goes
    virtual ~CPoolCreatorThread() { Deactivate( false, true ); }

    bool IsDone( void ) const
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_doneLock );
        return m_done;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( m_poolName );
        MT::CScopeMutex lock( m_doneLock );
        m_done = true;
        return true;
    }

    private:

    CORE::CString m_poolName;
    MT::CMutex m_doneLock;
    bool m_done;
};

/*-------------------------------------------------------------------------*/

/** Waits until the thread is done, up to the time; whether it was */
static bool
WaitUntilDone( const CPoolCreatorThread& thread, UInt32 timeoutInMs )
{GUCEF_TRACE;

    for ( UInt32 waited=0; !thread.IsDone() && waited<timeoutInMs; waited+=10 )
        MT::PrecisionDelay( 10 );
    return thread.IsDone();
}

/*-------------------------------------------------------------------------*/

void
PerformThreadPoolCancellationTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING thread pool cancellation TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "ThreadPoolCancellation" );

    typedef CORE::CTFactory< CORE::CTaskConsumer, CCancelTestBlocker, MT::CMutex > TBlockerFactory;
    typedef CORE::CTFactory< CORE::CTaskConsumer, CCancelTestQuick, MT::CMutex > TQuickFactory;
    TBlockerFactory blockerFactory;
    TQuickFactory quickFactory;
    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    taskManager.RegisterTaskConsumerFactory( CCancelTestBlocker::TaskType, &blockerFactory );
    taskManager.RegisterTaskConsumerFactory( CCancelTestQuick::TaskType, &quickFactory );

    GUCEF_TESTFW_TESTCASE( "Test: A task waiting in the queue is cancelled and never starts" )
        try
        {
            ResetCounters();
            CORE::ThreadPoolPtr pool = CreateSingleThreadPool( "CancelTestPoolQueued" );

            // the first task occupies the only thread, the second has to wait
            CORE::CFutureResult running = pool->QueueTask( CCancelTestBlocker::TaskType );
            ASSERT_TRUE( running.HasAFuture() );
            ASSERT_TRUE( WaitForCounter( g_blockersStarted, 1, 5000 ) );
            CORE::CFutureResult waiting = pool->QueueTask( CCancelTestQuick::TaskType );
            ASSERT_TRUE( waiting.HasAFuture() );
            ASSERT_FALSE( waiting.IsReady() );

            ASSERT_TRUE( waiting.RequestCancellation() );

            // the future is ready at once and reports the cancellation
            ASSERT_TRUE( waiting.IsReady() );
            CORE::CTaskPtr waitingTask = waiting.GetResult( 2000 );
            ASSERT_FALSE( waitingTask.IsNULL() );
            if ( !waitingTask.IsNULL() )
                ASSERT_TRUE( CORE::TASKSTATUS_CANCELLED == waitingTask->GetTaskStatus() );

            // the thread is free again and the cancelled task did not run
            ReleaseBlockers();
            CORE::CTaskPtr runningTask = running.GetResult( 5000 );
            ASSERT_FALSE( runningTask.IsNULL() );
            MT::ThreadDelay( 100 );
            ASSERT_TRUE( 0 == GetCounter( g_quickTasksRun ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        ReleaseBlockers();
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A running task is asked to end and ends as cancelled" )
        try
        {
            ResetCounters();
            CORE::ThreadPoolPtr pool = CreateSingleThreadPool( "CancelTestPoolRunning" );

            CORE::CFutureResult running = pool->QueueTask( CCancelTestBlocker::TaskType );
            ASSERT_TRUE( running.HasAFuture() );
            ASSERT_TRUE( WaitForCounter( g_blockersStarted, 1, 5000 ) );
            ASSERT_FALSE( running.IsReady() );

            ASSERT_TRUE( running.RequestCancellation() );

            CORE::CTaskPtr task = running.GetResult( 5000 );
            ASSERT_FALSE( task.IsNULL() );
            if ( !task.IsNULL() )
                ASSERT_TRUE( CORE::TASKSTATUS_CANCELLED == task->GetTaskStatus() );

            // the consumer got the chance to wrap up
            ASSERT_TRUE( WaitForCounter( g_blockersEnded, 1, 5000 ) );
            ASSERT_TRUE( GetCounter( g_blockersEnding ) >= 1 );

            // only the task was cancelled: the thread which carried it out is free to pick up the next one
            CORE::CFutureResult next = pool->QueueTask( CCancelTestQuick::TaskType );
            ASSERT_TRUE( next.HasAFuture() );
            CORE::CTaskPtr nextTask = next.GetResult( 5000 );
            ASSERT_FALSE( nextTask.IsNULL() );
            if ( !nextTask.IsNULL() )
                ASSERT_TRUE( CORE::TASKSTATUS_FINISHED == nextTask->GetTaskStatus() );
            ASSERT_TRUE( 1 == GetCounter( g_quickTasksRun ) );
            ASSERT_TRUE( 1 == pool->GetActiveNrOfThreads() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        ReleaseBlockers();
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Cancelling the first task of a chain cancels the whole chain" )
        try
        {
            ResetCounters();
            CORE::ThreadPoolPtr pool = CreateSingleThreadPool( "CancelTestPoolChain" );

            CORE::CFutureResult running = pool->QueueTask( CCancelTestBlocker::TaskType );
            ASSERT_TRUE( running.HasAFuture() );
            ASSERT_TRUE( WaitForCounter( g_blockersStarted, 1, 5000 ) );

            // a chain of two behind it, neither of which can start
            CChainAccess chain( pool );
            CORE::CFutureResult chainFuture = chain.QueueCallback( QuickChainCallback ).ThenCallback( QuickChainStepCallback );
            ASSERT_TRUE( chainFuture.HasAFuture() );
            CORE::CTaskPtr first = chain.GetFirstTaskPublic();
            CORE::CTaskPtr last = chain.GetLastTaskPublic();
            ASSERT_FALSE( first.IsNULL() );
            ASSERT_FALSE( last.IsNULL() );
            ASSERT_FALSE( first->GetTaskId() == last->GetTaskId() );

            if ( !first.IsNULL() && !last.IsNULL() )
            {
                ASSERT_TRUE( pool->RequestTaskCancellation( first->GetTaskId(), false, false ) );
                ASSERT_TRUE( CORE::TASKSTATUS_CANCELLED == first->GetTaskStatus() );
                ASSERT_TRUE( CORE::TASKSTATUS_CANCELLED == last->GetTaskStatus() );
            }

            ReleaseBlockers();
            running.GetResult( 5000 );
            MT::ThreadDelay( 100 );
            ASSERT_TRUE( 0 == GetCounter( g_quickTasksRun ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        ReleaseBlockers();
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Cancelling an unknown task or a task which has finished is handled" )
        try
        {
            ResetCounters();
            CORE::ThreadPoolPtr pool = CreateSingleThreadPool( "CancelTestPoolOther" );

            // a task id which does not exist: only a problem if the caller says so
            ASSERT_TRUE( pool->RequestTaskCancellation( 0x7FFFFFF0, false, true ) );
            ASSERT_FALSE( pool->RequestTaskCancellation( 0x7FFFFFF0, false, false ) );

            // a task which is done stays what it is
            CORE::CFutureResult quick = pool->QueueTask( CCancelTestQuick::TaskType );
            ASSERT_TRUE( quick.HasAFuture() );
            CORE::CTaskPtr task = quick.GetResult( 5000 );
            ASSERT_FALSE( task.IsNULL() );
            if ( !task.IsNULL() )
            {
                ASSERT_TRUE( CORE::TASKSTATUS_FINISHED == task->GetTaskStatus() );
                ASSERT_TRUE( quick.RequestCancellation() );
                ASSERT_TRUE( CORE::TASKSTATUS_FINISHED == task->GetTaskStatus() );
            }
            ASSERT_TRUE( 1 == GetCounter( g_quickTasksRun ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The cancelled status can be converted to and from text" )
        try
        {
            ASSERT_TRUE( CORE::CString( "CANCELLED" ) == CORE::TaskStatusToTaskStatusString( CORE::TASKSTATUS_CANCELLED ) );
            ASSERT_TRUE( CORE::TASKSTATUS_CANCELLED == CORE::TaskStatusStringToTaskStatus( "CANCELLED" ) );
            ASSERT_TRUE( CORE::TaskStatusIsAnEndState( CORE::TASKSTATUS_CANCELLED ) );
            ASSERT_FALSE( CORE::TaskStatusIsAnError( CORE::TASKSTATUS_CANCELLED ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A thread pool made on a worker thread while the main thread pulses does not block the other users of the task manager" )
        try
        {
            // the main thread notifies the observers of the pulse generator (it holds its notification lock) when a task makes a
            // new pool, which subscribes to that pulse generator: the task manager must not be locked while the pool is made,
            // or the main thread blocks on it as soon as it asks the task manager for a pool
            taskManager.GetOrCreateThreadPool( "PoolCreationTestExisting" );
            CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
            CORE::CNotifierScopeLock* pulseLock = GUCEF_NEW CORE::CNotifierScopeLock( pulseGenerator.GetPointerAlways() );
            CPoolCreatorThread creator( "PoolCreationTestNew" );
            ASSERT_TRUE( creator.Activate( GUCEF_NULL, 0, 0 ) );
            MT::PrecisionDelay( 300 );
            CPoolCreatorThread finder( "PoolCreationTestExisting" );
            ASSERT_TRUE( finder.Activate( GUCEF_NULL, 0, 0 ) );
            const bool notBlocked = WaitUntilDone( finder, 3000 );
            GUCEF_DELETE pulseLock;

            ASSERT_TRUE( notBlocked );
            ASSERT_TRUE( WaitUntilDone( creator, 70000 ) && WaitUntilDone( finder, 70000 ) );
            ASSERT_FALSE( taskManager.GetThreadPool( "PoolCreationTestNew" ).IsNULL() );
            taskManager.UnregisterThreadPool( "PoolCreationTestNew" );
            taskManager.UnregisterThreadPool( "PoolCreationTestExisting" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    ReleaseBlockers();
    taskManager.UnregisterTaskConsumerFactory( CCancelTestBlocker::TaskType );
    taskManager.UnregisterTaskConsumerFactory( CCancelTestQuick::TaskType );

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL thread pool cancellation TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
