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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRN_CDRNVotePolicy.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNVotePolicy::ClassTypeName = "GUCEF::DRN::CDRNVotePolicy";

/*-------------------------------------------------------------------------*/

CDRNVotePolicy::CDRNVotePolicy( void )
    : CORE::CIConfigurable()
    , rule( VOTERULE_MAJORITY )
    , quorumFraction( 0.5f )
    , timeoutMs( 5000 )
    , proposerVoteCounts( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNVotePolicy::~CDRNVotePolicy()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNVotePolicy::TVoteOutcome
CDRNVotePolicy::Evaluate( UInt32 eligibleVoters, UInt32 yesVotes, UInt32 noVotes, bool timedOut ) const
{GUCEF_TRACE;

    if ( 0 == eligibleVoters )
        return VOTEOUTCOME_PASSED;

    switch ( rule )
    {
        case VOTERULE_UNANIMOUS:
        {
            if ( noVotes > 0 )
                return VOTEOUTCOME_FAILED;
            if ( yesVotes >= eligibleVoters )
                return VOTEOUTCOME_PASSED;
            break;
        }
        case VOTERULE_QUORUM:
        {
            Float32 fraction = quorumFraction < 0.0f ? 0.0f : ( quorumFraction > 1.0f ? 1.0f : quorumFraction );
            UInt32 needed = (UInt32) ( fraction * eligibleVoters + 0.999f );
            if ( 0 == needed )
                needed = 1;
            if ( yesVotes >= needed )
                return VOTEOUTCOME_PASSED;
            if ( eligibleVoters - noVotes < needed )
                return VOTEOUTCOME_FAILED;
            break;
        }
        case VOTERULE_MAJORITY:
        default:
        {
            if ( yesVotes * 2 > eligibleVoters )
                return VOTEOUTCOME_PASSED;
            if ( noVotes * 2 >= eligibleVoters )
                return VOTEOUTCOME_FAILED;
            break;
        }
    }
    return timedOut ? VOTEOUTCOME_TIMEOUT : VOTEOUTCOME_PENDING;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNVotePolicy::RuleToString( TVoteRule rule )
{GUCEF_TRACE;

    switch ( rule )
    {
        case VOTERULE_UNANIMOUS: return "UNANIMOUS";
        case VOTERULE_QUORUM: return "QUORUM";
        default: return "MAJORITY";
    }
}

/*-------------------------------------------------------------------------*/

CDRNVotePolicy::TVoteRule
CDRNVotePolicy::StringToRule( const CORE::CString& rule, TVoteRule defaultRule )
{GUCEF_TRACE;

    CORE::CString lower = rule.Lowercase();
    if ( lower == "majority" )
        return VOTERULE_MAJORITY;
    if ( lower == "unanimous" )
        return VOTERULE_UNANIMOUS;
    if ( lower == "quorum" )
        return VOTERULE_QUORUM;
    return defaultRule;
}

/*-------------------------------------------------------------------------*/

bool
CDRNVotePolicy::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    totalSuccess = cfg.SetAttribute( "rule", RuleToString( rule ) ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "quorumFraction", quorumFraction ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "timeoutMs", timeoutMs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "proposerVoteCounts", proposerVoteCounts ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDRNVotePolicy::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    rule = StringToRule( cfg.GetAttributeValueOrChildValueByName( "rule" ).AsString( RuleToString( rule ), true ), rule );
    quorumFraction = cfg.GetAttributeValueOrChildValueByName( "quorumFraction" ).AsFloat32( quorumFraction, true );
    timeoutMs = cfg.GetAttributeValueOrChildValueByName( "timeoutMs" ).AsUInt32( timeoutMs, true );
    proposerVoteCounts = cfg.GetAttributeValueOrChildValueByName( "proposerVoteCounts" ).AsBool( proposerVoteCounts, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNVotePolicy::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
