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

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestNotifierDelivery.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const CEvent&
DeliveryTestEvent( void )
{
    static const CEvent event( "GUCEF::CORE::TestNotifierDelivery::DeliveryTestEvent" );
    event.EnsureInitialized();
    return event;
}

/*-------------------------------------------------------------------------*/

/** How the observers of a delivery are subscribed and how the event goes out */
enum EDeliveryKind
{
    DELIVERYKIND_ONE_EVENT = 0,     /**< subscribed to the test event, which is sent */
    DELIVERYKIND_ALL_EVENTS,        /**< subscribed to all events, the test event is sent */
    DELIVERYKIND_MODIFY_BROADCAST   /**< subscribed to all events, the modify event goes to every observer */
};

/*-------------------------------------------------------------------------*/

class CDeliveryTestNotifier : public CNotifier
{
    public:

    CDeliveryTestNotifier( void ) : CNotifier() {}
    virtual ~CDeliveryTestNotifier() {}

    void Send( EDeliveryKind kind )
    {
        if ( DELIVERYKIND_MODIFY_BROADCAST == kind )
            NotifyObservers();
        else
            NotifyObservers( DeliveryTestEvent() );
    }

    /** Sends the test event to the given observer only */
    void SendTo( CObserver& observer )
    {
        NotifySpecificObserver( observer, DeliveryTestEvent() );
    }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CString typeName = "CDeliveryTestNotifier";
        return typeName;
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  Counts the deliveries of the test event (or the modify event) and, at its first one, subscribes another observer
 *  and unsubscribes another one, as it was told to
 */
class CDeliveryTestObserver : public CObserver
{
    public:

    CNotifier* notifier;
    EDeliveryKind kind;
    CDeliveryTestObserver* toSubscribe;
    CDeliveryTestObserver* toUnsubscribe;
    UInt32 received;
    UInt32 destructionEvents;
    bool unsubscribedBeforeItsTurn;
    bool receivedAfterUnsubscribe;

    CDeliveryTestObserver( void )
        : CObserver()
        , notifier( GUCEF_NULL )
        , kind( DELIVERYKIND_ONE_EVENT )
        , toSubscribe( GUCEF_NULL )
        , toUnsubscribe( GUCEF_NULL )
        , received( 0 )
        , destructionEvents( 0 )
        , unsubscribedBeforeItsTurn( false )
        , receivedAfterUnsubscribe( false )
    {}

    virtual ~CDeliveryTestObserver() { UnsubscribeAllFromObserver(); }

    void Subscribe( void )
    {
        if ( DELIVERYKIND_ONE_EVENT == kind )
            SubscribeTo( notifier, DeliveryTestEvent() );
        else
            SubscribeTo( notifier );
    }

    virtual void OnNotify( CNotifier* sender, const CEvent& eventId, CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( CNotifier::DestructionEvent == eventId )
        {
            ++destructionEvents;
            return;
        }
        const CEvent& expected = DELIVERYKIND_MODIFY_BROADCAST == kind ? CNotifier::ModifyEvent : DeliveryTestEvent();
        if ( expected != eventId )
            return;

        ++received;
        if ( unsubscribedBeforeItsTurn )
            receivedAfterUnsubscribe = true;

        if ( GUCEF_NULL != toSubscribe )
        {
            toSubscribe->Subscribe();
            toSubscribe = GUCEF_NULL;
        }
        if ( GUCEF_NULL != toUnsubscribe )
        {
            if ( 0 == toUnsubscribe->received )
                toUnsubscribe->unsubscribedBeforeItsTurn = true;
            if ( DELIVERYKIND_ONE_EVENT == kind )
                notifier->Unsubscribe( toUnsubscribe, DeliveryTestEvent() );
            else
                notifier->Unsubscribe( toUnsubscribe );
            toUnsubscribe = GUCEF_NULL;
        }
    }
};

typedef GUCEF::vector< CDeliveryTestObserver* > TDeliveryTestObservers;

/*-------------------------------------------------------------------------*/

/** What an observer throws from a delivery */
struct SDeliveryTestFailure {};

/**
 *  An observer that throws from the first delivery of the test event (or the modify event) it gets, and counts the
 *  deliveries after that
 */
class CDeliveryThrowingObserver : public CDeliveryTestObserver
{
    public:

    bool thrown;

    CDeliveryThrowingObserver( void ) : CDeliveryTestObserver(), thrown( false ) {}

    virtual void OnNotify( CNotifier* sender, const CEvent& eventId, CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        const CEvent& expected = DELIVERYKIND_MODIFY_BROADCAST == kind ? CNotifier::ModifyEvent : DeliveryTestEvent();
        if ( !thrown && expected == eventId )
        {
            thrown = true;
            throw SDeliveryTestFailure();
        }
        CDeliveryTestObserver::OnNotify( sender, eventId, eventData );
    }
};

/*-------------------------------------------------------------------------*/

/** A gateway notifier on a pulse generator, as application objects are */
class CPulseGatewayNotifier : public CTSGNotifier
{
    public:

    CPulseGatewayNotifier( const PulseGeneratorPtr& pulseGenerator ) : CTSGNotifier( pulseGenerator, true, false ) {}
    virtual ~CPulseGatewayNotifier() { SignalUpcomingDestruction(); }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CString typeName = "CPulseGatewayNotifier";
        return typeName;
    }
};

/*-------------------------------------------------------------------------*/

/** Destroys a gateway notifier when it gets a pulse */
class CGatewayDestroyingObserver : public CObserver
{
    public:

    CPulseGatewayNotifier* gateway;

    CGatewayDestroyingObserver( void ) : CObserver(), gateway( GUCEF_NULL ) {}
    virtual ~CGatewayDestroyingObserver() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CNotifier* sender, const CEvent& eventId, CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( CPulseGenerator::PulseEvent == eventId && GUCEF_NULL != gateway )
        {
            GUCEF_DELETE gateway;
            gateway = GUCEF_NULL;
        }
    }
};

/*-------------------------------------------------------------------------*/

/** The outcome of one delivery to many observers */
struct SDeliveryOutcome
{
    Float64 elapsedMs;
    UInt32 wrongFirst;      /**< observers whose count after the first event is not what it should be */
    UInt32 wrongSecond;     /**< the same after a second event, which nobody changes subscriptions for */

    SDeliveryOutcome( void ) : elapsedMs( 0.0 ), wrongFirst( 0 ), wrongSecond( 0 ) {}
};

/*-------------------------------------------------------------------------*/

/**
 *  Delivers the event of the given kind to 'count' observers. Every second one subscribes a newcomer when it gets
 *  the event, every fourth one (from the second) unsubscribes another one of the original observers. Which of them
 *  have had their turn by then depends on the order of delivery, which the expectations do not assume.
 */
SDeliveryOutcome
DeliverWhileSubscriptionsChange( UInt32 count, EDeliveryKind kind )
{
    SDeliveryOutcome outcome;
    CDeliveryTestNotifier notifier;
    TDeliveryTestObservers originals;
    TDeliveryTestObservers newcomers;
    GUCEF::vector< bool > isVictim( count, false );

    for ( UInt32 i=0; i<count; ++i )
    {
        CDeliveryTestObserver* observer = GUCEF_NEW CDeliveryTestObserver();
        observer->notifier = &notifier;
        observer->kind = kind;
        originals.push_back( observer );
    }
    for ( UInt32 i=0; i<count/2; ++i )
    {
        CDeliveryTestObserver* observer = GUCEF_NEW CDeliveryTestObserver();
        observer->notifier = &notifier;
        observer->kind = kind;
        newcomers.push_back( observer );
    }
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( 0 == i % 2 )
            originals[ i ]->toSubscribe = newcomers[ i / 2 ];
        if ( 1 == i % 4 )
        {
            UInt32 victim = ( i + 2 ) % count;
            originals[ i ]->toUnsubscribe = originals[ victim ];
            isVictim[ victim ] = true;
        }
        originals[ i ]->Subscribe();
    }

    UInt64 startTicks = MT::PrecisionTickCount();
    notifier.Send( kind );
    outcome.elapsedMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks );

    // every observer subscribed when its turn came got the event once, an observer unsubscribed before its turn never
    for ( UInt32 i=0; i<count; ++i )
    {
        const CDeliveryTestObserver& observer = *originals[ i ];
        bool right = isVictim[ i ] ? ( observer.received <= 1 && !observer.receivedAfterUnsubscribe && ( !observer.unsubscribedBeforeItsTurn || 0 == observer.received ) )
                                   : 1 == observer.received;
        if ( !right )
            ++outcome.wrongFirst;
    }
    for ( UInt32 i=0; i<newcomers.size(); ++i )
    {
        if ( 1 != newcomers[ i ]->received )
            ++outcome.wrongFirst;
    }

    // and the subscriptions made during the delivery hold: the next event reaches the newcomers, not the unsubscribed
    for ( UInt32 i=0; i<count; ++i )
        originals[ i ]->received = 0;
    for ( UInt32 i=0; i<newcomers.size(); ++i )
        newcomers[ i ]->received = 0;
    notifier.Send( kind );
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( originals[ i ]->received != ( isVictim[ i ] ? 0u : 1u ) )
            ++outcome.wrongSecond;
    }
    for ( UInt32 i=0; i<newcomers.size(); ++i )
    {
        if ( 1 != newcomers[ i ]->received )
            ++outcome.wrongSecond;
    }

    for ( UInt32 i=0; i<originals.size(); ++i )
        GUCEF_DELETE originals[ i ];
    for ( UInt32 i=0; i<newcomers.size(); ++i )
        GUCEF_DELETE newcomers[ i ];
    return outcome;
}

/*-------------------------------------------------------------------------*/

/**
 *  Delivers to a small and to an eight times larger set of observers, the fastest of two runs each, and checks
 *  the outcome of every run and that the time grew linearly: eight times as many observers may take up to three
 *  times eight as long (allocation and cache effects), a delivery that starts over after every change takes
 *  about sixty four times as long
 */
void
CheckDeliveryScaling( EDeliveryKind kind, const char* name )
{
    const UInt32 smallCount = 1000;
    const UInt32 largeCount = 8 * smallCount;
    Float64 smallMs = 0.0;
    Float64 largeMs = 0.0;
    for ( UInt32 run=0; run<2; ++run )
    {
        SDeliveryOutcome small = DeliverWhileSubscriptionsChange( smallCount, kind );
        SDeliveryOutcome large = DeliverWhileSubscriptionsChange( largeCount, kind );
        if ( 0 != small.wrongFirst + small.wrongSecond + large.wrongFirst + large.wrongSecond )
        {
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "NotifierDelivery (" << name << "): observers with the wrong number of events: " << small.wrongFirst
                       << " and " << small.wrongSecond << " of " << smallCount << ", " << large.wrongFirst << " and " << large.wrongSecond << " of " << largeCount );
        }
        ASSERT_TRUE( 0 == small.wrongFirst && 0 == small.wrongSecond );
        ASSERT_TRUE( 0 == large.wrongFirst && 0 == large.wrongSecond );
        if ( 0 == run || small.elapsedMs < smallMs )
            smallMs = small.elapsedMs;
        if ( 0 == run || large.elapsedMs < largeMs )
            largeMs = large.elapsedMs;
    }

    Float64 ratio = smallMs > 0.0 ? largeMs / smallMs : 0.0;
    GUCEF_LOG( LOGLEVEL_IMPORTANT, "NotifierDelivery (" << name << "): delivery while subscriptions change: " << smallCount << " observers "
               << smallMs << " ms, " << largeCount << " observers " << largeMs << " ms, ratio " << ratio );
    ASSERT_TRUE( ratio < 24.0 );
}

}

/*-------------------------------------------------------------------------*/

void
PerformNotifierDeliveryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "NotifierDelivery" );

    GUCEF_TESTFW_TESTCASE( "Observers of one event that subscribe and unsubscribe others during its delivery: each gets it once, in time linear in their number" )
        try
        {
            CheckDeliveryScaling( DELIVERYKIND_ONE_EVENT, "one event" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Observers of all events that subscribe and unsubscribe others during a delivery: each gets it once, in time linear in their number" )
        try
        {
            CheckDeliveryScaling( DELIVERYKIND_ALL_EVENTS, "all events" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "The modify event broadcast while observers subscribe and unsubscribe others: each gets it once, in time linear in their number" )
        try
        {
            CheckDeliveryScaling( DELIVERYKIND_MODIFY_BROADCAST, "modify broadcast" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "An observer subscribed to an event while the notifier delivers gets the destruction event of the notifier" )
        try
        {
            CDeliveryTestNotifier* notifier = GUCEF_NEW CDeliveryTestNotifier();
            CDeliveryTestObserver first;
            CDeliveryTestObserver late;
            first.notifier = notifier;
            late.notifier = notifier;
            first.toSubscribe = &late;
            first.Subscribe();

            notifier->Send( DELIVERYKIND_ONE_EVENT );
            ASSERT_TRUE( 1 == late.received );
            ASSERT_TRUE( 1 == late.GetNotifierCount() );

            GUCEF_DELETE notifier;
            ASSERT_TRUE( 1 == first.destructionEvents );
            ASSERT_TRUE( 1 == late.destructionEvents );
            ASSERT_TRUE( 0 == late.GetNotifierCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "A gateway notifier destroyed by a handler of a pulse of its own pulse generator leaves no request for itself behind" )
        try
        {
            // while the pulse generator delivers, a request to unsubscribe is mail; a request made for the gateway's
            // observer as it shuts down would be carried out after the gateway is gone
            PulseGeneratorPtr pulseGenerator = CPulseGenerator::CreateSharedObj();
            CGatewayDestroyingObserver destroyer;
            destroyer.SubscribeTo( pulseGenerator.GetPointerAlways(), CPulseGenerator::PulseEvent );
            for ( UInt32 i=0; i<20; ++i )
            {
                destroyer.gateway = GUCEF_NEW CPulseGatewayNotifier( pulseGenerator );
                pulseGenerator->ForceDirectPulse();
                ASSERT_TRUE( GUCEF_NULL == destroyer.gateway );
                pulseGenerator->ForceDirectPulse();
            }
            ASSERT_TRUE( 0 == pulseGenerator->GetNrOfPeriodicPulseRequestors() );
            destroyer.UnsubscribeAllFromObserver();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "An observer that throws from a delivery: the exception leaves the notification, and the notifier delivers its later events, carries out subscriptions and is destroyed as usual" )
        try
        {
            const EDeliveryKind kinds[] = { DELIVERYKIND_ONE_EVENT, DELIVERYKIND_ALL_EVENTS, DELIVERYKIND_MODIFY_BROADCAST };
            for ( UInt32 k=0; k<3; ++k )
            {
                CDeliveryTestNotifier* notifier = GUCEF_NEW CDeliveryTestNotifier();
                CDeliveryThrowingObserver thrower;
                CDeliveryTestObserver counter;
                CDeliveryTestObserver late;
                thrower.notifier = counter.notifier = late.notifier = notifier;
                thrower.kind = counter.kind = late.kind = kinds[ k ];
                thrower.Subscribe();
                counter.Subscribe();

                bool thrown = false;
                try
                {
                    notifier->Send( kinds[ k ] );
                }
                catch ( const SDeliveryTestFailure& )
                {
                    thrown = true;
                }
                ASSERT_TRUE( thrown );

                // the observers walked after the one that threw did not get that event; the next one reaches all of them
                const UInt32 counterReceived = counter.received;
                notifier->Send( kinds[ k ] );
                const bool delivered = ( counterReceived + 1 == counter.received ) && ( 1 == thrower.received );
                ASSERT_TRUE( delivered );

                late.Subscribe();
                notifier->Send( kinds[ k ] );
                ASSERT_TRUE( 1 == late.received );

                // a notifier left busy would be destroyed while busy (an assert): it is left alone then
                if ( delivered && 1 == late.received )
                {
                    GUCEF_DELETE notifier;
                    ASSERT_TRUE( 1 == counter.destructionEvents && 1 == late.destructionEvents );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "An observer that throws from an event sent to it alone: the exception leaves the notification, and the notifier delivers its later events and is destroyed as usual" )
        try
        {
            const EDeliveryKind kinds[] = { DELIVERYKIND_ONE_EVENT, DELIVERYKIND_ALL_EVENTS };
            for ( UInt32 k=0; k<2; ++k )
            {
                CDeliveryTestNotifier* notifier = GUCEF_NEW CDeliveryTestNotifier();
                CDeliveryThrowingObserver thrower;
                CDeliveryTestObserver counter;
                thrower.notifier = counter.notifier = notifier;
                thrower.kind = counter.kind = kinds[ k ];
                thrower.Subscribe();
                counter.Subscribe();

                bool thrown = false;
                try
                {
                    notifier->SendTo( thrower );
                }
                catch ( const SDeliveryTestFailure& )
                {
                    thrown = true;
                }
                ASSERT_TRUE( thrown );

                notifier->SendTo( counter );
                notifier->Send( kinds[ k ] );
                const bool delivered = ( 2 == counter.received ) && ( 1 == thrower.received );
                ASSERT_TRUE( delivered );

                // a notifier left busy would be destroyed while busy (an assert): it is left alone then
                if ( delivered )
                {
                    GUCEF_DELETE notifier;
                    ASSERT_TRUE( 1 == counter.destructionEvents && 1 == thrower.destructionEvents );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
