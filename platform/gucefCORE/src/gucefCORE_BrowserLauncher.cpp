/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include <string.h>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

  #include <windows.h>
  #include <shellapi.h>

#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

  #include <emscripten.h>

#endif

#include "gucefCORE_BrowserLauncher.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

/** How long a launched helper (xdg-open, open) is waited for: long enough to see it fail, short enough not to stall the caller */
#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) )
static const UInt32 HelperWaitTimeInMs = 500;
#endif

static MT::CMutex g_handlerLock;
static CIBrowserLauncherHandler* g_handler = GUCEF_NULL;

/** Held while the provider is called, so a provider that is taken away is not in use any more when SetActivationTokenProvider() returns */
static MT::CMutex g_tokenProviderLock;
static CIActivationTokenProvider* g_tokenProvider = GUCEF_NULL;

static MT::CMutex g_helperLauncherLock;
static CIBrowserHelperLauncher* g_helperLauncher = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIBrowserLauncherHandler::CIBrowserLauncherHandler( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIBrowserLauncherHandler::~CIBrowserLauncherHandler()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIBrowserLauncherHandler*
SetBrowserLauncherHandler( CIBrowserLauncherHandler* handler )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_handlerLock );
    CIBrowserLauncherHandler* previous = g_handler;
    g_handler = handler;
    return previous;
}

/*-------------------------------------------------------------------------*/

CIBrowserLauncherHandler*
GetBrowserLauncherHandler( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_handlerLock );
    return g_handler;
}

/*-------------------------------------------------------------------------*/

CIActivationTokenProvider::CIActivationTokenProvider( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIActivationTokenProvider::~CIActivationTokenProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIBrowserHelperLauncher::CIBrowserHelperLauncher( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIBrowserHelperLauncher::~CIBrowserHelperLauncher()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIActivationTokenProvider*
SetActivationTokenProvider( CIActivationTokenProvider* provider )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_tokenProviderLock );
    CIActivationTokenProvider* previous = g_tokenProvider;
    g_tokenProvider = provider;
    return previous;
}

/*-------------------------------------------------------------------------*/

CIActivationTokenProvider*
GetActivationTokenProvider( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_tokenProviderLock );
    return g_tokenProvider;
}

/*-------------------------------------------------------------------------*/

CIBrowserHelperLauncher*
SetBrowserHelperLauncher( CIBrowserHelperLauncher* launcher )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_helperLauncherLock );
    CIBrowserHelperLauncher* previous = g_helperLauncher;
    g_helperLauncher = launcher;
    return previous;
}

/*-------------------------------------------------------------------------*/

bool
AddActivationTokenToEnvironment( const CString& token, TActivationTokenKind kind, CProcess::TStringMap& envOverrides )
{GUCEF_TRACE;

    if ( token.IsNULLOrEmpty() )
        return false;
    if ( ACTIVATIONTOKEN_XDG_ACTIVATION == kind )
        envOverrides[ ActivationTokenEnvironmentVariable ] = token;
    envOverrides[ StartupIdEnvironmentVariable ] = token;
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
IsUrlCharacterAllowed( unsigned char c )
{
    if ( ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) )
        return true;
    switch ( c )
    {
        case '-' : case '.' : case '_' : case '~' : case ':' : case '/' : case '?' : case '#' :
        case '[' : case ']' : case '@' : case '!' : case '&' : case '(' : case ')' : case '*' :
        case '+' : case ',' : case ';' : case '=' : case '%' :
            return true;
        default :
            return false;
    }
}

/*-------------------------------------------------------------------------*/

static bool
HasSchemePrefix( const unsigned char* p, UInt32 length, const char* scheme, UInt32& schemeLength )
{
    schemeLength = (UInt32) strlen( scheme );
    if ( length < schemeLength )
        return false;
    for ( UInt32 i=0; i<schemeLength; ++i )
    {
        unsigned char c = p[ i ];
        if ( c >= 'A' && c <= 'Z' )
            c = (unsigned char) ( c - 'A' + 'a' );
        if ( c != (unsigned char) scheme[ i ] )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
IsOpenableBrowserUrl( const CString& url )
{GUCEF_TRACE;

    if ( url.IsNULLOrEmpty() )
        return false;

    const UInt32 length = url.ByteSize() - 1;
    if ( 0 == length || length > BrowserLauncherMaxUrlLength )
        return false;

    const unsigned char* p = reinterpret_cast< const unsigned char* >( url.C_String() );

    // the whole text first: a control character or a space anywhere is a refusal, whatever comes before it
    for ( UInt32 i=0; i<length; ++i )
    {
        if ( !IsUrlCharacterAllowed( p[ i ] ) )
            return false;
    }

    UInt32 schemeLength = 0;
    if ( !HasSchemePrefix( p, length, "https://", schemeLength ) && !HasSchemePrefix( p, length, "http://", schemeLength ) )
        return false;

    // the authority runs up to the first '/', '?' or '#': it needs a host, and no user information
    UInt32 authorityEnd = schemeLength;
    while ( authorityEnd < length && '/' != p[ authorityEnd ] && '?' != p[ authorityEnd ] && '#' != p[ authorityEnd ] )
    {
        if ( '@' == p[ authorityEnd ] )
            return false;
        ++authorityEnd;
    }
    if ( authorityEnd == schemeLength )
        return false;

    // a host starts with a letter, a digit or the '[' of an IPv6 address; ':' would be an empty host with a port
    unsigned char first = p[ schemeLength ];
    bool validFirst = ( first >= 'A' && first <= 'Z' ) || ( first >= 'a' && first <= 'z' ) || ( first >= '0' && first <= '9' ) || '[' == first;
    return validFirst;
}

/*-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

static bool
OpenWithPlatform( const CString& url )
{GUCEF_TRACE;

    // the URL is plain ASCII at this point
    GUCEF::vector< wchar_t > wideUrl;
    wideUrl.reserve( url.ByteSize() );
    const char* p = url.C_String();
    for ( UInt32 i=0; i<url.ByteSize()-1; ++i )
        wideUrl.push_back( (wchar_t) (unsigned char) p[ i ] );
    wideUrl.push_back( L'\0' );

    // The program that was started may take the foreground from this one, which got the last input; a cheap fallback for a
    // default browser that would else open behind the window (unverified on Windows)
    ::AllowSetForegroundWindow( ASFW_ANY );

    HINSTANCE result = ::ShellExecuteW( GUCEF_NULL, L"open", &wideUrl[ 0 ], GUCEF_NULL, GUCEF_NULL, SW_SHOWNORMAL );
    if ( (INT_PTR) result <= 32 )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "OpenUrlInSystemBrowser: ShellExecute failed with code " << (Int32) (INT_PTR) result );
        return false;
    }
    return true;
}

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) )

static bool
OpenWithPlatform( const CString& url )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    const CString helper = "xdg-open";
    #else
    const CString helper = "open";
    #endif

    // an argument vector, the URL is one argument and no shell sees it
    CProcess::TStringVector args;
    args.push_back( url );

    // the proof of a recent user action, so that the window of the browser is let to the front; the token is not logged
    CProcess::TStringMap environment;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    {
        MT::CScopeMutex providerLock( g_tokenProviderLock );
        if ( GUCEF_NULL != g_tokenProvider )
        {
            CString token;
            TActivationTokenKind kind = ACTIVATIONTOKEN_XDG_ACTIVATION;
            if ( g_tokenProvider->GetActivationToken( token, kind, ActivationTokenMaxWaitInMs ) && AddActivationTokenToEnvironment( token, kind, environment ) )
            {
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "OpenUrlInSystemBrowser: an activation token goes with the browser" );
            }
            else
            {
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "OpenUrlInSystemBrowser: no activation token, the browser may open behind the window" );
            }
            token.Clear();
        }
    }
    #endif

    CIBrowserHelperLauncher* helperLauncher = GUCEF_NULL;
    {
        MT::CScopeMutex lock( g_helperLauncherLock );
        helperLauncher = g_helperLauncher;
    }
    if ( GUCEF_NULL != helperLauncher )
        return helperLauncher->LaunchHelper( helper, args, environment, true );

    // the browser outlives the program and must not write to its terminal (a browser prints its own start up complaints to the
    // console it inherits) or hold on to its sockets: /dev/null for the standard streams, a session of its own, no other descriptors
    CProcess process;
    process.SetIsolateFromParent( true );
    if ( !process.Launch( helper, args, CString(), environment, true ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "OpenUrlInSystemBrowser: Unable to start \"" << helper << "\"" );
        return false;
    }

    // the helper hands the URL to the browser and ends; one that is still busy is on its way to opening a browser
    Int32 exitCode = 0;
    if ( process.WaitForExit( HelperWaitTimeInMs, exitCode ) && 0 != exitCode )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "OpenUrlInSystemBrowser: \"" << helper << "\" ended with exit code " << exitCode );
        return false;
    }
    return true;
}

#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

static bool
OpenWithPlatform( const CString& url )
{GUCEF_TRACE;

    EM_ASM(
    {
        window.open( UTF8ToString( $0 ), "_blank", "noopener,noreferrer" );
    }, url.C_String() );
    return true;
}

#else

static bool
OpenWithPlatform( const CString& )
{GUCEF_TRACE;

    // Android: the Android plugin installs a handler that fires the Intent
    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "OpenUrlInSystemBrowser: This platform has no browser launcher in gucefCORE and no handler is installed" );
    return false;
}

#endif

/*-------------------------------------------------------------------------*/

bool
OpenUrlInSystemBrowser( const CString& url )
{GUCEF_TRACE;

    if ( !IsOpenableBrowserUrl( url ) )
    {
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "OpenUrlInSystemBrowser: Refused a URL that is not a plain http or https URL" );
        return false;
    }

    CIBrowserLauncherHandler* handler = GetBrowserLauncherHandler();
    if ( GUCEF_NULL != handler )
        return handler->OpenUrl( url );

    return OpenWithPlatform( url );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
