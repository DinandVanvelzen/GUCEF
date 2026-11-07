/*
 *  gucefCOM_TestApp: GUCEF test application for the COM module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COM_CMIMECODEC_H
#include "gucefCOM_CMimeCodec.h"
#define GUCEF_COM_CMIMECODEC_H
#endif /* GUCEF_COM_CMIMECODEC_H ? */

#ifndef GUCEF_COM_CMIMEPARSER_H
#include "gucefCOM_CMimeParser.h"
#define GUCEF_COM_CMIMEPARSER_H
#endif /* GUCEF_COM_CMIMEPARSER_H ? */

#ifndef GUCEF_COM_CMIMECOMPOSER_H
#include "gucefCOM_CMimeComposer.h"
#define GUCEF_COM_CMIMECOMPOSER_H
#endif /* GUCEF_COM_CMIMECOMPOSER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestMime.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::COM;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CString
Repeat( UInt32 count, char c )
{
    if ( 0 == count )
        return CORE::CString();
    GUCEF::vector< char > chars( count, c );
    return CORE::CString( &chars[ 0 ], count );
}

/*-------------------------------------------------------------------------*/

CORE::CString
AsString( const CORE::CDynamicBuffer& buffer )
{
    return buffer.GetDataSize() > 0 ? CORE::CString( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() ) : CORE::CString();
}

/*-------------------------------------------------------------------------*/

bool
ParseText( const CORE::CString& text, CMailMessage& message )
{
    CMimeParser parser;
    return parser.ParseMessage( text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0, message );
}

/*-------------------------------------------------------------------------*/

/** the longest line of the text, lines ended by LF */
UInt32
LongestLine( const CORE::CString& text )
{
    UInt32 longest = 0, current = 0;
    const char* p = text.C_String();
    for ( UInt32 i=0; i<text.ByteSize()-1; ++i )
    {
        if ( '\n' == p[ i ] ) { current = 0; continue; }
        if ( '\r' != p[ i ] && ++current > longest ) longest = current;
    }
    return longest;
}

/*-------------------------------------------------------------------------*/

bool
HasBareLineEnds( const CORE::CDynamicBuffer& buffer )
{
    const char* p = static_cast< const char* >( buffer.GetConstBufferPtr() );
    for ( UInt32 i=0; i<buffer.GetDataSize(); ++i )
    {
        if ( '\n' == p[ i ] && ( 0 == i || '\r' != p[ i - 1 ] ) ) return true;
        if ( '\r' == p[ i ] && ( i + 1 >= buffer.GetDataSize() || '\n' != p[ i + 1 ] ) ) return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
AllAscii( const CORE::CDynamicBuffer& buffer )
{
    return CMimeCodec::IsAscii( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CODEC TESTS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMimeCodecTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "MimeCodec" );

    GUCEF_TESTFW_TESTCASE( "Mime codec: base64 is wrapped at 76 columns and decoded leniently" )
    try
    {
        UInt8 data[ 100 ];
        for ( int i=0; i<100; ++i ) data[ i ] = (UInt8)( i * 7 );
        CORE::CDynamicBuffer wrapped;
        CMimeCodec::AppendBase64Wrapped( wrapped, data, sizeof( data ) );
        CORE::CString text = AsString( wrapped );
        ASSERT_TRUE( LongestLine( text ) == 76 );
        ASSERT_TRUE( text.EndsWith( "\r\n" ) );
        ASSERT_FALSE( HasBareLineEnds( wrapped ) );

        CORE::CDynamicBuffer decoded;
        bool errors = true;
        ASSERT_TRUE( CMimeCodec::DecodeBase64Lenient( text.C_String(), text.ByteSize() - 1, decoded, 1000, &errors ) );
        ASSERT_FALSE( errors );
        ASSERT_TRUE( 100 == decoded.GetDataSize() && 0 == memcmp( decoded.GetConstBufferPtr(), data, 100 ) );

        // white space everywhere, missing padding, bare LF
        CORE::CDynamicBuffer lenient;
        ASSERT_TRUE( CMimeCodec::DecodeBase64Lenient( " aGVs\nbG8g d29y\tbGQ", 20, lenient, 100 ) );
        ASSERT_TRUE( AsString( lenient ) == "hello world" );

        // characters that are not base64 are skipped and reported
        CORE::CDynamicBuffer stray;
        ASSERT_TRUE( CMimeCodec::DecodeBase64Lenient( "aGVs!bG8=", 9, stray, 100, &errors ) );
        ASSERT_TRUE( errors && AsString( stray ) == "hello" );

        // the limit is kept
        CORE::CDynamicBuffer limited;
        ASSERT_FALSE( CMimeCodec::DecodeBase64Lenient( text.C_String(), text.ByteSize() - 1, limited, 10 ) );
        ASSERT_TRUE( limited.GetDataSize() <= 10 );

        // empty
        CORE::CDynamicBuffer empty;
        ASSERT_TRUE( CMimeCodec::DecodeBase64Lenient( "", 0, empty, 10 ) && 0 == empty.GetDataSize() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mime codec: quoted-printable round trip, soft breaks, padding and damaged escapes" )
    try
    {
        const char* source = "Gr\xC3\xBC\xC3\x9F" "e aus K\xC3\xB6ln = 100% \r\nline two with trailing space \r\n.dot\r\n";
        CORE::CDynamicBuffer encoded;
        CMimeCodec::AppendQuotedPrintable( encoded, source, (UInt32) strlen( source ) );
        ASSERT_TRUE( AllAscii( encoded ) );
        CORE::CString text = AsString( encoded );
        ASSERT_TRUE( text.HasSubstr( "=3D" ) >= 0 );
        ASSERT_TRUE( text.HasSubstr( "space=20\r\n" ) >= 0 );
        ASSERT_TRUE( LongestLine( text ) <= 76 );

        CORE::CDynamicBuffer decoded;
        ASSERT_TRUE( CMimeCodec::DecodeQuotedPrintable( text.C_String(), text.ByteSize() - 1, decoded, false, 10000 ) );
        ASSERT_TRUE( AsString( decoded ) == source );

        // a long line is broken with soft line breaks and comes back as one line
        CORE::CString longLine;
        for ( int i=0; i<40; ++i ) longLine += "word" + CORE::Int32ToString( i ) + " ";
        CORE::CDynamicBuffer longEncoded;
        CMimeCodec::AppendQuotedPrintable( longEncoded, longLine.C_String(), longLine.ByteSize() - 1 );
        ASSERT_TRUE( LongestLine( AsString( longEncoded ) ) <= 76 );
        ASSERT_TRUE( AsString( longEncoded ).HasSubstr( "=\r\n" ) >= 0 );
        CORE::CDynamicBuffer longDecoded;
        CMimeCodec::DecodeQuotedPrintable( static_cast< const char* >( longEncoded.GetConstBufferPtr() ), longEncoded.GetDataSize(), longDecoded, false, 100000 );
        ASSERT_TRUE( AsString( longDecoded ) == longLine.Trim( false ) + " " || AsString( longDecoded ) == longLine );

        // lower case hex, soft break with LF only, an escape that is none
        CORE::CDynamicBuffer lower;
        const char* odd = "caf=c3=a9 soft=\nbreak =ZZ and =4";
        CMimeCodec::DecodeQuotedPrintable( odd, (UInt32) strlen( odd ), lower, false, 1000 );
        ASSERT_TRUE( AsString( lower ) == "caf\xC3\xA9 softbreak =ZZ and =4" );

        // Q encoding: underscore is a space
        CORE::CDynamicBuffer q;
        CMimeCodec::DecodeQuotedPrintable( "a_b=5F", 6, q, true, 100 );
        ASSERT_TRUE( AsString( q ) == "a b_" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mime codec: encoded words are made within 75 characters and read back, in B and Q, in several character sets" )
    try
    {
        ASSERT_TRUE( CMimeCodec::EncodeHeaderText( "Plain subject" ) == "Plain subject" );

        CORE::CString text = "Gr\xC3\xBC\xC3\x9F" "e aus K\xC3\xB6ln, und viele weitere Worte damit der Text lang genug wird f\xC3\xBCr mehrere Woerter";
        CORE::CString encoded = CMimeCodec::EncodeHeaderText( text );
        ASSERT_TRUE( CMimeCodec::IsAscii( encoded.C_String(), encoded.ByteSize() - 1 ) );
        CORE::CString::StringVector words = encoded.ParseElements( ' ', false );
        ASSERT_TRUE( words.size() >= 2 );
        for ( size_t i=0; i<words.size(); ++i )
            ASSERT_TRUE( words[ i ].ByteSize() - 1 <= 75 && words[ i ].StartsWith( "=?UTF-8?B?" ) && words[ i ].EndsWith( "?=" ) );
        ASSERT_TRUE( CMimeCodec::DecodeHeaderText( encoded ) == text );

        // Q, Latin-1, RFC 2047 examples
        ASSERT_TRUE( CMimeCodec::DecodeHeaderText( "=?ISO-8859-1?Q?Keld_J=F8rn_Simonsen?=" ) == "Keld J\xC3\xB8rn Simonsen" );
        ASSERT_TRUE( CMimeCodec::DecodeHeaderText( "=?ISO-8859-1?Q?Andr=E9?= Pirard" ) == "Andr\xC3\xA9 Pirard" );
        ASSERT_TRUE( CMimeCodec::DecodeHeaderText( "(=?ISO-8859-1?Q?a?= =?ISO-8859-1?Q?b?=)" ) == "(ab)" );
        ASSERT_TRUE( CMimeCodec::DecodeHeaderText( "=?ISO-8859-1?Q?a?=  =?ISO-8859-1?Q?_b?=" ) == "a b" );
        ASSERT_TRUE( CMimeCodec::DecodeHeaderText( "=?windows-1252?B?gA==?=" ) == "\xE2\x82\xAC" );
        ASSERT_TRUE( CMimeCodec::DecodeHeaderText( "=?utf-8*en?q?hello?=" ) == "hello" );

        // a character split over two words is repaired (the check mark is E2 9C 93)
        ASSERT_TRUE( CMimeCodec::DecodeHeaderText( "=?UTF-8?B?4pw=?= =?UTF-8?B?kw==?=" ) == "\xE2\x9C\x93" );

        // things that look like words and are not stay as they are
        ASSERT_TRUE( CMimeCodec::DecodeHeaderText( "=?broken" ) == "=?broken" );
        ASSERT_TRUE( CMimeCodec::DecodeHeaderText( "=?UTF-8?X?abc?=" ) == "=?UTF-8?X?abc?=" );
        ASSERT_FALSE( CMimeCodec::IsPlainHeaderText( "a =?b?Q?c?= d" ) );

        // control characters, a line break among them, never get through raw
        CORE::CString hostile = CMimeCodec::EncodeHeaderText( "Hi\r\nBcc: evil@example.com\r\n" );
        ASSERT_FALSE( CMimeCodec::HasControlCharacter( hostile ) );
        ASSERT_TRUE( hostile.HasSubstr( "Bcc" ) < 0 );
        ASSERT_TRUE( CMimeCodec::DecodeHeaderText( hostile ) == "Hi\r\nBcc: evil@example.com\r\n" );

        // text that is not valid UTF-8 is made valid before it is encoded
        CORE::CString damaged = CMimeCodec::EncodeHeaderText( "ab\xFF" "cd" );
        ASSERT_TRUE( CMimeCodec::DecodeHeaderText( damaged ) == "ab\xEF\xBF\xBD" "cd" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mime codec: header fields are folded within 78 characters, refuse line breaks and unfold again" )
    try
    {
        CORE::CString value;
        for ( int i=0; i<30; ++i ) value += "token" + CORE::Int32ToString( i ) + " ";
        value = value.Trim( false );
        CORE::CDynamicBuffer out;
        ASSERT_TRUE( CMimeCodec::AppendHeaderField( out, "X-Long", value ) );
        CORE::CString text = AsString( out );
        ASSERT_TRUE( LongestLine( text ) <= 78 );
        ASSERT_TRUE( text.HasSubstr( "\r\n " ) >= 0 );
        ASSERT_TRUE( text.EndsWith( "\r\n" ) );
        ASSERT_FALSE( HasBareLineEnds( out ) );
        CORE::CString body = text.SubstrFromRange( 7, text.Length() );
        ASSERT_TRUE( CMimeCodec::Unfold( body.C_String(), body.ByteSize() - 1 ) == value );

        const UInt32 before = out.GetDataSize();
        ASSERT_FALSE( CMimeCodec::AppendHeaderField( out, "X-Test", "a\r\nBcc: x@example.com" ) );
        ASSERT_FALSE( CMimeCodec::AppendHeaderField( out, "X-Test", "a\nb" ) );
        ASSERT_FALSE( CMimeCodec::AppendHeaderField( out, "X-Test", CORE::CString( "a\0b", 3 ) ) );
        ASSERT_FALSE( CMimeCodec::AppendHeaderField( out, "X Test", "a" ) );
        ASSERT_FALSE( CMimeCodec::AppendHeaderField( out, "X:Test", "a" ) );
        ASSERT_FALSE( CMimeCodec::AppendHeaderField( out, "", "a" ) );
        ASSERT_FALSE( CMimeCodec::AppendHeaderField( out, "X-Test", Repeat( 1500, 'a' ) ) );     // cannot be folded below 998
        ASSERT_TRUE( out.GetDataSize() == before );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mime codec: parameters of RFC 2231 (extended, continued, quoted) and the encoded words some senders put in them" )
    try
    {
        CORE::CString value;
        CMimeCodec::TParamVector params;

        CMimeCodec::ParseValueWithParameters( "text/plain; charset=\"utf-8\"; format=flowed", value, params );
        ASSERT_TRUE( value == "text/plain" && CMimeCodec::FindParameter( params, "charset" ) == "utf-8" && CMimeCodec::FindParameter( params, "format" ) == "flowed" );

        CMimeCodec::ParseValueWithParameters( "attachment; filename*=utf-8''%E2%82%AC%20rates.txt", value, params );
        ASSERT_TRUE( value == "attachment" && CMimeCodec::FindParameter( params, "filename" ) == "\xE2\x82\xAC rates.txt" );

        CMimeCodec::ParseValueWithParameters( "attachment; filename*0*=utf-8''long%20name; filename*1*=%E2%82%AC; filename*2=\".txt\"", value, params );
        ASSERT_TRUE( CMimeCodec::FindParameter( params, "filename" ) == "long name\xE2\x82\xAC.txt" );

        CMimeCodec::ParseValueWithParameters( "attachment; filename*1=b; filename*0=a", value, params );      // sections in any order
        ASSERT_TRUE( CMimeCodec::FindParameter( params, "filename" ) == "ab" );

        CMimeCodec::ParseValueWithParameters( "application/octet-stream; NAME=\"say \\\"hi\\\".bin\"", value, params );
        ASSERT_TRUE( CMimeCodec::FindParameter( params, "name" ) == "say \"hi\".bin" );

        CMimeCodec::ParseValueWithParameters( "attachment; filename=\"=?UTF-8?B?w7xiZXIudHh0?=\"", value, params );
        ASSERT_TRUE( CMimeCodec::FindParameter( params, "filename" ) == "\xC3\xBC" "ber.txt" );

        CMimeCodec::ParseValueWithParameters( "attachment; filename*=iso-8859-1''caf%E9.txt", value, params );
        ASSERT_TRUE( CMimeCodec::FindParameter( params, "filename" ) == "caf\xC3\xA9.txt" );

        CMimeCodec::ParseValueWithParameters( "text/plain; broken; charset=us-ascii;;", value, params );
        ASSERT_TRUE( CMimeCodec::FindParameter( params, "charset" ) == "us-ascii" );

        // what is written is read back, for a token, a text with a space, non ASCII text and a long non ASCII text
        const char* names[] = { "plain.txt", "with space.txt", "k\xC3\xB6ln \xE2\x82\xAC.txt", "" };
        CORE::CString longName;
        for ( int i=0; i<30; ++i ) longName += "\xC3\xBC" "ber\xE2\x82\xAC";
        longName += ".bin";
        for ( int n=0; n<5; ++n )
        {
            CORE::CString name = n < 4 ? CORE::CString( names[ n ] ) : longName;
            if ( name.IsNULLOrEmpty() ) continue;
            CORE::CString written = CMimeCodec::FormatParameter( "filename", name );
            ASSERT_TRUE( CMimeCodec::IsAscii( written.C_String(), written.ByteSize() - 1 ) );
            CMimeCodec::ParseValueWithParameters( "attachment; " + written, value, params );
            ASSERT_TRUE( CMimeCodec::FindParameter( params, "filename" ) == name );
        }
        ASSERT_TRUE( CMimeCodec::FormatParameter( "name", "a.bin" ) == "name=a.bin" );
        ASSERT_TRUE( CMimeCodec::FormatParameter( "name", "a b.bin" ) == "name=\"a b.bin\"" );
        ASSERT_TRUE( CMimeCodec::FormatParameter( "name", "x\r\ny" ).HasSubstr( "\r" ) < 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mime codec: RFC 5322 dates are written and read in the forms mail has" )
    try
    {
        CORE::CDateTime date( 2020, 11, 2, 10, 11, 12, 0, 0 );
        ASSERT_TRUE( CMimeCodec::FormatDate( date ) == "Mon, 02 Nov 2020 10:11:12 +0000" );
        CORE::CDateTime zoned( 1997, 11, 21, 9, 55, 6, 0, -360 );
        ASSERT_TRUE( CMimeCodec::FormatDate( zoned ) == "Fri, 21 Nov 1997 09:55:06 -0600" );
        CORE::CDateTime india( 2024, 2, 29, 23, 59, 59, 0, 330 );
        ASSERT_TRUE( CMimeCodec::FormatDate( india ) == "Thu, 29 Feb 2024 23:59:59 +0530" );

        CORE::CDateTime parsed;
        ASSERT_TRUE( CMimeCodec::ParseDate( "Fri, 21 Nov 1997 09:55:06 -0600", parsed ) );
        ASSERT_TRUE( 1997 == parsed.GetYear() && 11 == parsed.GetMonth() && 21 == parsed.GetDay() && 9 == parsed.GetHours() && 55 == parsed.GetMinutes() && 6 == parsed.GetSeconds() && -360 == parsed.GetTimeZoneUTCOffsetInMins() );
        ASSERT_TRUE( CMimeCodec::ParseDate( "21 Nov 97 09:55 GMT", parsed ) && 1997 == parsed.GetYear() && 0 == parsed.GetSeconds() && 0 == parsed.GetTimeZoneUTCOffsetInMins() );
        ASSERT_TRUE( CMimeCodec::ParseDate( "Tue, 5 Jan 2021 07:08:09 EST (Eastern)", parsed ) && 2021 == parsed.GetYear() && 5 == parsed.GetDay() && -300 == parsed.GetTimeZoneUTCOffsetInMins() );
        ASSERT_TRUE( CMimeCodec::ParseDate( "Wed, 01 Jan 2020 00:00:00 +0000", parsed ) && 2020 == parsed.GetYear() );
        ASSERT_TRUE( CMimeCodec::ParseDate( "1 Jan 05 10:00:00 Z", parsed ) && 2005 == parsed.GetYear() );

        ASSERT_FALSE( CMimeCodec::ParseDate( "", parsed ) );
        ASSERT_FALSE( CMimeCodec::ParseDate( "yesterday", parsed ) );
        ASSERT_FALSE( CMimeCodec::ParseDate( "32 Nov 2020 10:00:00 +0000", parsed ) );
        ASSERT_FALSE( CMimeCodec::ParseDate( "2 Foo 2020 10:00:00 +0000", parsed ) );
        ASSERT_FALSE( CMimeCodec::ParseDate( "2 Nov 2020 25:00:00 +0000", parsed ) );
        ASSERT_FALSE( CMimeCodec::ParseDate( "2 Nov 2020", parsed ) );

        // written and read again
        CORE::CDateTime again;
        ASSERT_TRUE( CMimeCodec::ParseDate( CMimeCodec::FormatDate( india ), again ) && india == again );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mime codec: character sets become UTF-8, invalid bytes become U+FFFD, unknown sets are flagged" )
    try
    {
        bool lossy = false;
        ASSERT_TRUE( CMimeCodec::ToUtf8( "iso-8859-1", "caf\xE9", 4, lossy ) == "caf\xC3\xA9" && !lossy );
        ASSERT_TRUE( CMimeCodec::ToUtf8( "ISO-8859-15", "\xA4", 1, lossy ) == "\xE2\x82\xAC" );
        ASSERT_TRUE( CMimeCodec::ToUtf8( "windows-1252", "\x93q\x94", 3, lossy ) == "\xE2\x80\x9Cq\xE2\x80\x9D" );
        ASSERT_TRUE( CMimeCodec::ToUtf8( "utf-16le", "h\0i\0", 4, lossy ) == "hi" );
        ASSERT_TRUE( CMimeCodec::ToUtf8( "utf-16", "\xFE\xFF\0h\0i", 6, lossy ) == "hi" );
        ASSERT_TRUE( CMimeCodec::ToUtf8( "utf-16be", "\xD8\x3D\xDE\x00", 4, lossy ) == "\xF0\x9F\x98\x80" );
        ASSERT_TRUE( CMimeCodec::ToUtf8( "us-ascii", "a\xE9", 2, lossy ) == "a\xEF\xBF\xBD" && lossy );
        ASSERT_TRUE( CMimeCodec::ToUtf8( "shift_jis", "abc", 3, lossy ) == "abc" && lossy );
        ASSERT_TRUE( CMimeCodec::ToUtf8( "utf-8", "ok\xC0\xAF" "x", 5, lossy ) == "ok\xEF\xBF\xBD\xEF\xBF\xBD" "x" );            // overlong
        ASSERT_TRUE( CMimeCodec::ToUtf8( "utf-8", "\xED\xA0\x80", 3, lossy ) == "\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD" );        // surrogate
        ASSERT_TRUE( CMimeCodec::ToUtf8( "utf-8", "a\xE2\x82", 3, lossy ) == "a\xEF\xBF\xBD\xEF\xBF\xBD" );                    // truncated
        ASSERT_TRUE( CMimeCodec::SanitizeUtf8( "a\0b", 3 ) == "a\xEF\xBF\xBD" "b" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mime codec: message ids and boundaries are unique and well formed" )
    try
    {
        CORE::CString a = CMimeCodec::GenerateMessageId( "example.com" );
        CORE::CString b = CMimeCodec::GenerateMessageId( "example.com" );
        ASSERT_TRUE( a != b && CMimeCodec::IsValidMessageId( a ) && a.EndsWith( "@example.com>" ) );
        ASSERT_TRUE( CMimeCodec::GenerateMessageId( "bad domain\r\n" ).EndsWith( "@localhost.localdomain>" ) );
        ASSERT_TRUE( CMimeCodec::GenerateBoundary() != CMimeCodec::GenerateBoundary() );
        ASSERT_FALSE( CMimeCodec::IsValidMessageId( "<a b@c>" ) );
        ASSERT_FALSE( CMimeCodec::IsValidMessageId( "<a@b>\r\nBcc: x" ) );
        ASSERT_FALSE( CMimeCodec::IsValidMessageId( "a@b" ) );
        ASSERT_TRUE( CMimeCodec::IsValidMessageId( "<abc.def@host.example>" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      PARSER TESTS                                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMimeParserTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "MimeParser" );

    GUCEF_TESTFW_TESTCASE( "Mime parser: a plain message with LF line ends and the usual headers" )
    try
    {
        CMailMessage message;
        ASSERT_TRUE( ParseText(
            "Return-Path: <bounce@example.org>\n"
            "From: Jane Doe <jane@example.org>\n"
            "To: bob@example.com, \"Smith, Alice\" <alice@example.com>\n"
            "Subject: Hello there\n"
            "Date: Tue, 3 Nov 2020 08:30:00 +0100\n"
            "Message-ID: <abc123@example.org>\n"
            "X-Mailer: Test 1.0\n"
            "\n"
            "First line\n"
            "second line\n", message ) );
        ASSERT_TRUE( 1 == message.from.size() && message.from[ 0 ].displayName == "Jane Doe" && message.from[ 0 ].GetAddress() == "jane@example.org" );
        ASSERT_TRUE( 2 == message.to.size() && message.to[ 1 ].displayName == "Smith, Alice" && message.to[ 1 ].GetAddress() == "alice@example.com" );
        ASSERT_TRUE( message.subject == "Hello there" );
        ASSERT_TRUE( message.hasDate && 2020 == message.date.GetYear() && 3 == message.date.GetDay() && 60 == message.date.GetTimeZoneUTCOffsetInMins() );
        ASSERT_TRUE( message.messageId == "<abc123@example.org>" );
        ASSERT_TRUE( message.textBody == "First line\nsecond line" );
        ASSERT_TRUE( message.htmlBody.IsNULLOrEmpty() && message.attachments.empty() );
        ASSERT_TRUE( message.extraHeaders.Get( "X-Mailer" ) == "Test 1.0" && message.extraHeaders.Has( "Return-Path" ) && !message.extraHeaders.Has( "Subject" ) );
        ASSERT_TRUE( message.headers.Get( "subject" ) == "Hello there" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mime parser: multipart/alternative with a quoted-printable text and an HTML part in another character set" )
    try
    {
        CMailMessage message;
        ASSERT_TRUE( ParseText(
            "From: =?UTF-8?Q?Zo=C3=AB_Example?= <zoe@example.org>\r\n"
            "To: bob@example.com\r\n"
            "Subject: =?UTF-8?B?V2Vsa29tIOKAkyBiaWVudmVudWU=?=\r\n"
            "MIME-Version: 1.0\r\n"
            "Content-Type: multipart/alternative;\r\n boundary=\"----=_NextPart_000_0001\"\r\n"
            "\r\n"
            "This is a multi-part message in MIME format.\r\n"
            "------=_NextPart_000_0001\r\n"
            "Content-Type: text/plain; charset=\"utf-8\"\r\n"
            "Content-Transfer-Encoding: quoted-printable\r\n"
            "\r\n"
            "Caf=C3=A9 au lait =E2=82=AC3,50=\r\n"
            " per cup\r\n"
            "------=_NextPart_000_0001\r\n"
            "Content-Type: text/html; charset=\"windows-1252\"\r\n"
            "Content-Transfer-Encoding: base64\r\n"
            "\r\n"
            "PGI+Q2Fm6SBhdSBsYWl0IIAzLDUwPC9iPg==\r\n"
            "------=_NextPart_000_0001--\r\n"
            "epilogue is ignored\r\n", message ) );
        ASSERT_TRUE( message.from[ 0 ].displayName == "Zo\xC3\xAB Example" );
        ASSERT_TRUE( message.subject == "Welkom \xE2\x80\x93 bienvenue" );
        ASSERT_TRUE( message.textBody == "Caf\xC3\xA9 au lait \xE2\x82\xAC" "3,50 per cup" );
        ASSERT_TRUE( message.htmlBody == "<b>Caf\xC3\xA9 au lait \xE2\x82\xAC" "3,50</b>" );
        ASSERT_TRUE( message.attachments.empty() );
        ASSERT_TRUE( message.warnings.empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mime parser: mixed with attachments, file names in three notations, inline image and an attached message" )
    try
    {
        CMailMessage message;
        ASSERT_TRUE( ParseText(
            "From: a@example.org\r\nTo: b@example.org\r\nSubject: files\r\n"
            "Content-Type: multipart/mixed; boundary=outer\r\n\r\n"
            "--outer\r\n"
            "Content-Type: multipart/alternative; boundary=inner\r\n\r\n"
            "--inner\r\nContent-Type: text/plain\r\n\r\nplain text\r\n"
            "--inner\r\nContent-Type: multipart/related; boundary=rel\r\n\r\n"
            "--rel\r\nContent-Type: text/html; charset=us-ascii\r\n\r\n<img src=\"cid:logo@x\">\r\n"
            "--rel\r\nContent-Type: image/png\r\nContent-ID: <logo@x>\r\nContent-Disposition: inline\r\nContent-Transfer-Encoding: base64\r\n\r\niVBORw0K\r\n"
            "--rel--\r\n"
            "--inner--\r\n"
            "--outer\r\n"
            "Content-Type: application/zip; name=\"turn 1.zip\"\r\nContent-Disposition: attachment; filename=\"turn 1.zip\"\r\nContent-Transfer-Encoding: base64\r\n\r\nUEsDBAoAAAAA\r\n"
            "--outer\r\n"
            "Content-Type: application/octet-stream\r\nContent-Disposition: attachment; filename*=utf-8''%E2%82%AC.bin\r\nContent-Transfer-Encoding: base64\r\n\r\nAAEC\r\n"
            "--outer\r\n"
            "Content-Type: application/pdf; name=\"=?UTF-8?B?w7xiZXIucGRm?=\"\r\nContent-Transfer-Encoding: base64\r\n\r\nJVBERg==\r\n"
            "--outer\r\n"
            "Content-Type: message/rfc822\r\n\r\nSubject: inner\r\n\r\ninner body\r\n"
            "--outer--\r\n", message ) );
        ASSERT_TRUE( message.textBody == "plain text" );
        ASSERT_TRUE( message.htmlBody == "<img src=\"cid:logo@x\">" );
        ASSERT_TRUE( 5 == message.attachments.size() );
        ASSERT_TRUE( message.attachments[ 0 ].contentType == "image/png" && message.attachments[ 0 ].contentId == "logo@x" && message.attachments[ 0 ].isInline && 6 == message.attachments[ 0 ].data.GetDataSize() );
        ASSERT_TRUE( message.attachments[ 1 ].fileName == "turn 1.zip" && 9 == message.attachments[ 1 ].data.GetDataSize() && 'P' == message.attachments[ 1 ].data.GetChar( 0 ) );
        ASSERT_TRUE( message.attachments[ 2 ].fileName == "\xE2\x82\xAC.bin" && 3 == message.attachments[ 2 ].data.GetDataSize() && 2 == message.attachments[ 2 ].data.GetChar( 2 ) );
        ASSERT_TRUE( message.attachments[ 3 ].fileName == "\xC3\xBC" "ber.pdf" && message.attachments[ 3 ].contentType == "application/pdf" );
        ASSERT_TRUE( message.attachments[ 4 ].contentType == "message/rfc822" && message.attachments[ 4 ].fileName == "message.eml" );
        ASSERT_TRUE( AsString( message.attachments[ 4 ].data ) == "Subject: inner\r\n\r\ninner body" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mime parser: folded headers, groups, comments, duplicate headers, an mbox line and a header section without body" )
    try
    {
        CMailMessage message;
        ASSERT_TRUE( ParseText(
            "From MAILER-DAEMON Fri Jul  8 12:08:34 2011\n"
            "From: jane@example.org (Jane the Sender)\n"
            "To: Team: a@example.org,\n  b@example.org;\n"
            "Cc: c@example.org,\n\t\"D, E\" <d@example.org>\n"
            "Received: from a by b\n"
            "Received: from c by d\n"
            "References: <one@x>\n <two@x>\n   <three@x>\n"
            "In-Reply-To: <three@x>\n"
            "Subject: a very long subject that goes on\n and continues on the next line\n"
            "No colon on this line\n"
            "\n", message ) );
        ASSERT_TRUE( 1 == message.from.size() && message.from[ 0 ].displayName == "Jane the Sender" );
        ASSERT_TRUE( 2 == message.to.size() && message.to[ 1 ].GetAddress() == "b@example.org" );
        ASSERT_TRUE( 2 == message.cc.size() && message.cc[ 1 ].displayName == "D, E" );
        ASSERT_TRUE( 3 == message.references.size() && message.references[ 2 ] == "<three@x>" && 1 == message.inReplyTo.size() );
        ASSERT_TRUE( message.subject == "a very long subject that goes on and continues on the next line" );
        CORE::CString::StringVector received;
        message.headers.GetAll( "Received", received );
        ASSERT_TRUE( 2 == received.size() );
        ASSERT_FALSE( message.warnings.empty() );

        CMailMessage headersOnly;
        ASSERT_TRUE( ParseText( "Subject: only headers\nFrom: a@example.org", headersOnly ) );
        ASSERT_TRUE( headersOnly.subject == "only headers" && headersOnly.textBody.IsNULLOrEmpty() );

        CMailMessage empty;
        ASSERT_TRUE( ParseText( "", empty ) );
        ASSERT_TRUE( empty.from.empty() && empty.textBody.IsNULLOrEmpty() );

        CMailMessage bodyFirst;
        ASSERT_TRUE( ParseText( "\njust a body\n", bodyFirst ) );
        ASSERT_TRUE( bodyFirst.textBody == "just a body" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mime parser: a multipart without closing boundary, without boundary, with unknown encoding and with a second text part" )
    try
    {
        CMailMessage message;
        ASSERT_TRUE( ParseText(
            "Content-Type: multipart/mixed; boundary=b\n\n--b\nContent-Type: text/plain\n\nbody text\n--b\nContent-Type: application/x-foo; name=f.bin\nContent-Transfer-Encoding: x-uuencode\n\nraw\n", message ) );
        ASSERT_TRUE( message.textBody == "body text" && 1 == message.attachments.size() && message.attachments[ 0 ].fileName == "f.bin" );
        ASSERT_TRUE( AsString( message.attachments[ 0 ].data ) == "raw\n" );
        ASSERT_TRUE( message.warnings.size() >= 2 );

        CMailMessage noBoundary;
        ASSERT_TRUE( ParseText( "Content-Type: multipart/mixed\n\nplain body\n", noBoundary ) );
        ASSERT_TRUE( noBoundary.textBody == "plain body" && !noBoundary.warnings.empty() );

        CMailMessage twoTexts;
        ASSERT_TRUE( ParseText(
            "Content-Type: multipart/mixed; boundary=b\n\n--b\nContent-Type: text/plain\n\nfirst\n--b\nContent-Type: text/plain\n\nsignature\n--b--\n", twoTexts ) );
        ASSERT_TRUE( twoTexts.textBody == "first" && 1 == twoTexts.attachments.size() && twoTexts.attachments[ 0 ].data.GetDataSize() == 9 );

        CMailMessage mac;
        ASSERT_TRUE( ParseText( "Subject: s\rContent-Type: text/plain\r\rold mac body\r", mac ) );
        ASSERT_TRUE( mac.subject == "s" && mac.textBody == "old mac body" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mime parser: hostile input exceeds a limit and fails cleanly, or is read as harmless" )
    try
    {
        CMimeParser parser;
        CMailMessage message;
        CORE::CString error;

        // nested multiparts deeper than the limit
        CORE::CString nested;
        for ( int i=0; i<40; ++i ) nested += "Content-Type: multipart/mixed; boundary=b" + CORE::Int32ToString( i ) + "\n\n--b" + CORE::Int32ToString( i ) + "\n";
        nested += "Content-Type: text/plain\n\nx\n";
        ASSERT_FALSE( parser.ParseMessage( nested.C_String(), nested.ByteSize() - 1, message, &error ) );
        ASSERT_TRUE( error.HasSubstr( "nested" ) >= 0 );

        // a thousand parts
        CORE::CString many = "Content-Type: multipart/mixed; boundary=b\n\n";
        for ( int i=0; i<1000; ++i ) many += "--b\nContent-Type: text/plain\n\nx\n";
        many += "--b--\n";
        ASSERT_FALSE( parser.ParseMessage( many.C_String(), many.ByteSize() - 1, message, &error ) );
        ASSERT_TRUE( error.HasSubstr( "parts" ) >= 0 );

        // a limit given by the caller
        CMimeLimits limits;
        limits.maxMessageSize = 100;
        CMimeParser small( limits );
        CORE::CString big = Repeat( 500, 'a' );
        ASSERT_FALSE( small.ParseMessage( big.C_String(), big.ByteSize() - 1, message, &error ) );

        // too many header fields and a header section that does not end
        CORE::CString fields;
        for ( int i=0; i<1500; ++i ) fields += "X-" + CORE::Int32ToString( i ) + ": v\n";
        ASSERT_FALSE( parser.ParseMessage( fields.C_String(), fields.ByteSize() - 1, message, &error ) );
        CORE::CString endless = Repeat( 400 * 1024, 'x' );
        CORE::CString endlessHeader = "Subject: " + endless;
        ASSERT_FALSE( parser.ParseMessage( endlessHeader.C_String(), endlessHeader.ByteSize() - 1, message, &error ) );

        // a body that decodes to more than the limit
        CMimeLimits decodeLimits;
        decodeLimits.maxDecodedPartSize = 10;
        CMimeParser tight( decodeLimits );
        CORE::CString bomb = "Content-Type: application/octet-stream\nContent-Transfer-Encoding: base64\n\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n";
        ASSERT_FALSE( tight.ParseMessage( bomb.C_String(), bomb.ByteSize() - 1, message, &error ) );

        // binary garbage is not a crash
        CORE::CString garbage;
        for ( int i=0; i<2000; ++i ) garbage += (char)( ( i * 131 + 7 ) % 255 + 1 );
        parser.ParseMessage( garbage.C_String(), garbage.ByteSize() - 1, message, &error );
        CMailMessage nulls;
        ASSERT_TRUE( parser.ParseMessage( "Subject: a\0b\n\nbody\0body\n", 24, nulls, &error ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mime parser: attachment names that try to leave their directory are cleaned for use" )
    try
    {
        ASSERT_TRUE( CMailAttachment::MakeSafeFileName( "../../etc/passwd" ) == "passwd" );
        ASSERT_TRUE( CMailAttachment::MakeSafeFileName( "..\\..\\windows\\system32\\evil.dll" ) == "evil.dll" );
        ASSERT_TRUE( CMailAttachment::MakeSafeFileName( "C:\\x\\y.txt" ) == "y.txt" );
        ASSERT_TRUE( CMailAttachment::MakeSafeFileName( "..." ) == "attachment" );
        ASSERT_TRUE( CMailAttachment::MakeSafeFileName( "" ) == "attachment" );
        ASSERT_TRUE( CMailAttachment::MakeSafeFileName( "a\r\nb:c*d?.txt" ) == "a__b_c_d_.txt" );
        ASSERT_TRUE( CMailAttachment::MakeSafeFileName( ".hidden" ) == "hidden" );
        ASSERT_TRUE( CMailAttachment::MakeSafeFileName( "CON.txt" ) == "_CON.txt" );
        ASSERT_TRUE( CMailAttachment::MakeSafeFileName( "x/", "fallback" ) == "fallback" );
        CORE::CString longName = Repeat( 500, '\xC3' );
        CORE::CString cleaned = CMailAttachment::MakeSafeFileName( longName );
        ASSERT_TRUE( cleaned.ByteSize() - 1 <= 200 );
        CMailAttachment attachment;
        attachment.fileName = "../x.zip";
        ASSERT_TRUE( attachment.GetSafeFileName() == "x.zip" && attachment.fileName == "../x.zip" );

        CMailMessage message;
        ASSERT_TRUE( ParseText( "Content-Type: application/zip; name=\"../../../.ssh/authorized_keys\"\nContent-Transfer-Encoding: base64\n\nAAEC\n", message ) );
        ASSERT_TRUE( 1 == message.attachments.size() && message.attachments[ 0 ].GetSafeFileName() == "authorized_keys" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      COMPOSER TESTS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CMailMessage
MakeFullMessage( void )
{
    CMailMessage message;
    message.from.push_back( CMailAddress( "Zo\xC3\xAB \"The Sender\" Example", "zoe@example.org" ) );
    message.to.push_back( CMailAddress( "Bob, Jr.", "bob@example.com" ) );
    message.to.push_back( CMailAddress( "alice@example.com" ) );
    message.cc.push_back( CMailAddress( "carol@example.com" ) );
    message.bcc.push_back( CMailAddress( "secret@example.com" ) );
    message.replyTo.push_back( CMailAddress( "replies@example.org" ) );
    message.subject = "Turn 7 \xE2\x80\x93 Gr\xC3\xBC\xC3\x9F" "e";
    message.date = CORE::CDateTime( 2020, 11, 2, 10, 11, 12, 0, 60 );
    message.hasDate = true;
    message.messageId = "<turn7.abc@example.org>";
    message.inReplyTo.push_back( "<turn6.abc@example.org>" );
    message.references.push_back( "<turn5.abc@example.org>" );
    message.references.push_back( "<turn6.abc@example.org>" );
    message.textBody = "Hello Bob,\nthe turn file is attached.\n\n-- \nZo\xC3\xAB \xE2\x82\xAC";
    message.htmlBody = "<html><body><p>Hello <b>Bob</b></p><img src=\"cid:logo@example.org\"></body></html>";
    message.extraHeaders.Add( "X-Redux-Game", "g-1234" );
    message.extraHeaders.AddText( "X-Redux-Title", "Wunderschöne Reise" );

    UInt8 all[ 256 ];
    for ( int i=0; i<256; ++i ) all[ i ] = (UInt8) i;
    CMailAttachment turn( "turn-7.rdx", "application/octet-stream", all, sizeof( all ) );
    message.attachments.push_back( turn );
    CMailAttachment unicodeName( "k\xC3\xB6ln \xE2\x82\xAC rates.txt", "text/plain", "rates\r\n", 7 );
    message.attachments.push_back( unicodeName );
    CMailAttachment logo( "logo.png", "image/png", "\x89PNG\r\n\x1a\n", 8 );
    logo.isInline = true;
    logo.contentId = "logo@example.org";
    message.attachments.push_back( logo );
    return message;
}

}

/*-------------------------------------------------------------------------*/

void
PerformMimeComposerTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "MimeComposer" );

    GUCEF_TESTFW_TESTCASE( "Mime composer: a full message is 7 bit clean with CR LF line ends and short lines, and reads back as written" )
    try
    {
        CMailMessage message = MakeFullMessage();
        CMimeComposer composer;
        CORE::CDynamicBuffer wire;
        CORE::CString error;
        ASSERT_TRUE( composer.Compose( message, wire, error ) );
        ASSERT_TRUE( AllAscii( wire ) );
        ASSERT_FALSE( HasBareLineEnds( wire ) );
        CORE::CString text = AsString( wire );
        ASSERT_TRUE( LongestLine( text ) <= 998 );
        ASSERT_TRUE( text.HasSubstr( "secret@example.com" ) < 0 );     // Bcc stays out of the headers
        ASSERT_TRUE( text.HasSubstr( "Bcc" ) < 0 );

        CMimeParser parser;
        CMailMessage parsed;
        ASSERT_TRUE( parser.ParseMessage( wire.GetConstBufferPtr(), wire.GetDataSize(), parsed, &error ) );
        ASSERT_TRUE( parsed.warnings.empty() );
        ASSERT_TRUE( parsed.from.size() == 1 && parsed.from[ 0 ].displayName == message.from[ 0 ].displayName && parsed.from[ 0 ].IsSameAddress( message.from[ 0 ] ) );
        ASSERT_TRUE( parsed.to.size() == 2 && parsed.to[ 0 ].displayName == "Bob, Jr." && parsed.to[ 1 ].GetAddress() == "alice@example.com" );
        ASSERT_TRUE( parsed.cc.size() == 1 && parsed.bcc.empty() && parsed.replyTo.size() == 1 );
        ASSERT_TRUE( parsed.subject == message.subject );
        ASSERT_TRUE( parsed.hasDate && parsed.date == message.date );
        ASSERT_TRUE( parsed.messageId == message.messageId && parsed.inReplyTo == message.inReplyTo && parsed.references == message.references );
        ASSERT_TRUE( parsed.textBody == message.textBody );
        ASSERT_TRUE( parsed.htmlBody == message.htmlBody );
        ASSERT_TRUE( parsed.extraHeaders.Get( "X-Redux-Game" ) == "g-1234" );
        ASSERT_TRUE( parsed.extraHeaders.GetDecoded( "X-Redux-Title" ) == "Wunderschöne Reise" );
        ASSERT_TRUE( 3 == parsed.attachments.size() );

        // the inline image comes first (related), then the regular attachments
        const CMailAttachment* turn = GUCEF_NULL;
        const CMailAttachment* rates = GUCEF_NULL;
        const CMailAttachment* logo = GUCEF_NULL;
        for ( size_t i=0; i<parsed.attachments.size(); ++i )
        {
            if ( parsed.attachments[ i ].fileName == "turn-7.rdx" ) turn = &parsed.attachments[ i ];
            if ( parsed.attachments[ i ].fileName == "k\xC3\xB6ln \xE2\x82\xAC rates.txt" ) rates = &parsed.attachments[ i ];
            if ( parsed.attachments[ i ].fileName == "logo.png" ) logo = &parsed.attachments[ i ];
        }
        ASSERT_TRUE( GUCEF_NULL != turn && 256 == turn->data.GetDataSize() && turn->data == message.attachments[ 0 ].data );
        ASSERT_TRUE( GUCEF_NULL != rates && AsString( rates->data ) == "rates\r\n" && rates->contentType == "text/plain" );
        ASSERT_TRUE( GUCEF_NULL != logo && logo->contentId == "logo@example.org" && logo->isInline && 8 == logo->data.GetDataSize() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mime composer: the structure follows the content" )
    try
    {
        CMimeComposer composer;
        CMimeParser parser;
        CORE::CString error;

        CMailMessage textOnly;
        textOnly.from.push_back( CMailAddress( "a@example.org" ) );
        textOnly.to.push_back( CMailAddress( "b@example.org" ) );
        textOnly.subject = "plain";
        textOnly.textBody = "just text";
        CORE::CDynamicBuffer wire;
        ASSERT_TRUE( composer.Compose( textOnly, wire, error ) );
        CMimePart root;
        ASSERT_TRUE( parser.ParseParts( wire.GetConstBufferPtr(), wire.GetDataSize(), root ) );
        ASSERT_TRUE( root.contentType == "text/plain" && root.transferEncoding == "7bit" && root.GetCharset() == "utf-8" );

        CMailMessage alternative = textOnly;
        alternative.htmlBody = "<p>just text</p>";
        wire.Clear();
        ASSERT_TRUE( composer.Compose( alternative, wire, error ) );
        ASSERT_TRUE( parser.ParseParts( wire.GetConstBufferPtr(), wire.GetDataSize(), root ) );
        ASSERT_TRUE( root.contentType == "multipart/alternative" && 2 == root.children.size() && root.children[ 0 ]->contentType == "text/plain" && root.children[ 1 ]->contentType == "text/html" );

        CMailMessage htmlOnly = textOnly;
        htmlOnly.textBody.Clear();
        htmlOnly.htmlBody = "<p>only html</p>";
        wire.Clear();
        ASSERT_TRUE( composer.Compose( htmlOnly, wire, error ) );
        ASSERT_TRUE( parser.ParseParts( wire.GetConstBufferPtr(), wire.GetDataSize(), root ) );
        ASSERT_TRUE( root.contentType == "text/html" );

        CMailMessage mixed = alternative;
        mixed.attachments.push_back( CMailAttachment( "a.bin", "application/octet-stream", "abc", 3 ) );
        wire.Clear();
        ASSERT_TRUE( composer.Compose( mixed, wire, error ) );
        ASSERT_TRUE( parser.ParseParts( wire.GetConstBufferPtr(), wire.GetDataSize(), root ) );
        ASSERT_TRUE( root.contentType == "multipart/mixed" && 2 == root.children.size() && root.children[ 0 ]->contentType == "multipart/alternative" && root.children[ 1 ]->contentType == "application/octet-stream" );

        // text with non ASCII becomes quoted-printable, mostly non ASCII text base64
        CMailMessage accents = textOnly;
        accents.textBody = "caf\xC3\xA9 au lait with mostly ascii text around it";
        wire.Clear();
        ASSERT_TRUE( composer.Compose( accents, wire, error ) );
        ASSERT_TRUE( parser.ParseParts( wire.GetConstBufferPtr(), wire.GetDataSize(), root ) && root.transferEncoding == "quoted-printable" );
        accents.textBody = "\xE2\x82\xAC\xE2\x82\xAC\xE2\x82\xAC\xE2\x82\xAC";
        wire.Clear();
        ASSERT_TRUE( composer.Compose( accents, wire, error ) );
        ASSERT_TRUE( parser.ParseParts( wire.GetConstBufferPtr(), wire.GetDataSize(), root ) && root.transferEncoding == "base64" );

        // bare line ends in the text are written as CR LF, a very long line is broken
        CMailMessage ends = textOnly;
        ends.textBody = "one\ntwo\rthree\r\nfour " + Repeat( 3000, 'x' );
        wire.Clear();
        ASSERT_TRUE( composer.Compose( ends, wire, error ) );
        ASSERT_FALSE( HasBareLineEnds( wire ) );
        ASSERT_TRUE( LongestLine( AsString( wire ) ) <= 998 );
        CMailMessage endsParsed;
        ASSERT_TRUE( parser.ParseMessage( wire.GetConstBufferPtr(), wire.GetDataSize(), endsParsed ) );
        ASSERT_TRUE( endsParsed.textBody == "one\ntwo\nthree\nfour " + Repeat( 3000, 'x' ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mime composer: long recipient lists and subjects are folded, and the boundary is not found in the text" )
    try
    {
        CMailMessage message;
        message.from.push_back( CMailAddress( "a@example.org" ) );
        for ( int i=0; i<60; ++i )
            message.to.push_back( CMailAddress( "Recipient Number " + CORE::Int32ToString( i ), "recipient" + CORE::Int32ToString( i ) + "@example.com" ) );
        for ( int i=0; i<40; ++i )
            message.subject += "word\xC3\xA9 ";
        message.textBody = "--gucefmime-0000\n--not a boundary\nfine";
        CMimeComposer composer;
        CORE::CDynamicBuffer wire;
        CORE::CString error;
        ASSERT_TRUE( composer.Compose( message, wire, error ) );
        CMimeParser parser;
        CMailMessage parsed;
        ASSERT_TRUE( parser.ParseMessage( wire.GetConstBufferPtr(), wire.GetDataSize(), parsed ) );
        ASSERT_TRUE( 60 == parsed.to.size() && parsed.to[ 59 ].displayName == "Recipient Number 59" );
        ASSERT_TRUE( parsed.subject == message.subject );
        ASSERT_TRUE( parsed.textBody == message.textBody );

        // headers within 78 characters where they can be folded
        CORE::CString text = AsString( wire );
        Int32 end = text.HasSubstr( "\r\n\r\n" );
        ASSERT_TRUE( end > 0 );
        ASSERT_TRUE( LongestLine( text.SubstrToIndex( (UInt32) end, true ) ) <= 78 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mime composer: line breaks in addresses, ids, header names and values are refused, in free text they are encoded" )
    try
    {
        CMimeComposer composer;
        CMimeParser parser;
        CORE::CString error;
        CORE::CDynamicBuffer wire;

        CMailMessage base;
        base.from.push_back( CMailAddress( "a@example.org" ) );
        base.to.push_back( CMailAddress( "b@example.org" ) );
        base.textBody = "x";

        // subject and display name: encoded, nothing leaks into the header section
        CMailMessage subject = base;
        subject.subject = "Hi\r\nBcc: evil@example.com\r\n\r\nInjected body";
        subject.from[ 0 ].displayName = "Eve\r\nBcc: evil2@example.com";
        ASSERT_TRUE( composer.Compose( subject, wire, error ) );
        CMailMessage parsed;
        ASSERT_TRUE( parser.ParseMessage( wire.GetConstBufferPtr(), wire.GetDataSize(), parsed ) );
        ASSERT_FALSE( parsed.headers.Has( "Bcc" ) );
        ASSERT_TRUE( parsed.bcc.empty() && parsed.textBody == "x" );
        ASSERT_TRUE( AsString( wire ).HasSubstr( "evil" ) < 0 );
        ASSERT_TRUE( parsed.subject == subject.subject );

        const UInt32 size = wire.GetDataSize();
        CMailMessage bad = base;
        bad.to.push_back( CMailAddress( CORE::CString( "x@example.org\r\nBcc: evil@example.com" ) ) );
        ASSERT_TRUE( bad.to.size() == 1 || bad.to[ 1 ].IsEmpty() || !bad.to[ 1 ].IsValid() );
        ASSERT_FALSE( composer.Compose( bad, wire, error ) );

        bad = base;
        CMailAddress sneaky;
        sneaky.localPart = "x";
        sneaky.domain = "example.org>\r\nBcc: evil@example.com";
        bad.to.push_back( sneaky );
        ASSERT_FALSE( composer.Compose( bad, wire, error ) );
        sneaky.domain = "example.org";
        sneaky.localPart = "x\r\nBcc: evil";
        bad.to[ 1 ] = sneaky;
        ASSERT_FALSE( composer.Compose( bad, wire, error ) );

        bad = base;
        bad.messageId = "<a@b>\r\nBcc: evil@example.com";
        ASSERT_FALSE( composer.Compose( bad, wire, error ) );
        bad = base;
        bad.references.push_back( "<a@b>\nX: y" );
        ASSERT_FALSE( composer.Compose( bad, wire, error ) );

        bad = base;
        ASSERT_FALSE( bad.extraHeaders.Add( "X-A", "v\r\nBcc: evil@example.com" ) );
        ASSERT_FALSE( bad.extraHeaders.Add( "X-A\r\nBcc", "v" ) );
        ASSERT_FALSE( bad.extraHeaders.Add( "X-A: b", "v" ) );
        ASSERT_TRUE( bad.extraHeaders.AddText( "X-A", "v\r\nBcc: evil@example.com" ) );
        ASSERT_TRUE( composer.Compose( bad, wire, error ) );
        ASSERT_TRUE( AsString( wire ).HasSubstr( "evil" ) < 0 );
        bad = base;
        bad.extraHeaders.Add( "Bcc", "evil@example.com" );
        ASSERT_FALSE( composer.Compose( bad, wire, error ) );
        bad = base;
        bad.extraHeaders.Add( "Content-Type", "text/html" );
        ASSERT_FALSE( composer.Compose( bad, wire, error ) );

        bad = base;
        bad.attachments.push_back( CMailAttachment( "a.txt\r\nContent-Type: x", "text/plain", "x", 1 ) );
        ASSERT_FALSE( composer.Compose( bad, wire, error ) );
        bad = base;
        bad.attachments.push_back( CMailAttachment( "a.txt", "text/plain\r\nX: y", "x", 1 ) );
        ASSERT_FALSE( composer.Compose( bad, wire, error ) );
        bad = base;
        bad.attachments.push_back( CMailAttachment( "a.txt", "text/plain", "x", 1 ) );
        bad.attachments[ 0 ].contentId = "id>\r\nX: y";
        ASSERT_FALSE( composer.Compose( bad, wire, error ) );

        // refused messages leave the output as it was
        ASSERT_TRUE( wire.GetDataSize() > 0 );
        CORE::CDynamicBuffer untouched;
        ASSERT_FALSE( composer.Compose( bad, untouched, error ) );
        ASSERT_TRUE( 0 == untouched.GetDataSize() && !error.IsNULLOrEmpty() );
        ASSERT_TRUE( size > 0 );

        // the reason does not repeat the content
        ASSERT_TRUE( error.HasSubstr( "evil" ) < 0 );

        // too large
        CMimeComposeOptions options;
        options.maxSize = 500;
        CMimeComposer small( options );
        CMailMessage large = base;
        large.textBody = Repeat( 5000, 'q' );
        ASSERT_FALSE( small.Compose( large, untouched, error ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mime composer: international addresses need SMTPUTF8, a non ASCII subject does not" )
    try
    {
        CMailMessage message;
        message.from.push_back( CMailAddress( "a@example.org" ) );
        message.to.push_back( CMailAddress( "j\xC3\xBC" "rgen@example.org" ) );
        message.textBody = "x";
        CMimeComposer composer;
        CORE::CDynamicBuffer wire;
        CORE::CString error;
        ASSERT_FALSE( composer.Compose( message, wire, error ) );

        CMimeComposeOptions options;
        options.allowUtf8Addresses = true;
        CMimeComposer utf8( options );
        ASSERT_TRUE( utf8.Compose( message, wire, error ) );
        CMimeParser parser;
        CMailMessage parsed;
        ASSERT_TRUE( parser.ParseMessage( wire.GetConstBufferPtr(), wire.GetDataSize(), parsed ) );
        ASSERT_TRUE( 1 == parsed.to.size() && parsed.to[ 0 ].localPart == "j\xC3\xBC" "rgen" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      MODEL TESTS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMailModelTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "MailModel" );

    GUCEF_TESTFW_TESTCASE( "Mail model: addresses are parsed in the forms mail has, validated and written" )
    try
    {
        CMailAddress address;
        ASSERT_TRUE( CMailAddress::Parse( "Jane Doe <jane@example.org>", address ) && address.displayName == "Jane Doe" && address.localPart == "jane" && address.domain == "example.org" );
        ASSERT_TRUE( CMailAddress::Parse( "jane@example.org", address ) && address.displayName.IsNULLOrEmpty() );
        ASSERT_TRUE( CMailAddress::Parse( "<jane@example.org>", address ) && address.GetAddress() == "jane@example.org" );
        ASSERT_TRUE( CMailAddress::Parse( "\"Doe, Jane\" <jane@example.org>", address ) && address.displayName == "Doe, Jane" );
        ASSERT_TRUE( CMailAddress::Parse( "=?UTF-8?B?SsO8cmdlbg==?= <j@example.org>", address ) && address.displayName == "J\xC3\xBC" "rgen" );
        ASSERT_TRUE( CMailAddress::Parse( "\"odd local\"@example.org", address ) && address.localPart == "odd local" && address.GetAddress() == "\"odd local\"@example.org" );
        ASSERT_TRUE( CMailAddress::Parse( "a.b+tag@sub.example.org (A B)", address ) && address.displayName == "A B" );
        ASSERT_TRUE( CMailAddress::Parse( "x@[192.0.2.1]", address ) && address.IsValid() );
        ASSERT_TRUE( CMailAddress::Parse( "<@relay.example,@other.example:user@example.org>", address ) && address.GetAddress() == "user@example.org" );
        ASSERT_FALSE( CMailAddress::Parse( "no address here", address ) );
        ASSERT_FALSE( CMailAddress::Parse( "@example.org", address ) );
        ASSERT_FALSE( CMailAddress::Parse( "user@", address ) );
        ASSERT_FALSE( CMailAddress::Parse( "a b@example.org", address ) );
        ASSERT_FALSE( CMailAddress::Parse( "", address ) );

        CMailAddress::TMailAddressVector list;
        ASSERT_TRUE( 4 == CMailAddress::ParseList( "a@x.org, garbage, Team: b@x.org, c@x.org;, d@x.org", list ) );
        list.clear();
        ASSERT_TRUE( 2 == CMailAddress::ParseList( "a@x.org, b@x.org, c@x.org", list, 2 ) );

        // validity
        ASSERT_TRUE( CMailAddress( "a@example.org" ).IsValid() );
        ASSERT_FALSE( CMailAddress( "a@exa mple.org" ).IsValid() );
        ASSERT_FALSE( CMailAddress( "a@-example.org" ).IsValid() );
        ASSERT_FALSE( CMailAddress( "a@example..org" ).IsValid() );
        ASSERT_FALSE( CMailAddress( "a@example.org." ).IsValid() );
        CMailAddress tooLong;
        tooLong.localPart = Repeat( 65, 'a' );
        tooLong.domain = "example.org";
        ASSERT_FALSE( tooLong.IsValid() );
        CMailAddress control;
        control.localPart = "a\x01";
        control.domain = "example.org";
        ASSERT_FALSE( control.IsValid() );
        ASSERT_TRUE( CMailAddress( "j\xC3\xBC@example.org" ).IsInternational() && !CMailAddress( "j\xC3\xBC@example.org" ).IsValid() && CMailAddress( "j\xC3\xBC@example.org" ).IsValid( true ) );

        // written
        CORE::CString text;
        ASSERT_TRUE( CMailAddress( "Jane Doe", "jane@example.org" ).Format( text ) && text == "Jane Doe <jane@example.org>" );
        ASSERT_TRUE( CMailAddress( "Doe, Jane", "jane@example.org" ).Format( text ) && text == "\"Doe, Jane\" <jane@example.org>" );
        ASSERT_TRUE( CMailAddress( "J\xC3\xBC" "rgen", "j@example.org" ).Format( text ) && text == "=?UTF-8?B?SsO8cmdlbg==?= <j@example.org>" );
        ASSERT_TRUE( CMailAddress( "jane@example.org" ).Format( text ) && text == "jane@example.org" );
        ASSERT_FALSE( tooLong.Format( text ) );
        ASSERT_TRUE( text.IsNULLOrEmpty() );
        ASSERT_TRUE( CMailAddress( "A", "a@x.org" ).IsSameAddress( CMailAddress( "B", "a@X.ORG" ) ) );
        ASSERT_FALSE( CMailAddress( "a@x.org" ).IsSameAddress( CMailAddress( "A@x.org" ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mail model: headers keep their order, look up without regard for case and refuse what could break out" )
    try
    {
        CMailHeaders headers;
        ASSERT_TRUE( headers.Add( "Received", "from a" ) );
        ASSERT_TRUE( headers.Add( "X-One", "1" ) );
        ASSERT_TRUE( headers.Add( "received", "from b" ) );
        ASSERT_TRUE( 3 == headers.GetCount() && headers.GetField( 1 ).name == "X-One" );
        ASSERT_TRUE( headers.Has( "RECEIVED" ) && headers.Get( "received" ) == "from a" );
        CORE::CString::StringVector all;
        headers.GetAll( "Received", all );
        ASSERT_TRUE( 2 == all.size() && all[ 1 ] == "from b" );
        ASSERT_TRUE( headers.Set( "X-One", "uno" ) && headers.Get( "x-one" ) == "uno" && 3 == headers.GetCount() );
        ASSERT_TRUE( 2 == headers.Remove( "RECEIVED" ) && 1 == headers.GetCount() );
        ASSERT_FALSE( headers.Add( "", "v" ) );
        ASSERT_FALSE( headers.Add( "A B", "v" ) );
        ASSERT_FALSE( headers.Add( "A", "v\nw" ) );
        ASSERT_FALSE( headers.Set( "X-One", "bad\r\n" ) );
        ASSERT_TRUE( headers.Get( "X-One" ) == "uno" );
        ASSERT_TRUE( headers.AddText( "X-Text", "caf\xC3\xA9" ) && headers.GetDecoded( "X-Text" ) == "caf\xC3\xA9" && headers.Get( "X-Text" ).StartsWith( "=?UTF-8?B?" ) );

        CORE::CDynamicBuffer out;
        ASSERT_TRUE( headers.AppendTo( out ) );
        ASSERT_TRUE( AsString( out ).HasSubstr( "X-One: uno\r\n" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mail model: a message is validated and cloned, the envelope recipients have no doubles" )
    try
    {
        CMailMessage message = MakeFullMessage();
        CORE::CString reason;
        ASSERT_TRUE( message.Validate( reason ) );

        CMailAddress::TMailAddressVector recipients;
        message.to.push_back( CMailAddress( "SECRET@example.com" ) );
        message.GetEnvelopeRecipients( recipients );
        ASSERT_TRUE( 5 == recipients.size() && recipients[ 4 ].GetAddress() == "secret@example.com" );

        CMailMessage noFrom = message;
        noFrom.from.clear();
        ASSERT_FALSE( noFrom.Validate( reason ) );
        CMailMessage noTo = message;
        noTo.to.clear(); noTo.cc.clear(); noTo.bcc.clear();
        ASSERT_FALSE( noTo.Validate( reason ) );
        CMailMessage badId = message;
        badId.messageId = "no brackets";
        ASSERT_FALSE( badId.Validate( reason ) );
        CMailMessage badType = message;
        badType.attachments[ 0 ].contentType = "nonsense";
        ASSERT_FALSE( badType.Validate( reason ) );

        CORE::CICloneable* clone = message.Clone();
        CMailMessage* copy = CORE::CICloneable::CloneableAs< CMailMessage >( clone );
        ASSERT_TRUE( GUCEF_NULL != copy && copy->subject == message.subject && 3 == copy->attachments.size() && copy->attachments[ 0 ].data == message.attachments[ 0 ].data );
        delete clone;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
