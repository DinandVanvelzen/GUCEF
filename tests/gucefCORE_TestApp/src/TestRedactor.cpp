/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#include <string.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestRedactor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** compares and logs the actual text when it differs, so a failure tells what was produced */
static bool
SameText( const CORE::CString& actual, const CORE::CString& expected )
{
    if ( actual == expected )
        return true;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Redactor test: expected \"" << expected << "\" but got \"" << actual << "\"" );
    return false;
}

/*-------------------------------------------------------------------------*/

/** what the URL preset makes of the URL */
static bool
RedactsTo( const CORE::CString& url, const CORE::CString& expected )
{
    return SameText( CRedactor::ForUrl().RedactUrl( url ), expected );
}

/*-------------------------------------------------------------------------*/

static bool
UrlRedactsTo( const CRedactor& redactor, const CORE::CString& url, const CORE::CString& expected )
{
    return SameText( redactor.RedactUrl( url ), expected );
}

/*-------------------------------------------------------------------------*/

static bool
TextRedactsTo( const CRedactor& redactor, const CORE::CString& text, const CORE::CString& expected )
{
    return SameText( redactor.RedactText( text ), expected );
}

/*-------------------------------------------------------------------------*/

static bool
HeadersRedactTo( const CRedactor& redactor, const CORE::CString& headers, const CORE::CString& expected )
{
    return SameText( redactor.RedactHeaders( headers ), expected );
}

/*-------------------------------------------------------------------------*/

static bool
Contains( const CORE::CString& text, const CORE::CString& part )
{
    return text.HasSubstr( part, true ) >= 0;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Repeat( char c, UInt32 count )
{
    // doubling, so that megabytes are not built one byte at a time
    CORE::CString result;
    CORE::CString piece( c );
    while ( count > 0 )
    {
        if ( 0 != ( count & 1 ) )
            result += piece;
        count >>= 1;
        if ( count > 0 )
            piece += piece;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
RepeatText( const CORE::CString& piece, UInt32 count )
{
    CORE::CString result;
    CORE::CString part = piece;
    while ( count > 0 )
    {
        if ( 0 != ( count & 1 ) )
            result += part;
        count >>= 1;
        if ( count > 0 )
            part += part;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

/** whether the text is one line of printable ASCII */
static bool
IsPlainLine( const CORE::CString& text )
{
    const char* chars = text.C_String();
    UInt32 length = text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
    for ( UInt32 i=0; i<length; ++i )
    {
        if ( chars[ i ] < 0x20 || chars[ i ] >= 0x7F )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** whether the text has no control characters */
static bool
HasNoControlCharacters( const CORE::CString& text )
{
    const char* chars = text.C_String();
    UInt32 length = text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
    for ( UInt32 i=0; i<length; ++i )
    {
        if ( (unsigned char) chars[ i ] < 0x20 || 0x7F == (unsigned char) chars[ i ] )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** the number of bytes of the text, which is not the length of text with characters of more than one byte */
static UInt32
Bytes( const CORE::CString& text )
{
    return text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
}

/*-------------------------------------------------------------------------*/

/** a redactor with the flags and the lists of the URL preset only, for tests of one flag at a time */
static CRedactor
Only( UInt32 flags )
{
    return CRedactor( flags );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS: THE PRESETS                                                 //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformPresetUrlTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A URL keeps scheme, host, port and path and loses the values of its query" )
        try
        {
            ASSERT_TRUE( RedactsTo( "http://example.com/a/b", "http://example.com/a/b" ) );
            ASSERT_TRUE( RedactsTo( "https://example.com", "https://example.com" ) );
            ASSERT_TRUE( RedactsTo( "https://example.com:8443/a?x=1", "https://example.com:8443/a?x=***" ) );
            ASSERT_TRUE( RedactsTo( "https://api.steampowered.com/ISteamUserAuth/AuthenticateUserTicket/v1/?key=ABCDEF0123&appid=480&ticket=00AA",
                                    "https://api.steampowered.com/ISteamUserAuth/AuthenticateUserTicket/v1/?key=***&appid=***&ticket=***" ) );
            ASSERT_TRUE( RedactsTo( "https://api.example.com/v1/me?access_token=AT-123", "https://api.example.com/v1/me?access_token=***" ) );
            ASSERT_TRUE( RedactsTo( "https://idp.example.com/cb?code=C&state=S&client_secret=X", "https://idp.example.com/cb?code=***&state=***&client_secret=***" ) );
            ASSERT_TRUE( RedactsTo( "http://[::1]:8080/x?k=v", "http://[::1]:8080/x?k=***" ) );
            ASSERT_TRUE( RedactsTo( "http://127.0.0.1:47699/", "http://127.0.0.1:47699/" ) );
            ASSERT_TRUE( RedactsTo( "HTTPS://Example.COM/Path", "HTTPS://Example.COM/Path" ) );
            ASSERT_TRUE( RedactsTo( "ws://host:81/socket?token=T", "ws://host:81/socket?token=***" ) );
            ASSERT_TRUE( RedactsTo( "", "" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A URL loses its user info, also when the password holds an @ or a :" )
        try
        {
            ASSERT_TRUE( RedactsTo( "http://user:pw@host:81/p?q=1", "http://***@host:81/p?q=***" ) );
            ASSERT_TRUE( RedactsTo( "http://token@host/", "http://***@host/" ) );
            ASSERT_TRUE( RedactsTo( "http://u:p@ss@host/x", "http://***@host/x" ) );
            ASSERT_TRUE( RedactsTo( "http://u:p:w@host/x", "http://***@host/x" ) );
            ASSERT_TRUE( RedactsTo( "http://us%40er:pw@h/", "http://***@h/" ) );
            ASSERT_TRUE( RedactsTo( "http://:pw@host/", "http://***@host/" ) );
            ASSERT_TRUE( RedactsTo( "http://user:pw@[::1]:80/", "http://***@[::1]:80/" ) );

            // an @ after the authority belongs to the path or the query
            ASSERT_TRUE( RedactsTo( "http://host/@alice/x", "http://host/@alice/x" ) );
            ASSERT_TRUE( RedactsTo( "http://host/p?email=a@b.c", "http://host/p?email=***" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A URL loses its fragment" )
        try
        {
            ASSERT_TRUE( RedactsTo( "http://host/p#access_token=abc", "http://host/p#***" ) );
            ASSERT_TRUE( RedactsTo( "http://host/cb?code=1&state=2#token=3", "http://host/cb?code=***&state=***#***" ) );
            ASSERT_TRUE( RedactsTo( "http://host/#", "http://host/#" ) );
            ASSERT_TRUE( RedactsTo( "http://host/p#frag?key=secret", "http://host/p#***" ) );
            ASSERT_TRUE( RedactsTo( "http://host/p?a=1#x#y", "http://host/p?a=***#***" ) );
            ASSERT_TRUE( RedactsTo( "http://host#frag", "http://host#***" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Repeated, empty and nameless query items lose their values" )
        try
        {
            ASSERT_TRUE( RedactsTo( "http://h/?a=1&a=2&b=&a=3", "http://h/?a=***&a=***&b=***&a=***" ) );
            ASSERT_TRUE( RedactsTo( "http://h/?a=1&&b=2", "http://h/?a=***&&b=***" ) );
            ASSERT_TRUE( RedactsTo( "http://h/?a=1&", "http://h/?a=***&" ) );
            ASSERT_TRUE( RedactsTo( "http://h/?", "http://h/?" ) );
            ASSERT_TRUE( RedactsTo( "http://h/?a=b=c", "http://h/?a=***" ) );
            ASSERT_TRUE( RedactsTo( "http://h/?=value", "http://h/?=***" ) );
            ASSERT_TRUE( RedactsTo( "http://h/?a=1;b=2", "http://h/?a=***" ) );

            // a query item without a name/value split is a value, a secret can be written that way
            ASSERT_TRUE( RedactsTo( "http://h/?secrettoken", "http://h/?***" ) );
            ASSERT_TRUE( RedactsTo( "http://h/?flag&x=1", "http://h/?***&x=***" ) );

            // URLs without a scheme or without a host
            ASSERT_TRUE( RedactsTo( "/path?key=abc#f", "/path?key=***#***" ) );
            ASSERT_TRUE( RedactsTo( "?key=abc", "?key=***" ) );
            ASSERT_TRUE( RedactsTo( "//host/p?k=v", "//host/p?k=***" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Encoded characters are not decoded, so they cannot hide or reveal anything" )
        try
        {
            ASSERT_TRUE( RedactsTo( "http://h/a%20b?na%20me=%73ecret&x%3Dy=1", "http://h/a%20b?na%20me=***&x%3Dy=***" ) );
            ASSERT_TRUE( RedactsTo( "http://h/p?a=1%26b=2", "http://h/p?a=***" ) );
            ASSERT_TRUE( RedactsTo( "http://h/p?a=%23frag", "http://h/p?a=***" ) );
            ASSERT_TRUE( RedactsTo( "http://h/p%3Fkey=1", "http://h/p%3Fkey=1" ) );
            ASSERT_TRUE( RedactsTo( "http://h%2Eexample/p?k=%00", "http://h%2Eexample/p?k=***" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A malformed URL never lets a secret through" )
        try
        {
            ASSERT_TRUE( RedactsTo( "http://", "http://" ) );
            ASSERT_TRUE( RedactsTo( "://", "://" ) );
            ASSERT_TRUE( RedactsTo( "http:///path?k=v", "http:///path?k=***" ) );
            ASSERT_TRUE( RedactsTo( "not a url", "***" ) );
            ASSERT_TRUE( RedactsTo( "key=secret&x=1", "***" ) );
            ASSERT_TRUE( RedactsTo( "user:pw@host/p?k=v", "***@host/p?k=***" ) );

            // not a host:port, so it may be credentials, as is the path that follows it
            ASSERT_TRUE( RedactsTo( "http://host:notaport/p?k=v", "http://***/***?k=***" ) );
            ASSERT_TRUE( RedactsTo( "http://user:pa/ss@host/x?k=v", "http://***/***?k=***" ) );
            ASSERT_TRUE( RedactsTo( "http://host:123456/p", "http://***/***" ) );
            ASSERT_TRUE( RedactsTo( "http://[::1/p", "http://***/***" ) );
            ASSERT_TRUE( RedactsTo( "http://[::1]x/p", "http://***/***" ) );
            ASSERT_TRUE( RedactsTo( "http://ho st/", "http://***/***" ) );
            ASSERT_TRUE( RedactsTo( "http://a:b:c/", "http://***/***" ) );

            // no host, only a scheme
            ASSERT_TRUE( RedactsTo( "mailto:someone@example.com", "***@example.com" ) );
            ASSERT_TRUE( RedactsTo( "http:/host/p?k=v", "http:/host/p?k=***" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The result is one line of printable characters whatever the input holds" )
        try
        {
            CORE::CString injected = CORE::CRedactor::ForUrl().RedactUrl( "http://h/p\r\nERROR fake log line?k=SECRETVALUE" );
            ASSERT_TRUE( IsPlainLine( injected ) );
            ASSERT_TRUE( Contains( injected, "k=***" ) && !Contains( injected, "SECRETVALUE" ) );

            CORE::CString tabbed = CORE::CRedactor::ForUrl().RedactUrl( "http://h\t/p?k=v" );
            ASSERT_TRUE( IsPlainLine( tabbed ) && !Contains( tabbed, "\t" ) );

            // UTF-8 and bytes which are not
            CORE::CString nonAscii = CORE::CRedactor::ForUrl().RedactUrl( "http://h/\xC3\xBC\xE2\x82\xAC?k=SECRETVALUE" );
            ASSERT_TRUE( IsPlainLine( nonAscii ) && !Contains( nonAscii, "SECRETVALUE" ) && Contains( nonAscii, "k=***" ) );
            CORE::CString nonAsciiHost = CORE::CRedactor::ForUrl().RedactUrl( "http://h\xC3\xBC.example/p" );
            ASSERT_TRUE( SameText( nonAsciiHost, "http://***/***" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A very long URL gives a result of limited size without the secrets that come after the limit" )
        try
        {
            // exactly at the limit nothing is cut, one more and the end is marked
            CORE::CString atLimit = "http://h/" + Repeat( 'a', 1015 );
            ASSERT_TRUE( 1024 == atLimit.Length() );
            ASSERT_TRUE( SameText( CORE::CRedactor::ForUrl().RedactUrl( atLimit ), atLimit ) );
            CORE::CString overLimit = "http://h/" + Repeat( 'a', 1016 );
            CORE::CString cut = CORE::CRedactor::ForUrl().RedactUrl( overLimit );
            ASSERT_TRUE( 1024 == cut.Length() && cut.EndsWith( "..." ) && cut.StartsWith( "http://h/aaaa" ) );

            // a long path with a query after it
            CORE::CString longPath = CORE::CRedactor::ForUrl().RedactUrl( "http://h/" + Repeat( 'p', 100000 ) + "?key=SECRETVALUE#SECRETFRAGMENT" );
            ASSERT_TRUE( longPath.Length() <= 1024 && longPath.EndsWith( "..." ) );
            ASSERT_TRUE( !Contains( longPath, "SECRETVALUE" ) && !Contains( longPath, "SECRETFRAGMENT" ) );

            // a long host
            CORE::CString longHost = CORE::CRedactor::ForUrl().RedactUrl( "http://" + Repeat( 'h', 100000 ) + "/p?key=SECRETVALUE" );
            ASSERT_TRUE( longHost.Length() <= 1024 && !Contains( longHost, "SECRETVALUE" ) );

            // a long user info and a long query
            ASSERT_TRUE( SameText( CORE::CRedactor::ForUrl().RedactUrl( "http://" + Repeat( 'u', 100000 ) + "@host/p" ), "http://***@host/p" ) );
            CORE::CString query = "http://h/p?";
            for ( UInt32 i=0; i<20000; ++i )
                query += "k" + CORE::ToString( i ) + "=SECRETVALUE" + CORE::ToString( i ) + "&";
            CORE::CString longQuery = CORE::CRedactor::ForUrl().RedactUrl( query );
            ASSERT_TRUE( longQuery.Length() <= 1024 && longQuery.EndsWith( "..." ) && !Contains( longQuery, "SECRETVALUE" ) );
            ASSERT_TRUE( longQuery.StartsWith( "http://h/p?k0=***&k1=***&k2=***&" ) );

            // a long name is shortened, its value is still gone
            CORE::CString longName = CORE::CRedactor::ForUrl().RedactUrl( "http://h/p?" + Repeat( 'n', 5000 ) + "=SECRETVALUE" );
            ASSERT_TRUE( longName.Length() < 100 && longName.EndsWith( "...=***" ) && !Contains( longName, "SECRETVALUE" ) );

            // megabytes
            CORE::CString huge = CORE::CRedactor::ForUrl().RedactUrl( "http://h/p?key=" + Repeat( 'x', 4 * 1024 * 1024 ) );
            ASSERT_TRUE( SameText( huge, "http://h/p?key=***" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}


/*-------------------------------------------------------------------------*/

static void
PerformUrlFlagTests( void )
{GUCEF_TRACE;

    const char* const full = "http://user:pw@h/a/reset-9f3a/b?access_token=T&page=2&code=C#frag";

    GUCEF_TESTFW_TESTCASE( "Test: A redactor without flags redacts nothing, also of malformed input" )
        try
        {
            CRedactor none;
            ASSERT_TRUE( none.GetFlags() == REDACT_NONE );
            ASSERT_TRUE( UrlRedactsTo( none, full, full ) );
            ASSERT_TRUE( UrlRedactsTo( none, "not a url", "not a url" ) );
            ASSERT_TRUE( UrlRedactsTo( none, "http://host:notaport/p?k=v", "http://host:notaport/p?k=v" ) );
            ASSERT_TRUE( UrlRedactsTo( none, "http://u:p@host/p#f", "http://u:p@host/p#f" ) );
            ASSERT_TRUE( UrlRedactsTo( none, "", "" ) );
            ASSERT_TRUE( SameText( none.RedactText( "password=x Bearer abc" ), "password=x Bearer abc" ) );
            ASSERT_TRUE( SameText( none.RedactHeaders( "GET /a?k=v HTTP/1.1\r\nCookie: x\r\n" ), "GET /a?k=v HTTP/1.1\r\nCookie: x\r\n" ) );
            ASSERT_TRUE( !none.IsSensitiveName( "Authorization" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: REDACT_URL_USERINFO replaces the user info and nothing else" )
        try
        {
            CRedactor redactor = Only( REDACT_URL_USERINFO );
            ASSERT_TRUE( UrlRedactsTo( redactor, full, "http://***@h/a/reset-9f3a/b?access_token=T&page=2&code=C#frag" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://u:p@ss@host/x", "http://***@host/x" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://host/@alice/x", "http://host/@alice/x" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://host/x", "http://host/x" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: REDACT_URL_QUERY_VALUES replaces all values of the query and keeps the names" )
        try
        {
            CRedactor redactor = Only( REDACT_URL_QUERY_VALUES );
            ASSERT_TRUE( UrlRedactsTo( redactor, full, "http://user:pw@h/a/reset-9f3a/b?access_token=***&page=***&code=***#frag" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://h/?flag&x=1", "http://h/?***&x=***" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://h/", "http://h/" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: REDACT_URL_QUERY_LISTED replaces the values of the names that are sensitive or listed" )
        try
        {
            // by the rule for names
            CRedactor words = Only( REDACT_URL_QUERY_LISTED | REDACT_NAME_WORDS );
            ASSERT_TRUE( UrlRedactsTo( words, full, "http://user:pw@h/a/reset-9f3a/b?access_token=***&page=2&code=C#frag" ) );
            ASSERT_TRUE( UrlRedactsTo( words, "http://h/p?ApiKey=1&PASSWORD=2&name=3&sid=4&x=5", "http://h/p?ApiKey=***&PASSWORD=***&name=3&sid=***&x=5" ) );

            // a name that is written with escapes is the name it stands for
            ASSERT_TRUE( UrlRedactsTo( words, "http://h/p?ap%69key=1&ke%79=2&pa%67e=3", "http://h/p?ap%69key=***&ke%79=***&pa%67e=3" ) );

            // by the list for the names of the query
            CRedactor listed = Only( REDACT_URL_QUERY_LISTED );
            ASSERT_TRUE( listed.AddQueryName( "code" ) );
            ASSERT_TRUE( listed.AddQueryName( "st*" ) );
            ASSERT_TRUE( UrlRedactsTo( listed, "http://h/cb?code=1&state=2&page=3&Code=4&access_token=5", "http://h/cb?code=***&state=***&page=3&Code=***&access_token=5" ) );
            ASSERT_TRUE( UrlRedactsTo( listed, "http://h/cb?c%6Fde=1&stuff=2", "http://h/cb?c%6Fde=***&stuff=***" ) );

            // an item without a name/value split is its own name
            ASSERT_TRUE( UrlRedactsTo( listed, "http://h/cb?code&x&state", "http://h/cb?***&x&***" ) );

            // the list of the sensitive names is used as well
            CRedactor both = Only( REDACT_URL_QUERY_LISTED );
            ASSERT_TRUE( both.AddSensitiveName( "*pin*" ) );
            ASSERT_TRUE( UrlRedactsTo( both, "http://h/cb?pincode=1&spinner=2&page=3", "http://h/cb?pincode=***&spinner=***&page=3" ) );

            // without a name that counts nothing is replaced
            CRedactor nothing = Only( REDACT_URL_QUERY_LISTED );
            ASSERT_TRUE( UrlRedactsTo( nothing, full, full ) );

            // a name of the query is not a header name
            ASSERT_TRUE( !listed.IsSensitiveName( "code" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: REDACT_URL_FRAGMENT replaces the fragment" )
        try
        {
            CRedactor redactor = Only( REDACT_URL_FRAGMENT );
            ASSERT_TRUE( UrlRedactsTo( redactor, full, "http://user:pw@h/a/reset-9f3a/b?access_token=T&page=2&code=C#***" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://h/p#a#b", "http://h/p#***" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://h/p#", "http://h/p#" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://h/p?k=v", "http://h/p?k=v" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: REDACT_URL_PATH_SEGMENTS replaces the segments of the path that match a pattern" )
        try
        {
            CRedactor redactor = Only( REDACT_URL_PATH_SEGMENTS );
            ASSERT_TRUE( redactor.AddPathSegment( "reset-*" ) );
            ASSERT_TRUE( redactor.AddPathSegment( "????????-????-????-????-????????????" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, full, "http://user:pw@h/a/***/b?access_token=T&page=2&code=C#frag" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://h/users/RESET-1/confirm", "http://h/users/***/confirm" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://h/users/%72eset-1/x", "http://h/users/***/x" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://h/u/123e4567-e89b-12d3-a456-426614174000/x?a=1", "http://h/u/***/x?a=1" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://h/u/prereset-1/", "http://h/u/prereset-1/" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "/reset-1", "/***" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://h", "http://h" ) );

            // a pattern that is not valid is not added
            ASSERT_FALSE( redactor.AddPathSegment( "a[" ) );

            // without the flag the list does nothing
            CRedactor unflagged = Only( REDACT_NONE );
            ASSERT_TRUE( unflagged.AddPathSegment( "reset-*" ) );
            ASSERT_TRUE( UrlRedactsTo( unflagged, "http://h/reset-1", "http://h/reset-1" ) );

            // and without a list the flag does nothing
            CRedactor unlisted = Only( REDACT_URL_PATH_SEGMENTS );
            ASSERT_TRUE( UrlRedactsTo( unlisted, "http://h/reset-1", "http://h/reset-1" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: REDACT_URL_INVALID_HOST replaces a host that is not a host and the path that follows it" )
        try
        {
            CRedactor redactor = Only( REDACT_URL_INVALID_HOST );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://host:notaport/p?k=v", "http://***/***?k=v" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://user:pa/ss@host/x?k=v", "http://***/***?k=v" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://u:p@host:81/x?k=v", "http://u:p@host:81/x?k=v" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://[::1]:80/x", "http://[::1]:80/x" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://ho st/p", "http://***/***" ) );

            // a name with characters that are not ASCII is fine when the output may hold them
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://h\xC3\xBC.example/p", "http://h\xC3\xBC.example/p" ) );
            CRedactor ascii = Only( REDACT_URL_INVALID_HOST | REDACT_SANITIZE_NON_ASCII );
            ASSERT_TRUE( UrlRedactsTo( ascii, "http://h\xC3\xBC.example/p", "http://***/***" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The flags of the URL are combined as they are set, and set, added and removed as asked" )
        try
        {
            CRedactor redactor;
            redactor.AddFlags( REDACT_URL_USERINFO );
            redactor.AddFlags( REDACT_URL_FRAGMENT );
            ASSERT_TRUE( redactor.GetFlags() == ( REDACT_URL_USERINFO | REDACT_URL_FRAGMENT ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, full, "http://***@h/a/reset-9f3a/b?access_token=T&page=2&code=C#***" ) );
            redactor.RemoveFlags( REDACT_URL_USERINFO ).AddFlags( REDACT_URL_QUERY_VALUES );
            ASSERT_TRUE( UrlRedactsTo( redactor, full, "http://user:pw@h/a/reset-9f3a/b?access_token=***&page=***&code=***#***" ) );
            redactor.SetFlags( REDACT_URL_ALL );
            ASSERT_TRUE( redactor.GetFlags() == REDACT_URL_ALL );
            ASSERT_TRUE( UrlRedactsTo( redactor, full, "http://***@h/a/reset-9f3a/b?access_token=***&page=***&code=***#***" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A part of a text is redacted as a URL of its own" )
        try
        {
            ASSERT_TRUE( SameText( CRedactor::ForUrl().RedactUrlBytes( "http://h/p?k=vSECRETTAIL", 14 ), "http://h/p?k=***" ) );
            ASSERT_TRUE( SameText( CRedactor::ForUrl().RedactUrlBytes( "http://user:pw@h/p", 18 ), "http://***@h/p" ) );
            ASSERT_TRUE( SameText( CRedactor::ForUrl().RedactUrlBytes( GUCEF_NULL, 0 ), "" ) );
            ASSERT_TRUE( SameText( CRedactor::ForUrl().RedactUrlBytes( GUCEF_NULL, 5 ), "" ) );
            ASSERT_TRUE( SameText( CRedactor::ForUrl().RedactUrlBytes( "x", 0 ), "" ) );

            // a NUL in the bytes does not end them, and does not end up in the result
            ASSERT_TRUE( SameText( CRedactor( REDACT_NONE ).RedactUrlBytes( "a\0b", 3 ), "a?b" ) );
            ASSERT_TRUE( SameText( CRedactor::ForUrl().RedactUrlBytes( "http://h/a\0b?k=v", 16 ), "http://h/a?b?k=***" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformMarkerLengthAndSanitationTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The marker is *** unless set otherwise, and is used for everything that is replaced" )
        try
        {
            ASSERT_TRUE( CRedactor().GetMarker() == "***" );
            ASSERT_TRUE( CRedactor::ForUrl().GetMarker() == GUCEF_CORE_REDACTION_MARKER );

            CRedactor redactor( CRedactor::ForHeaders() );
            redactor.SetMarker( "[hidden]" );
            ASSERT_TRUE( redactor.GetMarker() == "[hidden]" );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://u:p@h/p?k=v#f", "http://[hidden]@h/p?k=[hidden]#[hidden]" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://u:pa/ss@host/x?k=v", "http://[hidden]/[hidden]?k=[hidden]" ) );
            ASSERT_TRUE( SameText( redactor.RedactHeaderValue( "Authorization", "Bearer abc" ), "[hidden]" ) );
            ASSERT_TRUE( HeadersRedactTo( redactor, "GET /a?k=v HTTP/1.1\r\nCookie: x\r\nnot a header\r\n", "GET /a?k=[hidden] HTTP/1.1\r\nCookie: [hidden]\r\n[hidden]\r\n" ) );
            ASSERT_TRUE( SameText( redactor.RedactUrl( "http://h/p?a=1&b" ), "http://h/p?a=[hidden]&[hidden]" ) );

            // a redactor that is a copy of a preset does not change the preset
            ASSERT_TRUE( CRedactor::ForHeaders().GetMarker() == "***" );
            ASSERT_TRUE( SameText( CRedactor::ForHeaders().RedactHeaderValue( "Authorization", "x" ), "***" ) );

            // another marker is no problem for what is redacted twice
            ASSERT_TRUE( SameText( redactor.RedactUrl( redactor.RedactUrl( "http://u:p@h/p?k=v#f" ) ), "http://[hidden]@h/p?k=[hidden]#[hidden]" ) );

            // no marker takes the redacted part away
            CRedactor empty( CRedactor::ForUrl() );
            empty.SetMarker( "" );
            ASSERT_TRUE( UrlRedactsTo( empty, "http://u:p@h/p?k=v#f", "http://@h/p?k=#" ) );

            // a marker of text that does not fit in a line is made to fit: no control characters and at most 64 bytes
            CRedactor odd( CRedactor::ForUrl() );
            odd.SetMarker( "a\r\nb" );
            ASSERT_TRUE( odd.GetMarker() == "a??b" );
            odd.SetMarker( Repeat( 'm', 100 ) );
            ASSERT_TRUE( odd.GetMarker().Length() == 64 );
            ASSERT_TRUE( IsPlainLine( odd.RedactUrl( "http://u:p@h/p?k=v#f" ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The maximum length limits the result and marks what is cut" )
        try
        {
            CRedactor redactor( REDACT_URL_USERINFO | REDACT_URL_QUERY_VALUES );
            ASSERT_TRUE( redactor.GetMaxLength() == 0 );
            const CORE::CString url = "http://u:p@host.example/path/to/something?alpha=1&beta=2";

            // no limit
            ASSERT_TRUE( UrlRedactsTo( redactor, url, "http://***@host.example/path/to/something?alpha=***&beta=***" ) );

            // exactly at the limit nothing is cut, one over and the last three bytes are dots
            CORE::CString whole = redactor.RedactUrl( url );
            redactor.SetMaxLength( whole.Length() );
            ASSERT_TRUE( SameText( redactor.RedactUrl( url ), whole ) );
            redactor.SetMaxLength( whole.Length() - 1 );
            CORE::CString cut = redactor.RedactUrl( url );
            ASSERT_TRUE( cut.Length() == whole.Length() - 1 && cut.EndsWith( "..." ) && cut.StartsWith( "http://***@host.example/path/to/something?alpha=***&b" ) );

            // very short limits
            redactor.SetMaxLength( 4 );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://h/abcdef", "h..." ) );
            redactor.SetMaxLength( 3 );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://h/abcdef", "htt" ) );
            redactor.SetMaxLength( 1 );
            ASSERT_TRUE( UrlRedactsTo( redactor, "http://h/abcdef", "h" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "", "" ) );
            ASSERT_TRUE( redactor.GetMaxLength() == 1 );

            // the largest limit is no limit that costs anything
            redactor.SetMaxLength( 0xFFFFFFFF );
            ASSERT_TRUE( UrlRedactsTo( redactor, url, whole ) );

            // all the functions and a marker that is longer than what is left
            CRedactor limited( CRedactor::ForHeaders() );
            limited.SetMaxLength( 40 );
            CORE::CString headers = limited.RedactHeaders( "GET /a/long/path/that/goes/on?k=v HTTP/1.1\r\nCookie: x\r\nHost: example.com\r\n" );
            ASSERT_TRUE( headers.Length() == 40 && headers.EndsWith( "..." ) );
            ASSERT_TRUE( headers.StartsWith( "GET /a/long/path/that/goes/on?k=***" ) );
            CRedactor text( REDACT_TEXT_KEY_VALUES | REDACT_NAME_WORDS );
            text.SetMaxLength( 12 );
            ASSERT_TRUE( TextRedactsTo( text, "password=hunter2 and more words", "password=..." ) );
            text.SetMarker( "[very long marker]" );
            ASSERT_TRUE( TextRedactsTo( text, "pw=hunter2", "pw=[very ..." ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A cut does not go through a character of UTF-8 when the output may hold them" )
        try
        {
            CRedactor redactor( REDACT_SANITIZE_CONTROL );
            redactor.SetMaxLength( 10 );
            const CORE::CString text = "ab" + RepeatText( "\xC3\xBC", 10 );
            ASSERT_TRUE( TextRedactsTo( redactor, text, "ab\xC3\xBC\xC3\xBC..." ) );
            redactor.SetMaxLength( 9 );
            ASSERT_TRUE( TextRedactsTo( redactor, text, "ab\xC3\xBC\xC3\xBC..." ) );
            redactor.SetMaxLength( 8 );
            ASSERT_TRUE( TextRedactsTo( redactor, text, "ab\xC3\xBC..." ) );

            // text which holds what is not UTF-8 is cut at the limit without trouble
            ASSERT_TRUE( TextRedactsTo( redactor, "\xBC\xBC\xBC\xBC\xBC\xBC\xBC\xBC\xBC\xBC", "..." ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Control characters and bytes that are not ASCII are replaced as asked, independently of each other" )
        try
        {
            const CORE::CString text = "a\tb\r\nc\x01\x7F" "d\xC3\xBC" "e";

            ASSERT_TRUE( TextRedactsTo( CRedactor( REDACT_NONE ), text, text ) );
            ASSERT_TRUE( TextRedactsTo( CRedactor( REDACT_SANITIZE_CONTROL ), text, "a?b??c??d\xC3\xBC" "e" ) );
            ASSERT_TRUE( TextRedactsTo( CRedactor( REDACT_SANITIZE_NON_ASCII ), text, "a\tb\r\nc\x01\x7F" "d??e" ) );
            ASSERT_TRUE( TextRedactsTo( CRedactor( REDACT_SANITIZE_ALL ), text, "a?b??c??d??e" ) );

            // the same goes for what a header and a URL keep
            ASSERT_TRUE( UrlRedactsTo( CRedactor( REDACT_SANITIZE_ALL ), "http://h/\xC3\xBC\t?k=v", "http://h/???" "?k=v" ) );
            ASSERT_TRUE( SameText( CRedactor( REDACT_SANITIZE_CONTROL ).RedactHeaderValue( "X", "a\rb" ), "a?b" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformHeaderTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Headers which carry credentials are recognized by name" )
        try
        {
            const CRedactor& redactor = CRedactor::ForHeaders();
            ASSERT_TRUE( redactor.IsSensitiveName( "Authorization" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "authorization" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "PROXY-AUTHORIZATION" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "Cookie" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "Set-Cookie" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "x-webapi-key" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "X-Api-Key" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "X-Auth-Token" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "X-CSRF-Token" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "X-Amz-Security-Token" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "Sec-WebSocket-Key" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "password" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "client_secret" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "PWD" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "pw" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "Sig" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "sid" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "otp" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "id_token" ) );
            ASSERT_FALSE( redactor.IsSensitiveName( "Content-Type" ) );
            ASSERT_FALSE( redactor.IsSensitiveName( "Host" ) );
            ASSERT_FALSE( redactor.IsSensitiveName( "User-Agent" ) );
            ASSERT_FALSE( redactor.IsSensitiveName( "Accept" ) );
            ASSERT_FALSE( redactor.IsSensitiveName( "Content-Length" ) );
            ASSERT_FALSE( redactor.IsSensitiveName( "Keep-Alive" ) );
            ASSERT_FALSE( redactor.IsSensitiveName( "page" ) );
            ASSERT_FALSE( redactor.IsSensitiveName( "pwx" ) );
            ASSERT_FALSE( redactor.IsSensitiveName( "" ) );

            ASSERT_TRUE( SameText( redactor.RedactHeaderValue( "Authorization", "Bearer abc" ), "***" ) );
            ASSERT_TRUE( SameText( redactor.RedactHeaderValue( "Authorization", "" ), "***" ) );
            ASSERT_TRUE( SameText( redactor.RedactHeaderValue( "Cookie", "sid=1; other=2" ), "***" ) );
            ASSERT_TRUE( SameText( redactor.RedactHeaderValue( "Location", "http://h/p?code=1" ), "http://h/p?code=***" ) );
            ASSERT_TRUE( SameText( redactor.RedactHeaderValue( "referer", "http://u:p@h/p?t=1#f" ), "http://***@h/p?t=***#***" ) );
            ASSERT_TRUE( SameText( redactor.RedactHeaderValue( "Content-Type", "text/plain" ), "text/plain" ) );
            ASSERT_TRUE( SameText( redactor.RedactHeaderValue( "Host", "" ), "" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The text of a message header loses its secrets and keeps the rest and its line endings" )
        try
        {
            const CRedactor& redactor = CRedactor::ForHeaders();
            ASSERT_TRUE( HeadersRedactTo( redactor,
                "GET /api?key=SECRET HTTP/1.1\r\nHost: h\r\nAuthorization: Bearer ABC\r\nCookie: sid=XYZ\r\nx-webapi-key: K\r\nReferer: http://r/p?token=T\r\nContent-Type: text/plain\r\n\r\n",
                "GET /api?key=*** HTTP/1.1\r\nHost: h\r\nAuthorization: ***\r\nCookie: ***\r\nx-webapi-key: ***\r\nReferer: http://r/p?token=***\r\nContent-Type: text/plain\r\n\r\n" ) );
            ASSERT_TRUE( HeadersRedactTo( redactor,
                "HTTP/1.1 302 Found\r\nSet-Cookie: s=1; Path=/\r\nLocation: /x?code=9\r\nContent-Length: 0\r\n\r\n",
                "HTTP/1.1 302 Found\r\nSet-Cookie: ***\r\nLocation: /x?code=***\r\nContent-Length: 0\r\n\r\n" ) );
            ASSERT_TRUE( HeadersRedactTo( redactor, "POST http://h/p?k=v HTTP/1.1\nCookie:x\nHost:  h", "POST http://h/p?k=*** HTTP/1.1\nCookie: ***\nHost: h" ) );

            // not understood, so not known to be harmless
            ASSERT_TRUE( HeadersRedactTo( redactor, "GET\r\nnot a header SECRET\r\n", "GET\r\n***\r\n" ) );
            ASSERT_TRUE( HeadersRedactTo( redactor, "GET  HTTP/1.1\r\n", "GET  HTTP/1.1\r\n" ) );
            ASSERT_TRUE( HeadersRedactTo( redactor, "", "" ) );
            ASSERT_TRUE( HeadersRedactTo( redactor, "\r\n\r\n", "\r\n\r\n" ) );

            // a bare CR is not a line end, and does not get to fake a line
            ASSERT_TRUE( HeadersRedactTo( redactor, "GET / HTTP/1.1\r\nX-A: b\rc\r\n", "GET / HTTP/1.1\r\nX-A: b?c\r\n" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A header value that continues on the next line is redacted as the header is" )
        try
        {
            const CRedactor& redactor = CRedactor::ForHeaders();
            ASSERT_TRUE( HeadersRedactTo( redactor, "GET / HTTP/1.1\r\nCookie: a=1;\r\n b=SECRET2\r\n\tc: SECRET3\r\nHost: h\r\n",
                                                    "GET / HTTP/1.1\r\nCookie: ***\r\n ***\r\n\t***\r\nHost: h\r\n" ) );
            ASSERT_TRUE( HeadersRedactTo( redactor, "GET / HTTP/1.1\r\nAccept: a,\r\n b\r\n", "GET / HTTP/1.1\r\nAccept: a,\r\n b\r\n" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: REDACT_HEADER_VALUES, REDACT_HEADER_URLS and REDACT_HEADER_UNPARSEABLE_LINES each do their own part" )
        try
        {
            const CORE::CString block = "GET /a?k=v HTTP/1.1\r\nCookie: x\r\nLocation: http://h/p?k=v\r\nnot a header\r\n";

            ASSERT_TRUE( HeadersRedactTo( Only( REDACT_HEADER_VALUES | REDACT_NAME_WORDS ), block,
                "GET /a?k=v HTTP/1.1\r\nCookie: ***\r\nLocation: http://h/p?k=v\r\nnot a header\r\n" ) );
            ASSERT_TRUE( HeadersRedactTo( Only( REDACT_HEADER_URLS | REDACT_URL_QUERY_VALUES ), block,
                "GET /a?k=*** HTTP/1.1\r\nCookie: x\r\nLocation: http://h/p?k=***\r\nnot a header\r\n" ) );
            ASSERT_TRUE( HeadersRedactTo( Only( REDACT_HEADER_UNPARSEABLE_LINES ), block,
                "GET /a?k=v HTTP/1.1\r\nCookie: x\r\nLocation: http://h/p?k=v\r\n***\r\n" ) );
            ASSERT_TRUE( HeadersRedactTo( Only( REDACT_NONE ), block, block ) );

            // the header values of the other headers go through the text flags
            CRedactor text( REDACT_TEXT_AUTH_SCHEMES );
            ASSERT_TRUE( HeadersRedactTo( text, "GET / HTTP/1.1\r\nX-Forward: Bearer abc\r\n", "GET / HTTP/1.1\r\nX-Forward: Bearer ***\r\n" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The names of the sensitive headers can be added to, and need no rule to do it" )
        try
        {
            CRedactor redactor( CRedactor::ForHeaders() );
            ASSERT_TRUE( redactor.AddSensitiveName( "x-my-*" ) );
            ASSERT_TRUE( redactor.AddSensitiveName( "Account" ) );
            ASSERT_FALSE( redactor.AddSensitiveName( "bad{pattern" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "X-My-Thing" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "account" ) );
            ASSERT_FALSE( redactor.IsSensitiveName( "x-other" ) );
            ASSERT_FALSE( redactor.IsSensitiveName( "accounts" ) );
            ASSERT_TRUE( redactor.IsSensitiveName( "Authorization" ) );
            ASSERT_TRUE( HeadersRedactTo( redactor, "GET / HTTP/1.1\r\nX-My-Thing: abc\r\nX-Other: abc\r\nAccount: 7\r\n", "GET / HTTP/1.1\r\nX-My-Thing: ***\r\nX-Other: abc\r\nAccount: ***\r\n" ) );

            // and the preset did not change
            ASSERT_FALSE( CRedactor::ForHeaders().IsSensitiveName( "x-my-thing" ) );

            // only the list: the rule is a flag
            CRedactor onlyList( REDACT_HEADER_VALUES );
            ASSERT_TRUE( onlyList.AddSensitiveName( "x-internal" ) );
            ASSERT_FALSE( onlyList.IsSensitiveName( "Authorization" ) );
            ASSERT_TRUE( onlyList.IsSensitiveName( "X-INTERNAL" ) );
            ASSERT_TRUE( SameText( onlyList.RedactHeaderValue( "Authorization", "x" ), "x" ) );
            ASSERT_TRUE( SameText( onlyList.RedactHeaderValue( "X-Internal", "x" ), "***" ) );

            onlyList.ClearPatterns();
            ASSERT_FALSE( onlyList.IsSensitiveName( "X-INTERNAL" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformTextTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: REDACT_TEXT_URLS redacts the URLs in a text with the flags for URLs" )
        try
        {
            CRedactor redactor( REDACT_TEXT_URLS | REDACT_URL_USERINFO | REDACT_URL_QUERY_VALUES | REDACT_URL_FRAGMENT );
            ASSERT_TRUE( TextRedactsTo( redactor, "see http://h/p?k=abc.", "see http://h/p?k=***." ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "a <http://u:p@h/x?q=1#f> b", "a <http://***@h/x?q=***#***> b" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "{\"url\":\"https://h/p?token=T\",\"x\":1}", "{\"url\":\"https://h/p?token=***\",\"x\":1}" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "(see https://h/a_(b)?k=v)", "(see https://h/a_(b)?k=***)" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "two: ftp://h/x?a=1 and wss://u@h:1/?b=2, done", "two: ftp://h/x?a=*** and wss://***@h:1/?b=***, done" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "no url here: a://b or http:/x", "no url here: a://b or http:/x" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "xhttp://h/p?k=v", "xhttp://h/p?k=***" ) );

            // without the flag a URL is text
            CRedactor off( REDACT_URL_USERINFO | REDACT_URL_QUERY_VALUES );
            ASSERT_TRUE( TextRedactsTo( off, "see http://u:p@h/p?k=abc.", "see http://u:p@h/p?k=abc." ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: REDACT_TEXT_AUTH_SCHEMES redacts the credential after Bearer, Basic, Digest, Negotiate and NTLM" )
        try
        {
            CRedactor redactor( REDACT_TEXT_AUTH_SCHEMES );
            ASSERT_TRUE( TextRedactsTo( redactor, "Authorization: Bearer abc.def", "Authorization: Bearer ***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "bearer   tok-en_1~2+3/4=", "bearer   ***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "got Bearer abc, then", "got Bearer ***, then" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "Basic dXNlcjpwYXNz", "Basic ***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==", "Basic ***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "NTLM TlRMTVNTUAABAAAA", "NTLM ***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "Negotiate YIIBsQYGKwYBBQUCoIIBpTCCAaGg", "Negotiate ***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "Digest username=\"bob\", realm=\"x\", response=\"abc\"\nnext", "Digest ***\nnext" ) );

            // words are not credentials
            ASSERT_TRUE( TextRedactsTo( redactor, "basic setup failed", "basic setup failed" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "Basic information here", "Basic information here" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "Digest of the file", "Digest of the file" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "Bearer", "Bearer" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "Bearer ", "Bearer " ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "xBearer abc", "xBearer abc" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "Bearer ***", "Bearer ***" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: REDACT_TEXT_KEY_VALUES redacts the value of a pair of which the name is sensitive" )
        try
        {
            CRedactor redactor( REDACT_TEXT_KEY_VALUES | REDACT_NAME_WORDS );
            ASSERT_TRUE( TextRedactsTo( redactor, "password=hunter2 user=bob", "password=*** user=bob" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "user=bob password: hunter2 and more", "user=bob password: ***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "Cookie: a=1; b=2\nHost: h", "Cookie: ***\nHost: h" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "Authorization: Bearer abc", "Authorization: ***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "{\"token\":\"abc\",\"user\":\"bob\"}", "{\"token\":\"***\",\"user\":\"bob\"}" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "{\"api_key\": \"a b c\", \"n\": 1}", "{\"api_key\": \"***\", \"n\": 1}" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "{\"password\": 12345, \"n\": 1}", "{\"password\": ***, \"n\": 1}" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "{'secret':'a\\'b','n':1}", "{'secret':'***','n':1}" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "a=1&password=x&b=2", "a=1&password=***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "password = hunter2", "password = ***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "password= x", "password= ***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "password=\"abc def\" next", "password=\"***\" next" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "password=\"abc", "password=\"***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "client.secret=abc", "client.secret=***" ) );

            // no value, no marker
            ASSERT_TRUE( TextRedactsTo( redactor, "password=", "password=" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "password:\nnext=1", "password:\nnext=1" ) );

            // what is not a sensitive name is text
            ASSERT_TRUE( TextRedactsTo( redactor, "user=bob Keep-Alive: timeout=5 code=404", "user=bob Keep-Alive: timeout=5 code=404" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "the password is hunter2", "the password is hunter2" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "password", "password" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "password=***", "password=***" ) );

            // the names of the list count as well
            CRedactor listed( REDACT_TEXT_KEY_VALUES );
            ASSERT_TRUE( listed.AddSensitiveName( "ssn" ) );
            ASSERT_TRUE( TextRedactsTo( listed, "ssn=123-45-6789 password=x", "ssn=*** password=x" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: REDACT_TEXT_TOKEN_RUNS redacts long runs that look like a key, a digest or a token" )
        try
        {
            CRedactor redactor( REDACT_TEXT_TOKEN_RUNS );
            ASSERT_TRUE( redactor.GetTokenMinLength() == 32 );
            ASSERT_TRUE( TextRedactsTo( redactor, "md5 d41d8cd98f00b204e9800998ecf8427e done", "md5 *** done" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "md5 d41d8cd98f00b204e9800998ecf8427 done", "md5 d41d8cd98f00b204e9800998ecf8427 done" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "key AbCdEf0123456789AbCdEf0123456789+/==", "key ***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "sha=0123456789abcdef0123456789abcdef", "sha=***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIn0.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c", "***.***.***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "[sk-live-0123456789abcdefghijklmnopqrstuvwxyz]", "[***]" ) );

            // words and paths are not tokens
            ASSERT_TRUE( TextRedactsTo( redactor, "internationalization_configuration_manager_class", "internationalization_configuration_manager_class" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "/home/liberator/CODE/GUCEF/platform/gucefCORE/src/gucefCORE_CRedactor.cpp", "/home/liberator/CODE/GUCEF/platform/gucefCORE/src/gucefCORE_CRedactor.cpp" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "/var/log/app-server-2025-10-28/errors.log", "/var/log/app-server-2025-10-28/errors.log" ) );

            // the length that counts is set, and is never less than eight
            redactor.SetTokenMinLength( 10 );
            ASSERT_TRUE( TextRedactsTo( redactor, "id 0123456789 x", "id *** x" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "id 012345678 x", "id 012345678 x" ) );
            redactor.SetTokenMinLength( 3 );
            ASSERT_TRUE( redactor.GetTokenMinLength() == 8 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: REDACT_TEXT_EMAILS redacts e-mail addresses" )
        try
        {
            CRedactor redactor( REDACT_TEXT_EMAILS );
            ASSERT_TRUE( TextRedactsTo( redactor, "mail bob.smith@example.com now", "mail *** now" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "(x_y+z@sub.example.org)", "(***)" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "to bob@example.com.", "to ***." ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "a@b.co", "***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "user=bob@example.com&x=1", "user=***&x=1" ) );

            ASSERT_TRUE( TextRedactsTo( redactor, "bob@localhost", "bob@localhost" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "a@b.c", "a@b.c" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "@handle and 5 @ 10", "@handle and 5 @ 10" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "bob@exa_mple.com", "bob@exa_mple.com" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: REDACT_TEXT_IPS redacts IPv4 and IPv6 addresses" )
        try
        {
            CRedactor redactor( REDACT_TEXT_IPS );
            ASSERT_TRUE( TextRedactsTo( redactor, "from 192.168.1.20:80", "from ***:80" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "0.0.0.0 255.255.255.255", "*** ***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "::1 and fe80::1ff:fe23:4567:890a", "*** and ***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "[2001:db8::1]:443", "[***]:443" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "::ffff:1.2.3.4", "***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "1:2:3:4:5:6:7:8", "***" ) );

            ASSERT_TRUE( TextRedactsTo( redactor, "1.2.3.4.5", "1.2.3.4.5" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "999.1.1.1 and 1.2.3", "999.1.1.1 and 1.2.3" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "v1.2.3.4 build", "v1.2.3.4 build" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "at 12:30:45 ok", "at 12:30:45 ok" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "std::vector Foo :: Bar", "std::vector Foo :: Bar" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "dead::beefg", "dead::beefg" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: REDACT_TEXT_CARD_NUMBERS redacts numbers that pass the check of payment cards" )
        try
        {
            CRedactor redactor( REDACT_TEXT_CARD_NUMBERS );
            ASSERT_TRUE( TextRedactsTo( redactor, "card 4111 1111 1111 1111 ok", "card *** ok" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "4111-1111-1111-1111", "***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "5500005555555559.", "***." ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "340000000000009", "***" ) );

            ASSERT_TRUE( TextRedactsTo( redactor, "4111111111111112", "4111111111111112" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "order 1234567890123456789012", "order 1234567890123456789012" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "4111111111111111x", "4111111111111111x" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "2026-10-28 12:30", "2026-10-28 12:30" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "411111111111", "411111111111" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: All the flags for text work together, and each finds what it is for" )
        try
        {
            CRedactor redactor( REDACT_TEXT_ALL | REDACT_NAME_WORDS | REDACT_URL_ALL );
            ASSERT_TRUE( TextRedactsTo( redactor,
                "user bob@example.com from 10.0.0.1 token=SECRETX card 4111111111111111 url http://u:p@h/p?k=v Authorization: Bearer abc",
                "user *** from *** token=*** card *** url http://***@h/p?k=*** Authorization: ***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "", "" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "nothing to see here", "nothing to see here" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformPresetTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The presets are what they are said to be" )
        try
        {
            ASSERT_TRUE( CRedactor::ForUrl().GetMaxLength() == 1024 );
            ASSERT_TRUE( ( CRedactor::ForUrl().GetFlags() & REDACT_SANITIZE_ALL ) == REDACT_SANITIZE_ALL );
            ASSERT_TRUE( ( CRedactor::ForUrl().GetFlags() & REDACT_HEADER_ALL ) == 0 );
            ASSERT_TRUE( ( CRedactor::ForHeaders().GetFlags() & REDACT_HEADER_ALL ) == REDACT_HEADER_ALL );
            ASSERT_TRUE( CRedactor::ForHeaders().GetMaxLength() == 16384 );
            ASSERT_TRUE( ( CRedactor::ForLogLine().GetFlags() & REDACT_SANITIZE_CONTROL ) != 0 );
            ASSERT_TRUE( ( CRedactor::ForLogLine().GetFlags() & REDACT_SANITIZE_NON_ASCII ) == 0 );
            ASSERT_TRUE( CRedactor::ForLogLine().GetMaxLength() == 4096 );
            ASSERT_TRUE( ( CRedactor::ForDisplay().GetFlags() & REDACT_URL_QUERY_VALUES ) == 0 );
            ASSERT_TRUE( ( CRedactor::ForDisplay().GetFlags() & REDACT_URL_QUERY_LISTED ) != 0 );
            ASSERT_TRUE( CRedactor::ForDisplay().GetMaxLength() == 256 );

            // the same object every time
            ASSERT_TRUE( &CRedactor::ForUrl() == &CRedactor::ForUrl() );
            ASSERT_TRUE( &CRedactor::ForUrl() != &CRedactor::ForHeaders() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: ForLogLine makes a line of text safe to log" )
        try
        {
            const CRedactor& redactor = CRedactor::ForLogLine();
            ASSERT_TRUE( TextRedactsTo( redactor, "GET failed: https://api.x.com/v1?key=SECRET Authorization: Bearer abc\r\nnext",
                                                  "GET failed: https://api.x.com/v1?key=*** Authorization: ***??next" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "Gr\xC3\xBC\xC3\x9F" "e token=abc", "Gr\xC3\xBC\xC3\x9F" "e token=***" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "bob@example.com from 10.0.0.1 card 4111111111111111", "bob@example.com from 10.0.0.1 card 4111111111111111" ) );
            ASSERT_TRUE( TextRedactsTo( redactor, "digest d41d8cd98f00b204e9800998ecf8427e", "digest ***" ) );
            ASSERT_TRUE( HasNoControlCharacters( redactor.RedactText( "a\x01" "b\x7F" "c\r\nd\te" ) ) );

            CORE::CString cut = redactor.RedactText( RepeatText( "word ", 3000 ) );
            ASSERT_TRUE( Bytes( cut ) == 4096 && cut.EndsWith( "..." ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: ForDisplay shows what a user may see of a URL, in the characters of the user" )
        try
        {
            const CRedactor& redactor = CRedactor::ForDisplay();
            ASSERT_TRUE( UrlRedactsTo( redactor, "https://u:p@h.example/p?access_token=T&page=2&api_key=K#frag", "https://***@h.example/p?access_token=***&page=2&api_key=***#***" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "https://ex\xC3\xA4mple.com/p\xC3\xBC?q=s\xC3\xBC", "https://ex\xC3\xA4mple.com/p\xC3\xBC?q=s\xC3\xBC" ) );
            ASSERT_TRUE( UrlRedactsTo( CRedactor::ForUrl(), "https://ex\xC3\xA4mple.com/p", "https://***/***" ) );
            ASSERT_TRUE( UrlRedactsTo( redactor, "https://h/p\r\nfake?token=SECRET", "https://h/p??fake?token=***" ) );
            ASSERT_TRUE( SameText( redactor.RedactText( "see https://u:p@h/x?pw=1 and Bearer abcdefgh1" ), "see https://***@h/x?pw=*** and Bearer ***" ) );

            CORE::CString cut = redactor.RedactUrl( "https://h/" + Repeat( 'a', 1000 ) );
            ASSERT_TRUE( cut.Length() == 256 && cut.EndsWith( "..." ) );

            // a text that is cut keeps its UTF-8 intact
            CORE::CString cutText = redactor.RedactText( RepeatText( "\xC3\xBC", 300 ) );
            ASSERT_TRUE( Bytes( cutText ) <= 256 && cutText.EndsWith( "..." ) && cutText.StartsWith( "\xC3\xBC\xC3\xBC" ) );
            ASSERT_TRUE( 0 == ( ( Bytes( cutText ) - 3 ) % 2 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}


/*-------------------------------------------------------------------------*/

/** A text that holds a secret and the secret, which must not be in what is made of the text */
struct SSecretCase
{
    const char* text;
    const char* secret;
};

/*-------------------------------------------------------------------------*/

/** URLs and parts of URLs, in all the places that carry a secret */
static const SSecretCase UrlCorpus[] =
{
    { "https://api.example.com/v1/me?access_token=SECRETVALUE1", "SECRETVALUE1" },
    { "https://user:SECRETPASSWORD@h.example/x", "SECRETPASSWORD" },
    { "https://SECRETUSER@h.example/x", "SECRETUSER" },
    { "https://h/cb?code=SECRETCODE&state=2", "SECRETCODE" },
    { "https://h/cb?code=1&state=SECRETSTATE", "SECRETSTATE" },
    { "https://h/cb#access_token=SECRETFRAG&token_type=bearer", "SECRETFRAG" },
    { "https://h/cb?x=1#id_token=SECRETIDTOKEN", "SECRETIDTOKEN" },
    { "https://h/?SECRETBARE", "SECRETBARE" },
    { "ws://h:81/s?token=SECRETWS", "SECRETWS" },
    { "//h/x?key=SECRETKEY", "SECRETKEY" },
    { "/path?apikey=SECRETAPIKEY&a=b", "SECRETAPIKEY" },
    { "?client_secret=SECRETCLIENT", "SECRETCLIENT" },
    { "http://h:SECRETPORT/path", "SECRETPORT" },
    { "http://SECRETHOST.example:80@h/x", "SECRETHOST" },
    { "http://u:p%40SECRETENC@h/", "SECRETENC" },
    { "http://h/p?a=1&%73ecret=SECRETENCNAME", "SECRETENCNAME" },
    { "http://h/p?k=SECRET%26MORE", "SECRET" },
    { "http://h/p?k=a;SECRETSEMI", "SECRETSEMI" },
    { "http://h/p?k=a?SECRETQ", "SECRETQ" },
    { "http://h/p?k=a#SECRETHASH#SECRETHASH2", "SECRETHASH" },
    { "http://ho st SECRETSPACE/p", "SECRETSPACE" },
    { "http://h/p?k=v\r\nERROR SECRETINJECT", "SECRETINJECT" },
    { "http://h/p?k=\xC3\xBC" "SECRETUTF&j=2", "SECRETUTF" },
    { "http://[::1]:SECRETV6/p", "SECRETV6" },
    { "user:SECRETBARE2@host/p", "SECRETBARE2" },
    { "mailto:SECRETMAIL@example.com", "SECRETMAIL" },
    { "javascript:alert('SECRETJS')", "SECRETJS" },
    { "http://h/p?k=1&k=SECRETREPEAT", "SECRETREPEAT" },
    { "http://h/p?=SECRETNONAME", "SECRETNONAME" },
    { "http://h/p?a=b=SECRETEQ", "SECRETEQ" },
    { "http://h/p?SECRETNAMEONLY&b=1", "SECRETNAMEONLY" },
    { "http://:SECRETEMPTYUSER@h/", "SECRETEMPTYUSER" },
    { "http://u:p:SECRETCOLON@h/", "SECRETCOLON" },
    { "http://h/p?k=%SECRETPERCENT2", "SECRETPERCENT2" }
};

/** Headers of messages, all of which have a secret in the place where it is found */
static const SSecretCase HeaderCorpus[] =
{
    { "GET /api?key=SECRETKEY HTTP/1.1\r\nHost: h\r\n\r\n", "SECRETKEY" },
    { "GET / HTTP/1.1\r\nAuthorization: Bearer SECRETBEARER\r\n", "SECRETBEARER" },
    { "GET / HTTP/1.1\r\nauthorization:SECRETNOSPACE\r\n", "SECRETNOSPACE" },
    { "GET / HTTP/1.1\r\nCookie: sid=SECRETSID; a=b\r\n", "SECRETSID" },
    { "HTTP/1.1 200 OK\r\nSet-Cookie: s=SECRETSET; Path=/\r\n", "SECRETSET" },
    { "GET / HTTP/1.1\r\nX-Api-Key: SECRETAPIKEY\r\n", "SECRETAPIKEY" },
    { "GET / HTTP/1.1\r\nX-Amz-Security-Token: SECRETAMZ\r\n", "SECRETAMZ" },
    { "GET / HTTP/1.1\r\nCookie: a=1;\r\n b=SECRETFOLD\r\n", "SECRETFOLD" },
    { "GET / HTTP/1.1\r\nLocation: http://h/p?code=SECRETLOC\r\n", "SECRETLOC" },
    { "GET / HTTP/1.1\r\nReferer: http://u:SECRETREF@h/\r\n", "SECRETREF" },
    { "GET / HTTP/1.1\r\nRefresh: 0; url=http://h/n?t=SECRETREFRESH\r\n", "SECRETREFRESH" },
    { "GET / HTTP/1.1\r\nnot a header SECRETLINE\r\n", "SECRETLINE" },
    { "GET http://SECRETU:p@h/ HTTP/1.1\r\n", "SECRETU" },
    { "GET / HTTP/1.1\nX-Session-Id: SECRETSESSION\n", "SECRETSESSION" },
    { "GET / HTTP/1.1\r\nX-Foo: ok\r\nProxy-Authorization: Basic SECRETPROXY\r\n", "SECRETPROXY" },
    { "GET / HTTP/1.1\r\nSec-WebSocket-Key: SECRETWSKEY\r\n", "SECRETWSKEY" },
    { "GET / HTTP/1.1\r\nX-Token:SECRETTOKENNOSPACE", "SECRETTOKENNOSPACE" }
};

/** Free text, all of which has a secret that is found by the way it is written */
static const SSecretCase TextCorpus[] =
{
    { "Authorization: Bearer SECRETBEARER", "SECRETBEARER" },
    { "token=SECRETTOKEN rest", "SECRETTOKEN" },
    { "{\"password\":\"SECRETJSONPW\",\"user\":\"bob\"}", "SECRETJSONPW" },
    { "client_secret=SECRETCLIENT&grant_type=x", "SECRETCLIENT" },
    { "Cookie: sid=SECRETSID; other=1", "SECRETSID" },
    { "failed for https://u:SECRETPW@h/p?key=1, retrying", "SECRETPW" },
    { "failed for https://u:p@h/p?key=SECRETKEY, retrying", "SECRETKEY" },
    { "api key is 0123456789abcdef0123456789abcdef", "0123456789abcdef0123456789abcdef" },
    { "using sk-live-SECRETKEYVALUE0123456789abcdefghij now", "SECRETKEYVALUE" },
    { "Proxy-Authorization: Basic U0VDUkVUOlNFQ1JFVA==", "U0VDUkVU" },
    { "Basic U0VDUkVUOlNFQ1JFVA== end", "U0VDUkVU" },
    { "got Bearer SECRETBEARER2 end", "SECRETBEARER2" },
    { "refresh_token: SECRETREFRESHTOKEN", "SECRETREFRESHTOKEN" },
    { "session=SECRETSESSION; path=/", "SECRETSESSION" },
    { "x-api-key=SECRETXAPI", "SECRETXAPI" },
    { "{'password': 'SECRETPYDICT'}", "SECRETPYDICT" },
    { "PASSWORD = \"SECRET PHRASE\"", "SECRET PHRASE" },
    { "url=http://h/p?token=SECRETURLCODE", "SECRETURLCODE" },
    { "{\"token\": \"SECRETJSON2\"}", "SECRETJSON2" },
    { "Digest username=\"bob\", response=\"SECRETDIGEST\"", "SECRETDIGEST" },
    { "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiJTRUNSRVRKV1QifQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c", "SflKxwRJSMeKKF2QT4fw" }
};

/** URLs for the interface, in which only what is named as secret is replaced */
static const SSecretCase DisplayCorpus[] =
{
    { "https://u:SECRETPW@h/p", "SECRETPW" },
    { "https://h/p?access_token=SECRETAT&page=2", "SECRETAT" },
    { "https://h/p?API_KEY=SECRETAK", "SECRETAK" },
    { "https://h/p#SECRETFRAGD", "SECRETFRAGD" },
    { "https://h/p?c%6Fokie=SECRETCK", "SECRETCK" },
    { "https://h/p?sid=SECRETSID", "SECRETSID" },
    { "http://h:SECRETBADPORT/p", "SECRETBADPORT" },
    { "https://h/p?pass%77ord=SECRETPE", "SECRETPE" }
};

#define NR_OF( a ) ( sizeof( a ) / sizeof( a[ 0 ] ) )

/*-------------------------------------------------------------------------*/

static bool
HasWhiteSpace( const CORE::CString& text )
{
    const char* chars = text.C_String();
    UInt32 length = Bytes( text );
    for ( UInt32 i=0; i<length; ++i )
    {
        if ( (unsigned char) chars[ i ] <= ' ' )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** a redactor which does all there is to do, for texts that must hold nothing */
static CRedactor
MakeParanoidRedactor( void )
{
    CRedactor redactor( REDACT_URL_ALL | REDACT_HEADER_ALL | REDACT_TEXT_ALL | REDACT_NAME_WORDS | REDACT_SANITIZE_ALL );
    redactor.SetMaxLength( 0 );
    return redactor;
}

/*-------------------------------------------------------------------------*/

static void
PerformSecretCorpusTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: No secret of the corpus of URLs survives the redaction of a URL" )
        try
        {
            const CRedactor paranoid = MakeParanoidRedactor();
            for ( UInt32 i=0; i<NR_OF( UrlCorpus ); ++i )
            {
                CORE::CString viaPreset = CRedactor::ForUrl().RedactUrl( UrlCorpus[ i ].text );
                CORE::CString viaParanoid = paranoid.RedactUrl( UrlCorpus[ i ].text );
                if ( Contains( viaPreset, UrlCorpus[ i ].secret ) || Contains( viaParanoid, UrlCorpus[ i ].secret ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Redactor test: the secret \"" << UrlCorpus[ i ].secret << "\" survived in \"" << viaPreset << "\" / \"" << viaParanoid << "\"" );
                    ASSERT_TRUE( false );
                }
                ASSERT_TRUE( IsPlainLine( viaPreset ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: No secret of the corpus of URLs survives when the URL is part of a text" )
        try
        {
            const CRedactor paranoid = MakeParanoidRedactor();
            for ( UInt32 i=0; i<NR_OF( UrlCorpus ); ++i )
            {
                CORE::CString url = UrlCorpus[ i ].text;
                if ( url.HasSubstr( "://", true ) < 0 || HasWhiteSpace( url ) )
                    continue;
                CORE::CString text = "failed to fetch " + url + " after 3 tries";
                CORE::CString viaLogLine = CRedactor::ForLogLine().RedactText( text );
                CORE::CString viaParanoid = paranoid.RedactText( text );
                if ( Contains( viaLogLine, UrlCorpus[ i ].secret ) || Contains( viaParanoid, UrlCorpus[ i ].secret ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Redactor test: the secret \"" << UrlCorpus[ i ].secret << "\" survived in \"" << viaLogLine << "\" / \"" << viaParanoid << "\"" );
                    ASSERT_TRUE( false );
                }
                ASSERT_TRUE( viaLogLine.StartsWith( "failed to fetch " ) && viaLogLine.EndsWith( " after 3 tries" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: No secret of the corpus of headers survives the redaction of the headers" )
        try
        {
            const CRedactor paranoid = MakeParanoidRedactor();
            for ( UInt32 i=0; i<NR_OF( HeaderCorpus ); ++i )
            {
                CORE::CString viaPreset = CRedactor::ForHeaders().RedactHeaders( HeaderCorpus[ i ].text );
                CORE::CString viaParanoid = paranoid.RedactHeaders( HeaderCorpus[ i ].text );
                if ( Contains( viaPreset, HeaderCorpus[ i ].secret ) || Contains( viaParanoid, HeaderCorpus[ i ].secret ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Redactor test: the secret \"" << HeaderCorpus[ i ].secret << "\" survived in \"" << viaPreset << "\" / \"" << viaParanoid << "\"" );
                    ASSERT_TRUE( false );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: No secret of the corpus of text survives the redaction of the text" )
        try
        {
            const CRedactor paranoid = MakeParanoidRedactor();
            for ( UInt32 i=0; i<NR_OF( TextCorpus ); ++i )
            {
                CORE::CString viaLogLine = CRedactor::ForLogLine().RedactText( TextCorpus[ i ].text );
                CORE::CString viaParanoid = paranoid.RedactText( TextCorpus[ i ].text );
                CORE::CString viaDisplay = CRedactor::ForDisplay().RedactText( TextCorpus[ i ].text );
                if ( Contains( viaLogLine, TextCorpus[ i ].secret ) || Contains( viaParanoid, TextCorpus[ i ].secret ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Redactor test: the secret \"" << TextCorpus[ i ].secret << "\" survived in \"" << viaLogLine << "\" / \"" << viaParanoid << "\"" );
                    ASSERT_TRUE( false );
                }

                // what the interface shows holds none of them either, except what the interface does not look for
                if ( !Contains( TextCorpus[ i ].text, "sk-live" ) && !Contains( TextCorpus[ i ].text, "api key is" ) && !Contains( TextCorpus[ i ].text, "eyJ" ) )
                {
                    if ( Contains( viaDisplay, TextCorpus[ i ].secret ) )
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Redactor test: the secret \"" << TextCorpus[ i ].secret << "\" survived for display in \"" << viaDisplay << "\"" );
                        ASSERT_TRUE( false );
                    }
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: No secret of the corpus for the interface survives the display preset" )
        try
        {
            for ( UInt32 i=0; i<NR_OF( DisplayCorpus ); ++i )
            {
                CORE::CString shown = CRedactor::ForDisplay().RedactUrl( DisplayCorpus[ i ].text );
                if ( Contains( shown, DisplayCorpus[ i ].secret ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Redactor test: the secret \"" << DisplayCorpus[ i ].secret << "\" survived for display in \"" << shown << "\"" );
                    ASSERT_TRUE( false );
                }
                ASSERT_TRUE( shown.StartsWith( "http" ) );
            }
            ASSERT_TRUE( SameText( CRedactor::ForDisplay().RedactUrl( "https://h/p?access_token=SECRETAT&page=2" ), "https://h/p?access_token=***&page=2" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformIdempotenceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: What is redacted already is redacted as it is: redacting twice gives the same text" )
        try
        {
            const CRedactor paranoid = MakeParanoidRedactor();
            CRedactor bracketed( CRedactor::ForUrl() );
            bracketed.SetMarker( "[hidden]" );

            for ( UInt32 i=0; i<NR_OF( UrlCorpus ); ++i )
            {
                CORE::CString once = CRedactor::ForUrl().RedactUrl( UrlCorpus[ i ].text );
                ASSERT_TRUE( SameText( CRedactor::ForUrl().RedactUrl( once ), once ) );
                once = paranoid.RedactUrl( UrlCorpus[ i ].text );
                ASSERT_TRUE( SameText( paranoid.RedactUrl( once ), once ) );
                once = bracketed.RedactUrl( UrlCorpus[ i ].text );
                ASSERT_TRUE( SameText( bracketed.RedactUrl( once ), once ) );
                once = CRedactor::ForDisplay().RedactUrl( UrlCorpus[ i ].text );
                ASSERT_TRUE( SameText( CRedactor::ForDisplay().RedactUrl( once ), once ) );
            }
            for ( UInt32 i=0; i<NR_OF( DisplayCorpus ); ++i )
            {
                CORE::CString once = CRedactor::ForDisplay().RedactUrl( DisplayCorpus[ i ].text );
                ASSERT_TRUE( SameText( CRedactor::ForDisplay().RedactUrl( once ), once ) );
            }
            for ( UInt32 i=0; i<NR_OF( HeaderCorpus ); ++i )
            {
                CORE::CString once = CRedactor::ForHeaders().RedactHeaders( HeaderCorpus[ i ].text );
                ASSERT_TRUE( SameText( CRedactor::ForHeaders().RedactHeaders( once ), once ) );
                once = paranoid.RedactHeaders( HeaderCorpus[ i ].text );
                ASSERT_TRUE( SameText( paranoid.RedactHeaders( once ), once ) );
            }
            for ( UInt32 i=0; i<NR_OF( TextCorpus ); ++i )
            {
                CORE::CString once = CRedactor::ForLogLine().RedactText( TextCorpus[ i ].text );
                ASSERT_TRUE( SameText( CRedactor::ForLogLine().RedactText( once ), once ) );
                once = paranoid.RedactText( TextCorpus[ i ].text );
                ASSERT_TRUE( SameText( paranoid.RedactText( once ), once ) );
                once = CRedactor::ForDisplay().RedactText( TextCorpus[ i ].text );
                ASSERT_TRUE( SameText( CRedactor::ForDisplay().RedactText( once ), once ) );
            }
            for ( UInt32 i=0; i<NR_OF( UrlCorpus ); ++i )
            {
                CORE::CString url = UrlCorpus[ i ].text;
                if ( url.HasSubstr( "://", true ) < 0 || HasWhiteSpace( url ) )
                    continue;
                CORE::CString once = CRedactor::ForLogLine().RedactText( "x " + url + " y" );
                ASSERT_TRUE( SameText( CRedactor::ForLogLine().RedactText( once ), once ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** a number from a sequence that is the same for every run */
static UInt32
NextRandom( UInt32& state )
{
    state = state * 1664525U + 1013904223U;
    return state >> 8;
}

/*-------------------------------------------------------------------------*/

static void
PerformHostileInputTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Empty and missing input gives empty output" )
        try
        {
            ASSERT_TRUE( SameText( CRedactor::ForUrl().RedactUrl( CORE::CString() ), "" ) );
            ASSERT_TRUE( SameText( CRedactor::ForHeaders().RedactHeaders( CORE::CString() ), "" ) );
            ASSERT_TRUE( SameText( CRedactor::ForLogLine().RedactText( CORE::CString() ), "" ) );
            ASSERT_TRUE( SameText( CRedactor::ForDisplay().RedactText( "" ), "" ) );
            ASSERT_TRUE( SameText( CRedactor::ForHeaders().RedactHeaderValue( "", "" ), "" ) );
            ASSERT_TRUE( SameText( CRedactor::ForHeaders().RedactHeaderValue( CORE::CString(), CORE::CString() ), "" ) );
            ASSERT_TRUE( !CRedactor::ForHeaders().IsSensitiveName( CORE::CString() ) );
            ASSERT_TRUE( SameText( CRedactor::ForUrl().RedactUrl( "?" ), "?" ) );
            ASSERT_TRUE( SameText( CRedactor::ForUrl().RedactUrl( "#" ), "#" ) );
            ASSERT_TRUE( SameText( CRedactor::ForUrl().RedactUrl( "@" ), "***@" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Every byte value is accepted, and none of the control characters is in the result" )
        try
        {
            // every value but NUL, which the other test has
            char all[ 255 ];
            for ( UInt32 i=0; i<255; ++i )
                all[ i ] = (char) ( i + 1 );
            CORE::CString bytes( all, 255 );
            CORE::CString url = CORE::CString( "http://h/" ) + bytes + "?k=SECRETVALUE";

            CORE::CString viaUrl = CRedactor::ForUrl().RedactUrl( url );
            ASSERT_TRUE( IsPlainLine( viaUrl ) && !Contains( viaUrl, "SECRETVALUE" ) );
            ASSERT_TRUE( HasNoControlCharacters( CRedactor::ForLogLine().RedactText( bytes ) ) );
            ASSERT_TRUE( HasNoControlCharacters( CRedactor::ForDisplay().RedactText( bytes ) ) );
            ASSERT_TRUE( HasNoControlCharacters( CRedactor::ForDisplay().RedactUrl( url ) ) );
            CORE::CString viaHeaders = CRedactor::ForHeaders().RedactHeaders( "GET / HTTP/1.1\r\nX-A: " + bytes + "\r\n" );
            ASSERT_TRUE( viaHeaders.StartsWith( "GET / HTTP/1.1\r\nX-A: " ) && viaHeaders.EndsWith( "\r\n" ) );
            MakeParanoidRedactor().RedactText( bytes );
            char withNul[ 256 ];
            for ( UInt32 i=0; i<256; ++i )
                withNul[ i ] = (char) i;
            ASSERT_TRUE( IsPlainLine( CRedactor::ForUrl().RedactUrlBytes( withNul, 256 ) ) );
            ASSERT_TRUE( SameText( CRedactor::ForUrl().RedactUrlBytes( withNul, 1 ), "***" ) );
            MakeParanoidRedactor().RedactHeaders( bytes );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A hundred thousand bytes of any structure character give a result of limited size" )
        try
        {
            const char* const structure = "?&=#@:/%[]\r\n \t.-_;,\"'{}<>*";
            const CRedactor paranoid = MakeParanoidRedactor();
            for ( UInt32 s=0; structure[ s ] != '\0'; ++s )
            {
                CORE::CString input = Repeat( structure[ s ], 100000 );
                ASSERT_TRUE( CRedactor::ForUrl().RedactUrl( input ).Length() <= 1024 );
                ASSERT_TRUE( CRedactor::ForUrl().RedactUrl( "http://h/p?k=v" + input ).Length() <= 1024 );
                ASSERT_TRUE( CRedactor::ForUrl().RedactUrl( "http://" + input + "/p" ).Length() <= 1024 );
                ASSERT_TRUE( CRedactor::ForUrl().RedactUrl( "http://h/p#" + input ).Length() <= 1024 );
                ASSERT_TRUE( CRedactor::ForHeaders().RedactHeaders( input ).Length() <= 16384 );
                ASSERT_TRUE( CRedactor::ForHeaders().RedactHeaders( "GET / HTTP/1.1\r\n" + input ).Length() <= 16384 );
                ASSERT_TRUE( Bytes( CRedactor::ForLogLine().RedactText( input ) ) <= 4096 );
                ASSERT_TRUE( Bytes( CRedactor::ForDisplay().RedactText( input ) ) <= 256 );
                CORE::CString viaParanoid = paranoid.RedactText( input );
                paranoid.RedactHeaders( input );
                paranoid.RedactUrl( input );
                ASSERT_TRUE( Bytes( viaParanoid ) <= 100000 * 3 );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Megabytes of input are no problem and give no secrets" )
        try
        {
            ASSERT_TRUE( SameText( CRedactor::ForHeaders().RedactHeaders( "GET / HTTP/1.1\r\nCookie: " + Repeat( 'x', 4 * 1024 * 1024 ) ), "GET / HTTP/1.1\r\nCookie: ***" ) );
            ASSERT_TRUE( SameText( CRedactor::ForLogLine().RedactText( "password=" + Repeat( 'x', 4 * 1024 * 1024 ) ), "password=***" ) );
            ASSERT_TRUE( SameText( CRedactor::ForLogLine().RedactText( "https://h/p?k=" + Repeat( 'x', 4 * 1024 * 1024 ) ), "https://h/p?k=***" ) );
            ASSERT_TRUE( SameText( CRedactor::ForLogLine().RedactText( "token: " + Repeat( 'x', 4 * 1024 * 1024 ) + "\nnext" ), "token: ***?next" ) );
            ASSERT_TRUE( SameText( CRedactor::ForUrl().RedactUrl( "http://" + Repeat( 'u', 4 * 1024 * 1024 ) + "@h/p" ), "http://***@h/p" ) );

            // no limit: all of it is looked at, in time that goes with its size
            CRedactor unlimited( MakeParanoidRedactor() );
            CORE::CString dotted = RepeatText( "a.", 500000 );
            ASSERT_TRUE( unlimited.RedactText( dotted ) == dotted );
            CORE::CString addresses = RepeatText( "1.1.1.1 ", 100000 );
            CORE::CString hidden = unlimited.RedactText( addresses );
            ASSERT_TRUE( hidden.Length() == 100000 * 4 && hidden.StartsWith( "*** *** ***" ) );
            CORE::CString digits = RepeatText( "1 ", 500000 );
            ASSERT_TRUE( unlimited.RedactText( digits ) == digits );
            CORE::CString pairs = RepeatText( "a=", 500000 );
            ASSERT_TRUE( unlimited.RedactText( pairs ) == pairs );
            CORE::CString names = RepeatText( "password=x ", 100000 );
            ASSERT_TRUE( unlimited.RedactText( names ) == RepeatText( "password=*** ", 100000 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Strings made of the parts of URLs, headers and text give a result that is within its limits and plain" )
        try
        {
            static const char* const parts[] =
            {
                "http://", "https://", "ws://", "//", "/", "?", "#", "&", "=", "@", ":", "%", "%41", "%00", ".", "[", "]", "::1",
                "a", "key", "token", "password", "secret", "Bearer ", "Basic ", "Digest ", " ", "\r", "\n", "\r\n", "\t", "\x01",
                "\xFF", "\xC3\xBC", "0", "1.2.3.4", "4111111111111111", "x@y.com", "0123456789abcdef0123456789abcdef", "\"", "'",
                "{", "}", ",", ";", "*", "-", "_", "+", "Cookie: ", "Authorization: ", "HTTP/1.1", "GET ", "Location: "
            };

            const CRedactor paranoid = MakeParanoidRedactor();
            CRedactor small( CRedactor::ForLogLine() );
            small.SetMaxLength( 40 ).SetMarker( "<>" );
            UInt32 state = 12345;
            for ( UInt32 round=0; round<4000; ++round )
            {
                CORE::CString input;
                UInt32 count = NextRandom( state ) % 40;
                for ( UInt32 p=0; p<count; ++p )
                    input += parts[ NextRandom( state ) % ( sizeof( parts ) / sizeof( parts[ 0 ] ) ) ];

                CORE::CString viaUrl = CRedactor::ForUrl().RedactUrl( input );
                ASSERT_TRUE( viaUrl.Length() <= 1024 && IsPlainLine( viaUrl ) );
                CORE::CString viaHeaders = CRedactor::ForHeaders().RedactHeaders( input );
                ASSERT_TRUE( viaHeaders.Length() <= 16384 );
                CORE::CString viaText = CRedactor::ForLogLine().RedactText( input );
                ASSERT_TRUE( Bytes( viaText ) <= 4096 && HasNoControlCharacters( viaText ) );
                CORE::CString viaDisplay = CRedactor::ForDisplay().RedactText( input );
                ASSERT_TRUE( Bytes( viaDisplay ) <= 256 && HasNoControlCharacters( viaDisplay ) );
                CORE::CString viaSmall = small.RedactText( input );
                ASSERT_TRUE( Bytes( viaSmall ) <= 40 );
                CORE::CString viaParanoid = paranoid.RedactText( input );
                ASSERT_TRUE( IsPlainLine( viaParanoid ) );
                paranoid.RedactHeaders( input );
                paranoid.RedactUrl( input );
                CRedactor::ForHeaders().RedactHeaderValue( input, input );

                // only what makes no replacement with another character is the same when redacted twice
                bool replaced = false;
                for ( UInt32 p=0; p<Bytes( input ); ++p )
                {
                    if ( (unsigned char) input[ p ] < 0x20 || (unsigned char) input[ p ] >= 0x7F )
                        replaced = true;
                }
                if ( !replaced && viaUrl.Length() < 1000 )
                    ASSERT_TRUE( SameText( CRedactor::ForUrl().RedactUrl( viaUrl ), viaUrl ) );
                if ( !replaced && Bytes( viaText ) < 4000 )
                    ASSERT_TRUE( SameText( CRedactor::ForLogLine().RedactText( viaText ), viaText ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** Uses the presets from a thread, as every other thread does at the same time */
class CRedactorTestWorker : public MT::CActiveObject
{
    public:

    bool allCorrect;

    CRedactorTestWorker( void )
        : MT::CActiveObject()
        , allCorrect( false )
    {}

    virtual ~CRedactorTestWorker()
    {}

    bool WaitForCompletion( UInt32 timeoutInMs )
    {
        UInt32 waitedInMs = 0;
        while ( IsActive() && waitedInMs < timeoutInMs )
        {
            MT::ThreadDelay( 5 );
            waitedInMs += 5;
        }
        return !IsActive();
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return this;
    }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        return true;
    }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        // a custom redactor which is only read, as the presets are
        CRedactor shared( CRedactor::ForHeaders() );
        shared.AddSensitiveName( "x-my-*" );
        shared.AddQueryName( "code" );

        allCorrect = true;
        for ( UInt32 i=0; i<2000; ++i )
        {
            if ( CRedactor::ForUrl().RedactUrl( "http://u:p@h/p?k=v#f" ) != "http://***@h/p?k=***#***" )
                allCorrect = false;
            if ( CRedactor::ForHeaders().RedactHeaders( "GET /a?k=v HTTP/1.1\r\nCookie: x\r\n" ) != "GET /a?k=*** HTTP/1.1\r\nCookie: ***\r\n" )
                allCorrect = false;
            if ( CRedactor::ForLogLine().RedactText( "password=x Bearer abc" ) != "password=*** Bearer ***" )
                allCorrect = false;
            if ( CRedactor::ForDisplay().RedactUrl( "https://h/p?access_token=T&page=2" ) != "https://h/p?access_token=***&page=2" )
                allCorrect = false;
            if ( shared.RedactHeaders( "GET / HTTP/1.1\r\nX-My-A: 1\r\nX-B: 2\r\n" ) != "GET / HTTP/1.1\r\nX-My-A: ***\r\nX-B: 2\r\n" )
                allCorrect = false;
            if ( !shared.IsSensitiveName( "x-my-c" ) )
                allCorrect = false;
        }
        return true; // done, the thread ends
    }
};

/*-------------------------------------------------------------------------*/

static void
PerformThreadTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Presets and redactors that are not changed are used by several threads at once" )
        try
        {
            static const UInt32 nrOfWorkers = 4;
            CRedactorTestWorker* workers[ nrOfWorkers ];
            for ( UInt32 w=0; w<nrOfWorkers; ++w )
            {
                workers[ w ] = new CRedactorTestWorker();
                ASSERT_TRUE( workers[ w ]->Activate() );
            }
            for ( UInt32 w=0; w<nrOfWorkers; ++w )
            {
                ASSERT_TRUE( workers[ w ]->WaitForCompletion( 60000 ) );
                ASSERT_TRUE( workers[ w ]->allCorrect );
                delete workers[ w ];
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformRedactorTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING redactor TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "Redactor" );

    PerformPresetUrlTests();
    PerformUrlFlagTests();
    PerformMarkerLengthAndSanitationTests();
    PerformHeaderTests();
    PerformTextTests();
    PerformPresetTests();
    PerformSecretCorpusTests();
    PerformIdempotenceTests();
    PerformHostileInputTests();
    PerformThreadTests();
}

/*-------------------------------------------------------------------------*/
