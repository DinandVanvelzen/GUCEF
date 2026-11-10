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

#ifndef GUCEF_MULTIPLAY_CMATCHSIGNING_H
#include "gucefMULTIPLAY_CMatchSigning.h"
#define GUCEF_MULTIPLAY_CMATCHSIGNING_H
#endif /* GUCEF_MULTIPLAY_CMATCHSIGNING_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHID_H
#include "gucefMULTIPLAY_CMatchId.h"
#define GUCEF_MULTIPLAY_CMATCHID_H
#endif /* GUCEF_MULTIPLAY_CMATCHID_H ? */

#include "gucefMULTIPLAY_CMatchVote.h"

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

CMatchVote::CMatchVote( void )
    : CORE::CIDataNodeSerializable()
    , matchId()
    , proposalId()
    , proposalDigest()
    , voterUserId()
    , yes( false )
    , createdAt( 0 )
    , signature()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchVote::CMatchVote( const CMatchVote& src )
    : CORE::CIDataNodeSerializable( src )
    , matchId( src.matchId )
    , proposalId( src.proposalId )
    , proposalDigest( src.proposalDigest )
    , voterUserId( src.voterUserId )
    , yes( src.yes )
    , createdAt( src.createdAt )
    , signature( src.signature )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchVote::~CMatchVote()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchVote&
CMatchVote::operator=( const CMatchVote& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        matchId = src.matchId;
        proposalId = src.proposalId;
        proposalDigest = src.proposalDigest;
        voterUserId = src.voterUserId;
        yes = src.yes;
        createdAt = src.createdAt;
        signature = src.signature;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CMatchVote::WriteSigningBytes( CMatchBinaryWriter& writer ) const
{GUCEF_TRACE;

    writer.WriteString( matchId );
    writer.WriteString( proposalId );
    writer.WriteString( proposalDigest );
    writer.WriteString( voterUserId );
    writer.WriteBool( yes );
    writer.WriteInt64( createdAt );
}

/*-------------------------------------------------------------------------*/

bool
CMatchVote::Sign( const CPlayerIdentity& voter, const CMatchProposal& proposal, bool voteYes, Int64 now )
{GUCEF_TRACE;

    matchId = proposal.matchId;
    proposalId = proposal.proposalId;
    proposalDigest = proposal.GetDigest();
    voterUserId = voter.userId;
    yes = voteYes;
    createdAt = now;
    CORE::CDynamicBuffer bytes;
    CMatchBinaryWriter writer( bytes );
    WriteSigningBytes( writer );
    return CMatchSigning::Sign( voter, CMatchSigning::KindVote, matchId, bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature );
}

/*-------------------------------------------------------------------------*/

bool
CMatchVote::VerifySignature( const CMatchInfo& info ) const
{GUCEF_TRACE;

    if ( matchId != info.matchId )
        return false;
    const CMatchParticipant* voter = info.FindParticipant( voterUserId );
    if ( GUCEF_NULL == voter )
        return false;
    CORE::CDynamicBuffer bytes;
    CMatchBinaryWriter writer( bytes );
    WriteSigningBytes( writer );
    return CMatchSigning::Verify( voter->publicKey, CMatchSigning::KindVote, matchId, bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature );
}

/*-------------------------------------------------------------------------*/

bool
CMatchVote::Serialize( CORE::CDataNode& domRootNode                        ,
                       const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "matchId", matchId );
    domRootNode.SetAttribute( "proposalId", proposalId );
    domRootNode.SetAttribute( "proposalDigest", proposalDigest );
    domRootNode.SetAttribute( "voterUserId", voterUserId );
    domRootNode.SetAttribute( "yes", yes );
    domRootNode.SetAttribute( "createdAt", createdAt );
    domRootNode.SetAttribute( "signature", signature );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchVote::Deserialize( const CORE::CDataNode& domRootNode                  ,
                         const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    matchId = domRootNode.GetAttributeValueOrChildValueByName( "matchId" ).AsString( CORE::CString::Empty, true );
    proposalId = domRootNode.GetAttributeValueOrChildValueByName( "proposalId" ).AsString( CORE::CString::Empty, true );
    proposalDigest = domRootNode.GetAttributeValueOrChildValueByName( "proposalDigest" ).AsString( CORE::CString::Empty, true );
    voterUserId = domRootNode.GetAttributeValueOrChildValueByName( "voterUserId" ).AsString( CORE::CString::Empty, true );
    yes = domRootNode.GetAttributeValueOrChildValueByName( "yes" ).AsBool( false, true );
    createdAt = domRootNode.GetAttributeValueOrChildValueByName( "createdAt" ).AsInt64( 0, true );
    signature = domRootNode.GetAttributeValueOrChildValueByName( "signature" ).AsString( CORE::CString::Empty, true );
    return CMatchId::IsValid( matchId ) && !voterUserId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchVote::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchVote( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchVote::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CMatchVote";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CMatchVote::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchVote::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchVote";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
