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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefMULTIPLAY_CMatchDecisionEvidence.h"

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

CMatchDecisionEvidence::CMatchDecisionEvidence( void )
    : CORE::CIDataNodeSerializable()
    , proposal()
    , votes()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchDecisionEvidence::CMatchDecisionEvidence( const CMatchDecisionEvidence& src )
    : CORE::CIDataNodeSerializable( src )
    , proposal( src.proposal )
    , votes( src.votes )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchDecisionEvidence::~CMatchDecisionEvidence()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchDecisionEvidence&
CMatchDecisionEvidence::operator=( const CMatchDecisionEvidence& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        proposal = src.proposal;
        votes = src.votes;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDecisionEvidence::Serialize( CORE::CDataNode& domRootNode                        ,
                                   const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    proposal.Serialize( *domRootNode.AddChild( "proposal", GUCEF_DATATYPE_OBJECT ), settings );
    CORE::CDataNode* votesNode = domRootNode.AddChild( "votes", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<votes.size(); ++i )
        votes[ i ].Serialize( *votesNode->AddChild( "vote", GUCEF_DATATYPE_OBJECT ), settings );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchDecisionEvidence::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                     const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    proposal = CMatchProposal();
    votes.clear();
    const CORE::CDataNode* proposalNode = domRootNode.FindChild( "proposal" );
    if ( GUCEF_NULL == proposalNode || !proposal.Deserialize( *proposalNode, settings ) )
        return false;

    const CORE::CDataNode* votesNode = domRootNode.FindChild( "votes" );
    if ( GUCEF_NULL != votesNode )
    {
        CORE::CDataNode::const_iterator i = votesNode->ConstBegin();
        while ( i != votesNode->ConstEnd() )
        {
            // more votes than there are seats is not evidence, it is refused whole
            if ( votes.size() >= MaxVotes )
                return false;
            CMatchVote vote;
            if ( !vote.Deserialize( *(*i), settings ) )
                return false;
            votes.push_back( vote );
            ++i;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchDecisionEvidence::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchDecisionEvidence( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchDecisionEvidence::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CMatchDecisionEvidence";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CMatchDecisionEvidence::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchDecisionEvidence::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchDecisionEvidence";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
