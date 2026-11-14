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
#include "TestVectors.h"

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

/** The alignment of T in C++98: the offset of T after a char */
template < typename T >
struct TAlignmentOf
{
    struct SProbe { char c; T value; };
    enum { Value = sizeof( SProbe ) - sizeof( T ) };
};

/** Compile-time type identity */
template < typename A, typename B > struct TIsSameType { enum { Value = 0 }; };
template < typename A > struct TIsSameType< A, A > { enum { Value = 1 }; };

/*-------------------------------------------------------------------------*/

/** The typedef family names the classes the design states (static checks, acceptance 1) */
GUCEF_MATH_STATIC_ASSERT( ( TIsSameType< TFloat64Vector3D, CTVector3D< Float64 > >::Value ), TFloat64Vector3D_is_CTVector3D_of_Float64 );
GUCEF_MATH_STATIC_ASSERT( ( TIsSameType< TFixed48_16Vector2D, CTVector2D< TFixed48_16 > >::Value ), TFixed48_16Vector2D_is_CTVector2D_of_Q48_16 );
GUCEF_MATH_STATIC_ASSERT( ( TIsSameType< TInt64FractionVector4D, CTVector4D< CORE::TInt64Fraction > >::Value ), TInt64FractionVector4D_is_CTVector4D_of_Int64_fraction );
GUCEF_MATH_STATIC_ASSERT( sizeof( TFloat32Vector3D ) == 12, TFloat32Vector3D_is_12_bytes );
GUCEF_MATH_STATIC_ASSERT( sizeof( TFloat32Vector4D ) == 16, TFloat32Vector4D_is_16_bytes );

/*-------------------------------------------------------------------------*/

/** a and b within 'ulps' units in the last place, component-wise */
template < typename TVector >
static bool
NearUlp( const TVector& a, const TVector& b, UInt32 ulps )
{
    return a.EqualsUlp( b, ulps );
}

/*-------------------------------------------------------------------------*/

/** A uniform Float32 vector with components in [ -range, range ) */
static TFloat32Vector3D
RandomVector3( MathReference::CSeededSequence& generator, Float64 range )
{
    return TFloat32Vector3D( (Float32) generator.NextFloat64( -range, range ),
                             (Float32) generator.NextFloat64( -range, range ),
                             (Float32) generator.NextFloat64( -range, range ) );
}

/*-------------------------------------------------------------------------*/

/** Calls realtime-safe value-type operations from a realtime context: a RealtimeSanitizer build reports any allocation */
static Float32
RealtimeVectorWork( const TFloat32Vector3D& a, const TFloat32Vector3D& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    TFloat32Vector3D sum = a + b - a * 0.5f + 2.0f * b;
    TFloat32Vector3D n = sum.Normalized();
    TFloat32Vector3D c = Cross( a, b );
    TFloat32Vector3D clamped = c.ClampLength( 1.0f );
    TFloat32Vector3D moved = TFloat32Vector3D::MoveTowards( a, b, 0.25f );
    TFloat32DirectionVector3D direction( TFloat32Vector3D( 1.0f, 2.0f, 3.0f ) );
    TFloat32DirectionVector3D halfway = TFloat32DirectionVector3D::Slerp( direction, TFloat32DirectionVector3D::Up(), 0.5f );
    TFloat32Point3D p( 1.0f, 2.0f, 3.0f );
    TFloat32Point3D q = p + moved;
    TFloat32Vector4D v4 = TFloat32Vector4D::FromPoint( q ) + b.XYZ0();
    Float32 angle = TFloat32Vector3D::SignedAngle( a, b, TFloat32DirectionVector3D::Up() ).GetValue();
    return n.Dot( clamped ) + angle + halfway.Dot( a ) + p.GetDistance( q ) + v4.Normalized().GetLength() +
           b.Reflect( direction ).GetLength() + a.ProjectOnPlane( direction ).x + a.Swizzle< 2, 1, 0 >().x;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVectorsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Vectors" );

    GUCEF_TESTFW_TESTCASE( "Vectors: sizes, alignment equal to the component, contiguous arrays" )
    try
    {
        ASSERT_TRUE( 12 == sizeof( CTVector3D< Float32 > ) );
        ASSERT_TRUE( 12 == sizeof( CTPoint3D< Float32 > ) );
        ASSERT_TRUE( 12 == sizeof( CTDirectionVector3D< Float32 > ) );
        ASSERT_TRUE( 16 == sizeof( CTVector4D< Float32 > ) );
        ASSERT_TRUE( 24 == sizeof( CTVector3D< Float64 > ) );
        ASSERT_TRUE( 8 == sizeof( TFloat32Vector2D ) && 8 == sizeof( TFloat32Point2D ) && 8 == sizeof( TFloat32DirectionVector2D ) );
        ASSERT_TRUE( 3 == sizeof( TInt8Vector3D ) && 12 == sizeof( TFixed32Vector3D ) && 24 == sizeof( TFixed64Vector3D ) );

        ASSERT_TRUE( (int) TAlignmentOf< Float32 >::Value == (int) TAlignmentOf< TFloat32Vector3D >::Value );
        ASSERT_TRUE( (int) TAlignmentOf< Float32 >::Value == (int) TAlignmentOf< TFloat32Vector4D >::Value );
        ASSERT_TRUE( (int) TAlignmentOf< Float32 >::Value == (int) TAlignmentOf< TFloat32Point3D >::Value );
        ASSERT_TRUE( (int) TAlignmentOf< Float32 >::Value == (int) TAlignmentOf< TFloat32DirectionVector3D >::Value );
        ASSERT_TRUE( (int) TAlignmentOf< Float64 >::Value == (int) TAlignmentOf< TFloat64Vector3D >::Value );
        ASSERT_TRUE( (int) TAlignmentOf< Int16 >::Value == (int) TAlignmentOf< TInt16Vector3D >::Value );

        static TFloat32Vector3D vectors[ 1000 ];
        static TFloat32Vector4D vectors4[ 1000 ];
        static TFloat32Point3D points[ 1000 ];
        ASSERT_TRUE( (size_t) ( (const char*) &vectors[ 999 ] - (const char*) &vectors[ 0 ] ) == 999 * sizeof( TFloat32Vector3D ) );
        ASSERT_TRUE( (size_t) ( (const char*) &vectors4[ 999 ] - (const char*) &vectors4[ 0 ] ) == 999 * 16 );
        ASSERT_TRUE( (size_t) ( (const char*) &points[ 999 ] - (const char*) &points[ 0 ] ) == 999 * 12 );
        ASSERT_TRUE( (const char*) &vectors[ 1 ].x - (const char*) &vectors[ 0 ].z == 4 );
        ASSERT_TRUE( ( (int) TIsSameType< TFloat64Vector3D, CTVector3D< Float64 > >::Value ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vectors: constants, operators and component access" )
    try
    {
        typedef TFloat32Vector3D V;
        ASSERT_TRUE( V() == V::Zero() && V::Zero().IsZero() );
        ASSERT_TRUE( V::One() == V( 1, 1, 1 ) );
        ASSERT_TRUE( V::Right() == V( 1, 0, 0 ) && V::Left() == V( -1, 0, 0 ) );
        ASSERT_TRUE( V::Up() == V( 0, 1, 0 ) && V::Down() == V( 0, -1, 0 ) );
        ASSERT_TRUE( V::Forward() == V( 0, 0, -1 ) && V::Back() == V( 0, 0, 1 ) );

        V a( 1, 2, 3 );
        V b( 4, -5, 6 );
        ASSERT_TRUE( a + b == V( 5, -3, 9 ) );
        ASSERT_TRUE( a - b == V( -3, 7, -3 ) );
        ASSERT_TRUE( -a == V( -1, -2, -3 ) && +a == a );
        ASSERT_TRUE( a * 2.0f == V( 2, 4, 6 ) && 2.0f * a == V( 2, 4, 6 ) );
        ASSERT_TRUE( b / 2.0f == V( 2, -2.5f, 3 ) );
        V c = a;
        c += b;  ASSERT_TRUE( c == V( 5, -3, 9 ) );
        c -= b;  ASSERT_TRUE( c == a );
        c *= 3;  ASSERT_TRUE( c == V( 3, 6, 9 ) );
        c /= 3;  ASSERT_TRUE( c == a );
        ASSERT_TRUE( a != b && !( a != a ) );
        ASSERT_TRUE( a.ComponentMultiply( b ) == V( 4, -10, 18 ) );
        ASSERT_TRUE( b.ComponentDivide( V( 2, 5, 3 ) ) == V( 2, -1, 2 ) );
        ASSERT_TRUE( 12.0f == a.Dot( b ) && 12.0f == Dot( a, b ) );
        ASSERT_TRUE( 12.0 == a.DotWide( b ) );
        ASSERT_TRUE( 14.0f == a.GetSquaredLength() );
        ASSERT_TRUE( 15.0f == b.GetManhattanLength() && 6.0f == b.GetChebyshevLength() );
        ASSERT_TRUE( b.Abs() == V( 4, 5, 6 ) );
        ASSERT_TRUE( a.Min( b ) == V( 1, -5, 3 ) && a.Max( b ) == V( 4, 2, 6 ) );
        ASSERT_TRUE( -5.0f == b.GetMinComponent() && 6.0f == b.GetMaxComponent() );
        ASSERT_TRUE( 2 == b.GetMaxComponentIndex() && 0 == V( 3, 3, 1 ).GetMaxComponentIndex() && 1 == V( 1, 3, 3 ).GetMaxComponentIndex() );

        for ( UInt32 i=0; i<3; ++i )
        {
            V d;
            d.SetComponent( i, (Float32) ( i + 7 ) );
            ASSERT_TRUE( (Float32) ( i + 7 ) == d.GetComponent( i ) );
            ASSERT_TRUE( (Float32) ( i + 7 ) == d.GetManhattanLength() );
        }
        ASSERT_TRUE( 1.0f == a.GetComponent( 0 ) && 2.0f == a.GetComponent( 1 ) && 3.0f == a.GetComponent( 2 ) );

        // 2D and 4D components
        TFloat32Vector2D e( 3, 4 );
        for ( UInt32 i=0; i<2; ++i )
        {
            TFloat32Vector2D f;
            f.SetComponent( i, 9.0f );
            ASSERT_TRUE( 9.0f == f.GetComponent( i ) && 9.0f == f.GetManhattanLength() );
        }
        ASSERT_TRUE( 3.0f == e.GetComponent( 0 ) && 4.0f == e.GetComponent( 1 ) );
        TFloat32Vector4D g( 1, 2, 3, 4 );
        for ( UInt32 i=0; i<4; ++i )
        {
            ASSERT_TRUE( (Float32) ( i + 1 ) == g.GetComponent( i ) );
            TFloat32Vector4D h;
            h.SetComponent( i, 5.0f );
            ASSERT_TRUE( 5.0f == h.GetComponent( i ) && 5.0f == h.GetManhattanLength() );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vectors: cross products, lengths, normalisation, angles, reflection and projection" )
    try
    {
        typedef TFloat32Vector3D V;
        typedef TFloat32DirectionVector3D D;
        ASSERT_TRUE( V::UnitX().Cross( V::UnitY() ) == V::UnitZ() );
        ASSERT_TRUE( V::UnitY().Cross( V::UnitZ() ) == V::UnitX() );
        ASSERT_TRUE( V::UnitZ().Cross( V::UnitX() ) == V::UnitY() );
        ASSERT_TRUE( Cross( V::UnitY(), V::UnitX() ) == -V::UnitZ() );

        ASSERT_TRUE( 5.0f == V( 3, 4, 0 ).GetLength() );
        ASSERT_TRUE( 5.0 == TFloat64Vector3D( 0, -3, 4 ).GetLength() );

        V normalized = V( 3, 4, 0 ).Normalized();
        ASSERT_TRUE( normalized.Equals( V( 0.6f, 0.8f, 0 ) ) );
        ASSERT_TRUE( V::Zero().Normalized() == V::Zero() );
        V unchanged( 7, 7, 7 );
        ASSERT_FALSE( V::Zero().TryNormalize( unchanged, 0.0f ) );
        ASSERT_TRUE( unchanged == V( 7, 7, 7 ) );
        ASSERT_FALSE( V( 0.5f, 0, 0 ).TryNormalize( unchanged, 1.0f ) );
        ASSERT_TRUE( V( 0, 0, 2 ).TryNormalize( unchanged, 1.0f ) && unchanged == V::UnitZ() );
        D direction;
        ASSERT_TRUE( V( 0, 5, 0 ).TryGetDirection( direction ) && direction == D::Up() );
        ASSERT_FALSE( V::Zero().TryGetDirection( direction ) );

        const Float32 halfPi = CTNumericTraits< Float32 >::HalfPi();
        ASSERT_TRUE( EqualsUlp( V::Angle( V::UnitX(), V::UnitY() ).GetValue(), halfPi, 1 ) );
        ASSERT_TRUE( 0.0f == V::Angle( V::UnitX(), V( 5, 0, 0 ) ).GetValue() );
        ASSERT_TRUE( EqualsUlp( V::Angle( V::UnitX(), -V::UnitX() ).GetValue(), CTNumericTraits< Float32 >::Pi(), 1 ) );
        ASSERT_TRUE( EqualsUlp( V::SignedAngle( V::UnitX(), V::UnitY(), D::UnitZ() ).GetValue(), halfPi, 1 ) );
        ASSERT_TRUE( EqualsUlp( V::SignedAngle( V::UnitX(), V::UnitY(), -D::UnitZ() ).GetValue(), -halfPi, 1 ) );

        ASSERT_TRUE( V( 1, -1, 0 ).Reflect( D::UnitY() ) == V( 1, 1, 0 ) );
        ASSERT_TRUE( V( 1, 1, 1 ).ProjectOnPlane( D::UnitY() ) == V( 1, 0, 1 ) );
        ASSERT_TRUE( V( 2, 3, 4 ).Project( V( 0, 10, 0 ) ) == V( 0, 3, 0 ) );
        ASSERT_TRUE( V( 2, 3, 4 ).Project( V::Zero() ) == V::Zero() );

        ASSERT_TRUE( V( 3, 4, 0 ).ClampLength( 2.5f ).Equals( V( 1.5f, 2.0f, 0 ) ) );
        ASSERT_TRUE( V( 3, 4, 0 ).ClampLength( 5.0f ) == V( 3, 4, 0 ) );
        ASSERT_TRUE( V( 3, 4, 0 ).ClampLength( 10.0f ) == V( 3, 4, 0 ) );

        V start( 0, 0, 0 );
        V target( 10, 0, 0 );
        ASSERT_TRUE( V::MoveTowards( start, target, 3.0f ) == V( 3, 0, 0 ) );
        ASSERT_TRUE( V::MoveTowards( start, target, 10.0f ) == target );
        ASSERT_TRUE( V::MoveTowards( start, target, 1000.0f ) == target );
        ASSERT_TRUE( V::MoveTowards( target, target, 1.0f ) == target );
        V position = start;
        for ( int i=0; i<20; ++i )
        {
            position = V::MoveTowards( position, target, 0.75f );
            ASSERT_FALSE( position.x > 10.0f );
        }
        ASSERT_TRUE( position == target );

        ASSERT_TRUE( V::Lerp( V( 0, 0, 0 ), V( 2, 4, 8 ), 0.5f ) == V( 1, 2, 4 ) );
        ASSERT_TRUE( V::Lerp( V( 0, 0, 0 ), V( 2, 4, 8 ), 2.0f ) == V( 4, 8, 16 ) );

        // perpendiculars for every axis order and the zero vector
        const V perpendicularInputs[ 5 ] = { V( 1, 2, 3 ), V( 3, 1, 2 ), V( 3, 2, 1 ), V( 0, 0, -4 ), V( 5, 5, 5 ) };
        for ( UInt32 i=0; i<5; ++i )
        {
            V p = perpendicularInputs[ i ].GetAnyPerpendicular();
            ASSERT_TRUE( 0.0f == p.Dot( perpendicularInputs[ i ] ) && !p.IsZero() );
        }
        ASSERT_TRUE( V::Zero().GetAnyPerpendicular().IsZero() );

        // seeded: cross product perpendicular to both inputs and |a x b|^2 + ( a . b )^2 = |a|^2 |b|^2 against Float64
        const UInt64 seed = 31337;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<10000; ++i )
        {
            V a = RandomVector3( generator, 100.0 );
            V b = RandomVector3( generator, 100.0 );
            TFloat64Vector3D a64( a );
            TFloat64Vector3D b64( b );
            TFloat64Vector3D c64 = a64.Cross( b64 );
            V c = a.Cross( b );
            Float64 scale = a64.GetLength() * b64.GetLength();
            if ( fabs( c64.Dot( a64 ) ) > 1e-9 * scale * a64.GetLength() || fabs( c64.Dot( b64 ) ) > 1e-9 * scale * b64.GetLength() )
                ++failures;
            if ( !TFloat64Vector3D( c ).Equals( c64, 1e-4 * scale, 0.0 ) )
                ++failures;
            if ( fabs( c64.GetSquaredLength() + a64.Dot( b64 ) * a64.Dot( b64 ) - a64.GetSquaredLength() * b64.GetSquaredLength() ) > 1e-9 * scale * scale )
                ++failures;
            V n = a.Normalized();
            if ( fabs( TFloat64Vector3D( n ).GetLength() - 1.0 ) > 4e-7 )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Vectors: cross product sweep seed " << seed << " failures " << failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vectors: swizzles" )
    try
    {
        TFloat32Vector3D a( 1, 2, 3 );
        ASSERT_TRUE( a.XY() == TFloat32Vector2D( 1, 2 ) );
        ASSERT_TRUE( a.XZ() == TFloat32Vector2D( 1, 3 ) );
        ASSERT_TRUE( a.YZ() == TFloat32Vector2D( 2, 3 ) );
        ASSERT_TRUE( a.XYZ0() == TFloat32Vector4D( 1, 2, 3, 0 ) );
        ASSERT_TRUE( a.XYZ1() == TFloat32Vector4D( 1, 2, 3, 1 ) );
        ASSERT_TRUE( ( a.Swizzle< 2, 1, 0 >() == TFloat32Vector3D( 3, 2, 1 ) ) );
        ASSERT_TRUE( ( a.Swizzle< 0, 0, 2 >() == TFloat32Vector3D( 1, 1, 3 ) ) );
        TFloat32Vector2D b( 1, 2 );
        ASSERT_TRUE( b.YX() == TFloat32Vector2D( 2, 1 ) );
        ASSERT_TRUE( ( b.Swizzle< 1, 1 >() == TFloat32Vector2D( 2, 2 ) ) );
        ASSERT_TRUE( b.ToVector3D( 7 ) == TFloat32Vector3D( 1, 2, 7 ) );
        TFloat32Vector4D c( 1, 2, 3, 4 );
        ASSERT_TRUE( c.XY() == b && c.XYZ() == a );
        ASSERT_TRUE( ( c.Swizzle< 3, 2, 1, 0 >() == TFloat32Vector4D( 4, 3, 2, 1 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vectors: 2D specifics" )
    try
    {
        typedef TFloat32Vector2D V;
        ASSERT_TRUE( 1.0f == V::UnitX().Cross( V::UnitY() ) && -1.0f == Cross( V::UnitY(), V::UnitX() ) );
        ASSERT_TRUE( V( 2, 3 ).GetPerpendicular() == V( -3, 2 ) );
        ASSERT_TRUE( 5.0f == V( 3, -4 ).GetLength() && 7.0f == V( 3, -4 ).GetManhattanLength() && 4.0f == V( 3, -4 ).GetChebyshevLength() );
        ASSERT_TRUE( V( 3, 4 ).Normalized().Equals( V( 0.6f, 0.8f ) ) && V::Zero().Normalized() == V::Zero() );
        const Float32 halfPi = CTNumericTraits< Float32 >::HalfPi();
        ASSERT_TRUE( EqualsUlp( V::SignedAngle( V::UnitX(), V::UnitY() ).GetValue(), halfPi, 1 ) );
        ASSERT_TRUE( EqualsUlp( V::SignedAngle( V::UnitY(), V::UnitX() ).GetValue(), -halfPi, 1 ) );
        ASSERT_TRUE( EqualsUlp( V::Angle( V::UnitY(), V::UnitX() ).GetValue(), halfPi, 1 ) );
        ASSERT_TRUE( V( 1, -1 ).Reflect( TFloat32DirectionVector2D::UnitY() ) == V( 1, 1 ) );
        ASSERT_TRUE( V( 2, 3 ).Project( V( 5, 0 ) ) == V( 2, 0 ) );
        ASSERT_TRUE( V( 3, 4 ).ClampLength( 2.5f ).Equals( V( 1.5f, 2.0f ) ) );
        ASSERT_TRUE( V::MoveTowards( V( 0, 0 ), V( 0, 10 ), 4.0f ) == V( 0, 4 ) );
        ASSERT_TRUE( 1 == V( 1, 2 ).GetMaxComponentIndex() && 0 == V( 2, 2 ).GetMaxComponentIndex() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vectors: 4D, homogeneous factories and the canonical dot product order" )
    try
    {
        typedef TFloat32Vector4D V;
        V a( 1, 2, 3, 4 );
        V b( -1, 0.5f, 2, -3 );
        ASSERT_TRUE( a + b == V( 0, 2.5f, 5, 1 ) && a - b == V( 2, 1.5f, 1, 7 ) && -a == V( -1, -2, -3, -4 ) );
        ASSERT_TRUE( a * 2.0f == V( 2, 4, 6, 8 ) && 2.0f * a == a * 2.0f && a / 2.0f == V( 0.5f, 1, 1.5f, 2 ) );
        V c = a;
        c += b; c -= b; c *= 4.0f; c /= 4.0f;
        ASSERT_TRUE( c == a );
        ASSERT_TRUE( -6.0f == a.Dot( b ) && -6.0f == Dot( a, b ) && 30.0f == a.GetSquaredLength() );
        ASSERT_TRUE( a.Min( b ) == V( -1, 0.5f, 2, -3 ) && a.Max( b ) == V( 1, 2, 3, 4 ) );
        ASSERT_TRUE( 10.0f == a.GetManhattanLength() && 4.0f == a.GetChebyshevLength() );
        ASSERT_TRUE( V( 2, 0, 0, 0 ).Normalized() == V::UnitX() && V::Zero().Normalized() == V::Zero() );
        ASSERT_TRUE( V::FromPoint( TFloat32Point3D( 1, 2, 3 ) ) == V( 1, 2, 3, 1 ) );
        ASSERT_TRUE( V::FromVector( TFloat32Vector3D( 1, 2, 3 ) ) == V( 1, 2, 3, 0 ) );
        TFloat32Point3D point( 9, 9, 9 );
        ASSERT_TRUE( V( 2, 4, 6, 2 ).TryGetPoint( point ) && point == TFloat32Point3D( 1, 2, 3 ) );
        ASSERT_FALSE( V( 2, 4, 6, 0 ).TryGetPoint( point ) );
        ASSERT_TRUE( point == TFloat32Point3D( 1, 2, 3 ) );

        // canonical order ( px + pz ) + ( py + pw ): with px = 1, py = 2^-24, pz = -1, pw = 2^-24 a left-to-right sum
        // gives 2^-24 ( 1 + 2^-24 rounds to 1 ), the canonical order gives the exact 2^-23
        const Float32 tiny = ldexpf( 1.0f, -24 );
        V p( 1.0f, tiny, -1.0f, tiny );
        ASSERT_TRUE( ldexpf( 1.0f, -23 ) == p.Dot( V::One() ) );
        ASSERT_TRUE( ldexpf( 1.0f, -23 ) == ( CTVector4Ops< Float32 >::Dot( p, V::One() ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vectors: tolerant equality" )
    try
    {
        typedef TFloat32Vector3D V;
        ASSERT_TRUE( V( 1, 1, 1 ).Equals( V( 1, 1, 1 ) ) );
        ASSERT_TRUE( V( 1, 1, 1 ).Equals( V( 1 + 5e-7f, 1, 1 ) ) );
        ASSERT_FALSE( V( 1, 1, 1 ).Equals( V( 1 + 1e-4f, 1, 1 ) ) );
        ASSERT_TRUE( V( 1, 1, 1 ).Equals( V( 1 + 1e-4f, 1, 1 ), 1e-3f, 0.0f ) );
        ASSERT_TRUE( V( 1000, 1, 1 ).Equals( V( 1000.5f, 1, 1 ), 0.0f, 1e-3f ) );
        const Float32 next = nextafterf( 1.0f, 2.0f );
        ASSERT_TRUE( V( 1, 1, 1 ).EqualsUlp( V( next, 1, 1 ), 1 ) );
        ASSERT_FALSE( V( 1, 1, 1 ).EqualsUlp( V( nextafterf( next, 2.0f ), 1, 1 ), 1 ) );
        ASSERT_TRUE( V( -0.0f, 0, 0 ).EqualsUlp( V( 0, 0, 0 ), 0 ) );
        const Float32 nan = (Float32) MathQuietNaN64();
        ASSERT_FALSE( V( nan, 0, 0 ).Equals( V( nan, 0, 0 ) ) );
        ASSERT_FALSE( V( nan, 0, 0 ).EqualsUlp( V( nan, 0, 0 ), 100 ) );
        ASSERT_FALSE( V( nan, 0, 0 ) == V( nan, 0, 0 ) );
        ASSERT_FALSE( V( nan, 0, 0 ).IsFinite() );
        ASSERT_TRUE( V( 1, 2, 3 ).IsFinite() );
        ASSERT_TRUE( NearUlp( TFloat64Vector2D( 1, 2 ), TFloat64Vector2D( 1, 2 ), 0 ) );
        ASSERT_TRUE( TFloat32Vector4D( 1, 1, 1, 1 ).Equals( TFloat32Vector4D( 1, 1, 1, 1 + 5e-7f ) ) );
        ASSERT_FALSE( TFloat32Vector4D( 1, 1, 1, 1 ).Equals( TFloat32Vector4D( 1, 1, 1, 1 + 1e-4f ) ) );
        ASSERT_TRUE( TFixed32Vector3D( TFixed32( 1 ), TFixed32( 1 ), TFixed32( 1 ) ).EqualsUlp( TFixed32Vector3D( TFixed32::FromRaw( 65538 ), TFixed32( 1 ), TFixed32( 1 ) ), 2 ) );
        ASSERT_TRUE( TFixed32Vector3D( TFixed32( 1 ), TFixed32( 1 ), TFixed32( 1 ) ).Equals( TFixed32Vector3D( TFixed32::FromRaw( 65538 ), TFixed32( 1 ), TFixed32( 1 ) ) ) );
        ASSERT_FALSE( TFixed32Vector3D( TFixed32( 1 ), TFixed32( 1 ), TFixed32( 1 ) ).Equals( TFixed32Vector3D( TFixed32::FromRaw( 65539 ), TFixed32( 1 ), TFixed32( 1 ) ) ) );
        ASSERT_TRUE( TInt32Vector3D( 1, 2, 3 ).Equals( TInt32Vector3D( 1, 2, 3 ) ) && !TInt32Vector3D( 1, 2, 3 ).Equals( TInt32Vector3D( 1, 2, 4 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vectors: integer kinds (Dot, DotWide, Cross, Manhattan, DivideRounded)" )
    try
    {
        typedef TInt32Vector3D V;
        const Int32 big = 1 << 20;
        V a( big, big, big );
        ASSERT_TRUE( CORE::CInt128( (Int64) 3 << 40 ) == a.DotWide( a ) );
        ASSERT_TRUE( CORE::CInt128( (Int64) 2 << 40 ) == TInt32Vector2D( big, big ).DotWide( TInt32Vector2D( big, big ) ) );
        ASSERT_TRUE( CORE::CInt128( (Int64) 4 << 40 ) == TInt32Vector4D( big, big, big, big ).DotWide( TInt32Vector4D( big, big, big, big ) ) );
        ASSERT_TRUE( 32 == V( 1, 2, 3 ).Dot( V( 4, 5, 6 ) ) );
        ASSERT_TRUE( V( 1, 2, 3 ).Cross( V( 4, 5, 6 ) ) == V( -3, 6, -3 ) );
        ASSERT_TRUE( 12 == V( -3, 4, -5 ).GetManhattanLength() && 5 == V( -3, 4, -5 ).GetChebyshevLength() );
        ASSERT_TRUE( 50 == V( -3, 4, -5 ).GetSquaredLength() );
        ASSERT_TRUE( -V( 1, -2, 3 ) == V( -1, 2, -3 ) && V( 1, 2, 3 ) * 3 == V( 3, 6, 9 ) );
        ASSERT_TRUE( V( 1, 2, 3 ).GetAnyPerpendicular().Dot( V( 1, 2, 3 ) ) == 0 );
        ASSERT_TRUE( TInt8Vector3D( 100, 0, 0 ).DotWide( TInt8Vector3D( 100, 0, 0 ) ) == 10000 );
        ASSERT_TRUE( TUInt16Vector2D( 3, 4 ).GetSquaredLength() == 25 );

        // 7 / 2 = 3.5 and -7 / 2 = -3.5 with every rounding mode
        struct SCase { TRoundingMode mode; Int32 positive; Int32 negative; TMathStatus status; };
        const SCase cases[] =
        {
            { CORE::ROUNDINGMODE_HALF_EVEN,      4,  -4, MATHSTATUS_INEXACT },
            { CORE::ROUNDINGMODE_HALF_UP,        4,  -4, MATHSTATUS_INEXACT },
            { CORE::ROUNDINGMODE_HALF_DOWN,      3,  -3, MATHSTATUS_INEXACT },
            { CORE::ROUNDINGMODE_TOWARD_ZERO,    3,  -3, MATHSTATUS_INEXACT },
            { CORE::ROUNDINGMODE_AWAY_FROM_ZERO, 4,  -4, MATHSTATUS_INEXACT },
            { CORE::ROUNDINGMODE_FLOOR,          3,  -4, MATHSTATUS_INEXACT },
            { CORE::ROUNDINGMODE_CEILING,        4,  -3, MATHSTATUS_INEXACT }
        };
        for ( UInt32 i=0; i<sizeof( cases ) / sizeof( cases[ 0 ] ); ++i )
        {
            V out;
            TMathStatus status = V( 7, -7, 8 ).DivideRounded( 2, cases[ i ].mode, out );
            ASSERT_TRUE( out == V( cases[ i ].positive, cases[ i ].negative, 4 ) );
            ASSERT_TRUE( cases[ i ].status == status );
            TInt32Vector2D out2;
            ASSERT_TRUE( cases[ i ].status == TInt32Vector2D( 7, -7 ).DivideRounded( 2, cases[ i ].mode, out2 ) );
            ASSERT_TRUE( out2 == TInt32Vector2D( cases[ i ].positive, cases[ i ].negative ) );
        }
        V unchanged( 1, 1, 1 );
        ASSERT_TRUE( 0 != ( MATHSTATUS_ROUNDING_REQUIRED & V( 7, -7, 8 ).DivideRounded( 2, CORE::ROUNDINGMODE_UNNECESSARY, unchanged ) ) );
        ASSERT_TRUE( unchanged == V( 1, 1, 1 ) );
        ASSERT_TRUE( MATHSTATUS_OK == V( 8, -6, 0 ).DivideRounded( 2, CORE::ROUNDINGMODE_UNNECESSARY, unchanged ) && unchanged == V( 4, -3, 0 ) );
        ASSERT_TRUE( 0 != ( MATHSTATUS_DIVISION_BY_ZERO & V( 7, -7, 8 ).DivideRounded( 0, CORE::ROUNDINGMODE_HALF_EVEN, unchanged ) ) );
        ASSERT_TRUE( unchanged == V( 4, -3, 0 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vectors: integer extremes, exact DotWide and wrapping arithmetic without undefined behaviour" )
    try
    {
        const Int32 minimum = (Int32) 0x80000000u;
        const Int32 maximum = 0x7FFFFFFF;
        typedef TInt32Vector3D V;
        // 3 * 2^62 does not fit Int64: the exact results in 128 bits (constants from the decimal values)
        ASSERT_TRUE( CORE::CInt128( 0, 0xC000000000000000ULL ) == V( minimum, minimum, minimum ).DotWide( V( minimum, minimum, minimum ) ) );
        ASSERT_TRUE( CORE::CInt128( 0, 0xBFFFFFFD00000003ULL ) == V( maximum, maximum, maximum ).DotWide( V( maximum, maximum, maximum ) ) );
        ASSERT_TRUE( CORE::CInt128::Negate( CORE::CInt128( 0, 0xBFFFFFFE80000000ULL ) ) == V( minimum, minimum, minimum ).DotWide( V( maximum, maximum, maximum ) ) );
        ASSERT_TRUE( CORE::CInt128( 0, 0x8000000000000000ULL ) == TInt32Vector2D( minimum, minimum ).DotWide( TInt32Vector2D( minimum, minimum ) ) );
        ASSERT_TRUE( CORE::CInt128( 1, 0 ) == TInt32Vector4D( minimum, minimum, minimum, minimum ).DotWide( TInt32Vector4D( minimum, minimum, minimum, minimum ) ) );
        ASSERT_TRUE( CORE::CUInt128( 2, 0xFFFFFFFA00000003ULL ) == TUInt32Vector3D( 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu ).DotWide( TUInt32Vector3D( 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu ) ) );
        ASSERT_TRUE( CORE::CUInt128( (UInt64) 1 << 62, 0 ) == TUInt64Vector2D( (UInt64) 1 << 63, 0 ).DotWide( TUInt64Vector2D( (UInt64) 1 << 63, 0 ) ) );
        ASSERT_TRUE( CORE::CInt128( (UInt64) 1 << 60, 0 ) == TInt64Vector2D( (Int64) 1 << 62, 0 ).DotWide( TInt64Vector2D( (Int64) 1 << 62, 0 ) ) );

        // 16 bit: DotWide in Int64 (3 * 2^30 does not fit Int32), Dot wraps in the kind
        const Int16 minimum16 = (Int16) -32768;
        TInt16Vector3D m16( minimum16, minimum16, minimum16 );
        ASSERT_TRUE( (Int64) 3221225472LL == m16.DotWide( m16 ) );
        ASSERT_TRUE( 0 == m16.Dot( m16 ) );
        ASSERT_TRUE( (Int64) 3 * 32767 * 32767 == TInt16Vector3D( 32767, 32767, 32767 ).DotWide( TInt16Vector3D( 32767, 32767, 32767 ) ) );
        ASSERT_TRUE( (UInt64) 3 * 65535 * 65535 == TUInt16Vector3D( 65535, 65535, 65535 ).DotWide( TUInt16Vector3D( 65535, 65535, 65535 ) ) );
        ASSERT_TRUE( (UInt16) ( 65535u * 65535u ) == TUInt16Vector2D( 65535, 0 ).Dot( TUInt16Vector2D( 65535, 0 ) ) );
        ASSERT_TRUE( (Int32) 3 * 128 * 128 == TInt8Vector3D( -128, -128, -128 ).DotWide( TInt8Vector3D( -128, -128, -128 ) ) );

        // the operators wrap as the built-in types do, through the unsigned image (UBSan finds no overflow)
        ASSERT_TRUE( V( maximum, 0, minimum ) + V( 1, 0, -1 ) == V( minimum, 0, maximum ) );
        ASSERT_TRUE( V( minimum, 0, 0 ) - V( 1, 0, 0 ) == V( maximum, 0, 0 ) );
        ASSERT_TRUE( -V( minimum, 1, 0 ) == V( minimum, -1, 0 ) );
        ASSERT_TRUE( V( maximum, minimum, 3 ) * 2 == V( -2, 0, 6 ) );
        ASSERT_TRUE( 2 * V( maximum, minimum, 3 ) == V( -2, 0, 6 ) );
        ASSERT_TRUE( V( maximum, maximum, 0 ).Dot( V( 2, 0, 0 ) ) == -2 );
        ASSERT_TRUE( V( minimum, 0, 0 ).Cross( V( 0, 2, 0 ) ) == V( 0, 0, 0 ) );
        ASSERT_TRUE( TInt8Vector2D( 127, -128 ) + TInt8Vector2D( 1, -1 ) == TInt8Vector2D( -128, 127 ) );
        ASSERT_TRUE( TInt64Vector2D( (Int64) 0x7FFFFFFFFFFFFFFFLL, 0 ) + TInt64Vector2D( 1, 0 ) == TInt64Vector2D( (Int64) 0x8000000000000000ULL, 0 ) );
        ASSERT_TRUE( TUInt32Vector2D( 0, 1 ) - TUInt32Vector2D( 1, 1 ) == TUInt32Vector2D( 0xFFFFFFFFu, 0 ) );
        ASSERT_TRUE( TInt32Point3D( maximum, 0, 0 ) - TInt32Point3D( -1, 0, 0 ) == V( minimum, 0, 0 ) );
        ASSERT_TRUE( TInt32Vector4D( maximum, 0, 0, 0 ) + TInt32Vector4D( 1, 0, 0, 0 ) == TInt32Vector4D( minimum, 0, 0, 0 ) );
        ASSERT_TRUE( 0 == TInt32Vector4D( minimum, minimum, minimum, minimum ).Dot( TInt32Vector4D( minimum, minimum, minimum, minimum ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vectors: fixed point over the whole format, lengths correctly rounded against the Float64 reference" )
    try
    {
        typedef TFixed32Vector3D V;
        typedef TFixed32 F;
        ASSERT_TRUE( F( 5 ) == V( F( 3 ), F( 4 ), F( 0 ) ).GetLength() );
        ASSERT_TRUE( V::UnitX().Cross( V::UnitY() ) == V::UnitZ() );

        // far beyond the old limit of sqrt( 32768 ) = 181: the squared length is never formed in the format
        ASSERT_TRUE( F( 200 ) == V( F( 200 ), F( 0 ), F( 0 ) ).GetLength() );
        ASSERT_TRUE( F( 25000 ) == V( F( 15000 ), F( -20000 ), F( 0 ) ).GetLength() );
        ASSERT_TRUE( F::Highest() == V( F( 32767 ), F( 32767 ), F( 32767 ) ).GetLength() );
        ASSERT_TRUE( V( F( 200 ), F( 0 ), F( 0 ) ).Normalized() == V::UnitX() );
        ASSERT_TRUE( V( F( 0 ), F( -32767 ), F( 0 ) ).Normalized() == -V::UnitY() );
        ASSERT_TRUE( V( F::Lowest(), F( 0 ), F( 0 ) ).Normalized() == -V::UnitX() );
        ASSERT_TRUE( V( F( 200 ), F( 0 ), F( 0 ) ).ClampLength( F( 2 ) ) == V( F( 2 ), F( 0 ), F( 0 ) ) );
        ASSERT_TRUE( V::MoveTowards( V(), V( F( 300 ), F( 400 ), F( 0 ) ), F( 5 ) ).EqualsUlp( V( F( 3 ), F( 4 ), F( 0 ) ), 8 ) );
        const F pi = CTNumericTraits< F >::Pi();
        ASSERT_TRUE( CTNumericTraits< F >::EqualsUlp( V::Angle( V( F( 20 ), F( 0 ), F( 0 ) ), V( F( 20 ), F( 20 ), F( 0 ) ) ).GetValue(), pi / F( 4 ), 4 ) );
        ASSERT_TRUE( CTNumericTraits< F >::EqualsUlp( V::Angle( V( F( 30000 ), F( 0 ), F( 0 ) ), V( F( 0 ), F( 0 ), F( -30000 ) ) ).GetValue(), pi / F( 2 ), 4 ) );
        ASSERT_TRUE( CTNumericTraits< F >::EqualsUlp( V::SignedAngle( V( F( 20000 ), F( 0 ), F( 0 ) ), V( F( 0 ), F( 20000 ), F( 0 ) ), TFixed32DirectionVector3D::UnitZ() ).GetValue(), pi / F( 2 ), 4 ) );
        TFixed32Point2D origin;
        TFixed32Point2D far( F( 150 ), F( 150 ) );
        ASSERT_TRUE( 13902285 == origin.GetDistance( far ).GetRaw() );     // 150 sqrt 2 = 212.132034..., 13902285.004 raw

        // small vectors keep full precision: the vector is divided by its largest component before normalising
        ASSERT_TRUE( V( F::FromRaw( 655 ), F( 0 ), F( 0 ) ).Normalized() == V::UnitX() );
        TFixed32Vector2D small( F::FromRaw( 655 ), F::FromRaw( 655 ) );
        ASSERT_TRUE( small.Normalized().EqualsUlp( TFixed32Vector2D( F::FromRaw( 46341 ), F::FromRaw( 46341 ) ), 1 ) );

        // Dot and the squared length: the exact sum of the raw products rounded once, saturating
        ASSERT_TRUE( F::FromRaw( 3 ) == V( F::FromRaw( 1 ), F::FromRaw( 1 ), F::FromRaw( 1 ) ).Dot( V( F::FromRaw( 65536 + 32768 ), F::FromRaw( 65536 ), F::FromRaw( 32768 ) ) ) );
        ASSERT_TRUE( F::Highest() == V( F( 200 ), F( 0 ), F( 0 ) ).GetSquaredLength() );
        ASSERT_TRUE( F::Lowest() == V( F( 200 ), F( 0 ), F( 0 ) ).Dot( V( F( -200 ), F( 0 ), F( 0 ) ) ) );

        // seeded: every Q16.16 component value, lengths correctly rounded (the Float64 reference is exact to 1e-6 raw)
        const UInt64 seed = 4242;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<20000; ++i )
        {
            V v;
            Float64 squares = 0.0;
            for ( UInt32 c=0; c<3; ++c )
            {
                Int32 raw = (Int32) generator.NextUInt32();
                if ( 0 != ( i & 1 ) )
                    raw >>= (Int32) ( generator.NextUInt32() % 31 );     // also small magnitudes
                v.SetComponent( c, F::FromRaw( raw ) );
                squares += (Float64) raw * (Float64) raw;
            }
            Float64 exactRaw = sqrt( squares );
            Int64 got = (Int64) v.GetLength().GetRaw();
            if ( exactRaw > 2147483647.0 ? got != 2147483647 : fabs( (Float64) got - exactRaw ) > 0.5 + 1e-6 )
                ++failures;
            // normalisation: unit length within 2 raw units and direction within 2 raw units, measured in Float64
            V unit = v.Normalized();
            Float64 ux = unit.x.GetRaw() / 65536.0;
            Float64 uy = unit.y.GetRaw() / 65536.0;
            Float64 uz = unit.z.GetRaw() / 65536.0;
            if ( squares > 0.0 && fabs( sqrt( ux * ux + uy * uy + uz * uz ) - 1.0 ) > 2.0 / 65536.0 )
                ++failures;
            for ( UInt32 c=0; c<3 && squares > 0.0; ++c )
            {
                Float64 reference = v.GetComponent( c ).GetRaw() / sqrt( squares );
                if ( fabs( unit.GetComponent( c ).GetRaw() / 65536.0 - reference ) > 2.0 / 65536.0 )
                    ++failures;
            }
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Vectors: Q16.16 length and normalisation seed " << seed << " failures " << failures );

        // Q32.32 over its whole range ( +-2^31 ), and Q48.16
        TFixed64Vector3D v64( TFixed64( 3 ), TFixed64( 4 ), TFixed64( 12 ) );
        ASSERT_TRUE( TFixed64( 13 ) == v64.GetLength() );
        ASSERT_TRUE( TFixed64( 2000000000 ) == TFixed64Vector3D( TFixed64( 1200000000 ), TFixed64( -1600000000 ), TFixed64( 0 ) ).GetLength() );
        ASSERT_TRUE( TFixed64Vector3D( TFixed64( 2000000000 ), TFixed64( 0 ), TFixed64( 0 ) ).Normalized() == TFixed64Vector3D::UnitX() );
        UInt32 failures64 = 0;
        MathReference::CSeededSequence generator64( seed + 1 );
        for ( UInt32 i=0; i<20000; ++i )
        {
            TFixed64Vector3D v;
            Float64 reference[ 3 ];
            for ( UInt32 c=0; c<3; ++c )
            {
                Int64 raw = (Int64) generator64.NextUInt64() / 4;     // |raw| < 2^61: the length stays representable
                if ( 0 != ( i & 1 ) )
                    raw >>= (Int32) ( generator64.NextUInt32() % 62 );
                v.SetComponent( c, TFixed64::FromRaw( raw ) );
                reference[ c ] = (Float64) raw;
            }
            Float64 length = sqrt( reference[ 0 ] * reference[ 0 ] + reference[ 1 ] * reference[ 1 ] + reference[ 2 ] * reference[ 2 ] );
            TFixed64Vector3D unit = v.Normalized();
            Float64 ux = (Float64) unit.x.GetRaw() / 4294967296.0;
            Float64 uy = (Float64) unit.y.GetRaw() / 4294967296.0;
            Float64 uz = (Float64) unit.z.GetRaw() / 4294967296.0;
            if ( length > 0.0 && fabs( sqrt( ux * ux + uy * uy + uz * uz ) - 1.0 ) > 2.0 / 4294967296.0 + 1e-15 )
                ++failures64;
            for ( UInt32 c=0; c<3 && length > 0.0; ++c )
            {
                if ( fabs( (Float64) unit.GetComponent( c ).GetRaw() / 4294967296.0 - reference[ c ] / length ) > 2.0 / 4294967296.0 + 1e-15 )
                    ++failures64;
            }
            // the length relative to the Float64 reference (which carries 53 bits)
            if ( fabs( (Float64) v.GetLength().GetRaw() - length ) > 1e-15 * length + 0.5 )
                ++failures64;
        }
        ASSERT_TRUE( 0 == failures64 );
        if ( 0 != failures64 )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Vectors: Q32.32 length and normalisation seed " << ( seed + 1 ) << " failures " << failures64 );
        TFixed48_16Vector2D v48( TFixed48_16( 30000 ), TFixed48_16( 40000 ) );
        ASSERT_TRUE( TFixed48_16( 50000 ) == v48.GetLength() );
        ASSERT_TRUE( TFixed32Vector2D( F( 3 ), F( 4 ) ).Normalized().EqualsUlp( TFixed32Vector2D( F::FromRaw( 39322 ), F::FromRaw( 52429 ) ), 1 ) );
        ASSERT_TRUE( TFixed32Vector4D( F( 20000 ), F( 0 ), F( 0 ), F( 0 ) ).Normalized() == TFixed32Vector4D::UnitX() );
        ASSERT_TRUE( F( 20000 ) == TFixed32Vector4D( F( 0 ), F( 0 ), F( 0 ), F( -20000 ) ).GetLength() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vectors: floats near the ends of the range, no overflow or underflow of the squared length" )
    try
    {
        typedef TFloat32Vector3D V;
        ASSERT_TRUE( V( 1e20f, 0, 0 ).Normalized() == V::UnitX() );
        ASSERT_TRUE( 1e20f == V( 1e20f, 0, 0 ).GetLength() );
        ASSERT_TRUE( EqualsUlp( (Float64) V( 1.5e38f, 2e38f, 0 ).GetLength(), sqrt( (Float64) 1.5e38f * (Float64) 1.5e38f + (Float64) 2e38f * (Float64) 2e38f ), 1u << 30 ) );
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( V( 2e38f, 2e38f, 0 ).GetLength() ) );
        ASSERT_TRUE( !CTNumericTraits< Float32 >::IsFinite( V( FLT_MAX, FLT_MAX, 0 ).GetLength() ) );     // the length itself exceeds FLT_MAX
        ASSERT_TRUE( V( FLT_MAX, FLT_MAX, 0 ).Normalized().Equals( V( 0.70710678f, 0.70710678f, 0 ) ) );
        ASSERT_TRUE( V( 1e-23f, 1e-23f, 0 ).Normalized().Equals( V( 0.70710678f, 0.70710678f, 0 ) ) );
        ASSERT_TRUE( V( 1e-45f, 0, 0 ).Normalized() == V::UnitX() );
        ASSERT_TRUE( EqualsUlp( V( FLT_MIN, 0, FLT_MIN ).GetLength(), FLT_MIN * 1.41421356f, 1 ) );
        ASSERT_TRUE( EqualsUlp( V( 3e-39f, 4e-39f, 0 ).GetLength(), 5e-39f, 2 ) );
        V unchanged( 1, 2, 3 );
        ASSERT_TRUE( V( 0, 1e-30f, 0 ).TryNormalize( unchanged, 0.0f ) && unchanged == V::UnitY() );
        ASSERT_TRUE( V( 1e30f, 0, 0 ).ClampLength( 1.0f ) == V::UnitX() );
        ASSERT_TRUE( V::MoveTowards( V( -1.5e38f, 0, 0 ), V( 1.5e38f, 0, 0 ), 1e38f ).Equals( V( -0.5e38f, 0, 0 ) ) );
        ASSERT_TRUE( EqualsUlp( V::Angle( V( 1e30f, 0, 0 ), V( 0, 1e-30f, 0 ) ).GetValue(), CTNumericTraits< Float32 >::HalfPi(), 1 ) );
        ASSERT_TRUE( EqualsUlp( V::Angle( V( 3e38f, 3e38f, 0 ), V( 3e38f, 0, 0 ) ).GetValue(), CTNumericTraits< Float32 >::Pi() / 4.0f, 2 ) );
        ASSERT_TRUE( 0.0f == V::Angle( V::Zero(), V::UnitX() ).GetValue() );
        ASSERT_TRUE( V( (Float32) MathQuietNaN64(), 0, 0 ).Normalized().IsZero() );
        ASSERT_FALSE( CTNumericTraits< Float32 >::IsFinite( V( (Float32) MathQuietNaN64(), 0, 0 ).GetLength() ) );

        ASSERT_TRUE( TFloat64Vector3D( 1e300, 1e300, 0 ).Normalized().Equals( TFloat64Vector3D( 0.70710678118654752, 0.70710678118654752, 0 ) ) );
        ASSERT_TRUE( EqualsUlp( TFloat64Vector3D( 3e-310, 4e-310, 0 ).GetLength(), 5e-310, 2 ) );
        ASSERT_TRUE( TFloat32Vector2D( 1e20f, 0 ).Normalized() == TFloat32Vector2D::UnitX() );
        ASSERT_TRUE( TFloat32Vector2D( 3e38f, 3e38f ).GetLength() == V( 3e38f, 3e38f, 0 ).GetLength() );
        ASSERT_TRUE( TFloat32Vector4D( 0, 0, 0, 1e20f ).Normalized() == TFloat32Vector4D::UnitW() );
        ASSERT_TRUE( TFloat32Vector4D( 0, 1e-23f, 0, 1e-23f ).Normalized().Equals( TFloat32Vector4D( 0, 0.70710678f, 0, 0.70710678f ) ) );
        ASSERT_TRUE( 1e20f == TFloat32Vector4D( 0, 0, 1e20f, 0 ).GetLength() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vectors: exact fraction kinds, Dot and Cross against hand-computed rationals" )
    try
    {
        typedef CORE::TInt64Fraction F;
        typedef TInt64FractionVector3D V;
        V a( F( 1, 2 ), F( 1, 3 ), F( -2, 5 ) );
        V b( F( 3, 4 ), F( -1, 6 ), F( 5, 7 ) );
        // 1/2 * 3/4 + 1/3 * -1/6 + -2/5 * 5/7 = 3/8 - 1/18 - 2/7 = ( 189 - 28 - 144 ) / 504 = 17/504
        ASSERT_TRUE( F( 17, 504 ) == a.Dot( b ) );
        // ( 1/3 * 5/7 - -2/5 * -1/6, -2/5 * 3/4 - 1/2 * 5/7, 1/2 * -1/6 - 1/3 * 3/4 ) = ( 5/21 - 1/15, -3/10 - 5/14, -1/12 - 1/4 ) = ( 6/35, -23/35, -1/3 )
        ASSERT_TRUE( a.Cross( b ) == V( F( 6, 35 ), F( -23, 35 ), F( -1, 3 ) ) );
        ASSERT_TRUE( 0 == a.Cross( b ).Dot( a ).GetNumerator() && 0 == a.Cross( b ).Dot( b ).GetNumerator() );
        ASSERT_TRUE( -a == V( F( -1, 2 ), F( -1, 3 ), F( 2, 5 ) ) );
        ASSERT_TRUE( a / F( 2, 1 ) == V( F( 1, 4 ), F( 1, 6 ), F( -1, 5 ) ) );
        ASSERT_TRUE( a.Project( V( F( 0 ), F( 3 ), F( 0 ) ) ) == V( F( 0 ), F( 1, 3 ), F( 0 ) ) );
        ASSERT_TRUE( F( 3, 2 ) == TInt64FractionVector2D( F( 1, 2 ), F( 1 ) ).Dot( TInt64FractionVector2D( F( 1 ), F( 1 ) ) ) );

        typedef TMathBigFraction B;
        TBigFractionVector3D c( B( 1, 2 ), B( 1, 3 ), B( -2, 5 ) );
        TBigFractionVector3D d( B( 3, 4 ), B( -1, 6 ), B( 5, 7 ) );
        ASSERT_TRUE( B( 17, 504 ) == c.Dot( d ) );
        ASSERT_TRUE( c.Cross( d ) == TBigFractionVector3D( B( 6, 35 ), B( -23, 35 ), B( -1, 3 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Vectors: realtime-safe operations run in a realtime context" )
    try
    {
        Float32 result = RealtimeVectorWork( TFloat32Vector3D( 1, 2, 3 ), TFloat32Vector3D( -2, 0.5f, 4 ) );
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( result ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
