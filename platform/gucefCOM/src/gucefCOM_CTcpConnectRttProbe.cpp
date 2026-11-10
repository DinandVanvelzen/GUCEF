/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "gucefCOM_CTcpConnectRttProbe.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <winsock2.h>
  #define GUCEF_COM_CONNECTION_REFUSED     WSAECONNREFUSED
#else
  #include <errno.h>
  #define GUCEF_COM_CONNECTION_REFUSED     ECONNREFUSED
#endif

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

const CORE::CString CTcpConnectRttProbe::MethodName = "tcp-connect";

static const UInt32 TimerIntervalInMs = 5;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTcpConnectRttProbe::CTcpConnectRttProbe( void )
    : COMCORE::CIRttProbe()
    , m_pulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() )
    , m_socket( m_pulseGenerator, false )
    , m_timer( m_pulseGenerator, TimerIntervalInMs )
    , m_target()
    , m_count( 0 )
    , m_intervalInMicroSecs( 0 )
    , m_timeoutInMicroSecs( 0 )
    , m_probesStarted( 0 )
    , m_isActive( false )
    , m_probeInProgress( false )
    , m_probeStartInMicroSecs( 0 )
    , m_nextProbeInMicroSecs( 0 )
{GUCEF_TRACE;

    SubscribeTo( &m_socket );
    SubscribeTo( &m_timer );
}

/*-------------------------------------------------------------------------*/

CTcpConnectRttProbe::CTcpConnectRttProbe( const CORE::PulseGeneratorPtr& pulseGenerator )
    : COMCORE::CIRttProbe()
    , m_pulseGenerator( pulseGenerator.IsNULL() ? CORE::CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator )
    , m_socket( m_pulseGenerator, false )
    , m_timer( m_pulseGenerator, TimerIntervalInMs )
    , m_target()
    , m_count( 0 )
    , m_intervalInMicroSecs( 0 )
    , m_timeoutInMicroSecs( 0 )
    , m_probesStarted( 0 )
    , m_isActive( false )
    , m_probeInProgress( false )
    , m_probeStartInMicroSecs( 0 )
    , m_nextProbeInMicroSecs( 0 )
{GUCEF_TRACE;

    SubscribeTo( &m_socket );
    SubscribeTo( &m_timer );
}

/*-------------------------------------------------------------------------*/

CTcpConnectRttProbe::~CTcpConnectRttProbe()
{GUCEF_TRACE;

    m_timer.SetEnabled( false );
    UnsubscribeAllFromObserver();
    m_socket.Close();
}

/*-------------------------------------------------------------------------*/

bool
CTcpConnectRttProbe::IsAvailable( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTcpConnectRttProbe::GetMethodName( void ) const
{GUCEF_TRACE;

    return MethodName;
}

/*-------------------------------------------------------------------------*/

bool
CTcpConnectRttProbe::IsActive( void ) const
{GUCEF_TRACE;

    return m_isActive;
}

/*-------------------------------------------------------------------------*/

bool
CTcpConnectRttProbe::Start( const COMCORE::CHostAddress& target ,
                            UInt32 count                        ,
                            UInt32 intervalMs                   ,
                            UInt32 timeoutMs                    )
{GUCEF_TRACE;

    if ( m_isActive || target.GetIPv4Addresses().empty() || 0 == target.GetPortInHostByteOrder() )
        return false;

    m_target = target;
    m_count = count;
    m_intervalInMicroSecs = intervalMs * 1000;
    m_timeoutInMicroSecs = ( timeoutMs > 0 ? timeoutMs : 1000 ) * 1000;
    m_probesStarted = 0;
    m_isActive = true;
    m_probeInProgress = false;

    m_timer.SetEnabled( true );
    return StartProbe( GetTimeInMicroSecs() );
}

/*-------------------------------------------------------------------------*/

void
CTcpConnectRttProbe::Stop( void )
{GUCEF_TRACE;

    if ( !m_isActive )
        return;
    if ( m_probeInProgress )
    {
        m_probeInProgress = false;
        m_socket.Close();
    }
    Finish();
}

/*-------------------------------------------------------------------------*/

bool
CTcpConnectRttProbe::Finish( void )
{GUCEF_TRACE;

    m_isActive = false;
    m_timer.SetEnabled( false );
    COMCORE::CRttProbeEventData eData( m_target, 0, m_probesStarted > 0 ? m_probesStarted-1 : 0 );
    return NotifyObservers( ProbeFinishedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CTcpConnectRttProbe::StartProbe( UInt64 nowInMicroSecs )
{GUCEF_TRACE;

    m_probeInProgress = true;
    m_probeStartInMicroSecs = nowInMicroSecs;
    m_nextProbeInMicroSecs = nowInMicroSecs + m_intervalInMicroSecs;
    ++m_probesStarted;

    if ( !m_socket.ConnectTo( m_target.GetFirstIPv4Address(), false ) )
    {
        // A refusal can already show up here, it still is an answer from the host
        if ( m_probeInProgress )
            return EndProbe( ProbeFailedEvent, 0 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTcpConnectRttProbe::EndProbe( const CORE::CEvent& eventId, UInt64 rttInMicroSecs )
{GUCEF_TRACE;

    m_probeInProgress = false;
    m_socket.Close();

    COMCORE::CRttProbeEventData eData( m_target, rttInMicroSecs, m_probesStarted-1 );
    if ( !NotifyObservers( eventId, &eData ) )
        return false;

    if ( m_isActive && 0 != m_count && m_probesStarted >= m_count )
        return Finish();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTcpConnectRttProbe::OnNotify( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( notifier == &m_socket )
    {
        if ( !m_probeInProgress )
            return;

        UInt64 now = GetTimeInMicroSecs();
        if ( COMCORE::CTCPClientSocket::ConnectedEvent == eventId )
        {
            EndProbe( ProbeResultEvent, now - m_probeStartInMicroSecs );
        }
        else
        if ( COMCORE::CTCPClientSocket::SocketErrorEvent == eventId )
        {
            Int32 errorCode = GUCEF_NULL != eventData ? static_cast< COMCORE::CTCPConnection::TSocketErrorEventData* >( eventData )->GetData() : 0;
            if ( GUCEF_COM_CONNECTION_REFUSED == errorCode )
                EndProbe( ProbeResultEvent, now - m_probeStartInMicroSecs );
            else
                EndProbe( ProbeFailedEvent, 0 );
        }
        else
        if ( COMCORE::CTCPClientSocket::DisconnectedEvent == eventId )
        {
            // Closed without a word: the host answered with a reset
            EndProbe( ProbeResultEvent, now - m_probeStartInMicroSecs );
        }
        return;
    }

    if ( notifier == &m_timer && CORE::CTimer::TimerUpdateEvent == eventId && m_isActive )
    {
        UInt64 now = GetTimeInMicroSecs();
        if ( m_probeInProgress )
        {
            if ( now - m_probeStartInMicroSecs >= m_timeoutInMicroSecs )
            {
                if ( !EndProbe( ProbeTimeoutEvent, 0 ) )
                    return;
            }
        }
        if ( m_isActive && !m_probeInProgress && now >= m_nextProbeInMicroSecs )
        {
            if ( 0 == m_count || m_probesStarted < m_count )
                StartProbe( now );
        }
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTcpConnectRttProbe::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CTcpConnectRttProbe";
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
