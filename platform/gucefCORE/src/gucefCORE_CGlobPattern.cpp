/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefCORE_CGlobPattern.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** More alternatives than this from {a,b} make the pattern invalid: a guard against patterns that explode */
const size_t MaxAlternatives = 1024;

/** The code point at p (UTF-8) and the position after it; a broken sequence gives its first byte as the code point */
UInt32
NextCodePoint( const char*& p, const char* end )
{
    const unsigned char lead = (unsigned char) *p;
    UInt32 length = 1;
    UInt32 cp = lead;
    if ( lead >= 0xF0 )      { length = 4; cp = lead & 0x07; }
    else if ( lead >= 0xE0 ) { length = 3; cp = lead & 0x0F; }
    else if ( lead >= 0xC0 ) { length = 2; cp = lead & 0x1F; }
    if ( length > 1 && p + length <= end )
    {
        bool valid = true;
        for ( UInt32 i=1; i<length; ++i )
        {
            const unsigned char next = (unsigned char) p[ i ];
            if ( 0x80 != ( next & 0xC0 ) )
            {
                valid = false;
                break;
            }
            cp = ( cp << 6 ) | ( next & 0x3F );
        }
        if ( valid )
        {
            p += length;
            return cp;
        }
    }
    ++p;
    return lead;
}

/*-------------------------------------------------------------------------*/

/** The end of a [...] class that starts at p (at the '['), or NULL when it is not closed */
const char*
FindClassEnd( const char* p, const char* end )
{
    const char* q = p + 1;
    if ( q < end && ( '!' == *q || '^' == *q ) )
        ++q;
    if ( q < end && ']' == *q )      // a ']' first is a character of the class
        ++q;
    while ( q < end && ']' != *q )
    {
        if ( '\\' == *q && q + 1 < end )
            ++q;
        ++q;
    }
    return q < end ? q : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

/** Whether the code point is in the class between p (after '[') and classEnd (at ']') */
bool
ClassMatches( const char* p, const char* classEnd, UInt32 cp )
{
    bool negate = false;
    if ( p < classEnd && ( '!' == *p || '^' == *p ) )
    {
        negate = true;
        ++p;
    }
    bool found = false;
    bool first = true;
    while ( p < classEnd )
    {
        if ( '\\' == *p && p + 1 < classEnd )
            ++p;
        else
        if ( ']' == *p && !first )
            break;
        first = false;
        const UInt32 low = NextCodePoint( p, classEnd );
        UInt32 high = low;
        if ( p + 1 < classEnd && '-' == *p )
        {
            ++p;
            if ( '\\' == *p && p + 1 < classEnd )
                ++p;
            high = NextCodePoint( p, classEnd );
        }
        if ( cp >= low && cp <= high )
            found = true;
    }
    return found != negate;
}

/*-------------------------------------------------------------------------*/

/** Matches a pattern without {a,b} alternatives against a text */
bool
MatchAlternative( const char* p, const char* pEnd, const char* t, const char* tEnd )
{
    while ( p < pEnd )
    {
        if ( '*' == *p )
        {
            if ( p + 1 < pEnd && '*' == p[ 1 ] )
            {
                p += 2;
                if ( p < pEnd && '/' == *p )
                {
                    // "**/": no directory, or any directories
                    if ( MatchAlternative( p + 1, pEnd, t, tEnd ) )
                        return true;
                    for ( const char* q=t; q<tEnd; ++q )
                        if ( '/' == *q && MatchAlternative( p + 1, pEnd, q + 1, tEnd ) )
                            return true;
                    return false;
                }
                for ( const char* q=t; q<=tEnd; ++q )
                    if ( MatchAlternative( p, pEnd, q, tEnd ) )
                        return true;
                return false;
            }
            ++p;
            const char* q = t;
            while ( true )
            {
                if ( MatchAlternative( p, pEnd, q, tEnd ) )
                    return true;
                if ( q >= tEnd || '/' == *q )
                    return false;
                NextCodePoint( q, tEnd );
            }
        }
        if ( '?' == *p )
        {
            if ( t >= tEnd || '/' == *t )
                return false;
            NextCodePoint( t, tEnd );
            ++p;
            continue;
        }
        if ( '[' == *p )
        {
            const char* classEnd = FindClassEnd( p, pEnd );
            if ( GUCEF_NULL == classEnd )
                return false;
            if ( t >= tEnd || '/' == *t )
                return false;
            const UInt32 cp = NextCodePoint( t, tEnd );
            if ( !ClassMatches( p + 1, classEnd, cp ) )
                return false;
            p = classEnd + 1;
            continue;
        }
        if ( '\\' == *p && p + 1 < pEnd )
            ++p;
        if ( t >= tEnd || *t != *p )
            return false;
        ++t;
        ++p;
    }
    return t == tEnd;
}

/*-------------------------------------------------------------------------*/

/** Expands the {a,b} alternatives of a pattern; false for an open brace or too many alternatives */
bool
ExpandAlternatives( const CString& pattern, CString::StringVector& alternatives )
{
    const char* begin = pattern.C_String();
    const char* end = begin + pattern.ByteSize() - ( pattern.ByteSize() > 0 ? 1 : 0 );

    // the first top level '{' (not escaped, not in a [...] class)
    const char* open = GUCEF_NULL;
    for ( const char* p=begin; p<end; ++p )
    {
        if ( '\\' == *p && p + 1 < end )
        {
            ++p;
            continue;
        }
        if ( '[' == *p )
        {
            const char* classEnd = FindClassEnd( p, end );
            if ( GUCEF_NULL != classEnd )
            {
                p = classEnd;
                continue;
            }
        }
        if ( '{' == *p )
        {
            open = p;
            break;
        }
        if ( '}' == *p )
            return false;
    }
    if ( GUCEF_NULL == open )
    {
        if ( alternatives.size() >= MaxAlternatives )
            return false;
        alternatives.push_back( pattern );
        return true;
    }

    // its closing brace, and the commas between them at the same depth
    GUCEF::vector< const char* > commas;
    const char* close = GUCEF_NULL;
    Int32 depth = 0;
    for ( const char* p=open; p<end; ++p )
    {
        if ( '\\' == *p && p + 1 < end )
        {
            ++p;
            continue;
        }
        if ( '{' == *p )
            ++depth;
        else
        if ( '}' == *p )
        {
            if ( 0 == --depth )
            {
                close = p;
                break;
            }
        }
        else
        if ( ',' == *p && 1 == depth )
            commas.push_back( p );
    }
    if ( GUCEF_NULL == close )
        return false;

    const CString prefix( begin, (UInt32) ( open - begin ) );
    const CString suffix( close + 1, (UInt32) ( end - close - 1 ) );
    const char* partBegin = open + 1;
    commas.push_back( close );
    for ( size_t i=0; i<commas.size(); ++i )
    {
        const CString part( partBegin, (UInt32) ( commas[ i ] - partBegin ) );
        if ( !ExpandAlternatives( prefix + part + suffix, alternatives ) )
            return false;
        partBegin = commas[ i ] + 1;
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGlobPattern::CGlobPattern( void )
    : m_pattern()
    , m_caseSensitive( false )
    , m_valid( true )
    , m_alternatives()
{GUCEF_TRACE;

    m_alternatives.push_back( CString::Empty );
}

/*-------------------------------------------------------------------------*/

CGlobPattern::CGlobPattern( const CString& pattern, bool caseSensitive )
    : m_pattern()
    , m_caseSensitive( false )
    , m_valid( false )
    , m_alternatives()
{GUCEF_TRACE;

    SetPattern( pattern, caseSensitive );
}

/*-------------------------------------------------------------------------*/

bool
CGlobPattern::SetPattern( const CString& pattern, bool caseSensitive )
{GUCEF_TRACE;

    m_pattern = pattern;
    m_caseSensitive = caseSensitive;
    m_alternatives.clear();
    m_valid = ExpandAlternatives( caseSensitive ? pattern : pattern.Lowercase(), m_alternatives );
    if ( m_valid )
    {
        // an open '[' makes the pattern invalid too
        CString::StringVector::const_iterator i = m_alternatives.begin();
        while ( i != m_alternatives.end() && m_valid )
        {
            const char* begin = (*i).C_String();
            const char* end = begin + (*i).ByteSize() - ( (*i).ByteSize() > 0 ? 1 : 0 );
            for ( const char* p=begin; p<end; ++p )
            {
                if ( '\\' == *p && p + 1 < end )
                {
                    ++p;
                    continue;
                }
                if ( '[' == *p )
                {
                    const char* classEnd = FindClassEnd( p, end );
                    if ( GUCEF_NULL == classEnd )
                    {
                        m_valid = false;
                        break;
                    }
                    p = classEnd;
                }
            }
            ++i;
        }
    }
    if ( !m_valid )
        m_alternatives.clear();
    return m_valid;
}

/*-------------------------------------------------------------------------*/

const CString&
CGlobPattern::GetPattern( void ) const
{GUCEF_TRACE;

    return m_pattern;
}

/*-------------------------------------------------------------------------*/

bool
CGlobPattern::IsValid( void ) const
{GUCEF_TRACE;

    return m_valid;
}

/*-------------------------------------------------------------------------*/

bool
CGlobPattern::IsCaseSensitive( void ) const
{GUCEF_TRACE;

    return m_caseSensitive;
}

/*-------------------------------------------------------------------------*/

bool
CGlobPattern::IsMatch( const CString& text ) const
{GUCEF_TRACE;

    if ( !m_valid )
        return false;
    const CString subject = m_caseSensitive ? text : text.Lowercase();
    const char* t = subject.C_String();
    const char* tEnd = t + subject.ByteSize() - ( subject.ByteSize() > 0 ? 1 : 0 );
    if ( GUCEF_NULL == t )
        tEnd = t = "";

    CString::StringVector::const_iterator i = m_alternatives.begin();
    while ( i != m_alternatives.end() )
    {
        const char* p = (*i).C_String();
        const char* pEnd = p + (*i).ByteSize() - ( (*i).ByteSize() > 0 ? 1 : 0 );
        if ( GUCEF_NULL == p )
            pEnd = p = "";
        if ( MatchAlternative( p, pEnd, t, tEnd ) )
            return true;
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGlobPattern::IsMatch( const CString& pattern, const CString& text, bool caseSensitive )
{GUCEF_TRACE;

    return CGlobPattern( pattern, caseSensitive ).IsMatch( text );
}

/*-------------------------------------------------------------------------*/

bool
CGlobPattern::HasGlobCharacters( const CString& text )
{GUCEF_TRACE;

    return text.HasChar( '*' ) >= 0 || text.HasChar( '?' ) >= 0 || text.HasChar( '[' ) >= 0 || text.HasChar( '{' ) >= 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
