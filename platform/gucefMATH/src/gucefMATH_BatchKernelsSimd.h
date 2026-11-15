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

#ifndef GUCEF_MATH_BATCHKERNELSSIMD_H
#define GUCEF_MATH_BATCHKERNELSSIMD_H

/*
 *  Private to the module: the SIMD batch kernels written once over a lane policy L (the members of CMathSse2, see
 *  gucefMATH_CTVector4Ops_Sse2.h; CMathNeon and CMathWasm128 have the same), instantiated by the SSE2, NEON and WASM128
 *  provider files. Each output lane is computed with the operations of the scalar element operation in the same order
 *  (canonical order, gucefMATH_CTMatrix4Ops.h and gucefMATH_BatchKernels.h); tails and lanes outside a fast path call
 *  the scalar element operations. Elements are read with 16 byte loads where the 4 bytes after an element are still
 *  inside the array (every element but the last one), and written with exact 12 or 16 byte stores, so strided data
 *  between elements is never touched. All inputs of a block are read before its outputs are written (in place).
 *  No fused multiply-add: the module is compiled with floating-point contraction off.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <float.h>

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_CPERLINNOISE_H
#include "gucefMATH_CPerlinNoise.h"
#define GUCEF_MATH_CPERLINNOISE_H
#endif /* GUCEF_MATH_CPERLINNOISE_H ? */

#include "gucefMATH_NoiseKernels.h"
#include "gucefMATH_BatchKernels.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

template < class L >
class CMathBatchSimd
{
    public:

    typedef typename L::TVec TVec;
    typedef CTMathComponentTraits< Float32 >::TKernels TKernels;

    /*-------------------------------------------------------------------------*/

    static GUCEF_MATH_FORCE_INLINE TVec
    LoadElement3( const UInt8* p, bool last ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !last )
            return L::Load4( reinterpret_cast< const Float32* >( p ) );
        Float32 t[ 4 ];
        memcpy( t, p, 3 * sizeof( Float32 ) );
        t[ 3 ] = 0.0f;
        return L::Load4( t );
    }

    static GUCEF_MATH_FORCE_INLINE void
    StoreElement3( UInt8* p, TVec v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        Float32 t[ 4 ];
        L::Store4( t, v );
        memcpy( p, t, 3 * sizeof( Float32 ) );
    }

    static GUCEF_MATH_FORCE_INLINE void
    StoreLanes3( TVec v, Float32 ( &out )[ 3 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        Float32 t[ 4 ];
        L::Store4( t, v );
        out[ 0 ] = t[ 0 ]; out[ 1 ] = t[ 1 ]; out[ 2 ] = t[ 2 ];
    }

    /** The lanes whose squared length is in the safe range of TKernels::IsSafeSquaredLength, as a 4 bit mask */
    static GUCEF_MATH_FORCE_INLINE int
    SafeSquaredLengthMask( TVec squared ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const TVec minimum = L::Set1( TKernels::SafeSquaredLengthMinimum() );
        const TVec maximum = L::Set1( FLT_MAX );
        return L::MoveMask( L::And( L::CmpGE( squared, minimum ), L::CmpLE( squared, maximum ) ) );
    }

    /** ( x x + y y ) + z z per lane: the order of the 3 component dot product */
    static GUCEF_MATH_FORCE_INLINE TVec
    SquaredLength3( TVec x, TVec y, TVec z ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_FP_CONTRACT_OFF
        return L::Add( L::Add( L::Mul( x, x ), L::Mul( y, y ) ), L::Mul( z, z ) );
    }

    /*-------------------------------------------------------------------------*/

    template < int W >
    static TMathStatus
    Transform3( void* /* context */, const SMathTransformPoints3F32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const Float32* m = args.matrix->GetData();
        const TVec c0 = L::Load4( m );
        const TVec c1 = L::Load4( m + 4 );
        const TVec c2 = L::Load4( m + 8 );
        const TVec c3 = L::Load4( m + 12 );
        const TVec w = L::Set1( (Float32) W );
        const UInt8* in = static_cast< const UInt8* >( args.in );
        UInt8* out = static_cast< UInt8* >( args.out );
        const UInt32 count = args.count;
        for ( UInt32 i=0; i<count; ++i )
        {
            const TVec v = LoadElement3( in + (size_t) i * args.inStrideBytes, i + 1 == count );
            const TVec r = L::Combine4( c0, c1, c2, c3, L::template Splat< 0 >( v ), L::template Splat< 1 >( v ), L::template Splat< 2 >( v ), w );
            StoreElement3( out + (size_t) i * args.outStrideBytes, r );
        }
        return MATHSTATUS_OK;
    }

    /*-------------------------------------------------------------------------*/

    static TMathStatus
    TransformVectors4( void* /* context */, const SMathTransformVectors4F32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const Float32* m = args.matrix->GetData();
        const TVec c0 = L::Load4( m );
        const TVec c1 = L::Load4( m + 4 );
        const TVec c2 = L::Load4( m + 8 );
        const TVec c3 = L::Load4( m + 12 );
        const UInt8* in = static_cast< const UInt8* >( args.in );
        UInt8* out = static_cast< UInt8* >( args.out );
        for ( UInt32 i=0; i<args.count; ++i )
        {
            const TVec v = L::Load4( reinterpret_cast< const Float32* >( in + (size_t) i * args.inStrideBytes ) );
            const TVec r = L::Combine4( c0, c1, c2, c3, L::template Splat< 0 >( v ), L::template Splat< 1 >( v ), L::template Splat< 2 >( v ), L::template Splat< 3 >( v ) );
            L::Store4( reinterpret_cast< Float32* >( out + (size_t) i * args.outStrideBytes ), r );
        }
        return MATHSTATUS_OK;
    }

    /*-------------------------------------------------------------------------*/

    static TMathStatus
    TransformNormals3( void* /* context */, const SMathTransformNormals3F32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_FP_CONTRACT_OFF
        SMathBatchNormalMatrix n;
        MathBatchComputeNormalMatrix( *args.matrix, n );
        const TVec k0 = L::Set4( n.columns[ 0 ][ 0 ], n.columns[ 0 ][ 1 ], n.columns[ 0 ][ 2 ], 0.0f );
        const TVec k1 = L::Set4( n.columns[ 1 ][ 0 ], n.columns[ 1 ][ 1 ], n.columns[ 1 ][ 2 ], 0.0f );
        const TVec k2 = L::Set4( n.columns[ 2 ][ 0 ], n.columns[ 2 ][ 1 ], n.columns[ 2 ][ 2 ], 0.0f );
        const TVec sign = n.negate ? L::Set1( -0.0f ) : L::Zero();
        const UInt8* in = static_cast< const UInt8* >( args.in );
        UInt8* out = static_cast< UInt8* >( args.out );
        const UInt32 count = args.count;
        UInt32 i = 0;
        for ( ; i + 4 <= count; i += 4 )
        {
            TVec t[ 4 ];
            for ( UInt32 k=0; k<4; ++k )
            {
                const TVec v = LoadElement3( in + (size_t) ( i + k ) * args.inStrideBytes, i + k + 1 == count );
                const TVec r = L::Add( L::Add( L::Mul( k0, L::template Splat< 0 >( v ) ), L::Mul( k1, L::template Splat< 1 >( v ) ) ),
                                       L::Mul( k2, L::template Splat< 2 >( v ) ) );
                t[ k ] = L::Xor( r, sign );
            }
            TVec x = t[ 0 ], y = t[ 1 ], z = t[ 2 ], w = t[ 3 ];
            L::Transpose4( x, y, z, w );
            const TVec squared = SquaredLength3( x, y, z );
            const int safe = SafeSquaredLengthMask( squared );
            const TVec length = L::Sqrt( squared );
            TVec ux = L::Div( x, length ), uy = L::Div( y, length ), uz = L::Div( z, length ), uw = L::Zero();
            L::Transpose4( ux, uy, uz, uw );
            const TVec units[ 4 ] = { ux, uy, uz, uw };
            for ( UInt32 k=0; k<4; ++k )
            {
                UInt8* o = out + (size_t) ( i + k ) * args.outStrideBytes;
                if ( 0 != ( safe & ( 1 << k ) ) )
                {
                    StoreElement3( o, units[ k ] );
                }
                else
                {
                    Float32 transformed[ 3 ];
                    StoreLanes3( t[ k ], transformed );
                    MathBatchFinishNormalElement( transformed, in + (size_t) ( i + k ) * args.inStrideBytes, o );
                }
            }
        }
        for ( ; i<count; ++i )
            MathBatchTransformNormalElement( n, in + (size_t) i * args.inStrideBytes, out + (size_t) i * args.outStrideBytes );
        return MATHSTATUS_OK;
    }

    /*-------------------------------------------------------------------------*/

    static TMathStatus
    MultiplyMatrices4( void* /* context */, const SMathMultiplyMatrices4F32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const UInt8* a = static_cast< const UInt8* >( args.a );
        const UInt8* b = static_cast< const UInt8* >( args.b );
        UInt8* out = static_cast< UInt8* >( args.out );
        for ( UInt32 i=0; i<args.count; ++i )
        {
            const Float32* pa = reinterpret_cast< const Float32* >( a + (size_t) i * args.aStrideBytes );
            const Float32* pb = reinterpret_cast< const Float32* >( b + (size_t) i * args.bStrideBytes );
            Float32* po = reinterpret_cast< Float32* >( out + (size_t) i * args.outStrideBytes );
            const TVec a0 = L::Load4( pa );
            const TVec a1 = L::Load4( pa + 4 );
            const TVec a2 = L::Load4( pa + 8 );
            const TVec a3 = L::Load4( pa + 12 );
            const TVec b0 = L::Load4( pb );
            const TVec b1 = L::Load4( pb + 4 );
            const TVec b2 = L::Load4( pb + 8 );
            const TVec b3 = L::Load4( pb + 12 );
            const TVec r0 = L::Combine4( a0, a1, a2, a3, L::template Splat< 0 >( b0 ), L::template Splat< 1 >( b0 ), L::template Splat< 2 >( b0 ), L::template Splat< 3 >( b0 ) );
            const TVec r1 = L::Combine4( a0, a1, a2, a3, L::template Splat< 0 >( b1 ), L::template Splat< 1 >( b1 ), L::template Splat< 2 >( b1 ), L::template Splat< 3 >( b1 ) );
            const TVec r2 = L::Combine4( a0, a1, a2, a3, L::template Splat< 0 >( b2 ), L::template Splat< 1 >( b2 ), L::template Splat< 2 >( b2 ), L::template Splat< 3 >( b2 ) );
            const TVec r3 = L::Combine4( a0, a1, a2, a3, L::template Splat< 0 >( b3 ), L::template Splat< 1 >( b3 ), L::template Splat< 2 >( b3 ), L::template Splat< 3 >( b3 ) );
            L::Store4( po, r0 );
            L::Store4( po + 4, r1 );
            L::Store4( po + 8, r2 );
            L::Store4( po + 12, r3 );
        }
        return MATHSTATUS_OK;
    }

    /*-------------------------------------------------------------------------*/

    /**
     *  Four quaternions at a time in structure of arrays form, the element formulas of CTMatrix3D::FromQuaternion:
     *  n = ( x x + z z ) + ( y y + w w ), s = 2 / n, each element one 2 term dot product; a block with a quaternion whose n
     *  is not positive (zero, NaN: the identity) goes through the scalar element operation
     */
    static TMathStatus
    QuaternionsToMatrices( void* /* context */, const SMathQuaternionsToMatricesF32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_FP_CONTRACT_OFF
        const UInt8* in = static_cast< const UInt8* >( args.in );
        UInt8* out = static_cast< UInt8* >( args.out );
        const UInt32 count = args.count;
        const TVec zero = L::Zero();
        const TVec one = L::Set1( 1.0f );
        const TVec two = L::Set1( 2.0f );
        const TVec signBit = L::Set1( -0.0f );
        const TVec column3 = L::Set4( 0.0f, 0.0f, 0.0f, 1.0f );
        UInt32 i = 0;
        for ( ; i + 4 <= count; i += 4 )
        {
            TVec x = L::Load4( reinterpret_cast< const Float32* >( in + (size_t) i * args.inStrideBytes ) );
            TVec y = L::Load4( reinterpret_cast< const Float32* >( in + (size_t) ( i + 1 ) * args.inStrideBytes ) );
            TVec z = L::Load4( reinterpret_cast< const Float32* >( in + (size_t) ( i + 2 ) * args.inStrideBytes ) );
            TVec w = L::Load4( reinterpret_cast< const Float32* >( in + (size_t) ( i + 3 ) * args.inStrideBytes ) );
            L::Transpose4( x, y, z, w );
            const TVec n = L::Add( L::Add( L::Mul( x, x ), L::Mul( z, z ) ), L::Add( L::Mul( y, y ), L::Mul( w, w ) ) );
            if ( 0x0F != L::MoveMask( L::CmpLT( zero, n ) ) )
            {
                for ( UInt32 k=0; k<4; ++k )
                    MathBatchQuaternionToMatrixElement( in + (size_t) ( i + k ) * args.inStrideBytes, out + (size_t) ( i + k ) * args.outStrideBytes );
                continue;
            }
            const TVec s = L::Div( two, n );
            const TVec sx = L::Mul( s, x );
            const TVec sy = L::Mul( s, y );
            const TVec sz = L::Mul( s, z );
            const TVec negSx = L::Xor( sx, signBit );
            const TVec negSy = L::Xor( sy, signBit );
            const TVec negSz = L::Xor( sz, signBit );
            TVec m0 = L::Sub( one, L::Add( L::Mul( sy, y ), L::Mul( sz, z ) ) );
            TVec m4 = L::Sub( one, L::Add( L::Mul( sx, x ), L::Mul( sz, z ) ) );
            TVec m8 = L::Sub( one, L::Add( L::Mul( sx, x ), L::Mul( sy, y ) ) );
            TVec m3 = L::Add( L::Mul( sx, y ), L::Mul( negSz, w ) );
            TVec m1 = L::Add( L::Mul( sx, y ), L::Mul( sz, w ) );
            TVec m6 = L::Add( L::Mul( sx, z ), L::Mul( sy, w ) );
            TVec m2 = L::Add( L::Mul( sx, z ), L::Mul( negSy, w ) );
            TVec m7 = L::Add( L::Mul( sy, z ), L::Mul( negSx, w ) );
            TVec m5 = L::Add( L::Mul( sy, z ), L::Mul( sx, w ) );
            TVec pad0 = zero, pad1 = zero, pad2 = zero;
            L::Transpose4( m0, m1, m2, pad0 );      // column 0 of the matrices 0..3
            L::Transpose4( m3, m4, m5, pad1 );      // column 1
            L::Transpose4( m6, m7, m8, pad2 );      // column 2
            const TVec col0[ 4 ] = { m0, m1, m2, pad0 };
            const TVec col1[ 4 ] = { m3, m4, m5, pad1 };
            const TVec col2[ 4 ] = { m6, m7, m8, pad2 };
            for ( UInt32 k=0; k<4; ++k )
            {
                Float32* o = reinterpret_cast< Float32* >( out + (size_t) ( i + k ) * args.outStrideBytes );
                L::Store4( o, col0[ k ] );
                L::Store4( o + 4, col1[ k ] );
                L::Store4( o + 8, col2[ k ] );
                L::Store4( o + 12, column3 );
            }
        }
        for ( ; i<count; ++i )
            MathBatchQuaternionToMatrixElement( in + (size_t) i * args.inStrideBytes, out + (size_t) i * args.outStrideBytes );
        return MATHSTATUS_OK;
    }

    /*-------------------------------------------------------------------------*/

    static TMathStatus
    DotProducts3( void* /* context */, const SMathDotProducts3F32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_FP_CONTRACT_OFF
        const UInt8* a = static_cast< const UInt8* >( args.a );
        const UInt8* b = static_cast< const UInt8* >( args.b );
        UInt8* out = static_cast< UInt8* >( args.out );
        const UInt32 count = args.count;
        UInt32 i = 0;
        for ( ; i + 4 <= count; i += 4 )
        {
            TVec ax = LoadElement3( a + (size_t) i * args.aStrideBytes, false );
            TVec ay = LoadElement3( a + (size_t) ( i + 1 ) * args.aStrideBytes, false );
            TVec az = LoadElement3( a + (size_t) ( i + 2 ) * args.aStrideBytes, false );
            TVec aw = LoadElement3( a + (size_t) ( i + 3 ) * args.aStrideBytes, i + 4 == count );
            TVec bx = LoadElement3( b + (size_t) i * args.bStrideBytes, false );
            TVec by = LoadElement3( b + (size_t) ( i + 1 ) * args.bStrideBytes, false );
            TVec bz = LoadElement3( b + (size_t) ( i + 2 ) * args.bStrideBytes, false );
            TVec bw = LoadElement3( b + (size_t) ( i + 3 ) * args.bStrideBytes, i + 4 == count );
            L::Transpose4( ax, ay, az, aw );
            L::Transpose4( bx, by, bz, bw );
            const TVec d = L::Add( L::Add( L::Mul( ax, bx ), L::Mul( ay, by ) ), L::Mul( az, bz ) );
            Float32 lanes[ 4 ];
            L::Store4( lanes, d );
            for ( UInt32 k=0; k<4; ++k )
                memcpy( out + (size_t) ( i + k ) * args.outStrideBytes, &lanes[ k ], sizeof( Float32 ) );
        }
        for ( ; i<count; ++i )
            MathBatchDotElement3( a + (size_t) i * args.aStrideBytes, b + (size_t) i * args.bStrideBytes, out + (size_t) i * args.outStrideBytes );
        return MATHSTATUS_OK;
    }

    /*-------------------------------------------------------------------------*/

    static TMathStatus
    NormalizeVectors3( void* /* context */, const SMathNormalizeVectors3F32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const UInt8* in = static_cast< const UInt8* >( args.in );
        UInt8* out = static_cast< UInt8* >( args.out );
        const UInt32 count = args.count;
        UInt32 i = 0;
        for ( ; i + 4 <= count; i += 4 )
        {
            TVec x = LoadElement3( in + (size_t) i * args.inStrideBytes, false );
            TVec y = LoadElement3( in + (size_t) ( i + 1 ) * args.inStrideBytes, false );
            TVec z = LoadElement3( in + (size_t) ( i + 2 ) * args.inStrideBytes, false );
            TVec w = LoadElement3( in + (size_t) ( i + 3 ) * args.inStrideBytes, i + 4 == count );
            L::Transpose4( x, y, z, w );
            const TVec squared = SquaredLength3( x, y, z );
            const int safe = SafeSquaredLengthMask( squared );
            const TVec length = L::Sqrt( squared );
            TVec ux = L::Div( x, length ), uy = L::Div( y, length ), uz = L::Div( z, length ), uw = L::Zero();
            L::Transpose4( ux, uy, uz, uw );
            const TVec units[ 4 ] = { ux, uy, uz, uw };
            for ( UInt32 k=0; k<4; ++k )
            {
                UInt8* o = out + (size_t) ( i + k ) * args.outStrideBytes;
                if ( 0 != ( safe & ( 1 << k ) ) )
                    StoreElement3( o, units[ k ] );
                else
                    MathBatchNormalizeElement3( in + (size_t) ( i + k ) * args.inStrideBytes, o );
            }
        }
        for ( ; i<count; ++i )
            MathBatchNormalizeElement3( in + (size_t) i * args.inStrideBytes, out + (size_t) i * args.outStrideBytes );
        return MATHSTATUS_OK;
    }

    /*-------------------------------------------------------------------------*/

    /**
     *  Four sources at a time. Per source the arc, sin and cos of the position are found as ComputeGains does (the
     *  libm calls stay scalar, they are the same calls), the rest of ComputeGains (the 2 x 2 transform, the clamp to 0,
     *  the power, the scale by the square root) runs in SIMD, per lane in the same order. A source that is not in the
     *  plain VBAP case, and every source when the target power is not positive, goes through ComputeGains.
     */
    static TMathStatus
    VbapGains2D( void* /* context */, const SMathVbapGains2DF32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_FP_CONTRACT_OFF
        const TFloat32Vbap2D& vbap = *args.vbap;
        const UInt32 speakers = vbap.GetSpeakerCount();
        if ( 0 == speakers )
            return MATHSTATUS_OK;
        const UInt8* azimuths = static_cast< const UInt8* >( args.azimuths );
        UInt8* gains = static_cast< UInt8* >( args.gains );
        const UInt32 count = args.count;
        UInt32 i = 0;
        if ( 0.0f < args.targetPower )
        {
            const TVec zero = L::Zero();
            const TVec one = L::Set1( 1.0f );
            const TVec targetPower = L::Set1( args.targetPower );
            for ( ; i + 4 <= count; i += 4 )
            {
                Float32 s[ 4 ], c[ 4 ], m0[ 4 ], m1[ 4 ], m2[ 4 ], m3[ 4 ];
                UInt32 arc[ 4 ];
                bool plain[ 4 ];
                for ( UInt32 k=0; k<4; ++k )
                {
                    Float32 azimuth;
                    memcpy( &azimuth, azimuths + (size_t) ( i + k ) * args.azimuthStrideBytes, sizeof( azimuth ) );
                    arc[ k ] = 0;
                    s[ k ] = c[ k ] = 0.0f;
                    plain[ k ] = SMathVbapBatchAccess::Locate( vbap, azimuth, arc[ k ], s[ k ], c[ k ] );
                    if ( plain[ k ] )
                    {
                        m0[ k ] = SMathVbapBatchAccess::GetInverse( vbap, arc[ k ], 0 );
                        m1[ k ] = SMathVbapBatchAccess::GetInverse( vbap, arc[ k ], 1 );
                        m2[ k ] = SMathVbapBatchAccess::GetInverse( vbap, arc[ k ], 2 );
                        m3[ k ] = SMathVbapBatchAccess::GetInverse( vbap, arc[ k ], 3 );
                    }
                    else
                    {
                        m0[ k ] = m1[ k ] = m2[ k ] = m3[ k ] = 0.0f;
                    }
                }
                const TVec vs = L::Load4( s );
                const TVec vc = L::Load4( c );
                const TVec gx = L::Add( L::Mul( L::Load4( m0 ), vs ), L::Mul( L::Load4( m2 ), vc ) );
                const TVec gy = L::Add( L::Mul( L::Load4( m1 ), vs ), L::Mul( L::Load4( m3 ), vc ) );
                TVec g1 = L::Select( L::CmpLT( gx, zero ), zero, gx );
                TVec g2 = L::Select( L::CmpLT( gy, zero ), zero, gy );
                const TVec power = L::Add( L::Mul( g1, g1 ), L::Mul( g2, g2 ) );
                const typename L::TMask positive = L::CmpLT( zero, power );
                g1 = L::Select( positive, g1, one );
                g2 = L::Select( positive, g2, zero );
                const TVec scale = L::Sqrt( L::Div( targetPower, L::Select( positive, power, one ) ) );
                Float32 r1[ 4 ], r2[ 4 ];
                L::Store4( r1, L::Mul( g1, scale ) );
                L::Store4( r2, L::Mul( g2, scale ) );
                for ( UInt32 k=0; k<4; ++k )
                {
                    UInt8* o = gains + (size_t) ( i + k ) * args.gainsStrideBytes;
                    if ( plain[ k ] )
                    {
                        Float32 g[ TFloat32Vbap2D::MaxSpeakers ];
                        for ( UInt32 j=0; j<speakers; ++j )
                            g[ j ] = 0.0f;
                        g[ SMathVbapBatchAccess::GetFirstSpeaker( vbap, arc[ k ] ) ] = r1[ k ];
                        g[ SMathVbapBatchAccess::GetSecondSpeaker( vbap, arc[ k ] ) ] = r2[ k ];
                        memcpy( o, g, (size_t) speakers * sizeof( Float32 ) );
                    }
                    else
                    {
                        MathBatchVbapElement( vbap, azimuths + (size_t) ( i + k ) * args.azimuthStrideBytes, args.targetPower, o );
                    }
                }
            }
        }
        for ( ; i<count; ++i )
            MathBatchVbapElement( vbap, azimuths + (size_t) i * args.azimuthStrideBytes, args.targetPower, gains + (size_t) i * args.gainsStrideBytes );
        return MATHSTATUS_OK;
    }

    /*-------------------------------------------------------------------------*/

    /** The interpolation weight of the improved noise, SmootherStep( 0, 1, t ) of NoiseKernels::Fade: clamp to [ 0, 1 ], then t t t ( t ( 6 t - 15 ) + 10 ) */
    static GUCEF_MATH_FORCE_INLINE TVec
    Fade( TVec x ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_FP_CONTRACT_OFF
        const TVec zero = L::Zero();
        const TVec one = L::Set1( 1.0f );
        const TVec t = L::Select( L::CmpLT( x, zero ), zero, L::Select( L::CmpLT( one, x ), one, x ) );
        const TVec tt = L::Mul( L::Mul( t, t ), t );
        const TVec inner = L::Add( L::Mul( t, L::Sub( L::Mul( t, L::Set1( 6.0f ) ), L::Set1( 15.0f ) ) ), L::Set1( 10.0f ) );
        return L::Mul( tt, inner );
    }

    /** a + ( b - a ) t */
    static GUCEF_MATH_FORCE_INLINE TVec
    Lerp( TVec a, TVec b, TVec t ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return L::Add( a, L::Mul( L::Sub( b, a ), t ) );
    }

    /** v > 1 ? 1 : ( v < -1 ? -1 : v ) */
    static GUCEF_MATH_FORCE_INLINE TVec
    Clamp1( TVec v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const TVec one = L::Set1( 1.0f );
        const TVec minusOne = L::Set1( -1.0f );
        return L::Select( L::CmpGT( v, one ), one, L::Select( L::CmpLT( v, minusOne ), minusOne, v ) );
    }

    /*
     *  Perlin noise ( CPerlinNoise, the scale factors are those of gucefMATH_CPerlinNoise.cpp ): per lane the lattice
     *  cell, the permutation lookups and the gradients are scalar ( no gathers in the instruction sets ), the
     *  interpolation arithmetic is SIMD in the order of Perlin2D / Perlin3D.
     */

    static TMathStatus
    NoiseField2D( void* /* context */, const SMathNoiseField2DF32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_FP_CONTRACT_OFF
        if ( MATHNOISE_PERLIN != args.noiseType )
        {
            MathBatchNoiseField2DScalar( args );
            return MATHSTATUS_OK;
        }
        if ( 0 == args.count || 0 == args.width )
            return MATHSTATUS_OK;
        const UInt8* p = static_cast< const CPerlinNoise* >( args.noise )->GetPermutationTable();
        const TVec one = L::Set1( 1.0f );
        const TVec scale = L::Set1( (Float32) 1.41421356237309504880 );
        UInt32 ix = args.baseItem % args.width;
        UInt32 iy = args.baseItem / args.width;
        UInt32 i = 0;
        for ( ; i + 4 <= args.count; i += 4 )
        {
            Float32 xf[ 4 ], yf[ 4 ];
            Float32 g[ 8 ][ 4 ];
            for ( UInt32 k=0; k<4; ++k )
            {
                const Float32 x = args.originX + (Float32) ix * args.stepX;
                const Float32 y = args.originY + (Float32) iy * args.stepY;
                if ( ++ix == args.width )
                {
                    ix = 0;
                    ++iy;
                }
                const Int32 xi = NoiseKernels::FloorToLattice( x );
                const Int32 yi = NoiseKernels::FloorToLattice( y );
                xf[ k ] = x - (Float32) xi;
                yf[ k ] = y - (Float32) yi;
                const UInt32 x0 = (UInt32) xi & 255U;
                const UInt32 y0 = (UInt32) yi & 255U;
                const UInt32 x1 = ( x0 + 1U ) & 255U;
                const UInt32 y1 = ( y0 + 1U ) & 255U;
                const UInt32 hashes[ 4 ] = { p[ p[ x0 ] + y0 ], p[ p[ x1 ] + y0 ], p[ p[ x0 ] + y1 ], p[ p[ x1 ] + y1 ] };      // 00 10 01 11
                for ( UInt32 c=0; c<4; ++c )
                {
                    const Float64* gradient = NoiseKernels::GetGradients2D( hashes[ c ] );
                    g[ 2 * c ][ k ] = (Float32) gradient[ 0 ];
                    g[ 2 * c + 1 ][ k ] = (Float32) gradient[ 1 ];
                }
            }
            const TVec fx = L::Load4( xf );
            const TVec fy = L::Load4( yf );
            const TVec fx1 = L::Sub( fx, one );
            const TVec fy1 = L::Sub( fy, one );
            const TVec n00 = L::Add( L::Mul( L::Load4( g[ 0 ] ), fx ), L::Mul( L::Load4( g[ 1 ] ), fy ) );
            const TVec n10 = L::Add( L::Mul( L::Load4( g[ 2 ] ), fx1 ), L::Mul( L::Load4( g[ 3 ] ), fy ) );
            const TVec n01 = L::Add( L::Mul( L::Load4( g[ 4 ] ), fx ), L::Mul( L::Load4( g[ 5 ] ), fy1 ) );
            const TVec n11 = L::Add( L::Mul( L::Load4( g[ 6 ] ), fx1 ), L::Mul( L::Load4( g[ 7 ] ), fy1 ) );
            const TVec u = Fade( fx );
            const TVec v = Fade( fy );
            const TVec result = Clamp1( L::Mul( scale, Lerp( Lerp( n00, n10, u ), Lerp( n01, n11, u ), v ) ) );
            Float32 lanes[ 4 ];
            L::Store4( lanes, result );
            memcpy( args.out + i, lanes, sizeof( lanes ) );
        }
        if ( i < args.count )
        {
            SMathNoiseField2DF32Args rest = args;
            rest.baseItem = args.baseItem + i;
            rest.out = args.out + i;
            rest.count = args.count - i;
            MathBatchNoiseField2DScalar( rest );
        }
        return MATHSTATUS_OK;
    }

    /*-------------------------------------------------------------------------*/

    static TMathStatus
    NoiseField3D( void* /* context */, const SMathNoiseField3DF32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_FP_CONTRACT_OFF
        if ( MATHNOISE_PERLIN != args.noiseType )
        {
            MathBatchNoiseField3DScalar( args );
            return MATHSTATUS_OK;
        }
        if ( 0 == args.count || 0 == args.width || 0 == args.height )
            return MATHSTATUS_OK;
        const UInt8* p = static_cast< const CPerlinNoise* >( args.noise )->GetPermutationTable();
        const TVec one = L::Set1( 1.0f );
        const TVec scale = L::Set1( (Float32) 0.9649 );
        const UInt64 plane = (UInt64) args.width * args.height;
        UInt32 ix = args.baseItem % args.width;
        UInt32 iy = (UInt32) ( ( args.baseItem / args.width ) % args.height );
        UInt32 iz = (UInt32) ( args.baseItem / plane );
        UInt32 i = 0;
        for ( ; i + 4 <= args.count; i += 4 )
        {
            Float32 xf[ 4 ], yf[ 4 ], zf[ 4 ];
            Float32 g[ 8 ][ 3 ][ 4 ];
            for ( UInt32 k=0; k<4; ++k )
            {
                const Float32 x = args.originX + (Float32) ix * args.stepX;
                const Float32 y = args.originY + (Float32) iy * args.stepY;
                const Float32 z = args.originZ + (Float32) iz * args.stepZ;
                if ( ++ix == args.width )
                {
                    ix = 0;
                    if ( ++iy == args.height )
                    {
                        iy = 0;
                        ++iz;
                    }
                }
                const Int32 xi = NoiseKernels::FloorToLattice( x );
                const Int32 yi = NoiseKernels::FloorToLattice( y );
                const Int32 zi = NoiseKernels::FloorToLattice( z );
                xf[ k ] = x - (Float32) xi;
                yf[ k ] = y - (Float32) yi;
                zf[ k ] = z - (Float32) zi;
                const UInt32 x0 = (UInt32) xi & 255U;
                const UInt32 y0 = (UInt32) yi & 255U;
                const UInt32 z0 = (UInt32) zi & 255U;
                const UInt32 x1 = ( x0 + 1U ) & 255U;
                const UInt32 y1 = ( y0 + 1U ) & 255U;
                const UInt32 z1 = ( z0 + 1U ) & 255U;
                const UInt32 a0 = p[ x0 ];
                const UInt32 a1 = p[ x1 ];
                const UInt32 a00 = p[ a0 + y0 ], a01 = p[ a0 + y1 ], a10 = p[ a1 + y0 ], a11 = p[ a1 + y1 ];
                // corners in the order 000 100 010 110 001 101 011 111 ( x fastest )
                const UInt32 hashes[ 8 ] = { p[ a00 + z0 ], p[ a10 + z0 ], p[ a01 + z0 ], p[ a11 + z0 ], p[ a00 + z1 ], p[ a10 + z1 ], p[ a01 + z1 ], p[ a11 + z1 ] };
                for ( UInt32 c=0; c<8; ++c )
                {
                    const Int8* gradient = NoiseKernels::GetGradients3D( hashes[ c ] );
                    g[ c ][ 0 ][ k ] = (Float32) gradient[ 0 ];
                    g[ c ][ 1 ][ k ] = (Float32) gradient[ 1 ];
                    g[ c ][ 2 ][ k ] = (Float32) gradient[ 2 ];
                }
            }
            const TVec fx = L::Load4( xf );
            const TVec fy = L::Load4( yf );
            const TVec fz = L::Load4( zf );
            const TVec fx1 = L::Sub( fx, one );
            const TVec fy1 = L::Sub( fy, one );
            const TVec fz1 = L::Sub( fz, one );
            const TVec dx[ 8 ] = { fx, fx1, fx, fx1, fx, fx1, fx, fx1 };
            const TVec dy[ 8 ] = { fy, fy, fy1, fy1, fy, fy, fy1, fy1 };
            const TVec dz[ 8 ] = { fz, fz, fz, fz, fz1, fz1, fz1, fz1 };
            TVec n[ 8 ];
            for ( UInt32 c=0; c<8; ++c )
                n[ c ] = L::Add( L::Add( L::Mul( L::Load4( g[ c ][ 0 ] ), dx[ c ] ), L::Mul( L::Load4( g[ c ][ 1 ] ), dy[ c ] ) ), L::Mul( L::Load4( g[ c ][ 2 ] ), dz[ c ] ) );
            const TVec u = Fade( fx );
            const TVec v = Fade( fy );
            const TVec w = Fade( fz );
            const TVec lower = Lerp( Lerp( n[ 0 ], n[ 1 ], u ), Lerp( n[ 2 ], n[ 3 ], u ), v );
            const TVec upper = Lerp( Lerp( n[ 4 ], n[ 5 ], u ), Lerp( n[ 6 ], n[ 7 ], u ), v );
            const TVec result = Clamp1( L::Mul( scale, Lerp( lower, upper, w ) ) );
            Float32 lanes[ 4 ];
            L::Store4( lanes, result );
            memcpy( args.out + i, lanes, sizeof( lanes ) );
        }
        if ( i < args.count )
        {
            SMathNoiseField3DF32Args rest = args;
            rest.baseItem = args.baseItem + i;
            rest.out = args.out + i;
            rest.count = args.count - i;
            MathBatchNoiseField3DScalar( rest );
        }
        return MATHSTATUS_OK;
    }

    /*-------------------------------------------------------------------------*/

    /** Fills the functions of the kernels that the lane policy L serves, the others stay as they are */
    static void
    FillKernelSet( SMathBatchKernelSet& set )
    {
        set.functions[ MATHBATCHKERNEL_TRANSFORM_POINTS3_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathTransformPoints3F32Function >( &Transform3< 1 > ) );
        set.functions[ MATHBATCHKERNEL_TRANSFORM_VECTORS3_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathTransformVectors3F32Function >( &Transform3< 0 > ) );
        set.functions[ MATHBATCHKERNEL_TRANSFORM_NORMALS3_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathTransformNormals3F32Function >( &TransformNormals3 ) );
        set.functions[ MATHBATCHKERNEL_TRANSFORM_VECTORS4_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathTransformVectors4F32Function >( &TransformVectors4 ) );
        set.functions[ MATHBATCHKERNEL_MULTIPLY_MATRICES4_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathMultiplyMatrices4F32Function >( &MultiplyMatrices4 ) );
        set.functions[ MATHBATCHKERNEL_QUATERNIONS_TO_MATRICES_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathQuaternionsToMatricesF32Function >( &QuaternionsToMatrices ) );
        set.functions[ MATHBATCHKERNEL_DOT_PRODUCTS3_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathDotProducts3F32Function >( &DotProducts3 ) );
        set.functions[ MATHBATCHKERNEL_NORMALIZE_VECTORS3_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathNormalizeVectors3F32Function >( &NormalizeVectors3 ) );
        set.functions[ MATHBATCHKERNEL_VBAP_GAINS2D_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathVbapGains2DF32Function >( &VbapGains2D ) );
        set.functions[ MATHBATCHKERNEL_NOISE_FIELD2D_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathNoiseField2DF32Function >( &NoiseField2D ) );
        set.functions[ MATHBATCHKERNEL_NOISE_FIELD3D_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathNoiseField3DF32Function >( &NoiseField3D ) );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_BATCHKERNELSSIMD_H ? */
