/*
 *  gucefMT_TestApp: GUCEF test application for the MT module
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

#ifndef GUCEF_MT_CTSPSCQUEUE_H
#include "gucefMT_CTSpscQueue.h"
#define GUCEF_MT_CTSPSCQUEUE_H
#endif /* GUCEF_MT_CTSPSCQUEUE_H ? */

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

#include "TestSpscQueue.h"

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

struct SItem
{
    UInt32 sequence;
    UInt32 check;
    SItem( void ) : sequence( 0 ), check( 0 ) {}
    SItem( UInt32 s ) : sequence( s ), check( s ^ 0xA5A5A5A5U ) {}
};

/** Runs a function on its own thread */
class CQueueTestThread : public MT::CActiveObject
{
    public:

    typedef void ( *TFunction )( void* argument );

    CQueueTestThread( TFunction function, void* argument ) : MT::CActiveObject(), m_function( function ), m_argument( argument ), m_done( false ) {}

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_function( m_argument );
        m_done.Store( true, MT::MEMORDER_RELEASE );
        return true;
    }
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

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

const UInt32 TransferCount = 2000000;

struct STransfer
{
    MT::CTSpscQueue< SItem > queue;
    UInt32 received;
    UInt32 outOfOrder;
    UInt32 corrupted;
    STransfer( void ) : queue( 64 ), received( 0 ), outOfOrder( 0 ), corrupted( 0 ) {}
};

void
Produce( void* argument )
{
    STransfer& t = *static_cast< STransfer* >( argument );
    for ( UInt32 i=0; i<TransferCount; ++i )
    {
        const SItem item( i );
        UInt32 spins = 0;
        while ( !t.queue.Push( item ) && spins < 400000000 )
        {
            MT::AtomicCpuRelax();
            ++spins;
        }
    }
}

void
Consume( void* argument )
{
    STransfer& t = *static_cast< STransfer* >( argument );
    UInt32 expected = 0;
    UInt32 spins = 0;
    while ( expected < TransferCount && spins < 400000000 )
    {
        SItem item;
        if ( t.queue.Pop( item ) )
        {
            if ( item.sequence != expected ) ++t.outOfOrder;
            if ( item.check != ( item.sequence ^ 0xA5A5A5A5U ) ) ++t.corrupted;
            ++expected;
            ++t.received;
            spins = 0;
        }
        else
        {
            MT::AtomicCpuRelax();
            ++spins;
        }
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSpscQueueTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING SpscQueue TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefMT.SpscQueue" );

    GUCEF_TESTFW_TESTCASE( "SpscQueue: Capacity is rounded up to a power of two, new queues are empty" )
    try
    {
        ASSERT_TRUE( 2 == MT::CTSpscQueue< UInt32 >( 0 ).GetCapacity() );
        ASSERT_TRUE( 2 == MT::CTSpscQueue< UInt32 >( 1 ).GetCapacity() );
        ASSERT_TRUE( 4 == MT::CTSpscQueue< UInt32 >( 3 ).GetCapacity() );
        ASSERT_TRUE( 8 == MT::CTSpscQueue< UInt32 >( 8 ).GetCapacity() );
        ASSERT_TRUE( 1024 == MT::CTSpscQueue< UInt32 >( 1000 ).GetCapacity() );
        MT::CTSpscQueue< UInt32 > q( 8 );
        ASSERT_TRUE( q.IsEmpty() && 0 == q.GetSize() );
        UInt32 v = 77;
        ASSERT_FALSE( q.Pop( v ) );
        ASSERT_TRUE( 77 == v );                                  // a failed pop leaves the item alone
        ASSERT_FALSE( q.Peek( v ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SpscQueue: Items come out in the order they went in, full and empty are reported" )
    try
    {
        MT::CTSpscQueue< UInt32 > q( 8 );
        for ( UInt32 i=0; i<8; ++i )
            ASSERT_TRUE( q.Push( 100 + i ) );
        ASSERT_FALSE( q.Push( 999 ) );                           // full: all 8 places are used
        ASSERT_TRUE( 8 == q.GetSize() && !q.IsEmpty() );

        UInt32 v = 0;
        ASSERT_TRUE( q.Peek( v ) && 100 == v );
        ASSERT_TRUE( 8 == q.GetSize() );                         // peeking takes nothing
        for ( UInt32 i=0; i<8; ++i )
        {
            ASSERT_TRUE( q.Pop( v ) );
            ASSERT_TRUE( 100 + i == v );
        }
        ASSERT_FALSE( q.Pop( v ) );
        ASSERT_TRUE( q.IsEmpty() );
        ASSERT_TRUE( q.Push( 5 ) && q.Pop( v ) && 5 == v );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SpscQueue: Wrapping around the end of the storage many times, also past the point the counters wrap" )
    try
    {
        MT::CTSpscQueue< UInt32 > q( 4 );
        UInt32 next = 0, expected = 0, v = 0;
        for ( UInt32 round=0; round<300000; ++round )
        {
            // push 1 to 3 items, pop what is there
            const UInt32 pushes = 1 + round % 3;
            for ( UInt32 p=0; p<pushes; ++p )
                if ( q.Push( next ) )
                    ++next;
            const UInt32 pops = 1 + ( round * 7 ) % 3;
            for ( UInt32 p=0; p<pops; ++p )
                if ( q.Pop( v ) )
                {
                    ASSERT_TRUE( expected == v );
                    ++expected;
                }
        }
        while ( q.Pop( v ) )
        {
            ASSERT_TRUE( expected == v );
            ++expected;
        }
        ASSERT_TRUE( next == expected && next > 100000 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SpscQueue: Structures go through intact" )
    try
    {
        MT::CTSpscQueue< SItem > q( 16 );
        for ( UInt32 i=0; i<10; ++i )
            ASSERT_TRUE( q.Push( SItem( i * 1000 ) ) );
        SItem item;
        for ( UInt32 i=0; i<10; ++i )
        {
            ASSERT_TRUE( q.Pop( item ) );
            ASSERT_TRUE( i * 1000 == item.sequence && item.check == ( item.sequence ^ 0xA5A5A5A5U ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SpscQueue: A producer and a consumer on separate threads lose nothing and keep the order" )
    try
    {
        STransfer transfer;
        CQueueTestThread producer( &Produce, &transfer );
        CQueueTestThread consumer( &Consume, &transfer );
        ASSERT_TRUE( consumer.Activate( GUCEF_NULL, 0, 0 ) );
        ASSERT_TRUE( producer.Activate( GUCEF_NULL, 0, 0 ) );
        ASSERT_TRUE( producer.WaitUntilDone( 120000 ) );
        ASSERT_TRUE( consumer.WaitUntilDone( 120000 ) );
        ASSERT_TRUE( TransferCount == transfer.received );
        ASSERT_TRUE( 0 == transfer.outOfOrder );
        ASSERT_TRUE( 0 == transfer.corrupted );
        ASSERT_TRUE( transfer.queue.IsEmpty() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
