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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#include "gucefWEB_CIHttpServerRequestHandler.h"
#define GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#endif /* GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#include "TestFakeGatewayFront.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace MCPCITEST {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Passes the requests of the HTTP server to a CFakeGatewayFront */
class CFakeGatewayFrontHandler : public WEB::CIHttpServerRequestHandler
{
    public:

    CFakeGatewayFrontHandler( void ) : WEB::CIHttpServerRequestHandler(), m_front( GUCEF_NULL ) {}
    virtual ~CFakeGatewayFrontHandler() {}

    void SetFront( CFakeGatewayFront* front ) { m_front = front; }

    virtual bool OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL == m_front )
        {
            response.statusCode = 503;
            return true;
        }
        m_front->Handle( request, response );
        return true;
    }

    virtual bool OnRead( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnReadMetaData( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnUpdate( const WEB::CHttpRequestData&, bool, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnCreate( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnDelete( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool SetRouterController( WEB::CIHTTPServerRouterController* ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual WEB::CIHTTPServerRouterController* GetRouterController( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    private:

    CFakeGatewayFront* m_front;
};

/*-------------------------------------------------------------------------*/

class CFakeGatewayFrontHandlerFactory : public CORE::CTFactory< WEB::CIHttpServerRequestHandler, CFakeGatewayFrontHandler, MT::CMutex >
{
    public:

    typedef CORE::CTFactory< WEB::CIHttpServerRequestHandler, CFakeGatewayFrontHandler, MT::CMutex > TBase;

    explicit CFakeGatewayFrontHandlerFactory( CFakeGatewayFront* front ) : TBase(), m_front( front ) {}
    virtual ~CFakeGatewayFrontHandlerFactory() {}

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        TProductPtr product = TBase::Create();
        CFakeGatewayFrontHandler* handler = static_cast< CFakeGatewayFrontHandler* >( product.GetPointerAlways() );
        if ( GUCEF_NULL != handler )
            handler->SetFront( m_front );
        return product;
    }

    private:

    CFakeGatewayFront* m_front;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFakeGatewayFront::CFakeGatewayFront( const CORE::CString& targetUrl )
    : MT::CActiveObject()
    , m_targetUrl( targetUrl )
    , m_lock()
    , m_pulseGenerator( CORE::CPulseGenerator::CreateSharedObj() )
    , m_server( GUCEF_NULL )
    , m_factory( GUCEF_NULL )
    , m_running( false )
    , m_mode( MODE_FORWARD )
    , m_failures()
    , m_chunkCount( 0 )
    , m_failedCount( 0 )
{
}

/*-------------------------------------------------------------------------*/

CFakeGatewayFront::~CFakeGatewayFront()
{
    Stop();
}

/*-------------------------------------------------------------------------*/

bool
CFakeGatewayFront::Start( void )
{
    CFakeGatewayFrontHandlerFactory* factory = GUCEF_NEW CFakeGatewayFrontHandlerFactory( this );
    m_factory = factory;
    WEB::CHttpServerSettings settings;
    settings.bindAddress = "127.0.0.1";
    settings.keepAliveConnections = false;
    settings.processRequestsAsync = false;
    settings.allowWebSocketUpgrades = false;
    settings.maxBodyBytes = 9 * 1024 * 1024;
    settings.bodyTimeoutMs = 120000;
    m_server = GUCEF_NEW WEB::CHTTPServer( m_pulseGenerator, factory, &settings );
    if ( !m_server->ListenOnPort( 0 ) )
        return false;
    m_running = true;
    return Activate( GUCEF_NULL, 1, 1 );
}

/*-------------------------------------------------------------------------*/

void
CFakeGatewayFront::Stop( void )
{
    if ( !m_running )
        return;
    m_running = false;
    Deactivate( false, true );
    m_server->Close();
    GUCEF_DELETE m_server;
    m_server = GUCEF_NULL;
    GUCEF_DELETE static_cast< CFakeGatewayFrontHandlerFactory* >( m_factory );
    m_factory = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeGatewayFront::GetUrl( void ) const
{
    return "http://127.0.0.1:" + CORE::UInt16ToString( GUCEF_NULL != m_server ? m_server->GetBoundPort() : 0 );
}

/*-------------------------------------------------------------------------*/

void
CFakeGatewayFront::SetMode( EMode mode )
{
    MT::CScopeMutex lock( m_lock );
    m_mode = mode;
}

/*-------------------------------------------------------------------------*/

void
CFakeGatewayFront::FailChunk( UInt32 number, bool passOn )
{
    MT::CScopeMutex lock( m_lock );
    m_failures[ number ] = passOn;
}

/*-------------------------------------------------------------------------*/

UInt32 CFakeGatewayFront::GetChunkCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_chunkCount; }
UInt32 CFakeGatewayFront::GetFailedChunkCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_failedCount; }

/*-------------------------------------------------------------------------*/

void
CFakeGatewayFront::PassOn( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response )
{
    WEB::CHttpRequestData forward;
    const CORE::CString url = m_targetUrl + request.requestUri + ( request.requestUriParams.IsNULLOrEmpty() ? CORE::CString::Empty : "?" + request.requestUriParams );
    if ( !forward.SetUrl( url ) )
    {
        response.statusCode = 500;
        return;
    }
    forward.requestType = request.requestType;
    WEB::CHttpRequestData::TStringMap::const_iterator authorization = request.otherHeaders.find( "authorization" );
    if ( authorization != request.otherHeaders.end() )
        forward.otherHeaders[ "Authorization" ] = (*authorization).second;
    forward.contentRepresentation = request.contentRepresentation;
    forward.content = request.content;
    WEB::CHttpClientRequestOptions options;
    options.totalTimeoutInMs = 120000;
    options.connectTimeoutInMs = 5000;
    options.useEnvironmentProxy = false;
    WEB::CHTTPClient client;
    WEB::CHttpResponseData answer;
    if ( !client.Execute( forward, options, answer ) )
    {
        response.statusCode = 502;
        return;
    }
    response.statusCode = answer.statusCode;
    response.contentType = answer.contentType;
    response.content = answer.content;
}

/*-------------------------------------------------------------------------*/

void
CFakeGatewayFront::Handle( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response )
{
    EMode mode = MODE_FORWARD;
    bool fail = false;
    bool passOn = true;
    {
        MT::CScopeMutex lock( m_lock );
        mode = m_mode;
        if ( request.requestType == "PUT" )
        {
            ++m_chunkCount;
            TFailureMap::iterator i = m_failures.find( m_chunkCount );
            if ( i != m_failures.end() )
            {
                fail = true;
                passOn = (*i).second;
                ++m_failedCount;
            }
        }
    }
    if ( MODE_GARBAGE == mode )
    {
        static const char garbage[] = "<html><body>Welcome to the captive portal</body></html>";
        response.statusCode = 200;
        response.contentType = "text/html";
        response.content.CopyFrom( sizeof( garbage ) - 1, garbage );
        return;
    }
    if ( !fail || passOn )
        PassOn( request, response );
    if ( fail )
    {
        static const char lost[] = "bad gateway";
        response = WEB::CHttpResponseData();
        response.statusCode = 502;
        response.contentType = "text/plain";
        response.content.CopyFrom( sizeof( lost ) - 1, lost );
    }
}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CFakeGatewayFront::AsLockable( void ) const
{
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CFakeGatewayFront::OnThreadStart( void* taskdata )
{
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeGatewayFront::OnThreadCycle( void* taskdata )
{
    m_pulseGenerator->ForceDirectPulse();
    return false;
}

/*-------------------------------------------------------------------------*/

}; /* namespace MCPCITEST */

/*-------------------------------------------------------------------------*/
