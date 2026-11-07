/*
 *  gucefCOM_TestApp: GUCEF test application for the COM module
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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestMcpSupport.h"
#include "TestMcpLegacy.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const InitializeParams = "\"protocolVersion\":\"2025-11-25\",\"capabilities\":{\"roots\":{\"listChanged\":true}},\"clientInfo\":{\"name\":\"legacy-client\",\"version\":\"0.9\"}";

const CORE::CDataNode*
LegacyResult( const CORE::CDataNode& response )
{
    return response.FindChild( "result" );
}

/** Opens the 2025-11-25 session the way a legacy client does: initialize, then notifications/initialized */
bool
Handshake( CMcpTestRig& rig, CMcpTestChannel* channel = GUCEF_NULL, bool sendInitialized = true )
{
    CORE::CDataNode response;
    if ( !rig.Call( "\"init\"", McpTestLegacyRequest( "\"init\"", "initialize", InitializeParams ), response, 5000, channel ) )
        return false;
    if ( sendInitialized )
        rig.Send( McpTestNotification( "notifications/initialized" ), channel );
    return GUCEF_NULL != LegacyResult( response );
}

/** A legacy request may carry a progress token in _meta, without the modern keys */
CORE::CString
LegacyRequestWithProgress( const CORE::CString& id, const CORE::CString& method, const CORE::CString& params, const CORE::CString& token )
{
    return "{\"jsonrpc\":\"2.0\",\"id\":" + id + ",\"method\":\"" + method + "\",\"params\":{\"_meta\":{\"progressToken\":" + token + "}," + params + "}}";
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMcpLegacyTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING MCP LEGACY ERA TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "McpLegacy" );

    if ( !McpTestJsonAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MCP TESTS skipped: the json codec is not available" );
        return;
    }

    GUCEF_TESTFW_TESTCASE( "Test: initialize opens a 2025-11-25 session and answers with the InitializeResult" )
        try
        {
            CMcpTestRig rig;
            rig.server->SetInstructions( "legacy instructions" );
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestLegacyRequest( "1", "initialize", InitializeParams ), response ) );
            const CORE::CDataNode* result = LegacyResult( response );
            ASSERT_TRUE( GUCEF_NULL != result );
            if ( GUCEF_NULL != result )
            {
                ASSERT_TRUE( McpTestAttr( *result, "protocolVersion" ) == "2025-11-25" );
                ASSERT_TRUE( McpTestAttr( *result, "resultType" ) == "<none>" );
                ASSERT_TRUE( McpTestAttr( *result, "instructions" ) == "legacy instructions" );
                ASSERT_TRUE( GUCEF_NULL != result->FindChild( "serverInfo" ) && McpTestAttr( *result->FindChild( "serverInfo" ), "name" ) == "test-server" );
                ASSERT_TRUE( McpTestAttr( *result->FindChild( "serverInfo" ), "version" ) == "2.3.4" );
                const CORE::CDataNode* capabilities = result->FindChild( "capabilities" );
                ASSERT_TRUE( GUCEF_NULL != capabilities && capabilities->FindChild( "tools" )->GetAttributeValue( "listChanged" ).AsBool() );
                ASSERT_TRUE( GUCEF_NULL != capabilities && capabilities->FindChild( "resources" )->GetAttributeValue( "subscribe" ).AsBool() );
                ASSERT_TRUE( GUCEF_NULL != capabilities && GUCEF_NULL == capabilities->FindChild( "logging" ) && GUCEF_NULL == capabilities->FindChild( "tasks" ) && GUCEF_NULL == capabilities->FindChild( "completions" ) );
                ASSERT_TRUE( GUCEF_NULL == result->FindChild( "_meta" ) );
            }
            const COM::CMcpLegacySession* session = rig.channel->GetLegacySession();
            ASSERT_TRUE( GUCEF_NULL != session && session->protocolVersion == "2025-11-25" && !session->isOperational );
            ASSERT_TRUE( GUCEF_NULL != session && session->hasClientInfo && session->clientInfo.name == "legacy-client" );
            ASSERT_TRUE( GUCEF_NULL != session && GUCEF_NULL != session->clientCapabilities.FindChild( "roots" ) );
            rig.Send( McpTestNotification( "notifications/initialized" ) );
            ASSERT_TRUE( GUCEF_NULL != session && session->isOperational );

            // an older version asked for: the server answers with the version it speaks
            CMcpTestRig older;
            ASSERT_TRUE( older.Call( "1", McpTestLegacyRequest( "1", "initialize", "\"protocolVersion\":\"2025-06-18\",\"capabilities\":{},\"clientInfo\":{\"name\":\"old\",\"version\":\"1\"}" ), response ) );
            ASSERT_TRUE( McpTestResultAttr( response, "protocolVersion" ) == "2025-11-25" );
            ASSERT_TRUE( older.Call( "2", McpTestLegacyRequest( "2", "initialize", "\"capabilities\":{}" ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( older.Call( "3", McpTestLegacyRequest( "3", "initialize", "\"protocolVersion\":\"2025-11-25\",\"capabilities\":5" ), response ) && -32602 == McpTestErrorCode( response ) );

            // a second initialize replaces the session
            ASSERT_TRUE( rig.Call( "2", McpTestLegacyRequest( "2", "initialize", "\"protocolVersion\":\"2025-11-25\",\"capabilities\":{},\"clientInfo\":{\"name\":\"second\",\"version\":\"1\"}" ), response ) );
            session = rig.channel->GetLegacySession();
            ASSERT_TRUE( GUCEF_NULL != session && session->clientInfo.name == "second" && !session->isOperational );
            ASSERT_TRUE( rig.AllSentAreJsonLines() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The capabilities of initialize follow the channel kind" )
        try
        {
            CMcpTestRig rig( COM::CMcpChannel::CHANNEL_EXCHANGE );
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestLegacyRequest( "1", "initialize", InitializeParams ), response ) );
            const CORE::CDataNode* capabilities = McpTestResultChild( response, "capabilities" );
            ASSERT_TRUE( GUCEF_NULL != capabilities && !capabilities->FindChild( "tools" )->GetAttributeValue( "listChanged" ).AsBool() );
            ASSERT_TRUE( GUCEF_NULL != capabilities && !capabilities->FindChild( "resources" )->GetAttributeValue( "subscribe" ).AsBool() );
            ASSERT_TRUE( GUCEF_NULL != capabilities && !capabilities->FindChild( "prompts" )->GetAttributeValue( "listChanged" ).AsBool() );
            // an exchange keeps no session
            ASSERT_TRUE( GUCEF_NULL == rig.channel->GetLegacySession() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Legacy results carry no resultType, cache hints or serverInfo; ping answers" )
        try
        {
            CMcpTestRig rig;
            CORE::CString error;
            CORE::CDataNode schema;
            ASSERT_TRUE( rig.server->GetToolRegistry().Register( COM::CIMcpToolPtr( GUCEF_NEW CMcpTestTool( "echo", CMcpTestTool::ECHO ) ), error ) );
            ASSERT_TRUE( rig.server->GetToolRegistry().Register( COM::CIMcpToolPtr( GUCEF_NEW CMcpTestTool( "echo_pool", CMcpTestTool::ECHO, COM::MCP_EXECUTION_THREAD_POOL ) ), error ) );
            ASSERT_TRUE( rig.server->GetResourceRegistry().RegisterResource( COM::CIMcpResourcePtr( GUCEF_NEW CMcpTestResource( "test://readme", "readme", 5000 ) ), error ) );
            ASSERT_TRUE( rig.server->GetPromptRegistry().Register( COM::CIMcpPromptPtr( GUCEF_NEW CMcpTestPrompt( "greet" ) ), error ) );
            ASSERT_TRUE( Handshake( rig ) );

            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestLegacyRequest( "1", "ping" ), response ) && GUCEF_NULL != LegacyResult( response ) );
            ASSERT_TRUE( GUCEF_NULL != LegacyResult( response ) && 0 == LegacyResult( response )->GetAttCount() && !LegacyResult( response )->HasChildren() );

            ASSERT_TRUE( rig.Call( "2", McpTestLegacyRequest( "2", "tools/list" ), response ) );
            const CORE::CDataNode* result = LegacyResult( response );
            ASSERT_TRUE( GUCEF_NULL != result && 2 == McpTestChildCount( result->FindChild( "tools" ) ) );
            ASSERT_TRUE( GUCEF_NULL != result && McpTestAttr( *result, "resultType" ) == "<none>" && McpTestAttr( *result, "ttlMs" ) == "<none>" && McpTestAttr( *result, "cacheScope" ) == "<none>" );
            ASSERT_TRUE( GUCEF_NULL != result && GUCEF_NULL == result->FindChild( "_meta" ) );

            ASSERT_TRUE( rig.Call( "3", McpTestLegacyRequest( "3", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"old\"}" ), response ) );
            result = LegacyResult( response );
            ASSERT_TRUE( GUCEF_NULL != result && McpTestFirstText( result ) == "echo: old" && McpTestAttr( *result, "resultType" ) == "<none>" );
            ASSERT_TRUE( rig.Call( "4", McpTestLegacyRequest( "4", "tools/call", "\"name\":\"echo_pool\",\"arguments\":{\"text\":\"old\"}" ), response ) );
            ASSERT_TRUE( McpTestFirstText( response.FindChild( "result" ) ) == "echo: old" );
            // input validation is a tool execution error in 2025-11-25 as well
            ASSERT_TRUE( rig.Call( "5", McpTestLegacyRequest( "5", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":1}" ), response ) );
            ASSERT_TRUE( McpTestResultAttr( response, "isError" ).Lowercase() == "true" );

            ASSERT_TRUE( rig.Call( "6", McpTestLegacyRequest( "6", "resources/read", "\"uri\":\"test://readme\"" ), response ) );
            result = LegacyResult( response );
            ASSERT_TRUE( GUCEF_NULL != result && McpTestAttr( *result, "ttlMs" ) == "<none>" && 1 == McpTestChildCount( result->FindChild( "contents" ) ) );
            ASSERT_TRUE( rig.Call( "7", McpTestLegacyRequest( "7", "prompts/get", "\"name\":\"greet\",\"arguments\":{\"who\":\"Ed\"}" ), response ) );
            ASSERT_TRUE( 2 == McpTestChildCount( McpTestResultChild( response, "messages" ) ) );
            ASSERT_TRUE( rig.Call( "8", McpTestLegacyRequest( "8", "resources/templates/list" ), response ) && GUCEF_NULL != LegacyResult( response ) );

            // what 2025-11-25 does not have here, or what came after it
            ASSERT_TRUE( rig.Call( "9", McpTestLegacyRequest( "9", "subscriptions/listen", "\"notifications\":{}" ), response ) && -32601 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "10", McpTestLegacyRequest( "10", "server/discover" ), response ) && -32601 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "11", McpTestLegacyRequest( "11", "logging/setLevel", "\"level\":\"debug\"" ), response ) && -32601 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "12", McpTestLegacyRequest( "12", "completion/complete" ), response ) && -32601 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.AllSentAreJsonLines() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A missing resource is -32002 for a legacy request" )
        try
        {
            CMcpTestRig rig;
            CORE::CString error;
            ASSERT_TRUE( rig.server->GetResourceRegistry().RegisterTemplate( COM::CIMcpResourceTemplatePtr( GUCEF_NEW CMcpTestTemplate( "items", "test://items/{id}" ) ), error ) );
            ASSERT_TRUE( Handshake( rig ) );
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestLegacyRequest( "1", "resources/read", "\"uri\":\"test://none\"" ), response ) && -32002 == McpTestErrorCode( response ) );
            const CORE::CDataNode* data = McpTestErrorData( response );
            ASSERT_TRUE( GUCEF_NULL != data && McpTestAttr( *data, "uri" ) == "test://none" );
            // also when the handler finds that the resource does not exist
            ASSERT_TRUE( rig.Call( "2", McpTestLegacyRequest( "2", "resources/read", "\"uri\":\"test://items/0\"" ), response ) && -32002 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "3", McpTestLegacyRequest( "3", "resources/read", "\"uri\":\"test://items/5\"" ), response ) && GUCEF_NULL != LegacyResult( response ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: resources/subscribe on a stream sends resources/updated, unsubscribe stops it" )
        try
        {
            CMcpTestRig rig;
            CORE::CString error;
            ASSERT_TRUE( rig.server->GetResourceRegistry().RegisterResource( COM::CIMcpResourcePtr( GUCEF_NEW CMcpTestResource( "test://readme", "readme" ) ), error ) );
            ASSERT_TRUE( Handshake( rig ) );
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestLegacyRequest( "1", "resources/subscribe", "\"uri\":\"test://readme\"" ), response ) && GUCEF_NULL != LegacyResult( response ) );
            ASSERT_TRUE( rig.Call( "2", McpTestLegacyRequest( "2", "resources/subscribe", "\"uri\":\"test://none\"" ), response ) && -32002 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "3", McpTestLegacyRequest( "3", "resources/subscribe", "\"uri\":3" ), response ) && -32602 == McpTestErrorCode( response ) );
            rig.server->NotifyResourceUpdated( "test://readme" );
            rig.Pulse( 2 );
            GUCEF::vector< CORE::CDataNode > found;
            ASSERT_TRUE( 1 == rig.GetNotifications( "notifications/resources/updated", found ) );
            const CORE::CDataNode* params = found[ 0 ].FindChild( "params" );
            ASSERT_TRUE( GUCEF_NULL != params && McpTestAttr( *params, "uri" ) == "test://readme" && GUCEF_NULL == params->FindChild( "_meta" ) );
            ASSERT_TRUE( rig.Call( "4", McpTestLegacyRequest( "4", "resources/unsubscribe", "\"uri\":\"test://readme\"" ), response ) && GUCEF_NULL != LegacyResult( response ) );
            rig.server->NotifyResourceUpdated( "test://readme" );
            rig.Pulse( 2 );
            ASSERT_TRUE( 1 == rig.GetNotifications( "notifications/resources/updated", found ) );

            // over an exchange there is no session to hold it
            CMcpTestChannel* exchange = rig.AddChannel( COM::CMcpChannel::CHANNEL_EXCHANGE, "local:http" );
            ASSERT_TRUE( rig.Call( "\"init\"", McpTestLegacyRequest( "\"init\"", "initialize", InitializeParams ), response, 5000, exchange ) );
            ASSERT_TRUE( rig.Call( "5", McpTestLegacyRequest( "5", "resources/subscribe", "\"uri\":\"test://readme\"" ), response, 5000, exchange ) && -32602 == McpTestErrorCode( response ) );    // no session: not legacy, no _meta
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: list_changed is sent unasked once the session is operational" )
        try
        {
            CMcpTestRig rig;
            CORE::CString error;
            ASSERT_TRUE( Handshake( rig, GUCEF_NULL, false ) );
            ASSERT_TRUE( rig.server->GetToolRegistry().Register( COM::CIMcpToolPtr( GUCEF_NEW CMcpTestTool( "first", CMcpTestTool::ECHO ) ), error ) );
            rig.Pulse( 2 );
            GUCEF::vector< CORE::CDataNode > found;
            ASSERT_TRUE( 0 == rig.GetNotifications( "notifications/tools/list_changed", found ) );       // not before notifications/initialized
            rig.Send( McpTestNotification( "notifications/initialized" ) );
            ASSERT_TRUE( rig.server->GetToolRegistry().Register( COM::CIMcpToolPtr( GUCEF_NEW CMcpTestTool( "second", CMcpTestTool::ECHO ) ), error ) );
            ASSERT_TRUE( rig.server->GetToolRegistry().Register( COM::CIMcpToolPtr( GUCEF_NEW CMcpTestTool( "third", CMcpTestTool::ECHO ) ), error ) );
            ASSERT_TRUE( rig.server->GetPromptRegistry().Register( COM::CIMcpPromptPtr( GUCEF_NEW CMcpTestPrompt( "greet" ) ), error ) );
            ASSERT_TRUE( rig.server->GetResourceRegistry().RegisterResource( COM::CIMcpResourcePtr( GUCEF_NEW CMcpTestResource( "test://a", "a" ) ), error ) );
            rig.Pulse( 2 );
            ASSERT_TRUE( 1 == rig.GetNotifications( "notifications/tools/list_changed", found ) );
            ASSERT_TRUE( GUCEF_NULL == found[ 0 ].FindChild( "params" ) );
            ASSERT_TRUE( 1 == rig.GetNotifications( "notifications/prompts/list_changed", found ) );
            ASSERT_TRUE( 1 == rig.GetNotifications( "notifications/resources/list_changed", found ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A handler's input_required becomes a tool execution error for a legacy client" )
        try
        {
            CMcpTestRig rig;
            CORE::CString error;
            ASSERT_TRUE( rig.server->GetToolRegistry().Register( COM::CIMcpToolPtr( GUCEF_NEW CMcpTestTool( "ask", CMcpTestTool::ASK_NAME ) ), error ) );
            CMcpTestPrompt* prompt = GUCEF_NEW CMcpTestPrompt( "careful" );
            prompt->askInput = true;
            ASSERT_TRUE( rig.server->GetPromptRegistry().Register( COM::CIMcpPromptPtr( prompt ), error ) );
            ASSERT_TRUE( Handshake( rig ) );
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestLegacyRequest( "1", "tools/call", "\"name\":\"ask\",\"arguments\":{}" ), response ) );
            const CORE::CDataNode* result = LegacyResult( response );
            ASSERT_TRUE( GUCEF_NULL != result && McpTestAttr( *result, "isError" ).Lowercase() == "true" );
            ASSERT_TRUE( GUCEF_NULL != result && McpTestFirstText( result ) == "this operation needs client input, which requires MCP protocol 2026-07-28" );
            ASSERT_TRUE( GUCEF_NULL != result && GUCEF_NULL == result->FindChild( "inputRequests" ) && McpTestAttr( *result, "requestState" ) == "<none>" );
            ASSERT_TRUE( rig.Call( "2", McpTestLegacyRequest( "2", "prompts/get", "\"name\":\"careful\",\"arguments\":{\"who\":\"x\"}" ), response ) && -32603 == McpTestErrorCode( response ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Modern and legacy requests interleaved on one channel" )
        try
        {
            CMcpTestRig rig;
            CORE::CString error;
            ASSERT_TRUE( rig.server->GetToolRegistry().Register( COM::CIMcpToolPtr( GUCEF_NEW CMcpTestTool( "echo", CMcpTestTool::ECHO ) ), error ) );
            ASSERT_TRUE( Handshake( rig ) );
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestLegacyRequest( "1", "tools/list" ), response ) && McpTestResultAttr( response, "resultType" ) == "<none>" );
            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "tools/list" ), response ) && McpTestResultAttr( response, "resultType" ) == "complete" );
            ASSERT_TRUE( McpTestResultAttr( response, "ttlMs" ) == "60000" );
            ASSERT_TRUE( rig.Call( "3", McpTestRequest( "3", "server/discover" ), response ) && McpTestResultAttr( response, "resultType" ) == "complete" );
            ASSERT_TRUE( rig.Call( "4", McpTestLegacyRequest( "4", "ping" ), response ) && GUCEF_NULL != LegacyResult( response ) );
            ASSERT_TRUE( rig.Call( "5", McpTestRequest( "5", "ping" ), response ) && -32601 == McpTestErrorCode( response ) );     // modern _meta: modern rules
            ASSERT_TRUE( rig.Call( "6", McpTestLegacyRequest( "6", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"a\"}" ), response ) && McpTestFirstText( response.FindChild( "result" ) ) == "echo: a" );
            ASSERT_TRUE( rig.Call( "7", McpTestRequest( "7", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"b\"}" ), response ) && McpTestResultAttr( response, "resultType" ) == "complete" );

            // without legacy support initialize gets -32022 naming the modern versions
            CMcpTestRig modernOnly;
            COM::CMcpServerSettings settings = modernOnly.server->GetSettings();
            settings.supportLegacy = false;
            modernOnly.SetSettings( settings );
            ASSERT_TRUE( modernOnly.Call( "1", McpTestLegacyRequest( "1", "initialize", InitializeParams ), response ) && -32022 == McpTestErrorCode( response ) );
            const CORE::CDataNode* data = McpTestErrorData( response );
            ASSERT_TRUE( GUCEF_NULL != data && 1 == McpTestChildCount( data->FindChild( "supported" ) ) && McpTestAttr( *data, "requested" ) == "2025-11-25" );
            ASSERT_TRUE( McpTestErrorMessage( response ).HasSubstr( "2026-07-28" ) >= 0 );
            ASSERT_TRUE( GUCEF_NULL == modernOnly.channel->GetLegacySession() );
            // and its -32022 for a modern request lists modern versions only
            ASSERT_TRUE( modernOnly.Call( "2", "{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"server/discover\",\"params\":{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2025-11-25\",\"io.modelcontextprotocol/clientCapabilities\":{}}}}", response ) );
            data = McpTestErrorData( response );
            ASSERT_TRUE( GUCEF_NULL != data && 1 == McpTestChildCount( data->FindChild( "supported" ) ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A dual-era client probing with server/discover finds a modern server" )
        try
        {
            CMcpTestRig rig;
            CORE::CDataNode response;
            // the probe with its preferred modern version: a DiscoverResult
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "server/discover" ), response ) && McpTestResultAttr( response, "resultType" ) == "complete" );
            // with a version the server does not serve: a recognised modern error, so the client stays modern
            ASSERT_TRUE( rig.Call( "2", "{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"server/discover\",\"params\":{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2027-01-01\",\"io.modelcontextprotocol/clientCapabilities\":{}}}}", response ) );
            ASSERT_TRUE( -32022 == McpTestErrorCode( response ) );
            ASSERT_TRUE( GUCEF_NULL == rig.channel->GetLegacySession() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Legacy progress and cancellation on a stream have the same shape" )
        try
        {
            CMcpTestRig rig;
            CORE::CString error;
            CMcpTestTool* count = GUCEF_NEW CMcpTestTool( "count", CMcpTestTool::COUNT, COM::MCP_EXECUTION_THREAD_POOL );
            count->steps = 400;
            count->stepMs = 5;
            ASSERT_TRUE( rig.server->GetToolRegistry().Register( COM::CIMcpToolPtr( count ), error ) );
            ASSERT_TRUE( Handshake( rig ) );
            rig.Send( LegacyRequestWithProgress( "1", "tools/call", "\"name\":\"count\",\"arguments\":{}", "\"tok\"" ) );
            GUCEF::vector< CORE::CDataNode > found;
            for ( UInt32 i=0; i<3000 && 0 == rig.GetNotifications( "notifications/progress", found ); ++i )
                rig.Pulse();
            ASSERT_TRUE( found.size() >= 1 );
            if ( !found.empty() )
                ASSERT_TRUE( McpTestAttr( *found[ 0 ].FindChild( "params" ), "progressToken" ) == "tok" );
            rig.Send( McpTestNotification( "notifications/cancelled", "\"requestId\":1,\"reason\":\"enough\"" ) );
            ASSERT_TRUE( 0 == rig.server->GetInFlightCount() );
            for ( UInt32 i=0; i<3000 && 0 == count->sawCancellation.Load(); ++i )
                rig.Pulse();
            ASSERT_TRUE( 1 == count->sawCancellation.Load() );
            CORE::CDataNode response;
            rig.Pulse( 20 );
            ASSERT_FALSE( rig.FindResponse( "1", response ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Over an exchange a legacy request runs on after a disconnect and its result is dropped" )
        try
        {
            CMcpTestRig rig;
            CMcpTestEventRecorder events( *rig.server );
            CORE::CString error;
            CMcpTestTool* deferred = GUCEF_NEW CMcpTestTool( "later", CMcpTestTool::DEFERRED );
            ASSERT_TRUE( rig.server->GetToolRegistry().Register( COM::CIMcpToolPtr( deferred ), error ) );

            // HTTP: initialize keeps no session; later requests are legacy by their MCP-Protocol-Version header
            CMcpTestChannel* exchange = rig.AddChannel( COM::CMcpChannel::CHANNEL_EXCHANGE, "bearer:client" );
            COM::CMcpChannelPtr exchangePtr = rig.extraChannels.back();
            exchange->legacyRequests = true;
            rig.Send( McpTestLegacyRequest( "1", "tools/call", "\"name\":\"later\",\"arguments\":{}" ), exchange );
            ASSERT_TRUE( 1 == rig.server->GetInFlightCount() );

            // 2025-11-25 does not take a disconnect for a cancellation: the request runs on
            rig.server->CloseChannel( exchangePtr );
            ASSERT_TRUE( 1 == rig.server->GetInFlightCount() );
            size_t sent = exchange->sent.size();
            ASSERT_TRUE( deferred->CompleteLast( "late" ) );
            rig.Pulse( 3 );
            ASSERT_TRUE( 0 == rig.server->GetInFlightCount() );
            ASSERT_TRUE( exchange->sent.size() == sent );
            ASSERT_TRUE( 1 == events.CountOutcome( COM::MCP_OUTCOME_CLIENT_GONE ) );

            // a legacy notifications/cancelled over HTTP counts when one request of the same principal has the id
            CMcpTestChannel* busy = rig.AddChannel( COM::CMcpChannel::CHANNEL_EXCHANGE, "bearer:client" );
            busy->legacyRequests = true;
            rig.Send( McpTestLegacyRequest( "5", "tools/call", "\"name\":\"later\",\"arguments\":{}" ), busy );
            ASSERT_TRUE( 1 == rig.server->GetInFlightCount() );
            CMcpTestChannel* stranger = rig.AddChannel( COM::CMcpChannel::CHANNEL_EXCHANGE, "bearer:someone-else" );
            rig.Send( McpTestNotification( "notifications/cancelled", "\"requestId\":5" ), stranger );
            ASSERT_TRUE( 1 == rig.server->GetInFlightCount() );
            CMcpTestChannel* canceller = rig.AddChannel( COM::CMcpChannel::CHANNEL_EXCHANGE, "bearer:client" );
            rig.Send( McpTestNotification( "notifications/cancelled", "\"requestId\":5" ), canceller );
            ASSERT_TRUE( 0 == rig.server->GetInFlightCount() );
            ASSERT_TRUE( 1 == events.CountOutcome( COM::MCP_OUTCOME_CANCELLED ) );

            // a modern request over an exchange is cancelled by the disconnect
            CMcpTestChannel* modern = rig.AddChannel( COM::CMcpChannel::CHANNEL_EXCHANGE, "bearer:client" );
            COM::CMcpChannelPtr modernPtr = rig.extraChannels.back();
            rig.Send( McpTestRequest( "6", "tools/call", "\"name\":\"later\",\"arguments\":{}" ), modern );
            ASSERT_TRUE( 1 == rig.server->GetInFlightCount() );
            rig.server->CloseChannel( modernPtr );
            ASSERT_TRUE( 0 == rig.server->GetInFlightCount() );
            ASSERT_FALSE( deferred->CompleteLast( "never" ) );

            // a stream close is the end of the client process: a legacy request there is cancelled too
            ASSERT_TRUE( Handshake( rig ) );
            rig.Send( McpTestLegacyRequest( "7", "tools/call", "\"name\":\"later\",\"arguments\":{}" ) );
            ASSERT_TRUE( 1 == rig.server->GetInFlightCount() );
            rig.server->CloseChannel( rig.channelPtr );
            ASSERT_TRUE( 0 == rig.server->GetInFlightCount() );
            ASSERT_FALSE( deferred->CompleteLast( "late" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
