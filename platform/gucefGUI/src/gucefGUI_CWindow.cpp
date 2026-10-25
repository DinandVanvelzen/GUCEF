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

#include "gucefGUI_CGUIContext.h"

#include "gucefGUI_CWindow.h"

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

const CORE::CEvent CWindow::MovedEvent = "GUCEF::GUI::CWindow::MovedEvent";
const CORE::CEvent CWindow::CollapsedChangedEvent = "GUCEF::GUI::CWindow::CollapsedChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

Int32
Clamp( Int32 value, Int32 low, Int32 high )
{
    if ( value > high )
        value = high;
    if ( value < low )
        value = low;
    return value;
}

CUVector
MovedBy( const CUVector& position, Int32 dx, Int32 dy )
{
    return CUVector( CUDim( position.GetX().GetRelativeValue(), position.GetX().GetPixelValue() + dx ),
                     CUDim( position.GetY().GetRelativeValue(), position.GetY().GetPixelValue() + dy ) );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CWindow::RegisterEvents( void )
{GUCEF_TRACE;

    MovedEvent.Initialize();
    CollapsedChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CWindow::CWindow( void )
    : CPanel( "Window" )
    , m_dragging( false )
    , m_movedByUser( false )
    , m_collapsed( false )
    , m_dragStartX( 0 )
    , m_dragStartY( 0 )
    , m_dragStartPosition()
    , m_dragStartRect()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CWindow::~CWindow()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CWidget*
CWindow::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CWindow();
}

/*-------------------------------------------------------------------------*/

bool
CWindow::SetTitleBarText( const CString& title )
{GUCEF_TRACE;

    return SetProperty( EWidgetProperty_Title, title );
}

/*-------------------------------------------------------------------------*/

bool
CWindow::GetTitleBarText( CString& title ) const
{GUCEF_TRACE;

    title = GetProperty( EWidgetProperty_Title );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWindow::SetTitleBarVisible( const bool isVisible )
{GUCEF_TRACE;

    return SetBoolProperty( EWidgetProperty_TitleVisible, isVisible );
}

/*-------------------------------------------------------------------------*/

bool
CWindow::GetTitleBarVisible( bool& isVisible ) const
{GUCEF_TRACE;

    isVisible = GetBoolProperty( EWidgetProperty_TitleVisible );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWindow::SetMovable( bool movable )
{GUCEF_TRACE;

    return SetBoolProperty( EWidgetProperty_Movable, movable );
}

/*-------------------------------------------------------------------------*/

bool
CWindow::IsMovable( void ) const
{GUCEF_TRACE;

    return GetBoolProperty( EWidgetProperty_Movable ) && GetTitleBarHeight() > 0;
}

/*-------------------------------------------------------------------------*/

bool
CWindow::SetCollapsible( bool collapsible )
{GUCEF_TRACE;

    return SetBoolProperty( EWidgetProperty_Collapsible, collapsible );
}

/*-------------------------------------------------------------------------*/

bool
CWindow::IsCollapsible( void ) const
{GUCEF_TRACE;

    return GetBoolProperty( EWidgetProperty_Collapsible ) && GetTitleBarHeight() > 0;
}

/*-------------------------------------------------------------------------*/

bool
CWindow::SetCollapsed( bool collapsed )
{GUCEF_TRACE;

    return SetBoolProperty( EWidgetProperty_Collapsed, collapsed );
}

/*-------------------------------------------------------------------------*/

bool
CWindow::IsCollapsed( void ) const
{GUCEF_TRACE;

    return GetBoolProperty( EWidgetProperty_Collapsed ) && IsCollapsible();
}

/*-------------------------------------------------------------------------*/

bool
CWindow::ToggleCollapsed( void )
{GUCEF_TRACE;

    return SetBoolProperty( EWidgetProperty_Collapsed, !GetBoolProperty( EWidgetProperty_Collapsed ) );
}

/*-------------------------------------------------------------------------*/

Int32
CWindow::GetTitleBarHeight( void ) const
{GUCEF_TRACE;

    if ( !GetBoolProperty( EWidgetProperty_TitleVisible ) )
        return 0;
    Int32 height = GetIntProperty( EWidgetProperty_TitleHeight );
    return height > 0 ? height : 0;
}

/*-------------------------------------------------------------------------*/

CRect
CWindow::GetTitleBarRect( void ) const
{GUCEF_TRACE;

    Int32 height = GetTitleBarHeight();
    if ( height <= 0 )
        return CRect();
    return CRect( 0, 0, GetRect().width, height );
}

/*-------------------------------------------------------------------------*/

CRect
CWindow::GetFoldButtonRect( void ) const
{GUCEF_TRACE;

    if ( !IsCollapsible() )
        return CRect();
    Int32 size = GetTitleBarHeight();
    Int32 width = GetRect().width;
    if ( size > width )
        size = width;
    return CRect( width - size, 0, size, GetTitleBarHeight() );
}

/*-------------------------------------------------------------------------*/

bool
CWindow::IsDragging( void ) const
{GUCEF_TRACE;

    return m_dragging;
}

/*-------------------------------------------------------------------------*/

bool
CWindow::IsMovedByUser( void ) const
{GUCEF_TRACE;

    return m_movedByUser;
}

/*-------------------------------------------------------------------------*/

bool
CWindow::AreChildrenShown( void ) const
{GUCEF_TRACE;

    return !IsCollapsed();
}

/*-------------------------------------------------------------------------*/

CRect
CWindow::GetMoveArea( void ) const
{GUCEF_TRACE;

    const CWidget* parent = GetParentWidget();
    if ( GUCEF_NULL != parent )
        return parent->GetRect();
    CGUIContext* context = GetContext();
    if ( GUCEF_NULL != context )
        return CRect( 0, 0, (Int32) context->GetLogicalWidth(), (Int32) context->GetLogicalHeight() );
    return CRect( 0, 0, 0x7fffff, 0x7fffff );
}

/*-------------------------------------------------------------------------*/

void
CWindow::ClampToArea( CRect& rect, const CRect& area ) const
{GUCEF_TRACE;

    // a window that fits stays inside, a wider one covers the area: either way the title bar is reachable
    Int32 left = area.left;
    Int32 right = area.Right() - rect.width;
    if ( rect.width > area.width )
    {
        left = area.Right() - rect.width;
        right = area.left;
    }
    rect.left = Clamp( rect.left, left, right );

    Int32 bottom = area.Bottom() - GetTitleBarHeight();
    if ( bottom < area.top )
        bottom = area.top;
    rect.top = Clamp( rect.top, area.top, bottom );
}

/*-------------------------------------------------------------------------*/

void
CWindow::BeginDrag( const CPointerEventData& pointer )
{GUCEF_TRACE;

    m_dragging = true;
    m_dragStartX = pointer.x;
    m_dragStartY = pointer.y;
    m_dragStartPosition = GetUVectorProperty( EWidgetProperty_Position );
    m_dragStartRect = GetRect();
}

/*-------------------------------------------------------------------------*/

void
CWindow::DragTo( const CPointerEventData& pointer )
{GUCEF_TRACE;

    if ( 0 == ( pointer.buttonStates & ( 1u << (UInt32) MOUSEBUTTON_LEFT ) ) )
    {
        EndDrag();
        return;
    }

    CRect target = m_dragStartRect;
    target.left += pointer.x - m_dragStartX;
    target.top += pointer.y - m_dragStartY;
    ClampToArea( target, GetMoveArea() );

    CUVector current = GetUVectorProperty( EWidgetProperty_Position );
    CUVector moved = MovedBy( m_dragStartPosition, target.left - m_dragStartRect.left, target.top - m_dragStartRect.top );
    if ( moved == current )
        return;

    CString oldValue = current.ToString();
    SetProperty( EWidgetProperty_Position, moved.ToString() );
    m_movedByUser = true;

    CValueChangedEventData eventData;
    eventData.source = this;
    eventData.phase = EVENTPHASE_TARGET;
    eventData.property = EWidgetProperty_Position;
    eventData.oldValue = oldValue;
    eventData.newValue = moved.ToString();
    NotifyObservers( MovedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CWindow::EndDrag( void )
{GUCEF_TRACE;

    m_dragging = false;
}

/*-------------------------------------------------------------------------*/

void
CWindow::UpdateCollapsedState( void )
{GUCEF_TRACE;

    bool collapsed = IsCollapsed();
    if ( collapsed == m_collapsed )
        return;
    m_collapsed = collapsed;

    // what was shown in the window is gone: the focus cannot stay on it
    CGUIContext* context = GetContext();
    if ( collapsed && GUCEF_NULL != context && GUCEF_NULL != context->GetFocus() )
    {
        for ( const CWidget* w=context->GetFocus(); GUCEF_NULL != w; w=w->GetParentWidget() )
        {
            if ( w == this )
            {
                context->SetFocus( GUCEF_NULL );
                break;
            }
        }
    }

    CValueChangedEventData eventData;
    eventData.source = this;
    eventData.phase = EVENTPHASE_TARGET;
    eventData.property = EWidgetProperty_Collapsed;
    eventData.oldValue = collapsed ? "false" : "true";
    eventData.newValue = collapsed ? "true" : "false";
    NotifyObservers( CollapsedChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CWindow::OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

    if ( property == EWidgetProperty_Collapsed || property == EWidgetProperty_Collapsible || property == EWidgetProperty_TitleVisible || property == EWidgetProperty_TitleHeight )
        UpdateCollapsedState();
    if ( property == EWidgetProperty_Movable && !IsMovable() )
        EndDrag();
}

/*-------------------------------------------------------------------------*/

void
CWindow::AdjustSize( Int32& width, Int32& height, const CRect& area ) const
{GUCEF_TRACE;

    CString minValue = GetProperty( EWidgetProperty_MinSize );
    if ( !minValue.IsNULLOrEmpty() )
    {
        CUVector minSize;
        if ( CUVector::FromString( minValue, minSize ) )
        {
            Int32 minWidth = minSize.GetX().Resolve( area.width );
            Int32 minHeight = minSize.GetY().Resolve( area.height );
            // a window is never made larger than its parent for the sake of a minimum
            if ( minWidth > area.width )
                minWidth = area.width;
            if ( minHeight > area.height )
                minHeight = area.height;
            if ( minWidth > 0 && width < minWidth )
                width = minWidth;
            if ( minHeight > 0 && height < minHeight )
                height = minHeight;
        }
    }
    CString maxValue = GetProperty( EWidgetProperty_MaxSize );
    if ( !maxValue.IsNULLOrEmpty() )
    {
        CUVector maxSize;
        if ( CUVector::FromString( maxValue, maxSize ) )
        {
            Int32 maxWidth = maxSize.GetX().Resolve( area.width );
            Int32 maxHeight = maxSize.GetY().Resolve( area.height );
            if ( maxWidth > 0 && width > maxWidth )
                width = maxWidth;
            if ( maxHeight > 0 && height > maxHeight )
                height = maxHeight;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CWindow::AdjustPlacement( CRect& rect, const CRect& area, bool sizeFromArea )
{GUCEF_TRACE;

    // folded: only the title bar, the top edge stays where the unfolded window has it
    if ( IsCollapsed() )
        rect.height = GetTitleBarHeight();

    // a window the user moved stays reachable when its parent changes size
    if ( sizeFromArea || !m_movedByUser )
        return;
    CRect clamped = rect;
    ClampToArea( clamped, area );
    if ( clamped.left == rect.left && clamped.top == rect.top )
        return;
    SetProperty( EWidgetProperty_Position, MovedBy( GetUVectorProperty( EWidgetProperty_Position ), clamped.left - rect.left, clamped.top - rect.top ).ToString() );
    rect = clamped;
}

/*-------------------------------------------------------------------------*/

bool
CWindow::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( MouseMoveEvent == eventId )
    {
        const CPointerEventData* pointer = static_cast< const CPointerEventData* >( &eventData );
        if ( m_dragging )
        {
            DragTo( *pointer );
            return true;
        }
        SetBoolProperty( EWidgetProperty_FoldButtonHovered, GetFoldButtonRect().Contains( pointer->localX, pointer->localY ) );
        return false;
    }
    if ( MouseFocusLostEvent == eventId )
    {
        SetBoolProperty( EWidgetProperty_FoldButtonHovered, false );
        return false;
    }
    if ( MouseDownEvent == eventId )
    {
        const CPointerEventData* pointer = static_cast< const CPointerEventData* >( &eventData );
        if ( MOUSEBUTTON_LEFT != pointer->button )
            return false;
        if ( GetFoldButtonRect().Contains( pointer->localX, pointer->localY ) )
        {
            SetBoolProperty( EWidgetProperty_FoldButtonPressed, true );
            return true;
        }
        if ( IsMovable() && GetTitleBarRect().Contains( pointer->localX, pointer->localY ) )
        {
            BeginDrag( *pointer );
            return true;
        }
        return false;
    }
    if ( MouseUpEvent == eventId || MouseCaptureLostEvent == eventId )
    {
        bool wasActive = m_dragging || GetBoolProperty( EWidgetProperty_FoldButtonPressed );
        EndDrag();
        SetBoolProperty( EWidgetProperty_FoldButtonPressed, false );
        return wasActive;
    }
    if ( MouseClickEvent == eventId )
    {
        const CPointerEventData* pointer = static_cast< const CPointerEventData* >( &eventData );
        if ( MOUSEBUTTON_LEFT == pointer->button && GetFoldButtonRect().Contains( pointer->localX, pointer->localY ) )
        {
            ToggleCollapsed();
            return true;
        }
        return false;
    }
    if ( MouseDoubleClickEvent == eventId )
    {
        const CPointerEventData* pointer = static_cast< const CPointerEventData* >( &eventData );
        if ( MOUSEBUTTON_LEFT == pointer->button && IsCollapsible() && GetTitleBarRect().Contains( pointer->localX, pointer->localY ) &&
             !GetFoldButtonRect().Contains( pointer->localX, pointer->localY ) )
        {
            ToggleCollapsed();
            return true;
        }
        return false;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CString&
CWindow::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CWindow";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CWindow*
CWindow::AsWindow( void )
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

const CWindow*
CWindow::AsWindow( void ) const
{GUCEF_TRACE;

    return this;
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
