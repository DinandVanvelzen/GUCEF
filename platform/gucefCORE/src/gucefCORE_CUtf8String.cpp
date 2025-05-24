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

#include "gucefCORE_utf8.h"

#ifndef GUCEF_CORE_DVSTRUTILS_H
#include "dvstrutils.h"
#define GUCEF_CORE_DVSTRUTILS_H
#endif /* GUCEF_CORE_DVSTRUTILS_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_CASCIISTRING_H
#include "gucefCORE_CAsciiString.h"
#define GUCEF_CORE_CASCIISTRING_H
#endif /* GUCEF_CORE_CASCIISTRING_H ? */

#include "gucefCORE_CUtf8String.h"

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

const CUtf8String CUtf8String::Empty;
const CUtf8String::StringVector CUtf8String::EmptyStringVector;
const CUtf8String::StringSet CUtf8String::EmptyStringSet;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CUtf8String::CUtf8String( void )
    : m_string( GUCEF_NULL )
    , m_length( 0 )
    , m_byteSize( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUtf8String::CUtf8String( const CUtf8String& src )
    : m_string( GUCEF_NULL )
    , m_length( 0 )
    , m_byteSize( 0 )
{GUCEF_TRACE;

    Set( src.C_String(), src.ByteSize(), src.Length() );
}

/*-------------------------------------------------------------------------*/

CUtf8String::CUtf8String( const CAsciiString& src )
    : m_string( GUCEF_NULL )
    , m_length( 0 )
    , m_byteSize( 0 )
{GUCEF_TRACE;

    Set( src.C_String(), src.ByteSize(), src.Length() );
}

/*-------------------------------------------------------------------------*/

CUtf8String::CUtf8String( const std::string& src )
    : m_string( GUCEF_NULL )
    , m_length( 0 )
    , m_byteSize( 0 )
{GUCEF_TRACE;

    if ( src.size() > 0 )
    {
        Set( src.c_str(), (UInt32) ( sizeof( std::string::value_type ) * ( src.size() + 1 ) ) );
    }
}

/*-------------------------------------------------------------------------*/
#ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED

CUtf8String::CUtf8String( CUtf8String&& src ) GUCEF_NOEXCEPT
    : m_string( src.m_string )
    , m_length( src.m_length )
    , m_byteSize( src.m_byteSize )
{GUCEF_TRACE;

    src.m_string = GUCEF_NULL;
    src.m_length = 0;
    src.m_byteSize = 0;
}

#endif
/*-------------------------------------------------------------------------*/

CUtf8String::CUtf8String( const char* src )
    : m_string( GUCEF_NULL )
    , m_length( 0 )
    , m_byteSize( 0 )
{GUCEF_TRACE;

    Set( src );
}

/*-------------------------------------------------------------------------*/

CUtf8String::CUtf8String( const char *src        ,
                          UInt32 byteSize        ,
                          bool reexamineByteSize )
    : m_string( GUCEF_NULL )
    , m_length( 0 )
    , m_byteSize( 0 )
{GUCEF_TRACE;

    Set( src, byteSize, reexamineByteSize );
}

/*-------------------------------------------------------------------------*/

CUtf8String::CUtf8String( const char* src        ,
                          UInt32 byteSize        ,
                          UInt32 length          ,
                          bool reexamineByteSize )
    : m_string( GUCEF_NULL )
    , m_length( 0 )
    , m_byteSize( 0 )
{GUCEF_TRACE;

    Set( src, byteSize, length, reexamineByteSize );
}

/*-------------------------------------------------------------------------*/

CUtf8String::CUtf8String( const char src )
    : m_string( GUCEF_NULL )
    , m_length( 0 )
    , m_byteSize( 0 )
{GUCEF_TRACE;

    Set( &src, (UInt32) sizeof( src ) );
}

/*-------------------------------------------------------------------------*/

CUtf8String::CUtf8String( const wchar_t* src )
    : m_string( GUCEF_NULL )
    , m_length( 0 )
    , m_byteSize( 0 )
{GUCEF_TRACE;

    Set( src );
}

/*-------------------------------------------------------------------------*/

CUtf8String::CUtf8String( const wchar_t* src         ,
                          UInt32 lengthInWCodePoints ,
                          bool reexamineByteSize     )
    : m_string( GUCEF_NULL )
    , m_length( 0 )
    , m_byteSize( 0 )
{GUCEF_TRACE;

    Set( src, lengthInWCodePoints, reexamineByteSize );
}

/*-------------------------------------------------------------------------*/

CUtf8String::CUtf8String( const wchar_t* src         ,
                          UInt32 byteSize            ,
                          UInt32 lengthInWCodePoints ,
                          bool reexamineByteSize     )
    : m_string( GUCEF_NULL )
    , m_length( 0 )
    , m_byteSize( 0 )
{GUCEF_TRACE;

    Set( src, byteSize, lengthInWCodePoints, reexamineByteSize );
}

/*-------------------------------------------------------------------------*/

CUtf8String::CUtf8String( const Int32 NULLvalueOrUtf32 )
    : m_string( GUCEF_NULL )
    , m_length( 0 )
    , m_byteSize( 0 )
{GUCEF_TRACE;

    Set( (const char*) &NULLvalueOrUtf32, sizeof( NULLvalueOrUtf32 ) );
}

/*-------------------------------------------------------------------------*/

CUtf8String::CUtf8String( const UInt32 utf32 )
    : m_string( GUCEF_NULL )
    , m_length( 0 )
    , m_byteSize( 0 )
{GUCEF_TRACE;

    Set( (const char*) &utf32, sizeof( utf32 ) );
}

/*-------------------------------------------------------------------------*/

CUtf8String::~CUtf8String()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

Int32
CUtf8String::EncodeUtf32CodePointToUtf8( const UInt32 utf32CodePoint ,
                                         char* outUtf8Buffer         ,
                                         UInt32 outUtf8BufferSize    )
{GUCEF_TRACE;

    if ( GUCEF_NULL != outUtf8Buffer && outUtf8BufferSize > 0 )
    {
        memset( outUtf8Buffer, 0, outUtf8BufferSize );
        const char* newPos = (const char*) utf8catcodepoint( outUtf8Buffer, utf32CodePoint, outUtf8BufferSize );
        if ( GUCEF_NULL != newPos )
            return (Int32) ( newPos - outUtf8Buffer );
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CUtf8String::EncodeUtf8CodePointToUtf32( const char* utf8Buffer        ,
                                         const UInt32 utf8BufferSize   ,
                                         UInt32& outUtf32CodePoint     )
{GUCEF_TRACE;

    outUtf32CodePoint = 0;
    if ( GUCEF_NULL != utf8Buffer && utf8BufferSize > 0 )
    {
        const char* newPos = (const char*) utf8codepoint( utf8Buffer, &outUtf32CodePoint );
        if ( GUCEF_NULL != newPos )
            return (Int32) ( newPos - utf8Buffer );
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

CUtf8String&
CUtf8String::operator=( const CUtf8String &src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        Set( src.m_string, src.m_byteSize, src.m_length, false );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CUtf8String::Set( const CUtf8String& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        Set( src.m_string, src.m_byteSize, src.m_length, false );
    }
}

/*-------------------------------------------------------------------------*/

CUtf8String&
CUtf8String::operator=( const std::string& src )
{GUCEF_TRACE;

    Set( src.c_str(), (UInt32) ( sizeof( std::string::value_type ) * ( src.size() + 1 ) ) );
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf8String&
CUtf8String::operator=( const char* src )
{GUCEF_TRACE;

    Set( src );
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CUtf8String::operator==( const char character ) const
{GUCEF_TRACE;

    return *this == CUtf8String( character );
}

/*-------------------------------------------------------------------------*/

bool
CUtf8String::operator!=( const char character ) const
{GUCEF_TRACE;

    return *this != CUtf8String( character );
}

/*-------------------------------------------------------------------------*/

bool
CUtf8String::operator!=( const UInt32 NULLvalueOrUtf32 ) const
{GUCEF_TRACE;

    if ( NULLvalueOrUtf32 == 0 )
    {
        return m_length > 0;
    }
    if ( GUCEF_NULL == m_string )
    {
        return false;
    }

    // Treat as UTF32 code point, compress as needed for compatibility with UTF8 followed by
    // the actual compare
    char utfTempBuffer[ 5 ] = { 0, 0, 0, 0, 0 };
    void* newPos = utf8catcodepoint( utfTempBuffer, NULLvalueOrUtf32, 5 );
    if ( GUCEF_NULL != newPos )
        return 0 != utf8cmp( m_string, utfTempBuffer );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUtf8String::operator!=( const Int32 NULLvalue ) const
{GUCEF_TRACE;

    return (*this) != static_cast< UInt32 >( NULLvalue );
}

/*-------------------------------------------------------------------------*/

bool
CUtf8String::operator==( const UInt32 NULLvalueOrUtf32 ) const
{GUCEF_TRACE;

    if ( NULLvalueOrUtf32 == 0 )
    {
        return m_length == 0;
    }
    if ( GUCEF_NULL == m_string )
    {
        return false;
    }

    // Treat as UTF32 code point, compress as needed for compatibility with UTF8 followed by
    // the actual compare
    char utfTempBuffer[ 5 ] = { 0, 0, 0, 0, 0 };
    void* newPos = utf8catcodepoint( utfTempBuffer, NULLvalueOrUtf32, 5 );
    if ( GUCEF_NULL != newPos )
        return 0 == utf8cmp( m_string, utfTempBuffer );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CUtf8String::operator==( const Int32 NULLvalue ) const
{GUCEF_TRACE;

    return (*this) == static_cast< UInt32 >( NULLvalue );
}

/*-------------------------------------------------------------------------*/

bool
CUtf8String::operator==( const char *other ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_string && GUCEF_NULL == other )
    {
        return true;
    }
    if ( GUCEF_NULL == m_string || GUCEF_NULL == other )
    {
        return false;
    }
    return 0 == utf8cmp( m_string, other );
}

/*-------------------------------------------------------------------------*/

bool
CUtf8String::operator==( const CUtf8String &other ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_string && GUCEF_NULL == other.m_string )
    {
        return true;
    }
    if ( GUCEF_NULL == m_string )
    {
        // special case handling for comparison to "" or String::Empty
        if ( 0 != other.m_length )
            return false;
        return true;
    }
    if ( GUCEF_NULL == other.m_string )
    {
        // special case handling for comparison to "" or String::Empty
        if ( 0 != m_length )
            return false;
        return true;
    }
    return 0 == utf8cmp( m_string, other.m_string );
}

/*-------------------------------------------------------------------------*/

bool
CUtf8String::operator!=( const CUtf8String &other ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_string && GUCEF_NULL == other.m_string )
    {
        return false;
    }
    if ( GUCEF_NULL == m_string )
    {
        // special case handling for comparison to "" or String::Empty
        if ( 0 != other.m_length )
            return true;
        return false;
    }
    if ( GUCEF_NULL == other.m_string )
    {
        // special case handling for comparison to "" or String::Empty
        if ( 0 != m_length )
            return true;
        return false;
    }
    return 0 != utf8cmp( m_string, other.m_string );
}

/*-------------------------------------------------------------------------*/

bool
CUtf8String::operator!=( const char* other ) const
{GUCEF_TRACE;

    // Comparison against ASCII encoded string to UTF8 string
    // This works transparently due to UTF8 being a superset

    if ( GUCEF_NULL == m_string && GUCEF_NULL == other )
    {
        return false;
    }
    if ( GUCEF_NULL == m_string || GUCEF_NULL == other )
    {
        return true;
    }
    return 0 != utf8cmp( m_string, other );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8String::operator[]( const UInt32 index ) const
{GUCEF_TRACE;

    return CodepointAtIndex( index );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8String::CodepointAtIndex( const UInt32 index ) const
{GUCEF_TRACE;

    if ( index >= m_length )
        return 0;

    const char* cpPos = m_string;
    UInt32 cp = 0;
    UInt32 i=0;
    do
    {
        cpPos = (const char*) utf8codepoint( cpPos, &cp );
        ++i;
    }
    while ( i<=index );
    return cp;
}

/*-------------------------------------------------------------------------*/

const char*
CUtf8String::CodepointPtrAtIndex( const char* str        ,
                                  const UInt32 byteSize  ,
                                  const UInt32 length    ,
                                  const UInt32 index     ,
                                  UInt32& bytesFromStart )
{GUCEF_TRACE;

    bytesFromStart = 0;

    // We allow up to 'length' because the null terminator is also a
    // valid code point. Its also needed to support index-exclusive-ranging
    if ( index > length )
        return GUCEF_NULL;

    const char* cpPos = str;
    UInt32 cp = 0; UInt32 i=0;
    while ( i<index && bytesFromStart < byteSize )
    {
        const char* newCpPos = (const char*) utf8codepoint( cpPos, &cp );
        if ( 0 == cp && i < index )
        {
            // there is a disconnect between the given bytesize & length vs where we found a null terminator
            // we would not have found one in the given range during typical operations
            return GUCEF_NULL;
        }
        bytesFromStart += (UInt32) ( newCpPos - cpPos );
        cpPos = newCpPos;
        ++i;
    }

    // We protect against generating offsets beyond the available buffer
    // we do allow the trailing null terminator to be accessed
    if ( cpPos > str+byteSize )
    {        
        return GUCEF_NULL;
    }
    return cpPos;
}

/*-------------------------------------------------------------------------*/

const char*
CUtf8String::CodepointPtrAtIndex( const UInt32 index, UInt32& bytesFromStart ) const
{GUCEF_TRACE;

    return CodepointPtrAtIndex( m_string, m_byteSize, m_length, index, bytesFromStart );
}

/*-------------------------------------------------------------------------*/

const char*
CUtf8String::NextCodepointPtr( const char* currentCpPos ,
                               UInt32& currentUtf32Cp   ) const
{GUCEF_TRACE;

    if ( currentCpPos < (m_string+m_byteSize) &&
         currentCpPos >= m_string              )
    {
        return (const char*) utf8codepoint( currentCpPos, &currentUtf32Cp );
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const char*
CUtf8String::NextCodepointPtr( const char* currentCpPos ) const
{GUCEF_TRACE;

    UInt32 currentUtf32Cp = 0;
    return NextCodepointPtr( currentCpPos, currentUtf32Cp );
}

/*-------------------------------------------------------------------------*/

bool
CUtf8String::operator<( const CUtf8String& other ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_string && GUCEF_NULL != other.m_string )
    {
        return utf8cmp( m_string, other.m_string ) < 0;
    }
    if ( GUCEF_NULL == m_string && GUCEF_NULL != other.m_string )
    {
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CUtf8String::operator<=( const CUtf8String& other ) const
{GUCEF_TRACE;

    return ( *this < other ) || ( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CUtf8String::IsNULLOrEmpty( void ) const
{GUCEF_TRACE;

    return 0 == m_length;
}

/*-------------------------------------------------------------------------*/

CUtf8String::operator std::string() const
{GUCEF_TRACE;

    if ( m_length > 0 )
    {
        return std::string( (const char*) m_string, m_byteSize-1 );
    }
    return std::string();
}

/*-------------------------------------------------------------------------*/

void
CUtf8String::Set( const char* str           ,
                  UInt32 byteSize           ,
                  UInt32 lengthInCodePoints ,
                  bool reexamineByteSize    )
{GUCEF_TRACE;

    if ( GUCEF_NULL == str || 0 == byteSize || 0 == lengthInCodePoints )
    {
        Clear();
    }
    else
    {
        if ( reexamineByteSize )
        {
            // Check if we are given a larger buffer out of which a subset is used for actual content
            UInt32 newByteSize = (UInt32) utf8size_s( str, byteSize );
            if ( newByteSize != byteSize )
            {                
                byteSize = newByteSize;

                // re-evaluate length regardless of what we were given since we cannot allow length to exceed the byte size
                lengthInCodePoints = (UInt32) utf8len_s( str, newByteSize );
            }
        }

        assert( byteSize >= lengthInCodePoints );

        // Protect against self-assignment
        if ( (void*) str != (void*) m_string )
        {
            if ( lengthInCodePoints > byteSize )
                lengthInCodePoints = byteSize-1;

            bool hasNullTerminator = true;
            if ( lengthInCodePoints == byteSize )
            {
                // there is no room left for a null terminator
                // if we have one bytesize should always be at least 1 greater than lengthInCodePoints
                hasNullTerminator = false;
            }
            else
            {
                UInt32 indexBytesFromStart = 0;
                const char* ptrAtCodePointIndex = CodepointPtrAtIndex( str, byteSize, lengthInCodePoints, lengthInCodePoints, indexBytesFromStart );

                // Check for a null terminator
                if ( GUCEF_NULL == ptrAtCodePointIndex || *ptrAtCodePointIndex != '\0' )
                {
                    // did not find the expected null terminator at the end of 'length'
                    hasNullTerminator = false;
                }
            }

            if ( hasNullTerminator )
            {
                // a null-terminator is already present
                // all we have to do it copy the string buffer as-is
                if ( GUCEF_NULL != Reserve( byteSize ) )
                {
                    memcpy( m_string, str, byteSize );
                    m_length = lengthInCodePoints;
                }
                else
                {
                    // bad alloc
                    Clear();
                }
            }
            else
            {
                // Add the null-terminator since input didnt have one
                if ( GUCEF_NULL != Reserve( byteSize+1 ) )
                {
                    memcpy( m_string, str, byteSize );

                    // Add the null-terminator at the end of the buffer.
                    // this doesnt mean there isnt one earlier in the given buffer as well
                    // however this garantuees that we have at least 1
                    // also note that  '\0' in UTF8 is a codepoint and not to be interpreted as a null terminator
                    *(m_string+byteSize) = '\0';
                    m_length = lengthInCodePoints;
                }
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CUtf8String::Set( const char* str        ,
                  UInt32 byteSize        ,
                  bool reexamineByteSize )
{GUCEF_TRACE;

    if ( GUCEF_NULL != str )
    {
        if ( !reexamineByteSize )
            Set( str, byteSize, (UInt32) utf8len_s( str, byteSize ), false );
        else
            Set( str, (UInt32) utf8size_s( str, byteSize ), (UInt32) utf8len_s( str, byteSize ), false );
    }
    else
    {
        Clear();
    }
}

/*-------------------------------------------------------------------------*/

void
CUtf8String::Set( const char* str )
{GUCEF_TRACE;

    if ( GUCEF_NULL != str )
    {
        Set( str, (UInt32) utf8size( str ) );
    }
    else
    {
        Clear();
    }
}

/*-------------------------------------------------------------------------*/

void
CUtf8String::Set( const wchar_t* str         ,
                  UInt32 lengthInWCodePoints ,
                  bool reexamineByteSize     )
{GUCEF_TRACE;

    if ( GUCEF_NULL == str || 0 == lengthInWCodePoints )
    {
        Clear();
    }
    else
    {
        std::wstring wtmp( str, lengthInWCodePoints );
        std::string str;
        if ( Utf16toUtf8( wtmp, str ) )
        {
            Set( str.c_str(), (UInt32) str.size(), reexamineByteSize );
        }
        else
        {
            Clear();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CUtf8String::Set( const wchar_t* str         ,
                  UInt32 byteSize            ,
                  UInt32 lengthInWCodePoints ,
                  bool reexamineByteSize     )
{GUCEF_TRACE;

    // Put a sanity check on the lengthInWCodePoints to prevent it from exceeding the byte size
    // this ensures that regardless of the supposed code point length we do not exceed the byte size which is 
    // likely to be the input buffer size
    if ( lengthInWCodePoints * sizeof(wchar_t) > byteSize )
        lengthInWCodePoints = byteSize / sizeof(wchar_t);
    
    if ( GUCEF_NULL == str || 0 == lengthInWCodePoints )
    {
        Clear();
    }
    else
    {
        std::wstring wtmp( str, lengthInWCodePoints );
        std::string str;
        if ( Utf16toUtf8( wtmp, str ) )
        {
            Set( str.c_str(), (UInt32) str.size(), reexamineByteSize );
        }
        else
        {
            Clear();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CUtf8String::Set( const wchar_t* str )
{GUCEF_TRACE;

    if ( GUCEF_NULL != str )
    {
        Set( str, (UInt32) wcslen( str ) );
    }
    else
    {
        Clear();
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8String::GetCharacterCount( const Int32 searchChar ) const
{GUCEF_TRACE;

    UInt32 charCount = 0;
    void* cpPos = m_string;
    UInt32 cp = 0;
    for ( UInt32 i=0; i<m_length; ++i )
    {
        cpPos = utf8codepoint( cpPos, &cp );
        if ( cp == searchChar )
        {
            ++charCount;
        }
    }
    return charCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8String::GetCharactersCount( const Int32* searchChars ,
                                 UInt32 nrOfSearchChars   ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == searchChars )
        return 0;

    UInt32 charCount = 0;
    void* cpPos = m_string;
    UInt32 cp = 0;
    for ( UInt32 i=0; i<m_length; ++i )
    {
        cpPos = utf8codepoint( cpPos, &cp );
        for ( UInt32 n=0; n<nrOfSearchChars; ++n )
        {
            if ( cp == searchChars[ n ] )
            {
                ++charCount;
            }
        }
    }
    return charCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8String::GetCharactersCount( const char* searchChars ,
                                 UInt32 nrOfSearchChars  ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == searchChars )
        return 0;

    UInt32 charCount = 0;
    void* cpPos = m_string;
    UInt32 cp = 0;
    for ( UInt32 i=0; i<m_length; ++i )
    {
        cpPos = utf8codepoint( cpPos, &cp );
        for ( UInt32 n=0; n<nrOfSearchChars; ++n )
        {
            if ( cp == searchChars[ n ] )
            {
                ++charCount;
            }
        }
    }
    return charCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8String::GetNonMatchCharactersCount( const char* searchChars ,
                                         UInt32 nrOfSearchChars  ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == searchChars )
        return 0;

    UInt32 charCount = 0;
    void* cpPos = m_string;
    UInt32 cp = 0;
    for ( UInt32 i=0; i<m_length; ++i )
    {
        cpPos = utf8codepoint( cpPos, &cp );
        for ( UInt32 n=0; n<nrOfSearchChars; ++n )
        {
            if ( cp != searchChars[ n ] )
            {
                ++charCount;
            }
        }
    }
    return charCount;
}

/*-------------------------------------------------------------------------*/

UInt32 
CUtf8String::GetNonMatchCharactersCount( const CUtf8String& searchChars ) const
{GUCEF_TRACE;

    if ( searchChars.IsNULLOrEmpty() )
        return 0;

    UInt32 charCount = 0;
    const void* cpPos = m_string;    
    UInt32 cp = 0;
    for ( UInt32 i=0; i<m_length; ++i )
    {
        // get current codepoint of this string
        cpPos = utf8codepoint( cpPos, &cp );

        // check if the current codepoint is in the search string
        bool wasFound = false;
        UInt32 searchCp = 0;
        const void* searchCpPos = searchChars.m_string;
        for ( UInt32 n=0; n<searchChars.m_length; ++n )
        {
            // get current codepoint of the search Cp string
            searchCpPos = utf8codepoint( searchCpPos, &searchCp );
            if ( cp == searchCp )
            {
                wasFound = true;
            }
        }

        // if the current codepoint was not found in the search string
        if ( !wasFound )
            ++charCount;
    }
    return charCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8String::GetNonMatchCharactersCount( const Int32* searchChars ,
                                         UInt32 nrOfSearchChars   ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == searchChars )
        return 0;

    UInt32 charCount = 0;
    const void* cpPos = m_string;
    UInt32 cp = 0;
    for ( UInt32 i=0; i<m_length; ++i )
    {
        cpPos = utf8codepoint( cpPos, &cp );
        for ( UInt32 n=0; n<nrOfSearchChars; ++n )
        {
            if ( cp != searchChars[ n ] )
            {
                ++charCount;
            }
        }
    }
    return charCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8String::GetCharacterRepeatCount( const Int32 searchChar ) const
{GUCEF_TRACE;

    UInt32 charRepeatCount = 0;
    const void* cpPos = m_string;
    UInt32 cp = 0;
    for ( UInt32 i=0; i<m_length; ++i )
    {
        cpPos = utf8codepoint( cpPos, &cp );
        if ( cp == searchChar )
        {
            ++i;
            for ( i; i<m_length; ++i )
            {
                cpPos = utf8codepoint( cpPos, &cp );
                if ( cp == searchChar )
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
CUtf8String::Append( const UInt32 utf32CodePoint )
{GUCEF_TRACE;

    char utf8CodePointBuffer[ 5 ] = { 0,0,0,0,0 };
    char* endPtr = (char*) utf8catcodepoint( utf8CodePointBuffer, utf32CodePoint, sizeof( utf8CodePointBuffer ) );
    if ( GUCEF_NULL == endPtr )
    {
        // corrupt codepoint
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "CUtf8String:Append: Encountered corrupt codepoint" );
        return;
    }

    size_t byteSize = (size_t) ( endPtr - utf8CodePointBuffer );
    Append( utf8CodePointBuffer, (UInt32) byteSize, 1 );
}

/*-------------------------------------------------------------------------*/

void
CUtf8String::Append( const Int32 utf32CodePoint )
{GUCEF_TRACE;

    Append( static_cast< UInt32 >( utf32CodePoint ) );
}

/*-------------------------------------------------------------------------*/

void
CUtf8String::Append( const char* appendstr    ,
                     UInt32 byteSize          ,
                     Int32 lengthInCodePoints )
{GUCEF_TRACE;

    if ( appendstr == GUCEF_NULL || 0 == byteSize )
        return;

    // we always determine the length ourselves in order to either use direct or to sanity check the lengh given
    // as being a shorter subset of the available string
    const char* inputNullAddress = GUCEF_NULL;
    size_t foundLengthInCodePoints = utf8len_s_withaddr( appendstr, byteSize, &inputNullAddress );

    if ( ( lengthInCodePoints < 0 ) ||                       // caller doesnt know, please determine, which we already did
         ( foundLengthInCodePoints < lengthInCodePoints ) )  // the length requested violates the actual available data, we will clamp
    {
        lengthInCodePoints = (Int32) foundLengthInCodePoints;
    }

    // Check if there is anything to append to, if not just treat it as a regular assignment
    if ( 0 == m_length )
    {
        Set( appendstr, byteSize, (UInt32) lengthInCodePoints );
        return;
    }

    // we could only be using a subset of the buffer via Reserve()
    // As such make sure we know the end of the actually used space
    char* addNullAddress = GUCEF_NULL;
    if ( m_byteSize != m_length+1 )
        m_length = (UInt32) utf8len_s_withaddr( m_string, m_byteSize, (const char**) &addNullAddress );
    else
        addNullAddress = m_string + m_length; // no need to seek in this case, small optimization for ASCII text

    // Since we are combining strings we need to work codepoint wise not byte buffer wise
    // this is because the underlying byte buffer may actually be larger than nessesary
    // due to Reserve() style usage. Hence we dont just want to append at the end of that

    UInt32 usedBytes = (UInt32) ( addNullAddress - m_string );
    UInt32 unusedBytes = ((m_byteSize-usedBytes)-1);

   // Check for a null terminator
    if ( GUCEF_NULL != inputNullAddress )
    {
        const char* originalAddress = m_string;
        UInt32 neededBytes = ( m_byteSize - unusedBytes ) + byteSize - 1;
        if ( GUCEF_NULL != Reserve( neededBytes ) )
        {
            // did realloc move the memory or enlarge from current address?
            // don't seek if we dont have to
            if ( GUCEF_NULL == addNullAddress || m_string != originalAddress )
                m_length = (UInt32) utf8len_s_withaddr( m_string, m_byteSize, (const char**) &addNullAddress ); // redetermine address of \0
            memcpy( (char*)addNullAddress, appendstr, byteSize );
            m_length += lengthInCodePoints;
        }
    }
    else
    {
        // Add the null-terminator since input didnt have one
        const char* originalAddress = m_string;
        UInt32 neededBytes = ( m_byteSize - unusedBytes ) + byteSize;
        if ( GUCEF_NULL != Reserve( neededBytes ) )
        {
            // did realloc move the memory or enlarge from current address?
            // don't seek if we dont have to
            if ( GUCEF_NULL == addNullAddress || m_string != originalAddress )
                m_length = (UInt32) utf8len_s_withaddr( m_string, m_byteSize, (const char**) &addNullAddress ); // redetermine address of \0
            memcpy( addNullAddress, appendstr, byteSize );
            m_string[ m_byteSize-1 ] = '\0';
            char* nullPtr;
            m_length = (UInt32) utf8len_s_withaddr( m_string, m_byteSize, (const char**) &nullPtr );
            if ( GUCEF_NULL != nullPtr )
                *nullPtr = '\0';
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CUtf8String::Append( const char* appendstr )
{GUCEF_TRACE;

    if ( appendstr == GUCEF_NULL )
        return;

    Append( appendstr, (UInt32) utf8size( appendstr ), -1 );
}

/*-------------------------------------------------------------------------*/

void
CUtf8String::Append( const CUtf8String& appendstr )
{GUCEF_TRACE;

    Append( appendstr.m_string, appendstr.m_byteSize, (Int32) appendstr.m_length );
}

/*-------------------------------------------------------------------------*/

CUtf8String&
CUtf8String::operator+=( const CUtf8String& other )
{GUCEF_TRACE;

    Append( other.m_string, other.m_byteSize, (Int32) other.m_length );
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf8String&
CUtf8String::operator+=( const char* other )
{GUCEF_TRACE;

    Append( other, (UInt32) utf8size( other ) );
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf8String&
CUtf8String::operator+=( char lastchar )
{GUCEF_TRACE;

    Append( &lastchar, 1 );
    return *this;
}


/*-------------------------------------------------------------------------*/

const char*
CUtf8String::C_String( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_string )
        return reinterpret_cast< const char* >( m_string );
    return "";
}

/*-------------------------------------------------------------------------*/

char*
CUtf8String::C_String( void )
{GUCEF_TRACE;

    return reinterpret_cast< char* >( m_string );
}

/*-------------------------------------------------------------------------*/

char*
CUtf8String::Reserve( const UInt32 byteSize, Int32 newLength )
{GUCEF_TRACE;

    if ( m_byteSize == byteSize )
        return m_string;

    if ( GUCEF_NULL == m_string )
    {
        m_string = (char*) calloc( byteSize, 1 );
        m_byteSize = byteSize;
        m_length = 0;
        return m_string;
    }

    char* newBuffer = (char*) realloc( m_string, byteSize );

    assert( GUCEF_NULL != newBuffer );
    if ( GUCEF_NULL != newBuffer )
    {
        m_string = newBuffer;

        if ( byteSize < m_byteSize )
            m_length = 0;
        if ( newLength >= 0 && (UInt32) newLength < byteSize )
        {
            m_length = (UInt32) newLength;
            newBuffer[ m_length ] = '\0';
        }
        m_byteSize = byteSize;
    }
    else
    {
        if ( GUCEF_NULL != m_string )
            free( m_string );
        m_byteSize = 0;
        m_length = 0;
        m_string = GUCEF_NULL;
    }
    return newBuffer;
}

/*-------------------------------------------------------------------------*/

std::string
CUtf8String::STL_String( void ) const
{GUCEF_TRACE;

    if ( m_length > 0 )
    {
        return std::string( reinterpret_cast< const char* >( m_string ), m_byteSize-1 );
    }
    return std::string();
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8String::Length( void ) const
{GUCEF_TRACE;

    return m_length;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8String::ByteSize( void ) const
{GUCEF_TRACE;

    return m_byteSize;
}

/*-------------------------------------------------------------------------*/

void
CUtf8String::SetLength( UInt32 newLength, UInt32 maxCodePointSize )
{GUCEF_TRACE;

    if ( 0 == newLength || 0 == maxCodePointSize )
    {
        Clear();
        return;
    }

    // Accept the given newLength as truth except for validation of UTF8 validity
    size_t validCpCount = 0;
    char* invalidCpPos = (char*) utf8valid_s( m_string, m_byteSize, &validCpCount );
    if ( GUCEF_NULL == invalidCpPos || validCpCount >= newLength )
    {
        m_string[ m_byteSize-1 ] = '\0'; // guarantee a null terminator
        m_length = newLength;
    }
    else
    {
        // Accepting the length as given would result in an invalid string
        // this could cause crashes and the like, we will not allow this
        m_length = (UInt32) validCpCount;
        *invalidCpPos = '\0'; // guarantee a null terminator
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8String::DetermineLength( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_string )
    {
        m_length = (UInt32) utf8len_s( m_string, m_byteSize );
    }
    else
    {
        m_length = 0;
    }
    return m_length;
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::ReplaceChar( Int32 oldchar ,
                          Int32 newchar ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_string || 0 == m_byteSize )
        return Empty;

    // Calculate new space required

    Int32 codePointSizeDiff = (Int32) ( utf8codepointsize( newchar ) - utf8codepointsize( oldchar ) );
    Int32 charsToReplace = (Int32) GetCharacterCount( oldchar );

    if ( 0 < charsToReplace )
    {
        // Allocate enough space

        Int32 bufferSize = (Int32)m_byteSize + ( charsToReplace * codePointSizeDiff );
        Int32 bufferSpaceLeft = bufferSize;
        CUtf8String newStr;
        if ( 0 < bufferSize && GUCEF_NULL != newStr.Reserve( bufferSize ) )
        {
            // Iterate all current string code points replacing as we go

            const char* srcCpPos = m_string;
            char* destCpPos = newStr.m_string;
            UInt32 cp = 0;
            newStr.m_string[ 0 ] = '\0';
            for ( UInt32 i=0; i<m_length; ++i )
            {
                // Grab code point from current string
                srcCpPos = (const char*) utf8codepoint( srcCpPos, &cp );
                char* newDestCpPos = GUCEF_NULL;
                if ( cp != oldchar )
                {
                    // This one doesnt need replacing, just concat it into the pre-allocated destination buffer
                    newDestCpPos = (char*) utf8catcodepoint( destCpPos, cp, (size_t) bufferSpaceLeft );
                }
                else
                {
                    // This one needs replacing, just concat the given new code point into the pre-allocated destination buffer
                    newDestCpPos = (char*) utf8catcodepoint( destCpPos, newchar, (size_t) bufferSpaceLeft );
                }
                bufferSpaceLeft -= (Int32)( newDestCpPos - destCpPos );
                if ( bufferSpaceLeft < 0 )
                {
                    GUCEF_ASSERT_ALWAYS;
                    newStr.Clear();
                    return newStr;
                }
                destCpPos = newDestCpPos;
            }
            newStr.m_length = m_length;
            newStr.m_string[ newStr.m_byteSize-1 ] = '\0';
            return newStr;
        }
        else
        {
            GUCEF_ASSERT_ALWAYS;
            newStr.Clear();
            return newStr;
        }
    }
    else
    {
        return *this;
    }
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::ReplaceChars( const Int32* oldchars ,
                           UInt32 nrOfOldChars   ,
                           Int32 newchar         ) const
{GUCEF_TRACE;

    // @TODO: Optimize

    CUtf8String result( *this );
    for ( UInt32 i=0; i<nrOfOldChars; ++i )
    {
        result = result.ReplaceChar( oldchars[ i ], newchar );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::ReplaceSubstr( const CUtf8String& substr      ,
                            const CUtf8String& replacement ) const
{GUCEF_TRACE;

    CUtf8String testStr = *this;
    CUtf8String newStr;
    Int32 subStrIndex = 0;
    do
    {
        subStrIndex = testStr.HasSubstr( substr, true );
        if ( subStrIndex >= 0 )
        {
            // turn the code point offset into a byte offset
            UInt32 bytesFromStart=0;
            testStr.CodepointPtrAtIndex( subStrIndex, bytesFromStart );

            // add the prefix segment (if any) before the matching segment
            newStr.Append( testStr.C_String(), bytesFromStart );
            // put in the replacement segment
            newStr += replacement;
            // jump ahead
            testStr = testStr.CutChars( (UInt32)subStrIndex + substr.Length(), true );
        }
    }
    while ( subStrIndex >= 0 );
    newStr += testStr;

    return newStr;
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::ReplaceEnvelopingSubstr( const CUtf8String& envelopPrefix     ,
                                      const CUtf8String& envelopPostfix    ,
                                      const CUtf8String& newEnvelopPrefix  ,
                                      const CUtf8String& newEnvelopPostfix ) const
{GUCEF_TRACE;

    CUtf8String resultStr;

    Int32 startIndex = 0;
    Int32 envSegIndex = HasSubstr( envelopPrefix, startIndex, true );
    while ( envSegIndex >= 0 )
    {
        if ( startIndex < envSegIndex )
        {
            resultStr += SubstrFromRange( startIndex, envSegIndex );
        }

        envSegIndex+=envelopPrefix.Length();
        CUtf8String envelopedSegment = SubstrToSubstr( envelopPostfix, envSegIndex, true );
        resultStr += newEnvelopPrefix + envelopedSegment + newEnvelopPostfix;

        startIndex = envSegIndex+envelopedSegment.Length()+envelopPostfix.Length();
        envSegIndex = HasSubstr( envelopPrefix, startIndex, true );
    }

    // append the remainder, if any
    resultStr += SubstrToIndex( startIndex, false );

    return resultStr;
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::CutEnvelopedSubstr( const CUtf8String& envelopPrefix  ,
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

CUtf8String
CUtf8String::ReplaceSubStr( UInt32 startIndex            ,
                            UInt32 length                ,
                            const CUtf8String& newSubstr ) const
{GUCEF_TRACE;

    // Sanity check on the given range
    if ( 0 == m_length )
        return CUtf8String();
    if ( startIndex >= m_length )
        startIndex = m_length -1;
    if ( startIndex+length >= m_length )
        length = (m_length-1) - startIndex;

    CUtf8String prefix = SubstrFromRange( 0, startIndex );
    CUtf8String postfix = SubstrFromRange( startIndex+length+1, m_length );

    return prefix + newSubstr + postfix;
}

/*-------------------------------------------------------------------------*/

void
CUtf8String::Clear( void )
{GUCEF_TRACE;

    m_length = 0;
    m_byteSize = 0;
    if ( GUCEF_NULL != m_string )
    {
        free( m_string );
        m_string = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::SubstrToChar( UInt32 searchchar              ,
                           UInt32 startIndex              ,
                           bool frontToBack               ,
                           bool returnEmptyIfCharNotFound ) const
{GUCEF_TRACE;

    if ( startIndex < m_length )
    {
        if ( frontToBack )
        {
            UInt32 bytesFromStart = 0;
            const char* startPos = CodepointPtrAtIndex( startIndex, bytesFromStart );
            const char* cpPos = startPos;
            const char* prevCpPos = cpPos;
            UInt32 codePoint = 0;
            for ( UInt32 i=startIndex; i<m_length; ++i )
            {
                cpPos = (const char*) utf8codepoint( prevCpPos, &codePoint );
                if ( codePoint == searchchar )
                {
                    CUtf8String substr;
                    substr.Set( startPos, (UInt32)(prevCpPos - startPos), i-startIndex );
                    return substr;
                }
                prevCpPos = cpPos;
            }
            if ( returnEmptyIfCharNotFound )
                return CUtf8String();
            else
                return CUtf8String( *this );
        }
        else
        {
            UInt32 bytesFromStart = 0;
            const char* startPos = CodepointPtrAtIndex( startIndex, bytesFromStart );
            const char* cpPos = startPos;
            const char* prevCpPos = cpPos;
            UInt32 codePoint = 0;
            for ( Int32 i=startIndex; i>=0; --i )
            {
                cpPos = (const char*) utf8rcodepoint( prevCpPos, &codePoint );
                if ( codePoint == searchchar )
                {
                    const char* prevPrevCpPos = (const char*) utf8codepoint( prevCpPos, &codePoint );
                    CUtf8String substr;
                    substr.Set( prevPrevCpPos, (UInt32)(startPos - prevCpPos), startIndex-i );
                    return substr;
                }
                prevCpPos = cpPos;
            }
            if ( returnEmptyIfCharNotFound )
                return CUtf8String();
            else
                return CUtf8String( *this );
        }
    }
    return CUtf8String();
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::SubstrToChar( char searchchar                ,
                           UInt32 startIndex              ,
                           bool frontToBack               ,
                           bool returnEmptyIfCharNotFound ) const
{GUCEF_TRACE;

    return SubstrToChar( static_cast< UInt32 >( searchchar ), startIndex, frontToBack, returnEmptyIfCharNotFound );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::SubstrToIndex( UInt32 index     ,
                            bool frontToBack ) const
{GUCEF_TRACE;

    if ( !frontToBack )
    {
         if ( index >= m_length )
            return CUtf8String();

            // @TODO: this is inconsistent !!!
            //   frontToBack is exclusive of index while the reverse is not
         return SubstrFromRange( index, m_length );
    }

    if ( index >= m_length )
        return CUtf8String( m_string, m_length );
    return SubstrFromRange( 0, index );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::SubstrFromRange( UInt32 startIndex ,
                              UInt32 endIndex   ) const
{GUCEF_TRACE;

    if ( startIndex == endIndex )
        return CUtf8String();

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
        UInt32 bytesFromStartAtStart = 0;
        UInt32 bytesFromStartAtEnd = 0;
        const char* subStrStartPtr = CodepointPtrAtIndex( maxStart, bytesFromStartAtStart );
        CodepointPtrAtIndex( maxEnd, bytesFromStartAtEnd );

        // create the string from the sub-segment, a null terminator will be auto added
        return CUtf8String( subStrStartPtr, bytesFromStartAtEnd-bytesFromStartAtStart );
    }
    return CUtf8String();
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::SubstrToChar( UInt32 searchchar              ,
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

CUtf8String
CUtf8String::SubstrToChar( char searchchar                ,
                           bool frontToBack               ,
                           bool returnEmptyIfCharNotFound ) const
{GUCEF_TRACE;

    return SubstrToChar( static_cast< UInt32 >( searchchar ), frontToBack, returnEmptyIfCharNotFound );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::Trim( bool frontToBack ) const
{GUCEF_TRACE;

    if ( m_length > 0 )
    {
        if ( frontToBack )
        {
            UInt32 charsToCut = 0;
            void* cpPos = m_string;
            UInt32 cp = 0;
            UInt32 i=0;
            do
            {
                cpPos = utf8codepoint( cpPos, &cp );
                if ( cp == ' ' ||  cp == '\t' )
                {
                    ++charsToCut;
                }
                else
                {
                    break;
                }
                ++i;
            }
            while ( i<m_length );

            return CutChars( charsToCut, frontToBack );
        }
        else
        {
            UInt32 bytesFromStart = 0;
            UInt32 charsToCut = 0;
            void* cpPos = (void*) CodepointPtrAtIndex( m_length-1, bytesFromStart );
            UInt32 cp = 0;
            Int32 i = (Int32) m_length-1;
            do
            {
                cpPos = utf8rcodepoint( cpPos, &cp );
                if ( cp == ' ' ||  cp == '\t' )
                {
                    ++charsToCut;
                }
                else
                {
                    break;
                }
                --i;
            }
            while ( i>=0 );

            return CutChars( charsToCut, frontToBack );
        }
    }
    return CUtf8String();
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::SubstrToSubstr( const CUtf8String& searchstr ,
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

CUtf8String
CUtf8String::SubstrToSubstr( const CUtf8String& searchstr ,
                             UInt32 startIndex            ,
                             bool startfront              ) const
{GUCEF_TRACE;

    Int32 subStrIndex = HasSubstr( searchstr, startIndex, startfront, true );
    if ( subStrIndex >= 0 )
    {
        if ( startfront )
            return SubstrFromRange( startIndex, subStrIndex );
        else
            return SubstrFromRange( subStrIndex + searchstr.m_length, startIndex+1 );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::RemoveChar( const Int32 charToRemove ) const
{GUCEF_TRACE;

    // Calculate new space required

    Int32 codePointSize = (Int32) utf8codepointsize( charToRemove );
    Int32 charsToRemove = (Int32) GetCharacterCount( charToRemove );

    if ( 0 < charsToRemove )
    {
        // Allocate enough space

        Int32 bufferSize = (Int32)m_byteSize - ( charsToRemove * codePointSize );
        Int32 bufferSpaceLeft = bufferSize;
        CUtf8String newStr;
        if ( 0 < bufferSize && GUCEF_NULL != newStr.Reserve( bufferSize ) )
        {
            // Iterate all current string code points skipping matches as we go

            const char* srcCpPos = m_string;
            char* destCpPos = newStr.m_string;
            UInt32 cp = 0;
            newStr.m_string[ 0 ] = '\0';
            for ( UInt32 i=0; i<m_length; ++i )
            {
                // Grab code point from current string
                srcCpPos = (const char*) utf8codepoint( srcCpPos, &cp );
                if ( cp != charToRemove )
                {
                    // This one can stay, just concat it into the pre-allocated destination buffer
                    char* newDestCpPos = (char*) utf8catcodepoint( destCpPos, cp, (size_t) bufferSpaceLeft );
                    bufferSpaceLeft -= (Int32)( newDestCpPos - destCpPos );
                    destCpPos = newDestCpPos;
                }
                // else: This one needs removal, just skip the copy into the pre-allocated destination buffer
            }
            newStr.m_length = (UInt32)( m_length - charsToRemove );
            newStr.m_string[ newStr.m_byteSize-1 ] = '\0';
            return newStr;
        }
        else
        {
            GUCEF_ASSERT_ALWAYS;
            return newStr;
        }
    }
    else
    {
        return *this;
    }
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::CompactRepeatingChar( const Int32 charToCompact ) const
{GUCEF_TRACE;

    // Calculate new space required

    Int32 codePointSize = (Int32) utf8codepointsize( charToCompact );
    Int32 charsToRemove = (Int32) GetCharacterRepeatCount( charToCompact );

    if ( 0 < charsToRemove )
    {
        // Allocate enough space

        Int32 bufferSize = (Int32)m_byteSize - ( charsToRemove * codePointSize );
        Int32 bufferSpaceLeft = bufferSize;
        CUtf8String newStr;
        if ( 0 < bufferSize && GUCEF_NULL != newStr.Reserve( bufferSize ) )
        {
            // Iterate all current string code points skipping repeat matches as we go

            const char* srcCpPos = m_string;
            char* destCpPos = newStr.m_string;
            UInt32 cp = 0;
            newStr.m_string[ 0 ] = '\0';
            for ( UInt32 i=0; i<m_length; ++i )
            {
                // Grab code point from current string
                srcCpPos = (const char*) utf8codepoint( srcCpPos, &cp );

                // This one can stay, just concat it into the pre-allocated destination buffer
                char* newDestCpPos = (char*) utf8catcodepoint( destCpPos, cp, (size_t) bufferSpaceLeft );
                bufferSpaceLeft -= (Int32)( newDestCpPos - destCpPos );
                destCpPos = newDestCpPos;

                if ( cp == charToCompact )
                {
                    ++i;
                    for ( i; i<m_length; ++i )
                    {
                        // Grab code point from current string
                        srcCpPos = (const char*) utf8codepoint( srcCpPos, &cp );

                        // Skip all that now match the code point we wish to compact
                        if ( cp != charToCompact )
                        {
                            // This one can stay, just concat it into the pre-allocated destination buffer
                            newDestCpPos = (char*) utf8catcodepoint( destCpPos, cp, (size_t) bufferSpaceLeft );
                            bufferSpaceLeft -= (Int32)( newDestCpPos - destCpPos );
                            destCpPos = newDestCpPos;

                            // This was the end of the repeat series since a non-matching code point was found
                            break;
                        }
                    }
                }
            }
            newStr.m_length = m_length - charsToRemove;
            newStr.m_string[ newStr.m_byteSize-1 ] = '\0';
            return newStr;
        }
        else
        {
            GUCEF_ASSERT_ALWAYS;
            newStr.Clear();
            return newStr;
        }
    }
    else
    {
        return *this;
    }
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::CutChars( UInt32 charcount   ,
                       bool startfront    ,
                       UInt32 startOffset ) const
{GUCEF_TRACE;

    if ( startfront )
    {
        if ( 0 == startOffset )
        {
            if ( charcount < m_length )
            {
                return SubstrFromRange( charcount, m_length );
            }

            return CUtf8String::Empty;
        }

        return SubstrFromRange( 0, startOffset ) + SubstrFromRange( startOffset+charcount, m_length );
    }
    else
    {
        if ( 0 == startOffset )
        {
            if ( charcount < m_length )
            {
                return SubstrFromRange( 0, m_length-charcount );
            }

            return CUtf8String::Empty;
        }

        return SubstrFromRange( 0, m_length-startOffset-charcount ) + SubstrFromRange( m_length-1-startOffset, m_length );
    }
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::Lowercase( void ) const
{GUCEF_TRACE;

    if ( 0 < m_length && GUCEF_NULL != m_string )
    {
        CUtf8String lwrCopy( *this );
        utf8lwr( lwrCopy.m_string );
        return lwrCopy;
    }
    return CUtf8String();
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::Uppercase( void ) const
{GUCEF_TRACE;

    if ( 0 < m_length && GUCEF_NULL != m_string )
    {
        CUtf8String uprCopy( *this );
        utf8upr( uprCopy.m_string );
        return uprCopy;
    }
    return CUtf8String();
}

/*-------------------------------------------------------------------------*/

Int32
CUtf8String::HasChar( UInt32 searchchar       ,
                      const UInt32 startIndex ,
                      bool frontToBack        ) const
{GUCEF_TRACE;

    if ( startIndex >= m_length )
        return -1;

    if ( frontToBack )
    {
        void* cpPos = m_string;
        UInt32 cp = 0;
        UInt32 i=0;
        do
        {
            cpPos = utf8codepoint( cpPos, &cp );
            if ( i >= startIndex && cp == searchchar )
                return i;
            ++i;
        }
        while ( i<m_length );
        return -1;
    }
    else
    {
        UInt32 bytesFromStart = 0;
        Int32 max = startIndex > m_length-1 ? m_length-1 : startIndex;
        void* cpPos = (void*) CodepointPtrAtIndex( max, bytesFromStart );
        UInt32 cp = 0;
        Int32 i = (Int32) max;
        do
        {
            cpPos = utf8rcodepoint( cpPos, &cp );
            if ( cp == searchchar )
                return i;
            --i;
        }
        while ( i>=0 );
        return -1;
    }
}

/*-------------------------------------------------------------------------*/

Int32
CUtf8String::HasChar( char searchchar         ,
                      const UInt32 startIndex ,
                      bool frontToBack        ) const
{GUCEF_TRACE;

    return HasChar( static_cast< UInt32 >( searchchar ), startIndex, frontToBack );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf8String::HasChar( UInt32 searchchar ,
                      bool startfront   ) const
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

Int32
CUtf8String::HasChar( char searchchar ,
                      bool startfront ) const
{GUCEF_TRACE;

    return HasChar( static_cast< UInt32 >( searchchar ), startfront );
}

/*-------------------------------------------------------------------------*/

CUtf8String::StringSet
CUtf8String::ParseUniqueElements( UInt32 seperator      ,
                                  bool addEmptyElements ) const
{GUCEF_TRACE;

    return ParseUniqueElements( (const UInt8*) m_string, m_byteSize, seperator, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

CUtf8String::StringSet
CUtf8String::ParseUniqueElements( const UInt8* bufferPtr ,
                                  UInt32 bufferSize      ,
                                  UInt32 seperator       ,
                                  bool addEmptyElements  )
{GUCEF_TRACE;

    if ( GUCEF_NULL != bufferPtr && bufferSize > 0 )
    {
        UInt32 bufferCps = 0;
        StringSet list;
        CUtf8String entry;

        char* cpPos = (char*) bufferPtr;
        UInt32 lastSepCpIndex = 0;
        char* lastSepCpPtr = cpPos;
        UInt32 i=0;
        UInt64 bytesRead = 0;
        do
        {
            UInt32 codePoint=0;
            char* newCpPos = (char*) utf8codepoint( cpPos, &codePoint );
            if ( cpPos != newCpPos )
                ++bufferCps;

            if ( codePoint == seperator )
            {
                UInt32 stringLength = i-lastSepCpIndex;
                if ( ( 0 == stringLength && addEmptyElements ) ||
                     ( stringLength > 0 ) )
                {
                    entry.Set( lastSepCpPtr, (UInt32)(cpPos-lastSepCpPtr), stringLength );
                    list.insert( entry );
                }

                lastSepCpIndex = i+1;
                lastSepCpPtr = newCpPos;
            }

            cpPos = newCpPos;
            bytesRead = (UInt64) ( (const UInt8*)newCpPos - (const UInt8*)bufferPtr );
            ++i;
        }
        while ( GUCEF_NULL != cpPos && bytesRead < bufferSize );

        // add last item
        UInt32 stringLength = bufferCps-lastSepCpIndex;
        if ( ( 0 == stringLength && addEmptyElements ) ||
             ( stringLength > 0 && ( GUCEF_NULL != lastSepCpPtr && ( *lastSepCpPtr != '\0' || addEmptyElements ) ) ) )
        {
            entry.Set( lastSepCpPtr, (UInt32)(cpPos-lastSepCpPtr), stringLength );
            list.insert( entry );
        }
        return list;
    }
    return StringSet();
}

/*-------------------------------------------------------------------------*/

CUtf8String::StringVector
CUtf8String::ParseElements( UInt32 seperator      ,
                            bool addEmptyElements ) const
{GUCEF_TRACE;

    return ParseElements( (const UInt8*) m_string, m_byteSize, seperator, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

CUtf8String::StringVector
CUtf8String::ParseElements( const UInt8* bufferPtr ,
                            UInt32 bufferSize      ,
                            UInt32 seperator       ,
                            bool addEmptyElements  )
{GUCEF_TRACE;

    if ( GUCEF_NULL != bufferPtr && bufferSize > 0 )
    {
        UInt32 bufferCps = 0;
        StringVector list;
        CUtf8String entry;

        char* cpPos = (char*) bufferPtr;
        UInt32 lastSepCpIndex = 0;
        char* lastSepCpPtr = cpPos;
        UInt32 i=0;
        UInt64 bytesRead = 0;
        UInt32 codePoint = 0;
        do
        {
            char* newCpPos = (char*) utf8codepoint( cpPos, &codePoint );
            if ( codePoint != '\0' || seperator == '\0' )
            {
                if ( cpPos != newCpPos )
                    ++bufferCps;

                if ( codePoint == seperator )
                {
                    UInt32 stringLength = i-lastSepCpIndex;
                    if ( ( 0 == stringLength && addEmptyElements ) ||
                         ( stringLength > 0 ) )
                    {
                        entry.Set( lastSepCpPtr, (UInt32)(cpPos-lastSepCpPtr), stringLength );
                        list.push_back( entry );
                    }

                    lastSepCpIndex = i+1;
                    lastSepCpPtr = newCpPos;
                }

                cpPos = newCpPos;
                bytesRead = (UInt64) ( (const UInt8*)newCpPos - (const UInt8*)bufferPtr );
                ++i;
            }
        }
        while ( GUCEF_NULL != cpPos && ( codePoint != '\0' || seperator == '\0' ) && bytesRead < bufferSize );

        // add last item
        UInt32 stringLength = bufferCps-lastSepCpIndex;
        if ( ( 0 == stringLength && addEmptyElements ) ||
             ( stringLength > 0 ) )
        {
            entry.Set( lastSepCpPtr, (UInt32)(cpPos-lastSepCpPtr), stringLength );
            list.push_back( entry );
        }
        return list;
    }
    return StringVector();
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8String::FindMaxSubstrEquality( const CUtf8String& searchStr ,
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
        CUtf8String lowercaseSearchStrStorage;
        CUtf8String lowercaseThisStrStorage;
        const CUtf8String* theSearchStr = &searchStr;
        const CUtf8String* thisStr = this;

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
        UInt32 maxMatchLength = theSearchStr->m_length;
        if ( thisStr->m_length - startOffset < maxMatchLength )
        {
            maxMatchLength = thisStr->m_length - startOffset;
        }

        if ( startFront )
        {
            // Loop trough comparing code points
            UInt32 thisStrBytesFromStart=0;
            const char* thisStrStartPos = thisStr->CodepointPtrAtIndex( startOffset, thisStrBytesFromStart );
            const char* thisStrCpPos = thisStrStartPos;
            UInt32 searchStrBytesFromStart=0;
            const char* searchStrStartPos = theSearchStr->CodepointPtrAtIndex( 0, searchStrBytesFromStart );
            const char* searchStrCpPos = searchStrStartPos;

            UInt32 thisStrCp = 0;
            UInt32 searchStrCp = 0;
            UInt32 matchingCpCount=0;
            for ( ; matchingCpCount<maxMatchLength; ++matchingCpCount )
            {
                thisStrCpPos = (const char*) utf8codepoint( thisStrCpPos, &thisStrCp );
                searchStrCpPos = (const char*) utf8codepoint( searchStrCpPos, &searchStrCp );
                if ( thisStrCp != searchStrCp )
                    break;
            }
            return matchingCpCount;
        }
        else
        {
            // Loop trough the buffer growing our comparison string
            UInt32 bytesFromStart=0;
            const char* startPos = thisStr->CodepointPtrAtIndex( thisStr->m_length-1-startOffset, bytesFromStart );
            const char* cpPos = startPos;
            const char* otherString = theSearchStr->CodepointPtrAtIndex( theSearchStr->m_length-1-startOffset, bytesFromStart );

            UInt32 subLength=1;
            size_t subByteSize = utf8codepointcalcsize( cpPos );
            while ( subLength<=maxMatchLength )
            {                         // @TODO: refactor: compare based on code points not memcmp()
                if ( memcmp( startPos-subByteSize, otherString-subByteSize, subByteSize ) != 0 )
                {
                    // Reached the maximum equality length
                    return subLength-1;
                }

                // Grow the comparison segment enough bytes to fit 1 extra codepoint
                UInt32 cp;
                cpPos = (const char*) utf8rcodepoint( cpPos, &cp );
                subByteSize = (UInt32)( startPos - cpPos );

                ++subLength;
            }
            return subLength-1;
        }
    }

    // Unable to find an equality
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8String::FindMaxSegmentEquality( const CUtf8String& otherStr   ,
                                     Int32 segmentDividerCodePoint ,
                                     bool startFront               ,
                                     bool isCaseSentive            ) const
{GUCEF_TRACE;

    // Here we want to be able to support case insensitive compares
    // without having to suffer the performance penalty of making string lowercase
    // when not doing case insensitive compares. This we do some pointer magic to
    // avoid this overhead when we can
    CUtf8String lowercaseSearchStrStorage;
    CUtf8String lowercaseThisStrStorage;
    const CUtf8String* theSearchStr = &otherStr;
    const CUtf8String* thisStr = this;

    if ( !isCaseSentive )
    {
        // Caller wants a case-insensitive compare,..
        // take the performance hit
        lowercaseSearchStrStorage = otherStr.Lowercase();
        theSearchStr = &lowercaseSearchStrStorage;
        lowercaseThisStrStorage = Lowercase();
        thisStr = &lowercaseThisStrStorage;
    }

    if ( thisStr->m_length == theSearchStr->m_length )
    {
        // possible early out
        if ( (*thisStr) == (*theSearchStr) )
            return thisStr->m_length;
    }

    // Get the smallest of the 2 buffer limits
    UInt32 maxMatchLength = theSearchStr->m_length;
    if ( thisStr->m_length < maxMatchLength )
    {
        maxMatchLength = thisStr->m_length;
    }

    if ( startFront )
    {
        // Loop trough comparing code points
        UInt32 thisStrBytesFromStart=0;
        const char* thisStrStartPos = thisStr->CodepointPtrAtIndex( 0, thisStrBytesFromStart );
        const char* thisStrCpPos = thisStrStartPos;
        UInt32 searchStrBytesFromStart=0;
        const char* searchStrStartPos = theSearchStr->CodepointPtrAtIndex( 0, searchStrBytesFromStart );
        const char* searchStrCpPos = searchStrStartPos;

        UInt32 thisStrCp = 0;
        UInt32 searchStrCp = 0;
        UInt32 matchingCpCount=0;
        UInt32 matchingCpCountAtDivider=0;
        for ( ; matchingCpCount<maxMatchLength; ++matchingCpCount )
        {
            thisStrCpPos = (const char*) utf8codepoint( thisStrCpPos, &thisStrCp );
            searchStrCpPos = (const char*) utf8codepoint( searchStrCpPos, &searchStrCp );
            if ( thisStrCp != searchStrCp )
                break;
            if ( thisStrCp == segmentDividerCodePoint )
                matchingCpCountAtDivider = matchingCpCount;
        }

        if ( matchingCpCount >= maxMatchLength )
        {
            if ( thisStr->m_length > theSearchStr->m_length )
            {
                // check to see if the next code point in the longer string is a divider
                // if so we need to include this last segment
                thisStrCpPos = (const char*) utf8codepoint( thisStrCpPos, &thisStrCp );
                if ( thisStrCp == segmentDividerCodePoint )
                {
                    matchingCpCountAtDivider = matchingCpCount;
                }
            }
            else
            if ( thisStr->m_length < theSearchStr->m_length )
            {
                // check to see if the next code point in the longer string is a divider
                // if so we need to include this last segment
                searchStrCpPos = (const char*) utf8codepoint( searchStrCpPos, &searchStrCp );
                if ( searchStrCp == segmentDividerCodePoint )
                {
                    matchingCpCountAtDivider = matchingCpCount;
                }
            }
        }

        return matchingCpCountAtDivider;
    }
    else
    {
        // @TODO
    }

    // Unable to find an equality
    return 0;
}

/*-------------------------------------------------------------------------*/

Int32
CUtf8String::CodepointIndexAtPtr( const char* subStrPtr, UInt32& codePoint ) const
{GUCEF_TRACE;

     codePoint = 0;

     // Sanity check on the pointer
     if ( m_string > subStrPtr || m_string + m_byteSize < subStrPtr )
        return -1;

    char* cpPos = m_string;
    UInt32 i=0;
    do
    {
        // Check for exact offset match
        if ( cpPos == subStrPtr )
            return (Int32) i;

        char* newCpPos = (char*) utf8codepoint( cpPos, &codePoint );

        // The pointer given could be at an offset inside the boundaries of a single multi-byte codepoint
        // in this case we effectively rewind back to the start offset of the current code point index wise
        if ( newCpPos > subStrPtr )
            return (Int32) i;

        cpPos = newCpPos;
        ++i;
    }
    while ( i<m_length );

    codePoint = 0;
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CUtf8String::HasSubstr( const CUtf8String& substr ,
                        Int32 startIndex          ,
                        bool startfront           ,
                        bool shiftSearch          ) const
{GUCEF_TRACE;

    // Sanity check on the startindex range
    if ( startIndex < 0 || (UInt32) startIndex > m_length || substr.IsNULLOrEmpty() )
        return -1;

    // Cheap check for obvious mismatches
    if ( substr.m_length > m_length || substr.m_byteSize > m_byteSize )
    {
        return -1;
    }

    if ( startfront )
    {
        // Sanity check on the startindex range
        if ( ((UInt32)startIndex) + substr.m_length > m_length )
            return -1;

        UInt32 bytesFromStart = 0;
        char* cpPos = (char*) CodepointPtrAtIndex( startIndex, bytesFromStart );
        UInt32 maxByteOffset = m_byteSize - substr.m_byteSize;

        for ( UInt32 i=bytesFromStart; i<=maxByteOffset; ++i )
        {
            if ( 0 == memcmp( m_string+i, substr.m_string, substr.m_byteSize-1 ) )
            {
                UInt32 codePoint = 0;
                return CodepointIndexAtPtr( m_string+i, codePoint );
            }
            if ( !shiftSearch )
                break;
        }
        return -1;
    }
    else
    {
        // Sanity check on the startindex range
        if ( startIndex - (Int32)substr.m_length < 0 )
            return -1;

        UInt32 bytesFromStart = 0;
        char* cpPos = (char*) CodepointPtrAtIndex( startIndex, bytesFromStart );
        for ( Int32 i=bytesFromStart-(substr.m_byteSize-1); i>=0; --i )
        {
            if ( 0 == memcmp( m_string+i, substr.m_string, substr.m_byteSize-1 ) )
            {
                UInt32 codePoint = 0;
                return CodepointIndexAtPtr( m_string+i, codePoint );
            }
            if ( !shiftSearch )
                break;
        }
        return -1;
    }
}

/*-------------------------------------------------------------------------*/

Int32
CUtf8String::HasSubstr( const CUtf8String& substr ,
                        bool startfront           ,
                        bool shiftSearch          ) const
{GUCEF_TRACE;

    if ( startfront )
    {
        return HasSubstr( substr, 0, startfront, shiftSearch );
    }
    return HasSubstr( substr, m_length, startfront, shiftSearch );
}

/*-------------------------------------------------------------------------*/

bool 
CUtf8String::StartsWith( const CUtf8String& substr ) const
{GUCEF_TRACE;

    return 0 == HasSubstr( substr, 0, true, false );
}

/*-------------------------------------------------------------------------*/

bool 
CUtf8String::EndsWith( const CUtf8String& substr ) const
{GUCEF_TRACE;

    if ( m_length > 0 )
        return ((Int32)m_length)-substr.m_length == HasSubstr( substr, m_length, false, false );
    return substr.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CAsciiString
CUtf8String::ForceToAscii( char asciiReplacement ) const
{GUCEF_TRACE;

    CAsciiString ascii;
    if ( m_length > 0 )
    {
        char* asciiBuffer = ascii.Reserve( m_length+1 );

        const char* cpPos = m_string;
        UInt32 cp = 0;
        UInt32 i=0;
        do
        {
            cpPos = (const char*) utf8codepoint( cpPos, &cp );

            if ( cp >= 0 && cp <= 127 )
                *asciiBuffer = (char) cp;
            else
                *asciiBuffer = asciiReplacement;

            ++i; ++asciiBuffer;
        }
        while ( i<=m_length );
        ascii.SetLength( m_length );
    }
    return ascii;
}

/*-------------------------------------------------------------------------*/

bool
CUtf8String::WildcardEquals( const CUtf8String& strWithWildcards    ,
                             const UInt32 wildCardToken /* = '*' */ ,
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

    // If the string does not actually have wildcards treat it like a regular string compare
    if ( -1 == strWithWildcards.HasChar( wildCardToken ) )
        return Equals( strWithWildcards, caseSensitive );

    CUtf8String::StringVector segs = strWithWildcards.ParseElements( wildCardToken, false );
    Int32 lastSeg = 0;
    CUtf8String::StringVector::iterator i = segs.begin();
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
CUtf8String::WildcardEquals( const StringSet& strsWithWildcards     ,
                             const UInt32 wildCardToken /* = '*' */ ,
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
CUtf8String::Equals( const CUtf8String& otherStr           ,
                     const bool caseSensitive /* = true */ ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_string && GUCEF_NULL == otherStr.m_string )
        return true;
    else
    if ( GUCEF_NULL == m_string || GUCEF_NULL == otherStr.m_string )
        return false;

    if ( caseSensitive )
    {
        return 0 == utf8cmp( m_string, otherStr.m_string );
    }
    else
    {
        return 0 == utf8casecmp( m_string, otherStr.m_string );
    }
}

/*-------------------------------------------------------------------------*/

bool
CUtf8String::NotEquals( const CUtf8String& otherStr           ,
                        const bool caseSensitive /* = true */ ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_string && GUCEF_NULL == otherStr.m_string )
        return false;
    else
    if ( GUCEF_NULL == m_string || GUCEF_NULL == otherStr.m_string )
        return true;

    if ( caseSensitive )
    {
        return 0 != utf8cmp( m_string, otherStr.m_string );
    }
    else
    {
        return 0 != utf8casecmp( m_string, otherStr.m_string );
    }
}

/*-------------------------------------------------------------------------*/

void
CUtf8String::Scan( const char* newStr     ,
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

CUtf8String
CUtf8String::Combine( const StringVector& elements, Int32 seperator ) const
{GUCEF_TRACE;

    if ( elements.empty() && 0 == m_length )
        return CUtf8String();

    // Determine storage size needed

    size_t cpSize = utf8codepointsize( seperator );
    UInt32 bufferSize = 1;
    if ( m_byteSize > 0 )
        bufferSize += m_byteSize + (UInt32) cpSize;

    StringVector::const_iterator i = elements.begin();
    while ( i != elements.end() )
    {
        bool elementAdded = false;
        if ( !(*i).IsNULLOrEmpty() )
        {
            bufferSize += (*i).m_byteSize-1;
            elementAdded = true;
        }
        ++i;
        if ( elementAdded && i != elements.end() && !(*i).IsNULLOrEmpty() )
        {
            bufferSize += (UInt32) cpSize;
        }
    }

    // Allocate total storage

    CUtf8String comboStr;
    if ( GUCEF_NULL == comboStr.Reserve( bufferSize ) )
        return CUtf8String::Empty;

    // Write contents
    char* cpPos = comboStr.m_string;
    size_t bufferSpaceLeft = bufferSize;
    if ( GUCEF_NULL != m_string && 0 < m_length )
    {
        memcpy( comboStr.m_string, m_string, m_byteSize );
        bufferSpaceLeft -= m_byteSize;

        cpPos = (char*) utf8catcodepoint( cpPos, seperator, bufferSpaceLeft );
        bufferSpaceLeft -= cpSize;
    }
    else
    {
        comboStr.m_string[ 0 ] = '\0';
    }
    i = elements.begin();
    while ( i != elements.end() )
    {
        bool elementAdded = false;
        if ( !(*i).IsNULLOrEmpty() )
        {
            size_t bytesWritten = (*i).m_byteSize-1;
            memcpy( cpPos, (*i).m_string, bytesWritten );
            cpPos += bytesWritten;
            *cpPos = '\0';
            bufferSpaceLeft -= bytesWritten;
            elementAdded = true;
        }
        ++i;
        if ( elementAdded && i != elements.end() && !(*i).IsNULLOrEmpty() )
        {
            cpPos = (char*) utf8catcodepoint( cpPos, seperator, bufferSpaceLeft );
            bufferSpaceLeft -= cpSize;
        }
    }

    // Determine new string length codepoint wise
    comboStr.m_length = (UInt32) utf8len_s( comboStr.m_string, comboStr.m_byteSize );
    comboStr.m_string[ comboStr.m_byteSize-1 ] = '\0';
    return comboStr;
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::Combine( const StringSet& elements, Int32 seperator ) const
{GUCEF_TRACE;

    if ( elements.empty() && 0 == m_length )
        return CUtf8String();

    // Determine storage size needed

    size_t cpSize = utf8codepointsize( seperator );
    UInt32 bufferSize = 1;
    if ( m_byteSize > 0 )
        bufferSize += m_byteSize + (UInt32) cpSize;

    StringSet::const_iterator i = elements.begin();
    while ( i != elements.end() )
    {
        bool elementAdded = false;
        if ( !(*i).IsNULLOrEmpty() )
        {
            bufferSize += (*i).m_byteSize-1;
            elementAdded = true;
        }
        ++i;
        if ( elementAdded && i != elements.end() && !(*i).IsNULLOrEmpty() )
        {
            bufferSize += (UInt32) cpSize;
        }
    }

    // Allocate total storage

    CUtf8String comboStr;
    if ( GUCEF_NULL == comboStr.Reserve( bufferSize ) )
        return CUtf8String::Empty;

    // Write contents
    char* cpPos = comboStr.m_string;
    size_t bufferSpaceLeft = bufferSize;
    if ( GUCEF_NULL != m_string && 0 < m_length )
    {
        memcpy( comboStr.m_string, m_string, m_byteSize );
        bufferSpaceLeft -= m_byteSize;

        cpPos = (char*) utf8catcodepoint( cpPos, seperator, bufferSpaceLeft );
        bufferSpaceLeft -= cpSize;
    }
    else
    {
        comboStr.m_string[ 0 ] = '\0';
    }
    i = elements.begin();
    while ( i != elements.end() )
    {
        bool elementAdded = false;
        if ( !(*i).IsNULLOrEmpty() )
        {
            size_t bytesWritten = (*i).m_byteSize-1;
            memcpy( cpPos, (*i).m_string, bytesWritten );
            cpPos += bytesWritten;
            *cpPos = '\0';
            bufferSpaceLeft -= bytesWritten;
            elementAdded = true;
        }
        ++i;
        if ( elementAdded && i != elements.end() && !(*i).IsNULLOrEmpty() )
        {
            cpPos = (char*) utf8catcodepoint( cpPos, seperator, bufferSpaceLeft );
            bufferSpaceLeft -= cpSize;
        }
    }

    // Determine new string length codepoint wise
    comboStr.m_length = (UInt32) utf8len_s( comboStr.m_string, comboStr.m_byteSize );
    comboStr.m_string[ comboStr.m_byteSize-1 ] = '\0';
    return comboStr;
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::Combine( const StringMap& elements, Int32 valueSeperator, Int32 kvSeperator ) const
{GUCEF_TRACE;

    if ( elements.empty() && 0 == m_length )
        return CUtf8String();

    // Determine storage size needed

    size_t valueSepCpSize = utf8codepointsize( valueSeperator );
    size_t kvSepCpSize = utf8codepointsize( kvSeperator );
    size_t totalExtraCpSize = ( valueSepCpSize * elements.size() ) + ( kvSepCpSize * elements.size() );

    UInt32 bufferSize = (UInt32) totalExtraCpSize;
    if ( m_byteSize > 0 )
        bufferSize += m_byteSize + (UInt32) kvSepCpSize;

    StringMap::const_iterator i = elements.begin();
    while ( i != elements.end() )
    {
        const CUtf8String& key = (*i).first;
        const CUtf8String& value = (*i).second;

        UInt32 keySize = key.IsNULLOrEmpty() ? 0 : key.m_byteSize-1;
        UInt32 valueSize = value.IsNULLOrEmpty() ? 0 : value.m_byteSize-1;        
        bufferSize += keySize + valueSize;

        ++i;
    }

    // Allocate total storage

    CUtf8String comboStr;
    if ( GUCEF_NULL == comboStr.Reserve( bufferSize ) )
        return CUtf8String::Empty;

    // Write contents
    char* cpPos = comboStr.m_string;
    size_t bufferSpaceLeft = bufferSize;
    if ( GUCEF_NULL != m_string && 0 < m_length )
    {
        memcpy( comboStr.m_string, m_string, m_byteSize );
        bufferSpaceLeft -= m_byteSize;

        cpPos = (char*) utf8catcodepoint( cpPos, kvSeperator, bufferSpaceLeft );
        bufferSpaceLeft -= kvSepCpSize;
    }
    else
    {
        comboStr.m_string[ 0 ] = '\0';
    }
    i = elements.begin();
    while ( i != elements.end() )
    {
        const CUtf8String& key = (*i).first;
        const CUtf8String& value = (*i).second;

        UInt32 keySize = key.IsNULLOrEmpty() ? 0 : key.m_byteSize-1;
        UInt32 valueSize = value.IsNULLOrEmpty() ? 0 : value.m_byteSize-1;  

        memcpy( cpPos, key.m_string, keySize );
        cpPos += keySize;
        bufferSpaceLeft -= keySize;

        cpPos = (char*) utf8catcodepoint( cpPos, valueSeperator, bufferSpaceLeft );
        bufferSpaceLeft -= valueSepCpSize;

        memcpy( cpPos, value.m_string, valueSize );
        cpPos += valueSize;
        bufferSpaceLeft -= valueSize;

        ++i;
        if ( i != elements.end() )
        {
            cpPos = (char*) utf8catcodepoint( cpPos, kvSeperator, bufferSpaceLeft );
            bufferSpaceLeft -= kvSepCpSize;
        }

        *cpPos = '\0';
    }

    // Determine new string length codepoint wise
    comboStr.m_length = (UInt32) utf8len_s( comboStr.m_string, comboStr.m_byteSize );
    comboStr.m_string[ comboStr.m_byteSize-1 ] = '\0';
    return comboStr;
}

/*-------------------------------------------------------------------------*/

bool
CUtf8String::IsFormattingValid( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_string )
    {
        // Check if an invalid UTF8 code point is found
        size_t validCpCount = 0;
        return GUCEF_NULL == utf8valid_s( m_string, m_byteSize, &validCpCount );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CUtf8String::ReadUtf32CodePoint( CIOAccess* io, UInt32* utf32CodePoint )
{GUCEF_TRACE;

    if ( GUCEF_NULL != io && GUCEF_NULL != utf32CodePoint && !io->Eof() )
    {
        *utf32CodePoint = 0;

        char currentCodePoint[ 5 ] = {0,0,0,0,0};
        currentCodePoint[ 0 ] = io->GetChar();

        // by inspecting the first byte of a UTF-8 character, you can determine how many bytes the character is encoded in.
        // this tells us how many more bytes we need to read from IO to complete the code point
        size_t codepointSizeInBytes = utf8codepointcalcsize( currentCodePoint );

        if ( codepointSizeInBytes > 1 )
        {
            // read the rest of the codepoint bytes
            if ( 1 != io->Read( &currentCodePoint[ 1 ], (UInt32) codepointSizeInBytes-1, 1 ) )
            {
                // corrupt codepoint
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "CUtf8String:ReadUtf32CodePoint: Encountered corrupt codepoint - insufficient bytes" );
                return false; 
            }
        }

        utf8codepoint( currentCodePoint, utf32CodePoint );
        return true;

    }             
    return false;
}

/*-------------------------------------------------------------------------*/

CUtf8String 
CUtf8String::ReadLine( CIOAccess* io )
{GUCEF_TRACE;
    
    if ( GUCEF_NULL != io )
    {
        CUtf8String resultStr;
        UInt32 utf32CodePoint = 0;
        while ( ReadUtf32CodePoint( io, &utf32CodePoint ) )
        {
            if ( utf32CodePoint == '\0'        ||
                 utf32CodePoint == '\n'        || 
                 utf32CodePoint == '\r'        ||
                 utf32CodePoint == GUCEF_DOS_EOF_CHAR )
            {
                break;
            }

            resultStr.Append( utf32CodePoint );
        }
        return resultStr;
    }     
        
    return CUtf8String();
}

/*-------------------------------------------------------------------------*/

CUtf8String 
CUtf8String::ReadString( CIOAccess* io )
{GUCEF_TRACE;
    
    if ( GUCEF_NULL != io )
    {
        CUtf8String resultStr;
        UInt32 utf32CodePoint = 0;
        while ( ReadUtf32CodePoint( io, &utf32CodePoint ) )
        {
            if ( utf32CodePoint == '\0'               ||
                 utf32CodePoint == '\n'               || 
                 utf32CodePoint == '\r'               ||
                 utf32CodePoint == GUCEF_DOS_EOF_CHAR ||
                 utf32CodePoint == '\t'               ||
                 utf32CodePoint == ' '                 )
            {
                break;
            }

            resultStr.Append( utf32CodePoint );
        }
        return resultStr;
    }     
        
    return CUtf8String();
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::operator+( const CUtf8String& addition ) const
{GUCEF_TRACE;

    UInt32 neededBytes = ByteSize() + addition.ByteSize();

    // since we are using 2 string objects we know we have 2 null terminators potentially
    if ( Length() > 0 && addition.Length() > 0 && neededBytes > 0 )
        --neededBytes;

    // reserve enough memory in 1 step since allocation is the most costly step here
    CUtf8String tmp;
    tmp.Reserve( neededBytes );

    // combine the strings into the reserved memory
    tmp.Set( *this );
    tmp.Append( addition );

    return tmp;
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::operator+( const char* addition ) const
{GUCEF_TRACE;

    CUtf8String tmp( *this );
    tmp += addition;
    return tmp;
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8String::operator+( const char addition ) const
{GUCEF_TRACE;

    UInt32 neededBytes = ByteSize() + 1;

    // reserve enough memory in 1 step since allocation is the most costly step here
    CUtf8String tmp;
    tmp.Reserve( neededBytes );

    // combine the strings into the reserved memory
    tmp.Set( *this );
    tmp.Append( &addition, 1, 1 );

    return tmp;
}

/*-------------------------------------------------------------------------*/

CUtf8String
operator+( const char* lhs    ,
           const CUtf8String& rhs )
{GUCEF_TRACE;

    CUtf8String tmp( lhs );
    tmp += rhs;
    return tmp;
}

/*-------------------------------------------------------------------------*/

bool
operator==( const char* lhs    ,
            const CUtf8String& rhs )
{GUCEF_TRACE;

    return rhs == lhs;
}

/*-------------------------------------------------------------------------*/

bool
operator!=( const char* lhs    ,
            const CUtf8String& rhs )
{GUCEF_TRACE;

    return rhs != lhs;
}

/*-------------------------------------------------------------------------*/

CUtf8String
operator+( const char lhs     ,
           const CUtf8String& rhs )
{GUCEF_TRACE;

    return CUtf8String( &lhs, 1 ) + rhs;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
