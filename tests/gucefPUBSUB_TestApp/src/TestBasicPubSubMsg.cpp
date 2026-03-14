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

#include "TestBasicPubSubMsg.h"

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
PerformBasicPubSubMsgTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CBasicPubSubMsg TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CBasicPubSubMsg" );

    // Test 1: Default construction — all fields empty
    GUCEF_TESTFW_TESTCASE( "Test 1: Default construction yields empty/zero state" )
        try
        {
            PUBSUB::CBasicPubSubMsg msg;
            ASSERT_TRUE( msg.GetMsgId().IsNULLOrEmpty() );
            ASSERT_TRUE( msg.GetMsgIndex().IsNULLOrEmpty() );
            ASSERT_TRUE( msg.GetPrimaryPayload().IsNULLOrEmpty() );
            ASSERT_TRUE( msg.GetKeyValuePairs().empty() );
            ASSERT_TRUE( msg.GetMetaDataKeyValuePairs().empty() );
            ASSERT_TRUE( msg.GetReceiveActionId() == 0 );
            ASSERT_TRUE( msg.GetOriginClientTopicName().IsNULLOrEmpty() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: String primary payload set and get
    GUCEF_TESTFW_TESTCASE( "Test 2: String primary payload set/get round-trip" )
        try
        {
            PUBSUB::CBasicPubSubMsg msg;
            static const CORE::CString payload = "Hello, PubSub!";
            msg.GetPrimaryPayload() = CORE::CVariant( payload.C_String() );
            ASSERT_TRUE( msg.GetPrimaryPayload().AsString() == payload );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: Binary payload stored via CDynamicBuffer
    GUCEF_TESTFW_TESTCASE( "Test 3: Binary payload via CDynamicBuffer stored and retrieved" )
        try
        {
            PUBSUB::CBasicPubSubMsg msg;
            CORE::CDynamicBuffer buf( true );
            for ( UInt32 i = 0; i < 16; ++i )
                buf.AppendValue( (UInt8) i );

            msg.GetPrimaryPayload().LinkTo( buf );
            ASSERT_FALSE( msg.GetPrimaryPayload().IsNULLOrEmpty() );
            ASSERT_TRUE( msg.GetPrimaryPayload().AsBuffer().GetDataSize() == 16 );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: GetMsgId / GetMsgIndex / GetMsgDateTime set and get
    GUCEF_TESTFW_TESTCASE( "Test 4: MsgId, MsgIndex and MsgDateTime set/get" )
        try
        {
            PUBSUB::CBasicPubSubMsg msg;
            msg.GetMsgId() = CORE::CVariant( "id-999" );
            msg.GetMsgIndex() = (CORE::UInt64) 12345;
            CORE::CDateTime now = CORE::CDateTime::NowLocalDateTime();
            msg.GetMsgDateTime() = now;

            ASSERT_TRUE( msg.GetMsgId().AsString() == "id-999" );
            ASSERT_TRUE( msg.GetMsgIndex().AsUInt64() == (CORE::UInt64) 12345 );
            ASSERT_TRUE( msg.GetMsgDateTime() == now );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: SetReceiveActionId / GetReceiveActionId
    GUCEF_TESTFW_TESTCASE( "Test 5: SetReceiveActionId / GetReceiveActionId round-trip" )
        try
        {
            PUBSUB::CBasicPubSubMsg msg;
            ASSERT_TRUE( msg.GetReceiveActionId() == 0 );
            msg.SetReceiveActionId( 77777 );
            ASSERT_TRUE( msg.GetReceiveActionId() == 77777 );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: SetOriginClientTopicName / GetOriginClientTopicName
    GUCEF_TESTFW_TESTCASE( "Test 6: SetOriginClientTopicName / GetOriginClientTopicName round-trip" )
        try
        {
            PUBSUB::CBasicPubSubMsg msg;
            ASSERT_TRUE( msg.GetOriginClientTopicName().IsNULLOrEmpty() );
            ASSERT_TRUE( msg.SetOriginClientTopicName( "my-topic" ) );
            ASSERT_TRUE( msg.GetOriginClientTopicName() == "my-topic" );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: AddKeyValuePair and enumeration
    GUCEF_TESTFW_TESTCASE( "Test 7: AddKeyValuePair stores and retrieves pairs" )
        try
        {
            PUBSUB::CBasicPubSubMsg msg;
            ASSERT_TRUE( msg.AddKeyValuePair( CORE::CVariant( "key1" ), CORE::CVariant( "value1" ) ) );
            ASSERT_TRUE( msg.AddKeyValuePair( CORE::CVariant( "key2" ), CORE::CVariant( "value2" ) ) );
            ASSERT_TRUE( msg.GetKeyValuePairs().size() == 2 );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 8: AddMetaDataKeyValuePair and enumeration
    GUCEF_TESTFW_TESTCASE( "Test 8: AddMetaDataKeyValuePair stores and retrieves pairs" )
        try
        {
            PUBSUB::CBasicPubSubMsg msg;
            ASSERT_TRUE( msg.AddMetaDataKeyValuePair( CORE::CVariant( "meta-key" ), CORE::CVariant( "meta-value" ) ) );
            ASSERT_TRUE( msg.GetMetaDataKeyValuePairs().size() == 1 );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 9: AddLinkedKeyValuePair (string+string) — linked variant should compare equal
    GUCEF_TESTFW_TESTCASE( "Test 9: AddLinkedKeyValuePair with string+string" )
        try
        {
            static const CORE::CString key   = "linked-key";
            static const CORE::CString value = "linked-value";

            PUBSUB::CBasicPubSubMsg msg;
            ASSERT_TRUE( msg.AddLinkedKeyValuePair( key, value ) );
            ASSERT_TRUE( msg.GetKeyValuePairs().size() == 1 );
            ASSERT_TRUE( msg.GetKeyValuePairs()[ 0 ].first.AsString()  == key );
            ASSERT_TRUE( msg.GetKeyValuePairs()[ 0 ].second.AsString() == value );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 10: AddLinkedKeyValuePair (CVariant+CVariant)
    GUCEF_TESTFW_TESTCASE( "Test 10: AddLinkedKeyValuePair with CVariant+CVariant" )
        try
        {
            CORE::CVariant key( "vk" );
            CORE::CVariant val( "vv" );
            PUBSUB::CBasicPubSubMsg msg;
            ASSERT_TRUE( msg.AddLinkedKeyValuePair( key, val ) );
            ASSERT_TRUE( msg.GetKeyValuePairs().size() == 1 );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 11: AddLinkedMetaDataKeyValuePair (string+string)
    GUCEF_TESTFW_TESTCASE( "Test 11: AddLinkedMetaDataKeyValuePair with string+string" )
        try
        {
            static const CORE::CString mkey   = "mk";
            static const CORE::CString mvalue = "mv";
            PUBSUB::CBasicPubSubMsg msg;
            ASSERT_TRUE( msg.AddLinkedMetaDataKeyValuePair( mkey, mvalue ) );
            ASSERT_TRUE( msg.GetMetaDataKeyValuePairs().size() == 1 );
            ASSERT_TRUE( msg.GetMetaDataKeyValuePairs()[ 0 ].first.AsString()  == mkey );
            ASSERT_TRUE( msg.GetMetaDataKeyValuePairs()[ 0 ].second.AsString() == mvalue );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 12: Clear() resets all fields
    GUCEF_TESTFW_TESTCASE( "Test 12: Clear() resets message to empty state" )
        try
        {
            PUBSUB::CBasicPubSubMsg msg;
            msg.GetMsgId() = CORE::CVariant( "some-id" );
            msg.GetPrimaryPayload() = CORE::CVariant( "some-payload" );
            msg.AddKeyValuePair( CORE::CVariant( "k" ), CORE::CVariant( "v" ) );
            msg.SetReceiveActionId( 42 );

            msg.Clear();

            ASSERT_TRUE( msg.GetMsgId().IsNULLOrEmpty() );
            ASSERT_TRUE( msg.GetPrimaryPayload().IsNULLOrEmpty() );
            ASSERT_TRUE( msg.GetKeyValuePairs().empty() );
            ASSERT_TRUE( msg.GetReceiveActionId() == 0 );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 13: Copy constructor deep-copies all fields
    GUCEF_TESTFW_TESTCASE( "Test 13: Copy constructor preserves all fields" )
        try
        {
            PUBSUB::CBasicPubSubMsg src;
            src.GetMsgId() = CORE::CVariant( "copy-id" );
            src.GetPrimaryPayload() = CORE::CVariant( "copy-payload" );
            src.AddKeyValuePair( CORE::CVariant( "ck" ), CORE::CVariant( "cv" ) );
            src.SetReceiveActionId( 55 );
            src.SetOriginClientTopicName( "copy-topic" );

            PUBSUB::CBasicPubSubMsg dst( src );
            ASSERT_TRUE( dst.GetMsgId().AsString() == "copy-id" );
            ASSERT_TRUE( dst.GetPrimaryPayload().AsString() == "copy-payload" );
            ASSERT_TRUE( dst.GetKeyValuePairs().size() == 1 );
            ASSERT_TRUE( dst.GetReceiveActionId() == 55 );
            ASSERT_TRUE( dst.GetOriginClientTopicName() == "copy-topic" );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 14: Assignment operator
    GUCEF_TESTFW_TESTCASE( "Test 14: Assignment operator preserves all fields" )
        try
        {
            PUBSUB::CBasicPubSubMsg src;
            src.GetMsgId() = CORE::CVariant( "assign-id" );
            src.GetPrimaryPayload() = CORE::CVariant( "assign-payload" );

            PUBSUB::CBasicPubSubMsg dst;
            dst = src;

            ASSERT_TRUE( dst.GetMsgId().AsString() == "assign-id" );
            ASSERT_TRUE( dst.GetPrimaryPayload().AsString() == "assign-payload" );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 15: operator== / operator!=
    GUCEF_TESTFW_TESTCASE( "Test 15: operator== and operator!=" )
        try
        {
            PUBSUB::CBasicPubSubMsg a;
            a.GetMsgId() = CORE::CVariant( "eq-id" );
            a.GetPrimaryPayload() = CORE::CVariant( "eq-payload" );

            PUBSUB::CBasicPubSubMsg b( a );
            ASSERT_TRUE( a == b );
            ASSERT_FALSE( a != b );

            b.GetPrimaryPayload() = CORE::CVariant( "different" );
            ASSERT_TRUE( a != b );
            ASSERT_FALSE( a == b );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 16: Clone() produces equivalent copy
    GUCEF_TESTFW_TESTCASE( "Test 16: Clone() produces equivalent message" )
        try
        {
            PUBSUB::CBasicPubSubMsg src;
            src.GetMsgId() = CORE::CVariant( "clone-id" );
            src.GetPrimaryPayload() = CORE::CVariant( "clone-payload" );

            CORE::CICloneable* clonedBase = src.Clone();
            ASSERT_FALSE( clonedBase == GUCEF_NULL );

            if ( clonedBase != GUCEF_NULL )
            {
                PUBSUB::CBasicPubSubMsg* cloned = static_cast< PUBSUB::CBasicPubSubMsg* >( clonedBase );
                ASSERT_TRUE( cloned->GetMsgId().AsString() == "clone-id" );
                ASSERT_TRUE( cloned->GetPrimaryPayload().AsString() == "clone-payload" );
                GUCEF_DELETE clonedBase;
            }
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 17: Serialize / Deserialize via DataNode round-trip
    GUCEF_TESTFW_TESTCASE( "Test 17: Serialize/Deserialize via DataNode round-trip" )
        try
        {
            PUBSUB::CBasicPubSubMsg src;
            src.GetMsgId() = CORE::CVariant( "ser-id" );
            src.GetPrimaryPayload() = CORE::CVariant( "ser-payload" );
            src.AddKeyValuePair( CORE::CVariant( "sk" ), CORE::CVariant( "sv" ) );

            CORE::CDataNode domNode;
            CORE::CDataNodeSerializableSettings settings;
            ASSERT_TRUE( src.Serialize( domNode, settings ) );

            PUBSUB::CBasicPubSubMsg dst;
            ASSERT_TRUE( dst.Deserialize( domNode, settings ) );

            ASSERT_TRUE( dst.GetMsgId().AsString() == "ser-id" );
            ASSERT_TRUE( dst.GetPrimaryPayload().AsString() == "ser-payload" );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 18: GetClassTypeName returns non-empty
    GUCEF_TESTFW_TESTCASE( "Test 18: GetClassTypeName returns non-empty string" )
        try
        {
            PUBSUB::CBasicPubSubMsg msg;
            ASSERT_FALSE( msg.GetClassTypeName().IsNULLOrEmpty() );
            ASSERT_TRUE( msg.GetClassTypeName() == PUBSUB::CBasicPubSubMsg::ClassTypeName );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CBasicPubSubMsg TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
