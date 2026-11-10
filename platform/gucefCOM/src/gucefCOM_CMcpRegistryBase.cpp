/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H ? */

#include "gucefCOM_CMcpRegistryBase.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CMcpRegistryBase::ListChangedEvent = "GUCEF::COM::CMcpRegistryBase::ListChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CMcpRegistryBase::RegisterEvents( void )
{GUCEF_TRACE;

    ListChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CMcpRegistryBase::CMcpRegistryBase( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CObservingNotifier()
    , m_rwLock( true )
    , m_pulseGenerator( pulseGenerator.IsNULL() ? CORE::CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator )
    , m_queueLock()
    , m_queuedEvents()
    , m_pulseThreadId( 0 )
    , m_generation( 0 )
    , m_changePending( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    m_pulseThreadId.Store( m_pulseGenerator->GetPulseDriverThreadId() );

    typedef CORE::CTEventHandlerFunctor< CMcpRegistryBase > TEventCallback;
    TEventCallback callback( this, &CMcpRegistryBase::OnPulse );
    SubscribeTo( m_pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, callback );
}

/*-------------------------------------------------------------------------*/

CMcpRegistryBase::~CMcpRegistryBase()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

UInt64
CMcpRegistryBase::GetGeneration( void ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwLock );
    return m_generation;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRegistryBase::IsListChangePending( void ) const
{GUCEF_TRACE;

    return 0 != m_changePending.Load();
}

/*-------------------------------------------------------------------------*/

UInt64
CMcpRegistryBase::OnChangedWhileLocked( void )
{GUCEF_TRACE;

    ++m_generation;
    m_changePending.Store( 1 );
    return m_generation;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRegistryBase::IsPulseThread( void ) const
{GUCEF_TRACE;

    // kept by the registry itself: the pulse generator does not offer its driver thread id to other threads safely
    UInt32 pulseThreadId = m_pulseThreadId.Load();
    return 0 != pulseThreadId && MT::GetCurrentTaskID() == pulseThreadId;
}

/*-------------------------------------------------------------------------*/

void
CMcpRegistryBase::RaiseNameEvent( const CORE::CEvent& eventId, const CORE::CString& name )
{GUCEF_TRACE;

    if ( IsPulseThread() )
    {
        RaiseQueuedEvents();
        TNameEventData eData( name );
        NotifyObservers( eventId, &eData );
        return;
    }
    SQueuedEvent queued;
    queued.eventId = eventId;
    queued.name = name;
    MT::CScopeMutex lock( m_queueLock );
    m_queuedEvents.push_back( queued );
}

/*-------------------------------------------------------------------------*/

void
CMcpRegistryBase::RaiseQueuedEvents( void )
{GUCEF_TRACE;

    TQueuedEventVector queued;
    {
        MT::CScopeMutex lock( m_queueLock );
        if ( m_queuedEvents.empty() )
            return;
        queued.swap( m_queuedEvents );
    }
    for ( size_t i=0; i<queued.size(); ++i )
    {
        TNameEventData eData( queued[ i ].name );
        NotifyObservers( queued[ i ].eventId, &eData );
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpRegistryBase::FlushListChanged( void )
{GUCEF_TRACE;

    RaiseQueuedEvents();
    if ( 0 != m_changePending.Exchange( 0 ) )
        NotifyObservers( ListChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CMcpRegistryBase::OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_pulseThreadId.Store( MT::GetCurrentTaskID() );
    FlushListChanged();
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CMcpRegistryBase::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpRegistryBase::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CMcpRegistryBase";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
