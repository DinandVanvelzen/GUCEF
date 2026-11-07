/*
 *  gucefMULTIPLAY_MailProvider_TestApp: Application for testing the mail match provider of gucefMULTIPLAY
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

#include "TestMailFixture.h"
#include "TestMailDecisions.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::MULTIPLAYPLUGIN;
using namespace GUCEF::MAILTEST;

#define ERRORHERE       MAIL_ERRORHERE
#define ASSERT_TRUE(t)  MAIL_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) MAIL_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

GUCEF::vector< CORE::CString >
OneAddress( const char* a )
{GUCEF_TRACE;

    GUCEF::vector< CORE::CString > list;
    list.push_back( a );
    return list;
}

/*-------------------------------------------------------------------------*/

/** The player proposes to hand the target to the computer; returns the signed proposal */
bool
Propose( CMailTable& table, UInt32 proposer, UInt32 target, const CORE::CString& matchId, MULTIPLAY::CMatchProposal& proposal )
{GUCEF_TRACE;

    proposal = MULTIPLAY::CMatchProposal();
    proposal.matchId = matchId;
    proposal.kind = MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER;
    proposal.targetUserId = table.P( target ).identity.userId;
    proposal.createdAt = table.clock.nowMs;
    proposal.deadline = table.clock.nowMs + 7LL * 24 * 3600 * 1000;
    if ( !proposal.Sign( table.P( proposer ).identity ) )
        return false;
    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    MULTIPLAY::CMatchOperationResult result;
    return table.P( proposer ).provider->Propose( matchId, proposal, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status;
}

/*-------------------------------------------------------------------------*/

bool
CastVote( CMailTable& table, UInt32 voter, const CORE::CString& matchId, const MULTIPLAY::CMatchProposal& proposal, bool yes )
{GUCEF_TRACE;

    MULTIPLAY::CMatchVote vote;
    if ( !vote.Sign( table.P( voter ).identity, proposal, yes, table.clock.nowMs ) )
        return false;
    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    MULTIPLAY::CMatchOperationResult result;
    return table.P( voter ).provider->Vote( matchId, vote, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status;
}

/*-------------------------------------------------------------------------*/

/** Waits until the player has the proposal and the number of votes */
bool
WaitVotes( CMailTable& table, UInt32 viewer, const CORE::CString& matchId, const CORE::CString& proposalId, size_t votes )
{GUCEF_TRACE;

    bool ok = false;
    MULTIPLAY::CMatchDecisionEvidence evidence;
    PUMP_WAIT( ok, table.P( viewer ).provider->GetDecisionEvidence( matchId, proposalId, evidence ) && evidence.votes.size() >= votes, 8000 );
    return ok;
}

} /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMailDecisionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "matchprovidermail: decisions by signed proposals and votes" );

    GUCEF_TESTFW_TESTCASE( "Decision: a proposal and the votes reach everybody, more than half pass it, and the turn that applies it carries the evidence that every receiver checks" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Decision: a proposal and the votes reach everybody, more than half pass it, and the turn that applies it carries the evidence that every receiver checks" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "decision-pass" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        // Alice proposes to hand Carol to the computer
        MULTIPLAY::CMatchProposal proposal;
        ASSERT_TRUE( Propose( table, 0, 2, id, proposal ) );
        bool arrived = false;
        PUMP_WAIT( arrived, table.P( 1 ).recorder.decisions.size() >= 1 && table.P( 2 ).recorder.decisions.size() >= 1, 8000 );
        ASSERT_TRUE( arrived && table.P( 1 ).recorder.decisions[ 0 ].code == "PROPOSAL" && table.P( 1 ).recorder.decisions[ 0 ].matchId == id );
        MULTIPLAY::CMatchDecisionResult tally;
        ASSERT_TRUE( table.P( 0 ).provider->EvaluateDecision( id, proposal.proposalId, tally ) && tally.valid );
        ASSERT_TRUE( MULTIPLAY::MATCHVOTEOUTCOME_UNDECIDED == tally.outcome && 1 == tally.yes && 2 == tally.needed && 3 == tally.eligible );

        // Bob agrees: the proposer and Bob are two of three
        ASSERT_TRUE( WaitVotes( table, 1, id, proposal.proposalId, 0 ) && WaitVotes( table, 2, id, proposal.proposalId, 0 ) );
        ASSERT_TRUE( CastVote( table, 1, id, proposal, true ) );
        ASSERT_TRUE( WaitVotes( table, 0, id, proposal.proposalId, 1 ) && WaitVotes( table, 2, id, proposal.proposalId, 1 ) );
        ASSERT_TRUE( table.P( 0 ).provider->EvaluateDecision( id, proposal.proposalId, tally ) && tally.valid && MULTIPLAY::MATCHVOTEOUTCOME_PASSED == tally.outcome && 2 == tally.yes );
        ASSERT_TRUE( table.P( 2 ).recorder.decisions.size() >= 2 && table.P( 2 ).recorder.decisions[ 1 ].code == "VOTE" );

        // Bob's turn applies it: the game puts the signed evidence in the package
        MULTIPLAY::CMatchDecisionEvidence evidence;
        ASSERT_TRUE( table.P( 1 ).provider->GetDecisionEvidence( id, proposal.proposalId, evidence ) && 1 == evidence.votes.size() );
        MULTIPLAY::CMatchInfo info;
        ASSERT_TRUE( table.P( 1 ).provider->GetMatchInfo( id, info ) );
        CORE::CString why;
        ASSERT_TRUE( MULTIPLAY::CMatchDecisionVerifier::Verify( info, evidence, table.clock.nowMs, why ) );
        MULTIPLAY::CMatchPackage package;
        MULTIPLAY::CMatchPreview preview;
        ASSERT_TRUE( table.MakeTurn( 1, id, 0, "hand-carol-over", MULTIPLAY::MATCHSTATUS_ACTIVE, package, preview ) );
        for ( UInt32 i=0; i<3; ++i )
            table.P( i ).verifier.evidenceBySha[ package.sha256 ].push_back( evidence );
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.P( 1 ).provider->Submit( id, 1, preview, package, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        if ( !table.WaitAllTrusted( id, 2 ) )
            for ( UInt32 i=0; i<3; ++i )
            {
                MAIL_TEST_NOTE( "decision pass, player " << i << " trusted " << table.P( i ).provider->GetTrustedRevision( id ) << ", refused " << table.P( i ).recorder.refused.size() << ", verifier calls " << table.P( i ).verifier.calls << ", last reason " << table.P( i ).verifier.lastReason.C_String() );
                for ( size_t r=0; r<table.P( i ).recorder.refused.size(); ++r )
                    MAIL_TEST_NOTE( "  refused " << table.P( i ).recorder.refused[ r ].code.C_String() << ": " << table.P( i ).recorder.refused[ r ].description.C_String() );
            }
        ASSERT_TRUE( table.WaitAllTrusted( id, 2 ) );
        // on every machine Carol is played by the computer now
        for ( UInt32 viewer=0; viewer<3; ++viewer )
        {
            bool taken = false;
            PUMP_WAIT( taken, MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER == table.StatusOf( viewer, id, 2 ), 8000 );
            ASSERT_TRUE( taken );
            ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( viewer, id, 0 ) && MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( viewer, id, 1 ) );
            ASSERT_TRUE( 0 == table.P( viewer ).recorder.refused.size() );
        }
        ASSERT_TRUE( 1 == table.P( 0 ).recorder.CountUpdates( id, 2 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Decision: a tie changes nothing, and a turn that claims a decision that did not pass is refused" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Decision: a tie changes nothing, and a turn that claims a decision that did not pass is refused" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 4, "decision-tie" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        MULTIPLAY::CMatchProposal proposal;
        ASSERT_TRUE( Propose( table, 0, 3, id, proposal ) );
        for ( UInt32 i=1; i<4; ++i )
            ASSERT_TRUE( WaitVotes( table, i, id, proposal.proposalId, 0 ) );
        ASSERT_TRUE( CastVote( table, 1, id, proposal, true ) && CastVote( table, 2, id, proposal, false ) );
        ASSERT_TRUE( WaitVotes( table, 1, id, proposal.proposalId, 2 ) );
        MULTIPLAY::CMatchDecisionResult tally;
        // yes: Alice and Bob; no: Carol; Dave has not answered; three are needed of four
        ASSERT_TRUE( table.P( 1 ).provider->EvaluateDecision( id, proposal.proposalId, tally ) && tally.valid );
        ASSERT_TRUE( MULTIPLAY::MATCHVOTEOUTCOME_UNDECIDED == tally.outcome && 2 == tally.yes && 1 == tally.no && 3 == tally.needed && 4 == tally.eligible );
        // Dave votes no: two against two
        ASSERT_TRUE( CastVote( table, 3, id, proposal, false ) );
        ASSERT_TRUE( WaitVotes( table, 1, id, proposal.proposalId, 3 ) );
        ASSERT_TRUE( table.P( 1 ).provider->EvaluateDecision( id, proposal.proposalId, tally ) && tally.valid );
        ASSERT_TRUE( MULTIPLAY::MATCHVOTEOUTCOME_FAILED == tally.outcome && 2 == tally.yes && 2 == tally.no );

        // Bob plays a turn that applies it anyway
        MULTIPLAY::CMatchDecisionEvidence evidence;
        ASSERT_TRUE( table.P( 1 ).provider->GetDecisionEvidence( id, proposal.proposalId, evidence ) && 3 == evidence.votes.size() );
        MULTIPLAY::CMatchPackage package;
        MULTIPLAY::CMatchPreview preview;
        ASSERT_TRUE( table.MakeTurn( 1, id, 2, "hand-dave-over", MULTIPLAY::MATCHSTATUS_ACTIVE, package, preview ) );
        for ( UInt32 i=0; i<4; ++i )
            table.P( i ).verifier.evidenceBySha[ package.sha256 ].push_back( evidence );
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        MULTIPLAY::CMatchOperationResult result;
        // the provider of the player who plays it refuses it too; a client that was changed sends it anyway
        ASSERT_TRUE( table.P( 1 ).provider->Submit( id, 1, preview, package, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );
        for ( UInt32 i=0; i<4; ++i )
            if ( i != 1 )
                ASSERT_TRUE( table.Inject( i, table.TurnMailText( "bob@mail.test", OneAddress( table.P( i ).address.C_String() ), id, preview, package, MAIL::MAILTRANSFER_ATTACHMENT ) ) );
        bool refused = false;
        PUMP_WAIT( refused, table.P( 0 ).recorder.CountRefused( "BAD_EVIDENCE" ) >= 1 && table.P( 2 ).recorder.CountRefused( "BAD_EVIDENCE" ) >= 1 && table.P( 3 ).recorder.CountRefused( "BAD_EVIDENCE" ) >= 1, 8000 );
        if ( !refused )
            for ( UInt32 i=0; i<4; ++i )
            {
                MAIL_TEST_NOTE( "decision tie, player " << i << " trusted " << table.P( i ).provider->GetTrustedRevision( id ) << ", refused " << table.P( i ).recorder.refused.size() << ", verifier calls " << table.P( i ).verifier.calls );
                for ( size_t r=0; r<table.P( i ).recorder.refused.size(); ++r )
                    MAIL_TEST_NOTE( "  refused " << table.P( i ).recorder.refused[ r ].code.C_String() << ": " << table.P( i ).recorder.refused[ r ].description.C_String() );
            }
        ASSERT_TRUE( refused );
        for ( UInt32 viewer=0; viewer<4; ++viewer )
            if ( viewer != 1 )
            {
                ASSERT_TRUE( 1 == table.P( viewer ).provider->GetTrustedRevision( id ) );
                ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( viewer, id, 3 ) );
            }
        MAIL::TMailRefusalVector refusals;
        ASSERT_TRUE( 1 == table.P( 2 ).provider->GetRefusals( id, refusals ) && !refusals[ 0 ].reason.IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Decision: a participant who holds up the game can be handed to the computer by the turn of another participant that carries the evidence, on every machine" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Decision: a participant who holds up the game can be handed to the computer by the turn of another participant that carries the evidence, on every machine" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "decision-holder" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 2 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        // Carol is up and does not answer: Alice proposes to hand her to the computer and Bob agrees
        MULTIPLAY::CMatchProposal proposal;
        ASSERT_TRUE( Propose( table, 0, 2, id, proposal ) );
        ASSERT_TRUE( WaitVotes( table, 1, id, proposal.proposalId, 0 ) );
        // the proposals of a match are listed, also the ones of the others, and the notice names the proposal
        GUCEF::vector< MULTIPLAY::CMatchProposal > listed;
        ASSERT_TRUE( table.P( 1 ).provider->GetProposals( id, listed ) && 1 == listed.size() && listed[ 0 ].proposalId == proposal.proposalId && listed[ 0 ].targetUserId == table.P( 2 ).identity.userId );
        ASSERT_TRUE( table.P( 1 ).recorder.decisions.size() >= 1 && table.P( 1 ).recorder.decisions[ 0 ].proposalId == proposal.proposalId );
        ASSERT_FALSE( table.P( 1 ).provider->GetProposals( MULTIPLAY::CMatchId::Generate(), listed ) );
        ASSERT_TRUE( CastVote( table, 1, id, proposal, true ) );
        ASSERT_TRUE( WaitVotes( table, 0, id, proposal.proposalId, 1 ) );
        MULTIPLAY::CMatchDecisionEvidence evidence;
        ASSERT_TRUE( table.P( 0 ).provider->GetDecisionEvidence( id, proposal.proposalId, evidence ) && 1 == evidence.votes.size() );

        // Alice plays the turn that applies it, the game goes on with Bob
        MULTIPLAY::CMatchPackage package;
        MULTIPLAY::CMatchPreview preview;
        ASSERT_TRUE( table.MakeTurn( 0, id, 1, "hand-carol-over", MULTIPLAY::MATCHSTATUS_ACTIVE, package, preview ) );
        for ( UInt32 i=0; i<3; ++i )
            table.P( i ).verifier.evidenceBySha[ package.sha256 ].push_back( evidence );
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.P( 0 ).provider->Submit( id, 1, preview, package, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitAllTrusted( id, 2 ) );
        for ( UInt32 viewer=0; viewer<3; ++viewer )
        {
            bool taken = false;
            PUMP_WAIT( taken, MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER == table.StatusOf( viewer, id, 2 ), 8000 );
            ASSERT_TRUE( taken );
            ASSERT_TRUE( 0 == table.P( viewer ).recorder.refused.size() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Decision: a participant cannot sign the turn of another without a decision that passed and names the one who is up" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Decision: a participant cannot sign the turn of another without a decision that passed and names the one who is up" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 4, "decision-holder-hostile" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 3 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        // no evidence at all
        MULTIPLAY::CMatchPackage package;
        MULTIPLAY::CMatchPreview preview;
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.MakeTurn( 0, id, 1, "steal-the-turn", MULTIPLAY::MATCHSTATUS_ACTIVE, package, preview ) );
        ASSERT_TRUE( table.P( 0 ).provider->Submit( id, 1, preview, package, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );

        // Dave is up; the proposal to hand him over fails: two against two
        MULTIPLAY::CMatchProposal proposal;
        ASSERT_TRUE( Propose( table, 0, 3, id, proposal ) );
        for ( UInt32 i=1; i<4; ++i )
            ASSERT_TRUE( WaitVotes( table, i, id, proposal.proposalId, 0 ) );
        ASSERT_TRUE( CastVote( table, 1, id, proposal, true ) && CastVote( table, 2, id, proposal, false ) && CastVote( table, 3, id, proposal, false ) );
        ASSERT_TRUE( WaitVotes( table, 0, id, proposal.proposalId, 3 ) );
        MULTIPLAY::CMatchDecisionEvidence failed;
        ASSERT_TRUE( table.P( 0 ).provider->GetDecisionEvidence( id, proposal.proposalId, failed ) );
        for ( UInt32 i=0; i<4; ++i )
            table.P( i ).verifier.evidenceBySha[ package.sha256 ].push_back( failed );
        future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        ASSERT_TRUE( table.P( 0 ).provider->Submit( id, 1, preview, package, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );

        // a decision that passed, about Carol who is not up: it does not give Alice the turn of Dave
        MULTIPLAY::CMatchProposal aboutCarol;
        ASSERT_TRUE( Propose( table, 0, 2, id, aboutCarol ) );
        for ( UInt32 i=1; i<4; ++i )
            ASSERT_TRUE( WaitVotes( table, i, id, aboutCarol.proposalId, 0 ) );
        ASSERT_TRUE( CastVote( table, 1, id, aboutCarol, true ) && CastVote( table, 3, id, aboutCarol, true ) );
        ASSERT_TRUE( WaitVotes( table, 0, id, aboutCarol.proposalId, 2 ) );
        MULTIPLAY::CMatchDecisionEvidence passed;
        ASSERT_TRUE( table.P( 0 ).provider->GetDecisionEvidence( id, aboutCarol.proposalId, passed ) );
        MULTIPLAY::CMatchPackage other;
        MULTIPLAY::CMatchPreview otherPreview;
        ASSERT_TRUE( table.MakeTurn( 0, id, 1, "steal-the-turn-again", MULTIPLAY::MATCHSTATUS_ACTIVE, other, otherPreview ) );
        for ( UInt32 i=0; i<4; ++i )
            table.P( i ).verifier.evidenceBySha[ other.sha256 ].push_back( passed );
        future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        ASSERT_TRUE( table.P( 0 ).provider->Submit( id, 1, otherPreview, other, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );

        // a turn signed by Alice for Dave, injected as a client that was changed would send it: refused by the others as well
        for ( UInt32 i=1; i<4; ++i )
            ASSERT_TRUE( table.Inject( i, table.TurnMailText( "alice@mail.test", OneAddress( table.P( i ).address.C_String() ), id, otherPreview, other, MAIL::MAILTRANSFER_ATTACHMENT ) ) );
        bool refused = false;
        PUMP_WAIT( refused, table.P( 1 ).recorder.CountRefused( "NOT_ON_TURN" ) >= 1 && table.P( 2 ).recorder.CountRefused( "NOT_ON_TURN" ) >= 1 && table.P( 3 ).recorder.CountRefused( "NOT_ON_TURN" ) >= 1, 8000 );
        ASSERT_TRUE( refused );
        for ( UInt32 viewer=0; viewer<4; ++viewer )
            ASSERT_TRUE( 1 == table.P( viewer ).provider->GetTrustedRevision( id ) && MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( viewer, id, 3 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Decision: proposals and votes that are forged, repeated or in the wrong place are ignored, and a player cannot sign for another" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Decision: proposals and votes that are forged, repeated or in the wrong place are ignored, and a player cannot sign for another" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "decision-hostile" ) && table.StartAll() );
        const CORE::CString id = table.StartMatch( MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );
        MULTIPLAY::CMatchProposal proposal;
        ASSERT_TRUE( Propose( table, 0, 2, id, proposal ) );
        ASSERT_TRUE( WaitVotes( table, 1, id, proposal.proposalId, 0 ) );

        // not a signature of the player: refused when it is made
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        MULTIPLAY::CMatchOperationResult result;
        MULTIPLAY::CMatchVote asBob;
        ASSERT_TRUE( asBob.Sign( table.P( 1 ).identity, proposal, true, table.clock.nowMs ) );
        ASSERT_TRUE( table.P( 2 ).provider->Vote( id, asBob, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );
        MULTIPLAY::CMatchProposal forgedProposal = proposal;
        forgedProposal.targetUserId = table.P( 1 ).identity.userId;
        ASSERT_TRUE( table.P( 0 ).provider->Propose( id, forgedProposal, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );
        MULTIPLAY::CMatchProposal byBob;
        byBob.matchId = id;
        byBob.kind = MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER;
        byBob.targetUserId = table.P( 2 ).identity.userId;
        ASSERT_TRUE( byBob.Sign( table.P( 1 ).identity ) );
        ASSERT_TRUE( table.P( 0 ).provider->Propose( id, byBob, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );
        ASSERT_TRUE( table.P( 0 ).provider->Propose( MULTIPLAY::CMatchId::Generate(), proposal, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_NOT_FOUND == result.status );

        // a vote that Mallory signed in the name of Bob, mailed to Alice
        MULTIPLAY::CPlayerIdentity mallory;
        ASSERT_TRUE( TESTFW::MakeTestIdentity( "Mallory", mallory ) );
        MULTIPLAY::CMatchVote forged;
        ASSERT_TRUE( forged.Sign( mallory, proposal, true, table.clock.nowMs ) );
        forged.voterUserId = table.P( 1 ).identity.userId;
        CORE::CDynamicBuffer data;
        ASSERT_TRUE( MAIL::CMailMatchCodec::SerializeToBytes( forged, data, 64 * 1024 ) );
        ASSERT_TRUE( table.Inject( 0, table.MailTextForData( "bob@mail.test", OneAddress( "alice@mail.test" ), id, 1, MAIL::MAILKIND_VOTE, data, MAIL::CMailMatchCodec::ArmorLabelData, "vote", MAIL::MAILTRANSFER_ATTACHMENT, "<forged-vote@crafted.test>" ) ) );
        // a vote for a proposal nobody made
        MULTIPLAY::CMatchProposal imaginary = proposal;
        imaginary.proposalId = MULTIPLAY::CMatchId::Generate();
        MULTIPLAY::CMatchVote orphan;
        ASSERT_TRUE( orphan.Sign( table.P( 1 ).identity, imaginary, true, table.clock.nowMs ) );
        CORE::CDynamicBuffer orphanData;
        ASSERT_TRUE( MAIL::CMailMatchCodec::SerializeToBytes( orphan, orphanData, 64 * 1024 ) );
        ASSERT_TRUE( table.Inject( 0, table.MailTextForData( "bob@mail.test", OneAddress( "alice@mail.test" ), id, 1, MAIL::MAILKIND_VOTE, orphanData, MAIL::CMailMatchCodec::ArmorLabelData, "vote", MAIL::MAILTRANSFER_ATTACHMENT, "<orphan-vote@crafted.test>" ) ) );
        // a proposal of a stranger
        MULTIPLAY::CMatchProposal byMallory;
        byMallory.matchId = id;
        byMallory.kind = MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER;
        byMallory.targetUserId = table.P( 2 ).identity.userId;
        ASSERT_TRUE( byMallory.Sign( mallory ) );
        CORE::CDynamicBuffer strangerData;
        ASSERT_TRUE( MAIL::CMailMatchCodec::SerializeToBytes( byMallory, strangerData, 64 * 1024 ) );
        ASSERT_TRUE( table.Inject( 0, table.MailTextForData( "mallory@mail.test", OneAddress( "alice@mail.test" ), id, 1, MAIL::MAILKIND_PROPOSAL, strangerData, MAIL::CMailMatchCodec::ArmorLabelData, "proposal", MAIL::MAILTRANSFER_ATTACHMENT, "<stranger-proposal@crafted.test>" ) ) );
        table.Settle( 1000 );
        MULTIPLAY::CMatchDecisionEvidence evidence;
        ASSERT_TRUE( table.P( 0 ).provider->GetDecisionEvidence( id, proposal.proposalId, evidence ) && 0 == evidence.votes.size() );
        ASSERT_FALSE( table.P( 0 ).provider->GetDecisionEvidence( id, byMallory.proposalId, evidence ) );
        ASSERT_FALSE( table.P( 0 ).provider->GetDecisionEvidence( id, imaginary.proposalId, evidence ) );
        ASSERT_TRUE( 0 == table.P( 0 ).recorder.decisions.size() );

        // the real vote counts once, even when it comes twice
        ASSERT_TRUE( CastVote( table, 1, id, proposal, true ) );
        ASSERT_TRUE( WaitVotes( table, 0, id, proposal.proposalId, 1 ) );
        table.world.DuplicateNext( 1 );
        ASSERT_TRUE( CastVote( table, 1, id, proposal, true ) );
        table.Settle( 800 );
        ASSERT_TRUE( table.P( 0 ).provider->GetDecisionEvidence( id, proposal.proposalId, evidence ) && 1 == evidence.votes.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

namespace {

/** The creator opens a match for the whole table; the players in 'accepting' accept, the others do not answer. Returns the match id */
CORE::CString
OpenLobby( CMailTable& table, const GUCEF::vector< UInt32 >& accepting, MULTIPLAY::TMatchDecisionPolicy policy = MULTIPLAY::MATCHDECISIONPOLICY_VOTE, int boss = -1, MAIL::TMailDistribution distribution = MAIL::MAILDISTRIBUTION_ALL )
{GUCEF_TRACE;

    MULTIPLAY::CMatchRecord record = table.MakeRecord( "Lobby", MAIL::MAILTRANSFER_ATTACHMENT, distribution );
    record.info.decisionPolicy = policy;
    if ( boss >= 0 )
        record.info.participants[ (size_t) boss ].boss = true;
    record.Sign( table.P( 0 ).identity );
    MULTIPLAY::CMatchOperationResult result;
    if ( !table.CreateMatch( 0, record, 0, result ) || MULTIPLAY::MATCHOPERATION_OK != result.status )
        return CORE::CString::Empty;
    const CORE::CString id = record.info.matchId;
    for ( UInt32 i=1; i<table.players.size(); ++i )
        if ( !table.WaitKnown( i, id ) )
            return CORE::CString::Empty;
    for ( size_t a=0; a<accepting.size(); ++a )
        if ( !table.Respond( accepting[ a ], id, true, result ) || MULTIPLAY::MATCHOPERATION_OK != result.status )
            return CORE::CString::Empty;
    for ( size_t a=0; a<accepting.size(); ++a )
        for ( UInt32 viewer=0; viewer<table.players.size(); ++viewer )
        {
            bool known = false;
            PUMP_WAIT( known, MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( viewer, id, accepting[ a ] ), 8000 );
            if ( !known )
                return CORE::CString::Empty;
        }
    return id;
}

/*-------------------------------------------------------------------------*/

/** A proposal of a decision before the start, with the electorate of the proposer's view of the table */
bool
ProposeBeforeStart( CMailTable& table, UInt32 proposer, const CORE::CString& id, MULTIPLAY::TMatchDecisionKind kind, int target, const GUCEF::vector< UInt32 >& targets, MULTIPLAY::CMatchProposal& proposal, bool expectAccepted = true )
{GUCEF_TRACE;

    MULTIPLAY::CMatchInfo info;
    if ( !table.P( proposer ).provider->GetMatchInfo( id, info ) )
        return false;
    proposal = MULTIPLAY::CMatchProposal();
    proposal.matchId = id;
    proposal.kind = kind;
    if ( target >= 0 )
        proposal.targetUserId = table.P( (UInt32) target ).identity.userId;
    for ( size_t t=0; t<targets.size(); ++t )
        proposal.targetUserIds.push_back( table.P( targets[ t ] ).identity.userId );
    proposal.SetElectorateFrom( info );
    proposal.createdAt = table.clock.nowMs;
    proposal.deadline = table.clock.nowMs + 7LL * 24 * 3600 * 1000;
    if ( !proposal.Sign( table.P( proposer ).identity ) )
        return false;
    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    MULTIPLAY::CMatchOperationResult result;
    const bool done = table.P( proposer ).provider->Propose( id, proposal, future ) && table.Await( future, result );
    return done && ( expectAccepted == ( MULTIPLAY::MATCHOPERATION_OK == result.status ) );
}

/*-------------------------------------------------------------------------*/

/** The creator plays the first turn of the game with the evidence of the decisions in it; every machine's game check knows the evidence of the package */
bool
PlayFirstTurnWith( CMailTable& table, UInt32 player, const CORE::CString& id, int nextHolder, const GUCEF::vector< MULTIPLAY::CMatchDecisionEvidence >& evidence, MULTIPLAY::CMatchOperationResult& result )
{GUCEF_TRACE;

    MULTIPLAY::CMatchPackage package;
    MULTIPLAY::CMatchPreview preview;
    if ( !table.MakeTurn( player, id, nextHolder, "start", MULTIPLAY::MATCHSTATUS_ACTIVE, package, preview ) )
        return false;
    for ( UInt32 i=0; i<table.players.size(); ++i )
    {
        table.P( i ).verifier.evidenceBySha[ package.sha256 ].clear();
        for ( size_t e=0; e<evidence.size(); ++e )
            table.P( i ).verifier.evidenceBySha[ package.sha256 ].push_back( evidence[ e ] );
    }
    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    return table.P( player ).provider->Submit( id, package.revision - 1, preview, package, future ) && table.Await( future, result );
}

/*-------------------------------------------------------------------------*/

/** The statuses of every participant as the viewer sees them, with the reasons, as one text to compare machines */
CORE::CString
Table( CMailTable& table, UInt32 viewer, const CORE::CString& id )
{GUCEF_TRACE;

    MULTIPLAY::CMatchInfo info;
    if ( !table.P( viewer ).provider->GetMatchInfo( id, info ) )
        return "unknown";
    CORE::CString text = CORE::CString( MULTIPLAY::MatchStatusToString( info.status ) ) + ":";
    for ( size_t i=0; i<info.participants.size(); ++i )
        text += CORE::CString( MULTIPLAY::MatchParticipantStatusToString( info.participants[ i ].status ) ) + "/" + info.participants[ i ].statusCode + "/" + info.participants[ i ].statusReason + ";";
    return text;
}

/*-------------------------------------------------------------------------*/

bool
WaitStatus( CMailTable& table, UInt32 viewer, const CORE::CString& id, UInt32 subject, MULTIPLAY::TMatchParticipantStatus status )
{GUCEF_TRACE;

    bool ok = false;
    PUMP_WAIT( ok, status == table.StatusOf( viewer, id, subject ), 8000 );
    return ok;
}

} /* anonymous namespace */

/*-------------------------------------------------------------------------*/

void
PerformMailLobbyTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "matchprovidermail: decisions before the start" );

    GUCEF_TESTFW_TESTCASE( "Lobby: a force start by vote replaces the players who did not answer by the computer; the evidence in the first turn is what every machine checks" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Lobby: a force start by vote replaces the players who did not answer by the computer; the evidence in the first turn is what every machine checks" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 4, "lobby-force" ) && table.StartAll() );
        GUCEF::vector< UInt32 > accepting;
        accepting.push_back( 1 );
        const CORE::CString id = OpenLobby( table, accepting );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        // Carol and Dave did not answer: the creator cannot start the game as it is
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( PlayFirstTurnWith( table, 0, id, 1, GUCEF::vector< MULTIPLAY::CMatchDecisionEvidence >(), result ) );
        ASSERT_TRUE( MULTIPLAY::MATCHOPERATION_REJECTED == result.status );

        // Alice proposes a force start: two accepted, so both are needed; a no of Bob is a tie and changes nothing
        GUCEF::vector< UInt32 > missing;
        missing.push_back( 2 );
        missing.push_back( 3 );
        MULTIPLAY::CMatchProposal proposal;
        ASSERT_TRUE( ProposeBeforeStart( table, 0, id, MULTIPLAY::MATCHDECISION_FORCE_START, -1, missing, proposal ) );
        ASSERT_TRUE( 2 == proposal.voterUserIds.size() );
        bool arrived = false;
        PUMP_WAIT( arrived, table.P( 1 ).recorder.decisions.size() >= 1, 8000 );
        ASSERT_TRUE( arrived );
        ASSERT_TRUE( CastVote( table, 1, id, proposal, false ) );
        ASSERT_TRUE( WaitVotes( table, 0, id, proposal.proposalId, 1 ) );
        MULTIPLAY::CMatchDecisionResult tally;
        ASSERT_TRUE( table.P( 0 ).provider->EvaluateDecision( id, proposal.proposalId, tally ) && tally.valid );
        ASSERT_TRUE( MULTIPLAY::MATCHVOTEOUTCOME_FAILED == tally.outcome && 1 == tally.yes && 1 == tally.no && 2 == tally.eligible );
        MULTIPLAY::CMatchDecisionEvidence tie;
        ASSERT_TRUE( table.P( 0 ).provider->GetDecisionEvidence( id, proposal.proposalId, tie ) );
        GUCEF::vector< MULTIPLAY::CMatchDecisionEvidence > carried;
        carried.push_back( tie );
        ASSERT_TRUE( PlayFirstTurnWith( table, 0, id, 1, carried, result ) );
        ASSERT_TRUE( MULTIPLAY::MATCHOPERATION_REJECTED == result.status );
        ASSERT_TRUE( 1 == table.P( 0 ).provider->GetTrustedRevision( id ) );

        // a second proposal, Bob agrees: it passes
        MULTIPLAY::CMatchProposal second;
        ASSERT_TRUE( ProposeBeforeStart( table, 0, id, MULTIPLAY::MATCHDECISION_FORCE_START, -1, missing, second ) );
        ASSERT_TRUE( WaitVotes( table, 1, id, second.proposalId, 0 ) );
        ASSERT_TRUE( CastVote( table, 1, id, second, true ) );
        ASSERT_TRUE( WaitVotes( table, 0, id, second.proposalId, 1 ) );
        MULTIPLAY::CMatchDecisionEvidence passed;
        ASSERT_TRUE( table.P( 0 ).provider->GetDecisionEvidence( id, second.proposalId, passed ) );
        // what the creator carries into the first turn is what passed, not what failed
        GUCEF::vector< MULTIPLAY::CMatchDecisionEvidence > pre;
        ASSERT_TRUE( table.P( 0 ).provider->GetPreStartEvidence( id, pre ) && 1 == pre.size() && pre[ 0 ].proposal.proposalId == second.proposalId );
        carried.clear();
        carried.push_back( passed );
        ASSERT_TRUE( PlayFirstTurnWith( table, 0, id, 1, carried, result ) );
        ASSERT_TRUE( MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitTrusted( 0, id, 2 ) && table.WaitTrusted( 1, id, 2 ) );

        // on every machine the table is the same: Carol and Dave are played by the computer, the match is active
        const CORE::CString expected = Table( table, 0, id );
        ASSERT_TRUE( MULTIPLAY::MATCHSTATUS_ACTIVE == table.MatchStatusOf( 0, id ) );
        ASSERT_TRUE( WaitStatus( table, 1, id, 2, MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER ) && WaitStatus( table, 1, id, 3, MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER ) );
        ASSERT_TRUE( Table( table, 1, id ) == expected );
        ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER == table.StatusOf( 0, id, 2 ) && MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER == table.StatusOf( 0, id, 3 ) );
        // the one who got the turn mail without having answered applies it the same way
        ASSERT_TRUE( table.WaitTrusted( 2, id, 2 ) && table.WaitTrusted( 3, id, 2 ) );
        ASSERT_TRUE( Table( table, 2, id ) == expected && Table( table, 3, id ) == expected );
        for ( UInt32 i=0; i<4; ++i )
            ASSERT_TRUE( 0 == table.P( i ).recorder.refused.size() );
        // the lobby told what happened, once
        bool told = false;
        for ( size_t n=0; n<table.P( 1 ).recorder.lobby.size(); ++n )
            told = told || table.P( 1 ).recorder.lobby[ n ].code == "PLAYERS_REPLACED";
        ASSERT_TRUE( told );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lobby: under the BOSS policy the boss forces the start alone, and a player who is no boss cannot propose a force start or a kick" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Lobby: under the BOSS policy the boss forces the start alone, and a player who is no boss cannot propose a force start or a kick" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "lobby-boss" ) && table.StartAll() );
        GUCEF::vector< UInt32 > accepting;
        accepting.push_back( 1 );
        const CORE::CString id = OpenLobby( table, accepting, MULTIPLAY::MATCHDECISIONPOLICY_BOSS, 1 );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        GUCEF::vector< UInt32 > missing;
        missing.push_back( 2 );
        MULTIPLAY::CMatchProposal byCreator;
        MULTIPLAY::CMatchProposal byCarol;
        // the creator is no boss here, Carol is no boss and did not answer: both are refused
        ASSERT_TRUE( ProposeBeforeStart( table, 0, id, MULTIPLAY::MATCHDECISION_FORCE_START, -1, missing, byCreator, false ) );
        MULTIPLAY::CMatchProposal kick;
        GUCEF::vector< UInt32 > none;
        ASSERT_TRUE( ProposeBeforeStart( table, 0, id, MULTIPLAY::MATCHDECISION_KICK, 2, none, kick, false ) );

        // the boss decides alone
        MULTIPLAY::CMatchProposal force;
        ASSERT_TRUE( ProposeBeforeStart( table, 1, id, MULTIPLAY::MATCHDECISION_FORCE_START, -1, missing, force ) );
        MULTIPLAY::CMatchDecisionEvidence evidence;
        ASSERT_TRUE( table.P( 1 ).provider->GetDecisionEvidence( id, force.proposalId, evidence ) );
        // the creator carries it in the first turn
        bool known = false;
        PUMP_WAIT( known, table.P( 0 ).provider->GetDecisionEvidence( id, force.proposalId, evidence ), 8000 );
        ASSERT_TRUE( known );
        GUCEF::vector< MULTIPLAY::CMatchDecisionEvidence > carried;
        carried.push_back( evidence );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( PlayFirstTurnWith( table, 0, id, 1, carried, result ) );
        ASSERT_TRUE( MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitAllTrusted( id, 2 ) );
        ASSERT_TRUE( WaitStatus( table, 1, id, 2, MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER ) && WaitStatus( table, 2, id, 2, MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER ) );
        ASSERT_TRUE( Table( table, 0, id ) == Table( table, 1, id ) && Table( table, 1, id ) == Table( table, 2, id ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lobby: a kick before the start by vote removes the player from the table; the votes of others, of strangers and the forged ones do not" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Lobby: a kick before the start by vote removes the player from the table; the votes of others, of strangers and the forged ones do not" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 4, "lobby-kick" ) && table.StartAll() );
        GUCEF::vector< UInt32 > accepting;
        accepting.push_back( 1 );
        accepting.push_back( 2 );
        accepting.push_back( 3 );
        const CORE::CString id = OpenLobby( table, accepting );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        // the creator cannot be kicked, nor can a player kick themself
        MULTIPLAY::CMatchProposal refused;
        GUCEF::vector< UInt32 > none;
        ASSERT_TRUE( ProposeBeforeStart( table, 1, id, MULTIPLAY::MATCHDECISION_KICK, 0, none, refused, false ) );
        ASSERT_TRUE( ProposeBeforeStart( table, 1, id, MULTIPLAY::MATCHDECISION_KICK, 1, none, refused, false ) );

        // a forged proposal of a stranger who claims the kick of Dave never gets in
        MULTIPLAY::CPlayerIdentity mallory;
        ASSERT_TRUE( TESTFW::MakeTestIdentity( "Mallory", mallory ) );
        MULTIPLAY::CMatchProposal byMallory;
        byMallory.matchId = id;
        byMallory.kind = MULTIPLAY::MATCHDECISION_KICK;
        byMallory.targetUserId = table.P( 3 ).identity.userId;
        MULTIPLAY::CMatchInfo info;
        ASSERT_TRUE( table.P( 0 ).provider->GetMatchInfo( id, info ) );
        byMallory.SetElectorateFrom( info );
        byMallory.voterUserIds.push_back( mallory.userId );
        ASSERT_TRUE( byMallory.Sign( mallory ) );
        CORE::CDynamicBuffer strangerData;
        ASSERT_TRUE( MAIL::CMailMatchCodec::SerializeToBytes( byMallory, strangerData, 64 * 1024 ) );
        ASSERT_TRUE( table.Inject( 0, table.MailTextForData( "mallory@mail.test", OneAddress( "alice@mail.test" ), id, 1, MAIL::MAILKIND_PROPOSAL, strangerData, MAIL::CMailMatchCodec::ArmorLabelData, "proposal", MAIL::MAILTRANSFER_ATTACHMENT, "<stranger-kick@crafted.test>" ) ) );
        table.Settle( 800 );
        GUCEF::vector< MULTIPLAY::CMatchProposal > known;
        ASSERT_TRUE( table.P( 0 ).provider->GetProposals( id, known ) && 0 == known.size() );

        // Bob proposes to kick Dave: three of four are needed
        MULTIPLAY::CMatchProposal kick;
        ASSERT_TRUE( ProposeBeforeStart( table, 1, id, MULTIPLAY::MATCHDECISION_KICK, 3, none, kick ) );
        ASSERT_TRUE( WaitVotes( table, 0, id, kick.proposalId, 0 ) && WaitVotes( table, 2, id, kick.proposalId, 0 ) );
        ASSERT_TRUE( CastVote( table, 2, id, kick, true ) );
        ASSERT_TRUE( WaitVotes( table, 0, id, kick.proposalId, 1 ) );
        MULTIPLAY::CMatchDecisionEvidence evidence;
        ASSERT_TRUE( table.P( 0 ).provider->GetDecisionEvidence( id, kick.proposalId, evidence ) );
        GUCEF::vector< MULTIPLAY::CMatchDecisionEvidence > carried;
        carried.push_back( evidence );
        MULTIPLAY::CMatchOperationResult result;
        // two of four have voted for it: it did not pass, and the turn that claims it is refused
        ASSERT_TRUE( PlayFirstTurnWith( table, 0, id, 1, carried, result ) );
        ASSERT_TRUE( MULTIPLAY::MATCHOPERATION_REJECTED == result.status );
        const UInt32 trustedBefore = table.P( 0 ).provider->GetTrustedRevision( id );
        ASSERT_TRUE( 1 == trustedBefore );
        // Alice agrees too: it passes
        ASSERT_TRUE( CastVote( table, 0, id, kick, true ) );
        ASSERT_TRUE( WaitVotes( table, 1, id, kick.proposalId, 2 ) );
        ASSERT_TRUE( table.P( 1 ).provider->GetDecisionEvidence( id, kick.proposalId, evidence ) );
        carried.clear();
        carried.push_back( evidence );
        ASSERT_TRUE( PlayFirstTurnWith( table, 0, id, 1, carried, result ) );
        ASSERT_TRUE( MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitTrusted( 1, id, 2 ) && table.WaitTrusted( 2, id, 2 ) );
        ASSERT_TRUE( WaitStatus( table, 0, id, 3, MULTIPLAY::MATCHPARTICIPANT_DROPPED ) && WaitStatus( table, 1, id, 3, MULTIPLAY::MATCHPARTICIPANT_DROPPED ) && WaitStatus( table, 2, id, 3, MULTIPLAY::MATCHPARTICIPANT_DROPPED ) );
        ASSERT_TRUE( Table( table, 0, id ) == Table( table, 1, id ) && Table( table, 1, id ) == Table( table, 2, id ) );
        // Dave is gone: the turns do not go to Dave any more
        table.Settle( 400 );
        ASSERT_TRUE( 1 == table.P( 3 ).provider->GetTrustedRevision( id ) );
        for ( UInt32 i=0; i<3; ++i )
            ASSERT_TRUE( 0 == table.P( i ).recorder.refused.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lobby: under the BOSS policy the creator cancels a match that has not started alone; every invitee is told with a signed cancel and nothing else can cancel it" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Lobby: under the BOSS policy the creator cancels a match that has not started alone; every invitee is told with a signed cancel and nothing else can cancel it" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 4, "lobby-cancel" ) && table.StartAll() );
        GUCEF::vector< UInt32 > accepting;
        accepting.push_back( 1 );
        const CORE::CString id = OpenLobby( table, accepting, MULTIPLAY::MATCHDECISIONPOLICY_BOSS );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        // Bob is no creator and no boss: he cannot cancel, and a cancel he signs is refused by everybody
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.P( 1 ).provider->Cancel( id, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );
        MULTIPLAY::CMatchProposal forged;
        forged.matchId = id;
        forged.kind = MULTIPLAY::MATCHDECISION_CANCEL;
        forged.createdAt = table.clock.nowMs;
        ASSERT_TRUE( forged.Sign( table.P( 1 ).identity ) );
        CORE::CDynamicBuffer forgedData;
        ASSERT_TRUE( MAIL::CMailMatchCodec::SerializeToBytes( forged, forgedData, 64 * 1024 ) );
        ASSERT_TRUE( table.Inject( 2, table.MailTextForData( "bob@mail.test", OneAddress( "carol@mail.test" ), id, 1, MAIL::MAILKIND_CANCEL, forgedData, MAIL::CMailMatchCodec::ArmorLabelData, "cancel", MAIL::MAILTRANSFER_ATTACHMENT, "<forged-cancel@crafted.test>" ) ) );
        table.Settle( 800 );
        ASSERT_TRUE( MULTIPLAY::MATCHSTATUS_CANCELLED != table.MatchStatusOf( 2, id ) );

        // the creator cancels
        ASSERT_TRUE( table.P( 0 ).provider->Cancel( id, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( MULTIPLAY::MATCHSTATUS_CANCELLED == table.MatchStatusOf( 0, id ) );
        for ( UInt32 viewer=1; viewer<4; ++viewer )
        {
            bool cancelled = false;
            PUMP_WAIT( cancelled, MULTIPLAY::MATCHSTATUS_CANCELLED == table.MatchStatusOf( viewer, id ), 8000 );
            ASSERT_TRUE( cancelled );
            MULTIPLAY::CMatchInfo info;
            ASSERT_TRUE( table.P( viewer ).provider->GetMatchInfo( id, info ) && info.closedByUserId == table.P( 0 ).identity.userId );
            bool told = false;
            for ( size_t n=0; n<table.P( viewer ).recorder.lobby.size(); ++n )
                told = told || ( table.P( viewer ).recorder.lobby[ n ].code == "CANCELLED" && table.P( viewer ).recorder.lobby[ n ].userId == table.P( 0 ).identity.userId );
            ASSERT_TRUE( told );
        }
        // nothing is played or answered in a cancelled match
        MULTIPLAY::CMatchOperationResult answer;
        ASSERT_TRUE( table.Respond( 2, id, true, answer ) && MULTIPLAY::MATCHOPERATION_REJECTED == answer.status );
        ASSERT_TRUE( table.PlayTurn( 0, id, 1, "start", answer ) && MULTIPLAY::MATCHOPERATION_REJECTED == answer.status );
        // the cancel again, by mail, is the same again: nothing changes and nothing is announced twice
        const size_t announced = table.P( 2 ).recorder.lobby.size();
        ASSERT_TRUE( table.P( 0 ).provider->Cancel( id, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        table.Settle( 600 );
        ASSERT_TRUE( table.P( 2 ).recorder.lobby.size() == announced );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lobby: under the VOTE policy the creator does not cancel alone: the direct cancel is refused, a cancel mail of the creator alone is refused by everybody, and a cancel passes by the vote of the players who accepted" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Lobby: under the VOTE policy the creator does not cancel alone: the direct cancel is refused, a cancel mail of the creator alone is refused by everybody, and a cancel passes by the vote of the players who accepted" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 4, "lobby-cancel-vote" ) && table.StartAll() );
        GUCEF::vector< UInt32 > accepting;
        accepting.push_back( 1 );
        accepting.push_back( 2 );
        const CORE::CString id = OpenLobby( table, accepting );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        // the operation that cancels alone is not for a match decided by vote: it says so, and nothing changes
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.P( 0 ).provider->Cancel( id, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );
        ASSERT_TRUE( result.reason.HasSubstr( "vote", false ) >= 0 );
        ASSERT_TRUE( MULTIPLAY::MATCHSTATUS_CANCELLED != table.MatchStatusOf( 0, id ) );

        // a cancel the creator signed and mailed alone claims an authority the creator does not have: every participant refuses it
        MULTIPLAY::CMatchProposal host;
        host.matchId = id;
        host.kind = MULTIPLAY::MATCHDECISION_CANCEL;
        host.createdAt = table.clock.nowMs;
        ASSERT_TRUE( host.Sign( table.P( 0 ).identity ) );
        CORE::CDynamicBuffer hostData;
        ASSERT_TRUE( MAIL::CMailMatchCodec::SerializeToBytes( host, hostData, 64 * 1024 ) );
        ASSERT_TRUE( table.Inject( 1, table.MailTextForData( "alice@mail.test", OneAddress( "bob@mail.test" ), id, 1, MAIL::MAILKIND_CANCEL, hostData, MAIL::CMailMatchCodec::ArmorLabelData, "cancel", MAIL::MAILTRANSFER_ATTACHMENT, "<host-only-cancel-b@crafted.test>" ) ) );
        ASSERT_TRUE( table.Inject( 2, table.MailTextForData( "alice@mail.test", OneAddress( "carol@mail.test" ), id, 1, MAIL::MAILKIND_CANCEL, hostData, MAIL::CMailMatchCodec::ArmorLabelData, "cancel", MAIL::MAILTRANSFER_ATTACHMENT, "<host-only-cancel-c@crafted.test>" ) ) );
        ASSERT_TRUE( table.Inject( 3, table.MailTextForData( "alice@mail.test", OneAddress( "dave@mail.test" ), id, 1, MAIL::MAILKIND_CANCEL, hostData, MAIL::CMailMatchCodec::ArmorLabelData, "cancel", MAIL::MAILTRANSFER_ATTACHMENT, "<host-only-cancel-d@crafted.test>" ) ) );
        table.Settle( 800 );
        for ( UInt32 i=1; i<4; ++i )
            ASSERT_TRUE( MULTIPLAY::MATCHSTATUS_CANCELLED != table.MatchStatusOf( i, id ) );

        // the creator proposes it with the electorate (the three who accepted); a no from Bob and a yes of Carol decide nothing alone: two of three pass
        MULTIPLAY::CMatchProposal cancel;
        GUCEF::vector< UInt32 > none;
        ASSERT_TRUE( ProposeBeforeStart( table, 0, id, MULTIPLAY::MATCHDECISION_CANCEL, -1, none, cancel ) );
        ASSERT_TRUE( 3 == cancel.voterUserIds.size() );
        ASSERT_TRUE( WaitVotes( table, 1, id, cancel.proposalId, 0 ) && WaitVotes( table, 3, id, cancel.proposalId, 0 ) );
        ASSERT_TRUE( MULTIPLAY::MATCHSTATUS_CANCELLED != table.MatchStatusOf( 0, id ) );
        ASSERT_TRUE( CastVote( table, 1, id, cancel, false ) );
        ASSERT_TRUE( WaitVotes( table, 0, id, cancel.proposalId, 1 ) && WaitVotes( table, 2, id, cancel.proposalId, 1 ) );
        table.Settle( 400 );
        for ( UInt32 i=0; i<4; ++i )
            ASSERT_TRUE( MULTIPLAY::MATCHSTATUS_CANCELLED != table.MatchStatusOf( i, id ) );
        ASSERT_TRUE( CastVote( table, 2, id, cancel, true ) );

        // every machine counted the same: the match is cancelled for the players who accepted and for Dave, who did not answer, and the creator closed it
        for ( UInt32 viewer=0; viewer<4; ++viewer )
        {
            bool cancelled = false;
            PUMP_WAIT( cancelled, MULTIPLAY::MATCHSTATUS_CANCELLED == table.MatchStatusOf( viewer, id ), 8000 );
            ASSERT_TRUE( cancelled );
            MULTIPLAY::CMatchInfo info;
            ASSERT_TRUE( table.P( viewer ).provider->GetMatchInfo( id, info ) && info.closedByUserId == table.P( 0 ).identity.userId );
            bool told = false;
            for ( size_t n=0; n<table.P( viewer ).recorder.lobby.size(); ++n )
                told = told || ( table.P( viewer ).recorder.lobby[ n ].code == "CANCELLED" );
            ASSERT_TRUE( told );
        }
        // nothing is answered or played in a cancelled match
        MULTIPLAY::CMatchOperationResult answer;
        ASSERT_TRUE( table.Respond( 3, id, true, answer ) && MULTIPLAY::MATCHOPERATION_REJECTED == answer.status );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lobby: under the VOTE policy a player who accepted proposes the cancel as well, and when the creator is the only one who accepted the creator's proposal passes at once" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Lobby: under the VOTE policy a player who accepted proposes the cancel as well, and when the creator is the only one who accepted the creator's proposal passes at once" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        {
            CMailTable table;
            ASSERT_TRUE( table.Setup( 3, "lobby-cancel-bob" ) && table.StartAll() );
            GUCEF::vector< UInt32 > accepting;
            accepting.push_back( 1 );
            const CORE::CString id = OpenLobby( table, accepting );
            ASSERT_FALSE( id.IsNULLOrEmpty() );
            // Carol did not answer: she proposes nothing; Bob does, and the creator votes yes: two of two
            MULTIPLAY::CMatchProposal byCarol;
            GUCEF::vector< UInt32 > none;
            ASSERT_TRUE( ProposeBeforeStart( table, 2, id, MULTIPLAY::MATCHDECISION_CANCEL, -1, none, byCarol, false ) );
            MULTIPLAY::CMatchProposal byBob;
            ASSERT_TRUE( ProposeBeforeStart( table, 1, id, MULTIPLAY::MATCHDECISION_CANCEL, -1, none, byBob ) );
            ASSERT_TRUE( WaitVotes( table, 0, id, byBob.proposalId, 0 ) );
            ASSERT_TRUE( CastVote( table, 0, id, byBob, true ) );
            for ( UInt32 viewer=0; viewer<3; ++viewer )
            {
                bool cancelled = false;
                PUMP_WAIT( cancelled, MULTIPLAY::MATCHSTATUS_CANCELLED == table.MatchStatusOf( viewer, id ), 8000 );
                ASSERT_TRUE( cancelled );
                MULTIPLAY::CMatchInfo info;
                ASSERT_TRUE( table.P( viewer ).provider->GetMatchInfo( id, info ) && info.closedByUserId == table.P( 1 ).identity.userId );
            }
        }
        {
            CMailTable table;
            ASSERT_TRUE( table.Setup( 3, "lobby-cancel-sole" ) && table.StartAll() );
            GUCEF::vector< UInt32 > accepting;
            const CORE::CString id = OpenLobby( table, accepting );
            ASSERT_FALSE( id.IsNULLOrEmpty() );
            MULTIPLAY::CMatchProposal cancel;
            GUCEF::vector< UInt32 > none;
            ASSERT_TRUE( ProposeBeforeStart( table, 0, id, MULTIPLAY::MATCHDECISION_CANCEL, -1, none, cancel ) );
            ASSERT_TRUE( 1 == cancel.voterUserIds.size() );
            ASSERT_TRUE( MULTIPLAY::MATCHSTATUS_CANCELLED == table.MatchStatusOf( 0, id ) );
            for ( UInt32 viewer=1; viewer<3; ++viewer )
            {
                bool cancelled = false;
                PUMP_WAIT( cancelled, MULTIPLAY::MATCHSTATUS_CANCELLED == table.MatchStatusOf( viewer, id ), 8000 );
                ASSERT_TRUE( cancelled );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lobby: a game that waits for introductions is cancelled by its creator: the pending introductions are cancelled with the name of the creator, a cancel signed by anybody else is refused" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Lobby: a game that waits for introductions is cancelled by its creator: the pending introductions are cancelled with the name of the creator, a cancel signed by anybody else is refused" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 3, "lobby-cancel-draft" ) && table.StartAll() );
        const CORE::CString id = MULTIPLAY::CMatchId::Generate();
        GUCEF::vector< MAIL::CMailIntroContactInput > contacts;
        for ( UInt32 i=1; i<3; ++i )
        {
            MAIL::CMailIntroContactInput contact;
            contact.name = table.P( i ).name;
            contact.address = table.P( i ).address;
            contacts.push_back( contact );
        }
        GUCEF::map< CORE::CString, CORE::CString > settings;
        CORE::CString why;
        ASSERT_TRUE( table.P( 0 ).provider->BeginIntroductions( id, "Draft", settings, contacts, why ) );
        bool both = false;
        for ( int round=0; round<400 && !both; ++round )
        {
            table.Settle( 20 );
            GUCEF::vector< MAIL::CMailInboundIntro > b, c;
            table.P( 1 ).provider->GetPendingIntroductions( b );
            table.P( 2 ).provider->GetPendingIntroductions( c );
            both = 1 == b.size() && 1 == c.size();
        }
        ASSERT_TRUE( both );

        // Carol (a participant of the draft, not its creator) signs a cancel: refused by Bob
        MULTIPLAY::CMatchProposal forged;
        forged.matchId = id;
        forged.kind = MULTIPLAY::MATCHDECISION_CANCEL;
        forged.createdAt = table.clock.nowMs;
        ASSERT_TRUE( forged.Sign( table.P( 2 ).identity ) );
        CORE::CDynamicBuffer forgedData;
        ASSERT_TRUE( MAIL::CMailMatchCodec::SerializeToBytes( forged, forgedData, 64 * 1024 ) );
        ASSERT_TRUE( table.Inject( 1, table.MailTextForData( "alice@mail.test", OneAddress( "bob@mail.test" ), id, 0, MAIL::MAILKIND_CANCEL, forgedData, MAIL::CMailMatchCodec::ArmorLabelData, "cancel", MAIL::MAILTRANSFER_ATTACHMENT, "<forged-draft-cancel@crafted.test>" ) ) );
        table.Settle( 600 );
        GUCEF::vector< MAIL::CMailInboundIntro > bobPending;
        table.P( 1 ).provider->GetPendingIntroductions( bobPending );
        ASSERT_TRUE( 1 == bobPending.size() && MAIL::CMailInboundIntro::STATUS_PENDING == bobPending[ 0 ].status );

        // Alice cancels the game: both invitations are cancelled and name her; the draft is gone
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.P( 0 ).provider->Cancel( id, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        MAIL::CMailIntroDraft draft;
        ASSERT_FALSE( table.P( 0 ).provider->GetIntroductionDraft( id, draft ) );
        for ( UInt32 viewer=1; viewer<3; ++viewer )
        {
            bool cancelled = false;
            for ( int round=0; round<400 && !cancelled; ++round )
            {
                table.Settle( 20 );
                GUCEF::vector< MAIL::CMailInboundIntro > list;
                table.P( viewer ).provider->GetPendingIntroductions( list );
                cancelled = 1 == list.size() && MAIL::CMailInboundIntro::STATUS_CANCELLED == list[ 0 ].status && list[ 0 ].request.creatorName == "Alice";
            }
            ASSERT_TRUE( cancelled );
            bool told = false;
            for ( size_t n=0; n<table.P( viewer ).recorder.intros.size(); ++n )
                told = told || table.P( viewer ).recorder.intros[ n ].code == "INTRO_CANCELLED";
            ASSERT_TRUE( told );
        }
        // a cancelled introduction cannot be answered any more
        ASSERT_TRUE( MAIL::MAILINTRORESULT_OK != table.P( 1 ).provider->AcceptIntroduction( id, false ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lobby: a first turn a client refuses makes that participant a spectator on every machine, with the reason; the spectator gets the turns, holds none and does not vote" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Lobby: a first turn a client refuses makes that participant a spectator on every machine, with the reason; the spectator gets the turns, holds none and does not vote" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 4, "lobby-incompatible" ) && table.StartAll() );
        // Bob's client cannot reproduce the first turn
        table.P( 1 ).verifier.rejectFirstOnly = true;
        table.P( 1 ).verifier.refusalCode = "SETTINGS";
        MULTIPLAY::CMatchRecord record = table.MakeRecord( "Lobby", MAIL::MAILTRANSFER_ATTACHMENT, MAIL::MAILDISTRIBUTION_ALL );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.CreateMatch( 0, record, 0, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        const CORE::CString id = record.info.matchId;
        for ( UInt32 i=1; i<4; ++i )
            ASSERT_TRUE( table.WaitKnown( i, id ) );
        ASSERT_TRUE( table.Respond( 2, id, true, result ) && table.Respond( 3, id, true, result ) );

        // everybody sees Bob as a spectator with the reason, and it is the same text on every machine
        for ( UInt32 viewer=0; viewer<4; ++viewer )
            ASSERT_TRUE( WaitStatus( table, viewer, id, 1, MULTIPLAY::MATCHPARTICIPANT_SPECTATOR ) );
        for ( UInt32 viewer=0; viewer<4; ++viewer )
            ASSERT_TRUE( WaitStatus( table, viewer, id, 2, MULTIPLAY::MATCHPARTICIPANT_ACTIVE ) && WaitStatus( table, viewer, id, 3, MULTIPLAY::MATCHPARTICIPANT_ACTIVE ) );
        const CORE::CString expected = Table( table, 0, id );
        for ( UInt32 viewer=1; viewer<4; ++viewer )
            ASSERT_TRUE( Table( table, viewer, id ) == expected );
        MULTIPLAY::CMatchInfo info;
        ASSERT_TRUE( table.P( 0 ).provider->GetMatchInfo( id, info ) );
        const MULTIPLAY::CMatchParticipant* bob = info.FindParticipant( table.P( 1 ).identity.userId );
        ASSERT_TRUE( GUCEF_NULL != bob && bob->IsIncompatible() && "SETTINGS" == bob->statusCode && !bob->statusReason.IsNULLOrEmpty() && bob->statusReason.Length() <= 128 );
        ASSERT_TRUE( 3 == info.GetEligibleVoterCount() );
        bool told = false;
        for ( size_t n=0; n<table.P( 2 ).recorder.lobby.size(); ++n )
            told = told || ( table.P( 2 ).recorder.lobby[ n ].code == "PLAYER_INCOMPATIBLE" && table.P( 2 ).recorder.lobby[ n ].userId == table.P( 1 ).identity.userId );
        ASSERT_TRUE( told );

        // Bob cannot accept any more, cannot propose, and a notice of somebody else about Bob is refused
        MULTIPLAY::CMatchOperationResult answer;
        ASSERT_TRUE( table.Respond( 1, id, true, answer ) && MULTIPLAY::MATCHOPERATION_REJECTED == answer.status );
        MULTIPLAY::CMatchProposal byBob;
        GUCEF::vector< UInt32 > none;
        ASSERT_TRUE( ProposeBeforeStart( table, 1, id, MULTIPLAY::MATCHDECISION_KICK, 3, none, byBob, false ) );
        MULTIPLAY::CMatchProposal aboutCarol;
        aboutCarol.matchId = id;
        aboutCarol.kind = MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR;
        aboutCarol.targetUserId = table.P( 2 ).identity.userId;
        aboutCarol.key = "SETTINGS";
        aboutCarol.value = "forged";
        ASSERT_TRUE( aboutCarol.Sign( table.P( 3 ).identity ) );
        CORE::CDynamicBuffer forgedData;
        ASSERT_TRUE( MAIL::CMailMatchCodec::SerializeToBytes( aboutCarol, forgedData, 64 * 1024 ) );
        ASSERT_TRUE( table.Inject( 0, table.MailTextForData( "dave@mail.test", OneAddress( "alice@mail.test" ), id, 1, MAIL::MAILKIND_INCOMPATIBLE, forgedData, MAIL::CMailMatchCodec::ArmorLabelData, "spectator", MAIL::MAILTRANSFER_ATTACHMENT, "<forged-notice@crafted.test>" ) ) );
        table.Settle( 600 );
        ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_ACTIVE == table.StatusOf( 0, id, 2 ) );

        // the game starts without a force start: nobody is waited for. Bob watches and gets the turn mails
        ASSERT_TRUE( PlayFirstTurnWith( table, 0, id, 2, GUCEF::vector< MULTIPLAY::CMatchDecisionEvidence >(), result ) );
        ASSERT_TRUE( MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitTrusted( 2, id, 2 ) && table.WaitTrusted( 3, id, 2 ) );
        const UInt32 before = table.CountMatchMails( 1, "TURN" );
        ASSERT_TRUE( table.PlayTurn( 2, id, 3, "a move", result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitTrusted( 3, id, 3 ) );
        bool delivered = false;
        PUMP_WAIT( delivered, table.CountMatchMails( 1, "TURN" ) >= before + 1, 8000 );
        ASSERT_TRUE( delivered );
        ASSERT_TRUE( Table( table, 0, id ) == Table( table, 2, id ) && Table( table, 2, id ) == Table( table, 3, id ) );

        // a spectator signs no turn: the turn Bob crafts is refused by everybody
        MULTIPLAY::CMatchPackage package;
        MULTIPLAY::CMatchPreview preview;
        ASSERT_TRUE( table.CraftTurn( table.P( 1 ).identity, 3, id, 0, "bob moves", package, preview ) );
        ASSERT_TRUE( table.Inject( 3, table.TurnMailText( "bob@mail.test", OneAddress( "dave@mail.test" ), id, preview, package, MAIL::MAILTRANSFER_ATTACHMENT, "<spectator-turn@crafted.test>" ) ) );
        table.Settle( 800 );
        ASSERT_TRUE( 3 == table.P( 3 ).provider->GetTrustedRevision( id ) && table.P( 3 ).recorder.CountRefused( "NOT_ON_TURN" ) >= 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lobby: when one person is left to play, every turn of that person and the evidence of the start still go to the spectators" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Lobby: when one person is left to play, every turn of that person and the evidence of the start still go to the spectators" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 4, "lobby-one-human" ) && table.StartAll() );
        GUCEF::vector< UInt32 > accepting;
        accepting.push_back( 2 );
        const CORE::CString id = OpenLobby( table, accepting );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        // Bob and Dave never answer: the table starts without them, with Alice and Carol voting; then Carol decides to watch
        GUCEF::vector< UInt32 > missing;
        missing.push_back( 1 );
        missing.push_back( 3 );
        MULTIPLAY::CMatchProposal proposal;
        ASSERT_TRUE( ProposeBeforeStart( table, 0, id, MULTIPLAY::MATCHDECISION_FORCE_START, -1, missing, proposal ) );
        ASSERT_TRUE( WaitVotes( table, 2, id, proposal.proposalId, 0 ) );
        ASSERT_TRUE( CastVote( table, 2, id, proposal, true ) );
        ASSERT_TRUE( WaitVotes( table, 0, id, proposal.proposalId, 1 ) );
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.P( 2 ).provider->ChooseSpectator( id, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( WaitStatus( table, 0, id, 2, MULTIPLAY::MATCHPARTICIPANT_SPECTATOR ) );
        MULTIPLAY::CMatchDecisionEvidence evidence;
        ASSERT_TRUE( table.P( 0 ).provider->GetDecisionEvidence( id, proposal.proposalId, evidence ) );
        GUCEF::vector< MULTIPLAY::CMatchDecisionEvidence > carried;
        carried.push_back( evidence );

        // Alice is the only person who plays: the next turn is hers again, and the first turn carries the decision to the one who watches
        ASSERT_TRUE( PlayFirstTurnWith( table, 0, id, 0, carried, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitTrusted( 2, id, 2 ) );
        ASSERT_TRUE( WaitStatus( table, 2, id, 1, MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER ) && WaitStatus( table, 2, id, 3, MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER ) );

        // every turn after it too, each one the turn of the same person
        ASSERT_TRUE( table.PlayTurn( 0, id, 0, "a2", result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitTrusted( 2, id, 3 ) );
        ASSERT_TRUE( table.PlayTurn( 0, id, 0, "a3", result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitTrusted( 2, id, 4 ) && Table( table, 0, id ) == Table( table, 2, id ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lobby: a player can choose to watch before the start, the creator cannot, and the vote of somebody who watches afterwards stays counted" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Lobby: a player can choose to watch before the start, the creator cannot, and the vote of somebody who watches afterwards stays counted" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() || !JsonCodecAvailable(), "There is no ed25519 or json codec" );
        CMailTable table;
        ASSERT_TRUE( table.Setup( 4, "lobby-voluntary" ) && table.StartAll() );
        GUCEF::vector< UInt32 > accepting;
        accepting.push_back( 1 );
        accepting.push_back( 2 );
        const CORE::CString id = OpenLobby( table, accepting );
        ASSERT_FALSE( id.IsNULLOrEmpty() );

        // the creator proposes to start with the computer in Dave's seat; Bob and Carol vote yes, then Carol decides to watch
        GUCEF::vector< UInt32 > missing;
        missing.push_back( 3 );
        MULTIPLAY::CMatchProposal proposal;
        ASSERT_TRUE( ProposeBeforeStart( table, 0, id, MULTIPLAY::MATCHDECISION_FORCE_START, -1, missing, proposal ) );
        ASSERT_TRUE( WaitVotes( table, 2, id, proposal.proposalId, 0 ) );
        ASSERT_TRUE( CastVote( table, 2, id, proposal, true ) );
        ASSERT_TRUE( WaitVotes( table, 0, id, proposal.proposalId, 1 ) );
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        MULTIPLAY::CMatchOperationResult result;
        ASSERT_TRUE( table.P( 0 ).provider->ChooseSpectator( id, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );
        ASSERT_TRUE( table.P( 2 ).provider->ChooseSpectator( id, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_OK == result.status );
        for ( UInt32 viewer=0; viewer<4; ++viewer )
            ASSERT_TRUE( WaitStatus( table, viewer, id, 2, MULTIPLAY::MATCHPARTICIPANT_SPECTATOR ) );
        MULTIPLAY::CMatchInfo info;
        ASSERT_TRUE( table.P( 1 ).provider->GetMatchInfo( id, info ) );
        const MULTIPLAY::CMatchParticipant* carol = info.FindParticipant( table.P( 2 ).identity.userId );
        ASSERT_TRUE( GUCEF_NULL != carol && carol->IsSpectator() && !carol->IsIncompatible() );
        bool told = false;
        for ( size_t n=0; n<table.P( 1 ).recorder.lobby.size(); ++n )
            told = told || table.P( 1 ).recorder.lobby[ n ].code == "PLAYER_SPECTATING";
        ASSERT_TRUE( told );

        // the vote was cast when Carol took part: the force start passes on the electorate of the proposal and is carried out
        MULTIPLAY::CMatchDecisionResult tally;
        ASSERT_TRUE( table.P( 0 ).provider->EvaluateDecision( id, proposal.proposalId, tally ) && tally.valid && MULTIPLAY::MATCHVOTEOUTCOME_PASSED == tally.outcome && 3 == tally.eligible );
        MULTIPLAY::CMatchDecisionEvidence evidence;
        ASSERT_TRUE( table.P( 0 ).provider->GetDecisionEvidence( id, proposal.proposalId, evidence ) );
        GUCEF::vector< MULTIPLAY::CMatchDecisionEvidence > carried;
        carried.push_back( evidence );
        ASSERT_TRUE( PlayFirstTurnWith( table, 0, id, 1, carried, result ) );
        ASSERT_TRUE( MULTIPLAY::MATCHOPERATION_OK == result.status );
        ASSERT_TRUE( table.WaitTrusted( 1, id, 2 ) && table.WaitTrusted( 3, id, 2 ) );
        ASSERT_TRUE( WaitStatus( table, 1, id, 3, MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER ) );
        ASSERT_TRUE( MULTIPLAY::MATCHPARTICIPANT_SPECTATOR == table.StatusOf( 1, id, 2 ) );
        ASSERT_TRUE( Table( table, 0, id ) == Table( table, 1, id ) );

        // after the start nobody moves to the spectators this way
        ASSERT_TRUE( table.P( 1 ).provider->ChooseSpectator( id, future ) && table.Await( future, result ) && MULTIPLAY::MATCHOPERATION_REJECTED == result.status );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}
