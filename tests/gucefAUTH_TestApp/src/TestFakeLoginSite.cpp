/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
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
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_CURLENCODER_H
#include "gucefCORE_CUrlEncoder.h"
#define GUCEF_CORE_CURLENCODER_H
#endif /* GUCEF_CORE_CURLENCODER_H ? */
#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */
#ifndef GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#include "gucefWEB_CIHttpServerRequestHandler.h"
#define GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#endif /* GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H ? */

#include "TestFakeLoginSite.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace AUTHTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CFakeLoginSiteHandler : public WEB::CIHttpServerRequestHandler
{
    public:

    CFakeLoginSiteHandler( void ) : WEB::CIHttpServerRequestHandler(), m_site( GUCEF_NULL ) {}
    virtual ~CFakeLoginSiteHandler() {}

    void SetSite( CFakeLoginSite* site ) { m_site = site; }

    virtual bool OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL == m_site )
        {
            response.statusCode = 503;
            return true;
        }
        m_site->HandleRequest( request, response );
        return true;
    }

    virtual bool OnRead( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnReadMetaData( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnUpdate( const WEB::CHttpRequestData&, bool, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnCreate( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnDelete( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool SetRouterController( WEB::CIHTTPServerRouterController* ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual WEB::CIHTTPServerRouterController* GetRouterController( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    private:

    CFakeLoginSite* m_site;
};

/*-------------------------------------------------------------------------*/

class CFakeLoginSiteHandlerFactory : public CORE::CTFactory< WEB::CIHttpServerRequestHandler, CFakeLoginSiteHandler, MT::CMutex >
{
    public:

    typedef CORE::CTFactory< WEB::CIHttpServerRequestHandler, CFakeLoginSiteHandler, MT::CMutex > TBase;

    explicit CFakeLoginSiteHandlerFactory( CFakeLoginSite* site ) : TBase(), m_site( site ) {}
    virtual ~CFakeLoginSiteHandlerFactory() {}

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        TProductPtr product = TBase::Create();
        CFakeLoginSiteHandler* handler = static_cast< CFakeLoginSiteHandler* >( product.GetPointerAlways() );
        if ( GUCEF_NULL != handler )
            handler->SetSite( m_site );
        return product;
    }

    private:

    CFakeLoginSiteHandlerFactory( const CFakeLoginSiteHandlerFactory& src );
    CFakeLoginSiteHandlerFactory& operator=( const CFakeLoginSiteHandlerFactory& src );

    CFakeLoginSite* m_site;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CString
HeaderOf( const WEB::CHttpRequestData& request, const char* name )
{
    WEB::CHttpRequestData::TStringMap::const_iterator i = request.otherHeaders.begin();
    while ( i != request.otherHeaders.end() )
    {
        if ( (*i).first.Equals( name, false ) )
            return (*i).second;
        ++i;
    }
    return CORE::CString::Empty;
}

CORE::CString
CookieOf( const WEB::CHttpRequestData& request, const char* name )
{
    // the server of gucefWEB hands the Cookie header over as the transaction id
    const CORE::CString header = request.transactionID.IsNULLOrEmpty() ? HeaderOf( request, "Cookie" ) : request.transactionID;
    const CORE::CString::StringVector pairs = header.ParseElements( ';', false );
    for ( size_t i=0; i<pairs.size(); ++i )
    {
        const CORE::CString pair = pairs[ i ].Trim( true ).Trim( false );
        if ( pair.SubstrToChar( '=', true ) == name )
            return pair.SubstrToChar( '=', false, true );
    }
    return CORE::CString::Empty;
}

CORE::CString
FormValueOf( const WEB::CHttpRequestData& request, const char* name )
{
    CORE::CString body;
    if ( request.content.GetDataSize() > 0 )
        body.Set( request.content.AsConstTypePtr< char >(), request.content.GetDataSize() );
    const CORE::CString::StringVector pairs = body.ParseElements( '&', false );
    for ( size_t i=0; i<pairs.size(); ++i )
    {
        if ( pairs[ i ].SubstrToChar( '=', true ) != name )
            continue;
        CORE::CUtf8String decoded;
        CORE::CUrlEncoder::Decode( CORE::CUtf8String( pairs[ i ].SubstrToChar( '=', false, true ).ReplaceChar( '+', ' ' ) ), decoded );
        return decoded;
    }
    return CORE::CString::Empty;
}

bool
Contains( const CORE::CString::StringVector& values, const CORE::CString& value )
{
    for ( size_t i=0; i<values.size(); ++i )
        if ( values[ i ] == value )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFakeLoginSiteStats::CFakeLoginSiteStats( void )
    : loginPages( 0 )
    , loginPosts( 0 )
    , wrongPasswords( 0 )
    , goodPasswords( 0 )
    , challengePages( 0 )
    , codePages( 0 )
    , sessionsIssued( 0 )
    , apiAnswered( 0 )
    , apiRefused( 0 )
{
}

/*-------------------------------------------------------------------------*/

CFakeLoginSite::CFakeLoginSite( void )
    : MT::CActiveObject()
    , m_lock()
    , m_pulseGenerator( CORE::CPulseGenerator::CreateSharedObj() )
    , m_server( GUCEF_NULL )
    , m_factory( GUCEF_NULL )
    , m_port( 0 )
    , m_running( false )
    , m_email()
    , m_password()
    , m_challenge( false )
    , m_challengeApproved( false )
    , m_emailCode()
    , m_validSessions()
    , m_issuedSessions()
    , m_pending()
    , m_counter( 0 )
    , m_stats()
{
}

/*-------------------------------------------------------------------------*/

CFakeLoginSite::~CFakeLoginSite()
{
    Stop();
}

/*-------------------------------------------------------------------------*/

bool
CFakeLoginSite::Start( void )
{
    if ( m_running )
        return true;
    CFakeLoginSiteHandlerFactory* factory = GUCEF_NEW CFakeLoginSiteHandlerFactory( this );
    m_factory = factory;
    WEB::CHttpServerSettings settings;
    settings.bindAddress = "127.0.0.1";
    settings.keepAliveConnections = false;
    settings.processRequestsAsync = false;
    settings.allowWebSocketUpgrades = false;
    settings.maxClientConnections = 64;
    m_server = GUCEF_NEW WEB::CHTTPServer( m_pulseGenerator, factory, &settings );
    if ( !m_server->ListenOnPort( 0 ) )
        return false;
    m_port = m_server->GetBoundPort();
    m_running = true;
    return Activate( GUCEF_NULL, 1, 1 );
}

/*-------------------------------------------------------------------------*/

void
CFakeLoginSite::Stop( void )
{
    if ( !m_running )
        return;
    m_running = false;
    Deactivate( false, true );
    if ( GUCEF_NULL != m_server )
    {
        m_server->Close();
        GUCEF_DELETE m_server;
        m_server = GUCEF_NULL;
    }
    if ( GUCEF_NULL != m_factory )
    {
        GUCEF_DELETE static_cast< CFakeLoginSiteHandlerFactory* >( m_factory );
        m_factory = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CFakeLoginSite::AsLockable( void ) const
{
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CFakeLoginSite::OnThreadStart( void* taskdata )
{
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeLoginSite::OnThreadCycle( void* taskdata )
{
    m_pulseGenerator->ForceDirectPulse();
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeLoginSite::GetBaseUrl( void ) const
{
    return "http://127.0.0.1:" + CORE::ToString( m_port );
}

/*-------------------------------------------------------------------------*/

UInt16
CFakeLoginSite::GetPort( void ) const
{
    return m_port;
}

/*-------------------------------------------------------------------------*/

void
CFakeLoginSite::SetAccount( const CORE::CString& email, const CORE::CString& password )
{
    MT::CScopeMutex lock( m_lock );
    m_email = email;
    m_password = password;
}

/*-------------------------------------------------------------------------*/

void
CFakeLoginSite::SetChallenge( bool challenge )
{
    MT::CScopeMutex lock( m_lock );
    m_challenge = challenge;
    m_challengeApproved = false;
}

/*-------------------------------------------------------------------------*/

void
CFakeLoginSite::SetEmailCode( const CORE::CString& code )
{
    MT::CScopeMutex lock( m_lock );
    m_emailCode = code;
}

/*-------------------------------------------------------------------------*/

void
CFakeLoginSite::ApproveChallenge( void )
{
    MT::CScopeMutex lock( m_lock );
    m_challengeApproved = true;
}

/*-------------------------------------------------------------------------*/

void
CFakeLoginSite::ExpireSessions( void )
{
    MT::CScopeMutex lock( m_lock );
    m_validSessions.clear();
}

/*-------------------------------------------------------------------------*/

CFakeLoginSiteStats
CFakeLoginSite::GetStats( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_stats;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CFakeLoginSite::GetIssuedSessionValues( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_issuedSessions;
}

/*-------------------------------------------------------------------------*/

void
CFakeLoginSite::SendHtml( WEB::CHttpResponseData& response, int status, const CORE::CString& title, const CORE::CString& body, bool refresh )
{
    const CORE::CString page = "<!DOCTYPE html><html><head><meta charset=\"utf-8\">" +
                               CORE::CString( refresh ? "<meta http-equiv=\"refresh\" content=\"1\">" : "" ) +
                               "<title>" + title + "</title></head><body>" + body + "</body></html>";
    response.statusCode = status;
    response.contentType = "text/html; charset=utf-8";
    response.content.Clear();
    response.content.CopyFrom( page, 0, false );
}

/*-------------------------------------------------------------------------*/

void
CFakeLoginSite::Redirect( WEB::CHttpResponseData& response, const CORE::CString& path )
{
    response.statusCode = 302;
    response.otherHeaders[ "Location" ] = path.StartsWith( "http" ) ? path : GetBaseUrl() + path;
}

/*-------------------------------------------------------------------------*/

void
CFakeLoginSite::IssueSession( WEB::CHttpResponseData& response )
{
    ++m_counter;
    ++m_stats.sessionsIssued;
    const CORE::CString value = "sidValue" + CORE::ToString( m_counter ) + "Qz7Wv";
    m_validSessions.push_back( value );
    m_issuedSessions.push_back( value );
    m_challengeApproved = false;
    response.otherHeaders[ "Set-Cookie" ] = "sid=" + value + "; Path=/; HttpOnly; SameSite=Lax";
    Redirect( response, "/account" );
}

/*-------------------------------------------------------------------------*/

bool
CFakeLoginSite::HasValidSession( const WEB::CHttpRequestData& request ) const
{
    const CORE::CString sid = CookieOf( request, "sid" );
    return !sid.IsNULLOrEmpty() && Contains( m_validSessions, sid );
}

/*-------------------------------------------------------------------------*/

bool
CFakeLoginSite::HasPending( const WEB::CHttpRequestData& request ) const
{
    const CORE::CString pending = CookieOf( request, "pending" );
    return !pending.IsNULLOrEmpty() && pending == m_pending;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeLoginSite::LoginPage( bool wrongPassword ) const
{
    return "<h1>Sign in</h1>" +
           CORE::CString( wrongPassword ? "<p id=\"error\" class=\"login-error\">Wrong e-mail address or password</p>" : "" ) +
           "<form id=\"login\" action=\"/login\" method=\"post\">"
           "<input type=\"email\" id=\"email\" name=\"email\">"
           "<input type=\"password\" id=\"password\" name=\"password\">"
           "<button id=\"sign-in\" type=\"submit\">Sign in</button>"
           "</form>";
}

/*-------------------------------------------------------------------------*/

void
CFakeLoginSite::HandleRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response )
{
    MT::CScopeMutex lock( m_lock );
    const CORE::CString path = request.requestUri.SubstrToChar( '?', true );
    const bool isPost = request.requestType == "POST";

    if ( path == "/" )
    {
        Redirect( response, "/login" );
        return;
    }
    if ( path == "/elsewhere" )
    {
        Redirect( response, "http://localhost:" + CORE::ToString( m_port ) + "/login" );
        return;
    }
    if ( path == "/login" )
    {
        if ( !isPost )
        {
            ++m_stats.loginPages;
            if ( HasValidSession( request ) )
                Redirect( response, "/account" );
            else
                SendHtml( response, 200, "Sign in", LoginPage( false ), false );
            return;
        }
        ++m_stats.loginPosts;
        if ( m_email.IsNULLOrEmpty() || FormValueOf( request, "email" ) != m_email || FormValueOf( request, "password" ) != m_password )
        {
            ++m_stats.wrongPasswords;
            SendHtml( response, 200, "Sign in", LoginPage( true ), false );
            return;
        }
        ++m_stats.goodPasswords;
        if ( m_challenge || !m_emailCode.IsNULLOrEmpty() )
        {
            ++m_counter;
            m_pending = "pendingValue" + CORE::ToString( m_counter );
            response.otherHeaders[ "Set-Cookie" ] = "pending=" + m_pending + "; Path=/; HttpOnly";
            Redirect( response, m_challenge ? "/challenge" : "/verify" );
            return;
        }
        IssueSession( response );
        return;
    }
    if ( path == "/challenge" )
    {
        if ( !HasPending( request ) )
        {
            Redirect( response, "/login" );
            return;
        }
        const bool solved = m_challengeApproved || ( isPost && FormValueOf( request, "answer" ) == "person" );
        if ( solved )
        {
            m_challengeApproved = false;
            if ( m_emailCode.IsNULLOrEmpty() )
                IssueSession( response );
            else
                Redirect( response, "/verify" );
            return;
        }
        ++m_stats.challengePages;
        SendHtml( response, 200, "Are you a person?",
                  "<div id=\"captcha\" class=\"captcha-box\">Prove that you are a person</div>"
                  "<form id=\"challenge\" action=\"/challenge\" method=\"post\">"
                  "<input type=\"text\" id=\"answer\" name=\"answer\">"
                  "<button id=\"solve\" type=\"submit\">Go on</button></form>", true );
        return;
    }
    if ( path == "/verify" )
    {
        if ( !HasPending( request ) )
        {
            Redirect( response, "/login" );
            return;
        }
        if ( isPost && !m_emailCode.IsNULLOrEmpty() && FormValueOf( request, "code" ) == m_emailCode )
        {
            IssueSession( response );
            return;
        }
        ++m_stats.codePages;
        SendHtml( response, 200, "Check your e-mail",
                  "<p>We sent a code to your e-mail address.</p>"
                  "<form id=\"verify-form\" action=\"/verify\" method=\"post\">"
                  "<input type=\"text\" id=\"code\" name=\"code\">"
                  "<button id=\"verify\" type=\"submit\">Verify</button></form>", false );
        return;
    }
    if ( path == "/account" )
    {
        if ( !HasValidSession( request ) )
        {
            Redirect( response, "/login" );
            return;
        }
        SendHtml( response, 200, "Your account", "<div id=\"account\">Signed in</div><a id=\"library\" href=\"/api/library\">Library</a>", false );
        return;
    }
    if ( path == "/api/library" )
    {
        if ( !HasValidSession( request ) )
        {
            ++m_stats.apiRefused;
            response.statusCode = 401;
            return;
        }
        ++m_stats.apiAnswered;
        response.statusCode = 200;
        response.contentType = "application/json";
        response.content.CopyFrom( CORE::CString( "{\"items\":[{\"id\":\"pack-1\",\"title\":\"Pack one\"}]}" ), 0, false );
        return;
    }
    response.statusCode = 404;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTHTEST */

/*-------------------------------------------------------------------------*/
