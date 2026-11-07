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

#ifndef GUCEF_MULTIPLAY_CMATCHPARTICIPANT_H
#define GUCEF_MULTIPLAY_CMATCHPARTICIPANT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_MULTIPLAY_MATCHTYPES_H
#include "gucefMULTIPLAY_MatchTypes.h"
#define GUCEF_MULTIPLAY_MATCHTYPES_H
#endif /* GUCEF_MULTIPLAY_MATCHTYPES_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHBINARY_H
#include "gucefMULTIPLAY_CMatchBinary.h"
#define GUCEF_MULTIPLAY_CMATCHBINARY_H
#endif /* GUCEF_MULTIPLAY_CMATCHBINARY_H ? */

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
 *  A player in a match. The public key is pinned when the match is created: from then on every record
 *  that claims to come from this participant is checked against this key, and the user id has to be the
 *  one that follows from the key (CPlayerIdentity::MakeUserIdFromHex()).
 *
 *  contact is opaque to the generic layer and defined by the provider that carries the match: for peer mail the
 *  address of the participant, for a relay a handle that stays on the relay.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchParticipant : public CORE::CIDataNodeSerializable
{
    public:

    static const UInt32 MaxDisplayNameLength = 64;
    static const UInt32 MaxContactLength = 256;
    static const UInt32 MaxStatusCodeLength = 32;
    static const UInt32 MaxStatusReasonLength = 128;

    CORE::CString userId;
    CORE::CString displayName;
    UInt32 slot;                            /**< the order in which the participants take turns */
    CORE::CString publicKey;                /**< Ed25519 public key in hex, pinned */
    TMatchParticipantStatus status;
    TMatchOutcome outcome;
    CORE::CString contact;
    bool boss;                              /**< may issue decisions alone in a match with the BOSS decision policy */
    CORE::CString statusCode;               /**< why a spectator watches: a stable upper case word (see GUCEF_MULTIPLAY_SPECTATOR_REASON_VOLUNTARY); not signed */
    CORE::CString statusReason;             /**< the same in a short sentence for the players; not signed */

    CMatchParticipant( void );

    CMatchParticipant( const CMatchParticipant& src );

    virtual ~CMatchParticipant();

    CMatchParticipant& operator=( const CMatchParticipant& src );

    bool operator==( const CMatchParticipant& other ) const;

    /** Whether the participant votes: it accepted and still takes part as a person */
    bool IsEligibleVoter( void ) const;

    /** Whether the participant watches: it gets the turns, holds none, has no seat and does not vote */
    bool IsSpectator( void ) const;

    /** Whether the spectator watches because the game cannot be played by this participant, not by choice */
    bool IsIncompatible( void ) const;

    /**
     *  Appends the fields in the fixed layout that is signed and hashed. The status and outcome are not part of it:
     *  they change during the match and follow from signed acceptances and decisions
     */
    void WriteSigningBytes( CMatchBinaryWriter& writer ) const;

    /** Whether the fields are within their limits and the user id is the one of the public key */
    bool IsValid( CORE::CString& reason ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CMatchParticipant > TMatchParticipantVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHPARTICIPANT_H ? */
