/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHSIGNING_H
#include "gucefMULTIPLAY_CMatchSigning.h"
#define GUCEF_MULTIPLAY_CMATCHSIGNING_H
#endif /* GUCEF_MULTIPLAY_CMATCHSIGNING_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHID_H
#include "gucefMULTIPLAY_CMatchId.h"
#define GUCEF_MULTIPLAY_CMATCHID_H
#endif /* GUCEF_MULTIPLAY_CMATCHID_H ? */

#include "gucefMULTIPLAY_CMatchDraftBoard.h"

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

const char*
MatchDraftResultToString( TMatchDraftResult result )
{GUCEF_TRACE;

    switch ( result )
    {
        case MATCHDRAFT_OK:                 return "OK";
        case MATCHDRAFT_DUPLICATE:          return "DUPLICATE";
        case MATCHDRAFT_PENDING:            return "PENDING";
        case MATCHDRAFT_STALE:              return "STALE";
        case MATCHDRAFT_EQUIVOCATION:       return "EQUIVOCATION";
        case MATCHDRAFT_BAD_SIGNATURE:      return "BAD_SIGNATURE";
        case MATCHDRAFT_WRONG_HOST:         return "WRONG_HOST";
        case MATCHDRAFT_WRONG_MATCH:        return "WRONG_MATCH";
        case MATCHDRAFT_MALFORMED:          return "MALFORMED";
        case MATCHDRAFT_KEY_CHANGED:        return "KEY_CHANGED";
        case MATCHDRAFT_OWN_KEY_MISMATCH:   return "OWN_KEY_MISMATCH";
        case MATCHDRAFT_NOT_ALLOWED:        return "NOT_ALLOWED";
        case MATCHDRAFT_NOT_MEMBER:         return "NOT_MEMBER";
        case MATCHDRAFT_ROSTER_MISMATCH:    return "ROSTER_MISMATCH";
        case MATCHDRAFT_NO_PROPOSAL:        return "NO_PROPOSAL";
        case MATCHDRAFT_CONFLICT:           return "CONFLICT";
        case MATCHDRAFT_LIMIT:              return "LIMIT";
        case MATCHDRAFT_UNAUTHORIZED_REMOVAL: return "UNAUTHORIZED_REMOVAL";
        default:                            return "NOT_READY";
    }
}

/*-------------------------------------------------------------------------*/

const char*
MatchDraftStateToString( TMatchDraftState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case MATCHDRAFTSTATE_OPEN:          return "OPEN";
        case MATCHDRAFTSTATE_PASSED:        return "PASSED";
        case MATCHDRAFTSTATE_FAILED:        return "FAILED";
        case MATCHDRAFTSTATE_APPLIED:       return "APPLIED";
        case MATCHDRAFTSTATE_HOST_IGNORED:  return "HOST_IGNORED";
        case MATCHDRAFTSTATE_HOST_MISCOUNT: return "HOST_MISCOUNT";
        default:                            return "OBSOLETE";
    }
}

/*-------------------------------------------------------------------------*/

CMatchDraftBoard::CMatchDraftBoard( void )
    : CORE::CIDataNodeSerializable()
    , m_matchId()
    , m_hostUserId()
    , m_hostPublicKey()
    , m_ownUserId()
    , m_ownPublicKey()
    , m_ownContact()
    , m_isHost( false )
    , m_applyGraceMs( DefaultApplyGraceMs )
    , m_rosters()
    , m_hasHeld( false )
    , m_unauthorizedRemoval( false )
    , m_held()
    , m_equivocating( false )
    , m_equivocationRevision( 0 )
    , m_equivocationFirst()
    , m_equivocationSecond()
    , m_stored()
    , m_pendingProposals()
    , m_pendingVotes()
    , m_applied()
    , m_appliedWithEvidence()
    , m_appliedSeenAt()
    , m_toName()
    , m_identity()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchDraftBoard::CMatchDraftBoard( const CMatchDraftBoard& src )
    : CORE::CIDataNodeSerializable( src )
    , m_matchId( src.m_matchId )
    , m_hostUserId( src.m_hostUserId )
    , m_hostPublicKey( src.m_hostPublicKey )
    , m_ownUserId( src.m_ownUserId )
    , m_ownPublicKey( src.m_ownPublicKey )
    , m_ownContact( src.m_ownContact )
    , m_isHost( src.m_isHost )
    , m_applyGraceMs( src.m_applyGraceMs )
    , m_rosters( src.m_rosters )
    , m_hasHeld( src.m_hasHeld )
    , m_unauthorizedRemoval( src.m_unauthorizedRemoval )
    , m_held( src.m_held )
    , m_equivocating( src.m_equivocating )
    , m_equivocationRevision( src.m_equivocationRevision )
    , m_equivocationFirst( src.m_equivocationFirst )
    , m_equivocationSecond( src.m_equivocationSecond )
    , m_stored( src.m_stored )
    , m_pendingProposals( src.m_pendingProposals )
    , m_pendingVotes( src.m_pendingVotes )
    , m_applied( src.m_applied )
    , m_appliedWithEvidence( src.m_appliedWithEvidence )
    , m_appliedSeenAt( src.m_appliedSeenAt )
    , m_toName( src.m_toName )
    , m_identity( src.m_identity )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchDraftBoard::~CMatchDraftBoard()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchDraftBoard&
CMatchDraftBoard::operator=( const CMatchDraftBoard& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_matchId = src.m_matchId;
        m_hostUserId = src.m_hostUserId;
        m_hostPublicKey = src.m_hostPublicKey;
        m_ownUserId = src.m_ownUserId;
        m_ownPublicKey = src.m_ownPublicKey;
        m_ownContact = src.m_ownContact;
        m_isHost = src.m_isHost;
        m_applyGraceMs = src.m_applyGraceMs;
        m_rosters = src.m_rosters;
        m_hasHeld = src.m_hasHeld;
        m_unauthorizedRemoval = src.m_unauthorizedRemoval;
        m_held = src.m_held;
        m_equivocating = src.m_equivocating;
        m_equivocationRevision = src.m_equivocationRevision;
        m_equivocationFirst = src.m_equivocationFirst;
        m_equivocationSecond = src.m_equivocationSecond;
        m_stored = src.m_stored;
        m_pendingProposals = src.m_pendingProposals;
        m_pendingVotes = src.m_pendingVotes;
        m_applied = src.m_applied;
        m_appliedWithEvidence = src.m_appliedWithEvidence;
        m_appliedSeenAt = src.m_appliedSeenAt;
        m_toName = src.m_toName;
        m_identity = src.m_identity;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDraftBoard::InitializeAsHost( const CPlayerIdentity& host, const CORE::CString& matchId )
{GUCEF_TRACE;

    if ( !CMatchId::IsValid( matchId ) || !host.HasKeyPair() )
        return false;
    *this = CMatchDraftBoard();
    m_matchId = matchId;
    m_hostUserId = host.userId;
    m_hostPublicKey = host.publicKey;
    m_ownUserId = host.userId;
    m_ownPublicKey = host.publicKey;
    m_isHost = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDraftBoard::InitializeAsParticipant( const CORE::CString& matchId, const CORE::CString& hostUserId, const CORE::CString& hostPublicKey, const CPlayerIdentity& own )
{GUCEF_TRACE;

    if ( !CMatchId::IsValid( matchId ) || !CMatchSigning::IsHexOfLength( hostPublicKey, CMatchSigning::PublicKeyHexSize ) ||
         hostUserId != CPlayerIdentity::MakeUserIdFromHex( hostPublicKey ) || own.userId.IsNULLOrEmpty() )
        return false;
    *this = CMatchDraftBoard();
    m_matchId = matchId;
    m_hostUserId = hostUserId;
    m_hostPublicKey = hostPublicKey;
    m_ownUserId = own.userId;
    m_ownPublicKey = own.publicKey;
    m_isHost = false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMatchDraftBoard::SetOwnContact( const CORE::CString& contact )
{GUCEF_TRACE;

    m_ownContact = contact;
}

bool CMatchDraftBoard::IsHost( void ) const { return m_isHost; }
const CORE::CString& CMatchDraftBoard::GetMatchId( void ) const { return m_matchId; }
void CMatchDraftBoard::SetApplyGraceMs( Int64 graceMs ) { m_applyGraceMs = graceMs; }
bool CMatchDraftBoard::HasHeldRoster( void ) const { return m_hasHeld; }
bool CMatchDraftBoard::HasUnauthorizedRemoval( void ) const { return m_unauthorizedRemoval; }
bool CMatchDraftBoard::HasRoster( void ) const { return !m_rosters.empty(); }
bool CMatchDraftBoard::IsHostEquivocating( void ) const { return m_equivocating; }
UInt32 CMatchDraftBoard::GetPendingCount( void ) const { return (UInt32) ( m_pendingProposals.size() + m_pendingVotes.size() ); }

/*-------------------------------------------------------------------------*/

UInt32
CMatchDraftBoard::GetRevision( void ) const
{GUCEF_TRACE;

    return m_rosters.empty() ? 0 : m_rosters.back().revision;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDraftBoard::GetRoster( CMatchRoster& roster ) const
{GUCEF_TRACE;

    if ( m_rosters.empty() )
        return false;
    roster = m_rosters.back();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDraftBoard::GetRosterOfRevision( UInt32 revision, CMatchRoster& roster ) const
{GUCEF_TRACE;

    const CMatchRoster* found = FindRoster( revision );
    if ( GUCEF_NULL == found )
        return false;
    roster = *found;
    return true;
}

/*-------------------------------------------------------------------------*/

const CMatchRoster*
CMatchDraftBoard::FindRoster( UInt32 revision ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_rosters.size(); ++i )
        if ( m_rosters[ i ].revision == revision )
            return &m_rosters[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDraftBoard::GetEquivocation( UInt32& revision, CORE::CString& firstDigest, CORE::CString& secondDigest ) const
{GUCEF_TRACE;

    if ( !m_equivocating )
        return false;
    revision = m_equivocationRevision;
    firstDigest = m_equivocationFirst;
    secondDigest = m_equivocationSecond;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDraftBoard::CheckKeyChanges( const CMatchRoster& roster, GUCEF::vector< CORE::CString >& contacts, GUCEF::vector< CORE::CString >& oldKeys, GUCEF::vector< CORE::CString >& newKeys ) const
{GUCEF_TRACE;

    contacts.clear();
    oldKeys.clear();
    newKeys.clear();
    for ( size_t e=0; e<roster.entries.size(); ++e )
    {
        const CMatchRosterEntry& entry = roster.entries[ e ];
        if ( !entry.HasKey() )
            continue;
        // the newest key a roster gave this contact
        for ( size_t r=m_rosters.size(); r>0; --r )
        {
            const CMatchRosterEntry* before = m_rosters[ r - 1 ].FindEntryByContact( entry.contact );
            if ( GUCEF_NULL != before && before->HasKey() )
            {
                if ( before->publicKey.Lowercase() != entry.publicKey.Lowercase() )
                {
                    contacts.push_back( entry.contact );
                    oldKeys.push_back( before->publicKey );
                    newKeys.push_back( entry.publicKey );
                }
                break;
            }
        }
    }
    return !contacts.empty();
}

/*-------------------------------------------------------------------------*/

void
CMatchDraftBoard::GetHeldKeyChanges( GUCEF::vector< CORE::CString >& contacts, GUCEF::vector< CORE::CString >& oldKeys, GUCEF::vector< CORE::CString >& newKeys ) const
{GUCEF_TRACE;

    contacts.clear();
    oldKeys.clear();
    newKeys.clear();
    if ( m_hasHeld )
        CheckKeyChanges( m_held, contacts, oldKeys, newKeys );
}

/*-------------------------------------------------------------------------*/

void
CMatchDraftBoard::RememberRoster( const CMatchRoster& roster )
{GUCEF_TRACE;

    m_rosters.push_back( roster );
    while ( m_rosters.size() > MaxRosterHistory )
        m_rosters.erase( m_rosters.begin() );
}

/*-------------------------------------------------------------------------*/

/**
 *  Whether every participant that the roster in force lists and the new roster does not (by contact) was taken off the table with the authority the
 *  policy of the game gives. Under BOSS the host is the boss and decides alone. Under VOTE nobody does: a player who declined may be tidied away (the
 *  player decided that), anybody else only by a KICK of that player, or (for a player who has not answered) a FORCE_START that names the player, which
 *  passed under the regular vote of the roster that proposal refers to. The decisions are those the roster brings as evidence and those the board holds.
 */
bool
CMatchDraftBoard::IsRemovalAuthorized( const CMatchRoster& roster, Int64 nowMs ) const
{GUCEF_TRACE;

    if ( m_rosters.empty() )
        return true;
    const CMatchRoster& previous = m_rosters.back();
    if ( MATCHDECISIONPOLICY_VOTE != previous.decisionPolicy )
        return true;
    for ( size_t e=0; e<previous.entries.size(); ++e )
    {
        const CMatchRosterEntry& entry = previous.entries[ e ];
        if ( entry.userId == previous.hostUserId || MATCHPARTICIPANT_DECLINED == entry.status || GUCEF_NULL != roster.FindEntryByContact( entry.contact ) )
            continue;

        // gone: is there a decision that passed about this entry?
        bool authorized = false;
        for ( size_t c=0; c<roster.evidence.size() + m_stored.size() && !authorized; ++c )
        {
            CMatchDecisionEvidence candidate;
            if ( c < roster.evidence.size() )
                candidate = roster.evidence[ c ];
            else
            {
                candidate.proposal = m_stored[ c - roster.evidence.size() ].proposal;
                candidate.votes = m_stored[ c - roster.evidence.size() ].votes;
            }
            const CMatchProposal& proposal = candidate.proposal;
            bool about = MATCHDECISION_KICK == proposal.kind && proposal.targetUserId == entry.userId;
            if ( MATCHDECISION_FORCE_START == proposal.kind && MATCHPARTICIPANT_INVITED == entry.status )
                for ( size_t t=0; t<proposal.targetUserIds.size() && !about; ++t )
                    about = proposal.targetUserIds[ t ] == entry.userId;
            if ( !about )
                continue;
            const CMatchRoster* referred = FindRoster( proposal.rosterRevision );
            if ( GUCEF_NULL == referred || referred->GetDigest() != proposal.rosterDigest )
                continue;
            // the electorate is the players who had accepted in that roster, exactly: a host cannot make the table smaller to win a vote
            UInt32 voters = 0;
            bool sameElectorate = true;
            for ( size_t v=0; v<referred->entries.size(); ++v )
                if ( MATCHPARTICIPANT_ACTIVE == referred->entries[ v ].status )
                {
                    ++voters;
                    sameElectorate = sameElectorate && proposal.IsInElectorate( referred->entries[ v ].userId );
                }
            if ( !sameElectorate || voters != proposal.voterUserIds.size() )
                continue;
            CMatchInfo info;
            referred->ToMatchInfo( info );
            CMatchDecisionResult result;
            authorized = CMatchDecisionVerifier::Evaluate( info, candidate, nowMs, result, false ) && result.valid && MATCHVOTEOUTCOME_PASSED == result.outcome;
        }
        if ( !authorized )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

TMatchDraftResult
CMatchDraftBoard::AcceptRoster( const CMatchRoster& roster, Int64 nowMs )
{GUCEF_TRACE;

    if ( m_isHost )
        return MATCHDRAFT_NOT_ALLOWED;
    if ( roster.matchId != m_matchId )
        return MATCHDRAFT_WRONG_MATCH;
    if ( roster.hostUserId != m_hostUserId || roster.hostPublicKey.Lowercase() != m_hostPublicKey.Lowercase() )
        return MATCHDRAFT_WRONG_HOST;
    CORE::CString reason;
    if ( !roster.IsWellFormed( reason ) )
        return MATCHDRAFT_MALFORMED;
    if ( !roster.VerifySignature() )
        return MATCHDRAFT_BAD_SIGNATURE;

    // a second roster of the host for a revision that is known: the first stays
    const CMatchRoster* known = FindRoster( roster.revision );
    if ( GUCEF_NULL == known && m_hasHeld && m_held.revision == roster.revision )
        known = &m_held;
    if ( GUCEF_NULL != known )
    {
        if ( known->GetDigest() == roster.GetDigest() )
            return MATCHDRAFT_DUPLICATE;
        if ( !m_equivocating )
        {
            m_equivocating = true;
            m_equivocationRevision = roster.revision;
            m_equivocationFirst = known->GetDigest();
            m_equivocationSecond = roster.GetDigest();
        }
        return MATCHDRAFT_EQUIVOCATION;
    }
    if ( !m_rosters.empty() && roster.revision < m_rosters.back().revision )
        return MATCHDRAFT_STALE;

    // a game that is decided by vote: the host does not take a player off the table alone
    if ( !IsRemovalAuthorized( roster, nowMs ) )
    {
        m_unauthorizedRemoval = true;
        return MATCHDRAFT_UNAUTHORIZED_REMOVAL;
    }

    // what the roster says about the player
    if ( !m_ownContact.IsNULLOrEmpty() )
    {
        const CMatchRosterEntry* mine = roster.FindEntryByContact( m_ownContact );
        if ( GUCEF_NULL != mine && mine->HasKey() && !m_ownPublicKey.IsNULLOrEmpty() && mine->publicKey.Lowercase() != m_ownPublicKey.Lowercase() )
            return MATCHDRAFT_OWN_KEY_MISMATCH;
    }

    // another key for somebody than an earlier roster gave: the player decides
    GUCEF::vector< CORE::CString > contacts, oldKeys, newKeys;
    if ( CheckKeyChanges( roster, contacts, oldKeys, newKeys ) )
    {
        m_held = roster;
        m_hasHeld = true;
        return MATCHDRAFT_KEY_CHANGED;
    }
    m_hasHeld = false;
    m_unauthorizedRemoval = false;
    RememberRoster( roster );

    // what the host says it carried out, and the evidence it counted
    for ( size_t a=0; a<roster.appliedDigests.size(); ++a )
    {
        bool have = false;
        for ( size_t k=0; k<m_applied.size(); ++k )
            have = have || m_applied[ k ] == roster.appliedDigests[ a ];
        if ( !have && m_applied.size() < 4 * CMatchRoster::MaxAppliedDigests )
        {
            m_applied.push_back( roster.appliedDigests[ a ] );
            m_appliedSeenAt.push_back( nowMs );
        }
    }
    for ( size_t e=0; e<roster.evidence.size(); ++e )
    {
        m_appliedWithEvidence.push_back( roster.evidence[ e ].proposal.GetDigest() );
        CORE::CString ignored;
        AddProposal( roster.evidence[ e ].proposal, nowMs, ignored );
        for ( size_t v=0; v<roster.evidence[ e ].votes.size(); ++v )
            AddVote( roster.evidence[ e ].votes[ v ], nowMs, ignored );
    }
    Retry( nowMs );
    Refresh( nowMs );
    return MATCHDRAFT_OK;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDraftBoard::ConfirmHeldRoster( bool accept, Int64 nowMs )
{GUCEF_TRACE;

    if ( !m_hasHeld )
        return false;
    const CMatchRoster held = m_held;
    m_hasHeld = false;
    m_held = CMatchRoster();
    if ( !accept )
        return true;
    RememberRoster( held );
    for ( size_t a=0; a<held.appliedDigests.size(); ++a )
    {
        bool have = false;
        for ( size_t k=0; k<m_applied.size(); ++k )
            have = have || m_applied[ k ] == held.appliedDigests[ a ];
        if ( !have && m_applied.size() < 4 * CMatchRoster::MaxAppliedDigests )
        {
            m_applied.push_back( held.appliedDigests[ a ] );
            m_appliedSeenAt.push_back( nowMs );
        }
    }
    for ( size_t e=0; e<held.evidence.size(); ++e )
    {
        m_appliedWithEvidence.push_back( held.evidence[ e ].proposal.GetDigest() );
        CORE::CString ignored;
        AddProposal( held.evidence[ e ].proposal, nowMs, ignored );
        for ( size_t v=0; v<held.evidence[ e ].votes.size(); ++v )
            AddVote( held.evidence[ e ].votes[ v ], nowMs, ignored );
    }
    Retry( nowMs );
    Refresh( nowMs );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDraftBoard::PublishRoster( const CPlayerIdentity& host, const TMatchRosterEntryVector& entries, TMatchDecisionPolicy policy, const CORE::CString& settingsDigest, Int64 nowMs, CMatchRoster& published )
{GUCEF_TRACE;

    if ( !m_isHost || host.userId != m_hostUserId || entries.empty() || entries.size() > CMatchRoster::MaxEntries )
        return false;
    CMatchRoster roster;
    roster.matchId = m_matchId;
    roster.revision = GetRevision() + 1;
    roster.hostUserId = host.userId;
    roster.hostPublicKey = host.publicKey;
    roster.decisionPolicy = policy;
    roster.settingsDigest = settingsDigest;
    roster.createdAt = nowMs;
    roster.entries = entries;
    roster.appliedDigests = m_applied;

    // the decisions carried out since the last roster are named, with the evidence of the first few
    GUCEF::vector< CORE::CString > named;
    for ( size_t n=0; n<m_toName.size(); ++n )
    {
        const SStored* stored = FindStored( m_toName[ n ] );
        if ( GUCEF_NULL == stored )
            continue;
        if ( roster.evidence.size() >= CMatchRoster::MaxEvidence )
            break;
        CMatchDecisionEvidence item;
        GetEvidence( m_toName[ n ], item );
        roster.evidence.push_back( item );
        roster.appliedDigests.push_back( stored->proposal.GetDigest() );
        named.push_back( m_toName[ n ] );
    }
    while ( roster.appliedDigests.size() > CMatchRoster::MaxAppliedDigests )
        roster.appliedDigests.erase( roster.appliedDigests.begin() );

    // nothing to say that the roster in force does not say already
    if ( !m_rosters.empty() && m_rosters.back().GetContentDigest() == roster.GetContentDigest() )
        return false;
    if ( !roster.Sign( host ) )
        return false;
    CORE::CString reason;
    if ( !roster.IsWellFormed( reason ) )
        return false;
    RememberRoster( roster );
    for ( size_t n=0; n<named.size(); ++n )
    {
        const SStored* stored = FindStored( named[ n ] );
        if ( GUCEF_NULL != stored )
        {
            m_applied.push_back( stored->proposal.GetDigest() );
            m_appliedSeenAt.push_back( nowMs );
            m_appliedWithEvidence.push_back( stored->proposal.GetDigest() );
        }
        for ( size_t t=0; t<m_toName.size(); ++t )
            if ( m_toName[ t ] == named[ n ] )
            {
                m_toName.erase( m_toName.begin() + t );
                break;
            }
    }
    published = roster;
    Retry( nowMs );
    Refresh( nowMs );
    return true;
}

/*-------------------------------------------------------------------------*/

CMatchDraftBoard::SStored*
CMatchDraftBoard::FindStored( const CORE::CString& proposalId )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_stored.size(); ++i )
        if ( m_stored[ i ].proposal.proposalId == proposalId )
            return &m_stored[ i ];
    return GUCEF_NULL;
}

const CMatchDraftBoard::SStored*
CMatchDraftBoard::FindStored( const CORE::CString& proposalId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_stored.size(); ++i )
        if ( m_stored[ i ].proposal.proposalId == proposalId )
            return &m_stored[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDraftBoard::MakeProposal( const CPlayerIdentity& own, TMatchDecisionKind kind, const CORE::CString& targetUserId, Int64 nowMs, Int64 durationMs, CMatchProposal& proposal, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( m_rosters.empty() )
    {
        reason = "The table of the game is not known yet";
        return false;
    }
    const CMatchRoster& roster = m_rosters.back();
    const CMatchRosterEntry* me = roster.FindEntry( own.userId );
    if ( GUCEF_NULL == me || MATCHPARTICIPANT_ACTIVE != me->status )
    {
        reason = "Only the players who accepted take part in a decision";
        return false;
    }
    proposal = CMatchProposal();
    proposal.matchId = m_matchId;
    proposal.kind = kind;
    proposal.createdAt = nowMs;
    proposal.deadline = durationMs > 0 ? nowMs + durationMs : 0;
    proposal.rosterRevision = roster.revision;
    proposal.rosterDigest = roster.GetDigest();
    CMatchInfo info;
    roster.ToMatchInfo( info );
    const bool isHost = own.userId == roster.hostUserId;

    switch ( kind )
    {
        case MATCHDECISION_KICK:
        {
            const CMatchRosterEntry* target = roster.FindEntry( targetUserId );
            if ( GUCEF_NULL == target || targetUserId == roster.hostUserId || targetUserId == own.userId || MATCHPARTICIPANT_DECLINED == target->status )
            {
                reason = "Pick another player of the game, not the host and not yourself";
                return false;
            }
            proposal.targetUserId = targetUserId;
            break;
        }
        case MATCHDECISION_FORCE_START:
            for ( size_t i=0; i<roster.entries.size(); ++i )
                if ( MATCHPARTICIPANT_INVITED == roster.entries[ i ].status )
                    proposal.targetUserIds.push_back( roster.entries[ i ].userId );
            if ( proposal.targetUserIds.empty() )
            {
                reason = "Everybody has answered: there is nobody the computer could replace";
                return false;
            }
            break;
        case MATCHDECISION_MOVE_TO_SPECTATOR:
            if ( isHost || targetUserId != own.userId )
            {
                reason = "A player only chooses to watch for themselves, and the host plays";
                return false;
            }
            proposal.targetUserId = own.userId;
            proposal.key = GUCEF_MULTIPLAY_SPECTATOR_REASON_VOLUNTARY;
            break;
        default:
            reason = "This is not decided before the game is made";
            return false;
    }
    if ( MATCHDECISION_MOVE_TO_SPECTATOR != kind )
    {
        if ( MATCHDECISIONPOLICY_BOSS == roster.decisionPolicy && !me->boss )
        {
            reason = "In this game the host decides. Ask the host";
            return false;
        }
        proposal.SetElectorateFrom( info );
    }
    if ( !proposal.Sign( own ) )
    {
        reason = "The proposal could not be signed";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

TMatchDraftResult
CMatchDraftBoard::CheckProposal( const CMatchProposal& proposal, Int64 nowMs, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( proposal.matchId != m_matchId )
        return MATCHDRAFT_WRONG_MATCH;
    if ( MATCHDECISION_KICK != proposal.kind && MATCHDECISION_FORCE_START != proposal.kind && MATCHDECISION_MOVE_TO_SPECTATOR != proposal.kind )
    {
        reason = "This is not decided before the game is made";
        return MATCHDRAFT_NOT_ALLOWED;
    }
    if ( 0 == proposal.rosterRevision )
    {
        reason = "A decision of the draft stage names the roster it refers to";
        return MATCHDRAFT_NOT_ALLOWED;
    }
    const CMatchRoster* roster = FindRoster( proposal.rosterRevision );
    if ( GUCEF_NULL == roster )
        return MATCHDRAFT_PENDING;
    if ( roster->GetDigest() != proposal.rosterDigest )
    {
        reason = "The proposal names another roster than the one of that revision";
        return MATCHDRAFT_ROSTER_MISMATCH;
    }
    CMatchInfo info;
    roster->ToMatchInfo( info );
    const bool own = MATCHDECISION_MOVE_TO_SPECTATOR == proposal.kind;
    if ( own )
    {
        if ( proposal.proposerUserId != proposal.targetUserId || !proposal.voterUserIds.empty() )
        {
            reason = "A player only chooses to watch for themselves";
            return MATCHDRAFT_NOT_ALLOWED;
        }
    }
    else
    {
        // the electorate is the participants who are introduced and do not watch, in that revision, exactly
        GUCEF::vector< CORE::CString > voters;
        for ( size_t i=0; i<roster->entries.size(); ++i )
            if ( MATCHPARTICIPANT_ACTIVE == roster->entries[ i ].status )
                voters.push_back( roster->entries[ i ].userId );
        bool same = voters.size() == proposal.voterUserIds.size();
        for ( size_t i=0; same && i<voters.size(); ++i )
            same = proposal.IsInElectorate( voters[ i ] );
        if ( !same )
        {
            reason = "The electorate of the proposal is not the one of the roster it refers to";
            return MATCHDRAFT_NOT_ALLOWED;
        }
    }
    CMatchDecisionEvidence evidence;
    evidence.proposal = proposal;
    CMatchDecisionResult result;
    if ( !CMatchDecisionVerifier::Evaluate( info, evidence, nowMs, result, false ) )
    {
        reason = result.reason;
        return result.reason.HasSubstr( "not signed", true ) >= 0 ? MATCHDRAFT_BAD_SIGNATURE : MATCHDRAFT_NOT_ALLOWED;
    }
    return MATCHDRAFT_OK;
}

/*-------------------------------------------------------------------------*/

TMatchDraftResult
CMatchDraftBoard::AddProposal( const CMatchProposal& proposal, Int64 nowMs, CORE::CString& reason )
{GUCEF_TRACE;

    const SStored* known = FindStored( proposal.proposalId );
    if ( GUCEF_NULL != known )
        return known->proposal.GetDigest() == proposal.GetDigest() ? MATCHDRAFT_DUPLICATE : MATCHDRAFT_CONFLICT;
    const TMatchDraftResult checked = CheckProposal( proposal, nowMs, reason );
    if ( MATCHDRAFT_PENDING == checked )
    {
        for ( size_t i=0; i<m_pendingProposals.size(); ++i )
            if ( m_pendingProposals[ i ].proposalId == proposal.proposalId )
                return MATCHDRAFT_DUPLICATE;
        if ( m_pendingProposals.size() >= MaxPendingProposals )
            return MATCHDRAFT_LIMIT;
        m_pendingProposals.push_back( proposal );
        return MATCHDRAFT_PENDING;
    }
    if ( MATCHDRAFT_OK != checked )
        return checked;
    UInt32 fromProposer = 0;
    for ( size_t i=0; i<m_stored.size(); ++i )
        if ( m_stored[ i ].proposal.proposerUserId == proposal.proposerUserId )
            ++fromProposer;
    if ( m_stored.size() >= MaxProposals || fromProposer >= MaxProposalsPerProposer )
        return MATCHDRAFT_LIMIT;
    SStored stored;
    stored.proposal = proposal;
    m_stored.push_back( stored );
    Retry( nowMs );
    Refresh( nowMs );
    return MATCHDRAFT_OK;
}

/*-------------------------------------------------------------------------*/

TMatchDraftResult
CMatchDraftBoard::AddVote( const CMatchVote& vote, Int64 nowMs, CORE::CString& reason )
{GUCEF_TRACE;

    if ( vote.matchId != m_matchId )
        return MATCHDRAFT_WRONG_MATCH;
    SStored* stored = FindStored( vote.proposalId );
    if ( GUCEF_NULL == stored )
    {
        for ( size_t i=0; i<m_pendingVotes.size(); ++i )
            if ( m_pendingVotes[ i ].proposalId == vote.proposalId && m_pendingVotes[ i ].voterUserId == vote.voterUserId && m_pendingVotes[ i ].signature == vote.signature )
                return MATCHDRAFT_DUPLICATE;
        if ( m_pendingVotes.size() >= MaxPendingVotes )
            return MATCHDRAFT_LIMIT;
        m_pendingVotes.push_back( vote );
        return MATCHDRAFT_PENDING;
    }
    if ( vote.proposalDigest != stored->proposal.GetDigest() )
        return MATCHDRAFT_NO_PROPOSAL;
    const CMatchRoster* roster = FindRoster( stored->proposal.rosterRevision );
    if ( GUCEF_NULL == roster )
        return MATCHDRAFT_PENDING;
    const CMatchRosterEntry* voter = roster->FindEntry( vote.voterUserId );
    if ( GUCEF_NULL == voter || MATCHPARTICIPANT_ACTIVE != voter->status || ( !stored->proposal.voterUserIds.empty() && !stored->proposal.IsInElectorate( vote.voterUserId ) ) )
        return MATCHDRAFT_NOT_MEMBER;
    CMatchInfo info;
    roster->ToMatchInfo( info );
    if ( !vote.VerifySignature( info ) )
        return MATCHDRAFT_BAD_SIGNATURE;
    for ( size_t i=0; i<stored->votes.size(); ++i )
        if ( stored->votes[ i ].voterUserId == vote.voterUserId )
            return stored->votes[ i ].yes == vote.yes ? MATCHDRAFT_DUPLICATE : MATCHDRAFT_CONFLICT;
    if ( stored->votes.size() >= MaxVotesPerProposal )
        return MATCHDRAFT_LIMIT;

    // the whole of the evidence has to stay good: a vote that would spoil it is not kept
    CMatchDecisionEvidence evidence;
    evidence.proposal = stored->proposal;
    evidence.votes = stored->votes;
    evidence.votes.push_back( vote );
    CMatchDecisionResult result;
    if ( !CMatchDecisionVerifier::Evaluate( info, evidence, nowMs, result, false ) )
    {
        reason = result.reason;
        return MATCHDRAFT_NOT_ALLOWED;
    }
    stored->votes.push_back( vote );
    Refresh( nowMs );
    return MATCHDRAFT_OK;
}

/*-------------------------------------------------------------------------*/

void
CMatchDraftBoard::Retry( Int64 nowMs )
{GUCEF_TRACE;

    // what waited for a roster or a proposal that is here now: tried again, once each
    GUCEF::vector< CMatchProposal > proposals;
    proposals.swap( m_pendingProposals );
    for ( size_t i=0; i<proposals.size(); ++i )
    {
        CORE::CString ignored;
        if ( MATCHDRAFT_PENDING == AddProposal( proposals[ i ], nowMs, ignored ) )
            continue;
    }
    GUCEF::vector< CMatchVote > votes;
    votes.swap( m_pendingVotes );
    for ( size_t i=0; i<votes.size(); ++i )
    {
        CORE::CString ignored;
        AddVote( votes[ i ], nowMs, ignored );
    }
}

/*-------------------------------------------------------------------------*/

bool
CMatchDraftBoard::Evaluate( const SStored& stored, Int64 nowMs, CMatchDecisionResult& result ) const
{GUCEF_TRACE;

    const CMatchRoster* roster = FindRoster( stored.proposal.rosterRevision );
    if ( GUCEF_NULL == roster )
        return false;
    CMatchInfo info;
    roster->ToMatchInfo( info );
    CMatchDecisionEvidence evidence;
    evidence.proposal = stored.proposal;
    evidence.votes = stored.votes;
    return CMatchDecisionVerifier::Evaluate( info, evidence, nowMs, result, false );
}

/*-------------------------------------------------------------------------*/

void
CMatchDraftBoard::Refresh( Int64 nowMs )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_stored.size(); ++i )
    {
        if ( 0 != m_stored[ i ].passedSeenAt )
            continue;
        CMatchDecisionResult result;
        if ( Evaluate( m_stored[ i ], nowMs, result ) && result.valid && MATCHVOTEOUTCOME_PASSED == result.outcome )
            m_stored[ i ].passedSeenAt = nowMs;
    }
}

/*-------------------------------------------------------------------------*/

bool
CMatchDraftBoard::StillApplies( const CMatchProposal& proposal ) const
{GUCEF_TRACE;

    if ( m_rosters.empty() )
        return false;
    const CMatchRoster& roster = m_rosters.back();
    switch ( proposal.kind )
    {
        case MATCHDECISION_KICK:
            return GUCEF_NULL != roster.FindEntry( proposal.targetUserId );
        case MATCHDECISION_MOVE_TO_SPECTATOR:
        {
            const CMatchRosterEntry* target = roster.FindEntry( proposal.targetUserId );
            return GUCEF_NULL != target && MATCHPARTICIPANT_ACTIVE == target->status;
        }
        case MATCHDECISION_FORCE_START:
            for ( size_t i=0; i<proposal.targetUserIds.size(); ++i )
            {
                const CMatchRosterEntry* target = roster.FindEntry( proposal.targetUserIds[ i ] );
                if ( GUCEF_NULL != target && MATCHPARTICIPANT_INVITED == target->status )
                    return true;
            }
            return false;
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CMatchDraftBoard::IsApplied( const CORE::CString& proposalDigest ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_applied.size(); ++i )
        if ( m_applied[ i ] == proposalDigest )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CMatchDraftBoard::MakeTally( const SStored& stored, Int64 nowMs, CMatchDraftTally& tally ) const
{GUCEF_TRACE;

    const CMatchProposal& proposal = stored.proposal;
    tally = CMatchDraftTally();
    tally.proposalId = proposal.proposalId;
    tally.digest = proposal.GetDigest();
    tally.kind = proposal.kind;
    tally.proposerUserId = proposal.proposerUserId;
    tally.targetUserId = proposal.targetUserId;
    tally.targetUserIds = proposal.targetUserIds;
    tally.rosterRevision = proposal.rosterRevision;
    tally.createdAt = proposal.createdAt;
    tally.deadline = proposal.deadline;
    tally.passedSeenAt = stored.passedSeenAt;

    CMatchDecisionResult result;
    if ( Evaluate( stored, nowMs, result ) && result.valid )
    {
        tally.yes = result.yes;
        tally.no = result.no;
        tally.needed = result.needed;
        tally.eligible = result.eligible;
        tally.outcome = result.outcome;
    }
    tally.yesVoters.push_back( proposal.proposerUserId );
    for ( size_t v=0; v<stored.votes.size(); ++v )
    {
        if ( stored.votes[ v ].voterUserId == proposal.proposerUserId )
            continue;
        ( stored.votes[ v ].yes ? tally.yesVoters : tally.noVoters ).push_back( stored.votes[ v ].voterUserId );
    }
    for ( size_t e=0; e<proposal.voterUserIds.size(); ++e )
    {
        const CORE::CString& id = proposal.voterUserIds[ e ];
        bool voted = id == proposal.proposerUserId;
        for ( size_t v=0; v<stored.votes.size() && !voted; ++v )
            voted = stored.votes[ v ].voterUserId == id;
        if ( !voted )
            tally.notVoted.push_back( id );
    }

    const bool named = IsApplied( tally.digest );
    tally.appliedByHost = named;
    if ( named )
    {
        if ( MATCHVOTEOUTCOME_PASSED == tally.outcome || ( m_isHost && stored.appliedByUs ) )
            tally.state = MATCHDRAFTSTATE_APPLIED;
        else
        {
            bool withEvidence = false;
            Int64 seen = nowMs;
            for ( size_t i=0; i<m_appliedWithEvidence.size(); ++i )
                withEvidence = withEvidence || m_appliedWithEvidence[ i ] == tally.digest;
            for ( size_t i=0; i<m_applied.size() && i<m_appliedSeenAt.size(); ++i )
                if ( m_applied[ i ] == tally.digest )
                    seen = m_appliedSeenAt[ i ];
            tally.state = ( withEvidence || nowMs >= seen + m_applyGraceMs ) ? MATCHDRAFTSTATE_HOST_MISCOUNT : MATCHDRAFTSTATE_OPEN;
        }
        return;
    }
    if ( MATCHVOTEOUTCOME_FAILED == tally.outcome )
        tally.state = MATCHDRAFTSTATE_FAILED;
    else
    if ( !StillApplies( proposal ) )
        tally.state = MATCHDRAFTSTATE_OBSOLETE;
    else
    if ( MATCHVOTEOUTCOME_PASSED == tally.outcome )
        tally.state = ( 0 != stored.passedSeenAt && nowMs >= stored.passedSeenAt + m_applyGraceMs && !( m_isHost && stored.appliedByUs ) ) ? MATCHDRAFTSTATE_HOST_IGNORED : MATCHDRAFTSTATE_PASSED;
    else
        tally.state = MATCHDRAFTSTATE_OPEN;
    if ( tally.passedSeenAt == 0 && MATCHVOTEOUTCOME_PASSED == tally.outcome )
        tally.passedSeenAt = nowMs;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDraftBoard::GetTally( const CORE::CString& proposalId, Int64 nowMs, CMatchDraftTally& tally ) const
{GUCEF_TRACE;

    const SStored* stored = FindStored( proposalId );
    if ( GUCEF_NULL == stored )
        return false;
    MakeTally( *stored, nowMs, tally );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchDraftBoard::GetTallies( Int64 nowMs, GUCEF::vector< CMatchDraftTally >& tallies ) const
{GUCEF_TRACE;

    tallies.clear();
    for ( size_t i=0; i<m_stored.size(); ++i )
    {
        CMatchDraftTally tally;
        MakeTally( m_stored[ i ], nowMs, tally );
        tallies.push_back( tally );
    }
    return (UInt32) tallies.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchDraftBoard::GetPassedForApplication( Int64 nowMs, GUCEF::vector< CMatchDraftTally >& passed ) const
{GUCEF_TRACE;

    passed.clear();
    if ( !m_isHost )
        return 0;
    for ( size_t i=0; i<m_stored.size(); ++i )
    {
        if ( m_stored[ i ].appliedByUs )
            continue;
        CMatchDraftTally tally;
        MakeTally( m_stored[ i ], nowMs, tally );
        if ( MATCHVOTEOUTCOME_PASSED == tally.outcome && !tally.appliedByHost && MATCHDRAFTSTATE_OBSOLETE != tally.state )
            passed.push_back( tally );
    }
    return (UInt32) passed.size();
}

/*-------------------------------------------------------------------------*/

bool
CMatchDraftBoard::MarkApplied( const CORE::CString& proposalId )
{GUCEF_TRACE;

    SStored* stored = FindStored( proposalId );
    if ( !m_isHost || GUCEF_NULL == stored )
        return false;
    if ( !stored->appliedByUs )
    {
        stored->appliedByUs = true;
        m_toName.push_back( proposalId );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchDraftBoard::GetProposals( GUCEF::vector< CMatchProposal >& proposals ) const
{GUCEF_TRACE;

    proposals.clear();
    for ( size_t i=0; i<m_stored.size(); ++i )
        proposals.push_back( m_stored[ i ].proposal );
    return (UInt32) proposals.size();
}

bool
CMatchDraftBoard::GetProposal( const CORE::CString& proposalId, CMatchProposal& proposal ) const
{GUCEF_TRACE;

    const SStored* stored = FindStored( proposalId );
    if ( GUCEF_NULL == stored )
        return false;
    proposal = stored->proposal;
    return true;
}

bool
CMatchDraftBoard::GetVote( const CORE::CString& proposalId, const CORE::CString& voterUserId, CMatchVote& vote ) const
{GUCEF_TRACE;

    const SStored* stored = FindStored( proposalId );
    if ( GUCEF_NULL == stored )
        return false;
    for ( size_t i=0; i<stored->votes.size(); ++i )
        if ( stored->votes[ i ].voterUserId == voterUserId )
        {
            vote = stored->votes[ i ];
            return true;
        }
    return false;
}

bool
CMatchDraftBoard::GetEvidence( const CORE::CString& proposalId, CMatchDecisionEvidence& evidence ) const
{GUCEF_TRACE;

    const SStored* stored = FindStored( proposalId );
    if ( GUCEF_NULL == stored )
        return false;
    evidence = CMatchDecisionEvidence();
    evidence.proposal = stored->proposal;
    evidence.votes = stored->votes;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchDraftBoard::GetAppliedSpectators( GUCEF::vector< CORE::CString >& userIds ) const
{GUCEF_TRACE;

    userIds.clear();
    for ( size_t i=0; i<m_stored.size(); ++i )
        if ( MATCHDECISION_MOVE_TO_SPECTATOR == m_stored[ i ].proposal.kind && m_stored[ i ].appliedByUs )
            userIds.push_back( m_stored[ i ].proposal.targetUserId );
    return (UInt32) userIds.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchDraftBoard::GetSpectatorStatements( GUCEF::vector< CMatchProposal >& statements ) const
{GUCEF_TRACE;

    statements.clear();
    if ( m_rosters.empty() )
        return 0;
    for ( size_t i=0; i<m_stored.size(); ++i )
    {
        const CMatchRosterEntry* entry = m_rosters.back().FindEntry( m_stored[ i ].proposal.targetUserId );
        if ( MATCHDECISION_MOVE_TO_SPECTATOR == m_stored[ i ].proposal.kind && GUCEF_NULL != entry && MATCHPARTICIPANT_SPECTATOR == entry->status )
            statements.push_back( m_stored[ i ].proposal );
    }
    return (UInt32) statements.size();
}

/*-------------------------------------------------------------------------*/

bool
CMatchDraftBoard::Serialize( CORE::CDataNode& domRootNode                        ,
                             const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "matchId", m_matchId );
    domRootNode.SetAttribute( "hostUserId", m_hostUserId );
    domRootNode.SetAttribute( "hostPublicKey", m_hostPublicKey );
    domRootNode.SetAttribute( "ownUserId", m_ownUserId );
    domRootNode.SetAttribute( "ownPublicKey", m_ownPublicKey );
    domRootNode.SetAttribute( "ownContact", m_ownContact );
    domRootNode.SetAttribute( "isHost", m_isHost );
    domRootNode.SetAttribute( "applyGraceMs", m_applyGraceMs );
    domRootNode.SetAttribute( "equivocating", m_equivocating );
    domRootNode.SetAttribute( "equivocationRevision", m_equivocationRevision );
    domRootNode.SetAttribute( "equivocationFirst", m_equivocationFirst );
    domRootNode.SetAttribute( "equivocationSecond", m_equivocationSecond );
    CORE::CDataNode* list = domRootNode.AddChild( "rosters", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<m_rosters.size(); ++i )
        m_rosters[ i ].Serialize( *list->AddChild( "roster", GUCEF_DATATYPE_OBJECT ), settings );
    if ( m_hasHeld )
        m_held.Serialize( *domRootNode.AddChild( "held", GUCEF_DATATYPE_OBJECT ), settings );
    list = domRootNode.AddChild( "stored", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<m_stored.size(); ++i )
    {
        CORE::CDataNode* item = list->AddChild( "item", GUCEF_DATATYPE_OBJECT );
        item->SetAttribute( "passedSeenAt", m_stored[ i ].passedSeenAt );
        item->SetAttribute( "appliedByUs", m_stored[ i ].appliedByUs );
        m_stored[ i ].proposal.Serialize( *item->AddChild( "proposal", GUCEF_DATATYPE_OBJECT ), settings );
        CORE::CDataNode* votes = item->AddChild( "votes", GUCEF_DATATYPE_ARRAY );
        for ( size_t v=0; v<m_stored[ i ].votes.size(); ++v )
            m_stored[ i ].votes[ v ].Serialize( *votes->AddChild( "vote", GUCEF_DATATYPE_OBJECT ), settings );
    }
    list = domRootNode.AddChild( "pendingProposals", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<m_pendingProposals.size(); ++i )
        m_pendingProposals[ i ].Serialize( *list->AddChild( "proposal", GUCEF_DATATYPE_OBJECT ), settings );
    list = domRootNode.AddChild( "pendingVotes", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<m_pendingVotes.size(); ++i )
        m_pendingVotes[ i ].Serialize( *list->AddChild( "vote", GUCEF_DATATYPE_OBJECT ), settings );
    list = domRootNode.AddChild( "applied", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<m_applied.size(); ++i )
    {
        CORE::CDataNode* item = list->AddChild( "item", GUCEF_DATATYPE_OBJECT );
        item->SetAttribute( "digest", m_applied[ i ] );
        item->SetAttribute( "seenAt", i < m_appliedSeenAt.size() ? m_appliedSeenAt[ i ] : (Int64) 0 );
    }
    list = domRootNode.AddChild( "appliedWithEvidence", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<m_appliedWithEvidence.size(); ++i )
        list->AddChild( "item", GUCEF_DATATYPE_OBJECT )->SetAttribute( "digest", m_appliedWithEvidence[ i ] );
    list = domRootNode.AddChild( "toName", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<m_toName.size(); ++i )
        list->AddChild( "item", GUCEF_DATATYPE_OBJECT )->SetAttribute( "proposalId", m_toName[ i ] );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDraftBoard::Deserialize( const CORE::CDataNode& domRootNode                  ,
                               const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    *this = CMatchDraftBoard();
    m_matchId = domRootNode.GetAttributeValueOrChildValueByName( "matchId" ).AsString( CORE::CString::Empty, true );
    m_hostUserId = domRootNode.GetAttributeValueOrChildValueByName( "hostUserId" ).AsString( CORE::CString::Empty, true );
    m_hostPublicKey = domRootNode.GetAttributeValueOrChildValueByName( "hostPublicKey" ).AsString( CORE::CString::Empty, true );
    m_ownUserId = domRootNode.GetAttributeValueOrChildValueByName( "ownUserId" ).AsString( CORE::CString::Empty, true );
    m_ownPublicKey = domRootNode.GetAttributeValueOrChildValueByName( "ownPublicKey" ).AsString( CORE::CString::Empty, true );
    m_ownContact = domRootNode.GetAttributeValueOrChildValueByName( "ownContact" ).AsString( CORE::CString::Empty, true );
    m_isHost = domRootNode.GetAttributeValueOrChildValueByName( "isHost" ).AsBool( false, true );
    m_applyGraceMs = domRootNode.GetAttributeValueOrChildValueByName( "applyGraceMs" ).AsInt64( DefaultApplyGraceMs, true );
    m_equivocating = domRootNode.GetAttributeValueOrChildValueByName( "equivocating" ).AsBool( false, true );
    m_equivocationRevision = domRootNode.GetAttributeValueOrChildValueByName( "equivocationRevision" ).AsUInt32( 0, true );
    m_equivocationFirst = domRootNode.GetAttributeValueOrChildValueByName( "equivocationFirst" ).AsString( CORE::CString::Empty, true );
    m_equivocationSecond = domRootNode.GetAttributeValueOrChildValueByName( "equivocationSecond" ).AsString( CORE::CString::Empty, true );
    if ( !CMatchId::IsValid( m_matchId ) )
        return false;

    // stored data is read within the same bounds as received data, and what does not verify is not taken
    const CORE::CDataNode* list = domRootNode.FindChild( "rosters" );
    if ( GUCEF_NULL != list )
        for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd() && m_rosters.size() < MaxRosterHistory; ++i )
        {
            CMatchRoster roster;
            CORE::CString reason;
            if ( roster.Deserialize( *(*i), settings ) && roster.matchId == m_matchId && roster.IsWellFormed( reason ) && roster.VerifySignature() )
                m_rosters.push_back( roster );
        }
    const CORE::CDataNode* held = domRootNode.FindChild( "held" );
    if ( GUCEF_NULL != held && m_held.Deserialize( *held, settings ) )
        m_hasHeld = true;
    list = domRootNode.FindChild( "stored" );
    if ( GUCEF_NULL != list )
        for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd() && m_stored.size() < MaxProposals; ++i )
        {
            SStored stored;
            stored.passedSeenAt = (*i)->GetAttributeValueOrChildValueByName( "passedSeenAt" ).AsInt64( 0, true );
            stored.appliedByUs = (*i)->GetAttributeValueOrChildValueByName( "appliedByUs" ).AsBool( false, true );
            const CORE::CDataNode* proposal = (*i)->FindChild( "proposal" );
            if ( GUCEF_NULL == proposal || !stored.proposal.Deserialize( *proposal, settings ) )
                continue;
            const CORE::CDataNode* votes = (*i)->FindChild( "votes" );
            if ( GUCEF_NULL != votes )
                for ( CORE::CDataNode::const_iterator v = votes->ConstBegin(); v != votes->ConstEnd() && stored.votes.size() < MaxVotesPerProposal; ++v )
                {
                    CMatchVote vote;
                    if ( vote.Deserialize( *(*v), settings ) )
                        stored.votes.push_back( vote );
                }
            m_stored.push_back( stored );
        }
    list = domRootNode.FindChild( "pendingProposals" );
    if ( GUCEF_NULL != list )
        for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd() && m_pendingProposals.size() < MaxPendingProposals; ++i )
        {
            CMatchProposal proposal;
            if ( proposal.Deserialize( *(*i), settings ) )
                m_pendingProposals.push_back( proposal );
        }
    list = domRootNode.FindChild( "pendingVotes" );
    if ( GUCEF_NULL != list )
        for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd() && m_pendingVotes.size() < MaxPendingVotes; ++i )
        {
            CMatchVote vote;
            if ( vote.Deserialize( *(*i), settings ) )
                m_pendingVotes.push_back( vote );
        }
    list = domRootNode.FindChild( "applied" );
    if ( GUCEF_NULL != list )
        for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd() && m_applied.size() < 4 * CMatchRoster::MaxAppliedDigests; ++i )
        {
            m_applied.push_back( (*i)->GetAttributeValueOrChildValueByName( "digest" ).AsString( CORE::CString::Empty, true ) );
            m_appliedSeenAt.push_back( (*i)->GetAttributeValueOrChildValueByName( "seenAt" ).AsInt64( 0, true ) );
        }
    list = domRootNode.FindChild( "appliedWithEvidence" );
    if ( GUCEF_NULL != list )
        for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd() && m_appliedWithEvidence.size() < 4 * CMatchRoster::MaxAppliedDigests; ++i )
            m_appliedWithEvidence.push_back( (*i)->GetAttributeValueOrChildValueByName( "digest" ).AsString( CORE::CString::Empty, true ) );
    list = domRootNode.FindChild( "toName" );
    if ( GUCEF_NULL != list )
        for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd() && m_toName.size() < MaxProposals; ++i )
            m_toName.push_back( (*i)->GetAttributeValueOrChildValueByName( "proposalId" ).AsString( CORE::CString::Empty, true ) );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchDraftBoard::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchDraftBoard( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchDraftBoard::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CMatchDraftBoard";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CMatchDraftBoard::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchDraftBoard::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchDraftBoard";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
