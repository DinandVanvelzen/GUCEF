/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CPUMPEDOBSERVER_H
#define GUCEF_CORE_CPUMPEDOBSERVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTMAILBOX_H
#include "gucefMT_CTMailBox.h"
#define GUCEF_MT_CTMAILBOX_H
#endif /* GUCEF_MT_CTMAILBOX_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTORPROXY_H
#include "gucefCORE_CTEventHandlerFunctorProxy.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTORPROXY_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTORPROXY_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

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

class GUCEF_CORE_PUBLIC_CPP CPumpedObserver : public CObserver
{
    public:

    CPumpedObserver( bool allowSameThreadEventsToFlowThrough = true );

    CPumpedObserver( PulseGeneratorPtr pulsGenerator                ,
                     bool allowSameThreadEventsToFlowThrough = true );

    CPumpedObserver( const CPumpedObserver& src );

    virtual ~CPumpedObserver();

    CPumpedObserver& operator=( const CPumpedObserver& src );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetPulseGenerator( PulseGeneratorPtr newPulseGenerator );
    
    PulseGeneratorPtr GetPulseGenerator( void ) const;

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

    void ClearMailbox( bool acceptNewMail );

    bool AddEventToMailbox( CNotifier* notifier                 , 
                            const CEvent& eventId               ,
                            CICloneable* eventData = GUCEF_NULL );

    protected:

    /**
     *  Event callback member function.
     *  Implement this in your descending class to handle
     *  notification events.
     *
     *  Note that in contrast to OnNotify, which is performed in
     *  the calling thread, this call is always made in the main
     *  application thread or whatever thread provided the pulse generator. 
     *  As such it is well suited for linking
     *  non-tread safe code via an observer to a notifier that
     *  operates from within another thread.
     *
     *  @param notifier the notifier that sent the notification
     *  @param eventid the unique event id for an event
     *  @param eventdata optional notifier defined userdata
     */
    virtual void OnPumpedNotify( CNotifier* notifier                 ,
                                 const CEvent& eventid               ,
                                 CICloneable* eventdata = GUCEF_NULL );

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /**
     *  Event callback member function.
     *  Places the given event in the mailbox where is awaits
     *  the next pump cycle.
     *
     *  Note: DO NOT OVERRIDE !!!
     *
     *  @param notifier the notifier that sent the notification
     *  @param eventId the unique event id for an event
     *  @param eventData optional notifier defined userdata
     *  @param callback the functor callback to be invoked
     */
    void OnFunctorNotify( CNotifier* notifier                 ,
                          const CEvent& eventId               ,
                          CICloneable* eventData              ,
                          CIEventHandlerFunctorBase* callback );

    /**
     *  Event callback member function.
     *  Places the given event in the mailbox where is awaits
     *  the next pump cycle.
     *
     *  Note: DO NOT OVERRIDE !!!
     *
     *  @param notifier the notifier that sent the notification
     *  @param eventid the unique event id for an event
     *  @param eventdata optional notifier defined userdata
     */
    virtual void OnNotify( CNotifier* notifier           ,
                           const CEvent& eventid         ,
                           CICloneable* eventdata = NULL ) GUCEF_VIRTUAL_OVERRIDE;
    
    bool ProxySubscribeTo( CNotifier* threadedNotifier         ,
                           const CEvent& eventid               ,
                           CIEventHandlerFunctorBase& callback );

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus NotificationLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus NotificationUnlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus NotificationReadOnlyLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus NotificationReadOnlyUnlock( void ) const GUCEF_VIRTUAL_OVERRIDE; 

    virtual bool PlaceHoldOnNotifications( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ReleaseHoldOnNotifications( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsHoldingNotifications( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void Shutdown( void );

    private:
    typedef CTEventHandlerFunctor< CPumpedObserver > TEventCallback;
    typedef CTEventHandlerFunctorProxy< CPumpedObserver > TEventCallbackProxy;

    /**
     *  Event callback member function.
     *  Invokes the functor callback from the pumped thread
     *
     *  @param notifier the notifier that sent the notification
     *  @param eventId the unique event id for an event
     *  @param eventData optional notifier defined userdata
     *  @param callback the functor callback to be invoked
     */
    void OnPumpedFunctorNotify( CNotifier* notifier                 ,
                                const CEvent& eventId               ,
                                CICloneable* eventData              ,
                                CIEventHandlerFunctorBase* callback );

    void OnPulse( CNotifier* notifier           ,
                  const CEvent& eventid         ,
                  CICloneable* eventdata = NULL );

    void RegisterPulseGeneratorEventHandlers( void );

    void ClearNotifierReferencesFromMailbox( CNotifier* notifier );
    
    private:

    PulseGeneratorPtr m_pulseGenerator;
    bool m_propagatePulseEvent;
    bool m_allowSameThreadEventsToFlowThrough;
    MT::CTMailBox< CEvent > m_mailbox;
    mutable Int32 m_notificationsHolds;
    CICloneable* m_mailBeingDelivered;             /**< mail OnPulse() is delivering, it stays owned by OnPulse() */
    bool m_mailBeingDeliveredLostItsNotifier;      /**< its notifier was destroyed while it was being delivered */
    const CNotifier* m_pulseGeneratorBeingLinked;  /**< pulse generator this observer subscribes to or unsubscribes from right now */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CPUMPEDOBSERVER_H  ? */
