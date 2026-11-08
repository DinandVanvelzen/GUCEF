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

#ifndef GUCEF_MT_CTSLOTDATAEXCHANGE_H
#include "gucefMT_CTSlotDataExchange.h"
#define GUCEF_MT_CTSLOTDATAEXCHANGE_H
#endif /* GUCEF_MT_CTSLOTDATAEXCHANGE_H ? */

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

#include "TestSlotDataExchange.h"

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

typedef MT::CTSlotDataExchange< UInt32, MT::SSlotDeliveryAllInOrder, MT::SSlotFullFail >    TOrderedFail;
typedef MT::CTSlotDataExchange< UInt32, MT::SSlotDeliveryAllInOrder, MT::SSlotFullBlock >   TOrderedBlock;

/** Several fields that must always hold the same number, a torn read shows as a difference */
struct SPayload
{
    UInt64 a;
    UInt64 b;
    UInt64 c;
    UInt64 d;
    SPayload( void ) : a( 0 ), b( 0 ), c( 0 ), d( 0 ) {}
    SPayload( UInt64 n ) : a( n ), b( n ), c( n ), d( n ) {}
    bool IsConsistent( void ) const { return a == b && b == c && c == d; }
};

/** Runs a function on its own thread */
class CExchangeTestThread : public MT::CActiveObject
{
    public:

    typedef void ( *TFunction )( void* argument );

    CExchangeTestThread( TFunction function, void* argument ) : MT::CActiveObject(), m_function( function ), m_argument( argument ), m_done( false ) {}

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

// Reading through one name whatever the number of readers, for the stress code below
template< typename T, typename F >
bool
ReadLatest( MT::CTSlotDataExchange< T, MT::SSlotDeliveryLatestOnly, F, 1 >& exchange, T& out, UInt64& lastSeenVersion )
{
    return exchange.Read( out );
}

template< typename T, typename F, UInt32 N >
bool
ReadLatest( MT::CTSlotDataExchange< T, MT::SSlotDeliveryLatestOnly, F, N >& exchange, T& out, UInt64& lastSeenVersion )
{
    return exchange.Read( out, lastSeenVersion );
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MT_TESTAPP_SLOT_STRESS_MS
const UInt32 StressDurationMs = GUCEF_MT_TESTAPP_SLOT_STRESS_MS;
#else
const UInt32 StressDurationMs = 150;
#endif
const UInt32 OrderedTransferCount = 1000000;
const UInt32 OrderedBlockTransferCount = 100000;
const UInt32 SpinGuard = 400000000;
const Float64 WakeLatencyBoundInMs = 0.5;       // compared with the median of several runs; a polling wait would sit around 0.8 or more

/*-------------------------------------------------------------------------*/

struct SOrderedTransfer
{
    TOrderedFail failExchange;
    TOrderedBlock blockExchange;
    UInt32 count;
    UInt32 received;
    UInt32 outOfOrder;
    UInt32 slowReaderMask;      // when a bit of the received count (as a mask) is zero the reader sleeps a moment, to make the writer wait
    SOrderedTransfer( UInt32 slots, UInt32 transferCount, UInt32 mask )
        : failExchange( slots ), blockExchange( slots ), count( transferCount ), received( 0 ), outOfOrder( 0 ), slowReaderMask( mask ) {}
};

void
ProduceFail( void* argument )
{
    SOrderedTransfer& t = *static_cast< SOrderedTransfer* >( argument );
    for ( UInt32 i=0; i<t.count; ++i )
    {
        UInt32 spins = 0;
        while ( !t.failExchange.Write( i ) && spins < SpinGuard )
        {
            MT::AtomicCpuRelax();
            ++spins;
        }
    }
}

void
ConsumeFail( void* argument )
{
    SOrderedTransfer& t = *static_cast< SOrderedTransfer* >( argument );
    UInt32 expected = 0;
    UInt32 spins = 0;
    while ( expected < t.count && spins < SpinGuard )
    {
        UInt32 value = 0;
        if ( t.failExchange.Read( value ) )
        {
            if ( value != expected ) ++t.outOfOrder;
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

void
ProduceBlock( void* argument )
{
    SOrderedTransfer& t = *static_cast< SOrderedTransfer* >( argument );
    for ( UInt32 i=0; i<t.count; ++i )
    {
        if ( !t.blockExchange.Write( i, MT::SlotWaitForever ) )
            return;
    }
}

void
ConsumeBlock( void* argument )
{
    SOrderedTransfer& t = *static_cast< SOrderedTransfer* >( argument );
    UInt32 expected = 0;
    while ( expected < t.count )
    {
        UInt32 value = 0;
        if ( t.blockExchange.Read( value ) )
        {
            if ( value != expected ) ++t.outOfOrder;
            ++expected;
            ++t.received;
            if ( 0 != t.slowReaderMask && 0 == ( expected & t.slowReaderMask ) )
                MT::PrecisionDelay( 1 );        // lets the writer run into a full exchange
        }
        else
        if ( !t.blockExchange.WaitForData( MT::SlotWaitForever ) )
            return;
    }
}

/*-------------------------------------------------------------------------*/

template< UInt32 MaxReaders >
struct SLatestStress
{
    typedef MT::CTSlotDataExchange< SPayload, MT::SSlotDeliveryLatestOnly, MT::SSlotFullFail, MaxReaders > TExchange;

    struct SReaderResult
    {
        SLatestStress* owner;
        UInt64 lastSeenVersion;
        UInt64 lastValue;
        UInt64 newReads;
        UInt64 torn;
        UInt64 wentBackwards;
        UInt64 untouchedViolations;
        SReaderResult( void ) : owner( GUCEF_NULL ), lastSeenVersion( 0 ), lastValue( 0 ), newReads( 0 ), torn( 0 ), wentBackwards( 0 ), untouchedViolations( 0 ) {}
    };

    TExchange exchange;
    MT::CTAtomic< bool > writerDone;
    MT::CTAtomic< bool > stop;
    UInt32 writerPause;         // relax iterations between writes
    UInt32 readerPause;         // relax iterations between reads
    UInt64 written;
    SReaderResult readers[ MaxReaders ];

    SLatestStress( UInt32 wPause, UInt32 rPause ) : exchange(), writerDone( false ), stop( false ), writerPause( wPause ), readerPause( rPause ), written( 0 )
    {
        for ( UInt32 i=0; i<MaxReaders; ++i )
            readers[ i ].owner = this;
    }

    static void Write( void* argument )
    {
        SLatestStress& t = *static_cast< SLatestStress* >( argument );
        UInt64 n = 0;
        while ( !t.stop.Load( MT::MEMORDER_ACQUIRE ) )
        {
            t.exchange.Write( SPayload( ++n ) );
            for ( UInt32 i=0; i<t.writerPause; ++i )
                MT::AtomicCpuRelax();
        }
        t.written = n;
        t.writerDone.Store( true, MT::MEMORDER_RELEASE );
    }

    static void Read( void* argument )
    {
        SReaderResult& r = *static_cast< SReaderResult* >( argument );
        SLatestStress& t = *r.owner;
        bool lastPass = false;
        while ( !lastPass )
        {
            lastPass = t.writerDone.Load( MT::MEMORDER_ACQUIRE );        // after this, one more read must see the final value
            const SPayload sentinel( 0xFFFFFFFFFFFFFFFFULL );
            SPayload value( sentinel );
            const bool isNew = ReadLatest( t.exchange, value, r.lastSeenVersion );
            if ( isNew )
            {
                ++r.newReads;
                if ( !value.IsConsistent() ) ++r.torn;
                if ( value.a <= r.lastValue ) ++r.wentBackwards;
                r.lastValue = value.a;
            }
            else
            if ( !( value.a == sentinel.a && value.d == sentinel.d ) &&                     // false leaves 'out' alone, or
                 !( value.IsConsistent() && value.a == r.lastValue && 0 != r.lastValue ) )  // for one reader gives the value of the previous read again
                ++r.untouchedViolations;
            for ( UInt32 i=0; i<t.readerPause; ++i )
                MT::AtomicCpuRelax();
        }
    }
};

template< UInt32 MaxReaders >
bool
RunLatestStress( UInt32 writerPause, UInt32 readerPause )
{
    SLatestStress< MaxReaders > stress( writerPause, readerPause );
    CExchangeTestThread writer( &SLatestStress< MaxReaders >::Write, &stress );
    CExchangeTestThread* readers[ MaxReaders ];
    for ( UInt32 i=0; i<MaxReaders; ++i )
        readers[ i ] = new CExchangeTestThread( &SLatestStress< MaxReaders >::Read, &stress.readers[ i ] );

    bool ok = true;
    for ( UInt32 i=0; i<MaxReaders; ++i )
        ok = readers[ i ]->Activate( GUCEF_NULL, 0, 0 ) && ok;
    ok = writer.Activate( GUCEF_NULL, 0, 0 ) && ok;

    MT::PrecisionDelay( StressDurationMs );
    stress.stop.Store( true, MT::MEMORDER_RELEASE );

    ok = writer.WaitUntilDone( 120000 ) && ok;
    for ( UInt32 i=0; i<MaxReaders; ++i )
        ok = readers[ i ]->WaitUntilDone( 120000 ) && ok;

    if ( ok )
    {
        ok = stress.written > 0;
        for ( UInt32 i=0; i<MaxReaders; ++i )
        {
            const typename SLatestStress< MaxReaders >::SReaderResult& r = stress.readers[ i ];
            ok = ok && 0 == r.torn && 0 == r.wentBackwards && 0 == r.untouchedViolations;
            ok = ok && r.newReads > 0 && r.lastValue == stress.written;     // every reader ends on the final value
        }
    }

    for ( UInt32 i=0; i<MaxReaders; ++i )
        delete readers[ i ];
    return ok;
}


/*-------------------------------------------------------------------------*/

/** Measures how long a sleeper takes to wake up after the other side acted */
struct SWakeLatency
{
    TOrderedBlock exchange;
    MT::CTAtomic< UInt32 > started;         // the sleeper is about to wait
    MT::CTAtomic< UInt64 > wokeAtTicks;     // set by the sleeper as soon as its wait returned
    SWakeLatency( UInt32 slots ) : exchange( slots ), started( 0 ), wokeAtTicks( 0 ) {}
};

void
SleepInWaitForData( void* argument )
{
    SWakeLatency& t = *static_cast< SWakeLatency* >( argument );
    t.started.Store( 1, MT::MEMORDER_SEQ_CST );
    if ( t.exchange.WaitForData( MT::SlotWaitForever ) )
        t.wokeAtTicks.Store( MT::PrecisionTickCount(), MT::MEMORDER_SEQ_CST );
}

void
SleepInWrite( void* argument )
{
    SWakeLatency& t = *static_cast< SWakeLatency* >( argument );
    t.started.Store( 1, MT::MEMORDER_SEQ_CST );
    if ( t.exchange.Write( 99, MT::SlotWaitForever ) )
        t.wokeAtTicks.Store( MT::PrecisionTickCount(), MT::MEMORDER_SEQ_CST );
}

/**
 *  One run: the sleeper blocks (given long enough to be past its spin phase), the main thread acts at a
 *  measured moment, and the time until the sleeper's wait returned is the result. Negative on failure
 */
Float64
MeasureWakeLatencyInMs( bool sleeperIsReader )
{
    SWakeLatency t( 2 );
    if ( !sleeperIsReader )
    {
        if ( !t.exchange.Write( 1, 0 ) || !t.exchange.Write( 2, 0 ) )       // full, so the writer has to wait
            return -1;
    }

    CExchangeTestThread sleeper( sleeperIsReader ? &SleepInWaitForData : &SleepInWrite, &t );
    if ( !sleeper.Activate( GUCEF_NULL, 0, 0 ) )
        return -1;
    UInt32 waited = 0;
    while ( 0 == t.started.Load( MT::MEMORDER_SEQ_CST ) && waited++ < 5000 )
        MT::PrecisionDelay( 1 );
    MT::PrecisionDelay( 30 );

    const UInt64 actedAtTicks = MT::PrecisionTickCount();
    UInt32 v = 0;
    const bool acted = sleeperIsReader ? t.exchange.Write( 7, 0 ) : t.exchange.Read( v );
    if ( !acted || !sleeper.WaitUntilDone( 20000 ) )
        return -1;
    const UInt64 wokeAtTicks = t.wokeAtTicks.Load( MT::MEMORDER_SEQ_CST );
    if ( 0 == wokeAtTicks || wokeAtTicks < actedAtTicks )
        return -1;
    return MT::PrecisionTimerTicksToMs( wokeAtTicks - actedAtTicks );
}

Float64
MedianWakeLatencyInMs( bool sleeperIsReader, UInt32 runs )
{
    Float64 samples[ 64 ];
    if ( runs > 64 )
        runs = 64;
    for ( UInt32 i=0; i<runs; ++i )
    {
        samples[ i ] = MeasureWakeLatencyInMs( sleeperIsReader );
        if ( samples[ i ] < 0 )
            return -1;
        for ( UInt32 j=i; j>0 && samples[ j ] < samples[ j-1 ]; --j )     // insertion sort
        {
            const Float64 swap = samples[ j ];
            samples[ j ] = samples[ j-1 ];
            samples[ j-1 ] = swap;
        }
    }
    return samples[ runs / 2 ];
}
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSlotDataExchangeTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING SlotDataExchange TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefMT.SlotDataExchange" );

    GUCEF_TESTFW_TESTCASE( "SlotDataExchange: In order delivery, fail when full: order, full, empty, slot counts" )
    try
    {
        TOrderedFail x( 5 );
        ASSERT_TRUE( 8 == x.GetSlotCount() && 8 == x.GetCapacity() );
        ASSERT_TRUE( x.IsEmpty() && !x.HasData() && 0 == x.GetSize() );
        UInt32 v = 77;
        ASSERT_FALSE( x.Read( v ) );
        ASSERT_TRUE( 77 == v );                                  // a failed read leaves the value alone
        for ( UInt32 i=0; i<8; ++i )
            ASSERT_TRUE( x.Write( 100 + i ) );
        ASSERT_FALSE( x.Write( 999 ) );                          // full, and the value was not stored
        ASSERT_TRUE( x.HasData() && !x.IsEmpty() && 8 == x.GetSize() );
        for ( UInt32 i=0; i<8; ++i )
        {
            ASSERT_TRUE( x.Read( v ) );
            ASSERT_TRUE( 100 + i == v );
        }
        ASSERT_FALSE( x.Read( v ) );
        ASSERT_TRUE( x.IsEmpty() );
        ASSERT_TRUE( x.Write( 5 ) && x.Read( v ) && 5 == v );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SlotDataExchange: In order delivery, block when full: single thread behaviour and timeouts" )
    try
    {
        TOrderedBlock x( 4 );
        ASSERT_TRUE( 4 == x.GetSlotCount() && 4 == x.GetCapacity() && x.IsEmpty() );
        UInt32 v = 77;
        ASSERT_FALSE( x.Read( v ) && 77 != v );
        ASSERT_FALSE( x.WaitForData( 0 ) );
        UInt64 start = MT::PrecisionTickCount();
        ASSERT_FALSE( x.WaitForData( 60 ) );                     // nothing is coming: times out
        Float64 waited = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
        ASSERT_TRUE( waited >= 40 && waited < 5000 );

        for ( UInt32 i=0; i<4; ++i )
            ASSERT_TRUE( x.Write( i, 0 ) );
        ASSERT_TRUE( x.WaitForData( 0 ) && x.WaitForData( 60 ) && x.WaitForData( MT::SlotWaitForever ) );    // data there: returns at once
        ASSERT_FALSE( x.Write( 99, 0 ) );                        // zero timeout: does not wait
        start = MT::PrecisionTickCount();
        ASSERT_FALSE( x.Write( 99, 60 ) );                       // full and nobody reads: times out
        waited = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
        ASSERT_TRUE( waited >= 40 && waited < 5000 );
        for ( UInt32 i=0; i<4; ++i )
        {
            ASSERT_TRUE( x.Read( v ) );
            ASSERT_TRUE( i == v );                               // the refused values were not stored
        }
        ASSERT_FALSE( x.Read( v ) );
        ASSERT_TRUE( x.Write( 7, 60 ) && x.Read( v ) && 7 == v );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SlotDataExchange: Block: the writer blocks while full and resumes after a Read" )
    try
    {
        SOrderedTransfer t( 2, 3, 0 );
        ASSERT_TRUE( t.blockExchange.Write( 1, 0 ) && t.blockExchange.Write( 2, 0 ) );      // full
        CExchangeTestThread writer( &ProduceBlock, &t );                                    // writes 0, 1, 2 with no timeout
        ASSERT_TRUE( writer.Activate( GUCEF_NULL, 0, 0 ) );
        MT::PrecisionDelay( 150 );
        ASSERT_FALSE( writer.IsDone() );                         // still waiting for room
        UInt32 v = 0;
        ASSERT_TRUE( t.blockExchange.Read( v ) && 1 == v );
        MT::PrecisionDelay( 150 );
        ASSERT_FALSE( writer.IsDone() );                         // one slot freed, it wrote 0, and waits again for the next
        ASSERT_TRUE( t.blockExchange.Read( v ) && 2 == v );
        MT::PrecisionDelay( 150 );
        ASSERT_FALSE( writer.IsDone() );                         // it wrote 1 and waits for room for the last value
        ASSERT_TRUE( t.blockExchange.Read( v ) && 0 == v );
        ASSERT_TRUE( writer.WaitUntilDone( 20000 ) );
        ASSERT_TRUE( t.blockExchange.Read( v ) && 1 == v );
        ASSERT_TRUE( t.blockExchange.Read( v ) && 2 == v );
        ASSERT_FALSE( t.blockExchange.Read( v ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SlotDataExchange: Block: WaitForData sleeps until the writer writes" )
    try
    {
        SOrderedTransfer t( 4, 1, 0 );
        CExchangeTestThread reader( &ConsumeBlock, &t );         // reads one value, waiting for it without a timeout
        ASSERT_TRUE( reader.Activate( GUCEF_NULL, 0, 0 ) );
        MT::PrecisionDelay( 150 );
        ASSERT_FALSE( reader.IsDone() );
        ASSERT_TRUE( t.blockExchange.Write( 0, 0 ) );
        ASSERT_TRUE( reader.WaitUntilDone( 20000 ) );
        ASSERT_TRUE( 1 == t.received && 0 == t.outOfOrder );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SlotDataExchange: Block: a sleeping WaitForData and Write wake well under 1 ms after the other side acts" )
    try
    {
        const Float64 readerWake = MedianWakeLatencyInMs( true, 9 );
        const Float64 writerWake = MedianWakeLatencyInMs( false, 9 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SlotDataExchange: median wake latency in ms: WaitForData " << readerWake << ", Write " << writerWake );
        ASSERT_TRUE( readerWake >= 0 && readerWake < WakeLatencyBoundInMs );
        ASSERT_TRUE( writerWake >= 0 && writerWake < WakeLatencyBoundInMs );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SlotDataExchange: Latest only, one reader (triple buffer): newest wins, nothing new is false, never written leaves out alone" )
    try
    {
        MT::TSlotDataExchangeTypes< UInt32 >::TLatest x;
        ASSERT_TRUE( 3 == x.GetSlotCount() && 1 == x.GetCapacity() );
        ASSERT_TRUE( x.IsEmpty() && !x.HasData() && !x.HasNewData() );
        UInt32 v = 77;
        ASSERT_FALSE( x.Read( v ) );
        ASSERT_TRUE( 77 == v );
        x.Write( 1 );
        ASSERT_TRUE( x.HasData() && !x.IsEmpty() && x.HasNewData() );
        x.Write( 2 );
        x.Write( 3 );
        ASSERT_TRUE( x.Read( v ) && 3 == v );                    // 1 and 2 were skipped
        ASSERT_FALSE( x.HasNewData() );
        v = 0;
        ASSERT_FALSE( x.Read( v ) );
        ASSERT_TRUE( 3 == v );                                   // same value as before
        ASSERT_FALSE( x.Read( v ) && 3 != v );
        x.Write( 4 );
        ASSERT_TRUE( x.Read( v ) && 4 == v );
        ASSERT_FALSE( x.Read( v ) );
        for ( UInt32 i=10; i<1000; ++i )                         // alternate: every write is read
        {
            x.Write( i );
            ASSERT_TRUE( x.Read( v ) && i == v );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SlotDataExchange: Latest only, several readers: newest wins per reader, nothing new is false, never written leaves out alone" )
    try
    {
        MT::TSlotDataExchangeTypes< UInt32, 4 >::TLatest x;
        ASSERT_TRUE( 6 == x.GetSlotCount() && 1 == x.GetCapacity() && 4 == x.GetMaxReaders() );
        ASSERT_TRUE( x.IsEmpty() && !x.HasData() );
        UInt32 v = 77, w = 88;
        UInt64 versionA = 0, versionB = 0;
        ASSERT_FALSE( x.Read( v, versionA ) );
        ASSERT_TRUE( 77 == v && 0 == versionA );
        x.Write( 1 );
        ASSERT_TRUE( x.HasData() && !x.IsEmpty() );
        x.Write( 2 );
        x.Write( 3 );
        ASSERT_TRUE( x.Read( v, versionA ) && 3 == v );          // 1 and 2 were skipped
        v = 0;
        ASSERT_FALSE( x.Read( v, versionA ) );
        ASSERT_TRUE( 0 == v );                                   // nothing new: out is left alone
        ASSERT_TRUE( x.Read( w, versionB ) && 3 == w );          // another reader still has to see it
        ASSERT_FALSE( x.Read( w, versionB ) );
        x.Write( 4 );
        ASSERT_TRUE( x.Read( w, versionB ) && 4 == w );
        ASSERT_TRUE( x.Read( v, versionA ) && 4 == v );
        for ( UInt32 i=10; i<1000; ++i )
        {
            x.Write( i );
            ASSERT_TRUE( x.Read( v, versionA ) && i == v );
            ASSERT_FALSE( x.Read( v, versionA ) );
        }
        ASSERT_TRUE( x.Read( w, versionB ) && 999 == w );

        MT::CTSlotDataExchange< UInt32, MT::SSlotDeliveryLatestOnly, MT::SSlotFullFail, 2 > two;
        ASSERT_TRUE( 4 == two.GetSlotCount() );
        UInt64 version = 0;
        for ( UInt32 i=1; i<100000; ++i )                        // the slots rotate many times
        {
            two.Write( i );
            if ( 0 == i % 3 )
                ASSERT_TRUE( two.Read( v, version ) && i == v );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SlotDataExchange: Structures go through intact in every delivery" )
    try
    {
        MT::CTSlotDataExchange< SPayload, MT::SSlotDeliveryAllInOrder, MT::SSlotFullFail > ordered( 4 );
        MT::CTSlotDataExchange< SPayload, MT::SSlotDeliveryLatestOnly, MT::SSlotFullFail, 1 > one;
        MT::CTSlotDataExchange< SPayload, MT::SSlotDeliveryLatestOnly, MT::SSlotFullFail, 3 > three;
        SPayload p;
        UInt64 version = 0;
        ASSERT_TRUE( ordered.Write( SPayload( 42 ) ) && ordered.Read( p ) && 42 == p.a && p.IsConsistent() );
        one.Write( SPayload( 43 ) );
        ASSERT_TRUE( one.Read( p ) && 43 == p.d && p.IsConsistent() );
        three.Write( SPayload( 44 ) );
        ASSERT_TRUE( three.Read( p, version ) && 44 == p.b && p.IsConsistent() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SlotDataExchange: Stress, in order, fail when full with retry: every value once and in order" )
    try
    {
        SOrderedTransfer t( 64, OrderedTransferCount, 0 );
        CExchangeTestThread writer( &ProduceFail, &t );
        CExchangeTestThread reader( &ConsumeFail, &t );
        ASSERT_TRUE( reader.Activate( GUCEF_NULL, 0, 0 ) );
        ASSERT_TRUE( writer.Activate( GUCEF_NULL, 0, 0 ) );
        ASSERT_TRUE( writer.WaitUntilDone( 120000 ) );
        ASSERT_TRUE( reader.WaitUntilDone( 120000 ) );
        ASSERT_TRUE( OrderedTransferCount == t.received );
        ASSERT_TRUE( 0 == t.outOfOrder );
        ASSERT_TRUE( t.failExchange.IsEmpty() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SlotDataExchange: Stress, in order, block when full: every value once and in order, both sides sleep" )
    try
    {
        SOrderedTransfer t( 8, OrderedBlockTransferCount, 0 );
        CExchangeTestThread writer( &ProduceBlock, &t );
        CExchangeTestThread reader( &ConsumeBlock, &t );
        ASSERT_TRUE( reader.Activate( GUCEF_NULL, 0, 0 ) );
        ASSERT_TRUE( writer.Activate( GUCEF_NULL, 0, 0 ) );
        ASSERT_TRUE( writer.WaitUntilDone( 120000 ) );
        ASSERT_TRUE( reader.WaitUntilDone( 120000 ) );
        ASSERT_TRUE( OrderedBlockTransferCount == t.received );
        ASSERT_TRUE( 0 == t.outOfOrder );
        ASSERT_TRUE( t.blockExchange.IsEmpty() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SlotDataExchange: Stress, in order, block when full: a slow reader makes the writer wait repeatedly" )
    try
    {
        SOrderedTransfer t( 4, 600, 63 );                        // the reader pauses every 64 values
        CExchangeTestThread writer( &ProduceBlock, &t );
        CExchangeTestThread reader( &ConsumeBlock, &t );
        ASSERT_TRUE( reader.Activate( GUCEF_NULL, 0, 0 ) );
        ASSERT_TRUE( writer.Activate( GUCEF_NULL, 0, 0 ) );
        ASSERT_TRUE( writer.WaitUntilDone( 120000 ) );
        ASSERT_TRUE( reader.WaitUntilDone( 120000 ) );
        ASSERT_TRUE( 600 == t.received );
        ASSERT_TRUE( 0 == t.outOfOrder );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SlotDataExchange: Stress, latest only, writer faster than the readers, MaxReaders 1, 2 and 4" )
    try
    {
        ASSERT_TRUE( RunLatestStress< 1 >( 0, 300 ) );
        ASSERT_TRUE( RunLatestStress< 2 >( 0, 300 ) );
        ASSERT_TRUE( RunLatestStress< 4 >( 0, 300 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SlotDataExchange: Stress, latest only, readers faster than the writer, MaxReaders 1, 2 and 4" )
    try
    {
        ASSERT_TRUE( RunLatestStress< 1 >( 300, 0 ) );
        ASSERT_TRUE( RunLatestStress< 2 >( 300, 0 ) );
        ASSERT_TRUE( RunLatestStress< 4 >( 300, 0 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SlotDataExchange: Stress, latest only, writer and readers at full speed, MaxReaders 1, 2 and 4" )
    try
    {
        ASSERT_TRUE( RunLatestStress< 1 >( 0, 0 ) );
        ASSERT_TRUE( RunLatestStress< 2 >( 0, 0 ) );
        ASSERT_TRUE( RunLatestStress< 4 >( 0, 0 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
