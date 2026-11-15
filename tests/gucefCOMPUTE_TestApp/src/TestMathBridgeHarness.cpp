/*
 *  gucefCOMPUTE_TestApp: GUCEF test application for the Compute module
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_MATH_CMATHGLOBAL_H
#include "gucefMATH_CMathGlobal.h"
#define GUCEF_MATH_CMATHGLOBAL_H
#endif /* GUCEF_MATH_CMATHGLOBAL_H ? */

#ifndef GUCEF_MATH_CPERLINNOISE_H
#include "gucefMATH_CPerlinNoise.h"
#define GUCEF_MATH_CPERLINNOISE_H
#endif /* GUCEF_MATH_CPERLINNOISE_H ? */

#include "../../../plugins/COMPUTE/computebridgeMATH/include/computebridgeMATH_MathKernelTraits.h"

#include "TestMathBridgeHarness.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {
namespace TEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* BridgePluginGroup = "computebridge";

/*-------------------------------------------------------------------------*/

/**
 *  One float with a few special values among the ordinary ones: zeros, denormals, small and large magnitudes. With
 *  'allowNonFinite' also infinities and NaN. Operations that produce a NaN from several operands (infinity minus
 *  infinity, a NaN among the operands) give a NaN whose sign and payload depend on the order of the operands in the
 *  instructions, so the scalar and the SIMD providers of gucefMATH are not bit-identical for such inputs: the arrays of
 *  the arithmetic kernels stay finite, and the products of up to four of the large values stay finite too
 */
Float32
TestFloat( CTestRandom& random, Float32 scale, bool allowNonFinite )
{
    if ( 0 != random.Next() % 61 )
        return random.NextFloat( scale );

    UInt32 pattern = random.Next() % 10;
    if ( !allowNonFinite && ( 2 == pattern || 3 == pattern || 4 == pattern ) )
        pattern = 9;
    UInt32 bits = 0;
    switch ( pattern )
    {
        case 0: bits = 0x00000000u; break;      // +0
        case 1: bits = 0x80000000u; break;      // -0
        case 2: bits = 0x7F800000u; break;      // +inf
        case 3: bits = 0xFF800000u; break;      // -inf
        case 4: bits = 0x7FC00000u; break;      // NaN
        case 5: bits = 0x00000400u; break;      // denormal
        case 6: bits = 0x58635FA9u; break;      // 1e15
        case 7: bits = 0x1A000000u; break;      // 3.4e-23
        case 8: bits = 0x4E6E6B28u; break;      // 1e9
        default: bits = 0x3F800000u; break;     // 1
    }
    Float32 value;
    memcpy( &value, &bits, sizeof( value ) );
    return value;
}

/*-------------------------------------------------------------------------*/

/** 'count' elements of 'components' floats every 'strideBytes' */
void
FillFloats( void* data, UInt32 count, UInt32 strideBytes, UInt32 components, UInt32 seed )
{
    CTestRandom random( seed );
    UInt8* bytes = static_cast< UInt8* >( data );
    for ( UInt32 i=0; i<count; ++i )
    {
        Float32 element[ 16 ];
        for ( UInt32 c=0; c<components; ++c )
            element[ c ] = TestFloat( random, 50.0f, false );
        memcpy( bytes + static_cast< size_t >( i ) * strideBytes, element, components * sizeof( Float32 ) );
    }
}

/*-------------------------------------------------------------------------*/

void
FillVector3( UInt32, void* data, UInt32 count, UInt32 strideBytes, UInt32 seed ) { FillFloats( data, count, strideBytes, 3, seed ); }

void
FillVector4( UInt32, void* data, UInt32 count, UInt32 strideBytes, UInt32 seed ) { FillFloats( data, count, strideBytes, 4, seed ); }

void
FillMatrix4( UInt32, void* data, UInt32 count, UInt32 strideBytes, UInt32 seed ) { FillFloats( data, count, strideBytes, 16, seed ); }

/** Floats and 25 % random bit patterns, which are every kind of float, NaNs with payloads included */
void
FillFloat32Mix( UInt32, void* data, UInt32 count, UInt32, UInt32 seed )
{
    CTestRandom random( seed );
    UInt8* bytes = static_cast< UInt8* >( data );
    for ( UInt32 i=0; i<count; ++i )
    {
        Float32 value = TestFloat( random, 70000.0f, true );
        if ( 0 == random.Next() % 4 )
        {
            const UInt32 bits = random.Next();
            memcpy( &value, &bits, sizeof( value ) );
        }
        memcpy( bytes + static_cast< size_t >( i ) * sizeof( Float32 ), &value, sizeof( value ) );
    }
}

/** Every half: random bit patterns */
void
FillFloat16Bits( UInt32, void* data, UInt32 count, UInt32, UInt32 seed )
{
    CTestRandom random( seed );
    UInt8* bytes = static_cast< UInt8* >( data );
    for ( UInt32 i=0; i<count; ++i )
    {
        const UInt16 bits = static_cast< UInt16 >( random.Next() >> 7 );
        memcpy( bytes + static_cast< size_t >( i ) * sizeof( UInt16 ), &bits, sizeof( bits ) );
    }
}

/*-------------------------------------------------------------------------*/

/** Float64 components in the range of the Float32 data */
void
FillDoubles( void* data, UInt32 count, UInt32 strideBytes, UInt32 components, UInt32 seed )
{
    CTestRandom random( seed );
    UInt8* bytes = static_cast< UInt8* >( data );
    for ( UInt32 i=0; i<count; ++i )
    {
        Float64 element[ 16 ];
        for ( UInt32 c=0; c<components; ++c )
            element[ c ] = static_cast< Float64 >( TestFloat( random, 50.0f, false ) ) + static_cast< Float64 >( random.NextFloat( 1.0e-6f ) );
        memcpy( bytes + static_cast< size_t >( i ) * strideBytes, element, components * sizeof( Float64 ) );
    }
}

void FillVector3F64( UInt32, void* data, UInt32 count, UInt32 strideBytes, UInt32 seed ) { FillDoubles( data, count, strideBytes, 3, seed ); }
void FillMatrix4F64( UInt32, void* data, UInt32 count, UInt32 strideBytes, UInt32 seed ) { FillDoubles( data, count, strideBytes, 16, seed ); }

/** Raw Q16.16 values: any Int32, some at the ends of the range (the products saturate there, identically in every provider) */
void
FillFixed( void* data, UInt32 count, UInt32 strideBytes, UInt32 components, UInt32 seed )
{
    CTestRandom random( seed );
    UInt8* bytes = static_cast< UInt8* >( data );
    for ( UInt32 i=0; i<count; ++i )
    {
        Int32 element[ 16 ];
        for ( UInt32 c=0; c<components; ++c )
        {
            Int32 raw = static_cast< Int32 >( random.Next() ) >> 8;
            if ( 0 == random.Next() % 97 )
                raw = 0 == random.Next() % 2 ? 0x7FFFFFFF : static_cast< Int32 >( 0x80000000u );
            element[ c ] = raw;
        }
        memcpy( bytes + static_cast< size_t >( i ) * strideBytes, element, components * sizeof( Int32 ) );
    }
}

void FillVector3Fx32( UInt32, void* data, UInt32 count, UInt32 strideBytes, UInt32 seed ) { FillFixed( data, count, strideBytes, 3, seed ); }
void FillMatrix4Fx32( UInt32, void* data, UInt32 count, UInt32 strideBytes, UInt32 seed ) { FillFixed( data, count, strideBytes, 16, seed ); }

/** Azimuths in radians, a little beyond one turn either way */
void
FillAzimuths( UInt32, void* data, UInt32 count, UInt32 strideBytes, UInt32 seed )
{
    CTestRandom random( seed );
    UInt8* bytes = static_cast< UInt8* >( data );
    for ( UInt32 i=0; i<count; ++i )
    {
        const Float32 value = random.NextFloat( 7.0f );
        memcpy( bytes + static_cast< size_t >( i ) * strideBytes, &value, sizeof( value ) );
    }
}

void FillNothing( UInt32, void*, UInt32, UInt32, UInt32 ) {}

/*-------------------------------------------------------------------------*/

/** The panner and the noise the vector base amplitude panning and noise field cases use */
class CTestObjects
{
    public:

    CTestObjects( void ) : perlin( 1234 )
    {
        MATH::TFloat32Radian azimuths[ 5 ];
        for ( UInt32 i=0; i<5; ++i )
            azimuths[ i ] = MATH::TFloat32Radian( static_cast< Float32 >( i ) * 1.2566371f );
        vbap.Prepare( azimuths, 5 );
    }

    MATH::TFloat32Vbap2D vbap;
    MATH::CPerlinNoise perlin;
};

CTestObjects g_objects;

const Float32 VbapTargetPower = 1.5f;

MATH::TMathStatus
RunVbap( const MATH::CMathBatchDispatcher& dispatcher, UInt32 flags, const SMathCall& call )
{
    MATH::SMathVbapGains2DF32Args args;
    args.vbap = &g_objects.vbap;
    args.azimuths = call.in0;
    args.azimuthStrideBytes = call.in0Stride;
    args.targetPower = VbapTargetPower;
    args.gains = call.out;
    args.gainsStrideBytes = call.outStride;
    args.count = call.count;
    return dispatcher.Execute( MATH::MATHBATCHKERNEL_VBAP_GAINS2D_F32, args, call.count, flags, args.azimuths, args.gains );
}

/** A grid of 70 x 9 (x 3) lattice points of the noise, the first sample being item 5 of it */
MATH::TMathStatus
RunNoise2D( const MATH::CMathBatchDispatcher& dispatcher, UInt32 flags, const SMathCall& call )
{
    MATH::SMathNoiseField2DF32Args args;
    args.noise = &g_objects.perlin;
    args.noiseType = MATH::MATHNOISE_PERLIN;
    args.originX = -3.5f; args.originY = 2.25f; args.stepX = 0.173f; args.stepY = 0.31f;
    args.width = 70; args.height = 9;
    args.baseItem = 5;
    args.out = static_cast< Float32* >( call.out );
    args.count = call.count;
    return dispatcher.Execute( MATH::MATHBATCHKERNEL_NOISE_FIELD2D_F32, args, call.count, flags, args.out, args.out );
}

MATH::TMathStatus
RunNoise3D( const MATH::CMathBatchDispatcher& dispatcher, UInt32 flags, const SMathCall& call )
{
    MATH::SMathNoiseField3DF32Args args;
    args.noise = &g_objects.perlin;
    args.noiseType = MATH::MATHNOISE_PERLIN;
    args.originX = -3.5f; args.originY = 2.25f; args.originZ = 0.5f; args.stepX = 0.173f; args.stepY = 0.31f; args.stepZ = 0.07f;
    args.width = 70; args.height = 9;
    args.baseItem = 5;
    args.out = static_cast< Float32* >( call.out );
    args.count = call.count;
    return dispatcher.Execute( MATH::MATHBATCHKERNEL_NOISE_FIELD3D_F32, args, call.count, flags, args.out, args.out );
}

/*-------------------------------------------------------------------------*/

template < typename TArgs, MATH::TMathBatchKernel kernelId, typename TMatrix >
MATH::TMathStatus
RunMatrixMapOfKind( const MATH::CMathBatchDispatcher& dispatcher, UInt32 flags, const SMathCall& call )
{
    // the matrix of the call, converted to the kind of the kernel
    const TMatrix matrix( *call.matrix );
    TArgs args;
    args.matrix = &matrix;
    args.in = call.in0;
    args.inStrideBytes = call.in0Stride;
    args.out = call.out;
    args.outStrideBytes = call.outStride;
    args.count = call.count;
    return dispatcher.Execute( kernelId, args, call.count, flags, args.in, args.out );
}

/*-------------------------------------------------------------------------*/

/** The device calls: bindings in slots 0 and up, then the constants block, by the declaration of the kernel */

void
DeviceMatrixInConstants( const SMathCall& call, SMathDeviceCall& device )
{
    device.bindings[ 0 ] = CComputeHostBatch::SHostBinding::Input( 0, call.in0, static_cast< UInt64 >( call.count ) * call.in0Stride );
    device.bindings[ 1 ] = CComputeHostBatch::SHostBinding::Output( 1, call.out, static_cast< UInt64 >( call.count ) * call.outStride );
    device.bindingCount = 2;
    memcpy( reinterpret_cast< UInt8* >( device.constants ) + GUCEF_COMPUTE_CONSTANTS_HEADER_SIZE, call.matrix->GetData(), 16 * sizeof( Float32 ) );
    device.constantsSize = GUCEF_COMPUTE_CONSTANTS_HEADER_SIZE + 16 * sizeof( Float32 );
}

void
DeviceUnary( const SMathCall& call, SMathDeviceCall& device )
{
    device.bindings[ 0 ] = CComputeHostBatch::SHostBinding::Input( 0, call.in0, static_cast< UInt64 >( call.count ) * call.in0Stride );
    device.bindings[ 1 ] = CComputeHostBatch::SHostBinding::Output( 1, call.out, static_cast< UInt64 >( call.count ) * call.outStride );
    device.bindingCount = 2;
}

void
DeviceBinary( const SMathCall& call, SMathDeviceCall& device )
{
    device.bindings[ 0 ] = CComputeHostBatch::SHostBinding::Input( 0, call.in0, static_cast< UInt64 >( call.count ) * call.in0Stride );
    device.bindings[ 1 ] = CComputeHostBatch::SHostBinding::Input( 1, call.in1, static_cast< UInt64 >( call.count ) * call.in1Stride );
    device.bindings[ 2 ] = CComputeHostBatch::SHostBinding::Output( 2, call.out, static_cast< UInt64 >( call.count ) * call.outStride );
    device.bindingCount = 3;
}

void
DeviceMatrixFloat64( const SMathCall& call, SMathDeviceCall& device )
{
    const MATH::TFloat64Matrix4D matrix( *call.matrix );
    memcpy( device.matrixFloat64, matrix.GetData(), sizeof( device.matrixFloat64 ) );
    DeviceUnary( call, device );
    device.bindings[ 2 ] = CComputeHostBatch::SHostBinding::Input( 2, device.matrixFloat64, sizeof( device.matrixFloat64 ) );
    device.bindingCount = 3;
}

void
DeviceMatrixFixed( const SMathCall& call, SMathDeviceCall& device )
{
    const MATH::TFixed32Matrix4D matrix( *call.matrix );
    memcpy( device.matrixFixed, matrix.GetData(), sizeof( device.matrixFixed ) );
    DeviceUnary( call, device );
    device.bindings[ 2 ] = CComputeHostBatch::SHostBinding::Input( 2, device.matrixFixed, sizeof( device.matrixFixed ) );
    device.bindingCount = 3;
}

void
DeviceVbap( const SMathCall& call, SMathDeviceCall& device )
{
    DeviceUnary( call, device );
    COMPUTEBRIDGEMATH::SMathVbapGains2DF32Kernel::SConstants constants;
    constants.vbap = &g_objects.vbap;
    constants.targetPower = VbapTargetPower;
    constants.padding = 0;
    memcpy( reinterpret_cast< UInt8* >( device.constants ) + GUCEF_COMPUTE_CONSTANTS_HEADER_SIZE, &constants, sizeof( constants ) );
    device.constantsSize = GUCEF_COMPUTE_CONSTANTS_HEADER_SIZE + sizeof( constants );
}

void
DeviceNoise2D( const SMathCall& call, SMathDeviceCall& device )
{
    device.bindings[ 0 ] = CComputeHostBatch::SHostBinding::Output( 0, call.out, static_cast< UInt64 >( call.count ) * sizeof( Float32 ) );
    device.bindingCount = 1;
    COMPUTEBRIDGEMATH::SMathNoiseField2DF32Kernel::SConstants constants;
    memset( &constants, 0, sizeof( constants ) );
    constants.noise = &g_objects.perlin;
    constants.noiseType = MATH::MATHNOISE_PERLIN;
    constants.width = 70; constants.height = 9; constants.baseItem = 5;
    constants.origin[ 0 ] = -3.5f; constants.origin[ 1 ] = 2.25f;
    constants.step[ 0 ] = 0.173f; constants.step[ 1 ] = 0.31f;
    memcpy( reinterpret_cast< UInt8* >( device.constants ) + GUCEF_COMPUTE_CONSTANTS_HEADER_SIZE, &constants, sizeof( constants ) );
    device.constantsSize = GUCEF_COMPUTE_CONSTANTS_HEADER_SIZE + sizeof( constants );
}

void
DeviceNoise3D( const SMathCall& call, SMathDeviceCall& device )
{
    device.bindings[ 0 ] = CComputeHostBatch::SHostBinding::Output( 0, call.out, static_cast< UInt64 >( call.count ) * sizeof( Float32 ) );
    device.bindingCount = 1;
    COMPUTEBRIDGEMATH::SMathNoiseField3DF32Kernel::SConstants constants;
    memset( &constants, 0, sizeof( constants ) );
    constants.noise = &g_objects.perlin;
    constants.noiseType = MATH::MATHNOISE_PERLIN;
    constants.width = 70; constants.height = 9; constants.baseItem = 5;
    constants.origin[ 0 ] = -3.5f; constants.origin[ 1 ] = 2.25f; constants.origin[ 2 ] = 0.5f;
    constants.step[ 0 ] = 0.173f; constants.step[ 1 ] = 0.31f; constants.step[ 2 ] = 0.07f;
    memcpy( reinterpret_cast< UInt8* >( device.constants ) + GUCEF_COMPUTE_CONSTANTS_HEADER_SIZE, &constants, sizeof( constants ) );
    device.constantsSize = GUCEF_COMPUTE_CONSTANTS_HEADER_SIZE + sizeof( constants );
}

/*-------------------------------------------------------------------------*/

template < typename TArgs, MATH::TMathBatchKernel kernelId >
MATH::TMathStatus
RunMatrixMap( const MATH::CMathBatchDispatcher& dispatcher, UInt32 flags, const SMathCall& call )
{
    TArgs args;
    args.matrix = call.matrix;
    args.in = call.in0;
    args.inStrideBytes = call.in0Stride;
    args.out = call.out;
    args.outStrideBytes = call.outStride;
    args.count = call.count;
    return dispatcher.Execute( kernelId, args, call.count, flags, args.in, args.out );
}

template < typename TArgs, MATH::TMathBatchKernel kernelId >
MATH::TMathStatus
RunUnary( const MATH::CMathBatchDispatcher& dispatcher, UInt32 flags, const SMathCall& call )
{
    TArgs args;
    args.in = call.in0;
    args.inStrideBytes = call.in0Stride;
    args.out = call.out;
    args.outStrideBytes = call.outStride;
    args.count = call.count;
    return dispatcher.Execute( kernelId, args, call.count, flags, args.in, args.out );
}

template < typename TArgs, MATH::TMathBatchKernel kernelId >
MATH::TMathStatus
RunBinary( const MATH::CMathBatchDispatcher& dispatcher, UInt32 flags, const SMathCall& call )
{
    TArgs args;
    args.a = call.in0;
    args.aStrideBytes = call.in0Stride;
    args.b = call.in1;
    args.bStrideBytes = call.in1Stride;
    args.out = call.out;
    args.outStrideBytes = call.outStride;
    args.count = call.count;
    return dispatcher.Execute( kernelId, args, call.count, flags, args.a, args.out );
}

MATH::TMathStatus
RunF32ToF16( const MATH::CMathBatchDispatcher& dispatcher, UInt32 flags, const SMathCall& call )
{
    MATH::SMathConvertF32ToF16Args args;
    args.in = static_cast< const Float32* >( call.in0 );
    args.out = static_cast< MATH::CFloat16* >( call.out );
    args.count = call.count;
    return dispatcher.Execute( MATH::MATHBATCHKERNEL_CONVERT_F32_TO_F16, args, call.count, flags, args.in, args.out );
}

MATH::TMathStatus
RunF16ToF32( const MATH::CMathBatchDispatcher& dispatcher, UInt32 flags, const SMathCall& call )
{
    MATH::SMathConvertF16ToF32Args args;
    args.in = static_cast< const MATH::CFloat16* >( call.in0 );
    args.out = static_cast< Float32* >( call.out );
    args.count = call.count;
    return dispatcher.Execute( MATH::MATHBATCHKERNEL_CONVERT_F16_TO_F32, args, call.count, flags, args.in, args.out );
}

/*-------------------------------------------------------------------------*/

const SMathKernelCase g_cases[] =
{
    { MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_F32,      MATHCASESHAPE_MATRIX_MAP, 12, 0, 12, true,  true,  &RunMatrixMap< MATH::SMathTransformPoints3F32Args, MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_F32 >,        &FillVector3, &DeviceMatrixInConstants },
    { MATH::MATHBATCHKERNEL_TRANSFORM_VECTORS3_F32,     MATHCASESHAPE_MATRIX_MAP, 12, 0, 12, true,  true,  &RunMatrixMap< MATH::SMathTransformVectors3F32Args, MATH::MATHBATCHKERNEL_TRANSFORM_VECTORS3_F32 >,      &FillVector3, &DeviceMatrixInConstants },
    { MATH::MATHBATCHKERNEL_TRANSFORM_NORMALS3_F32,     MATHCASESHAPE_MATRIX_MAP, 12, 0, 12, true,  true,  &RunMatrixMap< MATH::SMathTransformNormals3F32Args, MATH::MATHBATCHKERNEL_TRANSFORM_NORMALS3_F32 >,      &FillVector3, &DeviceMatrixInConstants },
    { MATH::MATHBATCHKERNEL_TRANSFORM_VECTORS4_F32,     MATHCASESHAPE_MATRIX_MAP, 16, 0, 16, true,  true,  &RunMatrixMap< MATH::SMathTransformVectors4F32Args, MATH::MATHBATCHKERNEL_TRANSFORM_VECTORS4_F32 >,      &FillVector4, &DeviceMatrixInConstants },
    { MATH::MATHBATCHKERNEL_MULTIPLY_MATRICES4_F32,     MATHCASESHAPE_BINARY,     64, 64, 64, true, false, &RunBinary< MATH::SMathMultiplyMatrices4F32Args, MATH::MATHBATCHKERNEL_MULTIPLY_MATRICES4_F32 >,        &FillMatrix4, &DeviceBinary },
    { MATH::MATHBATCHKERNEL_QUATERNIONS_TO_MATRICES_F32, MATHCASESHAPE_UNARY,     16, 0, 64, true, false, &RunUnary< MATH::SMathQuaternionsToMatricesF32Args, MATH::MATHBATCHKERNEL_QUATERNIONS_TO_MATRICES_F32 >, &FillVector4, &DeviceUnary },
    { MATH::MATHBATCHKERNEL_DOT_PRODUCTS3_F32,          MATHCASESHAPE_BINARY,     12, 12, 4, true, false, &RunBinary< MATH::SMathDotProducts3F32Args, MATH::MATHBATCHKERNEL_DOT_PRODUCTS3_F32 >,                  &FillVector3, &DeviceBinary },
    { MATH::MATHBATCHKERNEL_NORMALIZE_VECTORS3_F32,     MATHCASESHAPE_UNARY,      12, 0, 12, true, false, &RunUnary< MATH::SMathNormalizeVectors3F32Args, MATH::MATHBATCHKERNEL_NORMALIZE_VECTORS3_F32 >,          &FillVector3, &DeviceUnary },
    { MATH::MATHBATCHKERNEL_CONVERT_F32_TO_F16,         MATHCASESHAPE_CONVERT,     4, 0,  2, false, false, &RunF32ToF16,                                                                                         &FillFloat32Mix, &DeviceUnary },
    { MATH::MATHBATCHKERNEL_CONVERT_F16_TO_F32,         MATHCASESHAPE_CONVERT,     2, 0,  4, false, false, &RunF16ToF32,                                                                                         &FillFloat16Bits, &DeviceUnary },
    { MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_F64,      MATHCASESHAPE_MATRIX_MAP, 24, 0, 24, true,  true,  &RunMatrixMapOfKind< MATH::SMathTransformPoints3F64Args, MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_F64, MATH::TFloat64Matrix4D >,    &FillVector3F64, &DeviceMatrixFloat64 },
    { MATH::MATHBATCHKERNEL_TRANSFORM_VECTORS3_F64,     MATHCASESHAPE_MATRIX_MAP, 24, 0, 24, true,  true,  &RunMatrixMapOfKind< MATH::SMathTransformVectors3F64Args, MATH::MATHBATCHKERNEL_TRANSFORM_VECTORS3_F64, MATH::TFloat64Matrix4D >,  &FillVector3F64, &DeviceMatrixFloat64 },
    { MATH::MATHBATCHKERNEL_MULTIPLY_MATRICES4_F64,     MATHCASESHAPE_BINARY,    128, 128, 128, true, false, &RunBinary< MATH::SMathMultiplyMatrices4F64Args, MATH::MATHBATCHKERNEL_MULTIPLY_MATRICES4_F64 >,      &FillMatrix4F64, &DeviceBinary },
    { MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_FX32,     MATHCASESHAPE_MATRIX_MAP, 12, 0, 12, true,  true,  &RunMatrixMapOfKind< MATH::SMathTransformPoints3Fx32Args, MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_FX32, MATH::TFixed32Matrix4D >,   &FillVector3Fx32, &DeviceMatrixFixed },
    { MATH::MATHBATCHKERNEL_MULTIPLY_MATRICES4_FX32,    MATHCASESHAPE_BINARY,     64, 64, 64, true, false, &RunBinary< MATH::SMathMultiplyMatrices4Fx32Args, MATH::MATHBATCHKERNEL_MULTIPLY_MATRICES4_FX32 >,    &FillMatrix4Fx32, &DeviceBinary },
    { MATH::MATHBATCHKERNEL_VBAP_GAINS2D_F32,           MATHCASESHAPE_UNARY,       4, 0, 20, true, false, &RunVbap,                                                                                              &FillAzimuths, &DeviceVbap },
    { MATH::MATHBATCHKERNEL_NOISE_FIELD2D_F32,          MATHCASESHAPE_GENERATE,    0, 0,  4, false, false, &RunNoise2D,                                                                                          &FillNothing, &DeviceNoise2D },
    { MATH::MATHBATCHKERNEL_NOISE_FIELD3D_F32,          MATHCASESHAPE_GENERATE,    0, 0,  4, false, false, &RunNoise3D,                                                                                          &FillNothing, &DeviceNoise3D }
};

const UInt32 g_caseCount = sizeof( g_cases ) / sizeof( g_cases[ 0 ] );

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

SMathDeviceCall::SMathDeviceCall( void )
    : bindingCount( 0 )
    , constantsSize( GUCEF_COMPUTE_CONSTANTS_HEADER_SIZE )
{
    memset( constants, 0, sizeof( constants ) );
    memset( matrixFloat64, 0, sizeof( matrixFloat64 ) );
    memset( matrixFixed, 0, sizeof( matrixFixed ) );
}

/*-------------------------------------------------------------------------*/

SMathCall::SMathCall( void )
    : in0( GUCEF_NULL )
    , in0Stride( 0 )
    , in1( GUCEF_NULL )
    , in1Stride( 0 )
    , out( GUCEF_NULL )
    , outStride( 0 )
    , count( 0 )
    , matrix( GUCEF_NULL )
{
}

/*-------------------------------------------------------------------------*/

const SMathKernelCase*
FindMathKernelCase( MATH::TMathBatchKernel kernel )
{
    for ( UInt32 i=0; i<g_caseCount; ++i )
        if ( g_cases[ i ].kernel == kernel )
            return &g_cases[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
GetMathMultiCoreKernels( MATH::TMathBatchKernel* kernels, UInt32 maxKernels )
{
    const MATH::CMathBatchDispatcher& dispatcher = MATH::CMathGlobal::Instance()->GetBatchDispatcher();
    UInt32 found = 0;
    for ( UInt32 k=0; k<(UInt32) MATH::MATHBATCHKERNEL_MATH_COUNT && found < maxKernels; ++k )
    {
        MATH::SMathBatchProvider provider;
        if ( FindMathProvider( (MATH::TMathBatchKernel) k, MATH::MATHBATCHTIER_MULTICORE, provider ) )
            kernels[ found++ ] = (MATH::TMathBatchKernel) k;
    }
    (void) dispatcher;
    return found;
}

/*-------------------------------------------------------------------------*/

bool
FindMathProvider( MATH::TMathBatchKernel kernel, MATH::TMathBatchTier tier, MATH::SMathBatchProvider& provider )
{
    const MATH::CMathBatchDispatcher& dispatcher = MATH::CMathGlobal::Instance()->GetBatchDispatcher();
    const UInt32 count = dispatcher.GetProviderCount( kernel );
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( dispatcher.GetProvider( kernel, i, provider ) && provider.tier == tier )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const COMPUTEBRIDGEMATH::SMathBridgeProviderContext*
FindMathProviderContext( MATH::TMathBatchKernel kernel, MATH::TMathBatchTier tier )
{
    MATH::SMathBatchProvider provider;
    if ( !FindMathProvider( kernel, tier, provider ) )
        return GUCEF_NULL;
    return static_cast< const COMPUTEBRIDGEMATH::SMathBridgeProviderContext* >( provider.context );
}

/*-------------------------------------------------------------------------*/

void
SetMathBridgeSettings( TComputeProviderTier forcedTier, UInt32 minCountMultiCore, UInt32 disabledTiers )
{
    CComputeGlobal* compute = CComputeGlobal::Instance();
    const MATH::CMathBatchDispatcher& dispatcher = MATH::CMathGlobal::Instance()->GetBatchDispatcher();
    CComputeSettings settings = compute->GetSettings();
    for ( UInt32 k=0; k<(UInt32) MATH::MATHBATCHKERNEL_MATH_COUNT; ++k )
    {
        const char* mathKernelId = dispatcher.GetKernelId( (MATH::TMathBatchKernel) k );
        if ( GUCEF_NULL == mathKernelId )
            continue;
        const CString kernelId = mathKernelId;
        CComputeSettings::SKernelProviderOverride providerOverride;
        providerOverride.forcedTier = forcedTier;
        providerOverride.disabledTiers = disabledTiers;
        providerOverride.minCount[ PROVIDERTIER_MULTICORE ] = minCountMultiCore;
        settings.SetKernelProviderOverride( kernelId, providerOverride );
    }
    compute->ApplySettings( settings );
}

/*-------------------------------------------------------------------------*/

void
ClearMathBridgeSettings( void )
{
    CComputeGlobal* compute = CComputeGlobal::Instance();
    const MATH::CMathBatchDispatcher& dispatcher = MATH::CMathGlobal::Instance()->GetBatchDispatcher();
    CComputeSettings settings = compute->GetSettings();
    for ( UInt32 k=0; k<(UInt32) MATH::MATHBATCHKERNEL_MATH_COUNT; ++k )
    {
        const char* mathKernelId = dispatcher.GetKernelId( (MATH::TMathBatchKernel) k );
        if ( GUCEF_NULL != mathKernelId )
            settings.RemoveKernelProviderOverride( mathKernelId );
    }
    compute->ApplySettings( settings );
}

/*-------------------------------------------------------------------------*/

bool
LoadMathBridgePlugin( void )
{
    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString first = "$MODULEDIR$/computebridgeMATH_d";
    const CORE::CString second = "$MODULEDIR$/computebridgeMATH";
    #else
    const CORE::CString first = "$MODULEDIR$/computebridgeMATH";
    const CORE::CString second = "$MODULEDIR$/computebridgeMATH_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    return pluginControl.AddPluginFromDir( first, BridgePluginGroup, true ) || pluginControl.AddPluginFromDir( second, BridgePluginGroup, true );
}

/*-------------------------------------------------------------------------*/

bool
UnloadMathBridgePlugin( void )
{
    return CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( BridgePluginGroup );
}

/*-------------------------------------------------------------------------*/

MATH::TFloat32Matrix4D
MakeTestMatrix( UInt32 index )
{
    static const Float32 general[ 16 ] = {  0.8f, 0.6f, 0.0f, 0.0f,
                                           -0.6f, 0.8f, 0.0f, 0.0f,
                                            0.1f, -0.2f, 1.5f, 0.0f,
                                            3.0f, -4.0f, 5.0f, 1.0f };
    static const Float32 singular[ 16 ] = {  1.0f, 2.0f, 3.0f, 0.0f,
                                             2.0f, 4.0f, 6.0f, 0.0f,
                                             0.0f, 0.0f, 0.0f, 0.0f,
                                             0.5f, 0.25f, 0.125f, 1.0f };
    return MATH::TFloat32Matrix4D::FromColumnMajor( 0 == index ? general : singular );
}

/*-------------------------------------------------------------------------*/

void
CTestBuffer::Resize( UInt64 bytes, UInt32 misalignBytes, UInt8 fill )
{
    m_misalign = misalignBytes;
    m_bytes.assign( static_cast< size_t >( bytes + misalignBytes ), fill );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace TEST */
}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
