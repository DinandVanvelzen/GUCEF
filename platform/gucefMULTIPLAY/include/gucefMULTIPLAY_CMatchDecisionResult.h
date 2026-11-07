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

#ifndef GUCEF_MULTIPLAY_CMATCHDECISIONRESULT_H
#define GUCEF_MULTIPLAY_CMATCHDECISIONRESULT_H

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

enum EMatchVoteOutcome
{
    MATCHVOTEOUTCOME_UNDECIDED = 0 ,
    MATCHVOTEOUTCOME_PASSED    = 1 ,
    MATCHVOTEOUTCOME_FAILED    = 2
};
typedef enum EMatchVoteOutcome TMatchVoteOutcome;

/*-------------------------------------------------------------------------*/

/** What CMatchDecisionVerifier::Evaluate() found out */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchDecisionResult
{
    public:

    bool valid;                     /**< whether the evidence is well formed, correctly signed and by the right people */
    TMatchVoteOutcome outcome;      /**< only meaningful when valid */
    UInt32 yes;                     /**< the yes votes, the proposer included */
    UInt32 no;
    UInt32 needed;                  /**< the yes votes that pass */
    UInt32 eligible;                /**< the participants that vote */
    CORE::CString reason;           /**< why the evidence is not valid, or why it did not pass */

    CMatchDecisionResult( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHDECISIONRESULT_H ? */
