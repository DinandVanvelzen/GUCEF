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

#ifndef GUCEF_MULTIPLAY_CMATCHDRAFTTALLY_H
#define GUCEF_MULTIPLAY_CMATCHDRAFTTALLY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CMATCHDECISIONVERIFIER_H
#include "gucefMULTIPLAY_CMatchDecisionVerifier.h"
#define GUCEF_MULTIPLAY_CMATCHDECISIONVERIFIER_H
#endif /* GUCEF_MULTIPLAY_CMATCHDECISIONVERIFIER_H ? */

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

/** Where a decision of the draft stage stands, for the display of every participant */
enum EMatchDraftState
{
    MATCHDRAFTSTATE_OPEN = 0 ,          /**< still counting */
    MATCHDRAFTSTATE_PASSED ,            /**< the tally passed here; the host has not carried it out yet (within the time the host is given) */
    MATCHDRAFTSTATE_FAILED ,            /**< it cannot pass any more, or its time is over */
    MATCHDRAFTSTATE_APPLIED ,           /**< the host carried it out and the tally agrees */
    MATCHDRAFTSTATE_HOST_IGNORED ,      /**< it passed here and the host has not carried it out within the time given: a warning, nobody can force the host */
    MATCHDRAFTSTATE_HOST_MISCOUNT ,     /**< the host says it carried it out, and the votes known here do not pass it: a warning */
    MATCHDRAFTSTATE_OBSOLETE            /**< it passed or is open, and what it is about has changed in the meantime (the participant is gone already) */
};
typedef enum EMatchDraftState TMatchDraftState;

/*-------------------------------------------------------------------------*/

/** How one proposal of the draft stage stands: the same figures on every machine that holds the same signed mails */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchDraftTally
{
    public:

    CORE::CString proposalId;
    CORE::CString digest;
    TMatchDecisionKind kind;
    CORE::CString proposerUserId;
    CORE::CString targetUserId;
    GUCEF::vector< CORE::CString > targetUserIds;
    UInt32 rosterRevision;
    UInt32 yes;                                         /**< the proposer included */
    UInt32 no;
    UInt32 needed;
    UInt32 eligible;
    GUCEF::vector< CORE::CString > yesVoters;           /**< user ids, the proposer first */
    GUCEF::vector< CORE::CString > noVoters;
    GUCEF::vector< CORE::CString > notVoted;
    TMatchVoteOutcome outcome;
    TMatchDraftState state;
    bool appliedByHost;                                 /**< a roster of the host names it as carried out */
    Int64 createdAt;
    Int64 deadline;
    Int64 passedSeenAt;                                 /**< when it was first seen to pass here, 0 while it has not */

    CMatchDraftTally( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHDRAFTTALLY_H ? */
