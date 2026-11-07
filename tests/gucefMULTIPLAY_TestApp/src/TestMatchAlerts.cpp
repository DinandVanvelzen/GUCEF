/*
 *  gucefMULTIPLAY_TestApp: Application for testing gucefMULTIPLAY
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
#include "TestMatchSupport.h"
#define GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H
#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H ? */

#ifndef GUCEF_MULTIPLAY_TESTAPP_TESTFAKEMATCHPROVIDER_H
#include "TestFakeMatchProvider.h"
#define GUCEF_MULTIPLAY_TESTAPP_TESTFAKEMATCHPROVIDER_H
#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTFAKEMATCHPROVIDER_H ? */

#include "TestMatchCommon.h"
#include "TestMatchAlerts.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Records the failure and abandons the test case, for what the rest of it needs */
#define REQUIRE_TRUE(t) do { if ( !(t) ) { GUCEF_TESTFW_ASSERT_TRUE( t ); throw CORE::CString( "required: " #t ); } } while ( 0 )

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

/** Records the events of the in-app sink */
class CInAppRecorder : public CORE::CObserver
{
    public:

    UInt32 added;
    UInt32 cleared;
    UInt32 countChanges;
    UInt32 lastCount;

    CInAppRecorder( void ) : CORE::CObserver(), added( 0 ), cleared( 0 ), countChanges( 0 ), lastCount( 0 ) {}

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CInAppRecorder";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MULTIPLAY::CInAppMatchAlertSink::MatchAlertAddedEvent == eventId ) ++added;
        else
        if ( MULTIPLAY::CInAppMatchAlertSink::MatchAlertClearedEvent == eventId ) ++cleared;
        else
        if ( MULTIPLAY::CInAppMatchAlertSink::PendingCountChangedEvent == eventId )
        {
            ++countChanges;
            lastCount = static_cast< const CORE::TCloneableUInt32* >( eventData )->GetData();
        }
    }
};

/*-------------------------------------------------------------------------*/

/** The local player (identity 0) with a book, a fake provider, an in-app sink, a sink that interrupts and a router */
struct SRouterFixture
{
    GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
    TESTFW::CManualMatchClock clock;
    CTestBook book;
    MULTIPLAY::CInAppMatchAlertSink inApp;
    TESTFW::CRecordingMatchAlertSink desktop;
    MULTIPLAY::CMatchAlertRouter router;
    MULTIPLAY::CIMatchProviderPtr providerPtr;
    TESTFW::CFakeMatchProvider* provider;
    GUCEF::map< CORE::CString, GUCEF::vector< MULTIPLAY::CMatchPackage > > packages;

    SRouterFixture( void ) : ids(), clock(), book(), inApp(), desktop( true ), router(), providerPtr(), provider( GUCEF_NULL ), packages() {}

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
        router.AddSink( &inApp );
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

    /** The local player plays: hands over the next revision to the given holder */
    bool PlayMyTurn( const MULTIPLAY::CMatchInfo& info, int holder )
    {
        GUCEF::vector< MULTIPLAY::CMatchPackage >& list = packages[ info.matchId ];
        MULTIPLAY::CMatchPackage next = TESTFW::MakeTestPackage( list.back().revision + 1, &list.back(), "my turn" );
        MULTIPLAY::CMatchPreview preview = TESTFW::MakeTestPreview( info, ids[ 0 ], next, &ids[ holder ], MULTIPLAY::MATCHSTATUS_ACTIVE, 0 );
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        bool ok = book.Submit( info.matchId, preview, next, future );
        if ( ok )
            list.push_back( next );
        return ok;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMatchAlertTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefMULTIPLAY matches: notifications and links" );

    TESTFW::TFakeMatchProviderFactory fakeFactory;
    MULTIPLAY::CMultiplayGlobal::Instance()->GetMatchProviderFactory().RegisterConcreteFactory( "fake", &fakeFactory );
    const bool signaturesAvailable = MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable();

    GUCEF_TESTFW_TESTCASE( "Test: A turn is told to the sinks with the title and the previous mover and nothing else" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SRouterFixture f;
            REQUIRE_TRUE( f.Setup() );
            MULTIPLAY::CMatchInfo m = f.NewMatch( "Friday game", 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 1 == f.desktop.delivered.size() && 1 == f.inApp.GetPendingCount() );
            const MULTIPLAY::CMatchAlert& n = f.desktop.delivered[ 0 ];
            ASSERT_TRUE( MULTIPLAY::MATCHALERTKIND_YOUR_TURN == n.kind && m.matchId == n.matchId && 1 == n.revision );
            ASSERT_TRUE( "Friday game" == n.title && "Me" == n.previousMover && 1 == n.count && n.createdAt == f.clock.nowMs );
            MULTIPLAY::TMatchAlertVector shown;
            f.inApp.GetPending( shown );
            ASSERT_TRUE( 1 == shown.size() && shown[ 0 ].matchId == m.matchId && shown[ 0 ].title == "Friday game" );

            // not my turn: nothing
            MULTIPLAY::CMatchInfo other = f.NewMatch( "Other", 1 );
            f.book.RefreshAll();
            ASSERT_TRUE( 1 == f.desktop.delivered.size() );
            // the match alert survives a data node round trip
            MULTIPLAY::CMatchAlert back;
            ASSERT_TRUE( MatchJsonRoundTrip( n, back, "notification" ) && back.matchId == n.matchId && back.title == n.title && back.kind == n.kind && back.revision == 1 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A muted match notifies nobody, muting takes back what was shown, unmuting lets the next turn through" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SRouterFixture f;
            REQUIRE_TRUE( f.Setup() );
            MULTIPLAY::CMatchInfo m1 = f.NewMatch( "M1", 1 );
            MULTIPLAY::CMatchInfo m2 = f.NewMatch( "M2", 1 );
            f.book.RefreshAll();
            REQUIRE_TRUE( f.book.SetMuted( m2.matchId, true ) );
            f.Play( m1, 1, 0 );
            f.Play( m2, 1, 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 1 == f.desktop.delivered.size() && m1.matchId == f.desktop.delivered[ 0 ].matchId );
            ASSERT_TRUE( 1 == f.inApp.GetPendingCount() );
            // the book still counts it as waiting: the list is the truth, not the match alert
            ASSERT_TRUE( 2 == f.book.GetYourTurnCount() );

            // muting a match that was notified takes the match alert back
            ASSERT_TRUE( f.book.SetMuted( m1.matchId, true ) );
            ASSERT_TRUE( f.desktop.CountClearedFor( m1.matchId ) >= 1 && 0 == f.inApp.GetPendingCount() );

            // unmuted: the next turn goes through
            ASSERT_TRUE( f.book.SetMuted( m2.matchId, false ) );
            f.Play( m2, 1, 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 2 == f.desktop.delivered.size() && m2.matchId == f.desktop.delivered[ 1 ].matchId && 3 == f.desktop.delivered[ 1 ].revision );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A match and revision is notified once, however often the book reports it" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SRouterFixture f;
            REQUIRE_TRUE( f.Setup() );
            MULTIPLAY::CMatchInfo m = f.NewMatch( "M", 0 );
            f.book.RefreshAll();
            REQUIRE_TRUE( 1 == f.desktop.delivered.size() );
            f.book.RaiseYourTurn( m.matchId, 1 );
            f.book.RaiseYourTurn( m.matchId, 1 );
            f.book.RaiseYourTurn( m.matchId, 0 );
            ASSERT_TRUE( 1 == f.desktop.delivered.size() && 1 == f.inApp.GetPendingCount() );
            f.book.RaiseYourTurn( m.matchId, 2 );
            f.book.RaiseYourTurn( m.matchId, 2 );
            ASSERT_TRUE( 2 == f.desktop.delivered.size() && 2 == f.desktop.delivered[ 1 ].revision );
            f.book.RaiseYourTurn( m.matchId, 1 );
            ASSERT_TRUE( 2 == f.desktop.delivered.size() );
            // a match the book does not know is nothing to tell
            f.book.RaiseYourTurn( MULTIPLAY::CMatchId::Generate(), 1 );
            ASSERT_TRUE( 2 == f.desktop.delivered.size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Quiet hours hold what interrupts, the application is told at once, and what waited comes when they end" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SRouterFixture f;
            REQUIRE_TRUE( f.Setup() );
            f.router.SetQuietHours( true, 22 * 60, 8 * 60 );                 // across midnight
            MULTIPLAY::CMatchInfo m1 = f.NewMatch( "M1", 1 );
            MULTIPLAY::CMatchInfo m2 = f.NewMatch( "M2", 1 );
            f.book.RefreshAll();

            f.clock.SetMinuteOfDay( 23 * 60 );
            f.Play( m1, 1, 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 0 == f.desktop.delivered.size() && 1 == f.router.GetHeldCount() );
            ASSERT_TRUE( 1 == f.inApp.GetPendingCount() );
            f.router.Update();
            ASSERT_TRUE( 0 == f.desktop.delivered.size() );

            // after midnight it is still quiet, a second match joins the first
            f.clock.SetMinuteOfDay( 3 * 60 );
            f.Play( m2, 1, 0 );
            f.book.RefreshAll();
            f.router.Update();
            ASSERT_TRUE( 0 == f.desktop.delivered.size() && 2 == f.router.GetHeldCount() );

            // 07:59 is quiet, 08:00 is not: one match alert for both
            f.clock.SetMinuteOfDay( 7 * 60 + 59 );
            f.router.Update();
            ASSERT_TRUE( 0 == f.desktop.delivered.size() );
            f.clock.SetMinuteOfDay( 8 * 60 );
            f.router.Update();
            ASSERT_TRUE( 1 == f.desktop.delivered.size() && MULTIPLAY::MATCHALERTKIND_COALESCED == f.desktop.delivered[ 0 ].kind && 2 == f.desktop.delivered[ 0 ].count );
            ASSERT_TRUE( 0 == f.router.GetHeldCount() );
            f.router.Update();
            ASSERT_TRUE( 1 == f.desktop.delivered.size() );

            // a window within one day, and its edges
            f.router.SetQuietHours( true, 13 * 60, 15 * 60 );
            f.clock.SetMinuteOfDay( 12 * 60 + 59 );
            f.Play( m1, 1, 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 2 == f.desktop.delivered.size() );
            f.clock.SetMinuteOfDay( 13 * 60 );
            f.Play( m1, 1, 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 2 == f.desktop.delivered.size() && 1 == f.router.GetHeldCount() );
            f.clock.SetMinuteOfDay( 15 * 60 - 1 );
            f.router.Update();
            ASSERT_TRUE( 2 == f.desktop.delivered.size() );
            f.clock.SetMinuteOfDay( 15 * 60 );
            f.router.Update();
            ASSERT_TRUE( 3 == f.desktop.delivered.size() && MULTIPLAY::MATCHALERTKIND_YOUR_TURN == f.desktop.delivered[ 2 ].kind );

            // no quiet time when start equals end, or when it is off
            f.router.SetQuietHours( true, 600, 600 );
            f.Play( m1, 1, 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 4 == f.desktop.delivered.size() );
            f.router.SetQuietHours( false, 0, 1439 );
            f.Play( m1, 1, 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 5 == f.desktop.delivered.size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Games that wait together are told as one, and there is a minimum interval between two notifications" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            {
                SRouterFixture f;
                REQUIRE_TRUE( f.Setup() );
                f.router.SetCoalesceWindowMs( 300000 );
                MULTIPLAY::CMatchInfo m1 = f.NewMatch( "M1", 0 );
                f.book.RefreshAll();
                f.clock.AdvanceMs( 60000 );
                MULTIPLAY::CMatchInfo m2 = f.NewMatch( "M2", 0 );
                f.book.RefreshAll();
                f.clock.AdvanceMs( 60000 );
                MULTIPLAY::CMatchInfo m3 = f.NewMatch( "M3", 0 );
                f.book.RefreshAll();
                ASSERT_TRUE( 0 == f.desktop.delivered.size() && 3 == f.router.GetHeldCount() );
                ASSERT_TRUE( 3 == f.inApp.GetPendingCount() );          // the application shows each of them at once
                f.clock.AdvanceMs( 179000 );
                f.router.Update();
                ASSERT_TRUE( 0 == f.desktop.delivered.size() );
                f.clock.AdvanceMs( 1000 );
                f.router.Update();
                ASSERT_TRUE( 1 == f.desktop.delivered.size() && MULTIPLAY::MATCHALERTKIND_COALESCED == f.desktop.delivered[ 0 ].kind && 3 == f.desktop.delivered[ 0 ].count );
                ASSERT_TRUE( f.desktop.delivered[ 0 ].matchId.IsNULLOrEmpty() && f.desktop.delivered[ 0 ].title.IsNULLOrEmpty() );
                ASSERT_TRUE( 1 == f.router.GetInterruptingDeliveryCount() );

                // a single waiting game after the window is a plain match alert
                f.Play( m1, 1, 0 );
                f.book.RefreshAll();
                f.clock.AdvanceMs( 300000 );
                f.router.Update();
                ASSERT_TRUE( 2 == f.desktop.delivered.size() && MULTIPLAY::MATCHALERTKIND_YOUR_TURN == f.desktop.delivered[ 1 ].kind && m1.matchId == f.desktop.delivered[ 1 ].matchId );
            }
            {
                SRouterFixture f;
                REQUIRE_TRUE( f.Setup() );
                f.router.SetMinimumIntervalMs( 600000 );
                MULTIPLAY::CMatchInfo m1 = f.NewMatch( "M1", 0 );
                f.book.RefreshAll();
                ASSERT_TRUE( 1 == f.desktop.delivered.size() );                     // the first one is not kept waiting
                f.clock.AdvanceMs( 60000 );
                MULTIPLAY::CMatchInfo m2 = f.NewMatch( "M2", 0 );
                f.book.RefreshAll();
                f.clock.AdvanceMs( 60000 );
                MULTIPLAY::CMatchInfo m3 = f.NewMatch( "M3", 0 );
                f.book.RefreshAll();
                ASSERT_TRUE( 1 == f.desktop.delivered.size() && 2 == f.router.GetHeldCount() && 3 == f.inApp.GetPendingCount() );
                f.clock.AdvanceMs( 400000 );
                f.router.Update();
                ASSERT_TRUE( 1 == f.desktop.delivered.size() );
                f.clock.AdvanceMs( 140000 );                                          // 600 s after the first
                f.router.Update();
                ASSERT_TRUE( 2 == f.desktop.delivered.size() && MULTIPLAY::MATCHALERTKIND_COALESCED == f.desktop.delivered[ 1 ].kind && 2 == f.desktop.delivered[ 1 ].count );
                MULTIPLAY::CMatchInfo m4 = f.NewMatch( "M4", 0 );
                f.book.RefreshAll();
                ASSERT_TRUE( 2 == f.desktop.delivered.size() && 1 == f.router.GetHeldCount() );
                f.clock.AdvanceMs( 600000 );
                f.router.Update();
                ASSERT_TRUE( 3 == f.desktop.delivered.size() && m4.matchId == f.desktop.delivered[ 2 ].matchId );
                ASSERT_TRUE( 3 == f.router.GetInterruptingDeliveryCount() );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: In the mode once until I play a match notifies once, and again after the player has played in it" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SRouterFixture f;
            REQUIRE_TRUE( f.Setup() );
            f.router.SetMode( MULTIPLAY::MATCHALERTMODE_ONCE_UNTIL_I_PLAY );
            MULTIPLAY::CMatchInfo m = f.NewMatch( "M", 0 );
            MULTIPLAY::CMatchInfo other = f.NewMatch( "Other", 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( MULTIPLAY::MATCHALERTMODE_ONCE_UNTIL_I_PLAY == f.router.GetMode() );
            ASSERT_TRUE( 2 == f.desktop.delivered.size() );

            // another turn arrives before I played: the application hears of it, the desktop does not
            f.Play( m, 2, 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 2 == f.desktop.delivered.size() && 2 == f.inApp.GetPendingCount() );
            ASSERT_TRUE( 0 == f.desktop.CountOfKind( MULTIPLAY::MATCHALERTKIND_COALESCED ) );

            // I play it, and when it is mine again I am told again
            REQUIRE_TRUE( f.PlayMyTurn( m, 1 ) );
            ASSERT_TRUE( 1 == f.book.GetYourTurnCount() );
            f.Play( m, 1, 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 3 == f.desktop.delivered.size() && m.matchId == f.desktop.delivered[ 2 ].matchId && 4 == f.desktop.delivered[ 2 ].revision );
            // the other one was never played: no more match alerts for it
            f.Play( other, 1, 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 3 == f.desktop.delivered.size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: In the daily digest mode nothing interrupts until the digest time, and then one summary is sent" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SRouterFixture f;
            REQUIRE_TRUE( f.Setup() );
            f.router.SetMode( MULTIPLAY::MATCHALERTMODE_DAILY_DIGEST );
            f.router.SetDigestMinute( 9 * 60 );
            f.clock.SetMinuteOfDay( 8 * 60 );
            f.router.Update();                                                      // armed: the day has not reached 09:00 yet

            MULTIPLAY::CMatchInfo m1 = f.NewMatch( "M1", 0 );
            MULTIPLAY::CMatchInfo m2 = f.NewMatch( "M2", 0 );
            MULTIPLAY::CMatchInfo m3 = f.NewMatch( "M3", 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 0 == f.desktop.delivered.size() && 3 == f.inApp.GetPendingCount() );
            f.clock.SetMinuteOfDay( 8 * 60 + 59 );
            f.router.Update();
            ASSERT_TRUE( 0 == f.desktop.delivered.size() );
            f.clock.SetMinuteOfDay( 9 * 60 );
            f.router.Update();
            ASSERT_TRUE( 1 == f.desktop.delivered.size() && MULTIPLAY::MATCHALERTKIND_DIGEST == f.desktop.delivered[ 0 ].kind && 3 == f.desktop.delivered[ 0 ].count );
            f.clock.SetMinuteOfDay( 12 * 60 );
            f.router.Update();
            ASSERT_TRUE( 1 == f.desktop.delivered.size() );

            // tomorrow again, with a muted match and a played one not counted
            ASSERT_TRUE( f.book.SetMuted( m3.matchId, true ) );
            f.clock.SetMinuteOfDay( 2 * 60 );
            f.router.Update();
            f.clock.SetMinuteOfDay( 9 * 60 + 5 );
            f.router.Update();
            ASSERT_TRUE( 2 == f.desktop.delivered.size() && 2 == f.desktop.delivered[ 1 ].count );
            REQUIRE_TRUE( f.PlayMyTurn( m1, 1 ) );
            REQUIRE_TRUE( f.PlayMyTurn( m2, 1 ) );
            f.clock.SetMinuteOfDay( 1 * 60 );
            f.router.Update();
            f.clock.SetMinuteOfDay( 9 * 60 );
            f.router.Update();
            ASSERT_TRUE( 2 == f.desktop.delivered.size() );                         // nothing waits that counts: no summary

            // a digest in quiet hours waits for the end of them
            f.router.SetQuietHours( true, 8 * 60 + 30, 9 * 60 + 30 );
            f.Play( m1, 1, 0 );
            f.book.RefreshAll();
            f.clock.SetMinuteOfDay( 2 * 60 );
            f.router.Update();
            f.clock.SetMinuteOfDay( 9 * 60 );
            f.router.Update();
            ASSERT_TRUE( 2 == f.desktop.delivered.size() );
            f.clock.SetMinuteOfDay( 9 * 60 + 30 );
            f.router.Update();
            ASSERT_TRUE( 3 == f.desktop.delivered.size() && 1 == f.desktop.delivered[ 2 ].count );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A digest is not sent for a day that is already past its time when the mode is chosen" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SRouterFixture f;
            REQUIRE_TRUE( f.Setup() );
            f.clock.SetMinuteOfDay( 15 * 60 );
            f.router.SetMode( MULTIPLAY::MATCHALERTMODE_DAILY_DIGEST );
            f.router.SetDigestMinute( 9 * 60 );
            MULTIPLAY::CMatchInfo m1 = f.NewMatch( "M1", 0 );
            f.book.RefreshAll();
            f.router.Update();
            f.clock.SetMinuteOfDay( 20 * 60 );
            f.router.Update();
            ASSERT_TRUE( 0 == f.desktop.delivered.size() );
            f.clock.SetMinuteOfDay( 5 * 60 );
            f.router.Update();
            f.clock.SetMinuteOfDay( 9 * 60 );
            f.router.Update();
            ASSERT_TRUE( 1 == f.desktop.delivered.size() && MULTIPLAY::MATCHALERTKIND_DIGEST == f.desktop.delivered[ 0 ].kind );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A clock that is set back does not hold notifications for longer than the windows" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SRouterFixture f;
            REQUIRE_TRUE( f.Setup() );
            f.router.SetCoalesceWindowMs( 300000 );
            f.router.SetMinimumIntervalMs( 300000 );
            MULTIPLAY::CMatchInfo m1 = f.NewMatch( "M1", 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 0 == f.desktop.delivered.size() && 1 == f.router.GetHeldCount() );
            f.clock.nowMs -= 3600000;                                            // an hour back
            f.router.Update();
            ASSERT_TRUE( 0 == f.desktop.delivered.size() );
            f.clock.AdvanceMs( 299000 );
            f.router.Update();
            ASSERT_TRUE( 0 == f.desktop.delivered.size() );
            f.clock.AdvanceMs( 1000 );
            f.router.Update();
            ASSERT_TRUE( 1 == f.desktop.delivered.size() );
            // and the minimum interval after a delivery
            MULTIPLAY::CMatchInfo m2 = f.NewMatch( "M2", 0 );
            f.book.RefreshAll();
            f.clock.nowMs -= 7200000;
            f.router.Update();
            f.clock.AdvanceMs( 300000 );
            f.router.Update();
            ASSERT_TRUE( 2 == f.desktop.delivered.size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Opening a match, playing it elsewhere or removing it takes its notifications back" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SRouterFixture f;
            REQUIRE_TRUE( f.Setup() );
            MULTIPLAY::CMatchInfo m1 = f.NewMatch( "M1", 0 );
            MULTIPLAY::CMatchInfo m2 = f.NewMatch( "M2", 0 );
            MULTIPLAY::CMatchInfo m3 = f.NewMatch( "M3", 0 );
            f.book.RefreshAll();
            REQUIRE_TRUE( 3 == f.desktop.delivered.size() && 3 == f.inApp.GetPendingCount() );

            // opened
            REQUIRE_TRUE( f.book.OpenMatch( m1.matchId ) );
            ASSERT_TRUE( f.desktop.CountClearedFor( m1.matchId ) >= 1 && 2 == f.inApp.GetPendingCount() );
            // played on another device
            f.Play( m2, 0, 1 );
            f.book.RefreshAll();
            ASSERT_TRUE( f.desktop.CountClearedFor( m2.matchId ) >= 1 && 1 == f.inApp.GetPendingCount() );
            // removed
            f.provider->InjectRemoval( m3.matchId );
            f.book.RefreshAll();
            ASSERT_TRUE( f.desktop.CountClearedFor( m3.matchId ) >= 1 && 0 == f.inApp.GetPendingCount() );
            // and with the match alert still held: it is never shown
            f.router.SetQuietHours( true, 0, 1439 );
            MULTIPLAY::CMatchInfo m4 = f.NewMatch( "M4", 1 );
            f.book.RefreshAll();
            f.Play( m4, 1, 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 1 == f.router.GetHeldCount() );
            REQUIRE_TRUE( f.book.OpenMatch( m4.matchId ) );
            ASSERT_TRUE( 0 == f.router.GetHeldCount() );
            f.router.SetQuietHours( false, 0, 0 );
            f.router.Update();
            ASSERT_TRUE( 3 == f.desktop.delivered.size() );
            // the router can clear a match by itself, for a sink that is given the same by other means
            f.router.ClearForMatch( m4.matchId );
            ASSERT_TRUE( f.desktop.CountClearedFor( m4.matchId ) >= 2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Only pinned matches can be the ones that notify, invitations carry no contents, the end of a match stays inside the application" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SRouterFixture f;
            REQUIRE_TRUE( f.Setup() );
            f.router.SetOnlyPinned( true );
            MULTIPLAY::CMatchInfo m1 = f.NewMatch( "M1", 1 );
            MULTIPLAY::CMatchInfo m2 = f.NewMatch( "M2", 1 );
            f.book.RefreshAll();
            ASSERT_TRUE( f.book.SetPinned( m2.matchId, true ) );
            f.Play( m1, 1, 0 );
            f.Play( m2, 1, 0 );
            f.book.RefreshAll();
            ASSERT_TRUE( 1 == f.desktop.delivered.size() && m2.matchId == f.desktop.delivered[ 0 ].matchId );
            ASSERT_TRUE( 1 == f.inApp.GetPendingCount() );
            f.router.SetOnlyPinned( false );

            // an invitation: told once, without the title and without a name
            MULTIPLAY::CMatchInfo invited = f.NewMatch( "Secret title", 1, MULTIPLAY::MATCHSTATUS_ACTIVE, true );
            f.book.RefreshAll();
            f.book.RefreshAll();
            ASSERT_TRUE( 1 == f.desktop.CountOfKind( MULTIPLAY::MATCHALERTKIND_INVITE ) );
            const MULTIPLAY::CMatchAlert* invite = GUCEF_NULL;
            for ( size_t i=0; i<f.desktop.delivered.size(); ++i )
                if ( MULTIPLAY::MATCHALERTKIND_INVITE == f.desktop.delivered[ i ].kind )
                    invite = &f.desktop.delivered[ i ];
            REQUIRE_TRUE( GUCEF_NULL != invite );
            ASSERT_TRUE( invite->title.IsNULLOrEmpty() && invite->previousMover.IsNULLOrEmpty() && invited.matchId == invite->matchId );

            // the end of a match
            f.Play( m1, 0, -1, MULTIPLAY::MATCHSTATUS_FINISHED );
            f.book.RefreshAll();
            ASSERT_TRUE( 0 == f.desktop.CountOfKind( MULTIPLAY::MATCHALERTKIND_FINISHED ) );
            TESTFW::CRecordingMatchAlertSink probe( false );
            f.router.AddSink( &probe );
            f.Play( m2, 0, -1, MULTIPLAY::MATCHSTATUS_FINISHED );
            f.book.RefreshAll();
            ASSERT_TRUE( 1 == probe.CountOfKind( MULTIPLAY::MATCHALERTKIND_FINISHED ) && 0 == f.desktop.CountOfKind( MULTIPLAY::MATCHALERTKIND_FINISHED ) );
            f.router.RemoveSink( &probe );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The in-app sink keeps one notification per match, raises events for the interface and is bounded" )
        try
        {
            MULTIPLAY::CInAppMatchAlertSink sink;
            CInAppRecorder recorder;
            recorder.SubscribeTo( &sink );
            ASSERT_FALSE( sink.IsInterrupting() );
            ASSERT_TRUE( "in-app" == sink.GetSinkName() );

            MULTIPLAY::CMatchAlert a;
            a.matchId = MULTIPLAY::CMatchId::Generate();
            a.title = "A";
            a.revision = 1;
            sink.Deliver( a );
            ASSERT_TRUE( 1 == sink.GetPendingCount() && 1 == recorder.added && 1 == recorder.lastCount && 1 == recorder.countChanges );
            a.revision = 2;
            sink.Deliver( a );
            MULTIPLAY::CMatchAlert b = a;
            b.matchId = MULTIPLAY::CMatchId::Generate();
            b.title = "B";
            sink.Deliver( b );
            MULTIPLAY::TMatchAlertVector pending;
            sink.GetPending( pending );
            ASSERT_TRUE( 2 == pending.size() && 2 == recorder.lastCount && 3 == recorder.added );
            ASSERT_TRUE( 2 == pending[ 0 ].revision && "B" == pending[ 1 ].title );

            sink.Clear( a.matchId );
            ASSERT_TRUE( 1 == sink.GetPendingCount() && 1 == recorder.cleared && 1 == recorder.lastCount );
            sink.Clear( a.matchId );
            ASSERT_TRUE( 1 == recorder.cleared );
            sink.ClearAll();
            ASSERT_TRUE( 0 == sink.GetPendingCount() && 2 == recorder.cleared && 0 == recorder.lastCount );

            for ( int i=0; i<300; ++i )
            {
                MULTIPLAY::CMatchAlert n;
                n.matchId = MULTIPLAY::CMatchId::Generate();
                sink.Deliver( n );
            }
            ASSERT_TRUE( MULTIPLAY::CInAppMatchAlertSink::MaxPending == sink.GetPendingCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The router settings are saved and loaded" )
        try
        {
            MULTIPLAY::CMatchAlertRouter router;
            CORE::CDataNode config;
            config.SetAttribute( "mode", CORE::CString( "daily_digest" ) );
            config.SetAttribute( "quietHoursEnabled", true );
            config.SetAttribute( "quietStartMinute", (Int32) 1320 );
            config.SetAttribute( "quietEndMinute", (Int32) 420 );
            config.SetAttribute( "minIntervalMs", (UInt32) 90000 );
            config.SetAttribute( "coalesceWindowMs", (UInt32) 30000 );
            config.SetAttribute( "digestMinute", (Int32) 600 );
            config.SetAttribute( "onlyPinned", true );
            ASSERT_TRUE( router.LoadConfig( config ) );
            ASSERT_TRUE( MULTIPLAY::MATCHALERTMODE_DAILY_DIGEST == router.GetMode() );
            CORE::CDataNode saved;
            ASSERT_TRUE( router.SaveConfig( saved ) );
            ASSERT_TRUE( "DAILY_DIGEST" == saved.GetAttributeValueOrChildValueByName( "mode" ).AsString() );
            ASSERT_TRUE( saved.GetAttributeValueOrChildValueByName( "quietHoursEnabled" ).AsBool( false ) && 1320 == saved.GetAttributeValueOrChildValueByName( "quietStartMinute" ).AsInt32() );
            ASSERT_TRUE( 420 == saved.GetAttributeValueOrChildValueByName( "quietEndMinute" ).AsInt32() && 90000 == saved.GetAttributeValueOrChildValueByName( "minIntervalMs" ).AsUInt32() );
            ASSERT_TRUE( 30000 == saved.GetAttributeValueOrChildValueByName( "coalesceWindowMs" ).AsUInt32() && 600 == saved.GetAttributeValueOrChildValueByName( "digestMinute" ).AsInt32() );
            ASSERT_TRUE( saved.GetAttributeValueOrChildValueByName( "onlyPinned" ).AsBool( false ) );
            CORE::CDataNode empty;
            ASSERT_TRUE( router.LoadConfig( empty ) && MULTIPLAY::MATCHALERTMODE_EVERY_TURN == router.GetMode() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A router outlives its book and a book outlives its router" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            MULTIPLAY::CMatchAlertRouter router;
            TESTFW::CRecordingMatchAlertSink sink( true );
            router.AddSink( &sink );
            {
                CTestBook book;
                router.SetBook( &book );
                book.RaiseYourTurn( MULTIPLAY::CMatchId::Generate(), 1 );
            }
            router.Update();
            ASSERT_TRUE( 0 == sink.delivered.size() );
            CTestBook longLived;
            {
                MULTIPLAY::CMatchAlertRouter shortLived;
                shortLived.SetBook( &longLived );
            }
            longLived.RaiseYourTurn( MULTIPLAY::CMatchId::Generate(), 1 );
            ASSERT_TRUE( 0 == sink.delivered.size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A link names a match in the custom scheme or in https form and nothing else is accepted" )
        try
        {
            MULTIPLAY::CMatchLink link( "mygame", "play.example.invalid" );
            link.AddHttpsHost( "Alt.Example.Invalid" );
            const CORE::CString id = "3f2504e0-4f89-41d3-9a0c-0305e82c3301";

            ASSERT_TRUE( "mygame://match/" + id == link.MakeCustomLink( id ) );
            ASSERT_TRUE( "https://play.example.invalid/match/" + id == link.MakeHttpsLink( id ) );
            ASSERT_TRUE( link.MakeCustomLink( "nope" ).IsNULLOrEmpty() && link.MakeHttpsLink( "../x" ).IsNULLOrEmpty() );
            CORE::CString got;
            ASSERT_TRUE( link.Parse( link.MakeCustomLink( id ), got ) && id == got );
            ASSERT_TRUE( link.Parse( link.MakeHttpsLink( id ), got ) && id == got );

            // forms that are accepted
            const char* good[] = {
                "mygame://match/3f2504e0-4f89-41d3-9a0c-0305e82c3301",
                "MYGAME://MATCH/3F2504E0-4F89-41D3-9A0C-0305E82C3301",
                "mygame://match/3f2504e0-4f89-41d3-9a0c-0305e82c3301/",
                "mygame://match/3f2504e0-4f89-41d3-9a0c-0305e82c3301?from=mail#top",
                "  mygame://match/3f2504e0-4f89-41d3-9a0c-0305e82c3301  ",
                "https://play.example.invalid/match/3f2504e0-4f89-41d3-9a0c-0305e82c3301",
                "HTTPS://Play.Example.Invalid/match/3f2504e0-4f89-41d3-9a0c-0305e82c3301/",
                "https://alt.example.invalid/match/3f2504e0-4f89-41d3-9a0c-0305e82c3301?utm=x"
            };
            for ( size_t i=0; i<sizeof( good ) / sizeof( good[ 0 ] ); ++i )
                ASSERT_TRUE( link.Parse( good[ i ], got ) && id == got );

            // forms that are not
            const char* bad[] = {
                "", "mygame://match/", "mygame://match", "mygame://match/nope", "mygame://other/3f2504e0-4f89-41d3-9a0c-0305e82c3301",
                "other://match/3f2504e0-4f89-41d3-9a0c-0305e82c3301", "http://play.example.invalid/match/3f2504e0-4f89-41d3-9a0c-0305e82c3301",
                "https://evil.example.invalid/match/3f2504e0-4f89-41d3-9a0c-0305e82c3301", "https://play.example.invalid/other/3f2504e0-4f89-41d3-9a0c-0305e82c3301",
                "https://play.example.invalid/match/3f2504e0-4f89-41d3-9a0c-0305e82c330", "https://play.example.invalid:8443/match/3f2504e0-4f89-41d3-9a0c-0305e82c3301",
                "https://user@play.example.invalid/match/3f2504e0-4f89-41d3-9a0c-0305e82c3301", "https://play.example.invalid.evil.invalid/match/3f2504e0-4f89-41d3-9a0c-0305e82c3301",
                "https://evil.invalid/https://play.example.invalid/match/3f2504e0-4f89-41d3-9a0c-0305e82c3301",
                "mygame://match/3f2504e0-4f89-41d3-9a0c-0305e82c3301/extra", "mygame://match/%33f2504e0-4f89-41d3-9a0c-0305e82c3301",
                "mygame://match/3f2504e0-4f89-41d3-9a0c-0305e82c3301\\..", "mygame://match/3f2504e0-4f89-41d3-9a0c-0305e82c3301 junk",
                "mygame://match/../3f2504e0-4f89-41d3-9a0c-0305e82c3301", "mygame:match/3f2504e0-4f89-41d3-9a0c-0305e82c3301", "//match/3f2504e0-4f89-41d3-9a0c-0305e82c3301",
                "mygame://match/3f2504e0-4f89-41d3-9a0c-0305e82c3301\n--open --evil", "mygame://match/g f2504e0-4f89-41d3-9a0c-0305e82c3301"
            };
            for ( size_t i=0; i<sizeof( bad ) / sizeof( bad[ 0 ] ); ++i )
            {
                ASSERT_FALSE( link.Parse( bad[ i ], got ) );
                ASSERT_TRUE( got.IsNULLOrEmpty() );
            }

            // too long, and a game without a custom scheme
            CORE::CString tooLong = "mygame://match/" + id + "?";
            for ( int i=0; i<60; ++i ) tooLong += "0123456789";
            ASSERT_FALSE( link.Parse( tooLong, got ) );
            MULTIPLAY::CMatchLink webOnly( "", "play.example.invalid" );
            ASSERT_TRUE( webOnly.MakeCustomLink( id ).IsNULLOrEmpty() && !webOnly.Parse( "mygame://match/" + id, got ) && webOnly.Parse( "https://play.example.invalid/match/" + id, got ) );
            MULTIPLAY::CMatchLink customHost( "mygame", "" );
            customHost.SetCustomHost( "games" );
            ASSERT_TRUE( "games" == customHost.GetCustomHost() && customHost.Parse( "mygame://games/" + id, got ) && !customHost.Parse( "mygame://match/" + id, got ) );
            ASSERT_TRUE( customHost.MakeHttpsLink( id ).IsNULLOrEmpty() && !customHost.Parse( "https://play.example.invalid/match/" + id, got ) );
            MULTIPLAY::CMatchLink invalidScheme( "9bad scheme", "x" );
            ASSERT_TRUE( invalidScheme.GetCustomScheme().IsNULLOrEmpty() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    MULTIPLAY::CMultiplayGlobal::Instance()->GetMatchProviderFactory().UnregisterConcreteFactory( "fake" );
}
