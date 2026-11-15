/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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
#include <algorithm>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#include "gucefASSETSTORE_CAssetKeywordNormalizer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The bytes of the text without the terminating zero, for every string format of CString */
inline UInt32
ByteLengthOf( const CORE::CString& text )
{
    const UInt32 size = text.ByteSize();
    return size > 0 ? size - 1 : 0;
}

/** The letters of Latin-1 Supplement and Latin Extended-A without their accents, lower case; GUCEF_NULL: not a letter */
const char*
FoldLatin( UInt32 codePoint )
{
    if ( codePoint >= 0xC0 && codePoint <= 0xFF )
    {
        static const char* const latin1[ 64 ] =
        {
            "a", "a", "a", "a", "a", "a", "ae", "c", "e", "e", "e", "e", "i", "i", "i", "i",        // C0 .. CF
            "d", "n", "o", "o", "o", "o", "o", GUCEF_NULL, "o", "u", "u", "u", "u", "y", "th", "ss", // D0 .. DF
            "a", "a", "a", "a", "a", "a", "ae", "c", "e", "e", "e", "e", "i", "i", "i", "i",        // E0 .. EF
            "d", "n", "o", "o", "o", "o", "o", GUCEF_NULL, "o", "u", "u", "u", "u", "y", "th", "y"   // F0 .. FF
        };
        return latin1[ codePoint - 0xC0 ];
    }
    if ( codePoint >= 0x100 && codePoint <= 0x17F )
    {
        static const char* const extendedA[ 128 ] =
        {
            "a", "a", "a", "a", "a", "a", "c", "c", "c", "c", "c", "c", "c", "c", "d", "d",       // 100
            "d", "d", "e", "e", "e", "e", "e", "e", "e", "e", "e", "e", "g", "g", "g", "g",       // 110
            "g", "g", "g", "g", "h", "h", "h", "h", "i", "i", "i", "i", "i", "i", "i", "i",       // 120
            "i", "i", "ij", "ij", "j", "j", "k", "k", "k", "l", "l", "l", "l", "l", "l", "l",     // 130
            "l", "l", "l", "n", "n", "n", "n", "n", "n", "n", "n", "n", "o", "o", "o", "o",       // 140
            "o", "o", "oe", "oe", "r", "r", "r", "r", "r", "r", "s", "s", "s", "s", "s", "s",     // 150
            "s", "s", "t", "t", "t", "t", "t", "t", "u", "u", "u", "u", "u", "u", "u", "u",       // 160
            "u", "u", "u", "u", "w", "w", "y", "y", "y", "z", "z", "z", "z", "z", "z", "s"        // 170
        };
        return extendedA[ codePoint - 0x100 ];
    }
    return GUCEF_NULL;
}

/** Decodes one UTF-8 sequence at 'index', moving it past the sequence; a broken byte is taken as it is */
UInt32
DecodeUtf8( const unsigned char* text, UInt32 length, UInt32& index )
{
    const unsigned char c = text[ index ];
    UInt32 needed = 0;
    UInt32 codePoint = c;
    if ( c >= 0xF0 && c < 0xF8 )        { needed = 3; codePoint = c & 0x07; }
    else if ( c >= 0xE0 )               { needed = 2; codePoint = c & 0x0F; }
    else if ( c >= 0xC0 )               { needed = 1; codePoint = c & 0x1F; }
    if ( c >= 0x80 && 0 == needed )
    {
        ++index;
        return c;
    }
    for ( UInt32 n=1; n<=needed; ++n )
    {
        if ( index + n >= length || ( text[ index+n ] & 0xC0 ) != 0x80 )
        {
            ++index;
            return c;
        }
        codePoint = ( codePoint << 6 ) | ( text[ index+n ] & 0x3F );
    }
    index += needed + 1;
    return codePoint;
}

enum ECharClass
{
    CHAR_SEPARATOR  ,
    CHAR_LOWER      ,
    CHAR_UPPER      ,
    CHAR_DIGIT      ,
    CHAR_OTHER_LETTER
};

ECharClass
ClassOf( UInt32 codePoint )
{
    if ( codePoint >= 'a' && codePoint <= 'z' )
        return CHAR_LOWER;
    if ( codePoint >= 'A' && codePoint <= 'Z' )
        return CHAR_UPPER;
    if ( codePoint >= '0' && codePoint <= '9' )
        return CHAR_DIGIT;
    if ( codePoint < 0x80 )
        return CHAR_SEPARATOR;
    if ( ( codePoint >= 0xC0 && codePoint <= 0xDE && codePoint != 0xD7 ) )
        return CHAR_UPPER;
    if ( codePoint >= 0x100 && codePoint <= 0x17F )
        return ( 0 == ( codePoint & 1 ) ) ? CHAR_UPPER : CHAR_LOWER;
    if ( 0xD7 == codePoint || 0xF7 == codePoint || ( codePoint >= 0x80 && codePoint < 0xC0 ) || ( codePoint >= 0x2000 && codePoint <= 0x206F ) )
        return CHAR_SEPARATOR;    // symbols and punctuation
    return CHAR_OTHER_LETTER;
}

void
AppendUtf8( CORE::CString& text, UInt32 codePoint )
{
    char bytes[ 4 ];
    UInt32 count = 0;
    if ( codePoint < 0x80 )
    {
        bytes[ 0 ] = (char) codePoint;
        count = 1;
    }
    else if ( codePoint < 0x800 )
    {
        bytes[ 0 ] = (char) ( 0xC0 | ( codePoint >> 6 ) );
        bytes[ 1 ] = (char) ( 0x80 | ( codePoint & 0x3F ) );
        count = 2;
    }
    else if ( codePoint < 0x10000 )
    {
        bytes[ 0 ] = (char) ( 0xE0 | ( codePoint >> 12 ) );
        bytes[ 1 ] = (char) ( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) );
        bytes[ 2 ] = (char) ( 0x80 | ( codePoint & 0x3F ) );
        count = 3;
    }
    else
    {
        bytes[ 0 ] = (char) ( 0xF0 | ( codePoint >> 18 ) );
        bytes[ 1 ] = (char) ( 0x80 | ( ( codePoint >> 12 ) & 0x3F ) );
        bytes[ 2 ] = (char) ( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) );
        bytes[ 3 ] = (char) ( 0x80 | ( codePoint & 0x3F ) );
        count = 4;
    }
    text.Append( bytes, count );
}

bool
EndsWith( const CORE::CString& word, const char* suffix )
{
    const UInt32 suffixLength = (UInt32) strlen( suffix );
    const UInt32 length = ByteLengthOf( word );
    return length >= suffixLength && 0 == strcmp( word.C_String() + length - suffixLength, suffix );
}

bool
IsAllDigits( const CORE::CString& word )
{
    const char* text = word.C_String();
    const UInt32 length = ByteLengthOf( word );
    for ( UInt32 i=0; i<length; ++i )
        if ( text[ i ] < '0' || text[ i ] > '9' )
            return false;
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetKeywordNormalizer::CAssetKeywordNormalizer( void )
    : m_stopWords()
    , m_synonyms()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetKeywordNormalizer::SetStopWords( const CORE::CString::StringSet& stopWords )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_stopWords.clear();
    CORE::CString::StringSet::const_iterator i = stopWords.begin();
    while ( i != stopWords.end() )
    {
        m_stopWords.insert( Fold( (*i) ) );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetKeywordNormalizer::SetSynonyms( const CORE::CString::StringMap& synonyms )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_synonyms.clear();
    CORE::CString::StringMap::const_iterator i = synonyms.begin();
    while ( i != synonyms.end() )
    {
        m_synonyms[ Fold( (*i).first ) ] = Fold( (*i).second );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetKeywordNormalizer::AddSynonym( const CORE::CString& word, const CORE::CString& synonym )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_synonyms[ Fold( word ) ] = Fold( synonym );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetKeywordNormalizer::Fold( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString folded;
    const unsigned char* bytes = (const unsigned char*) text.C_String();
    const UInt32 length = ByteLengthOf( text );
    UInt32 index = 0;
    while ( index < length )
    {
        const UInt32 codePoint = DecodeUtf8( bytes, length, index );
        if ( codePoint < 0x80 )
        {
            folded += (char) ( ( codePoint >= 'A' && codePoint <= 'Z' ) ? codePoint + 32 : codePoint );
            continue;
        }
        const char* latin = FoldLatin( codePoint );
        if ( GUCEF_NULL != latin )
            folded += latin;
        else
            AppendUtf8( folded, codePoint );
    }
    return folded;
}

/*-------------------------------------------------------------------------*/

void
CAssetKeywordNormalizer::SplitWords( const CORE::CString& text, CORE::CString::StringVector& words )
{GUCEF_TRACE;

    words.clear();
    const unsigned char* bytes = (const unsigned char*) text.C_String();
    const UInt32 length = ByteLengthOf( text );

    // the code points with their classes first, so camel case can look ahead
    GUCEF::vector< UInt32 > codePoints;
    GUCEF::vector< int > classes;
    UInt32 index = 0;
    while ( index < length )
    {
        const UInt32 codePoint = DecodeUtf8( bytes, length, index );
        codePoints.push_back( codePoint );
        classes.push_back( (int) ClassOf( codePoint ) );
    }

    CORE::CString word;
    for ( size_t i=0; i<codePoints.size(); ++i )
    {
        const int current = classes[ i ];
        if ( CHAR_SEPARATOR == current )
        {
            if ( !word.IsNULLOrEmpty() )
                words.push_back( word );
            word.Clear();
            continue;
        }
        if ( !word.IsNULLOrEmpty() && i > 0 )
        {
            const int previous = classes[ i-1 ];
            const int next = i+1 < classes.size() ? classes[ i+1 ] : (int) CHAR_SEPARATOR;
            bool split = false;
            if ( ( CHAR_DIGIT == current ) != ( CHAR_DIGIT == previous ) )
                split = true;                                                       // Wall01
            else if ( CHAR_UPPER == current && CHAR_LOWER == previous )
                split = true;                                                       // metalCrate
            else if ( CHAR_UPPER == current && CHAR_UPPER == previous && CHAR_LOWER == next )
                split = true;                                                       // HTTPServer
            if ( split )
            {
                words.push_back( word );
                word.Clear();
            }
        }
        AppendUtf8( word, codePoints[ i ] );
    }
    if ( !word.IsNULLOrEmpty() )
        words.push_back( word );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetKeywordNormalizer::Stem( const CORE::CString& word )
{GUCEF_TRACE;

    const UInt32 length = word.Length();
    if ( length > 4 && EndsWith( word, "ies" ) )
        return word.CutChars( 3, false ) + "y";
    if ( length > 4 && EndsWith( word, "sses" ) )
        return word.CutChars( 2, false );
    if ( length > 4 && ( EndsWith( word, "xes" ) || EndsWith( word, "ches" ) || EndsWith( word, "shes" ) || EndsWith( word, "zes" ) ) )
        return word.CutChars( 2, false );
    if ( length > 3 && EndsWith( word, "s" ) && !EndsWith( word, "ss" ) && !EndsWith( word, "us" ) && !EndsWith( word, "is" ) )
        return word.CutChars( 1, false );
    return word;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetKeywordNormalizer::ApplySynonym( const CORE::CString& word ) const
{GUCEF_TRACE;

    CORE::CString::StringMap::const_iterator i = m_synonyms.find( word );
    return i == m_synonyms.end() ? word : (*i).second;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetKeywordNormalizer::NormalizeWord( const CORE::CString& word ) const
{GUCEF_TRACE;

    const CORE::CString folded = Fold( word );
    if ( folded.Length() < 2 || IsAllDigits( folded ) )
        return CORE::CString::Empty;

    MT::CScopeMutex lock( m_lock );
    if ( m_stopWords.end() != m_stopWords.find( folded ) )
        return CORE::CString::Empty;
    if ( m_synonyms.end() != m_synonyms.find( folded ) )
        return ApplySynonym( folded );
    const CORE::CString stem = Stem( folded );
    if ( m_stopWords.end() != m_stopWords.find( stem ) )
        return CORE::CString::Empty;
    return ApplySynonym( stem );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetKeywordNormalizer::Normalize( const CORE::CString& text, const CORE::CString& field, Int64 weight, TAssetKeywordVector& keywords ) const
{GUCEF_TRACE;

    CORE::CString::StringVector words;
    SplitWords( text, words );
    UInt32 added = 0;
    for ( size_t w=0; w<words.size(); ++w )
    {
        const CORE::CString normalized = NormalizeWord( words[ w ] );
        if ( normalized.IsNULLOrEmpty() )
            continue;
        bool found = false;
        for ( size_t k=0; k<keywords.size(); ++k )
        {
            if ( keywords[ k ].keyword == normalized && keywords[ k ].field == field )
            {
                if ( weight > keywords[ k ].weight )
                    keywords[ k ].weight = weight;
                found = true;
                break;
            }
        }
        if ( found )
            continue;
        CAssetKeyword keyword;
        keyword.keyword = normalized;
        keyword.original = words[ w ];
        keyword.field = field;
        keyword.weight = weight;
        keywords.push_back( keyword );
        ++added;
    }
    return added;
}

/*-------------------------------------------------------------------------*/

void
CAssetKeywordNormalizer::NormalizeQuery( const CORE::CString& text, CORE::CString::StringVector& keywords ) const
{GUCEF_TRACE;

    keywords.clear();
    CORE::CString::StringVector words;
    SplitWords( text, words );
    for ( size_t w=0; w<words.size(); ++w )
    {
        const CORE::CString normalized = NormalizeWord( words[ w ] );
        if ( normalized.IsNULLOrEmpty() )
            continue;
        if ( keywords.end() == std::find( keywords.begin(), keywords.end(), normalized ) )
            keywords.push_back( normalized );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
