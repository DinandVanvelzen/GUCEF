/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#include "gucefCOMCORE_CRpcReleaseCourier.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CRpcReleaseCourier::ReleaseEvent = "GUCEF::COMCORE::CRpcReleaseCourier::ReleaseEvent";

/*-------------------------------------------------------------------------*/

void
CRpcReleaseCourier::RegisterEvents( void )
{GUCEF_TRACE;

    ReleaseEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CRpcReleaseCourier::CRpcReleaseCourier( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CTSGNotifier( pulseGenerator, true, false )
    , m_lock()
    , m_streams()
    , m_calls()
    , m_rounds( 0 )
    , m_mailed( false )
    , m_selfObserver()
{GUCEF_TRACE;

    RegisterEvents();
    typedef CORE::CTEventHandlerFunctor< CRpcReleaseCourier > TEventCallback;
    TEventCallback releaseCallback( this, &CRpcReleaseCourier::OnRelease );
    m_selfObserver.SubscribeTo( this, ReleaseEvent, releaseCallback );
}

/*-------------------------------------------------------------------------*/

CRpcReleaseCourier::~CRpcReleaseCourier()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    m_selfObserver.UnsubscribeAllFromObserver();

    // at the teardown of the module whatever is left goes here
    GUCEF::vector< CIRpcStreamPtr > streams;
    GUCEF::vector< CRpcServerCallPtr > calls;
    {
        MT::CScopeMutex lock( m_lock );
        streams.swap( m_streams );
        calls.swap( m_calls );
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcReleaseCourier::Hold( GUCEF::vector< CIRpcStreamPtr >& streams, GUCEF::vector< CRpcServerCallPtr >& calls )
{GUCEF_TRACE;

    bool mail = false;
    {
        MT::CScopeMutex lock( m_lock );
        for ( UInt32 i=0; i<streams.size(); ++i )
            if ( !streams[ i ].IsNULL() )
                m_streams.push_back( streams[ i ] );
        for ( UInt32 i=0; i<calls.size(); ++i )
            if ( !calls[ i ].IsNULL() )
                m_calls.push_back( calls[ i ] );
        streams.clear();
        calls.clear();
        if ( !m_mailed && ( !m_streams.empty() || !m_calls.empty() ) )
        {
            m_mailed = true;
            mail = true;
        }
    }
    // mailed, also on the courier's own thread: the references never go inside the caller's frames
    if ( mail )
        NotifyObserversFromThread( ReleaseEvent );
}

/*-------------------------------------------------------------------------*/

bool
CRpcReleaseCourier::IsIdle( void ) const
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( m_mailed || !m_streams.empty() || !m_calls.empty() )
            return false;
    }
    return !HasMail();
}

/*-------------------------------------------------------------------------*/

void
CRpcReleaseCourier::OnRelease( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // On the pulse generator's thread, in the delivery of the courier's own mail. A reference whose object has no mail
    // of its own goes now (outside the courier's lock); one whose object still has mail waits for the next pulse: its
    // handlers may still let go of their references, and the last one would then be theirs inside that delivery
    GUCEF::vector< CIRpcStreamPtr > streams;
    GUCEF::vector< CRpcServerCallPtr > calls;
    bool giveUp = false;
    {
        MT::CScopeMutex lock( m_lock );
        streams.swap( m_streams );
        calls.swap( m_calls );
        m_mailed = false;
        ++m_rounds;
        giveUp = m_rounds > MaxRounds;
    }
    GUCEF::vector< CIRpcStreamPtr > laterStreams;
    GUCEF::vector< CRpcServerCallPtr > laterCalls;
    for ( UInt32 i=0; i<streams.size(); ++i )
    {
        if ( !giveUp && streams[ i ]->HasMail() )
            laterStreams.push_back( streams[ i ] );
    }
    for ( UInt32 i=0; i<calls.size(); ++i )
    {
        if ( !giveUp && calls[ i ]->HasMail() )
            laterCalls.push_back( calls[ i ] );
    }
    // the references of this pulse that may go, go here
    streams.clear();
    calls.clear();
    if ( laterStreams.empty() && laterCalls.empty() )
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_streams.empty() && m_calls.empty() )
            m_rounds = 0;
        return;
    }
    Hold( laterStreams, laterCalls );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcReleaseCourier::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CRpcReleaseCourier";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
