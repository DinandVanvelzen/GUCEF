/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#include "gucefMULTIPLAY_CMatchDecisionVerifier.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMatchDecisionResult::CMatchDecisionResult( void )
    : valid( false )
    , outcome( MATCHVOTEOUTCOME_UNDECIDED )
    , yes( 0 )
    , no( 0 )
    , needed( 0 )
    , eligible( 0 )
    , reason()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CMatchDecisionVerifier::GetVotesNeeded( UInt32 eligibleCount )
{GUCEF_TRACE;

    return eligibleCount / 2 + 1;
}

/*-------------------------------------------------------------------------*/

TMatchVoteOutcome
CMatchDecisionVerifier::EvaluateTally( UInt32 yes, UInt32 no, UInt32 eligibleCount )
{GUCEF_TRACE;

    UInt32 needed = GetVotesNeeded( eligibleCount );
    if ( yes >= needed )
        return MATCHVOTEOUTCOME_PASSED;

    // It fails once the yes votes that are left cannot reach what is needed
    UInt32 notYetVoted = eligibleCount > ( yes + no ) ? eligibleCount - ( yes + no ) : 0;
    if ( yes + notYetVoted < needed )
        return MATCHVOTEOUTCOME_FAILED;
    return MATCHVOTEOUTCOME_UNDECIDED;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDecisionVerifier::CheckShape( const CMatchInfo& info, const CMatchProposal& proposal, CORE::CString& reason )
{GUCEF_TRACE;

    switch ( proposal.kind )
    {
        case MATCHDECISION_HAND_TO_COMPUTER:
        case MATCHDECISION_DROP_PARTICIPANT:
        {
            const CMatchParticipant* target = info.FindParticipant( proposal.targetUserId );
            if ( GUCEF_NULL == target )
            {
                reason = "The decision is about someone who is not a participant";
                return false;
            }
            if ( MATCHDECISION_HAND_TO_COMPUTER == proposal.kind && MATCHPARTICIPANT_AI_TAKEOVER == target->status )
            {
                reason = "The participant is played by the computer already";
                return false;
            }
            if ( MATCHDECISION_DROP_PARTICIPANT == proposal.kind && MATCHPARTICIPANT_DROPPED == target->status )
            {
                reason = "The participant is dropped already";
                return false;
            }
            return true;
        }
        case MATCHDECISION_CHANGE_CONTACT:
        {
            if ( proposal.targetUserId != proposal.proposerUserId )
            {
                reason = "A contact is only changed by its own participant";
                return false;
            }
            if ( proposal.value.Length() > CMatchParticipant::MaxContactLength )
            {
                reason = "The contact is too long";
                return false;
            }
            return true;
        }
        case MATCHDECISION_SETTING_CHANGE:
        {
            if ( proposal.key.IsNULLOrEmpty() || proposal.key.Length() > CMatchInfo::MaxSettingKeyLength ||
                 proposal.value.Length() > CMatchInfo::MaxSettingValueLength )
            {
                reason = "The setting is not valid";
                return false;
            }
            return true;
        }
        case MATCHDECISION_FORCE_START:
        {
            if ( proposal.targetUserIds.empty() || proposal.targetUserIds.size() > CMatchProposal::MaxListedUsers )
            {
                reason = "A force start names the participants the computer replaces";
                return false;
            }
            CORE::CString::StringSet seen;
            for ( size_t i=0; i<proposal.targetUserIds.size(); ++i )
            {
                const CORE::CString& userId = proposal.targetUserIds[ i ];
                if ( GUCEF_NULL == info.FindParticipant( userId ) || !seen.insert( userId ).second )
                {
                    reason = "A force start names somebody who is not a participant, or names one twice";
                    return false;
                }
                if ( userId == info.creatorUserId || userId == proposal.proposerUserId || proposal.IsInElectorate( userId ) )
                {
                    reason = "A force start does not replace the creator, the proposer or somebody who votes on it";
                    return false;
                }
            }
            return true;
        }
        case MATCHDECISION_KICK:
        {
            if ( GUCEF_NULL == info.FindParticipant( proposal.targetUserId ) )
            {
                reason = "The decision is about someone who is not a participant";
                return false;
            }
            if ( proposal.targetUserId == info.creatorUserId || proposal.targetUserId == proposal.proposerUserId )
            {
                reason = "Neither the creator nor the proposer is kicked";
                return false;
            }
            return true;
        }
        case MATCHDECISION_CANCEL:
            return true;
        case MATCHDECISION_MOVE_TO_SPECTATOR:
        {
            if ( GUCEF_NULL == info.FindParticipant( proposal.targetUserId ) )
            {
                reason = "The decision is about someone who is not a participant";
                return false;
            }
            if ( proposal.targetUserId == info.creatorUserId )
            {
                reason = "The creator hosts the match and does not watch";
                return false;
            }
            if ( !IsValidReasonCode( proposal.key ) || proposal.value.Length() > CMatchParticipant::MaxStatusReasonLength )
            {
                reason = "The reason for watching is not valid";
                return false;
            }
            for ( UInt32 i=0; i<proposal.value.Length(); ++i )
                if ( (UInt8) proposal.value[ i ] < 0x20 )
                {
                    reason = "The reason for watching is not valid";
                    return false;
                }
            return true;
        }
        default:
            break;
    }
    reason = "Unknown kind of decision";
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDecisionVerifier::IsPreStartKind( TMatchDecisionKind kind )
{GUCEF_TRACE;

    return MATCHDECISION_FORCE_START == kind || MATCHDECISION_KICK == kind || MATCHDECISION_CANCEL == kind || MATCHDECISION_MOVE_TO_SPECTATOR == kind;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDecisionVerifier::IsValidReasonCode( const CORE::CString& code )
{GUCEF_TRACE;

    if ( code.IsNULLOrEmpty() || code.Length() > CMatchParticipant::MaxStatusCodeLength )
        return false;
    for ( UInt32 i=0; i<code.Length(); ++i )
    {
        const char c = code[ i ];
        if ( !( ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '_' == c ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The electorate of the proposal, when it has one: participants of the match, each once, the proposer among them */
bool
CMatchDecisionVerifier::CheckElectorate( const CMatchInfo& info, const CMatchProposal& proposal, CORE::CString& reason )
{GUCEF_TRACE;

    if ( proposal.voterUserIds.empty() )
        return true;
    if ( proposal.voterUserIds.size() > CMatchProposal::MaxListedUsers )
    {
        reason = "The proposal names more voters than a match has participants";
        return false;
    }
    CORE::CString::StringSet seen;
    for ( size_t i=0; i<proposal.voterUserIds.size(); ++i )
        if ( GUCEF_NULL == info.FindParticipant( proposal.voterUserIds[ i ] ) || !seen.insert( proposal.voterUserIds[ i ] ).second )
        {
            reason = "The proposal names a voter who is not a participant, or names one twice";
            return false;
        }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDecisionVerifier::Evaluate( const CMatchInfo& info, const CMatchDecisionEvidence& evidence, Int64 nowMs, CMatchDecisionResult& result, bool started )
{GUCEF_TRACE;

    result = CMatchDecisionResult();
    const CMatchProposal& proposal = evidence.proposal;
    const bool hasElectorate = !proposal.voterUserIds.empty();
    result.eligible = hasElectorate ? (UInt32) proposal.voterUserIds.size() : info.GetEligibleVoterCount();
    result.needed = GetVotesNeeded( result.eligible );

    if ( proposal.matchId != info.matchId )
    {
        result.reason = "The decision is for another match";
        return false;
    }
    if ( proposal.proposalId.IsNULLOrEmpty() )
    {
        result.reason = "The proposal has no id";
        return false;
    }
    const bool preStart = IsPreStartKind( proposal.kind );
    if ( MATCHSTATUS_CANCELLED == info.status )
    {
        result.reason = "The match was cancelled";
        return false;
    }
    if ( preStart && started )
    {
        result.reason = "This decision can only be taken before the match starts";
        return false;
    }
    const CMatchParticipant* proposer = info.FindParticipant( proposal.proposerUserId );
    if ( GUCEF_NULL == proposer )
    {
        result.reason = "The proposer does not take part in the match";
        return false;
    }
    if ( !proposal.VerifySignature( info ) )
    {
        result.reason = "The proposal is not signed by its proposer";
        return false;
    }
    if ( !CheckElectorate( info, proposal, result.reason ) )
        return false;
    if ( !CheckShape( info, proposal, result.reason ) )
        return false;
    if ( proposal.deadline != 0 && proposal.deadline < proposal.createdAt )
    {
        result.reason = "The proposal ends before it starts";
        return false;
    }

    // a participant decides about the own contact alone
    if ( MATCHDECISION_CHANGE_CONTACT == proposal.kind )
    {
        if ( !proposer->IsEligibleVoter() )
        {
            result.reason = "The proposer does not take part in the match";
            return false;
        }
        result.valid = true;
        result.yes = 1;
        result.outcome = MATCHVOTEOUTCOME_PASSED;
        return true;
    }

    // closing the match: under the BOSS policy the creator or a boss decides alone, under VOTE it is a vote like any other (the creator is a participant
    // like the rest: the creator alone passes it only when nobody else takes part in the vote)
    if ( MATCHDECISION_CANCEL == proposal.kind && MATCHDECISIONPOLICY_BOSS == info.decisionPolicy )
    {
        const bool isCreator = proposer->userId == info.creatorUserId;
        const bool isBoss = proposer->boss && ( MATCHPARTICIPANT_ACTIVE == proposer->status || MATCHPARTICIPANT_INVITED == proposer->status );
        if ( !isCreator && !isBoss )
        {
            result.reason = "Only the creator, or a boss, closes the match";
            return false;
        }
        result.valid = true;
        result.yes = 1;
        result.outcome = MATCHVOTEOUTCOME_PASSED;
        return true;
    }

    // a participant who cannot or will not play says so with the own signature alone
    if ( MATCHDECISION_MOVE_TO_SPECTATOR == proposal.kind && proposal.targetUserId == proposal.proposerUserId )
    {
        result.valid = true;
        result.yes = 1;
        result.outcome = MATCHVOTEOUTCOME_PASSED;
        return true;
    }

    // who proposes: a participant who votes, and for a decision before the start the electorate the proposal names
    if ( preStart && !hasElectorate && MATCHDECISIONPOLICY_BOSS != info.decisionPolicy )
    {
        result.reason = "A decision of the table before the start names the participants who vote";
        return false;
    }
    if ( hasElectorate ? !proposal.IsInElectorate( proposal.proposerUserId ) : !proposer->IsEligibleVoter() )
    {
        if ( !( MATCHDECISIONPOLICY_BOSS == info.decisionPolicy && preStart && proposer->boss && !hasElectorate ) )
        {
            result.reason = "The proposer does not take part in the match";
            return false;
        }
    }

    // a boss decides alone
    if ( MATCHDECISIONPOLICY_BOSS == info.decisionPolicy )
    {
        const bool bossStands = preStart ? ( MATCHPARTICIPANT_ACTIVE == proposer->status || MATCHPARTICIPANT_INVITED == proposer->status ) : proposer->IsEligibleVoter();
        if ( !proposer->boss || !bossStands )
        {
            result.reason = "In this match only a boss decides";
            return false;
        }
        result.valid = true;
        result.yes = 1;
        result.outcome = MATCHVOTEOUTCOME_PASSED;
        return true;
    }

    // a vote: the proposer counts as a yes
    if ( evidence.votes.size() > CMatchDecisionEvidence::MaxVotes )
    {
        result.reason = "There are more votes than participants";
        return false;
    }
    CORE::CString::StringSet voted;
    bool proposerVoted = false;
    UInt32 yes = 1;
    UInt32 no = 0;
    const CORE::CString proposalDigest = proposal.GetDigest();
    for ( size_t i=0; i<evidence.votes.size(); ++i )
    {
        const CMatchVote& vote = evidence.votes[ i ];
        if ( vote.matchId != info.matchId || vote.proposalId != proposal.proposalId || vote.proposalDigest != proposalDigest )
        {
            result.reason = "A vote is for another proposal";
            return false;
        }
        const CMatchParticipant* voter = info.FindParticipant( vote.voterUserId );
        if ( GUCEF_NULL == voter || ( hasElectorate ? !proposal.IsInElectorate( vote.voterUserId ) : !voter->IsEligibleVoter() ) )
        {
            result.reason = "A vote is by someone who does not vote in this match";
            return false;
        }
        if ( !vote.VerifySignature( info ) )
        {
            result.reason = "A vote is not signed by its voter";
            return false;
        }
        if ( vote.createdAt < proposal.createdAt || ( proposal.deadline != 0 && vote.createdAt > proposal.deadline ) )
        {
            result.reason = "A vote was cast outside the time of the proposal";
            return false;
        }
        if ( vote.voterUserId == proposal.proposerUserId )
        {
            // the proposer counts as a yes already: a yes of the proposer is the same vote, a no contradicts the proposal
            if ( proposerVoted || !vote.yes )
            {
                result.reason = proposerVoted ? "A participant voted twice" : "The proposer voted against the own proposal";
                return false;
            }
            proposerVoted = true;
            continue;
        }
        if ( voted.find( vote.voterUserId ) != voted.end() )
        {
            result.reason = "A participant voted twice";
            return false;
        }
        voted.insert( vote.voterUserId );
        if ( vote.yes )
            ++yes;
        else
            ++no;
    }

    result.valid = true;
    result.yes = yes;
    result.no = no;
    result.outcome = EvaluateTally( yes, no, result.eligible );
    if ( MATCHVOTEOUTCOME_UNDECIDED == result.outcome && proposal.deadline != 0 && nowMs > proposal.deadline )
        result.outcome = MATCHVOTEOUTCOME_FAILED;
    if ( MATCHVOTEOUTCOME_PASSED != result.outcome )
        result.reason = MATCHVOTEOUTCOME_FAILED == result.outcome ? "The proposal did not pass" : "The proposal is still open";
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDecisionVerifier::Verify( const CMatchInfo& info, const CMatchDecisionEvidence& evidence, Int64 nowMs, CORE::CString& reason, bool started )
{GUCEF_TRACE;

    CMatchDecisionResult result;
    Evaluate( info, evidence, nowMs, result, started );
    reason = result.reason;
    return result.valid && MATCHVOTEOUTCOME_PASSED == result.outcome;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDecisionVerifier::ApplyDecision( CMatchInfo& info, const CMatchProposal& proposal, CORE::CString& reason )
{GUCEF_TRACE;

    if ( proposal.matchId != info.matchId || !CheckShape( info, proposal, reason ) )
    {
        if ( reason.IsNULLOrEmpty() )
            reason = "The decision is for another match";
        return false;
    }
    switch ( proposal.kind )
    {
        case MATCHDECISION_HAND_TO_COMPUTER:
            info.FindParticipant( proposal.targetUserId )->status = MATCHPARTICIPANT_AI_TAKEOVER;
            return true;
        case MATCHDECISION_DROP_PARTICIPANT:
            info.FindParticipant( proposal.targetUserId )->status = MATCHPARTICIPANT_DROPPED;
            return true;
        case MATCHDECISION_CHANGE_CONTACT:
            info.FindParticipant( proposal.targetUserId )->contact = proposal.value;
            return true;
        case MATCHDECISION_SETTING_CHANGE:
            if ( info.settings.find( proposal.key ) == info.settings.end() && info.settings.size() >= CMatchInfo::MaxSettings )
            {
                reason = "The match has too many settings";
                return false;
            }
            info.SetSetting( proposal.key, proposal.value );
            return true;
        case MATCHDECISION_FORCE_START:
            // whatever status a participant has on this machine: somebody who is not playing is replaced; a spectator or a kicked participant stays what they are
            for ( size_t i=0; i<proposal.targetUserIds.size(); ++i )
            {
                CMatchParticipant* target = info.FindParticipant( proposal.targetUserIds[ i ] );
                if ( MATCHPARTICIPANT_INVITED == target->status || MATCHPARTICIPANT_ACTIVE == target->status || MATCHPARTICIPANT_DECLINED == target->status )
                    target->status = MATCHPARTICIPANT_AI_TAKEOVER;
            }
            return true;
        case MATCHDECISION_KICK:
        {
            CMatchParticipant* target = info.FindParticipant( proposal.targetUserId );
            if ( MATCHPARTICIPANT_LEFT != target->status )
                target->status = MATCHPARTICIPANT_DROPPED;
            return true;
        }
        case MATCHDECISION_CANCEL:
            info.status = MATCHSTATUS_CANCELLED;
            info.closedByUserId = proposal.proposerUserId;
            return true;
        case MATCHDECISION_MOVE_TO_SPECTATOR:
        {
            // kicked stays kicked, whatever the order in which the two decisions reached this machine
            CMatchParticipant* target = info.FindParticipant( proposal.targetUserId );
            if ( MATCHPARTICIPANT_INVITED == target->status || MATCHPARTICIPANT_ACTIVE == target->status || MATCHPARTICIPANT_AI_TAKEOVER == target->status )
            {
                target->status = MATCHPARTICIPANT_SPECTATOR;
                target->statusCode = proposal.key;
                target->statusReason = proposal.value;
            }
            return true;
        }
        default:
            break;
    }
    reason = "Unknown kind of decision";
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
