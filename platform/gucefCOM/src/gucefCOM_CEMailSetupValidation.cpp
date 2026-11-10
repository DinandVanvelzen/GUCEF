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


#include <string.h>

#include "gucefCOM_CEMailSetupValidation.h"

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
    const UInt32 size = text.ByteSize();
    return size > 0 ? size - 1 : 0;
}

inline bool
IsAlnum( char c )
{
    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' );
}

inline bool
IsDigit( char c )
{
    return c >= '0' && c <= '9';
}

/** The characters of a local part besides the dot: letters, digits and the atext symbols of RFC 5322 */
inline bool
IsLocalChar( char c )
{
    return IsAlnum( c ) || ( '\0' != c && NULL != strchr( "!#$%&'*+-/=?^_`{|}~", c ) );
}

/** Whether the text is made of printable ASCII only: nothing below the space, nothing above the tilde */
bool
IsPrintableAscii( const char* p, UInt32 size )
{
    for ( UInt32 i=0; i<size; ++i )
    {
        const UInt8 c = (UInt8) p[ i ];
        if ( c <= 0x20 || c >= 0x7F )
            return false;
    }
    return true;
}

/** Labels of letters, digits and hyphens; the labels are at least 'minLabels' */
bool
IsDomainName( const char* p, UInt32 size, UInt32 minLabels )
{
    if ( 0 == size || size > CEMailSetupValidation::MaxDomainLength )
        return false;
    UInt32 labelLength = 0;
    UInt32 labels = 0;
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
        if ( !( IsAlnum( c ) || '-' == c ) || ( '-' == c && 0 == labelLength ) )
            return false;
        if ( 0 == labelLength )
            ++labels;
        if ( ++labelLength > CEMailSetupValidation::MaxLabelLength )
            return false;
    }
    return 0 != labelLength && '-' != p[ size - 1 ] && labels >= minLabels;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CEMailSetupValidation::SplitAddress( const CORE::CString& address, CORE::CString& localPart, CORE::CString& domain )
{GUCEF_TRACE;

    const UInt32 size = ByteLength( address );
    if ( 0 == size || size > MaxAddressLength )
        return false;
    const char* p = address.C_String();
    if ( !IsPrintableAscii( p, size ) )
        return false;
    UInt32 at = size;
    for ( UInt32 i=0; i<size; ++i )
        if ( '@' == p[ i ] )
        {
            if ( at != size )
                return false;
            at = i;
        }
    if ( size == at || 0 == at || at > MaxLocalPartLength || at + 1 >= size )
        return false;

    if ( '.' == p[ 0 ] || '.' == p[ at - 1 ] )
        return false;
    for ( UInt32 i=0; i<at; ++i )
    {
        if ( '.' == p[ i ] )
        {
            if ( i > 0 && '.' == p[ i - 1 ] )
                return false;
        }
        else
        if ( !IsLocalChar( p[ i ] ) )
            return false;
    }
    if ( !IsDomainName( p + at + 1, size - at - 1, 2 ) )
        return false;
    localPart = CORE::CString( p, at );
    domain = CORE::CString( p + at + 1, size - at - 1 );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEMailSetupValidation::GetDomain( const CORE::CString& address )
{GUCEF_TRACE;

    CORE::CString localPart, domain;
    if ( !SplitAddress( address, localPart, domain ) )
        return CORE::CString::Empty;
    return domain.Lowercase();
}

/*-------------------------------------------------------------------------*/

bool
CEMailSetupValidation::IsValidHostName( const CORE::CString& host )
{GUCEF_TRACE;

    const UInt32 size = ByteLength( host );
    const char* p = host.C_String();
    if ( 0 == size || size > MaxDomainLength || !IsDomainName( p, size, 1 ) )
        return false;

    // the last label of a name is never a number: a name of numbers is an IPv4 address, and nothing else that starts with a digit is a name
    UInt32 lastStart = size;
    while ( lastStart > 0 && '.' != p[ lastStart - 1 ] )
        --lastStart;
    bool numbersOnly = true;
    for ( UInt32 i=lastStart; i<size; ++i )
        numbersOnly = numbersOnly && IsDigit( p[ i ] );
    if ( numbersOnly )
    {
        UInt32 parts = 0;
        UInt32 value = 0;
        UInt32 digits = 0;
        for ( UInt32 i=0; i<=size; ++i )
        {
            if ( i == size || '.' == p[ i ] )
            {
                if ( 0 == digits || value > 255 )
                    return false;
                ++parts;
                value = 0;
                digits = 0;
            }
            else
            {
                if ( ++digits > 3 )
                    return false;
                value = value * 10 + (UInt32) ( p[ i ] - '0' );
            }
        }
        return 4 == parts;
    }
    return !IsDigit( p[ lastStart ] );
}

/*-------------------------------------------------------------------------*/

bool
CEMailSetupValidation::IsPlainText( const CORE::CString& text, UInt32 maxLength )
{GUCEF_TRACE;

    const UInt32 size = ByteLength( text );
    if ( size > maxLength )
        return false;
    const char* p = text.C_String();
    for ( UInt32 i=0; i<size; ++i )
    {
        const UInt8 c = (UInt8) p[ i ];
        if ( c < 0x20 || 0x7F == c )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailSetupValidation::CheckAddress( const CORE::CString& address, CORE::CString& problem )
{GUCEF_TRACE;

    CORE::CString localPart, domain;
    if ( SplitAddress( address, localPart, domain ) )
        return true;
    if ( ByteLength( address ) > MaxAddressLength )
        problem = "The mail address is too long.";
    else
    if ( !IsPlainText( address, MaxAddressLength ) )
        problem = "The mail address has a character that cannot be in an address, such as a line break.";
    else
        problem = "This is not a mail address that can be used. Type it in full, like name@example.org: no spaces, one @ and a domain with a dot.";
    return false;
}

bool
CEMailSetupValidation::CheckHost( const CORE::CString& host, CORE::CString& problem )
{GUCEF_TRACE;

    if ( IsValidHostName( host ) )
        return true;
    if ( ByteLength( host ) > MaxDomainLength )
        problem = "The name of the server is too long.";
    else
    if ( !IsPlainText( host, MaxDomainLength ) )
        problem = "The name of the server has a character that cannot be in a name, such as a line break.";
    else
        problem = "The name of the server is not right. Type only the name, like imap.example.org: no spaces, no http://, no port and no slash.";
    return false;
}

bool
CEMailSetupValidation::CheckUserName( const CORE::CString& userName, CORE::CString& problem )
{GUCEF_TRACE;

    if ( IsPlainText( userName, MaxUserNameLength ) )
        return true;
    problem = ByteLength( userName ) > MaxUserNameLength ? "The user name is too long." : "The user name has a character that cannot be in a user name, such as a line break.";
    return false;
}

bool
CEMailSetupValidation::CheckFolderName( const CORE::CString& folder, CORE::CString& problem )
{GUCEF_TRACE;

    if ( IsPlainText( folder, MaxFolderNameLength ) )
        return true;
    problem = ByteLength( folder ) > MaxFolderNameLength ? "The name of the mail folder is too long." : "The name of the mail folder has a character that cannot be in a name, such as a line break.";
    return false;
}

bool
CEMailSetupValidation::CheckDisplayName( const CORE::CString& name, CORE::CString& problem )
{GUCEF_TRACE;

    if ( IsPlainText( name, MaxDisplayNameLength ) )
        return true;
    problem = ByteLength( name ) > MaxDisplayNameLength ? "Your name is too long." : "Your name has a character that cannot be in a name, such as a line break.";
    return false;
}

bool
CEMailSetupValidation::CheckPortText( const CORE::CString& text, UInt16& port, CORE::CString& problem )
{GUCEF_TRACE;

    port = 0;
    const UInt32 size = ByteLength( text );
    if ( 0 == size )
        return true;
    const char* p = text.C_String();
    UInt32 value = 0;
    bool ok = size <= 5;
    for ( UInt32 i=0; ok && i<size; ++i )
    {
        ok = IsDigit( p[ i ] );
        value = value * 10 + (UInt32) ( p[ i ] - '0' );
    }
    if ( !ok || 0 == value || value > 65535 )
    {
        problem = "The port is not right. It is a number from 1 to 65535, or empty for the usual one.";
        return false;
    }
    port = (UInt16) value;
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
