/*
 *  gucefDRN_TestApp: GUCEF test application for the DRN module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#include "TestDRNLossyChannel.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const CORE::CString LossyPrefix = "lossy-";

/** The first protected packet of a channel with recordAuth */
static CORE::CDynamicBuffer g_recordedAuthPacket;

static Float64
RandomUnit( void )
{
    return CORE::CRandom::GetRandomFloat64();
}

/*-------------------------------------------------------------------------*/

/** The factories need default constructible products, one class per inner type */
class CDRNLossyTcpChannel : public CDRNLossyChannel { public: CDRNLossyTcpChannel( void ) : CDRNLossyChannel( "tcp" ) {} };
class CDRNLossyUdpChannel : public CDRNLossyChannel { public: CDRNLossyUdpChannel( void ) : CDRNLossyChannel( "udp" ) {} };
class CDRNLossyWebSocketChannel : public CDRNLossyChannel { public: CDRNLossyWebSocketChannel( void ) : CDRNLossyChannel( "websocket" ) {} };
class CDRNLossyTcpChannelListener : public CDRNLossyChannelListener { public: CDRNLossyTcpChannelListener( void ) : CDRNLossyChannelListener( "tcp" ) {} };
class CDRNLossyUdpChannelListener : public CDRNLossyChannelListener { public: CDRNLossyUdpChannelListener( void ) : CDRNLossyChannelListener( "udp" ) {} };
class CDRNLossyWebSocketChannelListener : public CDRNLossyChannelListener { public: CDRNLossyWebSocketChannelListener( void ) : CDRNLossyChannelListener( "websocket" ) {} };

static CORE::CTFactory< DRN::CIDRNChannel, CDRNLossyTcpChannel, MT::CMutex > g_lossyTcpChannelFactory;
static CORE::CTFactory< DRN::CIDRNChannel, CDRNLossyUdpChannel, MT::CMutex > g_lossyUdpChannelFactory;
static CORE::CTFactory< DRN::CIDRNChannel, CDRNLossyWebSocketChannel, MT::CMutex > g_lossyWebSocketChannelFactory;
static CORE::CTFactory< DRN::CIDRNChannelListener, CDRNLossyTcpChannelListener, MT::CMutex > g_lossyTcpChannelListenerFactory;
static CORE::CTFactory< DRN::CIDRNChannelListener, CDRNLossyUdpChannelListener, MT::CMutex > g_lossyUdpChannelListenerFactory;
static CORE::CTFactory< DRN::CIDRNChannelListener, CDRNLossyWebSocketChannelListener, MT::CMutex > g_lossyWebSocketChannelListenerFactory;

/*-------------------------------------------------------------------------*/

void
RegisterLossyChannelFactories( void )
{
    DRN::CDRNGlobal* global = DRN::CDRNGlobal::Instance();
    global->GetChannelFactory().RegisterConcreteFactory( LossyPrefix + "tcp", &g_lossyTcpChannelFactory );
    global->GetChannelFactory().RegisterConcreteFactory( LossyPrefix + "udp", &g_lossyUdpChannelFactory );
    global->GetChannelFactory().RegisterConcreteFactory( LossyPrefix + "websocket", &g_lossyWebSocketChannelFactory );
    global->GetChannelListenerFactory().RegisterConcreteFactory( LossyPrefix + "tcp", &g_lossyTcpChannelListenerFactory );
    global->GetChannelListenerFactory().RegisterConcreteFactory( LossyPrefix + "udp", &g_lossyUdpChannelListenerFactory );
    global->GetChannelListenerFactory().RegisterConcreteFactory( LossyPrefix + "websocket", &g_lossyWebSocketChannelListenerFactory );
}

/*-------------------------------------------------------------------------*/

void
UnregisterLossyChannelFactories( void )
{
    DRN::CDRNGlobal* global = DRN::CDRNGlobal::Instance();
    global->GetChannelFactory().UnregisterConcreteFactory( LossyPrefix + "tcp" );
    global->GetChannelFactory().UnregisterConcreteFactory( LossyPrefix + "udp" );
    global->GetChannelFactory().UnregisterConcreteFactory( LossyPrefix + "websocket" );
    global->GetChannelListenerFactory().UnregisterConcreteFactory( LossyPrefix + "tcp" );
    global->GetChannelListenerFactory().UnregisterConcreteFactory( LossyPrefix + "udp" );
    global->GetChannelListenerFactory().UnregisterConcreteFactory( LossyPrefix + "websocket" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNLossySettings::CDRNLossySettings( void )
    : lossRate( 0 )
    , duplicateRate( 0 )
    , reorderRate( 0 )
    , delayMs( 0 )
    , jitterMs( 0 )
    , clockRate( 1.0 )
    , holdAfterMs( 0 )
    , holdForMs( 0 )
    , tamperRate( 0 )
    , downgrade( false )
    , recordAuth( false )
    , injectAuth( false )
{

}

/*-------------------------------------------------------------------------*/

void
CDRNLossySettings::FromParams( const CORE::CValueList& params )
{
    lossRate = params.GetValueAlways( "loss", CORE::CVariant( 0.0 ) ).AsFloat64( 0.0 );
    duplicateRate = params.GetValueAlways( "dup", CORE::CVariant( 0.0 ) ).AsFloat64( 0.0 );
    reorderRate = params.GetValueAlways( "reorder", CORE::CVariant( 0.0 ) ).AsFloat64( 0.0 );
    delayMs = params.GetValueAlways( "delay", CORE::CVariant( (UInt32) 0 ) ).AsUInt32( 0 );
    jitterMs = params.GetValueAlways( "jitter", CORE::CVariant( (UInt32) 0 ) ).AsUInt32( 0 );
    clockRate = params.GetValueAlways( "warp", CORE::CVariant( 1.0 ) ).AsFloat64( 1.0 );
    holdAfterMs = params.GetValueAlways( "holdAfter", CORE::CVariant( (UInt32) 0 ) ).AsUInt32( 0 );
    holdForMs = params.GetValueAlways( "holdFor", CORE::CVariant( (UInt32) 0 ) ).AsUInt32( 0 );
    tamperRate = params.GetValueAlways( "tamper", CORE::CVariant( 0.0 ) ).AsFloat64( 0.0 );
    downgrade = 0 != params.GetValueAlways( "downgrade", CORE::CVariant( (UInt32) 0 ) ).AsUInt32( 0 );
    recordAuth = 0 != params.GetValueAlways( "recordAuth", CORE::CVariant( (UInt32) 0 ) ).AsUInt32( 0 );
    injectAuth = 0 != params.GetValueAlways( "injectAuth", CORE::CVariant( (UInt32) 0 ) ).AsUInt32( 0 );
}

/*-------------------------------------------------------------------------*/

bool
CDRNLossySettings::MakesUnreliable( void ) const
{
    return lossRate > 0 || duplicateRate > 0 || reorderRate > 0 || jitterMs > 0;
}

/*-------------------------------------------------------------------------*/

CDRNLossyChannel::CDRNLossyChannel( const CORE::CString& innerType )
    : DRN::CIDRNChannel()
    , m_innerType( innerType )
    , m_inner()
    , m_settings()
    , m_capabilities()
    , m_pulseGenerator()
    , m_delayed()
    , m_dropped( 0 )
    , m_duplicated( 0 )
    , m_holdStart( 0 )
    , m_holdEnd( 0 )
    , m_warpBuffer()
    , m_tamperBuffer()
    , m_tampered( 0 )
    , m_maxSentSize( 0 )
    , m_sentProtected( false )
{
    // what the inner channel will offer, so that a link started before Connect() gets the right budget
    if ( m_innerType == DRN::CDRNEndpoint::ChannelTypeUdp )
    {
        m_capabilities.isReliable = false;
        m_capabilities.isDatagram = true;
        m_capabilities.maxPacketSize = DRN::DRN_DEFAULT_DATAGRAM_SIZE;
    }
}

/*-------------------------------------------------------------------------*/

CDRNLossyChannel::CDRNLossyChannel( const DRN::CIDRNChannelPtr& inner, const CDRNLossySettings& settings, const CORE::CString& innerType )
    : DRN::CIDRNChannel()
    , m_innerType( innerType )
    , m_inner( inner )
    , m_settings( settings )
    , m_capabilities()
    , m_pulseGenerator( inner->GetPulseGenerator() )
    , m_delayed()
    , m_dropped( 0 )
    , m_duplicated( 0 )
    , m_holdStart( 0 )
    , m_holdEnd( 0 )
    , m_warpBuffer()
    , m_tamperBuffer()
    , m_tampered( 0 )
    , m_maxSentSize( 0 )
    , m_sentProtected( false )
{
    SubscribeTo( m_inner.GetPointerAlways() );
    if ( !m_pulseGenerator.IsNULL() )
        SubscribeTo( m_pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent );
    UpdateCapabilities();
}

/*-------------------------------------------------------------------------*/

CDRNLossyChannel::~CDRNLossyChannel()
{
    SignalUpcomingObserverDestruction();
    UnsubscribeAllFromObserver();
    if ( !m_inner.IsNULL() )
        m_inner->Close();
}

/*-------------------------------------------------------------------------*/

void
CDRNLossyChannel::UpdateCapabilities( void )
{
    if ( !m_inner.IsNULL() )
        m_capabilities = m_inner->GetCapabilities();
    if ( m_settings.MakesUnreliable() )
    {
        m_capabilities.isReliable = false;
        m_capabilities.isDatagram = true;
        if ( 0 == m_capabilities.maxPacketSize )
            m_capabilities.maxPacketSize = DRN::DRN_DEFAULT_DATAGRAM_SIZE;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNLossyChannel::SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator )
{
    m_pulseGenerator = pulseGenerator;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CDRNLossyChannel::GetPulseGenerator( void ) const
{
    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

bool
CDRNLossyChannel::Connect( const DRN::CDRNEndpoint& remote )
{
    m_settings.FromParams( remote.GetParams() );
    m_inner = DRN::CIDRNChannelPtr( DRN::CDRNGlobal::Instance()->GetChannelFactory().Create( m_innerType ) );
    if ( m_inner.IsNULL() )
        return false;
    if ( m_pulseGenerator.IsNULL() )
        m_pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    m_inner->SetPulseGenerator( m_pulseGenerator );
    SubscribeTo( m_inner.GetPointerAlways() );
    SubscribeTo( m_pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent );
    UpdateCapabilities();

    DRN::CDRNEndpoint innerEndpoint( m_innerType, remote.GetAddress() );
    bool result = m_inner->Connect( innerEndpoint );
    UpdateCapabilities();
    return result;
}

/*-------------------------------------------------------------------------*/

void
CDRNLossyChannel::Close( void )
{
    m_delayed.clear();
    if ( !m_inner.IsNULL() )
    {
        UnsubscribeFrom( m_inner.GetPointerAlways() );
        m_inner->Close();
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNLossyChannel::IsConnected( void ) const
{
    return !m_inner.IsNULL() && m_inner->IsConnected();
}

/*-------------------------------------------------------------------------*/

const void*
CDRNLossyChannel::Warp( const void* data, UInt32& size )
{
    // the sendTime of the header as a clock running at clockRate would give it
    DRN::CDRNWireReader reader( data, size );
    DRN::CDRNPacketHeader header;
    if ( !DRN::CDRNPacketCodec::ReadHeader( reader, header ) )
        return data;
    header.sendTime = (UInt64) ( (Float64) header.sendTime * m_settings.clockRate );
    m_warpBuffer.SetDataSize( 0 );
    DRN::CDRNWireWriter writer( m_warpBuffer );
    DRN::CDRNPacketCodec::WriteHeader( writer, header );
    writer.WriteBytes( reader.GetCurrentPtr(), reader.GetRemaining() );
    size = writer.GetOffset();
    return m_warpBuffer.GetConstBufferPtr();
}

/*-------------------------------------------------------------------------*/

const void*
CDRNLossyChannel::Downgrade( const void* data, UInt32& size )
{
    // a HELLO in the packet is written again with only the last cipher it offers
    DRN::CDRNWireReader reader( data, size );
    DRN::CDRNPacketHeader header;
    if ( !DRN::CDRNPacketCodec::ReadHeader( reader, header ) )
        return data;
    m_warpBuffer.SetDataSize( 0 );
    DRN::CDRNWireWriter writer( m_warpBuffer );
    DRN::CDRNPacketCodec::WriteHeader( writer, header );
    bool changed = false;
    while ( reader.GetRemaining() > 0 )
    {
        DRN::CDRNMessageView message;
        if ( !DRN::CDRNPacketCodec::ReadMessage( reader, message ) )
            return data;
        if ( DRN::DRN_MSGCLASS_LINK != message.msgClass || DRN::DRN_LINK_HELLO != message.opcode )
        {
            DRN::CDRNPacketCodec::WriteMessage( writer, message.msgClass, message.opcode, message.lane, message.laneSequence, message.payload, message.payloadSize );
            continue;
        }
        DRN::CDRNWireReader hello( message.payload, message.payloadSize );
        UInt8 major = 0, minor = 0;
        UInt32 features = 0, count = 0;
        UInt64 epoch = 0;
        CORE::CString nodeId, service;
        CORE::CDynamicBuffer policyHash, ephemeral;
        if ( !hello.ReadUInt8( major ) || !hello.ReadUInt8( minor ) || !hello.ReadVarUInt32( features ) || !hello.ReadString( nodeId ) ||
             !hello.ReadString( service ) || !hello.ReadVarUInt64( epoch ) || !hello.ReadSizedBytes( policyHash ) || !hello.ReadSizedBytes( ephemeral ) ||
             !hello.ReadVarUInt32( count ) )
            return data;
        CORE::CString cipher;
        for ( UInt32 i=0; i<count; ++i )
            hello.ReadString( cipher );
        CORE::CDynamicBuffer payload;
        DRN::CDRNWireWriter helloWriter( payload );
        helloWriter.WriteUInt8( major );
        helloWriter.WriteUInt8( minor );
        helloWriter.WriteVarUInt32( features );
        helloWriter.WriteString( nodeId );
        helloWriter.WriteString( service );
        helloWriter.WriteVarUInt64( epoch );
        helloWriter.WriteSizedBytes( policyHash.GetConstBufferPtr(), policyHash.GetDataSize() );
        helloWriter.WriteSizedBytes( ephemeral.GetConstBufferPtr(), ephemeral.GetDataSize() );
        helloWriter.WriteVarUInt32( count > 0 ? 1 : 0 );
        if ( count > 0 )
            helloWriter.WriteString( cipher );
        DRN::CDRNPacketCodec::WriteMessage( writer, message.msgClass, message.opcode, message.lane, message.laneSequence, payload.GetConstBufferPtr(), helloWriter.GetOffset() );
        changed = true;
    }
    if ( !changed )
        return data;
    size = writer.GetOffset();
    return m_warpBuffer.GetConstBufferPtr();
}

/*-------------------------------------------------------------------------*/

bool
CDRNLossyChannel::SendPacket( const void* data, UInt32 size )
{
    if ( m_inner.IsNULL() || !m_inner->IsConnected() )
        return false;

    if ( 1.0 != m_settings.clockRate )
        data = Warp( data, size );
    if ( m_settings.downgrade )
        data = Downgrade( data, size );
    if ( !m_sentProtected && DRN::CDRNPacketProtection::IsProtectedPacket( static_cast< const UInt8* >( data ), size ) )
    {
        // the first protected packet of the initiator carries AUTH
        m_sentProtected = true;
        if ( m_settings.recordAuth )
        {
            g_recordedAuthPacket.SetDataSize( 0 );
            g_recordedAuthPacket.CopyFrom( 0, size, data );
        }
        if ( m_settings.injectAuth && g_recordedAuthPacket.GetDataSize() > 0 )
        {
            data = g_recordedAuthPacket.GetConstBufferPtr();
            size = g_recordedAuthPacket.GetDataSize();
        }
    }
    if ( m_settings.tamperRate > 0 && DRN::CDRNPacketProtection::IsProtectedPacket( static_cast< const UInt8* >( data ), size ) &&
         RandomUnit() < m_settings.tamperRate )
    {
        m_tamperBuffer.SetDataSize( 0 );
        m_tamperBuffer.CopyFrom( 0, size, data );
        static_cast< UInt8* >( m_tamperBuffer.GetBufferPtr() )[ size - 1 ] ^= 0x01;
        data = m_tamperBuffer.GetConstBufferPtr();
        ++m_tampered;
    }
    if ( size > m_maxSentSize )
        m_maxSentSize = size;

    // the lag switch: everything from holdAfter on waits until the hold ends, then goes at once and in order
    if ( m_settings.holdForMs > 0 )
    {
        UInt64 now = DRN::CDRNClock::NowInMicroseconds();
        if ( 0 == m_holdStart )
        {
            m_holdStart = now + (UInt64) m_settings.holdAfterMs * 1000;
            m_holdEnd = m_holdStart + (UInt64) m_settings.holdForMs * 1000;
        }
        if ( now >= m_holdStart && ( now < m_holdEnd || !m_delayed.empty() ) )
        {
            m_delayed.push_back( CDelayedPacket() );
            CDelayedPacket& delayed = m_delayed.back();
            delayed.dueTime = m_holdEnd;
            delayed.data.CopyFrom( 0, size, data );
            SendDue( false );
            return true;
        }
    }

    if ( m_settings.lossRate > 0 && RandomUnit() < m_settings.lossRate )
    {
        ++m_dropped;
        return true;
    }

    UInt32 copies = 1;
    if ( m_settings.duplicateRate > 0 && RandomUnit() < m_settings.duplicateRate )
    {
        copies = 2;
        ++m_duplicated;
    }

    bool needsDelay = m_settings.delayMs > 0 || m_settings.jitterMs > 0 || m_settings.reorderRate > 0;
    if ( !needsDelay )
    {
        bool sent = true;
        for ( UInt32 i=0; i<copies; ++i )
            sent = m_inner->SendPacket( data, size ) && sent;
        return sent;
    }

    UInt64 now = DRN::CDRNClock::NowInMicroseconds();
    for ( UInt32 i=0; i<copies; ++i )
    {
        UInt64 delayUs = (UInt64) m_settings.delayMs * 1000;
        if ( m_settings.jitterMs > 0 )
            delayUs += (UInt64) ( RandomUnit() * m_settings.jitterMs * 1000 );
        if ( m_settings.reorderRate > 0 && RandomUnit() < m_settings.reorderRate )
            delayUs += 2000 + (UInt64) ( RandomUnit() * 18000 );

        m_delayed.push_back( CDelayedPacket() );
        CDelayedPacket& delayed = m_delayed.back();
        delayed.dueTime = now + delayUs;
        delayed.data.CopyFrom( 0, size, data );
    }
    SendDue( false );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNLossyChannel::SendDue( bool all )
{
    if ( m_delayed.empty() || m_inner.IsNULL() )
        return;

    UInt64 now = DRN::CDRNClock::NowInMicroseconds();
    for ( UInt32 i=0; i<m_delayed.size(); )
    {
        if ( all || m_delayed[ i ].dueTime <= now )
        {
            CDelayedPacket packet = m_delayed[ i ];
            m_delayed.erase( m_delayed.begin() + i );
            if ( m_inner->IsConnected() )
                m_inner->SendPacket( packet.data.GetConstBufferPtr(), packet.data.GetDataSize() );
            if ( m_inner.IsNULL() )
                return;
        }
        else
        {
            ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/

const DRN::CDRNChannelCapabilities&
CDRNLossyChannel::GetCapabilities( void ) const
{
    return m_capabilities;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNEndpoint
CDRNLossyChannel::GetRemoteEndpoint( void ) const
{
    if ( m_inner.IsNULL() )
        return DRN::CDRNEndpoint();
    DRN::CDRNEndpoint endpoint = m_inner->GetRemoteEndpoint();
    endpoint.SetChannelType( GetChannelType() );
    return endpoint;
}

/*-------------------------------------------------------------------------*/

const GUCEF::CORE::CString&
CDRNLossyChannel::ClassTypeName( void )
{
    static const GUCEF::CORE::CString classTypeName = "CDRNLossyChannel";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const GUCEF::CORE::CString&
CDRNLossyChannel::GetClassTypeName( void ) const
{
    return ClassTypeName();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNLossyChannel::GetChannelType( void ) const
{
    return LossyPrefix + m_innerType;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNLossyChannel::GetDroppedCount( void ) const
{
    return m_dropped;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNLossyChannel::GetDuplicatedCount( void ) const
{
    return m_duplicated;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNLossyChannel::GetTamperedCount( void ) const
{
    return m_tampered;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNLossyChannel::GetMaxSentPacketSize( void ) const
{
    return m_maxSentSize;
}

/*-------------------------------------------------------------------------*/

const CORE::CDynamicBuffer&
CDRNLossyChannel::GetRecordedAuthPacket( void )
{
    return g_recordedAuthPacket;
}

/*-------------------------------------------------------------------------*/

void
CDRNLossyChannel::ClearRecordedAuthPacket( void )
{
    g_recordedAuthPacket.Clear();
}

/*-------------------------------------------------------------------------*/

void
CDRNLossyChannel::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata )
{
    if ( CORE::CPulseGenerator::PulseEvent == eventid )
    {
        SendDue( false );
        return;
    }
    if ( m_inner.IsNULL() || notifier != m_inner.GetPointerAlways() )
        return;

    if ( DRN::CIDRNChannel::PacketReceivedEvent == eventid ||
         DRN::CIDRNChannel::ChannelConnectedEvent == eventid ||
         DRN::CIDRNChannel::ChannelDisconnectedEvent == eventid ||
         DRN::CIDRNChannel::ChannelErrorEvent == eventid )
    {
        if ( DRN::CIDRNChannel::ChannelConnectedEvent == eventid )
            UpdateCapabilities();
        NotifyObservers( eventid, eventdata );
    }
}

/*-------------------------------------------------------------------------*/

CDRNLossyChannelListener::CDRNLossyChannelListener( const CORE::CString& innerType )
    : DRN::CIDRNChannelListener()
    , m_innerType( innerType )
    , m_inner()
    , m_settings()
    , m_listenEndpoint()
    , m_pulseGenerator()
{

}

/*-------------------------------------------------------------------------*/

CDRNLossyChannelListener::~CDRNLossyChannelListener()
{
    SignalUpcomingObserverDestruction();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CDRNLossyChannelListener::Listen( const DRN::CDRNEndpoint& local )
{
    m_settings.FromParams( local.GetParams() );
    m_listenEndpoint = local;
    if ( m_inner.IsNULL() )
    {
        m_inner = DRN::CIDRNChannelListenerPtr( DRN::CDRNGlobal::Instance()->GetChannelListenerFactory().Create( m_innerType ) );
        if ( m_inner.IsNULL() )
            return false;
        m_inner->SetPulseGenerator( m_pulseGenerator );
        SubscribeTo( m_inner.GetPointerAlways() );
    }
    return m_inner->Listen( DRN::CDRNEndpoint( m_innerType, local.GetAddress() ) );
}

/*-------------------------------------------------------------------------*/

void
CDRNLossyChannelListener::StopListening( void )
{
    if ( !m_inner.IsNULL() )
        m_inner->StopListening();
}

/*-------------------------------------------------------------------------*/

bool
CDRNLossyChannelListener::IsListening( void ) const
{
    return !m_inner.IsNULL() && m_inner->IsListening();
}

/*-------------------------------------------------------------------------*/

DRN::CDRNEndpoint
CDRNLossyChannelListener::GetListenEndpoint( void ) const
{
    return m_listenEndpoint;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNLossyChannelListener::GetChannelType( void ) const
{
    return LossyPrefix + m_innerType;
}

/*-------------------------------------------------------------------------*/

void
CDRNLossyChannelListener::SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator )
{
    m_pulseGenerator = pulseGenerator;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CDRNLossyChannelListener::GetPulseGenerator( void ) const
{
    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

void
CDRNLossyChannelListener::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata )
{
    if ( m_inner.IsNULL() || notifier != m_inner.GetPointerAlways() || GUCEF_NULL == eventdata )
        return;

    if ( DRN::CIDRNChannelListener::ChannelAcceptedEvent == eventid )
    {
        const DRN::CIDRNChannelPtr& innerChannel = static_cast< DRN::CDRNChannelEventData* >( eventdata )->GetData();
        DRN::CIDRNChannelPtr channel( GUCEF_NEW CDRNLossyChannel( innerChannel, m_settings, m_innerType ) );
        DRN::CDRNChannelEventData channelData( channel );
        NotifyObservers( ChannelAcceptedEvent, &channelData );
    }
}

/*-------------------------------------------------------------------------*/
