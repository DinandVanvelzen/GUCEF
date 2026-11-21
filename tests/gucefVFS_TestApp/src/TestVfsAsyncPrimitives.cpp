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

#include "TestVfsAsyncPrimitives.h"

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

/** A call that records where and when it ran, can wait to be let go, and looks at its cancellation source while it waits */
class CTestAsyncCall : public VFS::CIVfsAsyncCall
{
    public:

    MT::CTAtomic< UInt32 > started;
    MT::CTAtomic< UInt32 > finished;
    MT::CTAtomic< bool > release;                   /**< a call that blocks waits for this */
    MT::CTAtomic< bool > sawCancel;
    MT::CTAtomic< TThreadId > threadId;
    bool blocks;
    bool throws;

    CTestAsyncCall( bool blocking, bool throwing ) : VFS::CIVfsAsyncCall(), started( 0 ), finished( 0 ), release( false ), sawCancel( false ), threadId( 0 ), blocks( blocking ), throws( throwing ) {}

    virtual void Run( const VFS::CIVfsCancelSource& cancel ) GUCEF_VIRTUAL_OVERRIDE
    {
        threadId.Store( MT::GetCurrentThreadID() );
        started.FetchAdd( 1 );
        if ( throws )
            throw std::runtime_error( "a call that throws" );
        UInt32 waited = 0;
        while ( blocks && !release.Load() && waited < 10000 )
        {
            if ( cancel.IsCancelRequested() )
            {
                sawCancel.Store( true );
                break;
            }
            MT::ThreadDelay( 1 );
            ++waited;
        }
        finished.FetchAdd( 1 );
    }
};

typedef CORE::CTSharedPtr< CTestAsyncCall, MT::CMutex > CTestAsyncCallPtr;

/** A cancellation source that the case switches by hand */
class CSwitchSource : public VFS::CIVfsCancelSource
{
    public:

    MT::CTAtomic< bool > on;
    CSwitchSource( void ) : on( false ) {}
    virtual bool IsCancelRequested( void ) const GUCEF_VIRTUAL_OVERRIDE { return on.Load(); }
};

bool
WaitReady( const CORE::CFutureResult& future, UInt32 timeoutMs )
{
    for ( UInt32 waited=0; waited<=timeoutMs; ++waited )
    {
        if ( future.IsReady() )
            return true;
        MT::ThreadDelay( 1 );
    }
    return false;
}

bool
WaitFor( const MT::CTAtomic< UInt32 >& counter, UInt32 atLeast, UInt32 timeoutMs )
{
    for ( UInt32 waited=0; waited<=timeoutMs; ++waited )
    {
        if ( counter.Load() >= atLeast )
            return true;
        MT::ThreadDelay( 1 );
    }
    return false;
}

CORE::TTaskStatus
StatusOf( const CORE::CFutureResult& future )
{
    CORE::CTaskPtr task = future.GetResult( 5000 );
    return task.IsNULL() ? CORE::TASKSTATUS_UNDEFINED : task->GetTaskStatus();
}

/** A pool of its own with a number of workers; the call queues on it by its name */
CORE::ThreadPoolPtr
MakePool( const CORE::CString& name, UInt32 workers )
{
    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( name );
    if ( !pool.IsNULL() )
    {
        pool->SetDesiredMinNrOfWorkerThreads( workers );
        pool->SetDesiredMaxTotalNrOfThreads( (Int32) workers );
    }
    return pool;
}

/** Enters and leaves the gate in a loop */
class CGateWalkers : public CTestThreadBody
{
    public:

    VFS::CVfsAsyncCallGatePtr gate;
    MT::CTAtomic< UInt32 > entered;
    MT::CTAtomic< UInt32 > refused;
    MT::CTAtomic< UInt32 > insideAfterClose;

    explicit CGateWalkers( const VFS::CVfsAsyncCallGatePtr& g ) : gate( g ), entered( 0 ), refused( 0 ), insideAfterClose( 0 ) {}

    virtual void Run( UInt32 /* index */ ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 i=0; i<3000; ++i )
        {
            VFS::CVfsAsyncCallGateScope scope( gate );
            if ( scope.IsEntered() )
            {
                entered.FetchAdd( 1 );
                if ( gate->IsClosed() && gate->GetInside() == 0 )
                    insideAfterClose.FetchAdd( 1 );         // inside a closed gate with nobody counted inside: impossible
                MT::ThreadDelay( 0 );
            }
            else
            {
                refused.FetchAdd( 1 );
            }
        }
    }
};

/** Takes files from a work list until it is empty */
class CWorkTakers : public CTestThreadBody
{
    public:

    VFS::CVfsPrefetchWorkPtr work;
    MT::CMutex lock;
    GUCEF::map< CORE::CString, UInt32 > seen;
    MT::CTAtomic< UInt32 > total;

    explicit CWorkTakers( const VFS::CVfsPrefetchWorkPtr& w ) : work( w ), lock(), seen(), total( 0 ) {}

    virtual void Run( UInt32 /* index */ ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CString file;
        while ( work->TakeNext( file ) )
        {
            total.FetchAdd( 1 );
            MT::CScopeMutex scope( lock );
            ++seen[ file ];
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
PerformVfsAsyncPrimitivesTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "VfsAsyncPrimitives" );

    GUCEF_TESTFW_TESTCASE( "CVfsAsyncResult: made as a call that has not run (cancelled, nothing else set); CVfsFetchProgress: the cancel flag and a source of cancellation each make IsCancelRequested() true, the scope puts the source back to none" )
    try
    {
        VFS::CVfsAsyncResult result;
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_CANCELLED == result.result.result && !result.result.IsSuccess() );
        ASSERT_TRUE( !result.exists && result.names.empty() && result.resource.IsNULL() && 0 == result.data.GetDataSize() && 0 == result.bytesDone && result.digest.IsEmpty() );

        VFS::CVfsFetchProgressPtr progress( GUCEF_NEW VFS::CVfsFetchProgress() );
        ASSERT_FALSE( progress->IsCancelRequested() );
        progress->cancel.Store( true );
        ASSERT_TRUE( progress->IsCancelRequested() );
        progress->cancel.Store( false );

        CSwitchSource source;
        {
            VFS::CVfsFetchProgressCancelScope scope( progress, &source );
            ASSERT_FALSE( progress->IsCancelRequested() );
            source.on.Store( true );
            ASSERT_TRUE( progress->IsCancelRequested() );
            ASSERT_FALSE( progress->cancel.Load() );                    // the flag is the caller's, the source does not touch it
            source.on.Store( false );
            ASSERT_FALSE( progress->IsCancelRequested() );
            source.on.Store( true );
        }
        // the scope took the source away: it can go and is not asked any more
        ASSERT_FALSE( progress->IsCancelRequested() );

        // a scope on no progress does nothing
        {
            VFS::CVfsFetchProgressCancelScope scope( VFS::CVfsFetchProgressPtr(), &source );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CVfsEitherCancelSource: asks to stop when either of the two does, and not otherwise" )
    try
    {
        CSwitchSource a, b;
        VFS::CVfsEitherCancelSource either( a, b );
        ASSERT_FALSE( either.IsCancelRequested() );
        a.on.Store( true );
        ASSERT_TRUE( either.IsCancelRequested() );
        b.on.Store( true );
        ASSERT_TRUE( either.IsCancelRequested() );
        a.on.Store( false );
        ASSERT_TRUE( either.IsCancelRequested() );
        b.on.Store( false );
        ASSERT_FALSE( either.IsCancelRequested() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CVfsAsyncCallGate: counts the calls inside, a shut door refuses new ones and asks the ones inside to stop, closing waits for them to leave and gives up after its time" )
    try
    {
        VFS::CVfsAsyncCallGatePtr gate( GUCEF_NEW VFS::CVfsAsyncCallGate() );
        ASSERT_FALSE( gate->IsClosed() || gate->IsCancelRequested() );
        ASSERT_TRUE( gate->TryEnter() && gate->TryEnter() );
        ASSERT_TRUE( 2 == gate->GetInside() );
        gate->Leave();
        ASSERT_TRUE( 1 == gate->GetInside() );

        // a call is inside: closing waits the whole time and says so
        ASSERT_FALSE( gate->CloseAndWait( 30 ) );
        ASSERT_TRUE( gate->IsClosed() && gate->IsCancelRequested() );
        ASSERT_FALSE( gate->TryEnter() );
        ASSERT_TRUE( 1 == gate->GetInside() );

        // the call leaves: closing again is at once
        gate->Leave();
        ASSERT_TRUE( 0 == gate->GetInside() );
        ASSERT_TRUE( gate->CloseAndWait( 0 ) );
        gate->Leave();                                                  // more leaves than enters do not go below zero
        ASSERT_TRUE( 0 == gate->GetInside() );

        // the scope
        VFS::CVfsAsyncCallGatePtr second( GUCEF_NEW VFS::CVfsAsyncCallGate() );
        {
            VFS::CVfsAsyncCallGateScope scope( second );
            ASSERT_TRUE( scope.IsEntered() && 1 == second->GetInside() );
        }
        ASSERT_TRUE( 0 == second->GetInside() );
        ASSERT_TRUE( second->CloseAndWait( 0 ) );
        {
            VFS::CVfsAsyncCallGateScope scope( second );
            ASSERT_FALSE( scope.IsEntered() );
            ASSERT_TRUE( 0 == second->GetInside() );
        }
        {
            VFS::CVfsAsyncCallGatePtr noGate;
            VFS::CVfsAsyncCallGateScope scope( noGate );
            ASSERT_FALSE( scope.IsEntered() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CVfsAsyncCallGate: closing waits for a call that is inside on another thread and then no call is inside a shut door, with eight threads walking through" )
    try
    {
        VFS::CVfsAsyncCallGatePtr gate( GUCEF_NEW VFS::CVfsAsyncCallGate() );
        CGateWalkers walkers( gate );
        CTestThreads threads( walkers, 8 );
        ASSERT_TRUE( threads.Start() );
        MT::ThreadDelay( 5 );
        ASSERT_TRUE( gate->CloseAndWait( 10000 ) );
        ASSERT_TRUE( 0 == gate->GetInside() );
        ASSERT_TRUE( threads.Join( 20000 ) );
        ASSERT_TRUE( 0 == walkers.insideAfterClose.Load() );
        ASSERT_TRUE( 0 == gate->GetInside() );
        ASSERT_TRUE( walkers.entered.Load() + walkers.refused.Load() == 8 * 3000 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CVfsPrefetchWork: every file is taken exactly once in order of the list, also by eight threads at once; the outcomes are counted, a cancel is not a failure, the helpers are counted" )
    try
    {
        CORE::CString::StringVector names;
        for ( UInt32 i=0; i<20000; ++i )
            names.push_back( "file" + CORE::ToString( i ) );

        VFS::CVfsPrefetchWorkPtr ordered( GUCEF_NEW VFS::CVfsPrefetchWork( names ) );
        CORE::CString file;
        ASSERT_TRUE( ordered->TakeNext( file ) && "file0" == file );
        ASSERT_TRUE( ordered->TakeNext( file ) && "file1" == file );
        ASSERT_TRUE( 2 == ordered->GetFilesTaken() );

        VFS::CVfsPrefetchWorkPtr shared( GUCEF_NEW VFS::CVfsPrefetchWork( names ) );
        CWorkTakers takers( shared );
        CTestThreads threads( takers, 8 );
        ASSERT_TRUE( threads.Start() );
        ASSERT_TRUE( threads.Join( 20000 ) );
        ASSERT_TRUE( 20000 == takers.total.Load() && 20000 == takers.seen.size() && 20000 == shared->GetFilesTaken() );
        bool eachOnce = true;
        for ( GUCEF::map< CORE::CString, UInt32 >::const_iterator i = takers.seen.begin(); i != takers.seen.end(); ++i )
            eachOnce = eachOnce && 1 == i->second;
        ASSERT_TRUE( eachOnce );
        ASSERT_FALSE( shared->TakeNext( file ) );

        VFS::CVfsPrefetchWork outcomes( names );
        ASSERT_TRUE( outcomes.GetFirstFailure().IsSuccess() && !outcomes.HasCancelled() );
        outcomes.NoteEnded( VFS::CVfsCallResult() );
        outcomes.NoteEnded( VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_CANCELLED, "stopped" ) );
        ASSERT_TRUE( outcomes.HasCancelled() && 1 == outcomes.GetFilesSucceeded() && 1 == outcomes.GetFilesFailed() );
        ASSERT_TRUE( outcomes.GetFirstFailure().IsSuccess() );          // a cancel is not the failure that is reported
        outcomes.NoteEnded( VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_NOT_FOUND, "first" ) );
        outcomes.NoteEnded( VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "second" ) );
        ASSERT_TRUE( 3 == outcomes.GetFilesFailed() && CORE::FILESYSTEMACCESS_NOT_FOUND == outcomes.GetFirstFailure().result );

        ASSERT_TRUE( 0 == outcomes.GetHelpersRunning() );
        outcomes.HelperStarted();
        outcomes.HelperStarted();
        ASSERT_TRUE( 2 == outcomes.GetHelpersRunning() );
        outcomes.HelperEnded();
        outcomes.HelperEnded();
        outcomes.HelperEnded();
        ASSERT_TRUE( 0 == outcomes.GetHelpersRunning() );

        const CORE::CString::StringVector noFiles;
        VFS::CVfsPrefetchWork empty( noFiles );
        ASSERT_FALSE( empty.TakeNext( file ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CVfsAsyncCallTaskConsumer: a call runs once on a thread of the pool and not on the caller, the future ends after it, and the pool is made with workers when it had none" )
    try
    {
        CTestAsyncCallPtr call( GUCEF_NEW CTestAsyncCall( false, false ) );
        const CORE::CString poolName = "gucefvfs-async-prim-1";
        CORE::CFutureResult future = VFS::CVfsAsyncCallTaskConsumer::Queue( poolName, 2, call );
        REQUIRE( future.HasAFuture() );
        ASSERT_TRUE( WaitReady( future, 5000 ) );
        ASSERT_TRUE( CORE::TASKSTATUS_FINISHED == StatusOf( future ) );
        ASSERT_TRUE( 1 == call->started.Load() && 1 == call->finished.Load() );
        ASSERT_TRUE( call->threadId.Load() != MT::GetCurrentThreadID() );

        CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( poolName );
        ASSERT_TRUE( !pool.IsNULL() && 2 == pool->GetDesiredMinNrOfWorkerThreads() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CVfsAsyncCallTaskConsumer: a call that throws ends its task and the pool carries on with the next call; no call and a pool that takes no more work give a future without a task" )
    try
    {
        const CORE::CString poolName = "gucefvfs-async-prim-2";
        CTestAsyncCallPtr thrower( GUCEF_NEW CTestAsyncCall( false, true ) );
        CORE::CFutureResult first = VFS::CVfsAsyncCallTaskConsumer::Queue( poolName, 1, thrower );
        REQUIRE( first.HasAFuture() );
        ASSERT_TRUE( WaitReady( first, 5000 ) );
        ASSERT_TRUE( 1 == thrower->started.Load() );

        CTestAsyncCallPtr next( GUCEF_NEW CTestAsyncCall( false, false ) );
        CORE::CFutureResult second = VFS::CVfsAsyncCallTaskConsumer::Queue( poolName, 1, next );
        REQUIRE( second.HasAFuture() );
        ASSERT_TRUE( WaitReady( second, 5000 ) );
        ASSERT_TRUE( 1 == next->finished.Load() );

        ASSERT_TRUE( VFS::CVfsAsyncCallTaskConsumer::Queue( poolName, 1, VFS::CIVfsAsyncCallPtr() ).HasNoFuture() );

        // a pool that stops taking work refuses: no future
        const CORE::CString refusing = "gucefvfs-async-prim-refuse";
        CORE::ThreadPoolPtr pool = MakePool( refusing, 1 );
        REQUIRE( !pool.IsNULL() );
        CTestAsyncCallPtr before( GUCEF_NEW CTestAsyncCall( false, false ) );
        CORE::CFutureResult accepted = VFS::CVfsAsyncCallTaskConsumer::Queue( refusing, 1, before );
        ASSERT_TRUE( accepted.HasAFuture() && WaitReady( accepted, 5000 ) );
        pool->RequestAllThreadsToStop( true, false, 10000 );
        CTestAsyncCallPtr after( GUCEF_NEW CTestAsyncCall( false, false ) );
        CORE::CFutureResult refused = VFS::CVfsAsyncCallTaskConsumer::Queue( refusing, 1, after );
        ASSERT_TRUE( refused.HasNoFuture() );
        MT::ThreadDelay( 20 );
        ASSERT_TRUE( 0 == after->started.Load() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CVfsAsyncCallTaskConsumer: a cancel before a thread takes the call means it never runs; a cancel while it runs reaches it through its source; a cancel after it ended changes nothing" )
    try
    {
        const CORE::CString poolName = "gucefvfs-async-prim-3";
        CORE::ThreadPoolPtr pool = MakePool( poolName, 1 );
        REQUIRE( !pool.IsNULL() );

        // the single thread of the pool is busy with the first call, the second waits in the queue
        CTestAsyncCallPtr busy( GUCEF_NEW CTestAsyncCall( true, false ) );
        CORE::CFutureResult busyFuture = VFS::CVfsAsyncCallTaskConsumer::Queue( poolName, 1, busy );
        REQUIRE( busyFuture.HasAFuture() );
        REQUIRE( WaitFor( busy->started, 1, 5000 ) );

        CTestAsyncCallPtr queued( GUCEF_NEW CTestAsyncCall( false, false ) );
        CORE::CFutureResult queuedFuture = VFS::CVfsAsyncCallTaskConsumer::Queue( poolName, 1, queued );
        REQUIRE( queuedFuture.HasAFuture() );
        ASSERT_FALSE( queuedFuture.IsReady() );
        ASSERT_TRUE( queuedFuture.RequestCancellation() );
        ASSERT_TRUE( WaitReady( queuedFuture, 5000 ) );
        ASSERT_TRUE( CORE::TASKSTATUS_CANCELLED == StatusOf( queuedFuture ) );

        // while it runs: the source says so, the call stops
        ASSERT_FALSE( busy->sawCancel.Load() );
        ASSERT_TRUE( busyFuture.RequestCancellation() );
        ASSERT_TRUE( WaitReady( busyFuture, 5000 ) );
        ASSERT_TRUE( busy->sawCancel.Load() && 1 == busy->finished.Load() );
        ASSERT_TRUE( 0 == queued->started.Load() );

        // after it ended
        CTestAsyncCallPtr done( GUCEF_NEW CTestAsyncCall( false, false ) );
        CORE::CFutureResult doneFuture = VFS::CVfsAsyncCallTaskConsumer::Queue( poolName, 1, done );
        ASSERT_TRUE( WaitReady( doneFuture, 5000 ) );
        const CORE::TTaskStatus statusBefore = StatusOf( doneFuture );
        doneFuture.RequestCancellation();
        ASSERT_TRUE( statusBefore == StatusOf( doneFuture ) && CORE::TASKSTATUS_FINISHED == statusBefore );
        ASSERT_FALSE( done->sawCancel.Load() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CVfsAsyncCallTaskConsumer: a thousand calls queued from four threads on a pool of three workers all run exactly once, on pool threads only" )
    try
    {
        class CQueuers : public CTestThreadBody
        {
            public:
            GUCEF::vector< CTestAsyncCallPtr > calls;
            GUCEF::vector< CORE::CFutureResult > futures;
            MT::CMutex lock;
            MT::CTAtomic< UInt32 > refused;
            CQueuers( void ) : calls(), futures(), lock(), refused( 0 ) {}
            virtual void Run( UInt32 /* index */ ) GUCEF_VIRTUAL_OVERRIDE
            {
                for ( UInt32 i=0; i<250; ++i )
                {
                    CTestAsyncCallPtr call( GUCEF_NEW CTestAsyncCall( false, false ) );
                    CORE::CFutureResult future = VFS::CVfsAsyncCallTaskConsumer::Queue( "gucefvfs-async-prim-4", 3, call );
                    MT::CScopeMutex scope( lock );
                    calls.push_back( call );
                    futures.push_back( future );
                    if ( future.HasNoFuture() )
                        refused.FetchAdd( 1 );
                }
            }
        };

        CQueuers queuers;
        CTestThreads threads( queuers, 4 );
        ASSERT_TRUE( threads.Start() );
        ASSERT_TRUE( threads.Join( 30000 ) );
        ASSERT_TRUE( 1000 == queuers.calls.size() && 0 == queuers.refused.Load() );
        UInt32 wrong = 0;
        for ( size_t i=0; i<queuers.futures.size(); ++i )
        {
            if ( !WaitReady( queuers.futures[ i ], 30000 ) )
            {
                ++wrong;
                continue;
            }
            if ( 1 != queuers.calls[ i ]->started.Load() || 1 != queuers.calls[ i ]->finished.Load() )
                ++wrong;
            if ( queuers.calls[ i ]->threadId.Load() == MT::GetCurrentThreadID() )
                ++wrong;
            for ( UInt32 t=0; t<4; ++t )
                if ( queuers.calls[ i ]->threadId.Load() == threads.GetThreadId( t ) )
                    ++wrong;
        }
        ASSERT_TRUE( 0 == wrong );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}
