/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNVOTEPOLICY_H
#define GUCEF_DRN_CDRNVOTEPOLICY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How a vote on a write to a PROPOSAL_VOTE grid is decided.
 *  Settings: rule (MAJORITY, UNANIMOUS, QUORUM), quorumFraction, timeoutMs, proposerVoteCounts
 */
class GUCEF_DRN_EXPORT_CPP CDRNVotePolicy : public CORE::CIConfigurable
{
    public:

    static const CORE::CString ClassTypeName;

    enum TVoteRule
    {
        VOTERULE_MAJORITY   = 0,    /**< more than half of the eligible voters say yes */
        VOTERULE_UNANIMOUS  = 1,    /**< every eligible voter says yes */
        VOTERULE_QUORUM     = 2     /**< at least quorumFraction of the eligible voters say yes */
    };

    enum TVoteOutcome
    {
        VOTEOUTCOME_PENDING     = 0,
        VOTEOUTCOME_PASSED      = 1,
        VOTEOUTCOME_FAILED      = 2,
        VOTEOUTCOME_CANCELLED   = 3,
        VOTEOUTCOME_TIMEOUT     = 4
    };

    TVoteRule rule;
    Float32 quorumFraction;
    UInt32 timeoutMs;
    bool proposerVoteCounts;    /**< whether the proposer is one of the voters */

    CDRNVotePolicy( void );
    virtual ~CDRNVotePolicy();

    /** The outcome so far; a vote with no eligible voters passes */
    TVoteOutcome Evaluate( UInt32 eligibleVoters, UInt32 yesVotes, UInt32 noVotes, bool timedOut ) const;

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static CORE::CString RuleToString( TVoteRule rule );
    static TVoteRule StringToRule( const CORE::CString& rule, TVoteRule defaultRule );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNVOTEPOLICY_H ? */
