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

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

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

#include "TestAllocationCounter.h"

#include "TestNotificationCost.h"

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
CostTriggerEvent( void )
{
    static const CEvent event( "GUCEF::CORE::TestNotificationCost::CostTriggerEvent" );
    event.EnsureInitialized();
    return event;
}

/*-------------------------------------------------------------------------*/

const CEvent&
CostMailedEvent( void )
{
    static const CEvent event( "GUCEF::CORE::TestNotificationCost::CostMailedEvent" );
    event.EnsureInitialized();
    return event;
}

/*-------------------------------------------------------------------------*/

class CCostTestNotifier : public CNotifier
{
    public:

    CCostTestNotifier( void ) : CNotifier() {}
    virtual ~CCostTestNotifier() {}

    void Send( const CEvent& eventId, CICloneable* eventData = GUCEF_NULL ) { NotifyObservers( eventId, eventData ); }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CString typeName = "CCostTestNotifier";
        return typeName;
    }
};

/*-------------------------------------------------------------------------*/

/** Event data that says which mail it is */
class CCostTestMailData : public CICloneable
{
    public:

    UInt32 index;

    explicit CCostTestMailData( UInt32 mailIndex ) : CICloneable(), index( mailIndex ) {}
    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CCostTestMailData( index ); }
};

/*-------------------------------------------------------------------------*/

/** Counts the events it gets; the sender raises mailCount events from inside the trigger's delivery */
class CCostTestObserver : public CObserver
{
    public:

    CCostTestNotifier* sender;
    UInt32 mailCount;
    UInt32 received;
    UInt32 mailReceived;
    bool mailInOrder;

    CCostTestObserver( void )
        : CObserver(), sender( GUCEF_NULL ), mailCount( 0 ), received( 0 ), mailReceived( 0 ), mailInOrder( true )
    {
    }

    virtual ~CCostTestObserver() { UnsubscribeAllFromObserver(); }

    void Watch( CCostTestNotifier& notifier, bool allEvents )
    {
        if ( allEvents )
        {
            SubscribeTo( &notifier );
            return;
        }
        typedef CTEventHandlerFunctor< CCostTestObserver > TEventCallback;
        TEventCallback trigger( this, &CCostTestObserver::OnTrigger );
        TEventCallback mail( this, &CCostTestObserver::OnMail );
        SubscribeTo( &notifier, CostTriggerEvent(), trigger );
        SubscribeTo( &notifier, CostMailedEvent(), mail );
    }

    virtual void OnNotify( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventId == CostTriggerEvent() )
            OnTrigger( notifier, eventId, eventData );
    }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CString typeName = "CCostTestObserver";
        return typeName;
    }

    private:

    void OnTrigger( CNotifier*, const CEvent&, CICloneable* )
    {
        ++received;
        if ( GUCEF_NULL == sender )
            return;

        // the notifier is busy delivering the trigger: every one of these is mail, delivered once the trigger is
        CCostTestNotifier* notifier = sender;
        sender = GUCEF_NULL;
        for ( UInt32 i=0; i<mailCount; ++i )
        {
            CCostTestMailData data( i );
            notifier->Send( CostMailedEvent(), &data );
        }
    }

    void OnMail( CNotifier*, const CEvent&, CICloneable* eventData )
    {
        if ( GUCEF_NULL == eventData || static_cast< CCostTestMailData* >( eventData )->index != mailReceived )
            mailInOrder = false;
        ++mailReceived;
    }
};

/*-------------------------------------------------------------------------*/

Float64
ElapsedNs( UInt64 startTicks )
{
    return ( MT::PrecisionTickCount() - startTicks ) * 1000000000.0 / MT::PrecisionTimerResolution();
}

/*-------------------------------------------------------------------------*/

/** Nanoseconds per mail of mailCount events mailed while the notifier delivers one event */
Float64
MeasureMailbox( UInt32 mailCount, bool& allDelivered )
{
    CCostTestNotifier notifier;
    CCostTestObserver observer;
    observer.Watch( notifier, false );
    observer.sender = &notifier;
    observer.mailCount = mailCount;
    UInt64 start = MT::PrecisionTickCount();
    notifier.Send( CostTriggerEvent() );
    Float64 elapsedNs = ElapsedNs( start );
    allDelivered = observer.mailReceived == mailCount && observer.mailInOrder;
    return elapsedNs / mailCount;
}

/*-------------------------------------------------------------------------*/

/** Nanoseconds per delivery to one observer of an event that observerCount observers get */
Float64
MeasureWalk( UInt32 observerCount, bool allEvents, UInt32 deliveries, bool& allDelivered )
{
    CCostTestNotifier notifier;
    GUCEF::vector< CCostTestObserver* > observers;
    for ( UInt32 i=0; i<observerCount; ++i )
    {
        observers.push_back( GUCEF_NEW CCostTestObserver() );
        observers.back()->Watch( notifier, allEvents );
    }
    for ( UInt32 i=0; i<20; ++i )
        notifier.Send( CostTriggerEvent() );
    UInt32 events = deliveries / observerCount;
    if ( events < 20 )
        events = 20;
    UInt64 start = MT::PrecisionTickCount();
    for ( UInt32 i=0; i<events; ++i )
        notifier.Send( CostTriggerEvent() );
    Float64 elapsedNs = ElapsedNs( start );
    allDelivered = true;
    for ( UInt32 i=0; i<observerCount; ++i )
    {
        if ( observers[ i ]->received != 20 + events )
            allDelivered = false;
        GUCEF_DELETE observers[ i ];
    }
    return elapsedNs / ( (Float64) events * observerCount );
}

/*-------------------------------------------------------------------------*/

/** Heap allocations of the calling thread for 200 deliveries of an event to observerCount observers, after warm up */
UInt32
CountDeliveryAllocations( UInt32 observerCount, bool allEvents )
{
    CCostTestNotifier notifier;
    GUCEF::vector< CCostTestObserver* > observers;
    for ( UInt32 i=0; i<observerCount; ++i )
    {
        observers.push_back( GUCEF_NEW CCostTestObserver() );
        observers.back()->Watch( notifier, allEvents );
    }
    for ( UInt32 i=0; i<20; ++i )
        notifier.Send( CostTriggerEvent() );
    CORETEST::StartAllocationCounting();
    for ( UInt32 i=0; i<200; ++i )
        notifier.Send( CostTriggerEvent() );
    UInt32 allocations = CORETEST::StopAllocationCounting();
    for ( UInt32 i=0; i<observerCount; ++i )
        GUCEF_DELETE observers[ i ];
    return allocations;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformNotificationCostTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( LOGLEVEL_NORMAL, "COMMENCING notification cost TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "NotificationCost" );

    GUCEF_TESTFW_TESTCASE( "Events mailed to a busy notifier are delivered in order, in time linear in their number" )
        try
        {
            // the cheapest of the runs of each size: other work on the machine only ever adds
            Float64 small = 0.0;
            Float64 large = 0.0;
            bool allDelivered = true;
            for ( UInt32 run=0; run<3; ++run )
            {
                bool delivered = false;
                Float64 perMail = MeasureMailbox( 1000, delivered );
                allDelivered = allDelivered && delivered;
                if ( 0 == run || perMail < small )
                    small = perMail;
                if ( run < 2 )
                {
                    perMail = MeasureMailbox( 16000, delivered );
                    allDelivered = allDelivered && delivered;
                    if ( 0 == run || perMail < large )
                        large = perMail;
                }
            }
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "NotificationCost: mailed events, " << small << " ns per mail of 1000, " << large << " ns per mail of 16000" );
            ASSERT_TRUE( allDelivered );

            // linear: the same per mail (taking each one off the front of the list made it grow with their number)
            ASSERT_TRUE( large < 3.0 * small );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    for ( UInt32 kind=0; kind<2; ++kind )
    {
        const bool allEvents = 1 == kind;
        GUCEF_TESTFW_TESTCASE( allEvents ? "Delivering an event to observers of all events costs about the same per observer for 8 as for 512"
                                         : "Delivering an event to its observers costs about the same per observer for 8 as for 512" )
            try
            {
                Float64 few = 0.0;
                Float64 many = 0.0;
                bool allDelivered = true;
                for ( UInt32 run=0; run<3; ++run )
                {
                    bool delivered = false;
                    Float64 perDelivery = MeasureWalk( 8, allEvents, 400000, delivered );
                    allDelivered = allDelivered && delivered;
                    if ( 0 == run || perDelivery < few )
                        few = perDelivery;
                    perDelivery = MeasureWalk( 512, allEvents, 400000, delivered );
                    allDelivered = allDelivered && delivered;
                    if ( 0 == run || perDelivery < many )
                        many = perDelivery;
                }
                GUCEF_LOG( LOGLEVEL_IMPORTANT, "NotificationCost: " << ( allEvents ? "observers of all events" : "observers of the event" ) << ", "
                           << few << " ns per observer of 8, " << many << " ns per observer of 512" );
                ASSERT_TRUE( allDelivered );

                // the sanitizers add a cost per memory access that grows with the size of the subscription maps
                #if defined( __SANITIZE_THREAD__ ) || defined( __SANITIZE_ADDRESS__ )
                ASSERT_TRUE( many < 3.0 * few );
                #else
                ASSERT_TRUE( many < 2.0 * few );
                #endif
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;
    }

    GUCEF_TESTFW_TESTCASE( "Delivering an event allocates nothing once the notifier delivered before, for 8 and for 512 observers" )
        GUCEF_TESTFW_SKIP_IF( !CORETEST::IsAllocationCountingAvailable(), CORETEST::GetAllocationCountingUnavailableReason() );
        try
        {
            const UInt32 counts[] = { 8, 512 };
            for ( UInt32 kind=0; kind<2; ++kind )
            {
                for ( UInt32 c=0; c<2; ++c )
                {
                    UInt32 allocations = CountDeliveryAllocations( counts[ c ], 1 == kind );
                    GUCEF_LOG( LOGLEVEL_IMPORTANT, "NotificationCost: " << allocations << " allocations in 200 deliveries to " << counts[ c ]
                               << ( 1 == kind ? " observers of all events" : " observers of the event" ) );
                    ASSERT_TRUE( 0 == allocations );
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
