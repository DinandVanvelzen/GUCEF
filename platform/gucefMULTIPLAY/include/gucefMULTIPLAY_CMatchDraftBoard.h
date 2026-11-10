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

#ifndef GUCEF_MULTIPLAY_CMATCHDRAFTBOARD_H
#define GUCEF_MULTIPLAY_CMATCHDRAFTBOARD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CMATCHROSTER_H
#include "gucefMULTIPLAY_CMatchRoster.h"
#define GUCEF_MULTIPLAY_CMATCHROSTER_H
#endif /* GUCEF_MULTIPLAY_CMATCHROSTER_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHDECISIONVERIFIER_H
#include "gucefMULTIPLAY_CMatchDecisionVerifier.h"
#define GUCEF_MULTIPLAY_CMATCHDECISIONVERIFIER_H
#endif /* GUCEF_MULTIPLAY_CMATCHDECISIONVERIFIER_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHDRAFTTALLY_H
#include "gucefMULTIPLAY_CMatchDraftTally.h"
#define GUCEF_MULTIPLAY_CMATCHDRAFTTALLY_H
#endif /* GUCEF_MULTIPLAY_CMATCHDRAFTTALLY_H ? */

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

/** What became of a roster, a proposal or a vote that was handed to a board */
enum EMatchDraftResult
{
    MATCHDRAFT_OK = 0 ,
    MATCHDRAFT_DUPLICATE ,          /**< the same thing again: ignored */
    MATCHDRAFT_PENDING ,            /**< kept: it refers to a roster (or a proposal) that has not arrived yet */
    MATCHDRAFT_STALE ,              /**< a roster older than the one held */
    MATCHDRAFT_EQUIVOCATION ,       /**< a second, different roster of the host for a revision that is known: the first stays, the host is shown as having sent two */
    MATCHDRAFT_BAD_SIGNATURE ,
    MATCHDRAFT_WRONG_HOST ,         /**< the roster is signed by a key that is not the key of the host of this game */
    MATCHDRAFT_WRONG_MATCH ,
    MATCHDRAFT_MALFORMED ,
    MATCHDRAFT_KEY_CHANGED ,        /**< valid, but it gives another key to somebody than an earlier roster did: held until the player decides */
    MATCHDRAFT_OWN_KEY_MISMATCH ,   /**< the roster lists the player under another key than the player's own: not believed */
    MATCHDRAFT_NOT_ALLOWED ,        /**< a kind of decision that is not taken in the draft stage, or a proposal by somebody who may not make it */
    MATCHDRAFT_NOT_MEMBER ,         /**< a vote by somebody who is not in the electorate of the proposal */
    MATCHDRAFT_ROSTER_MISMATCH ,    /**< the proposal names the digest of another roster than the one that revision is */
    MATCHDRAFT_NO_PROPOSAL ,
    MATCHDRAFT_CONFLICT ,           /**< a voter voted twice with different answers: the first stays */
    MATCHDRAFT_LIMIT ,
    MATCHDRAFT_NOT_READY ,          /**< no roster is held yet */
    MATCHDRAFT_UNAUTHORIZED_REMOVAL /**< a game decided by vote: the roster takes somebody off the table without a decision that passed (and names that player). The roster is not taken */
};
typedef enum EMatchDraftResult TMatchDraftResult;

GUCEF_MULTIPLAY_EXPORT_CPP const char* MatchDraftResultToString( TMatchDraftResult result );

GUCEF_MULTIPLAY_EXPORT_CPP const char* MatchDraftStateToString( TMatchDraftState state );

/*-------------------------------------------------------------------------*/

/**
 *  The decisions of a game that has no match record yet, from the point of view of one participant (the host included).
 *
 *  The board holds the rosters of the host that were accepted (the newest and the ones the proposals refer to), the signed proposals and the signed
 *  votes, and tells how each proposal stands. There is no mail, no clock and no file in here: the caller hands in what arrived and the time.
 *
 *  Rosters. A roster is accepted when it is signed by the key of the host of this game, is not older than the one held, and does not list the player under
 *  another key. A roster of the same revision as one that is held but different (the host signed two) is not accepted: the first stays and the board shows that
 *  the host sent two (IsHostEquivocating()). A roster that gives somebody another key than an earlier roster did is held (HasHeldRoster()) until the player
 *  confirms it, as a changed key is held in the introduction handshake. The keys of the other participants are learned from rosters only.
 *
 *  Decisions. The kinds taken in the draft stage are FORCE_START (replaces the participants that have not answered by the computer), KICK and the
 *  participant's own MOVE_TO_SPECTATOR. A proposal carries the revision and digest of the roster it refers to and the electorate of that revision
 *  (the participants who are introduced and do not watch, which is exactly what the roster of that revision says), signed. The vote and BOSS rules are those of
 *  CMatchDecisionVerifier: more than half of the electorate pass a proposal, a tie changes nothing, the proposer counts as a yes; under BOSS the host decides.
 *  A proposal or a vote that arrives before the roster or proposal it refers to is kept (a bounded number) and counted when that arrives.
 *
 *  The host carries out what passed (GetPassedForApplication(), MarkApplied()) and names it in its next roster, with the evidence. Every other participant
 *  counts for itself and shows PASSED, APPLIED, HOST_IGNORED (passed, and the host has not carried it out within the grace time) or HOST_MISCOUNT.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchDraftBoard : public CORE::CIDataNodeSerializable
{
    public:

    static const UInt32 MaxRosterHistory = 16;
    static const UInt32 MaxProposals = 32;
    static const UInt32 MaxProposalsPerProposer = 8;
    static const UInt32 MaxVotesPerProposal = CMatchRoster::MaxEntries;
    static const UInt32 MaxPendingProposals = 16;
    static const UInt32 MaxPendingVotes = 64;
    static const Int64 DefaultApplyGraceMs = 24LL * 60 * 60 * 1000;

    CMatchDraftBoard( void );

    CMatchDraftBoard( const CMatchDraftBoard& src );

    virtual ~CMatchDraftBoard();

    CMatchDraftBoard& operator=( const CMatchDraftBoard& src );

    /** The board of the host of a game: the identity signs the rosters */
    bool InitializeAsHost( const CPlayerIdentity& host, const CORE::CString& matchId );

    /** The board of a participant that knows the host from the introduction: the user id and key of the host and the player's own */
    bool InitializeAsParticipant( const CORE::CString& matchId, const CORE::CString& hostUserId, const CORE::CString& hostPublicKey, const CPlayerIdentity& own );

    /** The contact (the mail address) the player is listed under: a roster that lists this contact under another key than the player's own is not believed */
    void SetOwnContact( const CORE::CString& contact );

    bool IsHost( void ) const;

    const CORE::CString& GetMatchId( void ) const;

    /** How long a decision that passed may wait for the host to carry it out before it is shown as ignored */
    void SetApplyGraceMs( Int64 graceMs );

    /* ---- rosters ---- */

    /** A roster that arrived (a participant). The caller says what to tell the player from the result */
    TMatchDraftResult AcceptRoster( const CMatchRoster& roster, Int64 nowMs );

    /** The roster that waits for the player's decision (KEY_CHANGED) is taken or refused. False when none waits */
    bool ConfirmHeldRoster( bool accept, Int64 nowMs );

    bool HasHeldRoster( void ) const;

    /** A roster of the host was refused because it took somebody off the table of a game decided by vote without a decision that passed: the player is warned */
    bool HasUnauthorizedRemoval( void ) const;

    /** What the held roster changes: for every participant whose key differs, the contact and both keys */
    void GetHeldKeyChanges( GUCEF::vector< CORE::CString >& contacts, GUCEF::vector< CORE::CString >& oldKeys, GUCEF::vector< CORE::CString >& newKeys ) const;

    bool HasRoster( void ) const;

    UInt32 GetRevision( void ) const;

    bool GetRoster( CMatchRoster& roster ) const;

    bool GetRosterOfRevision( UInt32 revision, CMatchRoster& roster ) const;

    /** The host signed two different rosters for one revision: both digests */
    bool IsHostEquivocating( void ) const;

    bool GetEquivocation( UInt32& revision, CORE::CString& firstDigest, CORE::CString& secondDigest ) const;

    /**
     *  The host makes its next roster from the table of the draft: a new revision, signed, kept as the roster of the board and handed back. False (and nothing
     *  changes) when the table says what the roster held says and no decision was carried out since. 'entries' has the host first
     */
    bool PublishRoster( const CPlayerIdentity& host, const TMatchRosterEntryVector& entries, TMatchDecisionPolicy policy, const CORE::CString& settingsDigest, Int64 nowMs, CMatchRoster& published );

    /* ---- decisions ---- */

    /** Fills in a proposal of the player from the roster that is held (revision, digest, electorate, id, time) and signs it. 'kind' is FORCE_START (the targets are the participants that have not answered), KICK or MOVE_TO_SPECTATOR (the player's own) */
    bool MakeProposal( const CPlayerIdentity& own, TMatchDecisionKind kind, const CORE::CString& targetUserId, Int64 nowMs, Int64 durationMs, CMatchProposal& proposal, CORE::CString& reason ) const;

    TMatchDraftResult AddProposal( const CMatchProposal& proposal, Int64 nowMs, CORE::CString& reason );

    TMatchDraftResult AddVote( const CMatchVote& vote, Int64 nowMs, CORE::CString& reason );

    UInt32 GetProposals( GUCEF::vector< CMatchProposal >& proposals ) const;

    bool GetProposal( const CORE::CString& proposalId, CMatchProposal& proposal ) const;

    bool GetVote( const CORE::CString& proposalId, const CORE::CString& voterUserId, CMatchVote& vote ) const;

    /** The proposal and the votes cast on it */
    bool GetEvidence( const CORE::CString& proposalId, CMatchDecisionEvidence& evidence ) const;

    bool GetTally( const CORE::CString& proposalId, Int64 nowMs, CMatchDraftTally& tally ) const;

    UInt32 GetTallies( Int64 nowMs, GUCEF::vector< CMatchDraftTally >& tallies ) const;

    /** The proposals that passed, still apply to the table and have not been carried out (the host) */
    UInt32 GetPassedForApplication( Int64 nowMs, GUCEF::vector< CMatchDraftTally >& passed ) const;

    /** The host carried the decision out: it is named in the next roster with its evidence */
    bool MarkApplied( const CORE::CString& proposalId );

    /** Whether a roster of the host names the proposal as carried out */
    bool IsApplied( const CORE::CString& proposalDigest ) const;

    /** The host: the participants who watch by their own statement that the host carried out (the roster of the host lists them as watching) */
    UInt32 GetAppliedSpectators( GUCEF::vector< CORE::CString >& userIds ) const;

    /** The statements of the participants who watch, as the roster in force says: they go on into the match record, where they are decisions of the table before the start */
    UInt32 GetSpectatorStatements( GUCEF::vector< CMatchProposal >& statements ) const;

    /** Pending proposals and votes that wait for what they refer to */
    UInt32 GetPendingCount( void ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    private:

    struct SStored
    {
        CMatchProposal proposal;
        GUCEF::vector< CMatchVote > votes;
        Int64 passedSeenAt;
        bool appliedByUs;                   /**< the host's side: carried out, to be named in the next roster */
        SStored( void ) : proposal(), votes(), passedSeenAt( 0 ), appliedByUs( false ) {}
    };

    typedef GUCEF::vector< SStored > TStoredVector;
    typedef GUCEF::vector< CMatchRoster > TRosterVector;

    const CMatchRoster* FindRoster( UInt32 revision ) const;
    SStored* FindStored( const CORE::CString& proposalId );
    const SStored* FindStored( const CORE::CString& proposalId ) const;
    bool IsRemovalAuthorized( const CMatchRoster& roster, Int64 nowMs ) const;
    bool CheckKeyChanges( const CMatchRoster& roster, GUCEF::vector< CORE::CString >& contacts, GUCEF::vector< CORE::CString >& oldKeys, GUCEF::vector< CORE::CString >& newKeys ) const;
    void TakeRoster( const CMatchRoster& roster, Int64 nowMs );
    void RememberRoster( const CMatchRoster& roster );
    void Retry( Int64 nowMs );
    bool Evaluate( const SStored& stored, Int64 nowMs, CMatchDecisionResult& result ) const;
    void Refresh( Int64 nowMs );
    void MakeTally( const SStored& stored, Int64 nowMs, CMatchDraftTally& tally ) const;
    bool StillApplies( const CMatchProposal& proposal ) const;
    TMatchDraftResult CheckProposal( const CMatchProposal& proposal, Int64 nowMs, CORE::CString& reason ) const;

    private:

    CORE::CString m_matchId;
    CORE::CString m_hostUserId;
    CORE::CString m_hostPublicKey;
    CORE::CString m_ownUserId;
    CORE::CString m_ownPublicKey;
    CORE::CString m_ownContact;
    bool m_isHost;
    Int64 m_applyGraceMs;
    TRosterVector m_rosters;                                    /**< oldest first; the last is the roster in force */
    bool m_hasHeld;
    bool m_unauthorizedRemoval;
    CMatchRoster m_held;
    bool m_equivocating;
    UInt32 m_equivocationRevision;
    CORE::CString m_equivocationFirst;
    CORE::CString m_equivocationSecond;
    TStoredVector m_stored;
    GUCEF::vector< CMatchProposal > m_pendingProposals;
    GUCEF::vector< CMatchVote > m_pendingVotes;
    GUCEF::vector< CORE::CString > m_applied;                   /**< digests of the proposals the host named as carried out */
    GUCEF::vector< CORE::CString > m_appliedWithEvidence;       /**< of those, the ones whose evidence came with the roster */
    GUCEF::vector< Int64 > m_appliedSeenAt;                     /**< parallel to m_applied */
    GUCEF::vector< CORE::CString > m_toName;                    /**< the host: proposal ids carried out and not named in a roster yet */
    CPlayerIdentity m_identity;                                 /**< the host's, for the rosters; a copy */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHDRAFTBOARD_H ? */
