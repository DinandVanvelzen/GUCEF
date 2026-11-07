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
#include "TestMatchBook.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A provider that implements only what the interface requires */
class CMinimalMatchProvider : public MULTIPLAY::CIMatchProvider
{
    public:

    CMinimalMatchProvider( void ) : MULTIPLAY::CIMatchProvider(), m_started( false ) {}

    virtual bool Start( void ) GUCEF_VIRTUAL_OVERRIDE { m_started = true; return true; }
    virtual void Stop( void ) GUCEF_VIRTUAL_OVERRIDE { m_started = false; }
    virtual bool IsStarted( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_started; }
    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE { return MULTIPLAY::MATCHCAPABILITY_PREVIEW_POLL; }
    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE { static const CORE::CString name = "minimal"; return name; }
    virtual bool RefreshAll( void ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool Refresh( const CORE::CString& matchId ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool CreateMatch( const MULTIPLAY::CMatchRecord& record, const MULTIPLAY::CMatchPackage& firstPackage, const MULTIPLAY::CMatchPreview& firstPreview, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool Fetch( const CORE::CString& matchId, UInt32 revision, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool Submit( const CORE::CString& matchId, UInt32 expectedRevision, const MULTIPLAY::CMatchPreview& preview, const MULTIPLAY::CMatchPackage& package, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { static const CORE::CString name = "CMinimalMatchProvider"; return name; }

    private:

    bool m_started;
};

/*-------------------------------------------------------------------------*/

/** A match book of the local player (identity 0) with a fake provider, a manual clock and a recorder */
struct SBookFixture
{
    GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
    TESTFW::CManualMatchClock clock;
    MULTIPLAY::CMatchBook book;
    TESTFW::CMatchBookRecorder recorder;
    MULTIPLAY::CIMatchProviderPtr providerPtr;
    TESTFW::CFakeMatchProvider* provider;
    GUCEF::map< CORE::CString, GUCEF::vector< MULTIPLAY::CMatchPackage > > packages;
    GUCEF::map< CORE::CString, MULTIPLAY::CMatchInfo > infos;

    SBookFixture( void ) : ids(), clock(), book(), recorder(), providerPtr(), provider( GUCEF_NULL ), packages(), infos() {}

    /** @param storeName a directory for the records of the book, none for no store; sharedIds the identities of an earlier fixture */
    bool Setup( const char* storeName, const GUCEF::vector< MULTIPLAY::CPlayerIdentity >* sharedIds = GUCEF_NULL )
    {
        if ( GUCEF_NULL != sharedIds )
            ids = *sharedIds;
        else
        {
            const char* names[ 3 ] = { "Me", "Bob", "Carol" };
            for ( int i=0; i<3; ++i )
            {
                MULTIPLAY::CPlayerIdentity identity;
                if ( !TESTFW::MakeTestIdentity( names[ i ], identity ) )
                    return false;
                ids.push_back( identity );
            }
        }
        book.SetClock( &clock );
        book.SetLocalUserId( ids[ 0 ].userId );
        if ( GUCEF_NULL != storeName )
            book.SetStoreDirectory( CORE::CombinePath( MatchTestDirectory( storeName ), "records" ) );
        recorder.SubscribeTo( &book );
        providerPtr = MULTIPLAY::CMultiplayGlobal::Instance()->GetMatchProviderFactory().Create( "fake" );
        if ( providerPtr.IsNULL() )
            return false;
        provider = static_cast< TESTFW::CFakeMatchProvider* >( providerPtr.GetPointerAlways() );
        return book.AddProvider( providerPtr );
    }

    /** A match at the provider; holder is the index of the participant on turn, -1 for nobody */
    MULTIPLAY::CMatchInfo NewMatch( const char* title, int holder, MULTIPLAY::TMatchStatus status, Int64 deadline = 0, bool meInvited = false, Int64 lastActivity = 0 )
    {
        MULTIPLAY::CMatchInfo info = TESTFW::MakeTestMatchInfo( ids, title, true );
        info.status = status;
        if ( meInvited )
            info.participants[ 0 ].status = MULTIPLAY::MATCHPARTICIPANT_INVITED;
        MULTIPLAY::CMatchPackage p1 = TESTFW::MakeTestPackage( 1, GUCEF_NULL, title );
        MULTIPLAY::CMatchPreview preview = TESTFW::MakeTestPreview( info, ids[ 0 ], p1, holder >= 0 ? &ids[ holder ] : GUCEF_NULL, status, lastActivity );
        if ( 0 != deadline )
        {
            preview.deadline = deadline;
            preview.Sign( ids[ 0 ] );
        }
        provider->InjectMatch( info, preview, p1 );
        packages[ info.matchId ].push_back( p1 );
        infos[ info.matchId ] = info;
        return info;
    }

    /** The next revision of a match, played by the issuer, injected at the provider; returns the preview and package made */
    void Play( const MULTIPLAY::CMatchInfo& info, int issuer, int holder, MULTIPLAY::TMatchStatus status, MULTIPLAY::CMatchPreview* previewOut = GUCEF_NULL, MULTIPLAY::CMatchPackage* packageOut = GUCEF_NULL, Int64 lastActivity = 0 )
    {
        GUCEF::vector< MULTIPLAY::CMatchPackage >& list = packages[ info.matchId ];
        MULTIPLAY::CMatchPackage next = TESTFW::MakeTestPackage( list.back().revision + 1, &list.back(), ( info.title + CORE::UInt32ToString( list.back().revision + 1 ) ).C_String() );
        MULTIPLAY::CMatchPreview preview = TESTFW::MakeTestPreview( info, ids[ issuer ], next, holder >= 0 ? &ids[ holder ] : GUCEF_NULL, status, lastActivity );
        provider->InjectTurn( preview, next );
        list.push_back( next );
        if ( GUCEF_NULL != previewOut ) *previewOut = preview;
        if ( GUCEF_NULL != packageOut ) *packageOut = next;
    }

    /** A preview and package of the next revision as the local player would hand it over, not injected */
    void MakeMyTurn( const MULTIPLAY::CMatchInfo& info, int holder, MULTIPLAY::CMatchPreview& preview, MULTIPLAY::CMatchPackage& package )
    {
        GUCEF::vector< MULTIPLAY::CMatchPackage >& list = packages[ info.matchId ];
        package = TESTFW::MakeTestPackage( list.back().revision + 1, &list.back(), "my turn" );
        preview = TESTFW::MakeTestPreview( info, ids[ 0 ], package, &ids[ holder ], MULTIPLAY::MATCHSTATUS_ACTIVE, 0 );
    }
};

/*-------------------------------------------------------------------------*/

static CORE::CString::StringVector
ListFiles( const CORE::CString& dir )
{GUCEF_TRACE;

    CORE::CString::StringVector names;
    CORE::CFileSystemIterator iterator;
    if ( iterator.FindFirst( dir ) )
    {
        do { if ( iterator.IsAFile() ) names.push_back( iterator.GetResourceName() ); } while ( iterator.FindNext() );
        iterator.FindClose();
    }
    return names;
}

/*-------------------------------------------------------------------------*/

static CORE::CFutureResult
NoFuture( void )
{GUCEF_TRACE;

    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
}

/*-------------------------------------------------------------------------*/

static MULTIPLAY::CMatchUpdate
UpdateOf( const MULTIPLAY::CMatchInfo& info, const MULTIPLAY::CMatchPreview& preview )
{GUCEF_TRACE;

    MULTIPLAY::CMatchUpdate update;
    update.preview = preview;
    update.info = info;
    update.hasInfo = true;
    return update;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMatchBookTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefMULTIPLAY matches: match book" );

    TESTFW::TFakeMatchProviderFactory fakeFactory;
    MULTIPLAY::CMultiplayGlobal::Instance()->GetMatchProviderFactory().RegisterConcreteFactory( "fake", &fakeFactory );
    bool signaturesAvailable = MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable();

    GUCEF_TESTFW_TESTCASE( "Test: The book groups matches into your turn, their turn, forming or invited and finished, in order" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SBookFixture f;
            REQUIRE_TRUE( f.Setup( GUCEF_NULL ) );
            const Int64 base = f.clock.nowMs;
            MULTIPLAY::CMatchInfo m1 = f.NewMatch( "M1", 0, MULTIPLAY::MATCHSTATUS_ACTIVE );
            MULTIPLAY::CMatchInfo m2 = f.NewMatch( "M2", 1, MULTIPLAY::MATCHSTATUS_ACTIVE );
            MULTIPLAY::CMatchInfo m3 = f.NewMatch( "M3", -1, MULTIPLAY::MATCHSTATUS_FORMING );
            MULTIPLAY::CMatchInfo m4 = f.NewMatch( "M4", -1, MULTIPLAY::MATCHSTATUS_FINISHED );
            MULTIPLAY::CMatchInfo m5 = f.NewMatch( "M5", 0, MULTIPLAY::MATCHSTATUS_ACTIVE, base + 5000 );
            MULTIPLAY::CMatchInfo m6 = f.NewMatch( "M6", 1, MULTIPLAY::MATCHSTATUS_ACTIVE, 0, true );
            MULTIPLAY::CMatchInfo m7 = f.NewMatch( "M7", 0, MULTIPLAY::MATCHSTATUS_ABANDONED );
            MULTIPLAY::CMatchInfo m8 = f.NewMatch( "M8", 0, MULTIPLAY::MATCHSTATUS_ACTIVE, base + 9000 );

            ASSERT_TRUE( 0 == f.book.GetEntryCount() );
            REQUIRE_TRUE( f.book.Start() );
            ASSERT_TRUE( f.provider->startCount >= 1 && 1 == f.provider->refreshAllCount );
            ASSERT_TRUE( 8 == f.book.GetEntryCount() && 8 == f.recorder.added );

            MULTIPLAY::TMatchEntryVector yourTurn, theirTurn, forming, finished;
            f.book.GetGroup( MULTIPLAY::MATCHGROUP_YOUR_TURN, yourTurn );
            f.book.GetGroup( MULTIPLAY::MATCHGROUP_THEIR_TURN, theirTurn );
            f.book.GetGroup( MULTIPLAY::MATCHGROUP_FORMING_OR_INVITED, forming );
            f.book.GetGroup( MULTIPLAY::MATCHGROUP_FINISHED, finished );
            ASSERT_TRUE( 3 == yourTurn.size() && 1 == theirTurn.size() && 2 == forming.size() && 2 == finished.size() );
            ASSERT_TRUE( 3 == f.book.GetYourTurnCount() );

            // by deadline, the nearest first, those without a deadline last
            REQUIRE_TRUE( 3 == yourTurn.size() );
            ASSERT_TRUE( "M5" == yourTurn[ 0 ].info.title && "M8" == yourTurn[ 1 ].info.title && "M1" == yourTurn[ 2 ].info.title );
            ASSERT_TRUE( "M2" == theirTurn[ 0 ].info.title );
            ASSERT_TRUE( f.book.GetGroupOf( yourTurn[ 0 ] ) == MULTIPLAY::MATCHGROUP_YOUR_TURN );

            // the invitation is told once, a match where I am invited is not my turn
            ASSERT_TRUE( 1 == f.recorder.invites.size() && m6.matchId == f.recorder.invites[ 0 ] );
            ASSERT_TRUE( 3 == f.recorder.yourTurn.size() );

            // pinned ones come first, archived ones are left out
            ASSERT_TRUE( f.book.SetPinned( m1.matchId, true ) );
            yourTurn.clear();
            f.book.GetGroup( MULTIPLAY::MATCHGROUP_YOUR_TURN, yourTurn );
            ASSERT_TRUE( "M1" == yourTurn[ 0 ].info.title );
            ASSERT_TRUE( f.book.SetArchived( m8.matchId, true ) );
            yourTurn.clear();
            f.book.GetGroup( MULTIPLAY::MATCHGROUP_YOUR_TURN, yourTurn );
            ASSERT_TRUE( 2 == yourTurn.size() && 2 == f.book.GetYourTurnCount() );
            yourTurn.clear();
            f.book.GetGroup( MULTIPLAY::MATCHGROUP_YOUR_TURN, yourTurn, true );
            ASSERT_TRUE( 3 == yourTurn.size() );
            ASSERT_TRUE( f.recorder.flags >= 2 );
            ASSERT_FALSE( f.book.SetMuted( "3f2504e0-4f89-41d3-9a0c-0305e82c3301", true ) );

            // whose turn it is depends on who the book is for
            f.book.SetLocalUserId( f.ids[ 2 ].userId );
            ASSERT_TRUE( 0 == f.book.GetYourTurnCount() );
            f.book.SetLocalUserId( f.ids[ 0 ].userId );
            ASSERT_TRUE( 2 == f.book.GetYourTurnCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A match whose record says it is still forming is in the forming group, with no your turn, although its first turn is the player's, until the record says it is active" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SBookFixture f;
            REQUIRE_TRUE( f.Setup( GUCEF_NULL ) );
            MULTIPLAY::CMatchInfo info = TESTFW::MakeTestMatchInfo( f.ids, "Forming", true );
            info.status = MULTIPLAY::MATCHSTATUS_FORMING;
            MULTIPLAY::CMatchPackage p1 = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "Forming" );
            MULTIPLAY::CMatchPreview preview = TESTFW::MakeTestPreview( info, f.ids[ 0 ], p1, &f.ids[ 0 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 0 );
            f.provider->InjectMatch( info, preview, p1 );
            REQUIRE_TRUE( f.book.Start() );
            MULTIPLAY::CMatchEntry entry;
            REQUIRE_TRUE( f.book.GetEntry( info.matchId, entry ) );
            ASSERT_TRUE( MULTIPLAY::MATCHGROUP_FORMING_OR_INVITED == f.book.GetGroupOf( entry ) && 0 == f.book.GetYourTurnCount() && 0 == f.recorder.yourTurn.size() );

            // everybody answered: the same turn is the player's now, told once
            MULTIPLAY::CMatchInfo active = info;
            active.status = MULTIPLAY::MATCHSTATUS_ACTIVE;
            f.provider->RaiseUpdated( UpdateOf( active, preview ) );
            REQUIRE_TRUE( f.book.GetEntry( info.matchId, entry ) );
            ASSERT_TRUE( MULTIPLAY::MATCHGROUP_YOUR_TURN == f.book.GetGroupOf( entry ) && 1 == f.book.GetYourTurnCount() && 1 == f.recorder.yourTurn.size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: YourTurnEvent is raised once per match and revision, however often it is reported" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SBookFixture f;
            REQUIRE_TRUE( f.Setup( GUCEF_NULL ) );
            f.provider->SetDuplicateEvents( 3 );
            MULTIPLAY::CMatchInfo m = f.NewMatch( "M", 0, MULTIPLAY::MATCHSTATUS_ACTIVE );
            REQUIRE_TRUE( f.book.Start() );
            ASSERT_TRUE( 1 == f.recorder.yourTurn.size() && 1 == f.recorder.CountYourTurn( m.matchId, 1 ) );
            ASSERT_TRUE( 1 == f.recorder.added );
            f.book.RefreshAll();
            f.book.RefreshAll();
            f.book.Refresh( m.matchId );
            ASSERT_TRUE( 1 == f.recorder.yourTurn.size() );

            // I hand over my turn, bob plays, and it is mine again: revision 3 raises it once
            MULTIPLAY::CMatchPreview myPreview;
            MULTIPLAY::CMatchPackage myPackage;
            f.MakeMyTurn( m, 1, myPreview, myPackage );
            CORE::CFutureResult future = NoFuture();
            REQUIRE_TRUE( f.book.Submit( m.matchId, myPreview, myPackage, future ) );
            MULTIPLAY::CMatchEntry entry;
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            ASSERT_TRUE( 2 == entry.preview.revision && 0 == entry.yourTurnSince && 0 == f.book.GetYourTurnCount() );
            f.packages[ m.matchId ].push_back( myPackage );
            f.Play( m, 1, 0, MULTIPLAY::MATCHSTATUS_ACTIVE );
            f.book.RefreshAll();
            f.book.RefreshAll();
            ASSERT_TRUE( 2 == f.recorder.yourTurn.size() && 1 == f.recorder.CountYourTurn( m.matchId, 3 ) );

            // a new revision while it is still my turn is a new event, and the turn has been mine since the first
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            Int64 since = entry.yourTurnSince;
            ASSERT_TRUE( 0 != since );
            f.clock.AdvanceMs( 60000 );
            f.Play( m, 2, 0, MULTIPLAY::MATCHSTATUS_ACTIVE );
            f.book.RefreshAll();
            ASSERT_TRUE( 3 == f.recorder.yourTurn.size() && 1 == f.recorder.CountYourTurn( m.matchId, 4 ) );
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            ASSERT_TRUE( since == entry.yourTurnSince && 4 == entry.notifiedRevision );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Reconcile merges only newer revisions: old, equal and conflicting previews and bad ones are ignored" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SBookFixture f;
            REQUIRE_TRUE( f.Setup( GUCEF_NULL ) );
            MULTIPLAY::CMatchInfo m = f.NewMatch( "M", 1, MULTIPLAY::MATCHSTATUS_ACTIVE );
            REQUIRE_TRUE( f.book.Start() );
            MULTIPLAY::CMatchEntry entry;
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            MULTIPLAY::CMatchPreview r1 = entry.preview;
            MULTIPLAY::CMatchPreview r2, r3;
            f.Play( m, 1, 0, MULTIPLAY::MATCHSTATUS_ACTIVE, &r2 );
            f.book.RefreshAll();
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) && 2 == entry.preview.revision );
            UInt32 changed = f.recorder.changed;

            // revision 1 again, late
            f.provider->RaiseUpdated( UpdateOf( m, r1 ) );
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            ASSERT_TRUE( 2 == entry.preview.revision && changed == f.recorder.changed );
            // revision 2 again
            f.provider->RaiseUpdated( UpdateOf( m, r2 ) );
            ASSERT_TRUE( changed == f.recorder.changed );
            // another signed revision 2 by the same signer: the first one accepted stays
            MULTIPLAY::CMatchPackage other = TESTFW::MakeTestPackage( 2, &f.packages[ m.matchId ][ 0 ], "equivocation" );
            MULTIPLAY::CMatchPreview conflicting = TESTFW::MakeTestPreview( m, f.ids[ 1 ], other, &f.ids[ 2 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 0 );
            f.provider->RaiseUpdated( UpdateOf( m, conflicting ) );
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            ASSERT_TRUE( r2.packageSha256 == entry.preview.packageSha256 && changed == f.recorder.changed );
            // a preview that is outside its limits, with no match id, or of an info of another match
            MULTIPLAY::CMatchPreview junk = r2;
            junk.revision = 9;
            junk.matchId = "nonsense";
            f.provider->RaiseUpdated( UpdateOf( m, junk ) );
            junk.matchId = m.matchId;
            junk.summary = CORE::CString( "x" );
            for ( int i=0; i<200; ++i ) junk.summary += "x";
            f.provider->RaiseUpdated( UpdateOf( m, junk ) );
            MULTIPLAY::CMatchInfo otherInfo = m;
            otherInfo.matchId = MULTIPLAY::CMatchId::Generate();
            junk = r2;
            junk.revision = 9;
            f.provider->RaiseUpdated( UpdateOf( otherInfo, junk ) );
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            ASSERT_TRUE( 2 == entry.preview.revision && 1 == f.book.GetEntryCount() && changed == f.recorder.changed );

            // a newer one with a cursor and the info with a changed participant
            f.Play( m, 0, 2, MULTIPLAY::MATCHSTATUS_ACTIVE, &r3 );
            MULTIPLAY::CMatchUpdate update = UpdateOf( m, r3 );
            update.cursor = "uid=77";
            update.info.participants[ 2 ].status = MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER;
            f.provider->RaiseUpdated( update );
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            ASSERT_TRUE( 3 == entry.preview.revision && "uid=77" == entry.cursor && MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER == entry.info.participants[ 2 ].status );
            ASSERT_TRUE( changed + 1 == f.recorder.changed );
            // the info changed without a newer preview is a change too
            update.info.participants[ 2 ].status = MULTIPLAY::MATCHPARTICIPANT_DROPPED;
            f.provider->RaiseUpdated( update );
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            ASSERT_TRUE( 3 == entry.preview.revision && MULTIPLAY::MATCHPARTICIPANT_DROPPED == entry.info.participants[ 2 ].status && changed + 2 == f.recorder.changed );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Only the provider that carries a match can remove it, and a finished match is told once" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SBookFixture f;
            REQUIRE_TRUE( f.Setup( "removal" ) );
            MULTIPLAY::CIMatchProviderPtr secondPtr = MULTIPLAY::CMultiplayGlobal::Instance()->GetMatchProviderFactory().Create( "fake" );
            REQUIRE_TRUE( !secondPtr.IsNULL() );
            CORE::CDataNode secondConfig;
            secondConfig.SetAttribute( "name", CORE::CString( "second" ) );
            secondPtr->LoadConfig( secondConfig );
            REQUIRE_TRUE( f.book.AddProvider( secondPtr ) );
            TESTFW::CFakeMatchProvider* second = static_cast< TESTFW::CFakeMatchProvider* >( secondPtr.GetPointerAlways() );
            ASSERT_TRUE( 2 == f.book.GetProviders().size() );
            ASSERT_FALSE( f.book.AddProvider( secondPtr ) );

            MULTIPLAY::CMatchInfo m = f.NewMatch( "M", 1, MULTIPLAY::MATCHSTATUS_ACTIVE );
            REQUIRE_TRUE( f.book.Start() );
            ASSERT_TRUE( second->IsStarted() && 1 == second->refreshAllCount );
            MULTIPLAY::CMatchEntry entry;
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            ASSERT_TRUE( "fake" == entry.providerName );
            CORE::CString path = f.book.GetStore().GetRecordPath( m.matchId );
            ASSERT_TRUE( CORE::FileExists( path ) );

            // the other provider says it is gone: it does not carry it
            second->RaiseRemoved( m.matchId );
            ASSERT_TRUE( 1 == f.book.GetEntryCount() && CORE::FileExists( path ) && 0 == f.recorder.removed );
            f.provider->InjectRemoval( m.matchId );
            f.book.RefreshAll();
            ASSERT_TRUE( 0 == f.book.GetEntryCount() && !CORE::FileExists( path ) && 1 == f.recorder.removed );
            ASSERT_FALSE( f.book.GetEntry( m.matchId, entry ) );

            // the end of a match is told when it is seen, not when it is first met
            MULTIPLAY::CMatchInfo m2 = f.NewMatch( "M2", 1, MULTIPLAY::MATCHSTATUS_ACTIVE );
            MULTIPLAY::CMatchInfo done = f.NewMatch( "Done", -1, MULTIPLAY::MATCHSTATUS_FINISHED );
            f.book.RefreshAll();
            ASSERT_TRUE( 0 == f.recorder.finished.size() );
            f.Play( m2, 1, -1, MULTIPLAY::MATCHSTATUS_FINISHED );
            f.book.RefreshAll();
            f.book.RefreshAll();
            ASSERT_TRUE( 1 == f.recorder.finished.size() && m2.matchId == f.recorder.finished[ 0 ] );
            MULTIPLAY::TMatchEntryVector finished;
            f.book.GetGroup( MULTIPLAY::MATCHGROUP_FINISHED, finished );
            ASSERT_TRUE( 2 == finished.size() );
            ASSERT_TRUE( f.book.RemoveMatch( done.matchId ) );
            finished.clear();
            f.book.GetGroup( MULTIPLAY::MATCHGROUP_FINISHED, finished );
            ASSERT_TRUE( 1 == finished.size() && m2.matchId == finished[ 0 ].preview.matchId );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Opening a match marks it seen, and unseen follows new revisions" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SBookFixture f;
            REQUIRE_TRUE( f.Setup( GUCEF_NULL ) );
            MULTIPLAY::CMatchInfo m = f.NewMatch( "M", 1, MULTIPLAY::MATCHSTATUS_ACTIVE );
            REQUIRE_TRUE( f.book.Start() );
            MULTIPLAY::CMatchEntry entry;
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            ASSERT_TRUE( entry.unseen && 0 == entry.lastSeenRevision );
            ASSERT_TRUE( f.book.OpenMatch( m.matchId ) && 1 == f.recorder.opened );
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            ASSERT_TRUE( !entry.unseen && 1 == entry.lastSeenRevision && m.matchId == f.book.GetOpenMatchId() );
            // a revision that arrives while the match is open is seen
            f.Play( m, 1, 2, MULTIPLAY::MATCHSTATUS_ACTIVE );
            f.book.RefreshAll();
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            ASSERT_TRUE( !entry.unseen && 2 == entry.lastSeenRevision );
            // after it is closed the next one is not
            f.book.CloseMatch();
            f.Play( m, 2, 1, MULTIPLAY::MATCHSTATUS_ACTIVE );
            f.book.RefreshAll();
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            ASSERT_TRUE( entry.unseen && 2 == entry.lastSeenRevision && 3 == entry.preview.revision );
            ASSERT_TRUE( f.book.MarkSeen( m.matchId ) );
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            ASSERT_TRUE( !entry.unseen && 3 == entry.lastSeenRevision );
            ASSERT_FALSE( f.book.OpenMatch( "3f2504e0-4f89-41d3-9a0c-0305e82c3301" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The next match is the one that has been ready for the player the longest" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            for ( int serverClock=0; serverClock<2; ++serverClock )
            {
                SBookFixture f;
                REQUIRE_TRUE( f.Setup( GUCEF_NULL ) );
                if ( serverClock )
                    f.provider->SetCapabilities( f.provider->GetCapabilities() | MULTIPLAY::MATCHCAPABILITY_SERVER_CLOCK );
                REQUIRE_TRUE( f.book.Start() );
                const Int64 base = f.clock.nowMs;

                // they arrive A, B, C one after the other; the issuer's clock says C, A, B
                MULTIPLAY::CMatchInfo a = f.NewMatch( "A", 0, MULTIPLAY::MATCHSTATUS_ACTIVE, 0, false, base - 1000 );
                f.book.RefreshAll();
                f.clock.AdvanceMs( 10000 );
                MULTIPLAY::CMatchInfo b = f.NewMatch( "B", 0, MULTIPLAY::MATCHSTATUS_ACTIVE, 0, false, base + 500 );
                f.book.RefreshAll();
                f.clock.AdvanceMs( 10000 );
                MULTIPLAY::CMatchInfo c = f.NewMatch( "C", 0, MULTIPLAY::MATCHSTATUS_ACTIVE, 0, false, base - 5000 );
                f.book.RefreshAll();

                CORE::CString next;
                const CORE::CString& first = serverClock ? c.matchId : a.matchId;
                const CORE::CString& second = serverClock ? a.matchId : b.matchId;
                const CORE::CString& third = serverClock ? b.matchId : c.matchId;
                ASSERT_TRUE( f.book.GetNextYourTurn( CORE::CString::Empty, next ) && first == next );
                ASSERT_TRUE( f.book.GetNextYourTurn( first, next ) && second == next );
                ASSERT_TRUE( f.book.GetNextYourTurn( second, next ) && first == next );
                MULTIPLAY::CMatchEntry entry;
                REQUIRE_TRUE( f.book.GetEntry( a.matchId, entry ) );
                ASSERT_TRUE( ( serverClock ? base - 1000 : base ) == entry.yourTurnSince );

                // having played the first, the next is the second; archived ones are not offered
                MULTIPLAY::CMatchInfo firstInfo = f.infos[ first ];
                MULTIPLAY::CMatchPreview preview;
                MULTIPLAY::CMatchPackage package;
                f.MakeMyTurn( firstInfo, 1, preview, package );
                CORE::CFutureResult future = NoFuture();
                REQUIRE_TRUE( f.book.Submit( first, preview, package, future ) );
                ASSERT_TRUE( f.book.GetNextYourTurn( first, next ) && second == next );
                ASSERT_TRUE( f.book.GetNextYourTurn( CORE::CString::Empty, next ) && second == next );
                ASSERT_TRUE( f.book.SetArchived( second, true ) );
                ASSERT_TRUE( f.book.GetNextYourTurn( CORE::CString::Empty, next ) && third == next );
                ASSERT_TRUE( f.book.SetArchived( third, true ) );
                ASSERT_FALSE( f.book.GetNextYourTurn( CORE::CString::Empty, next ) );
                ASSERT_TRUE( 0 == f.book.GetYourTurnCount() );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Entries are kept one file per match, and come back with the local state and without new events" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
            MULTIPLAY::CMatchInfo m1, m2;
            MULTIPLAY::CMatchPackage m1Package;
            CORE::CString dir;
            {
                SBookFixture f;
                REQUIRE_TRUE( f.Setup( "persist" ) );
                ids = f.ids;
                dir = f.book.GetStore().GetDirectory();
                m1 = f.NewMatch( "M1", 0, MULTIPLAY::MATCHSTATUS_ACTIVE );
                m2 = f.NewMatch( "M2", 1, MULTIPLAY::MATCHSTATUS_ACTIVE );
                REQUIRE_TRUE( f.book.Start() );
                m1Package = f.packages[ m1.matchId ][ 0 ];
                ASSERT_TRUE( f.book.SetMuted( m1.matchId, true ) && f.book.SetPinned( m1.matchId, true ) && f.book.SetArchived( m2.matchId, true ) );
                ASSERT_TRUE( f.book.OpenMatch( m2.matchId ) );
                MULTIPLAY::CMatchEntry entry;
                REQUIRE_TRUE( f.book.GetEntry( m1.matchId, entry ) );
                // one file per match, named by the match id, nothing else in the directory
                CORE::CString::StringVector names = ListFiles( dir );
                ASSERT_TRUE( 2 == names.size() );
                for ( size_t i=0; i<names.size(); ++i )
                    ASSERT_TRUE( names[ i ] == ( m1.matchId + ".json" ) || names[ i ] == ( m2.matchId + ".json" ) );
            }
            {
                SBookFixture g;
                g.Setup( GUCEF_NULL, &ids );
                g.book.SetStoreDirectory( dir );
                REQUIRE_TRUE( g.book.LoadFromStore() );
                ASSERT_TRUE( 2 == g.book.GetEntryCount() && 2 == g.recorder.added );
                ASSERT_TRUE( 0 == g.recorder.yourTurn.size() );
                MULTIPLAY::CMatchEntry e1, e2;
                REQUIRE_TRUE( g.book.GetEntry( m1.matchId, e1 ) && g.book.GetEntry( m2.matchId, e2 ) );
                ASSERT_TRUE( e1.muted && e1.pinned && !e1.archived && e2.archived && !e2.muted );
                ASSERT_TRUE( 1 == e1.notifiedRevision && 0 != e1.yourTurnSince && e1.hasInfo && e1.info.GetDigest() == m1.GetDigest() );
                ASSERT_TRUE( e1.preview.VerifySignature() && 1 == e2.lastSeenRevision && !e2.unseen && e1.unseen );
                ASSERT_TRUE( "fake" == e1.providerName );

                // the provider tells the same again: the revision that was notified before is not notified again
                g.provider->InjectMatch( m1, e1.preview, m1Package );
                g.book.Start();
                ASSERT_TRUE( 0 == g.recorder.yourTurn.size() );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A crash between the temporary file and the rename leaves the old record, and damaged files do not stop the others" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            SBookFixture f;
            REQUIRE_TRUE( f.Setup( "crash" ) );
            MULTIPLAY::CMatchInfo a = f.NewMatch( "A", 0, MULTIPLAY::MATCHSTATUS_ACTIVE );
            MULTIPLAY::CMatchInfo b = f.NewMatch( "B", 1, MULTIPLAY::MATCHSTATUS_ACTIVE );
            REQUIRE_TRUE( f.book.Start() );
            MULTIPLAY::CMatchStore& store = f.book.GetStore();
            const CORE::CString dir = store.GetDirectory();

            // a changed record is written to its temporary file; the process dies before the rename
            MULTIPLAY::CMatchEntry entry;
            REQUIRE_TRUE( f.book.GetEntry( a.matchId, entry ) );
            MULTIPLAY::CMatchEntry changed = entry;
            changed.muted = true;
            changed.lastSeenRevision = 77;
            REQUIRE_TRUE( store.WriteTemporary( changed ) );
            ASSERT_TRUE( CORE::FileExists( store.GetTemporaryPath( a.matchId ) ) && CORE::FileExists( store.GetRecordPath( a.matchId ) ) );
            MULTIPLAY::CMatchEntry loaded;
            REQUIRE_TRUE( store.Load( a.matchId, loaded ) );
            ASSERT_TRUE( !loaded.muted && 77 != loaded.lastSeenRevision );

            // a temporary file that is half written, for a match that has no record yet
            const CORE::CString newId = MULTIPLAY::CMatchId::Generate();
            CORE::CString halfTemp = CORE::CombinePath( dir, newId + ".json.tmp" );
            REQUIRE_TRUE( CORE::WriteStringAsTextFile( halfTemp, "{\"matchId\": \"" + newId + "\", \"previ", true ) );
            // a record that is not valid, one that is of another match than its name, and files that are no records
            CORE::CString corrupt = CORE::CombinePath( dir, MULTIPLAY::CMatchId::Generate() + ".json" );
            REQUIRE_TRUE( CORE::WriteStringAsTextFile( corrupt, "{ this is not json", true ) );
            CORE::CString misnamed = CORE::CombinePath( dir, MULTIPLAY::CMatchId::Generate() + ".json" );
            REQUIRE_TRUE( CORE::CopyFile( store.GetRecordPath( b.matchId ), misnamed, true ) );
            REQUIRE_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "notes.txt" ), "hello", true ) );
            REQUIRE_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "../escape.json" ), "{}", true ) || true );

            // the next start: the old records are there, the leftovers are cleaned up, the damaged files are counted and left alone
            MULTIPLAY::TMatchEntryVector entries;
            UInt32 skipped = 0;
            REQUIRE_TRUE( store.LoadAll( entries, &skipped ) );
            ASSERT_TRUE( 2 == entries.size() && 2 == skipped );
            ASSERT_FALSE( CORE::FileExists( store.GetTemporaryPath( a.matchId ) ) );
            ASSERT_FALSE( CORE::FileExists( halfTemp ) );
            ASSERT_TRUE( CORE::FileExists( corrupt ) && CORE::FileExists( misnamed ) );
            for ( size_t i=0; i<entries.size(); ++i )
                ASSERT_TRUE( !entries[ i ].muted && 77 != entries[ i ].lastSeenRevision );
            ASSERT_FALSE( store.Load( newId, loaded ) );

            // the rename completed: the new record
            REQUIRE_TRUE( store.WriteTemporary( changed ) );
            REQUIRE_TRUE( store.Commit( a.matchId ) );
            REQUIRE_TRUE( store.Load( a.matchId, loaded ) );
            ASSERT_TRUE( loaded.muted && 77 == loaded.lastSeenRevision );
            ASSERT_FALSE( CORE::FileExists( store.GetTemporaryPath( a.matchId ) ) );
            ASSERT_FALSE( store.Commit( a.matchId ) );

            // nothing from outside is a path: bad ids give no path
            ASSERT_TRUE( store.GetRecordPath( "../../etc/passwd" ).IsNULLOrEmpty() && store.GetRecordPath( "" ).IsNULLOrEmpty() );
            MULTIPLAY::CMatchEntry bad = changed;
            bad.preview.matchId = "../x";
            ASSERT_FALSE( store.Save( bad ) );
            ASSERT_TRUE( store.Remove( b.matchId ) && !CORE::FileExists( store.GetRecordPath( b.matchId ) ) );

            // without a directory or a codec nothing is saved and nothing breaks
            MULTIPLAY::CMatchStore nowhere;
            ASSERT_FALSE( nowhere.Save( changed ) );
            MULTIPLAY::CMatchStore noCodec;
            noCodec.SetDirectory( dir );
            noCodec.SetCodecName( "no-such-codec" );
            ASSERT_FALSE( noCodec.Save( changed ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Submit is conditional and idempotent: OK, the same again OK, STALE with the current preview, REJECTED" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SBookFixture f;
            REQUIRE_TRUE( f.Setup( GUCEF_NULL ) );
            MULTIPLAY::CMatchInfo m = f.NewMatch( "M", 0, MULTIPLAY::MATCHSTATUS_ACTIVE );
            REQUIRE_TRUE( f.book.Start() );
            MULTIPLAY::CMatchPreview myPreview;
            MULTIPLAY::CMatchPackage myPackage;
            f.MakeMyTurn( m, 1, myPreview, myPackage );

            CORE::CFutureResult future = NoFuture();
            REQUIRE_TRUE( f.book.Submit( m.matchId, myPreview, myPackage, future ) );
            MULTIPLAY::CMatchOperationResult result;
            REQUIRE_TRUE( MULTIPLAY::CMatchOperationResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( MULTIPLAY::MATCHOPERATION_OK == result.status && 1 == f.provider->submitCount );
            MULTIPLAY::CMatchEntry entry;
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            ASSERT_TRUE( 2 == entry.preview.revision && 0 == f.book.GetYourTurnCount() && 2 == f.provider->GetLatestRevision( m.matchId ) );
            UInt32 changes = f.recorder.changed;

            // the answer was lost and the player sends it again: OK, and nothing changes
            future = NoFuture();
            REQUIRE_TRUE( f.book.Submit( m.matchId, myPreview, myPackage, future ) );
            REQUIRE_TRUE( MULTIPLAY::CMatchOperationResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( MULTIPLAY::MATCHOPERATION_OK == result.status && 2 == f.provider->submitCount && changes == f.recorder.changed );
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            ASSERT_TRUE( 2 == entry.preview.revision );

            // meanwhile bob and carol played; the book has not heard of it. My next submit names the revision the book knows
            f.packages[ m.matchId ].push_back( myPackage );
            f.Play( m, 1, 2, MULTIPLAY::MATCHSTATUS_ACTIVE );
            f.Play( m, 2, 0, MULTIPLAY::MATCHSTATUS_ACTIVE );
            MULTIPLAY::CMatchPreview latePreview;
            MULTIPLAY::CMatchPackage latePackage;
            f.MakeMyTurn( m, 1, latePreview, latePackage );
            latePackage.revision = 3;
            latePackage.baseRevision = 2;
            latePreview.revision = 3;
            latePreview.packageSha256 = latePackage.sha256;
            latePreview.Sign( f.ids[ 0 ] );
            future = NoFuture();
            REQUIRE_TRUE( f.book.Submit( m.matchId, latePreview, latePackage, future ) );
            REQUIRE_TRUE( MULTIPLAY::CMatchOperationResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( MULTIPLAY::MATCHOPERATION_STALE == result.status && result.hasPreview && 4 == result.preview.revision );
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            ASSERT_TRUE( 4 == entry.preview.revision && 1 == f.book.GetYourTurnCount() && 1 == f.recorder.CountYourTurn( m.matchId, 4 ) );
            ASSERT_TRUE( 4 == f.provider->GetLatestRevision( m.matchId ) );

            // a revision that does not follow what the provider has is rejected, and nothing is merged
            f.MakeMyTurn( m, 1, latePreview, latePackage );
            latePackage.revision = 9;
            latePreview.revision = 9;
            latePreview.packageSha256 = latePackage.sha256;
            latePreview.Sign( f.ids[ 0 ] );
            future = NoFuture();
            REQUIRE_TRUE( f.book.Submit( m.matchId, latePreview, latePackage, future ) );
            REQUIRE_TRUE( MULTIPLAY::CMatchOperationResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( MULTIPLAY::MATCHOPERATION_REJECTED == result.status && !result.reason.IsNULLOrEmpty() );
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            ASSERT_TRUE( 4 == entry.preview.revision );

            // a match the book does not know cannot be submitted to
            future = NoFuture();
            ASSERT_FALSE( f.book.Submit( MULTIPLAY::CMatchId::Generate(), latePreview, latePackage, future ) );
            ASSERT_FALSE( f.book.Submit( m.matchId, MULTIPLAY::CMatchPreview(), latePackage, future ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Failures, delays and a provider that is offline: the future fails or waits, the book is told, and it asks again when the provider is back" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SBookFixture f;
            REQUIRE_TRUE( f.Setup( GUCEF_NULL ) );
            MULTIPLAY::CMatchInfo m = f.NewMatch( "M", 0, MULTIPLAY::MATCHSTATUS_ACTIVE );
            REQUIRE_TRUE( f.book.Start() );
            MULTIPLAY::CMatchPreview myPreview;
            MULTIPLAY::CMatchPackage myPackage;
            f.MakeMyTurn( m, 1, myPreview, myPackage );
            MULTIPLAY::CMatchOperationResult result;
            MULTIPLAY::CMatchEntry entry;

            // a scripted failure: the future fails, the entry stays, the next try works
            f.provider->FailNext( TESTFW::FAKEMATCHOP_SUBMIT, 1 );
            CORE::CFutureResult future = NoFuture();
            REQUIRE_TRUE( f.book.Submit( m.matchId, myPreview, myPackage, future ) );
            ASSERT_TRUE( future.IsReady() || future.HasNoFuture() );
            ASSERT_FALSE( MULTIPLAY::CMatchOperationResult::TryGetFromFuture( future, result ) );
            REQUIRE_TRUE( f.book.GetEntry( m.matchId, entry ) );
            ASSERT_TRUE( 1 == entry.preview.revision && 1 == f.book.GetYourTurnCount() );
            future = NoFuture();
            REQUIRE_TRUE( f.book.Submit( m.matchId, myPreview, myPackage, future ) );
            ASSERT_TRUE( MULTIPLAY::CMatchOperationResult::TryGetFromFuture( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );

            // a delay: the call completes when the provider is ticked, in the order the calls were made
            MULTIPLAY::CMatchInfo m2 = f.NewMatch( "M2", 0, MULTIPLAY::MATCHSTATUS_ACTIVE );
            f.book.RefreshAll();
            f.packages[ m.matchId ].push_back( myPackage );
            f.provider->SetDelayTicks( 2 );
            MULTIPLAY::CMatchPreview p2;
            MULTIPLAY::CMatchPackage k2;
            f.MakeMyTurn( m2, 2, p2, k2 );
            future = NoFuture();
            REQUIRE_TRUE( f.book.Submit( m2.matchId, p2, k2, future ) );
            ASSERT_FALSE( future.IsReady() );
            ASSERT_TRUE( 1 == f.provider->GetPendingCount() );
            REQUIRE_TRUE( f.book.GetEntry( m2.matchId, entry ) );
            ASSERT_TRUE( 1 == entry.preview.revision );
            f.provider->Tick();
            ASSERT_FALSE( future.IsReady() );
            f.provider->Tick();
            ASSERT_TRUE( future.IsReady() && MULTIPLAY::CMatchOperationResult::TryGetFromFuture( future, result ) );
            REQUIRE_TRUE( f.book.GetEntry( m2.matchId, entry ) );
            ASSERT_TRUE( 2 == entry.preview.revision );
            f.provider->SetDelayTicks( 0 );

            // offline: calls fail, the book is told, and when the provider returns the book asks for all previews
            UInt32 asked = f.provider->refreshAllCount;
            f.provider->SetOnline( false );
            ASSERT_TRUE( 1 == f.recorder.problems && "offline" == f.recorder.lastProblem );
            MULTIPLAY::CMatchInfo m3 = f.NewMatch( "M3", 0, MULTIPLAY::MATCHSTATUS_ACTIVE );
            ASSERT_FALSE( f.book.RefreshAll() );
            ASSERT_TRUE( 2 == f.book.GetEntryCount() && asked + 1 == f.provider->refreshAllCount );
            f.provider->SetOnline( true );
            ASSERT_TRUE( asked + 2 == f.provider->refreshAllCount );
            ASSERT_TRUE( 3 == f.book.GetEntryCount() );
            REQUIRE_TRUE( f.book.GetEntry( m3.matchId, entry ) );

            // errors reported by the provider reach the book
            UInt32 problems = f.recorder.problems;
            f.provider->RaiseError( "mailbox full" );
            ASSERT_TRUE( problems + 1 == f.recorder.problems && "mailbox full" == f.recorder.lastProblem );

            // fetch gives the package of a revision, or says it is not there
            future = NoFuture();
            REQUIRE_TRUE( f.book.Fetch( m3.matchId, 1, future ) );
            REQUIRE_TRUE( MULTIPLAY::CMatchOperationResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( MULTIPLAY::MATCHOPERATION_OK == result.status && result.hasPackage && result.package.IsConsistent() && 1 == result.package.revision );
            future = NoFuture();
            REQUIRE_TRUE( f.book.Fetch( m3.matchId, 9, future ) );
            REQUIRE_TRUE( MULTIPLAY::CMatchOperationResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( MULTIPLAY::MATCHOPERATION_NOT_FOUND == result.status && !result.hasPackage );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The reconcile timers are fast for the open match and slow for the rest, and idle in the background" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SBookFixture f;
            REQUIRE_TRUE( f.Setup( GUCEF_NULL ) );
            f.book.SetIntervals( 15000, 300000 );
            MULTIPLAY::CMatchInfo m = f.NewMatch( "M", 1, MULTIPLAY::MATCHSTATUS_ACTIVE );

            // not started: nothing
            f.clock.AdvanceMs( 3600000 );
            f.book.Update();
            ASSERT_TRUE( 0 == f.provider->refreshAllCount );

            REQUIRE_TRUE( f.book.Start() );
            ASSERT_TRUE( 1 == f.provider->refreshAllCount && 0 == f.provider->refreshCount );
            f.clock.AdvanceMs( 14000 );
            f.book.Update();
            ASSERT_TRUE( 1 == f.provider->refreshAllCount && 0 == f.provider->refreshCount );

            // the open match is refreshed every 15 seconds, the rest every 5 minutes
            REQUIRE_TRUE( f.book.OpenMatch( m.matchId ) );
            f.clock.AdvanceMs( 14000 );
            f.book.Update();
            ASSERT_TRUE( 0 == f.provider->refreshCount );
            f.clock.AdvanceMs( 2000 );
            f.book.Update();
            ASSERT_TRUE( 1 == f.provider->refreshCount && m.matchId == f.provider->lastRefreshedMatchId && 1 == f.provider->refreshAllCount );
            f.book.Update();
            ASSERT_TRUE( 1 == f.provider->refreshCount );
            f.clock.AdvanceMs( 16000 );
            f.book.Update();
            ASSERT_TRUE( 2 == f.provider->refreshCount && 1 == f.provider->refreshAllCount );
            f.clock.AdvanceMs( 290000 );
            f.book.Update();
            ASSERT_TRUE( 2 == f.provider->refreshAllCount );

            // closing it ends the fast refresh
            f.book.CloseMatch();
            UInt32 refreshes = f.provider->refreshCount;
            f.clock.AdvanceMs( 60000 );
            f.book.Update();
            ASSERT_TRUE( refreshes == f.provider->refreshCount );

            // in the background nothing is asked, on resume everything is at once
            f.book.SetBackground( true );
            ASSERT_TRUE( f.book.IsBackground() );
            UInt32 all = f.provider->refreshAllCount;
            f.clock.AdvanceMs( 7200000 );
            f.book.Update();
            ASSERT_TRUE( all == f.provider->refreshAllCount );
            f.book.Resume();
            ASSERT_TRUE( all + 1 == f.provider->refreshAllCount && !f.book.IsBackground() );

            // a push drop is a refresh of everything
            ASSERT_TRUE( f.book.RefreshAll() );
            ASSERT_TRUE( all + 2 == f.provider->refreshAllCount );

            // a provider that cannot poll is not asked
            f.provider->SetCapabilities( 0 );
            ASSERT_FALSE( f.book.RefreshAll() );
            ASSERT_FALSE( f.book.Refresh( m.matchId ) );
            ASSERT_TRUE( all + 2 == f.provider->refreshAllCount );

            f.book.Stop();
            ASSERT_FALSE( f.book.IsStarted() );
            ASSERT_TRUE( 1 == f.provider->stopCount );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Providers come from the factory and a config, and the interface only offers what the capabilities say" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            MULTIPLAY::CMatchBook book;
            CORE::CDataNode config;
            config.SetAttribute( "storeDirectory", CORE::CombinePath( MatchTestDirectory( "bookconfig" ), "records" ) );
            config.SetAttribute( "fastIntervalMs", (UInt32) 1234 );
            config.SetAttribute( "slowIntervalMs", (UInt32) 56789 );
            CORE::CDataNode* alpha = config.AddChild( "Provider", GUCEF_DATATYPE_OBJECT );
            alpha->SetAttribute( "type", CORE::CString( "fake" ) );
            alpha->AddChild( "Config", GUCEF_DATATYPE_OBJECT )->SetAttribute( "name", CORE::CString( "alpha" ) );
            CORE::CDataNode* beta = config.AddChild( "Provider", GUCEF_DATATYPE_OBJECT );
            beta->SetAttribute( "type", CORE::CString( "fake" ) );
            beta->AddChild( "Config", GUCEF_DATATYPE_OBJECT )->SetAttribute( "name", CORE::CString( "beta" ) );
            ASSERT_TRUE( book.LoadConfig( config ) );
            ASSERT_TRUE( 2 == book.GetProviders().size() && !book.FindProvider( "alpha" ).IsNULL() && !book.FindProvider( "beta" ).IsNULL() && book.FindProvider( "gamma" ).IsNULL() );
            ASSERT_TRUE( book.GetStore().GetDirectory().HasSubstr( "records" ) >= 0 );
            CORE::CDataNode saved;
            ASSERT_TRUE( book.SaveConfig( saved ) );
            ASSERT_TRUE( 1234 == saved.GetAttributeValueOrChildValueByName( "fastIntervalMs" ).AsUInt32() && 56789 == saved.GetAttributeValueOrChildValueByName( "slowIntervalMs" ).AsUInt32() );
            ASSERT_TRUE( "json" == saved.GetAttributeValueOrChildValueByName( "storeCodec" ).AsString() );

            // a provider type that does not exist, a name that is taken
            ASSERT_FALSE( book.AddProvider( "no-such-provider", GUCEF_NULL ) );
            ASSERT_FALSE( book.AddProvider( "fake", alpha->FindChild( "Config" ) ) );
            CORE::CDataNode withUnknown;
            withUnknown.AddChild( "Provider", GUCEF_DATATYPE_OBJECT )->SetAttribute( "type", CORE::CString( "unknown" ) );
            MULTIPLAY::CMatchBook other;
            ASSERT_FALSE( other.LoadConfig( withUnknown ) );

            // capabilities: what is not offered is not done
            MULTIPLAY::CIMatchProviderPtr providerPtr = book.FindProvider( "alpha" );
            TESTFW::CFakeMatchProvider* provider = static_cast< TESTFW::CFakeMatchProvider* >( providerPtr.GetPointerAlways() );
            CORE::CFutureResult future = NoFuture();
            ASSERT_TRUE( provider->HasCapability( MULTIPLAY::MATCHCAPABILITY_NUDGE ) && provider->Nudge( "id", "user", future ) && 1 == provider->nudgeCount && "user" == provider->lastNudgedUserId );
            ASSERT_TRUE( provider->Resign( "id", future ) && provider->Rematch( "id", future ) );
            provider->SetCapabilities( MULTIPLAY::MATCHCAPABILITY_PREVIEW_POLL );
            ASSERT_FALSE( provider->HasCapability( MULTIPLAY::MATCHCAPABILITY_NUDGE ) );
            ASSERT_FALSE( provider->Nudge( "id", "user", future ) );
            ASSERT_FALSE( provider->Resign( "id", future ) );
            ASSERT_FALSE( provider->Rematch( "id", future ) );
            ASSERT_TRUE( 1 == provider->nudgeCount );

            // the operations a provider does not implement answer false, so the interface can hide them
            CMinimalMatchProvider minimal;
            MULTIPLAY::CMatchProposal proposal;
            MULTIPLAY::CMatchVote vote;
            MULTIPLAY::CMatchAcceptance acceptance;
            MULTIPLAY::CMatchParticipant participant;
            ASSERT_FALSE( minimal.Invite( "id", participant, future ) || minimal.Respond( "id", acceptance, future ) );
            ASSERT_FALSE( minimal.Resign( "id", future ) || minimal.Nudge( "id", "user", future ) || minimal.Remove( "id", future ) || minimal.Rematch( "id", future ) );
            ASSERT_FALSE( minimal.Propose( "id", proposal, future ) || minimal.Vote( "id", vote, future ) );
            ASSERT_FALSE( minimal.HasCapability( MULTIPLAY::MATCHCAPABILITY_NUDGE ) || minimal.HasCapability( MULTIPLAY::MATCHCAPABILITY_PUSH ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A match is created through the book with the provider named, and the book learns about it" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SBookFixture f;
            REQUIRE_TRUE( f.Setup( GUCEF_NULL ) );
            REQUIRE_TRUE( f.book.Start() );
            MULTIPLAY::CMatchInfo info = TESTFW::MakeTestMatchInfo( f.ids, "Created", false );
            MULTIPLAY::CMatchRecord record( info );
            REQUIRE_TRUE( record.Sign( f.ids[ 0 ] ) );
            MULTIPLAY::CMatchPackage package = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "start" );
            MULTIPLAY::CMatchPreview preview = TESTFW::MakeTestPreview( info, f.ids[ 0 ], package, &f.ids[ 1 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 0 );
            CORE::CFutureResult future = NoFuture();
            ASSERT_FALSE( f.book.CreateMatch( "nobody", record, package, preview, future ) );
            REQUIRE_TRUE( f.book.CreateMatch( "fake", record, package, preview, future ) );
            MULTIPLAY::CMatchOperationResult result;
            ASSERT_TRUE( MULTIPLAY::CMatchOperationResult::TryGetFromFuture( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
            ASSERT_TRUE( 1 == f.book.GetEntryCount() && 1 == f.recorder.added );
            MULTIPLAY::CMatchEntry entry;
            REQUIRE_TRUE( f.book.GetEntry( info.matchId, entry ) );
            ASSERT_TRUE( entry.hasInfo && "fake" == entry.providerName && 0 == f.book.GetYourTurnCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The player's own name for a game is kept by the book, persisted, bounded, and survives newer revisions" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
            CORE::CString dir;
            MULTIPLAY::CMatchInfo m1, m2;
            {
                SBookFixture f;
                REQUIRE_TRUE( f.Setup( "localtitle" ) );
                ids = f.ids;
                dir = f.book.GetStore().GetDirectory();
                m1 = f.NewMatch( "Shared name", 1, MULTIPLAY::MATCHSTATUS_ACTIVE );
                m2 = f.NewMatch( "Other", 0, MULTIPLAY::MATCHSTATUS_ACTIVE );
                REQUIRE_TRUE( f.book.Start() );

                MULTIPLAY::CMatchEntry entry;
                REQUIRE_TRUE( f.book.GetEntry( m1.matchId, entry ) );
                ASSERT_TRUE( entry.localTitle.IsNULLOrEmpty() && "Shared name" == entry.GetDisplayTitle() );

                // setting it raises a flags change for that match only, and an unknown match is refused
                const UInt32 flagsBefore = f.recorder.flags;
                ASSERT_FALSE( f.book.SetLocalTitle( "3f2504e0-4f89-41d3-9a0c-0305e82c3301", "x" ) );
                ASSERT_TRUE( flagsBefore == f.recorder.flags );
                ASSERT_TRUE( f.book.SetLocalTitle( m1.matchId, "Friday game" ) );
                ASSERT_TRUE( flagsBefore + 1 == f.recorder.flags );
                REQUIRE_TRUE( f.book.GetEntry( m1.matchId, entry ) );
                ASSERT_TRUE( "Friday game" == entry.localTitle && "Friday game" == entry.GetDisplayTitle() && "Shared name" == entry.info.title );

                // the same name again is not a change
                ASSERT_TRUE( f.book.SetLocalTitle( m1.matchId, "Friday game" ) );
                ASSERT_TRUE( flagsBefore + 1 == f.recorder.flags );

                // the other match is untouched
                REQUIRE_TRUE( f.book.GetEntry( m2.matchId, entry ) );
                ASSERT_TRUE( entry.localTitle.IsNULLOrEmpty() && "Other" == entry.GetDisplayTitle() );

                // a newer revision of the match from the provider keeps the player's name
                f.Play( m1, 1, 0, MULTIPLAY::MATCHSTATUS_ACTIVE );
                f.book.RefreshAll();
                REQUIRE_TRUE( f.book.GetEntry( m1.matchId, entry ) );
                ASSERT_TRUE( 2 == entry.preview.revision && "Friday game" == entry.localTitle );

                // the name is bounded to 64 bytes, whole characters only
                CORE::CString longTitle;
                for ( int i=0; i<200; ++i )
                    longTitle += "a";
                ASSERT_TRUE( f.book.SetLocalTitle( m2.matchId, longTitle ) );
                REQUIRE_TRUE( f.book.GetEntry( m2.matchId, entry ) );
                ASSERT_TRUE( 64 == entry.localTitle.Length() );
                CORE::CString wide;
                for ( int i=0; i<100; ++i )
                    wide += CORE::CString( "\xC3\xA9" );     // e with an acute accent: two bytes each
                ASSERT_TRUE( f.book.SetLocalTitle( m2.matchId, wide ) );
                REQUIRE_TRUE( f.book.GetEntry( m2.matchId, entry ) );
                ASSERT_TRUE( 32 == entry.localTitle.Length() && entry.localTitle == wide.SubstrToIndex( 32 ) );

                // control characters are not kept, and a name of only blanks is no name
                ASSERT_TRUE( f.book.SetLocalTitle( m2.matchId, "Two\nlines\t!" ) );
                REQUIRE_TRUE( f.book.GetEntry( m2.matchId, entry ) );
                ASSERT_TRUE( "Twolines!" == entry.localTitle );
                ASSERT_TRUE( f.book.SetLocalTitle( m2.matchId, "   " ) );
                REQUIRE_TRUE( f.book.GetEntry( m2.matchId, entry ) );
                ASSERT_TRUE( entry.localTitle.IsNULLOrEmpty() && "Other" == entry.GetDisplayTitle() );
            }
            {
                // the next start: the name is read from the record on disk
                SBookFixture g;
                g.Setup( GUCEF_NULL, &ids );
                g.book.SetStoreDirectory( dir );
                REQUIRE_TRUE( g.book.LoadFromStore() );
                MULTIPLAY::CMatchEntry e1, e2;
                REQUIRE_TRUE( g.book.GetEntry( m1.matchId, e1 ) && g.book.GetEntry( m2.matchId, e2 ) );
                ASSERT_TRUE( "Friday game" == e1.localTitle && e2.localTitle.IsNULLOrEmpty() );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A cancelled match is finished for the player, a spectator never has the turn, and each entry has a state for its label" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            SBookFixture f;
            REQUIRE_TRUE( f.Setup( GUCEF_NULL ) );

            // forming: the creator waits for answers
            MULTIPLAY::CMatchInfo info = TESTFW::MakeTestMatchInfo( f.ids, "Lobby", true );
            info.status = MULTIPLAY::MATCHSTATUS_FORMING;
            info.participants[ 1 ].status = MULTIPLAY::MATCHPARTICIPANT_INVITED;
            MULTIPLAY::CMatchPackage p1 = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "Lobby" );
            MULTIPLAY::CMatchPreview preview = TESTFW::MakeTestPreview( info, f.ids[ 0 ], p1, &f.ids[ 0 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 0 );
            f.provider->InjectMatch( info, preview, p1 );
            MULTIPLAY::CMatchInfo invited = f.NewMatch( "Invited", 1, MULTIPLAY::MATCHSTATUS_FORMING, 0, true );
            MULTIPLAY::CMatchInfo watching = TESTFW::MakeTestMatchInfo( f.ids, "Watching", true );
            watching.participants[ 0 ].status = MULTIPLAY::MATCHPARTICIPANT_SPECTATOR;
            watching.participants[ 0 ].statusCode = "SEED";
            watching.participants[ 0 ].statusReason = "The galaxy of the first turn is not the one of the settings";
            MULTIPLAY::CMatchPackage w1 = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "Watching" );
            MULTIPLAY::CMatchPreview wp = TESTFW::MakeTestPreview( watching, f.ids[ 1 ], w1, &f.ids[ 0 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 0 );
            f.provider->InjectMatch( watching, wp, w1 );
            REQUIRE_TRUE( f.book.Start() );
            MULTIPLAY::CMatchEntry entry;
            REQUIRE_TRUE( f.book.GetEntry( info.matchId, entry ) );
            ASSERT_TRUE( MULTIPLAY::MATCHENTRYSTATE_FORMING == f.book.GetStateOf( entry ) && MULTIPLAY::MATCHGROUP_FORMING_OR_INVITED == f.book.GetGroupOf( entry ) );

            // cancelled by the creator: finished for the player and not their turn, whatever the preview says
            MULTIPLAY::CMatchInfo cancelled = info;
            cancelled.status = MULTIPLAY::MATCHSTATUS_CANCELLED;
            cancelled.closedByUserId = f.ids[ 0 ].userId;
            f.provider->RaiseUpdated( UpdateOf( cancelled, preview ) );
            REQUIRE_TRUE( f.book.GetEntry( info.matchId, entry ) );
            ASSERT_TRUE( MULTIPLAY::MATCHENTRYSTATE_CANCELLED == f.book.GetStateOf( entry ) && MULTIPLAY::MATCHGROUP_FINISHED == f.book.GetGroupOf( entry ) );
            ASSERT_TRUE( entry.info.closedByUserId == f.ids[ 0 ].userId && 0 == f.book.GetYourTurnCount() );

            // invited: another player's match that the local player did not answer
            REQUIRE_TRUE( f.book.GetEntry( invited.matchId, entry ) );
            ASSERT_TRUE( MULTIPLAY::MATCHENTRYSTATE_INVITED == f.book.GetStateOf( entry ) );

            // a spectator: the player is outside the game, it is never their turn and the state says why
            REQUIRE_TRUE( f.book.GetEntry( watching.matchId, entry ) );
            ASSERT_TRUE( MULTIPLAY::MATCHENTRYSTATE_INCOMPATIBLE == f.book.GetStateOf( entry ) && MULTIPLAY::MATCHGROUP_THEIR_TURN == f.book.GetGroupOf( entry ) );
            watching.participants[ 0 ].statusCode = GUCEF_MULTIPLAY_SPECTATOR_REASON_VOLUNTARY;
            f.provider->RaiseUpdated( UpdateOf( watching, wp ) );
            REQUIRE_TRUE( f.book.GetEntry( watching.matchId, entry ) );
            ASSERT_TRUE( MULTIPLAY::MATCHENTRYSTATE_SPECTATING == f.book.GetStateOf( entry ) && entry.info.participants[ 0 ].IsSpectator() );
            ASSERT_TRUE( CORE::CString( "SPECTATING" ) == MULTIPLAY::MatchEntryStateToString( MULTIPLAY::MATCHENTRYSTATE_SPECTATING ) );
            ASSERT_TRUE( 0 == f.book.GetYourTurnCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A match that is being initialized by its creator (invitations not sent, players still to introduce themselves) has the state Initializing for its creator, is in the group of the forming matches, is never the turn of anybody, and survives the store" )
        GUCEF_TESTFW_SKIP_IF( !signaturesAvailable, "There is no ed25519 signature codec" );
        try
        {
            ASSERT_TRUE( CORE::CString( "INITIALIZING" ) == MULTIPLAY::MatchStatusToString( MULTIPLAY::MATCHSTATUS_INITIALIZING ) && MULTIPLAY::MATCHSTATUS_INITIALIZING == MULTIPLAY::StringToMatchStatus( "initializing" ) );
            ASSERT_TRUE( CORE::CString( "INITIALIZING" ) == MULTIPLAY::MatchEntryStateToString( MULTIPLAY::MATCHENTRYSTATE_INITIALIZING ) );
            const CORE::CString dir = CORE::RelativePath( "$TEMPDIR$/gucefMULTIPLAY_TestApp_initializing" );
            CORE::DeleteDir( dir.C_String(), true );
            SBookFixture f;
            REQUIRE_TRUE( f.Setup( GUCEF_NULL ) );
            f.book.SetStoreDirectory( dir );

            // the creator's draft: a match with a status of its own, before any record exists
            MULTIPLAY::CMatchInfo draft = TESTFW::MakeTestMatchInfo( f.ids, "Friday draft", true );
            draft.status = MULTIPLAY::MATCHSTATUS_INITIALIZING;
            draft.participants[ 1 ].status = MULTIPLAY::MATCHPARTICIPANT_INVITED;
            MULTIPLAY::CMatchPackage p1 = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "Draft" );
            MULTIPLAY::CMatchPreview preview = TESTFW::MakeTestPreview( draft, f.ids[ 0 ], p1, &f.ids[ 0 ], MULTIPLAY::MATCHSTATUS_INITIALIZING, 0 );
            f.provider->InjectMatch( draft, preview, p1 );
            REQUIRE_TRUE( f.book.Start() );
            MULTIPLAY::CMatchEntry entry;
            REQUIRE_TRUE( f.book.GetEntry( draft.matchId, entry ) );
            ASSERT_TRUE( MULTIPLAY::MATCHENTRYSTATE_INITIALIZING == f.book.GetStateOf( entry ) && MULTIPLAY::MATCHGROUP_FORMING_OR_INVITED == f.book.GetGroupOf( entry ) && 0 == f.book.GetYourTurnCount() );

            // once the record exists the match is forming like any other until everybody answered
            MULTIPLAY::CMatchInfo forming = draft;
            forming.status = MULTIPLAY::MATCHSTATUS_FORMING;
            MULTIPLAY::CMatchPackage first = TESTFW::MakeTestPackage( 2, GUCEF_NULL, "Record" );
            MULTIPLAY::CMatchPreview started = TESTFW::MakeTestPreview( forming, f.ids[ 0 ], first, &f.ids[ 0 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 0 );
            f.provider->RaiseUpdated( UpdateOf( forming, started ) );
            REQUIRE_TRUE( f.book.GetEntry( draft.matchId, entry ) );
            ASSERT_TRUE( MULTIPLAY::MATCHENTRYSTATE_FORMING == f.book.GetStateOf( entry ) );

            // and it can be written to the store and read from it with the status it had
            MULTIPLAY::CMatchInfo second = TESTFW::MakeTestMatchInfo( f.ids, "Second draft", true );
            second.status = MULTIPLAY::MATCHSTATUS_INITIALIZING;
            MULTIPLAY::CMatchPackage p2 = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "Second" );
            MULTIPLAY::CMatchPreview preview2 = TESTFW::MakeTestPreview( second, f.ids[ 0 ], p2, &f.ids[ 0 ], MULTIPLAY::MATCHSTATUS_INITIALIZING, 0 );
            f.provider->RaiseUpdated( UpdateOf( second, preview2 ) );
            REQUIRE_TRUE( f.book.GetEntry( second.matchId, entry ) && MULTIPLAY::MATCHSTATUS_INITIALIZING == entry.preview.status );
            CORE::CDataNode node;
            ASSERT_TRUE( entry.preview.Serialize( node, CORE::CDataNodeSerializableSettings() ) );
            MULTIPLAY::CMatchPreview read;
            ASSERT_TRUE( read.Deserialize( node, CORE::CDataNodeSerializableSettings() ) && MULTIPLAY::MATCHSTATUS_INITIALIZING == read.status );
            // the status is the creator's own: the strict reader of what comes from outside does not take it from anybody else
            CORE::CDynamicBuffer bytes;
            ASSERT_TRUE( entry.preview.ToBinary( bytes ) );
            MULTIPLAY::CMatchPreview outside;
            ASSERT_FALSE( outside.FromBinary( bytes.GetConstBufferPtr(), bytes.GetDataSize() ) );
            CORE::DeleteDir( dir.C_String(), true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    MULTIPLAY::CMultiplayGlobal::Instance()->GetMatchProviderFactory().UnregisterConcreteFactory( "fake" );

}
