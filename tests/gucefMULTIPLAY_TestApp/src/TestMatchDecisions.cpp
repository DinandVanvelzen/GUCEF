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

#include "TestMatchCommon.h"
#include "TestMatchDecisions.h"

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

/** A match of the given number of active participants, the first creates it */
struct SDecisionFixture
{
    GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
    MULTIPLAY::CMatchInfo info;

    bool Make( UInt32 count, MULTIPLAY::TMatchDecisionPolicy policy )
    {
        ids.clear();
        for ( UInt32 i=0; i<count; ++i )
        {
            MULTIPLAY::CPlayerIdentity identity;
            if ( !TESTFW::MakeTestIdentity( ( CORE::CString( "Player" ) + CORE::UInt32ToString( i ) ).C_String(), identity ) )
                return false;
            ids.push_back( identity );
        }
        info = TESTFW::MakeTestMatchInfo( ids, "decisions", true );
        info.decisionPolicy = policy;
        return true;
    }

    MULTIPLAY::CMatchProposal Propose( UInt32 proposer, MULTIPLAY::TMatchDecisionKind kind, UInt32 target, Int64 deadline = 0 ) const
    {
        MULTIPLAY::CMatchProposal proposal;
        proposal.matchId = info.matchId;
        proposal.kind = kind;
        proposal.targetUserId = ids[ target ].userId;
        proposal.createdAt = T0;
        proposal.deadline = deadline;
        proposal.Sign( ids[ proposer ] );
        return proposal;
    }

    MULTIPLAY::CMatchVote VoteOn( UInt32 voter, const MULTIPLAY::CMatchProposal& proposal, bool yes, Int64 when = T0 + 1000 ) const
    {
        MULTIPLAY::CMatchVote vote;
        vote.Sign( ids[ voter ], proposal, yes, when );
        return vote;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMatchDecisionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefMULTIPLAY matches: decisions" );

    GUCEF_TESTFW_TESTCASE( "Test: A decision needs more than half of the participants, as in a lobby, and the tally agrees with CLobbyRules" )
        try
        {
            for ( UInt32 players=1; players<=7; ++players )
            {
                MULTIPLAY::CLobbyState state;
                for ( UInt32 p=0; p<players; ++p )
                {
                    MULTIPLAY::CLobbyMember member;
                    member.id = CORE::CString( "m" ) + CORE::UInt32ToString( p );
                    member.role = MULTIPLAY::LOBBYMEMBERROLE_PLAYER;
                    state.members.push_back( member );
                }
                MULTIPLAY::CLobbyRules rules;
                ASSERT_TRUE( rules.GetVotesNeeded( state ) == MULTIPLAY::CMatchDecisionVerifier::GetVotesNeeded( players ) );
                ASSERT_TRUE( players / 2 + 1 == MULTIPLAY::CMatchDecisionVerifier::GetVotesNeeded( players ) );

                for ( UInt32 yes=1; yes<=players; ++yes )
                {
                    for ( UInt32 no=0; yes + no <= players; ++no )
                    {
                        state.activeVote.id = "vote";
                        state.activeVote.yes = yes;
                        state.activeVote.no = no;
                        state.activeVote.needed = rules.GetVotesNeeded( state );
                        MULTIPLAY::CLobbyRules::TVoteOutcome lobby = rules.EvaluateVote( state );
                        MULTIPLAY::TMatchVoteOutcome mine = MULTIPLAY::CMatchDecisionVerifier::EvaluateTally( yes, no, players );
                        ASSERT_TRUE( ( MULTIPLAY::CLobbyRules::VOTEOUTCOME_PASSED == lobby ) == ( MULTIPLAY::MATCHVOTEOUTCOME_PASSED == mine ) );
                        ASSERT_TRUE( ( MULTIPLAY::CLobbyRules::VOTEOUTCOME_FAILED == lobby ) == ( MULTIPLAY::MATCHVOTEOUTCOME_FAILED == mine ) );
                        ASSERT_TRUE( ( MULTIPLAY::CLobbyRules::VOTEOUTCOME_UNDECIDED == lobby ) == ( MULTIPLAY::MATCHVOTEOUTCOME_UNDECIDED == mine ) );
                    }
                }
            }
            // an equal number of yes and no changes nothing
            ASSERT_TRUE( MULTIPLAY::MATCHVOTEOUTCOME_FAILED == MULTIPLAY::CMatchDecisionVerifier::EvaluateTally( 2, 2, 4 ) );
            ASSERT_TRUE( MULTIPLAY::MATCHVOTEOUTCOME_FAILED == MULTIPLAY::CMatchDecisionVerifier::EvaluateTally( 1, 1, 2 ) );
            ASSERT_TRUE( MULTIPLAY::MATCHVOTEOUTCOME_PASSED == MULTIPLAY::CMatchDecisionVerifier::EvaluateTally( 3, 2, 5 ) );
            ASSERT_TRUE( MULTIPLAY::MATCHVOTEOUTCOME_UNDECIDED == MULTIPLAY::CMatchDecisionVerifier::EvaluateTally( 2, 1, 5 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A vote passes with the proposer and enough yes votes, a tie changes nothing, and it fails as soon as it cannot pass" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDecisionFixture f;
            REQUIRE_TRUE( f.Make( 5, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            MULTIPLAY::CMatchProposal proposal = f.Propose( 0, MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER, 4 );
            MULTIPLAY::CMatchDecisionEvidence evidence;
            evidence.proposal = proposal;
            MULTIPLAY::CMatchDecisionResult result;
            CORE::CString reason;

            // the proposer alone is a yes, three of five are needed
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Evaluate( f.info, evidence, T0, result ) );
            ASSERT_TRUE( result.valid && 1 == result.yes && 0 == result.no && 3 == result.needed && 5 == result.eligible );
            ASSERT_TRUE( MULTIPLAY::MATCHVOTEOUTCOME_UNDECIDED == result.outcome );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );

            evidence.votes.push_back( f.VoteOn( 1, proposal, true ) );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            evidence.votes.push_back( f.VoteOn( 2, proposal, false ) );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            MULTIPLAY::CMatchDecisionVerifier::Evaluate( f.info, evidence, T0, result );
            ASSERT_TRUE( 2 == result.yes && 1 == result.no && MULTIPLAY::MATCHVOTEOUTCOME_UNDECIDED == result.outcome );
            evidence.votes.push_back( f.VoteOn( 3, proposal, true ) );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            MULTIPLAY::CMatchDecisionVerifier::Evaluate( f.info, evidence, T0, result );
            ASSERT_TRUE( 3 == result.yes && 1 == result.no && MULTIPLAY::MATCHVOTEOUTCOME_PASSED == result.outcome );

            // after a JSON round trip the evidence is as good as before
            MULTIPLAY::CMatchDecisionEvidence back;
            REQUIRE_TRUE( MatchJsonRoundTrip( evidence, back, "evidence" ) );
            ASSERT_TRUE( 3 == back.votes.size() && MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, back, T0, reason ) );

            // it fails as soon as the votes that are left cannot pass it: two no votes of five leave 1 + 2 < 3
            MULTIPLAY::CMatchDecisionEvidence failing;
            failing.proposal = proposal;
            failing.votes.push_back( f.VoteOn( 1, proposal, false ) );
            failing.votes.push_back( f.VoteOn( 2, proposal, false ) );
            MULTIPLAY::CMatchDecisionVerifier::Evaluate( f.info, failing, T0, result );
            ASSERT_TRUE( result.valid && MULTIPLAY::MATCHVOTEOUTCOME_UNDECIDED == result.outcome );
            failing.votes.push_back( f.VoteOn( 3, proposal, false ) );
            MULTIPLAY::CMatchDecisionVerifier::Evaluate( f.info, failing, T0, result );
            ASSERT_TRUE( result.valid && MULTIPLAY::MATCHVOTEOUTCOME_FAILED == result.outcome );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, failing, T0, reason ) );

            // a tie changes nothing: two of four
            SDecisionFixture four;
            REQUIRE_TRUE( four.Make( 4, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            MULTIPLAY::CMatchProposal tieProposal = four.Propose( 0, MULTIPLAY::MATCHDECISION_DROP_PARTICIPANT, 3 );
            MULTIPLAY::CMatchDecisionEvidence tie;
            tie.proposal = tieProposal;
            tie.votes.push_back( four.VoteOn( 1, tieProposal, true ) );
            tie.votes.push_back( four.VoteOn( 2, tieProposal, false ) );
            tie.votes.push_back( four.VoteOn( 3, tieProposal, false ) );
            MULTIPLAY::CMatchDecisionVerifier::Evaluate( four.info, tie, T0, result );
            ASSERT_TRUE( result.valid && 2 == result.yes && 2 == result.no && MULTIPLAY::MATCHVOTEOUTCOME_FAILED == result.outcome );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( four.info, tie, T0, reason ) );

            // two participants: both have to agree
            SDecisionFixture two;
            REQUIRE_TRUE( two.Make( 2, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            MULTIPLAY::CMatchProposal p2 = two.Propose( 0, MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER, 1 );
            MULTIPLAY::CMatchDecisionEvidence e2;
            e2.proposal = p2;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( two.info, e2, T0, reason ) );
            e2.votes.push_back( two.VoteOn( 1, p2, true ) );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( two.info, e2, T0, reason ) );

            // alone in a match the proposer decides
            SDecisionFixture one;
            REQUIRE_TRUE( one.Make( 1, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            MULTIPLAY::CMatchDecisionEvidence e1;
            e1.proposal = one.Propose( 0, MULTIPLAY::MATCHDECISION_SETTING_CHANGE, 0 );
            e1.proposal.key = "turnLimit";
            e1.proposal.value = "7";
            e1.proposal.Sign( one.ids[ 0 ] );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( one.info, e1, T0, reason ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Every forged variant of the evidence of a vote is refused" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDecisionFixture f;
            REQUIRE_TRUE( f.Make( 5, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            MULTIPLAY::CMatchProposal proposal = f.Propose( 0, MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER, 4, T0 + 60000 );
            MULTIPLAY::CMatchDecisionEvidence good;
            good.proposal = proposal;
            good.votes.push_back( f.VoteOn( 1, proposal, true ) );
            good.votes.push_back( f.VoteOn( 2, proposal, true ) );
            CORE::CString reason;
            REQUIRE_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, good, T0 + 1000, reason ) );

            MULTIPLAY::CPlayerIdentity outsider;
            REQUIRE_TRUE( TESTFW::MakeTestIdentity( "Outsider", outsider ) );

            // a vote twice, the same vote copied
            MULTIPLAY::CMatchDecisionEvidence e = good;
            e.votes.push_back( good.votes[ 0 ] );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            // a participant who changed their mind: two signed votes
            e = good;
            e.votes.push_back( f.VoteOn( 1, proposal, false ) );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            // the proposer votes again, with a no
            e = good;
            e.votes.push_back( f.VoteOn( 0, proposal, false ) );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            // a yes of the proposer is the proposer's yes that counts already: once, not twice
            e = good;
            e.votes.clear();
            e.votes.push_back( f.VoteOn( 0, proposal, true ) );
            e.votes.push_back( f.VoteOn( 1, proposal, true ) );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            e.votes.push_back( f.VoteOn( 2, proposal, true ) );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            MULTIPLAY::CMatchDecisionResult counted;
            MULTIPLAY::CMatchDecisionVerifier::Evaluate( f.info, e, T0, counted );
            ASSERT_TRUE( 3 == counted.yes );
            e.votes.push_back( f.VoteOn( 0, proposal, true ) );                 // the proposer twice
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            // a vote of someone who is not a participant, signed with their own key
            e = good;
            e.votes.clear();
            e.votes.push_back( f.VoteOn( 1, proposal, true ) );
            MULTIPLAY::CMatchVote outsiderVote;
            outsiderVote.Sign( outsider, proposal, true, T0 + 1000 );
            e.votes.push_back( outsiderVote );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            // a vote that claims to be by a participant but is signed by another key
            e = good;
            MULTIPLAY::CMatchVote impersonation = f.VoteOn( 3, proposal, true );
            impersonation.voterUserId = f.ids[ 2 ].userId;
            e.votes.push_back( impersonation );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            // a vote whose answer was flipped after signing
            e = good;
            e.votes[ 1 ].yes = false;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            e = good;
            e.votes.clear();
            e.votes.push_back( f.VoteOn( 1, proposal, false ) );
            e.votes.push_back( f.VoteOn( 2, proposal, false ) );
            e.votes[ 0 ].yes = true;
            e.votes[ 1 ].yes = true;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            // a bad signature
            e = good;
            e.votes[ 0 ].signature = good.votes[ 0 ].signature.SubstrFromRange( 0, 127 ) + ( good.votes[ 0 ].signature[ 127 ] == '0' ? "1" : "0" );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            e = good;
            e.votes[ 0 ].signature.Clear();
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            // a vote for another proposal of the same match
            MULTIPLAY::CMatchProposal other = f.Propose( 0, MULTIPLAY::MATCHDECISION_DROP_PARTICIPANT, 4, T0 + 60000 );
            e = good;
            e.votes[ 1 ] = f.VoteOn( 2, other, true );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            // a vote with the proposal id of this proposal but made for different contents of it
            MULTIPLAY::CMatchProposal sameIdOtherContent = proposal;
            sameIdOtherContent.targetUserId = f.ids[ 3 ].userId;
            sameIdOtherContent.Sign( f.ids[ 0 ] );
            e = good;
            e.votes[ 1 ] = f.VoteOn( 2, sameIdOtherContent, true );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            // votes of another match, signed correctly for it
            SDecisionFixture otherMatch;
            REQUIRE_TRUE( otherMatch.Make( 5, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            otherMatch.ids = f.ids;
            otherMatch.info = f.info;
            otherMatch.info.matchId = MULTIPLAY::CMatchId::Generate();
            MULTIPLAY::CMatchProposal otherProposal = otherMatch.Propose( 0, MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER, 4, T0 + 60000 );
            MULTIPLAY::CMatchDecisionEvidence otherEvidence;
            otherEvidence.proposal = otherProposal;
            otherEvidence.votes.push_back( otherMatch.VoteOn( 1, otherProposal, true ) );
            otherEvidence.votes.push_back( otherMatch.VoteOn( 2, otherProposal, true ) );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( otherMatch.info, otherEvidence, T0, reason ) );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, otherEvidence, T0, reason ) );
            e = good;
            e.votes = otherEvidence.votes;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            // the context of the signature is the match id: a vote moved to this match with the id rewritten
            e = good;
            e.votes[ 1 ] = otherEvidence.votes[ 1 ];
            e.votes[ 1 ].matchId = f.info.matchId;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            // a vote cast after the end of the time of the proposal, and before it started
            e = good;
            e.votes[ 1 ] = f.VoteOn( 2, proposal, true, T0 + 60001 );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            e = good;
            e.votes[ 1 ] = f.VoteOn( 2, proposal, true, T0 - 1 );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            // a vote of a participant who does not vote: declined, left, still invited
            MULTIPLAY::CMatchInfo notVoting = f.info;
            notVoting.participants[ 2 ].status = MULTIPLAY::MATCHPARTICIPANT_DECLINED;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( notVoting, good, T0, reason ) );
            notVoting = f.info;
            notVoting.participants[ 2 ].status = MULTIPLAY::MATCHPARTICIPANT_INVITED;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( notVoting, good, T0, reason ) );
            // more votes than there are seats
            e = good;
            e.votes.clear();
            for ( int i=0; i<20; ++i )
                e.votes.push_back( good.votes[ 0 ] );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );

            // the proposal itself: changed after signing, of a non participant, of a participant who does not vote, for another match
            e = good;
            e.proposal.targetUserId = f.ids[ 3 ].userId;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            e = good;
            e.proposal.kind = MULTIPLAY::MATCHDECISION_DROP_PARTICIPANT;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            e = good;
            e.proposal.deadline = 0;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            e = good;
            e.proposal.proposerUserId = f.ids[ 1 ].userId;                      // claimed to be by another participant
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            MULTIPLAY::CMatchProposal byOutsider;
            byOutsider.matchId = f.info.matchId;
            byOutsider.kind = MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER;
            byOutsider.targetUserId = f.ids[ 4 ].userId;
            byOutsider.createdAt = T0;
            byOutsider.Sign( outsider );
            e = good;
            e.proposal = byOutsider;
            e.votes.clear();
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            MULTIPLAY::CMatchProposal toNobody = f.Propose( 0, MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER, 4, T0 + 60000 );
            toNobody.targetUserId = outsider.userId;
            toNobody.Sign( f.ids[ 0 ] );
            e.proposal = toNobody;
            e.votes.clear();
            e.votes.push_back( f.VoteOn( 1, toNobody, true ) );
            e.votes.push_back( f.VoteOn( 2, toNobody, true ) );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            MULTIPLAY::CMatchProposal wrongMatch = proposal;
            wrongMatch.matchId = MULTIPLAY::CMatchId::Generate();
            wrongMatch.Sign( f.ids[ 0 ] );
            e = good;
            e.proposal = wrongMatch;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            MULTIPLAY::CMatchProposal noId = proposal;
            noId.proposalId.Clear();
            e = good;
            e.proposal = noId;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            // a proposal that ends before it starts
            MULTIPLAY::CMatchProposal backwards = f.Propose( 0, MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER, 4, T0 - 5 );
            e.proposal = backwards;
            e.votes.clear();
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) );
            // a participant who does not vote cannot propose
            SDecisionFixture pending;
            REQUIRE_TRUE( pending.Make( 5, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            pending.info.participants[ 0 ].status = MULTIPLAY::MATCHPARTICIPANT_LEFT;
            MULTIPLAY::CMatchProposal byLeft = pending.Propose( 0, MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER, 4 );
            MULTIPLAY::CMatchDecisionEvidence leftEvidence;
            leftEvidence.proposal = byLeft;
            leftEvidence.votes.push_back( pending.VoteOn( 1, byLeft, true ) );
            leftEvidence.votes.push_back( pending.VoteOn( 2, byLeft, true ) );
            leftEvidence.votes.push_back( pending.VoteOn( 3, byLeft, true ) );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( pending.info, leftEvidence, T0, reason ) );
            // a participant who is already played by the computer cannot be handed over again
            MULTIPLAY::CMatchInfo already = f.info;
            already.participants[ 4 ].status = MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( already, good, T0, reason ) );
            // the unchanged evidence is still good
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, good, T0, reason ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A proposal that ran out of time fails, and a decision that passed in time stays good" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDecisionFixture f;
            REQUIRE_TRUE( f.Make( 5, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            const Int64 deadline = T0 + 60000;
            MULTIPLAY::CMatchProposal proposal = f.Propose( 0, MULTIPLAY::MATCHDECISION_DROP_PARTICIPANT, 4, deadline );
            MULTIPLAY::CMatchDecisionEvidence open;
            open.proposal = proposal;
            open.votes.push_back( f.VoteOn( 1, proposal, true ) );
            MULTIPLAY::CMatchDecisionResult result;
            CORE::CString reason;

            MULTIPLAY::CMatchDecisionVerifier::Evaluate( f.info, open, deadline, result );
            ASSERT_TRUE( result.valid && MULTIPLAY::MATCHVOTEOUTCOME_UNDECIDED == result.outcome );
            MULTIPLAY::CMatchDecisionVerifier::Evaluate( f.info, open, deadline + 1, result );
            ASSERT_TRUE( result.valid && MULTIPLAY::MATCHVOTEOUTCOME_FAILED == result.outcome );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, open, deadline + 1, reason ) );

            MULTIPLAY::CMatchDecisionEvidence passed = open;
            passed.votes.push_back( f.VoteOn( 2, proposal, true, deadline ) );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, passed, deadline, reason ) );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, passed, deadline + 86400000, reason ) );
            // no deadline: it stays open
            MULTIPLAY::CMatchProposal endless = f.Propose( 0, MULTIPLAY::MATCHDECISION_DROP_PARTICIPANT, 4, 0 );
            MULTIPLAY::CMatchDecisionEvidence endlessEvidence;
            endlessEvidence.proposal = endless;
            MULTIPLAY::CMatchDecisionVerifier::Evaluate( f.info, endlessEvidence, deadline * 100, result );
            ASSERT_TRUE( result.valid && MULTIPLAY::MATCHVOTEOUTCOME_UNDECIDED == result.outcome );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: In a boss match the signed command of a boss is the evidence and nobody else can issue one" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDecisionFixture f;
            REQUIRE_TRUE( f.Make( 5, MULTIPLAY::MATCHDECISIONPOLICY_BOSS ) );
            f.info.participants[ 1 ].boss = true;
            CORE::CString reason;

            MULTIPLAY::CMatchDecisionEvidence byBoss;
            byBoss.proposal = f.Propose( 1, MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER, 4 );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, byBoss, T0, reason ) );

            MULTIPLAY::CMatchDecisionEvidence byOther;
            byOther.proposal = f.Propose( 2, MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER, 4 );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, byOther, T0, reason ) );
            // votes do not make a participant a boss
            byOther.votes.push_back( f.VoteOn( 0, byOther.proposal, true ) );
            byOther.votes.push_back( f.VoteOn( 3, byOther.proposal, true ) );
            byOther.votes.push_back( f.VoteOn( 4, byOther.proposal, true ) );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, byOther, T0, reason ) );
            // the boss command with a changed target, or claimed by a non boss
            MULTIPLAY::CMatchDecisionEvidence tampered = byBoss;
            tampered.proposal.targetUserId = f.ids[ 3 ].userId;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, tampered, T0, reason ) );
            tampered = byBoss;
            tampered.proposal.proposerUserId = f.ids[ 2 ].userId;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, tampered, T0, reason ) );
            // the boss who is no longer in the match
            MULTIPLAY::CMatchInfo gone = f.info;
            gone.participants[ 1 ].status = MULTIPLAY::MATCHPARTICIPANT_LEFT;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( gone, byBoss, T0, reason ) );
            // the same evidence in a match that votes: a boss has no special right
            MULTIPLAY::CMatchInfo voting = f.info;
            voting.decisionPolicy = MULTIPLAY::MATCHDECISIONPOLICY_VOTE;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( voting, byBoss, T0, reason ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A player changes the own contact alone, whatever the policy, and nobody changes the contact of another" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            CORE::CString reason;
            for ( int policy=0; policy<2; ++policy )
            {
                SDecisionFixture f;
                REQUIRE_TRUE( f.Make( 4, (MULTIPLAY::TMatchDecisionPolicy) policy ) );
                MULTIPLAY::CMatchDecisionEvidence own;
                own.proposal.matchId = f.info.matchId;
                own.proposal.kind = MULTIPLAY::MATCHDECISION_CHANGE_CONTACT;
                own.proposal.targetUserId = f.ids[ 2 ].userId;
                own.proposal.value = "new.address@example.invalid";
                own.proposal.createdAt = T0;
                own.proposal.Sign( f.ids[ 2 ] );
                ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, own, T0, reason ) );

                MULTIPLAY::CMatchInfo applied = f.info;
                ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( applied, own.proposal, reason ) );
                ASSERT_TRUE( "new.address@example.invalid" == applied.participants[ 2 ].contact );
                ASSERT_TRUE( f.info.participants[ 2 ].contact != applied.participants[ 2 ].contact );

                // by another participant, even a boss, or a change of the address of someone else
                MULTIPLAY::CMatchDecisionEvidence other = own;
                other.proposal.targetUserId = f.ids[ 3 ].userId;
                other.proposal.Sign( f.ids[ 2 ] );
                ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, other, T0, reason ) );
                f.info.participants[ 0 ].boss = true;
                other.proposal.proposerUserId = f.ids[ 0 ].userId;
                other.proposal.Sign( f.ids[ 0 ] );
                ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, other, T0, reason ) );
                // a changed value after signing
                MULTIPLAY::CMatchDecisionEvidence changed = own;
                changed.proposal.value = "mallory@example.invalid";
                ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, changed, T0, reason ) );
                // too long
                MULTIPLAY::CMatchDecisionEvidence tooLong = own;
                for ( int i=0; i<40; ++i )
                    tooLong.proposal.value += "0123456789";
                tooLong.proposal.Sign( f.ids[ 2 ] );
                ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, tooLong, T0, reason ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A decision that passed is carried out on the participants and settings of the match" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDecisionFixture f;
            REQUIRE_TRUE( f.Make( 3, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            CORE::CString reason;

            MULTIPLAY::CMatchInfo info = f.info;
            MULTIPLAY::CMatchProposal hand = f.Propose( 0, MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER, 2 );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( info, hand, reason ) );
            ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER == info.participants[ 2 ].status );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( info, hand, reason ) );     // already
            ASSERT_TRUE( 2 == info.GetEligibleVoterCount() );                                              // the computer does not vote

            MULTIPLAY::CMatchProposal drop = f.Propose( 0, MULTIPLAY::MATCHDECISION_DROP_PARTICIPANT, 1 );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( info, drop, reason ) );
            ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_DROPPED == info.participants[ 1 ].status );
            ASSERT_TRUE( 1 == info.GetEligibleVoterCount() );

            MULTIPLAY::CMatchProposal setting = f.Propose( 0, MULTIPLAY::MATCHDECISION_SETTING_CHANGE, 0 );
            setting.key = "mailDistribution";
            setting.value = "ALL";
            setting.Sign( f.ids[ 0 ] );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( info, setting, reason ) );
            ASSERT_TRUE( "ALL" == info.GetSetting( "mailDistribution" ) );
            setting.key.Clear();
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( info, setting, reason ) );

            MULTIPLAY::CMatchProposal other = f.Propose( 0, MULTIPLAY::MATCHDECISION_DROP_PARTICIPANT, 1 );
            other.matchId = MULTIPLAY::CMatchId::Generate();
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( info, other, reason ) );
            MULTIPLAY::CMatchProposal nobody = f.Propose( 0, MULTIPLAY::MATCHDECISION_DROP_PARTICIPANT, 1 );
            nobody.targetUserId = "not-a-participant";
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( info, nobody, reason ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Evidence with bytes changed at random is never accepted when it was not good, and loops are bounded" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDecisionFixture f;
            REQUIRE_TRUE( f.Make( 5, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            MULTIPLAY::CMatchProposal proposal = f.Propose( 0, MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER, 4 );
            MULTIPLAY::CMatchDecisionEvidence good;
            good.proposal = proposal;
            good.votes.push_back( f.VoteOn( 1, proposal, true ) );
            good.votes.push_back( f.VoteOn( 2, proposal, true ) );
            CORE::CString reason;
            REQUIRE_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, good, T0, reason ) );

            // one text field of the proposal or a vote at a time is changed by one character
            CTestRandom random( 7 );
            UInt32 accepted = 0;
            for ( int round=0; round<400; ++round )
            {
                MULTIPLAY::CMatchDecisionEvidence e = good;
                UInt32 which = random.Next() % 9;
                CORE::CString* field = GUCEF_NULL;
                switch ( which )
                {
                    case 0: field = &e.proposal.proposalId; break;
                    case 1: field = &e.proposal.targetUserId; break;
                    case 2: field = &e.proposal.signature; break;
                    case 3: field = &e.votes[ 0 ].signature; break;
                    case 4: field = &e.votes[ 1 ].signature; break;
                    case 5: field = &e.votes[ 0 ].proposalDigest; break;
                    case 6: field = &e.votes[ 1 ].voterUserId; break;
                    case 7: field = &e.proposal.matchId; break;
                    default: field = &e.votes[ 0 ].proposalId; break;
                }
                if ( field->IsNULLOrEmpty() )
                    continue;
                UInt32 index = random.Next() % field->Length();
                char original = (*field)[ index ];
                field->C_String()[ index ] = ( 'z' == original ) ? 'y' : (char)( original + 1 );
                if ( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, e, T0, reason ) )
                    ++accepted;
            }
            ASSERT_TRUE( 0 == accepted );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The decisions before the start, the spectator status and the cancelled state have stable text forms and survive a round trip with their signatures" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            ASSERT_TRUE( CORE::CString( "FORCE_START" ) == MULTIPLAY::MatchDecisionKindToString( MULTIPLAY::MATCHDECISION_FORCE_START ) );
            ASSERT_TRUE( CORE::CString( "KICK" ) == MULTIPLAY::MatchDecisionKindToString( MULTIPLAY::MATCHDECISION_KICK ) );
            ASSERT_TRUE( CORE::CString( "CANCEL" ) == MULTIPLAY::MatchDecisionKindToString( MULTIPLAY::MATCHDECISION_CANCEL ) );
            ASSERT_TRUE( CORE::CString( "MOVE_TO_SPECTATOR" ) == MULTIPLAY::MatchDecisionKindToString( MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR ) );
            MULTIPLAY::TMatchDecisionKind kind = MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER;
            ASSERT_TRUE( MULTIPLAY::TryStringToMatchDecisionKind( "move_to_spectator", kind ) && MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR == kind );
            ASSERT_TRUE( CORE::CString( "SPECTATOR" ) == MULTIPLAY::MatchParticipantStatusToString( MULTIPLAY::MATCHPARTICIPANT_SPECTATOR ) );
            ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_SPECTATOR == MULTIPLAY::StringToMatchParticipantStatus( "SPECTATOR" ) );
            ASSERT_TRUE( CORE::CString( "CANCELLED" ) == MULTIPLAY::MatchStatusToString( MULTIPLAY::MATCHSTATUS_CANCELLED ) );
            ASSERT_TRUE( MULTIPLAY::MATCHSTATUS_CANCELLED == MULTIPLAY::StringToMatchStatus( "cancelled" ) );

            SDecisionFixture f;
            REQUIRE_TRUE( f.Make( 4, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            MULTIPLAY::CMatchProposal proposal;
            proposal.matchId = f.info.matchId;
            proposal.kind = MULTIPLAY::MATCHDECISION_FORCE_START;
            proposal.createdAt = T0;
            proposal.SetElectorateFrom( f.info );
            proposal.targetUserIds.push_back( f.ids[ 3 ].userId );
            REQUIRE_TRUE( proposal.Sign( f.ids[ 0 ] ) );
            MULTIPLAY::CMatchProposal back;
            REQUIRE_TRUE( MatchJsonRoundTrip( proposal, back, "proposal" ) );
            ASSERT_TRUE( 4 == back.voterUserIds.size() && 1 == back.targetUserIds.size() && back.VerifySignature( f.info ) && back.GetDigest() == proposal.GetDigest() );

            // the electorate and the targets are signed: changing either breaks the signature
            MULTIPLAY::CMatchProposal changed = proposal;
            changed.voterUserIds.pop_back();
            ASSERT_FALSE( changed.VerifySignature( f.info ) );
            changed = proposal;
            changed.targetUserIds.push_back( f.ids[ 2 ].userId );
            ASSERT_FALSE( changed.VerifySignature( f.info ) );

            // the reason a participant watches, and who closed the match, are part of the stored info
            MULTIPLAY::CMatchInfo info = f.info;
            info.participants[ 2 ].status = MULTIPLAY::MATCHPARTICIPANT_SPECTATOR;
            info.participants[ 2 ].statusCode = "SETTINGS";
            info.participants[ 2 ].statusReason = "The first turn does not begin as the settings say";
            info.status = MULTIPLAY::MATCHSTATUS_CANCELLED;
            info.closedByUserId = f.ids[ 0 ].userId;
            MULTIPLAY::CMatchInfo infoBack;
            REQUIRE_TRUE( MatchJsonRoundTrip( info, infoBack, "info" ) );
            ASSERT_TRUE( infoBack.participants[ 2 ].IsSpectator() && infoBack.participants[ 2 ].IsIncompatible() && "SETTINGS" == infoBack.participants[ 2 ].statusCode );
            ASSERT_TRUE( MULTIPLAY::MATCHSTATUS_CANCELLED == infoBack.status && infoBack.closedByUserId == f.ids[ 0 ].userId );
            // not signed: the record of the match is the same with or without them
            ASSERT_TRUE( info.GetDigest() == f.info.GetDigest() );
            // a spectator is not an eligible voter, so it is in no denominator
            ASSERT_TRUE( 3 == info.GetEligibleVoterCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A force start passes with more than half of the electorate that the proposal names, as in a lobby, and replaces the missing participants by the computer" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            // five at the table: three accepted, two did not answer
            SDecisionFixture f;
            REQUIRE_TRUE( f.Make( 5, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            f.info.status = MULTIPLAY::MATCHSTATUS_FORMING;
            f.info.participants[ 3 ].status = MULTIPLAY::MATCHPARTICIPANT_INVITED;
            f.info.participants[ 4 ].status = MULTIPLAY::MATCHPARTICIPANT_INVITED;

            MULTIPLAY::CMatchProposal proposal;
            proposal.matchId = f.info.matchId;
            proposal.kind = MULTIPLAY::MATCHDECISION_FORCE_START;
            proposal.createdAt = T0;
            proposal.SetElectorateFrom( f.info );
            proposal.targetUserIds.push_back( f.ids[ 3 ].userId );
            proposal.targetUserIds.push_back( f.ids[ 4 ].userId );
            REQUIRE_TRUE( proposal.Sign( f.ids[ 0 ] ) );
            ASSERT_TRUE( 3 == proposal.voterUserIds.size() );

            MULTIPLAY::CMatchDecisionEvidence evidence;
            evidence.proposal = proposal;
            MULTIPLAY::CMatchDecisionResult result;
            CORE::CString reason;

            // the proposer is a yes: one of three, two are needed
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Evaluate( f.info, evidence, T0, result ) );
            ASSERT_TRUE( result.valid && 1 == result.yes && 2 == result.needed && 3 == result.eligible && MULTIPLAY::MATCHVOTEOUTCOME_UNDECIDED == result.outcome );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );

            // a tie changes nothing: one yes and one no of two who accepted
            SDecisionFixture two;
            REQUIRE_TRUE( two.Make( 3, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            two.info.status = MULTIPLAY::MATCHSTATUS_FORMING;
            two.info.participants[ 2 ].status = MULTIPLAY::MATCHPARTICIPANT_INVITED;
            MULTIPLAY::CMatchProposal tieProposal;
            tieProposal.matchId = two.info.matchId;
            tieProposal.kind = MULTIPLAY::MATCHDECISION_FORCE_START;
            tieProposal.createdAt = T0;
            tieProposal.SetElectorateFrom( two.info );
            tieProposal.targetUserIds.push_back( two.ids[ 2 ].userId );
            REQUIRE_TRUE( tieProposal.Sign( two.ids[ 0 ] ) );
            MULTIPLAY::CMatchDecisionEvidence tie;
            tie.proposal = tieProposal;
            tie.votes.push_back( two.VoteOn( 1, tieProposal, false ) );
            MULTIPLAY::CMatchDecisionVerifier::Evaluate( two.info, tie, T0, result );
            ASSERT_TRUE( result.valid && 1 == result.yes && 1 == result.no && 2 == result.eligible && MULTIPLAY::MATCHVOTEOUTCOME_FAILED == result.outcome );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( two.info, tie, T0, reason ) );
            // the same table with the second yes passes
            tie.votes.clear();
            tie.votes.push_back( two.VoteOn( 1, tieProposal, true ) );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( two.info, tie, T0, reason ) );

            // a second vote makes three of three... the first plus one yes pass
            evidence.votes.push_back( f.VoteOn( 1, proposal, true ) );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );

            // a vote by somebody who did not accept when the proposal was made does not count: the evidence is refused as a whole
            MULTIPLAY::CMatchDecisionEvidence late = evidence;
            f.info.participants[ 3 ].status = MULTIPLAY::MATCHPARTICIPANT_ACTIVE;       // accepted afterwards
            late.votes.push_back( f.VoteOn( 3, proposal, true ) );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, late, T0, reason ) );
            // ... and the outcome of the good evidence does not depend on who accepted since: the denominator is the signed one
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            MULTIPLAY::CMatchDecisionVerifier::Evaluate( f.info, evidence, T0, result );
            ASSERT_TRUE( 3 == result.eligible && 2 == result.needed );
            f.info.participants[ 3 ].status = MULTIPLAY::MATCHPARTICIPANT_INVITED;

            // a vote cast by somebody who became a spectator afterwards stays counted
            SDecisionFixture four;
            REQUIRE_TRUE( four.Make( 4, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            four.info.status = MULTIPLAY::MATCHSTATUS_FORMING;
            four.info.participants[ 3 ].status = MULTIPLAY::MATCHPARTICIPANT_INVITED;
            MULTIPLAY::CMatchProposal p4;
            p4.matchId = four.info.matchId;
            p4.kind = MULTIPLAY::MATCHDECISION_FORCE_START;
            p4.createdAt = T0;
            p4.SetElectorateFrom( four.info );
            p4.targetUserIds.push_back( four.ids[ 3 ].userId );
            REQUIRE_TRUE( p4.Sign( four.ids[ 0 ] ) );
            MULTIPLAY::CMatchDecisionEvidence e4;
            e4.proposal = p4;
            e4.votes.push_back( four.VoteOn( 2, p4, true ) );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( four.info, e4, T0, reason ) );
            four.info.participants[ 2 ].status = MULTIPLAY::MATCHPARTICIPANT_SPECTATOR;
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( four.info, e4, T0, reason ) );
            MULTIPLAY::CMatchDecisionVerifier::Evaluate( four.info, e4, T0, result );
            ASSERT_TRUE( 3 == result.eligible && 2 == result.yes );

            // carrying it out: the named participants go to the computer, the others stay as they are; a kicked or watching one stays so
            MULTIPLAY::CMatchInfo applied = f.info;
            applied.participants[ 4 ].status = MULTIPLAY::MATCHPARTICIPANT_SPECTATOR;
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( applied, proposal, reason ) );
            ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER == applied.participants[ 3 ].status );
            ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_SPECTATOR == applied.participants[ 4 ].status );
            ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_ACTIVE == applied.participants[ 0 ].status && MULTIPLAY::MATCHPARTICIPANT_ACTIVE == applied.participants[ 1 ].status );

            // it is for the time before the start
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason, true ) );

            // a force start needs the electorate and targets that make sense
            MULTIPLAY::CMatchProposal noElectorate = proposal;
            noElectorate.voterUserIds.clear();
            noElectorate.Sign( f.ids[ 0 ] );
            MULTIPLAY::CMatchDecisionEvidence eNo;
            eNo.proposal = noElectorate;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, eNo, T0, reason ) );
            MULTIPLAY::CMatchProposal noTargets = proposal;
            noTargets.targetUserIds.clear();
            noTargets.Sign( f.ids[ 0 ] );
            eNo.proposal = noTargets;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, eNo, T0, reason ) );
            MULTIPLAY::CMatchProposal replacesVoter = proposal;
            replacesVoter.targetUserIds.push_back( f.ids[ 1 ].userId );
            replacesVoter.Sign( f.ids[ 0 ] );
            eNo.proposal = replacesVoter;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, eNo, T0, reason ) );
            MULTIPLAY::CMatchProposal replacesCreator = proposal;
            replacesCreator.targetUserIds.push_back( f.ids[ 0 ].userId );
            replacesCreator.Sign( f.ids[ 0 ] );
            eNo.proposal = replacesCreator;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, eNo, T0, reason ) );
            // an electorate with a stranger or the same participant twice
            MULTIPLAY::CMatchProposal twice = proposal;
            twice.voterUserIds.push_back( f.ids[ 1 ].userId );
            twice.Sign( f.ids[ 0 ] );
            eNo.proposal = twice;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, eNo, T0, reason ) );
            MULTIPLAY::CPlayerIdentity stranger;
            REQUIRE_TRUE( TESTFW::MakeTestIdentity( "Stranger", stranger ) );
            MULTIPLAY::CMatchProposal strange = proposal;
            strange.voterUserIds.push_back( stranger.userId );
            strange.Sign( f.ids[ 0 ] );
            eNo.proposal = strange;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, eNo, T0, reason ) );
            // the proposer is not in the electorate they name
            MULTIPLAY::CMatchProposal outside = proposal;
            outside.voterUserIds.erase( outside.voterUserIds.begin() );
            outside.Sign( f.ids[ 0 ] );
            eNo.proposal = outside;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, eNo, T0, reason ) );
            // a participant who did not accept proposes: not in the electorate
            MULTIPLAY::CMatchProposal byInvited = proposal;
            byInvited.targetUserIds.assign( 1, f.ids[ 4 ].userId );
            byInvited.Sign( f.ids[ 3 ] );
            eNo.proposal = byInvited;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, eNo, T0, reason ) );
            // a forged tally: the electorate shrunk after signing
            MULTIPLAY::CMatchDecisionEvidence shrunk = evidence;
            shrunk.proposal.voterUserIds.assign( 1, f.ids[ 0 ].userId );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, shrunk, T0, reason ) );
            // a vote that names another proposal
            MULTIPLAY::CMatchProposal other = proposal;
            other.proposalId = "";
            other.Sign( f.ids[ 0 ] );
            MULTIPLAY::CMatchDecisionEvidence wrongVote = evidence;
            wrongVote.votes[ 0 ] = f.VoteOn( 1, other, true );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, wrongVote, T0, reason ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Under the BOSS policy a boss forces the start, kicks or cancels alone; nobody else does" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDecisionFixture f;
            REQUIRE_TRUE( f.Make( 4, MULTIPLAY::MATCHDECISIONPOLICY_BOSS ) );
            f.info.status = MULTIPLAY::MATCHSTATUS_FORMING;
            f.info.participants[ 3 ].status = MULTIPLAY::MATCHPARTICIPANT_INVITED;
            f.info.participants[ 1 ].boss = true;                    // the second player is the boss, the creator is not
            CORE::CString reason;

            MULTIPLAY::CMatchProposal force;
            force.matchId = f.info.matchId;
            force.kind = MULTIPLAY::MATCHDECISION_FORCE_START;
            force.createdAt = T0;
            force.targetUserIds.push_back( f.ids[ 3 ].userId );
            MULTIPLAY::CMatchDecisionEvidence evidence;
            force.Sign( f.ids[ 1 ] );
            evidence.proposal = force;
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            force.Sign( f.ids[ 2 ] );                                // a player who is no boss
            evidence.proposal = force;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            force.Sign( f.ids[ 0 ] );                                // the creator is no boss either
            evidence.proposal = force;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );

            MULTIPLAY::CMatchProposal kick = f.Propose( 1, MULTIPLAY::MATCHDECISION_KICK, 2 );
            evidence.proposal = kick;
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            kick = f.Propose( 2, MULTIPLAY::MATCHDECISION_KICK, 1 );
            evidence.proposal = kick;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );

            // the creator and a boss close the match, nobody else
            MULTIPLAY::CMatchProposal cancel = f.Propose( 0, MULTIPLAY::MATCHDECISION_CANCEL, 0 );
            evidence.proposal = cancel;
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            cancel = f.Propose( 1, MULTIPLAY::MATCHDECISION_CANCEL, 0 );
            evidence.proposal = cancel;
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            cancel = f.Propose( 2, MULTIPLAY::MATCHDECISION_CANCEL, 0 );
            evidence.proposal = cancel;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            // a boss who is not among the participants of the match
            MULTIPLAY::CPlayerIdentity stranger;
            REQUIRE_TRUE( TESTFW::MakeTestIdentity( "Stranger", stranger ) );
            MULTIPLAY::CMatchProposal strange = f.Propose( 0, MULTIPLAY::MATCHDECISION_CANCEL, 0 );
            strange.Sign( stranger );
            evidence.proposal = strange;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A kick before the start is a vote like any other, never of the creator or the proposer, and the participant is gone" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDecisionFixture f;
            REQUIRE_TRUE( f.Make( 4, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            f.info.status = MULTIPLAY::MATCHSTATUS_FORMING;
            CORE::CString reason;
            MULTIPLAY::CMatchProposal kick = f.Propose( 1, MULTIPLAY::MATCHDECISION_KICK, 3 );
            kick.SetElectorateFrom( f.info );
            REQUIRE_TRUE( kick.Sign( f.ids[ 1 ] ) );
            MULTIPLAY::CMatchDecisionEvidence evidence;
            evidence.proposal = kick;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            evidence.votes.push_back( f.VoteOn( 2, kick, true ) );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );          // two of four is a tie
            evidence.votes.push_back( f.VoteOn( 0, kick, true ) );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );

            // the kicked one may vote against; the outcome stands
            MULTIPLAY::CMatchDecisionEvidence against = evidence;
            against.votes.push_back( f.VoteOn( 3, kick, false ) );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, against, T0, reason ) );

            MULTIPLAY::CMatchInfo applied = f.info;
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( applied, kick, reason ) );
            ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_DROPPED == applied.participants[ 3 ].status && MULTIPLAY::MATCHPARTICIPANT_ACTIVE == applied.participants[ 2 ].status );
            ASSERT_TRUE( 3 == applied.GetEligibleVoterCount() );

            // never the creator, never oneself
            MULTIPLAY::CMatchProposal kickCreator = f.Propose( 1, MULTIPLAY::MATCHDECISION_KICK, 0 );
            kickCreator.SetElectorateFrom( f.info );
            kickCreator.Sign( f.ids[ 1 ] );
            MULTIPLAY::CMatchDecisionEvidence eC;
            eC.proposal = kickCreator;
            eC.votes.push_back( f.VoteOn( 2, kickCreator, true ) );
            eC.votes.push_back( f.VoteOn( 3, kickCreator, true ) );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, eC, T0, reason ) );
            MULTIPLAY::CMatchProposal kickSelf = f.Propose( 1, MULTIPLAY::MATCHDECISION_KICK, 1 );
            kickSelf.SetElectorateFrom( f.info );
            kickSelf.Sign( f.ids[ 1 ] );
            eC.proposal = kickSelf;
            eC.votes.clear();
            eC.votes.push_back( f.VoteOn( 2, kickSelf, true ) );
            eC.votes.push_back( f.VoteOn( 3, kickSelf, true ) );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, eC, T0, reason ) );

            // a non participant cannot propose, a spectator is not in the electorate
            MULTIPLAY::CPlayerIdentity stranger;
            REQUIRE_TRUE( TESTFW::MakeTestIdentity( "Stranger", stranger ) );
            MULTIPLAY::CMatchProposal byStranger = kick;
            byStranger.Sign( stranger );
            eC.proposal = byStranger;
            eC.votes.clear();
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, eC, T0, reason ) );
            f.info.participants[ 2 ].status = MULTIPLAY::MATCHPARTICIPANT_SPECTATOR;
            MULTIPLAY::CMatchProposal bySpectator = f.Propose( 2, MULTIPLAY::MATCHDECISION_KICK, 3 );
            bySpectator.SetElectorateFrom( f.info );
            bySpectator.Sign( f.ids[ 2 ] );
            eC.proposal = bySpectator;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, eC, T0, reason ) );
            // once the match started nobody is kicked this way
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason, true ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Under VOTE closing the match is a vote like any other, also for the creator; a cancelled match accepts no further decision" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDecisionFixture f;
            REQUIRE_TRUE( f.Make( 3, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            f.info.status = MULTIPLAY::MATCHSTATUS_FORMING;
            f.info.participants[ 2 ].status = MULTIPLAY::MATCHPARTICIPANT_INVITED;
            f.info.participants[ 1 ].boss = true;           // a boss flag means nothing under VOTE
            CORE::CString reason;
            MULTIPLAY::CMatchDecisionEvidence evidence;

            // the creator alone, with or without the electorate, is not a decision while somebody else takes part
            evidence.proposal = f.Propose( 0, MULTIPLAY::MATCHDECISION_CANCEL, 0 );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            MULTIPLAY::CMatchProposal cancel = f.Propose( 0, MULTIPLAY::MATCHDECISION_CANCEL, 0 );
            cancel.SetElectorateFrom( f.info );
            REQUIRE_TRUE( cancel.Sign( f.ids[ 0 ] ) );
            evidence.proposal = cancel;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );          // one of two is a tie
            evidence.votes.push_back( f.VoteOn( 1, cancel, false ) );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            evidence.votes.clear();
            evidence.votes.push_back( f.VoteOn( 1, cancel, true ) );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );

            // anybody who takes part proposes it, the creator votes; a player who has not answered is outside the electorate and proposes nothing
            MULTIPLAY::CMatchProposal byBob = f.Propose( 1, MULTIPLAY::MATCHDECISION_CANCEL, 0 );
            byBob.SetElectorateFrom( f.info );
            REQUIRE_TRUE( byBob.Sign( f.ids[ 1 ] ) );
            evidence.proposal = byBob;
            evidence.votes.clear();
            evidence.votes.push_back( f.VoteOn( 0, byBob, true ) );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            MULTIPLAY::CMatchProposal byCarol = f.Propose( 2, MULTIPLAY::MATCHDECISION_CANCEL, 0 );
            byCarol.SetElectorateFrom( f.info );
            REQUIRE_TRUE( byCarol.Sign( f.ids[ 2 ] ) );
            evidence.proposal = byCarol;
            evidence.votes.clear();
            evidence.votes.push_back( f.VoteOn( 0, byCarol, true ) );
            evidence.votes.push_back( f.VoteOn( 1, byCarol, true ) );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );

            // the creator is the only one who accepted: the electorate is one person and the proposer counts as the yes, as in a lobby of one
            SDecisionFixture alone;
            REQUIRE_TRUE( alone.Make( 3, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            alone.info.status = MULTIPLAY::MATCHSTATUS_FORMING;
            alone.info.participants[ 1 ].status = MULTIPLAY::MATCHPARTICIPANT_INVITED;
            alone.info.participants[ 2 ].status = MULTIPLAY::MATCHPARTICIPANT_INVITED;
            MULTIPLAY::CMatchProposal soleVoter = alone.Propose( 0, MULTIPLAY::MATCHDECISION_CANCEL, 0 );
            soleVoter.SetElectorateFrom( alone.info );
            REQUIRE_TRUE( soleVoter.Sign( alone.ids[ 0 ] ) );
            ASSERT_TRUE( 1 == soleVoter.voterUserIds.size() );
            evidence.proposal = soleVoter;
            evidence.votes.clear();
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( alone.info, evidence, T0, reason ) );

            // carrying it out records who closed the match
            MULTIPLAY::CMatchInfo applied = f.info;
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( applied, cancel, reason ) );
            ASSERT_TRUE( MULTIPLAY::MATCHSTATUS_CANCELLED == applied.status && applied.closedByUserId == f.ids[ 0 ].userId );

            // after it nothing else is decided
            evidence.proposal = cancel;
            evidence.votes.clear();
            evidence.votes.push_back( f.VoteOn( 1, cancel, true ) );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( applied, evidence, T0, reason ) );
            // once the game started nobody cancels this way
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason, true ) );
            // a signature of the creator for another match is no cancel here
            SDecisionFixture other;
            REQUIRE_TRUE( other.Make( 3, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            other.ids = f.ids;
            other.info.creatorUserId = f.ids[ 0 ].userId;
            evidence.proposal = other.Propose( 0, MULTIPLAY::MATCHDECISION_CANCEL, 0 );
            evidence.votes.clear();
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Under VOTE a decision that claims the authority of the host alone is refused by everybody, whatever the kind; under BOSS the same command of the boss is valid" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            CORE::CString reason;
            for ( UInt32 mode=0; mode<2; ++mode )
            {
                const bool boss = 1 == mode;
                SDecisionFixture f;
                REQUIRE_TRUE( f.Make( 4, boss ? MULTIPLAY::MATCHDECISIONPOLICY_BOSS : MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
                f.info.status = MULTIPLAY::MATCHSTATUS_FORMING;
                f.info.participants[ 3 ].status = MULTIPLAY::MATCHPARTICIPANT_INVITED;
                f.info.participants[ 0 ].boss = boss;           // the host is the boss of a BOSS match
                MULTIPLAY::CMatchDecisionEvidence evidence;

                // a kick that names no electorate and carries no vote: the host signed it, nobody else did
                MULTIPLAY::CMatchProposal kick = f.Propose( 0, MULTIPLAY::MATCHDECISION_KICK, 2 );
                evidence.proposal = kick;
                ASSERT_TRUE( boss == MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
                // the same with the electorate named: one yes of three is not enough while the others did not say yes
                kick.SetElectorateFrom( f.info );
                REQUIRE_TRUE( kick.Sign( f.ids[ 0 ] ) );
                evidence.proposal = kick;
                ASSERT_TRUE( boss == MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
                // and with a no from the other: the host cannot carry it
                evidence.votes.push_back( f.VoteOn( 1, kick, false ) );
                ASSERT_TRUE( boss == MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
                // a vote cannot be made up: the signature of the host on a vote of another player
                evidence.votes.clear();
                MULTIPLAY::CMatchVote made = f.VoteOn( 1, kick, true );
                made.voterUserId = f.ids[ 1 ].userId;
                made.signature = f.VoteOn( 0, kick, true ).signature;
                evidence.votes.push_back( made );
                ASSERT_TRUE( boss == MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );

                // a force start of the host alone
                MULTIPLAY::CMatchProposal force;
                force.matchId = f.info.matchId;
                force.kind = MULTIPLAY::MATCHDECISION_FORCE_START;
                force.createdAt = T0;
                force.targetUserIds.push_back( f.ids[ 3 ].userId );
                force.Sign( f.ids[ 0 ] );
                evidence.proposal = force;
                evidence.votes.clear();
                ASSERT_TRUE( boss == MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
                force.SetElectorateFrom( f.info );
                force.Sign( f.ids[ 0 ] );
                evidence.proposal = force;
                ASSERT_TRUE( boss == MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );

                // closing the match
                MULTIPLAY::CMatchProposal cancel = f.Propose( 0, MULTIPLAY::MATCHDECISION_CANCEL, 0 );
                evidence.proposal = cancel;
                ASSERT_TRUE( boss == MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
                cancel.SetElectorateFrom( f.info );
                cancel.Sign( f.ids[ 0 ] );
                evidence.proposal = cancel;
                ASSERT_TRUE( boss == MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );

                // moving another participant to the spectators
                MULTIPLAY::CMatchProposal watch = f.Propose( 0, MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR, 2 );
                watch.key = "REMOVED";
                watch.value = "moved by the host";
                watch.Sign( f.ids[ 0 ] );
                evidence.proposal = watch;
                ASSERT_TRUE( boss == MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
                watch.SetElectorateFrom( f.info );
                watch.Sign( f.ids[ 0 ] );
                evidence.proposal = watch;
                ASSERT_TRUE( boss == MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );

                // the table agrees with the host: under VOTE that is a decision, under BOSS the vote is not needed
                kick.Sign( f.ids[ 0 ] );
                evidence.proposal = kick;
                evidence.votes.clear();
                evidence.votes.push_back( f.VoteOn( 1, kick, true ) );
                ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A move to spectator is the participant's own signature alone, or the table's rule; a stranger's notice is refused; a spectator is outside every denominator" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDecisionFixture f;
            REQUIRE_TRUE( f.Make( 4, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            f.info.status = MULTIPLAY::MATCHSTATUS_FORMING;
            f.info.participants[ 2 ].status = MULTIPLAY::MATCHPARTICIPANT_INVITED;
            f.info.participants[ 3 ].status = MULTIPLAY::MATCHPARTICIPANT_INVITED;
            CORE::CString reason;

            MULTIPLAY::CMatchProposal self = f.Propose( 2, MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR, 2 );
            self.key = "SETTINGS";
            self.value = "The first turn does not begin as the settings say";
            REQUIRE_TRUE( self.Sign( f.ids[ 2 ] ) );
            MULTIPLAY::CMatchDecisionEvidence evidence;
            evidence.proposal = self;
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );

            // the person alone: the same words signed by somebody else are a proposal of the table
            MULTIPLAY::CMatchProposal forged = self;
            forged.Sign( f.ids[ 3 ] );
            evidence.proposal = forged;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            // a notice that names another person than the signer, edited after signing
            MULTIPLAY::CMatchProposal edited = self;
            edited.targetUserId = f.ids[ 3 ].userId;
            evidence.proposal = edited;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            // the creator hosts the match
            MULTIPLAY::CMatchProposal creator = f.Propose( 0, MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR, 0 );
            creator.key = "SETTINGS";
            creator.Sign( f.ids[ 0 ] );
            evidence.proposal = creator;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            // reason codes and words are bounded and readable
            const char* badCodes[] = { "", "lower", "WITH SPACE", "TOO_LONG_TOO_LONG_TOO_LONG_TOO_LONG_X" };
            for ( int i=0; i<4; ++i )
            {
                MULTIPLAY::CMatchProposal bad = self;
                bad.key = badCodes[ i ];
                bad.Sign( f.ids[ 2 ] );
                evidence.proposal = bad;
                ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            }
            MULTIPLAY::CMatchProposal longWords = self;
            longWords.value.Clear();
            for ( int i=0; i<129; ++i )
                longWords.value += "x";
            longWords.Sign( f.ids[ 2 ] );
            evidence.proposal = longWords;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            MULTIPLAY::CMatchProposal control = self;
            control.value = "line\nbreak";
            control.Sign( f.ids[ 2 ] );
            evidence.proposal = control;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason ) );
            // once the game started this is not the way
            evidence.proposal = self;
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason, true ) );

            // carrying it out: a spectator with the reason
            MULTIPLAY::CMatchInfo applied = f.info;
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( applied, self, reason ) );
            ASSERT_TRUE( applied.participants[ 2 ].IsSpectator() && applied.participants[ 2 ].IsIncompatible() && "SETTINGS" == applied.participants[ 2 ].statusCode );
            // a person who chose to watch is a spectator without being incompatible
            MULTIPLAY::CMatchProposal chosen = self;
            chosen.key = GUCEF_MULTIPLAY_SPECTATOR_REASON_VOLUNTARY;
            chosen.value = "";
            chosen.Sign( f.ids[ 2 ] );
            MULTIPLAY::CMatchInfo watching = f.info;
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( watching, chosen, reason ) );
            ASSERT_TRUE( watching.participants[ 2 ].IsSpectator() && !watching.participants[ 2 ].IsIncompatible() );

            // a spectator is outside the denominator: three at the table, two of them vote
            ASSERT_TRUE( 2 == applied.GetEligibleVoterCount() );
            MULTIPLAY::CMatchProposal kick = f.Propose( 1, MULTIPLAY::MATCHDECISION_KICK, 3 );
            kick.SetElectorateFrom( applied );
            kick.Sign( f.ids[ 1 ] );
            MULTIPLAY::CMatchDecisionEvidence eK;
            eK.proposal = kick;
            MULTIPLAY::CMatchDecisionResult result;
            MULTIPLAY::CMatchDecisionVerifier::Evaluate( applied, eK, T0, result );
            ASSERT_TRUE( result.valid && 2 == result.eligible && 2 == result.needed );
            // and a spectator does not vote
            eK.votes.push_back( f.VoteOn( 2, kick, true ) );
            ASSERT_FALSE( MULTIPLAY::CMatchDecisionVerifier::Verify( applied, eK, T0, reason ) );

            // precedence on every machine, whatever order the decisions arrive: a kicked participant stays kicked, a watching one is not replaced again
            MULTIPLAY::CMatchInfo kicked = applied;
            MULTIPLAY::CMatchProposal kickTwo = f.Propose( 1, MULTIPLAY::MATCHDECISION_KICK, 2 );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( kicked, kickTwo, reason ) );
            ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_DROPPED == kicked.participants[ 2 ].status );
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( kicked, self, reason ) );
            ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_DROPPED == kicked.participants[ 2 ].status );
            // a move after a force start (arrived later) still makes a spectator
            MULTIPLAY::CMatchInfo replaced = f.info;
            replaced.participants[ 2 ].status = MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER;
            ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( replaced, self, reason ) );
            ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_SPECTATOR == replaced.participants[ 2 ].status );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The tally of a force start agrees with the votes needed in a lobby for every table size" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            for ( UInt32 players=2; players<=6; ++players )
            {
                SDecisionFixture f;
                REQUIRE_TRUE( f.Make( players + 1, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
                f.info.status = MULTIPLAY::MATCHSTATUS_FORMING;
                f.info.participants[ players ].status = MULTIPLAY::MATCHPARTICIPANT_INVITED;
                MULTIPLAY::CMatchProposal proposal;
                proposal.matchId = f.info.matchId;
                proposal.kind = MULTIPLAY::MATCHDECISION_FORCE_START;
                proposal.createdAt = T0;
                proposal.SetElectorateFrom( f.info );
                proposal.targetUserIds.push_back( f.ids[ players ].userId );
                REQUIRE_TRUE( proposal.Sign( f.ids[ 0 ] ) );
                MULTIPLAY::CMatchDecisionEvidence evidence;
                evidence.proposal = proposal;
                MULTIPLAY::CLobbyState state;
                for ( UInt32 p=0; p<players; ++p )
                {
                    MULTIPLAY::CLobbyMember member;
                    member.id = CORE::CString( "m" ) + CORE::UInt32ToString( p );
                    member.role = MULTIPLAY::LOBBYMEMBERROLE_PLAYER;
                    state.members.push_back( member );
                }
                MULTIPLAY::CLobbyRules rules;
                // the proposer and each yes in turn: it passes exactly when a lobby would pass it
                for ( UInt32 yes=1; yes<=players; ++yes )
                {
                    evidence.votes.clear();
                    for ( UInt32 v=1; v<yes; ++v )
                        evidence.votes.push_back( f.VoteOn( v, proposal, true ) );
                    CORE::CString reason;
                    const bool passed = MULTIPLAY::CMatchDecisionVerifier::Verify( f.info, evidence, T0, reason );
                    ASSERT_TRUE( passed == ( yes >= rules.GetVotesNeeded( state ) ) );
                }
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A spectator signs no turn and is never given one" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SDecisionFixture f;
            REQUIRE_TRUE( f.Make( 3, MULTIPLAY::MATCHDECISIONPOLICY_VOTE ) );
            CORE::CString reason;
            MULTIPLAY::CMatchPackage p1 = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "one" );
            MULTIPLAY::CMatchPreview r1 = TESTFW::MakeTestPreview( f.info, f.ids[ 0 ], p1, &f.ids[ 1 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 1000 );
            MULTIPLAY::CMatchPackage p2 = TESTFW::MakeTestPackage( 2, &p1, "two" );
            MULTIPLAY::CMatchPreview r2 = TESTFW::MakeTestPreview( f.info, f.ids[ 1 ], p2, &f.ids[ 2 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 2000 );
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_OK == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( f.info, &r1, r2, &p2, reason ) );

            // the signer watches: its turn is refused even though the preview of the trusted turn names it
            MULTIPLAY::CMatchInfo watching = f.info;
            watching.participants[ 1 ].status = MULTIPLAY::MATCHPARTICIPANT_SPECTATOR;
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_NOT_ON_TURN == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( watching, &r1, r2, &p2, reason ) );
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_NOT_ON_TURN == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( watching, &r1, r2, &p2, reason, true ) );

            // a preview that gives the turn to a spectator
            watching = f.info;
            watching.participants[ 2 ].status = MULTIPLAY::MATCHPARTICIPANT_SPECTATOR;
            ASSERT_TRUE( MULTIPLAY::MATCHCHAIN_BAD_PREVIEW == MULTIPLAY::CMatchChainVerifier::VerifySuccessor( watching, &r1, r2, &p2, reason ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}
