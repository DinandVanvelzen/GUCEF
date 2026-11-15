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

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CSHA1_H
#include "gucefCORE_CSha1.h"
#define GUCEF_CORE_CSHA1_H
#endif /* GUCEF_CORE_CSHA1_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestSha1.h"

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

/** Compares the digest with the expected hexadecimal text, logging both when they differ */
bool
Sha1TestMatches( const CString& actualHex, const char* expectedHex )
{
    if ( actualHex == expectedHex )
        return true;
    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Sha1 test: expected " << expectedHex << " but got " << actualHex );
    return false;
}

/*-------------------------------------------------------------------------*/

/** One shot hash of a C string */
CString
Sha1TestHashText( const char* text )
{
    return CSha1::HashHex( text, (UInt32) strlen( text ) );
}

/*-------------------------------------------------------------------------*/

/** Deterministic pseudo random bytes (a 32 bit xorshift), so a failure can be repeated */
void
Sha1TestFill( UInt8* bytes, UInt32 size, UInt32 seed )
{
    UInt32 x = 0 == seed ? 0x9E3779B9 : seed;
    for ( UInt32 i=0; i<size; ++i )
    {
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        bytes[ i ] = (UInt8) ( x >> 24 );
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSha1Tests( void )
{GUCEF_TRACE;

    GUCEF_LOG( LOGLEVEL_NORMAL, "COMMENCING SHA-1 TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Sha1" );

    GUCEF_TESTFW_TESTCASE( "Test: SHA-1 gives the FIPS 180 vectors of the empty message, \"abc\" and the 448 and 896 bit messages" )
        try
        {
            ASSERT_TRUE( Sha1TestMatches( Sha1TestHashText( "" ), "da39a3ee5e6b4b0d3255bfef95601890afd80709" ) );
            ASSERT_TRUE( Sha1TestMatches( Sha1TestHashText( "abc" ), "a9993e364706816aba3e25717850c26c9cd0d89d" ) );
            ASSERT_TRUE( Sha1TestMatches( Sha1TestHashText( "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq" ),
                                          "84983e441c3bd26ebaae4aa1f95129e5e54670f1" ) );
            ASSERT_TRUE( Sha1TestMatches( Sha1TestHashText( "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu" ),
                                          "a49b2446a02c645bf419f995b67091253a04a259" ) );
            ASSERT_TRUE( CSha1::DigestSize == 20 && CSha1::BlockSize == 64 && CSha1::HexDigestLength == 40 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: SHA-1 of one million 'a' is the FIPS 180 vector, in one call and in a thousand calls" )
        try
        {
            const UInt32 size = 1000000;
            CDynamicBuffer buffer( size, false );
            memset( buffer.GetBufferPtr(), 'a', size );

            ASSERT_TRUE( Sha1TestMatches( CSha1::HashHex( buffer.GetConstBufferPtr(), size ), "34aa973cd4c4daa4f61eeb2bdbad27316534016f" ) );

            CSha1 sha;
            const UInt8* bytes = static_cast< const UInt8* >( buffer.GetConstBufferPtr() );
            for ( UInt32 i=0; i<1000; ++i )
                sha.Update( bytes + i*1000, 1000 );
            ASSERT_TRUE( Sha1TestMatches( sha.FinalHex(), "34aa973cd4c4daa4f61eeb2bdbad27316534016f" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: SHA-1 of the 1 GiB message of the NIST examples (2^33 bits, the length takes both words) is its vector" )
        try
        {
            // 16777216 times the 64 byte pattern, fed in pieces of 64 KiB
            static const char pattern[] = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno";
            const UInt32 patternSize = 64;
            const UInt32 repeatsPerPiece = 1024;
            CDynamicBuffer piece( patternSize * repeatsPerPiece, false );
            UInt8* bytes = static_cast< UInt8* >( piece.GetBufferPtr() );
            for ( UInt32 i=0; i<repeatsPerPiece; ++i )
                memcpy( bytes + i*patternSize, pattern, patternSize );

            CSha1 sha;
            const UInt32 pieces = 16777216 / repeatsPerPiece;
            for ( UInt32 i=0; i<pieces; ++i )
                sha.Update( bytes, patternSize * repeatsPerPiece );
            ASSERT_TRUE( Sha1TestMatches( sha.FinalHex(), "7789f0c9ef7bfc40d93311143dfbe69e2017f592" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: SHA-1 incremental equals one shot for every split of inputs up to 200 bytes, around the block and padding boundaries" )
        try
        {
            UInt8 data[ 200 ];
            Sha1TestFill( data, sizeof( data ), 12345 );
            bool allEqual = true;
            for ( UInt32 size=0; size<=200 && allEqual; ++size )
            {
                UInt8 oneShot[ CSha1::DigestSize ];
                CSha1::Hash( data, size, oneShot );
                for ( UInt32 split=0; split<=size; ++split )
                {
                    CSha1 sha;
                    sha.Update( data, split );
                    sha.Update( data + split, 0 );
                    sha.Update( data + split, size - split );
                    UInt8 incremental[ CSha1::DigestSize ];
                    sha.Final( incremental );
                    if ( 0 != memcmp( oneShot, incremental, CSha1::DigestSize ) )
                    {
                        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Sha1 test: size " << size << " split at " << split << " differs from the one shot hash" );
                        allEqual = false;
                        break;
                    }
                }
            }
            ASSERT_TRUE( allEqual );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: SHA-1 incremental equals one shot for long inputs fed in random chunks" )
        try
        {
            const UInt32 size = 3 * 1024 * 1024 + 17;
            CDynamicBuffer buffer( size, false );
            UInt8* data = static_cast< UInt8* >( buffer.GetBufferPtr() );
            Sha1TestFill( data, size, 777 );
            UInt8 oneShot[ CSha1::DigestSize ];
            CSha1::Hash( data, size, oneShot );

            UInt32 chunkSeed = 4242;
            for ( UInt32 round=0; round<4; ++round )
            {
                CSha1 sha;
                UInt32 offset = 0;
                while ( offset < size )
                {
                    chunkSeed ^= chunkSeed << 13;
                    chunkSeed ^= chunkSeed >> 17;
                    chunkSeed ^= chunkSeed << 5;
                    UInt32 chunk = chunkSeed % ( 1u << ( 4 + round * 4 ) );
                    if ( chunk > size - offset )
                        chunk = size - offset;
                    sha.Update( data + offset, chunk );
                    offset += chunk;
                }
                UInt8 incremental[ CSha1::DigestSize ];
                sha.Final( incremental );
                ASSERT_TRUE( 0 == memcmp( oneShot, incremental, CSha1::DigestSize ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: SHA-1 Reset() starts a new message, the hexadecimal forms round trip and bad text is refused" )
        try
        {
            CSha1 sha;
            sha.Update( "something else", 14 );
            sha.Reset();
            sha.Update( "abc", 3 );
            ASSERT_TRUE( Sha1TestMatches( sha.FinalHex(), "a9993e364706816aba3e25717850c26c9cd0d89d" ) );
            sha.Reset();
            ASSERT_TRUE( Sha1TestMatches( sha.FinalHex(), "da39a3ee5e6b4b0d3255bfef95601890afd80709" ) );

            UInt8 digest[ CSha1::DigestSize ];
            CSha1::Hash( "abc", 3, digest );
            ASSERT_TRUE( Sha1TestMatches( CSha1::DigestToHex( digest ), "a9993e364706816aba3e25717850c26c9cd0d89d" ) );

            UInt8 parsed[ CSha1::DigestSize ];
            ASSERT_TRUE( CSha1::DigestFromHex( "A9993E364706816ABA3E25717850C26C9CD0D89D", parsed ) );
            ASSERT_TRUE( 0 == memcmp( digest, parsed, CSha1::DigestSize ) );

            UInt8 untouched[ CSha1::DigestSize ];
            memset( untouched, 0x5A, sizeof( untouched ) );
            ASSERT_FALSE( CSha1::DigestFromHex( "a9993e364706816aba3e25717850c26c9cd0d89", untouched ) );
            ASSERT_FALSE( CSha1::DigestFromHex( "a9993e364706816aba3e25717850c26c9cd0d89d0", untouched ) );
            ASSERT_FALSE( CSha1::DigestFromHex( "g9993e364706816aba3e25717850c26c9cd0d89d", untouched ) );
            ASSERT_FALSE( CSha1::DigestFromHex( CString::Empty, untouched ) );
            bool unchanged = true;
            for ( UInt32 i=0; i<CSha1::DigestSize; ++i )
                unchanged = unchanged && 0x5A == untouched[ i ];
            ASSERT_TRUE( unchanged );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
