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

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHID_H
#include "gucefMULTIPLAY_CMatchId.h"
#define GUCEF_MULTIPLAY_CMATCHID_H
#endif /* GUCEF_MULTIPLAY_CMATCHID_H ? */

#ifndef GUCEF_GAMEGUI_UTILS_H
#include "gucefGAMEGUI_Utils.h"
#define GUCEF_GAMEGUI_UTILS_H
#endif /* GUCEF_GAMEGUI_UTILS_H ? */

#include "gucefGAMEGUI_CMatchIndicatorForm.h"

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

const CORE::CEvent CMatchIndicatorForm::OpenRequestedEvent = "GUCEF::GAMEGUI::CMatchIndicatorForm::OpenRequestedEvent";
const CORE::CEvent CMatchIndicatorForm::MyGamesRequestedEvent = "GUCEF::GAMEGUI::CMatchIndicatorForm::MyGamesRequestedEvent";
const CORE::CEvent CMatchIndicatorForm::BadgeRequestedEvent = "GUCEF::GAMEGUI::CMatchIndicatorForm::BadgeRequestedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const Int32 Gap = 4;
const Int32 EntryGap = 2;
const Int32 TimeWidth = 88;
const Int32 EntryTextPadding = 24;
const UInt32 TimesIntervalMs = 1000;
const char* const NormalTextColour = "226,229,236,255";
const char* const OverdueColour = "255,110,110,255";

CORE::CString
UDimText( const char* x, Int32 xPixels, const char* y, Int32 yPixels )
{
    return CORE::CString( x ) + "," + CORE::Int32ToString( xPixels ) + " " + y + "," + CORE::Int32ToString( yPixels );
}

/** The game that has waited the longest first, ties by match id */
class CWaitedLonger
{
    public:

    bool operator()( const MULTIPLAY::CMatchEntry& a, const MULTIPLAY::CMatchEntry& b ) const
    {
        if ( a.yourTurnSince != b.yourTurnSince )
            return a.yourTurnSince < b.yourTurnSince;
        return a.preview.matchId < b.preview.matchId;
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::RegisterEvents( void )
{GUCEF_TRACE;

    OpenRequestedEvent.Initialize();
    MyGamesRequestedEvent.Initialize();
    BadgeRequestedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CMatchIndicatorForm::SEntry::SEntry( void )
    : button( GUCEF_NULL )
    , time( GUCEF_NULL )
    , matchId()
    , source()
    , textShown()
    , timeShown()
    , deadline( 0 )
    , shown( false )
    , overdueShown( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchIndicatorForm::CMatchIndicatorForm( void )
    : GUI::CFormEx()
    , m_book( GUCEF_NULL )
    , m_clock( GUCEF_NULL )
    , m_entries()
    , m_entryOfButton()
    , m_badge( GUCEF_NULL )
    , m_popup( GUCEF_NULL )
    , m_myGames( GUCEF_NULL )
    , m_anchor( ANCHOR_TOP_RIGHT )
    , m_offsetX( 12 )
    , m_offsetY( 12 )
    , m_maxListed( DefaultMaxListed )
    , m_count( 0 )
    , m_listedShown( 0 )
    , m_sinceTimesMs( 0 )
    , m_badgeOpensList( true )
    , m_hideWhenEmpty( true )
    , m_open( false )
    , m_popupOpen( false )
    , m_badgeShownNow( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CMatchIndicatorForm::CMatchIndicatorForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_book( GUCEF_NULL )
    , m_clock( GUCEF_NULL )
    , m_entries()
    , m_entryOfButton()
    , m_badge( GUCEF_NULL )
    , m_popup( GUCEF_NULL )
    , m_myGames( GUCEF_NULL )
    , m_anchor( ANCHOR_TOP_RIGHT )
    , m_offsetX( 12 )
    , m_offsetY( 12 )
    , m_maxListed( DefaultMaxListed )
    , m_count( 0 )
    , m_listedShown( 0 )
    , m_sinceTimesMs( 0 )
    , m_badgeOpensList( true )
    , m_hideWhenEmpty( true )
    , m_open( false )
    , m_popupOpen( false )
    , m_badgeShownNow( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CMatchIndicatorForm::~CMatchIndicatorForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    // a badge, not a window of the player: no title bar, so it cannot be moved or folded, and nothing around it
    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "MatchIndicator", "1,-12 0,12", UDimText( "0", BadgeWidth, "0", BadgeHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_HAlign, "Right" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_TitleVisible, "false" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Movable, "false" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Collapsible, "false" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_BackgroundColour, "0,0,0,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_BorderColour, "0,0,0,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_BorderWidth, "0" );

    CORE::CDataNode* badge = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Badge", "0,0 0,0", UDimText( "0", BadgeWidth, "0", BadgeHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *badge, GUI::EWidgetProperty_Text, "Your turn: 0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *badge, GUI::EWidgetProperty_StyleClass, "Primary" );

    CORE::CDataNode* popup = GUI::CFormLayout::AddWidgetNode( *window, "Panel", "Popup", UDimText( "0", 0, "0", BadgeHeight + Gap ), UDimText( "0", ListWidth, "0", 2 * Gap + EntryHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *popup, GUI::EWidgetProperty_BackgroundColour, "28,31,38,245" );
    GUI::CFormLayout::SetWidgetNodeProperty( *popup, GUI::EWidgetProperty_BorderColour, "74,136,232,255" );
    GUI::CFormLayout::SetWidgetNodeProperty( *popup, GUI::EWidgetProperty_BorderWidth, "1" );
    GUI::CFormLayout::SetWidgetNodeProperty( *popup, GUI::EWidgetProperty_Visible, "false" );
    CORE::CDataNode* myGames = GUI::CFormLayout::AddWidgetNode( *popup, "Button", "MyGames", UDimText( "0", Gap, "0", Gap ), UDimText( "0", ListWidth - 2 * Gap, "0", EntryHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *myGames, GUI::EWidgetProperty_Text, "My Games" );
}

/*-------------------------------------------------------------------------*/

bool
CMatchIndicatorForm::Build( void )
{GUCEF_TRACE;

    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;
    Hide();
    m_badgeShownNow = false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchIndicatorForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;
    m_open = true;
    m_sinceTimesMs = 0;
    Refresh();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::Close( void )
{GUCEF_TRACE;

    m_open = false;
    m_popupOpen = false;
    Refresh();
}

/*-------------------------------------------------------------------------*/

bool
CMatchIndicatorForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

bool
CMatchIndicatorForm::IsBadgeShown( void ) const
{GUCEF_TRACE;

    return m_badgeShownNow;
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    // the widgets of a layout that was there before are gone
    m_entries.clear();
    m_entryOfButton.clear();

    m_badge = GetWidget( "Badge" );
    m_popup = GetWidget( "Popup" );
    m_myGames = GetWidget( "MyGames" );

    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context )
    {
        TEventCallback callback( this, &CMatchIndicatorForm::OnFrameStarted );
        SubscribeTo( context, GUI::CGUIContext::FrameStartedEvent, callback );
    }
    if ( GUCEF_NULL != m_badge )
    {
        TEventCallback callback( this, &CMatchIndicatorForm::OnBadgeActivated );
        SubscribeTo( m_badge, GUI::CWidget::ActivatedEvent, callback );
    }
    if ( GUCEF_NULL != m_myGames )
    {
        TEventCallback callback( this, &CMatchIndicatorForm::OnMyGamesActivated );
        SubscribeTo( m_myGames, GUI::CWidget::ActivatedEvent, callback );
    }
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::SetBook( MULTIPLAY::CMatchBook* book )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_book )
        UnsubscribeFrom( m_book );
    m_book = book;
    if ( GUCEF_NULL != m_book )
    {
        TEventCallback changedCallback( this, &CMatchIndicatorForm::OnBookChanged );
        SubscribeTo( m_book, MULTIPLAY::CMatchBook::MatchBookChangedEvent, changedCallback );
        TEventCallback turnCallback( this, &CMatchIndicatorForm::OnYourTurn );
        SubscribeTo( m_book, MULTIPLAY::CMatchBook::YourTurnEvent, turnCallback );
    }
    Refresh();
}

/*-------------------------------------------------------------------------*/

MULTIPLAY::CMatchBook*
CMatchIndicatorForm::GetBook( void ) const
{GUCEF_TRACE;

    return m_book;
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::SetClock( const MULTIPLAY::CIMatchClock* clock )
{GUCEF_TRACE;

    m_clock = clock;
    Refresh();
}

/*-------------------------------------------------------------------------*/

Int64
CMatchIndicatorForm::GetNowMs( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_clock ? m_clock->GetNowMs() : MULTIPLAY::CMatchSystemClock::Instance().GetNowMs();
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::SetAnchor( TAnchor corner, Int32 offsetX, Int32 offsetY )
{GUCEF_TRACE;

    m_anchor = corner;
    m_offsetX = offsetX < 0 ? 0 : offsetX;
    m_offsetY = offsetY < 0 ? 0 : offsetY;
    Place();
}

/*-------------------------------------------------------------------------*/

CMatchIndicatorForm::TAnchor
CMatchIndicatorForm::GetAnchor( void ) const
{GUCEF_TRACE;

    return m_anchor;
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::SetHideWhenEmpty( bool hideWhenEmpty )
{GUCEF_TRACE;

    m_hideWhenEmpty = hideWhenEmpty;
    Refresh();
}

/*-------------------------------------------------------------------------*/

bool
CMatchIndicatorForm::GetHideWhenEmpty( void ) const
{GUCEF_TRACE;

    return m_hideWhenEmpty;
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::SetMaxListed( UInt32 maxListed )
{GUCEF_TRACE;

    m_maxListed = maxListed < 1 ? 1 : ( maxListed > LargestMaxListed ? LargestMaxListed : maxListed );
    Refresh();
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchIndicatorForm::GetMaxListed( void ) const
{GUCEF_TRACE;

    return m_maxListed;
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchIndicatorForm::GetYourTurnCount( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_book ? m_book->GetYourTurnCount() : 0;
}

/*-------------------------------------------------------------------------*/

bool
CMatchIndicatorForm::IsPopupOpen( void ) const
{GUCEF_TRACE;

    return m_popupOpen;
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::GetOrdered( MULTIPLAY::TMatchEntryVector& entries ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_book )
        return;
    m_book->GetGroup( MULTIPLAY::MATCHGROUP_YOUR_TURN, entries, false );
    std::stable_sort( entries.begin(), entries.end(), CWaitedLonger() );
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::GetListedMatchIds( GUCEF::vector< CORE::CString >& matchIds ) const
{GUCEF_TRACE;

    matchIds.clear();
    MULTIPLAY::TMatchEntryVector entries;
    GetOrdered( entries );
    for ( size_t i=0; i<entries.size() && i<m_maxListed; ++i )
        matchIds.push_back( entries[ i ].preview.matchId );
}

/*-------------------------------------------------------------------------*/

bool
CMatchIndicatorForm::EnsureEntries( UInt32 count )
{GUCEF_TRACE;

    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == context || GUCEF_NULL == m_popup )
        return false;

    while ( m_entries.size() < count )
    {
        const UInt32 k = (UInt32) m_entries.size();
        const CORE::CString suffix = CORE::UInt32ToString( k );
        SEntry entry;
        entry.button = context->CreateWidget( "Button", "Entry" + suffix, m_popup );
        entry.time = context->CreateWidget( "Label", "EntryTime" + suffix, m_popup );
        if ( GUCEF_NULL == entry.button || GUCEF_NULL == entry.time )
            return false;
        entry.button->SetVisibility( false );
        entry.time->SetVisibility( false );
        entry.time->SetProperty( GUI::EWidgetProperty_HTextAlign, "Right" );
        entry.time->SetProperty( GUI::EWidgetProperty_VTextAlign, "Center" );
        entry.time->SetProperty( GUI::EWidgetProperty_TextColour, NormalTextColour );
        entry.time->SetBoolProperty( GUI::EWidgetProperty_InputTransparent, true );
        const Int32 top = Gap + (Int32) k * ( EntryHeight + EntryGap );
        entry.button->SetProperty( GUI::EWidgetProperty_Position, UDimText( "0", Gap, "0", top ) );
        entry.button->SetProperty( GUI::EWidgetProperty_Size, UDimText( "0", ListWidth - 2 * Gap - TimeWidth, "0", EntryHeight ) );
        entry.time->SetProperty( GUI::EWidgetProperty_Position, UDimText( "0", ListWidth - Gap - TimeWidth, "0", top ) );
        entry.time->SetProperty( GUI::EWidgetProperty_Size, UDimText( "0", TimeWidth - 8, "0", EntryHeight ) );
        TEventCallback callback( this, &CMatchIndicatorForm::OnEntryActivated );
        SubscribeTo( entry.button, GUI::CWidget::ActivatedEvent, callback );
        m_entryOfButton[ entry.button ] = k;
        m_entries.push_back( entry );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::ShowPopup( void )
{GUCEF_TRACE;

    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == context || GUCEF_NULL == m_popup )
        return;

    MULTIPLAY::TMatchEntryVector entries;
    if ( m_popupOpen )
        GetOrdered( entries );
    const UInt32 listed = (UInt32) entries.size() < m_maxListed ? (UInt32) entries.size() : m_maxListed;
    if ( !EnsureEntries( listed ) )
        return;

    const Int64 now = GetNowMs();
    for ( UInt32 k=0; k<(UInt32) m_entries.size(); ++k )
    {
        SEntry& slot = m_entries[ k ];
        const bool shown = k < listed;
        if ( shown != slot.shown )
        {
            slot.shown = shown;
            slot.button->SetVisibility( shown );
            slot.time->SetVisibility( shown );
        }
        if ( !shown )
        {
            slot.matchId.Clear();
            continue;
        }

        const MULTIPLAY::CMatchEntry& entry = entries[ k ];
        slot.matchId = entry.preview.matchId;
        slot.deadline = entry.preview.deadline;
        const CORE::CString& displayTitle = entry.GetDisplayTitle();
        const CORE::CString title = displayTitle.IsNULLOrEmpty() ? CORE::CString( "Game " + MULTIPLAY::CMatchId::Short( entry.preview.matchId ) ) : displayTitle;
        if ( slot.source != title || slot.textShown.IsNULLOrEmpty() )
        {
            slot.source = title;
            slot.textShown = EllipsizeText( *context, slot.button->GetProperty( GUI::EWidgetProperty_Font ), title, ListWidth - 2 * Gap - TimeWidth - EntryTextPadding );
            slot.button->SetProperty( GUI::EWidgetProperty_Text, slot.textShown );
        }
        const CORE::CString timeLeft = slot.deadline > 0 ? FormatTimeLeft( slot.deadline - now ) : CORE::CString::Empty;
        if ( slot.timeShown != timeLeft )
        {
            slot.timeShown = timeLeft;
            slot.time->SetProperty( GUI::EWidgetProperty_Text, timeLeft );
        }
        const bool overdue = "Overdue" == timeLeft;
        if ( overdue != slot.overdueShown )
        {
            slot.overdueShown = overdue;
            slot.time->SetProperty( GUI::EWidgetProperty_TextColour, overdue ? OverdueColour : NormalTextColour );
        }
    }

    if ( GUCEF_NULL != m_myGames )
    {
        const CORE::CString text = entries.size() > listed ? "More games ..." : "My Games";
        if ( m_myGames->GetPropertyRef( GUI::EWidgetProperty_Text ) != text )
            m_myGames->SetProperty( GUI::EWidgetProperty_Text, text );
        const Int32 top = Gap + (Int32) listed * ( EntryHeight + EntryGap );
        m_myGames->SetProperty( GUI::EWidgetProperty_Position, UDimText( "0", Gap, "0", top ) );
    }
    m_listedShown = listed;
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::ShowTimes( void )
{GUCEF_TRACE;

    const Int64 now = GetNowMs();
    for ( size_t k=0; k<m_entries.size(); ++k )
    {
        SEntry& slot = m_entries[ k ];
        if ( !slot.shown )
            continue;
        const CORE::CString timeLeft = slot.deadline > 0 ? FormatTimeLeft( slot.deadline - now ) : CORE::CString::Empty;
        if ( slot.timeShown != timeLeft )
        {
            slot.timeShown = timeLeft;
            slot.time->SetProperty( GUI::EWidgetProperty_Text, timeLeft );
            const bool overdue = "Overdue" == timeLeft;
            if ( overdue != slot.overdueShown )
            {
                slot.overdueShown = overdue;
                slot.time->SetProperty( GUI::EWidgetProperty_TextColour, overdue ? OverdueColour : NormalTextColour );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::Place( void )
{GUCEF_TRACE;

    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL == root || GUCEF_NULL == m_badge || GUCEF_NULL == m_popup )
        return;

    const bool right = ANCHOR_TOP_RIGHT == m_anchor || ANCHOR_BOTTOM_RIGHT == m_anchor;
    const bool bottom = ANCHOR_BOTTOM_LEFT == m_anchor || ANCHOR_BOTTOM_RIGHT == m_anchor;
    const bool popup = m_popupOpen && m_badgeShownNow;
    const Int32 popupHeight = popup ? 2 * Gap + ( (Int32) m_listedShown + 1 ) * EntryHeight + (Int32) m_listedShown * EntryGap : 0;
    const Int32 rootWidth = popup ? ListWidth : BadgeWidth;
    const Int32 rootHeight = popup ? BadgeHeight + Gap + popupHeight : BadgeHeight;

    // the window is as large as what it shows and stays at its corner
    root->SetProperty( GUI::EWidgetProperty_HAlign, right ? "Right" : "Left" );
    root->SetProperty( GUI::EWidgetProperty_VAlign, bottom ? "Bottom" : "Top" );
    root->SetProperty( GUI::EWidgetProperty_Position, UDimText( right ? "1" : "0", right ? -m_offsetX : m_offsetX, bottom ? "1" : "0", bottom ? -m_offsetY : m_offsetY ) );
    root->SetProperty( GUI::EWidgetProperty_Size, UDimText( "0", rootWidth, "0", rootHeight ) );

    m_badge->SetProperty( GUI::EWidgetProperty_Position, UDimText( "0", right ? rootWidth - BadgeWidth : 0, "0", bottom ? rootHeight - BadgeHeight : 0 ) );
    m_badge->SetProperty( GUI::EWidgetProperty_Size, UDimText( "0", BadgeWidth, "0", BadgeHeight ) );
    if ( m_popup->IsVisible() != popup )
        m_popup->SetVisibility( popup );
    if ( popup )
    {
        m_popup->SetProperty( GUI::EWidgetProperty_Position, UDimText( "0", 0, "0", bottom ? 0 : BadgeHeight + Gap ) );
        m_popup->SetProperty( GUI::EWidgetProperty_Size, UDimText( "0", ListWidth, "0", popupHeight ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::ShowBadge( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_badge )
        return;
    const CORE::CString text = m_count > 99 ? CORE::CString( "Your turn: 99+" ) : "Your turn: " + CORE::UInt32ToString( m_count );
    if ( m_badge->GetPropertyRef( GUI::EWidgetProperty_Text ) != text )
        m_badge->SetProperty( GUI::EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::Refresh( void )
{GUCEF_TRACE;

    m_count = GetYourTurnCount();
    if ( GUCEF_NULL == GetRootWidget() )
        return;

    const bool shown = m_open && ( m_count > 0 || !m_hideWhenEmpty );
    m_badgeShownNow = shown;
    if ( !shown )
        m_popupOpen = false;

    ShowBadge();
    ShowPopup();
    Place();

    if ( shown && !IsVisible() )
        Show();
    else
    if ( !shown && IsVisible() )
        Hide();
}

/*-------------------------------------------------------------------------*/

bool
CMatchIndicatorForm::TogglePopup( void )
{GUCEF_TRACE;

    if ( !m_badgeShownNow )
        return false;
    m_popupOpen = !m_popupOpen;
    m_sinceTimesMs = 0;
    ShowPopup();
    Place();
    return m_popupOpen;
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::ClosePopup( void )
{GUCEF_TRACE;

    if ( !m_popupOpen )
        return;
    m_popupOpen = false;
    ShowPopup();
    Place();
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::Update( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( !m_open || !m_popupOpen )
        return;
    m_sinceTimesMs += deltaInMs;
    if ( m_sinceTimesMs >= TimesIntervalMs )
    {
        m_sinceTimesMs = 0;
        ShowTimes();
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::OnBookChanged( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::OnYourTurn( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::OnFrameStarted( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != eventData )
        Update( static_cast< CORE::TCloneableUInt32* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::OnBadgeActivated( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_badgeOpensList )
    {
        ClosePopup();
        NotifyObservers( BadgeRequestedEvent );
        return;
    }
    TogglePopup();
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::SetBadgeOpensList( bool opensList )
{GUCEF_TRACE;

    m_badgeOpensList = opensList;
}

/*-------------------------------------------------------------------------*/

bool
CMatchIndicatorForm::GetBadgeOpensList( void ) const
{GUCEF_TRACE;

    return m_badgeOpensList;
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::OnEntryActivated( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    TWidgetEntryMap::const_iterator found = m_entryOfButton.find( static_cast< GUI::CWidget* >( notifier ) );
    if ( found == m_entryOfButton.end() || (*found).second >= m_entries.size() )
        return;
    const CORE::CString matchId = m_entries[ (*found).second ].matchId;
    if ( matchId.IsNULLOrEmpty() )
        return;
    ClosePopup();
    TOpenRequestedEventData data( matchId );
    NotifyObservers( OpenRequestedEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::OnMyGamesActivated( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ClosePopup();
    NotifyObservers( MyGamesRequestedEvent );
}

/*-------------------------------------------------------------------------*/

void
CMatchIndicatorForm::OnNotify( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the standard events do not reach the handlers of the subscriptions
    if ( GUCEF_NULL != m_book && notifier == m_book && CORE::CINotifier::DestructionEvent == eventId )
    {
        m_book = GUCEF_NULL;
        Refresh();
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchIndicatorForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GAMEGUI::CMatchIndicatorForm";
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
