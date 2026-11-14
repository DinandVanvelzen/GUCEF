/*
 *  gucefMATH_TestApp: GUCEF test application for the Math module
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

#include <math.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_CPCG32_H
#include "gucefMATH_CPcg32.h"
#define GUCEF_MATH_CPCG32_H
#endif /* GUCEF_MATH_CPCG32_H ? */

#ifndef GUCEF_MATH_CXOSHIRO256STARSTAR_H
#include "gucefMATH_CXoshiro256StarStar.h"
#define GUCEF_MATH_CXOSHIRO256STARSTAR_H
#endif /* GUCEF_MATH_CXOSHIRO256STARSTAR_H ? */

#ifndef GUCEF_MATH_CSPLITMIX64_H
#include "gucefMATH_CSplitMix64.h"
#define GUCEF_MATH_CSPLITMIX64_H
#endif /* GUCEF_MATH_CSPLITMIX64_H ? */

#include "TestRandomGenerators.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
StepXoshiro( MATH::CXoshiro256StarStar& generator, MATH::UInt32 steps )
{
    for ( MATH::UInt32 i = 0; i < steps; ++i )
        generator.NextUInt64();
}

/*-------------------------------------------------------------------------*/

void
PerformRandomGeneratorsTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING RandomGenerators TESTS" );

    GUCEF_TESTFW_TESTCASE( "RandomGenerators: CPcg32 reproduces the reference sequence for state 42, stream 54" )
    {
        MATH::CPcg32 generator( 42, 54 );
        ASSERT_TRUE( generator.NextUInt32() == 0xa15c02b7U );
        ASSERT_TRUE( generator.NextUInt32() == 0x7b47f409U );
        ASSERT_TRUE( generator.NextUInt32() == 0xba1d3330U );
        ASSERT_TRUE( generator.NextUInt32() == 0x83d2f293U );
        ASSERT_TRUE( generator.NextUInt32() == 0xbfa4784bU );
        ASSERT_TRUE( generator.NextUInt32() == 0xcbed606eU );

        // 64 bit output: the upper half first
        MATH::CPcg32 wide( 42, 54 );
        ASSERT_TRUE( wide.NextUInt64() == 0xa15c02b77b47f409ULL );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomGenerators: CPcg32 streams differ, state round trip, Advance equals stepping" )
    {
        MATH::CPcg32 a( 42, 54 );
        MATH::CPcg32 b( 42, 55 );
        bool differs = false;
        for ( int i = 0; i < 8; ++i )
            if ( a.NextUInt32() != b.NextUInt32() ) differs = true;
        ASSERT_TRUE( differs );

        MATH::CPcg32 source( 7, 11 );
        for ( int i = 0; i < 100; ++i ) source.NextUInt32();
        MATH::UInt64 words[ MATH::CPcg32::STATE_WORD_COUNT ];
        source.GetState( words );
        MATH::CPcg32 restored;
        restored.SetState( words );
        ASSERT_TRUE( restored == source );
        for ( int i = 0; i < 100; ++i )
            ASSERT_TRUE( restored.NextUInt32() == source.NextUInt32() );

        const MATH::UInt64 deltas[] = { 0, 1, 2, 3, 17, 1000, 123456 };
        for ( int d = 0; d < 7; ++d )
        {
            MATH::CPcg32 stepped( 99, 3 );
            MATH::CPcg32 advanced( 99, 3 );
            for ( MATH::UInt64 i = 0; i < deltas[ d ]; ++i ) stepped.NextUInt32();
            advanced.Advance( deltas[ d ] );
            ASSERT_TRUE( stepped == advanced );
            ASSERT_TRUE( stepped.NextUInt32() == advanced.NextUInt32() );
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomGenerators: CSplitMix64 reproduces the reference sequence for seed 0" )
    {
        MATH::CSplitMix64 generator( 0 );
        ASSERT_TRUE( generator.NextUInt64() == 0xe220a8397b1dcdafULL );
        ASSERT_TRUE( generator.NextUInt64() == 0x6e789e6aa1b965f4ULL );
        ASSERT_TRUE( generator.NextUInt64() == 0x06c45d188009454fULL );

        MATH::UInt64 words[ MATH::CSplitMix64::STATE_WORD_COUNT ];
        generator.GetState( words );
        MATH::CSplitMix64 restored( 12345 );
        restored.SetState( words );
        ASSERT_TRUE( restored == generator );
        ASSERT_TRUE( restored.NextUInt64() == generator.NextUInt64() );

        MATH::CSplitMix64 reseeded( 5 );
        reseeded.NextUInt64();
        reseeded.Seed( 0 );
        ASSERT_TRUE( reseeded.NextUInt64() == 0xe220a8397b1dcdafULL );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomGenerators: CXoshiro256StarStar reproduces the reference output for state { 1, 2, 3, 4 }" )
    {
        MATH::CXoshiro256StarStar generator;
        const MATH::UInt64 state[ 4 ] = { 1, 2, 3, 4 };
        generator.SetState( state );
        ASSERT_TRUE( generator.NextUInt64() == 11520ULL );
        ASSERT_TRUE( generator.NextUInt64() == 0ULL );
        ASSERT_TRUE( generator.NextUInt64() == 1509978240ULL );
        ASSERT_TRUE( generator.NextUInt64() == 1215971899390074240ULL );

        // The seeded state comes from SplitMix64 (values computed independently of the implementation)
        MATH::CXoshiro256StarStar seeded( 12345 );
        MATH::UInt64 words[ MATH::CXoshiro256StarStar::STATE_WORD_COUNT ];
        seeded.GetState( words );
        ASSERT_TRUE( words[ 0 ] == 0x22118258a9d111a0ULL && words[ 1 ] == 0x346edce5f713f8edULL );
        ASSERT_TRUE( words[ 2 ] == 0x1e9a57bc80e6721dULL && words[ 3 ] == 0x2d160e7e5c3f42caULL );
        ASSERT_TRUE( seeded.NextUInt64() == 0xbe6a36374160d49bULL );
        ASSERT_TRUE( seeded.NextUInt64() == 0x214aaa0637a688c6ULL );
        ASSERT_TRUE( seeded.NextUInt64() == 0xf69d16de9954d388ULL );

        // All zero is not a valid state and is replaced
        const MATH::UInt64 zero[ 4 ] = { 0, 0, 0, 0 };
        generator.SetState( zero );
        ASSERT_TRUE( generator.NextUInt64() != 0 || generator.NextUInt64() != 0 );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomGenerators: CXoshiro256StarStar state round trip reproduces sequences" )
    {
        MATH::CXoshiro256StarStar source( 99 );
        StepXoshiro( source, 50 );
        MATH::UInt64 words[ 4 ];
        source.GetState( words );
        MATH::CXoshiro256StarStar restored( 1 );
        restored.SetState( words );
        ASSERT_TRUE( restored == source );
        for ( int i = 0; i < 100; ++i )
            ASSERT_TRUE( restored.NextUInt64() == source.NextUInt64() );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomGenerators: Jump and LongJump equal 2^128 and 2^192 steps (matrix power over GF(2), computed independently)" )
    {
        MATH::CXoshiro256StarStar jumped( 12345 );
        jumped.Jump();
        MATH::UInt64 words[ 4 ];
        jumped.GetState( words );
        ASSERT_TRUE( words[ 0 ] == 0xc447e65c62d994cfULL && words[ 1 ] == 0xaf415ed201c9e97eULL );
        ASSERT_TRUE( words[ 2 ] == 0x620fb38cd6dd52f4ULL && words[ 3 ] == 0xe6ba5be4e54b26c6ULL );
        ASSERT_TRUE( jumped.NextUInt64() == 0x3ed575283f0594e6ULL );
        ASSERT_TRUE( jumped.NextUInt64() == 0x4b77bcfa88a79146ULL );

        MATH::CXoshiro256StarStar longJumped( 12345 );
        longJumped.LongJump();
        longJumped.GetState( words );
        ASSERT_TRUE( words[ 0 ] == 0x8214f87eab5fb1f3ULL && words[ 1 ] == 0x026ce80e481688eaULL );
        ASSERT_TRUE( words[ 2 ] == 0x14ad1dceca88f2deULL && words[ 3 ] == 0xead921a5c5e3ea25ULL );
        ASSERT_TRUE( longJumped.NextUInt64() == 0x92654155fb089136ULL );
        ASSERT_TRUE( longJumped.NextUInt64() == 0xb9b536ab88690194ULL );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomGenerators: the jump mechanism with reduced polynomials equals stepping" )
    {
        // Polynomial x^k selects the state after k steps; the sum of polynomials the xor of those states
        const MATH::UInt32 exponents[] = { 0, 1, 5, 63, 64, 100, 255 };
        for ( int e = 0; e < 7; ++e )
        {
            MATH::UInt64 polynomial[ 4 ] = { 0, 0, 0, 0 };
            polynomial[ exponents[ e ] / 64 ] = 1ULL << ( exponents[ e ] % 64 );
            MATH::CXoshiro256StarStar viaPolynomial( 777 );
            viaPolynomial.ApplyJumpPolynomial( polynomial );
            MATH::CXoshiro256StarStar stepped( 777 );
            StepXoshiro( stepped, exponents[ e ] );
            if ( !( viaPolynomial == stepped ) )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RandomGenerators: the jump polynomial x^" << exponents[ e ] << " differs from stepping" );
            ASSERT_TRUE( viaPolynomial == stepped );
        }

        MATH::UInt64 sum[ 4 ] = { 0x2ULL | 0x20ULL, 0, 0, 0 };     // x^1 + x^5
        MATH::CXoshiro256StarStar viaSum( 777 );
        viaSum.ApplyJumpPolynomial( sum );
        MATH::CXoshiro256StarStar one( 777 );
        StepXoshiro( one, 1 );
        MATH::CXoshiro256StarStar five( 777 );
        StepXoshiro( five, 5 );
        MATH::UInt64 wordsOne[ 4 ], wordsFive[ 4 ], wordsSum[ 4 ];
        one.GetState( wordsOne );
        five.GetState( wordsFive );
        viaSum.GetState( wordsSum );
        for ( int i = 0; i < 4; ++i )
            ASSERT_TRUE( wordsSum[ i ] == ( wordsOne[ i ] ^ wordsFive[ i ] ) );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RandomGenerators TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
