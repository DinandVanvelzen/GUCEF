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

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef GUCEF_MATH_CTSPHERICALHARMONICSROTATION_H
#include "gucefMATH_CTSphericalHarmonicsRotation.h"
#define GUCEF_MATH_CTSPHERICALHARMONICSROTATION_H
#endif /* GUCEF_MATH_CTSPHERICALHARMONICSROTATION_H ? */

#ifndef GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H
#include "gucefMATH_LowDiscrepancySequences.h"
#define GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H
#endif /* GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H ? */

#ifndef GUCEF_MATH_CTRANDOMSTREAM_H
#include "gucefMATH_CTRandomStream.h"
#define GUCEF_MATH_CTRANDOMSTREAM_H
#endif /* GUCEF_MATH_CTRANDOMSTREAM_H ? */

#include "TestMathReference.h"
#include "TestMathAllocationCounter.h"
#include "TestSphericalHarmonics.h"

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

/** The Legendre polynomial P_l( x ) by Bonnet's recurrence, independent of the associated-Legendre code under test */
static Float64
Legendre( UInt32 l, Float64 x )
{
    Float64 previous = 1.0;
    if ( 0 == l )
        return previous;
    Float64 current = x;
    for ( UInt32 n=1; n<l; ++n )
    {
        const Float64 next = ( (Float64) ( 2 * n + 1 ) * x * current - (Float64) n * previous ) / (Float64) ( n + 1 );
        previous = current;
        current = next;
    }
    return current;
}

/*-------------------------------------------------------------------------*/

/** n! in Float64 */
static Float64
Factorial( UInt32 n )
{
    Float64 f = 1.0;
    for ( UInt32 i=2; i<=n; ++i )
        f *= (Float64) i;
    return f;
}

/*-------------------------------------------------------------------------*/

/**
 *  The SN3D real spherical harmonic (no Condon-Shortley phase) from the explicit polynomial of P_l (Rodrigues),
 *  differentiated m times, and the azimuth through atan2 / cos / sin: independent of the recurrences under test
 */
static Float64
ReferenceSn3d( UInt32 l, Int32 m, Float64 x, Float64 y, Float64 z )
{
    const UInt32 am = (UInt32) ( m < 0 ? -m : m );
    // P_l( t ) = 2^-l sum_k ( -1 )^k C( l, k ) C( 2l - 2k, l ) t^( l - 2k ); the m-th derivative term by term
    Float64 derivative = 0.0;
    for ( UInt32 k=0; 2*k<=l; ++k )
    {
        const UInt32 power = l - 2 * k;
        if ( power < am )
            continue;
        const Float64 coefficient = ( 0 == k % 2 ? 1.0 : -1.0 ) * Factorial( l ) / ( Factorial( k ) * Factorial( l - k ) )
                                  * Factorial( 2 * l - 2 * k ) / ( Factorial( l ) * Factorial( l - 2 * k ) );
        derivative += coefficient * Factorial( power ) / Factorial( power - am ) * pow( z, (Float64) ( power - am ) );
    }
    derivative /= pow( 2.0, (Float64) l );
    const Float64 legendre = pow( 1.0 - z * z > 0.0 ? 1.0 - z * z : 0.0, 0.5 * (Float64) am ) * derivative;
    const Float64 norm = sqrt( ( 0 == am ? 1.0 : 2.0 ) * Factorial( l - am ) / Factorial( l + am ) );
    const Float64 azimuth = atan2( y, x );
    const Float64 angular = m > 0 ? cos( am * azimuth ) : ( m < 0 ? sin( am * azimuth ) : 1.0 );
    return norm * legendre * angular;
}

/*-------------------------------------------------------------------------*/

/**
 *  For every order 1 to 3: rotating the projection of a delta in d equals the projection of a delta in R d.
 *  Returns the largest difference over the rotations and directions drawn from 'stream'.
 */
template < UInt32 Order >
static Float64
RotationError32( CRandomStream& stream, UInt32 count, TShNormalisation norm, Float64& worstEnergyChange )
{
    typedef CTSphericalHarmonics< Float32, Order > TSh;
    Float64 worst = 0.0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const TFloat32Quaternion q = stream.RandomRotationAs< Float32 >();
        const TFloat32Vector3D v = stream.OnUnitSphereAs< Float32 >();
        const TFloat32DirectionVector3D d( v );
        const TFloat32Matrix3D rotation = TFloat32Matrix3D::FromQuaternion( q );
        CTSphericalHarmonicsRotation< Float32, Order > shRotation( rotation );
        TSh projection;
        projection.AddDirection( d, 1.0f, norm );
        TSh rotated;
        shRotation.Apply( projection, rotated );
        TSh expected;
        expected.AddDirection( rotation.TransformDirection( d ), 1.0f, norm );
        for ( UInt32 k=0; k<TSh::CoefficientCount; ++k )
        {
            const Float64 difference = fabs( (Float64) rotated.coefficients[ k ] - (Float64) expected.coefficients[ k ] );
            worst = difference > worst ? difference : worst;
        }
        for ( UInt32 l=0; l<=Order; ++l )
        {
            const Float64 change = fabs( (Float64) rotated.GetBandEnergy( l ) - (Float64) projection.GetBandEnergy( l ) );
            worstEnergyChange = change > worstEnergyChange ? change : worstEnergyChange;
        }
    }
    return worst;
}

/*-------------------------------------------------------------------------*/

/** Evaluation and rotation from a realtime context: a RealtimeSanitizer build reports any allocation */
static Float32
RealtimeShWork( const TFloat32DirectionVector3D& d, const TFloat32Matrix3D& rotation ) GUCEF_MATH_REALTIME_CONTEXT
{
    TFloat32SphericalHarmonics3 sh;
    sh.AddDirection( d, 1.0f, SHNORM_SN3D );
    Float32 basis[ 16 ];
    TFloat32SphericalHarmonics3::EvaluateBasis( d, SHNORM_N3D, basis );
    TFloat32SphericalHarmonicsRotation3 shRotation;
    shRotation.Set( rotation );
    shRotation.Apply( sh, sh );
    sh.ConvolveWithCosineLobe();
    return basis[ 5 ] + sh.Evaluate( d, SHNORM_SN3D ) + sh.Dot( sh ) + sh.GetBandEnergy( 2 ) + shRotation.GetElement( 2, 1, -1 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSphericalHarmonicsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "SphericalHarmonics" );

    GUCEF_TESTFW_TESTCASE( "SphericalHarmonics: Y00, SN3D W, ACN indices and the closed forms of orders 1 to 3" )
    try
    {
        typedef CTSphericalHarmonics< Float64, 3 > TSh;
        ASSERT_TRUE( 16 == TSh::CoefficientCount && 4 == TSh::BandCount );
        ASSERT_TRUE( 0 == TSh::GetIndex( 0, 0 ) && 1 == TSh::GetIndex( 1, -1 ) && 2 == TSh::GetIndex( 1, 0 ) && 3 == TSh::GetIndex( 1, 1 ) );
        ASSERT_TRUE( 6 == TSh::GetIndex( 2, 0 ) && 15 == TSh::GetIndex( 3, 3 ) && 9 == TSh::GetIndex( 3, -3 ) );
        for ( UInt32 index=0; index<64; ++index )
        {
            const UInt32 l = CTSphericalHarmonicsBasis< Float64 >::GetDegree( index );
            ASSERT_TRUE( l * l <= index && index < ( l + 1 ) * ( l + 1 ) );
        }

        const UInt64 seed = 0x4B70;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<500; ++i )
        {
            const Float64 vx = generator.NextFloat64( -1, 1 );
            const Float64 vy = generator.NextFloat64( -1, 1 );
            const Float64 vz = generator.NextFloat64( -1, 1 );
            TFloat64DirectionVector3D d;
            if ( !TFloat64DirectionVector3D::TryCreate( TFloat64Vector3D( vx, vy, vz ), d ) )
                continue;
            const Float64 x = d.GetX();
            const Float64 y = d.GetY();
            const Float64 z = d.GetZ();
            Float64 orthonormal[ 16 ];
            Float64 sn3d[ 16 ];
            Float64 n3d[ 16 ];
            TSh::EvaluateBasis( d, SHNORM_ORTHONORMAL, orthonormal );
            TSh::EvaluateBasis( d, SHNORM_SN3D, sn3d );
            TSh::EvaluateBasis( d, SHNORM_N3D, n3d );
            // Y00 = 1 / ( 2 sqrt( pi ) ) = 0.28209479; SN3D W is 1; first order SN3D is ( y, z, x )
            if ( !( fabs( orthonormal[ 0 ] - 0.28209479177387814 ) <= 1e-15 ) || !( 1.0 == sn3d[ 0 ] ) )
                ++failures;
            if ( !( fabs( sn3d[ 1 ] - y ) <= 1e-15 && fabs( sn3d[ 2 ] - z ) <= 1e-15 && fabs( sn3d[ 3 ] - x ) <= 1e-15 ) )
                ++failures;
            // the closed forms of orders 2 and 3 (orthonormal, no Condon-Shortley phase)
            const Float64 expected[ 16 ] =
            {
                0.28209479177387814,
                0.4886025119029199 * y, 0.4886025119029199 * z, 0.4886025119029199 * x,
                1.0925484305920792 * x * y, 1.0925484305920792 * y * z, 0.31539156525252005 * ( 3 * z * z - 1 ), 1.0925484305920792 * x * z, 0.5462742152960396 * ( x * x - y * y ),
                0.5900435899266435 * y * ( 3 * x * x - y * y ), 2.890611442640554 * x * y * z, 0.4570457994644658 * y * ( 5 * z * z - 1 ), 0.3731763325901154 * z * ( 5 * z * z - 3 ),
                0.4570457994644658 * x * ( 5 * z * z - 1 ), 1.445305721320277 * z * ( x * x - y * y ), 0.5900435899266435 * x * ( x * x - 3 * y * y )
            };
            for ( UInt32 k=0; k<16; ++k )
            {
                if ( !( fabs( orthonormal[ k ] - expected[ k ] ) <= 1e-14 ) )
                    ++failures;
                const UInt32 l = CTSphericalHarmonicsBasis< Float64 >::GetDegree( k );
                // N3D = sqrt( 2l + 1 ) SN3D = sqrt( 4 pi ) orthonormal
                if ( !( fabs( n3d[ k ] - sqrt( 2.0 * l + 1.0 ) * sn3d[ k ] ) <= 1e-14 ) || !( fabs( n3d[ k ] - sqrt( 4.0 * g_pi ) * orthonormal[ k ] ) <= 1e-13 ) )
                    ++failures;
            }
            // the zonal harmonics against Bonnet's Legendre recurrence, orders 0 to 7
            Float64 order7[ 64 ];
            CTSphericalHarmonics< Float64, 7 >::EvaluateBasis( d, SHNORM_ORTHONORMAL, order7 );
            for ( UInt32 l=0; l<=7; ++l )
                if ( !( fabs( order7[ l * l + l ] - sqrt( ( 2.0 * l + 1.0 ) / ( 4.0 * g_pi ) ) * Legendre( l, z ) ) <= 1e-13 ) )
                    ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SphericalHarmonics: " << failures << " basis values off, seed " << seed );
        // the poles: only the zonal harmonics are non-zero
        Float64 pole[ 16 ];
        TSh::EvaluateBasis( TFloat64DirectionVector3D::UnitZ(), SHNORM_SN3D, pole );
        ASSERT_TRUE( 1.0 == pole[ 0 ] && 1.0 == pole[ 2 ] && 1.0 == pole[ 6 ] && 1.0 == pole[ 12 ] && 0.0 == pole[ 1 ] && 0.0 == pole[ 15 ] );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SphericalHarmonics: orthonormal up to order 7 by Fibonacci-sphere quadrature with 10^5 points within 1e-3" )
    try
    {
        const UInt32 points = 100000;
        const UInt32 count = 64;
        static Float64 gram[ 64 ][ 64 ];
        memset( gram, 0, sizeof( gram ) );
        Float64 basis[ 64 ];
        for ( UInt32 p=0; p<points; ++p )
        {
            const TFloat64Vector3D v = FibonacciSphere< Float64 >( p, points );
            CTSphericalHarmonicsBasis< Float64 >::Evaluate( v.x, v.y, v.z, 7, SHNORM_ORTHONORMAL, basis );
            for ( UInt32 i=0; i<count; ++i )
                for ( UInt32 j=i; j<count; ++j )
                    gram[ i ][ j ] += basis[ i ] * basis[ j ];
        }
        Float64 worst = 0.0;
        for ( UInt32 i=0; i<count; ++i )
            for ( UInt32 j=i; j<count; ++j )
            {
                const Float64 value = gram[ i ][ j ] * 4.0 * g_pi / (Float64) points;
                const Float64 error = fabs( value - ( i == j ? 1.0 : 0.0 ) );
                worst = error > worst ? error : worst;
            }
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "SphericalHarmonics: largest Gram matrix error to order 7 with 10^5 Fibonacci points in units of 1e-6 " << worst * 1e6 );
        ASSERT_TRUE( worst <= 1e-3 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SphericalHarmonics: projection, evaluation and dot products obey the addition theorem" )
    try
    {
        const UInt64 seed = 0x4B71;
        MathReference::CSeededSequence generator( seed );
        typedef CTSphericalHarmonics< Float64, 7 > TSh7;
        UInt32 failures = 0;
        for ( UInt32 i=0; i<200; ++i )
        {
            const Float64 ax = generator.NextFloat64( -1, 1 );
            const Float64 ay = generator.NextFloat64( -1, 1 );
            const Float64 az = generator.NextFloat64( -1, 1 );
            const Float64 bx = generator.NextFloat64( -1, 1 );
            const Float64 by = generator.NextFloat64( -1, 1 );
            const Float64 bz = generator.NextFloat64( -1, 1 );
            TFloat64DirectionVector3D a;
            TFloat64DirectionVector3D b;
            if ( !TFloat64DirectionVector3D::TryCreate( TFloat64Vector3D( ax, ay, az ), a ) || !TFloat64DirectionVector3D::TryCreate( TFloat64Vector3D( bx, by, bz ), b ) )
                continue;
            TSh7 pa;
            TSh7 pb;
            pa.AddDirection( a, 1.0, SHNORM_ORTHONORMAL );
            pb.AddDirection( b, 1.0, SHNORM_ORTHONORMAL );
            // sum_m Y_lm( a ) Y_lm( b ) = ( 2l + 1 ) / 4 pi P_l( a . b )
            Float64 expected = 0.0;
            for ( UInt32 l=0; l<=7; ++l )
                expected += ( 2.0 * l + 1.0 ) / ( 4.0 * g_pi ) * Legendre( l, a.Dot( b.AsVector() ) );
            if ( !( fabs( pa.Dot( pb ) - expected ) <= 1e-12 ) || !( fabs( pa.Evaluate( b, SHNORM_ORTHONORMAL ) - expected ) <= 1e-12 ) )
                ++failures;
            // the value of the projection at its own direction: ( L + 1 )^2 / 4 pi
            if ( !( fabs( pa.Evaluate( a, SHNORM_ORTHONORMAL ) - 64.0 / ( 4.0 * g_pi ) ) <= 1e-12 ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SphericalHarmonics: " << failures << " addition theorem checks failed, seed " << seed );

        // Add, Scale, SetZero, Equals, weights
        TFloat64SphericalHarmonics2 sh;
        sh.AddDirection( TFloat64DirectionVector3D::UnitX(), 2.0, SHNORM_SN3D );
        ASSERT_TRUE( 2.0 == sh.coefficients[ 0 ] && 2.0 == sh.coefficients[ 3 ] && 0.0 == sh.coefficients[ 1 ] );
        TFloat64SphericalHarmonics2 doubled = sh;
        doubled.Add( sh );
        TFloat64SphericalHarmonics2 scaled = sh;
        scaled.Scale( 2.0 );
        ASSERT_TRUE( doubled.Equals( scaled, 0.0, 0.0 ) && !doubled.Equals( sh, 1e-12, 0.0 ) );
        scaled.SetZero();
        ASSERT_TRUE( 0.0 == scaled.Dot( sh ) && 0.0 == scaled.GetBandEnergy( 1 ) && 0.0 == sh.GetBandEnergy( 5 ) );
        ASSERT_TRUE( 4.0 == sh.GetBandEnergy( 1 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SphericalHarmonics: rotating the projection of d gives the projection of R d within 1e-5 (Float32, orders 1 to 3, 10^3 rotations)" )
    try
    {
        const UInt64 seed = 0x4B72;
        CRandomStream stream( CPcg32( seed, 7 ) );
        Float64 energy = 0.0;
        const Float64 error1 = RotationError32< 1 >( stream, 1000, SHNORM_SN3D, energy );
        const Float64 error2 = RotationError32< 2 >( stream, 1000, SHNORM_SN3D, energy );
        const Float64 error3 = RotationError32< 3 >( stream, 1000, SHNORM_SN3D, energy );
        const Float64 error3n = RotationError32< 3 >( stream, 1000, SHNORM_ORTHONORMAL, energy );
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "SphericalHarmonics: Float32 rotation errors in units of 1e-7: order 1 " << error1 * 1e7 << ", 2 " << error2 * 1e7 << ", 3 " << error3 * 1e7 << " (orthonormal " << error3n * 1e7 << "), band energy change " << energy * 1e7 );
        ASSERT_TRUE( error1 <= 1e-5 && error2 <= 1e-5 && error3 <= 1e-5 && error3n <= 1e-5 && energy <= 1e-5 );
        if ( !( error1 <= 1e-5 && error2 <= 1e-5 && error3 <= 1e-5 && error3n <= 1e-5 ) )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SphericalHarmonics: rotation off, seed " << seed );

        // the band matrices of a known rotation: 90 degrees about +Z maps +X to +Y, so ( y, z, x ) -> ( x, z, -y )
        TFloat64SphericalHarmonicsRotation1 quarter( TFloat64Matrix3D::RotationZ( TFloat64Radian( g_pi / 2.0 ) ) );
        TFloat64SphericalHarmonics1 px;
        px.AddDirection( TFloat64DirectionVector3D::UnitX(), 1.0, SHNORM_SN3D );
        quarter.Apply( px, px );
        ASSERT_TRUE( fabs( px.coefficients[ 1 ] - 1.0 ) < 1e-15 && fabs( px.coefficients[ 3 ] ) < 1e-15 && 1.0 == px.coefficients[ 0 ] );
        // the identity rotation and the quaternion constructor
        TFloat64SphericalHarmonicsRotation3 identity;
        ASSERT_TRUE( 1.0 == identity.GetElement( 3, -2, -2 ) && 0.0 == identity.GetElement( 3, -2, 1 ) && 0.0 == identity.GetElement( 4, 0, 0 ) );
        TFloat64SphericalHarmonicsRotation3 fromQuaternion( TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::UnitZ(), TFloat64Radian( g_pi / 2.0 ) ) );
        CTSphericalHarmonicsRotation< Float64, 3 > fromMatrix( TFloat64Matrix3D::RotationZ( TFloat64Radian( g_pi / 2.0 ) ) );
        for ( Int32 m=-3; m<=3; ++m )
            for ( Int32 n=-3; n<=3; ++n )
                ASSERT_TRUE( fabs( fromQuaternion.GetElement( 3, m, n ) - fromMatrix.GetElement( 3, m, n ) ) < 1e-14 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SphericalHarmonics: rotation to order 7 keeps band matrices orthogonal and preserves band energy (Float64)" )
    try
    {
        const UInt64 seed = 0x4B73;
        CRandomStream stream( CPcg32( seed, 3 ) );
        typedef CTSphericalHarmonics< Float64, 7 > TSh7;
        Float64 worstOrthogonality = 0.0;
        Float64 worstProjection = 0.0;
        Float64 worstEnergy = 0.0;
        for ( UInt32 i=0; i<200; ++i )
        {
            const TFloat64Quaternion q = stream.RandomRotationAs< Float64 >();
            const TFloat64Vector3D v = stream.OnUnitSphereAs< Float64 >();
            const TFloat64Matrix3D rotation = TFloat64Matrix3D::FromQuaternion( q );
            CTSphericalHarmonicsRotation< Float64, 7 > shRotation( rotation );
            for ( UInt32 l=0; l<=7; ++l )
            {
                const Int32 li = (Int32) l;
                for ( Int32 a=-li; a<=li; ++a )
                    for ( Int32 b=-li; b<=li; ++b )
                    {
                        Float64 sum = 0.0;
                        for ( Int32 k=-li; k<=li; ++k )
                            sum += shRotation.GetElement( l, a, k ) * shRotation.GetElement( l, b, k );
                        const Float64 error = fabs( sum - ( a == b ? 1.0 : 0.0 ) );
                        worstOrthogonality = error > worstOrthogonality ? error : worstOrthogonality;
                    }
            }
            const TFloat64DirectionVector3D d( v );
            TSh7 projection;
            projection.AddDirection( d, 1.0, SHNORM_N3D );
            TSh7 rotated;
            shRotation.Apply( projection, rotated );
            TSh7 expected;
            expected.AddDirection( rotation.TransformDirection( d ), 1.0, SHNORM_N3D );
            for ( UInt32 k=0; k<TSh7::CoefficientCount; ++k )
            {
                const Float64 error = fabs( rotated.coefficients[ k ] - expected.coefficients[ k ] );
                worstProjection = error > worstProjection ? error : worstProjection;
            }
            // arbitrary coefficients keep their band energy
            TSh7 random;
            for ( UInt32 k=0; k<TSh7::CoefficientCount; ++k )
            {
                const Float64 value = stream.Range< Float64 >( -1.0, 1.0 );
                random.coefficients[ k ] = value;
            }
            TSh7 randomRotated;
            shRotation.Apply( random, randomRotated );
            for ( UInt32 l=0; l<=7; ++l )
            {
                const Float64 error = fabs( randomRotated.GetBandEnergy( l ) - random.GetBandEnergy( l ) );
                worstEnergy = error > worstEnergy ? error : worstEnergy;
            }
        }
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "SphericalHarmonics: order 7 rotation in units of 1e-15: |R R^T - I| " << worstOrthogonality * 1e15 << ", projection " << worstProjection * 1e15 << ", energy " << worstEnergy * 1e15 );
        ASSERT_TRUE( worstOrthogonality <= 1e-10 && worstProjection <= 1e-10 && worstEnergy <= 1e-10 );
        if ( !( worstOrthogonality <= 1e-10 && worstProjection <= 1e-10 && worstEnergy <= 1e-10 ) )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SphericalHarmonics: order 7 rotation off, seed " << seed );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SphericalHarmonics: cosine-lobe convolution turns radiance into irradiance" )
    try
    {
        // a constant radiance of 1 gives the irradiance pi in every direction
        TFloat64SphericalHarmonics2 constant;
        constant.coefficients[ 0 ] = sqrt( 4.0 * g_pi );     // 1 = c00 Y00
        constant.ConvolveWithCosineLobe();
        ASSERT_TRUE( fabs( constant.Evaluate( TFloat64DirectionVector3D( TFloat64Vector3D( 1, 2, 3 ) ), SHNORM_ORTHONORMAL ) - g_pi ) < 1e-14 );
        // a delta of radiance in n: the irradiance at n is sum_l A_l ( 2l + 1 ) / 4 pi = ( 1 + 2 + 5/4 ) / 4 = 1.0625 at
        // order 2, and the clamped cosine max( 0, cos ) is approximated elsewhere
        TFloat64SphericalHarmonics2 delta;
        const TFloat64DirectionVector3D n( TFloat64Vector3D( 0.3, -0.4, 0.8 ) );
        delta.AddDirection( n, 1.0, SHNORM_ORTHONORMAL );
        delta.ConvolveWithCosineLobe();
        ASSERT_TRUE( fabs( delta.Evaluate( n, SHNORM_ORTHONORMAL ) - 1.0625 ) < 1e-14 );
        // the band factors: pi, 2 pi / 3, pi / 4, 0, -pi / 24, 0, pi / 64
        CTSphericalHarmonics< Float64, 6 > ones;
        for ( UInt32 k=0; k<49; ++k )
            ones.coefficients[ k ] = 1.0;
        ones.ConvolveWithCosineLobe();
        ASSERT_TRUE( fabs( ones.coefficients[ 0 ] - g_pi ) < 1e-15 && fabs( ones.coefficients[ 2 ] - 2.0 * g_pi / 3.0 ) < 1e-15 && fabs( ones.coefficients[ 6 ] - g_pi / 4.0 ) < 1e-15 );
        ASSERT_TRUE( 0.0 == ones.coefficients[ 12 ] && fabs( ones.coefficients[ 20 ] + g_pi / 24.0 ) < 1e-15 && 0.0 == ones.coefficients[ 30 ] && fabs( ones.coefficients[ 42 ] - g_pi / 64.0 ) < 1e-15 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SphericalHarmonics: text and DataNode forms round trip" )
    try
    {
        TFloat64SphericalHarmonics1 sh;
        sh.AddDirection( TFloat64DirectionVector3D::UnitY(), 0.5, SHNORM_SN3D );
        ASSERT_TRUE( "(0.5, 0.5, 0, 0)" == sh.ToString() );
        TFloat64SphericalHarmonics1 parsed;
        ASSERT_TRUE( parsed.FromString( sh.ToString() ) && parsed.Equals( sh, 0.0, 0.0 ) );
        ASSERT_FALSE( parsed.FromString( "(1, 2, 3)" ) );
        ASSERT_TRUE( parsed.Equals( sh, 0.0, 0.0 ) );
        CORE::CDataNodeSerializableSettings settings;
        CORE::CDataNode node( "value", GUCEF_DATATYPE_OBJECT );
        CTSphericalHarmonics< Float64, 3 > big;
        big.coefficients[ 12 ] = 3.25;
        ASSERT_TRUE( big.Serialize( node, settings ) && GUCEF_NULL != node.GetAttribute( "c12" ) );
        CTSphericalHarmonics< Float64, 3 > restored;
        ASSERT_TRUE( restored.Deserialize( node, settings ) && 3.25 == restored.coefficients[ 12 ] );
        CORE::CDataNode partial( "value", GUCEF_DATATYPE_OBJECT );
        partial.SetAttribute( "c0", CORE::CVariant( 1.0 ) );
        ASSERT_FALSE( restored.Deserialize( partial, settings ) );
        ASSERT_TRUE( 3.25 == restored.coefficients[ 12 ] );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SphericalHarmonics: evaluation and rotation run in a realtime context" )
    try
    {
        const TFloat32DirectionVector3D d( TFloat32Vector3D( 0.2f, -0.5f, 0.7f ) );
        Float32 result = RealtimeShWork( d, TFloat32Matrix3D::RotationY( TFloat32Radian( 0.4f ) ) );
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( result ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SphericalHarmonics: realtime-safe and right-sized operations do not allocate" )
    GUCEF_TESTFW_SKIP_IF( !MathAllocationCounter::IsAvailable(), "no allocation counter with a sanitizer that replaces the allocator (the RealtimeSanitizer build proves the same in a realtime context)" );
    try
    {
        // the counter counts: one probe allocation
        MathAllocationCounter::StartCounting();
        MathAllocationCounter::AllocateProbe();
        ASSERT_TRUE( 1 == MathAllocationCounter::StopCounting() );
        const TFloat32DirectionVector3D d( TFloat32Vector3D( 0.2f, -0.5f, 0.7f ) );
        const TFloat32Matrix3D rotation = TFloat32Matrix3D::RotationY( TFloat32Radian( 0.4f ) );
        MathAllocationCounter::StartCounting();
        Float32 result = RealtimeShWork( d, rotation );
        const UInt32 allocations = MathAllocationCounter::StopCounting();
        ASSERT_TRUE( 0 == allocations && CTNumericTraits< Float32 >::IsFinite( result ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SphericalHarmonics: orders 0 to 7 against an independent associated-Legendre reference" )
    try
    {
        const UInt64 seed = 0x4B92;
        MathReference::CSeededSequence generator( seed );
        Float64 worst = 0.0;
        #if defined( __cpp_lib_math_special_functions ) && ( __cplusplus >= 201703L )
        Float64 worstStd = 0.0;
        #endif
        for ( UInt32 i=0; i<500; ++i )
        {
            const Float64 vx = generator.NextFloat64( -1, 1 );
            const Float64 vy = generator.NextFloat64( -1, 1 );
            const Float64 vz = generator.NextFloat64( -1, 1 );
            TFloat64DirectionVector3D d;
            if ( !TFloat64DirectionVector3D::TryCreate( TFloat64Vector3D( vx, vy, vz ), d ) )
                continue;
            Float64 basis[ 64 ];
            CTSphericalHarmonics< Float64, 7 >::EvaluateBasis( d, SHNORM_SN3D, basis );
            for ( UInt32 l=0; l<=7; ++l )
            {
                for ( Int32 m=-(Int32) l; m<=(Int32) l; ++m )
                {
                    const Float64 error = fabs( basis[ l * l + l + m ] - ReferenceSn3d( l, m, d.GetX(), d.GetY(), d.GetZ() ) );
                    worst = error > worst ? error : worst;
                    #if defined( __cpp_lib_math_special_functions ) && ( __cplusplus >= 201703L )
                    // std::assoc_legendre has no Condon-Shortley phase either
                    const UInt32 am = (UInt32) ( m < 0 ? -m : m );
                    const Float64 azimuth = atan2( d.GetY(), d.GetX() );
                    const Float64 reference = sqrt( ( 0 == am ? 1.0 : 2.0 ) * Factorial( l - am ) / Factorial( l + am ) ) * std::assoc_legendre( l, am, d.GetZ() ) *
                                              ( m > 0 ? cos( am * azimuth ) : ( m < 0 ? sin( am * azimuth ) : 1.0 ) );
                    const Float64 errorStd = fabs( basis[ l * l + l + m ] - reference );
                    worstStd = errorStd > worstStd ? errorStd : worstStd;
                    #endif
                }
            }
        }
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "SphericalHarmonics: orders 0 to 7 against the polynomial reference, largest difference in units of 1e-15 " << worst * 1e15 );
        ASSERT_TRUE( worst <= 1e-12 );
        #if defined( __cpp_lib_math_special_functions ) && ( __cplusplus >= 201703L )
        ASSERT_TRUE( worstStd <= 1e-12 );
        #endif
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
