/*
 *  gucefMT_TestApp: GUCEF test application for the MT module
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

#ifndef GUCEF_MT_CSEMAPHORE_H
#include "gucefMT_CSemaphore.h"
#define GUCEF_MT_CSEMAPHORE_H
#endif /* GUCEF_MT_CSEMAPHORE_H ? */

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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestSemaphore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Runs a function on its own thread */
class CSemaphoreTestThread : public MT::CActiveObject
{
    public:

    typedef void ( *TFunction )( void* argument );

    CSemaphoreTestThread( TFunction function, void* argument ) : MT::CActiveObject(), m_function( function ), m_argument( argument ), m_done( false ) {}

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_function( m_argument );
        m_done.Store( true, MT::MEMORDER_RELEASE );
        return true;
    }
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool IsDone( void ) const { return m_done.Load( MT::MEMORDER_ACQUIRE ); }

    bool WaitUntilDone( UInt32 timeoutInMs )
    {
        UInt32 waited = 0;
        while ( ( !m_done.Load( MT::MEMORDER_ACQUIRE ) || IsActive() ) && waited < timeoutInMs )
        {
            MT::PrecisionDelay( 1 );
            ++waited;
        }
        return m_done.Load( MT::MEMORDER_ACQUIRE ) && !IsActive();
    }

    private:

    TFunction m_function;
    void* m_argument;
    MT::CTAtomic< bool > m_done;
};

/*-------------------------------------------------------------------------*/

struct SWakeTest
{
    MT::CSemaphore semaphore;
    MT::CTAtomic< UInt32 > started;
    MT::CTAtomic< UInt32 > waitResult;
    MT::CTAtomic< UInt64 > wokeAtTicks;
    UInt32 timeoutInMs;
    SWakeTest( UInt32 timeout ) : semaphore( 0 ), started( 0 ), waitResult( 99 ), wokeAtTicks( 0 ), timeoutInMs( timeout ) {}
};

void
WaitForPost( void* argument )
{
    SWakeTest& t = *static_cast< SWakeTest* >( argument );
    t.started.Store( 1, MT::MEMORDER_SEQ_CST );
    const Int32 result = t.semaphore.WaitOrTimeout( t.timeoutInMs );
    t.wokeAtTicks.Store( MT::PrecisionTickCount(), MT::MEMORDER_SEQ_CST );
    t.waitResult.Store( (UInt32) result, MT::MEMORDER_SEQ_CST );
}

/*-------------------------------------------------------------------------*/

const UInt32 PostersAndConsumers = 2;
const UInt32 PostsPerPoster = 100000;
const UInt32 ManyPosts = 100000;

struct SProducerConsumer
{
    MT::CSemaphore semaphore;
    MT::CTAtomic< UInt32 > posted;
    MT::CTAtomic< UInt32 > consumed;
    MT::CTAtomic< UInt32 > failedWaits;
    SProducerConsumer( void ) : semaphore( 0 ), posted( 0 ), consumed( 0 ), failedWaits( 0 ) {}
};

void
Poster( void* argument )
{
    SProducerConsumer& t = *static_cast< SProducerConsumer* >( argument );
    for ( UInt32 i=0; i<PostsPerPoster; ++i )
    {
        if ( t.semaphore.Post() )
            ++t.posted;
        if ( 0 == ( i & 1023 ) )
            MT::PrecisionDelay( 0 );        // lets consumers run dry now and then, so they really sleep
    }
}

void
Consumer( void* argument )
{
    SProducerConsumer& t = *static_cast< SProducerConsumer* >( argument );
    for ( UInt32 i=0; i<PostsPerPoster; ++i )    // as many consumers as posters, so each takes what one poster posts
    {
        if ( 0 == t.semaphore.WaitOrTimeout( 20000 ) )
            ++t.consumed;
        else
            ++t.failedWaits;
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSemaphoreTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Semaphore TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefMT.Semaphore" );

    GUCEF_TESTFW_TESTCASE( "Semaphore: Single thread: initial count, post and wait count" )
    try
    {
        MT::CSemaphore s( 2 );
        ASSERT_TRUE( 2 == s.Value() );
        ASSERT_TRUE( 0 == s.TryWait() );
        ASSERT_TRUE( 1 == s.Value() );
        ASSERT_TRUE( s.Wait() );
        ASSERT_TRUE( 0 == s.Value() );
        ASSERT_TRUE( 1 == s.TryWait() );                         // would block
        ASSERT_TRUE( 0 == s.Value() );
        ASSERT_TRUE( s.Post() && s.Post() && s.Post() );
        ASSERT_TRUE( 3 == s.Value() );
        ASSERT_TRUE( s.Wait() && s.Wait() );
        ASSERT_TRUE( 1 == s.Value() );
        ASSERT_TRUE( 0 == s.WaitOrTimeout( 0 ) );                // zero timeout takes what is there
        ASSERT_TRUE( 1 == s.WaitOrTimeout( 0 ) );
        ASSERT_TRUE( s.Post() && 0 == s.WaitOrTimeout( 10 ) && 0 == s.Value() );
        ASSERT_TRUE( s.Post() && 0 == s.WaitOrTimeout( GUCEF_MT_INFINITE_LOCK_TIMEOUT ) && 0 == s.Value() );     // count there: returns at once

        MT::CSemaphore zero( 0 );
        ASSERT_TRUE( 0 == zero.Value() && 1 == zero.TryWait() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Semaphore: Timeout: returns 1 after about the requested time, and does not take a count it did not get" )
    try
    {
        MT::CSemaphore s( 0 );
        UInt64 start = MT::PrecisionTickCount();
        ASSERT_TRUE( 1 == s.WaitOrTimeout( 0 ) );
        ASSERT_TRUE( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < 50 );

        start = MT::PrecisionTickCount();
        ASSERT_TRUE( 1 == s.WaitOrTimeout( 100 ) );
        Float64 waited = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
        ASSERT_TRUE( waited >= 95 && waited < 2000 );

        start = MT::PrecisionTickCount();
        ASSERT_TRUE( 1 == s.WaitOrTimeout( 1 ) );
        waited = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
        ASSERT_TRUE( waited >= 0.5 && waited < 2000 );

        ASSERT_TRUE( 0 == s.Value() );
        ASSERT_TRUE( s.Post() && 1 == s.Value() );     // still usable after timing out
        ASSERT_TRUE( s.Wait() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Semaphore: Cross thread: a blocked Wait is woken by Post, a timed wait is woken early" )
    try
    {
        {
            SWakeTest t( GUCEF_MT_INFINITE_LOCK_TIMEOUT );
            CSemaphoreTestThread waiter( &WaitForPost, &t );
            ASSERT_TRUE( waiter.Activate( GUCEF_NULL, 0, 0 ) );
            UInt32 guard = 0;
            while ( 0 == t.started.Load( MT::MEMORDER_SEQ_CST ) && guard++ < 5000 )
                MT::PrecisionDelay( 1 );
            MT::PrecisionDelay( 100 );
            ASSERT_FALSE( waiter.IsDone() );                     // still blocked
            const UInt64 postedAt = MT::PrecisionTickCount();
            ASSERT_TRUE( t.semaphore.Post() );
            ASSERT_TRUE( waiter.WaitUntilDone( 20000 ) );
            ASSERT_TRUE( 0 == t.waitResult.Load( MT::MEMORDER_SEQ_CST ) );
            ASSERT_TRUE( MT::PrecisionTimerTicksToMs( t.wokeAtTicks.Load( MT::MEMORDER_SEQ_CST ) - postedAt ) < 1000 );
            ASSERT_TRUE( 0 == t.semaphore.Value() );             // the waiter took the count
        }
        {
            SWakeTest t( 60000 );
            CSemaphoreTestThread waiter( &WaitForPost, &t );
            ASSERT_TRUE( waiter.Activate( GUCEF_NULL, 0, 0 ) );
            UInt32 guard = 0;
            while ( 0 == t.started.Load( MT::MEMORDER_SEQ_CST ) && guard++ < 5000 )
                MT::PrecisionDelay( 1 );
            MT::PrecisionDelay( 100 );
            ASSERT_TRUE( t.semaphore.Post() );
            ASSERT_TRUE( waiter.WaitUntilDone( 20000 ) );        // well before its 60 s timeout
            ASSERT_TRUE( 0 == t.waitResult.Load( MT::MEMORDER_SEQ_CST ) );
        }
        {
            SWakeTest t( 80 );                                   // nobody posts: the waiter ends on its timeout
            CSemaphoreTestThread waiter( &WaitForPost, &t );
            ASSERT_TRUE( waiter.Activate( GUCEF_NULL, 0, 0 ) );
            ASSERT_TRUE( waiter.WaitUntilDone( 20000 ) );
            ASSERT_TRUE( 1 == t.waitResult.Load( MT::MEMORDER_SEQ_CST ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Semaphore: Stress: producers and consumers, every post is consumed exactly once" )
    try
    {
        SProducerConsumer t;
        CSemaphoreTestThread* threads[ PostersAndConsumers * 2 ];
        for ( UInt32 i=0; i<PostersAndConsumers; ++i )
        {
            threads[ i ] = new CSemaphoreTestThread( &Consumer, &t );
            threads[ PostersAndConsumers + i ] = new CSemaphoreTestThread( &Poster, &t );
        }
        for ( UInt32 i=0; i<PostersAndConsumers * 2; ++i )
            ASSERT_TRUE( threads[ i ]->Activate( GUCEF_NULL, 0, 0 ) );
        bool allDone = true;
        for ( UInt32 i=0; i<PostersAndConsumers * 2; ++i )
            allDone = threads[ i ]->WaitUntilDone( 60000 ) && allDone;
        for ( UInt32 i=0; i<PostersAndConsumers * 2; ++i )
            delete threads[ i ];
        ASSERT_TRUE( allDone );
        ASSERT_TRUE( PostersAndConsumers * PostsPerPoster == t.posted.Load() );
        ASSERT_TRUE( PostersAndConsumers * PostsPerPoster == t.consumed.Load() );
        ASSERT_TRUE( 0 == t.failedWaits.Load() );
        ASSERT_TRUE( 0 == t.semaphore.Value() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Semaphore: Many posts followed by many waits" )
    try
    {
        MT::CSemaphore s( 0 );
        for ( UInt32 i=0; i<ManyPosts; ++i )
            ASSERT_TRUE( s.Post() );
        ASSERT_TRUE( ManyPosts == s.Value() );
        for ( UInt32 i=0; i<ManyPosts / 2; ++i )
            ASSERT_TRUE( s.Wait() );
        ASSERT_TRUE( ManyPosts / 2 == s.Value() );
        for ( UInt32 i=0; i<ManyPosts / 2; ++i )
            ASSERT_TRUE( 0 == s.TryWait() );
        ASSERT_TRUE( 0 == s.Value() && 1 == s.TryWait() && 1 == s.WaitOrTimeout( 5 ) );

        // the same with the posts made by another thread and all of them in before the first wait
        SProducerConsumer t;
        CSemaphoreTestThread poster( &Poster, &t );
        ASSERT_TRUE( poster.Activate( GUCEF_NULL, 0, 0 ) );
        ASSERT_TRUE( poster.WaitUntilDone( 60000 ) );
        ASSERT_TRUE( PostsPerPoster == t.semaphore.Value() );
        for ( UInt32 i=0; i<PostsPerPoster; ++i )
            ASSERT_TRUE( 0 == t.semaphore.WaitOrTimeout( 1000 ) );
        ASSERT_TRUE( 0 == t.semaphore.Value() && 1 == t.semaphore.TryWait() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
