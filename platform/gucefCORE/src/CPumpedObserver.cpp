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
        TEventCallback callback( this, &CPumpedObserver::OnPulse );
        SubscribeTo( pulseGenerator.GetPointerAlways() ,
                     CPulseGenerator::PulseEvent         ,
                     callback                            );

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
     *  The lock of this observer is not held here, on purpose.
     *  The notification lock of a pumped observer is the lock of its mailbox. The pulse generator holds its own
     *  notification lock while it delivers a pulse, which takes the lock of the mailbox of every observer on it.
     *  Unsubscribing from the pulse generator takes the notification lock of the pulse generator. Doing that
     *  while holding the mailbox lock is the opposite order: with another thread pumping the pulse generator
     *  that is a deadlock. A thread which is not the pulse thread has to be able to destroy a pumped observer.
     *
     *  ClearMailbox() takes the mailbox lock itself, so it waits for a delivery in progress, and from then on
     *  new mail is refused. SetPulseGenerator() waits for a pulse that is being delivered, after that no more
     *  pulses reach this observer.
     */
    ClearMailbox( false );
    SetPulseGenerator( PulseGeneratorPtr() );
    SignalUpcomingObserverDestruction();
}

/*-------------------------------------------------------------------------*/

void
CPumpedObserver::ClearNotifierReferencesFromMailbox( CNotifier* notifier )
{GUCEF_TRACE;

    MT::CScopeMutex writer( m_mailbox.GetLock() );

    MT::CTMailBox< CEvent >::TMailQueue::iterator i = m_mailbox.begin( writer );
    while ( i != m_mailbox.end() )
    {
        CMailElement* mail = static_cast< CMailElement* >( (*i).data );
        if ( GUCEF_NULL != mail )
        {
            if ( mail->m_notifier == notifier )
            {
                if ( mail == m_mailBeingDelivered )
                {
                    // The notifier was destroyed from within the handling of its own event, on the pumping thread
                    // OnPulse() still uses this mail and will dispose of it, we must leave it in place
                    m_mailBeingDeliveredLostItsNotifier = true;
                    ++i;
                    continue;
                }

                GUCEF_DELETE mail->GetCallback();
                GUCEF_DELETE mail->GetData();
                GUCEF_DELETE mail;
                i = m_mailbox.erase( writer, i );
                continue;
            }
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CPumpedObserver::ClearMailbox( bool acceptNewMail )
{GUCEF_TRACE;

    if ( !acceptNewMail )
        m_mailbox.SetAcceptsNewMail( acceptNewMail );

    CEvent mailEventID;
    CICloneable* dataptr( GUCEF_NULL );
    CMailElement* maildata( GUCEF_NULL );
    while ( m_mailbox.GetMail( mailEventID ,
                               &dataptr    ) )
    {
        maildata = static_cast< CMailElement* >( dataptr );
        if ( GUCEF_NULL != maildata )
        {
            GUCEF_DELETE maildata->GetCallback();
            GUCEF_DELETE maildata->GetData();
            GUCEF_DELETE maildata;
        }
    }

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
            UnsubscribeFrom( *oldPulseGenerator.GetPointerAlways() );
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
        MT::CScopeMutex lock( m_mailbox.GetLock() );
        
        CEvent mailEventID;
        CICloneable* dataptr( GUCEF_NULL );
        CMailElement* maildata( GUCEF_NULL );
        while ( m_mailbox.PeekMail( lock        ,
                                    mailEventID ,
                                    &dataptr    ) )
        {
            try
            {
                maildata = static_cast< CMailElement* >( dataptr );
                if ( GUCEF_NULL != maildata )
                {
                    CICloneable* eventData = maildata->GetData();
                    CIEventHandlerFunctorBase* callback = maildata->GetCallback();
                    CNotifier* eventNotifier = maildata->GetNotifier();

                    // Handlers can destroy the notifier of the mail being delivered, which purges its mail
                    // That purge has to leave this mail alone, see ClearNotifierReferencesFromMailbox()
                    m_mailBeingDelivered = maildata;
                    m_mailBeingDeliveredLostItsNotifier = false;

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

                        GUCEF_DELETE callback;
                    }

                    GUCEF_DELETE eventData;
                }
            }
            catch ( const timeout_exception& )
            {
                m_mailBeingDelivered = GUCEF_NULL;
                if ( m_mailBeingDeliveredLostItsNotifier )
                {
                    // There is no retrying an event of a notifier which no longer exists
                    GUCEF_DELETE maildata->GetCallback();
                    GUCEF_DELETE maildata->GetData();
                    m_mailbox.PopMail( lock );
                    m_mailBeingDeliveredLostItsNotifier = false;
                }

                // no luck, perhaps better luck next pulse
                GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, "PumpedObserver: caught timeout_exception while attempting to notify regarding event " + eventid.GetName() );
                return;
            }

            m_mailBeingDelivered = GUCEF_NULL;
            m_mailBeingDeliveredLostItsNotifier = false;
            m_mailbox.PopMail( lock );
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

    CMailElement maildata( notifier, eventData );
    return m_mailbox.AddMail( eventId, &maildata );
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

    m_mailbox.AddMail( eventid   ,
                       &maildata );
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

    return m_mailbox.GetLock().Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CPumpedObserver::NotificationUnlock( void ) const
{GUCEF_TRACE;

    return m_mailbox.GetLock().Unlock();
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CPumpedObserver::NotificationReadOnlyLock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_mailbox.GetLock().Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CPumpedObserver::NotificationReadOnlyUnlock( void ) const
{GUCEF_TRACE;

    return m_mailbox.GetLock().Unlock();
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
