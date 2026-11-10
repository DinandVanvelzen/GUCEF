/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#include "TestBrowser.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTestBrowser::CTestBrowser( void )
    : CORE::CIBrowserLauncherHandler()
    , MT::CActiveObject()
    , m_lock()
    , m_mode( MODE_FOLLOW )
    , m_launchResult( true )
    , m_queue()
    , m_urls()
    , m_redirects()
    , m_relayed()
    , m_statuses()
    , m_running( 0 )
    , m_previous( GUCEF_NULL )
    , m_installed( false )
    , m_started( false )
    , m_client()
{
}

/*-------------------------------------------------------------------------*/

CTestBrowser::~CTestBrowser()
{
    Uninstall();
    Stop();
}

/*-------------------------------------------------------------------------*/

bool
CTestBrowser::Start( void )
{
    if ( m_started )
        return true;
    m_started = Activate( GUCEF_NULL, 1, 1 );
    return m_started;
}

/*-------------------------------------------------------------------------*/

void
CTestBrowser::Stop( void )
{
    if ( !m_started )
        return;
    m_started = false;
    Deactivate( false, true );
}

/*-------------------------------------------------------------------------*/

void
CTestBrowser::Install( void )
{
    if ( !m_installed )
    {
        m_previous = CORE::SetBrowserLauncherHandler( this );
        m_installed = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CTestBrowser::Uninstall( void )
{
    if ( m_installed )
    {
        CORE::SetBrowserLauncherHandler( m_previous );
        m_previous = GUCEF_NULL;
        m_installed = false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CTestBrowser::OpenUrl( const CORE::CString& url )
{
    MT::CScopeMutex lock( m_lock );
    m_urls.push_back( url );
    if ( !m_launchResult )
        return false;
    m_queue.push_back( url );
    ++m_running;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTestBrowser::Browse( const CORE::CString& url )
{
    MT::CScopeMutex lock( m_lock );
    m_queue.push_back( url );
    ++m_running;
}

/*-------------------------------------------------------------------------*/

void
CTestBrowser::SetMode( EMode mode )
{
    MT::CScopeMutex lock( m_lock );
    m_mode = mode;
}

/*-------------------------------------------------------------------------*/

void
CTestBrowser::SetLaunchResult( bool launched )
{
    MT::CScopeMutex lock( m_lock );
    m_launchResult = launched;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestBrowser::GetOpenCount( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_urls.size();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestBrowser::GetLastUrl( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_urls.empty() ? CORE::CString::Empty : m_urls.back();
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CTestBrowser::GetRedirects( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_redirects;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CTestBrowser::GetRelayed( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_relayed;
}

/*-------------------------------------------------------------------------*/

vector< int >
CTestBrowser::GetStatuses( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_statuses;
}

/*-------------------------------------------------------------------------*/

bool
CTestBrowser::IsIdle( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return 0 == m_running;
}

/*-------------------------------------------------------------------------*/

void
CTestBrowser::ClearHistory( void )
{
    MT::CScopeMutex lock( m_lock );
    m_urls.clear();
    m_redirects.clear();
    m_relayed.clear();
    m_statuses.clear();
}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CTestBrowser::AsLockable( void ) const
{
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CTestBrowser::OnThreadStart( void* taskdata )
{
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestBrowser::OnThreadCycle( void* taskdata )
{
    CORE::CString url;
    EMode mode = MODE_FOLLOW;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_queue.empty() )
            return false;
        url = m_queue.front();
        m_queue.erase( m_queue.begin() );
        mode = m_mode;
    }
    RunScript( url, mode );
    {
        MT::CScopeMutex lock( m_lock );
        --m_running;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
FindHeader( const WEB::CHttpResponseData& response, const CORE::CString& name )
{
    for ( WEB::CHttpResponseData::TStringMap::const_iterator i = response.otherHeaders.begin(); i != response.otherHeaders.end(); ++i )
        if ( (*i).first.Equals( name, false ) )
            return (*i).second;
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

int
CTestBrowser::Fetch( const CORE::CString& url, const CORE::CString& method, const CORE::CString& hostHeaderUrl, CORE::CString& location )
{
    // The client lives as long as the browser: a notifier that is made or destroyed on another thread than the one that pumps its pulse generator
    // can deadlock with it
    WEB::CHTTPClient& client = m_client;
    WEB::CHttpRequestData request;
    WEB::CHttpClientRequestOptions options;
    options.followRedirects = false;
    options.totalTimeoutInMs = 10000;
    request.requestType = method;
    if ( hostHeaderUrl.IsNULLOrEmpty() )
    {
        if ( !request.SetUrl( url ) )
            return 0;
    }
    else
    {
        // The request goes to the loopback listener, with the name of another host in it: through a "proxy" that is the listener itself
        WEB::CHttpRequestData target;
        if ( !target.SetUrl( url ) || !request.SetUrl( hostHeaderUrl ) )
            return 0;
        options.proxyHost = target.GetHostName();
        options.proxyPort = target.GetPort();
    }
    if ( "POST" == method )
        request.contentRepresentation = "text/plain";

    WEB::CHttpResponseData response;
    WEB::CHttpClientTransferInfo info;
    if ( !client.Execute( request, options, response, &info ) )
        return 0;
    location = FindHeader( response, "Location" );
    return response.statusCode;
}

/*-------------------------------------------------------------------------*/

/**
 *  The page of the relay: the same checks as the page itself, from the address the provider redirected to (a page on the domain of
 *  the developer) to the address on the loopback interface. False when the page would refuse.
 */
static bool
RelayTarget( const CORE::CString& redirect, bool changeState, bool renameState, CORE::CString& target )
{
    target.Clear();
    CORE::CString scheme, host, path, query;
    UInt16 port = 0;
    if ( !AUTH::CAuthUrl::Split( redirect, scheme, host, port, path, query ) )
        return false;
    CORE::CString::StringMap parameters;
    bool duplicate = false;
    if ( !AUTH::CAuthUrl::ParseQuery( query, parameters, duplicate ) || duplicate )
        return false;
    if ( parameters.find( "openid.mode" ) == parameters.end() || parameters.find( "port" ) == parameters.end() || parameters.find( "s" ) == parameters.end() )
        return false;
    CORE::CString portText = parameters[ "port" ];
    UInt32 portSize = (UInt32) portText.Length();
    if ( portSize < 4 || portSize > 5 || '0' == portText[ 0 ] )
        return false;
    for ( UInt32 i=0; i<portSize; ++i )
        if ( portText[ i ] < '0' || portText[ i ] > '9' )
            return false;
    UInt32 relayPort = CORE::StringToUInt32( portText );
    if ( relayPort < AUTH::COpenId2Relay::MinPort || relayPort > 65535 || !AUTH::COpenId2Relay::IsValidState( parameters[ "s" ] ) )
        return false;
    if ( changeState )
    {
        parameters[ "s" ] = "not-the-state-but-long-enough";
        query = AUTH::CAuthUrl::EncodeForm( parameters );
    }
    else
    if ( renameState )
    {
        parameters[ "state" ] = parameters[ "s" ];
        parameters.erase( "s" );
        query = AUTH::CAuthUrl::EncodeForm( parameters );
    }
    target = "http://127.0.0.1:" + portText + AUTH::COpenId2Relay::CallbackPath + "?" + query;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTestBrowser::RunScript( const CORE::CString& url, EMode mode )
{
    if ( MODE_IGNORE == mode )
        return;

    // the provider authorizes and redirects
    CORE::CString redirect;
    int status = Fetch( url, "GET", CORE::CString::Empty, redirect );
    if ( 302 != status || redirect.IsNULLOrEmpty() )
    {
        MT::CScopeMutex lock( m_lock );
        m_statuses.push_back( status );
        return;
    }
    {
        MT::CScopeMutex lock( m_lock );
        m_redirects.push_back( redirect );
    }
    if ( MODE_CAPTURE == mode )
        return;

    if ( MODE_RELAY == mode || MODE_RELAY_WRONG_STATE == mode || MODE_RELAY_STATE_RENAMED == mode )
    {
        CORE::CString target;
        if ( !RelayTarget( redirect, MODE_RELAY_WRONG_STATE == mode, MODE_RELAY_STATE_RENAMED == mode, target ) )
        {
            MT::CScopeMutex lock( m_lock );
            m_statuses.push_back( 0 );
            return;
        }
        {
            MT::CScopeMutex lock( m_lock );
            m_relayed.push_back( target );
        }
        CORE::CString ignoredLocation;
        int relayed = Fetch( target, "GET", CORE::CString::Empty, ignoredLocation );
        MT::CScopeMutex lock( m_lock );
        m_statuses.push_back( relayed );
        return;
    }

    // the address the browser goes to
    CORE::CString scheme, host, path, query;
    UInt16 port = 0;
    CORE::CString dummy;
    if ( !AUTH::CAuthUrl::Split( redirect, scheme, host, port, path, query ) )
        return;
    CORE::CString origin = scheme + "://" + host + ":" + CORE::ToString( port );
    CORE::CString::StringMap parameters;
    bool duplicate = false;
    AUTH::CAuthUrl::ParseQuery( query, parameters, duplicate );

    CORE::CString::StringMap wrong = parameters;
    wrong[ "state" ] = "not-the-state";
    CORE::CString wrongStateUrl = origin + path + "?" + AUTH::CAuthUrl::EncodeForm( wrong );

    int result = 0;
    CORE::CString ignored;
    if ( MODE_WRONG_STATE_ONLY == mode )
    {
        result = Fetch( wrongStateUrl, "GET", CORE::CString::Empty, ignored );
        MT::CScopeMutex lock( m_lock );
        m_statuses.push_back( result );
        return;
    }

    if ( MODE_SEQUENCE == mode )
    {
        GUCEF::vector< int > statuses;
        statuses.push_back( Fetch( redirect, "GET", "http://evil.example.invalid:" + CORE::ToString( port ) + path + "?" + query, ignored ) );
        statuses.push_back( Fetch( redirect, "POST", CORE::CString::Empty, ignored ) );
        statuses.push_back( Fetch( origin + "/other?" + query, "GET", CORE::CString::Empty, ignored ) );
        statuses.push_back( Fetch( wrongStateUrl, "GET", CORE::CString::Empty, ignored ) );
        statuses.push_back( Fetch( redirect, "GET", CORE::CString::Empty, ignored ) );
        statuses.push_back( Fetch( redirect, "GET", CORE::CString::Empty, ignored ) );
        MT::CScopeMutex lock( m_lock );
        for ( size_t i=0; i<statuses.size(); ++i )
            m_statuses.push_back( statuses[ i ] );
        return;
    }

    result = Fetch( redirect, "GET", CORE::CString::Empty, ignored );
    MT::CScopeMutex lock( m_lock );
    m_statuses.push_back( result );
}

/*-------------------------------------------------------------------------*/
