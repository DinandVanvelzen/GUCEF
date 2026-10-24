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

        const CORE::CString caseName2 = "Links: Large messages over " + channelType;
        GUCEF_TESTFW_TESTCASE( caseName2.C_String() )
        try
        {
            CLinkedPair pair;
            REQUIRE_TRUE( SetupPair( pair, channelType, CORE::CString::Empty, TestNodeSettings() ) );

            // 10 MB on the stream channels, the datagram channel fragments a smaller one
            UInt32 size = channelType == "udp" ? 300 * 1024 : 10 * 1024 * 1024;
            CORE::CDynamicBuffer payload;
            MakeCountedPayload( payload, 4242, size );
            ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, payload ) );
            CORE::CDynamicBuffer small;
            MakeCountedPayload( small, 1, 10 );
            ASSERT_TRUE( pair.linkA->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, small ) );
            pair.linkA->Flush();

            ASSERT_TRUE( PumpUntil( pair.recorderB.messageCount, 2, 60000 ) );
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
}

/*-------------------------------------------------------------------------*/
