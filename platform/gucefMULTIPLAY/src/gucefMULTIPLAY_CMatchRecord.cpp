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

#include "gucefMULTIPLAY_CMatchRecord.h"

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

CMatchRecord::CMatchRecord( void )
    : CORE::CIDataNodeSerializable()
    , info()
    , signature()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchRecord::CMatchRecord( const CMatchInfo& matchInfo )
    : CORE::CIDataNodeSerializable()
    , info( matchInfo )
    , signature()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchRecord::CMatchRecord( const CMatchRecord& src )
    : CORE::CIDataNodeSerializable( src )
    , info( src.info )
    , signature( src.signature )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchRecord::~CMatchRecord()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchRecord&
CMatchRecord::operator=( const CMatchRecord& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        info = src.info;
        signature = src.signature;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CMatchRecord::Sign( const CPlayerIdentity& creator )
{GUCEF_TRACE;

    signature.Clear();
    const CMatchParticipant* participant = info.FindParticipant( creator.userId );
    if ( GUCEF_NULL == participant || creator.userId != info.creatorUserId || participant->publicKey.Lowercase() != creator.publicKey.Lowercase() )
        return false;

    CORE::CDynamicBuffer bytes;
    info.GetSigningBytes( bytes );
    return CMatchSigning::Sign( creator, CMatchSigning::KindInfo, info.matchId, bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature );
}

/*-------------------------------------------------------------------------*/

bool
CMatchRecord::Verify( CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !info.IsValid( reason ) )
        return false;
    const CMatchParticipant* creator = info.FindParticipant( info.creatorUserId );
    if ( GUCEF_NULL == creator )
    {
        reason = "The creator is not one of the participants";
        return false;
    }
    CORE::CDynamicBuffer bytes;
    info.GetSigningBytes( bytes );
    if ( !CMatchSigning::Verify( creator->publicKey, CMatchSigning::KindInfo, info.matchId, bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature ) )
    {
        reason = "The match record is not signed by its creator";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchRecord::Serialize( CORE::CDataNode& domRootNode                        ,
                         const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    info.Serialize( *domRootNode.AddChild( "info", GUCEF_DATATYPE_OBJECT ), settings );
    domRootNode.SetAttribute( "signature", signature );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchRecord::Deserialize( const CORE::CDataNode& domRootNode                  ,
                           const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    info = CMatchInfo();
    const CORE::CDataNode* infoNode = domRootNode.FindChild( "info" );
    signature = domRootNode.GetAttributeValueOrChildValueByName( "signature" ).AsString( CORE::CString::Empty, true );
    return GUCEF_NULL != infoNode && info.Deserialize( *infoNode, settings );
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchRecord::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchRecord( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchRecord::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CMatchRecord";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CMatchRecord::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchRecord::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchRecord";
}

/*-------------------------------------------------------------------------*/

CMatchAcceptance::CMatchAcceptance( void )
    : CORE::CIDataNodeSerializable()
    , matchId()
    , userId()
    , accepted( false )
    , recordDigest()
    , signature()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchAcceptance::CMatchAcceptance( const CMatchAcceptance& src )
    : CORE::CIDataNodeSerializable( src )
    , matchId( src.matchId )
    , userId( src.userId )
    , accepted( src.accepted )
    , recordDigest( src.recordDigest )
    , signature( src.signature )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchAcceptance::~CMatchAcceptance()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchAcceptance&
CMatchAcceptance::operator=( const CMatchAcceptance& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        matchId = src.matchId;
        userId = src.userId;
        accepted = src.accepted;
        recordDigest = src.recordDigest;
        signature = src.signature;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CMatchAcceptance::GetSigningBytes( CORE::CDynamicBuffer& bytes ) const
{GUCEF_TRACE;

    bytes.Clear();
    CMatchBinaryWriter writer( bytes );
    writer.WriteString( matchId );
    writer.WriteString( userId );
    writer.WriteBool( accepted );
    writer.WriteString( recordDigest );
}

/*-------------------------------------------------------------------------*/

bool
CMatchAcceptance::Sign( const CPlayerIdentity& identity, const CMatchInfo& record, bool accept )
{GUCEF_TRACE;

    matchId = record.matchId;
    userId = identity.userId;
    accepted = accept;
    recordDigest = record.GetDigest();
    signature.Clear();
    const CMatchParticipant* participant = record.FindParticipant( identity.userId );
    if ( GUCEF_NULL == participant || participant->publicKey.Lowercase() != identity.publicKey.Lowercase() )
        return false;

    CORE::CDynamicBuffer bytes;
    GetSigningBytes( bytes );
    return CMatchSigning::Sign( identity, CMatchSigning::KindAcceptance, matchId, bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature );
}

/*-------------------------------------------------------------------------*/

bool
CMatchAcceptance::Verify( const CMatchInfo& record, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( matchId != record.matchId )
    {
        reason = "The acceptance is for another match";
        return false;
    }
    if ( recordDigest != record.GetDigest() )
    {
        reason = "The acceptance is for another version of the match record";
        return false;
    }
    const CMatchParticipant* participant = record.FindParticipant( userId );
    if ( GUCEF_NULL == participant )
    {
        reason = "The acceptance is by someone who is not a participant";
        return false;
    }
    CORE::CDynamicBuffer bytes;
    GetSigningBytes( bytes );
    if ( !CMatchSigning::Verify( participant->publicKey, CMatchSigning::KindAcceptance, matchId, bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature ) )
    {
        reason = "The acceptance is not signed by the key of the participant";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchAcceptance::Serialize( CORE::CDataNode& domRootNode                        ,
                             const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "matchId", matchId );
    domRootNode.SetAttribute( "userId", userId );
    domRootNode.SetAttribute( "accepted", accepted );
    domRootNode.SetAttribute( "recordDigest", recordDigest );
    domRootNode.SetAttribute( "signature", signature );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchAcceptance::Deserialize( const CORE::CDataNode& domRootNode                  ,
                               const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    matchId = domRootNode.GetAttributeValueOrChildValueByName( "matchId" ).AsString( CORE::CString::Empty, true );
    userId = domRootNode.GetAttributeValueOrChildValueByName( "userId" ).AsString( CORE::CString::Empty, true );
    accepted = domRootNode.GetAttributeValueOrChildValueByName( "accepted" ).AsBool( false, true );
    recordDigest = domRootNode.GetAttributeValueOrChildValueByName( "recordDigest" ).AsString( CORE::CString::Empty, true );
    signature = domRootNode.GetAttributeValueOrChildValueByName( "signature" ).AsString( CORE::CString::Empty, true );
    return CMatchId::IsValid( matchId ) && !userId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchAcceptance::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchAcceptance( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchAcceptance::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CMatchAcceptance";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CMatchAcceptance::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchAcceptance::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchAcceptance";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
