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
#include <algorithm>

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

/**
 *  The observers that got the event being delivered, in the order of their pointers (the order of the subscription
 *  maps). A delivery walks the observers in that order, so an observer is nearly always added at the end and found
 *  not to be there by one comparison with the last; any other look up is a binary search. The first 64 are kept
 *  inline; beyond that the notifier's own spill list takes over, which keeps its capacity between deliveries: once a
 *  notifier has delivered an event to its observers, delivering another one does not allocate.
 */
class CNotifiedObservers
{
    public:

    typedef GUCEF::vector< CObserver*, gucef_allocator< CObserver* > > TSpill;

    explicit CNotifiedObservers( TSpill& spill )
        : m_count( 0 )
        , m_spill( spill )
        , m_spilled( false )
    {
    }

    bool Contains( CObserver* observer ) const
    {
        const UInt32 count = Count();
        if ( 0 == count )
            return false;
        CObserver* const* first = Begin();
        CObserver* const* last = first + count;
        std::less< CObserver* > order;
        if ( order( *( last - 1 ), observer ) )
            return false;
        CObserver* const* place = std::lower_bound( first, last, observer, order );
        return place != last && *place == observer;
    }

    void Add( CObserver* observer )
    {
        std::less< CObserver* > order;
        const UInt32 count = Count();
        if ( 0 != count && !order( *( Begin() + count - 1 ), observer ) )
        {
            // not after the last: an observer a delivery comes back to, its place is looked up
            CObserver* const* first = Begin();
            CObserver* const* place = std::lower_bound( first, first + count, observer, order );
            if ( place != first + count && *place == observer )
                return;
            Insert( (UInt32) ( place - first ), observer );
            return;
        }
        Insert( count, observer );
    }

    private:

    enum { InlineCapacity = 64 };

    UInt32 Count( void ) const { return m_spilled ? (UInt32) m_spill.size() : m_count; }

    CObserver* const* Begin( void ) const { return m_spilled ? &m_spill[ 0 ] : m_inline; }

    void Insert( UInt32 index, CObserver* observer )
    {
        if ( !m_spilled && m_count < InlineCapacity )
        {
            for ( UInt32 i=m_count; i>index; --i )
                m_inline[ i ] = m_inline[ i-1 ];
            m_inline[ index ] = observer;
            ++m_count;
            return;
        }
        if ( !m_spilled )
        {
            m_spill.assign( m_inline, m_inline + m_count );
            m_spilled = true;
        }
        m_spill.insert( m_spill.begin() + index, observer );
    }

    CNotifiedObservers( const CNotifiedObservers& src );                /**< not implemented */
    CNotifiedObservers& operator=( const CNotifiedObservers& src );     /**< not implemented */

    CObserver* m_inline[ InlineCapacity ];
    UInt32 m_count;
    TSpill& m_spill;
    bool m_spilled;
};

/*-------------------------------------------------------------------------*/


CNotifierImplementor::CNotifierImplementor( CNotifier* ownerNotifier )
    : m_isBusy( false )                 ,
      m_observers()                     ,
      m_eventobservers()                ,
      m_eventMailStack()                ,
      m_cmdMailStack()                  ,
      m_cmdMailProcessed( 0 )           ,
      m_eventMailProcessed( 0 )         ,
      m_notifiedSpill()                 ,
      m_notifiedSpillInUse( false )     ,
      m_subscriptionErasures( 0 )       ,
      m_ownerNotifier( ownerNotifier )  ,
      m_scheduledForDestruction( false ) ,
      m_newSubscribersOfDelivery( GUCEF_NULL )
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
      m_cmdMailProcessed( 0 )           ,
      m_eventMailProcessed( 0 )         ,
      m_notifiedSpill()                 ,
      m_notifiedSpillInUse( false )     ,
      m_subscriptionErasures( 0 )       ,
      m_ownerNotifier( ownerNotifier )  ,
      m_scheduledForDestruction( false ) ,
      m_newSubscribersOfDelivery( GUCEF_NULL )
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

    // Requests that were never carried out still hold their copy of a callback
    TCmdMailVector::iterator c = m_cmdMailStack.begin();
    while ( c != m_cmdMailStack.end() )
    {
        GUCEF_DELETE (*c).callback;
        (*c).callback = GUCEF_NULL;
        ++c;
    }
    m_cmdMailStack.clear();
    m_cmdMailProcessed = 0;

    TEventMailVector::iterator e = m_eventMailStack.begin();
    while ( e != m_eventMailStack.end() )
    {
        GUCEF_DELETE (*e).eventData;
        (*e).eventData = GUCEF_NULL;
        ++e;
    }
    m_eventMailStack.clear();
    m_eventMailProcessed = 0;
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

    // The walk of NotifyObservers(): it carries on after its place when the mail changed the administration, the
    // observers the mail subscribed at a place the walk has passed get the event first
    TObserverVector newSubscribers;
    TObserverVector* outerNewSubscribers = m_newSubscribersOfDelivery;
    m_newSubscribersOfDelivery = &newSubscribers;
    try
    {
        TObserverSet behindWalk;
        std::less< CObserver* > observerOrder;

        TObserverList::iterator i = m_observers.begin();
        CObserver* walkPlace = GUCEF_NULL;
        bool walked = false;
        for ( ;; )
        {
            TObserverList::iterator current;
            if ( !behindWalk.empty() )
            {
                oPtr = *behindWalk.begin();
                behindWalk.erase( behindWalk.begin() );
                current = m_observers.find( oPtr );
                if ( current == m_observers.end() )
                    continue;
            }
            else
            {
                if ( i == m_observers.end() )
                    break;
                current = i;
                oPtr = (*i).first;
                walkPlace = oPtr;
                walked = true;
            }

            // Check if we have not already notified this observer
            if ( notifiedObservers.find( oPtr ) == notifiedObservers.end() )
            {
                GUCEF_DEBUG_LOG_EVERYTHING( "NotifierImplementor(" << CORE::PointerToString( this ) << "): Class " << m_ownerNotifier->GetClassTypeName() << ": Dispatching event \"" << eventid.GetName() << "\" to " << oPtr->GetClassTypeName() << "(" << CORE::PointerToString( oPtr ) << ")" );

                oPtr->OnNotify( m_ownerNotifier ,
                                eventid         ,
                                data            );

                // Check if someone deleted our owner notifier
                if ( m_ownerNotifier == GUCEF_NULL )
                {
                    // Gracefully handle the destruction sequence
                    m_newSubscribersOfDelivery = outerNewSubscribers;
                    m_isBusy = false;
                    Destroy( this );
                    return false;
                }

                // Add the observer to our 'notified' list
                notifiedObservers.insert( oPtr );

                // Process command mail if needed
                if ( !m_cmdMailStack.empty() )
                    ProcessCmdMailbox();
            }

            // Observers the mail subscribed at a place the walk has passed get the event before the walk goes on
            if ( !newSubscribers.empty() )
            {
                if ( walked )
                {
                    for ( TObserverVector::iterator s = newSubscribers.begin(); s != newSubscribers.end(); ++s )
                        if ( observerOrder( (*s), walkPlace ) )
                            behindWalk.insert( (*s) );
                }
                newSubscribers.clear();
            }

            // The observer may have been removed while it was notified: the walk carries on after its place
            i = walked ? m_observers.upper_bound( walkPlace ) : m_observers.begin();
        }
    }
    catch ( ... )
    {
        // An observer threw: the exception leaves the delivery, and the notifier is left as after a delivery (the
        // caller ends the delivery, see NotifyObservers())
        m_newSubscribersOfDelivery = outerNewSubscribers;
        m_isBusy = false;
        throw;
    }

    m_newSubscribersOfDelivery = outerNewSubscribers;
    m_isBusy = false;

    // The destruction event is sent while the owner notifier is already being destroyed
    if ( m_scheduledForDestruction && CNotifier::DestructionEvent != eventid )
    {
        DestroyScheduledOwner();
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
        ++m_subscriptionErasures;
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
        ++m_subscriptionErasures;
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
        if ( !LinkObserver( observer, true, busyProcessingMailbox ) )
        {
            // While the mailbox is processed the frame which processes it takes care of a scheduled destruction
            if ( !busyProcessingMailbox )
            {
                if ( !m_scheduledForDestruction )
                    ProcessMailbox();
                if ( m_scheduledForDestruction )
                {
                    lock.EarlyUnlock();
                    DestroyScheduledOwner();
                    return false;
                }
            }
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

        LinkObserver( observer, true, false );
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
                 *  Without a callback the entry stays empty, which means the event goes to OnNotify()
                 */
                TEventHandlerFunctorInterfaceVector& callbacks = eventObservers[ observer ];
                if ( GUCEF_NULL != callback )
                    callbacks.push_back( static_cast< CIEventHandlerFunctorBase* >( callback->Clone() ) );
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
                TEventHandlerFunctorInterfaceVector& callbacks = eventObservers[ observer ];
                if ( GUCEF_NULL != callback )
                    callbacks.push_back( static_cast< CIEventHandlerFunctorBase* >( callback->Clone() ) );
            }

            // the new subscription may be behind a delivery of the event in progress, see NotifyObservers()
            if ( GUCEF_NULL != m_newSubscribersOfDelivery )
                m_newSubscribersOfDelivery->push_back( observer );
        }

        m_isBusy = wasBusy;
        
        LinkObserver( observer, false, busyProcessingMailbox );

        // While the mailbox is processed the frame which processes it takes care of a scheduled destruction
        if ( !busyProcessingMailbox )
        {
            if ( !m_scheduledForDestruction )
                ProcessMailbox();
            if ( m_scheduledForDestruction )
            {
                lock.EarlyUnlock();
                DestroyScheduledOwner();
                return false;
            }
        }
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

        LinkObserver( observer, false, false );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CNotifierImplementor::LinkObserver( CObserver* observer          ,
                                    bool subscribeToAllEvents    ,
                                    bool busyProcessingMailbox   )
{GUCEF_TRACE;

    // A request carried out from the mailbox completes the administration, also while an event is being delivered:
    // the request was mailed because the notifier was busy, waiting longer would lose it
    if ( !m_isBusy || busyProcessingMailbox )
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

            // the new subscription may be behind a delivery in progress, see NotifyObservers()
            if ( GUCEF_NULL != m_newSubscribersOfDelivery )
                m_newSubscribersOfDelivery->push_back( observer );

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
                if ( !(*i).second && GUCEF_NULL != m_newSubscribersOfDelivery )
                    m_newSubscribersOfDelivery->push_back( observer );
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
            ++m_subscriptionErasures;
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
                ++m_subscriptionErasures;
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

        // When an observer is destroyed while we deliver an event the mail waits for the end of that delivery:
        // processing it here would change the administration under the feet of the delivery
        // While the mailbox is processed the frame which processes it takes care of a scheduled destruction
        if ( !busyProcessingMailbox && !wasBusy )
        {
            if ( !m_scheduledForDestruction )
                ProcessMailbox();
            if ( m_scheduledForDestruction )
            {
                NotificationUnlock();
                DestroyScheduledOwner();
                return;
            }
        }
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
            TEventNotificationMap::iterator i( eventObservers.find( observer ) );
            if ( i != eventObservers.end() )
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
                ++m_subscriptionErasures;
                eventObservers.erase( i );
                m_isBusy = wasBusy;                    
                return;
            }
        }

        m_isBusy = wasBusy;

        // While the mailbox is processed the frame which processes it takes care of a scheduled destruction
        if ( !busyProcessingMailbox )
        {
            if ( !m_scheduledForDestruction )
                ProcessMailbox();
            if ( m_scheduledForDestruction )
            {
                lock.EarlyUnlock();
                DestroyScheduledOwner();
                return;
            }
        }
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
    bool ownerAlive = true;
    try
    {
        ownerAlive = ForceNotifyObserversOnce( CNotifier::ModifyEvent );
    }
    catch ( ... )
    {
        // An observer threw: the exception leaves the delivery, which ends as it would have otherwise
        if ( m_ownerNotifier == GUCEF_NULL )
            Destroy( this );
        else
        if ( m_scheduledForDestruction )
            DestroyScheduledOwner();
        else
            NotificationUnlock();
        throw;
    }
    if ( ownerAlive )
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

        // The spill list of the observers that got the event is the notifier's own, so it keeps its capacity. One
        // delivery uses it at a time (another one on this notifier is mail, delivered after this one); a nested one
        // would get a list of its own
        TObserverVector nestedSpill;
        const bool usesOwnSpill = !m_notifiedSpillInUse;
        m_notifiedSpillInUse = true;
        CNotifiedObservers notifiedObservers( usesOwnSpill ? m_notifiedSpill : nestedSpill );

        // Subscription requests that arrive while the event is delivered are mail, carried out after an observer got the
        // event. The delivery walks the observers in their order and carries on after the place it is at. The observers
        // that the mail subscribed at a place the walk has passed already get the event first, in their order. That is
        // the order and the set of observers of starting over after every change, without passing the observers that
        // got the event again each time, which made a delivery during which many observers subscribe others quadratic
        TObserverVector newSubscribers;
        TObserverVector* outerNewSubscribers = m_newSubscribersOfDelivery;
        if ( !busyProcessingMailbox )
            m_newSubscribersOfDelivery = &newSubscribers;
        try
        {
            TObserverSet behindWalk;
            std::less< CObserver* > observerOrder;

            // Notify observers that are subscribed to this specific event (if any)
            TNotificationList::iterator l = m_eventobservers.find( eventid );
            if ( l != m_eventobservers.end() )
            {
                TEventNotificationMap* observers = &(*l).second;
                TEventNotificationMap::iterator n = observers->begin();
                CObserver* walkPlace = GUCEF_NULL;
                bool walked = false;
                for ( ;; )
                {
                    TEventNotificationMap::iterator current;
                    bool fromWalk = false;
                    if ( !behindWalk.empty() )
                    {
                        oPtr = *behindWalk.begin();
                        behindWalk.erase( behindWalk.begin() );
                        current = observers->find( oPtr );
                        if ( current == observers->end() )
                            continue;
                    }
                    else
                    {
                        if ( n == observers->end() )
                            break;
                        current = n;
                        oPtr = (*n).first;
                        walkPlace = oPtr;
                        walked = true;
                        fromWalk = true;
                    }
                    const UInt32 erasuresBefore = m_subscriptionErasures;

                    // Observers that already got the event are passed by
                    if ( !notifiedObservers.Contains( oPtr ) )
                    {
                        bool notified = false;

                        // Check if we should perform notification using the generic
                        // handler or a user specified callback.
                        TEventHandlerFunctorInterfaceVector& callbacks = (*current).second;
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
                                        m_newSubscribersOfDelivery = outerNewSubscribers;
                                        m_isBusy = wasBusy;
                                        if ( usesOwnSpill )
                                            m_notifiedSpillInUse = false;
                                        lock.EarlyUnlock();
                                        Destroy( this );
                                        return false;
                                    }

                                    // The observer may be gone, destroyed by its callback: then so are its callbacks
                                    if ( !IsSubscribedToEvent( oPtr, eventid ) )
                                        break;
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
                                m_newSubscribersOfDelivery = outerNewSubscribers;
                                m_isBusy = wasBusy;
                                if ( usesOwnSpill )
                                    m_notifiedSpillInUse = false;
                                lock.EarlyUnlock();
                                Destroy( this );
                                return false;
                            }

                            // Add the observer to our 'notified' list
                            notifiedObservers.Add( oPtr );

                            // Process command mail if needed
                            if ( !busyProcessingMailbox && !m_cmdMailStack.empty() )
                                ProcessCmdMailbox();
                        }
                    }

                    // Observers the mail subscribed at a place the walk has passed get the event before the walk goes on
                    if ( !newSubscribers.empty() )
                    {
                        if ( walked )
                        {
                            for ( TObserverVector::iterator s = newSubscribers.begin(); s != newSubscribers.end(); ++s )
                                if ( observerOrder( (*s), walkPlace ) )
                                    behindWalk.insert( (*s) );
                        }
                        newSubscribers.clear();
                    }

                    // The administration may have changed: by the mail, or by an observer that was destroyed while it was
                    // notified (its destruction cannot wait for the mailbox). The observers of the event may be gone
                    // altogether, else the walk carries on after its place. Without anything taken out the iterators hold:
                    // the walk steps on, which meets observers subscribed after its place as well
                    if ( erasuresBefore == m_subscriptionErasures )
                    {
                        if ( fromWalk )
                            ++n;
                        continue;
                    }
                    l = m_eventobservers.find( eventid );
                    if ( l == m_eventobservers.end() )
                        break;
                    observers = &(*l).second;
                    n = walked ? observers->upper_bound( walkPlace ) : observers->begin();
                }
            }

            // Now we process observers that are subscribed to 'all' events
            // these are catch-all subscriptions. This walk starts at the beginning, it meets every subscription made so far
            newSubscribers.clear();
            behindWalk.clear();
            {
                TObserverList::iterator i = m_observers.begin();
                CObserver* walkPlace = GUCEF_NULL;
                bool walked = false;
                for ( ;; )
                {
                    TObserverList::iterator current;
                    bool fromWalk = false;
                    if ( !behindWalk.empty() )
                    {
                        oPtr = *behindWalk.begin();
                        behindWalk.erase( behindWalk.begin() );
                        current = m_observers.find( oPtr );
                        if ( current == m_observers.end() )
                            continue;
                    }
                    else
                    {
                        if ( i == m_observers.end() )
                            break;
                        current = i;
                        oPtr = (*i).first;
                        walkPlace = oPtr;
                        walked = true;
                        fromWalk = true;
                    }
                    const UInt32 erasuresBefore = m_subscriptionErasures;

                    // Check if this observer is registered as interested in all events which is just a flag
                    // and if we have not already notified this observer using a more dedicated subscription and potentially dedicated callback
                    if ( (*current).second && !notifiedObservers.Contains( oPtr ) )
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
                            m_newSubscribersOfDelivery = outerNewSubscribers;
                            m_isBusy = wasBusy;
                            if ( usesOwnSpill )
                                m_notifiedSpillInUse = false;
                            lock.EarlyUnlock();
                            Destroy( this );
                            return false;
                        }

                        // Add the observer to our 'notified' list
                        notifiedObservers.Add( oPtr );

                        // Process command mail if needed
                        if ( !busyProcessingMailbox && !m_cmdMailStack.empty() )
                            ProcessCmdMailbox();
                    }

                    // Observers the mail subscribed at a place the walk has passed get the event before the walk goes on
                    if ( !newSubscribers.empty() )
                    {
                        if ( walked )
                        {
                            for ( TObserverVector::iterator s = newSubscribers.begin(); s != newSubscribers.end(); ++s )
                                if ( observerOrder( (*s), walkPlace ) )
                                    behindWalk.insert( (*s) );
                        }
                        newSubscribers.clear();
                    }

                    // The observer may have been removed while it was notified: the walk carries on after its place.
                    // Without anything taken out the iterator holds and the walk steps on
                    if ( erasuresBefore == m_subscriptionErasures )
                    {
                        if ( fromWalk )
                            ++i;
                        continue;
                    }
                    i = walked ? m_observers.upper_bound( walkPlace ) : m_observers.begin();
                }
            }
        }
        catch ( ... )
        {
            // An observer threw: the exception leaves the delivery, and the notifier is left as a delivery that ends
            // leaves it, so it delivers its later events and carries out subscription requests as usual
            m_newSubscribersOfDelivery = outerNewSubscribers;
            m_isBusy = wasBusy;
            if ( usesOwnSpill )
                m_notifiedSpillInUse = false;
            if ( m_ownerNotifier == GUCEF_NULL )
            {
                lock.EarlyUnlock();
                Destroy( this );
                throw;
            }
            if ( !busyProcessingMailbox )
            {
                if ( !m_scheduledForDestruction )
                    ProcessMailbox();
                if ( m_scheduledForDestruction )
                {
                    lock.EarlyUnlock();
                    DestroyScheduledOwner();
                }
            }
            throw;
        }

        m_newSubscribersOfDelivery = outerNewSubscribers;
        m_isBusy = wasBusy;
        if ( usesOwnSpill )
            m_notifiedSpillInUse = false;

        // Now we check if any requests where received while we where busy
        // While the mailbox is processed the frame which processes it takes care of a scheduled destruction
        if ( !busyProcessingMailbox )
        {
            if ( !m_scheduledForDestruction )
                ProcessMailbox();
            if ( m_scheduledForDestruction )
            {
                lock.EarlyUnlock();
                DestroyScheduledOwner();
                return false;
            }
        }
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
    // They are carried out in the order in which they were given: an unsubscribe followed by a subscribe
    // must leave the subscription in place
    // Once the owner notifier is scheduled for destruction the remaining mail is no longer carried out
    // The requests are taken in place and the list is cleared once all of them were carried out: taking each one off
    // the front would move all the others every time, and a busy notifier (a pulse generator during a pulse) gets
    // thousands. Requests carried out by a nested call are not seen again: the position is the notifier's own
    while ( m_cmdMailProcessed < m_cmdMailStack.size() && !m_scheduledForDestruction )
    {
        TCmdMailElement cmdMailElement( m_cmdMailStack[ m_cmdMailProcessed ] );
        m_cmdMailStack[ m_cmdMailProcessed ].callback = GUCEF_NULL;
        ++m_cmdMailProcessed;

        if ( cmdMailElement.observerIsDestroyed )
        {
            // Ignore this mail, the observer is was destroyed after sending it
            GUCEF_DELETE cmdMailElement.callback;
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

        // The mail held a copy of the callback of its own, a subscription makes another copy
        GUCEF_DELETE cmdMailElement.callback;
    }
    if ( m_cmdMailProcessed >= m_cmdMailStack.size() )
    {
        m_cmdMailStack.clear();
        m_cmdMailProcessed = 0;
    }
}

/*-------------------------------------------------------------------------*/

void
CNotifierImplementor::ProcessEventMailbox( void )
{GUCEF_TRACE;

    // Now we check if any notifications requests where received while we where busy
    // They are delivered in the order in which they were sent, all of them
    // Once the owner notifier is scheduled for destruction the remaining mail is no longer delivered
    // The mail is taken in place and the list is cleared once all of it was delivered: taking each one off the front
    // would move all the others every time, which made the delivery of much mail quadratic in its amount (a busy
    // connection mails every event it raises while it delivers one). Mail delivered by a nested call is not seen
    // again: the position is the notifier's own
    while ( m_eventMailProcessed < m_eventMailStack.size() && !m_scheduledForDestruction )
    {
        TEventMailElement entry( m_eventMailStack[ m_eventMailProcessed ] );
        m_eventMailStack[ m_eventMailProcessed ].eventData = GUCEF_NULL;
        ++m_eventMailProcessed;

        if ( !entry.specificObserverIsDestroyed )
        {
            // Dispatch the message in our mailbox
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
    if ( m_eventMailProcessed >= m_eventMailStack.size() )
    {
        m_eventMailStack.clear();
        m_eventMailProcessed = 0;
    }
}

/*-------------------------------------------------------------------------*/

bool
CNotifierImplementor::IsSubscribedToEvent( CObserver* observer, const CEvent& eventid ) const
{GUCEF_TRACE;

    TNotificationList::const_iterator l = m_eventobservers.find( eventid );
    if ( l == m_eventobservers.end() )
        return false;
    return (*l).second.find( observer ) != (*l).second.end();
}

/*-------------------------------------------------------------------------*/

void
CNotifierImplementor::ProcessMailbox( void )
{GUCEF_TRACE;

    ProcessCmdMailbox();
    ProcessEventMailbox();
}

/*-------------------------------------------------------------------------*/

void
CNotifierImplementor::DestroyScheduledOwner( void )
{GUCEF_TRACE;

    // The owner keeps its link to this object so that its destruction runs OnDeathOfOwnerNotifier()
    m_scheduledForDestruction = false;
    CNotifier* ownerNotifier = m_ownerNotifier;
    GUCEF_DELETE ownerNotifier;
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
                try
                {
                    specificObserver.OnNotify( &sender   ,
                                               eventid   ,
                                               eventData );
                }
                catch ( ... )
                {
                    // An observer threw: the exception leaves the delivery, and the notifier is left as after a
                    // delivery, see NotifyObservers()
                    m_isBusy = false;
                    if ( m_ownerNotifier == GUCEF_NULL )
                    {
                        lock.EarlyUnlock();
                        Destroy( this );
                        throw;
                    }
                    if ( !busyProcessingMailbox )
                    {
                        if ( !m_cmdMailStack.empty() && !m_scheduledForDestruction )
                            ProcessCmdMailbox();
                        if ( m_scheduledForDestruction )
                        {
                            lock.EarlyUnlock();
                            DestroyScheduledOwner();
                        }
                    }
                    throw;
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
                // While the mailbox is processed the frame which processes it takes care of a scheduled destruction
                if ( !busyProcessingMailbox )
                {
                    if ( !m_cmdMailStack.empty() && !m_scheduledForDestruction )
                    {
                        // We have command mail
                        ProcessCmdMailbox();
                    }
                    if ( m_scheduledForDestruction )
                    {
                        lock.EarlyUnlock();
                        DestroyScheduledOwner();
                        return false;
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
            try
            {
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
            }
            catch ( ... )
            {
                // An observer threw: the exception leaves the delivery, and the notifier is left as after a
                // delivery, see NotifyObservers()
                m_isBusy = false;
                if ( m_ownerNotifier == GUCEF_NULL )
                {
                    lock.EarlyUnlock();
                    Destroy( this );
                    throw;
                }
                if ( !busyProcessingMailbox )
                {
                    if ( !m_cmdMailStack.empty() && !m_scheduledForDestruction )
                        ProcessCmdMailbox();
                    if ( m_scheduledForDestruction )
                    {
                        lock.EarlyUnlock();
                        DestroyScheduledOwner();
                    }
                }
                throw;
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
                if ( !m_cmdMailStack.empty() && !m_scheduledForDestruction )
                {
                    // We have command mail
                    ProcessMailbox();
                }
            }
        }

        // While the mailbox is processed the frame which processes it takes care of a scheduled destruction
        if ( m_scheduledForDestruction && !busyProcessingMailbox )
        {
            lock.EarlyUnlock();
            DestroyScheduledOwner();
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
