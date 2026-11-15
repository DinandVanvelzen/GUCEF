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

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_MATH_CCOORDINATESYSTEM_H
#include "gucefMATH_CCoordinateSystem.h"
#define GUCEF_MATH_CCOORDINATESYSTEM_H
#endif /* GUCEF_MATH_CCOORDINATESYSTEM_H ? */

#include "TestMathReference.h"
#include "TestQuaternion.h"

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

/** A uniformly distributed unit quaternion (rejection sampling in the 4D ball) */
static TFloat64Quaternion
RandomUnitQuaternion( MathReference::CSeededSequence& generator )
{
    for ( ;; )
    {
        Float64 c[ 4 ];
        Float64 sum = 0.0;
        for ( UInt32 i=0; i<4; ++i )
        {
            c[ i ] = generator.NextFloat64( -1.0, 1.0 );
            sum += c[ i ] * c[ i ];
        }
        if ( sum > 1e-4 && sum <= 1.0 )
        {
            Float64 n = sqrt( sum );
            return TFloat64Quaternion::FromXYZW( c[ 0 ] / n, c[ 1 ] / n, c[ 2 ] / n, c[ 3 ] / n );
        }
    }
}

/*-------------------------------------------------------------------------*/

static TFloat64Vector3D
RandomVector( MathReference::CSeededSequence& generator, Float64 range )
{
    return TFloat64Vector3D( generator.NextFloat64( -range, range ), generator.NextFloat64( -range, range ), generator.NextFloat64( -range, range ) );
}

/*-------------------------------------------------------------------------*/

static TFloat64DirectionVector3D
RandomDirection( MathReference::CSeededSequence& generator )
{
    for ( ;; )
    {
        TFloat64Vector3D v = RandomVector( generator, 1.0 );
        TFloat64DirectionVector3D d;
        if ( v.GetLength() > 1e-3 && v.GetLength() <= 1.0 && TFloat64DirectionVector3D::TryCreate( v, d ) )
            return d;
    }
}

/*-------------------------------------------------------------------------*/

template < typename TVector >
static bool
NearVector( const TVector& a, const TVector& b, Float64 tolerance )
{
    return fabs( (Float64) a.x - (Float64) b.x ) <= tolerance && fabs( (Float64) a.y - (Float64) b.y ) <= tolerance && fabs( (Float64) a.z - (Float64) b.z ) <= tolerance;
}

/*-------------------------------------------------------------------------*/

/** a and b are the same quaternion up to sign within 'tolerance' per component */
template < typename TQuaternion >
static bool
NearQuaternionUpToSign( const TQuaternion& a, const TQuaternion& b, Float64 tolerance )
{
    Float64 same = 0.0;
    Float64 opposite = 0.0;
    const Float64 ca[ 4 ] = { (Float64) a.x, (Float64) a.y, (Float64) a.z, (Float64) a.w };
    const Float64 cb[ 4 ] = { (Float64) b.x, (Float64) b.y, (Float64) b.z, (Float64) b.w };
    for ( UInt32 i=0; i<4; ++i )
    {
        if ( fabs( ca[ i ] - cb[ i ] ) > same ) same = fabs( ca[ i ] - cb[ i ] );
        if ( fabs( ca[ i ] + cb[ i ] ) > opposite ) opposite = fabs( ca[ i ] + cb[ i ] );
    }
    return same <= tolerance || opposite <= tolerance;
}

/*-------------------------------------------------------------------------*/

/** The largest absolute element difference of two 3x3 matrices */
template < typename TMatrix >
static Float64
Matrix3Distance( const TMatrix& a, const TMatrix& b )
{
    Float64 largest = 0.0;
    for ( UInt32 r=0; r<3; ++r )
        for ( UInt32 c=0; c<3; ++c )
            if ( fabs( (Float64) a( r, c ) - (Float64) b( r, c ) ) > largest )
                largest = fabs( (Float64) a( r, c ) - (Float64) b( r, c ) );
    return largest;
}

/*-------------------------------------------------------------------------*/

/** Calls realtime-safe quaternion operations from a realtime context: a RealtimeSanitizer build reports any allocation */
static Float32
RealtimeQuaternionWork( const TFloat32Quaternion& a, const TFloat32Quaternion& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    TFloat32Quaternion c = a * b;
    TFloat32Quaternion s = TFloat32Quaternion::Slerp( a, b, 0.3f );
    TFloat32Quaternion n = TFloat32Quaternion::Nlerp( a, b, 0.6f );
    TFloat32Quaternion q = TFloat32Quaternion::Squad( a, s, n, b, 0.5f );
    TFloat32Quaternion t = TFloat32Quaternion::SquadTangent( a, s, b );
    TFloat32Quaternion r = TFloat32Quaternion::RotateTowards( a, b, TFloat32Radian( 0.1f ) );
    TFloat32Quaternion f = TFloat32Quaternion::FromTo( TFloat32DirectionVector3D::UnitX(), TFloat32DirectionVector3D::Up() );
    TFloat32Quaternion l = TFloat32Quaternion::LookRotation( TFloat32DirectionVector3D::Right(), TFloat32Vector3D::Up(), CCoordinateSystem::GucefDefault() );
    TFloat32DirectionVector3D axis;
    TFloat32Radian angle;
    c.ToAxisAngle( axis, angle );
    TFloat32EulerAngles e = q.ToEuler( EULERORDER_ZYX, EULERFRAME_INTRINSIC );
    return c.Rotate( TFloat32Vector3D( 1, 2, 3 ) ).x + t.Log().Exp().w + r.GetAngleTo( f ).GetValue() + l.Inverse().GetLength() +
           angle.GetValue() + e.first.GetValue() + q.ToRotationMatrix4D()( 0, 0 ) + TFloat32Quaternion::FromRotationMatrix( s.ToRotationMatrix3D() ).w +
           l.GetForward( CCoordinateSystem::GucefDefault() ).Dot( TFloat32Vector3D( 1, 0, 0 ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformQuaternionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Quaternion" );

    GUCEF_TESTFW_TESTCASE( "Quaternion: layout, storage order x y z w, named factories, conversions" )
    try
    {
        ASSERT_TRUE( 16 == sizeof( TFloat32Quaternion ) && 32 == sizeof( TFloat64Quaternion ) && 16 == sizeof( TFixed32Quaternion ) );
        TFloat32Quaternion identity;
        ASSERT_TRUE( 0.0f == identity.x && 0.0f == identity.y && 0.0f == identity.z && 1.0f == identity.w && identity == TFloat32Quaternion::Identity() );
        TFloat32Quaternion q = TFloat32Quaternion::FromXYZW( 1, 2, 3, 4 );
        ASSERT_TRUE( 1.0f == ( &q.x )[ 0 ] && 2.0f == ( &q.x )[ 1 ] && 3.0f == ( &q.x )[ 2 ] && 4.0f == ( &q.x )[ 3 ] );
        ASSERT_TRUE( TFloat32Quaternion::FromWXYZ( 4, 1, 2, 3 ) == q );
        ASSERT_TRUE( TFloat64Quaternion( q ).w == 4.0 && TFloat32Quaternion( TFloat64Quaternion( q ) ) == q );
        ASSERT_TRUE( ( -q ).x == -1.0f && q.Conjugate() == TFloat32Quaternion::FromXYZW( -1, -2, -3, 4 ) && q.GetVectorPart() == TFloat32Vector3D( 1, 2, 3 ) );
        ASSERT_TRUE( 30.0f == q.Dot( q ) && 30.0f == Dot( q, q ) && fabs( q.GetLength() - sqrt( 30.0 ) ) < 1e-6 );
        ASSERT_TRUE( fabs( q.Normalized().GetLength() - 1.0f ) < 1e-6f && TFloat32Quaternion::FromXYZW( 0, 0, 0, 0 ).Normalized() == identity );
        ASSERT_TRUE( ( q * q.Inverse() ).Equals( identity ) && TFloat32Quaternion::FromXYZW( 0, 0, 0, 0 ).Inverse() == TFloat32Quaternion::FromXYZW( 0, 0, 0, 0 ) );
        TFixed32Quaternion fixedQ( TFloat64Quaternion::FromXYZW( 0.5, -0.5, 0.25, 0.75 ) );
        ASSERT_TRUE( TFixed32( 0.25 ) == fixedQ.z && TFixed32( 0.75 ) == fixedQ.w );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Quaternion: composition qB * qA applies qA first, as the matrices do (10^4 seeded pairs)" )
    try
    {
        // acceptance 3: tolerance 1e-5 in Float32, 1e-12 in Float64
        const UInt64 seed = 0xC0B0;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<10000; ++i )
        {
            TFloat64Quaternion qA = RandomUnitQuaternion( generator );
            TFloat64Quaternion qB = RandomUnitQuaternion( generator );
            TFloat64Vector3D v = RandomVector( generator, 2.0 );
            if ( !NearVector( ( qB * qA ).Rotate( v ), qB.Rotate( qA.Rotate( v ) ), 1e-12 ) )
                ++failures;
            if ( !( Matrix3Distance( ( qB * qA ).ToRotationMatrix3D(), qB.ToRotationMatrix3D() * qA.ToRotationMatrix3D() ) <= 1e-12 ) )
                ++failures;
            TFloat32Quaternion fA( qA );
            TFloat32Quaternion fB( qB );
            TFloat32Vector3D fv( v );
            if ( !NearVector( ( fB * fA ).Rotate( fv ), fB.Rotate( fA.Rotate( fv ) ), 1e-5 * 4.0 ) )
                ++failures;
            if ( !( Matrix3Distance( ( fB * fA ).ToRotationMatrix3D(), fB.ToRotationMatrix3D() * fA.ToRotationMatrix3D() ) <= 1e-5 ) )
                ++failures;
            // the rotation of a vector equals the matrix of the quaternion applied to it
            if ( !NearVector( qA.Rotate( v ), qA.ToRotationMatrix3D() * v, 1e-12 ) || !NearVector( qA * v, qA.ToRotationMatrix4D() * v, 1e-12 ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Quaternion: " << failures << " composition checks failed, seed " << seed );
        TFloat64Quaternion a = TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::UnitY(), TFloat64Radian( g_pi / 2 ) );
        TFloat64Quaternion b = TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::UnitX(), TFloat64Radian( g_pi / 2 ) );
        TFloat64Quaternion c = a;
        c *= b;
        ASSERT_TRUE( c == a * b && NearVector( c.Rotate( TFloat64Vector3D::UnitY() ), TFloat64Vector3D::UnitX(), 1e-15 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Quaternion: rotation conventions, axis and angle, angle between rotations" )
    try
    {
        TFloat32Quaternion y90 = TFloat32Quaternion::FromAxisAngle( TFloat32DirectionVector3D::UnitY(), TFloat32Radian( TFloat32Degree( 90.0f ) ) );
        ASSERT_TRUE( NearVector( y90.Rotate( TFloat32Vector3D( 1, 0, 0 ) ), TFloat32Vector3D( 0, 0, -1 ), 1e-6 ) );
        ASSERT_TRUE( NearVector( y90.Rotate( TFloat32DirectionVector3D::UnitX() ).AsVector(), TFloat32Vector3D( 0, 0, -1 ), 1e-6 ) );
        ASSERT_TRUE( TFloat32Quaternion::FromAxisAngle( TFloat32AxisAngle( TFloat32DirectionVector3D::UnitY(), TFloat32Radian( TFloat32Degree( 90.0f ) ) ) ) == y90 );

        // the identity has axis +X and angle 0
        TFloat64DirectionVector3D axis = TFloat64DirectionVector3D::UnitZ();
        TFloat64Radian angle( 1.0 );
        TFloat64Quaternion().ToAxisAngle( axis, angle );
        ASSERT_TRUE( axis == TFloat64DirectionVector3D::UnitX() && 0.0 == angle.GetValue() );

        const UInt64 seed = 0xA815;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<1000; ++i )
        {
            TFloat64DirectionVector3D d = RandomDirection( generator );
            TFloat64Radian a( generator.NextFloat64( 0.001, g_pi - 0.001 ) );
            TFloat64Quaternion q = TFloat64Quaternion::FromAxisAngle( d, a );
            q.ToAxisAngle( axis, angle );
            if ( !( fabs( angle.GetValue() - a.GetValue() ) < 1e-12 ) || !NearVector( axis.AsVector(), d.AsVector(), 1e-12 ) )
                ++failures;
            // -q is the same rotation: the same angle, the axis of the representative with w >= 0
            ( -q ).ToAxisAngle( axis, angle );
            if ( !( fabs( angle.GetValue() - a.GetValue() ) < 1e-12 ) )
                ++failures;
            if ( !q.EqualsRotation( -q, TFloat64Radian( 1e-12 ) ) || !( q.GetAngleTo( -q ).GetValue() < 1e-12 ) )
                ++failures;
            TFloat64Quaternion r = RandomUnitQuaternion( generator );
            if ( !( fabs( r.GetAngleTo( q * r ).GetValue() - a.GetValue() ) < 1e-10 ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Quaternion: " << failures << " axis-angle checks failed, seed " << seed );
        ASSERT_FALSE( TFloat64Quaternion().EqualsRotation( TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::UnitX(), TFloat64Radian( 0.01 ) ), TFloat64Radian( 0.005 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Quaternion: Slerp end points, midpoint, shortest path, long chains; Nlerp; RotateTowards" )
    try
    {
        // acceptance 6
        const TFloat64Quaternion identity;
        const TFloat64Quaternion y90 = TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::UnitY(), TFloat64Radian( g_pi / 2 ) );
        const TFloat64Quaternion y45 = TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::UnitY(), TFloat64Radian( g_pi / 4 ) );
        ASSERT_TRUE( TFloat64Quaternion::Slerp( identity, y90, 0.0 ) == identity && TFloat64Quaternion::Slerp( identity, y90, 1.0 ) == y90 );
        ASSERT_TRUE( TFloat64Quaternion::Slerp( identity, y90, 0.5 ).Equals( y45 ) );
        ASSERT_TRUE( TFloat64Quaternion::Slerp( identity, y90, 0.25 ).EqualsRotation( TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::UnitY(), TFloat64Radian( g_pi / 8 ) ), TFloat64Radian( 1e-14 ) ) );
        // shortest path: -y90 is the same rotation, the interpolation takes the short arc either way
        ASSERT_TRUE( TFloat64Quaternion::Slerp( identity, -y90, 0.5 ).EqualsRotation( y45, TFloat64Radian( 1e-14 ) ) );
        ASSERT_TRUE( TFloat64Quaternion::Slerp( identity, -y90, 1.0 ) == y90 );
        // the long path goes the other way around
        ASSERT_TRUE( TFloat64Quaternion::SlerpLongPath( identity, -y90, 0.5 ).EqualsRotation( TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::UnitY(), TFloat64Radian( -3 * g_pi / 4 ) ), TFloat64Radian( 1e-14 ) ) );
        ASSERT_TRUE( TFloat64Quaternion::Nlerp( identity, -y90, 0.5 ).EqualsRotation( y45, TFloat64Radian( 1e-14 ) ) );
        // nearly parallel inputs
        const TFloat64Quaternion tiny = TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::UnitZ(), TFloat64Radian( 1e-12 ) );
        ASSERT_TRUE( fabs( TFloat64Quaternion::Slerp( identity, tiny, 0.5 ).GetLength() - 1.0 ) < 1e-15 );

        // 10^5 chained slerps keep the unit length within 1e-6 (Float32)
        const UInt64 seed = 0x51E4;
        MathReference::CSeededSequence generator( seed );
        TFloat32Quaternion chain;
        Float64 worst = 0.0;
        for ( UInt32 i=0; i<100000; ++i )
        {
            TFloat32Quaternion target( RandomUnitQuaternion( generator ) );
            chain = TFloat32Quaternion::Slerp( chain, target, (Float32) generator.NextFloat64( 0.0, 1.0 ) );
            Float64 error = fabs( (Float64) chain.GetLength() - 1.0 );
            if ( error > worst ) worst = error;
        }
        ASSERT_TRUE( worst <= 1e-6 );
        if ( !( worst <= 1e-6 ) )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Quaternion: chained slerp length error " << worst << ", seed " << seed );

        // RotateTowards never overshoots
        const TFloat64Quaternion stepped = TFloat64Quaternion::RotateTowards( identity, y90, TFloat64Radian( 0.1 ) );
        ASSERT_TRUE( fabs( identity.GetAngleTo( stepped ).GetValue() - 0.1 ) < 1e-14 );
        ASSERT_TRUE( TFloat64Quaternion::RotateTowards( identity, y90, TFloat64Radian( 2.0 ) ) == y90 );
        ASSERT_TRUE( TFloat64Quaternion::RotateTowards( identity, y90, TFloat64Radian( 0.0 ) ) == identity );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Quaternion: FromTo shortest arc, antiparallel input, LookRotation and the axes of a coordinate system" )
    try
    {
        // acceptance 6
        TFloat64Quaternion half = TFloat64Quaternion::FromTo( TFloat64DirectionVector3D::UnitX(), TFloat64DirectionVector3D( TFloat64Vector3D( -1, 0, 0 ) ) );
        TFloat64DirectionVector3D axis;
        TFloat64Radian angle;
        half.ToAxisAngle( axis, angle );
        ASSERT_TRUE( fabs( angle.GetValue() - g_pi ) < 1e-15 && fabs( axis.Dot( TFloat64Vector3D::UnitX() ) ) < 1e-15 );
        ASSERT_TRUE( NearVector( half.Rotate( TFloat64Vector3D::UnitX() ), TFloat64Vector3D( -1, 0, 0 ), 1e-15 ) );
        ASSERT_TRUE( TFloat64Quaternion::FromTo( TFloat64DirectionVector3D::UnitZ(), TFloat64DirectionVector3D::UnitZ() ) == TFloat64Quaternion() );

        const UInt64 seed = 0xF207;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<10000; ++i )
        {
            TFloat64DirectionVector3D from = RandomDirection( generator );
            TFloat64DirectionVector3D to = RandomDirection( generator );
            if ( 0 == i % 100 )
                to = TFloat64DirectionVector3D( -from.AsVector() + RandomVector( generator, 1e-7 ) );    // nearly opposite
            TFloat64Quaternion q = TFloat64Quaternion::FromTo( from, to );
            if ( !NearVector( q.Rotate( from.AsVector() ), to.AsVector(), 1e-12 ) || !( fabs( q.GetLength() - 1.0 ) < 1e-15 ) )
                ++failures;
            // shortest arc: the rotation angle is the angle between the directions
            if ( !( fabs( TFloat64Quaternion().GetAngleTo( q ).GetValue() - TFloat64DirectionVector3D::Angle( from, to ).GetValue() ) < 1e-9 ) )
                ++failures;
            TFloat32Quaternion q32 = TFloat32Quaternion::FromTo( TFloat32DirectionVector3D( TFloat32Vector3D( from.AsVector() ) ), TFloat32DirectionVector3D( TFloat32Vector3D( to.AsVector() ) ) );
            if ( !NearVector( q32.Rotate( TFloat32Vector3D( from.AsVector() ) ), TFloat32Vector3D( to.AsVector() ), 1e-5 ) )
                ++failures;
            // FromRotationMatrix( ToRotationMatrix3D( q ) ) equals +-q
            TFloat64Quaternion r = RandomUnitQuaternion( generator );
            if ( !NearQuaternionUpToSign( TFloat64Quaternion::FromRotationMatrix( r.ToRotationMatrix3D() ), r, 1e-14 ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Quaternion: " << failures << " FromTo or matrix round trips failed, seed " << seed );

        // LookRotation( Forward, Up, GucefDefault ) is the identity; the result maps the axes of the system
        const CCoordinateSystem gucef = CCoordinateSystem::GucefDefault();
        ASSERT_TRUE( TFloat64Quaternion::LookRotation( TFloat64DirectionVector3D::Forward(), TFloat64Vector3D::Up(), gucef ).EqualsRotation( TFloat64Quaternion(), TFloat64Radian( 1e-15 ) ) );
        TFloat64DirectionVector3D target( TFloat64Vector3D( 1, 0.5, -2 ) );
        TFloat64Quaternion look = TFloat64Quaternion::LookRotation( target, TFloat64Vector3D::Up(), gucef );
        ASSERT_TRUE( look.GetForward( gucef ).Equals( target ) && fabs( look.GetRight( gucef ).AsVector().y ) < 1e-15 && look.GetUp( gucef ).AsVector().y > 0.0 );
        ASSERT_TRUE( fabs( look.GetUp( gucef ).Dot( target.AsVector() ) ) < 1e-15 );
        // in a left-handed system (Unity: forward +Z) the identity also looks along the system's forward axis
        const CCoordinateSystem unity = CCoordinateSystem::Unity();
        ASSERT_TRUE( TFloat64Quaternion::LookRotation( TFloat64DirectionVector3D::UnitZ(), TFloat64Vector3D::Up(), unity ).EqualsRotation( TFloat64Quaternion(), TFloat64Radian( 1e-15 ) ) );
        TFloat64Quaternion lookUnity = TFloat64Quaternion::LookRotation( target, TFloat64Vector3D::Up(), unity );
        ASSERT_TRUE( lookUnity.GetForward( unity ).Equals( target ) && fabs( lookUnity.ToRotationMatrix3D().GetDeterminant() - 1.0 ) < 1e-14 );
        // an up hint parallel to the view direction still gives a rotation
        TFloat64Quaternion straightUp = TFloat64Quaternion::LookRotation( TFloat64DirectionVector3D::Up(), TFloat64Vector3D::Up(), gucef );
        ASSERT_TRUE( straightUp.GetForward( gucef ).Equals( TFloat64DirectionVector3D::Up() ) && fabs( straightUp.GetLength() - 1.0 ) < 1e-15 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Quaternion: Squad passes through its keys; logarithm and exponential" )
    try
    {
        const UInt64 seed = 0x5C0AD;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<1000; ++i )
        {
            TFloat64Quaternion keys[ 4 ] = { RandomUnitQuaternion( generator ), RandomUnitQuaternion( generator ), RandomUnitQuaternion( generator ), RandomUnitQuaternion( generator ) };
            TFloat64Quaternion a1 = TFloat64Quaternion::SquadTangent( keys[ 0 ], keys[ 1 ], keys[ 2 ] );
            TFloat64Quaternion a2 = TFloat64Quaternion::SquadTangent( keys[ 1 ], keys[ 2 ], keys[ 3 ] );
            if ( !( TFloat64Quaternion::Squad( keys[ 1 ], a1, a2, keys[ 2 ], 0.0 ) == keys[ 1 ] ) || !( TFloat64Quaternion::Squad( keys[ 1 ], a1, a2, keys[ 2 ], 1.0 ) == keys[ 2 ] ) )
                ++failures;
            TFloat64Quaternion middle = TFloat64Quaternion::Squad( keys[ 1 ], a1, a2, keys[ 2 ], 0.5 );
            if ( !( fabs( middle.GetLength() - 1.0 ) < 1e-14 ) )
                ++failures;
            // Exp( Log( q ) ) == q for unit and non-unit quaternions
            TFloat64Quaternion q = keys[ 0 ];
            if ( !q.Log().Exp().Equals( q ) )
                ++failures;
            TFloat64Quaternion scaled = TFloat64Quaternion::FromXYZW( q.x * 3.0, q.y * 3.0, q.z * 3.0, q.w * 3.0 );
            if ( !scaled.Log().Exp().Equals( scaled, 1e-13, 1e-13 ) || !( fabs( scaled.Log().w - log( 3.0 ) ) < 1e-14 ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Quaternion: " << failures << " squad or log / exp checks failed, seed " << seed );
        // the logarithm of a unit quaternion is ( axis angle / 2, 0 )
        TFloat64Quaternion log = TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::UnitZ(), TFloat64Radian( 1.0 ) ).Log();
        ASSERT_TRUE( fabs( log.z - 0.5 ) < 1e-15 && fabs( log.w ) < 1e-15 && 0.0 == log.x );
        ASSERT_TRUE( TFloat64Quaternion().Log() == TFloat64Quaternion::FromXYZW( 0, 0, 0, 0 ) && TFloat64Quaternion::FromXYZW( 0, 0, 0, 0 ).Exp() == TFloat64Quaternion() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Quaternion: Squad with sign-flipped keys takes the short arc; the antipodal long path" )
    try
    {
        // regression: keys about Y at 0, 0.4, 0.8, 1.2 rad with the 0.8 key negated
        TFloat64Quaternion keys[ 4 ];
        for ( UInt32 i=0; i<4; ++i )
            keys[ i ] = TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::UnitY(), TFloat64Radian( 0.4 * i ) );
        keys[ 2 ] = -keys[ 2 ];
        TFloat64Quaternion a1 = TFloat64Quaternion::SquadTangent( keys[ 0 ], keys[ 1 ], keys[ 2 ] );
        TFloat64Quaternion a2 = TFloat64Quaternion::SquadTangent( keys[ 1 ], keys[ 2 ], keys[ 3 ] );
        Float64 travelled = 0.0;
        TFloat64Quaternion previous = keys[ 1 ];
        for ( UInt32 step=1; step<=100; ++step )
        {
            TFloat64Quaternion current = TFloat64Quaternion::Squad( keys[ 1 ], a1, a2, keys[ 2 ], 0.01 * step );
            travelled += previous.GetAngleTo( current ).GetValue();
            previous = current;
        }
        // the segment turns 0.4 rad about Y (allow 5 % for the spline's shape)
        ASSERT_TRUE( travelled < 0.42 && travelled > 0.39 );
        ASSERT_TRUE( TFloat64Quaternion::Squad( keys[ 1 ], a1, a2, keys[ 2 ], 0.5 ).EqualsRotation( TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::UnitY(), TFloat64Radian( 0.6 ) ), TFloat64Radian( 1e-3 ) ) );
        ASSERT_TRUE( TFloat64Quaternion::Squad( keys[ 1 ], a1, a2, keys[ 2 ], 1.0 ) == keys[ 2 ] && TFloat64Quaternion::Squad( keys[ 1 ], a1, a2, keys[ 2 ], 0.0 ) == keys[ 1 ] );
        // control points of either sign give the same curve
        TFloat64Quaternion flipped = TFloat64Quaternion::Squad( keys[ 1 ], -a1, -a2, -keys[ 2 ], 0.3 );
        ASSERT_TRUE( flipped.EqualsRotation( TFloat64Quaternion::Squad( keys[ 1 ], a1, a2, keys[ 2 ], 0.3 ), TFloat64Radian( 1e-12 ) ) );

        // SlerpLongPath( a, -a, t ): a full turn through a deterministic orthogonal quaternion, unit length throughout
        TFloat64Quaternion a = TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D( TFloat64Vector3D( 1, 2, 3 ) ), TFloat64Radian( 0.8 ) );
        ASSERT_TRUE( TFloat64Quaternion::SlerpLongPath( a, -a, 0.0 ) == a && TFloat64Quaternion::SlerpLongPath( a, -a, 1.0 ) == -a );
        TFloat64Quaternion halfway = TFloat64Quaternion::SlerpLongPath( a, -a, 0.5 );
        ASSERT_TRUE( fabs( halfway.GetLength() - 1.0 ) < 1e-15 && fabs( halfway.Dot( a ) ) < 1e-15 && fabs( a.GetAngleTo( halfway ).GetValue() - g_pi ) < 1e-12 );
        ASSERT_TRUE( fabs( TFloat64Quaternion::SlerpLongPath( a, -a, 0.25 ).GetLength() - 1.0 ) < 1e-15 && fabs( a.GetAngleTo( TFloat64Quaternion::SlerpLongPath( a, -a, 0.25 ) ).GetValue() - g_pi / 2 ) < 1e-12 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Quaternion: fixed point rotations against the Float64 reference" )
    try
    {
        const UInt64 seed = 0xF1C5;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        Float64 worst64 = 0.0;
        Float64 worst32 = 0.0;
        for ( UInt32 i=0; i<1000; ++i )
        {
            TFloat64Quaternion a = RandomUnitQuaternion( generator );
            TFloat64Quaternion b = RandomUnitQuaternion( generator );
            TFloat64Vector3D v = RandomVector( generator, 10.0 );
            TFloat64Vector3D reference = ( b * a ).Rotate( v );
            TFixed64Quaternion fa( a );
            TFixed64Quaternion fb( b );
            TFloat64Vector3D got64( ( fb * fa ).Rotate( TFixed64Vector3D( v ) ) );
            TFixed32Quaternion ga( a );
            TFixed32Quaternion gb( b );
            TFloat64Vector3D got32( ( gb * ga ).Rotate( TFixed32Vector3D( v ) ) );
            Float64 e64 = ( got64 - reference ).GetChebyshevLength();
            Float64 e32 = ( got32 - reference ).GetChebyshevLength();
            if ( e64 > worst64 ) worst64 = e64;
            if ( e32 > worst32 ) worst32 = e32;
            // Q32.32: 2^-33 per rounding of the inputs and products, amplified by |v| <= 17 (about 1.6e-8);
            // Q16.16: 2^-17 per rounding, likewise
            if ( !( e64 <= 4e-8 ) || !( e32 <= 2e-3 ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Quaternion: largest fixed point rotation error, in units of 1e-9 for Q32.32: " << worst64 * 1e9 << ", in units of 1e-6 for Q16.16: " << worst32 * 1e6 );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Quaternion: " << failures << " fixed point rotations off, seed " << seed );
        // slerp end points are exact in fixed point too
        TFixed32Quaternion fy( TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::UnitY(), TFloat64Radian( 1.0 ) ) );
        ASSERT_TRUE( TFixed32Quaternion::Slerp( TFixed32Quaternion(), fy, TFixed32( 0.0 ) ) == TFixed32Quaternion() && TFixed32Quaternion::Slerp( TFixed32Quaternion(), fy, TFixed32( 1.0 ) ) == fy );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Quaternion: realtime-safe operations run in a realtime context" )
    try
    {
        Float32 result = RealtimeQuaternionWork( TFloat32Quaternion::FromAxisAngle( TFloat32DirectionVector3D::UnitX(), TFloat32Radian( 0.4f ) ),
                                                 TFloat32Quaternion::FromAxisAngle( TFloat32DirectionVector3D::UnitY(), TFloat32Radian( -1.1f ) ) );
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( result ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
