/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_CURLENCODER_H
#include "gucefCORE_CUrlEncoder.h"
#define GUCEF_CORE_CURLENCODER_H
#endif /* GUCEF_CORE_CURLENCODER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CURI_H
#include "gucefCORE_CUri.h"
#define GUCEF_CORE_CURI_H
#endif /* GUCEF_CORE_CURI_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestUri.h"

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

void
PerformUriTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CUri TESTS" );
    
    GUCEF_TESTFW_SUITE_SCOPE( "CUri" );

    GUCEF_TESTFW_TESTCASE( "Test: URI parsing" )
    try
    {
        CORE::CUri emptyUri;
        ASSERT_TRUE( emptyUri.GetScheme().IsNULLOrEmpty() );
        ASSERT_TRUE( emptyUri.GetAuthority().IsNULLOrEmpty() );
        ASSERT_TRUE( !emptyUri.HasAuthority() );
        ASSERT_TRUE( emptyUri.GetAuthorityUsername().IsNULLOrEmpty() );
        ASSERT_TRUE( emptyUri.GetAuthorityPassword().IsNULLOrEmpty() );        
        ASSERT_TRUE( emptyUri.GetAuthorityHost().IsNULLOrEmpty() );
        ASSERT_TRUE( emptyUri.GetAuthorityPort().IsNULLOrEmpty() );
        ASSERT_TRUE( emptyUri.GetPath().IsNULLOrEmpty() );
        ASSERT_TRUE( emptyUri.GetQuery().IsNULLOrEmpty() );
        ASSERT_TRUE( emptyUri.GetFragment().IsNULLOrEmpty() );

        CORE::CUri uri( "scheme://user:pass@host:port/path/path/path?query#fragment" );
        ASSERT_TRUE( uri.GetScheme() == "scheme" );
        ASSERT_TRUE( uri.GetAuthority() == "user:pass@host:port" );
        ASSERT_TRUE( uri.HasAuthority() );
        ASSERT_TRUE( uri.GetAuthorityUsername() == "user" );
        ASSERT_TRUE( uri.GetAuthorityPassword() == "pass" );        
        ASSERT_TRUE( uri.GetAuthorityHost() == "host" );
        ASSERT_TRUE( uri.GetAuthorityPort() == "port" );
        ASSERT_TRUE( uri.GetPath() == "path/path/path" );
        ASSERT_TRUE( uri.GetQuery() == "query" );
        ASSERT_TRUE( uri.GetFragment() == "fragment" );
        ASSERT_TRUE( "scheme://user:pass@host:port/path/path/path?query#fragment" == CORE::ToString( uri ) );

        CORE::CUri uri2( "scheme:path/path/path?query#fragment" );
        ASSERT_TRUE( uri2.GetScheme() == "scheme" );
        ASSERT_TRUE( uri2.GetAuthority().IsNULLOrEmpty() );
        ASSERT_TRUE( !uri2.HasAuthority() );
        ASSERT_TRUE( uri2.GetAuthorityUsername().IsNULLOrEmpty() );
        ASSERT_TRUE( uri2.GetAuthorityPassword().IsNULLOrEmpty() );        
        ASSERT_TRUE( uri2.GetAuthorityHost().IsNULLOrEmpty() );
        ASSERT_TRUE( uri2.GetAuthorityPort().IsNULLOrEmpty() );
        ASSERT_TRUE( uri2.GetPath() == "path/path/path" );
        ASSERT_TRUE( uri2.GetQuery() == "query" );
        ASSERT_TRUE( uri2.GetFragment() == "fragment" );

        CORE::CUri uri3( "scheme:path/path/path" );
        ASSERT_TRUE( uri3.GetScheme() == "scheme" );
        ASSERT_TRUE( uri3.GetAuthority().IsNULLOrEmpty() );
        ASSERT_TRUE( !uri3.HasAuthority() );
        ASSERT_TRUE( uri3.GetAuthorityUsername().IsNULLOrEmpty() );
        ASSERT_TRUE( uri3.GetAuthorityPassword().IsNULLOrEmpty() );        
        ASSERT_TRUE( uri3.GetAuthorityHost().IsNULLOrEmpty() );
        ASSERT_TRUE( uri3.GetAuthorityPort().IsNULLOrEmpty() );
        ASSERT_TRUE( uri3.GetPath() == "path/path/path" );
        ASSERT_TRUE( uri3.GetQuery().IsNULLOrEmpty() );
        ASSERT_TRUE( uri3.GetFragment().IsNULLOrEmpty() );

        CORE::CUri uri4( "scheme:path/path/path?query" );
        ASSERT_TRUE( uri4.GetScheme() == "scheme" );
        ASSERT_TRUE( uri4.GetAuthority().IsNULLOrEmpty() );
        ASSERT_TRUE( !uri4.HasAuthority() );
        ASSERT_TRUE( uri4.GetAuthorityUsername().IsNULLOrEmpty() );
        ASSERT_TRUE( uri4.GetAuthorityPassword().IsNULLOrEmpty() );        
        ASSERT_TRUE( uri4.GetAuthorityHost().IsNULLOrEmpty() );
        ASSERT_TRUE( uri4.GetAuthorityPort().IsNULLOrEmpty() );
        ASSERT_TRUE( uri4.GetPath() == "path/path/path" );
        ASSERT_TRUE( uri4.GetQuery() == "query" );
        ASSERT_TRUE( uri4.GetFragment().IsNULLOrEmpty() );

        ASSERT_TRUE( emptyUri != uri );
        ASSERT_TRUE( uri != uri2 );
        ASSERT_TRUE( uri2 != uri3 );
        ASSERT_TRUE( uri3 != uri4 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: URL encoding and decoding and the hex of bytes give the hex of every byte, the bytes of UTF-8 from 0x80 on included" )
    try
    {
        // a byte is two upper case hex digits, whatever the sign of char is on this platform
        char a = 0;
        char b = 0;
        CORE::ConvertByteToHexChars( (char) 0x00, a, b );
        ASSERT_TRUE( '0' == a && '0' == b );
        CORE::ConvertByteToHexChars( (char) 0x20, a, b );
        ASSERT_TRUE( '2' == a && '0' == b );
        CORE::ConvertByteToHexChars( (char) 0x7F, a, b );
        ASSERT_TRUE( '7' == a && 'F' == b );
        CORE::ConvertByteToHexChars( (char) 0x80, a, b );
        ASSERT_TRUE( '8' == a && '0' == b );
        CORE::ConvertByteToHexChars( (char) 0xC3, a, b );
        ASSERT_TRUE( 'C' == a && '3' == b );
        CORE::ConvertByteToHexChars( (char) 0xFF, a, b );
        ASSERT_TRUE( 'F' == a && 'F' == b );

        // a buffer of bytes as hex gives the same for every byte
        const unsigned char bytes[] = { 0x00, 0x1F, 0x7F, 0x80, 0xC3, 0xFF };
        ASSERT_TRUE( "001F7F80C3FF" == CORE::ConvertBytesToHexString( bytes, 6, false, false ) );
        ASSERT_TRUE( "00 1F 7F 80 C3 FF " == CORE::ConvertBytesToHexString( bytes, 6, true, false ) );
        ASSERT_TRUE( "0x00 0x1F 0x7F 0x80 0xC3 0xFF " == CORE::ConvertBytesToHexString( bytes, 6, true, true ) );

        // ASCII: only the unreserved characters stay as they are
        CORE::CString encoded;
        CORE::CUrlEncoder::Encode( CORE::CString( "plain-text_1.0~" ), encoded );
        ASSERT_TRUE( "plain-text_1.0~" == encoded );
        CORE::CUrlEncoder::Encode( CORE::CString( "a b+c@d/e?f=g&h" ), encoded );
        ASSERT_TRUE( "a%20b%2Bc%40d%2Fe%3Ff%3Dg%26h" == encoded );

        // UTF-8: every byte of a code point is a %XX
        CORE::CUrlEncoder::Encode( CORE::CString( "j\xC3\xBCrgen+test" ), encoded );
        ASSERT_TRUE( "j%C3%BCrgen%2Btest" == encoded );
        CORE::CUrlEncoder::Encode( CORE::CString( "\xE2\x82\xAC 100%" ), encoded );
        ASSERT_TRUE( "%E2%82%AC%20100%25" == encoded );
        CORE::CUrlEncoder::Encode( CORE::CString( "\xF0\x9F\x98\x80" ), encoded );
        ASSERT_TRUE( "%F0%9F%98%80" == encoded );

        // and back: what was encoded is what is decoded, for ASCII and for the bytes of every length of UTF-8
        CORE::CUtf8String decoded;
        ASSERT_TRUE( CORE::CUrlEncoder::DecodeFromAscii( CORE::CAsciiString( "a%20b%2Bc%40d%2Fe%3Ff%3Dg%26h" ), decoded ) );
        ASSERT_TRUE( "a b+c@d/e?f=g&h" == decoded );
        decoded.Clear();
        ASSERT_TRUE( CORE::CUrlEncoder::DecodeFromAscii( CORE::CAsciiString( "j%C3%BCrgen%2Btest%e2%82%AC-%F0%9F%98%80" ), decoded ) );
        ASSERT_TRUE( "j\xC3\xBCrgen+test\xE2\x82\xAC-\xF0\x9F\x98\x80" == decoded );
        decoded.Clear();
        ASSERT_TRUE( CORE::CUrlEncoder::DecodeFromAscii( CORE::CAsciiString( "" ), decoded ) && decoded.IsNULLOrEmpty() );

        // an escape that is cut off, has no hex digits, or is the start of a code point that is not finished is refused
        const char* malformed[] = { "%", "a%", "a%4", "%zz", "%4g", "%C3", "%C3x", "%C3%", "%E2%82", "%F0%9F%98" };
        for ( size_t i=0; i<sizeof( malformed ) / sizeof( malformed[ 0 ] ); ++i )
        {
            decoded.Clear();
            ASSERT_FALSE( CORE::CUrlEncoder::DecodeFromAscii( CORE::CAsciiString( malformed[ i ] ), decoded ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CUri TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
