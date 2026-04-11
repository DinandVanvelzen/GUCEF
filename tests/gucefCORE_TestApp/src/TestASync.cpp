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

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

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

#include "TestASync.h"

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

int IDForChainCallback1 = 1;
int IDForChainCallback2 = 2;
int IDForChainCallback3 = 3;
int IDForChainFwdCallback0 = 4;
int IDForChainFwdCallback0B = 5;
int IDForChainFwdCallback1 = 6;
int IDForChainFwdCallback1B = 7;
int IDForChainFwdCallback2 = 8;
int IDForChainFwdCallback2B = 9;
int IDForChainFwdCallback3 = 10;
int IDForChainFwdCallback3B = 11;
int IDForChainFwdCallback4 = 12;
int IDForChainFwdCallback4B = 13;


// Chain callback functions
int ChainCallback1( CORE::CTaskPtr taskPtr )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ChainCallback1 called with task ID: " + CORE::ToString( taskPtr->GetTaskId() ) );
    
    MT::CScopeMutex lock(g_testMutex);
    g_testResults.push_back( IDForChainCallback1 );
    return 100;
}

int ChainCallback2( CORE::CTaskPtr taskPtr, int multiplier )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ChainCallback2 called with task ID: " + CORE::ToString( taskPtr->GetTaskId() ) + ", multiplier: " + CORE::ToString( multiplier ) );
    
    MT::CScopeMutex lock(g_testMutex);
    g_testResults.push_back( IDForChainCallback2 );
    return 200 * multiplier;
}

int ChainCallback3( CORE::CTaskPtr taskPtr, int a, int b )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ChainCallback3 called with task ID: " + CORE::ToString( taskPtr->GetTaskId() ) + ", a: " + CORE::ToString( a ) + ", b: " + CORE::ToString( b ) );
    
    MT::CScopeMutex lock(g_testMutex);
    g_testResults.push_back( IDForChainCallback3 );
    return 300 + a + b;
}

int ChainForwardingCallback0( CORE::CTaskPtr taskPtr, int priorResult )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ChainForwardingCallback0 called with task ID: " + CORE::ToString( taskPtr->GetTaskId() ) + ", priorResult: " + CORE::ToString( priorResult ) );
    
    MT::CScopeMutex lock( g_testMutex );
    g_testResults.push_back( IDForChainFwdCallback0 );
    return priorResult + IDForChainFwdCallback0;
}

int ChainForwardingCallback0B( CORE::CTaskPtr taskPtr, int priorResult, CORE::CTaskPtr priortask )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ChainForwardingCallback0B called with task ID: " + CORE::ToString( taskPtr->GetTaskId() ) +
        ", priorResult: " + CORE::ToString( priorResult ) + ", priorTask with ID: " + CORE::ToString( priortask->GetTaskId() ) );
    
    MT::CScopeMutex lock( g_testMutex );
    g_testResults.push_back( IDForChainFwdCallback0B );
    return priorResult + IDForChainFwdCallback0B;
}

int ChainForwardingCallback1( CORE::CTaskPtr taskPtr, int priorResult, int priorParamA )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ChainForwardingCallback1 called with task ID: " + CORE::ToString( taskPtr->GetTaskId() ) + ", priorResult: " + CORE::ToString( priorResult ) + ", priorParamA: " + CORE::ToString( priorParamA ) );
    
    MT::CScopeMutex lock( g_testMutex );
    g_testResults.push_back( IDForChainFwdCallback1 );
    return priorResult + priorParamA + IDForChainFwdCallback1;
}

int ChainForwardingCallback1B( CORE::CTaskPtr taskPtr, int priorResult, CORE::CTaskPtr priortask, int priorParamA )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ChainForwardingCallback1B called with task ID: " + CORE::ToString( taskPtr->GetTaskId() ) +
        ", priorResult: " + CORE::ToString( priorResult ) + ", priorTask with ID: " + CORE::ToString( priortask->GetTaskId() ) +
        ", priorParamA: " + CORE::ToString( priorParamA )  );
    
    MT::CScopeMutex lock( g_testMutex );
    g_testResults.push_back( IDForChainFwdCallback1B );
    return priorResult + priorParamA + IDForChainFwdCallback1B;
}

int ChainForwardingCallback2( CORE::CTaskPtr taskPtr, int priorResult, int priorParamA, int priorParamB )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ChainForwardingCallback1 called with task ID: " + CORE::ToString( taskPtr->GetTaskId() ) +
        ", priorResult: " + CORE::ToString( priorResult ) + ", priorParamA: " + CORE::ToString( priorParamA ) + ", priorParamB: " + CORE::ToString( priorParamB ) );
    
    MT::CScopeMutex lock( g_testMutex );
    g_testResults.push_back( IDForChainFwdCallback2 );
    return priorResult + priorParamA + priorParamB + IDForChainFwdCallback2;
}

int ChainForwardingCallback2B( CORE::CTaskPtr taskPtr, int priorResult, CORE::CTaskPtr priortask, int priorParamA, int priorParamB )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ChainForwardingCallback2B called with task ID: " + CORE::ToString( taskPtr->GetTaskId() ) +
        ", priorResult: " + CORE::ToString( priorResult ) + ", priorTask with ID: " + CORE::ToString( priortask->GetTaskId() ) +
        ", priorParamA: " + CORE::ToString( priorParamA ) + ", priorParamB: " + CORE::ToString( priorParamB ) );
    
    MT::CScopeMutex lock( g_testMutex );
    g_testResults.push_back( IDForChainFwdCallback2B );
    return priorResult + priorParamA + priorParamB + IDForChainFwdCallback2B;
}

int ChainForwardingCallback3( CORE::CTaskPtr taskPtr, int priorResult, int priorParamA, int priorParamB, int priorParamC )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ChainForwardingCallback3 called with task ID: " + CORE::ToString( taskPtr->GetTaskId() ) +
        ", priorResult: " + CORE::ToString( priorResult ) + ", priorParamA: " + CORE::ToString( priorParamA ) +
        ", priorParamB: " + CORE::ToString( priorParamB ) + ", priorParamC: " + CORE::ToString( priorParamC ) );
    
    MT::CScopeMutex lock( g_testMutex );
    g_testResults.push_back( IDForChainFwdCallback3 );
    return priorResult + priorParamA + priorParamB + priorParamC + IDForChainFwdCallback3;
}

int ChainForwardingCallback3B( CORE::CTaskPtr taskPtr, int priorResult, CORE::CTaskPtr priortask, int priorParamA, int priorParamB, int priorParamC )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ChainForwardingCallback3B called with task ID: " + CORE::ToString( taskPtr->GetTaskId() ) +
        ", priorResult: " + CORE::ToString( priorResult ) + ", priorTask with ID: " + CORE::ToString( priortask->GetTaskId() ) +
        ", priorParamA: " + CORE::ToString( priorParamA ) +
        ", priorParamB: " + CORE::ToString( priorParamB ) + ", priorParamC: " + CORE::ToString( priorParamC ) );
    
    MT::CScopeMutex lock( g_testMutex );
    g_testResults.push_back( IDForChainFwdCallback3B );
    return priorResult + priorParamA + priorParamB + priorParamC + IDForChainFwdCallback3B;
}

int ChainForwardingCallback4( CORE::CTaskPtr taskPtr, int priorResult, int priorParamA, int priorParamB, int priorParamC, int priorParamD )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ChainForwardingCallback4 called with task ID: " + CORE::ToString( taskPtr->GetTaskId() ) +
        ", priorResult: " + CORE::ToString( priorResult ) + ", priorParamA: " + CORE::ToString( priorParamA ) +
        ", priorParamB: " + CORE::ToString( priorParamB ) + ", priorParamC: " + CORE::ToString( priorParamC ) +
        ", priorParamD: " + CORE::ToString( priorParamD ) );
    
    MT::CScopeMutex lock( g_testMutex );
    g_testResults.push_back( IDForChainFwdCallback4 );
    return priorResult + priorParamA + priorParamB + priorParamC + priorParamD + IDForChainFwdCallback4;
}

int ChainForwardingCallback4B( CORE::CTaskPtr taskPtr, int priorResult, CORE::CTaskPtr priortask, int priorParamA, int priorParamB, int priorParamC, int priorParamD )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ChainForwardingCallback4B called with task ID: " + CORE::ToString( taskPtr->GetTaskId() ) +
        ", priorResult: " + CORE::ToString( priorResult ) + ", priorTask with ID: " + CORE::ToString( priortask->GetTaskId() ) +
        ", priorParamA: " + CORE::ToString( priorParamA ) +
        ", priorParamB: " + CORE::ToString( priorParamB ) + ", priorParamC: " + CORE::ToString( priorParamC ) +
        ", priorParamD: " + CORE::ToString( priorParamD ) );
    
    MT::CScopeMutex lock( g_testMutex );
    g_testResults.push_back( IDForChainFwdCallback4B );
    return priorResult + priorParamA + priorParamB + priorParamC + priorParamD + IDForChainFwdCallback4B;
}

/*----------------------------------------------------------------------*/
// Helper class for member-function callback tests.
// Inherits CTSharedObjCreator so both raw and shared-ptr tests share the same type.

static int g_memberCallbackResult = 0;

class MemberCallbackTest : public CORE::CTSharedObjCreator< MemberCallbackTest, MT::CMutex >
{
    public:

    int m_value;

    MemberCallbackTest( int value = 0 )
        : CORE::CTSharedObjCreator< MemberCallbackTest, MT::CMutex >( this )
        , m_value( value )
    {}

    // Arity-0 member (no args beyond implicit this)
    int Method0()
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MemberCallbackTest::Method0 called" );
        MT::CScopeMutex lock( g_testMutex );
        g_memberCallbackResult = m_value + 100;
        return g_memberCallbackResult;
    }

    // Arity-1 member (one user arg)
    int Method1( int a )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MemberCallbackTest::Method1 called a=" + CORE::ToString( a ) );
        MT::CScopeMutex lock( g_testMutex );
        g_memberCallbackResult = m_value + a;
        return g_memberCallbackResult;
    }

    // Then-member: takes CTaskPtr (mandatory chain arg)
    int ChainMethod( CORE::CTaskPtr taskPtr )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MemberCallbackTest::ChainMethod called taskId=" + CORE::ToString( taskPtr->GetTaskId() ) );
        MT::CScopeMutex lock( g_testMutex );
        g_memberCallbackResult += 200;
        return g_memberCallbackResult;
    }

    // Forward arity-0: receives (CTaskPtr, priorResult)
    int FwdMethod0( CORE::CTaskPtr taskPtr, int priorResult )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MemberCallbackTest::FwdMethod0 priorResult=" + CORE::ToString( priorResult ) );
        MT::CScopeMutex lock( g_testMutex );
        g_memberCallbackResult = priorResult + m_value;
        return g_memberCallbackResult;
    }

    // Forward arity-1: receives (CTaskPtr, priorResult, priorA1)
    int FwdMethod1( CORE::CTaskPtr taskPtr, int priorResult, int priorA1 )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MemberCallbackTest::FwdMethod1 priorResult=" + CORE::ToString( priorResult ) + " priorA1=" + CORE::ToString( priorA1 ) );
        MT::CScopeMutex lock( g_testMutex );
        g_memberCallbackResult = priorResult + priorA1 + m_value;
        return g_memberCallbackResult;
    }
};

typedef CORE::CTSharedPtr< MemberCallbackTest, MT::CMutex > MemberCallbackTestPtr;

/*----------------------------------------------------------------------*/

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
        : CORE::CASync( threadPoolName )
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

bool
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

             // check if the remaining work is specific to the 'other' pools
             if ( ( 0 == poolNrOfInUseTasks && 0 == poolNrOfActiveTasks && 0 == poolNrOfDormantTasks ) &&
                  ( 0 != nrOfInUseTasks || 0 != nrOfActiveTasks || 0 != nrOfDormantTasks ) )
             {
                 // Waiting for other pools, lets get some extra diagnostics

                 CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();

                 CORE::CTaskManager::ThreadPoolVector pools;
                 taskManager.GetAllThreadPools( pools );
                 CORE::CTaskManager::ThreadPoolVector::iterator n = pools.begin();
                 while ( n != pools.end() )
                 {
                     CORE::ThreadPoolPtr otherPool = *n;
                     if ( otherPool != threadPool )
                     {
                         UInt32 otherPoolNrOfInUseTasks = 0;
                         UInt32 otherPoolNrOfActiveTasks = 0;
                         UInt32 otherPoolNrOfDormantTasks = 0;
                         UInt32 otherPoolNrOfFreeTaskObjs = 0;
                         ASSERT_TRUE( otherPool->GetTaskTotals( otherPoolNrOfInUseTasks, otherPoolNrOfActiveTasks, otherPoolNrOfDormantTasks, otherPoolNrOfFreeTaskObjs ) );
                         if ( otherPoolNrOfInUseTasks != 0 || otherPoolNrOfActiveTasks != 0 || otherPoolNrOfDormantTasks != 0 )
                         {
                             GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Waiting for other pool '" + otherPool->GetThreadPoolName() + "' to complete its tasks. Current counts - InUse=" + CORE::ToString( otherPoolNrOfInUseTasks ) +
                                 ", Active=" + CORE::ToString( otherPoolNrOfActiveTasks ) +
                                 ", Dormant=" + CORE::ToString( otherPoolNrOfDormantTasks ) +
                                 ", FreeObjs=" + CORE::ToString( otherPoolNrOfFreeTaskObjs ) );
                         }
                     }
                     ++n;
                 }
             }

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
        return false;
    }
    return true;
}

void TestBasicASyncConstruction()
{
    GUCEF_TESTFW_TESTCASE( "Test 1: Basic CASync Construction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: Basic CASync Construction" );
            
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
    GUCEF_TESTFW_TESTCASE_END
}

void TestSimpleCallbacks()
{
    GUCEF_TESTFW_TESTCASE( "Test 2: Simple Callbacks" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: Simple Callbacks" );

            // Test callback with no parameters
            {
                CASyncTestAccess async;
                CORE::CFutureResult result = async.QueueCallback( SimpleCallback0 );
         
                ASSERT_TRUE(result.HasAFuture());
                CORE::CTaskPtr task = result.GetResult(50000);
                ASSERT_TRUE(!task.IsNULL());
                ASSERT_TRUE(task->IsTaskInEndState());
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Simple callback 0 test passed" );
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
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Simple callback 1 test passed" );
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
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Simple callback 2 test passed" );
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
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Simple callback 3 test passed" );
            }
            TestFinalTaskCountsAfterTests();
     
            // Test callback with 4 parameters
            {
                CASyncTestAccess async;
                CORE::CFutureResult result = async.QueueCallback( SimpleCallback4, 1, 2, 3, 4);
       
                ASSERT_TRUE(result.HasAFuture());
                CORE::CTaskPtr task = result.GetResult(50000);
                ASSERT_TRUE(!task.IsNULL());
                ASSERT_TRUE(task->IsTaskInEndState());
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Simple callback 4 test passed" );
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
    GUCEF_TESTFW_TESTCASE_END
}

void TestTaskChaining1Deep()
{
    GUCEF_TESTFW_TESTCASE( "Test 3: Task Chaining 1 Deep" )
        try
        {
            GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Test 3: Task Chaining 1 Deep");
            g_testResults.clear();
            
            // Test simple chain
            {
                CASyncTestAccess async;
                CORE::CFutureResult result = async.QueueCallback( SimpleCallback0 )
                                                  .ThenCallback( ChainCallback1 );
         
                ASSERT_TRUE(result.HasAFuture());
                CORE::CTaskPtr task = result.GetResult( 10000 );
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
    GUCEF_TESTFW_TESTCASE_END
}

void TestTaskChaining3Deep()
{
    GUCEF_TESTFW_TESTCASE( "Test 4: Task Chaining 3 Deep" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: Task Chaining 3 Deep" );
            g_testResults.clear();
            
            // Test simple chain
            {
                CASyncTestAccess async;
                CORE::CFutureResult result = async.QueueCallback( SimpleCallback0 )
                                                  .ThenCallback( ChainCallback1 )
                                                  .ThenCallback( ChainCallback2, 3 )
                                                  .ThenCallback( ChainCallback3, 10, 20 );
         
                ASSERT_TRUE(result.HasAFuture());
                CORE::CTaskPtr task = result.GetResult(1000800000);
                ASSERT_TRUE(!task.IsNULL());
                ASSERT_TRUE(task->IsTaskInEndState());
                
                // Verify the chain executed in order
                MT::CScopeMutex lock(g_testMutex);
                ASSERT_TRUE(g_testResults.size() == 3);
                ASSERT_TRUE(g_testResults[0] == 1);
                ASSERT_TRUE(g_testResults[1] == 2);
                ASSERT_TRUE(g_testResults[2] == 3);
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Task chaining test passed" );
            }
          
            g_testResults.clear();

            TestFinalTaskCountsAfterTests();

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Task Chaining 3 Deep test completed" );
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
    GUCEF_TESTFW_TESTCASE_END
}

void TestTaskChaining3DeepWithResultPassing()
{
    GUCEF_TESTFW_TESTCASE( "Test 5: Task Chaining 3 Deep With Result Passing" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: Task Chaining 3 Deep With Prior Result Passing" );
            g_testResults.clear();
            
            // Test chain going back and forth with prior result passing
            {
                CASyncTestAccess async;
                
                CORE::CFutureResult result = async.QueueCallback( SimpleCallback1, 6 )
                                                  .ThenCallback( ChainCallback1 )
                                                  .ThenPassToCallback( ChainForwardingCallback0B )
                                                  .ThenCallback( ChainCallback2, 3 )
                                                  .ThenPassToCallback( ChainForwardingCallback1B )
                                                  .ThenCallback( ChainCallback3, 3, 6 )
                                                  .ThenPassToCallback( ChainForwardingCallback2B );

                ASSERT_TRUE(result.HasAFuture());
                CORE::CTaskPtr task = result.GetResult(10000);
                ASSERT_TRUE(!task.IsNULL());
                ASSERT_TRUE(task->IsTaskInEndState());
                
                // Verify the chain executed in order
                MT::CScopeMutex lock(g_testMutex);
                ASSERT_TRUE( g_testResults.size() == 6 );
                ASSERT_TRUE( g_testResults[0] == IDForChainCallback1 );
                ASSERT_TRUE( g_testResults[1] == IDForChainFwdCallback0B );
                ASSERT_TRUE( g_testResults[2] == IDForChainCallback2 );
                ASSERT_TRUE( g_testResults[3] == IDForChainFwdCallback1B );
                ASSERT_TRUE( g_testResults[4] == IDForChainCallback3 );
                ASSERT_TRUE( g_testResults[5] == IDForChainFwdCallback2B );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Task chaining test passed" );
            }
          
            g_testResults.clear();

            TestFinalTaskCountsAfterTests();

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Task Chaining 3 Deep With Prior Result Passing test completed" );
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
    GUCEF_TESTFW_TESTCASE_END
}

void TestMemoryManagement()
{
    GUCEF_TESTFW_TESTCASE( "Test 6: Memory Management" )
        try
        {
            GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Test 6: Memory Management");
            
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
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Initial tracker count: " + CORE::ToString( initialCount ) );
       
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

            ASSERT_TRUE( TestFinalTaskCountsAfterTests() );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Memory management test passed - no leaks detected" );
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
    GUCEF_TESTFW_TESTCASE_END
}

void TestTaskChainCleanup()
{
    GUCEF_TESTFW_TESTCASE( "Test 7: Task Chain Cleanup" )
        try
        {
            GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Test 7: Task Chain Cleanup");
            
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

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Task chain cleanup test completed");
        }
        catch(const std::exception& e)
        {
            GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Exception in TestTaskChainCleanup: " + CORE::ToString( e.what() ));
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

void TestErrorHandling()
{
    GUCEF_TESTFW_TESTCASE( "Test 8: Error Handling" )
        try
        {
            GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Test 8: Error Handling");
            
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
                CASyncTestAccess async;
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
    GUCEF_TESTFW_TESTCASE_END
}

void TestConcurrentOperations()
{
    GUCEF_TESTFW_TESTCASE( "Test 9: Concurrent Operations" )
        try
        {
            GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Test 9: Concurrent Operations");
            const int numConcurrentTasks = 20;

            CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( CORE::CTaskManager::DefaultThreadPoolName );
            threadPool->SetDesiredMinNrOfWorkerThreads( numConcurrentTasks );
            
            std::vector<CORE::CFutureResult> futures;
            futures.reserve( numConcurrentTasks );
        
            // Launch multiple concurrent async operations
            for (int i = 0; i < numConcurrentTasks; ++i)
            {
                CASyncTestAccess async( threadPool );
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
    GUCEF_TESTFW_TESTCASE_END
}

void TestASyncConversionOperator()
{
    GUCEF_TESTFW_TESTCASE( "Test 10: CASync Conversion Operator" )
        try
        {
            GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Test 10: CASync Conversion Operator");
            
            // Test implicit conversion to CFutureResult
            CASyncTestAccess async;
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
    GUCEF_TESTFW_TESTCASE_END
}

void TestClearChain()
{
    GUCEF_TESTFW_TESTCASE( "Test 11: ClearChain Functionality" )
        try
        {
            GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Test 11: ClearChain Functionality");
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

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ClearChain test passed" );
        }
        catch( const std::exception& e )
        {
            GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Exception in TestClearChain: " + CORE::ToString( e.what() ));
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

void TestTaskTypeOperations()
{
    GUCEF_TESTFW_TESTCASE( "Test 12: Task Type Operations" )
        try
        {
            GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Test 12: Task Type Operations");
            
            // Test Start() method
            {
                CASyncTestAccess async;
                CORE::CFutureResult result = async.Start( CORE::CGenericCallbackTaskConsumer::TaskType );
        
                ASSERT_TRUE( result.HasAFuture() );
                CORE::CTaskPtr task = result.GetResult( 5000 );
                ASSERT_TRUE( !task.IsNULL() );
                ASSERT_TRUE( task->IsTaskInEndState() );

                // we need to give some time for the internal cleanup to complete
                MT::PrecisionDelay( 3000 );

                // Only held by us now that it's done in local scope which means
                //  1 reference from the CASync object internally
                //  1 reference from the CFutureResult object internally
                //  1 reference from the extracted task object in the local scope
                ASSERT_TRUE( 3 == task.GetReferenceCount() ); 

                GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Start() method test passed");
            }
          
            // Test Queue() method
            {
                CASyncTestAccess async;
                CORE::CFutureResult result = async.Queue( CORE::CGenericCallbackTaskConsumer::TaskType );
        
                ASSERT_TRUE( result.HasAFuture() );
                CORE::CTaskPtr task = result.GetResult(5000);
                ASSERT_TRUE(!task.IsNULL());
                ASSERT_TRUE( task->IsTaskInEndState() );

                // we need to give some time for the internal cleanup to complete
                MT::PrecisionDelay( 3000 );

                // Only held by us now that it's done in local scope which means
                //  1 reference from the CASync object internally
                //  1 reference from the CFutureResult object internally
                //  1 reference from the extracted task object in the local scope
                ASSERT_TRUE( 3 == task.GetReferenceCount() );

                GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Queue() method test passed");
            }

            ASSERT_TRUE( TestFinalTaskCountsAfterTests() );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Task Type Operations test passed" );
        }
        catch(const std::exception& e)
        {
            GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Exception in TestTaskTypeOperations: " + CORE::ToString( e.what() ));
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

void TestSubmitMethod()
{
    GUCEF_TESTFW_TESTCASE( "Test 13: Submit Method" )
        try
        {
            GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Test 13: Submit Method");
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

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Submit method test passed" );
        }
        catch( const std::exception& e )
        {
            GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "Exception in TestSubmitMethod: " + CORE::ToString( e.what() ));
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

void TestMemberCallbacksRaw()
{
    GUCEF_TESTFW_TESTCASE( "Test 14: Member Function Callbacks (Raw Pointer)" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 14: Member Function Callbacks (Raw Pointer)" );
            g_memberCallbackResult = 0;

            MemberCallbackTest obj( 10 );
            CASyncTestAccess async;

            // arity 0: QueueMemberCallback(&obj, &Method0)
            {
                g_memberCallbackResult = 0;
                CORE::CFutureResult result = async.QueueMemberCallback( &obj, &MemberCallbackTest::Method0 );
                ASSERT_TRUE( result.HasAFuture() );
                CORE::CTaskPtr task = result.GetResult( 10000 );
                ASSERT_TRUE( !task.IsNULL() );
                ASSERT_TRUE( task->IsTaskInEndState() );
                MT::CScopeMutex lock( g_testMutex );
                ASSERT_TRUE( g_memberCallbackResult == 110 ); // m_value(10) + 100
            }

            // arity 1: QueueMemberCallback(&obj, &Method1, 5)
            {
                g_memberCallbackResult = 0;
                CORE::CFutureResult result = async.QueueMemberCallback( &obj, &MemberCallbackTest::Method1, 5 );
                ASSERT_TRUE( result.HasAFuture() );
                CORE::CTaskPtr task = result.GetResult( 10000 );
                ASSERT_TRUE( !task.IsNULL() );
                ASSERT_TRUE( task->IsTaskInEndState() );
                MT::CScopeMutex lock( g_testMutex );
                ASSERT_TRUE( g_memberCallbackResult == 15 ); // m_value(10) + a(5)
            }

            ASSERT_TRUE( TestFinalTaskCountsAfterTests() );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Member Function Callbacks (Raw Pointer) test passed" );
        }
        catch( const std::exception& e )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Exception in TestMemberCallbacksRaw: " + CORE::ToString( e.what() ) );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

void TestMemberCallbacksSharedPtr()
{
    GUCEF_TESTFW_TESTCASE( "Test 15: Member Function Callbacks (Shared Pointer)" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 15: Member Function Callbacks (Shared Pointer)" );
            g_memberCallbackResult = 0;

            MemberCallbackTestPtr obj = MemberCallbackTest::CreateSharedObj();
            obj->m_value = 20;

            CASyncTestAccess async;

            // arity 0 via shared ptr: QueueMemberCallback(sharedPtr, &Method0)
            {
                g_memberCallbackResult = 0;
                CORE::CFutureResult result = async.QueueMemberCallback( obj, &MemberCallbackTest::Method0 );
                ASSERT_TRUE( result.HasAFuture() );
                CORE::CTaskPtr task = result.GetResult( 10000 );
                ASSERT_TRUE( !task.IsNULL() );
                ASSERT_TRUE( task->IsTaskInEndState() );
                MT::CScopeMutex lock( g_testMutex );
                ASSERT_TRUE( g_memberCallbackResult == 120 ); // m_value(20) + 100
            }

            // arity 1 via shared ptr: QueueMemberCallback(sharedPtr, &Method1, 7)
            {
                g_memberCallbackResult = 0;
                CORE::CFutureResult result = async.QueueMemberCallback( obj, &MemberCallbackTest::Method1, 7 );
                ASSERT_TRUE( result.HasAFuture() );
                CORE::CTaskPtr task = result.GetResult( 10000 );
                ASSERT_TRUE( !task.IsNULL() );
                ASSERT_TRUE( task->IsTaskInEndState() );
                MT::CScopeMutex lock( g_testMutex );
                ASSERT_TRUE( g_memberCallbackResult == 27 ); // m_value(20) + a(7)
            }

            ASSERT_TRUE( TestFinalTaskCountsAfterTests() );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Member Function Callbacks (Shared Pointer) test passed" );
        }
        catch( const std::exception& e )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Exception in TestMemberCallbacksSharedPtr: " + CORE::ToString( e.what() ) );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

void TestMemberCallbackChaining()
{
    GUCEF_TESTFW_TESTCASE( "Test 16: Member Function Callback Chaining (ThenMemberCallback)" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 16: Member Function Callback Chaining" );
            g_memberCallbackResult = 0;

            MemberCallbackTest obj( 10 );
            CASyncTestAccess async;

            // QueueMemberCallback (arity 0) -> ThenMemberCallback
            {
                g_memberCallbackResult = 0;
                CORE::CFutureResult result = async.QueueMemberCallback( &obj, &MemberCallbackTest::Method0 )
                                                  .ThenMemberCallback( &obj, &MemberCallbackTest::ChainMethod );
                ASSERT_TRUE( result.HasAFuture() );
                CORE::CTaskPtr task = result.GetResult( 10000 );
                ASSERT_TRUE( !task.IsNULL() );
                ASSERT_TRUE( task->IsTaskInEndState() );
                MT::CScopeMutex lock( g_testMutex );
                // Method0: g_memberCallbackResult = 10+100 = 110
                // ChainMethod: g_memberCallbackResult += 200  => 310
                ASSERT_TRUE( g_memberCallbackResult == 310 );
            }

            // Shared-ptr variant: QueueMemberCallback (arity 0) -> ThenMemberCallback
            {
                g_memberCallbackResult = 0;
                MemberCallbackTestPtr sharedObj = MemberCallbackTest::CreateSharedObj();
                sharedObj->m_value = 5;
                CORE::CFutureResult result = async.QueueMemberCallback( sharedObj, &MemberCallbackTest::Method0 )
                                                  .ThenMemberCallback( sharedObj, &MemberCallbackTest::ChainMethod );
                ASSERT_TRUE( result.HasAFuture() );
                CORE::CTaskPtr task = result.GetResult( 10000 );
                ASSERT_TRUE( !task.IsNULL() );
                ASSERT_TRUE( task->IsTaskInEndState() );
                MT::CScopeMutex lock( g_testMutex );
                // Method0: g_memberCallbackResult = 5+100 = 105
                // ChainMethod: g_memberCallbackResult += 200 => 305
                ASSERT_TRUE( g_memberCallbackResult == 305 );
            }

            ASSERT_TRUE( TestFinalTaskCountsAfterTests() );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Member Function Callback Chaining test passed" );
        }
        catch( const std::exception& e )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Exception in TestMemberCallbackChaining: " + CORE::ToString( e.what() ) );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

void TestMemberCallbackResultPassing()
{
    GUCEF_TESTFW_TESTCASE( "Test 17: Member Function Callback Result Passing (ThenPassToMemberCallback)" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 17: Member Function Callback Result Passing" );
            g_memberCallbackResult = 0;

            MemberCallbackTest obj( 10 );
            CASyncTestAccess async;

            // Raw: QueueMemberCallback(arity_1) -> ThenPassToMemberCallback (forwards result + a1)
            // Method1(5) returns 15 (m_value(10)+5); FwdMethod1 gets (taskPtr, 15, 5), returns 15+5+10=30
            {
                g_memberCallbackResult = 0;
                CORE::CFutureResult result =
                    async.QueueMemberCallback( &obj, &MemberCallbackTest::Method1, 5 )
                         .ThenPassToMemberCallback( &obj, &MemberCallbackTest::FwdMethod1 );
                ASSERT_TRUE( result.HasAFuture() );
                CORE::CTaskPtr task = result.GetResult( 10000 );
                ASSERT_TRUE( !task.IsNULL() );
                ASSERT_TRUE( task->IsTaskInEndState() );
                MT::CScopeMutex lock( g_testMutex );
                ASSERT_TRUE( g_memberCallbackResult == 30 );
            }

            // Shared: QueueMemberCallback(arity_1) -> ThenPassToMemberCallback
            // Method1(3) returns 13 (m_value(10)+3); FwdMethod1 gets (taskPtr, 13, 3), returns 13+3+10=26
            {
                g_memberCallbackResult = 0;
                MemberCallbackTestPtr sharedObj = MemberCallbackTest::CreateSharedObj();
                sharedObj->m_value = 10;
                CORE::CFutureResult result =
                    async.QueueMemberCallback( sharedObj, &MemberCallbackTest::Method1, 3 )
                         .ThenPassToMemberCallback( sharedObj, &MemberCallbackTest::FwdMethod1 );
                ASSERT_TRUE( result.HasAFuture() );
                CORE::CTaskPtr task = result.GetResult( 10000 );
                ASSERT_TRUE( !task.IsNULL() );
                ASSERT_TRUE( task->IsTaskInEndState() );
                MT::CScopeMutex lock( g_testMutex );
                ASSERT_TRUE( g_memberCallbackResult == 26 );
            }

            ASSERT_TRUE( TestFinalTaskCountsAfterTests() );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Member Function Callback Result Passing test passed" );
        }
        catch( const std::exception& e )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Exception in TestMemberCallbackResultPassing: " + CORE::ToString( e.what() ) );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      MAIN TEST FUNCTION                                                 //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformASyncTests( void )
{
    GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "COMMENCING CASync TESTS");
    
    GUCEF_TESTFW_SUITE_SCOPE( "CASync" );
    
    try
    {
        // Run all test cases
        TestBasicASyncConstruction();
        TestSimpleCallbacks();
        TestTaskChaining1Deep();
        TestTaskChaining3Deep();
        TestTaskChaining3DeepWithResultPassing();
        TestMemoryManagement();
        TestTaskChainCleanup();
        TestErrorHandling();
        TestConcurrentOperations();
        TestASyncConversionOperator();
        TestClearChain();
        TestTaskTypeOperations();
        TestSubmitMethod();
        TestMemberCallbacksRaw();
        TestMemberCallbacksSharedPtr();
        TestMemberCallbackChaining();
        TestMemberCallbackResultPassing();

        TestFinalTaskCountsAfterTests();
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

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG(CORE::LOGLEVEL_NORMAL, "ALL CASync TESTS COMPLETED");
}

/*-------------------------------------------------------------------------*/
