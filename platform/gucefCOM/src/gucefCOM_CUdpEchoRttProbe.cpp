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

#ifndef GUCEF_COM_NETWORKBYTEORDER_H
#include "gucefCOM_NetworkByteOrder.h"
#define GUCEF_COM_NETWORKBYTEORDER_H
#endif /* GUCEF_COM_NETWORKBYTEORDER_H ? */

#ifndef GUCEF_COM_CUDPECHORESPONDER_H
#include "gucefCOM_CUdpEchoResponder.h"
#define GUCEF_COM_CUDPECHORESPONDER_H
#endif /* GUCEF_COM_CUDPECHORESPONDER_H ? */

#include "gucefCOM_CUdpEchoRttProbe.h"

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

const CORE::CString CUdpEchoRttProbe::MethodName = "udp-echo";

static const UInt32 TimerIntervalInMs = 5;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUdpEchoRttProbe::CUdpEchoRttProbe( void )
    : COMCORE::CIRttProbe()
    , m_pulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() )
    , m_socket( m_pulseGenerator, false )
    , m_timer( m_pulseGenerator, TimerIntervalInMs )
    , m_target()
    , m_targetAddress()
    , m_count( 0 )
    , m_intervalInMicroSecs( 0 )
    , m_timeoutInMicroSecs( 0 )
    , m_probesSent( 0 )
    , m_nextProbeInMicroSecs( 0 )
    , m_isActive( false )
    , m_outstanding()
{GUCEF_TRACE;

    m_socket.SetNrOfReceiveBuffers( 16, 64 );
    SubscribeTo( &m_socket );
    SubscribeTo( &m_timer );
}

/*-------------------------------------------------------------------------*/

CUdpEchoRttProbe::CUdpEchoRttProbe( const CORE::PulseGeneratorPtr& pulseGenerator )
    : COMCORE::CIRttProbe()
    , m_pulseGenerator( pulseGenerator.IsNULL() ? CORE::CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator )
    , m_socket( m_pulseGenerator, false )
    , m_timer( m_pulseGenerator, TimerIntervalInMs )
    , m_target()
    , m_targetAddress()
    , m_count( 0 )
    , m_intervalInMicroSecs( 0 )
    , m_timeoutInMicroSecs( 0 )
    , m_probesSent( 0 )
    , m_nextProbeInMicroSecs( 0 )
    , m_isActive( false )
    , m_outstanding()
{GUCEF_TRACE;

    m_socket.SetNrOfReceiveBuffers( 16, 64 );
    SubscribeTo( &m_socket );
    SubscribeTo( &m_timer );
}

/*-------------------------------------------------------------------------*/

CUdpEchoRttProbe::~CUdpEchoRttProbe()
{GUCEF_TRACE;

    m_timer.SetEnabled( false );
    UnsubscribeAllFromObserver();
    m_socket.Close();
}

/*-------------------------------------------------------------------------*/

void
CUdpEchoRttProbe::WriteEchoPacket( UInt8* packet, UInt32 sequence, UInt64 sendTimeInMicroSecs )
{GUCEF_TRACE;

    memcpy( packet, CUdpEchoResponder::EchoMagic, 4 );
    packet[ 4 ] = (UInt8)( sequence >> 24 );
    packet[ 5 ] = (UInt8)( sequence >> 16 );
    packet[ 6 ] = (UInt8)( sequence >> 8 );
    packet[ 7 ] = (UInt8)( sequence );
    for ( int i=0; i<8; ++i )
        packet[ 8+i ] = (UInt8)( sendTimeInMicroSecs >> ( 56 - 8*i ) );
}

/*-------------------------------------------------------------------------*/

bool
CUdpEchoRttProbe::ReadEchoPacket( const void* data, UInt32 dataSize, UInt32& sequence, UInt64& sendTimeInMicroSecs )
{GUCEF_TRACE;

    if ( !CUdpEchoResponder::IsEchoPacket( data, dataSize ) )
        return false;
    CNetworkByteReader reader( data, dataSize );
    return reader.Skip( 4 ) && reader.ReadUInt32( sequence ) && reader.ReadUInt64( sendTimeInMicroSecs );
}

/*-------------------------------------------------------------------------*/

bool
CUdpEchoRttProbe::IsAvailable( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CUdpEchoRttProbe::GetMethodName( void ) const
{GUCEF_TRACE;

    return MethodName;
}

/*-------------------------------------------------------------------------*/

bool
CUdpEchoRttProbe::IsActive( void ) const
{GUCEF_TRACE;

    return m_isActive;
}

/*-------------------------------------------------------------------------*/

bool
CUdpEchoRttProbe::Start( const COMCORE::CHostAddress& target ,
                         UInt32 count                        ,
                         UInt32 intervalMs                   ,
                         UInt32 timeoutMs                    )
{GUCEF_TRACE;

    if ( m_isActive || target.GetIPv4Addresses().empty() || 0 == target.GetPortInHostByteOrder() )
        return false;

    if ( !m_socket.IsActive() && !m_socket.Open( (UInt16) 0 ) )
        return false;

    m_target = target;
    m_targetAddress = target.GetFirstIPv4Address();
    m_count = count;
    m_intervalInMicroSecs = intervalMs * (UInt64) 1000;
    m_timeoutInMicroSecs = ( timeoutMs > 0 ? timeoutMs : 1000 ) * (UInt64) 1000;
    m_probesSent = 0;
    m_outstanding.clear();
    m_isActive = true;

    m_timer.SetEnabled( true );
    return SendProbe( GetTimeInMicroSecs() );
}

/*-------------------------------------------------------------------------*/

void
CUdpEchoRttProbe::Stop( void )
{GUCEF_TRACE;

    if ( !m_isActive )
        return;

    m_isActive = false;
    m_timer.SetEnabled( false );
    m_outstanding.clear();
    COMCORE::CRttProbeEventData eData( m_target, 0, m_probesSent > 0 ? m_probesSent-1 : 0 );
    NotifyObservers( ProbeFinishedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CUdpEchoRttProbe::SendProbe( UInt64 nowInMicroSecs )
{GUCEF_TRACE;

    UInt32 sequence = m_probesSent++;
    m_nextProbeInMicroSecs = nowInMicroSecs + m_intervalInMicroSecs;

    UInt8 packet[ 16 ];
    WriteEchoPacket( packet, sequence, nowInMicroSecs );
    if ( m_socket.SendPacketTo( m_targetAddress, packet, (UInt16) sizeof( packet ) ) != (Int32) sizeof( packet ) )
    {
        COMCORE::CRttProbeEventData eData( m_target, 0, sequence );
        if ( !NotifyObservers( ProbeFailedEvent, &eData ) )
            return false;
        return CheckFinished();
    }
    m_outstanding[ sequence ] = nowInMicroSecs;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUdpEchoRttProbe::CheckFinished( void )
{GUCEF_TRACE;

    if ( m_isActive && 0 != m_count && m_probesSent >= m_count && m_outstanding.empty() )
    {
        m_isActive = false;
        m_timer.SetEnabled( false );
        COMCORE::CRttProbeEventData eData( m_target, 0, m_probesSent-1 );
        return NotifyObservers( ProbeFinishedEvent, &eData );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CUdpEchoRttProbe::OnNotify( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_isActive )
        return;

    if ( notifier == &m_socket )
    {
        if ( COMCORE::CUDPSocket::UDPPacketsRecievedEvent != eventId || GUCEF_NULL == eventData )
            return;

        UInt64 now = GetTimeInMicroSecs();
        const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& data = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( eventData )->GetData();
        for ( UInt32 i=0; i<data.packetsReceived; ++i )
        {
            const CORE::CDynamicBuffer& buffer = data.packets[ i ].dataBuffer.GetData();
            UInt32 sequence = 0;
            UInt64 sendTime = 0;
            if ( !ReadEchoPacket( buffer.GetConstBufferPtr(), buffer.GetDataSize(), sequence, sendTime ) )
                continue;

            // Only answers to probes still waiting count: late ones already timed out
            TOutstandingProbeMap::iterator n = m_outstanding.find( sequence );
            if ( n == m_outstanding.end() || (*n).second != sendTime )
                continue;
            m_outstanding.erase( n );

            COMCORE::CRttProbeEventData eData( m_target, now >= sendTime ? now - sendTime : 0, sequence );
            if ( !NotifyObservers( ProbeResultEvent, &eData ) )
                return;
        }
        CheckFinished();
        return;
    }

    if ( notifier == &m_timer && CORE::CTimer::TimerUpdateEvent == eventId )
    {
        UInt64 now = GetTimeInMicroSecs();
        TOutstandingProbeMap::iterator n = m_outstanding.begin();
        while ( n != m_outstanding.end() )
        {
            if ( now - (*n).second >= m_timeoutInMicroSecs )
            {
                UInt32 sequence = (*n).first;
                m_outstanding.erase( n++ );
                COMCORE::CRttProbeEventData eData( m_target, 0, sequence );
                if ( !NotifyObservers( ProbeTimeoutEvent, &eData ) )
                    return;
            }
            else
                ++n;
        }

        if ( m_isActive && ( 0 == m_count || m_probesSent < m_count ) && now >= m_nextProbeInMicroSecs )
        {
            if ( !SendProbe( now ) )
                return;
        }
        CheckFinished();
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CUdpEchoRttProbe::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CUdpEchoRttProbe";
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
