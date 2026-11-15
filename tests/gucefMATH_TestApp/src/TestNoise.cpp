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
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_CPERLINNOISE_H
#include "gucefMATH_CPerlinNoise.h"
#define GUCEF_MATH_CPERLINNOISE_H
#endif /* GUCEF_MATH_CPERLINNOISE_H ? */

#ifndef GUCEF_MATH_COPENSIMPLEX2NOISE_H
#include "gucefMATH_COpenSimplex2Noise.h"
#define GUCEF_MATH_COPENSIMPLEX2NOISE_H
#endif /* GUCEF_MATH_COPENSIMPLEX2NOISE_H ? */

#ifndef GUCEF_MATH_CVALUENOISE_H
#include "gucefMATH_CValueNoise.h"
#define GUCEF_MATH_CVALUENOISE_H
#endif /* GUCEF_MATH_CVALUENOISE_H ? */

#ifndef GUCEF_MATH_CWORLEYNOISE_H
#include "gucefMATH_CWorleyNoise.h"
#define GUCEF_MATH_CWORLEYNOISE_H
#endif /* GUCEF_MATH_CWORLEYNOISE_H ? */

#ifndef GUCEF_MATH_CTFRACTALNOISE_H
#include "gucefMATH_CTFractalNoise.h"
#define GUCEF_MATH_CTFRACTALNOISE_H
#endif /* GUCEF_MATH_CTFRACTALNOISE_H ? */

#ifndef GUCEF_MATH_CTDOMAINWARP_H
#include "gucefMATH_CTDomainWarp.h"
#define GUCEF_MATH_CTDOMAINWARP_H
#endif /* GUCEF_MATH_CTDOMAINWARP_H ? */

#include "TestRandomStatistics.h"
#include "TestNoise.h"

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

/** The positions of a deterministic sweep: arithmetic in Float64 only, so every platform sees the same inputs */
static void
SweepPoint( UInt32 i, Float64& x, Float64& y, Float64& z, Float64& w )
{
    x = i * 0.173 + 0.01;
    y = i * 0.291 - 3.7;
    z = i * 0.117 + 1.3;
    w = i * 0.059 - 0.6;
}

/** FNV-1a over the bits of the values of a noise on the sweep ( 'expression' uses x, y, z and w ) */
#define SWEEP_HASH( hash, expression )                                                          \
    {                                                                                           \
        hash = MathReference::Fnv1aOffsetBasis();                                               \
        for ( UInt32 sweepIndex=0; sweepIndex<2000; ++sweepIndex )                              \
        {                                                                                       \
            Float64 x, y, z, w;                                                                 \
            SweepPoint( sweepIndex, x, y, z, w );                                               \
            hash = MathReference::Fnv1aInt64( BitsOf( (Float64) ( expression ) ), hash );       \
        }                                                                                       \
    }

/** Counts the pinned hashes: logs the actual value when it differs from the pinned one, so a deliberate change can be re-pinned */
static bool
CheckPinned( const char* name, UInt64 actual, UInt64 pinned )
{
    if ( actual != pinned )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Noise: pinned hash of " << name << " is " << actual << " expected " << pinned );
    return actual == pinned;
}

/** Range statistics of 'expression' over random points in [-50, 50]^4 */
struct SRange
{
    Float64 minimum;
    Float64 maximum;
    SRange( void ) : minimum( 1e300 ) , maximum( -1e300 ) {}
    void Add( Float64 v ) { if ( v < minimum ) minimum = v; if ( v > maximum ) maximum = v; }
};

#define SAMPLE_RANGE( range, count, seed, expression )                                  \
    {                                                                                   \
        MathReference::CSeededSequence sequence( seed );                                \
        for ( UInt32 sampleIndex=0; sampleIndex<count; ++sampleIndex )                  \
        {                                                                               \
            const Float64 x = RandomCoordinate( sequence );                             \
            const Float64 y = RandomCoordinate( sequence );                             \
            const Float64 z = RandomCoordinate( sequence );                             \
            const Float64 w = RandomCoordinate( sequence );                             \
            range.Add( (Float64) ( expression ) );                                      \
        }                                                                               \
    }

static Float64
RandomCoordinate( MathReference::CSeededSequence& sequence )
{
    return ( (Float64) ( sequence.NextUInt64() >> 11 ) * ldexp( 1.0, -53 ) - 0.5 ) * 100.0;
}

/** The largest | f( p + d ) - f( p ) | / h over random points and a fixed direction of length h ( 'expression' uses x y z w, 'shifted' uses xs ys zs ws ) */
#define SAMPLE_LIPSCHITZ( worst, count, seed, h, expression, shifted )                  \
    {                                                                                   \
        MathReference::CSeededSequence sequence( seed );                                \
        const Float64 step = h / sqrt( 1.0 + 0.49 + 0.16 + 0.09 );                      \
        for ( UInt32 sampleIndex=0; sampleIndex<count; ++sampleIndex )                  \
        {                                                                               \
            const Float64 x = RandomCoordinate( sequence );                             \
            const Float64 y = RandomCoordinate( sequence );                             \
            const Float64 z = RandomCoordinate( sequence );                             \
            const Float64 w = RandomCoordinate( sequence );                             \
            const Float64 xs = x + step, ys = y + 0.7 * step, zs = z - 0.4 * step, ws = w + 0.3 * step; \
            const Float64 difference = fabs( (Float64) ( shifted ) - (Float64) ( expression ) ) / h;    \
            if ( difference > worst ) worst = difference;                               \
        }                                                                               \
    }

/** The brute force F1 and F2 of the regular grid of feature points at cell centres ( jitter 0 ), searched over a window of 7 cells */
static void
GridFeatures( TWorleyMetric metric, Float64 x, Float64 y, Float64 z, bool threeD, Float64& f1, Float64& f2 )
{
    f1 = 1e300;
    f2 = 1e300;
    const Int32 cx = (Int32) floor( x ), cy = (Int32) floor( y ), cz = (Int32) floor( z );
    for ( Int32 k = threeD ? -3 : 0; k <= ( threeD ? 3 : 0 ); ++k )
        for ( Int32 j=-3; j<=3; ++j )
            for ( Int32 i=-3; i<=3; ++i )
            {
                const Float64 dx = fabs( cx + i + 0.5 - x ), dy = fabs( cy + j + 0.5 - y ), dz = threeD ? fabs( cz + k + 0.5 - z ) : 0.0;
                Float64 d;
                if ( metric == WORLEYMETRIC_MANHATTAN ) d = dx + dy + dz;
                else if ( metric == WORLEYMETRIC_CHEBYSHEV ) d = fmax( dx, fmax( dy, dz ) );
                else d = sqrt( dx * dx + dy * dy + dz * dz );
                if ( d < f1 ) { f2 = f1; f1 = d; }
                else if ( d < f2 ) f2 = d;
            }
}

/** The distance between two displacements in a metric */
static Float64
MetricLength( TWorleyMetric metric, Float64 dx, Float64 dy, Float64 dz )
{
    dx = fabs( dx ); dy = fabs( dy ); dz = fabs( dz );
    if ( metric == WORLEYMETRIC_MANHATTAN ) return dx + dy + dz;
    if ( metric == WORLEYMETRIC_CHEBYSHEV ) return fmax( dx, fmax( dy, dz ) );
    return sqrt( dx * dx + dy * dy + dz * dz );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformNoiseTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Noise TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Noise" );

    GUCEF_TESTFW_TESTCASE( "Noise: Perlin noise is exactly 0 at every integer lattice point, in 1D to 4D and both kinds" )
        const CPerlinNoise noise( 12345 );
        bool zero = true;
        for ( Int32 i=-40; i<=40; ++i )
        {
            const Float64 a = (Float64) i;
            const Float32 b = (Float32) i;
            if ( 0.0 != noise.Noise1D( a ) || 0.0f != noise.Noise1D( b ) ) zero = false;
            if ( 0.0 != noise.Noise2D( a, a * 3.0 + 1.0 ) || 0.0f != noise.Noise2D( b, b * 2.0f - 5.0f ) ) zero = false;
            if ( 0.0 != noise.Noise3D( a, 7.0 - a, a * 5.0 ) || 0.0f != noise.Noise3D( b, 7.0f - b, b + 3.0f ) ) zero = false;
            if ( 0.0 != noise.Noise4D( a, 2.0 * a, 9.0 - a, a + 100.0 ) || 0.0f != noise.Noise4D( b, b, -b, b + 1.0f ) ) zero = false;
        }
        ASSERT_TRUE( zero );
        // and it is not zero in between
        ASSERT_TRUE( 0.0 != noise.Noise2D( 0.5, 0.5 ) || 0.0 != noise.Noise2D( 0.3, 0.8 ) );
        // the lattice repeats every 256 cells
        ASSERT_TRUE( fabs( noise.Noise3D( 0.37, 1.25, 2.5 ) - noise.Noise3D( 256.37, 1.25 - 256.0, 2.5 + 512.0 ) ) < 1e-9 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Noise: every noise stays within [-1, 1] over 10^6 points and uses most of the range" )
        const CPerlinNoise perlin( 1 );
        const COpenSimplex2Noise simplex( 1 );
        const CValueNoise value( 1 );
        const CWorleyNoise worleyF1( 1, WORLEYMETRIC_EUCLIDEAN, WORLEYFUNCTION_F1 );
        const CWorleyNoise worleyF2( 1, WORLEYMETRIC_MANHATTAN, WORLEYFUNCTION_F2 );
        const CWorleyNoise worleyDiff( 1, WORLEYMETRIC_CHEBYSHEV, WORLEYFUNCTION_F2_MINUS_F1, 0.7f );
        const UInt32 count = 1000000;
        const UInt32 countWorley = 300000;
        SRange p1, p2, p3, p4, s2, s3, v1, v2, v3, w2a, w3a, w2b, w3b, w2c, w3c;
        SAMPLE_RANGE( p1, count, 1, perlin.Noise1D( x ) );
        SAMPLE_RANGE( p2, count, 2, perlin.Noise2D( x, y ) );
        SAMPLE_RANGE( p3, count, 3, perlin.Noise3D( x, y, z ) );
        SAMPLE_RANGE( p4, count, 4, perlin.Noise4D( x, y, z, w ) );
        SAMPLE_RANGE( s2, count, 5, simplex.Noise2D( x, y ) );
        SAMPLE_RANGE( s3, count, 6, simplex.Noise3D( x, y, z ) );
        SAMPLE_RANGE( v1, count, 7, value.Noise1D( x ) );
        SAMPLE_RANGE( v2, count, 8, value.Noise2D( x, y ) );
        SAMPLE_RANGE( v3, count, 9, value.Noise3D( x, y, z ) );
        SAMPLE_RANGE( w2a, countWorley, 10, worleyF1.Noise2D( x, y ) );
        SAMPLE_RANGE( w3a, countWorley, 11, worleyF1.Noise3D( x, y, z ) );
        SAMPLE_RANGE( w2b, countWorley, 12, worleyF2.Noise2D( x, y ) );
        SAMPLE_RANGE( w3b, countWorley, 13, worleyF2.Noise3D( x, y, z ) );
        SAMPLE_RANGE( w2c, countWorley, 14, worleyDiff.Noise2D( x, y ) );
        SAMPLE_RANGE( w3c, countWorley, 15, worleyDiff.Noise3D( x, y, z ) );
        const SRange* all[ 15 ] = { &p1, &p2, &p3, &p4, &s2, &s3, &v1, &v2, &v3, &w2a, &w3a, &w2b, &w3b, &w2c, &w3c };
        const char* names[ 15 ] = { "perlin1", "perlin2", "perlin3", "perlin4", "simplex2", "simplex3", "value1", "value2", "value3", "worley2F1", "worley3F1", "worley2F2", "worley3F2", "worley2diff", "worley3diff" };
        for ( UInt32 i=0; i<15; ++i )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Noise: " << names[ i ] << " range [" << all[ i ]->minimum << ", " << all[ i ]->maximum << "]" );
            ASSERT_TRUE( all[ i ]->minimum >= -1.0 && all[ i ]->maximum <= 1.0 );
            // the gradient and value noises should reach beyond +- 0.6 ( 4D Perlin reaches 0.73, its extreme corner combinations are rare ), a scaling bug would show here; the cellular noises span more than half of the range
            if ( i < 9 )
                ASSERT_TRUE( all[ i ]->minimum < -0.6 && all[ i ]->maximum > 0.6 );
            else
                ASSERT_TRUE( all[ i ]->maximum - all[ i ]->minimum > 0.5 );
        }
        // the Float32 overloads obey the range as well
        Float32 low = 0.0f, high = 0.0f;
        MathReference::CSeededSequence sequence( 77 );
        for ( UInt32 i=0; i<200000; ++i )
        {
            const Float32 x = (Float32) RandomCoordinate( sequence ), y = (Float32) RandomCoordinate( sequence ), z = (Float32) RandomCoordinate( sequence );
            const Float32 a = perlin.Noise3D( x, y, z ), b = simplex.Noise3D( x, y, z ), c = value.Noise2D( x, y ), d = worleyF1.Noise3D( x, y, z );
            const Float32 vs[ 4 ] = { a, b, c, d };
            for ( UInt32 k=0; k<4; ++k ) { if ( vs[ k ] < low ) low = vs[ k ]; if ( vs[ k ] > high ) high = vs[ k ]; }
        }
        ASSERT_TRUE( low >= -1.0f && high <= 1.0f && low < -0.7f && high > 0.7f );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Noise: finite differences are bounded, the noises are continuous" )
        const CPerlinNoise perlin( 3 );
        const COpenSimplex2Noise simplex( 3 );
        const CValueNoise value( 3 );
        const CWorleyNoise worley( 3, WORLEYMETRIC_EUCLIDEAN, WORLEYFUNCTION_F1 );
        const UInt32 count = 100000;
        const Float64 h = 1e-4;
        Float64 worst[ 11 ];
        for ( UInt32 i=0; i<11; ++i ) worst[ i ] = 0.0;
        SAMPLE_LIPSCHITZ( worst[ 0 ], count, 21, h, perlin.Noise1D( x ), perlin.Noise1D( xs ) );
        SAMPLE_LIPSCHITZ( worst[ 1 ], count, 22, h, perlin.Noise2D( x, y ), perlin.Noise2D( xs, ys ) );
        SAMPLE_LIPSCHITZ( worst[ 2 ], count, 23, h, perlin.Noise3D( x, y, z ), perlin.Noise3D( xs, ys, zs ) );
        SAMPLE_LIPSCHITZ( worst[ 3 ], count, 24, h, perlin.Noise4D( x, y, z, w ), perlin.Noise4D( xs, ys, zs, ws ) );
        SAMPLE_LIPSCHITZ( worst[ 4 ], count, 25, h, simplex.Noise2D( x, y ), simplex.Noise2D( xs, ys ) );
        SAMPLE_LIPSCHITZ( worst[ 5 ], count, 26, h, simplex.Noise3D( x, y, z ), simplex.Noise3D( xs, ys, zs ) );
        SAMPLE_LIPSCHITZ( worst[ 6 ], count, 27, h, value.Noise1D( x ), value.Noise1D( xs ) );
        SAMPLE_LIPSCHITZ( worst[ 7 ], count, 28, h, value.Noise2D( x, y ), value.Noise2D( xs, ys ) );
        SAMPLE_LIPSCHITZ( worst[ 8 ], count, 29, h, value.Noise3D( x, y, z ), value.Noise3D( xs, ys, zs ) );
        SAMPLE_LIPSCHITZ( worst[ 9 ], count, 30, h, worley.Noise2D( x, y ), worley.Noise2D( xs, ys ) );
        SAMPLE_LIPSCHITZ( worst[ 10 ], count, 31, h, worley.Noise3D( x, y, z ), worley.Noise3D( xs, ys, zs ) );
        // measured: perlin 2.0 to 2.6, simplex 6.7 and 4.6, value 2.4 to 3.0, worley 1.9; the bounds leave a good margin
        const Float64 bounds[ 11 ] = { 4.0, 5.0, 5.0, 5.0, 10.0, 8.0, 5.0, 5.0, 5.0, 4.0, 4.0 };
        for ( UInt32 i=0; i<11; ++i )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Noise: largest finite difference quotient " << i << " is " << worst[ i ] );
            ASSERT_TRUE( worst[ i ] < bounds[ i ] );
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Noise: Worley F1 <= F2, and both equal a brute force search on the regular grid of every metric" )
        const TWorleyMetric metrics[ 3 ] = { WORLEYMETRIC_EUCLIDEAN, WORLEYMETRIC_MANHATTAN, WORLEYMETRIC_CHEBYSHEV };
        MathReference::CSeededSequence sequence( 515 );
        Float64 worstGrid = 0.0;
        bool ordered = true;
        for ( UInt32 m=0; m<3; ++m )
        {
            const CWorleyNoise grid( 4, metrics[ m ], WORLEYFUNCTION_F1, 0.0f );
            const CWorleyNoise jittered( 4, metrics[ m ], WORLEYFUNCTION_F1, 1.0f );
            for ( UInt32 i=0; i<20000; ++i )
            {
                const Float64 x = RandomCoordinate( sequence ), y = RandomCoordinate( sequence ), z = RandomCoordinate( sequence );
                Float64 f1, f2, b1, b2;
                grid.Evaluate2D( x, y, f1, f2 );
                GridFeatures( metrics[ m ], x, y, 0.0, false, b1, b2 );
                worstGrid = fmax( worstGrid, fmax( fabs( f1 - b1 ), fabs( f2 - b2 ) ) );
                grid.Evaluate3D( x, y, z, f1, f2 );
                GridFeatures( metrics[ m ], x, y, z, true, b1, b2 );
                worstGrid = fmax( worstGrid, fmax( fabs( f1 - b1 ), fabs( f2 - b2 ) ) );
                jittered.Evaluate2D( x, y, f1, f2 );
                if ( !( f1 <= f2 ) || f1 < 0.0 ) ordered = false;
                jittered.Evaluate3D( x, y, z, f1, f2 );
                if ( !( f1 <= f2 ) || f1 < 0.0 ) ordered = false;
                Float32 g1, g2;
                jittered.Evaluate3D( (Float32) x, (Float32) y, (Float32) z, g1, g2 );
                if ( !( g1 <= g2 ) ) ordered = false;
            }
        }
        ASSERT_TRUE( ordered );
        ASSERT_TRUE( worstGrid < 1e-12 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Noise: Worley F1 and F2 are Lipschitz continuous in their metric, which proves that no feature point is missed" )
        const TWorleyMetric metrics[ 3 ] = { WORLEYMETRIC_EUCLIDEAN, WORLEYMETRIC_MANHATTAN, WORLEYMETRIC_CHEBYSHEV };
        const Float32 jitters[ 3 ] = { 1.0f, 0.6f, 0.2f };
        for ( UInt32 m=0; m<3; ++m )
        {
            for ( UInt32 j=0; j<3; ++j )
            {
                const UInt64 seed = 600 + m * 10 + j;
                const CWorleyNoise noise( (UInt32) seed, metrics[ m ], WORLEYFUNCTION_F1, jitters[ j ] );
                MathReference::CSeededSequence sequence( seed );
                Float64 worstRatio = 0.0;
                for ( UInt32 i=0; i<150000; ++i )
                {
                    const Float64 x = RandomCoordinate( sequence ), y = RandomCoordinate( sequence ), z = RandomCoordinate( sequence );
                    const Float64 dx = ( (Float64) ( sequence.NextUInt32() >> 8 ) / 16777216.0 - 0.5 ) * 0.01;
                    const Float64 dy = ( (Float64) ( sequence.NextUInt32() >> 8 ) / 16777216.0 - 0.5 ) * 0.01;
                    const Float64 dz = ( (Float64) ( sequence.NextUInt32() >> 8 ) / 16777216.0 - 0.5 ) * 0.01;
                    Float64 a1, a2, b1, b2;
                    noise.Evaluate3D( x, y, z, a1, a2 );
                    noise.Evaluate3D( x + dx, y + dy, z + dz, b1, b2 );
                    const Float64 length = MetricLength( metrics[ m ], dx, dy, dz );
                    if ( length > 0.0 )
                        worstRatio = fmax( worstRatio, fmax( fabs( b1 - a1 ), fabs( b2 - a2 ) ) / length );
                    noise.Evaluate2D( x, y, a1, a2 );
                    noise.Evaluate2D( x + dx, y + dy, b1, b2 );
                    const Float64 length2 = MetricLength( metrics[ m ], dx, dy, 0.0 );
                    if ( length2 > 0.0 )
                        worstRatio = fmax( worstRatio, fmax( fabs( b1 - a1 ), fabs( b2 - a2 ) ) / length2 );
                }
                ASSERT_SEEDED( worstRatio <= 1.0 + 1e-9, seed );
            }
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Noise: Worley functions, jitter and metrics map to [-1, 1] as documented" )
        // jitter 0 and the Euclidean F1 at a cell centre is 0 -> -1; at a cell corner the distance is sqrt( 2 ) / 2 in 2D
        const CWorleyNoise f1( 1, WORLEYMETRIC_EUCLIDEAN, WORLEYFUNCTION_F1, 0.0f );
        ASSERT_TRUE( -1.0 == f1.Noise2D( 0.5, 0.5 ) );
        ASSERT_TRUE( fabs( f1.Noise2D( 0.0, 0.0 ) - ( 2.0 * sqrt( 0.5 ) - 1.0 ) ) < 1e-12 );
        const CWorleyNoise diff( 1, WORLEYMETRIC_EUCLIDEAN, WORLEYFUNCTION_F2_MINUS_F1, 0.0f );
        // on the border between two cells F1 = F2: -1
        ASSERT_TRUE( fabs( diff.Noise2D( 1.0, 0.5 ) + 1.0 ) < 1e-12 );
        CWorleyNoise setter( 5 );
        ASSERT_TRUE( 1.0f == setter.GetJitter() && WORLEYMETRIC_EUCLIDEAN == setter.GetMetric() && WORLEYFUNCTION_F1 == setter.GetFunction() && 5 == setter.GetSeed() );
        setter.SetJitter( 7.0f );
        ASSERT_TRUE( 1.0f == setter.GetJitter() );
        setter.SetJitter( -2.0f );
        ASSERT_TRUE( 0.0f == setter.GetJitter() );
        setter.SetMetric( WORLEYMETRIC_CHEBYSHEV );
        setter.SetFunction( WORLEYFUNCTION_F2 );
        ASSERT_TRUE( WORLEYMETRIC_CHEBYSHEV == setter.GetMetric() && WORLEYFUNCTION_F2 == setter.GetFunction() );
        // the Float32 form follows the Float64 form
        const CWorleyNoise noise( 9 );
        for ( UInt32 i=0; i<1000; ++i )
        {
            const Float64 x = i * 0.1373 - 20.0, y = i * 0.0911 + 3.0, z = i * 0.0477 - 8.0;
            ASSERT_TRUE( fabs( noise.Noise3D( (Float32) x, (Float32) y, (Float32) z ) - noise.Noise3D( (Float64) (Float32) x, (Float64) (Float32) y, (Float64) (Float32) z ) ) < 1e-4 );
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Noise: the permutation table is a permutation and a seed always produces the same noise" )
        const CPerlinNoise a( 42 ), b( 42 ), c( 43 );
        ASSERT_TRUE( a == b && a != c && 42 == a.GetSeed() );
        const UInt8* table = a.GetPermutationTable();
        bool seen[ 256 ];
        memset( seen, 0, sizeof( seen ) );
        bool repeats = true;
        for ( UInt32 i=0; i<256; ++i )
        {
            seen[ table[ i ] ] = true;
            if ( table[ i + 256 ] != table[ i ] ) repeats = false;
        }
        bool permutation = true;
        for ( UInt32 i=0; i<256; ++i ) if ( !seen[ i ] ) permutation = false;
        ASSERT_TRUE( permutation && repeats );
        ASSERT_TRUE( 0 != memcmp( a.GetPermutationTable(), c.GetPermutationTable(), 256 ) );
        // a copy and a reseeded object give the same values as a fresh one
        CPerlinNoise copy( a );
        CPerlinNoise reseeded( 7 );
        reseeded.Seed( 42 );
        for ( UInt32 i=0; i<200; ++i )
        {
            const Float64 x = i * 0.31 - 7.7, y = i * 0.17 + 0.3;
            ASSERT_TRUE( a.Noise2D( x, y ) == copy.Noise2D( x, y ) && a.Noise2D( x, y ) == reseeded.Noise2D( x, y ) );
        }
        // the other noises, too
        ASSERT_TRUE( COpenSimplex2Noise( 5 ) == COpenSimplex2Noise( 5 ) && COpenSimplex2Noise( 5 ) != COpenSimplex2Noise( 6 ) );
        ASSERT_TRUE( CValueNoise( 5 ) == CValueNoise( 5 ) && CValueNoise( 5 ) != CValueNoise( 6 ) );
        ASSERT_TRUE( CWorleyNoise( 5 ) == CWorleyNoise( 5 ) && CWorleyNoise( 5 ) != CWorleyNoise( 6 ) );
        ASSERT_TRUE( COpenSimplex2Noise( 5 ).Noise3D( 1.3, 2.7, 3.1 ) != COpenSimplex2Noise( 6 ).Noise3D( 1.3, 2.7, 3.1 ) );
        ASSERT_TRUE( CValueNoise( 5 ).Noise2D( 1.3, 2.7 ) != CValueNoise( 6 ).Noise2D( 1.3, 2.7 ) );
        ASSERT_TRUE( CWorleyNoise( 5 ).Noise2D( 1.3, 2.7 ) != CWorleyNoise( 6 ).Noise2D( 1.3, 2.7 ) );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Noise: the determinism hash of every noise for seed 12345 is pinned" )
        const CPerlinNoise perlin( 12345 );
        const COpenSimplex2Noise simplex( 12345 );
        const CValueNoise value( 12345 );
        const CWorleyNoise worley( 12345, WORLEYMETRIC_EUCLIDEAN, WORLEYFUNCTION_F2_MINUS_F1 );
        UInt64 hash = 0;
        UInt64 tableHash = MathReference::Fnv1a( perlin.GetPermutationTable(), 512, MathReference::Fnv1aOffsetBasis() );
        ASSERT_TRUE( CheckPinned( "perlin table", tableHash, 15274396400289808697ULL ) );
        SWEEP_HASH( hash, perlin.Noise1D( x ) );               ASSERT_TRUE( CheckPinned( "perlin 1D", hash, 3636939837580910640ULL ) );
        SWEEP_HASH( hash, perlin.Noise2D( x, y ) );            ASSERT_TRUE( CheckPinned( "perlin 2D", hash, 1059167385880611273ULL ) );
        SWEEP_HASH( hash, perlin.Noise3D( x, y, z ) );         ASSERT_TRUE( CheckPinned( "perlin 3D", hash, 13413209051605891456ULL ) );
        SWEEP_HASH( hash, perlin.Noise4D( x, y, z, w ) );      ASSERT_TRUE( CheckPinned( "perlin 4D", hash, 13962606691462273101ULL ) );
        SWEEP_HASH( hash, simplex.Noise2D( x, y ) );           ASSERT_TRUE( CheckPinned( "simplex 2D", hash, 14421019825181325662ULL ) );
        SWEEP_HASH( hash, simplex.Noise3D( x, y, z ) );        ASSERT_TRUE( CheckPinned( "simplex 3D", hash, 4855522635247945057ULL ) );
        SWEEP_HASH( hash, value.Noise1D( x ) );                ASSERT_TRUE( CheckPinned( "value 1D", hash, 5392683729637265182ULL ) );
        SWEEP_HASH( hash, value.Noise2D( x, y ) );             ASSERT_TRUE( CheckPinned( "value 2D", hash, 15695389005373551817ULL ) );
        SWEEP_HASH( hash, value.Noise3D( x, y, z ) );          ASSERT_TRUE( CheckPinned( "value 3D", hash, 11873680534973749356ULL ) );
        SWEEP_HASH( hash, worley.Noise2D( x, y ) );            ASSERT_TRUE( CheckPinned( "worley 2D", hash, 116389585374533826ULL ) );
        SWEEP_HASH( hash, worley.Noise3D( x, y, z ) );         ASSERT_TRUE( CheckPinned( "worley 3D", hash, 3009000307450239884ULL ) );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Noise: the Float32 overloads follow the Float64 overloads" )
        const CPerlinNoise perlin( 8 );
        const COpenSimplex2Noise simplex( 8 );
        const CValueNoise value( 8 );
        Float64 worst = 0.0;
        for ( UInt32 i=0; i<20000; ++i )
        {
            // coordinates exactly representable as Float32
            const Float32 x = (Float32) i * 0.0625f - 600.0f, y = (Float32) i * 0.03125f + 7.5f, z = (Float32) i * 0.015625f - 3.25f, w = (Float32) i * 0.0078125f;
            worst = fmax( worst, fabs( perlin.Noise1D( x ) - perlin.Noise1D( (Float64) x ) ) );
            worst = fmax( worst, fabs( perlin.Noise2D( x, y ) - perlin.Noise2D( (Float64) x, (Float64) y ) ) );
            worst = fmax( worst, fabs( perlin.Noise3D( x, y, z ) - perlin.Noise3D( (Float64) x, (Float64) y, (Float64) z ) ) );
            worst = fmax( worst, fabs( perlin.Noise4D( x, y, z, w ) - perlin.Noise4D( (Float64) x, (Float64) y, (Float64) z, (Float64) w ) ) );
            worst = fmax( worst, fabs( simplex.Noise2D( x, y ) - simplex.Noise2D( (Float64) x, (Float64) y ) ) );
            worst = fmax( worst, fabs( simplex.Noise3D( x, y, z ) - simplex.Noise3D( (Float64) x, (Float64) y, (Float64) z ) ) );
            worst = fmax( worst, fabs( value.Noise3D( x, y, z ) - value.Noise3D( (Float64) x, (Float64) y, (Float64) z ) ) );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Noise: largest difference between Float32 and Float64 evaluation " << worst );
        ASSERT_TRUE( worst < 2e-3 );
        // far away coordinates are clamped to the lattice limit, not undefined
        ASSERT_TRUE( perlin.Noise2D( 1e30, -1e30 ) == perlin.Noise2D( 2e30, -3e30 ) );
        const Float64 nan = std::numeric_limits< Float64 >::quiet_NaN();
        ASSERT_TRUE( fabs( perlin.Noise2D( nan, 1.0 ) ) <= 1.0 || perlin.Noise2D( nan, 1.0 ) != perlin.Noise2D( nan, 1.0 ) );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Noise: CTFractalNoise with one octave equals the base noise, for every noise and both kinds" )
        SFractalNoiseSettings settings;
        settings.octaves = 1;
        const CPerlinNoise perlin( 77 );
        const COpenSimplex2Noise simplex( 77 );
        const CValueNoise value( 77 );
        const CWorleyNoise worley( 77 );
        const CTFractalNoise< CPerlinNoise > fp( perlin, settings );
        const CTFractalNoise< COpenSimplex2Noise > fs( simplex, settings );
        const CTFractalNoise< CValueNoise > fv( value, settings );
        const CTFractalNoise< CWorleyNoise > fw( worley, settings );
        bool equal = true;
        for ( UInt32 i=0; i<2000; ++i )
        {
            const Float64 x = i * 0.137 - 100.0, y = i * 0.0731 + 5.0, z = i * 0.0417 - 3.0;
            const Float32 xf = (Float32) x, yf = (Float32) y, zf = (Float32) z;
            if ( fp.Noise2D( x, y ) != perlin.Noise2D( x, y ) || fp.Noise3D( x, y, z ) != perlin.Noise3D( x, y, z ) ) equal = false;
            if ( fp.Noise2D( xf, yf ) != perlin.Noise2D( xf, yf ) || fp.Noise3D( xf, yf, zf ) != perlin.Noise3D( xf, yf, zf ) ) equal = false;
            if ( fs.Noise2D( x, y ) != simplex.Noise2D( x, y ) || fs.Noise3D( x, y, z ) != simplex.Noise3D( x, y, z ) ) equal = false;
            if ( fv.Noise2D( x, y ) != value.Noise2D( x, y ) || fv.Noise3D( xf, yf, zf ) != value.Noise3D( xf, yf, zf ) ) equal = false;
            if ( fw.Noise2D( x, y ) != worley.Noise2D( x, y ) || fw.Noise3D( x, y, z ) != worley.Noise3D( x, y, z ) ) equal = false;
        }
        ASSERT_TRUE( equal );
        // frequency scales the position
        settings.frequency = 2.5;
        const CTFractalNoise< CPerlinNoise > scaled( perlin, settings );
        ASSERT_TRUE( scaled.Noise2D( 1.3, 0.7 ) == perlin.Noise2D( 1.3 * 2.5, 0.7 * 2.5 ) );
        ASSERT_TRUE( 2.5 == scaled.GetSettings().frequency && perlin == scaled.GetNoise() );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Noise: CTFractalNoise of every type and noise stays within [-1, 1], more octaves add detail, the types differ" )
        const CPerlinNoise perlin( 5 );
        const COpenSimplex2Noise simplex( 5 );
        const TFractalType types[ 4 ] = { FRACTALTYPE_FBM, FRACTALTYPE_RIDGED, FRACTALTYPE_BILLOW, FRACTALTYPE_PINGPONG };
        Float64 sampleValues[ 4 ];
        for ( UInt32 t=0; t<4; ++t )
        {
            for ( UInt32 weighted=0; weighted<2; ++weighted )
            {
                SFractalNoiseSettings settings;
                settings.type = types[ t ];
                settings.octaves = 6;
                settings.weightedStrength = weighted ? 1.0 : 0.0;
                CTFractalNoise< CPerlinNoise > fp( perlin, settings );
                CTFractalNoise< COpenSimplex2Noise > fs( simplex, settings );
                SRange r2, r3, rs;
                SAMPLE_RANGE( r2, 200000, 40 + t, fp.Noise2D( x, y ) );
                SAMPLE_RANGE( r3, 200000, 50 + t, fp.Noise3D( x, y, z ) );
                SAMPLE_RANGE( rs, 200000, 60 + t, fs.Noise3D( x, y, z ) );
                ASSERT_TRUE( r2.minimum >= -1.0 && r2.maximum <= 1.0 && r3.minimum >= -1.0 && r3.maximum <= 1.0 && rs.minimum >= -1.0 && rs.maximum <= 1.0 );
                ASSERT_TRUE( r2.maximum - r2.minimum > 0.8 && r3.maximum - r3.minimum > 0.8 && rs.maximum - rs.minimum > 0.8 );
            }
            SFractalNoiseSettings settings;
            settings.type = types[ t ];
            sampleValues[ t ] = CTFractalNoise< CPerlinNoise >( perlin, settings ).Noise2D( 3.3, 4.4 );
        }
        ASSERT_TRUE( sampleValues[ 0 ] != sampleValues[ 1 ] && sampleValues[ 1 ] != sampleValues[ 2 ] && sampleValues[ 0 ] != sampleValues[ 3 ] );
        // ridged and billow are mirror images of each other for the same octave
        SFractalNoiseSettings ridged, billow;
        ridged.type = FRACTALTYPE_RIDGED; ridged.octaves = 1;
        billow.type = FRACTALTYPE_BILLOW; billow.octaves = 1;
        ASSERT_TRUE( fabs( CTFractalNoise< CPerlinNoise >( perlin, ridged ).Noise2D( 1.37, 2.9 ) + CTFractalNoise< CPerlinNoise >( perlin, billow ).Noise2D( 1.37, 2.9 ) ) < 1e-12 );
        // another octave changes the value, a larger gain adds more of it; the first octave alone is the base noise
        SFractalNoiseSettings few, many;
        few.octaves = 2; many.octaves = 5;
        ASSERT_TRUE( CTFractalNoise< CPerlinNoise >( perlin, few ).Noise2D( 1.37, 2.9 ) != CTFractalNoise< CPerlinNoise >( perlin, many ).Noise2D( 1.37, 2.9 ) );
        // zero octaves behave like one
        SFractalNoiseSettings none;
        none.octaves = 0;
        ASSERT_TRUE( CTFractalNoise< CPerlinNoise >( perlin, none ).Noise2D( 1.37, 2.9 ) == perlin.Noise2D( 1.37, 2.9 ) );
        // the lattice zeros of the first octave do not survive further octaves
        none.octaves = 3;
        ASSERT_TRUE( 0.0 != CTFractalNoise< CPerlinNoise >( perlin, none ).Noise2D( 4.0, 6.0 ) );
        // fBm with the weights summing to 1 stays below the base range: two octaves of amplitude 1 and 0.5
        SFractalNoiseSettings weights;
        weights.octaves = 2;
        weights.gain = 0.5;
        const Float64 combined = CTFractalNoise< CPerlinNoise >( perlin, weights ).Noise2D( 1.37, 2.9 );
        const Float64 expected = ( perlin.Noise2D( 1.37, 2.9 ) + 0.5 * perlin.Noise2D( 1.37 * 2.0 + 37.3, 2.9 * 2.0 + 53.1 ) ) / 1.5;
        ASSERT_TRUE( fabs( combined - expected ) < 1e-12 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Noise: CTDomainWarp displaces the position by a bounded, deterministic noise" )
        const CPerlinNoise perlin( 12 );
        const CWorleyNoise worley( 12 );
        SDomainWarpSettings settings;
        settings.amplitude = 0.0;
        const CTDomainWarp< CPerlinNoise > off( perlin, settings );
        Float64 x = 1.5, y = -2.5, z = 3.5;
        off.Warp2D( x, y );
        ASSERT_TRUE( 1.5 == x && -2.5 == y );
        off.Warp3D( x, y, z );
        ASSERT_TRUE( 1.5 == x && -2.5 == y && 3.5 == z );

        settings.amplitude = 0.8;
        settings.frequency = 0.9;
        const CTDomainWarp< CPerlinNoise > single( perlin, settings );
        settings.octaves = 3;
        const CTDomainWarp< CPerlinNoise > progressive( perlin, settings );
        const CTDomainWarp< CWorleyNoise > cellular( worley, settings );
        MathReference::CSeededSequence sequence( 909 );
        bool bounded = true, differs = false, moved = false;
        for ( UInt32 i=0; i<20000; ++i )
        {
            const Float64 px = RandomCoordinate( sequence ), py = RandomCoordinate( sequence ), pz = RandomCoordinate( sequence );
            Float64 ax = px, ay = py, az = pz;
            single.Warp3D( ax, ay, az );
            Float64 bx = px, by = py, bz = pz;
            progressive.Warp3D( bx, by, bz );
            Float64 cx = px, cy = py;
            cellular.Warp2D( cx, cy );
            // single: |d| <= amplitude per axis; progressive: amplitude * ( 1 + 1/2 + 1/4 )
            if ( fabs( ax - px ) > 0.8 || fabs( ay - py ) > 0.8 || fabs( az - pz ) > 0.8 ) bounded = false;
            if ( fabs( bx - px ) > 0.8 * 1.75 + 1e-12 || fabs( by - py ) > 0.8 * 1.75 + 1e-12 || fabs( bz - pz ) > 0.8 * 1.75 + 1e-12 ) bounded = false;
            if ( fabs( cx - px ) > 0.8 * 1.75 + 1e-12 || fabs( cy - py ) > 0.8 * 1.75 + 1e-12 ) bounded = false;
            if ( ax != bx || ay != by ) differs = true;
            if ( ax != px || ay != py ) moved = true;
            // the 2D form is the 3D form without z: the same displacement of x and y is not required, only determinism
            Float64 ax2 = px, ay2 = py;
            single.Warp2D( ax2, ay2 );
            Float64 ax3 = px, ay3 = py;
            single.Warp2D( ax3, ay3 );
            if ( ax2 != ax3 || ay2 != ay3 ) bounded = false;
        }
        ASSERT_TRUE( bounded && differs && moved );
        // the Float32 form
        Float32 fx = 1.25f, fy = 2.5f, fz = -0.75f;
        single.Warp3D( fx, fy, fz );
        Float64 gx = 1.25, gy = 2.5, gz = -0.75;
        single.Warp3D( gx, gy, gz );
        ASSERT_TRUE( fabs( fx - gx ) < 1e-4 && fabs( fy - gy ) < 1e-4 && fabs( fz - gz ) < 1e-4 );
        Float32 hx = 1.25f, hy = 2.5f;
        progressive.Warp2D( hx, hy );
        ASSERT_TRUE( hx != 1.25f || hy != 2.5f );
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
