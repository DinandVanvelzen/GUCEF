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

#ifndef GUCEF_MATH_CTVBAP2D_H
#include "gucefMATH_CTVbap2D.h"
#define GUCEF_MATH_CTVBAP2D_H
#endif /* GUCEF_MATH_CTVBAP2D_H ? */

#ifndef GUCEF_MATH_CTVBAP3D_H
#include "gucefMATH_CTVbap3D.h"
#define GUCEF_MATH_CTVBAP3D_H
#endif /* GUCEF_MATH_CTVBAP3D_H ? */

#ifndef GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H
#include "gucefMATH_LowDiscrepancySequences.h"
#define GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H
#endif /* GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H ? */

#include "TestMathReference.h"
#include "TestMathAllocationCounter.h"
#include "TestVbap.h"

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

static const Float64 TestPi = 3.14159265358979323846264338327950288;

/*-------------------------------------------------------------------------*/

static Float64
ToRadians( Float64 degrees )
{
    return degrees * TestPi / 180.0;
}

/*-------------------------------------------------------------------------*/

static bool
Near( Float64 a, Float64 b, Float64 tolerance )
{
    return fabs( a - b ) <= tolerance;
}

/*-------------------------------------------------------------------------*/

/** The sum of the squares of count gains */
template < typename T >
static Float64
Power( const T* gains, UInt32 count )
{
    Float64 sum = 0.0;
    for ( UInt32 i = 0; i < count; ++i )
        sum += (Float64) gains[ i ] * (Float64) gains[ i ];
    return sum;
}

/*-------------------------------------------------------------------------*/

/** The number of gains above 'threshold' */
template < typename T >
static UInt32
CountNonZero( const T* gains, UInt32 count, Float64 threshold )
{
    UInt32 nonZero = 0;
    for ( UInt32 i = 0; i < count; ++i )
        if ( (Float64) gains[ i ] > threshold )
            ++nonZero;
    return nonZero;
}

/*-------------------------------------------------------------------------*/

/** The smallest gain */
template < typename T >
static Float64
SmallestGain( const T* gains, UInt32 count )
{
    Float64 smallest = 1.0e300;
    for ( UInt32 i = 0; i < count; ++i )
        if ( (Float64) gains[ i ] < smallest )
            smallest = (Float64) gains[ i ];
    return smallest;
}

/*-------------------------------------------------------------------------*/

/** A uniformly distributed direction on the sphere in frame coordinates ( x right, y up, z forward ) */
static TFloat64DirectionVector3D
RandomDirection( MathReference::CSeededSequence& generator )
{
    const Float64 z = generator.NextFloat64( -1.0, 1.0 );
    const Float64 angle = generator.NextFloat64( 0.0, 2.0 * TestPi );
    const Float64 radius = sqrt( 1.0 - z * z );
    return TFloat64DirectionVector3D( TFloat64Vector3D( radius * cos( angle ), z, radius * sin( angle ) ) );
}

/*-------------------------------------------------------------------------*/

/** The azimuth and elevation of a frame direction, from first principles */
static void
ToAngles( const TFloat64Vector3D& v, TFloat64Radian& azimuth, TFloat64Radian& elevation )
{
    azimuth = TFloat64Radian( atan2( v.x, v.z ) );
    elevation = TFloat64Radian( atan2( v.y, sqrt( v.x * v.x + v.z * v.z ) ) );
}

/*-------------------------------------------------------------------------*/

/** The unit vector of a speaker, from first principles ( x right, y up, z forward ) */
static TFloat64Vector3D
SpeakerVector( Float64 azimuth, Float64 elevation )
{
    return TFloat64Vector3D( cos( elevation ) * sin( azimuth ), sin( elevation ), cos( elevation ) * cos( azimuth ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  How far the direction of the gain-weighted sum of the speaker vectors is from 'direction': the panning property of VBAP
 *  for a source inside a triangle of real speakers. Large (10) when the sum is zero.
 */
static Float64
ReconstructionError( const TFloat64Vector3D* speakers, const Float64* gains, UInt32 count, const TFloat64Vector3D& direction )
{
    TFloat64Vector3D sum;
    for ( UInt32 i = 0; i < count; ++i )
        sum += speakers[ i ] * gains[ i ];
    if ( sum.GetLength() < 1e-9 )
        return 10.0;
    return ( sum.Normalized() - direction ).GetLength();
}

/*-------------------------------------------------------------------------*/

/** The largest gap between adjacent speakers of a ring, in radians */
static Float64
LargestGap( const Float64* azimuthsRadians, UInt32 count )
{
    Float64 sorted[ 64 ];
    for ( UInt32 i = 0; i < count; ++i )
    {
        Float64 a = fmod( azimuthsRadians[ i ], 2.0 * TestPi );
        if ( a < 0.0 ) a += 2.0 * TestPi;
        UInt32 position = i;
        while ( position > 0 && a < sorted[ position - 1 ] )
        {
            sorted[ position ] = sorted[ position - 1 ];
            --position;
        }
        sorted[ position ] = a;
    }
    Float64 largest = sorted[ 0 ] + 2.0 * TestPi - sorted[ count - 1 ];
    for ( UInt32 i = 0; i + 1 < count; ++i )
        if ( sorted[ i + 1 ] - sorted[ i ] > largest )
            largest = sorted[ i + 1 ] - sorted[ i ];
    return largest;
}

/*-------------------------------------------------------------------------*/

/** The cube corners of 8 speakers as azimuths and elevations (frame coordinates ( +-1, +-1, +-1 ) / sqrt( 3 )) */
static void
CubeLayout( TFloat64Radian* azimuths, TFloat64Radian* elevations, TFloat64Vector3D* vectors )
{
    UInt32 index = 0;
    for ( Int32 x = -1; x <= 1; x += 2 )
        for ( Int32 y = -1; y <= 1; y += 2 )
            for ( Int32 z = -1; z <= 1; z += 2 )
            {
                const TFloat64Vector3D v( (Float64) x / sqrt( 3.0 ), (Float64) y / sqrt( 3.0 ), (Float64) z / sqrt( 3.0 ) );
                ToAngles( v, azimuths[ index ], elevations[ index ] );
                vectors[ index ] = v;
                ++index;
            }
}

/*-------------------------------------------------------------------------*/

/** A layout from the Fibonacci sphere of the library: speakers at the points, vectors in frame coordinates */
static void
FibonacciLayout( UInt32 count, TFloat64Radian* azimuths, TFloat64Radian* elevations, TFloat64Vector3D* vectors )
{
    for ( UInt32 i = 0; i < count; ++i )
    {
        const TFloat64Vector3D v = FibonacciSphere< Float64 >( i, count );
        // the library's points are ( x, y, z ) on the sphere with the pole on z: map them to frame coordinates by a fixed rotation of the axes
        const TFloat64Vector3D frameVector( v.x, v.z, v.y );
        ToAngles( frameVector, azimuths[ i ], elevations[ i ] );
        vectors[ i ] = frameVector;
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  Prepares the speakers of a layout in 'permutations' seeded random orders and compares the gains of seeded random directions
 *  and of the directions of the speakers with those of the given order (mapped back to the speakers): the number of
 *  differences larger than 'tolerance', plus one when a permuted layout is prepared differently.
 */
static UInt32
CountOrderDifferences( const TFloat64Radian* azimuths, const TFloat64Radian* elevations, UInt32 count, UInt32 permutations,
                       UInt32 directions, MathReference::CSeededSequence& generator, Float64 tolerance )
{
    TFloat64Vbap3D reference;
    if ( !reference.Prepare( azimuths, elevations, count ) )
        return 1;
    UInt32 differences = 0;
    for ( UInt32 p = 0; p < permutations; ++p )
    {
        UInt32 order[ 64 ];
        for ( UInt32 i = 0; i < count; ++i )
            order[ i ] = i;
        for ( UInt32 i = count - 1; i > 0; --i )
        {
            const UInt32 j = generator.NextUInt32() % ( i + 1 );
            const UInt32 swap = order[ i ];
            order[ i ] = order[ j ];
            order[ j ] = swap;
        }
        TFloat64Radian mixedAzimuths[ 64 ];
        TFloat64Radian mixedElevations[ 64 ];
        for ( UInt32 i = 0; i < count; ++i )
        {
            mixedAzimuths[ i ] = azimuths[ order[ i ] ];
            mixedElevations[ i ] = elevations[ order[ i ] ];
        }
        TFloat64Vbap3D mixed;
        if ( !mixed.Prepare( mixedAzimuths, mixedElevations, count ) || mixed.GetTriangleCount() != reference.GetTriangleCount() ||
             mixed.GetVirtualSpeakerCount() != reference.GetVirtualSpeakerCount() )
        {
            ++differences;
            continue;
        }
        for ( UInt32 i = 0; i < directions + count; ++i )
        {
            TFloat64DirectionVector3D direction = RandomDirection( generator );
            if ( i >= directions )
                direction = TFloat64DirectionVector3D( SpeakerVector( azimuths[ i - directions ].GetValue(), elevations[ i - directions ].GetValue() ) );
            Float64 a[ 64 ];
            Float64 b[ 64 ];
            reference.ComputeGains( direction, a, 2.0 );
            mixed.ComputeGains( direction, b, 2.0 );
            for ( UInt32 j = 0; j < count; ++j )
                if ( !Near( b[ j ], a[ order[ j ] ], tolerance ) )
                    ++differences;
        }
    }
    return differences;
}

/*-------------------------------------------------------------------------*/

/** The panners from a realtime context: a RealtimeSanitizer build reports any allocation */
static Float32
RealtimeVbapWork( Float32 azimuth ) GUCEF_MATH_REALTIME_CONTEXT
{
    const TFloat32Radian azimuths2D[ 5 ] = { TFloat32Radian( -1.9199f ), TFloat32Radian( -0.5236f ), TFloat32Radian( 0.0f ), TFloat32Radian( 0.5236f ), TFloat32Radian( 1.9199f ) };
    TFloat32Vbap2D ring;
    Float32 gains2D[ 5 ];
    const bool prepared2D = ring.Prepare( azimuths2D, 5 );
    ring.ComputeGains( TFloat32Radian( azimuth ), gains2D, 2.0f );
    const TFloat32Radian azimuths3D[ 6 ] = { TFloat32Radian( -0.5f ), TFloat32Radian( 0.5f ), TFloat32Radian( 2.0f ), TFloat32Radian( -2.0f ), TFloat32Radian( 0.0f ), TFloat32Radian( 3.0f ) };
    const TFloat32Radian elevations3D[ 6 ] = { TFloat32Radian( 0.1f ), TFloat32Radian( 0.1f ), TFloat32Radian( 0.3f ), TFloat32Radian( 0.3f ), TFloat32Radian( 0.8f ), TFloat32Radian( -0.4f ) };
    TFloat32Vbap3D dome;
    Float32 gains3D[ 6 ];
    const bool prepared3D = dome.Prepare( azimuths3D, elevations3D, 6 );
    dome.ComputeGains( TFloat32Radian( azimuth ), TFloat32Radian( 0.2f ), gains3D, 2.0f );
    dome.ComputeGains( TFloat32DirectionVector3D( TFloat32Vector3D( 0.0f, -1.0f, 0.1f ) ), gains3D, 1.0f );
    Float32 sum = prepared2D && prepared3D ? 1.0f : 0.0f;
    for ( UInt32 i = 0; i < 5; ++i )
        sum += gains2D[ i ];
    for ( UInt32 i = 0; i < 6; ++i )
        sum += gains3D[ i ];
    return sum;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVbapTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Vbap" );

    GUCEF_TESTFW_TESTCASE( "Vbap2D: stereo +-30 degrees: azimuth 0 gives equal gains with the power 2, +30 gives sqrt( 2 ) on the right speaker only" )
    try
    {
        // acceptance 2
        const TFloat64Radian speakers[ 2 ] = { TFloat64Radian( ToRadians( -30.0 ) ), TFloat64Radian( ToRadians( 30.0 ) ) };
        TFloat64Vbap2D panner;
        ASSERT_FALSE( panner.IsPrepared() );
        ASSERT_TRUE( panner.Prepare( speakers, 2 ) && panner.IsPrepared() && 2 == panner.GetSpeakerCount() );
        Float64 gains[ 2 ] = { -1.0, -1.0 };
        panner.ComputeGains( TFloat64Radian( 0.0 ), gains, 2.0 );
        ASSERT_TRUE( Near( gains[ 0 ], gains[ 1 ], 1e-12 ) && Near( gains[ 0 ], 1.0, 1e-12 ) && Near( Power( gains, 2 ), 2.0, 1e-5 ) );
        panner.ComputeGains( TFloat64Radian( ToRadians( 30.0 ) ), gains, 2.0 );
        ASSERT_TRUE( Near( gains[ 0 ], 0.0, 1e-12 ) && Near( gains[ 1 ], sqrt( 2.0 ), 1e-12 ) );
        panner.ComputeGains( TFloat64Radian( ToRadians( -30.0 ) ), gains, 2.0 );
        ASSERT_TRUE( Near( gains[ 0 ], sqrt( 2.0 ), 1e-12 ) && Near( gains[ 1 ], 0.0, 1e-12 ) );
        // the target power is a parameter
        panner.ComputeGains( TFloat64Radian( 0.0 ), gains, 0.5 );
        ASSERT_TRUE( Near( Power( gains, 2 ), 0.5, 1e-12 ) );

        // 1 degree steps over the whole circle: non-negative, the power 2, and the pair formula inside the arc: [ g1 g2 ] ~ [ sin( a2 - a ) sin( a - a1 ) ]
        UInt32 failures = 0;
        for ( Int32 degrees = -180; degrees <= 180; ++degrees )
        {
            const Float64 a = ToRadians( (Float64) degrees );
            panner.ComputeGains( TFloat64Radian( a ), gains, 2.0 );
            if ( SmallestGain( gains, 2 ) < 0.0 || !Near( Power( gains, 2 ), 2.0, 1e-5 ) )
                ++failures;
            if ( degrees >= -30 && degrees <= 30 )
            {
                const Float64 s1 = sin( ToRadians( 30.0 ) - a );
                const Float64 s2 = sin( a + ToRadians( 30.0 ) );
                const Float64 norm = sqrt( 2.0 / ( s1 * s1 + s2 * s2 ) );
                if ( !Near( gains[ 0 ], s1 * norm, 1e-9 ) || !Near( gains[ 1 ], s2 * norm, 1e-9 ) )
                    ++failures;
            }
        }
        ASSERT_TRUE( 0 == failures );

        // behind the listener (the open side of the pair) both speakers share the power equally, left of it more on the left
        panner.ComputeGains( TFloat64Radian( TestPi ), gains, 2.0 );
        ASSERT_TRUE( Near( gains[ 0 ], gains[ 1 ], 1e-9 ) && Near( gains[ 0 ], 1.0, 1e-9 ) );
        panner.ComputeGains( TFloat64Radian( -ToRadians( 100.0 ) ), gains, 2.0 );
        ASSERT_TRUE( gains[ 0 ] > gains[ 1 ] );
        panner.ComputeGains( TFloat64Radian( ToRadians( 100.0 ) ), gains, 2.0 );
        ASSERT_TRUE( gains[ 1 ] > gains[ 0 ] );
        // the crossfade is continuous at both speakers
        Float64 before[ 2 ];
        Float64 after[ 2 ];
        panner.ComputeGains( TFloat64Radian( ToRadians( 30.0 ) - 1e-9 ), before, 2.0 );
        panner.ComputeGains( TFloat64Radian( ToRadians( 30.0 ) + 1e-9 ), after, 2.0 );
        ASSERT_TRUE( Near( before[ 0 ], after[ 0 ], 1e-6 ) && Near( before[ 1 ], after[ 1 ], 1e-6 ) );
        panner.ComputeGains( TFloat64Radian( -ToRadians( 30.0 ) - 1e-9 ), before, 2.0 );
        panner.ComputeGains( TFloat64Radian( -ToRadians( 30.0 ) + 1e-9 ), after, 2.0 );
        ASSERT_TRUE( Near( before[ 0 ], after[ 0 ], 1e-6 ) && Near( before[ 1 ], after[ 1 ], 1e-6 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vbap2D: ring -110, -30, 0, 30, 110: one or two non-negative gains at every degree, power 2, speaker azimuths pan fully to the speaker" )
    try
    {
        // acceptance 2
        const Float64 ring[ 5 ] = { -110.0, -30.0, 0.0, 30.0, 110.0 };
        TFloat64Radian speakers[ 5 ];
        for ( UInt32 i = 0; i < 5; ++i )
            speakers[ i ] = TFloat64Radian( ToRadians( ring[ i ] ) );
        TFloat64Vbap2D panner;
        ASSERT_TRUE( panner.Prepare( speakers, 5 ) && 5 == panner.GetSpeakerCount() );
        UInt32 failures = 0;
        for ( Int32 degrees = -180; degrees <= 180; ++degrees )
        {
            Float64 gains[ 5 ];
            const Float64 a = ToRadians( (Float64) degrees );
            panner.ComputeGains( TFloat64Radian( a ), gains, 2.0 );
            const UInt32 nonZero = CountNonZero( gains, 5, 1e-12 );
            // the reconstruction: the gain-weighted sum of the speaker vectors points at the source (inside the covered ring)
            TFloat64Vector3D vectors[ 5 ];
            for ( UInt32 i = 0; i < 5; ++i )
                vectors[ i ] = TFloat64Vector3D( sin( speakers[ i ].GetValue() ), 0.0, cos( speakers[ i ].GetValue() ) );
            if ( ( nonZero != 1 && nonZero != 2 ) || SmallestGain( gains, 5 ) < 0.0 || !Near( Power( gains, 5 ), 2.0, 1e-5 ) ||
                 ReconstructionError( vectors, gains, 5, TFloat64Vector3D( sin( a ), 0.0, cos( a ) ) ) > 1e-9 )
                ++failures;
            // the two gains belong to adjacent speakers of the ring
            if ( 2 == nonZero )
            {
                UInt32 first = 5;
                UInt32 second = 5;
                for ( UInt32 i = 0; i < 5; ++i )
                {
                    if ( gains[ i ] > 1e-12 )
                    {
                        if ( 5 == first ) first = i; else second = i;
                    }
                }
                if ( !( second == first + 1 || ( 0 == first && 4 == second ) ) )
                    ++failures;
            }
        }
        ASSERT_TRUE( 0 == failures );

        // the gains are continuous: no step of 0.1 degrees changes a gain by more than 0.02 (the steepest slope is about 3 per radian)
        Float64 previous[ 5 ];
        Float64 largestStep = 0.0;
        for ( UInt32 step = 0; step <= 3600; ++step )
        {
            Float64 gains[ 5 ];
            panner.ComputeGains( TFloat64Radian( ToRadians( -180.0 + 0.1 * (Float64) step ) ), gains, 2.0 );
            for ( UInt32 i = 0; i < 5 && step > 0; ++i )
                if ( fabs( gains[ i ] - previous[ i ] ) > largestStep )
                    largestStep = fabs( gains[ i ] - previous[ i ] );
            for ( UInt32 i = 0; i < 5; ++i )
                previous[ i ] = gains[ i ];
        }
        ASSERT_TRUE( largestStep < 0.02 && largestStep > 0.0 );

        // a source at the azimuth of a speaker: that speaker alone with the full power
        for ( UInt32 s = 0; s < 5; ++s )
        {
            Float64 gains[ 5 ];
            panner.ComputeGains( speakers[ s ], gains, 2.0 );
            for ( UInt32 i = 0; i < 5; ++i )
                ASSERT_TRUE( Near( gains[ i ], s == i ? sqrt( 2.0 ) : 0.0, 1e-9 ) );
        }

        // the azimuth is periodic
        Float64 a[ 5 ];
        Float64 b[ 5 ];
        panner.ComputeGains( TFloat64Radian( 0.7 ), a, 2.0 );
        panner.ComputeGains( TFloat64Radian( 0.7 + 6.0 * TestPi ), b, 2.0 );
        for ( UInt32 i = 0; i < 5; ++i )
            ASSERT_TRUE( Near( a[ i ], b[ i ], 1e-9 ) );
        panner.ComputeGains( TFloat64Radian( 0.7 - 4.0 * TestPi ), b, 2.0 );
        for ( UInt32 i = 0; i < 5; ++i )
            ASSERT_TRUE( Near( a[ i ], b[ i ], 1e-9 ) );

        // the gains follow the order of the speakers given to Prepare, whatever their ring order
        const TFloat64Radian shuffled[ 5 ] = { TFloat64Radian( ToRadians( 110.0 ) ), TFloat64Radian( ToRadians( -30.0 ) ), TFloat64Radian( ToRadians( 30.0 ) ), TFloat64Radian( ToRadians( -110.0 ) ), TFloat64Radian( ToRadians( 0.0 ) ) };
        const UInt32 mapping[ 5 ] = { 4, 1, 3, 0, 2 };
        TFloat64Vbap2D shuffledPanner;
        ASSERT_TRUE( shuffledPanner.Prepare( shuffled, 5 ) );
        for ( Int32 degrees = -180; degrees <= 180; degrees += 3 )
        {
            Float64 sorted[ 5 ];
            Float64 mixed[ 5 ];
            panner.ComputeGains( TFloat64Radian( ToRadians( (Float64) degrees ) ), sorted, 2.0 );
            shuffledPanner.ComputeGains( TFloat64Radian( ToRadians( (Float64) degrees ) ), mixed, 2.0 );
            for ( UInt32 i = 0; i < 5; ++i )
                ASSERT_TRUE( Near( mixed[ i ], sorted[ mapping[ i ] ], 1e-12 ) );
        }
        // speakers given as angles outside ( -pi, pi ] are the same speakers
        const TFloat64Radian wrapped[ 5 ] = { TFloat64Radian( ToRadians( 250.0 ) ), TFloat64Radian( ToRadians( -30.0 + 360.0 ) ), TFloat64Radian( ToRadians( 0.0 - 720.0 ) ), TFloat64Radian( ToRadians( 30.0 ) ), TFloat64Radian( ToRadians( 110.0 ) ) };
        TFloat64Vbap2D wrappedPanner;
        ASSERT_TRUE( wrappedPanner.Prepare( wrapped, 5 ) );
        Float64 wrappedGains[ 5 ];
        Float64 sortedGains[ 5 ];
        wrappedPanner.ComputeGains( TFloat64Radian( ToRadians( -100.0 ) ), wrappedGains, 2.0 );
        panner.ComputeGains( TFloat64Radian( ToRadians( -100.0 ) ), sortedGains, 2.0 );
        ASSERT_TRUE( Near( wrappedGains[ 0 ], sortedGains[ 0 ], 1e-9 ) && Near( wrappedGains[ 1 ], sortedGains[ 1 ], 1e-9 ) && Near( wrappedGains[ 2 ], sortedGains[ 2 ], 1e-9 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vbap2D: Prepare rejects coincident speakers and gaps of 180 degrees or more, and leaves the object unprepared" )
    try
    {
        // acceptance 2
        TFloat64Vbap2D panner;
        const TFloat64Radian good[ 3 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( ToRadians( 120.0 ) ), TFloat64Radian( ToRadians( 240.0 ) ) };
        ASSERT_TRUE( panner.Prepare( good, 3 ) && 3 == panner.GetSpeakerCount() );
        Float64 gains[ 3 ] = { 7.0, 7.0, 7.0 };

        // coincident speakers: the same angle, an angle that differs by a full turn, and one that is too close
        const TFloat64Radian same[ 3 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( 1.0 ), TFloat64Radian( 1.0 ) };
        ASSERT_FALSE( panner.Prepare( same, 3 ) );
        ASSERT_TRUE( 0 == panner.GetSpeakerCount() && !panner.IsPrepared() );
        panner.ComputeGains( TFloat64Radian( 0.5 ), gains, 2.0 );
        ASSERT_TRUE( 7.0 == gains[ 0 ] && 7.0 == gains[ 1 ] && 7.0 == gains[ 2 ] );
        const TFloat64Radian turn[ 3 ] = { TFloat64Radian( 0.5 ), TFloat64Radian( 2.0 ), TFloat64Radian( 0.5 + 2.0 * TestPi ) };
        ASSERT_FALSE( panner.Prepare( turn, 3 ) );
        const TFloat64Radian close[ 3 ] = { TFloat64Radian( 0.5 ), TFloat64Radian( 2.0 ), TFloat64Radian( 2.0 + 1e-9 ) };
        ASSERT_FALSE( panner.Prepare( close, 3 ) );
        const TFloat64Radian aroundPi[ 3 ] = { TFloat64Radian( TestPi ), TFloat64Radian( -TestPi ), TFloat64Radian( 0.0 ) };
        ASSERT_FALSE( panner.Prepare( aroundPi, 3 ) );
        const TFloat64Radian coincidentPair[ 2 ] = { TFloat64Radian( 0.3 ), TFloat64Radian( 0.3 ) };
        ASSERT_FALSE( panner.Prepare( coincidentPair, 2 ) );

        // gaps of 180 degrees or more
        const TFloat64Radian rightAngles[ 3 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( ToRadians( 90.0 ) ), TFloat64Radian( ToRadians( 180.0 ) ) };
        ASSERT_FALSE( panner.Prepare( rightAngles, 3 ) );
        const TFloat64Radian wide[ 3 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( ToRadians( 60.0 ) ), TFloat64Radian( ToRadians( 120.0 ) ) };
        ASSERT_FALSE( panner.Prepare( wide, 3 ) );
        const TFloat64Radian front[ 3 ] = { TFloat64Radian( ToRadians( -30.0 ) ), TFloat64Radian( 0.0 ), TFloat64Radian( ToRadians( 30.0 ) ) };
        ASSERT_FALSE( panner.Prepare( front, 3 ) );
        // a pair exactly opposite each other has two arcs of 180 degrees: it is a stereo pair and is accepted (see the test of +-90 degrees)
        const TFloat64Radian antipodes[ 2 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( TestPi ) };
        ASSERT_TRUE( panner.Prepare( antipodes, 2 ) && 2 == panner.GetSpeakerCount() );
        panner.Clear();
        ASSERT_TRUE( 0 == panner.GetSpeakerCount() );
        // just inside 180 degrees (largest gap 160 degrees) works, as does a pair
        const TFloat64Radian justInside[ 3 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( ToRadians( 100.0 ) ), TFloat64Radian( ToRadians( 200.0 ) ) };
        ASSERT_TRUE( panner.Prepare( justInside, 3 ) );
        const TFloat64Radian pair[ 2 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( ToRadians( 170.0 ) ) };
        ASSERT_TRUE( panner.Prepare( pair, 2 ) );

        // the capacity and the arguments
        TFloat64Radian many[ 40 ];
        for ( UInt32 i = 0; i < 40; ++i )
            many[ i ] = TFloat64Radian( 2.0 * TestPi * (Float64) i / 40.0 );
        ASSERT_TRUE( panner.Prepare( many, 32 ) && 32 == panner.GetSpeakerCount() );
        ASSERT_FALSE( panner.Prepare( many, 33 ) );
        ASSERT_FALSE( panner.Prepare( many, 40 ) );
        ASSERT_FALSE( panner.Prepare( many, 1 ) );
        ASSERT_FALSE( panner.Prepare( many, 0 ) );
        ASSERT_FALSE( panner.Prepare( GUCEF_NULL, 3 ) );
        const Float64 nan = CTNumericTraits< Float64 >::Max() * 2.0 - CTNumericTraits< Float64 >::Max() * 2.0;
        const TFloat64Radian notFinite[ 3 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( nan ), TFloat64Radian( 3.0 ) };
        ASSERT_FALSE( panner.Prepare( notFinite, 3 ) );
        ASSERT_TRUE( 0 == panner.GetSpeakerCount() );
        // a prepared object can be prepared again, and cleared
        ASSERT_TRUE( panner.Prepare( good, 3 ) );
        panner.Clear();
        ASSERT_TRUE( 0 == panner.GetSpeakerCount() );
        ASSERT_TRUE( 32 == (UInt32) TFloat64Vbap2D::MaxSpeakers );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vbap2D: a stereo pair at exactly +-90 degrees (two arcs of 180 degrees) is accepted: defined, symmetric, continuous gains with the power 2, in every speaker order" )
    try
    {
        const TFloat64Radian pairs[ 4 ][ 2 ] = {
            { TFloat64Radian( -ToRadians( 90.0 ) ), TFloat64Radian( ToRadians( 90.0 ) ) },
            { TFloat64Radian( ToRadians( 90.0 ) ), TFloat64Radian( -ToRadians( 90.0 ) ) },
            { TFloat64Radian( ToRadians( 270.0 ) ), TFloat64Radian( ToRadians( 90.0 ) ) },
            { TFloat64Radian( 0.0 ), TFloat64Radian( TestPi ) } };
        for ( UInt32 p = 0; p < 4; ++p )
        {
            TFloat64Vbap2D panner;
            ASSERT_TRUE( panner.Prepare( pairs[ p ], 2 ) && 2 == panner.GetSpeakerCount() );
            Float64 gains[ 2 ];
            UInt32 failures = 0;
            for ( Int32 tenth = -3600; tenth <= 3600; ++tenth )
            {
                panner.ComputeGains( TFloat64Radian( ToRadians( (Float64) tenth / 10.0 ) ), gains, 2.0 );
                if ( SmallestGain( gains, 2 ) < 0.0 || !Near( Power( gains, 2 ), 2.0, 1e-9 ) )
                    ++failures;
            }
            ASSERT_TRUE( 0 == failures );
            // a source at a speaker is that speaker alone, from both sides of it
            for ( UInt32 s = 0; s < 2; ++s )
            {
                panner.ComputeGains( pairs[ p ][ s ], gains, 2.0 );
                ASSERT_TRUE( Near( gains[ s ], sqrt( 2.0 ), 1e-9 ) && Near( gains[ 1 - s ], 0.0, 1e-9 ) );
                Float64 before[ 2 ];
                Float64 after[ 2 ];
                panner.ComputeGains( TFloat64Radian( pairs[ p ][ s ].GetValue() - 1e-9 ), before, 2.0 );
                panner.ComputeGains( TFloat64Radian( pairs[ p ][ s ].GetValue() + 1e-9 ), after, 2.0 );
                ASSERT_TRUE( Near( before[ 0 ], after[ 0 ], 1e-6 ) && Near( before[ 1 ], after[ 1 ], 1e-6 ) );
            }
        }

        // the pair at -90 and +90: equal gains straight ahead and straight behind, a source towards a side favours that speaker, mirror symmetric
        TFloat64Vbap2D stereo;
        ASSERT_TRUE( stereo.Prepare( pairs[ 0 ], 2 ) );
        Float64 gains[ 2 ];
        Float64 mirrored[ 2 ];
        stereo.ComputeGains( TFloat64Radian( 0.0 ), gains, 2.0 );
        ASSERT_TRUE( Near( gains[ 0 ], 1.0, 1e-9 ) && Near( gains[ 1 ], 1.0, 1e-9 ) );
        stereo.ComputeGains( TFloat64Radian( TestPi ), gains, 2.0 );
        ASSERT_TRUE( Near( gains[ 0 ], 1.0, 1e-9 ) && Near( gains[ 1 ], 1.0, 1e-9 ) );
        UInt32 asymmetric = 0;
        for ( Int32 degrees = 1; degrees < 90; ++degrees )
        {
            stereo.ComputeGains( TFloat64Radian( ToRadians( (Float64) degrees ) ), gains, 2.0 );
            stereo.ComputeGains( TFloat64Radian( -ToRadians( (Float64) degrees ) ), mirrored, 2.0 );
            if ( !( gains[ 1 ] > gains[ 0 ] ) || !Near( gains[ 0 ], mirrored[ 1 ], 1e-9 ) || !Near( gains[ 1 ], mirrored[ 0 ], 1e-9 ) )
                ++asymmetric;
        }
        ASSERT_TRUE( 0 == asymmetric );

        // Float32, and the pair a little inside 180 degrees (VBAP between the speakers) is continuous with the pair at 180 degrees
        const TFloat32Radian pair32[ 2 ] = { TFloat32Radian( -1.5707964f ), TFloat32Radian( 1.5707964f ) };
        TFloat32Vbap2D stereo32;
        ASSERT_TRUE( stereo32.Prepare( pair32, 2 ) );
        Float32 gains32[ 2 ];
        stereo32.ComputeGains( TFloat32Radian( 0.3f ), gains32, 2.0f );
        ASSERT_TRUE( Near( Power( gains32, 2 ), 2.0, 1e-5 ) && gains32[ 1 ] > gains32[ 0 ] );
        const TFloat64Radian nearlyOpposite[ 2 ] = { TFloat64Radian( -ToRadians( 89.0 ) ), TFloat64Radian( ToRadians( 89.0 ) ) };
        TFloat64Vbap2D wide;
        ASSERT_TRUE( wide.Prepare( nearlyOpposite, 2 ) );
        wide.ComputeGains( TFloat64Radian( 0.0 ), gains, 2.0 );
        ASSERT_TRUE( Near( gains[ 0 ], gains[ 1 ], 1e-9 ) && Near( Power( gains, 2 ), 2.0, 1e-9 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vbap2D: seeded rings of 3 to 32 speakers: power, continuity, the rejection agrees with the largest gap, other power targets, bad arguments" )
    try
    {
        const UInt64 seed = 0x2D01;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        UInt32 accepted = 0;
        UInt32 rejected = 0;
        for ( UInt32 layout = 0; layout < 300; ++layout )
        {
            const UInt32 count = 3 + (UInt32) ( generator.NextUInt32() % 30 );
            Float64 azimuths[ 32 ];
            TFloat64Radian speakers[ 32 ];
            for ( UInt32 i = 0; i < count; ++i )
            {
                azimuths[ i ] = generator.NextFloat64( -TestPi, TestPi );
                speakers[ i ] = TFloat64Radian( azimuths[ i ] );
            }
            const Float64 gap = LargestGap( azimuths, count );
            TFloat64Vbap2D panner;
            const bool prepared = panner.Prepare( speakers, count );
            if ( prepared != ( gap < TestPi - 1e-6 ) )
            {
                // the margin of the rejection is the square root of the epsilon: ambiguity only inside it
                if ( fabs( gap - TestPi ) > 1e-6 )
                    ++failures;
                continue;
            }
            if ( !prepared )
            {
                ++rejected;
                continue;
            }
            ++accepted;
            const Float64 target = generator.NextFloat64( 0.25, 4.0 );
            Float64 gains[ 32 ];
            TFloat64Vector3D vectors[ 32 ];
            for ( UInt32 i = 0; i < count; ++i )
                vectors[ i ] = TFloat64Vector3D( sin( azimuths[ i ] ), 0.0, cos( azimuths[ i ] ) );
            for ( UInt32 step = 0; step <= 720; ++step )
            {
                const Float64 a = -TestPi + 2.0 * TestPi * (Float64) step / 720.0;
                panner.ComputeGains( TFloat64Radian( a ), gains, target );
                if ( SmallestGain( gains, count ) < 0.0 || !Near( Power( gains, count ), target, 1e-9 * target ) || CountNonZero( gains, count, 1e-12 ) > 2 ||
                     ReconstructionError( vectors, gains, count, TFloat64Vector3D( sin( a ), 0.0, cos( a ) ) ) > 1e-8 )
                    ++failures;
            }
            // every speaker pans fully to itself
            for ( UInt32 i = 0; i < count; ++i )
            {
                panner.ComputeGains( speakers[ i ], gains, target );
                for ( UInt32 j = 0; j < count; ++j )
                    if ( !Near( gains[ j ], i == j ? sqrt( target ) : 0.0, 1e-7 ) )
                        ++failures;
            }
        }
        ASSERT_TRUE( 0 == failures );
        ASSERT_TRUE( accepted > 50 && rejected > 0 );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Vbap: " << failures << " 2D ring checks failed, seed " << seed );

        // arguments of ComputeGains: a target power that is not positive and a non-finite azimuth write zeros, a null array is ignored
        const TFloat64Radian ring[ 3 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( ToRadians( 120.0 ) ), TFloat64Radian( ToRadians( 240.0 ) ) };
        TFloat64Vbap2D panner;
        ASSERT_TRUE( panner.Prepare( ring, 3 ) );
        Float64 gains[ 3 ] = { 5.0, 5.0, 5.0 };
        panner.ComputeGains( TFloat64Radian( 0.1 ), gains, 0.0 );
        ASSERT_TRUE( 0.0 == gains[ 0 ] && 0.0 == gains[ 1 ] && 0.0 == gains[ 2 ] );
        gains[ 0 ] = gains[ 1 ] = gains[ 2 ] = 5.0;
        panner.ComputeGains( TFloat64Radian( 0.1 ), gains, -1.0 );
        ASSERT_TRUE( 0.0 == gains[ 0 ] && 0.0 == gains[ 1 ] && 0.0 == gains[ 2 ] );
        gains[ 0 ] = gains[ 1 ] = gains[ 2 ] = 5.0;
        const Float64 nan = CTNumericTraits< Float64 >::Max() * 2.0 - CTNumericTraits< Float64 >::Max() * 2.0;
        panner.ComputeGains( TFloat64Radian( nan ), gains, 2.0 );
        ASSERT_TRUE( 0.0 == gains[ 0 ] && 0.0 == gains[ 1 ] && 0.0 == gains[ 2 ] );
        panner.ComputeGains( TFloat64Radian( 0.1 ), GUCEF_NULL, 2.0 );

        // Float32: 5.0 layout in 1 degree steps
        const TFloat32Radian ring32[ 5 ] = { TFloat32Radian( -1.9198622f ), TFloat32Radian( -0.5235988f ), TFloat32Radian( 0.0f ), TFloat32Radian( 0.5235988f ), TFloat32Radian( 1.9198622f ) };
        TFloat32Vbap2D panner32;
        ASSERT_TRUE( panner32.Prepare( ring32, 5 ) );
        UInt32 failures32 = 0;
        for ( Int32 degrees = -180; degrees <= 180; ++degrees )
        {
            Float32 gains32[ 5 ];
            panner32.ComputeGains( TFloat32Radian( (Float32) ToRadians( (Float64) degrees ) ), gains32, 2.0f );
            if ( SmallestGain( gains32, 5 ) < 0.0 || !Near( Power( gains32, 5 ), 2.0, 1e-5 ) || CountNonZero( gains32, 5, 1e-6 ) > 2 )
                ++failures32;
        }
        ASSERT_TRUE( 0 == failures32 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vbap3D: eight cube corners: a source at a speaker gives that speaker alone; 10^4 seeded directions: gains >= -1e-6, power = target, the sum of the speaker vectors points at the source" )
    try
    {
        // acceptance 3
        TFloat64Radian azimuths[ 8 ];
        TFloat64Radian elevations[ 8 ];
        TFloat64Vector3D vectors[ 8 ];
        CubeLayout( azimuths, elevations, vectors );
        TFloat64Vbap3D panner;
        ASSERT_TRUE( !panner.IsPrepared() && 0 == panner.GetTriangleCount() );
        ASSERT_TRUE( panner.Prepare( azimuths, elevations, 8 ) && panner.IsPrepared() && 8 == panner.GetSpeakerCount() );
        // all eight corners surround the listener: no virtual speakers, 2 V - 4 triangles (every face of the cube is split in two)
        ASSERT_TRUE( 0 == panner.GetVirtualSpeakerCount() && 12 == panner.GetTriangleCount() );
        // every triangle uses real speakers only, three different ones
        for ( UInt32 t = 0; t < panner.GetTriangleCount(); ++t )
        {
            UInt32 a = 99, b = 99, c = 99;
            ASSERT_TRUE( panner.GetTriangle( t, a, b, c ) && a < 8 && b < 8 && c < 8 && a != b && b != c && a != c );
        }
        UInt32 d = 0;
        ASSERT_FALSE( panner.GetTriangle( 12, d, d, d ) );

        for ( UInt32 s = 0; s < 8; ++s )
        {
            Float64 gains[ 8 ];
            panner.ComputeGains( TFloat64DirectionVector3D( vectors[ s ] ), gains, 2.0 );
            for ( UInt32 i = 0; i < 8; ++i )
                ASSERT_TRUE( Near( gains[ i ], s == i ? sqrt( 2.0 ) : 0.0, 1e-9 ) );
            // the angle form gives the same
            Float64 byAngles[ 8 ];
            panner.ComputeGains( azimuths[ s ], elevations[ s ], byAngles, 2.0 );
            for ( UInt32 i = 0; i < 8; ++i )
                ASSERT_TRUE( Near( byAngles[ i ], gains[ i ], 1e-9 ) );
        }

        const UInt64 seed = 0x3D08;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        Float64 smallest = 1.0;
        Float64 worstPower = 0.0;
        Float64 worstReconstruction = 0.0;
        for ( UInt32 i = 0; i < 10000; ++i )
        {
            const TFloat64DirectionVector3D direction = RandomDirection( generator );
            const Float64 target = 2.0;
            Float64 gains[ 8 ];
            panner.ComputeGains( direction, gains, target );
            const Float64 low = SmallestGain( gains, 8 );
            const Float64 powerError = fabs( Power( gains, 8 ) - target );
            const Float64 reconstruction = ReconstructionError( vectors, gains, 8, direction.AsVector() );
            if ( low < smallest ) smallest = low;
            if ( powerError > worstPower ) worstPower = powerError;
            if ( reconstruction > worstReconstruction ) worstReconstruction = reconstruction;
            if ( low < -1e-6 || powerError > 1e-5 || reconstruction > 1e-6 || CountNonZero( gains, 8, 1e-12 ) > 3 )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Vbap: " << failures << " 3D cube checks failed, seed " << seed << ", smallest gain " << smallest << ", worst power error " << worstPower << ", worst reconstruction " << worstReconstruction );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Vbap: cube, smallest gain " << smallest << ", worst power error " << worstPower << ", worst reconstruction error " << worstReconstruction );

        // other target powers
        Float64 gains[ 8 ];
        panner.ComputeGains( RandomDirection( generator ), gains, 0.125 );
        ASSERT_TRUE( Near( Power( gains, 8 ), 0.125, 1e-9 ) );
        panner.ComputeGains( RandomDirection( generator ), gains, 0.0 );
        ASSERT_TRUE( 0.0 == Power( gains, 8 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vbap3D: a horizontal ring gets a virtual nadir and zenith; on the horizon it gives the gains of the 2D panner" )
    try
    {
        const Float64 ring[ 5 ] = { -110.0, -30.0, 0.0, 30.0, 110.0 };
        TFloat64Radian azimuths[ 5 ];
        TFloat64Radian elevations[ 5 ];
        TFloat64Vector3D vectors[ 5 ];
        for ( UInt32 i = 0; i < 5; ++i )
        {
            azimuths[ i ] = TFloat64Radian( ToRadians( ring[ i ] ) );
            elevations[ i ] = TFloat64Radian( 0.0 );
            vectors[ i ] = SpeakerVector( ToRadians( ring[ i ] ), 0.0 );
        }
        TFloat64Vbap3D dome;
        TFloat64Vbap2D flat;
        ASSERT_TRUE( dome.Prepare( azimuths, elevations, 5 ) && flat.Prepare( azimuths, 5 ) );
        // five speakers and the two virtual speakers: 2 * 7 - 4 triangles
        ASSERT_TRUE( 2 == dome.GetVirtualSpeakerCount() && 10 == dome.GetTriangleCount() );
        // the virtual speakers are the indices after the real ones: the nadir first
        UInt32 virtualTriangles = 0;
        for ( UInt32 t = 0; t < dome.GetTriangleCount(); ++t )
        {
            UInt32 a = 0, b = 0, c = 0;
            ASSERT_TRUE( dome.GetTriangle( t, a, b, c ) );
            if ( a >= 5 || b >= 5 || c >= 5 )
                ++virtualTriangles;
            ASSERT_TRUE( a < 7 && b < 7 && c < 7 );
        }
        ASSERT_TRUE( 10 == virtualTriangles );

        UInt32 failures = 0;
        for ( Int32 degrees = -180; degrees <= 180; ++degrees )
        {
            Float64 gains3D[ 5 ];
            Float64 gains2D[ 5 ];
            const Float64 a = ToRadians( (Float64) degrees );
            dome.ComputeGains( TFloat64Radian( a ), TFloat64Radian( 0.0 ), gains3D, 2.0 );
            flat.ComputeGains( TFloat64Radian( a ), gains2D, 2.0 );
            for ( UInt32 i = 0; i < 5; ++i )
                if ( !Near( gains3D[ i ], gains2D[ i ], 1e-7 ) )
                    ++failures;
        }
        ASSERT_TRUE( 0 == failures );

        // a source at a speaker: that speaker alone; above and below the ring: finite, non-negative, power = target, also straight up and down
        for ( UInt32 s = 0; s < 5; ++s )
        {
            Float64 gains[ 5 ];
            dome.ComputeGains( azimuths[ s ], elevations[ s ], gains, 2.0 );
            for ( UInt32 i = 0; i < 5; ++i )
                ASSERT_TRUE( Near( gains[ i ], s == i ? sqrt( 2.0 ) : 0.0, 1e-7 ) );
        }
        Float64 up[ 5 ];
        Float64 down[ 5 ];
        dome.ComputeGains( TFloat64DirectionVector3D::Up(), up, 2.0 );
        dome.ComputeGains( TFloat64DirectionVector3D::Down(), down, 2.0 );
        // a source straight up or down is shared by the whole ring, equally by the speakers that surround the virtual one
        for ( UInt32 i = 0; i < 5; ++i )
            ASSERT_TRUE( Near( up[ i ], sqrt( 2.0 / 5.0 ), 1e-9 ) && Near( down[ i ], sqrt( 2.0 / 5.0 ), 1e-9 ) );

        const UInt64 seed = 0xD073;
        MathReference::CSeededSequence generator( seed );
        UInt32 badGains = 0;
        for ( UInt32 i = 0; i < 5000; ++i )
        {
            Float64 gains[ 5 ];
            dome.ComputeGains( RandomDirection( generator ), gains, 2.0 );
            if ( SmallestGain( gains, 5 ) < 0.0 || !Near( Power( gains, 5 ), 2.0, 1e-9 ) )
                ++badGains;
        }
        ASSERT_TRUE( 0 == badGains );

        // continuity along a meridian that crosses both poles and along a circle of elevation: no gain jumps
        Float64 previous[ 5 ];
        Float64 largestJump = 0.0;
        for ( UInt32 step = 0; step <= 7200; ++step )
        {
            // the great circle through the zenith, 0.05 degrees per step: elevation goes up to 90 and over to the back at azimuth 180
            const Float64 t = 2.0 * TestPi * (Float64) step / 7200.0;
            const TFloat64Vector3D v( 0.0, sin( t + 0.3 ), -cos( t + 0.3 ) );
            Float64 gains[ 5 ];
            dome.ComputeGains( TFloat64DirectionVector3D( v ), gains, 2.0 );
            if ( step > 0 )
                for ( UInt32 i = 0; i < 5; ++i )
                    if ( fabs( gains[ i ] - previous[ i ] ) > largestJump )
                        largestJump = fabs( gains[ i ] - previous[ i ] );
            for ( UInt32 i = 0; i < 5; ++i )
                previous[ i ] = gains[ i ];
        }
        ASSERT_TRUE( largestJump < 0.02 );
        largestJump = 0.0;
        for ( UInt32 step = 0; step <= 7200; ++step )
        {
            const Float64 a = 2.0 * TestPi * (Float64) step / 7200.0;
            const Float64 e = ToRadians( -35.0 );
            Float64 gains[ 5 ];
            dome.ComputeGains( TFloat64Radian( a ), TFloat64Radian( e ), gains, 2.0 );
            if ( step > 0 )
                for ( UInt32 i = 0; i < 5; ++i )
                    if ( fabs( gains[ i ] - previous[ i ] ) > largestJump )
                        largestJump = fabs( gains[ i ] - previous[ i ] );
            for ( UInt32 i = 0; i < 5; ++i )
                previous[ i ] = gains[ i ];
        }
        ASSERT_TRUE( largestJump < 0.02 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Vbap: ring with virtual speakers, largest gain step of 0.05 degrees " << largestJump );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vbap3D: speakers in the upper hemisphere only get a virtual nadir: finite normalised gains for sources below the horizon" )
    try
    {
        // acceptance 3
        TFloat64Radian azimuths[ 10 ];
        TFloat64Radian elevations[ 10 ];
        for ( UInt32 i = 0; i < 6; ++i )
        {
            azimuths[ i ] = TFloat64Radian( ToRadians( 60.0 * (Float64) i ) );
            elevations[ i ] = TFloat64Radian( ToRadians( 20.0 ) );
        }
        for ( UInt32 i = 0; i < 4; ++i )
        {
            azimuths[ 6 + i ] = TFloat64Radian( ToRadians( 90.0 * (Float64) i + 45.0 ) );
            elevations[ 6 + i ] = TFloat64Radian( ToRadians( 60.0 ) );
        }
        TFloat64Vbap3D panner;
        ASSERT_TRUE( panner.Prepare( azimuths, elevations, 10 ) );
        ASSERT_TRUE( 1 == panner.GetVirtualSpeakerCount() && 2 * 11 - 4 == (Int32) panner.GetTriangleCount() );

        const UInt64 seed = 0x70B0;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        UInt32 below = 0;
        for ( UInt32 i = 0; i < 5000; ++i )
        {
            const TFloat64DirectionVector3D direction = RandomDirection( generator );
            Float64 gains[ 10 ];
            const Float64 target = 2.0;
            panner.ComputeGains( direction, gains, target );
            bool finite = true;
            for ( UInt32 j = 0; j < 10; ++j )
                finite = finite && CTNumericTraits< Float64 >::IsFinite( gains[ j ] );
            if ( direction.GetY() < 0.0 )
                ++below;
            if ( !finite || SmallestGain( gains, 10 ) < 0.0 || !Near( Power( gains, 10 ), target, 1e-9 ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures && below > 2000 );
        // straight down: the lowest ring shares the source, the upper speakers do not take part
        Float64 gains[ 10 ];
        panner.ComputeGains( TFloat64DirectionVector3D::Down(), gains, 2.0 );
        ASSERT_TRUE( Near( Power( gains, 10 ), 2.0, 1e-9 ) );
        for ( UInt32 i = 0; i < 6; ++i )
            ASSERT_TRUE( gains[ i ] > 0.1 );
        // and a source at a speaker is that speaker alone
        for ( UInt32 s = 0; s < 10; ++s )
        {
            panner.ComputeGains( azimuths[ s ], elevations[ s ], gains, 2.0 );
            for ( UInt32 i = 0; i < 10; ++i )
                ASSERT_TRUE( Near( gains[ i ], s == i ? sqrt( 2.0 ) : 0.0, 1e-7 ) );
        }
        // a single speaker above and none below, and the mirror: a virtual zenith
        const TFloat64Radian lowAzimuths[ 4 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( ToRadians( 90.0 ) ), TFloat64Radian( ToRadians( 180.0 ) ), TFloat64Radian( ToRadians( 270.0 ) ) };
        const TFloat64Radian lowElevations[ 4 ] = { TFloat64Radian( ToRadians( -30.0 ) ), TFloat64Radian( ToRadians( -30.0 ) ), TFloat64Radian( ToRadians( -30.0 ) ), TFloat64Radian( ToRadians( -30.0 ) ) };
        TFloat64Vbap3D lower;
        ASSERT_TRUE( lower.Prepare( lowAzimuths, lowElevations, 4 ) && 1 == lower.GetVirtualSpeakerCount() );
        Float64 lowerGains[ 4 ];
        lower.ComputeGains( TFloat64DirectionVector3D::Up(), lowerGains, 2.0 );
        ASSERT_TRUE( Near( Power( lowerGains, 4 ), 2.0, 1e-9 ) && SmallestGain( lowerGains, 4 ) > 0.1 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vbap3D: spheres of 16, 24 and 64 speakers: 2 V - 4 triangles, every direction found, the panning property, speakers in a random order" )
    try
    {
        const UInt32 counts[ 3 ] = { 16, 24, 64 };
        const UInt64 seed = 0x5F1B;
        MathReference::CSeededSequence generator( seed );
        for ( UInt32 c = 0; c < 3; ++c )
        {
            const UInt32 count = counts[ c ];
            TFloat64Radian azimuths[ 64 ];
            TFloat64Radian elevations[ 64 ];
            TFloat64Vector3D vectors[ 64 ];
            FibonacciLayout( count, azimuths, elevations, vectors );
            TFloat64Vbap3D panner;
            ASSERT_TRUE( panner.Prepare( azimuths, elevations, count ) );
            ASSERT_TRUE( 0 == panner.GetVirtualSpeakerCount() && 2 * count - 4 == panner.GetTriangleCount() );
            UInt32 failures = 0;
            for ( UInt32 i = 0; i < 3000; ++i )
            {
                const TFloat64DirectionVector3D direction = RandomDirection( generator );
                Float64 gains[ 64 ];
                panner.ComputeGains( direction, gains, 2.0 );
                if ( SmallestGain( gains, count ) < -1e-6 || !Near( Power( gains, count ), 2.0, 1e-5 ) || CountNonZero( gains, count, 1e-12 ) > 3 ||
                     ReconstructionError( vectors, gains, count, direction.AsVector() ) > 1e-6 )
                    ++failures;
            }
            for ( UInt32 s = 0; s < count; ++s )
            {
                Float64 gains[ 64 ];
                panner.ComputeGains( TFloat64DirectionVector3D( vectors[ s ] ), gains, 2.0 );
                for ( UInt32 i = 0; i < count; ++i )
                    if ( !Near( gains[ i ], s == i ? sqrt( 2.0 ) : 0.0, 1e-6 ) )
                        ++failures;
            }
            ASSERT_TRUE( 0 == failures );
            if ( 0 != failures )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Vbap: " << failures << " checks of the sphere with " << count << " speakers failed, seed " << seed );

            // the speakers in another order give the same gains for the same speakers
            UInt32 order[ 64 ];
            for ( UInt32 i = 0; i < count; ++i )
                order[ i ] = i;
            for ( UInt32 i = count - 1; i > 0; --i )
            {
                const UInt32 j = generator.NextUInt32() % ( i + 1 );
                const UInt32 swap = order[ i ];
                order[ i ] = order[ j ];
                order[ j ] = swap;
            }
            TFloat64Radian mixedAzimuths[ 64 ];
            TFloat64Radian mixedElevations[ 64 ];
            for ( UInt32 i = 0; i < count; ++i )
            {
                mixedAzimuths[ i ] = azimuths[ order[ i ] ];
                mixedElevations[ i ] = elevations[ order[ i ] ];
            }
            TFloat64Vbap3D mixed;
            ASSERT_TRUE( mixed.Prepare( mixedAzimuths, mixedElevations, count ) && mixed.GetTriangleCount() == panner.GetTriangleCount() );
            UInt32 differences = 0;
            for ( UInt32 i = 0; i < 500; ++i )
            {
                const TFloat64DirectionVector3D direction = RandomDirection( generator );
                Float64 a[ 64 ];
                Float64 b[ 64 ];
                panner.ComputeGains( direction, a, 2.0 );
                mixed.ComputeGains( direction, b, 2.0 );
                // the hulls agree up to the triangulation of coplanar speakers; a Fibonacci sphere has none, so the gains agree
                for ( UInt32 j = 0; j < count; ++j )
                    if ( !Near( b[ j ], a[ order[ j ] ], 1e-6 ) )
                        ++differences;
            }
            ASSERT_TRUE( 0 == differences );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vbap3D: speakers in one plane through the listener get both virtual speakers; two speakers; Prepare rejections" )
    try
    {
        // five speakers on a ring that is tilted by 20 degrees: all in one plane through the listener
        TFloat64Radian azimuths[ 5 ];
        TFloat64Radian elevations[ 5 ];
        TFloat64Vector3D vectors[ 5 ];
        const Float64 tilt = ToRadians( 20.0 );
        for ( UInt32 i = 0; i < 5; ++i )
        {
            const Float64 phi = 2.0 * TestPi * (Float64) i / 5.0 + 0.2;
            const TFloat64Vector3D v( cos( phi ), sin( phi ) * sin( tilt ), sin( phi ) * cos( tilt ) );
            vectors[ i ] = v;
            ToAngles( v, azimuths[ i ], elevations[ i ] );
        }
        TFloat64Vbap3D tilted;
        ASSERT_TRUE( tilted.Prepare( azimuths, elevations, 5 ) && 2 == tilted.GetVirtualSpeakerCount() );
        for ( UInt32 s = 0; s < 5; ++s )
        {
            Float64 gains[ 5 ];
            tilted.ComputeGains( TFloat64DirectionVector3D( vectors[ s ] ), gains, 2.0 );
            for ( UInt32 i = 0; i < 5; ++i )
                ASSERT_TRUE( Near( gains[ i ], s == i ? sqrt( 2.0 ) : 0.0, 1e-6 ) );
        }
        const UInt64 seed = 0x7117;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i = 0; i < 2000; ++i )
        {
            Float64 gains[ 5 ];
            tilted.ComputeGains( RandomDirection( generator ), gains, 2.0 );
            if ( SmallestGain( gains, 5 ) < 0.0 || !Near( Power( gains, 5 ), 2.0, 1e-9 ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );

        // stereo in three dimensions: Prepare works, a source at a speaker is that speaker alone, every other direction gets finite normalised gains
        const TFloat64Radian stereoAzimuths[ 2 ] = { TFloat64Radian( ToRadians( -30.0 ) ), TFloat64Radian( ToRadians( 30.0 ) ) };
        const TFloat64Radian stereoElevations[ 2 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( 0.0 ) };
        TFloat64Vbap3D stereo;
        ASSERT_TRUE( stereo.Prepare( stereoAzimuths, stereoElevations, 2 ) );
        Float64 stereoGains[ 2 ];
        stereo.ComputeGains( stereoAzimuths[ 1 ], stereoElevations[ 1 ], stereoGains, 2.0 );
        ASSERT_TRUE( Near( stereoGains[ 0 ], 0.0, 1e-7 ) && Near( stereoGains[ 1 ], sqrt( 2.0 ), 1e-7 ) );
        UInt32 badStereo = 0;
        for ( UInt32 i = 0; i < 2000; ++i )
        {
            stereo.ComputeGains( RandomDirection( generator ), stereoGains, 2.0 );
            if ( SmallestGain( stereoGains, 2 ) < 0.0 || !Near( Power( stereoGains, 2 ), 2.0, 1e-9 ) )
                ++badStereo;
        }
        ASSERT_TRUE( 0 == badStereo );

        // rejections: too few and too many speakers, null pointers, coincident speakers, not finite angles, a single speaker
        TFloat64Vbap3D panner;
        TFloat64Radian a64[ 70 ];
        TFloat64Radian e64[ 70 ];
        TFloat64Vector3D unused[ 70 ];
        FibonacciLayout( 64, a64, e64, unused );
        ASSERT_TRUE( panner.Prepare( a64, e64, 64 ) && 64 == panner.GetSpeakerCount() );
        ASSERT_FALSE( panner.Prepare( a64, e64, 65 ) );
        ASSERT_TRUE( 0 == panner.GetSpeakerCount() && 0 == panner.GetTriangleCount() && 0 == panner.GetVirtualSpeakerCount() );
        ASSERT_FALSE( panner.Prepare( a64, e64, 1 ) );
        ASSERT_FALSE( panner.Prepare( a64, e64, 0 ) );
        ASSERT_FALSE( panner.Prepare( GUCEF_NULL, e64, 8 ) );
        ASSERT_FALSE( panner.Prepare( a64, GUCEF_NULL, 8 ) );
        TFloat64Radian duplicateA[ 8 ];
        TFloat64Radian duplicateE[ 8 ];
        for ( UInt32 i = 0; i < 8; ++i )
        {
            duplicateA[ i ] = a64[ i ];
            duplicateE[ i ] = e64[ i ];
        }
        duplicateA[ 5 ] = duplicateA[ 2 ];
        duplicateE[ 5 ] = duplicateE[ 2 ];
        ASSERT_FALSE( panner.Prepare( duplicateA, duplicateE, 8 ) );
        // the same direction given with a full turn added to the azimuth
        duplicateA[ 5 ] = TFloat64Radian( duplicateA[ 2 ].GetValue() + 2.0 * TestPi );
        ASSERT_FALSE( panner.Prepare( duplicateA, duplicateE, 8 ) );
        const Float64 nan = CTNumericTraits< Float64 >::Max() * 2.0 - CTNumericTraits< Float64 >::Max() * 2.0;
        duplicateA[ 5 ] = a64[ 5 ];
        duplicateE[ 5 ] = TFloat64Radian( nan );
        ASSERT_FALSE( panner.Prepare( duplicateA, duplicateE, 8 ) );
        ASSERT_TRUE( 0 == panner.GetSpeakerCount() );
        // an unprepared panner writes nothing, a non-positive target writes zeros
        Float64 gains[ 64 ];
        for ( UInt32 i = 0; i < 64; ++i )
            gains[ i ] = 9.0;
        panner.ComputeGains( TFloat64DirectionVector3D::Up(), gains, 2.0 );
        ASSERT_TRUE( 9.0 == gains[ 0 ] && 9.0 == gains[ 5 ] );
        ASSERT_TRUE( panner.Prepare( a64, e64, 64 ) );
        panner.ComputeGains( TFloat64DirectionVector3D::Up(), gains, -3.0 );
        ASSERT_TRUE( 0.0 == Power( gains, 64 ) );
        ASSERT_TRUE( 64 == (UInt32) TFloat64Vbap3D::MaxSpeakers && 130 == (UInt32) TFloat64Vbap3D::MaxTriangles );
        panner.Clear();
        ASSERT_TRUE( !panner.IsPrepared() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vbap3D: speakers in front only (a cap with a centre speaker): the centre speaker is not silenced at its own direction, whatever its position in the list" )
    try
    {
        // four corners at azimuth +-45 and elevation +-30 and a centre speaker: the corners are coplanar (the back of the cap)
        const Float64 cornerAzimuths[ 4 ] = { -45.0, 45.0, -45.0, 45.0 };
        const Float64 cornerElevations[ 4 ] = { 30.0, 30.0, -30.0, -30.0 };
        const UInt64 seed = 0xCA9;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 permutation = 0; permutation < 60; ++permutation )
        {
            // permutation 0: the centre speaker last, 1: first, then seeded random orders
            UInt32 order[ 5 ] = { 0, 1, 2, 3, 4 };         // the speaker at each list position: 4 is the centre speaker
            if ( 1 == permutation )
            {
                order[ 0 ] = 4; order[ 1 ] = 0; order[ 2 ] = 1; order[ 3 ] = 2; order[ 4 ] = 3;
            }
            else if ( permutation > 1 )
            {
                for ( UInt32 i = 4; i > 0; --i )
                {
                    const UInt32 j = generator.NextUInt32() % ( i + 1 );
                    const UInt32 swap = order[ i ];
                    order[ i ] = order[ j ];
                    order[ j ] = swap;
                }
            }
            TFloat64Radian azimuths[ 5 ];
            TFloat64Radian elevations[ 5 ];
            TFloat64Vector3D vectors[ 5 ];
            for ( UInt32 i = 0; i < 5; ++i )
            {
                const Float64 az = 4 == order[ i ] ? 0.0 : cornerAzimuths[ order[ i ] ];
                const Float64 el = 4 == order[ i ] ? 0.0 : cornerElevations[ order[ i ] ];
                azimuths[ i ] = TFloat64Radian( ToRadians( az ) );
                elevations[ i ] = TFloat64Radian( ToRadians( el ) );
                vectors[ i ] = SpeakerVector( ToRadians( az ), ToRadians( el ) );
            }
            TFloat64Vbap3D panner;
            if ( !panner.Prepare( azimuths, elevations, 5 ) || 0 != panner.GetVirtualSpeakerCount() || 4 != panner.GetTriangleCount() )
            {
                ++failures;
                continue;
            }
            // a source at each speaker is that speaker alone
            for ( UInt32 s = 0; s < 5; ++s )
            {
                Float64 gains[ 5 ];
                panner.ComputeGains( azimuths[ s ], elevations[ s ], gains, 2.0 );
                for ( UInt32 i = 0; i < 5; ++i )
                    if ( !Near( gains[ i ], s == i ? sqrt( 2.0 ) : 0.0, 1e-6 ) )
                        ++failures;
            }
            // a source inside the cap: the speakers of one front triangle, the panning property, and the same in every order
            Float64 gains[ 5 ];
            const TFloat64Radian sourceAzimuth( ToRadians( 10.0 ) );
            const TFloat64Radian sourceElevation( ToRadians( 5.0 ) );
            panner.ComputeGains( sourceAzimuth, sourceElevation, gains, 2.0 );
            const TFloat64Vector3D source = SpeakerVector( ToRadians( 10.0 ), ToRadians( 5.0 ) );
            UInt32 centrePosition = 0;
            for ( UInt32 i = 0; i < 5; ++i )
                if ( 4 == order[ i ] )
                    centrePosition = i;
            if ( SmallestGain( gains, 5 ) < 0.0 || !Near( Power( gains, 5 ), 2.0, 1e-9 ) || CountNonZero( gains, 5, 1e-12 ) > 3 ||
                 !( gains[ centrePosition ] > 0.1 ) || ReconstructionError( vectors, gains, 5, source ) > 1e-6 )
                ++failures;
            // a source behind the listener: no triangle contains it, the gains are still finite, non-negative and normalised
            panner.ComputeGains( TFloat64Radian( ToRadians( 170.0 ) ), TFloat64Radian( 0.0 ), gains, 2.0 );
            if ( SmallestGain( gains, 5 ) < 0.0 || !Near( Power( gains, 5 ), 2.0, 1e-9 ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Vbap: " << failures << " checks of the front cap failed, seed " << seed );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vbap3D: the gains do not depend on the order of the speakers (seeded layouts: cube, spheres, random layouts with and without virtual speakers, a median plane ring)" )
    try
    {
        const UInt64 seed = 0x0DE7;
        MathReference::CSeededSequence generator( seed );
        UInt32 differences = 0;

        // the cube has coplanar faces: the triangulation of a face must not depend on the order either
        TFloat64Radian cubeA[ 8 ];
        TFloat64Radian cubeE[ 8 ];
        TFloat64Vector3D cubeV[ 8 ];
        CubeLayout( cubeA, cubeE, cubeV );
        differences += CountOrderDifferences( cubeA, cubeE, 8, 40, 400, generator, 1e-9 );

        // Fibonacci spheres, and the same with the lower half removed (a virtual nadir)
        TFloat64Radian fa[ 64 ];
        TFloat64Radian fe[ 64 ];
        TFloat64Vector3D fv[ 64 ];
        FibonacciLayout( 24, fa, fe, fv );
        differences += CountOrderDifferences( fa, fe, 24, 20, 300, generator, 1e-9 );
        TFloat64Radian ha[ 64 ];
        TFloat64Radian he[ 64 ];
        UInt32 upper = 0;
        for ( UInt32 i = 0; i < 24; ++i )
        {
            if ( fv[ i ].y > 0.2 )
            {
                ha[ upper ] = fa[ i ];
                he[ upper ] = fe[ i ];
                ++upper;
            }
        }
        differences += CountOrderDifferences( ha, he, upper, 20, 300, generator, 1e-9 );

        // seeded random layouts of 4 to 40 speakers (generic position: no coplanar facets), some of them not surrounding the listener
        for ( UInt32 layout = 0; layout < 40; ++layout )
        {
            const UInt32 count = 4 + generator.NextUInt32() % 37;
            TFloat64Radian ra[ 64 ];
            TFloat64Radian re[ 64 ];
            for ( UInt32 i = 0; i < count; ++i )
            {
                const TFloat64DirectionVector3D d = RandomDirection( generator );
                ToAngles( d.AsVector(), ra[ i ], re[ i ] );
            }
            differences += CountOrderDifferences( ra, re, count, 4, 100, generator, 1e-9 );
        }

        // a median plane ring (the plane contains the Y axis), and a ring in the side plane
        const TFloat64Radian ma[ 4 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( 0.0 ), TFloat64Radian( TestPi ), TFloat64Radian( TestPi ) };
        const TFloat64Radian me[ 4 ] = { TFloat64Radian( ToRadians( 45.0 ) ), TFloat64Radian( -ToRadians( 45.0 ) ), TFloat64Radian( ToRadians( 45.0 ) ), TFloat64Radian( -ToRadians( 45.0 ) ) };
        differences += CountOrderDifferences( ma, me, 4, 12, 300, generator, 1e-9 );
        ASSERT_TRUE( 0 == differences );
        if ( 0 != differences )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Vbap: " << differences << " gain differences between speaker orders, seed " << seed );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vbap3D: planar layouts whose plane contains the Y axis (median plane ring, a vertical pair, the side plane) are prepared with virtual speakers on the plane normal; opposite speakers alone are rejected" )
    try
    {
        const UInt64 seed = 0x91A4;
        MathReference::CSeededSequence generator( seed );

        // a ring of four speakers in the median plane: a bipyramid of 8 triangles around the listener
        {
            const TFloat64Radian azimuths[ 4 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( 0.0 ), TFloat64Radian( TestPi ), TFloat64Radian( TestPi ) };
            const TFloat64Radian elevations[ 4 ] = { TFloat64Radian( ToRadians( 45.0 ) ), TFloat64Radian( -ToRadians( 45.0 ) ), TFloat64Radian( ToRadians( 45.0 ) ), TFloat64Radian( -ToRadians( 45.0 ) ) };
            TFloat64Vbap3D panner;
            ASSERT_TRUE( panner.Prepare( azimuths, elevations, 4 ) );
            ASSERT_TRUE( 2 == panner.GetVirtualSpeakerCount() && 8 == panner.GetTriangleCount() );
            UInt32 failures = 0;
            for ( UInt32 s = 0; s < 4; ++s )
            {
                Float64 gains[ 4 ];
                panner.ComputeGains( azimuths[ s ], elevations[ s ], gains, 2.0 );
                for ( UInt32 i = 0; i < 4; ++i )
                    if ( !Near( gains[ i ], s == i ? sqrt( 2.0 ) : 0.0, 1e-6 ) )
                        ++failures;
            }
            for ( UInt32 i = 0; i < 3000; ++i )
            {
                Float64 gains[ 4 ];
                panner.ComputeGains( RandomDirection( generator ), gains, 2.0 );
                if ( SmallestGain( gains, 4 ) < 0.0 || !Near( Power( gains, 4 ), 2.0, 1e-9 ) )
                    ++failures;
            }
            // directions in the plane between two speakers use those two speakers (the virtual speakers are 90 degrees off the plane)
            Float64 gains[ 4 ];
            panner.ComputeGains( TFloat64Radian( 0.0 ), TFloat64Radian( 0.0 ), gains, 2.0 );
            if ( !Near( gains[ 0 ], gains[ 1 ], 1e-9 ) || !( gains[ 0 ] > 0.5 ) )
                ++failures;
            ASSERT_TRUE( 0 == failures );
            if ( 0 != failures )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Vbap: " << failures << " checks of the median plane ring failed, seed " << seed );
        }

        // a vertical pair (one above the other, ahead): prepared, each speaker is itself, the direction between them shares equally
        {
            const TFloat64Radian azimuths[ 2 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( 0.0 ) };
            const TFloat64Radian elevations[ 2 ] = { TFloat64Radian( ToRadians( 40.0 ) ), TFloat64Radian( -ToRadians( 40.0 ) ) };
            TFloat64Vbap3D panner;
            ASSERT_TRUE( panner.Prepare( azimuths, elevations, 2 ) && 2 == panner.GetVirtualSpeakerCount() && panner.GetTriangleCount() >= 2 );
            Float64 gains[ 2 ];
            panner.ComputeGains( azimuths[ 0 ], elevations[ 0 ], gains, 2.0 );
            ASSERT_TRUE( Near( gains[ 0 ], sqrt( 2.0 ), 1e-6 ) && Near( gains[ 1 ], 0.0, 1e-6 ) );
            panner.ComputeGains( azimuths[ 1 ], elevations[ 1 ], gains, 2.0 );
            ASSERT_TRUE( Near( gains[ 1 ], sqrt( 2.0 ), 1e-6 ) && Near( gains[ 0 ], 0.0, 1e-6 ) );
            panner.ComputeGains( TFloat64Radian( 0.0 ), TFloat64Radian( 0.0 ), gains, 2.0 );
            ASSERT_TRUE( Near( gains[ 0 ], gains[ 1 ], 1e-9 ) && Near( Power( gains, 2 ), 2.0, 1e-9 ) );
            UInt32 bad = 0;
            for ( UInt32 i = 0; i < 2000; ++i )
            {
                panner.ComputeGains( RandomDirection( generator ), gains, 2.0 );
                if ( SmallestGain( gains, 2 ) < 0.0 || !Near( Power( gains, 2 ), 2.0, 1e-9 ) )
                    ++bad;
            }
            ASSERT_TRUE( 0 == bad );
        }

        // a ring in the side plane (x = 0 is the median plane, z = 0 the side plane): four speakers at azimuth +-90 and elevation +-45
        {
            const TFloat64Radian azimuths[ 4 ] = { TFloat64Radian( -ToRadians( 90.0 ) ), TFloat64Radian( ToRadians( 90.0 ) ), TFloat64Radian( -ToRadians( 90.0 ) ), TFloat64Radian( ToRadians( 90.0 ) ) };
            const TFloat64Radian elevations[ 4 ] = { TFloat64Radian( ToRadians( 45.0 ) ), TFloat64Radian( ToRadians( 45.0 ) ), TFloat64Radian( -ToRadians( 45.0 ) ), TFloat64Radian( -ToRadians( 45.0 ) ) };
            TFloat64Vbap3D panner;
            ASSERT_TRUE( panner.Prepare( azimuths, elevations, 4 ) && 2 == panner.GetVirtualSpeakerCount() && 8 == panner.GetTriangleCount() );
            UInt32 failures = 0;
            for ( UInt32 s = 0; s < 4; ++s )
            {
                Float64 gains[ 4 ];
                panner.ComputeGains( azimuths[ s ], elevations[ s ], gains, 2.0 );
                for ( UInt32 i = 0; i < 4; ++i )
                    if ( !Near( gains[ i ], s == i ? sqrt( 2.0 ) : 0.0, 1e-6 ) )
                        ++failures;
            }
            ASSERT_TRUE( 0 == failures );
        }

        // opposite speakers alone (front and back, left and right, up and down): the listener is on the line between them, no solid exists
        {
            TFloat64Vbap3D panner;
            const TFloat64Radian frontBack[ 2 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( TestPi ) };
            const TFloat64Radian level[ 2 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( 0.0 ) };
            ASSERT_FALSE( panner.Prepare( frontBack, level, 2 ) );
            ASSERT_TRUE( !panner.IsPrepared() && 0 == panner.GetTriangleCount() );
            const TFloat64Radian leftRight[ 2 ] = { TFloat64Radian( -ToRadians( 90.0 ) ), TFloat64Radian( ToRadians( 90.0 ) ) };
            ASSERT_FALSE( panner.Prepare( leftRight, level, 2 ) );
            const TFloat64Radian upDownA[ 2 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( 0.0 ) };
            const TFloat64Radian upDownE[ 2 ] = { TFloat64Radian( ToRadians( 90.0 ) ), TFloat64Radian( -ToRadians( 90.0 ) ) };
            ASSERT_FALSE( panner.Prepare( upDownA, upDownE, 2 ) );
            // a third speaker off the line makes it a layout again
            const TFloat64Radian threeA[ 3 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( TestPi ), TFloat64Radian( ToRadians( 90.0 ) ) };
            const TFloat64Radian threeE[ 3 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( 0.0 ), TFloat64Radian( 0.0 ) };
            ASSERT_TRUE( panner.Prepare( threeA, threeE, 3 ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vbap3D: seeded random layouts of 4 to 64 speakers: Prepare finds the hull, gains are normalised, a source at a speaker is that speaker alone (spread-out layouts)" )
    try
    {
        const UInt64 seed = 0x6B1D;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 layout = 0; layout < 60; ++layout )
        {
            const UInt32 count = 4 + generator.NextUInt32() % 61;
            TFloat64Radian azimuths[ 64 ];
            TFloat64Radian elevations[ 64 ];
            TFloat64Vector3D vectors[ 64 ];
            for ( UInt32 i = 0; i < count; ++i )
            {
                const TFloat64DirectionVector3D d = RandomDirection( generator );
                vectors[ i ] = d.AsVector();
                ToAngles( vectors[ i ], azimuths[ i ], elevations[ i ] );
            }
            TFloat64Vbap3D panner;
            if ( !panner.Prepare( azimuths, elevations, count ) )
            {
                ++failures;
                continue;
            }
            for ( UInt32 i = 0; i < 200; ++i )
            {
                Float64 gains[ 64 ];
                panner.ComputeGains( RandomDirection( generator ), gains, 2.0 );
                if ( SmallestGain( gains, count ) < 0.0 || !Near( Power( gains, count ), 2.0, 1e-9 ) )
                    ++failures;
            }
            for ( UInt32 s = 0; s < count; ++s )
            {
                Float64 gains[ 64 ];
                panner.ComputeGains( azimuths[ s ], elevations[ s ], gains, 2.0 );
                // a speaker next to a virtual speaker is shared with the neighbours of it only when the source is at the virtual speaker itself
                if ( !Near( gains[ s ], sqrt( 2.0 ), 1e-6 ) )
                    ++failures;
            }
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Vbap: " << failures << " checks of random layouts failed, seed " << seed );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vbap3D: Float32 layouts: cube corners and a 24 speaker sphere" )
    try
    {
        TFloat64Radian a64[ 64 ];
        TFloat64Radian e64[ 64 ];
        TFloat64Vector3D vectors[ 64 ];
        TFloat32Radian azimuths[ 24 ];
        TFloat32Radian elevations[ 24 ];
        FibonacciLayout( 24, a64, e64, vectors );
        for ( UInt32 i = 0; i < 24; ++i )
        {
            azimuths[ i ] = TFloat32Radian( (Float32) a64[ i ].GetValue() );
            elevations[ i ] = TFloat32Radian( (Float32) e64[ i ].GetValue() );
        }
        TFloat32Vbap3D sphere;
        ASSERT_TRUE( sphere.Prepare( azimuths, elevations, 24 ) && 44 == sphere.GetTriangleCount() );
        const UInt64 seed = 0xF320;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i = 0; i < 2000; ++i )
        {
            const TFloat64DirectionVector3D d = RandomDirection( generator );
            Float32 gains[ 24 ];
            Float64 widened[ 24 ];
            sphere.ComputeGains( TFloat32DirectionVector3D( TFloat32Vector3D( (Float32) d.GetX(), (Float32) d.GetY(), (Float32) d.GetZ() ) ), gains, 2.0f );
            for ( UInt32 j = 0; j < 24; ++j )
                widened[ j ] = (Float64) gains[ j ];
            if ( SmallestGain( gains, 24 ) < -1e-5 || !Near( Power( gains, 24 ), 2.0, 1e-5 ) || ReconstructionError( vectors, widened, 24, d.AsVector() ) > 1e-3 )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );

        // the cube in Float32: the corners are on planar faces within the rounding of the kind, the speaker is alone at its own direction
        TFloat64Radian ca[ 8 ];
        TFloat64Radian ce[ 8 ];
        TFloat64Vector3D cv[ 8 ];
        CubeLayout( ca, ce, cv );
        TFloat32Radian cube32a[ 8 ];
        TFloat32Radian cube32e[ 8 ];
        for ( UInt32 i = 0; i < 8; ++i )
        {
            cube32a[ i ] = TFloat32Radian( (Float32) ca[ i ].GetValue() );
            cube32e[ i ] = TFloat32Radian( (Float32) ce[ i ].GetValue() );
        }
        TFloat32Vbap3D cube;
        ASSERT_TRUE( cube.Prepare( cube32a, cube32e, 8 ) && 12 == cube.GetTriangleCount() && 0 == cube.GetVirtualSpeakerCount() );
        for ( UInt32 s = 0; s < 8; ++s )
        {
            Float32 gains[ 8 ];
            cube.ComputeGains( cube32a[ s ], cube32e[ s ], gains, 2.0f );
            for ( UInt32 i = 0; i < 8; ++i )
                ASSERT_TRUE( Near( gains[ i ], s == i ? sqrt( 2.0 ) : 0.0, 1e-5 ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vbap: Prepare and ComputeGains do not allocate (allocation counter), and the counter does count" )
    GUCEF_TESTFW_SKIP_IF( !MathAllocationCounter::IsAvailable(), "no allocation counter with a sanitizer that replaces the allocator (the RealtimeSanitizer build proves the same in a realtime context)" );
    try
    {
        // acceptance 3: a direct call of malloc is counted, so the counter works in this build
        MathAllocationCounter::StartCounting();
        MathAllocationCounter::AllocateProbe();
        const UInt32 counted = MathAllocationCounter::StopCounting();
        ASSERT_TRUE( 1 == counted );

        // objects and inputs outside the counted region
        TFloat64Radian a64[ 64 ];
        TFloat64Radian e64[ 64 ];
        TFloat64Vector3D vectors[ 64 ];
        FibonacciLayout( 64, a64, e64, vectors );
        TFloat32Radian a32[ 64 ];
        TFloat32Radian e32[ 64 ];
        for ( UInt32 i = 0; i < 64; ++i )
        {
            a32[ i ] = TFloat32Radian( (Float32) a64[ i ].GetValue() );
            e32[ i ] = TFloat32Radian( (Float32) e64[ i ].GetValue() );
        }
        const TFloat64Radian ring[ 5 ] = { TFloat64Radian( ToRadians( -110.0 ) ), TFloat64Radian( ToRadians( -30.0 ) ), TFloat64Radian( 0.0 ), TFloat64Radian( ToRadians( 30.0 ) ), TFloat64Radian( ToRadians( 110.0 ) ) };
        const TFloat32Radian ring32[ 5 ] = { TFloat32Radian( -1.9198622f ), TFloat32Radian( -0.5235988f ), TFloat32Radian( 0.0f ), TFloat32Radian( 0.5235988f ), TFloat32Radian( 1.9198622f ) };
        const TFloat64Radian flat[ 5 ] = { TFloat64Radian( 0.0 ), TFloat64Radian( 0.0 ), TFloat64Radian( 0.0 ), TFloat64Radian( 0.0 ), TFloat64Radian( 0.0 ) };
        TFloat64Vbap2D ring2D;
        TFloat32Vbap2D ring2D32;
        TFloat64Vbap3D sphere;
        TFloat32Vbap3D sphere32;
        TFloat64Vbap3D horizontal;
        Float64 gains[ 64 ];
        Float32 gains32[ 64 ];
        Float64 accumulated = 0.0;

        MathAllocationCounter::StartCounting();
        bool ok = ring2D.Prepare( ring, 5 );
        ok = ok && ring2D32.Prepare( ring32, 5 );
        ok = ok && sphere.Prepare( a64, e64, 64 ) && sphere32.Prepare( a32, e32, 64 );
        ok = ok && horizontal.Prepare( ring, flat, 5 );
        for ( UInt32 i = 0; i < 2000; ++i )
        {
            const Float64 t = (Float64) i * 0.0031;
            ring2D.ComputeGains( TFloat64Radian( t ), gains, 2.0 );
            accumulated += gains[ 0 ];
            ring2D32.ComputeGains( TFloat32Radian( (Float32) t ), gains32, 2.0f );
            accumulated += gains32[ 0 ];
            sphere.ComputeGains( TFloat64Radian( t ), TFloat64Radian( sin( t ) ), gains, 2.0 );
            accumulated += gains[ 7 ];
            sphere32.ComputeGains( TFloat32Radian( (Float32) t ), TFloat32Radian( (Float32) sin( t ) ), gains32, 2.0f );
            accumulated += gains32[ 7 ];
            horizontal.ComputeGains( TFloat64Radian( t ), TFloat64Radian( -0.9 ), gains, 2.0 );
            accumulated += gains[ 3 ];
        }
        const UInt32 allocations = MathAllocationCounter::StopCounting();
        ASSERT_TRUE( ok );
        ASSERT_TRUE( accumulated > 0.0 );
        ASSERT_TRUE( 0 == allocations );
        if ( 0 != allocations )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Vbap: " << allocations << " allocations in Prepare and ComputeGains" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vbap: realtime-safe functions run in a realtime context" )
    try
    {
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( RealtimeVbapWork( 0.3f ) ) );
        ASSERT_TRUE( RealtimeVbapWork( 0.3f ) > 1.0f );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
