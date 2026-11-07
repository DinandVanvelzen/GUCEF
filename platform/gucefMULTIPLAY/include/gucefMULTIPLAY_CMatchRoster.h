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

#ifndef GUCEF_MULTIPLAY_CMATCHROSTER_H
#define GUCEF_MULTIPLAY_CMATCHROSTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CMATCHDECISIONEVIDENCE_H
#include "gucefMULTIPLAY_CMatchDecisionEvidence.h"
#define GUCEF_MULTIPLAY_CMATCHDECISIONEVIDENCE_H
#endif /* GUCEF_MULTIPLAY_CMATCHDECISIONEVIDENCE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One participant in the signed table of a game that has no match record yet (CMatchRoster).
 *
 *  A participant that is introduced has a key and the user id of that key. A participant that was asked and has not answered has no key: it is
 *  listed under a placeholder user id (MakePendingUserId()), which is what a decision about it names (a force start replaces such a participant).
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchRosterEntry
{
    public:

    CORE::CString userId;                   /**< the user id of the key, or the placeholder of a participant that has no key */
    CORE::CString displayName;
    CORE::CString contact;                  /**< opaque to this layer: the mail address for the mail provider */
    CORE::CString publicKey;                /**< Ed25519 public key in hex; empty for a participant that has not introduced itself */
    TMatchParticipantStatus status;         /**< ACTIVE: introduced (has accepted); INVITED: asked, not answered; DECLINED; SPECTATOR: watches. Nobody else is listed */
    bool boss;
    CORE::CString statusCode;               /**< why a spectator watches (see GUCEF_MULTIPLAY_SPECTATOR_REASON_VOLUNTARY) */

    CMatchRosterEntry( void );

    bool operator==( const CMatchRosterEntry& other ) const;

    /** Whether the participant has a key and with it can sign and vote */
    bool HasKey( void ) const;

    /** The user id a participant without a key is listed under: "pending-" and 24 hex digits of the SHA-256 of the match id and the lower case contact */
    static CORE::CString MakePendingUserId( const CORE::CString& matchId, const CORE::CString& contact );

    static bool IsPendingUserId( const CORE::CString& userId );

    void WriteSigningBytes( CMatchBinaryWriter& writer ) const;

    void Serialize( CORE::CDataNode& node ) const;

    bool Deserialize( const CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CMatchRosterEntry > TMatchRosterEntryVector;

/*-------------------------------------------------------------------------*/

/**
 *  The signed table of a game that is being set up and has no match record yet (the draft stage).
 *
 *  Before the record exists the participants do not know each other's keys: the host learns the key of a participant when that participant
 *  introduces itself. The host publishes this roster whenever the draft changes, to every participant: who is in, with which key and contact,
 *  in which status, the digest of the settings, a revision that counts up by one per change, which decisions the host has carried out, and the
 *  evidence of the decisions carried out in this revision. A participant learns the keys of the others only from a roster whose signature is the
 *  host's, whose key the participant knows from the introduction. The decisions of the draft stage (CMatchDraftBoard) refer to a revision of the
 *  roster and are checked against the keys and statuses that revision holds, so every participant can count them.
 *
 *  The roster is the host's statement: the host may change the draft at will (as the host of any lobby may). What a participant checks is that
 *  the roster is the host's, that it is not older than the one it holds, and what the roster says about the participant's own key.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchRoster : public CORE::CIDataNodeSerializable
{
    public:

    static const UInt32 MaxEntries = CMatchInfo::MaxParticipants;
    static const UInt32 MaxEvidence = 4;                    /**< decisions whose evidence one roster carries */
    static const UInt32 MaxAppliedDigests = 32;             /**< the decisions a roster names as carried out, the most recent ones */
    static const UInt32 MaxNameBytes = CMatchParticipant::MaxDisplayNameLength;

    CORE::CString matchId;
    UInt32 revision;                                        /**< 1 for the first roster of a draft, then one more per change */
    CORE::CString hostUserId;
    CORE::CString hostPublicKey;
    TMatchDecisionPolicy decisionPolicy;
    CORE::CString settingsDigest;                           /**< SHA-256 in hex over the settings of the draft (MakeSettingsDigest()); empty for none */
    Int64 createdAt;                                        /**< ms since the Unix epoch, the host's clock */
    TMatchRosterEntryVector entries;                        /**< the host first */
    GUCEF::vector< CORE::CString > appliedDigests;          /**< the digests of the proposals the host has carried out, oldest first */
    GUCEF::vector< CMatchDecisionEvidence > evidence;       /**< the evidence of the decisions carried out in this revision */
    CORE::CString signature;                                /**< the host's, hex */

    CMatchRoster( void );

    CMatchRoster( const CMatchRoster& src );

    virtual ~CMatchRoster();

    CMatchRoster& operator=( const CMatchRoster& src );

    /** Sets the host to the identity and signs; Verify() finds out that the host is the first entry. False without a key pair */
    bool Sign( const CPlayerIdentity& host );

    /**
     *  Whether the roster is well formed (bounds, distinct ids and contacts, the host first and the creator of the table, every key the key of its user id,
     *  no key where the status says there is none) and signed by the host. 'expectedHostKey', when not empty, is the key the host has to have
     */
    bool Verify( const CORE::CString& expectedHostKey, CORE::CString& reason ) const;

    /** Whether the roster is well formed: bounds, distinct ids and contacts, the host first, every key the key of its user id and no key where the status says there is none. Not the signature */
    bool IsWellFormed( CORE::CString& reason ) const;

    /** Whether the signature is the one of the key the roster names for its host, for this match. Not the structure */
    bool VerifySignature( void ) const;

    /** Hash of the signed bytes and the signature: what a decision binds itself to */
    CORE::CString GetDigest( void ) const;

    /** Hash of what the roster says without its revision, time, evidence and signature: two rosters with the same content digest say the same */
    CORE::CString GetContentDigest( void ) const;

    const CMatchRosterEntry* FindEntry( const CORE::CString& userId ) const;
    const CMatchRosterEntry* FindEntryByContact( const CORE::CString& contact ) const;

    /** The participants that vote: introduced and not watching */
    UInt32 CountVoters( void ) const;

    /** The table as a match record would hold it, for the checks of decisions (CMatchDecisionVerifier); the match is INITIALIZING */
    void ToMatchInfo( CMatchInfo& info ) const;

    /** SHA-256 in hex over the settings in the order of their keys */
    static CORE::CString MakeSettingsDigest( const CMatchInfo::TSettingsMap& settings );

    void WriteSigningBytes( CMatchBinaryWriter& writer ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHROSTER_H ? */
