/*
 *  gucefPUBSUB_TestApp: GUCEF test application for the PUBSUB module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_PUBSUB_H
#include "gucefPUBSUB.h"
#define GUCEF_PUBSUB_H
#endif /* GUCEF_PUBSUB_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPubSubMsgContainerBinarySerializer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef PUBSUB::CPubSubMsgContainerBinarySerializer    TContainerSer;
typedef PUBSUB::CPubSubMsgBinarySerializerOptions      TSerOptions;
typedef PUBSUB::CBasicPubSubMsg::TBasicPubSubMsgVector TMsgVector;
typedef TContainerSer::TMsgOffsetIndex                 TOffsetIndex;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static TSerOptions
MakeFullOptions( void )
{GUCEF_TRACE;

    TSerOptions opts;
    opts.msgIdIncluded                      = true;
    opts.msgIndexIncluded                   = true;
    opts.msgDateTimeIncluded                = true;
    opts.msgDateTimeAsMsSinceUnixEpochInUtc = true;
    opts.msgPrimaryPayloadIncluded          = true;
    opts.msgKeyValuePairsIncluded           = true;
    opts.msgMetaDataKeyValuePairsIncluded   = true;
    return opts;
}

/*-------------------------------------------------------------------------*/

static void
PopulateMsgs( TMsgVector& msgs, CORE::UInt32 count, const CORE::CDateTime& dt )
{GUCEF_TRACE;

    static const CORE::CString payload = "TestPayload_ContainerTests";
    msgs.resize( count );
    for ( CORE::UInt32 i=0; i<count; ++i )
    {
        msgs[ i ].GetMsgId() = CORE::CVariant( CORE::ToString( i ).C_String() );
        msgs[ i ].GetMsgIndex() = (CORE::UInt64) i;
        msgs[ i ].GetMsgDateTime() = dt;
        msgs[ i ].GetPrimaryPayload().LinkTo( payload );
        msgs[ i ].AddKeyValuePair( CORE::CVariant( "k" ), CORE::CVariant( CORE::ToString( i ) ) );
    }
}

/*-------------------------------------------------------------------------*/

void
PerformPubSubMsgContainerBinarySerializerTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CPubSubMsgContainerBinarySerializer advanced TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CPubSubMsgContainerBinarySerializer_Advanced" );

    // Test 1: Header + footer explicit serialization/deserialization round-trip
    GUCEF_TESTFW_TESTCASE( "Test 1: SerializeHeader/DeserializeHeader round-trip" )
        try
        {
            TSerOptions opts = MakeFullOptions();
            CORE::CDynamicBuffer buf( true );
            UInt32 bytesWritten = 0;

            ASSERT_TRUE( TContainerSer::SerializeHeader( opts, 0, buf, bytesWritten ) );
            ASSERT_TRUE( bytesWritten > 0 );

            TSerOptions loaded;
            UInt32 bytesRead = 0;
            ASSERT_TRUE( TContainerSer::DeserializeHeader( loaded, buf, bytesRead ) );
            ASSERT_TRUE( bytesRead == bytesWritten );

            ASSERT_TRUE( loaded.msgIdIncluded                    == opts.msgIdIncluded );
            ASSERT_TRUE( loaded.msgIndexIncluded                 == opts.msgIndexIncluded );
            ASSERT_TRUE( loaded.msgDateTimeIncluded              == opts.msgDateTimeIncluded );
            ASSERT_TRUE( loaded.msgPrimaryPayloadIncluded        == opts.msgPrimaryPayloadIncluded );
            ASSERT_TRUE( loaded.msgKeyValuePairsIncluded         == opts.msgKeyValuePairsIncluded );
            ASSERT_TRUE( loaded.msgMetaDataKeyValuePairsIncluded == opts.msgMetaDataKeyValuePairsIncluded );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: SerializeFooter / DeserializeFooter round-trip
    GUCEF_TESTFW_TESTCASE( "Test 2: SerializeFooter/DeserializeFooter round-trip" )
        try
        {
            TOffsetIndex indexIn;
            indexIn.push_back( 10 );
            indexIn.push_back( 200 );
            indexIn.push_back( 450 );

            CORE::CDynamicBuffer buf( true );
            UInt32 bytesWritten = 0;
            ASSERT_TRUE( TContainerSer::SerializeFooter( indexIn, 0, buf, bytesWritten ) );
            ASSERT_TRUE( bytesWritten > 0 );

            TOffsetIndex indexOut;
            UInt32 bytesRead = 0;
            ASSERT_TRUE( TContainerSer::DeserializeFooter( indexOut, buf, bytesRead, true ) );

            ASSERT_TRUE( indexOut.size() == indexIn.size() );
            for ( size_t i=0; i<indexIn.size(); ++i )
            {
                ASSERT_TRUE( indexOut[ i ] == indexIn[ i ] );
            }
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: DeserializeMsgAtIndex — first message (fromStart=true, index=0)
    GUCEF_TESTFW_TESTCASE( "Test 3: DeserializeMsgAtIndex - first message by index from start" )
        try
        {
            CORE::CDateTime dt = CORE::CDateTime::NowLocalDateTime();
            TMsgVector msgs;
            PopulateMsgs( msgs, 5, dt );

            TSerOptions opts = MakeFullOptions();
            CORE::CDynamicBuffer buf( true );
            UInt32 bytesWritten = 0;
            ASSERT_TRUE( TContainerSer::Serialize( opts, msgs, 0, buf, bytesWritten ) );

            PUBSUB::CBasicPubSubMsg first;
            bool isCorrupted = false;
            ASSERT_TRUE( TContainerSer::DeserializeMsgAtIndex( first, true, buf, 0, true, isCorrupted ) );
            ASSERT_FALSE( isCorrupted );
            ASSERT_TRUE( first.GetMsgId().AsString() == "0" );
            ASSERT_TRUE( first.GetMsgIndex().AsUInt64() == (CORE::UInt64) 0 );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: DeserializeMsgAtIndex — last message (fromStart=false, index=0 means last)
    GUCEF_TESTFW_TESTCASE( "Test 4: DeserializeMsgAtIndex - last message by index from end" )
        try
        {
            CORE::CDateTime dt = CORE::CDateTime::NowLocalDateTime();
            TMsgVector msgs;
            PopulateMsgs( msgs, 5, dt );

            TSerOptions opts = MakeFullOptions();
            CORE::CDynamicBuffer buf( true );
            UInt32 bytesWritten = 0;
            ASSERT_TRUE( TContainerSer::Serialize( opts, msgs, 0, buf, bytesWritten ) );

            PUBSUB::CBasicPubSubMsg last;
            bool isCorrupted = false;
            ASSERT_TRUE( TContainerSer::DeserializeMsgAtIndex( last, true, buf, 0, false, isCorrupted ) );
            ASSERT_FALSE( isCorrupted );
            ASSERT_TRUE( last.GetMsgIndex().AsUInt64() == (CORE::UInt64) 4 );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: DeserializeFirstAndLastMsgDateTime — timestamps match original
    GUCEF_TESTFW_TESTCASE( "Test 5: DeserializeFirstAndLastMsgDateTime matches original timestamps" )
        try
        {
            CORE::CDateTime firstDt = CORE::CDateTime::NowLocalDateTime();
            CORE::CDateTime lastDt  = firstDt;

            TMsgVector msgs;
            msgs.resize( 3 );
            msgs[ 0 ].GetMsgDateTime() = firstDt;
            msgs[ 0 ].GetPrimaryPayload() = CORE::CVariant( "p0" );
            msgs[ 1 ].GetMsgDateTime() = firstDt;
            msgs[ 1 ].GetPrimaryPayload() = CORE::CVariant( "p1" );
            msgs[ 2 ].GetMsgDateTime() = lastDt;
            msgs[ 2 ].GetPrimaryPayload() = CORE::CVariant( "p2" );

            TSerOptions opts = MakeFullOptions();
            CORE::CDynamicBuffer buf( true );
            UInt32 bytesWritten = 0;
            ASSERT_TRUE( TContainerSer::Serialize( opts, msgs, 0, buf, bytesWritten ) );

            CORE::CDateTime readFirst, readLast;
            bool isSupported = false, isCorrupted = false;
            ASSERT_TRUE( TContainerSer::DeserializeFirstAndLastMsgDateTime( readFirst, readLast, buf, isSupported, isCorrupted ) );
            ASSERT_FALSE( isCorrupted );

            if ( isSupported )
            {
                ASSERT_TRUE( readFirst == firstDt );
                ASSERT_TRUE( readLast  == lastDt );
            }
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: IndexRebuildScan on valid serialized data produces correct index
    GUCEF_TESTFW_TESTCASE( "Test 6: IndexRebuildScan on valid data produces matching index" )
        try
        {
            CORE::CDateTime dt = CORE::CDateTime::NowLocalDateTime();
            TMsgVector msgs;
            PopulateMsgs( msgs, 8, dt );

            TSerOptions opts = MakeFullOptions();
            CORE::CDynamicBuffer buf( true );
            UInt32 bytesWritten = 0;
            ASSERT_TRUE( TContainerSer::Serialize( opts, msgs, 0, buf, bytesWritten ) );

            // Get the original footer index
            TOffsetIndex originalIndex;
            bool isCorrupted = false;
            TMsgVector dummy;
            ASSERT_TRUE( TContainerSer::DeserializeWithRebuild( dummy, false, originalIndex, buf, isCorrupted, false ) );
            ASSERT_FALSE( isCorrupted );
            ASSERT_TRUE( originalIndex.size() == msgs.size() );

            // Rebuild from a buffer that has the header but we scan forward
            TOffsetIndex rebuiltIndex;
            UInt32 bytesRead = 0;
            ASSERT_TRUE( TContainerSer::IndexRebuildScan( rebuiltIndex, buf, bytesRead, true ) );
            ASSERT_TRUE( rebuiltIndex.size() == msgs.size() );

            for ( size_t i=0; i<originalIndex.size(); ++i )
            {
                ASSERT_TRUE( rebuiltIndex[ i ] == originalIndex[ i ] );
            }
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: DeserializeWithRebuild on file with truncated footer (partial write simulation)
    GUCEF_TESTFW_TESTCASE( "Test 7: DeserializeWithRebuild handles truncated footer" )
        try
        {
            CORE::CDateTime dt = CORE::CDateTime::NowLocalDateTime();
            TMsgVector msgs;
            PopulateMsgs( msgs, 5, dt );

            TSerOptions opts = MakeFullOptions();
            CORE::CDynamicBuffer fullBuf( true );
            UInt32 bytesWritten = 0;
            ASSERT_TRUE( TContainerSer::Serialize( opts, msgs, 0, fullBuf, bytesWritten ) );

            // Truncate: keep only the header + messages but chop the footer by removing last 20 bytes
            CORE::UInt32 truncSize = fullBuf.GetDataSize() > 20 ? fullBuf.GetDataSize() - 20 : fullBuf.GetDataSize();
            CORE::CDynamicBuffer truncBuf( true );
            truncBuf.CopyFrom( 0, truncSize, fullBuf.GetConstBufferPtr() );

            TMsgVector recovered;
            bool isCorrupted = false;
            // bestEffortIsOk=true: recover whatever messages are intact
            bool result = TContainerSer::DeserializeWithRebuild( recovered, true, truncBuf, isCorrupted, true );
            // We may or may not get all 5 messages back depending on truncation point,
            // but the call should succeed and we should recover at least some messages.
            ASSERT_TRUE( result || recovered.size() > 0 );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 8: DeserializeWithRebuild on file with corrupted footer — valid messages still recovered
    GUCEF_TESTFW_TESTCASE( "Test 8: DeserializeWithRebuild recovers valid msgs with corrupted footer" )
        try
        {
            CORE::CDateTime dt = CORE::CDateTime::NowLocalDateTime();
            TMsgVector msgs;
            PopulateMsgs( msgs, 6, dt );

            TSerOptions opts = MakeFullOptions();
            CORE::CDynamicBuffer buf( true );
            UInt32 bytesWritten = 0;
            ASSERT_TRUE( TContainerSer::Serialize( opts, msgs, 0, buf, bytesWritten ) );

            // Corrupt the last 10 bytes of the footer (keep messages intact)
            if ( buf.GetDataSize() > 10 )
            {
                UInt8* raw = static_cast< UInt8* >( buf.GetBufferPtr() );
                CORE::UInt32 sz = buf.GetDataSize();
                for ( CORE::UInt32 i = sz - 10; i < sz; ++i )
                    raw[ i ] = 0xAB;
            }

            TMsgVector recovered;
            bool isCorrupted = false;
            bool result = TContainerSer::DeserializeWithRebuild( recovered, true, buf, isCorrupted, true );
            // Should succeed (best-effort) and recover all 6 messages
            ASSERT_TRUE( result );
            ASSERT_TRUE( recovered.size() == msgs.size() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 9: CPubSubMsgBinarySerializerOptions bitmask round-trip
    GUCEF_TESTFW_TESTCASE( "Test 9: CPubSubMsgBinarySerializerOptions bitmask ToOptionsBitMask/FromOptionsBitMask" )
        try
        {
            TSerOptions original = MakeFullOptions();
            UInt32 bitmask = original.ToOptionsBitMask();

            TSerOptions loaded;
            loaded.FromOptionsBitMask( bitmask );

            ASSERT_TRUE( loaded.msgIdIncluded                    == original.msgIdIncluded );
            ASSERT_TRUE( loaded.msgIndexIncluded                 == original.msgIndexIncluded );
            ASSERT_TRUE( loaded.msgDateTimeIncluded              == original.msgDateTimeIncluded );
            ASSERT_TRUE( loaded.msgDateTimeAsMsSinceUnixEpochInUtc == original.msgDateTimeAsMsSinceUnixEpochInUtc );
            ASSERT_TRUE( loaded.msgPrimaryPayloadIncluded        == original.msgPrimaryPayloadIncluded );
            ASSERT_TRUE( loaded.msgKeyValuePairsIncluded         == original.msgKeyValuePairsIncluded );
            ASSERT_TRUE( loaded.msgMetaDataKeyValuePairsIncluded == original.msgMetaDataKeyValuePairsIncluded );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 10: CPubSubMsgBinarySerializerOptions SaveConfig/LoadConfig round-trip
    GUCEF_TESTFW_TESTCASE( "Test 10: CPubSubMsgBinarySerializerOptions SaveConfig/LoadConfig round-trip" )
        try
        {
            TSerOptions original = MakeFullOptions();
            CORE::CDataNode cfg;
            ASSERT_TRUE( original.SaveConfig( cfg ) );

            TSerOptions loaded;
            ASSERT_TRUE( loaded.LoadConfig( cfg ) );

            ASSERT_TRUE( loaded.msgIdIncluded                    == original.msgIdIncluded );
            ASSERT_TRUE( loaded.msgPrimaryPayloadIncluded        == original.msgPrimaryPayloadIncluded );
            ASSERT_TRUE( loaded.msgKeyValuePairsIncluded         == original.msgKeyValuePairsIncluded );
            ASSERT_TRUE( loaded.msgMetaDataKeyValuePairsIncluded == original.msgMetaDataKeyValuePairsIncluded );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 11: SerializeKvPairs / DeserializeKvPairs round-trip
    GUCEF_TESTFW_TESTCASE( "Test 11: SerializeKvPairs / DeserializeKvPairs round-trip" )
        try
        {
            PUBSUB::CIPubSubMsg::TKeyValuePairs kvPairs;
            PUBSUB::CIPubSubMsg::TKeyValuePair p1, p2;
            static const CORE::CString k1 = "key-one";
            static const CORE::CString v1 = "val-one";
            static const CORE::CString k2 = "key-two";
            static const CORE::CString v2 = "val-two";
            p1.first.LinkTo( k1 ); p1.second.LinkTo( v1 );
            p2.first.LinkTo( k2 ); p2.second.LinkTo( v2 );
            kvPairs.push_back( p1 );
            kvPairs.push_back( p2 );

            CORE::CDynamicBuffer buf( true );
            UInt32 bytesWritten = 0;
            ASSERT_TRUE( PUBSUB::CPubSubMsgBinarySerializer::SerializeKvPairs( kvPairs, 0, buf, bytesWritten ) );
            ASSERT_TRUE( bytesWritten > 0 );

            PUBSUB::CIPubSubMsg::TKeyValuePairs kvOut;
            UInt32 bytesRead = 0;
            ASSERT_TRUE( PUBSUB::CPubSubMsgBinarySerializer::DeserializeKvPairs( false, kvOut, 0, buf, bytesRead ) );
            ASSERT_TRUE( bytesRead == bytesWritten );
            ASSERT_TRUE( kvOut.size() == 2 );
            ASSERT_TRUE( kvOut[ 0 ].first.AsString()  == k1 );
            ASSERT_TRUE( kvOut[ 0 ].second.AsString() == v1 );
            ASSERT_TRUE( kvOut[ 1 ].first.AsString()  == k2 );
            ASSERT_TRUE( kvOut[ 1 ].second.AsString() == v2 );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CPubSubMsgContainerBinarySerializer advanced TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
