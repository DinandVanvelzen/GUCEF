/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include "gucefCORE_CRedactor.h"

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

/** The most bytes of the name of a query item that are kept when its value is redacted */
static const UInt32 MaxQueryNameBytes = 64;

/** The most digits of a port */
static const UInt32 MaxPortDigits = 5;

/** The most bytes of the marker */
static const UInt32 MaxMarkerBytes = 64;

/** The most bytes of the name in front of a ':' or '=' that is taken for the name of a name/value pair */
static const UInt32 MaxPairNameBytes = 64;

/** The most bytes of an e-mail address that are looked at: the local part and the domain */
static const UInt32 MaxEmailLocalBytes = 64;
static const UInt32 MaxEmailDomainBytes = 255;

/** The most bytes of an IPv6 address */
static const UInt32 MaxIPv6Bytes = 45;

/** The most digits of a payment card number */
static const UInt32 MaxCardDigits = 19;

/** The bound of a result when the input and the marker do not limit it before that */
static const UInt32 MaxResultBytes = 0x7FFFFFF0;

/*-------------------------------------------------------------------------*/

static bool
IsAlpha( char c )
{
    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' );
}

/*-------------------------------------------------------------------------*/

static bool
IsDigit( char c )
{
    return c >= '0' && c <= '9';
}

/*-------------------------------------------------------------------------*/

static bool
IsAlphaNumeric( char c )
{
    return IsAlpha( c ) || IsDigit( c );
}

/*-------------------------------------------------------------------------*/

static bool
IsHexDigit( char c )
{
    return IsDigit( c ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' );
}

/*-------------------------------------------------------------------------*/

static bool
IsLower( char c )
{
    return c >= 'a' && c <= 'z';
}

/*-------------------------------------------------------------------------*/

static bool
IsUpper( char c )
{
    return c >= 'A' && c <= 'Z';
}

/*-------------------------------------------------------------------------*/

/** A character of a word: what a name, a scheme or a credential in text starts on the other side of */
static bool
IsWordChar( char c )
{
    return IsAlphaNumeric( c ) || '_' == c;
}

/*-------------------------------------------------------------------------*/

static bool
IsLineEnd( char c )
{
    return '\r' == c || '\n' == c;
}

/*-------------------------------------------------------------------------*/

/** Space, tab, the line ends and the other control characters up to the space */
static bool
IsWhiteSpace( char c )
{
    return (unsigned char) c <= ' ';
}

/*-------------------------------------------------------------------------*/

static char
ToLowerChar( char c )
{
    return IsUpper( c ) ? (char) ( c - 'A' + 'a' ) : c;
}

/*-------------------------------------------------------------------------*/

static Int32
HexValue( char c )
{
    if ( c >= '0' && c <= '9' )
        return c - '0';
    if ( c >= 'a' && c <= 'f' )
        return 10 + ( c - 'a' );
    if ( c >= 'A' && c <= 'F' )
        return 10 + ( c - 'A' );
    return -1;
}

/*-------------------------------------------------------------------------*/

/** Whether the bytes are the text, ignoring case of letters; the literal is lower case */
static bool
EqualsNoCase( const char* text, UInt32 length, const char* literal )
{
    UInt32 literalLength = (UInt32) strlen( literal );
    if ( length != literalLength )
        return false;
    for ( UInt32 i=0; i<length; ++i )
    {
        if ( ToLowerChar( text[ i ] ) != literal[ i ] )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Whether the bytes start with the text, ignoring case of letters; the literal is lower case */
static bool
StartsWithNoCase( const char* text, UInt32 length, const char* literal, UInt32 literalLength )
{
    if ( length < literalLength )
        return false;
    for ( UInt32 i=0; i<literalLength; ++i )
    {
        if ( ToLowerChar( text[ i ] ) != literal[ i ] )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Whether the bytes hold the text, ignoring case of letters; the literal is lower case */
static bool
ContainsNoCase( const char* text, UInt32 length, const char* literal )
{
    UInt32 literalLength = (UInt32) strlen( literal );
    if ( 0 == literalLength )
        return true;
    if ( length < literalLength )
        return false;
    for ( UInt32 i=0; i<=length-literalLength; ++i )
    {
        if ( StartsWithNoCase( text + i, length - i, literal, literalLength ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** The number of bytes of the string, which is not the length of it when it is made of characters of more than one byte */
static UInt32
ByteLength( const CString& text )
{
    return text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
}

/*-------------------------------------------------------------------------*/

/** The bytes as a string, the escapes of percent encoding undone (an escaped NUL becomes '?'), for the comparison with names */
static CString
DecodeForComparison( const char* text, UInt32 length )
{
    if ( 0 == length )
        return CString();
    if ( GUCEF_NULL == memchr( text, '%', length ) )
        return CString( text, length );

    GUCEF::vector< char > buffer( length + 1, '\0' );
    UInt32 written = 0;
    for ( UInt32 i=0; i<length; ++i )
    {
        char c = text[ i ];
        if ( '%' == c && i + 2 < length && HexValue( text[ i + 1 ] ) >= 0 && HexValue( text[ i + 2 ] ) >= 0 )
        {
            c = (char) ( HexValue( text[ i + 1 ] ) * 16 + HexValue( text[ i + 2 ] ) );
            i += 2;
            if ( 0 == c )
                c = '?';
        }
        buffer[ written++ ] = c;
    }
    buffer[ written ] = '\0';
    return CString( &buffer[ 0 ] );
}

/*-------------------------------------------------------------------------*/

/** Whether the text is a host name or address, with an optional port: reg-name, IPv4 or [IPv6], and :digits */
static bool
IsValidHostAndPort( const char* text, UInt32 length, bool allowNonAscii )
{
    if ( 0 == length )
        return true;

    UInt32 portStart = length;
    if ( '[' == text[ 0 ] )
    {
        UInt32 close = 1;
        while ( close < length && ']' != text[ close ] )
        {
            char c = text[ close ];
            if ( !IsAlphaNumeric( c ) && ':' != c && '.' != c && '%' != c && '-' != c && '_' != c && '~' != c )
                return false;
            ++close;
        }
        if ( close >= length )
            return false;
        if ( close + 1 == length )
            return true;
        if ( ':' != text[ close + 1 ] )
            return false;
        portStart = close + 2;
    }
    else
    {
        UInt32 i = 0;
        while ( i < length && ':' != text[ i ] )
        {
            char c = text[ i ];
            bool nonAscii = (unsigned char) c >= 0x80;
            if ( !IsAlphaNumeric( c ) && '.' != c && '%' != c && '-' != c && '_' != c && '~' != c && !( nonAscii && allowNonAscii ) )
                return false;
            ++i;
        }
        if ( i == length )
            return true;
        portStart = i + 1;
    }

    if ( length - portStart > MaxPortDigits )
        return false;
    for ( UInt32 i=portStart; i<length; ++i )
    {
        if ( !IsDigit( text[ i ] ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Whether the header holds a URL: its value is redacted as one */
static bool
IsUrlHeaderName( const CString& headerName )
{
    static const char* const urlHeaders[] =
    {
        "location", "content-location", "referer", "referrer", "origin", "refresh", "link", "sourcemap", "x-sourcemap"
    };

    if ( headerName.IsNULLOrEmpty() )
        return false;
    for ( UInt32 i=0; i<sizeof( urlHeaders ) / sizeof( urlHeaders[ 0 ] ); ++i )
    {
        if ( EqualsNoCase( headerName.C_String(), ByteLength( headerName ), urlHeaders[ i ] ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** Where a URL in text ends: it takes everything but white space, control characters and the characters which quote or enclose a URL */
static bool
IsUrlTerminator( char c )
{
    return IsWhiteSpace( c ) || '"' == c || '\'' == c || '<' == c || '>' == c || '`' == c || 0x7F == c;
}

/*-------------------------------------------------------------------------*/

/** The end of the URL (scheme://...) that starts at the position, or the position when there is none */
static UInt32
MatchUrl( const char* text, UInt32 length, UInt32 position )
{
    if ( position >= length || !IsAlpha( text[ position ] ) )
        return position;

    UInt32 schemeEnd = position + 1;
    while ( schemeEnd < length && schemeEnd - position < 17 &&
            ( IsAlphaNumeric( text[ schemeEnd ] ) || '+' == text[ schemeEnd ] || '-' == text[ schemeEnd ] || '.' == text[ schemeEnd ] ) )
    {
        ++schemeEnd;
    }
    if ( schemeEnd + 2 >= length || ':' != text[ schemeEnd ] || '/' != text[ schemeEnd + 1 ] || '/' != text[ schemeEnd + 2 ] )
        return position;

    UInt32 end = schemeEnd + 3;
    while ( end < length && !IsUrlTerminator( text[ end ] ) )
        ++end;

    // punctuation of the sentence that holds the URL is not of the URL, nor is a ')' that closes a '(' before the URL
    UInt32 opens = 0;
    UInt32 closes = 0;
    for ( UInt32 i=schemeEnd+3; i<end; ++i )
    {
        if ( '(' == text[ i ] )
            ++opens;
        else
        if ( ')' == text[ i ] )
            ++closes;
    }
    while ( end > schemeEnd + 3 )
    {
        char last = text[ end - 1 ];
        if ( '.' == last || ',' == last || ';' == last || ':' == last || '!' == last || '?' == last )
        {
            --end;
        }
        else
        if ( ')' == last && closes > opens )
        {
            --closes;
            --end;
        }
        else
            break;
    }
    return end;
}

/*-------------------------------------------------------------------------*/

static bool
IsEmailLocalChar( char c )
{
    return IsAlphaNumeric( c ) || '.' == c || '_' == c || '%' == c || '+' == c || '-' == c;
}

/*-------------------------------------------------------------------------*/

/** The end of the e-mail address that starts at the position, or the position when there is none */
static UInt32
MatchEmail( const char* text, UInt32 length, UInt32 position )
{
    UInt32 i = position;
    while ( i < length && i - position < MaxEmailLocalBytes && IsEmailLocalChar( text[ i ] ) )
        ++i;
    if ( i == position || i >= length || '@' != text[ i ] )
        return position;
    ++i;

    UInt32 domainStart = i;
    UInt32 labelCount = 0;
    UInt32 end = i;
    UInt32 lastLabelStart = i;
    while ( i < length && i - domainStart < MaxEmailDomainBytes )
    {
        UInt32 labelStart = i;
        while ( i < length && ( IsAlphaNumeric( text[ i ] ) || '-' == text[ i ] ) )
            ++i;
        if ( i == labelStart )
            break;
        ++labelCount;
        end = i;
        lastLabelStart = labelStart;
        if ( i + 1 < length && '.' == text[ i ] && IsAlphaNumeric( text[ i + 1 ] ) )
            ++i;
        else
            break;
    }
    if ( labelCount < 2 || end - lastLabelStart < 2 )
        return position;
    for ( UInt32 k=lastLabelStart; k<end; ++k )
    {
        if ( !IsAlpha( text[ k ] ) )
            return position;
    }
    return end;
}

/*-------------------------------------------------------------------------*/

/** The end of the IPv4 address (d.d.d.d) that starts at the position, or the position when there is none */
static UInt32
MatchIPv4( const char* text, UInt32 length, UInt32 position )
{
    UInt32 i = position;
    for ( UInt32 group=0; group<4; ++group )
    {
        UInt32 digitsStart = i;
        UInt32 value = 0;
        while ( i < length && IsDigit( text[ i ] ) && i - digitsStart < 4 )
        {
            value = value * 10 + (UInt32) ( text[ i ] - '0' );
            ++i;
        }
        UInt32 digitCount = i - digitsStart;
        if ( 0 == digitCount || digitCount > 3 || value > 255 )
            return position;
        if ( group < 3 )
        {
            if ( i >= length || '.' != text[ i ] )
                return position;
            ++i;
        }
    }

    // part of something longer, like a version number of five parts
    if ( i + 1 < length && '.' == text[ i ] && IsDigit( text[ i + 1 ] ) )
        return position;
    return i;
}

/*-------------------------------------------------------------------------*/

/** The end of the IPv6 address that starts at the position, or the position when there is none */
static UInt32
MatchIPv6( const char* text, UInt32 length, UInt32 position )
{
    UInt32 end = position;
    while ( end < length && end - position < MaxIPv6Bytes && ( IsHexDigit( text[ end ] ) || ':' == text[ end ] || '.' == text[ end ] ) )
        ++end;
    if ( end < length && IsAlphaNumeric( text[ end ] ) )
        return position;
    const UInt32 runLength = end - position;
    if ( runLength < 2 )
        return position;

    // an address does not start or end on a lone ':'
    if ( ':' == text[ position ] && ':' != text[ position + 1 ] )
        return position;
    if ( ':' == text[ end - 1 ] && ':' != text[ end - 2 ] )
        return position;

    bool hasHex = false;
    for ( UInt32 i=position; i<end; ++i )
    {
        if ( IsHexDigit( text[ i ] ) )
            hasHex = true;
    }
    if ( !hasHex )
        return position;

    UInt32 colons = 0;
    bool doubleColon = false;
    UInt32 groupStart = position;
    for ( UInt32 i=position; i<=end; ++i )
    {
        if ( i == end || ':' == text[ i ] )
        {
            UInt32 groupLength = i - groupStart;
            bool lastGroup = i == end;
            if ( groupLength > 0 )
            {
                bool hasDot = false;
                for ( UInt32 k=groupStart; k<i; ++k )
                {
                    if ( '.' == text[ k ] )
                        hasDot = true;
                }
                if ( hasDot )
                {
                    // the tail of an address that holds an IPv4 address
                    if ( !lastGroup || MatchIPv4( text, i, groupStart ) != i )
                        return position;
                }
                else
                if ( groupLength > 4 )
                    return position;
            }
            else
            if ( i > position && i < end && ':' == text[ i - 1 ] )
            {
                if ( doubleColon )
                    return position;
                doubleColon = true;
            }
            if ( i < end )
                ++colons;
            groupStart = i + 1;
        }
    }
    if ( colons < 2 )
        return position;
    if ( !doubleColon && colons < 5 )
        return position;
    return end;
}

/*-------------------------------------------------------------------------*/

/** The end of the payment card number that starts at the position (on a digit), or the position when there is none */
static UInt32
MatchCardNumber( const char* text, UInt32 length, UInt32 position )
{
    unsigned char digits[ MaxCardDigits ];
    UInt32 digitCount = 0;
    UInt32 i = position;
    UInt32 lastDigitEnd = position;
    while ( i < length && i - position < 32 )
    {
        if ( IsDigit( text[ i ] ) )
        {
            if ( digitCount >= MaxCardDigits )
                return position;
            digits[ digitCount++ ] = (unsigned char) ( text[ i ] - '0' );
            ++i;
            lastDigitEnd = i;
        }
        else
        if ( ( ' ' == text[ i ] || '-' == text[ i ] ) && digitCount > 0 && i + 1 < length && IsDigit( text[ i + 1 ] ) )
        {
            ++i;
        }
        else
            break;
    }
    if ( digitCount < 13 )
        return position;
    if ( lastDigitEnd < length && IsAlphaNumeric( text[ lastDigitEnd ] ) )
        return position;

    // Luhn
    UInt32 sum = 0;
    bool doubled = false;
    for ( UInt32 k=digitCount; k>0; --k )
    {
        UInt32 d = digits[ k - 1 ];
        if ( doubled )
        {
            d *= 2;
            if ( d > 9 )
                d -= 9;
        }
        sum += d;
        doubled = !doubled;
    }
    return 0 == ( sum % 10 ) ? lastDigitEnd : position;
}

/*-------------------------------------------------------------------------*/

static bool
IsTokenChar( char c )
{
    return IsAlphaNumeric( c ) || '+' == c || '/' == c || '_' == c || '-' == c;
}

/*-------------------------------------------------------------------------*/

/** Whether the run of token characters (letters, digits and + / _ -) looks like a key, digest or token and not like a word or a path */
static bool
IsTokenLike( const char* run, UInt32 length )
{
    bool allHex = true;
    bool hasDigit = false;
    bool hasLower = false;
    bool hasUpper = false;
    bool hasSlash = false;
    for ( UInt32 i=0; i<length; ++i )
    {
        char c = run[ i ];
        if ( !IsHexDigit( c ) )
            allHex = false;
        if ( IsDigit( c ) )
            hasDigit = true;
        else
        if ( IsLower( c ) )
            hasLower = true;
        else
        if ( IsUpper( c ) )
            hasUpper = true;
        else
        if ( '/' == c )
            hasSlash = true;
    }
    if ( allHex )
        return true;
    if ( !hasDigit || !( hasLower || hasUpper ) )
        return false;

    // a path is no token: it needs to look as random as a base64 string does
    if ( hasSlash )
        return hasLower && hasUpper && '/' != run[ 0 ];
    return true;
}

/*-------------------------------------------------------------------------*/

/** Whether the credential of the schemes with encoded credentials looks like an encoded credential and not like a word */
static bool
LooksEncoded( const char* credential, UInt32 length )
{
    if ( length < 8 )
        return false;
    for ( UInt32 i=0; i<length; ++i )
    {
        char c = credential[ i ];
        if ( IsDigit( c ) || '=' == c || '+' == c || '/' == c || ( i > 0 && IsUpper( c ) ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** Whether the character can be part of a credential after a scheme: letters, digits and -._~+/= */
static bool
IsCredentialChar( char c )
{
    return IsAlphaNumeric( c ) || '-' == c || '.' == c || '_' == c || '~' == c || '+' == c || '/' == c || '=' == c;
}

/*-------------------------------------------------------------------------*/

/** Whether the character is part of the name of a name/value pair */
static bool
IsPairNameChar( char c )
{
    return IsAlphaNumeric( c ) || '_' == c || '.' == c || '-' == c;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Collects the result in a buffer of the size that the input and the maximum length allow, which is on the
 *  stack for the sizes that logs and user interfaces need, and makes the string of it at the end, so that the
 *  result is one allocation. Nothing written beyond the size is kept.
 */
class CRedactor::COutput
{
    public:

    COutput( UInt32 capacity                ,
             const CString& marker          ,
             bool sanitizeControl           ,
             bool sanitizeNonAscii          )
        : m_buffer( m_inline )
        , m_capacity( capacity )
        , m_length( 0 )
        , m_full( false )
        , m_marker( marker.C_String() )
        , m_markerLength( ByteLength( marker ) )
        , m_sanitizeControl( sanitizeControl )
        , m_sanitizeNonAscii( sanitizeNonAscii )
    {
        if ( m_capacity >= InlineCapacity )
            m_buffer = GUCEF_NEW char[ (size_t) m_capacity + 1 ];
        m_buffer[ 0 ] = '\0';
    }

    ~COutput()
    {
        if ( m_buffer != m_inline )
            GUCEF_DELETE [] m_buffer;
    }

    bool IsFull( void ) const
    {
        return m_full;
    }

    /** Writes a byte of what is kept: it is replaced when it is of a kind that sanitation is asked for */
    void Put( char c )
    {
        if ( m_length >= m_capacity )
        {
            m_full = true;
            return;
        }
        unsigned char u = (unsigned char) c;
        if ( 0 == u || ( m_sanitizeControl && ( u < 0x20 || 0x7F == u ) ) || ( m_sanitizeNonAscii && u >= 0x80 ) )
            c = '?';
        m_buffer[ m_length++ ] = c;
    }

    void Put( const char* text, UInt32 length )
    {
        for ( UInt32 i=0; i<length && !m_full; ++i )
            Put( text[ i ] );
    }

    /** Writes a byte as it is, for what is structure and not content: the line ends of a header block */
    void PutRaw( char c )
    {
        if ( m_length >= m_capacity )
        {
            m_full = true;
            return;
        }
        m_buffer[ m_length++ ] = c;
    }

    void PutMarker( void )
    {
        for ( UInt32 i=0; i<m_markerLength && !m_full; ++i )
            PutRaw( m_marker[ i ] );
    }

    /** Ends the result: marks a cut and makes the string of it */
    CString Finish( void )
    {
        if ( m_full && m_capacity >= 4 )
        {
            // the last three bytes are the dots, and a UTF-8 character that the cut goes through is left out
            UInt32 end = m_capacity - 3;
            while ( end > 0 && 0x80 == ( (unsigned char) m_buffer[ end ] & 0xC0 ) )
                --end;
            m_buffer[ end ] = '.';
            m_buffer[ end + 1 ] = '.';
            m_buffer[ end + 2 ] = '.';
            m_length = end + 3;
        }
        m_buffer[ m_length ] = '\0';
        return CString( m_buffer );
    }

    private:

    static const UInt32 InlineCapacity = 1024;

    COutput( const COutput& src );
    COutput& operator=( const COutput& src );

    char m_inline[ InlineCapacity + 1 ];
    char* m_buffer;
    UInt32 m_capacity;
    UInt32 m_length;
    bool m_full;
    const char* m_marker;
    UInt32 m_markerLength;
    bool m_sanitizeControl;
    bool m_sanitizeNonAscii;
};

/*-------------------------------------------------------------------------*/

CRedactor::CRedactor( void )
    : m_flags( REDACT_NONE )
    , m_marker( GUCEF_CORE_REDACTION_MARKER )
    , m_maxLength( 0 )
    , m_tokenMinLength( 32 )
    , m_sensitiveNames()
    , m_queryNames()
    , m_pathSegments()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRedactor::CRedactor( TRedactionFlags flags )
    : m_flags( flags )
    , m_marker( GUCEF_CORE_REDACTION_MARKER )
    , m_maxLength( 0 )
    , m_tokenMinLength( 32 )
    , m_sensitiveNames()
    , m_queryNames()
    , m_pathSegments()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRedactor::CRedactor( const CRedactor& src )
    : m_flags( src.m_flags )
    , m_marker( src.m_marker )
    , m_maxLength( src.m_maxLength )
    , m_tokenMinLength( src.m_tokenMinLength )
    , m_sensitiveNames( src.m_sensitiveNames )
    , m_queryNames( src.m_queryNames )
    , m_pathSegments( src.m_pathSegments )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRedactor::~CRedactor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRedactor&
CRedactor::operator=( const CRedactor& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_flags = src.m_flags;
        m_marker = src.m_marker;
        m_maxLength = src.m_maxLength;
        m_tokenMinLength = src.m_tokenMinLength;
        m_sensitiveNames = src.m_sensitiveNames;
        m_queryNames = src.m_queryNames;
        m_pathSegments = src.m_pathSegments;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

static CRedactor
MakeUrlRedactor( void )
{
    CRedactor redactor( REDACT_URL_USERINFO | REDACT_URL_QUERY_VALUES | REDACT_URL_FRAGMENT | REDACT_URL_INVALID_HOST | REDACT_SANITIZE_ALL );
    redactor.SetMaxLength( 1024 );
    return redactor;
}

/*-------------------------------------------------------------------------*/

static CRedactor
MakeHeadersRedactor( void )
{
    CRedactor redactor( MakeUrlRedactor() );
    redactor.AddFlags( REDACT_HEADER_ALL | REDACT_NAME_WORDS );
    redactor.SetMaxLength( 16384 );
    return redactor;
}

/*-------------------------------------------------------------------------*/

static CRedactor
MakeLogLineRedactor( void )
{
    CRedactor redactor( REDACT_URL_USERINFO | REDACT_URL_QUERY_VALUES | REDACT_URL_FRAGMENT | REDACT_URL_INVALID_HOST | REDACT_NAME_WORDS |
                        REDACT_TEXT_URLS | REDACT_TEXT_AUTH_SCHEMES | REDACT_TEXT_KEY_VALUES | REDACT_TEXT_TOKEN_RUNS | REDACT_SANITIZE_CONTROL );
    redactor.SetMaxLength( 4096 );
    return redactor;
}

/*-------------------------------------------------------------------------*/

static CRedactor
MakeDisplayRedactor( void )
{
    CRedactor redactor( REDACT_URL_USERINFO | REDACT_URL_QUERY_LISTED | REDACT_URL_FRAGMENT | REDACT_URL_INVALID_HOST | REDACT_NAME_WORDS |
                        REDACT_TEXT_URLS | REDACT_TEXT_AUTH_SCHEMES | REDACT_TEXT_KEY_VALUES | REDACT_SANITIZE_CONTROL );
    redactor.SetMaxLength( 256 );
    return redactor;
}

/*-------------------------------------------------------------------------*/

const CRedactor&
CRedactor::ForUrl( void )
{GUCEF_TRACE;

    static const CRedactor redactor = MakeUrlRedactor();
    return redactor;
}

/*-------------------------------------------------------------------------*/

const CRedactor&
CRedactor::ForHeaders( void )
{GUCEF_TRACE;

    static const CRedactor redactor = MakeHeadersRedactor();
    return redactor;
}

/*-------------------------------------------------------------------------*/

const CRedactor&
CRedactor::ForLogLine( void )
{GUCEF_TRACE;

    static const CRedactor redactor = MakeLogLineRedactor();
    return redactor;
}

/*-------------------------------------------------------------------------*/

const CRedactor&
CRedactor::ForDisplay( void )
{GUCEF_TRACE;

    static const CRedactor redactor = MakeDisplayRedactor();
    return redactor;
}

/*-------------------------------------------------------------------------*/

TRedactionFlags
CRedactor::GetFlags( void ) const
{GUCEF_TRACE;

    return m_flags;
}

/*-------------------------------------------------------------------------*/

CRedactor&
CRedactor::SetFlags( TRedactionFlags flags )
{GUCEF_TRACE;

    m_flags = flags;
    return *this;
}

/*-------------------------------------------------------------------------*/

CRedactor&
CRedactor::AddFlags( TRedactionFlags flags )
{GUCEF_TRACE;

    m_flags |= flags;
    return *this;
}

/*-------------------------------------------------------------------------*/

CRedactor&
CRedactor::RemoveFlags( TRedactionFlags flags )
{GUCEF_TRACE;

    m_flags &= ~flags;
    return *this;
}

/*-------------------------------------------------------------------------*/

CRedactor&
CRedactor::SetMarker( const CString& marker )
{GUCEF_TRACE;

    UInt32 length = ByteLength( marker );
    if ( length > MaxMarkerBytes )
        length = MaxMarkerBytes;

    GUCEF::vector< char > clean( length + 1, '\0' );
    for ( UInt32 i=0; i<length; ++i )
    {
        unsigned char u = (unsigned char) marker.C_String()[ i ];
        clean[ i ] = ( u < 0x20 || 0x7F == u ) ? '?' : (char) u;
    }
    m_marker = CString( &clean[ 0 ] );
    return *this;
}

/*-------------------------------------------------------------------------*/

const CString&
CRedactor::GetMarker( void ) const
{GUCEF_TRACE;

    return m_marker;
}

/*-------------------------------------------------------------------------*/

CRedactor&
CRedactor::SetMaxLength( UInt32 maxLength )
{GUCEF_TRACE;

    m_maxLength = maxLength;
    return *this;
}

/*-------------------------------------------------------------------------*/

UInt32
CRedactor::GetMaxLength( void ) const
{GUCEF_TRACE;

    return m_maxLength;
}

/*-------------------------------------------------------------------------*/

CRedactor&
CRedactor::SetTokenMinLength( UInt32 minLength )
{GUCEF_TRACE;

    m_tokenMinLength = minLength < 8 ? 8 : minLength;
    return *this;
}

/*-------------------------------------------------------------------------*/

UInt32
CRedactor::GetTokenMinLength( void ) const
{GUCEF_TRACE;

    return m_tokenMinLength;
}

/*-------------------------------------------------------------------------*/

bool
CRedactor::AddSensitiveName( const CString& pattern )
{GUCEF_TRACE;

    CGlobPattern glob( pattern, false );
    if ( !glob.IsValid() )
        return false;
    m_sensitiveNames.push_back( glob );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRedactor::AddQueryName( const CString& pattern )
{GUCEF_TRACE;

    CGlobPattern glob( pattern, false );
    if ( !glob.IsValid() )
        return false;
    m_queryNames.push_back( glob );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRedactor::AddPathSegment( const CString& pattern )
{GUCEF_TRACE;

    CGlobPattern glob( pattern, false );
    if ( !glob.IsValid() )
        return false;
    m_pathSegments.push_back( glob );
    return true;
}

/*-------------------------------------------------------------------------*/

CRedactor&
CRedactor::ClearPatterns( void )
{GUCEF_TRACE;

    m_sensitiveNames.clear();
    m_queryNames.clear();
    m_pathSegments.clear();
    return *this;
}

/*-------------------------------------------------------------------------*/

static bool
MatchesAnyPattern( const GUCEF::vector< CGlobPattern, gucef_allocator< CGlobPattern > >& patterns, const CString& text )
{
    for ( size_t i=0; i<patterns.size(); ++i )
    {
        if ( patterns[ i ].IsMatch( text ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CRedactor::IsSensitiveName( const CString& name ) const
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() )
        return false;

    if ( 0 != ( m_flags & REDACT_NAME_WORDS ) )
    {
        static const char* const sensitiveParts[] =
        {
            "auth", "cookie", "token", "secret", "passw", "pwd", "key", "credential", "session", "signature", "bearer", "jwt"
        };
        static const char* const sensitiveNames[] =
        {
            "pw", "sig", "sid", "otp"
        };

        const char* chars = name.C_String();
        const UInt32 length = ByteLength( name );
        for ( UInt32 i=0; i<sizeof( sensitiveParts ) / sizeof( sensitiveParts[ 0 ] ); ++i )
        {
            if ( ContainsNoCase( chars, length, sensitiveParts[ i ] ) )
                return true;
        }
        for ( UInt32 i=0; i<sizeof( sensitiveNames ) / sizeof( sensitiveNames[ 0 ] ); ++i )
        {
            if ( EqualsNoCase( chars, length, sensitiveNames[ i ] ) )
                return true;
        }
    }
    return MatchesAnyPattern( m_sensitiveNames, name );
}

/*-------------------------------------------------------------------------*/

bool
CRedactor::IsSensitiveNameBytes( const char* name, UInt32 length ) const
{
    if ( 0 == length )
        return false;
    return IsSensitiveName( DecodeForComparison( name, length ) );
}

/*-------------------------------------------------------------------------*/

bool
CRedactor::IsListedQueryName( const char* name, UInt32 length ) const
{
    if ( 0 == length )
        return false;
    CString decoded = DecodeForComparison( name, length );
    return IsSensitiveName( decoded ) || MatchesAnyPattern( m_queryNames, decoded );
}

/*-------------------------------------------------------------------------*/

bool
CRedactor::IsPathSegmentToRedact( const char* segment, UInt32 length ) const
{
    if ( 0 == length )
        return false;
    return MatchesAnyPattern( m_pathSegments, DecodeForComparison( segment, length ) );
}

/*-------------------------------------------------------------------------*/

UInt32
CRedactor::ReserveSize( UInt32 inputLength ) const
{
    // what the input can give at most: every byte can be replaced by a marker, and a line takes one byte more
    UInt64 bound = (UInt64) inputLength * ( (UInt64) ByteLength( m_marker ) + 3 ) + 64;
    if ( bound > MaxResultBytes )
        bound = MaxResultBytes;
    if ( 0 != m_maxLength && m_maxLength < bound )
        bound = m_maxLength;
    return (UInt32) bound;
}

/*-------------------------------------------------------------------------*/

void
CRedactor::WritePath( COutput& out, const char* url, UInt32 length, UInt32& position ) const
{
    UInt32 i = position;
    if ( 0 != ( m_flags & REDACT_URL_PATH_SEGMENTS ) && !m_pathSegments.empty() )
    {
        while ( i < length && '?' != url[ i ] && '#' != url[ i ] && !out.IsFull() )
        {
            if ( '/' == url[ i ] )
            {
                out.Put( '/' );
                ++i;
                continue;
            }
            UInt32 segmentStart = i;
            while ( i < length && '/' != url[ i ] && '?' != url[ i ] && '#' != url[ i ] )
                ++i;
            if ( IsPathSegmentToRedact( url + segmentStart, i - segmentStart ) )
                out.PutMarker();
            else
                out.Put( url + segmentStart, i - segmentStart );
        }
    }
    else
    {
        while ( i < length && '?' != url[ i ] && '#' != url[ i ] && !out.IsFull() )
            out.Put( url[ i++ ] );
    }
    position = i;
}

/*-------------------------------------------------------------------------*/

void
CRedactor::WriteQuery( COutput& out, const char* url, UInt32 length, UInt32& position ) const
{
    UInt32 i = position;
    if ( i >= length || '?' != url[ i ] )
        return;

    const bool allValues = 0 != ( m_flags & REDACT_URL_QUERY_VALUES );
    const bool listedValues = 0 != ( m_flags & REDACT_URL_QUERY_LISTED );

    out.Put( '?' );
    ++i;
    while ( i < length && '#' != url[ i ] && !out.IsFull() )
    {
        UInt32 itemStart = i;
        UInt32 split = length;
        while ( i < length && '#' != url[ i ] && '&' != url[ i ] )
        {
            if ( '=' == url[ i ] && length == split )
                split = i;
            ++i;
        }
        const bool hasSplit = length != split;

        bool redact = allValues;
        if ( !redact && listedValues )
        {
            if ( hasSplit )
                redact = IsListedQueryName( url + itemStart, split - itemStart );
            else
                redact = IsListedQueryName( url + itemStart, i - itemStart );
        }

        if ( !redact )
        {
            out.Put( url + itemStart, i - itemStart );
        }
        else
        if ( !hasSplit )
        {
            // an item without a name/value split is itself the value
            if ( i > itemStart )
                out.PutMarker();
        }
        else
        {
            UInt32 nameLength = split - itemStart;
            if ( nameLength > MaxQueryNameBytes )
            {
                out.Put( url + itemStart, MaxQueryNameBytes );
                out.Put( "...", 3 );
            }
            else
                out.Put( url + itemStart, nameLength );
            out.Put( '=' );
            out.PutMarker();
        }
        if ( i < length && '&' == url[ i ] )
        {
            out.Put( '&' );
            ++i;
        }
    }
    position = i;
}

/*-------------------------------------------------------------------------*/

void
CRedactor::WriteUrl( COutput& out, const char* url, UInt32 length ) const
{
    UInt32 i = 0;

    // The scheme, and with it whether an authority follows: scheme://, //, or a bare authority
    bool hasAuthority = true;
    UInt32 schemeLength = 0;
    if ( length > 0 && IsAlpha( url[ 0 ] ) )
    {
        UInt32 j = 1;
        while ( j < length && ( IsAlphaNumeric( url[ j ] ) || '+' == url[ j ] || '-' == url[ j ] || '.' == url[ j ] ) )
            ++j;
        if ( j + 2 < length && ':' == url[ j ] && '/' == url[ j + 1 ] && '/' == url[ j + 2 ] )
        {
            schemeLength = j;
            out.Put( url, schemeLength );
            out.Put( "://", 3 );
            i = j + 3;
        }
    }
    if ( 0 == schemeLength )
    {
        if ( length >= 2 && '/' == url[ 0 ] && '/' == url[ 1 ] )
        {
            out.Put( "//", 2 );
            i = 2;
        }
        else
        if ( length > 0 && ( '/' == url[ 0 ] || '?' == url[ 0 ] || '#' == url[ 0 ] ) )
        {
            hasAuthority = false;
        }
    }

    // The authority: the user info goes, the host and port stay if they are what they should be
    if ( hasAuthority )
    {
        UInt32 end = i;
        while ( end < length && '/' != url[ end ] && '?' != url[ end ] && '#' != url[ end ] )
            ++end;
        UInt32 hostStart = i;
        for ( UInt32 k=end; k>i; --k )
        {
            if ( '@' == url[ k-1 ] )
            {
                if ( 0 != ( m_flags & REDACT_URL_USERINFO ) )
                {
                    out.PutMarker();
                    out.Put( '@' );
                }
                else
                    out.Put( url + i, k - i );
                hostStart = k;
                break;
            }
        }
        const bool allowNonAscii = 0 == ( m_flags & REDACT_SANITIZE_NON_ASCII );
        if ( 0 == ( m_flags & REDACT_URL_INVALID_HOST ) || IsValidHostAndPort( url + hostStart, end - hostStart, allowNonAscii ) )
        {
            out.Put( url + hostStart, end - hostStart );
            i = end;
        }
        else
        {
            // Not a host, so possibly credentials which are not in the form that was expected, in
            // which case the start of the path can be the rest of them: the path goes as well
            out.PutMarker();
            i = end;
            if ( i < length && '/' == url[ i ] )
            {
                out.Put( '/' );
                out.PutMarker();
                while ( i < length && '?' != url[ i ] && '#' != url[ i ] )
                    ++i;
            }
        }
    }

    WritePath( out, url, length, i );
    WriteQuery( out, url, length, i );

    // The fragment
    if ( i < length && '#' == url[ i ] )
    {
        out.Put( '#' );
        if ( i + 1 < length )
        {
            if ( 0 != ( m_flags & REDACT_URL_FRAGMENT ) )
                out.PutMarker();
            else
                out.Put( url + i + 1, length - ( i + 1 ) );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CRedactor::WriteHeaderValue( COutput& out, const CString& name, const char* value, UInt32 length ) const
{
    if ( 0 != ( m_flags & REDACT_HEADER_VALUES ) && IsSensitiveName( name ) )
    {
        out.PutMarker();
        return;
    }
    if ( 0 != ( m_flags & REDACT_HEADER_URLS ) && IsUrlHeaderName( name ) )
    {
        WriteUrl( out, value, length );
        return;
    }
    WriteText( out, value, length );
}

/*-------------------------------------------------------------------------*/

void
CRedactor::WriteHeaders( COutput& out, const char* text, UInt32 length ) const
{
    UInt32 lineStart = 0;
    bool firstLine = true;
    CString lastName;
    bool hasLastName = false;
    while ( lineStart < length && !out.IsFull() )
    {
        UInt32 lineEnd = lineStart;
        while ( lineEnd < length && '\n' != text[ lineEnd ] )
            ++lineEnd;
        UInt32 contentEnd = lineEnd;
        if ( contentEnd > lineStart && '\r' == text[ contentEnd - 1 ] )
            --contentEnd;
        const char* line = text + lineStart;
        const UInt32 lineLength = contentEnd - lineStart;

        if ( firstLine )
        {
            // A status line holds nothing secret, the target of a request line may
            if ( ( lineLength >= 5 && 0 == memcmp( line, "HTTP/", 5 ) ) || 0 == ( m_flags & REDACT_HEADER_URLS ) )
            {
                out.Put( line, lineLength );
            }
            else
            {
                UInt32 firstSpace = 0;
                while ( firstSpace < lineLength && ' ' != line[ firstSpace ] )
                    ++firstSpace;
                UInt32 lastSpace = lineLength;
                while ( lastSpace > firstSpace && ' ' != line[ lastSpace - 1 ] )
                    --lastSpace;
                if ( firstSpace < lineLength && lastSpace > firstSpace + 1 )
                {
                    // method, space, target, space, version
                    out.Put( line, firstSpace + 1 );
                    WriteUrl( out, line + firstSpace + 1, ( lastSpace - 1 ) - ( firstSpace + 1 ) );
                    out.Put( line + ( lastSpace - 1 ), lineLength - ( lastSpace - 1 ) );
                }
                else
                    WriteUrl( out, line, lineLength );
            }
            firstLine = false;
        }
        else
        if ( 0 == lineLength )
        {
            hasLastName = false;
        }
        else
        {
            UInt32 colon = 0;
            while ( colon < lineLength && ':' != line[ colon ] )
                ++colon;

            if ( hasLastName && ( ' ' == line[ 0 ] || '\t' == line[ 0 ] ) )
            {
                // the continuation of the value of the header before it
                UInt32 valueStart = 0;
                while ( valueStart < lineLength && ( ' ' == line[ valueStart ] || '\t' == line[ valueStart ] ) )
                {
                    out.PutRaw( line[ valueStart ] );
                    ++valueStart;
                }
                WriteHeaderValue( out, lastName, line + valueStart, lineLength - valueStart );
            }
            else
            if ( colon < lineLength )
            {
                UInt32 valueStart = colon + 1;
                while ( valueStart < lineLength && ( ' ' == line[ valueStart ] || '\t' == line[ valueStart ] ) )
                    ++valueStart;
                lastName = CString( line, colon );
                hasLastName = true;
                out.Put( line, colon + 1 );
                out.Put( ' ' );
                WriteHeaderValue( out, lastName, line + valueStart, lineLength - valueStart );
            }
            else
            {
                // not a header line, so not known to be harmless
                if ( 0 != ( m_flags & REDACT_HEADER_UNPARSEABLE_LINES ) )
                    out.PutMarker();
                else
                    out.Put( line, lineLength );
            }
        }

        // keep the line ending as it was
        if ( lineEnd < length )
        {
            for ( UInt32 k=contentEnd; k<=lineEnd; ++k )
                out.PutRaw( text[ k ] );
        }
        lineStart = lineEnd + 1;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CRedactor::WriteAuthScheme( COutput& out, const char* text, UInt32 length, UInt32 position )
{
    static const struct SScheme
    {
        const char* name;
        UInt32 nameLength;
        bool needsEncodedLook;
        bool isDigest;
    }
    schemes[] =
    {
        { "bearer", 6, false, false }, { "basic", 5, true, false }, { "negotiate", 9, true, false },
        { "ntlm", 4, true, false }, { "digest", 6, false, true }
    };

    for ( UInt32 s=0; s<sizeof( schemes ) / sizeof( schemes[ 0 ] ); ++s )
    {
        const SScheme& scheme = schemes[ s ];
        UInt32 afterName = position + scheme.nameLength;
        if ( !StartsWithNoCase( text + position, length - position, scheme.name, scheme.nameLength ) ||
             afterName >= length || ( ' ' != text[ afterName ] && '\t' != text[ afterName ] ) )
        {
            continue;
        }

        UInt32 credentialStart = afterName;
        while ( credentialStart < length && ( ' ' == text[ credentialStart ] || '\t' == text[ credentialStart ] ) )
            ++credentialStart;

        UInt32 credentialEnd = credentialStart;
        if ( scheme.isDigest )
        {
            // name=value pairs up to the end of the line
            while ( credentialEnd < length && !IsLineEnd( text[ credentialEnd ] ) )
                ++credentialEnd;
            bool hasPair = false;
            for ( UInt32 k=credentialStart; k<credentialEnd; ++k )
            {
                if ( '=' == text[ k ] )
                    hasPair = true;
            }
            if ( !hasPair )
                return position;
        }
        else
        {
            while ( credentialEnd < length && IsCredentialChar( text[ credentialEnd ] ) )
                ++credentialEnd;
            if ( credentialEnd == credentialStart )
                return position;
            if ( scheme.needsEncodedLook && !LooksEncoded( text + credentialStart, credentialEnd - credentialStart ) )
                return position;
        }

        out.Put( text + position, credentialStart - position );
        out.PutMarker();
        return credentialEnd;
    }
    return position;
}

/*-------------------------------------------------------------------------*/

UInt32
CRedactor::WriteKeyValue( COutput& out, const char* text, UInt32 length, UInt32 position ) const
{
    // the name, which can be quoted
    UInt32 i = position;
    char nameQuote = '\0';
    if ( '"' == text[ i ] || '\'' == text[ i ] )
    {
        nameQuote = text[ i ];
        ++i;
    }
    UInt32 nameStart = i;
    while ( i < length && i - nameStart <= MaxPairNameBytes && IsPairNameChar( text[ i ] ) )
        ++i;
    UInt32 nameLength = i - nameStart;
    if ( 0 == nameLength || nameLength > MaxPairNameBytes )
        return position;
    if ( '\0' != nameQuote )
    {
        if ( i >= length || nameQuote != text[ i ] )
            return position;
        ++i;
    }

    // the separator
    UInt32 spaces = 0;
    while ( i < length && spaces < 8 && ( ' ' == text[ i ] || '\t' == text[ i ] ) )
    {
        ++i;
        ++spaces;
    }
    if ( i >= length || ( ':' != text[ i ] && '=' != text[ i ] ) )
        return position;
    const char separator = text[ i ];
    ++i;

    // only a name that says its value is secret makes it a secret
    if ( !IsSensitiveNameBytes( text + nameStart, nameLength ) )
        return position;

    UInt32 valueStart = i;
    while ( valueStart < length && ( ' ' == text[ valueStart ] || '\t' == text[ valueStart ] ) )
        ++valueStart;
    out.Put( text + position, valueStart - position );
    if ( valueStart >= length || IsLineEnd( text[ valueStart ] ) )
        return valueStart;

    if ( '"' == text[ valueStart ] || '\'' == text[ valueStart ] )
    {
        const char quote = text[ valueStart ];
        UInt32 end = valueStart + 1;
        while ( end < length && quote != text[ end ] && !IsLineEnd( text[ end ] ) )
        {
            if ( '\\' == text[ end ] && end + 1 < length && !IsLineEnd( text[ end + 1 ] ) )
                ++end;
            ++end;
        }
        out.Put( quote );
        out.PutMarker();
        if ( end < length && quote == text[ end ] )
        {
            out.Put( quote );
            ++end;
        }
        return end;
    }

    UInt32 end = valueStart;
    if ( ':' == separator && '\0' == nameQuote )
    {
        // a header or a line of a listing: all of the line is the value, "Authorization: Bearer abc"
        while ( end < length && !IsLineEnd( text[ end ] ) )
            ++end;
    }
    else
    if ( ':' == separator )
    {
        while ( end < length && !IsWhiteSpace( text[ end ] ) && ',' != text[ end ] && '}' != text[ end ] && ']' != text[ end ] )
            ++end;
    }
    else
    {
        while ( end < length && !IsWhiteSpace( text[ end ] ) )
            ++end;
    }
    if ( end > valueStart )
        out.PutMarker();
    return end;
}

/*-------------------------------------------------------------------------*/

void
CRedactor::WriteText( COutput& out, const char* text, UInt32 length ) const
{
    if ( 0 == ( m_flags & REDACT_TEXT_ALL ) )
    {
        out.Put( text, length );
        return;
    }

    const bool urls = 0 != ( m_flags & REDACT_TEXT_URLS );
    const bool auth = 0 != ( m_flags & REDACT_TEXT_AUTH_SCHEMES );
    const bool pairs = 0 != ( m_flags & REDACT_TEXT_KEY_VALUES );
    const bool emails = 0 != ( m_flags & REDACT_TEXT_EMAILS );
    const bool ips = 0 != ( m_flags & REDACT_TEXT_IPS );
    const bool cards = 0 != ( m_flags & REDACT_TEXT_CARD_NUMBERS );
    const bool tokens = 0 != ( m_flags & REDACT_TEXT_TOKEN_RUNS );

    UInt32 i = 0;
    while ( i < length && !out.IsFull() )
    {
        const char c = text[ i ];
        const char previous = i > 0 ? text[ i - 1 ] : ' ';
        UInt32 end = i;

        if ( !IsWordChar( previous ) )
        {
            if ( urls && IsAlpha( c ) )
            {
                end = MatchUrl( text, length, i );
                if ( end > i )
                {
                    WriteUrl( out, text + i, end - i );
                    i = end;
                    continue;
                }
            }
            if ( auth && IsAlpha( c ) )
            {
                end = WriteAuthScheme( out, text, length, i );
                if ( end > i )
                {
                    i = end;
                    continue;
                }
            }
            if ( pairs && ( IsWordChar( c ) || '"' == c || '\'' == c ) )
            {
                end = WriteKeyValue( out, text, length, i );
                if ( end > i )
                {
                    i = end;
                    continue;
                }
            }
        }

        if ( emails && IsEmailLocalChar( c ) && ( 0 == i || !IsEmailLocalChar( previous ) ) )
        {
            end = MatchEmail( text, length, i );
            if ( end > i )
            {
                out.PutMarker();
                i = end;
                continue;
            }
        }

        if ( ips )
        {
            if ( IsDigit( c ) && !IsAlphaNumeric( previous ) && '.' != previous )
            {
                end = MatchIPv4( text, length, i );
                if ( end > i )
                {
                    out.PutMarker();
                    i = end;
                    continue;
                }
            }
            if ( ( IsHexDigit( c ) || ':' == c ) && !IsAlphaNumeric( previous ) && ':' != previous && '.' != previous )
            {
                end = MatchIPv6( text, length, i );
                if ( end > i )
                {
                    out.PutMarker();
                    i = end;
                    continue;
                }
            }
        }

        if ( cards && IsDigit( c ) && !IsAlphaNumeric( previous ) && '-' != previous )
        {
            end = MatchCardNumber( text, length, i );
            if ( end > i )
            {
                out.PutMarker();
                i = end;
                continue;
            }
        }

        if ( tokens && IsTokenChar( c ) && ( 0 == i || !IsTokenChar( previous ) ) )
        {
            end = i;
            while ( end < length && IsTokenChar( text[ end ] ) )
                ++end;
            if ( end - i >= m_tokenMinLength && IsTokenLike( text + i, end - i ) )
            {
                // the padding of base64 is part of it
                for ( UInt32 padding=0; padding<2 && end < length && '=' == text[ end ]; ++padding )
                    ++end;
                out.PutMarker();
                i = end;
                continue;
            }
        }

        out.Put( c );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

CString
CRedactor::RedactUrlBytes( const char* url, UInt32 length ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == url || 0 == length )
        return CString();

    COutput out( ReserveSize( length ), m_marker, 0 != ( m_flags & REDACT_SANITIZE_CONTROL ), 0 != ( m_flags & REDACT_SANITIZE_NON_ASCII ) );
    WriteUrl( out, url, length );
    return out.Finish();
}

/*-------------------------------------------------------------------------*/

CString
CRedactor::RedactUrl( const CString& url ) const
{GUCEF_TRACE;

    if ( url.IsNULLOrEmpty() )
        return CString();

    return RedactUrlBytes( url.C_String(), ByteLength( url ) );
}

/*-------------------------------------------------------------------------*/

CString
CRedactor::RedactHeaderValue( const CString& name  ,
                              const CString& value ) const
{GUCEF_TRACE;

    const UInt32 length = ByteLength( value );
    if ( 0 == length && !( 0 != ( m_flags & REDACT_HEADER_VALUES ) && IsSensitiveName( name ) ) )
        return CString();

    COutput out( ReserveSize( length ), m_marker, 0 != ( m_flags & REDACT_SANITIZE_CONTROL ), 0 != ( m_flags & REDACT_SANITIZE_NON_ASCII ) );
    WriteHeaderValue( out, name, value.C_String(), length );
    return out.Finish();
}

/*-------------------------------------------------------------------------*/

CString
CRedactor::RedactHeaders( const CString& block ) const
{GUCEF_TRACE;

    if ( block.IsNULLOrEmpty() )
        return CString();

    const UInt32 length = ByteLength( block );
    COutput out( ReserveSize( length ), m_marker, 0 != ( m_flags & REDACT_SANITIZE_CONTROL ), 0 != ( m_flags & REDACT_SANITIZE_NON_ASCII ) );
    WriteHeaders( out, block.C_String(), length );
    return out.Finish();
}

/*-------------------------------------------------------------------------*/

CString
CRedactor::RedactText( const CString& text ) const
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() )
        return CString();

    const UInt32 length = ByteLength( text );
    COutput out( ReserveSize( length ), m_marker, 0 != ( m_flags & REDACT_SANITIZE_CONTROL ), 0 != ( m_flags & REDACT_SANITIZE_NON_ASCII ) );
    WriteText( out, text.C_String(), length );
    return out.Finish();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
