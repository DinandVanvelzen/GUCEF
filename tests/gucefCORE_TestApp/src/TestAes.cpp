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

#ifndef GUCEF_CORE_CAES_H
#include "gucefCORE_CAes.h"
#define GUCEF_CORE_CAES_H
#endif /* GUCEF_CORE_CAES_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAes.h"

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

/** The bytes of a hexadecimal text; the count written */
UInt32
FromHex( const char* hex, UInt8* bytes, UInt32 capacity )
{
    UInt32 count = 0;
    while ( 0 != hex[ 0 ] && 0 != hex[ 1 ] && count < capacity )
    {
        UInt8 value = 0;
        for ( int i=0; i<2; ++i )
        {
            const char c = hex[ i ];
            value = (UInt8) ( value << 4 );
            if ( c >= '0' && c <= '9' ) value = (UInt8) ( value | ( c - '0' ) );
            else if ( c >= 'a' && c <= 'f' ) value = (UInt8) ( value | ( c - 'a' + 10 ) );
            else if ( c >= 'A' && c <= 'F' ) value = (UInt8) ( value | ( c - 'A' + 10 ) );
        }
        bytes[ count++ ] = value;
        hex += 2;
    }
    return count;
}

/** One block of FIPS-197 Appendix C both ways */
bool
BlockVector( const char* keyHex, const char* plainHex, const char* cipherHex )
{
    UInt8 key[ 32 ], plain[ 16 ], cipher[ 16 ], out[ 16 ];
    const UInt32 keySize = FromHex( keyHex, key, sizeof( key ) );
    FromHex( plainHex, plain, sizeof( plain ) );
    FromHex( cipherHex, cipher, sizeof( cipher ) );
    CAes aes;
    if ( !aes.SetKey( key, keySize ) )
        return false;
    aes.EncryptBlock( plain, out );
    if ( 0 != memcmp( out, cipher, 16 ) )
        return false;
    aes.DecryptBlock( cipher, out );
    if ( 0 != memcmp( out, plain, 16 ) )
        return false;
    // in place
    memcpy( out, plain, 16 );
    aes.EncryptBlock( out, out );
    return 0 == memcmp( out, cipher, 16 );
}

/** One CBC vector of SP 800-38A F.2: four blocks, no padding, both ways */
bool
CbcVector( const char* keyHex, const char* ivHex, const char* plainHex, const char* cipherHex )
{
    UInt8 key[ 32 ], iv[ 16 ], plain[ 64 ], cipher[ 64 ];
    const UInt32 keySize = FromHex( keyHex, key, sizeof( key ) );
    FromHex( ivHex, iv, sizeof( iv ) );
    const UInt32 plainSize = FromHex( plainHex, plain, sizeof( plain ) );
    const UInt32 cipherSize = FromHex( cipherHex, cipher, sizeof( cipher ) );
    CDynamicBuffer out;
    if ( !CAes::CbcEncrypt( key, keySize, iv, plain, plainSize, false, out ) || out.GetDataSize() != cipherSize ||
         0 != memcmp( out.GetConstBufferPtr(), cipher, cipherSize ) )
        return false;
    CDynamicBuffer back;
    return CAes::CbcDecrypt( key, keySize, iv, cipher, cipherSize, false, back ) && back.GetDataSize() == plainSize &&
           0 == memcmp( back.GetConstBufferPtr(), plain, plainSize );
}

const char* const g_sp80038aPlain = "6bc1bee22e409f96e93d7e117393172aae2d8a571e03ac9c9eb76fac45af8e5130c81c46a35ce411e5fbc1191a0a52eff69f2445df4f9b17ad2b417be66c3710";
const char* const g_sp80038aIv = "000102030405060708090a0b0c0d0e0f";

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAesTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( LOGLEVEL_NORMAL, "COMMENCING AES TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Aes" );

    GUCEF_TESTFW_TESTCASE( "Test: AES gives the example vectors of FIPS-197 Appendix C for 128, 192 and 256 bit keys, both ways and in place" )
        try
        {
            ASSERT_TRUE( BlockVector( "000102030405060708090a0b0c0d0e0f", "00112233445566778899aabbccddeeff", "69c4e0d86a7b0430d8cdb78070b4c55a" ) );
            ASSERT_TRUE( BlockVector( "000102030405060708090a0b0c0d0e0f1011121314151617", "00112233445566778899aabbccddeeff", "dda97ca4864cdfe06eaf70a0ec0d7191" ) );
            ASSERT_TRUE( BlockVector( "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f", "00112233445566778899aabbccddeeff", "8ea2b7ca516745bfeafc49904b496089" ) );
            // FIPS-197 Appendix B
            ASSERT_TRUE( BlockVector( "2b7e151628aed2a6abf7158809cf4f3c", "3243f6a8885a308d313198a2e0370734", "3925841d02dc09fbdc118597196a0b32" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: AES-CBC gives the vectors of NIST SP 800-38A F.2.1 to F.2.6" )
        try
        {
            ASSERT_TRUE( CbcVector( "2b7e151628aed2a6abf7158809cf4f3c", g_sp80038aIv, g_sp80038aPlain,
                                    "7649abac8119b246cee98e9b12e9197d5086cb9b507219ee95db113a917678b273bed6b8e3c1743b7116e69e222295163ff1caa1681fac09120eca307586e1a7" ) );
            ASSERT_TRUE( CbcVector( "8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b", g_sp80038aIv, g_sp80038aPlain,
                                    "4f021db243bc633d7178183a9fa071e8b4d9ada9ad7dedf4e5e738763f69145a571b242012fb7ae07fa9baac3df102e008b0e27988598881d920a9e64f5615cd" ) );
            ASSERT_TRUE( CbcVector( "603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4", g_sp80038aIv, g_sp80038aPlain,
                                    "f58c4c04d6e5f1ba779eabfb5f7bfbd69cfc4e967edb808d679f777bc6702c7d39f23369a9d9bacfa530e26304231461b2eb05e2c39be9fcda6c19078c6a9d1b" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: AES-CBC with PKCS#7 padding round trips every length; refuses a wrong key size, a partial block and a broken padding" )
        try
        {
            UInt8 key[ 16 ], iv[ 16 ], data[ 40 ];
            for ( UInt32 i=0; i<16; ++i ) { key[ i ] = (UInt8) ( 3 * i + 1 ); iv[ i ] = (UInt8) ( 255 - i ); }
            for ( UInt32 i=0; i<40; ++i ) data[ i ] = (UInt8) ( i * 7 );
            for ( UInt32 length=0; length<=40; ++length )
            {
                CDynamicBuffer cipher, plain;
                ASSERT_TRUE( CAes::CbcEncrypt( key, 16, iv, data, length, true, cipher ) );
                ASSERT_TRUE( cipher.GetDataSize() == ( length / 16 + 1 ) * 16 );
                ASSERT_TRUE( CAes::CbcDecrypt( key, 16, iv, static_cast< const UInt8* >( cipher.GetConstBufferPtr() ), cipher.GetDataSize(), true, plain ) );
                ASSERT_TRUE( plain.GetDataSize() == length && ( 0 == length || 0 == memcmp( plain.GetConstBufferPtr(), data, length ) ) );
            }
            CDynamicBuffer out;
            ASSERT_FALSE( CAes::CbcEncrypt( key, 15, iv, data, 16, false, out ) );
            ASSERT_FALSE( CAes::CbcEncrypt( key, 16, iv, data, 17, false, out ) );
            ASSERT_FALSE( CAes::CbcDecrypt( key, 16, iv, data, 20, false, out ) );
            ASSERT_TRUE( 0 == out.GetDataSize() );

            // a padding byte that does not fit: refused, nothing given
            CDynamicBuffer cipher;
            ASSERT_TRUE( CAes::CbcEncrypt( key, 16, iv, data, 16, false, cipher ) );
            CDynamicBuffer plain;
            ASSERT_FALSE( CAes::CbcDecrypt( key, 16, iv, static_cast< const UInt8* >( cipher.GetConstBufferPtr() ), cipher.GetDataSize(), true, plain ) );
            ASSERT_TRUE( 0 == plain.GetDataSize() );

            CAes aes;
            ASSERT_FALSE( aes.IsKeySet() );
            ASSERT_TRUE( aes.SetKey( key, 16 ) && aes.IsKeySet() );
            aes.Clear();
            ASSERT_FALSE( aes.IsKeySet() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
