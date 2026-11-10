/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#ifndef GUCEF_COM_CEMAILADDRESS_H
#include "gucefCOM_CEMailAddress.h"
#define GUCEF_COM_CEMAILADDRESS_H
#endif /* GUCEF_COM_CEMAILADDRESS_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREQUEST_H
#include "matchprovidermail_CEMailIntroRequest.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREQUEST_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREQUEST_H ? */

#include "matchprovidermail_CEMailIntroReply.h"

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

static const Int64 g_maxValidityMs = 90LL * 24 * 60 * 60 * 1000;
static const UInt32 g_maxUserIdBytes = 128;
static const UInt32 g_maxChildNodes = 16;

/** Text for a person or a mail: no control characters, within the bytes */
static bool
IsPlainText( const CORE::CString& text, UInt32 maxBytes, bool mayBeEmpty )
{GUCEF_TRACE;

    const UInt32 size = text.Length();
    if ( size > maxBytes || ( !mayBeEmpty && 0 == size ) )
        return false;
    for ( UInt32 i=0; i<size; ++i )
    {
        const unsigned char c = (unsigned char) text[ i ];
        if ( c < 0x20 || 0x7F == c )
            return false;
    }
    return true;
}

static bool
IsMailAddressText( const CORE::CString& address )
{GUCEF_TRACE;

    return IsPlainText( address, CEMailIntroRequest::MaxAddressBytes, false ) && COM::CEMailAddress( address ).IsValid();
}

static bool
IsNonce( const CORE::CString& nonce )
{GUCEF_TRACE;

    return MULTIPLAY::CMatchSigning::IsHexOfLength( nonce, CEMailIntroRequest::NonceHexSize );
}

static bool
IsKeyOfUserId( const CORE::CString& publicKey, const CORE::CString& userId )
{GUCEF_TRACE;

    return MULTIPLAY::CMatchSigning::IsHexOfLength( publicKey, MULTIPLAY::CMatchSigning::PublicKeyHexSize ) && userId == MULTIPLAY::CPlayerIdentity::MakeUserIdFromHex( publicKey.Lowercase() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailIntroReply::CEMailIntroReply( void )
    : CORE::CIDataNodeSerializable()
    , matchId()
    , nonce()
    , accepted( false )
    , userId()
    , publicKey()
    , displayName()
    , inviteeAddress()
    , creatorAddress()
    , createdAt( 0 )
    , signature()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailIntroReply::CEMailIntroReply( const CEMailIntroReply& src )
    : CORE::CIDataNodeSerializable( src )
    , matchId( src.matchId )
    , nonce( src.nonce )
    , accepted( src.accepted )
    , userId( src.userId )
    , publicKey( src.publicKey )
    , displayName( src.displayName )
    , inviteeAddress( src.inviteeAddress )
    , creatorAddress( src.creatorAddress )
    , createdAt( src.createdAt )
    , signature( src.signature )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailIntroReply::~CEMailIntroReply()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailIntroReply&
CEMailIntroReply::operator=( const CEMailIntroReply& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        matchId = src.matchId;
        nonce = src.nonce;
        accepted = src.accepted;
        userId = src.userId;
        publicKey = src.publicKey;
        displayName = src.displayName;
        inviteeAddress = src.inviteeAddress;
        creatorAddress = src.creatorAddress;
        createdAt = src.createdAt;
        signature = src.signature;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CEMailIntroReply::GetSigningBytes( CORE::CDynamicBuffer& bytes ) const
{GUCEF_TRACE;

    bytes.Clear();
    MULTIPLAY::CMatchBinaryWriter writer( bytes );
    writer.WriteBool( accepted );
    writer.WriteString( nonce );
    writer.WriteString( userId );
    writer.WriteString( publicKey );
    writer.WriteString( displayName );
    writer.WriteString( inviteeAddress );
    writer.WriteString( creatorAddress );
    writer.WriteInt64( createdAt );
}

/*-------------------------------------------------------------------------*/

bool
CEMailIntroReply::Sign( const MULTIPLAY::CPlayerIdentity& identity )
{GUCEF_TRACE;

    if ( !identity.HasKeyPair() || !MULTIPLAY::CMatchId::IsValid( matchId ) )
        return false;
    accepted = true;
    userId = identity.userId;
    publicKey = identity.publicKey.Lowercase();
    if ( displayName.IsNULLOrEmpty() )
        displayName = identity.displayName;
    signature.Clear();
    CORE::CDynamicBuffer bytes;
    GetSigningBytes( bytes );
    return MULTIPLAY::CMatchSigning::Sign( identity, SignedKind, matchId, bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature );
}

/*-------------------------------------------------------------------------*/

bool
CEMailIntroReply::IsWellFormed( CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !MULTIPLAY::CMatchId::IsValid( matchId ) || !IsNonce( nonce ) )
        reason = "The match id or the nonce is not valid";
    else
    if ( !IsMailAddressText( inviteeAddress ) || !IsMailAddressText( creatorAddress ) )
        reason = "An address is not valid";
    else
    if ( !IsPlainText( displayName, CEMailIntroRequest::MaxNameBytes, true ) || createdAt <= 0 )
        reason = "The name or the time is not valid";
    else
    if ( !accepted )
    {
        // a decline holds no key and no signature
        if ( !userId.IsNULLOrEmpty() || !publicKey.IsNULLOrEmpty() || !signature.IsNULLOrEmpty() || !displayName.IsNULLOrEmpty() )
            reason = "A decline holds nothing but the nonce";
        else
        {
            reason.Clear();
            return true;
        }
    }
    else
    if ( !MULTIPLAY::CMatchSigning::IsHexOfLength( publicKey, MULTIPLAY::CMatchSigning::PublicKeyHexSize ) || !IsPlainText( userId, g_maxUserIdBytes, false ) ||
         !MULTIPLAY::CMatchSigning::IsHexOfLength( signature, MULTIPLAY::CMatchSigning::SignatureHexSize ) )
        reason = "The key or the signature is not valid";
    else
    {
        reason.Clear();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CEMailIntroReply::Verify( CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !IsWellFormed( reason ) )
        return false;
    if ( !accepted )
        return true;
    if ( !IsKeyOfUserId( publicKey, userId ) )
    {
        reason = "The user id is not the one of the key";
        return false;
    }
    CORE::CDynamicBuffer bytes;
    GetSigningBytes( bytes );
    if ( !MULTIPLAY::CMatchSigning::Verify( publicKey, SignedKind, matchId, bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature ) )
    {
        reason = "The signature is not valid";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailIntroReply::Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "matchId", matchId );
    domRootNode.SetAttribute( "nonce", nonce );
    domRootNode.SetAttribute( "accepted", accepted );
    domRootNode.SetAttribute( "userId", userId );
    domRootNode.SetAttribute( "publicKey", publicKey );
    domRootNode.SetAttribute( "displayName", displayName );
    domRootNode.SetAttribute( "inviteeAddress", inviteeAddress );
    domRootNode.SetAttribute( "creatorAddress", creatorAddress );
    domRootNode.SetAttribute( "createdAt", createdAt );
    domRootNode.SetAttribute( "signature", signature );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailIntroReply::Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    matchId = domRootNode.GetAttributeValueOrChildValueByName( "matchId" ).AsString( CORE::CString::Empty, true );
    nonce = domRootNode.GetAttributeValueOrChildValueByName( "nonce" ).AsString( CORE::CString::Empty, true );
    accepted = domRootNode.GetAttributeValueOrChildValueByName( "accepted" ).AsBool( false, true );
    userId = domRootNode.GetAttributeValueOrChildValueByName( "userId" ).AsString( CORE::CString::Empty, true );
    publicKey = domRootNode.GetAttributeValueOrChildValueByName( "publicKey" ).AsString( CORE::CString::Empty, true );
    displayName = domRootNode.GetAttributeValueOrChildValueByName( "displayName" ).AsString( CORE::CString::Empty, true );
    inviteeAddress = domRootNode.GetAttributeValueOrChildValueByName( "inviteeAddress" ).AsString( CORE::CString::Empty, true );
    creatorAddress = domRootNode.GetAttributeValueOrChildValueByName( "creatorAddress" ).AsString( CORE::CString::Empty, true );
    createdAt = domRootNode.GetAttributeValueOrChildValueByName( "createdAt" ).AsInt64( 0, true );
    signature = domRootNode.GetAttributeValueOrChildValueByName( "signature" ).AsString( CORE::CString::Empty, true );
    return MULTIPLAY::CMatchId::IsValid( matchId );
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CEMailIntroReply::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CEMailIntroReply( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEMailIntroReply::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailIntroReply";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CEMailIntroReply::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CEMailIntroReply::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailIntroReply";
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
