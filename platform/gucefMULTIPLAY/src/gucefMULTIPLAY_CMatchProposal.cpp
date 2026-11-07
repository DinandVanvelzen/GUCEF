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

#ifndef GUCEF_MULTIPLAY_CMATCHSIGNING_H
#include "gucefMULTIPLAY_CMatchSigning.h"
#define GUCEF_MULTIPLAY_CMATCHSIGNING_H
#endif /* GUCEF_MULTIPLAY_CMATCHSIGNING_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHID_H
#include "gucefMULTIPLAY_CMatchId.h"
#define GUCEF_MULTIPLAY_CMATCHID_H
#endif /* GUCEF_MULTIPLAY_CMATCHID_H ? */

#include "gucefMULTIPLAY_CMatchProposal.h"

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

CMatchProposal::CMatchProposal( void )
    : CORE::CIDataNodeSerializable()
    , matchId()
    , proposalId()
    , kind( MATCHDECISION_HAND_TO_COMPUTER )
    , proposerUserId()
    , targetUserId()
    , key()
    , value()
    , createdAt( 0 )
    , deadline( 0 )
    , voterUserIds()
    , targetUserIds()
    , rosterRevision( 0 )
    , rosterDigest()
    , signature()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchProposal::CMatchProposal( const CMatchProposal& src )
    : CORE::CIDataNodeSerializable( src )
    , matchId( src.matchId )
    , proposalId( src.proposalId )
    , kind( src.kind )
    , proposerUserId( src.proposerUserId )
    , targetUserId( src.targetUserId )
    , key( src.key )
    , value( src.value )
    , createdAt( src.createdAt )
    , deadline( src.deadline )
    , voterUserIds( src.voterUserIds )
    , targetUserIds( src.targetUserIds )
    , rosterRevision( src.rosterRevision )
    , rosterDigest( src.rosterDigest )
    , signature( src.signature )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchProposal::~CMatchProposal()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchProposal&
CMatchProposal::operator=( const CMatchProposal& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        matchId = src.matchId;
        proposalId = src.proposalId;
        kind = src.kind;
        proposerUserId = src.proposerUserId;
        targetUserId = src.targetUserId;
        key = src.key;
        value = src.value;
        createdAt = src.createdAt;
        deadline = src.deadline;
        voterUserIds = src.voterUserIds;
        targetUserIds = src.targetUserIds;
        rosterRevision = src.rosterRevision;
        rosterDigest = src.rosterDigest;
        signature = src.signature;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CMatchProposal::SetElectorateFrom( const CMatchInfo& info )
{GUCEF_TRACE;

    voterUserIds.clear();
    for ( size_t i=0; i<info.participants.size(); ++i )
        if ( info.participants[ i ].IsEligibleVoter() )
            voterUserIds.push_back( info.participants[ i ].userId );
}

/*-------------------------------------------------------------------------*/

bool
CMatchProposal::IsInElectorate( const CORE::CString& userId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<voterUserIds.size(); ++i )
        if ( voterUserIds[ i ] == userId )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CMatchProposal::WriteSigningBytes( CMatchBinaryWriter& writer ) const
{GUCEF_TRACE;

    writer.WriteString( matchId );
    writer.WriteString( proposalId );
    writer.WriteUInt8( (UInt8) kind );
    writer.WriteString( proposerUserId );
    writer.WriteString( targetUserId );
    writer.WriteString( key );
    writer.WriteString( value );
    writer.WriteInt64( createdAt );
    writer.WriteInt64( deadline );
    writer.WriteUInt8( (UInt8) ( voterUserIds.size() > 255 ? 255 : voterUserIds.size() ) );
    for ( size_t i=0; i<voterUserIds.size() && i<255; ++i )
        writer.WriteString( voterUserIds[ i ] );
    writer.WriteUInt8( (UInt8) ( targetUserIds.size() > 255 ? 255 : targetUserIds.size() ) );
    for ( size_t i=0; i<targetUserIds.size() && i<255; ++i )
        writer.WriteString( targetUserIds[ i ] );
    if ( 0 != rosterRevision )
    {
        // only a decision of the time before the record exists has these: nothing else changes in the bytes that are signed
        writer.WriteUInt32( rosterRevision );
        writer.WriteString( rosterDigest );
    }
}

/*-------------------------------------------------------------------------*/

bool
CMatchProposal::Sign( const CPlayerIdentity& proposer )
{GUCEF_TRACE;

    signature.Clear();
    if ( proposalId.IsNULLOrEmpty() )
        proposalId = CMatchId::Generate();
    proposerUserId = proposer.userId;
    CORE::CDynamicBuffer bytes;
    CMatchBinaryWriter writer( bytes );
    WriteSigningBytes( writer );
    return CMatchSigning::Sign( proposer, CMatchSigning::KindProposal, matchId, bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMatchProposal::GetDigest( void ) const
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    CMatchBinaryWriter writer( bytes );
    WriteSigningBytes( writer );
    writer.WriteString( signature );
    return CMatchSigning::Sha256Hex( bytes.GetConstBufferPtr(), bytes.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
CMatchProposal::VerifySignature( const CMatchInfo& info ) const
{GUCEF_TRACE;

    if ( matchId != info.matchId )
        return false;
    const CMatchParticipant* proposer = info.FindParticipant( proposerUserId );
    if ( GUCEF_NULL == proposer )
        return false;
    CORE::CDynamicBuffer bytes;
    CMatchBinaryWriter writer( bytes );
    WriteSigningBytes( writer );
    return CMatchSigning::Verify( proposer->publicKey, CMatchSigning::KindProposal, matchId, bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature );
}

/*-------------------------------------------------------------------------*/

bool
CMatchProposal::Serialize( CORE::CDataNode& domRootNode                        ,
                           const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "matchId", matchId );
    domRootNode.SetAttribute( "proposalId", proposalId );
    domRootNode.SetAttribute( "kind", CORE::CString( MatchDecisionKindToString( kind ) ) );
    domRootNode.SetAttribute( "proposerUserId", proposerUserId );
    domRootNode.SetAttribute( "targetUserId", targetUserId );
    domRootNode.SetAttribute( "key", key );
    domRootNode.SetAttribute( "value", value );
    domRootNode.SetAttribute( "createdAt", createdAt );
    domRootNode.SetAttribute( "deadline", deadline );
    domRootNode.SetAttribute( "signature", signature );
    if ( 0 != rosterRevision )
    {
        domRootNode.SetAttribute( "rosterRevision", rosterRevision );
        domRootNode.SetAttribute( "rosterDigest", rosterDigest );
    }
    CORE::CDataNode* voters = domRootNode.AddChild( "voters", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<voterUserIds.size(); ++i )
        voters->AddChild( "voter", GUCEF_DATATYPE_OBJECT )->SetAttribute( "userId", voterUserIds[ i ] );
    CORE::CDataNode* targets = domRootNode.AddChild( "targets", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<targetUserIds.size(); ++i )
        targets->AddChild( "target", GUCEF_DATATYPE_OBJECT )->SetAttribute( "userId", targetUserIds[ i ] );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchProposal::Deserialize( const CORE::CDataNode& domRootNode                  ,
                             const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    matchId = domRootNode.GetAttributeValueOrChildValueByName( "matchId" ).AsString( CORE::CString::Empty, true );
    proposalId = domRootNode.GetAttributeValueOrChildValueByName( "proposalId" ).AsString( CORE::CString::Empty, true );
    proposerUserId = domRootNode.GetAttributeValueOrChildValueByName( "proposerUserId" ).AsString( CORE::CString::Empty, true );
    targetUserId = domRootNode.GetAttributeValueOrChildValueByName( "targetUserId" ).AsString( CORE::CString::Empty, true );
    key = domRootNode.GetAttributeValueOrChildValueByName( "key" ).AsString( CORE::CString::Empty, true );
    value = domRootNode.GetAttributeValueOrChildValueByName( "value" ).AsString( CORE::CString::Empty, true );
    createdAt = domRootNode.GetAttributeValueOrChildValueByName( "createdAt" ).AsInt64( 0, true );
    deadline = domRootNode.GetAttributeValueOrChildValueByName( "deadline" ).AsInt64( 0, true );
    signature = domRootNode.GetAttributeValueOrChildValueByName( "signature" ).AsString( CORE::CString::Empty, true );
    rosterRevision = domRootNode.GetAttributeValueOrChildValueByName( "rosterRevision" ).AsUInt32( 0, true );
    rosterDigest = domRootNode.GetAttributeValueOrChildValueByName( "rosterDigest" ).AsString( CORE::CString::Empty, true );

    // stored and received data is not trusted: at most as many users as a match has participants are read, one more than allowed is kept so that the check refuses it
    voterUserIds.clear();
    targetUserIds.clear();
    for ( int list=0; list<2; ++list )
    {
        const CORE::CDataNode* node = domRootNode.FindChild( 0 == list ? "voters" : "targets" );
        TUserIdVector& users = 0 == list ? voterUserIds : targetUserIds;
        if ( GUCEF_NULL == node )
            continue;
        CORE::CDataNode::const_iterator i = node->ConstBegin();
        while ( i != node->ConstEnd() && users.size() <= MaxListedUsers )
        {
            users.push_back( (*i)->GetAttributeValueOrChildValueByName( "userId" ).AsString( CORE::CString::Empty, true ) );
            ++i;
        }
    }
    return CMatchId::IsValid( matchId ) &&
           TryStringToMatchDecisionKind( domRootNode.GetAttributeValueOrChildValueByName( "kind" ).AsString( CORE::CString::Empty, true ), kind );
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchProposal::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchProposal( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchProposal::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CMatchProposal";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CMatchProposal::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchProposal::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchProposal";
}

/*-------------------------------------------------------------------------*/

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
