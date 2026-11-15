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

#include <string.h>

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef CTMatrix4Ops< Float32, SMathSimdScalar > TScalarMatrixOps;
typedef CTMathComponentTraits< Float32 >::TKernels TKernels;

/*-------------------------------------------------------------------------*/

GUCEF_MATH_FORCE_INLINE void
TransformElement3( const Float32 ( &m )[ 16 ], const Float32 w, const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 v[ 4 ];
    memcpy( v, in, 3 * sizeof( Float32 ) );
    v[ 3 ] = w;
    Float32 r[ 4 ];
    TScalarMatrixOps::Transform( m, v, r );
    memcpy( out, r, 3 * sizeof( Float32 ) );
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_FORCE_INLINE void
TransformElement4( const Float32 ( &m )[ 16 ], const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 v[ 4 ];
    memcpy( v, in, 4 * sizeof( Float32 ) );
    Float32 r[ 4 ];
    TScalarMatrixOps::Transform( m, v, r );
    memcpy( out, r, 4 * sizeof( Float32 ) );
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_FORCE_INLINE void
FinishNormalElement( const Float32 ( &t )[ 3 ], const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    // CTDirectionVector3D::TryCreate: a zero or non-finite vector, or one Normalized cannot handle, keeps the input
    const TFloat32Vector3D v( t[ 0 ], t[ 1 ], t[ 2 ] );
    if ( v.IsFinite() && !v.IsZero() )
    {
        const TFloat32Vector3D unit = v.Normalized();
        if ( !unit.IsZero() )
        {
            const Float32 r[ 3 ] = { unit.x, unit.y, unit.z };
            memcpy( out, r, sizeof( r ) );
            return;
        }
    }
    if ( out != in )
        memmove( out, in, 3 * sizeof( Float32 ) );
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_FORCE_INLINE void
TransformNormalElement( const SMathBatchNormalMatrix& n, const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 v[ 3 ];
    memcpy( v, in, sizeof( v ) );
    Float32 t[ 3 ];
    for ( UInt32 r=0; r<3; ++r )
    {
        // row r of the cofactor matrix times v, as CTMatrix3D::Transform
        const Float32 row[ 3 ] = { n.columns[ 0 ][ r ], n.columns[ 1 ][ r ], n.columns[ 2 ][ r ] };
        t[ r ] = TKernels::Dot( row, v );
    }
    if ( n.negate )
    {
        for ( UInt32 r=0; r<3; ++r )
            t[ r ] = -t[ r ];
    }
    FinishNormalElement( t, in, out );
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_FORCE_INLINE void
MultiplyElement( const void* a, const void* b, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 ma[ 16 ];
    Float32 mb[ 16 ];
    memcpy( ma, a, sizeof( ma ) );
    memcpy( mb, b, sizeof( mb ) );
    Float32 r[ 16 ];
    TScalarMatrixOps::Multiply( ma, mb, r );
    memcpy( out, r, sizeof( r ) );
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_FORCE_INLINE void
QuaternionToMatrixElement( const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 q[ 4 ];
    memcpy( q, in, sizeof( q ) );
    const TFloat32Matrix4D m = TFloat32Matrix4D::FromQuaternion( TFloat32Quaternion::FromXYZW( q[ 0 ], q[ 1 ], q[ 2 ], q[ 3 ] ) );
    memcpy( out, m.GetData(), 16 * sizeof( Float32 ) );
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_FORCE_INLINE void
DotElement3( const void* a, const void* b, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 va[ 3 ];
    Float32 vb[ 3 ];
    memcpy( va, a, sizeof( va ) );
    memcpy( vb, b, sizeof( vb ) );
    const Float32 d = TFloat32Vector3D( va[ 0 ], va[ 1 ], va[ 2 ] ).Dot( TFloat32Vector3D( vb[ 0 ], vb[ 1 ], vb[ 2 ] ) );
    memcpy( out, &d, sizeof( d ) );
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_FORCE_INLINE void
NormalizeElement3( const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 v[ 3 ];
    memcpy( v, in, sizeof( v ) );
    const TFloat32Vector3D unit = TFloat32Vector3D( v[ 0 ], v[ 1 ], v[ 2 ] ).Normalized();
    const Float32 r[ 3 ] = { unit.x, unit.y, unit.z };
    memcpy( out, r, sizeof( r ) );
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_FORCE_INLINE void
LoadMatrix( const TFloat32Matrix4D& matrix, Float32 ( &m )[ 16 ] ) GUCEF_MATH_REALTIME_CONTEXT
{
    memcpy( m, matrix.GetData(), sizeof( m ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      SCALAR PROVIDERS                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

template < int W >
TMathStatus
ScalarTransform3( void* /* context */, const SMathTransformPoints3F32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 m[ 16 ];
    LoadMatrix( *args.matrix, m );
    const UInt8* in = static_cast< const UInt8* >( args.in );
    UInt8* out = static_cast< UInt8* >( args.out );
    for ( UInt32 i=0; i<args.count; ++i )
        TransformElement3( m, (Float32) W, in + (size_t) i * args.inStrideBytes, out + (size_t) i * args.outStrideBytes );
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TMathStatus
ScalarTransformNormals3( void* /* context */, const SMathTransformNormals3F32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    SMathBatchNormalMatrix n;
    MathBatchComputeNormalMatrix( *args.matrix, n );
    const UInt8* in = static_cast< const UInt8* >( args.in );
    UInt8* out = static_cast< UInt8* >( args.out );
    for ( UInt32 i=0; i<args.count; ++i )
        TransformNormalElement( n, in + (size_t) i * args.inStrideBytes, out + (size_t) i * args.outStrideBytes );
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TMathStatus
ScalarTransformVectors4( void* /* context */, const SMathTransformVectors4F32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 m[ 16 ];
    LoadMatrix( *args.matrix, m );
    const UInt8* in = static_cast< const UInt8* >( args.in );
    UInt8* out = static_cast< UInt8* >( args.out );
    for ( UInt32 i=0; i<args.count; ++i )
        TransformElement4( m, in + (size_t) i * args.inStrideBytes, out + (size_t) i * args.outStrideBytes );
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TMathStatus
ScalarMultiplyMatrices4( void* /* context */, const SMathMultiplyMatrices4F32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt8* a = static_cast< const UInt8* >( args.a );
    const UInt8* b = static_cast< const UInt8* >( args.b );
    UInt8* out = static_cast< UInt8* >( args.out );
    for ( UInt32 i=0; i<args.count; ++i )
        MultiplyElement( a + (size_t) i * args.aStrideBytes, b + (size_t) i * args.bStrideBytes, out + (size_t) i * args.outStrideBytes );
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TMathStatus
ScalarQuaternionsToMatrices( void* /* context */, const SMathQuaternionsToMatricesF32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt8* in = static_cast< const UInt8* >( args.in );
    UInt8* out = static_cast< UInt8* >( args.out );
    for ( UInt32 i=0; i<args.count; ++i )
        QuaternionToMatrixElement( in + (size_t) i * args.inStrideBytes, out + (size_t) i * args.outStrideBytes );
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TMathStatus
ScalarDotProducts3( void* /* context */, const SMathDotProducts3F32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt8* a = static_cast< const UInt8* >( args.a );
    const UInt8* b = static_cast< const UInt8* >( args.b );
    UInt8* out = static_cast< UInt8* >( args.out );
    for ( UInt32 i=0; i<args.count; ++i )
        DotElement3( a + (size_t) i * args.aStrideBytes, b + (size_t) i * args.bStrideBytes, out + (size_t) i * args.outStrideBytes );
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TMathStatus
ScalarNormalizeVectors3( void* /* context */, const SMathNormalizeVectors3F32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt8* in = static_cast< const UInt8* >( args.in );
    UInt8* out = static_cast< UInt8* >( args.out );
    for ( UInt32 i=0; i<args.count; ++i )
        NormalizeElement3( in + (size_t) i * args.inStrideBytes, out + (size_t) i * args.outStrideBytes );
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

/** Forward: each half is written at or below the bytes of the floats already read, so the same array works in place */
TMathStatus
ScalarConvertF32ToF16( void* /* context */, const SMathConvertF32ToF16Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt8* in = reinterpret_cast< const UInt8* >( args.in );
    UInt8* out = reinterpret_cast< UInt8* >( args.out );
    for ( UInt32 i=0; i<args.count; ++i )
    {
        Float32 value;
        memcpy( &value, in + (size_t) i * sizeof( Float32 ), sizeof( value ) );
        const UInt16 bits = CFloat16::FromFloat32( value ).GetBits();
        memcpy( out + (size_t) i * sizeof( UInt16 ), &bits, sizeof( bits ) );
    }
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_FORCE_INLINE void
ConvertF16ToF32Element( const UInt8* in, UInt8* out, UInt32 i ) GUCEF_MATH_REALTIME_CONTEXT
{
    UInt16 bits;
    memcpy( &bits, in + (size_t) i * sizeof( UInt16 ), sizeof( bits ) );
    const Float32 value = CFloat16::FromBits( bits ).ToFloat32();
    memcpy( out + (size_t) i * sizeof( Float32 ), &value, sizeof( value ) );
}

/** In place the floats are wider than the halves, so the array is converted from its end */
TMathStatus
ScalarConvertF16ToF32( void* /* context */, const SMathConvertF16ToF32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt8* in = reinterpret_cast< const UInt8* >( args.in );
    UInt8* out = reinterpret_cast< UInt8* >( args.out );
    if ( static_cast< const void* >( in ) == static_cast< const void* >( out ) )
    {
        for ( UInt32 i=args.count; i>0; --i )
            ConvertF16ToF32Element( in, out, i - 1 );
    }
    else
    {
        for ( UInt32 i=0; i<args.count; ++i )
            ConvertF16ToF32Element( in, out, i );
    }
    return MATHSTATUS_OK;
}


/*-------------------------------------------------------------------------*/

/** Float64: the canonical order of CTMatrix4Ops, as TFloat64Matrix4D::TransformPoint and TransformVector */
template < int W >
GUCEF_MATH_FORCE_INLINE void
TransformElement3F64( const Float64 ( &m )[ 16 ], const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 v[ 4 ];
    memcpy( v, in, 3 * sizeof( Float64 ) );
    v[ 3 ] = (Float64) W;
    Float64 r[ 4 ];
    CTMatrix4Ops< Float64, SMathSimdScalar >::Transform( m, v, r );
    memcpy( out, r, 3 * sizeof( Float64 ) );
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_FORCE_INLINE void
MultiplyElementF64( const void* a, const void* b, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 ma[ 16 ];
    Float64 mb[ 16 ];
    memcpy( ma, a, sizeof( ma ) );
    memcpy( mb, b, sizeof( mb ) );
    Float64 r[ 16 ];
    CTMatrix4Ops< Float64, SMathSimdScalar >::Multiply( ma, mb, r );
    memcpy( out, r, sizeof( r ) );
}

/*-------------------------------------------------------------------------*/

/** Q16.16: the raw Int32 values of the arrays are the fixed point values */
GUCEF_MATH_FORCE_INLINE void
LoadFixed( const void* in, UInt32 count, TFixed32* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt8* p = static_cast< const UInt8* >( in );
    for ( UInt32 i=0; i<count; ++i )
    {
        Int32 raw;
        memcpy( &raw, p + (size_t) i * sizeof( Int32 ), sizeof( raw ) );
        out[ i ] = TFixed32::FromRaw( raw );
    }
}

GUCEF_MATH_FORCE_INLINE void
StoreFixed( void* out, UInt32 count, const TFixed32* in ) GUCEF_MATH_REALTIME_CONTEXT
{
    UInt8* p = static_cast< UInt8* >( out );
    for ( UInt32 i=0; i<count; ++i )
    {
        const Int32 raw = in[ i ].GetRaw();
        memcpy( p + (size_t) i * sizeof( Int32 ), &raw, sizeof( raw ) );
    }
}

GUCEF_MATH_FORCE_INLINE void
TransformPointElementFx32( const TFixed32 ( &m )[ 16 ], const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    TFixed32 v[ 4 ];
    LoadFixed( in, 3, v );
    v[ 3 ] = CTNumericTraits< TFixed32 >::One();
    TFixed32 r[ 4 ];
    CTMatrix4Ops< TFixed32, SMathSimdScalar >::Transform( m, v, r );
    StoreFixed( out, 3, r );
}

GUCEF_MATH_FORCE_INLINE void
MultiplyElementFx32( const void* a, const void* b, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    TFixed32 ma[ 16 ];
    TFixed32 mb[ 16 ];
    LoadFixed( a, 16, ma );
    LoadFixed( b, 16, mb );
    TFixed32 r[ 16 ];
    CTMatrix4Ops< TFixed32, SMathSimdScalar >::Multiply( ma, mb, r );
    StoreFixed( out, 16, r );
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_FORCE_INLINE Float32
SampleNoise2D( UInt32 type, const void* noise, Float32 x, Float32 y ) GUCEF_MATH_REALTIME_CONTEXT
{
    switch ( type )
    {
        case MATHNOISE_PERLIN: return static_cast< const CPerlinNoise* >( noise )->Noise2D( x, y );
        case MATHNOISE_VALUE: return static_cast< const CValueNoise* >( noise )->Noise2D( x, y );
        case MATHNOISE_OPENSIMPLEX2: return static_cast< const COpenSimplex2Noise* >( noise )->Noise2D( x, y );
        case MATHNOISE_WORLEY: return static_cast< const CWorleyNoise* >( noise )->Noise2D( x, y );
        default: return 0.0f;
    }
}

GUCEF_MATH_FORCE_INLINE Float32
SampleNoise3D( UInt32 type, const void* noise, Float32 x, Float32 y, Float32 z ) GUCEF_MATH_REALTIME_CONTEXT
{
    switch ( type )
    {
        case MATHNOISE_PERLIN: return static_cast< const CPerlinNoise* >( noise )->Noise3D( x, y, z );
        case MATHNOISE_VALUE: return static_cast< const CValueNoise* >( noise )->Noise3D( x, y, z );
        case MATHNOISE_OPENSIMPLEX2: return static_cast< const COpenSimplex2Noise* >( noise )->Noise3D( x, y, z );
        case MATHNOISE_WORLEY: return static_cast< const CWorleyNoise* >( noise )->Noise3D( x, y, z );
        default: return 0.0f;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      SCALAR PROVIDERS OF THE LATER KERNELS                              //
//                                                                         //
//-------------------------------------------------------------------------*/

template < int W >
TMathStatus
ScalarTransform3F64( void* /* context */, const SMathTransformPoints3F64Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 m[ 16 ];
    memcpy( m, args.matrix->GetData(), sizeof( m ) );
    const UInt8* in = static_cast< const UInt8* >( args.in );
    UInt8* out = static_cast< UInt8* >( args.out );
    for ( UInt32 i=0; i<args.count; ++i )
        TransformElement3F64< W >( m, in + (size_t) i * args.inStrideBytes, out + (size_t) i * args.outStrideBytes );
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TMathStatus
ScalarMultiplyMatrices4F64( void* /* context */, const SMathMultiplyMatrices4F64Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt8* a = static_cast< const UInt8* >( args.a );
    const UInt8* b = static_cast< const UInt8* >( args.b );
    UInt8* out = static_cast< UInt8* >( args.out );
    for ( UInt32 i=0; i<args.count; ++i )
        MultiplyElementF64( a + (size_t) i * args.aStrideBytes, b + (size_t) i * args.bStrideBytes, out + (size_t) i * args.outStrideBytes );
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TMathStatus
ScalarTransformPoints3Fx32( void* /* context */, const SMathTransformPoints3Fx32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    TFixed32 m[ 16 ];
    for ( UInt32 i=0; i<16; ++i )
        m[ i ] = args.matrix->GetData()[ i ];
    const UInt8* in = static_cast< const UInt8* >( args.in );
    UInt8* out = static_cast< UInt8* >( args.out );
    for ( UInt32 i=0; i<args.count; ++i )
        TransformPointElementFx32( m, in + (size_t) i * args.inStrideBytes, out + (size_t) i * args.outStrideBytes );
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TMathStatus
ScalarMultiplyMatrices4Fx32( void* /* context */, const SMathMultiplyMatrices4Fx32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt8* a = static_cast< const UInt8* >( args.a );
    const UInt8* b = static_cast< const UInt8* >( args.b );
    UInt8* out = static_cast< UInt8* >( args.out );
    for ( UInt32 i=0; i<args.count; ++i )
        MultiplyElementFx32( a + (size_t) i * args.aStrideBytes, b + (size_t) i * args.bStrideBytes, out + (size_t) i * args.outStrideBytes );
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TMathStatus
ScalarVbapGains2D( void* /* context */, const SMathVbapGains2DF32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == args.vbap->GetSpeakerCount() )
        return MATHSTATUS_OK;
    const UInt8* azimuths = static_cast< const UInt8* >( args.azimuths );
    UInt8* gains = static_cast< UInt8* >( args.gains );
    for ( UInt32 i=0; i<args.count; ++i )
        MathBatchVbapElement( *args.vbap, azimuths + (size_t) i * args.azimuthStrideBytes, args.targetPower, gains + (size_t) i * args.gainsStrideBytes );
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TMathStatus
ScalarNoiseField2D( void* /* context */, const SMathNoiseField2DF32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    MathBatchNoiseField2DScalar( args );
    return MATHSTATUS_OK;
}

TMathStatus
ScalarNoiseField3D( void* /* context */, const SMathNoiseField3DF32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    MathBatchNoiseField3DScalar( args );
    return MATHSTATUS_OK;
}
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
MathBatchComputeNormalMatrix( const TFloat32Matrix4D& m, SMathBatchNormalMatrix& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    const CTMatrix3D< Float32 > linear = m.GetLinear();
    const TFloat32Vector3D c0 = linear.GetColumn( 0 );
    const TFloat32Vector3D c1 = linear.GetColumn( 1 );
    const TFloat32Vector3D c2 = linear.GetColumn( 2 );
    const TFloat32Vector3D k[ 3 ] = { c1.Cross( c2 ), c2.Cross( c0 ), c0.Cross( c1 ) };
    for ( UInt32 c=0; c<3; ++c )
    {
        out.columns[ c ][ 0 ] = k[ c ].x;
        out.columns[ c ][ 1 ] = k[ c ].y;
        out.columns[ c ][ 2 ] = k[ c ].z;
    }
    out.negate = CTNumericTraits< Float32 >::IsNegative( linear.GetDeterminant() );
}

/*-------------------------------------------------------------------------*/

void
MathBatchTransformElement3( const Float32 ( &m )[ 16 ], const Float32 w, const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    TransformElement3( m, w, in, out );
}

/*-------------------------------------------------------------------------*/

void
MathBatchTransformElement4( const Float32 ( &m )[ 16 ], const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    TransformElement4( m, in, out );
}

/*-------------------------------------------------------------------------*/

void
MathBatchTransformNormalElement( const SMathBatchNormalMatrix& n, const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    TransformNormalElement( n, in, out );
}

/*-------------------------------------------------------------------------*/

void
MathBatchFinishNormalElement( const Float32 ( &transformed )[ 3 ], const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    FinishNormalElement( transformed, in, out );
}

/*-------------------------------------------------------------------------*/

void
MathBatchMultiplyElement( const void* a, const void* b, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    MultiplyElement( a, b, out );
}

/*-------------------------------------------------------------------------*/

void
MathBatchQuaternionToMatrixElement( const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    QuaternionToMatrixElement( in, out );
}

/*-------------------------------------------------------------------------*/

void
MathBatchDotElement3( const void* a, const void* b, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    DotElement3( a, b, out );
}

/*-------------------------------------------------------------------------*/

void
MathBatchNormalizeElement3( const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    NormalizeElement3( in, out );
}

/*-------------------------------------------------------------------------*/

void
MathBatchVbapElement( const TFloat32Vbap2D& vbap, const void* azimuth, Float32 targetPower, void* gains ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 value;
    memcpy( &value, azimuth, sizeof( value ) );
    Float32 g[ TFloat32Vbap2D::MaxSpeakers ];
    vbap.ComputeGains( CTRadian< Float32 >( value ), g, targetPower );
    memcpy( gains, g, (size_t) vbap.GetSpeakerCount() * sizeof( Float32 ) );
}

/*-------------------------------------------------------------------------*/

void
MathBatchNoiseField2DScalar( const SMathNoiseField2DF32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == args.count || 0 == args.width )
        return;
    UInt32 ix = args.baseItem % args.width;
    UInt32 iy = args.baseItem / args.width;
    for ( UInt32 i=0; i<args.count; ++i )
    {
        const Float32 x = args.originX + (Float32) ix * args.stepX;
        const Float32 y = args.originY + (Float32) iy * args.stepY;
        args.out[ i ] = SampleNoise2D( args.noiseType, args.noise, x, y );
        if ( ++ix == args.width )
        {
            ix = 0;
            ++iy;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
MathBatchNoiseField3DScalar( const SMathNoiseField3DF32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == args.count || 0 == args.width || 0 == args.height )
        return;
    const UInt64 plane = (UInt64) args.width * args.height;
    UInt32 ix = args.baseItem % args.width;
    UInt32 iy = (UInt32) ( ( args.baseItem / args.width ) % args.height );
    UInt32 iz = (UInt32) ( args.baseItem / plane );
    for ( UInt32 i=0; i<args.count; ++i )
    {
        const Float32 x = args.originX + (Float32) ix * args.stepX;
        const Float32 y = args.originY + (Float32) iy * args.stepY;
        const Float32 z = args.originZ + (Float32) iz * args.stepZ;
        args.out[ i ] = SampleNoise3D( args.noiseType, args.noise, x, y, z );
        if ( ++ix == args.width )
        {
            ix = 0;
            if ( ++iy == args.height )
            {
                iy = 0;
                ++iz;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
MathBatchGetScalarKernels( SMathBatchKernelSet& set )
{
    set.name = "Scalar";
    set.tier = MATHBATCHTIER_SCALAR;
    set.requiredCpuFeatures = 0;
    for ( UInt32 k=0; k<MATHBATCHKERNEL_MATH_COUNT; ++k )
        set.functions[ k ] = GUCEF_NULL;
    set.functions[ MATHBATCHKERNEL_TRANSFORM_POINTS3_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathTransformPoints3F32Function >( &ScalarTransform3< 1 > ) );
    set.functions[ MATHBATCHKERNEL_TRANSFORM_VECTORS3_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathTransformVectors3F32Function >( &ScalarTransform3< 0 > ) );
    set.functions[ MATHBATCHKERNEL_TRANSFORM_NORMALS3_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathTransformNormals3F32Function >( &ScalarTransformNormals3 ) );
    set.functions[ MATHBATCHKERNEL_TRANSFORM_VECTORS4_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathTransformVectors4F32Function >( &ScalarTransformVectors4 ) );
    set.functions[ MATHBATCHKERNEL_MULTIPLY_MATRICES4_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathMultiplyMatrices4F32Function >( &ScalarMultiplyMatrices4 ) );
    set.functions[ MATHBATCHKERNEL_QUATERNIONS_TO_MATRICES_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathQuaternionsToMatricesF32Function >( &ScalarQuaternionsToMatrices ) );
    set.functions[ MATHBATCHKERNEL_DOT_PRODUCTS3_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathDotProducts3F32Function >( &ScalarDotProducts3 ) );
    set.functions[ MATHBATCHKERNEL_NORMALIZE_VECTORS3_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathNormalizeVectors3F32Function >( &ScalarNormalizeVectors3 ) );
    set.functions[ MATHBATCHKERNEL_CONVERT_F32_TO_F16 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathConvertF32ToF16Function >( &ScalarConvertF32ToF16 ) );
    set.functions[ MATHBATCHKERNEL_CONVERT_F16_TO_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathConvertF16ToF32Function >( &ScalarConvertF16ToF32 ) );
    set.functions[ MATHBATCHKERNEL_TRANSFORM_POINTS3_F64 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathTransformPoints3F64Function >( &ScalarTransform3F64< 1 > ) );
    set.functions[ MATHBATCHKERNEL_TRANSFORM_VECTORS3_F64 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathTransformVectors3F64Function >( &ScalarTransform3F64< 0 > ) );
    set.functions[ MATHBATCHKERNEL_MULTIPLY_MATRICES4_F64 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathMultiplyMatrices4F64Function >( &ScalarMultiplyMatrices4F64 ) );
    set.functions[ MATHBATCHKERNEL_TRANSFORM_POINTS3_FX32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathTransformPoints3Fx32Function >( &ScalarTransformPoints3Fx32 ) );
    set.functions[ MATHBATCHKERNEL_MULTIPLY_MATRICES4_FX32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathMultiplyMatrices4Fx32Function >( &ScalarMultiplyMatrices4Fx32 ) );
    set.functions[ MATHBATCHKERNEL_VBAP_GAINS2D_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathVbapGains2DF32Function >( &ScalarVbapGains2D ) );
    set.functions[ MATHBATCHKERNEL_NOISE_FIELD2D_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathNoiseField2DF32Function >( &ScalarNoiseField2D ) );
    set.functions[ MATHBATCHKERNEL_NOISE_FIELD3D_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathNoiseField3DF32Function >( &ScalarNoiseField3D ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
