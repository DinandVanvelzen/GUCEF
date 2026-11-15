/*
 *  gucefWEB_TestApp: tests of the gucefWEB module, its web views
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

#ifndef GUCEF_WEB_CWEBVIEWHOSTALLOWLIST_H
#include "gucefWEB_CWebViewHostAllowList.h"
#define GUCEF_WEB_CWEBVIEWHOSTALLOWLIST_H
#endif /* GUCEF_WEB_CWEBVIEWHOSTALLOWLIST_H ? */

#include "TestFakeWebSite.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEBVIEWTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt8 CFakeWebSite::QuadrantColours[ 4 ][ 3 ] = { { 220, 30, 30 }, { 30, 200, 60 }, { 40, 60, 220 }, { 240, 220, 40 } };

/*-------------------------------------------------------------------------*/

CFakeWebSite::SElement::SElement( void )
    : id()
    , kind( ELEMENT_INPUT )
    , label()
    , value()
    , target()
    , trustedOnly( false )
    , disabled( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFakeWebSite::SPage::SPage( void )
    : title()
    , lines()
    , elements()
    , redirectTo()
    , requiresSession( false )
    , signsIn( false )
    , popupUrl()
    , downloadUrl()
    , downloadName()
    , permissions()
    , dialogs()
    , subframeUrl()
    , consoleText()
    , colourBlocks( false )
    , closesWindow( false )
    , loops( false )
    , crashes( false )
    , redefinesQuerySelector( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFakeWebSite::CFakeWebSite( void )
    : m_pages()
{GUCEF_TRACE;

    BuildPages();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeWebSite::Url( const CORE::CString& path )
{GUCEF_TRACE;

    return "https://app.test" + path;
}

/*-------------------------------------------------------------------------*/

bool
CFakeWebSite::SplitUrl( const CORE::CString& url, CORE::CString& host, CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString scheme;
    UInt16 port = 0;
    bool userInfo = false;
    if ( !WEB::CWebViewHostAllowList::SplitUrl( url, scheme, host, port, userInfo ) )
        return false;
    path = "/";
    const Int32 schemeEnd = url.HasSubstr( "://" );
    const Int32 slash = url.HasChar( '/', (UInt32) schemeEnd + 3, true );
    if ( slash >= 0 )
    {
        path = url.CutChars( (UInt32) slash, true );
        Int32 end = path.HasChar( '?' );
        const Int32 fragment = path.HasChar( '#' );
        if ( fragment >= 0 && ( end < 0 || fragment < end ) )
            end = fragment;
        if ( end >= 0 )
            path = path.SubstrToIndex( (UInt32) end, true );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFakeWebSite::SetPage( const CORE::CString& path, const TPage& page )
{GUCEF_TRACE;

    m_pages[ "app.test" + path ] = page;
}

/*-------------------------------------------------------------------------*/

bool
CFakeWebSite::FindPage( const CORE::CString& url, TPage& page, bool& knownHost, bool& certificateError ) const
{GUCEF_TRACE;

    CORE::CString host, path;
    knownHost = false;
    certificateError = false;
    if ( !SplitUrl( url, host, path ) )
        return false;
    knownHost = "app.test" == host || "challenge.test" == host || "bad-cert.test" == host;
    certificateError = "bad-cert.test" == host;
    if ( !knownHost )
        return false;
    TPageMap::const_iterator i = m_pages.find( host + path );
    if ( i == m_pages.end() )
    {
        page = TPage();
        page.title = "Not found";
        page.lines.push_back( "There is no page " + path );
        return false;
    }
    page = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFakeWebSite::BuildPages( void )
{GUCEF_TRACE;

    TPage home;
    home.title = "Home";
    home.lines.push_back( "Welcome to the test site" );
    SElement toLogin;
    toLogin.id = "signin";
    toLogin.kind = ELEMENT_LINK;
    toLogin.label = "Sign in";
    toLogin.target = Url( "/login" );
    home.elements.push_back( toLogin );
    m_pages[ "app.test/" ] = home;

    TPage login;
    login.title = "Sign in";
    login.lines.push_back( "Sign in to the test site" );
    login.signsIn = true;
    SElement user;
    user.id = "user";
    user.kind = ELEMENT_INPUT;
    user.label = "User";
    login.elements.push_back( user );
    SElement password;
    password.id = "password";
    password.kind = ELEMENT_PASSWORD;
    password.label = "Password";
    login.elements.push_back( password );
    SElement submit;
    submit.id = "submit";
    submit.kind = ELEMENT_BUTTON;
    submit.label = "Sign in";
    submit.target = Url( "/account" );
    login.elements.push_back( submit );
    m_pages[ "app.test/login" ] = login;

    TPage account;
    account.title = "Account";
    account.lines.push_back( "Signed in" );
    account.requiresSession = true;
    m_pages[ "app.test/account" ] = account;

    TPage challenge;
    challenge.title = "Challenge";
    challenge.lines.push_back( "Prove you are a person" );
    challenge.subframeUrl = "https://challenge.test/frame";
    SElement code;
    code.id = "code";
    code.kind = ELEMENT_INPUT;
    code.label = "Code";
    code.trustedOnly = true;
    challenge.elements.push_back( code );
    SElement verify;
    verify.id = "verify";
    verify.kind = ELEMENT_BUTTON;
    verify.label = "Verify";
    verify.trustedOnly = true;
    challenge.elements.push_back( verify );
    m_pages[ "app.test/challenge" ] = challenge;

    TPage frame;
    frame.title = "Challenge frame";
    m_pages[ "challenge.test/frame" ] = frame;

    TPage redirect;
    redirect.redirectTo = Url( "/landing" );
    m_pages[ "app.test/redirect" ] = redirect;

    TPage landing;
    landing.title = "Landing";
    landing.lines.push_back( "You were sent here" );
    m_pages[ "app.test/landing" ] = landing;

    TPage redirectOut;
    redirectOut.redirectTo = "https://elsewhere.example/";
    m_pages[ "app.test/redirect-out" ] = redirectOut;

    TPage popup;
    popup.title = "Popup";
    popup.popupUrl = Url( "/landing" );
    m_pages[ "app.test/popup" ] = popup;

    TPage download;
    download.title = "Download";
    download.downloadUrl = Url( "/file.bin" );
    download.downloadName = "file.bin";
    m_pages[ "app.test/download" ] = download;

    TPage permissions;
    permissions.title = "Permissions";
    permissions.permissions.push_back( WEB::WEBVIEWPERMISSION_NOTIFICATIONS );
    permissions.permissions.push_back( WEB::WEBVIEWPERMISSION_GEOLOCATION );
    m_pages[ "app.test/permissions" ] = permissions;

    TPage dialogs;
    dialogs.title = "Dialogs";
    dialogs.dialogs.push_back( WEB::WEBVIEWDIALOG_ALERT );
    dialogs.dialogs.push_back( WEB::WEBVIEWDIALOG_CONFIRM );
    dialogs.dialogs.push_back( WEB::WEBVIEWDIALOG_PROMPT );
    m_pages[ "app.test/dialogs" ] = dialogs;

    TPage colours;
    colours.title = "Colours";
    colours.colourBlocks = true;
    m_pages[ "app.test/colours" ] = colours;

    TPage close;
    close.title = "Closing";
    close.closesWindow = true;
    m_pages[ "app.test/close" ] = close;

    TPage loop;
    loop.title = "Busy";
    loop.loops = true;
    m_pages[ "app.test/loop" ] = loop;

    TPage crash;
    crash.title = "Crash";
    crash.crashes = true;
    m_pages[ "app.test/crash" ] = crash;

    TPage console;
    console.title = "Console";
    console.consoleText = "A message of the page";
    m_pages[ "app.test/console" ] = console;

    TPage hostile;
    hostile.title = "Hostile";
    hostile.lines.push_back( "This page replaces document.querySelector" );
    hostile.redefinesQuerySelector = true;
    SElement field;
    field.id = "field";
    field.kind = ELEMENT_INPUT;
    field.label = "Field";
    hostile.elements.push_back( field );
    m_pages[ "app.test/hostile" ] = hostile;

    TPage badCert;
    badCert.title = "Bad certificate";
    m_pages[ "bad-cert.test/" ] = badCert;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEBVIEWTEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
