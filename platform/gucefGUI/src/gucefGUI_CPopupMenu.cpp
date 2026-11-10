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

#ifndef GUCEF_GUI_UTILS_H
#include "gucefGUI_Utils.h"
#define GUCEF_GUI_UTILS_H
#endif /* GUCEF_GUI_UTILS_H ? */

#include "gucefGUI_CPopupMenu.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/


namespace {

Int32
RowHeightOf( const CWidget& widget )
{
    Int32 rowHeight = widget.GetIntProperty( EWidgetProperty_RowHeight );
    if ( rowHeight > 0 )
        return rowHeight;
    if ( GUCEF_NULL != widget.GetContext() )
        return widget.GetContext()->GetLineHeight( widget.GetProperty( EWidgetProperty_Font ) );
    return 16;
}

Int32
TextWidthOf( const CWidget& widget, const CString& text )
{
    Int32 width = 0, height = 0;
    if ( GUCEF_NULL != widget.GetContext() )
        widget.GetContext()->MeasureText( widget.GetProperty( EWidgetProperty_Font ), text, 0, width, height );
    return width;
}

}

CPopupMenu::CPopupMenu( void )
    : CWidget( "PopupMenu" )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CPopupMenu::~CPopupMenu()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CWidget*
CPopupMenu::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CPopupMenu();
}

/*-------------------------------------------------------------------------*/

bool
CPopupMenu::ShowAt( Int32 x, Int32 y )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetContext() )
        return false;
    SetIntProperty( EWidgetProperty_HoverIndex, -1 );
    return GetContext()->ShowPopup( this, x, y );
}

/*-------------------------------------------------------------------------*/

bool
CPopupMenu::Close( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetContext() || GetContext()->GetPopup() != this )
        return false;
    return GetContext()->ClosePopup();
}

/*-------------------------------------------------------------------------*/

CPopupMenu::TStringVector
CPopupMenu::GetItems( void ) const
{GUCEF_TRACE;

    return GetStringListProperty( EWidgetProperty_Items );
}

/*-------------------------------------------------------------------------*/

bool
CPopupMenu::SetItems( const TStringVector& items )
{GUCEF_TRACE;

    return SetStringListProperty( EWidgetProperty_Items, items );
}

/*-------------------------------------------------------------------------*/

bool
CPopupMenu::IsItemSelectable( Int32 index ) const
{GUCEF_TRACE;

    TStringVector items = GetItems();
    if ( index < 0 || index >= (Int32) items.size() || items[ index ] == "-" )
        return false;
    GUCEF::vector< Int32 > disabled = GetIntListProperty( EWidgetProperty_DisabledItems );
    for ( UInt32 i=0; i<(UInt32)disabled.size(); ++i )
        if ( disabled[ i ] == index )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
CPopupMenu::GetRowHeight( void ) const
{GUCEF_TRACE;

    return RowHeightOf( *this );
}

/*-------------------------------------------------------------------------*/

Int32
CPopupMenu::GetRowAt( Int32 localY ) const
{GUCEF_TRACE;

    Int32 rowHeight = GetRowHeight();
    if ( localY < 0 || rowHeight <= 0 )
        return -1;
    Int32 row = localY / rowHeight;
    return row < (Int32) GetItems().size() ? row : -1;
}

/*-------------------------------------------------------------------------*/

bool
CPopupMenu::GetNaturalSize( Int32& width, Int32& height ) const
{GUCEF_TRACE;

    TStringVector items = GetItems();
    width = 0;
    for ( UInt32 i=0; i<(UInt32)items.size(); ++i )
    {
        Int32 w = TextWidthOf( *this, items[ i ] );
        if ( w > width ) width = w;
    }
    width += 16;
    height = (Int32) items.size() * GetRowHeight();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPopupMenu::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( MouseMoveEvent == eventId )
    {
        Int32 row = GetRowAt( static_cast< CPointerEventData* >( &eventData )->localY );
        SetIntProperty( EWidgetProperty_HoverIndex, IsItemSelectable( row ) ? row : -1 );
        return true;
    }
    if ( MouseClickEvent == eventId && MOUSEBUTTON_LEFT == static_cast< CPointerEventData* >( &eventData )->button )
    {
        Int32 row = GetRowAt( static_cast< CPointerEventData* >( &eventData )->localY );
        if ( !IsItemSelectable( row ) )
            return true;
        CString item = GetItems()[ row ];
        Close();
        ActivateItem( row, item );
        return true;
    }
    if ( NavigationEvent == eventId )
    {
        TNavigation direction = static_cast< CNavigationEventData* >( &eventData )->direction;
        Int32 hover = GetIntProperty( EWidgetProperty_HoverIndex );
        Int32 count = (Int32) GetItems().size();
        if ( NAVIGATION_UP == direction || NAVIGATION_DOWN == direction )
        {
            Int32 step = NAVIGATION_UP == direction ? -1 : 1;
            for ( Int32 next = hover + step; next >= 0 && next < count; next += step )
            {
                if ( IsItemSelectable( next ) )
                {
                    SetIntProperty( EWidgetProperty_HoverIndex, next );
                    break;
                }
            }
            return true;
        }
        if ( NAVIGATION_ACCEPT == direction && IsItemSelectable( hover ) )
        {
            CString item = GetItems()[ hover ];
            Close();
            ActivateItem( hover, item );
            return true;
        }
        if ( NAVIGATION_CANCEL == direction )
            return Close();
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CString&
CPopupMenu::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CPopupMenu";
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
