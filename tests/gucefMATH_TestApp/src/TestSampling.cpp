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

#ifndef GUCEF_MATH_SAMPLEWARPING_H
#include "gucefMATH_SampleWarping.h"
#define GUCEF_MATH_SAMPLEWARPING_H
#endif /* GUCEF_MATH_SAMPLEWARPING_H ? */

#ifndef GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H
#include "gucefMATH_LowDiscrepancySequences.h"
#define GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H
#endif /* GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H ? */

#include "TestRandomStatistics.h"
#include "TestSampling.h"

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
using namespace RandomStatistics;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The largest number of points in one cell of the elementary intervals 2^-a x 2^-b of the first 2^m points of two Sobol dimensions */
static UInt32
MaxPointsPerElementaryInterval( UInt32 dimensionA, UInt32 dimensionB, UInt32 m )
{
    UInt32 worst = 0;
    const UInt32 count = 1U << m;
    for ( UInt32 a=0; a<=m; ++a )
    {
        const UInt32 b = m - a;
        const UInt32 columns = 1U << a;
        const UInt32 rows = 1U << b;
        GUCEF::vector< UInt32 > cells( (size_t) columns * rows, 0 );
        for ( UInt32 i=0; i<count; ++i )
        {
            const UInt32 column = (UInt32) ( Sobol< Float64 >( i, dimensionA ) * columns );
            const UInt32 row = (UInt32) ( Sobol< Float64 >( i, dimensionB ) * rows );
            ++cells[ (size_t) column * rows + row ];
        }
        for ( size_t c=0; c<cells.size(); ++c )
            if ( cells[ c ] > worst ) worst = cells[ c ];
    }
    return worst;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSamplingTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Sampling TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Sampling" );

    GUCEF_TESTFW_TESTCASE( "Sampling: Halton( 1..5, 2 ) is 1/2, 1/4, 3/4, 1/8, 5/8 and Halton( 1..3, 3 ) is 1/3, 2/3, 1/9" )
        const Float64 base2[ 5 ] = { 0.5, 0.25, 0.75, 0.125, 0.625 };
        for ( UInt32 i=0; i<5; ++i )
            ASSERT_TRUE( base2[ i ] == Halton< Float64 >( i + 1, 2 ) && (Float32) base2[ i ] == Halton< Float32 >( i + 1, 2 ) );
        ASSERT_TRUE( 1.0 / 3.0 == Halton< Float64 >( 1, 3 ) && 2.0 / 3.0 == Halton< Float64 >( 2, 3 ) && 1.0 / 9.0 == Halton< Float64 >( 3, 3 ) );
        ASSERT_TRUE( 4.0 / 9.0 == Halton< Float64 >( 4, 3 ) && 7.0 / 9.0 == Halton< Float64 >( 5, 3 ) );
        ASSERT_TRUE( 0.0 == Halton< Float64 >( 0, 2 ) && 0.0 == Halton< Float64 >( 5, 1 ) && 0.0 == Halton< Float64 >( 5, 0 ) );
        // base 5: 1/5, 2/5, 3/5, 4/5, 1/25
        ASSERT_TRUE( 1.0 / 5.0 == Halton< Float64 >( 1, 5 ) && 4.0 / 5.0 == Halton< Float64 >( 4, 5 ) && 1.0 / 25.0 == Halton< Float64 >( 5, 5 ) );
        // the base 2 form agrees with the bit reversal and stays below 1 in both kinds, also for the largest index
        for ( UInt32 i=1; i<5000; ++i )
            ASSERT_TRUE( Halton< Float64 >( i, 2 ) == RadicalInverseBase2< Float64 >( i ) );
        ASSERT_TRUE( RadicalInverseBase2< Float32 >( 0xFFFFFFFFU ) < 1.0f && RadicalInverseBase2< Float64 >( 0xFFFFFFFFU ) < 1.0 );
        ASSERT_TRUE( Halton< Float32 >( 0xFFFFFFFFU, 2 ) < 1.0f && 0.0f == RadicalInverseBase2< Float32 >( 0 ) );
        // large bases do not overflow
        const Float64 largeBase = Halton< Float64 >( 0xFFFFFFFEU, 0xFFFFFFFFU );
        ASSERT_TRUE( largeBase > 0.0 && largeBase < 1.0 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sampling: Sobol dimension 0 equals the base 2 radical inverse, dimensions 1 to 3 stratify and form nets" )
        for ( UInt32 i=0; i<5000; ++i )
            ASSERT_TRUE( Sobol< Float64 >( i, 0 ) == RadicalInverseBase2< Float64 >( i ) );

        // independent reference values of the Joe-Kuo dimensions ( direct recurrence of the direction numbers in Python, natural index order )
        const Float64 dim1[ 16 ] = { 0.5, 0.75, 0.25, 0.625, 0.125, 0.375, 0.875, 0.9375, 0.4375, 0.1875, 0.6875, 0.3125, 0.8125, 0.5625, 0.0625, 0.53125 };
        const Float64 dim2[ 16 ] = { 0.5, 0.75, 0.25, 0.375, 0.875, 0.625, 0.125, 0.5625, 0.0625, 0.3125, 0.8125, 0.9375, 0.4375, 0.1875, 0.6875, 0.90625 };
        const Float64 dim3[ 16 ] = { 0.5, 0.75, 0.25, 0.125, 0.625, 0.875, 0.375, 0.3125, 0.8125, 0.5625, 0.0625, 0.4375, 0.9375, 0.6875, 0.1875, 0.96875 };
        for ( UInt32 i=0; i<16; ++i )
        {
            ASSERT_TRUE( dim1[ i ] == Sobol< Float64 >( i + 1, 1 ) && (Float32) dim1[ i ] == Sobol< Float32 >( i + 1, 1 ) );
            ASSERT_TRUE( dim2[ i ] == Sobol< Float64 >( i + 1, 2 ) );
            ASSERT_TRUE( dim3[ i ] == Sobol< Float64 >( i + 1, 3 ) );
        }
        ASSERT_TRUE( 0.0 == Sobol< Float64 >( 0, 1 ) && 0.0 == Sobol< Float64 >( 7, 4 ) && 0.0 == Sobol< Float64 >( 7, 99 ) );

        // every dimension alone hits each of the 2^m cells of width 2^-m exactly once in the first 2^m points
        for ( UInt32 d=0; d<4; ++d )
        {
            bool seen[ 1024 ];
            for ( UInt32 i=0; i<1024; ++i ) seen[ i ] = false;
            for ( UInt32 i=0; i<1024; ++i )
                seen[ (UInt32) ( Sobol< Float64 >( i, d ) * 1024.0 ) ] = true;
            bool all = true;
            for ( UInt32 i=0; i<1024; ++i ) if ( !seen[ i ] ) all = false;
            ASSERT_TRUE( all );
        }
        // dimensions 0 and 1 form a ( 0, m, 2 ) net: exactly one point in every elementary interval of volume 2^-m
        for ( UInt32 m=1; m<=10; ++m )
            ASSERT_TRUE( 1 == MaxPointsPerElementaryInterval( 0, 1, m ) );
        // the other pairs have a small quality parameter t: at most 2^t points per elementary interval
        for ( UInt32 a=0; a<4; ++a )
            for ( UInt32 b=a+1; b<4; ++b )
                ASSERT_TRUE( MaxPointsPerElementaryInterval( a, b, 8 ) <= 4 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sampling: Hammersley2D pairs i / n with the radical inverse" )
        for ( UInt32 i=0; i<16; ++i )
        {
            const TFloat64Vector2D p = Hammersley2D< Float64 >( i, 16 );
            ASSERT_TRUE( p.x == i / 16.0 && p.y == RadicalInverseBase2< Float64 >( i ) );
        }
        const TFloat32Vector2D q = Hammersley2D< Float32 >( 3, 8 );
        ASSERT_TRUE( 0.375f == q.x && 0.75f == q.y );
        const TFloat64Vector2D none = Hammersley2D< Float64 >( 3, 0 );
        ASSERT_TRUE( 0.0 == none.x && 0.0 == none.y );
        // 2^m points, one in each 2^-a x 2^-( m - a ) cell: the Hammersley set is a ( 0, m, 2 ) net too
        bool unique = true;
        for ( UInt32 a=0; a<=6; ++a )
        {
            const UInt32 columns = 1U << a, rows = 1U << ( 6 - a );
            GUCEF::vector< UInt32 > cells( (size_t) columns * rows, 0 );
            for ( UInt32 i=0; i<64; ++i )
            {
                const TFloat64Vector2D p = Hammersley2D< Float64 >( i, 64 );
                ++cells[ (size_t) ( (UInt32) ( p.x * columns ) ) * rows + (UInt32) ( p.y * rows ) ];
            }
            for ( size_t c=0; c<cells.size(); ++c ) if ( cells[ c ] != 1 ) unique = false;
        }
        ASSERT_TRUE( unique );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sampling: FibonacciSphere points have unit length and a minimum distance above 3 / sqrt( n ) for n = 1000" )
        const UInt32 n = 1000;
        GUCEF::vector< TFloat64Vector3D > points;
        for ( UInt32 i=0; i<n; ++i )
        {
            const TFloat64Vector3D p = FibonacciSphere< Float64 >( i, n );
            ASSERT_TRUE( fabs( p.x * p.x + p.y * p.y + p.z * p.z - 1.0 ) < 1e-12 );
            const TFloat32Vector3D p32 = FibonacciSphere< Float32 >( i, n );
            ASSERT_TRUE( fabs( (Float64) p32.x * p32.x + (Float64) p32.y * p32.y + (Float64) p32.z * p32.z - 1.0 ) < 1e-6 );
            points.push_back( p );
        }
        // the half offset construction: z of the first and last point are 1 - 1/n and -( 1 - 1/n )
        ASSERT_TRUE( fabs( points[ 0 ].z - ( 1.0 - 1.0 / n ) ) < 1e-14 && fabs( points[ n - 1 ].z + ( 1.0 - 1.0 / n ) ) < 1e-14 );
        Float64 minimum = 1e9;
        for ( UInt32 i=0; i<n; ++i )
            for ( UInt32 j=i+1; j<n; ++j )
            {
                const TFloat64Vector3D d = points[ i ] - points[ j ];
                const Float64 distance = sqrt( d.x * d.x + d.y * d.y + d.z * d.z );
                if ( distance < minimum ) minimum = distance;
            }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Sampling: Fibonacci sphere n = 1000, minimum distance times sqrt( n ) is " << minimum * sqrt( (Float64) n ) );
        ASSERT_TRUE( minimum > 3.0 / sqrt( (Float64) n ) );
        // equal areas: z is uniform, so the bands of equal height hold equal numbers of points
        double bins[ 10 ];
        memset( bins, 0, sizeof( bins ) );
        for ( UInt32 i=0; i<n; ++i )
        {
            UInt32 bin = (UInt32) ( ( points[ i ].z + 1.0 ) * 5.0 );
            bins[ bin > 9 ? 9 : bin ] += 1.0;
        }
        for ( UInt32 i=0; i<10; ++i )
            ASSERT_TRUE( fabs( bins[ i ] - 100.0 ) <= 1.0 );
        // index wraps, count 0 gives the pole
        const TFloat64Vector3D wrapped = FibonacciSphere< Float64 >( n + 5, n );
        ASSERT_TRUE( wrapped.x == FibonacciSphere< Float64 >( 5, n ).x );
        ASSERT_TRUE( 1.0 == FibonacciSphere< Float64 >( 3, 0 ).z );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sampling: disk warps map the unit square onto the unit disk with equal areas" )
        const UInt64 seed = 2718;
        MathReference::CSeededSequence generator( seed );
        const UInt32 count = 1000000;
        double rings[ 10 ], quadrants[ 4 ], ringsConcentric[ 10 ];
        memset( rings, 0, sizeof( rings ) );
        memset( ringsConcentric, 0, sizeof( ringsConcentric ) );
        memset( quadrants, 0, sizeof( quadrants ) );
        bool inside = true;
        for ( UInt32 i=0; i<count; ++i )
        {
            const Float64 u = (Float64) ( generator.NextUInt64() >> 11 ) * ldexp( 1.0, -53 );
            const Float64 v = (Float64) ( generator.NextUInt64() >> 11 ) * ldexp( 1.0, -53 );
            const TFloat64Vector2D a = UniformDisk< Float64 >( u, v );
            const TFloat64Vector2D b = ConcentricDisk< Float64 >( u, v );
            const Float64 ra = a.x * a.x + a.y * a.y, rb = b.x * b.x + b.y * b.y;
            if ( ra > 1.0 + 1e-12 || rb > 1.0 + 1e-12 ) inside = false;
            UInt32 binA = (UInt32) ( ra * 10.0 ), binB = (UInt32) ( rb * 10.0 );
            rings[ binA > 9 ? 9 : binA ] += 1.0;
            ringsConcentric[ binB > 9 ? 9 : binB ] += 1.0;
            quadrants[ ( b.x > 0 ? 1 : 0 ) + ( b.y > 0 ? 2 : 0 ) ] += 1.0;
        }
        ASSERT_SEEDED( inside, seed );
        ASSERT_SEEDED( ChiSquaredUniform( rings, 10, count ) < ChiSquaredCritical( 9 ), seed );
        ASSERT_SEEDED( ChiSquaredUniform( ringsConcentric, 10, count ) < ChiSquaredCritical( 9 ), seed );
        ASSERT_SEEDED( ChiSquaredUniform( quadrants, 4, count ) < ChiSquaredCritical( 3 ), seed );
        // the concentric map: the centre of the square is the centre of the disk, edge midpoints are on the rim, neighbours stay neighbours
        const TFloat64Vector2D centre = ConcentricDisk< Float64 >( 0.5, 0.5 );
        ASSERT_TRUE( 0.0 == centre.x && 0.0 == centre.y );
        const TFloat64Vector2D right = ConcentricDisk< Float64 >( 1.0, 0.5 );
        ASSERT_TRUE( fabs( right.x - 1.0 ) < 1e-12 && fabs( right.y ) < 1e-12 );
        const TFloat64Vector2D top = ConcentricDisk< Float64 >( 0.5, 1.0 );
        ASSERT_TRUE( fabs( top.y - 1.0 ) < 1e-12 && fabs( top.x ) < 1e-12 );
        // out of range input is clamped, not extrapolated
        const TFloat64Vector2D clamped = UniformDisk< Float64 >( 2.0, -1.0 );
        ASSERT_TRUE( fabs( clamped.x - 1.0 ) < 1e-12 && fabs( clamped.y ) < 1e-12 );
        const TFloat32Vector2D unit32 = UniformCircle< Float32 >( 0.25f );
        ASSERT_TRUE( fabs( unit32.x ) < 1e-6f && fabs( unit32.y - 1.0f ) < 1e-6f );
        const TFloat64Vector2D unit64 = UniformCircle< Float64 >( 0.5 );
        ASSERT_TRUE( fabs( unit64.x + 1.0 ) < 1e-12 && fabs( unit64.y ) < 1e-12 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sampling: sphere, hemisphere, cosine hemisphere and cone warps have the stated densities" )
        const UInt64 seed = 31415;
        MathReference::CSeededSequence generator( seed );
        const UInt32 count = 1000000;
        double sphereZ[ 20 ], hemiZ[ 10 ], coneZ[ 10 ];
        memset( sphereZ, 0, sizeof( sphereZ ) );
        memset( hemiZ, 0, sizeof( hemiZ ) );
        memset( coneZ, 0, sizeof( coneZ ) );
        Float64 cosineSum = 0.0, cosineSquares = 0.0, worst = 0.0;
        const Float64 cosMax = 0.5;
        for ( UInt32 i=0; i<count; ++i )
        {
            const Float64 u = (Float64) ( generator.NextUInt64() >> 11 ) * ldexp( 1.0, -53 );
            const Float64 v = (Float64) ( generator.NextUInt64() >> 11 ) * ldexp( 1.0, -53 );
            const TFloat64Vector3D s = UniformSphere< Float64 >( u, v );
            const TFloat64Vector3D h = UniformHemisphere< Float64 >( u, v );
            const TFloat64Vector3D c = CosineHemisphere< Float64 >( u, v );
            const TFloat64Vector3D k = UniformCone< Float64 >( u, v, cosMax );
            worst = fmax( worst, fabs( s.x * s.x + s.y * s.y + s.z * s.z - 1.0 ) );
            worst = fmax( worst, fabs( h.x * h.x + h.y * h.y + h.z * h.z - 1.0 ) );
            worst = fmax( worst, fabs( c.x * c.x + c.y * c.y + c.z * c.z - 1.0 ) );
            worst = fmax( worst, fabs( k.x * k.x + k.y * k.y + k.z * k.z - 1.0 ) );
            UInt32 bs = (UInt32) ( ( s.z + 1.0 ) * 10.0 ), bh = (UInt32) ( h.z * 10.0 ), bk = (UInt32) ( ( k.z - cosMax ) / ( 1.0 - cosMax ) * 10.0 );
            sphereZ[ bs > 19 ? 19 : bs ] += 1.0;
            hemiZ[ bh > 9 ? 9 : bh ] += 1.0;
            coneZ[ bk > 9 ? 9 : bk ] += 1.0;
            ASSERT_SEEDED( h.z >= 0.0 && c.z >= 0.0 && k.z >= cosMax - 1e-12 && k.z <= 1.0, seed );
            cosineSum += c.z;
            cosineSquares += c.z * c.z;
        }
        ASSERT_SEEDED( worst < 1e-12, seed );
        ASSERT_SEEDED( ChiSquaredUniform( sphereZ, 20, count ) < ChiSquaredCritical( 19 ), seed );
        ASSERT_SEEDED( ChiSquaredUniform( hemiZ, 10, count ) < ChiSquaredCritical( 9 ), seed );
        ASSERT_SEEDED( ChiSquaredUniform( coneZ, 10, count ) < ChiSquaredCritical( 9 ), seed );
        // density cos( theta ) / pi on the hemisphere: E[ cos ] = 2/3, E[ cos^2 ] = 1/2
        ASSERT_SEEDED( fabs( cosineSum / count - 2.0 / 3.0 ) < 0.002 && fabs( cosineSquares / count - 0.5 ) < 0.002, seed );
        // Float32 agrees with Float64
        const TFloat32Vector3D s32 = UniformSphere< Float32 >( 0.3f, 0.7f );
        const TFloat64Vector3D s64 = UniformSphere< Float64 >( 0.3, 0.7 );
        ASSERT_TRUE( fabs( s32.x - s64.x ) < 1e-6 && fabs( s32.y - s64.y ) < 1e-6 && fabs( s32.z - s64.z ) < 1e-6 );
        // a cone of cosine 1 is the pole, a cone of cosine -1 the sphere
        const TFloat64Vector3D pole = UniformCone< Float64 >( 0.4, 0.4, 1.0 );
        ASSERT_TRUE( 1.0 == pole.z && 0.0 == pole.x );
        ASSERT_TRUE( fabs( UniformCone< Float64 >( 0.3, 0.7, -1.0 ).z - UniformSphere< Float64 >( 0.3, 0.7 ).z ) < 1e-12 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sampling: UniformTriangle gives barycentric coordinates that sum to 1 and are uniform over the triangle" )
        const UInt64 seed = 1618;
        MathReference::CSeededSequence generator( seed );
        const UInt32 count = 1000000;
        double parts[ 4 ];
        memset( parts, 0, sizeof( parts ) );
        Float64 sums[ 3 ] = { 0.0, 0.0, 0.0 };
        bool valid = true;
        for ( UInt32 i=0; i<count; ++i )
        {
            const Float64 u = (Float64) ( generator.NextUInt64() >> 11 ) * ldexp( 1.0, -53 );
            const Float64 v = (Float64) ( generator.NextUInt64() >> 11 ) * ldexp( 1.0, -53 );
            const TFloat64Vector3D b = UniformTriangle< Float64 >( u, v );
            if ( b.x < 0.0 || b.y < 0.0 || b.z < 0.0 || fabs( b.x + b.y + b.z - 1.0 ) > 1e-12 ) valid = false;
            sums[ 0 ] += b.x; sums[ 1 ] += b.y; sums[ 2 ] += b.z;
            // the triangle is cut into four equal triangles by its medial triangle
            UInt32 part = 3;
            if ( b.x > 0.5 ) part = 0; else if ( b.y > 0.5 ) part = 1; else if ( b.z > 0.5 ) part = 2;
            parts[ part ] += 1.0;
        }
        ASSERT_SEEDED( valid, seed );
        ASSERT_SEEDED( ChiSquaredUniform( parts, 4, count ) < ChiSquaredCritical( 3 ), seed );
        for ( UInt32 i=0; i<3; ++i )
            ASSERT_SEEDED( fabs( sums[ i ] / count - 1.0 / 3.0 ) < 0.002, seed );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sampling: BuildOrthonormalBasis gives a right handed orthonormal basis for every normal, also at the poles" )
        const UInt64 seed = 99;
        MathReference::CSeededSequence generator( seed );
        Float64 worst = 0.0;
        for ( UInt32 i=0; i<100000; ++i )
        {
            TFloat64Vector3D n;
            if ( i == 0 ) n = TFloat64Vector3D( 0.0, 0.0, 1.0 );
            else if ( i == 1 ) n = TFloat64Vector3D( 0.0, 0.0, -1.0 );
            else if ( i == 2 ) n = TFloat64Vector3D( 1.0, 0.0, 0.0 );
            else if ( i == 3 ) n = TFloat64Vector3D( 0.0, -1.0, 0.0 );
            else
            {
                const Float64 u = (Float64) ( generator.NextUInt64() >> 11 ) * ldexp( 1.0, -53 );
                const Float64 v = (Float64) ( generator.NextUInt64() >> 11 ) * ldexp( 1.0, -53 );
                n = UniformSphere< Float64 >( u, v );
            }
            TFloat64Vector3D t, b;
            BuildOrthonormalBasis< Float64 >( n, t, b );
            const Float64 tn = t.Dot( n ), bn = b.Dot( n ), tb = t.Dot( b );
            const Float64 lt = t.Dot( t ) - 1.0, lb = b.Dot( b ) - 1.0;
            const TFloat64Vector3D cross = t.Cross( b );
            const Float64 handed = ( cross - n ).GetLength();
            worst = fmax( worst, fmax( fmax( fabs( tn ), fabs( bn ) ), fmax( fabs( tb ), fmax( fabs( lt ), fmax( fabs( lb ), handed ) ) ) ) );
        }
        ASSERT_SEEDED( worst < 1e-12, seed );
        // OrientAroundNormal maps +Z to the normal and keeps lengths
        const TFloat64Vector3D n = TFloat64Vector3D( 1.0, 2.0, 2.0 ) / 3.0;
        const TFloat64Vector3D mapped = OrientAroundNormal< Float64 >( TFloat64Vector3D( 0.0, 0.0, 1.0 ), n );
        ASSERT_TRUE( ( mapped - n ).GetLength() < 1e-12 );
        const TFloat64Vector3D other = OrientAroundNormal< Float64 >( TFloat64Vector3D( 0.6, 0.0, 0.8 ), n );
        ASSERT_TRUE( fabs( other.GetLength() - 1.0 ) < 1e-12 && fabs( other.Dot( n ) - 0.8 ) < 1e-12 );
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
