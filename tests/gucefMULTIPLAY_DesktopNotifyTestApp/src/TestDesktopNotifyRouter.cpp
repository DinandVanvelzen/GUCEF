/*
 *  gucefMULTIPLAY_DesktopNotifyTestApp: Application for testing the desktop match alert sink plugin of gucefMULTIPLAY
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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H
#include "../../gucefMULTIPLAY_TestApp/include/TestMatchSupport.h"
#define GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H
#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H ? */

#ifndef GUCEF_MULTIPLAY_TESTAPP_TESTFAKEMATCHPROVIDER_H
#include "../../gucefMULTIPLAY_TestApp/include/TestFakeMatchProvider.h"
#define GUCEF_MULTIPLAY_TESTAPP_TESTFAKEMATCHPROVIDER_H
#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTFAKEMATCHPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTSINK_H
#include "notifysinkpluginDESKTOP_CDesktopMatchAlertSink.h"
#define GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTSINK_H
#endif /* GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTSINK_H ? */

#include "TestDesktopNotifyCommon.h"
#include "TestDesktopNotify.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::MULTIPLAYPLUGIN::DESKTOP;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Records the failure and abandons the test case, for what the rest of it needs */
#define REQUIRE_TRUE(t) do { if ( !(t) ) { GUCEF_TESTFW_ASSERT_TRUE( t ); throw CORE::CString( "required: " #t ); } } while ( 0 )

typedef INotifyLauncher::TArgumentVector TArgv;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A match book that can be made to raise events the way a flaky source would: more than once */
class CTestBook : public MULTIPLAY::CMatchBook
{
    public:

    void RaiseYourTurn( const CORE::CString& matchId, UInt32 revision )
    {
        MULTIPLAY::CMatchTurnEventData data( matchId, revision );
        NotifyObservers( YourTurnEvent, &data );
    }
};

/*-------------------------------------------------------------------------*/

/** The local player (identity 0) with a book, a fake provider, the desktop sink on a fake launcher and a router */
struct SDesktopFixture
{
    GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
    TESTFW::CManualMatchClock clock;
    CTestBook book;
    CFakeNotifyLauncher launcher;
    CDesktopMatchAlertSink desktop;
    MULTIPLAY::CMatchAlertRouter router;
    MULTIPLAY::CIMatchProviderPtr providerPtr;
    TESTFW::CFakeMatchProvider* provider;
    GUCEF::map< CORE::CString, GUCEF::vector< MULTIPLAY::CMatchPackage > > packages;

    SDesktopFixture( void ) : ids(), clock(), book(), launcher(), desktop( &launcher ), router(), providerPtr(), provider( GUCEF_NULL ), packages() {}

    bool Setup( void )
    {
        const char* names[ 3 ] = { "Me", "Bob", "Carol" };
        for ( int i=0; i<3; ++i )
        {
            MULTIPLAY::CPlayerIdentity identity;
            if ( !TESTFW::MakeTestIdentity( names[ i ], identity ) )
                return false;
            ids.push_back( identity );
        }
        clock.SetMinuteOfDay( 12 * 60 );
        book.SetClock( &clock );
        book.SetLocalUserId( ids[ 0 ].userId );
        router.SetClock( &clock );
        router.SetBook( &book );
        router.AddSink( &desktop );
        providerPtr = MULTIPLAY::CMultiplayGlobal::Instance()->GetMatchProviderFactory().Create( "fake" );
        if ( providerPtr.IsNULL() )
            return false;
        provider = static_cast< TESTFW::CFakeMatchProvider* >( providerPtr.GetPointerAlways() );
        return book.AddProvider( providerPtr ) && book.Start();
    }

    MULTIPLAY::CMatchInfo NewMatch( const char* title, int holder, MULTIPLAY::TMatchStatus status = MULTIPLAY::MATCHSTATUS_ACTIVE, bool meInvited = false )
    {
        MULTIPLAY::CMatchInfo info = TESTFW::MakeTestMatchInfo( ids, title, true );
        info.status = status;
        if ( meInvited )
            info.participants[ 0 ].status = MULTIPLAY::MATCHPARTICIPANT_INVITED;
        MULTIPLAY::CMatchPackage p1 = TESTFW::MakeTestPackage( 1, GUCEF_NULL, title );
        MULTIPLAY::CMatchPreview preview = TESTFW::MakeTestPreview( info, ids[ 0 ], p1, holder >= 0 ? &ids[ holder ] : GUCEF_NULL, status, 0 );
        provider->InjectMatch( info, preview, p1 );
        packages[ info.matchId ].push_back( p1 );
        return info;
    }

    void Play( const MULTIPLAY::CMatchInfo& info, int issuer, int holder, MULTIPLAY::TMatchStatus status = MULTIPLAY::MATCHSTATUS_ACTIVE )
    {
        GUCEF::vector< MULTIPLAY::CMatchPackage >& list = packages[ info.matchId ];
        MULTIPLAY::CMatchPackage next = TESTFW::MakeTestPackage( list.back().revision + 1, &list.back(), "turn" );
        provider->InjectTurn( TESTFW::MakeTestPreview( info, ids[ issuer ], next, holder >= 0 ? &ids[ holder ] : GUCEF_NULL, status, 0 ), next );
        list.push_back( next );
    }
};

/*-------------------------------------------------------------------------*/

static bool
SameArgv( const TArgv& a, const TArgv& b )
{
    if ( a.size() != b.size() )
        return false;
    for ( size_t i=0; i<a.size(); ++i )
        if ( !( a[ i ] == b[ i ] ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDesktopNotifyRouterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "notifysinkpluginDESKTOP: the desktop match alert sink behind the match alert router" );

    TESTFW::TFakeMatchProviderFactory fakeFactory;
    MULTIPLAY::CMultiplayGlobal::Instance()->GetMatchProviderFactory().RegisterConcreteFactory( "fake", &fakeFactory );
    const bool signaturesAvailable = MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable();

    GUCEF_TESTFW_TESTCASE( "Test: A turn in a match reaches the launcher exactly once, however often the book reports it" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SDesktopFixture f;
            REQUIRE_TRUE( f.Setup() );
            MULTIPLAY::CMatchInfo m = f.NewMatch( "Friday game", 0 );
            f.book.RefreshAll();
            REQUIRE_TRUE( 1 == f.launcher.launched.size() );
            ASSERT_TRUE( SameArgv( f.launcher.launched[ 0 ], MakeArgv( "notify-send", "--app-name=Game", "--urgency=normal", "--", "Friday game", "It is your turn (after Me)", GUCEF_NULL ) ) );
            f.book.RaiseYourTurn( m.matchId, 1 );
            f.book.RaiseYourTurn( m.matchId, 1 );
            f.book.RefreshAll();
            f.router.Update();
            ASSERT_TRUE( 1 == f.launcher.launched.size() && 1 == f.desktop.GetLaunchedCount() );
            // the next revision is a new turn
            f.Play( m, 1, 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 2 == f.launcher.launched.size() );
            ASSERT_TRUE( SameArgv( f.launcher.launched[ 1 ], MakeArgv( "notify-send", "--app-name=Game", "--urgency=normal", "--", "Friday game", "It is your turn (after Bob)", GUCEF_NULL ) ) );
            // a match in which it is another player's turn is nothing to tell
            f.NewMatch( "Other", 1 );
            f.book.RefreshAll();
            ASSERT_TRUE( 2 == f.launcher.launched.size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A muted match starts nothing, and clearing a match on the router starts nothing either" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SDesktopFixture f;
            REQUIRE_TRUE( f.Setup() );
            MULTIPLAY::CMatchInfo m1 = f.NewMatch( "M1", 1 );
            MULTIPLAY::CMatchInfo m2 = f.NewMatch( "M2", 1 );
            f.book.RefreshAll();
            REQUIRE_TRUE( f.book.SetMuted( m2.matchId, true ) );
            f.Play( m1, 1, 0 );
            f.Play( m2, 1, 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 1 == f.launcher.launched.size() );
            ASSERT_TRUE( 6 == f.launcher.launched[ 0 ].size() );
            ASSERT_TRUE( "M1" == f.launcher.launched[ 0 ][ 4 ] );

            // the player opens the match elsewhere: the notification is taken back from the sinks, which for this one is nothing
            f.router.ClearForMatch( m1.matchId );
            f.desktop.Clear( m1.matchId );
            ASSERT_TRUE( 1 == f.launcher.launched.size() );

            // unmuted, the next turn comes through
            REQUIRE_TRUE( f.book.SetMuted( m2.matchId, false ) );
            f.Play( m2, 1, 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 2 == f.launcher.launched.size() && "M2" == f.launcher.launched[ 1 ][ 4 ] );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Quiet hours hold the notification, and the games that waited are one notification when they end" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SDesktopFixture f;
            REQUIRE_TRUE( f.Setup() );
            f.router.SetQuietHours( true, 22 * 60, 8 * 60 );
            MULTIPLAY::CMatchInfo m1 = f.NewMatch( "M1", 1 );
            MULTIPLAY::CMatchInfo m2 = f.NewMatch( "M2", 1 );
            f.book.RefreshAll();

            f.clock.SetMinuteOfDay( 23 * 60 );
            f.Play( m1, 1, 0 );
            f.book.RefreshAll();
            f.clock.SetMinuteOfDay( 3 * 60 );
            f.Play( m2, 1, 0 );
            f.book.RefreshAll();
            f.router.Update();
            ASSERT_TRUE( 0 == f.launcher.launched.size() && 2 == f.router.GetHeldCount() );
            f.clock.SetMinuteOfDay( 7 * 60 + 59 );
            f.router.Update();
            ASSERT_TRUE( 0 == f.launcher.launched.size() );
            f.clock.SetMinuteOfDay( 8 * 60 );
            f.router.Update();
            REQUIRE_TRUE( 1 == f.launcher.launched.size() );
            ASSERT_TRUE( SameArgv( f.launcher.launched[ 0 ], MakeArgv( "notify-send", "--app-name=Game", "--urgency=normal", "--", "2 games are waiting for you", GUCEF_NULL ) ) );
            f.router.Update();
            ASSERT_TRUE( 1 == f.launcher.launched.size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The minimum interval holds the second notification until it has passed" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SDesktopFixture f;
            REQUIRE_TRUE( f.Setup() );
            f.router.SetMinimumIntervalMs( 600000 );
            f.NewMatch( "M1", 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 1 == f.launcher.launched.size() );
            f.clock.AdvanceMs( 60000 );
            f.NewMatch( "M2", 0 );
            f.book.RefreshAll();
            f.clock.AdvanceMs( 60000 );
            f.NewMatch( "M3", 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 1 == f.launcher.launched.size() && 2 == f.router.GetHeldCount() );
            f.clock.AdvanceMs( 400000 );
            f.router.Update();
            ASSERT_TRUE( 1 == f.launcher.launched.size() );
            f.clock.AdvanceMs( 140000 );
            f.router.Update();
            REQUIRE_TRUE( 2 == f.launcher.launched.size() );
            ASSERT_TRUE( "2 games are waiting for you" == f.launcher.launched[ 1 ][ 4 ] && 5 == f.launcher.launched[ 1 ].size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The daily digest reaches the launcher as a digest" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SDesktopFixture f;
            REQUIRE_TRUE( f.Setup() );
            f.router.SetMode( MULTIPLAY::MATCHALERTMODE_DAILY_DIGEST );
            f.router.SetDigestMinute( 9 * 60 );
            f.clock.SetMinuteOfDay( 8 * 60 );
            f.router.Update();
            f.NewMatch( "M1", 0 );
            f.NewMatch( "M2", 0 );
            f.NewMatch( "M3", 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 0 == f.launcher.launched.size() );
            f.clock.SetMinuteOfDay( 9 * 60 );
            f.router.Update();
            REQUIRE_TRUE( 1 == f.launcher.launched.size() );
            ASSERT_TRUE( SameArgv( f.launcher.launched[ 0 ], MakeArgv( "notify-send", "--app-name=Game", "--urgency=normal", "--", "Daily digest", "3 games are waiting for you", GUCEF_NULL ) ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An invitation is told without the title and the name, and the end of a match is not told to the desktop" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SDesktopFixture f;
            REQUIRE_TRUE( f.Setup() );
            MULTIPLAY::CMatchInfo m1 = f.NewMatch( "Running game", 1 );
            f.book.RefreshAll();
            f.NewMatch( "Secret title", 1, MULTIPLAY::MATCHSTATUS_ACTIVE, true );
            f.book.RefreshAll();
            f.book.RefreshAll();
            REQUIRE_TRUE( 1 == f.launcher.launched.size() );
            ASSERT_TRUE( SameArgv( f.launcher.launched[ 0 ], MakeArgv( "notify-send", "--app-name=Game", "--urgency=normal", "--", "You have a game invitation", GUCEF_NULL ) ) );
            f.Play( m1, 0, -1, MULTIPLAY::MATCHSTATUS_FINISHED );
            f.book.RefreshAll();
            ASSERT_TRUE( 1 == f.launcher.launched.size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    MULTIPLAY::CMultiplayGlobal::Instance()->GetMatchProviderFactory().UnregisterConcreteFactory( "fake" );
}

/*-------------------------------------------------------------------------*/
