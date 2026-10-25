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

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#define GUCEF_CORE_CTSGNOTIFIER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSGOBSERVER_H
#include "CTSGObserver.h"       /* observer implementation used internally by the CTSGNotifier */
#define GUCEF_CORE_CTSGOBSERVER_H
#endif /* GUCEF_CORE_CTSGOBSERVER_H ? */

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"          /* notification base class */
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

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

class CPulseGenerator;

/*-------------------------------------------------------------------------*/

/**
 *  ThreadSafe Gateway Notifier class a.k.a TSGNotifier class a.k.a CTSGNotifier
 *
 *  Implements a base class for notifiers that can listnen to another notifier
 *  that is dispatching events from within a thread.
 *  Using this mechanism you can use a Notifier-Observer based design while
 *  mixing threadsafe code with non-threadsafe code.
 *  So basicly like the name says,... it is a messaging gateway between
 *  threadsafe code and non-threadsafe code.
 *
 *  Note that if you do not override OnPumpedNotify() then the instance
 *  will simply function as a pass-trough notifier. Override to manipulate
 *  the event data before it is sent to the observers.
 *  Keep in mind that the CTSGNotifier will become the source notifier !!!
 *
 *  Threading: a CTSGNotifier can be created and destroyed on any thread, also while another thread pumps the
 *  pulse generator it is on and while the notifiers it is subscribed to are delivering events. The destructor
 *  waits for a delivery in progress and does not return before the object is unsubscribed everywhere.
 *  An observer's lock is therefore never held while a notifier is called (see CObserver::SignalUpcomingObserverDestruction()
 *  and CPumpedObserver::Shutdown()): a notifier holds its own lock while delivering to an observer, taking
 *  the locks in the opposite order from another thread is a deadlock.
 */
class GUCEF_CORE_PUBLIC_CPP CTSGNotifier : public CNotifier  ,
                                           public CIObserver
{
    public:

    CTSGNotifier( bool allowSameThreadEventsToFlowThrough = true  ,
                  bool forwardAllNotifications            = false );

    CTSGNotifier( PulseGeneratorPtr pulsGenerator                 ,
                  bool allowSameThreadEventsToFlowThrough = true  ,
                  bool forwardAllNotifications            = false );

    CTSGNotifier( const CTSGNotifier& src );

    virtual ~CTSGNotifier() GUCEF_VIRTUAL_OVERRIDE;

    CTSGNotifier& operator=( const CTSGNotifier& src );

    /**
     *  Subscribes to the given notifier to all
     *  notification events. Every event dispatched by
     *  the threadedNotifier will be sent to the
     *  instance of this class.
     */
    bool SubscribeTo( CNotifier* threadedNotifier );

    void UnsubscribeFrom( CNotifier* threadedNotifier );

    /**
     *  Subscribes to the given notifier's four standard
     *  notifier events, if it is not yet subscribed, plus
     *  subscribes to the given custom event.
     */
    void SubscribeTo( CNotifier* threadedNotifier ,
                      const CEvent& eventid       );

    /**
     *  Cancels the subscription at the given threadedNotifier
     *  for the given event.
     *
     *  Note that subscriptions to the standard notifier events
     *  cannot be cancelled, attempts to do so will be ignored.
     */
    void UnsubscribeFrom( CNotifier* threadedNotifier ,
                          const CEvent& eventid       );

    /**
     *  Subscribes the observer component to the four standard
     *  notifier events if it is not yet subscribed plus
     *  subscribes to the given custom event.
     */
    bool SubscribeTo( CNotifier* threadedNotifier         ,
                      const CEvent& eventid               ,
                      CIEventHandlerFunctorBase& callback );

    void UnsubscribeAllFromObserver( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;


    virtual const CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CObserver& AsObserver( void );

    virtual void SetPulseGenerator( PulseGeneratorPtr newPulseGenerator );

    virtual PulseGeneratorPtr GetPulseGenerator( void ) const;

    /**
     *  A pumped observer uses pulse generator events to allow for async
     *  OnNotify processing via OnPumpedNotify()
     *
     *  When doing this it does NOT by default also consider the pulse event itself as a
     *  event message to be propagated. You can change this with SetPropagatePulseEvent()
     *  This allows you to take advantage of the same pulse to trigger other things.
     */
    void SetPropagatePulseEvent( bool propagatePulseEventMsg );

    bool GetPropagatePulseEvent( void ) const;

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus NotificationLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus NotificationUnlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus NotificationReadOnlyLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus NotificationReadOnlyUnlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:
    friend class CTSGObserver;

    /**
     *  Same as NotifyObservers() except the notification is pumped first
     */
    void NotifyObserversFromThread( void );

    /**
     *  Same as NotifyObservers( eventid, CICloneable* ) except the
     *  notification is pumped first.
     */
    void NotifyObserversFromThread( const CEvent& eventid         ,
                                    CICloneable* eventData = NULL );

    /**
     *  Event callback member function.
     *  Implement this in your descending class to handle
     *  notification events.
     *
     *  Note that this call is always made from the thread running the
     *  puls generator. As such it is well suited for linking non-treadsafe
     *  code via an observer behavior to a notifier that operates from
     *  within another thread.
     *
     *  @param notifier the notifier that sent the notification
     *  @param eventid the unique event id for an event
     *  @param eventdata optional notifier defined user data
     */
    virtual void OnPumpedNotify( CNotifier* notifier                 ,
                                 const CEvent& eventid               ,
                                 CICloneable* eventdata = GUCEF_NULL );

    /**
     *  Implemented to satisfy the IObserver interface.
     *  The purpose of this class is thread uniformity in event handling
     *  As such OnPumpedNotify() is correct corresponding handler you may wish to override.
     *  This handler merely forwards to the observer component
     */
    virtual void OnNotify( CNotifier* notifier                ,
                           const CEvent& eventId              ,
                           CICloneable* evenData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    void SignalUpcomingDestruction( void );

    virtual bool PlaceHoldOnNotifications( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ReleaseHoldOnNotifications( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsHoldingNotifications( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    MT::CMutex m_dataLock;

    private:

    CTSGObserver m_tsgObserver;
    bool m_forwardAllNotifications;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTSGNOTIFIER_H  ? */
