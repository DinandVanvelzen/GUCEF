/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CPBKDF2HMACSHA256_H
#include "gucefCORE_CPbkdf2HmacSha256.h"
#define GUCEF_CORE_CPBKDF2HMACSHA256_H
#endif /* GUCEF_CORE_CPBKDF2HMACSHA256_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPbkdf2HmacSha256.h"

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

namespace {

/** Lower case hexadecimal text of the bytes */
CString
Pbkdf2TestHex( const UInt8* bytes, UInt32 size )
{
    static const char digits[] = "0123456789abcdef";
    CString text;
    for ( UInt32 i=0; i<size; ++i )
    {
        char pair[ 3 ] = { digits[ bytes[ i ] >> 4 ], digits[ bytes[ i ] & 0x0F ], '\0' };
        text += pair;
    }
    return text;
}

/*-------------------------------------------------------------------------*/

/** Derives and compares with the expected hexadecimal text, logging both when they differ */
bool
Pbkdf2TestDerives( const void* password, UInt32 passwordSize, const void* salt, UInt32 saltSize, UInt32 iterations, const char* expectedHex )
{
    const UInt32 outputSize = (UInt32) strlen( expectedHex ) / 2;
    UInt8 output[ 128 ];
    memset( output, 0xEE, sizeof( output ) );
    if ( !CPbkdf2HmacSha256::Derive( password, passwordSize, salt, saltSize, iterations, output, outputSize ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Pbkdf2 test: Derive() refused " << iterations << " iterations for " << outputSize << " bytes" );
        return false;
    }
    // nothing is written past the requested size
    for ( UInt32 i=outputSize; i<sizeof( output ); ++i )
    {
        if ( 0xEE != output[ i ] )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Pbkdf2 test: Derive() wrote past " << outputSize << " bytes" );
            return false;
        }
    }
    const CString actual = Pbkdf2TestHex( output, outputSize );
    if ( actual == expectedHex )
        return true;
    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Pbkdf2 test: expected " << expectedHex << " but got " << actual );
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPbkdf2HmacSha256Tests( void )
{GUCEF_TRACE;

    GUCEF_LOG( LOGLEVEL_NORMAL, "COMMENCING PBKDF2-HMAC-SHA256 TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Pbkdf2HmacSha256" );

    GUCEF_TESTFW_TESTCASE( "Test: PBKDF2-HMAC-SHA256 gives the vectors of RFC 7914 section 11 (64 bytes, two blocks)" )
        try
        {
            ASSERT_TRUE( Pbkdf2TestDerives( "passwd", 6, "salt", 4, 1,
                "55ac046e56e3089fec1691c22544b605f94185216dde0465e68b9d57c20dacbc49ca9cccf179b645991664b39d77ef317c71b845b1e30bd509112041d3a19783" ) );
            ASSERT_TRUE( Pbkdf2TestDerives( "Password", 8, "NaCl", 4, 80000,
                "4ddcd8f60b98be21830cee5ef22701f9641a4418d04c0414aeff08876b34ab56a1d425a1225833549adb841b51c9b3176a272bdebba1d078478f62b397f33c8d" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: PBKDF2-HMAC-SHA256 gives the SHA-256 results for the inputs of RFC 6070" )
        try
        {
            ASSERT_TRUE( Pbkdf2TestDerives( "password", 8, "salt", 4, 1, "120fb6cffcf8b32c43e7225256c4f837a86548c92ccc35480805987cb70be17b" ) );
            ASSERT_TRUE( Pbkdf2TestDerives( "password", 8, "salt", 4, 2, "ae4d0c95af6b46d32d0adff928f06dd02a303f8ef3c251dfd6e2d85a95474c43" ) );
            ASSERT_TRUE( Pbkdf2TestDerives( "password", 8, "salt", 4, 4096, "c5e478d59288c841aa530db6845c4c8d962893a001ce4e11a4963873aa98134a" ) );

            // 40 bytes: a whole block and a part of the second
            ASSERT_TRUE( Pbkdf2TestDerives( "passwordPASSWORDpassword", 24, "saltSALTsaltSALTsaltSALTsaltSALTsalt", 36, 4096,
                "348c89dbcbd32b2f32d814b8116e84cf2b17347ebc1800181c4e2a1fb8dd53e1c635518c7dac47e9" ) );

            // NUL bytes are bytes like any other, 16 bytes: a part of the first block
            ASSERT_TRUE( Pbkdf2TestDerives( "pass\0word", 9, "sa\0lt", 5, 4096, "89b69d0516f829893c696226650a8687" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: PBKDF2-HMAC-SHA256 takes an empty password or salt and the 600 000 iterations of an approval PIN" )
        try
        {
            ASSERT_TRUE( Pbkdf2TestDerives( "", 0, "salt", 4, 1, "f135c27993baf98773c5cdb40a5706ce6a345cde61b000a67858650cd6a324d7" ) );
            ASSERT_TRUE( Pbkdf2TestDerives( "password", 8, GUCEF_NULL, 0, 1, "c1232f10f62715fda06ae7c0a2037ca19b33cf103b727ba56d870c11f290a2ab" ) );
            ASSERT_TRUE( Pbkdf2TestDerives( "password", 8, "salt", 4, 600000, "669cfe52482116fda1aa2cbe409b2f56c8e4563752b7a28f6eaab614ee005178" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: PBKDF2-HMAC-SHA256 refuses 0 iterations, 0 bytes and no output, and writes nothing then" )
        try
        {
            UInt8 output[ 32 ];
            memset( output, 0x5A, sizeof( output ) );
            ASSERT_FALSE( CPbkdf2HmacSha256::Derive( "password", 8, "salt", 4, 0, output, sizeof( output ) ) );
            ASSERT_FALSE( CPbkdf2HmacSha256::Derive( "password", 8, "salt", 4, 1, output, 0 ) );
            ASSERT_FALSE( CPbkdf2HmacSha256::Derive( "password", 8, "salt", 4, 1, GUCEF_NULL, 32 ) );
            bool untouched = true;
            for ( UInt32 i=0; i<sizeof( output ); ++i )
                untouched = untouched && 0x5A == output[ i ];
            ASSERT_TRUE( untouched );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
