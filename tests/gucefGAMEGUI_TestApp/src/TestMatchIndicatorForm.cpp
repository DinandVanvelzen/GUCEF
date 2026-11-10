/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGAMEGUI_TestCommon.h"
#include "TestMatchFixture.h"
#include "TestMatchLayoutChecks.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef GAMEGUI::CMatchIndicatorForm TIndicator;

struct SScreenSize
{
    UInt32 width;
    UInt32 height;
};

const SScreenSize ScreenSizes[] = { { 800, 600 }, { 1242, 952 }, { 1920, 1080 }, { 3440, 1440 }, { 3840, 1080 } };
const UInt32 ScreenSizeCount = 5;

CORE::CString
SizeName( UInt32 width, UInt32 height )
{
    return CORE::UInt32ToString( width ) + "x" + CORE::UInt32ToString( height );
}

CORE::CString
TextOf( TIndicator& form, const CORE::CString& name )
{
    GUI::CWidget* widget = form.GetWidget( name );
    return GUCEF_NULL == widget ? CORE::CString( "<none>" ) : widget->GetProperty( "Text" );
}

CORE::CString
LongName( UInt32 index )
{
    CORE::CString title = "Long " + CORE::UInt32ToString( index ) + " ";
    while ( title.Length() < 200 )
        title += "Wxyz ";
    return title;
}

/** The matches that wait, joined with commas, by the title of the first character of their name */
CORE::CString
ListedText( TIndicator& form, SMatchBookFixture& f )
{
    GUCEF::vector< CORE::CString > ids;
    form.GetListedMatchIds( ids );
    CORE::CString text;
    for ( size_t i=0; i<ids.size(); ++i )
    {
        MULTIPLAY::CMatchEntry entry;
        f.book.GetEntry( ids[ i ], entry );
        text += ( 0 == i ? "" : "," ) + entry.GetDisplayTitle();
    }
    return text;
}

}

/*-------------------------------------------------------------------------*/

void
PerformMatchIndicatorFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGAMEGUI.MatchIndicator" );

    CFakeProviderScope fakeScope;

    GUCEF_TESTFW_TESTCASE( "MatchIndicator: The badge counts the games that wait, is hidden without any, and takes no room and no input then" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        SMatchBookFixture f;
        ASSERT_TRUE( f.Setup() );
        TIndicator form( context );
        form.SetClock( &f.clock );
        ASSERT_TRUE( form.GetHideWhenEmpty() && TIndicator::ANCHOR_TOP_RIGHT == form.GetAnchor() && 8 == form.GetMaxListed() );
        ASSERT_FALSE( form.IsOpen() );
        form.SetBook( &f.book );
        ASSERT_TRUE( &f.book == form.GetBook() );
        ASSERT_TRUE( f.Start() );
        ASSERT_TRUE( form.Open() && form.IsOpen() );
        context->FlushChanges();

        // nothing waits: nothing is there
        ASSERT_TRUE( 0 == form.GetYourTurnCount() && !form.IsBadgeShown() && !form.IsVisible() );
        ASSERT_FALSE( form.TogglePopup() || form.IsPopupOpen() );

        // a game that waits: the badge
        MULTIPLAY::CMatchInfo a = f.AddMatch( "A", SMatchBookFixture::KIND_YOURS );
        f.book.RefreshAll();
        ASSERT_TRUE( 1 == form.GetYourTurnCount() && form.IsBadgeShown() && form.IsVisible() );
        ASSERT_TRUE( "Your turn: 1" == TextOf( form, "Badge" ) );
        context->FlushChanges();
        ASSERT_FALSE( context->IsModalRoot( form.GetRootWidget() ) );
        const GUI::CRect badge = form.GetWidget( "Badge" )->GetRect();
        ASSERT_TRUE( badge.height >= 44 && badge.width >= 100 );

        // the count follows the book: more games, a game played, a turn that comes
        MULTIPLAY::CMatchInfo b = f.AddMatch( "B", SMatchBookFixture::KIND_YOURS );
        MULTIPLAY::CMatchInfo c = f.AddMatch( "C", SMatchBookFixture::KIND_THEIRS );
        f.book.RefreshAll();
        ASSERT_TRUE( 2 == form.GetYourTurnCount() && "Your turn: 2" == TextOf( form, "Badge" ) );
        f.Play( c, "user-me", 2 );
        f.book.RefreshAll();
        ASSERT_TRUE( 3 == form.GetYourTurnCount() && "Your turn: 3" == TextOf( form, "Badge" ) );
        f.Play( a, "user-bob", 2 );
        f.book.RefreshAll();
        ASSERT_TRUE( 2 == form.GetYourTurnCount() && "Your turn: 2" == TextOf( form, "Badge" ) );
        ASSERT_TRUE( f.book.SetArchived( b.matchId, true ) );
        ASSERT_TRUE( 1 == form.GetYourTurnCount() );

        // many games: the number stays short
        ASSERT_TRUE( f.book.SetArchived( b.matchId, false ) );
        for ( UInt32 i=0; i<120; ++i )
            f.AddMatch( "M" + CORE::UInt32ToString( i ), SMatchBookFixture::KIND_YOURS );
        f.book.RefreshAll();
        ASSERT_TRUE( 122 == form.GetYourTurnCount() && "Your turn: 99+" == TextOf( form, "Badge" ) );
        context->FlushChanges();
        ASSERT_TRUE( MfFindLayoutProblem( context, form.GetRootWidget() ).IsNULLOrEmpty() );

        // the last game goes: hidden again, and what was under the badge gets the input
        MULTIPLAY::TMatchEntryVector entries;
        f.book.GetEntries( entries );
        for ( size_t i=0; i<entries.size(); ++i )
            f.book.RemoveMatch( entries[ i ].GetMatchId() );
        ASSERT_TRUE( 0 == form.GetYourTurnCount() && !form.IsBadgeShown() && !form.IsVisible() );
        context->FlushChanges();
        GUI::CWidget* hit = context->HitTest( badge.left + badge.width / 2, badge.top + badge.height / 2 );
        ASSERT_TRUE( GUCEF_NULL == hit );

        // kept without games, it shows 0
        form.SetHideWhenEmpty( false );
        ASSERT_TRUE( form.IsBadgeShown() && form.IsVisible() && "Your turn: 0" == TextOf( form, "Badge" ) );
        context->FlushChanges();
        ASSERT_TRUE( form.TogglePopup() && form.IsPopupOpen() );
        ASSERT_TRUE( "My Games" == TextOf( form, "MyGames" ) );
        form.SetHideWhenEmpty( true );
        ASSERT_TRUE( !form.IsVisible() && !form.IsPopupOpen() );

        // closed: gone whatever the book says; open again: back
        f.AddMatch( "Z", SMatchBookFixture::KIND_YOURS );
        f.book.RefreshAll();
        ASSERT_TRUE( form.IsVisible() );
        form.Close();
        ASSERT_FALSE( form.IsOpen() || form.IsVisible() || form.IsBadgeShown() );
        f.AddMatch( "Y", SMatchBookFixture::KIND_YOURS );
        f.book.RefreshAll();
        ASSERT_FALSE( form.IsVisible() );
        ASSERT_TRUE( form.Open() && form.IsVisible() && "Your turn: 2" == TextOf( form, "Badge" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchIndicator: Where the application has a list of its own for the games that wait, a click on the badge asks for that list and opens no popup" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        SMatchBookFixture f;
        ASSERT_TRUE( f.Setup() );
        TIndicator form( context );
        form.SetClock( &f.clock );
        form.SetBook( &f.book );
        ASSERT_TRUE( f.Start() );
        ASSERT_TRUE( form.Open() );
        f.AddMatch( "A", SMatchBookFixture::KIND_YOURS );
        f.book.RefreshAll();
        context->FlushChanges();
        CEventCount requests( &form, TIndicator::BadgeRequestedEvent );

        // as it was: the badge opens the popup
        ASSERT_TRUE( form.GetBadgeOpensList() );
        ClickWidget( context, form.GetWidget( "Badge" ) );
        ASSERT_TRUE( form.IsPopupOpen() && 0 == requests.count );
        ClickWidget( context, form.GetWidget( "Badge" ) );
        ASSERT_FALSE( form.IsPopupOpen() );

        // with a list of its own: the click is a request, nothing opens, and the badge still counts
        form.SetBadgeOpensList( false );
        ClickWidget( context, form.GetWidget( "Badge" ) );
        ASSERT_TRUE( !form.IsPopupOpen() && 1 == requests.count );
        ClickWidget( context, form.GetWidget( "Badge" ) );
        ASSERT_TRUE( !form.IsPopupOpen() && 2 == requests.count && "Your turn: 1" == TextOf( form, "Badge" ) );
        // the popup is still there for whoever asks for it
        ASSERT_TRUE( form.TogglePopup() && form.IsPopupOpen() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchIndicator: The badge sits in the corner it is anchored to, and the list opens away from the edge" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( 1242, 952, true );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        SMatchBookFixture f;
        ASSERT_TRUE( f.Setup() );
        TIndicator form( context );
        form.SetClock( &f.clock );
        form.SetBook( &f.book );
        f.AddMatch( "A", SMatchBookFixture::KIND_YOURS );
        f.AddMatch( "B", SMatchBookFixture::KIND_YOURS );
        ASSERT_TRUE( f.Start() );
        ASSERT_TRUE( form.Open() );
        context->FlushChanges();

        // the default: top right, 12 pixels from the edges
        GUI::CRect rect = form.GetRootWidget()->GetRect();
        ASSERT_TRUE( 1242 - 12 == rect.Right() && 12 == rect.top );
        ASSERT_TRUE( rect.width == form.GetWidget( "Badge" )->GetRect().width && rect.height == form.GetWidget( "Badge" )->GetRect().height );

        struct SCase { TIndicator::TAnchor anchor; Int32 x; Int32 y; };
        const SCase cases[] = { { TIndicator::ANCHOR_TOP_LEFT, 20, 30 }, { TIndicator::ANCHOR_TOP_RIGHT, 5, 7 }, { TIndicator::ANCHOR_BOTTOM_LEFT, 40, 50 }, { TIndicator::ANCHOR_BOTTOM_RIGHT, 0, 0 } };
        for ( UInt32 i=0; i<4; ++i )
        {
            form.SetAnchor( cases[ i ].anchor, cases[ i ].x, cases[ i ].y );
            ASSERT_TRUE( cases[ i ].anchor == form.GetAnchor() );
            context->FlushChanges();
            rect = form.GetRootWidget()->GetRect();
            const bool right = TIndicator::ANCHOR_TOP_RIGHT == cases[ i ].anchor || TIndicator::ANCHOR_BOTTOM_RIGHT == cases[ i ].anchor;
            const bool bottom = TIndicator::ANCHOR_BOTTOM_LEFT == cases[ i ].anchor || TIndicator::ANCHOR_BOTTOM_RIGHT == cases[ i ].anchor;
            ASSERT_TRUE( right ? 1242 - cases[ i ].x == rect.Right() : cases[ i ].x == rect.left );
            ASSERT_TRUE( bottom ? 952 - cases[ i ].y == rect.Bottom() : cases[ i ].y == rect.top );
            const GUI::CRect closedBadge = form.GetWidget( "Badge" )->GetRect();

            // the list opens: the badge stays where it is, the list is below it, or above it at the bottom
            ASSERT_TRUE( form.TogglePopup() );
            context->FlushChanges();
            const GUI::CRect badge = form.GetWidget( "Badge" )->GetRect();
            const GUI::CRect popup = form.GetWidget( "Popup" )->GetRect();
            ASSERT_TRUE( badge.left == closedBadge.left && badge.top == closedBadge.top );
            if ( bottom )
                ASSERT_TRUE( popup.Bottom() <= badge.top );
            else
                ASSERT_TRUE( popup.top >= badge.Bottom() );
            const GUI::CRect openRect = form.GetRootWidget()->GetRect();
            ASSERT_TRUE( openRect.left >= 0 && openRect.top >= 0 && openRect.Right() <= 1242 && openRect.Bottom() <= 952 );
            ASSERT_TRUE( right ? 1242 - cases[ i ].x == openRect.Right() : cases[ i ].x == openRect.left );
            ASSERT_TRUE( MfInside( popup, openRect ) && MfInside( badge, openRect ) );
            form.ClosePopup();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchIndicator: The list has the games that wait, the longest waiting first, at most as many as set, with the time left" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        SMatchBookFixture f;
        ASSERT_TRUE( f.Setup() );
        TIndicator form( context );
        form.SetClock( &f.clock );
        form.SetBook( &f.book );
        CEventLog log;
        log.Watch( &form, TIndicator::OpenRequestedEvent, "open" );
        log.Watch( &form, TIndicator::MyGamesRequestedEvent, "mygames" );
        ASSERT_TRUE( f.Start() );
        ASSERT_TRUE( form.Open() );

        // the games arrive one after the other, in an order that is not the order of their names or deadlines; yourTurnSince is the arrival
        const Int64 now = f.clock.nowMs;
        MULTIPLAY::CMatchInfo d = f.AddMatch( "Delta", SMatchBookFixture::KIND_YOURS, now + 3900000 );
        f.book.RefreshAll();
        f.clock.AdvanceMs( 1000 );
        MULTIPLAY::CMatchInfo a = f.AddMatch( "Alpha", SMatchBookFixture::KIND_YOURS, now + 60000 );
        f.book.RefreshAll();
        f.clock.AdvanceMs( 1000 );
        MULTIPLAY::CMatchInfo c = f.AddMatch( "Charlie", SMatchBookFixture::KIND_YOURS );
        f.book.RefreshAll();
        f.clock.AdvanceMs( 1000 );
        MULTIPLAY::CMatchInfo b = f.AddMatch( "Bravo", SMatchBookFixture::KIND_THEIRS );
        f.book.RefreshAll();
        f.clock.AdvanceMs( 1000 );
        f.Play( b, "user-me", 2 );
        f.book.RefreshAll();
        MULTIPLAY::CMatchInfo e = f.AddMatch( "Echo", SMatchBookFixture::KIND_YOURS );
        MULTIPLAY::CMatchInfo notMine = f.AddMatch( "Theirs", SMatchBookFixture::KIND_THEIRS );
        MULTIPLAY::CMatchInfo invited = f.AddMatch( "Invited", SMatchBookFixture::KIND_INVITED );
        f.clock.AdvanceMs( 1000 );
        f.book.RefreshAll();
        ASSERT_TRUE( 5 == form.GetYourTurnCount() );
        ASSERT_TRUE( "Delta,Alpha,Charlie,Bravo,Echo" == ListedText( form, f ) );

        // the order is the one of GetNextYourTurn
        CORE::CString next;
        ASSERT_TRUE( f.book.GetNextYourTurn( CORE::CString::Empty, next ) && next == d.matchId );
        GUCEF::vector< CORE::CString > ids;
        form.GetListedMatchIds( ids );
        ASSERT_TRUE( 5 == ids.size() && ids[ 0 ] == d.matchId && ids[ 1 ] == a.matchId && ids[ 4 ] == e.matchId );

        // the list is closed until the badge is clicked
        context->FlushChanges();
        ASSERT_FALSE( form.IsPopupOpen() || form.GetWidget( "Popup" )->IsVisible() );
        ClickWidget( context, form.GetWidget( "Badge" ) );
        context->RenderFrame( 500 );
        ASSERT_TRUE( form.IsPopupOpen() && form.GetWidget( "Popup" )->IsVisible() );
        ASSERT_TRUE( "Delta" == TextOf( form, "Entry0" ) && "Alpha" == TextOf( form, "Entry1" ) && "Echo" == TextOf( form, "Entry4" ) );
        ASSERT_TRUE( !form.GetWidget( "Entry5" ) || !form.GetWidget( "Entry5" )->IsVisible() );
        ASSERT_TRUE( "1h 04m" == TextOf( form, "EntryTime0" ) && "0:55" == TextOf( form, "EntryTime1" ) && TextOf( form, "EntryTime2" ).IsNULLOrEmpty() );
        ASSERT_TRUE( "My Games" == TextOf( form, "MyGames" ) );

        // the time left follows the clock and the frames
        f.clock.AdvanceMs( 30000 );
        context->RenderFrame( 1000 );
        ASSERT_TRUE( "0:25" == TextOf( form, "EntryTime1" ) );

        // the badge toggles the list
        ClickWidget( context, form.GetWidget( "Badge" ) );
        context->RenderFrame( 500 );
        ASSERT_FALSE( form.IsPopupOpen() || form.GetWidget( "Popup" )->IsVisible() );
        ASSERT_TRUE( form.TogglePopup() && form.IsPopupOpen() );
        ASSERT_TRUE( form.TogglePopup() == false && !form.IsPopupOpen() );

        // a limit: the list is cut, the last entry says there are more
        form.SetMaxListed( 3 );
        ASSERT_TRUE( 3 == form.GetMaxListed() );
        ASSERT_TRUE( "Delta,Alpha,Charlie" == ListedText( form, f ) );
        ASSERT_TRUE( form.TogglePopup() );
        context->FlushChanges();
        ASSERT_TRUE( "Delta" == TextOf( form, "Entry0" ) && "Charlie" == TextOf( form, "Entry2" ) );
        ASSERT_TRUE( !form.GetWidget( "Entry3" ) || !form.GetWidget( "Entry3" )->IsVisible() );
        ASSERT_TRUE( "More games ..." == TextOf( form, "MyGames" ) );
        form.SetMaxListed( 0 );
        ASSERT_TRUE( 1 == form.GetMaxListed() );
        form.SetMaxListed( 500 );
        ASSERT_TRUE( 20 == form.GetMaxListed() );
        form.SetMaxListed( 5 );
        ASSERT_TRUE( "My Games" == TextOf( form, "MyGames" ) );

        // clicking a game opens it (the application does that) and closes the list
        context->FlushChanges();
        ClickWidget( context, form.GetWidget( "Entry1" ) );
        ASSERT_TRUE( ( "open:" + a.matchId ) == log.Last() && 1 == log.Count( "open" ) );
        ASSERT_FALSE( form.IsPopupOpen() );
        ASSERT_TRUE( f.book.GetOpenMatchId().IsNULLOrEmpty() );

        // My Games notifies and closes
        context->RenderFrame( 500 );
        ASSERT_TRUE( form.TogglePopup() );
        context->FlushChanges();
        ClickWidget( context, form.GetWidget( "MyGames" ) );
        ASSERT_TRUE( "mygames" == log.Last() && 1 == log.Count( "mygames" ) && !form.IsPopupOpen() );

        // while the list is open it follows the book: a game is played, one arrives
        ASSERT_TRUE( form.TogglePopup() );
        f.Play( d, "user-bob", 2 );
        f.book.RefreshAll();
        ASSERT_TRUE( form.IsPopupOpen() && "Alpha,Charlie,Bravo,Echo" == ListedText( form, f ) );
        ASSERT_TRUE( "Alpha" == TextOf( form, "Entry0" ) && "Echo" == TextOf( form, "Entry3" ) );
        f.clock.AdvanceMs( 1000 );
        MULTIPLAY::CMatchInfo g = f.AddMatch( "Golf", SMatchBookFixture::KIND_YOURS );
        f.book.RefreshAll();
        ASSERT_TRUE( "Alpha,Charlie,Bravo,Echo,Golf" == ListedText( form, f ) && "Golf" == TextOf( form, "Entry4" ) );

        // a game that is played and comes back waits again from now on: it goes to the end
        f.Play( a, "user-bob", 2 );
        f.book.RefreshAll();
        f.clock.AdvanceMs( 1000 );
        f.Play( a, "user-me", 3 );
        f.book.RefreshAll();
        ASSERT_TRUE( "Charlie,Bravo,Echo,Golf,Alpha" == ListedText( form, f ) );

        // the player's own name is the name in the list; archived games are not in it
        ASSERT_TRUE( f.book.SetLocalTitle( c.matchId, "My Charlie" ) );
        ASSERT_TRUE( "My Charlie,Bravo,Echo,Golf,Alpha" == ListedText( form, f ) && "My Charlie" == TextOf( form, "Entry0" ) );
        ASSERT_TRUE( f.book.SetArchived( e.matchId, true ) );
        ASSERT_TRUE( "My Charlie,Bravo,Golf,Alpha" == ListedText( form, f ) );
        ASSERT_TRUE( !form.GetWidget( "Entry4" ) || !form.GetWidget( "Entry4" )->IsVisible() );

        // an overdue game says so
        f.clock.AdvanceMs( 1000 );
        MULTIPLAY::CMatchInfo late = f.AddMatch( "Late", SMatchBookFixture::KIND_YOURS, f.clock.nowMs - 5 );
        f.book.RefreshAll();
        ASSERT_TRUE( "Overdue" == TextOf( form, "EntryTime4" ) );
        (void) notMine;
        (void) invited;
        (void) late;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchIndicator: Layout from 800 x 600 to 3840 x 1080 in every corner with a full list of long names: inside, apart, touch sized, text inside its box" )
    try
    {
        for ( UInt32 s=0; s<ScreenSizeCount; ++s )
        {
            const UInt32 screenWidth = ScreenSizes[ s ].width;
            const UInt32 screenHeight = ScreenSizes[ s ].height;
            const CORE::CString where = " at " + SizeName( screenWidth, screenHeight );
            GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( screenWidth, screenHeight, true );
            GUI::CGUIContext* context = ContextOf( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                return;
            SMatchBookFixture f;
            ASSERT_TRUE( f.Setup() );
            TIndicator form( context );
            form.SetClock( &f.clock );
            form.SetBook( &f.book );
            for ( UInt32 i=0; i<12; ++i )
            {
                f.AddMatch( 0 == i % 2 ? LongName( i ) : ( "Game " + CORE::UInt32ToString( i ) ), SMatchBookFixture::KIND_YOURS, 0 == i % 3 ? f.clock.nowMs + (Int64) i * 5000000 : 0 );
                f.clock.AdvanceMs( 10 );
            }
            ASSERT_TRUE( f.Start() );
            ASSERT_TRUE( form.Open() );
            for ( UInt32 corner=0; corner<4; ++corner )
            {
                form.SetAnchor( static_cast< TIndicator::TAnchor >( corner ), 12, 12 );
                form.ClosePopup();
                context->RenderFrame( 16 );
                context->RenderFrame( 16 );
                CORE::CString problem = MfFindLayoutProblem( context, form.GetRootWidget() );
                if ( !problem.IsNULLOrEmpty() )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MatchIndicator: badge" << where.C_String() << ": " << problem.C_String() );
                ASSERT_TRUE( problem.IsNULLOrEmpty() );

                ASSERT_TRUE( form.TogglePopup() );
                context->RenderFrame( 16 );
                context->RenderFrame( 16 );
                problem = MfFindLayoutProblem( context, form.GetRootWidget() );
                if ( !problem.IsNULLOrEmpty() )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MatchIndicator: list" << where.C_String() << " corner " << CORE::UInt32ToString( corner ).C_String() << ": " << problem.C_String() );
                ASSERT_TRUE( problem.IsNULLOrEmpty() );
                const GUI::CRect rect = form.GetRootWidget()->GetRect();
                ASSERT_TRUE( rect.left >= 0 && rect.top >= 0 && rect.Right() <= (Int32) screenWidth && rect.Bottom() <= (Int32) screenHeight );
                for ( UInt32 e=0; e<8; ++e )
                {
                    GUI::CWidget* entry = form.GetWidget( "Entry" + CORE::UInt32ToString( e ) );
                    ASSERT_TRUE( GUCEF_NULL != entry && entry->IsVisible() && entry->GetRect().height >= 44 );
                }
                WriteShot( CaptureFrame( context ), "match-indicator-" + SizeName( screenWidth, screenHeight ) + "-" + CORE::UInt32ToString( corner ) );
                form.ClosePopup();
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchIndicator: A book that goes away leaves an indicator of 0 and no dangling subscription" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        SMatchBookFixture f;
        ASSERT_TRUE( f.Setup() );
        TIndicator form( context );
        form.SetClock( &f.clock );
        f.AddMatch( "A", SMatchBookFixture::KIND_YOURS );
        ASSERT_TRUE( f.Start() );
        ASSERT_TRUE( form.Open() );
        const UInt32 before = GetEventSubscriptionCount( form );
        form.SetBook( &f.book );
        ASSERT_TRUE( 1 == form.GetYourTurnCount() && before + 2 == GetEventSubscriptionCount( form ) );
        form.SetBook( GUCEF_NULL );
        ASSERT_TRUE( 0 == form.GetYourTurnCount() && before == GetEventSubscriptionCount( form ) && !form.IsVisible() );
        form.SetBook( &f.book );

        MULTIPLAY::CMatchBook* book = GUCEF_NEW MULTIPLAY::CMatchBook();
        book->SetLocalUserId( "user-me" );
        form.SetBook( book );
        GUCEF_DELETE book;
        ASSERT_TRUE( GUCEF_NULL == form.GetBook() && 0 == form.GetYourTurnCount() && !form.IsVisible() );
        context->RenderFrame( 1000 );
        ASSERT_TRUE( 0 == form.GetYourTurnCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
