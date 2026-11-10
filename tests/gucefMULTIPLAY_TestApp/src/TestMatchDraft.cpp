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

#ifndef GUCEF_MULTIPLAY_CMATCHDRAFTBOARD_H
#include "gucefMULTIPLAY_CMatchDraftBoard.h"
#define GUCEF_MULTIPLAY_CMATCHDRAFTBOARD_H
#endif /* GUCEF_MULTIPLAY_CMATCHDRAFTBOARD_H ? */

#include "TestMatchCommon.h"
#include "TestMatchDraft.h"

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

static const Int64 T0 = 1700000000000LL;
static const Int64 Hour = 60LL * 60 * 1000;

/** A draft with a host (0) and three invitees (1 to 3), a board per person and the table as the host knows it */
struct SDraftFixture
{
    GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
    CORE::CString matchId;
    MULTIPLAY::CMatchDraftBoard boards[ 4 ];
    MULTIPLAY::TMatchRosterEntryVector table;       /**< what the host knows */
    MULTIPLAY::TMatchDecisionPolicy policy;
    CORE::CString settingsDigest;

    SDraftFixture( void ) : ids(), matchId(), table(), policy( MULTIPLAY::MATCHDECISIONPOLICY_VOTE ), settingsDigest() {}

    MULTIPLAY::CMatchRosterEntry Introduced( UInt32 index ) const
    {
        MULTIPLAY::CMatchRosterEntry entry;
        entry.userId = ids[ index ].userId;
        entry.displayName = ids[ index ].displayName;
        entry.contact = CORE::CString( "player" ) + CORE::UInt32ToString( index ) + "@example.org";
        entry.publicKey = ids[ index ].publicKey;
        entry.status = MULTIPLAY::MATCHPARTICIPANT_ACTIVE;
        entry.boss = 0 == index && MULTIPLAY::MATCHDECISIONPOLICY_BOSS == policy;
        return entry;
    }

    MULTIPLAY::CMatchRosterEntry Waiting( UInt32 index ) const
    {
        MULTIPLAY::CMatchRosterEntry entry;
        entry.contact = CORE::CString( "player" ) + CORE::UInt32ToString( index ) + "@example.org";
        entry.userId = MULTIPLAY::CMatchRosterEntry::MakePendingUserId( matchId, entry.contact );
        entry.displayName = ids[ index ].displayName;
        entry.status = MULTIPLAY::MATCHPARTICIPANT_INVITED;
        return entry;
    }

    bool Make( MULTIPLAY::TMatchDecisionPolicy decisionPolicy = MULTIPLAY::MATCHDECISIONPOLICY_VOTE )
    {
        policy = decisionPolicy;
        ids.clear();
        for ( UInt32 i=0; i<4; ++i )
        {
            MULTIPLAY::CPlayerIdentity identity;
            if ( !TESTFW::MakeTestIdentity( ( CORE::CString( "Player" ) + CORE::UInt32ToString( i ) ).C_String(), identity ) )
                return false;
            ids.push_back( identity );
        }
        matchId = MULTIPLAY::CMatchId::Generate();
        MULTIPLAY::CMatchInfo::TSettingsMap settings;
        settings[ "seats" ] = "4";
        settingsDigest = MULTIPLAY::CMatchRoster::MakeSettingsDigest( settings );
        if ( !boards[ 0 ].InitializeAsHost( ids[ 0 ], matchId ) )
            return false;
        for ( UInt32 i=1; i<4; ++i )
        {
            if ( !boards[ i ].InitializeAsParticipant( matchId, ids[ 0 ].userId, ids[ 0 ].publicKey, ids[ i ] ) )
                return false;
            boards[ i ].SetOwnContact( Introduced( i ).contact );
        }
        table.clear();
        table.push_back( Introduced( 0 ) );
        return true;
    }

    /** The host publishes the table as it is and every participant board accepts the roster */
    bool Publish( Int64 now, MULTIPLAY::CMatchRoster* out = GUCEF_NULL )
    {
        MULTIPLAY::CMatchRoster roster;
        if ( !boards[ 0 ].PublishRoster( ids[ 0 ], table, policy, settingsDigest, now, roster ) )
            return false;
        for ( UInt32 i=1; i<4; ++i )
            boards[ i ].AcceptRoster( roster, now );
        if ( GUCEF_NULL != out )
            *out = roster;
        return true;
    }

    MULTIPLAY::CMatchVote VoteOn( UInt32 voter, const MULTIPLAY::CMatchProposal& proposal, bool yes, Int64 when ) const
    {
        MULTIPLAY::CMatchVote vote;
        vote.Sign( ids[ voter ], proposal, yes, when );
        return vote;
    }

    /** Hands a proposal to every board other than the proposer's */
    void Send( UInt32 from, const MULTIPLAY::CMatchProposal& proposal, Int64 now )
    {
        CORE::CString reason;
        for ( UInt32 i=0; i<4; ++i )
            if ( i != from )
                boards[ i ].AddProposal( proposal, now, reason );
    }

    /** Hands a vote to every board other than the voter's */
    void Send( UInt32 from, const MULTIPLAY::CMatchVote& vote, Int64 now )
    {
        CORE::CString reason;
        for ( UInt32 i=0; i<4; ++i )
            if ( i != from )
                boards[ i ].AddVote( vote, now, reason );
    }

    /** Player 'from' proposes; the proposer's own board and the others hold it */
    bool Propose( UInt32 from, MULTIPLAY::TMatchDecisionKind kind, UInt32 target, Int64 now, MULTIPLAY::CMatchProposal& proposal )
    {
        CORE::CString reason;
        const CORE::CString targetId = kind == MULTIPLAY::MATCHDECISION_FORCE_START ? CORE::CString() : ids[ target ].userId;
        if ( !boards[ from ].MakeProposal( ids[ from ], kind, targetId, now, 72 * Hour, proposal, reason ) )
            return false;
        if ( MULTIPLAY::MATCHDRAFT_OK != boards[ from ].AddProposal( proposal, now, reason ) )
            return false;
        Send( from, proposal, now );
        return true;
    }

    bool Cast( UInt32 voter, const MULTIPLAY::CMatchProposal& proposal, bool yes, Int64 now )
    {
        const MULTIPLAY::CMatchVote vote = VoteOn( voter, proposal, yes, now );
        CORE::CString reason;
        if ( MULTIPLAY::MATCHDRAFT_OK != boards[ voter ].AddVote( vote, now, reason ) )
            return false;
        Send( voter, vote, now );
        return true;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMatchDraftTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefMULTIPLAY matches: the roster and the decisions of the draft stage" );

    GUCEF_TESTFW_TESTCASE( "Test: A roster is signed by the host, survives a round trip, and any change of it breaks the signature" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDraftFixture f;
            REQUIRE_TRUE( f.Make() );
            f.table.push_back( f.Introduced( 1 ) );
            f.table.push_back( f.Waiting( 2 ) );
            MULTIPLAY::CMatchRoster roster;
            REQUIRE_TRUE( f.boards[ 0 ].PublishRoster( f.ids[ 0 ], f.table, f.policy, f.settingsDigest, T0, roster ) );
            ASSERT_TRUE( 1 == roster.revision && 3 == roster.entries.size() && 2 == roster.CountVoters() );
            CORE::CString reason;
            ASSERT_TRUE( roster.Verify( f.ids[ 0 ].publicKey, reason ) );
            ASSERT_FALSE( roster.Verify( f.ids[ 1 ].publicKey, reason ) );        // another key than the host's

            MULTIPLAY::CMatchRoster back;
            REQUIRE_TRUE( MatchJsonRoundTrip( roster, back, "roster" ) );
            ASSERT_TRUE( back.Verify( f.ids[ 0 ].publicKey, reason ) && back.GetDigest() == roster.GetDigest() );

            // somebody else's key under the same name, a status, the revision, the settings: each breaks the signature
            MULTIPLAY::CMatchRoster forged = roster;
            forged.entries[ 1 ].publicKey = f.ids[ 3 ].publicKey;
            forged.entries[ 1 ].userId = f.ids[ 3 ].userId;
            ASSERT_FALSE( forged.Verify( f.ids[ 0 ].publicKey, reason ) );
            forged = roster;
            forged.entries[ 2 ].status = MULTIPLAY::MATCHPARTICIPANT_ACTIVE;
            ASSERT_FALSE( forged.Verify( f.ids[ 0 ].publicKey, reason ) );
            forged = roster;
            forged.revision = 9;
            ASSERT_FALSE( forged.Verify( f.ids[ 0 ].publicKey, reason ) );
            forged = roster;
            forged.settingsDigest = "00";
            ASSERT_FALSE( forged.Verify( f.ids[ 0 ].publicKey, reason ) );

            // a key that is not the key of its user id is refused even when it is signed
            forged = roster;
            forged.entries[ 1 ].publicKey = f.ids[ 3 ].publicKey;
            REQUIRE_TRUE( forged.Sign( f.ids[ 0 ] ) );
            ASSERT_FALSE( forged.Verify( f.ids[ 0 ].publicKey, reason ) );
            // a participant who has not introduced itself has no key; one that has, has
            forged = roster;
            forged.entries[ 2 ].publicKey = f.ids[ 2 ].publicKey;
            REQUIRE_TRUE( forged.Sign( f.ids[ 0 ] ) );
            ASSERT_FALSE( forged.Verify( f.ids[ 0 ].publicKey, reason ) );
            // the same contact twice
            forged = roster;
            forged.entries[ 2 ].contact = forged.entries[ 1 ].contact;
            REQUIRE_TRUE( forged.Sign( f.ids[ 0 ] ) );
            ASSERT_FALSE( forged.Verify( f.ids[ 0 ].publicKey, reason ) );
            // too many entries
            forged = roster;
            while ( forged.entries.size() <= MULTIPLAY::CMatchRoster::MaxEntries )
            {
                MULTIPLAY::CMatchRosterEntry extra = f.Waiting( 3 );
                extra.contact = CORE::CString( "extra" ) + CORE::UInt32ToString( (UInt32) forged.entries.size() ) + "@example.org";
                extra.userId = MULTIPLAY::CMatchRosterEntry::MakePendingUserId( f.matchId, extra.contact );
                forged.entries.push_back( extra );
            }
            REQUIRE_TRUE( forged.Sign( f.ids[ 0 ] ) );
            ASSERT_FALSE( forged.Verify( f.ids[ 0 ].publicKey, reason ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A participant takes the roster of the host and refuses a forged, a stale, a foreign one and one that lists the player under another key" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDraftFixture f;
            REQUIRE_TRUE( f.Make() );
            f.table.push_back( f.Introduced( 1 ) );
            f.table.push_back( f.Introduced( 2 ) );
            MULTIPLAY::CMatchRoster r1, r2, r3;
            REQUIRE_TRUE( f.boards[ 0 ].PublishRoster( f.ids[ 0 ], f.table, f.policy, f.settingsDigest, T0, r1 ) );
            // nothing changed: nothing is published
            MULTIPLAY::CMatchRoster none;
            ASSERT_FALSE( f.boards[ 0 ].PublishRoster( f.ids[ 0 ], f.table, f.policy, f.settingsDigest, T0 + 1, none ) );
            f.table.push_back( f.Introduced( 3 ) );
            REQUIRE_TRUE( f.boards[ 0 ].PublishRoster( f.ids[ 0 ], f.table, f.policy, f.settingsDigest, T0 + 2, r2 ) );
            f.table[ 3 ].displayName = "Renamed";
            REQUIRE_TRUE( f.boards[ 0 ].PublishRoster( f.ids[ 0 ], f.table, f.policy, f.settingsDigest, T0 + 3, r3 ) );
            ASSERT_TRUE( 1 == r1.revision && 2 == r2.revision && 3 == r3.revision );

            MULTIPLAY::CMatchDraftBoard& b = f.boards[ 1 ];
            ASSERT_FALSE( b.HasRoster() );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_OK == b.AcceptRoster( r1, T0 ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_DUPLICATE == b.AcceptRoster( r1, T0 ) );
            // a roster further on is taken at once (a roster is the whole table, nothing is built on the one before)
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_OK == b.AcceptRoster( r3, T0 ) );
            ASSERT_TRUE( 3 == b.GetRevision() );
            // the one in between is stale now
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_STALE == b.AcceptRoster( r2, T0 ) );
            ASSERT_TRUE( 3 == b.GetRevision() );

            // signed by another key than the host's: the key in it is the stranger's
            MULTIPLAY::CPlayerIdentity stranger;
            REQUIRE_TRUE( TESTFW::MakeTestIdentity( "Stranger", stranger ) );
            MULTIPLAY::CMatchRoster foreign = r3;
            foreign.revision = 4;
            foreign.hostUserId = stranger.userId;
            foreign.hostPublicKey = stranger.publicKey;
            foreign.entries[ 0 ].userId = stranger.userId;
            foreign.entries[ 0 ].publicKey = stranger.publicKey;
            REQUIRE_TRUE( foreign.Sign( stranger ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_WRONG_HOST == b.AcceptRoster( foreign, T0 ) );

            // claims the host and is signed by a stranger
            MULTIPLAY::CMatchRoster claimed = r3;
            claimed.revision = 4;
            REQUIRE_TRUE( claimed.Sign( stranger ) );
            claimed.hostUserId = f.ids[ 0 ].userId;
            claimed.hostPublicKey = f.ids[ 0 ].publicKey;
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_BAD_SIGNATURE == b.AcceptRoster( claimed, T0 ) );

            // a tampered copy of a roster the host did sign
            MULTIPLAY::CMatchRoster tampered = r3;
            tampered.revision = 5;
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_BAD_SIGNATURE == b.AcceptRoster( tampered, T0 ) );

            // another game
            MULTIPLAY::CMatchRoster other = r3;
            other.matchId = MULTIPLAY::CMatchId::Generate();
            other.revision = 6;
            REQUIRE_TRUE( other.Sign( f.ids[ 0 ] ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_WRONG_MATCH == b.AcceptRoster( other, T0 ) );
            ASSERT_TRUE( 3 == b.GetRevision() );

            // a roster that lists the contact of the player under another key
            MULTIPLAY::CMatchRoster swapped = r3;
            swapped.revision = 7;
            swapped.entries[ 1 ].publicKey = stranger.publicKey;
            swapped.entries[ 1 ].userId = stranger.userId;
            REQUIRE_TRUE( swapped.Sign( f.ids[ 0 ] ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_OWN_KEY_MISMATCH == b.AcceptRoster( swapped, T0 ) );
            ASSERT_TRUE( 3 == b.GetRevision() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Two different rosters of the host for one revision are shown as equivocation and the first stays; a changed key is held until it is confirmed" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDraftFixture f;
            REQUIRE_TRUE( f.Make() );
            f.table.push_back( f.Introduced( 1 ) );
            f.table.push_back( f.Introduced( 2 ) );
            MULTIPLAY::CMatchRoster r1;
            REQUIRE_TRUE( f.boards[ 0 ].PublishRoster( f.ids[ 0 ], f.table, f.policy, f.settingsDigest, T0, r1 ) );
            MULTIPLAY::CMatchRoster twin = r1;
            twin.entries[ 2 ].displayName = "Somebody else";
            REQUIRE_TRUE( twin.Sign( f.ids[ 0 ] ) );

            MULTIPLAY::CMatchDraftBoard& b = f.boards[ 1 ];
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_OK == b.AcceptRoster( r1, T0 ) );
            ASSERT_FALSE( b.IsHostEquivocating() );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_EQUIVOCATION == b.AcceptRoster( twin, T0 ) );
            ASSERT_TRUE( b.IsHostEquivocating() );
            MULTIPLAY::CMatchRoster held;
            REQUIRE_TRUE( b.GetRoster( held ) );
            ASSERT_TRUE( held.GetDigest() == r1.GetDigest() );
            UInt32 revision = 0;
            CORE::CString first, second;
            ASSERT_TRUE( b.GetEquivocation( revision, first, second ) && 1 == revision && first == r1.GetDigest() && second == twin.GetDigest() );
            // the same twin again changes nothing
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_EQUIVOCATION == b.AcceptRoster( twin, T0 ) && b.IsHostEquivocating() );

            // a key for a contact that differs from an earlier roster is held
            MULTIPLAY::CPlayerIdentity newKey;
            REQUIRE_TRUE( TESTFW::MakeTestIdentity( "NewKey", newKey ) );
            MULTIPLAY::CMatchRoster changed = r1;
            changed.revision = 2;
            changed.entries[ 2 ].publicKey = newKey.publicKey;
            changed.entries[ 2 ].userId = newKey.userId;
            REQUIRE_TRUE( changed.Sign( f.ids[ 0 ] ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_KEY_CHANGED == b.AcceptRoster( changed, T0 ) );
            ASSERT_TRUE( b.HasHeldRoster() && 1 == b.GetRevision() );
            GUCEF::vector< CORE::CString > contacts, oldKeys, newKeys;
            b.GetHeldKeyChanges( contacts, oldKeys, newKeys );
            ASSERT_TRUE( 1 == contacts.size() && oldKeys[ 0 ] == f.ids[ 2 ].publicKey && newKeys[ 0 ] == newKey.publicKey );
            ASSERT_TRUE( b.ConfirmHeldRoster( true, T0 ) );
            ASSERT_FALSE( b.HasHeldRoster() );
            ASSERT_TRUE( 2 == b.GetRevision() );

            // refused: the roster in force stays
            MULTIPLAY::CMatchDraftBoard& c = f.boards[ 3 ];
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_OK == c.AcceptRoster( r1, T0 ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_KEY_CHANGED == c.AcceptRoster( changed, T0 ) );
            ASSERT_TRUE( c.ConfirmHeldRoster( false, T0 ) );
            ASSERT_TRUE( 1 == c.GetRevision() && !c.HasHeldRoster() );
            ASSERT_FALSE( c.ConfirmHeldRoster( true, T0 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Invitees propose a kick, the others vote, and every participant counts the same tally" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDraftFixture f;
            REQUIRE_TRUE( f.Make() );
            f.table.push_back( f.Introduced( 1 ) );
            f.table.push_back( f.Introduced( 2 ) );
            f.table.push_back( f.Introduced( 3 ) );
            REQUIRE_TRUE( f.Publish( T0 ) );

            MULTIPLAY::CMatchProposal kick;
            REQUIRE_TRUE( f.Propose( 1, MULTIPLAY::MATCHDECISION_KICK, 3, T0 + 1000, kick ) );
            ASSERT_TRUE( 1 == kick.rosterRevision && 4 == kick.voterUserIds.size() );

            // after the proposal alone: 1 of 4, three needed, nobody else voted
            for ( UInt32 i=0; i<4; ++i )
            {
                MULTIPLAY::CMatchDraftTally tally;
                REQUIRE_TRUE( f.boards[ i ].GetTally( kick.proposalId, T0 + 2000, tally ) );
                ASSERT_TRUE( 1 == tally.yes && 0 == tally.no && 3 == tally.needed && 4 == tally.eligible );
                ASSERT_TRUE( MULTIPLAY::MATCHDRAFTSTATE_OPEN == tally.state && 3 == tally.notVoted.size() );
            }
            ASSERT_TRUE( f.Cast( 0, kick, true, T0 + 2000 ) );
            ASSERT_TRUE( f.Cast( 2, kick, false, T0 + 3000 ) );
            MULTIPLAY::CMatchDraftTally mid;
            REQUIRE_TRUE( f.boards[ 3 ].GetTally( kick.proposalId, T0 + 3500, mid ) );
            ASSERT_TRUE( 2 == mid.yes && 1 == mid.no && MULTIPLAY::MATCHDRAFTSTATE_OPEN == mid.state );
            ASSERT_TRUE( f.Cast( 3, kick, true, T0 + 4000 ) );

            for ( UInt32 i=0; i<4; ++i )
            {
                MULTIPLAY::CMatchDraftTally tally;
                REQUIRE_TRUE( f.boards[ i ].GetTally( kick.proposalId, T0 + 5000, tally ) );
                ASSERT_TRUE( 3 == tally.yes && 1 == tally.no && MULTIPLAY::MATCHVOTEOUTCOME_PASSED == tally.outcome );
                ASSERT_TRUE( MULTIPLAY::MATCHDRAFTSTATE_PASSED == tally.state && 0 == tally.notVoted.size() && 1 == tally.noVoters.size() );
                ASSERT_TRUE( tally.yesVoters[ 0 ] == f.ids[ 1 ].userId );      // the proposer first
            }
            // the host sees it as a decision to carry out
            GUCEF::vector< MULTIPLAY::CMatchDraftTally > passed;
            ASSERT_TRUE( 1 == f.boards[ 0 ].GetPassedForApplication( T0 + 5000, passed ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A tie changes nothing, and a vote by somebody outside the electorate, a vote twice and a vote for another roster are refused" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDraftFixture f;
            REQUIRE_TRUE( f.Make() );
            f.table.push_back( f.Introduced( 1 ) );
            f.table.push_back( f.Introduced( 2 ) );
            f.table.push_back( f.Introduced( 3 ) );
            REQUIRE_TRUE( f.Publish( T0 ) );
            MULTIPLAY::CMatchProposal kick;
            REQUIRE_TRUE( f.Propose( 1, MULTIPLAY::MATCHDECISION_KICK, 3, T0 + 1000, kick ) );
            ASSERT_TRUE( f.Cast( 0, kick, true, T0 + 2000 ) );
            ASSERT_TRUE( f.Cast( 2, kick, false, T0 + 3000 ) );
            ASSERT_TRUE( f.Cast( 3, kick, false, T0 + 4000 ) );
            for ( UInt32 i=0; i<4; ++i )
            {
                MULTIPLAY::CMatchDraftTally tally;
                REQUIRE_TRUE( f.boards[ i ].GetTally( kick.proposalId, T0 + 5000, tally ) );
                ASSERT_TRUE( 2 == tally.yes && 2 == tally.no && MULTIPLAY::MATCHVOTEOUTCOME_FAILED == tally.outcome && MULTIPLAY::MATCHDRAFTSTATE_FAILED == tally.state );
            }
            GUCEF::vector< MULTIPLAY::CMatchDraftTally > passed;
            ASSERT_TRUE( 0 == f.boards[ 0 ].GetPassedForApplication( T0 + 5000, passed ) );

            CORE::CString reason;
            // a vote twice: the same again is a duplicate, another answer is a conflict, neither counts
            const MULTIPLAY::CMatchVote again = f.VoteOn( 2, kick, false, T0 + 3000 );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_DUPLICATE == f.boards[ 0 ].AddVote( again, T0 + 6000, reason ) );
            const MULTIPLAY::CMatchVote flipped = f.VoteOn( 2, kick, true, T0 + 7000 );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_CONFLICT == f.boards[ 0 ].AddVote( flipped, T0 + 7000, reason ) );
            MULTIPLAY::CMatchDraftTally tally;
            REQUIRE_TRUE( f.boards[ 0 ].GetTally( kick.proposalId, T0 + 8000, tally ) );
            ASSERT_TRUE( 2 == tally.yes && 2 == tally.no );

            // somebody who is not a participant
            MULTIPLAY::CPlayerIdentity stranger;
            REQUIRE_TRUE( TESTFW::MakeTestIdentity( "Stranger", stranger ) );
            MULTIPLAY::CMatchVote strange;
            strange.Sign( stranger, kick, true, T0 + 2500 );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_NOT_MEMBER == f.boards[ 0 ].AddVote( strange, T0 + 2500, reason ) );
            // a vote that is not signed by the voter it names
            MULTIPLAY::CMatchVote forged = f.VoteOn( 2, kick, true, T0 + 2600 );
            forged.voterUserId = f.ids[ 3 ].userId;
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_BAD_SIGNATURE == f.boards[ 0 ].AddVote( forged, T0 + 2600, reason ) );
            // a vote for a proposal that is not known
            MULTIPLAY::CMatchProposal unknown = kick;
            unknown.proposalId = "other";
            MULTIPLAY::CMatchVote stray;
            stray.Sign( f.ids[ 2 ], unknown, true, T0 + 2700 );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_PENDING == f.boards[ 0 ].AddVote( stray, T0 + 2700, reason ) );
            // an invitee who has not introduced itself has no key and no vote; neither has a spectator
            MULTIPLAY::CMatchVote late = f.VoteOn( 2, kick, false, T0 + 3000 );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_DUPLICATE == f.boards[ 1 ].AddVote( late, T0 + 3000, reason ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A proposal names the revision and the digest of a roster; another digest is refused and an unknown revision waits for its roster" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDraftFixture f;
            REQUIRE_TRUE( f.Make() );
            f.table.push_back( f.Introduced( 1 ) );
            f.table.push_back( f.Introduced( 2 ) );
            MULTIPLAY::CMatchRoster r1, r2;
            REQUIRE_TRUE( f.boards[ 0 ].PublishRoster( f.ids[ 0 ], f.table, f.policy, f.settingsDigest, T0, r1 ) );
            for ( UInt32 i=1; i<4; ++i )
                f.boards[ i ].AcceptRoster( r1, T0 );
            f.table.push_back( f.Introduced( 3 ) );
            REQUIRE_TRUE( f.boards[ 0 ].PublishRoster( f.ids[ 0 ], f.table, f.policy, f.settingsDigest, T0 + 10, r2 ) );
            // players 1 and 2 got the second roster, player 3 not yet
            f.boards[ 1 ].AcceptRoster( r2, T0 + 10 );
            f.boards[ 2 ].AcceptRoster( r2, T0 + 10 );
            f.boards[ 3 ].AcceptRoster( r1, T0 + 10 );

            CORE::CString reason;
            MULTIPLAY::CMatchProposal kick;
            REQUIRE_TRUE( f.boards[ 1 ].MakeProposal( f.ids[ 1 ], MULTIPLAY::MATCHDECISION_KICK, f.ids[ 2 ].userId, T0 + 100, 72 * Hour, kick, reason ) );
            ASSERT_TRUE( 2 == kick.rosterRevision && 4 == kick.voterUserIds.size() && kick.rosterDigest == r2.GetDigest() );

            // the same proposal naming the digest of the first roster for the second revision
            MULTIPLAY::CMatchProposal wrong = kick;
            wrong.rosterDigest = r1.GetDigest();
            wrong.proposalId = "wrong";
            REQUIRE_TRUE( wrong.Sign( f.ids[ 1 ] ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_ROSTER_MISMATCH == f.boards[ 2 ].AddProposal( wrong, T0 + 100, reason ) );
            // an electorate that is not the one of that roster
            MULTIPLAY::CMatchProposal small = kick;
            small.voterUserIds.pop_back();
            small.proposalId = "small";
            REQUIRE_TRUE( small.Sign( f.ids[ 1 ] ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_NOT_ALLOWED == f.boards[ 2 ].AddProposal( small, T0 + 100, reason ) );

            // player 3 has not got the second roster: the proposal waits
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_PENDING == f.boards[ 3 ].AddProposal( kick, T0 + 200, reason ) );
            ASSERT_TRUE( 1 == f.boards[ 3 ].GetPendingCount() );
            GUCEF::vector< MULTIPLAY::CMatchProposal > list;
            ASSERT_TRUE( 0 == f.boards[ 3 ].GetProposals( list ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_OK == f.boards[ 3 ].AcceptRoster( r2, T0 + 300 ) );
            ASSERT_TRUE( 0 == f.boards[ 3 ].GetPendingCount() && 1 == f.boards[ 3 ].GetProposals( list ) );
            MULTIPLAY::CMatchDraftTally tally;
            REQUIRE_TRUE( f.boards[ 3 ].GetTally( kick.proposalId, T0 + 400, tally ) );
            ASSERT_TRUE( 1 == tally.yes && 4 == tally.eligible && 3 == tally.needed );
            // a proposal repeated is a duplicate
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_DUPLICATE == f.boards[ 3 ].AddProposal( kick, T0 + 500, reason ) );

            // a vote before its proposal waits too
            MULTIPLAY::CMatchProposal second;
            REQUIRE_TRUE( f.boards[ 2 ].MakeProposal( f.ids[ 2 ], MULTIPLAY::MATCHDECISION_KICK, f.ids[ 1 ].userId, T0 + 600, 72 * Hour, second, reason ) );
            const MULTIPLAY::CMatchVote vote = f.VoteOn( 0, second, true, T0 + 700 );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_PENDING == f.boards[ 3 ].AddVote( vote, T0 + 700, reason ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_OK == f.boards[ 3 ].AddProposal( second, T0 + 800, reason ) );
            REQUIRE_TRUE( f.boards[ 3 ].GetTally( second.proposalId, T0 + 900, tally ) );
            ASSERT_TRUE( 2 == tally.yes && 4 == tally.eligible && MULTIPLAY::MATCHVOTEOUTCOME_UNDECIDED == tally.outcome );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Only a force start, a kick and the own spectating are decided in the draft stage; the host decides alone under BOSS" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDraftFixture f;
            REQUIRE_TRUE( f.Make() );
            f.table.push_back( f.Introduced( 1 ) );
            f.table.push_back( f.Introduced( 2 ) );
            f.table.push_back( f.Waiting( 3 ) );
            REQUIRE_TRUE( f.Publish( T0 ) );
            CORE::CString reason;

            // a force start replaces the participant who has not answered, which has no key and a placeholder id
            MULTIPLAY::CMatchProposal force;
            REQUIRE_TRUE( f.Propose( 1, MULTIPLAY::MATCHDECISION_FORCE_START, 0, T0 + 100, force ) );
            ASSERT_TRUE( 1 == force.targetUserIds.size() && MULTIPLAY::CMatchRosterEntry::IsPendingUserId( force.targetUserIds[ 0 ] ) );
            ASSERT_TRUE( 3 == force.voterUserIds.size() );
            ASSERT_TRUE( f.Cast( 0, force, true, T0 + 200 ) );
            MULTIPLAY::CMatchDraftTally tally;
            REQUIRE_TRUE( f.boards[ 2 ].GetTally( force.proposalId, T0 + 300, tally ) );
            ASSERT_TRUE( 2 == tally.yes && 2 == tally.needed && MULTIPLAY::MATCHDRAFTSTATE_PASSED == tally.state );

            // a participant cannot be handed to the computer or a game cancelled by a vote of the draft
            MULTIPLAY::CMatchRoster roster;
            REQUIRE_TRUE( f.boards[ 1 ].GetRoster( roster ) );
            MULTIPLAY::CMatchProposal hand;
            hand.matchId = f.matchId;
            hand.kind = MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER;
            hand.targetUserId = f.ids[ 2 ].userId;
            hand.createdAt = T0 + 400;
            hand.rosterRevision = roster.revision;
            hand.rosterDigest = roster.GetDigest();
            MULTIPLAY::CMatchInfo rosterInfo;
            roster.ToMatchInfo( rosterInfo );
            hand.SetElectorateFrom( rosterInfo );
            REQUIRE_TRUE( hand.Sign( f.ids[ 1 ] ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_NOT_ALLOWED == f.boards[ 0 ].AddProposal( hand, T0 + 400, reason ) );
            MULTIPLAY::CMatchProposal cancel = hand;
            cancel.kind = MULTIPLAY::MATCHDECISION_CANCEL;
            cancel.proposalId = "cancel";
            REQUIRE_TRUE( cancel.Sign( f.ids[ 1 ] ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_NOT_ALLOWED == f.boards[ 0 ].AddProposal( cancel, T0 + 400, reason ) );

            // the participant's own spectating: the own signature alone; another's spectating by a vote is refused
            MULTIPLAY::CMatchProposal watch;
            REQUIRE_TRUE( f.Propose( 2, MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR, 2, T0 + 500, watch ) );
            REQUIRE_TRUE( f.boards[ 0 ].GetTally( watch.proposalId, T0 + 600, tally ) );
            ASSERT_TRUE( MULTIPLAY::MATCHVOTEOUTCOME_PASSED == tally.outcome && MULTIPLAY::MATCHDRAFTSTATE_PASSED == tally.state );
            MULTIPLAY::CMatchProposal other;
            ASSERT_FALSE( f.boards[ 1 ].MakeProposal( f.ids[ 1 ], MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR, f.ids[ 2 ].userId, T0 + 700, 72 * Hour, other, reason ) );
            // nor kicks the creator or oneself
            ASSERT_FALSE( f.boards[ 1 ].MakeProposal( f.ids[ 1 ], MULTIPLAY::MATCHDECISION_KICK, f.ids[ 0 ].userId, T0 + 700, 72 * Hour, other, reason ) );
            ASSERT_FALSE( f.boards[ 1 ].MakeProposal( f.ids[ 1 ], MULTIPLAY::MATCHDECISION_KICK, f.ids[ 1 ].userId, T0 + 700, 72 * Hour, other, reason ) );
            // a force start with nobody to replace
            SDraftFixture g;
            REQUIRE_TRUE( g.Make() );
            g.table.push_back( g.Introduced( 1 ) );
            REQUIRE_TRUE( g.Publish( T0 ) );
            ASSERT_FALSE( g.boards[ 1 ].MakeProposal( g.ids[ 1 ], MULTIPLAY::MATCHDECISION_FORCE_START, CORE::CString(), T0 + 700, 72 * Hour, other, reason ) );

            // BOSS: the host decides alone, the invitees' proposals are not taken
            SDraftFixture b;
            REQUIRE_TRUE( b.Make( MULTIPLAY::MATCHDECISIONPOLICY_BOSS ) );
            b.table.push_back( b.Introduced( 1 ) );
            b.table.push_back( b.Introduced( 2 ) );
            REQUIRE_TRUE( b.Publish( T0 ) );
            MULTIPLAY::CMatchProposal boss;
            REQUIRE_TRUE( b.Propose( 0, MULTIPLAY::MATCHDECISION_KICK, 2, T0 + 100, boss ) );
            for ( UInt32 i=0; i<3; ++i )
            {
                REQUIRE_TRUE( b.boards[ i ].GetTally( boss.proposalId, T0 + 200, tally ) );
                ASSERT_TRUE( MULTIPLAY::MATCHVOTEOUTCOME_PASSED == tally.outcome );
            }
            MULTIPLAY::CMatchProposal invitee;
            const bool made = b.boards[ 1 ].MakeProposal( b.ids[ 1 ], MULTIPLAY::MATCHDECISION_KICK, b.ids[ 2 ].userId, T0 + 300, 72 * Hour, invitee, reason );
            if ( made )
                ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_NOT_ALLOWED == b.boards[ 0 ].AddProposal( invitee, T0 + 300, reason ) );
            else
                ASSERT_TRUE( !reason.IsNULLOrEmpty() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The host carries out a passed decision, names it with its evidence, and every participant sees it as applied; a kick resets nobody's answer" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDraftFixture f;
            REQUIRE_TRUE( f.Make() );
            f.table.push_back( f.Introduced( 1 ) );
            f.table.push_back( f.Introduced( 2 ) );
            f.table.push_back( f.Introduced( 3 ) );
            REQUIRE_TRUE( f.Publish( T0 ) );
            MULTIPLAY::CMatchProposal kick;
            REQUIRE_TRUE( f.Propose( 1, MULTIPLAY::MATCHDECISION_KICK, 3, T0 + 1000, kick ) );
            // player 2 never gets the votes of the others: only the roster that follows
            const MULTIPLAY::CMatchVote v0 = f.VoteOn( 0, kick, true, T0 + 2000 );
            const MULTIPLAY::CMatchVote v2 = f.VoteOn( 2, kick, true, T0 + 3000 );
            CORE::CString reason;
            REQUIRE_TRUE( MULTIPLAY::MATCHDRAFT_OK == f.boards[ 0 ].AddVote( v0, T0 + 2000, reason ) );
            REQUIRE_TRUE( MULTIPLAY::MATCHDRAFT_OK == f.boards[ 0 ].AddVote( v2, T0 + 3000, reason ) );
            f.boards[ 1 ].AddVote( v0, T0 + 2000, reason );
            f.boards[ 1 ].AddVote( v2, T0 + 3000, reason );
            f.boards[ 3 ].AddVote( v0, T0 + 2000, reason );
            f.boards[ 3 ].AddVote( v2, T0 + 3000, reason );
            f.boards[ 2 ].AddVote( v0, T0 + 2000, reason );

            GUCEF::vector< MULTIPLAY::CMatchDraftTally > passed;
            REQUIRE_TRUE( 1 == f.boards[ 0 ].GetPassedForApplication( T0 + 4000, passed ) );
            ASSERT_TRUE( passed[ 0 ].proposalId == kick.proposalId );
            REQUIRE_TRUE( f.boards[ 0 ].MarkApplied( kick.proposalId ) );
            ASSERT_TRUE( 0 == f.boards[ 0 ].GetPassedForApplication( T0 + 4000, passed ) );

            // the host removes the participant: the others keep their answer
            f.table.pop_back();
            MULTIPLAY::CMatchRoster r2;
            REQUIRE_TRUE( f.boards[ 0 ].PublishRoster( f.ids[ 0 ], f.table, f.policy, f.settingsDigest, T0 + 5000, r2 ) );
            ASSERT_TRUE( 2 == r2.revision && 1 == r2.evidence.size() && 1 == r2.appliedDigests.size() && r2.appliedDigests[ 0 ] == kick.GetDigest() );
            ASSERT_TRUE( 3 == r2.entries.size() && MULTIPLAY::MATCHPARTICIPANT_ACTIVE == r2.entries[ 1 ].status && MULTIPLAY::MATCHPARTICIPANT_ACTIVE == r2.entries[ 2 ].status );
            CORE::CString why;
            ASSERT_TRUE( r2.Verify( f.ids[ 0 ].publicKey, why ) );
            for ( UInt32 i=1; i<3; ++i )
            {
                ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_OK == f.boards[ i ].AcceptRoster( r2, T0 + 5000 ) );
                MULTIPLAY::CMatchRoster now;
                REQUIRE_TRUE( f.boards[ i ].GetRoster( now ) );
                ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_ACTIVE == now.entries[ 1 ].status && MULTIPLAY::MATCHPARTICIPANT_ACTIVE == now.entries[ 2 ].status );
                MULTIPLAY::CMatchDraftTally tally;
                REQUIRE_TRUE( f.boards[ i ].GetTally( kick.proposalId, T0 + 5001, tally ) );
                ASSERT_TRUE( tally.appliedByHost && MULTIPLAY::MATCHDRAFTSTATE_APPLIED == tally.state );
                ASSERT_TRUE( 3 == tally.yes );
            }
            // a second publish without a change publishes nothing: the evidence went out once
            MULTIPLAY::CMatchRoster none;
            ASSERT_FALSE( f.boards[ 0 ].PublishRoster( f.ids[ 0 ], f.table, f.policy, f.settingsDigest, T0 + 6000, none ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A host that does not carry out a passed decision is shown as ignoring it after the time it is given; a host that names a decision that did not pass is shown as miscounting" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDraftFixture f;
            REQUIRE_TRUE( f.Make() );
            f.table.push_back( f.Introduced( 1 ) );
            f.table.push_back( f.Introduced( 2 ) );
            f.table.push_back( f.Introduced( 3 ) );
            REQUIRE_TRUE( f.Publish( T0 ) );
            for ( UInt32 i=1; i<4; ++i )
                f.boards[ i ].SetApplyGraceMs( 2 * Hour );
            MULTIPLAY::CMatchProposal kick;
            REQUIRE_TRUE( f.Propose( 1, MULTIPLAY::MATCHDECISION_KICK, 3, T0 + 1000, kick ) );
            REQUIRE_TRUE( f.Cast( 0, kick, true, T0 + 2000 ) );
            REQUIRE_TRUE( f.Cast( 2, kick, true, T0 + 3000 ) );

            MULTIPLAY::CMatchDraftTally tally;
            REQUIRE_TRUE( f.boards[ 2 ].GetTally( kick.proposalId, T0 + 4000, tally ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFTSTATE_PASSED == tally.state && 0 != tally.passedSeenAt );
            // within the time: waiting; after it: ignored
            REQUIRE_TRUE( f.boards[ 2 ].GetTally( kick.proposalId, T0 + 3000 + Hour, tally ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFTSTATE_PASSED == tally.state );
            REQUIRE_TRUE( f.boards[ 2 ].GetTally( kick.proposalId, T0 + 3000 + 3 * Hour, tally ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFTSTATE_HOST_IGNORED == tally.state );
            // the host publishes something else and still ignores it
            f.table[ 1 ].displayName = "Renamed";
            MULTIPLAY::CMatchRoster r2;
            REQUIRE_TRUE( f.boards[ 0 ].PublishRoster( f.ids[ 0 ], f.table, f.policy, f.settingsDigest, T0 + 3 * Hour, r2 ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_OK == f.boards[ 2 ].AcceptRoster( r2, T0 + 3 * Hour ) );
            REQUIRE_TRUE( f.boards[ 2 ].GetTally( kick.proposalId, T0 + 3 * Hour + 1, tally ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFTSTATE_HOST_IGNORED == tally.state && !tally.appliedByHost );
            // and it is no evidence that the participant could have been removed: the roster still lists everybody
            ASSERT_TRUE( 4 == r2.entries.size() );

            // a decision that did not pass, named as carried out
            SDraftFixture g;
            REQUIRE_TRUE( g.Make() );
            g.table.push_back( g.Introduced( 1 ) );
            g.table.push_back( g.Introduced( 2 ) );
            g.table.push_back( g.Introduced( 3 ) );
            REQUIRE_TRUE( g.Publish( T0 ) );
            MULTIPLAY::CMatchProposal weak;
            REQUIRE_TRUE( g.Propose( 1, MULTIPLAY::MATCHDECISION_KICK, 3, T0 + 1000, weak ) );
            REQUIRE_TRUE( g.Cast( 2, weak, false, T0 + 2000 ) );
            REQUIRE_TRUE( g.boards[ 0 ].MarkApplied( weak.proposalId ) );
            // the host names it and still lists everybody (a roster that took the player off the table without a decision that passed is refused: the next case)
            g.table[ 3 ].displayName = "Renamed";
            MULTIPLAY::CMatchRoster claimed;
            REQUIRE_TRUE( g.boards[ 0 ].PublishRoster( g.ids[ 0 ], g.table, g.policy, g.settingsDigest, T0 + 3000, claimed ) );
            ASSERT_TRUE( 4 == claimed.entries.size() );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_OK == g.boards[ 2 ].AcceptRoster( claimed, T0 + 3000 ) );
            REQUIRE_TRUE( g.boards[ 2 ].GetTally( weak.proposalId, T0 + 3001, tally ) );
            ASSERT_TRUE( tally.appliedByHost && MULTIPLAY::MATCHDRAFTSTATE_HOST_MISCOUNT == tally.state );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Under VOTE the host does not take a player off the table alone: every participant refuses a roster that does it without a decision that passed, and accepts one that carries a passed decision about that player" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            CORE::CString reason;

            // the host removes a player who was introduced, with nothing behind it
            SDraftFixture f;
            REQUIRE_TRUE( f.Make() );
            f.table.push_back( f.Introduced( 1 ) );
            f.table.push_back( f.Introduced( 2 ) );
            f.table.push_back( f.Introduced( 3 ) );
            REQUIRE_TRUE( f.Publish( T0 ) );
            f.table.pop_back();
            MULTIPLAY::CMatchRoster alone;
            REQUIRE_TRUE( f.boards[ 0 ].PublishRoster( f.ids[ 0 ], f.table, f.policy, f.settingsDigest, T0 + 1000, alone ) );
            ASSERT_TRUE( 3 == alone.entries.size() && alone.VerifySignature() );
            for ( UInt32 i=1; i<4; ++i )
            {
                ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_UNAUTHORIZED_REMOVAL == f.boards[ i ].AcceptRoster( alone, T0 + 1000 ) );
                ASSERT_TRUE( 1 == f.boards[ i ].GetRevision() );
            }
            // what the host publishes after it lacks the player as well, so it is refused as long as nothing authorizes it
            f.table[ 1 ].displayName = "Renamed";
            MULTIPLAY::CMatchRoster later;
            REQUIRE_TRUE( f.boards[ 0 ].PublishRoster( f.ids[ 0 ], f.table, f.policy, f.settingsDigest, T0 + 2000, later ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_UNAUTHORIZED_REMOVAL == f.boards[ 1 ].AcceptRoster( later, T0 + 2000 ) && 1 == f.boards[ 1 ].GetRevision() );
            ASSERT_TRUE( f.boards[ 1 ].HasUnauthorizedRemoval() && !f.boards[ 0 ].HasUnauthorizedRemoval() );

            // the host proposes the kick itself and counts that as the decision: one yes of four is not a decision, and naming it does not make it one
            SDraftFixture h;
            REQUIRE_TRUE( h.Make() );
            h.table.push_back( h.Introduced( 1 ) );
            h.table.push_back( h.Introduced( 2 ) );
            h.table.push_back( h.Introduced( 3 ) );
            REQUIRE_TRUE( h.Publish( T0 ) );
            MULTIPLAY::CMatchProposal selfish;
            REQUIRE_TRUE( h.Propose( 0, MULTIPLAY::MATCHDECISION_KICK, 3, T0 + 100, selfish ) );
            MULTIPLAY::CMatchDraftTally tally;
            REQUIRE_TRUE( h.boards[ 0 ].GetTally( selfish.proposalId, T0 + 200, tally ) );
            ASSERT_TRUE( 1 == tally.yes && 3 == tally.needed && MULTIPLAY::MATCHVOTEOUTCOME_UNDECIDED == tally.outcome && MULTIPLAY::MATCHDRAFTSTATE_OPEN == tally.state );
            GUCEF::vector< MULTIPLAY::CMatchDraftTally > passedNow;
            ASSERT_TRUE( 0 == h.boards[ 0 ].GetPassedForApplication( T0 + 300, passedNow ) );
            REQUIRE_TRUE( h.boards[ 0 ].MarkApplied( selfish.proposalId ) );
            h.table.pop_back();
            MULTIPLAY::CMatchRoster forged;
            REQUIRE_TRUE( h.boards[ 0 ].PublishRoster( h.ids[ 0 ], h.table, h.policy, h.settingsDigest, T0 + 400, forged ) );
            ASSERT_TRUE( 1 == forged.evidence.size() && 3 == forged.entries.size() );
            for ( UInt32 i=1; i<4; ++i )
                ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_UNAUTHORIZED_REMOVAL == h.boards[ i ].AcceptRoster( forged, T0 + 400 ) );

            // a decision that passed, about another player than the one who was taken off the table, is no authority for this one
            SDraftFixture o;
            REQUIRE_TRUE( o.Make() );
            o.table.push_back( o.Introduced( 1 ) );
            o.table.push_back( o.Introduced( 2 ) );
            o.table.push_back( o.Introduced( 3 ) );
            REQUIRE_TRUE( o.Publish( T0 ) );
            MULTIPLAY::CMatchProposal kickThree;
            REQUIRE_TRUE( o.Propose( 1, MULTIPLAY::MATCHDECISION_KICK, 3, T0 + 100, kickThree ) );
            REQUIRE_TRUE( o.Cast( 0, kickThree, true, T0 + 200 ) );
            REQUIRE_TRUE( o.Cast( 2, kickThree, true, T0 + 300 ) );
            REQUIRE_TRUE( o.boards[ 0 ].MarkApplied( kickThree.proposalId ) );
            o.table.erase( o.table.begin() + 2 );                  // player 2 goes, not player 3
            MULTIPLAY::CMatchRoster wrongOne;
            REQUIRE_TRUE( o.boards[ 0 ].PublishRoster( o.ids[ 0 ], o.table, o.policy, o.settingsDigest, T0 + 400, wrongOne ) );
            for ( UInt32 i=1; i<4; ++i )
                ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_UNAUTHORIZED_REMOVAL == o.boards[ i ].AcceptRoster( wrongOne, T0 + 400 ) );

            // with the decision that passed, about that player, the same kind of roster is accepted by everybody (even by a participant who never saw the votes)
            SDraftFixture g;
            REQUIRE_TRUE( g.Make() );
            g.table.push_back( g.Introduced( 1 ) );
            g.table.push_back( g.Introduced( 2 ) );
            g.table.push_back( g.Introduced( 3 ) );
            REQUIRE_TRUE( g.Publish( T0 ) );
            MULTIPLAY::CMatchProposal kick;
            REQUIRE_TRUE( g.boards[ 1 ].MakeProposal( g.ids[ 1 ], MULTIPLAY::MATCHDECISION_KICK, g.ids[ 3 ].userId, T0 + 100, 72 * Hour, kick, reason ) );
            REQUIRE_TRUE( MULTIPLAY::MATCHDRAFT_OK == g.boards[ 1 ].AddProposal( kick, T0 + 100, reason ) );
            REQUIRE_TRUE( MULTIPLAY::MATCHDRAFT_OK == g.boards[ 0 ].AddProposal( kick, T0 + 100, reason ) );
            const MULTIPLAY::CMatchVote v0 = g.VoteOn( 0, kick, true, T0 + 200 );
            const MULTIPLAY::CMatchVote v2 = g.VoteOn( 2, kick, true, T0 + 300 );
            REQUIRE_TRUE( MULTIPLAY::MATCHDRAFT_OK == g.boards[ 0 ].AddVote( v0, T0 + 200, reason ) && MULTIPLAY::MATCHDRAFT_OK == g.boards[ 0 ].AddVote( v2, T0 + 300, reason ) );
            REQUIRE_TRUE( g.boards[ 0 ].MarkApplied( kick.proposalId ) );
            g.table.pop_back();
            MULTIPLAY::CMatchRoster lawful;
            REQUIRE_TRUE( g.boards[ 0 ].PublishRoster( g.ids[ 0 ], g.table, g.policy, g.settingsDigest, T0 + 400, lawful ) );
            ASSERT_TRUE( 1 == lawful.evidence.size() && 3 == lawful.entries.size() );
            for ( UInt32 i=1; i<3; ++i )
            {
                ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_OK == g.boards[ i ].AcceptRoster( lawful, T0 + 400 ) );
                ASSERT_TRUE( 2 == g.boards[ i ].GetRevision() && !g.boards[ i ].HasUnauthorizedRemoval() );
            }

            // a force start removes the players who did not answer, a player who declined can be taken off the table as housekeeping, and an unanswered player cannot
            SDraftFixture s;
            REQUIRE_TRUE( s.Make() );
            s.table.push_back( s.Introduced( 1 ) );
            s.table.push_back( s.Introduced( 2 ) );
            s.table.push_back( s.Waiting( 3 ) );
            REQUIRE_TRUE( s.Publish( T0 ) );
            MULTIPLAY::CMatchRoster silent;
            s.table.pop_back();
            REQUIRE_TRUE( s.boards[ 0 ].PublishRoster( s.ids[ 0 ], s.table, s.policy, s.settingsDigest, T0 + 100, silent ) );
            for ( UInt32 i=1; i<3; ++i )
                ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_UNAUTHORIZED_REMOVAL == s.boards[ i ].AcceptRoster( silent, T0 + 100 ) );
            SDraftFixture t;
            REQUIRE_TRUE( t.Make() );
            t.table.push_back( t.Introduced( 1 ) );
            t.table.push_back( t.Introduced( 2 ) );
            t.table.push_back( t.Waiting( 3 ) );
            REQUIRE_TRUE( t.Publish( T0 ) );
            MULTIPLAY::CMatchProposal force;
            REQUIRE_TRUE( t.Propose( 1, MULTIPLAY::MATCHDECISION_FORCE_START, 0, T0 + 100, force ) );
            REQUIRE_TRUE( t.Cast( 0, force, true, T0 + 200 ) );
            REQUIRE_TRUE( t.boards[ 0 ].MarkApplied( force.proposalId ) );
            t.table.pop_back();
            MULTIPLAY::CMatchRoster forced;
            REQUIRE_TRUE( t.boards[ 0 ].PublishRoster( t.ids[ 0 ], t.table, t.policy, t.settingsDigest, T0 + 300, forced ) );
            for ( UInt32 i=1; i<3; ++i )
                ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_OK == t.boards[ i ].AcceptRoster( forced, T0 + 300 ) && 2 == t.boards[ i ].GetRevision() );

            SDraftFixture d;
            REQUIRE_TRUE( d.Make() );
            d.table.push_back( d.Introduced( 1 ) );
            MULTIPLAY::CMatchRosterEntry declined = d.Waiting( 2 );
            declined.status = MULTIPLAY::MATCHPARTICIPANT_DECLINED;
            d.table.push_back( declined );
            REQUIRE_TRUE( d.Publish( T0 ) );
            d.table.pop_back();
            MULTIPLAY::CMatchRoster tidy;
            REQUIRE_TRUE( d.boards[ 0 ].PublishRoster( d.ids[ 0 ], d.table, d.policy, d.settingsDigest, T0 + 100, tidy ) );
            ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_OK == d.boards[ 1 ].AcceptRoster( tidy, T0 + 100 ) && 2 == d.boards[ 1 ].GetRevision() );

            // BOSS: the host is the boss and takes a player off the table without a vote
            SDraftFixture b;
            REQUIRE_TRUE( b.Make( MULTIPLAY::MATCHDECISIONPOLICY_BOSS ) );
            b.table.push_back( b.Introduced( 1 ) );
            b.table.push_back( b.Introduced( 2 ) );
            b.table.push_back( b.Introduced( 3 ) );
            REQUIRE_TRUE( b.Publish( T0 ) );
            b.table.pop_back();
            MULTIPLAY::CMatchRoster bossRoster;
            REQUIRE_TRUE( b.boards[ 0 ].PublishRoster( b.ids[ 0 ], b.table, b.policy, b.settingsDigest, T0 + 100, bossRoster ) );
            for ( UInt32 i=1; i<3; ++i )
                ASSERT_TRUE( MULTIPLAY::MATCHDRAFT_OK == b.boards[ i ].AcceptRoster( bossRoster, T0 + 100 ) && 2 == b.boards[ i ].GetRevision() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A board survives being stored and read back, with its rosters, proposals, votes and what the host carried out" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDraftFixture f;
            REQUIRE_TRUE( f.Make() );
            f.table.push_back( f.Introduced( 1 ) );
            f.table.push_back( f.Introduced( 2 ) );
            f.table.push_back( f.Waiting( 3 ) );
            REQUIRE_TRUE( f.Publish( T0 ) );
            MULTIPLAY::CMatchProposal force;
            REQUIRE_TRUE( f.Propose( 1, MULTIPLAY::MATCHDECISION_FORCE_START, 0, T0 + 100, force ) );
            REQUIRE_TRUE( f.Cast( 0, force, true, T0 + 200 ) );
            REQUIRE_TRUE( f.boards[ 0 ].MarkApplied( force.proposalId ) );
            f.table.pop_back();
            REQUIRE_TRUE( f.Publish( T0 + 300 ) );

            for ( UInt32 i=0; i<3; ++i )
            {
                MULTIPLAY::CMatchDraftBoard back;
                REQUIRE_TRUE( MatchJsonRoundTrip( f.boards[ i ], back, "draftboard" ) );
                ASSERT_TRUE( back.GetMatchId() == f.matchId && back.IsHost() == ( 0 == i ) );
                ASSERT_TRUE( back.GetRevision() == f.boards[ i ].GetRevision() );
                MULTIPLAY::CMatchDraftTally a, b;
                REQUIRE_TRUE( f.boards[ i ].GetTally( force.proposalId, T0 + 400, a ) );
                REQUIRE_TRUE( back.GetTally( force.proposalId, T0 + 400, b ) );
                ASSERT_TRUE( a.yes == b.yes && a.no == b.no && a.state == b.state && a.appliedByHost == b.appliedByHost );
                ASSERT_TRUE( MULTIPLAY::MATCHDRAFTSTATE_APPLIED == b.state );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
