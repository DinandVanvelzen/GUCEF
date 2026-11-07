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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_COM_CMAILADDRESS_H
#include "gucefCOM_CMailAddress.h"
#define GUCEF_COM_CMAILADDRESS_H
#endif /* GUCEF_COM_CMAILADDRESS_H ? */

#include "matchprovidermail_CMailIntroduction.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CMailIntroRequest::SignedKind = "intro-request";
const char* CMailIntroReply::SignedKind = "intro-reply";

static const Int64 g_maxValidityMs = 90LL * 24 * 60 * 60 * 1000;
static const UInt32 g_maxUserIdBytes = 128;
static const UInt32 g_maxChildNodes = 16;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

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

/*-------------------------------------------------------------------------*/

static bool
IsMailAddressText( const CORE::CString& address )
{GUCEF_TRACE;

    return IsPlainText( address, CMailIntroRequest::MaxAddressBytes, false ) && COM::CMailAddress( address ).IsValid();
}

/*-------------------------------------------------------------------------*/

static bool
IsNonce( const CORE::CString& nonce )
{GUCEF_TRACE;

    return MULTIPLAY::CMatchSigning::IsHexOfLength( nonce, CMailIntroRequest::NonceHexSize );
}

/*-------------------------------------------------------------------------*/

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

const char*
MailIntroStateToString( TMailIntroState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case MAILINTRO_NOT_ASKED:   return "NOT_ASKED";
        case MAILINTRO_WAITING:     return "WAITING";
        case MAILINTRO_INTRODUCED:  return "INTRODUCED";
        case MAILINTRO_DECLINED:    return "DECLINED";
        case MAILINTRO_EXPIRED:     return "EXPIRED";
        case MAILINTRO_KEY_CHANGED: return "KEY_CHANGED";
        case MAILINTRO_REFUSED:     return "REFUSED";
        default:                    return "NOT_ASKED";
    }
}

/*-------------------------------------------------------------------------*/

const char*
MailIntroResultToString( TMailIntroResult result )
{GUCEF_TRACE;

    switch ( result )
    {
        case MAILINTRORESULT_OK:                    return "OK";
        case MAILINTRORESULT_DUPLICATE:             return "DUPLICATE";
        case MAILINTRORESULT_CONFLICT:              return "CONFLICT";
        case MAILINTRORESULT_NO_DRAFT:              return "NO_DRAFT";
        case MAILINTRORESULT_NO_CONTACT:            return "NO_CONTACT";
        case MAILINTRORESULT_NOT_WAITING:           return "NOT_WAITING";
        case MAILINTRORESULT_EXPIRED:               return "EXPIRED";
        case MAILINTRORESULT_BAD_NONCE:             return "BAD_NONCE";
        case MAILINTRORESULT_BAD_ADDRESS:           return "BAD_ADDRESS";
        case MAILINTRORESULT_BAD_SIGNATURE:         return "BAD_SIGNATURE";
        case MAILINTRORESULT_BAD_STATEMENT:         return "BAD_STATEMENT";
        case MAILINTRORESULT_BAD_KEY:               return "BAD_KEY";
        case MAILINTRORESULT_KEY_CHANGED:           return "KEY_CHANGED";
        case MAILINTRORESULT_DECLINED:              return "DECLINED";
        case MAILINTRORESULT_BLOCKED:               return "BLOCKED";
        case MAILINTRORESULT_LIMIT:                 return "LIMIT";
        case MAILINTRORESULT_NOT_FOR_ME:            return "NOT_FOR_ME";
        case MAILINTRORESULT_KNOWN_MATCH:           return "KNOWN_MATCH";
        case MAILINTRORESULT_RATE_LIMITED:          return "RATE_LIMITED";
        case MAILINTRORESULT_RESEND_REPLY:          return "RESEND_REPLY";
        case MAILINTRORESULT_NO_REQUEST:            return "NO_REQUEST";
        case MAILINTRORESULT_NEEDS_CONFIRMATION:    return "NEEDS_CONFIRMATION";
        default:                                    return "BAD_STATEMENT";
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMailIntroFingerprint::Of( const CORE::CString& publicKeyHex )
{GUCEF_TRACE;

    if ( !MULTIPLAY::CMatchSigning::IsHexOfLength( publicKeyHex, MULTIPLAY::CMatchSigning::PublicKeyHexSize ) )
        return CORE::CString::Empty;
    UInt8 key[ MULTIPLAY::CPlayerIdentity::KeySize ];
    for ( UInt32 i=0; i<MULTIPLAY::CPlayerIdentity::KeySize; ++i )
    {
        UInt8 value = 0;
        for ( UInt32 n=0; n<2; ++n )
        {
            const char c = publicKeyHex[ i * 2 + n ];
            const UInt8 digit = (UInt8)( c >= '0' && c <= '9' ? c - '0' : ( c >= 'a' && c <= 'f' ? c - 'a' + 10 : c - 'A' + 10 ) );
            value = (UInt8)( ( value << 4 ) | digit );
        }
        key[ i ] = value;
    }
    UInt8 digest[ CORE::CSha256::DigestSize ];
    CORE::CSha256::Hash( key, sizeof( key ), digest );
    static const char digits[] = "0123456789ABCDEF";
    CORE::CString text;
    for ( UInt32 i=0; i<10; ++i )
    {
        if ( i > 0 && 0 == i % 2 )
            text += '-';
        text += digits[ digest[ i ] >> 4 ];
        text += digits[ digest[ i ] & 0x0F ];
    }
    return text;
}

/*-------------------------------------------------------------------------*/

CMailIntroParty::CMailIntroParty( void )
    : name()
    , address()
{GUCEF_TRACE;

}

CMailIntroParty::CMailIntroParty( const CORE::CString& partyName, const CORE::CString& partyAddress )
    : name( partyName )
    , address( partyAddress )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailIntroRequest::CMailIntroRequest( void )
    : CORE::CIDataNodeSerializable()
    , matchId()
    , nonce()
    , creatorUserId()
    , creatorPublicKey()
    , creatorName()
    , creatorAddress()
    , inviteeName()
    , inviteeAddress()
    , title()
    , parties()
    , settings()
    , createdAt( 0 )
    , expiresAt( 0 )
    , signature()
{GUCEF_TRACE;

}

CMailIntroRequest::CMailIntroRequest( const CMailIntroRequest& src )
    : CORE::CIDataNodeSerializable( src )
    , matchId( src.matchId )
    , nonce( src.nonce )
    , creatorUserId( src.creatorUserId )
    , creatorPublicKey( src.creatorPublicKey )
    , creatorName( src.creatorName )
    , creatorAddress( src.creatorAddress )
    , inviteeName( src.inviteeName )
    , inviteeAddress( src.inviteeAddress )
    , title( src.title )
    , parties( src.parties )
    , settings( src.settings )
    , createdAt( src.createdAt )
    , expiresAt( src.expiresAt )
    , signature( src.signature )
{GUCEF_TRACE;

}

CMailIntroRequest::~CMailIntroRequest()
{GUCEF_TRACE;

}

CMailIntroRequest&
CMailIntroRequest::operator=( const CMailIntroRequest& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        matchId = src.matchId;
        nonce = src.nonce;
        creatorUserId = src.creatorUserId;
        creatorPublicKey = src.creatorPublicKey;
        creatorName = src.creatorName;
        creatorAddress = src.creatorAddress;
        inviteeName = src.inviteeName;
        inviteeAddress = src.inviteeAddress;
        title = src.title;
        parties = src.parties;
        settings = src.settings;
        createdAt = src.createdAt;
        expiresAt = src.expiresAt;
        signature = src.signature;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CMailIntroRequest::GetSigningBytes( CORE::CDynamicBuffer& bytes ) const
{GUCEF_TRACE;

    bytes.Clear();
    MULTIPLAY::CMatchBinaryWriter writer( bytes );
    writer.WriteString( creatorUserId );
    writer.WriteString( creatorPublicKey );
    writer.WriteString( creatorName );
    writer.WriteString( creatorAddress );
    writer.WriteString( inviteeName );
    writer.WriteString( inviteeAddress );
    writer.WriteString( title );
    writer.WriteString( nonce );
    writer.WriteInt64( createdAt );
    writer.WriteInt64( expiresAt );
    writer.WriteUInt32( (UInt32) parties.size() );
    for ( size_t i=0; i<parties.size(); ++i )
    {
        writer.WriteString( parties[ i ].name );
        writer.WriteString( parties[ i ].address );
    }
    writer.WriteUInt32( (UInt32) settings.size() );
    for ( GUCEF::map< CORE::CString, CORE::CString >::const_iterator s = settings.begin(); s != settings.end(); ++s )
    {
        writer.WriteString( (*s).first );
        writer.WriteString( (*s).second );
    }
}

/*-------------------------------------------------------------------------*/

bool
CMailIntroRequest::IsWellFormed( CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !MULTIPLAY::CMatchId::IsValid( matchId ) )
        reason = "The match id is not valid";
    else
    if ( !IsNonce( nonce ) )
        reason = "The nonce is not valid";
    else
    if ( !MULTIPLAY::CMatchSigning::IsHexOfLength( creatorPublicKey, MULTIPLAY::CMatchSigning::PublicKeyHexSize ) || !IsPlainText( creatorUserId, g_maxUserIdBytes, false ) )
        reason = "The key of the creator is not valid";
    else
    if ( !IsPlainText( creatorName, MaxNameBytes, true ) || !IsPlainText( inviteeName, MaxNameBytes, true ) )
        reason = "A name is not valid";
    else
    if ( !IsPlainText( title, MaxTitleBytes, false ) )
        reason = "The title is not valid";
    else
    if ( !IsMailAddressText( creatorAddress ) || !IsMailAddressText( inviteeAddress ) )
        reason = "An address is not valid";
    else
    if ( parties.empty() || parties.size() > MaxParties )
        reason = "The number of players is not valid";
    else
    if ( settings.size() > MaxSettings )
        reason = "The game has too many settings";
    else
    if ( createdAt <= 0 || expiresAt <= createdAt || expiresAt - createdAt > g_maxValidityMs )
        reason = "The times are not valid";
    else
    if ( !MULTIPLAY::CMatchSigning::IsHexOfLength( signature, MULTIPLAY::CMatchSigning::SignatureHexSize ) && !signature.IsNULLOrEmpty() )
        reason = "The signature is not valid";
    else
    {
        for ( size_t i=0; i<parties.size(); ++i )
            if ( !IsPlainText( parties[ i ].name, MaxNameBytes, true ) || !IsMailAddressText( parties[ i ].address ) )
            {
                reason = "A player of the table is not valid";
                return false;
            }
        for ( GUCEF::map< CORE::CString, CORE::CString >::const_iterator s = settings.begin(); s != settings.end(); ++s )
            if ( !IsPlainText( (*s).first, MaxSettingKeyBytes, false ) || !IsPlainText( (*s).second, MaxSettingValueBytes, true ) )
            {
                reason = "A setting of the game is not valid";
                return false;
            }
        reason.Clear();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMailIntroRequest::Sign( const MULTIPLAY::CPlayerIdentity& identity )
{GUCEF_TRACE;

    if ( !identity.HasKeyPair() )
        return false;
    creatorUserId = identity.userId;
    creatorPublicKey = identity.publicKey.Lowercase();
    signature.Clear();
    CORE::CString reason;
    if ( !IsWellFormed( reason ) )
        return false;
    CORE::CDynamicBuffer bytes;
    GetSigningBytes( bytes );
    return MULTIPLAY::CMatchSigning::Sign( identity, SignedKind, matchId, bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature );
}

/*-------------------------------------------------------------------------*/

bool
CMailIntroRequest::Verify( CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !IsWellFormed( reason ) )
        return false;
    if ( !IsKeyOfUserId( creatorPublicKey, creatorUserId ) )
    {
        reason = "The user id is not the one of the key";
        return false;
    }
    CORE::CDynamicBuffer bytes;
    GetSigningBytes( bytes );
    if ( !MULTIPLAY::CMatchSigning::Verify( creatorPublicKey, SignedKind, matchId, bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature ) )
    {
        reason = "The signature is not valid";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailIntroRequest::Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "matchId", matchId );
    domRootNode.SetAttribute( "nonce", nonce );
    domRootNode.SetAttribute( "creatorUserId", creatorUserId );
    domRootNode.SetAttribute( "creatorPublicKey", creatorPublicKey );
    domRootNode.SetAttribute( "creatorName", creatorName );
    domRootNode.SetAttribute( "creatorAddress", creatorAddress );
    domRootNode.SetAttribute( "inviteeName", inviteeName );
    domRootNode.SetAttribute( "inviteeAddress", inviteeAddress );
    domRootNode.SetAttribute( "title", title );
    domRootNode.SetAttribute( "createdAt", createdAt );
    domRootNode.SetAttribute( "expiresAt", expiresAt );
    domRootNode.SetAttribute( "signature", signature );
    CORE::CDataNode* partiesNode = domRootNode.AddChild( "parties", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<parties.size(); ++i )
    {
        CORE::CDataNode* party = partiesNode->AddChild( "party", GUCEF_DATATYPE_OBJECT );
        party->SetAttribute( "name", parties[ i ].name );
        party->SetAttribute( "address", parties[ i ].address );
    }
    CORE::CDataNode* settingsNode = domRootNode.AddChild( "settings", GUCEF_DATATYPE_OBJECT );
    for ( GUCEF::map< CORE::CString, CORE::CString >::const_iterator s = this->settings.begin(); s != this->settings.end(); ++s )
        settingsNode->SetAttribute( (*s).first, (*s).second );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailIntroRequest::Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    matchId = domRootNode.GetAttributeValueOrChildValueByName( "matchId" ).AsString( CORE::CString::Empty, true );
    nonce = domRootNode.GetAttributeValueOrChildValueByName( "nonce" ).AsString( CORE::CString::Empty, true );
    creatorUserId = domRootNode.GetAttributeValueOrChildValueByName( "creatorUserId" ).AsString( CORE::CString::Empty, true );
    creatorPublicKey = domRootNode.GetAttributeValueOrChildValueByName( "creatorPublicKey" ).AsString( CORE::CString::Empty, true );
    creatorName = domRootNode.GetAttributeValueOrChildValueByName( "creatorName" ).AsString( CORE::CString::Empty, true );
    creatorAddress = domRootNode.GetAttributeValueOrChildValueByName( "creatorAddress" ).AsString( CORE::CString::Empty, true );
    inviteeName = domRootNode.GetAttributeValueOrChildValueByName( "inviteeName" ).AsString( CORE::CString::Empty, true );
    inviteeAddress = domRootNode.GetAttributeValueOrChildValueByName( "inviteeAddress" ).AsString( CORE::CString::Empty, true );
    title = domRootNode.GetAttributeValueOrChildValueByName( "title" ).AsString( CORE::CString::Empty, true );
    createdAt = domRootNode.GetAttributeValueOrChildValueByName( "createdAt" ).AsInt64( 0, true );
    expiresAt = domRootNode.GetAttributeValueOrChildValueByName( "expiresAt" ).AsInt64( 0, true );
    signature = domRootNode.GetAttributeValueOrChildValueByName( "signature" ).AsString( CORE::CString::Empty, true );
    parties.clear();
    const CORE::CDataNode* partiesNode = domRootNode.FindChild( "parties" );
    if ( GUCEF_NULL != partiesNode )
    {
        CORE::CDataNode::const_iterator i = partiesNode->ConstBegin();
        while ( i != partiesNode->ConstEnd() && parties.size() <= g_maxChildNodes )
        {
            CMailIntroParty party;
            party.name = (*i)->GetAttributeValueOrChildValueByName( "name" ).AsString( CORE::CString::Empty, true );
            party.address = (*i)->GetAttributeValueOrChildValueByName( "address" ).AsString( CORE::CString::Empty, true );
            parties.push_back( party );
            ++i;
        }
    }
    this->settings.clear();
    const CORE::CDataNode* settingsNode = domRootNode.FindChild( "settings" );
    if ( GUCEF_NULL != settingsNode )
    {
        // read at most one setting more than is allowed: the bounds check refuses the request
        CORE::CDataNode::TAttributeMap::const_iterator a = settingsNode->AttributeBegin();
        while ( a != settingsNode->AttributeEnd() && this->settings.size() <= MaxSettings )
        {
            this->settings[ (*a).first ] = (*a).second.AsString( CORE::CString::Empty, true );
            ++a;
        }
    }
    return MULTIPLAY::CMatchId::IsValid( matchId );
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMailIntroRequest::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMailIntroRequest( *this );
}

const CORE::CString&
CMailIntroRequest::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAYPLUGIN::MAIL::CMailIntroRequest";
    return classTypeName;
}

const char*
CMailIntroRequest::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

const char*
CMailIntroRequest::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAYPLUGIN::MAIL::CMailIntroRequest";
}

/*-------------------------------------------------------------------------*/

CMailIntroReply::CMailIntroReply( void )
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

CMailIntroReply::CMailIntroReply( const CMailIntroReply& src )
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

CMailIntroReply::~CMailIntroReply()
{GUCEF_TRACE;

}

CMailIntroReply&
CMailIntroReply::operator=( const CMailIntroReply& src )
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
CMailIntroReply::GetSigningBytes( CORE::CDynamicBuffer& bytes ) const
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
CMailIntroReply::Sign( const MULTIPLAY::CPlayerIdentity& identity )
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
CMailIntroReply::IsWellFormed( CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !MULTIPLAY::CMatchId::IsValid( matchId ) || !IsNonce( nonce ) )
        reason = "The match id or the nonce is not valid";
    else
    if ( !IsMailAddressText( inviteeAddress ) || !IsMailAddressText( creatorAddress ) )
        reason = "An address is not valid";
    else
    if ( !IsPlainText( displayName, CMailIntroRequest::MaxNameBytes, true ) || createdAt <= 0 )
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
CMailIntroReply::Verify( CORE::CString& reason ) const
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
CMailIntroReply::Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const
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

bool
CMailIntroReply::Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings )
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

CORE::CICloneable*
CMailIntroReply::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMailIntroReply( *this );
}

const CORE::CString&
CMailIntroReply::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAYPLUGIN::MAIL::CMailIntroReply";
    return classTypeName;
}

const char*
CMailIntroReply::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

const char*
CMailIntroReply::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAYPLUGIN::MAIL::CMailIntroReply";
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
