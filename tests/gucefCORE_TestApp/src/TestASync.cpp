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

#include <iostream>
#include <vector>
#include <memory>

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

#ifndef GUCEF_CORE_CGENERICCALLBACKTASKCONSUMER_H
#include "gucefCORE_CGenericCallbackTaskConsumer.h"
#define GUCEF_CORE_CGENERICCALLBACKTASKCONSUMER_H
#endif /* GUCEF_CORE_CGENERICCALLBACKTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "TestASync.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#if GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX || GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID
  #define DEBUGBREAK __builtin_trap()
#elif GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN
  #define DEBUGBREAK DebugBreak()
#else
  #define DEBUGBREAK
#endif

#define ERRORHERE { std::cout << "Test failed @ " << __FILE__ << "(" << __LINE__ << ")\n"; DEBUGBREAK; }
#define ASSERT_TRUE( test ) if ( !(test) ) { ERRORHERE; } 
#define ASSERT_FALSE( test ) if ( (test) ) { ERRORHERE; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static MT::CMutex g_testMutex;
static std::vector<int> g_testResults;
static int g_testCounter = 0;
static CORE::CString g_threadPoolName = "TestASyncThreadPool";

/*-------------------------------------------------------------------------//
//                                                                         //
//   UTILITIES                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

// Simple test callback functions with various signatures
int SimpleCallback0()
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SimpleCallback0 called" );
    return 42;
}

int SimpleCallback1(int value)
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SimpleCallback1 called with value: " + CORE::ToString( value ) );
    return value * 2;
}

int SimpleCallback2(int a, int b)
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SimpleCallback2 called with values: " + CORE::ToString( a ) + ", " + CORE::ToString( b ) );
    return a + b;
}

int SimpleCallback3(int a, int b, int c)
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SimpleCallback3 called with values: " + CORE::ToString( a ) + ", " + CORE::ToString( b ) + ", " + CORE::ToString( c ) );
    return a + b + c;
}

int SimpleCallback4(int a, int b, int c, int d)
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SimpleCallback4 called with values: " + CORE::ToString( a ) + ", " + CORE::ToString( b ) + ", " + CORE::ToString( c ) + ", " + CORE::ToString( d ) );
    return a + b + c + d;
}

// Chain callback functions
int ChainCallback1(CORE::CTaskPtr taskPtr)
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ChainCallback1 called with task ID: " + CORE::ToString( taskPtr->GetTaskId() ) );
    
    MT::CScopeMutex lock(g_testMutex);
    g_testResults.push_back(1);
    return 100;
}

int ChainCallback2(CORE::CTaskPtr taskPtr, int multiplier)
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ChainCallback2 called with task ID: " + CORE::ToString( taskPtr->GetTaskId() ) + ", multiplier: " + CORE::ToString( multiplier ) );
    
    MT::CScopeMutex lock(g_testMutex);
    g_testResults.push_back(2);
    return 200 * multiplier;
}

int ChainCallback3(CORE::CTaskPtr taskPtr, int a, int b)
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ChainCallback3 called with task ID: " + CORE::ToString( taskPtr->GetTaskId() ) + ", a: " + CORE::ToString( a ) + ", b: " + CORE::ToString( b ) );
    
    MT::CScopeMutex lock(g_testMutex);
    g_testResults.push_back(3);
    return 300 + a + b;
}

// Memory leak detection helpers
struct TaskMemoryTracker
{
    static int s_taskCount;
    static MT::CMutex s_mutex;
    
    TaskMemoryTracker() 
    {
        MT::CScopeMutex lock(s_mutex);
        ++s_taskCount;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "TaskMemoryTracker created, count: " + CORE::ToString( s_taskCount ) );
    }
    
    ~TaskMemoryTracker() 
    {
        MT::CScopeMutex lock(s_mutex);
        --s_taskCount;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "TaskMemoryTracker destroyed, count: " + CORE::ToString( s_taskCount ) );
    }
    
    static int GetTaskCount() 
    {
        MT::CScopeMutex lock(s_mutex);
        return s_taskCount;
    }
};

int TaskMemoryTracker::s_taskCount = 0;
MT::CMutex TaskMemoryTracker::s_mutex;

int MemoryTrackingCallback()
{
    TaskMemoryTracker tracker;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MemoryTrackingCallback executed" );
    return 123;
}

int MemoryTrackingChainCallback(CORE::CTaskPtr taskPtr)
{
    TaskMemoryTracker tracker;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MemoryTrackingChainCallback executed with task ID: " + CORE::ToString( taskPtr->GetTaskId() ) );
    return 456;
}

// Error simulation callback
int ErrorCallback()
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ErrorCallback called - simulating error" );
    throw std::runtime_error("Simulated error in callback");
}

// Long running callback for timeout tests
int LongRunningCallback()
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LongRunningCallback started" );
    MT::PrecisionDelay(2000); // 2 second delay
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LongRunningCallback finished" );
    return 999;
}


class CASyncTestAccess : public CORE::CASync
{
    public:

    typedef CORE::CASync::TASyncChainStatePtr TASyncChainStatePtr;

    CASyncTestAccess( const CORE::CString& threadPoolName = g_threadPoolName )
        : CORE::CASync(threadPoolName)
    {
    }

    CASyncTestAccess( CORE::ThreadPoolPtr threadPool )
        : CORE::CASync( threadPool )
    {
    }

    virtual ~CASyncTestAccess()
    {
    }

    CORE::CTaskPtr GetLastTaskPublic() const
    {
        return GetLastTask();
    }

    TASyncChainStatePtr GetChainStatePublic() const
    {
        return GetChainState();
    }

    CORE::ThreadPoolPtr GetThreadPoolPublic() const
    {
        TASyncChainStatePtr state = GetChainState();
        return state ? state->m_threadPool : CORE::ThreadPoolPtr();
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//   TEST FUNCTIONS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

void
TestFinalTaskCountsAfterTests()
{
    // check the state of the thread pool wrt task counts
    // there could be residual tasks from previous tests which is also a leak we want to catch those as well
    CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( g_threadPoolName );
    ASSERT_TRUE( !threadPool.IsNULL() );

    UInt32 nrOfInUseTasks = 0;
    UInt32 nrOfActiveTasks = 0;
    UInt32 nrOfDormantTasks = 0;
    UInt32 nrOfFreeTaskObjs = 0;

    UInt32 poolNrOfInUseTasks = 0;
    UInt32 poolNrOfActiveTasks = 0;
    UInt32 poolNrOfDormantTasks = 0;
    UInt32 poolNrOfFreeTaskObjs = 0;

    ASSERT_TRUE( threadPool->GetTaskTotals( poolNrOfInUseTasks, poolNrOfActiveTasks, poolNrOfDormantTasks, poolNrOfFreeTaskObjs ) );
    ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetTaskManager().GetTaskTotals( nrOfInUseTasks, nrOfActiveTasks, nrOfDormantTasks, nrOfFreeTaskObjs ) );

    // give some time for any administrative upkeep to complete, we try a few times before giving up
    bool allTasksFinished = false;
    for ( UInt32 i=0; i<10; ++i )
    {
        ASSERT_TRUE( threadPool->GetTaskTotals( poolNrOfInUseTasks, poolNrOfActiveTasks, poolNrOfDormantTasks, poolNrOfFreeTaskObjs ) );
        ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetTaskManager().GetTaskTotals( nrOfInUseTasks, nrOfActiveTasks, nrOfDormantTasks, nrOfFreeTaskObjs ) );

        allTasksFinished = ( nrOfInUseTasks == 0 &&         // we completed all the work hence no active or otherwise tracked tasks (due to chains) should remain
                             nrOfActiveTasks == 0 &&        // we completed all the work hence no active tasks should remain
                             nrOfDormantTasks == 0 &&       // we created all objects in local scopes so no dormant tasks should remain
                             nrOfFreeTaskObjs >= 0 &&       // we should have a bunch of free task objects available for reuse if other tests ran and initialized the free list

                             poolNrOfInUseTasks == 0 &&     // we completed all the work hence no active or otherwise tracked tasks (due to chains) should remain
                             poolNrOfActiveTasks == 0 &&    // we completed all the work hence no active tasks should remain
                             poolNrOfDormantTasks == 0 &&   // we created all objects in local scopes so no dormant tasks should remain
                             poolNrOfFreeTaskObjs >= 0 );   // we should have a bunch of free task objects available for reuse if other tests ran and initialized the free list

        if ( allTasksFinished )
        {
            break;
        }
        else
        {    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Waiting for all tasks to complete. Current counts - Global: InUse=" + CORE::ToString( nrOfInUseTasks ) +
                ", Active=" + CORE::ToString( nrOfActiveTasks ) +
                ", Dormant=" + CORE::ToString( nrOfDormantTasks ) +
                ", FreeObjs=" + CORE::ToString( nrOfFreeTaskObjs ) +
                " | Pool: InUse=" + CORE::ToString( poolNrOfInUseTasks ) +
                ", Active=" + CORE::ToString( poolNrOfActiveTasks ) +
                ", Dormant=" + CORE::ToString( poolNrOfDormantTasks ) +
                ", FreeObjs=" + CORE::ToString( poolNrOfFreeTaskObjs ) );

            // give some time for the administrative upkeep to complete
            MT::PrecisionDelay( 1000 );
        }
    }
    if ( !allTasksFinished )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Final task counts did not reach zero after waiting. Global: InUse=" + CORE::ToString( nrOfInUseTasks ) +
                ", Active=" + CORE::ToString( nrOfActiveTasks ) +
                ", Dormant=" + CORE::ToString( nrOfDormantTasks ) +
                ", FreeObjs=" + CORE::ToString( nrOfFreeTaskObjs ) +
                " | Pool: InUse=" + CORE::ToString( poolNrOfInUseTasks ) +
                ", Active=" + CORE::ToString( poolNrOfActiveTasks ) +
                ", Dormant=" + CORE::ToString( poolNrOfDormantTasks ) +
                ", FreeObjs=" + CORE::ToString( poolNrOfFreeTaskObjs ) );
        ERRORHERE;
    }
}

void TestBasicASyncConstruction()
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "\n=== Testing Basic CASync Construction ===\n" );
    
    try
    {
        // Test default construction
        CASyncTestAccess async1;
        CORE::CTaskPtr lastTask = async1.GetLastTaskPublic();
        ASSERT_TRUE( lastTask.IsNULL() );
        CASyncTestAccess::TASyncChainStatePtr state = async1.GetChainStatePublic();
        ASSERT_TRUE( !state.IsNULL() );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Default CASync construction successful" );
        
        // Test construction with different thread pool name
        CASyncTestAccess async2( "TestPoolWithDifferentName" );
        lastTask = async2.GetLastTaskPublic();
        CORE::ThreadPoolPtr threadPool1 = async2.GetThreadPoolPublic();
        ASSERT_TRUE( lastTask.IsNULL() );
        ASSERT_TRUE( !threadPool1.IsNULL() );
        ASSERT_TRUE( threadPool1->GetThreadPoolName() == "TestPoolWithDifferentName" );
        state = async2.GetChainStatePublic();
        ASSERT_TRUE( !state.IsNULL() );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Named thread pool CASync construction successful" );

        // Test construction with thread pool pointer
        CORE::ThreadPoolPtr threadPool2 = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "TestPoolWithDifferentName2" );
        ASSERT_TRUE( !threadPool2.IsNULL() );
        ASSERT_TRUE( threadPool2->GetThreadPoolName() == "TestPoolWithDifferentName2" );
        CASyncTestAccess async3( threadPool2 );
        lastTask = async3.GetLastTaskPublic();
        ASSERT_TRUE( lastTask.IsNULL() );
        state = async3.GetChainStatePublic();
        ASSERT_TRUE( !state.IsNULL() );

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Thread pool pointer CASync construction successful" );
        
    }
    catch(const std::exception& e)
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Exception in TestBasicASyncConstruction: " + CORE::ToString( e.what() ) );
        ERRORHERE;
    }
}

void TestSimpleCallbacks()
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, " === Testing Simple Callbacks === " );

    try
    {
        // Test callback with no parameters
        {
            CASyncTestAccess async;
            CORE::CFutureResult result = async.QueueCallback( SimpleCallback0 );
     
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(50000);
            ASSERT_TRUE(!task.IsNULL());
            ASSERT_TRUE(task->IsTaskInEndState());
            std::cout << "Simple callback 0 test passed\n";
        }
        TestFinalTaskCountsAfterTests();
      
        // Test callback with 1 parameter
        {
            CASyncTestAccess async;
            CORE::CFutureResult result = async.QueueCallback( SimpleCallback1, 10);
    
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(50000);
            ASSERT_TRUE(!task.IsNULL());
            ASSERT_TRUE(task->IsTaskInEndState());
            std::cout << "Simple callback 1 test passed\n";
        }
        TestFinalTaskCountsAfterTests();
        
        // Test callback with 2 parameters
        {
            CASyncTestAccess async;
            CORE::CFutureResult result = async.QueueCallback( SimpleCallback2, 5, 7);
            
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(50000);
            ASSERT_TRUE(!task.IsNULL());
            ASSERT_TRUE(task->IsTaskInEndState());
            std::cout << "Simple callback 2 test passed\n";
        }
        TestFinalTaskCountsAfterTests();
        
        // Test callback with 3 parameters
        {
            CASyncTestAccess async;
            CORE::CFutureResult result = async.QueueCallback(SimpleCallback3, 1, 2, 3);
    
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(50000);
            ASSERT_TRUE(!task.IsNULL());
            ASSERT_TRUE(task->IsTaskInEndState());
            std::cout << "Simple callback 3 test passed\n";
        }
        TestFinalTaskCountsAfterTests();
 
        // Test callback with 4 parameters
        {
            CASyncTestAccess async;
            CORE::CFutureResult result = async.QueueCallback(SimpleCallback4, 1, 2, 3, 4);
   
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(50000);
            ASSERT_TRUE(!task.IsNULL());
            ASSERT_TRUE(task->IsTaskInEndState());
            std::cout << "Simple callback 4 test passed\n";
        }
        TestFinalTaskCountsAfterTests();
    }
    catch( const timeout_exception& e )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Timeout Exception in TestSimpleCallbacks: " + CORE::ToString( e.what() ) );
        ERRORHERE;
    }
    catch( const std::exception& e )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Exception in TestSimpleCallbacks: " + CORE::ToString( e.what() ) );
        ERRORHERE;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, " === Finished Testing Simple Callbacks === " );
}

void TestTaskChaining1Deep()
{
    GUCEF_LOG(CORE::LOGLEVEL_NORMAL, " === Testing Task Chaining 1 Deep === ");

    try
    {
        g_testResults.clear();
        
        // Test simple chain
        {
            CASyncTestAccess async;
            CORE::CFutureResult result = async.QueueCallback(SimpleCallback0)
                                              .ThenCallback(ChainCallback1);
     
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(10000000);
            ASSERT_TRUE(!task.IsNULL());
            ASSERT_TRUE(task->IsTaskInEndState());

            // Verify the chain executed in order
            MT::CScopeMutex lock( g_testMutex );
            ASSERT_TRUE( g_testResults.size() == 1 );
            ASSERT_TRUE( g_testResults[ 0 ] == 1 );
            GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Task chaining 1 Deep test passed");
        }
      
        g_testResults.clear();

        TestFinalTaskCountsAfterTests();
    }
    catch(const timeout_exception& e)
    {
        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Timeout Exception in TestTaskChaining 1 Deep: " + CORE::ToString( e.what() ));
        ERRORHERE;
    }
    catch(const std::exception& e)
    {
        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Exception in TestTaskChaining 1 Deep: " + CORE::ToString( e.what() ));
        ERRORHERE;
    }


}

void TestTaskChaining3Deep()
{
    GUCEF_LOG(CORE::LOGLEVEL_NORMAL, " === Testing Task Chaining 3 Deep === ");

    try
    {
        g_testResults.clear();
        
        // Test simple chain
        {
            CASyncTestAccess async;
            CORE::CFutureResult result = async.QueueCallback(SimpleCallback0)
                                              .ThenCallback(ChainCallback1)
                                              .ThenCallback(ChainCallback2, 3)
                                              .ThenCallback(ChainCallback3, 10, 20);
     
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(1000800000);
            ASSERT_TRUE(!task.IsNULL());
            ASSERT_TRUE(task->IsTaskInEndState());
            
            // Wait a bit for all chained tasks to complete
            //MT::PrecisionDelay(1000);

            // Verify the chain executed in order
            MT::CScopeMutex lock(g_testMutex);
            ASSERT_TRUE(g_testResults.size() == 3);
            ASSERT_TRUE(g_testResults[0] == 1);
            ASSERT_TRUE(g_testResults[1] == 2);
            ASSERT_TRUE(g_testResults[2] == 3);
            std::cout << "Task chaining test passed\n";
        }
      
        g_testResults.clear();

        TestFinalTaskCountsAfterTests();
    }
    catch(const timeout_exception& e)
    {
        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Timeout Exception in TestTaskChaining 3Deep: " + CORE::ToString( e.what() ));
        ERRORHERE;
    }
    catch(const std::exception& e)
    {
        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Exception in TestTaskChaining 3Deep: " + CORE::ToString( e.what() ));
        ERRORHERE;
    }
}

void TestMemoryManagement()
{
    GUCEF_LOG(CORE::LOGLEVEL_NORMAL, " === Testing Memory Management === ");
    
    try
    {
        // check the initial state of the thread pool wrt task counts
        // there could be residual tasks from previous tests which is also a leak we want to catch those as well
        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( g_threadPoolName );
        ASSERT_TRUE( !threadPool.IsNULL() );

        UInt32 nrOfInUseTasks = 0;
        UInt32 nrOfActiveTasks = 0;
        UInt32 nrOfDormantTasks = 0;
        UInt32 nrOfFreeTaskObjs = 0;

        ASSERT_TRUE( threadPool->GetTaskTotals( nrOfInUseTasks, nrOfActiveTasks, nrOfDormantTasks, nrOfFreeTaskObjs ) );
        ASSERT_TRUE( nrOfInUseTasks == 0 );   // we completed all the work hence no active or otherwise tracked tasks (due to chains) should remain
        ASSERT_TRUE( nrOfActiveTasks == 0 );  // we completed all the work hence no active tasks should remain
        ASSERT_TRUE( nrOfDormantTasks == 0 ); // we created all objects in local scopes so no dormant tasks should remain
        ASSERT_TRUE( nrOfFreeTaskObjs >= 0 );  // we should have a bunch of free task objects available for reuse if other tests ran and initialized the free list

        int initialCount = TaskMemoryTracker::GetTaskCount();
        std::cout << "Initial tracker count: " << initialCount << "\n";
   
        // Test simple callback memory management
        {
            CASyncTestAccess async( threadPool );
            CORE::CFutureResult result = async.QueueCallback(MemoryTrackingCallback);
        
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(5000);
            ASSERT_TRUE(!task.IsNULL());
            ASSERT_TRUE(task->IsTaskInEndState());
        }
        
        // Give time for cleanup
        MT::PrecisionDelay(500);
        
        // Test chained callback memory management
        {
            CASyncTestAccess async( threadPool );
            CORE::CFutureResult result = async.QueueCallback(MemoryTrackingCallback)
                                              .ThenCallback(MemoryTrackingChainCallback)
                                              .ThenCallback(MemoryTrackingChainCallback)
                                              .ThenCallback(MemoryTrackingChainCallback);
            
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(10000);
            ASSERT_TRUE(!task.IsNULL());
            ASSERT_TRUE(task->IsTaskInEndState());
        }
    
        // Give time for all cleanup to complete
        MT::PrecisionDelay(2000);
        
        int finalCount = TaskMemoryTracker::GetTaskCount();
        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Final tracker count: " + CORE::ToString(finalCount));

        // Check for memory leaks - this is the critical test for the known issue
        ASSERT_TRUE(finalCount == initialCount);

        // Now check and see if we did not leak any task objects in the thread pool
        // this test does assume that this test is the only thing actually using the thread pool right now in the process

        UInt32 nrOfInUseTasks2 = 0;
        UInt32 nrOfActiveTasks2 = 0;
        UInt32 nrOfDormantTasks2 = 0;
        UInt32 nrOfFreeTaskObjs2 = 0;

        ASSERT_TRUE( threadPool->GetTaskTotals( nrOfInUseTasks2, nrOfActiveTasks2, nrOfDormantTasks2, nrOfFreeTaskObjs2 ) );
        //ASSERT_TRUE( nrOfInUseTasks2 == 0 );   // we completed all the work hence no active or otherwise tracked tasks (due to chains) should remain
        //ASSERT_TRUE( nrOfActiveTasks2 == 0 );  // we completed all the work hence no active tasks should remain
        //ASSERT_TRUE( nrOfDormantTasks2 == 0 ); // we created all objects in local scopes so no dormant tasks should remain
        //ASSERT_TRUE( nrOfFreeTaskObjs2 > 0 );  // we should have a bunch of free task objects available for reuse

        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Memory management test passed - no leaks detected");
    }
    catch(const timeout_exception& e)
    {
        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Timeout Exception in TestTaskChaining: " + CORE::ToString( e.what() ));
        ERRORHERE;
    }
    catch(const std::exception& e)
    {
        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Exception in TestMemoryManagement: " + CORE::ToString( e.what() ));
        ERRORHERE;
    }
}

void TestTaskChainCleanup()
{
    GUCEF_LOG(CORE::LOGLEVEL_NORMAL, " === Testing Task Chain Cleanup === ");
    
    try
    {
        // This test specifically targets the chain reference cleanup issue mentioned
        GUCEF::vector< CORE::CFutureResult > futures;
        
        // Create multiple chains that should be cleaned up properly
        for (int i = 0; i < 10; ++i)
        {
            CASyncTestAccess async;
            CORE::CFutureResult result = async.QueueCallback(SimpleCallback0)
                                                .ThenCallback(ChainCallback1)
                                                .ThenCallback(ChainCallback2, i)
                                                .ThenCallback(ChainCallback3, i, i+1);
 
            futures.push_back(result);
        }
        
        // Wait for all chains to complete
        GUCEF::vector< CORE::CFutureResult >::iterator it = futures.begin();
        for ( ; it != futures.end(); ++it )
        {
            CORE::CFutureResult& future = *it;

            ASSERT_TRUE(future.HasAFuture());
            CORE::CTaskPtr task = future.GetResult(15000);
            ASSERT_TRUE(!task.IsNULL());
            ASSERT_TRUE(task->IsTaskInEndState());
        }
        
        // Clear futures to release references
        futures.clear();
      
        // Give time for cleanup
        //MT::PrecisionDelay(2000);

        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Task chain cleanup test completed");
    }
    catch(const std::exception& e)
    {
        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Exception in TestTaskChainCleanup: " + CORE::ToString( e.what() ));
        ERRORHERE;
    }
}

void TestErrorHandling()
{
    GUCEF_LOG(CORE::LOGLEVEL_NORMAL, " === Testing Error Handling === ");
    
    try
    {
        // Test error in callback - this should not crash the system
        {
            CASyncTestAccess async;
            CORE::CFutureResult result = async.QueueCallback(ErrorCallback);
       
            ASSERT_TRUE(result.HasAFuture());
            try
            {
                CORE::CTaskPtr task = result.GetResult(5000);

                // Task should exist but may be in an error state
                ASSERT_TRUE(!task.IsNULL());
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Error callback test completed - task state: " + task->GetTaskStatusString() );
            }
            catch(const std::exception& e)
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Expected exception caught in error callback test: " + CORE::ToString( e.what() ));
            }
        }
        
        // Test timeout handling
        {
            CORE::CASync async;
            CORE::CFutureResult result = async.QueueCallback(LongRunningCallback);
        
            ASSERT_TRUE(result.HasAFuture());
            try
            {
                // Use short timeout to test timeout behavior
                CORE::CTaskPtr task = result.GetResult(1000); // 1 second timeout for 2 second task
                GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Unexpected: Long running task completed within timeout");
            }
            catch(const std::exception& e)
            {
                GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Expected timeout exception caught: " + CORE::ToString( e.what() ));
            }
        }

        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Error handling tests completed");
    }
    catch(const std::exception& e)
    {
        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Exception in TestErrorHandling: " + CORE::ToString( e.what() ));
        ERRORHERE;
    }
}

void TestConcurrentOperations()
{
    GUCEF_LOG(CORE::LOGLEVEL_NORMAL, " === Testing Concurrent Operations === ");
    
    try
    {
        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( CORE::CTaskManager::DefaultThreadPoolName );
        threadPool->SetDesiredMinNrOfWorkerThreads( 20 );

        const int numConcurrentTasks = 20;
        std::vector<CORE::CFutureResult> futures;
    
        // Launch multiple concurrent async operations
        for (int i = 0; i < numConcurrentTasks; ++i)
        {
            CORE::CASync async( threadPool );
            CORE::CFutureResult result = async.QueueCallback( SimpleCallback1, i );
            futures.push_back(result);
        }
        
        // Wait for all to complete
        int completedCount = 0;
        for (auto& future : futures)
        {
            ASSERT_TRUE(future.HasAFuture());
            CORE::CTaskPtr task = future.GetResult(10000);
            ASSERT_TRUE(!task.IsNULL());
            ASSERT_TRUE(task->IsTaskInEndState());
            completedCount++;
        }
   
        ASSERT_TRUE(completedCount == numConcurrentTasks);
        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Concurrent operations test passed - " + CORE::ToString(completedCount) + " tasks completed");
    }
    catch(const std::exception& e)
    {
        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Exception in TestConcurrentOperations: " + CORE::ToString( e.what() ));
        ERRORHERE;
    }
}

void TestASyncConversionOperator()
{
    GUCEF_LOG(CORE::LOGLEVEL_NORMAL, " === Testing CASync Conversion Operator === ");
    
    try
    {
        // Test implicit conversion to CFutureResult
        CORE::CASync async;
        CORE::CFutureResult result = async.QueueCallback(SimpleCallback0);
        
        ASSERT_TRUE(result.HasAFuture());
        CORE::CTaskPtr task = result.GetResult(5000);
        ASSERT_TRUE(!task.IsNULL());
        ASSERT_TRUE(task->IsTaskInEndState());

        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "CASync conversion operator test passed");
    }
    catch(const std::exception& e)
    {
        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Exception in TestASyncConversionOperator: " + CORE::ToString( e.what() ));
        ERRORHERE;
    }
}

void TestClearChain()
{
    GUCEF_LOG(CORE::LOGLEVEL_NORMAL, " === Testing ClearChain Functionality === ");
    
    try
    {
        CASyncTestAccess async;
        
        // Build a chain
        async.QueueCallback( SimpleCallback1, 42 );

        CORE::CTaskPtr lastTask = async.GetLastTaskPublic();
        ASSERT_TRUE( !lastTask.IsNULL() );
        CASyncTestAccess::TASyncChainStatePtr state = async.GetChainStatePublic();
        ASSERT_TRUE( !state.IsNULL() );
        UInt64 lastTaskId = lastTask->GetTaskId();
        ASSERT_TRUE( 0 != lastTaskId );
        CORE::ThreadPoolPtr threadPool = async.GetThreadPoolPublic();

        // Clear the chain
        async.ClearChain();
        lastTask = async.GetLastTaskPublic();
        ASSERT_TRUE( lastTask.IsNULL() );
        
        // Build a new chain on the same async object
        CORE::CFutureResult result = async.QueueCallback( SimpleCallback2, 10, 20 );
        
        ASSERT_TRUE( result.HasAFuture() );
        CORE::CTaskPtr task = result.GetResult( 30000 );
        ASSERT_TRUE( !task.IsNULL() );
        ASSERT_TRUE( task->IsTaskInEndState() );
        UInt64 newTaskId = task->GetTaskId();
        ASSERT_TRUE( 0 != newTaskId );
        ASSERT_TRUE( lastTaskId != newTaskId );

        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "ClearChain test passed");
    }
    catch( const std::exception& e )
    {
        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Exception in TestClearChain: " + CORE::ToString( e.what() ));
        ERRORHERE;
    }
}

void TestTaskTypeOperations()
{
    GUCEF_LOG(CORE::LOGLEVEL_NORMAL, " === Testing Task Type Operations === ");
    
    try
    {
        // Test Start() method
        {
            CORE::CASync async;
            CORE::CFutureResult result = async.Start(CORE::CGenericCallbackTaskConsumer::TaskType);
    
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(5000);
            ASSERT_TRUE(!task.IsNULL());
            GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Start() method test passed");
        }
      
        // Test Queue() method
        {
            CORE::CASync async;
            CORE::CFutureResult result = async.Queue(CORE::CGenericCallbackTaskConsumer::TaskType);
    
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(5000);
            ASSERT_TRUE(!task.IsNULL());
            GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Queue() method test passed");
        }
    }
    catch(const std::exception& e)
    {
        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Exception in TestTaskTypeOperations: " + CORE::ToString( e.what() ));
        ERRORHERE;
    }
}

void TestSubmitMethod()
{
    GUCEF_LOG(CORE::LOGLEVEL_NORMAL, " === Testing Submit Method === ");
    
    try
    {
        CASyncTestAccess async;
        CORE::CTaskPtr lastTask = async.GetLastTaskPublic();
        ASSERT_TRUE( lastTask.IsNULL() );
        CASyncTestAccess::TASyncChainStatePtr state = async.GetChainStatePublic();
        ASSERT_TRUE( !state.IsNULL() );

        async.QueueCallback( SimpleCallback1, 123 );
        lastTask = async.GetLastTaskPublic();
        ASSERT_TRUE( !lastTask.IsNULL() );
        state = async.GetChainStatePublic();
        ASSERT_TRUE( !state.IsNULL() );

        // Explicitly call Submit()
        CORE::CFutureResult result = async.Submit();
   
        ASSERT_TRUE( result.HasAFuture() );
        CORE::CTaskPtr task = result.GetResult( 30000 );
        ASSERT_TRUE( !task.IsNULL() );
        ASSERT_TRUE( task->IsTaskInEndState() );
        ASSERT_TRUE( task == lastTask );

        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Submit method test passed");
    }
    catch( const std::exception& e )
    {
        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Exception in TestSubmitMethod: " + CORE::ToString( e.what() ));
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      MAIN TEST FUNCTION                                                 //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformASyncTests( void )
{
    GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "**** COMMENCING ASync TESTS ****");
    
    try
    {
        // Run all test cases
        TestBasicASyncConstruction();
        TestSimpleCallbacks();
        TestTaskChaining1Deep();
        TestTaskChaining3Deep();
        TestMemoryManagement();
        TestTaskChainCleanup();
        TestErrorHandling();
        TestConcurrentOperations();
        TestASyncConversionOperator();
        TestClearChain();
        TestTaskTypeOperations();
        TestSubmitMethod();

        TestFinalTaskCountsAfterTests();

        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "**** ALL ASync TESTS COMPLETED SUCCESSFULLY ****");
    }
    catch( const std::exception& e )
    {
        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Exception in PerformASyncTests: " + CORE::ToString( e.what() ));
        ERRORHERE;
    }
    catch( ... )
    {
        GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Unknown exception in PerformASyncTests");
        ERRORHERE;
    }

    GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "**** FINISHED ASync TESTS ****");
}

/*-------------------------------------------------------------------------*/
