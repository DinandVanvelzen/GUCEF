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

#include <float.h>
#include <math.h>
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef GUCEF_MATH_CAMBISONICFORMAT_H
#include "gucefMATH_CAmbisonicFormat.h"
#define GUCEF_MATH_CAMBISONICFORMAT_H
#endif /* GUCEF_MATH_CAMBISONICFORMAT_H ? */

#ifndef GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H
#include "gucefMATH_LowDiscrepancySequences.h"
#define GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H
#endif /* GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H ? */

#include "TestMathReference.h"
#include "TestMathAllocationCounter.h"
#include "TestAmbisonics.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::MATH;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const Float64 g_pi = 3.14159265358979323846;

/*-------------------------------------------------------------------------*/

/** Conversion and encoding from a realtime context: a RealtimeSanitizer build reports any allocation */
static Float32
RealtimeAmbisonicWork( Float32* block, UInt32 frames ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 gains[ 16 ];
    bool ok = CAmbisonicFormat::EncodeDirection( TFloat32Radian( 0.5f ), TFloat32Radian( 0.1f ), 3, SHNORM_SN3D, gains );
    ok = CAmbisonicFormat::EncodeListenerDirection( TFloat32DirectionVector3D::Forward(), 3, SHNORM_N3D, gains ) && ok;
    ok = CAmbisonicFormat::ConvertBlock( block, AMBIORDER_ACN, SHNORM_SN3D, block, AMBIORDER_FUMA, SHNORM_FUMA, 3, frames ) && ok;
    ok = CAmbisonicFormat::ConvertChannels( block, AMBIORDER_FUMA, SHNORM_FUMA, block, AMBIORDER_ACN, SHNORM_N3D, 3 ) && ok;
    return ( ok ? 1.0f : 0.0f ) + gains[ 3 ] + block[ 1 ];
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAmbisonicsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Ambisonics" );

    GUCEF_TESTFW_TESTCASE( "Ambisonics: channel counts, supported combinations and the FuMa channel order" )
    try
    {
        ASSERT_TRUE( 1 == CAmbisonicFormat::GetChannelCount( 0 ) && 4 == CAmbisonicFormat::GetChannelCount( 1 ) && 16 == CAmbisonicFormat::GetChannelCount( 3 ) );
        ASSERT_TRUE( CAmbisonicFormat::IsSupported( AMBIORDER_ACN, SHNORM_SN3D, 15 ) && !CAmbisonicFormat::IsSupported( AMBIORDER_ACN, SHNORM_SN3D, 16 ) );
        ASSERT_TRUE( CAmbisonicFormat::IsSupported( AMBIORDER_FUMA, SHNORM_FUMA, 3 ) && !CAmbisonicFormat::IsSupported( AMBIORDER_FUMA, SHNORM_SN3D, 4 ) );
        ASSERT_TRUE( !CAmbisonicFormat::IsSupported( AMBIORDER_ACN, SHNORM_FUMA, 4 ) && CAmbisonicFormat::IsSupported( AMBIORDER_ACN, SHNORM_N3D, 4 ) );
        // FuMa W X Y Z R S T U V K L M N O P Q in ACN
        const UInt32 expected[ 16 ] = { 0, 3, 1, 2, 6, 7, 5, 8, 4, 12, 13, 11, 14, 10, 15, 9 };
        for ( UInt32 c=0; c<16; ++c )
        {
            ASSERT_TRUE( expected[ c ] == CAmbisonicFormat::GetAcnIndex( c, AMBIORDER_FUMA ) );
            ASSERT_TRUE( c == CAmbisonicFormat::GetChannelIndex( expected[ c ], AMBIORDER_FUMA ) );
            ASSERT_TRUE( c == CAmbisonicFormat::GetAcnIndex( c, AMBIORDER_ACN ) && c == CAmbisonicFormat::GetChannelIndex( c, AMBIORDER_ACN ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Ambisonics: FuMa W is SN3D W / sqrt( 2 ), first-order FuMa X, Y, Z are ACN 3, 1, 2" )
    try
    {
        // a first-order ACN/SN3D frame (W, Y, Z, X) to FuMa
        const Float64 acn[ 4 ] = { 1.0, 0.25, -0.5, 0.75 };
        Float64 fuma[ 4 ];
        ASSERT_TRUE( CAmbisonicFormat::ConvertChannels( acn, AMBIORDER_ACN, SHNORM_SN3D, fuma, AMBIORDER_FUMA, SHNORM_FUMA, 1 ) );
        ASSERT_TRUE( fabs( fuma[ 0 ] - 1.0 / sqrt( 2.0 ) ) < 1e-15 && 0.75 == fuma[ 1 ] && 0.25 == fuma[ 2 ] && -0.5 == fuma[ 3 ] );
        // the encoders agree: FuMa W = SN3D W / sqrt( 2 ) for any direction
        Float64 gainsSn3d[ 4 ];
        Float64 gainsFuma[ 4 ];
        ASSERT_TRUE( CAmbisonicFormat::EncodeDirection( TFloat64Radian( 0.7 ), TFloat64Radian( -0.3 ), 1, SHNORM_SN3D, gainsSn3d ) );
        ASSERT_TRUE( CAmbisonicFormat::EncodeDirection( TFloat64Radian( 0.7 ), TFloat64Radian( -0.3 ), 1, SHNORM_FUMA, gainsFuma ) );
        ASSERT_TRUE( 1.0 == gainsSn3d[ 0 ] && fabs( gainsFuma[ 0 ] - 1.0 / sqrt( 2.0 ) ) < 1e-15 );
        for ( UInt32 k=1; k<4; ++k )
            ASSERT_TRUE( gainsSn3d[ k ] == gainsFuma[ k ] );

        // the FuMa weights have a maximum magnitude of 1 per channel (W 1 / sqrt( 2 )) up to order 3: an independent
        // check of the weight table by sampling the sphere
        Float64 largest[ 16 ];
        memset( largest, 0, sizeof( largest ) );
        const UInt32 points = 20000;
        Float64 gains[ 16 ];
        for ( UInt32 p=0; p<points; ++p )
        {
            const TFloat64Vector3D v = FibonacciSphere< Float64 >( p, points );
            CTSphericalHarmonicsBasis< Float64 >::Evaluate( v.x, v.y, v.z, 3, SHNORM_FUMA, gains );
            for ( UInt32 k=0; k<16; ++k )
                largest[ k ] = fabs( gains[ k ] ) > largest[ k ] ? fabs( gains[ k ] ) : largest[ k ];
        }
        ASSERT_TRUE( fabs( largest[ 0 ] - 1.0 / sqrt( 2.0 ) ) < 1e-12 );
        for ( UInt32 k=1; k<16; ++k )
        {
            ASSERT_TRUE( largest[ k ] <= 1.0 + 1e-12 && largest[ k ] > 0.995 );
            if ( !( largest[ k ] <= 1.0 + 1e-12 && largest[ k ] > 0.995 ) )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Ambisonics: FuMa ACN " << k << " has maximum " << largest[ k ] );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Ambisonics: ACN/SN3D to ACN/N3D to FuMa and back round trips up to order 3, in place and in blocks" )
    try
    {
        const UInt64 seed = 0x4B80;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 order=0; order<=3; ++order )
        {
            const UInt32 count = CAmbisonicFormat::GetChannelCount( order );
            for ( UInt32 i=0; i<50; ++i )
            {
                Float64 original[ 16 ];
                Float32 original32[ 16 ];
                for ( UInt32 c=0; c<count; ++c )
                {
                    const Float64 value = generator.NextFloat64( -1.0, 1.0 );
                    original[ c ] = value;
                    original32[ c ] = (Float32) value;
                }
                Float64 n3d[ 16 ];
                Float64 fuma[ 16 ];
                Float64 back[ 16 ];
                bool ok = CAmbisonicFormat::ConvertChannels( original, AMBIORDER_ACN, SHNORM_SN3D, n3d, AMBIORDER_ACN, SHNORM_N3D, order );
                ok = CAmbisonicFormat::ConvertChannels( n3d, AMBIORDER_ACN, SHNORM_N3D, fuma, AMBIORDER_FUMA, SHNORM_FUMA, order ) && ok;
                ok = CAmbisonicFormat::ConvertChannels( fuma, AMBIORDER_FUMA, SHNORM_FUMA, back, AMBIORDER_ACN, SHNORM_SN3D, order ) && ok;
                if ( !ok )
                    ++failures;
                for ( UInt32 c=0; c<count; ++c )
                {
                    if ( !( fabs( back[ c ] - original[ c ] ) <= 1e-14 ) )
                        ++failures;
                    // N3D = sqrt( 2l + 1 ) SN3D
                    const UInt32 l = CTSphericalHarmonicsBasis< Float64 >::GetDegree( c );
                    if ( !( fabs( n3d[ c ] - sqrt( 2.0 * l + 1.0 ) * original[ c ] ) <= 1e-14 ) )
                        ++failures;
                }
                // Float32, in place
                Float32 inPlace[ 16 ];
                memcpy( inPlace, original32, sizeof( inPlace ) );
                ok = CAmbisonicFormat::ConvertChannels( inPlace, AMBIORDER_ACN, SHNORM_SN3D, inPlace, AMBIORDER_FUMA, SHNORM_FUMA, order );
                ok = CAmbisonicFormat::ConvertChannels( inPlace, AMBIORDER_FUMA, SHNORM_FUMA, inPlace, AMBIORDER_ACN, SHNORM_ORTHONORMAL, order ) && ok;
                ok = CAmbisonicFormat::ConvertChannels( inPlace, AMBIORDER_ACN, SHNORM_ORTHONORMAL, inPlace, AMBIORDER_ACN, SHNORM_SN3D, order ) && ok;
                if ( !ok )
                    ++failures;
                for ( UInt32 c=0; c<count; ++c )
                    if ( !( fabs( inPlace[ c ] - original32[ c ] ) <= 1e-6f ) )
                        ++failures;
            }
            // a block of 3 interleaved frames equals 3 single-frame conversions
            Float64 block[ 48 ];
            Float64 single[ 48 ];
            for ( UInt32 k=0; k<3*count; ++k )
                block[ k ] = (Float64) k * 0.125 - 1.0;
            for ( UInt32 f=0; f<3; ++f )
                CAmbisonicFormat::ConvertChannels( block + f * count, AMBIORDER_ACN, SHNORM_N3D, single + f * count, AMBIORDER_FUMA, SHNORM_FUMA, order );
            if ( !CAmbisonicFormat::ConvertBlock( block, AMBIORDER_ACN, SHNORM_N3D, block, AMBIORDER_FUMA, SHNORM_FUMA, order, 3 ) )
                ++failures;
            for ( UInt32 k=0; k<3*count; ++k )
                if ( !( block[ k ] == single[ k ] ) )
                    ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Ambisonics: " << failures << " conversion checks failed, seed " << seed );

        // higher orders in ACN between the full-sphere normalisations
        Float32 high[ 64 ];
        for ( UInt32 k=0; k<64; ++k )
            high[ k ] = 1.0f;
        ASSERT_TRUE( CAmbisonicFormat::ConvertChannels( high, AMBIORDER_ACN, SHNORM_SN3D, high, AMBIORDER_ACN, SHNORM_N3D, 7 ) );
        ASSERT_TRUE( fabs( high[ 63 ] - sqrtf( 15.0f ) ) < 1e-6f && 1.0f == high[ 0 ] );
        #ifndef GUCEF_MATH_DEBUG_MODE
        // unsupported: FuMa beyond order 3 (asserts in debug builds), the output stays unchanged
        Float32 keep[ 25 ];
        for ( UInt32 k=0; k<25; ++k )
            keep[ k ] = 2.0f;
        ASSERT_FALSE( CAmbisonicFormat::ConvertChannels( high, AMBIORDER_ACN, SHNORM_SN3D, keep, AMBIORDER_FUMA, SHNORM_FUMA, 4 ) );
        ASSERT_FALSE( CAmbisonicFormat::EncodeDirection( TFloat32Radian( 0.0f ), TFloat32Radian( 0.0f ), 4, SHNORM_FUMA, keep ) );
        ASSERT_FALSE( CAmbisonicFormat::EncodeDirection( TFloat32Radian( 0.0f ), TFloat32Radian( 0.0f ), 16, SHNORM_SN3D, keep ) );
        ASSERT_TRUE( 2.0f == keep[ 0 ] && 2.0f == keep[ 24 ] );
        #endif
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Ambisonics: EncodeDirection takes the listener azimuth (positive right): +90 degrees gives a negative ACN 1 (Y)" )
    try
    {
        Float64 gains[ 16 ];
        // straight ahead: W = 1, X = 1 (SN3D)
        ASSERT_TRUE( CAmbisonicFormat::EncodeDirection( TFloat64Radian( 0.0 ), TFloat64Radian( 0.0 ), 3, SHNORM_SN3D, gains ) );
        ASSERT_TRUE( 1.0 == gains[ 0 ] && 1.0 == gains[ 3 ] && 0.0 == gains[ 1 ] && 0.0 == gains[ 2 ] );
        // to the right: Y = -1
        ASSERT_TRUE( CAmbisonicFormat::EncodeDirection( TFloat64Radian( g_pi / 2.0 ), TFloat64Radian( 0.0 ), 3, SHNORM_SN3D, gains ) );
        ASSERT_TRUE( fabs( gains[ 1 ] + 1.0 ) < 1e-15 && fabs( gains[ 3 ] ) < 1e-15 && gains[ 1 ] < 0.0 );
        // to the left: Y = +1; above: Z = 1
        ASSERT_TRUE( CAmbisonicFormat::EncodeDirection( TFloat64Radian( -g_pi / 2.0 ), TFloat64Radian( 0.0 ), 1, SHNORM_SN3D, gains ) && fabs( gains[ 1 ] - 1.0 ) < 1e-15 );
        ASSERT_TRUE( CAmbisonicFormat::EncodeDirection( TFloat64Radian( 0.3 ), TFloat64Radian( g_pi / 2.0 ), 1, SHNORM_SN3D, gains ) && fabs( gains[ 2 ] - 1.0 ) < 1e-15 );
        // Float32 and the degree overload of the angle
        Float32 gains32[ 4 ];
        ASSERT_TRUE( CAmbisonicFormat::EncodeDirection( TFloat32Radian( TFloat32Degree( 90.0f ) ), TFloat32Radian( 0.0f ), 1, SHNORM_N3D, gains32 ) );
        ASSERT_TRUE( gains32[ 1 ] < 0.0f && fabs( gains32[ 1 ] + sqrtf( 3.0f ) ) < 1e-6f );

        // the listener-space direction (x right, y up, -z forward) gives the same gains as the azimuth form
        const UInt64 seed = 0x4B81;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<200; ++i )
        {
            const Float64 azimuth = generator.NextFloat64( -g_pi, g_pi );
            const Float64 elevation = generator.NextFloat64( -g_pi / 2.0, g_pi / 2.0 );
            // listener azimuth positive right: right = +x, forward = -z
            const TFloat64Vector3D listener( cos( elevation ) * sin( azimuth ), sin( elevation ), -cos( elevation ) * cos( azimuth ) );
            Float64 fromAngles[ 16 ];
            Float64 fromDirection[ 16 ];
            const bool ok = CAmbisonicFormat::EncodeDirection( TFloat64Radian( azimuth ), TFloat64Radian( elevation ), 3, SHNORM_N3D, fromAngles ) &&
                            CAmbisonicFormat::EncodeListenerDirection( TFloat64DirectionVector3D::FromNormalized( listener ), 3, SHNORM_N3D, fromDirection );
            if ( !ok )
                ++failures;
            for ( UInt32 k=0; k<16; ++k )
                if ( !( fabs( fromAngles[ k ] - fromDirection[ k ] ) <= 1e-13 ) )
                    ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Ambisonics: " << failures << " direction encodings differ, seed " << seed );
        ASSERT_TRUE( CAmbisonicFormat::EncodeListenerDirection( TFloat64DirectionVector3D( TFloat64Vector3D( 1, 0, 0 ) ), 1, SHNORM_SN3D, gains ) && fabs( gains[ 1 ] + 1.0 ) < 1e-15 );
        ASSERT_TRUE( CAmbisonicFormat::EncodeListenerDirection( TFloat64DirectionVector3D( TFloat64Vector3D( 0, 0, -1 ) ), 1, SHNORM_SN3D, gains ) && fabs( gains[ 3 ] - 1.0 ) < 1e-15 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Ambisonics: conversion and encoding run in a realtime context" )
    try
    {
        Float32 block[ 32 ];
        for ( UInt32 k=0; k<32; ++k )
            block[ k ] = 0.01f * (Float32) k;
        Float32 result = RealtimeAmbisonicWork( block, 2 );
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( result ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Ambisonics: realtime-safe and right-sized operations do not allocate" )
    GUCEF_TESTFW_SKIP_IF( !MathAllocationCounter::IsAvailable(), "no allocation counter with a sanitizer that replaces the allocator (the RealtimeSanitizer build proves the same in a realtime context)" );
    try
    {
        // the counter counts: one probe allocation
        MathAllocationCounter::StartCounting();
        MathAllocationCounter::AllocateProbe();
        ASSERT_TRUE( 1 == MathAllocationCounter::StopCounting() );
        Float32 block[ 32 ];
        for ( UInt32 k=0; k<32; ++k )
            block[ k ] = 0.01f * (Float32) k;
        MathAllocationCounter::StartCounting();
        Float32 result = RealtimeAmbisonicWork( block, 2 );
        const UInt32 allocations = MathAllocationCounter::StopCounting();
        ASSERT_TRUE( 0 == allocations && CTNumericTraits< Float32 >::IsFinite( result ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
