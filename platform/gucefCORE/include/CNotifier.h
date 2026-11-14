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

#ifndef GUCEF_CORE_CNOTIFIER_H
#define GUCEF_CORE_CNOTIFIER_H

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

#ifndef GUCEF_CORE_CIEVENTHANDLERFUNCTORBASE_H
#include "gucefCORE_CIEventHandlerFunctorBase.h"
#define GUCEF_CORE_CIEVENTHANDLERFUNCTORBASE_H
#endif /* GUCEF_CORE_CIEVENTHANDLERFUNCTORBASE_H ? */

#ifndef GUCEF_CORE_CITYPENAMED_H
#include "CITypeNamed.h"
#define GUCEF_CORE_CITYPENAMED_H
#endif /* GUCEF_CORE_CITYPENAMED_H ? */

#ifndef GUCEF_CORE_CEVENT_H
#include "CEvent.h"
#define GUCEF_CORE_CEVENT_H
#endif /* GUCEF_CORE_CEVENT_H ? */

#ifndef GUCEF_CORE_CINOTIFIER_H
#include "gucefCORE_CINotifier.h"
#define GUCEF_CORE_CINOTIFIER_H
#endif /* GUCEF_CORE_CINOTIFIER_H ? */

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
class CNotifierImplementor;

/*-------------------------------------------------------------------------*/

/**
 *  Use the CNotificationIDRegistry to obtain unique event id's for
 *  your events. It is strongly recommended to include the various namespaces
 *  in the event strings. Following this simply rule will ensure that your strings
 *  are unique simply because your namespace::class::event combination is unique in
 *  C++ as well.
 *
 *  Note that the base class automatically registers four notification events
 *  if they are not already registered. These are:
 *      - "GUCEF::CORE::CINotifier::SubscribeEvent"
 *      - "GUCEF::CORE::CINotifier::UnsubscribeEvent"
 *      - "GUCEF::CORE::CINotifier::ModifyEvent"
 *      - "GUCEF::CORE::CINotifier::DestructionEvent"
 *
 *  Note that every observer will be automatically subscribed to
 *  the four standard notification events.
 *
 *  Note that CNotifier is NOT threadsafe on it's own to avoid introducing
 *  unnecessary overhead. If you want a threadsafe notifier then simply
 *  implement Lock() and Unlock() yourself in your descending class.
 *
 *  Note that notification can cause a chain reaction resulting in the destruction
 *  of the object that triggered the notification. If you wish to safely handle this scenario
 *  you will have to check the boolean return value of the member functions.
 *  The return value is the 'alive' state of the notifier itself, If false then the
 *  notifier has been destroyed. Any attempt to access data in that object will result
 *  in invalid memory access. If the notifier has been destroyed you should exit the code
 *  that called the member function without accessing any data members.
 */
class GUCEF_CORE_PUBLIC_CPP CNotifier : public CINotifier
{
    public:

    CNotifier( bool registerStdEvents = true );

    CNotifier( const CNotifier& src );

    virtual ~CNotifier();

    CNotifier& operator=( const CNotifier& src );

    /**
     *  Subscribes the given observer to all
     *  notifier events. Every event dispatched by
     *  the notifier will be sent to the observer.
     */
    virtual bool Subscribe( CObserver* observer ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Subscribes the given observer to the four standard
     *  notifier events if it is not yet subscribed plus
     *  subscribes to the given custom event.
     */
    virtual bool Subscribe( CObserver* observer                              ,
                            const CEvent& eventid                            ,
                            CIEventHandlerFunctorBase* callback = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Detaches the given observer from the notifier.
     *  All the observers subscriptions will be cancelled
     *  This includes both standard notifier events as well
     *  as custom events.
     */
    virtual void Unsubscribe( CObserver* observer ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Cancels the observer's subscription to the given event.
     *
     *  Note that subscriptions to the standard notifier events
     *  cannot be cancelled, attempts to do so will be ignored.
     */
    virtual void Unsubscribe( CObserver* observer   ,
                              const CEvent& eventid ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Cancels the subscription of all observers subscribed to this notifier
     */
    virtual void UnsubscribeAllFromNotifier( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  descending classes should override this with the class name
     *  ie: a class named CMyClass should implement this member
     *  function as: { return "MyNamespace::CMyClass"; }
     *
     *  Note that this mechanism is meant as a replacement for
     *  RTTI. You can accomplish the same with a dynamic_cast for
     *  example. The catch is that RTTI has an implicit system-wide
     *  performance impact !!!. Since the scenario's where you need
     *  functionality like this are limited or even non-existent
     *  we chose to use this approach.
     */
    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Sets a destruction flag that will cause the notifier object to be
     *  destroyed after notification is finished (if not busy the notifier
     *  is destroyed immediately).
     *  Using this member function is the recommenced method for deleting a
     *  notifier while being notified from this notifier.
     */
    void ScheduleForDestruction( void );

    virtual UInt32 GetSubscriptionCountForObserver( CObserver* observer ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:
    friend class CNotifierImplementor;

    /**
     *  Dispatches the standard CNotifier::ModifyEvent
     *  Useful for a notification system where you only care if
     *  a mutation is performed on an object.
     *
     *  @return Alive state of the notifier, if false then the notifier died itself as a result of the notification.
     */
    bool NotifyObservers( void ) const;

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
     *
     *  @return Alive state of the notifier, if false then the notifier died itself as a result of the notification.
     */
    bool NotifyObservers( const CEvent& eventid         ,
                          CICloneable* eventData = GUCEF_NULL ) const;

    /**
     *  The same as NotifyObservers( CEvent, CICloneable )
     *  except that this allows you to specify the sender yourself.
     *  You will basically be faking an event emitted at the given sender.
     *
     *  Use with great care !!!
     *  Use of this version should be an exception and not standard practice
     *
     *  @return Alive state of the notifier, if false then the notifier died itself as a result of the notification.
     */
    bool NotifyObservers( CNotifier& sender             ,
                          const CEvent& eventid         ,
                          CICloneable* eventData = GUCEF_NULL ) const;

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
                                 CICloneable* eventData = GUCEF_NULL ) const;

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
    bool NotifySpecificObserver( CNotifier& sender             ,
                                 CObserver& specificObserver   ,
                                 const CEvent& eventid         ,
                                 CICloneable* eventData = GUCEF_NULL ) const;
    
    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Certain edge-cases require knowledge of the destructions of
     *  a specific observer. Typically this is not needed since it is handled
     *  automatically. However if you use a function like NotifySpecificObserver()
     *  then you might have use for this function. No-op at this level.
     */
    virtual void OnObserverDestruction( CObserver* observer );

    private:
    friend class CObserver;

    /**
     *  Handler for observers that are deleted without having been
     *  un-subscribed first. This results in an observer that is no longer
     *  able to handle notification messages due to the deallocation
     *  chain of events (descending class is already deallocated).
     *
     *  Note that if you code neatly you will want to call UnsubscribeAllFromObserver()
     *  from your class destructor that descends from CObserver and implements
     *  OnNotify()
     */
    void OnObserverDestroy( CObserver* observer );

    private:

    CNotifierImplementor* m_imp;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CNOTIFIER_H ? */
