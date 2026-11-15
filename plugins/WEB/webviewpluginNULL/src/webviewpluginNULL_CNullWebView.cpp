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

#include <string.h>

#ifndef GUCEF_WEB_CWEBVIEWSETTINGS_H
#include "gucefWEB_CWebViewSettings.h"
#define GUCEF_WEB_CWEBVIEWSETTINGS_H
#endif /* GUCEF_WEB_CWEBVIEWSETTINGS_H ? */

#include "webviewpluginNULL_CNullWebView.h"

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

CNullWebView::CNullWebView( WEB::CWebViewEngineBase& engine, UInt32 viewId, const WEB::CWebViewSettings& settings, const WEB::CWebViewCapabilities& capabilities )
    : WEB::CWebViewBase( engine, viewId, settings, capabilities )
    , m_page( GUCEF_NULL, 1, 1, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 )
    , m_visible( true )
    , m_inputCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CNullWebView::~CNullWebView()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CNullWebView::GetInputCount( void ) const
{GUCEF_TRACE;

    return m_inputCount;
}

/*-------------------------------------------------------------------------*/

void
CNullWebView::RenderBlank( void )
{GUCEF_TRACE;

    UInt32 width = 0, height = 0;
    Float32 scale = 1.0f;
    GetSize( width, height, scale );
    UInt32 deviceWidth = 0, deviceHeight = 0;
    WEB::CWebViewSettings::ToDeviceSize( width, height, scale, deviceWidth, deviceHeight );
    if ( m_page.GetWidthInPixels() != deviceWidth || m_page.GetHeightInPixels() != deviceHeight )
        m_page.Assign( GUCEF_NULL, deviceWidth, deviceHeight, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 );
    // white, or nothing at all for a transparent background
    memset( m_page.GetDataPtr(), GetSettings().transparentBackground ? 0 : 255, (size_t) deviceWidth * deviceHeight * 4 );
    if ( m_visible )
        PublishFrame( m_page, WEB::CWebViewRect::Whole( deviceWidth, deviceHeight ) );
}

/*-------------------------------------------------------------------------*/

void
CNullWebView::OnCreated( void )
{GUCEF_TRACE;

    RenderBlank();
}

/*-------------------------------------------------------------------------*/

void
CNullWebView::OnNavigate( const CORE::CString& url )
{GUCEF_TRACE;

    if ( !url.Equals( "about:blank", false ) )
    {
        NotifyLoadFailed( url, WEB::WEBVIEWERROR_NOT_SUPPORTED, "The engine \"Null\" loads no pages" );
        return;
    }
    NotifyLoadStarted( url );
    NotifyLoadCommitted( url );
    NotifyTitleChanged( CORE::CString::Empty );
    RenderBlank();
    NotifyLoadFinished( url );
}

/*-------------------------------------------------------------------------*/

bool
CNullWebView::OnReload( void )
{GUCEF_TRACE;

    RenderBlank();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNullWebView::OnStop( void )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNullWebView::OnGoBack( void )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CNullWebView::OnGoForward( void )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
CNullWebView::OnEvaluateScript( const CORE::CString& script, const CORE::CString& world, WEB::CWebViewCommand& command )
{GUCEF_TRACE;

    // not reached: the engine does not report script results, the base refuses scripts already
    FailCommand( command, WEB::WEBVIEWERROR_NOT_SUPPORTED, "The engine \"Null\" evaluates no scripts" );
}

/*-------------------------------------------------------------------------*/

void
CNullWebView::OnGetCookies( const CORE::CString& urlFilter, WEB::CWebViewCommand& command )
{GUCEF_TRACE;

    CompleteCookies( command, WEB::TWebViewCookieVector() );
}

/*-------------------------------------------------------------------------*/

void
CNullWebView::OnClearCookies( WEB::CWebViewCommand& command )
{GUCEF_TRACE;

    CompleteCommand( command );
}

/*-------------------------------------------------------------------------*/

void
CNullWebView::OnSetCookie( const WEB::CWebViewCookie& cookie, WEB::CWebViewCommand& command )
{GUCEF_TRACE;

    FailCommand( command, WEB::WEBVIEWERROR_NOT_SUPPORTED, "The engine \"Null\" keeps no cookies" );
}

/*-------------------------------------------------------------------------*/

void
CNullWebView::OnSetSize( UInt32 logicalWidth, UInt32 logicalHeight, Float32 deviceScale )
{GUCEF_TRACE;

    RenderBlank();
}

/*-------------------------------------------------------------------------*/

void
CNullWebView::OnSetVisible( bool visible )
{GUCEF_TRACE;

    const bool wasVisible = m_visible;
    m_visible = visible;
    if ( visible && !wasVisible )
        RenderBlank();
}

/*-------------------------------------------------------------------------*/

void
CNullWebView::OnSetFocus( bool focused )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CNullWebView::OnInjectInput( const WEB::CWebViewInputEvent& inputEvent )
{GUCEF_TRACE;

    ++m_inputCount;
}

/*-------------------------------------------------------------------------*/

bool
CNullWebView::OnSetZoom( Float32 factor )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
CNullWebView::OnDecisionResolved( UInt32 decisionId, WEB::TWebViewDecisionKind kind, bool allow, const CORE::CString& text, bool timedOut )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CNullWebView::OnEditCommand( WEB::TWebViewEditCommand command, CORE::CString& copiedText )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
CNullWebView::OnClose( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CNullWebView::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::WEBVIEWPLUGINNULL::CNullWebView";
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
