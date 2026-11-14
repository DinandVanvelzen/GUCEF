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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#include "CTSGNotifier.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTSGNotifier::CTSGNotifier( bool allowSameThreadEventsToFlowThrough ,
                            bool forwardAllNotifications            )
    : CNotifier()                    
    , m_dataLock()
    , m_tsgObserver( CCoreGlobal::Instance()->GetPulseGenerator(), allowSameThreadEventsToFlowThrough )
    , m_forwardAllNotifications( forwardAllNotifications )
{GUCEF_TRACE;

    m_tsgObserver.SetParent( this );
}

/*-------------------------------------------------------------------------*/

CTSGNotifier::CTSGNotifier( PulseGeneratorPtr pulsGenerator         ,
                            bool allowSameThreadEventsToFlowThrough ,
                            bool forwardAllNotifications            )
    : CNotifier()                    
    , m_dataLock()
    , m_tsgObserver( pulsGenerator, allowSameThreadEventsToFlowThrough )
    , m_forwardAllNotifications( forwardAllNotifications )
{GUCEF_TRACE;

    m_tsgObserver.SetParent( this );
}

/*-------------------------------------------------------------------------*/

CTSGNotifier::CTSGNotifier( const CTSGNotifier& src )
    : CNotifier( src )                   
    , m_dataLock()
    , m_tsgObserver( src.m_tsgObserver )
    , m_forwardAllNotifications( src.m_forwardAllNotifications )
{GUCEF_TRACE;

    m_tsgObserver.SetParent( this );
}

/*-------------------------------------------------------------------------*/

CTSGNotifier::~CTSGNotifier()
{GUCEF_TRACE;
   
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

CTSGNotifier&
CTSGNotifier::operator=( const CTSGNotifier& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CNotifier::operator=( src );
        m_forwardAllNotifications = src.m_forwardAllNotifications;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CTSGNotifier::SubscribeTo( CNotifier* threadedNotifier )
{GUCEF_TRACE;

    if ( GUCEF_NULL != threadedNotifier )
        return threadedNotifier->Subscribe( &m_tsgObserver );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CTSGNotifier::UnsubscribeFrom( CNotifier* threadedNotifier )
{GUCEF_TRACE;

    if ( GUCEF_NULL != threadedNotifier )
        threadedNotifier->Unsubscribe( &m_tsgObserver );
}

/*-------------------------------------------------------------------------*/

void
CTSGNotifier::SubscribeTo( CNotifier* threadedNotifier ,
                           const CEvent& eventid       )
{GUCEF_TRACE;

    if ( GUCEF_NULL != threadedNotifier )
        threadedNotifier->Subscribe( &m_tsgObserver ,
                                     eventid        );
}

/*-------------------------------------------------------------------------*/

void
CTSGNotifier::UnsubscribeFrom( CNotifier* threadedNotifier ,
                               const CEvent& eventid       )
{GUCEF_TRACE;

    if ( GUCEF_NULL != threadedNotifier )
        threadedNotifier->Unsubscribe( &m_tsgObserver ,
                                       eventid        );
}

/*-------------------------------------------------------------------------*/

bool
CTSGNotifier::SubscribeTo( CNotifier* threadedNotifier         ,
                           const CEvent& eventid               ,
                           CIEventHandlerFunctorBase& callback )
{GUCEF_TRACE;

    assert( GUCEF_NULL != &callback );
    return m_tsgObserver.ProxySubscribeTo( threadedNotifier ,
                                           eventid          ,
                                           callback         );
}

/*-------------------------------------------------------------------------*/

void
CTSGNotifier::OnPumpedNotify( CNotifier* notifier                 ,
                              const CEvent& eventid               ,
                              CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( m_forwardAllNotifications )
    {
        NotifyObservers( eventid   ,
                         eventdata );
    }
}

/*-------------------------------------------------------------------------*/

void
CTSGNotifier::SetPropagatePulseEvent( bool propagatePulseEventMsg )
{GUCEF_TRACE;

    m_tsgObserver.SetPropagatePulseEvent( propagatePulseEventMsg );
}

/*-------------------------------------------------------------------------*/

bool
CTSGNotifier::GetPropagatePulseEvent( void ) const
{GUCEF_TRACE;

    return m_tsgObserver.GetPropagatePulseEvent();
}

/*-------------------------------------------------------------------------*/

void
CTSGNotifier::NotifyObserversFromThread( void )
{GUCEF_TRACE;

    m_tsgObserver.AddEventToMailbox( this                   ,
                                     CNotifier::ModifyEvent ,
                                     GUCEF_NULL                   );
}

/*-------------------------------------------------------------------------*/

void
CTSGNotifier::NotifyObserversFromThread( const CEvent& eventid               ,
                                         CICloneable* eventData /* = NULL */ )
{GUCEF_TRACE;

    m_tsgObserver.AddEventToMailbox( this      ,
                                     eventid   ,
                                     eventData );
}

/*-------------------------------------------------------------------------*/

bool
CTSGNotifier::PlaceHoldOnNotifications( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_tsgObserver.PlaceHoldOnNotifications( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

bool
CTSGNotifier::ReleaseHoldOnNotifications( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_tsgObserver.ReleaseHoldOnNotifications( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

bool
CTSGNotifier::IsHoldingNotifications( void ) const
{GUCEF_TRACE;

    return m_tsgObserver.IsHoldingNotifications();
}

/*-------------------------------------------------------------------------*/

const CString&
CTSGNotifier::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CString typeName = "GUCEF::CORE::CTSGNotifier";
    return typeName;
}

/*-------------------------------------------------------------------------*/

const MT::CILockable* 
CTSGNotifier::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTSGNotifier::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE; 

    return m_dataLock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTSGNotifier::Unlock( void ) const
{GUCEF_TRACE;

    return m_dataLock.Unlock();
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTSGNotifier::NotificationLock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE; 

    return m_tsgObserver.NotificationLock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTSGNotifier::NotificationUnlock( void ) const
{GUCEF_TRACE;

    return m_tsgObserver.NotificationUnlock();
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CTSGNotifier::NotificationReadOnlyLock( UInt32 lockWaitTimeoutInMs ) const 
{GUCEF_TRACE; 

    return m_tsgObserver.NotificationReadOnlyLock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CTSGNotifier::NotificationReadOnlyUnlock( void ) const 
{GUCEF_TRACE;

    return m_tsgObserver.NotificationReadOnlyUnlock();
}

/*-------------------------------------------------------------------------*/

CObserver&
CTSGNotifier::AsObserver( void )
{GUCEF_TRACE;

    return m_tsgObserver;
}

/*-------------------------------------------------------------------------*/

void 
CTSGNotifier::SetPulseGenerator( PulseGeneratorPtr newPulseGenerator )
{GUCEF_TRACE;

    m_tsgObserver.SetPulseGenerator( newPulseGenerator );
}

/*-------------------------------------------------------------------------*/

PulseGeneratorPtr 
CTSGNotifier::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_tsgObserver.GetPulseGenerator();
}

/*-------------------------------------------------------------------------*/

void 
CTSGNotifier::UnsubscribeAllFromObserver( void )
{GUCEF_TRACE;

    m_tsgObserver.UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void 
CTSGNotifier::SignalUpcomingDestruction( void )
{GUCEF_TRACE;

    m_tsgObserver.Shutdown();         
}

/*-------------------------------------------------------------------------*/

void
CTSGNotifier::OnNotify( CNotifier* notifier   ,
                        const CEvent& eventId ,
                        CICloneable* evenData )
{GUCEF_TRACE;

    // we will route to the observer component for actual processing
    m_tsgObserver.OnNotify( notifier ,
                            eventId  ,
                            evenData );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
