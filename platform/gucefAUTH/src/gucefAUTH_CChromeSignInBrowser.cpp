/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_WEB_CHTTPLOGREDACTION_H
#include "gucefWEB_CHttpLogRedaction.h"
#define GUCEF_WEB_CHTTPLOGREDACTION_H
#endif /* GUCEF_WEB_CHTTPLOGREDACTION_H ? */
#ifndef GUCEF_WEB_CCHROMEBROWSERLOCATOR_H
#include "gucefWEB_CChromeBrowserLocator.h"
#define GUCEF_WEB_CCHROMEBROWSERLOCATOR_H
#endif /* GUCEF_WEB_CCHROMEBROWSERLOCATOR_H ? */
#ifndef GUCEF_WEB_CWEBPAGESCRIPTHELPERS_H
#include "gucefWEB_CWebPageScriptHelpers.h"
#define GUCEF_WEB_CWEBPAGESCRIPTHELPERS_H
#endif /* GUCEF_WEB_CWEBPAGESCRIPTHELPERS_H ? */
#ifndef GUCEF_AUTH_CCHROMESIGNINBROWSER_H
#include "gucefAUTH_CChromeSignInBrowser.h"
#define GUCEF_AUTH_CCHROMESIGNINBROWSER_H
#endif /* GUCEF_AUTH_CCHROMESIGNINBROWSER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CChromeSignInBrowser::ActivePortFileName = "DevToolsActivePort";

/*-------------------------------------------------------------------------*/

namespace {

UInt64
NowInMs( void )
{
    return (UInt64) ( MT::PrecisionTickCount() / ( MT::PrecisionTimerResolution() / 1000.0 ) );
}

}

/*-------------------------------------------------------------------------*/

CChromeSignInBrowser::CChromeSignInBrowser( void )
    : CIWebSignInBrowser()
    , m_process()
    , m_pulseGenerator()
    , m_client( GUCEF_NULL )
    , m_pageSessionId()
    , m_settings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CChromeSignInBrowser::~CChromeSignInBrowser()
{GUCEF_TRACE;

    if ( m_settings.keepOpen )
        Release();
    else
        Stop();
}

/*-------------------------------------------------------------------------*/

void
CChromeSignInBrowser::BuildArguments( const WEB::CWebSignInBrowserSettings& settings, const CORE::CString& profileDirectory, CORE::CProcess::TStringVector& arguments )
{GUCEF_TRACE;

    arguments.clear();
    arguments.push_back( "--remote-debugging-port=" + CORE::ToString( (UInt32) settings.debuggingPort ) );
    arguments.push_back( "--user-data-dir=" + profileDirectory );
    arguments.push_back( "--no-first-run" );
    arguments.push_back( "--no-default-browser-check" );
    if ( settings.headless )
        arguments.push_back( "--headless=new" );
    for ( size_t i=0; i<settings.extraArguments.size(); ++i )
    {
        if ( WEB::CWebSignInBrowserSettings::IsArgumentAllowed( settings.extraArguments[ i ] ) )
            arguments.push_back( settings.extraArguments[ i ] );
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "ChromeSignInBrowser: The browser argument " << settings.extraArguments[ i ] << " is not allowed and left out: the debugging port and the profile are set by the sign in" );
        }
    }
    arguments.push_back( "about:blank" );
}

/*-------------------------------------------------------------------------*/

bool
CChromeSignInBrowser::ReadActivePortFile( const CORE::CString& path, UInt16& port, CORE::CString& browserPath )
{GUCEF_TRACE;

    if ( !CORE::FileExists( path ) )
        return false;
    CORE::CString content;
    if ( !CORE::LoadTextFileAsString( path, content, true, "\n" ) )
        return false;
    const CORE::CString::StringVector lines = content.ParseElements( '\n', false );
    if ( lines.size() < 2 )
        return false;
    const CORE::CString portText = lines[ 0 ].Trim( true ).Trim( false );
    const UInt32 value = CORE::StringToUInt32( portText, 0 );
    browserPath = lines[ 1 ].Trim( true ).Trim( false );
    if ( 0 == value || value > 65535 || !browserPath.StartsWith( "/devtools/browser/" ) )
        return false;
    port = (UInt16) value;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChromeSignInBrowser::Start( const WEB::CWebSignInBrowserSettings& settings, CORE::CString& problem )
{GUCEF_TRACE;

    Stop();
    m_settings = settings;

    CORE::CString executable;
    if ( !WEB::CChromeBrowserLocator::FindBrowser( settings.executablePath, executable ) )
    {
        problem = settings.executablePath.IsNULLOrEmpty() ?
            CORE::CString( "No browser was found: install Google Chrome, Chromium, Microsoft Edge or Brave, or set the path of one" ) :
            "The browser " + settings.executablePath + " does not exist";
        return false;
    }

    const CORE::CString profileDirectory = CORE::RelativePath( settings.profileDirectory );
    if ( profileDirectory.IsNULLOrEmpty() || !CORE::CreateDirs( profileDirectory ) )
    {
        problem = "The profile folder of the browser " + profileDirectory + " cannot be made";
        return false;
    }
    CORE::SetOwnerOnlyAccess( profileDirectory );

    CORE::CString activePortFile = profileDirectory;
    CORE::AppendToPath( activePortFile, ActivePortFileName );
    if ( CORE::FileExists( activePortFile ) )
        CORE::DeleteFile( activePortFile );

    CORE::CProcess::TStringVector arguments;
    BuildArguments( settings, profileDirectory, arguments );

    // the browser writes nothing to our terminal and gets none of our handles; it is closed with us unless kept open
    m_process.SetIsolateFromParent( true );
    if ( !m_process.Launch( executable, arguments, CORE::CString::Empty, CORE::CProcess::TStringMap(), settings.keepOpen ) )
    {
        problem = "The browser " + executable + " could not be started";
        return false;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ChromeSignInBrowser: Started " << executable << ( settings.headless ? " without a window" : " with a window" ) <<
        ", process " << m_process.GetProcessId() );

    // the port is known once the browser wrote it to its profile folder
    const UInt32 timeout = settings.startTimeoutInMs > 0 ? settings.startTimeoutInMs : WEB::CWebSignInBrowserSettings::DefaultStartTimeoutInMs;
    const UInt64 deadline = NowInMs() + timeout;
    UInt16 port = 0;
    CORE::CString browserPath;
    while ( !ReadActivePortFile( activePortFile, port, browserPath ) )
    {
        if ( !m_process.IsRunning() )
        {
            Int32 exitCode = 0;
            m_process.WaitForExit( 0, exitCode );
            problem = "The browser ended right after it started (exit code " + CORE::ToString( exitCode ) +
                      "): is its profile folder in use by another browser window?";
            Stop();
            return false;
        }
        if ( NowInMs() >= deadline )
        {
            problem = "The browser did not open its debugging port in time";
            Stop();
            return false;
        }
        MT::ThreadDelay( 50 );
    }
    if ( 0 != settings.debuggingPort && port != settings.debuggingPort )
    {
        problem = "The browser opened another debugging port than the one asked for: is another browser using the profile folder?";
        Stop();
        return false;
    }

    const UInt64 now = NowInMs();
    if ( !ConnectToBrowser( port, browserPath, deadline > now ? (UInt32) ( deadline - now ) : 1000, problem ) )
    {
        Stop();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChromeSignInBrowser::ConnectToBrowser( UInt16 port, const CORE::CString& browserPath, UInt32 timeoutInMs, CORE::CString& problem )
{GUCEF_TRACE;

    // a pulse generator of our own, driven by this thread, so waiting for the browser never pumps one of another thread
    m_pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
    m_pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
    m_client = GUCEF_NEW WEB::CChromeDevToolsClient( m_pulseGenerator );
    m_client->SetCommandTimeout( m_settings.commandTimeoutInMs > 0 ? m_settings.commandTimeoutInMs : WEB::CWebSignInBrowserSettings::DefaultCommandTimeoutInMs );

    const CORE::CString url = "ws://127.0.0.1:" + CORE::ToString( (UInt32) port ) + browserPath;
    if ( !m_client->ConnectAndWait( url, timeoutInMs ) )
    {
        problem = "No connection to the debugging port of the browser";
        return false;
    }

    // the page the browser opened, else a new one
    CORE::CString targetId;
    WEB::TChromeTargetVector targets;
    if ( m_client->TargetGetTargets( targets ) )
    {
        for ( size_t i=0; i<targets.size(); ++i )
        {
            if ( targets[ i ].type == "page" )
            {
                targetId = targets[ i ].id;
                break;
            }
        }
    }
    if ( targetId.IsNULLOrEmpty() && !m_client->TargetCreate( "about:blank", targetId ) )
    {
        problem = "The browser has no page to sign in on";
        return false;
    }
    if ( !m_client->AttachToTarget( targetId, m_pageSessionId ) )
    {
        problem = "The page of the browser cannot be driven";
        return false;
    }
    m_client->SetSessionId( m_pageSessionId );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CChromeSignInBrowser::Disconnect( bool closeBrowser )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_client )
    {
        if ( closeBrowser && m_client->IsConnected() )
        {
            m_client->SetSessionId( CORE::CString::Empty );
            m_client->BrowserClose();
        }
        m_client->Disconnect();
        GUCEF_DELETE m_client;
        m_client = GUCEF_NULL;
    }
    m_pageSessionId.Clear();
    m_pulseGenerator = CORE::PulseGeneratorPtr();
}

/*-------------------------------------------------------------------------*/

void
CChromeSignInBrowser::Stop( void )
{GUCEF_TRACE;

    Disconnect( true );
    if ( 0 != m_process.GetProcessId() && m_process.IsRunning() )
    {
        Int32 exitCode = 0;
        if ( !m_process.WaitForExit( 5000, exitCode ) )
            m_process.Terminate( true, 3000 );
    }
}

/*-------------------------------------------------------------------------*/

void
CChromeSignInBrowser::Release( void )
{GUCEF_TRACE;

    Disconnect( false );
}

/*-------------------------------------------------------------------------*/

bool
CChromeSignInBrowser::IsAlive( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client )
        return false;
    m_pulseGenerator->ForceDirectPulse();
    return m_client->IsConnected();
}

/*-------------------------------------------------------------------------*/

bool
CChromeSignInBrowser::Evaluate( const CORE::CString& expression, CORE::CDataNode& result )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client || !m_client->IsConnected() )
        return false;
    return m_client->RuntimeEvaluate( expression, false, result );
}

/*-------------------------------------------------------------------------*/

bool
CChromeSignInBrowser::Navigate( const CORE::CString& url )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client || !m_client->IsConnected() )
        return false;
    return m_client->PageNavigate( url );
}

/*-------------------------------------------------------------------------*/

bool
CChromeSignInBrowser::GetLocation( CORE::CString& url )
{GUCEF_TRACE;

    CORE::CDataNode result;
    if ( !Evaluate( WEB::CWebPageScriptHelpers::LocationExpression(), result ) )
        return false;
    url = result.GetValue().AsString();
    return true;
}

/*-------------------------------------------------------------------------*/

WEB::TWebElementState
CChromeSignInBrowser::GetElementState( const CORE::CString& selector )
{GUCEF_TRACE;

    CORE::CDataNode result;
    if ( !Evaluate( WEB::CWebPageScriptHelpers::ElementStateExpression( selector ), result ) )
        return WEB::WEBELEMENT_STATE_NO_ANSWER;
    switch ( result.GetValue().AsInt32( -2 ) )
    {
        case -1: return WEB::WEBELEMENT_STATE_INVALID_SELECTOR;
        case 0: return WEB::WEBELEMENT_STATE_ABSENT;
        case 1: return WEB::WEBELEMENT_STATE_PRESENT;
        case 2: return WEB::WEBELEMENT_STATE_VISIBLE;
        default: return WEB::WEBELEMENT_STATE_NO_ANSWER;
    }
}

/*-------------------------------------------------------------------------*/

bool
CChromeSignInBrowser::ContainsText( const CORE::CString& text, bool& contains )
{GUCEF_TRACE;

    CORE::CDataNode result;
    if ( !Evaluate( WEB::CWebPageScriptHelpers::ContainsTextExpression( text ), result ) )
        return false;
    contains = result.GetValue().AsBool( false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChromeSignInBrowser::FillField( const CORE::CString& selector, const AUTH::CSecretString& value )
{GUCEF_TRACE;

    CORE::CDataNode result;
    if ( !Evaluate( WEB::CWebPageScriptHelpers::FocusAndClearExpression( selector ), result ) || !result.GetValue().AsBool( false ) )
        return false;
    if ( value.IsEmpty() )
        return true;

    // typed into the field with the focus: the value is a parameter of the command, never part of an expression
    if ( !m_client->InputInsertText( value.Reveal() ) )
        return false;
    if ( !Evaluate( WEB::CWebPageScriptHelpers::ValueLengthExpression( selector ), result ) )
        return false;
    return result.GetValue().AsInt32( -1 ) > 0;
}

/*-------------------------------------------------------------------------*/

bool
CChromeSignInBrowser::Click( const CORE::CString& selector )
{GUCEF_TRACE;

    CORE::CDataNode result;
    return Evaluate( WEB::CWebPageScriptHelpers::ClickExpression( selector ), result ) && result.GetValue().AsBool( false );
}

/*-------------------------------------------------------------------------*/

bool
CChromeSignInBrowser::Submit( const CORE::CString& selector )
{GUCEF_TRACE;

    CORE::CDataNode result;
    return Evaluate( WEB::CWebPageScriptHelpers::SubmitExpression( selector ), result ) && result.GetValue().AsBool( false );
}

/*-------------------------------------------------------------------------*/

bool
CChromeSignInBrowser::GetCookies( TWebSessionCookieVector& cookies )
{GUCEF_TRACE;

    cookies.clear();
    if ( GUCEF_NULL == m_client || !m_client->IsConnected() )
        return false;

    // a command of the browser, not of the page
    m_client->SetSessionId( CORE::CString::Empty );
    WEB::TChromeCookieVector browserCookies;
    const bool success = m_client->StorageGetCookies( browserCookies );
    m_client->SetSessionId( m_pageSessionId );
    for ( size_t i=0; i<browserCookies.size(); ++i )
    {
        cookies.push_back( CWebSessionCookie::FromChromeCookie( browserCookies[ i ] ) );
        browserCookies[ i ].value.Clear();
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CChromeSignInBrowser::ClearCookies( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client || !m_client->IsConnected() )
        return false;
    m_client->SetSessionId( CORE::CString::Empty );
    const bool success = m_client->StorageClearCookies();
    m_client->SetSessionId( m_pageSessionId );
    return success;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
