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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHCONTAINER_H
#include "gucefMULTIPLAY_CMatchContainer.h"
#define GUCEF_MULTIPLAY_CMATCHCONTAINER_H
#endif /* GUCEF_MULTIPLAY_CMATCHCONTAINER_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHSIGNING_H
#include "gucefMULTIPLAY_CMatchSigning.h"
#define GUCEF_MULTIPLAY_CMATCHSIGNING_H
#endif /* GUCEF_MULTIPLAY_CMATCHSIGNING_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHID_H
#include "gucefMULTIPLAY_CMatchId.h"
#define GUCEF_MULTIPLAY_CMATCHID_H
#endif /* GUCEF_MULTIPLAY_CMATCHID_H ? */

#include "gucefMULTIPLAY_CMatchTextArmor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char Base64Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/** A block found in a text, with what the reader made of it */
struct SArmorBlock
{
    CORE::CString matchId;
    UInt32 revision;
    UInt32 length;
    CORE::CString sha256;
    CORE::CString checksum;
    CORE::CDynamicBuffer encoded;               /**< the data characters, without blanks; bytes, not a string: a string would be rescanned for every character */
    bool hasEnd;
    bool inBody;
    bool tooLarge;
    bool badCharacters;
    bool hasChecksum;
    CORE::CString headerError;

    SArmorBlock( void ) : matchId(), revision( 0 ), length( 0 ), sha256(), checksum(), encoded(), hasEnd( false ), inBody( false ), tooLarge( false ), badCharacters( false ), hasChecksum( false ), headerError() {}
};
typedef GUCEF::vector< SArmorBlock > TArmorBlockVector;

/** Header values are plain ASCII: anything else is not a header of ours, and is never made into a string */
bool
IsPrintableAscii( const char* p, UInt32 size )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<size; ++i )
        if ( (unsigned char) p[ i ] < 0x20 || (unsigned char) p[ i ] > 0x7E )
            return false;
    return true;
}

inline bool IsBlank( char c ) { return ' ' == c || '\t' == c || '\r' == c || '\n' == c || '\f' == c || '\v' == c; }

inline int Base64Value( char c )
{
    if ( c >= 'A' && c <= 'Z' ) return c - 'A';
    if ( c >= 'a' && c <= 'z' ) return c - 'a' + 26;
    if ( c >= '0' && c <= '9' ) return c - '0' + 52;
    if ( '+' == c ) return 62;
    if ( '/' == c ) return 63;
    return -1;
}

/** Strict: whole groups of four, padding only at the end, unused bits zero. Output is appended after the size check */
bool
Base64DecodeStrict( const char* p, UInt32 n, UInt32 expectedSize, CORE::CDynamicBuffer& output )
{GUCEF_TRACE;

    if ( 0 == n || 0 != n % 4 )
        return false;
    UInt32 padding = 0;
    if ( '=' == p[ n - 1 ] )
        padding = ( '=' == p[ n - 2 ] ) ? 2 : 1;
    UInt32 decodedSize = n / 4 * 3 - padding;
    if ( decodedSize != expectedSize )
        return false;

    output.Clear();
    output.SetBufferSize( decodedSize + 3 );
    UInt8* out = static_cast< UInt8* >( output.GetBufferPtr() );
    UInt32 outPos = 0;
    for ( UInt32 i=0; i<n; i+=4 )
    {
        int v[ 4 ];
        for ( int j=0; j<4; ++j )
        {
            char c = p[ i + j ];
            if ( '=' == c )
            {
                // padding is only allowed in the last group, in the last positions
                if ( i + 4 != n || j < 4 - (int) padding )
                    return false;
                v[ j ] = 0;
            }
            else
            {
                v[ j ] = Base64Value( c );
                if ( v[ j ] < 0 )
                    return false;
            }
        }
        UInt32 triple = ( (UInt32) v[ 0 ] << 18 ) | ( (UInt32) v[ 1 ] << 12 ) | ( (UInt32) v[ 2 ] << 6 ) | (UInt32) v[ 3 ];
        if ( i + 4 == n && padding == 2 && ( triple & 0xFFFF ) != 0 ) return false;
        if ( i + 4 == n && padding == 1 && ( triple & 0xFF ) != 0 ) return false;
        out[ outPos++ ] = (UInt8)( triple >> 16 );
        if ( outPos < decodedSize ) out[ outPos++ ] = (UInt8)( triple >> 8 );
        if ( outPos < decodedSize ) out[ outPos++ ] = (UInt8) triple;
    }
    output.SetDataSize( decodedSize );
    return outPos == decodedSize;
}

CORE::CString
Base64EncodeLines( const UInt8* data, UInt32 size, UInt32 width )
{GUCEF_TRACE;

    CORE::CString result;
    UInt32 encodedLength = ( size + 2 ) / 3 * 4;
    char* out = result.Reserve( encodedLength + encodedLength / width + 2 );
    UInt32 pos = 0;
    UInt32 column = 0;
    for ( UInt32 i=0; i<size; i+=3 )
    {
        UInt32 triple = (UInt32) data[ i ] << 16;
        if ( i + 1 < size ) triple |= (UInt32) data[ i + 1 ] << 8;
        if ( i + 2 < size ) triple |= (UInt32) data[ i + 2 ];
        char quad[ 4 ];
        quad[ 0 ] = Base64Alphabet[ ( triple >> 18 ) & 63 ];
        quad[ 1 ] = Base64Alphabet[ ( triple >> 12 ) & 63 ];
        quad[ 2 ] = i + 1 < size ? Base64Alphabet[ ( triple >> 6 ) & 63 ] : '=';
        quad[ 3 ] = i + 2 < size ? Base64Alphabet[ triple & 63 ] : '=';
        for ( int j=0; j<4; ++j )
        {
            if ( column == width )
            {
                out[ pos++ ] = '\n';
                column = 0;
            }
            out[ pos++ ] = quad[ j ];
            ++column;
        }
    }
    out[ pos ] = '\0';
    result.DetermineLength();
    return result;
}

bool
ParseUInt32Strict( const CORE::CString& text, UInt32& value )
{GUCEF_TRACE;

    const char* p = text.C_String();
    const UInt32 length = (UInt32) strlen( p );
    if ( 0 == length || length > 10 )
        return false;
    UInt64 v = 0;
    for ( UInt32 i=0; i<length; ++i )
    {
        if ( p[ i ] < '0' || p[ i ] > '9' )
            return false;
        v = v * 10 + (UInt64)( p[ i ] - '0' );
    }
    if ( v > 0xFFFFFFFFull )
        return false;
    value = (UInt32) v;
    return true;
}

CORE::CString
HexOfUInt24( UInt32 value )
{GUCEF_TRACE;

    static const char digits[] = "0123456789abcdef";
    char text[ 7 ];
    for ( int i=0; i<6; ++i )
        text[ i ] = digits[ ( value >> ( ( 5 - i ) * 4 ) ) & 0x0F ];
    text[ 6 ] = '\0';
    return CORE::CString( text );
}

bool
EqualsIgnoreCase( const char* a, UInt32 aLen, const char* b )
{GUCEF_TRACE;

    UInt32 bLen = (UInt32) strlen( b );
    if ( aLen != bLen )
        return false;
    for ( UInt32 i=0; i<aLen; ++i )
    {
        char x = a[ i ]; char y = b[ i ];
        if ( x >= 'A' && x <= 'Z' ) x += 32;
        if ( y >= 'A' && y <= 'Z' ) y += 32;
        if ( x != y ) return false;
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
MatchArmorResultToString( TMatchArmorResult result )
{GUCEF_TRACE;

    switch ( result )
    {
        case MATCHARMOR_OK:           return "ok";
        case MATCHARMOR_NO_BLOCK:     return "There is no turn in the text";
        case MATCHARMOR_TOO_LARGE:    return "The text is larger than a turn may be";
        case MATCHARMOR_BAD_HEADER:   return "The header of the turn in the text is not valid";
        case MATCHARMOR_BAD_ENCODING: return "The turn in the text is not valid, please ask for it to be sent again";
        case MATCHARMOR_TRUNCATED:    return "The turn in the text is cut off, please ask for it to be sent again";
        case MATCHARMOR_BAD_CHECKSUM: return "The turn in the text was damaged on its way, please ask for it to be sent again";
        case MATCHARMOR_BAD_HASH:     return "The turn in the text was damaged on its way, please ask for it to be sent again";
        default:                      return "unknown";
    }
}

/*-------------------------------------------------------------------------*/

CMatchTextArmor::CMatchTextArmor( const CORE::CString& label )
    : m_label( label )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchTextArmor::CMatchTextArmor( const CMatchTextArmor& src )
    : m_label( src.m_label )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchTextArmor::~CMatchTextArmor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchTextArmor&
CMatchTextArmor::operator=( const CMatchTextArmor& src )
{GUCEF_TRACE;

    if ( &src != this )
        m_label = src.m_label;
    return *this;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchTextArmor::GetLabel( void ) const
{GUCEF_TRACE;

    return m_label;
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchTextArmor::Crc24( const void* data, UInt32 size )
{GUCEF_TRACE;

    UInt32 crc = 0xB704CE;
    const UInt8* p = static_cast< const UInt8* >( data );
    for ( UInt32 i=0; i<size; ++i )
    {
        crc ^= (UInt32) p[ i ] << 16;
        for ( int j=0; j<8; ++j )
        {
            crc <<= 1;
            if ( crc & 0x1000000 )
                crc ^= 0x1864CFB;
        }
    }
    return crc & 0xFFFFFF;
}

/*-------------------------------------------------------------------------*/

bool
CMatchTextArmor::Encode( const CORE::CString& matchId, UInt32 revision, const void* container, UInt32 containerSize, CORE::CString& text ) const
{GUCEF_TRACE;

    text.Clear();
    if ( GUCEF_NULL == container || 0 == containerSize || containerSize > CMatchContainer::MaxContainerSize || !CMatchId::IsValid( matchId ) )
        return false;
    // the cap is on the text: the encoded data and the few lines around it
    if ( (UInt64)( containerSize + 2 ) / 3 * 4 + ( (UInt64)( containerSize + 2 ) / 3 * 4 ) / LineWidth + 512 > MaxEncodedTextSize )
        return false;

    UInt8 digest[ 32 ];
    DRN::CDRNCrypto::Sha256( container, containerSize, digest );

    text += "-----BEGIN ";
    text += m_label;
    text += "-----\n";
    text += "Match: ";
    text += matchId;
    text += "\nRevision: ";
    text += CORE::UInt32ToString( revision );
    text += "\nLength: ";
    text += CORE::UInt32ToString( containerSize );
    text += "\nSHA-256: ";
    text += DRN::CDRNCrypto::ToHex( digest, 32 );
    text += "\nVersion: 1\n\n";
    text += Base64EncodeLines( static_cast< const UInt8* >( container ), containerSize, LineWidth );
    text += "\nChecksum: ";
    text += HexOfUInt24( Crc24( container, containerSize ) );
    text += "\n-----END ";
    text += m_label;
    text += "-----\n";
    return strlen( text.C_String() ) <= MaxEncodedTextSize;
}

/*-------------------------------------------------------------------------*/

TMatchArmorResult
CMatchTextArmor::Decode( const void* text                   ,
                         UInt32 textSize                    ,
                         const CORE::CString& wantedMatchId ,
                         UInt32 wantedRevision              ,
                         CORE::CDynamicBuffer& container    ,
                         UInt32* foundRevision              ) const
{GUCEF_TRACE;

    container.Clear();
    if ( GUCEF_NULL == text || 0 == textSize )
        return MATCHARMOR_NO_BLOCK;
    if ( textSize > MaxScannedTextSize )
        return MATCHARMOR_TOO_LARGE;

    const CORE::CString wantedId = wantedMatchId.IsNULLOrEmpty() ? CORE::CString::Empty : CMatchId::Normalize( wantedMatchId );
    if ( !wantedMatchId.IsNULLOrEmpty() && wantedId.IsNULLOrEmpty() )
        return MATCHARMOR_NO_BLOCK;

    const char* chars = static_cast< const char* >( text );
    const CORE::CString beginLine = "-----BEGIN " + m_label + "-----";
    const CORE::CString endLine = "-----END " + m_label + "-----";
    const UInt32 beginLength = (UInt32) strlen( beginLine.C_String() );
    const UInt32 endLength = (UInt32) strlen( endLine.C_String() );

    TArmorBlockVector blocks;
    bool inBlock = false;
    UInt32 pos = 0;
    while ( pos < textSize )
    {
        // one line: up to CR, LF or CRLF
        UInt32 lineStart = pos;
        while ( pos < textSize && '\n' != chars[ pos ] && '\r' != chars[ pos ] )
            ++pos;
        UInt32 lineEnd = pos;
        if ( pos < textSize )
        {
            if ( '\r' == chars[ pos ] && pos + 1 < textSize && '\n' == chars[ pos + 1 ] )
                pos += 2;
            else
                ++pos;
        }

        // the quote prefix and the blanks around the line are not part of it
        while ( lineStart < lineEnd && ( '>' == chars[ lineStart ] || IsBlank( chars[ lineStart ] ) ) )
            ++lineStart;
        while ( lineEnd > lineStart && IsBlank( chars[ lineEnd - 1 ] ) )
            --lineEnd;
        UInt32 lineLength = lineEnd - lineStart;
        const char* line = chars + lineStart;

        if ( lineLength == beginLength && 0 == memcmp( line, beginLine.C_String(), lineLength ) )
        {
            // a block that was not ended is kept as it is: it is a truncated block
            blocks.push_back( SArmorBlock() );
            blocks.back().encoded.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
            inBlock = true;
            continue;
        }
        if ( !inBlock )
            continue;
        SArmorBlock& block = blocks.back();

        if ( lineLength == endLength && 0 == memcmp( line, endLine.C_String(), lineLength ) )
        {
            block.hasEnd = true;
            inBlock = false;
            continue;
        }
        if ( 0 == lineLength )
        {
            if ( !block.inBody && block.headerError.IsNULLOrEmpty() )
                block.inBody = true;
            continue;
        }

        // a header line is "Name: value" until the data starts
        const char* colon = static_cast< const char* >( memchr( line, ':', lineLength ) );
        if ( GUCEF_NULL != colon )
        {
            UInt32 nameLength = (UInt32)( colon - line );
            const char* value = colon + 1;
            UInt32 valueLength = lineLength - nameLength - 1;
            while ( valueLength > 0 && IsBlank( *value ) ) { ++value; --valueLength; }
            if ( EqualsIgnoreCase( line, nameLength, "Checksum" ) )
            {
                if ( IsPrintableAscii( value, valueLength ) )
                    block.checksum.Set( value, valueLength );
                else
                    block.headerError = "checksum";
                block.hasChecksum = true;
                block.inBody = true;
                continue;
            }
            if ( !block.inBody )
            {
                if ( !IsPrintableAscii( value, valueLength ) )
                {
                    block.headerError = "value";
                    continue;
                }
                CORE::CString valueText( value, valueLength );
                if ( EqualsIgnoreCase( line, nameLength, "Match" ) )
                    block.matchId = CMatchId::Normalize( valueText );
                else
                if ( EqualsIgnoreCase( line, nameLength, "Revision" ) )
                {
                    if ( !ParseUInt32Strict( valueText, block.revision ) )
                        block.headerError = "revision";
                }
                else
                if ( EqualsIgnoreCase( line, nameLength, "Length" ) )
                {
                    if ( !ParseUInt32Strict( valueText, block.length ) )
                        block.headerError = "length";
                }
                else
                if ( EqualsIgnoreCase( line, nameLength, "SHA-256" ) )
                    block.sha256 = valueText.Lowercase();
                // other header lines (Version) are not needed to read the data
                continue;
            }
        }

        // data: the first line without a colon ends the header when the blank line is lost
        block.inBody = true;
        if ( block.hasChecksum || block.tooLarge )
            continue;
        for ( UInt32 i=0; i<lineLength; ++i )
        {
            char c = line[ i ];
            if ( IsBlank( c ) )
                continue;
            if ( Base64Value( c ) < 0 && '=' != c )
            {
                block.badCharacters = true;
                break;
            }
            if ( block.encoded.GetDataSize() >= MaxEncodedTextSize )
            {
                block.tooLarge = true;
                break;
            }
            block.encoded.Append( &c, 1 );
        }
    }

    // judge the blocks of the wanted match
    TMatchArmorResult bestError = MATCHARMOR_NO_BLOCK;
    bool haveError = false;
    bool found = false;
    UInt32 bestRevision = 0;
    for ( size_t b=0; b<blocks.size(); ++b )
    {
        SArmorBlock& block = blocks[ b ];
        if ( !wantedId.IsNULLOrEmpty() && block.matchId != wantedId )
            continue;
        if ( wantedRevision != 0 && block.revision != wantedRevision )
            continue;

        TMatchArmorResult verdict = MATCHARMOR_OK;
        CORE::CDynamicBuffer bytes;
        if ( !block.headerError.IsNULLOrEmpty() || !CMatchId::IsValid( block.matchId ) || 0 == block.length ||
             !CMatchSigning::IsHexOfLength( block.sha256, CMatchSigning::Sha256HexSize ) )
            verdict = MATCHARMOR_BAD_HEADER;
        else
        if ( block.tooLarge || block.length > CMatchContainer::MaxContainerSize )
            verdict = MATCHARMOR_TOO_LARGE;
        else
        if ( !block.hasEnd || !block.hasChecksum )
            verdict = MATCHARMOR_TRUNCATED;
        else
        if ( block.badCharacters )
            verdict = MATCHARMOR_BAD_ENCODING;
        else
        if ( !Base64DecodeStrict( static_cast< const char* >( block.encoded.GetConstBufferPtr() ), block.encoded.GetDataSize(), block.length, bytes ) )
            verdict = block.encoded.GetDataSize() < ( block.length + 2 ) / 3 * 4 ? MATCHARMOR_TRUNCATED : MATCHARMOR_BAD_ENCODING;
        else
        if ( block.checksum.Lowercase() != HexOfUInt24( Crc24( bytes.GetConstBufferPtr(), bytes.GetDataSize() ) ) )
            verdict = MATCHARMOR_BAD_CHECKSUM;
        else
        {
            UInt8 digest[ 32 ];
            DRN::CDRNCrypto::Sha256( bytes.GetConstBufferPtr(), bytes.GetDataSize(), digest );
            if ( DRN::CDRNCrypto::ToHex( digest, 32 ) != block.sha256 )
                verdict = MATCHARMOR_BAD_HASH;
        }

        if ( MATCHARMOR_OK == verdict )
        {
            if ( !found || block.revision > bestRevision )
            {
                found = true;
                bestRevision = block.revision;
                container.Swap( bytes );
            }
        }
        else
        if ( !haveError )
        {
            haveError = true;
            bestError = verdict;
        }
    }

    if ( found )
    {
        if ( GUCEF_NULL != foundRevision )
            *foundRevision = bestRevision;
        return MATCHARMOR_OK;
    }
    return bestError;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
