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

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef GUCEF_MATH_TRANSFORMS_H
#include "gucefMATH_transforms.h"
#define GUCEF_MATH_TRANSFORMS_H
#endif /* GUCEF_MATH_TRANSFORMS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_MATH_CCOORDINATESYSTEM_H
#include "gucefMATH_CCoordinateSystem.h"
#define GUCEF_MATH_CCOORDINATESYSTEM_H
#endif /* GUCEF_MATH_CCOORDINATESYSTEM_H ? */

#include "TestMathReference.h"
#include "TestCoordinateSystems.h"

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

static Float64
Matrix3Distance( const TFloat64Matrix3D& a, const TFloat64Matrix3D& b )
{
    Float64 largest = 0.0;
    for ( UInt32 r=0; r<3; ++r )
        for ( UInt32 c=0; c<3; ++c )
            if ( fabs( a( r, c ) - b( r, c ) ) > largest )
                largest = fabs( a( r, c ) - b( r, c ) );
    return largest;
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

/** Calls the coordinate system conversions from a realtime context */
static Float32
RealtimeCoordinateWork( const CCoordinateSystem& a, const CCoordinateSystem& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    TFloat32Matrix3D m = a.GetConversionMatrixTo< Float32 >( b );
    TFloat32Quaternion q = a.ConvertRotation( b, TFloat32Quaternion::FromAxisAngle( TFloat32DirectionVector3D::UnitY(), TFloat32Radian( 0.5f ) ) );
    TFloat32Transform3D t = a.ConvertTransform( b, TFloat32Transform3D() );
    return m( 0, 0 ) + q.y + t.scale.x + ( a.ConversionFlipsWinding( b ) ? 1.0f : 0.0f ) + (Float32) a.GetHandedness() + a.GetForward< Float32 >().z;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformCoordinateSystemsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "CoordinateSystems" );

    GUCEF_TESTFW_TESTCASE( "CoordinateSystems: presets, derived handedness, validity, axis vectors" )
    try
    {
        // acceptance 12: handedness of every preset
        ASSERT_TRUE( HANDEDNESS_RIGHT == CCoordinateSystem::GucefDefault().GetHandedness() );
        ASSERT_TRUE( HANDEDNESS_LEFT == CCoordinateSystem::Unity().GetHandedness() );
        ASSERT_TRUE( HANDEDNESS_LEFT == CCoordinateSystem::Unreal().GetHandedness() );
        ASSERT_TRUE( HANDEDNESS_RIGHT == CCoordinateSystem::Blender().GetHandedness() );
        ASSERT_TRUE( HANDEDNESS_RIGHT == CCoordinateSystem::Godot().GetHandedness() );
        ASSERT_TRUE( CCoordinateSystem::GucefDefault().IsValid() && CCoordinateSystem::Unreal().IsValid() && 100.0 == CCoordinateSystem::Unreal().GetUnitsPerMetre() );
        ASSERT_TRUE( CCoordinateSystem() == CCoordinateSystem::GucefDefault() && CCoordinateSystem::Godot() == CCoordinateSystem::GucefDefault() && CCoordinateSystem::Unity() != CCoordinateSystem::GucefDefault() );
        ASSERT_TRUE( SIGNEDAXIS_POSITIVE_Y == CCoordinateSystem::Unreal().GetRightAxis() && SIGNEDAXIS_POSITIVE_Z == CCoordinateSystem::Unreal().GetUpAxis() && SIGNEDAXIS_POSITIVE_X == CCoordinateSystem::Unreal().GetForwardAxis() );
        // handedness agrees with right x up == -forward for every valid combination of axes
        UInt32 failures = 0;
        UInt32 valid = 0;
        for ( UInt32 r=0; r<6; ++r )
            for ( UInt32 u=0; u<6; ++u )
                for ( UInt32 f=0; f<6; ++f )
                {
                    if ( r / 2 == u / 2 || r / 2 == f / 2 || u / 2 == f / 2 )
                        continue;
                    ++valid;
                    CCoordinateSystem system( (TSignedAxis) r, (TSignedAxis) u, (TSignedAxis) f, 1.0 );
                    TFloat64Vector3D cross = system.GetRight< Float64 >().Cross( system.GetUp< Float64 >() );
                    bool right = cross == -system.GetForward< Float64 >();
                    if ( !system.IsValid() || right != ( HANDEDNESS_RIGHT == system.GetHandedness() ) )
                        ++failures;
                }
        ASSERT_TRUE( 0 == failures && 48 == valid );
        ASSERT_TRUE( TFloat32Vector3D( 0, 0, -1 ) == CCoordinateSystem::GucefDefault().GetForward< Float32 >() && TInt32Vector3D( 0, 1, 0 ) == CCoordinateSystem::Unreal().GetRight< Int32 >() );
        // axes on one coordinate axis or a non-positive unit scale are invalid; debug builds assert on them unless the
        // assert is off, as it is in this test application (GUCEF_MATH_NO_COORDINATESYSTEM_VALIDITY_ASSERT)
        ASSERT_FALSE( CCoordinateSystem::IsValidityAssertEnabled() );
        ASSERT_FALSE( CCoordinateSystem( SIGNEDAXIS_POSITIVE_X, SIGNEDAXIS_NEGATIVE_X, SIGNEDAXIS_POSITIVE_Z, 1.0 ).IsValid() );
        ASSERT_FALSE( CCoordinateSystem( SIGNEDAXIS_POSITIVE_X, SIGNEDAXIS_POSITIVE_Y, SIGNEDAXIS_POSITIVE_Z, 0.0 ).IsValid() );
        ASSERT_FALSE( CCoordinateSystem( SIGNEDAXIS_POSITIVE_Y, SIGNEDAXIS_POSITIVE_Y, SIGNEDAXIS_POSITIVE_Y, 1.0 ).IsValid() );
        ASSERT_FALSE( CCoordinateSystem( SIGNEDAXIS_POSITIVE_X, SIGNEDAXIS_POSITIVE_Y, SIGNEDAXIS_POSITIVE_Z, -1.0 ).IsValid() );
        // an invalid system is refused by the text and DataNode forms, which keep the value they have
        CCoordinateSystem kept = CCoordinateSystem::Unity();
        ASSERT_FALSE( kept.FromString( CCoordinateSystem( SIGNEDAXIS_POSITIVE_X, SIGNEDAXIS_NEGATIVE_X, SIGNEDAXIS_POSITIVE_Z, 1.0 ).ToString() ) );
        ASSERT_TRUE( kept == CCoordinateSystem::Unity() );
        CORE::CDataNode invalidNode;
        ASSERT_TRUE( CCoordinateSystem( SIGNEDAXIS_POSITIVE_Z, SIGNEDAXIS_POSITIVE_Y, SIGNEDAXIS_NEGATIVE_Z, 1.0 ).Serialize( invalidNode, CORE::CDataNodeSerializableSettings() ) );
        ASSERT_FALSE( kept.Deserialize( invalidNode, CORE::CDataNodeSerializableSettings() ) );
        ASSERT_TRUE( kept == CCoordinateSystem::Unity() );
        // switching the assert back on and off again
        CCoordinateSystem::SetValidityAssertEnabled( true );
        ASSERT_TRUE( CCoordinateSystem::IsValidityAssertEnabled() );
        CCoordinateSystem::SetValidityAssertEnabled( false );
        ASSERT_FALSE( CCoordinateSystem::IsValidityAssertEnabled() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CoordinateSystems: conversions to Unity, Unreal and Blender" )
    try
    {
        // acceptance 12
        const CCoordinateSystem gucef = CCoordinateSystem::GucefDefault();
        TFloat64Matrix3D toUnity = gucef.GetConversionMatrixTo< Float64 >( CCoordinateSystem::Unity() );
        ASSERT_TRUE( TFloat64Vector3D( 0, 0, 1 ) == toUnity * TFloat64Vector3D( 0, 0, -1 ) && gucef.ConversionFlipsWinding( CCoordinateSystem::Unity() ) );
        TFloat64Matrix3D toUnreal = gucef.GetConversionMatrixTo< Float64 >( CCoordinateSystem::Unreal() );
        ASSERT_TRUE( TFloat64Vector3D( 100, 0, 0 ) == toUnreal * TFloat64Vector3D( 0, 0, -1 ) );
        ASSERT_TRUE( TFloat64Vector3D( 0, 0, 100 ) == toUnreal * TFloat64Vector3D( 0, 1, 0 ) );
        ASSERT_TRUE( TFloat64Vector3D( 0, 100, 0 ) == toUnreal * TFloat64Vector3D( 1, 0, 0 ) );
        ASSERT_TRUE( gucef.ConversionFlipsWinding( CCoordinateSystem::Unreal() ) && 100.0 == gucef.GetConversionScale( CCoordinateSystem::Unreal() ) );
        TFloat64Matrix3D toBlender = gucef.GetConversionMatrixTo< Float64 >( CCoordinateSystem::Blender() );
        ASSERT_TRUE( TFloat64Vector3D( 0, 1, 0 ) == toBlender * TFloat64Vector3D( 0, 0, -1 ) && TFloat64Vector3D( 0, 0, 1 ) == toBlender * TFloat64Vector3D( 0, 1, 0 ) );
        ASSERT_TRUE( 1.0 == toBlender.GetDeterminant() && !gucef.ConversionFlipsWinding( CCoordinateSystem::Blender() ) );
        ASSERT_FALSE( gucef.ConversionFlipsWinding( CCoordinateSystem::Godot() ) );
        ASSERT_TRUE( gucef.GetConversionMatrixTo< Float64 >( CCoordinateSystem::Godot() ).IsIdentity() );
        TFloat64Matrix4D toUnreal4 = gucef.GetConversionMatrix4To< Float64 >( CCoordinateSystem::Unreal() );
        ASSERT_TRUE( TFloat64Point3D( 100, 0, 0 ) == toUnreal4 * TFloat64Point3D( 0, 0, -1 ) && toUnreal4.IsAffine() );
        Int32 permutation[ 9 ];
        gucef.GetConversionPermutation( CCoordinateSystem::Unity(), permutation );
        ASSERT_TRUE( 1 == permutation[ 0 ] && 1 == permutation[ 4 ] && -1 == permutation[ 8 ] );

        // ConvertRotation of +90 degrees about +Y to Unity gives -90 degrees about Unity +Y
        TFloat64Quaternion yaw = TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::UnitY(), TFloat64Radian( g_pi / 2 ) );
        TFloat64Quaternion inUnity = gucef.ConvertRotation( CCoordinateSystem::Unity(), yaw );
        ASSERT_TRUE( inUnity.EqualsRotation( TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::UnitY(), TFloat64Radian( -g_pi / 2 ) ), TFloat64Radian( 1e-15 ) ) );
        // the converted rotation acts on converted vectors as the original acts on the originals: M R v == R' M v
        TFloat64Quaternion general = TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D( TFloat64Vector3D( 1, 2, -3 ) ), TFloat64Radian( 0.8 ) );
        const CCoordinateSystem targets[ 4 ] = { CCoordinateSystem::Unity(), CCoordinateSystem::Unreal(), CCoordinateSystem::Blender(), CCoordinateSystem::Godot() };
        UInt32 failures = 0;
        for ( UInt32 i=0; i<4; ++i )
        {
            TFloat64Matrix3D m = gucef.GetConversionMatrixTo< Float64 >( targets[ i ] );
            TFloat64Quaternion converted = gucef.ConvertRotation( targets[ i ], general );
            TFloat64Vector3D v( 0.3, -1.2, 2.5 );
            if ( !( ( m * general.Rotate( v ) - converted.Rotate( m * v ) ).GetLength() < 1e-12 ) )
                ++failures;
            // the transform: M T M^-1 equals the matrix of the converted transform
            TFloat64Transform3D t( TFloat64Vector3D( 1, 2, 3 ), general, TFloat64Vector3D( 2, 3, 4 ) );
            TFloat64Transform3D ct = gucef.ConvertTransform( targets[ i ], t );
            TFloat64Matrix4D m4 = gucef.GetConversionMatrix4To< Float64 >( targets[ i ] );
            if ( !( Matrix4Distance( ct.ToMatrix(), m4 * t.ToMatrix() * m4.Inverse() ) < 1e-12 ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CoordinateSystems: conversions compose and invert" )
    try
    {
        // acceptance 12: A -> B -> C equals A -> C, A -> B -> A is the identity, for every pair and triple of axis sets
        CCoordinateSystem systems[ 49 ];
        UInt32 count = 0;
        for ( UInt32 r=0; r<6; ++r )
            for ( UInt32 u=0; u<6; ++u )
                for ( UInt32 f=0; f<6; ++f )
                {
                    if ( r / 2 == u / 2 || r / 2 == f / 2 || u / 2 == f / 2 )
                        continue;
                    const Float64 unitsPerMetre = 1.0 + ( count % 3 ) * 49.5;
                    systems[ count ] = CCoordinateSystem( (TSignedAxis) r, (TSignedAxis) u, (TSignedAxis) f, unitsPerMetre );
                    ++count;
                }
        UInt32 failures = 0;
        TFloat64Quaternion q = TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D( TFloat64Vector3D( -2, 1, 0.5 ) ), TFloat64Radian( 1.3 ) );
        for ( UInt32 a=0; a<count; ++a )
        {
            for ( UInt32 b=0; b<count; ++b )
            {
                TFloat64Matrix3D ab = systems[ a ].GetConversionMatrixTo< Float64 >( systems[ b ] );
                TFloat64Matrix3D ba = systems[ b ].GetConversionMatrixTo< Float64 >( systems[ a ] );
                if ( !( Matrix3Distance( ba * ab, TFloat64Matrix3D() ) < 1e-14 ) )
                    ++failures;
                if ( !systems[ b ].ConvertRotation( systems[ a ], systems[ a ].ConvertRotation( systems[ b ], q ) ).EqualsRotation( q, TFloat64Radian( 1e-15 ) ) )
                    ++failures;
                const UInt32 c = ( a * 7 + b * 13 ) % count;
                TFloat64Matrix3D bc = systems[ b ].GetConversionMatrixTo< Float64 >( systems[ c ] );
                TFloat64Matrix3D ac = systems[ a ].GetConversionMatrixTo< Float64 >( systems[ c ] );
                if ( !( Matrix3Distance( bc * ab, ac ) < 1e-12 ) )
                    ++failures;
                TFloat64Quaternion viaB = systems[ b ].ConvertRotation( systems[ c ], systems[ a ].ConvertRotation( systems[ b ], q ) );
                if ( !viaB.EqualsRotation( systems[ a ].ConvertRotation( systems[ c ], q ), TFloat64Radian( 1e-15 ) ) )
                    ++failures;
                if ( systems[ a ].ConversionFlipsWinding( systems[ b ] ) != ( ab.GetDeterminant() < 0.0 ) )
                    ++failures;
            }
        }
        ASSERT_TRUE( 48 == count && 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CoordinateSystems: " << failures << " composition checks failed" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CoordinateSystems: realtime-safe conversions run in a realtime context" )
    try
    {
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( RealtimeCoordinateWork( CCoordinateSystem::GucefDefault(), CCoordinateSystem::Unreal() ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
