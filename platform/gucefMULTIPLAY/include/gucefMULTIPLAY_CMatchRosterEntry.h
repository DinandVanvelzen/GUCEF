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

#ifndef GUCEF_MULTIPLAY_CMATCHROSTERENTRY_H
#define GUCEF_MULTIPLAY_CMATCHROSTERENTRY_H

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHROSTERENTRY_H ? */
