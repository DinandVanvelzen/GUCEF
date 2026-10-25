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
#include "TestDRNLossyChannel.h"
#include "TestDRNEncryption.h"

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

static const UInt32 TestOpcode = 77;

/*-------------------------------------------------------------------------*/

static DRN::CDRNNodeSettings
EncryptionNodeSettings( void )
{
    DRN::CDRNNodeSettings settings;
    settings.echoIntervalMs = 20;
    settings.handshakeTimeoutMs = 2000;
    settings.connectTimeoutMs = 2000;
    settings.pulseIntervalMs = 1;
    return settings;
}

/*-------------------------------------------------------------------------*/

/** The first cipher of the policy that has a codec: what two default nodes agree on */
static CORE::CString
ExpectedCipher( const DRN::CDRNSecurityPolicy& policy )
{
    for ( UInt32 i=0; i<policy.ciphers.size(); ++i )
        if ( DRN::CDRNCrypto::IsCipherAvailable( policy.ciphers[ i ] ) )
            return policy.ciphers[ i ];
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
TestFilePath( const CORE::CString& name )
{
    CORE::CString path = CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( path, name );
    return path;
}

/*-------------------------------------------------------------------------*/

/** Watches a connector */
class CEncryptionConnectorRecorder : public CORE::CObserver
{
    public:
    CEncryptionConnectorRecorder( void ) : connected( 0 ), failed( 0 ), failReason( DRN::DRN_LINKREASON_NONE ), failText() {}
    virtual ~CEncryptionConnectorRecorder() { SignalUpcomingObserverDestruction(); }
    UInt32 connected;
    UInt32 failed;
    DRN::TDRNLinkReason failReason;
    CORE::CString failText;
    protected:
    virtual void OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( DRN::CDRNConnector::ConnectedEvent == eventid )
            ++connected;
        else
        if ( DRN::CDRNConnector::ConnectFailedEvent == eventid )
        {
            if ( GUCEF_NULL != eventdata )
            {
                const DRN::CDRNLinkClosedInfo& info = static_cast< DRN::CDRNLinkClosedEventData* >( eventdata )->GetData();
                failReason = info.reason;
                failText = info.text;
            }
            ++failed;
        }
    }
};

/*-------------------------------------------------------------------------*/

class CDoneCondition : public CTestCondition
{
    public:
    explicit CDoneCondition( const CEncryptionConnectorRecorder& recorder ) : m_recorder( recorder ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return m_recorder.connected + m_recorder.failed > 0; }
    private:
    const CEncryptionConnectorRecorder& m_recorder;
};

/*-------------------------------------------------------------------------*/

/** Connects nodeA to nodeB and waits until the connector is done */
static bool
ConnectAndWait( DRN::CDRNNode& nodeA, const DRN::CDRNEndpointList& endpoints, CEncryptionConnectorRecorder& recorder, UInt32 timeoutMs = 10000 )
{
    DRN::CDRNConnectorPtr connector = nodeA.Connect( endpoints );
    recorder.SubscribeTo( connector.GetPointerAlways() );
    if ( connector->IsDone() )
    {
        if ( connector->HasSucceeded() )
            ++recorder.connected;
        else
            ++recorder.failed;
        return true;
    }
    CDoneCondition done( recorder );
    return PumpUntil( done, timeoutMs );
}

/*-------------------------------------------------------------------------*/

/** The app messages a recorder got */
class CMessageCountCondition : public CTestCondition
{
    public:
    CMessageCountCondition( const CLinkRecorder& recorder, UInt32 expected ) : m_recorder( recorder ), m_expected( expected ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt32 count = 0;
        for ( UInt32 i=0; i<m_recorder.messages.size(); ++i )
            if ( DRN::DRN_MSGCLASS_APP == m_recorder.messages[ i ].msgClass && TestOpcode == m_recorder.messages[ i ].opcode )
                ++count;
        return count >= m_expected;
    }
    private:
    const CLinkRecorder& m_recorder;
    UInt32 m_expected;
};

/*-------------------------------------------------------------------------*/

/** Sends count numbered messages on a lane */
static void
SendNumbered( DRN::CDRNPeerLinkPtr& link, UInt8 lane, UInt32 count )
{
    for ( UInt32 i=0; i<count; ++i )
    {
        link->Send( lane, DRN::DRN_MSGCLASS_APP, TestOpcode, &i, sizeof( i ) );
        if ( 0 == ( i % 10 ) )
            PumpFor( 1 );
    }
}

/*-------------------------------------------------------------------------*/

/** The numbered messages arrived once each and in order */
static bool
ArrivedInOrder( const CLinkRecorder& recorder, UInt32 count )
{
    UInt32 next = 0;
    for ( UInt32 i=0; i<recorder.messages.size(); ++i )
    {
        const CRecordedMessage& message = recorder.messages[ i ];
        if ( DRN::DRN_MSGCLASS_APP != message.msgClass || TestOpcode != message.opcode || sizeof( UInt32 ) != message.payload.GetDataSize() )
            continue;
        UInt32 value = 0;
        memcpy( &value, message.payload.GetConstBufferPtr(), sizeof( value ) );
        if ( value != next )
            return false;
        ++next;
    }
    return next == count;
}

/*-------------------------------------------------------------------------*/

static CDRNLossyChannel*
GetLossyChannel( const DRN::CDRNPeerLinkPtr& link )
{
    DRN::CIDRNChannelPtr channel = link->GetChannel();
    if ( channel.IsNULL() )
        return GUCEF_NULL;
    return channel->GetClassTypeName() == CDRNLossyChannel::ClassTypeName() ? static_cast< CDRNLossyChannel* >( channel.GetPointerAlways() ) : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

/** A plain packet: header, one APP message on lane 0 */
static void
MakePlainPacket( CORE::CDynamicBuffer& packet, UInt32 sequence, UInt32 bodySize )
{
    packet.SetDataSize( 0 );
    DRN::CDRNWireWriter writer( packet );
    DRN::CDRNPacketHeader header;
    header.sequence = sequence;
    header.sendTime = 1000 * (UInt64) sequence;
    header.ackSequence = sequence > 1 ? sequence - 1 : 0;
    DRN::CDRNPacketCodec::WriteHeader( writer, header );
    CORE::CDynamicBuffer payload;
    payload.SetDataSize( bodySize );
    for ( UInt32 i=0; i<bodySize; ++i )
        static_cast< UInt8* >( payload.GetBufferPtr() )[ i ] = (UInt8) ( i + sequence );
    DRN::CDRNPacketCodec::WriteMessage( writer, DRN::DRN_MSGCLASS_APP, TestOpcode, DRN::DRN_LANE_RELIABLE_ORDERED, sequence, payload.GetConstBufferPtr(), bodySize );
}

/*-------------------------------------------------------------------------*/

static bool
SameBytes( const CORE::CDynamicBuffer& a, const CORE::CDynamicBuffer& b )
{
    return a.GetDataSize() == b.GetDataSize() && 0 == memcmp( a.GetConstBufferPtr(), b.GetConstBufferPtr(), a.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

static bool
LoadCodecPlugin( const CORE::CString& name )
{
    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPath = "$MODULEDIR$/" + name + "_d";
    CORE::CString secondPath = "$MODULEDIR$/" + name;
    #else
    CORE::CString firstPath = "$MODULEDIR$/" + name;
    CORE::CString secondPath = "$MODULEDIR$/" + name + "_d";
    #endif
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( firstPath, "test", true ) )
        return true;
    return pluginControl.AddPluginFromDir( secondPath, "test", true );
}

/*-------------------------------------------------------------------------*/

/** Seals and opens a datagram of 1200 bytes with an instance of the codec, microseconds per seal and open */
static Float64
TimeAeadProvider( CORE::CCodecRegistry::TICodecPtr codec, UInt32 nonceSize, UInt32 rounds, bool& allOpened )
{
    allOpened = false;
    UInt8 key[ 32 ];
    if ( !DRN::CDRNCrypto::RandomBytes( key, sizeof( key ) ) )
        return 0;
    CORE::CValueList params;
    params.Set( "key", CORE::CVariant( key, sizeof( key ) ) );
    CORE::CICodecInstancePtr sealer;
    CORE::CICodecInstancePtr opener;
    if ( !codec->CreateInstance( params, sealer ) || !codec->CreateInstance( params, opener ) || sealer.IsNULL() || opener.IsNULL() )
        return 0;

    UInt8 nonce[ 24 ];
    memset( nonce, 0, sizeof( nonce ) );
    UInt8 aad[ 20 ];
    memset( aad, 0xD4, sizeof( aad ) );
    CORE::SCodecCallArgs args;
    memset( &args, 0, sizeof( args ) );
    args.structSize = sizeof( args );
    args.nonce = nonce;
    args.nonceSize = nonceSize;
    args.aad = aad;
    args.aadSize = sizeof( aad );
    CORE::CDynamicBuffer plain;
    plain.SetDataSize( DRN::DRN_DEFAULT_DATAGRAM_SIZE );
    memset( plain.GetBufferPtr(), 0x42, plain.GetDataSize() );
    CORE::CDynamicBuffer sealed;
    CORE::CDynamicBuffer opened;

    bool ok = true;
    UInt64 start = DRN::CDRNClock::NowInMicroseconds();
    for ( UInt32 i=0; i<rounds; ++i )
    {
        nonce[ nonceSize - 1 ] = (UInt8) i;
        nonce[ nonceSize - 2 ] = (UInt8) ( i >> 8 );
        nonce[ nonceSize - 3 ] = (UInt8) ( i >> 16 );
        sealed.SetDataSize( 0 );
        opened.SetDataSize( 0 );
        ok = sealer->Process( GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, plain.GetConstBufferPtr(), plain.GetDataSize(), &args, sealed ) &&
             opener->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, sealed.GetConstBufferPtr(), sealed.GetDataSize(), &args, opened ) &&
             opened.GetDataSize() == plain.GetDataSize() && ok;
    }
    UInt64 took = DRN::CDRNClock::NowInMicroseconds() - start;
    allOpened = ok;
    return (Float64) took / rounds;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDRNEncryptionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.Encryption" );

    if ( !DRN::CDRNCrypto::IsKeyAgreementAvailable() || !DRN::CDRNCrypto::IsSignatureAvailable() )
    {
        GUCEF_TESTFW_TESTCASE( "Encryption: key agreement and signature codecs are available" )
        ASSERT_TRUE( DRN::CDRNCrypto::IsKeyAgreementAvailable() && DRN::CDRNCrypto::IsSignatureAvailable() );
        GUCEF_TESTFW_TESTCASE_END
        return;
    }
    bool webSocketAvailable = EnsureHashCodecPluginLoaded();

    const char* channelTypes[] = { "tcp", "udp", "websocket" };
    for ( UInt32 c=0; c<3; ++c )
    {
        const CORE::CString channelType = channelTypes[ c ];
        if ( channelType == "websocket" && !webSocketAvailable )
            continue;
        const CORE::CString caseName = "Encryption: links are encrypted with the first cipher both sides have, the acceptor proves its key, over " + channelType;
        GUCEF_TESTFW_TESTCASE( caseName.C_String() )
        try
        {
            CLinkedPair pair;
            UInt16 port = NextTestPort();
            REQUIRE_TRUE( pair.Setup( EncryptionNodeSettings(), EncryptionNodeSettings(), LoopbackEndpoint( channelType, port ), LoopbackEndpoint( channelType, port ) ) );
            ASSERT_TRUE( pair.linkA->IsProtected() && pair.linkB->IsProtected() );
            CORE::CString expected = ExpectedCipher( pair.nodeA.GetSettings().securityPolicy );
            ASSERT_TRUE( !expected.IsNULLOrEmpty() && pair.linkA->GetProtection().GetCipher() == expected && pair.linkB->GetProtection().GetCipher() == expected );
            UInt8 keyB[ DRN::DRN_HANDSHAKE_KEY_SIZE ];
            REQUIRE_TRUE( pair.nodeB.GetPublicKey( keyB ) );
            REQUIRE_TRUE( GUCEF_NULL != pair.linkA->GetPeerStaticKey() );
            ASSERT_TRUE( 0 == memcmp( keyB, pair.linkA->GetPeerStaticKey(), sizeof( keyB ) ) );
            ASSERT_TRUE( 0 == memcmp( pair.linkA->GetTranscriptHash(), pair.linkB->GetTranscriptHash(), DRN::DRN_HANDSHAKE_HASH_SIZE ) );

            SendNumbered( pair.linkA, DRN::DRN_LANE_RELIABLE_ORDERED, 100 );
            CMessageCountCondition all( pair.recorderB, 100 );
            ASSERT_TRUE( PumpUntil( all, 10000 ) );
            ASSERT_TRUE( ArrivedInOrder( pair.recorderB, 100 ) );
            ASSERT_TRUE( 0 == pair.linkB->GetAuthenticationFailureCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        GUCEF_TESTFW_TESTCASE_END
    }

    GUCEF_TESTFW_TESTCASE( "Encryption: a pinned key links, another key is refused, a second handshake gives other keys" )
    try
    {
        DRN::CDRNNode nodeA;
        DRN::CDRNNode nodeB;
        REQUIRE_TRUE( nodeA.SetSettings( EncryptionNodeSettings() ) && nodeB.SetSettings( EncryptionNodeSettings() ) );
        CLinkRecorder recorderA;
        recorderA.WatchNode( nodeA );
        UInt16 port = NextTestPort();
        REQUIRE_TRUE( nodeB.Listen( LoopbackEndpoint( "udp", port ) ) );

        CEncryptionConnectorRecorder pinned;
        REQUIRE_TRUE( ConnectAndWait( nodeA, DRN::CDRNEndpointList( LoopbackEndpoint( "udp", port, "?key=" + nodeB.GetPublicKeyString() ) ), pinned ) );
        ASSERT_TRUE( 1 == pinned.connected );

        CEncryptionConnectorRecorder wrong;
        CORE::CString otherKey = "1111111111111111111111111111111111111111111111111111111111111111";
        REQUIRE_TRUE( ConnectAndWait( nodeA, DRN::CDRNEndpointList( LoopbackEndpoint( "udp", port, "?key=" + otherKey ) ), wrong ) );
        ASSERT_TRUE( 1 == wrong.failed && DRN::DRN_LINKREASON_KEY_MISMATCH == wrong.failReason );

        // the published endpoints carry the key: they pin it
        DRN::CDRNEndpointList published = nodeB.GetPublishedEndpoints();
        REQUIRE_TRUE( 1 == published.GetCount() );
        ASSERT_TRUE( published.Get( 0 ).GetParams().GetValueAlways( "key" ).AsString( CORE::CString::Empty, true ) == nodeB.GetPublicKeyString() );
        CEncryptionConnectorRecorder publishedRecorder;
        REQUIRE_TRUE( ConnectAndWait( nodeA, published, publishedRecorder ) );
        ASSERT_TRUE( 1 == publishedRecorder.connected );

        // every handshake has its own transcript and so its own keys: what one link sends no other opens
        REQUIRE_TRUE( PumpUntil( recorderA.operationalCount, 2 ) );
        ASSERT_TRUE( 0 != memcmp( recorderA.links[ 0 ]->GetTranscriptHash(), recorderA.links[ 1 ]->GetTranscriptHash(), DRN::DRN_HANDSHAKE_HASH_SIZE ) );
        recorderA.links.clear();
        nodeA.Stop();
        nodeB.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Encryption: a peer with another security policy is refused" )
    try
    {
        DRN::CDRNNodeSettings settingsB = EncryptionNodeSettings();
        settingsB.securityPolicy.limits.maxMessagesPerSecond = 1234;
        DRN::CDRNNode nodeA;
        DRN::CDRNNode nodeB;
        REQUIRE_TRUE( nodeA.SetSettings( EncryptionNodeSettings() ) && nodeB.SetSettings( settingsB ) );
        UInt16 port = NextTestPort();
        REQUIRE_TRUE( nodeB.Listen( LoopbackEndpoint( "tcp", port ) ) );
        CEncryptionConnectorRecorder recorder;
        REQUIRE_TRUE( ConnectAndWait( nodeA, DRN::CDRNEndpointList( LoopbackEndpoint( "tcp", port ) ), recorder ) );
        ASSERT_TRUE( 1 == recorder.failed && DRN::DRN_LINKREASON_POLICY == recorder.failReason );
        ASSERT_TRUE( recorder.failText == "the security policies differ" );
        nodeA.Stop();
        nodeB.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Encryption: a cipher offer changed on the way, a downgrade, fails the handshake" )
    try
    {
        DRN::CDRNNodeSettings settings = EncryptionNodeSettings();
        settings.handshakeTimeoutMs = 500;
        CLinkedPair pair;
        UInt16 port = NextTestPort();
        ASSERT_FALSE( pair.Setup( settings, settings, LoopbackEndpoint( "lossy-tcp", port ), LoopbackEndpoint( "lossy-tcp", port, "?downgrade=1" ), 3000 ) );
        ASSERT_TRUE( 0 == pair.recorderA.operationalCount && 0 == pair.recorderB.operationalCount );
        ASSERT_TRUE( PumpUntil( pair.recorderB.closedCount, 1, 3000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Encryption: changed packets are dropped on udp, the stream is closed on tcp" )
    try
    {
        CLinkedPair udp;
        UInt16 port = NextTestPort();
        REQUIRE_TRUE( udp.Setup( EncryptionNodeSettings(), EncryptionNodeSettings(), LoopbackEndpoint( "lossy-udp", port ), LoopbackEndpoint( "lossy-udp", port, "?tamper=0.2" ) ) );
        SendNumbered( udp.linkA, DRN::DRN_LANE_RELIABLE_ORDERED, 200 );
        CMessageCountCondition all( udp.recorderB, 200 );
        ASSERT_TRUE( PumpUntil( all, 15000 ) );
        ASSERT_TRUE( ArrivedInOrder( udp.recorderB, 200 ) );
        CDRNLossyChannel* lossy = GetLossyChannel( udp.linkA );
        REQUIRE_TRUE( GUCEF_NULL != lossy );
        ASSERT_TRUE( lossy->GetTamperedCount() > 0 );
        ASSERT_TRUE( udp.linkB->GetAuthenticationFailureCount() > 0 );
        ASSERT_TRUE( udp.linkB->IsOperational() );

        CLinkedPair tcp;
        UInt16 tcpPort = NextTestPort();
        bool linked = tcp.Setup( EncryptionNodeSettings(), EncryptionNodeSettings(), LoopbackEndpoint( "lossy-tcp", tcpPort ), LoopbackEndpoint( "lossy-tcp", tcpPort, "?tamper=0.2" ) );
        if ( linked )
            SendNumbered( tcp.linkA, DRN::DRN_LANE_RELIABLE_ORDERED, 200 );
        ASSERT_TRUE( PumpUntil( tcp.recorderB.closedCount, 1, 5000 ) );
        ASSERT_TRUE( DRN::DRN_LINKREASON_PROTOCOL_ERROR == tcp.recorderB.lastClosedReason );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Encryption: duplicated and replayed packets open but are delivered once" )
    try
    {
        CLinkedPair pair;
        UInt16 port = NextTestPort();
        REQUIRE_TRUE( pair.Setup( EncryptionNodeSettings(), EncryptionNodeSettings(), LoopbackEndpoint( "lossy-udp", port ), LoopbackEndpoint( "lossy-udp", port, "?dup=0.5&reorder=0.3" ) ) );
        REQUIRE_TRUE( pair.linkB->IsProtected() );
        SendNumbered( pair.linkA, DRN::DRN_LANE_RELIABLE_ORDERED, 200 );
        CMessageCountCondition all( pair.recorderB, 200 );
        ASSERT_TRUE( PumpUntil( all, 15000 ) );
        PumpFor( 200 );
        ASSERT_TRUE( ArrivedInOrder( pair.recorderB, 200 ) );
        ASSERT_TRUE( pair.linkB->GetStats().duplicatePackets > 0 );
        ASSERT_TRUE( 0 == pair.linkB->GetAuthenticationFailureCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Encryption: packet protection opens only what it sealed, rekeys, and opens late packets of the previous key" )
    try
    {
        CORE::CString cipher = ExpectedCipher( DRN::CDRNSecurityPolicy() );
        UInt8 keyOne[ 32 ];
        UInt8 keyTwo[ 32 ];
        UInt8 keyThree[ 32 ];
        REQUIRE_TRUE( DRN::CDRNCrypto::RandomBytes( keyOne, 32 ) && DRN::CDRNCrypto::RandomBytes( keyTwo, 32 ) && DRN::CDRNCrypto::RandomBytes( keyThree, 32 ) );
        DRN::CDRNPacketProtection sender;
        DRN::CDRNPacketProtection receiver;
        DRN::CDRNPacketProtection stranger;
        REQUIRE_TRUE( sender.Setup( cipher, keyOne, keyTwo, 0 ) );
        REQUIRE_TRUE( receiver.Setup( cipher, keyTwo, keyOne, 1 ) );
        REQUIRE_TRUE( stranger.Setup( cipher, keyThree, keyThree, 1 ) );

        CORE::CDynamicBuffer plain;
        CORE::CDynamicBuffer sealed;
        CORE::CDynamicBuffer opened;
        MakePlainPacket( plain, 1, 300 );
        REQUIRE_TRUE( sender.Protect( static_cast< const UInt8* >( plain.GetConstBufferPtr() ), plain.GetDataSize(), sealed ) );
        ASSERT_TRUE( DRN::CDRNPacketProtection::IsProtectedPacket( static_cast< const UInt8* >( sealed.GetConstBufferPtr() ), sealed.GetDataSize() ) );
        ASSERT_TRUE( sealed.GetDataSize() == plain.GetDataSize() + DRN::CDRNPacketProtection::TagSize );
        ASSERT_TRUE( receiver.Unprotect( static_cast< const UInt8* >( sealed.GetConstBufferPtr() ), sealed.GetDataSize(), opened ) );
        ASSERT_TRUE( SameBytes( plain, opened ) );

        // another connection's keys, a changed header (the associated data), a changed body
        ASSERT_FALSE( stranger.Unprotect( static_cast< const UInt8* >( sealed.GetConstBufferPtr() ), sealed.GetDataSize(), opened ) );
        CORE::CDynamicBuffer changed( sealed );
        static_cast< UInt8* >( changed.GetBufferPtr() )[ 2 ] ^= 0x04;
        ASSERT_FALSE( receiver.Unprotect( static_cast< const UInt8* >( changed.GetConstBufferPtr() ), changed.GetDataSize(), opened ) );
        changed = sealed;
        static_cast< UInt8* >( changed.GetBufferPtr() )[ changed.GetDataSize() / 2 ] ^= 0x80;
        ASSERT_FALSE( receiver.Unprotect( static_cast< const UInt8* >( changed.GetConstBufferPtr() ), changed.GetDataSize(), opened ) );
        ASSERT_TRUE( 2 == receiver.GetFailedCount() );

        // a rekey every 10 packets; a packet of the old key that arrives after the receiver moved on still opens
        sender.SetRekeyLimits( 10, 0 );
        CORE::CDynamicBuffer late;
        bool allOpened = true;
        for ( UInt32 sequence=2; sequence<=40; ++sequence )
        {
            MakePlainPacket( plain, sequence, 100 );
            REQUIRE_TRUE( sender.Protect( static_cast< const UInt8* >( plain.GetConstBufferPtr() ), plain.GetDataSize(), sealed ) );
            if ( 9 == sequence )
            {
                late = sealed;
                continue;
            }
            allOpened = receiver.Unprotect( static_cast< const UInt8* >( sealed.GetConstBufferPtr() ), sealed.GetDataSize(), opened ) && SameBytes( plain, opened ) && allOpened;
            if ( 12 == sequence )
                allOpened = receiver.Unprotect( static_cast< const UInt8* >( late.GetConstBufferPtr() ), late.GetDataSize(), opened ) && allOpened;
        }
        ASSERT_TRUE( allOpened );
        ASSERT_TRUE( 4 == sender.GetSendRekeyCount() );
        ASSERT_TRUE( 3 == receiver.GetReceiveRekeyCount() );   // the fourth key has not been used yet
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Encryption: the datagram budget keeps room for the tag" )
    try
    {
        CLinkedPair pair;
        UInt16 port = NextTestPort();
        REQUIRE_TRUE( pair.Setup( EncryptionNodeSettings(), EncryptionNodeSettings(), LoopbackEndpoint( "lossy-udp", port ), LoopbackEndpoint( "lossy-udp", port ) ) );
        REQUIRE_TRUE( pair.linkA->IsProtected() );
        UInt32 largest = pair.linkA->GetMaxUnfragmentedPayload();
        CORE::CDynamicBuffer payload;
        payload.SetDataSize( largest );
        memset( payload.GetBufferPtr(), 0x5A, largest );
        for ( UInt32 i=0; i<20; ++i )
            ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_APP, TestOpcode, payload ) );
        CORE::CDynamicBuffer big;
        big.SetDataSize( 50000 );
        memset( big.GetBufferPtr(), 0x33, 50000 );
        ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, big ) );
        // the reliable one arrives whole, unreliable ones as far as loopback does not drop them
        CMessageCountCondition some( pair.recorderB, 2 );
        ASSERT_TRUE( PumpUntil( some, 10000 ) );
        CDRNLossyChannel* lossy = GetLossyChannel( pair.linkA );
        REQUIRE_TRUE( GUCEF_NULL != lossy );
        ASSERT_TRUE( lossy->GetMaxSentPacketSize() <= DRN::DRN_DEFAULT_DATAGRAM_SIZE );
        ASSERT_TRUE( lossy->GetMaxSentPacketSize() > DRN::DRN_DEFAULT_DATAGRAM_SIZE - 64 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Encryption: Ed25519 gives the RFC 8032 test vector through the signature codec" )
    try
    {
        UInt8 seed[ 32 ];
        UInt8 expectedPublic[ 32 ];
        UInt8 expectedSignature[ 64 ];
        REQUIRE_TRUE( DRN::CDRNCrypto::FromHex( "9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60", seed, 32 ) );
        REQUIRE_TRUE( DRN::CDRNCrypto::FromHex( "d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a", expectedPublic, 32 ) );
        REQUIRE_TRUE( DRN::CDRNCrypto::FromHex( "e5564300c360ac729086e2cc806e828a84877f1eb8e5d974d873e065224901555fb8821590a33bacc61e39701cf9b46bd25bf5f0595bbe24655141438e7a100b", expectedSignature, 64 ) );
        UInt8 publicKey[ 32 ];
        UInt8 signature[ 64 ];
        ASSERT_TRUE( DRN::CDRNCrypto::GetSignaturePublicKey( seed, publicKey ) && 0 == memcmp( publicKey, expectedPublic, 32 ) );
        ASSERT_TRUE( DRN::CDRNCrypto::Sign( seed, "", 0, signature ) && 0 == memcmp( signature, expectedSignature, 64 ) );
        ASSERT_TRUE( DRN::CDRNCrypto::Verify( expectedPublic, "", 0, expectedSignature ) );
        signature[ 10 ] ^= 1;
        ASSERT_FALSE( DRN::CDRNCrypto::Verify( expectedPublic, "", 0, signature ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Encryption: signed tokens are checked with the issuer's public key, whoever checks cannot make them" )
    try
    {
        UInt8 issuerSeed[ 32 ];
        UInt8 issuerPublic[ 32 ];
        UInt8 memberSeed[ 32 ];
        UInt8 memberPublic[ 32 ];
        UInt8 userPublic[ 32 ];
        UInt8 userSeed[ 32 ];
        REQUIRE_TRUE( DRN::CDRNCrypto::GenerateSignatureKeyPair( issuerSeed, issuerPublic ) );
        REQUIRE_TRUE( DRN::CDRNCrypto::GenerateSignatureKeyPair( memberSeed, memberPublic ) );
        REQUIRE_TRUE( DRN::CDRNCrypto::GenerateSignatureKeyPair( userSeed, userPublic ) );

        DRN::CDRNBanList bans;
        bans.Ban( DRN::DRN_BAN_PUBLIC_KEY, DRN::CDRNBanEntry::PublicKeyToString( userPublic, 32 ), 0, "known cheater" );

        DRN::CDRNNodeSettings hostSettings = EncryptionNodeSettings();
        hostSettings.securityPolicy.requireSignedTokens = true;
        hostSettings.auth.authenticatorType = "token";
        hostSettings.auth.SetSetting( "issuerPublicKey", DRN::CDRNCrypto::ToHex( issuerPublic, 32 ) );
        DRN::CDRNNode host;
        REQUIRE_TRUE( host.SetSettings( hostSettings ) );
        host.SetBanList( &bans );
        UInt16 port = NextTestPort();
        REQUIRE_TRUE( host.Listen( LoopbackEndpoint( "udp", port ) ) );

        UInt64 expiry = DRN::CDRNTokenAuthenticator::GetUnixTimeNow() + 600;
        CORE::CString tokens[ 4 ];
        tokens[ 0 ] = DRN::CDRNTokenAuthenticator::IssueSignedToken( issuerSeed, "arena", "alice", "player", expiry );
        tokens[ 1 ] = DRN::CDRNTokenAuthenticator::IssueToken( "lobby key", "arena", "bob", "player", expiry );
        tokens[ 2 ] = DRN::CDRNTokenAuthenticator::IssueSignedToken( memberSeed, "arena", "mallory", "admin", expiry );
        tokens[ 3 ] = DRN::CDRNTokenAuthenticator::IssueSignedToken( issuerSeed, "arena", "eve", "player", expiry, userPublic );
        const char* expectedText[ 4 ] = { "", "a signed token is required", "invalid token", "banned: known cheater" };
        for ( UInt32 i=0; i<4; ++i )
        {
            REQUIRE_TRUE( !tokens[ i ].IsNULLOrEmpty() );
            DRN::CDRNNodeSettings userSettings = EncryptionNodeSettings();
            userSettings.securityPolicy.requireSignedTokens = true;
            userSettings.auth.authenticatorType = "token";
            userSettings.auth.SetSetting( "token", tokens[ i ] );
            DRN::CDRNNode user;
            REQUIRE_TRUE( user.SetSettings( userSettings ) );
            CEncryptionConnectorRecorder recorder;
            REQUIRE_TRUE( ConnectAndWait( user, DRN::CDRNEndpointList( LoopbackEndpoint( "udp", port ) ), recorder ) );
            if ( 0 == i )
            {
                ASSERT_TRUE( 1 == recorder.connected );
            }
            else
            {
                ASSERT_TRUE( 1 == recorder.failed );
                ASSERT_TRUE( recorder.failText == expectedText[ i ] );
            }
            user.Stop();
        }

        // a node that checks signed tokens has to have the issuer's key
        DRN::CDRNNodeSettings hmacOnly = EncryptionNodeSettings();
        hmacOnly.securityPolicy.requireSignedTokens = true;
        hmacOnly.auth.authenticatorType = "token";
        hmacOnly.auth.SetSetting( "key", "lobby key" );
        DRN::CDRNNode hmacNode;
        ASSERT_FALSE( hmacNode.SetSettings( hmacOnly ) );

        host.SetBanList( GUCEF_NULL );
        host.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Encryption: a token carries the policy hash of its game, another hash or none (when the policy wants one) is refused" )
    try
    {
        DRN::CDRNNodeSettings hostSettings = EncryptionNodeSettings();
        hostSettings.securityPolicy.requireTokenPolicyHash = true;
        hostSettings.auth.authenticatorType = "token";
        hostSettings.auth.SetSetting( "key", "lobby key" );
        DRN::CDRNNode host;
        REQUIRE_TRUE( host.SetSettings( hostSettings ) );
        UInt16 port = NextTestPort();
        REQUIRE_TRUE( host.Listen( LoopbackEndpoint( "tcp", port ) ) );

        UInt8 policyHash[ DRN::DRN_HANDSHAKE_HASH_SIZE ];
        hostSettings.securityPolicy.GetPolicyHash( policyHash );
        DRN::CDRNSecurityPolicy otherPolicy;
        UInt8 otherHash[ DRN::DRN_HANDSHAKE_HASH_SIZE ];
        otherPolicy.GetPolicyHash( otherHash );
        UInt64 expiry = DRN::CDRNTokenAuthenticator::GetUnixTimeNow() + 600;
        CORE::CString tokens[ 3 ];
        tokens[ 0 ] = DRN::CDRNTokenAuthenticator::IssueToken( "lobby key", "arena", "alice", "player", expiry, policyHash );
        tokens[ 1 ] = DRN::CDRNTokenAuthenticator::IssueToken( "lobby key", "arena", "bob", "player", expiry, otherHash );
        tokens[ 2 ] = DRN::CDRNTokenAuthenticator::IssueToken( "lobby key", "arena", "carol", "player", expiry );
        const char* expectedText[ 3 ] = { "", "the token is for another security policy", "the token has no security policy hash" };
        for ( UInt32 i=0; i<3; ++i )
        {
            DRN::CDRNNodeSettings userSettings = EncryptionNodeSettings();
            userSettings.securityPolicy.requireTokenPolicyHash = true;
            userSettings.auth.authenticatorType = "token";
            userSettings.auth.SetSetting( "token", tokens[ i ] );
            DRN::CDRNNode user;
            REQUIRE_TRUE( user.SetSettings( userSettings ) );
            CEncryptionConnectorRecorder recorder;
            REQUIRE_TRUE( ConnectAndWait( user, DRN::CDRNEndpointList( LoopbackEndpoint( "tcp", port ) ), recorder ) );
            if ( 0 == i )
                ASSERT_TRUE( 1 == recorder.connected );
            else
                ASSERT_TRUE( 1 == recorder.failed && recorder.failText == expectedText[ i ] );
            user.Stop();
        }

        // the fields round trip, the hash included
        CORE::CDynamicBuffer binary;
        binary.SetBufferSize( tokens[ 0 ].ByteSize(), false );
        UInt32 used = 0;
        REQUIRE_TRUE( CORE::Base64Decode( tokens[ 0 ], binary.GetBufferPtr(), tokens[ 0 ].ByteSize(), used ) );
        CORE::CString session, userId, role;
        UInt64 parsedExpiry = 0;
        CORE::CDynamicBuffer parsedHash;
        ASSERT_TRUE( DRN::CDRNTokenAuthenticator::ParseToken( "lobby key", static_cast< const UInt8* >( binary.GetConstBufferPtr() ), used, session, userId, role, parsedExpiry, &parsedHash ) );
        ASSERT_TRUE( session == "arena" && userId == "alice" && parsedExpiry == expiry );
        ASSERT_TRUE( DRN::DRN_HANDSHAKE_HASH_SIZE == parsedHash.GetDataSize() && 0 == memcmp( parsedHash.GetConstBufferPtr(), policyHash, sizeof( policyHash ) ) );
        host.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Encryption: the static key is made once and kept in its file, or given in the settings" )
    try
    {
        CORE::CString keyFile = TestFilePath( "gucefDRN_TestApp_static.key" );
        CORE::DeleteFile( keyFile );
        DRN::CDRNNodeSettings settings = EncryptionNodeSettings();
        settings.staticKeyFile = keyFile;
        CORE::CString firstKey;
        {
            DRN::CDRNNode first;
            REQUIRE_TRUE( first.SetSettings( settings ) );
            firstKey = first.GetPublicKeyString();
        }
        ASSERT_TRUE( 64 == firstKey.Length() && CORE::FileExists( keyFile ) );
        DRN::CDRNNode second;
        REQUIRE_TRUE( second.SetSettings( settings ) );
        ASSERT_TRUE( second.GetPublicKeyString() == firstKey );

        CORE::CString content;
        REQUIRE_TRUE( CORE::LoadTextFileAsString( keyFile, content ) );
        DRN::CDRNNodeSettings given = EncryptionNodeSettings();
        given.staticPrivateKey = content.RemoveChar( '\n' ).RemoveChar( '\r' );
        DRN::CDRNNode third;
        REQUIRE_TRUE( third.SetSettings( given ) );
        ASSERT_TRUE( third.GetPublicKeyString() == firstKey );

        DRN::CDRNNode fresh;
        REQUIRE_TRUE( fresh.SetSettings( EncryptionNodeSettings() ) );
        ASSERT_TRUE( fresh.GetPublicKeyString() != firstKey );
        CORE::DeleteFile( keyFile );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Encryption: an AUTH taken from one handshake and put into another is refused" )
    try
    {
        CDRNLossyChannel::ClearRecordedAuthPacket();
        CLinkedPair first;
        UInt16 port = NextTestPort();
        REQUIRE_TRUE( first.Setup( EncryptionNodeSettings(), EncryptionNodeSettings(), LoopbackEndpoint( "lossy-tcp", port ), LoopbackEndpoint( "lossy-tcp", port, "?recordAuth=1" ) ) );
        REQUIRE_TRUE( first.linkB->IsProtected() );
        REQUIRE_TRUE( CDRNLossyChannel::GetRecordedAuthPacket().GetDataSize() > 0 );

        // a second peer that sends the recorded AUTH in place of its own, as an eavesdropper would
        DRN::CDRNNode replayer;
        REQUIRE_TRUE( replayer.SetSettings( EncryptionNodeSettings() ) );
        CEncryptionConnectorRecorder recorder;
        REQUIRE_TRUE( ConnectAndWait( replayer, DRN::CDRNEndpointList( LoopbackEndpoint( "lossy-tcp", port, "?injectAuth=1" ) ), recorder ) );
        ASSERT_TRUE( 1 == recorder.failed && 0 == recorder.connected );
        ASSERT_TRUE( 1 == first.recorderB.operationalCount );
        ASSERT_TRUE( PumpUntil( first.recorderB.closedCount, 1, 3000 ) );
        ASSERT_TRUE( DRN::DRN_LINKREASON_PROTOCOL_ERROR == first.recorderB.lastClosedReason );
        ASSERT_TRUE( first.linkB->IsOperational() );
        replayer.Stop();
        CDRNLossyChannel::ClearRecordedAuthPacket();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Encryption: every AEAD provider seals and opens a datagram, timed" )
    try
    {
        // the providers there are now, then those of codecspluginOPENSSL when it is there
        CORE::CCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
        const char* types[] = { "xchacha20-poly1305", "chacha20-poly1305", "aes-256-gcm" };
        GUCEF::set< const CORE::CICodec* > before;
        for ( UInt32 t=0; t<3; ++t )
        {
            CORE::CCodecRegistry::TICodecPtrVector codecs;
            registry.GetCodecs( CORE::CoreCodecTypes::EncryptionCodec, types[ t ], codecs );
            for ( UInt32 i=0; i<codecs.size(); ++i )
                before.insert( codecs[ i ].GetPointerAlways() );
        }
        bool haveOpenSsl = LoadCodecPlugin( "codecspluginOPENSSL" );
        UInt32 measured = 0;
        for ( UInt32 t=0; t<3; ++t )
        {
            CORE::CCodecRegistry::TICodecPtrVector codecs;
            registry.GetCodecs( CORE::CoreCodecTypes::EncryptionCodec, types[ t ], codecs );
            for ( UInt32 i=0; i<codecs.size(); ++i )
            {
                const CORE::CICodec* codec = codecs[ i ].GetPointerAlways();
                CORE::CString provider = codec->GetClassTypeName().HasSubstr( "AfAlg", true ) >= 0 ? CORE::CString( "AF_ALG" ) :
                                         ( before.find( codec ) != before.end() ? CORE::CString( "codecspluginMONOCYPHER" ) : CORE::CString( "codecspluginOPENSSL" ) );
                Int32 priority = 0;
                registry.TryGetCodecPriority( CORE::CoreCodecTypes::EncryptionCodec, types[ t ], codec, priority );
                bool allOpened = false;
                Float64 us = TimeAeadProvider( codecs[ i ], 0 == t ? 24 : 12, 20000, allOpened );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Encryption: AEAD " << types[ t ] << " of " << provider << " (priority " << priority << "): " << us << " microseconds to seal and open " << DRN::DRN_DEFAULT_DATAGRAM_SIZE << " bytes" );
                ASSERT_TRUE( allOpened );
                ++measured;
            }
        }
        ASSERT_TRUE( measured >= 2 );
        if ( !haveOpenSsl )
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Encryption: codecspluginOPENSSL is not there, its providers are not timed" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Encryption: the cost of a packet and the latency it adds" )
    try
    {
        // seal and open a full datagram many times
        CORE::CString cipher = ExpectedCipher( DRN::CDRNSecurityPolicy() );
        UInt8 keyOne[ 32 ];
        UInt8 keyTwo[ 32 ];
        REQUIRE_TRUE( DRN::CDRNCrypto::RandomBytes( keyOne, 32 ) && DRN::CDRNCrypto::RandomBytes( keyTwo, 32 ) );
        DRN::CDRNPacketProtection sender;
        DRN::CDRNPacketProtection receiver;
        REQUIRE_TRUE( sender.Setup( cipher, keyOne, keyTwo, 0 ) && receiver.Setup( cipher, keyTwo, keyOne, 1 ) );
        CORE::CDynamicBuffer plain;
        CORE::CDynamicBuffer sealed;
        CORE::CDynamicBuffer opened;
        const UInt32 rounds = 20000;
        bool allOpened = true;
        UInt64 start = DRN::CDRNClock::NowInMicroseconds();
        for ( UInt32 i=1; i<=rounds; ++i )
        {
            MakePlainPacket( plain, i, DRN::DRN_DEFAULT_DATAGRAM_SIZE - 100 );
            allOpened = sender.Protect( static_cast< const UInt8* >( plain.GetConstBufferPtr() ), plain.GetDataSize(), sealed ) &&
                        receiver.Unprotect( static_cast< const UInt8* >( sealed.GetConstBufferPtr() ), sealed.GetDataSize(), opened ) && allOpened;
        }
        UInt64 took = DRN::CDRNClock::NowInMicroseconds() - start;
        Float64 perPacketUs = (Float64) took / rounds;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Encryption: " << cipher << " seals and opens a packet of " << DRN::DRN_DEFAULT_DATAGRAM_SIZE << " bytes in " << perPacketUs << " microseconds (with building the packet)" );
        ASSERT_TRUE( allOpened );
        ASSERT_TRUE( perPacketUs < 200.0 );

        // the round trip of a plain and of an encrypted link on loopback
        DRN::CDRNNodeSettings plainSettings = EncryptionNodeSettings();
        plainSettings.securityPolicy.requireEncryption = false;
        plainSettings.securityPolicy.ciphers.clear();
        DRN::CDRNNodeSettings plainSettingsB = plainSettings;
        plainSettingsB.nodeId = DRN::CDRNNodeSettings::GenerateNodeId();
        CLinkedPair plainPair;
        UInt16 plainPort = NextTestPort();
        REQUIRE_TRUE( plainPair.Setup( plainSettings, plainSettingsB, LoopbackEndpoint( "udp", plainPort ), LoopbackEndpoint( "udp", plainPort ) ) );
        ASSERT_FALSE( plainPair.linkA->IsProtected() );
        CLinkedPair encryptedPair;
        UInt16 encryptedPort = NextTestPort();
        REQUIRE_TRUE( encryptedPair.Setup( EncryptionNodeSettings(), EncryptionNodeSettings(), LoopbackEndpoint( "udp", encryptedPort ), LoopbackEndpoint( "udp", encryptedPort ) ) );
        ASSERT_TRUE( encryptedPair.linkA->IsProtected() );
        PumpFor( 1500 );
        DRN::CDRNLatencyInfo plainInfo;
        DRN::CDRNLatencyInfo encryptedInfo;
        plainPair.linkA->GetLatencyInfo( plainInfo );
        encryptedPair.linkA->GetLatencyInfo( encryptedInfo );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Encryption: loopback round trip plain " << plainInfo.smoothedRtt << " us, encrypted " << encryptedInfo.smoothedRtt << " us" );
        ASSERT_TRUE( plainInfo.sampleCount > 0 && encryptedInfo.sampleCount > 0 );
        ASSERT_TRUE( encryptedInfo.smoothedRtt < plainInfo.smoothedRtt + 5000 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
