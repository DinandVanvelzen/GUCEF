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

typedef GAMEGUI::CMatchListForm::SRowInfo TRowInfo;

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
LabelText( GAMEGUI::CMatchListForm& form, const CORE::CString& matchId, const char* part )
{
    GUI::CWidget* widget = form.GetRowWidget( matchId, part );
    return GUCEF_NULL == widget ? CORE::CString( "<none>" ) : widget->GetProperty( "Text" );
}

bool
IsShown( GAMEGUI::CMatchListForm& form, const CORE::CString& matchId, const char* part )
{
    GUI::CWidget* widget = form.GetRowWidget( matchId, part );
    return GUCEF_NULL != widget && widget->IsVisible();
}

CORE::CString::StringVector
MenuOf( GAMEGUI::CMatchListForm& form, const CORE::CString& matchId )
{
    GUCEF::vector< CORE::CString > items;
    form.GetRowMenuItems( matchId, items );
    CORE::CString::StringVector result;
    for ( size_t i=0; i<items.size(); ++i )
        result.push_back( items[ i ] );
    return result;
}

/** The items as one text, "Open,Mute,Rename" */
CORE::CString
MenuText( GAMEGUI::CMatchListForm& form, const CORE::CString& matchId )
{
    CORE::CString::StringVector items = MenuOf( form, matchId );
    CORE::CString text;
    for ( size_t i=0; i<items.size(); ++i )
        text += ( 0 == i ? "" : "," ) + items[ i ];
    return text;
}

/** The user chooses an item of the menu that is open */
bool
ChooseMenuItem( GUI::CGUIContext* context, const CORE::CString& item )
{
    context->FlushChanges();
    GUI::CPopupMenu* menu = GAMEGUI::WidgetAs< GUI::CPopupMenu >( context->GetPopup(), "PopupMenu" );
    if ( GUCEF_NULL == menu )
        return false;
    GUI::CPopupMenu::TStringVector items = menu->GetItems();
    for ( size_t i=0; i<items.size(); ++i )
    {
        if ( items[ i ] == item )
        {
            const GUI::CRect& rect = menu->GetRect();
            const Int32 rowHeight = menu->GetRowHeight();
            ClickAt( context, rect.left + 8, rect.top + (Int32) i * rowHeight + rowHeight / 2 );
            return true;
        }
    }
    return false;
}

void
ClickRowOf( GUI::CGUIContext* context, GAMEGUI::CMatchListForm& form, const CORE::CString& matchId, GUI::TMouseButton button = GUI::MOUSEBUTTON_LEFT )
{
    context->FlushChanges();
    GUI::CWidget* row = form.GetRowWidget( matchId, "Row" );
    if ( GUCEF_NULL == row )
        return;
    const GUI::CRect seen = MfVisibleRect( row );
    ClickAt( context, seen.left + seen.width / 2, seen.top + seen.height / 2, button );
    context->RenderFrame( 500 );
}

CORE::CString
LongTitle( UInt32 index )
{
    CORE::CString title = "Long " + CORE::UInt32ToString( index ) + " ";
    while ( title.Length() < 200 )
        title += "Wxyz ";
    return title;
}

}

/*-------------------------------------------------------------------------*/

void
PerformMatchListFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGAMEGUI.MatchList" );

    CFakeProviderScope fakeScope;

    GUCEF_TESTFW_TESTCASE( "MatchList: The time left reads Overdue, m:ss, 1h 05m and 2d 3h, and text is cut with dots to the width it has" )
    try
    {
        ASSERT_TRUE( "Overdue" == GAMEGUI::FormatTimeLeft( 0 ) && "Overdue" == GAMEGUI::FormatTimeLeft( -5000 ) );
        ASSERT_TRUE( "0:01" == GAMEGUI::FormatTimeLeft( 1 ) && "0:01" == GAMEGUI::FormatTimeLeft( 1000 ) && "0:02" == GAMEGUI::FormatTimeLeft( 1001 ) );
        ASSERT_TRUE( "1:30" == GAMEGUI::FormatTimeLeft( 90000 ) && "59:59" == GAMEGUI::FormatTimeLeft( 3599000 ) );
        ASSERT_TRUE( "1h 00m" == GAMEGUI::FormatTimeLeft( 3599001 ) && "1h 00m" == GAMEGUI::FormatTimeLeft( 3600000 ) );
        ASSERT_TRUE( "1h 05m" == GAMEGUI::FormatTimeLeft( 3900000 ) && "23h 59m" == GAMEGUI::FormatTimeLeft( 86399000 ) );
        ASSERT_TRUE( "1d 0h" == GAMEGUI::FormatTimeLeft( 86400000 ) && "2d 3h" == GAMEGUI::FormatTimeLeft( ( 2 * 86400 + 3 * 3600 + 120 ) * 1000LL ) );

        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        // the test font is 8 pixels wide for every character
        ASSERT_TRUE( "Short" == GAMEGUI::EllipsizeText( *context, "Test", "Short", 40 ) );
        ASSERT_TRUE( "Shor..." == GAMEGUI::EllipsizeText( *context, "Test", "Short name", 56 + 8 ) || "Short..." == GAMEGUI::EllipsizeText( *context, "Test", "Short name", 64 ) );
        CORE::CString cut = GAMEGUI::EllipsizeText( *context, "Test", LongTitle( 1 ), 200 );
        ASSERT_TRUE( cut.Length() * 8 <= 200 && cut.Length() > 10 && cut.HasSubstr( "...", true ) >= 0 );
        ASSERT_TRUE( GAMEGUI::EllipsizeText( *context, "Test", "Anything", 16 ).IsNULLOrEmpty() );
        ASSERT_TRUE( "a b" == GAMEGUI::EllipsizeText( *context, "Test", "a\nb", 100 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchList: The rows are grouped Your Turn, Their Turn, Invitations and Finished with counts, empty groups and archived matches are left out" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        SMatchBookFixture f;
        ASSERT_TRUE( f.Setup() );

        GAMEGUI::CMatchListForm form( context );
        form.SetClock( &f.clock );
        form.SetBook( &f.book );
        ASSERT_TRUE( form.Open() && form.IsOpen() && form.IsVisible() );
        ASSERT_FALSE( context->IsModalRoot( form.GetRootWidget() ) );
        ASSERT_TRUE( 0 == form.GetRowCount() && &f.book == form.GetBook() );

        MULTIPLAY::CMatchInfo y1 = f.AddMatch( "Y1", SMatchBookFixture::KIND_YOURS, 0, 100 );
        MULTIPLAY::CMatchInfo y2 = f.AddMatch( "Y2", SMatchBookFixture::KIND_YOURS, 0, 200 );
        MULTIPLAY::CMatchInfo t1 = f.AddMatch( "T1", SMatchBookFixture::KIND_THEIRS );
        MULTIPLAY::CMatchInfo i1 = f.AddMatch( "I1", SMatchBookFixture::KIND_INVITED );
        MULTIPLAY::CMatchInfo i2 = f.AddMatch( "I2", SMatchBookFixture::KIND_FORMING );
        MULTIPLAY::CMatchInfo i3 = f.AddMatch( "I3", SMatchBookFixture::KIND_INVITED );
        ASSERT_TRUE( f.Start() );

        // no finished match: no header for it
        ASSERT_TRUE( 9 == form.GetRowCount() );
        TRowInfo row;
        ASSERT_TRUE( form.GetRow( 0, row ) && row.isHeader && MULTIPLAY::MATCHGROUP_YOUR_TURN == row.group && "Your Turn (2)" == row.headerText && row.matchId.IsNULLOrEmpty() );
        ASSERT_TRUE( form.GetRow( 3, row ) && row.isHeader && MULTIPLAY::MATCHGROUP_THEIR_TURN == row.group && "Their Turn (1)" == row.headerText );
        ASSERT_TRUE( form.GetRow( 5, row ) && row.isHeader && MULTIPLAY::MATCHGROUP_FORMING_OR_INVITED == row.group && "Invitations (3)" == row.headerText );
        ASSERT_TRUE( form.GetRow( 1, row ) && !row.isHeader && row.matchId == y2.matchId );          // the book orders by deadline, then the latest activity first
        ASSERT_TRUE( form.GetRow( 2, row ) && row.matchId == y1.matchId );
        ASSERT_TRUE( form.GetRow( 4, row ) && row.matchId == t1.matchId && MULTIPLAY::MATCHGROUP_THEIR_TURN == row.group );
        ASSERT_FALSE( form.GetRow( 10, row ) );
        ASSERT_TRUE( 1 == form.FindRow( y2.matchId ) && 4 == form.FindRow( t1.matchId ) && -1 == form.FindRow( "unknown" ) && -1 == form.FindRow( CORE::CString::Empty ) );

        // a finished match gets the last group
        MULTIPLAY::CMatchInfo fin = f.AddMatch( "Fin", SMatchBookFixture::KIND_FINISHED );
        f.book.RefreshAll();
        ASSERT_TRUE( 11 == form.GetRowCount() );
        ASSERT_TRUE( form.GetRow( 9, row ) && row.isHeader && MULTIPLAY::MATCHGROUP_FINISHED == row.group && "Finished (1)" == row.headerText );
        ASSERT_TRUE( form.GetRow( 10, row ) && row.matchId == fin.matchId );

        // the book changes and the rows follow at once: Y1 is played and now waits for Bob
        f.Play( y1, "user-bob", 2 );
        f.book.RefreshAll();
        ASSERT_TRUE( form.GetRow( 0, row ) && "Your Turn (1)" == row.headerText );
        ASSERT_TRUE( form.GetRow( 2, row ) && "Their Turn (2)" == row.headerText );

        // archived matches are not listed, until the form is asked to
        ASSERT_TRUE( f.book.SetArchived( t1.matchId, true ) );
        ASSERT_TRUE( -1 == form.FindRow( t1.matchId ) );
        ASSERT_TRUE( form.GetRow( 2, row ) && "Their Turn (1)" == row.headerText );
        form.SetShowArchived( true );
        ASSERT_TRUE( form.GetShowArchived() && form.FindRow( t1.matchId ) >= 0 );
        ASSERT_TRUE( form.GetRow( 2, row ) && "Their Turn (2)" == row.headerText );
        form.SetShowArchived( false );
        ASSERT_TRUE( -1 == form.FindRow( t1.matchId ) );

        // the last match of a group takes the header along
        ASSERT_TRUE( f.book.RemoveMatch( y2.matchId ) );
        ASSERT_TRUE( form.GetRow( 0, row ) && row.isHeader && MULTIPLAY::MATCHGROUP_THEIR_TURN == row.group );

        // closing hides the window, the rows are kept up to date meanwhile
        form.Close();
        ASSERT_FALSE( form.IsOpen() || form.IsVisible() );
        f.book.RemoveMatch( fin.matchId );
        ASSERT_TRUE( -1 == form.FindRow( fin.matchId ) );
        ASSERT_TRUE( form.Open() && form.IsVisible() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchList: A row has the name of the game, the turn and who has it, and the time left against the clock once a second" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        SMatchBookFixture f;
        ASSERT_TRUE( f.Setup() );
        GAMEGUI::CMatchListForm form( context );
        form.SetClock( &f.clock );
        form.SetBook( &f.book );
        const Int64 now = f.clock.nowMs;
        MULTIPLAY::CMatchInfo named = f.AddMatch( "Friday game", SMatchBookFixture::KIND_THEIRS, now + 90000, 0, 41 );
        MULTIPLAY::CMatchInfo unnamed = f.AddMatch( CORE::CString::Empty, SMatchBookFixture::KIND_YOURS, 0, 0, 7 );
        MULTIPLAY::CMatchInfo overdue = f.AddMatch( "Late", SMatchBookFixture::KIND_THEIRS, now - 1000 );
        MULTIPLAY::CMatchInfo hours = f.AddMatch( "Hours", SMatchBookFixture::KIND_THEIRS, now + 3900000 );
        MULTIPLAY::CMatchInfo days = f.AddMatch( "Days", SMatchBookFixture::KIND_THEIRS, now + ( 2 * 86400 + 3 * 3600 + 600 ) * 1000LL );
        MULTIPLAY::CMatchInfo strange = f.AddMatch( "Strange", SMatchBookFixture::KIND_THEIRS );
        f.Play( strange, "user-zed", 2 );                       // on turn: nobody the match knows
        MULTIPLAY::CMatchInfo finished = f.AddMatch( "Finished", SMatchBookFixture::KIND_FINISHED, now + 5000 );
        MULTIPLAY::CMatchInfo rude = f.AddMatch( "Line one\nline two\t!", SMatchBookFixture::KIND_THEIRS );
        ASSERT_TRUE( f.Start() );
        ASSERT_TRUE( form.Open() );
        context->FlushChanges();

        // a name with control characters is one line
        TRowInfo rudeRow;
        ASSERT_TRUE( form.GetRow( form.FindRow( rude.matchId ), rudeRow ) && "Line one line two !" == rudeRow.title );
        ASSERT_TRUE( "Line one line two !" == LabelText( form, rude.matchId, "Title" ) );

        TRowInfo row;
        ASSERT_TRUE( form.GetRow( form.FindRow( named.matchId ), row ) );
        ASSERT_TRUE( "Friday game" == row.title && "Turn 41 - Bob" == row.subtitle && "1:30" == row.timeLeft );
        ASSERT_TRUE( "Friday game" == LabelText( form, named.matchId, "Title" ) && "Turn 41 - Bob" == LabelText( form, named.matchId, "Subtitle" ) );
        ASSERT_TRUE( "1:30" == LabelText( form, named.matchId, "TimeLeft" ) );

        // no name: Game and the short id; no deadline: no time
        ASSERT_TRUE( form.GetRow( form.FindRow( unnamed.matchId ), row ) );
        ASSERT_TRUE( ( "Game " + MULTIPLAY::CMatchId::Short( unnamed.matchId ) ) == row.title && "Turn 7 - Me" == row.subtitle && row.timeLeft.IsNULLOrEmpty() );
        ASSERT_TRUE( LabelText( form, unnamed.matchId, "TimeLeft" ).IsNULLOrEmpty() );

        // the player's own name wins over the shared one
        ASSERT_TRUE( f.book.SetLocalTitle( named.matchId, "Mine" ) );
        ASSERT_TRUE( form.GetRow( form.FindRow( named.matchId ), row ) && "Mine" == row.title && "Mine" == LabelText( form, named.matchId, "Title" ) );

        // the player on turn is not one of the match: the summary; and without one only the turn
        ASSERT_TRUE( form.GetRow( form.FindRow( strange.matchId ), row ) && "Summary" == row.subtitle );
        MULTIPLAY::CMatchPreview strangePreview;
        strangePreview.matchId = strange.matchId;
        strangePreview.revision = 3;
        strangePreview.turn = 9;
        strangePreview.status = MULTIPLAY::MATCHSTATUS_ACTIVE;
        strangePreview.currentUserIds.insert( "user-zed" );
        f.provider->InjectTurn( strangePreview, TESTFW::MakeTestPackage( 3, GUCEF_NULL, "x" ) );
        f.book.RefreshAll();
        ASSERT_TRUE( form.GetRow( form.FindRow( strange.matchId ), row ) && "Turn 9" == row.subtitle );

        // overdue, hours and days; a finished match shows no time
        ASSERT_TRUE( form.GetRow( form.FindRow( overdue.matchId ), row ) && "Overdue" == row.timeLeft );
        ASSERT_TRUE( form.GetRow( form.FindRow( hours.matchId ), row ) && "1h 05m" == row.timeLeft );
        ASSERT_TRUE( form.GetRow( form.FindRow( days.matchId ), row ) && "2d 3h" == row.timeLeft );
        ASSERT_TRUE( form.GetRow( form.FindRow( finished.matchId ), row ) && row.timeLeft.IsNULLOrEmpty() );

        // time passes with the clock and the frames of the context: the text is shown again once a second
        f.clock.AdvanceMs( 31000 );
        ASSERT_TRUE( "1:30" == LabelText( form, named.matchId, "TimeLeft" ) );          // nobody told the form
        context->RenderFrame( 400 );
        ASSERT_TRUE( "1:30" == LabelText( form, named.matchId, "TimeLeft" ) );          // less than a second since
        context->RenderFrame( 600 );
        ASSERT_TRUE( "0:59" == LabelText( form, named.matchId, "TimeLeft" ) );
        ASSERT_TRUE( form.GetRow( form.FindRow( named.matchId ), row ) && "0:59" == row.timeLeft );

        // only the labels whose text changed are touched: a text put there by hand stays where the time did not change
        form.GetRowWidget( overdue.matchId, "TimeLeft" )->SetProperty( "Text", "touched" );
        form.GetRowWidget( named.matchId, "Subtitle" )->SetProperty( "Text", "touched" );
        f.clock.AdvanceMs( 1000 );
        form.Update( 1000 );
        ASSERT_TRUE( "0:58" == LabelText( form, named.matchId, "TimeLeft" ) );
        ASSERT_TRUE( "touched" == LabelText( form, overdue.matchId, "TimeLeft" ) && "touched" == LabelText( form, named.matchId, "Subtitle" ) );
        form.Update( 999 );
        f.clock.AdvanceMs( 1000 );
        ASSERT_TRUE( "0:58" == LabelText( form, named.matchId, "TimeLeft" ) );
        form.Update( 1 );
        ASSERT_TRUE( "0:57" == LabelText( form, named.matchId, "TimeLeft" ) );

        // up: Overdue
        f.clock.AdvanceMs( 60000 );
        form.Update( 1000 );
        ASSERT_TRUE( "Overdue" == LabelText( form, named.matchId, "TimeLeft" ) );
        ASSERT_TRUE( form.GetRow( form.FindRow( named.matchId ), row ) && "Overdue" == row.timeLeft );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchList: The marks show unseen, muted, pinned and problem, and the portrait slot shows an image the context knows" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        SMatchBookFixture f;
        ASSERT_TRUE( f.Setup() );
        GAMEGUI::CMatchListForm form( context );
        form.SetClock( &f.clock );
        form.SetBook( &f.book );
        MULTIPLAY::CMatchInfo a = f.AddMatch( "A", SMatchBookFixture::KIND_YOURS );
        MULTIPLAY::CMatchInfo b = f.AddMatch( "B", SMatchBookFixture::KIND_THEIRS );
        ASSERT_TRUE( f.Start() );
        ASSERT_TRUE( form.Open() );
        context->FlushChanges();

        TRowInfo row;
        ASSERT_TRUE( form.GetRow( form.FindRow( a.matchId ), row ) );
        ASSERT_TRUE( row.unseen && !row.muted && !row.pinned && !row.problem );
        ASSERT_TRUE( IsShown( form, a.matchId, "Dot" ) && !IsShown( form, a.matchId, "Muted" ) && !IsShown( form, a.matchId, "Pinned" ) && !IsShown( form, a.matchId, "Problem" ) );

        // seen: the dot goes
        ASSERT_TRUE( f.book.MarkSeen( a.matchId ) );
        ASSERT_TRUE( form.GetRow( form.FindRow( a.matchId ), row ) && !row.unseen && !IsShown( form, a.matchId, "Dot" ) );
        ASSERT_TRUE( IsShown( form, b.matchId, "Dot" ) );

        ASSERT_TRUE( f.book.SetMuted( a.matchId, true ) && f.book.SetPinned( a.matchId, true ) );
        ASSERT_TRUE( form.GetRow( form.FindRow( a.matchId ), row ) && row.muted && row.pinned && !row.problem );
        ASSERT_TRUE( IsShown( form, a.matchId, "Muted" ) && IsShown( form, a.matchId, "Pinned" ) && !IsShown( form, b.matchId, "Muted" ) );
        ASSERT_TRUE( f.book.SetMuted( a.matchId, false ) );
        ASSERT_TRUE( form.GetRow( form.FindRow( a.matchId ), row ) && !row.muted && !IsShown( form, a.matchId, "Muted" ) && row.pinned );

        // a problem pushed by the application: the mark, and the text as tooltip and as the detail of the selected row
        form.SetMatchProblem( b.matchId, "Package refused: bad checksum" );
        ASSERT_TRUE( form.GetRow( form.FindRow( b.matchId ), row ) && row.problem && IsShown( form, b.matchId, "Problem" ) );
        ASSERT_TRUE( "Package refused: bad checksum" == form.GetRowWidget( b.matchId, "Row" )->GetProperty( "Tooltip" ) );
        ASSERT_TRUE( form.SelectRow( b.matchId ) );
        ASSERT_TRUE( "Package refused: bad checksum" == form.GetWidget( "Detail" )->GetProperty( "Text" ) );
        form.SetMatchProblem( b.matchId, CORE::CString::Empty );
        ASSERT_TRUE( form.GetRow( form.FindRow( b.matchId ), row ) && !row.problem && !IsShown( form, b.matchId, "Problem" ) );
        ASSERT_TRUE( "B" == form.GetWidget( "Detail" )->GetProperty( "Text" ) );
        form.SetMatchProblem( "not a match of the book", "ignored" );

        // the portrait: an image the context knows, else the slot is empty
        ASSERT_FALSE( IsShown( form, a.matchId, "Portrait" ) );
        form.SetMatchPortrait( a.matchId, "no-such-image" );
        ASSERT_FALSE( IsShown( form, a.matchId, "Portrait" ) );
        ASSERT_TRUE( context->RegisterImage( "portrait-a", MakeSolidImage( 8, 8, 200, 20, 20, 255 ) ) );
        form.SetMatchPortrait( a.matchId, "portrait-a" );
        ASSERT_TRUE( IsShown( form, a.matchId, "Portrait" ) && "portrait-a" == form.GetRowWidget( a.matchId, "Portrait" )->GetProperty( "Image" ) );
        ASSERT_FALSE( IsShown( form, b.matchId, "Portrait" ) );
        form.SetMatchPortrait( a.matchId, CORE::CString::Empty );
        ASSERT_FALSE( IsShown( form, a.matchId, "Portrait" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchList: The row menu has what applies to the match and the provider, nothing that does not" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        SMatchBookFixture f;
        ASSERT_TRUE( f.Setup() );
        GAMEGUI::CMatchListForm form( context );
        form.SetClock( &f.clock );
        form.SetBook( &f.book );
        MULTIPLAY::CMatchInfo yours = f.AddMatch( "Yours", SMatchBookFixture::KIND_YOURS );
        MULTIPLAY::CMatchInfo theirs = f.AddMatch( "Theirs", SMatchBookFixture::KIND_THEIRS );
        MULTIPLAY::CMatchInfo invited = f.AddMatch( "Invited", SMatchBookFixture::KIND_INVITED );
        MULTIPLAY::CMatchInfo finished = f.AddMatch( "Finished", SMatchBookFixture::KIND_FINISHED );
        ASSERT_TRUE( f.Start() );
        ASSERT_TRUE( form.Open() );

        // everything the fake provider has: nudge, resign and rematch
        ASSERT_TRUE( "Open,Mute,Rename,Resign,Archive" == MenuText( form, yours.matchId ) );
        ASSERT_TRUE( "Open,Mute,Nudge,Rename,Resign,Archive" == MenuText( form, theirs.matchId ) );
        ASSERT_TRUE( "Open,Mute,Rename,Archive" == MenuText( form, invited.matchId ) );
        ASSERT_TRUE( "Open,Mute,Rename,Rematch,Remove" == MenuText( form, finished.matchId ) );
        GUCEF::vector< CORE::CString > none;
        ASSERT_FALSE( form.GetRowMenuItems( "unknown", none ) );

        // a muted match offers to unmute, an archived one to unarchive
        ASSERT_TRUE( f.book.SetMuted( yours.matchId, true ) );
        ASSERT_TRUE( "Open,Unmute,Rename,Resign,Archive" == MenuText( form, yours.matchId ) );
        form.SetShowArchived( true );
        ASSERT_TRUE( f.book.SetArchived( yours.matchId, true ) );
        ASSERT_TRUE( "Open,Unmute,Rename,Resign,Unarchive" == MenuText( form, yours.matchId ) );
        ASSERT_TRUE( f.book.SetArchived( yours.matchId, false ) && f.book.SetMuted( yours.matchId, false ) );

        // the provider can do less
        f.provider->SetCapabilities( MULTIPLAY::MATCHCAPABILITY_PREVIEW_POLL | MULTIPLAY::MATCHCAPABILITY_NUDGE );
        ASSERT_TRUE( "Open,Mute,Rename,Archive" == MenuText( form, yours.matchId ) );
        ASSERT_TRUE( "Open,Mute,Nudge,Rename,Archive" == MenuText( form, theirs.matchId ) );
        ASSERT_TRUE( "Open,Mute,Rename,Remove" == MenuText( form, finished.matchId ) );
        f.provider->SetCapabilities( MULTIPLAY::MATCHCAPABILITY_RESIGN );
        ASSERT_TRUE( "Open,Mute,Rename,Resign,Archive" == MenuText( form, yours.matchId ) );
        ASSERT_TRUE( "Open,Mute,Rename,Resign,Archive" == MenuText( form, theirs.matchId ) );
        f.provider->SetCapabilities( MULTIPLAY::MATCHCAPABILITY_REMATCH );
        ASSERT_TRUE( "Open,Mute,Rename,Archive" == MenuText( form, theirs.matchId ) );
        ASSERT_TRUE( "Open,Mute,Rename,Rematch,Remove" == MenuText( form, finished.matchId ) );
        f.provider->SetCapabilities( 0 );
        ASSERT_TRUE( "Open,Mute,Rename,Archive" == MenuText( form, yours.matchId ) );
        ASSERT_TRUE( "Open,Mute,Rename,Remove" == MenuText( form, finished.matchId ) );

        // nobody else is on turn: nobody to nudge
        f.provider->SetCapabilities( MULTIPLAY::MATCHCAPABILITY_PREVIEW_POLL | MULTIPLAY::MATCHCAPABILITY_NUDGE );
        f.Play( theirs, GUCEF_NULL, 2 );
        f.book.RefreshAll();
        ASSERT_TRUE( "Open,Mute,Rename,Archive" == MenuText( form, theirs.matchId ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchList: The application can add an action of its own to the menu of the games of one provider, after Open, and is told when it is chosen" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        SMatchBookFixture f;
        ASSERT_TRUE( f.Setup() );
        GAMEGUI::CMatchListForm form( context );
        form.SetClock( &f.clock );
        form.SetBook( &f.book );
        CEventLog log;
        log.Watch( &form, GAMEGUI::CMatchListForm::MatchActionEvent, "action" );
        MULTIPLAY::CMatchInfo yours = f.AddMatch( "Yours", SMatchBookFixture::KIND_YOURS );
        MULTIPLAY::CMatchInfo finished = f.AddMatch( "Finished", SMatchBookFixture::KIND_FINISHED );
        ASSERT_TRUE( f.Start() );
        ASSERT_TRUE( form.Open() );

        // none until the application asks for it
        ASSERT_TRUE( "Open,Mute,Rename,Resign,Archive" == MenuText( form, yours.matchId ) );
        form.SetDetailsAction( "someone else", "Mail details" );
        ASSERT_TRUE( "Open,Mute,Rename,Resign,Archive" == MenuText( form, yours.matchId ) );
        ASSERT_FALSE( form.PerformRowAction( yours.matchId, "Mail details" ) );

        // the games of the provider get it
        form.SetDetailsAction( "fake", "Mail details" );
        ASSERT_TRUE( "Open,Mail details,Mute,Rename,Resign,Archive" == MenuText( form, yours.matchId ) );
        ASSERT_TRUE( "Open,Mail details,Mute,Rename,Rematch,Remove" == MenuText( form, finished.matchId ) );
        const UInt32 told = log.Count( "action" );
        ASSERT_TRUE( form.PerformRowAction( yours.matchId, "Mail details" ) );
        ASSERT_TRUE( told + 1 == log.Count( "action" ) && ( "action:" + yours.matchId + "|" + CORE::Int32ToString( GAMEGUI::MATCHACTION_DETAILS ) ) == log.Last() );

        // without a text it is gone again
        form.SetDetailsAction( "fake", CORE::CString::Empty );
        ASSERT_TRUE( "Open,Mute,Rename,Resign,Archive" == MenuText( form, yours.matchId ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchList: The menu actions reach the book and the provider, and the application is told afterwards" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        SMatchBookFixture f;
        ASSERT_TRUE( f.Setup() );
        GAMEGUI::CMatchListForm form( context );
        form.SetClock( &f.clock );
        form.SetBook( &f.book );
        CEventLog log;
        log.Watch( &form, GAMEGUI::CMatchListForm::MatchActionEvent, "action" );
        log.Watch( &form, GAMEGUI::CMatchListForm::OpenRequestedEvent, "open" );
        MULTIPLAY::CMatchInfo yours = f.AddMatch( "Yours", SMatchBookFixture::KIND_YOURS );
        MULTIPLAY::CMatchInfo theirs = f.AddMatch( "Theirs", SMatchBookFixture::KIND_THEIRS );
        MULTIPLAY::CMatchInfo finished = f.AddMatch( "Finished", SMatchBookFixture::KIND_FINISHED );
        ASSERT_TRUE( f.Start() );
        ASSERT_TRUE( form.Open() );
        context->FlushChanges();

        // mute and unmute, through the book
        ASSERT_TRUE( form.PerformRowAction( yours.matchId, "Mute" ) );
        MULTIPLAY::CMatchEntry entry;
        ASSERT_TRUE( f.book.GetEntry( yours.matchId, entry ) && entry.muted );
        ASSERT_TRUE( ( "action:" + yours.matchId + "|" + CORE::Int32ToString( GAMEGUI::MATCHACTION_MUTE ) ) == log.Last() );
        ASSERT_TRUE( form.PerformRowAction( yours.matchId, "Unmute" ) );
        ASSERT_TRUE( f.book.GetEntry( yours.matchId, entry ) && !entry.muted );
        ASSERT_TRUE( ( "action:" + yours.matchId + "|" + CORE::Int32ToString( GAMEGUI::MATCHACTION_UNMUTE ) ) == log.Last() );

        // an item that is not in the menu of the match does nothing and tells nothing
        const UInt32 told = log.Count( "action" );
        ASSERT_FALSE( form.PerformRowAction( yours.matchId, "Nudge" ) || form.PerformRowAction( yours.matchId, "Remove" ) || form.PerformRowAction( yours.matchId, "Rematch" ) );
        ASSERT_FALSE( form.PerformRowAction( finished.matchId, "Resign" ) || form.PerformRowAction( finished.matchId, "Archive" ) || form.PerformRowAction( "unknown", "Mute" ) );
        ASSERT_TRUE( told == log.Count( "action" ) && 0 == f.provider->nudgeCount && 0 == f.provider->resignCount && 0 == f.provider->rematchCount );

        // nudge the player on turn, resign, rematch: the provider is asked
        ASSERT_TRUE( form.PerformRowAction( theirs.matchId, "Nudge" ) );
        ASSERT_TRUE( 1 == f.provider->nudgeCount && "user-bob" == f.provider->lastNudgedUserId );
        ASSERT_TRUE( ( "action:" + theirs.matchId + "|" + CORE::Int32ToString( GAMEGUI::MATCHACTION_NUDGE ) ) == log.Last() );
        ASSERT_TRUE( form.PerformRowAction( yours.matchId, "Resign" ) );
        ASSERT_TRUE( 1 == f.provider->resignCount );
        ASSERT_TRUE( ( "action:" + yours.matchId + "|" + CORE::Int32ToString( GAMEGUI::MATCHACTION_RESIGN ) ) == log.Last() );
        ASSERT_TRUE( form.PerformRowAction( finished.matchId, "Rematch" ) );
        ASSERT_TRUE( 1 == f.provider->rematchCount );
        ASSERT_TRUE( ( "action:" + finished.matchId + "|" + CORE::Int32ToString( GAMEGUI::MATCHACTION_REMATCH ) ) == log.Last() );

        // archive leaves the list, remove (finished) takes the match out of the book
        ASSERT_TRUE( form.PerformRowAction( theirs.matchId, "Archive" ) );
        ASSERT_TRUE( f.book.GetEntry( theirs.matchId, entry ) && entry.archived && -1 == form.FindRow( theirs.matchId ) );
        ASSERT_TRUE( ( "action:" + theirs.matchId + "|" + CORE::Int32ToString( GAMEGUI::MATCHACTION_ARCHIVE ) ) == log.Last() );
        form.SetShowArchived( true );
        ASSERT_TRUE( form.PerformRowAction( theirs.matchId, "Unarchive" ) );
        ASSERT_TRUE( f.book.GetEntry( theirs.matchId, entry ) && !entry.archived );
        ASSERT_TRUE( ( "action:" + theirs.matchId + "|" + CORE::Int32ToString( GAMEGUI::MATCHACTION_UNARCHIVE ) ) == log.Last() );
        ASSERT_TRUE( form.PerformRowAction( finished.matchId, "Remove" ) );
        ASSERT_FALSE( f.book.GetEntry( finished.matchId, entry ) );
        ASSERT_TRUE( -1 == form.FindRow( finished.matchId ) );
        ASSERT_TRUE( ( "action:" + finished.matchId + "|" + CORE::Int32ToString( GAMEGUI::MATCHACTION_REMOVE ) ) == log.Last() );
        ASSERT_TRUE( 0 == log.Count( "open" ) );

        // the same through the menu the user sees: the "..." button and a right click
        context->FlushChanges();
        GUI::CWidget* menuButton = form.GetRowWidget( yours.matchId, "Menu" );
        ASSERT_TRUE( GUCEF_NULL != menuButton && menuButton->GetRect().height >= 44 && menuButton->GetRect().width >= 44 );
        ClickWidget( context, menuButton );
        context->RenderFrame( 500 );
        ASSERT_TRUE( GUCEF_NULL != context->GetPopup() );
        ASSERT_TRUE( ChooseMenuItem( context, "Mute" ) );
        ASSERT_TRUE( f.book.GetEntry( yours.matchId, entry ) && entry.muted && GUCEF_NULL == context->GetPopup() );
        ClickRowOf( context, form, yours.matchId, GUI::MOUSEBUTTON_RIGHT );
        ASSERT_TRUE( GUCEF_NULL != context->GetPopup() && yours.matchId == form.GetSelectedMatchId() );
        GUI::CPopupMenu* popup = GAMEGUI::WidgetAs< GUI::CPopupMenu >( context->GetPopup(), "PopupMenu" );
        ASSERT_TRUE( GUCEF_NULL != popup && "Open" == popup->GetItems()[ 0 ] && "Unmute" == popup->GetItems()[ 1 ] && popup->GetRowHeight() >= 44 );
        ASSERT_TRUE( ChooseMenuItem( context, "Unmute" ) );
        ASSERT_TRUE( f.book.GetEntry( yours.matchId, entry ) && !entry.muted );
        // Open in the menu asks the application to open the match, nothing more
        ClickRowOf( context, form, yours.matchId, GUI::MOUSEBUTTON_RIGHT );
        ASSERT_TRUE( ChooseMenuItem( context, "Open" ) );
        ASSERT_TRUE( ( "open:" + yours.matchId ) == log.Last() && 1 == log.Count( "open" ) );
        ASSERT_TRUE( f.book.GetOpenMatchId().IsNULLOrEmpty() );

        // a right click on a header opens nothing
        context->FlushChanges();
        GUI::CWidget* header = form.GetWidget( "Header0" );
        ASSERT_TRUE( GUCEF_NULL != header );
        if ( GUCEF_NULL != header )
        {
            ClickAt( context, header->GetRect().left + 10, header->GetRect().top + 10, GUI::MOUSEBUTTON_RIGHT );
            ASSERT_TRUE( GUCEF_NULL == context->GetPopup() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchList: Rename shows a panel with a field, OK gives the game the name in the book, Cancel and Escape change nothing" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        SMatchBookFixture f;
        ASSERT_TRUE( f.Setup() );
        GAMEGUI::CMatchListForm form( context );
        form.SetClock( &f.clock );
        form.SetBook( &f.book );
        CEventLog log;
        log.Watch( &form, GAMEGUI::CMatchListForm::MatchActionEvent, "action" );
        MULTIPLAY::CMatchInfo m = f.AddMatch( "Shared name", SMatchBookFixture::KIND_YOURS );
        MULTIPLAY::CMatchInfo other = f.AddMatch( "Other", SMatchBookFixture::KIND_THEIRS );
        ASSERT_TRUE( f.Start() );
        ASSERT_TRUE( form.Open() );
        context->FlushChanges();
        GUI::CWidget* panel = form.GetWidget( "RenamePanel" );
        GUI::CWidget* edit = form.GetWidget( "RenameEdit" );
        ASSERT_TRUE( GUCEF_NULL != panel && GUCEF_NULL != edit );
        if ( GUCEF_NULL == panel || GUCEF_NULL == edit )
            return;
        ASSERT_FALSE( panel->IsVisible() || form.IsRenaming() );

        // the panel opens with the name the game has
        ASSERT_TRUE( form.PerformRowAction( m.matchId, "Rename" ) );
        ASSERT_TRUE( form.IsRenaming() && panel->IsVisible() && "Shared name" == edit->GetProperty( "Text" ) );
        ASSERT_FALSE( form.GetWidget( "Open" )->IsVisible() );
        ASSERT_TRUE( 0 == log.Count( "action" ) );
        context->FlushChanges();
        ASSERT_TRUE( edit->GetRect().height >= 44 && form.GetWidget( "RenameOk" )->GetRect().height >= 44 && form.GetWidget( "RenameCancel" )->GetRect().height >= 44 );

        // typed and accepted with the button
        TypeInto( context, edit, "Friday game", false );
        ClickWidget( context, form.GetWidget( "RenameOk" ) );
        MULTIPLAY::CMatchEntry entry;
        ASSERT_TRUE( f.book.GetEntry( m.matchId, entry ) && "Friday game" == entry.localTitle && "Shared name" == entry.info.title );
        ASSERT_FALSE( form.IsRenaming() || panel->IsVisible() );
        ASSERT_TRUE( form.GetWidget( "Open" )->IsVisible() );
        TRowInfo row;
        ASSERT_TRUE( form.GetRow( form.FindRow( m.matchId ), row ) && "Friday game" == row.title );
        ASSERT_TRUE( ( "action:" + m.matchId + "|" + CORE::Int32ToString( GAMEGUI::MATCHACTION_RENAME ) ) == log.Last() );

        // Cancel and Escape: nothing changes, nothing is told
        const UInt32 told = log.Count( "action" );
        ASSERT_TRUE( form.BeginRename( m.matchId ) && "Friday game" == edit->GetProperty( "Text" ) );
        TypeInto( context, edit, "Not this", false );
        ClickWidget( context, form.GetWidget( "RenameCancel" ) );
        ASSERT_FALSE( form.IsRenaming() );
        ASSERT_TRUE( f.book.GetEntry( m.matchId, entry ) && "Friday game" == entry.localTitle && told == log.Count( "action" ) );
        ASSERT_TRUE( form.BeginRename( m.matchId ) );
        TypeInto( context, edit, "Not this either", false );
        PressKey( context, INPUT::KEYCODE_ESCAPE );
        ASSERT_FALSE( form.IsRenaming() );
        ASSERT_TRUE( f.book.GetEntry( m.matchId, entry ) && "Friday game" == entry.localTitle && told == log.Count( "action" ) );

        // Enter accepts; an empty name takes the player's name away again
        ASSERT_TRUE( form.BeginRename( m.matchId ) );
        TypeInto( context, edit, "Saturday game", true );
        ASSERT_TRUE( f.book.GetEntry( m.matchId, entry ) && "Saturday game" == entry.localTitle && !form.IsRenaming() );
        ASSERT_TRUE( form.BeginRename( m.matchId ) );
        TypeInto( context, edit, "   ", true );
        ASSERT_TRUE( f.book.GetEntry( m.matchId, entry ) && entry.localTitle.IsNULLOrEmpty() );
        ASSERT_TRUE( form.GetRow( form.FindRow( m.matchId ), row ) && "Shared name" == row.title );

        // a long name is bounded by the book, and the match going away cancels the rename
        ASSERT_TRUE( form.BeginRename( other.matchId ) );
        TypeInto( context, edit, LongTitle( 3 ), true );
        ASSERT_TRUE( f.book.GetEntry( other.matchId, entry ) && entry.localTitle.Length() > 0 && entry.localTitle.Length() <= 64 );
        ASSERT_TRUE( form.BeginRename( other.matchId ) && form.IsRenaming() );
        ASSERT_TRUE( f.book.SetArchived( other.matchId, true ) );
        ASSERT_FALSE( form.IsRenaming() || panel->IsVisible() );
        ASSERT_FALSE( form.BeginRename( "unknown" ) );
        ASSERT_FALSE( form.CommitRename() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchList: Open, New Game and Invitations notify the application, a double click and Enter open a row, a click selects" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        SMatchBookFixture f;
        ASSERT_TRUE( f.Setup() );
        GAMEGUI::CMatchListForm form( context );
        form.SetClock( &f.clock );
        form.SetBook( &f.book );
        CEventLog log;
        log.Watch( &form, GAMEGUI::CMatchListForm::OpenRequestedEvent, "open" );
        log.Watch( &form, GAMEGUI::CMatchListForm::NewGameRequestedEvent, "new" );
        log.Watch( &form, GAMEGUI::CMatchListForm::InvitationsRequestedEvent, "invitations" );
        MULTIPLAY::CMatchInfo a = f.AddMatch( "A", SMatchBookFixture::KIND_YOURS );
        MULTIPLAY::CMatchInfo b = f.AddMatch( "B", SMatchBookFixture::KIND_THEIRS );
        ASSERT_TRUE( f.Start() );
        ASSERT_TRUE( form.Open() );
        context->FlushChanges();

        // nothing is selected: Open is off and OpenSelected has nothing to open
        GUI::CWidget* openButton = form.GetWidget( "Open" );
        ASSERT_TRUE( form.GetSelectedMatchId().IsNULLOrEmpty() && !openButton->IsEnabled() && "Invitations" == form.GetWidget( "Invitations" )->GetProperty( "Text" ) );
        ASSERT_FALSE( form.OpenSelected() );
        ASSERT_TRUE( form.GetWidget( "Detail" )->GetProperty( "Text" ).IsNULLOrEmpty() );

        // a click selects, nothing opens
        ClickRowOf( context, form, b.matchId );
        ASSERT_TRUE( b.matchId == form.GetSelectedMatchId() && openButton->IsEnabled() && 0 == log.lines.size() );
        ASSERT_TRUE( "B" == form.GetWidget( "Detail" )->GetProperty( "Text" ) );
        ClickRowOf( context, form, a.matchId );
        ASSERT_TRUE( a.matchId == form.GetSelectedMatchId() && 0 == log.lines.size() );

        // the Open button
        ClickWidget( context, openButton );
        ASSERT_TRUE( ( "open:" + a.matchId ) == log.Last() && 1 == log.lines.size() );
        ASSERT_TRUE( f.book.GetOpenMatchId().IsNULLOrEmpty() );             // the application does that

        // Enter opens the selected row
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( ( "open:" + a.matchId ) == log.Last() && 2 == log.Count( "open" ) );

        // a double click opens the row it is on
        context->RenderFrame( 500 );
        GUI::CWidget* rowB = form.GetRowWidget( b.matchId, "Row" );
        const GUI::CRect seen = MfVisibleRect( rowB );
        ClickAt( context, seen.left + 30, seen.top + 10 );
        ClickAt( context, seen.left + 30, seen.top + 10 );
        context->RenderFrame( 300 );
        ASSERT_TRUE( ( "open:" + b.matchId ) == log.Last() && 3 == log.Count( "open" ) && b.matchId == form.GetSelectedMatchId() );

        // a double click on a header opens nothing
        GUI::CWidget* header = form.GetWidget( "Header0" );
        ASSERT_TRUE( GUCEF_NULL != header );
        context->RenderFrame( 500 );
        ClickAt( context, header->GetRect().left + 10, header->GetRect().top + 10 );
        ClickAt( context, header->GetRect().left + 10, header->GetRect().top + 10 );
        ASSERT_TRUE( 3 == log.Count( "open" ) && b.matchId == form.GetSelectedMatchId() );

        // New Game and Invitations
        ClickWidget( context, form.GetWidget( "NewGame" ) );
        ASSERT_TRUE( "new" == log.Last() && 1 == log.Count( "new" ) );
        context->RenderFrame( 500 );
        ClickWidget( context, form.GetWidget( "Invitations" ) );
        ASSERT_TRUE( "invitations" == log.Last() && 1 == log.Count( "invitations" ) );

        // the invitations are counted on the button
        f.AddMatch( "I1", SMatchBookFixture::KIND_INVITED );
        f.AddMatch( "I2", SMatchBookFixture::KIND_FORMING );
        f.book.RefreshAll();
        ASSERT_TRUE( "Invitations (2)" == form.GetWidget( "Invitations" )->GetProperty( "Text" ) );

        // the selection stays with its match when the rows change, and goes with the match
        ASSERT_TRUE( form.SelectRow( a.matchId ) && a.matchId == form.GetSelectedMatchId() );
        f.AddMatch( "Another", SMatchBookFixture::KIND_YOURS );
        f.book.RefreshAll();
        ASSERT_TRUE( a.matchId == form.GetSelectedMatchId() );
        ASSERT_TRUE( f.book.RemoveMatch( a.matchId ) );
        ASSERT_TRUE( form.GetSelectedMatchId().IsNULLOrEmpty() && !openButton->IsEnabled() );
        ASSERT_FALSE( form.SelectRow( a.matchId ) || form.SelectRow( "unknown" ) );
        ASSERT_FALSE( form.SelectRow( CORE::CString::Empty ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchList: The button of the application is hidden until it has a text, then it is shown, touch sized, apart from the others, and notifies ExtraRequestedEvent" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        SMatchBookFixture f;
        ASSERT_TRUE( f.Setup() );
        GAMEGUI::CMatchListForm form( context );
        form.SetClock( &f.clock );
        form.SetBook( &f.book );
        CEventLog log;
        log.Watch( &form, GAMEGUI::CMatchListForm::ExtraRequestedEvent, "extra" );
        f.AddMatch( "A", SMatchBookFixture::KIND_YOURS );
        ASSERT_TRUE( f.Start() );
        ASSERT_TRUE( form.Open() );
        context->FlushChanges();
        GUI::CWidget* extra = form.GetWidget( "Extra" );
        ASSERT_TRUE( GUCEF_NULL != extra && !extra->IsVisible() );

        // given a text before and after the window is built
        form.SetExtraButton( "Add game from folder" );
        context->FlushChanges();
        context->RenderFrame( 16 );
        ASSERT_TRUE( extra->IsVisible() && "Add game from folder" == extra->GetProperty( "Text" ) && extra->GetRect().height >= 44 );
        GUI::CWidget* invitations = form.GetWidget( "Invitations" );
        ASSERT_TRUE( invitations->GetRect().Right() <= extra->GetRect().left || extra->GetRect().Right() <= invitations->GetRect().left );
        ASSERT_TRUE( extra->GetRect().Right() <= form.GetWidget( "ButtonRow" )->GetRect().Right() );
        ClickWidget( context, extra );
        ASSERT_TRUE( "extra" == log.Last() && 1 == log.Count( "extra" ) );
        form.SetExtraButton( CORE::CString::Empty );
        context->FlushChanges();
        ASSERT_TRUE( !extra->IsVisible() );
        form.Close();
        form.SetExtraButton( "Again" );
        ASSERT_TRUE( form.Open() );
        context->FlushChanges();
        ASSERT_TRUE( form.GetWidget( "Extra" )->IsVisible() && "Again" == form.GetWidget( "Extra" )->GetProperty( "Text" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchList: The rows follow the book, and a book that goes away leaves an empty list and no dangling subscription" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        SMatchBookFixture f;
        ASSERT_TRUE( f.Setup() );
        GAMEGUI::CMatchListForm form( context );
        form.SetClock( &f.clock );
        MULTIPLAY::CMatchInfo a = f.AddMatch( "A", SMatchBookFixture::KIND_THEIRS );
        ASSERT_TRUE( f.Start() );
        ASSERT_TRUE( form.Open() );
        ASSERT_TRUE( 0 == form.GetRowCount() && GUCEF_NULL == form.GetBook() );

        // the subscriptions of the book: the changes and the turns, and of the context: the frames
        const UInt32 before = GetEventSubscriptionCount( form );
        form.SetBook( &f.book );
        ASSERT_TRUE( 2 == form.GetRowCount() && before + 2 == GetEventSubscriptionCount( form ) );
        form.SetBook( GUCEF_NULL );
        ASSERT_TRUE( 0 == form.GetRowCount() && before == GetEventSubscriptionCount( form ) );
        form.SetBook( &f.book );

        // a turn that arrives
        MULTIPLAY::CMatchInfo b = f.AddMatch( "B", SMatchBookFixture::KIND_THEIRS );
        f.Play( b, "user-me", 2 );
        f.book.RefreshAll();
        TRowInfo row;
        ASSERT_TRUE( form.GetRow( 0, row ) && "Your Turn (1)" == row.headerText );
        ASSERT_TRUE( form.GetRow( 1, row ) && row.matchId == b.matchId );

        // the book is destroyed first
        MULTIPLAY::CMatchBook* book = GUCEF_NEW MULTIPLAY::CMatchBook();
        book->SetLocalUserId( "user-me" );
        form.SetBook( book );
        ASSERT_TRUE( book == form.GetBook() );
        GUCEF_DELETE book;
        ASSERT_TRUE( GUCEF_NULL == form.GetBook() && 0 == form.GetRowCount() );
        context->RenderFrame( 1000 );
        form.UpdateRows();
        ASSERT_TRUE( 0 == form.GetRowCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchList: 300 matches with names of 200 characters scroll with the wheel, the thumb and the keys, and the selection stays in view" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( 1242, 952, true );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        SMatchBookFixture f;
        ASSERT_TRUE( f.Setup() );
        GAMEGUI::CMatchListForm form( context );
        form.SetClock( &f.clock );
        form.SetBook( &f.book );
        CORE::CString::StringVector ids;
        for ( UInt32 i=0; i<300; ++i )
        {
            SMatchBookFixture::EKind kind = 0 == i % 3 ? SMatchBookFixture::KIND_YOURS : ( 1 == i % 3 ? SMatchBookFixture::KIND_THEIRS : SMatchBookFixture::KIND_FINISHED );
            ids.push_back( f.AddMatch( LongTitle( i ), kind, 0 == i % 3 ? f.clock.nowMs + (Int64) i * 60000 : 0, (Int64) i * 1000 ).matchId );
        }
        ASSERT_TRUE( f.Start() );
        ASSERT_TRUE( 300 == f.book.GetEntryCount() );
        ASSERT_TRUE( form.Open() );
        context->RenderFrame( 16 );
        context->RenderFrame( 16 );
        ASSERT_TRUE( 303 == form.GetRowCount() );

        GUI::CScrollPanel* list = form.GetWidget( "List" )->AsScrollPanel();
        ASSERT_TRUE( GUCEF_NULL != list );
        if ( GUCEF_NULL == list )
            return;
        ASSERT_TRUE( list->IsScrollable() && list->GetContentHeight() >= 300 * 56 && 0 == list->GetScrollPosition() );
        CORE::CString problem = MfFindLayoutProblem( context, form.GetRootWidget() );
        if ( !problem.IsNULLOrEmpty() )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MatchList: 300 matches: " << problem.C_String() );
        ASSERT_TRUE( problem.IsNULLOrEmpty() );
        WriteShot( CaptureFrame( context ), "match-list-300-top" );

        // the wheel
        const GUI::CRect listRect = list->GetRect();
        context->InjectMouseMove( listRect.left + listRect.width / 2, listRect.top + listRect.height / 2 );
        context->InjectMouseWheel( -1 );
        context->FlushChanges();
        ASSERT_TRUE( list->GetScrollPosition() > 0 );
        context->InjectMouseWheel( 1 );
        context->FlushChanges();
        ASSERT_TRUE( 0 == list->GetScrollPosition() );

        // the thumb
        const GUI::CRect thumb = list->GetThumbRect();
        ASSERT_FALSE( thumb.IsEmpty() );
        const Int32 thumbX = listRect.left + thumb.left + thumb.width / 2;
        const Int32 thumbY = listRect.top + thumb.top + thumb.height / 2;
        context->InjectMouseMove( thumbX, thumbY );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseMove( thumbX, thumbY + 100 );
        ASSERT_TRUE( list->IsThumbDragged() && list->GetScrollPosition() > 300 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        context->FlushChanges();
        list->SetScrollPosition( 0 );
        context->FlushChanges();

        // the keys move the selection and keep it in view: Down from nothing selects the first match
        PressKey( context, INPUT::KEYCODE_DOWN );
        ASSERT_FALSE( form.GetSelectedMatchId().IsNULLOrEmpty() );
        TRowInfo row;
        ASSERT_TRUE( form.GetRow( 1, row ) && row.matchId == form.GetSelectedMatchId() );
        for ( UInt32 i=0; i<150; ++i )
        {
            PressKey( context, INPUT::KEYCODE_DOWN );
            context->FlushChanges();
            GUI::CWidget* selected = form.GetRowWidget( form.GetSelectedMatchId(), "Row" );
            ASSERT_TRUE( GUCEF_NULL != selected && MfInside( selected->GetRect(), list->GetRect() ) );
            if ( GUCEF_NULL == selected || !MfInside( selected->GetRect(), list->GetRect() ) )
                break;
        }
        ASSERT_TRUE( list->GetScrollPosition() > 0 );
        // 1 header + 150 moves down from the first match: the matches skip the headers
        const Int32 afterDown = form.FindRow( form.GetSelectedMatchId() );
        ASSERT_TRUE( afterDown > 100 );
        for ( UInt32 i=0; i<20; ++i )
        {
            PressKey( context, INPUT::KEYCODE_UP );
            context->FlushChanges();
            GUI::CWidget* selected = form.GetRowWidget( form.GetSelectedMatchId(), "Row" );
            ASSERT_TRUE( GUCEF_NULL != selected && MfInside( selected->GetRect(), list->GetRect() ) );
        }
        ASSERT_TRUE( form.FindRow( form.GetSelectedMatchId() ) < afterDown );
        PressKey( context, INPUT::KEYCODE_PGDOWN );
        context->FlushChanges();
        const Int32 afterPage = form.FindRow( form.GetSelectedMatchId() );
        ASSERT_TRUE( afterPage > afterDown - 20 + 3 );
        ASSERT_TRUE( MfInside( form.GetRowWidget( form.GetSelectedMatchId(), "Row" )->GetRect(), list->GetRect() ) );
        PressKey( context, INPUT::KEYCODE_PGUP );
        context->FlushChanges();
        ASSERT_TRUE( form.FindRow( form.GetSelectedMatchId() ) < afterPage );
        PressKey( context, INPUT::KEYCODE_END );
        context->FlushChanges();
        ASSERT_TRUE( form.GetRow( 302, row ) && row.matchId == form.GetSelectedMatchId() );
        ASSERT_TRUE( list->GetScrollPosition() == list->GetMaxScrollPosition() );
        ASSERT_TRUE( MfInside( form.GetRowWidget( form.GetSelectedMatchId(), "Row" )->GetRect(), list->GetRect() ) );
        PressKey( context, INPUT::KEYCODE_DOWN );
        ASSERT_TRUE( form.GetRow( 302, row ) && row.matchId == form.GetSelectedMatchId() );        // nowhere to go
        problem = MfFindLayoutProblem( context, form.GetRootWidget() );
        if ( !problem.IsNULLOrEmpty() )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MatchList: 300 matches at the end: " << problem.C_String() );
        ASSERT_TRUE( problem.IsNULLOrEmpty() );
        WriteShot( CaptureFrame( context ), "match-list-300-end" );
        PressKey( context, INPUT::KEYCODE_HOME );
        context->FlushChanges();
        ASSERT_TRUE( form.GetRow( 1, row ) && row.matchId == form.GetSelectedMatchId() && 0 == list->GetScrollPosition() );

        // selecting a match far down scrolls to it
        ASSERT_TRUE( form.SelectRow( ids[ 250 ] ) );
        context->FlushChanges();
        ASSERT_TRUE( list->GetScrollPosition() > 0 && MfInside( form.GetRowWidget( ids[ 250 ], "Row" )->GetRect(), list->GetRect() ) );

        // the long names are cut to the box and the whole name is in the model and the detail line
        TRowInfo info;
        ASSERT_TRUE( form.GetRow( form.FindRow( ids[ 250 ] ), info ) && LongTitle( 250 ) == info.title );
        const CORE::CString shown = LabelText( form, ids[ 250 ], "Title" );
        ASSERT_TRUE( shown.Length() < info.title.Length() && shown.HasSubstr( "...", true ) >= 0 );
        ASSERT_TRUE( form.GetWidget( "Detail" )->GetProperty( "Text" ).Length() < info.title.Length() && form.GetWidget( "Detail" )->GetProperty( "Text" ).HasSubstr( "...", true ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchList: Layout from 800 x 600 to 3840 x 1080: controls inside their parent, apart, touch sized, text inside its box, window on the screen" )
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
            GAMEGUI::CMatchListForm form( context );
            form.SetClock( &f.clock );
            form.SetBook( &f.book );
            const UInt32 matchCount = 40;
            CORE::CString firstId;
            for ( UInt32 i=0; i<matchCount; ++i )
            {
                SMatchBookFixture::EKind kind = static_cast< SMatchBookFixture::EKind >( i % 5 );
                MULTIPLAY::CMatchInfo info = f.AddMatch( 0 == i % 4 ? LongTitle( i ) : ( "Game " + CORE::UInt32ToString( i ) ), kind, 0 == i % 2 ? f.clock.nowMs + (Int64) i * 7200000 : 0, (Int64) i );
                if ( 0 == i )
                    firstId = info.matchId;
            }
            ASSERT_TRUE( f.Start() );
            ASSERT_TRUE( form.Open() );
            ASSERT_TRUE( f.book.SetMuted( firstId, true ) && f.book.SetPinned( firstId, true ) );
            form.SetMatchProblem( firstId, "A long text about what went wrong with the package of this match, longer than the detail line" );
            ASSERT_TRUE( form.SelectRow( firstId ) );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );

            GUI::CWidget* window = form.GetRootWidget();
            const GUI::CRect rect = window->GetRect();
            ASSERT_TRUE( rect.left >= 0 && rect.top >= 0 && rect.Right() <= (Int32) screenWidth && rect.Bottom() <= (Int32) screenHeight );
            ASSERT_TRUE( rect.width >= 560 && rect.width <= 1000 && rect.height >= 400 );
            ASSERT_TRUE( context->GetModalRoot() != window && "Window" == window->GetType() && window->GetBoolProperty( "Movable" ) );

            GUI::CWidget* list = form.GetWidget( "List" );
            GUI::CWidget* strip = form.GetWidget( "ButtonRow" );
            ASSERT_TRUE( GUCEF_NULL != list && GUCEF_NULL != strip );
            if ( GUCEF_NULL == list || GUCEF_NULL == strip )
                return;
            ASSERT_TRUE( list->GetRect().Bottom() <= strip->GetRect().top && strip->GetRect().Bottom() <= rect.Bottom() );
            ASSERT_TRUE( list->GetRect().height >= 200 );

            CORE::CString problem = MfFindLayoutProblem( context, window );
            if ( !problem.IsNULLOrEmpty() )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MatchList: layout" << where.C_String() << ": " << problem.C_String() );
            ASSERT_TRUE( problem.IsNULLOrEmpty() );

            // every row is high enough, and the three buttons are touch sized
            UInt32 rowsInView = 0;
            for ( UInt32 i=0; i<form.GetRowCount(); ++i )
            {
                TRowInfo row;
                ASSERT_TRUE( form.GetRow( i, row ) );
                GUI::CWidget* rowWidget = row.isHeader ? GUCEF_NULL : form.GetRowWidget( row.matchId, "Row" );
                if ( GUCEF_NULL != rowWidget )
                {
                    ASSERT_TRUE( rowWidget->GetRect().height >= 56 );
                    ++rowsInView;
                }
            }
            ASSERT_TRUE( rowsInView >= 3 );
            ASSERT_TRUE( form.GetWidget( "Open" )->GetRect().height >= 44 && form.GetWidget( "NewGame" )->GetRect().height >= 44 && form.GetWidget( "Invitations" )->GetRect().height >= 44 );
            WriteShot( CaptureFrame( context ), "match-list-" + SizeName( screenWidth, screenHeight ) );

            // scrolled to the end it is still in order
            GUI::CScrollPanel* scroll = list->AsScrollPanel();
            ASSERT_TRUE( GUCEF_NULL != scroll );
            if ( GUCEF_NULL != scroll )
            {
                scroll->SetScrollPosition( scroll->GetMaxScrollPosition() );
                context->FlushChanges();
                problem = MfFindLayoutProblem( context, window );
                ASSERT_TRUE( problem.IsNULLOrEmpty() );
                scroll->SetScrollPosition( 0 );
            }

            // the rename panel in the strip
            ASSERT_TRUE( form.BeginRename( firstId ) );
            context->FlushChanges();
            problem = MfFindLayoutProblem( context, window );
            if ( !problem.IsNULLOrEmpty() )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MatchList: rename panel" << where.C_String() << ": " << problem.C_String() );
            ASSERT_TRUE( problem.IsNULLOrEmpty() );
            WriteShot( CaptureFrame( context ), "match-list-rename-" + SizeName( screenWidth, screenHeight ) );
            form.CancelRename();

            // the menu, open
            ASSERT_TRUE( form.OpenRowMenu( firstId, rect.left + 100, rect.top + 100 ) );
            context->FlushChanges();
            GUI::CPopupMenu* popup = GAMEGUI::WidgetAs< GUI::CPopupMenu >( context->GetPopup(), "PopupMenu" );
            ASSERT_TRUE( GUCEF_NULL != popup && popup->GetRowHeight() >= 44 );
            WriteShot( CaptureFrame( context ), "match-list-menu-" + SizeName( screenWidth, screenHeight ) );
            context->ClosePopup();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchList: The window follows a change of resolution, cuts the names again, and 300 matches keep their layout on the widest screens" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( 800, 600, true );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        SMatchBookFixture f;
        ASSERT_TRUE( f.Setup() );
        GAMEGUI::CMatchListForm form( context );
        form.SetClock( &f.clock );
        form.SetBook( &f.book );
        MULTIPLAY::CMatchInfo m = f.AddMatch( LongTitle( 1 ), SMatchBookFixture::KIND_THEIRS, f.clock.nowMs + 5000 );
        for ( UInt32 i=0; i<300; ++i )
            f.AddMatch( LongTitle( i + 2 ), static_cast< SMatchBookFixture::EKind >( i % 5 ), 0, (Int64) i );
        ASSERT_TRUE( f.Start() );
        ASSERT_TRUE( form.Open() );
        context->RenderFrame( 16 );
        context->RenderFrame( 16 );
        ASSERT_TRUE( form.SelectRow( m.matchId ) );
        context->FlushChanges();
        const CORE::CString narrow = LabelText( form, m.matchId, "Title" );
        for ( UInt32 s=0; s<ScreenSizeCount; ++s )
        {
            context->SetLogicalResolution( ScreenSizes[ s ].width, ScreenSizes[ s ].height );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );
            CORE::CString problem = MfFindLayoutProblem( context, form.GetRootWidget() );
            if ( !problem.IsNULLOrEmpty() )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MatchList: changed to " << SizeName( ScreenSizes[ s ].width, ScreenSizes[ s ].height ).C_String() << ": " << problem.C_String() );
            ASSERT_TRUE( problem.IsNULLOrEmpty() );
            const GUI::CRect rect = form.GetRootWidget()->GetRect();
            ASSERT_TRUE( rect.Right() <= (Int32) ScreenSizes[ s ].width && rect.Bottom() <= (Int32) ScreenSizes[ s ].height );
        }
        // the wide window shows more of the name than the narrow one
        ASSERT_TRUE( form.SelectRow( m.matchId ) );
        context->FlushChanges();
        ASSERT_TRUE( LabelText( form, m.matchId, "Title" ).Length() > narrow.Length() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
