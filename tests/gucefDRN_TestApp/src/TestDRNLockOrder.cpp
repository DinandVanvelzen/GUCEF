/*
 *  gucefDRN_TestApp: GUCEF test application for the DRN module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDRNTestUtils.h"
#include "TestDRNLockOrder.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace DRNTEST;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------*/

/** How long the threads of a case run side by side */
static const UInt32 NodeLockOrderRunTimeInMs = 3000;

/** A thread that makes no progress for this long is waiting on a lock another thread will not give up */
static const UInt32 NodeLockOrderStallTimeInMs = 2000;

/** After a deadlock a lock wait ends with an exception after the lock timeout of gucefMT: the threads end after that */
static const UInt32 NodeLockOrderThreadEndTimeInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS + 15000;

/** How long the nodes may take to link and to form the session */
static const UInt32 NodeLockOrderSetupTimeInMs = 10000;

/** Application messages each side starts the echo with */
static const UInt32 NodeLockOrderEchoesInFlight = 32;

/*-------------------------------------------------------------------------*/

/** What one thread of a case shares with the test thread */
struct SNodeLockOrderThread
{
    MT::CTAtomic< bool > stop;
    MT::CTAtomic< UInt32 > progress;
    MT::CTAtomic< UInt32 > lockTimeouts;
    MT::CTAtomic< UInt32 > otherExceptions;

    SNodeLockOrderThread( void )
        : stop( false )
        , progress( 0 )
        , lockTimeouts( 0 )
        , otherExceptions( 0 )
    {}
};

/*-------------------------------------------------------------------------*/

/** Counts the application messages of a session and, while asked to, sends every one back to its sender */
class CNodeLockOrderEcho : public CORE::CObserver
{
    public:

    MT::CTAtomic< UInt32 > received;
    MT::CTAtomic< bool > echo;

    CNodeLockOrderEcho( void )
        : CORE::CObserver()
        , received( 0 )
        , echo( false )
        , m_session( GUCEF_NULL )
    {}

    virtual ~CNodeLockOrderEcho()
    {
        SignalUpcomingObserverDestruction();
    }

    void Watch( DRN::CDRNSession* session )
    {
        m_session = session;
        TEventCallback callback( this, &CNodeLockOrderEcho::OnAppMessage );
        SubscribeTo( session, DRN::CDRNSession::AppMessageEvent, callback );
    }

    private:

    typedef CORE::CTEventHandlerFunctor< CNodeLockOrderEcho > TEventCallback;

    void OnAppMessage( CORE::CNotifier* /*notifier*/, const CORE::CEvent& /*eventid*/, CORE::CICloneable* eventdata )
    {
        received.FetchAdd( 1 );
        if ( GUCEF_NULL == eventdata || !echo.Load( MT::MEMORDER_ACQUIRE ) )
            return;
        const DRN::CDRNAppMessage& message = static_cast< DRN::CDRNAppMessageEventData* >( eventdata )->GetData();
        m_session->SendAppMessage( message.fromPeer, 0, message.payload );
    }

    DRN::CDRNSession* m_session;
};

/*-------------------------------------------------------------------------*/

/**
 *  Two linked nodes on one pulse generator of the application, the host and the joiner of one session. The test
 *  thread pulses that pulse generator while it sets the nodes up; then the pulse thread of the case does, the only
 *  thread that pulses it from then on.
 */
struct SNodeLockOrderCase
{
    CORE::PulseGeneratorPtr appPulse;
    SNodeLockOrderThread pulser;               /**< pulses the pulse generator of the application */
    SNodeLockOrderThread caller;               /**< calls into the nodes */
    DRN::CDRNNode* host;
    DRN::CDRNNode* joiner;
    DRN::CDRNSession* hostSession;
    DRN::CDRNSession* joinSession;
    CNodeLockOrderEcho* hostEcho;
    CNodeLockOrderEcho* joinEcho;
    DRN::CDRNEndpoint hostEndpoint;
    UInt32 hostPeerId;
    UInt32 joinPeerId;

    SNodeLockOrderCase( void )
        : appPulse()
        , pulser()
        , caller()
        , host( GUCEF_NULL )
        , joiner( GUCEF_NULL )
        , hostSession( GUCEF_NULL )
        , joinSession( GUCEF_NULL )
        , hostEcho( GUCEF_NULL )
        , joinEcho( GUCEF_NULL )
        , hostEndpoint()
        , hostPeerId( 0 )
        , joinPeerId( 0 )
    {}
};

/*-------------------------------------------------------------------------*/

/** Pulses the pulse generator of the application until it is asked to stop */
static UInt32 GUCEF_CALLSPEC_PREFIX
NodeLockOrderPulserMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SNodeLockOrderCase& lockCase = *static_cast< SNodeLockOrderCase* >( data );
    while ( !lockCase.pulser.stop.Load( MT::MEMORDER_ACQUIRE ) )
    {
        try
        {
            lockCase.appPulse->ForceDirectPulse();
            lockCase.pulser.progress.FetchAdd( 1 );
        }
        catch ( const timeout_exception& )
        {
            lockCase.pulser.lockTimeouts.FetchAdd( 1 );
            break;
        }
        catch ( ... )
        {
            lockCase.pulser.otherExceptions.FetchAdd( 1 );
            break;
        }
        MT::ThreadDelay( 1 );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

/**
 *  Calls into both nodes, round after round: makes and destroys sessions, sends application messages over the session
 *  and every now and then connects the joiner to the host again (a link of its own, a channel of its own)
 */
static UInt32 GUCEF_CALLSPEC_PREFIX
NodeLockOrderCallerMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SNodeLockOrderCase& lockCase = *static_cast< SNodeLockOrderCase* >( data );
    const char payload[ 32 ] = "node lock order: caller";
    UInt32 round = 0;
    while ( !lockCase.caller.stop.Load( MT::MEMORDER_ACQUIRE ) )
    {
        try
        {
            DRN::CDRNSession* session = lockCase.host->CreateSession();
            lockCase.host->DestroySession( session );
            session = lockCase.joiner->CreateSession();
            lockCase.joiner->DestroySession( session );

            lockCase.joinSession->SendAppMessage( lockCase.hostPeerId, 0, payload, sizeof( payload ) );
            lockCase.hostSession->SendAppMessage( lockCase.joinPeerId, 0, payload, sizeof( payload ) );

            if ( 0 == ( round % 50 ) )
                lockCase.joiner->Connect( DRN::CDRNEndpointList( lockCase.hostEndpoint ) );

            ++round;
            lockCase.caller.progress.FetchAdd( 1 );
        }
        catch ( const timeout_exception& )
        {
            lockCase.caller.lockTimeouts.FetchAdd( 1 );
            break;
        }
        catch ( ... )
        {
            lockCase.caller.otherExceptions.FetchAdd( 1 );
            break;
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

/** Pulses the pulse generator of the application on the test thread until the condition is met or the time is up */
static bool
PumpAppPulseUntil( SNodeLockOrderCase& lockCase, CTestCondition& condition, UInt32 timeoutInMs )
{
    const UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    const UInt64 startTicks = MT::PrecisionTickCount();
    while ( !condition.IsMet() )
    {
        if ( ( MT::PrecisionTickCount() - startTicks ) > timeoutInMs * ticksPerMs )
            return false;
        lockCase.appPulse->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Both sessions see both members */
class CNodeLockOrderSessionFormed : public CTestCondition
{
    public:

    explicit CNodeLockOrderSessionFormed( SNodeLockOrderCase& lockCase ) : m_case( lockCase ) {}

    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        return m_case.hostSession->GetMemberCount() >= 2 && m_case.joinSession->GetMemberCount() >= 2 && 0 != m_case.joinSession->GetLocalPeerId();
    }

    private:

    SNodeLockOrderCase& m_case;
};

/*-------------------------------------------------------------------------*/

/** Sets up the two nodes and the session, on the test thread, which pulses the pulse generator of the application meanwhile */
static bool
SetupNodeLockOrderCase( SNodeLockOrderCase& lockCase, bool ownThread )
{
    lockCase.appPulse = CORE::CPulseGenerator::CreateSharedObj();
    lockCase.host = GUCEF_NEW DRN::CDRNNode( lockCase.appPulse );
    lockCase.joiner = GUCEF_NEW DRN::CDRNNode( lockCase.appPulse );

    DRN::CDRNNodeSettings settings;
    settings.echoIntervalMs = 50;
    settings.pulseIntervalMs = 1;
    settings.ownThread = ownThread;
    if ( !lockCase.host->SetSettings( settings ) || !lockCase.joiner->SetSettings( settings ) )
        return false;

    lockCase.hostEndpoint = LoopbackEndpoint( "tcp", NextTestPort() );
    if ( !lockCase.host->Listen( lockCase.hostEndpoint ) )
        return false;

    lockCase.hostSession = lockCase.host->CreateSession();
    lockCase.joinSession = lockCase.joiner->CreateSession();
    lockCase.hostEcho = GUCEF_NEW CNodeLockOrderEcho();
    lockCase.joinEcho = GUCEF_NEW CNodeLockOrderEcho();
    lockCase.hostEcho->Watch( lockCase.hostSession );
    lockCase.joinEcho->Watch( lockCase.joinSession );

    DRN::CDRNSessionSettings sessionSettings;
    sessionSettings.sessionName = "lockorder";
    if ( !lockCase.hostSession->Host( sessionSettings ) )
        return false;
    if ( !lockCase.joinSession->Join( DRN::CDRNEndpointList( lockCase.hostEndpoint ), sessionSettings ) )
        return false;

    CNodeLockOrderSessionFormed formed( lockCase );
    if ( !PumpAppPulseUntil( lockCase, formed, NodeLockOrderSetupTimeInMs ) )
        return false;
    lockCase.hostPeerId = lockCase.hostSession->GetLocalPeerId();
    lockCase.joinPeerId = lockCase.joinSession->GetLocalPeerId();
    return 0 != lockCase.hostPeerId && 0 != lockCase.joinPeerId;
}

/*-------------------------------------------------------------------------*/

/** The outcome of running the threads of a case side by side */
struct SNodeLockOrderOutcome
{
    bool started;
    bool stalled;
    bool threadsEnded;
    UInt32 lockTimeouts;
    UInt32 otherExceptions;
};

/*-------------------------------------------------------------------------*/

/** A progress counter the test thread watches */
struct SNodeLockOrderProgress
{
    const char* name;
    MT::CTAtomic< UInt32 >* counter;
    UInt32 last;
    UInt64 lastChangeTicks;
};

/*-------------------------------------------------------------------------*/

/**
 *  Starts the pulse thread and the caller thread, and with echo the echo of both sessions, and watches the progress
 *  for NodeLockOrderRunTimeInMs; then stops the caller, the echo and the pulse thread in that order
 */
static SNodeLockOrderOutcome
RunNodeLockOrderCase( SNodeLockOrderCase& lockCase, bool echo )
{
    SNodeLockOrderOutcome outcome;
    outcome.started = false;
    outcome.stalled = false;
    outcome.threadsEnded = false;
    outcome.lockTimeouts = 0;
    outcome.otherExceptions = 0;

    if ( echo )
    {
        lockCase.hostEcho->echo.Store( true, MT::MEMORDER_RELEASE );
        lockCase.joinEcho->echo.Store( true, MT::MEMORDER_RELEASE );
        const char payload[ 32 ] = "node lock order: echo";
        for ( UInt32 i=0; i<NodeLockOrderEchoesInFlight; ++i )
        {
            lockCase.joinSession->SendAppMessage( lockCase.hostPeerId, 0, payload, sizeof( payload ) );
            lockCase.hostSession->SendAppMessage( lockCase.joinPeerId, 0, payload, sizeof( payload ) );
        }
    }

    MT::SThreadData* pulserThread = MT::ThreadDataReserve();
    MT::SThreadData* callerThread = MT::ThreadDataReserve();
    if ( GUCEF_NULL == pulserThread || GUCEF_NULL == callerThread )
        return outcome;
    if ( 0 == MT::ThreadCreate( pulserThread, &NodeLockOrderPulserMain, &lockCase ) )
        return outcome;
    if ( 0 == MT::ThreadCreate( callerThread, &NodeLockOrderCallerMain, &lockCase ) )
    {
        lockCase.pulser.stop.Store( true, MT::MEMORDER_RELEASE );
        MT::ThreadWait( pulserThread, NodeLockOrderThreadEndTimeInMs );
        return outcome;
    }
    outcome.started = true;

    const UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    const UInt64 startTicks = MT::PrecisionTickCount();
    SNodeLockOrderProgress progress[ 4 ] =
    {
        { "pulses", &lockCase.pulser.progress, 0, startTicks },
        { "caller rounds", &lockCase.caller.progress, 0, startTicks },
        { "messages to the host", &lockCase.hostEcho->received, 0, startTicks },
        { "messages to the joiner", &lockCase.joinEcho->received, 0, startTicks }
    };
    const UInt32 watched = echo ? 4 : 2;
    while ( !outcome.stalled )
    {
        MT::ThreadDelay( 10 );
        const UInt64 nowTicks = MT::PrecisionTickCount();
        for ( UInt32 i=0; i<watched; ++i )
        {
            const UInt32 value = progress[ i ].counter->Load();
            if ( value != progress[ i ].last )
            {
                progress[ i ].last = value;
                progress[ i ].lastChangeTicks = nowTicks;
            }
            else
            if ( ( nowTicks - progress[ i ].lastChangeTicks ) > NodeLockOrderStallTimeInMs * ticksPerMs )
            {
                outcome.stalled = true;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Node lock order: no progress of the " << progress[ i ].name << " for "
                    << NodeLockOrderStallTimeInMs << " ms (at " << value << ")" );
            }
        }
        if ( ( nowTicks - startTicks ) > NodeLockOrderRunTimeInMs * ticksPerMs )
            break;
    }

    lockCase.caller.stop.Store( true, MT::MEMORDER_RELEASE );
    const bool callerEnded = 1 == MT::ThreadWait( callerThread, NodeLockOrderThreadEndTimeInMs );
    lockCase.hostEcho->echo.Store( false, MT::MEMORDER_RELEASE );
    lockCase.joinEcho->echo.Store( false, MT::MEMORDER_RELEASE );
    lockCase.pulser.stop.Store( true, MT::MEMORDER_RELEASE );
    const bool pulserEnded = 1 == MT::ThreadWait( pulserThread, NodeLockOrderThreadEndTimeInMs );
    outcome.threadsEnded = callerEnded && pulserEnded;

    // the thread data of a thread that did not end stays: that thread may still use it
    if ( callerEnded )
        MT::ThreadDataCleanup( callerThread );
    if ( pulserEnded )
        MT::ThreadDataCleanup( pulserThread );

    outcome.lockTimeouts = lockCase.pulser.lockTimeouts.Load() + lockCase.caller.lockTimeouts.Load();
    outcome.otherExceptions = lockCase.pulser.otherExceptions.Load() + lockCase.caller.otherExceptions.Load();
    return outcome;
}

/*-------------------------------------------------------------------------*/

/** Lets go of the nodes on the test thread, once no other thread uses them */
static void
TeardownNodeLockOrderCase( SNodeLockOrderCase& lockCase )
{
    GUCEF_DELETE lockCase.hostEcho;
    lockCase.hostEcho = GUCEF_NULL;
    GUCEF_DELETE lockCase.joinEcho;
    lockCase.joinEcho = GUCEF_NULL;
    GUCEF_DELETE lockCase.joiner;
    lockCase.joiner = GUCEF_NULL;
    GUCEF_DELETE lockCase.host;
    lockCase.host = GUCEF_NULL;
    lockCase.hostSession = GUCEF_NULL;
    lockCase.joinSession = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/

/** The threads of a case may outlive it after a deadlock, so what they use is made on the heap and then never let go of */
static void
RunNodeLockOrderTest( bool ownThread, bool echo )
{
    SNodeLockOrderCase& lockCase = *( GUCEF_NEW SNodeLockOrderCase() );
    bool setUp = SetupNodeLockOrderCase( lockCase, ownThread );
    ASSERT_TRUE( setUp );
    if ( !setUp )
    {
        TeardownNodeLockOrderCase( lockCase );
        GUCEF_DELETE &lockCase;
        return;
    }

    SNodeLockOrderOutcome outcome = RunNodeLockOrderCase( lockCase, echo );
    ASSERT_TRUE( outcome.started );
    ASSERT_FALSE( outcome.stalled );
    ASSERT_TRUE( outcome.threadsEnded );
    ASSERT_TRUE( 0 == outcome.lockTimeouts );
    ASSERT_TRUE( 0 == outcome.otherExceptions );
    ASSERT_TRUE( lockCase.caller.progress.Load() > 0 );
    ASSERT_TRUE( lockCase.hostEcho->received.Load() > 0 );
    ASSERT_TRUE( lockCase.joinEcho->received.Load() > 0 );
    if ( echo )
    {
        ASSERT_TRUE( lockCase.hostEcho->received.Load() > NodeLockOrderEchoesInFlight );
        ASSERT_TRUE( lockCase.joinEcho->received.Load() > NodeLockOrderEchoesInFlight );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Node lock order: " << lockCase.pulser.progress.Load() << " pulses, " << lockCase.caller.progress.Load()
        << " caller rounds, " << lockCase.hostEcho->received.Load() << " messages to the host, " << lockCase.joinEcho->received.Load() << " to the joiner" );

    // nodes that a thread which did not end may still use stay
    if ( outcome.threadsEnded )
    {
        TeardownNodeLockOrderCase( lockCase );
        GUCEF_DELETE &lockCase;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformDRNLockOrderTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING DRN node lock order TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "DRNLockOrder" );

    GUCEF_TESTFW_TESTCASE( "Nodes on the pulse generator of the application, which another thread pulses, are called from a third thread while their links carry traffic: no deadlock" )
        try
        {
            RunNodeLockOrderTest( false, false );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Nodes on threads of their own whose session events the application answers from its pulse thread while a third thread calls into the nodes: no deadlock" )
        try
        {
            RunNodeLockOrderTest( true, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
