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

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHSIGNING_H
#include "gucefMULTIPLAY_CMatchSigning.h"
#define GUCEF_MULTIPLAY_CMATCHSIGNING_H
#endif /* GUCEF_MULTIPLAY_CMATCHSIGNING_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHID_H
#include "gucefMULTIPLAY_CMatchId.h"
#define GUCEF_MULTIPLAY_CMATCHID_H
#endif /* GUCEF_MULTIPLAY_CMATCHID_H ? */

#include "gucefMULTIPLAY_CMatchPreview.h"

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

CMatchPreview::CMatchPreview( void )
    : CORE::CIDataNodeSerializable()
    , matchId()
    , revision( 0 )
    , turn( 0 )
    , currentUserIds()
    , status( MATCHSTATUS_FORMING )
    , deadline( 0 )
    , packageSha256()
    , baseSha256()
    , summary()
    , lastActivity( 0 )
    , issuerKey()
    , signature()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchPreview::CMatchPreview( const CMatchPreview& src )
    : CORE::CIDataNodeSerializable( src )
    , matchId( src.matchId )
    , revision( src.revision )
    , turn( src.turn )
    , currentUserIds( src.currentUserIds )
    , status( src.status )
    , deadline( src.deadline )
    , packageSha256( src.packageSha256 )
    , baseSha256( src.baseSha256 )
    , summary( src.summary )
    , lastActivity( src.lastActivity )
    , issuerKey( src.issuerKey )
    , signature( src.signature )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchPreview::~CMatchPreview()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchPreview&
CMatchPreview::operator=( const CMatchPreview& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        matchId = src.matchId;
        revision = src.revision;
        turn = src.turn;
        currentUserIds = src.currentUserIds;
        status = src.status;
        deadline = src.deadline;
        packageSha256 = src.packageSha256;
        baseSha256 = src.baseSha256;
        summary = src.summary;
        lastActivity = src.lastActivity;
        issuerKey = src.issuerKey;
        signature = src.signature;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CMatchPreview::IsCurrent( const CORE::CString& userId ) const
{GUCEF_TRACE;

    return currentUserIds.find( userId ) != currentUserIds.end();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMatchPreview::GetIssuerUserId( void ) const
{GUCEF_TRACE;

    return CPlayerIdentity::MakeUserIdFromHex( issuerKey );
}

/*-------------------------------------------------------------------------*/

void
CMatchPreview::WriteSigningBytes( CMatchBinaryWriter& writer ) const
{GUCEF_TRACE;

    writer.WriteString( matchId );
    writer.WriteUInt32( revision );
    writer.WriteUInt32( turn );
    writer.WriteUInt8( (UInt8) status );
    writer.WriteInt64( deadline );
    writer.WriteInt64( lastActivity );
    writer.WriteString( packageSha256 );
    writer.WriteString( baseSha256 );
    writer.WriteString( summary );
    writer.WriteString( issuerKey );
    writer.WriteUInt32( (UInt32) currentUserIds.size() );
    CORE::CString::StringSet::const_iterator i = currentUserIds.begin();
    while ( i != currentUserIds.end() )
    {
        writer.WriteString( (*i) );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchPreview::GetSigningBytes( CORE::CDynamicBuffer& bytes ) const
{GUCEF_TRACE;

    bytes.Clear();
    CMatchBinaryWriter writer( bytes );
    WriteSigningBytes( writer );
}

/*-------------------------------------------------------------------------*/

bool
CMatchPreview::Sign( const CPlayerIdentity& issuer )
{GUCEF_TRACE;

    signature.Clear();
    if ( !issuer.HasKeyPair() )
        return false;
    issuerKey = issuer.publicKey.Lowercase();
    CORE::CDynamicBuffer bytes;
    GetSigningBytes( bytes );
    return CMatchSigning::Sign( issuer, CMatchSigning::KindPreview, matchId, bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature );
}

/*-------------------------------------------------------------------------*/

bool
CMatchPreview::VerifySignature( void ) const
{GUCEF_TRACE;

    if ( !IsWithinLimits() )
        return false;
    CORE::CDynamicBuffer bytes;
    GetSigningBytes( bytes );
    return CMatchSigning::Verify( issuerKey, CMatchSigning::KindPreview, matchId, bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature );
}

/*-------------------------------------------------------------------------*/

bool
CMatchPreview::VerifyWithKey( const CORE::CString& pinnedPublicKeyHex ) const
{GUCEF_TRACE;

    return issuerKey.Lowercase() == pinnedPublicKeyHex.Lowercase() && VerifySignature();
}

/*-------------------------------------------------------------------------*/

bool
CMatchPreview::IsWithinLimits( void ) const
{GUCEF_TRACE;

    if ( !CMatchId::IsValid( matchId ) || strlen( summary.C_String() ) > MaxSummaryLength || currentUserIds.size() > MaxCurrentUserIds )
        return false;
    CORE::CString::StringSet::const_iterator i = currentUserIds.begin();
    while ( i != currentUserIds.end() )
    {
        if ( (*i).IsNULLOrEmpty() || strlen( (*i).C_String() ) > CMatchId::TextLength )
            return false;
        ++i;
    }
    return
           ( packageSha256.IsNULLOrEmpty() || CMatchSigning::IsHexOfLength( packageSha256, CMatchSigning::Sha256HexSize ) ) &&
           ( baseSha256.IsNULLOrEmpty() || CMatchSigning::IsHexOfLength( baseSha256, CMatchSigning::Sha256HexSize ) );
}

/*-------------------------------------------------------------------------*/

bool
CMatchPreview::ToBinary( CORE::CDynamicBuffer& bytes ) const
{GUCEF_TRACE;

    bytes.Clear();
    if ( !IsWithinLimits() || !CMatchSigning::IsHexOfLength( signature, CMatchSigning::SignatureHexSize ) )
        return false;
    CMatchBinaryWriter writer( bytes );
    WriteSigningBytes( writer );
    UInt8 raw[ CPlayerIdentity::SignatureSize ];
    if ( !DRN::CDRNCrypto::FromHex( signature, raw, CPlayerIdentity::SignatureSize ) )
    {
        bytes.Clear();
        return false;
    }
    writer.WriteRaw( raw, CPlayerIdentity::SignatureSize );
    return bytes.GetDataSize() <= MaxBinarySize;
}

/*-------------------------------------------------------------------------*/

bool
CMatchPreview::FromBinary( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( GUCEF_NULL == data || size > MaxBinarySize )
        return false;

    CMatchPreview parsed;
    CMatchBinaryReader reader( data, size );
    reader.ReadString( parsed.matchId, CMatchId::TextLength );
    parsed.revision = reader.ReadUInt32();
    parsed.turn = reader.ReadUInt32();
    UInt8 statusValue = reader.ReadUInt8();
    parsed.deadline = reader.ReadInt64();
    parsed.lastActivity = reader.ReadInt64();
    reader.ReadString( parsed.packageSha256, CMatchSigning::Sha256HexSize );
    reader.ReadString( parsed.baseSha256, CMatchSigning::Sha256HexSize );
    reader.ReadString( parsed.summary, MaxSummaryLength );
    reader.ReadString( parsed.issuerKey, CMatchSigning::PublicKeyHexSize );
    UInt32 idCount = reader.ReadUInt32();
    if ( !reader.IsOk() || idCount > MaxCurrentUserIds || statusValue > (UInt8) MATCHSTATUS_CANCELLED )
        return false;
    for ( UInt32 i=0; i<idCount; ++i )
    {
        CORE::CString userId;
        if ( !reader.ReadString( userId, CMatchId::TextLength ) )
            return false;
        parsed.currentUserIds.insert( userId );
    }
    UInt8 raw[ CPlayerIdentity::SignatureSize ];
    if ( !reader.ReadRaw( raw, CPlayerIdentity::SignatureSize ) || !reader.IsAtEndAndOk() )
        return false;
    parsed.status = (TMatchStatus) statusValue;
    parsed.signature = DRN::CDRNCrypto::ToHex( raw, CPlayerIdentity::SignatureSize );
    if ( !parsed.IsWithinLimits() || parsed.currentUserIds.size() != idCount )
        return false;
    *this = parsed;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchPreview::Serialize( CORE::CDataNode& domRootNode                        ,
                          const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "matchId", matchId );
    domRootNode.SetAttribute( "revision", revision );
    domRootNode.SetAttribute( "turn", turn );
    domRootNode.SetAttribute( "status", CORE::CString( MatchStatusToString( status ) ) );
    domRootNode.SetAttribute( "deadline", deadline );
    domRootNode.SetAttribute( "packageSha256", packageSha256 );
    domRootNode.SetAttribute( "baseSha256", baseSha256 );
    domRootNode.SetAttribute( "summary", summary );
    domRootNode.SetAttribute( "lastActivity", lastActivity );
    domRootNode.SetAttribute( "issuerKey", issuerKey );
    domRootNode.SetAttribute( "signature", signature );
    CORE::CDataNode* idsNode = domRootNode.AddChild( "currentUserIds", GUCEF_DATATYPE_ARRAY );
    CORE::CString::StringSet::const_iterator i = currentUserIds.begin();
    while ( i != currentUserIds.end() )
    {
        idsNode->AddChild( "user", GUCEF_DATATYPE_OBJECT )->SetAttribute( "userId", (*i) );
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchPreview::Deserialize( const CORE::CDataNode& domRootNode                  ,
                            const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    matchId = domRootNode.GetAttributeValueOrChildValueByName( "matchId" ).AsString( CORE::CString::Empty, true );
    revision = domRootNode.GetAttributeValueOrChildValueByName( "revision" ).AsUInt32( 0, true );
    turn = domRootNode.GetAttributeValueOrChildValueByName( "turn" ).AsUInt32( 0, true );
    status = StringToMatchStatus( domRootNode.GetAttributeValueOrChildValueByName( "status" ).AsString( CORE::CString::Empty, true ) );
    deadline = domRootNode.GetAttributeValueOrChildValueByName( "deadline" ).AsInt64( 0, true );
    packageSha256 = domRootNode.GetAttributeValueOrChildValueByName( "packageSha256" ).AsString( CORE::CString::Empty, true );
    baseSha256 = domRootNode.GetAttributeValueOrChildValueByName( "baseSha256" ).AsString( CORE::CString::Empty, true );
    summary = domRootNode.GetAttributeValueOrChildValueByName( "summary" ).AsString( CORE::CString::Empty, true );
    lastActivity = domRootNode.GetAttributeValueOrChildValueByName( "lastActivity" ).AsInt64( 0, true );
    issuerKey = domRootNode.GetAttributeValueOrChildValueByName( "issuerKey" ).AsString( CORE::CString::Empty, true );
    signature = domRootNode.GetAttributeValueOrChildValueByName( "signature" ).AsString( CORE::CString::Empty, true );

    currentUserIds.clear();
    const CORE::CDataNode* idsNode = domRootNode.FindChild( "currentUserIds" );
    if ( GUCEF_NULL != idsNode )
    {
        CORE::CDataNode::const_iterator i = idsNode->ConstBegin();
        while ( i != idsNode->ConstEnd() && currentUserIds.size() <= MaxCurrentUserIds )
        {
            CORE::CString userId = (*i)->GetAttributeValueOrChildValueByName( "userId" ).AsString( CORE::CString::Empty, true );
            if ( !userId.IsNULLOrEmpty() )
                currentUserIds.insert( userId );
            ++i;
        }
    }
    return CMatchId::IsValid( matchId );
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchPreview::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchPreview( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchPreview::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CMatchPreview";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CMatchPreview::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchPreview::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchPreview";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
