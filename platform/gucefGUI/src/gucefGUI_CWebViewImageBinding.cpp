/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#include <math.h>

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "gucefGUI_CWebViewImageBinding.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CWebViewImageBinding::ImageUpdatedEvent = "GUCEF::GUI::CWebViewImageBinding::ImageUpdatedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CWebViewImageBinding::RegisterEvents( void )
{GUCEF_TRACE;

    ImageUpdatedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CWebViewImageBinding::CWebViewImageBinding( void )
    : CORE::CObservingNotifier()
    , m_context( GUCEF_NULL )
    , m_alias()
    , m_view()
    , m_pixels()
    , m_density( 1 )
    , m_hasImage( false )
    , m_autoUpdate( false )
    , m_info()
    , m_frames( 0 )
    , m_copiedPixels( 0 )
    , m_registrations( 0 )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CWebViewImageBinding::~CWebViewImageBinding()
{GUCEF_TRACE;

    Unbind();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewImageBinding::Bind( CGUIContext* context, const CString& imageAlias, const WEB::TWebViewPtr& view, bool autoUpdate )
{GUCEF_TRACE;

    Unbind();
    if ( GUCEF_NULL == context || imageAlias.IsNULLOrEmpty() || view.IsNULL() )
        return false;
    m_context = context;
    m_alias = imageAlias;
    m_view = view;
    m_autoUpdate = autoUpdate;
    m_info = WEB::CWebViewFrameInfo();
    if ( m_pixels.IsNULL() )
        m_pixels = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 1, 1, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    if ( m_autoUpdate )
    {
        TEventCallback callback( this, &CWebViewImageBinding::OnFrameStarted );
        SubscribeTo( m_context, CGUIContext::FrameStartedEvent, callback );
    }
    // a frame that is there already is shown at once
    Update();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebViewImageBinding::Unbind( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_context )
    {
        if ( m_autoUpdate )
            UnsubscribeFrom( m_context, CGUIContext::FrameStartedEvent );
        if ( m_hasImage )
            m_context->UnregisterImage( m_alias );
    }
    m_context = GUCEF_NULL;
    m_view.Unlink();
    m_hasImage = false;
    m_autoUpdate = false;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewImageBinding::Update( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_context || m_view.IsNULL() || m_pixels.IsNULL() )
        return false;
    WEB::CWebViewFrameExchange& frames = m_view->GetFrameExchange();
    if ( !frames.HasNewFrame() )
        return false;

    const UInt32 oldWidth = m_pixels->GetWidthInPixels();
    const UInt32 oldHeight = m_pixels->GetHeightInPixels();
    WEB::CWebViewRect changed;
    if ( !frames.TakeLatestInto( *m_pixels, m_info, changed ) )
        return false;

    ++m_frames;
    m_copiedPixels += changed.GetArea();
    const Float32 scale = m_info.deviceScale > 0.0f ? m_info.deviceScale : 1.0f;
    UInt32 density = (UInt32) ceil( scale - 0.001f );
    if ( 0 == density )
        density = 1;
    const CRect region( changed.left, changed.top, changed.width, changed.height );
    if ( !m_hasImage || density != m_density || oldWidth != m_pixels->GetWidthInPixels() || oldHeight != m_pixels->GetHeightInPixels() )
    {
        // a new size or density: registered again, the layout learns the new size
        m_density = density;
        m_hasImage = m_context->RegisterImage( m_alias, m_pixels, m_density );
        ++m_registrations;
    }
    else
    {
        m_context->UpdateImageRegion( m_alias, region );
    }
    TRegionEventData eventData( region );
    NotifyObservers( ImageUpdatedEvent, &eventData );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebViewImageBinding::OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Update();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewImageBinding::IsBound( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_context && !m_view.IsNULL();
}

/*-------------------------------------------------------------------------*/

WEB::TWebViewPtr
CWebViewImageBinding::GetView( void ) const
{GUCEF_TRACE;

    return m_view;
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewImageBinding::GetImageAlias( void ) const
{GUCEF_TRACE;

    return m_alias;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewImageBinding::HasImage( void ) const
{GUCEF_TRACE;

    return m_hasImage;
}

/*-------------------------------------------------------------------------*/

void
CWebViewImageBinding::GetImageSize( UInt32& width, UInt32& height, UInt32& density ) const
{GUCEF_TRACE;

    width = m_hasImage ? m_pixels->GetWidthInPixels() : 0;
    height = m_hasImage ? m_pixels->GetHeightInPixels() : 0;
    density = m_density;
}

/*-------------------------------------------------------------------------*/

const WEB::CWebViewFrameInfo&
CWebViewImageBinding::GetFrameInfo( void ) const
{GUCEF_TRACE;

    return m_info;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewImageBinding::GetFrameCount( void ) const
{GUCEF_TRACE;

    return m_frames;
}

/*-------------------------------------------------------------------------*/

UInt64
CWebViewImageBinding::GetCopiedPixelCount( void ) const
{GUCEF_TRACE;

    return m_copiedPixels;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewImageBinding::GetRegistrationCount( void ) const
{GUCEF_TRACE;

    return m_registrations;
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewImageBinding::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CWebViewImageBinding";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
