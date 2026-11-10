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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINT_H
#include "gucefCOMCORE_CRpcEndpoint.h"
#define GUCEF_COMCORE_CRPCENDPOINT_H
#endif /* GUCEF_COMCORE_CRPCENDPOINT_H ? */

#include "gucefCOMCORE_CRpcClientStream.h"

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

const CORE::CEvent CRpcClientStream::InternalMessageEvent = "GUCEF::COMCORE::CRpcClientStream::InternalMessageEvent";

/*-------------------------------------------------------------------------*/

CRpcClientStream::CRpcClientStream( const CORE::PulseGeneratorPtr& pulseGenerator, const CRpcEndpointLinkPtr& link, const CRpcMethodRef& ref )
    : CIRpcStream( pulseGenerator )
    , m_lock()
    , m_link( link )
    , m_ref( ref )
    , m_callRef( 0 )
    , m_sessionGeneration( 0 )
    , m_queue()
    , m_readMode( false )
    , m_done( false )
    , m_status()
    , m_response()
    , m_manualCredit( false )
    , m_selfObserver()
{GUCEF_TRACE;

    InternalMessageEvent.Initialize();
    typedef CORE::CTEventHandlerFunctor< CRpcClientStream > TEventCallback;
    TEventCallback callback( this, &CRpcClientStream::OnInternalMessage );
    m_selfObserver.SubscribeTo( this, InternalMessageEvent, callback );
}

/*-------------------------------------------------------------------------*/

CRpcClientStream::~CRpcClientStream()
{GUCEF_TRACE;

    m_selfObserver.UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcClientStream::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CRpcClientStream";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

UInt64
CRpcClientStream::GetCallRef( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_callRef;
}

/*-------------------------------------------------------------------------*/

void
CRpcClientStream::RecordMessage( const CRpcMessage& message )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_queue.push_back( message );
}

/*-------------------------------------------------------------------------*/

void
CRpcClientStream::RaiseStreamEvent( const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObserversFromThread( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

void
CRpcClientStream::OnInternalMessage( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( MessageEvent, eventData );

    // without ReadNext() the event was the consumption; with it, ReadNext() is
    bool consumed = false;
    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_readMode && !m_queue.empty() )
        {
            m_queue.erase( m_queue.begin() );
            consumed = true;
        }
    }
    if ( consumed )
        Consumed( 1 );
}

/*-------------------------------------------------------------------------*/

void
CRpcClientStream::Consumed( UInt32 count )
{GUCEF_TRACE;

    bool manual = false;
    {
        MT::CScopeMutex lock( m_lock );
        manual = m_manualCredit;
    }
    if ( manual )
        return;
    CRpcEndpointLink::CScope scope( m_link );
    if ( GUCEF_NULL != scope.GetEndpoint() )
        scope.GetEndpoint()->StreamConsumed( *this, count, false );
}

/*-------------------------------------------------------------------------*/

bool
CRpcClientStream::RecordEnd( const CRpcStatus& status, const CRpcMessage* response )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_done )
        return false;
    m_done = true;
    m_status = status;
    if ( GUCEF_NULL != response )
        m_response = *response;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcClientStream::Write( const CRpcMessage& message )
{GUCEF_TRACE;

    if ( IsDone() )
        return false;
    CRpcEndpointLink::CScope scope( m_link );
    if ( GUCEF_NULL == scope.GetEndpoint() )
        return false;
    return scope.GetEndpoint()->StreamWrite( *this, message );
}

/*-------------------------------------------------------------------------*/

bool
CRpcClientStream::WritesDone( void )
{GUCEF_TRACE;

    if ( IsDone() )
        return false;
    CRpcEndpointLink::CScope scope( m_link );
    if ( GUCEF_NULL == scope.GetEndpoint() )
        return false;
    return scope.GetEndpoint()->StreamWritesDone( *this );
}

/*-------------------------------------------------------------------------*/

void
CRpcClientStream::Cancel( const CRpcStatus& status )
{GUCEF_TRACE;

    CRpcEndpointLink::CScope scope( m_link );
    if ( GUCEF_NULL != scope.GetEndpoint() )
        scope.GetEndpoint()->StreamCancel( *this, status );
}

/*-------------------------------------------------------------------------*/

bool
CRpcClientStream::ReadNext( CRpcMessage& message, CRpcStatus& status, UInt32 timeoutMs )
{GUCEF_TRACE;

    CORE::PulseGeneratorPtr protocolPulseGenerator;
    {
        CRpcEndpointLink::CScope scope( m_link );
        if ( GUCEF_NULL != scope.GetEndpoint() )
            protocolPulseGenerator = scope.GetEndpoint()->GetProtocolPulseGenerator();
    }
    CORE::PulseGeneratorPtr eventPulseGenerator = GetPulseGenerator();
    UInt32 self = MT::GetCurrentTaskID();
    bool pumpProtocol = !protocolPulseGenerator.IsNULL() && protocolPulseGenerator->GetPulseDriverThreadId() == self;
    bool pumpEvents = !eventPulseGenerator.IsNULL() && eventPulseGenerator->GetPulseDriverThreadId() == self &&
                      eventPulseGenerator != protocolPulseGenerator;

    UInt64 start = MT::PrecisionTickCount();
    UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    if ( 0 == ticksPerMs )
        ticksPerMs = 1;
    while ( true )
    {
        {
            MT::CScopeMutex lock( m_lock );
            m_readMode = true;
            if ( !m_queue.empty() )
            {
                message = m_queue.front();
                m_queue.erase( m_queue.begin() );
                lock.EarlyUnlock();
                Consumed( 1 );
                status = CRpcStatus::Ok;
                return true;
            }
            if ( m_done )
            {
                status = m_status;
                return false;
            }
        }
        if ( ( MT::PrecisionTickCount() - start ) / ticksPerMs >= timeoutMs )
        {
            status.Set( RPC_STATUS_DEADLINE_EXCEEDED, "no stream message in time" );
            return false;
        }
        // the blocking form runs the work on this thread where it is this thread's work (10.9)
        if ( pumpProtocol )
            protocolPulseGenerator->ForceDirectPulse();
        if ( pumpEvents )
            eventPulseGenerator->ForceDirectPulse();
        if ( !pumpProtocol && !pumpEvents )
            MT::PrecisionDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcClientStream::SetManualCredit( bool manual )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_manualCredit = manual;
}

/*-------------------------------------------------------------------------*/

void
CRpcClientStream::GrantCredit( UInt32 messages )
{GUCEF_TRACE;

    CRpcEndpointLink::CScope scope( m_link );
    if ( GUCEF_NULL != scope.GetEndpoint() )
        scope.GetEndpoint()->StreamConsumed( *this, messages, true );
}

/*-------------------------------------------------------------------------*/

bool
CRpcClientStream::IsDone( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_done;
}

/*-------------------------------------------------------------------------*/

CRpcStatus
CRpcClientStream::GetStatus( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_status;
}

/*-------------------------------------------------------------------------*/

CRpcMessage
CRpcClientStream::GetResponse( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_response;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
