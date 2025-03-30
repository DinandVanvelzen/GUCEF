/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#include <string.h>
#include <assert.h>

#ifndef GUCEF_CORE_DVSTRUTILS_H
#include "dvstrutils.h"
#define GUCEF_CORE_DVSTRUTILS_H
#endif /* GUCEF_CORE_DVSTRUTILS_H ? */

#ifndef GUCEF_CORE_CUTF8STRING_H
#include "gucefCORE_CUtf8String.h"
#define GUCEF_CORE_CUTF8STRING_H
#endif /* GUCEF_CORE_CUTF8STRING_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#include "gucefCORE_CAsciiString.h"

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CAsciiString CAsciiString::Empty;
const CAsciiString::StringVector CAsciiString::EmptyStringVector;
const CAsciiString::StringSet CAsciiString::EmptyStringSet;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CAsciiString::CAsciiString( void )
        : m_string( NULL ) ,
          m_length( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsciiString::CAsciiString( const CAsciiString &src )
        : m_string( NULL ) ,
          m_length( 0 )
{GUCEF_TRACE;

    if ( src.m_length > 0 )
    {
        m_string = GUCEF_NEW char[ src.m_length+1 ];
        assert( m_string != NULL );
        m_length = src.m_length;
        memcpy( m_string, src.m_string, m_length+1 );
    }
}

/*-------------------------------------------------------------------------*/

CAsciiString::CAsciiString( const CUtf8String &src ) GUCEF_NOEXCEPT
    : m_string( GUCEF_NULL )
    , m_length( 0 )
{GUCEF_TRACE;

    *this = src.ForceToAscii();
}

/*-------------------------------------------------------------------------*/

CAsciiString::CAsciiString( const std::string& src ) GUCEF_NOEXCEPT
        : m_string( NULL ) ,
          m_length( 0 )
{GUCEF_TRACE;

    if ( src.size() > 0 )
    {
        m_length = (UInt32)src.size();
        m_string = GUCEF_NEW char[ m_length+1 ];
        assert( m_string != NULL );
        memcpy( m_string, src.c_str(), m_length+1 );
    }
}

/*-------------------------------------------------------------------------*/
#ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED

CAsciiString::CAsciiString( CAsciiString&& src ) GUCEF_NOEXCEPT
    : m_string( src.m_string ) ,
      m_length( src.m_length )
{GUCEF_TRACE;

    src.m_string = NULL;
    src.m_length = 0;
}

#endif
/*-------------------------------------------------------------------------*/

CAsciiString::CAsciiString( const char* src ) GUCEF_NOEXCEPT
        : m_string( GUCEF_NULL ) ,
          m_length( 0 )
{GUCEF_TRACE;

    if ( src != NULL )
    {
        m_length = (UInt32)strlen( src );
        m_string = GUCEF_NEW char[ m_length+1 ];
        assert( m_string );
        memcpy( m_string, src, m_length+1 );
    }
}

/*-------------------------------------------------------------------------*/

CAsciiString::CAsciiString( const char* src      ,
                            UInt32 length        ,
                            bool reexamineLength )  GUCEF_NOEXCEPT
        : m_string( NULL ) ,
          m_length( 0 )
{GUCEF_TRACE;

    if ( GUCEF_NULL != src && 0 < length )
    {
        if ( reexamineLength )
        {
            length = (UInt32) strnlen( src, length );
        }

        // dont add null term twice
        if ( '\0' == src[ length-1 ] )
        {
            m_length = length-1;
            m_string = GUCEF_NEW char[ length ];
            assert( m_string );
            memcpy( m_string, src, length );
        }
        else
        {
            m_length = length;
            m_string = GUCEF_NEW char[ length+1 ];
            assert( m_string );
            memcpy( m_string, src, length );
            m_string[ length ] = '\0';
        }
    }
}

/*-------------------------------------------------------------------------*/

CAsciiString::CAsciiString( const char src )
    : m_string( NULL ) ,
      m_length( 0 )
{GUCEF_TRACE;

    Set( &src, 1 );
}

/*-------------------------------------------------------------------------*/

CAsciiString::CAsciiString( const int NULLvalue )
    : m_string( NULL ) ,
      m_length( 0 )
{GUCEF_TRACE;

    assert( NULLvalue == (int) NULL );
}
/*-------------------------------------------------------------------------*/

CAsciiString::~CAsciiString()
{GUCEF_TRACE;

    GUCEF_DELETE []m_string;
    m_string = NULL;
    m_length = 0;
}

/*-------------------------------------------------------------------------*/

CAsciiString&
CAsciiString::operator=( const CAsciiString &src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        GUCEF_DELETE []m_string;
        m_string = NULL;
        m_length = src.m_length;

        if ( m_length > 0 )
        {
            m_string = GUCEF_NEW char[ m_length+1 ];
            assert( m_string );
            memcpy( m_string, src.m_string, m_length+1 );
        }
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CAsciiString&
CAsciiString::operator=( const std::string& src )
{GUCEF_TRACE;

    GUCEF_DELETE []m_string;
    m_string = NULL;
    m_length = (UInt32) src.size();

    if ( m_length > 0 )
    {
        m_string = GUCEF_NEW char[ m_length+1 ];
        assert( m_string );
        memcpy( m_string, src.c_str(), m_length+1 );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CAsciiString&
CAsciiString::operator=( const char *src )
{GUCEF_TRACE;

    // protect against self-assignment
    if ( src != m_string )
    {
        GUCEF_DELETE []m_string;
        m_string = NULL;
        m_length = 0;

        if ( src != NULL )
        {
            m_length = (UInt32)strlen( src );
            if ( m_length > 0 )
            {
                m_string = GUCEF_NEW char[ m_length+1 ];
                assert( m_string );
                memcpy( m_string, src, m_length+1 );
            }
        }
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiString::operator==( const char character ) const
{GUCEF_TRACE;          //@TODO  //@FIXME
       return *this == CAsciiString(  character);
    if ( m_length == 1 )
    {
        return *m_string == character;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiString::operator!=( const char character ) const
{GUCEF_TRACE;
                     //@TODO  //@FIXME
    return *this != CAsciiString(  character);

    //if ( m_length == 1 )
    //{
    //    return *m_string != character;
    //}
    //return false;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiString::operator!=( const int NULLvalue ) const
{GUCEF_TRACE;

    if ( NULLvalue == static_cast< int >( GUCEF_NULL ) )
    {
        return m_length > 0;
    }
    GUCEF_ASSERT_ALWAYS;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiString::operator==( const int NULLvalue ) const
{GUCEF_TRACE;

    if ( NULLvalue == static_cast< int >( GUCEF_NULL ) )
    {
        return m_length == 0;
    }
    GUCEF_ASSERT_ALWAYS;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiString::operator==( const char* other ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_string && GUCEF_NULL == other )
    {
        return true;
    }
    if ( GUCEF_NULL == m_string || GUCEF_NULL == other )
    {
        return false;
    }
    return strcmp( m_string ,
                   other    ) == 0;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiString::operator==( const CAsciiString &other ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_string && GUCEF_NULL == other.m_string )
    {
        return true;
    }
    if ( GUCEF_NULL == m_string || GUCEF_NULL == other.m_string )
    {
        return false;
    }
    return strcmp( m_string       ,
                   other.m_string ) == 0;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiString::operator!=( const CAsciiString& other ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_string && GUCEF_NULL == other.m_string )
    {
        return false;
    }
    if ( GUCEF_NULL == m_string || GUCEF_NULL == other.m_string )
    {
        return true;
    }
    return strcmp( m_string       ,
                   other.m_string ) != 0;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiString::operator!=( const char* other ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_string && GUCEF_NULL == other )
    {
        return false;
    }
    if ( GUCEF_NULL == m_string || GUCEF_NULL == other )
    {
        return true;
    }
    return strcmp( m_string ,
                   other   ) != 0;
}

/*-------------------------------------------------------------------------*/

char
CAsciiString::operator[]( const UInt32 index ) const
{GUCEF_TRACE;

    if ( index <= m_length )
        return m_string[ index ];
    return '\0';
}

/*-------------------------------------------------------------------------*/

char&
CAsciiString::operator[]( const UInt32 index )
{GUCEF_TRACE;

    static char outOfBoundsChar = '\0';
    if ( index <= m_length )
        return m_string[ index ];

    GUCEF_ASSERT_ALWAYS;
    return outOfBoundsChar;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiString::operator<( const CAsciiString& other ) const
{GUCEF_TRACE;

    if ( NULL != m_string && NULL != other.m_string )
    {
        return strcmp( m_string, other.m_string ) < 0;
    }
    if ( NULL == m_string && NULL != other.m_string )
    {
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiString::operator<=( const CAsciiString& other ) const
{GUCEF_TRACE;

    return ( *this < other ) || ( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CAsciiString::IsNULLOrEmpty( void ) const
{GUCEF_TRACE;

    return m_length == 0;
}

/*-------------------------------------------------------------------------*/

CAsciiString::operator std::string() const
{GUCEF_TRACE;

    if ( m_length > 0 )
    {
        return std::string( m_string, m_length );
    }
    return std::string();
}

/*-------------------------------------------------------------------------*/
/*
     Disabled in favor of std::string() conversion

CAsciiString::operator const char*() const
{GUCEF_TRACE;
        return m_string;
}
*/
/*-------------------------------------------------------------------------*/

void
CAsciiString::Set( const char *new_str ,
                  UInt32 len          )
{GUCEF_TRACE;

    if ( new_str != m_string )
    {
        GUCEF_DELETE []m_string;
        m_string = NULL;
        m_length = 0;

        if ( ( new_str != NULL ) &&
             ( len > 0 )          )
        {
            m_length = len;
            m_string = GUCEF_NEW char[ len+1 ];
            assert( m_string != NULL );
            memcpy( m_string, new_str, m_length+1 );
            m_string[ len ] = '\0';
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAsciiString::Set( const char* new_str )
{GUCEF_TRACE;

    Clear();
    if ( GUCEF_NULL != new_str )
        Set( new_str, (UInt32) strlen( new_str ) );
}

/*-------------------------------------------------------------------------*/

UInt32
CAsciiString::GetCharacterCount( const char searchChar ) const
{GUCEF_TRACE;

    UInt32 charCount = 0;
    for ( UInt32 i=0; i<m_length; ++i )
    {
        if ( m_string[ i ] == searchChar )
        {
            ++charCount;
        }
    }
    return charCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CAsciiString::GetCharactersCount( const char* searchChars ,
                                  UInt32 nrOfSearchChars  ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == searchChars )
        return 0;

    UInt32 charCount = 0;
    for ( UInt32 i=0; i<m_length; ++i )
    {
        for ( UInt32 n=0; n<nrOfSearchChars; ++n )
        {
            if ( m_string[ i ] == searchChars[ n ] )
            {
                ++charCount;
            }
        }
    }
    return charCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CAsciiString::GetNonMatchCharactersCount( const char* searchChars ,
                                          UInt32 nrOfSearchChars  ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == searchChars )
        return 0;

    UInt32 charCount = 0;
    for ( UInt32 i=0; i<m_length; ++i )
    {
        for ( UInt32 n=0; n<nrOfSearchChars; ++n )
        {
            if ( m_string[ i ] != searchChars[ n ] )
            {
                ++charCount;
            }
        }
    }
    return charCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CAsciiString::GetCharacterRepeatCount( const char searchChar ) const
{GUCEF_TRACE;

    UInt32 charRepeatCount = 0;
    for ( UInt32 i=0; i<m_length; ++i )
    {
        if ( m_string[ i ] == searchChar )
        {
            ++i;
            for ( i; i<m_length; ++i )
            {
                if ( m_string[ i ] == searchChar )
                    ++charRepeatCount;
                else
                    break;
            }
        }
    }
    return charRepeatCount;
}

/*-------------------------------------------------------------------------*/

void
CAsciiString::Append( const char *appendstr ,
                      UInt32 len            )
{GUCEF_TRACE;

    if ( ( appendstr != m_string ) &&
         ( appendstr != NULL )     &&
         ( len > 0 )                )
    {
        if ( m_length > 0 )
        {
            char* newString = GUCEF_NEW char[ m_length+len+1 ];
            assert( newString != NULL );
            memcpy( newString, m_string, m_length );
            memcpy( newString+m_length, appendstr, len );
            m_length = m_length+len;
            newString[ m_length ] = 0;
            GUCEF_DELETE []m_string;
            m_string = newString;
        }
        else
        {
            GUCEF_DELETE []m_string;
            m_length = len;
            m_string = GUCEF_NEW char[ m_length+1 ];
            assert( m_string );
            memcpy( m_string, appendstr, m_length );
            m_string[ m_length ] = 0;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAsciiString::Append( const char* appendstr )
{GUCEF_TRACE;

    if ( GUCEF_NULL != appendstr )
        Append( appendstr, (UInt32) strlen( appendstr ) );
}

/*-------------------------------------------------------------------------*/

CAsciiString&
CAsciiString::operator+=( const CAsciiString &other )
{GUCEF_TRACE;

    Append( other.m_string, other.m_length );
    return *this;
}

/*-------------------------------------------------------------------------*/

CAsciiString&
CAsciiString::operator+=( const char *other )
{GUCEF_TRACE;

    Append( other, UInt32( strlen( other ) ) );
    return *this;
}

/*-------------------------------------------------------------------------*/

CAsciiString&
CAsciiString::operator+=( char lastchar )
{GUCEF_TRACE;

    Append( &lastchar, 1 );
    return *this;
}

/*-------------------------------------------------------------------------*/

const char*
CAsciiString::C_String( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_string )
        return m_string;
    return "";
}

/*-------------------------------------------------------------------------*/

char*
CAsciiString::C_String( void )
{GUCEF_TRACE;

    return m_string;
}

/*-------------------------------------------------------------------------*/

char*
CAsciiString::Reserve( const UInt32 bufferSize, Int32 newLength )
{GUCEF_TRACE;

    GUCEF_DELETE []m_string;
    m_string = GUCEF_NULL;

    if ( bufferSize > 0 )
    {
        m_string = GUCEF_NEW char[ bufferSize ];
    }
    m_length = bufferSize;
    if ( newLength >= 0 )
        m_length = (UInt32) newLength;

    return m_string;
}

/*-------------------------------------------------------------------------*/

std::string
CAsciiString::STL_String( void ) const
{GUCEF_TRACE;

    if ( m_length > 0 )
    {
        return std::string( m_string, m_length );
    }
    return std::string();
}

/*-------------------------------------------------------------------------*/

UInt32
CAsciiString::Length( void ) const
{GUCEF_TRACE;

    return m_length;
}

/*-------------------------------------------------------------------------*/

UInt32
CAsciiString::ByteSize( void ) const
{GUCEF_TRACE;

    return m_length+1;
}

/*-------------------------------------------------------------------------*/

void
CAsciiString::SetLength( UInt32 newLength )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_string )
    {
        if ( m_length < newLength )
        {
            char* newString = GUCEF_NEW char[ newLength+1 ];
            memcpy( newString, m_string, m_length+1 );
            GUCEF_DELETE []m_string;
            m_string = newString;
            memset( m_string+m_length, 0, newLength-m_length );
            m_length = newLength;
            return;
        }

        memset( m_string+newLength, 0, m_length-newLength );
        m_length = newLength;
        return;
    }

    if ( newLength > 0 )
    {
        m_string = GUCEF_NEW char[ newLength+1 ];
        memset( m_string, 0, newLength );
    }
    m_length = newLength;
}

/*-------------------------------------------------------------------------*/

UInt32
CAsciiString::DetermineLength( void )
{
    if ( 0 != m_string )
    {
        m_length = (UInt32) strlen( m_string );
        return m_length;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::ReplaceChar( char oldchar ,
                           char newchar ) const
{GUCEF_TRACE;

    if ( m_length > 0 )
    {
        char* str = GUCEF_NEW char[ m_length+1 ];
        memcpy( str, m_string, m_length+1 );
        for ( UInt32 i=0; i<m_length; ++i )
        {
                if ( str[ i ] == oldchar )
                {
                        str[ i ] = newchar;
                }
        }
        CAsciiString newstr( str, m_length );
        GUCEF_DELETE []str;
        return newstr;
    }
    CAsciiString emptystr;
    return emptystr;
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::ReplaceChars( const char* oldchars ,
                            UInt32 nrOfOldChars  ,
                            char newchar         ) const
{GUCEF_TRACE;

    CAsciiString result( *this );

    if ( GUCEF_NULL == oldchars )
        return result;

    for ( UInt32 i=0; i<result.m_length; ++i )
    {
        for ( UInt32 n=0; n<nrOfOldChars; ++n )
        {
            if ( result.m_string[ i ] == oldchars[ n ] )
                result.m_string[ i ] = newchar;
        }

    }

    return result;
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::ReplaceSubstr( const CAsciiString& substr      ,
                             const CAsciiString& replacement ) const
{GUCEF_TRACE;

    CAsciiString testStr = *this;
    CAsciiString newStr;
    Int32 subStrIndex = 0;
    do
    {
        subStrIndex = testStr.HasSubstr( substr, true );
        if ( subStrIndex >= 0 )
        {
            newStr.Append( testStr.C_String(), subStrIndex );
            newStr += replacement;
            testStr = testStr.CutChars( (UInt32)subStrIndex + substr.Length(), true );
        }
    }
    while ( subStrIndex >= 0 );
    newStr += testStr;

    return newStr;
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::ReplaceEnvelopingSubstr( const CAsciiString& envelopPrefix     ,
                                       const CAsciiString& envelopPostfix    ,
                                       const CAsciiString& newEnvelopPrefix  ,
                                       const CAsciiString& newEnvelopPostfix ) const
{GUCEF_TRACE;

    CAsciiString resultStr;

    Int32 startIndex = 0;
    Int32 envSegIndex = this->HasSubstr( envelopPrefix, startIndex, true );
    while ( envSegIndex >= 0 )
    {
        if ( startIndex < envSegIndex )
        {
            resultStr += this->SubstrFromRange( startIndex, envSegIndex );
        }

        envSegIndex+=envelopPrefix.Length();
        CAsciiString envelopedSegment = this->SubstrToSubstr( envelopPostfix, envSegIndex, true );
        resultStr += newEnvelopPrefix + envelopedSegment + newEnvelopPostfix;

        startIndex = envSegIndex+envelopedSegment.Length()+envelopPostfix.Length();
        envSegIndex = this->HasSubstr( envelopPrefix, startIndex, true );
    }

    // append the remainder, if any
    resultStr += this->SubstrToIndex( startIndex, false );

    return resultStr;
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::CutEnvelopedSubstr( const CUtf8String& envelopPrefix  ,
                                  const CUtf8String& envelopPostfix ,
                                  UInt32 envelopedStrIndex          ) const
{GUCEF_TRACE;

    UInt32 currentEnvelopedStrIndex = 0;
    Int32 startIndex = 0;
    Int32 envSegIndex = HasSubstr( envelopPrefix, startIndex, true );
    while ( envSegIndex >= 0 )
    {
        envSegIndex += envelopPrefix.Length();
        envSegIndex = HasSubstr( envelopPostfix, envSegIndex, true );
        if ( envSegIndex >= 0 )
        {
            if ( currentEnvelopedStrIndex == envelopedStrIndex )
            {
                // This is the enveloped string we are looking to cut
                Int32 charCount = ( envSegIndex + envelopPostfix.Length() ) - startIndex;
                return CutChars( charCount, true, startIndex );
            }
            ++currentEnvelopedStrIndex;

            startIndex = envSegIndex + envelopPostfix.Length();
            envSegIndex = HasSubstr( envelopPrefix, startIndex, true );
        }
    }

    return *this;
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::ReplaceSubStr( UInt32 startIndex            ,
                             UInt32 length                ,
                             const CAsciiString& newSubstr ) const
{GUCEF_TRACE;

    // Sanity check on the given range
    if ( 0 == m_length ) return CAsciiString();
    if ( startIndex >= m_length ) startIndex = m_length -1;
    if ( startIndex+length >= m_length ) length = (m_length-1) - startIndex;

    CAsciiString prefix = this->SubstrFromRange( 0, startIndex );
    CAsciiString postfix = this->SubstrFromRange( startIndex+length+1, m_length );

    return prefix + newSubstr + postfix;
}

/*-------------------------------------------------------------------------*/

void
CAsciiString::Clear( void )
{GUCEF_TRACE;

    m_length = 0;
    GUCEF_DELETE []m_string;
    m_string = NULL;
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::SubstrToChar( char searchchar                ,
                            UInt32 startIndex              ,
                            bool frontToBack               ,
                            bool returnEmptyIfCharNotFound ) const
{GUCEF_TRACE;

    if ( startIndex < m_length )
    {
        if ( frontToBack )
        {
            for ( UInt32 i=startIndex; i<m_length; ++i )
            {
                if ( m_string[ i ] == searchchar )
                {
                    CAsciiString substr;
                    substr.Set( m_string+startIndex ,
                                i-startIndex        );
                    return substr;
                }
            }
            if ( returnEmptyIfCharNotFound )
                return CAsciiString();
            else
                return CAsciiString( *this );
        }

        for ( Int32 i=startIndex; i>=0; --i )
        {
            if ( m_string[ i ] == searchchar )
            {
                CAsciiString substr;
                substr.Set( m_string+i+1 ,
                            startIndex-i );
                return substr;
            }
        }
        if ( returnEmptyIfCharNotFound )
            return CAsciiString();
        else
            return CAsciiString( *this );
    }
    return CAsciiString();
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::SubstrToIndex( UInt32 index     ,
                             bool frontToBack ) const
{GUCEF_TRACE;

    if ( !frontToBack )
    {
         if ( index >= m_length )
            return CAsciiString();
         return SubstrFromRange( index, m_length );
    }

    if ( index >= m_length )
        return CAsciiString( m_string, m_length );
    return SubstrFromRange( 0, index );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::SubstrFromRange( UInt32 startIndex ,
                               UInt32 endIndex   ) const
{GUCEF_TRACE;

    if ( startIndex == endIndex ) return CAsciiString();

    // we want the user to be able to pass a range conveniently
    if  ( startIndex > endIndex )
    {
        // swap
        UInt32 swapIndex = startIndex;
        startIndex = endIndex;
        endIndex = swapIndex;
    }

    // gracefully protect against out-of-bounds index
    // keep in mind that endIndex is exclusive, thus out of bounds is endIndex
    // beyond the null terminator
    UInt32 maxEnd = endIndex > m_length ? m_length : endIndex;
    UInt32 maxStart = startIndex > m_length ? m_length : startIndex;

    // Check for 0 length string
    if ( maxEnd >= maxStart )
    {
        // make the new string using the given range
        return CAsciiString( m_string+maxStart, maxEnd-maxStart );
    }
    return CAsciiString();
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::SubstrToChar( char searchchar                ,
                            bool frontToBack               ,
                            bool returnEmptyIfCharNotFound ) const
{GUCEF_TRACE;

    if ( frontToBack )
    {
        return SubstrToChar( searchchar, 0, frontToBack, returnEmptyIfCharNotFound );
    }
    return SubstrToChar( searchchar, m_length-1, frontToBack, returnEmptyIfCharNotFound );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::Trim( bool frontToBack ) const
{GUCEF_TRACE;

    if ( m_length > 0 )
    {
        if ( frontToBack )
        {
            UInt32 charsToCut = 0;
            while ( m_string[ charsToCut ] == ' ' ||  m_string[ charsToCut ] == '\t' )
            {
                ++charsToCut;
            }
            return CutChars( charsToCut, frontToBack );
        }

        UInt32 i=m_length-1;
        UInt32 charsToCut = 0;
        while ( i>=0 && ( m_string[ i ] == ' ' ||  m_string[ i ] == '\t' ) )
        {
            --i;
            ++charsToCut;
        }
        return CutChars( charsToCut, frontToBack );
    }
    return CAsciiString( *this );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::SubstrToSubstr( const CAsciiString& searchstr ,
                              bool startfront              ) const
{GUCEF_TRACE;

    if ( searchstr.Length() > m_length || 0 == searchstr.Length() || 0 == m_length )
    {
        return *this;
    }

    if ( startfront )
    {
        return SubstrToSubstr( searchstr, 0, startfront );
    }

    return SubstrToSubstr( searchstr, m_length-1, startfront );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::SubstrToSubstr( const CAsciiString& searchstr ,
                              UInt32 startIndex            ,
                              bool startfront              ) const
{GUCEF_TRACE;

    UInt32 slen = searchstr.Length();
    if ( slen > m_length || 0 == slen || 0 == m_length )
    {
        return *this;
    }
    if ( startIndex >= m_length )
    {
        // cap the index to disallow over/under flows
        startIndex = m_length-1;
    }

    if ( startfront )
    {
        UInt32 max = m_length - slen;
        for ( UInt32 i=startIndex; i<=max; ++i )
        {
            if ( memcmp( m_string+i, searchstr.m_string, slen ) == 0 )
            {
                return CAsciiString( m_string+startIndex, i-startIndex );
            }
        }
        return *this;
    }

    UInt32 max = startIndex - slen;
    for ( Int32 i=max; i>=0; --i )
    {
        if ( memcmp( m_string+i, searchstr.m_string, slen ) == 0 )
        {
            CAsciiString substr;
            substr.Set( m_string+i+slen                    ,
                        m_length-(i+slen) - ((m_length-1)-startIndex) );
            return substr;
        }
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::RemoveChar( const char charToRemove ) const
{GUCEF_TRACE;

    CAsciiString newString;
    for ( UInt32 i=0; i<m_length; ++i )
    {
        if ( m_string[ i ] != charToRemove )
        {
            newString += m_string[ i ];
        }
    }
    return newString;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiString::HasRepeatingChar( const char charToCheck ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_length; ++i )
    {
        if ( m_string[ i ] == charToCheck )
        {
            if ( ( i+1 < m_length ) &&
                    ( m_string[ i+1 ] == charToCheck ) )
            {
                return true;
            }
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::CompactRepeatingChar( const char charToCompact ) const
{GUCEF_TRACE;

    char* newString = GUCEF_NEW char[ m_length+1 ];
    UInt32 newStringLength = 0;

    for ( UInt32 i=0; i<m_length; ++i )
    {
        newString[ newStringLength ] = m_string[ i ];
        ++newStringLength;

        if ( m_string[ i ] == charToCompact )
        {
            while ( ( i+1 < m_length ) &&
                    ( m_string[ i+1 ] == charToCompact ) )
            {
                ++i;
            }
        }
    }

    CAsciiString returnStr( newString, newStringLength );
    GUCEF_DELETE []newString;
    return returnStr;
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::CutChars( UInt32 charcount   ,
                        bool startfront    ,
                        UInt32 startOffset ) const
{GUCEF_TRACE;

    if ( startfront )
    {
        if ( 0 == startOffset )
        {
            if ( charcount < m_length )
            {
                return CAsciiString( m_string+charcount, m_length-charcount );
            }

            return CAsciiString();
        }

        return CAsciiString( m_string, startOffset ) + CAsciiString( m_string+startOffset+charcount, m_length-startOffset-charcount );
    }

    if ( 0 == startOffset )
    {
        if ( charcount < m_length )
        {
            return CAsciiString( m_string, m_length-charcount );
        }

        return CAsciiString();
    }

    return CAsciiString( m_string, m_length-startOffset-charcount ) + CAsciiString( m_string+m_length-charcount, charcount );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::Lowercase( void ) const
{GUCEF_TRACE;

    if ( m_length > 0 )
    {
        char* lcstr = GUCEF_NEW char[ m_length+1 ];
        memcpy( lcstr, m_string, m_length+1 );
        for ( UInt32 i=0; i<m_length; ++i )
        {
            if ( ( lcstr[ i ] > 64 ) &&
                 ( lcstr[ i ] < 91 ) )
            {
                lcstr[ i ] += 32;
            }
        }
        CAsciiString lower( lcstr, m_length );
        GUCEF_DELETE []lcstr;
        return lower;
    }
    return CAsciiString();
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::Uppercase( void ) const
{GUCEF_TRACE;

    if ( m_length > 0 )
    {
        char* ucstr = GUCEF_NEW char[ m_length+1 ];
        memcpy( ucstr, m_string, m_length+1 );
        for ( UInt32 i=0; i<m_length; ++i )
        {
            if ( ( ucstr[ i ] > 96 ) &&
                 ( ucstr[ i ] < 123 ) )
            {
                ucstr[ i ] -= 32;
            }
        }
        CAsciiString upper( ucstr, m_length );
        GUCEF_DELETE []ucstr;
        return upper;
    }
    return CAsciiString();
}

/*-------------------------------------------------------------------------*/

Int32
CAsciiString::HasChar( char searchchar         ,
                       const UInt32 startIndex ,
                       bool frontToBack        ) const
{GUCEF_TRACE;

    if ( frontToBack )
    {
        for ( UInt32 i=startIndex; i<m_length; ++i )
        {
            if ( m_string[ i ] == searchchar )
            {
                return i;
            }
        }
        return -1;
    }

    UInt32 max = startIndex > m_length-1 ? m_length-1 : startIndex;
    for ( Int32 i=max; i>=0; --i )
    {
        if ( m_string[ i ] == searchchar )
        {
            return i;
        }
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CAsciiString::HasChar( char searchchar  ,
                       bool startfront  ) const
{GUCEF_TRACE;

    if ( startfront )
    {
        return HasChar( searchchar, 0, startfront );
    }

    if ( m_length > 0 )
    {
        return HasChar( searchchar, m_length-1, startfront );
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

CAsciiString::StringSet
CAsciiString::ParseUniqueElements( char seperator        ,
                                   bool addEmptyElements ) const
{GUCEF_TRACE;

    return ParseUniqueElements( m_string, m_length, seperator, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

CAsciiString::StringVector
CAsciiString::ParseElements( char seperator        ,
                             bool addEmptyElements ) const
{GUCEF_TRACE;

    return ParseElements( m_string, m_length, seperator, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

CAsciiString::StringVector 
CAsciiString::ParseElements( const char* bufferPtr  ,
                             UInt32 bufferSize      ,
                             char seperator         ,
                             bool addEmptyElements  )
{GUCEF_TRACE;

    if ( bufferSize > 0 )
    {
        StringVector list;
        CAsciiString entry;
        UInt32 last = 0;
        for ( UInt32 i=0; i<bufferSize; ++i )
        {
            if ( bufferPtr[ i ] == seperator )
            {
                UInt32 stringLength = i-last;
                if ( ( 0 == stringLength && addEmptyElements ) ||
                     ( stringLength > 0 ) )
                {
                    entry.Set( bufferPtr+last ,
                               stringLength   );
                    list.push_back( entry );
                }
                last = i+1;
            }
        }

        // add last item
        UInt32 stringLength = bufferSize-last;
        if ( ( 0 == stringLength && addEmptyElements ) ||
             ( stringLength > 0 ) )
        {
            entry.Set( bufferPtr+last ,
                       stringLength   );
            list.push_back( entry );
        }
        return list;
    }
    return StringVector();
}

/*-------------------------------------------------------------------------*/

CAsciiString::StringSet 
CAsciiString::ParseUniqueElements( const char* bufferPtr ,
                                   UInt32 bufferSize     ,
                                   char seperator        ,
                                   bool addEmptyElements )
{GUCEF_TRACE;

    if ( bufferSize > 0 )
    {
        StringSet list;
        CAsciiString entry;
        UInt32 last = 0;
        for ( UInt32 i=0; i<bufferSize; ++i )
        {
            if ( bufferPtr[ i ] == seperator )
            {
                UInt32 stringLength = i-last;
                if ( ( 0 == stringLength && addEmptyElements ) ||
                     ( stringLength > 0 ) )
                {
                    entry.Set( bufferPtr+last ,
                               stringLength   );
                    list.insert( entry );
                }
                last = i+1;
            }
        }

        // add last item
        UInt32 stringLength = bufferSize-last;
        if ( ( 0 == stringLength && addEmptyElements ) ||
             ( stringLength > 0 ) )
        {
            entry.Set( bufferPtr+last ,
                       stringLength );
            list.insert( entry );
        }
        return list;
    }
    return StringSet();
}

/*-------------------------------------------------------------------------*/

UInt32
CAsciiString::FindMaxSubstrEquality( const CAsciiString& searchStr ,
                                     const UInt32 startOffset     ,
                                     bool startFront              ,
                                     bool isCaseSentive           ) const
{GUCEF_TRACE;

    // Sanity check on the startOffset
    if ( (Int32)m_length - startOffset > 0 )
    {
        // Here we want to be able to support case insensitive compares
        // without having to suffer the performance penalty of making string lowercase
        // when not doing case insensitive compares. This we do some pointer magic to
        // avoid this overhead when we can
        CAsciiString lowercaseSearchStrStorage;
        CAsciiString lowercaseThisStrStorage;
        const CAsciiString* theSearchStr = &searchStr;
        const CAsciiString* thisStr = this;

        if ( !isCaseSentive )
        {
            // Caller wants a case-insensitive compare,..
            // take the performance hit
            lowercaseSearchStrStorage = searchStr.Lowercase();
            theSearchStr = &lowercaseSearchStrStorage;
            lowercaseThisStrStorage = Lowercase();
            thisStr = &lowercaseThisStrStorage;
        }

        // Get the smallest of the 2 buffer limits
        UInt32 max = theSearchStr->Length();
        if ( thisStr->m_length - startOffset < max )
        {
            max = thisStr->m_length - startOffset;
        }

        if ( startFront )
        {
            // Loop trough the buffer growing our comparison string
            UInt32 subLength=1;
            while ( subLength<=max )
            {
                if ( memcmp( thisStr->m_string+startOffset, theSearchStr->m_string, subLength ) != 0 )
                {
                    // Reached the maximum equality length
                    return subLength-1;
                }
                ++subLength;
            }
            return subLength-1;
        }
        else
        {
            // Loop trough the buffer growing our comparison string
            const char* string = thisStr->m_string + thisStr->m_length - startOffset;
            const char* otherString = theSearchStr->m_string + theSearchStr->m_length;

            UInt32 subLength=1;
            while ( subLength<=max )
            {
                if ( memcmp( string-subLength, otherString-subLength, subLength ) != 0 )
                {
                    // Reached the maximum equality length
                    return subLength-1;
                }
                ++subLength;
            }
            return subLength-1;
        }
    }

    // Unable to find an equality
    return 0;
}

/*-------------------------------------------------------------------------*/

Int32
CAsciiString::HasSubstr( const CAsciiString& substr ,
                         Int32 startIndex          ,
                         bool startfront           ) const
{GUCEF_TRACE;

    // Sanity check on the startindex range
    if ( startIndex < 0 || (UInt32)startIndex > m_length ) return -1;

    UInt32 subStrLength = substr.Length();
    if ( subStrLength > m_length )
    {
        return -1;
    }

    if ( startfront )
    {
        UInt32 max = m_length - subStrLength;
        for ( UInt32 i=startIndex; i<=max; ++i )
        {
            if ( memcmp( m_string+i, substr.m_string, subStrLength ) == 0 )
            {
                return i;
            }
        }
        return -1;
    }

    Int32 max = (Int32) ( m_length - subStrLength );
    if ( startIndex-(Int32)subStrLength < max )
    {
        max = startIndex-subStrLength;
    }
    for ( Int32 i=max; i>=0; --i )
    {
        if ( memcmp( m_string+i, substr.m_string, subStrLength ) == 0 )
        {
            return i;
        }
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CAsciiString::HasSubstr( const CAsciiString& substr ,
                         bool startfront           ) const
{GUCEF_TRACE;

    if ( startfront )
    {
        return HasSubstr( substr, 0, startfront );
    }
    if ( m_length > 0 )
    {
        return HasSubstr( substr, m_length, startfront );
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiString::WildcardEquals( const CAsciiString& strWithWildcards    ,
                              const char wildCardToken /* = '*' */   ,
                              const bool caseSensitive /* = true */  ,
                              const bool biDirectional /* = false */ ) const
{GUCEF_TRACE;

    if ( biDirectional )
    {
        if ( strWithWildcards.WildcardEquals( *this, wildCardToken, caseSensitive, false ) )
            return true;
    }

    if ( strWithWildcards == wildCardToken || *this == wildCardToken )
        return true;

    CAsciiString::StringVector segs = strWithWildcards.ParseElements( wildCardToken, false );
    Int32 lastSeg = 0;
    CAsciiString::StringVector::iterator i = segs.begin();
    while ( i != segs.end() )
    {
        lastSeg = HasSubstr( (*i), lastSeg, true );
        if ( 0 > lastSeg )
            return false;
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiString::WildcardEquals( const StringSet& strsWithWildcards     ,
                              const Int32 wildCardToken /* = '*' */  ,
                              const bool caseSensitive /* = true */  ,
                              const bool biDirectional /* = false */ ) const
{GUCEF_TRACE;

    if ( strsWithWildcards.empty() )
        return WildcardEquals( Empty, wildCardToken, caseSensitive, biDirectional );

    StringSet::const_iterator i = strsWithWildcards.begin();
    while ( i != strsWithWildcards.end() )
    {
        if ( WildcardEquals( (*i), wildCardToken, caseSensitive, biDirectional ) )
            return true;
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiString::Equals( const CAsciiString& otherStr           ,
                      const bool caseSensitive /* = true */ ) const
{GUCEF_TRACE;

    if ( caseSensitive )
    {
        return *this == otherStr;
    }
    else
    {
        return Lowercase() == otherStr.Lowercase();
    }
}

/*-------------------------------------------------------------------------*/

bool
CAsciiString::NotEquals( const CAsciiString& otherStr           ,
                         const bool caseSensitive /* = true */ ) const
{GUCEF_TRACE;

    if ( caseSensitive )
    {
        return *this != otherStr;
    }
    else
    {
        return Lowercase() != otherStr.Lowercase();
    }
}

/*-------------------------------------------------------------------------*/

void
CAsciiString::Scan( const char* newStr     ,
                    const UInt32 maxLength )
{GUCEF_TRACE;

    if ( newStr != NULL )
    {
        for ( UInt32 i=0; i<maxLength; ++i )
        {
            if ( newStr[ i ] == '\0' )
            {
                // null-terminator found
                Set( newStr ,
                     i      );
                return;
            }
        }

        // If we get here then no null-terminator was found in the buffer
        // before the maximum was reached
        Set( newStr    ,
             maxLength );
    }
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::Combine( const StringVector& elements, char seperator ) const
{GUCEF_TRACE;

    CAsciiString currentStr( *this );
    if ( !currentStr.IsNULLOrEmpty() && !elements.empty() )
        currentStr += seperator;

    StringVector::const_iterator i = elements.begin();
    while ( i != elements.end() )
    {
        if ( !(*i).IsNULLOrEmpty() )
            currentStr += (*i);
        ++i;
        if ( i != elements.end() && !(*i).IsNULLOrEmpty() )
            currentStr += seperator;
    }
    return currentStr;
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::Combine( const StringSet& elements, char seperator ) const
{GUCEF_TRACE;

    CAsciiString currentStr( *this );
    if ( !currentStr.IsNULLOrEmpty() && !elements.empty() )
        currentStr += seperator;

    StringSet::const_iterator i = elements.begin();
    while ( i != elements.end() )
    {
        if ( !(*i).IsNULLOrEmpty() )
            currentStr += (*i);
        ++i;
        if ( i != elements.end() && !(*i).IsNULLOrEmpty() )
            currentStr += seperator;
    }
    return currentStr;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiString::IsFormattingValid( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_string )
    {
        // Check if an invalid ASCII code point is found
        for ( UInt32 i=0; i<m_length; ++i )
        {
            // Even though its a signed value ASCII is from 0-127
            Int8 testCp = (Int8) m_string[ i ];
            if ( 0 > testCp )
                return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::ReadLine( CIOAccess* io )
{GUCEF_TRACE;

    if ( GUCEF_NULL != io )
    {
        CAsciiString resultStr;
        while ( !io->Eof() )
        {
            char currentChar = io->GetChar();
            if ( EOF != currentChar )
            {
                if ( currentChar == '\0'        ||
                     currentChar == '\n'        ||
                     currentChar == '\r'        ||
                     currentChar == GUCEF_DOS_EOF_CHAR )
                {
                    break;
                }

                resultStr += currentChar;
            }
            else
            {
                return resultStr;
            }
        }
        return resultStr;
    }

    return CAsciiString();
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::ReadString( CIOAccess* io )
{GUCEF_TRACE;

    if ( GUCEF_NULL != io )
    {
        CAsciiString resultStr;
        while ( !io->Eof() )
        {
            char currentChar = io->GetChar();
            if ( EOF != currentChar )
            {
                if ( currentChar == '\0'          ||
                     currentChar == '\n'          ||
                     currentChar == '\r'          ||
                     currentChar ==  GUCEF_DOS_EOF_CHAR ||
                     currentChar == '\t'          ||
                     currentChar == ' '            )
                {
                    break;
                }

                resultStr += currentChar;
            }
            else
            {
                return resultStr;
            }
        }
        return resultStr;
    }

    return CAsciiString();
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::operator+( const CAsciiString& addition ) const
{GUCEF_TRACE;

    CAsciiString tmp( *this );
    tmp += addition;
    return tmp;
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::operator+( const char* addition ) const
{GUCEF_TRACE;
        CAsciiString tmp( *this );
        tmp += addition;
        return tmp;
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiString::operator+( const char addition ) const
{GUCEF_TRACE;

    CAsciiString tmp( *this );
    tmp += addition;
    return tmp;
}

/*-------------------------------------------------------------------------*/

CAsciiString
operator+( const char* lhs    ,
           const CAsciiString& rhs )
{GUCEF_TRACE;
        CAsciiString tmp( lhs );
        tmp += rhs;
        return tmp;
}

/*-------------------------------------------------------------------------*/

bool
operator==( const char* lhs    ,
            const CAsciiString& rhs )
{GUCEF_TRACE;
        return rhs == lhs;
}

/*-------------------------------------------------------------------------*/

bool
operator!=( const char* lhs    ,
            const CAsciiString& rhs )
{GUCEF_TRACE;
        return rhs != lhs;
}

/*-------------------------------------------------------------------------*/

CAsciiString
operator+( const char lhs     ,
           const CAsciiString& rhs )
{GUCEF_TRACE;

    return CAsciiString( &lhs, 1 ) + rhs;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
