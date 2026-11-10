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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREPLY_H
#include "matchprovidermail_CEMailIntroReply.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREPLY_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREPLY_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROPARTY_H
#include "matchprovidermail_CEMailIntroParty.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROPARTY_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROPARTY_H ? */

#include "matchprovidermail_CEMailIntroRequest.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CEMailIntroRequest::SignedKind = "intro-request";
const char* CEMailIntroReply::SignedKind = "intro-reply";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailIntroRequest::CEMailIntroRequest( void )
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

/*-------------------------------------------------------------------------*/

CEMailIntroRequest::CEMailIntroRequest( const CEMailIntroRequest& src )
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

/*-------------------------------------------------------------------------*/

CEMailIntroRequest::~CEMailIntroRequest()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailIntroRequest&
CEMailIntroRequest::operator=( const CEMailIntroRequest& src )
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
CEMailIntroRequest::GetSigningBytes( CORE::CDynamicBuffer& bytes ) const
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
CEMailIntroRequest::IsWellFormed( CORE::CString& reason ) const
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
CEMailIntroRequest::Sign( const MULTIPLAY::CPlayerIdentity& identity )
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
CEMailIntroRequest::Verify( CORE::CString& reason ) const
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
CEMailIntroRequest::Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const
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
CEMailIntroRequest::Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings )
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
            CEMailIntroParty party;
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
CEMailIntroRequest::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CEMailIntroRequest( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEMailIntroRequest::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailIntroRequest";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CEMailIntroRequest::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CEMailIntroRequest::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailIntroRequest";
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
