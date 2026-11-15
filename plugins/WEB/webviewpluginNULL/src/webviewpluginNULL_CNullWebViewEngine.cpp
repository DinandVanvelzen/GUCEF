/*
 *  webviewpluginNULL: gucefWEB plugin with the web view engine "Null"
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

#include "webviewpluginNULL_CNullWebView.h"
#include "webviewpluginNULL_CNullWebViewEngine.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEBVIEWPLUGINNULL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CNullWebViewEngine::EngineName = "Null";

/*-------------------------------------------------------------------------*/

CNullWebViewEngine::CNullWebViewEngine( void )
    : WEB::CWebViewEngineBase( "Null" )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CNullWebViewEngine::~CNullWebViewEngine()
{GUCEF_TRACE;

    ShutdownEngine();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CNullWebViewEngine::GetEngineVersion( void ) const
{GUCEF_TRACE;

    return "Null 0.1 (blank pages, no network)";
}

/*-------------------------------------------------------------------------*/

bool
CNullWebViewEngine::IsAvailable( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CNullWebViewEngine::GetAvailabilityDescription( void ) const
{GUCEF_TRACE;

    return "always available: it renders blank pages and loads nothing";
}

/*-------------------------------------------------------------------------*/

WEB::CWebViewCapabilities
CNullWebViewEngine::GetCapabilities( void ) const
{GUCEF_TRACE;

    return WEB::CWebViewCapabilities( WEB::CWebViewCapabilities::CAP_OFFSCREEN_FRAMES |
                                          WEB::CWebViewCapabilities::CAP_INJECT_POINTER |
                                          WEB::CWebViewCapabilities::CAP_INJECT_KEYBOARD |
                                          WEB::CWebViewCapabilities::CAP_INJECT_TEXT |
                                          WEB::CWebViewCapabilities::CAP_TRANSPARENT_BACKGROUND |
                                          WEB::CWebViewCapabilities::CAP_HEADLESS );
}

/*-------------------------------------------------------------------------*/

WEB::CWebViewBase*
CNullWebViewEngine::CreateViewOnEngineThread( const WEB::CWebViewSettings& settings, UInt32 viewId, CORE::CString& problem )
{GUCEF_TRACE;

    return GUCEF_NEW CNullWebView( *this, viewId, settings, GetCapabilities() );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CNullWebViewEngine::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::WEBVIEWPLUGINNULL::CNullWebViewEngine";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEBVIEWPLUGINNULL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
