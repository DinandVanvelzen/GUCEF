/*
 *  gucefMULTIPLAY_EosTestApp: Application for testing the Epic Online Services plugins of gucefMULTIPLAY, gucefDRN and gucefAUTH
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#include "TestEosCommon.h"
#include "TestEosCodec.h"

using namespace GUCEF;
using namespace GUCEF::PLUGINGLUE::EOS;
using namespace GUCEF::MULTIPLAYPLUGIN::EOS;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* OwnerId = "00112233445566778899aabbccdd10";

static CORE::CString
Repeat( char c, UInt32 count )
{
    GUCEF::vector< char > chars( count + 1, c );
    chars[ count ] = '\0';
    return CORE::CString( &chars[ 0 ] );
}

static UInt32
ChildCount( const CORE::CDataNode& node )
{
    return node.GetNrOfChildNodes();     // the trees of these tests have no grandchildren
}

/** A tree with the given number of children, each with a text attribute of the given length */
static void
MakeTree( CORE::CDataNode& tree, UInt32 children, UInt32 textLength )
{
    for ( UInt32 i=0; i<children; ++i )
    {
        CORE::CDataNode* child = tree.AddChild( "c" + CORE::UInt32ToString( i ), GUCEF_DATATYPE_OBJECT );
        CORE::CString text;
        for ( UInt32 n=0; n<textLength; ++n )
            text += (char) ( 'a' + ( ( i + n ) % 26 ) );
        child->SetAttribute( "t", text );
        child->SetAttribute( "n", i );
    }
}

/** The first 8 bytes of the SHA-256 of a text as hex, the way a blob states its hash */
static CORE::CString
HashOf( const CORE::CString& text )
{
    UInt8 digest[ CORE::CSha256::DigestSize ];
    CORE::CSha256::Hash( text.C_String(), text.ByteSize() - 1, digest );
    static const char* hex = "0123456789abcdef";
    char out[ 17 ];
    for ( UInt32 i=0; i<8; ++i )
    {
        out[ i * 2 ] = hex[ digest[ i ] >> 4 ];
        out[ i * 2 + 1 ] = hex[ digest[ i ] & 0xF ];
    }
    out[ 16 ] = '\0';
    return CORE::CString( out );
}

static SEosAttribute*
FindMutable( TEosAttributeVector& attributes, const CORE::CString& key )
{
    for ( size_t i=0; i<attributes.size(); ++i )
        if ( attributes[ i ].key == key )
            return &attributes[ i ];
    return GUCEF_NULL;
}

static void
RemoveKey( TEosAttributeVector& attributes, const CORE::CString& key )
{
    for ( size_t i=0; i<attributes.size(); ++i )
    {
        if ( attributes[ i ].key == key )
        {
            attributes.erase( attributes.begin() + i );
            return;
        }
    }
}

static CEosLobbySummary
MakeSummary( void )
{
    CEosLobbySummary summary;
    summary.stamp = OwnerId;
    summary.name = "Lobby of the tests";
    summary.hostName = "Hosty";
    summary.gameId = "testgame";
    summary.gameVersion = "1.2.3";
    summary.topology = MULTIPLAY::GAMETOPOLOGY_PEER_TO_PEER;
    summary.passworded = true;
    summary.passwordSalt = "00112233445566778899aabbccddeeff";
    summary.allowSpectators = true;
    summary.maxPlayers = 6;
    summary.spectatorSlots = 4;
    summary.phase = MULTIPLAY::LOBBYPHASE_OPEN;
    summary.players = 2;
    summary.spectators = 1;
    summary.isPublic = true;
    summary.revision = 41;
    return summary;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformEosCodecTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING EOS Codec TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "EosCodec" );

    GUCEF_TESTFW_TESTCASE( "Test: Data that fits in one attribute is one blob of three attributes and comes back the same" )
        try
        {
            CEosLobbyLimits limits;
            CORE::CDataNode tree( "st" );
            MakeTree( tree, 2, 10 );
            TEosAttributeVector attributes;
            CORE::CString reason;
            ASSERT_TRUE( CEosLobbyCodec::EncodeBlob( "st", tree, limits, attributes, reason ) );
            ASSERT_TRUE( 3 == attributes.size() );
            ASSERT_TRUE( "st_n" == attributes[ 0 ].key && EOSATTRIBUTE_INT64 == attributes[ 0 ].type && 1 == attributes[ 0 ].intValue );
            ASSERT_TRUE( "st_h" == attributes[ 1 ].key && 17 == attributes[ 1 ].stringValue.ByteSize() );
            ASSERT_TRUE( "st_0" == attributes[ 2 ].key );

            CORE::CDataNode read;
            ASSERT_TRUE( CEosLobbyCodec::DecodeBlob( "st", attributes, limits, read, reason ) );
            ASSERT_TRUE( 2 == ChildCount( read ) );
            const CORE::CDataNode* second = read.FindChild( "c1" );
            ASSERT_TRUE( GUCEF_NULL != second && second->GetAttributeValueOrChildValueByName( "n" ).AsUInt32( 99 ) == 1 );
            ASSERT_TRUE( second->GetAttributeValueOrChildValueByName( "t" ).AsString() == "bcdefghijk" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Larger data is cut in pieces that each fit an attribute, and is put back together whatever the order of the attributes" )
        try
        {
            CEosLobbyLimits limits;
            limits.maxAttributeLength = 100;
            CORE::CDataNode tree( "st" );
            MakeTree( tree, 20, 40 );
            TEosAttributeVector attributes;
            CORE::CString reason;
            ASSERT_TRUE( CEosLobbyCodec::EncodeBlob( "st", tree, limits, attributes, reason ) );
            ASSERT_TRUE( attributes.size() > 10 );
            for ( size_t i=0; i<attributes.size(); ++i )
            {
                ASSERT_TRUE( CEosLobbyCodec::IsValidKey( attributes[ i ].key, limits ) );
                if ( EOSATTRIBUTE_STRING == attributes[ i ].type )
                    ASSERT_TRUE( attributes[ i ].stringValue.ByteSize() - 1 <= 100 );
            }
            CORE::CString::StringVector keys;
            CEosLobbyCodec::GetBlobKeys( "st", attributes, keys );
            ASSERT_TRUE( keys.size() == attributes.size() );
            ASSERT_TRUE( attributes.size() == CEosLobbyCodec::GetBlobAttributeCount( ( (UInt32) attributes.size() - 2 ) * 100, limits ) || attributes.size() <= CEosLobbyCodec::GetBlobAttributeCount( ( (UInt32) attributes.size() - 2 ) * 100, limits ) );

            // the SDK says nothing of the order of attributes
            TEosAttributeVector reversed;
            for ( size_t i=attributes.size(); i>0; --i )
                reversed.push_back( attributes[ i - 1 ] );
            CORE::CDataNode read;
            ASSERT_TRUE( CEosLobbyCodec::DecodeBlob( "st", reversed, limits, read, reason ) );
            ASSERT_TRUE( 20 == ChildCount( read ) );
            ASSERT_TRUE( read.FindChild( "c19" )->GetAttributeValueOrChildValueByName( "n" ).AsUInt32( 0 ) == 19 );

            // another blob next to it is left alone
            TEosAttributeVector both = attributes;
            CORE::CDataNode other( "la" );
            MakeTree( other, 2, 5 );
            ASSERT_TRUE( CEosLobbyCodec::EncodeBlob( "la", other, limits, both, reason ) );
            CORE::CString::StringVector stKeys;
            CORE::CString::StringVector laKeys;
            CEosLobbyCodec::GetBlobKeys( "st", both, stKeys );
            CEosLobbyCodec::GetBlobKeys( "la", both, laKeys );
            ASSERT_TRUE( stKeys.size() == attributes.size() && 3 == laKeys.size() );
            CORE::CDataNode readOther;
            ASSERT_TRUE( CEosLobbyCodec::DecodeBlob( "la", both, limits, readOther, reason ) && 2 == ChildCount( readOther ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Data that does not fit is refused with the reason: more bytes than a lobby holds, more attributes than a lobby has" )
        try
        {
            CEosLobbyLimits limits;
            CORE::CString reason;
            TEosAttributeVector attributes;

            CORE::CDataNode huge( "st" );
            MakeTree( huge, 100, 1000 );
            ASSERT_FALSE( CEosLobbyCodec::EncodeBlob( "st", huge, limits, attributes, reason ) );
            ASSERT_TRUE( reason.HasSubstr( "more than" ) >= 0 && attributes.empty() );

            CEosLobbyLimits fewAttributes;
            fewAttributes.maxLobbyAttributes = 6;
            fewAttributes.maxAttributeLength = 50;
            CORE::CDataNode tree( "st" );
            MakeTree( tree, 10, 30 );
            reason.Clear();
            ASSERT_FALSE( CEosLobbyCodec::EncodeBlob( "st", tree, fewAttributes, attributes, reason ) );
            ASSERT_TRUE( reason.HasSubstr( "attributes" ) >= 0 && attributes.empty() );

            // and exactly at the limit it does
            CEosLobbyLimits tight;
            tight.maxAttributeLength = 50;
            CORE::CDataNode small( "st" );
            MakeTree( small, 1, 5 );
            ASSERT_TRUE( CEosLobbyCodec::EncodeBlob( "st", small, tight, attributes, reason ) );
            tight.maxLobbyAttributes = (UInt32) attributes.size();
            attributes.clear();
            ASSERT_TRUE( CEosLobbyCodec::EncodeBlob( "st", small, tight, attributes, reason ) );
            tight.maxLobbyAttributes = (UInt32) attributes.size() - 1;
            attributes.clear();
            ASSERT_FALSE( CEosLobbyCodec::EncodeBlob( "st", small, tight, attributes, reason ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Data of a blob that was damaged or forged is not read: a missing piece, changed text, a wrong count, a hash that does not fit" )
        try
        {
            CEosLobbyLimits limits;
            limits.maxAttributeLength = 100;
            CORE::CDataNode tree( "st" );
            MakeTree( tree, 10, 40 );
            TEosAttributeVector good;
            CORE::CString reason;
            REQUIRE_TRUE( CEosLobbyCodec::EncodeBlob( "st", tree, limits, good, reason ) );
            CORE::CDataNode read;

            // nothing there
            TEosAttributeVector empty;
            ASSERT_FALSE( CEosLobbyCodec::DecodeBlob( "st", empty, limits, read, reason ) );

            // a piece is missing
            TEosAttributeVector missing = good;
            RemoveKey( missing, "st_1" );
            ASSERT_FALSE( CEosLobbyCodec::DecodeBlob( "st", missing, limits, read, reason ) && reason.IsNULLOrEmpty() );
            reason.Clear();
            ASSERT_FALSE( CEosLobbyCodec::DecodeBlob( "st", missing, limits, read, reason ) );
            ASSERT_TRUE( reason.HasSubstr( "missing" ) >= 0 );

            // a piece was changed
            TEosAttributeVector changed = good;
            SEosAttribute* piece = FindMutable( changed, "st_0" );
            piece->stringValue = "A" + piece->stringValue.SubstrToIndex( piece->stringValue.Length() - 1, true );
            reason.Clear();
            ASSERT_FALSE( CEosLobbyCodec::DecodeBlob( "st", changed, limits, read, reason ) );
            ASSERT_TRUE( reason.HasSubstr( "do not belong together" ) >= 0 );

            // the count lies: fewer pieces than there are, none, negative, and more than a lobby has attributes
            Int64 counts[] = { 1, 0, -3, 1000000 };
            for ( UInt32 i=0; i<4; ++i )
            {
                TEosAttributeVector lie = good;
                FindMutable( lie, "st_n" )->intValue = counts[ i ];
                reason.Clear();
                ASSERT_FALSE( CEosLobbyCodec::DecodeBlob( "st", lie, limits, read, reason ) );
                ASSERT_FALSE( reason.IsNULLOrEmpty() );
            }

            // the hash is not the one of the text
            TEosAttributeVector wrongHash = good;
            FindMutable( wrongHash, "st_h" )->stringValue = "0000000000000000";
            ASSERT_FALSE( CEosLobbyCodec::DecodeBlob( "st", wrongHash, limits, read, reason ) );

            // a piece that is longer than an attribute can be
            TEosAttributeVector longPiece = good;
            FindMutable( longPiece, "st_0" )->stringValue = Repeat( 'A', 500 );
            ASSERT_FALSE( CEosLobbyCodec::DecodeBlob( "st", longPiece, limits, read, reason ) );

            // text with the hash of itself that is no data: not base64, base64 of nothing useful, and a version that is not known
            const char* texts[] = { "!!!!not base64 at all!!!!", "AAAA", "AwAAAA==", "////" };
            for ( UInt32 i=0; i<4; ++i )
            {
                TEosAttributeVector forged;
                CORE::CString text( texts[ i ] );
                forged.push_back( CEosLobbyCodec::MakeInt( "st_n", 1 ) );
                forged.push_back( CEosLobbyCodec::MakeString( "st_h", HashOf( text ) ) );
                forged.push_back( CEosLobbyCodec::MakeString( "st_0", text ) );
                reason.Clear();
                ASSERT_FALSE( CEosLobbyCodec::DecodeBlob( "st", forged, limits, read, reason ) );
                ASSERT_FALSE( reason.IsNULLOrEmpty() );
            }

            // and the good one still reads
            ASSERT_TRUE( CEosLobbyCodec::DecodeBlob( "st", good, limits, read, reason ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A blob from another member that is larger than a lobby holds is refused before it is decoded" )
        try
        {
            CEosLobbyLimits writer;
            writer.maxBlobBytes = 2000000;
            writer.maxLobbyAttributes = 4096;
            writer.maxAttributeLength = 1000;
            CORE::CDataNode tree( "st" );
            MakeTree( tree, 200, 900 );
            TEosAttributeVector attributes;
            CORE::CString reason;
            REQUIRE_TRUE( CEosLobbyCodec::EncodeBlob( "st", tree, writer, attributes, reason ) );

            CEosLobbyLimits reader;
            reader.maxBlobBytes = 65536;
            reader.maxLobbyAttributes = 4096;
            CORE::CDataNode read;
            ASSERT_FALSE( CEosLobbyCodec::DecodeBlob( "st", attributes, reader, read, reason ) );
            ASSERT_TRUE( reason.HasSubstr( "larger" ) >= 0 );
            ASSERT_TRUE( 0 == ChildCount( read ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The summary of a lobby is written and read back whole, and the limits of the data are kept to" )
        try
        {
            CEosLobbySummary summary = MakeSummary();
            TEosAttributeVector attributes;
            CEosLobbyCodec::EncodeSummary( summary, attributes );
            CEosLobbySummary read;
            CORE::CString reason;
            ASSERT_TRUE( CEosLobbyCodec::DecodeSummary( attributes, OwnerId, read, reason ) );
            ASSERT_TRUE( read.stamp == OwnerId && read.name == summary.name && read.hostName == "Hosty" && read.gameId == "testgame" && read.gameVersion == "1.2.3" );
            ASSERT_TRUE( MULTIPLAY::GAMETOPOLOGY_PEER_TO_PEER == read.topology && read.passworded && read.passwordSalt == summary.passwordSalt );
            ASSERT_TRUE( read.allowSpectators && 6 == read.maxPlayers && 4 == read.spectatorSlots && 2 == read.players && 1 == read.spectators && read.isPublic && 41 == read.revision );
            ASSERT_TRUE( CEosLobbyCodec::WireVersion == read.wireVersion && MULTIPLAY::LOBBYPHASE_OPEN == read.phase );

            // written by anybody but the owner
            ASSERT_FALSE( CEosLobbyCodec::DecodeSummary( attributes, "00112233445566778899aabbccdd11", read, reason ) );
            ASSERT_TRUE( reason.HasSubstr( "owner" ) >= 0 );

            // a version this does not know, none at all
            TEosAttributeVector version2 = attributes;
            FindMutable( version2, CEosLobbyCodec::KeyVersion )->intValue = 2;
            ASSERT_FALSE( CEosLobbyCodec::DecodeSummary( version2, OwnerId, read, reason ) );
            TEosAttributeVector noVersion = attributes;
            RemoveKey( noVersion, CEosLobbyCodec::KeyVersion );
            ASSERT_FALSE( CEosLobbyCodec::DecodeSummary( noVersion, OwnerId, read, reason ) );

            // numbers out of their bounds
            Int64 bad[] = { -1, 2000, 99 };
            const CORE::CString* keys[] = { &CEosLobbyCodec::KeyMaxPlayers, &CEosLobbyCodec::KeyMaxPlayers, &CEosLobbyCodec::KeyPhase };
            for ( UInt32 i=0; i<3; ++i )
            {
                TEosAttributeVector forged = attributes;
                FindMutable( forged, *keys[ i ] )->intValue = bad[ i ];
                ASSERT_FALSE( CEosLobbyCodec::DecodeSummary( forged, OwnerId, read, reason ) );
            }
            TEosAttributeVector topology = attributes;
            FindMutable( topology, CEosLobbyCodec::KeyTopology )->intValue = 7;
            ASSERT_FALSE( CEosLobbyCodec::DecodeSummary( topology, OwnerId, read, reason ) );

            // a password without a usable salt, a name that is not usable
            TEosAttributeVector salt = attributes;
            FindMutable( salt, CEosLobbyCodec::KeyPasswordSalt )->stringValue = "not hex";
            ASSERT_FALSE( CEosLobbyCodec::DecodeSummary( salt, OwnerId, read, reason ) );
            TEosAttributeVector name = attributes;
            FindMutable( name, CEosLobbyCodec::KeyName )->stringValue = Repeat( 'x', 400 );
            ASSERT_FALSE( CEosLobbyCodec::DecodeSummary( name, OwnerId, read, reason ) );
            FindMutable( name, CEosLobbyCodec::KeyName )->stringValue = "\x01\x02\x03";
            ASSERT_FALSE( CEosLobbyCodec::DecodeSummary( name, OwnerId, read, reason ) );

            // what is not essential is made safe: control characters leave a name
            TEosAttributeVector host = attributes;
            FindMutable( host, CEosLobbyCodec::KeyHostName )->stringValue = "Ho\x07sty\n";
            ASSERT_TRUE( CEosLobbyCodec::DecodeSummary( host, OwnerId, read, reason ) && read.hostName == "Hosty" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Text from another member is cleaned: control characters out, broken UTF-8 out, cut on a character boundary" )
        try
        {
            ASSERT_TRUE( CEosLobbyCodec::SanitizeText( "plain", 64 ) == "plain" );
            ASSERT_TRUE( CEosLobbyCodec::SanitizeText( "a\tb\nc\x01" "d\x7f" "e", 64 ) == "abcde" );
            ASSERT_TRUE( CEosLobbyCodec::SanitizeText( "", 64 ).IsNULLOrEmpty() );
            ASSERT_TRUE( CEosLobbyCodec::SanitizeText( "abcdefgh", 4 ) == "abcd" );

            // a valid two byte character stays, a lone continuation byte and an overlong one go
            ASSERT_TRUE( CEosLobbyCodec::SanitizeText( "caf\xC3\xA9", 64 ) == "caf\xC3\xA9" );
            ASSERT_TRUE( CEosLobbyCodec::SanitizeText( "a\x80" "b", 64 ) == "ab" );
            ASSERT_TRUE( CEosLobbyCodec::SanitizeText( "a\xC0\xAF" "b", 64 ).HasSubstr( "\xC0" ) < 0 );
            ASSERT_TRUE( CEosLobbyCodec::SanitizeText( "a\xF8\x88\x80\x80\x80" "b", 64 ).HasSubstr( "\xF8" ) < 0 );

            // a character that does not fit whole is left out, not cut in two
            ASSERT_TRUE( CEosLobbyCodec::SanitizeText( "ab\xE2\x82\xAC", 4 ) == "ab" );
            ASSERT_TRUE( CEosLobbyCodec::SanitizeText( "ab\xE2\x82\xAC", 5 ) == "ab\xE2\x82\xAC" );

            // a character cut off by the end of the text goes
            ASSERT_TRUE( CEosLobbyCodec::SanitizeText( "ab\xE2\x82", 64 ) == "ab" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The proof of a password depends on the salt, the user and the password and shows nothing of the password; salts are random" )
        try
        {
            CORE::CString salt = "00112233445566778899aabbccddeeff";
            CORE::CString proof = CEosLobbyCodec::MakePasswordProof( salt, OwnerId, "open sesame" );
            ASSERT_TRUE( 65 == proof.ByteSize() );
            ASSERT_TRUE( proof == CEosLobbyCodec::MakePasswordProof( salt, OwnerId, "open sesame" ) );
            ASSERT_TRUE( proof != CEosLobbyCodec::MakePasswordProof( salt, OwnerId, "open sesamE" ) );
            ASSERT_TRUE( proof != CEosLobbyCodec::MakePasswordProof( salt, "00112233445566778899aabbccdd11", "open sesame" ) );
            ASSERT_TRUE( proof != CEosLobbyCodec::MakePasswordProof( "ffeeddccbbaa99887766554433221100", OwnerId, "open sesame" ) );
            ASSERT_TRUE( proof.HasSubstr( "sesame" ) < 0 );

            // the user and the password cannot be shifted into each other
            ASSERT_TRUE( CEosLobbyCodec::MakePasswordProof( salt, "ab", "cd" ) != CEosLobbyCodec::MakePasswordProof( salt, "a", "bcd" ) );

            CORE::CString first = CEosLobbyCodec::MakeSalt();
            CORE::CString second = CEosLobbyCodec::MakeSalt();
            ASSERT_TRUE( 33 == first.ByteSize() && 33 == second.ByteSize() && first != second );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The socket of a lobby is named from the lobby id: the same for all, another for another lobby and another kind" )
        try
        {
            CORE::CString a = CEosLobbyCodec::MakeSocketName( "gg", "ABCDEF0123456789abcdef0123456789" );
            ASSERT_TRUE( 27 == a.ByteSize() && 0 == a.HasSubstr( "gg" ) );
            ASSERT_TRUE( a == CEosLobbyCodec::MakeSocketName( "gg", "ABCDEF0123456789abcdef0123456789" ) );
            ASSERT_TRUE( a != CEosLobbyCodec::MakeSocketName( "gg", "ABCDEF0123456789abcdef0123456788" ) );
            ASSERT_TRUE( a != CEosLobbyCodec::MakeSocketName( "gc", "ABCDEF0123456789abcdef0123456789" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Ids and keys: what is not letters and digits is not an id, keys are bounded" )
        try
        {
            ASSERT_TRUE( CEosLobbyCodec::IsValidUserId( OwnerId ) && CEosLobbyCodec::IsValidLobbyId( "abcDEF0123" ) );
            ASSERT_FALSE( CEosLobbyCodec::IsValidUserId( "" ) || CEosLobbyCodec::IsValidUserId( "ab cd" ) || CEosLobbyCodec::IsValidUserId( "ab/cd" ) || CEosLobbyCodec::IsValidUserId( "ab|cd" ) );
            ASSERT_FALSE( CEosLobbyCodec::IsValidUserId( Repeat( 'a', 100 ) ) );
            ASSERT_FALSE( CEosLobbyCodec::IsValidLobbyId( "" ) || CEosLobbyCodec::IsValidLobbyId( "../x" ) || CEosLobbyCodec::IsValidLobbyId( Repeat( 'a', 80 ) ) );

            CEosLobbyLimits limits;
            ASSERT_TRUE( CEosLobbyCodec::IsValidKey( "st_12", limits ) && CEosLobbyCodec::IsValidKey( "s_map", limits ) );
            ASSERT_FALSE( CEosLobbyCodec::IsValidKey( "", limits ) || CEosLobbyCodec::IsValidKey( "a b", limits ) || CEosLobbyCodec::IsValidKey( "a-b", limits ) );
            ASSERT_FALSE( CEosLobbyCodec::IsValidKey( Repeat( 'k', 65 ), limits ) );
            ASSERT_TRUE( CEosLobbyCodec::IsValidKey( Repeat( 'k', 64 ), limits ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A request of a member is parsed within bounds: the sequence, the verb and two arguments, the separator cannot be smuggled in" )
        try
        {
            UInt32 sequence = 0;
            CORE::CString verb;
            CORE::CString a1;
            CORE::CString a2;
            CORE::CString text = CEosLobbyCodec::MakeRequest( 42, "set", "map", "ice|dust" );
            ASSERT_TRUE( CEosLobbyCodec::ParseRequest( text, sequence, verb, a1, a2 ) );
            ASSERT_TRUE( 42 == sequence && "set" == verb && "map" == a1 && "ice dust" == a2 );

            ASSERT_TRUE( CEosLobbyCodec::ParseRequest( "7|kick", sequence, verb, a1, a2 ) && 7 == sequence && "kick" == verb && a1.IsNULLOrEmpty() );

            // not requests
            ASSERT_FALSE( CEosLobbyCodec::ParseRequest( "", sequence, verb, a1, a2 ) );
            ASSERT_FALSE( CEosLobbyCodec::ParseRequest( "garbage that is no request", sequence, verb, a1, a2 ) );
            ASSERT_FALSE( CEosLobbyCodec::ParseRequest( "x|set|a|b", sequence, verb, a1, a2 ) );
            ASSERT_FALSE( CEosLobbyCodec::ParseRequest( "-1|set|a|b", sequence, verb, a1, a2 ) );
            ASSERT_FALSE( CEosLobbyCodec::ParseRequest( "99999999999|set|a|b", sequence, verb, a1, a2 ) );
            ASSERT_FALSE( CEosLobbyCodec::ParseRequest( "1|set|a|b|c|d", sequence, verb, a1, a2 ) );
            ASSERT_FALSE( CEosLobbyCodec::ParseRequest( "1||a|b", sequence, verb, a1, a2 ) );
            ASSERT_FALSE( CEosLobbyCodec::ParseRequest( CORE::CString( "1|set|" ) + Repeat( 'a', 800 ), sequence, verb, a1, a2 ) );

            // what is cut: a long verb and long arguments, control characters
            ASSERT_TRUE( CEosLobbyCodec::ParseRequest( CORE::CString( "5|" ) + Repeat( 'v', 60 ) + "|" + Repeat( 'a', 300 ), sequence, verb, a1, a2 ) );
            ASSERT_TRUE( 32 == verb.Length() && 256 == a1.Length() );
            ASSERT_TRUE( CEosLobbyCodec::ParseRequest( "5|s\x01" "et|a\x02" "b|", sequence, verb, a1, a2 ) && "set" == verb && "ab" == a1 );

            bool ready = false;
            ASSERT_TRUE( CEosLobbyCodec::ParseReady( "12:1", sequence, ready ) && 12 == sequence && ready );
            ASSERT_TRUE( CEosLobbyCodec::ParseReady( "3:0", sequence, ready ) && 3 == sequence && !ready );
            ASSERT_FALSE( CEosLobbyCodec::ParseReady( "no:way", sequence, ready ) );
            ASSERT_FALSE( CEosLobbyCodec::ParseReady( "1:2", sequence, ready ) );
            ASSERT_FALSE( CEosLobbyCodec::ParseReady( "", sequence, ready ) );
            ASSERT_FALSE( CEosLobbyCodec::ParseReady( "1", sequence, ready ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A lobby found by a search becomes a listing with its endpoint, and one that does not hold up is not listed" )
        try
        {
            CEosLobbyLimits limits;
            SEosLobbyDetails details;
            details.lobbyId = "ABCDEF0123456789abcdef0123456789";
            details.ownerPuid = OwnerId;
            details.bucketId = "gucef-testgame";
            CEosLobbySummary summary = MakeSummary();
            CEosLobbyCodec::EncodeSummary( summary, details.attributes );

            MULTIPLAY::CGameListingEntry listing;
            CORE::CString reason;
            ASSERT_TRUE( CEosLobbyCodec::MakeListing( details, limits, listing, reason ) );
            ASSERT_TRUE( listing.listingId == details.lobbyId && listing.name == "Lobby of the tests" && listing.gameId == "testgame" && "eos" == listing.backendName );
            ASSERT_TRUE( 6 == listing.maxPlayers && 2 == listing.players && 1 == listing.spectators && listing.passworded && listing.hostName == "Hosty" );
            ASSERT_TRUE( 1 == listing.endpoints.size() && listing.endpoints[ 0 ] == "eos-p2p://" + CORE::CString( OwnerId ) + "/" + CEosLobbyCodec::MakeSocketName( "gg", details.lobbyId ) );
            ASSERT_TRUE( MULTIPLAY::CGameListingEntry::KIND_LOBBY == listing.kind && listing.extra.GetValueAlways( "lobbyService" ).AsString() == "eos" );

            // players that exceed the maximum are shown as the maximum
            TEosAttributeVector more = details.attributes;
            FindMutable( more, CEosLobbyCodec::KeyPlayers )->intValue = 900;
            details.attributes = more;
            ASSERT_TRUE( CEosLobbyCodec::MakeListing( details, limits, listing, reason ) && 6 == listing.players );

            // not written by its owner, an owner that is no id, a lobby id that is no id
            SEosLobbyDetails forged = details;
            forged.ownerPuid = "00112233445566778899aabbccdd11";
            ASSERT_FALSE( CEosLobbyCodec::MakeListing( forged, limits, listing, reason ) );
            forged = details;
            forged.ownerPuid = "../../x";
            ASSERT_FALSE( CEosLobbyCodec::MakeListing( forged, limits, listing, reason ) );
            forged = details;
            forged.lobbyId = "no id here";
            ASSERT_FALSE( CEosLobbyCodec::MakeListing( forged, limits, listing, reason ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The limits and the settings of the lobby service are saved and loaded the same, a limit that makes no sense is refused" )
        try
        {
            CEosLobbyLimits limits;
            limits.maxLobbyAttributes = 40;
            limits.maxAttributeLength = 500;
            CORE::CDataNode node( "Limits" );
            ASSERT_TRUE( limits.SaveConfig( node ) );
            CEosLobbyLimits loaded;
            ASSERT_TRUE( loaded.LoadConfig( node ) );
            ASSERT_TRUE( 40 == loaded.maxLobbyAttributes && 500 == loaded.maxAttributeLength && limits.maxBlobBytes == loaded.maxBlobBytes );
            ASSERT_TRUE( loaded.IsWellFormed() );

            CEosLobbyLimits tooSmall;
            tooSmall.maxAttributeLength = 1;
            ASSERT_FALSE( tooSmall.IsWellFormed() );
            CEosLobbyLimits noAttributes;
            noAttributes.maxLobbyAttributes = 2;
            ASSERT_FALSE( noAttributes.IsWellFormed() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
