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

#ifndef GUCEF_MULTIPLAY_CMATCHDECISIONVERIFIER_H
#define GUCEF_MULTIPLAY_CMATCHDECISIONVERIFIER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CMATCHDECISIONEVIDENCE_H
#include "gucefMULTIPLAY_CMatchDecisionEvidence.h"
#define GUCEF_MULTIPLAY_CMATCHDECISIONEVIDENCE_H
#endif /* GUCEF_MULTIPLAY_CMATCHDECISIONEVIDENCE_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHDECISIONRESULT_H
#include "gucefMULTIPLAY_CMatchDecisionResult.h"
#define GUCEF_MULTIPLAY_CMATCHDECISIONRESULT_H
#endif /* GUCEF_MULTIPLAY_CMATCHDECISIONRESULT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Decides whether the evidence of a decision is good, from the participants of the match and their pinned keys
 *  alone (a pure function: no clock besides the time given, no state).
 *
 *  The rules are those of a lobby (CLobbyRules::GetVotesNeeded() and EvaluateVote()): more than half of the
 *  participants that vote pass a proposal, so an equal number of yes and no votes changes nothing; a proposal
 *  fails as soon as the votes that are left cannot pass it, or when its time (deadline) is over without a pass;
 *  the proposer counts as a yes. In a match with the BOSS decision policy the signed command of a participant
 *  that is a boss is enough. The change of the contact of a player is that player's own decision: the player's
 *  signature alone, whatever the policy.
 *
 *  The decisions before the start (force start, kick, cancel, move to spectator) are valid only while no turn of the game was played ('started'
 *  false): a force start replaces the participants named in the proposal by the computer, a kick removes a participant, a cancel closes the match,
 *  and a move to spectator is the participant's own signature alone or is decided like a kick. Who acts alone follows the policy of the match, as in a
 *  lobby: under BOSS a boss (the creator is one) decides each of these alone; under VOTE nobody does, the creator included: each of them needs the
 *  electorate in the proposal and more than half of it (the proposer counts as a yes, so the creator alone passes one only when the electorate is the creator). The electorate of a vote is the one the signed proposal names (CMatchProposal::voterUserIds), not the status of the participants
 *  when the evidence is checked, so the outcome is the same on every machine whatever the order in which acceptances and notices arrived.
 *
 *  Evidence is refused as a whole when it contains anything that is not right: a vote twice, a vote by someone
 *  who does not vote in the match, a vote for another proposal or match, a bad signature, a vote after the
 *  deadline, a vote of the proposer that is a no.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchDecisionVerifier
{
    public:

    /** The yes votes that pass: more than half of the participants that vote */
    static UInt32 GetVotesNeeded( UInt32 eligibleCount );

    /** Passed, failed (it cannot pass any more) or still open, as a lobby decides it */
    static TMatchVoteOutcome EvaluateTally( UInt32 yes, UInt32 no, UInt32 eligibleCount );

    /**
     *  @param nowMs the time in ms since the Unix epoch, used to end a proposal whose deadline has passed
     *  @return result.valid
     */
    static bool Evaluate( const CMatchInfo& info, const CMatchDecisionEvidence& evidence, Int64 nowMs, CMatchDecisionResult& result, bool started = false );

    /** Whether the evidence is valid and the decision passed. 'started': a turn of the game was played already, which ends the decisions that are only for the time before */
    static bool Verify( const CMatchInfo& info, const CMatchDecisionEvidence& evidence, Int64 nowMs, CORE::CString& reason, bool started = false );

    /** Whether the kind is only for the time before the match starts (force start, kick, cancel, move to spectator) */
    static bool IsPreStartKind( TMatchDecisionKind kind );

    /** Whether the reason code of a move to spectator is a short upper case word */
    static bool IsValidReasonCode( const CORE::CString& code );

    /** Carries out a decision that was verified: the participant is handed to the computer or dropped, a contact or setting is changed */
    static bool ApplyDecision( CMatchInfo& info, const CMatchProposal& proposal, CORE::CString& reason );

    private:

    static bool CheckShape( const CMatchInfo& info, const CMatchProposal& proposal, CORE::CString& reason );
    static bool CheckElectorate( const CMatchInfo& info, const CMatchProposal& proposal, CORE::CString& reason );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHDECISIONVERIFIER_H ? */
