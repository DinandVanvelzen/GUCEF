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

#ifndef GUCEF_MULTIPLAY_CMATCHPROPOSAL_H
#define GUCEF_MULTIPLAY_CMATCHPROPOSAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CMATCHINFO_H
#include "gucefMULTIPLAY_CMatchInfo.h"
#define GUCEF_MULTIPLAY_CMATCHINFO_H
#endif /* GUCEF_MULTIPLAY_CMATCHINFO_H ? */

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#include "gucefMULTIPLAY_CPlayerIdentity.h"
#define GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITY_H ? */

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
 *  A decision about a match, signed by whoever proposes it (in a match with the BOSS decision policy: the
 *  command of an assigned boss). Hand a participant to the computer and drop a participant name the target
 *  in targetUserId; a change of the contact is the player's own (targetUserId is the proposer, value the new
 *  contact); a setting change names the setting in key and the new value in value.
 *
 *  deadline is the end of the voting time in ms since the Unix epoch, 0 for none: votes cast later do not count.
 *
 *  voterUserIds is the electorate: the participants who vote on the proposal, written into the signed proposal when it is made, so the
 *  outcome does not depend on the order in which acceptances and notices reach each machine (a vote of somebody who became a
 *  spectator afterwards stays counted, an invitee who accepts afterwards does not vote). An empty electorate is the participants that
 *  vote when the evidence is checked. The decisions before the start (force start, kick) need one.
 *  targetUserIds names the participants a force start replaces by the computer.
 *  rosterRevision and rosterDigest bind a decision of the time before the record exists to the signed roster of the host (see CMatchRoster).
 *  A move to spectator names the reason code in key (see GUCEF_MULTIPLAY_SPECTATOR_REASON_VOLUNTARY) and the reason in words in value.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchProposal : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString matchId;
    typedef GUCEF::vector< CORE::CString > TUserIdVector;

    static const UInt32 MaxListedUsers = CMatchInfo::MaxParticipants;

    CORE::CString proposalId;           /**< random, a GUID */
    TMatchDecisionKind kind;
    CORE::CString proposerUserId;
    CORE::CString targetUserId;
    CORE::CString key;
    CORE::CString value;
    Int64 createdAt;
    Int64 deadline;
    TUserIdVector voterUserIds;         /**< the electorate; empty for the participants that vote when the evidence is checked */
    TUserIdVector targetUserIds;        /**< the participants a force start replaces by the computer */
    UInt32 rosterRevision;              /**< a decision taken before the match record exists: the revision of the signed roster (CMatchRoster) whose participants and keys it refers to; 0 for none. Signed only when not 0, so the signed bytes of every other proposal are as they were */
    CORE::CString rosterDigest;         /**< the digest of that roster (CMatchRoster::GetDigest()); a proposal that names another digest for a revision is no proposal for the roster that revision is */
    CORE::CString signature;            /**< hex */

    CMatchProposal( void );

    CMatchProposal( const CMatchProposal& src );

    virtual ~CMatchProposal();

    CMatchProposal& operator=( const CMatchProposal& src );

    void WriteSigningBytes( CMatchBinaryWriter& writer ) const;

    /** Sets the proposer to the identity and signs; a proposal id is made when there is none */
    bool Sign( const CPlayerIdentity& proposer );

    /** Writes the participants that vote now (status ACTIVE) as the electorate of the proposal; call before Sign() */
    void SetElectorateFrom( const CMatchInfo& info );

    /** Whether the user is in the electorate */
    bool IsInElectorate( const CORE::CString& userId ) const;

    /** Hash of the signed bytes and the signature: what votes bind themselves to */
    CORE::CString GetDigest( void ) const;

    /** Whether the signature is by the key the match pins for the proposer, for this match */
    bool VerifySignature( const CMatchInfo& info ) const;

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

#endif /* GUCEF_MULTIPLAY_CMATCHPROPOSAL_H ? */
