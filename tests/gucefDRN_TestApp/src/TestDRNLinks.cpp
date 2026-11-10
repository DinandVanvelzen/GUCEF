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
#include "TestDRNAllocationCounter.h"
#include "TestDRNGroup.h"
#include "TestDRNMemoryChannel.h"
#include "TestDRNLinks.h"

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

static const UInt32 TestOpcode = 9;

static const char* const ChannelTypes[] = { "tcp", "udp", "websocket" };
static const UInt32 ChannelTypeCount = 3;

/*-------------------------------------------------------------------------*/

static DRN::CDRNNodeSettings
TestNodeSettings( void )
{
    DRN::CDRNNodeSettings settings;
    settings.echoIntervalMs = 20;
    settings.latencyUpdateIntervalMs = 200;
    settings.handshakeTimeoutMs = 3000;
    settings.linkTimeoutMs = 10000;
    settings.pulseIntervalMs = 1;
    return settings;
}

/*-------------------------------------------------------------------------*/

/** A payload carrying a counter and filler up to size */
static void
MakeCountedPayload( CORE::CDynamicBuffer& buffer, UInt32 counter, UInt32 size )
{
    if ( size < 4 )
        size = 4;
    buffer.SetDataSize( size );
    UInt8* p = static_cast< UInt8* >( buffer.GetBufferPtr() );
    p[ 0 ] = (UInt8) ( counter >> 24 );
    p[ 1 ] = (UInt8) ( counter >> 16 );
    p[ 2 ] = (UInt8) ( counter >> 8 );
    p[ 3 ] = (UInt8) counter;
    for ( UInt32 i=4; i<size; ++i )
        p[ i ] = (UInt8) ( counter + i * 7 );
}

/*-------------------------------------------------------------------------*/

/** @return the counter of a counted payload, GUCEF_UINT32MAX if the filler is not intact */
static UInt32
ReadCountedPayload( const CORE::CDynamicBuffer& buffer )
{
    if ( buffer.GetDataSize() < 4 )
        return GUCEF_UINT32MAX;
    const UInt8* p = static_cast< const UInt8* >( buffer.GetConstBufferPtr() );
    UInt32 counter = ( (UInt32) p[ 0 ] << 24 ) | ( (UInt32) p[ 1 ] << 16 ) | ( (UInt32) p[ 2 ] << 8 ) | p[ 3 ];
    for ( UInt32 i=4; i<buffer.GetDataSize(); ++i )
        if ( p[ i ] != (UInt8) ( counter + i * 7 ) )
            return GUCEF_UINT32MAX;
    return counter;
}

/*-------------------------------------------------------------------------*/

/** Counts the messages of one lane a recorder got */
class CLaneCountCondition : public CTestCondition
{
    public:
    CLaneCountCondition( const CLinkRecorder& recorder, UInt8 lane, UInt32 expected ) : m_recorder( recorder ), m_lane( lane ), m_expected( expected ) {}
    UInt32 Count( void ) const
    {
        UInt32 count = 0;
        for ( UInt32 i=0; i<m_recorder.messages.size(); ++i )
            if ( m_recorder.messages[ i ].lane == m_lane )
                ++count;
        return count;
    }
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return Count() >= m_expected; }
    private:
    const CLinkRecorder& m_recorder;
    UInt8 m_lane;
    UInt32 m_expected;
};

/*-------------------------------------------------------------------------*/

/** The counters of the messages of one lane, in arrival order */
static void
GetLaneCounters( const CLinkRecorder& recorder, UInt8 lane, GUCEF::vector< UInt32 >& counters )
{
    counters.clear();
    for ( UInt32 i=0; i<recorder.messages.size(); ++i )
        if ( recorder.messages[ i ].lane == lane )
            counters.push_back( ReadCountedPayload( recorder.messages[ i ].payload ) );
}

/*-------------------------------------------------------------------------*/

static bool
IsSequence( const GUCEF::vector< UInt32 >& counters, UInt32 count )
{
    if ( counters.size() != count )
        return false;
    for ( UInt32 i=0; i<count; ++i )
        if ( counters[ i ] != i )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/** Every counter 0..count-1 exactly once, in any order */
static bool
IsCompleteSet( const GUCEF::vector< UInt32 >& counters, UInt32 count )
{
    if ( counters.size() != count )
        return false;
    GUCEF::vector< UInt8 > seen( count, 0 );
    for ( UInt32 i=0; i<counters.size(); ++i )
    {
        if ( counters[ i ] >= count || 0 != seen[ counters[ i ] ] )
            return false;
        seen[ counters[ i ] ] = 1;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Answers every message of a link at once and flushes each answer on its own, as an RPC server answers requests while
 *  it receives them; closes the link right after the answer to the last one
 */
class CEchoAtOnce : public CORE::CObserver
{
    public:

    DRN::CDRNPeerLinkPtr link;
    UInt32 closeAfter;      /**< closes the link once it answered this many, 0 for never */
    UInt32 answered;

    CEchoAtOnce( void ) : CORE::CObserver(), link(), closeAfter( 0 ), answered( 0 ) {}
    virtual ~CEchoAtOnce() { UnsubscribeAllFromObserver(); }

    void OnMessage( CORE::CNotifier* /* notifier */, const CORE::CEvent& /* eventId */, CORE::CICloneable* eventData )
    {
        if ( link.IsNULL() || GUCEF_NULL == eventData )
            return;
        const DRN::CDRNLinkMessage& message = static_cast< DRN::CDRNLinkMessageEventData* >( eventData )->GetData();
        link->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, message.payload );
        link->Flush();
        ++answered;
        if ( answered == closeAfter )
            link->Close();
    }
};

typedef CORE::CTEventHandlerFunctor< CEchoAtOnce > TEchoAtOnceCallback;

/*-------------------------------------------------------------------------*/

/** The counters of the messages a CDRNLinkReliability hands over, per reliable lane, in delivery order */
class CReliabilitySink : public DRN::CIDRNLinkMessageSink
{
    public:

    GUCEF::vector< UInt32 > lane0;
    GUCEF::vector< UInt32 > lane1;

    virtual bool OnLinkMessage( const DRN::CDRNMessageView& message ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( message.payloadSize < 4 )
            return true;
        UInt32 counter = ( (UInt32) message.payload[ 0 ] << 24 ) | ( (UInt32) message.payload[ 1 ] << 16 ) | ( (UInt32) message.payload[ 2 ] << 8 ) | message.payload[ 3 ];
        if ( DRN::DRN_LANE_RELIABLE_ORDERED == message.lane )
            lane0.push_back( counter );
        else
        if ( DRN::DRN_LANE_RELIABLE_UNORDERED == message.lane )
            lane1.push_back( counter );
        return true;
    }
};

/*-------------------------------------------------------------------------*/

/** The messages of a lane in a packet built by a CDRNLinkReliability, with the highest lane sequence among them */
static UInt32
CountLaneMessages( const UInt8* packet, UInt32 size, UInt8 lane, UInt32& highestLaneSequence )
{
    DRN::CDRNWireReader reader( packet, size );
    DRN::CDRNPacketHeader header;
    if ( !DRN::CDRNPacketCodec::ReadHeader( reader, header ) )
        return 0;
    UInt32 count = 0;
    while ( reader.GetRemaining() > 0 )
    {
        DRN::CDRNMessageView message;
        if ( !DRN::CDRNPacketCodec::ReadMessage( reader, message ) )
            break;
        if ( message.lane != lane )
            continue;
        ++count;
        if ( message.laneSequence > highestLaneSequence )
            highestLaneSequence = message.laneSequence;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

/** The ack fields of the next packet a CDRNLinkReliability builds, a header only one if it has nothing to send */
static bool
GetNextAck( DRN::CDRNLinkReliability& reliability, UInt64 now, UInt32& ackSequence, UInt32& ackBits )
{
    const UInt8* packet = GUCEF_NULL;
    UInt32 packetSize = 0;
    if ( !reliability.BuildPacket( now, true, packet, packetSize ) )
        return false;
    DRN::CDRNWireReader reader( packet, packetSize );
    DRN::CDRNPacketHeader header;
    if ( !DRN::CDRNPacketCodec::ReadHeader( reader, header ) )
        return false;
    ackSequence = header.ackSequence;
    ackBits = header.ackBits;
    return true;
}

/*-------------------------------------------------------------------------*/

/** A packet of one reliable message with the given lane sequence and a counted payload, as a peer's link would send it */
static void
MakeReliablePacket( CORE::CDynamicBuffer& packet, UInt32 sequence, UInt8 lane, UInt32 laneSequence, UInt32 counter )
{
    CORE::CDynamicBuffer payload;
    MakeCountedPayload( payload, counter, 8 );
    packet.SetDataSize( 0 );
    DRN::CDRNWireWriter writer( packet );
    DRN::CDRNPacketHeader header;
    header.sequence = sequence;
    header.sendTime = 1000 * sequence;
    DRN::CDRNPacketCodec::WriteHeader( writer, header );
    DRN::CDRNPacketCodec::WriteMessage( writer, DRN::DRN_MSGCLASS_APP, TestOpcode, lane, laneSequence, payload.GetConstBufferPtr(), payload.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

static bool
SetupPair( CLinkedPair& pair, const CORE::CString& channelType, const CORE::CString& params, const DRN::CDRNNodeSettings& settings )
{
    UInt16 port = NextTestPort();
    DRN::CDRNEndpoint listen = LoopbackEndpoint( channelType, port, params );
    DRN::CDRNEndpoint connect = LoopbackEndpoint( channelType, port, params );
    return pair.Setup( settings, settings, listen, connect );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDRNLinkTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.Links" );

    bool webSocketAvailable = EnsureHashCodecPluginLoaded();

    GUCEF_TESTFW_TESTCASE( "Links: The web socket channel is available (sha1 codec loaded)" )
    try
    {
        ASSERT_TRUE( webSocketAvailable );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    for ( UInt32 t=0; t<ChannelTypeCount; ++t )
    {
        CORE::CString channelType = ChannelTypes[ t ];
        if ( channelType == "websocket" && !webSocketAvailable )
            continue;

        const CORE::CString caseName1 = "Links: Messages on every lane over " + channelType;
        GUCEF_TESTFW_TESTCASE( caseName1.C_String() )
        try
        {
            CLinkedPair pair;
            REQUIRE_TRUE( SetupPair( pair, channelType, CORE::CString::Empty, TestNodeSettings() ) );
            ASSERT_TRUE( pair.linkA->IsOperational() && pair.linkB->IsOperational() );
            ASSERT_TRUE( pair.linkA->IsInitiator() && !pair.linkB->IsInitiator() );
            ASSERT_TRUE( pair.linkA->GetRemoteNodeId() == pair.nodeB.GetSettings().nodeId );
            ASSERT_TRUE( pair.linkB->GetRemoteNodeId() == pair.nodeA.GetSettings().nodeId );

            const UInt32 count = 100;
            CORE::CDynamicBuffer payload;
            for ( UInt32 i=0; i<count; ++i )
            {
                MakeCountedPayload( payload, i, 4 + ( i * 13 ) % 200 );
                for ( UInt8 lane=0; lane<DRN::DRN_LANE_COUNT; ++lane )
                    ASSERT_TRUE( pair.linkA->Send( lane, DRN::DRN_MSGCLASS_APP, TestOpcode, payload ) );
                if ( 0 == i % 10 )
                    pair.linkA->Flush();
            }
            CLaneCountCondition lane0( pair.recorderB, DRN::DRN_LANE_RELIABLE_ORDERED, count );
            CLaneCountCondition lane1( pair.recorderB, DRN::DRN_LANE_RELIABLE_UNORDERED, count );
            ASSERT_TRUE( PumpUntil( lane0 ) && PumpUntil( lane1 ) );
            PumpFor( 50 );

            GUCEF::vector< UInt32 > counters;
            GetLaneCounters( pair.recorderB, DRN::DRN_LANE_RELIABLE_ORDERED, counters );
            ASSERT_TRUE( IsSequence( counters, count ) );
            GetLaneCounters( pair.recorderB, DRN::DRN_LANE_RELIABLE_UNORDERED, counters );
            ASSERT_TRUE( IsCompleteSet( counters, count ) );
            // loopback without a lossy channel: nothing is lost on the other lanes either
            GetLaneCounters( pair.recorderB, DRN::DRN_LANE_UNRELIABLE, counters );
            ASSERT_TRUE( IsCompleteSet( counters, count ) );
            GetLaneCounters( pair.recorderB, DRN::DRN_LANE_UNRELIABLE_SEQUENCED, counters );
            ASSERT_TRUE( count == counters.size() );

            // the link's own messages are not for the application
            ASSERT_FALSE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_LINK, DRN::DRN_LINK_ECHO, GUCEF_NULL, 0 ) );
            if ( channelType == "udp" )
            {
                // an unreliable message larger than a datagram is refused, a reliable one is fragmented
                CORE::CDynamicBuffer big;
                MakeCountedPayload( big, 7, 5000 );
                ASSERT_FALSE( pair.linkA->Send( DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_APP, TestOpcode, big ) );
            }

            // closing: both ends learn about it
            pair.linkA->Close();
            ASSERT_TRUE( PumpUntil( pair.recorderA.closedCount, 1 ) );
            ASSERT_TRUE( PumpUntil( pair.recorderB.closedCount, 1, 12000 ) );
            ASSERT_TRUE( DRN::DRN_LINKREASON_CLOSED_BY_APP == pair.recorderA.lastClosedReason );
            ASSERT_TRUE( DRN::DRN_LINKREASON_REMOTE_CLOSED == pair.recorderB.lastClosedReason || DRN::DRN_LINKREASON_CHANNEL_LOST == pair.recorderB.lastClosedReason );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        GUCEF_TESTFW_TESTCASE_END
    }

    for ( UInt32 t=0; t<ChannelTypeCount; ++t )
    {
        CORE::CString channelType = ChannelTypes[ t ];
        if ( channelType == "websocket" && !webSocketAvailable )
            continue;

        // a burst of packets arrives in few reads: the answers each flushed on their own while a read is delivered
        // leave complete and in order, and the close that follows the last answer comes after it
        const CORE::CString caseNameBurst = "Links: Answers flushed one by one while a burst is received arrive complete and in order, before the close, over " + channelType;
        GUCEF_TESTFW_TESTCASE( caseNameBurst.C_String() )
        try
        {
            CLinkedPair pair;
            REQUIRE_TRUE( SetupPair( pair, channelType, CORE::CString::Empty, TestNodeSettings() ) );

            const UInt32 count = 400;
            CEchoAtOnce echo;
            echo.link = pair.linkB;
            echo.closeAfter = count;
            TEchoAtOnceCallback onMessage( &echo, &CEchoAtOnce::OnMessage );
            echo.SubscribeTo( pair.linkB.GetPointerAlways(), DRN::CDRNPeerLink::MessageReceivedEvent, onMessage );

            CORE::CDynamicBuffer payload;
            for ( UInt32 i=0; i<count; ++i )
            {
                MakeCountedPayload( payload, i, 16 + ( i * 13 ) % 64 );
                ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, payload ) );
                pair.linkA->Flush();
            }
            CLaneCountCondition answers( pair.recorderA, DRN::DRN_LANE_RELIABLE_ORDERED, count );
            ASSERT_TRUE( PumpUntil( answers, 30000 ) );
            ASSERT_TRUE( count == echo.answered );
            GUCEF::vector< UInt32 > counters;
            GetLaneCounters( pair.recorderA, DRN::DRN_LANE_RELIABLE_ORDERED, counters );
            ASSERT_TRUE( IsSequence( counters, count ) );

            ASSERT_TRUE( PumpUntil( pair.recorderA.closedCount, 1, 12000 ) );
            ASSERT_TRUE( DRN::DRN_LINKREASON_REMOTE_CLOSED == pair.recorderA.lastClosedReason || DRN::DRN_LINKREASON_CHANNEL_LOST == pair.recorderA.lastClosedReason );
            echo.link.Unlink();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        GUCEF_TESTFW_TESTCASE_END
    }

    for ( UInt32 t=0; t<ChannelTypeCount; ++t )
    {
        CORE::CString channelType = ChannelTypes[ t ];
        if ( channelType == "websocket" && !webSocketAvailable )
            continue;

        const CORE::CString caseName2 = "Links: Large messages over " + channelType;
        GUCEF_TESTFW_TESTCASE( caseName2.C_String() )
        try
        {
            // both nodes run on this thread: while one seals and frames 10 MB the other hears nothing. Under a sanitizer
            // that takes longer than the link timeout of the test settings (10 s), and the links time out before the
            // message is through; they get the time it takes
            DRN::CDRNNodeSettings settings = TestNodeSettings();
            if ( IsSanitizerBuild() )
                settings.linkTimeoutMs = 120000;
            CLinkedPair pair;
            REQUIRE_TRUE( SetupPair( pair, channelType, CORE::CString::Empty, settings ) );

            // 10 MB on the stream channels, the datagram channel fragments a smaller one
            UInt32 size = channelType == "udp" ? 300 * 1024 : 10 * 1024 * 1024;
            CORE::CDynamicBuffer payload;
            MakeCountedPayload( payload, 4242, size );
            ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, payload ) );
            CORE::CDynamicBuffer small;
            MakeCountedPayload( small, 1, 10 );
            ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, small ) );
            pair.linkA->Flush();

            // the whole size in every build: what is tested is a message that large; a sanitizer slows the sealing and
            // the framing of it down many times
            UInt64 startUs = DRN::CDRNClock::NowInMicroseconds();
            ASSERT_TRUE( PumpUntil( pair.recorderB.messageCount, 2, IsSanitizerBuild() ? 300000 : 60000 ) );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Large message of " << size << " bytes over " << channelType << ": " << ( DRN::CDRNClock::NowInMicroseconds() - startUs ) / 1000 << " ms" );
            ASSERT_TRUE( 2 == pair.recorderB.messages.size() );
            if ( 2 == pair.recorderB.messages.size() )
            {
                ASSERT_TRUE( size == pair.recorderB.messages[ 0 ].payload.GetDataSize() );
                ASSERT_TRUE( 4242 == ReadCountedPayload( pair.recorderB.messages[ 0 ].payload ) );
                ASSERT_TRUE( 1 == ReadCountedPayload( pair.recorderB.messages[ 1 ].payload ) );
            }
            if ( channelType == "udp" )
                ASSERT_TRUE( pair.linkA->GetStats().fragmentedMessages > 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        GUCEF_TESTFW_TESTCASE_END
    }

    for ( UInt32 t=0; t<ChannelTypeCount; ++t )
    {
        CORE::CString channelType = ChannelTypes[ t ];
        if ( channelType == "websocket" && !webSocketAvailable )
            continue;

        for ( UInt32 lossy=0; lossy<2; ++lossy )
        {
            CORE::CString type = lossy > 0 ? "lossy-" + channelType : channelType;
            CORE::CString params = lossy > 0 ? CORE::CString( "?loss=0.1&reorder=0.1" ) : CORE::CString::Empty;

            const CORE::CString caseName3 = "Links: Packet sequences wrap around 2^32 without losing or reordering over " + type;
            GUCEF_TESTFW_TESTCASE( caseName3.C_String() )
            try
            {
                DRN::CDRNNodeSettings settings = TestNodeSettings();
                settings.firstSequence = 0xFFFFFF00;
                CLinkedPair pair;
                REQUIRE_TRUE( SetupPair( pair, type, params, settings ) );

                const UInt32 count = 600;
                CORE::CDynamicBuffer payload;
                for ( UInt32 i=0; i<count; ++i )
                {
                    MakeCountedPayload( payload, i, 16 );
                    ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, payload ) );
                    ASSERT_TRUE( pair.linkB->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, payload ) );
                    pair.linkA->Flush();
                    pair.linkB->Flush();
                    if ( 0 == i % 50 )
                        PumpFor( 2 );
                }
                ASSERT_TRUE( PumpUntil( pair.recorderB.messageCount, count, 30000 ) );
                ASSERT_TRUE( PumpUntil( pair.recorderA.messageCount, count, 30000 ) );
                GUCEF::vector< UInt32 > counters;
                GetLaneCounters( pair.recorderB, DRN::DRN_LANE_RELIABLE_ORDERED, counters );
                ASSERT_TRUE( IsSequence( counters, count ) );
                GetLaneCounters( pair.recorderA, DRN::DRN_LANE_RELIABLE_ORDERED, counters );
                ASSERT_TRUE( IsSequence( counters, count ) );

                // both directions went past 2^32
                ASSERT_TRUE( pair.linkA->GetNextSequence() < 0x80000000 );
                ASSERT_TRUE( pair.linkB->GetNextSequence() < 0x80000000 );
                ASSERT_TRUE( pair.linkA->IsOperational() && pair.linkB->IsOperational() );
                if ( lossy > 0 )
                    ASSERT_TRUE( pair.linkA->GetStats().messagesResent > 0 || pair.linkB->GetStats().messagesResent > 0 );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
            GUCEF_TESTFW_TESTCASE_END
        }
    }

    for ( UInt32 t=0; t<ChannelTypeCount; ++t )
    {
        CORE::CString channelType = ChannelTypes[ t ];
        if ( channelType == "websocket" && !webSocketAvailable )
            continue;

        const CORE::CString caseName4 = "Links: Duplicated packets are dropped over lossy-" + channelType;
        GUCEF_TESTFW_TESTCASE( caseName4.C_String() )
        try
        {
            CLinkedPair pair;
            REQUIRE_TRUE( SetupPair( pair, "lossy-" + channelType, "?dup=1.0", TestNodeSettings() ) );

            const UInt32 count = 200;
            CORE::CDynamicBuffer payload;
            for ( UInt32 i=0; i<count; ++i )
            {
                MakeCountedPayload( payload, i, 8 );
                ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, payload ) );
                ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_APP, TestOpcode, payload ) );
                pair.linkA->Flush();
                if ( 0 == i % 10 )
                    PumpFor( 1 );
            }
            CLaneCountCondition lane0( pair.recorderB, DRN::DRN_LANE_RELIABLE_ORDERED, count );
            ASSERT_TRUE( PumpUntil( lane0, 20000 ) );
            PumpFor( 100 );

            GUCEF::vector< UInt32 > counters;
            GetLaneCounters( pair.recorderB, DRN::DRN_LANE_RELIABLE_ORDERED, counters );
            ASSERT_TRUE( IsSequence( counters, count ) );

            // every packet came twice, yet nothing is delivered twice
            GetLaneCounters( pair.recorderB, DRN::DRN_LANE_UNRELIABLE, counters );
            GUCEF::vector< UInt8 > seen( count, 0 );
            bool unique = true;
            for ( UInt32 i=0; i<counters.size(); ++i )
            {
                if ( counters[ i ] >= count || 0 != seen[ counters[ i ] ] )
                    unique = false;
                else
                    seen[ counters[ i ] ] = 1;
            }
            ASSERT_TRUE( unique );
            if ( channelType != "udp" )
                ASSERT_TRUE( count == counters.size() );    // over udp the doubled traffic may overflow the socket buffer
            ASSERT_TRUE( pair.linkB->GetStats().duplicatePackets >= count / 2 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        GUCEF_TESTFW_TESTCASE_END

        const CORE::CString caseName5 = "Links: Stale messages on lane 2 are dropped, the newest win, over lossy-" + channelType;
        GUCEF_TESTFW_TESTCASE( caseName5.C_String() )
        try
        {
            CLinkedPair pair;
            REQUIRE_TRUE( SetupPair( pair, "lossy-" + channelType, "?reorder=0.5", TestNodeSettings() ) );

            const UInt32 count = 300;
            CORE::CDynamicBuffer payload;
            for ( UInt32 i=0; i<count; ++i )
            {
                MakeCountedPayload( payload, i, 8 );
                ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_UNRELIABLE_SEQUENCED, DRN::DRN_MSGCLASS_APP, TestOpcode, payload ) );
                pair.linkA->Flush();
                if ( 0 == i % 20 )
                    PumpFor( 1 );
            }
            PumpFor( 300 );

            GUCEF::vector< UInt32 > counters;
            GetLaneCounters( pair.recorderB, DRN::DRN_LANE_UNRELIABLE_SEQUENCED, counters );
            bool increasing = !counters.empty();
            for ( UInt32 i=1; i<counters.size(); ++i )
                if ( counters[ i ] <= counters[ i-1 ] )
                    increasing = false;
            ASSERT_TRUE( increasing );
            UInt64 stale = pair.linkB->GetStats().staleMessages;
            ASSERT_TRUE( stale > 0 );
            ASSERT_TRUE( counters.size() + stale == count );
            ASSERT_TRUE( counters.size() > count / 4 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        GUCEF_TESTFW_TESTCASE_END
    }

    GUCEF_TESTFW_TESTCASE( "Links: Reliable lanes over udp through loss, reordering, duplicates and delay" )
    try
    {
        CLinkedPair pair;
        REQUIRE_TRUE( SetupPair( pair, "lossy-udp", "?loss=0.2&dup=0.1&reorder=0.2&delay=3&jitter=4", TestNodeSettings() ) );

        const UInt32 orderedCount = 400;
        const UInt32 unorderedCount = 300;
        CORE::CDynamicBuffer payload;
        for ( UInt32 i=0; i<orderedCount; ++i )
        {
            // every 50th message is larger than a datagram and travels in fragments
            MakeCountedPayload( payload, i, 0 == i % 50 ? 3000 + i : 8 + ( i % 300 ) );
            ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, payload ) );
            if ( i < unorderedCount )
            {
                MakeCountedPayload( payload, i, 20 );
                ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_UNORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, payload ) );
            }
            MakeCountedPayload( payload, i, 12 );
            ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_APP, TestOpcode, payload ) );
            if ( 0 == i % 5 )
            {
                pair.linkA->Flush();
                PumpFor( 1 );
            }
        }
        MakeCountedPayload( payload, 99999, 64 * 1024 );
        ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, payload ) );
        pair.linkA->Flush();

        CLaneCountCondition lane0( pair.recorderB, DRN::DRN_LANE_RELIABLE_ORDERED, orderedCount + 1 );
        CLaneCountCondition lane1( pair.recorderB, DRN::DRN_LANE_RELIABLE_UNORDERED, unorderedCount );
        ASSERT_TRUE( PumpUntil( lane0, 60000 ) );
        ASSERT_TRUE( PumpUntil( lane1, 60000 ) );
        PumpFor( 200 );

        GUCEF::vector< UInt32 > counters;
        GetLaneCounters( pair.recorderB, DRN::DRN_LANE_RELIABLE_ORDERED, counters );
        ASSERT_TRUE( orderedCount + 1 == counters.size() );
        bool inOrder = orderedCount + 1 == counters.size();
        for ( UInt32 i=0; inOrder && i<orderedCount; ++i )
            inOrder = counters[ i ] == i;
        ASSERT_TRUE( inOrder );
        if ( orderedCount + 1 == counters.size() )
            ASSERT_TRUE( 99999 == counters[ orderedCount ] );
        GetLaneCounters( pair.recorderB, DRN::DRN_LANE_RELIABLE_UNORDERED, counters );
        ASSERT_TRUE( IsCompleteSet( counters, unorderedCount ) );

        // the unreliable lane loses some, but never delivers twice or something broken
        GetLaneCounters( pair.recorderB, DRN::DRN_LANE_UNRELIABLE, counters );
        ASSERT_TRUE( counters.size() < orderedCount && counters.size() > orderedCount / 2 );
        GUCEF::vector< UInt8 > seen( orderedCount, 0 );
        bool unique = true;
        for ( UInt32 i=0; i<counters.size(); ++i )
        {
            if ( counters[ i ] >= orderedCount || 0 != seen[ counters[ i ] ] )
                unique = false;
            else
                seen[ counters[ i ] ] = 1;
        }
        ASSERT_TRUE( unique );

        const DRN::CDRNLinkStats& stats = pair.linkA->GetStats();
        ASSERT_TRUE( stats.messagesResent > 0 );
        ASSERT_TRUE( stats.fragmentedMessages > 0 );
        ASSERT_TRUE( pair.linkB->GetStats().packetsLost > 0 );
        ASSERT_TRUE( pair.linkB->GetStats().duplicatePackets > 0 );

        // everything sent reliably is acknowledged in the end
        PumpFor( 500 );
        ASSERT_TRUE( 0 == pair.linkA->GetUnackedCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Links: A burst on the reliable lanes three times the receive window, its first packet lost, arrives complete; the sender keeps no more than the window in flight per lane" )
    try
    {
        DRN::CDRNLinkReliability sender;
        DRN::CDRNLinkReliability receiver;
        DRN::CDRNLinkReliability::TPacketResult result = DRN::CDRNLinkReliability::PACKET_ACCEPTED;
        sender.Reset( false, DRN::DRN_DEFAULT_DATAGRAM_SIZE, 1 );
        receiver.Reset( false, DRN::DRN_DEFAULT_DATAGRAM_SIZE, 1 );

        const UInt32 count = 3 * DRN::DRN_RECEIVE_WINDOW_SIZE;
        CORE::CDynamicBuffer payload;
        for ( UInt32 i=0; i<count; ++i )
        {
            MakeCountedPayload( payload, i, 16 );
            REQUIRE_TRUE( sender.QueueMessage( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, payload.GetConstBufferPtr(), payload.GetDataSize(), GUCEF_NULL, 0 ) );
            REQUIRE_TRUE( sender.QueueMessage( DRN::DRN_LANE_RELIABLE_UNORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, payload.GetConstBufferPtr(), payload.GetDataSize(), GUCEF_NULL, 0 ) );
        }

        // the rounds of a link: what the sender sends goes to the receiver (round 0 loses its first packet), every
        // packet the receiver takes is acknowledged at once, and what is not acknowledged within the RTO goes again
        CReliabilitySink delivered;
        CReliabilitySink none;
        UInt64 now = 1000000;
        const UInt64 rto = 50000;
        UInt32 firstRoundLane0 = 0, firstRoundLane1 = 0, highestLane0 = 0, highestLane1 = 0;
        UInt32 refusedPackets = 0;
        GUCEF::vector< CORE::CDynamicBuffer > packets;
        UInt32 round = 0;
        for ( ; round<200; ++round )
        {
            packets.clear();
            const UInt8* packet = GUCEF_NULL;
            UInt32 packetSize = 0;
            while ( sender.BuildPacket( now, false, packet, packetSize ) )
            {
                packets.push_back( CORE::CDynamicBuffer() );
                packets.back().CopyFrom( packetSize, packet );
                if ( 0 == round )
                {
                    firstRoundLane0 += CountLaneMessages( packet, packetSize, DRN::DRN_LANE_RELIABLE_ORDERED, highestLane0 );
                    firstRoundLane1 += CountLaneMessages( packet, packetSize, DRN::DRN_LANE_RELIABLE_UNORDERED, highestLane1 );
                }
            }
            for ( UInt32 p=0; p<packets.size(); ++p )
            {
                if ( 0 == round && 0 == p )
                    continue;
                DRN::CDRNPacketHeader header;
                UInt32 lostBefore = 0;
                result = receiver.ProcessPacket( static_cast< const UInt8* >( packets[ p ].GetConstBufferPtr() ), packets[ p ].GetDataSize(), now, rto, delivered, header, lostBefore );
                if ( DRN::CDRNLinkReliability::PACKET_ACCEPTED != result && DRN::CDRNLinkReliability::PACKET_DUPLICATE != result )
                    ++refusedPackets;
                if ( receiver.BuildPacket( now, true, packet, packetSize ) )
                    sender.ProcessPacket( packet, packetSize, now, rto, none, header, lostBefore );
            }
            if ( delivered.lane0.size() >= count && delivered.lane1.size() >= count && 0 == sender.GetUnackedCount() )
                break;
            now += 2 * rto;
            sender.CheckRetransmits( now, rto );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Reliable burst of " << count << " per lane: " << round << " rounds, first round lane 0 " << firstRoundLane0 <<
                   " (up to lane sequence " << highestLane0 << "), lane 1 " << firstRoundLane1 << " (up to " << highestLane1 << "), delivered " <<
                   delivered.lane0.size() << " / " << delivered.lane1.size() << ", unacked " << sender.GetUnackedCount() << ", refused packets " << refusedPackets );

        ASSERT_TRUE( IsSequence( delivered.lane0, count ) );
        ASSERT_TRUE( IsCompleteSet( delivered.lane1, count ) );
        ASSERT_TRUE( 0 == sender.GetUnackedCount() );

        // a conforming receiver never has to refuse a packet of a conforming sender: the sender kept within the window
        ASSERT_TRUE( 0 == refusedPackets );
        ASSERT_TRUE( firstRoundLane0 <= DRN::DRN_RECEIVE_WINDOW_SIZE && highestLane0 < DRN::DRN_RECEIVE_WINDOW_SIZE );
        ASSERT_TRUE( firstRoundLane1 <= DRN::DRN_RECEIVE_WINDOW_SIZE && highestLane1 < DRN::DRN_RECEIVE_WINDOW_SIZE );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Links: A packet with a reliable message beyond what the receiving lane holds is not taken and not acknowledged" )
    try
    {
        for ( UInt8 lane=DRN::DRN_LANE_RELIABLE_ORDERED; lane<=DRN::DRN_LANE_RELIABLE_UNORDERED; ++lane )
        {
            DRN::CDRNLinkReliability receiver;
            receiver.Reset( false, DRN::DRN_DEFAULT_DATAGRAM_SIZE, 1 );
            CReliabilitySink delivered;
            CORE::CDynamicBuffer packet;
            DRN::CDRNPacketHeader header;
            UInt32 lostBefore = 0;

            // lane sequence 0 is the next one the lane expects: the window holds 0 to DRN_RECEIVE_WINDOW_SIZE - 1
            MakeReliablePacket( packet, 1, lane, DRN::DRN_RECEIVE_WINDOW_SIZE + 5, 1 );
            DRN::CDRNLinkReliability::TPacketResult result = receiver.ProcessPacket( static_cast< const UInt8* >( packet.GetConstBufferPtr() ), packet.GetDataSize(), 1000, 50000, delivered, header, lostBefore );
            ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_WINDOW_FULL == result );
            ASSERT_TRUE( 1 == receiver.GetStats().windowRefusedPackets );
            UInt32 ackSequence = 0, ackBits = 0;
            ASSERT_TRUE( GetNextAck( receiver, 2000, ackSequence, ackBits ) );
            ASSERT_TRUE( 0 == ackSequence && 0 == ackBits );

            // the last lane sequence of the window is taken (held back on lane 0), and so is its packet
            MakeReliablePacket( packet, 2, lane, DRN::DRN_RECEIVE_WINDOW_SIZE - 1, 2 );
            result = receiver.ProcessPacket( static_cast< const UInt8* >( packet.GetConstBufferPtr() ), packet.GetDataSize(), 3000, 50000, delivered, header, lostBefore );
            ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_ACCEPTED == result );
            MakeReliablePacket( packet, 3, lane, 0, 3 );
            result = receiver.ProcessPacket( static_cast< const UInt8* >( packet.GetConstBufferPtr() ), packet.GetDataSize(), 4000, 50000, delivered, header, lostBefore );
            ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_ACCEPTED == result );
            ASSERT_TRUE( GetNextAck( receiver, 5000, ackSequence, ackBits ) );
            ASSERT_TRUE( 3 == ackSequence );
            ASSERT_TRUE( 0x1 == ( ackBits & 0x3 ) );    // 2 taken, 1 not

            // the packet that was refused comes again once the window moved on: now it is taken
            MakeReliablePacket( packet, 1, lane, DRN::DRN_RECEIVE_WINDOW_SIZE + 5, 1 );
            result = receiver.ProcessPacket( static_cast< const UInt8* >( packet.GetConstBufferPtr() ), packet.GetDataSize(), 6000, 50000, delivered, header, lostBefore );
            ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_WINDOW_FULL == result );
            for ( UInt32 s=1; s<=6; ++s )
            {
                MakeReliablePacket( packet, 3 + s, lane, s, 100 + s );
                ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_ACCEPTED == receiver.ProcessPacket( static_cast< const UInt8* >( packet.GetConstBufferPtr() ), packet.GetDataSize(), 7000, 50000, delivered, header, lostBefore ) );
            }
            MakeReliablePacket( packet, 1, lane, DRN::DRN_RECEIVE_WINDOW_SIZE + 5, 1 );
            result = receiver.ProcessPacket( static_cast< const UInt8* >( packet.GetConstBufferPtr() ), packet.GetDataSize(), 8000, 50000, delivered, header, lostBefore );
            ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_ACCEPTED == result );
            if ( DRN::DRN_LANE_RELIABLE_ORDERED == lane )
                ASSERT_TRUE( 7 == delivered.lane0.size() );     // 0 to 6, the rest is held back in order
            else
                ASSERT_TRUE( 9 == delivered.lane1.size() );     // every one, as it came
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Links: A burst of reliable messages far beyond the receive window arrives complete over lossy-udp, in order on lane 0" )
    try
    {
        CLinkedPair pair;
        REQUIRE_TRUE( SetupPair( pair, "lossy-udp", "?loss=0.05&reorder=0.05", TestNodeSettings() ) );

        const UInt32 orderedCount = 6000;
        const UInt32 unorderedCount = 3000;
        CORE::CDynamicBuffer payload;
        for ( UInt32 i=0; i<orderedCount; ++i )
        {
            MakeCountedPayload( payload, i, 16 + ( i % 40 ) );
            ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, payload ) );
            if ( i < unorderedCount )
                ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_UNORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, payload ) );
        }
        pair.linkA->Flush();

        CLaneCountCondition lane0( pair.recorderB, DRN::DRN_LANE_RELIABLE_ORDERED, orderedCount );
        CLaneCountCondition lane1( pair.recorderB, DRN::DRN_LANE_RELIABLE_UNORDERED, unorderedCount );
        ASSERT_TRUE( PumpUntil( lane0, 120000 ) );
        ASSERT_TRUE( PumpUntil( lane1, 120000 ) );
        PumpFor( 200 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Reliable burst over lossy-udp: delivered " << lane0.Count() << " / " << lane1.Count() << ", resent " <<
                   pair.linkA->GetStats().messagesResent << ", unacked " << pair.linkA->GetUnackedCount() << ", refused for the window " <<
                   pair.linkB->GetStats().windowRefusedPackets );

        GUCEF::vector< UInt32 > counters;
        GetLaneCounters( pair.recorderB, DRN::DRN_LANE_RELIABLE_ORDERED, counters );
        ASSERT_TRUE( IsSequence( counters, orderedCount ) );
        GetLaneCounters( pair.recorderB, DRN::DRN_LANE_RELIABLE_UNORDERED, counters );
        ASSERT_TRUE( IsCompleteSet( counters, unorderedCount ) );
        ASSERT_TRUE( pair.linkA->GetStats().messagesResent > 0 );
        ASSERT_TRUE( 0 == pair.linkB->GetStats().windowRefusedPackets );
        ASSERT_TRUE( pair.linkA->IsOperational() && pair.linkB->IsOperational() );
        PumpFor( 500 );
        ASSERT_TRUE( 0 == pair.linkA->GetUnackedCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    for ( UInt32 t=0; t<ChannelTypeCount; ++t )
    {
        CORE::CString channelType = ChannelTypes[ t ];
        if ( channelType == "websocket" && !webSocketAvailable )
            continue;

        const CORE::CString caseName6 = "Links: Round trip time and clock offset within tolerance over a delaying lossy-" + channelType;
        GUCEF_TESTFW_TESTCASE( caseName6.C_String() )
        try
        {
            // 25 ms each way
            CLinkedPair pair;
            REQUIRE_TRUE( SetupPair( pair, "lossy-" + channelType, "?delay=25", TestNodeSettings() ) );

            PumpFor( 1500 );

            DRN::CDRNLatencyInfo infoA;
            DRN::CDRNLatencyInfo infoB;
            pair.linkA->GetLatencyInfo( infoA );
            pair.linkB->GetLatencyInfo( infoB );
            ASSERT_TRUE( infoA.sampleCount > 10 && infoB.sampleCount > 10 );
            ASSERT_TRUE( infoA.smoothedRtt >= 48000 && infoA.smoothedRtt <= 95000 );
            ASSERT_TRUE( infoB.smoothedRtt >= 48000 && infoB.smoothedRtt <= 95000 );
            ASSERT_TRUE( infoA.minRtt >= 48000 && infoA.minRtt <= infoA.smoothedRtt + 1000 );
            ASSERT_TRUE( infoA.hasClockOffset && infoB.hasClockOffset );

            // both nodes share the process clock: the remote time estimated for now is now. Leaving out the half
            // round trip would be 25 ms off. Over the stream channels the two directions of the sockets do not take
            // equally long (a few ms on loopback), which the estimate cannot see: it gets a wider tolerance there
            UInt64 now = DRN::CDRNClock::NowInMicroseconds();
            Int64 errorA = (Int64) pair.linkA->GetEstimatedRemoteTime( now ) - (Int64) now;
            Int64 errorB = (Int64) pair.linkB->GetEstimatedRemoteTime( now ) - (Int64) now;
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Latency over lossy-" << channelType << ": rtt " << infoA.smoothedRtt << " / " << infoB.smoothedRtt <<
                       " us, remote time error " << errorA << " / " << errorB << " us" );
            Int64 tolerance = channelType == "udp" ? 4000 : 15000;
            ASSERT_TRUE( errorA > -tolerance && errorA < tolerance );
            ASSERT_TRUE( errorB > -tolerance && errorB < tolerance );
            // the link clocks started at different times: the offsets mirror each other
            Int64 sum = infoA.clockOffset + infoB.clockOffset;
            ASSERT_TRUE( sum > -16000 && sum < 16000 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        GUCEF_TESTFW_TESTCASE_END
    }

    GUCEF_TESTFW_TESTCASE( "Links: Latency updates arrive as events at the set interval" )
    try
    {
        DRN::CDRNNodeSettings settings = TestNodeSettings();
        settings.latencyUpdateIntervalMs = 100;
        CLinkedPair pair;
        REQUIRE_TRUE( SetupPair( pair, "tcp", CORE::CString::Empty, settings ) );

        ASSERT_TRUE( PumpUntil( pair.recorderA.latencyEventCount, 3, 3000 ) );
        ASSERT_TRUE( PumpUntil( pair.recorderB.latencyEventCount, 3, 3000 ) );
        ASSERT_TRUE( pair.recorderA.lastLatencyInfo.sampleCount > 0 );
        ASSERT_TRUE( pair.recorderA.lastLatencyInfo.smoothedRtt < 100000 );
        ASSERT_TRUE( pair.recorderA.lastLatencyInfo.lossRatio == 0.0f );

        // about one per interval, not one per pulse
        UInt32 before = pair.recorderA.latencyEventCount;
        PumpFor( 1000 );
        UInt32 events = pair.recorderA.latencyEventCount - before;
        ASSERT_TRUE( events >= 6 && events <= 12 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Links: A link that hears nothing times out, echoes keep a quiet link alive" )
    try
    {
        DRN::CDRNNodeSettings settings = TestNodeSettings();
        settings.linkTimeoutMs = 400;
        CLinkedPair pair;
        REQUIRE_TRUE( SetupPair( pair, "udp", CORE::CString::Empty, settings ) );

        // quiet but echoing: stays up
        PumpFor( 1000 );
        ASSERT_TRUE( pair.linkA->IsOperational() && pair.linkB->IsOperational() );

        // the other end goes away without a word
        pair.nodeB.Stop();
        ASSERT_TRUE( PumpUntil( pair.recorderA.closedCount, 1, 3000 ) );
        ASSERT_TRUE( DRN::DRN_LINKREASON_LINK_TIMEOUT == pair.recorderA.lastClosedReason || DRN::DRN_LINKREASON_REMOTE_CLOSED == pair.recorderA.lastClosedReason );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Links: The connector tries the next endpoint when one does not answer" )
    try
    {
        DRN::CDRNNodeSettings settings = TestNodeSettings();
        settings.connectTimeoutMs = 500;
        DRN::CDRNNode nodeA;
        DRN::CDRNNode nodeB;
        ASSERT_TRUE( nodeA.SetSettings( settings ) && nodeB.SetSettings( settings ) );
        CLinkRecorder recorderA;
        recorderA.WatchNode( nodeA );
        UInt16 goodPort = NextTestPort();
        UInt16 deadPort = NextTestPort();
        ASSERT_TRUE( nodeB.Listen( LoopbackEndpoint( "tcp", goodPort ) ) );

        DRN::CDRNEndpointList endpoints;
        endpoints.Add( LoopbackEndpoint( "udp", deadPort ) );      // nobody listens there
        endpoints.Add( LoopbackEndpoint( "tcp", goodPort ) );
        DRN::CDRNConnectorPtr connector = nodeA.Connect( endpoints );
        REQUIRE_TRUE( PumpUntil( recorderA.operationalCount, 1, 5000 ) );
        ASSERT_TRUE( connector->IsDone() && connector->HasSucceeded() );
        ASSERT_TRUE( 2 == connector->GetAttemptCount() );
        ASSERT_TRUE( connector->GetCurrentEndpoint().GetChannelType() == "tcp" );
        ASSERT_FALSE( connector->GetLink().IsNULL() );
        recorderA.links.clear();
        connector.Unlink();
        nodeA.Stop();
        nodeB.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Links: Nodes on threads of their own stopped one after the other: a channel let go of on the stopping thread while its peer's node still runs (TSan)" )
    try
    {
        // the first node's links and channels go on the calling thread once its own thread ended (its channel reaper
        // has no owner thread any more); the other end of each channel belongs to the node that still runs. Under
        // ThreadSanitizer this finds what the two ends of a channel share without a lock
        for ( UInt32 round=0; round<3; ++round )
        {
            CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "stop-order", "mem" );
            group.nodeSettings.ownThread = true;
            REQUIRE_TRUE( group.Start( 3 ) );
            PumpFor( 50 );
            group.nodes[ 0 ]->Stop();
            PumpFor( 50 );
            group.nodes[ 1 ]->Stop();
            PumpFor( 20 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Links: Channels connecting over mem to a node on a thread of its own while it pulses: accepting never waits for that thread" )
    try
    {
        // the connecting thread accepts on behalf of the listener; the listener's thread looks for accepted channels
        // on every pulse. Many connects in a row find the listener's thread in its pulse.
        DRN::CDRNNodeSettings settings = TestNodeSettings();
        settings.ownThread = true;
        DRN::CDRNNode host;
        REQUIRE_TRUE( host.SetSettings( settings ) && host.IsRunningOwnThread() );
        DRN::CDRNEndpoint endpoint = LoopbackEndpoint( "mem", NextTestPort() );
        REQUIRE_TRUE( host.Listen( endpoint ) );
        GUCEF::vector< DRN::CIDRNChannelPtr > channels;
        UInt32 connected = 0;
        for ( UInt32 i=0; i<400; ++i )
        {
            DRN::CIDRNChannelPtr channel( GUCEF_NEW CDRNMemoryChannel() );
            if ( channel->Connect( endpoint ) )
                ++connected;
            channels.push_back( channel );
        }
        ASSERT_TRUE( 400 == connected );
        for ( UInt32 i=0; i<channels.size(); ++i )
            channels[ i ]->Close();
        channels.clear();
        host.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Links: The unfragmented payload of a link: the datagram budget on udp, the stream limit on stream channels" )
    try
    {
        for ( UInt32 t=0; t<ChannelTypeCount; ++t )
        {
            CORE::CString channelType = ChannelTypes[ t ];
            if ( channelType == "websocket" && !webSocketAvailable )
                continue;
            CLinkedPair pair;
            REQUIRE_TRUE( SetupPair( pair, channelType, CORE::CString::Empty, TestNodeSettings() ) );
            UInt32 payloadA = pair.linkA->GetMaxUnfragmentedPayload();
            UInt32 payloadB = pair.linkB->GetMaxUnfragmentedPayload();
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Unfragmented payload of a " << channelType << " link: " << payloadA );
            ASSERT_TRUE( payloadA == payloadB );
            if ( channelType == "udp" )
            {
                // a 1200 byte datagram less the tag of packet protection, the packet header and a message header
                ASSERT_TRUE( DRN::DRN_DEFAULT_DATAGRAM_SIZE - DRN::DRN_PACKET_PROTECTION_OVERHEAD - DRN::DRN_MAX_PACKET_HEADER_SIZE - DRN::DRN_MAX_MESSAGE_HEADER_SIZE == payloadA );
            }
            else
            {
                // the stream limit: a stream packet, bounded by the policy's largest message (16 MB by default)
                ASSERT_TRUE( DRN::DRN_MAX_STREAM_PACKET_SIZE - DRN::DRN_MAX_PACKET_HEADER_SIZE - DRN::DRN_MAX_MESSAGE_HEADER_SIZE == payloadA );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
