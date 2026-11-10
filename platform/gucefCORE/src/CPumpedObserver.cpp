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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <assert.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#include "CPumpedObserver.h"

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

/**
 *  Internally used data container class used for mailing
 *  extra data using the CMailBox class.
 */
class CMailElement : public CICloneable
{
    private:

    CICloneable* m_eventdata;
    CNotifier* m_notifier;
    CIEventHandlerFunctorBase* m_callback;

    CMailElement( void ); /**< dummy, dont use */

    private:
    friend class CPumpedObserver;

    CMailElement( CNotifier* notifier                              ,
                  CICloneable* eventdata                           ,
                  CIEventHandlerFunctorBase* callback = GUCEF_NULL )
        : m_notifier( notifier )
        , m_eventdata( eventdata )
        , m_callback( callback )
    {GUCEF_TRACE;

    }

    CMailElement( const CMailElement& src )
        : m_notifier( src.m_notifier )
        , m_eventdata( src.m_eventdata )
        , m_callback( src.m_callback )
    {GUCEF_TRACE;

    }

    virtual ~CMailElement()
    {GUCEF_TRACE;

    }

    CMailElement& operator=( const CMailElement& src )
    {GUCEF_TRACE;

        if ( this != &src )
        {
            m_notifier = src.m_notifier;
            m_eventdata = src.m_eventdata;
            m_callback = src.m_callback;
        }
        return *this;
    }

    inline CNotifier* GetNotifier( void ) const
    {GUCEF_TRACE;

        return m_notifier;
    }

    inline CICloneable* GetData( void ) const
    {GUCEF_TRACE;

        return m_eventdata;
    }

    inline CIEventHandlerFunctorBase* GetCallback( void ) const
    {GUCEF_TRACE;

        return m_callback;
    }

    virtual CICloneable* Clone( void ) const
    {GUCEF_TRACE;

        return GUCEF_NEW CMailElement( *this );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CPumpedObserver::CPumpedObserver( bool allowSameThreadEventsToFlowThrough )
    : CObserver()
    , m_pulseGenerator( CCoreGlobal::Instance()->GetPulseGenerator() )
    , m_propagatePulseEvent( false )
    , m_allowSameThreadEventsToFlowThrough( allowSameThreadEventsToFlowThrough )
    , m_mailbox()
    , m_notificationsHolds( 0 )
    , m_mailBeingDelivered( GUCEF_NULL )
    , m_mailBeingDeliveredLostItsNotifier( false )
    , m_pulseGeneratorBeingLinked( GUCEF_NULL )
    , m_deliveringThreadId( 0 )
    , m_deliveryLock()
    , m_notificationLock()
{GUCEF_TRACE;

    assert( !m_pulseGenerator.IsNULL() );
    RegisterPulseGeneratorEventHandlers();
}

/*-------------------------------------------------------------------------*/

CPumpedObserver::CPumpedObserver( PulseGeneratorPtr pulseGenerator        ,
                                  bool allowSameThreadEventsToFlowThrough )
    : CObserver()
    , m_pulseGenerator( pulseGenerator )
    , m_propagatePulseEvent( false )
    , m_allowSameThreadEventsToFlowThrough( allowSameThreadEventsToFlowThrough )
    , m_mailbox()
    , m_notificationsHolds( 0 )
    , m_mailBeingDelivered( GUCEF_NULL )
    , m_mailBeingDeliveredLostItsNotifier( false )
    , m_pulseGeneratorBeingLinked( GUCEF_NULL )
    , m_deliveringThreadId( 0 )
    , m_deliveryLock()
    , m_notificationLock()
{GUCEF_TRACE;

    RegisterPulseGeneratorEventHandlers();
}

/*-------------------------------------------------------------------------*/

CPumpedObserver::CPumpedObserver( const CPumpedObserver& src )
    : CObserver( src )
    , m_pulseGenerator()
    , m_propagatePulseEvent( src.m_propagatePulseEvent )
    , m_allowSameThreadEventsToFlowThrough( src.m_allowSameThreadEventsToFlowThrough )
    , m_mailbox()
    , m_notificationsHolds( 0 )
    , m_mailBeingDelivered( GUCEF_NULL )
    , m_mailBeingDeliveredLostItsNotifier( false )
    , m_pulseGeneratorBeingLinked( GUCEF_NULL )
    , m_deliveringThreadId( 0 )
    , m_deliveryLock()
    , m_notificationLock()
{GUCEF_TRACE;

    m_pulseGenerator = src.m_pulseGenerator;

    RegisterPulseGeneratorEventHandlers();
}

/*-------------------------------------------------------------------------*/

void
CPumpedObserver::RegisterPulseGeneratorEventHandlers( void )
{GUCEF_TRACE;

    PulseGeneratorPtr pulseGenerator = m_pulseGenerator;
    if ( !pulseGenerator.IsNULL() )
    {
        // The subscribe event of this link is not mail, see OnNotify()
        m_pulseGeneratorBeingLinked = pulseGenerator.GetPointerAlways();
        TEventCallback callback( this, &CPumpedObserver::OnPulse );
        SubscribeTo( pulseGenerator.GetPointerAlways() ,
                     CPulseGenerator::PulseEvent         ,
                     callback                            );
        m_pulseGeneratorBeingLinked = GUCEF_NULL;

        pulseGenerator->RequestPeriodicPulses( this );
    }
}

/*-------------------------------------------------------------------------*/

CPumpedObserver::~CPumpedObserver()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CPumpedObserver::Shutdown( void )
{GUCEF_TRACE;

    /*
     *  The locks of this observer are not held here, on purpose.
     *  The pulse generator holds its own notification lock while it delivers a pulse, which takes the mailbox lock
     *  of every observer on it (a leaf: never held while mail is delivered) and, in the delivery, the notification
     *  lock of this observer (m_notificationLock, a lock of its own) where a thread safe notifier built on it raises
     *  its events. Unsubscribing from the pulse generator takes the notification lock of the pulse generator. Doing
     *  that while holding either lock of this observer is the opposite order: with another thread pumping the pulse
     *  generator that is a deadlock. A thread which is not the pulse thread has to be able to destroy a pumped observer.
     *
     *  ClearMailbox() refuses new mail from then on and lets go of the mail that waits; mail that the pulse thread
     *  is delivering right now is left to that thread (OnPulse() delivers without the mailbox lock).
     *  SignalUpcomingObserverDestruction() takes the lock of every notifier this observer
     *  is subscribed to, the pulse generator included, so it waits for an event or a pulse that is being
     *  delivered by another thread, after that no notifier reaches this observer. Only then is the pulse
     *  generator let go: OnNotify() looks at it on the thread of whichever notifier delivers.
     *  The subscription to the pulse generator is gone with the others, it is not asked for again: a busy pulse
     *  generator would keep that request as mail for an observer that may be destroyed before it is carried out.
     */
    ClearMailbox( false );
    SignalUpcomingObserverDestruction();

    PulseGeneratorPtr pulseGenerator = m_pulseGenerator;
    m_pulseGenerator.Unlink();
    if ( !pulseGenerator.IsNULL() )
        pulseGenerator->RequestStopOfPeriodicUpdates( this );
}

/*-------------------------------------------------------------------------*/

void
CPumpedObserver::DeleteMail( TMailElementVector& mail )
{GUCEF_TRACE;

    for ( TMailElementVector::iterator i = mail.begin(); i != mail.end(); ++i )
    {
        CMailElement* element = static_cast< CMailElement* >( (*i) );
        GUCEF_DELETE element->GetCallback();
        GUCEF_DELETE element->GetData();
        GUCEF_DELETE element;
    }
    mail.clear();
}

/*-------------------------------------------------------------------------*/

void
CPumpedObserver::ClearNotifierReferencesFromMailbox( CNotifier* notifier )
{GUCEF_TRACE;

    TMailElementVector purged;
    {
        MT::CScopeMutex writer( m_mailbox.GetLock() );

        /*
         *  Mail of the notifier that another thread is delivering right now (OnPulse() delivers without the mailbox
         *  lock) may still use the notifier: the notifier goes only after that delivery. The delivery lock is held for
         *  exactly one delivery, it is taken without the mailbox lock so the mailbox lock stays a leaf. When the
         *  delivering thread is this one, the notifier is destroyed from within the handling of its own event: OnPulse()
         *  still uses this mail and will dispose of it, it has to stay in place.
         */
        while ( GUCEF_NULL != m_mailBeingDelivered &&
                static_cast< CMailElement* >( m_mailBeingDelivered )->GetNotifier() == notifier )
        {
            if ( m_deliveringThreadId == MT::GetCurrentTaskID() )
            {
                m_mailBeingDeliveredLostItsNotifier = true;
                break;
            }

            const CICloneable* waitedFor = m_mailBeingDelivered;
            writer.EarlyUnlock();
            {
                MT::CScopeMutex delivery( m_deliveryLock );
            }
            writer.ReLock();

            // the delivering thread takes the delivery lock right after it gave up the mailbox lock: give it the time
            if ( waitedFor == m_mailBeingDelivered )
            {
                writer.EarlyUnlock();
                MT::ThreadDelay( 0 );
                writer.ReLock();
            }
        }

        MT::CTMailBox< CEvent >::TMailQueue::iterator i = m_mailbox.begin( writer );
        while ( i != m_mailbox.end() )
        {
            CMailElement* mail = static_cast< CMailElement* >( (*i).data );
            if ( GUCEF_NULL != mail && mail->m_notifier == notifier && mail != m_mailBeingDelivered )
            {
                purged.push_back( mail );
                i = m_mailbox.erase( writer, i );
                continue;
            }
            ++i;
        }
    }

    // the event data may own objects whose destruction takes locks of their own: let go of it without the mailbox lock
    DeleteMail( purged );
}

/*-------------------------------------------------------------------------*/

void
CPumpedObserver::DiscardMailOf( CNotifier* notifier )
{GUCEF_TRACE;

    ClearNotifierReferencesFromMailbox( notifier );
}

/*-------------------------------------------------------------------------*/

void
CPumpedObserver::ClearMailbox( bool acceptNewMail )
{GUCEF_TRACE;

    if ( !acceptNewMail )
        m_mailbox.SetAcceptsNewMail( acceptNewMail );

    TMailElementVector cleared;
    {
        // mail that the pulse thread is delivering right now stays: that thread disposes of it (OnPulse())
        MT::CScopeMutex writer( m_mailbox.GetLock() );
        MT::CTMailBox< CEvent >::TMailQueue::iterator i = m_mailbox.begin( writer );
        while ( i != m_mailbox.end() )
        {
            CMailElement* mail = static_cast< CMailElement* >( (*i).data );
            if ( mail != m_mailBeingDelivered )
            {
                if ( GUCEF_NULL != mail )
                    cleared.push_back( mail );
                i = m_mailbox.erase( writer, i );
                continue;
            }
            ++i;
        }
    }
    DeleteMail( cleared );

    if ( acceptNewMail )
        m_mailbox.SetAcceptsNewMail( acceptNewMail );
}

/*-------------------------------------------------------------------------*/

CPumpedObserver&
CPumpedObserver::operator=( const CPumpedObserver& src )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

void
CPumpedObserver::SetPulseGenerator( PulseGeneratorPtr newPulseGenerator )
{GUCEF_TRACE;

    if ( m_pulseGenerator != newPulseGenerator )
    {
        PulseGeneratorPtr oldPulseGenerator = m_pulseGenerator;
        m_pulseGenerator.Unlink();

        if ( !oldPulseGenerator.IsNULL() )
        {
            oldPulseGenerator->RequestStopOfPeriodicUpdates( this );

            // The unsubscribe event of this link is not mail, see OnNotify()
            m_pulseGeneratorBeingLinked = oldPulseGenerator.GetPointerAlways();
            UnsubscribeFrom( *oldPulseGenerator.GetPointerAlways() );
            m_pulseGeneratorBeingLinked = GUCEF_NULL;
        }

        m_pulseGenerator = newPulseGenerator;
        RegisterPulseGeneratorEventHandlers();
    }
}

/*-------------------------------------------------------------------------*/

PulseGeneratorPtr
CPumpedObserver::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

void
CPumpedObserver::OnPulse( CNotifier* notifier                       ,
                          const CEvent& eventid                     ,
                          CICloneable* eventdata /* = GUCEF_NULL */ )
{GUCEF_TRACE;

    if ( m_notificationsHolds <= 0 )
    {
        /*
         *  The mailbox lock is a leaf: it is held to take the next mail and to let go of it, never while mail is
         *  delivered. A handler that mails another pumped observer from within its mail (a thread safe notifier that
         *  raises an event from thread, CTSGNotifier::NotifyObserversFromThread()) then holds no mailbox lock while
         *  it waits for the lock of the other mailbox, whose pulse thread may do the same in the opposite direction.
         *
         *  The mail stays first in the mailbox while it is delivered, marked as the mail being delivered: mail that
         *  arrives meanwhile goes behind it, ClearNotifierReferencesFromMailbox() and ClearMailbox() leave it to this
         *  thread, and after a timeout it stays for the next pulse. The delivery lock is held for the one delivery: a
         *  thread that destroys the notifier of the mail waits for it (ClearNotifierReferencesFromMailbox()).
         *
         *  A pulse delivers the mail that waited when it began. Mail that arrives meanwhile, which other threads can put
         *  in now, waits for the next pulse: a thread that mails as fast as the mail is delivered would otherwise keep
         *  this pulse, and every other observer of the pulse generator, waiting without end.
         */
        MT::CScopeMutex lock( m_mailbox.GetLock() );
        UInt32 mailOfThisPulse = m_mailbox.AmountOfMail();
        CEvent mailEventID;
        CICloneable* dataptr( GUCEF_NULL );
        while ( 0 < mailOfThisPulse &&
                m_mailbox.PeekMail( lock        ,
                                    mailEventID ,
                                    &dataptr    ) )
        {
            --mailOfThisPulse;
            CMailElement* maildata = static_cast< CMailElement* >( dataptr );
            if ( GUCEF_NULL == maildata )
            {
                m_mailbox.PopMail( lock );
                continue;
            }

            CICloneable* eventData = maildata->GetData();
            CIEventHandlerFunctorBase* callback = maildata->GetCallback();
            CNotifier* eventNotifier = maildata->GetNotifier();

            // Handlers can destroy the notifier of the mail being delivered, which purges its mail
            // That purge has to leave this mail alone, see ClearNotifierReferencesFromMailbox()
            m_mailBeingDelivered = maildata;
            m_mailBeingDeliveredLostItsNotifier = false;
            m_deliveringThreadId = MT::GetCurrentTaskID();
            lock.EarlyUnlock();

            bool timedOut = false;
            {
                MT::CScopeMutex delivery( m_deliveryLock );
                try
                {
                    if ( GUCEF_NULL == callback )
                    {
                        OnPumpedNotify( eventNotifier ,
                                        mailEventID   ,
                                        eventData     );
                    }
                    else
                    {
                        OnPumpedFunctorNotify( eventNotifier ,
                                               mailEventID   ,
                                               eventData     ,
                                               callback      );
                    }
                }
                catch ( const timeout_exception& )
                {
                    timedOut = true;
                }
            }

            // what the mail carries is ours once it was delivered: nobody else touches the mail being delivered
            const bool letGo = !timedOut || m_mailBeingDeliveredLostItsNotifier;
            if ( letGo )
            {
                GUCEF_DELETE callback;
                GUCEF_DELETE eventData;
            }

            lock.ReLock();
            m_mailBeingDelivered = GUCEF_NULL;
            m_deliveringThreadId = 0;
            m_mailBeingDeliveredLostItsNotifier = false;

            // a delivered mail, or one whose notifier no longer exists (there is no retrying its event), goes;
            // after a timeout the mail stays for the next pulse: no luck, perhaps better luck next pulse
            if ( letGo )
                m_mailbox.PopMail( lock );
            if ( timedOut )
            {
                lock.EarlyUnlock();
                GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, "PumpedObserver: caught timeout_exception while attempting to notify regarding event " << mailEventID.GetName() );
                return;
            }
        }
    }

    if ( m_propagatePulseEvent )
        OnPumpedNotify( notifier, eventid, eventdata );
}

/*-------------------------------------------------------------------------*/

void
CPumpedObserver::SetPropagatePulseEvent( bool propagatePulseEventMsg )
{GUCEF_TRACE;

    m_propagatePulseEvent = propagatePulseEventMsg;
}

/*-------------------------------------------------------------------------*/

bool
CPumpedObserver::GetPropagatePulseEvent( void ) const
{GUCEF_TRACE;

    return m_propagatePulseEvent;
}

/*-------------------------------------------------------------------------*/

void
CPumpedObserver::OnNotify( CNotifier* notifier                       ,
                           const CEvent& eventId                     ,
                           CICloneable* eventData /* = GUCEF_NULL */ )
{GUCEF_TRACE;

    if ( CNotifier::DestructionEvent == eventId )
    {
        ClearNotifierReferencesFromMailbox( notifier );
    }
    else
    if ( ( CNotifier::SubscribeEvent == eventId || CNotifier::UnsubscribeEvent == eventId ) &&
         GUCEF_NULL != notifier && notifier == m_pulseGeneratorBeingLinked                      )
    {
        // The (un)subscription to the pulse generator is how this observer gets its pulses, it is not mail for the
        // observer. Mailed, it would wait for a pulse: never, when the pulse generator is being left behind. An
        // observer which moves between pulse generators, as a task consumer does per task, would pile it up
        return;
    }

    try
    {
        if ( !m_pulseGenerator.IsNULL()                                               &&
             m_allowSameThreadEventsToFlowThrough                                     &&
             m_notificationsHolds <= 0                                                &&
             ( MT::GetCurrentTaskID() == m_pulseGenerator->GetPulseDriverThreadId() ) )
        {
            // We are already in the thread that will pump the events and there is no hold on notifications
            OnPumpedNotify( notifier  ,
                            eventId   ,
                            eventData );
            return;
        }
    }
    catch ( const timeout_exception& )
    {
        // no harm no foul, just put the notification in the mailbox instead
        // less efficient, but we tried
    }

    AddEventToMailbox( notifier, eventId, eventData );
}

/*-------------------------------------------------------------------------*/

bool
CPumpedObserver::AddEventToMailbox( CNotifier* notifier    ,
                                    const CEvent& eventId  ,
                                    CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != eventData )
        eventData = eventData->Clone();

    // The mail element does not own the event data: the mail holds the pointer and whoever takes the mail out deletes
    // the data. Mail that is refused (the observer is shutting down) never gets there, so the copy is ours to delete.
    CMailElement maildata( notifier, eventData );
    if ( m_mailbox.AddMail( eventId, &maildata ) )
        return true;

    GUCEF_DELETE eventData;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPumpedObserver::OnFunctorNotify( CNotifier* notifier                 ,
                                  const CEvent& eventid               ,
                                  CICloneable* eventdata              ,
                                  CIEventHandlerFunctorBase* callback )
{GUCEF_TRACE;

    try
    {
        if ( !m_pulseGenerator.IsNULL()                                               &&
             m_allowSameThreadEventsToFlowThrough                                     &&
             m_notificationsHolds <= 0                                                &&
             ( MT::GetCurrentTaskID() == m_pulseGenerator->GetPulseDriverThreadId() ) )
        {        
            // We are already in the thread that will pump the events and there is no hold on notifications
            OnPumpedFunctorNotify( notifier  ,
                                   eventid   ,
                                   eventdata ,
                                   callback  );
            return;
        }
    }
    catch ( const timeout_exception& )
    {
        // no harm no foul, just put the notification in the mailbox instead
        // less efficient, but we tried
    }

    if ( GUCEF_NULL != eventdata )
    {
        eventdata = eventdata->Clone();
    }
    if ( GUCEF_NULL != callback )
    {
        callback = static_cast< CIEventHandlerFunctorBase* >( callback->Clone() );
    }

    CMailElement maildata( notifier  ,
                           eventdata ,
                           callback  );

    // Refused mail (the observer is shutting down) is not owned by anyone: the copies made above are ours to delete
    if ( !m_mailbox.AddMail( eventid   ,
                             &maildata ) )
    {
        GUCEF_DELETE eventdata;
        GUCEF_DELETE callback;
    }
}

/*-------------------------------------------------------------------------*/

void
CPumpedObserver::OnPumpedNotify( CNotifier* notifier                       ,
                                 const CEvent& eventid                     ,
                                 CICloneable* eventdata /* = GUCEF_NULL */ )

{GUCEF_TRACE;

    // dummy, override in decending classes
}

/*-------------------------------------------------------------------------*/

void
CPumpedObserver::OnPumpedFunctorNotify( CNotifier* notifier                 ,
                                        const CEvent& eventId               ,
                                        CICloneable* eventdData             ,
                                        CIEventHandlerFunctorBase* callback )

{GUCEF_TRACE;

    if ( GUCEF_NULL != callback )
    {
        callback->OnNotify( notifier   ,
                            eventId    ,
                            eventdData );
    }
}

/*-------------------------------------------------------------------------*/

bool
CPumpedObserver::ProxySubscribeTo( CNotifier* threadedNotifier         ,
                                   const CEvent& eventid               ,
                                   CIEventHandlerFunctorBase& callback )
{GUCEF_TRACE;

    assert( GUCEF_NULL != &callback );

    if ( GUCEF_NULL != threadedNotifier )
    {
        TEventCallbackProxy callbackProxy( this, &CPumpedObserver::OnFunctorNotify, &callback );
        return threadedNotifier->Subscribe( this           ,
                                            eventid        ,
                                            &callbackProxy );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CPumpedObserver::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CPumpedObserver::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return MT::LOCKSTATUS_NOT_APPLICABLE;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CPumpedObserver::Unlock( void ) const
{GUCEF_TRACE;

    return MT::LOCKSTATUS_NOT_APPLICABLE;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CPumpedObserver::NotificationLock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    // the notification lock of the thread safe notifier this observer serves (CTSGNotifier): a lock of its own, not
    // the mailbox lock, so that events are delivered without the mailbox lock (OnPulse())
    return m_notificationLock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CPumpedObserver::NotificationUnlock( void ) const
{GUCEF_TRACE;

    return m_notificationLock.Unlock();
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CPumpedObserver::NotificationReadOnlyLock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_notificationLock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CPumpedObserver::NotificationReadOnlyUnlock( void ) const
{GUCEF_TRACE;

    return m_notificationLock.Unlock();
}

/*-------------------------------------------------------------------------*/

bool
CPumpedObserver::PlaceHoldOnNotifications( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    ++m_notificationsHolds;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPumpedObserver::ReleaseHoldOnNotifications( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    --m_notificationsHolds;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPumpedObserver::IsHoldingNotifications( void ) const
{GUCEF_TRACE;

    return m_notificationsHolds > 0;
}

/*-------------------------------------------------------------------------*/

const CString&
CPumpedObserver::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString typeName = "GUCEF::CORE::CPumpedObserver";
    return typeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
