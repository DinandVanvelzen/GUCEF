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

#include "TestFakeWebView.h"
#include "TestFakeWebViewEngine.h"

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

const CORE::CString CFakeWebViewEngine::EngineName = "Fake";

/*-------------------------------------------------------------------------*/

CFakeWebViewEngine::CFakeWebViewEngine( const CORE::CString& engineName )
    : WEB::CWebViewEngineBase( engineName )
    , m_site()
    , m_cookieJars()
    , m_sessionNumber( 0 )
    , m_available( 1 )
    , m_capabilities( GetDefaultCapabilities() )
    , m_loopMs( 1500 )
    , m_createdViews( 0 )
    , m_threadStarted( 0 )
    , m_threadId( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFakeWebViewEngine::~CFakeWebViewEngine()
{GUCEF_TRACE;

    ShutdownEngine();
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeWebViewEngine::GetDefaultCapabilities( void )
{GUCEF_TRACE;

    typedef WEB::CWebViewCapabilities C;
    return C::CAP_OFFSCREEN_FRAMES | C::CAP_INJECT_POINTER | C::CAP_INJECT_KEYBOARD | C::CAP_INJECT_TEXT | C::CAP_COOKIES |
           C::CAP_SCRIPT_RESULT | C::CAP_ISOLATED_WORLDS | C::CAP_USER_AGENT_OVERRIDE | C::CAP_PROFILE_DIRECTORY |
           C::CAP_EPHEMERAL_PROFILE | C::CAP_PROXY_SETTINGS | C::CAP_NAVIGATION_POLICY | C::CAP_DEFERRED_DECISIONS |
           C::CAP_HEADLESS | C::CAP_ZOOM | C::CAP_EDIT_COMMANDS;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeWebViewEngine::GetEngineVersion( void ) const
{GUCEF_TRACE;

    return "Fake 1.0 (the test site in memory)";
}

/*-------------------------------------------------------------------------*/

bool
CFakeWebViewEngine::IsAvailable( void ) const
{GUCEF_TRACE;

    return 0 != m_available.Load();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeWebViewEngine::GetAvailabilityDescription( void ) const
{GUCEF_TRACE;

    return IsAvailable() ? CORE::CString( "available: the test site in memory" ) : CORE::CString( "made unavailable by the test" );
}

/*-------------------------------------------------------------------------*/

WEB::CWebViewCapabilities
CFakeWebViewEngine::GetCapabilities( void ) const
{GUCEF_TRACE;

    return WEB::CWebViewCapabilities( m_capabilities.Load() );
}

/*-------------------------------------------------------------------------*/

void
CFakeWebViewEngine::SetAvailable( bool available )
{GUCEF_TRACE;

    m_available.Store( available ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

void
CFakeWebViewEngine::SetCapabilities( UInt32 capabilities )
{GUCEF_TRACE;

    m_capabilities.Store( capabilities );
}

/*-------------------------------------------------------------------------*/

CFakeWebSite&
CFakeWebViewEngine::GetSite( void )
{GUCEF_TRACE;

    return m_site;
}

/*-------------------------------------------------------------------------*/

void
CFakeWebViewEngine::SetLoopMs( UInt32 loopMs )
{GUCEF_TRACE;

    m_loopMs.Store( loopMs );
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeWebViewEngine::GetLoopMs( void ) const
{GUCEF_TRACE;

    return m_loopMs.Load();
}

/*-------------------------------------------------------------------------*/

WEB::TWebViewCookieVector&
CFakeWebViewEngine::GetCookieJar( const CORE::CString& profileId )
{GUCEF_TRACE;

    return m_cookieJars[ profileId ];
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeWebViewEngine::NextSessionNumber( void )
{GUCEF_TRACE;

    return ++m_sessionNumber;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeWebViewEngine::GetCreatedViewCount( void ) const
{GUCEF_TRACE;

    return m_createdViews.Load();
}

/*-------------------------------------------------------------------------*/

bool
CFakeWebViewEngine::WasEngineThreadStarted( void ) const
{GUCEF_TRACE;

    return 0 != m_threadStarted.Load();
}

/*-------------------------------------------------------------------------*/

TThreadId
CFakeWebViewEngine::GetStartedThreadId( void ) const
{GUCEF_TRACE;

    return m_threadId.Load();
}

/*-------------------------------------------------------------------------*/

WEB::CWebViewBase*
CFakeWebViewEngine::CreateViewOnEngineThread( const WEB::CWebViewSettings& settings, UInt32 viewId, CORE::CString& problem )
{GUCEF_TRACE;

    ++m_createdViews;
    return GUCEF_NEW CFakeWebView( *this, viewId, settings, GetCapabilities() );
}

/*-------------------------------------------------------------------------*/

bool
CFakeWebViewEngine::OnEngineThreadStart( void )
{GUCEF_TRACE;

    m_threadId.Store( MT::GetCurrentThreadID() );
    m_threadStarted.Store( 1 );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFakeWebViewEngine::OnEngineThreadStop( void )
{GUCEF_TRACE;

    m_cookieJars.clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFakeWebViewEngine::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::WEBVIEWTEST::CFakeWebViewEngine";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEBVIEWTEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
