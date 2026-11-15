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

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_TRANSFORMS_H
#include "gucefMATH_transforms.h"
#define GUCEF_MATH_TRANSFORMS_H
#endif /* GUCEF_MATH_TRANSFORMS_H ? */

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

#include "TestMathReference.h"
#include "TestTransforms.h"

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

/** A transform with position in [-100, 100]^3, a random rotation and a scale in [0.5, 2] (uniform when asked) */
static TFloat64Transform3D
RandomTransform( MathReference::CSeededSequence& generator, bool uniform )
{
    Float64 s = generator.NextFloat64( 0.5, 2.0 );
    TFloat64Vector3D scale = uniform ? TFloat64Vector3D( s, s, s ) : TFloat64Vector3D( s, generator.NextFloat64( 0.5, 2.0 ), generator.NextFloat64( 0.5, 2.0 ) );
    return TFloat64Transform3D( RandomVector( generator, 100.0 ), RandomUnitQuaternion( generator ), scale );
}

/*-------------------------------------------------------------------------*/

template < typename TVector >
static bool
NearVector( const TVector& a, const TVector& b, Float64 tolerance )
{
    return fabs( (Float64) a.x - (Float64) b.x ) <= tolerance && fabs( (Float64) a.y - (Float64) b.y ) <= tolerance && fabs( (Float64) a.z - (Float64) b.z ) <= tolerance;
}

/*-------------------------------------------------------------------------*/

static Float64
Matrix4Distance( const TFloat64Matrix4D& a, const TFloat64Matrix4D& b )
{
    Float64 largest = 0.0;
    for ( UInt32 r=0; r<4; ++r )
        for ( UInt32 c=0; c<4; ++c )
            if ( fabs( a( r, c ) - b( r, c ) ) > largest )
                largest = fabs( a( r, c ) - b( r, c ) );
    return largest;
}

/*-------------------------------------------------------------------------*/

/** Calls realtime-safe transform operations from a realtime context */
static Float32
RealtimeTransformWork( const TFloat32Transform3D& a, const TFloat32Transform3D& b, const TFloat32Transform2D& c ) GUCEF_MATH_REALTIME_CONTEXT
{
    TFloat32Transform3D combined = TFloat32Transform3D::Combine( a, b );
    TFloat32Transform3D blended = TFloat32Transform3D::Interpolate( a, b, 0.25f );
    TFloat32Transform3D fromMatrix;
    bool ok = TFloat32Transform3D::FromMatrix( combined.ToMatrix(), fromMatrix );
    TFloat32Point3D p = combined.TransformPoint( TFloat32Point3D( 1, 2, 3 ) );
    TFloat32Point3D back = combined.InverseTransformPoint( p );
    TFloat32DirectionVector3D d = blended.TransformDirection( TFloat32DirectionVector3D::Up() );
    TFloat32Transform2D c2 = TFloat32Transform2D::Combine( c, c.Inverse() );
    TFloat32Point2D p2 = TFloat32Transform2D::Interpolate( c, c2, 0.5f ).TransformPoint( TFloat32Point2D( 1, 1 ) );
    TFloat32Float64Transform3D large( TFloat64Vector3D( 1e7, 0, 0 ), a.rotation, a.scale );
    return ( ok ? 1.0f : 0.0f ) + back.x + d.Dot( TFloat32Vector3D( 1, 1, 1 ) ) + p2.y + combined.Inverse().scale.x + c.ToMatrix3D()( 0, 2 ) +
           large.ToMatrixRelativeTo( TFloat64Vector3D( 1e7, 1, 0 ) )( 1, 3 ) + ( a.HasUniformScale( 0.01f ) ? 1.0f : 0.0f ) + c.InverseTransformPoint( p2 ).x;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformTransformsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Transforms" );

    GUCEF_TESTFW_TESTCASE( "Transforms: identity, point and vector maps equal the matrix of the transform" )
    try
    {
        TFloat64Transform3D identity;
        ASSERT_TRUE( identity.position.IsZero() && identity.rotation == TFloat64Quaternion() && TFloat64Vector3D::One() == identity.scale && identity.ToMatrix().IsIdentity() );
        ASSERT_TRUE( identity == TFloat64Transform3D::Identity() );

        // acceptance 8: TransformPoint equals ToMatrix() * p
        const UInt64 seed = 0x7A3D;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<1000; ++i )
        {
            TFloat64Transform3D t = RandomTransform( generator, false );
            TFloat64Point3D p = TFloat64Point3D::Origin() + RandomVector( generator, 10.0 );
            TFloat64Matrix4D m = t.ToMatrix();
            if ( !NearVector( t.TransformPoint( p ), m * p, 1e-12 ) )
                ++failures;
            TFloat64Vector3D v = RandomVector( generator, 10.0 );
            if ( !NearVector( t.TransformVector( v ), m * v, 1e-12 ) )
                ++failures;
            if ( !t.TransformDirection( TFloat64DirectionVector3D( v ) ).Equals( m.TransformDirection( TFloat64DirectionVector3D( v ) ) ) )
                ++failures;
            if ( !NearVector( t.InverseTransformPoint( t.TransformPoint( p ) ), p, 1e-12 ) )
                ++failures;
            TFloat32Transform3D t32( TFloat32Vector3D( t.position ), TFloat32Quaternion( t.rotation ), TFloat32Vector3D( t.scale ) );
            TFloat32Point3D p32( p );
            if ( !NearVector( t32.TransformPoint( p32 ), t32.ToMatrix() * p32, 1e-4 ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Transforms: " << failures << " point or vector maps differ from the matrix, seed " << seed );

        TFloat64Transform3D zeroScale( TFloat64Vector3D( 1, 2, 3 ), TFloat64Quaternion(), TFloat64Vector3D( 0, 1, 1 ) );
        ASSERT_TRUE( TFloat64Point3D( 0, 0, 0 ) == zeroScale.InverseTransformPoint( TFloat64Point3D( 5, 2, 3 ) ) );
        ASSERT_TRUE( zeroScale.TransformDirection( TFloat64DirectionVector3D::UnitX() ) == TFloat64DirectionVector3D::UnitX() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Transforms: Interpolate end points, Combine and Inverse exact for uniform scale, FromMatrix" )
    try
    {
        const UInt64 seed = 0xC0B1;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<1000; ++i )
        {
            TFloat64Transform3D a = RandomTransform( generator, false );
            TFloat64Transform3D b = RandomTransform( generator, false );
            if ( !( TFloat64Transform3D::Interpolate( a, b, 0.0 ) == a ) || !( TFloat64Transform3D::Interpolate( a, b, 1.0 ) == b ) )
                ++failures;
            TFloat64Transform3D middle = TFloat64Transform3D::Interpolate( a, b, 0.5 );
            if ( !NearVector( middle.position, ( a.position + b.position ) * 0.5, 1e-12 ) || !middle.rotation.EqualsRotation( TFloat64Quaternion::Slerp( a.rotation, b.rotation, 0.5 ), TFloat64Radian( 1e-12 ) ) )
                ++failures;

            // uniform scales: Combine equals the matrix product, Inverse the matrix inverse
            TFloat64Transform3D parent = RandomTransform( generator, true );
            TFloat64Transform3D child = RandomTransform( generator, true );
            if ( !( Matrix4Distance( TFloat64Transform3D::Combine( parent, child ).ToMatrix(), parent.ToMatrix() * child.ToMatrix() ) < 1e-11 ) )
                ++failures;
            if ( !( Matrix4Distance( parent.Inverse().ToMatrix(), parent.ToMatrix().Inverse() ) < 1e-12 ) )
                ++failures;
            TFloat64Transform3D roundTrip = TFloat64Transform3D::Combine( parent, parent.Inverse() );
            if ( !NearVector( roundTrip.position, TFloat64Vector3D::Zero(), 1e-11 ) || !roundTrip.rotation.EqualsRotation( TFloat64Quaternion(), TFloat64Radian( 1e-12 ) ) )
                ++failures;

            // FromMatrix recovers the transform
            TFloat64Transform3D recovered;
            if ( !TFloat64Transform3D::FromMatrix( a.ToMatrix(), recovered ) || !NearVector( recovered.position, a.position, 1e-12 ) ||
                 !NearVector( recovered.scale, a.scale, 1e-12 ) || !recovered.rotation.EqualsRotation( a.rotation, TFloat64Radian( 1e-9 ) ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Transforms: " << failures << " interpolation, combination or inverse checks failed, seed " << seed );

        // non-uniform scale under a rotated parent cannot be represented: Combine returns the TRS approximation, still
        // exact for the child's origin
        TFloat64Transform3D parent( TFloat64Vector3D( 1, 2, 3 ), TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::UnitZ(), TFloat64Radian( 0.5 ) ), TFloat64Vector3D( 1, 3, 1 ) );
        TFloat64Transform3D child( TFloat64Vector3D( 4, 5, 6 ), TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::UnitX(), TFloat64Radian( 0.3 ) ), TFloat64Vector3D( 1, 1, 1 ) );
        TFloat64Transform3D combined = TFloat64Transform3D::Combine( parent, child );
        ASSERT_TRUE( NearVector( combined.position, parent.TransformPoint( TFloat64Point3D( 4, 5, 6 ) ).ToPositionVector(), 1e-14 ) );
        ASSERT_FALSE( parent.HasUniformScale( 0.1 ) );
        ASSERT_TRUE( child.HasUniformScale( 0.0 ) );
        // shear is rejected by FromMatrix
        TFloat64Matrix4D shear;
        shear( 0, 1 ) = 0.5;
        TFloat64Transform3D unchanged = parent;
        ASSERT_FALSE( TFloat64Transform3D::FromMatrix( shear, unchanged ) );
        ASSERT_TRUE( unchanged == parent );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Transforms: mirrored uniform scales combine and invert exactly (3D and 2D)" )
    try
    {
        // regression: parent scale ( -2, 2, 2 ) rotated 0.7 rad about Z, child scale 3 rotated 1.1 rad about ( 1, 1, 0 )
        TFloat64Transform3D parent( TFloat64Vector3D( 1, -2, 3 ), TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::UnitZ(), TFloat64Radian( 0.7 ) ), TFloat64Vector3D( -2, 2, 2 ) );
        TFloat64Transform3D child( TFloat64Vector3D( 0.5, 4, -1 ), TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D( TFloat64Vector3D( 1, 1, 0 ) ), TFloat64Radian( 1.1 ) ), TFloat64Vector3D( 3, 3, 3 ) );
        ASSERT_TRUE( parent.HasUniformScale( 1e-12 ) );
        ASSERT_TRUE( Matrix4Distance( TFloat64Transform3D::Combine( parent, child ).ToMatrix(), parent.ToMatrix() * child.ToMatrix() ) < 1e-13 );
        ASSERT_TRUE( Matrix4Distance( parent.Inverse().ToMatrix(), parent.ToMatrix().Inverse() ) < 1e-14 );

        // every sign pattern of a uniform magnitude, seeded rotations, children with any scale
        const UInt64 seed = 0x3141;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<800; ++i )
        {
            const Float64 magnitude = generator.NextFloat64( 0.5, 2.0 );
            TFloat64Vector3D signs( 0 != ( i & 1 ) ? -1.0 : 1.0, 0 != ( i & 2 ) ? -1.0 : 1.0, 0 != ( i & 4 ) ? -1.0 : 1.0 );
            TFloat64Transform3D p( RandomVector( generator, 10.0 ), RandomUnitQuaternion( generator ), signs * magnitude );
            TFloat64Transform3D c = RandomTransform( generator, false );
            if ( !p.HasUniformScale( 1e-12 ) )
                ++failures;
            if ( !( Matrix4Distance( TFloat64Transform3D::Combine( p, c ).ToMatrix(), p.ToMatrix() * c.ToMatrix() ) < 1e-11 ) )
                ++failures;
            if ( !( Matrix4Distance( p.Inverse().ToMatrix(), p.ToMatrix().Inverse() ) < 1e-12 ) )
                ++failures;
            TFloat64Point3D point = TFloat64Point3D::Origin() + RandomVector( generator, 5.0 );
            if ( !NearVector( p.Inverse().TransformPoint( p.TransformPoint( point ) ), point, 1e-12 ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Transforms: " << failures << " mirrored uniform scale checks failed, seed " << seed );

        // 2D: parent ( -1, 1 ) mirrors, the child's rotation turns the other way
        TFloat64Transform2D parent2( TFloat64Vector2D( 1, 2 ), TFloat64Radian( 0.4 ), TFloat64Vector2D( -1, 1 ) );
        TFloat64Transform2D child2( TFloat64Vector2D( 3, -1 ), TFloat64Radian( 0.9 ), TFloat64Vector2D( 2, 2 ) );
        UInt32 failures2 = 0;
        const TFloat64Vector2D patterns[ 4 ] = { TFloat64Vector2D( -1, 1 ), TFloat64Vector2D( 1, -1 ), TFloat64Vector2D( -1, -1 ), TFloat64Vector2D( 1, 1 ) };
        for ( UInt32 n=0; n<4; ++n )
        {
            parent2.scale = patterns[ n ] * 1.5;
            for ( UInt32 k=0; k<20; ++k )
            {
                TFloat64Point2D q( generator.NextFloat64( -5, 5 ), generator.NextFloat64( -5, 5 ) );
                TFloat64Point2D combined = TFloat64Transform2D::Combine( parent2, child2 ).TransformPoint( q );
                TFloat64Point2D nested = parent2.TransformPoint( child2.TransformPoint( q ) );
                TFloat64Point2D undone = parent2.Inverse().TransformPoint( parent2.TransformPoint( q ) );
                if ( !( fabs( combined.x - nested.x ) < 1e-12 && fabs( combined.y - nested.y ) < 1e-12 ) || !( fabs( undone.x - q.x ) < 1e-12 && fabs( undone.y - q.y ) < 1e-12 ) )
                    ++failures2;
            }
        }
        ASSERT_TRUE( 0 == failures2 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Transforms: mixed precision keeps Float64 positions and gives exact origin-relative Float32 matrices" )
    try
    {
        // acceptance 8: a position of 10^7 + 0.01
        TFloat32Float64Transform3D large( TFloat64Vector3D( 1e7 + 0.01, 0, 0 ), TFloat32Quaternion(), TFloat32Vector3D::One() );
        ASSERT_TRUE( 1e7 + 0.01 == large.position.x );
        TFloat32Matrix4D relative = large.ToMatrixRelativeTo( TFloat64Vector3D( 1e7, 0, 0 ) );
        ASSERT_TRUE( 0.01f == relative( 0, 3 ) && 0.0f == relative( 1, 3 ) );
        // without the origin the Float32 matrix cannot hold the offset
        ASSERT_TRUE( 1e7f == large.ToMatrix()( 0, 3 ) );
        // point transforms are computed in Float64
        TFloat64Point3D p = large.TransformPoint( TFloat64Point3D( 0.005, 0, 0 ) );
        ASSERT_TRUE( fabs( p.x - ( 1e7 + 0.015 ) ) < 1e-8 );
        TFloat32Float64Transform3D turned( TFloat64Vector3D( 1e7, 0, 0 ), TFloat32Quaternion::FromAxisAngle( TFloat32DirectionVector3D::UnitY(), TFloat32Radian( TFloat32Degree( 90.0f ) ) ), TFloat32Vector3D( 2, 2, 2 ) );
        TFloat64Point3D q = turned.TransformPoint( TFloat64Point3D( 1, 0, 0 ) );
        ASSERT_TRUE( fabs( q.x - 1e7 ) < 1e-6 && fabs( q.z + 2.0 ) < 1e-6 );
        ASSERT_TRUE( fabs( turned.InverseTransformPoint( q ).x - 1.0 ) < 1e-6 );
        TFloat32Float64Transform3D inverse = turned.Inverse();
        ASSERT_TRUE( fabs( inverse.TransformPoint( q ).x - 1.0 ) < 1e-6 );
        TFloat32Float64Transform3D halfway = TFloat32Float64Transform3D::Interpolate( large, turned, 0.5f );
        ASSERT_TRUE( fabs( halfway.position.x - ( 1e7 + 0.005 ) ) < 4e-9 );    // two ulps of Float64 at 10^7
        TFloat32Float64Transform3D fromMatrix;
        ASSERT_TRUE( TFloat32Float64Transform3D::FromMatrix( TFloat32Matrix4D::Translation( TFloat32Vector3D( 1, 2, 3 ) ), fromMatrix ) && 2.0 == fromMatrix.position.y );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Transforms: 2D transforms against their homogeneous matrices" )
    try
    {
        const UInt64 seed = 0x2D2D;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<1000; ++i )
        {
            Float64 s = generator.NextFloat64( 0.5, 2.0 );
            TFloat64Transform2D t( TFloat64Vector2D( generator.NextFloat64( -50, 50 ), generator.NextFloat64( -50, 50 ) ), TFloat64Radian( generator.NextFloat64( -g_pi, g_pi ) ), TFloat64Vector2D( s, generator.NextFloat64( 0.5, 2.0 ) ) );
            TFloat64Point2D p( generator.NextFloat64( -10, 10 ), generator.NextFloat64( -10, 10 ) );
            TFloat64Matrix3D m = t.ToMatrix3D();
            TFloat64Point2D viaMatrix = m.TransformPoint2D( p );
            TFloat64Point2D viaTransform = t.TransformPoint( p );
            if ( !( fabs( viaMatrix.x - viaTransform.x ) < 1e-12 && fabs( viaMatrix.y - viaTransform.y ) < 1e-12 ) )
                ++failures;
            TFloat64Point2D back = t.InverseTransformPoint( viaTransform );
            if ( !( fabs( back.x - p.x ) < 1e-12 && fabs( back.y - p.y ) < 1e-12 ) )
                ++failures;
            TFloat64Transform2D uniform( t.position, t.rotation, TFloat64Vector2D( s, s ) );
            TFloat64Transform2D other( TFloat64Vector2D( 3, -4 ), TFloat64Radian( generator.NextFloat64( -g_pi, g_pi ) ), TFloat64Vector2D( 1.5, 1.5 ) );
            TFloat64Point2D combined = TFloat64Transform2D::Combine( uniform, other ).TransformPoint( p );
            TFloat64Point2D nested = uniform.TransformPoint( other.TransformPoint( p ) );
            if ( !( fabs( combined.x - nested.x ) < 1e-11 && fabs( combined.y - nested.y ) < 1e-11 ) )
                ++failures;
            TFloat64Point2D undone = uniform.Inverse().TransformPoint( uniform.TransformPoint( p ) );
            if ( !( fabs( undone.x - p.x ) < 1e-11 && fabs( undone.y - p.y ) < 1e-11 ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Transforms: " << failures << " 2D checks failed, seed " << seed );
        // interpolation takes the shorter way around the circle
        TFloat64Transform2D a( TFloat64Vector2D( 0, 0 ), TFloat64Radian( 3.0 ), TFloat64Vector2D( 1, 1 ) );
        TFloat64Transform2D b( TFloat64Vector2D( 2, 4 ), TFloat64Radian( -3.0 ), TFloat64Vector2D( 3, 3 ) );
        TFloat64Transform2D middle = TFloat64Transform2D::Interpolate( a, b, 0.5 );
        ASSERT_TRUE( fabs( fabs( middle.rotation.GetValue() ) - g_pi ) < 1e-12 && TFloat64Vector2D( 1, 2 ) == middle.position && TFloat64Vector2D( 2, 2 ) == middle.scale );
        ASSERT_TRUE( TFloat64Transform2D::Interpolate( a, b, 0.0 ) == a && TFloat64Transform2D::Interpolate( a, b, 1.0 ) == b );
        ASSERT_TRUE( TFloat64Transform2D().ToMatrix3D().IsIdentity() && TFloat64Transform2D::Identity() == TFloat64Transform2D() );
        ASSERT_TRUE( TFloat64Transform2D::Combine( a, b ).rotation.GetValue() > -g_pi && TFloat64Transform2D::Combine( a, b ).rotation.GetValue() <= g_pi );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Transforms: fixed point transforms against the Float64 reference" )
    try
    {
        const UInt64 seed = 0xF17A;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<300; ++i )
        {
            TFloat64Transform3D t = RandomTransform( generator, false );
            TFixed64Transform3D f( TFixed64Vector3D( t.position ), TFixed64Quaternion( t.rotation ), TFixed64Vector3D( t.scale ) );
            TFloat64Point3D p = TFloat64Point3D::Origin() + RandomVector( generator, 10.0 );
            TFloat64Point3D got( f.TransformPoint( TFixed64Point3D( p ) ) );
            if ( !NearVector( got, t.TransformPoint( p ), 1e-7 ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Transforms: " << failures << " Q32.32 transforms off by more than 1e-7, seed " << seed );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Transforms: realtime-safe operations run in a realtime context" )
    try
    {
        TFloat32Transform3D a( TFloat32Vector3D( 1, 2, 3 ), TFloat32Quaternion::FromAxisAngle( TFloat32DirectionVector3D::UnitY(), TFloat32Radian( 0.5f ) ), TFloat32Vector3D( 2, 2, 2 ) );
        TFloat32Transform3D b( TFloat32Vector3D( -1, 0, 4 ), TFloat32Quaternion::FromAxisAngle( TFloat32DirectionVector3D::UnitX(), TFloat32Radian( 0.2f ) ), TFloat32Vector3D( 1, 1, 1 ) );
        TFloat32Transform2D c( TFloat32Vector2D( 3, 4 ), TFloat32Radian( 0.7f ), TFloat32Vector2D( 2, 2 ) );
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( RealtimeTransformWork( a, b, c ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
