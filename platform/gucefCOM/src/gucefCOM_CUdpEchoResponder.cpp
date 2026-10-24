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

#include "gucefCOM_CUdpEchoResponder.h"

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

const UInt8 CUdpEchoResponder::EchoMagic[ 4 ] = { 'G', 'D', 'E', 'C' };
const UInt32 CUdpEchoResponder::EchoPacketSize = 16;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUdpEchoResponder::CUdpEchoResponder( void )
    : CORE::CObservingNotifier()
    , m_ownSocket( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), false )
    , m_socket( GUCEF_NULL )
    , m_echoCount( 0 )
{GUCEF_TRACE;

    m_ownSocket.SetNrOfReceiveBuffers( 16, 64 );
}

/*-------------------------------------------------------------------------*/

CUdpEchoResponder::CUdpEchoResponder( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CObservingNotifier()
    , m_ownSocket( pulseGenerator.IsNULL() ? CORE::CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator, false )
    , m_socket( GUCEF_NULL )
    , m_echoCount( 0 )
{GUCEF_TRACE;

    m_ownSocket.SetNrOfReceiveBuffers( 16, 64 );
}

/*-------------------------------------------------------------------------*/

CUdpEchoResponder::~CUdpEchoResponder()
{GUCEF_TRACE;

    Stop();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CUdpEchoResponder::IsEchoPacket( const void* data, UInt32 dataSize )
{GUCEF_TRACE;

    return EchoPacketSize == dataSize && GUCEF_NULL != data && 0 == memcmp( data, EchoMagic, 4 );
}

/*-------------------------------------------------------------------------*/

bool
CUdpEchoResponder::Start( UInt16 port )
{GUCEF_TRACE;

    Stop();
    if ( !m_ownSocket.Open( port ) )
        return false;
    m_socket = &m_ownSocket;
    SubscribeTo( m_socket );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUdpEchoResponder::Start( const COMCORE::CIPv4Address& localAddress )
{GUCEF_TRACE;

    Stop();
    if ( !m_ownSocket.Open( localAddress ) )
        return false;
    m_socket = &m_ownSocket;
    SubscribeTo( m_socket );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUdpEchoResponder::Attach( COMCORE::CUDPSocket& socket )
{GUCEF_TRACE;

    Stop();
    m_socket = &socket;
    SubscribeTo( m_socket );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CUdpEchoResponder::Stop( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_socket )
    {
        UnsubscribeFrom( m_socket );
        if ( m_socket == &m_ownSocket )
            m_ownSocket.Close();
        m_socket = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool
CUdpEchoResponder::IsActive( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_socket && m_socket->IsActive();
}

/*-------------------------------------------------------------------------*/

UInt16
CUdpEchoResponder::GetPort( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_socket ? m_socket->GetPort() : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CUdpEchoResponder::GetEchoCount( void ) const
{GUCEF_TRACE;

    return m_echoCount;
}

/*-------------------------------------------------------------------------*/

void
CUdpEchoResponder::OnNotify( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_socket || notifier != m_socket )
        return;

    if ( COMCORE::CUDPSocket::UDPPacketsRecievedEvent == eventId && GUCEF_NULL != eventData )
    {
        const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& data = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( eventData )->GetData();
        for ( UInt32 i=0; i<data.packetsReceived; ++i )
        {
            const COMCORE::CUDPSocket::TPacketEntry& packet = data.packets[ i ];
            const CORE::CDynamicBuffer& buffer = packet.dataBuffer.GetData();
            if ( IsEchoPacket( buffer.GetConstBufferPtr(), buffer.GetDataSize() ) )
            {
                if ( m_socket->SendPacketTo( packet.sourceAddress, buffer.GetConstBufferPtr(), (UInt16) buffer.GetDataSize() ) == (Int32) EchoPacketSize )
                    ++m_echoCount;
            }
        }
    }
    else
    if ( CORE::CNotifier::DestructionEvent == eventId )
    {
        // The socket we rode along on is going away, do not touch it anymore
        m_socket = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CUdpEchoResponder::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CUdpEchoResponder";
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
