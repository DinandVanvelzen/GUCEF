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

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CFOLDABLEPANEL_H
#include "gucefGUI_CFoldablePanel.h"
#define GUCEF_GUI_CFOLDABLEPANEL_H
#endif /* GUCEF_GUI_CFOLDABLEPANEL_H ? */

#include "gucefGUI_CSplitter.h"

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

const CORE::CEvent CSplitter::SizeChangedEvent = "GUCEF::GUI::CSplitter::SizeChangedEvent";
const CORE::CEvent CSplitter::CollapsedChangedEvent = "GUCEF::GUI::CSplitter::CollapsedChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CSplitter::RegisterEvents( void )
{GUCEF_TRACE;

    SizeChangedEvent.Initialize();
    CollapsedChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CSplitter::CSplitter( void )
    : CPanel( "Splitter" )
    , m_dragging( false )
    , m_handleActive( false )
    , m_cursorApplied( false )
    , m_previousCursor( 0 )
    , m_grabOffset( 0 )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CSplitter::CSplitter( const CString& widgetTypeName )
    : CPanel( widgetTypeName )
    , m_dragging( false )
    , m_handleActive( false )
    , m_cursorApplied( false )
    , m_previousCursor( 0 )
    , m_grabOffset( 0 )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CSplitter::~CSplitter()
{GUCEF_TRACE;

    RestoreCursor();
}

/*-------------------------------------------------------------------------*/

CWidget*
CSplitter::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CSplitter();
}

/*-------------------------------------------------------------------------*/

bool
CSplitter::IsSideBySide( void ) const
{GUCEF_TRACE;

    return GetPropertyRef( EWidgetProperty_Orientation ) != "Vertical";
}

/*-------------------------------------------------------------------------*/

Float32
CSplitter::GetSplitRatio( void ) const
{GUCEF_TRACE;

    Float32 ratio = GetFloatProperty( EWidgetProperty_SplitRatio );
    if ( ratio < 0.0f ) ratio = 0.0f;
    if ( ratio > 1.0f ) ratio = 1.0f;
    return ratio;
}

/*-------------------------------------------------------------------------*/

bool
CSplitter::SetSplitRatio( Float32 ratio )
{GUCEF_TRACE;

    if ( ratio < 0.0f ) ratio = 0.0f;
    if ( ratio > 1.0f ) ratio = 1.0f;
    return SetFloatProperty( EWidgetProperty_SplitRatio, ratio );
}

/*-------------------------------------------------------------------------*/

bool
CSplitter::ResetSplitRatio( void )
{GUCEF_TRACE;

    return SetSplitRatio( GetFloatProperty( EWidgetProperty_DefaultSplitRatio ) );
}

/*-------------------------------------------------------------------------*/

Int32
CSplitter::GetTotalExtent( void ) const
{GUCEF_TRACE;

    return IsSideBySide() ? GetRect().width : GetRect().height;
}

/*-------------------------------------------------------------------------*/

CWidget*
CSplitter::GetFirstPane( void ) const
{GUCEF_TRACE;

    const TWidgetVector& children = GetChildren();
    return children.size() > 0 ? children[ 0 ] : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CWidget*
CSplitter::GetSecondPane( void ) const
{GUCEF_TRACE;

    const TWidgetVector& children = GetChildren();
    return children.size() > 1 ? children[ 1 ] : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

Int32
CSplitter::GetFoldedExtentOf( const CWidget* pane ) const
{GUCEF_TRACE;

    // -1: the pane is not folded
    const CFoldablePanel* panel = CWidget::As< CFoldablePanel >( pane );
    if ( GUCEF_NULL == panel || !panel->IsFolded() )
        return -1;
    return panel->GetFoldedExtent();
}

/*-------------------------------------------------------------------------*/

bool
CSplitter::IsFirstPaneFolded( void ) const
{GUCEF_TRACE;

    return GetFoldedExtentOf( GetFirstPane() ) >= 0;
}

/*-------------------------------------------------------------------------*/

bool
CSplitter::IsSecondPaneFolded( void ) const
{GUCEF_TRACE;

    return GetFoldedExtentOf( GetSecondPane() ) >= 0;
}

/*-------------------------------------------------------------------------*/

void
CSplitter::ComputeExtents( Int32& first, Int32& handle, Int32& second ) const
{GUCEF_TRACE;

    const Int32 total = GetTotalExtent() > 0 ? GetTotalExtent() : 0;
    const Int32 foldedFirst = GetFoldedExtentOf( GetFirstPane() );
    const Int32 foldedSecond = GetFoldedExtentOf( GetSecondPane() );

    // a folded pane is its strip and the handle has nothing to move
    handle = ( foldedFirst >= 0 || foldedSecond >= 0 ) ? 0 : GetIntProperty( EWidgetProperty_HandleSize );
    if ( handle < 0 ) handle = 0;
    if ( handle > total ) handle = total;
    const Int32 available = total - handle;

    if ( foldedFirst >= 0 )
    {
        first = foldedFirst < available ? foldedFirst : available;
        second = available - first;
        return;
    }
    if ( foldedSecond >= 0 )
    {
        second = foldedSecond < available ? foldedSecond : available;
        first = available - second;
        return;
    }

    Int32 minFirst = GetIntProperty( EWidgetProperty_FirstMinSize );
    Int32 minSecond = GetIntProperty( EWidgetProperty_SecondMinSize );
    if ( minFirst < 0 ) minFirst = 0;
    if ( minSecond < 0 ) minSecond = 0;
    // minimum sizes that do not both fit give the first pane its share and the second what is left
    const Int32 low = minFirst < available ? minFirst : available;
    Int32 high = available - minSecond;
    if ( high < low )
        high = low;
    first = (Int32) ( GetSplitRatio() * (Float32) available + 0.5f );
    if ( first > high ) first = high;
    if ( first < low ) first = low;
    second = available - first;
}

/*-------------------------------------------------------------------------*/

Int32
CSplitter::GetFirstPaneExtent( void ) const
{GUCEF_TRACE;

    Int32 first = 0, handle = 0, second = 0;
    ComputeExtents( first, handle, second );
    return first;
}

/*-------------------------------------------------------------------------*/

Int32
CSplitter::GetHandleExtent( void ) const
{GUCEF_TRACE;

    Int32 first = 0, handle = 0, second = 0;
    ComputeExtents( first, handle, second );
    return handle;
}

/*-------------------------------------------------------------------------*/

Int32
CSplitter::GetSecondPaneExtent( void ) const
{GUCEF_TRACE;

    Int32 first = 0, handle = 0, second = 0;
    ComputeExtents( first, handle, second );
    return second;
}

/*-------------------------------------------------------------------------*/

bool
CSplitter::SetFirstPaneExtent( Int32 pixels )
{GUCEF_TRACE;

    const Int32 total = GetTotalExtent() > 0 ? GetTotalExtent() : 0;
    Int32 handle = GetIntProperty( EWidgetProperty_HandleSize );
    if ( handle < 0 ) handle = 0;
    if ( handle > total ) handle = total;
    const Int32 available = total - handle;
    if ( available <= 0 )
        return false;

    Int32 minFirst = GetIntProperty( EWidgetProperty_FirstMinSize );
    Int32 minSecond = GetIntProperty( EWidgetProperty_SecondMinSize );
    if ( minFirst < 0 ) minFirst = 0;
    if ( minSecond < 0 ) minSecond = 0;
    const Int32 low = minFirst < available ? minFirst : available;
    Int32 high = available - minSecond;
    if ( high < low )
        high = low;
    if ( pixels > high ) pixels = high;
    if ( pixels < low ) pixels = low;
    return SetSplitRatio( (Float32) pixels / (Float32) available );
}

/*-------------------------------------------------------------------------*/

void
CSplitter::GetPaneRects( CRect& first, CRect& second ) const
{GUCEF_TRACE;

    Int32 firstExtent = 0, handle = 0, secondExtent = 0;
    ComputeExtents( firstExtent, handle, secondExtent );
    const CRect& rect = GetRect();
    if ( IsSideBySide() )
    {
        first = CRect( rect.left, rect.top, firstExtent, rect.height );
        second = CRect( rect.left + firstExtent + handle, rect.top, secondExtent, rect.height );
    }
    else
    {
        first = CRect( rect.left, rect.top, rect.width, firstExtent );
        second = CRect( rect.left, rect.top + firstExtent + handle, rect.width, secondExtent );
    }
}

/*-------------------------------------------------------------------------*/

CRect
CSplitter::GetHandleRect( void ) const
{GUCEF_TRACE;

    Int32 first = 0, handle = 0, second = 0;
    ComputeExtents( first, handle, second );
    if ( handle <= 0 )
        return CRect();
    const CRect& rect = GetRect();
    return IsSideBySide() ? CRect( first, 0, handle, rect.height ) : CRect( 0, first, rect.width, handle );
}

/*-------------------------------------------------------------------------*/

bool
CSplitter::IsHandleActive( void ) const
{GUCEF_TRACE;

    return m_handleActive || m_dragging;
}

/*-------------------------------------------------------------------------*/

bool
CSplitter::IsDragging( void ) const
{GUCEF_TRACE;

    return m_dragging;
}

/*-------------------------------------------------------------------------*/

void
CSplitter::SetHandleActive( bool active )
{GUCEF_TRACE;

    if ( active == m_handleActive )
        return;
    m_handleActive = active;
    // tells the painter and the drivers: the handle is drawn lighter
    SetBoolProperty( EWidgetProperty_ThumbHovered, active );
}

/*-------------------------------------------------------------------------*/

void
CSplitter::UpdateCursor( bool onHandle )
{GUCEF_TRACE;

    CGUIContext* context = GetContext();
    if ( GUCEF_NULL == context )
        return;
    if ( !onHandle )
    {
        RestoreCursor();
        return;
    }
    if ( !m_cursorApplied )
    {
        m_previousCursor = (UInt32) context->GetSystemCursorShape();
        m_cursorApplied = true;
    }
    context->SetSystemCursorShape( IsSideBySide() ? CURSORSHAPE_RESIZE_EW : CURSORSHAPE_RESIZE_NS );
}

/*-------------------------------------------------------------------------*/

void
CSplitter::RestoreCursor( void )
{GUCEF_TRACE;

    CGUIContext* context = GetContext();
    if ( m_cursorApplied && GUCEF_NULL != context )
        context->SetSystemCursorShape( (TCursorShape) m_previousCursor );
    m_cursorApplied = false;
}

/*-------------------------------------------------------------------------*/

void
CSplitter::EndDrag( void )
{GUCEF_TRACE;

    m_dragging = false;
}

/*-------------------------------------------------------------------------*/

void
CSplitter::OnPaneFoldChanged( CWidget& pane )
{GUCEF_TRACE;

    const CFoldablePanel* panel = CWidget::As< CFoldablePanel >( &pane );
    const bool folded = GUCEF_NULL != panel && panel->IsFolded();
    if ( folded )
    {
        // the handle is gone: nothing to be dragged and no resize cursor
        EndDrag();
        SetHandleActive( false );
        RestoreCursor();
    }
    CValueChangedEventData eventData;
    eventData.source = &pane;
    eventData.phase = EVENTPHASE_TARGET;
    eventData.property = EWidgetProperty_Collapsed;
    eventData.oldValue = folded ? "false" : "true";
    eventData.newValue = folded ? "true" : "false";
    NotifyObservers( CollapsedChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CSplitter::OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

    if ( EWidgetProperty_SplitRatio == property )
    {
        CValueChangedEventData eventData;
        eventData.source = this;
        eventData.phase = EVENTPHASE_TARGET;
        eventData.property = EWidgetProperty_SplitRatio;
        eventData.oldValue = oldValue;
        eventData.newValue = newValue;
        NotifyObservers( SizeChangedEvent, &eventData );
    }
}

/*-------------------------------------------------------------------------*/

bool
CSplitter::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( MouseMoveEvent == eventId )
    {
        const CPointerEventData* pointer = static_cast< const CPointerEventData* >( &eventData );
        if ( m_dragging )
        {
            if ( 0 == ( pointer->buttonStates & ( 1u << (UInt32) MOUSEBUTTON_LEFT ) ) )
            {
                EndDrag();
                return true;
            }
            const Int32 position = IsSideBySide() ? pointer->localX : pointer->localY;
            SetFirstPaneExtent( position - m_grabOffset );
            return true;
        }
        const bool onHandle = GetHandleRect().Contains( pointer->localX, pointer->localY );
        SetHandleActive( onHandle );
        UpdateCursor( onHandle );
        return false;
    }
    if ( MouseFocusLostEvent == eventId )
    {
        if ( !m_dragging )
        {
            SetHandleActive( false );
            RestoreCursor();
        }
        return false;
    }
    if ( MouseDownEvent == eventId )
    {
        const CPointerEventData* pointer = static_cast< const CPointerEventData* >( &eventData );
        if ( MOUSEBUTTON_LEFT != pointer->button )
            return false;
        const CRect handle = GetHandleRect();
        if ( !handle.Contains( pointer->localX, pointer->localY ) )
            return false;
        m_dragging = true;
        m_grabOffset = IsSideBySide() ? pointer->localX - handle.left : pointer->localY - handle.top;
        SetHandleActive( true );
        SetFocus();
        return true;
    }
    if ( MouseUpEvent == eventId || MouseCaptureLostEvent == eventId )
    {
        const bool wasDragging = m_dragging;
        EndDrag();
        if ( MouseCaptureLostEvent == eventId )
        {
            SetHandleActive( false );
            RestoreCursor();
        }
        return wasDragging;
    }
    if ( MouseDoubleClickEvent == eventId )
    {
        const CPointerEventData* pointer = static_cast< const CPointerEventData* >( &eventData );
        if ( MOUSEBUTTON_LEFT == pointer->button && GetHandleRect().Contains( pointer->localX, pointer->localY ) )
        {
            EndDrag();
            ResetSplitRatio();
            return true;
        }
        return false;
    }
    if ( NavigationEvent == eventId )
    {
        if ( GetHandleRect().IsEmpty() )
            return false;
        const TNavigation direction = static_cast< CNavigationEventData* >( &eventData )->direction;
        Int32 change = 0;
        if ( IsSideBySide() )
        {
            if ( NAVIGATION_RIGHT == direction ) change = KEYSTEP;
            if ( NAVIGATION_LEFT == direction ) change = -KEYSTEP;
        }
        else
        {
            if ( NAVIGATION_DOWN == direction ) change = KEYSTEP;
            if ( NAVIGATION_UP == direction ) change = -KEYSTEP;
        }
        if ( 0 == change )
            return false;
        SetFirstPaneExtent( GetFirstPaneExtent() + change );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CString&
CSplitter::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CSplitter";
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
