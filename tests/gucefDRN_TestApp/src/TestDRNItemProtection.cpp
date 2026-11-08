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

#include <string.h>

#include <exception>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDRNTestUtils.h"
#include "TestDRNGroup.h"
#include "TestDRNLossyChannel.h"
#include "TestDRNPeerLinkTestAccess.h"
#include "TestDRNItemProtection.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace DRNTEST;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE(t)  do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw std::exception(); } } while ( 0 )

namespace {

/*-------------------------------------------------------------------------*/

/** The SHA-256 of the canonical encoding (version 3) of a default policy with allowPublicItems and mac, computed outside DRN */
static const char* PublicMacPolicyHash = "c361e8b3176ddd9e7274878779e00d3376376b3f27f4f3a29529378d30fdc682";

static const UInt32 TestOpcode = 91;

/*-------------------------------------------------------------------------*/

UInt64
NowMs( void )
{
    return DRN::CDRNClock::NowInMicroseconds() / 1000;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNNodeSettings
PublicNodeSettings( DRN::TDRNPublicItemProtection protection )
{
    DRN::CDRNNodeSettings settings;
    settings.echoIntervalMs = 50;
    settings.pulseIntervalMs = 1;
    settings.securityPolicy.allowPublicItems = true;
    settings.securityPolicy.publicItemProtection = protection;
    return settings;
}

/*-------------------------------------------------------------------------*/

bool
Contains( const CORE::CDynamicBuffer& buffer, const CORE::CString& text )
{
    const UInt8* data = static_cast< const UInt8* >( buffer.GetConstBufferPtr() );
    UInt32 size = buffer.GetDataSize();
    UInt32 length = text.Length();
    if ( 0 == length || size < length )
        return false;
    for ( UInt32 i=0; i+length<=size; ++i )
        if ( 0 == memcmp( data + i, text.C_String(), length ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/** Records what a link delivers: the counter in the first 4 bytes of the payload, the class and the packet kind */
class CKindRecorder : public CORE::CObserver
{
    public:

    class CEntry
    {
        public:
        UInt32 counter;
        UInt8 lane;
        UInt8 msgClass;
        UInt32 opcode;
        UInt8 packetKind;
    };

    GUCEF::vector< CEntry > entries;
    UInt32 anomalies;
    CORE::CString lastAnomalyText;
    UInt32 closed;

    CKindRecorder( void ) : CORE::CObserver(), entries(), anomalies( 0 ), lastAnomalyText(), closed( 0 ) {}
    virtual ~CKindRecorder() { SignalUpcomingObserverDestruction(); }

    UInt32 Count( UInt8 msgClass, UInt8 packetKind ) const
    {
        UInt32 count = 0;
        for ( UInt32 i=0; i<entries.size(); ++i )
            if ( entries[ i ].msgClass == msgClass && entries[ i ].packetKind == packetKind )
                ++count;
        return count;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( DRN::CDRNPeerLink::MessageReceivedEvent == eventid && GUCEF_NULL != eventdata )
        {
            const DRN::CDRNLinkMessage& message = static_cast< DRN::CDRNLinkMessageEventData* >( eventdata )->GetData();
            CEntry entry;
            entry.counter = 0;
            if ( message.payload.GetDataSize() >= 4 )
            {
                const UInt8* p = static_cast< const UInt8* >( message.payload.GetConstBufferPtr() );
                entry.counter = ( (UInt32) p[ 0 ] << 24 ) | ( (UInt32) p[ 1 ] << 16 ) | ( (UInt32) p[ 2 ] << 8 ) | p[ 3 ];
            }
            entry.lane = message.lane;
            entry.msgClass = message.msgClass;
            entry.opcode = message.opcode;
            entry.packetKind = message.packetKind;
            entries.push_back( entry );
        }
        else
        if ( ( DRN::CDRNPeerLink::ProtectionAnomalyEvent == eventid || DRN::CDRNNode::ProtectionAnomalyEvent == eventid ) && GUCEF_NULL != eventdata )
        {
            ++anomalies;
            if ( DRN::CDRNPeerLink::ProtectionAnomalyEvent == eventid )
                lastAnomalyText = static_cast< DRN::CDRNLinkAnomalyEventData* >( eventdata )->GetData().text;
            else
                lastAnomalyText = static_cast< DRN::CDRNNodeLinkEventData* >( eventdata )->GetData().anomaly.text;
        }
        else
        if ( DRN::CDRNPeerLink::LinkClosedEvent == eventid )
        {
            ++closed;
        }
    }
};

/*-------------------------------------------------------------------------*/

void
MakeCounted( CORE::CDynamicBuffer& buffer, UInt32 counter, UInt32 size )
{
    buffer.SetDataSize( size < 4 ? 4 : size );
    UInt8* p = static_cast< UInt8* >( buffer.GetBufferPtr() );
    memset( p, 0x5A, buffer.GetDataSize() );
    p[ 0 ] = (UInt8) ( counter >> 24 );
    p[ 1 ] = (UInt8) ( counter >> 16 );
    p[ 2 ] = (UInt8) ( counter >> 8 );
    p[ 3 ] = (UInt8) counter;
}

/*-------------------------------------------------------------------------*/

bool
SetupPublicPair( CLinkedPair& pair, const CORE::CString& channelType, const CORE::CString& params, DRN::TDRNPublicItemProtection protection )
{
    UInt16 port = NextTestPort();
    DRN::CDRNNodeSettings settings = PublicNodeSettings( protection );
    DRN::CDRNNodeSettings settingsB = settings;
    settingsB.nodeId = DRN::CDRNNodeSettings::GenerateNodeId();
    return pair.Setup( settings, settingsB, LoopbackEndpoint( channelType, port ), LoopbackEndpoint( channelType, port, params ) );
}

/*-------------------------------------------------------------------------*/

/** A codec instance of an AEAD provider with a random key */
bool
MakeAeadInstance( CORE::CCodecRegistry::TICodecPtr codec, const UInt8* key, CORE::CICodecInstancePtr& instance )
{
    CORE::CValueList params;
    params.Set( "key", CORE::CVariant( key, 32 ) );
    return codec->CreateInstance( params, instance ) && !instance.IsNULL();
}

/*-------------------------------------------------------------------------*/

/** A PLAIN packet as anyone on the path could make it: given sequence, acks claiming everything, one message */
void
ForgePlainPacket( CORE::CDynamicBuffer& packet, UInt32 sequence, UInt32 ackSequence, UInt8 lane, UInt32 laneSequence, UInt8 msgClass, UInt32 opcode, const CORE::CDynamicBuffer& payload )
{
    packet.SetDataSize( 0 );
    DRN::CDRNWireWriter writer( packet );
    writer.WriteUInt8( DRN::DRN_PROTOCOL_ID_PUBLIC_PLAIN );
    writer.WriteVarUInt32( sequence );
    writer.WriteVarUInt64( 1 );
    writer.WriteVarUInt32( ackSequence );
    writer.WriteUInt32( 0xFFFFFFFF );
    writer.WriteVarUInt64( 1 );
    writer.WriteVarUInt64( 0 );
    DRN::CDRNPacketCodec::WriteMessage( writer, msgClass, opcode, lane, laneSequence, payload.GetConstBufferPtr(), payload.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

/** Takes every message the link layer hands up */
class CCollectingSink : public DRN::CIDRNLinkMessageSink
{
    public:
    GUCEF::vector< DRN::CDRNMessageView > messages;
    virtual bool OnLinkMessage( const DRN::CDRNMessageView& message ) GUCEF_VIRTUAL_OVERRIDE { messages.push_back( message ); return true; }
};

/*-------------------------------------------------------------------------*/

/** Grid values that are easy to find in a capture */
CORE::CString
MarkedValue( const char* marker, UInt32 i )
{
    return CORE::CString( marker ) + CORE::ToString( 1000 + i );
}

/*-------------------------------------------------------------------------*/

/**
 *  CLIENT_SERVER over a capturing channel: the client publishes a PUBLIC and a CONFIDENTIAL reliable grid, the server
 *  subscribes to both. Returns false when the values did not arrive.
 */
bool
RunCapture( const CORE::CString& channel, DRN::TDRNPublicItemProtection protection, bool allowPublic, UInt32& macPackets, UInt32& mixedPackets, bool& publicSeen, bool& secretSeen )
{
    macPackets = 0;
    mixedPackets = 0;
    publicSeen = false;
    secretSeen = false;
    CDRNLossyChannel::ClearCapturedPackets();

    CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "capture", "lossy-" + channel, "?capture=1" );
    group.nodeSettings = PublicNodeSettings( protection );
    group.nodeSettings.securityPolicy.allowPublicItems = allowPublic;
    if ( !group.Start( 2 ) )
        return false;

    DRN::CDRNDataGridSettings settings;
    settings.name = "prices";
    settings.authority = DRN::DRN_AUTHORITY_OWNER;
    settings.reliable = true;
    settings.protection = DRN::DRN_PROTECTION_PUBLIC;
    DRN::CDRNDataGrid* prices = group[ 1 ].CreateGrid( settings );
    settings.name = "holdings";
    settings.protection = DRN::DRN_PROTECTION_CONFIDENTIAL;
    DRN::CDRNDataGrid* holdings = group[ 1 ].CreateGrid( settings );
    if ( GUCEF_NULL == prices || GUCEF_NULL == holdings )
        return false;
    for ( UInt32 i=0; i<40; ++i )
    {
        prices->SetValue( CORE::CVariant( i ), CORE::CVariant( MarkedValue( "PUBLICVALUE", i ) ) );
        holdings->SetValue( CORE::CVariant( i ), CORE::CVariant( MarkedValue( "SECRETVALUE", i ) ) );
    }
    group[ 0 ].Subscribe( "prices" );
    group[ 0 ].Subscribe( "holdings" );
    CGridCondition arrived;
    arrived.Expect( group[ 0 ], "prices", group[ 1 ].GetLocalPeerId(), CORE::CVariant( (UInt32) 39 ), CORE::CVariant( MarkedValue( "PUBLICVALUE", 39 ) ) );
    arrived.Expect( group[ 0 ], "holdings", group[ 1 ].GetLocalPeerId(), CORE::CVariant( (UInt32) 39 ), CORE::CVariant( MarkedValue( "SECRETVALUE", 39 ) ) );
    if ( !PumpUntil( arrived, 10000 ) )
        return false;
    PumpFor( 50 );

    // what an observer of the link saw
    const CDRNLossyChannel::TPacketVector& packets = CDRNLossyChannel::GetCapturedPackets();
    for ( UInt32 p=0; p<packets.size(); ++p )
    {
        const CORE::CDynamicBuffer& packet = packets[ p ];
        if ( Contains( packet, "SECRETVALUE" ) )
            secretSeen = true;
        if ( Contains( packet, "PUBLICVALUE" ) )
            publicSeen = true;
        const UInt8* bytes = static_cast< const UInt8* >( packet.GetConstBufferPtr() );
        if ( !DRN::CDRNPacketProtection::IsMacPacket( bytes, packet.GetDataSize() ) || packet.GetDataSize() <= DRN::CDRNPacketProtection::TagSize )
            continue;

        // a MAC packet carries public item data only: read its clear messages
        ++macPackets;
        CORE::CDynamicBuffer clear;
        clear.CopyFrom( 0, packet.GetDataSize() - DRN::CDRNPacketProtection::TagSize, bytes );
        static_cast< UInt8* >( clear.GetBufferPtr() )[ 0 ] = DRN::DRN_PROTOCOL_ID;
        DRN::CDRNWireReader reader( clear );
        DRN::CDRNPacketHeader header;
        if ( !DRN::CDRNPacketCodec::ReadHeader( reader, header ) )
        {
            ++mixedPackets;
            continue;
        }
        while ( reader.GetRemaining() > 0 )
        {
            DRN::CDRNMessageView message;
            if ( !DRN::CDRNPacketCodec::ReadMessage( reader, message ) || !DRN::CDRNDataItem::IsPublicDataMessage( message.msgClass, message.opcode ) )
            {
                ++mixedPackets;
                break;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** A GRID_DELTA of one key as anyone who saw the item reference can write it */
void
MakeForgedDelta( CORE::CDynamicBuffer& delta, UInt32 owner, UInt32 item, UInt32 seq, UInt8 flags, UInt32 index, const CORE::CVariant& value )
{
    delta.SetDataSize( 0 );
    DRN::CDRNWireWriter writer( delta );
    writer.WriteVarUInt32( owner );
    writer.WriteVarUInt32( item );
    writer.WriteVarUInt32( seq );
    writer.WriteUInt8( flags );
    if ( 0 != ( flags & 0x01 ) && 0 == ( flags & 0x02 ) )
        writer.WriteVarUInt32( seq - 1 );       // the reliable delta before it
    writer.WriteVarUInt64( 1 );
    writer.WriteUInt16( 1 );
    writer.WriteVarUInt32( index );
    if ( 0 != ( flags & 0x04 ) )
    {
        writer.WriteVarUInt32( 99 );            // revision
        writer.WriteVarUInt32( owner );         // writer
    }
    DRN::CDRNVariantCodec::Write( writer, value );
}

/*-------------------------------------------------------------------------*/

/** A PLAIN packet with one message, sent through the channel of a link as if its peer sent it */
void
SendForgedPlain( DRN::CIDRNChannelPtr channel, UInt32 sequence, UInt8 lane, UInt8 msgClass, UInt32 opcode, const CORE::CDynamicBuffer& payload )
{
    CORE::CDynamicBuffer packet;
    ForgePlainPacket( packet, sequence, 0, lane, 0, msgClass, opcode, payload );
    channel->SendPacket( packet.GetConstBufferPtr(), packet.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

/** Pumps until the link refused at least count more messages than before */
bool
WaitForDowngrades( const DRN::CDRNPeerLinkPtr& link, UInt64 before, UInt64 count, UInt32 timeoutMs )
{
    UInt64 start = NowMs();
    while ( link->GetProtectionDowngradeCount() < before + count && NowMs() - start < timeoutMs )
        PumpFor( 5 );
    PumpFor( 50 );
    return link->GetProtectionDowngradeCount() >= before + count;
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDRNItemProtectionTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING DRN ITEM PROTECTION TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.ItemProtection" );

    GUCEF_TESTFW_TESTCASE( "Protection: every AEAD provider makes and verifies a MAC with an empty plaintext; a changed header, body or tag fails" )
    try
    {
        CORE::CCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
        const char* types[] = { "xchacha20-poly1305", "chacha20-poly1305", "aes-256-gcm" };
        const UInt32 nonceSizes[] = { 24, 12, 12 };
        UInt32 providers = 0;
        for ( UInt32 t=0; t<3; ++t )
        {
            CORE::CCodecRegistry::TICodecPtrVector codecs;
            registry.GetCodecs( CORE::CoreCodecTypes::EncryptionCodec, types[ t ], codecs );
            for ( UInt32 c=0; c<codecs.size(); ++c )
            {
                UInt8 key[ 32 ];
                REQUIRE_TRUE( DRN::CDRNCrypto::RandomBytes( key, sizeof( key ) ) );
                CORE::CICodecInstancePtr maker;
                CORE::CICodecInstancePtr verifier;
                REQUIRE_TRUE( MakeAeadInstance( codecs[ c ], key, maker ) && MakeAeadInstance( codecs[ c ], key, verifier ) );

                UInt8 nonce[ 24 ];
                memset( nonce, 0, sizeof( nonce ) );
                nonce[ nonceSizes[ t ] - 1 ] = 7;
                CORE::CDynamicBuffer packet;
                packet.SetDataSize( 1200 );
                for ( UInt32 i=0; i<packet.GetDataSize(); ++i )
                    static_cast< UInt8* >( packet.GetBufferPtr() )[ i ] = (UInt8) i;
                static_cast< UInt8* >( packet.GetBufferPtr() )[ 0 ] = DRN::DRN_PROTOCOL_ID_MAC_0;

                CORE::SCodecCallArgs args;
                memset( &args, 0, sizeof( args ) );
                args.structSize = sizeof( args );
                args.nonce = nonce;
                args.nonceSize = nonceSizes[ t ];
                args.aad = packet.GetConstBufferPtr();
                args.aadSize = packet.GetDataSize();
                CORE::CDynamicBuffer tag;
                ASSERT_TRUE( maker->Process( GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, GUCEF_NULL, 0, &args, tag ) );
                ASSERT_TRUE( DRN::CDRNPacketProtection::TagSize == tag.GetDataSize() );

                CORE::CDynamicBuffer none;
                ASSERT_TRUE( verifier->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, tag.GetConstBufferPtr(), tag.GetDataSize(), &args, none ) && 0 == none.GetDataSize() );

                // a changed header byte, body byte or tag byte: refused
                const UInt32 changeAt[] = { 0, 600 };
                for ( UInt32 n=0; n<2; ++n )
                {
                    static_cast< UInt8* >( packet.GetBufferPtr() )[ changeAt[ n ] ] ^= 0x04;
                    none.SetDataSize( 0 );
                    ASSERT_FALSE( verifier->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, tag.GetConstBufferPtr(), tag.GetDataSize(), &args, none ) );
                    static_cast< UInt8* >( packet.GetBufferPtr() )[ changeAt[ n ] ] ^= 0x04;
                }
                static_cast< UInt8* >( tag.GetBufferPtr() )[ 5 ] ^= 0x01;
                none.SetDataSize( 0 );
                ASSERT_FALSE( verifier->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, tag.GetConstBufferPtr(), tag.GetDataSize(), &args, none ) );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Protection: " << types[ t ] << " provider " << codecs[ c ]->GetClassTypeName() << " makes and verifies MACs with an empty plaintext" );
                ++providers;
            }
        }
        ASSERT_TRUE( providers >= 2 );

        // through CDRNPacketProtection: a MAC packet opens, as MAC, and a MAC packet relabelled as AEAD does not (and back)
        UInt8 keyOne[ 32 ];
        UInt8 keyTwo[ 32 ];
        REQUIRE_TRUE( DRN::CDRNCrypto::RandomBytes( keyOne, 32 ) && DRN::CDRNCrypto::RandomBytes( keyTwo, 32 ) );
        DRN::CDRNPacketProtection sender;
        DRN::CDRNPacketProtection receiver;
        CORE::CString cipher = DRN::CDRNSecurityPolicy().ciphers.front();
        REQUIRE_TRUE( sender.Setup( cipher, keyOne, keyTwo, 0 ) && receiver.Setup( cipher, keyTwo, keyOne, 1 ) );
        CORE::CDynamicBuffer plain;
        {
            DRN::CDRNWireWriter writer( plain );
            writer.WriteUInt8( DRN::DRN_PROTOCOL_ID );
            writer.WriteVarUInt32( 5 );
            writer.WriteVarUInt64( 1000 );
            writer.WriteVarUInt32( 0 );
            writer.WriteUInt32( 0 );
            writer.WriteVarUInt64( 0 );
            writer.WriteVarUInt64( 0 );
            writer.WriteString( "a clear body" );
        }
        CORE::CDynamicBuffer sealed;
        CORE::CDynamicBuffer opened;
        UInt8 kind = 0;
        REQUIRE_TRUE( sender.Protect( static_cast< const UInt8* >( plain.GetConstBufferPtr() ), plain.GetDataSize(), sealed, DRN::DRN_PACKETKIND_MAC ) );
        ASSERT_TRUE( Contains( sealed, "a clear body" ) && sealed.GetDataSize() == plain.GetDataSize() + DRN::CDRNPacketProtection::TagSize );
        ASSERT_TRUE( receiver.Unprotect( static_cast< const UInt8* >( sealed.GetConstBufferPtr() ), sealed.GetDataSize(), opened, kind ) && DRN::DRN_PACKETKIND_MAC == kind );
        ASSERT_TRUE( opened.GetDataSize() == plain.GetDataSize() && 0 == memcmp( opened.GetConstBufferPtr(), plain.GetConstBufferPtr(), plain.GetDataSize() ) );
        static_cast< UInt8* >( sealed.GetBufferPtr() )[ 0 ] = DRN::DRN_PROTOCOL_ID_PROTECTED_0;
        ASSERT_FALSE( receiver.Unprotect( static_cast< const UInt8* >( sealed.GetConstBufferPtr() ), sealed.GetDataSize(), opened, kind ) );
        REQUIRE_TRUE( sender.Protect( static_cast< const UInt8* >( plain.GetConstBufferPtr() ), plain.GetDataSize(), sealed, DRN::DRN_PACKETKIND_AEAD ) );
        ASSERT_FALSE( Contains( sealed, "a clear body" ) );
        static_cast< UInt8* >( sealed.GetBufferPtr() )[ 0 ] = DRN::DRN_PROTOCOL_ID_MAC_0;
        ASSERT_FALSE( receiver.Unprotect( static_cast< const UInt8* >( sealed.GetConstBufferPtr() ), sealed.GetDataSize(), opened, kind ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Protection: a capture shows public values in clear in MAC packets only, no confidential value anywhere, no packet mixes kinds" )
    try
    {
        const char* channels[] = { "udp", "tcp" };
        for ( UInt32 c=0; c<2; ++c )
        {
            UInt32 macPackets = 0;
            UInt32 mixedPackets = 0;
            bool publicSeen = false;
            bool secretSeen = false;
            ASSERT_TRUE( RunCapture( channels[ c ], DRN::DRN_PUBLICPROTECTION_MAC, true, macPackets, mixedPackets, publicSeen, secretSeen ) );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Protection capture over " << channels[ c ] << ": " << macPackets << " MAC packets, " << mixedPackets << " mixed" );
            ASSERT_TRUE( macPackets > 0 && 0 == mixedPackets );
            ASSERT_TRUE( publicSeen );
            ASSERT_FALSE( secretSeen );

            // without allowPublicItems: everything sealed, the public grid too
            ASSERT_TRUE( RunCapture( channels[ c ], DRN::DRN_PUBLICPROTECTION_MAC, false, macPackets, mixedPackets, publicSeen, secretSeen ) );
            ASSERT_TRUE( 0 == macPackets && !publicSeen && !secretSeen );
        }
        CDRNLossyChannel::ClearCapturedPackets();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Protection: tampered MAC packets are dropped on udp and close the link on tcp; the subscriber keeps the true values" )
    try
    {
        // udp: a share of the MAC packets changed on the way; the reliable public grid still arrives right
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "tamper", "lossy-udp", "?tamperMac=0.3" );
        group.nodeSettings = PublicNodeSettings( DRN::DRN_PUBLICPROTECTION_MAC );
        REQUIRE_TRUE( group.Start( 2 ) );
        DRN::CDRNDataGridSettings settings;
        settings.name = "quotes";
        settings.authority = DRN::DRN_AUTHORITY_OWNER;
        settings.reliable = true;
        settings.protection = DRN::DRN_PROTECTION_PUBLIC;
        DRN::CDRNDataGrid* quotes = group[ 1 ].CreateGrid( settings );
        REQUIRE_TRUE( GUCEF_NULL != quotes );
        for ( UInt32 i=0; i<200; ++i )
            quotes->SetValue( CORE::CVariant( i ), CORE::CVariant( MarkedValue( "QUOTE", i ) ) );
        group[ 0 ].Subscribe( "quotes" );
        CGridCondition arrived;
        arrived.Expect( group[ 0 ], "quotes", group[ 1 ].GetLocalPeerId(), CORE::CVariant( (UInt32) 199 ), CORE::CVariant( MarkedValue( "QUOTE", 199 ) ) );
        REQUIRE_TRUE( PumpUntil( arrived, 20000 ) );

        // a share of the packets is changed: when the snapshot took so few packets that none was, more updates follow
        // until one was (each round a new value per key, the last round is what the replica has to show)
        DRN::CDRNPeerLinkPtr tamperedLink = group[ 0 ].GetLinkToMember( group[ 1 ].GetLocalPeerId() );
        REQUIRE_TRUE( !tamperedLink.IsNULL() );
        for ( UInt32 round=0; round<20 && 0 == tamperedLink->GetAuthenticationFailureCount(); ++round )
        {
            for ( UInt32 i=0; i<200; ++i )
                quotes->SetValue( CORE::CVariant( i ), CORE::CVariant( MarkedValue( "QUOTE", 1000 * ( round + 1 ) + i ) ) );
            CGridCondition updated;
            updated.Expect( group[ 0 ], "quotes", group[ 1 ].GetLocalPeerId(), CORE::CVariant( (UInt32) 199 ), CORE::CVariant( MarkedValue( "QUOTE", 1000 * ( round + 1 ) + 199 ) ) );
            REQUIRE_TRUE( PumpUntil( updated, 20000 ) );
        }
        DRN::CDRNDataGrid* replica = group[ 0 ].FindGrid( "quotes" );
        bool allTrue = true;
        for ( UInt32 i=0; i<200; ++i )
        {
            CORE::CVariant value;
            CORE::CVariant expected;
            quotes->GetValue( CORE::CVariant( i ), expected );
            if ( !replica->GetValue( CORE::CVariant( i ), value ) || value.AsString() != expected.AsString() )
                allTrue = false;
        }
        ASSERT_TRUE( allTrue );
        DRN::CDRNPeerLinkPtr serverLink = group[ 0 ].GetLinkToMember( group[ 1 ].GetLocalPeerId() );
        REQUIRE_TRUE( !serverLink.IsNULL() );
        ASSERT_TRUE( serverLink->GetAuthenticationFailureCount() > 0 && serverLink->IsOperational() );

        // tcp: one changed MAC packet breaks the stream, the link closes
        CLinkedPair pair;
        REQUIRE_TRUE( SetupPublicPair( pair, "lossy-tcp", "?tamperMac=1", DRN::DRN_PUBLICPROTECTION_MAC ) );
        CORE::CDynamicBuffer payload;
        MakeCounted( payload, 1, 64 );
        ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_REPL, TestOpcode, payload, DRN::DRN_PACKETKIND_MAC ) );
        pair.linkA->Flush();
        ASSERT_TRUE( PumpUntil( pair.recorderB.closedCount, 1, 5000 ) );
        ASSERT_TRUE( DRN::DRN_LINKREASON_PROTOCOL_ERROR == pair.recorderB.lastClosedReason );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Protection: confidential data, APP, SESSION and announcements in MAC or PLAIN packets are refused and reported, nothing reaches the application" )
    try
    {
        const DRN::TDRNPublicItemProtection modes[] = { DRN::DRN_PUBLICPROTECTION_MAC, DRN::DRN_PUBLICPROTECTION_NONE };
        for ( UInt32 m=0; m<2; ++m )
        {
            CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "downgrade", "udp" );
            group.nodeSettings = PublicNodeSettings( modes[ m ] );
            REQUIRE_TRUE( group.Start( 2 ) );
            CKindRecorder serverEvents;
            serverEvents.SubscribeTo( group.nodes[ 0 ] );

            // a confidential grid the server subscribed to
            DRN::CDRNDataGridSettings settings;
            settings.name = "secret";
            settings.authority = DRN::DRN_AUTHORITY_OWNER;
            settings.reliable = true;
            DRN::CDRNDataGrid* secret = group[ 1 ].CreateGrid( settings );
            REQUIRE_TRUE( GUCEF_NULL != secret );
            secret->SetValue( CORE::CVariant( (UInt32) 1 ), CORE::CVariant( "true value" ) );
            group[ 0 ].Subscribe( "secret" );
            CGridCondition arrived;
            arrived.Expect( group[ 0 ], "secret", group[ 1 ].GetLocalPeerId(), CORE::CVariant( (UInt32) 1 ), CORE::CVariant( "true value" ) );
            REQUIRE_TRUE( PumpUntil( arrived, 10000 ) );
            UInt32 itemsBefore = 0;
            {
                DRN::TDRNItemInfoVector items;
                group[ 0 ].GetReplicator().GetItems( items );
                itemsBefore = (UInt32) items.size();
            }

            DRN::CDRNPeerLinkPtr clientLink = group[ 1 ].GetLinkToMember( group[ 0 ].GetLocalPeerId() );
            DRN::CDRNPeerLinkPtr serverLink = group[ 0 ].GetLinkToMember( group[ 1 ].GetLocalPeerId() );
            REQUIRE_TRUE( !clientLink.IsNULL() && !serverLink.IsNULL() );
            UInt64 downgradesBefore = serverLink->GetProtectionDowngradeCount();

            // a GRID_DELTA of the confidential grid, an APP message, a SESSION message and an ITEM_ANNOUNCE
            CORE::CDynamicBuffer delta;
            {
                DRN::CDRNWireWriter writer( delta );
                writer.WriteVarUInt32( secret->GetOwnerPeerId() );
                writer.WriteVarUInt32( secret->GetItemId() );
                writer.WriteString( "forged" );
            }
            CORE::CDynamicBuffer announce;
            {
                DRN::CDRNWireWriter writer( announce );
                writer.WriteVarUInt32( group[ 1 ].GetLocalPeerId() );
                writer.WriteVarUInt32( 77 );
                writer.WriteString( "injected" );
                writer.WriteUInt8( 1 );
            }
            CORE::CDynamicBuffer app;
            MakeCounted( app, 5, 16 );
            UInt8 kind = DRN::DRN_PUBLICPROTECTION_MAC == modes[ m ] ? (UInt8) DRN::DRN_PACKETKIND_MAC : (UInt8) DRN::DRN_PACKETKIND_PLAIN;
            UInt8 lane = DRN::DRN_PACKETKIND_PLAIN == kind ? (UInt8) DRN::DRN_LANE_UNRELIABLE : (UInt8) DRN::DRN_LANE_RELIABLE_ORDERED;
            ASSERT_TRUE( DRN::CDRNPeerLinkTestAccess::SendAsPacketKind( *clientLink, lane, DRN::DRN_MSGCLASS_REPL, DRN::DRN_REPL_GRID_DELTA, delta.GetConstBufferPtr(), delta.GetDataSize(), kind ) );
            ASSERT_TRUE( DRN::CDRNPeerLinkTestAccess::SendAsPacketKind( *clientLink, lane, DRN::DRN_MSGCLASS_APP, 1, app.GetConstBufferPtr(), app.GetDataSize(), kind ) );
            ASSERT_TRUE( DRN::CDRNPeerLinkTestAccess::SendAsPacketKind( *clientLink, lane, DRN::DRN_MSGCLASS_SESSION, DRN::DRN_SESSION_LEAVE, app.GetConstBufferPtr(), app.GetDataSize(), kind ) );
            ASSERT_TRUE( DRN::CDRNPeerLinkTestAccess::SendAsPacketKind( *clientLink, lane, DRN::DRN_MSGCLASS_REPL, DRN::DRN_REPL_ITEM_ANNOUNCE, announce.GetConstBufferPtr(), announce.GetDataSize(), kind ) );
            clientLink->Flush();
            UInt64 start = NowMs();
            while ( serverLink->GetProtectionDowngradeCount() < downgradesBefore + 4 && NowMs() - start < 5000 )
                PumpFor( 5 );
            PumpFor( 50 );

            ASSERT_TRUE( serverLink->GetProtectionDowngradeCount() == downgradesBefore + 4 );
            ASSERT_TRUE( serverEvents.anomalies >= 1 && serverEvents.lastAnomalyText.HasSubstr( "class", true ) >= 0 );
            ASSERT_TRUE( serverLink->IsOperational() && group[ 0 ].IsActive() && 2 == group[ 0 ].GetMemberCount() );
            DRN::CDRNDataGrid* replica = group[ 0 ].FindGrid( "secret" );
            CORE::CVariant value;
            ASSERT_TRUE( GUCEF_NULL != replica && replica->GetValue( CORE::CVariant( (UInt32) 1 ), value ) && value.AsString() == "true value" );
            DRN::TDRNItemInfoVector items;
            group[ 0 ].GetReplicator().GetItems( items );
            ASSERT_TRUE( items.size() == itemsBefore );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Protection downgrade (" << ( 0 == m ? "mac" : "none" ) << "): " << serverLink->GetProtectionDowngradeCount() - downgradesBefore << " refused, last report: " << serverEvents.lastAnomalyText );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Protection: forged PLAIN packets suppress no reliable message and acknowledge nothing" )
    try
    {
        // the link layer alone: forged acks in a PLAIN packet leave what is in flight in flight
        DRN::CDRNLinkReliability reliability;
        reliability.Reset( false, 1200, 1 );
        CORE::CDynamicBuffer payload;
        MakeCounted( payload, 1, 32 );
        for ( UInt32 i=0; i<5; ++i )
            REQUIRE_TRUE( reliability.QueueMessage( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, payload.GetConstBufferPtr(), payload.GetDataSize(), GUCEF_NULL, 0 ) );
        const UInt8* packet = GUCEF_NULL;
        UInt32 packetSize = 0;
        while ( reliability.BuildPacket( 1000, false, packet, packetSize ) ) {}
        UInt32 unacked = reliability.GetUnackedCount();
        REQUIRE_TRUE( 5 == unacked );
        CORE::CDynamicBuffer forged;
        CCollectingSink sink;
        UInt32 refused = 0;
        ForgePlainPacket( forged, 1, reliability.GetNextSequence() - 1, DRN::DRN_LANE_UNRELIABLE, 0, DRN::DRN_MSGCLASS_REPL, DRN::DRN_REPL_GRID_DELTA, payload );
        ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_ACCEPTED == reliability.ProcessPlainPacket( static_cast< const UInt8* >( forged.GetConstBufferPtr() ), forged.GetDataSize(), sink, refused ) );
        ASSERT_TRUE( unacked == reliability.GetUnackedCount() && 0 == refused && 1 == sink.messages.size() );

        // a lane 0 message in a PLAIN packet is refused, the genuine one with the same lane sequence still arrives
        ForgePlainPacket( forged, 2, 0, DRN::DRN_LANE_RELIABLE_ORDERED, 0, DRN::DRN_MSGCLASS_APP, TestOpcode, payload );
        reliability.ProcessPlainPacket( static_cast< const UInt8* >( forged.GetConstBufferPtr() ), forged.GetDataSize(), sink, refused );
        ASSERT_TRUE( 1 == refused && 1 == sink.messages.size() );

        // end to end over udp: forged PLAIN packets with the sequences of genuine packets and lane 0 messages
        CLinkedPair pair;
        REQUIRE_TRUE( SetupPublicPair( pair, "udp", CORE::CString::Empty, DRN::DRN_PUBLICPROTECTION_NONE ) );
        CKindRecorder received;
        received.SubscribeTo( pair.linkB.GetPointerAlways() );
        DRN::CIDRNChannelPtr channel = pair.linkA->GetChannel();
        REQUIRE_TRUE( !channel.IsNULL() );
        const UInt32 count = 100;
        for ( UInt32 i=0; i<count; ++i )
        {
            // the forgery goes first, with the sequence and the lane sequence the genuine packet will have
            UInt32 nextSequence = pair.linkA->GetNextSequence();
            MakeCounted( payload, 100000 + i, 16 );
            ForgePlainPacket( forged, nextSequence, nextSequence, DRN::DRN_LANE_RELIABLE_ORDERED, i, DRN::DRN_MSGCLASS_APP, TestOpcode, payload );
            channel->SendPacket( forged.GetConstBufferPtr(), forged.GetDataSize() );
            MakeCounted( payload, i, 16 );
            ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_REPL, TestOpcode, payload ) );
            pair.linkA->Flush();
            if ( 0 == i % 10 )
                PumpFor( 1 );
        }
        UInt64 start = NowMs();
        while ( received.Count( DRN::DRN_MSGCLASS_REPL, DRN::DRN_PACKETKIND_AEAD ) < count && NowMs() - start < 10000 )
            PumpFor( 5 );
        ASSERT_TRUE( count == received.Count( DRN::DRN_MSGCLASS_REPL, DRN::DRN_PACKETKIND_AEAD ) );
        ASSERT_TRUE( 0 == received.Count( DRN::DRN_MSGCLASS_APP, DRN::DRN_PACKETKIND_PLAIN ) );
        bool inOrder = true;
        UInt32 expected = 0;
        for ( UInt32 i=0; i<received.entries.size(); ++i )
        {
            if ( DRN::DRN_MSGCLASS_REPL != received.entries[ i ].msgClass )
                continue;
            if ( received.entries[ i ].counter != expected )
                inOrder = false;
            ++expected;
        }
        ASSERT_TRUE( inOrder );
        ASSERT_TRUE( pair.linkB->GetProtectionDowngradeCount() > 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Protection: the policy: without allowPublicItems MAC and PLAIN are refused and PUBLIC items are sealed; the hash rule of versions 2 and 3; weaker and honoured" )
    try
    {
        // the hash of a policy without public items is that of version 2, pinned in the security suite
        DRN::CDRNSecurityPolicy defaults;
        CORE::CString v2Hash = defaults.GetPolicyHashString();
        DRN::CDRNSecurityPolicy noPublic = defaults;
        noPublic.publicItemProtection = DRN::DRN_PUBLICPROTECTION_NONE;     // irrelevant without allowPublicItems
        ASSERT_TRUE( noPublic.GetPolicyHashString() == v2Hash );
        DRN::CDRNSecurityPolicy withPublic = defaults;
        withPublic.allowPublicItems = true;
        ASSERT_TRUE( withPublic.GetPolicyHashString() != v2Hash );
        // the version 3 encoding is the version 2 one with the version byte 3 and the bytes 1 (allowPublicItems), 0 (mac)
        // appended; its SHA-256 was computed from those bytes outside DRN
        CORE::CDynamicBuffer v2Encoding;
        CORE::CDynamicBuffer v3Encoding;
        defaults.GetCanonicalEncoding( v2Encoding );
        withPublic.GetCanonicalEncoding( v3Encoding );
        ASSERT_TRUE( v3Encoding.GetDataSize() == v2Encoding.GetDataSize() + 2 );
        ASSERT_TRUE( withPublic.GetPolicyHashString() == PublicMacPolicyHash );
        DRN::CDRNSecurityPolicy withNone = withPublic;
        withNone.publicItemProtection = DRN::DRN_PUBLICPROTECTION_NONE;
        ASSERT_TRUE( withNone.GetPolicyHashString() != withPublic.GetPolicyHashString() );

        // config round trip, only when not the default
        CORE::CDataNode node( "DRNSecurityPolicy" );
        ASSERT_TRUE( defaults.SaveConfig( node ) );
        ASSERT_TRUE( GUCEF_NULL == node.GetAttribute( "allowPublicItems" ) && GUCEF_NULL == node.GetAttribute( "publicItemProtection" ) );
        CORE::CDataNode publicNode( "DRNSecurityPolicy" );
        ASSERT_TRUE( withNone.SaveConfig( publicNode ) );
        DRN::CDRNSecurityPolicy loaded;
        ASSERT_TRUE( loaded.LoadConfig( publicNode ) && loaded == withNone );

        // weaker and honoured
        CORE::CString why;
        ASSERT_TRUE( withPublic.IsWeakerThan( defaults, why ) );
        ASSERT_FALSE( defaults.IsWeakerThan( withPublic, why ) );
        ASSERT_TRUE( withNone.IsWeakerThan( withPublic, why ) );
        ASSERT_FALSE( withPublic.IsWeakerThan( withNone, why ) );
        ASSERT_TRUE( withPublic.CanBeHonoured( why ) == defaults.CanBeHonoured( why ) );
        ASSERT_TRUE( DRN::DRN_PACKETKIND_AEAD == defaults.ResolvePacketKind( DRN::DRN_PROTECTION_PUBLIC, DRN::DRN_LANE_UNRELIABLE ) );
        ASSERT_TRUE( DRN::DRN_PACKETKIND_MAC == withPublic.ResolvePacketKind( DRN::DRN_PROTECTION_PUBLIC, DRN::DRN_LANE_UNRELIABLE ) );
        ASSERT_TRUE( DRN::DRN_PACKETKIND_PLAIN == withNone.ResolvePacketKind( DRN::DRN_PROTECTION_PUBLIC, DRN::DRN_LANE_UNRELIABLE ) );
        ASSERT_TRUE( DRN::DRN_PACKETKIND_MAC == withNone.ResolvePacketKind( DRN::DRN_PROTECTION_PUBLIC, DRN::DRN_LANE_RELIABLE_ORDERED ) );
        ASSERT_TRUE( DRN::DRN_PACKETKIND_AEAD == withNone.ResolvePacketKind( DRN::DRN_PROTECTION_CONFIDENTIAL, DRN::DRN_LANE_UNRELIABLE ) );

        // without allowPublicItems: a PUBLIC item is sealed, MAC packets are refused at the link
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "sealed", "udp" );
        REQUIRE_TRUE( group.Start( 2 ) );
        DRN::CDRNDataGridSettings settings;
        settings.name = "wanted-public";
        settings.authority = DRN::DRN_AUTHORITY_OWNER;
        settings.protection = DRN::DRN_PROTECTION_PUBLIC;
        DRN::CDRNDataGrid* grid = group[ 1 ].CreateGrid( settings );
        REQUIRE_TRUE( GUCEF_NULL != grid );
        ASSERT_TRUE( DRN::DRN_PROTECTION_CONFIDENTIAL == grid->GetProtectionClass() );
        DRN::CDRNPeerLinkPtr clientLink = group[ 1 ].GetLinkToMember( group[ 0 ].GetLocalPeerId() );
        DRN::CDRNPeerLinkPtr serverLink = group[ 0 ].GetLinkToMember( group[ 1 ].GetLocalPeerId() );
        REQUIRE_TRUE( !clientLink.IsNULL() && !serverLink.IsNULL() );
        ASSERT_TRUE( DRN::DRN_PACKETKIND_AEAD == clientLink->ResolvePacketKind( DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_REPL, DRN::DRN_PACKETKIND_MAC ) );
        CORE::CDynamicBuffer payload;
        MakeCounted( payload, 1, 16 );
        ASSERT_TRUE( DRN::CDRNPeerLinkTestAccess::SendAsPacketKind( *clientLink, DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_REPL, TestOpcode, payload.GetConstBufferPtr(), payload.GetDataSize(), DRN::DRN_PACKETKIND_MAC ) );
        ASSERT_TRUE( DRN::CDRNPeerLinkTestAccess::SendAsPacketKind( *clientLink, DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_REPL, TestOpcode, payload.GetConstBufferPtr(), payload.GetDataSize(), DRN::DRN_PACKETKIND_PLAIN ) );
        clientLink->Flush();
        UInt64 start = NowMs();
        while ( serverLink->GetDowngradedPacketCount() < 2 && NowMs() - start < 3000 )
            PumpFor( 5 );
        ASSERT_TRUE( 2 == serverLink->GetDowngradedPacketCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Protection: MAC and AEAD packets across rekeys open with the next and the previous key, late ones too" )
    try
    {
        UInt8 keyOne[ 32 ];
        UInt8 keyTwo[ 32 ];
        REQUIRE_TRUE( DRN::CDRNCrypto::RandomBytes( keyOne, 32 ) && DRN::CDRNCrypto::RandomBytes( keyTwo, 32 ) );
        DRN::CDRNPacketProtection sender;
        DRN::CDRNPacketProtection receiver;
        CORE::CString cipher = DRN::CDRNSecurityPolicy().ciphers.front();
        REQUIRE_TRUE( sender.Setup( cipher, keyOne, keyTwo, 0 ) && receiver.Setup( cipher, keyTwo, keyOne, 1 ) );
        sender.SetRekeyLimits( 5, 0 );

        // 30 packets, alternating kinds, a new key every 5; every fourth one held back and opened late
        GUCEF::vector< CORE::CDynamicBuffer > late;
        bool allOpened = true;
        UInt32 macs = 0;
        for ( UInt32 i=1; i<=30; ++i )
        {
            CORE::CDynamicBuffer plain;
            DRN::CDRNWireWriter writer( plain );
            writer.WriteUInt8( DRN::DRN_PROTOCOL_ID );
            writer.WriteVarUInt32( i );
            writer.WriteVarUInt64( i );
            writer.WriteVarUInt32( 0 );
            writer.WriteUInt32( 0 );
            writer.WriteVarUInt64( 0 );
            writer.WriteVarUInt64( 0 );
            writer.WriteString( "body" );
            UInt8 kind = 0 == i % 2 ? (UInt8) DRN::DRN_PACKETKIND_MAC : (UInt8) DRN::DRN_PACKETKIND_AEAD;
            CORE::CDynamicBuffer sealed;
            REQUIRE_TRUE( sender.Protect( static_cast< const UInt8* >( plain.GetConstBufferPtr() ), plain.GetDataSize(), sealed, kind ) );
            if ( 0 == i % 4 )
            {
                late.push_back( sealed );
                continue;
            }
            CORE::CDynamicBuffer opened;
            UInt8 openedKind = 0;
            allOpened = receiver.Unprotect( static_cast< const UInt8* >( sealed.GetConstBufferPtr() ), sealed.GetDataSize(), opened, openedKind ) && openedKind == kind && allOpened;
            if ( DRN::DRN_PACKETKIND_MAC == kind )
                ++macs;
        }
        ASSERT_TRUE( allOpened && macs > 0 );
        ASSERT_TRUE( sender.GetSendRekeyCount() >= 5 && receiver.GetReceiveRekeyCount() >= 4 );

        // the late ones of the phase before the current one still open; older ones may not (two keys are kept)
        UInt32 lateOpened = 0;
        for ( UInt32 i=0; i<late.size(); ++i )
        {
            CORE::CDynamicBuffer opened;
            UInt8 openedKind = 0;
            if ( receiver.Unprotect( static_cast< const UInt8* >( late[ i ].GetConstBufferPtr() ), late[ i ].GetDataSize(), opened, openedKind ) )
                ++lateOpened;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Protection rekey: " << sender.GetSendRekeyCount() << " rekeys, " << lateOpened << " of " << late.size() << " late packets opened" );
        ASSERT_TRUE( lateOpened >= 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Protection: lane 0 keeps the order of sending across packet kinds, on stream and on lossy datagram channels; each message keeps its kind" )
    try
    {
        const char* channels[] = { "tcp", "lossy-udp" };
        const char* params[] = { "", "?loss=0.2&reorder=0.2" };
        for ( UInt32 c=0; c<2; ++c )
        {
            CLinkedPair pair;
            REQUIRE_TRUE( SetupPublicPair( pair, channels[ c ], params[ c ], DRN::DRN_PUBLICPROTECTION_MAC ) );
            CKindRecorder received;
            received.SubscribeTo( pair.linkB.GetPointerAlways() );
            const UInt32 count = 300;
            UInt64 packetsBefore = pair.linkA->GetStats().packetsSent;
            CORE::CDynamicBuffer payload;
            for ( UInt32 i=0; i<count; ++i )
            {
                MakeCounted( payload, i, 24 );
                UInt8 kind = 0 == ( i / 3 ) % 2 ? (UInt8) DRN::DRN_PACKETKIND_MAC : (UInt8) DRN::DRN_PACKETKIND_AEAD;
                ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_REPL, TestOpcode, payload, kind ) );
                if ( 0 == i % 30 )
                {
                    pair.linkA->Flush();
                    PumpFor( 1 );
                }
            }
            pair.linkA->Flush();
            UInt64 start = NowMs();
            while ( received.entries.size() < count && NowMs() - start < 20000 )
                PumpFor( 5 );
            REQUIRE_TRUE( count == received.entries.size() );
            bool inOrder = true;
            bool kindsKept = true;
            for ( UInt32 i=0; i<count; ++i )
            {
                if ( received.entries[ i ].counter != i )
                    inOrder = false;
                UInt8 kind = 0 == ( i / 3 ) % 2 ? (UInt8) DRN::DRN_PACKETKIND_MAC : (UInt8) DRN::DRN_PACKETKIND_AEAD;
                if ( received.entries[ i ].packetKind != kind )
                    kindsKept = false;
            }
            ASSERT_TRUE( inOrder && kindsKept );
            UInt64 interleavedPackets = pair.linkA->GetStats().packetsSent - packetsBefore;

            // the same messages all sealed: what the runs of equal kind cost in packets
            packetsBefore = pair.linkA->GetStats().packetsSent;
            UInt64 bytesBefore = pair.linkA->GetStats().bytesSent;
            for ( UInt32 i=0; i<count; ++i )
            {
                MakeCounted( payload, count + i, 24 );
                ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_REPL, TestOpcode, payload ) );
                if ( 0 == i % 30 )
                {
                    pair.linkA->Flush();
                    PumpFor( 1 );
                }
            }
            pair.linkA->Flush();
            start = NowMs();
            while ( received.entries.size() < 2 * count && NowMs() - start < 20000 )
                PumpFor( 5 );
            ASSERT_TRUE( 2 * count == received.entries.size() );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Protection: " << count << " lane 0 messages over " << channels[ c ] << ": " << interleavedPackets << " packets with kinds interleaved per 3 messages, " <<
                       pair.linkA->GetStats().packetsSent - packetsBefore << " packets all sealed (" << pair.linkA->GetStats().bytesSent - bytesBefore << " bytes)" );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Protection: a client's PUBLIC stream is served by the server's replica to a third member in MAC packets" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "public-relay", "udp" );
        group.nodeSettings = PublicNodeSettings( DRN::DRN_PUBLICPROTECTION_MAC );
        REQUIRE_TRUE( group.Start( 3 ) );
        DRN::CDRNDataStreamSettings settings;
        settings.name = "ticker";
        settings.protection = DRN::DRN_PROTECTION_PUBLIC;
        DRN::CDRNDataStream* ticker = group[ 1 ].CreateStream( settings );
        REQUIRE_TRUE( GUCEF_NULL != ticker && DRN::DRN_PROTECTION_PUBLIC == ticker->GetProtectionClass() );
        DRN::CDRNPeerLinkPtr thirdLink = group[ 2 ].GetLinkToMember( group[ 0 ].GetLocalPeerId() );
        REQUIRE_TRUE( !thirdLink.IsNULL() );
        CKindRecorder received;
        received.SubscribeTo( thirdLink.GetPointerAlways() );
        group[ 2 ].Subscribe( "ticker" );
        UInt64 start = NowMs();
        while ( GUCEF_NULL == group[ 2 ].FindStream( "ticker" ) && NowMs() - start < 10000 )
            PumpFor( 5 );
        DRN::CDRNDataStream* replica = group[ 2 ].FindStream( "ticker" );
        REQUIRE_TRUE( GUCEF_NULL != replica );
        PumpFor( 100 );
        for ( UInt32 i=0; i<20; ++i )
        {
            CORE::CVariantStream& entry = ticker->GetWriteStream();
            entry << i;
            entry.WriteSegmentEnd();
            ASSERT_TRUE( ticker->CommitEntry() );
        }
        start = NowMs();
        while ( replica->GetEntryCount() < 20 && NowMs() - start < 10000 )
            PumpFor( 5 );
        ASSERT_TRUE( 20 == replica->GetEntryCount() );
        DRN::CDRNDataStream* serving = group[ 0 ].FindStream( "ticker" );
        ASSERT_TRUE( GUCEF_NULL != serving && DRN::DRN_PROTECTION_PUBLIC == serving->GetProtectionClass() && DRN::DRN_PROTECTION_PUBLIC == replica->GetProtectionClass() );
        ASSERT_TRUE( received.Count( DRN::DRN_MSGCLASS_STREAM, DRN::DRN_PACKETKIND_MAC ) > 0 );
        ASSERT_TRUE( 0 == received.Count( DRN::DRN_MSGCLASS_STREAM, DRN::DRN_PACKETKIND_AEAD ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Protection: forged PLAIN packets neither change nor freeze a reliable PUBLIC grid; deltas, key definitions and snapshot boundaries on a lane or in a packet kind the grid does not use are refused, in MAC packets too (none)" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "forged-grid", "udp" );
        group.nodeSettings = PublicNodeSettings( DRN::DRN_PUBLICPROTECTION_NONE );
        REQUIRE_TRUE( group.Start( 2 ) );
        DRN::CDRNDataGridSettings settings;
        settings.name = "quotes";
        settings.authority = DRN::DRN_AUTHORITY_OWNER;
        settings.reliable = true;
        settings.protection = DRN::DRN_PROTECTION_PUBLIC;
        DRN::CDRNDataGrid* quotes = group[ 1 ].CreateGrid( settings );
        REQUIRE_TRUE( GUCEF_NULL != quotes );
        quotes->SetValue( CORE::CVariant( (UInt32) 0 ), CORE::CVariant( "genuine-0" ) );
        group[ 0 ].Subscribe( "quotes" );
        CGridCondition arrived;
        arrived.Expect( group[ 0 ], "quotes", group[ 1 ].GetLocalPeerId(), CORE::CVariant( (UInt32) 0 ), CORE::CVariant( "genuine-0" ) );
        REQUIRE_TRUE( PumpUntil( arrived, 10000 ) );
        DRN::CDRNDataGrid* replica = group[ 0 ].FindGrid( "quotes" );
        REQUIRE_TRUE( GUCEF_NULL != replica );
        const UInt32 keysBefore = replica->GetKeyCount();

        DRN::CDRNPeerLinkPtr clientLink = group[ 1 ].GetLinkToMember( group[ 0 ].GetLocalPeerId() );
        DRN::CDRNPeerLinkPtr serverLink = group[ 0 ].GetLinkToMember( group[ 1 ].GetLocalPeerId() );
        REQUIRE_TRUE( !clientLink.IsNULL() && !serverLink.IsNULL() );
        DRN::CIDRNChannelPtr channel = clientLink->GetChannel();
        REQUIRE_TRUE( !channel.IsNULL() );
        UInt64 downgradesBefore = serverLink->GetProtectionDowngradeCount();

        // keyless: the item reference is all it takes, every MAC or PLAIN packet of the item shows it
        const UInt32 owner = quotes->GetOwnerPeerId();
        const UInt32 item = quotes->GetItemId();
        CORE::CDynamicBuffer payload;
        MakeForgedDelta( payload, owner, item, 0x40000000u, 0x08, 0, CORE::CVariant( "FORGED" ) );
        SendForgedPlain( channel, 500, DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_REPL, DRN::DRN_REPL_GRID_DELTA, payload );
        MakeForgedDelta( payload, owner, item, 0xC0000000u, 0x08, 0, CORE::CVariant( "FORGED" ) );
        SendForgedPlain( channel, 501, DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_REPL, DRN::DRN_REPL_GRID_DELTA, payload );
        MakeForgedDelta( payload, owner, item, 0x40000001u, 0x01 | 0x04, 0, CORE::CVariant( "FORGED" ) );
        SendForgedPlain( channel, 502, DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_REPL, DRN::DRN_REPL_GRID_DELTA, payload );
        {
            payload.SetDataSize( 0 );
            DRN::CDRNWireWriter writer( payload );
            writer.WriteVarUInt32( owner );
            writer.WriteVarUInt32( item );
            writer.WriteUInt16( 1 );
            writer.WriteVarUInt32( 7 );
            writer.WriteUInt8( 0 );
            writer.WriteVarInt32( -1 );
            writer.WriteVarUInt32( 0 );
            DRN::CDRNVariantCodec::Write( writer, CORE::CVariant( "forged-key" ) );
        }
        SendForgedPlain( channel, 503, DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_REPL, DRN::DRN_REPL_KEY_DEFINE, payload );
        {
            payload.SetDataSize( 0 );
            DRN::CDRNWireWriter writer( payload );
            writer.WriteVarUInt32( owner );
            writer.WriteVarUInt32( item );
            writer.WriteVarUInt32( 0x40000000u );
        }
        SendForgedPlain( channel, 504, DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_REPL, DRN::DRN_REPL_SNAPSHOT_BEGIN, payload );

        // in a MAC packet, which only a holder of the link's keys makes: a reliable grid sends no delta on lane 3
        MakeForgedDelta( payload, owner, item, 0x40000002u, 0x08, 0, CORE::CVariant( "MACFORGED" ) );
        REQUIRE_TRUE( DRN::CDRNPeerLinkTestAccess::SendAsPacketKind( *clientLink, DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_REPL, DRN::DRN_REPL_GRID_DELTA, payload.GetConstBufferPtr(), payload.GetDataSize(), DRN::DRN_PACKETKIND_MAC ) );
        clientLink->Flush();

        ASSERT_TRUE( WaitForDowngrades( serverLink, downgradesBefore, 6, 5000 ) );
        CORE::CVariant value;
        ASSERT_TRUE( replica->GetValue( CORE::CVariant( (UInt32) 0 ), value ) && value.AsString() == "genuine-0" );
        ASSERT_TRUE( keysBefore == replica->GetKeyCount() );
        ASSERT_TRUE( serverLink->IsOperational() );

        // the genuine updates that follow arrive
        for ( UInt32 n=1; n<=5; ++n )
        {
            quotes->SetValue( CORE::CVariant( (UInt32) 0 ), CORE::CVariant( CORE::CString( "genuine-" ) + CORE::ToString( n ) ) );
            PumpFor( 20 );
        }
        CGridCondition updated;
        updated.Expect( group[ 0 ], "quotes", group[ 1 ].GetLocalPeerId(), CORE::CVariant( (UInt32) 0 ), CORE::CVariant( "genuine-5" ) );
        ASSERT_TRUE( PumpUntil( updated, 5000 ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Protection: forged grid data refused " << serverLink->GetProtectionDowngradeCount() - downgradesBefore << " times" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Protection: forged ENTRIES with a far sequence in a PLAIN packet, or on lane 3 of a MAC packet, do not move a PUBLIC stream; the entries that follow arrive without a gap (none)" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "forged-stream", "udp" );
        group.nodeSettings = PublicNodeSettings( DRN::DRN_PUBLICPROTECTION_NONE );
        REQUIRE_TRUE( group.Start( 2 ) );
        DRN::CDRNDataStreamSettings settings;
        settings.name = "ticks";
        settings.protection = DRN::DRN_PROTECTION_PUBLIC;
        DRN::CDRNDataStream* ticks = group[ 1 ].CreateStream( settings );
        REQUIRE_TRUE( GUCEF_NULL != ticks );
        group[ 0 ].Subscribe( "ticks" );
        CORE::CDynamicBuffer entry;
        entry.CopyFrom( 4, "abcd" );
        UInt32 appended = 0;
        UInt64 start = NowMs();
        while ( appended < 3 && NowMs() - start < 5000 )
        {
            if ( ticks->AppendEntry( entry.GetConstBufferPtr(), entry.GetDataSize() ) )
                ++appended;
            PumpFor( 20 );
        }
        REQUIRE_TRUE( 3 == appended );
        DRN::CDRNDataStream* replica = GUCEF_NULL;
        start = NowMs();
        while ( ( GUCEF_NULL == replica || replica->GetNextExpectedSeq() != ticks->GetNextSeq() ) && NowMs() - start < 5000 )
        {
            PumpFor( 5 );
            replica = group[ 0 ].FindStream( "ticks" );
        }
        REQUIRE_TRUE( GUCEF_NULL != replica && replica->GetNextExpectedSeq() == ticks->GetNextSeq() );
        const UInt64 expectedBefore = replica->GetNextExpectedSeq();

        DRN::CDRNPeerLinkPtr clientLink = group[ 1 ].GetLinkToMember( group[ 0 ].GetLocalPeerId() );
        DRN::CDRNPeerLinkPtr serverLink = group[ 0 ].GetLinkToMember( group[ 1 ].GetLocalPeerId() );
        REQUIRE_TRUE( !clientLink.IsNULL() && !serverLink.IsNULL() );
        UInt64 downgradesBefore = serverLink->GetProtectionDowngradeCount();
        CORE::CDynamicBuffer payload;
        {
            DRN::CDRNWireWriter writer( payload );
            writer.WriteVarUInt32( ticks->GetOwnerPeerId() );
            writer.WriteVarUInt32( ticks->GetItemId() );
            writer.WriteVarUInt64( 1000000 );
            writer.WriteUInt16( 1 );
            writer.WriteSizedBytes( "evil", 4 );
        }
        SendForgedPlain( clientLink->GetChannel(), 777, DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_STREAM, DRN::DRN_STREAM_ENTRIES, payload );
        REQUIRE_TRUE( DRN::CDRNPeerLinkTestAccess::SendAsPacketKind( *clientLink, DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_STREAM, DRN::DRN_STREAM_ENTRIES, payload.GetConstBufferPtr(), payload.GetDataSize(), DRN::DRN_PACKETKIND_MAC ) );
        clientLink->Flush();
        ASSERT_TRUE( WaitForDowngrades( serverLink, downgradesBefore, 2, 5000 ) );
        ASSERT_TRUE( expectedBefore == replica->GetNextExpectedSeq() );

        for ( UInt32 n=0; n<3; ++n )
        {
            ticks->AppendEntry( entry.GetConstBufferPtr(), entry.GetDataSize() );
            PumpFor( 20 );
        }
        start = NowMs();
        while ( replica->GetNextExpectedSeq() != ticks->GetNextSeq() && NowMs() - start < 5000 )
            PumpFor( 5 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Protection: forged stream entries, next expected " << replica->GetNextExpectedSeq() << ", gaps " << replica->GetGapCount() << ", publisher next " << ticks->GetNextSeq() );
        ASSERT_TRUE( replica->GetNextExpectedSeq() == ticks->GetNextSeq() && replica->GetNextExpectedSeq() < 1000000 );
        ASSERT_TRUE( 0 == replica->GetGapCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Protection: an unreliable PUBLIC grid takes its lane 3 deltas in PLAIN packets; a PLAIN delta claiming to be reliable is refused and none reaches a reliable key (none)" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "plain-grid", "udp" );
        group.nodeSettings = PublicNodeSettings( DRN::DRN_PUBLICPROTECTION_NONE );
        REQUIRE_TRUE( group.Start( 2 ) );
        DRN::CDRNDataGridSettings settings;
        settings.name = "spot";
        settings.authority = DRN::DRN_AUTHORITY_OWNER;
        settings.reliable = false;
        settings.protection = DRN::DRN_PROTECTION_PUBLIC;
        settings.reliableKeys.push_back( CORE::CVariant( (UInt32) 1 ) );
        DRN::CDRNDataGrid* spot = group[ 1 ].CreateGrid( settings );
        REQUIRE_TRUE( GUCEF_NULL != spot );
        DRN::CDRNPeerLinkPtr clientLink = group[ 1 ].GetLinkToMember( group[ 0 ].GetLocalPeerId() );
        DRN::CDRNPeerLinkPtr serverLink = group[ 0 ].GetLinkToMember( group[ 1 ].GetLocalPeerId() );
        REQUIRE_TRUE( !clientLink.IsNULL() && !serverLink.IsNULL() );
        CKindRecorder received;
        received.SubscribeTo( serverLink.GetPointerAlways() );
        group[ 0 ].Subscribe( "spot" );
        PumpFor( 200 );
        for ( UInt32 n=0; n<10; ++n )
        {
            spot->SetValue( CORE::CVariant( (UInt32) 0 ), CORE::CVariant( CORE::CString( "spot-" ) + CORE::ToString( n ) ) );
            PumpFor( 20 );
        }
        spot->SetValue( CORE::CVariant( (UInt32) 1 ), CORE::CVariant( "reliable-0" ) );
        CGridCondition arrived;
        arrived.Expect( group[ 0 ], "spot", group[ 1 ].GetLocalPeerId(), CORE::CVariant( (UInt32) 0 ), CORE::CVariant( "spot-9" ) );
        arrived.Expect( group[ 0 ], "spot", group[ 1 ].GetLocalPeerId(), CORE::CVariant( (UInt32) 1 ), CORE::CVariant( "reliable-0" ) );
        REQUIRE_TRUE( PumpUntil( arrived, 10000 ) );
        ASSERT_TRUE( received.Count( DRN::DRN_MSGCLASS_REPL, DRN::DRN_PACKETKIND_PLAIN ) > 0 );
        DRN::CDRNDataGrid* replica = group[ 0 ].FindGrid( "spot" );
        REQUIRE_TRUE( GUCEF_NULL != replica );
        UInt32 reliableIndex = replica->GetKeyIndex( CORE::CVariant( (UInt32) 1 ) );
        UInt32 unreliableIndex = replica->GetKeyIndex( CORE::CVariant( (UInt32) 0 ) );
        REQUIRE_TRUE( DRN::CDRNDataGrid::InvalidKeyIndex != reliableIndex && DRN::CDRNDataGrid::InvalidKeyIndex != unreliableIndex );

        // a PLAIN delta with the reliable flag, and an unreliable one for the reliable key
        UInt64 downgradesBefore = serverLink->GetProtectionDowngradeCount();
        CORE::CDynamicBuffer payload;
        MakeForgedDelta( payload, spot->GetOwnerPeerId(), spot->GetItemId(), 0x40000000u, 0x01 | 0x04, unreliableIndex, CORE::CVariant( "FORGED" ) );
        SendForgedPlain( clientLink->GetChannel(), 900, DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_REPL, DRN::DRN_REPL_GRID_DELTA, payload );
        ASSERT_TRUE( WaitForDowngrades( serverLink, downgradesBefore, 1, 5000 ) );
        MakeForgedDelta( payload, spot->GetOwnerPeerId(), spot->GetItemId(), 0x40000001u, 0x08, reliableIndex, CORE::CVariant( "FORGED" ) );
        SendForgedPlain( clientLink->GetChannel(), 901, DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_REPL, DRN::DRN_REPL_GRID_DELTA, payload );
        PumpFor( 200 );
        CORE::CVariant value;
        ASSERT_TRUE( replica->GetValue( CORE::CVariant( (UInt32) 1 ), value ) && value.AsString() == "reliable-0" );

        // the reliable key keeps following its publisher
        spot->SetValue( CORE::CVariant( (UInt32) 1 ), CORE::CVariant( "reliable-1" ) );
        CGridCondition updated;
        updated.Expect( group[ 0 ], "spot", group[ 1 ].GetLocalPeerId(), CORE::CVariant( (UInt32) 1 ), CORE::CVariant( "reliable-1" ) );
        ASSERT_TRUE( PumpUntil( updated, 5000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Protection: lane 0 messages held back keep their packet kind when the hold back ring grows at the end of the handshake" )
    try
    {
        DRN::CDRNLinkReliability sender;
        DRN::CDRNLinkReliability receiver;
        sender.Reset( false, 1200, 1 );
        receiver.Reset( false, 1200, 1 );
        receiver.SetPreAuthRestricted( true );
        CORE::CDynamicBuffer payload;
        GUCEF::vector< CORE::CDynamicBuffer > packets;
        for ( UInt32 i=0; i<3; ++i )
        {
            MakeCounted( payload, i, 16 );
            REQUIRE_TRUE( sender.QueueMessage( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_LINK, DRN::DRN_LINK_ECHO, payload.GetConstBufferPtr(), payload.GetDataSize(), GUCEF_NULL, 0 ) );
            const UInt8* packet = GUCEF_NULL;
            UInt32 packetSize = 0;
            REQUIRE_TRUE( sender.BuildPacket( 1000 + i, false, packet, packetSize ) );
            packets.push_back( CORE::CDynamicBuffer() );
            packets.back().CopyFrom( 0, packetSize, packet );
        }

        // the second and the third arrive first, in MAC packets: held back for the first
        CCollectingSink sink;
        DRN::CDRNPacketHeader header;
        UInt32 lostBefore = 0;
        for ( UInt32 i=1; i<3; ++i )
            REQUIRE_TRUE( DRN::CDRNLinkReliability::PACKET_ACCEPTED == receiver.ProcessPacket( static_cast< const UInt8* >( packets[ i ].GetConstBufferPtr() ), packets[ i ].GetDataSize(), 2000 + i, 10000, sink, header, lostBefore, DRN::DRN_PACKETKIND_MAC ) );
        REQUIRE_TRUE( sink.messages.empty() );

        // the handshake ends: the ring takes the size of the operational link, then the first arrives sealed
        receiver.SetPreAuthRestricted( false );
        REQUIRE_TRUE( DRN::CDRNLinkReliability::PACKET_ACCEPTED == receiver.ProcessPacket( static_cast< const UInt8* >( packets[ 0 ].GetConstBufferPtr() ), packets[ 0 ].GetDataSize(), 2010, 10000, sink, header, lostBefore, DRN::DRN_PACKETKIND_AEAD ) );
        REQUIRE_TRUE( 3 == sink.messages.size() );
        ASSERT_TRUE( DRN::DRN_PACKETKIND_AEAD == sink.messages[ 0 ].packetKind );
        ASSERT_TRUE( DRN::DRN_PACKETKIND_MAC == sink.messages[ 1 ].packetKind && DRN::DRN_PACKETKIND_MAC == sink.messages[ 2 ].packetKind );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Protection: forged PLAIN packets do not keep a link alive whose peer went quiet (none)" )
    try
    {
        // a peer that sends nothing by itself: no echoes; the other end times out after 400 ms of silence
        DRN::CDRNNodeSettings settingsA = PublicNodeSettings( DRN::DRN_PUBLICPROTECTION_NONE );
        settingsA.echoIntervalMs = 60000;
        settingsA.linkTimeoutMs = 60000;
        DRN::CDRNNodeSettings settingsB = settingsA;
        settingsB.nodeId = DRN::CDRNNodeSettings::GenerateNodeId();
        settingsB.linkTimeoutMs = 400;
        UInt16 port = NextTestPort();
        CLinkedPair pair;
        REQUIRE_TRUE( pair.Setup( settingsA, settingsB, LoopbackEndpoint( "udp", port ), LoopbackEndpoint( "udp", port ) ) );
        DRN::CIDRNChannelPtr channel = pair.linkA->GetChannel();
        REQUIRE_TRUE( !channel.IsNULL() );
        CORE::CDynamicBuffer payload;
        MakeCounted( payload, 1, 16 );
        UInt64 start = NowMs();
        UInt32 sequence = 1;
        while ( 0 == pair.recorderB.closedCount && NowMs() - start < 3000 )
        {
            SendForgedPlain( channel, sequence++, DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_REPL, TestOpcode, payload );
            PumpFor( 20 );
        }
        UInt64 took = NowMs() - start;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Protection: a quiet peer with forged PLAIN packets: link closed after " << took << " ms, " << sequence - 1 << " forged packets" );
        ASSERT_TRUE( 1 == pair.recorderB.closedCount && DRN::DRN_LINKREASON_LINK_TIMEOUT == pair.recorderB.lastClosedReason );
        ASSERT_TRUE( took < 2000 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Protection: a forged PLAIN sequence far ahead, or a series of jumps, does not shut out the genuine PLAIN packets" )
    try
    {
        DRN::CDRNLinkReliability reliability;
        reliability.Reset( false, 1200, 1 );
        CCollectingSink sink;
        UInt32 refused = 0;
        CORE::CDynamicBuffer payload;
        CORE::CDynamicBuffer packet;
        MakeCounted( payload, 1, 16 );
        ForgePlainPacket( packet, 1, 0, DRN::DRN_LANE_UNRELIABLE, 0, DRN::DRN_MSGCLASS_REPL, TestOpcode, payload );
        ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_ACCEPTED == reliability.ProcessPlainPacket( static_cast< const UInt8* >( packet.GetConstBufferPtr() ), packet.GetDataSize(), sink, refused ) );
        ASSERT_TRUE( 1 == sink.messages.size() );

        // far ahead: not taken
        ForgePlainPacket( packet, 1 + 0x40000000u, 0, DRN::DRN_LANE_UNRELIABLE, 0, DRN::DRN_MSGCLASS_REPL, TestOpcode, payload );
        ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_ACCEPTED != reliability.ProcessPlainPacket( static_cast< const UInt8* >( packet.GetConstBufferPtr() ), packet.GetDataSize(), sink, refused ) );
        ASSERT_TRUE( 1 == sink.messages.size() );

        // the genuine sequence goes on
        ForgePlainPacket( packet, 2, 0, DRN::DRN_LANE_UNRELIABLE, 0, DRN::DRN_MSGCLASS_REPL, TestOpcode, payload );
        ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_ACCEPTED == reliability.ProcessPlainPacket( static_cast< const UInt8* >( packet.GetConstBufferPtr() ), packet.GetDataSize(), sink, refused ) );
        ASSERT_TRUE( 2 == sink.messages.size() );

        // jumps, each within what is taken, move the window far from the genuine sequence: the genuine packets still arrive
        for ( UInt32 j=1; j<=100; ++j )
        {
            ForgePlainPacket( packet, 2 + j * 1000, 0, DRN::DRN_LANE_UNRELIABLE, 0, DRN::DRN_MSGCLASS_REPL, TestOpcode, payload );
            reliability.ProcessPlainPacket( static_cast< const UInt8* >( packet.GetConstBufferPtr() ), packet.GetDataSize(), sink, refused );
        }
        UInt32 delivered = (UInt32) sink.messages.size();
        for ( UInt32 s=3; s<6; ++s )
        {
            ForgePlainPacket( packet, s, 0, DRN::DRN_LANE_UNRELIABLE, 0, DRN::DRN_MSGCLASS_REPL, TestOpcode, payload );
            ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_ACCEPTED == reliability.ProcessPlainPacket( static_cast< const UInt8* >( packet.GetConstBufferPtr() ), packet.GetDataSize(), sink, refused ) );
        }
        ASSERT_TRUE( delivered + 3 == sink.messages.size() );

        // a duplicate within the window is still dropped
        ForgePlainPacket( packet, 2 + 100 * 1000, 0, DRN::DRN_LANE_UNRELIABLE, 0, DRN::DRN_MSGCLASS_REPL, TestOpcode, payload );
        ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_DUPLICATE == reliability.ProcessPlainPacket( static_cast< const UInt8* >( packet.GetConstBufferPtr() ), packet.GetDataSize(), sink, refused ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Protection: a flood of forged PLAIN packets takes nothing of the receive limits of the authenticated traffic (none)" )
    try
    {
        DRN::CDRNNodeSettings settings = PublicNodeSettings( DRN::DRN_PUBLICPROTECTION_NONE );
        settings.securityPolicy.limits.maxMessagesPerSecond = 200;
        settings.securityPolicy.limits.sustainedLimitSeconds = 1;
        DRN::CDRNNodeSettings settingsB = settings;
        settingsB.nodeId = DRN::CDRNNodeSettings::GenerateNodeId();
        UInt16 port = NextTestPort();
        CLinkedPair pair;
        REQUIRE_TRUE( pair.Setup( settings, settingsB, LoopbackEndpoint( "udp", port ), LoopbackEndpoint( "udp", port ) ) );
        CKindRecorder received;
        received.SubscribeTo( pair.linkB.GetPointerAlways() );
        DRN::CIDRNChannelPtr channel = pair.linkA->GetChannel();
        REQUIRE_TRUE( !channel.IsNULL() );
        const UInt64 hitsBefore = pair.linkB->GetMonitor().GetLimitHitCount();
        const UInt64 trafficBefore = pair.linkB->GetMonitor().GetTrafficAnomalyCount();
        CORE::CDynamicBuffer payload;
        MakeCounted( payload, 1, 16 );
        for ( UInt32 i=0; i<1500; ++i )
        {
            SendForgedPlain( channel, 100 + i, DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_REPL, TestOpcode, payload );
            if ( 0 == i % 50 )
                PumpFor( 1 );
        }
        PumpFor( 1500 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Protection: a flood of forged PLAIN packets: limit hits " << pair.linkB->GetMonitor().GetLimitHitCount() - hitsBefore <<
                   ", traffic anomalies " << pair.linkB->GetMonitor().GetTrafficAnomalyCount() - trafficBefore );
        ASSERT_TRUE( pair.linkB->IsOperational() );
        ASSERT_TRUE( hitsBefore == pair.linkB->GetMonitor().GetLimitHitCount() );
        ASSERT_TRUE( trafficBefore == pair.linkB->GetMonitor().GetTrafficAnomalyCount() );

        // the authenticated traffic flows
        MakeCounted( payload, 7, 16 );
        UInt32 sealedBefore = received.Count( DRN::DRN_MSGCLASS_REPL, DRN::DRN_PACKETKIND_AEAD );
        REQUIRE_TRUE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_REPL, TestOpcode, payload ) );
        pair.linkA->Flush();
        UInt64 start = NowMs();
        while ( received.Count( DRN::DRN_MSGCLASS_REPL, DRN::DRN_PACKETKIND_AEAD ) == sealedBefore && NowMs() - start < 3000 )
            PumpFor( 5 );
        ASSERT_TRUE( sealedBefore + 1 == received.Count( DRN::DRN_MSGCLASS_REPL, DRN::DRN_PACKETKIND_AEAD ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Protection: the sender resolves MAC and PLAIN only for the message classes the receiver takes in them (REPL, STREAM, RPC; PLAIN for REPL and STREAM on lane 3)" )
    try
    {
        CLinkedPair pair;
        REQUIRE_TRUE( SetupPublicPair( pair, "udp", CORE::CString::Empty, DRN::DRN_PUBLICPROTECTION_NONE ) );
        const UInt8 sealedClasses[] = { DRN::DRN_MSGCLASS_LINK, DRN::DRN_MSGCLASS_SESSION, DRN::DRN_MSGCLASS_BULK, DRN::DRN_MSGCLASS_RELAY, DRN::DRN_MSGCLASS_APP };
        for ( UInt32 i=0; i<sizeof( sealedClasses ); ++i )
        {
            ASSERT_TRUE( DRN::DRN_PACKETKIND_AEAD == pair.linkA->ResolvePacketKind( DRN::DRN_LANE_RELIABLE_ORDERED, sealedClasses[ i ], DRN::DRN_PACKETKIND_MAC ) );
            ASSERT_TRUE( DRN::DRN_PACKETKIND_AEAD == pair.linkA->ResolvePacketKind( DRN::DRN_LANE_UNRELIABLE, sealedClasses[ i ], DRN::DRN_PACKETKIND_PLAIN ) );
        }
        ASSERT_TRUE( DRN::DRN_PACKETKIND_MAC == pair.linkA->ResolvePacketKind( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_REPL, DRN::DRN_PACKETKIND_MAC ) );
        ASSERT_TRUE( DRN::DRN_PACKETKIND_MAC == pair.linkA->ResolvePacketKind( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_STREAM, DRN::DRN_PACKETKIND_MAC ) );
        ASSERT_TRUE( DRN::DRN_PACKETKIND_MAC == pair.linkA->ResolvePacketKind( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_RPC, DRN::DRN_PACKETKIND_MAC ) );
        ASSERT_TRUE( DRN::DRN_PACKETKIND_PLAIN == pair.linkA->ResolvePacketKind( DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_REPL, DRN::DRN_PACKETKIND_PLAIN ) );
        ASSERT_TRUE( DRN::DRN_PACKETKIND_PLAIN == pair.linkA->ResolvePacketKind( DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_STREAM, DRN::DRN_PACKETKIND_PLAIN ) );
        ASSERT_TRUE( DRN::DRN_PACKETKIND_MAC == pair.linkA->ResolvePacketKind( DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_RPC, DRN::DRN_PACKETKIND_PLAIN ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
