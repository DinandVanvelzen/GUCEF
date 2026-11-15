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

#ifndef GUCEF_MATH_BATCHFUNCTIONS_H
#define GUCEF_MATH_BATCHFUNCTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef GUCEF_MATH_MATHBATCHTYPES_H
#include "gucefMATH_MathBatchTypes.h"
#define GUCEF_MATH_MATHBATCHTYPES_H
#endif /* GUCEF_MATH_MATHBATCHTYPES_H ? */

#ifndef GUCEF_MATH_CPERLINNOISE_H
#include "gucefMATH_CPerlinNoise.h"
#define GUCEF_MATH_CPERLINNOISE_H
#endif /* GUCEF_MATH_CPERLINNOISE_H ? */

#ifndef GUCEF_MATH_CVALUENOISE_H
#include "gucefMATH_CValueNoise.h"
#define GUCEF_MATH_CVALUENOISE_H
#endif /* GUCEF_MATH_CVALUENOISE_H ? */

#ifndef GUCEF_MATH_COPENSIMPLEX2NOISE_H
#include "gucefMATH_COpenSimplex2Noise.h"
#define GUCEF_MATH_COPENSIMPLEX2NOISE_H
#endif /* GUCEF_MATH_COPENSIMPLEX2NOISE_H ? */

#ifndef GUCEF_MATH_CWORLEYNOISE_H
#include "gucefMATH_CWorleyNoise.h"
#define GUCEF_MATH_CWORLEYNOISE_H
#endif /* GUCEF_MATH_CWORLEYNOISE_H ? */

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

/*
 *      Batch functions (section 4.12 of the design): one operation over arrays, run by the best provider of the batch
 *      dispatch table of CMathGlobal (scalar, SIMD, and the multi-core and accelerator tiers that bridge plugins add).
 *      Every function gives the result of its value-type operation per element, bit for bit on the SCALAR and SIMD
 *      tiers (section 5.5), takes any count (0 does nothing), pointers with the alignment of Float32, and works in place
 *      (out equal to in); partial overlap is not permitted.
 *
 *      'flags' is a bit set of EMathBatchFlags. Callers on a realtime thread pass MATHBATCH_REALTIME: the call then only
 *      reaches the SCALAR and SIMD tiers and is realtime-safe (no allocation, no lock), provided CMathGlobal::Instance()
 *      was called before. MATHBATCH_DETERMINISTIC restricts the call to providers that are bit-identical with the scalar
 *      one, MATHBATCH_NO_MULTICORE suits callers that already run in a parallel task.
 *
 *      The Q16.16 functions are always deterministic (fixed point has one result on every machine). The Float64 and
 *      noise functions follow the same rules as the Float32 ones.
 */

/** out[ i ] = m.TransformPoint( in[ i ] ) (w = 1) */
GUCEF_MATH_PUBLIC_CPP void
TransformPoints( const TFloat32Matrix4D& m, const TFloat32Point3D* in, TFloat32Point3D* out, UInt32 count, UInt32 flags = 0 );

/**
 *  TransformPoints over 3 Float32 (x, y, z) every 'strideBytes' (interleaved vertex data). Strides must be at least 12
 *  bytes (any stride for a single point): otherwise nothing is written, false is returned and debug builds assert.
 *  True when the points were transformed (also for count 0).
 */
GUCEF_MATH_PUBLIC_CPP bool
TransformPointsStrided( const TFloat32Matrix4D& m, const void* in, UInt32 inStrideBytes, void* out, UInt32 outStrideBytes, UInt32 count, UInt32 flags = 0 );

/** out[ i ] = m.TransformVector( in[ i ] ) (w = 0) */
GUCEF_MATH_PUBLIC_CPP void
TransformVectors( const TFloat32Matrix4D& m, const TFloat32Vector3D* in, TFloat32Vector3D* out, UInt32 count, UInt32 flags = 0 );

/**
 *  out[ i ] = in[ i ] transformed by the inverse transpose of the upper 3 x 3 block of m and normalised, as
 *  m.TransformNormal does for directions (computed once per call). A normal that cannot be normalised after the
 *  transform (zero, not finite) is passed through unchanged.
 */
GUCEF_MATH_PUBLIC_CPP void
TransformNormals( const TFloat32Matrix4D& m, const TFloat32Vector3D* in, TFloat32Vector3D* out, UInt32 count, UInt32 flags = 0 );

/** out[ i ] = m.Transform( in[ i ] ) */
GUCEF_MATH_PUBLIC_CPP void
TransformVectors4( const TFloat32Matrix4D& m, const TFloat32Vector4D* in, TFloat32Vector4D* out, UInt32 count, UInt32 flags = 0 );

/** out[ i ] = a[ i ] * b[ i ]; out may be a or b */
GUCEF_MATH_PUBLIC_CPP void
MultiplyMatrices( const TFloat32Matrix4D* a, const TFloat32Matrix4D* b, TFloat32Matrix4D* out, UInt32 count, UInt32 flags = 0 );

/** out[ i ] = a[ i ].Dot( b[ i ] ) */
GUCEF_MATH_PUBLIC_CPP void
DotProducts3( const TFloat32Vector3D* a, const TFloat32Vector3D* b, Float32* out, UInt32 count, UInt32 flags = 0 );

/** out[ i ] = in[ i ].Normalized() (the zero vector for zero and non-finite vectors) */
GUCEF_MATH_PUBLIC_CPP void
NormalizeVectors3( const TFloat32Vector3D* in, TFloat32Vector3D* out, UInt32 count, UInt32 flags = 0 );

/** out[ i ] = TFloat32Matrix4D::FromQuaternion( in[ i ] ) */
GUCEF_MATH_PUBLIC_CPP void
QuaternionsToMatrices( const TFloat32Quaternion* in, TFloat32Matrix4D* out, UInt32 count, UInt32 flags = 0 );

/** out[ i ] = CFloat16::FromFloat32( in[ i ] ) (nearest, ties to even); out may be the same array as in */
GUCEF_MATH_PUBLIC_CPP void
ConvertFloat32ToFloat16( const Float32* in, CFloat16* out, UInt32 count, UInt32 flags = 0 );

/** out[ i ] = in[ i ].ToFloat32() (exact); out may be the same array as in */
GUCEF_MATH_PUBLIC_CPP void
ConvertFloat16ToFloat32( const CFloat16* in, Float32* out, UInt32 count, UInt32 flags = 0 );

/*-------------------------------------------------------------------------*/

/** Float64: as the Float32 functions of the same name, bit for bit the TFloat64Matrix4D operations of every element */
GUCEF_MATH_PUBLIC_CPP void
TransformPoints( const TFloat64Matrix4D& m, const TFloat64Point3D* in, TFloat64Point3D* out, UInt32 count, UInt32 flags = 0 );

GUCEF_MATH_PUBLIC_CPP void
TransformVectors( const TFloat64Matrix4D& m, const TFloat64Vector3D* in, TFloat64Vector3D* out, UInt32 count, UInt32 flags = 0 );

GUCEF_MATH_PUBLIC_CPP void
MultiplyMatrices( const TFloat64Matrix4D* a, const TFloat64Matrix4D* b, TFloat64Matrix4D* out, UInt32 count, UInt32 flags = 0 );

/**
 *  Q16.16 fixed point: the TFixed32Matrix4D operations of every element. Fixed point results do not depend on the
 *  hardware, so these calls are always deterministic: only providers that are bit-identical with the scalar one serve
 *  them, whatever the flags.
 */
GUCEF_MATH_PUBLIC_CPP void
TransformPoints( const TFixed32Matrix4D& m, const TFixed32Point3D* in, TFixed32Point3D* out, UInt32 count, UInt32 flags = 0 );

GUCEF_MATH_PUBLIC_CPP void
MultiplyMatrices( const TFixed32Matrix4D* a, const TFixed32Matrix4D* b, TFixed32Matrix4D* out, UInt32 count, UInt32 flags = 0 );

/**
 *  The gains of every source of a prepared panner: gains[ i * speakerCount + s ] is speaker s for the source at
 *  azimuths[ i ], the result of vbap.ComputeGains( azimuths[ i ], ..., targetPower ) (nothing is written when the panner
 *  is not prepared). 'gains' holds sourceCount * vbap.GetSpeakerCount() values.
 */
GUCEF_MATH_PUBLIC_CPP void
ComputeVbapGains2D( const TFloat32Vbap2D& vbap, const TFloat32Radian* azimuths, Float32* gains, UInt32 sourceCount, Float32 targetPower, UInt32 flags = 0 );

/**
 *  Fills a grid of width x height samples of a noise, x first: out[ iy * width + ix ] =
 *  noise.Noise2D( originX + ix * stepX, originY + iy * stepY ) with the lattice positions converted to Float32 before
 *  the multiplication. The 3D forms fill width x height x depth samples, z last. Perlin noise is computed in SIMD,
 *  every noise gives the values of its own Noise2D / Noise3D. 'out' holds width * height ( * depth ) values; nothing
 *  is written when an extent is 0 or the grid has more than 2^32 - 1 samples.
 */
GUCEF_MATH_PUBLIC_CPP void
FillNoise2D( const CPerlinNoise& noise, Float32 originX, Float32 originY, Float32 stepX, Float32 stepY, UInt32 width, UInt32 height, Float32* out, UInt32 flags = 0 );

GUCEF_MATH_PUBLIC_CPP void
FillNoise2D( const CValueNoise& noise, Float32 originX, Float32 originY, Float32 stepX, Float32 stepY, UInt32 width, UInt32 height, Float32* out, UInt32 flags = 0 );

GUCEF_MATH_PUBLIC_CPP void
FillNoise2D( const COpenSimplex2Noise& noise, Float32 originX, Float32 originY, Float32 stepX, Float32 stepY, UInt32 width, UInt32 height, Float32* out, UInt32 flags = 0 );

GUCEF_MATH_PUBLIC_CPP void
FillNoise2D( const CWorleyNoise& noise, Float32 originX, Float32 originY, Float32 stepX, Float32 stepY, UInt32 width, UInt32 height, Float32* out, UInt32 flags = 0 );

GUCEF_MATH_PUBLIC_CPP void
FillNoise3D( const CPerlinNoise& noise, Float32 originX, Float32 originY, Float32 originZ, Float32 stepX, Float32 stepY, Float32 stepZ, UInt32 width, UInt32 height, UInt32 depth, Float32* out, UInt32 flags = 0 );

GUCEF_MATH_PUBLIC_CPP void
FillNoise3D( const CValueNoise& noise, Float32 originX, Float32 originY, Float32 originZ, Float32 stepX, Float32 stepY, Float32 stepZ, UInt32 width, UInt32 height, UInt32 depth, Float32* out, UInt32 flags = 0 );

GUCEF_MATH_PUBLIC_CPP void
FillNoise3D( const COpenSimplex2Noise& noise, Float32 originX, Float32 originY, Float32 originZ, Float32 stepX, Float32 stepY, Float32 stepZ, UInt32 width, UInt32 height, UInt32 depth, Float32* out, UInt32 flags = 0 );

GUCEF_MATH_PUBLIC_CPP void
FillNoise3D( const CWorleyNoise& noise, Float32 originX, Float32 originY, Float32 originZ, Float32 stepX, Float32 stepY, Float32 stepZ, UInt32 width, UInt32 height, UInt32 depth, Float32* out, UInt32 flags = 0 );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_BATCHFUNCTIONS_H ? */
