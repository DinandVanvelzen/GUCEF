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

#include "gucefGUI_CListBox.h"

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

const CORE::CEvent CListbox::SelectionChangedEvent = "GUCEF::GUI::CListbox::SelectionChangedEvent";
const CORE::CEvent CListbox::ItemAddedEvent = "GUCEF::GUI::CListbox::ItemAddedEvent";
const CORE::CEvent CListbox::ItemRemovedEvent = "GUCEF::GUI::CListbox::ItemRemovedEvent";
const CORE::CEvent CListbox::ItemSelectedEvent = "GUCEF::GUI::CListbox::ItemSelectedEvent";
const CORE::CEvent CListbox::ItemDeselectedEvent = "GUCEF::GUI::CListbox::ItemDeselectedEvent";
const CORE::CEvent CListbox::ItemClickedEvent = "GUCEF::GUI::CListbox::ItemClickedEvent";
const CORE::CEvent CListbox::ItemDoubleClickedEvent = "GUCEF::GUI::CListbox::ItemDoubleClickedEvent";
const CORE::CEvent CListbox::ItemTripleClickedEvent = "GUCEF::GUI::CListbox::ItemTripleClickedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/


namespace {

/** The height of one row: the RowHeight property, else the line height of the font */
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

}

void
CListbox::RegisterEvents( void )
{GUCEF_TRACE;

    SelectionChangedEvent.Initialize();
    ItemAddedEvent.Initialize();
    ItemRemovedEvent.Initialize();
    ItemSelectedEvent.Initialize();
    ItemDeselectedEvent.Initialize();
    ItemClickedEvent.Initialize();
    ItemDoubleClickedEvent.Initialize();
    ItemTripleClickedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CListbox::CListbox( void )
    : CWidget( "Listbox" )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CListbox::~CListbox()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CWidget*
CListbox::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CListbox();
}

/*-------------------------------------------------------------------------*/

bool
CListbox::SetAllowMultiSelect( bool allowMultiselect )
{GUCEF_TRACE;

    return SetBoolProperty( EWidgetProperty_MultiSelect, allowMultiselect );
}

/*-------------------------------------------------------------------------*/

bool
CListbox::GetAllowMultiSelect( void ) const
{GUCEF_TRACE;

    return GetBoolProperty( EWidgetProperty_MultiSelect );
}

/*-------------------------------------------------------------------------*/

UInt32
CListbox::GetLineCount( void )
{GUCEF_TRACE;

    return (UInt32) GetItems().size();
}

/*-------------------------------------------------------------------------*/

bool
CListbox::Clear( void )
{GUCEF_TRACE;

    SetProperty( EWidgetProperty_SelectedIndices, CString::Empty );
    SetIntProperty( EWidgetProperty_SelectedIndex, -1 );
    SetIntProperty( EWidgetProperty_ScrollPosition, 0 );
    return SetProperty( EWidgetProperty_Items, CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CListbox::GetLineAtIndex( const UInt32 lineIndex ,
                          CString& lineText      ) const
{GUCEF_TRACE;

    TStringVector items = GetItems();
    if ( lineIndex >= (UInt32) items.size() )
        return false;
    lineText = items[ lineIndex ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CListbox::InsertLineAtIndex( const UInt32 lineIndex ,
                             const CString& text    )
{GUCEF_TRACE;

    TStringVector items = GetItems();
    if ( lineIndex > (UInt32) items.size() )
        return false;
    items.insert( items.begin() + lineIndex, text );
    bool result = SetItems( items );
    NotifyObservers( ItemAddedEvent );
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CListbox::AppendLine( const CString& text )
{GUCEF_TRACE;

    return InsertLineAtIndex( (UInt32) GetItems().size(), text );
}

/*-------------------------------------------------------------------------*/

bool
CListbox::GetText( CString& text ) const
{GUCEF_TRACE;

    text = GetProperty( EWidgetProperty_Items );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CListbox::SetLineSelection( const UInt32 lineIndex ,
                            const bool isSelected  )
{GUCEF_TRACE;

    if ( !GetAllowMultiSelect() )
        return SetSelectedIndex( isSelected ? (Int32) lineIndex : -1 );

    GUCEF::vector< Int32 > selected = GetIntListProperty( EWidgetProperty_SelectedIndices );
    GUCEF::vector< Int32 > result;
    for ( UInt32 i=0; i<(UInt32)selected.size(); ++i )
        if ( selected[ i ] != (Int32) lineIndex )
            result.push_back( selected[ i ] );
    if ( isSelected )
        result.push_back( (Int32) lineIndex );
    return SetIntListProperty( EWidgetProperty_SelectedIndices, result );
}

/*-------------------------------------------------------------------------*/

bool
CListbox::GetLineSelection( const UInt32 lineIndex ,
                            bool& isSelected       ) const
{GUCEF_TRACE;

    if ( lineIndex >= (UInt32) GetItems().size() )
        return false;
    isSelected = IsItemSelected( (Int32) lineIndex );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CListbox::GetFirstSelectedLine( CString& text ) const
{GUCEF_TRACE;

    TStringVector lines;
    if ( !GetSelectedLines( lines ) || lines.empty() )
        return false;
    text = lines.front();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CListbox::GetLastSelectedLine( CString& text ) const
{GUCEF_TRACE;

    TStringVector lines;
    if ( !GetSelectedLines( lines ) || lines.empty() )
        return false;
    text = lines.back();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CListbox::GetSelectedLines( TStringVector& selectedLines ) const
{GUCEF_TRACE;

    TStringVector items = GetItems();
    for ( UInt32 i=0; i<(UInt32)items.size(); ++i )
        if ( IsItemSelected( (Int32) i ) )
            selectedLines.push_back( items[ i ] );
    return true;
}

/*-------------------------------------------------------------------------*/

CListbox::TStringVector
CListbox::GetItems( void ) const
{GUCEF_TRACE;

    return GetStringListProperty( EWidgetProperty_Items );
}

/*-------------------------------------------------------------------------*/

bool
CListbox::SetItems( const TStringVector& items )
{GUCEF_TRACE;

    return SetStringListProperty( EWidgetProperty_Items, items );
}

/*-------------------------------------------------------------------------*/

Int32
CListbox::GetSelectedIndex( void ) const
{GUCEF_TRACE;

    return GetIntProperty( EWidgetProperty_SelectedIndex );
}

/*-------------------------------------------------------------------------*/

bool
CListbox::SetSelectedIndex( Int32 index )
{GUCEF_TRACE;

    return SetIntProperty( EWidgetProperty_SelectedIndex, index );
}

/*-------------------------------------------------------------------------*/

bool
CListbox::IsItemDisabled( Int32 index ) const
{GUCEF_TRACE;

    GUCEF::vector< Int32 > disabled = GetIntListProperty( EWidgetProperty_DisabledItems );
    for ( UInt32 i=0; i<(UInt32)disabled.size(); ++i )
        if ( disabled[ i ] == index )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CListbox::IsItemSelected( Int32 index ) const
{GUCEF_TRACE;

    if ( !GetAllowMultiSelect() )
        return index >= 0 && index == GetSelectedIndex();
    GUCEF::vector< Int32 > selected = GetIntListProperty( EWidgetProperty_SelectedIndices );
    for ( UInt32 i=0; i<(UInt32)selected.size(); ++i )
        if ( selected[ i ] == index )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

Int32
CListbox::GetRowHeight( void ) const
{GUCEF_TRACE;

    return RowHeightOf( *this );
}

/*-------------------------------------------------------------------------*/

Int32
CListbox::GetVisibleRowCount( void ) const
{GUCEF_TRACE;

    Int32 rowHeight = GetRowHeight();
    return rowHeight > 0 ? GetRect().height / rowHeight : 0;
}

/*-------------------------------------------------------------------------*/

Int32
CListbox::GetRowAt( Int32 localY ) const
{GUCEF_TRACE;

    Int32 rowHeight = GetRowHeight();
    if ( localY < 0 || rowHeight <= 0 || localY >= GetRect().height )
        return -1;
    Int32 row = GetIntProperty( EWidgetProperty_ScrollPosition ) + localY / rowHeight;
    return row < (Int32) GetItems().size() ? row : -1;
}

/*-------------------------------------------------------------------------*/

void
CListbox::ScrollToRow( Int32 index )
{GUCEF_TRACE;

    Int32 scroll = GetIntProperty( EWidgetProperty_ScrollPosition );
    Int32 visible = GetVisibleRowCount();
    if ( index < scroll )
        scroll = index;
    else
    if ( visible > 0 && index >= scroll + visible )
        scroll = index - visible + 1;
    if ( scroll < 0 )
        scroll = 0;
    SetIntProperty( EWidgetProperty_ScrollPosition, scroll );
}

/*-------------------------------------------------------------------------*/

void
CListbox::SelectByUser( Int32 index )
{GUCEF_TRACE;

    if ( index < 0 || index >= (Int32) GetItems().size() || IsItemDisabled( index ) )
        return;
    if ( GetAllowMultiSelect() )
    {
        bool wasSelected = IsItemSelected( index );
        CString oldValue = GetProperty( EWidgetProperty_SelectedIndices );
        SetLineSelection( (UInt32) index, !wasSelected );
        CValueChangedEventData eventData;
        eventData.property = EWidgetProperty_SelectedIndices;
        eventData.oldValue = oldValue;
        eventData.newValue = GetProperty( EWidgetProperty_SelectedIndices );
        DispatchEvent( ValueChangedEvent, eventData );
        NotifyObservers( wasSelected ? ItemDeselectedEvent : ItemSelectedEvent );
        return;
    }
    ChangeValueByUser( EWidgetProperty_SelectedIndex, CORE::Int32ToString( index ) );
}

/*-------------------------------------------------------------------------*/

void
CListbox::OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

    if ( property == EWidgetProperty_SelectedIndex || property == EWidgetProperty_SelectedIndices )
    {
        NotifyObservers( SelectionChangedEvent );
        if ( property == EWidgetProperty_SelectedIndex && CORE::StringToInt32( newValue, -1 ) >= 0 )
            NotifyObservers( ItemSelectedEvent );
    }
}

/*-------------------------------------------------------------------------*/

bool
CListbox::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( MouseMoveEvent == eventId )
    {
        CPointerEventData* pointer = static_cast< CPointerEventData* >( &eventData );
        SetIntProperty( EWidgetProperty_HoverIndex, GetRowAt( pointer->localY ) );
        return true;
    }
    if ( MouseFocusLostEvent == eventId )
    {
        SetIntProperty( EWidgetProperty_HoverIndex, -1 );
        return true;
    }
    if ( MouseClickEvent == eventId )
    {
        CPointerEventData* pointer = static_cast< CPointerEventData* >( &eventData );
        if ( MOUSEBUTTON_LEFT != pointer->button )
            return false;
        Int32 row = GetRowAt( pointer->localY );
        if ( row < 0 || IsItemDisabled( row ) )
            return false;
        NotifyObservers( 3 == pointer->clickCount ? ItemTripleClickedEvent : 2 == pointer->clickCount ? ItemDoubleClickedEvent : ItemClickedEvent );
        if ( 1 == pointer->clickCount )
            SelectByUser( row );
        else
        if ( 2 == pointer->clickCount )
            ActivateItem( row, GetItems()[ row ] );
        return true;
    }
    if ( MouseWheelEvent == eventId )
    {
        CWheelEventData* wheel = static_cast< CWheelEventData* >( &eventData );
        Int32 maxScroll = (Int32) GetItems().size() - GetVisibleRowCount();
        if ( maxScroll < 0 )
            maxScroll = 0;
        Int32 scroll = GetIntProperty( EWidgetProperty_ScrollPosition ) - wheel->delta;
        if ( scroll < 0 ) scroll = 0;
        if ( scroll > maxScroll ) scroll = maxScroll;

        // with nothing left to scroll the wheel is for what the list is in
        if ( scroll == GetIntProperty( EWidgetProperty_ScrollPosition ) )
            return false;
        return ChangeValueByUser( EWidgetProperty_ScrollPosition, CORE::Int32ToString( scroll ) );
    }
    if ( NavigationEvent == eventId )
    {
        CNavigationEventData* nav = static_cast< CNavigationEventData* >( &eventData );
        Int32 count = (Int32) GetItems().size();
        Int32 current = GetSelectedIndex();
        if ( NAVIGATION_UP == nav->direction || NAVIGATION_DOWN == nav->direction )
        {
            Int32 step = NAVIGATION_UP == nav->direction ? -1 : 1;
            for ( Int32 next = current + step; next >= 0 && next < count; next += step )
            {
                if ( !IsItemDisabled( next ) )
                {
                    SelectByUser( next );
                    ScrollToRow( next );
                    return true;
                }
            }
            return true;
        }
        if ( NAVIGATION_ACCEPT == nav->direction && current >= 0 && current < count && !IsItemDisabled( current ) )
        {
            ActivateItem( current, GetItems()[ current ] );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CString&
CListbox::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CListbox";
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
