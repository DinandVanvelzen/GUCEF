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

#include "gucefGUI_CTreeview.h"

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

CTreeview::SRow::SRow( void )
    : path()
    , label()
    , depth( 0 )
    , hasChildren( false )
    , expanded( false )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CTreeview::CTreeview( void )
    : CWidget( "Treeview" )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CTreeview::~CTreeview()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CWidget*
CTreeview::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CTreeview();
}

/*-------------------------------------------------------------------------*/

bool
CTreeview::IsExpanded( const CString& path ) const
{GUCEF_TRACE;

    TStringVector expanded = GetStringListProperty( EWidgetProperty_ExpandedItems );
    for ( UInt32 i=0; i<(UInt32)expanded.size(); ++i )
        if ( expanded[ i ] == path )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTreeview::SetExpanded( const CString& path, bool expanded )
{GUCEF_TRACE;

    TStringVector current = GetStringListProperty( EWidgetProperty_ExpandedItems );
    TStringVector result;
    for ( UInt32 i=0; i<(UInt32)current.size(); ++i )
        if ( current[ i ] != path )
            result.push_back( current[ i ] );
    if ( expanded )
        result.push_back( path );
    return SetStringListProperty( EWidgetProperty_ExpandedItems, result );
}

/*-------------------------------------------------------------------------*/

CString
CTreeview::GetSelectedItem( void ) const
{GUCEF_TRACE;

    return GetProperty( EWidgetProperty_SelectedItem );
}

/*-------------------------------------------------------------------------*/

bool
CTreeview::SetSelectedItem( const CString& path )
{GUCEF_TRACE;

    return SetProperty( EWidgetProperty_SelectedItem, path );
}

/*-------------------------------------------------------------------------*/

Int32
CTreeview::GetRowHeight( void ) const
{GUCEF_TRACE;

    return RowHeightOf( *this );
}

/*-------------------------------------------------------------------------*/

CTreeview::TRowVector
CTreeview::GetVisibleRows( void ) const
{GUCEF_TRACE;

    // every node in the order it first appears, parents before children
    TStringVector items = GetStringListProperty( EWidgetProperty_Items );
    TStringVector nodes;
    GUCEF::set< CString > known;
    for ( UInt32 i=0; i<(UInt32)items.size(); ++i )
    {
        CString::StringVector parts = items[ i ].ParseElements( '/', false );
        CString path;
        for ( UInt32 p=0; p<(UInt32)parts.size(); ++p )
        {
            path = 0 == p ? parts[ p ] : path + '/' + parts[ p ];
            if ( known.insert( path ).second )
                nodes.push_back( path );
        }
    }

    // a node is shown when every ancestor is open; nodes are grouped below their parent
    TRowVector rows;
    GUCEF::vector< CString > stack;
    struct SLocal
    {
        static void Add( const CTreeview& tree, const TStringVector& all, const CString& parent, Int32 depth, TRowVector& out )
        {
            for ( UInt32 i=0; i<(UInt32)all.size(); ++i )
            {
                const CString& node = all[ i ];
                Int32 slash = node.HasChar( '/', false );
                CString nodeParent = slash >= 0 ? node.SubstrFromRange( 0, (UInt32) slash ) : CString::Empty;
                if ( nodeParent != parent )
                    continue;
                TRow row;
                row.path = node;
                row.label = slash >= 0 ? node.SubstrFromRange( (UInt32) slash + 1, node.Length() ) : node;
                row.depth = depth;
                CString prefix = node + '/';
                for ( UInt32 c=0; c<(UInt32)all.size() && !row.hasChildren; ++c )
                    if ( all[ c ].Length() > prefix.Length() && all[ c ].SubstrFromRange( 0, prefix.Length() ) == prefix )
                        row.hasChildren = true;
                row.expanded = row.hasChildren && tree.IsExpanded( node );
                out.push_back( row );
                if ( row.expanded )
                    Add( tree, all, node, depth + 1, out );
            }
        }
    };
    SLocal::Add( *this, nodes, CString::Empty, 0, rows );
    return rows;
}

/*-------------------------------------------------------------------------*/

Int32
CTreeview::GetRowAt( Int32 localY ) const
{GUCEF_TRACE;

    Int32 rowHeight = GetRowHeight();
    if ( localY < 0 || rowHeight <= 0 )
        return -1;
    Int32 row = GetIntProperty( EWidgetProperty_ScrollPosition ) + localY / rowHeight;
    return row < (Int32) GetVisibleRows().size() ? row : -1;
}

/*-------------------------------------------------------------------------*/

bool
CTreeview::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( MouseMoveEvent == eventId )
    {
        SetIntProperty( EWidgetProperty_HoverIndex, GetRowAt( static_cast< CPointerEventData* >( &eventData )->localY ) );
        return true;
    }
    if ( MouseFocusLostEvent == eventId )
    {
        SetIntProperty( EWidgetProperty_HoverIndex, -1 );
        return true;
    }
    TRowVector rows = GetVisibleRows();
    if ( MouseClickEvent == eventId )
    {
        CPointerEventData* pointer = static_cast< CPointerEventData* >( &eventData );
        if ( MOUSEBUTTON_LEFT != pointer->button )
            return false;
        Int32 index = GetRowAt( pointer->localY );
        if ( index < 0 )
            return false;
        const TRow& row = rows[ index ];
        Int32 indent = GetIntProperty( EWidgetProperty_Indent );
        bool onToggle = pointer->localX >= row.depth * indent && pointer->localX < ( row.depth + 1 ) * indent;
        if ( row.hasChildren && ( onToggle || 2 == pointer->clickCount ) )
            SetExpanded( row.path, !row.expanded );
        if ( !onToggle )
        {
            if ( 2 == pointer->clickCount )
                ActivateItem( index, row.path );
            else
                ChangeValueByUser( EWidgetProperty_SelectedItem, row.path );
        }
        return true;
    }
    if ( MouseWheelEvent == eventId )
    {
        Int32 visible = GetRowHeight() > 0 ? GetRect().height / GetRowHeight() : 0;
        Int32 maxScroll = (Int32) rows.size() - visible;
        if ( maxScroll < 0 ) maxScroll = 0;
        Int32 scroll = GetIntProperty( EWidgetProperty_ScrollPosition ) - static_cast< CWheelEventData* >( &eventData )->delta;
        if ( scroll < 0 ) scroll = 0;
        if ( scroll > maxScroll ) scroll = maxScroll;

        // with nothing left to scroll the wheel is for what the tree is in
        if ( scroll == GetIntProperty( EWidgetProperty_ScrollPosition ) )
            return false;
        return ChangeValueByUser( EWidgetProperty_ScrollPosition, CORE::Int32ToString( scroll ) );
    }
    if ( NavigationEvent == eventId )
    {
        TNavigation direction = static_cast< CNavigationEventData* >( &eventData )->direction;
        Int32 current = -1;
        CString selected = GetSelectedItem();
        for ( UInt32 i=0; i<(UInt32)rows.size(); ++i )
            if ( rows[ i ].path == selected )
                current = (Int32) i;
        if ( NAVIGATION_UP == direction && current > 0 )
            return ChangeValueByUser( EWidgetProperty_SelectedItem, rows[ current - 1 ].path );
        if ( NAVIGATION_DOWN == direction && current + 1 < (Int32) rows.size() )
            return ChangeValueByUser( EWidgetProperty_SelectedItem, rows[ current + 1 ].path );
        if ( current < 0 )
            return false;
        if ( NAVIGATION_RIGHT == direction && rows[ current ].hasChildren )
            return SetExpanded( rows[ current ].path, true );
        if ( NAVIGATION_LEFT == direction && rows[ current ].expanded )
            return SetExpanded( rows[ current ].path, false );
        if ( NAVIGATION_ACCEPT == direction )
        {
            ActivateItem( current, rows[ current ].path );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CString&
CTreeview::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CTreeview";
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
