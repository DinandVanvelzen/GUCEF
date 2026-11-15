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
#include <stdio.h>
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_IMAGE_CCHANNELOPERATIONS_H
#include "gucefIMAGE_CChannelOperations.h"
#define GUCEF_IMAGE_CCHANNELOPERATIONS_H
#endif /* GUCEF_IMAGE_CCHANNELOPERATIONS_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAPCHANNELACCESS_H
#include "gucefIMAGE_CPixelMapChannelAccess.h"
#define GUCEF_IMAGE_CPIXELMAPCHANNELACCESS_H
#endif /* GUCEF_IMAGE_CPIXELMAPCHANNELACCESS_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAPRESAMPLER_H
#include "gucefIMAGE_CPixelMapResampler.h"
#define GUCEF_IMAGE_CPIXELMAPRESAMPLER_H
#endif /* GUCEF_IMAGE_CPIXELMAPRESAMPLER_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CIMAGEFRAME_H
#include "gucefGUI_CImageFrame.h"
#define GUCEF_GUI_CIMAGEFRAME_H
#endif /* GUCEF_GUI_CIMAGEFRAME_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_GUI_CCHECKBOX_H
#include "gucefGUI_CCheckbox.h"
#define GUCEF_GUI_CCHECKBOX_H
#endif /* GUCEF_GUI_CCHECKBOX_H ? */

#include "gucefGUI_CImageViewForm.h"

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

const CORE::CEvent CImageViewForm::PixelProbedEvent = "GUCEF::GUI::CImageViewForm::PixelProbedEvent";
const CORE::CEvent CImageViewForm::ViewChangedEvent = "GUCEF::GUI::CImageViewForm::ViewChangedEvent";

namespace {

const Float32 MinZoom = 1.0f / 64.0f;
const Float32 MaxZoom = 64.0f;

/** The names of the channel buttons, by channel */
const char* const ChannelButtons[] = { "ChannelRGB", "ChannelR", "ChannelG", "ChannelB", "ChannelA" };
const char* const ChannelTexts[] = { "RGB", "R", "G", "B", "A" };

/** The widest picture that is made: a guard against a runaway size of a level or of tiling */
const UInt32 MaxShownSide = 16384;

Float32
ClampZoom( Float32 zoom )
{
    return zoom < MinZoom ? MinZoom : ( zoom > MaxZoom ? MaxZoom : zoom );
}

CString
FormatFloat( Float32 value )
{
    char buffer[ 48 ];
    sprintf( buffer, "%.4g", (double) value );
    return buffer;
}

/** The letters of the channels of a pixel storage format, in storage order */
const char*
ChannelLetters( IMAGE::TPixelStorageFormat format )
{
    switch ( format )
    {
        case IMAGE::PSF_RGB:    return "RGB";
        case IMAGE::PSF_RGBA:   return "RGBA";
        case IMAGE::PSF_BGR:    return "BGR";
        case IMAGE::PSF_BGRA:   return "BGRA";
        case IMAGE::PSF_HSV:    return "HSV";
        case IMAGE::PSF_HSVA:   return "HSVA";
        case IMAGE::PSF_HSL:    return "HSL";
        case IMAGE::PSF_HSLA:   return "HSLA";
        case IMAGE::PSF_SINGLE_CHANNEL_RED:     return "R";
        case IMAGE::PSF_SINGLE_CHANNEL_GREEN:   return "G";
        case IMAGE::PSF_SINGLE_CHANNEL_BLUE:    return "B";
        case IMAGE::PSF_SINGLE_CHANNEL_ALPHA:   return "A";
        case IMAGE::PSF_LUMINANCE_ALPHA:        return "L";
        case IMAGE::PSF_PALETTE_INDICES:        return "I";
        default:                                return "L";
    }
}

/** One value of a pixel as the data holds it: integers as integers, floating point numbers as such */
CString
RawValueText( const void* valuePtr, IMAGE::TBuildinDataType dataType )
{
    switch ( dataType )
    {
        case GUCEF::DATATYPE_UINT8:        return CORE::UInt32ToString( (UInt32) *static_cast< const UInt8* >( valuePtr ) );
        case GUCEF::DATATYPE_INT8:         return CORE::Int32ToString( (Int32) *static_cast< const Int8* >( valuePtr ) );
        case GUCEF::DATATYPE_LE_UINT16:    { UInt16 v; memcpy( &v, valuePtr, sizeof( v ) ); return CORE::UInt32ToString( (UInt32) v ); }
        case GUCEF::DATATYPE_LE_INT16:     { Int16 v; memcpy( &v, valuePtr, sizeof( v ) ); return CORE::Int32ToString( (Int32) v ); }
        case GUCEF::DATATYPE_LE_UINT32:    { UInt32 v; memcpy( &v, valuePtr, sizeof( v ) ); return CORE::UInt32ToString( v ); }
        case GUCEF::DATATYPE_LE_INT32:     { Int32 v; memcpy( &v, valuePtr, sizeof( v ) ); return CORE::Int32ToString( v ); }
        default:
            return FormatFloat( IMAGE::CPixelMapChannelAccess::ReadValue( valuePtr, dataType ) );
    }
}

UInt8
ToByte( Float32 value )
{
    if ( !( value > 0.0f ) )
        return 0;
    if ( value >= 1.0f )
        return 255;
    return (UInt8) ( value * 255.0f + 0.5f );
}

IMAGE::TPixelMapPtr
MakeRgba( UInt32 width, UInt32 height )
{
    return IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImageViewForm::SPixelProbe::SPixelProbe( void )
    : x( 0 )
    , y( 0 )
    , inSecond( false )
    , storedValue()
    , shownValue()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::RegisterEvents( void )
{GUCEF_TRACE;

    PixelProbedEvent.Initialize();
    ViewChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CImageViewForm::CImageViewForm( void )
    : CFormEx()
    , m_first()
    , m_second()
    , m_sourceSpace( IMAGE::COLOURSPACE_SRGB )
    , m_displaySpace( IMAGE::COLOURSPACE_SRGB )
    , m_channel( CHANNEL_RGB )
    , m_checkerboard( true )
    , m_level( 0 )
    , m_frame( 0 )
    , m_frameMeaning( FRAMEMEANING_FRAMES )
    , m_tiling( 1 )
    , m_split( 0.5f )
    , m_zoom( 1.0f )
    , m_panX( 0 )
    , m_panY( 0 )
    , m_fitted( true )
    , m_viewWidth( 0 )
    , m_viewHeight( 0 )
    , m_rebuilds( 0 )
    , m_panning( false )
    , m_splitting( false )
    , m_lastPointerX( 0 )
    , m_lastPointerY( 0 )
    , m_updatingWidgets( false )
    , m_frameSubscribed( false )
    , m_status()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CImageViewForm::CImageViewForm( CIGUIContext* context )
    : CFormEx( context )
    , m_first()
    , m_second()
    , m_sourceSpace( IMAGE::COLOURSPACE_SRGB )
    , m_displaySpace( IMAGE::COLOURSPACE_SRGB )
    , m_channel( CHANNEL_RGB )
    , m_checkerboard( true )
    , m_level( 0 )
    , m_frame( 0 )
    , m_frameMeaning( FRAMEMEANING_FRAMES )
    , m_tiling( 1 )
    , m_split( 0.5f )
    , m_zoom( 1.0f )
    , m_panX( 0 )
    , m_panY( 0 )
    , m_fitted( true )
    , m_viewWidth( 0 )
    , m_viewHeight( 0 )
    , m_rebuilds( 0 )
    , m_panning( false )
    , m_splitting( false )
    , m_lastPointerX( 0 )
    , m_lastPointerY( 0 )
    , m_updatingWidgets( false )
    , m_frameSubscribed( false )
    , m_status()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CImageViewForm::~CImageViewForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title )
{GUCEF_TRACE;

    CORE::CDataNode* content = GUCEF_NULL;
    if ( asWindow )
    {
        CORE::CDataNode* window = CFormLayout::AddWidgetNode( layout, "Window", "ImageViewForm", "0.05,0 0.05,0", "0.9,0 0.9,0" );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_Title, title );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_TitleHeight, "28" );
        content = CFormLayout::AddWidgetNode( *window, "Panel", "Content", "0,0 0,28", "1,0 1,-28" );
    }
    else
        content = CFormLayout::AddWidgetNode( layout, "Panel", "ImageViewForm", "0,0 0,0", "1,0 1,0" );

    // the first row of the tool bar: the channel, alpha, tiling and the fit
    Int32 x = 6;
    for ( UInt32 i=0; i<5; ++i )
    {
        const Int32 width = 0 == i ? 52 : 36;
        CORE::CDataNode* button = CFormLayout::AddWidgetNode( *content, "Button", ChannelButtons[ i ], "0," + CORE::Int32ToString( x ) + " 0,4", "0," + CORE::Int32ToString( width ) + " 0,28" );
        CFormLayout::SetWidgetNodeProperty( *button, EWidgetProperty_Text, ChannelTexts[ i ] );
        CFormLayout::SetWidgetNodeProperty( *button, EWidgetProperty_Tooltip, 0 == i ? CString( "Colour" ) : ( 4 == i ? CString( "Alpha" ) : CString( ChannelTexts[ i ] ) + " channel" ) );
        x += width + 4;
    }
    x += 8;
    CORE::CDataNode* checker = CFormLayout::AddWidgetNode( *content, "Checkbox", "Checker", "0," + CORE::Int32ToString( x ) + " 0,4", "0,100 0,28" );
    CFormLayout::SetWidgetNodeProperty( *checker, EWidgetProperty_Text, "Alpha" );
    CFormLayout::SetWidgetNodeProperty( *checker, EWidgetProperty_Tooltip, "Alpha over a checkerboard" );
    CFormLayout::SetWidgetNodeProperty( *checker, EWidgetProperty_Checked, "true" );
    x += 104;
    CORE::CDataNode* tile = CFormLayout::AddWidgetNode( *content, "Checkbox", "Tile", "0," + CORE::Int32ToString( x ) + " 0,4", "0,100 0,28" );
    CFormLayout::SetWidgetNodeProperty( *tile, EWidgetProperty_Text, "3x3" );
    CFormLayout::SetWidgetNodeProperty( *tile, EWidgetProperty_Tooltip, "The picture 3 times in 3 rows" );
    x += 104;
    CORE::CDataNode* fit = CFormLayout::AddWidgetNode( *content, "Button", "Fit", "0," + CORE::Int32ToString( x ) + " 0,4", "0,44 0,28" );
    CFormLayout::SetWidgetNodeProperty( *fit, EWidgetProperty_Text, "Fit" );
    CFormLayout::SetWidgetNodeProperty( *fit, EWidgetProperty_Tooltip, "Fit the picture to the view (F)" );
    x += 48;
    CORE::CDataNode* actual = CFormLayout::AddWidgetNode( *content, "Button", "Actual", "0," + CORE::Int32ToString( x ) + " 0,4", "0,48 0,28" );
    CFormLayout::SetWidgetNodeProperty( *actual, EWidgetProperty_Text, "1:1" );
    CFormLayout::SetWidgetNodeProperty( *actual, EWidgetProperty_Tooltip, "One pixel of the picture on one of the view (1)" );

    // the second row: the level, the frame and the colour space
    CORE::CDataNode* levelLabel = CFormLayout::AddWidgetNode( *content, "Label", "LevelLabel", "0,6 0,40", "0,56 0,24" );
    CFormLayout::SetWidgetNodeProperty( *levelLabel, EWidgetProperty_Text, "Level" );
    CORE::CDataNode* level = CFormLayout::AddWidgetNode( *content, "Combobox", "Level", "0,64 0,36", "0,70 0,28" );
    CFormLayout::SetWidgetNodeProperty( *level, EWidgetProperty_Items, "0" );
    CFormLayout::SetWidgetNodeProperty( *level, EWidgetProperty_SelectedIndex, "0" );
    CFormLayout::SetWidgetNodeProperty( *level, EWidgetProperty_DropDownRows, "10" );
    CORE::CDataNode* frameLabel = CFormLayout::AddWidgetNode( *content, "Label", "FrameLabel", "0,150 0,40", "0,60 0,24" );
    CFormLayout::SetWidgetNodeProperty( *frameLabel, EWidgetProperty_Text, "Frame" );
    CORE::CDataNode* frame = CFormLayout::AddWidgetNode( *content, "Combobox", "Frame", "0,214 0,36", "0,110 0,28" );
    CFormLayout::SetWidgetNodeProperty( *frame, EWidgetProperty_Items, "0" );
    CFormLayout::SetWidgetNodeProperty( *frame, EWidgetProperty_SelectedIndex, "0" );
    CFormLayout::SetWidgetNodeProperty( *frame, EWidgetProperty_DropDownRows, "10" );
    CORE::CDataNode* space = CFormLayout::AddWidgetNode( *content, "Combobox", "ColourSpace", "0,336 0,36", "0,110 0,28" );
    CFormLayout::SetWidgetNodeProperty( *space, EWidgetProperty_Items, "sRGB\nLinear" );
    CFormLayout::SetWidgetNodeProperty( *space, EWidgetProperty_SelectedIndex, "0" );
    CFormLayout::SetWidgetNodeProperty( *space, EWidgetProperty_Tooltip, "Shown as sRGB or as linear values" );

    // the picture in a panel that clips it, the second picture of a comparison over it up to the split line, and the status line
    CORE::CDataNode* view = CFormLayout::AddWidgetNode( *content, "Panel", "View", "0,0 0,68", "1,0 1,-96" );
    CFormLayout::SetWidgetNodeProperty( *view, EWidgetProperty_Clip, "true" );
    CFormLayout::SetWidgetNodeProperty( *view, EWidgetProperty_Focusable, "true" );
    CFormLayout::SetWidgetNodeProperty( *view, EWidgetProperty_BackgroundColour, "24,25,30,255" );
    CORE::CDataNode* imageA = CFormLayout::AddWidgetNode( *view, "ImageFrame", "ImageA", "0,0 0,0", "0,1 0,1" );
    CFormLayout::SetWidgetNodeProperty( *imageA, EWidgetProperty_InputTransparent, "true" );
    CFormLayout::SetWidgetNodeProperty( *imageA, EWidgetProperty_ScaleMode, "Stretch" );
    // the pictures are panned and zoomed in their clipping panels: they pass them on purpose (layout lint)
    CFormLayout::SetWidgetNodeProperty( *imageA, EWidgetProperty_LintIgnore, "OutsideParent,ContentUnreachable,OutsideScreen" );
    CORE::CDataNode* clip = CFormLayout::AddWidgetNode( *view, "Panel", "BClip", "0,0 0,0", "0,1 1,0" );
    CFormLayout::SetWidgetNodeProperty( *clip, EWidgetProperty_Clip, "true" );
    CFormLayout::SetWidgetNodeProperty( *clip, EWidgetProperty_InputTransparent, "true" );
    CFormLayout::SetWidgetNodeProperty( *clip, EWidgetProperty_Visible, "false" );
    CFormLayout::SetWidgetNodeProperty( *clip, EWidgetProperty_BackgroundColour, "0,0,0,0" );
    CFormLayout::SetWidgetNodeProperty( *clip, EWidgetProperty_BorderWidth, "0" );
    CORE::CDataNode* imageB = CFormLayout::AddWidgetNode( *clip, "ImageFrame", "ImageB", "0,0 0,0", "0,1 0,1" );
    CFormLayout::SetWidgetNodeProperty( *imageB, EWidgetProperty_InputTransparent, "true" );
    CFormLayout::SetWidgetNodeProperty( *imageB, EWidgetProperty_ScaleMode, "Stretch" );
    CFormLayout::SetWidgetNodeProperty( *imageB, EWidgetProperty_LintIgnore, "OutsideParent,ContentUnreachable,OutsideScreen" );
    CORE::CDataNode* line = CFormLayout::AddWidgetNode( *view, "Panel", "SplitLine", "0,0 0,0", "0,2 1,0" );
    CFormLayout::SetWidgetNodeProperty( *line, EWidgetProperty_InputTransparent, "true" );
    CFormLayout::SetWidgetNodeProperty( *line, EWidgetProperty_Visible, "false" );
    CFormLayout::SetWidgetNodeProperty( *line, EWidgetProperty_BackgroundColour, "255,210,80,255" );
    CFormLayout::SetWidgetNodeProperty( *line, EWidgetProperty_BorderWidth, "0" );
    CORE::CDataNode* status = CFormLayout::AddWidgetNode( *content, "Label", "Status", "0,6 1,-88", "1,-12 0,80" );
    CFormLayout::SetWidgetNodeProperty( *status, EWidgetProperty_WordWrap, "true" );
}

/*-------------------------------------------------------------------------*/

bool
CImageViewForm::Build( const CString& title )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetContext() )
        return false;
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout, true, title );
    if ( !LoadLayout( layout ) )
        return false;
    return Show();
}

/*-------------------------------------------------------------------------*/

bool
CImageViewForm::Build( CWidget* placeholder )
{GUCEF_TRACE;

    if ( GUCEF_NULL == placeholder )
        return false;
    SetContext( placeholder->GetContext() );
    SetParent( placeholder );
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout, false, CString::Empty );
    return LoadLayout( layout );
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();

    for ( UInt32 i=0; i<5; ++i )
    {
        CWidget* button = GetWidget( ChannelButtons[ i ] );
        if ( GUCEF_NULL != button )
        {
            TEventCallback callback( this, &CImageViewForm::OnChannelActivated );
            SubscribeTo( button, CWidget::ActivatedEvent, callback );
        }
    }
    CWidget* widget = GetWidget( "Fit" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CImageViewForm::OnFitActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Actual" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CImageViewForm::OnActualActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Checker" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback checkedCallback( this, &CImageViewForm::OnCheckerChecked );
        SubscribeTo( widget, CCheckbox::BoxCheckedEvent, checkedCallback );
        TEventCallback uncheckedCallback( this, &CImageViewForm::OnCheckerUnchecked );
        SubscribeTo( widget, CCheckbox::BoxUncheckedEvent, uncheckedCallback );
    }
    widget = GetWidget( "Tile" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback checkedCallback( this, &CImageViewForm::OnTileChecked );
        SubscribeTo( widget, CCheckbox::BoxCheckedEvent, checkedCallback );
        TEventCallback uncheckedCallback( this, &CImageViewForm::OnTileUnchecked );
        SubscribeTo( widget, CCheckbox::BoxUncheckedEvent, uncheckedCallback );
    }
    widget = GetWidget( "Level" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CImageViewForm::OnLevelValueChanged );
        SubscribeTo( widget, CWidget::ValueChangedEvent, callback );
    }
    widget = GetWidget( "Frame" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CImageViewForm::OnFrameValueChanged );
        SubscribeTo( widget, CWidget::ValueChangedEvent, callback );
    }
    widget = GetWidget( "ColourSpace" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CImageViewForm::OnColourSpaceValueChanged );
        SubscribeTo( widget, CWidget::ValueChangedEvent, callback );
    }
    widget = GetWidget( "View" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback downCallback( this, &CImageViewForm::OnViewMouseDown );
        SubscribeTo( widget, CWidget::MouseDownEvent, downCallback );
        TEventCallback upCallback( this, &CImageViewForm::OnViewMouseUp );
        SubscribeTo( widget, CWidget::MouseUpEvent, upCallback );
        TEventCallback moveCallback( this, &CImageViewForm::OnViewMouseMove );
        SubscribeTo( widget, CWidget::MouseMoveEvent, moveCallback );
        TEventCallback wheelCallback( this, &CImageViewForm::OnViewMouseWheel );
        SubscribeTo( widget, CWidget::MouseWheelEvent, wheelCallback );
        TEventCallback lostCallback( this, &CImageViewForm::OnViewCaptureLost );
        SubscribeTo( widget, CWidget::MouseCaptureLostEvent, lostCallback );
        TEventCallback keyCallback( this, &CImageViewForm::OnViewKeyDown );
        SubscribeTo( widget, CWidget::KeyDownEvent, keyCallback );
    }
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_frameSubscribed )
    {
        m_frameSubscribed = true;
        TEventCallback callback( this, &CImageViewForm::OnFrameRendered );
        SubscribeTo( context, CGUIContext::FrameRenderedEvent, callback );
    }

    m_viewWidth = 0;
    m_viewHeight = 0;
    ShowChoices();
    ShowChannelButtons();
    RebuildShown();
}

/*-------------------------------------------------------------------------*/

CImageFrame*
CImageViewForm::GetFrameWidget( const CString& name )
{GUCEF_TRACE;

    CWidget* widget = GetWidget( name );
    return GUCEF_NULL != widget && widget->IsRegisteredAs( "ImageFrame" ) ? static_cast< CImageFrame* >( widget ) : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::SetImage( IMAGE::CImage* image )
{GUCEF_TRACE;

    SetImages( image, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::SetImages( IMAGE::CImage* first, IMAGE::CImage* second )
{GUCEF_TRACE;

    m_first.image = first;
    m_first.made.clear();
    m_second.image = GUCEF_NULL != first ? second : GUCEF_NULL;
    m_second.made.clear();
    m_fitted = true;
    m_level = 0;
    m_frame = 0;
    ShowChoices();
    RebuildShown();
    NotifyViewChanged();
}

/*-------------------------------------------------------------------------*/

IMAGE::CImage*
CImageViewForm::GetImage( void ) const
{GUCEF_TRACE;

    return m_first.image;
}

/*-------------------------------------------------------------------------*/

IMAGE::CImage*
CImageViewForm::GetSecondImage( void ) const
{GUCEF_TRACE;

    return m_second.image;
}

/*-------------------------------------------------------------------------*/

bool
CImageViewForm::IsComparing( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_first.image && GUCEF_NULL != m_second.image;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::SetSourceColourSpace( IMAGE::TColourSpace colourSpace )
{GUCEF_TRACE;

    if ( colourSpace == m_sourceSpace )
        return;
    m_sourceSpace = colourSpace;
    m_first.made.clear();
    m_second.made.clear();
    RebuildShown();
    NotifyViewChanged();
}

/*-------------------------------------------------------------------------*/

IMAGE::TColourSpace
CImageViewForm::GetSourceColourSpace( void ) const
{GUCEF_TRACE;

    return m_sourceSpace;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::SetChannel( EChannel channel )
{GUCEF_TRACE;

    if ( channel == m_channel || (UInt32) channel > (UInt32) CHANNEL_ALPHA )
        return;
    m_channel = channel;
    ShowChannelButtons();
    RebuildShown();
    NotifyViewChanged();
}

/*-------------------------------------------------------------------------*/

CImageViewForm::EChannel
CImageViewForm::GetChannel( void ) const
{GUCEF_TRACE;

    return m_channel;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::SetAlphaCheckerboard( bool on )
{GUCEF_TRACE;

    if ( on == m_checkerboard )
        return;
    m_checkerboard = on;
    ShowChoices();
    RebuildShown();
    NotifyViewChanged();
}

/*-------------------------------------------------------------------------*/

bool
CImageViewForm::GetAlphaCheckerboard( void ) const
{GUCEF_TRACE;

    return m_checkerboard;
}

/*-------------------------------------------------------------------------*/

UInt32
CImageViewForm::GetLevelCount( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_first.image || !m_first.image->HasFrames() )
        return 0;
    const UInt32 frame = m_frame < m_first.image->GetFrameCount() ? m_frame : m_first.image->GetFrameCount() - 1;
    const UInt32 levels = m_first.image->GetMipmapLevels( frame );
    if ( levels > 1 )
        return levels;

    // an image without levels of its own: the ones down to 1x1 are made from it
    IMAGE::TPixelMapPtr base;
    if ( !m_first.image->TryGetPixelMap( frame, 0, base ) || base.IsNULL() )
        return levels;
    UInt32 side = base->GetWidthInPixels() > base->GetHeightInPixels() ? base->GetWidthInPixels() : base->GetHeightInPixels();
    UInt32 count = 1;
    while ( side > 1 && count < 16 )
    {
        side >>= 1;
        ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::ClampLevelAndFrame( void )
{GUCEF_TRACE;

    const UInt32 frames = GetFrameCount();
    if ( m_frame >= frames )
        m_frame = frames > 0 ? frames - 1 : 0;
    const UInt32 levels = GetLevelCount();
    if ( m_level >= levels )
        m_level = levels > 0 ? levels - 1 : 0;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::SetLevel( UInt32 level )
{GUCEF_TRACE;

    const UInt32 before = m_level;
    m_level = level;
    ClampLevelAndFrame();
    if ( m_level == before )
        return;
    ShowChoices();
    RebuildShown();
    NotifyViewChanged();
}

/*-------------------------------------------------------------------------*/

UInt32
CImageViewForm::GetLevel( void ) const
{GUCEF_TRACE;

    return m_level;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::SetFrame( UInt32 frame )
{GUCEF_TRACE;

    const UInt32 before = m_frame;
    m_frame = frame;
    ClampLevelAndFrame();
    if ( m_frame == before )
        return;
    ShowChoices();
    RebuildShown();
    NotifyViewChanged();
}

/*-------------------------------------------------------------------------*/

UInt32
CImageViewForm::GetFrame( void ) const
{GUCEF_TRACE;

    return m_frame;
}

/*-------------------------------------------------------------------------*/

UInt32
CImageViewForm::GetFrameCount( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_first.image ? m_first.image->GetFrameCount() : 0;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::SetFrameMeaning( EFrameMeaning meaning )
{GUCEF_TRACE;

    m_frameMeaning = meaning;
    ShowChoices();
}

/*-------------------------------------------------------------------------*/

CImageViewForm::EFrameMeaning
CImageViewForm::GetFrameMeaning( void ) const
{GUCEF_TRACE;

    return m_frameMeaning;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::SetDisplayColourSpace( IMAGE::TColourSpace colourSpace )
{GUCEF_TRACE;

    if ( colourSpace == m_displaySpace )
        return;
    m_displaySpace = colourSpace;
    ShowChoices();
    RebuildShown();
    NotifyViewChanged();
}

/*-------------------------------------------------------------------------*/

IMAGE::TColourSpace
CImageViewForm::GetDisplayColourSpace( void ) const
{GUCEF_TRACE;

    return m_displaySpace;
}

/*-------------------------------------------------------------------------*/

bool
CImageViewForm::SetTiling( UInt32 tiles )
{GUCEF_TRACE;

    if ( 1 != tiles && 3 != tiles )
        return false;
    if ( tiles == m_tiling )
        return true;
    m_tiling = tiles;
    ShowChoices();
    RebuildShown();
    NotifyViewChanged();
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CImageViewForm::GetTiling( void ) const
{GUCEF_TRACE;

    return m_tiling;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::SetSplit( Float32 part )
{GUCEF_TRACE;

    m_split = part < 0.0f ? 0.0f : ( part > 1.0f ? 1.0f : part );
    ApplyView();
}

/*-------------------------------------------------------------------------*/

Float32
CImageViewForm::GetSplit( void ) const
{GUCEF_TRACE;

    return m_split;
}

/*-------------------------------------------------------------------------*/

bool
CImageViewForm::GetLevelMap( SSlot& slot, UInt32 frame, UInt32 level, IMAGE::TPixelMapPtr& map )
{GUCEF_TRACE;

    if ( GUCEF_NULL == slot.image || !slot.image->HasFrames() )
        return false;
    if ( frame >= slot.image->GetFrameCount() )
        frame = slot.image->GetFrameCount() - 1;
    const UInt32 levels = slot.image->GetMipmapLevels( frame );
    if ( level < levels && ( levels > 1 || 0 == level ) )
        return slot.image->TryGetPixelMap( frame, level, map ) && !map.IsNULL();

    // a level the image does not have: made from the first one
    const UInt32 key = frame * 64 + level;
    GUCEF::map< UInt32, IMAGE::TPixelMapPtr >::const_iterator found = slot.made.find( key );
    if ( found != slot.made.end() )
    {
        map = (*found).second;
        return !map.IsNULL();
    }
    IMAGE::TPixelMapPtr base;
    if ( !slot.image->TryGetPixelMap( frame, 0, base ) || base.IsNULL() )
        return false;
    UInt32 width = base->GetWidthInPixels() >> level;
    UInt32 height = base->GetHeightInPixels() >> level;
    if ( 0 == width )
        width = 1;
    if ( 0 == height )
        height = 1;
    IMAGE::TPixelMapPtr result;
    if ( !IMAGE::CPixelMapResampler::Resample( *base, width, height, IMAGE::RESAMPLEFILTER_BOX, m_sourceSpace, IMAGE::WRAPMODE_CLAMP, result ) || result.IsNULL() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ImageViewForm: The level " << level << " of the picture cannot be made" );
        return false;
    }
    slot.made[ key ] = result;
    map = result;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImageViewForm::MakeShown( SSlot& slot )
{GUCEF_TRACE;

    slot.shown = IMAGE::TPixelMapPtr();
    slot.shownWidth = 0;
    slot.shownHeight = 0;
    IMAGE::TPixelMapPtr source;
    if ( !GetLevelMap( slot, m_frame, m_level, source ) )
        return false;

    // the channels: a copy of the level that has the red, green, blue and alpha that are shown
    const char* mapping = "RGB1";
    switch ( m_channel )
    {
        case CHANNEL_RED:   mapping = "RRR1"; break;
        case CHANNEL_GREEN: mapping = "GGG1"; break;
        case CHANNEL_BLUE:  mapping = "BBB1"; break;
        case CHANNEL_ALPHA: mapping = "AAA1"; break;
        default:            mapping = m_checkerboard ? "RGBA" : "RGB1"; break;
    }
    IMAGE::TPixelMapPtr swizzled;
    bool made = IMAGE::CChannelOperations::Swizzle( *source, mapping, swizzled );
    if ( !made && CHANNEL_RGB == m_channel && m_checkerboard )
        made = IMAGE::CChannelOperations::Swizzle( *source, "RGB1", swizzled );       // no alpha to show
    if ( !made && CHANNEL_ALPHA == m_channel )
        made = IMAGE::CChannelOperations::Swizzle( *source, "1111", swizzled );       // no alpha: all of it is there
    if ( !made )
    {
        // a format the operations do not take: the picture as 8 bit RGBA
        IMAGE::TPixelMapPtr converted;
        if ( source->ConvertFormatTo( IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8, converted ) && !converted.IsNULL() )
            made = IMAGE::CChannelOperations::Swizzle( *converted, mapping, swizzled );
    }
    if ( !made || swizzled.IsNULL() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ImageViewForm: The channels of the picture cannot be read" );
        return false;
    }

    // the colour space: the stored values shown as they are, or converted to the other space
    if ( CHANNEL_ALPHA != m_channel && m_sourceSpace != m_displaySpace )
        IMAGE::CChannelOperations::ConvertColourSpace( *swizzled, m_sourceSpace, m_displaySpace );

    IMAGE::CPixelMapChannelAccess::TFloatVector values;
    if ( !IMAGE::CPixelMapChannelAccess::Read( *swizzled, values ) )
        return false;
    const UInt32 width = swizzled->GetWidthInPixels();
    const UInt32 height = swizzled->GetHeightInPixels();
    const UInt32 channels = swizzled->GetNumberOfChannelsPerPixel();
    if ( 0 == width || 0 == height || 0 == channels || values.size() < (size_t) width * height * channels )
        return false;
    const bool withAlpha = CHANNEL_RGB == m_channel && m_checkerboard && 4 == channels;

    // 8 bit RGBA, the alpha over the checkerboard when asked for
    IMAGE::TPixelMapPtr tile = MakeRgba( width, height );
    UInt8* out = static_cast< UInt8* >( tile->GetDataPtr() );
    for ( UInt32 y=0; y<height; ++y )
    {
        for ( UInt32 x=0; x<width; ++x )
        {
            const Float32* in = &values[ ( (size_t) y * width + x ) * channels ];
            Float32 r = in[ 0 ], g = channels > 1 ? in[ 1 ] : in[ 0 ], b = channels > 2 ? in[ 2 ] : in[ 0 ];
            if ( withAlpha )
            {
                const Float32 a = in[ 3 ] < 0.0f ? 0.0f : ( in[ 3 ] > 1.0f ? 1.0f : in[ 3 ] );
                const Float32 back = ( ( x / (UInt32) CHECKER_CELL_SIZE ) + ( y / (UInt32) CHECKER_CELL_SIZE ) ) % 2 ? 0.60f : 0.40f;
                r = r * a + back * ( 1.0f - a );
                g = g * a + back * ( 1.0f - a );
                b = b * a + back * ( 1.0f - a );
            }
            UInt8* pixel = out + ( (size_t) y * width + x ) * 4;
            pixel[ 0 ] = ToByte( r );
            pixel[ 1 ] = ToByte( g );
            pixel[ 2 ] = ToByte( b );
            pixel[ 3 ] = 255;
        }
    }

    // the tiles: the picture 3 times in 3 rows
    if ( 3 == m_tiling && width * 3 <= MaxShownSide && height * 3 <= MaxShownSide )
    {
        IMAGE::TPixelMapPtr tiled = MakeRgba( width * 3, height * 3 );
        UInt8* target = static_cast< UInt8* >( tiled->GetDataPtr() );
        for ( UInt32 row=0; row<height * 3; ++row )
            for ( UInt32 repeat=0; repeat<3; ++repeat )
                memcpy( target + ( (size_t) row * width * 3 + (size_t) repeat * width ) * 4, out + (size_t) ( row % height ) * width * 4, (size_t) width * 4 );
        slot.shown = tiled;
        slot.shownWidth = width * 3;
        slot.shownHeight = height * 3;
    }
    else
    {
        slot.shown = tile;
        slot.shownWidth = width;
        slot.shownHeight = height;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::RebuildShown( void )
{GUCEF_TRACE;

    ClampLevelAndFrame();
    ++m_rebuilds;
    MakeShown( m_first );
    if ( GUCEF_NULL != m_second.image )
        MakeShown( m_second );
    else
        m_second.shown = IMAGE::TPixelMapPtr();

    CImageFrame* frameA = GetFrameWidget( "ImageA" );
    if ( GUCEF_NULL != frameA )
    {
        if ( !m_first.shown.IsNULL() )
            frameA->SetImage( m_first.shown );
        else
            frameA->SetImageName( CString::Empty );
    }
    CImageFrame* frameB = GetFrameWidget( "ImageB" );
    if ( GUCEF_NULL != frameB )
    {
        if ( !m_second.shown.IsNULL() )
            frameB->SetImage( m_second.shown );
        else
            frameB->SetImageName( CString::Empty );
    }
    if ( m_fitted )
        m_zoom = FittedZoom();
    ApplyView();
}

/*-------------------------------------------------------------------------*/

Float32
CImageViewForm::FittedZoom( void ) const
{GUCEF_TRACE;

    if ( 0 == m_first.shownWidth || 0 == m_first.shownHeight || m_viewWidth <= 0 || m_viewHeight <= 0 )
        return 1.0f;
    const Float32 byWidth = (Float32) m_viewWidth / (Float32) m_first.shownWidth;
    const Float32 byHeight = (Float32) m_viewHeight / (Float32) m_first.shownHeight;
    return ClampZoom( byWidth < byHeight ? byWidth : byHeight );
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::ApplyView( void )
{GUCEF_TRACE;

    CWidget* view = GetWidget( "View" );
    if ( GUCEF_NULL == view )
        return;
    if ( m_fitted && m_viewWidth > 0 && m_viewHeight > 0 )
    {
        m_zoom = FittedZoom();
        m_panX = ( m_viewWidth - (Int32) ( (Float32) m_first.shownWidth * m_zoom + 0.5f ) ) / 2;
        m_panY = ( m_viewHeight - (Int32) ( (Float32) m_first.shownHeight * m_zoom + 0.5f ) ) / 2;
    }
    const bool smooth = m_zoom < 1.0f;

    CImageFrame* frameA = GetFrameWidget( "ImageA" );
    if ( GUCEF_NULL != frameA )
    {
        const Int32 width = (Int32) ( (Float32) m_first.shownWidth * m_zoom + 0.5f );
        const Int32 height = (Int32) ( (Float32) m_first.shownHeight * m_zoom + 0.5f );
        frameA->SetProperty( EWidgetProperty_Position, "0," + CORE::Int32ToString( m_panX ) + " 0," + CORE::Int32ToString( m_panY ) );
        frameA->SetProperty( EWidgetProperty_Size, "0," + CORE::Int32ToString( width > 0 ? width : 1 ) + " 0," + CORE::Int32ToString( height > 0 ? height : 1 ) );
        frameA->SetBoolProperty( EWidgetProperty_Smooth, smooth );
        frameA->SetBoolProperty( EWidgetProperty_Visible, !m_first.shown.IsNULL() );
    }

    // the second picture of a comparison, up to the split line
    const bool comparing = IsComparing() && !m_second.shown.IsNULL();
    const Int32 splitX = (Int32) ( m_split * (Float32) ( m_viewWidth > 0 ? m_viewWidth : 0 ) + 0.5f );
    CWidget* clip = GetWidget( "BClip" );
    if ( GUCEF_NULL != clip )
    {
        clip->SetBoolProperty( EWidgetProperty_Visible, comparing );
        clip->SetProperty( EWidgetProperty_Size, "0," + CORE::Int32ToString( splitX > 0 ? splitX : 1 ) + " 1,0" );
    }
    CImageFrame* frameB = GetFrameWidget( "ImageB" );
    if ( GUCEF_NULL != frameB )
    {
        const Int32 width = (Int32) ( (Float32) m_second.shownWidth * m_zoom + 0.5f );
        const Int32 height = (Int32) ( (Float32) m_second.shownHeight * m_zoom + 0.5f );
        frameB->SetProperty( EWidgetProperty_Position, "0," + CORE::Int32ToString( m_panX ) + " 0," + CORE::Int32ToString( m_panY ) );
        frameB->SetProperty( EWidgetProperty_Size, "0," + CORE::Int32ToString( width > 0 ? width : 1 ) + " 0," + CORE::Int32ToString( height > 0 ? height : 1 ) );
        frameB->SetBoolProperty( EWidgetProperty_Smooth, smooth );
    }
    CWidget* line = GetWidget( "SplitLine" );
    if ( GUCEF_NULL != line )
    {
        line->SetBoolProperty( EWidgetProperty_Visible, comparing );
        line->SetProperty( EWidgetProperty_Position, "0," + CORE::Int32ToString( splitX - 1 ) + " 0,0" );
    }
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::ShowChoices( void )
{GUCEF_TRACE;

    m_updatingWidgets = true;
    CCombobox* level = GetWidgetAs< CCombobox >( "Level" );
    if ( GUCEF_NULL != level )
    {
        CCombobox::TStringVector items;
        const UInt32 count = GetLevelCount();
        for ( UInt32 i=0; i<( count > 0 ? count : 1 ); ++i )
            items.push_back( CORE::UInt32ToString( i ) );
        level->SetListItems( items );
        level->SetSelectedIndex( (Int32) m_level );
    }
    CCombobox* frame = GetWidgetAs< CCombobox >( "Frame" );
    if ( GUCEF_NULL != frame )
    {
        static const char* const faces[] = { "+X", "-X", "+Y", "-Y", "+Z", "-Z" };
        CCombobox::TStringVector items;
        const UInt32 count = GetFrameCount();
        for ( UInt32 i=0; i<( count > 0 ? count : 1 ); ++i )
        {
            if ( FRAMEMEANING_CUBE_FACES == m_frameMeaning && i < 6 )
                items.push_back( faces[ i ] );
            else
                items.push_back( CORE::UInt32ToString( i ) );
        }
        frame->SetListItems( items );
        frame->SetSelectedIndex( (Int32) m_frame );
    }
    CWidget* label = GetWidget( "FrameLabel" );
    if ( GUCEF_NULL != label )
        label->SetProperty( EWidgetProperty_Text, FRAMEMEANING_CUBE_FACES == m_frameMeaning ? "Face" : ( FRAMEMEANING_ARRAY_LAYERS == m_frameMeaning ? "Layer" : "Frame" ) );
    CWidget* space = GetWidget( "ColourSpace" );
    if ( GUCEF_NULL != space )
        space->SetIntProperty( EWidgetProperty_SelectedIndex, IMAGE::COLOURSPACE_SRGB == m_displaySpace ? 0 : 1 );
    CWidget* checker = GetWidget( "Checker" );
    if ( GUCEF_NULL != checker )
        checker->SetBoolProperty( EWidgetProperty_Checked, m_checkerboard );
    CWidget* tile = GetWidget( "Tile" );
    if ( GUCEF_NULL != tile )
        tile->SetBoolProperty( EWidgetProperty_Checked, 3 == m_tiling );
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::ShowChannelButtons( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<5; ++i )
    {
        CWidget* button = GetWidget( ChannelButtons[ i ] );
        if ( GUCEF_NULL != button )
            button->SetProperty( EWidgetProperty_StyleClass, i == (UInt32) m_channel ? "Primary" : "" );
    }
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::NotifyViewChanged( void )
{GUCEF_TRACE;

    TViewState state;
    state.zoom = m_zoom;
    state.panX = m_panX;
    state.panY = m_panY;
    state.channel = m_channel;
    state.frame = m_frame;
    state.level = m_level;
    TViewChangedEventData eventData( state );
    NotifyObservers( ViewChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::SetZoomKeeping( Float32 zoom, Int32 viewX, Int32 viewY, bool user )
{GUCEF_TRACE;

    zoom = ClampZoom( zoom );
    if ( zoom == m_zoom && !m_fitted )
        return;

    // the point of the picture under the point of the panel stays there
    const Float32 pictureX = ( (Float32) ( viewX - m_panX ) ) / m_zoom;
    const Float32 pictureY = ( (Float32) ( viewY - m_panY ) ) / m_zoom;
    m_fitted = false;
    m_zoom = zoom;
    m_panX = viewX - (Int32) ( pictureX * zoom + ( pictureX >= 0.0f ? 0.5f : -0.5f ) );
    m_panY = viewY - (Int32) ( pictureY * zoom + ( pictureY >= 0.0f ? 0.5f : -0.5f ) );
    ApplyView();
    NotifyViewChanged();
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::SetZoom( Float32 zoom )
{GUCEF_TRACE;

    SetZoomKeeping( zoom, m_viewWidth / 2, m_viewHeight / 2, false );
}

/*-------------------------------------------------------------------------*/

Float32
CImageViewForm::GetZoom( void ) const
{GUCEF_TRACE;

    return m_zoom;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::ZoomAt( Float32 factor, Int32 viewX, Int32 viewY )
{GUCEF_TRACE;

    if ( !( factor > 0.0f ) )
        return;
    SetZoomKeeping( m_zoom * factor, viewX, viewY, false );
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::FitToView( void )
{GUCEF_TRACE;

    m_fitted = true;
    ApplyView();
    NotifyViewChanged();
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::ActualSize( void )
{GUCEF_TRACE;

    m_fitted = false;
    m_zoom = 1.0f;
    m_panX = ( m_viewWidth - (Int32) m_first.shownWidth ) / 2;
    m_panY = ( m_viewHeight - (Int32) m_first.shownHeight ) / 2;
    ApplyView();
    NotifyViewChanged();
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::SetPan( Int32 panX, Int32 panY )
{GUCEF_TRACE;

    if ( panX == m_panX && panY == m_panY && !m_fitted )
        return;
    m_fitted = false;
    m_panX = panX;
    m_panY = panY;
    ApplyView();
    NotifyViewChanged();
}

/*-------------------------------------------------------------------------*/

Int32
CImageViewForm::GetPanX( void ) const
{GUCEF_TRACE;

    return m_panX;
}

/*-------------------------------------------------------------------------*/

Int32
CImageViewForm::GetPanY( void ) const
{GUCEF_TRACE;

    return m_panY;
}

/*-------------------------------------------------------------------------*/

bool
CImageViewForm::IsFitted( void ) const
{GUCEF_TRACE;

    return m_fitted;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::GetViewSize( Int32& width, Int32& height ) const
{GUCEF_TRACE;

    width = m_viewWidth;
    height = m_viewHeight;
}

/*-------------------------------------------------------------------------*/

IMAGE::TPixelMapPtr
CImageViewForm::GetShownPixels( void ) const
{GUCEF_TRACE;

    return m_first.shown;
}

/*-------------------------------------------------------------------------*/

IMAGE::TPixelMapPtr
CImageViewForm::GetSecondShownPixels( void ) const
{GUCEF_TRACE;

    return m_second.shown;
}

/*-------------------------------------------------------------------------*/

UInt32
CImageViewForm::GetRebuildCount( void ) const
{GUCEF_TRACE;

    return m_rebuilds;
}

/*-------------------------------------------------------------------------*/

CString
CImageViewForm::GetStatusText( void )
{GUCEF_TRACE;

    return m_status;
}

/*-------------------------------------------------------------------------*/

CString
CImageViewForm::FormatStored( const IMAGE::CPixelMap& map, UInt32 x, UInt32 y ) const
{GUCEF_TRACE;

    if ( x >= map.GetWidthInPixels() || y >= map.GetHeightInPixels() )
        return CString::Empty;
    const UInt8* pixel = static_cast< const UInt8* >( map.GetDataAtScanLine( y ) ) + (size_t) x * map.GetSizeOfPixelInBytes();
    const UInt32 channels = map.GetNumberOfChannelsPerPixel();
    const UInt32 size = map.GetPixelChannelSize();
    const char* letters = ChannelLetters( map.GetPixelStorageFormat() );
    const UInt32 lettersCount = (UInt32) strlen( letters );
    CString text;
    for ( UInt32 c=0; c<channels; ++c )
    {
        if ( c > 0 )
            text += "  ";
        text += ( c < lettersCount && channels == lettersCount ? CString( &letters[ c ], 1 ) : CString( "#" ) + CORE::UInt32ToString( c ) ) + " " + RawValueText( pixel + (size_t) c * size, map.GetPixelComponentDataType() );
    }
    return text;
}

/*-------------------------------------------------------------------------*/

CString
CImageViewForm::FormatShown( const IMAGE::CPixelMap& map, UInt32 x, UInt32 y ) const
{GUCEF_TRACE;

    if ( x >= map.GetWidthInPixels() || y >= map.GetHeightInPixels() )
        return CString::Empty;
    const UInt8* pixel = static_cast< const UInt8* >( map.GetDataAtScanLine( y ) ) + (size_t) x * 4;
    return "R " + CORE::UInt32ToString( pixel[ 0 ] ) + "  G " + CORE::UInt32ToString( pixel[ 1 ] ) + "  B " + CORE::UInt32ToString( pixel[ 2 ] ) + "  A " + CORE::UInt32ToString( pixel[ 3 ] );
}

/*-------------------------------------------------------------------------*/

bool
CImageViewForm::ReadProbe( Int32 viewX, Int32 viewY, TPixelProbe& probe )
{GUCEF_TRACE;

    if ( m_first.shown.IsNULL() || !( m_zoom > 0.0f ) )
        return false;

    // a comparison shows the second picture left of the split line
    const bool inSecond = IsComparing() && !m_second.shown.IsNULL() && viewX < (Int32) ( m_split * (Float32) m_viewWidth + 0.5f );
    SSlot& slot = inSecond ? m_second : m_first;
    const Float32 px = (Float32) ( viewX - m_panX ) / m_zoom;
    const Float32 py = (Float32) ( viewY - m_panY ) / m_zoom;
    if ( px < 0.0f || py < 0.0f || px >= (Float32) slot.shownWidth || py >= (Float32) slot.shownHeight )
        return false;
    UInt32 x = (UInt32) px, y = (UInt32) py;

    // the pixel of the level that was read, in the tile it is in
    IMAGE::TPixelMapPtr source;
    if ( !GetLevelMap( slot, m_frame, m_level, source ) )
        return false;
    const UInt32 width = source->GetWidthInPixels(), height = source->GetHeightInPixels();
    if ( 0 == width || 0 == height )
        return false;
    probe.x = (Int32) ( x % width );
    probe.y = (Int32) ( y % height );
    probe.inSecond = inSecond;
    probe.storedValue = FormatStored( *source, (UInt32) probe.x, (UInt32) probe.y );
    probe.shownValue = FormatShown( *slot.shown, x, y );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImageViewForm::ProbeAt( Int32 viewX, Int32 viewY )
{GUCEF_TRACE;

    TPixelProbe probe;
    if ( !ReadProbe( viewX, viewY, probe ) )
        return false;
    m_status = CORE::Int32ToString( probe.x ) + ", " + CORE::Int32ToString( probe.y ) + ( probe.inSecond ? " (second)" : "" ) +
               "\nStored: " + probe.storedValue + "\nShown: " + probe.shownValue;
    CWidget* status = GetWidget( "Status" );
    if ( GUCEF_NULL != status )
        status->SetProperty( EWidgetProperty_Text, m_status );
    TPixelProbedEventData eventData( probe );
    NotifyObservers( PixelProbedEvent, &eventData );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::Update( void )
{GUCEF_TRACE;

    CWidget* view = GetWidget( "View" );
    if ( GUCEF_NULL == view )
        return;
    const CRect& rect = view->GetRect();
    if ( rect.width == m_viewWidth && rect.height == m_viewHeight )
        return;
    m_viewWidth = rect.width;
    m_viewHeight = rect.height;
    ApplyView();
    if ( m_fitted )
        NotifyViewChanged();
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::OnChannelActivated( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<5; ++i )
        if ( notifier == GetWidget( ChannelButtons[ i ] ) )
            SetChannel( (EChannel) i );
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::OnFitActivated( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    FitToView();
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::OnActualActivated( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ActualSize();
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::OnCheckerChecked( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_updatingWidgets )
        SetAlphaCheckerboard( true );
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::OnCheckerUnchecked( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_updatingWidgets )
        SetAlphaCheckerboard( false );
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::OnTileChecked( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_updatingWidgets )
        SetTiling( 3 );
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::OnTileUnchecked( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_updatingWidgets )
        SetTiling( 1 );
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::OnLevelValueChanged( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CCombobox* level = GetWidgetAs< CCombobox >( "Level" );
    if ( !m_updatingWidgets && GUCEF_NULL != level && level->GetSelectedIndex() >= 0 )
        SetLevel( (UInt32) level->GetSelectedIndex() );
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::OnFrameValueChanged( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CCombobox* frame = GetWidgetAs< CCombobox >( "Frame" );
    if ( !m_updatingWidgets && GUCEF_NULL != frame && frame->GetSelectedIndex() >= 0 )
        SetFrame( (UInt32) frame->GetSelectedIndex() );
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::OnColourSpaceValueChanged( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CCombobox* space = GetWidgetAs< CCombobox >( "ColourSpace" );
    if ( !m_updatingWidgets && GUCEF_NULL != space && space->GetSelectedIndex() >= 0 )
        SetDisplayColourSpace( 0 == space->GetSelectedIndex() ? IMAGE::COLOURSPACE_SRGB : IMAGE::COLOURSPACE_LINEAR );
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::OnViewMouseDown( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CPointerEventData* pointer = static_cast< CPointerEventData* >( eventData );
    if ( GUCEF_NULL == pointer || MOUSEBUTTON_LEFT != pointer->button )
        return;
    CWidget* view = GetWidget( "View" );
    if ( GUCEF_NULL != view )
        view->SetFocus();
    const Int32 splitX = (Int32) ( m_split * (Float32) m_viewWidth + 0.5f );
    if ( IsComparing() && abs( pointer->localX - splitX ) <= 6 )
        m_splitting = true;
    else
        m_panning = true;
    m_lastPointerX = pointer->localX;
    m_lastPointerY = pointer->localY;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::OnViewMouseUp( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_panning = false;
    m_splitting = false;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::OnViewCaptureLost( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_panning = false;
    m_splitting = false;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::OnViewMouseMove( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CPointerEventData* pointer = static_cast< CPointerEventData* >( eventData );
    if ( GUCEF_NULL == pointer )
        return;
    if ( m_splitting && m_viewWidth > 0 )
    {
        SetSplit( (Float32) pointer->localX / (Float32) m_viewWidth );
        NotifyViewChanged();
    }
    else
    if ( m_panning )
    {
        m_panX += pointer->localX - m_lastPointerX;
        m_panY += pointer->localY - m_lastPointerY;
        m_fitted = false;
        ApplyView();
        NotifyViewChanged();
    }
    m_lastPointerX = pointer->localX;
    m_lastPointerY = pointer->localY;
    ProbeAt( pointer->localX, pointer->localY );
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::OnViewMouseWheel( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CWheelEventData* wheel = static_cast< CWheelEventData* >( eventData );
    if ( GUCEF_NULL == wheel || 0 == wheel->delta )
        return;
    const Float32 factor = (Float32) pow( 1.25, (double) wheel->delta );
    ZoomAt( factor, wheel->localX, wheel->localY );
    wheel->handled = true;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::OnViewKeyDown( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CKeyEventData* key = static_cast< CKeyEventData* >( eventData );
    if ( GUCEF_NULL == key || key->handled || EVENTPHASE_TARGET != key->phase )
        return;
    bool used = true;
    switch ( key->keyCode )
    {
        case INPUT::KEYCODE_F:          FitToView(); break;
        case INPUT::KEYCODE_1:          ActualSize(); break;
        case INPUT::KEYCODE_EQUALS:
        case INPUT::KEYCODE_ADD:        ZoomAt( 1.25f, m_viewWidth / 2, m_viewHeight / 2 ); break;
        case INPUT::KEYCODE_MINUS:
        case INPUT::KEYCODE_SUBTRACT:   ZoomAt( 0.8f, m_viewWidth / 2, m_viewHeight / 2 ); break;
        case INPUT::KEYCODE_LEFT:       SetPan( m_panX + 24, m_panY ); break;
        case INPUT::KEYCODE_RIGHT:      SetPan( m_panX - 24, m_panY ); break;
        case INPUT::KEYCODE_UP:         SetPan( m_panX, m_panY + 24 ); break;
        case INPUT::KEYCODE_DOWN:       SetPan( m_panX, m_panY - 24 ); break;
        default:                        used = false; break;
    }
    if ( used )
        key->handled = true;
}

/*-------------------------------------------------------------------------*/

void
CImageViewForm::OnFrameRendered( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Update();
}

/*-------------------------------------------------------------------------*/

const CString&
CImageViewForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CImageViewForm";
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
