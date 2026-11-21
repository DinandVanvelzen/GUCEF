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

#include <algorithm>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */

#ifndef GUCEF_GUI_CPOPUPMENU_H
#include "gucefGUI_CPopupMenu.h"
#define GUCEF_GUI_CPOPUPMENU_H
#endif /* GUCEF_GUI_CPOPUPMENU_H ? */

#ifndef GUCEF_GUI_CTREEVIEW_H
#include "gucefGUI_CTreeview.h"
#define GUCEF_GUI_CTREEVIEW_H
#endif /* GUCEF_GUI_CTREEVIEW_H ? */

#ifndef GUCEF_GUI_CSCROLLPANEL_H
#include "gucefGUI_CScrollPanel.h"
#define GUCEF_GUI_CSCROLLPANEL_H
#endif /* GUCEF_GUI_CSCROLLPANEL_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_GUI_CCHECKBOX_H
#include "gucefGUI_CCheckbox.h"
#define GUCEF_GUI_CCHECKBOX_H
#endif /* GUCEF_GUI_CCHECKBOX_H ? */

#ifndef GUCEF_GUI_CEDITBOX_H
#include "gucefGUI_CEditbox.h"
#define GUCEF_GUI_CEDITBOX_H
#endif /* GUCEF_GUI_CEDITBOX_H ? */

#ifndef GUCEF_GUI_CIMAGEFRAME_H
#include "gucefGUI_CImageFrame.h"
#define GUCEF_GUI_CIMAGEFRAME_H
#endif /* GUCEF_GUI_CIMAGEFRAME_H ? */

#include "gucefGUI_CContentBrowserForm.h"

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

const CORE::CEvent CContentBrowserForm::SelectionChangedEvent = "GUCEF::GUI::CContentBrowserForm::SelectionChangedEvent";
const CORE::CEvent CContentBrowserForm::ItemActivatedEvent = "GUCEF::GUI::CContentBrowserForm::ItemActivatedEvent";
const CORE::CEvent CContentBrowserForm::FolderChangedEvent = "GUCEF::GUI::CContentBrowserForm::FolderChangedEvent";
const CORE::CEvent CContentBrowserForm::FolderActivatedEvent = "GUCEF::GUI::CContentBrowserForm::FolderActivatedEvent";
const CORE::CEvent CContentBrowserForm::FilterChangedEvent = "GUCEF::GUI::CContentBrowserForm::FilterChangedEvent";
const CORE::CEvent CContentBrowserForm::ViewSwitchedEvent = "GUCEF::GUI::CContentBrowserForm::ViewSwitchedEvent";
const CORE::CEvent CContentBrowserForm::ItemActionEvent = "GUCEF::GUI::CContentBrowserForm::ItemActionEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const AllTypesText = "All types";

/** The colours of the tiles */
const CColour TileColour( 45, 48, 56, 255 );
const CColour TileSelectedColour( 52, 98, 168, 255 );

/** The most pages of directory entries the tree and the list read per folder: a guard against a runaway listing */
const UInt32 MaxEntriesPerFolder = 200000;

bool
IsSeparator( UInt32 c )
{
    return '/' == c || '\\' == c;
}

CString
JoinPath( const CString& directory, const CString& name )
{
    if ( directory.IsNULLOrEmpty() )
        return name;
    if ( IsSeparator( directory[ directory.Length()-1 ] ) )
        return directory + name;
    return directory + "/" + name;
}

/** The directory above a path; empty when the path is a top of the file system without one ("" or a drive) */
CString
ParentOf( const CString& path )
{
    if ( path.IsNULLOrEmpty() || "/" == path )
        return CString::Empty;
    CString trimmed = path;
    while ( trimmed.Length() > 1 && IsSeparator( trimmed[ trimmed.Length()-1 ] ) )
        trimmed = trimmed.CutChars( 1, false );
    Int32 last = -1;
    for ( Int32 i=(Int32)trimmed.Length()-1; i>=0; --i )
    {
        if ( IsSeparator( trimmed[ i ] ) )
        {
            last = i;
            break;
        }
    }
    if ( last < 0 )
        return CString::Empty;
    if ( 0 == last )
        return "/";
    CString parent = trimmed.SubstrToIndex( (UInt32) last, true );
    if ( 2 == parent.Length() && ':' == parent[ 1 ] )
        parent += "\\";             // "C:" is the drive, "C:\" its top
    return parent;
}

CString
NameOf( const CString& path )
{
    CString trimmed = path;
    while ( trimmed.Length() > 1 && IsSeparator( trimmed[ trimmed.Length()-1 ] ) )
        trimmed = trimmed.CutChars( 1, false );
    Int32 last = -1;
    for ( Int32 i=(Int32)trimmed.Length()-1; i>=0; --i )
    {
        if ( IsSeparator( trimmed[ i ] ) )
        {
            last = i;
            break;
        }
    }
    return last < 0 ? trimmed : trimmed.CutChars( (UInt32) last + 1, true );
}

/** The same folder written one way: slashes, no slash at the end unless it is the top */
CString
NormalizeDirectory( const CString& path )
{
    CString result = path.ReplaceChar( '\\', '/' );
    while ( result.Length() > 1 && '/' == result[ result.Length()-1 ] )
        result = result.CutChars( 1, false );
    return result;
}

CString
ExtensionOf( const CString& name )
{
    Int32 dot = name.HasChar( '.', false );
    return dot > 0 ? name.CutChars( (UInt32) dot + 1, true ) : CString::Empty;
}

CString
TwoDigits( Int32 value )
{
    return value < 10 ? "0" + CORE::Int32ToString( value ) : CORE::Int32ToString( value );
}

CString
FormatSize( UInt64 size )
{
    if ( size < 1024 )
        return CORE::UInt64ToString( size ) + " B";
    if ( size < 1024 * 1024 )
        return CORE::UInt64ToString( ( size + 512 ) / 1024 ) + " KB";
    if ( size < (UInt64) 1024 * 1024 * 1024 )
        return CORE::UInt64ToString( ( size + 512 * 1024 ) / ( 1024 * 1024 ) ) + " MB";
    return CORE::UInt64ToString( ( size + (UInt64) 512 * 1024 * 1024 ) / ( (UInt64) 1024 * 1024 * 1024 ) ) + " GB";
}

/** What the tree shows of the name of a root: no separators, which are the levels of the tree */
CString
TreeLabel( const CString& name )
{
    return name.ReplaceChar( '/', '-' ).ReplaceChar( '\\', '-' );
}

/** Folders first, then by the column; equal items keep their order (a stable sort) */
class CItemOrder
{
    public:

    CItemOrder( Int32 column, bool ascending ) : m_column( column ), m_ascending( ascending ) {}

    template< class TItem >
    bool operator()( const TItem& a, const TItem& b ) const
    {
        if ( a.isDirectory != b.isDirectory )
            return a.isDirectory;
        if ( 0 == m_column && a.rank != b.rank )
            return m_ascending ? a.rank < b.rank : a.rank > b.rank;
        Int32 compared = 0;
        if ( 1 == m_column )
        {
            const CString typeA = a.typeName.Lowercase(), typeB = b.typeName.Lowercase();
            compared = typeA < typeB ? -1 : ( typeB < typeA ? 1 : 0 );
        }
        else
        if ( 2 == m_column )
            compared = a.size < b.size ? -1 : ( b.size < a.size ? 1 : 0 );
        else
        if ( 3 == m_column )
            compared = a.modified < b.modified ? -1 : ( b.modified < a.modified ? 1 : 0 );
        if ( 0 == compared )
        {
            const CString nameA = a.name.Lowercase(), nameB = b.name.Lowercase();
            compared = nameA < nameB ? -1 : ( nameB < nameA ? 1 : 0 );
        }
        return m_ascending ? compared < 0 : compared > 0;
    }

    private:

    Int32 m_column;
    bool m_ascending;
};

/** The order of the tree: the rank of the root first, then the names */
class CRankNameOrder
{
    public:

    explicit CRankNameOrder( GUCEF::map< CString, Int32 >* ranks ) : m_ranks( ranks ) {}

    bool operator()( const CString& a, const CString& b ) const
    {
        const Int32 rankA = (*m_ranks)[ a ];
        const Int32 rankB = (*m_ranks)[ b ];
        return rankA != rankB ? rankA < rankB : a < b;
    }

    private:

    GUCEF::map< CString, Int32 >* m_ranks;
};

CString
TileName( UInt32 slot )
{
    return "Tile_" + CORE::UInt32ToString( slot );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CContentBrowserForm::SItem::SItem( void )
    : name()
    , path()
    , isDirectory( false )
    , hasSize( false )
    , size( 0 )
    , hasModified( false )
    , modified()
    , hidden( false )
    , sidecar( false )
    , rank( 0 )
    , typeName()
    , iconName()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::RegisterEvents( void )
{GUCEF_TRACE;

    SelectionChangedEvent.Initialize();
    ItemActivatedEvent.Initialize();
    FolderChangedEvent.Initialize();
    FolderActivatedEvent.Initialize();
    FilterChangedEvent.Initialize();
    ViewSwitchedEvent.Initialize();
    ItemActionEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CContentBrowserForm::CContentBrowserForm( void )
    : CFormEx()
    , m_roots()
    , m_currentRoot( 0 )
    , m_currentDirectory()
    , m_back()
    , m_forward()
    , m_items()
    , m_listed()
    , m_sortColumn( 0 )
    , m_sortAscending( true )
    , m_filter()
    , m_filterPattern()
    , m_typeFilter()
    , m_typeChoices()
    , m_showHidden( false )
    , m_showSidecars( false )
    , m_viewMode( VIEWMODE_DETAILS )
    , m_thumbnailSize( DEFAULT_THUMBNAIL_SIZE )
    , m_selection()
    , m_anchorItem( -1 )
    , m_tree()
    , m_tiles()
    , m_thumbnails()
    , m_requested()
    , m_requestCount( 0 )
    , m_provider( GUCEF_NULL )
    , m_resolver( GUCEF_NULL )
    , m_lastScroll( -1 )
    , m_lastViewportWidth( 0 )
    , m_lastViewportHeight( 0 )
    , m_tilesPerRow( 1 )
    , m_tilesDirty( true )
    , m_updatingWidgets( false )
    , m_frameSubscribed( false )
    , m_toolbarMode( -1 )
    , m_toolbarRowHeight( 0 )
    , m_alternateRoot( -1 )
    , m_normalRoot( 0 )
    , m_normalDirectory()
    , m_normalLabel( "Files" )
    , m_alternateLabel( "Alternate" )
    , m_itemActions()
    , m_itemMenu( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CContentBrowserForm::CContentBrowserForm( CIGUIContext* context )
    : CFormEx( context )
    , m_roots()
    , m_currentRoot( 0 )
    , m_currentDirectory()
    , m_back()
    , m_forward()
    , m_items()
    , m_listed()
    , m_sortColumn( 0 )
    , m_sortAscending( true )
    , m_filter()
    , m_filterPattern()
    , m_typeFilter()
    , m_typeChoices()
    , m_showHidden( false )
    , m_showSidecars( false )
    , m_viewMode( VIEWMODE_DETAILS )
    , m_thumbnailSize( DEFAULT_THUMBNAIL_SIZE )
    , m_selection()
    , m_anchorItem( -1 )
    , m_tree()
    , m_tiles()
    , m_thumbnails()
    , m_requested()
    , m_requestCount( 0 )
    , m_provider( GUCEF_NULL )
    , m_resolver( GUCEF_NULL )
    , m_lastScroll( -1 )
    , m_lastViewportWidth( 0 )
    , m_lastViewportHeight( 0 )
    , m_tilesPerRow( 1 )
    , m_tilesDirty( true )
    , m_updatingWidgets( false )
    , m_frameSubscribed( false )
    , m_toolbarMode( -1 )
    , m_toolbarRowHeight( 0 )
    , m_alternateRoot( -1 )
    , m_normalRoot( 0 )
    , m_normalDirectory()
    , m_normalLabel( "Files" )
    , m_alternateLabel( "Alternate" )
    , m_itemActions()
    , m_itemMenu( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CContentBrowserForm::~CContentBrowserForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    m_provider = GUCEF_NULL;
    m_resolver = GUCEF_NULL;
    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != m_itemMenu && GUCEF_NULL != root && GUCEF_NULL != root->GetContext() )
        root->GetContext()->DestroyWidget( m_itemMenu );
    m_itemMenu = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title )
{GUCEF_TRACE;

    CORE::CDataNode* content = GUCEF_NULL;
    if ( asWindow )
    {
        CORE::CDataNode* window = CFormLayout::AddWidgetNode( layout, "Window", "ContentBrowserForm", "0.05,0 0.05,0", "0.9,0 0.9,0" );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_Title, title );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_TitleHeight, "28" );
        content = CFormLayout::AddWidgetNode( *window, "Panel", "Content", "0,0 0,28", "1,0 1,-28" );
    }
    else
        content = CFormLayout::AddWidgetNode( layout, "Panel", "ContentBrowserForm", "0,0 0,0", "1,0 1,0" );

    // the tool bar, in two rows so nothing depends on the room beside the other controls: back, forward and up with the view
    // at the right, then the filter and the type filter in proportions of the full width
    struct SButtonSpec { const char* name; const char* text; const char* tooltip; const char* position; const char* size; };
    static const SButtonSpec buttons[] = { { "Back", "Back", "Back to the folder before", "0,6 0,4", "0,56 0,28" },
                                           { "Forward", "Forward", "Forward again", "0,66 0,4", "0,76 0,28" },
                                           { "Up", "Up", "To the folder that holds this one", "0,146 0,4", "0,44 0,28" } };
    for ( UInt32 i=0; i<sizeof( buttons ) / sizeof( buttons[ 0 ] ); ++i )
    {
        CORE::CDataNode* button = CFormLayout::AddWidgetNode( *content, "Button", buttons[ i ].name, buttons[ i ].position, buttons[ i ].size );
        CFormLayout::SetWidgetNodeProperty( *button, EWidgetProperty_Text, buttons[ i ].text );
        CFormLayout::SetWidgetNodeProperty( *button, EWidgetProperty_Tooltip, buttons[ i ].tooltip );
    }
    CORE::CDataNode* filter = CFormLayout::AddWidgetNode( *content, "Editbox", "Filter", "0,6 0,38", "0.6,-9 0,28" );
    CFormLayout::SetWidgetNodeProperty( *filter, EWidgetProperty_Tooltip, "Filter the names: a glob such as *.png, or a part of a name" );
    CORE::CDataNode* types = CFormLayout::AddWidgetNode( *content, "Combobox", "TypeFilter", "0.6,3 0,38", "0.4,-9 0,28" );
    CFormLayout::SetWidgetNodeProperty( *types, EWidgetProperty_Items, AllTypesText );
    CFormLayout::SetWidgetNodeProperty( *types, EWidgetProperty_SelectedIndex, "0" );
    CFormLayout::SetWidgetNodeProperty( *types, EWidgetProperty_DropDownRows, "10" );
    CORE::CDataNode* viewSwitch = CFormLayout::AddWidgetNode( *content, "Combobox", "ViewSwitch", "0,198 0,4", "0,170 0,28" );
    CFormLayout::SetWidgetNodeProperty( *viewSwitch, EWidgetProperty_Visible, "false" );
    CFormLayout::SetWidgetNodeProperty( *viewSwitch, EWidgetProperty_Tooltip, "Show the content as files, or grouped by function (scenes, models, textures, ...): the resources of the pack" );
    CFormLayout::SetWidgetNodeProperty( *viewSwitch, EWidgetProperty_DropDownRows, "2" );
    CORE::CDataNode* sidecars = CFormLayout::AddWidgetNode( *content, "Checkbox", "Sidecars", "1,-236 0,4", "0,108 0,28" );
    CFormLayout::SetWidgetNodeProperty( *sidecars, EWidgetProperty_Text, "Metadata" );
    CFormLayout::SetWidgetNodeProperty( *sidecars, EWidgetProperty_Tooltip, "Show the metadata files that belong to an asset (Unity .meta, Godot .import, Unreal .uexp)" );
    CORE::CDataNode* view = CFormLayout::AddWidgetNode( *content, "Checkbox", "Thumbnails", "1,-124 0,4", "0,118 0,28" );
    CFormLayout::SetWidgetNodeProperty( *view, EWidgetProperty_Text, "Thumbnails" );

    // the folders at the left, the items of the open folder at the right: a list, or tiles
    CFormLayout::AddWidgetNode( *content, "Treeview", "Folders", "0,0 0,72", "0.25,0 1,-72" );
    CORE::CDataNode* items = CFormLayout::AddWidgetNode( *content, "GridView", "Items", "0.25,6 0,72", "0.75,-6 1,-72" );
    CFormLayout::SetWidgetNodeProperty( *items, EWidgetProperty_Columns, "Name\nType\nSize\nModified" );
    CFormLayout::SetWidgetNodeProperty( *items, EWidgetProperty_MultiSelect, "true" );
    CFormLayout::SetWidgetNodeProperty( *items, EWidgetProperty_FitColumns, "true" );
    CFormLayout::SetWidgetNodeProperty( *items, EWidgetProperty_TextOverflow, "Ellipsis" );
    CORE::CDataNode* tiles = CFormLayout::AddWidgetNode( *content, "ScrollPanel", "Thumbs", "0.25,6 0,72", "0.75,-6 1,-72" );
    CFormLayout::SetWidgetNodeProperty( *tiles, EWidgetProperty_Visible, "false" );
    CFormLayout::SetWidgetNodeProperty( *tiles, EWidgetProperty_Focusable, "true" );
    CFormLayout::SetWidgetNodeProperty( *tiles, EWidgetProperty_BackgroundColour, "30,32,38,255" );
}

/*-------------------------------------------------------------------------*/

bool
CContentBrowserForm::Build( const CString& title )
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
CContentBrowserForm::Build( CWidget* placeholder )
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
CContentBrowserForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();

    CWidget* widget = GetWidget( "Back" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CContentBrowserForm::OnBackActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Forward" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CContentBrowserForm::OnForwardActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Up" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CContentBrowserForm::OnUpActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Filter" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CContentBrowserForm::OnFilterTextChanged );
        SubscribeTo( widget, CEditbox::TextChangedEvent, callback );
    }
    widget = GetWidget( "TypeFilter" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CContentBrowserForm::OnTypeFilterValueChanged );
        SubscribeTo( widget, CWidget::ValueChangedEvent, callback );
    }
    widget = GetWidget( "ViewSwitch" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CContentBrowserForm::OnViewSwitchValueChanged );
        SubscribeTo( widget, CWidget::ValueChangedEvent, callback );
    }
    widget = GetWidget( "Thumbnails" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback checkedCallback( this, &CContentBrowserForm::OnThumbnailsChecked );
        SubscribeTo( widget, CCheckbox::BoxCheckedEvent, checkedCallback );
        TEventCallback uncheckedCallback( this, &CContentBrowserForm::OnThumbnailsUnchecked );
        SubscribeTo( widget, CCheckbox::BoxUncheckedEvent, uncheckedCallback );
    }
    widget = GetWidget( "Sidecars" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback checkedCallback( this, &CContentBrowserForm::OnSidecarsChecked );
        SubscribeTo( widget, CCheckbox::BoxCheckedEvent, checkedCallback );
        TEventCallback uncheckedCallback( this, &CContentBrowserForm::OnSidecarsUnchecked );
        SubscribeTo( widget, CCheckbox::BoxUncheckedEvent, uncheckedCallback );
    }
    widget = GetWidget( "Folders" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback valueCallback( this, &CContentBrowserForm::OnTreeValueChanged );
        SubscribeTo( widget, CWidget::ValueChangedEvent, valueCallback );
        TEventCallback propertyCallback( this, &CContentBrowserForm::OnTreePropertyChanged );
        SubscribeTo( widget, CWidget::PropertyChangedEvent, propertyCallback );
    }
    widget = GetWidget( "Items" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback selectionCallback( this, &CContentBrowserForm::OnItemsSelectionChanged );
        SubscribeTo( widget, CGridView::SelectionChangedEvent, selectionCallback );
        TEventCallback columnCallback( this, &CContentBrowserForm::OnItemsColumnClicked );
        SubscribeTo( widget, CGridView::ColumnClickedEvent, columnCallback );
        TEventCallback itemCallback( this, &CContentBrowserForm::OnItemsItemActivated );
        SubscribeTo( widget, CWidget::ItemActivatedEvent, itemCallback );
        // the actions of the items as the context menu of the list (nothing is shown while there are none)
        CGridView* grid = CWidget::As< CGridView >( widget );
        if ( GUCEF_NULL != grid )
            grid->SetContextMenuProvider( this );
    }
    widget = GetWidget( "Thumbs" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback clickCallback( this, &CContentBrowserForm::OnTilesMouseClick );
        SubscribeTo( widget, CWidget::MouseClickEvent, clickCallback );
        TEventCallback keyCallback( this, &CContentBrowserForm::OnTilesKeyDown );
        SubscribeTo( widget, CWidget::KeyDownEvent, keyCallback );
    }
    widget = GetRootWidget();
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CContentBrowserForm::OnRootKeyDown );
        SubscribeTo( widget, CWidget::KeyDownEvent, callback );
    }
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_frameSubscribed )
    {
        m_frameSubscribed = true;
        TEventCallback callback( this, &CContentBrowserForm::OnFrameStarted );
        SubscribeTo( context, CGUIContext::FrameStartedEvent, callback );
    }

    // a layout that was loaded again has new widgets: what the form knows goes into them
    m_toolbarMode = -1;
    m_tiles.clear();
    m_tilesDirty = true;
    m_updatingWidgets = true;
    widget = GetWidget( "Thumbnails" );
    if ( GUCEF_NULL != widget )
        widget->SetBoolProperty( EWidgetProperty_Checked, VIEWMODE_THUMBNAILS == m_viewMode );
    widget = GetWidget( "Sidecars" );
    if ( GUCEF_NULL != widget )
        widget->SetBoolProperty( EWidgetProperty_Checked, m_showSidecars );
    widget = GetWidget( "Filter" );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( EWidgetProperty_Text, m_filter );
    m_updatingWidgets = false;
    ShowViewMode();
    ResetTree();
    if ( !m_roots.empty() )
        GoTo( m_currentRoot, m_currentDirectory, false );
    UpdateButtons();
}

/*-------------------------------------------------------------------------*/

bool
CContentBrowserForm::AddRoot( const CString& displayName, CORE::TFileSystemAccessPtr access, const CString& initialDirectory )
{GUCEF_TRACE;

    if ( displayName.IsNULLOrEmpty() || access.IsNULL() )
        return false;
    for ( UInt32 i=0; i<(UInt32)m_roots.size(); ++i )
        if ( m_roots[ i ].name == displayName )
            return false;
    SRoot root;
    root.name = displayName;
    root.access = access;
    root.initialDirectory = initialDirectory;
    root.topDirectory = NormalizeDirectory( access->GetRootPath() );
    root.order = GUCEF_NULL;
    m_roots.push_back( root );
    if ( 1 == m_roots.size() )
    {
        m_currentRoot = 0;
        m_currentDirectory = initialDirectory.IsNULLOrEmpty() ? root.topDirectory : initialDirectory;
    }
    ResetTree();
    if ( 1 == m_roots.size() && GUCEF_NULL != GetRootWidget() )
        GoTo( 0, m_currentDirectory, false );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CContentBrowserForm::GetRootCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_roots.size();
}

/*-------------------------------------------------------------------------*/

const CString&
CContentBrowserForm::GetRootName( UInt32 index ) const
{GUCEF_TRACE;

    return index < (UInt32) m_roots.size() ? m_roots[ index ].name : CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::SetThumbnailProvider( CIThumbnailProvider* provider )
{GUCEF_TRACE;

    if ( provider == m_provider )
        return;
    if ( GUCEF_NULL != m_provider )
        UnsubscribeFrom( m_provider );
    m_provider = provider;
    m_requested.clear();
    if ( GUCEF_NULL != m_provider )
    {
        TEventCallback callback( this, &CContentBrowserForm::OnThumbnailReady );
        SubscribeTo( m_provider, CIThumbnailProvider::ThumbnailReadyEvent, callback );
    }
    m_tilesDirty = true;
    Update( 0 );
}

/*-------------------------------------------------------------------------*/

CIThumbnailProvider*
CContentBrowserForm::GetThumbnailProvider( void ) const
{GUCEF_TRACE;

    return m_provider;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::SetContentTypeResolver( CIContentTypeResolver* resolver )
{GUCEF_TRACE;

    m_resolver = resolver;
    Refresh();
}

/*-------------------------------------------------------------------------*/

CIContentTypeResolver*
CContentBrowserForm::GetContentTypeResolver( void ) const
{GUCEF_TRACE;

    return m_resolver;
}

/*-------------------------------------------------------------------------*/

CORE::IFileSystemAccess*
CContentBrowserForm::GetCurrentAccess( void ) const
{GUCEF_TRACE;

    if ( m_currentRoot >= (UInt32) m_roots.size() || m_roots[ m_currentRoot ].access.IsNULL() )
        return GUCEF_NULL;
    return const_cast< CORE::IFileSystemAccess* >( m_roots[ m_currentRoot ].access.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::GoTo( UInt32 root, const CString& directory, bool remember )
{GUCEF_TRACE;

    if ( root >= (UInt32) m_roots.size() )
        return;
    if ( remember && ( root != m_currentRoot || directory != m_currentDirectory ) )
    {
        SLocation here;
        here.root = m_currentRoot;
        here.directory = m_currentDirectory;
        m_back.push_back( here );
        m_forward.clear();
    }

    // what was asked of the provider for the folder that is left is not needed any more
    if ( GUCEF_NULL != m_provider )
    {
        for ( GUCEF::set< CString >::const_iterator r = m_requested.begin(); r != m_requested.end(); ++r )
            if ( m_thumbnails.find( *r ) == m_thumbnails.end() )
                m_provider->CancelThumbnail( *r, m_thumbnailSize );
    }
    m_requested.clear();

    m_currentRoot = root;
    m_currentDirectory = directory;
    m_selection.clear();
    m_anchorItem = -1;
    ReadDirectory();
    RevealInTree( root, directory );
    UpdateButtons();
    UpdateViewSwitch();
    // the tree and the list share the room differently in the alternate view
    if ( m_alternateRoot >= 0 )
        m_toolbarMode = -1;
    CORE::TCloneableString eventData( m_currentDirectory );
    NotifyObservers( FolderChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

bool
CContentBrowserForm::NavigateTo( const CString& rootName, const CString& directory )
{GUCEF_TRACE;

    UInt32 root = m_currentRoot;
    if ( !rootName.IsNULLOrEmpty() )
    {
        bool found = false;
        for ( UInt32 i=0; i<(UInt32)m_roots.size(); ++i )
        {
            if ( m_roots[ i ].name == rootName )
            {
                root = i;
                found = true;
            }
        }
        if ( !found )
            return false;
    }
    if ( root >= (UInt32) m_roots.size() )
        return false;
    const CString target = directory.IsNULLOrEmpty() ? ( m_roots[ root ].initialDirectory.IsNULLOrEmpty() ? m_roots[ root ].topDirectory : m_roots[ root ].initialDirectory ) : directory;
    CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
    if ( CORE::FILESYSTEMACCESS_SUCCESS != m_roots[ root ].access->TryGetEntryType( target, type ) || CORE::FILESYSTEMENTRYTYPE_DIRECTORY != type )
        return false;
    GoTo( root, target, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CContentBrowserForm::GoBack( void )
{GUCEF_TRACE;

    if ( m_back.empty() )
        return false;
    SLocation here;
    here.root = m_currentRoot;
    here.directory = m_currentDirectory;
    SLocation there = m_back.back();
    m_back.pop_back();
    m_forward.push_back( here );
    GoTo( there.root, there.directory, false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CContentBrowserForm::GoForward( void )
{GUCEF_TRACE;

    if ( m_forward.empty() )
        return false;
    SLocation here;
    here.root = m_currentRoot;
    here.directory = m_currentDirectory;
    SLocation there = m_forward.back();
    m_forward.pop_back();
    m_back.push_back( here );
    GoTo( there.root, there.directory, false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CContentBrowserForm::GoUp( void )
{GUCEF_TRACE;

    if ( m_currentRoot >= (UInt32) m_roots.size() )
        return false;
    const CString parent = ParentOf( m_currentDirectory );
    if ( parent.IsNULLOrEmpty() || NormalizeDirectory( parent ) == NormalizeDirectory( m_currentDirectory ) )
        return false;

    // not above the top of the file system of the root
    const CString& top = m_roots[ m_currentRoot ].topDirectory;
    if ( !top.IsNULLOrEmpty() && NormalizeDirectory( m_currentDirectory ) == top )
        return false;
    GoTo( m_currentRoot, parent, true );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::Refresh( void )
{GUCEF_TRACE;

    if ( m_currentRoot < (UInt32) m_roots.size() && GUCEF_NULL != GetRootWidget() )
    {
        ReadDirectory();
        ResetTree();
        RevealInTree( m_currentRoot, m_currentDirectory );
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CContentBrowserForm::GetCurrentRootName( void ) const
{GUCEF_TRACE;

    return m_currentRoot < (UInt32) m_roots.size() ? m_roots[ m_currentRoot ].name : CString::Empty;
}

/*-------------------------------------------------------------------------*/

const CString&
CContentBrowserForm::GetCurrentDirectory( void ) const
{GUCEF_TRACE;

    return m_currentDirectory;
}

/*-------------------------------------------------------------------------*/

CString
CContentBrowserForm::TypeNameOf( const TItem& item ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_resolver )
    {
        const CString resolved = m_resolver->GetTypeName( item.path, item.isDirectory );
        if ( !resolved.IsNULLOrEmpty() )
            return resolved;
    }
    if ( item.isDirectory )
        return "Folder";
    const CString extension = ExtensionOf( item.name );
    return extension.IsNULLOrEmpty() ? CString( "File" ) : extension.Uppercase() + " file";
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::ReadDirectory( void )
{GUCEF_TRACE;

    m_items.clear();
    CORE::IFileSystemAccess* access = GetCurrentAccess();
    if ( GUCEF_NULL != access )
    {
        CORE::TFileSystemIteratorPtr iterator;
        CORE::TFileSystemAccessResult result = access->TryGetFileSystemIterator( m_currentDirectory, iterator );
        if ( CORE::FILESYSTEMACCESS_SUCCESS == result && !iterator.IsNULL() )
        {
            if ( iterator->FindFirst( m_currentDirectory ) )
            {
                do
                {
                    TItem item;
                    item.name = NameOf( iterator->GetResourceName() );        // the last part, whatever the file system says
                    item.path = iterator->GetResourcePath();
                    if ( item.path.IsNULLOrEmpty() )
                        item.path = JoinPath( m_currentDirectory, item.name );
                    item.isDirectory = iterator->IsADirectory();
                    CORE::CResourceMetaData metaData;
                    const bool hasMetaData = iterator->TryReadMetaData( metaData );
                    item.hasSize = hasMetaData && metaData.hasResourceSizeInBytes && !item.isDirectory;
                    item.size = item.hasSize ? metaData.resourceSizeInBytes : 0;
                    item.hasModified = hasMetaData && metaData.hasModifiedDateTime;
                    if ( item.hasModified )
                        item.modified = metaData.modifiedDateTime;
                    item.hidden = ( item.name.Length() > 0 && '.' == item.name[ 0 ] ) || ( hasMetaData && metaData.hasIsHidden && metaData.isHidden );
                    item.sidecar = GUCEF_NULL != m_resolver && !item.isDirectory && m_resolver->IsSidecar( item.path, item.isDirectory );
                    item.rank = GUCEF_NULL != m_roots[ m_currentRoot ].order ? m_roots[ m_currentRoot ].order->GetRank( item.path, item.isDirectory ) : 0;
                    item.typeName = TypeNameOf( item );
                    if ( GUCEF_NULL != m_resolver )
                        item.iconName = m_resolver->GetIconName( item.path, item.isDirectory );
                    m_items.push_back( item );
                }
                while ( (UInt32) m_items.size() < MaxEntriesPerFolder && iterator->FindNext() );
                iterator->FindClose();
            }
        }
        else
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ContentBrowserForm: The folder " << m_currentDirectory << " cannot be read" );
    }
    SortItems();
    UpdateTypeFilterChoices();
    ApplyFilters();
    ShowItems();
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::SortItems( void )
{GUCEF_TRACE;

    std::stable_sort( m_items.begin(), m_items.end(), CItemOrder( m_sortColumn, m_sortAscending ) );
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::UpdateTypeFilterChoices( void )
{GUCEF_TRACE;

    GUCEF::set< CString > types;
    for ( UInt32 i=0; i<(UInt32)m_items.size(); ++i )
        if ( !m_items[ i ].isDirectory && ( m_showHidden || !m_items[ i ].hidden ) && ( m_showSidecars || !m_items[ i ].sidecar ) )
            types.insert( m_items[ i ].typeName );
    m_typeChoices.clear();
    for ( GUCEF::set< CString >::const_iterator t = types.begin(); t != types.end(); ++t )
        m_typeChoices.push_back( *t );
    if ( !m_typeFilter.IsNULLOrEmpty() && types.find( m_typeFilter ) == types.end() )
        m_typeFilter.Clear();

    CCombobox* combobox = GetWidgetAs< CCombobox >( "TypeFilter" );
    if ( GUCEF_NULL != combobox )
    {
        CCombobox::TStringVector items;
        items.push_back( AllTypesText );
        Int32 selected = 0;
        for ( UInt32 i=0; i<(UInt32)m_typeChoices.size(); ++i )
        {
            items.push_back( m_typeChoices[ i ] );
            if ( m_typeChoices[ i ] == m_typeFilter )
                selected = (Int32) i + 1;
        }
        m_updatingWidgets = true;
        combobox->SetListItems( items );
        combobox->SetSelectedIndex( selected );
        m_updatingWidgets = false;
    }
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::ApplyFilters( void )
{GUCEF_TRACE;

    if ( !m_filter.IsNULLOrEmpty() )
        m_filterPattern.SetPattern( CORE::CGlobPattern::HasGlobCharacters( m_filter ) ? m_filter : "*" + m_filter + "*", false );
    m_listed.clear();
    for ( UInt32 i=0; i<(UInt32)m_items.size(); ++i )
    {
        const TItem& item = m_items[ i ];
        if ( item.hidden && !m_showHidden )
            continue;
        if ( item.sidecar && !m_showSidecars )
            continue;
        if ( !item.isDirectory )
        {
            if ( !m_filter.IsNULLOrEmpty() && !m_filterPattern.IsMatch( item.name ) )
                continue;
            if ( !m_typeFilter.IsNULLOrEmpty() && item.typeName != m_typeFilter )
                continue;
        }
        m_listed.push_back( i );
    }
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::ShowItems( void )
{GUCEF_TRACE;

    // the selection is of the listed items: what is listed now is another list
    ShowDetails();
    m_tilesDirty = true;
    if ( VIEWMODE_THUMBNAILS == m_viewMode )
        Update( 0 );
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::ShowDetails( void )
{GUCEF_TRACE;

    CGridView* grid = GetWidgetAs< CGridView >( "Items" );
    if ( GUCEF_NULL == grid )
        return;
    CGridView::TRowVector rows;
    for ( UInt32 i=0; i<(UInt32)m_listed.size(); ++i )
    {
        const TItem& item = m_items[ m_listed[ i ] ];
        CGridView::TStringVector cells;
        cells.push_back( item.isDirectory ? item.name + "/" : item.name );
        cells.push_back( item.typeName );
        cells.push_back( item.hasSize ? FormatSize( item.size ) : CString::Empty );
        cells.push_back( item.hasModified ? item.modified.ToIso8601DateString( true ) + " " + TwoDigits( item.modified.GetHours() ) + ":" + TwoDigits( item.modified.GetMinutes() ) : CString::Empty );
        rows.push_back( cells );
    }
    m_updatingWidgets = true;
    grid->SetRows( rows );
    grid->SetIntProperty( EWidgetProperty_SortColumn, m_sortColumn );
    grid->SetBoolProperty( EWidgetProperty_SortAscending, m_sortAscending );
    GUCEF::vector< Int32 > widths;
    widths.push_back( 46 );
    widths.push_back( 22 );
    widths.push_back( 12 );
    widths.push_back( 20 );
    grid->SetIntListProperty( EWidgetProperty_ColumnWidths, widths );
    if ( !m_selection.empty() )
        grid->SetSelectedRows( m_selection );
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::ShowViewMode( void )
{GUCEF_TRACE;

    CWidget* grid = GetWidget( "Items" );
    CWidget* tiles = GetWidget( "Thumbs" );
    if ( GUCEF_NULL != grid )
        grid->SetBoolProperty( EWidgetProperty_Visible, VIEWMODE_DETAILS == m_viewMode );
    if ( GUCEF_NULL != tiles )
        tiles->SetBoolProperty( EWidgetProperty_Visible, VIEWMODE_THUMBNAILS == m_viewMode );
    m_tilesDirty = true;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::UpdateButtons( void )
{GUCEF_TRACE;

    CWidget* back = GetWidget( "Back" );
    if ( GUCEF_NULL != back )
        back->SetBoolProperty( EWidgetProperty_Enabled, !m_back.empty() );
    CWidget* forward = GetWidget( "Forward" );
    if ( GUCEF_NULL != forward )
        forward->SetBoolProperty( EWidgetProperty_Enabled, !m_forward.empty() );
    CWidget* up = GetWidget( "Up" );
    if ( GUCEF_NULL != up )
    {
        bool canGoUp = false;
        if ( m_currentRoot < (UInt32) m_roots.size() )
        {
            const CString parent = ParentOf( m_currentDirectory );
            canGoUp = !parent.IsNULLOrEmpty() && NormalizeDirectory( m_currentDirectory ) != m_roots[ m_currentRoot ].topDirectory;
        }
        up->SetBoolProperty( EWidgetProperty_Enabled, canGoUp );
    }
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::SetViewMode( EViewMode mode )
{GUCEF_TRACE;

    if ( mode == m_viewMode )
        return;
    m_viewMode = mode;
    CWidget* box = GetWidget( "Thumbnails" );
    if ( GUCEF_NULL != box )
    {
        m_updatingWidgets = true;
        box->SetBoolProperty( EWidgetProperty_Checked, VIEWMODE_THUMBNAILS == mode );
        m_updatingWidgets = false;
    }
    ShowViewMode();
    Update( 0 );
}

/*-------------------------------------------------------------------------*/

CContentBrowserForm::EViewMode
CContentBrowserForm::GetViewMode( void ) const
{GUCEF_TRACE;

    return m_viewMode;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::SetThumbnailSize( UInt32 size )
{GUCEF_TRACE;

    if ( size < (UInt32) MIN_THUMBNAIL_SIZE )
        size = (UInt32) MIN_THUMBNAIL_SIZE;
    if ( size > (UInt32) MAX_THUMBNAIL_SIZE )
        size = (UInt32) MAX_THUMBNAIL_SIZE;
    if ( size == m_thumbnailSize )
        return;
    m_thumbnailSize = size;

    // thumbnails of another size are not these: they are asked for again
    m_thumbnails.clear();
    m_requested.clear();
    m_tilesDirty = true;
    Update( 0 );
}

/*-------------------------------------------------------------------------*/

UInt32
CContentBrowserForm::GetThumbnailSize( void ) const
{GUCEF_TRACE;

    return m_thumbnailSize;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::SetFilter( const CString& glob )
{GUCEF_TRACE;

    if ( glob == m_filter )
        return;
    m_filter = glob;
    CWidget* field = GetWidget( "Filter" );
    if ( GUCEF_NULL != field && field->GetProperty( EWidgetProperty_Text ) != glob )
    {
        m_updatingWidgets = true;
        field->SetProperty( EWidgetProperty_Text, glob );
        m_updatingWidgets = false;
    }
    TInt32Vector before = m_selection;
    ApplyFilters();
    m_selection.clear();
    ShowItems();
    NotifyObservers( FilterChangedEvent );
    if ( !before.empty() )
        NotifySelectionChanged();
}

/*-------------------------------------------------------------------------*/

const CString&
CContentBrowserForm::GetFilter( void ) const
{GUCEF_TRACE;

    return m_filter;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::SetTypeFilter( const CString& typeName )
{GUCEF_TRACE;

    if ( typeName == m_typeFilter )
        return;
    m_typeFilter = typeName;
    UpdateTypeFilterChoices();
    TInt32Vector before = m_selection;
    ApplyFilters();
    m_selection.clear();
    ShowItems();
    if ( !before.empty() )
        NotifySelectionChanged();
}

/*-------------------------------------------------------------------------*/

const CString&
CContentBrowserForm::GetTypeFilter( void ) const
{GUCEF_TRACE;

    return m_typeFilter;
}

/*-------------------------------------------------------------------------*/

const CString::StringVector&
CContentBrowserForm::GetTypeFilterChoices( void ) const
{GUCEF_TRACE;

    return m_typeChoices;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::SetShowSidecars( bool showSidecars )
{GUCEF_TRACE;

    if ( showSidecars == m_showSidecars )
        return;
    m_showSidecars = showSidecars;
    UpdateTypeFilterChoices();
    ApplyFilters();
    m_selection.clear();
    ShowItems();
    CWidget* box = GetWidget( "Sidecars" );
    if ( GUCEF_NULL != box && box->GetBoolProperty( EWidgetProperty_Checked ) != showSidecars )
    {
        m_updatingWidgets = true;
        box->SetBoolProperty( EWidgetProperty_Checked, showSidecars );
        m_updatingWidgets = false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CContentBrowserForm::GetShowSidecars( void ) const
{GUCEF_TRACE;

    return m_showSidecars;
}

/*-------------------------------------------------------------------------*/

UInt32
CContentBrowserForm::GetHiddenSidecarCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    if ( !m_showSidecars )
        for ( UInt32 i=0; i<(UInt32)m_items.size(); ++i )
            if ( m_items[ i ].sidecar && ( m_showHidden || !m_items[ i ].hidden ) )
                ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::SetShowHidden( bool showHidden )
{GUCEF_TRACE;

    if ( showHidden == m_showHidden )
        return;
    m_showHidden = showHidden;
    UpdateTypeFilterChoices();
    ApplyFilters();
    m_selection.clear();
    ShowItems();
    ResetTree();
    RevealInTree( m_currentRoot, m_currentDirectory );
}

/*-------------------------------------------------------------------------*/

bool
CContentBrowserForm::GetShowHidden( void ) const
{GUCEF_TRACE;

    return m_showHidden;
}

/*-------------------------------------------------------------------------*/

UInt32
CContentBrowserForm::GetListedCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_listed.size();
}

/*-------------------------------------------------------------------------*/

const CContentBrowserForm::TItem&
CContentBrowserForm::GetListedItem( UInt32 index ) const
{GUCEF_TRACE;

    static const TItem none;
    return index < (UInt32) m_listed.size() ? m_items[ m_listed[ index ] ] : none;
}

/*-------------------------------------------------------------------------*/

CString::StringVector
CContentBrowserForm::GetListedNames( void ) const
{GUCEF_TRACE;

    CString::StringVector names;
    for ( UInt32 i=0; i<(UInt32)m_listed.size(); ++i )
    {
        const TItem& item = m_items[ m_listed[ i ] ];
        names.push_back( item.isDirectory ? item.name + "/" : item.name );
    }
    return names;
}

/*-------------------------------------------------------------------------*/

CContentBrowserForm::TInt32Vector
CContentBrowserForm::GetSelection( void ) const
{GUCEF_TRACE;

    return m_selection;
}

/*-------------------------------------------------------------------------*/

CString::StringVector
CContentBrowserForm::GetSelectedPaths( void ) const
{GUCEF_TRACE;

    CString::StringVector paths;
    for ( UInt32 i=0; i<(UInt32)m_selection.size(); ++i )
        if ( m_selection[ i ] >= 0 && m_selection[ i ] < (Int32) m_listed.size() )
            paths.push_back( m_items[ m_listed[ m_selection[ i ] ] ].path );
    return paths;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::NotifySelectionChanged( void )
{GUCEF_TRACE;

    NotifyObservers( SelectionChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::SetSelectionRows( const TInt32Vector& items, bool notify )
{GUCEF_TRACE;

    TInt32Vector sorted = items;
    std::sort( sorted.begin(), sorted.end() );
    sorted.erase( std::unique( sorted.begin(), sorted.end() ), sorted.end() );
    const bool changed = sorted != m_selection;
    m_selection = sorted;
    CGridView* grid = GetWidgetAs< CGridView >( "Items" );
    if ( GUCEF_NULL != grid )
    {
        m_updatingWidgets = true;
        grid->SetSelectedRows( m_selection );
        if ( !m_selection.empty() )
            grid->ScrollToRow( m_selection.front() );
        m_updatingWidgets = false;
    }
    ShowTileSelection();
    if ( notify && changed )
        NotifySelectionChanged();
}

/*-------------------------------------------------------------------------*/

bool
CContentBrowserForm::SelectItems( const TInt32Vector& items )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)items.size(); ++i )
        if ( items[ i ] < 0 || items[ i ] >= (Int32) m_listed.size() )
            return false;
    SetSelectionRows( items, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CContentBrowserForm::SelectNames( const CString::StringVector& names )
{GUCEF_TRACE;

    TInt32Vector rows;
    for ( UInt32 i=0; i<(UInt32)m_listed.size(); ++i )
    {
        const TItem& item = m_items[ m_listed[ i ] ];
        for ( UInt32 n=0; n<(UInt32)names.size(); ++n )
            if ( names[ n ] == item.name || names[ n ] == item.name + "/" )
                rows.push_back( (Int32) i );
    }
    if ( rows.size() != names.size() )
        return false;
    SetSelectionRows( rows, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CContentBrowserForm::ActivateItem( UInt32 index )
{GUCEF_TRACE;

    if ( index >= (UInt32) m_listed.size() )
        return false;
    const TItem item = m_items[ m_listed[ index ] ];
    if ( item.isDirectory )
    {
        GoTo( m_currentRoot, item.path, true );
        // a folder is entered; the people who listen may do more with it (the group of a resource view opens its presentation)
        CORE::TCloneableString folderData( item.path );
        NotifyObservers( FolderActivatedEvent, &folderData );
    }
    else
    {
        CORE::TCloneableString eventData( item.path );
        NotifyObservers( ItemActivatedEvent, &eventData );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::SetItemActions( const TContextMenuItemVector& actions )
{GUCEF_TRACE;

    m_itemActions = actions;
}

/*-------------------------------------------------------------------------*/

const TContextMenuItemVector&
CContentBrowserForm::GetItemActions( void ) const
{GUCEF_TRACE;

    return m_itemActions;
}

/*-------------------------------------------------------------------------*/

bool
CContentBrowserForm::SelectionHasFile( void ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_selection.size(); ++i )
        if ( m_selection[ i ] >= 0 && m_selection[ i ] < (Int32) m_listed.size() && !m_items[ m_listed[ m_selection[ i ] ] ].isDirectory )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CContentBrowserForm::OpenItemActions( Int32 x, Int32 y )
{GUCEF_TRACE;

    CWidget* root = GetRootWidget();
    CGUIContext* context = GUCEF_NULL != root ? root->GetContext() : GUCEF_NULL;
    if ( GUCEF_NULL == context || m_itemActions.empty() || !SelectionHasFile() )
        return false;
    if ( GUCEF_NULL == m_itemMenu )
    {
        m_itemMenu = CWidget::As< CPopupMenu >( context->CreateWidget( CPopupMenu::GetRegisteredTypeName(), root->GetName() + "#ItemActions", GUCEF_NULL ) );
        if ( GUCEF_NULL == m_itemMenu )
            return false;
        TEventCallback callback( this, &CContentBrowserForm::OnItemMenuActivated );
        SubscribeTo( m_itemMenu, CWidget::ItemActivatedEvent, callback );
    }
    // the menu of the tiles looks like the one of the list: the items of CContextMenuItem
    CGridView* grid = GetWidgetAs< CGridView >( "Items" );
    if ( GUCEF_NULL != grid )
        m_itemMenu->SetProperty( EWidgetProperty_Font, grid->GetPropertyRef( EWidgetProperty_Font ) );
    CPopupMenu::TStringVector texts;
    TInt32Vector disabled;
    for ( UInt32 i=0; i<(UInt32)m_itemActions.size(); ++i )
    {
        texts.push_back( m_itemActions[ i ].GetDisplayText() );
        if ( !m_itemActions[ i ].separator && !m_itemActions[ i ].enabled )
            disabled.push_back( (Int32) i );
    }
    m_itemMenu->SetItems( texts );
    m_itemMenu->SetIntListProperty( EWidgetProperty_DisabledItems, disabled );
    return m_itemMenu->ShowAt( x, y );
}

/*-------------------------------------------------------------------------*/

bool
CContentBrowserForm::IsItemActionsOpen( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_itemMenu && GUCEF_NULL != m_itemMenu->GetContext() && m_itemMenu->GetContext()->GetPopup() == m_itemMenu )
        return true;
    const CGridView* grid = const_cast< CContentBrowserForm* >( this )->GetWidgetAs< CGridView >( "Items" );
    return GUCEF_NULL != grid && grid->IsContextMenuOpen();
}

/*-------------------------------------------------------------------------*/

bool
CContentBrowserForm::ChooseItemAction( const CString& actionId )
{GUCEF_TRACE;

    if ( !SelectionHasFile() )
        return false;
    for ( UInt32 i=0; i<(UInt32)m_itemActions.size(); ++i )
    {
        if ( !m_itemActions[ i ].separator && m_itemActions[ i ].enabled && m_itemActions[ i ].id == actionId )
        {
            CORE::TCloneableString eventData( actionId );
            NotifyObservers( ItemActionEvent, &eventData );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnItemMenuActivated( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the row of the menu that was chosen is the action of that row, as the context menu of a grid does it
    CItemEventData* item = static_cast< CItemEventData* >( eventData );
    if ( GUCEF_NULL == item || EVENTPHASE_TUNNEL == item->phase )
        return;
    if ( item->index < 0 || item->index >= (Int32) m_itemActions.size() || m_itemActions[ (UInt32) item->index ].separator )
        return;
    ChooseItemAction( m_itemActions[ (UInt32) item->index ].id );
}

/*-------------------------------------------------------------------------*/

bool
CContentBrowserForm::GetContextMenuItems( const CGridView& grid, Int32 row, Int32 column, TContextMenuItemVector& items )
{GUCEF_TRACE;

    if ( m_itemActions.empty() || row < 0 || row >= (Int32) m_listed.size() )
        return false;
    // the menu is for the selection: a row that is not in it becomes the selection
    if ( std::find( m_selection.begin(), m_selection.end(), row ) == m_selection.end() )
    {
        TInt32Vector one( 1, row );
        SelectItems( one );
    }
    if ( !SelectionHasFile() )
        return false;
    items = m_itemActions;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnContextMenuItem( CGridView& grid, Int32 row, Int32 column, const CString& itemId )
{GUCEF_TRACE;

    ChooseItemAction( itemId );
}

/*-------------------------------------------------------------------------*/

UInt32
CContentBrowserForm::GetRealizedTileCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( UInt32 i=0; i<(UInt32)m_tiles.size(); ++i )
        if ( m_tiles[ i ].item >= 0 )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

bool
CContentBrowserForm::HasThumbnail( const CString& path ) const
{GUCEF_TRACE;

    return m_thumbnails.find( path ) != m_thumbnails.end();
}

/*-------------------------------------------------------------------------*/

UInt32
CContentBrowserForm::GetThumbnailRequestCount( void ) const
{GUCEF_TRACE;

    return m_requestCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CContentBrowserForm::GetTilesPerRow( void ) const
{GUCEF_TRACE;

    return m_tilesPerRow;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::UpdateToolbarLayout( void )
{GUCEF_TRACE;

    // the tool bar follows the room the form has: one row (wide), two rows, or three rows (narrow), so that no control is drawn over another
    // and each keeps room for its text (one row needs the 656 pixels of the buttons and the type filter and room for the filter). The room is the width of the panel the buttons are in, as it was laid out last
    CWidget* back = GetWidget( "Back" );
    CWidget* content = GUCEF_NULL != back ? back->GetParentWidget() : GUCEF_NULL;
    if ( GUCEF_NULL == content || content->GetRect().width <= 0 )
        return;
    const Int32 width = content->GetRect().width;
    // with the switch for the alternate view the one row of the tool bar needs more room before it is chosen
    const Int32 mode = width >= ( m_alternateRoot >= 0 ? 1100 : 700 ) ? 0 : width >= 380 ? 1 : 2;

    // the rows are as high as a line of the text of the controls (28 pixels, more for a larger font or line spacing: the controls
    // take that height anyway and would overlap the next row and the list)
    Int32 rowHeight = 28;
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context )
    {
        const Int32 line = context->GetMinimumLineHeight( back->GetPropertyRef( EWidgetProperty_Font ) );
        if ( line > rowHeight )
            rowHeight = line;
    }
    if ( mode == m_toolbarMode && rowHeight == m_toolbarRowHeight )
        return;
    m_toolbarMode = mode;
    m_toolbarRowHeight = rowHeight;
    const Int32 pitch = rowHeight + 6;

    // the left and the width of each control per mode, and the row it is in
    struct SSlot { const char* name; const char* left[ 3 ]; Int32 row[ 3 ]; const char* width[ 3 ]; };
    static const SSlot slots[] =
    {
        { "Back",       { "0,6",        "0,6",      "0,6"       }, { 0, 0, 0 }, { "0,56",      "0,56",     "0.3,-9"    } },
        { "Forward",    { "0,66",       "0,66",     "0.3,3"     }, { 0, 0, 0 }, { "0,76",      "0,76",     "0.4,-6"    } },
        { "Up",         { "0,146",      "0,146",    "0.7,3"     }, { 0, 0, 0 }, { "0,44",      "0,44",     "0.3,-9"    } },
        { "Filter",     { "0,198",      "0,6",      "0,6"       }, { 0, 1, 1 }, { "1,-656",    "0.4,-9",   "0.55,-9"   } },
        { "TypeFilter", { "1,-450",     "0.4,3",    "0,6"       }, { 0, 1, 2 }, { "0,210",     "0.3,-6",   "0.55,-9"   } },
        { "Sidecars",   { "1,-236",     "0.7,3",    "0.55,3"    }, { 0, 1, 1 }, { "0,108",     "0.3,-9",   "0.45,-9"   } },
        { "Thumbnails", { "1,-124",     "1,-124",   "0.55,3"    }, { 0, 0, 2 }, { "0,118",     "0,118",    "0.45,-9"   } }
    };
    for ( UInt32 i=0; i<sizeof( slots ) / sizeof( slots[ 0 ] ); ++i )
    {
        CWidget* widget = GetWidget( slots[ i ].name );
        if ( GUCEF_NULL == widget )
            continue;
        widget->SetProperty( EWidgetProperty_Position, CString( slots[ i ].left[ mode ] ) + " 0," + CORE::Int32ToString( 4 + pitch * slots[ i ].row[ mode ] ) );
        widget->SetProperty( EWidgetProperty_Size, CString( slots[ i ].width[ mode ] ) + " 0," + CORE::Int32ToString( rowHeight ) );
    }

    // the switch between the two views, when there is an alternate view: after "Up" in the first row (below the filter in a narrow form);
    // the filter makes room for it
    CWidget* viewSwitch = GetWidget( "ViewSwitch" );
    if ( GUCEF_NULL != viewSwitch )
    {
        const bool shown = m_alternateRoot >= 0;
        viewSwitch->SetProperty( EWidgetProperty_Visible, shown ? "true" : "false" );
        CWidget* filter = GetWidget( "Filter" );
        if ( shown && GUCEF_NULL != filter )
        {
            const CString height = CORE::Int32ToString( rowHeight );
            if ( 0 == mode )
            {
                viewSwitch->SetProperty( EWidgetProperty_Position, "0,198 0,4" );
                viewSwitch->SetProperty( EWidgetProperty_Size, "0,210 0," + height );
                filter->SetProperty( EWidgetProperty_Position, "0,416 0,4" );
                filter->SetProperty( EWidgetProperty_Size, "1,-874 0," + height );
            }
            else
            {
                // a form that is not wide has a row of its own for it, below the others
                viewSwitch->SetProperty( EWidgetProperty_Position, "0,6 0," + CORE::Int32ToString( 4 + pitch * ( 1 == mode ? 2 : 3 ) ) );
                viewSwitch->SetProperty( EWidgetProperty_Size, "1,-12 0," + height );
            }
        }
    }

    // below the last row of the tool bar
    static const Int32 bodyTop[ 3 ] = { 36, 72, 106 };
    const CString top = CORE::Int32ToString( bodyTop[ mode ] + ( rowHeight - 28 ) * ( mode + 1 ) + ( m_alternateRoot >= 0 && mode >= 1 ? pitch : 0 ) );
    // the names of the groups of the alternate view and their counts need more room than folder names do
    const bool alternateShown = IsAlternateViewActive();
    const char* const split = alternateShown ? "0.54" : "0.25";
    const char* const rest = alternateShown ? "0.46" : "0.75";
    CWidget* folders = GetWidget( "Folders" );
    if ( GUCEF_NULL != folders )
    {
        folders->SetProperty( EWidgetProperty_Position, "0,0 0," + top );
        folders->SetProperty( EWidgetProperty_Size, CString( split ) + ",0 1,-" + top );
    }
    const char* const bodyNames[ 2 ] = { "Items", "Thumbs" };
    for ( UInt32 i=0; i<2; ++i )
    {
        CWidget* widget = GetWidget( bodyNames[ i ] );
        if ( GUCEF_NULL != widget )
        {
            widget->SetProperty( EWidgetProperty_Position, CString( split ) + ",6 0," + top );
            widget->SetProperty( EWidgetProperty_Size, CString( rest ) + ",-6 1,-" + top );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::Update( UInt32 deltaInMs )
{GUCEF_TRACE;

    UpdateToolbarLayout();
    if ( VIEWMODE_THUMBNAILS != m_viewMode )
        return;
    CScrollPanel* thumbs = CWidget::ToScrollPanel( GetWidget( "Thumbs" ) );
    if ( GUCEF_NULL == thumbs )
        return;
    const CRect viewport = thumbs->GetViewportRect();
    if ( m_tilesDirty || thumbs->GetScrollPosition() != m_lastScroll || viewport.width != m_lastViewportWidth || viewport.height != m_lastViewportHeight )
        RealizeTiles();
}

/*-------------------------------------------------------------------------*/

CWidget*
CContentBrowserForm::MakeTile( UInt32 slot )
{GUCEF_TRACE;

    CGUIContext* context = GetGUIContext();
    CWidget* thumbs = GetWidget( "Thumbs" );
    if ( GUCEF_NULL == context || GUCEF_NULL == thumbs )
        return GUCEF_NULL;
    CWidget* tile = context->CreateWidget( "Panel", TileName( slot ), thumbs );
    if ( GUCEF_NULL == tile )
        return GUCEF_NULL;
    tile->SetBoolProperty( EWidgetProperty_Clip, true );
    CWidget* image = context->CreateWidget( "ImageFrame", "Image", tile );
    image->SetProperty( EWidgetProperty_ScaleMode, "Fit" );
    image->SetBoolProperty( EWidgetProperty_Smooth, true );
    image->SetBoolProperty( EWidgetProperty_InputTransparent, true );
    CWidget* kind = context->CreateWidget( "Label", "Kind", tile );
    kind->SetProperty( EWidgetProperty_HTextAlign, "Center" );
    kind->SetProperty( EWidgetProperty_VTextAlign, "Center" );
    kind->SetBoolProperty( EWidgetProperty_InputTransparent, true );
    CWidget* name = context->CreateWidget( "Label", "Name", tile );
    name->SetProperty( EWidgetProperty_HTextAlign, "Center" );
    name->SetBoolProperty( EWidgetProperty_WordWrap, true );
    name->SetBoolProperty( EWidgetProperty_InputTransparent, true );
    return tile;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::RealizeTiles( void )
{GUCEF_TRACE;

    CScrollPanel* thumbs = CWidget::ToScrollPanel( GetWidget( "Thumbs" ) );
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == thumbs || GUCEF_NULL == context )
        return;
    const CRect viewport = thumbs->GetViewportRect();
    if ( viewport.width <= 0 || viewport.height <= 0 )
        return;     // not laid out yet: the next frame

    const Int32 cellWidth = (Int32) m_thumbnailSize + 2 * (Int32) TILE_PADDING;
    const Int32 cellHeight = (Int32) m_thumbnailSize + (Int32) TILE_LABEL_HEIGHT + 2 * (Int32) TILE_PADDING;
    const Int32 columns = viewport.width / cellWidth > 0 ? viewport.width / cellWidth : 1;
    const Int32 count = (Int32) m_listed.size();
    const Int32 rows = ( count + columns - 1 ) / columns;
    m_tilesPerRow = (UInt32) columns;

    // the room of all the tiles: a spacer at the bottom makes the scroll panel as high as the rows need
    CWidget* spacer = thumbs->GetChildWidget( "TileSpacer" );
    if ( GUCEF_NULL == spacer )
    {
        spacer = context->CreateWidget( "Panel", "TileSpacer", thumbs );
        spacer->SetColourProperty( EWidgetProperty_BackgroundColour, CColour( 0, 0, 0, 0 ) );
        spacer->SetColourProperty( EWidgetProperty_BorderColour, CColour( 0, 0, 0, 0 ) );
        spacer->SetBoolProperty( EWidgetProperty_InputTransparent, true );
    }
    spacer->SetProperty( EWidgetProperty_Position, "0,0 0," + CORE::Int32ToString( rows > 0 ? rows * cellHeight - 1 : 0 ) );
    spacer->SetProperty( EWidgetProperty_Size, "0,1 0,1" );
    thumbs->SetIntProperty( EWidgetProperty_ScrollStep, cellHeight / 2 );

    // the rows in view and one more each way
    Int32 scroll = thumbs->GetScrollPosition();
    const Int32 maxScroll = rows * cellHeight - viewport.height;
    if ( scroll > ( maxScroll > 0 ? maxScroll : 0 ) )
        scroll = maxScroll > 0 ? maxScroll : 0;
    Int32 firstRow = scroll / cellHeight - 1;
    if ( firstRow < 0 )
        firstRow = 0;
    const Int32 visibleRows = viewport.height / cellHeight + 3;
    const UInt32 slots = (UInt32) ( visibleRows * columns );
    while ( (UInt32) m_tiles.size() < slots )
    {
        STile tile;
        tile.panel = MakeTile( (UInt32) m_tiles.size() );
        tile.item = -1;
        if ( GUCEF_NULL == tile.panel )
            break;
        m_tiles.push_back( tile );
    }
    for ( UInt32 s=0; s<(UInt32)m_tiles.size(); ++s )
    {
        const Int32 item = firstRow * columns + (Int32) s;
        if ( s < slots && item < count )
            BindTile( s, item );
        else
        {
            m_tiles[ s ].item = -1;
            m_tiles[ s ].panel->SetBoolProperty( EWidgetProperty_Visible, false );
        }
    }
    m_lastScroll = thumbs->GetScrollPosition();
    m_lastViewportWidth = viewport.width;
    m_lastViewportHeight = viewport.height;
    m_tilesDirty = false;
    RequestThumbnails();
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::BindTile( UInt32 slot, Int32 item )
{GUCEF_TRACE;

    STile& tile = m_tiles[ slot ];
    const TItem& entry = m_items[ m_listed[ item ] ];
    const Int32 size = (Int32) m_thumbnailSize;
    const Int32 pad = (Int32) TILE_PADDING;
    const Int32 cellWidth = size + 2 * pad;
    const Int32 cellHeight = size + (Int32) TILE_LABEL_HEIGHT + 2 * pad;
    const Int32 column = item % (Int32) m_tilesPerRow;
    const Int32 row = item / (Int32) m_tilesPerRow;
    tile.item = item;

    CWidget* panel = tile.panel;
    panel->SetProperty( EWidgetProperty_Position, "0," + CORE::Int32ToString( column * cellWidth ) + " 0," + CORE::Int32ToString( row * cellHeight ) );
    panel->SetProperty( EWidgetProperty_Size, "0," + CORE::Int32ToString( cellWidth - 2 ) + " 0," + CORE::Int32ToString( cellHeight - 2 ) );
    panel->SetProperty( EWidgetProperty_Tooltip, entry.name );
    panel->SetBoolProperty( EWidgetProperty_Visible, true );

    CWidget* image = panel->GetChildWidget( "Image" );
    CWidget* kind = panel->GetChildWidget( "Kind" );
    CWidget* name = panel->GetChildWidget( "Name" );
    image->SetProperty( EWidgetProperty_Position, "0," + CORE::Int32ToString( pad ) + " 0," + CORE::Int32ToString( pad ) );
    image->SetProperty( EWidgetProperty_Size, "0," + CORE::Int32ToString( size ) + " 0," + CORE::Int32ToString( size ) );
    kind->SetProperty( EWidgetProperty_Position, "0," + CORE::Int32ToString( pad ) + " 0," + CORE::Int32ToString( pad + size / 2 - 10 ) );
    kind->SetProperty( EWidgetProperty_Size, "0," + CORE::Int32ToString( size ) + " 0,20" );
    name->SetProperty( EWidgetProperty_Position, "0," + CORE::Int32ToString( pad ) + " 0," + CORE::Int32ToString( pad + size + 2 ) );
    name->SetProperty( EWidgetProperty_Size, "0," + CORE::Int32ToString( size ) + " 0," + CORE::Int32ToString( (Int32) TILE_LABEL_HEIGHT ) );
    name->SetProperty( EWidgetProperty_Text, entry.isDirectory ? entry.name + "/" : entry.name );

    CImageFrame* frame = image->IsRegisteredAs( "ImageFrame" ) ? static_cast< CImageFrame* >( image ) : GUCEF_NULL;
    GUCEF::map< CString, IMAGE::TPixelMapPtr >::const_iterator thumbnail = m_thumbnails.find( entry.path );
    bool pictured = false;
    if ( thumbnail != m_thumbnails.end() && !(*thumbnail).second.IsNULL() && GUCEF_NULL != frame )
    {
        frame->SetImage( (*thumbnail).second );
        pictured = true;
    }
    else
    if ( !entry.iconName.IsNULLOrEmpty() && GUCEF_NULL != frame )
    {
        frame->SetImageName( entry.iconName );
        pictured = true;
    }
    else
    if ( GUCEF_NULL != frame )
        frame->SetImageName( CString::Empty );
    image->SetBoolProperty( EWidgetProperty_Visible, pictured );
    kind->SetBoolProperty( EWidgetProperty_Visible, !pictured );
    kind->SetProperty( EWidgetProperty_Text, entry.typeName );
    panel->SetColourProperty( EWidgetProperty_BackgroundColour, std::binary_search( m_selection.begin(), m_selection.end(), item ) ? TileSelectedColour : TileColour );
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::ShowTileSelection( void )
{GUCEF_TRACE;

    for ( UInt32 s=0; s<(UInt32)m_tiles.size(); ++s )
    {
        if ( m_tiles[ s ].item >= 0 )
            m_tiles[ s ].panel->SetColourProperty( EWidgetProperty_BackgroundColour,
                std::binary_search( m_selection.begin(), m_selection.end(), m_tiles[ s ].item ) ? TileSelectedColour : TileColour );
    }
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::RequestThumbnails( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_provider )
        return;
    for ( UInt32 s=0; s<(UInt32)m_tiles.size(); ++s )
    {
        if ( m_tiles[ s ].item < 0 )
            continue;
        const TItem& entry = m_items[ m_listed[ m_tiles[ s ].item ] ];
        if ( entry.isDirectory || m_thumbnails.find( entry.path ) != m_thumbnails.end() || m_requested.find( entry.path ) != m_requested.end() )
            continue;
        m_requested.insert( entry.path );
        ++m_requestCount;
        m_provider->RequestThumbnail( entry.path, m_thumbnailSize );
    }
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::SelectByTile( Int32 item, UInt32 modifiers )
{GUCEF_TRACE;

    if ( item < 0 || item >= (Int32) m_listed.size() )
        return;
    TInt32Vector selection;
    if ( 0 != ( modifiers & INPUT::KEYMOD_SHIFT ) && m_anchorItem >= 0 )
    {
        const Int32 first = m_anchorItem < item ? m_anchorItem : item;
        const Int32 last = m_anchorItem < item ? item : m_anchorItem;
        for ( Int32 i=first; i<=last; ++i )
            selection.push_back( i );
    }
    else
    if ( 0 != ( modifiers & INPUT::KEYMOD_CTRL ) )
    {
        selection = m_selection;
        TInt32Vector::iterator found = std::find( selection.begin(), selection.end(), item );
        if ( found != selection.end() )
            selection.erase( found );
        else
            selection.push_back( item );
        m_anchorItem = item;
    }
    else
    {
        selection.push_back( item );
        m_anchorItem = item;
    }
    SetSelectionRows( selection, true );
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::MoveTileSelection( Int32 step, bool absolute )
{GUCEF_TRACE;

    if ( m_listed.empty() )
        return;
    Int32 current = m_selection.empty() ? -1 : m_selection.back();
    Int32 next = absolute ? ( step < 0 ? (Int32) m_listed.size() - 1 : step ) : ( current < 0 ? ( step > 0 ? 0 : (Int32) m_listed.size() - 1 ) : current + step );
    if ( next < 0 )
        next = 0;
    if ( next >= (Int32) m_listed.size() )
        next = (Int32) m_listed.size() - 1;
    m_anchorItem = next;
    TInt32Vector selection;
    selection.push_back( next );
    SetSelectionRows( selection, true );

    // the tile is brought into view
    CScrollPanel* thumbs = CWidget::ToScrollPanel( GetWidget( "Thumbs" ) );
    if ( GUCEF_NULL != thumbs && m_tilesPerRow > 0 )
    {
        const Int32 cellHeight = (Int32) m_thumbnailSize + (Int32) TILE_LABEL_HEIGHT + 2 * (Int32) TILE_PADDING;
        const Int32 top = ( next / (Int32) m_tilesPerRow ) * cellHeight;
        const Int32 viewHeight = thumbs->GetViewportRect().height;
        Int32 scroll = thumbs->GetScrollPosition();
        if ( top < scroll )
            scroll = top;
        else
        if ( top + cellHeight > scroll + viewHeight )
            scroll = top + cellHeight - viewHeight;
        if ( scroll != thumbs->GetScrollPosition() )
            thumbs->SetScrollPosition( scroll );
        Update( 0 );
    }
}

/*-------------------------------------------------------------------------*/

Int32
CContentBrowserForm::FindTreeNode( const CString& treePath ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_tree.size(); ++i )
        if ( -2 != m_tree[ i ].parent && m_tree[ i ].treePath == treePath )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CContentBrowserForm::AddTreeNode( Int32 parent, const CString& label, UInt32 root, const CString& directory )
{GUCEF_TRACE;

    STreeNode node;
    node.label = label;
    node.parent = parent;
    node.isPlaceholder = false;
    node.loaded = false;
    node.removed = false;
    node.location.root = root;
    node.location.directory = directory;
    node.treePath = parent < 0 ? TreeLabel( label ) : m_tree[ parent ].treePath + "/" + TreeLabel( label );
    m_tree.push_back( node );
    const Int32 index = (Int32) m_tree.size() - 1;

    // the arrow of a folder that was not read yet
    STreeNode placeholder;
    placeholder.label = "...";
    placeholder.parent = index;
    placeholder.isPlaceholder = true;
    placeholder.loaded = true;
    placeholder.removed = false;
    placeholder.location = node.location;
    placeholder.treePath = m_tree[ index ].treePath + "/...";
    m_tree.push_back( placeholder );
    return index;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::ResetTree( void )
{GUCEF_TRACE;

    m_tree.clear();
    for ( UInt32 r=0; r<(UInt32)m_roots.size(); ++r )
        AddTreeNode( -1, m_roots[ r ].name, r, m_roots[ r ].topDirectory.IsNULLOrEmpty() ? m_roots[ r ].initialDirectory : m_roots[ r ].topDirectory );
    ShowTree();
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::LoadTreeChildren( Int32 node )
{GUCEF_TRACE;

    if ( node < 0 || node >= (Int32) m_tree.size() || m_tree[ node ].loaded || m_tree[ node ].isPlaceholder )
        return;
    const SLocation location = m_tree[ node ].location;
    m_tree[ node ].loaded = true;
    for ( UInt32 i=0; i<(UInt32)m_tree.size(); ++i )
        if ( m_tree[ i ].parent == node && m_tree[ i ].isPlaceholder )
        {
            m_tree[ i ].parent = -2;
            m_tree[ i ].removed = true;
        }
    if ( location.root >= (UInt32) m_roots.size() )
        return;
    CORE::IFileSystemAccess* access = m_roots[ location.root ].access.GetPointerAlways();
    CORE::TFileSystemIteratorPtr iterator;
    if ( CORE::FILESYSTEMACCESS_SUCCESS != access->TryGetFileSystemIterator( location.directory, iterator ) || iterator.IsNULL() )
        return;
    CString::StringVector names;
    GUCEF::map< CString, CString > paths;
    if ( iterator->FindFirst( location.directory ) )
    {
        do
        {
            if ( !iterator->IsADirectory() )
                continue;
            const CString name = NameOf( iterator->GetResourceName() );
            if ( name.IsNULLOrEmpty() || ( !m_showHidden && '.' == name[ 0 ] ) )
                continue;
            CString path = iterator->GetResourcePath();
            if ( path.IsNULLOrEmpty() )
                path = JoinPath( location.directory, name );
            names.push_back( name );
            paths[ name ] = path;
        }
        while ( (UInt32) names.size() < MaxEntriesPerFolder && iterator->FindNext() );
        iterator->FindClose();
    }
    CIContentItemOrder* order = m_roots[ location.root ].order;
    if ( GUCEF_NULL != order )
    {
        // the order of the root first, then the names
        GUCEF::map< CString, Int32 > ranks;
        for ( UInt32 i=0; i<(UInt32)names.size(); ++i )
            ranks[ names[ i ] ] = order->GetRank( paths[ names[ i ] ], true );
        CRankNameOrder rankOrder( &ranks );
        std::sort( names.begin(), names.end(), rankOrder );
    }
    else
    std::sort( names.begin(), names.end() );
    for ( UInt32 i=0; i<(UInt32)names.size(); ++i )
    {
        // a folder the chain of the open folder made already is kept
        const CString treePath = m_tree[ node ].treePath + "/" + TreeLabel( names[ i ] );
        if ( FindTreeNode( treePath ) < 0 )
            AddTreeNode( node, names[ i ], location.root, paths[ names[ i ] ] );
    }
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::RevealInTree( UInt32 root, const CString& directory )
{GUCEF_TRACE;

    if ( root >= (UInt32) m_roots.size() )
        return;
    CTreeview* tree = GetWidgetAs< CTreeview >( "Folders" );

    // the folders from the top of the root down to the open folder
    const CString top = m_roots[ root ].topDirectory;
    CString::StringVector chain;
    CString current = NormalizeDirectory( directory );
    bool reachedTop = top.IsNULLOrEmpty() || current == top;
    chain.push_back( current );
    for ( UInt32 guard=0; guard<64 && !reachedTop; ++guard )
    {
        const CString parent = NormalizeDirectory( ParentOf( current ) );
        if ( parent.IsNULLOrEmpty() || parent == current )
            break;
        current = parent;
        if ( current == top )
        {
            reachedTop = true;
            break;
        }
        chain.push_back( current );
    }
    std::reverse( chain.begin(), chain.end() );

    // the node of the root, then the nodes below it for the folders of the chain; each is read, so that its folders are there
    Int32 node = FindTreeNode( TreeLabel( m_roots[ root ].name ) );
    if ( node < 0 )
        return;
    LoadTreeChildren( node );
    CString::StringVector expanded;
    expanded.push_back( m_tree[ node ].treePath );
    for ( UInt32 c=0; c<(UInt32)chain.size(); ++c )
    {
        if ( reachedTop && chain[ c ] == top )
            continue;
        Int32 child = -1;
        for ( UInt32 i=0; i<(UInt32)m_tree.size() && child < 0; ++i )
            if ( m_tree[ i ].parent == node && !m_tree[ i ].isPlaceholder && NormalizeDirectory( m_tree[ i ].location.directory ) == chain[ c ] )
                child = (Int32) i;
        if ( child < 0 )
            child = AddTreeNode( node, NameOf( chain[ c ] ), root, chain[ c ] );
        node = child;
        LoadTreeChildren( node );
        expanded.push_back( m_tree[ node ].treePath );
    }
    ShowTree();
    if ( GUCEF_NULL != tree )
    {
        m_updatingWidgets = true;
        // an ancestor is open, what the user opened stays open
        for ( UInt32 i=0; i+1<(UInt32)expanded.size(); ++i )
            tree->SetExpanded( expanded[ i ], true );
        tree->SetSelectedItem( m_tree[ node ].treePath );

        // with several roots the tree is long: the roots that are not the one that is open are folded, so the open folder is in view
        for ( UInt32 r=0; r<(UInt32)m_roots.size(); ++r )
        {
            if ( r == root )
                continue;
            const Int32 otherRoot = FindTreeNode( TreeLabel( m_roots[ r ].name ) );
            if ( otherRoot >= 0 )
                tree->SetExpanded( m_tree[ otherRoot ].treePath, false );
        }
        m_updatingWidgets = false;
    }
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::ShowTree( void )
{GUCEF_TRACE;

    CTreeview* tree = GetWidgetAs< CTreeview >( "Folders" );
    if ( GUCEF_NULL == tree )
        return;

    // every node in the order of the tree: a node, then what is below it
    CString items;
    struct SLocal
    {
        static void Add( const TTreeNodeVector& nodes, Int32 parent, CString& out )
        {
            for ( UInt32 i=0; i<(UInt32)nodes.size(); ++i )
            {
                if ( nodes[ i ].parent != parent || nodes[ i ].removed )
                    continue;
                out += ( out.IsNULLOrEmpty() ? "" : "\n" ) + nodes[ i ].treePath;
                Add( nodes, (Int32) i, out );
            }
        }
    };
    SLocal::Add( m_tree, -1, items );
    m_updatingWidgets = true;
    tree->SetProperty( EWidgetProperty_Items, items );
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnBackActivated( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GoBack();
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnForwardActivated( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GoForward();
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnUpActivated( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GoUp();
}

/*-------------------------------------------------------------------------*/

bool
CContentBrowserForm::SetAlternateRoot( const CString& rootName, const CString& normalLabel, const CString& alternateLabel )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) m_roots.size(); ++i )
    {
        if ( m_roots[ i ].name != rootName )
            continue;
        m_alternateRoot = (Int32) i;
        m_normalLabel = normalLabel;
        m_alternateLabel = alternateLabel;
        CCombobox* combobox = GetWidgetAs< CCombobox >( "ViewSwitch" );
        if ( GUCEF_NULL != combobox )
        {
            CCombobox::TStringVector items;
            items.push_back( m_normalLabel );
            items.push_back( m_alternateLabel );
            m_updatingWidgets = true;
            combobox->SetListItems( items );
            combobox->SetSelectedIndex( m_currentRoot == i ? 1 : 0 );
            m_updatingWidgets = false;
        }
        // the tool bar is laid out again with the switch in it
        m_toolbarMode = -1;
        UpdateToolbarLayout();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CString&
CContentBrowserForm::GetAlternateRootName( void ) const
{GUCEF_TRACE;

    static const CString none;
    return m_alternateRoot >= 0 ? m_roots[ (size_t) m_alternateRoot ].name : none;
}

/*-------------------------------------------------------------------------*/
bool
CContentBrowserForm::SetRootOrder( const CString& rootName, CIContentItemOrder* order )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) m_roots.size(); ++i )
    {
        if ( m_roots[ i ].name == rootName )
        {
            m_roots[ i ].order = order;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/


bool
CContentBrowserForm::IsAlternateViewActive( void ) const
{GUCEF_TRACE;

    return m_alternateRoot >= 0 && (Int32) m_currentRoot == m_alternateRoot;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::SetAlternateViewActive( bool active )
{GUCEF_TRACE;

    if ( m_alternateRoot < 0 || active == IsAlternateViewActive() )
        return;
    if ( active )
    {
        m_normalRoot = m_currentRoot;
        m_normalDirectory = m_currentDirectory;
        NavigateTo( m_roots[ (size_t) m_alternateRoot ].name, CString::Empty );
    }
    else
    {
        const UInt32 root = m_normalRoot < (UInt32) m_roots.size() && (Int32) m_normalRoot != m_alternateRoot ? m_normalRoot : ( 0 == m_alternateRoot && m_roots.size() > 1 ? 1 : 0 );
        GoTo( root, m_normalDirectory, true );
    }
    UpdateViewSwitch();
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::UpdateViewSwitch( void )
{GUCEF_TRACE;

    CCombobox* combobox = GetWidgetAs< CCombobox >( "ViewSwitch" );
    if ( GUCEF_NULL == combobox || m_alternateRoot < 0 )
        return;
    const Int32 wanted = IsAlternateViewActive() ? 1 : 0;
    if ( combobox->GetSelectedIndex() != wanted )
    {
        m_updatingWidgets = true;
        combobox->SetSelectedIndex( wanted );
        m_updatingWidgets = false;
    }
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnViewSwitchValueChanged( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    CCombobox* combobox = GetWidgetAs< CCombobox >( "ViewSwitch" );
    if ( GUCEF_NULL == combobox )
        return;
    SetAlternateViewActive( 1 == combobox->GetSelectedIndex() );
    NotifyObservers( ViewSwitchedEvent );
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnFilterTextChanged( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    CWidget* field = GetWidget( "Filter" );
    if ( GUCEF_NULL != field )
        SetFilter( field->GetProperty( EWidgetProperty_Text ) );
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnTypeFilterValueChanged( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    CCombobox* combobox = GetWidgetAs< CCombobox >( "TypeFilter" );
    if ( GUCEF_NULL == combobox )
        return;
    const Int32 selected = combobox->GetSelectedIndex();
    if ( selected <= 0 )
        SetTypeFilter( CString::Empty );
    else
    if ( selected <= (Int32) m_typeChoices.size() )
        SetTypeFilter( m_typeChoices[ selected - 1 ] );
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnThumbnailsChecked( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_updatingWidgets )
        SetViewMode( VIEWMODE_THUMBNAILS );
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnThumbnailsUnchecked( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_updatingWidgets )
        SetViewMode( VIEWMODE_DETAILS );
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnSidecarsChecked( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_updatingWidgets )
        SetShowSidecars( true );
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnSidecarsUnchecked( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_updatingWidgets )
        SetShowSidecars( false );
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnTreeValueChanged( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    CTreeview* tree = GetWidgetAs< CTreeview >( "Folders" );
    if ( GUCEF_NULL == tree )
        return;
    const Int32 node = FindTreeNode( tree->GetSelectedItem() );
    if ( node < 0 || m_tree[ node ].isPlaceholder )
        return;
    const SLocation location = m_tree[ node ].location;
    if ( location.root != m_currentRoot || NormalizeDirectory( location.directory ) != NormalizeDirectory( m_currentDirectory ) )
        GoTo( location.root, location.directory, true );
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnTreePropertyChanged( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets || GUCEF_NULL == eventData )
        return;
    CPropertyChangedEventData* changed = static_cast< CPropertyChangedEventData* >( eventData );
    if ( EWidgetProperty_ExpandedItems != changed->property )
        return;

    // a folder that was opened: its folders are read now
    CTreeview* tree = GetWidgetAs< CTreeview >( "Folders" );
    if ( GUCEF_NULL == tree )
        return;
    const CTreeview::TStringVector expanded = tree->GetStringListProperty( EWidgetProperty_ExpandedItems );
    bool loaded = false;
    for ( UInt32 i=0; i<(UInt32)expanded.size(); ++i )
    {
        const Int32 node = FindTreeNode( expanded[ i ] );
        if ( node >= 0 && !m_tree[ node ].loaded )
        {
            LoadTreeChildren( node );
            loaded = true;
        }
    }
    if ( loaded )
        ShowTree();
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnItemsSelectionChanged( CORE::CNotifier* notifier    ,
                                              const CORE::CEvent& eventId  ,
                                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    CGridView* grid = GetWidgetAs< CGridView >( "Items" );
    if ( GUCEF_NULL == grid )
        return;
    const CGridView::TInt32Vector rows = grid->GetSelectedRows();
    TInt32Vector selection;
    for ( UInt32 i=0; i<(UInt32)rows.size(); ++i )
        if ( rows[ i ] >= 0 && rows[ i ] < (Int32) m_listed.size() )
            selection.push_back( rows[ i ] );
    if ( selection == m_selection )
        return;
    m_selection = selection;
    m_anchorItem = grid->GetSelectedRow();
    ShowTileSelection();
    NotifySelectionChanged();
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnItemsColumnClicked( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets || GUCEF_NULL == eventData )
        return;
    CGridView::TColumnClickedEventData* column = static_cast< CGridView::TColumnClickedEventData* >( eventData );
    const Int32 clicked = (Int32) column->GetData().column;
    m_sortAscending = clicked == m_sortColumn ? !m_sortAscending : true;
    m_sortColumn = clicked;
    const bool hadSelection = !m_selection.empty();
    SortItems();
    ApplyFilters();
    m_selection.clear();
    ShowItems();
    if ( hadSelection )
        NotifySelectionChanged();
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnItemsItemActivated( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CRoutedEventData* routed = static_cast< CRoutedEventData* >( eventData );
    if ( GUCEF_NULL != routed && EVENTPHASE_TUNNEL == routed->phase )
        return;
    CGridView* grid = GetWidgetAs< CGridView >( "Items" );
    if ( GUCEF_NULL != grid && grid->GetSelectedRow() >= 0 )
        ActivateItem( (UInt32) grid->GetSelectedRow() );
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnTilesMouseClick( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CPointerEventData* pointer = static_cast< CPointerEventData* >( eventData );
    if ( GUCEF_NULL == pointer || ( MOUSEBUTTON_LEFT != pointer->button && MOUSEBUTTON_RIGHT != pointer->button ) || EVENTPHASE_TUNNEL == pointer->phase )
        return;
    CWidget* thumbs = GetWidget( "Thumbs" );

    // the tile that was hit: the widget below the scroll panel that the source is, or is in
    CWidget* hit = pointer->source;
    while ( GUCEF_NULL != hit && hit->GetParentWidget() != thumbs )
        hit = hit->GetParentWidget();
    if ( GUCEF_NULL == hit )
        return;
    for ( UInt32 s=0; s<(UInt32)m_tiles.size(); ++s )
    {
        if ( m_tiles[ s ].panel == hit && m_tiles[ s ].item >= 0 )
        {
            if ( GUCEF_NULL != thumbs )
                thumbs->SetFocus();
            if ( MOUSEBUTTON_RIGHT == pointer->button )
            {
                // the menu is for the selection: a tile that is not in it becomes the selection
                if ( std::find( m_selection.begin(), m_selection.end(), m_tiles[ s ].item ) == m_selection.end() )
                    SelectByTile( m_tiles[ s ].item, 0 );
                OpenItemActions( pointer->x, pointer->y );
                return;
            }
            SelectByTile( m_tiles[ s ].item, pointer->modifiers );
            if ( pointer->clickCount >= 2 )
                ActivateItem( (UInt32) m_tiles[ s ].item );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnTilesKeyDown( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CKeyEventData* key = static_cast< CKeyEventData* >( eventData );
    if ( GUCEF_NULL == key || key->handled || EVENTPHASE_BUBBLE == key->phase || 0 != ( key->modifiers & INPUT::KEYMOD_ALT ) )
        return;
    const Int32 columns = (Int32) m_tilesPerRow;
    bool used = true;
    switch ( key->keyCode )
    {
        case INPUT::KEYCODE_LEFT:   MoveTileSelection( -1, false ); break;
        case INPUT::KEYCODE_RIGHT:  MoveTileSelection( 1, false ); break;
        case INPUT::KEYCODE_UP:     MoveTileSelection( -columns, false ); break;
        case INPUT::KEYCODE_DOWN:   MoveTileSelection( columns, false ); break;
        case INPUT::KEYCODE_HOME:   MoveTileSelection( 0, true ); break;
        case INPUT::KEYCODE_END:    MoveTileSelection( -1, true ); break;
        case INPUT::KEYCODE_RETURN:
        {
            if ( !m_selection.empty() )
                ActivateItem( (UInt32) m_selection.back() );
            break;
        }
        default:                    used = false; break;
    }
    if ( used )
        key->handled = true;
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnRootKeyDown( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CKeyEventData* key = static_cast< CKeyEventData* >( eventData );
    if ( GUCEF_NULL == key || key->handled || EVENTPHASE_BUBBLE != key->phase )
        return;

    // keys of the browser that no widget wanted; a line of text keeps Backspace
    CWidget* focused = key->source;
    const bool inEditbox = GUCEF_NULL != focused && focused->IsRegisteredAs( "Editbox" );
    if ( inEditbox )
        return;
    if ( INPUT::KEYCODE_BACK == key->keyCode && 0 == ( key->modifiers & INPUT::KEYMOD_ALT ) )
        key->handled = GoUp();
    else
    if ( INPUT::KEYCODE_LEFT == key->keyCode && 0 != ( key->modifiers & INPUT::KEYMOD_ALT ) )
        key->handled = GoBack();
    else
    if ( INPUT::KEYCODE_RIGHT == key->keyCode && 0 != ( key->modifiers & INPUT::KEYMOD_ALT ) )
        key->handled = GoForward();
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnThumbnailReady( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    CIThumbnailProvider::TThumbnailReadyEventData* ready = static_cast< CIThumbnailProvider::TThumbnailReadyEventData* >( eventData );
    const CIThumbnailProvider::TThumbnail& thumbnail = ready->GetData();
    if ( thumbnail.size != m_thumbnailSize || m_requested.find( thumbnail.path ) == m_requested.end() )
        return;     // asked for another size, or not for this folder any more
    if ( thumbnail.pixels.IsNULL() )
        return;     // there is none to make: the tile keeps its type
    m_thumbnails[ thumbnail.path ] = thumbnail.pixels;
    for ( UInt32 s=0; s<(UInt32)m_tiles.size(); ++s )
        if ( m_tiles[ s ].item >= 0 && m_items[ m_listed[ m_tiles[ s ].item ] ].path == thumbnail.path )
            BindTile( s, m_tiles[ s ].item );
}

/*-------------------------------------------------------------------------*/

void
CContentBrowserForm::OnFrameStarted( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Update( GUCEF_NULL != eventData ? static_cast< CORE::TCloneableUInt32* >( eventData )->GetData() : 0 );
}

/*-------------------------------------------------------------------------*/

const CString&
CContentBrowserForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CContentBrowserForm";
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
