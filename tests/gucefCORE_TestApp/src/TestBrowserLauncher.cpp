/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#ifndef GUCEF_CORE_BROWSERLAUNCHER_H
#include "gucefCORE_BrowserLauncher.h"
#define GUCEF_CORE_BROWSERLAUNCHER_H
#endif /* GUCEF_CORE_BROWSERLAUNCHER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CILOGGER_H
#include "CILogger.h"
#define GUCEF_CORE_CILOGGER_H
#endif /* GUCEF_CORE_CILOGGER_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

#ifndef GUCEF_CORE_CTIMESTAMP_H
#include "gucefCORE_CTimestamp.h"
#define GUCEF_CORE_CTIMESTAMP_H
#endif /* GUCEF_CORE_CTIMESTAMP_H ? */

#ifndef GUCEF_CORE_CLOGSTREAMSCOPE_H
#include "gucefCORE_CLogStreamScope.h"
#define GUCEF_CORE_CLOGSTREAMSCOPE_H
#endif /* GUCEF_CORE_CLOGSTREAMSCOPE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestBrowserLauncher.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

/** Records the URLs it is given and opens nothing */
class CRecordingBrowserLauncherHandler : public CORE::CIBrowserLauncherHandler
{
    public:

    CRecordingBrowserLauncherHandler( bool result )
        : CORE::CIBrowserLauncherHandler()
        , m_result( result )
        , m_calls( 0 )
        , m_lastUrl()
    {GUCEF_TRACE;
    }

    virtual bool OpenUrl( const CORE::CString& url ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        ++m_calls;
        m_lastUrl = url;
        return m_result;
    }

    bool m_result;
    UInt32 m_calls;
    CORE::CString m_lastUrl;
};

/*-------------------------------------------------------------------------*/

/** Gives a token as told and notes what it was asked */
class CFakeActivationTokenProvider : public CORE::CIActivationTokenProvider
{
    public:

    CFakeActivationTokenProvider( void )
        : CORE::CIActivationTokenProvider()
        , m_result( true )
        , m_token( "token-1234-secret" )
        , m_kind( CORE::ACTIVATIONTOKEN_XDG_ACTIVATION )
        , m_calls( 0 )
        , m_lastMaxWaitInMs( 0 )
    {GUCEF_TRACE;
    }

    virtual bool GetActivationToken( CORE::CString& token, CORE::TActivationTokenKind& kind, UInt32 maxWaitInMs ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        ++m_calls;
        m_lastMaxWaitInMs = maxWaitInMs;
        token = m_token;
        kind = m_kind;
        return m_result;
    }

    bool m_result;
    CORE::CString m_token;
    CORE::TActivationTokenKind m_kind;
    UInt32 m_calls;
    UInt32 m_lastMaxWaitInMs;
};

/*-------------------------------------------------------------------------*/

/** Starts nothing: notes what the launcher would have started */
class CRecordingHelperLauncher : public CORE::CIBrowserHelperLauncher
{
    public:

    CRecordingHelperLauncher( void )
        : CORE::CIBrowserHelperLauncher()
        , m_result( true )
        , m_calls( 0 )
        , m_isolated( false )
        , m_exePath()
        , m_args()
        , m_environment()
    {GUCEF_TRACE;
    }

    virtual bool LaunchHelper( const CORE::CString& exePath                      ,
                               const CORE::CProcess::TStringVector& args         ,
                               const CORE::CProcess::TStringMap& envOverrides    ,
                               bool isolated                                     ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        ++m_calls;
        m_isolated = isolated;
        m_exePath = exePath;
        m_args = args;
        m_environment = envOverrides;
        return m_result;
    }

    bool Has( const char* name ) const
    {
        return m_environment.find( name ) != m_environment.end();
    }

    CORE::CString Get( const char* name ) const
    {
        CORE::CProcess::TStringMap::const_iterator i = m_environment.find( name );
        return i != m_environment.end() ? (*i).second : CORE::CString();
    }

    bool m_result;
    UInt32 m_calls;
    bool m_isolated;
    CORE::CString m_exePath;
    CORE::CProcess::TStringVector m_args;
    CORE::CProcess::TStringMap m_environment;
};

/*-------------------------------------------------------------------------*/

/** Keeps what is logged */
class CCaptureLogger : public CORE::CILogger
{
    public:

    CCaptureLogger( void ) : CORE::CILogger(), m_text(), m_minLogLevel( CORE::LOGLEVEL_EVERYTHING ) {}

    virtual void Log( const CORE::TLogMsgType, const Int32, const CORE::CString& logMessage, const UInt32, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_text += logMessage;
        m_text += '\n';
    }

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage, threadId, timestamp );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
    }

    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE {}

    virtual void SetMinimalLogLevel( const Int32 logLevel ) GUCEF_VIRTUAL_OVERRIDE { m_minLogLevel = logLevel; }

    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_minLogLevel; }

    CORE::CString m_text;
    Int32 m_minLogLevel;
};

/*-------------------------------------------------------------------------*/

void
PerformBrowserLauncherTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING BrowserLauncher TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "BrowserLauncher" );

    GUCEF_TESTFW_TESTCASE( "Test: Plain http and https URLs are openable, in any case of the scheme, with a port, a path, a query, a fragment and an IPv6 host" )
        try
        {
            ASSERT_TRUE( CORE::IsOpenableBrowserUrl( "https://example.com" ) );
            ASSERT_TRUE( CORE::IsOpenableBrowserUrl( "http://example.com/" ) );
            ASSERT_TRUE( CORE::IsOpenableBrowserUrl( "HTTPS://EXAMPLE.COM/" ) );
            ASSERT_TRUE( CORE::IsOpenableBrowserUrl( "HtTp://example.com" ) );
            ASSERT_TRUE( CORE::IsOpenableBrowserUrl( "http://127.0.0.1:8080/callback" ) );
            ASSERT_TRUE( CORE::IsOpenableBrowserUrl( "http://[::1]:8080/callback?code=a1&state=b2#frag" ) );
            ASSERT_TRUE( CORE::IsOpenableBrowserUrl( "https://accounts.google.com/o/oauth2/v2/auth?client_id=1234-abc.apps.googleusercontent.com&redirect_uri=http%3A%2F%2F127.0.0.1%3A53682%2Fcallback&response_type=code&scope=openid%20email&code_challenge=E9Melhoa2OwvFrEMTJguCHaoeK1t8URWbuGJSstw-cM&code_challenge_method=S256&state=xyz_123~abc" ) );
            ASSERT_TRUE( CORE::IsOpenableBrowserUrl( "https://example.com/a(b)*c!d,e;f=g+h@i" ) );
            ASSERT_TRUE( CORE::IsOpenableBrowserUrl( "https://example.com?x=1" ) );
            ASSERT_TRUE( CORE::IsOpenableBrowserUrl( "https://example.com#top" ) );
            ASSERT_TRUE( CORE::IsOpenableBrowserUrl( "https://localhost" ) );
            ASSERT_TRUE( CORE::IsOpenableBrowserUrl( "https://a" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Other schemes, missing hosts, user information, control characters, whitespace and shell or command line characters are refused" )
        try
        {
            // other schemes
            const char* otherSchemes[] =
            {
                "ftp://example.com/", "file:///etc/passwd", "javascript:alert(1)", "data:text/html,<script>alert(1)</script>",
                "mailto:someone@example.com", "steam://run/480", "ms-msdt:/id", "vbscript:msgbox(1)", "about:blank", "chrome://settings",
                "example.com", "//example.com", "/etc/passwd", "ssh://example.com", "x-http://example.com", "httpx://example.com",
                "http:example.com", "http:/example.com", "https:\\\\example.com", "http//example.com", "http:///example.com"
            };
            for ( UInt32 i=0; i<sizeof( otherSchemes ) / sizeof( otherSchemes[ 0 ] ); ++i )
            {
                ASSERT_FALSE( CORE::IsOpenableBrowserUrl( otherSchemes[ i ] ) );
                if ( CORE::IsOpenableBrowserUrl( otherSchemes[ i ] ) )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Accepted the URL \"" << otherSchemes[ i ] << "\"" );
            }

            // nothing, a scheme without a host, a host that does not start like one
            ASSERT_FALSE( CORE::IsOpenableBrowserUrl( CORE::CString::Empty ) );
            ASSERT_FALSE( CORE::IsOpenableBrowserUrl( "http://" ) );
            ASSERT_FALSE( CORE::IsOpenableBrowserUrl( "https://" ) );
            ASSERT_FALSE( CORE::IsOpenableBrowserUrl( "https:///path" ) );
            ASSERT_FALSE( CORE::IsOpenableBrowserUrl( "https://?query" ) );
            ASSERT_FALSE( CORE::IsOpenableBrowserUrl( "https://#frag" ) );
            ASSERT_FALSE( CORE::IsOpenableBrowserUrl( "https://:80/" ) );
            ASSERT_FALSE( CORE::IsOpenableBrowserUrl( "https://.example.com" ) );
            ASSERT_FALSE( CORE::IsOpenableBrowserUrl( "https://-example.com" ) );
            ASSERT_FALSE( CORE::IsOpenableBrowserUrl( "https://%65xample.com" ) );

            // user information disguises the host
            ASSERT_FALSE( CORE::IsOpenableBrowserUrl( "https://accounts.google.com@evil.example/" ) );
            ASSERT_FALSE( CORE::IsOpenableBrowserUrl( "https://user:pass@example.com/" ) );
            ASSERT_FALSE( CORE::IsOpenableBrowserUrl( "https://@example.com/" ) );

            // control characters and whitespace, at the start, the end and in the middle
            const char* whitespace[] =
            {
                " https://example.com/", "https://example.com/ ", "https://example.com/a b", "https://example.com/\t", "\thttps://example.com/",
                "https://example.com/\n", "\nhttps://example.com/", "https://example.com/\r\nHost: evil", "https://exa\nmple.com/", "https://example.com/\r",
                "https://example.com/\v", "https://example.com/\f", "https://example.com/\x01", "https://example.com/\x1f", "https://example.com/\x7f",
                "https://example.com/\x1b[31m", "https://exam ple.com", "https://example.com\xC2\xA0/", "https://example.com/\xE2\x80\xA8",
                "https://example.com/\xC3\xA9", "https://\xD0\xB5xample.com/", "https://example.com/\x80"
            };
            for ( UInt32 i=0; i<sizeof( whitespace ) / sizeof( whitespace[ 0 ] ); ++i )
            {
                ASSERT_FALSE( CORE::IsOpenableBrowserUrl( whitespace[ i ] ) );
            }

            // a NUL ends a C string, so what follows it can never be part of a URL; the text before it stands on its own
            const char withNul[] = { 'h', 't', 't', 'p', 's', ':', '/', '/', 'e', 'x', '.', 'c', 'o', 'm', '\0', '/', 'x', '\0' };
            ASSERT_TRUE( CORE::IsOpenableBrowserUrl( CORE::CString( withNul ) ) );

            // characters that leave a command line, a script or a markup
            const char* dangerous[] =
            {
                "https://example.com/\"", "https://example.com/'", "https://example.com/`id`", "https://example.com/$(id)", "https://example.com/${HOME}",
                "https://example.com/;rm -rf", "https://example.com/<script>", "https://example.com/>", "https://example.com/\\", "https://example.com/^",
                "https://example.com/|cat", "https://example.com/{", "https://example.com/}", "https://example.com/\" --new-window \"",
                "https://example.com/$HOME", "https://example.com/'; calc; '"
            };
            for ( UInt32 i=0; i<sizeof( dangerous ) / sizeof( dangerous[ 0 ] ); ++i )
            {
                // the ones with only characters of the allowed set are the "$(id)" and "${HOME}" style: '$' is never allowed
                ASSERT_FALSE( CORE::IsOpenableBrowserUrl( dangerous[ i ] ) );
            }
            ASSERT_TRUE( CORE::IsOpenableBrowserUrl( "https://example.com/%22%20--new-window%20%22" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A URL of exactly the longest size is openable and one byte more is not" )
        try
        {
            CORE::CString url = "https://example.com/";
            CORE::CString filler( "a" );
            while ( url.ByteSize() - 1 < CORE::BrowserLauncherMaxUrlLength )
                url += filler;
            ASSERT_TRUE( CORE::BrowserLauncherMaxUrlLength == url.ByteSize() - 1 );
            ASSERT_TRUE( CORE::IsOpenableBrowserUrl( url ) );
            url += filler;
            ASSERT_FALSE( CORE::IsOpenableBrowserUrl( url ) );

            CORE::CString veryLong = "https://example.com/";
            for ( UInt32 i=0; i<2000; ++i )
                veryLong += "0123456789";
            ASSERT_FALSE( CORE::IsOpenableBrowserUrl( veryLong ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An installed handler takes the place of the platform code and only sees URLs that passed the checks" )
        try
        {
            ASSERT_TRUE( GUCEF_NULL == CORE::GetBrowserLauncherHandler() );

            CRecordingBrowserLauncherHandler handler( true );
            ASSERT_TRUE( GUCEF_NULL == CORE::SetBrowserLauncherHandler( &handler ) );
            ASSERT_TRUE( &handler == CORE::GetBrowserLauncherHandler() );

            // a good URL reaches the handler as given and its answer is the answer
            ASSERT_TRUE( CORE::OpenUrlInSystemBrowser( "https://example.com/login?state=abc" ) );
            ASSERT_TRUE( 1 == handler.m_calls && "https://example.com/login?state=abc" == handler.m_lastUrl );
            handler.m_result = false;
            ASSERT_FALSE( CORE::OpenUrlInSystemBrowser( "http://127.0.0.1:8080/" ) );
            ASSERT_TRUE( 2 == handler.m_calls && "http://127.0.0.1:8080/" == handler.m_lastUrl );

            // a refused URL never reaches it, even when the handler would take anything
            handler.m_result = true;
            ASSERT_FALSE( CORE::OpenUrlInSystemBrowser( "file:///etc/passwd" ) );
            ASSERT_FALSE( CORE::OpenUrlInSystemBrowser( "javascript:alert(1)" ) );
            ASSERT_FALSE( CORE::OpenUrlInSystemBrowser( CORE::CString::Empty ) );
            ASSERT_FALSE( CORE::OpenUrlInSystemBrowser( "https://example.com/ \"--evil\"" ) );
            ASSERT_FALSE( CORE::OpenUrlInSystemBrowser( "https://example.com/\r\n" ) );
            ASSERT_FALSE( CORE::OpenUrlInSystemBrowser( "https://user@example.com/" ) );
            ASSERT_TRUE( 2 == handler.m_calls );

            // replacing a handler hands back the one that was there, removing it goes back to the platform code
            CRecordingBrowserLauncherHandler other( true );
            ASSERT_TRUE( &handler == CORE::SetBrowserLauncherHandler( &other ) );
            ASSERT_TRUE( CORE::OpenUrlInSystemBrowser( "https://example.org/" ) );
            ASSERT_TRUE( 2 == handler.m_calls && 1 == other.m_calls );
            ASSERT_TRUE( &other == CORE::SetBrowserLauncherHandler( GUCEF_NULL ) );
            ASSERT_TRUE( GUCEF_NULL == CORE::GetBrowserLauncherHandler() );

            // without a handler a URL that is refused is refused before any platform code is reached; nothing is launched here
            ASSERT_FALSE( CORE::OpenUrlInSystemBrowser( "ftp://example.com/" ) );
            ASSERT_FALSE( CORE::OpenUrlInSystemBrowser( "https://example.com/\n" ) );
            ASSERT_TRUE( 2 == handler.m_calls && 1 == other.m_calls );
        }
        catch( ... )
        {
            CORE::SetBrowserLauncherHandler( GUCEF_NULL );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The environment of the browser is made from the token: both variables for a token of xdg-activation, the startup id alone for an X11 id, nothing for an empty token" )
        try
        {
            CORE::CProcess::TStringMap environment;
            ASSERT_FALSE( CORE::AddActivationTokenToEnvironment( CORE::CString::Empty, CORE::ACTIVATIONTOKEN_XDG_ACTIVATION, environment ) );
            ASSERT_TRUE( environment.empty() );

            ASSERT_TRUE( CORE::AddActivationTokenToEnvironment( "wl-token", CORE::ACTIVATIONTOKEN_XDG_ACTIVATION, environment ) );
            ASSERT_TRUE( 2 == environment.size() );
            ASSERT_TRUE( "wl-token" == environment[ "XDG_ACTIVATION_TOKEN" ] && "wl-token" == environment[ "DESKTOP_STARTUP_ID" ] );

            CORE::CProcess::TStringMap x11;
            ASSERT_TRUE( CORE::AddActivationTokenToEnvironment( "prog/1/0-host_TIME99", CORE::ACTIVATIONTOKEN_STARTUP_ID, x11 ) );
            ASSERT_TRUE( 1 == x11.size() && "prog/1/0-host_TIME99" == x11[ "DESKTOP_STARTUP_ID" ] && x11.find( "XDG_ACTIVATION_TOKEN" ) == x11.end() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The provider of tokens is installed and handed back like the handler of the launcher" )
        try
        {
            ASSERT_TRUE( GUCEF_NULL == CORE::GetActivationTokenProvider() );
            CFakeActivationTokenProvider first;
            CFakeActivationTokenProvider second;
            ASSERT_TRUE( GUCEF_NULL == CORE::SetActivationTokenProvider( &first ) );
            ASSERT_TRUE( &first == CORE::GetActivationTokenProvider() );
            ASSERT_TRUE( &first == CORE::SetActivationTokenProvider( &second ) );
            ASSERT_TRUE( &second == CORE::SetActivationTokenProvider( GUCEF_NULL ) );
            ASSERT_TRUE( GUCEF_NULL == CORE::GetActivationTokenProvider() );
            ASSERT_TRUE( 0 == first.m_calls && 0 == second.m_calls );
        }
        catch( ... )
        {
            CORE::SetActivationTokenProvider( GUCEF_NULL );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) )

    GUCEF_TESTFW_TESTCASE( "Test: The helper gets the URL as its only argument and, on Linux, the token of the provider in its environment; with no provider, a provider without a token or an empty one the environment is left alone" )
        try
        {
            CRecordingHelperLauncher helper;
            ASSERT_TRUE( GUCEF_NULL == CORE::SetBrowserHelperLauncher( &helper ) );

            // no provider
            ASSERT_TRUE( CORE::OpenUrlInSystemBrowser( "https://example.com/login?state=abc" ) );
            ASSERT_TRUE( 1 == helper.m_calls && 1 == helper.m_args.size() && "https://example.com/login?state=abc" == helper.m_args[ 0 ] );
            // the browser is cut off from the terminal and the descriptors of the program
            ASSERT_TRUE( helper.m_isolated );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( "xdg-open" == helper.m_exePath );
            #endif
            ASSERT_TRUE( helper.m_environment.empty() );

            CFakeActivationTokenProvider provider;
            ASSERT_TRUE( GUCEF_NULL == CORE::SetActivationTokenProvider( &provider ) );

            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            // a token of xdg-activation: both variables, and the time the provider has is the one the launcher allows
            ASSERT_TRUE( CORE::OpenUrlInSystemBrowser( "https://example.com/a" ) );
            ASSERT_TRUE( 1 == provider.m_calls && CORE::ActivationTokenMaxWaitInMs == provider.m_lastMaxWaitInMs );
            ASSERT_TRUE( 2 == helper.m_calls && 2 == helper.m_environment.size() );
            ASSERT_TRUE( "token-1234-secret" == helper.Get( "XDG_ACTIVATION_TOKEN" ) && "token-1234-secret" == helper.Get( "DESKTOP_STARTUP_ID" ) );
            ASSERT_TRUE( 1 == helper.m_args.size() && "https://example.com/a" == helper.m_args[ 0 ] );

            // an X11 startup id: the one variable
            provider.m_kind = CORE::ACTIVATIONTOKEN_STARTUP_ID;
            provider.m_token = "redux/42/0-host_TIME777";
            ASSERT_TRUE( CORE::OpenUrlInSystemBrowser( "https://example.com/b" ) );
            ASSERT_TRUE( 3 == helper.m_calls && 1 == helper.m_environment.size() );
            ASSERT_TRUE( "redux/42/0-host_TIME777" == helper.Get( "DESKTOP_STARTUP_ID" ) && !helper.Has( "XDG_ACTIVATION_TOKEN" ) );

            // a provider without a token
            provider.m_result = false;
            ASSERT_TRUE( CORE::OpenUrlInSystemBrowser( "https://example.com/c" ) );
            ASSERT_TRUE( 4 == helper.m_calls && 3 == provider.m_calls && helper.m_environment.empty() );

            // a provider that says yes with an empty token
            provider.m_result = true;
            provider.m_token.Clear();
            ASSERT_TRUE( CORE::OpenUrlInSystemBrowser( "https://example.com/d" ) );
            ASSERT_TRUE( 5 == helper.m_calls && 4 == provider.m_calls && helper.m_environment.empty() );

            // the answer of the helper is the answer of the launch
            provider.m_token = "t";
            helper.m_result = false;
            ASSERT_FALSE( CORE::OpenUrlInSystemBrowser( "https://example.com/e" ) );
            ASSERT_TRUE( 6 == helper.m_calls );
            helper.m_result = true;

            // a handler of the launcher takes the place of all of it: the provider is not asked and the helper does not start
            CRecordingBrowserLauncherHandler handler( true );
            CORE::SetBrowserLauncherHandler( &handler );
            ASSERT_TRUE( CORE::OpenUrlInSystemBrowser( "https://example.com/f" ) );
            ASSERT_TRUE( 1 == handler.m_calls && 6 == helper.m_calls && 5 == provider.m_calls );
            CORE::SetBrowserLauncherHandler( GUCEF_NULL );

            // a URL that is refused never reaches the provider or the helper
            ASSERT_FALSE( CORE::OpenUrlInSystemBrowser( "file:///etc/passwd" ) );
            ASSERT_FALSE( CORE::OpenUrlInSystemBrowser( "https://example.com/\n" ) );
            ASSERT_TRUE( 6 == helper.m_calls && 5 == provider.m_calls );
            #endif

            CORE::SetActivationTokenProvider( GUCEF_NULL );
            ASSERT_TRUE( &helper == CORE::SetBrowserHelperLauncher( GUCEF_NULL ) );

            // without a provider again nothing is added
            CRecordingHelperLauncher again;
            CORE::SetBrowserHelperLauncher( &again );
            ASSERT_TRUE( CORE::OpenUrlInSystemBrowser( "https://example.com/g" ) );
            ASSERT_TRUE( 1 == again.m_calls && again.m_environment.empty() );
            CORE::SetBrowserHelperLauncher( GUCEF_NULL );
        }
        catch( ... )
        {
            CORE::SetBrowserLauncherHandler( GUCEF_NULL );
            CORE::SetActivationTokenProvider( GUCEF_NULL );
            CORE::SetBrowserHelperLauncher( GUCEF_NULL );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The activation token and the URL are not written to the log" )
        try
        {
            CORE::CLogStreamScope::FlushLogs();
            CCaptureLogger capture;
            CORE::CLogManager& logManager = CORE::CCoreGlobal::Instance()->GetLogManager();
            logManager.AddLogger( &capture );

            CRecordingHelperLauncher helper;
            CORE::SetBrowserHelperLauncher( &helper );
            CFakeActivationTokenProvider provider;
            provider.m_token = "SECRET-ACTIVATION-TOKEN-5f1c";
            CORE::SetActivationTokenProvider( &provider );

            ASSERT_TRUE( CORE::OpenUrlInSystemBrowser( "https://example.com/login?state=STATE-VALUE-77" ) );
            provider.m_result = false;
            ASSERT_TRUE( CORE::OpenUrlInSystemBrowser( "https://example.com/login?state=STATE-VALUE-77" ) );
            CORE::CLogStreamScope::FlushLogs();

            CORE::SetActivationTokenProvider( GUCEF_NULL );
            CORE::SetBrowserHelperLauncher( GUCEF_NULL );
            logManager.RemoveLogger( &capture );

            // the log is seen, and there is a line about the token that does not hold it
            ASSERT_TRUE( capture.m_text.HasSubstr( "an activation token goes with the browser" ) >= 0 );
            ASSERT_TRUE( capture.m_text.HasSubstr( "SECRET-ACTIVATION-TOKEN-5f1c" ) < 0 );
            ASSERT_TRUE( capture.m_text.HasSubstr( "STATE-VALUE-77" ) < 0 );
        }
        catch( ... )
        {
            CORE::SetActivationTokenProvider( GUCEF_NULL );
            CORE::SetBrowserHelperLauncher( GUCEF_NULL );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A process that is isolated from this one has /dev/null for its standard streams, a session of its own and none of the descriptors of this process" )
        try
        {
            // descriptors that are not close on exec, the way a socket of a program that forgot to set it is: a pipe, a file and a socket pair
            int pipeEnds[ 2 ] = { -1, -1 };
            int socketEnds[ 2 ] = { -1, -1 };
            ASSERT_TRUE( 0 == ::pipe( pipeEnds ) );
            ASSERT_TRUE( 0 == ::socketpair( AF_UNIX, SOCK_STREAM, 0, socketEnds ) );
            const int fileDescriptor = ::open( "/dev/null", O_RDONLY );
            ASSERT_TRUE( fileDescriptor >= 0 );
            const int leaked[] = { pipeEnds[ 0 ], pipeEnds[ 1 ], socketEnds[ 0 ], socketEnds[ 1 ], fileDescriptor };

            // the child looks at its own descriptors and writes what it found to a file (its standard output is what is being tested)
            const CORE::CString report = CORE::CombinePath( CORE::TempDir(), "gucef-process-isolation-report.txt" );
            ::unlink( report.C_String() );
            const char* script =
                "r=\"$$\"; out=\"$1\"; shift; "
                "std=\"$(readlink /proc/$$/fd/0)|$(readlink /proc/$$/fd/1)|$(readlink /proc/$$/fd/2)\"; "
                "sid=\"$(cut -d' ' -f6 /proc/$$/stat)\"; "
                "open=\"\"; for n in \"$@\"; do if [ -e /proc/$$/fd/$n ]; then open=\"$open $n\"; fi; done; "
                "printf '%s' \"$std;$sid;$r;$open\" > \"$out\"";

            for ( int pass=0; pass<2; ++pass )
            {
                const bool isolate = 1 == pass;
                CORE::CProcess::TStringVector args;
                args.push_back( "-c" );
                args.push_back( script );
                args.push_back( "sh" );
                args.push_back( report );
                for ( size_t i=0; i<sizeof( leaked ) / sizeof( leaked[ 0 ] ); ++i )
                    args.push_back( CORE::Int32ToString( leaked[ i ] ) );

                CORE::CProcess process;
                process.SetIsolateFromParent( isolate );
                ASSERT_TRUE( isolate == process.GetIsolateFromParent() );
                ASSERT_TRUE( process.Launch( "/bin/sh", args, CORE::CString(), CORE::CProcess::TStringMap(), true ) );
                Int32 exitCode = -1;
                ASSERT_TRUE( process.WaitForExit( 10000, exitCode ) && 0 == exitCode );

                CORE::CString text;
                FILE* file = ::fopen( report.C_String(), "r" );
                ASSERT_TRUE( GUCEF_NULL != file );
                if ( GUCEF_NULL == file )
                    break;
                char buffer[ 512 ];
                const size_t got = ::fread( buffer, 1, sizeof( buffer ) - 1, file );
                ::fclose( file );
                buffer[ got ] = '\0';
                text = buffer;
                ::unlink( report.C_String() );

                // standard;session;pid;open leaked descriptors
                const CORE::CString::StringVector parts = text.Trim( true ).Trim( false ).ParseElements( ';', false );
                ASSERT_TRUE( 4 == parts.size() || ( 3 == parts.size() && isolate ) );
                const CORE::CString standard = parts.size() > 0 ? parts[ 0 ] : CORE::CString();
                const CORE::CString open = parts.size() > 3 ? parts[ 3 ].Trim( true ).Trim( false ) : CORE::CString();
                if ( isolate )
                {
                    ASSERT_TRUE( "/dev/null|/dev/null|/dev/null" == standard );
                    ASSERT_TRUE( open.IsNULLOrEmpty() );
                    // a session of its own: the child is the leader of it
                    ASSERT_TRUE( parts.size() > 2 && parts[ 1 ] == parts[ 2 ] );
                }
                else
                {
                    // the control: what the test looks for is there when nobody closes it
                    ASSERT_TRUE( !open.IsNULLOrEmpty() && open.HasSubstr( CORE::Int32ToString( leaked[ 0 ] ) ) >= 0 );
                }
            }
            for ( size_t i=0; i<sizeof( leaked ) / sizeof( leaked[ 0 ] ); ++i )
                ::close( leaked[ i ] );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #endif /* Linux or macOS ? */

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL BrowserLauncher TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
