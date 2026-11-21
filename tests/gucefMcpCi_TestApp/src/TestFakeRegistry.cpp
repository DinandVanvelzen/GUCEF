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

#include "TestMcpCiSupport.h"
#include "TestFakeRegistry.h"

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

/** Passes the requests of the HTTP server to a CFakeRegistry */
class CFakeRegistryHandler : public WEB::CIHttpServerRequestHandler
{
    public:

    CFakeRegistryHandler( void ) : WEB::CIHttpServerRequestHandler(), m_server( GUCEF_NULL ) {}
    virtual ~CFakeRegistryHandler() {}

    void SetServer( CFakeRegistry* server ) { m_server = server; }

    virtual bool OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL == m_server )
        {
            response.statusCode = 503;
            return true;
        }
        m_server->Handle( request, response );
        return true;
    }

    virtual bool OnRead( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnReadMetaData( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnUpdate( const WEB::CHttpRequestData&, bool, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnCreate( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnDelete( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool SetRouterController( WEB::CIHTTPServerRouterController* ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual WEB::CIHTTPServerRouterController* GetRouterController( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    private:

    CFakeRegistry* m_server;
};

/*-------------------------------------------------------------------------*/

class CFakeRegistryHandlerFactory : public CORE::CTFactory< WEB::CIHttpServerRequestHandler, CFakeRegistryHandler, MT::CMutex >
{
    public:

    typedef CORE::CTFactory< WEB::CIHttpServerRequestHandler, CFakeRegistryHandler, MT::CMutex > TBase;

    explicit CFakeRegistryHandlerFactory( CFakeRegistry* server ) : TBase(), m_server( server ) {}
    virtual ~CFakeRegistryHandlerFactory() {}

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        TProductPtr product = TBase::Create();
        CFakeRegistryHandler* handler = static_cast< CFakeRegistryHandler* >( product.GetPointerAlways() );
        if ( GUCEF_NULL != handler )
            handler->SetServer( m_server );
        return product;
    }

    private:

    CFakeRegistry* m_server;
};

/*-------------------------------------------------------------------------*/

static CORE::CString
QueryValue( const CORE::CString& query, const char* name )
{
    CORE::CString::StringVector items = query.ParseElements( '&', false );
    for ( size_t i=0; i<items.size(); ++i )
    {
        const Int32 equals = items[ i ].HasChar( '=', true );
        if ( equals > 0 && items[ i ].SubstrToIndex( (UInt32) equals, true ) == name )
            return PercentDecode( items[ i ].CutChars( (UInt32) equals + 1, true ) );
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

static void
Respond( WEB::CHttpResponseData& response, int status, const CORE::CString& json )
{
    response.statusCode = status;
    response.content.Clear( true );
    if ( !json.IsNULLOrEmpty() )
    {
        response.contentType = "application/json";
        response.content.CopyFrom( json.ByteSize() - 1, json.C_String() );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFakeRegistry::CFakeRegistry( EMode mode, const CORE::CString& user, const CORE::CString& token )
    : MT::CActiveObject()
    , m_mode( mode )
    , m_user( user )
    , m_token( token )
    , m_issuedToken( "issued-bearer-4c7e1a9d" )
    , m_manifests()
    , m_sendDigestHeader( true )
    , m_tokenRequests( 0 )
    , m_unauthorized( 0 )
    , m_requestLog()
    , m_lock()
    , m_pulseGenerator( CORE::CPulseGenerator::CreateSharedObj() )
    , m_server( GUCEF_NULL )
    , m_factory( GUCEF_NULL )
    , m_running( false )
{
}

/*-------------------------------------------------------------------------*/

CFakeRegistry::~CFakeRegistry()
{
    Stop();
}

/*-------------------------------------------------------------------------*/

bool
CFakeRegistry::Start( void )
{
    CFakeRegistryHandlerFactory* factory = GUCEF_NEW CFakeRegistryHandlerFactory( this );
    m_factory = factory;
    WEB::CHttpServerSettings settings;
    settings.bindAddress = "127.0.0.1";
    settings.keepAliveConnections = false;
    settings.processRequestsAsync = false;
    settings.allowWebSocketUpgrades = false;
    m_server = GUCEF_NEW WEB::CHTTPServer( m_pulseGenerator, factory, &settings );
    if ( !m_server->ListenOnPort( 0 ) )
        return false;
    m_running = true;
    return Activate( GUCEF_NULL, 1, 1 );
}

/*-------------------------------------------------------------------------*/

void
CFakeRegistry::Stop( void )
{
    if ( !m_running )
        return;
    m_running = false;
    Deactivate( false, true );
    m_server->Close();
    GUCEF_DELETE m_server;
    m_server = GUCEF_NULL;
    GUCEF_DELETE static_cast< CFakeRegistryHandlerFactory* >( m_factory );
    m_factory = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeRegistry::GetHost( void ) const
{
    return "127.0.0.1:" + CORE::UInt16ToString( GUCEF_NULL != m_server ? m_server->GetBoundPort() : 0 );
}

/*-------------------------------------------------------------------------*/

void
CFakeRegistry::AddManifest( const CORE::CString& repository, const CORE::CString& tag, const CORE::CString& digest )
{
    MT::CScopeMutex lock( m_lock );
    m_manifests[ repository + ":" + tag ] = digest;
}

/*-------------------------------------------------------------------------*/

void
CFakeRegistry::SetSendDigestHeader( bool send )
{
    MT::CScopeMutex lock( m_lock );
    m_sendDigestHeader = send;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeRegistry::ManifestBody( const CORE::CString& digest )
{
    return "{\"schemaVersion\":2,\"mediaType\":\"application/vnd.oci.image.manifest.v1+json\",\"annotations\":{\"for\":" + Json( digest ) + "}}";
}

/*-------------------------------------------------------------------------*/

UInt32 CFakeRegistry::GetTokenRequestCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_tokenRequests; }
UInt32 CFakeRegistry::GetUnauthorizedCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_unauthorized; }
CORE::CString CFakeRegistry::GetRequestLog( void ) const { MT::CScopeMutex lock( m_lock ); return m_requestLog; }

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CFakeRegistry::AsLockable( void ) const
{
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CFakeRegistry::OnThreadStart( void* taskdata )
{
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeRegistry::OnThreadCycle( void* taskdata )
{
    m_pulseGenerator->ForceDirectPulse();
    return false;
}

/*-------------------------------------------------------------------------*/

void
CFakeRegistry::Handle( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response )
{
    const CORE::CString method = request.requestType.Uppercase();
    const CORE::CString path = request.requestUri;
    const CORE::CString query = request.requestUriParams;
    WEB::CHttpRequestData::TStringMap::const_iterator header = request.otherHeaders.find( "authorization" );
    const CORE::CString authorization = header == request.otherHeaders.end() ? CORE::CString::Empty : (*header).second;
    const CORE::CString basic = "Basic " + CORE::Base64Encode( ( m_user + ":" + m_token ).C_String(), ( m_user + ":" + m_token ).ByteSize() - 1 );

    MT::CScopeMutex lock( m_lock );
    const Int32 space = authorization.HasChar( ' ', true );
    m_requestLog += method + " " + path + ( query.IsNULLOrEmpty() ? CORE::CString::Empty : "?" + query ) + " auth=" +
                    ( space > 0 ? authorization.SubstrToIndex( (UInt32) space, true ) : CORE::CString( "none" ) ) + "\n";

    // the token handshake
    if ( method == "GET" && path == "/v2/token" )
    {
        ++m_tokenRequests;
        const CORE::CString scope = QueryValue( query, "scope" );
        if ( MODE_BEARER != m_mode || authorization != basic || !scope.StartsWith( "repository:" ) || !scope.EndsWith( ":pull" ) ||
             QueryValue( query, "service" ) != "fake-registry" )
        {
            ++m_unauthorized;
            Respond( response, 401, "{\"errors\":[{\"code\":\"UNAUTHORIZED\"}]}" );
            return;
        }
        Respond( response, 200, "{\"token\":" + Json( m_issuedToken ) + "}" );
        return;
    }

    // /v2/<owner>/<name>/manifests/<tag>
    const Int32 marker = path.HasSubstr( "/manifests/", true );
    if ( ( method != "GET" && method != "HEAD" ) || !path.StartsWith( "/v2/" ) || marker < 4 )
    {
        Respond( response, 404, "{\"errors\":[{\"code\":\"NAME_UNKNOWN\"}]}" );
        return;
    }
    const bool authorized = MODE_BEARER == m_mode ? authorization == "Bearer " + m_issuedToken : authorization == basic;
    if ( !authorized )
    {
        ++m_unauthorized;
        Respond( response, 401, method == "HEAD" ? CORE::CString::Empty : CORE::CString( "{\"errors\":[{\"code\":\"UNAUTHORIZED\"}]}" ) );
        if ( MODE_BEARER == m_mode )
            response.otherHeaders[ "WWW-Authenticate" ] = "Bearer realm=\"http://" + GetHost() + "/v2/token\",service=\"fake-registry\",scope=\"repository:x:pull\"";
        else
            response.otherHeaders[ "WWW-Authenticate" ] = "Basic realm=\"fake\"";
        return;
    }
    const CORE::CString repository = path.SubstrToIndex( (UInt32) marker, true ).CutChars( 4, true );
    const CORE::CString tag = path.CutChars( (UInt32) marker + 11, true );
    TStringMap::const_iterator manifest = m_manifests.find( repository + ":" + tag );
    if ( manifest == m_manifests.end() )
    {
        Respond( response, 404, method == "HEAD" ? CORE::CString::Empty : CORE::CString( "{\"errors\":[{\"code\":\"MANIFEST_UNKNOWN\"}]}" ) );
        return;
    }
    Respond( response, 200, method == "HEAD" ? CORE::CString::Empty : ManifestBody( (*manifest).second ) );
    response.contentType = "application/vnd.oci.image.manifest.v1+json";
    if ( m_sendDigestHeader )
        response.otherHeaders[ "Docker-Content-Digest" ] = (*manifest).second;
}

/*-------------------------------------------------------------------------*/

}; /* namespace MCPCITEST */

/*-------------------------------------------------------------------------*/
