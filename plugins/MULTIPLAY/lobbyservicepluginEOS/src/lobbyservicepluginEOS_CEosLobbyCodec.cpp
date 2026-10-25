/*
 *  lobbyservicepluginEOS: gucefMULTIPLAY lobby service plugin for lobbies on Epic Online Services
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

#include <string.h>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNWIREWRITER_H
#include "gucefDRN_CDRNWireWriter.h"
#define GUCEF_DRN_CDRNWIREWRITER_H
#endif /* GUCEF_DRN_CDRNWIREWRITER_H ? */

#ifndef GUCEF_DRN_CDRNWIREREADER_H
#include "gucefDRN_CDRNWireReader.h"
#define GUCEF_DRN_CDRNWIREREADER_H
#endif /* GUCEF_DRN_CDRNWIREREADER_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H
#include "gucefMULTIPLAY_CGucefLobbyProtocol.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H ? */

#include "lobbyservicepluginEOS_CEosLobbyCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace EOS {

using namespace PLUGINGLUE::EOS;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CEosLobbyCodec::KeyVersion = "v";
const CORE::CString CEosLobbyCodec::KeyStamp = "by";
const CORE::CString CEosLobbyCodec::KeyName = "nm";
const CORE::CString CEosLobbyCodec::KeyHostName = "hn";
const CORE::CString CEosLobbyCodec::KeyGameId = "gid";
const CORE::CString CEosLobbyCodec::KeyGameVersion = "gv";
const CORE::CString CEosLobbyCodec::KeyTopology = "topo";
const CORE::CString CEosLobbyCodec::KeyPassworded = "pw";
const CORE::CString CEosLobbyCodec::KeyPasswordSalt = "pws";
const CORE::CString CEosLobbyCodec::KeyAllowSpectators = "sp";
const CORE::CString CEosLobbyCodec::KeyMaxPlayers = "mx";
const CORE::CString CEosLobbyCodec::KeySpectatorSlots = "sps";
const CORE::CString CEosLobbyCodec::KeyPhase = "ph";
const CORE::CString CEosLobbyCodec::KeyPlayers = "pl";
const CORE::CString CEosLobbyCodec::KeySpectators = "sc";
const CORE::CString CEosLobbyCodec::KeyPublic = "pub";
const CORE::CString CEosLobbyCodec::KeyRevision = "rv";
const CORE::CString CEosLobbyCodec::MirrorPrefix = "s_";
const CORE::CString CEosLobbyCodec::BlobDefinition = "df";
const CORE::CString CEosLobbyCodec::BlobState = "st";
const CORE::CString CEosLobbyCodec::BlobLaunch = "la";

const CORE::CString CEosLobbyCodec::MemberKeyName = "n";
const CORE::CString CEosLobbyCodec::MemberKeyPasswordProof = "pwp";
const CORE::CString CEosLobbyCodec::MemberKeySpectator = "sp";
const CORE::CString CEosLobbyCodec::MemberKeyReady = "rdy";
const CORE::CString CEosLobbyCodec::MemberKeyRequest = "rq";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEosLobbyLimits::CEosLobbyLimits( void )
    : CORE::CIConfigurable()
    , maxLobbyMembers( EosDefaultMaxLobbyMembers )
    , maxLobbyAttributes( EosDefaultMaxLobbyAttributes )
    , maxAttributeLength( EosDefaultMaxAttributeLength )
    , maxAttributeKeyLength( EosDefaultMaxAttributeKeyLength )
    , maxSearchResults( EosDefaultMaxSearchResults )
    , maxBlobBytes( 65536 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosLobbyLimits::CEosLobbyLimits( const CEosLobbyLimits& src )
    : CORE::CIConfigurable( src )
    , maxLobbyMembers( src.maxLobbyMembers )
    , maxLobbyAttributes( src.maxLobbyAttributes )
    , maxAttributeLength( src.maxAttributeLength )
    , maxAttributeKeyLength( src.maxAttributeKeyLength )
    , maxSearchResults( src.maxSearchResults )
    , maxBlobBytes( src.maxBlobBytes )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosLobbyLimits::~CEosLobbyLimits()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosLobbyLimits&
CEosLobbyLimits::operator=( const CEosLobbyLimits& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        maxLobbyMembers = src.maxLobbyMembers;
        maxLobbyAttributes = src.maxLobbyAttributes;
        maxAttributeLength = src.maxAttributeLength;
        maxAttributeKeyLength = src.maxAttributeKeyLength;
        maxSearchResults = src.maxSearchResults;
        maxBlobBytes = src.maxBlobBytes;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyLimits::IsWellFormed( void ) const
{GUCEF_TRACE;

    // The fixed attributes and the pieces of one blob have to fit, and a piece has to be able to hold something
    return maxLobbyMembers >= 2 && maxLobbyAttributes >= 24 && maxAttributeLength >= 64 && maxAttributeKeyLength >= 8 &&
           maxSearchResults >= 1 && maxBlobBytes >= 1024;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyLimits::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "maxLobbyMembers", maxLobbyMembers );
    config.SetAttribute( "maxLobbyAttributes", maxLobbyAttributes );
    config.SetAttribute( "maxAttributeLength", maxAttributeLength );
    config.SetAttribute( "maxAttributeKeyLength", maxAttributeKeyLength );
    config.SetAttribute( "maxSearchResults", maxSearchResults );
    config.SetAttribute( "maxBlobBytes", maxBlobBytes );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyLimits::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CEosLobbyLimits loaded( *this );
    loaded.maxLobbyMembers = config.GetAttributeValueOrChildValueByName( "maxLobbyMembers" ).AsUInt32( maxLobbyMembers, true );
    loaded.maxLobbyAttributes = config.GetAttributeValueOrChildValueByName( "maxLobbyAttributes" ).AsUInt32( maxLobbyAttributes, true );
    loaded.maxAttributeLength = config.GetAttributeValueOrChildValueByName( "maxAttributeLength" ).AsUInt32( maxAttributeLength, true );
    loaded.maxAttributeKeyLength = config.GetAttributeValueOrChildValueByName( "maxAttributeKeyLength" ).AsUInt32( maxAttributeKeyLength, true );
    loaded.maxSearchResults = config.GetAttributeValueOrChildValueByName( "maxSearchResults" ).AsUInt32( maxSearchResults, true );
    loaded.maxBlobBytes = config.GetAttributeValueOrChildValueByName( "maxBlobBytes" ).AsUInt32( maxBlobBytes, true );
    if ( !loaded.IsWellFormed() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "EosLobbyLimits: The limits are too small to hold a lobby, they are not used" );
        return false;
    }
    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosLobbyLimits::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAYPLUGIN::EOS::CEosLobbyLimits";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CEosLobbySummary::CEosLobbySummary( void )
    : wireVersion( CEosLobbyCodec::WireVersion )
    , stamp()
    , name()
    , hostName()
    , gameId()
    , gameVersion()
    , topology( MULTIPLAY::GAMETOPOLOGY_CLIENT_SERVER )
    , passworded( false )
    , passwordSalt()
    , allowSpectators( true )
    , maxPlayers( 0 )
    , spectatorSlots( 0 )
    , phase( MULTIPLAY::LOBBYPHASE_OPEN )
    , players( 0 )
    , spectators( 0 )
    , isPublic( true )
    , revision( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

SEosAttribute
CEosLobbyCodec::MakeString( const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    SEosAttribute attribute;
    attribute.key = key;
    attribute.type = EOSATTRIBUTE_STRING;
    attribute.stringValue = value;
    attribute.visibility = EOSVISIBILITY_PUBLIC;
    return attribute;
}

/*-------------------------------------------------------------------------*/

SEosAttribute
CEosLobbyCodec::MakeInt( const CORE::CString& key, Int64 value )
{GUCEF_TRACE;

    SEosAttribute attribute;
    attribute.key = key;
    attribute.type = EOSATTRIBUTE_INT64;
    attribute.intValue = value;
    attribute.visibility = EOSVISIBILITY_PUBLIC;
    return attribute;
}

/*-------------------------------------------------------------------------*/

const SEosAttribute*
CEosLobbyCodec::FindAttribute( const TEosAttributeVector& attributes, const CORE::CString& key )
{GUCEF_TRACE;

    for ( size_t i=0; i<attributes.size(); ++i )
        if ( attributes[ i ].key == key )
            return &attributes[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyCodec::GetString( const TEosAttributeVector& attributes, const CORE::CString& key, CORE::CString& value )
{GUCEF_TRACE;

    const SEosAttribute* attribute = FindAttribute( attributes, key );
    if ( GUCEF_NULL == attribute || EOSATTRIBUTE_STRING != attribute->type )
        return false;
    value = attribute->stringValue;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyCodec::GetInt( const TEosAttributeVector& attributes, const CORE::CString& key, Int64& value )
{GUCEF_TRACE;

    const SEosAttribute* attribute = FindAttribute( attributes, key );
    if ( GUCEF_NULL == attribute || EOSATTRIBUTE_INT64 != attribute->type )
        return false;
    value = attribute->intValue;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyCodec::IsValidKey( const CORE::CString& key, const CEosLobbyLimits& limits )
{GUCEF_TRACE;

    UInt32 size = key.ByteSize();
    if ( size < 2 || size - 1 > limits.maxAttributeKeyLength )
        return false;
    const char* chars = key.C_String();
    for ( UInt32 i=0; i<size-1; ++i )
    {
        char c = chars[ i ];
        if ( !( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || '_' == c ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEosLobbyCodec::SanitizeText( const CORE::CString& text, UInt32 maxBytes )
{GUCEF_TRACE;

    // What another member sent is text only: no control characters and nothing that is not well formed UTF-8
    UInt32 size = text.ByteSize();
    if ( size < 2 )
        return CORE::CString::Empty;
    const unsigned char* bytes = reinterpret_cast< const unsigned char* >( text.C_String() );
    GUCEF::vector< char > clean;
    clean.reserve( size );
    UInt32 i = 0;
    while ( i < size - 1 && clean.size() < maxBytes )
    {
        unsigned char c = bytes[ i ];
        UInt32 length = 0;
        if ( c < 0x20 || 0x7f == c )
        {
            ++i;
            continue;
        }
        if ( c < 0x80 )
            length = 1;
        else
        if ( ( c & 0xE0 ) == 0xC0 && c >= 0xC2 )
            length = 2;
        else
        if ( ( c & 0xF0 ) == 0xE0 )
            length = 3;
        else
        if ( ( c & 0xF8 ) == 0xF0 && c <= 0xF4 )
            length = 4;
        else
        {
            ++i;
            continue;
        }
        bool valid = i + length <= size - 1;
        for ( UInt32 n=1; valid && n<length; ++n )
            valid = ( bytes[ i + n ] & 0xC0 ) == 0x80;
        if ( !valid )
        {
            ++i;
            continue;
        }
        // A character that would not fit whole is not cut in two
        if ( clean.size() + length > maxBytes )
            break;
        for ( UInt32 n=0; n<length; ++n )
            clean.push_back( (char) bytes[ i + n ] );
        i += length;
    }
    clean.push_back( '\0' );
    return CORE::CString( &clean[ 0 ] );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyCodec::IsValidLobbyId( const CORE::CString& lobbyId )
{GUCEF_TRACE;

    UInt32 size = lobbyId.ByteSize();
    if ( size < 2 || size > 65 )
        return false;
    const char* chars = lobbyId.C_String();
    for ( UInt32 i=0; i<size-1; ++i )
    {
        char c = chars[ i ];
        if ( !( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyCodec::IsValidUserId( const CORE::CString& userId )
{GUCEF_TRACE;

    UInt32 size = userId.ByteSize();
    if ( size < 2 || size > EosP2PUserIdBufferSize )
        return false;
    const char* chars = userId.C_String();
    for ( UInt32 i=0; i<size-1; ++i )
    {
        char c = chars[ i ];
        if ( !( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
HexOf( const UInt8* bytes, UInt32 size )
{GUCEF_TRACE;

    static const char digits[] = "0123456789abcdef";
    char text[ 129 ];
    UInt32 length = size > 64 ? 64 : size;
    for ( UInt32 i=0; i<length; ++i )
    {
        text[ i*2 ] = digits[ bytes[ i ] >> 4 ];
        text[ i*2 + 1 ] = digits[ bytes[ i ] & 0x0F ];
    }
    text[ length*2 ] = '\0';
    return CORE::CString( text );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEosLobbyCodec::MakePasswordProof( const CORE::CString& salt, const CORE::CString& userId, const CORE::CString& password )
{GUCEF_TRACE;

    static const char domain[] = "gucef-eos-lobby-password-v1";
    CORE::CSha256 sha;
    sha.Update( domain, (UInt32) sizeof( domain ) );
    sha.Update( salt.C_String(), salt.ByteSize() );
    sha.Update( userId.C_String(), userId.ByteSize() );
    sha.Update( password.C_String(), password.ByteSize() );
    UInt8 digest[ CORE::CSha256::DigestSize ];
    sha.Final( digest );
    return HexOf( digest, CORE::CSha256::DigestSize );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEosLobbyCodec::MakeSalt( void )
{GUCEF_TRACE;

    UInt8 bytes[ 16 ];
    if ( !DRN::CDRNCrypto::RandomBytes( bytes, sizeof( bytes ) ) )
        return CORE::CString::Empty;
    return HexOf( bytes, sizeof( bytes ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEosLobbyCodec::MakeSocketName( const CORE::CString& kind, const CORE::CString& lobbyId )
{GUCEF_TRACE;

    UInt8 digest[ CORE::CSha256::DigestSize ];
    CORE::CSha256::Hash( lobbyId.C_String(), lobbyId.ByteSize(), digest );
    return kind + HexOf( digest, 12 );
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyCodec::EncodeSummary( const CEosLobbySummary& summary, TEosAttributeVector& attributes )
{GUCEF_TRACE;

    attributes.push_back( MakeInt( KeyVersion, summary.wireVersion ) );
    attributes.push_back( MakeString( KeyStamp, summary.stamp ) );
    attributes.push_back( MakeString( KeyName, summary.name ) );
    attributes.push_back( MakeString( KeyHostName, summary.hostName ) );
    attributes.push_back( MakeString( KeyGameId, summary.gameId ) );
    attributes.push_back( MakeString( KeyGameVersion, summary.gameVersion ) );
    attributes.push_back( MakeInt( KeyTopology, (Int64) summary.topology ) );
    attributes.push_back( MakeInt( KeyPassworded, summary.passworded ? 1 : 0 ) );
    if ( summary.passworded )
        attributes.push_back( MakeString( KeyPasswordSalt, summary.passwordSalt ) );
    attributes.push_back( MakeInt( KeyAllowSpectators, summary.allowSpectators ? 1 : 0 ) );
    attributes.push_back( MakeInt( KeyMaxPlayers, summary.maxPlayers ) );
    attributes.push_back( MakeInt( KeySpectatorSlots, summary.spectatorSlots ) );
    attributes.push_back( MakeInt( KeyPhase, (Int64) summary.phase ) );
    attributes.push_back( MakeInt( KeyPlayers, summary.players ) );
    attributes.push_back( MakeInt( KeySpectators, summary.spectators ) );
    attributes.push_back( MakeInt( KeyPublic, summary.isPublic ? 1 : 0 ) );
    attributes.push_back( MakeInt( KeyRevision, summary.revision ) );
}

/*-------------------------------------------------------------------------*/

static bool
IsHexText( const CORE::CString& text, UInt32 length )
{GUCEF_TRACE;

    if ( text.ByteSize() != length + 1 )
        return false;
    const char* chars = text.C_String();
    for ( UInt32 i=0; i<length; ++i )
    {
        char c = chars[ i ];
        if ( !( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyCodec::DecodeSummary( const TEosAttributeVector& attributes, const CORE::CString& ownerPuid, CEosLobbySummary& summary, CORE::CString& reason )
{GUCEF_TRACE;

    CEosLobbySummary decoded;
    Int64 number = 0;
    CORE::CString text;

    if ( !GetInt( attributes, KeyVersion, number ) || number < 1 || number > WireVersion )
    {
        reason = "The lobby has no data this version knows";
        return false;
    }
    decoded.wireVersion = (UInt32) number;

    // Only the owner writes the lobby: the stamp of anything else is not the data of the lobby
    if ( !GetString( attributes, KeyStamp, decoded.stamp ) || decoded.stamp != ownerPuid || !IsValidUserId( decoded.stamp ) )
    {
        reason = "The data of the lobby was not written by its owner";
        return false;
    }

    if ( !GetString( attributes, KeyName, text ) || text.ByteSize() < 2 || text.ByteSize() > 121 )
    {
        reason = "The lobby has no usable name";
        return false;
    }
    decoded.name = SanitizeText( text, 120 );
    if ( decoded.name.IsNULLOrEmpty() )
    {
        reason = "The lobby has no usable name";
        return false;
    }
    if ( GetString( attributes, KeyHostName, text ) )
        decoded.hostName = SanitizeText( text, 64 );
    if ( GetString( attributes, KeyGameId, text ) )
        decoded.gameId = SanitizeText( text, 64 );
    if ( GetString( attributes, KeyGameVersion, text ) )
        decoded.gameVersion = SanitizeText( text, 64 );

    if ( !GetInt( attributes, KeyTopology, number ) || number < 0 || number > 1 )
    {
        reason = "The lobby names no known topology";
        return false;
    }
    decoded.topology = (MULTIPLAY::TGameTopology) number;

    if ( !GetInt( attributes, KeyPhase, number ) || number < 0 || number > MULTIPLAY::LOBBYPHASE_CLOSED )
    {
        reason = "The lobby names no known phase";
        return false;
    }
    decoded.phase = (MULTIPLAY::TLobbyPhase) number;

    decoded.passworded = GetInt( attributes, KeyPassworded, number ) && 0 != number;
    if ( decoded.passworded )
    {
        if ( !GetString( attributes, KeyPasswordSalt, decoded.passwordSalt ) || !IsHexText( decoded.passwordSalt, 32 ) )
        {
            reason = "The password of the lobby has no usable salt";
            return false;
        }
    }
    decoded.allowSpectators = GetInt( attributes, KeyAllowSpectators, number ) && 0 != number;
    decoded.isPublic = !GetInt( attributes, KeyPublic, number ) || 0 != number;

    if ( !GetInt( attributes, KeyMaxPlayers, number ) || number < 0 || number > 1024 )
    {
        reason = "The lobby names no usable number of players";
        return false;
    }
    decoded.maxPlayers = (UInt32) number;
    decoded.spectatorSlots = ( GetInt( attributes, KeySpectatorSlots, number ) && number >= 0 && number <= 1024 ) ? (UInt32) number : 0;
    decoded.players = ( GetInt( attributes, KeyPlayers, number ) && number >= 0 && number <= 1024 ) ? (UInt32) number : 0;
    decoded.spectators = ( GetInt( attributes, KeySpectators, number ) && number >= 0 && number <= 1024 ) ? (UInt32) number : 0;
    decoded.revision = ( GetInt( attributes, KeyRevision, number ) && number >= 0 && number <= 0x7FFFFFFF ) ? (UInt32) number : 0;

    summary = decoded;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CEosLobbyCodec::GetBlobAttributeCount( UInt32 textLength, const CEosLobbyLimits& limits )
{GUCEF_TRACE;

    UInt32 pieces = ( textLength + limits.maxAttributeLength - 1 ) / limits.maxAttributeLength;
    return pieces + 2;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyCodec::EncodeBlob( const CORE::CString& prefix, const CORE::CDataNode& tree, const CEosLobbyLimits& limits, TEosAttributeVector& attributes, CORE::CString& reason )
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer;
    DRN::CDRNWireWriter writer( buffer );
    writer.WriteUInt8( (UInt8) WireVersion );
    MULTIPLAY::CGucefLobbyProtocol::WriteDataTree( writer, tree );
    UInt32 size = writer.GetOffset();
    if ( size > limits.maxBlobBytes )
    {
        reason = "The data of " + prefix + " is " + CORE::UInt32ToString( size ) + " bytes, more than the " + CORE::UInt32ToString( limits.maxBlobBytes ) + " that a lobby holds";
        return false;
    }

    CORE::CString text = CORE::Base64Encode( buffer.GetConstBufferPtr(), size );
    UInt32 length = text.ByteSize() - 1;
    UInt32 pieces = ( length + limits.maxAttributeLength - 1 ) / limits.maxAttributeLength;
    if ( 0 == pieces )
        pieces = 1;
    if ( pieces + 2 > limits.maxLobbyAttributes )
    {
        reason = "The data of " + prefix + " needs " + CORE::UInt32ToString( pieces + 2 ) + " attributes, a lobby has " + CORE::UInt32ToString( limits.maxLobbyAttributes );
        return false;
    }
    if ( !IsValidKey( prefix + "_" + CORE::UInt32ToString( pieces ), limits ) )
    {
        reason = "The key of the data of " + prefix + " is too long";
        return false;
    }

    UInt8 digest[ CORE::CSha256::DigestSize ];
    CORE::CSha256::Hash( text.C_String(), length, digest );
    attributes.push_back( MakeInt( prefix + "_n", pieces ) );
    attributes.push_back( MakeString( prefix + "_h", HexOf( digest, 8 ) ) );
    for ( UInt32 i=0; i<pieces; ++i )
    {
        UInt32 start = i * limits.maxAttributeLength;
        UInt32 end = start + limits.maxAttributeLength;
        if ( end > length )
            end = length;
        attributes.push_back( MakeString( prefix + "_" + CORE::UInt32ToString( i ), text.SubstrFromRange( start, end ) ) );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyCodec::DecodeBlob( const CORE::CString& prefix, const TEosAttributeVector& attributes, const CEosLobbyLimits& limits, CORE::CDataNode& tree, CORE::CString& reason )
{GUCEF_TRACE;

    Int64 pieces = 0;
    CORE::CString hash;
    if ( !GetInt( attributes, prefix + "_n", pieces ) || !GetString( attributes, prefix + "_h", hash ) )
    {
        reason = "The data of " + prefix + " is not there";
        return false;
    }
    if ( pieces < 1 || pieces > (Int64) limits.maxLobbyAttributes )
    {
        reason = "The data of " + prefix + " has an impossible number of pieces";
        return false;
    }

    // Everything that follows is data of another member: its size is bounded before anything is made of it
    UInt32 maxText = limits.maxBlobBytes / 3 * 4 + 8;
    CORE::CString text;
    for ( Int64 i=0; i<pieces; ++i )
    {
        CORE::CString piece;
        if ( !GetString( attributes, prefix + "_" + CORE::Int64ToString( i ), piece ) )
        {
            reason = "A piece of the data of " + prefix + " is missing";
            return false;
        }
        if ( piece.ByteSize() - 1 > limits.maxAttributeLength || text.ByteSize() + piece.ByteSize() > maxText + 2 )
        {
            reason = "The data of " + prefix + " is larger than a lobby holds";
            return false;
        }
        text += piece;
    }

    UInt32 length = text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
    UInt8 digest[ CORE::CSha256::DigestSize ];
    CORE::CSha256::Hash( text.C_String(), length, digest );
    if ( HexOf( digest, 8 ).Lowercase() != hash.Lowercase() )
    {
        reason = "The pieces of the data of " + prefix + " do not belong together";
        return false;
    }

    CORE::CDynamicBuffer decoded;
    decoded.SetBufferSize( length + 8 );
    UInt32 used = 0;
    if ( !CORE::Base64Decode( text, decoded.GetBufferPtr(), length + 8, used ) || used < 2 )
    {
        reason = "The data of " + prefix + " is not base64";
        return false;
    }
    if ( used > limits.maxBlobBytes )
    {
        reason = "The data of " + prefix + " is larger than a lobby holds";
        return false;
    }

    DRN::CDRNWireReader reader( decoded.GetConstBufferPtr(), used );
    UInt8 version = 0;
    if ( !reader.ReadUInt8( version ) || version < 1 || version > WireVersion )
    {
        reason = "The data of " + prefix + " has a version this does not know";
        return false;
    }
    CORE::CDataNode read;
    if ( !MULTIPLAY::CGucefLobbyProtocol::ReadDataTree( reader, read ) || 0 != reader.GetRemaining() )
    {
        reason = "The data of " + prefix + " is not well formed";
        return false;
    }
    tree = read;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyCodec::GetBlobKeys( const CORE::CString& prefix, const TEosAttributeVector& attributes, CORE::CString::StringVector& keys )
{GUCEF_TRACE;

    CORE::CString start = prefix + "_";
    for ( size_t i=0; i<attributes.size(); ++i )
        if ( 0 == attributes[ i ].key.HasSubstr( start, true ) )
            keys.push_back( attributes[ i ].key );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyCodec::ParseRequest( const CORE::CString& text, UInt32& sequence, CORE::CString& verb, CORE::CString& arg1, CORE::CString& arg2 )
{GUCEF_TRACE;

    if ( text.ByteSize() < 4 || text.ByteSize() > 700 )
        return false;
    CORE::CString::StringVector parts = text.ParseElements( '|', true );
    if ( parts.size() < 2 || parts.size() > 4 )
        return false;
    if ( parts[ 0 ].IsNULLOrEmpty() || parts[ 0 ].ByteSize() > 11 )
        return false;
    const char* digits = parts[ 0 ].C_String();
    UInt64 value = 0;
    for ( UInt32 i=0; i<parts[ 0 ].ByteSize()-1; ++i )
    {
        if ( digits[ i ] < '0' || digits[ i ] > '9' )
            return false;
        value = value * 10 + (UInt32) ( digits[ i ] - '0' );
    }
    if ( value > 0xFFFFFFFFu )
        return false;
    sequence = (UInt32) value;
    verb = SanitizeText( parts[ 1 ], 32 );
    if ( verb.IsNULLOrEmpty() )
        return false;
    arg1 = parts.size() > 2 ? SanitizeText( parts[ 2 ], 256 ) : CORE::CString::Empty;
    arg2 = parts.size() > 3 ? SanitizeText( parts[ 3 ], 256 ) : CORE::CString::Empty;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEosLobbyCodec::MakeRequest( UInt32 sequence, const CORE::CString& verb, const CORE::CString& arg1, const CORE::CString& arg2 )
{GUCEF_TRACE;

    // The separator is not part of an argument
    return CORE::UInt32ToString( sequence ) + "|" + verb + "|" + arg1.ReplaceChar( '|', ' ' ) + "|" + arg2.ReplaceChar( '|', ' ' );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyCodec::ParseReady( const CORE::CString& text, UInt32& sequence, bool& ready )
{GUCEF_TRACE;

    UInt32 size = text.ByteSize();
    if ( size < 4 || size > 14 )
        return false;
    const char* chars = text.C_String();
    UInt32 colon = 0;
    UInt64 value = 0;
    for ( ; colon<size-1 && ':' != chars[ colon ]; ++colon )
    {
        if ( chars[ colon ] < '0' || chars[ colon ] > '9' )
            return false;
        value = value * 10 + (UInt32) ( chars[ colon ] - '0' );
    }
    if ( 0 == colon || colon + 2 != size - 1 || value > 0xFFFFFFFFu )
        return false;
    char flag = chars[ colon + 1 ];
    if ( '0' != flag && '1' != flag )
        return false;
    sequence = (UInt32) value;
    ready = '1' == flag;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyCodec::MakeListing( const SEosLobbyDetails& details, const CEosLobbyLimits& limits, MULTIPLAY::CGameListingEntry& listing, CORE::CString& reason )
{GUCEF_TRACE;

    if ( !IsValidLobbyId( details.lobbyId ) || !IsValidUserId( details.ownerPuid ) )
    {
        reason = "The lobby has no usable id or owner";
        return false;
    }
    CEosLobbySummary summary;
    if ( !DecodeSummary( details.attributes, details.ownerPuid, summary, reason ) )
        return false;

    MULTIPLAY::CGameListingEntry entry;
    entry.listingId = details.lobbyId;
    entry.kind = MULTIPLAY::LOBBYPHASE_IN_GAME == summary.phase ? MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS : MULTIPLAY::CGameListingEntry::KIND_LOBBY;
    entry.name = summary.name;
    entry.gameId = summary.gameId;
    entry.gameVersion = summary.gameVersion;
    entry.maxPlayers = summary.maxPlayers;
    entry.players = summary.players > summary.maxPlayers ? summary.maxPlayers : summary.players;
    entry.spectators = summary.spectators;
    entry.passworded = summary.passworded;
    entry.hostName = summary.hostName;
    entry.endpoints.push_back( "eos-p2p://" + details.ownerPuid + "/" + MakeSocketName( "gg", details.lobbyId ) );
    entry.backendName = "eos";
    entry.pingMs = -1;
    entry.lastSeen = CORE::CDateTime::NowUTCDateTime();
    entry.extra.SetAllowMultipleValues( false );
    entry.extra.Set( "lobbyService", CORE::CString( "eos" ) );
    entry.extra.Set( "ownerPuid", details.ownerPuid );
    entry.extra.Set( "bucket", details.bucketId );
    entry.extra.Set( "revision", summary.revision );
    entry.extra.Set( "socket", MakeSocketName( "gg", details.lobbyId ) );

    // A summary of the settings, from the state when it holds up: the map and the mode are the ones the game names
    CORE::CDataNode stateTree;
    CORE::CString stateReason;
    if ( DecodeBlob( BlobState, details.attributes, limits, stateTree, stateReason ) )
    {
        const CORE::CDataNode* stateNode = stateTree.FindChild( "state" );
        MULTIPLAY::CLobbyState state;
        if ( GUCEF_NULL != stateNode && state.Deserialize( *stateNode, CORE::CDataNodeSerializableSettings() ) )
        {
            entry.mapName = SanitizeText( state.settings.GetValueAlways( "map" ).AsString( CORE::CString::Empty, true ), 64 );
            entry.modeName = SanitizeText( state.settings.GetValueAlways( "mode" ).AsString( CORE::CString::Empty, true ), 64 );
            UInt32 count = state.settings.GetKeyCount();
            for ( UInt32 i=0; i<count && i<16; ++i )
                entry.extra.Set( "set." + SanitizeText( state.settings.GetKey( i ).AsString(), 32 ), SanitizeText( state.settings.GetValue( i ).AsString(), 64 ) );
        }
    }
    listing = entry;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
