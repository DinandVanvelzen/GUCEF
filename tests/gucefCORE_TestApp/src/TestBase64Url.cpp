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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAllocationCounter.h"
#include "TestBase64Url.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

static bool
EncodesTo( const char* text, const char* expectedUnpadded, const char* expectedPadded )
{GUCEF_TRACE;

    CORE::CString unpadded;
    CORE::CString padded;
    CORE::CString defaulted;
    bool ok = CORE::Base64UrlEncode( text, (UInt32) strlen( text ), unpadded, false );
    ok = CORE::Base64UrlEncode( text, (UInt32) strlen( text ), padded, true ) && ok;
    ok = CORE::Base64UrlEncode( text, (UInt32) strlen( text ), defaulted ) && ok;
    if ( !ok )
        return false;
    const bool unpaddedMatches = 0 == strlen( expectedUnpadded ) ? unpadded.IsNULLOrEmpty() : expectedUnpadded == unpadded;
    const bool paddedMatches = 0 == strlen( expectedPadded ) ? padded.IsNULLOrEmpty() : expectedPadded == padded;
    return unpaddedMatches && paddedMatches && unpadded == defaulted;
}

/*-------------------------------------------------------------------------*/

static bool
DecodesTo( const char* encoded, const char* expectedText )
{GUCEF_TRACE;

    CORE::CDynamicBuffer result;
    if ( !CORE::Base64UrlDecode( encoded, result ) )
        return false;
    UInt32 expectedSize = (UInt32) strlen( expectedText );
    return expectedSize == result.GetDataSize() && 0 == memcmp( expectedText, result.GetConstBufferPtr(), expectedSize );
}

/*-------------------------------------------------------------------------*/

static bool
IsRefused( const char* encoded )
{GUCEF_TRACE;

    // an earlier result must not survive a failed decode
    CORE::CDynamicBuffer result;
    result.Append( "left over", 9, true );
    bool decoded = CORE::Base64UrlDecode( encoded, result );
    return !decoded && 0 == result.GetDataSize();
}

/*-------------------------------------------------------------------------*/

void
PerformBase64UrlTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Base64Url TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "Base64Url" );

    GUCEF_TESTFW_TESTCASE( "Test: The RFC 4648 section 10 vectors encode with and without padding and decode with and without padding" )
        try
        {
            ASSERT_TRUE( EncodesTo( "", "", "" ) );
            ASSERT_TRUE( EncodesTo( "f", "Zg", "Zg==" ) );
            ASSERT_TRUE( EncodesTo( "fo", "Zm8", "Zm8=" ) );
            ASSERT_TRUE( EncodesTo( "foo", "Zm9v", "Zm9v" ) );
            ASSERT_TRUE( EncodesTo( "foob", "Zm9vYg", "Zm9vYg==" ) );
            ASSERT_TRUE( EncodesTo( "fooba", "Zm9vYmE", "Zm9vYmE=" ) );
            ASSERT_TRUE( EncodesTo( "foobar", "Zm9vYmFy", "Zm9vYmFy" ) );

            ASSERT_TRUE( DecodesTo( "", "" ) );
            ASSERT_TRUE( DecodesTo( "Zg", "f" ) );
            ASSERT_TRUE( DecodesTo( "Zg==", "f" ) );
            ASSERT_TRUE( DecodesTo( "Zm8", "fo" ) );
            ASSERT_TRUE( DecodesTo( "Zm8=", "fo" ) );
            ASSERT_TRUE( DecodesTo( "Zm9v", "foo" ) );
            ASSERT_TRUE( DecodesTo( "Zm9vYg", "foob" ) );
            ASSERT_TRUE( DecodesTo( "Zm9vYg==", "foob" ) );
            ASSERT_TRUE( DecodesTo( "Zm9vYmE", "fooba" ) );
            ASSERT_TRUE( DecodesTo( "Zm9vYmE=", "fooba" ) );
            ASSERT_TRUE( DecodesTo( "Zm9vYmFy", "foobar" ) );

            // an empty decode leaves an empty result, even for a buffer that had data
            CORE::CDynamicBuffer result;
            result.Append( "x", 1, true );
            ASSERT_TRUE( CORE::Base64UrlDecode( CORE::CString::Empty, result ) );
            ASSERT_TRUE( 0 == result.GetDataSize() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The alphabet is - and _ instead of + and /, for bytes that need them" )
        try
        {
            // 0xFB 0xEF 0xBE is "++++" in base64, 0xFF 0xFF 0xFF is "////", 0xFB 0xFF is "+/8="
            const UInt8 plus[ 3 ] = { 0xFB, 0xEF, 0xBE };
            const UInt8 slash[ 3 ] = { 0xFF, 0xFF, 0xFF };
            const UInt8 mixed[ 2 ] = { 0xFB, 0xFF };
            CORE::CString text;
            ASSERT_TRUE( CORE::Base64UrlEncode( plus, 3, text ) && "----" == text );
            ASSERT_TRUE( "++++" == CORE::Base64Encode( plus, 3 ) );
            ASSERT_TRUE( CORE::Base64UrlEncode( slash, 3, text ) && "____" == text );
            ASSERT_TRUE( CORE::Base64UrlEncode( mixed, 2, text ) && "-_8" == text );
            ASSERT_TRUE( CORE::Base64UrlEncode( mixed, 2, text, true ) && "-_8=" == text );

            CORE::CDynamicBuffer result;
            ASSERT_TRUE( CORE::Base64UrlDecode( "----", result ) && 3 == result.GetDataSize() && 0 == memcmp( plus, result.GetConstBufferPtr(), 3 ) );
            ASSERT_TRUE( CORE::Base64UrlDecode( "____", result ) && 3 == result.GetDataSize() && 0 == memcmp( slash, result.GetConstBufferPtr(), 3 ) );
            ASSERT_TRUE( CORE::Base64UrlDecode( "-_8", result ) && 2 == result.GetDataSize() && 0 == memcmp( mixed, result.GetConstBufferPtr(), 2 ) );
            ASSERT_TRUE( CORE::Base64UrlDecode( "-_8=", result ) && 2 == result.GetDataSize() && 0 == memcmp( mixed, result.GetConstBufferPtr(), 2 ) );

            // the standard encoder is not changed
            ASSERT_TRUE( "Zg==" == CORE::Base64Encode( "f", 1 ) );
            ASSERT_TRUE( "Zm9vYmE=" == CORE::Base64Encode( "fooba", 5 ) );
            ASSERT_TRUE( CORE::Base64Encode( "", 0 ).IsNULLOrEmpty() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: JWT style input: the header and payload of RFC 7515 and a JWT header decode and re-encode" )
        try
        {
            // RFC 7515 appendix A.2
            ASSERT_TRUE( DecodesTo( "eyJhbGciOiJSUzI1NiJ9", "{\"alg\":\"RS256\"}" ) );
            ASSERT_TRUE( EncodesTo( "{\"alg\":\"RS256\"}", "eyJhbGciOiJSUzI1NiJ9", "eyJhbGciOiJSUzI1NiJ9" ) );
            const char* payload = "eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFtcGxlLmNvbS9pc19yb290Ijp0cnVlfQ";
            ASSERT_TRUE( DecodesTo( payload, "{\"iss\":\"joe\",\r\n \"exp\":1300819380,\r\n \"http://example.com/is_root\":true}" ) );
            const char* payloadText = "{\"iss\":\"joe\",\r\n \"exp\":1300819380,\r\n \"http://example.com/is_root\":true}";
            ASSERT_TRUE( EncodesTo( payloadText, payload, ( CORE::CString( payload ) + "==" ).C_String() ) );

            // the usual JWT header
            ASSERT_TRUE( DecodesTo( "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9", "{\"alg\":\"RS256\",\"typ\":\"JWT\"}" ) );
            CORE::CString text;
            ASSERT_TRUE( CORE::Base64UrlEncode( "{\"alg\":\"RS256\",\"typ\":\"JWT\"}", 27, text ) && "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9" == text );

            // a signature sized blob that has every byte value, through both ways
            CORE::CDynamicBuffer blob;
            for ( UInt32 i=0; i<256; ++i )
            {
                UInt8 value = (UInt8) i;
                blob.Append( &value, 1, true );
            }
            ASSERT_TRUE( CORE::Base64UrlEncode( blob.GetConstBufferPtr(), blob.GetDataSize(), text ) );
            ASSERT_TRUE( 342 == text.Length() );
            ASSERT_TRUE( GUCEF_NULL == strchr( text.C_String(), '+' ) && GUCEF_NULL == strchr( text.C_String(), '/' ) && GUCEF_NULL == strchr( text.C_String(), '=' ) );
            CORE::CDynamicBuffer decoded;
            ASSERT_TRUE( CORE::Base64UrlDecode( text, decoded ) );
            ASSERT_TRUE( 256 == decoded.GetDataSize() && 0 == memcmp( blob.GetConstBufferPtr(), decoded.GetConstBufferPtr(), 256 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Every length from 0 to 300 round trips, with and without padding" )
        try
        {
            CORE::CDynamicBuffer data;
            UInt32 seed = 12345;
            for ( UInt32 length=0; length<=300; ++length )
            {
                if ( length > 0 )
                {
                    seed = seed * 1664525 + 1013904223;
                    UInt8 value = (UInt8) ( seed >> 24 );
                    data.Append( &value, 1, true );
                }
                for ( UInt32 pad=0; pad<2; ++pad )
                {
                    CORE::CString text;
                    CORE::CDynamicBuffer decoded;
                    ASSERT_TRUE( CORE::Base64UrlEncode( data.GetConstBufferPtr(), length, text, 0 != pad ) );
                    ASSERT_TRUE( CORE::Base64UrlDecode( text, decoded ) );
                    ASSERT_TRUE( length == decoded.GetDataSize() );
                    ASSERT_TRUE( 0 == length || 0 == memcmp( data.GetConstBufferPtr(), decoded.GetConstBufferPtr(), length ) );
                    ASSERT_TRUE( ( pad ? ( ( length + 2 ) / 3 ) * 4 : ( length * 4 + 2 ) / 3 ) == text.Length() );
                }
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Invalid input is refused: the standard alphabet, whitespace, bad padding, bad length, stray bits" )
        try
        {
            // the standard alphabet
            ASSERT_TRUE( IsRefused( "++++" ) );
            ASSERT_TRUE( IsRefused( "////" ) );
            ASSERT_TRUE( IsRefused( "Zm9+" ) );
            ASSERT_TRUE( IsRefused( "-_8/" ) );

            // whitespace of any kind, anywhere
            ASSERT_TRUE( IsRefused( " Zm9v" ) );
            ASSERT_TRUE( IsRefused( "Zm9v " ) );
            ASSERT_TRUE( IsRefused( "Zm 9v" ) );
            ASSERT_TRUE( IsRefused( "Zm9v\n" ) );
            ASSERT_TRUE( IsRefused( "Zm9v\r\n" ) );
            ASSERT_TRUE( IsRefused( "Zm\t9v" ) );
            ASSERT_TRUE( IsRefused( "Zg== " ) );
            ASSERT_TRUE( IsRefused( "Zm9v\vYmFy" ) );

            // other characters
            ASSERT_TRUE( IsRefused( "Zm9v!" ) );
            ASSERT_TRUE( IsRefused( "Zm.v" ) );
            ASSERT_TRUE( IsRefused( "Zm9v\xC3\xA9" ) );
            ASSERT_TRUE( IsRefused( "Zm9\x80" ) );
            ASSERT_TRUE( IsRefused( "." ) );
            ASSERT_TRUE( IsRefused( "eyJhbGciOiJSUzI1NiJ9.eyJpc3MiOiJqb2UifQ" ) );

            // lengths that leave a single character in the last group
            ASSERT_TRUE( IsRefused( "Z" ) );
            ASSERT_TRUE( IsRefused( "Zm9vY" ) );
            ASSERT_TRUE( IsRefused( "Zm9vYmFyZ" ) );

            // padding that is not right
            ASSERT_TRUE( IsRefused( "=" ) );
            ASSERT_TRUE( IsRefused( "==" ) );
            ASSERT_TRUE( IsRefused( "====" ) );
            ASSERT_TRUE( IsRefused( "Zg=" ) );
            ASSERT_TRUE( IsRefused( "Zg===" ) );
            ASSERT_TRUE( IsRefused( "Zm8==" ) );
            ASSERT_TRUE( IsRefused( "Zm9v=" ) );
            ASSERT_TRUE( IsRefused( "Zm9v====" ) );
            ASSERT_TRUE( IsRefused( "Z===" ) );
            ASSERT_TRUE( IsRefused( "=Zg=" ) );
            ASSERT_TRUE( IsRefused( "Z=g=" ) );
            ASSERT_TRUE( IsRefused( "Zg==Zg==" ) );
            ASSERT_TRUE( IsRefused( "Zm9vYg=" ) );

            // bits that belong to no byte are set: more than one text for the same bytes
            ASSERT_TRUE( IsRefused( "Zh" ) );
            ASSERT_TRUE( IsRefused( "Zh==" ) );
            ASSERT_TRUE( IsRefused( "Zm9" ) );
            ASSERT_TRUE( IsRefused( "Zm9=" ) );
            ASSERT_TRUE( DecodesTo( "Zm8", "fo" ) );

            // a refused text does not need a lot of allocated room first
            CORE::CDynamicBuffer result;
            ASSERT_FALSE( CORE::Base64UrlDecode( "Zm9v!mFy", result ) );
            ASSERT_TRUE( 0 == result.GetDataSize() );

            // NULL data with a length
            CORE::CString text;
            text = "left over";
            ASSERT_FALSE( CORE::Base64UrlEncode( GUCEF_NULL, 3, text ) );
            ASSERT_TRUE( text.IsNULLOrEmpty() );
            ASSERT_TRUE( CORE::Base64UrlEncode( GUCEF_NULL, 0, text ) && text.IsNULLOrEmpty() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A large input round trips, and decoding into a buffer that has its size allocates nothing" )
        try
        {
            const UInt32 size = 1024 * 1024 + 1;
            CORE::CDynamicBuffer data;
            data.SetBufferSize( size );
            UInt8* p = static_cast< UInt8* >( data.GetBufferPtr() );
            UInt32 seed = 99;
            for ( UInt32 i=0; i<size; ++i )
            {
                seed = seed * 1664525 + 1013904223;
                p[ i ] = (UInt8) ( seed >> 24 );
            }
            data.SetDataSize( size );

            CORE::CString text;
            CORE::CDynamicBuffer decoded;
            ASSERT_TRUE( CORE::Base64UrlEncode( data.GetConstBufferPtr(), size, text ) );
            ASSERT_TRUE( CORE::Base64UrlDecode( text, decoded ) );
            ASSERT_TRUE( size == decoded.GetDataSize() && 0 == memcmp( data.GetConstBufferPtr(), decoded.GetConstBufferPtr(), size ) );

            if ( CORETEST::IsAllocationCountingAvailable() )
            {
                // the second decode into the same buffer needs no memory at all
                CORETEST::StartAllocationCounting();
                bool decodedAgain = CORE::Base64UrlDecode( text, decoded );
                UInt32 decodeAllocations = CORETEST::StopAllocationCounting();
                ASSERT_TRUE( decodedAgain );
                ASSERT_TRUE( 0 == decodeAllocations );

                // and encoding is one allocation for the text, not one per character
                CORE::CString again;
                CORETEST::StartAllocationCounting();
                bool encodedAgain = CORE::Base64UrlEncode( data.GetConstBufferPtr(), size, again );
                UInt32 encodeAllocations = CORETEST::StopAllocationCounting();
                ASSERT_TRUE( encodedAgain && again == text );
                ASSERT_TRUE( encodeAllocations <= 2 );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The standard decoder refuses a text whose length cannot be base64, and does not read past the end of it" )
        try
        {
            // one character more than a whole number of groups is never a valid text: the last group has no byte in it
            const char* impossible[] = { "A", "AAAAA", "AAAAAAAAA", "!!!!not base64 at all!!!!", "AAAA=" };
            for ( UInt32 i=0; i<sizeof( impossible ) / sizeof( impossible[ 0 ] ); ++i )
            {
                CORE::CString text( impossible[ i ] );
                GUCEF::vector< UInt8 > buffer( 64, 0 );
                UInt32 used = 99;
                ASSERT_FALSE( CORE::Base64Decode( text, &buffer[ 0 ], (UInt32) buffer.size(), used ) );
                ASSERT_TRUE( 0 == used );
            }

            // and the texts that are base64 still decode
            GUCEF::vector< UInt8 > buffer( 64, 0 );
            UInt32 used = 0;
            ASSERT_TRUE( CORE::Base64Decode( "TWFu", &buffer[ 0 ], (UInt32) buffer.size(), used ) && 3 == used && 'M' == buffer[ 0 ] && 'a' == buffer[ 1 ] && 'n' == buffer[ 2 ] );
            ASSERT_TRUE( CORE::Base64Decode( "TWE=", &buffer[ 0 ], (UInt32) buffer.size(), used ) && 2 == used );
            ASSERT_TRUE( CORE::Base64Decode( "TQ==", &buffer[ 0 ], (UInt32) buffer.size(), used ) && 1 == used );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL Base64Url TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
