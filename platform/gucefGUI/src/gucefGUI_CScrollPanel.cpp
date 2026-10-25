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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "gucefGUI_CScrollPanel.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The thumb never gets smaller than this, so it can be grabbed */
const Int32 MinThumbHeight = 32;

/** Room kept around a widget that is scrolled into view, so its neighbours show too */
const Int32 ScrollIntoViewMargin = 8;

Int32
Clamp( Int32 value, Int32 low, Int32 high )
{
    if ( value > high )
        value = high;
    if ( value < low )
        value = low;
    return value;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CScrollPanel::CScrollPanel( void )
    : CPanel( "ScrollPanel" )
    , m_contentHeight( 0 )
    , m_viewportHeight( 0 )
    , m_thumbDragged( false )
    , m_thumbGrabOffset( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CScrollPanel::~CScrollPanel()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWidget*
CScrollPanel::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CScrollPanel();
}

/*-------------------------------------------------------------------------*/

bool
CScrollPanel::SetScrollPosition( Int32 pixels )
{GUCEF_TRACE;

    return SetIntProperty( EWidgetProperty_ScrollPosition, pixels < 0 ? 0 : pixels );
}

/*-------------------------------------------------------------------------*/

Int32
CScrollPanel::GetScrollPosition( void ) const
{GUCEF_TRACE;

    return GetIntProperty( EWidgetProperty_ScrollPosition );
}

/*-------------------------------------------------------------------------*/

Int32
CScrollPanel::GetContentHeight( void ) const
{GUCEF_TRACE;

    return m_contentHeight;
}

/*-------------------------------------------------------------------------*/

Int32
CScrollPanel::GetViewportHeight( void ) const
{GUCEF_TRACE;

    return m_viewportHeight;
}

/*-------------------------------------------------------------------------*/

Int32
CScrollPanel::GetMaxScrollPosition( void ) const
{GUCEF_TRACE;

    Int32 range = m_contentHeight - m_viewportHeight;
    return range > 0 ? range : 0;
}

/*-------------------------------------------------------------------------*/

bool
CScrollPanel::IsScrollable( void ) const
{GUCEF_TRACE;

    return m_contentHeight > m_viewportHeight && m_viewportHeight > 0;
}

/*-------------------------------------------------------------------------*/

CRect
CScrollPanel::GetColumnRect( void ) const
{GUCEF_TRACE;

    const CRect& rect = GetRect();
    Int32 width = rect.width < 0 ? 0 : rect.width;
    Int32 left = 0;
    const Int32 maxWidth = GetIntProperty( EWidgetProperty_MaxColumnWidth );
    if ( maxWidth > 0 && maxWidth < width )
    {
        left = ( width - maxWidth ) / 2;
        width = maxWidth;
    }
    return CRect( left, 0, width, rect.height );
}

/*-------------------------------------------------------------------------*/

CRect
CScrollPanel::GetViewportRect( void ) const
{GUCEF_TRACE;

    CRect column = GetColumnRect();
    Int32 bar = GetIntProperty( EWidgetProperty_ScrollBarWidth );
    if ( bar < 0 )
        bar = 0;
    Int32 width = column.width - bar;
    return CRect( column.left, 0, width > 0 ? width : 0, column.height );
}

/*-------------------------------------------------------------------------*/

CRect
CScrollPanel::GetTrackRect( void ) const
{GUCEF_TRACE;

    CRect column = GetColumnRect();
    Int32 bar = GetIntProperty( EWidgetProperty_ScrollBarWidth );
    if ( bar <= 0 )
        return CRect();
    if ( bar > column.width )
        bar = column.width;
    return CRect( column.left + column.width - bar, 0, bar, column.height );
}

/*-------------------------------------------------------------------------*/

CRect
CScrollPanel::GetThumbRect( void ) const
{GUCEF_TRACE;

    CRect track = GetTrackRect();
    if ( !IsScrollable() || track.IsEmpty() )
        return CRect();

    Int32 height = (Int32) ( (Int64) track.height * m_viewportHeight / m_contentHeight );
    if ( height < MinThumbHeight )
        height = MinThumbHeight;
    if ( height > track.height )
        height = track.height;
    Int32 range = GetMaxScrollPosition();
    Int32 scroll = Clamp( GetScrollPosition(), 0, range );
    Int32 top = range > 0 ? (Int32) ( (Int64) ( track.height - height ) * scroll / range ) : 0;
    return CRect( track.left, track.top + top, track.width, height );
}

/*-------------------------------------------------------------------------*/

bool
CScrollPanel::IsThumbDragged( void ) const
{GUCEF_TRACE;

    return m_thumbDragged;
}

/*-------------------------------------------------------------------------*/

void
CScrollPanel::SetExtent( Int32 contentHeight, Int32 viewportHeight )
{GUCEF_TRACE;

    if ( contentHeight == m_contentHeight && viewportHeight == m_viewportHeight )
        return;
    m_contentHeight = contentHeight;
    m_viewportHeight = viewportHeight;

    // where the bar is, for whoever reads the log to see what the pointer was over
    const CRect& rect = GetRect();
    const CRect track = GetTrackRect();
    const Int32 barLeft = rect.left + track.left;
    const Int32 barRight = rect.left + track.Right();
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "Scroll panel " << GetNamePath() << ": at " << rect.left << "," << rect.top << " size " << rect.width << "x" << rect.height
                    << ", scroll bar at x " << barLeft << ".." << barRight << ", content " << contentHeight << ", viewport " << viewportHeight );

}

/*-------------------------------------------------------------------------*/

bool
CScrollPanel::ScrollIntoView( const CWidget* widget )
{GUCEF_TRACE;

    if ( GUCEF_NULL == widget || !IsScrollable() )
        return false;

    // the rectangles are those of the last layout, relative to the panel
    const CRect& panel = GetRect();
    const CRect& target = widget->GetRect();
    Int32 top = target.top - panel.top;
    Int32 bottom = target.Bottom() - panel.top;
    Int32 scroll = GetScrollPosition();

    Int32 delta = 0;
    if ( target.height + 2 * ScrollIntoViewMargin >= m_viewportHeight )
        delta = top - ScrollIntoViewMargin;                                      // higher than the viewport: its top matters
    else
    if ( top < ScrollIntoViewMargin )
        delta = top - ScrollIntoViewMargin;
    else
    if ( bottom > m_viewportHeight - ScrollIntoViewMargin )
        delta = bottom - ( m_viewportHeight - ScrollIntoViewMargin );
    scroll += delta;

    scroll = Clamp( scroll, 0, GetMaxScrollPosition() );
    if ( scroll == GetScrollPosition() )
        return false;
    return SetIntProperty( EWidgetProperty_ScrollPosition, scroll );
}

/*-------------------------------------------------------------------------*/

bool
CScrollPanel::ScrollByUser( Int32 position )
{GUCEF_TRACE;

    position = Clamp( position, 0, GetMaxScrollPosition() );
    if ( position == GetScrollPosition() )
        return false;
    return ChangeValueByUser( EWidgetProperty_ScrollPosition, CORE::Int32ToString( position ) );
}

/*-------------------------------------------------------------------------*/

bool
CScrollPanel::ScrollByKey( INPUT::KeyCode keyCode, bool lines )
{GUCEF_TRACE;

    if ( !IsScrollable() )
        return false;
    Int32 page = m_viewportHeight - m_viewportHeight / 10;
    if ( page < 1 )
        page = 1;
    Int32 step = GetIntProperty( EWidgetProperty_ScrollStep );
    if ( step <= 0 )
        step = 48;
    const Int32 position = GetScrollPosition();
    switch ( keyCode )
    {
        case INPUT::KEYCODE_PGUP: ScrollByUser( position - page ); return true;
        case INPUT::KEYCODE_PGDOWN: ScrollByUser( position + page ); return true;
        case INPUT::KEYCODE_HOME: ScrollByUser( 0 ); return true;
        case INPUT::KEYCODE_END: ScrollByUser( GetMaxScrollPosition() ); return true;
        case INPUT::KEYCODE_UP: if ( lines ) { ScrollByUser( position - step ); return true; } return false;
        case INPUT::KEYCODE_DOWN: if ( lines ) { ScrollByUser( position + step ); return true; } return false;
        default: return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CScrollPanel::DragThumbTo( Int32 localY )
{GUCEF_TRACE;

    CRect track = GetTrackRect();
    CRect thumb = GetThumbRect();
    Int32 travel = track.height - thumb.height;
    if ( travel <= 0 )
        return;
    Int32 top = Clamp( localY - m_thumbGrabOffset - track.top, 0, travel );
    ScrollByUser( (Int32) ( ( (Int64) top * GetMaxScrollPosition() + travel / 2 ) / travel ) );
}

/*-------------------------------------------------------------------------*/

void
CScrollPanel::OnDescendantFocusReceived( CWidget* descendant )
{GUCEF_TRACE;

    ScrollIntoView( descendant );
}

/*-------------------------------------------------------------------------*/

bool
CScrollPanel::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( MouseWheelEvent == eventId )
    {
        if ( !IsScrollable() )
            return false;
        const CWheelEventData* wheel = static_cast< const CWheelEventData* >( &eventData );
        Int32 step = GetIntProperty( EWidgetProperty_ScrollStep );
        if ( step <= 0 )
            step = 48;
        // at the end of its range it leaves the wheel to a scroll panel around it
        return ScrollByUser( GetScrollPosition() - wheel->delta * step );
    }
    if ( MouseDownEvent == eventId )
    {
        const CPointerEventData* pointer = static_cast< const CPointerEventData* >( &eventData );
        if ( MOUSEBUTTON_LEFT != pointer->button || !IsScrollable() || !GetTrackRect().Contains( pointer->localX, pointer->localY ) )
            return false;
        CRect thumb = GetThumbRect();
        if ( thumb.Contains( pointer->localX, pointer->localY ) )
        {
            m_thumbDragged = true;
            m_thumbGrabOffset = pointer->localY - thumb.top;
            SetBoolProperty( EWidgetProperty_ThumbHovered, true );
            GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "Scroll panel " << GetNamePath() << ": thumb grabbed at " << pointer->localY << " (thumb " << thumb.top << ".." << thumb.Bottom()
                            << "), position " << GetScrollPosition() << " of " << GetMaxScrollPosition() );
        }
        else
        {
            // a page towards the click
            GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "Scroll panel " << GetNamePath() << ": track clicked at " << pointer->localY << " (thumb " << thumb.top << ".." << thumb.Bottom()
                            << "), position " << GetScrollPosition() << " of " << GetMaxScrollPosition() );
            ScrollByUser( GetScrollPosition() + ( pointer->localY < thumb.top ? -1 : 1 ) * ( m_viewportHeight - m_viewportHeight / 10 ) );
        }
        return true;
    }
    if ( MouseMoveEvent == eventId )
    {
        const CPointerEventData* pointer = static_cast< const CPointerEventData* >( &eventData );
        if ( m_thumbDragged )
        {
            if ( 0 == ( pointer->buttonStates & ( 1u << (UInt32) MOUSEBUTTON_LEFT ) ) )
            {
                m_thumbDragged = false;
                return true;
            }
            DragThumbTo( pointer->localY );
            return true;
        }
        SetBoolProperty( EWidgetProperty_ThumbHovered, GetThumbRect().Contains( pointer->localX, pointer->localY ) );
        return false;
    }
    if ( MouseUpEvent == eventId || MouseCaptureLostEvent == eventId )
    {
        bool wasDragged = m_thumbDragged;
        m_thumbDragged = false;
        if ( wasDragged )
        {
            SetBoolProperty( EWidgetProperty_ThumbHovered, false );
            CString how = "lost";
            if ( MouseUpEvent == eventId )
                how = "released";
            GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "Scroll panel " << GetNamePath() << ": thumb " << how << " at position "
                            << GetScrollPosition() << " of " << GetMaxScrollPosition() );
        }
        return wasDragged;
    }
    if ( MouseFocusLostEvent == eventId )
    {
        if ( !m_thumbDragged )
            SetBoolProperty( EWidgetProperty_ThumbHovered, false );
        return false;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CString&
CScrollPanel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CScrollPanel";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CScrollPanel*
CScrollPanel::AsScrollPanel( void )
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

const CScrollPanel*
CScrollPanel::AsScrollPanel( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
