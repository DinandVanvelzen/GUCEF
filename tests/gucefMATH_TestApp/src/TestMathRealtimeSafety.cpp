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

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_MATH_TRANSFORMS_H
#include "gucefMATH_transforms.h"
#define GUCEF_MATH_TRANSFORMS_H
#endif /* GUCEF_MATH_TRANSFORMS_H ? */

#ifndef GUCEF_MATH_CRREALFFT_H
#include "gucefMATH_CRealFft.h"
#define GUCEF_MATH_CRREALFFT_H
#endif /* GUCEF_MATH_CRREALFFT_H ? */

#ifndef GUCEF_MATH_CTVBAP2D_H
#include "gucefMATH_CTVbap2D.h"
#define GUCEF_MATH_CTVBAP2D_H
#endif /* GUCEF_MATH_CTVBAP2D_H ? */

#ifndef GUCEF_MATH_CTVBAP3D_H
#include "gucefMATH_CTVbap3D.h"
#define GUCEF_MATH_CTVBAP3D_H
#endif /* GUCEF_MATH_CTVBAP3D_H ? */

#ifndef GUCEF_MATH_CTSPHERICALHARMONICSROTATION_H
#include "gucefMATH_CTSphericalHarmonicsRotation.h"
#define GUCEF_MATH_CTSPHERICALHARMONICSROTATION_H
#endif /* GUCEF_MATH_CTSPHERICALHARMONICSROTATION_H ? */

#ifndef GUCEF_MATH_CTORTHONORMALFRAME3D_H
#include "gucefMATH_CTOrthonormalFrame3D.h"
#define GUCEF_MATH_CTORTHONORMALFRAME3D_H
#endif /* GUCEF_MATH_CTORTHONORMALFRAME3D_H ? */

#ifndef GUCEF_MATH_CTDISTANCEATTENUATION_H
#include "gucefMATH_CTDistanceAttenuation.h"
#define GUCEF_MATH_CTDISTANCEATTENUATION_H
#endif /* GUCEF_MATH_CTDISTANCEATTENUATION_H ? */

#ifndef GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#include "gucefMATH_SpatialAudioFunctions.h"
#define GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#endif /* GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H ? */

#ifndef GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H
#include "gucefMATH_SpatialAudioVectorFunctions.h"
#define GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H
#endif /* GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CFLOAT16_H
#include "gucefMATH_CFloat16.h"
#define GUCEF_MATH_CFLOAT16_H
#endif /* GUCEF_MATH_CFLOAT16_H ? */

#ifndef GUCEF_MATH_CMATHGLOBAL_H
#include "gucefMATH_CMathGlobal.h"
#define GUCEF_MATH_CMATHGLOBAL_H
#endif /* GUCEF_MATH_CMATHGLOBAL_H ? */

#ifndef GUCEF_MATH_CMATHBATCHDISPATCHER_H
#include "gucefMATH_CMathBatchDispatcher.h"
#define GUCEF_MATH_CMATHBATCHDISPATCHER_H
#endif /* GUCEF_MATH_CMATHBATCHDISPATCHER_H ? */

#ifndef GUCEF_MATH_BATCHFUNCTIONS_H
#include "gucefMATH_BatchFunctions.h"
#define GUCEF_MATH_BATCHFUNCTIONS_H
#endif /* GUCEF_MATH_BATCHFUNCTIONS_H ? */

#include "TestMathAllocationCounter.h"
#include "TestMathRealtimeSafety.h"

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

namespace {

const UInt32 BatchCount = 37;           /**< not a multiple of any SIMD width: tails are exercised too */

/** The work buffers of the realtime calls, sized before the realtime context is entered */
struct SRealtimeBuffers
{
    CRealFft* fft;
    Float32 signal[ 64 ];
    Float32 spectrum[ 66 ];
    TFloat32Point3D points[ BatchCount ];
    TFloat32Vector3D vectors[ BatchCount ];
    TFloat32Vector4D vectors4[ BatchCount ];
    TFloat32Quaternion quaternions[ BatchCount ];
    TFloat32Matrix4D matrices[ BatchCount ];
    Float32 floats[ BatchCount ];
    CFloat16 halves[ BatchCount ];
    TFloat64Point3D points64[ BatchCount ];
    TFloat64Vector3D vectors64[ BatchCount ];
    TFloat64Matrix4D matrices64[ BatchCount ];
    TFixed32Point3D fixedPoints[ BatchCount ];
    TFixed32Matrix4D fixedMatrices[ BatchCount ];
    TFloat32Radian azimuths[ BatchCount ];
    Float32 gains[ BatchCount * 5 ];
    Float32 field2D[ 9 * 5 ];
    Float32 field3D[ 9 * 5 * 3 ];
    TFloat32Vbap2D panner;
    CPerlinNoise noise;
};

/*-------------------------------------------------------------------------*/

/** The value types (section 2.4 item 5): vectors, points, directions, matrices, quaternions, transforms */
Float32
RealtimeValueTypeWork( Float32 x ) GUCEF_MATH_REALTIME_CONTEXT
{
    const TFloat32Vector3D a( x, 2.0f, -1.0f );
    const TFloat32Vector3D b( 0.5f, -x, 3.0f );
    const TFloat32Vector3D sum = a + b - a * 0.5f;
    const TFloat32DirectionVector3D direction( sum );
    const TFloat32Vector4D v4 = TFloat32Vector4D( x, 1.0f, 2.0f, 3.0f ) + TFloat32Vector4D( 1.0f, 1.0f, 1.0f, 1.0f ) * 2.0f;
    const TFloat32Vector4D v4n = v4.Normalized().Min( v4 ).Max( -v4 );
    const TFloat32Point3D p( 1.0f, x, 3.0f );
    const TFloat32Quaternion q = TFloat32Quaternion::FromAxisAngle( TFloat32DirectionVector3D::Up(), TFloat32Radian( x ) );
    const TFloat32Quaternion r = TFloat32Quaternion::Multiply( q, q.Normalized() );
    const TFloat32Matrix4D m = TFloat32Matrix4D::FromQuaternion( r ) * TFloat32Matrix4D::Translation( b );
    TFloat32Matrix4D inverse;
    const bool invertible = m.TryInverse( inverse, 1e-6f );
    const TFloat32Transform3D t = TFloat32Transform3D::Combine( TFloat32Transform3D( b, q, TFloat32Vector3D( 1.0f, 1.0f, 1.0f ) ), TFloat32Transform3D( a, r, TFloat32Vector3D( 2.0f, 2.0f, 2.0f ) ) );
    return sum.Dot( b ) + direction.Dot( a ) + v4n.Dot( v4 ) + m.TransformPoint( p ).x + m.Transposed().Transform( v4 ).w +
           r.Rotate( a ).z + q.Dot( r ) + t.TransformPoint( p ).y + ( invertible ? inverse.GetDeterminant() : 0.0f ) +
           m.TransformNormal( direction ).Dot( a ) + p.GetDistance( TFloat32Point3D( 0.0f, 0.0f, 0.0f ) );
}

/*-------------------------------------------------------------------------*/

/** Spatial audio: VBAP gains, spherical harmonics, attenuation, the scalar functions, frames, the FFT */
Float32
RealtimeSpatialAudioWork( Float32 x, SRealtimeBuffers& buffers ) GUCEF_MATH_REALTIME_CONTEXT
{
    const TFloat32Radian azimuths2D[ 5 ] = { TFloat32Radian( -1.9199f ), TFloat32Radian( -0.5236f ), TFloat32Radian( 0.0f ), TFloat32Radian( 0.5236f ), TFloat32Radian( 1.9199f ) };
    TFloat32Vbap2D ring;
    Float32 gains2D[ 5 ];
    const bool prepared2D = ring.Prepare( azimuths2D, 5 );
    ring.ComputeGains( TFloat32Radian( x ), gains2D, 2.0f );
    const TFloat32Radian azimuths3D[ 6 ] = { TFloat32Radian( -0.5f ), TFloat32Radian( 0.5f ), TFloat32Radian( 2.0f ), TFloat32Radian( -2.0f ), TFloat32Radian( 0.0f ), TFloat32Radian( 3.0f ) };
    const TFloat32Radian elevations3D[ 6 ] = { TFloat32Radian( 0.1f ), TFloat32Radian( 0.1f ), TFloat32Radian( 0.3f ), TFloat32Radian( 0.3f ), TFloat32Radian( 0.8f ), TFloat32Radian( -0.4f ) };
    TFloat32Vbap3D dome;
    Float32 gains3D[ 6 ];
    const bool prepared3D = dome.Prepare( azimuths3D, elevations3D, 6 );
    dome.ComputeGains( TFloat32Radian( x ), TFloat32Radian( 0.2f ), gains3D, 2.0f );

    const TFloat32DirectionVector3D d( TFloat32Vector3D( x, 0.3f, -1.0f ) );
    TFloat32SphericalHarmonics3 sh;
    sh.AddDirection( d, 1.0f, SHNORM_SN3D );
    Float32 basis[ 16 ];
    TFloat32SphericalHarmonics3::EvaluateBasis( d, SHNORM_N3D, basis );
    TFloat32SphericalHarmonicsRotation3 shRotation;
    shRotation.Set( TFloat32Matrix3D::FromQuaternion( TFloat32Quaternion::FromAxisAngle( TFloat32DirectionVector3D::Up(), TFloat32Radian( x ) ) ) );
    shRotation.Apply( sh, sh );

    const CTDistanceAttenuation< Float32 > attenuation( DISTANCEMODEL_INVERSE_CLAMPED, 1.0f, 50.0f, 1.0f );
    Float32 left = 0.0f;
    Float32 right = 0.0f;
    ConstantPowerPan( 0.25f, 1.0f, left, right );
    const Float32 cone = ConeGain( 0.5f, TFloat32Radian( 1.0f ), TFloat32Radian( 2.0f ), 0.25f );
    const Float32 doppler = DopplerRatio( TFloat32Point3D( 0, 0, 0 ), TFloat32Vector3D( 1, 0, 0 ), TFloat32Point3D( -5, 0, 0 ), TFloat32Vector3D( 2, 0, 0 ), 343.3f, 1.0f, 0.5f, 2.0f );

    const TFloat32OrthonormalFrame3D frame = TFloat32OrthonormalFrame3D::FromForwardUp( TFloat32Vector3D( 0.2f, 0.1f, -1.0f ), TFloat32Vector3D::Up() );
    const TFloat32Vector3D local = frame.ToFrameCoordinates( TFloat32Vector3D( x, 1.0f, -3.0f ) );

    const bool forward = buffers.fft->Forward( buffers.signal, buffers.spectrum );
    const bool inverse = buffers.fft->Inverse( buffers.spectrum, buffers.signal );

    const CFloat16 half = CFloat16::FromFloat32( x );
    return ( prepared2D && prepared3D && forward && inverse ? 1.0f : 0.0f ) + gains2D[ 1 ] + gains3D[ 2 ] + basis[ 5 ] + sh.Evaluate( d, SHNORM_SN3D ) +
           attenuation.Evaluate( x + 2.0f ) + DecibelsToGain( -6.0f ) + left + right + cone + doppler + local.z + half.ToFloat32() + buffers.signal[ 3 ];
}

/*-------------------------------------------------------------------------*/

/** The batch dispatcher with MATHBATCH_REALTIME: selection and every batch function (SCALAR and SIMD providers only) */
Float32
RealtimeBatchWork( Float32 x, SRealtimeBuffers& buffers ) GUCEF_MATH_REALTIME_CONTEXT
{
    const CMathBatchDispatcher& dispatcher = CMathGlobal::Instance()->GetBatchDispatcher();
    UInt32 selected = 0;
    for ( UInt32 k=0; k<MATHBATCHKERNEL_MATH_COUNT; ++k )
    {
        if ( GUCEF_NULL != dispatcher.SelectProvider( (TMathBatchKernel) k, BatchCount, MATHBATCH_REALTIME, buffers.points, buffers.points ) )
            ++selected;
    }
    const TFloat32Matrix4D m = TFloat32Matrix4D::Translation( TFloat32Vector3D( x, 1.0f, 2.0f ) ) * TFloat32Matrix4D::Scale( TFloat32Vector3D( 2.0f, 2.0f, 2.0f ) );
    const UInt32 flags = MATHBATCH_REALTIME;
    TransformPoints( m, buffers.points, buffers.points, BatchCount, flags );
    TransformPointsStrided( m, buffers.points, 12, buffers.points, 12, BatchCount, flags );
    TransformVectors( m, buffers.vectors, buffers.vectors, BatchCount, flags );
    TransformNormals( m, buffers.vectors, buffers.vectors, BatchCount, flags );
    TransformVectors4( m, buffers.vectors4, buffers.vectors4, BatchCount, flags );
    MultiplyMatrices( buffers.matrices, buffers.matrices, buffers.matrices, BatchCount, flags );
    QuaternionsToMatrices( buffers.quaternions, buffers.matrices, BatchCount, flags );
    DotProducts3( buffers.vectors, buffers.vectors, buffers.floats, BatchCount, flags );
    NormalizeVectors3( buffers.vectors, buffers.vectors, BatchCount, flags );
    ConvertFloat32ToFloat16( buffers.floats, buffers.halves, BatchCount, flags );
    ConvertFloat16ToFloat32( buffers.halves, buffers.floats, BatchCount, flags | MATHBATCH_DETERMINISTIC );
    // the Float64 and Q16.16 forms, the panning gains and the noise fields
    const TFloat64Matrix4D m64 = TFloat64Matrix4D::Translation( TFloat64Vector3D( (Float64) x, 1.0, 2.0 ) );
    TransformPoints( m64, buffers.points64, buffers.points64, BatchCount, flags );
    TransformVectors( m64, buffers.vectors64, buffers.vectors64, BatchCount, flags );
    MultiplyMatrices( buffers.matrices64, buffers.matrices64, buffers.matrices64, BatchCount, flags );
    TransformPoints( buffers.fixedMatrices[ 1 ], buffers.fixedPoints, buffers.fixedPoints, BatchCount, flags );
    MultiplyMatrices( buffers.fixedMatrices, buffers.fixedMatrices, buffers.fixedMatrices, BatchCount, flags );
    ComputeVbapGains2D( buffers.panner, buffers.azimuths, buffers.gains, BatchCount, 2.0f, flags );
    FillNoise2D( buffers.noise, x, 0.5f, 0.3f, 0.2f, 9, 5, buffers.field2D, flags );
    FillNoise3D( buffers.noise, x, 0.5f, 1.5f, 0.3f, 0.2f, 0.1f, 9, 5, 3, buffers.field3D, flags );
    return ( buffers.points64[ 2 ].x < 0.0 ? 1.0f : 0.0f ) + buffers.gains[ 3 ] + buffers.field2D[ 4 ] + buffers.field3D[ 7 ] + (Float32) buffers.fixedPoints[ 1 ].x.GetRaw() + (Float32) selected + buffers.points[ 3 ].x + buffers.vectors[ 5 ].y + buffers.vectors4[ 7 ].w + buffers.matrices[ 9 ]( 1, 1 ) + buffers.floats[ 11 ];
}

/*-------------------------------------------------------------------------*/

void
InitializeBuffers( SRealtimeBuffers& buffers, CRealFft& fft )
{
    buffers.fft = &fft;
    for ( UInt32 i=0; i<64; ++i )
        buffers.signal[ i ] = 0.01f * (Float32) i;
    for ( UInt32 i=0; i<BatchCount; ++i )
    {
        const Float32 f = (Float32) i;
        buffers.points[ i ] = TFloat32Point3D( f, 1.0f - f, 0.5f * f );
        buffers.vectors[ i ] = TFloat32Vector3D( 1.0f + f, -f, 2.0f );
        buffers.vectors4[ i ] = TFloat32Vector4D( f, 1.0f, -1.0f, 0.25f * f );
        buffers.quaternions[ i ] = TFloat32Quaternion::FromAxisAngle( TFloat32DirectionVector3D::Up(), TFloat32Radian( 0.1f * f ) );
        buffers.matrices[ i ] = TFloat32Matrix4D::Translation( TFloat32Vector3D( f, f, f ) );
        buffers.floats[ i ] = 0.5f * f;
        buffers.points64[ i ] = TFloat64Point3D( (Float64) f, 1.0 - f, 0.5 * f );
        buffers.vectors64[ i ] = TFloat64Vector3D( 1.0 + f, -f, 2.0 );
        buffers.matrices64[ i ] = TFloat64Matrix4D::Translation( TFloat64Vector3D( f, f, f ) );
        buffers.fixedPoints[ i ] = TFixed32Point3D( TFixed32::FromRaw( (Int32) ( i * 4096 ) ), TFixed32::FromRaw( -(Int32) ( i * 2048 ) ), TFixed32::FromRaw( 65536 ) );
        buffers.fixedMatrices[ i ] = TFixed32Matrix4D::Translation( TFixed32Vector3D( TFixed32::FromRaw( (Int32) ( i * 256 ) ), TFixed32::FromRaw( 256 ), TFixed32::FromRaw( 0 ) ) );
        buffers.azimuths[ i ] = TFloat32Radian( 0.37f * f );
    }
    const TFloat32Radian speakers[ 5 ] = { TFloat32Radian( -1.9199f ), TFloat32Radian( -0.5236f ), TFloat32Radian( 0.0f ), TFloat32Radian( 0.5236f ), TFloat32Radian( 1.9199f ) };
    buffers.panner.Prepare( speakers, 5 );
    buffers.noise.Seed( 17 );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMathRealtimeSafetyTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "MathRealtimeSafety" );

    // created outside the realtime context: the global (and its dispatcher) and the FFT tables allocate
    CMathGlobal::Instance();
    CRealFft fft;
    const bool fftPrepared = fft.Prepare( 64, true, false );

    GUCEF_TESTFW_TESTCASE( "MathRealtimeSafety: the realtime-safe functions run in a realtime context" )
    try
    {
        ASSERT_TRUE( fftPrepared );
        SRealtimeBuffers buffers;
        InitializeBuffers( buffers, fft );
        const Float32 values = RealtimeValueTypeWork( 0.75f );
        const Float32 audio = RealtimeSpatialAudioWork( 0.75f, buffers );
        const Float32 batch = RealtimeBatchWork( 0.75f, buffers );
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( values ) );
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( audio ) );
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( batch ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MathRealtimeSafety: the realtime-safe functions do not allocate" )
    GUCEF_TESTFW_SKIP_IF( !MathAllocationCounter::IsAvailable(), "no allocation counter with a sanitizer that replaces the allocator (the RealtimeSanitizer build proves the same in a realtime context)" );
    try
    {
        SRealtimeBuffers buffers;
        InitializeBuffers( buffers, fft );
        MathAllocationCounter::StartCounting();
        MathAllocationCounter::AllocateProbe();
        ASSERT_TRUE( 1 == MathAllocationCounter::StopCounting() );
        MathAllocationCounter::StartCounting();
        const Float32 values = RealtimeValueTypeWork( 0.5f );
        const Float32 audio = RealtimeSpatialAudioWork( 0.5f, buffers );
        const Float32 batch = RealtimeBatchWork( 0.5f, buffers );
        const UInt32 allocations = MathAllocationCounter::StopCounting();
        ASSERT_TRUE( 0 == allocations );
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( values + audio + batch ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
