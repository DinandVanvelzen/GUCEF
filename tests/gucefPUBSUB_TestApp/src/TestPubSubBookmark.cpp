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

#include "TestPubSubBookmark.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPubSubBookmarkTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CPubSubBookmark TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CPubSubBookmark" );

    // Test 1: Default construction yields NOT_INITIALIZED type
    GUCEF_TESTFW_TESTCASE( "Test 1: Default construction yields BOOKMARK_TYPE_NOT_INITIALIZED" )
        try
        {
            PUBSUB::CPubSubBookmark bm;
            ASSERT_TRUE( bm.GetBookmarkType() == PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_INITIALIZED );
            ASSERT_TRUE( bm.GetBookmarkData().IsNULLOrEmpty() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: Type-only construction
    GUCEF_TESTFW_TESTCASE( "Test 2: Type-only construction stores type correctly" )
        try
        {
            PUBSUB::CPubSubBookmark bm( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_INDEX );
            ASSERT_TRUE( bm.GetBookmarkType() == PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_INDEX );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: Type + data construction with string data
    GUCEF_TESTFW_TESTCASE( "Test 3: Type+data construction with string data" )
        try
        {
            CORE::CVariant data( "my-msg-id-123" );
            PUBSUB::CPubSubBookmark bm( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_ID, data );
            ASSERT_TRUE( bm.GetBookmarkType() == PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_ID );
            ASSERT_TRUE( bm.GetBookmarkData().AsString() == "my-msg-id-123" );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: Type + data construction with integer data (msg index)
    GUCEF_TESTFW_TESTCASE( "Test 4: Type+data construction with integer data" )
        try
        {
            CORE::CVariant data( (CORE::UInt64) 42 );
            PUBSUB::CPubSubBookmark bm( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_INDEX, data );
            ASSERT_TRUE( bm.GetBookmarkType() == PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_INDEX );
            ASSERT_TRUE( bm.GetBookmarkData().AsUInt64() == (CORE::UInt64) 42 );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: Type + data + datetime construction
    GUCEF_TESTFW_TESTCASE( "Test 5: Type+data+datetime construction" )
        try
        {
            CORE::CVariant data( (CORE::UInt64) 7 );
            CORE::CDateTime now = CORE::CDateTime::NowLocalDateTime();
            PUBSUB::CPubSubBookmark bm( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_DATETIME, data, now );
            ASSERT_TRUE( bm.GetBookmarkType() == PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_DATETIME );
            ASSERT_TRUE( bm.GetBookmarkDateTime() == now );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: SetBookmarkType / GetBookmarkType round-trip for all types
    GUCEF_TESTFW_TESTCASE( "Test 6: SetBookmarkType/GetBookmarkType for all enum values" )
        try
        {
            PUBSUB::CPubSubBookmark bm;

            bm.SetBookmarkType( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_AVAILABLE );
            ASSERT_TRUE( bm.GetBookmarkType() == PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_AVAILABLE );

            bm.SetBookmarkType( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_APPLICABLE );
            ASSERT_TRUE( bm.GetBookmarkType() == PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_APPLICABLE );

            bm.SetBookmarkType( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_ID );
            ASSERT_TRUE( bm.GetBookmarkType() == PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_ID );

            bm.SetBookmarkType( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_INDEX );
            ASSERT_TRUE( bm.GetBookmarkType() == PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_INDEX );

            bm.SetBookmarkType( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_DATETIME );
            ASSERT_TRUE( bm.GetBookmarkType() == PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_DATETIME );

            bm.SetBookmarkType( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_TOPIC_INDEX );
            ASSERT_TRUE( bm.GetBookmarkType() == PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_TOPIC_INDEX );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: SetBookmarkData / GetBookmarkData
    GUCEF_TESTFW_TESTCASE( "Test 7: SetBookmarkData / GetBookmarkData round-trip" )
        try
        {
            PUBSUB::CPubSubBookmark bm( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_ID );
            CORE::CVariant v( "topic-offset-99" );
            bm.SetBookmarkData( v );
            ASSERT_TRUE( bm.GetBookmarkData().AsString() == "topic-offset-99" );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 8: Assignment operator preserves all fields
    GUCEF_TESTFW_TESTCASE( "Test 8: Assignment operator preserves type, data, and datetime" )
        try
        {
            CORE::CDateTime now = CORE::CDateTime::NowLocalDateTime();
            CORE::CVariant data( "bm-data" );
            PUBSUB::CPubSubBookmark src( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_ID, data, now );

            PUBSUB::CPubSubBookmark dst;
            dst = src;

            ASSERT_TRUE( dst.GetBookmarkType() == PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_ID );
            ASSERT_TRUE( dst.GetBookmarkData().AsString() == "bm-data" );
            ASSERT_TRUE( dst.GetBookmarkDateTime() == now );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 9: LinkTo shares data reference (no copy)
    GUCEF_TESTFW_TESTCASE( "Test 9: LinkTo produces equivalent bookmark" )
        try
        {
            CORE::CVariant data( "linked-data" );
            PUBSUB::CPubSubBookmark src( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_ID, data );

            PUBSUB::CPubSubBookmark dst;
            dst.LinkTo( src );

            ASSERT_TRUE( dst.GetBookmarkType() == PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_ID );
            ASSERT_TRUE( dst.GetBookmarkData().AsString() == "linked-data" );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 10: GetBookmarkTypeName returns non-empty string for valid types
    GUCEF_TESTFW_TESTCASE( "Test 10: GetBookmarkTypeName returns non-empty for valid types" )
        try
        {
            PUBSUB::CPubSubBookmark bm;
            bm.SetBookmarkType( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_ID );
            ASSERT_FALSE( bm.GetBookmarkTypeName().IsNULLOrEmpty() );

            bm.SetBookmarkType( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_INDEX );
            ASSERT_FALSE( bm.GetBookmarkTypeName().IsNULLOrEmpty() );

            bm.SetBookmarkType( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_INITIALIZED );
            ASSERT_FALSE( bm.GetBookmarkTypeName().IsNULLOrEmpty() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 11: ToString returns non-empty for initialized bookmark
    GUCEF_TESTFW_TESTCASE( "Test 11: ToString returns non-empty for initialized bookmark" )
        try
        {
            CORE::CVariant data( "test-msg-007" );
            PUBSUB::CPubSubBookmark bm( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_ID, data );
            ASSERT_FALSE( bm.ToString().IsNULLOrEmpty() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 12: Copy constructor via linkIfPossible=false does a deep copy
    GUCEF_TESTFW_TESTCASE( "Test 12: Copy constructor with linkIfPossible=false produces deep copy" )
        try
        {
            CORE::CVariant data( "deep-copy-data" );
            CORE::CDateTime now = CORE::CDateTime::NowLocalDateTime();
            PUBSUB::CPubSubBookmark src( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_ID, data, now );

            PUBSUB::CPubSubBookmark dst( src, false, now );
            ASSERT_TRUE( dst.GetBookmarkType() == PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_ID );
            ASSERT_TRUE( dst.GetBookmarkData().AsString() == "deep-copy-data" );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CPubSubBookmark TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
