/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CMIMECODEC_H
#include "gucefCOM_CMimeCodec.h"
#define GUCEF_COM_CMIMECODEC_H
#endif /* GUCEF_COM_CMIMECODEC_H ? */

#include "gucefCOM_CEMailAddress.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

inline UInt32
ByteLength( const CORE::CString& text )
{
    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

inline bool
IsWhiteChar( char c )
{
    return ' ' == c || '\t' == c || '\r' == c || '\n' == c;
}

/*-------------------------------------------------------------------------*/

bool
IsAtextChar( char c, bool allowUtf8 )
{
    if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) )
        return true;
    if ( ( (UInt8) c ) >= 0x80 )
        return allowUtf8;
    return NULL != strchr( "!#$%&'*+-/=?^_`{|}~", c ) && '\0' != c;
}

/*-------------------------------------------------------------------------*/

bool
IsDotAtom( const char* p, UInt32 size, bool allowUtf8 )
{
    if ( 0 == size || '.' == p[ 0 ] || '.' == p[ size - 1 ] )
        return false;
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '.' == p[ i ] )
        {
            if ( i > 0 && '.' == p[ i - 1 ] )
                return false;
        }
        else
        if ( !IsAtextChar( p[ i ], allowUtf8 ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
IsValidDomain( const char* p, UInt32 size, bool allowUtf8 )
{
    if ( 0 == size || size > 253 )
        return false;

    if ( '[' == p[ 0 ] )
    {
        // domain literal: [192.0.2.1] or [IPv6:2001:db8::1]
        if ( size < 4 || ']' != p[ size - 1 ] )
            return false;
        for ( UInt32 i=1; i+1<size; ++i )
        {
            const char c = p[ i ];
            if ( !( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' ) || '.' == c || ':' == c || 'I' == c || 'P' == c || 'v' == c || '6' == c ) )
                return false;
        }
        return true;
    }

    UInt32 labelLength = 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        const char c = p[ i ];
        if ( '.' == c )
        {
            if ( 0 == labelLength || '-' == p[ i - 1 ] )
                return false;
            labelLength = 0;
            continue;
        }
        const bool alnum = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' );
        const bool high = ( (UInt8) c ) >= 0x80;
        if ( !( alnum || '-' == c || '_' == c || ( high && allowUtf8 ) ) )
            return false;
        if ( '-' == c && 0 == labelLength )
            return false;
        if ( ++labelLength > 63 * ( high ? 4 : 1 ) )
            return false;
    }
    return labelLength > 0 && '-' != p[ size - 1 ];
}

/*-------------------------------------------------------------------------*/

/** Removes the comments (outside quoted strings) and returns the text of the last one */
CORE::CString
StripComments( const CORE::CString& text, CORE::CString& lastComment )
{
    const char* p = text.C_String();
    const UInt32 size = ByteLength( text );
    GUCEF::vector< char > out;
    GUCEF::vector< char > comment;
    bool quoted = false;
    int depth = 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        const char c = p[ i ];
        if ( depth > 0 )
        {
            if ( '\\' == c && i + 1 < size ) { comment.push_back( p[ ++i ] ); continue; }
            if ( '(' == c ) { ++depth; comment.push_back( c ); continue; }
            if ( ')' == c ) { if ( --depth > 0 ) comment.push_back( c ); else { lastComment = comment.empty() ? CORE::CString() : CORE::CString( &comment[ 0 ], (UInt32) comment.size() ); comment.clear(); } continue; }
            comment.push_back( c );
            continue;
        }
        if ( quoted )
        {
            out.push_back( c );
            if ( '\\' == c && i + 1 < size ) out.push_back( p[ ++i ] );
            else if ( '"' == c ) quoted = false;
            continue;
        }
        if ( '"' == c ) { quoted = true; out.push_back( c ); continue; }
        if ( '(' == c ) { depth = 1; continue; }
        out.push_back( c );
    }
    return out.empty() ? CORE::CString() : CORE::CString( &out[ 0 ], (UInt32) out.size() );
}

/*-------------------------------------------------------------------------*/

CORE::CString
Trimmed( const char* p, UInt32 size )
{
    UInt32 first = 0;
    while ( first < size && IsWhiteChar( p[ first ] ) )
        ++first;
    while ( size > first && IsWhiteChar( p[ size - 1 ] ) )
        --size;
    return size > first ? CORE::CString( p + first, size - first ) : CORE::CString();
}

/*-------------------------------------------------------------------------*/

/** The words of a phrase with the quotes taken off ("Doe, Jane" becomes Doe, Jane) */
CORE::CString
UnquotePhrase( const CORE::CString& phrase )
{
    const char* p = phrase.C_String();
    const UInt32 size = ByteLength( phrase );
    GUCEF::vector< char > out;
    bool quoted = false;
    for ( UInt32 i=0; i<size; ++i )
    {
        const char c = p[ i ];
        if ( quoted )
        {
            if ( '\\' == c && i + 1 < size ) { out.push_back( p[ ++i ] ); continue; }
            if ( '"' == c ) { quoted = false; continue; }
            out.push_back( c );
            continue;
        }
        if ( '"' == c ) { quoted = true; continue; }
        if ( '\r' == c || '\n' == c ) continue;
        out.push_back( c );
    }
    return out.empty() ? CORE::CString() : Trimmed( &out[ 0 ], (UInt32) out.size() );
}

/*-------------------------------------------------------------------------*/

bool
ParseMailbox( const CORE::CString& element, CEMailAddress& address )
{
    CORE::CString comment;
    CORE::CString text = StripComments( element, comment );
    const char* p = text.C_String();
    const UInt32 size = ByteLength( text );

    // the '<' outside of quotes
    Int32 open = -1;
    bool quoted = false;
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( quoted )
        {
            if ( '\\' == p[ i ] ) ++i;
            else if ( '"' == p[ i ] ) quoted = false;
        }
        else
        if ( '"' == p[ i ] ) quoted = true;
        else
        if ( '<' == p[ i ] ) { open = (Int32) i; break; }
    }

    CORE::CString name;
    CORE::CString addrText;
    if ( open >= 0 )
    {
        UInt32 close = (UInt32) open + 1;
        while ( close < size && '>' != p[ close ] )
            ++close;
        if ( close >= size )
            return false;
        addrText = Trimmed( p + open + 1, close - (UInt32) open - 1 );
        name = UnquotePhrase( CORE::CString( p, (UInt32) open ) );

        // an obsolete source route, @a.example,@b.example:user@c.example
        if ( ByteLength( addrText ) > 0 && '@' == addrText.C_String()[ 0 ] )
        {
            Int32 colon = addrText.HasChar( ':' );
            if ( colon < 0 )
                return false;
            addrText = addrText.SubstrFromRange( (UInt32) colon + 1, addrText.Length() );
        }
    }
    else
    {
        addrText = Trimmed( p, size );
        name = UnquotePhrase( comment );
    }

    CEMailAddress parsed;
    if ( !parsed.SetAddress( addrText ) )
        return false;
    // without quotes the local part is a dot-atom: a space in it is a sign of text that is no address
    if ( ByteLength( addrText ) > 0 && '"' != addrText.C_String()[ 0 ] && !IsDotAtom( parsed.localPart.C_String(), ByteLength( parsed.localPart ), true ) )
        return false;
    parsed.displayName = CMimeCodec::DecodeHeaderText( name );
    if ( !parsed.IsValid( true ) )
        return false;
    address = parsed;
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailAddress::CEMailAddress( void )
    : displayName()
    , localPart()
    , domain()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailAddress::CEMailAddress( const CORE::CString& addrSpec )
    : displayName()
    , localPart()
    , domain()
{GUCEF_TRACE;

    SetAddress( addrSpec );
}

/*-------------------------------------------------------------------------*/

CEMailAddress::CEMailAddress( const CORE::CString& name, const CORE::CString& addrSpec )
    : displayName( name )
    , localPart()
    , domain()
{GUCEF_TRACE;

    SetAddress( addrSpec );
}

/*-------------------------------------------------------------------------*/

CEMailAddress::CEMailAddress( const CEMailAddress& src )
    : displayName( src.displayName )
    , localPart( src.localPart )
    , domain( src.domain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailAddress::~CEMailAddress()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailAddress&
CEMailAddress::operator=( const CEMailAddress& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        displayName = src.displayName;
        localPart = src.localPart;
        domain = src.domain;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CEMailAddress::IsEmpty( void ) const
{GUCEF_TRACE;

    return localPart.IsNULLOrEmpty() && domain.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEMailAddress::GetAddress( void ) const
{GUCEF_TRACE;

    if ( IsEmpty() )
        return CORE::CString();

    const char* p = localPart.C_String();
    const UInt32 size = ByteLength( localPart );
    if ( IsDotAtom( p, size, true ) )
        return localPart + "@" + domain;

    GUCEF::vector< char > quoted;
    quoted.push_back( '"' );
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '"' == p[ i ] || '\\' == p[ i ] )
            quoted.push_back( '\\' );
        quoted.push_back( p[ i ] );
    }
    quoted.push_back( '"' );
    return CORE::CString( &quoted[ 0 ], (UInt32) quoted.size() ) + "@" + domain;
}

/*-------------------------------------------------------------------------*/

bool
CEMailAddress::SetAddress( const CORE::CString& addrSpec )
{GUCEF_TRACE;

    localPart.Clear();
    domain.Clear();

    CORE::CString spec = Trimmed( addrSpec.C_String(), ByteLength( addrSpec ) );
    if ( ByteLength( spec ) >= 2 && '<' == spec.C_String()[ 0 ] && '>' == spec.C_String()[ ByteLength( spec ) - 1 ] )
        spec = Trimmed( spec.C_String() + 1, ByteLength( spec ) - 2 );
    const char* p = spec.C_String();
    const UInt32 size = ByteLength( spec );

    // the separating @ is the last one outside of a quoted local part
    UInt32 start = 0;
    bool quotedLocal = size > 0 && '"' == p[ 0 ];
    if ( quotedLocal )
    {
        UInt32 i = 1;
        while ( i < size && '"' != p[ i ] )
            i += ( '\\' == p[ i ] && i + 1 < size ) ? 2 : 1;
        if ( i >= size )
            return false;
        start = i + 1;
    }
    Int32 at = -1;
    for ( UInt32 i=start; i<size; ++i )
        if ( '@' == p[ i ] )
            at = (Int32) i;
    if ( at <= 0 || (UInt32) at + 1 >= size )
        return false;

    GUCEF::vector< char > local;
    if ( quotedLocal )
    {
        for ( UInt32 i=1; i<(UInt32) at; ++i )
        {
            if ( '\\' == p[ i ] && i + 1 < (UInt32) at ) { local.push_back( p[ ++i ] ); continue; }
            if ( '"' == p[ i ] ) continue;
            local.push_back( p[ i ] );
        }
    }
    else
        local.insert( local.end(), p, p + at );
    if ( local.empty() )
        return false;

    localPart = CORE::CString( &local[ 0 ], (UInt32) local.size() );
    domain = CORE::CString( p + at + 1, size - (UInt32) at - 1 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailAddress::IsValid( bool allowUtf8 ) const
{GUCEF_TRACE;

    const UInt32 localSize = ByteLength( localPart );
    const UInt32 domainSize = ByteLength( domain );
    if ( 0 == localSize || localSize > 64 || 0 == domainSize )
        return false;

    const char* l = localPart.C_String();
    if ( !IsDotAtom( l, localSize, allowUtf8 ) )
    {
        // a quoted local part may hold more, but nothing that is a control character
        for ( UInt32 i=0; i<localSize; ++i )
        {
            const UInt8 c = (UInt8) l[ i ];
            if ( c < 0x20 || 0x7F == c || ( c >= 0x80 && !allowUtf8 ) )
                return false;
        }
    }
    if ( !IsValidDomain( domain.C_String(), domainSize, allowUtf8 ) )
        return false;
    return ByteLength( GetAddress() ) <= 254;
}

/*-------------------------------------------------------------------------*/

bool
CEMailAddress::IsInternational( void ) const
{GUCEF_TRACE;

    return !CMimeCodec::IsAscii( localPart.C_String(), ByteLength( localPart ) ) || !CMimeCodec::IsAscii( domain.C_String(), ByteLength( domain ) );
}

/*-------------------------------------------------------------------------*/

bool
CEMailAddress::IsSameAddress( const CEMailAddress& other ) const
{GUCEF_TRACE;

    return localPart == other.localPart && domain.Equals( other.domain, false );
}

/*-------------------------------------------------------------------------*/

bool
CEMailAddress::Format( CORE::CString& text, bool allowUtf8 ) const
{GUCEF_TRACE;

    text.Clear();
    if ( !IsValid( allowUtf8 ) )
        return false;

    CORE::CString address = GetAddress();
    if ( displayName.IsNULLOrEmpty() )
    {
        text = address;
        return true;
    }

    CORE::CString phrase;
    if ( !CMimeCodec::IsPlainHeaderText( displayName ) )
        phrase = CMimeCodec::EncodeHeaderText( displayName );
    else
    {
        const char* p = displayName.C_String();
        const UInt32 size = ByteLength( displayName );
        bool needsQuotes = false;
        for ( UInt32 i=0; i<size; ++i )
            if ( NULL != strchr( "()<>[]:;@\\,.\"", p[ i ] ) )
                needsQuotes = true;
        if ( needsQuotes )
        {
            GUCEF::vector< char > quoted;
            quoted.push_back( '"' );
            for ( UInt32 i=0; i<size; ++i )
            {
                if ( '"' == p[ i ] || '\\' == p[ i ] )
                    quoted.push_back( '\\' );
                quoted.push_back( p[ i ] );
            }
            quoted.push_back( '"' );
            phrase = CORE::CString( &quoted[ 0 ], (UInt32) quoted.size() );
        }
        else
            phrase = displayName;
    }
    text = phrase + " <" + address + ">";
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailAddress::Parse( const CORE::CString& headerText, CEMailAddress& address )
{GUCEF_TRACE;

    TEMailAddressVector found;
    if ( 0 == ParseList( headerText, found, 1 ) )
        return false;
    address = found[ 0 ];
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CEMailAddress::ParseList( const CORE::CString& headerText, TEMailAddressVector& addresses, UInt32 maxEntries )
{GUCEF_TRACE;

    const char* p = headerText.C_String();
    UInt32 size = ByteLength( headerText );
    if ( size > 65536 )
        size = 65536;

    UInt32 added = 0;
    UInt32 elementStart = 0;
    bool quoted = false;
    int commentDepth = 0;
    bool angle = false;
    bool sawAngle = false;

    for ( UInt32 i=0; i<=size && added<maxEntries; ++i )
    {
        char c = i < size ? p[ i ] : ',';
        if ( quoted )
        {
            if ( '\\' == c && i + 1 < size ) ++i;
            else if ( '"' == c ) quoted = false;
            continue;
        }
        if ( commentDepth > 0 )
        {
            if ( '\\' == c && i + 1 < size ) ++i;
            else if ( '(' == c ) ++commentDepth;
            else if ( ')' == c ) --commentDepth;
            continue;
        }
        if ( '"' == c ) { quoted = true; continue; }
        if ( '(' == c ) { commentDepth = 1; continue; }
        if ( '<' == c ) { angle = true; sawAngle = true; continue; }
        if ( '>' == c ) { angle = false; continue; }
        if ( angle )
            continue;

        if ( ':' == c && !sawAngle )
        {
            // a group: its name is not an address, the members follow up to the semicolon
            elementStart = i + 1;
            continue;
        }
        if ( ',' == c || ';' == c )
        {
            CEMailAddress address;
            if ( i > elementStart && ParseMailbox( CORE::CString( p + elementStart, i - elementStart ), address ) )
            {
                addresses.push_back( address );
                ++added;
            }
            elementStart = i + 1;
            sawAngle = false;
        }
    }
    return added;
}

/*-------------------------------------------------------------------------*/

bool
CEMailAddress::FormatList( const TEMailAddressVector& addresses, CORE::CString& text, bool allowUtf8 )
{GUCEF_TRACE;

    text.Clear();
    if ( addresses.empty() )
        return false;

    CORE::CString result;
    for ( size_t i=0; i<addresses.size(); ++i )
    {
        CORE::CString one;
        if ( !addresses[ i ].Format( one, allowUtf8 ) )
            return false;
        if ( i > 0 )
            result += ", ";
        result += one;
    }
    text = result;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
