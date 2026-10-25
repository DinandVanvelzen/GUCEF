/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
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

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "gucefGUI_CImageFrame.h"

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

const CORE::CEvent CImageFrame::ImageChangedEvent = "GUCEF::GUI::CImageFrame::ImageChangedEvent";
const CORE::CEvent CImageFrame::AnimationEndedEvent = "GUCEF::GUI::CImageFrame::AnimationEndedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CImageFrame::RegisterEvents( void )
{GUCEF_TRACE;

    ImageChangedEvent.Initialize();
    AnimationEndedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CImageFrame::CImageFrame( void )
    : CWidget( "ImageFrame" )
    , m_ownImage()
    , m_animationImages()
    , m_animationDurations()
    , m_animationLoop( false )
    , m_animationPlaying( false )
    , m_animationFrame( 0 )
    , m_animationTime( 0 )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CImageFrame::CImageFrame( const CString& widgetTypeName )
    : CWidget( widgetTypeName )
    , m_ownImage()
    , m_animationImages()
    , m_animationDurations()
    , m_animationLoop( false )
    , m_animationPlaying( false )
    , m_animationFrame( 0 )
    , m_animationTime( 0 )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CImageFrame::~CImageFrame()
{GUCEF_TRACE;

    if ( !m_ownImage.IsNULLOrEmpty() && GUCEF_NULL != GetContext() )
        GetContext()->UnregisterImage( m_ownImage );
}

/*-------------------------------------------------------------------------*/

CWidget*
CImageFrame::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CImageFrame();
}

/*-------------------------------------------------------------------------*/

const CString&
CImageFrame::GetOwnImageName( void )
{GUCEF_TRACE;

    if ( m_ownImage.IsNULLOrEmpty() && GUCEF_NULL != GetContext() )
        m_ownImage = GetContext()->MakeUniqueImageAlias( "#ImageFrame" );
    return m_ownImage;
}

/*-------------------------------------------------------------------------*/

bool
CImageFrame::SetImage( const IMAGE::CImage& srcImage )
{GUCEF_TRACE;

    if ( 0 == srcImage.GetFrameCount() || srcImage.GetFrame( 0 ).empty() )
        return false;
    // a copy: the image belongs to the caller
    IMAGE::TPixelMapPtr pixels( GUCEF_NEW IMAGE::CPixelMap( *srcImage.GetFrame( 0 )[ 0 ] ) );
    return SetImage( pixels );
}

/*-------------------------------------------------------------------------*/

bool
CImageFrame::SetImage( const IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    CGUIContext* context = GetContext();
    if ( GUCEF_NULL == context || pixels.IsNULL() )
        return false;
    const CString& name = GetOwnImageName();
    if ( !context->RegisterImage( name, pixels ) )
        return false;
    if ( GetProperty( EWidgetProperty_Image ) == name )
    {
        // the same name, other pixels: the handler has to look again
        Touch( EWidgetProperty_Image );
        NotifyObservers( ImageChangedEvent );
        if ( !IsPropertySet( EWidgetProperty_Size ) )
            context->InvalidateLayout();
        return true;
    }
    return SetProperty( EWidgetProperty_Image, name );
}

/*-------------------------------------------------------------------------*/

bool
CImageFrame::SetImageName( const CString& imageName )
{GUCEF_TRACE;

    return SetProperty( EWidgetProperty_Image, imageName );
}

/*-------------------------------------------------------------------------*/

CString
CImageFrame::GetImageName( void ) const
{GUCEF_TRACE;

    return GetProperty( EWidgetProperty_Image );
}

/*-------------------------------------------------------------------------*/

bool
CImageFrame::UpdateImage( void )
{GUCEF_TRACE;

    if ( m_ownImage.IsNULLOrEmpty() || GUCEF_NULL == GetContext() )
        return false;
    return GetContext()->UpdateImage( m_ownImage );
}

/*-------------------------------------------------------------------------*/

bool
CImageFrame::SetAnimation( const TStringVector& images, const TUInt32Vector& durationsInMs, bool loop )
{GUCEF_TRACE;

    if ( images.empty() || images.size() != durationsInMs.size() )
        return false;
    m_animationImages = images;
    m_animationDurations = durationsInMs;
    m_animationLoop = loop;
    m_animationFrame = 0;
    m_animationTime = 0;
    m_animationPlaying = true;
    return SetProperty( EWidgetProperty_Image, images[ 0 ] );
}

/*-------------------------------------------------------------------------*/

void
CImageFrame::PlayAnimation( void )
{GUCEF_TRACE;

    m_animationPlaying = !m_animationImages.empty();
}

/*-------------------------------------------------------------------------*/

void
CImageFrame::StopAnimation( void )
{GUCEF_TRACE;

    m_animationPlaying = false;
}

/*-------------------------------------------------------------------------*/

bool
CImageFrame::IsAnimationPlaying( void ) const
{GUCEF_TRACE;

    return m_animationPlaying;
}

/*-------------------------------------------------------------------------*/

UInt32
CImageFrame::GetAnimationFrame( void ) const
{GUCEF_TRACE;

    return m_animationFrame;
}

/*-------------------------------------------------------------------------*/

bool
CImageFrame::NeedsUpdate( void ) const
{GUCEF_TRACE;

    return m_animationPlaying;
}

/*-------------------------------------------------------------------------*/

void
CImageFrame::OnUpdate( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( !m_animationPlaying || m_animationImages.empty() )
        return;

    m_animationTime += deltaInMs;
    UInt32 frame = m_animationFrame;
    while ( m_animationTime >= m_animationDurations[ frame ] )
    {
        UInt32 duration = m_animationDurations[ frame ];
        if ( 0 == duration )
            break;
        m_animationTime -= duration;
        if ( frame + 1 < (UInt32) m_animationImages.size() )
        {
            ++frame;
        }
        else
        if ( m_animationLoop )
        {
            frame = 0;
        }
        else
        {
            m_animationPlaying = false;
            m_animationTime = 0;
            break;
        }
    }
    if ( frame != m_animationFrame )
    {
        m_animationFrame = frame;
        SetProperty( EWidgetProperty_Image, m_animationImages[ frame ] );
    }
    if ( !m_animationPlaying )
        NotifyObservers( AnimationEndedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CImageFrame::GetNaturalSize( Int32& width, Int32& height ) const
{GUCEF_TRACE;

    width = height = 0;
    CRect subRect = GetRectProperty( EWidgetProperty_SubRect );
    if ( !subRect.IsEmpty() )
    {
        width = subRect.width;
        height = subRect.height;
        return true;
    }
    CGUIContext* context = GetContext();
    return GUCEF_NULL != context && context->GetImageLogicalSize( GetProperty( EWidgetProperty_Image ), width, height );
}

/*-------------------------------------------------------------------------*/

void
CImageFrame::OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

    if ( property == EWidgetProperty_Image )
    {
        NotifyObservers( ImageChangedEvent );
        if ( !IsPropertySet( EWidgetProperty_Size ) && GUCEF_NULL != GetContext() )
            GetContext()->InvalidateLayout();
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CImageFrame::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CImageFrame";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
