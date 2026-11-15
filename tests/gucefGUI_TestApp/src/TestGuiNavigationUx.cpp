/*
 *  gucefGUI_TestApp: test application for the gucefGUI module
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

/*
 *  Navigation aids: tab controls sized to their titles with an overflow menu and badges, keyboard shortcuts, two
 *  levels of tabs as one navigation, the history of visited places, the breadcrumb bar; and the empty state of a chart,
 *  the date axis, select all of an edit box and the badges of tree items. Widths are computed by hand with the bitmap
 *  test font of 8 x 10 pixels.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI_TestGridSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GridTest;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/** Keeps the texts that came with the events of one notifier: a TCloneableString, or with 'items' the item of a CItemEventData with its index */
class CEventTexts : public CORE::CObserver
{
    public:

    GUCEF::vector< CORE::CString > texts;
    GUCEF::vector< Int32 > indices;

    CEventTexts( CORE::CNotifier* notifier, const CORE::CEvent& eventId, bool items = false )
        : CORE::CObserver(), texts(), indices(), m_eventId( eventId ), m_items( items )
    {
        SubscribeTo( notifier, eventId );
    }

    virtual ~CEventTexts() GUCEF_VIRTUAL_OVERRIDE { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventId != m_eventId || GUCEF_NULL == eventData )
            return;
        if ( m_items )
        {
            const GUI::CItemEventData* item = static_cast< const GUI::CItemEventData* >( eventData );
            texts.push_back( item->item );
            indices.push_back( item->index );
            return;
        }
        const CORE::TCloneableString* text = static_cast< const CORE::TCloneableString* >( eventData );
        texts.push_back( text->GetData() );
        indices.push_back( -1 );
    }

    CORE::CString Last( void ) const { return texts.empty() ? CORE::CString::Empty : texts.back(); }

    private:

    CORE::CEvent m_eventId;
    bool m_items;
};

/** Keeps the places of the NavigatedEvent of a history */
class CNavigatedLog : public CORE::CObserver
{
    public:

    GUCEF::vector< GUI::CNavigationHistory::TNavigated > moves;
    UInt32 changes;

    explicit CNavigatedLog( GUI::CNavigationHistory* history )
        : CORE::CObserver(), moves(), changes( 0 )
    {
        SubscribeTo( history, GUI::CNavigationHistory::NavigatedEvent );
        SubscribeTo( history, GUI::CNavigationHistory::HistoryChangedEvent );
    }

    virtual ~CNavigatedLog() GUCEF_VIRTUAL_OVERRIDE { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUI::CNavigationHistory::HistoryChangedEvent == eventId )
        {
            ++changes;
            return;
        }
        if ( GUI::CNavigationHistory::NavigatedEvent != eventId || GUCEF_NULL == eventData )
            return;
        GUI::CNavigationHistory::TNavigatedEventData* data = static_cast< GUI::CNavigationHistory::TNavigatedEventData* >( eventData );
        moves.push_back( data->GetData() );
    }
};

void
PlaceWidget( GUI::CWidget* widget, Int32 left, Int32 top, Int32 width, Int32 height )
{
    widget->SetPosition( GUI::CUVector( GUI::CUDim( 0.0f, left ), GUI::CUDim( 0.0f, top ) ) );
    widget->SetSize( GUI::CUVector( GUI::CUDim( 0.0f, width ), GUI::CUDim( 0.0f, height ) ) );
    widget->SetProperty( "Font", "Test" );
}

GUI::CTabControl*
MakeTabs( GUI::CGUIContext* context, GUI::CWidget* parent, const char* name, const char* titles, Int32 width )
{
    GUI::CTabControl* tabs = GUI::CWidget::As< GUI::CTabControl >( context->CreateWidget( "TabControl", name, parent ) );
    if ( GUCEF_NULL == tabs )
        return GUCEF_NULL;
    PlaceWidget( tabs, 0, 0, width, 300 );
    tabs->SetProperty( "TabHeight", "20" );         // the dark theme of the test context has 36; the rectangles below are computed for 20
    tabs->SetProperty( "Tabs", titles );
    return tabs;
}

bool
HasFillOfColour( const GUI::CDrawSurface::TCommandVector& commands, const GUI::CRect& rect, const GUI::CColour& colour )
{
    for ( size_t i=0; i<commands.size(); ++i )
        if ( GUI::CDrawSurface::COMMAND_FILLRECT == commands[ i ].type && commands[ i ].rect == rect && commands[ i ].colour == colour )
            return true;
    return false;
}

bool
HasCommandOfType( const GUI::CDrawSurface::TCommandVector& commands, GUI::CDrawSurface::ECommandType type )
{
    for ( size_t i=0; i<commands.size(); ++i )
        if ( type == commands[ i ].type )
            return true;
    return false;
}

const GUI::CDrawSurface::TCommand*
FindTextCommand( const GUI::CDrawSurface::TCommandVector& commands, const CORE::CString& text )
{
    for ( size_t i=0; i<commands.size(); ++i )
        if ( GUI::CDrawSurface::COMMAND_TEXT == commands[ i ].type && text == commands[ i ].text )
            return &commands[ i ];
    return GUCEF_NULL;
}

/** Clicks the item of the open popup menu of the context */
bool
ClickPopupItem( GUI::CGUIContext* context, UInt32 index )
{
    GUI::CPopupMenu* menu = GUI::CWidget::As< GUI::CPopupMenu >( context->GetPopup() );
    if ( GUCEF_NULL == menu )
        return false;
    context->FlushChanges();
    const GUI::CRect& rect = menu->GetRect();
    ClickAt( context, rect.left + 10, rect.top + (Int32) index * menu->GetRowHeight() + menu->GetRowHeight() / 2 );
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiNavigationUxTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.NavigationUx" );

    GUCEF_TESTFW_TESTCASE( "NavigationUx: TabSizing Content gives tabs the width of their titles and badges, an overflow button lists every tab, the tabs shown follow the active one; Equal is drawn as before" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CTabControl* tabs = MakeTabs( context, screen, "Tabs", "Overview\nPositions\nIncome\nNet income\nTax", 400 );
        ASSERT_TRUE( GUCEF_NULL != tabs );
        if ( GUCEF_NULL == tabs )
            return;
        context->FlushChanges();

        // Equal: the tabs share the width, the headers and titles drawn as before, no overflow
        ASSERT_TRUE( !tabs->IsContentSized() && GUI::CRect( 80, 0, 80, 20 ) == tabs->GetTabRect( 1 ) && !tabs->HasOverflow() );
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, tabs );
        for ( UInt32 i=0; i<5; ++i )
        {
            const GUI::CRect header = tabs->GetTabRect( i );
            const GUI::CDrawSurface::TCommand* title = FindTextCommand( commands, tabs->GetTabTitle( i ) );
            ASSERT_TRUE( GUCEF_NULL != title && header.left + ( header.width - TextWidth( context, tabs->GetTabTitle( i ) ) ) / 2 == title->x0 && 5 == title->y0 );
        }
        ASSERT_FALSE( HasCommandOfType( commands, GUI::CDrawSurface::COMMAND_LINE ) );         // no overflow button

        // Content: the title and 4 pixels at each side, at least 48: 72, 80, 56, 88 and 48
        tabs->SetProperty( "TabSizing", "Content" );
        context->FlushChanges();
        ASSERT_TRUE( tabs->IsContentSized() && !tabs->HasOverflow() && tabs->GetOverflowButtonRect().IsEmpty() );
        ASSERT_TRUE( GUI::CRect( 0, 0, 72, 20 ) == tabs->GetTabRect( 0 ) && GUI::CRect( 72, 0, 80, 20 ) == tabs->GetTabRect( 1 ) && GUI::CRect( 152, 0, 56, 20 ) == tabs->GetTabRect( 2 ) );
        ASSERT_TRUE( GUI::CRect( 208, 0, 88, 20 ) == tabs->GetTabRect( 3 ) && GUI::CRect( 296, 0, 48, 20 ) == tabs->GetTabRect( 4 ) );
        ASSERT_FALSE( tabs->IsTabTitleCut( 3 ) );

        // a count on Income: 8 + 2 * 4 + 4 more pixels, the pill right in the tab, the title keeps its room
        ASSERT_TRUE( tabs->SetTabBadge( 2, "3" ) && "3" == tabs->GetTabBadge( 2 ) && 20 == tabs->GetTabBadgeRoom( 2 ) );
        ASSERT_TRUE( GUI::CRect( 152, 0, 76, 20 ) == tabs->GetTabRect( 2 ) && GUI::CRect( 208, 4, 16, 12 ) == tabs->GetTabBadgeRect( 2 ) );
        ASSERT_TRUE( 48 == tabs->GetTabTextRoom( 2 ) && !tabs->IsTabTitleCut( 2 ) );
        // a dot on Tax: the tab stays at its least width
        ASSERT_TRUE( tabs->SetTabBadge( 4, "*" ) && GUI::CRect( 316, 0, 48, 20 ) == tabs->GetTabRect( 4 ) && GUI::CRect( 352, 6, 8, 8 ) == tabs->GetTabBadgeRect( 4 ) );
        commands = PaintedCommands( context, tabs );
        ASSERT_TRUE( HasFillOfColour( commands, GUI::CRect( 208, 4, 16, 12 ), GUI::CTabControlPainter::BadgeColour ) );
        ASSERT_TRUE( HasFillOfColour( commands, GUI::CRect( 352, 6, 8, 8 ), GUI::CTabControlPainter::BadgeColour ) );
        const GUI::CDrawSurface::TCommand* count = FindTextCommand( commands, "3" );
        ASSERT_TRUE( GUCEF_NULL != count && 212 == count->x0 && 5 == count->y0 && GUI::CTabControlPainter::BadgeTextColour == count->colour );
        const GUI::CDrawSurface::TCommand* income = FindTextCommand( commands, "Income" );
        ASSERT_TRUE( GUCEF_NULL != income && 156 == income->x0 );
        ASSERT_TRUE( 5 == tabs->GetStringListProperty( GUI::EWidgetProperty_TabBadges ).size() );
        ASSERT_TRUE( tabs->SetTabBadge( 4, "" ) && 3 == tabs->GetStringListProperty( GUI::EWidgetProperty_TabBadges ).size() && tabs->SetTabBadge( 4, "*" ) );
        ASSERT_FALSE( tabs->SetTabBadge( 5, "1" ) );

        // 200 pixels: the tabs overflow; the button takes 20, Overview and Positions fit in the 180 left
        PlaceWidget( tabs, 0, 0, 200, 300 );
        context->FlushChanges();
        ASSERT_TRUE( tabs->HasOverflow() && GUI::CRect( 180, 0, 20, 20 ) == tabs->GetOverflowButtonRect() );
        ASSERT_TRUE( tabs->IsTabVisible( 0 ) && tabs->IsTabVisible( 1 ) && !tabs->IsTabVisible( 2 ) && tabs->GetTabRect( 2 ).IsEmpty() && 0 == tabs->GetFirstVisibleTab() );

        // the active tab is always shown whole: Tax active shows Net income and Tax
        tabs->SetActiveTab( 4 );
        ASSERT_TRUE( 3 == tabs->GetFirstVisibleTab() && GUI::CRect( 0, 0, 88, 20 ) == tabs->GetTabRect( 3 ) && GUI::CRect( 88, 0, 48, 20 ) == tabs->GetTabRect( 4 ) );
        ASSERT_FALSE( tabs->IsTabVisible( 0 ) );

        // the button opens the menu of every tab, the active one marked; a choice activates the tab
        ClickAt( context, 190, 10 );
        ASSERT_TRUE( tabs->IsOverflowMenuOpen() );
        GUI::CPopupMenu* menu = GUI::CWidget::As< GUI::CPopupMenu >( context->GetPopup() );
        ASSERT_TRUE( GUCEF_NULL != menu );
        if ( GUCEF_NULL != menu )
        {
            const GUI::CPopupMenu::TStringVector items = menu->GetItems();
            ASSERT_TRUE( 5 == items.size() && "  Overview" == items[ 0 ] && "  Income  (3)" == items[ 2 ] && "* Tax  (new)" == items[ 4 ] );
        }
        ASSERT_TRUE( ClickPopupItem( context, 1 ) );
        ASSERT_TRUE( 1 == tabs->GetActiveTab() && !tabs->IsOverflowMenuOpen() && tabs->IsTabVisible( 1 ) );
        commands = PaintedCommands( context, tabs );
        ASSERT_TRUE( HasCommandOfType( commands, GUI::CDrawSurface::COMMAND_LINE ) );          // the sign of the overflow button
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NavigationUx: A shortcut map binds one chord per action and one action per chord, ignores lock keys, raises the action and describes the chord" )
    try
    {
        GUI::CKeyboardShortcutMap map;
        CEventTexts activated( &map, GUI::CKeyboardShortcutMap::ShortcutActivatedEvent );
        ASSERT_TRUE( map.Bind( "area.1", INPUT::KEYCODE_1, INPUT::KEYMOD_CTRL ) );
        ASSERT_FALSE( map.Bind( "", INPUT::KEYCODE_2, INPUT::KEYMOD_CTRL ) || map.Bind( "x", INPUT::KEYCODE_UNASSIGNED, 0 ) );
        ASSERT_TRUE( "area.1" == map.FindAction( INPUT::KEYCODE_1, (UInt32) INPUT::KEYMOD_CTRL | (UInt32) INPUT::KEYMOD_NUMLOCK ) );
        ASSERT_TRUE( map.FindAction( INPUT::KEYCODE_1, 0 ).IsNULLOrEmpty() && map.FindAction( INPUT::KEYCODE_1, INPUT::KEYMOD_ALT ).IsNULLOrEmpty() );
        ASSERT_TRUE( "Ctrl+1" == map.GetShortcutText( "area.1" ) );

        ASSERT_TRUE( map.HandleKey( INPUT::KEYCODE_1, INPUT::KEYMOD_CTRL ) && 1 == activated.texts.size() && "area.1" == activated.Last() );
        ASSERT_FALSE( map.HandleKey( INPUT::KEYCODE_2, INPUT::KEYMOD_CTRL ) );
        GUI::CKeyEventData key;
        key.keyCode = INPUT::KEYCODE_1;
        key.modifiers = INPUT::KEYMOD_CTRL;
        ASSERT_TRUE( map.HandleKeyEvent( key ) && 2 == activated.texts.size() );

        // the chord moves to another action; a new chord replaces the old one of an action
        ASSERT_TRUE( map.Bind( "other", INPUT::KEYCODE_1, INPUT::KEYMOD_CTRL ) && "other" == map.FindAction( INPUT::KEYCODE_1, INPUT::KEYMOD_CTRL ) );
        INPUT::KeyCode chordKey = INPUT::KEYCODE_UNASSIGNED;
        UInt32 chordModifiers = 0;
        ASSERT_FALSE( map.GetChord( "area.1", chordKey, chordModifiers ) );
        ASSERT_TRUE( map.Bind( "other", INPUT::KEYCODE_LEFT, INPUT::KEYMOD_ALT ) && 1 == map.GetBindingCount() );
        ASSERT_TRUE( map.GetChord( "other", chordKey, chordModifiers ) && INPUT::KEYCODE_LEFT == chordKey && (UInt32) INPUT::KEYMOD_ALT == chordModifiers );
        ASSERT_TRUE( "Alt+Left" == map.GetShortcutText( "other" ) );
        ASSERT_TRUE( map.Unbind( "other" ) && !map.Unbind( "other" ) && 0 == map.GetBindingCount() );

        // the texts of chords
        ASSERT_TRUE( "Shift+F10" == GUI::CKeyboardShortcutMap::DescribeChord( INPUT::KEYCODE_F10, INPUT::KEYMOD_SHIFT ) );
        ASSERT_TRUE( "Ctrl+Shift+Tab" == GUI::CKeyboardShortcutMap::DescribeChord( INPUT::KEYCODE_TAB, (UInt32) INPUT::KEYMOD_CTRL | (UInt32) INPUT::KEYMOD_SHIFT ) );
        ASSERT_TRUE( "Ctrl+D" == GUI::CKeyboardShortcutMap::DescribeChord( INPUT::KEYCODE_D, INPUT::KEYMOD_CTRL ) && "Menu" == GUI::CKeyboardShortcutMap::DescribeKey( INPUT::KEYCODE_APPS ) );
        map.Bind( "a", INPUT::KEYCODE_A, 0 );
        map.Clear();
        ASSERT_TRUE( 0 == map.GetBindingCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NavigationUx: The area navigator selects a page on both levels, returns to the sub-view last shown in an area, cycles sub-views, follows clicks on tabs and does the default shortcuts" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CTabControl* areas = MakeTabs( context, screen, "Areas", "Home\nHoldings\nIncome", 600 );
        ASSERT_TRUE( GUCEF_NULL != areas );
        if ( GUCEF_NULL == areas )
            return;
        context->CreateWidget( "TabContentPane", "HomePane", areas );
        GUI::CWidget* holdingsPane = context->CreateWidget( "TabContentPane", "HoldingsPane", areas );
        GUI::CWidget* incomePane = context->CreateWidget( "TabContentPane", "IncomePane", areas );
        GUI::CTabControl* holdings = MakeTabs( context, holdingsPane, "HoldingsViews", "Positions\nLots", 600 );
        GUI::CTabControl* income = MakeTabs( context, incomePane, "IncomeViews", "Received\nForward", 600 );
        ASSERT_TRUE( GUCEF_NULL != holdings && GUCEF_NULL != income );
        if ( GUCEF_NULL == holdings || GUCEF_NULL == income )
            return;
        context->FlushChanges();

        GUI::CTabAreaNavigator navigator;
        CEventTexts pages( &navigator, GUI::CTabAreaNavigator::PageChangedEvent );
        ASSERT_TRUE( navigator.SetAreaControl( areas ) && navigator.SetSubViewControl( 1, holdings ) && navigator.SetSubViewControl( 2, income ) );
        ASSERT_TRUE( navigator.AddPage( "overview", 0, -1 ) && navigator.AddPage( "positions", 1, 0 ) && navigator.AddPage( "lots", 1, 1 ) );
        ASSERT_TRUE( navigator.AddPage( "income", 2, 0 ) && navigator.AddPage( "projection", 2, 1 ) && !navigator.AddPage( "", 0, 0 ) );
        ASSERT_TRUE( "overview" == navigator.GetCurrentPage() && "lots" == navigator.GetPageAt( 1, 1 ) );
        UInt32 area = 0;
        Int32 subView = 0;
        ASSERT_TRUE( navigator.FindPage( "projection", area, subView ) && 2 == area && 1 == subView );
        const size_t before = pages.texts.size();

        // a page on both levels, one event
        ASSERT_TRUE( navigator.SelectPage( "lots" ) );
        ASSERT_TRUE( 1 == areas->GetActiveTab() && 1 == holdings->GetActiveTab() && "lots" == navigator.GetCurrentPage() );
        ASSERT_TRUE( before + 1 == pages.texts.size() && "lots" == pages.Last() );
        ASSERT_FALSE( navigator.SelectPage( "unknown" ) );

        // back to the area: the sub-view last shown
        ASSERT_TRUE( navigator.SelectArea( 0 ) && "overview" == navigator.GetCurrentPage() && "overview" == pages.Last() );
        ASSERT_TRUE( navigator.SelectArea( 1 ) && "lots" == navigator.GetCurrentPage() );
        ASSERT_FALSE( navigator.SelectArea( 3 ) );

        // round the sub-views
        ASSERT_TRUE( navigator.NextSubView( false ) && "positions" == navigator.GetCurrentPage() );
        ASSERT_TRUE( navigator.NextSubView( true ) && "lots" == navigator.GetCurrentPage() );

        // a tab chosen on a control: the page follows and the area remembers it
        holdings->SetActiveTab( 0 );
        ASSERT_TRUE( "positions" == navigator.GetCurrentPage() && "positions" == pages.Last() );
        ASSERT_TRUE( navigator.SelectArea( 2 ) && "income" == navigator.GetCurrentPage() );
        ASSERT_TRUE( navigator.SelectArea( 1 ) && "positions" == navigator.GetCurrentPage() );
        areas->SetActiveTab( 2 );
        ASSERT_TRUE( "income" == pages.Last() );

        // the default shortcuts: Ctrl+1 .. Ctrl+9, Ctrl+Tab, Ctrl+Shift+Tab
        GUI::CKeyboardShortcutMap map;
        navigator.BindDefaultShortcuts( map );
        ASSERT_TRUE( 11 == map.GetBindingCount() && "area.3" == map.FindAction( INPUT::KEYCODE_3, INPUT::KEYMOD_CTRL ) );
        ASSERT_TRUE( "subview.previous" == map.FindAction( INPUT::KEYCODE_TAB, (UInt32) INPUT::KEYMOD_CTRL | (UInt32) INPUT::KEYMOD_SHIFT ) );
        ASSERT_TRUE( navigator.HandleShortcut( "area.2" ) && "positions" == navigator.GetCurrentPage() );
        ASSERT_TRUE( navigator.HandleShortcut( "area.3" ) && navigator.HandleShortcut( "subview.next" ) && "projection" == navigator.GetCurrentPage() );
        ASSERT_FALSE( navigator.HandleShortcut( "area.9" ) || navigator.HandleShortcut( "bogus" ) );
        ASSERT_FALSE( navigator.SelectArea( 0 ) && navigator.NextSubView( false ) );     // the home area has no sub-views

        navigator.SetSubViewControl( 1, GUCEF_NULL );
        navigator.SetSubViewControl( 2, GUCEF_NULL );
        navigator.SetAreaControl( GUCEF_NULL );
        ASSERT_TRUE( navigator.GetCurrentPage().IsNULLOrEmpty() && GUCEF_NULL == navigator.GetAreaControl() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NavigationUx: The navigation history goes back and forward as a browser does, drops the places ahead on a new visit, keeps a maximum and tells what to show" )
    try
    {
        GUI::CNavigationHistory history( 4 );
        CNavigatedLog log( &history );
        ASSERT_TRUE( !history.HasCurrent() && -1 == history.GetCurrentIndex() && history.GetCurrent().location.IsNULLOrEmpty() );
        ASSERT_FALSE( history.Back() || history.Forward() || history.Replace( "x" ) );

        history.Visit( "portfolio", "Portfolio" );
        history.Visit( "account:1", "Account 1" );
        history.Visit( "position:7", "ACME" );
        ASSERT_TRUE( 3 == history.GetCount() && 2 == history.GetCurrentIndex() && history.CanGoBack() && !history.CanGoForward() && 3 == log.changes && log.moves.empty() );

        // the same place again: nothing new, the title may change
        history.Visit( "position:7", "ACME Corp" );
        ASSERT_TRUE( 3 == history.GetCount() && "ACME Corp" == history.GetCurrent().title );

        // back: the application is told where to go
        ASSERT_TRUE( history.Back() && 1 == log.moves.size() && "account:1" == log.moves[ 0 ].location && GUI::CNavigationHistory::NAVIGATION_BACK == log.moves[ 0 ].kind && 1 == log.moves[ 0 ].index );
        ASSERT_TRUE( history.CanGoForward() && "account:1" == history.GetCurrent().location );
        ASSERT_TRUE( history.Forward() && "position:7" == log.moves.back().location && GUI::CNavigationHistory::NAVIGATION_FORWARD == log.moves.back().kind );
        ASSERT_TRUE( history.Back() && history.Back() && !history.CanGoBack() && "portfolio" == history.GetCurrent().location );

        // the places ahead and behind, the nearest first
        GUI::CNavigationHistory::TEntryVector ahead = history.GetForwardEntries();
        ASSERT_TRUE( 2 == ahead.size() && "account:1" == ahead[ 0 ].location && "position:7" == ahead[ 1 ].location );

        // a new visit after going back drops the places ahead
        history.Visit( "tax", "Tax" );
        ASSERT_TRUE( 2 == history.GetCount() && !history.CanGoForward() && "tax" == history.GetCurrent().location );
        GUI::CNavigationHistory::TEntryVector behind = history.GetBackEntries();
        ASSERT_TRUE( 1 == behind.size() && "portfolio" == behind[ 0 ].location );

        // at most 4: the oldest goes
        history.Visit( "a" );
        history.Visit( "b" );
        history.Visit( "c" );
        ASSERT_TRUE( 4 == history.GetCount() && "tax" == history.GetEntry( 0 ).location && 3 == history.GetCurrentIndex() );
        history.SetMaximumEntries( 2 );
        ASSERT_TRUE( 2 == history.GetCount() && "b" == history.GetEntry( 0 ).location && 1 == history.GetCurrentIndex() );
        ASSERT_TRUE( history.GetEntry( 9 ).location.IsNULLOrEmpty() );

        // a jump and a replace
        const size_t moves = log.moves.size();
        ASSERT_TRUE( history.GoToIndex( 0 ) && "b" == history.GetCurrent().location && moves + 1 == log.moves.size() && GUI::CNavigationHistory::NAVIGATION_JUMP == log.moves.back().kind );
        ASSERT_FALSE( history.GoToIndex( 0 ) || history.GoToIndex( 5 ) );
        ASSERT_TRUE( history.Replace( "b2", "B2" ) && "b2" == history.GetCurrent().location && 2 == history.GetCount() && moves + 1 == log.moves.size() );

        history.Clear();
        ASSERT_TRUE( 0 == history.GetCount() && !history.HasCurrent() );
        history.SetMaximumEntries( 0 );
        for ( UInt32 i=0; i<300; ++i )
            history.Visit( CORE::UInt32ToString( i ) );
        ASSERT_TRUE( 300 == history.GetCount() && 0 == history.GetMaximumEntries() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NavigationUx: The breadcrumb bar lays its items out, collapses the leading ones into a menu when narrow, cuts the last one when it must, and raises clicks by mouse, menu and keys" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CBreadcrumbBar* bar = GUI::CWidget::As< GUI::CBreadcrumbBar >( context->CreateWidget( "BreadcrumbBar", "Crumbs", screen ) );
        ASSERT_TRUE( GUCEF_NULL != bar && bar->IsRegisteredAs( "DrawSurface" ) );
        if ( GUCEF_NULL == bar )
            return;
        PlaceWidget( bar, 0, 0, 400, 24 );
        context->FlushChanges();
        CEventTexts clicked( bar, GUI::CBreadcrumbBar::ItemClickedEvent, true );

        GUI::CBreadcrumbBar::TItemVector items;
        items.push_back( GUI::CBreadcrumbBar::TItem( "home", "Home" ) );
        items.push_back( GUI::CBreadcrumbBar::TItem( "holdings", "Holdings" ) );
        items.push_back( GUI::CBreadcrumbBar::TItem( "positions", "Positions" ) );
        items.push_back( GUI::CBreadcrumbBar::TItem( "acme", "ACME Corp" ) );
        bar->SetItems( items );
        bar->Layout( 400, 24 );

        // the text and 6 pixels at each side, 16 pixels for the ">" between them
        ASSERT_TRUE( 4 == bar->GetItemCount() && GUI::CRect( 0, 0, 44, 24 ) == bar->GetItemRect( 0 ) && GUI::CRect( 60, 0, 76, 24 ) == bar->GetItemRect( 1 ) );
        ASSERT_TRUE( GUI::CRect( 152, 0, 84, 24 ) == bar->GetItemRect( 2 ) && GUI::CRect( 252, 0, 84, 24 ) == bar->GetItemRect( 3 ) && bar->GetOverflowRect().IsEmpty() );
        const GUI::CDrawSurface::TCommandVector& drawn = bar->GetCommands();
        const GUI::CDrawSurface::TCommand* home = FindTextCommand( drawn, "Home" );
        const GUI::CDrawSurface::TCommand* acme = FindTextCommand( drawn, "ACME Corp" );
        ASSERT_TRUE( GUCEF_NULL != home && GUCEF_NULL != acme && 6 == home->x0 && 7 == home->y0 && home->colour != acme->colour );
        ASSERT_TRUE( GUCEF_NULL != FindTextCommand( drawn, ">" ) );

        // a click on an item before the last raises it; the last item is where the user is
        ASSERT_FALSE( bar->ClickItem( 3 ) || bar->ClickItem( 9 ) );
        ASSERT_TRUE( bar->ClickItem( 1 ) && 1 == clicked.texts.size() && "holdings" == clicked.Last() && 1 == clicked.indices.back() );

        // 250 pixels: Holdings and Positions go into the "..." item
        PlaceWidget( bar, 0, 0, 250, 24 );
        context->FlushChanges();
        bar->Layout( 250, 24 );
        ASSERT_TRUE( GUI::CRect( 60, 0, 36, 24 ) == bar->GetOverflowRect() && GUI::CRect( 112, 0, 84, 24 ) == bar->GetItemRect( 3 ) );
        ASSERT_TRUE( bar->IsItemCollapsed( 1 ) && bar->IsItemCollapsed( 2 ) && !bar->IsItemCollapsed( 3 ) && bar->GetItemRect( 1 ).IsEmpty() );
        ASSERT_TRUE( GUI::CBreadcrumbBar::OverflowItem == bar->GetItemAt( 70, 10 ) && 3 == bar->GetItemAt( 120, 10 ) && -1 == bar->GetItemAt( 220, 10 ) );

        // 150 pixels: the last item is cut as well
        bar->Layout( 150, 24 );
        ASSERT_TRUE( GUI::CRect( 112, 0, 38, 24 ) == bar->GetItemRect( 3 ) && "..." == bar->GetShownText( 3 ) );
        bar->Layout( 250, 24 );
        ASSERT_TRUE( "ACME Corp" == bar->GetShownText( 3 ) );

        // the "..." item opens the menu of the collapsed items; a choice clicks the item
        ClickAt( context, 70, 12 );
        ASSERT_TRUE( bar->IsOverflowMenuOpen() );
        GUI::CPopupMenu* menu = GUI::CWidget::As< GUI::CPopupMenu >( context->GetPopup() );
        ASSERT_TRUE( GUCEF_NULL != menu && 2 == menu->GetItems().size() && "Holdings" == menu->GetItems()[ 0 ] && "Positions" == menu->GetItems()[ 1 ] );
        ASSERT_TRUE( ClickPopupItem( context, 1 ) );
        ASSERT_TRUE( !bar->IsOverflowMenuOpen() && "positions" == clicked.Last() && 2 == clicked.indices.back() );

        // a click on the first item
        ClickAt( context, 10, 12 );
        ASSERT_TRUE( "home" == clicked.Last() && 0 == clicked.indices.back() );

        // the keys: Right to the first item, again to "...", Enter opens the menu; Left back, Enter clicks
        context->SetFocus( bar );
        PressKey( context, INPUT::KEYCODE_RIGHT );
        ASSERT_TRUE( 0 == bar->GetFocusedItem() );
        PressKey( context, INPUT::KEYCODE_RIGHT );
        ASSERT_TRUE( GUI::CBreadcrumbBar::OverflowItem == bar->GetFocusedItem() );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( bar->IsOverflowMenuOpen() );
        context->ClosePopup();
        context->SetFocus( bar );
        const size_t clicks = clicked.texts.size();
        PressKey( context, INPUT::KEYCODE_LEFT );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( clicks + 1 == clicked.texts.size() && "home" == clicked.Last() );

        bar->ClearItems();
        ASSERT_TRUE( 0 == bar->GetItemCount() && bar->GetItem( 0 ).id.IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NavigationUx: A chart without data shows its EmptyText and no axes or ticks; a date axis never has ticks finer than a day" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CChartView* chart = GUI::CWidget::As< GUI::CChartView >( context->CreateWidget( "ChartView", "Chart", GUCEF_NULL ) );
        ASSERT_TRUE( GUCEF_NULL != chart );
        if ( GUCEF_NULL == chart )
            return;
        ASSERT_TRUE( "No data" == chart->GetProperty( "EmptyText" ) );

        // nothing to show: the text, no lines (no axes, no grid, no ticks)
        chart->Rebuild( 400, 200 );
        ASSERT_TRUE( chart->IsShowingEmptyState() && GUCEF_NULL != FindTextCommand( chart->GetCommands(), "No data" ) );
        ASSERT_FALSE( HasCommandOfType( chart->GetCommands(), GUI::CDrawSurface::COMMAND_LINE ) );

        // data: the axes are back
        const Int64 day = 86400000;
        const UInt32 series = chart->AddSeries( "Value", GUI::CChartSeries::SERIESTYPE_LINE );
        Int64 x[ 2 ] = { GUI::CChartAxis::MakeTime( 2026, 11, 17 ), GUI::CChartAxis::MakeTime( 2026, 11, 18 ) };
        Float64 values[ 2 ] = { 1.0, 2.0 };
        ASSERT_TRUE( chart->SetSeriesData( series, x, values, 2 ) );
        chart->Rebuild( 400, 200 );
        ASSERT_TRUE( !chart->IsShowingEmptyState() && HasCommandOfType( chart->GetCommands(), GUI::CDrawSurface::COMMAND_LINE ) );
        ASSERT_TRUE( GUCEF_NULL == FindTextCommand( chart->GetCommands(), "No data" ) );

        // one day of data: the time axis picks hours, the date axis the day itself
        ASSERT_TRUE( chart->SetSeriesData( series, &x[ 1 ], &values[ 1 ], 1 ) );
        chart->Rebuild( 400, 200 );
        ASSERT_TRUE( GUI::CChartAxis::TIMEUNIT_DAY > chart->GetXAxis().GetTimeUnit() );
        chart->SetProperty( "XAxisType", "Date" );
        chart->Rebuild( 400, 200 );
        ASSERT_TRUE( GUI::CChartAxis::TIMEUNIT_DAY == chart->GetXAxis().GetTimeUnit() && GUI::CChartAxis::TIMEUNIT_DAY == chart->GetXAxis().GetMinimumTimeUnit() );
        const GUI::CChartAxis::TTickVector& ticks = chart->GetXAxis().GetTicks();
        ASSERT_FALSE( ticks.empty() );
        for ( UInt32 i=0; i<(UInt32)ticks.size(); ++i )
            ASSERT_TRUE( 0 == ( (Int64) ticks[ i ].value ) % day );

        // no data again, and no text asked for: nothing in the plot
        chart->RemoveAllSeries();
        chart->SetProperty( "EmptyText", "" );
        chart->Rebuild( 400, 200 );
        ASSERT_TRUE( chart->IsShowingEmptyState() && !HasCommandOfType( chart->GetCommands(), GUI::CDrawSurface::COMMAND_TEXT ) );
        context->DestroyWidget( chart );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NavigationUx: Ctrl+A selects all of the text of an edit box so typing replaces it; tree items carry badges at the right end of their row" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CEditbox* edit = GUI::CWidget::As< GUI::CEditbox >( context->CreateWidget( "Editbox", "Name", screen ) );
        ASSERT_TRUE( GUCEF_NULL != edit );
        if ( GUCEF_NULL == edit )
            return;
        PlaceWidget( edit, 0, 0, 300, 24 );
        edit->SetText( "Positions.csv" );
        context->FlushChanges();
        context->SetFocus( edit );

        // without a selection typing adds to the text
        context->InjectText( "x" );
        ASSERT_TRUE( "Positions.csvx" == edit->GetText() && !edit->GetBoolProperty( "Selected" ) );

        // Ctrl+A: the whole text is selected, a typed path replaces it
        Chord( context, INPUT::KEYCODE_A, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( edit->GetBoolProperty( "Selected" ) );
        context->InjectText( "/tmp/out.csv" );
        ASSERT_TRUE( "/tmp/out.csv" == edit->GetText() && !edit->GetBoolProperty( "Selected" ) );
        edit->Clear();
        edit->SelectAll();
        ASSERT_FALSE( edit->GetBoolProperty( "Selected" ) );          // nothing to select

        // tree badges: a dot and a count at the right end of the rows, the label cut before them
        GUI::CTreeview* tree = GUI::CWidget::As< GUI::CTreeview >( context->CreateWidget( "Treeview", "Tree", screen ) );
        ASSERT_TRUE( GUCEF_NULL != tree );
        if ( GUCEF_NULL == tree )
            return;
        PlaceWidget( tree, 0, 100, 200, 200 );
        tree->SetProperty( "Items", "Area/Sub\nOther" );
        tree->SetIntProperty( "RowHeight", 20 );
        ASSERT_TRUE( tree->SetItemBadge( "Other", "5" ) && tree->SetItemBadge( "Area", "*" ) );
        ASSERT_TRUE( "5" == tree->GetItemBadge( "Other" ) && "*" == tree->GetItemBadge( "Area" ) && tree->GetItemBadge( "Area/Sub" ).IsNULLOrEmpty() );
        ASSERT_TRUE( tree->SetItemBadge( "Other", "6" ) && "6" == tree->GetItemBadge( "Other" ) && 2 == tree->GetStringListProperty( GUI::EWidgetProperty_ItemBadges ).size() );
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, tree );
        const GUI::CColour accent( 74, 136, 232, 255 );
        ASSERT_TRUE( HasFillOfColour( commands, GUI::CRect( 200 - 4 - 8, 6, 8, 8 ), accent ) );
        const Int32 pill = context->GetLineHeight( "Test" ) + 2 < 18 ? context->GetLineHeight( "Test" ) + 2 : 18;
        ASSERT_TRUE( HasFillOfColour( commands, GUI::CRect( 200 - 4 - 16, 20 + ( 20 - pill ) / 2, 16, pill ), accent ) );
        ASSERT_TRUE( GUCEF_NULL != FindTextCommand( commands, "6" ) );
        ASSERT_TRUE( tree->SetItemBadge( "Other", "" ) && tree->GetItemBadge( "Other" ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
