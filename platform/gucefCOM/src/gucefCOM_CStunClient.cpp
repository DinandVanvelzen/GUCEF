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

#include <stdlib.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COM_NETWORKBYTEORDER_H
#include "gucefCOM_NetworkByteOrder.h"
#define GUCEF_COM_NETWORKBYTEORDER_H
#endif /* GUCEF_COM_NETWORKBYTEORDER_H ? */

#include "gucefCOM_CStunClient.h"

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

const CORE::CEvent CStunClient::ReflexiveAddressEvent = "GUCEF::COM::CStunClient::ReflexiveAddressEvent";
const CORE::CEvent CStunClient::StunErrorEvent = "GUCEF::COM::CStunClient::StunErrorEvent";

const UInt32 CStunClient::MagicCookie = 0x2112A442;

static const UInt16 BindingRequest = 0x0001;
static const UInt16 BindingSuccessResponse = 0x0101;
static const UInt16 BindingErrorResponse = 0x0111;
static const UInt16 AttributeMappedAddress = 0x0001;
static const UInt16 AttributeErrorCode = 0x0009;
static const UInt16 AttributeXorMappedAddress = 0x0020;
static const UInt16 AttributeXorMappedAddressOld = 0x8020;     /* from the drafts, still sent by some servers */
static const UInt8 AddressFamilyIPv4 = 0x01;
static const UInt32 HeaderSize = 20;
static const UInt32 TimerIntervalInMs = 25;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CStunClient::CStunResult::CStunResult( void )
    : CORE::CICloneable()
    , reflexiveAddress()
    , server()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStunClient::CStunResult::CStunResult( const CStunResult& src )
    : CORE::CICloneable( src )
    , reflexiveAddress( src.reflexiveAddress )
    , server( src.server )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStunClient::CStunResult::~CStunResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStunClient::CStunResult&
CStunClient::CStunResult::operator=( const CStunResult& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        reflexiveAddress = src.reflexiveAddress;
        server = src.server;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CStunClient::CStunResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CStunResult( *this );
}

/*-------------------------------------------------------------------------*/

CStunClient::CStunClient( COMCORE::CUDPSocket& socket )
    : CORE::CObservingNotifier()
    , m_socket( &socket )
    , m_timer( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), TimerIntervalInMs )
    , m_queries()
    , m_initialIntervalInMs( 500 )
    , m_maxSends( 4 )
    , m_sendBuffer( HeaderSize, true )
{GUCEF_TRACE;

    ReflexiveAddressEvent.Initialize();
    StunErrorEvent.Initialize();
    SubscribeTo( m_socket );
    SubscribeTo( &m_timer );
}

/*-------------------------------------------------------------------------*/

CStunClient::CStunClient( COMCORE::CUDPSocket& socket, const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CObservingNotifier()
    , m_socket( &socket )
    , m_timer( pulseGenerator.IsNULL() ? CORE::CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator, TimerIntervalInMs )
    , m_queries()
    , m_initialIntervalInMs( 500 )
    , m_maxSends( 4 )
    , m_sendBuffer( HeaderSize, true )
{GUCEF_TRACE;

    ReflexiveAddressEvent.Initialize();
    StunErrorEvent.Initialize();
    SubscribeTo( m_socket );
    SubscribeTo( &m_timer );
}

/*-------------------------------------------------------------------------*/

CStunClient::~CStunClient()
{GUCEF_TRACE;

    m_timer.SetEnabled( false );
    UnsubscribeAllFromObserver();
    for ( size_t i=0; i<m_queries.size(); ++i )
        m_queries[ i ].source.Cancel();
    m_queries.clear();
}

/*-------------------------------------------------------------------------*/

UInt64
CStunClient::GetTimeInMs( void )
{GUCEF_TRACE;

    UInt64 resolution = MT::PrecisionTimerResolution();
    UInt64 ticks = MT::PrecisionTickCount();
    if ( resolution >= 1000 )
        return ticks / ( resolution / 1000 );
    if ( 0 == resolution )
        return ticks;
    return ticks * ( 1000 / resolution );
}

/*-------------------------------------------------------------------------*/

void
CStunClient::SetRetransmission( UInt32 initialIntervalInMs, UInt32 maxSends )
{GUCEF_TRACE;

    m_initialIntervalInMs = initialIntervalInMs > 0 ? initialIntervalInMs : 1;
    m_maxSends = maxSends > 0 ? maxSends : 1;
}

/*-------------------------------------------------------------------------*/

UInt32
CStunClient::GetPendingQueryCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_queries.size();
}

/*-------------------------------------------------------------------------*/

bool
CStunClient::IsStunMessage( const void* data, UInt32 dataSize )
{GUCEF_TRACE;

    if ( GUCEF_NULL == data || dataSize < HeaderSize )
        return false;
    const UInt8* bytes = static_cast< const UInt8* >( data );
    if ( 0 != ( bytes[ 0 ] & 0xC0 ) )
        return false;
    UInt32 cookie = ( (UInt32) bytes[ 4 ] << 24 ) | ( (UInt32) bytes[ 5 ] << 16 ) | ( (UInt32) bytes[ 6 ] << 8 ) | (UInt32) bytes[ 7 ];
    return MagicCookie == cookie;
}

/*-------------------------------------------------------------------------*/

void
CStunClient::EncodeBindingRequest( const UInt8* transactionId, CORE::CDynamicBuffer& message )
{GUCEF_TRACE;

    message.SetDataSize( 0 );
    CNetworkByteWriter writer( message );
    writer.WriteUInt16( BindingRequest );
    writer.WriteUInt16( 0 );                // no attributes
    writer.WriteUInt32( MagicCookie );
    writer.WriteBytes( transactionId, 12 );
}

/*-------------------------------------------------------------------------*/

bool
CStunClient::DecodeBindingResponse( const void* data                     ,
                                    UInt32 dataSize                      ,
                                    UInt8* transactionId                 ,
                                    bool& isSuccess                      ,
                                    COMCORE::CIPv4Address& mappedAddress ,
                                    UInt16& errorCode                    )
{GUCEF_TRACE;

    if ( !IsStunMessage( data, dataSize ) )
        return false;

    CNetworkByteReader reader( data, dataSize );
    UInt16 messageType = 0;
    UInt16 messageLength = 0;
    UInt32 cookie = 0;
    if ( !reader.ReadUInt16( messageType ) || !reader.ReadUInt16( messageLength ) || !reader.ReadUInt32( cookie ) || !reader.ReadBytes( transactionId, 12 ) )
        return false;
    if ( BindingSuccessResponse != messageType && BindingErrorResponse != messageType )
        return false;
    if ( messageLength > reader.GetRemaining() )
        return false;

    isSuccess = BindingSuccessResponse == messageType;
    errorCode = 0;
    bool haveXorAddress = false;
    bool haveAddress = false;
    UInt32 end = HeaderSize + messageLength;

    while ( reader.GetOffset() + 4 <= end )
    {
        UInt16 attributeType = 0;
        UInt16 attributeLength = 0;
        if ( !reader.ReadUInt16( attributeType ) || !reader.ReadUInt16( attributeLength ) )
            return false;
        UInt32 valueStart = reader.GetOffset();
        UInt32 paddedLength = ( attributeLength + 3 ) & ~3u;
        if ( valueStart + attributeLength > end )
            return false;

        const UInt8* value = reader.GetData() + valueStart;
        bool isXor = AttributeXorMappedAddress == attributeType || AttributeXorMappedAddressOld == attributeType;
        if ( ( isXor || AttributeMappedAddress == attributeType ) && attributeLength >= 8 && AddressFamilyIPv4 == value[ 1 ] )
        {
            UInt16 port = (UInt16)( ( value[ 2 ] << 8 ) | value[ 3 ] );
            UInt32 address = ( (UInt32) value[ 4 ] << 24 ) | ( (UInt32) value[ 5 ] << 16 ) | ( (UInt32) value[ 6 ] << 8 ) | (UInt32) value[ 7 ];
            if ( isXor )
            {
                port = (UInt16)( port ^ ( MagicCookie >> 16 ) );
                address ^= MagicCookie;
            }
            // The XOR form is preferred: middle boxes that rewrite addresses in payloads cannot spoil it
            if ( isXor || !haveXorAddress )
            {
                mappedAddress.SetAddressInHostByteOrder( address );
                mappedAddress.SetPortInHostByteOrder( port );
                haveAddress = true;
                haveXorAddress = haveXorAddress || isXor;
            }
        }
        else
        if ( AttributeErrorCode == attributeType && attributeLength >= 4 )
        {
            errorCode = (UInt16)( ( value[ 2 ] & 0x07 ) * 100 + value[ 3 ] );
        }

        if ( !reader.Seek( valueStart + paddedLength > end ? end : valueStart + paddedLength ) )
            return false;
    }

    return !isSuccess || haveAddress;
}

/*-------------------------------------------------------------------------*/

bool
CStunClient::TryGetResultFromFuture( const CORE::CFutureResult& future, CStunResult& result )
{GUCEF_TRACE;

    if ( future.HasNoFuture() || !future.IsReady() )
        return false;

    CORE::CTaskPtr task = future.GetResult();
    if ( task.IsNULL() || CORE::TTaskStatus::TASKSTATUS_FINISHED != task->GetTaskStatus() )
        return false;

    const CStunResult* data = dynamic_cast< const CStunResult* >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
        return false;
    result = *data;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStunClient::Send( TQuery& query )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_socket )
        return false;

    ++query.sends;
    query.nextActionInMs = GetTimeInMs() + query.intervalInMs;
    query.intervalInMs *= 2;

    EncodeBindingRequest( query.transactionId, m_sendBuffer );
    return m_socket->SendPacketTo( query.server, m_sendBuffer.GetConstBufferPtr(), (UInt16) m_sendBuffer.GetDataSize() ) == (Int32) m_sendBuffer.GetDataSize();
}

/*-------------------------------------------------------------------------*/

bool
CStunClient::QueryReflexiveAddress( const COMCORE::CHostAddress& server ,
                                    CORE::CFutureResult& future         )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_socket || !m_socket->IsActive() || server.GetIPv4Addresses().empty() )
        return false;

    TQuery query;
    query.server = server.GetFirstIPv4Address();
    if ( 0 == query.server.GetPort() )
        query.server.SetPortInHostByteOrder( 3478 );
    query.sends = 0;
    query.intervalInMs = m_initialIntervalInMs;

    UInt64 ticks = MT::PrecisionTickCount();
    for ( int i=0; i<8; ++i )
        query.transactionId[ i ] = (UInt8)( ( ticks >> ( 8*i ) ) ^ ( rand() & 0xFF ) );
    for ( int i=8; i<12; ++i )
        query.transactionId[ i ] = (UInt8)( rand() & 0xFF );

    if ( !Send( query ) )
        return false;

    future = query.source.GetFuture();
    m_queries.push_back( query );
    m_timer.SetEnabled( true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStunClient::OnTimer( void )
{GUCEF_TRACE;

    UInt64 now = GetTimeInMs();
    size_t i = 0;
    while ( i < m_queries.size() )
    {
        TQuery& query = m_queries[ i ];
        if ( query.nextActionInMs > now )
        {
            ++i;
            continue;
        }
        if ( query.sends < m_maxSends )
        {
            Send( query );
            ++i;
            continue;
        }

        TQuery failed = query;
        m_queries.erase( m_queries.begin() + i );
        CORE::CString reason = "The STUN server " + failed.server.AddressAndPortAsString() + " did not answer";
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StunClient: " << reason );
        failed.source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, reason );
        TStunErrorEventData eData( reason );
        if ( !NotifyObservers( StunErrorEvent, &eData ) )
            return false;
    }
    if ( m_queries.empty() )
        m_timer.SetEnabled( false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStunClient::OnPacket( const COMCORE::CIPv4Address& source, const CORE::CDynamicBuffer& packet )
{GUCEF_TRACE;

    if ( m_queries.empty() || !IsStunMessage( packet.GetConstBufferPtr(), packet.GetDataSize() ) )
        return true;

    UInt8 transactionId[ 12 ];
    bool isSuccess = false;
    COMCORE::CIPv4Address mapped;
    UInt16 errorCode = 0;
    if ( !DecodeBindingResponse( packet.GetConstBufferPtr(), packet.GetDataSize(), transactionId, isSuccess, mapped, errorCode ) )
        return true;

    for ( size_t i=0; i<m_queries.size(); ++i )
    {
        if ( 0 != memcmp( m_queries[ i ].transactionId, transactionId, 12 ) || source.GetAddress() != m_queries[ i ].server.GetAddress() )
            continue;

        TQuery query = m_queries[ i ];
        m_queries.erase( m_queries.begin() + i );
        if ( m_queries.empty() )
            m_timer.SetEnabled( false );

        if ( !isSuccess )
        {
            CORE::CString reason = "The STUN server " + query.server.AddressAndPortAsString() + " answered with error " + CORE::UInt16ToString( errorCode );
            query.source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, reason );
            TStunErrorEventData eData( reason );
            return NotifyObservers( StunErrorEvent, &eData );
        }

        CStunResult result;
        result.reflexiveAddress = mapped;
        result.server = query.server;
        query.source.Complete( result.Clone(), true );
        return NotifyObservers( ReflexiveAddressEvent, &result );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CStunClient::OnNotify( CORE::CNotifier* notifier    ,
                       const CORE::CEvent& eventId  ,
                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( notifier == m_socket )
    {
        if ( COMCORE::CUDPSocket::UDPPacketsRecievedEvent == eventId && GUCEF_NULL != eventData )
        {
            const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& data = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( eventData )->GetData();
            for ( UInt32 i=0; i<data.packetsReceived; ++i )
            {
                if ( !OnPacket( data.packets[ i ].sourceAddress, data.packets[ i ].dataBuffer.GetData() ) )
                    return;
            }
        }
        else
        if ( CORE::CNotifier::DestructionEvent == eventId )
        {
            m_socket = GUCEF_NULL;
        }
        return;
    }

    if ( notifier == &m_timer && CORE::CTimer::TimerUpdateEvent == eventId )
        OnTimer();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CStunClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CStunClient";
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
