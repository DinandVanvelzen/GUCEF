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
#include <limits>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_COCTAHEDRALENCODING_H
#include "gucefMATH_COctahedralEncoding.h"
#define GUCEF_MATH_COCTAHEDRALENCODING_H
#endif /* GUCEF_MATH_COCTAHEDRALENCODING_H ? */

#include "TestRandomStatistics.h"
#include "TestOctahedral.h"

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

/** The angle in degrees between two unit vectors, from the cross product so that tiny angles stay accurate */
static Float64
AngleDegrees( const TFloat64Vector3D& a, const TFloat64Vector3D& b )
{
    const TFloat64Vector3D cross = a.Cross( b );
    return atan2( cross.GetLength(), a.Dot( b ) ) * 180.0 / 3.14159265358979323846;
}

/** A uniformly distributed unit vector from rejection sampling in the cube: independent of the code under test */
static TFloat64Vector3D
RandomDirection( MathReference::CSeededSequence& sequence )
{
    for ( ; ; )
    {
        const Float64 x = (Float64) ( sequence.NextUInt64() >> 11 ) * ldexp( 2.0, -53 ) - 1.0;
        const Float64 y = (Float64) ( sequence.NextUInt64() >> 11 ) * ldexp( 2.0, -53 ) - 1.0;
        const Float64 z = (Float64) ( sequence.NextUInt64() >> 11 ) * ldexp( 2.0, -53 ) - 1.0;
        const Float64 r2 = x * x + y * y + z * z;
        if ( r2 > 1e-4 && r2 <= 1.0 )
        {
            const Float64 r = sqrt( r2 );
            return TFloat64Vector3D( x / r, y / r, z / r );
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformOctahedralTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Octahedral TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Octahedral" );

    GUCEF_TESTFW_TESTCASE( "Octahedral: the six axis directions encode and decode exactly, with every encoder and both kinds" )
        const Float64 axes[ 6 ][ 3 ] = { { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };
        for ( UInt32 i=0; i<6; ++i )
        {
            const TFloat64Vector3D v( axes[ i ][ 0 ], axes[ i ][ 1 ], axes[ i ][ 2 ] );
            const TFloat32Vector3D v32( (Float32) axes[ i ][ 0 ], (Float32) axes[ i ][ 1 ], (Float32) axes[ i ][ 2 ] );
            const UInt16 codes16[ 4 ] = { COctahedralEncoding::Encode16( v ), COctahedralEncoding::EncodePrecise16( v ), COctahedralEncoding::Encode16( v32 ), COctahedralEncoding::EncodePrecise16( v32 ) };
            const UInt32 codes32[ 4 ] = { COctahedralEncoding::Encode32( v ), COctahedralEncoding::EncodePrecise32( v ), COctahedralEncoding::Encode32( v32 ), COctahedralEncoding::EncodePrecise32( v32 ) };
            for ( UInt32 k=0; k<4; ++k )
            {
                TFloat64Vector3D d16, d32;
                COctahedralEncoding::Decode16( codes16[ k ], d16 );
                COctahedralEncoding::Decode32( codes32[ k ], d32 );
                ASSERT_TRUE( d16.x == v.x && d16.y == v.y && d16.z == v.z );
                ASSERT_TRUE( d32.x == v.x && d32.y == v.y && d32.z == v.z );
                TFloat32Vector3D f16, f32;
                COctahedralEncoding::Decode16( codes16[ k ], f16 );
                COctahedralEncoding::Decode32( codes32[ k ], f32 );
                ASSERT_TRUE( f16.x == v32.x && f16.y == v32.y && f16.z == v32.z && f32.x == v32.x && f32.y == v32.y && f32.z == v32.z );
            }
        }
        // the code of +X: 127 in the upper byte, 0 in the lower
        ASSERT_TRUE( 0x7F00 == COctahedralEncoding::Encode16( TFloat64Vector3D( 1.0, 0.0, 0.0 ) ) );
        ASSERT_TRUE( 0x7FFF0000U == COctahedralEncoding::Encode32( TFloat64Vector3D( 1.0, 0.0, 0.0 ) ) );
        // -Z folds to the corner ( 1, 1 ) in the positive direction of both components
        ASSERT_TRUE( 0x7F7F == COctahedralEncoding::Encode16( TFloat64Vector3D( 0.0, 0.0, -1.0 ) ) );
        // the code of -X has -127 as two's complement: 0x81
        ASSERT_TRUE( 0x8100 == COctahedralEncoding::Encode16( TFloat64Vector3D( -1.0, 0.0, 0.0 ) ) );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Octahedral: the largest angular error over 10^6 seeded directions is at most 1.0 degree for Encode16 and 0.005 degrees for Encode32" )
        const UInt64 seed = 2024;
        MathReference::CSeededSequence sequence( seed );
        Float64 worst16 = 0.0, worst32 = 0.0, worstPrecise16 = 0.0, worstPrecise32 = 0.0;
        UInt32 preciseWorse = 0;
        Float64 worstRoundTrip = 0.0;
        for ( UInt32 i=0; i<1000000; ++i )
        {
            const TFloat64Vector3D v = RandomDirection( sequence );
            TFloat64Vector3D a, b, c, d;
            COctahedralEncoding::Decode16( COctahedralEncoding::Encode16( v ), a );
            COctahedralEncoding::Decode16( COctahedralEncoding::EncodePrecise16( v ), b );
            COctahedralEncoding::Decode32( COctahedralEncoding::Encode32( v ), c );
            COctahedralEncoding::Decode32( COctahedralEncoding::EncodePrecise32( v ), d );
            const Float64 e16 = AngleDegrees( v, a ), p16 = AngleDegrees( v, b ), e32 = AngleDegrees( v, c ), p32 = AngleDegrees( v, d );
            worst16 = fmax( worst16, e16 );
            worst32 = fmax( worst32, e32 );
            worstPrecise16 = fmax( worstPrecise16, p16 );
            worstPrecise32 = fmax( worstPrecise32, p32 );
            // never worse than the plain encoding for any sample ( up to the rounding of the angle )
            if ( p16 > e16 + 1e-9 || p32 > e32 + 1e-9 ) ++preciseWorse;
            // the decoded vectors have unit length
            worstRoundTrip = fmax( worstRoundTrip, fmax( fabs( a.GetLength() - 1.0 ), fabs( c.GetLength() - 1.0 ) ) );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Octahedral: worst angular error Encode16 " << worst16 << ", Precise16 " << worstPrecise16 << ", Encode32 " << worst32 << ", Precise32 " << worstPrecise32 << " degrees" );
        ASSERT_SEEDED( worst16 <= 1.0 && worst32 <= 0.005, seed );
        ASSERT_SEEDED( worstPrecise16 <= worst16 && worstPrecise32 <= worst32, seed );
        ASSERT_SEEDED( 0 == preciseWorse, seed );
        ASSERT_SEEDED( worstRoundTrip < 1e-12, seed );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Octahedral: Float32 and Float64 inputs of the same value give the same codes, the input need not be normalised" )
        MathReference::CSeededSequence sequence( 31 );
        for ( UInt32 i=0; i<20000; ++i )
        {
            const TFloat64Vector3D v = RandomDirection( sequence );
            const TFloat32Vector3D f( (Float32) v.x, (Float32) v.y, (Float32) v.z );
            const TFloat64Vector3D widened( f.x, f.y, f.z );
            ASSERT_TRUE( COctahedralEncoding::Encode16( f ) == COctahedralEncoding::Encode16( widened ) );
            ASSERT_TRUE( COctahedralEncoding::EncodePrecise32( f ) == COctahedralEncoding::EncodePrecise32( widened ) );
            // scaling does not change the direction ( powers of two scale exactly )
            const TFloat64Vector3D scaled( v.x * 1024.0, v.y * 1024.0, v.z * 1024.0 );
            ASSERT_TRUE( COctahedralEncoding::Encode32( v ) == COctahedralEncoding::Encode32( scaled ) );
            const TFloat64Vector3D tiny( v.x * ldexp( 1.0, -500 ), v.y * ldexp( 1.0, -500 ), v.z * ldexp( 1.0, -500 ) );
            ASSERT_TRUE( COctahedralEncoding::Encode16( v ) == COctahedralEncoding::Encode16( tiny ) );
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Octahedral: Project and Unproject are inverse maps with components in [-1, 1], the lower hemisphere is folded" )
        MathReference::CSeededSequence sequence( 32 );
        Float64 worst = 0.0;
        bool inRange = true;
        for ( UInt32 i=0; i<100000; ++i )
        {
            const TFloat64Vector3D v = RandomDirection( sequence );
            Float64 u, w;
            COctahedralEncoding::Project( v, u, w );
            if ( fabs( u ) > 1.0 || fabs( w ) > 1.0 ) inRange = false;
            // the octahedron: |u| + |w| <= 1 above the equator, >= 1 below it
            if ( ( v.z >= 0.0 ) != ( fabs( u ) + fabs( w ) <= 1.0 + 1e-12 ) && fabs( v.z ) > 1e-9 ) inRange = false;
            TFloat64Vector3D back;
            COctahedralEncoding::Unproject( u, w, back );
            worst = fmax( worst, ( back - v ).GetLength() );
            Float32 u32, w32;
            COctahedralEncoding::Project( TFloat32Vector3D( (Float32) v.x, (Float32) v.y, (Float32) v.z ), u32, w32 );
            TFloat32Vector3D back32;
            COctahedralEncoding::Unproject( u32, w32, back32 );
            if ( ( TFloat64Vector3D( back32.x, back32.y, back32.z ) - v ).GetLength() > 1e-5 ) inRange = false;
        }
        ASSERT_TRUE( inRange );
        ASSERT_TRUE( worst < 1e-12 );
        // the standard points
        Float64 u, w;
        COctahedralEncoding::Project( TFloat64Vector3D( 0.0, 0.0, 1.0 ), u, w );
        ASSERT_TRUE( 0.0 == u && 0.0 == w );
        COctahedralEncoding::Project( TFloat64Vector3D( 1.0, 1.0, 0.0 ), u, w );
        ASSERT_TRUE( 0.5 == u && 0.5 == w );
        COctahedralEncoding::Project( TFloat64Vector3D( 0.0, 0.0, -1.0 ), u, w );
        ASSERT_TRUE( 1.0 == u && 1.0 == w );
        // out of range components are limited
        TFloat64Vector3D limited;
        COctahedralEncoding::Unproject( 5.0, 0.0, limited );
        ASSERT_TRUE( 1.0 == limited.x && 0.0 == limited.y && 0.0 == limited.z );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Octahedral: degenerate input encodes as +Z, the unused code decodes like the neighbouring one" )
        const Float64 nan = std::numeric_limits< Float64 >::quiet_NaN();
        const Float64 inf = std::numeric_limits< Float64 >::infinity();
        const TFloat64Vector3D bad[ 3 ] = { TFloat64Vector3D( 0.0, 0.0, 0.0 ), TFloat64Vector3D( nan, 1.0, 0.0 ), TFloat64Vector3D( inf, 0.0, 1.0 ) };
        for ( UInt32 i=0; i<3; ++i )
        {
            TFloat64Vector3D d;
            COctahedralEncoding::Decode16( COctahedralEncoding::Encode16( bad[ i ] ), d );
            ASSERT_TRUE( 0.0 == d.x && 0.0 == d.y && 1.0 == d.z );
            COctahedralEncoding::Decode32( COctahedralEncoding::EncodePrecise32( bad[ i ] ), d );
            ASSERT_TRUE( 0.0 == d.x && 0.0 == d.y && 1.0 == d.z );
        }
        // 0x8080 holds -128 in both bytes: decodes like -127
        TFloat64Vector3D a, b;
        COctahedralEncoding::Decode16( 0x8080, a );
        COctahedralEncoding::Decode16( 0x8181, b );
        ASSERT_TRUE( a.x == b.x && a.y == b.y && a.z == b.z );
        COctahedralEncoding::Decode32( 0x80008000U, a );
        COctahedralEncoding::Decode32( 0x80018001U, b );
        ASSERT_TRUE( a.x == b.x && a.y == b.y && a.z == b.z );
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
