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
	const CMyObserver& operator=(const CMyObserver&);
	CMyObserver(const CMyObserver&);

public:
    
    CMyObserver( UInt32 index       ,
                 CEvent* eventCache )
        : m_index( index )           ,
          m_eventCache( eventCache )
    {

    }
    
    virtual ~CMyObserver()
    {
        printf( "This is the end,.. observer %d signing off\n", m_index );
    }
    
    virtual void OnMyCallback( CNotifier* notifier           ,
                               const CEvent& eventid         ,
                               CICloneable* eventdata = NULL )
    {
        printf( "This is observer %d calling: I have received an event in my custom callback\n", m_index );
        m_eventCache[ m_index ] = eventid;
    }    

    protected:

    virtual void OnNotify( CNotifier* notifier           ,
                           const CEvent& eventid         ,
                           CICloneable* eventdata = NULL )
    {
        
        if ( eventid == CNotifier::DestructionEvent )
        {  
            printf( "This is observer %d calling: My notifier is beeing destroyed\n", m_index );
            m_eventCache[ m_index ] = eventid;

            // We unsubscribe to explicitly indicate that we are terminating the subscription now
            // This is not manditory, it can happen automaticly at the lowest level
            UnsubscribeFrom( *notifier );
        }
        else
        if ( eventid == CNotifier::ModifyEvent )
        {
            printf( "This is observer %d calling: My notifier has been modified\n", m_index );
            m_eventCache[ m_index ] = eventid;    
        }
        else
        if ( eventid == CNotifier::SubscribeEvent )
        {
            printf( "This is observer %d calling: I have been subscribed to a notifier\n", m_index );
            m_eventCache[ m_index ] = eventid;    
        }
        else
        if ( eventid == CNotifier::UnsubscribeEvent )
        {
            printf( "This is observer %d calling: I have been unsubscribed from a notifier\n", m_index );
            m_eventCache[ m_index ] = eventid;    
        }
        else
        {
            printf( "This is observer %d calling: Recieved event \"%s\" with id %d\n", m_index, CCoreGlobal::Instance()->GetNotificationIDRegistry().Lookup(eventid).C_String(), eventid.GetID() );
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
	const CMyNotifier& operator=(const CMyNotifier&);
	CMyNotifier(const CMyNotifier&);

public:
    CMyNotifier() {}

    virtual ~CMyNotifier()
    {
        printf( "Notifier destruction imminent\n" );
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
    
    std::string GetTypeName( void ) const
    {
        return "XtraBase::CMyNotifier";   
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

    GUCEF_TESTFW_TESTCASE( "Test: Notifier-Observer operations" )
    try
    {         
        /*
         *  Try to create unlinked notifier and observer objects
         */
        
        CMyNotifier notifierA, *notifierB;
        CMyObserver* observers[ 100 ];
        CEvent observerECache[ 100 ];
        SETARRAYMEM( observerECache, 100, CEvent() );
        CNotificationIDRegistry* registry = &CCoreGlobal::Instance()->GetNotificationIDRegistry();
        
        /*
         *  The default notifier events should now be registered
         *  The module load code should call RegisterEvents() on the classes
         *  in addition the same should occur for notifierA's constructor
         */
        if ( !registry->IsRegistered( CNotifier::ModifyEvent.GetName() ) )
        {
            ERRORHERE;
        }
        CEvent modifyEvent = registry->Lookup( CNotifier::ModifyEvent.GetName() );
        if ( !modifyEvent.IsInitialized() )
        {
            ERRORHERE;
        }
        if ( !registry->IsRegistered( CNotifier::DestructionEvent.GetName() ) )
        {
            ERRORHERE;
        }
        CEvent destructionEvent = registry->Lookup( CNotifier::DestructionEvent.GetName() );
        if ( !destructionEvent.IsInitialized() )
        {
            ERRORHERE;
        }
        if ( !registry->IsRegistered( CNotifier::SubscribeEvent.GetName() ) )
        {
            ERRORHERE;
        }
        CEvent subscribeEvent = registry->Lookup( CNotifier::SubscribeEvent.GetName() );
        if ( !subscribeEvent.IsInitialized() )
        {
            ERRORHERE;
        }
        if ( !registry->IsRegistered( CNotifier::UnsubscribeEvent.GetName() ) )
        {
            ERRORHERE;
        }
        CEvent unsubscribeEvent = registry->Lookup( CNotifier::UnsubscribeEvent.GetName() );
        if ( !unsubscribeEvent.IsInitialized() )
        {
            ERRORHERE;
        }
        
        /*
         *  Create our observers
         */
        UInt32 i;       
        for ( i=0; i<100; ++i )
        {
            observers[ i ] = new CMyObserver( i, observerECache );
            if ( observers[ i ] == NULL )
            {
                ERRORHERE;
            }   
        }
            
        
        printf( "Starting notifier modify event test\n" );
        /*
         *  First we will attempt a simple modification setup
         */
        for ( i=0; i<100; ++i )
        {
            notifierA.Subscribe( observers[ i ] );

            if ( observers[ i ]->GetNotifierCount() != 1 )
            {
                ERRORHERE;
            }
            if ( observers[ i ]->GetSubscriptionCount() != 1 )
            {
                ERRORHERE;
            }
        }
        notifierA.DoNotifyObservers();
        
        /*
         *  Now check to make sure all observers got the modify event
         */
        for ( i=0; i<100; ++i )
        {
            if ( observerECache[ i ] != modifyEvent )
            {
                ERRORHERE;
            }
        }
        printf( "Completed notifier modify event test\n" );
        printf( "Starting multi-notifier modify event test\n" );
        
        /*
         *  Now create notifier B and link half of the observers to it 
         */
        SETARRAYMEM( observerECache, 100, CEvent() );
        notifierB = new CMyNotifier();
        if ( notifierB == NULL )
        {
            ERRORHERE;
        }          
        for ( i=0; i<45; ++i )
        {
            notifierB->Subscribe( observers[ i ] );
            if ( observers[ i ]->GetNotifierCount() != 2 )
            {
                ERRORHERE;
            }
            if ( observers[ i ]->GetSubscriptionCount() != 2 )
            {
                ERRORHERE;
            }
        }
        for ( i=0; i<5; ++i )
        {
            notifierB->Unsubscribe( observers[ i ] );        
        }
        for ( i=0; i<5; ++i )
        {
            if ( observerECache[ i ] != unsubscribeEvent )
            {
                ERRORHERE;
            }
        } 
        for ( i=5; i<45; ++i )
        {
            if ( observerECache[ i ] != subscribeEvent )
            {
                ERRORHERE;
            }
        } 
        
        /*
         *  Check whether some recieve the modify event while the others
         *  do not.
         */
        SETARRAYMEM( observerECache, 100, CEvent() ); 
        notifierB->DoNotifyObservers();
        for ( i=5; i<45; ++i )
        {
            if ( observerECache[ i ] != modifyEvent )
            {
                ERRORHERE;
            }
        } 
        for ( i=45; i<100; ++i )
        {
            // Check if the event objects are still in their uninitialized state
            if ( observerECache[ i ].IsInitialized() )
            {
                ERRORHERE;
            }
        }
        printf( "Completed multi-notifier modify event test\n" );
        printf( "Starting notifier destruction test\n" );
        
        /*
         *  Check if deleting the notifier doesn't cause any problems,..
         *  observers should be notified. First the observer should recieve
         *  a destructionEvent and then later you recieve an unsubscribe event
         */ 
        SETARRAYMEM( observerECache, 100, CEvent() );               
        delete notifierB;
        for ( i=5; i<45; ++i )
        {
            if ( observerECache[ i ] != unsubscribeEvent )
            {
                ERRORHERE;
            }
        } 
        for ( i=45; i<100; ++i )
        {
            if ( observerECache[ i ].IsInitialized() )
            {
                // no change should have occured on this observer
                ERRORHERE;
            }
        }

        printf( "Completed notifier destruction test\n" );
        printf( "Performing custom event test\n" );

        /*
         *  Now we will test using custom events
         */
        CEvent myEventA = registry->Register( "TestApp::myEventA" ); 
        CEvent myEventB = registry->Register( "TestApp::myEventB" );
        CEvent myEventC = registry->Register( "TestApp::myEventC" );
        
        printf( "Registered test events: TestApp::myEventA(%d) & TestApp::myEventB(%d)\n", myEventA.GetID(), myEventB.GetID() );
        
        notifierB = new CMyNotifier();
        if ( notifierB == NULL )
        {
            ERRORHERE;
        }          
        for ( i=30; i<100; ++i )
        {
            notifierB->Subscribe( observers[ i ] );

            if ( observers[ i ]->GetNotifierCount() != 2 )
            {
                printf( "ERROR: expected a notifier count of 2 and I found %d\n", observers[ i ]->GetNotifierCount() );
                ERRORHERE;
            }
            if ( observers[ i ]->GetSubscriptionCount() != 2 )
            {
                ERRORHERE;
            }
        }
        
        /*
         *  We will perform two event sends.
         *  It should result in the following cache status:
         *      0-30 should have myEventA
         *      30-100 should have myEventB (because b is sent after a)
         */
        notifierA.DoNotifyObservers( myEventA );       
        notifierB->DoNotifyObservers( myEventB );
        
        for ( i=0; i<30; ++i )
        {
            if ( observerECache[ i ] != myEventA )
            {
                ERRORHERE;
            }
        } 
        for ( i=30; i<100; ++i )
        {
            if ( observerECache[ i ] != myEventB )
            {
                ERRORHERE;
            }
        }

        /*
         *  Now instead of having the observers unsubscribe to ALL events
         *  we will subscribe to our events specificly and then perform the same check again.
         *  First we must unsubscribe the observers to perform the test
         */
        for ( i=0; i<100; ++i )
        {
            observers[ i ]->UnsubscribeAllFromObserver();
            if ( observers[ i ]->GetNotifierCount() != 0 )
            {
                ERRORHERE;
            }
            if ( observers[ i ]->GetSubscriptionCount() != 0 )
            {
                ERRORHERE;
            }
        }
        
        /*
         *  Now we subscribe the observers specificly to our custom events
         *  You automaticly also subscribe to the standard
         */
        for ( i=0; i<30; ++i )
        {
            notifierA.Subscribe( observers[ i ], myEventA );

            if ( observers[ i ]->GetNotifierCount() != 1 )
            {
                ERRORHERE;
            }
            if ( observers[ i ]->GetSubscriptionCount() != 2 )
            {
                ERRORHERE;
            }
        }
        for ( i=30; i<100; ++i )
        {        
            notifierA.Subscribe( observers[ i ], myEventA );
            notifierB->Subscribe( observers[ i ], myEventB );

            if ( observers[ i ]->GetNotifierCount() != 2 )
            {
                ERRORHERE;
            }
            if ( observers[ i ]->GetSubscriptionCount() != 4 )
            {
                ERRORHERE;
            }
        }

        /*
         *  We will perform two event sends.
         *  It should result in the following cache status:
         *      0-30 should have myEventA, they are not subscribed to eventC and thus should not have recieved it
         *      30-100 should have myEventB because B is sent after A, overwiting it.
         */                               
        notifierA.DoNotifyObservers( myEventA );
        notifierA.DoNotifyObservers( myEventC );
        notifierB->DoNotifyObservers( myEventB );
        
        for ( i=0; i<30; ++i )
        {
            if ( observerECache[ i ] != myEventA )
            {
                ERRORHERE;
            }
        } 
        for ( i=30; i<100; ++i )
        {
            if ( observerECache[ i ] != myEventB )
            {
                ERRORHERE;
            }
        }
        notifierA.UnsubscribeAllFromNotifier();
        notifierB->UnsubscribeAllFromNotifier();
        
        printf( "Completed custom event test\n" );     
        printf( "Performing custom event handler callback test\n" );
        
        // Setup subscriptions for the test
        for ( i=0; i<100; ++i )
        {
            // Clear the test output storage
            observerECache[ i ] = CEvent();
            
            // Setup the callback
            CTEventHandlerFunctor< CMyObserver > callback( observers[ i ], &CMyObserver::OnMyCallback );
            observers[ i ]->UnsubscribeAllFromObserver();
            notifierA.Subscribe( observers[ i ], myEventB, &callback );
        }
        
        // Send the test event
        notifierA.DoNotifyObservers( myEventB );
        
        // Check whether it was processed correctly
        for ( i=0; i<100; ++i )
        {
            if ( observerECache[ i ] != myEventB )
            {
                // For some reason the output is not as expected
                ERRORHERE;
            }
        }        

        printf( "Completed custom event handler callback test\n" );
        
        /*
         *  Cleanup our toys
         */
        printf( "Commencing cleanup\n" );
        delete notifierB;
        for ( i=0; i<100; ++i )
        {
            delete observers[ i ];
        }
        
        int a =1;                

    }
    catch ( const char* )
    {
        ERRORHERE;
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
