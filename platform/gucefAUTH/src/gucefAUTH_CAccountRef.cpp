/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#include "gucefAUTH_CAccountRef.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAuthProviderIds::Epic = "epic";
const CORE::CString CAuthProviderIds::Facebook = "facebook";
const CORE::CString CAuthProviderIds::Google = "google";
const CORE::CString CAuthProviderIds::Microsoft = "microsoft";
const CORE::CString CAuthProviderIds::Amazon = "amazon";
const CORE::CString CAuthProviderIds::Aws = "aws";

const UInt32 CAccountRef::MaxProviderLength;
const UInt32 CAccountRef::MaxSubjectLength;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
IsSubjectChar( char c )
{GUCEF_TRACE;

    return ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) ||
           '.' == c || '_' == c || '-' == c || ':' == c || '@' == c || '~' == c || '+' == c || '=' == c || '|' == c;
}

/*-------------------------------------------------------------------------*/

static bool
IsUpperHexDigit( char c )
{GUCEF_TRACE;

    return ( c >= '0' && c <= '9' ) || ( c >= 'A' && c <= 'F' );
}

/*-------------------------------------------------------------------------*/

static Int32
HexDigitValue( char c )
{GUCEF_TRACE;

    if ( c >= '0' && c <= '9' )
        return c - '0';
    if ( c >= 'A' && c <= 'F' )
        return c - 'A' + 10;
    if ( c >= 'a' && c <= 'f' )
        return c - 'a' + 10;
    return -1;
}

/*-------------------------------------------------------------------------*/

CAccountRef::CAccountRef( void )
    : m_provider()
    , m_subject()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAccountRef::CAccountRef( const CORE::CString& provider, const CORE::CString& subject )
    : m_provider()
    , m_subject()
{GUCEF_TRACE;

    CORE::CString normalised = provider.Lowercase();
    if ( IsValidProvider( normalised ) && IsValidSubject( subject ) )
    {
        m_provider = normalised;
        m_subject = subject;
    }
}

/*-------------------------------------------------------------------------*/

CAccountRef::CAccountRef( const CAccountRef& src )
    : m_provider( src.m_provider )
    , m_subject( src.m_subject )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAccountRef&
CAccountRef::operator=( const CAccountRef& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_provider = src.m_provider;
        m_subject = src.m_subject;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CAccountRef::TryParse( const CORE::CString& text, CAccountRef& ref )
{GUCEF_TRACE;

    ref = CAccountRef();
    Int32 separator = text.HasChar( ':', true );
    if ( separator <= 0 )
        return false;
    CAccountRef parsed( text.SubstrToIndex( (UInt32) separator, true ), text.SubstrToIndex( (UInt32) separator + 1, false ) );
    if ( !parsed.IsValid() )
        return false;
    ref = parsed;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAccountRef::IsValidProvider( const CORE::CString& provider )
{GUCEF_TRACE;

    UInt32 length = GetByteLength( provider );
    if ( 0 == length || length > MaxProviderLength )
        return false;
    const char* text = provider.C_String();
    if ( text[ 0 ] < 'a' || text[ 0 ] > 'z' )
        return false;
    for ( UInt32 i=1; i<length; ++i )
    {
        char c = text[ i ];
        if ( !( ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || '-' == c ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAccountRef::IsValidSubject( const CORE::CString& subject )
{GUCEF_TRACE;

    UInt32 length = GetByteLength( subject );
    if ( 0 == length || length > MaxSubjectLength )
        return false;
    const char* text = subject.C_String();
    for ( UInt32 i=0; i<length; ++i )
    {
        if ( IsSubjectChar( text[ i ] ) )
            continue;
        // an escape of EncodeSubject(): % and two upper case hex digits
        if ( '%' == text[ i ] && i + 2 < length && IsUpperHexDigit( text[ i + 1 ] ) && IsUpperHexDigit( text[ i + 2 ] ) )
        {
            i += 2;
            continue;
        }
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAccountRef::EncodeSubject( const CORE::CString& rawSubject )
{GUCEF_TRACE;

    static const char hexDigits[] = "0123456789ABCDEF";
    CORE::CString encoded;
    const char* text = rawSubject.C_String();
    UInt32 length = GetByteLength( rawSubject );
    for ( UInt32 i=0; i<length; ++i )
    {
        char c = text[ i ];
        if ( IsSubjectChar( c ) )
        {
            encoded += c;
        }
        else
        {
            encoded += '%';
            encoded += hexDigits[ ( (UInt8) c ) >> 4 ];
            encoded += hexDigits[ ( (UInt8) c ) & 0x0F ];
        }
    }
    return encoded;
}

/*-------------------------------------------------------------------------*/

bool
CAccountRef::DecodeSubject( const CORE::CString& encodedSubject, CORE::CString& rawSubject )
{GUCEF_TRACE;

    rawSubject.Clear();
    // the bytes of the subject are collected first: a string takes whole characters, not single bytes of a multi byte one
    CORE::CDynamicBuffer bytes;
    const char* text = encodedSubject.C_String();
    UInt32 length = GetByteLength( encodedSubject );
    for ( UInt32 i=0; i<length; ++i )
    {
        if ( '%' != text[ i ] )
        {
            bytes.Append( &text[ i ], 1 );
            continue;
        }
        if ( i + 2 >= length )
        {
            rawSubject.Clear();
            return false;
        }
        Int32 high = HexDigitValue( text[ i + 1 ] );
        Int32 low = HexDigitValue( text[ i + 2 ] );
        if ( high < 0 || low < 0 )
        {
            rawSubject.Clear();
            return false;
        }
        if ( 0 == ( ( high << 4 ) | low ) )
        {
            return false;
        }
        char decoded = (char) ( ( high << 4 ) | low );
        bytes.Append( &decoded, 1 );
        i += 2;
    }
    if ( bytes.GetDataSize() > 0 )
        rawSubject = CORE::CString( bytes.AsConstTypePtr< char >(), bytes.GetDataSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAccountRef::IsValid( void ) const
{GUCEF_TRACE;

    return !m_provider.IsNULLOrEmpty() && !m_subject.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CAccountRef::IsEmpty( void ) const
{GUCEF_TRACE;

    return m_provider.IsNULLOrEmpty() && m_subject.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAccountRef::GetProvider( void ) const
{GUCEF_TRACE;

    return m_provider;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAccountRef::GetSubject( void ) const
{GUCEF_TRACE;

    return m_subject;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAccountRef::ToString( void ) const
{GUCEF_TRACE;

    if ( !IsValid() )
        return CORE::CString::Empty;
    CORE::CString text = m_provider;
    text += ':';
    text += m_subject;
    return text;
}

/*-------------------------------------------------------------------------*/

UInt32
CAccountRef::GetHash( void ) const
{GUCEF_TRACE;

    UInt32 hash = 2166136261U;
    const char* provider = m_provider.C_String();
    UInt32 length = GetByteLength( m_provider );
    for ( UInt32 i=0; i<length; ++i )
        hash = ( hash ^ (UInt8) provider[ i ] ) * 16777619U;
    hash = ( hash ^ (UInt8) ':' ) * 16777619U;
    const char* subject = m_subject.C_String();
    length = GetByteLength( m_subject );
    for ( UInt32 i=0; i<length; ++i )
        hash = ( hash ^ (UInt8) subject[ i ] ) * 16777619U;
    return hash;
}

/*-------------------------------------------------------------------------*/

bool
CAccountRef::operator==( const CAccountRef& other ) const
{GUCEF_TRACE;

    return m_provider == other.m_provider && m_subject == other.m_subject;
}

/*-------------------------------------------------------------------------*/

bool
CAccountRef::operator!=( const CAccountRef& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CAccountRef::operator<( const CAccountRef& other ) const
{GUCEF_TRACE;

    if ( m_provider != other.m_provider )
        return m_provider < other.m_provider;
    return m_subject < other.m_subject;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
