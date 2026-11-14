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

#ifndef GUCEF_CORE_CINOTIFIER_H
#define GUCEF_CORE_CINOTIFIER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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

/*-------------------------------------------------------------------------*/

/**
 *  Interface class for all notifier implementations
 */
class GUCEF_CORE_PUBLIC_CPP CINotifier : public virtual MT::CILockable ,
                                         public virtual CITypeNamed
{
    public:

    static const CEvent SubscribeEvent;
    static const CEvent UnsubscribeEvent;
    static const CEvent ModifyEvent;
    static const CEvent DestructionEvent;

    static void RegisterEvents( void );

    public:

    CINotifier( bool registerStdEvents = true );

    CINotifier( const CINotifier& src );

    virtual ~CINotifier();

    CINotifier& operator=( const CINotifier& src );

    /**
     *  Subscribes the given observer to all
     *  notifier events. Every event dispatched by
     *  the notifier will be sent to the observer.
     */
    virtual bool Subscribe( CObserver* observer ) = 0;

    /**
     *  Subscribes the given observer to the four standard
     *  notifier events if it is not yet subscribed plus
     *  subscribes to the given custom event.
     */
    virtual bool Subscribe( CObserver* observer                        ,
                            const CEvent& eventid                      ,
                            CIEventHandlerFunctorBase* callback = GUCEF_NULL ) = 0;

    /**
     *  Detaches the given observer from the notifier.
     *  All the observers subscriptions will be cancelled
     *  This includes both standard notifier events as well
     *  as custom events.
     */
    virtual void Unsubscribe( CObserver* observer ) = 0;

    /**
     *  Cancels the observer's subscription to the given event.
     *
     *  Note that subscriptions to the standard notifier events
     *  cannot be cancelled, attempts to do so will be ignored.
     */
    virtual void Unsubscribe( CObserver* observer   ,
                              const CEvent& eventid ) = 0;

    /**
     *  Cancels the subscription of all observers subscribed to this notifier
     */
    virtual void UnsubscribeAllFromNotifier( void ) = 0;

    virtual UInt32 GetSubscriptionCountForObserver( CObserver* observer ) const = 0;

    protected:
    friend class CNotifierImplementor;
    friend class CNotifierScopeLock;
    friend class CNotifierScopeReadOnlyLock;

    /**
     *  Can be implemented by the derived classes such that all notification system interactions
     *  are protected against multiple threads accessing them
     *  Typical implementation would be to have this call Lock() on a mutex member in a derived class
     *
     *  Default inline implementation simply calls the object global Lock() but you may wish to use a 
     *  distinct lock to guard the notification mechanism since it can operate independently
     */
    virtual MT::TLockStatus NotificationLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const 
        { return Lock( lockWaitTimeoutInMs ); }

    /**
     *  Counterpart to the NotificationLock() member function. This releases the lock obtained using NotificationLock() 
     *  Typical implementation would be to have this call Unlock() on a mutex member in a derived class
     */
    virtual MT::TLockStatus NotificationUnlock( void ) const 
        { return Unlock(); }

    /**
     *  Can be implemented by the derived classes such that all notification system interactions 
     *  are protected against multiple threads accessing them
     *  Typical implementation would be to have this call Lock() as a reader on a reader-writer lock in a derived class
     *
     *  Default inline implementation simply calls the object global ReadOnlyLock() but you may wish to use a 
     *  distinct lock to guard the notification mechanism since it can operate independently
     */
    virtual MT::TLockStatus NotificationReadOnlyLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const 
        { return ReadOnlyLock( lockWaitTimeoutInMs ); }

    /**
     *  Counterpart to the NotificationReadOnlyLock() member function. This releases the lock obtained using NotificationReadOnlyLock() 
     *  Typical implementation would be to have this call Unlock() as a reader on a reader-writer lock in a derived class
     */
    virtual MT::TLockStatus NotificationReadOnlyUnlock( void ) const 
        { return ReadOnlyUnlock(); };
};

/*-------------------------------------------------------------------------*/

/**
 *  Leverages the CINotifier interface to NotificationLock() and NotificationUnlock() the scope of an
 *  notification operation while still having the same stack unroll etc advantages of
 *  the CScopeMutex class
 *  Typical usage would be to pass in the 'this' pointer to lock the member function
 *  scope for a object who's class is derived from CINotifier
 */
class GUCEF_CORE_PUBLIC_CPP CNotifierScopeLock
{
    public:

    /**
     *  Locks if a lockable object is provided
     *  Otherwise ( lockableNotifier == GUCEF_NULL ) this object will act as a no-op
     *  This allows you to use the same code in a thread-safe or non-threadsafe manner depending on context
     *  Only taking the locking penalty when needed based on said context
     * 
     *  throws timeout_exception if unable to attain the lock before the specified timeout
     */
    CNotifierScopeLock( const CINotifier* lockableNotifier, UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS );

    /**
     *  Locks the lockable object and retaints the lock for the lifespan of this object by default
     *  You can unlock early via the EarlyUnlock() member function
     * 
     *  throws timeout_exception if unable to attain the lock before the specified timeout
     */
    CNotifierScopeLock( const CINotifier& lockableNotifier, UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS );

    ~CNotifierScopeLock();

    /**
     *  Returns whether the current lock state based on the Lock() and Unlock()
     *  operations on the lockable object
     */
    bool IsLocked( void ) const;

    /**
     *  Allows you to unlock before the scope lock triggers destruction of the CObjectScopeLock object
     *  Useful for more complex code flows where in most code paths you want to retain the lock except for
     *  a small subset of code path(s)
     */
    bool EarlyUnlock( void );

    private:
    const CINotifier* m_lockableNotifier;
    bool m_isLocked;

    CNotifierScopeLock( const CNotifierScopeLock& src );              /* Copying doesnt make sense */
    CNotifierScopeLock& operator=( const CNotifierScopeLock& src );   /* Copying doesnt make sense */
};

/*-------------------------------------------------------------------------*/

/**
 *  Leverages the CINotifier interface to NotificationReadOnlyLock() and NotificationReadOnlyUnlock() the scope of an
 *  notification operation while still having the same stack unroll etc advantages of
 *  the CScopeMutex class
 *  Typical usage would be to pass in the 'this' pointer to lock the member function
 *  scope for a object who's class is derived from CINotifier
 */
class GUCEF_CORE_PUBLIC_CPP CNotifierScopeReadOnlyLock
{
    public:

    /**
     *  Locks if a lockable object is provided
     *  Otherwise ( lockableNotifier == GUCEF_NULL ) this object will act as a no-op
     *  This allows you to use the same code in a thread-safe or non-threadsafe manner depending on context
     *  Only taking the locking penalty when needed based on said context
     * 
     *  throws timeout_exception if unable to attain the lock before the specified timeout
     */
    CNotifierScopeReadOnlyLock( const CINotifier* lockableNotifier, UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS );

    /**
     *  Locks the lockable object and retaints the lock for the lifespan of this object by default
     *  You can unlock early via the EarlyUnlock() member function
     * 
     *  throws timeout_exception if unable to attain the lock before the specified timeout
     */
    CNotifierScopeReadOnlyLock( const CINotifier& lockableNotifier, UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS );

    ~CNotifierScopeReadOnlyLock();

    /**
     *  Returns whether the current lock state based on the Lock() and Unlock()
     *  operations on the lockable object
     */
    bool IsLocked( void ) const;

    /**
     *  Allows you to unlock before the scope lock triggers destruction of the CObjectScopeLock object
     *  Useful for more complex code flows where in most code paths you want to retain the lock except for
     *  a small subset of code path(s)
     */
    bool EarlyReaderUnlock( void );

    private:
    const CINotifier* m_lockableNotifier;
    bool m_isLocked;

    CNotifierScopeReadOnlyLock( const CNotifierScopeReadOnlyLock& src );              /* Copying doesnt make sense */
    CNotifierScopeReadOnlyLock& operator=( const CNotifierScopeReadOnlyLock& src );   /* Copying doesnt make sense */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CINOTIFIER_H ? */
