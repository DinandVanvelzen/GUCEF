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

#ifndef GUCEF_MATH_CTRANDOMSTREAM_H
#include "gucefMATH_CTRandomStream.h"
#define GUCEF_MATH_CTRANDOMSTREAM_H
#endif /* GUCEF_MATH_CTRANDOMSTREAM_H ? */

#ifndef GUCEF_MATH_CTALIASTABLE_H
#include "gucefMATH_CTAliasTable.h"
#define GUCEF_MATH_CTALIASTABLE_H
#endif /* GUCEF_MATH_CTALIASTABLE_H ? */

#include "TestRandomStatistics.h"
#include "TestRandomStream.h"

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

/** A generator that returns the same bits for ever, to reach the extremes of the output range */
class CConstantGenerator
{
    public:
    explicit CConstantGenerator( UInt64 bits = 0 ) : m_bits( bits ) {}
    UInt32 NextUInt32( void ) { return (UInt32) m_bits; }
    UInt64 NextUInt64( void ) { return m_bits; }
    bool operator==( const CConstantGenerator& other ) const { return m_bits == other.m_bits; }
    private:
    UInt64 m_bits;
};

/** Rotates ( x, y, z ) by the unit quaternion given by its components */
static TFloat64Vector3D
RotateBy( const TFloat32Quaternion& q, const TFloat64Vector3D& v )
{
    const Float64 qx = q.x, qy = q.y, qz = q.z, qw = q.w;
    // v' = v + 2 w ( q x v ) + 2 q x ( q x v )
    const Float64 tx = 2.0 * ( qy * v.z - qz * v.y );
    const Float64 ty = 2.0 * ( qz * v.x - qx * v.z );
    const Float64 tz = 2.0 * ( qx * v.y - qy * v.x );
    return TFloat64Vector3D( v.x + qw * tx + ( qy * tz - qz * ty ), v.y + qw * ty + ( qz * tx - qx * tz ), v.z + qw * tz + ( qx * ty - qy * tx ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformRandomStreamTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING RandomStream TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "RandomStream" );

    GUCEF_TESTFW_TESTCASE( "RandomStream: NextFloat32 and NextFloat64 never return 1, the largest values are 1 - 2^-24 and 1 - 2^-53" )
        CTRandomStream< CConstantGenerator > allOnes( CConstantGenerator( 0xFFFFFFFFFFFFFFFFULL ) );
        ASSERT_TRUE( allOnes.NextFloat32() == 1.0f - ldexpf( 1.0f, -24 ) );
        ASSERT_TRUE( allOnes.NextFloat64() == 1.0 - ldexp( 1.0, -53 ) );
        CTRandomStream< CConstantGenerator > zeros( CConstantGenerator( 0 ) );
        ASSERT_TRUE( zeros.NextFloat32() == 0.0f && zeros.NextFloat64() == 0.0 );
        const UInt64 seed = 20260101;
        CRandomStream stream( CPcg32( seed, 3 ) );
        bool inRange = true;
        for ( UInt32 i=0; i<1000000; ++i )
        {
            const Float32 f = stream.NextFloat32();
            const Float64 d = stream.NextFloat64();
            if ( !( f >= 0.0f && f < 1.0f && d >= 0.0 && d < 1.0 ) )
                inRange = false;
        }
        ASSERT_SEEDED( inRange, seed );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomStream: integer and uniform outputs reproduce the generator, pinned for state 42 stream 54" )
        CRandomStream stream( CPcg32( 42, 54 ) );
        CPcg32 reference( 42, 54 );
        // 0xa15c02b7 is the first reference output; ( bits >> 8 ) * 2^-24
        ASSERT_TRUE( stream.GetGenerator() == reference );
        ASSERT_TRUE( stream.NextFloat32() == (Float32) ( 0xa15c02b7U >> 8 ) / 16777216.0f );
        ASSERT_TRUE( stream.NextUInt32() == 0x7b47f409U );
        ASSERT_TRUE( stream.NextUInt32() == 0xba1d3330U );
        CRandomStream second( CPcg32( 42, 54 ) );
        // Lemire: floor( 0xa15c02b7 * 10 / 2^32 ) = 6
        ASSERT_TRUE( 6 == second.Range( 0, 9 ) );
        // a copy continues with the same numbers
        CRandomStream copy( second );
        for ( UInt32 i=0; i<100; ++i )
            ASSERT_TRUE( copy.NextUInt64() == second.NextUInt64() );
        // the 64 bit float takes 53 bits of one 64 bit output
        CRandomStream wide( CPcg32( 42, 54 ) );
        ASSERT_TRUE( wide.NextFloat64() == (Float64) ( 0xa15c02b77b47f409ULL >> 11 ) * ldexp( 1.0, -53 ) );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomStream: Range( 0, 9 ) over 10^6 draws passes a chi-squared test at p > 0.001, also for the other generators" )
        const UInt64 seeds[ 3 ] = { 11, 12, 13 };
        for ( UInt32 g=0; g<3; ++g )
        {
            const UInt64 currentSeed = seeds[ g ];
            const CPcg32 pcgGenerator( currentSeed, 1 );
            const CXoshiro256StarStar xoshiroGenerator( currentSeed );
            const CSplitMix64 splitMixGenerator( currentSeed );
            CRandomStream pcg( pcgGenerator );
            CXoshiroRandomStream xoshiro( xoshiroGenerator );
            CSplitMixRandomStream splitMix( splitMixGenerator );
            double counts[ 3 ][ 10 ];
            memset( counts, 0, sizeof( counts ) );
            for ( UInt32 i=0; i<1000000; ++i )
            {
                counts[ 0 ][ pcg.Range( 0, 9 ) ] += 1.0;
                counts[ 1 ][ xoshiro.Range( 0, 9 ) ] += 1.0;
                counts[ 2 ][ splitMix.Range( 0, 9 ) ] += 1.0;
            }
            for ( UInt32 k=0; k<3; ++k )
            {
                const double chi = ChiSquaredUniform( counts[ k ], 10, 1000000.0 );
                ASSERT_SEEDED( chi < ChiSquaredCritical( 9 ), seeds[ g ] );
            }
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomStream: integer ranges include both ends, handle reversed and full ranges and Below( 0 )" )
        CRandomStream stream( CPcg32( 5, 5 ) );
        ASSERT_TRUE( 7 == stream.Range( 7, 7 ) );
        bool sawLow = false, sawHigh = false, inside = true;
        for ( UInt32 i=0; i<2000; ++i )
        {
            const Int32 v = stream.Range( 9, -3 );   // reversed
            if ( v < -3 || v > 9 ) inside = false;
            if ( v == -3 ) sawLow = true;
            if ( v == 9 ) sawHigh = true;
        }
        ASSERT_TRUE( inside && sawLow && sawHigh );
        // the whole Int32 range ( span 2^32 wraps to 0 )
        bool negative = false, positive = false;
        for ( UInt32 i=0; i<100; ++i )
        {
            const Int32 v = stream.Range( (Int32) 0x80000000, (Int32) 0x7FFFFFFF );
            if ( v < 0 ) negative = true; else positive = true;
        }
        ASSERT_TRUE( negative && positive );
        ASSERT_TRUE( 0 == stream.Below( 0 ) && 0 == stream.Below( 1 ) );
        for ( UInt32 i=0; i<1000; ++i )
            ASSERT_TRUE( stream.Below( 3 ) < 3 );
        // the extreme of the output range: ( 0xFFFFFFFF * bound ) >> 32 is bound - 1
        CTRandomStream< CConstantGenerator > top( CConstantGenerator( 0xFFFFFFFFFFFFFFFFULL ) );
        ASSERT_TRUE( 9 == top.Range( 0, 9 ) );
        CTRandomStream< CConstantGenerator > bottom( CConstantGenerator( 0 ) );
        ASSERT_TRUE( 0 == bottom.Range( 0, 9 ) && -5 == bottom.Range( -5, 5 ) );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomStream: Normal has the requested mean and variance within 4 standard errors" )
        const UInt64 seed = 77;
        const UInt32 count = 1000000;
        {
            CRandomStream stream( CPcg32( seed, 1 ) );
            const Float64 mean = 3.0, sd = 2.0;
            Float64 sum = 0.0, sumSquares = 0.0;
            for ( UInt32 i=0; i<count; ++i )
            {
                const Float64 v = stream.Normal( mean, sd );
                sum += v;
                sumSquares += ( v - mean ) * ( v - mean );
            }
            const Float64 sampleMean = sum / count;
            const Float64 sampleVariance = sumSquares / count;
            ASSERT_SEEDED( fabs( sampleMean - mean ) < 4.0 * sd / sqrt( (Float64) count ), seed );
            ASSERT_SEEDED( fabs( sampleVariance - sd * sd ) < 4.0 * sd * sd * sqrt( 2.0 / count ), seed );
        }
        {
            const CXoshiro256StarStar generator( seed );
            CXoshiroRandomStream stream( generator );
            Float64 sum = 0.0, sumSquares = 0.0;
            for ( UInt32 i=0; i<count; ++i )
            {
                const Float64 v = stream.Normal( -1.0f, 0.5f );
                sum += v;
                sumSquares += ( v + 1.0 ) * ( v + 1.0 );
            }
            ASSERT_SEEDED( fabs( sum / count + 1.0 ) < 4.0 * 0.5 / sqrt( (Float64) count ), seed );
            ASSERT_SEEDED( fabs( sumSquares / count - 0.25 ) < 4.0 * 0.25 * sqrt( 2.0 / count ), seed );
        }
        // a flat 'sum of tails' sanity: about 4.55 % beyond two standard deviations
        {
            CRandomStream stream( CPcg32( seed, 9 ) );
            UInt32 beyond = 0;
            for ( UInt32 i=0; i<count; ++i )
                if ( fabs( stream.Normal( 0.0, 1.0 ) ) > 2.0 ) ++beyond;
            ASSERT_SEEDED( fabs( (Float64) beyond / count - 0.0455 ) < 0.002, seed );
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomStream: Exponential, float Range and Bool have the right mean and frequency" )
        const UInt64 seed = 31;
        const UInt32 count = 1000000;
        CRandomStream stream( CPcg32( seed, 1 ) );
        Float64 sum = 0.0;
        for ( UInt32 i=0; i<count; ++i )
            sum += stream.Exponential( 4.0 );
        ASSERT_SEEDED( fabs( sum / count - 0.25 ) < 4.0 * 0.25 / sqrt( (Float64) count ), seed );
        ASSERT_TRUE( 0.0 == stream.Exponential( 0.0 ) && 0.0f == stream.Exponential( -1.0f ) );

        Float64 low = 1e300, high = -1e300, rangeSum = 0.0;
        for ( UInt32 i=0; i<count; ++i )
        {
            const Float64 v = stream.Range( -2.0, 6.0 );
            if ( v < low ) low = v;
            if ( v > high ) high = v;
            rangeSum += v;
        }
        ASSERT_SEEDED( low >= -2.0 && high < 6.0 && low < -1.99 && high > 5.99, seed );
        ASSERT_SEEDED( fabs( rangeSum / count - 2.0 ) < 4.0 * ( 8.0 / sqrt( 12.0 ) ) / sqrt( (Float64) count ), seed );
        for ( UInt32 i=0; i<1000; ++i )
        {
            const Float32 v = stream.Range( 1.5f, 2.5f );
            ASSERT_TRUE( v >= 1.5f && v <= 2.5f );
        }

        UInt32 hits = 0;
        for ( UInt32 i=0; i<count; ++i )
        {
            ASSERT_FALSE( stream.Bool( 0.0 ) );
            ASSERT_TRUE( stream.Bool( 1.0f ) );
            if ( stream.Bool( 0.3 ) ) ++hits;
        }
        ASSERT_SEEDED( fabs( (Float64) hits / count - 0.3 ) < 4.0 * sqrt( 0.21 / count ), seed );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomStream: OnUnitSphere has unit length and z uniform in [-1, 1], InUnitSphere stays inside, in both kinds" )
        const UInt64 seed = 99;
        const UInt32 count = 1000000;
        CRandomStream stream( CPcg32( seed, 2 ) );
        double zBins[ 20 ];
        memset( zBins, 0, sizeof( zBins ) );
        double worstLength = 0.0;
        for ( UInt32 i=0; i<count; ++i )
        {
            const TFloat32Vector3D v = stream.OnUnitSphere();
            const Float64 length = sqrt( (Float64) v.x * v.x + (Float64) v.y * v.y + (Float64) v.z * v.z );
            if ( fabs( length - 1.0 ) > worstLength ) worstLength = fabs( length - 1.0 );
            UInt32 bin = (UInt32) ( ( v.z + 1.0f ) * 10.0f );
            zBins[ bin > 19 ? 19 : bin ] += 1.0;
        }
        ASSERT_SEEDED( worstLength <= 1e-6, seed );
        ASSERT_SEEDED( ChiSquaredUniform( zBins, 20, count ) < ChiSquaredCritical( 19 ), seed );

        memset( zBins, 0, sizeof( zBins ) );
        worstLength = 0.0;
        double octants[ 8 ];
        memset( octants, 0, sizeof( octants ) );
        for ( UInt32 i=0; i<count; ++i )
        {
            const TFloat64Vector3D v = stream.OnUnitSphereAs< Float64 >();
            const Float64 length = sqrt( v.x * v.x + v.y * v.y + v.z * v.z );
            if ( fabs( length - 1.0 ) > worstLength ) worstLength = fabs( length - 1.0 );
            UInt32 bin = (UInt32) ( ( v.z + 1.0 ) * 10.0 );
            zBins[ bin > 19 ? 19 : bin ] += 1.0;
            octants[ ( v.x > 0 ? 1 : 0 ) + ( v.y > 0 ? 2 : 0 ) + ( v.z > 0 ? 4 : 0 ) ] += 1.0;
        }
        ASSERT_SEEDED( worstLength <= 1e-12, seed );
        ASSERT_SEEDED( ChiSquaredUniform( zBins, 20, count ) < ChiSquaredCritical( 19 ), seed );
        ASSERT_SEEDED( ChiSquaredUniform( octants, 8, count ) < ChiSquaredCritical( 7 ), seed );

        // inside the ball: radius <= 1 and r^3 uniform
        double radiusBins[ 10 ];
        memset( radiusBins, 0, sizeof( radiusBins ) );
        bool inside = true;
        for ( UInt32 i=0; i<count; ++i )
        {
            const TFloat32Vector3D v = stream.InUnitSphere();
            const Float64 r2 = (Float64) v.x * v.x + (Float64) v.y * v.y + (Float64) v.z * v.z;
            if ( r2 > 1.0 + 1e-6 ) inside = false;
            const Float64 r3 = r2 * sqrt( r2 );
            UInt32 bin = (UInt32) ( r3 * 10.0 );
            radiusBins[ bin > 9 ? 9 : bin ] += 1.0;
        }
        ASSERT_SEEDED( inside, seed );
        ASSERT_SEEDED( ChiSquaredUniform( radiusBins, 10, count ) < ChiSquaredCritical( 9 ), seed );
        for ( UInt32 i=0; i<10000; ++i )
        {
            const TFloat64Vector3D v = stream.InUnitSphereAs< Float64 >();
            ASSERT_TRUE( v.x * v.x + v.y * v.y + v.z * v.z <= 1.0 );
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomStream: InUnitCircle fills the disk uniformly and OnUnitCircle has unit length" )
        const UInt64 seed = 1234;
        const UInt32 count = 1000000;
        CRandomStream stream( CPcg32( seed, 2 ) );
        double ringBins[ 10 ];
        memset( ringBins, 0, sizeof( ringBins ) );
        double quadrants[ 4 ];
        memset( quadrants, 0, sizeof( quadrants ) );
        bool inside = true;
        for ( UInt32 i=0; i<count; ++i )
        {
            const TFloat32Vector2D v = stream.InUnitCircle();
            const Float64 r2 = (Float64) v.x * v.x + (Float64) v.y * v.y;
            if ( r2 > 1.0 + 1e-6 ) inside = false;
            UInt32 bin = (UInt32) ( r2 * 10.0 );          // equal areas: uniform in r^2
            ringBins[ bin > 9 ? 9 : bin ] += 1.0;
            quadrants[ ( v.x > 0 ? 1 : 0 ) + ( v.y > 0 ? 2 : 0 ) ] += 1.0;
        }
        ASSERT_SEEDED( inside, seed );
        ASSERT_SEEDED( ChiSquaredUniform( ringBins, 10, count ) < ChiSquaredCritical( 9 ), seed );
        ASSERT_SEEDED( ChiSquaredUniform( quadrants, 4, count ) < ChiSquaredCritical( 3 ), seed );
        for ( UInt32 i=0; i<10000; ++i )
        {
            const TFloat64Vector2D v = stream.OnUnitCircleAs< Float64 >();
            ASSERT_TRUE( fabs( v.x * v.x + v.y * v.y - 1.0 ) < 1e-12 );
            const TFloat32Vector2D w = stream.OnUnitCircle();
            ASSERT_TRUE( fabs( (Float64) w.x * w.x + (Float64) w.y * w.y - 1.0 ) < 1e-6 );
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomStream: CosineWeightedHemisphere has mean cos( theta ) 2/3, OnHemisphere mean 1/2, around any normal" )
        const UInt64 seed = 4242;
        const UInt32 count = 1000000;
        CRandomStream stream( CPcg32( seed, 4 ) );
        const TFloat64DirectionVector3D normal( TFloat64Vector3D( 1.0, 2.0, 2.0 ) );
        Float64 cosSum = 0.0, uniformSum = 0.0;
        bool above = true;
        for ( UInt32 i=0; i<count; ++i )
        {
            const TFloat64Vector3D v = stream.CosineWeightedHemisphere( normal );
            const Float64 d = v.x * normal.GetX() + v.y * normal.GetY() + v.z * normal.GetZ();
            if ( d < -1e-12 || fabs( v.x * v.x + v.y * v.y + v.z * v.z - 1.0 ) > 1e-9 ) above = false;
            cosSum += d;
            const TFloat64Vector3D u = stream.OnHemisphere( normal );
            const Float64 e = u.x * normal.GetX() + u.y * normal.GetY() + u.z * normal.GetZ();
            if ( e < -1e-12 ) above = false;
            uniformSum += e;
        }
        ASSERT_SEEDED( above, seed );
        ASSERT_SEEDED( fabs( cosSum / count - 2.0 / 3.0 ) < 0.005, seed );
        ASSERT_SEEDED( fabs( uniformSum / count - 0.5 ) < 0.005, seed );
        // the poles of the basis construction
        const TFloat64DirectionVector3D down( TFloat64Vector3D( 0.0, 0.0, -1.0 ) );
        for ( UInt32 i=0; i<1000; ++i )
        {
            const TFloat64Vector3D v = stream.CosineWeightedHemisphere( down );
            ASSERT_TRUE( v.z <= 1e-12 && fabs( v.x * v.x + v.y * v.y + v.z * v.z - 1.0 ) < 1e-9 );
        }
        const TFloat32DirectionVector3D side( TFloat32Vector3D( 1.0f, 0.0f, 0.0f ) );
        Float64 mean32 = 0.0;
        for ( UInt32 i=0; i<100000; ++i )
            mean32 += stream.CosineWeightedHemisphere( side ).x;
        ASSERT_SEEDED( fabs( mean32 / 100000 - 2.0 / 3.0 ) < 0.01, seed );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomStream: InCone stays inside the cone with cos( theta ) uniform, InBox stays inside the box" )
        const UInt64 seed = 555;
        const UInt32 count = 1000000;
        CRandomStream stream( CPcg32( seed, 6 ) );
        const TFloat64DirectionVector3D axis( TFloat64Vector3D( -1.0, 0.5, 2.0 ) );
        const CTRadian< Float64 > half = Degrees( 40.0 );
        const Float64 cosMax = cos( half.GetValue() );
        double bins[ 10 ];
        memset( bins, 0, sizeof( bins ) );
        bool inside = true;
        for ( UInt32 i=0; i<count; ++i )
        {
            const TFloat64Vector3D v = stream.InCone( axis, half );
            const Float64 c = v.x * axis.GetX() + v.y * axis.GetY() + v.z * axis.GetZ();
            if ( c < cosMax - 1e-9 || c > 1.0 + 1e-9 ) inside = false;
            UInt32 bin = (UInt32) ( ( c - cosMax ) / ( 1.0 - cosMax ) * 10.0 );
            bins[ bin > 9 ? 9 : bin ] += 1.0;
        }
        ASSERT_SEEDED( inside, seed );
        ASSERT_SEEDED( ChiSquaredUniform( bins, 10, count ) < ChiSquaredCritical( 9 ), seed );
        // a zero angle gives the axis itself
        const TFloat64Vector3D line = stream.InCone( axis, Radians( 0.0 ) );
        ASSERT_TRUE( fabs( line.x - axis.GetX() ) < 1e-12 && fabs( line.y - axis.GetY() ) < 1e-12 && fabs( line.z - axis.GetZ() ) < 1e-12 );

        const TFloat64Vector3D low( -1.0, 2.0, 10.0 ), high( 3.0, 2.5, 20.0 );
        Float64 sums[ 3 ] = { 0.0, 0.0, 0.0 };
        bool boxed = true;
        for ( UInt32 i=0; i<100000; ++i )
        {
            const TFloat64Vector3D v = stream.InBox( low, high );
            if ( v.x < low.x || v.x >= high.x || v.y < low.y || v.y >= high.y || v.z < low.z || v.z >= high.z ) boxed = false;
            sums[ 0 ] += v.x; sums[ 1 ] += v.y; sums[ 2 ] += v.z;
        }
        ASSERT_SEEDED( boxed, seed );
        ASSERT_SEEDED( fabs( sums[ 0 ] / 100000 - 1.0 ) < 0.03 && fabs( sums[ 1 ] / 100000 - 2.25 ) < 0.005 && fabs( sums[ 2 ] / 100000 - 15.0 ) < 0.08, seed );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomStream: RandomRotation gives unit quaternions that rotate a vector uniformly over the sphere" )
        const UInt64 seed = 8080;
        const UInt32 count = 1000000;
        CRandomStream stream( CPcg32( seed, 7 ) );
        double zBins[ 20 ];
        memset( zBins, 0, sizeof( zBins ) );
        double octants[ 8 ];
        memset( octants, 0, sizeof( octants ) );
        Float64 worst = 0.0;
        for ( UInt32 i=0; i<count; ++i )
        {
            const TFloat32Quaternion q = stream.RandomRotation();
            const Float64 norm = sqrt( (Float64) q.x * q.x + (Float64) q.y * q.y + (Float64) q.z * q.z + (Float64) q.w * q.w );
            if ( fabs( norm - 1.0 ) > worst ) worst = fabs( norm - 1.0 );
            const TFloat64Vector3D v = RotateBy( q, TFloat64Vector3D( 1.0, 0.0, 0.0 ) );
            UInt32 bin = (UInt32) ( ( v.z + 1.0 ) * 10.0 );
            zBins[ bin > 19 ? 19 : bin ] += 1.0;
            octants[ ( v.x > 0 ? 1 : 0 ) + ( v.y > 0 ? 2 : 0 ) + ( v.z > 0 ? 4 : 0 ) ] += 1.0;
        }
        ASSERT_SEEDED( worst < 1e-6, seed );
        ASSERT_SEEDED( ChiSquaredUniform( zBins, 20, count ) < ChiSquaredCritical( 19 ), seed );
        ASSERT_SEEDED( ChiSquaredUniform( octants, 8, count ) < ChiSquaredCritical( 7 ), seed );
        const TFloat64Quaternion q64 = stream.RandomRotationAs< Float64 >();
        ASSERT_TRUE( fabs( q64.x * q64.x + q64.y * q64.y + q64.z * q64.z + q64.w * q64.w - 1.0 ) < 1e-12 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomStream: Shuffle of 3 elements gives all 6 permutations with equal frequency, larger shuffles keep the elements" )
        const UInt64 seed = 606;
        const UInt32 count = 600000;
        CRandomStream stream( CPcg32( seed, 8 ) );
        double permutations[ 6 ];
        memset( permutations, 0, sizeof( permutations ) );
        for ( UInt32 i=0; i<count; ++i )
        {
            int items[ 3 ] = { 0, 1, 2 };
            stream.Shuffle( items, items + 3 );
            permutations[ items[ 0 ] * 2 + ( items[ 1 ] > items[ 2 ] ? 1 : 0 ) ] += 1.0;
        }
        for ( UInt32 i=0; i<6; ++i )
            ASSERT_SEEDED( permutations[ i ] > 0.0, seed );
        ASSERT_SEEDED( ChiSquaredUniform( permutations, 6, count ) < ChiSquaredCritical( 5 ), seed );

        // position of an element is uniform over a larger array; the multiset is preserved
        double positions[ 8 ];
        memset( positions, 0, sizeof( positions ) );
        for ( UInt32 i=0; i<400000; ++i )
        {
            UInt32 items[ 8 ] = { 0, 1, 2, 3, 4, 5, 6, 7 };
            stream.Shuffle( items, items + 8 );
            for ( UInt32 k=0; k<8; ++k )
                if ( items[ k ] == 0 ) positions[ k ] += 1.0;
        }
        ASSERT_SEEDED( ChiSquaredUniform( positions, 8, 400000.0 ) < ChiSquaredCritical( 7 ), seed );

        UInt32 big[ 100 ];
        for ( UInt32 i=0; i<100; ++i ) big[ i ] = i;
        stream.Shuffle( big, big + 100 );
        bool seen[ 100 ];
        memset( seen, 0, sizeof( seen ) );
        for ( UInt32 i=0; i<100; ++i ) seen[ big[ i ] ] = true;
        bool all = true;
        for ( UInt32 i=0; i<100; ++i ) if ( !seen[ i ] ) all = false;
        ASSERT_TRUE( all );
        int single[ 1 ] = { 5 };
        stream.Shuffle( single, single + 1 );
        stream.Shuffle( single, single );
        ASSERT_TRUE( 5 == single[ 0 ] );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomStream: Pick and CTAliasTable draw indices with frequencies that match the weights" )
        const UInt64 seed = 321;
        const UInt32 count = 1000000;
        const Float64 weights[ 6 ] = { 1.0, 2.0, 3.0, 4.0, 0.0, 10.0 };
        const Float64 total = 20.0;
        CRandomStream stream( CPcg32( seed, 10 ) );
        {
            double observed[ 6 ];
            memset( observed, 0, sizeof( observed ) );
            for ( UInt32 i=0; i<count; ++i )
                observed[ stream.Pick( weights, 6 ) ] += 1.0;
            double expected[ 6 ];
            for ( UInt32 i=0; i<6; ++i ) expected[ i ] = count * weights[ i ] / total;
            ASSERT_SEEDED( 0.0 == observed[ 4 ], seed );
            ASSERT_SEEDED( ChiSquared( observed, expected, 6 ) < ChiSquaredCritical( 4 ), seed );
        }
        {
            CTAliasTable< Float64 > table( weights, 6 );
            ASSERT_TRUE( table.IsValid() && 6 == table.GetCount() );
            double observed[ 6 ], viaNumbers[ 6 ];
            memset( observed, 0, sizeof( observed ) );
            memset( viaNumbers, 0, sizeof( viaNumbers ) );
            for ( UInt32 i=0; i<count; ++i )
            {
                observed[ table.Sample( stream ) ] += 1.0;
                const Float64 u1 = stream.NextFloat64();
                const Float64 u2 = stream.NextFloat64();
                viaNumbers[ table.Sample( u1, u2 ) ] += 1.0;
            }
            double expected[ 6 ];
            for ( UInt32 i=0; i<6; ++i ) expected[ i ] = count * weights[ i ] / total;
            ASSERT_SEEDED( 0.0 == observed[ 4 ] && 0.0 == viaNumbers[ 4 ], seed );
            ASSERT_SEEDED( ChiSquared( observed, expected, 6 ) < ChiSquaredCritical( 4 ), seed );
            ASSERT_SEEDED( ChiSquared( viaNumbers, expected, 6 ) < ChiSquaredCritical( 4 ), seed );
            // the table reproduces the weights exactly: sum over the columns that lead to an index
            Float64 mass[ 6 ] = { 0, 0, 0, 0, 0, 0 };
            for ( UInt32 c=0; c<6; ++c )
            {
                mass[ c ] += table.GetAcceptance( c ) / 6.0;
                mass[ table.GetAlias( c ) ] += ( 1.0 - table.GetAcceptance( c ) ) / 6.0;
            }
            for ( UInt32 i=0; i<6; ++i )
                ASSERT_TRUE( fabs( mass[ i ] - weights[ i ] / total ) < 1e-12 );
        }
        {
            const Float32 weights32[ 4 ] = { 5.0f, 1.0f, 1.0f, 3.0f };
            CTAliasTable< Float32 > table( weights32, 4 );
            double observed[ 4 ];
            memset( observed, 0, sizeof( observed ) );
            for ( UInt32 i=0; i<count; ++i )
                observed[ table.Sample( stream ) ] += 1.0;
            double expected[ 4 ] = { count * 0.5, count * 0.1, count * 0.1, count * 0.3 };
            ASSERT_SEEDED( ChiSquared( observed, expected, 4 ) < ChiSquaredCritical( 3 ), seed );
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomStream: Pick and CTAliasTable handle degenerate weights" )
        CRandomStream stream( CPcg32( 1, 1 ) );
        const Float32 none[ 3 ] = { 0.0f, 0.0f, 0.0f };
        const Float32 negative[ 3 ] = { -1.0f, 2.0f, -5.0f };
        const Float64 single[ 1 ] = { 0.25 };
        ASSERT_TRUE( 0 == stream.Pick( none, 3 ) && 0 == stream.Pick( none, 0 ) );
        for ( UInt32 i=0; i<100; ++i )
            ASSERT_TRUE( 1 == stream.Pick( negative, 3 ) && 0 == stream.Pick( single, 1 ) );
        CTAliasTable< Float32 > table;
        ASSERT_FALSE( table.IsValid() );
        ASSERT_TRUE( 0 == table.Sample( 0.5f, 0.5f ) && 0 == table.Sample( stream ) );
        ASSERT_FALSE( table.Initialize( none, 3 ) );
        ASSERT_FALSE( table.Initialize( negative, 0 ) );
        ASSERT_FALSE( table.IsValid() );
        ASSERT_TRUE( table.Initialize( negative, 3 ) );
        for ( UInt32 i=0; i<100; ++i )
            ASSERT_TRUE( 1 == table.Sample( stream ) );
        // a failed rebuild keeps the table
        ASSERT_FALSE( table.Initialize( none, 3 ) );
        ASSERT_TRUE( table.IsValid() && 3 == table.GetCount() && 1 == table.Sample( 0.9f, 0.9f ) );
        // the largest inputs stay inside the table
        ASSERT_TRUE( table.Sample( 0.99999994f, 0.99999994f ) < 3 && table.Sample( 5.0f, 0.5f ) < 3 && table.Sample( -1.0f, 0.5f ) < 3 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RandomStream: streams over the other generators are value types that can be saved and replayed" )
        CXoshiroRandomStream a( CXoshiro256StarStar( 99 ) );
        UInt64 state[ CXoshiro256StarStar::STATE_WORD_COUNT ];
        a.GetGenerator().GetState( state );
        const Float64 first = a.Normal( 0.0, 1.0 );
        const TFloat32Vector3D firstSphere = a.OnUnitSphere();
        CXoshiroRandomStream b;
        b.GetGenerator().SetState( state );
        ASSERT_TRUE( first == b.Normal( 0.0, 1.0 ) );
        const TFloat32Vector3D again = b.OnUnitSphere();
        ASSERT_TRUE( again.x == firstSphere.x && again.y == firstSphere.y && again.z == firstSphere.z );
        ASSERT_TRUE( a == b );
        CSplitMixRandomStream s( CSplitMix64( 5 ) );
        Float64 sum = 0.0;
        for ( UInt32 i=0; i<100000; ++i ) sum += s.NextFloat64();
        ASSERT_TRUE( fabs( sum / 100000 - 0.5 ) < 0.01 );
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
