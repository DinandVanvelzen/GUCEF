/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */
 
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"          /* GUCEF CORE library API */
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_PUBSUB_H
#include "gucefPUBSUB.h"       /* GUCEF PUBSUB library API */
#define GUCEF_PUBSUB_H
#endif /* GUCEF_PUBSUB_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#if GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX || GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID
  #define DEBUGBREAK __builtin_trap()
#elif GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN
  #define DEBUGBREAK DebugBreak()
#else
  #define DEBUGBREAK
#endif

#define ERRORHERE { std::cout << "Test failed @ " << __FILE__ << "(" << __LINE__ << ")\n"; DEBUGBREAK; }
#define ASSERT_TRUE( test ) if ( !(test) ) { ERRORHERE; } 
#define ASSERT_FALSE( test ) if ( (test) ) { ERRORHERE; }

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

typedef GUCEF::vector< CORE::CDynamicBuffer >       TDynamicBufferVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const CORE::CString testPlayload1 = "PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST_PAYLOAD_THIS_IS_A_TEST";
static const CORE::CString testKVKey1 = "KV_KEY1";
static const CORE::CString testKVValue1 = "KV_VALUE1";
static const CORE::CString testKVKey2 = "KV_KEY2";
static const CORE::CString testKVValue2 = "KV_VALUE2";
static const CORE::CString testMsgId1 = "MSG_ID_1";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
GenerateTestBinaryPayload( CORE::CDynamicBuffer& buffer, UInt32 startNum, UInt32 endNum )
{GUCEF_TRACE;

    for ( UInt32 i=startNum; i<endNum; ++i )
    {
        buffer.AppendValue( i );
    }    
}

/*-------------------------------------------------------------------------*/

void
TestWithSimilarMessagesWithFixedSizes( void )
{GUCEF_TRACE;

    try
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Running PerformPubSubMsgBinaryStorageTest - TestWithSimilarMessagesWithFixedSizes" );
        
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
            msg.GetPrimaryPayload().LinkTo( testPlayload1 );
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
            PUBSUB::CBasicPubSubMsg& msg = msgs2[ i ];
            ASSERT_TRUE( msgs[ i ] == msgs2[ i ] );
            ASSERT_TRUE( msg.GetPrimaryPayload() == testPlayload1 );
            ASSERT_TRUE( msg.GetMsgDateTime() == testStart );
            ASSERT_TRUE( msg.GetMsgIndex() == i );
            ASSERT_TRUE( msg.GetMsgId() == testMsgId1 );
            
            COMCORE::CIMessage::TKeyValuePairs& pairs = msg.GetKeyValuePairs();
            for ( UInt32 n=0; n<2; ++n )
            {
                ASSERT_TRUE( msgs[ i ].GetKeyValuePairs()[ n ] == msgs2[ i ].GetKeyValuePairs()[ n ] );
            }
        }

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Finished running PerformPubSubMsgBinaryStorageTest - TestWithSimilarMessagesWithFixedSizes" );
    }
    catch ( CORE::CMsgException& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "unhandled GUCEF exception during PubSubMsgBinaryStorage test: " ) + e.what() );
        ERRORHERE;
    }
    catch ( std::exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "unhandled std exception during PubSubMsgBinaryStorage test: " ) + e.what() );
        ERRORHERE;
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "unhandled exception during PubSubMsgBinaryStorage test" );
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
TestWithSimilarMessagesWithVariableSizes( void )
{GUCEF_TRACE;

    try
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Running PerformPubSubMsgBinaryStorageTest - TestWithSimilarMessagesWithVariableSizes" );
        
        CORE::CDateTime testStart = CORE::CDateTime::NowLocalDateTime();
        
        CORE::CDynamicBuffer testPlayload2;
        GenerateTestBinaryPayload( testPlayload2, 0, 100 );

        PUBSUB::CIPubSubMsg::TKeyValuePair pubsubKvPair1;
        pubsubKvPair1.first.LinkTo( testKVKey1 );
        pubsubKvPair1.second.LinkTo( testKVValue1 );

        PUBSUB::CIPubSubMsg::TKeyValuePair pubsubKvPair2;
        pubsubKvPair2.first.LinkTo( testKVKey2 );
        pubsubKvPair2.second.LinkTo( testKVValue2 );

        CORE::CDynamicBuffer inMemStorage1( true );
        CORE::CDynamicBuffer inMemStorage2( true );

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
            msg.GetPrimaryPayload().LinkTo( testPlayload1 );
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
            PUBSUB::CBasicPubSubMsg& msg = msgs2[ i ];
            ASSERT_TRUE( msgs[ i ] == msgs2[ i ] );
            ASSERT_TRUE( msg.GetPrimaryPayload() == testPlayload1 );
            ASSERT_TRUE( msg.GetMsgDateTime() == testStart );
            ASSERT_TRUE( msg.GetMsgIndex() == i );
            ASSERT_TRUE( msg.GetMsgId() == testMsgId1 );
            
            COMCORE::CIMessage::TKeyValuePairs& pairs = msg.GetKeyValuePairs();
            for ( UInt32 n=0; n<2; ++n )
            {
                ASSERT_TRUE( msgs[ i ].GetKeyValuePairs()[ n ] == msgs2[ i ].GetKeyValuePairs()[ n ] );
            }
        }

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Finished running PerformPubSubMsgBinaryStorageTest - TestWithSimilarMessagesWithVariableSizes" );
    }
    catch ( CORE::CMsgException& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "unhandled GUCEF exception during PubSubMsgBinaryStorage TestWithSimilarMessagesWithVariableSizes test: " ) + e.what() );
        ERRORHERE;
    }
    catch ( std::exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "unhandled std exception during PubSubMsgBinaryStorage TestWithSimilarMessagesWithVariableSizes test: " ) + e.what() );
        ERRORHERE;
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "unhandled exception during PubSubMsgBinaryStorage TestWithSimilarMessagesWithVariableSizes test" );
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformPubSubMsgBinaryStorageTest( void )
{GUCEF_TRACE;

    try
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Running PerformPubSubMsgBinaryStorageTest with PubSubMsgContainerBinarySerializer with format version: " + CORE::ToString( PUBSUB::CPubSubMsgContainerBinarySerializer::CurrentFormatVersion ) );
        
        TestWithSimilarMessagesWithFixedSizes();
        TestWithSimilarMessagesWithVariableSizes();

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Finished running PerformPubSubMsgBinaryStorageTest with PubSubMsgContainerBinarySerializer with format version: " + CORE::ToString( PUBSUB::CPubSubMsgContainerBinarySerializer::CurrentFormatVersion ) );
    }
    catch ( CORE::CMsgException& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "unhandled GUCEF exception during PubSubMsgBinaryStorage test: " ) + e.what() );
        ERRORHERE;
    }
    catch ( std::exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "unhandled std exception during PubSubMsgBinaryStorage test: " ) + e.what() );
        ERRORHERE;
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "unhandled exception during PubSubMsgBinaryStorage test" );
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/