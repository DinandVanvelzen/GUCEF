/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CINOTIFIER_H
#include "gucefCORE_CINotifier.h"
#define GUCEF_CORE_CINOTIFIER_H
#endif /* GUCEF_CORE_CINOTIFIER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_GUI_CSPINNER_H
#include "gucefGUI_CSpinner.h"
#define GUCEF_GUI_CSPINNER_H
#endif /* GUCEF_GUI_CSPINNER_H ? */

#ifndef GUCEF_GAMEGUI_UTILS_H
#include "gucefGAMEGUI_Utils.h"
#define GUCEF_GAMEGUI_UTILS_H
#endif /* GUCEF_GAMEGUI_UTILS_H ? */

#include "gucefGAMEGUI_CServerBrowserForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GAMEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CServerBrowserForm::JoinRequestedEvent = "GUCEF::GAMEGUI::CServerBrowserForm::JoinRequestedEvent";
const CORE::CEvent CServerBrowserForm::SpectateRequestedEvent = "GUCEF::GAMEGUI::CServerBrowserForm::SpectateRequestedEvent";
const CORE::CEvent CServerBrowserForm::CreateRequestedEvent = "GUCEF::GAMEGUI::CServerBrowserForm::CreateRequestedEvent";
const CORE::CEvent CServerBrowserForm::DirectConnectRequestedEvent = "GUCEF::GAMEGUI::CServerBrowserForm::DirectConnectRequestedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The order of the rows: by a column, the unknown pings last whichever way, ties by name and id */
class CListingLess
{
    public:

    CListingLess( Int32 column, bool ascending )
        : m_column( column )
        , m_ascending( ascending )
    {
    }

    bool operator()( const MULTIPLAY::CGameListingEntry& a, const MULTIPLAY::CGameListingEntry& b ) const
    {
        if ( CServerBrowserForm::COLUMN_PING == m_column )
        {
            bool unknownA = a.pingMs < 0;
            bool unknownB = b.pingMs < 0;
            if ( unknownA != unknownB )
                return unknownB;
        }

        Int32 order = CompareColumn( a, b );
        if ( 0 != order )
            return m_ascending ? order < 0 : order > 0;

        CORE::CString nameA = a.name.Lowercase();
        CORE::CString nameB = b.name.Lowercase();
        if ( nameA != nameB )
            return nameA < nameB;
        return a.listingId < b.listingId;
    }

    private:

    static Int32 CompareNumbers( Int64 a, Int64 b )
    {
        return a < b ? -1 : ( a > b ? 1 : 0 );
    }

    static Int32 CompareText( const CORE::CString& a, const CORE::CString& b )
    {
        CORE::CString lowerA = a.Lowercase();
        CORE::CString lowerB = b.Lowercase();
        return lowerA < lowerB ? -1 : ( lowerB < lowerA ? 1 : 0 );
    }

    Int32 CompareColumn( const MULTIPLAY::CGameListingEntry& a, const MULTIPLAY::CGameListingEntry& b ) const
    {
        switch ( m_column )
        {
            case CServerBrowserForm::COLUMN_NAME:      return CompareText( a.name, b.name );
            case CServerBrowserForm::COLUMN_KIND:      return CompareNumbers( a.kind, b.kind );
            case CServerBrowserForm::COLUMN_PLAYERS:   return CompareNumbers( a.players, b.players );
            case CServerBrowserForm::COLUMN_MAP:       return CompareText( a.mapName, b.mapName );
            case CServerBrowserForm::COLUMN_MODE:      return CompareText( a.modeName, b.modeName );
            case CServerBrowserForm::COLUMN_PING:      return CompareNumbers( a.pingMs, b.pingMs );
            case CServerBrowserForm::COLUMN_LOCK:      return CompareNumbers( a.passworded ? 1 : 0, b.passworded ? 1 : 0 );
            case CServerBrowserForm::COLUMN_SOURCE:    return CompareText( a.backendName, b.backendName );
            default:                                   return 0;
        }
    }

    private:

    Int32 m_column;
    bool m_ascending;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CServerBrowserForm::RegisterEvents( void )
{GUCEF_TRACE;

    JoinRequestedEvent.Initialize();
    SpectateRequestedEvent.Initialize();
    CreateRequestedEvent.Initialize();
    DirectConnectRequestedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CServerBrowserForm::CServerBrowserForm( void )
    : GUI::CFormEx()
    , m_browser( GUCEF_NULL )
    , m_filter()
    , m_listed()
    , m_sortColumn( COLUMN_NAME )
    , m_sortAscending( true )
    , m_updating( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CServerBrowserForm::CServerBrowserForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_browser( GUCEF_NULL )
    , m_filter()
    , m_listed()
    , m_sortColumn( COLUMN_NAME )
    , m_sortAscending( true )
    , m_updating( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CServerBrowserForm::~CServerBrowserForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    const Int32 filterTop = WindowContentTop;
    const Int32 filterTop2 = filterTop + 40;
    const Int32 listTop = filterTop2 + 40;

    // nine tenths of the screen (up to 2400 wide), from below a strip an application keeps along the top
    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "ServerBrowserForm", "0.5,0 0,56", "0.9,0 1,-72" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "Games" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_HAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MinSize, "0,640 0,420" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MaxSize, "0,2400 0,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Clip, "true" );

    // the filter bar, in two rows so it fits a narrow window: the text and the kind, then the options
    CORE::CDataNode* text = GUI::CFormLayout::AddWidgetNode( *window, "Editbox", "FilterText", "0,16 0," + CORE::Int32ToString( filterTop ), "1,-198 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *text, GUI::EWidgetProperty_Tooltip, "Find in the name, map, mode and host" );
    CORE::CDataNode* kind = GUI::CFormLayout::AddWidgetNode( *window, "Combobox", "FilterKind", "1,-166 0," + CORE::Int32ToString( filterTop ), "0,150 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *kind, GUI::EWidgetProperty_Items, "All\nLobbies\nIn progress" );
    GUI::CFormLayout::SetWidgetNodeProperty( *kind, GUI::EWidgetProperty_SelectedIndex, "0" );
    CORE::CDataNode* hideFull = GUI::CFormLayout::AddWidgetNode( *window, "Checkbox", "HideFull", "0,16 0," + CORE::Int32ToString( filterTop2 ), "0,130 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *hideFull, GUI::EWidgetProperty_Text, "Hide full" );
    CORE::CDataNode* hideLocked = GUI::CFormLayout::AddWidgetNode( *window, "Checkbox", "HideLocked", "0,160 0," + CORE::Int32ToString( filterTop2 ), "0,190 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *hideLocked, GUI::EWidgetProperty_Text, "Hide passworded" );
    CORE::CDataNode* pingLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "MaxPingLabel", "0,370 0," + CORE::Int32ToString( filterTop2 ), "0,90 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *pingLabel, GUI::EWidgetProperty_Text, "Max ping" );
    GUI::CFormLayout::SetWidgetNodeProperty( *pingLabel, GUI::EWidgetProperty_VTextAlign, "Center" );
    CORE::CDataNode* maxPing = GUI::CFormLayout::AddWidgetNode( *window, "Spinner", "MaxPing", "0,464 0," + CORE::Int32ToString( filterTop2 ), "0,120 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *maxPing, GUI::EWidgetProperty_Min, "0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *maxPing, GUI::EWidgetProperty_Max, "2000" );
    GUI::CFormLayout::SetWidgetNodeProperty( *maxPing, GUI::EWidgetProperty_Step, "10" );
    GUI::CFormLayout::SetWidgetNodeProperty( *maxPing, GUI::EWidgetProperty_Value, "0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *maxPing, GUI::EWidgetProperty_Tooltip, "0 is no limit" );

    // the listings take what the filter and the button strip leave
    CORE::CDataNode* listings = GUI::CFormLayout::AddWidgetNode( *window, "GridView", "Listings", "0,16 0," + CORE::Int32ToString( listTop ),
                                                                 "1,-32 1," + CORE::Int32ToString( -( listTop + WindowFooterHeight + 8 ) ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *listings, GUI::EWidgetProperty_Columns, "Name\nKind\nPlayers\nMap\nMode\nPing\nLock\nSource" );

    // the status and the buttons, from the right: a strip along the bottom edge that nothing else uses
    CORE::CDataNode* footer = GUI::CFormLayout::AddWidgetNode( *window, "Panel", "ButtonRow", "0,1 1," + CORE::Int32ToString( -( WindowFooterHeight + 1 ) ), "1,-2 0," + CORE::Int32ToString( WindowFooterHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *footer, GUI::EWidgetProperty_BackgroundColour, "18,20,26,255" );
    CORE::CDataNode* status = GUI::CFormLayout::AddWidgetNode( *footer, "Label", "Status", "0,16 0,8", "1,-32 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *status, GUI::EWidgetProperty_TextColour, "170,175,190,255" );
    CORE::CDataNode* join = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Join", "1,-106 1,-52", "0,90 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *join, GUI::EWidgetProperty_Text, "Join" );
    GUI::CFormLayout::SetWidgetNodeProperty( *join, GUI::EWidgetProperty_StyleClass, "Primary" );
    CORE::CDataNode* spectate = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Spectate", "1,-214 1,-52", "0,100 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *spectate, GUI::EWidgetProperty_Text, "Spectate" );
    CORE::CDataNode* create = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Create", "1,-312 1,-52", "0,90 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *create, GUI::EWidgetProperty_Text, "Create" );
    CORE::CDataNode* direct = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "DirectConnect", "1,-460 1,-52", "0,140 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *direct, GUI::EWidgetProperty_Text, "Direct connect" );
    CORE::CDataNode* refresh = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Refresh", "1,-558 1,-52", "0,90 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *refresh, GUI::EWidgetProperty_Text, "Refresh" );
}

/*-------------------------------------------------------------------------*/

bool
CServerBrowserForm::Build( void )
{GUCEF_TRACE;

    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;
    Hide();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CServerBrowserForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;
    UpdateRows();
    return Show();
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    GUI::CWidget* button = GetWidget( "Join" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CServerBrowserForm::OnJoinActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }
    button = GetWidget( "Spectate" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CServerBrowserForm::OnSpectateActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }
    button = GetWidget( "Create" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CServerBrowserForm::OnCreateActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }
    button = GetWidget( "DirectConnect" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CServerBrowserForm::OnDirectConnectActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }
    button = GetWidget( "Refresh" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CServerBrowserForm::OnRefreshActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }

    // every filter control tells the same: read them all again
    TEventCallback filterCallback( this, &CServerBrowserForm::OnFilterChanged );
    const char* filters[] = { "FilterText", "FilterKind", "HideFull", "HideLocked", "MaxPing" };
    for ( UInt32 i=0; i<5; ++i )
    {
        GUI::CWidget* filter = GetWidget( filters[ i ] );
        if ( GUCEF_NULL != filter )
            SubscribeTo( filter, GUI::CWidget::ValueChangedEvent, filterCallback );
    }

    GUI::CGridView* grid = GetWidgetAs< GUI::CGridView >( "Listings" );
    if ( GUCEF_NULL != grid )
    {
        TEventCallback selectionCallback( this, &CServerBrowserForm::OnSelectionChanged );
        SubscribeTo( grid, GUI::CGridView::SelectionChangedEvent, selectionCallback );
        TEventCallback columnCallback( this, &CServerBrowserForm::OnColumnClicked );
        SubscribeTo( grid, GUI::CGridView::ColumnClickedEvent, columnCallback );
        TEventCallback activatedCallback( this, &CServerBrowserForm::OnListingActivated );
        SubscribeTo( grid, GUI::CWidget::ItemActivatedEvent, activatedCallback );

        GUCEF::vector< Int32 > widths;
        widths.push_back( 300 );    // name
        widths.push_back( 120 );    // kind
        widths.push_back( 90 );     // players
        widths.push_back( 150 );    // map
        widths.push_back( 130 );    // mode
        widths.push_back( 80 );     // ping
        widths.push_back( 90 );     // lock
        widths.push_back( 110 );    // source
        grid->SetIntListProperty( GUI::EWidgetProperty_ColumnWidths, widths );
        grid->SetBoolProperty( GUI::EWidgetProperty_FitColumns, true );
    }
    ShowFilterInControls();
    UpdateRows();
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::SetBrowser( MULTIPLAY::CGameBrowser* browser )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_browser )
        UnsubscribeFrom( m_browser );
    m_browser = browser;
    if ( GUCEF_NULL != m_browser )
    {
        TEventCallback callback( this, &CServerBrowserForm::OnBrowserListChanged );
        SubscribeTo( m_browser, MULTIPLAY::CGameBrowser::BrowserListChangedEvent, callback );
    }
    UpdateRows();
}

/*-------------------------------------------------------------------------*/

MULTIPLAY::CGameBrowser*
CServerBrowserForm::GetBrowser( void ) const
{GUCEF_TRACE;

    return m_browser;
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::SetFilter( const MULTIPLAY::CGameListingFilter& filter )
{GUCEF_TRACE;

    m_filter = filter;
    ShowFilterInControls();
    UpdateRows();
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CGameListingFilter&
CServerBrowserForm::GetFilter( void ) const
{GUCEF_TRACE;

    return m_filter;
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::ShowFilterInControls( void )
{GUCEF_TRACE;

    bool wasUpdating = m_updating;
    m_updating = true;

    GUI::CWidget* text = GetWidget( "FilterText" );
    if ( GUCEF_NULL != text )
        text->SetProperty( GUI::EWidgetProperty_Text, m_filter.text );
    GUI::CCombobox* kind = GetWidgetAs< GUI::CCombobox >( "FilterKind" );
    if ( GUCEF_NULL != kind )
    {
        Int32 index = 0;
        if ( MULTIPLAY::CGameListingFilter::KINDFILTER_LOBBY == m_filter.kind )
            index = 1;
        else
        if ( MULTIPLAY::CGameListingFilter::KINDFILTER_GAME_IN_PROGRESS == m_filter.kind )
            index = 2;
        kind->SetSelectedIndex( index );
    }
    GUI::CWidget* hideFull = GetWidget( "HideFull" );
    if ( GUCEF_NULL != hideFull )
        hideFull->SetBoolProperty( GUI::EWidgetProperty_Checked, m_filter.notFull );
    GUI::CWidget* hideLocked = GetWidget( "HideLocked" );
    if ( GUCEF_NULL != hideLocked )
        hideLocked->SetBoolProperty( GUI::EWidgetProperty_Checked, m_filter.noPassword );
    GUI::CWidget* maxPing = GetWidget( "MaxPing" );
    if ( GUCEF_NULL != maxPing )
        maxPing->SetIntProperty( GUI::EWidgetProperty_Value, m_filter.maxPingMs < 0 ? 0 : m_filter.maxPingMs );

    m_updating = wasUpdating;
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::ReadFilterFromControls( void )
{GUCEF_TRACE;

    GUI::CWidget* text = GetWidget( "FilterText" );
    if ( GUCEF_NULL != text )
        m_filter.text = TrimText( text->GetProperty( GUI::EWidgetProperty_Text ) );
    GUI::CCombobox* kind = GetWidgetAs< GUI::CCombobox >( "FilterKind" );
    if ( GUCEF_NULL != kind )
    {
        switch ( kind->GetSelectedIndex() )
        {
            case 1:  m_filter.kind = MULTIPLAY::CGameListingFilter::KINDFILTER_LOBBY; break;
            case 2:  m_filter.kind = MULTIPLAY::CGameListingFilter::KINDFILTER_GAME_IN_PROGRESS; break;
            default: m_filter.kind = MULTIPLAY::CGameListingFilter::KINDFILTER_ANY; break;
        }
    }
    GUI::CWidget* hideFull = GetWidget( "HideFull" );
    if ( GUCEF_NULL != hideFull )
        m_filter.notFull = hideFull->GetBoolProperty( GUI::EWidgetProperty_Checked );
    GUI::CWidget* hideLocked = GetWidget( "HideLocked" );
    if ( GUCEF_NULL != hideLocked )
        m_filter.noPassword = hideLocked->GetBoolProperty( GUI::EWidgetProperty_Checked );
    GUI::CWidget* maxPing = GetWidget( "MaxPing" );
    if ( GUCEF_NULL != maxPing )
    {
        Int32 limit = maxPing->GetIntProperty( GUI::EWidgetProperty_Value );
        m_filter.maxPingMs = limit > 0 ? limit : -1;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServerBrowserForm::KindText( const MULTIPLAY::CGameListingEntry& listing )
{GUCEF_TRACE;

    return MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS == listing.kind ? "In progress" : "Lobby";
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::SortListed( void )
{GUCEF_TRACE;

    std::stable_sort( m_listed.begin(), m_listed.end(), CListingLess( m_sortColumn, m_sortAscending ) );
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::UpdateRows( void )
{GUCEF_TRACE;

    GUI::CGridView* grid = GetWidgetAs< GUI::CGridView >( "Listings" );

    // what the player had selected stays selected
    CORE::CString selectedId;
    MULTIPLAY::CGameListingEntry selected;
    if ( GetSelectedListing( selected ) )
        selectedId = selected.listingId;

    UInt32 available = 0;
    m_listed.clear();
    if ( GUCEF_NULL != m_browser )
    {
        TGameListingEntryVector all;
        m_browser->GetListings( all );
        available = (UInt32) all.size();
        for ( UInt32 i=0; i<(UInt32)all.size(); ++i )
            if ( m_filter.Matches( all[ i ] ) )
                m_listed.push_back( all[ i ] );
    }
    SortListed();

    if ( GUCEF_NULL == grid )
        return;

    bool wasUpdating = m_updating;
    m_updating = true;

    GUI::CGridView::TRowVector rows;
    Int32 selectedRow = -1;
    for ( UInt32 i=0; i<(UInt32)m_listed.size(); ++i )
    {
        const MULTIPLAY::CGameListingEntry& listing = m_listed[ i ];
        GUI::CGridView::TStringVector cells;
        cells.resize( COLUMN_COUNT );
        cells[ COLUMN_NAME ] = listing.name;
        cells[ COLUMN_KIND ] = KindText( listing );
        cells[ COLUMN_PLAYERS ] = FormatPlayers( listing.players, listing.maxPlayers );
        cells[ COLUMN_MAP ] = listing.mapName;
        cells[ COLUMN_MODE ] = listing.modeName;
        cells[ COLUMN_PING ] = FormatPing( listing.pingMs );
        cells[ COLUMN_LOCK ] = listing.passworded ? "Locked" : "";
        cells[ COLUMN_SOURCE ] = listing.backendName;
        rows.push_back( cells );
        if ( !selectedId.IsNULLOrEmpty() && listing.listingId == selectedId )
            selectedRow = (Int32) i;
    }
    grid->SetRows( rows );
    grid->SetIntProperty( GUI::EWidgetProperty_SortColumn, m_sortColumn );
    grid->SetBoolProperty( GUI::EWidgetProperty_SortAscending, m_sortAscending );
    grid->SetSelectedRow( selectedRow );

    GUI::CWidget* status = GetWidget( "Status" );
    if ( GUCEF_NULL != status )
    {
        CORE::CString text = CORE::UInt32ToString( (UInt32) m_listed.size() ) + ( 1 == m_listed.size() ? " game" : " games" );
        if ( available > (UInt32) m_listed.size() )
            text += " (" + CORE::UInt32ToString( available - (UInt32) m_listed.size() ) + " hidden by the filter)";
        status->SetProperty( GUI::EWidgetProperty_Text, text );
    }

    m_updating = wasUpdating;
    UpdateButtons();
}

/*-------------------------------------------------------------------------*/

const CServerBrowserForm::TGameListingEntryVector&
CServerBrowserForm::GetListedEntries( void ) const
{GUCEF_TRACE;

    return m_listed;
}

/*-------------------------------------------------------------------------*/

bool
CServerBrowserForm::GetSelectedListing( MULTIPLAY::CGameListingEntry& listing ) const
{GUCEF_TRACE;

    GUI::CGridView* grid = const_cast< CServerBrowserForm* >( this )->GetWidgetAs< GUI::CGridView >( "Listings" );
    if ( GUCEF_NULL == grid )
        return false;
    Int32 row = grid->GetSelectedRow();
    if ( row < 0 || row >= (Int32) m_listed.size() )
        return false;
    listing = m_listed[ row ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CServerBrowserForm::SelectListing( const CORE::CString& listingId )
{GUCEF_TRACE;

    GUI::CGridView* grid = GetWidgetAs< GUI::CGridView >( "Listings" );
    if ( GUCEF_NULL == grid )
        return false;
    for ( UInt32 i=0; i<(UInt32)m_listed.size(); ++i )
    {
        if ( m_listed[ i ].listingId == listingId )
        {
            grid->SetSelectedRow( (Int32) i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::SetSort( TColumn column, bool ascending )
{GUCEF_TRACE;

    m_sortColumn = (Int32) column;
    m_sortAscending = ascending;
    UpdateRows();
}

/*-------------------------------------------------------------------------*/

Int32
CServerBrowserForm::GetSortColumn( void ) const
{GUCEF_TRACE;

    return m_sortColumn;
}

/*-------------------------------------------------------------------------*/

bool
CServerBrowserForm::IsSortAscending( void ) const
{GUCEF_TRACE;

    return m_sortAscending;
}

/*-------------------------------------------------------------------------*/

bool
CServerBrowserForm::CanJoin( void ) const
{GUCEF_TRACE;

    MULTIPLAY::CGameListingEntry listing;
    return GetSelectedListing( listing ) && MULTIPLAY::CGameListingEntry::KIND_LOBBY == listing.kind && !listing.IsFull();
}

/*-------------------------------------------------------------------------*/

bool
CServerBrowserForm::CanSpectate( void ) const
{GUCEF_TRACE;

    MULTIPLAY::CGameListingEntry listing;
    return GetSelectedListing( listing );
}

/*-------------------------------------------------------------------------*/

bool
CServerBrowserForm::Join( void )
{GUCEF_TRACE;

    MULTIPLAY::CGameListingEntry listing;
    if ( !CanJoin() || !GetSelectedListing( listing ) )
        return false;
    NotifyObservers( JoinRequestedEvent, &listing );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CServerBrowserForm::Spectate( void )
{GUCEF_TRACE;

    MULTIPLAY::CGameListingEntry listing;
    if ( !GetSelectedListing( listing ) )
        return false;
    NotifyObservers( SpectateRequestedEvent, &listing );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CServerBrowserForm::RefreshListings( void )
{GUCEF_TRACE;

    return GUCEF_NULL != m_browser && m_browser->Refresh();
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::RequestCreate( void )
{GUCEF_TRACE;

    NotifyObservers( CreateRequestedEvent );
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::RequestDirectConnect( void )
{GUCEF_TRACE;

    NotifyObservers( DirectConnectRequestedEvent );
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::UpdateButtons( void )
{GUCEF_TRACE;

    GUI::CWidget* join = GetWidget( "Join" );
    if ( GUCEF_NULL != join )
        join->SetEnabled( CanJoin() );
    GUI::CWidget* spectate = GetWidget( "Spectate" );
    if ( GUCEF_NULL != spectate )
        spectate->SetEnabled( CanSpectate() );
    GUI::CWidget* refresh = GetWidget( "Refresh" );
    if ( GUCEF_NULL != refresh )
        refresh->SetEnabled( GUCEF_NULL != m_browser );
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::OnBrowserListChanged( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    UpdateRows();
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::OnJoinActivated( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Join();
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::OnSpectateActivated( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Spectate();
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::OnCreateActivated( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RequestCreate();
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::OnDirectConnectActivated( CORE::CNotifier* notifier    ,
                                              const CORE::CEvent& eventId  ,
                                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RequestDirectConnect();
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::OnRefreshActivated( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RefreshListings();
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::OnFilterChanged( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updating )
        return;
    ReadFilterFromControls();
    UpdateRows();
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::OnSelectionChanged( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_updating )
        UpdateButtons();
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::OnColumnClicked( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    const GUI::CGridView::TColumnClickedEventData* clicked = static_cast< const GUI::CGridView::TColumnClickedEventData* >( eventData );
    Int32 column = (Int32) clicked->GetData().column;
    SetSort( (TColumn) column, column == m_sortColumn ? !m_sortAscending : true );
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::OnListingActivated( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUI::CRoutedEventData* routed = static_cast< GUI::CRoutedEventData* >( eventData );
    if ( GUCEF_NULL != routed && GUI::EVENTPHASE_TUNNEL == routed->phase )
        return;
    if ( !Join() )
    {
        MULTIPLAY::CGameListingEntry listing;
        if ( GetSelectedListing( listing ) && MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS == listing.kind )
            Spectate();
    }
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::OnNotify( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the standard events do not reach the handlers of the subscriptions
    if ( GUCEF_NULL != m_browser && notifier == m_browser && CORE::CINotifier::DestructionEvent == eventId )
    {
        m_browser = GUCEF_NULL;
        UpdateRows();
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CServerBrowserForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GAMEGUI::CServerBrowserForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
