/*
 *  gucefMcpCi_TestApp: tests of the gucefMcpCi MCP server
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "TestMcpCiSupport.h"
#include "TestCiServer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace MCPCITEST {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTestCiServer::CTestCiServer( void )
    : m_client()
    , m_nextId( 1 )
    , m_started( false )
    , m_transcript()
{
}

/*-------------------------------------------------------------------------*/

bool
CTestCiServer::Start( const CORE::CProcess::TStringVector& args )
{
    const CORE::CString exe = FindServer();
    // the child gets its standard streams and nothing else of this process: not the sockets of the fake servers
    m_client.Process().SetIsolateFromParent( true );
    m_started = !exe.IsNULLOrEmpty() && m_client.Launch( exe, args, ChildEnvironment() );
    return m_started;
}

/*-------------------------------------------------------------------------*/

bool
CTestCiServer::Request( const CORE::CString& method, const CORE::CString& params, CORE::CDataNode& response, UInt32 timeoutInMs )
{
    const CORE::CString id = CORE::UInt32ToString( m_nextId++ );
    if ( !m_client.SendLine( McpTestRequest( id, method, params ) ) )
        return false;
    GUCEF::vector< CORE::CDataNode > others;
    if ( !m_client.ReadResponse( id, response, others, timeoutInMs ) )
        return false;
    m_transcript += McpTestToJson( response ) + "\n";
    for ( size_t i=0; i<others.size(); ++i )
        m_transcript += McpTestToJson( others[ i ] ) + "\n";
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestCiServer::Call( const CORE::CString& tool, const CORE::CString& argumentsJson, CORE::CString& text, bool& isError, UInt32 timeoutInMs )
{
    CORE::CDataNode response;
    if ( !Request( "tools/call", "\"name\":" + Json( tool ) + ",\"arguments\":" + argumentsJson, response, timeoutInMs ) )
        return false;
    const CORE::CDataNode* error = response.FindChild( "error" );
    text = GUCEF_NULL != error ? McpTestErrorMessage( response ) : McpTestFirstText( response.FindChild( "result" ) );
    isError = McpTestResultAttr( response, "isError" ) == "true" || GUCEF_NULL != error;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestCiServer::ToolNames( void )
{
    CORE::CDataNode response;
    if ( !Request( "tools/list", CORE::CString::Empty, response, 30000 ) )
        return CORE::CString::Empty;
    CORE::CString names;
    const CORE::CDataNode* tools = McpTestResultChild( response, "tools" );
    if ( GUCEF_NULL != tools )
        for ( CORE::CDataNode::const_iterator i = tools->ConstBegin(); i != tools->ConstEnd(); ++i )
            names += McpTestAttr( *(*i), "name" ) + ",";
    return names;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestCiServer::Instructions( void )
{
    CORE::CDataNode response;
    if ( !Request( "server/discover", CORE::CString::Empty, response, 30000 ) )
        return CORE::CString::Empty;
    return McpTestResultAttr( response, "instructions" );
}

/*-------------------------------------------------------------------------*/

bool
CTestCiServer::Stop( void )
{
    if ( !m_started )
        return true;
    m_client.CloseInput();
    Int32 exitCode = -1;
    const bool ended = m_client.WaitForExit( 30000, exitCode );
    m_transcript += m_client.GetStderrText();
    m_started = false;
    return ended && 0 == exitCode;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestCiServer::GetTranscript( void ) const
{
    return m_transcript + m_client.GetStderrText();
}

/*-------------------------------------------------------------------------*/

}; /* namespace MCPCITEST */

/*-------------------------------------------------------------------------*/
