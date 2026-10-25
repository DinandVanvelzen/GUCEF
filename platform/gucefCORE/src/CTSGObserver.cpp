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

#include <cassert>
#include "CTSGNotifier.h"

#include "CTSGObserver.h"

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

CTSGObserver::CTSGObserver( PulseGeneratorPtr pulseGenerator        ,
                            bool allowSameThreadEventsToFlowThrough )
    : CPumpedObserver( pulseGenerator, allowSameThreadEventsToFlowThrough )
    , m_parentNotifier( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTSGObserver::CTSGObserver( const CTSGObserver& src )
    : CPumpedObserver( src )                   
    , m_parentNotifier( src.m_parentNotifier )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTSGObserver::~CTSGObserver()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CTSGObserver::Shutdown( void )
{GUCEF_TRACE;

    // No lock is held while shutting down, see CPumpedObserver::Shutdown() for why
    CPumpedObserver::Shutdown();
    m_parentNotifier = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CTSGObserver&
CTSGObserver::operator=( const CTSGObserver& src )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

void 
CTSGObserver::SetParent( CTSGNotifier* parentNotifier )
{GUCEF_TRACE;

    m_parentNotifier = parentNotifier;
}

/*-------------------------------------------------------------------------*/

void 
CTSGObserver::OnPumpedNotify( CNotifier* notifier                  ,
                              const CEvent& eventid                ,
                              CICloneable* eventdata /* = NULL  */ )
{GUCEF_TRACE;

    // Make sure we still have a parent
    if ( GUCEF_NULL != m_parentNotifier )
    {
        if ( notifier != m_parentNotifier )
        {
            // Pass on the message as coming from an observed entity
            m_parentNotifier->OnPumpedNotify( notifier  ,
                                              eventid   ,
                                              eventdata );
        }
        else
        {
            // This is a pumped event message originating from ourselves
            // this is used by the NotifyObserversFromThread() functionality to 
            // ensure notifications go out from the pulse generator's associated thread
            m_parentNotifier->NotifyObservers( eventid, eventdata ); 
        }
    }
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTSGObserver::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_parentNotifier )
    {
        return m_parentNotifier->Lock( lockWaitTimeoutInMs );
    }
    return MT::LOCKSTATUS_OPERATION_FAILED;
}

/*-------------------------------------------------------------------------*/
    
MT::TLockStatus
CTSGObserver::Unlock( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_parentNotifier )
    {
        return m_parentNotifier->Unlock();
    }
    return MT::LOCKSTATUS_OPERATION_FAILED;
}

/*-------------------------------------------------------------------------*/

const CString& 
CTSGObserver::GetClassTypeName( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_parentNotifier )
    {
        return m_parentNotifier->GetClassTypeName();
    }
    
    static CString classTypeName = "GUCEF::CORE::CTSGObserver"; 
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
                        
