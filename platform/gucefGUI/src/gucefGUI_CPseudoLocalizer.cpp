/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#include "gucefGUI_CPseudoLocalizer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The UTF-8 bytes of the accented form of an ASCII letter, NULL for a letter that keeps its form */
const char*
AccentOf( char c )
{
    switch ( c )
    {
        case 'a': return "\xC3\xA1";    // U+00E1
        case 'e': return "\xC3\xA9";    // U+00E9
        case 'i': return "\xC3\xAD";    // U+00ED
        case 'o': return "\xC3\xB6";    // U+00F6
        case 'u': return "\xC3\xBC";    // U+00FC
        case 'c': return "\xC3\xA7";    // U+00E7
        case 'n': return "\xC3\xB1";    // U+00F1
        case 'y': return "\xC3\xBD";    // U+00FD
        case 'A': return "\xC3\x85";    // U+00C5
        case 'E': return "\xC3\x89";    // U+00C9
        case 'O': return "\xC3\x96";    // U+00D6
        case 'U': return "\xC3\x9C";    // U+00DC
        default: return GUCEF_NULL;
    }
}

/** The padding character, U+00B7 (middle dot), in UTF-8 */
const char* const Padding = "\xC2\xB7";

bool
IsContinuationByte( unsigned char c )
{
    return 0x80 == ( c & 0xC0 );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPseudoLocalizer::CPseudoLocalizer( void )
    : CIDisplayTextTransform()
    , m_expansionPercent( DefaultExpansionPercent )
    , m_shortStringExpansionPercent( DefaultShortStringExpansionPercent )
    , m_shortStringLength( DefaultShortStringLength )
    , m_accents( true )
    , m_brackets( true )
    , m_lineSpacing( 1.0f )
    , m_cache()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPseudoLocalizer::~CPseudoLocalizer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CPseudoLocalizer::Transform( const CString& text ) const
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() )
        return text;

    // a string without letters (a number, a time, a sign) is not translated and so not changed
    const char* bytes = text.C_String();
    const UInt32 length = text.Length();
    bool hasLetters = false;
    UInt32 codePoints = 0;
    for ( UInt32 i=0; i<length; ++i )
    {
        const unsigned char c = (unsigned char) bytes[ i ];
        if ( !IsContinuationByte( c ) )
            ++codePoints;
        if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || c >= 0xC0 )
            hasLetters = true;
    }
    if ( !hasLetters )
        return text;

    CString result;
    if ( m_brackets )
        result += '[';
    for ( UInt32 i=0; i<length; ++i )
    {
        const char* accent = m_accents ? AccentOf( bytes[ i ] ) : GUCEF_NULL;
        if ( GUCEF_NULL != accent )
            result += accent;
        else
            result += bytes[ i ];
    }

    // the padding: the expansion of the length in characters, rounded up, at least one character
    const UInt32 percent = codePoints <= m_shortStringLength ? m_shortStringExpansionPercent : m_expansionPercent;
    UInt32 extra = ( codePoints * percent + 99 ) / 100;
    if ( 0 == extra && percent > 0 )
        extra = 1;
    for ( UInt32 i=0; i<extra; ++i )
        result += Padding;
    if ( m_brackets )
        result += ']';
    return result;
}

/*-------------------------------------------------------------------------*/

CString
CPseudoLocalizer::TransformText( const CString& text )
{GUCEF_TRACE;

    TTextMap::iterator i = m_cache.find( text );
    if ( i != m_cache.end() )
        return (*i).second;
    if ( m_cache.size() >= (size_t) MaxCacheEntries )
        m_cache.clear();
    const CString result = Transform( text );
    m_cache[ text ] = result;
    return result;
}

/*-------------------------------------------------------------------------*/

Float32
CPseudoLocalizer::GetLineSpacing( void ) const
{GUCEF_TRACE;

    return m_lineSpacing;
}

/*-------------------------------------------------------------------------*/

void
CPseudoLocalizer::SetExpansionPercent( UInt32 percent )
{GUCEF_TRACE;

    m_expansionPercent = percent;
    ClearCache();
}

/*-------------------------------------------------------------------------*/

UInt32
CPseudoLocalizer::GetExpansionPercent( void ) const
{GUCEF_TRACE;

    return m_expansionPercent;
}

/*-------------------------------------------------------------------------*/

void
CPseudoLocalizer::SetShortStringExpansionPercent( UInt32 percent )
{GUCEF_TRACE;

    m_shortStringExpansionPercent = percent;
    ClearCache();
}

/*-------------------------------------------------------------------------*/

UInt32
CPseudoLocalizer::GetShortStringExpansionPercent( void ) const
{GUCEF_TRACE;

    return m_shortStringExpansionPercent;
}

/*-------------------------------------------------------------------------*/

void
CPseudoLocalizer::SetShortStringLength( UInt32 length )
{GUCEF_TRACE;

    m_shortStringLength = length;
    ClearCache();
}

/*-------------------------------------------------------------------------*/

UInt32
CPseudoLocalizer::GetShortStringLength( void ) const
{GUCEF_TRACE;

    return m_shortStringLength;
}

/*-------------------------------------------------------------------------*/

void
CPseudoLocalizer::SetAccents( bool accents )
{GUCEF_TRACE;

    m_accents = accents;
    ClearCache();
}

/*-------------------------------------------------------------------------*/

bool
CPseudoLocalizer::GetAccents( void ) const
{GUCEF_TRACE;

    return m_accents;
}

/*-------------------------------------------------------------------------*/

void
CPseudoLocalizer::SetBrackets( bool brackets )
{GUCEF_TRACE;

    m_brackets = brackets;
    ClearCache();
}

/*-------------------------------------------------------------------------*/

bool
CPseudoLocalizer::GetBrackets( void ) const
{GUCEF_TRACE;

    return m_brackets;
}

/*-------------------------------------------------------------------------*/

void
CPseudoLocalizer::SetLineSpacing( Float32 lineSpacing )
{GUCEF_TRACE;

    m_lineSpacing = lineSpacing < 1.0f ? 1.0f : lineSpacing;
}

/*-------------------------------------------------------------------------*/

void
CPseudoLocalizer::ClearCache( void )
{GUCEF_TRACE;

    m_cache.clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
