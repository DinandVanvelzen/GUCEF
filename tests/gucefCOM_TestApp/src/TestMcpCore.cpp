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
#include "TestMcpCore.h"

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

const char* const ElicitationCapabilities = "{\"elicitation\":{\"form\":{}}}";

bool
RegisterTestTool( CMcpTestRig& rig, CMcpTestTool* tool )
{
    CORE::CString error;
    return rig.server->GetToolRegistry().Register( COM::CIMcpToolPtr( tool ), error );
}

/** The result member of a response, empty when it is an error */
const CORE::CDataNode*
ResultOf( const CORE::CDataNode& response )
{
    return response.FindChild( "result" );
}

CORE::CString
ResultType( const CORE::CDataNode& response )
{
    const CORE::CDataNode* result = ResultOf( response );
    return GUCEF_NULL == result ? CORE::CString( "<error>" ) : McpTestAttr( *result, "resultType" );
}

/** The name of the serverInfo in the _meta of a result */
CORE::CString
ServerInfoName( const CORE::CDataNode& response )
{
    const CORE::CDataNode* result = ResultOf( response );
    const CORE::CDataNode* meta = GUCEF_NULL != result ? result->FindChild( "_meta" ) : GUCEF_NULL;
    const CORE::CDataNode* info = GUCEF_NULL != meta ? meta->FindChild( "io.modelcontextprotocol/serverInfo" ) : GUCEF_NULL;
    return GUCEF_NULL != info ? McpTestAttr( *info, "name" ) : CORE::CString( "<none>" );
}

/** The names of a list result, in order */
CORE::CString
ListNames( const CORE::CDataNode& response, const char* member, const char* key = "name" )
{
    CORE::CString names;
    const CORE::CDataNode* result = ResultOf( response );
    const CORE::CDataNode* items = GUCEF_NULL != result ? result->FindChild( member ) : GUCEF_NULL;
    if ( GUCEF_NULL == items )
        return "<none>";
    for ( CORE::CDataNode::const_iterator i = items->ConstBegin(); i != items->ConstEnd(); ++i )
    {
        if ( !names.IsNULLOrEmpty() )
            names += ',';
        names += McpTestAttr( *(*i), key );
    }
    return names;
}

/** Makes a request from another thread: an in-process call */
class CCallToolThread : public CMcpTestThread
{
    public:

    CCallToolThread( COM::CMcpServer& server, const CORE::CString& name, UInt32 calls )
        : CMcpTestThread(), m_server( server ), m_name( name ), m_calls( calls ), succeeded( 0 ), wrongText( 0 ) {}

    virtual void Job( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 i=0; i<m_calls; ++i )
        {
            CORE::CDataNode arguments( GUCEF_DATATYPE_OBJECT );
            CORE::CString text = "call " + CORE::ToString( i );
            arguments.SetAttribute( "text", CORE::CVariant( text ) );
            COM::CMcpToolResult result;
            CORE::CString error;
            if ( m_server.CallTool( m_name, arguments, COM::CMcpPrincipal(), result, error ) )
            {
                if ( 1 == result.content.size() && result.content[ 0 ].text == "echo: " + text )
                    succeeded.FetchAdd( 1 );
                else
                    wrongText.FetchAdd( 1 );
            }
        }
    }

    COM::CMcpServer& m_server;
    CORE::CString m_name;
    UInt32 m_calls;
    MT::CTAtomic< UInt32 > succeeded;
    MT::CTAtomic< UInt32 > wrongText;
};

/** Registers and unregisters tools from another thread */
class CRegisterThread : public CMcpTestThread
{
    public:

    CRegisterThread( COM::CMcpServer& server, UInt32 thread ) : CMcpTestThread(), m_server( server ), m_thread( thread ), registered( 0 ) {}

    virtual void Job( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 i=0; i<50; ++i )
        {
            CORE::CString name = "t" + CORE::ToString( m_thread ) + "_" + CORE::ToString( i );
            CORE::CString error;
            if ( m_server.GetToolRegistry().Register( COM::CIMcpToolPtr( GUCEF_NEW CMcpTestTool( name.C_String(), CMcpTestTool::ECHO ) ), error ) )
                registered.FetchAdd( 1 );
            if ( 0 == i % 3 )
                m_server.GetToolRegistry().Unregister( name );
            m_server.NotifyResourceUpdated( "test://readme" );
        }
    }

    COM::CMcpServer& m_server;
    UInt32 m_thread;
    MT::CTAtomic< UInt32 > registered;
};

/** Completes a deferred call from another thread after a while */
class CCompleteThread : public CMcpTestThread
{
    public:

    CCompleteThread( CMcpTestTool& tool ) : CMcpTestThread(), m_tool( tool ), completed( false ) {}

    virtual void Job( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 i=0; i<2000 && !m_tool.HasDeferred(); ++i )
            MT::ThreadDelay( 1 );
        MT::ThreadDelay( 10 );
        completed = m_tool.CompleteLast( "completed elsewhere" );
    }

    CMcpTestTool& m_tool;
    bool completed;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMcpCoreTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING MCP CORE TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "McpCore" );

    if ( !McpTestJsonAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MCP TESTS skipped: the json codec is not available" );
        return;
    }

    GUCEF_TESTFW_TESTCASE( "Test: server/discover answers with versions, capabilities, cache hints and serverInfo" )
        try
        {
            CMcpTestRig rig;
            rig.server->SetInstructions( "Use the echo tool." );
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "server/discover" ), response ) );
            const CORE::CDataNode* result = ResultOf( response );
            ASSERT_TRUE( GUCEF_NULL != result );
            if ( GUCEF_NULL != result )
            {
                ASSERT_TRUE( McpTestAttr( *result, "resultType" ) == "complete" );
                ASSERT_TRUE( McpTestAttr( *result, "ttlMs" ) == "3600000" && McpTestAttr( *result, "cacheScope" ) == "public" );
                ASSERT_TRUE( McpTestAttr( *result, "instructions" ) == "Use the echo tool." );
                const CORE::CDataNode* versions = result->FindChild( "supportedVersions" );
                ASSERT_TRUE( 1 == McpTestChildCount( versions ) && (*versions->ConstBegin())->GetValue().AsString() == "2026-07-28" );
                const CORE::CDataNode* capabilities = result->FindChild( "capabilities" );
                ASSERT_TRUE( GUCEF_NULL != capabilities );
                if ( GUCEF_NULL != capabilities )
                {
                    ASSERT_TRUE( GUCEF_NULL != capabilities->FindChild( "tools" ) && capabilities->FindChild( "tools" )->GetAttributeValue( "listChanged" ).AsBool() );
                    ASSERT_TRUE( GUCEF_NULL != capabilities->FindChild( "resources" ) && capabilities->FindChild( "resources" )->GetAttributeValue( "subscribe" ).AsBool() );
                    ASSERT_TRUE( GUCEF_NULL != capabilities->FindChild( "prompts" ) );
                    ASSERT_TRUE( GUCEF_NULL == capabilities->FindChild( "logging" ) && GUCEF_NULL == capabilities->FindChild( "completions" ) && GUCEF_NULL == capabilities->FindChild( "extensions" ) );
                }
                ASSERT_TRUE( ServerInfoName( response ) == "test-server" );
            }

            // a disabled registry is left out, and so is serverInfo when asked
            COM::CMcpServerSettings settings = rig.server->GetSettings();
            settings.enablePrompts = false;
            settings.includeServerInfo = false;
            rig.SetSettings( settings );
            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "server/discover" ), response ) );
            result = ResultOf( response );
            ASSERT_TRUE( GUCEF_NULL != result && GUCEF_NULL == result->FindChild( "capabilities" )->FindChild( "prompts" ) );
            ASSERT_TRUE( ServerInfoName( response ) == "<none>" );
            ASSERT_TRUE( rig.Call( "3", McpTestRequest( "3", "prompts/list" ), response ) && -32601 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.AllSentAreJsonLines() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: _meta is required and checked, an unsupported version gets -32022" )
        try
        {
            CMcpTestRig rig;
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"server/discover\"}", response ) );
            ASSERT_TRUE( -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "2", "{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"tools/list\",\"params\":{}}", response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "3", "{\"jsonrpc\":\"2.0\",\"id\":3,\"method\":\"tools/list\",\"params\":{\"_meta\":\"2026-07-28\"}}", response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "4", "{\"jsonrpc\":\"2.0\",\"id\":4,\"method\":\"tools/list\",\"params\":{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\"}}}", response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( McpTestErrorMessage( response ).HasSubstr( "clientCapabilities" ) >= 0 );
            ASSERT_TRUE( rig.Call( "5", "{\"jsonrpc\":\"2.0\",\"id\":5,\"method\":\"tools/list\",\"params\":{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":7,\"io.modelcontextprotocol/clientCapabilities\":{}}}}", response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "6", "{\"jsonrpc\":\"2.0\",\"id\":6,\"method\":\"tools/list\",\"params\":{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\",\"io.modelcontextprotocol/clientCapabilities\":true}}}", response ) && -32602 == McpTestErrorCode( response ) );

            // the version: -32022 lists what is served, the legacy version included, and what was asked
            CORE::CString unsupported = "{\"jsonrpc\":\"2.0\",\"id\":7,\"method\":\"server/discover\",\"params\":{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"1900-01-01\",\"io.modelcontextprotocol/clientCapabilities\":{}}}}";
            ASSERT_TRUE( rig.Call( "7", unsupported, response ) && -32022 == McpTestErrorCode( response ) );
            const CORE::CDataNode* data = McpTestErrorData( response );
            ASSERT_TRUE( GUCEF_NULL != data && McpTestAttr( *data, "requested" ) == "1900-01-01" );
            ASSERT_TRUE( GUCEF_NULL != data && 2 == McpTestChildCount( data->FindChild( "supported" ) ) );
            if ( GUCEF_NULL != data && 2 == McpTestChildCount( data->FindChild( "supported" ) ) )
            {
                CORE::CDataNode::const_iterator v = data->FindChild( "supported" )->ConstBegin();
                ASSERT_TRUE( (*v)->GetValue().AsString() == "2026-07-28" );
                ++v;
                ASSERT_TRUE( (*v)->GetValue().AsString() == "2025-11-25" );
            }
            // a legacy version in per request _meta is not a modern request either
            ASSERT_TRUE( rig.Call( "8", "{\"jsonrpc\":\"2.0\",\"id\":8,\"method\":\"tools/list\",\"params\":{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2025-11-25\",\"io.modelcontextprotocol/clientCapabilities\":{}}}}", response ) && -32022 == McpTestErrorCode( response ) );
            // the version is checked before the method
            ASSERT_TRUE( rig.Call( "9", "{\"jsonrpc\":\"2.0\",\"id\":9,\"method\":\"ping\",\"params\":{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2099-01-01\",\"io.modelcontextprotocol/clientCapabilities\":{}}}}", response ) && -32022 == McpTestErrorCode( response ) );
            // logLevel and trace context are accepted and change nothing
            ASSERT_TRUE( rig.Call( "10", McpTestRequest( "10", "tools/list", "", "{}", "\"io.modelcontextprotocol/logLevel\":\"debug\",\"traceparent\":\"00-0af7651916cd43dd8448eb211c80319c-00f067aa0ba902b7-01\"" ), response ) );
            ASSERT_TRUE( ResultType( response ) == "complete" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Removed and unsupported methods answer -32601" )
        try
        {
            CMcpTestRig rig;
            const char* methods[] = { "ping", "logging/setLevel", "completion/complete", "resources/subscribe", "resources/unsubscribe", "initialize", "tasks/get", "sampling/createMessage", "unknown/method" };
            for ( UInt32 i=0; i<sizeof( methods ) / sizeof( methods[ 0 ] ); ++i )
            {
                CORE::CString id = CORE::ToString( i + 1 );
                CORE::CDataNode response;
                ASSERT_TRUE( rig.Call( id, McpTestRequest( id, methods[ i ], "\"protocolVersion\":\"2025-11-25\"" ), response ) );
                ASSERT_TRUE( -32601 == McpTestErrorCode( response ) );
            }
            ASSERT_TRUE( 9 == rig.channel->errorCodes.size() && -32601 == rig.channel->errorCodes[ 0 ] );    // the channel hears every code first
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Lists are in byte order whatever the registration order, with stable cursors" )
        try
        {
            CMcpTestRig first;
            CMcpTestRig second;
            const char* names[] = { "delta", "Alpha", "charlie", "bravo", "echo" };
            for ( int i=0; i<5; ++i )
            {
                RegisterTestTool( first, GUCEF_NEW CMcpTestTool( names[ i ], CMcpTestTool::ECHO ) );
                RegisterTestTool( second, GUCEF_NEW CMcpTestTool( names[ 4 - i ], CMcpTestTool::ECHO ) );
            }
            CORE::CDataNode a;
            CORE::CDataNode b;
            ASSERT_TRUE( first.Call( "1", McpTestRequest( "1", "tools/list" ), a ) && second.Call( "1", McpTestRequest( "1", "tools/list" ), b ) );
            ASSERT_TRUE( ListNames( a, "tools" ) == "Alpha,bravo,charlie,delta,echo" );
            ASSERT_TRUE( McpTestToJson( a ) == McpTestToJson( b ) );
            ASSERT_TRUE( McpTestResultAttr( a, "ttlMs" ) == "60000" && McpTestResultAttr( a, "cacheScope" ) == "public" );
            ASSERT_TRUE( McpTestResultAttr( a, "nextCursor" ) == "<none>" );

            // pages of 2
            COM::CMcpServerSettings settings = first.server->GetSettings();
            settings.pageSize = 2;
            first.SetSettings( settings );
            CORE::CString collected;
            CORE::CString cursor;
            UInt32 pages = 0;
            for ( UInt32 page=0; page<10; ++page )
            {
                CORE::CString id = "p" + CORE::ToString( page );
                CORE::CString params = cursor.IsNULLOrEmpty() ? CORE::CString::Empty : "\"cursor\":\"" + cursor + "\"";
                CORE::CDataNode response;
                ASSERT_TRUE( first.Call( "\"" + id + "\"", McpTestRequest( "\"" + id + "\"", "tools/list", params ), response ) );
                ++pages;
                // every page carries the same cache scope
                ASSERT_TRUE( McpTestResultAttr( response, "cacheScope" ) == "public" );
                if ( !collected.IsNULLOrEmpty() )
                    collected += ',';
                collected += ListNames( response, "tools" );
                cursor = McpTestResultAttr( response, "nextCursor" );
                if ( cursor == "<none>" )
                    break;
            }
            ASSERT_TRUE( 3 == pages );
            ASSERT_TRUE( collected == "Alpha,bravo,charlie,delta,echo" );

            // the cursor is the key of the last item: it stays valid, also when that item is removed meanwhile
            CORE::CDataNode response;
            ASSERT_TRUE( first.Call( "10", McpTestRequest( "10", "tools/list" ), response ) );
            CORE::CString afterBravo = McpTestResultAttr( response, "nextCursor" );
            first.server->GetToolRegistry().Unregister( "bravo" );
            ASSERT_TRUE( first.Call( "11", McpTestRequest( "11", "tools/list", "\"cursor\":\"" + afterBravo + "\"" ), response ) );
            ASSERT_TRUE( ListNames( response, "tools" ) == "charlie,delta" );

            // a cursor that does not decode
            ASSERT_TRUE( first.Call( "12", McpTestRequest( "12", "tools/list", "\"cursor\":\"***\"" ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( McpTestErrorMessage( response ) == "Invalid cursor" );
            ASSERT_TRUE( first.Call( "13", McpTestRequest( "13", "tools/list", "\"cursor\":5" ), response ) && -32602 == McpTestErrorCode( response ) );
            // an empty cursor is a cursor: before every key
            ASSERT_TRUE( first.Call( "14", McpTestRequest( "14", "tools/list", "\"cursor\":\"\"" ), response ) && ListNames( response, "tools" ) == "Alpha,charlie" );

            // visibility that depends on the principal makes every list private
            CMcpTestPolicy* policy = GUCEF_NEW CMcpTestPolicy();
            policy->principalDependent = true;
            first.server->SetAccessPolicy( COM::CIMcpAccessPolicyPtr( policy ) );
            ASSERT_TRUE( first.Call( "15", McpTestRequest( "15", "tools/list" ), response ) && McpTestResultAttr( response, "cacheScope" ) == "private" );
            ASSERT_TRUE( first.Call( "16", McpTestRequest( "16", "prompts/list" ), response ) && McpTestResultAttr( response, "cacheScope" ) == "private" );
            ASSERT_TRUE( first.Call( "17", McpTestRequest( "17", "resources/templates/list" ), response ) && McpTestResultAttr( response, "cacheScope" ) == "private" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The same tool code answers on the calling thread, on the pool and deferred" )
        try
        {
            CMcpTestRig rig;
            CMcpTestTool* sync = GUCEF_NEW CMcpTestTool( "echo_sync", CMcpTestTool::ECHO );
            CMcpTestTool* pool = GUCEF_NEW CMcpTestTool( "echo_pool", CMcpTestTool::ECHO, COM::MCP_EXECUTION_THREAD_POOL );
            CMcpTestTool* deferred = GUCEF_NEW CMcpTestTool( "later", CMcpTestTool::DEFERRED );
            CMcpTestTool* deferredPool = GUCEF_NEW CMcpTestTool( "later_pool", CMcpTestTool::DEFERRED, COM::MCP_EXECUTION_THREAD_POOL );
            ASSERT_TRUE( RegisterTestTool( rig, sync ) && RegisterTestTool( rig, pool ) && RegisterTestTool( rig, deferred ) && RegisterTestTool( rig, deferredPool ) );

            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "tools/call", "\"name\":\"echo_sync\",\"arguments\":{\"text\":\"hi\"}" ), response, 0 ) );   // answered inside the dispatch
            ASSERT_TRUE( ResultType( response ) == "complete" && McpTestFirstText( response.FindChild( "result" ) ) == "echo: hi" );
            ASSERT_TRUE( McpTestResultAttr( response, "isError" ).Lowercase() == "false" );
            ASSERT_TRUE( ServerInfoName( response ) == "test-server" );
            ASSERT_TRUE( McpTestResultAttr( response, "ttlMs" ) == "<none>" );      // tools/call is not cacheable

            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "tools/call", "\"name\":\"echo_pool\",\"arguments\":{\"text\":\"hi\"}" ), response ) );
            ASSERT_TRUE( ResultType( response ) == "complete" && McpTestFirstText( response.FindChild( "result" ) ) == "echo: hi" );
            ASSERT_TRUE( 0 == rig.server->GetInFlightCount() );

            // deferred, completed from the test thread
            rig.Send( McpTestRequest( "3", "tools/call", "\"name\":\"later\",\"arguments\":{}" ) );
            rig.Pulse( 5 );
            ASSERT_FALSE( rig.FindResponse( "3", response ) );
            ASSERT_TRUE( 1 == rig.server->GetInFlightCount() );
            ASSERT_TRUE( deferred->CompleteLast( "done later" ) );
            ASSERT_FALSE( deferred->CompleteLast( "twice" ) );
            ASSERT_TRUE( rig.WaitForResponse( "3", response ) && McpTestFirstText( response.FindChild( "result" ) ) == "done later" );
            ASSERT_TRUE( 0 == rig.server->GetInFlightCount() );

            // deferred on the pool, completed from yet another thread
            CCompleteThread completer( *deferredPool );
            ASSERT_TRUE( completer.Start() );
            rig.Send( McpTestRequest( "4", "tools/call", "\"name\":\"later_pool\",\"arguments\":{}" ) );
            ASSERT_TRUE( rig.WaitForResponse( "4", response ) && McpTestFirstText( response.FindChild( "result" ) ) == "completed elsewhere" );
            ASSERT_TRUE( completer.Join( &rig ) && completer.completed );
            ASSERT_TRUE( rig.AllSentAreJsonLines() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Arguments: schema failures are tool execution errors, the wrong shape is -32602" )
        try
        {
            CMcpTestRig rig;
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "echo", CMcpTestTool::ECHO ) ) );
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":5,\"extra\":1}" ), response ) );
            ASSERT_TRUE( ResultType( response ) == "complete" && McpTestResultAttr( response, "isError" ).Lowercase() == "true" );
            CORE::CString text = McpTestFirstText( response.FindChild( "result" ) );
            ASSERT_TRUE( text.HasSubstr( "Invalid arguments for tool echo" ) >= 0 && text.HasSubstr( "/text: must be of type string" ) >= 0 );

            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "tools/call", "\"name\":\"echo\",\"arguments\":{}" ), response ) );
            ASSERT_TRUE( McpTestFirstText( response.FindChild( "result" ) ).HasSubstr( "(root): the required property \"text\" is missing" ) >= 0 );

            ASSERT_TRUE( rig.Call( "3", McpTestRequest( "3", "tools/call", "\"name\":\"echo\",\"arguments\":[1]" ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "4", McpTestRequest( "4", "tools/call", "\"name\":\"echo\",\"arguments\":\"text\"" ), response ) && -32602 == McpTestErrorCode( response ) );
            // arguments is optional in CallToolRequestParams (2026-07-28 schema): absent is {}, judged by the schema like {}
            ASSERT_TRUE( rig.Call( "5", McpTestRequest( "5", "tools/call", "\"name\":\"echo\"" ), response ) && 0 == McpTestErrorCode( response ) );
            ASSERT_TRUE( McpTestResultAttr( response, "isError" ).Lowercase() == "true" );
            ASSERT_TRUE( McpTestFirstText( response.FindChild( "result" ) ).HasSubstr( "(root): the required property \"text\" is missing" ) >= 0 );
            ASSERT_TRUE( rig.Call( "6", McpTestRequest( "6", "tools/call", "\"name\":7" ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "7", McpTestRequest( "7", "tools/call", "\"name\":\"nope\",\"arguments\":{}" ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( McpTestErrorMessage( response ) == "Unknown tool: nope" );
            ASSERT_TRUE( rig.Call( "8", McpTestRequest( "8", "tools/call" ), response ) && -32602 == McpTestErrorCode( response ) );

            // an argument with a control character arrives intact
            ASSERT_TRUE( rig.Call( "9", McpTestRequest( "9", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"a\\u0001b\\nc\"}" ), response ) );
            ASSERT_TRUE( McpTestFirstText( response.FindChild( "result" ) ) == "echo: a\x01" "b\nc" );
            ASSERT_TRUE( rig.AllSentAreJsonLines() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Structured output is validated before it is sent" )
        try
        {
            CMcpTestRig rig;
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "sum", CMcpTestTool::STRUCTURED ) ) );
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "sum_wrong", CMcpTestTool::STRUCTURED_WRONG ) ) );
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "tools/call", "\"name\":\"sum\",\"arguments\":{\"a\":2,\"b\":40}" ), response ) );
            const CORE::CDataNode* result = ResultOf( response );
            ASSERT_TRUE( GUCEF_NULL != result && GUCEF_NULL != result->FindChild( "structuredContent" ) );
            ASSERT_TRUE( GUCEF_NULL != result && McpTestAttr( *result->FindChild( "structuredContent" ), "sum" ) == "42" );
            ASSERT_TRUE( GUCEF_NULL != result && McpTestFirstText( result ) == "{\"sum\":42}" );
            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "tools/call", "\"name\":\"sum_wrong\",\"arguments\":{\"a\":2,\"b\":40}" ), response ) );
            ASSERT_TRUE( -32603 == McpTestErrorCode( response ) );

            // without validation the server sends what the tool gave
            COM::CMcpServerSettings settings = rig.server->GetSettings();
            settings.validateStructuredOutput = false;
            rig.SetSettings( settings );
            ASSERT_TRUE( rig.Call( "3", McpTestRequest( "3", "tools/call", "\"name\":\"sum_wrong\",\"arguments\":{\"a\":2,\"b\":40}" ), response ) && ResultType( response ) == "complete" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Handler protocol errors and exceptions" )
        try
        {
            CMcpTestRig rig;
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "fails", CMcpTestTool::PROTOCOL_ERROR ) ) );
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "throws", CMcpTestTool::THROWS ) ) );
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "throws_pool", CMcpTestTool::THROWS, COM::MCP_EXECUTION_THREAD_POOL ) ) );
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "tools/call", "\"name\":\"fails\",\"arguments\":{}" ), response ) && -32602 == McpTestErrorCode( response ) );
            const CORE::CDataNode* data = McpTestErrorData( response );
            ASSERT_TRUE( GUCEF_NULL != data && McpTestAttr( *data, "why" ) == "test" );
            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "tools/call", "\"name\":\"throws\",\"arguments\":{}" ), response ) && -32603 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "3", McpTestRequest( "3", "tools/call", "\"name\":\"throws_pool\",\"arguments\":{}" ), response ) && -32603 == McpTestErrorCode( response ) );

            // a handler may not use the reserved codes
            COM::CMcpRequestContextPtr context = COM::CMcpRequestContext::Create( "tools/call", "x", CORE::CVariant( (Int32) 1 ), COM::MCP_ERA_MODERN, COM::CMcpPrincipal(), COM::CMcpRequestMeta(), 0 );
            ASSERT_TRUE( context->CompleteWithProtocolError( -32042, "url elicitation" ) );
            ASSERT_TRUE( -32603 == context->GetProtocolErrorCode() );
            COM::CMcpRequestContextPtr appCode = COM::CMcpRequestContext::Create( "tools/call", "x", CORE::CVariant( (Int32) 1 ), COM::MCP_ERA_MODERN, COM::CMcpPrincipal(), COM::CMcpRequestMeta(), 0 );
            ASSERT_TRUE( appCode->CompleteWithProtocolError( -1234, "application" ) && -1234 == appCode->GetProtocolErrorCode() );
            // a completion of the wrong kind is refused
            COM::CMcpPromptResult prompt;
            COM::CMcpRequestContextPtr wrongKind = COM::CMcpRequestContext::Create( "tools/call", "x", CORE::CVariant( (Int32) 1 ), COM::MCP_ERA_MODERN, COM::CMcpPrincipal(), COM::CMcpRequestMeta(), 0 );
            ASSERT_FALSE( wrongKind->CompleteWithPromptResult( prompt ) );
            ASSERT_FALSE( wrongKind->IsCompleted() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Resources: lists, reads of resources and templates, not found, cache hints" )
        try
        {
            CMcpTestRig rig;
            CORE::CString error;
            ASSERT_TRUE( rig.server->GetResourceRegistry().RegisterResource( COM::CIMcpResourcePtr( GUCEF_NEW CMcpTestResource( "test://readme", "readme", 5000 ) ), error ) );
            ASSERT_TRUE( rig.server->GetResourceRegistry().RegisterResource( COM::CIMcpResourcePtr( GUCEF_NEW CMcpTestResource( "test://changelog", "changelog" ) ), error ) );
            ASSERT_TRUE( rig.server->GetResourceRegistry().RegisterResource( COM::CIMcpResourcePtr( GUCEF_NEW CMcpTestResource( "test://hidden", "hidden" ) ), error ) );
            ASSERT_TRUE( rig.server->GetResourceRegistry().RegisterTemplate( COM::CIMcpResourceTemplatePtr( GUCEF_NEW CMcpTestTemplate( "items", "test://items/{id}" ) ), error ) );

            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "resources/list" ), response ) );
            ASSERT_TRUE( ListNames( response, "resources", "uri" ) == "test://changelog,test://hidden,test://readme" );
            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "resources/templates/list" ), response ) );
            ASSERT_TRUE( ListNames( response, "resourceTemplates", "uriTemplate" ) == "test://items/{id}" && ResultType( response ) == "complete" );

            ASSERT_TRUE( rig.Call( "3", McpTestRequest( "3", "resources/read", "\"uri\":\"test://readme\"" ), response ) );
            const CORE::CDataNode* result = ResultOf( response );
            ASSERT_TRUE( GUCEF_NULL != result && McpTestAttr( *result, "resultType" ) == "complete" );
            ASSERT_TRUE( GUCEF_NULL != result && McpTestAttr( *result, "ttlMs" ) == "5000" && McpTestAttr( *result, "cacheScope" ) == "public" );
            ASSERT_TRUE( GUCEF_NULL != result && 1 == McpTestChildCount( result->FindChild( "contents" ) ) );
            ASSERT_TRUE( GUCEF_NULL != result && McpTestAttr( *(*result->FindChild( "contents" )->ConstBegin()), "text" ) == "contents of readme" );

            // the default hints of a resource: 0 ms, private
            ASSERT_TRUE( rig.Call( "4", McpTestRequest( "4", "resources/read", "\"uri\":\"test://changelog\"" ), response ) );
            ASSERT_TRUE( McpTestResultAttr( response, "ttlMs" ) == "0" && McpTestResultAttr( response, "cacheScope" ) == "private" );

            // a template read on the pool, with the hints of its contents
            ASSERT_TRUE( rig.Call( "5", McpTestRequest( "5", "resources/read", "\"uri\":\"test://items/3\"" ), response ) );
            ASSERT_TRUE( McpTestResultAttr( response, "ttlMs" ) == "1234" && McpTestResultAttr( response, "cacheScope" ) == "public" );
            ASSERT_TRUE( 1 == McpTestChildCount( McpTestResultChild( response, "contents" ) ) && McpTestAttr( *(*McpTestResultChild( response, "contents" )->ConstBegin()), "text" ) == "{\"item\":\"3\"}" );

            // not found: -32602 with the URI, by the registry and by the handler, never -32002 for a modern request
            ASSERT_TRUE( rig.Call( "6", McpTestRequest( "6", "resources/read", "\"uri\":\"test://nothing\"" ), response ) && -32602 == McpTestErrorCode( response ) );
            const CORE::CDataNode* data = McpTestErrorData( response );
            ASSERT_TRUE( GUCEF_NULL != data && McpTestAttr( *data, "uri" ) == "test://nothing" );
            ASSERT_TRUE( rig.Call( "7", McpTestRequest( "7", "resources/read", "\"uri\":\"test://items/0\"" ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "8", McpTestRequest( "8", "resources/read", "\"uri\":5" ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "9", McpTestRequest( "9", "resources/read" ), response ) && -32602 == McpTestErrorCode( response ) );

            // the policy: hidden is not found, denied is -31003, principal dependent makes reads private
            CMcpTestPolicy* policy = GUCEF_NEW CMcpTestPolicy();
            rig.server->SetAccessPolicy( COM::CIMcpAccessPolicyPtr( policy ) );
            ASSERT_TRUE( rig.Call( "10", McpTestRequest( "10", "resources/list" ), response ) && ListNames( response, "resources", "uri" ) == "test://changelog,test://readme" );
            ASSERT_TRUE( rig.Call( "11", McpTestRequest( "11", "resources/read", "\"uri\":\"test://hidden\"" ), response ) && -32602 == McpTestErrorCode( response ) );
            policy->principalDependent = true;
            ASSERT_TRUE( rig.Call( "12", McpTestRequest( "12", "resources/read", "\"uri\":\"test://readme\"" ), response ) && McpTestResultAttr( response, "cacheScope" ) == "private" );
            policy->denyResources = true;
            ASSERT_TRUE( rig.Call( "13", McpTestRequest( "13", "resources/read", "\"uri\":\"test://readme\"" ), response ) && -31003 == McpTestErrorCode( response ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Prompts: list, get, required and wrongly typed arguments" )
        try
        {
            CMcpTestRig rig;
            CORE::CString error;
            ASSERT_TRUE( rig.server->GetPromptRegistry().Register( COM::CIMcpPromptPtr( GUCEF_NEW CMcpTestPrompt( "greet" ) ), error ) );
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "prompts/list" ), response ) && ListNames( response, "prompts" ) == "greet" );
            const CORE::CDataNode* prompts = McpTestResultChild( response, "prompts" );
            ASSERT_TRUE( 2 == McpTestChildCount( (*prompts->ConstBegin())->FindChild( "arguments" ) ) );

            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "prompts/get", "\"name\":\"greet\",\"arguments\":{\"who\":\"Ada\"}" ), response ) );
            ASSERT_TRUE( ResultType( response ) == "complete" && McpTestResultAttr( response, "description" ) == "a greeting" );
            ASSERT_TRUE( 2 == McpTestChildCount( McpTestResultChild( response, "messages" ) ) );
            ASSERT_TRUE( McpTestResultAttr( response, "ttlMs" ) == "<none>" );      // prompts/get is not cacheable

            ASSERT_TRUE( rig.Call( "3", McpTestRequest( "3", "prompts/get", "\"name\":\"greet\"" ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "4", McpTestRequest( "4", "prompts/get", "\"name\":\"greet\",\"arguments\":{\"who\":5}" ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "5", McpTestRequest( "5", "prompts/get", "\"name\":\"greet\",\"arguments\":{\"who\":{\"a\":1}}" ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "6", McpTestRequest( "6", "prompts/get", "\"name\":\"nope\"" ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( McpTestErrorMessage( response ) == "Unknown prompt: nope" );

            CMcpTestPolicy* policy = GUCEF_NEW CMcpTestPolicy();
            policy->denyPrompts = true;
            rig.server->SetAccessPolicy( COM::CIMcpAccessPolicyPtr( policy ) );
            ASSERT_TRUE( rig.Call( "7", McpTestRequest( "7", "prompts/get", "\"name\":\"greet\",\"arguments\":{\"who\":\"Ada\"}" ), response ) && -31003 == McpTestErrorCode( response ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: MRTR: input_required with elicitation and the retry with responses and state" )
        try
        {
            CMcpTestRig rig;
            CMcpTestTool* ask = GUCEF_NEW CMcpTestTool( "ask", CMcpTestTool::ASK_NAME );
            ASSERT_TRUE( RegisterTestTool( rig, ask ) );
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "tools/call", "\"name\":\"ask\",\"arguments\":{\"text\":\"x\"}", ElicitationCapabilities ), response ) );
            const CORE::CDataNode* result = ResultOf( response );
            ASSERT_TRUE( ResultType( response ) == "input_required" );
            ASSERT_TRUE( GUCEF_NULL != result && McpTestAttr( *result, "ttlMs" ) == "<none>" && McpTestAttr( *result, "cacheScope" ) == "<none>" );   // never cacheable
            ASSERT_TRUE( ServerInfoName( response ) == "test-server" );
            const CORE::CDataNode* requests = GUCEF_NULL != result ? result->FindChild( "inputRequests" ) : GUCEF_NULL;
            const CORE::CDataNode* request = GUCEF_NULL != requests ? requests->FindChild( "name_request" ) : GUCEF_NULL;
            ASSERT_TRUE( GUCEF_NULL != request && McpTestAttr( *request, "method" ) == "elicitation/create" );
            ASSERT_TRUE( GUCEF_NULL != request && GUCEF_NULL != request->FindChild( "params" ) && McpTestAttr( *request->FindChild( "params" ), "mode" ) == "form" );
            CORE::CString state = GUCEF_NULL != result ? McpTestAttr( *result, "requestState" ) : CORE::CString::Empty;
            ASSERT_TRUE( !state.IsNULLOrEmpty() && state != "<none>" );

            // the retry: a new id, the answers and the state as it was given
            CORE::CString retry = "\"name\":\"ask\",\"arguments\":{\"text\":\"x\"},\"inputResponses\":{\"name_request\":{\"action\":\"accept\",\"content\":{\"name\":\"Ada\"}},\"unknown\":{}},\"requestState\":\"" + state + "\"";
            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "tools/call", retry, ElicitationCapabilities ), response ) );
            ASSERT_TRUE( ResultType( response ) == "complete" && McpTestFirstText( response.FindChild( "result" ) ) == "Hello Ada" );
            ASSERT_TRUE( ask->lastStateStep == "1" && 32 == ask->lastNonce.Length() );

            // the same state again: seals do not stop a replay within their lifetime, the nonce lets a handler do that
            CORE::CString firstNonce = ask->lastNonce;
            ASSERT_TRUE( rig.Call( "3", McpTestRequest( "3", "tools/call", retry, ElicitationCapabilities ), response ) && ResultType( response ) == "complete" );
            ASSERT_TRUE( ask->lastNonce == firstNonce );

            // an answer missing: the handler asks again rather than failing
            ASSERT_TRUE( rig.Call( "4", McpTestRequest( "4", "tools/call", "\"name\":\"ask\",\"arguments\":{\"text\":\"x\"},\"requestState\":\"" + state + "\"", ElicitationCapabilities ), response ) );
            ASSERT_TRUE( ResultType( response ) == "input_required" );
            ASSERT_TRUE( rig.AllSentAreJsonLines() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: MRTR: tampered, expired, other-principal and other-arguments state is refused" )
        try
        {
            CMcpTestRig rig;
            COM::CMcpServerSettings settings = rig.server->GetSettings();
            settings.requestStateTtlMs = 150;
            rig.SetSettings( settings );
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "ask", CMcpTestTool::ASK_NAME ) ) );
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "tools/call", "\"name\":\"ask\",\"arguments\":{\"text\":\"x\"}", ElicitationCapabilities ), response ) );
            CORE::CString state = McpTestResultAttr( response, "requestState" );
            const CORE::CString answers = "\"inputResponses\":{\"name_request\":{\"action\":\"accept\",\"content\":{\"name\":\"Ada\"}}}";

            // tampered
            CORE::CString tampered = state.SubstrFromRange( 0, 3 ) + ( state[ 3 ] == 'A' ? "B" : "A" ) + state.SubstrFromRange( 4, state.Length() );
            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "tools/call", "\"name\":\"ask\",\"arguments\":{\"text\":\"x\"}," + answers + ",\"requestState\":\"" + tampered + "\"", ElicitationCapabilities ), response ) );
            ASSERT_TRUE( -32602 == McpTestErrorCode( response ) && McpTestErrorMessage( response ).HasSubstr( "requestState invalid or expired" ) >= 0 );
            ASSERT_TRUE( rig.Call( "3", McpTestRequest( "3", "tools/call", "\"name\":\"ask\",\"arguments\":{\"text\":\"x\"}," + answers + ",\"requestState\":\"garbage\"", ElicitationCapabilities ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "4", McpTestRequest( "4", "tools/call", "\"name\":\"ask\",\"arguments\":{\"text\":\"x\"}," + answers + ",\"requestState\":5", ElicitationCapabilities ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "5", McpTestRequest( "5", "tools/call", "\"name\":\"ask\",\"arguments\":{\"text\":\"x\"},\"inputResponses\":[1],\"requestState\":\"" + state + "\"", ElicitationCapabilities ), response ) && -32602 == McpTestErrorCode( response ) );

            // other arguments
            ASSERT_TRUE( rig.Call( "6", McpTestRequest( "6", "tools/call", "\"name\":\"ask\",\"arguments\":{\"text\":\"y\"}," + answers + ",\"requestState\":\"" + state + "\"", ElicitationCapabilities ), response ) && -32602 == McpTestErrorCode( response ) );

            // another principal, on its own channel
            CMcpTestChannel* other = rig.AddChannel( COM::CMcpChannel::CHANNEL_STREAM, "local:someone-else" );
            ASSERT_TRUE( rig.Call( "7", McpTestRequest( "7", "tools/call", "\"name\":\"ask\",\"arguments\":{\"text\":\"x\"}," + answers + ",\"requestState\":\"" + state + "\"", ElicitationCapabilities ), response, 5000, other ) );
            ASSERT_TRUE( -32602 == McpTestErrorCode( response ) );

            // the right one still works, then it expires
            ASSERT_TRUE( rig.Call( "8", McpTestRequest( "8", "tools/call", "\"name\":\"ask\",\"arguments\":{\"text\":\"x\"}," + answers + ",\"requestState\":\"" + state + "\"", ElicitationCapabilities ), response ) );
            ASSERT_TRUE( ResultType( response ) == "complete" );
            MT::ThreadDelay( 200 );
            ASSERT_TRUE( rig.Call( "9", McpTestRequest( "9", "tools/call", "\"name\":\"ask\",\"arguments\":{\"text\":\"x\"}," + answers + ",\"requestState\":\"" + state + "\"", ElicitationCapabilities ), response ) );
            ASSERT_TRUE( -32602 == McpTestErrorCode( response ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: MRTR: missing client capabilities, state only, nothing at all, and the other methods" )
        try
        {
            CMcpTestRig rig;
            CORE::CString error;
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "ask", CMcpTestTool::ASK_NAME ) ) );
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "again", CMcpTestTool::STATE_ONLY ) ) );
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "nothing", CMcpTestTool::INPUT_NOTHING ) ) );
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "sample", CMcpTestTool::SAMPLING, COM::MCP_EXECUTION_THREAD_POOL ) ) );
            CORE::CDataNode response;

            // no elicitation capability: -32021, the request never goes out
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "tools/call", "\"name\":\"ask\",\"arguments\":{}" ), response ) && -32021 == McpTestErrorCode( response ) );
            const CORE::CDataNode* data = McpTestErrorData( response );
            const CORE::CDataNode* required = GUCEF_NULL != data ? data->FindChild( "requiredCapabilities" ) : GUCEF_NULL;
            ASSERT_TRUE( GUCEF_NULL != required && GUCEF_NULL != required->FindChild( "elicitation" ) );
            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "tools/call", "\"name\":\"ask\",\"arguments\":{}", "{\"elicitation\":{\"url\":{}}}" ), response ) && -32021 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "3", McpTestRequest( "3", "tools/call", "\"name\":\"ask\",\"arguments\":{}", "{\"elicitation\":{}}" ), response ) && ResultType( response ) == "input_required" );
            ASSERT_TRUE( rig.Call( "4", McpTestRequest( "4", "tools/call", "\"name\":\"sample\",\"arguments\":{}", "{\"elicitation\":{}}" ), response ) && -32021 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "5", McpTestRequest( "5", "tools/call", "\"name\":\"sample\",\"arguments\":{}", "{\"sampling\":{}}" ), response ) && ResultType( response ) == "input_required" );
            ASSERT_TRUE( McpTestResultAttr( response, "requestState" ) == "<none>" );

            // state only, the client retries right away
            ASSERT_TRUE( rig.Call( "6", McpTestRequest( "6", "tools/call", "\"name\":\"again\",\"arguments\":{}" ), response ) && ResultType( response ) == "input_required" );
            ASSERT_TRUE( GUCEF_NULL == McpTestResultChild( response, "inputRequests" ) );
            CORE::CString state = McpTestResultAttr( response, "requestState" );
            ASSERT_TRUE( rig.Call( "7", McpTestRequest( "7", "tools/call", "\"name\":\"again\",\"arguments\":{},\"requestState\":\"" + state + "\"" ), response ) );
            ASSERT_TRUE( McpTestFirstText( response.FindChild( "result" ) ) == "attempt 1" );

            // neither requests nor state: a server bug, refused
            ASSERT_TRUE( rig.Call( "8", McpTestRequest( "8", "tools/call", "\"name\":\"nothing\",\"arguments\":{}" ), response ) && -32603 == McpTestErrorCode( response ) );

            // resources/read and prompts/get take part as well
            CMcpTestResource* resource = GUCEF_NEW CMcpTestResource( "test://confirm", "confirm", 1000, true );
            ASSERT_TRUE( rig.server->GetResourceRegistry().RegisterResource( COM::CIMcpResourcePtr( resource ), error ) );
            ASSERT_TRUE( rig.Call( "9", McpTestRequest( "9", "resources/read", "\"uri\":\"test://confirm\"" ), response ) && ResultType( response ) == "input_required" );
            ASSERT_TRUE( McpTestResultAttr( response, "ttlMs" ) == "<none>" );
            state = McpTestResultAttr( response, "requestState" );
            ASSERT_TRUE( rig.Call( "10", McpTestRequest( "10", "resources/read", "\"uri\":\"test://confirm\",\"requestState\":\"" + state + "\"" ), response ) && ResultType( response ) == "complete" );
            ASSERT_TRUE( McpTestResultAttr( response, "ttlMs" ) == "1000" );     // a complete retry still carries its hints
            // a state of resources/read is no good for another resource
            ASSERT_TRUE( rig.server->GetResourceRegistry().RegisterResource( COM::CIMcpResourcePtr( GUCEF_NEW CMcpTestResource( "test://confirm2", "confirm2", 0, true ) ), error ) );
            ASSERT_TRUE( rig.Call( "11", McpTestRequest( "11", "resources/read", "\"uri\":\"test://confirm2\",\"requestState\":\"" + state + "\"" ), response ) && -32602 == McpTestErrorCode( response ) );

            CMcpTestPrompt* prompt = GUCEF_NEW CMcpTestPrompt( "careful" );
            prompt->askInput = true;
            ASSERT_TRUE( rig.server->GetPromptRegistry().Register( COM::CIMcpPromptPtr( prompt ), error ) );
            ASSERT_TRUE( rig.Call( "12", McpTestRequest( "12", "prompts/get", "\"name\":\"careful\",\"arguments\":{\"who\":\"Bo\"}" ), response ) && ResultType( response ) == "input_required" );
            state = McpTestResultAttr( response, "requestState" );
            ASSERT_TRUE( rig.Call( "13", McpTestRequest( "13", "prompts/get", "\"name\":\"careful\",\"arguments\":{\"who\":\"Al\"},\"requestState\":\"" + state + "\"" ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "14", McpTestRequest( "14", "prompts/get", "\"name\":\"careful\",\"arguments\":{\"who\":\"Bo\"},\"requestState\":\"" + state + "\"" ), response ) && ResultType( response ) == "complete" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Cancellation stops a pool task cooperatively and nothing follows" )
        try
        {
            CMcpTestRig rig;
            CMcpTestEventRecorder events( *rig.server );
            CMcpTestTool* count = GUCEF_NEW CMcpTestTool( "count", CMcpTestTool::COUNT, COM::MCP_EXECUTION_THREAD_POOL );
            count->steps = 400;
            count->stepMs = 5;
            ASSERT_TRUE( RegisterTestTool( rig, count ) );

            rig.Send( McpTestRequest( "\"c1\"", "tools/call", "\"name\":\"count\",\"arguments\":{}", "{}", "\"progressToken\":\"t1\"" ) );
            rig.Pulse( 50 );
            ASSERT_TRUE( 1 == rig.server->GetInFlightCount() );

            // unknown, malformed and wrongly typed references are ignored
            rig.Send( McpTestNotification( "notifications/cancelled", "\"requestId\":\"nobody\"" ) );
            rig.Send( McpTestNotification( "notifications/cancelled", "\"requestId\":true" ) );
            rig.Send( McpTestNotification( "notifications/cancelled" ) );
            rig.Send( McpTestNotification( "notifications/cancelled", "\"requestId\":1" ) );     // the integer 1 is not the string "c1"
            rig.Pulse( 5 );
            ASSERT_TRUE( 1 == rig.server->GetInFlightCount() );

            rig.Send( McpTestNotification( "notifications/cancelled", "\"requestId\":\"c1\",\"reason\":\"user\"" ) );
            ASSERT_TRUE( 0 == rig.server->GetInFlightCount() );
            for ( UInt32 i=0; i<3000 && 0 == count->sawCancellation.Load(); ++i )
                rig.Pulse();
            ASSERT_TRUE( 1 == count->sawCancellation.Load() );
            size_t sentAtCancel = rig.channel->sent.size();
            rig.Pulse( 100 );
            CORE::CDataNode response;
            ASSERT_FALSE( rig.FindResponse( "c1", response ) );
            ASSERT_TRUE( rig.channel->sent.size() == sentAtCancel );        // no progress, no response after the cancellation
            ASSERT_TRUE( 1 == events.CountOutcome( COM::MCP_OUTCOME_CANCELLED ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A request that runs too long is cancelled and answered with a timeout" )
        try
        {
            CMcpTestRig rig;
            CMcpTestEventRecorder events( *rig.server );
            CMcpTestTool* slow = GUCEF_NEW CMcpTestTool( "slow", CMcpTestTool::COUNT, COM::MCP_EXECUTION_THREAD_POOL );
            slow->steps = 1000;
            slow->stepMs = 5;
            slow->maxDurationMs = 100;
            CMcpTestTool* never = GUCEF_NEW CMcpTestTool( "never", CMcpTestTool::DEFERRED );
            never->maxDurationMs = 50;
            ASSERT_TRUE( RegisterTestTool( rig, slow ) && RegisterTestTool( rig, never ) );
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "tools/call", "\"name\":\"slow\",\"arguments\":{}" ), response ) );
            ASSERT_TRUE( -32603 == McpTestErrorCode( response ) && McpTestErrorMessage( response ).HasSubstr( "timed out" ) >= 0 );
            for ( UInt32 i=0; i<3000 && 0 == slow->sawCancellation.Load(); ++i )
                rig.Pulse();
            ASSERT_TRUE( 1 == slow->sawCancellation.Load() );
            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "tools/call", "\"name\":\"never\",\"arguments\":{}" ), response ) && -32603 == McpTestErrorCode( response ) );
            ASSERT_FALSE( never->CompleteLast( "too late" ) );
            ASSERT_TRUE( 2 == events.CountOutcome( COM::MCP_OUTCOME_TIMED_OUT ) );
            ASSERT_TRUE( 0 == rig.server->GetInFlightCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Progress is strictly increasing, coalesced, and never sent after the response" )
        try
        {
            CMcpTestRig rig;
            COM::CMcpServerSettings settings = rig.server->GetSettings();
            settings.progressMinIntervalMs = 40;
            rig.SetSettings( settings );
            CMcpTestTool* count = GUCEF_NEW CMcpTestTool( "count", CMcpTestTool::COUNT, COM::MCP_EXECUTION_THREAD_POOL );
            count->steps = 60;
            count->stepMs = 3;
            ASSERT_TRUE( RegisterTestTool( rig, count ) );
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "tools/call", "\"name\":\"count\",\"arguments\":{}", "{}", "\"progressToken\":42" ), response ) );
            ASSERT_TRUE( McpTestFirstText( response.FindChild( "result" ) ) == "counted" );
            rig.Pulse( 100 );

            GUCEF::vector< CORE::CDataNode > progress;
            UInt32 notifications = rig.GetNotifications( "notifications/progress", progress );
            ASSERT_TRUE( notifications >= 2 && notifications < 60 );        // coalesced
            Float64 last = -1.0;
            bool increasing = true;
            bool tokenOk = true;
            for ( size_t i=0; i<progress.size(); ++i )
            {
                const CORE::CDataNode* params = progress[ i ].FindChild( "params" );
                Float64 value = params->GetAttributeValue( "progress" ).AsFloat64();
                increasing = increasing && value > last;
                last = value;
                tokenOk = tokenOk && params->GetAttributeValue( "progressToken" ).AsInt32() == 42 && params->GetAttributeValue( "total" ).AsFloat64() == 60.0;
            }
            ASSERT_TRUE( increasing && tokenOk );
            // the response is the last message of the request
            GUCEF::Int32 responseIndex = rig.IndexOf( "\"id\":1" );
            GUCEF::Int32 lastProgressIndex = -1;
            for ( size_t i=0; i<rig.channel->sent.size(); ++i )
                if ( rig.channel->sent[ i ].HasSubstr( "notifications/progress" ) >= 0 )
                    lastProgressIndex = (GUCEF::Int32) i;
            ASSERT_TRUE( responseIndex > lastProgressIndex );
            ASSERT_TRUE( count->progressRefused.Load() >= 1 );     // the value that did not increase

            // without a token nothing is reported
            CMcpTestTool* quiet = GUCEF_NEW CMcpTestTool( "quiet", CMcpTestTool::COUNT, COM::MCP_EXECUTION_THREAD_POOL );
            quiet->steps = 5;
            quiet->stepMs = 1;
            ASSERT_TRUE( RegisterTestTool( rig, quiet ) );
            size_t before = rig.channel->sent.size();
            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "tools/call", "\"name\":\"quiet\",\"arguments\":{}" ), response ) );
            ASSERT_TRUE( rig.channel->sent.size() == before + 1 );
            ASSERT_TRUE( quiet->progressRefused.Load() == 6 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Subscriptions: acknowledgment first, ids on every notification, coalesced list changes" )
        try
        {
            CMcpTestRig rig;
            CORE::CString error;
            ASSERT_TRUE( rig.server->GetResourceRegistry().RegisterResource( COM::CIMcpResourcePtr( GUCEF_NEW CMcpTestResource( "test://readme", "readme" ) ), error ) );
            ASSERT_TRUE( rig.server->GetResourceRegistry().RegisterTemplate( COM::CIMcpResourceTemplatePtr( GUCEF_NEW CMcpTestTemplate( "items", "test://items/{id}" ) ), error ) );
            rig.Pulse( 2 );

            rig.Send( McpTestRequest( "\"sub-1\"", "subscriptions/listen", "\"notifications\":{\"toolsListChanged\":true,\"resourceSubscriptions\":[\"test://readme\",\"test://items/4\",\"test://unknown\"]}" ) );
            ASSERT_TRUE( 1 == rig.server->GetSubscriptionCount() );
            GUCEF::vector< CORE::CDataNode > found;
            ASSERT_TRUE( 1 == rig.GetNotifications( "notifications/subscriptions/acknowledged", found ) );
            const CORE::CDataNode* params = found[ 0 ].FindChild( "params" );
            ASSERT_TRUE( McpTestAttr( *params->FindChild( "_meta" ), "io.modelcontextprotocol/subscriptionId" ) == "sub-1" );
            const CORE::CDataNode* filter = params->FindChild( "notifications" );
            ASSERT_TRUE( GUCEF_NULL != filter && filter->GetAttributeValue( "toolsListChanged" ).AsBool() );
            ASSERT_TRUE( GUCEF_NULL != filter && GUCEF_NULL == filter->GetAttribute( "promptsListChanged" ) );
            ASSERT_TRUE( GUCEF_NULL != filter && 2 == McpTestChildCount( filter->FindChild( "resourceSubscriptions" ) ) );     // the unknown URI is left out

            // a burst of twenty registrations, as a plugin that loads: one notification
            for ( int i=0; i<20; ++i )
                RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( ( "burst" + CORE::ToString( i ) ).C_String(), CMcpTestTool::ECHO ) );
            rig.server->GetPromptRegistry().Register( COM::CIMcpPromptPtr( GUCEF_NEW CMcpTestPrompt( "p" ) ), error );     // not asked for
            rig.Pulse( 3 );
            ASSERT_TRUE( 1 == rig.GetNotifications( "notifications/tools/list_changed", found ) );
            ASSERT_TRUE( McpTestAttr( *found[ 0 ].FindChild( "params" )->FindChild( "_meta" ), "io.modelcontextprotocol/subscriptionId" ) == "sub-1" );
            ASSERT_TRUE( 0 == rig.GetNotifications( "notifications/prompts/list_changed", found ) );

            // resource updates, for the URIs of the subscription only
            rig.server->NotifyResourceUpdated( "test://readme" );
            rig.server->NotifyResourceUpdated( "test://items/9" );
            rig.Pulse( 2 );
            ASSERT_TRUE( 1 == rig.GetNotifications( "notifications/resources/updated", found ) );
            ASSERT_TRUE( McpTestAttr( *found[ 0 ].FindChild( "params" ), "uri" ) == "test://readme" );
            ASSERT_TRUE( McpTestAttr( *found[ 0 ].FindChild( "params" )->FindChild( "_meta" ), "io.modelcontextprotocol/subscriptionId" ) == "sub-1" );

            // a second subscription with an integer id, told apart by its id
            rig.Send( McpTestRequest( "7", "subscriptions/listen", "\"notifications\":{\"toolsListChanged\":true,\"promptsListChanged\":true}" ) );
            rig.server->GetToolRegistry().Unregister( "burst0" );
            rig.Pulse( 2 );
            ASSERT_TRUE( 3 == rig.GetNotifications( "notifications/tools/list_changed", found ) );
            if ( 3 == found.size() )
            {
                CORE::CString first = McpTestAttr( *found[ 1 ].FindChild( "params" )->FindChild( "_meta" ), "io.modelcontextprotocol/subscriptionId" );
                CORE::CString second = McpTestAttr( *found[ 2 ].FindChild( "params" )->FindChild( "_meta" ), "io.modelcontextprotocol/subscriptionId" );
                ASSERT_TRUE( ( first == "7" && second == "sub-1" ) || ( first == "sub-1" && second == "7" ) );
            }

            // the client ends one: nothing more for it
            rig.Send( McpTestNotification( "notifications/cancelled", "\"requestId\":\"sub-1\"" ) );
            ASSERT_TRUE( 1 == rig.server->GetSubscriptionCount() );
            rig.server->NotifyResourceUpdated( "test://readme" );
            rig.Pulse( 2 );
            ASSERT_TRUE( 1 == rig.GetNotifications( "notifications/resources/updated", found ) );

            // the server stops: the closing result, then notifications/cancelled on a stream
            rig.server->Stop( true );
            ASSERT_TRUE( 0 == rig.server->GetSubscriptionCount() );
            CORE::CDataNode response;
            ASSERT_TRUE( rig.FindResponse( "7", response ) && ResultType( response ) == "complete" );
            ASSERT_TRUE( GUCEF_NULL != McpTestResultChild( response, "_meta" ) && McpTestAttr( *McpTestResultChild( response, "_meta" ), "io.modelcontextprotocol/subscriptionId" ) == "7" );
            ASSERT_TRUE( 1 == rig.GetNotifications( "notifications/cancelled", found ) && McpTestAttr( *found[ 0 ].FindChild( "params" ), "requestId" ) == "7" );
            ASSERT_TRUE( rig.IndexOf( "notifications/cancelled" ) > rig.IndexOf( "\"id\":7" ) );
            ASSERT_FALSE( rig.FindResponse( "sub-1", response ) );
            ASSERT_TRUE( rig.AllSentAreJsonLines() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Subscriptions: malformed filters, limits and the policy" )
        try
        {
            CMcpTestRig rig;
            COM::CMcpServerSettings settings = rig.server->GetSettings();
            settings.maxSubscriptionsPerChannel = 1;
            rig.SetSettings( settings );
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "subscriptions/listen" ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "subscriptions/listen", "\"notifications\":{\"toolsListChanged\":\"yes\"}" ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "3", McpTestRequest( "3", "subscriptions/listen", "\"notifications\":{\"resourceSubscriptions\":\"test://a\"}" ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "4", McpTestRequest( "4", "subscriptions/listen", "\"notifications\":{\"resourceSubscriptions\":[1]}" ), response ) && -32602 == McpTestErrorCode( response ) );
            rig.Send( McpTestRequest( "5", "subscriptions/listen", "\"notifications\":{}" ) );
            ASSERT_TRUE( 1 == rig.server->GetSubscriptionCount() );
            ASSERT_TRUE( rig.Call( "6", McpTestRequest( "6", "subscriptions/listen", "\"notifications\":{}" ), response ) && -31002 == McpTestErrorCode( response ) );
            // the id of an open subscription is in use: refused without that id, which stays the subscription's
            const size_t beforeInUse = rig.channel->sent.size();
            rig.Send( McpTestRequest( "5", "tools/list" ) );
            rig.Pulse( 2 );
            ASSERT_TRUE( rig.channel->sent.size() == beforeInUse + 1 );
            if ( rig.channel->sent.size() == beforeInUse + 1 )
                ASSERT_TRUE( rig.channel->sent.back().HasSubstr( "\"id\":null" ) >= 0 && rig.channel->sent.back().HasSubstr( "-32600" ) >= 0 );

            CMcpTestPolicy* policy = GUCEF_NEW CMcpTestPolicy();
            policy->denySubscriptions = true;
            rig.server->SetAccessPolicy( COM::CIMcpAccessPolicyPtr( policy ) );
            CMcpTestChannel* other = rig.AddChannel( COM::CMcpChannel::CHANNEL_STREAM, "local:other" );
            ASSERT_TRUE( rig.Call( "8", McpTestRequest( "8", "subscriptions/listen", "\"notifications\":{}" ), response, 5000, other ) && -31003 == McpTestErrorCode( response ) );

            // a closed channel ends its subscriptions without a message
            rig.server->CloseChannel( rig.channelPtr );
            ASSERT_TRUE( 0 == rig.server->GetSubscriptionCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The default policy: read-only tools allowed, others hidden; a policy can allow them" )
        try
        {
            CMcpTestRig rig;
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "read", CMcpTestTool::ECHO ) ) );
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "add_note", CMcpTestTool::ECHO, COM::MCP_EXECUTION_CALLING_THREAD, COM::MCP_TOOLEFFECT_ADDITIVE ) ) );
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "wipe", CMcpTestTool::ECHO, COM::MCP_EXECUTION_CALLING_THREAD, COM::MCP_TOOLEFFECT_DESTRUCTIVE ) ) );
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "tools/list" ), response ) && ListNames( response, "tools" ) == "read" );
            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "tools/call", "\"name\":\"add_note\",\"arguments\":{\"text\":\"x\"}" ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( McpTestErrorMessage( response ) == "Unknown tool: add_note" );     // hidden: as if it did not exist

            // in-process callers get the same answer
            CORE::CDataNode arguments( GUCEF_DATATYPE_OBJECT );
            arguments.SetAttribute( "text", "x" );
            COM::CMcpToolResult result;
            CORE::CString error;
            ASSERT_FALSE( rig.server->CallTool( "wipe", arguments, COM::CMcpPrincipal(), result, error ) );
            ASSERT_TRUE( error == "Unknown tool: wipe" );

            // a policy that allows them for one principal only, and denies one visible tool
            CMcpTestPolicy* policy = GUCEF_NEW CMcpTestPolicy();
            policy->allowOnlySubject = "local:test";
            policy->denyTool = "wipe";
            rig.server->SetAccessPolicy( COM::CIMcpAccessPolicyPtr( policy ) );
            ASSERT_TRUE( rig.Call( "3", McpTestRequest( "3", "tools/list" ), response ) && ListNames( response, "tools" ) == "add_note,read,wipe" );
            ASSERT_TRUE( rig.Call( "4", McpTestRequest( "4", "tools/call", "\"name\":\"add_note\",\"arguments\":{\"text\":\"x\"}" ), response ) && McpTestFirstText( response.FindChild( "result" ) ) == "echo: x" );
            ASSERT_TRUE( rig.Call( "5", McpTestRequest( "5", "tools/call", "\"name\":\"wipe\",\"arguments\":{\"text\":\"x\"}" ), response ) );
            ASSERT_TRUE( McpTestResultAttr( response, "isError" ).Lowercase() == "true" && McpTestFirstText( response.FindChild( "result" ) ) == "Access denied: the test denies it" );
            CMcpTestChannel* other = rig.AddChannel( COM::CMcpChannel::CHANNEL_STREAM, "local:other" );
            ASSERT_TRUE( rig.Call( "6", McpTestRequest( "6", "tools/list" ), response, 5000, other ) && ListNames( response, "tools" ) == "read" );
            ASSERT_TRUE( rig.server->CallTool( "wipe", arguments, COM::CMcpPrincipal( "in-process", "local:test", true ), result, error ) );
            ASSERT_TRUE( result.isError );
            ASSERT_FALSE( rig.server->CallTool( "add_note", arguments, COM::CMcpPrincipal( "in-process", "local:other", true ), result, error ) );

            // back to the default
            rig.server->SetAccessPolicy( COM::CIMcpAccessPolicyPtr() );
            ASSERT_TRUE( rig.Call( "7", McpTestRequest( "7", "tools/list" ), response ) && ListNames( response, "tools" ) == "read" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: In-process calls: blocking on the calling thread and asynchronous with a future" )
        try
        {
            CMcpTestRig rig;
            CMcpTestTool* deferred = GUCEF_NEW CMcpTestTool( "later", CMcpTestTool::DEFERRED, COM::MCP_EXECUTION_THREAD_POOL );
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "echo", CMcpTestTool::ECHO, COM::MCP_EXECUTION_THREAD_POOL ) ) );
            ASSERT_TRUE( RegisterTestTool( rig, deferred ) );
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "ask", CMcpTestTool::ASK_NAME ) ) );
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "sum", CMcpTestTool::STRUCTURED ) ) );
            CORE::CString error;
            ASSERT_TRUE( rig.server->GetResourceRegistry().RegisterResource( COM::CIMcpResourcePtr( GUCEF_NEW CMcpTestResource( "test://readme", "readme" ) ), error ) );
            ASSERT_TRUE( rig.server->GetPromptRegistry().Register( COM::CIMcpPromptPtr( GUCEF_NEW CMcpTestPrompt( "greet" ) ), error ) );
            CMcpTestEventRecorder events( *rig.server );

            CORE::CDataNode arguments( GUCEF_DATATYPE_OBJECT );
            arguments.SetAttribute( "text", "inside" );
            COM::CMcpToolResult result;
            ASSERT_TRUE( rig.server->CallTool( "echo", arguments, COM::CMcpPrincipal(), result, error ) );     // runs here, whatever its mode
            ASSERT_TRUE( 1 == result.content.size() && result.content[ 0 ].text == "echo: inside" );
            CORE::CDataNode bad( GUCEF_DATATYPE_OBJECT );
            bad.SetAttribute( "text", (Int32) 1 );
            ASSERT_TRUE( rig.server->CallTool( "echo", bad, COM::CMcpPrincipal(), result, error ) && result.isError );
            CORE::CDataNode notAnObject( GUCEF_DATATYPE_ARRAY );
            ASSERT_FALSE( rig.server->CallTool( "echo", notAnObject, COM::CMcpPrincipal(), result, error ) );
            ASSERT_FALSE( rig.server->CallTool( "nope", arguments, COM::CMcpPrincipal(), result, error ) );
            CORE::CDataNode sum;
            McpTestParse( "{\"a\":1,\"b\":2}", sum );
            ASSERT_TRUE( rig.server->CallTool( "sum", sum, COM::CMcpPrincipal(), result, error ) && result.hasStructuredContent );

            // a deferred tool is driven by a local pulse generator of this thread until another thread completes it
            CCompleteThread completer( *deferred );
            ASSERT_TRUE( completer.Start() );
            ASSERT_TRUE( rig.server->CallTool( "later", arguments, COM::CMcpPrincipal(), result, error ) );
            ASSERT_TRUE( 1 == result.content.size() && result.content[ 0 ].text == "completed elsewhere" );
            ASSERT_TRUE( completer.Join( GUCEF_NULL ) );

            // no client capabilities: a tool that needs input fails
            ASSERT_FALSE( rig.server->CallTool( "ask", arguments, COM::CMcpPrincipal(), result, error ) );
            ASSERT_TRUE( error.HasSubstr( "-32021" ) >= 0 && error.HasSubstr( "elicitation" ) >= 0 );

            COM::CMcpResourceContents contents;
            ASSERT_TRUE( rig.server->ReadResource( "test://readme", COM::CMcpPrincipal(), contents, error ) && 1 == contents.items.size() );
            ASSERT_FALSE( rig.server->ReadResource( "test://none", COM::CMcpPrincipal(), contents, error ) );
            COM::CMcpPromptResult prompt;
            CORE::CValueList promptArguments;
            promptArguments.Set( "who", CORE::CVariant( "Ada" ) );
            ASSERT_TRUE( rig.server->GetPrompt( "greet", promptArguments, COM::CMcpPrincipal(), prompt, error ) && 2 == prompt.messages.size() );
            ASSERT_FALSE( rig.server->GetPrompt( "greet", CORE::CValueList(), COM::CMcpPrincipal(), prompt, error ) );

            // the asynchronous forms: the same work on the pool, the outcome in the future
            CORE::CFutureResult toolFuture = rig.server->CallToolAsync( "echo", arguments, COM::CMcpPrincipal() );
            CORE::CFutureResult readFuture = rig.server->ReadResourceAsync( "test://readme", COM::CMcpPrincipal() );
            CORE::CFutureResult promptFuture = rig.server->GetPromptAsync( "greet", promptArguments, COM::CMcpPrincipal() );
            CORE::CFutureResult unknownFuture = rig.server->CallToolAsync( "nope", arguments, COM::CMcpPrincipal() );
            ASSERT_TRUE( toolFuture.HasAFuture() && readFuture.HasAFuture() && promptFuture.HasAFuture() );
            toolFuture.Await( 10000 );
            readFuture.Await( 10000 );
            promptFuture.Await( 10000 );
            unknownFuture.Await( 10000 );
            result.Clear();
            ASSERT_TRUE( COM::CMcpServer::TryGetToolResult( toolFuture, result, error ) && 1 == result.content.size() && result.content[ 0 ].text == "echo: inside" );
            ASSERT_TRUE( COM::CMcpServer::TryGetResourceContents( readFuture, contents, error ) && 1 == contents.items.size() );
            ASSERT_TRUE( COM::CMcpServer::TryGetPromptResult( promptFuture, prompt, error ) && 2 == prompt.messages.size() );
            ASSERT_FALSE( COM::CMcpServer::TryGetToolResult( unknownFuture, result, error ) );
            ASSERT_TRUE( error == "Unknown tool: nope" );
            ASSERT_FALSE( COM::CMcpServer::TryGetResourceContents( toolFuture, contents, error ) );      // not a future of that call

            // every call is one request: started and finished; those of pool threads come on the next pulse
            ASSERT_TRUE( events.started == events.finished && events.finished == 11 );
            rig.Pulse( 2 );
            ASSERT_TRUE( events.started == events.finished && events.finished == 15 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Limits: requests in flight, the tool call rate and the nesting pre-scan" )
        try
        {
            CMcpTestRig rig;
            COM::CMcpServerSettings settings = rig.server->GetSettings();
            settings.maxInFlightPerChannel = 1;
            settings.toolCallsPerSecond = 1;
            settings.toolCallBurst = 2;
            settings.maxMessageBytes = 2048;
            settings.maxJsonDepth = 16;
            rig.SetSettings( settings );
            CMcpTestTool* deferred = GUCEF_NEW CMcpTestTool( "later", CMcpTestTool::DEFERRED );
            ASSERT_TRUE( RegisterTestTool( rig, deferred ) );
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "echo", CMcpTestTool::ECHO ) ) );

            CORE::CDataNode response;
            rig.Send( McpTestRequest( "1", "tools/call", "\"name\":\"later\",\"arguments\":{}" ) );
            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"x\"}" ), response ) && -31002 == McpTestErrorCode( response ) );
            ASSERT_TRUE( deferred->CompleteLast( "ok" ) );
            ASSERT_TRUE( rig.WaitForResponse( "1", response ) );
            // the burst is used up: the next call is rate limited
            ASSERT_TRUE( rig.Call( "3", McpTestRequest( "3", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"x\"}" ), response ) && -31001 == McpTestErrorCode( response ) );
            // another principal has a bucket of its own
            CMcpTestChannel* other = rig.AddChannel( COM::CMcpChannel::CHANNEL_STREAM, "local:other" );
            ASSERT_TRUE( rig.Call( "4", McpTestRequest( "4", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"x\"}" ), response, 5000, other ) && ResultType( response ) == "complete" );
            MT::ThreadDelay( 1100 );
            ASSERT_TRUE( rig.Call( "5", McpTestRequest( "5", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"x\"}" ), response ) && ResultType( response ) == "complete" );

            // nesting beyond the limit never reaches the parser: -32700 without an id
            CORE::CString deep = "{\"jsonrpc\":\"2.0\",\"id\":6,\"method\":\"tools/list\",\"params\":{\"x\":";
            for ( int i=0; i<20; ++i )
                deep += '[';
            for ( int i=0; i<20; ++i )
                deep += ']';
            deep += "}}";
            size_t before = rig.channel->sent.size();
            ASSERT_FALSE( rig.Send( deep ) );
            ASSERT_TRUE( rig.channel->sent.size() == before + 1 );
            CORE::CDataNode message;
            ASSERT_TRUE( McpTestParse( rig.channel->sent.back(), message ) && -32700 == McpTestErrorCode( message ) );
            ASSERT_TRUE( rig.channel->sent.back().HasSubstr( "\"id\":null" ) >= 0 );

            // too large: -32600 without an id
            CORE::CString large = McpTestRequest( "7", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"" + CORE::CString( "x" ) + "\"}" );
            CORE::CString padding;
            for ( int i=0; i<3000; ++i )
                padding += ' ';
            ASSERT_FALSE( rig.Send( large + padding ) );
            ASSERT_TRUE( McpTestParse( rig.channel->sent.back(), message ) && -32600 == McpTestErrorCode( message ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Malformed input: garbage, batches, bad ids and shapes, random bytes" )
        try
        {
            CMcpTestRig rig;
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "echo", CMcpTestTool::ECHO ) ) );
            struct SCase { const char* text; Int32 code; };
            SCase cases[] = {
                { "this is not json", -32700 },
                { "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":", -32700 },
                { "[]", -32600 },
                { "[{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/list\"}]", -32600 },
                { "{\"jsonrpc\":\"2.0\",\"id\":null,\"method\":\"tools/list\"}", -32600 },
                { "{\"jsonrpc\":\"2.0\",\"id\":1.5,\"method\":\"tools/list\"}", -32600 },
                { "{\"jsonrpc\":\"2.0\",\"id\":true,\"method\":\"tools/list\"}", -32600 },
                { "{\"jsonrpc\":\"2.0\",\"id\":99999999999999999999,\"method\":\"tools/list\"}", -32600 },
                { "{\"jsonrpc\":\"2.0\",\"id\":{\"a\":1},\"method\":\"tools/list\"}", -32600 },
                { "{\"jsonrpc\":\"2.0\",\"id\":[1],\"method\":\"tools/list\"}", -32600 },
                { "{\"jsonrpc\":\"1.0\",\"id\":1,\"method\":\"tools/list\"}", -32600 },
                { "{\"id\":1,\"method\":\"tools/list\"}", -32600 },
                { "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":5}", -32600 },
                { "{\"jsonrpc\":\"2.0\",\"id\":1}", -32600 },
                { "5", 0 },
                { "\"text\"", 0 },
                { "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/list\",\"params\":5}", -32602 },
                { "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/list\",\"params\":[1,2]}", -32602 } };
            for ( UInt32 i=0; i<sizeof( cases ) / sizeof( cases[ 0 ] ); ++i )
            {
                size_t before = rig.channel->sent.size();
                rig.Send( cases[ i ].text );
                rig.Pulse();
                ASSERT_TRUE( rig.channel->sent.size() == before + 1 );
                CORE::CDataNode message;
                bool parsed = rig.channel->sent.size() > before && McpTestParse( rig.channel->sent[ before ], message );
                ASSERT_TRUE( parsed );
                // a bare scalar is no message: the codec either refuses it (-32700) or gives no object (-32600)
                Int32 code = McpTestErrorCode( message );
                bool expected = 0 == cases[ i ].code ? ( -32700 == code || -32600 == code ) : code == cases[ i ].code;
                if ( !expected )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpCore malformed input case " << i << " got " << code << " for " << cases[ i ].text );
                ASSERT_TRUE( expected );
            }

            // notifications and client responses get no answer
            size_t before = rig.channel->sent.size();
            rig.Send( McpTestNotification( "notifications/unknown" ) );
            rig.Send( "{\"jsonrpc\":\"2.0\",\"id\":77,\"result\":{}}" );
            rig.Send( "{\"jsonrpc\":\"2.0\",\"id\":78,\"error\":{\"code\":1,\"message\":\"x\"}}" );
            rig.Pulse( 2 );
            ASSERT_TRUE( rig.channel->sent.size() == before );

            // a message whose members are nested deeper than it says does not mislead the lookup of params
            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "90", "{\"jsonrpc\":\"2.0\",\"id\":90,\"method\":\"tools/list\",\"extra\":{\"params\":{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\",\"io.modelcontextprotocol/clientCapabilities\":{}}}}}", response ) );
            ASSERT_TRUE( -32602 == McpTestErrorCode( response ) );

            // random bytes, and random cuts of a valid request: every answer is a JSON line, nothing crashes
            CORE::CString valid = McpTestRequest( "100", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"fuzz\"}" );
            UInt32 seed = 12345;
            for ( UInt32 round=0; round<400; ++round )
            {
                seed = seed * 1103515245 + 12345;
                CORE::CString text;
                if ( 0 == round % 2 )
                {
                    UInt32 length = 1 + ( seed >> 16 ) % 64;
                    for ( UInt32 n=0; n<length; ++n )
                    {
                        seed = seed * 1103515245 + 12345;
                        char c = (char) ( 1 + ( ( seed >> 16 ) % 126 ) );
                        text += c;
                    }
                }
                else
                {
                    UInt32 cut = ( seed >> 16 ) % ( valid.Length() );
                    text = valid.SubstrFromRange( 0, cut );
                    seed = seed * 1103515245 + 12345;
                    text += (char) ( '!' + ( ( seed >> 16 ) % 90 ) );
                    text += valid.SubstrFromRange( cut, valid.Length() );
                }
                rig.Send( text );
            }
            rig.Pulse( 5 );
            ASSERT_TRUE( rig.AllSentAreJsonLines() );
            ASSERT_TRUE( 0 == rig.server->GetInFlightCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Request ids: in use, strings and integers told apart, events per request" )
        try
        {
            CMcpTestRig rig;
            CMcpTestEventRecorder events( *rig.server );
            CMcpTestTool* deferred = GUCEF_NEW CMcpTestTool( "later", CMcpTestTool::DEFERRED );
            ASSERT_TRUE( RegisterTestTool( rig, deferred ) );
            rig.Send( McpTestRequest( "\"1\"", "tools/call", "\"name\":\"later\",\"arguments\":{}" ) );
            CORE::CDataNode response;
            // the integer 1 is another id than the string "1"
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "tools/list" ), response ) && ResultType( response ) == "complete" );
            // the string "1" is in use
            size_t before = rig.channel->sent.size();
            rig.Send( McpTestRequest( "\"1\"", "tools/list" ) );
            ASSERT_TRUE( rig.channel->sent.size() == before + 1 );
            CORE::CDataNode message;
            ASSERT_TRUE( McpTestParse( rig.channel->sent.back(), message ) && -32600 == McpTestErrorCode( message ) );
            ASSERT_TRUE( deferred->CompleteLast( "x" ) );
            rig.Pulse( 3 );
            ASSERT_TRUE( events.started == 3 && events.finished == 3 );
            ASSERT_TRUE( 1 == events.CountOutcome( COM::MCP_OUTCOME_PROTOCOL_ERROR ) );
            ASSERT_TRUE( events.outcomes.back().method == "tools/call" && events.outcomes.back().target == "later" );
            ASSERT_TRUE( events.outcomes.back().requestId == "1" && events.outcomes.back().subject == "local:test" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A closed channel cancels its modern requests and drops their results" )
        try
        {
            CMcpTestRig rig;
            CMcpTestEventRecorder events( *rig.server );
            CMcpTestTool* count = GUCEF_NEW CMcpTestTool( "count", CMcpTestTool::COUNT, COM::MCP_EXECUTION_THREAD_POOL );
            count->steps = 400;
            ASSERT_TRUE( RegisterTestTool( rig, count ) );
            CMcpTestChannel* other = rig.AddChannel( COM::CMcpChannel::CHANNEL_STREAM, "local:other" );
            rig.Send( McpTestRequest( "1", "tools/call", "\"name\":\"count\",\"arguments\":{}" ), other );
            rig.Pulse( 20 );
            ASSERT_TRUE( 1 == rig.server->GetInFlightCount() && 2 == rig.server->GetChannelCount() );
            rig.server->CloseChannel( rig.extraChannels.back() );
            ASSERT_TRUE( 0 == rig.server->GetInFlightCount() && 1 == rig.server->GetChannelCount() );
            ASSERT_TRUE( 1 == events.channelsClosed );
            for ( UInt32 i=0; i<3000 && 0 == count->sawCancellation.Load(); ++i )
                rig.Pulse();
            ASSERT_TRUE( 1 == count->sawCancellation.Load() );
            ASSERT_FALSE( rig.Send( McpTestRequest( "2", "tools/list" ), other ) );        // nothing is taken in on a closed channel
            ASSERT_TRUE( 1 == events.CountOutcome( COM::MCP_OUTCOME_CANCELLED ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Concurrency: registrations, in-process calls and pool work on several threads at once" )
        try
        {
            CMcpTestRig rig;
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "echo", CMcpTestTool::ECHO ) ) );
            CMcpTestTool* count = GUCEF_NEW CMcpTestTool( "count", CMcpTestTool::COUNT, COM::MCP_EXECUTION_THREAD_POOL );
            count->steps = 20;
            count->stepMs = 1;
            ASSERT_TRUE( RegisterTestTool( rig, count ) );
            CORE::CString error;
            ASSERT_TRUE( rig.server->GetResourceRegistry().RegisterResource( COM::CIMcpResourcePtr( GUCEF_NEW CMcpTestResource( "test://readme", "readme" ) ), error ) );
            COM::CMcpServerSettings settings = rig.server->GetSettings();
            settings.toolCallsPerSecond = 0;
            settings.progressMinIntervalMs = 1;
            rig.SetSettings( settings );
            rig.Send( McpTestRequest( "\"s\"", "subscriptions/listen", "\"notifications\":{\"toolsListChanged\":true,\"resourceSubscriptions\":[\"test://readme\"]}" ) );

            CRegisterThread registrar1( *rig.server, 1 );
            CRegisterThread registrar2( *rig.server, 2 );
            CCallToolThread caller1( *rig.server, "echo", 100 );
            CCallToolThread caller2( *rig.server, "echo", 100 );
            ASSERT_TRUE( registrar1.Start() && registrar2.Start() && caller1.Start() && caller2.Start() );

            // meanwhile this thread lists and calls over the channel, and pool tasks report progress
            UInt32 answered = 0;
            for ( UInt32 i=0; i<40; ++i )
            {
                CORE::CString id = CORE::ToString( 1000 + i );
                CORE::CDataNode response;
                if ( 0 == i % 2 )
                    rig.Call( id, McpTestRequest( id, "tools/list" ), response );
                else
                    rig.Call( id, McpTestRequest( id, "tools/call", "\"name\":\"count\",\"arguments\":{}", "{}", "\"progressToken\":" + id ), response );
                if ( ResultType( response ) == "complete" )
                    ++answered;
            }
            ASSERT_TRUE( registrar1.Join( &rig ) && registrar2.Join( &rig ) && caller1.Join( &rig ) && caller2.Join( &rig ) );
            rig.Pulse( 10 );
            ASSERT_TRUE( 40 == answered );
            ASSERT_TRUE( 100 == caller1.succeeded.Load() && 100 == caller2.succeeded.Load() );
            ASSERT_TRUE( 0 == caller1.wrongText.Load() && 0 == caller2.wrongText.Load() );
            ASSERT_TRUE( 50 == registrar1.registered.Load() && 50 == registrar2.registered.Load() );
            ASSERT_TRUE( 2 + 2 * ( 50 - 17 ) == rig.server->GetToolRegistry().GetCount() );
            GUCEF::vector< CORE::CDataNode > found;
            ASSERT_TRUE( rig.GetNotifications( "notifications/resources/updated", found ) >= 1 );
            ASSERT_TRUE( rig.AllSentAreJsonLines() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
    GUCEF_TESTFW_TESTCASE( "Test: A member name that occurs twice in an object of a request is refused, and the tool never runs" )
        try
        {
            CMcpTestRig rig;
            CMcpTestTool* echo = GUCEF_NEW CMcpTestTool( "echo", CMcpTestTool::ECHO );
            ASSERT_TRUE( RegisterTestTool( rig, echo ) );
            CORE::CDataNode response;

            // in the arguments: the schema would only have seen one of the two
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"a\",\"text\":{\"x\":1}}" ), response ) );
            ASSERT_TRUE( -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( McpTestErrorMessage( response ).HasSubstr( "\"text\"" ) >= 0 );

            // the same name written with an escape is the same name
            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"a\",\"t\\u0065xt\":\"b\"}" ), response ) );
            ASSERT_TRUE( -32602 == McpTestErrorCode( response ) );

            // in the envelope: no valid request
            ASSERT_TRUE( rig.Call( "3", "{\"jsonrpc\":\"2.0\",\"id\":3,\"method\":\"tools/list\",\"method\":\"tools/call\",\"params\":{" + McpTestMeta() + ",\"name\":\"echo\",\"arguments\":{\"text\":\"x\"}}}", response ) );
            ASSERT_TRUE( -32600 == McpTestErrorCode( response ) );
            ASSERT_TRUE( 0 == echo->executions.Load() );

            // the same name in two different objects is fine
            ASSERT_TRUE( rig.Call( "4", McpTestRequest( "4", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"fine\"}", "{\"name\":{}}" ), response ) );
            ASSERT_TRUE( McpTestFirstText( ResultOf( response ) ) == "echo: fine" );
            ASSERT_TRUE( rig.AllSentAreJsonLines() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A message that is not UTF-8 is a parse error; UTF-8 of any length passes" )
        try
        {
            CMcpTestRig rig;
            CMcpTestTool* echo = GUCEF_NEW CMcpTestTool( "echo", CMcpTestTool::ECHO );
            ASSERT_TRUE( RegisterTestTool( rig, echo ) );
            CORE::CDataNode response;

            // bytes that are no UTF-8, an overlong form of '/', an encoded surrogate, and a cut sequence
            const char* invalid[] = { "\xff\xfe", "\xc0\xaf", "\xed\xa0\x80", "\xe2\x82", "a\x80" };
            for ( UInt32 i=0; i<5; ++i )
            {
                const size_t before = rig.channel->sent.size();
                rig.Send( McpTestRequest( "1", "tools/call", CORE::CString( "\"name\":\"echo\",\"arguments\":{\"text\":\"" ) + invalid[ i ] + "\"}" ) );
                rig.Pulse( 5 );
                ASSERT_TRUE( rig.channel->sent.size() == before + 1 );
                if ( rig.channel->sent.size() == before + 1 )
                {
                    CORE::CDataNode error;
                    ASSERT_TRUE( McpTestParse( rig.channel->sent.back(), error ) );
                    ASSERT_TRUE( -32700 == McpTestErrorCode( error ) );
                    ASSERT_TRUE( rig.channel->sent.back().HasSubstr( "\"id\":null" ) >= 0 );
                }
            }
            ASSERT_TRUE( 0 == echo->executions.Load() );

            // two, three and four byte sequences
            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"caf\xc3\xa9 \xe2\x82\xac \xf0\x9f\x98\x80\"}" ), response ) );
            ASSERT_TRUE( McpTestFirstText( ResultOf( response ) ) == "echo: caf\xc3\xa9 \xe2\x82\xac \xf0\x9f\x98\x80" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A string with U+0000 or an unpaired surrogate is refused instead of being cut short or mangled" )
        try
        {
            CMcpTestRig rig;
            CMcpTestTool* echo = GUCEF_NEW CMcpTestTool( "echo", CMcpTestTool::ECHO );
            ASSERT_TRUE( RegisterTestTool( rig, echo ) );
            CORE::CDataNode response;

            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"c\\u0000d\"}" ), response ) );
            ASSERT_TRUE( -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( McpTestErrorMessage( response ).HasSubstr( "U+0000" ) >= 0 );
            // the parser refuses a lone surrogate itself (-32700); a high surrogate followed by another escape it takes, the
            // check before the parser refuses it (-32602)
            const char* surrogates[] = { "\\ud800", "\\udc00x", "\\ud800\\u0041" };
            const char* codes[] = { "-32700", "-32700", "-32602" };
            for ( UInt32 i=0; i<3; ++i )
            {
                const size_t before = rig.channel->sent.size();
                rig.Send( McpTestRequest( "2", "tools/call", CORE::CString( "\"name\":\"echo\",\"arguments\":{\"text\":\"" ) + surrogates[ i ] + "\"}" ) );
                rig.Pulse( 2 );
                ASSERT_TRUE( rig.channel->sent.size() == before + 1 && rig.channel->sent.back().HasSubstr( codes[ i ] ) >= 0 );
            }
            ASSERT_TRUE( 0 == echo->executions.Load() );

            // an escaped backslash before u0000 is no escape of U+0000, a surrogate pair is one character
            ASSERT_TRUE( rig.Call( "4", McpTestRequest( "4", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"a\\\\u0000b\"}" ), response ) );
            ASSERT_TRUE( McpTestFirstText( ResultOf( response ) ) == "echo: a\\u0000b" );
            ASSERT_TRUE( rig.Call( "5", McpTestRequest( "5", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"\\ud83d\\ude00\"}" ), response ) );
            ASSERT_TRUE( McpTestFirstText( ResultOf( response ) ) == "echo: \xf0\x9f\x98\x80" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An id that is in use by a request in progress gets an error without that id; only the first request answers with it" )
        try
        {
            CMcpTestRig rig;
            CMcpTestTool* slow = GUCEF_NEW CMcpTestTool( "slow", CMcpTestTool::COUNT, COM::MCP_EXECUTION_THREAD_POOL );
            slow->steps = 30;
            slow->stepMs = 10;
            ASSERT_TRUE( RegisterTestTool( rig, slow ) );
            ASSERT_TRUE( RegisterTestTool( rig, GUCEF_NEW CMcpTestTool( "echo", CMcpTestTool::ECHO ) ) );

            rig.Send( McpTestRequest( "9", "tools/call", "\"name\":\"slow\",\"arguments\":{}" ) );
            rig.Pulse( 2 );
            const size_t before = rig.channel->sent.size();
            rig.Send( McpTestRequest( "9", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"second\"}" ) );
            rig.Pulse( 2 );
            ASSERT_TRUE( rig.channel->sent.size() == before + 1 );
            if ( rig.channel->sent.size() == before + 1 )
            {
                CORE::CDataNode error;
                ASSERT_TRUE( McpTestParse( rig.channel->sent.back(), error ) );
                ASSERT_TRUE( -32600 == McpTestErrorCode( error ) );
                ASSERT_TRUE( rig.channel->sent.back().HasSubstr( "\"id\":null" ) >= 0 );
            }

            CORE::CDataNode response;
            ASSERT_TRUE( rig.WaitForResponse( "9", response, 10000 ) );
            ASSERT_TRUE( GUCEF_NULL != ResultOf( response ) );
            UInt32 withId = 0;
            for ( size_t i=0; i<rig.channel->sent.size(); ++i )
                if ( rig.channel->sent[ i ].HasSubstr( "\"id\":9," ) >= 0 || rig.channel->sent[ i ].HasSubstr( "\"id\":9}" ) >= 0 )
                    ++withId;
            ASSERT_TRUE( 1 == withId );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The smallest Int64 is a valid request id and comes back as it was sent" )
        try
        {
            CMcpTestRig rig;
            rig.Send( McpTestRequest( "-9223372036854775808", "tools/list" ) );
            rig.Pulse( 5 );
            const GUCEF::Int32 index = rig.IndexOf( "\"id\":-9223372036854775808" );
            ASSERT_TRUE( index >= 0 );
            if ( index >= 0 )
                ASSERT_TRUE( rig.channel->sent[ index ].HasSubstr( "\"result\"" ) >= 0 );

            // the largest one, and one beyond the range, which is refused
            rig.Send( McpTestRequest( "9223372036854775807", "tools/list" ) );
            rig.Pulse( 5 );
            ASSERT_TRUE( rig.IndexOf( "\"id\":9223372036854775807" ) >= 0 );
            const size_t before = rig.channel->sent.size();
            rig.Send( McpTestRequest( "-9223372036854775809", "tools/list" ) );
            rig.Pulse( 5 );
            ASSERT_TRUE( rig.channel->sent.size() == before + 1 && rig.channel->sent.back().HasSubstr( "-32600" ) >= 0 );
            rig.Send( McpTestRequest( "-9223372036854775808.5", "tools/list" ) );
            rig.Pulse( 5 );
            ASSERT_TRUE( rig.channel->sent.size() == before + 2 && rig.channel->sent.back().HasSubstr( "\"error\"" ) >= 0 && rig.channel->sent.back().HasSubstr( "\"result\"" ) < 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Strict JSON: a corpus of texts RFC 8259 refuses is a parse error, the texts it accepts are served" )
        try
        {
            CMcpTestRig rig;
            const CORE::CString head = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/list\",\"params\":{" + McpTestMeta() + ",\"x\":";

            // n_: no JSON, in the style of JSONTestSuite; the vendored parser takes several of them
            const char* refused[] = {
                "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/list\",}",           // trailing comma in an object
                "[1,]", "[1,,2]", "[,1]", "{\"a\":1,,\"b\":2}", "{,\"a\":1}",       // commas
                "/* comment */1", "1 // comment", "[1]/**/",                           // comments
                "'single'", "{'a':1}",                                                  // single quotes
                "NaN", "-Infinity", "Infinity", "nan", "True", "nul",                  // no literals of JSON
                "01", "-01", "00", "1.", ".1", "1e", "1e+", "+1", "0x10", "- 1", "1.2.3", "--1",   // numbers
                "\"a\x01" "b\"", "\"tab\there\"", "\"line\nbreak\"",                   // raw control characters in strings
                "\"\\x41\"", "\"\\u12\"", "\"\\u12g4\"", "\"\\a\"", "\"\\",            // escapes
                "{\"a\" 1}", "{\"a\":1 \"b\":2}", "{1:2}", "{\"a\"}", "[1 2]",         // structure
                "\"unterminated", "[1", "{\"a\":1", "1]", "{}}" };
            for ( UInt32 i=0; i<sizeof( refused ) / sizeof( refused[ 0 ] ); ++i )
            {
                // the first is a whole message, the others the value of a member of params
                const CORE::CString text = 0 == i ? CORE::CString( refused[ i ] ) : head + refused[ i ] + "}}";
                const size_t before = rig.channel->sent.size();
                rig.Send( text );
                rig.Pulse( 2 );
                const bool parseError = rig.channel->sent.size() == before + 1 && rig.channel->sent.back().HasSubstr( "-32700" ) >= 0 &&
                                        rig.channel->sent.back().HasSubstr( "\"id\":null" ) >= 0;
                if ( !parseError )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpCore test: refused corpus case " << i << " was taken: " << ( rig.channel->sent.size() > before ? rig.channel->sent.back() : CORE::CString( "<nothing sent>" ) ) );
                ASSERT_TRUE( parseError );
            }
            // trailing text after the message
            const size_t beforeTrailing = rig.channel->sent.size();
            rig.Send( McpTestRequest( "1", "tools/list" ) + " x" );
            rig.Pulse( 2 );
            ASSERT_TRUE( rig.channel->sent.size() == beforeTrailing + 1 && rig.channel->sent.back().HasSubstr( "-32700" ) >= 0 );

            // y_: JSON, served
            const char* accepted[] = {
                "[]", "{}", "[[],{}]", "0", "-0", "0.5", "-0.5e-3", "1E+2", "1e2", "123456789", "-1.25E-10", "true", "false", "null",
                "\"\\\"\\\\\\/\\b\\f\\n\\r\\t\\u00e9\\uD83D\\uDE00\"", "\"caf\xc3\xa9\"", " \t\r\n[ 1 , 2 ] \t\r\n", "{\"a\":{\"b\":[1,{\"c\":null}]}}" };
            for ( UInt32 i=0; i<sizeof( accepted ) / sizeof( accepted[ 0 ] ); ++i )
            {
                const CORE::CString id = CORE::ToString( 100 + i );
                CORE::CDataNode response;
                const bool served = rig.Call( id, "{\"jsonrpc\":\"2.0\",\"id\":" + id + ",\"method\":\"tools/list\",\"params\":{" + McpTestMeta() + ",\"x\":" + accepted[ i ] + "}}", response, 2000 );
                if ( !served || GUCEF_NULL == response.FindChild( "result" ) )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpCore test: accepted corpus case " << i << " was refused" );
                ASSERT_TRUE( served && GUCEF_NULL != response.FindChild( "result" ) );
            }
            ASSERT_TRUE( rig.AllSentAreJsonLines() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: tools/call without arguments is judged as {} by the input schema" )
        try
        {
            CMcpTestRig rig;
            CMcpTestTool* echo = GUCEF_NEW CMcpTestTool( "echo", CMcpTestTool::ECHO );
            ASSERT_TRUE( RegisterTestTool( rig, echo ) );
            CORE::CDataNode withoutArguments;
            CORE::CDataNode emptyArguments;
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "tools/call", "\"name\":\"echo\"" ), withoutArguments ) );
            ASSERT_TRUE( rig.Call( "2", McpTestRequest( "2", "tools/call", "\"name\":\"echo\",\"arguments\":{}" ), emptyArguments ) );
            ASSERT_TRUE( 0 == McpTestErrorCode( withoutArguments ) && McpTestResultAttr( withoutArguments, "isError" ).Lowercase() == "true" );
            ASSERT_TRUE( McpTestFirstText( withoutArguments.FindChild( "result" ) ) == McpTestFirstText( emptyArguments.FindChild( "result" ) ) );
            ASSERT_TRUE( 0 == echo->executions.Load() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
