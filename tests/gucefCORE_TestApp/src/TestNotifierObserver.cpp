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

#include <string.h>

#ifndef GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H
#include "CNotificationIDRegistry.h"
#define GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H
#endif /* GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H ? */

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestNotifierObserver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

#define SETARRAYMEM( ptr, max, value )    \
    {                                     \
        int x;                            \
        for ( x=0; x<max; ++x )           \
        {                                 \
            ptr[ x ] = value;             \
        }                                 \
    }

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CMyObserver : public CObserver
{
private:
    const CMyObserver& operator=( const CMyObserver& );
    CMyObserver( const CMyObserver& );

public:

    CMyObserver( UInt32 index       ,
                 CEvent* eventCache )
        : m_index( index )           ,
          m_eventCache( eventCache )
    {
    }

    virtual ~CMyObserver()
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Observer destruction" );
    }

    virtual void OnMyCallback( CNotifier* notifier           ,
                               const CEvent& eventid         ,
                               CICloneable* eventdata = NULL )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Observer received event in custom callback" );
        m_eventCache[ m_index ] = eventid;
    }

    protected:

    virtual void OnNotify( CNotifier* notifier           ,
                           const CEvent& eventid         ,
                           CICloneable* eventdata = NULL )
    {

        if ( eventid == CNotifier::DestructionEvent )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Observer notifier is being destroyed" );
            m_eventCache[ m_index ] = eventid;

            // We unsubscribe to explicitly indicate that we are terminating the subscription now
            // This is not mandatory, it can happen automatically at the lowest level
            UnsubscribeFrom( *notifier );
        }
        else
        if ( eventid == CNotifier::ModifyEvent )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Observer notifier has been modified" );
            m_eventCache[ m_index ] = eventid;
        }
        else
        if ( eventid == CNotifier::SubscribeEvent )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Observer has been subscribed to a notifier" );
            m_eventCache[ m_index ] = eventid;
        }
        else
        if ( eventid == CNotifier::UnsubscribeEvent )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Observer has been unsubscribed from a notifier" );
            m_eventCache[ m_index ] = eventid;
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, CString( "Observer received custom event: " ) +
                CCoreGlobal::Instance()->GetNotificationIDRegistry().Lookup( eventid ) );
            m_eventCache[ m_index ] = eventid;
        }
    }

    private:
    CEvent* m_eventCache;
    UInt32 m_index;

    private:

    CMyObserver( void );
};

/*-------------------------------------------------------------------------*/

class CMyNotifier : public CNotifier
{
private:
    const CMyNotifier& operator=( const CMyNotifier& );
    CMyNotifier( const CMyNotifier& );

public:
    CMyNotifier() {}

    virtual ~CMyNotifier()
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Notifier destruction imminent" );
    }

    void DoNotifyObservers( void )
    {
        NotifyObservers();
    }

    void DoNotifyObservers( const CEvent& eventid         ,
                            CICloneable* eventData = NULL )
    {
        NotifyObservers( eventid   ,
                         eventData );
    }

    bool DoNotifyObserversWithResult( void )
    {
        return NotifyObservers();
    }

    std::string GetTypeName( void ) const
    {
        return "XtraBase::CMyNotifier";
    }
};

/*-------------------------------------------------------------------------*/

class CScheduleDestructionObserver : public CObserver
{
private:
    const CScheduleDestructionObserver& operator=( const CScheduleDestructionObserver& );
    CScheduleDestructionObserver( const CScheduleDestructionObserver& );

public:

    CScheduleDestructionObserver()
        : m_notifierToDestroy( GUCEF_NULL )
    {
    }

    virtual ~CScheduleDestructionObserver()
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CScheduleDestructionObserver destruction" );
    }

    CNotifier* m_notifierToDestroy;

protected:

    virtual void OnNotify( CNotifier* notifier           ,
                           const CEvent& eventid         ,
                           CICloneable* eventdata = NULL )
    {
        if ( eventid == CNotifier::ModifyEvent && GUCEF_NULL != m_notifierToDestroy )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Observer: scheduling notifier for destruction" );
            m_notifierToDestroy->ScheduleForDestruction();
            m_notifierToDestroy = GUCEF_NULL;
        }
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformNotifierObserverTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING NOTIFIER-OBSERVER TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "NotifierObserver" );

    /*-----------------------------------------------------------------------*/
    /* Test 1: Standard event registration                                   */
    /*-----------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "Standard event registration" )
    try
    {
        CNotificationIDRegistry* registry = &CCoreGlobal::Instance()->GetNotificationIDRegistry();

        ASSERT_TRUE( registry->IsRegistered( CNotifier::ModifyEvent.GetName() ) );
        CEvent modifyEvent = registry->Lookup( CNotifier::ModifyEvent.GetName() );
        ASSERT_TRUE( modifyEvent.IsInitialized() );

        ASSERT_TRUE( registry->IsRegistered( CNotifier::DestructionEvent.GetName() ) );
        CEvent destructionEvent = registry->Lookup( CNotifier::DestructionEvent.GetName() );
        ASSERT_TRUE( destructionEvent.IsInitialized() );

        ASSERT_TRUE( registry->IsRegistered( CNotifier::SubscribeEvent.GetName() ) );
        CEvent subscribeEvent = registry->Lookup( CNotifier::SubscribeEvent.GetName() );
        ASSERT_TRUE( subscribeEvent.IsInitialized() );

        ASSERT_TRUE( registry->IsRegistered( CNotifier::UnsubscribeEvent.GetName() ) );
        CEvent unsubscribeEvent = registry->Lookup( CNotifier::UnsubscribeEvent.GetName() );
        ASSERT_TRUE( unsubscribeEvent.IsInitialized() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*-----------------------------------------------------------------------*/
    /* Test 2: Subscription count semantics                                  */
    /*-----------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "Subscription count semantics" )
    try
    {
        CNotificationIDRegistry* registry = &CCoreGlobal::Instance()->GetNotificationIDRegistry();
        CEvent myEventA = registry->Register( "TestApp::myEventA" );
        CEvent myEventB = registry->Register( "TestApp::myEventB" );

        CEvent observerECache[ 3 ];
        SETARRAYMEM( observerECache, 3, CEvent() );
        CMyObserver* observers[ 3 ];
        for ( UInt32 i = 0; i < 3; ++i )
            observers[ i ] = new CMyObserver( i, observerECache );

        CMyNotifier notifierA;
        CMyNotifier* notifierB = new CMyNotifier();

        // observer[0]: subscribe-all to one notifier (4 mandatory standard events)
        notifierA.Subscribe( observers[ 0 ] );
        ASSERT_TRUE( observers[ 0 ]->GetNotifierCount() == 1 );
        ASSERT_TRUE( observers[ 0 ]->GetSubscriptionCount() == 4 );

        // observer[0]: also subscribe-all to second notifier (4 + 4 = 8)
        notifierB->Subscribe( observers[ 0 ] );
        ASSERT_TRUE( observers[ 0 ]->GetNotifierCount() == 2 );
        ASSERT_TRUE( observers[ 0 ]->GetSubscriptionCount() == 8 );

        // observer[0]: unsubscribe from second notifier
        notifierB->Unsubscribe( observers[ 0 ] );
        ASSERT_TRUE( observers[ 0 ]->GetNotifierCount() == 1 );
        ASSERT_TRUE( observers[ 0 ]->GetSubscriptionCount() == 4 );

        // observer[0]: unsubscribe from all
        observers[ 0 ]->UnsubscribeAllFromObserver();
        ASSERT_TRUE( observers[ 0 ]->GetNotifierCount() == 0 );
        ASSERT_TRUE( observers[ 0 ]->GetSubscriptionCount() == 0 );

        // observer[1]: specific-event subscription to one notifier
        // Without a callback the observer is NOT written into m_eventobservers, so count = 4 (base link only)
        notifierA.Subscribe( observers[ 1 ], myEventA );
        ASSERT_TRUE( observers[ 1 ]->GetNotifierCount() == 1 );
        ASSERT_TRUE( observers[ 1 ]->GetSubscriptionCount() == 4 );

        // observer[1]: also specific-event subscription to second notifier (4 + 4 = 8)
        notifierB->Subscribe( observers[ 1 ], myEventB );
        ASSERT_TRUE( observers[ 1 ]->GetNotifierCount() == 2 );
        ASSERT_TRUE( observers[ 1 ]->GetSubscriptionCount() == 8 );

        // observer[2]: subscribe then unsubscribe-all
        notifierA.Subscribe( observers[ 2 ] );
        observers[ 2 ]->UnsubscribeAllFromObserver();
        ASSERT_TRUE( observers[ 2 ]->GetNotifierCount() == 0 );
        ASSERT_TRUE( observers[ 2 ]->GetSubscriptionCount() == 0 );

        delete notifierB;
        for ( UInt32 i = 0; i < 3; ++i )
            delete observers[ i ];
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*-----------------------------------------------------------------------*/
    /* Test 3: ModifyEvent broadcast delivery                                */
    /*-----------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "ModifyEvent broadcast delivery" )
    try
    {
        CNotificationIDRegistry* registry = &CCoreGlobal::Instance()->GetNotificationIDRegistry();
        CEvent modifyEvent = registry->Lookup( CNotifier::ModifyEvent.GetName() );

        CEvent observerECache[ 100 ];
        SETARRAYMEM( observerECache, 100, CEvent() );
        CMyObserver* observers[ 100 ];
        for ( UInt32 i = 0; i < 100; ++i )
            observers[ i ] = new CMyObserver( i, observerECache );

        CMyNotifier notifierA;

        for ( UInt32 i = 0; i < 100; ++i )
            notifierA.Subscribe( observers[ i ] );

        SETARRAYMEM( observerECache, 100, CEvent() );
        notifierA.DoNotifyObservers();

        for ( UInt32 i = 0; i < 100; ++i )
        {
            ASSERT_TRUE( observerECache[ i ] == modifyEvent );
        }

        for ( UInt32 i = 0; i < 100; ++i )
            delete observers[ i ];
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*-----------------------------------------------------------------------*/
    /* Test 4: Multi-notifier routing                                        */
    /*-----------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "Multi-notifier routing" )
    try
    {
        CNotificationIDRegistry* registry = &CCoreGlobal::Instance()->GetNotificationIDRegistry();
        CEvent modifyEvent = registry->Lookup( CNotifier::ModifyEvent.GetName() );

        CEvent observerECache[ 100 ];
        SETARRAYMEM( observerECache, 100, CEvent() );
        CMyObserver* observers[ 100 ];
        for ( UInt32 i = 0; i < 100; ++i )
            observers[ i ] = new CMyObserver( i, observerECache );

        CMyNotifier notifierA;
        CMyNotifier* notifierB = new CMyNotifier();

        for ( UInt32 i = 0; i < 100; ++i )
            notifierA.Subscribe( observers[ i ] );

        // Subscribe observers[0..44] to notifierB, then unsubscribe 0..4
        for ( UInt32 i = 0; i < 45; ++i )
            notifierB->Subscribe( observers[ i ] );

        for ( UInt32 i = 0; i < 5; ++i )
            notifierB->Unsubscribe( observers[ i ] );

        // observers[5..44] should be on both notifiers (4 + 4 = 8 subscriptions)
        for ( UInt32 i = 5; i < 45; ++i )
        {
            ASSERT_TRUE( observers[ i ]->GetNotifierCount() == 2 );
            ASSERT_TRUE( observers[ i ]->GetSubscriptionCount() == 8 );
        }
        // observers[0..4] should be on notifierA only (unsubscribed from notifierB)
        for ( UInt32 i = 0; i < 5; ++i )
        {
            ASSERT_TRUE( observers[ i ]->GetNotifierCount() == 1 );
        }

        SETARRAYMEM( observerECache, 100, CEvent() );
        notifierB->DoNotifyObservers();

        // Only observers[5..44] subscribed to notifierB should receive ModifyEvent
        for ( UInt32 i = 5; i < 45; ++i )
        {
            ASSERT_TRUE( observerECache[ i ] == modifyEvent );
        }
        // observers[0..4] unsubscribed from notifierB — cache should remain uninitialized
        for ( UInt32 i = 0; i < 5; ++i )
        {
            ASSERT_FALSE( observerECache[ i ].IsInitialized() );
        }
        // observers[45..99] never subscribed to notifierB — cache should remain uninitialized
        for ( UInt32 i = 45; i < 100; ++i )
        {
            ASSERT_FALSE( observerECache[ i ].IsInitialized() );
        }

        delete notifierB;
        for ( UInt32 i = 0; i < 100; ++i )
            delete observers[ i ];
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*-----------------------------------------------------------------------*/
    /* Test 5: Notifier destruction observer notification                    */
    /*-----------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "Notifier destruction observer notification" )
    try
    {
        CNotificationIDRegistry* registry = &CCoreGlobal::Instance()->GetNotificationIDRegistry();
        CEvent unsubscribeEvent = registry->Lookup( CNotifier::UnsubscribeEvent.GetName() );

        CEvent observerECache[ 100 ];
        SETARRAYMEM( observerECache, 100, CEvent() );
        CMyObserver* observers[ 100 ];
        for ( UInt32 i = 0; i < 100; ++i )
            observers[ i ] = new CMyObserver( i, observerECache );

        CMyNotifier* notifierB = new CMyNotifier();

        // Subscribe only observers[5..44] to notifierB
        for ( UInt32 i = 5; i < 45; ++i )
            notifierB->Subscribe( observers[ i ] );

        SETARRAYMEM( observerECache, 100, CEvent() );
        delete notifierB;

        // The observer's OnNotify for DestructionEvent calls UnsubscribeFrom(*notifier),
        // so the last event cached is UnsubscribeEvent (fired by UnsubscribeFrom)
        for ( UInt32 i = 5; i < 45; ++i )
        {
            ASSERT_TRUE( observerECache[ i ] == unsubscribeEvent );
        }
        // observers[0..4] were not on notifierB — no change
        for ( UInt32 i = 0; i < 5; ++i )
        {
            ASSERT_FALSE( observerECache[ i ].IsInitialized() );
        }
        // observers[45..99] were not on notifierB — no change
        for ( UInt32 i = 45; i < 100; ++i )
        {
            ASSERT_FALSE( observerECache[ i ].IsInitialized() );
        }

        for ( UInt32 i = 0; i < 100; ++i )
            delete observers[ i ];
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*-----------------------------------------------------------------------*/
    /* Test 6: Specific event subscription — selective delivery              */
    /*-----------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "Specific event subscription selective delivery" )
    try
    {
        // Subscribe(observer, specificEvent, callback) adds the observer to m_eventobservers
        // (Phase 1 dispatch) so only that event triggers the callback.  Without a callback
        // the observer is NOT placed in m_eventobservers and receives nothing for that event.
        // Selective delivery therefore requires callbacks.
        //
        // Subscription count with callback: 4 (base link) + 1 per callback entry = 5 per notifier.

        CNotificationIDRegistry* registry = &CCoreGlobal::Instance()->GetNotificationIDRegistry();
        CEvent myEventA = registry->Register( "TestApp::myEventA", true );
        CEvent myEventB = registry->Register( "TestApp::myEventB", true );
        CEvent myEventC = registry->Register( "TestApp::myEventC", true );

        CEvent observerECache[ 100 ];
        SETARRAYMEM( observerECache, 100, CEvent() );
        CMyObserver* observers[ 100 ];
        for ( UInt32 i = 0; i < 100; ++i )
            observers[ i ] = new CMyObserver( i, observerECache );

        CMyNotifier notifierA;
        CMyNotifier* notifierB = new CMyNotifier();

        // observers[0..29]: subscribe to notifierA for myEventA via callback (4 base + 1 = 5)
        for ( UInt32 i = 0; i < 30; ++i )
        {
            CTEventHandlerFunctor< CMyObserver > cbA( observers[ i ], &CMyObserver::OnMyCallback );
            notifierA.Subscribe( observers[ i ], myEventA, &cbA );
            ASSERT_TRUE( observers[ i ]->GetNotifierCount() == 1 );
            ASSERT_TRUE( observers[ i ]->GetSubscriptionCount() == 5 );
        }

        // observers[30..99]: subscribe to notifierA (myEventA) and notifierB (myEventB) via callbacks
        // 2 notifiers × 5 = 10
        for ( UInt32 i = 30; i < 100; ++i )
        {
            CTEventHandlerFunctor< CMyObserver > cbA( observers[ i ], &CMyObserver::OnMyCallback );
            notifierA.Subscribe( observers[ i ], myEventA, &cbA );
            CTEventHandlerFunctor< CMyObserver > cbB( observers[ i ], &CMyObserver::OnMyCallback );
            notifierB->Subscribe( observers[ i ], myEventB, &cbB );
            ASSERT_TRUE( observers[ i ]->GetNotifierCount() == 2 );
            ASSERT_TRUE( observers[ i ]->GetSubscriptionCount() == 10 );
        }

        // Clear the SubscribeEvent entries placed in the cache during the subscribe loop above
        SETARRAYMEM( observerECache, 100, CEvent() );

        // Fire myEventA — all 100 callbacks receive it (Phase 1 via m_eventobservers)
        notifierA.DoNotifyObservers( myEventA );
        // Fire myEventC — no one has a callback for it; cache unchanged
        notifierA.DoNotifyObservers( myEventC );
        // Fire myEventB — callbacks for observers[30..99] overwrite their cache
        notifierB->DoNotifyObservers( myEventB );

        // observers[0..29]: got myEventA, didn't get myEventC (no callback), didn't get myEventB (not on notifierB)
        for ( UInt32 i = 0; i < 30; ++i )
        {
            ASSERT_TRUE( observerECache[ i ] == myEventA );
        }
        // observers[30..99]: myEventB overwrote myEventA (fired after)
        for ( UInt32 i = 30; i < 100; ++i )
        {
            ASSERT_TRUE( observerECache[ i ] == myEventB );
        }

        delete notifierB;
        for ( UInt32 i = 0; i < 100; ++i )
            delete observers[ i ];
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*-----------------------------------------------------------------------*/
    /* Test 7: UnsubscribeAll clears all subscriptions                       */
    /*-----------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "UnsubscribeAll clears all subscriptions" )
    try
    {
        CEvent observerECache[ 10 ];
        SETARRAYMEM( observerECache, 10, CEvent() );
        CMyObserver* observers[ 10 ];
        for ( UInt32 i = 0; i < 10; ++i )
            observers[ i ] = new CMyObserver( i, observerECache );

        CMyNotifier notifierA;

        for ( UInt32 i = 0; i < 10; ++i )
        {
            notifierA.Subscribe( observers[ i ] );
            ASSERT_TRUE( observers[ i ]->GetNotifierCount() == 1 );
            ASSERT_TRUE( observers[ i ]->GetSubscriptionCount() == 4 );
        }

        for ( UInt32 i = 0; i < 10; ++i )
        {
            observers[ i ]->UnsubscribeAllFromObserver();
            ASSERT_TRUE( observers[ i ]->GetNotifierCount() == 0 );
            ASSERT_TRUE( observers[ i ]->GetSubscriptionCount() == 0 );
        }

        for ( UInt32 i = 0; i < 10; ++i )
            delete observers[ i ];
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*-----------------------------------------------------------------------*/
    /* Test 8: Functor callback dispatch                                     */
    /*-----------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "Functor callback dispatch" )
    try
    {
        CNotificationIDRegistry* registry = &CCoreGlobal::Instance()->GetNotificationIDRegistry();
        CEvent myEventB = registry->Register( "TestApp::myEventB", true );

        CEvent observerECache[ 10 ];
        SETARRAYMEM( observerECache, 10, CEvent() );
        CMyObserver* observers[ 10 ];
        for ( UInt32 i = 0; i < 10; ++i )
            observers[ i ] = new CMyObserver( i, observerECache );

        CMyNotifier notifierA;

        for ( UInt32 i = 0; i < 10; ++i )
        {
            CTEventHandlerFunctor< CMyObserver > callback( observers[ i ], &CMyObserver::OnMyCallback );
            observers[ i ]->UnsubscribeAllFromObserver();
            notifierA.Subscribe( observers[ i ], myEventB, &callback );
        }

        notifierA.DoNotifyObservers( myEventB );

        for ( UInt32 i = 0; i < 10; ++i )
        {
            ASSERT_TRUE( observerECache[ i ] == myEventB );
        }

        for ( UInt32 i = 0; i < 10; ++i )
            delete observers[ i ];
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*-----------------------------------------------------------------------*/
    /* Test 9: SubscribeEvent and UnsubscribeEvent delivery                  */
    /*-----------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "SubscribeEvent and UnsubscribeEvent delivery" )
    try
    {
        // The notifier fires SubscribeEvent directly to an observer when it first links,
        // and UnsubscribeEvent when the link is removed.  Both events bypass the normal
        // Phase 1/2 dispatch and are delivered unconditionally via OnNotify.

        CNotificationIDRegistry* registry = &CCoreGlobal::Instance()->GetNotificationIDRegistry();
        CEvent subscribeEvent   = registry->Lookup( CNotifier::SubscribeEvent.GetName() );
        CEvent unsubscribeEvent = registry->Lookup( CNotifier::UnsubscribeEvent.GetName() );

        CEvent observerECache[ 1 ];
        SETARRAYMEM( observerECache, 1, CEvent() );
        CMyObserver* observer = new CMyObserver( 0, observerECache );

        CMyNotifier notifierA;

        // Subscribe — LinkObserver fires SubscribeEvent directly to observer
        notifierA.Subscribe( observer );
        ASSERT_TRUE( observerECache[ 0 ] == subscribeEvent );
        ASSERT_TRUE( observer->GetNotifierCount() == 1 );

        // Unsubscribe — UnsubscribeFromAllEvents fires UnsubscribeEvent directly to observer
        notifierA.Unsubscribe( observer );
        ASSERT_TRUE( observerECache[ 0 ] == unsubscribeEvent );
        ASSERT_TRUE( observer->GetNotifierCount() == 0 );

        delete observer;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*-----------------------------------------------------------------------*/
    /* Test 10: Idempotent subscribe-all                                     */
    /*-----------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "Idempotent subscribe-all" )
    try
    {
        // Calling Subscribe(observer) twice on the same notifier must be a no-op
        // on the second call: observer count stays 1, subscription count stays 4,
        // and the notification fires once (not twice).

        CNotificationIDRegistry* registry = &CCoreGlobal::Instance()->GetNotificationIDRegistry();
        CEvent modifyEvent = registry->Lookup( CNotifier::ModifyEvent.GetName() );

        CEvent observerECache[ 1 ];
        SETARRAYMEM( observerECache, 1, CEvent() );
        CMyObserver* observer = new CMyObserver( 0, observerECache );

        CMyNotifier notifierA;

        notifierA.Subscribe( observer );
        notifierA.Subscribe( observer );  // second call — must be a no-op

        ASSERT_TRUE( observer->GetNotifierCount() == 1 );
        ASSERT_TRUE( observer->GetSubscriptionCount() == 4 );

        SETARRAYMEM( observerECache, 1, CEvent() );
        notifierA.DoNotifyObservers();

        ASSERT_TRUE( observerECache[ 0 ] == modifyEvent );

        delete observer;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*-----------------------------------------------------------------------*/
    /* Test 11: Re-subscribe after explicit unsubscribe                      */
    /*-----------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "Re-subscribe after explicit unsubscribe" )
    try
    {
        // An observer that has been explicitly unsubscribed can rejoin the notifier
        // cleanly: counts return to normal and events are delivered again.

        CNotificationIDRegistry* registry = &CCoreGlobal::Instance()->GetNotificationIDRegistry();
        CEvent modifyEvent = registry->Lookup( CNotifier::ModifyEvent.GetName() );

        CEvent observerECache[ 1 ];
        SETARRAYMEM( observerECache, 1, CEvent() );
        CMyObserver* observer = new CMyObserver( 0, observerECache );

        CMyNotifier notifierA;

        // First subscription
        notifierA.Subscribe( observer );
        ASSERT_TRUE( observer->GetNotifierCount() == 1 );
        ASSERT_TRUE( observer->GetSubscriptionCount() == 4 );

        // Unsubscribe
        observer->UnsubscribeAllFromObserver();
        ASSERT_TRUE( observer->GetNotifierCount() == 0 );
        ASSERT_TRUE( observer->GetSubscriptionCount() == 0 );

        // Re-subscribe — should behave identically to the first subscription
        notifierA.Subscribe( observer );
        ASSERT_TRUE( observer->GetNotifierCount() == 1 );
        ASSERT_TRUE( observer->GetSubscriptionCount() == 4 );

        SETARRAYMEM( observerECache, 1, CEvent() );
        notifierA.DoNotifyObservers();

        ASSERT_TRUE( observerECache[ 0 ] == modifyEvent );

        delete observer;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*-----------------------------------------------------------------------*/
    /* Test 12: UnsubscribeAllFromNotifier                                   */
    /*-----------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "UnsubscribeAllFromNotifier" )
    try
    {
        // notifierA.UnsubscribeAllFromNotifier() must remove every observer from
        // the notifier, resetting their counts to zero.  Subsequent fires must be
        // no-ops (no crash, no cache changes).

        CEvent observerECache[ 10 ];
        SETARRAYMEM( observerECache, 10, CEvent() );
        CMyObserver* observers[ 10 ];
        for ( UInt32 i = 0; i < 10; ++i )
            observers[ i ] = new CMyObserver( i, observerECache );

        CMyNotifier notifierA;

        for ( UInt32 i = 0; i < 10; ++i )
            notifierA.Subscribe( observers[ i ] );

        notifierA.UnsubscribeAllFromNotifier();

        for ( UInt32 i = 0; i < 10; ++i )
        {
            ASSERT_TRUE( observers[ i ]->GetNotifierCount() == 0 );
            ASSERT_TRUE( observers[ i ]->GetSubscriptionCount() == 0 );
        }

        // Fire with no subscribers — must not crash and must not change the cache
        SETARRAYMEM( observerECache, 10, CEvent() );
        notifierA.DoNotifyObservers();

        for ( UInt32 i = 0; i < 10; ++i )
        {
            ASSERT_FALSE( observerECache[ i ].IsInitialized() );
        }

        for ( UInt32 i = 0; i < 10; ++i )
            delete observers[ i ];
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*-----------------------------------------------------------------------*/
    /* Test 13: Observer auto-cleanup on destruction                         */
    /*-----------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "Observer auto-cleanup on destruction" )
    try
    {
        // When a subscribed observer is deleted its destructor triggers OnObserverDestroy
        // on every linked notifier, removing it from m_observers without sending
        // UnsubscribeEvent (observerDestruction=true path).  Subsequent notifications
        // must reach only the surviving observers and must not crash.

        CNotificationIDRegistry* registry = &CCoreGlobal::Instance()->GetNotificationIDRegistry();
        CEvent modifyEvent = registry->Lookup( CNotifier::ModifyEvent.GetName() );

        CEvent observerECache[ 5 ];
        SETARRAYMEM( observerECache, 5, CEvent() );
        CMyObserver* observers[ 5 ];
        for ( UInt32 i = 0; i < 5; ++i )
            observers[ i ] = new CMyObserver( i, observerECache );

        CMyNotifier notifierA;

        for ( UInt32 i = 0; i < 5; ++i )
            notifierA.Subscribe( observers[ i ] );

        // Delete observers[0..2] while they are still subscribed.
        // Their destructors call OnObserverDestroy → removes them from notifierA's m_observers.
        delete observers[ 0 ];  observers[ 0 ] = GUCEF_NULL;
        delete observers[ 1 ];  observers[ 1 ] = GUCEF_NULL;
        delete observers[ 2 ];  observers[ 2 ] = GUCEF_NULL;

        // Clear any subscribe/destruction events that accumulated before this point
        SETARRAYMEM( observerECache, 5, CEvent() );

        // Only observers[3] and [4] remain; notification must reach them and not crash
        notifierA.DoNotifyObservers();

        ASSERT_TRUE( observerECache[ 3 ] == modifyEvent );
        ASSERT_TRUE( observerECache[ 4 ] == modifyEvent );

        // Deleted observers can no longer write to the cache
        ASSERT_FALSE( observerECache[ 0 ].IsInitialized() );
        ASSERT_FALSE( observerECache[ 1 ].IsInitialized() );
        ASSERT_FALSE( observerECache[ 2 ].IsInitialized() );

        delete observers[ 3 ];
        delete observers[ 4 ];
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*-----------------------------------------------------------------------*/
    /* Test 14: NotifyObservers returns false when notifier destroyed        */
    /*          during dispatch via ScheduleForDestruction()                 */
    /*-----------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "NotifyObservers returns false when notifier destroyed during dispatch" )
    try
    {
        // When ScheduleForDestruction() is called from within an observer's
        // OnNotify() handler, the notifier is deleted after the current dispatch
        // loop completes.  NotifyObservers() then returns false to signal the
        // caller that the notifier no longer exists.
        //
        // Cleanup note: the scheduled-for-destruction path bypasses
        // OnDeathOfOwnerNotifier, so the observer's m_notifiers back-reference
        // is not cleared automatically.  We call UnlinkFrom() afterwards to
        // remove the (now-dangling) pointer before the observer goes out of scope.
        // UnlinkFrom only erases the pointer value from the set — it does not
        // dereference the deleted notifier — so this is safe.

        CMyNotifier* notifier = new CMyNotifier();

        // INTENTIONAL MEMORY LEAK — do not flag as a leak checker error.
        //
        // The kill observer is heap-allocated and deliberately never deleted.
        // Root cause: ScheduleForDestruction() bypasses OnDeathOfOwnerNotifier,
        // so the notifier never calls UnlinkFrom() on its observers.  After the
        // dispatch returns false, killObserver->m_notifiers still holds a dangling
        // pointer to the deleted notifier.  CObserver::UnlinkFrom() (the only way
        // to clear that pointer) is private / friend-only and cannot be called
        // from test code.  Calling delete on killObserver would cause
        // SignalUpcomingObserverDestruction() to dereference the dangling pointer,
        // which is undefined behaviour.
        //
        // This is a known design constraint of the ScheduleForDestruction() path:
        // observers that remain subscribed when the notifier is destroyed this way
        // are left with a stale back-reference.  The leak is intentional and small;
        // the OS reclaims the memory at process exit.
        //
        // TODO: if the memory-leak checker plugin is ever updated to support
        // suppression annotations, mark this allocation explicitly at that time.
        CScheduleDestructionObserver* killObserver = new CScheduleDestructionObserver();
        killObserver->m_notifierToDestroy = notifier;

        notifier->Subscribe( killObserver );

        // Fires ModifyEvent to killObserver. killObserver calls
        // ScheduleForDestruction() → notifier deleted after dispatch.
        // Return value must be false.
        bool notifierStillAlive = notifier->DoNotifyObserversWithResult();

        ASSERT_FALSE( notifierStillAlive );

        notifier = GUCEF_NULL;
        // killObserver intentionally not deleted — see note above
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL NOTIFIER-OBSERVER TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
