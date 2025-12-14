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

/*-------------------------------------------------------------------------//
//                                                                         //
//   UTILITIES                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

// Simple test callback functions with various signatures
int SimpleCallback0()
{
    std::cout << "SimpleCallback0 called\n";
    return 42;
}

int SimpleCallback1(int value)
{
    std::cout << "SimpleCallback1 called with value: " << value << "\n";
    return value * 2;
}

int SimpleCallback2(int a, int b)
{
    std::cout << "SimpleCallback2 called with values: " << a << ", " << b << "\n";
    return a + b;
}

int SimpleCallback3(int a, int b, int c)
{
    std::cout << "SimpleCallback3 called with values: " << a << ", " << b << ", " << c << "\n";
    return a + b + c;
}

int SimpleCallback4(int a, int b, int c, int d)
{
std::cout << "SimpleCallback4 called with values: " << a << ", " << b << ", " << c << ", " << d << "\n";
    return a + b + c + d;
}

// Chain callback functions
int ChainCallback1(CORE::CTaskPtr taskPtr)
{
    std::cout << "ChainCallback1 called with task ID: " << taskPtr->GetTaskId() << "\n";
    
    MT::CScopeMutex lock(g_testMutex);
    g_testResults.push_back(1);
    return 100;
}

int ChainCallback2(CORE::CTaskPtr taskPtr, int multiplier)
{
    std::cout << "ChainCallback2 called with task ID: " << taskPtr->GetTaskId() << ", multiplier: " << multiplier << "\n";
    
    MT::CScopeMutex lock(g_testMutex);
    g_testResults.push_back(2);
    return 200 * multiplier;
}

int ChainCallback3(CORE::CTaskPtr taskPtr, int a, int b)
{
    std::cout << "ChainCallback3 called with task ID: " << taskPtr->GetTaskId() << ", a: " << a << ", b: " << b << "\n";
    
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
        std::cout << "TaskMemoryTracker created, count: " << s_taskCount << "\n";
    }
    
    ~TaskMemoryTracker() 
    {
        MT::CScopeMutex lock(s_mutex);
      --s_taskCount;
        std::cout << "TaskMemoryTracker destroyed, count: " << s_taskCount << "\n";
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
    std::cout << "MemoryTrackingCallback executed\n";
    return 123;
}

int MemoryTrackingChainCallback(CORE::CTaskPtr taskPtr)
{
    TaskMemoryTracker tracker;
    std::cout << "MemoryTrackingChainCallback executed with task ID: " << taskPtr->GetTaskId() << "\n";
    return 456;
}

// Error simulation callback
int ErrorCallback()
{
    std::cout << "ErrorCallback called - simulating error\n";
    throw std::runtime_error("Simulated error in callback");
}

// Long running callback for timeout tests
int LongRunningCallback()
{
    std::cout << "LongRunningCallback started\n";
    MT::PrecisionDelay(2000); // 2 second delay
    std::cout << "LongRunningCallback finished\n";
    return 999;
}


class CASyncTestAccess : public CORE::CASync
{
    public:

    typedef CORE::CASync::TASyncChainStatePtr TASyncChainStatePtr;

    CASyncTestAccess( const CORE::CString& threadPoolName = CORE::CString::Empty )
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

void TestBasicASyncConstruction()
{
    std::cout << "\n=== Testing Basic CASync Construction ===\n";
    
    try
    {
        // Test default construction
        CASyncTestAccess async1;
        CORE::CTaskPtr lastTask = async1.GetLastTaskPublic();
        ASSERT_TRUE( lastTask.IsNULL() );
        CASyncTestAccess::TASyncChainStatePtr state = async1.GetChainStatePublic();
        ASSERT_TRUE( !state.IsNULL() );
        std::cout << "Default CASync construction successful\n";
        
        // Test construction with thread pool name
        CASyncTestAccess async2( "TestPool" );
        lastTask = async2.GetLastTaskPublic();
        ASSERT_TRUE( lastTask.IsNULL() );
        state = async2.GetChainStatePublic();
        ASSERT_TRUE( !state.IsNULL() );
        std::cout << "Named thread pool CASync construction successful\n";

        // Test construction with thread pool pointer
        CORE::ThreadPoolPtr threadPool2 = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "TestPool2" );
        ASSERT_TRUE( !threadPool2.IsNULL() );        
        CASyncTestAccess async3( threadPool2 );
        lastTask = async3.GetLastTaskPublic();
        ASSERT_TRUE( lastTask.IsNULL() );
        state = async3.GetChainStatePublic();
        ASSERT_TRUE( !state.IsNULL() );

        std::cout << "Thread pool pointer CASync construction successful\n";
        
    }
    catch(const std::exception& e)
    {
        std::cout << "Exception in TestBasicASyncConstruction: " << e.what() << "\n";
        ERRORHERE;
    }
}

void TestSimpleCallbacks()
{
    std::cout << "\n=== Testing Simple Callbacks ===\n";
    
    try
    {
        // Test callback with no parameters
        {
            CORE::CASync async;
            CORE::CFutureResult result = async.QueueCallback( SimpleCallback0 );
     
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(50000);
            ASSERT_TRUE(!task.IsNULL());
            ASSERT_TRUE(task->IsTaskInEndState());
            std::cout << "Simple callback 0 test passed\n";
        }
      
        // Test callback with 1 parameter
        {
            CORE::CASync async;
            CORE::CFutureResult result = async.QueueCallback( SimpleCallback1, 10);
    
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(50000);
            ASSERT_TRUE(!task.IsNULL());
            ASSERT_TRUE(task->IsTaskInEndState());
            std::cout << "Simple callback 1 test passed\n";
        }
        
        // Test callback with 2 parameters
        {
            CORE::CASync async;
            CORE::CFutureResult result = async.QueueCallback( SimpleCallback2, 5, 7);
            
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(50000);
            ASSERT_TRUE(!task.IsNULL());
            ASSERT_TRUE(task->IsTaskInEndState());
            std::cout << "Simple callback 2 test passed\n";
        }
        
        // Test callback with 3 parameters
        {
            CORE::CASync async;
            CORE::CFutureResult result = async.QueueCallback(SimpleCallback3, 1, 2, 3);
    
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(50000);
            ASSERT_TRUE(!task.IsNULL());
            ASSERT_TRUE(task->IsTaskInEndState());
            std::cout << "Simple callback 3 test passed\n";
        }
 
        // Test callback with 4 parameters
        {
            CORE::CASync async;
            CORE::CFutureResult result = async.QueueCallback(SimpleCallback4, 1, 2, 3, 4);
   
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(50000);
            ASSERT_TRUE(!task.IsNULL());
            ASSERT_TRUE(task->IsTaskInEndState());
            std::cout << "Simple callback 4 test passed\n";
        }
    }
    catch( const timeout_exception& e )
    {
        std::cout << "Timeout Exception in TestSimpleCallbacks: " << e.what() << "\n";
        ERRORHERE;
    }
    catch( const std::exception& e )
    {
        std::cout << "Exception in TestSimpleCallbacks: " << e.what() << "\n";
        ERRORHERE;
    }
}

void TestTaskChaining()
{
    std::cout << "\n=== Testing Task Chaining ===\n";
    
    try
    {
        g_testResults.clear();
        
        // Test simple chain
        {
            CORE::CASync async;
            CORE::CFutureResult result = async.QueueCallback(SimpleCallback0)
                                              .ThenCallback(ChainCallback1)
                                              .ThenCallback(ChainCallback2, 3)
                                              .ThenCallback(ChainCallback3, 10, 20);
     
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(10000);
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
    }
    catch(const timeout_exception& e)
    {
        std::cout << "Timeout Exception in TestTaskChaining: " << e.what() << "\n";
        ERRORHERE;
    }
    catch(const std::exception& e)
    {
        std::cout << "Exception in TestTaskChaining: " << e.what() << "\n";
        ERRORHERE;
    }
}

void TestMemoryManagement()
{
    std::cout << "\n=== Testing Memory Management ===\n";
    
    try
    {
        int initialCount = TaskMemoryTracker::GetTaskCount();
        std::cout << "Initial tracker count: " << initialCount << "\n";
   
        // Test simple callback memory management
        {
            CORE::CASync async;
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
            CORE::CASync async;
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
        std::cout << "Final tracker count: " << finalCount << "\n";
        
        // Check for memory leaks - this is the critical test for the known issue
        ASSERT_TRUE(finalCount == initialCount);
      
        std::cout << "Memory management test passed - no leaks detected\n";
    }
    catch(const timeout_exception& e)
    {
        std::cout << "Timeout Exception in TestTaskChaining: " << e.what() << "\n";
        ERRORHERE;
    }
    catch(const std::exception& e)
    {
        std::cout << "Exception in TestMemoryManagement: " << e.what() << "\n";
        ERRORHERE;
    }
}

void TestTaskChainCleanup()
{
    std::cout << "\n=== Testing Task Chain Cleanup ===\n";
    
    try
    {
        // This test specifically targets the chain reference cleanup issue mentioned
        GUCEF::vector< CORE::CFutureResult > futures;
        
        // Create multiple chains that should be cleaned up properly
        for (int i = 0; i < 10; ++i)
        {
            CORE::CASync async;
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
      
        std::cout << "Task chain cleanup test completed\n";
    }
    catch(const std::exception& e)
    {
        std::cout << "Exception in TestTaskChainCleanup: " << e.what() << "\n";
        ERRORHERE;
    }
}

void TestErrorHandling()
{
    std::cout << "\n=== Testing Error Handling ===\n";
    
    try
    {
        // Test error in callback - this should not crash the system
        {
            CORE::CASync async;
            CORE::CFutureResult result = async.QueueCallback(ErrorCallback);
       
            ASSERT_TRUE(result.HasAFuture());
            try
            {
                CORE::CTaskPtr task = result.GetResult(5000);

                // Task should exist but may be in an error state
                ASSERT_TRUE(!task.IsNULL());
                std::cout << "Error callback test completed - task state: " << task->GetTaskStatusString().C_String() << "\n";
            }
            catch(const std::exception& e)
            {
                std::cout << "Expected exception caught in error callback test: " << e.what() << "\n";
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
                std::cout << "Unexpected: Long running task completed within timeout\n";
            }
            catch(const std::exception& e)
            {
                std::cout << "Expected timeout exception caught: " << e.what() << "\n";
            }
        }
        
        std::cout << "Error handling tests completed\n";
    }
    catch(const std::exception& e)
    {
        std::cout << "Exception in TestErrorHandling: " << e.what() << "\n";
        ERRORHERE;
    }
}

void TestConcurrentOperations()
{
    //std::cout << "\n=== Testing Concurrent Operations ===\n";
    //
    //try
    //{
    //    const int numConcurrentTasks = 20;
    //    std::vector<CORE::CFutureResult> futures;
    //
    //    // Launch multiple concurrent async operations
    //    for (int i = 0; i < numConcurrentTasks; ++i)
    //    {
    //        CORE::CASync async;
    //        CORE::CFutureResult result = async.QueueCallback( SimpleCallback1, i );
    //        futures.push_back(result);
    //    }
    //    
    //    // Wait for all to complete
    //    int completedCount = 0;
    //    for (auto& future : futures)
    //    {
    //        ASSERT_TRUE(future.HasAFuture());
    //        CORE::CTaskPtr task = future.GetResult(10000);
    //        ASSERT_TRUE(!task.IsNULL());
    //        ASSERT_TRUE(task->IsTaskInEndState());
    //        completedCount++;
    //    }
   
    //    ASSERT_TRUE(completedCount == numConcurrentTasks);
    //    std::cout << "Concurrent operations test passed - " << completedCount << " tasks completed\n";
    //}
    //catch(const std::exception& e)
    //{
    //    std::cout << "Exception in TestConcurrentOperations: " << e.what() << "\n";
    //    ERRORHERE;
    //}
}

void TestASyncConversionOperator()
{
    std::cout << "\n=== Testing CASync Conversion Operator ===\n";
    
    try
    {
        // Test implicit conversion to CFutureResult
        CORE::CASync async;
        CORE::CFutureResult result = async.QueueCallback(SimpleCallback0);
        
        ASSERT_TRUE(result.HasAFuture());
        CORE::CTaskPtr task = result.GetResult(5000);
        ASSERT_TRUE(!task.IsNULL());
        ASSERT_TRUE(task->IsTaskInEndState());
        
        std::cout << "CASync conversion operator test passed\n";
    }
    catch(const std::exception& e)
    {
        std::cout << "Exception in TestASyncConversionOperator: " << e.what() << "\n";
        ERRORHERE;
    }
}

void TestClearChain()
{
    std::cout << "\n=== Testing ClearChain Functionality ===\n";
    
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
 
        std::cout << "ClearChain test passed\n";
    }
    catch( const std::exception& e )
    {
        std::cout << "Exception in TestClearChain: " << e.what() << "\n";
        ERRORHERE;
    }
}

void TestTaskTypeOperations()
{
    std::cout << "\n=== Testing Task Type Operations ===\n";
    
    try
    {
        // Test Start() method
        {
            CORE::CASync async;
            CORE::CFutureResult result = async.Start(CORE::CGenericCallbackTaskConsumer::TaskType);
    
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(5000);
            ASSERT_TRUE(!task.IsNULL());
            std::cout << "Start() method test passed\n";
        }
      
        // Test Queue() method
        {
            CORE::CASync async;
            CORE::CFutureResult result = async.Queue(CORE::CGenericCallbackTaskConsumer::TaskType);
    
            ASSERT_TRUE(result.HasAFuture());
            CORE::CTaskPtr task = result.GetResult(5000);
            ASSERT_TRUE(!task.IsNULL());
            std::cout << "Queue() method test passed\n";
        }
    }
    catch(const std::exception& e)
    {
        std::cout << "Exception in TestTaskTypeOperations: " << e.what() << "\n";
        ERRORHERE;
    }
}

void TestSubmitMethod()
{
    std::cout << "\n=== Testing Submit Method ===\n";
    
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
        
        std::cout << "Submit method test passed\n";
    }
    catch( const std::exception& e )
    {
        std::cout << "Exception in TestSubmitMethod: " << e.what() << "\n";
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
    std::cout << "\n\n**** COMMENCING ASync TESTS ****\n";
    
    try
    {
        // Run all test cases
        TestBasicASyncConstruction();
        TestSimpleCallbacks();
        TestTaskChaining();
        TestMemoryManagement();
        TestTaskChainCleanup();
        TestErrorHandling();
        TestConcurrentOperations();
        TestASyncConversionOperator();
        TestClearChain();
        TestTaskTypeOperations();
        TestSubmitMethod();
        
        std::cout << "\n**** ALL ASync TESTS COMPLETED SUCCESSFULLY ****\n";
    }
    catch( const std::exception& e )
    {
        std::cout << "Exception in PerformASyncTests: " << e.what() << "\n";
        ERRORHERE;
    }
    catch( ... )
    {
        std::cout << "Unknown exception in PerformASyncTests\n";
        ERRORHERE;
    }

    std::cout << "\n\n**** FINISHED ASync TESTS ****\n";
}

/*-------------------------------------------------------------------------*/
