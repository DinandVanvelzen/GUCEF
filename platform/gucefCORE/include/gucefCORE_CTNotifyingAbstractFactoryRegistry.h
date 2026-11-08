/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CTNOTIFYINGABSTRACTFACTORYREGISTRY_H
#define GUCEF_CORE_CTNOTIFYINGABSTRACTFACTORYREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CABSTRACTFACTORYBASE_H
#include "gucefCORE_CAbstractFactoryBase.h"
#define GUCEF_CORE_CABSTRACTFACTORYBASE_H
#endif /* GUCEF_CORE_CABSTRACTFACTORYBASE_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORYREGISTRY_H
#include "gucefCORE_CTAbstractFactoryRegistry.h"
#define GUCEF_CORE_CTABSTRACTFACTORYREGISTRY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORYREGISTRY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TEMPLATES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The notification layer of the abstract factories (CTAbstractFactory, CTAbstractFactoryWithParam), on top of the
 *  registry (CTAbstractFactoryRegistry, see there for the rules of creates and removals).
 *
 *  Events (when made with useEventing): one CAbstractFactoryBase::ConcreteFactoryRegisteredEvent per
 *  RegisterConcreteFactory() (a new registration, a replacement, a repeat of the same registration), one
 *  CAbstractFactoryBase::ConcreteFactoryUnregisteredEvent per removed entry (UnregisterConcreteFactory(),
 *  UnregisterAllConcreteFactories() and the destructor, in key order). The event data is a TKeyContainer holding the
 *  type. Events are sent once the operation is complete, on its thread, without the registry lock. The destructor sends
 *  them from its own body, while the notifier is alive; the classes derived from the abstract factory are destroyed by
 *  then, so their hooks are not called for those entries.
 *
 *  Order: the events of one operation, and of the operations of one thread, arrive in their order. The events of
 *  operations on different threads, also for one type, are not ordered: when one thread registers a type while another
 *  unregisters it, an observer may get the unregistered event last although the registration took effect last (the
 *  events go out after the operation, without the lock, after any wait of a removal). IsConstructible() tells the
 *  outcome.
 *
 *  Observers that throw: the exception leaves the operation once it is complete. UnregisterAllConcreteFactories() sends
 *  the event of every entry, also after an observer threw, then passes the first exception on and logs the later ones
 *  as errors; the destructor passes none on and logs each one as an error. Within one event, CNotifier does not notify
 *  the observers after the one that threw.
 *
 *  Destroying the abstract factory from within one of its events (an observer that deletes it) is not supported: the
 *  delivery of the event goes on to use the destroyed notifier and its notification lock.
 *
 *  Notification lock: event dispatch, subscriptions and unsubscriptions are guarded by a lock of their own (of the same
 *  LockType), not by the registry lock, so an observer may remove a type (and wait for the creates through it on other
 *  threads, which take the registry lock to end) from within an event. The factory code never takes the notification
 *  lock while it holds the registry lock; code that holds the registry lock itself does not subscribe to, unsubscribe
 *  from or notify through the abstract factory meanwhile, and a product constructor does not subscribe to or
 *  unsubscribe from the abstract factory that makes it. A removal by code that holds the registry lock does not wait
 *  for the creates of other threads, see CTAbstractFactoryRegistry.
 *
 *  A class derived from it that overrides OnConcreteFactoryRegistered() / OnConcreteFactoryUnregistered() calls these
 *  ones to keep the events.
 */
template< typename SelectionCriteriaType, class TFactoryType, class LockType >
class CTNotifyingAbstractFactoryRegistry : public CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, CAbstractFactoryBase >
{
    public:

    typedef CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, CAbstractFactoryBase >   TRegistry;
    typedef typename TRegistry::TKeyContainer                                                                  TKeyContainer;

    protected:

    CTNotifyingAbstractFactoryRegistry( const bool assumeFactoryOwnership, const bool useEventing );

    CTNotifyingAbstractFactoryRegistry( const CTNotifyingAbstractFactoryRegistry& src );

    /** Removes every entry, waiting as a removal does, with one unregistered event per entry */
    virtual ~CTNotifyingAbstractFactoryRegistry();

    CTNotifyingAbstractFactoryRegistry& operator=( const CTNotifyingAbstractFactoryRegistry& src );

    virtual void OnConcreteFactoryRegistered( const SelectionCriteriaType& selectedType ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnConcreteFactoryUnregistered( const SelectionCriteriaType& selectedType ) GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus NotificationLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus NotificationUnlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus NotificationReadOnlyLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus NotificationReadOnlyUnlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    LockType m_notificationLock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType >
CTNotifyingAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType >::CTNotifyingAbstractFactoryRegistry( const bool assumeFactoryOwnership ,
                                                                                                                         const bool useEventing            )
    : TRegistry( assumeFactoryOwnership, useEventing )
    , m_notificationLock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType >
CTNotifyingAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType >::CTNotifyingAbstractFactoryRegistry( const CTNotifyingAbstractFactoryRegistry& src )
    : TRegistry( src )
    , m_notificationLock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType >
CTNotifyingAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType >::~CTNotifyingAbstractFactoryRegistry()
{GUCEF_TRACE;

    // The events go out while the notifier and the notification lock are alive
    this->RemoveAllEntries( true );
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType >
CTNotifyingAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType >&
CTNotifyingAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType >::operator=( const CTNotifyingAbstractFactoryRegistry& /* src */ )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType >
void
CTNotifyingAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType >::OnConcreteFactoryRegistered( const SelectionCriteriaType& selectedType )
{GUCEF_TRACE;

    if ( this->m_useEventing )
    {
        TKeyContainer keyContainer( selectedType );
        this->NotifyObservers( CAbstractFactoryBase::ConcreteFactoryRegisteredEvent, &keyContainer );
    }
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType >
void
CTNotifyingAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType >::OnConcreteFactoryUnregistered( const SelectionCriteriaType& selectedType )
{GUCEF_TRACE;

    if ( this->m_useEventing )
    {
        TKeyContainer keyContainer( selectedType );
        this->NotifyObservers( CAbstractFactoryBase::ConcreteFactoryUnregisteredEvent, &keyContainer );
    }
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType >
MT::TLockStatus
CTNotifyingAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType >::NotificationLock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_notificationLock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType >
MT::TLockStatus
CTNotifyingAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType >::NotificationUnlock( void ) const
{GUCEF_TRACE;

    return m_notificationLock.Unlock();
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType >
MT::TLockStatus
CTNotifyingAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType >::NotificationReadOnlyLock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_notificationLock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType >
MT::TLockStatus
CTNotifyingAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType >::NotificationReadOnlyUnlock( void ) const
{GUCEF_TRACE;

    return m_notificationLock.Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTNOTIFYINGABSTRACTFACTORYREGISTRY_H ? */
