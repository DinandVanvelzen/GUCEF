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

#include "TestPubSubMsgBinarySerializer.h"

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

typedef std::vector< CORE::CDynamicBuffer >         TDynamicBufferVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const CORE::CString testPayload1 = "PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST";
static const CORE::CString testKVKey1   = "KV_KEY1";
static const CORE::CString testKVValue1 = "KV_VALUE1";
static const CORE::CString testKVKey2   = "KV_KEY2";
static const CORE::CString testKVValue2 = "KV_VALUE2";
static const CORE::CString testMsgId1   = "MSG_ID_1";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
GenerateTestBinaryPayload( CORE::CDynamicBuffer& buffer, UInt32 startNum, UInt32 endNum )
{GUCEF_TRACE;

    for ( UInt32 i=startNum; i<endNum; ++i )
    {
        buffer.AppendValue( i );
    }
}

/*-------------------------------------------------------------------------*/

void
PerformPubSubMsgBinarySerializerTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CPubSubMsgContainerBinarySerializer TESTS with format version: " +
        CORE::ToString( PUBSUB::CPubSubMsgContainerBinarySerializer::CurrentFormatVersion ) );

    GUCEF_TESTFW_SUITE_SCOPE( "CPubSubMsgBinarySerializer" );

    // Test 1: Serialize and deserialize 100 similar messages with fixed sizes
    GUCEF_TESTFW_TESTCASE( "Test 1: Serialize/deserialize 100 similar msgs with fixed sizes" )
        try
        {
            CORE::CDateTime testStart = CORE::CDateTime::NowLocalDateTime();

            PUBSUB::CIPubSubMsg::TKeyValuePair pubsubKvPair1;
            pubsubKvPair1.first.LinkTo( testKVKey1 );
            pubsubKvPair1.second.LinkTo( testKVValue1 );

            PUBSUB::CIPubSubMsg::TKeyValuePair pubsubKvPair2;
            pubsubKvPair2.first.LinkTo( testKVKey2 );
            pubsubKvPair2.second.LinkTo( testKVValue2 );

            CORE::CDynamicBuffer inMemStorage1( true );

            PUBSUB::CPubSubMsgBinarySerializerOptions serializerOptions;
            serializerOptions.msgIdIncluded = true;
            serializerOptions.msgIndexIncluded = true;
            serializerOptions.msgDateTimeIncluded = true;
            serializerOptions.msgDateTimeAsMsSinceUnixEpochInUtc = true;
            serializerOptions.msgPrimaryPayloadIncluded = true;
            serializerOptions.msgKeyValuePairsIncluded = true;
            serializerOptions.msgMetaDataKeyValuePairsIncluded = true;

            PUBSUB::CBasicPubSubMsg::TBasicPubSubMsgVector msgs;
            msgs.resize( 100 );
            for ( UInt32 i=0; i<100; ++i )
            {
                PUBSUB::CBasicPubSubMsg& msg = msgs[ i ];
                msg.GetPrimaryPayload().LinkTo( testPayload1 );
                msg.GetMsgDateTime() = testStart;
                msg.GetMsgId().LinkTo( testMsgId1 );
                msg.GetMsgIndex() = i;
                msg.AddLinkedKeyValuePair( pubsubKvPair1 );
                msg.AddLinkedKeyValuePair( pubsubKvPair2 );
            }

            UInt32 bytesWritten = 0;
            ASSERT_TRUE( PUBSUB::CPubSubMsgContainerBinarySerializer::Serialize( serializerOptions, msgs, 0, inMemStorage1, bytesWritten ) );
            ASSERT_TRUE( bytesWritten == inMemStorage1.GetDataSize() );

            bool isCorrupted = false;
            PUBSUB::CPubSubMsgContainerBinarySerializer::TMsgOffsetIndex msgIndex;
            PUBSUB::CBasicPubSubMsg::TBasicPubSubMsgVector msgs2;
            ASSERT_TRUE( PUBSUB::CPubSubMsgContainerBinarySerializer::DeserializeWithRebuild( msgs2, true, msgIndex, inMemStorage1, isCorrupted, false ) );
            ASSERT_FALSE( isCorrupted );
            ASSERT_FALSE( msgIndex.size() != msgs.size() );

            for ( UInt32 i=0; i<100; ++i )
            {
                ASSERT_TRUE( msgs[ i ] == msgs2[ i ] );
                ASSERT_TRUE( msgs2[ i ].GetPrimaryPayload() == testPayload1 );
                ASSERT_TRUE( msgs2[ i ].GetMsgDateTime() == testStart );
                ASSERT_TRUE( msgs2[ i ].GetMsgIndex() == i );
                ASSERT_TRUE( msgs2[ i ].GetMsgId() == testMsgId1 );

                COMCORE::CIMessage::TKeyValuePairs& pairs = msgs2[ i ].GetKeyValuePairs();
                for ( UInt32 n=0; n<2; ++n )
                {
                    ASSERT_TRUE( msgs[ i ].GetKeyValuePairs()[ n ] == msgs2[ i ].GetKeyValuePairs()[ n ] );
                }
            }
        }
        catch ( CORE::CMsgException& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "unhandled GUCEF exception during test: " ) + e.what() );
            ERRORHERE;
        }
        catch ( std::exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "unhandled std exception during test: " ) + e.what() );
            ERRORHERE;
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: Serialize and deserialize 100 similar messages with variable sizes
    GUCEF_TESTFW_TESTCASE( "Test 2: Serialize/deserialize 100 similar msgs with variable sizes" )
        try
        {
            CORE::CDateTime testStart = CORE::CDateTime::NowLocalDateTime();

            CORE::CDynamicBuffer testPayload2;
            GenerateTestBinaryPayload( testPayload2, 0, 100 );

            PUBSUB::CIPubSubMsg::TKeyValuePair pubsubKvPair1;
            pubsubKvPair1.first.LinkTo( testKVKey1 );
            pubsubKvPair1.second.LinkTo( testKVValue1 );

            PUBSUB::CIPubSubMsg::TKeyValuePair pubsubKvPair2;
            pubsubKvPair2.first.LinkTo( testKVKey2 );
            pubsubKvPair2.second.LinkTo( testKVValue2 );

            CORE::CDynamicBuffer inMemStorage1( true );

            PUBSUB::CPubSubMsgBinarySerializerOptions serializerOptions;
            serializerOptions.msgIdIncluded = true;
            serializerOptions.msgIndexIncluded = true;
            serializerOptions.msgDateTimeIncluded = true;
            serializerOptions.msgDateTimeAsMsSinceUnixEpochInUtc = true;
            serializerOptions.msgPrimaryPayloadIncluded = true;
            serializerOptions.msgKeyValuePairsIncluded = true;
            serializerOptions.msgMetaDataKeyValuePairsIncluded = true;

            PUBSUB::CBasicPubSubMsg::TBasicPubSubMsgVector msgs;
            msgs.resize( 100 );
            for ( UInt32 i=0; i<100; ++i )
            {
                PUBSUB::CBasicPubSubMsg& msg = msgs[ i ];
                msg.GetPrimaryPayload().LinkTo( testPayload1 );
                msg.GetMsgDateTime() = testStart;
                msg.GetMsgId().LinkTo( testMsgId1 );
                msg.GetMsgIndex() = i;
                msg.AddLinkedKeyValuePair( pubsubKvPair1 );
                msg.AddLinkedKeyValuePair( pubsubKvPair2 );
            }

            UInt32 bytesWritten = 0;
            ASSERT_TRUE( PUBSUB::CPubSubMsgContainerBinarySerializer::Serialize( serializerOptions, msgs, 0, inMemStorage1, bytesWritten ) );
            ASSERT_TRUE( bytesWritten == inMemStorage1.GetDataSize() );

            bool isCorrupted = false;
            PUBSUB::CPubSubMsgContainerBinarySerializer::TMsgOffsetIndex msgIndex;
            PUBSUB::CBasicPubSubMsg::TBasicPubSubMsgVector msgs2;
            ASSERT_TRUE( PUBSUB::CPubSubMsgContainerBinarySerializer::DeserializeWithRebuild( msgs2, true, msgIndex, inMemStorage1, isCorrupted, false ) );
            ASSERT_FALSE( isCorrupted );
            ASSERT_FALSE( msgIndex.size() != msgs.size() );

            for ( UInt32 i=0; i<100; ++i )
            {
                ASSERT_TRUE( msgs[ i ] == msgs2[ i ] );
                ASSERT_TRUE( msgs2[ i ].GetPrimaryPayload() == testPayload1 );
                ASSERT_TRUE( msgs2[ i ].GetMsgDateTime() == testStart );
                ASSERT_TRUE( msgs2[ i ].GetMsgIndex() == i );
                ASSERT_TRUE( msgs2[ i ].GetMsgId() == testMsgId1 );

                COMCORE::CIMessage::TKeyValuePairs& pairs = msgs2[ i ].GetKeyValuePairs();
                for ( UInt32 n=0; n<2; ++n )
                {
                    ASSERT_TRUE( msgs[ i ].GetKeyValuePairs()[ n ] == msgs2[ i ].GetKeyValuePairs()[ n ] );
                }
            }
        }
        catch ( CORE::CMsgException& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "unhandled GUCEF exception during test: " ) + e.what() );
            ERRORHERE;
        }
        catch ( std::exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "unhandled std exception during test: " ) + e.what() );
            ERRORHERE;
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: Serialization with only primary payload (minimal options)
    GUCEF_TESTFW_TESTCASE( "Test 3: Serialize/deserialize with minimal options (payload only)" )
        try
        {
            CORE::CDateTime testStart = CORE::CDateTime::NowLocalDateTime();

            CORE::CDynamicBuffer inMemStorage( true );

            PUBSUB::CPubSubMsgBinarySerializerOptions serializerOptions;
            serializerOptions.msgIdIncluded = false;
            serializerOptions.msgIndexIncluded = false;
            serializerOptions.msgDateTimeIncluded = false;
            serializerOptions.msgPrimaryPayloadIncluded = true;
            serializerOptions.msgKeyValuePairsIncluded = false;
            serializerOptions.msgMetaDataKeyValuePairsIncluded = false;

            PUBSUB::CBasicPubSubMsg::TBasicPubSubMsgVector msgs;
            msgs.resize( 10 );
            for ( UInt32 i=0; i<10; ++i )
            {
                msgs[ i ].GetPrimaryPayload().LinkTo( testPayload1 );
            }

            UInt32 bytesWritten = 0;
            ASSERT_TRUE( PUBSUB::CPubSubMsgContainerBinarySerializer::Serialize( serializerOptions, msgs, 0, inMemStorage, bytesWritten ) );
            ASSERT_TRUE( bytesWritten > 0 );
            ASSERT_TRUE( bytesWritten == inMemStorage.GetDataSize() );

            bool isCorrupted = false;
            PUBSUB::CPubSubMsgContainerBinarySerializer::TMsgOffsetIndex msgIndex;
            PUBSUB::CBasicPubSubMsg::TBasicPubSubMsgVector msgs2;
            ASSERT_TRUE( PUBSUB::CPubSubMsgContainerBinarySerializer::DeserializeWithRebuild( msgs2, true, msgIndex, inMemStorage, isCorrupted, false ) );
            ASSERT_FALSE( isCorrupted );
            ASSERT_TRUE( msgs2.size() == 10 );

            for ( UInt32 i=0; i<10; ++i )
            {
                ASSERT_TRUE( msgs2[ i ].GetPrimaryPayload() == testPayload1 );
            }
        }
        catch ( CORE::CMsgException& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "unhandled GUCEF exception during test: " ) + e.what() );
            ERRORHERE;
        }
        catch ( std::exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "unhandled std exception during test: " ) + e.what() );
            ERRORHERE;
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: Serialize empty message batch
    GUCEF_TESTFW_TESTCASE( "Test 4: Serialize/deserialize empty message batch" )
        try
        {
            CORE::CDynamicBuffer inMemStorage( true );

            PUBSUB::CPubSubMsgBinarySerializerOptions serializerOptions;
            serializerOptions.msgIdIncluded = true;
            serializerOptions.msgIndexIncluded = true;
            serializerOptions.msgDateTimeIncluded = true;
            serializerOptions.msgPrimaryPayloadIncluded = true;
            serializerOptions.msgKeyValuePairsIncluded = true;
            serializerOptions.msgMetaDataKeyValuePairsIncluded = true;

            PUBSUB::CBasicPubSubMsg::TBasicPubSubMsgVector msgs;

            UInt32 bytesWritten = 0;
            ASSERT_TRUE( PUBSUB::CPubSubMsgContainerBinarySerializer::Serialize( serializerOptions, msgs, 0, inMemStorage, bytesWritten ) );

            bool isCorrupted = false;
            PUBSUB::CPubSubMsgContainerBinarySerializer::TMsgOffsetIndex msgIndex;
            PUBSUB::CBasicPubSubMsg::TBasicPubSubMsgVector msgs2;
            ASSERT_TRUE( PUBSUB::CPubSubMsgContainerBinarySerializer::DeserializeWithRebuild( msgs2, true, msgIndex, inMemStorage, isCorrupted, false ) );
            ASSERT_FALSE( isCorrupted );
            ASSERT_TRUE( msgs2.size() == 0 );
        }
        catch ( CORE::CMsgException& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "unhandled GUCEF exception during test: " ) + e.what() );
            ERRORHERE;
        }
        catch ( std::exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "unhandled std exception during test: " ) + e.what() );
            ERRORHERE;
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CPubSubMsgContainerBinarySerializer TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
