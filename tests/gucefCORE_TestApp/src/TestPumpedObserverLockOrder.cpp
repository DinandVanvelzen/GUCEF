/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_TIMEOUT_EXCEPTION_H
#include "gucef_timeout_exception.h"
#define GUCEF_TIMEOUT_EXCEPTION_H
#endif /* GUCEF_TIMEOUT_EXCEPTION_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CPUMPEDOBSERVER_H
#include "CPumpedObserver.h"
#define GUCEF_CORE_CPUMPEDOBSERVER_H
#endif /* GUCEF_CORE_CPUMPEDOBSERVER_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPumpedObserverLockOrder.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

/** How long the two pulse threads of a case run side by side */
static const UInt32 MailLockOrderRunTimeInMs = 3000;

/** A pulse thread that delivers no mail for this long is waiting on a lock the other thread will not give up */
static const UInt32 MailLockOrderStallTimeInMs = 2000;

/** After a deadlock a lock wait ends with an exception after the lock timeout of gucefMT: the threads end after that */
static const UInt32 MailLockOrderThreadEndTimeInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS + 15000;

/** Mail each side starts with: the delivery of a pulse then takes a while, so the two deliveries overlap */
static const UInt32 MailLockOrderMailInFlight = 64;

/*-------------------------------------------------------------------------*/

static const CORE::CEvent&
MailPingEvent( void )
{
    static const CORE::CEvent event( "GUCEF::CORE::TestPumpedObserverLockOrder::MailPingEvent" );
    event.EnsureInitialized();
    return event;
}

/*-------------------------------------------------------------------------*/

/** What a pulse thread shares with the test thread */
struct SMailPulseThread
{
    CORE::PulseGeneratorPtr pulseGenerator;
    MT::CTAtomic< bool > stop;
    MT::CTAtomic< UInt32 > lockTimeouts;
    MT::CTAtomic< UInt32 > otherExceptions;

    SMailPulseThread( void )
        : pulseGenerator()
        , stop( false )
        , lockTimeouts( 0 )
        , otherExceptions( 0 )
    {}
};

/*-------------------------------------------------------------------------*/

/** Pulses its own pulse generator until it is asked to stop: the only thread that ever pulses it */
static UInt32 GUCEF_CALLSPEC_PREFIX
MailPulseThreadMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SMailPulseThread& pulseThread = *static_cast< SMailPulseThread* >( data );
    while ( !pulseThread.stop.Load( MT::MEMORDER_ACQUIRE ) )
    {
        try
        {
            pulseThread.pulseGenerator->ForceDirectPulse();
        }
        catch ( const timeout_exception& )
        {
            pulseThread.lockTimeouts.FetchAdd( 1 );
        }
        catch ( ... )
        {
            pulseThread.otherExceptions.FetchAdd( 1 );
        }
        MT::ThreadDelay( 0 );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

/** A pumped observer that answers every mail it gets with a mail to the other observer, from within the delivery */
class CMailPingObserver : public CORE::CPumpedObserver
{
    public:

    explicit CMailPingObserver( CORE::PulseGeneratorPtr pulseGenerator )
        : CORE::CPumpedObserver( pulseGenerator, false )
        , m_other( GUCEF_NULL )
        , received( 0 )
        , stopAnswering( false )
    {}

    virtual ~CMailPingObserver()
    {
        Shutdown();
    }

    void SetOther( CMailPingObserver* other )
    {
        m_other = other;
    }

    void Ping( void )
    {
        AddEventToMailbox( GUCEF_NULL, MailPingEvent(), GUCEF_NULL );
    }

    MT::CTAtomic< UInt32 > received;
    MT::CTAtomic< bool > stopAnswering;

    protected:

    virtual void OnPumpedNotify( CORE::CNotifier* /*notifier*/        ,
                                 const CORE::CEvent& eventid           ,
                                 CORE::CICloneable* /*eventdata*/      ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MailPingEvent() == eventid )
        {
            received.FetchAdd( 1 );
            if ( !stopAnswering.Load( MT::MEMORDER_ACQUIRE ) && GUCEF_NULL != m_other )
                m_other->Ping();
        }
    }

    private:

    CMailPingObserver* m_other;
};

/*-------------------------------------------------------------------------*/

/** A thread safe notifier whose events can be raised from any thread */
class CMailPingNotifier : public CORE::CTSGNotifier
{
    public:

    explicit CMailPingNotifier( CORE::PulseGeneratorPtr pulseGenerator )
        : CORE::CTSGNotifier( pulseGenerator, false, true )
    {}

    virtual ~CMailPingNotifier()
    {
        SignalUpcomingDestruction();
    }

    void PingFromThread( void )
    {
        NotifyObserversFromThread( MailPingEvent() );
    }
};

/*-------------------------------------------------------------------------*/

/** Observes one notifier and answers each of its events with an event of the other notifier, raised from thread */
class CMailPingAnswerer : public CORE::CObserver
{
    public:

    CMailPingAnswerer( void )
        : CORE::CObserver()
        , m_other( GUCEF_NULL )
        , received( 0 )
        , stopAnswering( false )
    {}

    virtual ~CMailPingAnswerer()
    {
        SignalUpcomingObserverDestruction();
    }

    void SetOther( CMailPingNotifier* other )
    {
        m_other = other;
    }

    MT::CTAtomic< UInt32 > received;
    MT::CTAtomic< bool > stopAnswering;

    protected:

    virtual void OnNotify( CORE::CNotifier* /*notifier*/          ,
                           const CORE::CEvent& eventid             ,
                           CORE::CICloneable* /*eventdata*/ = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MailPingEvent() == eventid )
        {
            received.FetchAdd( 1 );
            if ( !stopAnswering.Load( MT::MEMORDER_ACQUIRE ) && GUCEF_NULL != m_other )
                m_other->PingFromThread();
        }
    }

    private:

    CMailPingNotifier* m_other;
};

/*-------------------------------------------------------------------------*/

/** What the threads of a case work with. The threads may outlive a case that saw a deadlock, so it is not on a stack */
struct SMailLockOrderCase
{
    SMailPulseThread threadA;
    SMailPulseThread threadB;
};

/*-------------------------------------------------------------------------*/

/** The outcome of running the two pulse threads of a case side by side */
struct SMailLockOrderOutcome
{
    bool started;
    bool stalled;               /**< one side delivered no mail for MailLockOrderStallTimeInMs */
    bool threadsEnded;
    UInt32 lockTimeouts;
    UInt32 otherExceptions;
};

/*-------------------------------------------------------------------------*/

/**
 *  Runs both pulse threads for MailLockOrderRunTimeInMs, both received counters have to keep growing, then asks the
 *  sides to stop answering and the threads to end
 */
static SMailLockOrderOutcome
RunMailLockOrderCase( SMailLockOrderCase& lockCase            ,
                      MT::CTAtomic< UInt32 >& receivedA       ,
                      MT::CTAtomic< UInt32 >& receivedB       ,
                      MT::CTAtomic< bool >& stopAnsweringA    ,
                      MT::CTAtomic< bool >& stopAnsweringB    )
{
    SMailLockOrderOutcome outcome;
    outcome.started = false;
    outcome.stalled = false;
    outcome.threadsEnded = false;
    outcome.lockTimeouts = 0;
    outcome.otherExceptions = 0;

    MT::SThreadData* threadA = MT::ThreadDataReserve();
    MT::SThreadData* threadB = MT::ThreadDataReserve();
    if ( GUCEF_NULL == threadA || GUCEF_NULL == threadB )
        return outcome;
    if ( 0 == MT::ThreadCreate( threadA, &MailPulseThreadMain, &lockCase.threadA ) )
        return outcome;
    if ( 0 == MT::ThreadCreate( threadB, &MailPulseThreadMain, &lockCase.threadB ) )
    {
        lockCase.threadA.stop.Store( true, MT::MEMORDER_RELEASE );
        MT::ThreadWait( threadA, MailLockOrderThreadEndTimeInMs );
        return outcome;
    }
    outcome.started = true;

    const UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    const UInt64 startTicks = MT::PrecisionTickCount();
    UInt64 progressTicksA = startTicks;
    UInt64 progressTicksB = startTicks;
    UInt32 lastA = 0;
    UInt32 lastB = 0;
    while ( true )
    {
        MT::ThreadDelay( 10 );
        const UInt64 nowTicks = MT::PrecisionTickCount();
        const UInt32 a = receivedA.Load();
        const UInt32 b = receivedB.Load();
        if ( a != lastA )
        {
            lastA = a;
            progressTicksA = nowTicks;
        }
        if ( b != lastB )
        {
            lastB = b;
            progressTicksB = nowTicks;
        }
        if ( ( nowTicks - progressTicksA ) > MailLockOrderStallTimeInMs * ticksPerMs ||
             ( nowTicks - progressTicksB ) > MailLockOrderStallTimeInMs * ticksPerMs  )
        {
            outcome.stalled = true;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Pumped observer lock order: a pulse thread delivered no mail for "
                << MailLockOrderStallTimeInMs << " ms, delivered to A " << a << ", to B " << b );
            break;
        }
        if ( ( nowTicks - startTicks ) > MailLockOrderRunTimeInMs * ticksPerMs )
            break;
    }

    stopAnsweringA.Store( true, MT::MEMORDER_RELEASE );
    stopAnsweringB.Store( true, MT::MEMORDER_RELEASE );
    lockCase.threadA.stop.Store( true, MT::MEMORDER_RELEASE );
    lockCase.threadB.stop.Store( true, MT::MEMORDER_RELEASE );
    const bool endedA = 1 == MT::ThreadWait( threadA, MailLockOrderThreadEndTimeInMs );
    const bool endedB = 1 == MT::ThreadWait( threadB, MailLockOrderThreadEndTimeInMs );
    outcome.threadsEnded = endedA && endedB;

    // the thread data of a thread that did not end stays: that thread may still use it
    if ( endedA )
        MT::ThreadDataCleanup( threadA );
    if ( endedB )
        MT::ThreadDataCleanup( threadB );

    outcome.lockTimeouts = lockCase.threadA.lockTimeouts.Load() + lockCase.threadB.lockTimeouts.Load();
    outcome.otherExceptions = lockCase.threadA.otherExceptions.Load() + lockCase.threadB.otherExceptions.Load();
    return outcome;
}

/*-------------------------------------------------------------------------*/

/**
 *  A case with its pulse generators. Made when the case runs, not at static initialization: the pulse generators need
 *  the global objects of gucefCORE, which have to be made by main() (CoreGlobalTeardown relies on that order). A case
 *  whose threads did not end is never destroyed
 */
static SMailLockOrderCase*
CreateMailLockOrderCase( void )
{
    SMailLockOrderCase* lockCase = GUCEF_NEW SMailLockOrderCase();
    lockCase->threadA.pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
    lockCase->threadB.pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
    return lockCase;
}

/*-------------------------------------------------------------------------*/

void
PerformPumpedObserverLockOrderTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING pumped observer lock order TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "PumpedObserverLockOrder" );

    GUCEF_TESTFW_TESTCASE( "Two pumped observers on two pulse threads mail each other from within the delivery of their mail: no deadlock" )
        try
        {
            SMailLockOrderCase& lockCase = *CreateMailLockOrderCase();

            // the observers stay when the threads did not end: those may still deliver to them
            CMailPingObserver* observerA = GUCEF_NEW CMailPingObserver( lockCase.threadA.pulseGenerator );
            CMailPingObserver* observerB = GUCEF_NEW CMailPingObserver( lockCase.threadB.pulseGenerator );
            observerA->SetOther( observerB );
            observerB->SetOther( observerA );
            for ( UInt32 i=0; i<MailLockOrderMailInFlight; ++i )
            {
                observerA->Ping();
                observerB->Ping();
            }

            SMailLockOrderOutcome outcome = RunMailLockOrderCase( lockCase, observerA->received, observerB->received, observerA->stopAnswering, observerB->stopAnswering );
            ASSERT_TRUE( outcome.started );
            ASSERT_FALSE( outcome.stalled );
            ASSERT_TRUE( outcome.threadsEnded );
            ASSERT_TRUE( 0 == outcome.lockTimeouts );
            ASSERT_TRUE( 0 == outcome.otherExceptions );
            ASSERT_TRUE( observerA->received.Load() > MailLockOrderMailInFlight );
            ASSERT_TRUE( observerB->received.Load() > MailLockOrderMailInFlight );

            if ( outcome.threadsEnded )
            {
                GUCEF_DELETE observerA;
                GUCEF_DELETE observerB;
                GUCEF_DELETE &lockCase;
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Two thread safe notifiers on two pulse threads whose observers raise an event of the other one from thread: no deadlock" )
        try
        {
            SMailLockOrderCase& lockCase = *CreateMailLockOrderCase();

            CMailPingNotifier* notifierA = GUCEF_NEW CMailPingNotifier( lockCase.threadA.pulseGenerator );
            CMailPingNotifier* notifierB = GUCEF_NEW CMailPingNotifier( lockCase.threadB.pulseGenerator );
            CMailPingAnswerer* answererA = GUCEF_NEW CMailPingAnswerer();
            CMailPingAnswerer* answererB = GUCEF_NEW CMailPingAnswerer();
            answererA->SetOther( notifierB );
            answererB->SetOther( notifierA );
            answererA->SubscribeTo( notifierA, MailPingEvent() );
            answererB->SubscribeTo( notifierB, MailPingEvent() );
            for ( UInt32 i=0; i<MailLockOrderMailInFlight; ++i )
            {
                notifierA->PingFromThread();
                notifierB->PingFromThread();
            }

            SMailLockOrderOutcome outcome = RunMailLockOrderCase( lockCase, answererA->received, answererB->received, answererA->stopAnswering, answererB->stopAnswering );
            ASSERT_TRUE( outcome.started );
            ASSERT_FALSE( outcome.stalled );
            ASSERT_TRUE( outcome.threadsEnded );
            ASSERT_TRUE( 0 == outcome.lockTimeouts );
            ASSERT_TRUE( 0 == outcome.otherExceptions );
            ASSERT_TRUE( answererA->received.Load() > MailLockOrderMailInFlight );
            ASSERT_TRUE( answererB->received.Load() > MailLockOrderMailInFlight );

            if ( outcome.threadsEnded )
            {
                GUCEF_DELETE answererA;
                GUCEF_DELETE answererB;
                GUCEF_DELETE notifierA;
                GUCEF_DELETE notifierB;
                GUCEF_DELETE &lockCase;
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
