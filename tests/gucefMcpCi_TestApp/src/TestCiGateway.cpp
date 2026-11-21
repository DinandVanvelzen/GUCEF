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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_TEST_TLS_SERVER_H
#include "gucef_test_tls_server.h"
#define GUCEF_TEST_TLS_SERVER_H
#endif /* GUCEF_TEST_TLS_SERVER_H ? */

#include "TestMcpSupport.h"
#include "TestMcpCiSupport.h"
#include "TestCiGateway.h"

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

CTestCiGateway::CTestCiGateway( void )
    : m_process()
    , m_port( 0 )
    , m_tls( false )
    , m_started( false )
    , m_logFile()
{
}

/*-------------------------------------------------------------------------*/

CTestCiGateway::~CTestCiGateway()
{
    Stop();
}

/*-------------------------------------------------------------------------*/

bool
CTestCiGateway::Start( const CORE::CProcess::TStringVector& args, const CORE::CString& logFile, bool tls )
{
    m_tls = tls;
    m_logFile = logFile;
    m_port = TESTFW::CTlsTestServer::FindFreePort();
    CORE::CProcess::TStringVector all = args;
    all.push_back( tls ? "--listen-tls" : "--listen" );
    all.push_back( "127.0.0.1:" + CORE::UInt16ToString( m_port ) );
    all.push_back( "--log-file" );
    all.push_back( logFile );

    // the log goes to the file only: the console logger of the child stays quiet
    CORE::CProcess::TStringMap environment = ChildEnvironment();
    environment[ "GUCEF_CONSOLE_LOGGER" ] = "none";
    m_process.SetIsolateFromParent( true );
    if ( !m_process.Launch( FindServer(), all, CORE::CString::Empty, environment ) )
        return false;
    m_started = true;
    for ( int i=0; i<750; ++i )
    {
        if ( TESTFW::CTlsTestServer::CanConnect( m_port ) )
            return true;
        if ( !m_process.IsRunning() )
            break;
        MT::ThreadDelay( 20 );
    }
    Stop();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTestCiGateway::Stop( void )
{
    if ( !m_started )
        return true;
    m_started = false;
    if ( !m_process.IsRunning() )
        return true;
    return m_process.Terminate( true, 30000 ) && !m_process.IsRunning();
}

/*-------------------------------------------------------------------------*/

UInt16
CTestCiGateway::GetPort( void ) const
{
    return m_port;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestCiGateway::GetUrl( void ) const
{
    return ( m_tls ? "https://localhost:" : "http://127.0.0.1:" ) + CORE::UInt16ToString( m_port );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestCiGateway::GetLog( void ) const
{
    CORE::CString text;
    ReadFile( m_logFile, text );
    return text;
}

/*-------------------------------------------------------------------------*/

static bool
MakeRequest( const CORE::CString& baseUrl, const CORE::CString& token, const CORE::CString& method, const CORE::CString& pathAndQuery,
             const CORE::CDynamicBuffer& body, WEB::CHttpRequestData& request )
{
    if ( !request.SetUrl( baseUrl + pathAndQuery ) )
        return false;
    request.requestType = method;
    if ( !token.IsNULLOrEmpty() )
        request.otherHeaders[ "Authorization" ] = "Bearer " + token;
    if ( body.GetDataSize() > 0 )
    {
        request.contentRepresentation = method == "PUT" ? "application/octet-stream" : "application/json";
        request.content = body;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
GatewayRequest( const CORE::CString& baseUrl        ,
                const CORE::CString& token          ,
                const CORE::CString& method         ,
                const CORE::CString& pathAndQuery   ,
                const CORE::CDynamicBuffer& body    ,
                CORE::CString& answer               ,
                const CORE::CString& caBundle       ,
                UInt32 timeoutInMs                  )
{
    answer.Clear();
    WEB::CHttpRequestData request;
    if ( !MakeRequest( baseUrl, token, method, pathAndQuery, body, request ) )
        return 0;
    WEB::CHttpClientRequestOptions options;
    options.caBundlePath = caBundle;
    options.totalTimeoutInMs = timeoutInMs;
    options.connectTimeoutInMs = 5000;
    options.maxResponseBodySize = 64 * 1024 * 1024;
    options.useEnvironmentProxy = false;
    WEB::CHTTPClient client;
    WEB::CHttpResponseData response;
    if ( !client.Execute( request, options, response ) )
        return 0;
    answer = CORE::CString( static_cast< const char* >( response.content.GetConstBufferPtr() ), response.content.GetDataSize() );
    return response.statusCode;
}

/*-------------------------------------------------------------------------*/

Int32
GatewayRequest( const CORE::CString& baseUrl        ,
                const CORE::CString& token          ,
                const CORE::CString& method         ,
                const CORE::CString& pathAndQuery   ,
                const CORE::CString& body           ,
                CORE::CString& answer               )
{
    CORE::CDynamicBuffer buffer;
    if ( !body.IsNULLOrEmpty() )
        buffer.CopyFrom( body.ByteSize() - 1, body.C_String() );
    return GatewayRequest( baseUrl, token, method, pathAndQuery, buffer, answer );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
GatewayRequestAsync( WEB::CHTTPClient& client, const CORE::CString& baseUrl, const CORE::CString& token, const CORE::CString& pathAndQuery, UInt32 timeoutInMs )
{
    WEB::CHttpRequestData request;
    MakeRequest( baseUrl, token, "GET", pathAndQuery, CORE::CDynamicBuffer(), request );
    WEB::CHttpClientRequestOptions options;
    options.totalTimeoutInMs = timeoutInMs;
    options.connectTimeoutInMs = 5000;
    options.maxResponseBodySize = 4 * 1024 * 1024;
    options.useEnvironmentProxy = false;
    return client.ExecuteAsync( request, options );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
GatewayRequestAsync( WEB::CHTTPClient& client, const CORE::CString& baseUrl, const CORE::CString& token, const CORE::CString& method,
                     const CORE::CString& pathAndQuery, const CORE::CString& body, UInt32 timeoutInMs )
{
    CORE::CDynamicBuffer bytes;
    if ( !body.IsNULLOrEmpty() )
        bytes.CopyFrom( body.ByteSize() - 1, body.C_String() );
    WEB::CHttpRequestData request;
    MakeRequest( baseUrl, token, method, pathAndQuery, bytes, request );
    WEB::CHttpClientRequestOptions options;
    options.totalTimeoutInMs = timeoutInMs;
    options.connectTimeoutInMs = 5000;
    options.maxResponseBodySize = 4 * 1024 * 1024;
    options.useEnvironmentProxy = false;
    return client.ExecuteAsync( request, options );
}

/*-------------------------------------------------------------------------*/

Int32
GatewayResult( const CORE::CFutureResult& future, UInt32 timeoutInMs )
{
    CORE::CTaskPtr task = future.GetResult( (Int32) timeoutInMs );
    if ( task.IsNULL() )
        return 0;
    WEB::CHttpClientRequestTaskData* data = WEB::CHTTPClient::GetRequestTaskData( task );
    if ( GUCEF_NULL == data || !data->transferInfo.success )
        return 0;
    return data->response.statusCode;
}

/*-------------------------------------------------------------------------*/

CORE::CString
JsonValue( const CORE::CString& json, const char* name )
{
    CORE::CDataNode tree;
    if ( !McpTestParse( json, tree ) )
        return CORE::CString::Empty;
    return tree.GetAttributeValue( name ).AsString();
}

/*-------------------------------------------------------------------------*/

CORE::CString
Sha256Hex( const void* data, UInt32 size )
{
    UInt8 digest[ 32 ];
    CORE::CSha256::Hash( data, size, digest );
    static const char* digits = "0123456789abcdef";
    CORE::CString text;
    for ( UInt32 i=0; i<32; ++i )
    {
        text += digits[ digest[ i ] >> 4 ];
        text += digits[ digest[ i ] & 0x0F ];
    }
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
TokenLine( const CORE::CString& token, const CORE::CString& machine )
{
    return Sha256Hex( token.C_String(), token.ByteSize() - 1 ) + "\t" + machine + "\n";
}

/*-------------------------------------------------------------------------*/

}; /* namespace MCPCITEST */

/*-------------------------------------------------------------------------*/
