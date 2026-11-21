/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#include <stdexcept>

#ifndef GUCEF_MT_H
#include "gucefMT.h"
#define GUCEF_MT_H
#endif /* GUCEF_MT_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#include "TestThreads.h"

#include "TestVfsFetchScheduler.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** A precondition: when it does not hold the case ends here, so that the lines after it do not meet what it should have made */
#define REQUIRE(t)      do { if ( !(t) ) { GUCEF_TESTFW_ASSERT_TRUE(t); throw std::runtime_error( "a precondition of the case does not hold" ); } } while (0)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** What the jobs of a case tell the case: how often each was run and ended, where, and with what */
struct SJobStats
{
    MT::CTAtomic< UInt32 > runs;
    MT::CTAtomic< UInt32 > ended;
    MT::CTAtomic< UInt32 > cancelSeen;
    MT::CTAtomic< UInt32 > lastResult;
    MT::CTAtomic< UInt64 > runThread;
    MT::CTAtomic< UInt64 > endThread;

    SJobStats( void ) : runs( 0 ), ended( 0 ), cancelSeen( 0 ), lastResult( 999 ), runThread( 0 ), endThread( 0 ) {}
};

struct SConcurrency
{
    MT::CTAtomic< Int32 > now;
    MT::CTAtomic< Int32 > peak;
    SConcurrency( void ) : now( 0 ), peak( 0 ) {}
};

enum EBehaviour
{
    B_OK = 0,
    B_FAIL,
    B_THROW,
    B_BLOCK_UNTIL_GATE,         /**< ends when the gate opens; a cancel ends it as cancelled */
    B_BLOCK_UNTIL_CANCEL,       /**< ends only when asked to stop */
    B_SHORT_SLEEP
};

class CFakeJob : public VFS::CIVfsFetchJob
{
    public:

    CFakeJob( const CORE::CString& key, const void* owner, EBehaviour behaviour, SJobStats* stats, MT::CTAtomic< bool >* gate = GUCEF_NULL, SConcurrency* concurrency = GUCEF_NULL )
        : VFS::CIVfsFetchJob(), m_key( key ), m_owner( owner ), m_behaviour( behaviour ), m_stats( stats ), m_gate( gate ), m_concurrency( concurrency )
    {}

    virtual VFS::EVfsFetchJobKind GetKind( void ) const GUCEF_VIRTUAL_OVERRIDE { return VFS::FETCHJOB_STAT; }
    virtual CORE::CString GetKey( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_key; }
    virtual const void* GetOwner( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_owner; }

    virtual VFS::CVfsCallResult Run( const MT::CTAtomic< bool >& cancelRequested, VFS::CVfsArrivalInfo& /* outcome */ ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_stats->runs.FetchAdd( 1 );
        m_stats->runThread.Store( MT::GetCurrentThreadID() );
        if ( GUCEF_NULL != m_concurrency )
        {
            const Int32 now = m_concurrency->now.FetchAdd( 1 ) + 1;
            Int32 peak = m_concurrency->peak.Load();
            while ( now > peak && !m_concurrency->peak.CompareExchangeStrong( peak, now ) ) {}
        }

        VFS::CVfsCallResult result;
        switch ( m_behaviour )
        {
            case B_FAIL: { result = VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "failed on purpose" ); break; }
            case B_THROW: { Leave(); throw std::runtime_error( "thrown on purpose" ); }
            case B_BLOCK_UNTIL_GATE:
            case B_BLOCK_UNTIL_CANCEL:
            {
                for ( UInt32 waited=0; waited<20000; ++waited )
                {
                    if ( cancelRequested.Load() )
                    {
                        m_stats->cancelSeen.FetchAdd( 1 );
                        result = VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_CANCELLED, "stopped" );
                        break;
                    }
                    if ( B_BLOCK_UNTIL_GATE == m_behaviour && m_gate->Load() )
                        break;
                    MT::ThreadDelay( 1 );
                }
                break;
            }
            case B_SHORT_SLEEP: { MT::ThreadDelay( 2 ); break; }
            default: break;
        }
        Leave();
        return result;
    }

    virtual void OnEnded( const VFS::CVfsCallResult& result, const VFS::CVfsArrivalInfo& /* outcome */ ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_stats->lastResult.Store( (UInt32) result.result );
        m_stats->endThread.Store( MT::GetCurrentThreadID() );
        m_stats->ended.FetchAdd( 1 );
    }

    private:

    void Leave( void )
    {
        if ( GUCEF_NULL != m_concurrency )
            m_concurrency->now.FetchSub( 1 );
    }

    CORE::CString m_key;
    const void* m_owner;
    EBehaviour m_behaviour;
    SJobStats* m_stats;
    MT::CTAtomic< bool >* m_gate;
    SConcurrency* m_concurrency;
};

VFS::CIVfsFetchJobPtr
MakeJob( const CORE::CString& key, const void* owner, EBehaviour behaviour, SJobStats* stats, MT::CTAtomic< bool >* gate = GUCEF_NULL, SConcurrency* concurrency = GUCEF_NULL )
{
    return VFS::CIVfsFetchJobPtr( GUCEF_NEW CFakeJob( key, owner, behaviour, stats, gate, concurrency ) );
}

bool
WaitFor( const MT::CTAtomic< UInt32 >& value, UInt32 expected, UInt32 timeoutMs )
{
    for ( UInt32 waited=0; waited<=timeoutMs; ++waited )
    {
        if ( value.Load() >= expected )
            return true;
        MT::ThreadDelay( 1 );
    }
    return false;
}

UInt32
NextRandom( UInt32& state )
{
    state = state * 1664525U + 1013904223U;
    return state >> 8;
}

/** Submits jobs from several threads, each with keys of its own */
class CSubmitters : public CTestThreadBody
{
    public:

    VFS::CVfsFetchScheduler* scheduler;
    SJobStats* stats;
    UInt32 perThread;
    MT::CTAtomic< UInt32 > accepted;
    MT::CTAtomic< UInt32 > refused;

    CSubmitters( VFS::CVfsFetchScheduler* s, SJobStats* st, UInt32 n ) : scheduler( s ), stats( st ), perThread( n ), accepted( 0 ), refused( 0 ) {}

    virtual void Run( UInt32 index ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 i=0; i<perThread; ++i )
        {
            const UInt32 jobIndex = index * perThread + i;
            const VFS::CVfsFetchScheduler::ESubmitResult result = scheduler->Submit( MakeJob( "many|" + CORE::ToString( jobIndex ), GUCEF_NULL, B_OK, &stats[ jobIndex ] ) );
            if ( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == result )
                accepted.FetchAdd( 1 );
            else
                refused.FetchAdd( 1 );
        }
    }
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVfsFetchSchedulerTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "VfsFetchScheduler" );

    GUCEF_TESTFW_TESTCASE( "FetchScheduler: a job runs on a thread of the pool and not on the thread that asked, and it ends once with what it returned" )
    try
    {
        VFS::CVfsFetchScheduler scheduler( "VfsSchedTestA" );
        SJobStats stats;
        ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == scheduler.Submit( MakeJob( "k1", GUCEF_NULL, B_FAIL, &stats ) ) );
        REQUIRE( WaitFor( stats.ended, 1, 4000 ) );
        REQUIRE( scheduler.WaitUntilIdle( 4000 ) );
        ASSERT_TRUE( 1 == stats.runs.Load() );
        ASSERT_TRUE( 1 == stats.ended.Load() );
        ASSERT_TRUE( (UInt32) CORE::FILESYSTEMACCESS_FAILED == stats.lastResult.Load() );
        ASSERT_TRUE( stats.runThread.Load() != (UInt64) MT::GetCurrentThreadID() );
        ASSERT_TRUE( !CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool( "VfsSchedTestA" ).IsNULL() );

        VFS::CVfsFetchScheduler::SCounters counters;
        scheduler.GetCounters( counters );
        ASSERT_TRUE( 1 == counters.accepted && 1 == counters.failed && 0 == counters.completed && 0 == counters.cancelled && 0 == counters.running && 0 == counters.queued );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FetchScheduler: the pool has a name that is a setting, VfsRemoteOps by default" )
    try
    {
        ASSERT_TRUE( CORE::CString( "VfsRemoteOps" ) == VFS::CVfsFetchScheduler::DefaultThreadPoolName );
        VFS::CVfsFetchScheduler scheduler;
        ASSERT_TRUE( VFS::CVfsFetchScheduler::DefaultThreadPoolName == scheduler.GetThreadPoolName() );
        scheduler.SetThreadPoolName( "VfsSchedTestNamed" );
        ASSERT_TRUE( CORE::CString( "VfsSchedTestNamed" ) == scheduler.GetThreadPoolName() );
        SJobStats stats;
        ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == scheduler.Submit( MakeJob( "k1", GUCEF_NULL, B_OK, &stats ) ) );
        REQUIRE( WaitFor( stats.ended, 1, 4000 ) );
        ASSERT_TRUE( !CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool( "VfsSchedTestNamed" ).IsNULL() );
        ASSERT_TRUE( (UInt32) CORE::FILESYSTEMACCESS_SUCCESS == stats.lastResult.Load() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FetchScheduler: a job with the key of a job that is queued or running is merged into it: one run, one end, and a key is free again once its job ended" )
    try
    {
        VFS::CVfsFetchScheduler scheduler( "VfsSchedTestB" );
        MT::CTAtomic< bool > gate( false );
        SJobStats first, second, third;
        ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == scheduler.Submit( MakeJob( "same", GUCEF_NULL, B_BLOCK_UNTIL_GATE, &first, &gate ) ) );
        REQUIRE( WaitFor( first.runs, 1, 4000 ) );
        ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_MERGED == scheduler.Submit( MakeJob( "same", GUCEF_NULL, B_OK, &second ) ) );
        ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_MERGED == scheduler.Submit( MakeJob( "same", GUCEF_NULL, B_OK, &third ) ) );
        ASSERT_TRUE( scheduler.HasJob( "same" ) );
        gate.Store( true );
        REQUIRE( scheduler.WaitUntilIdle( 4000 ) );
        ASSERT_TRUE( 1 == first.runs.Load() && 1 == first.ended.Load() );
        ASSERT_TRUE( 0 == second.runs.Load() && 0 == second.ended.Load() );
        ASSERT_TRUE( 0 == third.runs.Load() && 0 == third.ended.Load() );

        VFS::CVfsFetchScheduler::SCounters counters;
        scheduler.GetCounters( counters );
        ASSERT_TRUE( 1 == counters.accepted && 2 == counters.merged && 0 == counters.refused && 1 == counters.completed );

        // the key is free again
        SJobStats again;
        ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == scheduler.Submit( MakeJob( "same", GUCEF_NULL, B_OK, &again ) ) );
        REQUIRE( scheduler.WaitUntilIdle( 4000 ) );
        ASSERT_TRUE( 1 == again.ended.Load() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FetchScheduler: with one running and one queued job allowed, the third distinct job is refused and never ends, and the two that were accepted end once each, in order" )
    try
    {
        VFS::CVfsFetchScheduler scheduler( "VfsSchedTestC" );
        scheduler.SetMaxRunning( 1 );
        scheduler.SetMaxQueued( 1 );
        MT::CTAtomic< bool > gate( false );
        SJobStats a, b, c;
        ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == scheduler.Submit( MakeJob( "a", GUCEF_NULL, B_BLOCK_UNTIL_GATE, &a, &gate ) ) );
        REQUIRE( WaitFor( a.runs, 1, 4000 ) );
        ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == scheduler.Submit( MakeJob( "b", GUCEF_NULL, B_OK, &b ) ) );
        ASSERT_TRUE( 1 == scheduler.GetQueuedCount() );
        ASSERT_TRUE( 1 == scheduler.GetRunningCount() );
        ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_REFUSED == scheduler.Submit( MakeJob( "c", GUCEF_NULL, B_OK, &c ) ) );
        // the job that waits has not run: first in first out
        MT::ThreadDelay( 30 );
        ASSERT_TRUE( 0 == b.runs.Load() );
        gate.Store( true );
        REQUIRE( scheduler.WaitUntilIdle( 4000 ) );
        ASSERT_TRUE( 1 == a.ended.Load() && 1 == b.ended.Load() );
        ASSERT_TRUE( 0 == c.runs.Load() && 0 == c.ended.Load() );

        VFS::CVfsFetchScheduler::SCounters counters;
        scheduler.GetCounters( counters );
        ASSERT_TRUE( 2 == counters.accepted && 1 == counters.refused && 2 == counters.completed );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FetchScheduler: with no queue allowed a job beyond the running ones is refused, and the running bound is never passed by more jobs" )
    try
    {
        VFS::CVfsFetchScheduler scheduler( "VfsSchedTestD" );
        scheduler.SetMaxRunning( 2 );
        scheduler.SetMaxQueued( 0 );
        MT::CTAtomic< bool > gate( false );
        SConcurrency concurrency;
        SJobStats stats[ 5 ];
        UInt32 accepted = 0, refused = 0;
        for ( UInt32 i=0; i<5; ++i )
        {
            const VFS::CVfsFetchScheduler::ESubmitResult result = scheduler.Submit( MakeJob( "d" + CORE::ToString( i ), GUCEF_NULL, B_BLOCK_UNTIL_GATE, &stats[ i ], &gate, &concurrency ) );
            if ( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == result )
                ++accepted;
            else
            if ( VFS::CVfsFetchScheduler::SUBMIT_REFUSED == result )
                ++refused;
        }
        ASSERT_TRUE( 2 == accepted && 3 == refused );
        gate.Store( true );
        REQUIRE( scheduler.WaitUntilIdle( 4000 ) );
        ASSERT_TRUE( concurrency.peak.Load() <= 2 );
        UInt32 ended = 0;
        for ( UInt32 i=0; i<5; ++i )
            ended += stats[ i ].ended.Load();
        ASSERT_TRUE( 2 == ended );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FetchScheduler: a job that throws ends once as a failure and does not take the pool thread with it" )
    try
    {
        VFS::CVfsFetchScheduler scheduler( "VfsSchedTestE" );
        scheduler.SetMaxRunning( 1 );
        SJobStats thrower, after;
        ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == scheduler.Submit( MakeJob( "t", GUCEF_NULL, B_THROW, &thrower ) ) );
        ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == scheduler.Submit( MakeJob( "u", GUCEF_NULL, B_OK, &after ) ) );
        REQUIRE( scheduler.WaitUntilIdle( 4000 ) );
        ASSERT_TRUE( 1 == thrower.ended.Load() );
        ASSERT_TRUE( (UInt32) CORE::FILESYSTEMACCESS_FAILED == thrower.lastResult.Load() );
        ASSERT_TRUE( 1 == after.ended.Load() );
        ASSERT_TRUE( (UInt32) CORE::FILESYSTEMACCESS_SUCCESS == after.lastResult.Load() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FetchScheduler: CancelJob takes a queued job out and ends it as cancelled on the calling thread, tells a running job to stop, and knows no other key" )
    try
    {
        VFS::CVfsFetchScheduler scheduler( "VfsSchedTestF" );
        scheduler.SetMaxRunning( 1 );
        SJobStats running, queued;
        ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == scheduler.Submit( MakeJob( "run", GUCEF_NULL, B_BLOCK_UNTIL_CANCEL, &running ) ) );
        REQUIRE( WaitFor( running.runs, 1, 4000 ) );
        ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == scheduler.Submit( MakeJob( "wait", GUCEF_NULL, B_OK, &queued ) ) );

        ASSERT_FALSE( scheduler.CancelJob( "nothing" ) );
        ASSERT_TRUE( scheduler.CancelJob( "wait" ) );
        ASSERT_TRUE( 1 == queued.ended.Load() );
        ASSERT_TRUE( (UInt32) CORE::FILESYSTEMACCESS_CANCELLED == queued.lastResult.Load() );
        ASSERT_TRUE( (UInt64) MT::GetCurrentThreadID() == queued.endThread.Load() );
        ASSERT_TRUE( 0 == queued.runs.Load() );

        ASSERT_TRUE( scheduler.CancelJob( "run" ) );
        REQUIRE( scheduler.WaitUntilIdle( 4000 ) );
        ASSERT_TRUE( 1 == running.cancelSeen.Load() );
        ASSERT_TRUE( 1 == running.ended.Load() );
        ASSERT_TRUE( (UInt32) CORE::FILESYSTEMACCESS_CANCELLED == running.lastResult.Load() );

        VFS::CVfsFetchScheduler::SCounters counters;
        scheduler.GetCounters( counters );
        ASSERT_TRUE( 2 == counters.accepted && 2 == counters.cancelled );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FetchScheduler: CancelAll of an owner drops its running and queued jobs, waits for the running ones, and nothing of them is called after it returned; the jobs of another owner go on" )
    try
    {
        VFS::CVfsFetchScheduler scheduler( "VfsSchedTestG" );
        scheduler.SetCancelWaitMs( 3000 );
        scheduler.SetMaxRunning( 2 );
        int ownerA = 0, ownerB = 0;
        MT::CTAtomic< bool > gate( false );
        SJobStats a[ 5 ], b;
        ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == scheduler.Submit( MakeJob( "a0", &ownerA, B_BLOCK_UNTIL_CANCEL, &a[ 0 ] ) ) );
        ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == scheduler.Submit( MakeJob( "a1", &ownerA, B_BLOCK_UNTIL_CANCEL, &a[ 1 ] ) ) );
        REQUIRE( WaitFor( a[ 0 ].runs, 1, 10000 ) && WaitFor( a[ 1 ].runs, 1, 4000 ) );
        for ( UInt32 i=2; i<5; ++i )
        {
            ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == scheduler.Submit( MakeJob( "a" + CORE::ToString( i ), &ownerA, B_OK, &a[ i ] ) ) );
        }
        ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == scheduler.Submit( MakeJob( "b0", &ownerB, B_BLOCK_UNTIL_GATE, &b, &gate ) ) );

        ASSERT_TRUE( 5 == scheduler.CancelAll( &ownerA ) );
        // the running ones saw the request and returned before CancelAll did; none of the five was reported
        ASSERT_TRUE( 1 == a[ 0 ].cancelSeen.Load() && 1 == a[ 1 ].cancelSeen.Load() );
        MT::ThreadDelay( 50 );
        for ( UInt32 i=0; i<5; ++i )
        {
            ASSERT_TRUE( 0 == a[ i ].ended.Load() );
        }
        for ( UInt32 i=2; i<5; ++i )
        {
            ASSERT_TRUE( 0 == a[ i ].runs.Load() );
        }

        // the other owner is not affected
        ASSERT_TRUE( 0 == b.ended.Load() );
        gate.Store( true );
        REQUIRE( scheduler.WaitUntilIdle( 4000 ) );
        ASSERT_TRUE( 1 == b.ended.Load() );
        ASSERT_TRUE( (UInt32) CORE::FILESYSTEMACCESS_SUCCESS == b.lastResult.Load() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FetchScheduler: the destructor drops the jobs that are under way: it returns, and nothing is called afterwards" )
    try
    {
        SJobStats running, queued;
        {
            VFS::CVfsFetchScheduler scheduler( "VfsSchedTestH" );
            scheduler.SetCancelWaitMs( 3000 );
            scheduler.SetMaxRunning( 1 );
            ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == scheduler.Submit( MakeJob( "r", GUCEF_NULL, B_BLOCK_UNTIL_CANCEL, &running ) ) );
            REQUIRE( WaitFor( running.runs, 1, 4000 ) );
            ASSERT_TRUE( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == scheduler.Submit( MakeJob( "q", GUCEF_NULL, B_OK, &queued ) ) );
        }
        MT::ThreadDelay( 50 );
        ASSERT_TRUE( 1 == running.cancelSeen.Load() );
        ASSERT_TRUE( 0 == running.ended.Load() && 0 == queued.ended.Load() && 0 == queued.runs.Load() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FetchScheduler: every accepted job of 1000 random jobs (successes, failures, exceptions, cancels) ends exactly once, the others never, and the counters add up" )
    try
    {
        VFS::CVfsFetchScheduler scheduler( "VfsSchedTestI" );
        scheduler.SetMaxRunning( 3 );
        scheduler.SetMaxQueued( 40 );
        const UInt32 jobCount = 1000;
        GUCEF::vector< SJobStats* > stats;
        GUCEF::vector< UInt32 > submitted;      // 0 refused, 1 accepted, 2 merged
        UInt32 random = 424242;
        for ( UInt32 i=0; i<jobCount; ++i )
        {
            stats.push_back( GUCEF_NEW SJobStats() );
            const UInt32 pick = NextRandom( random ) % 100;
            EBehaviour behaviour = B_OK;
            if ( pick < 20 ) behaviour = B_FAIL;
            else if ( pick < 30 ) behaviour = B_THROW;
            else if ( pick < 45 ) behaviour = B_BLOCK_UNTIL_CANCEL;
            else if ( pick < 60 ) behaviour = B_SHORT_SLEEP;

            // most jobs wait for room, so that most are accepted; every fourth does not, and may be refused
            for ( UInt32 waited=0; 0 != i % 4 && scheduler.GetQueuedCount() > 20 && waited < 20000; ++waited )
                MT::ThreadDelay( 1 );

            // some keys repeat on purpose: those merge while the first is still there
            const UInt32 keyNumber = ( 0 == NextRandom( random ) % 10 && i > 0 ) ? i - 1 : i;
            const VFS::CVfsFetchScheduler::ESubmitResult result = scheduler.Submit( MakeJob( "r" + CORE::ToString( keyNumber ), GUCEF_NULL, behaviour, stats[ i ] ) );
            submitted.push_back( VFS::CVfsFetchScheduler::SUBMIT_ACCEPTED == result ? 1 : ( VFS::CVfsFetchScheduler::SUBMIT_MERGED == result ? 2 : 0 ) );

            // jobs that wait for a cancel get it, or are cancelled while queued
            if ( B_BLOCK_UNTIL_CANCEL == behaviour && 1 == submitted.back() )
            {
                if ( 0 == NextRandom( random ) % 2 )
                    MT::ThreadDelay( 1 );
                scheduler.CancelJob( "r" + CORE::ToString( keyNumber ) );
            }
        }
        REQUIRE( scheduler.WaitUntilIdle( 4000 ) );

        UInt32 accepted = 0, endedWrong = 0, notAcceptedEnded = 0;
        for ( UInt32 i=0; i<jobCount; ++i )
        {
            if ( 1 == submitted[ i ] )
            {
                ++accepted;
                if ( 1 != stats[ i ]->ended.Load() )
                    ++endedWrong;
            }
            else
            if ( 0 != stats[ i ]->ended.Load() || 0 != stats[ i ]->runs.Load() )
                ++notAcceptedEnded;
        }
        VFS::CVfsFetchScheduler::SCounters counters;
        scheduler.GetCounters( counters );
        ASSERT_TRUE( 0 == endedWrong );
        ASSERT_TRUE( 0 == notAcceptedEnded );
        ASSERT_TRUE( accepted == counters.accepted );
        ASSERT_TRUE( counters.accepted == counters.completed + counters.failed + counters.cancelled );
        ASSERT_TRUE( 0 == counters.running && 0 == counters.queued );
        ASSERT_TRUE( accepted > 500 );
        for ( UInt32 i=0; i<jobCount; ++i )
            GUCEF_DELETE stats[ i ];
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FetchScheduler: several threads submitting at once: every accepted job ends once and the running bound holds" )
    try
    {
        VFS::CVfsFetchScheduler scheduler( "VfsSchedTestJ" );
        scheduler.SetMaxRunning( 3 );
        scheduler.SetMaxQueued( 100000 );
        const UInt32 threadCount = 4;
        const UInt32 perThread = 150;
        SJobStats* stats = GUCEF_NEW SJobStats[ threadCount * perThread ];
        {
            CSubmitters body( &scheduler, stats, perThread );
            CTestThreads threads( body, threadCount );
            ASSERT_TRUE( threads.Start() );
            ASSERT_TRUE( threads.Join( 120000 ) );
            ASSERT_TRUE( threadCount * perThread == body.accepted.Load() );
        }
        REQUIRE( scheduler.WaitUntilIdle( 4000 ) );
        UInt32 wrong = 0;
        for ( UInt32 i=0; i<threadCount * perThread; ++i )
        {
            if ( 1 != stats[ i ].ended.Load() || 1 != stats[ i ].runs.Load() )
                ++wrong;
        }
        ASSERT_TRUE( 0 == wrong );
        GUCEF_DELETE[] stats;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
