/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
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
const CORE::CEvent CServerBrowserForm::SettingsRequestedEvent = "GUCEF::GAMEGUI::CServerBrowserForm::SettingsRequestedEvent";
const CORE::CEvent CServerBrowserForm::FilterChangedEvent = "GUCEF::GAMEGUI::CServerBrowserForm::FilterChangedEvent";

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
            case CServerBrowserForm::COLUMN_KIND:
            {
                // the words of the status when the listings have them, else the kind
                const CORE::CString statusA = a.extra.GetValueAlways( MULTIPLAY::CGameListingEntry::ExtraStatusText ).AsString();
                const CORE::CString statusB = b.extra.GetValueAlways( MULTIPLAY::CGameListingEntry::ExtraStatusText ).AsString();
                if ( !statusA.IsNULLOrEmpty() || !statusB.IsNULLOrEmpty() )
                    return CompareText( statusA, statusB );
                return CompareNumbers( a.kind, b.kind );
            }
            case CServerBrowserForm::COLUMN_PLAYERS:   return CompareNumbers( a.players, b.players );
            case CServerBrowserForm::COLUMN_MAP:       return CompareText( a.mapName, b.mapName );
            case CServerBrowserForm::COLUMN_MODE:      return CompareText( a.modeName, b.modeName );
            case CServerBrowserForm::COLUMN_PING:      return CompareNumbers( a.pingMs, b.pingMs );
            case CServerBrowserForm::COLUMN_LOCK:      return CompareNumbers( a.passworded ? 1 : 0, b.passworded ? 1 : 0 );
            case CServerBrowserForm::COLUMN_SOURCE:    return CompareText( a.backendName, b.backendName );
            case CServerBrowserForm::COLUMN_DETAIL:    return CompareText( a.extra.GetValueAlways( MULTIPLAY::CGameListingEntry::ExtraDetailText ).AsString(), b.extra.GetValueAlways( MULTIPLAY::CGameListingEntry::ExtraDetailText ).AsString() );
            default:                                   return 0;
        }
    }

    private:

    Int32 m_column;
    bool m_ascending;
};

/** The order a list of games played over time starts with: the rows that need the player first, the nearest end of the turn first, then the name */
class CAttentionLess
{
    public:

    bool operator()( const MULTIPLAY::CGameListingEntry& a, const MULTIPLAY::CGameListingEntry& b ) const
    {
        const bool attentionA = IsAttention( a );
        const bool attentionB = IsAttention( b );
        if ( attentionA != attentionB )
            return attentionA;
        if ( attentionA )
        {
            const Int64 deadlineA = DeadlineOf( a );
            const Int64 deadlineB = DeadlineOf( b );
            if ( deadlineA != deadlineB )
                return deadlineA < deadlineB;
        }
        const CORE::CString nameA = a.name.Lowercase();
        const CORE::CString nameB = b.name.Lowercase();
        if ( nameA != nameB )
            return nameA < nameB;
        return a.listingId < b.listingId;
    }

    static bool IsAttention( const MULTIPLAY::CGameListingEntry& listing )
    {
        return listing.extra.GetValueAlways( MULTIPLAY::CGameListingEntry::ExtraAttention ).AsBool( false, true );
    }

    /** The end of the turn; a row that says none ends last */
    static Int64 DeadlineOf( const MULTIPLAY::CGameListingEntry& listing )
    {
        const CORE::CString text = listing.extra.GetValueAlways( MULTIPLAY::CGameListingEntry::ExtraDeadline ).AsString();
        return text.IsNULLOrEmpty() ? 0x7FFFFFFFFFFFFFFFLL : CORE::StringToInt64( text );
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CServerBrowserCapabilities::CServerBrowserCapabilities( void )
    : showPing( true )
    , showLock( true )
    , showSource( true )
    , showFullFilter( true )
    , showDirectConnect( true )
    , showSettings( false )
    , showDetail( false )
    , showNote( false )
    , title( "Games" )
    , kindColumnTitle( "Kind" )
    , mapColumnTitle( "Map" )
    , modeColumnTitle( "Mode" )
    , sourceColumnTitle( "Source" )
    , detailColumnTitle( "Detail" )
    , joinText( "Join" )
    , spectateText( "Spectate" )
    , createText( "Create" )
    , settingsText( "Settings" )
    , viewLabels()
    , viewKeys()
    , quickFilterKey()
    , quickFilterText( "Your turn" )
    , attentionText()
    , defaultOrderByAttention( false )
    , columnWidths()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::RegisterEvents( void )
{GUCEF_TRACE;

    JoinRequestedEvent.Initialize();
    SpectateRequestedEvent.Initialize();
    CreateRequestedEvent.Initialize();
    DirectConnectRequestedEvent.Initialize();
    SettingsRequestedEvent.Initialize();
    FilterChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CServerBrowserForm::CServerBrowserForm( void )
    : GUI::CFormEx()
    , m_browser( GUCEF_NULL )
    , m_capabilities()
    , m_note()
    , m_noteIsProblem( false )
    , m_shownColumns()
    , m_filter()
    , m_listed()
    , m_sortColumn( COLUMN_NAME )
    , m_sortAscending( true )
    , m_playerSorted( false )
    , m_attentionTotal( 0 )
    , m_updating( false )
{GUCEF_TRACE;

    RegisterEvents();
    ReadShownColumns();
}

/*-------------------------------------------------------------------------*/

CServerBrowserForm::CServerBrowserForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_browser( GUCEF_NULL )
    , m_capabilities()
    , m_shownColumns()
    , m_filter()
    , m_listed()
    , m_sortColumn( COLUMN_NAME )
    , m_sortAscending( true )
    , m_playerSorted( false )
    , m_attentionTotal( 0 )
    , m_updating( false )
{GUCEF_TRACE;

    RegisterEvents();
    ReadShownColumns();
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
    const bool hasSecondRow = m_capabilities.showFullFilter || m_capabilities.showLock || m_capabilities.showPing;
    const Int32 noteTop = hasSecondRow ? filterTop2 + 40 : filterTop2;
    const Int32 listTop = m_capabilities.showNote ? noteTop + 56 : noteTop;

    // nine tenths of the screen (up to 2400 wide), from below a strip an application keeps along the top
    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "ServerBrowserForm", "0.5,0 0,56", "0.9,0 1,-72" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, m_capabilities.title );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_HAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MinSize, "0,640 0,420" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MaxSize, "0,2400 0,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Clip, "true" );

    // the filter bar, in two rows so it fits a narrow window: the text and the kind, then the options
    // a list of groups (the games of a mailbox) has a wider control, and the box that shows one group with one click between the text and the control
    const bool hasViews = !m_capabilities.viewLabels.empty();
    const bool hasQuick = hasViews && !m_capabilities.quickFilterKey.IsNULLOrEmpty();
    const Int32 kindWidth = hasViews ? 230 : 150;
    const Int32 quickWidth = 172;
    const Int32 textRight = 16 + ( kindWidth + 16 ) + ( hasQuick ? quickWidth + 8 : 0 ) + 16;
    CORE::CDataNode* text = GUI::CFormLayout::AddWidgetNode( *window, "Editbox", "FilterText", "0,16 0," + CORE::Int32ToString( filterTop ), "1," + CORE::Int32ToString( -textRight ) + " 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *text, GUI::EWidgetProperty_Tooltip, "Find in the name, map, mode and host" );
    if ( hasQuick )
    {
        CORE::CDataNode* quick = GUI::CFormLayout::AddWidgetNode( *window, "Checkbox", "FilterQuick", "1," + CORE::Int32ToString( -( kindWidth + 16 + 8 + quickWidth ) ) + " 0," + CORE::Int32ToString( filterTop ), "0," + CORE::Int32ToString( quickWidth ) + " 0,32" );
        GUI::CFormLayout::SetWidgetNodeProperty( *quick, GUI::EWidgetProperty_Text, m_capabilities.quickFilterText );
        GUI::CFormLayout::SetWidgetNodeProperty( *quick, GUI::EWidgetProperty_Tooltip, "Show only the games that are waiting for you" );
    }
    CORE::CDataNode* kind = GUI::CFormLayout::AddWidgetNode( *window, "Combobox", "FilterKind", "1," + CORE::Int32ToString( -( kindWidth + 16 ) ) + " 0," + CORE::Int32ToString( filterTop ), "0," + CORE::Int32ToString( kindWidth ) + " 0,32" );
    CORE::CString kindItems = "All\nLobbies\nIn progress";
    if ( hasViews )
    {
        kindItems.Clear();
        for ( size_t v=0; v<m_capabilities.viewLabels.size(); ++v )
            kindItems += ( 0 == v ? CORE::CString::Empty : CORE::CString( "\n" ) ) + m_capabilities.viewLabels[ v ];
    }
    GUI::CFormLayout::SetWidgetNodeProperty( *kind, GUI::EWidgetProperty_Items, kindItems );
    GUI::CFormLayout::SetWidgetNodeProperty( *kind, GUI::EWidgetProperty_SelectedIndex, "0" );
    Int32 x = 16;
    if ( m_capabilities.showFullFilter )
    {
        CORE::CDataNode* hideFull = GUI::CFormLayout::AddWidgetNode( *window, "Checkbox", "HideFull", "0," + CORE::Int32ToString( x ) + " 0," + CORE::Int32ToString( filterTop2 ), "0,130 0,32" );
        GUI::CFormLayout::SetWidgetNodeProperty( *hideFull, GUI::EWidgetProperty_Text, "Hide full" );
        x += 144;
    }
    if ( m_capabilities.showLock )
    {
        CORE::CDataNode* hideLocked = GUI::CFormLayout::AddWidgetNode( *window, "Checkbox", "HideLocked", "0," + CORE::Int32ToString( x ) + " 0," + CORE::Int32ToString( filterTop2 ), "0,190 0,32" );
        GUI::CFormLayout::SetWidgetNodeProperty( *hideLocked, GUI::EWidgetProperty_Text, "Hide passworded" );
        x += 204;
    }
    if ( m_capabilities.showPing )
    {
        CORE::CDataNode* pingLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "MaxPingLabel", "0," + CORE::Int32ToString( x ) + " 0," + CORE::Int32ToString( filterTop2 ), "0,90 0,32" );
        GUI::CFormLayout::SetWidgetNodeProperty( *pingLabel, GUI::EWidgetProperty_Text, "Max ping" );
        GUI::CFormLayout::SetWidgetNodeProperty( *pingLabel, GUI::EWidgetProperty_VTextAlign, "Center" );
        CORE::CDataNode* maxPing = GUI::CFormLayout::AddWidgetNode( *window, "Spinner", "MaxPing", "0," + CORE::Int32ToString( x + 94 ) + " 0," + CORE::Int32ToString( filterTop2 ), "0,120 0,32" );
        GUI::CFormLayout::SetWidgetNodeProperty( *maxPing, GUI::EWidgetProperty_Min, "0" );
        GUI::CFormLayout::SetWidgetNodeProperty( *maxPing, GUI::EWidgetProperty_Max, "2000" );
        GUI::CFormLayout::SetWidgetNodeProperty( *maxPing, GUI::EWidgetProperty_Step, "10" );
        GUI::CFormLayout::SetWidgetNodeProperty( *maxPing, GUI::EWidgetProperty_Value, "0" );
        GUI::CFormLayout::SetWidgetNodeProperty( *maxPing, GUI::EWidgetProperty_Tooltip, "0 is no limit" );
    }

    // what the source of the listings has to say: up to two lines in the width of the window
    if ( m_capabilities.showNote )
    {
        CORE::CDataNode* note = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Note", "0,16 0," + CORE::Int32ToString( noteTop ), "1,-32 0,52" );
        GUI::CFormLayout::SetWidgetNodeProperty( *note, GUI::EWidgetProperty_WordWrap, "true" );
        GUI::CFormLayout::SetWidgetNodeProperty( *note, GUI::EWidgetProperty_TextColour, m_noteIsProblem ? "255,200,110,255" : "170,175,190,255" );
        GUI::CFormLayout::SetWidgetNodeProperty( *note, GUI::EWidgetProperty_Text, m_note );
    }

    // the listings take what the filter and the button strip leave
    CORE::CDataNode* listings = GUI::CFormLayout::AddWidgetNode( *window, "GridView", "Listings", "0,16 0," + CORE::Int32ToString( listTop ),
                                                                 "1,-32 1," + CORE::Int32ToString( -( listTop + WindowFooterHeight + 8 ) ) );
    CORE::CString columns;
    for ( size_t i=0; i<m_shownColumns.size(); ++i )
    {
        if ( i > 0 )
            columns += "\n";
        columns += TitleOf( (TColumn) m_shownColumns[ i ] );
    }
    GUI::CFormLayout::SetWidgetNodeProperty( *listings, GUI::EWidgetProperty_Columns, columns );

    // the status and the buttons, from the right: a strip along the bottom edge that nothing else uses
    CORE::CDataNode* footer = GUI::CFormLayout::AddWidgetNode( *window, "Panel", "ButtonRow", "0,1 1," + CORE::Int32ToString( -( WindowFooterHeight + 1 ) ), "1,-2 0," + CORE::Int32ToString( WindowFooterHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *footer, GUI::EWidgetProperty_BackgroundColour, "18,20,26,255" );
    CORE::CDataNode* status = GUI::CFormLayout::AddWidgetNode( *footer, "Label", "Status", "0,16 0,8", "1,-32 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *status, GUI::EWidgetProperty_TextColour, "170,175,190,255" );

    // the buttons from the right: Join, Spectate, Create, Direct connect, Refresh, Settings; as wide as their words need
    struct SButton { const char* name; CORE::CString text; bool shown; Int32 minWidth; };
    SButton buttons[ 6 ];
    buttons[ 0 ].name = "Join";          buttons[ 0 ].text = m_capabilities.joinText;     buttons[ 0 ].shown = true;                                 buttons[ 0 ].minWidth = 90;
    buttons[ 1 ].name = "Spectate";      buttons[ 1 ].text = m_capabilities.spectateText; buttons[ 1 ].shown = true;                                 buttons[ 1 ].minWidth = 100;
    buttons[ 2 ].name = "Create";        buttons[ 2 ].text = m_capabilities.createText;   buttons[ 2 ].shown = true;                                 buttons[ 2 ].minWidth = 90;
    buttons[ 3 ].name = "DirectConnect"; buttons[ 3 ].text = "Direct connect";            buttons[ 3 ].shown = m_capabilities.showDirectConnect;     buttons[ 3 ].minWidth = 140;
    buttons[ 4 ].name = "Refresh";       buttons[ 4 ].text = "Refresh";                   buttons[ 4 ].shown = true;                                 buttons[ 4 ].minWidth = 90;
    buttons[ 5 ].name = "Settings";      buttons[ 5 ].text = m_capabilities.settingsText; buttons[ 5 ].shown = m_capabilities.showSettings;          buttons[ 5 ].minWidth = 110;
    Int32 right = 16;
    for ( UInt32 i=0; i<6; ++i )
    {
        if ( !buttons[ i ].shown )
            continue;
        Int32 width = 28 + 8 * (Int32) buttons[ i ].text.Length();
        if ( width < buttons[ i ].minWidth )
            width = buttons[ i ].minWidth;
        right += width;
        CORE::CDataNode* button = GUI::CFormLayout::AddWidgetNode( *footer, "Button", buttons[ i ].name, "1," + CORE::Int32ToString( -right ) + " 1,-52", "0," + CORE::Int32ToString( width ) + " 0,38" );
        GUI::CFormLayout::SetWidgetNodeProperty( *button, GUI::EWidgetProperty_Text, buttons[ i ].text );
        if ( 0 == i )
            GUI::CFormLayout::SetWidgetNodeProperty( *button, GUI::EWidgetProperty_StyleClass, "Primary" );
        right += 8;
    }
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::SetCapabilities( const CServerBrowserCapabilities& capabilities )
{GUCEF_TRACE;

    m_capabilities = capabilities;
    if ( m_capabilities.viewKeys.size() != m_capabilities.viewLabels.size() )
        m_capabilities.viewKeys.resize( m_capabilities.viewLabels.size() );
    // the order a list of games over time starts with is not the order of a column: until the player picks a column
    if ( !m_playerSorted )
    {
        m_sortColumn = m_capabilities.defaultOrderByAttention ? -1 : COLUMN_NAME;
        m_sortAscending = true;
    }
    ReadShownColumns();
    if ( GUCEF_NULL != GetRootWidget() )
        ReloadLayout();
}

/*-------------------------------------------------------------------------*/

const CServerBrowserCapabilities&
CServerBrowserForm::GetCapabilities( void ) const
{GUCEF_TRACE;

    return m_capabilities;
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::ReadShownColumns( void )
{GUCEF_TRACE;

    m_shownColumns.clear();
    for ( Int32 column=0; column<(Int32) COLUMN_COUNT; ++column )
    {
        bool shown = true;
        switch ( column )
        {
            case COLUMN_PING:   shown = m_capabilities.showPing; break;
            case COLUMN_LOCK:   shown = m_capabilities.showLock; break;
            case COLUMN_SOURCE: shown = m_capabilities.showSource; break;
            case COLUMN_DETAIL: shown = m_capabilities.showDetail; break;
            default:            break;
        }
        if ( shown )
            m_shownColumns.push_back( column );
    }
}

/*-------------------------------------------------------------------------*/

bool
CServerBrowserForm::IsColumnShown( TColumn column ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_shownColumns.size(); ++i )
        if ( m_shownColumns[ i ] == (Int32) column )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::GetShownColumns( GUCEF::vector< Int32 >& columns ) const
{GUCEF_TRACE;

    columns = m_shownColumns;
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::GetColumnWidths( GUCEF::vector< Int32 >& widths ) const
{GUCEF_TRACE;

    static const Int32 usual[ COLUMN_COUNT ] = { 300 /* name */, 120 /* kind */, 90 /* players */, 150 /* map */, 130 /* mode */, 80 /* ping */, 90 /* lock */, 110 /* source */, 200 /* detail */ };
    widths.clear();
    for ( size_t i=0; i<m_shownColumns.size(); ++i )
    {
        const Int32 column = m_shownColumns[ i ];
        const Int32 given = column >= 0 && (size_t) column < m_capabilities.columnWidths.size() ? m_capabilities.columnWidths[ column ] : 0;
        widths.push_back( given > 0 ? given : usual[ column ] );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServerBrowserForm::TitleOf( TColumn column ) const
{GUCEF_TRACE;

    switch ( column )
    {
        case COLUMN_NAME:     return "Name";
        case COLUMN_KIND:     return m_capabilities.kindColumnTitle;
        case COLUMN_PLAYERS:  return "Players";
        case COLUMN_MAP:      return m_capabilities.mapColumnTitle;
        case COLUMN_MODE:     return m_capabilities.modeColumnTitle;
        case COLUMN_PING:     return "Ping";
        case COLUMN_LOCK:     return "Lock";
        case COLUMN_SOURCE:   return m_capabilities.sourceColumnTitle;
        case COLUMN_DETAIL:   return m_capabilities.detailColumnTitle;
        default:              return CORE::CString::Empty;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServerBrowserForm::ExtraText( const MULTIPLAY::CGameListingEntry& listing, const char* key )
{GUCEF_TRACE;

    return listing.extra.HasKey( key ) ? listing.extra.GetValueAlways( key ).AsString() : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CServerBrowserForm::ExtraFlag( const MULTIPLAY::CGameListingEntry& listing, const char* key, bool defaultValue )
{GUCEF_TRACE;

    if ( !listing.extra.HasKey( key ) )
        return defaultValue;
    const CORE::CString value = listing.extra.GetValueAlways( key ).AsString();
    return "true" == value || "1" == value;
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::ReloadLayout( void )
{GUCEF_TRACE;

    const bool wasShown = IsVisible();
    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return;
    if ( wasShown )
        Show();
    else
        Hide();
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::SetNote( const CORE::CString& text, bool problem )
{GUCEF_TRACE;

    m_note = text;
    m_noteIsProblem = problem;
    GUI::CWidget* note = GetWidget( "Note" );
    if ( GUCEF_NULL != note )
    {
        if ( note->GetProperty( GUI::EWidgetProperty_Text ) != text )
            note->SetProperty( GUI::EWidgetProperty_Text, text );
        note->SetProperty( GUI::EWidgetProperty_TextColour, problem ? "255,200,110,255" : "170,175,190,255" );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServerBrowserForm::GetNoteText( void )
{GUCEF_TRACE;

    GUI::CWidget* note = GetWidget( "Note" );
    return GUCEF_NULL != note ? note->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::RequestSettings( void )
{GUCEF_TRACE;

    NotifyObservers( SettingsRequestedEvent );
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::OnSettingsActivated( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RequestSettings();
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
    button = GetWidget( "Settings" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CServerBrowserForm::OnSettingsActivated );
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
    GUI::CWidget* quick = GetWidget( "FilterQuick" );
    if ( GUCEF_NULL != quick )
    {
        TEventCallback quickCallback( this, &CServerBrowserForm::OnQuickFilterChanged );
        SubscribeTo( quick, GUI::CWidget::ValueChangedEvent, quickCallback );
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
        GetColumnWidths( widths );
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
        SubscribeTo( m_browser, MULTIPLAY::CGameBrowser::BrowserListSettledEvent, callback );
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
        if ( !m_capabilities.viewLabels.empty() )
        {
            for ( size_t v=0; v<m_capabilities.viewKeys.size(); ++v )
                if ( m_capabilities.viewKeys[ v ] == m_filter.group )
                    index = (Int32) v;
        }
        else
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
        if ( !m_capabilities.viewLabels.empty() )
        {
            const Int32 picked = kind->GetSelectedIndex();
            m_filter.group = picked >= 0 && picked < (Int32) m_capabilities.viewKeys.size() ? m_capabilities.viewKeys[ picked ] : CORE::CString::Empty;
        }
        else
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

    const CORE::CString words = ExtraText( listing, MULTIPLAY::CGameListingEntry::ExtraStatusText );
    if ( !words.IsNULLOrEmpty() )
        return words;
    return MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS == listing.kind ? "In progress" : "Lobby";
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::SortListed( void )
{GUCEF_TRACE;

    if ( m_sortColumn < 0 )
        std::stable_sort( m_listed.begin(), m_listed.end(), CAttentionLess() );
    else
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
    UInt32 attentionTotal = 0;
    m_listed.clear();
    if ( GUCEF_NULL != m_browser )
    {
        TGameListingEntryVector all;
        m_browser->GetListings( all );
        available = (UInt32) all.size();
        m_listed.reserve( all.size() );
        for ( UInt32 i=0; i<(UInt32)all.size(); ++i )
        {
            if ( CAttentionLess::IsAttention( all[ i ] ) )
                ++attentionTotal;
            if ( m_filter.Matches( all[ i ] ) )
                m_listed.push_back( all[ i ] );
        }
    }
    SortListed();

    if ( GUCEF_NULL == grid )
        return;

    bool wasUpdating = m_updating;
    m_updating = true;

    GUI::CGridView::TRowVector rows;
    rows.reserve( m_listed.size() );
    GUI::CGridView::TInt32Vector emphasized;
    UInt32 attentionListed = 0;
    Int32 selectedRow = -1;
    for ( UInt32 i=0; i<(UInt32)m_listed.size(); ++i )
    {
        const MULTIPLAY::CGameListingEntry& listing = m_listed[ i ];
        if ( CAttentionLess::IsAttention( listing ) )
        {
            emphasized.push_back( (Int32) i );
            ++attentionListed;
        }
        GUI::CGridView::TStringVector all;
        all.resize( COLUMN_COUNT );
        all[ COLUMN_NAME ] = listing.name;
        all[ COLUMN_KIND ] = KindText( listing );
        all[ COLUMN_PLAYERS ] = FormatPlayers( listing.players, listing.maxPlayers );
        all[ COLUMN_MAP ] = listing.mapName;
        all[ COLUMN_MODE ] = listing.modeName;
        all[ COLUMN_PING ] = FormatPing( listing.pingMs );
        all[ COLUMN_LOCK ] = listing.passworded ? "Locked" : "";
        all[ COLUMN_SOURCE ] = m_capabilities.showDetail && !listing.hostName.IsNULLOrEmpty() ? listing.hostName : listing.backendName;
        all[ COLUMN_DETAIL ] = ExtraText( listing, MULTIPLAY::CGameListingEntry::ExtraDetailText );
        GUI::CGridView::TStringVector cells;
        for ( size_t c=0; c<m_shownColumns.size(); ++c )
            cells.push_back( all[ m_shownColumns[ c ] ] );
        rows.push_back( cells );
        if ( !selectedId.IsNULLOrEmpty() && listing.listingId == selectedId )
            selectedRow = (Int32) i;
    }
    grid->SetRows( rows );
    grid->SetEmphasizedRows( emphasized );
    Int32 sortIndex = -1;
    for ( size_t c=0; c<m_shownColumns.size(); ++c )
        if ( m_shownColumns[ c ] == m_sortColumn )
            sortIndex = (Int32) c;
    grid->SetIntProperty( GUI::EWidgetProperty_SortColumn, sortIndex );
    grid->SetBoolProperty( GUI::EWidgetProperty_SortAscending, m_sortAscending );
    grid->SetSelectedRow( selectedRow );

    GUI::CWidget* status = GetWidget( "Status" );
    if ( GUCEF_NULL != status )
    {
        CORE::CString text = CORE::UInt32ToString( (UInt32) m_listed.size() ) + ( 1 == m_listed.size() ? " game" : " games" );
        if ( !m_capabilities.attentionText.IsNULLOrEmpty() && attentionListed > 0 )
            text += ", " + CORE::UInt32ToString( attentionListed ) + " " + m_capabilities.attentionText;
        if ( available > (UInt32) m_listed.size() )
            text += " (" + CORE::UInt32ToString( available - (UInt32) m_listed.size() ) + " hidden by the filter)";
        status->SetProperty( GUI::EWidgetProperty_Text, text );
    }

    m_updating = wasUpdating;
    m_attentionTotal = attentionTotal;
    ShowQuickFilter();
    UpdateButtons();
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::ShowQuickFilter( void )
{GUCEF_TRACE;

    GUI::CWidget* quick = GetWidget( "FilterQuick" );
    if ( GUCEF_NULL == quick )
        return;
    bool wasUpdating = m_updating;
    m_updating = true;
    quick->SetBoolProperty( GUI::EWidgetProperty_Checked, m_filter.group == m_capabilities.quickFilterKey );
    // the number of games of that group, in the words of the box
    const CORE::CString text = m_capabilities.quickFilterText + " (" + CORE::UInt32ToString( m_attentionTotal ) + ")";
    if ( quick->GetProperty( GUI::EWidgetProperty_Text ) != text )
        quick->SetProperty( GUI::EWidgetProperty_Text, text );
    m_updating = wasUpdating;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServerBrowserForm::GetFilterGroup( void ) const
{GUCEF_TRACE;

    return m_filter.group;
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
    m_playerSorted = true;
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
    if ( !GetSelectedListing( listing ) )
        return false;
    return ExtraFlag( listing, MULTIPLAY::CGameListingEntry::ExtraCanJoin, MULTIPLAY::CGameListingEntry::KIND_LOBBY == listing.kind && !listing.IsFull() );
}

/*-------------------------------------------------------------------------*/

bool
CServerBrowserForm::CanSpectate( void ) const
{GUCEF_TRACE;

    MULTIPLAY::CGameListingEntry listing;
    return GetSelectedListing( listing ) && ExtraFlag( listing, MULTIPLAY::CGameListingEntry::ExtraCanSpectate, true );
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
    if ( !CanSpectate() || !GetSelectedListing( listing ) )
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
    {
        // the row can say what joining it is called
        MULTIPLAY::CGameListingEntry listing;
        CORE::CString joinText = m_capabilities.joinText;
        if ( GetSelectedListing( listing ) )
        {
            const CORE::CString rowText = ExtraText( listing, MULTIPLAY::CGameListingEntry::ExtraJoinText );
            if ( !rowText.IsNULLOrEmpty() )
                joinText = rowText;
        }
        if ( join->GetProperty( GUI::EWidgetProperty_Text ) != joinText )
            join->SetProperty( GUI::EWidgetProperty_Text, joinText );
        join->SetEnabled( CanJoin() );
    }
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
    const CORE::CString before = m_filter.group;
    ReadFilterFromControls();
    UpdateRows();
    if ( before != m_filter.group )
    {
        TFilterChangedEventData group( m_filter.group );
        NotifyObservers( FilterChangedEvent, &group );
    }
}

/*-------------------------------------------------------------------------*/

void
CServerBrowserForm::OnQuickFilterChanged( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updating )
        return;
    GUI::CWidget* quick = GetWidget( "FilterQuick" );
    if ( GUCEF_NULL == quick )
        return;
    // one click: only the games of that group; the next click: all of them again
    m_filter.group = quick->GetBoolProperty( GUI::EWidgetProperty_Checked ) ? m_capabilities.quickFilterKey : CORE::CString::Empty;
    ShowFilterInControls();
    UpdateRows();
    TFilterChangedEventData group( m_filter.group );
    NotifyObservers( FilterChangedEvent, &group );
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
    const UInt32 shownIndex = clicked->GetData().column;
    if ( shownIndex >= m_shownColumns.size() )
        return;
    const Int32 column = m_shownColumns[ shownIndex ];
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
