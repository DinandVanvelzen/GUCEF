/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include <assert.h>

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#include "gucefMT_CObjectScopeReadOnlyLock.h"
#define GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#endif /* GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H ? */

#ifndef GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H
#include "CNotificationIDRegistry.h"
#define GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H
#endif /* GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

#include "CNotifierImplementor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CNotifierImplementor::CNotifierImplementor( CNotifier* ownerNotifier )
    : m_isBusy( false )                 ,
      m_observers()                     ,
      m_eventobservers()                ,
      m_eventMailStack()                ,
      m_cmdMailStack()                  ,
      m_ownerNotifier( ownerNotifier )  ,
      m_scheduledForDestruction( false )
{GUCEF_TRACE;

    assert( m_ownerNotifier != GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

CNotifierImplementor::CNotifierImplementor( CNotifier* ownerNotifier ,
                                            const CNotifier& src     )
    : m_isBusy( false )                 ,
      m_observers()                     ,
      m_eventobservers()                ,
      m_eventMailStack()                ,
      m_cmdMailStack()                  ,
      m_ownerNotifier( ownerNotifier )  ,
      m_scheduledForDestruction( false )
{GUCEF_TRACE;

    assert( m_ownerNotifier != GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

CNotifierImplementor&
CNotifierImplementor::operator=( const CNotifierImplementor& src )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

CNotifierImplementor::~CNotifierImplementor()
{GUCEF_TRACE;

    /*
     *  The notifier should no longer be busy with some action if it is being destroyed.
     *  If it is still busy then the code would resume after the object is deleted which
     *  causes undefined behavior possibly resulting in a crash.
     *  If you hit this assert then you have found a bug in the logic of this class
     */
    if ( m_isBusy )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_CRITICAL, "CNotifierImplementor DESTROYED WHILE BUSY! Owner class: " +
            ( m_ownerNotifier != GUCEF_NULL ? m_ownerNotifier->GetClassTypeName() : CString( "NULL" ) ) +
            " ptr=" + PointerToString( m_ownerNotifier ) );
    }
    assert( !m_isBusy );

    UnsubscribeAllFromNotifier( false );
}

/*-------------------------------------------------------------------------*/

CNotifierImplementor*
CNotifierImplementor::Create( CNotifier* ownerNotifier )
{GUCEF_TRACE;

    // perform in-scope creation
    return GUCEF_NEW CNotifierImplementor( ownerNotifier );
}

/*-------------------------------------------------------------------------*/

CNotifierImplementor*
CNotifierImplementor::Create( CNotifier* ownerNotifier ,
                              const CNotifier& src     )
{GUCEF_TRACE;

    // perform in-scope creation
    return GUCEF_NEW CNotifierImplementor( ownerNotifier ,
                                     src           );
}

/*-------------------------------------------------------------------------*/

void
CNotifierImplementor::Destroy( CNotifierImplementor* obj )
{GUCEF_TRACE;

    // perform an in-scope destruction
    GUCEF_DELETE obj;
}

/*-------------------------------------------------------------------------*/

bool
CNotifierImplementor::ForceNotifyObserversOnce( const CEvent& eventid ,
                                                CICloneable* data     )
{GUCEF_TRACE;

    m_isBusy = true;

    CObserver* oPtr = GUCEF_NULL;
    TObserverSet notifiedObservers;

    TObserverList::iterator i = m_observers.begin();
    while ( i != m_observers.end() )
    {
        // Check if we have not already notified this observer
        oPtr = (*i).first;
        if ( notifiedObservers.find( oPtr ) == notifiedObservers.end() )
        {
            GUCEF_DEBUG_LOG_EVERYTHING( "NotifierImplementor(" + CORE::PointerToString( this ) + "): Class " + m_ownerNotifier->GetClassTypeName() + ": Dispatching event \"" + eventid.GetName() + "\" to " + (*i).first->GetClassTypeName() + "(" + CORE::PointerToString( (*i).first ) + ")" );

            (*i).first->OnNotify( m_ownerNotifier ,
                                  eventid         ,
                                  data            );

            // Check if someone deleted our owner notifier
            if ( m_ownerNotifier == GUCEF_NULL )
            {
                // Gracefully handle the destruction sequence
                m_isBusy = false;
                Destroy( this );
                return false;
            }

            // Add the observer to our 'notified' list
            notifiedObservers.insert( oPtr );

            // Process command mail if needed
            if ( !m_cmdMailStack.empty() )
            {
                // We have command mail
                ProcessCmdMailbox();

                // the administration has been altered, we now have no choice
                // but to start from the beginning
                i = m_observers.begin();
                continue;
            }
        }
        ++i;
    }

    m_isBusy = false;

    if ( m_scheduledForDestruction )
    {
        m_ownerNotifier->m_imp = GUCEF_NULL;
        GUCEF_DELETE m_ownerNotifier;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

void
CNotifierImplementor::UnsubscribeAllFromNotifier( bool busyProcessingMailbox )
{GUCEF_TRACE;

    CNotifierScopeLock lock( m_ownerNotifier );

    if ( !m_isBusy || busyProcessingMailbox )
    {
        // Now we will remove references to the notifier at the observers
        TObserverList::iterator i = m_observers.begin();
        while ( i != m_observers.end() )
        {
            (*i).first->UnlinkFrom( m_ownerNotifier );
            ++i;
        }
        m_observers.clear();

        TNotificationList::iterator n = m_eventobservers.begin();
        while ( n != m_eventobservers.end() )
        {
            TEventNotificationMap& eventMap = (*n).second;
            TEventNotificationMap::iterator m = eventMap.begin();
            while ( m != eventMap.end() )
            {
                (*m).first->UnlinkFrom( m_ownerNotifier );
                TEventHandlerFunctorInterfaceVector& callbacks = (*m).second;
                if ( !callbacks.empty() )
                {
                    TEventHandlerFunctorInterfaceVector::iterator v = callbacks.begin();
                    while ( v != callbacks.end() )
                    {
                        GUCEF_DELETE (*v);
                        (*v) = GUCEF_NULL;
                        ++v;
                    }
                    callbacks.clear();
                }
                ++m;
            }
            ++n;
        }
        m_eventobservers.clear();
    }
    else
    {
        TCmdMailElement cmdMailElement;
        cmdMailElement.cmdType = REQUEST_UNSUBSCRIBE_ALL;
        cmdMailElement.eventID = CEvent();     // <- not used in this context
        cmdMailElement.observer = GUCEF_NULL;        // <- not used in this context
        cmdMailElement.callback = GUCEF_NULL;        // <- not used in this context
        cmdMailElement.notify = true;          // <- not used in this context
        cmdMailElement.observerIsDestroyed = true;
        m_cmdMailStack.push_back( cmdMailElement );
    }
}

/*-------------------------------------------------------------------------*/

bool
CNotifierImplementor::Subscribe( CObserver* observer                      ,
                                 bool busyProcessingMailbox /* = false */ )
{GUCEF_TRACE;

    CNotifierScopeLock lock( m_ownerNotifier );

    if ( !m_isBusy || busyProcessingMailbox )
    {
        if ( !LinkObserver( observer, true ) )
        {
            if ( m_scheduledForDestruction )
            {
                lock.EarlyUnlock();
                m_ownerNotifier->m_imp = GUCEF_NULL;
                GUCEF_DELETE m_ownerNotifier;
                return false;
            }

            if ( !busyProcessingMailbox )
                ProcessMailbox();
        }
    }
    else
    {
        TCmdMailElement cmdMailElement;
        cmdMailElement.cmdType = REQUEST_SUBSCRIBE;
        cmdMailElement.eventID = CEvent();     // <- not used in this context
        cmdMailElement.observer = observer;
        cmdMailElement.callback = GUCEF_NULL;
        cmdMailElement.notify = true;
        cmdMailElement.observerIsDestroyed = false;
        m_cmdMailStack.push_back( cmdMailElement );

        LinkObserver( observer, true );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNotifierImplementor::Subscribe( CObserver* observer                                    ,
                                 const CEvent& eventid                                  ,
                                 CIEventHandlerFunctorBase* callback /* = GUCEF_NULL */ ,
                                 bool busyProcessingMailbox /* = false */               )
{GUCEF_TRACE;
    
    CNotifierScopeLock lock( m_ownerNotifier );

    bool wasBusy = m_isBusy;
    if ( !m_isBusy || busyProcessingMailbox )
    {
        m_isBusy = true;

        /*
         *  We filter subscription requests for standard events because there is
         *  no need to store them. All observers are subscribed to these events at all times anyway.
         * 
         *  The exception being if a specific callback is provided in which case we do record it since
         *  there is no way to divine that information later
         */
        if ( ( GUCEF_NULL != callback               ) || (
             ( eventid != CNotifier::DestructionEvent ) &&
             ( eventid != CNotifier::ModifyEvent )      &&
             ( eventid != CNotifier::SubscribeEvent )   &&
             ( eventid != CNotifier::UnsubscribeEvent )  ) )
        {
            // Get a list of all observers that listen to this specific event
            TNotificationList::iterator n( m_eventobservers.find( eventid ) );
            if ( n != m_eventobservers.end() )
            {
                // Check if a subscription is already present for the given observer
                TEventNotificationMap& eventObservers = (*n).second;
                TEventNotificationMap::iterator i( eventObservers.find( observer ) );
                if ( i != eventObservers.end() )
                {
                    // The observer is already subscribed to this event
                    // All we have to do is make sure the callback is added
                    TEventHandlerFunctorInterfaceVector& callbacks = (*i).second;
                    if ( GUCEF_NULL != callback )
                    {
                        bool isReplacement = false;
                        TEventHandlerFunctorInterfaceVector::iterator c = callbacks.begin();
                        while ( c != callbacks.end() )
                        {
                            CIEventHandlerFunctorBase* functor = (*c);                        
                            if ( functor == callback ||
                                 functor->GetTargetObserver() == callback->GetTargetObserver() )
                            {
                                // This is the same callback.
                                // We will treat this as an update/refresh but not as another independent subscription
                                isReplacement = true;
                                GUCEF_DELETE functor;
                                (*c) = static_cast< CIEventHandlerFunctorBase* >( callback->Clone() );
                                break;
                            }
                            ++c;
                        }

                        if ( !isReplacement )
                            callbacks.push_back( static_cast< CIEventHandlerFunctorBase* >( callback->Clone() ) );
                    }

                    m_isBusy = wasBusy;
                    return true;
                }

                /*
                 *  If we get here then we have a new observer on our hands for the given
                 *  known event, as such we need to update our administration.
                 */
                if ( GUCEF_NULL != callback )
                {
                    TEventHandlerFunctorInterfaceVector& callbacks = eventObservers[ observer ];
                    callbacks.push_back( static_cast< CIEventHandlerFunctorBase* >( callback->Clone() ) );
                }
            }
            else
            {
                /*
                 *  If we get here then the event is unknown to us at this time.
                 *  We will add an entry to the list for this event and hook up the
                 *  subscriber.
                 *  Adding the event entry is accomplished by using the index operator on the map
                 */
                TEventNotificationMap& eventObservers = m_eventobservers[ eventid ];
                if ( GUCEF_NULL != callback )
                {
                    TEventHandlerFunctorInterfaceVector& callbacks = eventObservers[ observer ];
                    callbacks.push_back( static_cast< CIEventHandlerFunctorBase* >( callback->Clone() ) );
                }
            }
        }

        m_isBusy = wasBusy;
        
        LinkObserver( observer, false );

        if ( m_scheduledForDestruction )
        {
            lock.EarlyUnlock();
            m_ownerNotifier->m_imp = GUCEF_NULL;
            GUCEF_DELETE m_ownerNotifier; // <- This will GUCEF_DELETE this object as well thus self-destructing
            return false;
        }

        if ( !busyProcessingMailbox )
            ProcessMailbox();
    }
    else
    {
        TCmdMailElement cmdMailElement;
        cmdMailElement.cmdType = REQUEST_SUBSCRIBE;
        cmdMailElement.eventID = eventid;
        cmdMailElement.observer = observer;
        cmdMailElement.notify = true;
        cmdMailElement.observerIsDestroyed = false;
        if ( callback != GUCEF_NULL )
        {
            cmdMailElement.callback = static_cast< CIEventHandlerFunctorBase* >( callback->Clone() );
        }
        else
        {
            cmdMailElement.callback = GUCEF_NULL;
        }
        m_cmdMailStack.push_back( cmdMailElement );

        LinkObserver( observer, false );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CNotifierImplementor::LinkObserver( CObserver* observer          ,
                                    bool subscribeToAllEvents    )
{GUCEF_TRACE;

    bool isNewLink = false;

    if ( !m_isBusy )
    {
        /*
         *  Add the observer to our list of observers as needed.
         *  This will automatically subscribe the observer to the
         *  four standard notification events.
         */
        TObserverList::iterator i = m_observers.find( observer );
        if ( i == m_observers.end() )
        {
            m_observers[ observer ] = subscribeToAllEvents;

            // Establish our communication path from observer -> notifier
            observer->LinkTo( m_ownerNotifier );

            GUCEF_DEBUG_LOG_EVERYTHING( "NotifierImplementor(" + CORE::PointerToString( this ) + "): Class " + m_ownerNotifier->GetClassTypeName() + 
                ": Dispatching event \"" + CNotifier::SubscribeEvent.GetName() + "\" to " + observer->GetClassTypeName() + "(" + CORE::PointerToString( observer ) + ")" );

            // Send the standard subscription event to notify of first link
            observer->OnNotify( m_ownerNotifier            ,
                                CNotifier::SubscribeEvent  ,
                                GUCEF_NULL                 );
            
            // New link, so return True
            return true;
        }
        else
        {
            // Already linked, but check if we need to switch to an "all events" subscription
            if ( subscribeToAllEvents )
            {
                (*i).second = subscribeToAllEvents;
            }
            
            // Existing link, so return False
            return false;
        }
    }
    else
    {
        // Updating administration on the Notifier side will be deferred via mailboxes
        // The same is true for notification of the subscription
        TObserverList::iterator i = m_observers.find( observer );
        if ( i == m_observers.end() )
        {
            // Establish our communication path from observer -> notifier
            // We do that even when the notifier is already busy because we dont want to miss out on observer destruction notification
            observer->LinkTo( m_ownerNotifier );

            // New link, so return True
            return true;
        }
        return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CNotifierImplementor::Unsubscribe( CObserver* observer )
{GUCEF_TRACE;

    CNotifierScopeLock lock( m_ownerNotifier );

    UnsubscribeFromAllEvents( observer ,
                              true     ,
                              false    ,
                              false    );
}

/*-------------------------------------------------------------------------*/

void
CNotifierImplementor::UnsubscribeFromAllEvents( CObserver* observer            ,
                                                const bool notifyObserver      ,
                                                const bool observerDestruction ,
                                                bool busyProcessingMailbox     )
{GUCEF_TRACE;

    bool wasBusy = m_isBusy;
    if ( !m_isBusy || busyProcessingMailbox || observerDestruction )
    {
        m_isBusy = true;

        /*
         *  Unsubscribe the observer from all standard
         *  notification events.
         */
        TObserverList::iterator observerEntry = m_observers.find( observer );
        if ( observerEntry != m_observers.end() )
        {
            m_observers.erase( observerEntry );
        }

        /*
         *  Unsubscribe the observer from all custom events
         */
        TNotificationList::iterator n( m_eventobservers.begin() );
        while ( n != m_eventobservers.end() )
        {
            TEventNotificationMap& eventObservers = (*n).second;
            TEventNotificationMap::iterator i( eventObservers.find( observer ) );
            if ( i != eventObservers.end() )
            {
                /*
                 *  Remove the reference to the given observer
                 *  Also GUCEF_DELETE the event handler functor as needed
                 */
                TEventHandlerFunctorInterfaceVector& callbacks = (*i).second;
                if ( !callbacks.empty() )
                {
                    TEventHandlerFunctorInterfaceVector::iterator m = callbacks.begin();
                    while ( m != callbacks.end() )
                    {
                        GUCEF_DELETE (*m);
                        (*m) = GUCEF_NULL;
                        ++m;
                    }
                    callbacks.clear();
                }
                eventObservers.erase( i );
            }

            ++n;
        }

        if ( notifyObserver && !observerDestruction )
        {
            /*
             *  Now remove the observer's reference to the notifier
             *  for all events
             */
            observer->UnlinkFrom( m_ownerNotifier );

            GUCEF_DEBUG_LOG_EVERYTHING( "NotifierImplementor(" + CORE::PointerToString( this ) + "): Class " + m_ownerNotifier->GetClassTypeName() + 
                    ": Dispatching event \"" + CNotifier::UnsubscribeEvent.GetName() + "\" to " + observer->GetClassTypeName() + "(" + CORE::PointerToString( observer ) + ")" );

            /*
             *  Send the standard un-subscribe event
             */
            observer->OnNotify( m_ownerNotifier             ,
                                CNotifier::UnsubscribeEvent );
        }

    if ( observerDestruction )
    {        
        // For a destroyed observer we also need to remove all remaining references in the mailboxes

        TEventMailVector::iterator e = m_eventMailStack.begin();
        while ( e != m_eventMailStack.end() )
        {
            TEventMailElement& mail = (*e);
            if ( observer == mail.specificObserver )
            {
                mail.specificObserver = GUCEF_NULL;
                mail.specificObserverIsDestroyed = true;
            }
            ++e;
        }
        TCmdMailVector::iterator c = m_cmdMailStack.begin();
        while ( c != m_cmdMailStack.end() )
        {
            TCmdMailElement& mail = (*c);
            if ( observer == mail.observer )
            {
                mail.observer = GUCEF_NULL;
                mail.observerIsDestroyed = true;
            }
            ++c;
        }
    }

        m_isBusy = wasBusy;

        if ( m_scheduledForDestruction )
        {
            NotificationUnlock();
            m_ownerNotifier->m_imp = GUCEF_NULL;
            GUCEF_DELETE m_ownerNotifier;
            return;
        }

        if ( !busyProcessingMailbox )
            ProcessMailbox();
    }
    else
    {
        TCmdMailElement cmdMailElement;
        cmdMailElement.cmdType = REQUEST_UNSUBSCRIBE;
        cmdMailElement.eventID = CEvent();    // <- not used in this context
        cmdMailElement.observer = observer;
        cmdMailElement.callback = GUCEF_NULL;
        cmdMailElement.notify = notifyObserver;
        cmdMailElement.observerIsDestroyed = observerDestruction;
        m_cmdMailStack.push_back( cmdMailElement );
    }
}

/*-------------------------------------------------------------------------*/

void
CNotifierImplementor::Unsubscribe( CObserver* observer                      ,
                                   const CEvent& eventid                    ,
                                   bool busyProcessingMailbox /* = false */ )
{GUCEF_TRACE;

    CNotifierScopeLock lock( m_ownerNotifier );

    bool wasBusy = m_isBusy;
    if ( !m_isBusy || busyProcessingMailbox )
    {
        m_isBusy = true;

        if ( ( eventid != CNotifier::DestructionEvent ) &&
             ( eventid != CNotifier::SubscribeEvent )   &&
             ( eventid != CNotifier::UnsubscribeEvent ) &&
             ( eventid != CNotifier::ModifyEvent )       )
        {
            TEventNotificationMap& eventObservers = m_eventobservers[ eventid ];
            TEventNotificationMap::iterator i( eventObservers.begin() );
            while ( i != eventObservers.end() )
            {
                if ( (*i).first == observer )
                {
                    /*
                     *  We found the observer and we will remove it from
                     *  our notification list for this event.
                     */
                    TEventHandlerFunctorInterfaceVector& callbacks = (*i).second;
                    if ( !callbacks.empty() )
                    {
                        TEventHandlerFunctorInterfaceVector::iterator m = callbacks.begin();
                        while ( m != callbacks.end() )
                        {                            
                            GUCEF_DELETE (*m);
                            (*m) = GUCEF_NULL;
                            ++m;
                        }
                        callbacks.clear();
                    }
                    eventObservers.erase( i );
                    m_isBusy = wasBusy;                    
                    return;
                }
                ++i;
            }
        }

        m_isBusy = wasBusy;

        if ( m_scheduledForDestruction )
        {
            lock.EarlyUnlock();
            m_ownerNotifier->m_imp = GUCEF_NULL;
            GUCEF_DELETE m_ownerNotifier;
            return;
        }

        if ( !busyProcessingMailbox )
            ProcessMailbox();
    }
    else
    {
        TCmdMailElement cmdMailElement;
        cmdMailElement.cmdType = REQUEST_UNSUBSCRIBE;
        cmdMailElement.eventID = eventid;
        cmdMailElement.observer = observer;
        cmdMailElement.callback = GUCEF_NULL;    // <- not used in this context
        cmdMailElement.notify = true;
        cmdMailElement.observerIsDestroyed = false;
        m_cmdMailStack.push_back( cmdMailElement );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CNotifierImplementor::GetSubscriptionCountForObserver( CObserver* observer ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == observer )
        return 0;

    UInt32 subscriptionCount( 0 );
    CNotifierScopeReadOnlyLock lock( m_ownerNotifier );

    TObserverList::const_iterator i = m_observers.find( observer );
    if ( i != m_observers.end() )
        subscriptionCount += 4; // Add the 4 hardcoded basic events that every link has a mandatory subscription on

    // Go through all subscriptions
    // Note that storage is optimized for notifcations, not this use-case
    TNotificationList::const_iterator n = m_eventobservers.begin();
    while ( n != m_eventobservers.end() )
    {
        const TEventNotificationMap& subsForEvent = (*n).second;
        TEventNotificationMap::const_iterator m = subsForEvent.begin();
        while ( m != subsForEvent.end() )
        {
            if ( (*m).first == observer )
                ++subscriptionCount;
            ++m;
        }
        ++n;
    }

    // We will ignore possible extra subscriptions that might be lurking in mailboxes
    // This is a snapshot in time anyway
    return subscriptionCount;
}

/*-------------------------------------------------------------------------*/

bool
CNotifierImplementor::NotifyObservers( void )
{GUCEF_TRACE;

    NotificationLock();
    if ( ForceNotifyObserversOnce( CNotifier::ModifyEvent ) )
    {
        NotificationUnlock();
        return true;
    }
    // Notifier is destroyed, don't unlock just return
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CNotifierImplementor::NotifyObserversFrom( CNotifier& sender     ,
                                           const CEvent& eventid ,
                                           CICloneable* eventData )
{GUCEF_TRACE;

    return sender.NotifyObservers( eventid   ,
                                   eventData );
}

/*-------------------------------------------------------------------------*/

bool
CNotifierImplementor::NotifyObservers( const CEvent& eventid  ,
                                       CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_ownerNotifier )
    {
        return NotifyObservers( *m_ownerNotifier ,
                                eventid          ,
                                eventData        );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CNotifierImplementor::NotifyObservers( CNotifier& sender          ,
                                       const CEvent& eventid      ,
                                       CICloneable* eventData     ,
                                       bool busyProcessingMailbox )
{GUCEF_TRACE;

    /*
     *  We will use the flag 'm_isBusy' to indicate whether
     *  we are busy. It is assumed that the code handling that flag can only be accessed
     *  from the same thread since the Lock() call should hold any other thread if
     *  we are operating in a multi threaded environment
     *  This action is required because a notification can trigger some action that results
     *  in a new notification request on the same notifier this could muck up our administration
     *  It could also cause stack overflows if the event chain where long enough.
     */
    CNotifierScopeLock lock( m_ownerNotifier );

    bool wasBusy = m_isBusy;
    if ( !m_isBusy || busyProcessingMailbox )
    {
        m_isBusy = true;

        CObserver* oPtr = GUCEF_NULL;
        TObserverSet notifiedObservers;

        // Notify observers that are subscribed to this specific event (if any)       
        TNotificationList::iterator l = m_eventobservers.find( eventid );
        if ( l != m_eventobservers.end() )
        {
            TEventNotificationMap& observers = (*l).second;        
            TEventNotificationMap::iterator n = observers.begin();
            while ( n != observers.end() )
            {
                oPtr = (*n).first;
                bool notified = false;

                // Check if we should perform notification using the generic
                // handler or a user specified callback.
                TEventHandlerFunctorInterfaceVector& callbacks = (*n).second;
                if ( !callbacks.empty() )
                {
                    TEventHandlerFunctorInterfaceVector::iterator m = callbacks.begin();
                    while ( m != callbacks.end() )
                    {
                        CIEventHandlerFunctorBase* callback = (*m);
                        if ( GUCEF_NULL != callback )
                        {
                            GUCEF_DEBUG_LOG_EVERYTHING( "NotifierImplementor(" + CORE::PointerToString( this ) + "): Class " + m_ownerNotifier->GetClassTypeName() +    
                                ": Dispatching event \"" + eventid.GetName() + "\" to " + callback->GetClassTypeName() + "(" + CORE::PointerToString( callback ) + ")" );

                            callback->OnNotify( &sender   ,
                                                eventid   ,
                                                eventData );
                            notified = true;

                            // Check if someone deleted our owner notifier
                            if ( m_ownerNotifier == GUCEF_NULL )
                            {
                                // Gracefully handle the destruction sequence
                                m_isBusy = wasBusy;
                                lock.EarlyUnlock();
                                Destroy( this );
                                return false;
                            }
                        }
                        ++m;
                    }
                }
                else
                {
                    GUCEF_DEBUG_LOG_EVERYTHING( "CNotifierImplementor(" + CORE::PointerToString( this ) + "): Class " + m_ownerNotifier->GetClassTypeName() + 
                        ": Dispatching event \"" + eventid.GetName() + "\" to " + oPtr->GetClassTypeName() + "(" + CORE::PointerToString( oPtr ) + ")" );

                    oPtr->OnNotify( &sender   ,
                                    eventid   ,
                                    eventData );                            
                    notified = true;
                }
                  
                if ( notified )
                {
                    // Check if someone deleted our owner notifier
                    if ( m_ownerNotifier == GUCEF_NULL )
                    {
                        // Gracefully handle the destruction sequence
                        m_isBusy = wasBusy;
                        lock.EarlyUnlock();
                        Destroy( this );
                        return false;
                    }

                    // Add the observer to our 'notified' list
                    notifiedObservers.insert( oPtr );

                    if ( !busyProcessingMailbox )
                    {
                        // Process command mail if needed
                        if ( !m_cmdMailStack.empty() )
                        {
                            // We have command mail
                            ProcessCmdMailbox();

                            // the administration has been altered, we now have no choice
                            // but to start from the beginning. Its some extra work but 'notifiedObservers'
                            // will make sure we do not notifiy redundantly
                            n = observers.begin();
                            continue;
                        }
                    }
                }
                ++n;
            }
        }

        // Now we process observers that are subscribed to 'all' events
        // these are catch-all subscriptions
        TObserverList::iterator i = m_observers.begin();
        while ( i != m_observers.end() )
        {
            // Check if this observer is registered as interested in all events which is just a flag
            if ( (*i).second )
            {
                // Check if we have not already notified this observer using a more dedicated subscription and potentially dedicated callback
                oPtr = (*i).first;
                if ( notifiedObservers.find( oPtr ) == notifiedObservers.end() )
                {
                    GUCEF_DEBUG_LOG_EVERYTHING( "NotifierImplementor(" + CORE::PointerToString( this ) + "): Class " + m_ownerNotifier->GetClassTypeName() + 
                        ": Dispatching event \"" + eventid.GetName() + "\" to " + oPtr->GetClassTypeName() + "(" + CORE::PointerToString( oPtr ) + ")" );

                    // Perform the notification because this observer has not been notified at all yet
                    oPtr->OnNotify( &sender   ,
                                    eventid   ,
                                    eventData );

                    // Check if someone deleted our owner notifier
                    if ( m_ownerNotifier == GUCEF_NULL )
                    {
                        // Gracefully handle the destruction sequence
                        m_isBusy = wasBusy;
                        lock.EarlyUnlock();
                        Destroy( this );
                        return false;
                    }

                    // Add the observer to our 'notified' list
                    notifiedObservers.insert( oPtr );

                    if ( !busyProcessingMailbox )
                    {
                        // Process command mail if needed
                        if ( !m_cmdMailStack.empty() )
                        {
                            // We have command mail
                            ProcessCmdMailbox();

                            // the administration has been altered, we now have no choice
                            // but to start from the beginning
                            i = m_observers.begin();
                            continue;
                        }
                    }
                }
            }
            ++i;
        }

        m_isBusy = wasBusy;

        if ( m_scheduledForDestruction )
        {            
            m_ownerNotifier->m_imp = GUCEF_NULL;
            lock.EarlyUnlock();
            GUCEF_DELETE m_ownerNotifier;
            return false;
        }

        // Now we check if any requests where received while we where busy
        if ( !busyProcessingMailbox )
            ProcessMailbox();
    }
    else
    {
        GUCEF_DEBUG_LOG_EVERYTHING( "CNotifierImplementor(" + CORE::PointerToString( this ) + "): Class " + m_ownerNotifier->GetClassTypeName() + 
            " is Busy, Mailboxing event \"" +  eventid.GetName() + "\" from " + sender.GetClassTypeName() );

        // This notifier is already busy processing a notification
        // We will store the request until we are done
        TEventMailElement mail;
        mail.eventID = eventid;
        mail.specificObserver = GUCEF_NULL; // <- not used in this context  
        mail.specificObserverIsDestroyed = false;
        mail.sender = &sender;
        if ( eventData != GUCEF_NULL )
        {
            mail.eventData = eventData->Clone();
        }
        else
        {
            mail.eventData = GUCEF_NULL;
        }
        m_eventMailStack.push_back( mail );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

void
CNotifierImplementor::ProcessCmdMailbox( void )
{GUCEF_TRACE;

    // Now we check if any command requests where received while we where busy
    while ( !m_cmdMailStack.empty() )
    {
        TCmdMailElement cmdMailElement( m_cmdMailStack[ m_cmdMailStack.size()-1 ] );
        m_cmdMailStack.pop_back();

        if ( cmdMailElement.observerIsDestroyed )
        {
            // Ignore this mail, the observer is was destroyed after sending it
            continue;
        }

        switch ( cmdMailElement.cmdType )
        {
            case REQUEST_SUBSCRIBE :
            {
                if ( cmdMailElement.eventID.IsInitialized() )
                {
                    Subscribe( cmdMailElement.observer ,
                               cmdMailElement.eventID  ,
                               cmdMailElement.callback ,
                               true                    );
                }
                else
                {
                    Subscribe( cmdMailElement.observer ,
                               true                    );
                }
                break;
            }
            case REQUEST_UNSUBSCRIBE :
            {
                GUCEF_CHECKMEM( cmdMailElement.observer, sizeof( CObserver ) );
                if ( cmdMailElement.eventID.IsInitialized() )
                {
                    Unsubscribe( cmdMailElement.observer ,
                                 cmdMailElement.eventID  ,
                                 true                    );
                }
                else
                {
                    UnsubscribeFromAllEvents( cmdMailElement.observer            ,
                                              cmdMailElement.notify              ,
                                              cmdMailElement.observerIsDestroyed ,
                                              true                               );
                }
                break;
            }
            case REQUEST_UNSUBSCRIBE_ALL :
            {
                GUCEF_CHECKMEM( cmdMailElement.observer, sizeof( CObserver ) );
                UnsubscribeAllFromNotifier( true );
                break;
            }
            default :
            {
                // We should not get here
                GUCEF_UNREACHABLE;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CNotifierImplementor::ProcessEventMailbox( void )
{GUCEF_TRACE;

    // Now we check if any notifications requests where received while we where busy
    if ( !m_eventMailStack.empty() )
    {
        TEventMailElement entry( m_eventMailStack[ m_eventMailStack.size()-1 ] );
        m_eventMailStack.pop_back();

        if ( !entry.specificObserverIsDestroyed )
        {
            // Dispatch the message in our mailbox
            // If the mailbox has any more items remaining we will end up back here
            // for the next mail item
            if ( GUCEF_NULL == entry.specificObserver )
            {
                NotifyObservers( *entry.sender, entry.eventID, entry.eventData, true );
            }
            else
            {
                NotifySpecificObserver( *entry.sender, *entry.specificObserver, entry.eventID, entry.eventData, true );
            }
        }
        GUCEF_DELETE entry.eventData;
    }
}

/*-------------------------------------------------------------------------*/

void
CNotifierImplementor::ProcessMailbox( void )
{GUCEF_TRACE;

    ProcessCmdMailbox();
    ProcessEventMailbox();
}

/*-------------------------------------------------------------------------*/

bool
CNotifierImplementor::NotifySpecificObserverFrom( CNotifier& sender           ,
                                                  CObserver& specificObserver ,
                                                  const CEvent& eventid       ,
                                                  CICloneable* eventData      )
{GUCEF_TRACE;

    return sender.NotifySpecificObserver( specificObserver ,
                                          eventid          ,
                                          eventData        );
}

/*-------------------------------------------------------------------------*/

bool
CNotifierImplementor::NotifySpecificObserver( CObserver& specificObserver ,
                                              const CEvent& eventid       ,
                                              CICloneable* eventData      )
{GUCEF_TRACE;

    return NotifySpecificObserver( *m_ownerNotifier ,
                                   specificObserver ,
                                   eventid          ,
                                   eventData        ,
                                   false            );
}

/*-------------------------------------------------------------------------*/

bool
CNotifierImplementor::NotifySpecificObserver( CNotifier& sender           ,
                                              CObserver& specificObserver ,
                                              const CEvent& eventid       ,
                                              CICloneable* eventData      ,
                                              bool busyProcessingMailbox  )
{GUCEF_TRACE;

    /*
     *  We will use the flag 'm_isBusy' to indicate whether
     *  we are busy. It is assumed that the code handling that flag can only be accessed
     *  from the same thread since the Lock() call should hold any other thread if
     *  we are operating in a multi threaded environment
     *  This action is required because a notification can trigger some action that results
     *  in a new notification request on the same notifier this could muck up our administration
     *  It could also cause stack overflows if the event chain where long enough.
     */
    CNotifierScopeLock lock( m_ownerNotifier );

    if ( !m_isBusy || busyProcessingMailbox )
    {
        m_isBusy = true;

        // First we check and validate a subscription for all events for the given observer
        TObserverList::iterator i = m_observers.find( &specificObserver );
        if ( i != m_observers.end() )
        {
            if ( (*i).second )
            {
                GUCEF_DEBUG_LOG_EVERYTHING( "CNotifierImplementor(" + CORE::PointerToString( this ) + "): Class " + m_ownerNotifier->GetClassTypeName() + ": Dispatching event \"" + eventid.GetName() + "\" to " + specificObserver.GetClassTypeName() + "(" + CORE::PointerToString( &specificObserver ) + ")" );

                // Perform the notification
                // If an observer is subscribed to all events we never used specialized callbacks
                // instead we always use the standard OnNotify()
                specificObserver.OnNotify( &sender   ,
                                           eventid   ,
                                           eventData );
                m_isBusy = false;

                // Check if someone deleted our owner notifier
                if ( m_ownerNotifier == GUCEF_NULL )
                {
                    // Gracefully handle the destruction sequence
                    lock.EarlyUnlock();
                    Destroy( this );
                    return false;
                }

                // Process command mail if needed
                if ( !busyProcessingMailbox )
                {
                    if ( !m_cmdMailStack.empty() )
                    {
                        // We have command mail
                        ProcessCmdMailbox();
                    }
                }

                // Since we only have no notify a single observer we can stop here
                // since an observer cannot be notified twice for the same event
                return true;
            }
        }

        // Validate a subscription of the given observer to this specific event
        TEventNotificationMap& observers = m_eventobservers[ eventid ];
        TEventNotificationMap::iterator n = observers.find( &specificObserver );
        if ( n != observers.end() )
        {
            // Check if we should perform notification using the generic
            // handler or a user specified callback.

            TEventHandlerFunctorInterfaceVector& callbacks = (*n).second;
            if ( !callbacks.empty() )
            {
                TEventHandlerFunctorInterfaceVector::iterator m = callbacks.begin();
                while ( m != callbacks.end() )
                {
                    CIEventHandlerFunctorBase* callback = (*m);

                    GUCEF_DEBUG_LOG_EVERYTHING( "CNotifierImplementor(" + CORE::PointerToString( this ) + "): Class " + m_ownerNotifier->GetClassTypeName() + 
                        ": Dispatching event \"" + eventid.GetName() + "\" to " + callback->GetClassTypeName() + "(" + CORE::PointerToString( callback ) + ")" );

                    callback->OnNotify( &sender   ,
                                        eventid   ,
                                        eventData );
                    ++m;
                }
            }
            else
            {
                GUCEF_DEBUG_LOG_EVERYTHING( "CNotifierImplementor(" + CORE::PointerToString( this ) + "): Class " + m_ownerNotifier->GetClassTypeName() + 
                    ": Dispatching event \"" + eventid.GetName() + "\" to " + specificObserver.GetClassTypeName() + "(" + CORE::PointerToString( &specificObserver ) + ")" );

                specificObserver.OnNotify( &sender   ,
                                           eventid   ,
                                           eventData );                           
            }

            m_isBusy = false;

            // Check if someone deleted our owner notifier
            if ( m_ownerNotifier == GUCEF_NULL )
            {
                // Gracefully handle the destruction sequence
                lock.EarlyUnlock();
                Destroy( this );
                return false;
            }

            // Process command mail if needed
            if ( !busyProcessingMailbox )
            {
                if ( !m_cmdMailStack.empty() )
                {
                    // We have command mail
                    ProcessMailbox();
                }
            }
        }

        if ( m_scheduledForDestruction )
        {
            m_ownerNotifier->m_imp = GUCEF_NULL;
            lock.EarlyUnlock();
            GUCEF_DELETE m_ownerNotifier;
            return false;
        }
    }
    else
    {
        GUCEF_DEBUG_LOG_EVERYTHING( "CNotifierImplementor(" + CORE::PointerToString( this ) + "): Class " + m_ownerNotifier->GetClassTypeName() + " is Busy, Mailboxing event \"" +  eventid.GetName() + "\" from " + sender.GetClassTypeName() + " to " + specificObserver.GetClassTypeName() );

        // This notifier is already busy processing a notification
        // We will store the request until we are done
        TEventMailElement mail;
        mail.eventID = eventid;
        mail.specificObserver = &specificObserver;
        mail.specificObserverIsDestroyed = false;
        mail.sender = &sender;
        if ( eventData != GUCEF_NULL )
        {
            mail.eventData = eventData->Clone();
        }
        else
        {
            mail.eventData = GUCEF_NULL;
        }
        m_eventMailStack.push_back( mail );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

void
CNotifierImplementor::OnObserverDestroy( CObserver* observer )
{GUCEF_TRACE;
    
    CNotifierScopeLock lock( m_ownerNotifier );

    GUCEF_DEBUG_LOG_EVERYTHING( "NotifierImplementor(" + CORE::PointerToString( this ) + "):OnObserverDestroy: for observer " + observer->GetClassTypeName() + "(" + CORE::PointerToString( observer ) + ")" );    
    
    UnsubscribeFromAllEvents( observer ,
                              false    ,
                              true     ,
                              false    );

    lock.EarlyUnlock();

    m_ownerNotifier->OnObserverDestruction( observer );
}

/*-------------------------------------------------------------------------*/

void
CNotifierImplementor::OnDeathOfOwnerNotifier( void )
{GUCEF_TRACE;

    /*
     *  Notify observers that the notifier is die-ing
     */
    ForceNotifyObserversOnce( CNotifier::DestructionEvent );

    /*
     *  Now we will remove references to the notifier at the observers
     */
    UnsubscribeAllFromNotifier( false );

    /*
     *  This is an important step,..
     *  We null the owner pointer which functions as a flag signaling that
     *  the owner notifier is now considered to be destroyed.
     */
    m_ownerNotifier = GUCEF_NULL;

    if ( !m_isBusy )
    {
        /*
         *  We are not busy, meaning we did not arrive here trough some action
         *  trigger by the notifier itself. Thus we can safely delete ourselves
         *  right now.
         */
        Destroy( this );
    }
}

/*-------------------------------------------------------------------------*/

void
CNotifierImplementor::ScheduleForDestruction( void )
{
   CNotifierScopeLock lock( m_ownerNotifier );

   GUCEF_DEBUG_LOG_EVERYTHING( "CNotifierImplementor(" + CORE::PointerToString( this ) + "): Scheduling for destruction" );

   if ( !m_isBusy )
    {
        /*
         *  We are not busy, meaning we did not arrive here trough some action
         *  trigger by the notifier itself. Thus we can safely delete the notifier
         */
        lock.EarlyUnlock();
        GUCEF_DELETE m_ownerNotifier;
    }
    else
    {
        /*
         *  We are busy and the user has opted to perform a delayed delete
         *  We will simply set a flag signaling that the notifier should be deleted
         *  once it finishes it's current operation
         */
        m_scheduledForDestruction = true;
    }
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CNotifierImplementor::NotificationLock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    if ( m_ownerNotifier != GUCEF_NULL )
    {
        return m_ownerNotifier->NotificationLock( lockWaitTimeoutInMs );
    }
    return MT::LOCKSTATUS_OPERATION_FAILED;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CNotifierImplementor::NotificationUnlock( void ) const
{GUCEF_TRACE;

    if ( m_ownerNotifier != GUCEF_NULL )
    {
        return m_ownerNotifier->NotificationUnlock();
    }
    return MT::LOCKSTATUS_OPERATION_FAILED;
}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CNotifierImplementor::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
