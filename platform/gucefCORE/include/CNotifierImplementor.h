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

#ifndef GUCEF_CORE_CNOTIFIERIMPLEMENTOR_H
#define GUCEF_CORE_CNOTIFIERIMPLEMENTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string>
#include <set>
#include <vector>
#include <map>

#ifndef GUCEF_MT_CILOCKABLE_H
#include "gucefMT_CILockable.h"
#define GUCEF_MT_CILOCKABLE_H
#endif /* GUCEF_MT_CILOCKABLE_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CITYPENAMED_H
#include "CITypeNamed.h"
#define GUCEF_CORE_CITYPENAMED_H
#endif /* GUCEF_CORE_CITYPENAMED_H ? */

#ifndef GUCEF_CORE_CEVENT_H
#include "CEvent.h"
#define GUCEF_CORE_CEVENT_H
#endif /* GUCEF_CORE_CEVENT_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CIEVENTHANDLERFUNCTORBASE_H
#include "gucefCORE_CIEventHandlerFunctorBase.h"
#define GUCEF_CORE_CIEVENTHANDLERFUNCTORBASE_H
#endif /* GUCEF_CORE_CIEVENTHANDLERFUNCTORBASE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF { 
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CObserver;
class CNotifier;
class CINotifier;

/*-------------------------------------------------------------------------*/

/**
 *  Internally used class.
 *  This class houses the actual notification mechanism. The reason for a separate
 *  class is simple, robustness. Since you can use notification for all your dynamic
 *  communication links it has to be able to handle all kinds of exotic scenarios.
 *  One of those scenarios is destruction of the notifier as a result of it's notification.
 *  this class makes that scenario something that can be handled safely and care-free.
 */
class GUCEF_HIDDEN CNotifierImplementor : public MT::CILockable
{  
    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const
        { return NotificationLock( lockWaitTimeoutInMs ); }

    virtual MT::TLockStatus Unlock( void ) const
        { return NotificationUnlock(); }

    virtual MT::TLockStatus NotificationLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const;

    virtual MT::TLockStatus NotificationUnlock( void ) const;

    virtual const MT::CILockable* AsLockable( void ) const;

    private:
    friend class CNotifier;
    
    static CNotifierImplementor* Create( CNotifier* ownerNotifier );
    
    static CNotifierImplementor* Create( CNotifier* ownerNotifier ,
                                         const CNotifier& src     );

    static void Destroy( CNotifierImplementor* obj );
        
    CNotifierImplementor( CNotifier* ownerNotifier );
    
    CNotifierImplementor( CNotifier* ownerNotifier ,
                          const CNotifier& src     );
    
    virtual ~CNotifierImplementor();
    
    CNotifierImplementor& operator=( const CNotifierImplementor& src );

    /**
     *  Subscribes the given observer to all
     *  notifier events. Every event dispatched by 
     *  the notifier will be sent to the observer.
     */
    bool Subscribe( CObserver* observer                ,
                    bool busyProcessingMailbox = false );
    
    /**
     *  Subscribes the given observer to the four standard
     *  notifier events if it is not yet subscribed plus
     *  subscribes to the given custom event.
     */    
    bool Subscribe( CObserver* observer                              ,
                    const CEvent& eventid                            ,
                    CIEventHandlerFunctorBase* callback = GUCEF_NULL ,
                    bool busyProcessingMailbox = false               );

    /**
     *  Detaches the given observer from the notifier.
     *  All the observers subscriptions will be cancelled
     *  This includes both standard notifier events as well 
     *  as custom events.
     */    
    void Unsubscribe( CObserver* observer );
                 
    /**     
     *  Cancels the observer's subscription to the given event.
     *
     *  Note that subscriptions to the standard notifier events 
     *  cannot be cancelled, attempts to do so will be ignored.
     */    
    void Unsubscribe( CObserver* observer                ,
                      const CEvent& eventid              ,
                      bool busyProcessingMailbox = false );
    
    /**
     *  Cancels the subscription of all observers subscribed to this notifier
     */
    void UnsubscribeAllFromNotifier( bool busyProcessingMailbox );

    /**
     *  Dispatches the standard CNotifier::ModifyEvent
     *  Useful for a notification system where you only care if
     *  a mutation is performed on an object.
     */
    bool NotifyObservers( void );
    
    /**
     *  Dispatches the given eventid and eventData to all observers
     *  that are subscribed to all events and the observers that are subscribed
     *  to this specific eventid.
     *
     *  Note that the calling thread is the one in which the observer OnNotify 
     *  event handlers will be processed. Keep this in mind when notification
     *  occurs across thread boundaries.
     *
     *  Note that eventData is not copied. So when passing data across threads
     *  consider allocating a copy and passing that in as the data argument.
     */
    bool NotifyObservers( const CEvent& eventid         ,
                          CICloneable* eventData = NULL );

    /**
     *  The same as NotifyObservers( CEvent, CICloneable )
     *  except that the notifier reference passed in the notifications will be the
     *  specified sender and not the sending notifier object.
     *  
     *  Use with great care !!!
     *  Use of this version should be an exception and not standard practice
     */
    bool NotifyObservers( CNotifier& sender                   ,
                          const CEvent& eventid               ,
                          CICloneable* eventData = GUCEF_NULL ,
                          bool busyProcessingMailbox = false  );

    /**
     *  The same as NotifyObservers( CEvent, CICloneable )
     *  except that this allows you to specify the sender yourself.
     *  You will basically be faking an event emitted at the given sender.
     *  
     *  Use with great care !!!
     *  Use of this version should be an exception and not standard practice
     */
    bool NotifyObserversFrom( CNotifier& sender             ,
                              const CEvent& eventid         ,
                              CICloneable* eventData = NULL );    
    
    /**
     *  The same as NotifyObservers( CEvent, CICloneable )
     *  except that this allows you to specify a specific observer the event 
     *  will be sent to ignoring other observers who are subscribed to the 
     *  same event. Note that a valid subscription for the given event is still
     *  validated for notification to the given observer to be processed.
     *  
     *  Use with great care !!!
     *  Use of this version should be an exception and not standard practice
     *  This is typically only needed for exotic internal use!
     */
    bool NotifySpecificObserver( CObserver& specificObserver   ,
                                 const CEvent& eventid         ,
                                 CICloneable* eventData = NULL );
                          
    /**
     *  The same as NotifyObservers( CNotifier, CEvent, CICloneable )
     *  except that this allows you to specify a specific observer the event 
     *  will be sent to ignoring other observers who are subscribed to the 
     *  same event. Note that a valid subscription for the given event is still
     *  validated for notification to the given observer to be processed.
     *  
     *  Use with great care !!!
     *  Use of this version should be an exception and not standard practice
     *  This is typically only needed for exotic internal use!
     */
    bool NotifySpecificObserver( CNotifier& sender                   ,
                                 CObserver& specificObserver         ,
                                 const CEvent& eventid               ,
                                 CICloneable* eventData = GUCEF_NULL ,
                                 bool busyProcessingMailbox = false  );

    /**
     *  The same as NotifyObservers( CNotifier, CEvent, CICloneable )
     *  except that this allows you to specify a specific observer the event 
     *  will be sent to ignoring other observers who are subscribed to the 
     *  same event. Note that a valid subscription for the given event is still
     *  validated for notification to the given observer to be processed.
     *
     *  The *From variant of this function allows you to specify the sender yourself.
     *  You will basically be faking an event emitted at the given sender.
     *  
     *  Use with great care !!!
     *  Use of this version should be an exception and not standard practice
     *  This is typically only needed for exotic internal use!
     */
    bool NotifySpecificObserverFrom( CNotifier& sender             ,
                                     CObserver& specificObserver   ,
                                     const CEvent& eventid         ,
                                     CICloneable* eventData = NULL );

    /**
     *  Handler for observers that are deleted without having been
     *  unsubscribed first. This results in an observer that is no longer
     *  able to handle notification messages due to the deallocation 
     *  chain of events (descending class is already deallocated).
     *
     *  Note that if you code neatly you will want to call UnsubscribeAll()
     *  from your class destructor that descends from CObserver and implements
     *  OnNotify()
     */
    void OnObserverDestroy( CObserver* observer );
    
    void OnDeathOfOwnerNotifier( void );    
    
    void ScheduleForDestruction( void );
   
    bool ForceNotifyObserversOnce( const CEvent& eventid    ,
                                   CICloneable* data = NULL );

    void UnsubscribeFromAllEvents( CObserver* observer            ,
                                   const bool notifyObserver      ,
                                   const bool observerDestruction ,
                                   bool busyProcessingMailbox     );

    UInt32 GetSubscriptionCountForObserver( CObserver* observer ) const;
    
    void ProcessMailbox( void );
    
    void ProcessCmdMailbox( void );
    
    void ProcessEventMailbox( void );

    // Adds the bi-directional link between the notifier and observer if this is a new link
    // Returns whether the this is a new link
    bool LinkObserver( CObserver* observer          ,
                       bool subscribeToAllEvents    );

    private:
    
    CNotifierImplementor( void );                            /**< should not be implemented */
    CNotifierImplementor( const CNotifierImplementor& src ); /**< should not be implemented */
    
    private:
    
    typedef GUCEF::vector< CIEventHandlerFunctorBase*, gucef_allocator< CIEventHandlerFunctorBase* > > TEventHandlerFunctorInterfaceVector;
    typedef std::pair< CObserver* const, TEventHandlerFunctorInterfaceVector > TObserverRawPtrAndFuncInterfVecPair;
    typedef GUCEF::map< CObserver* const, TEventHandlerFunctorInterfaceVector, std::less< CObserver* const >, gucef_allocator< TObserverRawPtrAndFuncInterfVecPair > > TEventNotificationMap;
    
    typedef GUCEF::set< CObserver*, std::less< CObserver* >, gucef_allocator< CObserver* >  > TObserverSet;
    typedef std::pair< const CEvent, TEventNotificationMap > TEventAndEventNotificationMapPair;
    typedef GUCEF::map< CEvent, TEventNotificationMap, std::less< CEvent >, gucef_allocator< TEventAndEventNotificationMapPair > > TNotificationList;
    typedef std::pair< CObserver* const, bool >   TObserverRawPtrAndBoolPair;
    typedef GUCEF::map< CObserver* const, bool, std::less< CObserver* const >, gucef_allocator< TObserverRawPtrAndBoolPair > > TObserverList;

    struct SEventMailElement
    {
        CNotifier* sender;
        CEvent eventID;
        CICloneable* eventData;
        CObserver* specificObserver;
        bool specificObserverIsDestroyed;
    };
    typedef struct SEventMailElement TEventMailElement;
    
    enum ECmdType
    {
        REQUEST_SUBSCRIBE       ,
        REQUEST_UNSUBSCRIBE     ,
        REQUEST_UNSUBSCRIBE_ALL
    };
    typedef enum ECmdType TCmdType;
    
    struct SCmdMailElement
    {
        TCmdType cmdType;
        CObserver* observer;
        CIEventHandlerFunctorBase* callback;
        CEvent eventID;
        bool notify;
        bool observerIsDestroyed;
    };
    typedef struct SCmdMailElement TCmdMailElement;

    typedef GUCEF::vector< TEventMailElement, gucef_allocator< TEventMailElement > > TEventMailVector;
    typedef GUCEF::vector< TCmdMailElement, gucef_allocator< TCmdMailElement > >     TCmdMailVector;

    private:
    
    CNotifier* m_ownerNotifier;
    
    TNotificationList m_eventobservers;
    TObserverList m_observers;
    
    bool m_isBusy;
    bool m_scheduledForDestruction;
    TEventMailVector m_eventMailStack;
    TCmdMailVector m_cmdMailStack;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CNOTIFIERIMPLEMENTOR_H ? */
