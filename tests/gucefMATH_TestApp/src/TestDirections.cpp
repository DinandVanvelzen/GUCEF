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

#ifndef GUCEF_MATH_FRACTIONTYPES_H
#include "gucefMATH_fractiontypes.h"
#define GUCEF_MATH_FRACTIONTYPES_H
#endif /* GUCEF_MATH_FRACTIONTYPES_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_MATH_SCALARFUNCTIONS_H
#include "gucefMATH_ScalarFunctions.h"
#define GUCEF_MATH_SCALARFUNCTIONS_H
#endif /* GUCEF_MATH_SCALARFUNCTIONS_H ? */

#include "TestMathReference.h"
#include "TestDirections.h"

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

/** A vector with a uniformly distributed direction and a length log-uniform in [ minLength, maxLength ] */
template < typename T >
static CTVector3D< T >
RandomVectorWithLength( MathReference::CSeededSequence& generator, Float64 minLength, Float64 maxLength )
{
    Float64 x, y, z, squared;
    do
    {
        x = generator.NextFloat64( -1.0, 1.0 );
        y = generator.NextFloat64( -1.0, 1.0 );
        z = generator.NextFloat64( -1.0, 1.0 );
        squared = x * x + y * y + z * z;
    }
    while ( squared < 1e-6 || squared > 1.0 );
    Float64 length = exp( generator.NextFloat64( log( minLength ), log( maxLength ) ) );
    Float64 scale = length / sqrt( squared );
    return CTVector3D< T >( CTVector3D< Float64 >( x * scale, y * scale, z * scale ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  10^5 seeded vectors: the direction satisfies the invariant and points the same way as the Float64 reference. Both
 *  are measured in Float64 from the component values (exact for the fixed point unit vectors), never with the
 *  arithmetic of the kind under test.
 */
template < typename T >
static UInt32
CountInvariantFailures( UInt64 seed, Float64 minLength, Float64 maxLength, Float64 invariantTolerance, Float64 directionTolerance )
{
    typedef CTDirectionVector3D< T > TDirection;
    MathReference::CSeededSequence generator( seed );
    UInt32 failures = 0;
    for ( UInt32 i=0; i<100000; ++i )
    {
        CTVector3D< T > v = RandomVectorWithLength< T >( generator, minLength, maxLength );
        if ( v.IsZero() )
            continue;
        TDirection d( v );
        if ( !TDirection::IsUnitLength( d.AsVector() ) )
            ++failures;
        CTVector3D< Float64 > unit( d.AsVector() );
        if ( fabs( sqrt( unit.x * unit.x + unit.y * unit.y + unit.z * unit.z ) - 1.0 ) > invariantTolerance )
            ++failures;
        CTVector3D< Float64 > reference = CTVector3D< Float64 >( v ).Normalized();
        if ( !CTVector3D< Float64 >( d.AsVector() ).Equals( reference, directionTolerance, 0.0 ) )
            ++failures;
        TDirection negated = -d;
        if ( !TDirection::IsUnitLength( negated.AsVector() ) || negated.AsVector() != -d.AsVector() )
            ++failures;
    }
    return failures;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDirectionsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Directions" );

    GUCEF_TESTFW_TESTCASE( "Directions: the unit length invariant for 10^5 seeded vectors with lengths in [1e-3, 1e3]" )
    try
    {
        const UInt64 seed = 20261114;
        UInt32 failures32 = CountInvariantFailures< Float32 >( seed, 1e-3, 1e3, 4e-5, 1e-6 );
        UInt32 failures64 = CountInvariantFailures< Float64 >( seed + 1, 1e-3, 1e3, 4e-10, 1e-14 );
        ASSERT_TRUE( 0 == failures32 && 0 == failures64 );
        if ( 0 != failures32 || 0 != failures64 )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Directions: invariant seed " << seed << " failures " << failures32 << " / " << failures64 );

        // very short and very long float vectors normalise through the scaling by the largest component
        ASSERT_TRUE( TFloat32DirectionVector3D( TFloat32Vector3D( 1e-30f, 0, 0 ) ) == TFloat32DirectionVector3D::UnitX() );
        ASSERT_TRUE( TFloat32DirectionVector3D( TFloat32Vector3D( 0, 3e38f, 3e38f ) ).Equals( TFloat32DirectionVector3D::FromNormalized( TFloat32Vector3D( 0, 0.70710678f, 0.70710678f ) ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Directions: fixed point invariant and direction within 2 raw units over the whole format" )
    try
    {
        const UInt64 seed = 1618;
        // Q16.16 components up to +-32767 (lengths to 3e4) and down to a few raw units; Q32.32 up to 1e9
        UInt32 failures16 = CountInvariantFailures< TFixed32 >( seed, 1e-3, 3e4, 2.0 / 65536.0, 2.0 / 65536.0 );
        UInt32 failures32 = CountInvariantFailures< TFixed64 >( seed + 1, 1e-6, 1e9, 2.0 / 4294967296.0, 2.0 / 4294967296.0 );
        ASSERT_TRUE( 0 == failures16 && 0 == failures32 );
        if ( 0 != failures16 || 0 != failures32 )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Directions: fixed point invariant seed " << seed << " failures " << failures16 << " / " << failures32 );
        ASSERT_TRUE( TFixed32DirectionVector3D( TFixed32Vector3D( TFixed32( 0 ), TFixed32( 300 ), TFixed32( 0 ) ) ) == TFixed32DirectionVector3D::Up() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Directions: degenerate input, TryCreate, FromNormalized and constants" )
    try
    {
        typedef TFloat32DirectionVector3D D;
        typedef TFloat32Vector3D V;
        ASSERT_TRUE( D() == D::Forward() && D::Forward().AsVector() == V( 0, 0, -1 ) );
        ASSERT_TRUE( D::Right().AsVector() == V::Right() && D::Left().AsVector() == V::Left() );
        ASSERT_TRUE( D::Up().AsVector() == V::Up() && D::Down().AsVector() == V::Down() );
        ASSERT_TRUE( D::Back().AsVector() == V::Back() && D::UnitZ() == D::Back() );

        D out = D::Up();
        ASSERT_FALSE( D::TryCreate( V::Zero(), out ) );
        const Float32 nan = (Float32) MathQuietNaN64();
        const Float32 infinity = (Float32) MathInfinity64();
        ASSERT_FALSE( D::TryCreate( V( nan, 1, 0 ), out ) );
        ASSERT_FALSE( D::TryCreate( V( infinity, 1, 0 ), out ) );
        ASSERT_TRUE( out == D::Up() );
        ASSERT_TRUE( D::TryCreate( V( 0, 0, 8 ), out ) && out == D::Back() );

        #ifndef GUCEF_MATH_DEBUG_MODE
        // the constructor asserts on degenerate input in debug builds; release builds give Forward()
        ASSERT_TRUE( D( V::Zero() ) == D::Forward() );
        ASSERT_TRUE( D( V( nan, 0, 0 ) ) == D::Forward() );
        ASSERT_TRUE( TFloat32DirectionVector2D( TFloat32Vector2D::Zero() ) == TFloat32DirectionVector2D::Right() );
        #endif

        D fromUnit = D::FromNormalized( V( 0.6f, 0.8f, 0 ) );
        ASSERT_TRUE( fromUnit.GetX() == 0.6f && fromUnit.GetY() == 0.8f && fromUnit.GetZ() == 0.0f );
        ASSERT_TRUE( D::IsUnitLength( V( 0.6f, 0.8f, 0 ) ) && !D::IsUnitLength( V( 0.6f, 0.8f, 0.01f ) ) );
        ASSERT_TRUE( 4e-5f == D::GetUnitLengthTolerance() );
        ASSERT_TRUE( TFixed32::FromRaw( 2 ) == TFixed32DirectionVector3D::GetUnitLengthTolerance() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Directions: a direction is a vector; products and angles" )
    try
    {
        typedef TFloat32DirectionVector3D D;
        typedef TFloat32Vector3D V;
        D x = D::UnitX();
        V asVector = x;
        ASSERT_TRUE( asVector == V::UnitX() );
        ASSERT_TRUE( x * 3.0f == V( 3, 0, 0 ) && 3.0f * x == V( 3, 0, 0 ) );
        ASSERT_TRUE( -x == D::Left() );
        ASSERT_TRUE( x + V( 0, 1, 0 ) == V( 1, 1, 0 ) );
        ASSERT_TRUE( 0.0f == x.Dot( D::UnitY() ) && 2.0f == x.Dot( V( 2, 5, 7 ) ) );
        ASSERT_TRUE( x.Cross( D::UnitY() ) == V::UnitZ() );
        ASSERT_TRUE( V( 1, 2, 3 ).Dot( x ) == 1.0f );
        const Float32 halfPi = CTNumericTraits< Float32 >::HalfPi();
        ASSERT_TRUE( EqualsUlp( D::Angle( D::UnitX(), D::UnitY() ).GetValue(), halfPi, 1 ) );
        ASSERT_TRUE( EqualsUlp( D::SignedAngle( D::UnitX(), D::UnitY(), D::UnitZ() ).GetValue(), halfPi, 1 ) );
        ASSERT_TRUE( EqualsUlp( D::SignedAngle( D::UnitX(), D::UnitY(), -D::UnitZ() ).GetValue(), -halfPi, 1 ) );
        ASSERT_TRUE( x.Equals( D::FromNormalized( V( 1, 5e-7f, 0 ) ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Directions: Slerp keeps the invariant and has constant angular speed" )
    try
    {
        typedef TFloat32DirectionVector3D D;
        typedef TFloat32Vector3D V;
        const Float32 h = 0.70710678f;
        ASSERT_TRUE( D::Slerp( D::UnitX(), D::UnitY(), 0.5f ).Equals( D::FromNormalized( V( h, h, 0 ) ) ) );
        ASSERT_TRUE( D::Slerp( D::UnitX(), D::UnitY(), 0.0f ).Equals( D::UnitX() ) );
        ASSERT_TRUE( D::Slerp( D::UnitX(), D::UnitY(), 1.0f ).Equals( D::UnitY() ) );
        ASSERT_TRUE( D::Slerp( D::UnitX(), D::UnitX(), 0.3f ) == D::UnitX() );
        // opposite directions turn through a perpendicular: halfway is perpendicular to both
        D halfway = D::Slerp( D::UnitX(), -D::UnitX(), 0.5f );
        ASSERT_TRUE( fabsf( halfway.Dot( D::UnitX() ) ) < 1e-6f && D::IsUnitLength( halfway.AsVector() ) );

        const UInt64 seed = 5551212;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<20000; ++i )
        {
            D a( V( (Float32) generator.NextFloat64( -1, 1 ), (Float32) generator.NextFloat64( -1, 1 ), (Float32) generator.NextFloat64( -1, 1 ) + 1e-3f ) );
            D b( V( (Float32) generator.NextFloat64( -1, 1 ), (Float32) generator.NextFloat64( -1, 1 ), (Float32) generator.NextFloat64( -1, 1 ) + 1e-3f ) );
            Float32 t = (Float32) generator.NextFloat64( 0, 1 );
            D s = D::Slerp( a, b, t );
            if ( !D::IsUnitLength( s.AsVector() ) )
                ++failures;
            // the angle from a is t * angle( a, b ), and the rest of the arc remains
            Float64 total = D::Angle( a, b ).GetValue();
            if ( total < 3.1 && ( fabs( D::Angle( a, s ).GetValue() - t * total ) > 1e-4 || fabs( D::Angle( s, b ).GetValue() - ( 1 - t ) * total ) > 1e-4 ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Directions: Slerp seed " << seed << " failures " << failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Directions: 2D angles, perpendicular and Slerp" )
    try
    {
        typedef TFloat32DirectionVector2D D;
        typedef TFloat32Vector2D V;
        const Float32 halfPi = CTNumericTraits< Float32 >::HalfPi();
        ASSERT_TRUE( D() == D::Right() && D::Up().AsVector() == V( 0, 1 ) && D::Left().AsVector() == V( -1, 0 ) && D::Down().AsVector() == V( 0, -1 ) );
        ASSERT_TRUE( D::FromAngle( TFloat32Radian( halfPi ) ).Equals( D::Up() ) );
        ASSERT_TRUE( D::FromAngle( CTDegree< Float32 >( 180.0f ) ).Equals( D::Left() ) );
        ASSERT_TRUE( EqualsUlp( D::Up().GetAngle().GetValue(), halfPi, 1 ) );
        ASSERT_TRUE( D::Right().GetPerpendicular() == D::Up() && D::Up().GetPerpendicular() == D::Left() );
        ASSERT_TRUE( 1.0f == D::Right().Cross( D::Up() ) && 0.0f == D::Right().Dot( D::Up() ) );
        ASSERT_TRUE( EqualsUlp( D::SignedAngle( D::Up(), D::Right() ).GetValue(), -halfPi, 1 ) );
        ASSERT_TRUE( EqualsUlp( D::Angle( D::Up(), D::Right() ).GetValue(), halfPi, 1 ) );
        ASSERT_TRUE( D::Right() * 2.0f == V( 2, 0 ) && -D::Right() == D::Left() );
        D d( V( 3, 4 ) );
        ASSERT_TRUE( d.Equals( D::FromNormalized( V( 0.6f, 0.8f ) ) ) );
        const Float32 h = 0.70710678f;
        ASSERT_TRUE( D::Slerp( D::Right(), D::Up(), 0.5f ).Equals( D::FromNormalized( V( h, h ) ) ) );
        ASSERT_TRUE( D::Slerp( D::Right(), D::Left(), 0.5f ).Equals( D::Up() ) );
        D out = D::Up();
        ASSERT_FALSE( D::TryCreate( V::Zero(), out ) );
        ASSERT_TRUE( out == D::Up() );
        TFixed32DirectionVector2D f( TFixed32Vector2D( TFixed32( 3 ), TFixed32( 4 ) ) );
        ASSERT_TRUE( TFixed32DirectionVector2D::IsUnitLength( f.AsVector() ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
