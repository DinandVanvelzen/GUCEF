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

#ifndef GUCEF_MATH_MATHBATCHTYPES_H
#define GUCEF_MATH_MATHBATCHTYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_MATHSTATUS_H
#include "gucefMATH_MathStatus.h"
#define GUCEF_MATH_MATHSTATUS_H
#endif /* GUCEF_MATH_MATHSTATUS_H ? */

#ifndef GUCEF_MATH_SIMD_CONFIG_H
#include "gucefMATH_simd_config.h"
#define GUCEF_MATH_SIMD_CONFIG_H
#endif /* GUCEF_MATH_SIMD_CONFIG_H ? */

#ifndef GUCEF_MATH_CFLOAT16_H
#include "gucefMATH_CFloat16.h"
#define GUCEF_MATH_CFLOAT16_H
#endif /* GUCEF_MATH_CFLOAT16_H ? */

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef GUCEF_MATH_CTVBAP2D_H
#include "gucefMATH_CTVbap2D.h"
#define GUCEF_MATH_CTVBAP2D_H
#endif /* GUCEF_MATH_CTVBAP2D_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Types of the batch dispatch table (section 5.6 of the design). A batch kernel is an operation over arrays,
 *      identified by a TMathBatchKernel with a string id ("MATH.TransformPoints3.F32", gucefCOMPUTE section 5.1). Each
 *      kernel has one argument struct and one typed provider signature
 *          TMathStatus ( * )( void* context, const S<Kernel>Args& args )
 *      that every provider of the kernel implements. Providers are registered in tiers; the dispatcher calls the best
 *      eligible one per call (CMathBatchDispatcher).
 *
 *      Rules for every kernel and every provider:
 *      - Pointers need only the alignment of Float32 (UInt16 for halves); byte strides are at least the size of one
 *        element and need not be multiples of 4: providers read and write strided data as bytes.
 *      - In place: an output may be the same pointer as an input with the same byte stride (the conversions, which
 *        have no strides, accept the same array). Partial overlap is not permitted.
 *      - count 0 is valid and touches nothing; counts are UInt32, callers split larger batches.
 *      - A provider returns MATHSTATUS_OK when it produced the complete result. Any other status is a failure, which
 *        only providers flagged MATHPROVIDER_MAY_FAIL may report; it must leave the output as it was where an output
 *        aliases an input. The dispatcher then calls the next eligible provider with the same arguments.
 */

/** The tiers of the dispatch chain. The values equal those of TComputeProviderTier (gucefCOMPUTE): never change them */
enum EMathBatchTier
{
    MATHBATCHTIER_SCALAR      = 0 ,     /**< the reference: every kernel has one, it never fails */
    MATHBATCHTIER_SIMD        = 1 ,     /**< SIMD on the calling thread */
    MATHBATCHTIER_MULTICORE   = 2 ,     /**< the CPU backend of gucefCOMPUTE (bridge plugin), fork-join */
    MATHBATCHTIER_ACCELERATOR = 3 ,     /**< a GPU or other device (bridge plugin) */

    MATHBATCHTIER_COUNT       = 4
};
typedef enum EMathBatchTier TMathBatchTier;

/*-------------------------------------------------------------------------*/

/**
 *  The kernels. MATH kernels are numbered from 0 (new ones are appended before MATHBATCHKERNEL_MATH_COUNT, the values
 *  are stable once released); other modules register theirs in the external range with
 *  CMathBatchDispatcher::RegisterExternalKernel (gucefGEOMETRY: 256 to 319, gucefPHYSICS: 320 to 383).
 */
enum EMathBatchKernel
{
    MATHBATCHKERNEL_TRANSFORM_POINTS3_F32 = 0 ,         /**< "MATH.TransformPoints3.F32", SMathTransformPoints3F32Args */
    MATHBATCHKERNEL_TRANSFORM_VECTORS3_F32 ,            /**< "MATH.TransformVectors3.F32", SMathTransformVectors3F32Args */
    MATHBATCHKERNEL_TRANSFORM_NORMALS3_F32 ,            /**< "MATH.TransformNormals3.F32", SMathTransformNormals3F32Args */
    MATHBATCHKERNEL_TRANSFORM_VECTORS4_F32 ,            /**< "MATH.TransformVectors4.F32", SMathTransformVectors4F32Args */
    MATHBATCHKERNEL_MULTIPLY_MATRICES4_F32 ,            /**< "MATH.MultiplyMatrices4.F32", SMathMultiplyMatrices4F32Args */
    MATHBATCHKERNEL_QUATERNIONS_TO_MATRICES_F32 ,       /**< "MATH.QuaternionsToMatrices.F32", SMathQuaternionsToMatricesF32Args */
    MATHBATCHKERNEL_DOT_PRODUCTS3_F32 ,                 /**< "MATH.DotProducts3.F32", SMathDotProducts3F32Args */
    MATHBATCHKERNEL_NORMALIZE_VECTORS3_F32 ,            /**< "MATH.NormalizeVectors3.F32", SMathNormalizeVectors3F32Args */
    MATHBATCHKERNEL_CONVERT_F32_TO_F16 ,                /**< "MATH.ConvertF32ToF16.F16", SMathConvertF32ToF16Args */
    MATHBATCHKERNEL_CONVERT_F16_TO_F32 ,                /**< "MATH.ConvertF16ToF32.F32", SMathConvertF16ToF32Args */
    MATHBATCHKERNEL_TRANSFORM_POINTS3_F64 ,             /**< "MATH.TransformPoints3.F64", SMathTransformPoints3F64Args */
    MATHBATCHKERNEL_TRANSFORM_VECTORS3_F64 ,            /**< "MATH.TransformVectors3.F64", SMathTransformVectors3F64Args */
    MATHBATCHKERNEL_MULTIPLY_MATRICES4_F64 ,            /**< "MATH.MultiplyMatrices4.F64", SMathMultiplyMatrices4F64Args */
    MATHBATCHKERNEL_TRANSFORM_POINTS3_FX32 ,            /**< "MATH.TransformPoints3.FX32" (Q16.16), SMathTransformPoints3Fx32Args; always deterministic */
    MATHBATCHKERNEL_MULTIPLY_MATRICES4_FX32 ,           /**< "MATH.MultiplyMatrices4.FX32" (Q16.16), SMathMultiplyMatrices4Fx32Args; always deterministic */
    MATHBATCHKERNEL_VBAP_GAINS2D_F32 ,                  /**< "MATH.VbapGains2D.F32", SMathVbapGains2DF32Args */
    MATHBATCHKERNEL_NOISE_FIELD2D_F32 ,                 /**< "MATH.NoiseField2D.F32", SMathNoiseField2DF32Args */
    MATHBATCHKERNEL_NOISE_FIELD3D_F32 ,                 /**< "MATH.NoiseField3D.F32", SMathNoiseField3DF32Args */

    MATHBATCHKERNEL_MATH_COUNT ,

    MATHBATCHKERNEL_EXTERNAL_FIRST = 256 ,
    MATHBATCHKERNEL_GEOMETRY_FIRST = 256 ,
    MATHBATCHKERNEL_GEOMETRY_LAST  = 319 ,
    MATHBATCHKERNEL_PHYSICS_FIRST  = 320 ,
    MATHBATCHKERNEL_PHYSICS_LAST   = 383 ,
    MATHBATCHKERNEL_EXTERNAL_LAST  = 511 ,

    MATHBATCHKERNEL_SLOT_COUNT     = 512                /**< one past the highest kernel id */
};
typedef enum EMathBatchKernel TMathBatchKernel;

/*-------------------------------------------------------------------------*/

/** Flags of one batch call, a bit set (the UInt32 flags argument of the batch functions) */
enum EMathBatchFlags
{
    MATHBATCH_REALTIME       = 0x01 ,   /**< SCALAR and SIMD providers only: no allocation, lock or blocking call */
    MATHBATCH_DETERMINISTIC  = 0x02 ,   /**< MATHPROVIDER_BITEXACT providers only */
    MATHBATCH_NO_ACCELERATOR = 0x04 ,
    MATHBATCH_NO_MULTICORE   = 0x08 ,   /**< for callers already running inside a parallel task */
    MATHBATCH_FORCE_SCALAR   = 0x10     /**< tests and diagnostics */
};
typedef UInt32 TMathBatchFlags;

/** Properties of a provider, a bit set */
enum EMathProviderFlags
{
    MATHPROVIDER_BITEXACT               = 0x01 ,    /**< bit-identical with the SCALAR provider of the kernel */
    MATHPROVIDER_REQUIRES_ALIGNED_INPUT = 0x02 ,    /**< the input and output pointers must be multiples of 'alignment' */
    MATHPROVIDER_MAY_FAIL               = 0x04      /**< may return an error status, the call then continues on the next tier */
};
typedef UInt32 TMathProviderFlags;

/*-------------------------------------------------------------------------*/

/** A provider function in generic form; it is cast back to the typed signature of its kernel before the call */
typedef void ( *TMathBatchGenericFunction )( void );

/** One implementation of a kernel */
struct SMathBatchProvider
{
    TMathBatchTier tier;
    const char* name;                       /**< static text for logs and diagnostics */
    TMathBatchGenericFunction function;     /**< the typed provider function of the kernel, cast */
    UInt32 minCount;                        /**< used for counts from minCount ... */
    UInt32 maxCount;                        /**< ... up to and including maxCount */
    UInt32 flags;                           /**< TMathProviderFlags */
    UInt32 alignment;                       /**< power of two, with MATHPROVIDER_REQUIRES_ALIGNED_INPUT */
    UInt64 requiredCpuFeatures;             /**< GUCEF_CPU_ISA_* bits, checked at registration */
    void* context;                          /**< passed to every call */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      KERNEL ARGUMENTS                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  TransformPoints3 (w = 1), TransformVectors3 (w = 0) and TransformNormals3 over arrays of 3 Float32 (x, y, z) with byte
 *  strides. Points and vectors: each output component is the canonical 4 term row times ( x, y, z, w ) product of
 *  CTMatrix4Ops. Normals: the transform of CTMatrix3D::TransformNormal on the upper 3 x 3 block (cofactor matrix, sign of
 *  the determinant, computed once per call) followed by normalisation; a normal that cannot be normalised (zero or
 *  not finite after the transform) is passed through unchanged, as TransformNormal does.
 */
struct SMathTransformPoints3F32Args
{
    const TFloat32Matrix4D* matrix;
    const void* in;
    UInt32 inStrideBytes;
    void* out;
    UInt32 outStrideBytes;
    UInt32 count;
};
typedef SMathTransformPoints3F32Args SMathTransformVectors3F32Args;
typedef SMathTransformPoints3F32Args SMathTransformNormals3F32Args;

/** TransformVectors4: arrays of 4 Float32, each output the canonical CTMatrix4Ops::Transform */
struct SMathTransformVectors4F32Args
{
    const TFloat32Matrix4D* matrix;
    const void* in;
    UInt32 inStrideBytes;
    void* out;
    UInt32 outStrideBytes;
    UInt32 count;
};

/** MultiplyMatrices4: out[ i ] = a[ i ] * b[ i ], 16 Float32 column-major each, the canonical CTMatrix4Ops::Multiply */
struct SMathMultiplyMatrices4F32Args
{
    const void* a;
    UInt32 aStrideBytes;
    const void* b;
    UInt32 bStrideBytes;
    void* out;
    UInt32 outStrideBytes;
    UInt32 count;
};

/** QuaternionsToMatrices: x, y, z, w to the 16 Float32 of CTMatrix4D::FromQuaternion (the zero quaternion gives identity) */
struct SMathQuaternionsToMatricesF32Args
{
    const void* in;
    UInt32 inStrideBytes;
    void* out;
    UInt32 outStrideBytes;
    UInt32 count;
};

/** DotProducts3: out[ i ] = ( ax bx + ay by ) + az bz, the order of CTVector3D::Dot */
struct SMathDotProducts3F32Args
{
    const void* a;
    UInt32 aStrideBytes;
    const void* b;
    UInt32 bStrideBytes;
    void* out;
    UInt32 outStrideBytes;
    UInt32 count;
};

/** NormalizeVectors3: CTVector3D::Normalized of each vector (the zero vector for zero and non-finite input) */
struct SMathNormalizeVectors3F32Args
{
    const void* in;
    UInt32 inStrideBytes;
    void* out;
    UInt32 outStrideBytes;
    UInt32 count;
};

/**
 *  ConvertF32ToF16: CFloat16::FromFloat32 with ROUNDINGMODE_HALF_EVEN; out may be the same array as in. The x86 F16C
 *  provider is bit-exact (its rounding is an immediate, independent of MXCSR); the NEON provider is registered without
 *  MATHPROVIDER_BITEXACT because the AArch64 conversions follow FPCR (AHP, DN, FZ16, rounding mode).
 */
struct SMathConvertF32ToF16Args
{
    const Float32* in;
    CFloat16* out;
    UInt32 count;
};

/**
 *  ConvertF16ToF32: CFloat16::ToFloat32 (exact, signalling NaN kept); out may be the same array as in. The F16C provider
 *  converts blocks with a NaN in software and is bit-exact; the NEON provider is not (FCVTL quiets signalling NaN and
 *  follows FPCR) and is registered without MATHPROVIDER_BITEXACT.
 */
struct SMathConvertF16ToF32Args
{
    const CFloat16* in;
    Float32* out;
    UInt32 count;
};

/*-------------------------------------------------------------------------*/

/**
 *  The Float64 and Q16.16 forms of the transform and matrix product kernels. The arguments are those of the Float32
 *  kernels with the element types changed: points and vectors are 3 Float64 (24 bytes) or 3 Q16.16 raw values
 *  (Int32, 12 bytes), matrices 16 of them in column-major order. Each output component is the canonical 4 term row times
 *  column product of CTMatrix4Ops of the kind: Float64 as the primary template, Q16.16 exact (one rounding, the value
 *  type rules of fixed point). The Q16.16 kernels are always deterministic (the dispatcher never selects a provider
 *  that is not MATHPROVIDER_BITEXACT for them).
 */
struct SMathTransformPoints3F64Args
{
    const TFloat64Matrix4D* matrix;
    const void* in;
    UInt32 inStrideBytes;
    void* out;
    UInt32 outStrideBytes;
    UInt32 count;
};
typedef SMathTransformPoints3F64Args SMathTransformVectors3F64Args;

struct SMathMultiplyMatrices4F64Args
{
    const void* a;
    UInt32 aStrideBytes;
    const void* b;
    UInt32 bStrideBytes;
    void* out;
    UInt32 outStrideBytes;
    UInt32 count;
};

struct SMathTransformPoints3Fx32Args
{
    const TFixed32Matrix4D* matrix;
    const void* in;
    UInt32 inStrideBytes;
    void* out;
    UInt32 outStrideBytes;
    UInt32 count;
};

typedef SMathMultiplyMatrices4F64Args SMathMultiplyMatrices4Fx32Args;

/*-------------------------------------------------------------------------*/

/**
 *  VbapGains2D: for source i the speaker gains CTVbap2D::ComputeGains( azimuth i, ..., targetPower ) of a prepared
 *  panner, GetSpeakerCount() Float32 values written at 'gains' + i * gainsStrideBytes (the stride must be at least
 *  GetSpeakerCount() * 4 bytes), the azimuths read as Float32 radians at 'azimuths' + i * azimuthStrideBytes. A panner
 *  that is not prepared writes nothing; a target power that is not positive or an azimuth that is not finite gives zero
 *  gains for that source (as ComputeGains does).
 */
struct SMathVbapGains2DF32Args
{
    const TFloat32Vbap2D* vbap;
    const void* azimuths;
    UInt32 azimuthStrideBytes;
    Float32 targetPower;
    void* gains;
    UInt32 gainsStrideBytes;
    UInt32 count;
};

/*-------------------------------------------------------------------------*/

/** The noise classes a noise field kernel can sample; 'noise' of the arguments points to an object of the class */
enum EMathNoiseType
{
    MATHNOISE_PERLIN        = 0 ,       /**< CPerlinNoise: Noise2D, Noise3D */
    MATHNOISE_VALUE         = 1 ,       /**< CValueNoise */
    MATHNOISE_OPENSIMPLEX2  = 2 ,       /**< COpenSimplex2Noise */
    MATHNOISE_WORLEY        = 3 ,       /**< CWorleyNoise: Noise2D, Noise3D (the mapped feature distance) */

    MATHNOISE_COUNT         = 4
};
typedef enum EMathNoiseType TMathNoiseType;

/**
 *  NoiseField2D and NoiseField3D: a regular grid of samples of a noise, in x-major order. Item j (counted from the grid
 *  origin: baseItem + i for output element i) has the lattice position ix = j mod width, iy = j / width ( 2D ), or
 *  ix = j mod width, iy = ( j / width ) mod height, iz = j / ( width * height ) ( 3D ), and the value
 *      noise.Noise2D( originX + ix * stepX, originY + iy * stepY )     ( the lattice position converted to Float32 first )
 *  or the 3 coordinate form with the same expression for z. 'out' receives 'count' contiguous Float32 values, the first
 *  being item baseItem: a provider that processes a part of a call adjusts out, baseItem and count. 'width' must be at
 *  least 1, and for 3D 'height' too (otherwise nothing is written); for 2D 'height' is the extent of the grid that
 *  providers with their own dispatch size use, items beyond it simply continue in further rows. Perlin noise is
 *  computed in SIMD by the SIMD providers; the other noise types are evaluated by their class, one sample at a time, in
 *  every provider.
 */
struct SMathNoiseField2DF32Args
{
    const void* noise;
    UInt32 noiseType;                       /**< TMathNoiseType */
    Float32 originX;
    Float32 originY;
    Float32 stepX;
    Float32 stepY;
    UInt32 width;
    UInt32 height;
    UInt32 baseItem;
    Float32* out;
    UInt32 count;
};

struct SMathNoiseField3DF32Args
{
    const void* noise;
    UInt32 noiseType;                       /**< TMathNoiseType */
    Float32 originX;
    Float32 originY;
    Float32 originZ;
    Float32 stepX;
    Float32 stepY;
    Float32 stepZ;
    UInt32 width;
    UInt32 height;
    UInt32 baseItem;
    Float32* out;
    UInt32 count;
};

/*-------------------------------------------------------------------------*/

typedef TMathStatus ( *TMathTransformPoints3F32Function )( void* context, const SMathTransformPoints3F32Args& args );
typedef TMathStatus ( *TMathTransformVectors3F32Function )( void* context, const SMathTransformVectors3F32Args& args );
typedef TMathStatus ( *TMathTransformNormals3F32Function )( void* context, const SMathTransformNormals3F32Args& args );
typedef TMathStatus ( *TMathTransformVectors4F32Function )( void* context, const SMathTransformVectors4F32Args& args );
typedef TMathStatus ( *TMathMultiplyMatrices4F32Function )( void* context, const SMathMultiplyMatrices4F32Args& args );
typedef TMathStatus ( *TMathQuaternionsToMatricesF32Function )( void* context, const SMathQuaternionsToMatricesF32Args& args );
typedef TMathStatus ( *TMathDotProducts3F32Function )( void* context, const SMathDotProducts3F32Args& args );
typedef TMathStatus ( *TMathNormalizeVectors3F32Function )( void* context, const SMathNormalizeVectors3F32Args& args );
typedef TMathStatus ( *TMathConvertF32ToF16Function )( void* context, const SMathConvertF32ToF16Args& args );
typedef TMathStatus ( *TMathConvertF16ToF32Function )( void* context, const SMathConvertF16ToF32Args& args );
typedef TMathStatus ( *TMathTransformPoints3F64Function )( void* context, const SMathTransformPoints3F64Args& args );
typedef TMathStatus ( *TMathTransformVectors3F64Function )( void* context, const SMathTransformVectors3F64Args& args );
typedef TMathStatus ( *TMathMultiplyMatrices4F64Function )( void* context, const SMathMultiplyMatrices4F64Args& args );
typedef TMathStatus ( *TMathTransformPoints3Fx32Function )( void* context, const SMathTransformPoints3Fx32Args& args );
typedef TMathStatus ( *TMathMultiplyMatrices4Fx32Function )( void* context, const SMathMultiplyMatrices4Fx32Args& args );
typedef TMathStatus ( *TMathVbapGains2DF32Function )( void* context, const SMathVbapGains2DF32Args& args );
typedef TMathStatus ( *TMathNoiseField2DF32Function )( void* context, const SMathNoiseField2DF32Args& args );
typedef TMathStatus ( *TMathNoiseField3DF32Function )( void* context, const SMathNoiseField3DF32Args& args );

/*-------------------------------------------------------------------------//
//                                                                         //
//      ARGUMENT CHECKS                                                    //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Used by CMathBatchDispatcher::Execute for every call, found by overload resolution (and argument dependent
 *      lookup for the argument structs of other modules, which provide their own overloads in their namespace):
 *      - MathBatchAlignmentBits: the bitwise or of every pointer and byte stride of the arguments, so a provider that
 *        requires aligned input (GUCEF_CPU_REQUIRES_ALIGNED_ACCESS builds) is only selected when all of them are
 *        multiples of its alignment;
 *      - MathBatchArgumentsValid: false when a byte stride is smaller than one element (with more than one element) or
 *        a required object pointer is NULL. Such a call is rejected without calling a provider: SIMD providers read
 *        whole registers from strided elements and rely on the strides.
 *      The templates are the fallback for argument structs without an overload: no alignment information, always valid.
 */

template < typename TArgs >
GUCEF_MATH_FORCE_INLINE size_t
MathBatchAlignmentBits( const TArgs& /* args */ ) GUCEF_MATH_REALTIME_CONTEXT
{
    return 0;
}

template < typename TArgs >
GUCEF_MATH_FORCE_INLINE bool
MathBatchArgumentsValid( TMathBatchKernel /* kernel */, const TArgs& /* args */ ) GUCEF_MATH_REALTIME_CONTEXT
{
    return true;
}

/** True when 'count' elements of 'elementSize' bytes fit their stride (a single element needs no stride) */
GUCEF_MATH_FORCE_INLINE bool
MathBatchStrideValid( UInt32 count, UInt32 strideBytes, UInt32 elementSize ) GUCEF_MATH_REALTIME_CONTEXT
{
    return count <= 1 || strideBytes >= elementSize;
}

GUCEF_MATH_FORCE_INLINE size_t
MathBatchAlignmentBits( const SMathTransformPoints3F32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (size_t) a.in | (size_t) a.out | a.inStrideBytes | a.outStrideBytes;
}

GUCEF_MATH_FORCE_INLINE bool
MathBatchArgumentsValid( TMathBatchKernel /* kernel */, const SMathTransformPoints3F32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return GUCEF_NULL != a.matrix && MathBatchStrideValid( a.count, a.inStrideBytes, 12 ) && MathBatchStrideValid( a.count, a.outStrideBytes, 12 );
}

GUCEF_MATH_FORCE_INLINE size_t
MathBatchAlignmentBits( const SMathTransformVectors4F32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (size_t) a.in | (size_t) a.out | a.inStrideBytes | a.outStrideBytes;
}

GUCEF_MATH_FORCE_INLINE bool
MathBatchArgumentsValid( TMathBatchKernel /* kernel */, const SMathTransformVectors4F32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return GUCEF_NULL != a.matrix && MathBatchStrideValid( a.count, a.inStrideBytes, 16 ) && MathBatchStrideValid( a.count, a.outStrideBytes, 16 );
}

GUCEF_MATH_FORCE_INLINE size_t
MathBatchAlignmentBits( const SMathMultiplyMatrices4F32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (size_t) a.a | (size_t) a.b | (size_t) a.out | a.aStrideBytes | a.bStrideBytes | a.outStrideBytes;
}

GUCEF_MATH_FORCE_INLINE bool
MathBatchArgumentsValid( TMathBatchKernel /* kernel */, const SMathMultiplyMatrices4F32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return MathBatchStrideValid( a.count, a.aStrideBytes, 64 ) && MathBatchStrideValid( a.count, a.bStrideBytes, 64 ) && MathBatchStrideValid( a.count, a.outStrideBytes, 64 );
}

GUCEF_MATH_FORCE_INLINE size_t
MathBatchAlignmentBits( const SMathQuaternionsToMatricesF32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (size_t) a.in | (size_t) a.out | a.inStrideBytes | a.outStrideBytes;
}

GUCEF_MATH_FORCE_INLINE bool
MathBatchArgumentsValid( TMathBatchKernel /* kernel */, const SMathQuaternionsToMatricesF32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return MathBatchStrideValid( a.count, a.inStrideBytes, 16 ) && MathBatchStrideValid( a.count, a.outStrideBytes, 64 );
}

GUCEF_MATH_FORCE_INLINE size_t
MathBatchAlignmentBits( const SMathDotProducts3F32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (size_t) a.a | (size_t) a.b | (size_t) a.out | a.aStrideBytes | a.bStrideBytes | a.outStrideBytes;
}

GUCEF_MATH_FORCE_INLINE bool
MathBatchArgumentsValid( TMathBatchKernel /* kernel */, const SMathDotProducts3F32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return MathBatchStrideValid( a.count, a.aStrideBytes, 12 ) && MathBatchStrideValid( a.count, a.bStrideBytes, 12 ) && MathBatchStrideValid( a.count, a.outStrideBytes, 4 );
}

GUCEF_MATH_FORCE_INLINE size_t
MathBatchAlignmentBits( const SMathNormalizeVectors3F32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (size_t) a.in | (size_t) a.out | a.inStrideBytes | a.outStrideBytes;
}

GUCEF_MATH_FORCE_INLINE bool
MathBatchArgumentsValid( TMathBatchKernel /* kernel */, const SMathNormalizeVectors3F32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return MathBatchStrideValid( a.count, a.inStrideBytes, 12 ) && MathBatchStrideValid( a.count, a.outStrideBytes, 12 );
}

GUCEF_MATH_FORCE_INLINE size_t
MathBatchAlignmentBits( const SMathConvertF32ToF16Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (size_t) a.in | (size_t) a.out;
}

GUCEF_MATH_FORCE_INLINE size_t
MathBatchAlignmentBits( const SMathConvertF16ToF32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (size_t) a.in | (size_t) a.out;
}

GUCEF_MATH_FORCE_INLINE size_t
MathBatchAlignmentBits( const SMathTransformPoints3F64Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (size_t) a.in | (size_t) a.out | a.inStrideBytes | a.outStrideBytes;
}

GUCEF_MATH_FORCE_INLINE bool
MathBatchArgumentsValid( TMathBatchKernel /* kernel */, const SMathTransformPoints3F64Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return GUCEF_NULL != a.matrix && MathBatchStrideValid( a.count, a.inStrideBytes, 24 ) && MathBatchStrideValid( a.count, a.outStrideBytes, 24 );
}

/** Shared by the Float64 (128 byte) and the Q16.16 (64 byte) matrix products */
GUCEF_MATH_FORCE_INLINE size_t
MathBatchAlignmentBits( const SMathMultiplyMatrices4F64Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (size_t) a.a | (size_t) a.b | (size_t) a.out | a.aStrideBytes | a.bStrideBytes | a.outStrideBytes;
}

GUCEF_MATH_FORCE_INLINE bool
MathBatchArgumentsValid( TMathBatchKernel kernel, const SMathMultiplyMatrices4F64Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt32 size = MATHBATCHKERNEL_MULTIPLY_MATRICES4_FX32 == kernel ? 64 : 128;
    return MathBatchStrideValid( a.count, a.aStrideBytes, size ) && MathBatchStrideValid( a.count, a.bStrideBytes, size ) && MathBatchStrideValid( a.count, a.outStrideBytes, size );
}

GUCEF_MATH_FORCE_INLINE size_t
MathBatchAlignmentBits( const SMathTransformPoints3Fx32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (size_t) a.in | (size_t) a.out | a.inStrideBytes | a.outStrideBytes;
}

GUCEF_MATH_FORCE_INLINE bool
MathBatchArgumentsValid( TMathBatchKernel /* kernel */, const SMathTransformPoints3Fx32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return GUCEF_NULL != a.matrix && MathBatchStrideValid( a.count, a.inStrideBytes, 12 ) && MathBatchStrideValid( a.count, a.outStrideBytes, 12 );
}

GUCEF_MATH_FORCE_INLINE size_t
MathBatchAlignmentBits( const SMathVbapGains2DF32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (size_t) a.azimuths | (size_t) a.gains | a.azimuthStrideBytes | a.gainsStrideBytes;
}

GUCEF_MATH_FORCE_INLINE bool
MathBatchArgumentsValid( TMathBatchKernel /* kernel */, const SMathVbapGains2DF32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return GUCEF_NULL != a.vbap && MathBatchStrideValid( a.count, a.azimuthStrideBytes, 4 ) &&
           MathBatchStrideValid( a.count, a.gainsStrideBytes, 4 * a.vbap->GetSpeakerCount() );
}

GUCEF_MATH_FORCE_INLINE size_t
MathBatchAlignmentBits( const SMathNoiseField2DF32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (size_t) a.out;
}

GUCEF_MATH_FORCE_INLINE size_t
MathBatchAlignmentBits( const SMathNoiseField3DF32Args& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (size_t) a.out;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_MATHBATCHTYPES_H ? */
