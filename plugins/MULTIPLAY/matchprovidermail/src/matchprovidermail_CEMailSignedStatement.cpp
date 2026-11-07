/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#include "matchprovidermail_CEMailSignedStatement.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
IsKnownStatementKind( const CORE::CString& kind )
{GUCEF_TRACE;

    return kind == CEMailSignedStatement::KindNudge || kind == CEMailSignedStatement::KindResign;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailSignedStatement::CEMailSignedStatement( void )
    : CORE::CIDataNodeSerializable()
    , matchId()
    , statementKind()
    , userId()
    , targetUserId()
    , revision( 0 )
    , createdAt( 0 )
    , signature()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailSignedStatement::CEMailSignedStatement( const CEMailSignedStatement& src )
    : CORE::CIDataNodeSerializable( src )
    , matchId( src.matchId )
    , statementKind( src.statementKind )
    , userId( src.userId )
    , targetUserId( src.targetUserId )
    , revision( src.revision )
    , createdAt( src.createdAt )
    , signature( src.signature )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailSignedStatement::~CEMailSignedStatement()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailSignedStatement&
CEMailSignedStatement::operator=( const CEMailSignedStatement& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        matchId = src.matchId;
        statementKind = src.statementKind;
        userId = src.userId;
        targetUserId = src.targetUserId;
        revision = src.revision;
        createdAt = src.createdAt;
        signature = src.signature;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CEMailSignedStatement::GetSigningBytes( CORE::CDynamicBuffer& bytes ) const
{GUCEF_TRACE;

    bytes.Clear();
    MULTIPLAY::CMatchBinaryWriter writer( bytes );
    writer.WriteString( userId );
    writer.WriteString( targetUserId );
    writer.WriteUInt32( revision );
    writer.WriteInt64( createdAt );
}

/*-------------------------------------------------------------------------*/

bool
CEMailSignedStatement::Sign( const MULTIPLAY::CPlayerIdentity& identity )
{GUCEF_TRACE;

    if ( !IsKnownStatementKind( statementKind ) || !MULTIPLAY::CMatchId::IsValid( matchId ) || !identity.HasKeyPair() )
        return false;
    userId = identity.userId;
    CORE::CDynamicBuffer bytes;
    GetSigningBytes( bytes );
    return MULTIPLAY::CMatchSigning::Sign( identity, statementKind.C_String(), matchId, bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature );
}

/*-------------------------------------------------------------------------*/

bool
CEMailSignedStatement::Verify( const MULTIPLAY::CMatchInfo& info, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !IsKnownStatementKind( statementKind ) )
    {
        reason = "The kind of the statement is not known";
        return false;
    }
    if ( matchId != info.matchId )
    {
        reason = "The statement is about another match";
        return false;
    }
    const MULTIPLAY::CMatchParticipant* signer = info.FindParticipant( userId );
    if ( GUCEF_NULL == signer )
    {
        reason = "The signer does not take part in the match";
        return false;
    }
    CORE::CDynamicBuffer bytes;
    GetSigningBytes( bytes );
    if ( !MULTIPLAY::CMatchSigning::Verify( signer->publicKey, statementKind.C_String(), matchId, bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature ) )
    {
        reason = "The signature of the statement is not valid";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailSignedStatement::Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "matchId", matchId );
    domRootNode.SetAttribute( "statementKind", statementKind );
    domRootNode.SetAttribute( "userId", userId );
    domRootNode.SetAttribute( "targetUserId", targetUserId );
    domRootNode.SetAttribute( "revision", revision );
    domRootNode.SetAttribute( "createdAt", createdAt );
    domRootNode.SetAttribute( "signature", signature );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailSignedStatement::Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    matchId = domRootNode.GetAttributeValueOrChildValueByName( "matchId" ).AsString( CORE::CString::Empty, true );
    statementKind = domRootNode.GetAttributeValueOrChildValueByName( "statementKind" ).AsString( CORE::CString::Empty, true );
    userId = domRootNode.GetAttributeValueOrChildValueByName( "userId" ).AsString( CORE::CString::Empty, true );
    targetUserId = domRootNode.GetAttributeValueOrChildValueByName( "targetUserId" ).AsString( CORE::CString::Empty, true );
    revision = domRootNode.GetAttributeValueOrChildValueByName( "revision" ).AsUInt32( 0, true );
    createdAt = domRootNode.GetAttributeValueOrChildValueByName( "createdAt" ).AsInt64( 0, true );
    signature = domRootNode.GetAttributeValueOrChildValueByName( "signature" ).AsString( CORE::CString::Empty, true );
    return MULTIPLAY::CMatchId::IsValid( matchId ) && IsKnownStatementKind( statementKind ) && !userId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CEMailSignedStatement::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CEMailSignedStatement( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEMailSignedStatement::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailSignedStatement";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CEMailSignedStatement::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CEMailSignedStatement::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailSignedStatement";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
