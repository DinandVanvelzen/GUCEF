/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#ifndef GUCEF_MATH_CMATHGLOBAL_H
#include "gucefMATH_CMathGlobal.h"
#define GUCEF_MATH_CMATHGLOBAL_H
#endif /* GUCEF_MATH_CMATHGLOBAL_H ? */

#ifndef GUCEF_MATH_CMATHBATCHDISPATCHER_H
#include "gucefMATH_CMathBatchDispatcher.h"
#define GUCEF_MATH_CMATHBATCHDISPATCHER_H
#endif /* GUCEF_MATH_CMATHBATCHDISPATCHER_H ? */

#include "gucefMATH_BatchFunctions.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

// the kernels read the value types as their plain components
GUCEF_MATH_STATIC_ASSERT( sizeof( TFloat32Point3D ) == 3 * sizeof( Float32 ), Batch_kernels_require_a_12_byte_point );
GUCEF_MATH_STATIC_ASSERT( sizeof( TFloat32Vector3D ) == 3 * sizeof( Float32 ), Batch_kernels_require_a_12_byte_vector );
GUCEF_MATH_STATIC_ASSERT( sizeof( TFloat32Vector4D ) == 4 * sizeof( Float32 ), Batch_kernels_require_a_16_byte_vector );
GUCEF_MATH_STATIC_ASSERT( sizeof( TFloat32Quaternion ) == 4 * sizeof( Float32 ), Batch_kernels_require_a_16_byte_quaternion );
GUCEF_MATH_STATIC_ASSERT( sizeof( TFloat32Matrix4D ) == 16 * sizeof( Float32 ), Batch_kernels_require_a_64_byte_matrix );
GUCEF_MATH_STATIC_ASSERT( sizeof( CFloat16 ) == sizeof( UInt16 ), Batch_kernels_require_a_2_byte_half );
GUCEF_MATH_STATIC_ASSERT( sizeof( TFloat64Point3D ) == 3 * sizeof( Float64 ), Batch_kernels_require_a_24_byte_point );
GUCEF_MATH_STATIC_ASSERT( sizeof( TFloat64Vector3D ) == 3 * sizeof( Float64 ), Batch_kernels_require_a_24_byte_vector );
GUCEF_MATH_STATIC_ASSERT( sizeof( TFloat64Matrix4D ) == 16 * sizeof( Float64 ), Batch_kernels_require_a_128_byte_matrix );
GUCEF_MATH_STATIC_ASSERT( sizeof( TFixed32Point3D ) == 3 * sizeof( Int32 ), Batch_kernels_require_a_12_byte_fixed_point );
GUCEF_MATH_STATIC_ASSERT( sizeof( TFixed32Matrix4D ) == 16 * sizeof( Int32 ), Batch_kernels_require_a_64_byte_fixed_matrix );
GUCEF_MATH_STATIC_ASSERT( sizeof( TFloat32Radian ) == sizeof( Float32 ), Batch_kernels_require_a_4_byte_radian );

/** The most samples of one noise grid: the item numbers of the kernels are 32 bit, a larger grid is not filled */
const UInt64 MaxNoiseItemsPerCall = 0xFFFFFFFFULL;

GUCEF_MATH_FORCE_INLINE const CMathBatchDispatcher&
Dispatcher( void )
{
    return CMathGlobal::Instance()->GetBatchDispatcher();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
TransformPoints( const TFloat32Matrix4D& m, const TFloat32Point3D* in, TFloat32Point3D* out, UInt32 count, UInt32 flags )
{
    TransformPointsStrided( m, in, (UInt32) sizeof( TFloat32Point3D ), out, (UInt32) sizeof( TFloat32Point3D ), count, flags );
}

/*-------------------------------------------------------------------------*/

bool
TransformPointsStrided( const TFloat32Matrix4D& m, const void* in, UInt32 inStrideBytes, void* out, UInt32 outStrideBytes, UInt32 count, UInt32 flags )
{
    if ( 0 == count )
        return true;
    if ( !MathBatchStrideValid( count, inStrideBytes, 12 ) || !MathBatchStrideValid( count, outStrideBytes, 12 ) )
    {
        GUCEF_MATH_ASSERT( false );     // strides smaller than a point would make the SIMD providers read past the elements
        return false;
    }
    SMathTransformPoints3F32Args args;
    args.matrix = &m;
    args.in = in;
    args.inStrideBytes = inStrideBytes;
    args.out = out;
    args.outStrideBytes = outStrideBytes;
    args.count = count;
    return MATHSTATUS_OK == Dispatcher().Execute( MATHBATCHKERNEL_TRANSFORM_POINTS3_F32, args, count, flags, in, out );
}

/*-------------------------------------------------------------------------*/

void
TransformVectors( const TFloat32Matrix4D& m, const TFloat32Vector3D* in, TFloat32Vector3D* out, UInt32 count, UInt32 flags )
{
    if ( 0 == count )
        return;
    SMathTransformVectors3F32Args args;
    args.matrix = &m;
    args.in = in;
    args.inStrideBytes = (UInt32) sizeof( TFloat32Vector3D );
    args.out = out;
    args.outStrideBytes = (UInt32) sizeof( TFloat32Vector3D );
    args.count = count;
    Dispatcher().Execute( MATHBATCHKERNEL_TRANSFORM_VECTORS3_F32, args, count, flags, in, out );
}

/*-------------------------------------------------------------------------*/

void
TransformNormals( const TFloat32Matrix4D& m, const TFloat32Vector3D* in, TFloat32Vector3D* out, UInt32 count, UInt32 flags )
{
    if ( 0 == count )
        return;
    SMathTransformNormals3F32Args args;
    args.matrix = &m;
    args.in = in;
    args.inStrideBytes = (UInt32) sizeof( TFloat32Vector3D );
    args.out = out;
    args.outStrideBytes = (UInt32) sizeof( TFloat32Vector3D );
    args.count = count;
    Dispatcher().Execute( MATHBATCHKERNEL_TRANSFORM_NORMALS3_F32, args, count, flags, in, out );
}

/*-------------------------------------------------------------------------*/

void
TransformVectors4( const TFloat32Matrix4D& m, const TFloat32Vector4D* in, TFloat32Vector4D* out, UInt32 count, UInt32 flags )
{
    if ( 0 == count )
        return;
    SMathTransformVectors4F32Args args;
    args.matrix = &m;
    args.in = in;
    args.inStrideBytes = (UInt32) sizeof( TFloat32Vector4D );
    args.out = out;
    args.outStrideBytes = (UInt32) sizeof( TFloat32Vector4D );
    args.count = count;
    Dispatcher().Execute( MATHBATCHKERNEL_TRANSFORM_VECTORS4_F32, args, count, flags, in, out );
}

/*-------------------------------------------------------------------------*/

void
MultiplyMatrices( const TFloat32Matrix4D* a, const TFloat32Matrix4D* b, TFloat32Matrix4D* out, UInt32 count, UInt32 flags )
{
    if ( 0 == count )
        return;
    SMathMultiplyMatrices4F32Args args;
    args.a = a;
    args.aStrideBytes = (UInt32) sizeof( TFloat32Matrix4D );
    args.b = b;
    args.bStrideBytes = (UInt32) sizeof( TFloat32Matrix4D );
    args.out = out;
    args.outStrideBytes = (UInt32) sizeof( TFloat32Matrix4D );
    args.count = count;
    Dispatcher().Execute( MATHBATCHKERNEL_MULTIPLY_MATRICES4_F32, args, count, flags, a, out );
}

/*-------------------------------------------------------------------------*/

void
DotProducts3( const TFloat32Vector3D* a, const TFloat32Vector3D* b, Float32* out, UInt32 count, UInt32 flags )
{
    if ( 0 == count )
        return;
    SMathDotProducts3F32Args args;
    args.a = a;
    args.aStrideBytes = (UInt32) sizeof( TFloat32Vector3D );
    args.b = b;
    args.bStrideBytes = (UInt32) sizeof( TFloat32Vector3D );
    args.out = out;
    args.outStrideBytes = (UInt32) sizeof( Float32 );
    args.count = count;
    Dispatcher().Execute( MATHBATCHKERNEL_DOT_PRODUCTS3_F32, args, count, flags, a, out );
}

/*-------------------------------------------------------------------------*/

void
NormalizeVectors3( const TFloat32Vector3D* in, TFloat32Vector3D* out, UInt32 count, UInt32 flags )
{
    if ( 0 == count )
        return;
    SMathNormalizeVectors3F32Args args;
    args.in = in;
    args.inStrideBytes = (UInt32) sizeof( TFloat32Vector3D );
    args.out = out;
    args.outStrideBytes = (UInt32) sizeof( TFloat32Vector3D );
    args.count = count;
    Dispatcher().Execute( MATHBATCHKERNEL_NORMALIZE_VECTORS3_F32, args, count, flags, in, out );
}

/*-------------------------------------------------------------------------*/

void
QuaternionsToMatrices( const TFloat32Quaternion* in, TFloat32Matrix4D* out, UInt32 count, UInt32 flags )
{
    if ( 0 == count )
        return;
    SMathQuaternionsToMatricesF32Args args;
    args.in = in;
    args.inStrideBytes = (UInt32) sizeof( TFloat32Quaternion );
    args.out = out;
    args.outStrideBytes = (UInt32) sizeof( TFloat32Matrix4D );
    args.count = count;
    Dispatcher().Execute( MATHBATCHKERNEL_QUATERNIONS_TO_MATRICES_F32, args, count, flags, in, out );
}

/*-------------------------------------------------------------------------*/

void
ConvertFloat32ToFloat16( const Float32* in, CFloat16* out, UInt32 count, UInt32 flags )
{
    if ( 0 == count )
        return;
    SMathConvertF32ToF16Args args;
    args.in = in;
    args.out = out;
    args.count = count;
    Dispatcher().Execute( MATHBATCHKERNEL_CONVERT_F32_TO_F16, args, count, flags, in, out );
}

/*-------------------------------------------------------------------------*/

void
ConvertFloat16ToFloat32( const CFloat16* in, Float32* out, UInt32 count, UInt32 flags )
{
    if ( 0 == count )
        return;
    SMathConvertF16ToF32Args args;
    args.in = in;
    args.out = out;
    args.count = count;
    Dispatcher().Execute( MATHBATCHKERNEL_CONVERT_F16_TO_F32, args, count, flags, in, out );
}

/*-------------------------------------------------------------------------*/

void
TransformPoints( const TFloat64Matrix4D& m, const TFloat64Point3D* in, TFloat64Point3D* out, UInt32 count, UInt32 flags )
{
    if ( 0 == count )
        return;
    SMathTransformPoints3F64Args args;
    args.matrix = &m;
    args.in = in;
    args.inStrideBytes = (UInt32) sizeof( TFloat64Point3D );
    args.out = out;
    args.outStrideBytes = (UInt32) sizeof( TFloat64Point3D );
    args.count = count;
    Dispatcher().Execute( MATHBATCHKERNEL_TRANSFORM_POINTS3_F64, args, count, flags, in, out );
}

/*-------------------------------------------------------------------------*/

void
TransformVectors( const TFloat64Matrix4D& m, const TFloat64Vector3D* in, TFloat64Vector3D* out, UInt32 count, UInt32 flags )
{
    if ( 0 == count )
        return;
    SMathTransformVectors3F64Args args;
    args.matrix = &m;
    args.in = in;
    args.inStrideBytes = (UInt32) sizeof( TFloat64Vector3D );
    args.out = out;
    args.outStrideBytes = (UInt32) sizeof( TFloat64Vector3D );
    args.count = count;
    Dispatcher().Execute( MATHBATCHKERNEL_TRANSFORM_VECTORS3_F64, args, count, flags, in, out );
}

/*-------------------------------------------------------------------------*/

void
MultiplyMatrices( const TFloat64Matrix4D* a, const TFloat64Matrix4D* b, TFloat64Matrix4D* out, UInt32 count, UInt32 flags )
{
    if ( 0 == count )
        return;
    SMathMultiplyMatrices4F64Args args;
    args.a = a;
    args.aStrideBytes = (UInt32) sizeof( TFloat64Matrix4D );
    args.b = b;
    args.bStrideBytes = (UInt32) sizeof( TFloat64Matrix4D );
    args.out = out;
    args.outStrideBytes = (UInt32) sizeof( TFloat64Matrix4D );
    args.count = count;
    Dispatcher().Execute( MATHBATCHKERNEL_MULTIPLY_MATRICES4_F64, args, count, flags, a, out );
}

/*-------------------------------------------------------------------------*/

void
TransformPoints( const TFixed32Matrix4D& m, const TFixed32Point3D* in, TFixed32Point3D* out, UInt32 count, UInt32 flags )
{
    if ( 0 == count )
        return;
    SMathTransformPoints3Fx32Args args;
    args.matrix = &m;
    args.in = in;
    args.inStrideBytes = (UInt32) sizeof( TFixed32Point3D );
    args.out = out;
    args.outStrideBytes = (UInt32) sizeof( TFixed32Point3D );
    args.count = count;
    Dispatcher().Execute( MATHBATCHKERNEL_TRANSFORM_POINTS3_FX32, args, count, flags, in, out );
}

/*-------------------------------------------------------------------------*/

void
MultiplyMatrices( const TFixed32Matrix4D* a, const TFixed32Matrix4D* b, TFixed32Matrix4D* out, UInt32 count, UInt32 flags )
{
    if ( 0 == count )
        return;
    SMathMultiplyMatrices4Fx32Args args;
    args.a = a;
    args.aStrideBytes = (UInt32) sizeof( TFixed32Matrix4D );
    args.b = b;
    args.bStrideBytes = (UInt32) sizeof( TFixed32Matrix4D );
    args.out = out;
    args.outStrideBytes = (UInt32) sizeof( TFixed32Matrix4D );
    args.count = count;
    Dispatcher().Execute( MATHBATCHKERNEL_MULTIPLY_MATRICES4_FX32, args, count, flags, a, out );
}

/*-------------------------------------------------------------------------*/

void
ComputeVbapGains2D( const TFloat32Vbap2D& vbap, const TFloat32Radian* azimuths, Float32* gains, UInt32 sourceCount, Float32 targetPower, UInt32 flags )
{
    if ( 0 == sourceCount || 0 == vbap.GetSpeakerCount() )
        return;
    SMathVbapGains2DF32Args args;
    args.vbap = &vbap;
    args.azimuths = azimuths;
    args.azimuthStrideBytes = (UInt32) sizeof( TFloat32Radian );
    args.targetPower = targetPower;
    args.gains = gains;
    args.gainsStrideBytes = vbap.GetSpeakerCount() * (UInt32) sizeof( Float32 );
    args.count = sourceCount;
    Dispatcher().Execute( MATHBATCHKERNEL_VBAP_GAINS2D_F32, args, sourceCount, flags, azimuths, gains );
}

/*-------------------------------------------------------------------------*/

namespace {

void
FillNoise2DImpl( const void* noise, UInt32 type, Float32 originX, Float32 originY, Float32 stepX, Float32 stepY, UInt32 width, UInt32 height, Float32* out, UInt32 flags )
{
    const UInt64 total = (UInt64) width * height;
    if ( 0 == total || total > MaxNoiseItemsPerCall )
        return;
    SMathNoiseField2DF32Args args;
    args.noise = noise;
    args.noiseType = type;
    args.originX = originX;
    args.originY = originY;
    args.stepX = stepX;
    args.stepY = stepY;
    args.width = width;
    args.height = height;
    args.baseItem = 0;
    args.out = out;
    args.count = (UInt32) total;
    Dispatcher().Execute( MATHBATCHKERNEL_NOISE_FIELD2D_F32, args, args.count, flags, GUCEF_NULL, out );
}

void
FillNoise3DImpl( const void* noise, UInt32 type, Float32 originX, Float32 originY, Float32 originZ, Float32 stepX, Float32 stepY, Float32 stepZ,
                 UInt32 width, UInt32 height, UInt32 depth, Float32* out, UInt32 flags )
{
    const UInt64 total = (UInt64) width * height * depth;
    if ( 0 == total || total > MaxNoiseItemsPerCall )
        return;
    SMathNoiseField3DF32Args args;
    args.noise = noise;
    args.noiseType = type;
    args.originX = originX;
    args.originY = originY;
    args.originZ = originZ;
    args.stepX = stepX;
    args.stepY = stepY;
    args.stepZ = stepZ;
    args.width = width;
    args.height = height;
    args.baseItem = 0;
    args.out = out;
    args.count = (UInt32) total;
    Dispatcher().Execute( MATHBATCHKERNEL_NOISE_FIELD3D_F32, args, args.count, flags, GUCEF_NULL, out );
}

}

/*-------------------------------------------------------------------------*/

void
FillNoise2D( const CPerlinNoise& noise, Float32 originX, Float32 originY, Float32 stepX, Float32 stepY, UInt32 width, UInt32 height, Float32* out, UInt32 flags )
{
    FillNoise2DImpl( &noise, MATHNOISE_PERLIN, originX, originY, stepX, stepY, width, height, out, flags );
}

void
FillNoise2D( const CValueNoise& noise, Float32 originX, Float32 originY, Float32 stepX, Float32 stepY, UInt32 width, UInt32 height, Float32* out, UInt32 flags )
{
    FillNoise2DImpl( &noise, MATHNOISE_VALUE, originX, originY, stepX, stepY, width, height, out, flags );
}

void
FillNoise2D( const COpenSimplex2Noise& noise, Float32 originX, Float32 originY, Float32 stepX, Float32 stepY, UInt32 width, UInt32 height, Float32* out, UInt32 flags )
{
    FillNoise2DImpl( &noise, MATHNOISE_OPENSIMPLEX2, originX, originY, stepX, stepY, width, height, out, flags );
}

void
FillNoise2D( const CWorleyNoise& noise, Float32 originX, Float32 originY, Float32 stepX, Float32 stepY, UInt32 width, UInt32 height, Float32* out, UInt32 flags )
{
    FillNoise2DImpl( &noise, MATHNOISE_WORLEY, originX, originY, stepX, stepY, width, height, out, flags );
}

/*-------------------------------------------------------------------------*/

void
FillNoise3D( const CPerlinNoise& noise, Float32 originX, Float32 originY, Float32 originZ, Float32 stepX, Float32 stepY, Float32 stepZ, UInt32 width, UInt32 height, UInt32 depth, Float32* out, UInt32 flags )
{
    FillNoise3DImpl( &noise, MATHNOISE_PERLIN, originX, originY, originZ, stepX, stepY, stepZ, width, height, depth, out, flags );
}

void
FillNoise3D( const CValueNoise& noise, Float32 originX, Float32 originY, Float32 originZ, Float32 stepX, Float32 stepY, Float32 stepZ, UInt32 width, UInt32 height, UInt32 depth, Float32* out, UInt32 flags )
{
    FillNoise3DImpl( &noise, MATHNOISE_VALUE, originX, originY, originZ, stepX, stepY, stepZ, width, height, depth, out, flags );
}

void
FillNoise3D( const COpenSimplex2Noise& noise, Float32 originX, Float32 originY, Float32 originZ, Float32 stepX, Float32 stepY, Float32 stepZ, UInt32 width, UInt32 height, UInt32 depth, Float32* out, UInt32 flags )
{
    FillNoise3DImpl( &noise, MATHNOISE_OPENSIMPLEX2, originX, originY, originZ, stepX, stepY, stepZ, width, height, depth, out, flags );
}

void
FillNoise3D( const CWorleyNoise& noise, Float32 originX, Float32 originY, Float32 originZ, Float32 stepX, Float32 stepY, Float32 stepZ, UInt32 width, UInt32 height, UInt32 depth, Float32* out, UInt32 flags )
{
    FillNoise3DImpl( &noise, MATHNOISE_WORLEY, originX, originY, originZ, stepX, stepY, stepZ, width, height, depth, out, flags );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
