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

#ifndef GUCEF_GUI_CSPLITTER_H
#include "gucefGUI_CSplitter.h"
#define GUCEF_GUI_CSPLITTER_H
#endif /* GUCEF_GUI_CSPLITTER_H ? */

#include "gucefGUI_CFoldablePanel.h"

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

const CORE::CEvent CFoldablePanel::FoldedChangedEvent = "GUCEF::GUI::CFoldablePanel::FoldedChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CFoldablePanel::RegisterEvents( void )
{GUCEF_TRACE;

    FoldedChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CFoldablePanel::CFoldablePanel( void )
    : CPanel( "FoldablePanel" )
    , m_folded( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CFoldablePanel::~CFoldablePanel()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWidget*
CFoldablePanel::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CFoldablePanel();
}

/*-------------------------------------------------------------------------*/

bool
CFoldablePanel::IsFolded( void ) const
{GUCEF_TRACE;

    return GetBoolProperty( EWidgetProperty_Collapsed );
}

/*-------------------------------------------------------------------------*/

bool
CFoldablePanel::SetFolded( bool folded )
{GUCEF_TRACE;

    return SetBoolProperty( EWidgetProperty_Collapsed, folded );
}

/*-------------------------------------------------------------------------*/

bool
CFoldablePanel::ToggleFolded( void )
{GUCEF_TRACE;

    return SetFolded( !IsFolded() );
}

/*-------------------------------------------------------------------------*/

bool
CFoldablePanel::IsStripVertical( void ) const
{GUCEF_TRACE;

    const CString& dock = GetPropertyRef( EWidgetProperty_Dock );
    return dock == "Left" || dock == "Right";
}

/*-------------------------------------------------------------------------*/

Int32
CFoldablePanel::GetFoldedExtent( void ) const
{GUCEF_TRACE;

    // a header is never less than a line of its text: a larger font or a larger line spacing makes the strip thicker
    Int32 height = GetIntProperty( EWidgetProperty_TitleHeight );
    CGUIContext* context = GetContext();
    if ( GUCEF_NULL != context )
    {
        const Int32 line = context->GetMinimumLineHeight( GetPropertyRef( EWidgetProperty_Font ) );
        if ( line > height )
            height = line;
    }
    return height > 0 ? height : 0;
}

/*-------------------------------------------------------------------------*/

CRect
CFoldablePanel::GetHeaderRect( void ) const
{GUCEF_TRACE;

    const CRect& rect = GetRect();
    if ( IsFolded() )
        return CRect( 0, 0, rect.width, rect.height );
    return CRect( 0, 0, rect.width, GetFoldedExtent() < rect.height ? GetFoldedExtent() : rect.height );
}

/*-------------------------------------------------------------------------*/

CRect
CFoldablePanel::GetFoldButtonRect( void ) const
{GUCEF_TRACE;

    const CRect header = GetHeaderRect();
    const Int32 size = GetFoldedExtent() < header.width ? GetFoldedExtent() : header.width;
    if ( IsFolded() )
    {
        // the strip: the button at its head, the top of a vertical one and the left end of a horizontal one
        return IsStripVertical() ? CRect( 0, 0, header.width, size < header.height ? size : header.height )
                                 : CRect( 0, 0, size < header.width ? size : header.width, header.height );
    }
    // the header: the button at the end that is away from the edge the panel sits at
    const bool atLeft = GetPropertyRef( EWidgetProperty_Dock ) == "Right";
    return atLeft ? CRect( 0, 0, size, header.height ) : CRect( header.width - size, 0, size, header.height );
}

/*-------------------------------------------------------------------------*/

CRect
CFoldablePanel::GetContentRect( void ) const
{GUCEF_TRACE;

    const CRect& rect = GetRect();
    if ( IsFolded() )
        return CRect( rect.left, rect.top, 0, 0 );
    const CRect header = GetHeaderRect();
    return CRect( rect.left, rect.top + header.height, rect.width, rect.height - header.height );
}

/*-------------------------------------------------------------------------*/

bool
CFoldablePanel::AreChildrenShown( void ) const
{GUCEF_TRACE;

    return !IsFolded();
}

/*-------------------------------------------------------------------------*/

void
CFoldablePanel::AdjustPlacement( CRect& rect, const CRect& area, bool sizeFromArea )
{GUCEF_TRACE;

    // folded: only the strip at the edge of the parent the panel sits at
    if ( !IsFolded() )
        return;
    const Int32 extent = GetFoldedExtent();
    const CString& dock = GetPropertyRef( EWidgetProperty_Dock );
    if ( dock == "Left" )
    {
        if ( rect.width > extent )
            rect.width = extent;
    }
    else
    if ( dock == "Right" )
    {
        if ( rect.width > extent )
        {
            rect.left = rect.Right() - extent;
            rect.width = extent;
        }
    }
    else
    if ( dock == "Bottom" )
    {
        if ( rect.height > extent )
        {
            rect.top = rect.Bottom() - extent;
            rect.height = extent;
        }
    }
    else
    {
        if ( rect.height > extent )
            rect.height = extent;
    }
}

/*-------------------------------------------------------------------------*/

void
CFoldablePanel::UpdateFoldedState( void )
{GUCEF_TRACE;

    const bool folded = IsFolded();
    if ( folded == m_folded )
        return;
    m_folded = folded;

    // what was shown in the panel is gone: the focus cannot stay on it
    CGUIContext* context = GetContext();
    if ( folded && GUCEF_NULL != context && GUCEF_NULL != context->GetFocus() )
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
    eventData.oldValue = folded ? "false" : "true";
    eventData.newValue = folded ? "true" : "false";
    NotifyObservers( FoldedChangedEvent, &eventData );

    CSplitter* splitter = CWidget::As< CSplitter >( GetParentWidget() );
    if ( GUCEF_NULL != splitter )
        splitter->OnPaneFoldChanged( *this );
}

/*-------------------------------------------------------------------------*/

void
CFoldablePanel::OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

    if ( EWidgetProperty_Collapsed == property )
        UpdateFoldedState();
}

/*-------------------------------------------------------------------------*/

bool
CFoldablePanel::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( MouseMoveEvent == eventId )
    {
        const CPointerEventData* pointer = static_cast< const CPointerEventData* >( &eventData );
        SetBoolProperty( EWidgetProperty_FoldButtonHovered, GetHeaderRect().Contains( pointer->localX, pointer->localY ) );
        return false;
    }
    if ( MouseFocusLostEvent == eventId )
    {
        SetBoolProperty( EWidgetProperty_FoldButtonHovered, false );
        return false;
    }
    if ( MouseClickEvent == eventId )
    {
        const CPointerEventData* pointer = static_cast< const CPointerEventData* >( &eventData );
        if ( MOUSEBUTTON_LEFT == pointer->button && GetHeaderRect().Contains( pointer->localX, pointer->localY ) )
        {
            ToggleFolded();
            return true;
        }
        return false;
    }
    if ( NavigationEvent == eventId )
    {
        if ( NAVIGATION_ACCEPT == static_cast< CNavigationEventData* >( &eventData )->direction && HasFocus() )
        {
            ToggleFolded();
            return true;
        }
        return false;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CString&
CFoldablePanel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CFoldablePanel";
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
