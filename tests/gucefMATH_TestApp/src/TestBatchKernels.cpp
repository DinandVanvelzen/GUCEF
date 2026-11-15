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
#include <string.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

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

#ifndef GUCEF_MATH_CTVBAP2D_H
#include "gucefMATH_CTVbap2D.h"
#define GUCEF_MATH_CTVBAP2D_H
#endif /* GUCEF_MATH_CTVBAP2D_H ? */

#include "TestMathReference.h"
#include "TestBatchKernels.h"

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

const UInt64 KernelSeed = 0xBA7C4E2A11005ULL;
const UInt8 Sentinel = 0xA5;

/** The counts of acceptance item 2 */
const UInt32 TestCounts[] = { 0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 1023, 100000 };
const UInt32 TestCountCount = sizeof( TestCounts ) / sizeof( TestCounts[ 0 ] );

/** The kinds of data of the arrays of a kernel */
enum EElementKind
{
    ELEMENT_FLOAT32 = 0 ,       /**< Float32 values */
    ELEMENT_FLOAT64 ,           /**< Float64 values */
    ELEMENT_FIXED32 ,           /**< Q16.16 raw values (Int32) */
    ELEMENT_HALF ,              /**< binary16 bit patterns */
    ELEMENT_BITS32              /**< any 32 bit pattern, NaN included */
};

/** The shape of the arrays of a kernel: element sizes in bytes, the strides to test, the kind of input data */
struct SKernelShape
{
    TMathBatchKernel kernel;
    UInt32 inSize;              /**< bytes of one input element */
    UInt32 secondSize;          /**< bytes of one element of the second input, 0 when there is none */
    UInt32 outSize;             /**< bytes of one output element */
    UInt32 element;             /**< EElementKind of the input (and output) values */
    UInt32 strides[ 3 ];        /**< byte strides for the strided runs, applied to input and output; 0 = unused */
};

const SKernelShape KernelShapes[ MATHBATCHKERNEL_MATH_COUNT ] =
{
    { MATHBATCHKERNEL_TRANSFORM_POINTS3_F32, 12, 0, 12, ELEMENT_FLOAT32, { 12, 16, 32 } },
    { MATHBATCHKERNEL_TRANSFORM_VECTORS3_F32, 12, 0, 12, ELEMENT_FLOAT32, { 12, 16, 32 } },
    { MATHBATCHKERNEL_TRANSFORM_NORMALS3_F32, 12, 0, 12, ELEMENT_FLOAT32, { 12, 16, 32 } },
    { MATHBATCHKERNEL_TRANSFORM_VECTORS4_F32, 16, 0, 16, ELEMENT_FLOAT32, { 16, 20, 32 } },
    { MATHBATCHKERNEL_MULTIPLY_MATRICES4_F32, 64, 64, 64, ELEMENT_FLOAT32, { 64, 68, 96 } },
    { MATHBATCHKERNEL_QUATERNIONS_TO_MATRICES_F32, 16, 0, 64, ELEMENT_FLOAT32, { 64, 68, 96 } },
    { MATHBATCHKERNEL_DOT_PRODUCTS3_F32, 12, 12, 4, ELEMENT_FLOAT32, { 12, 16, 32 } },
    { MATHBATCHKERNEL_NORMALIZE_VECTORS3_F32, 12, 0, 12, ELEMENT_FLOAT32, { 12, 16, 32 } },
    { MATHBATCHKERNEL_CONVERT_F32_TO_F16, 4, 0, 2, ELEMENT_BITS32, { 0, 0, 0 } },
    { MATHBATCHKERNEL_CONVERT_F16_TO_F32, 2, 0, 4, ELEMENT_HALF, { 0, 0, 0 } },
    { MATHBATCHKERNEL_TRANSFORM_POINTS3_F64, 24, 0, 24, ELEMENT_FLOAT64, { 24, 32, 40 } },
    { MATHBATCHKERNEL_TRANSFORM_VECTORS3_F64, 24, 0, 24, ELEMENT_FLOAT64, { 24, 32, 40 } },
    { MATHBATCHKERNEL_MULTIPLY_MATRICES4_F64, 128, 128, 128, ELEMENT_FLOAT64, { 128, 136, 160 } },
    { MATHBATCHKERNEL_TRANSFORM_POINTS3_FX32, 12, 0, 12, ELEMENT_FIXED32, { 12, 16, 32 } },
    { MATHBATCHKERNEL_MULTIPLY_MATRICES4_FX32, 64, 64, 64, ELEMENT_FIXED32, { 64, 68, 96 } },
    { MATHBATCHKERNEL_VBAP_GAINS2D_F32, 4, 0, 0, ELEMENT_FLOAT32, { 0, 0, 0 } },            // the output element depends on the layout, see ApplyVariant
    { MATHBATCHKERNEL_NOISE_FIELD2D_F32, 0, 0, 4, ELEMENT_FLOAT32, { 0, 0, 0 } },
    { MATHBATCHKERNEL_NOISE_FIELD3D_F32, 0, 0, 4, ELEMENT_FLOAT32, { 0, 0, 0 } }
};

/** The variants of a kernel the comparison with the scalar provider runs through: VBAP layouts, noise types and parameter sets */
const UInt32 VbapLayoutCount = 4;
const UInt32 NoiseVariantCount = 6;
const Float32 TargetPowers[ 4 ] = { 2.0f, 0.5f, -1.0f, 0.0f };

/** Everything besides the arrays that the kernels take: the matrices, the panners, the noises */
struct STestState
{
    STestState( void )
        : perlin( 123 ), value( 7 ), simplex( 99 ), worley( 5 )
        , layout( 0 ), targetPower( 2.0f ), noiseType( MATHNOISE_PERLIN ), originX( 0.0f ), originY( 0.0f ), originZ( 0.0f )
        , stepX( 0.0f ), stepY( 0.0f ), stepZ( 0.0f ), baseItem( 0 ), nanInputs( false )
    {
    }

    TFloat32Matrix4D matrix;
    TFloat64Matrix4D matrix64;
    TFixed32Matrix4D matrixFx;
    TFloat32Vbap2D vbap[ VbapLayoutCount ];
    CPerlinNoise perlin;
    CValueNoise value;
    COpenSimplex2Noise simplex;
    CWorleyNoise worley;
    UInt32 layout;
    Float32 targetPower;
    UInt32 noiseType;
    Float32 originX, originY, originZ, stepX, stepY, stepZ;
    UInt32 baseItem;
    bool nanInputs;             /**< FillElements puts NaN of random sign and payload into the float inputs */
};

STestState* g_state = GUCEF_NULL;

/** The noise object of the current noise type */
const void*
CurrentNoise( void )
{
    switch ( g_state->noiseType )
    {
        case MATHNOISE_VALUE: return &g_state->value;
        case MATHNOISE_OPENSIMPLEX2: return &g_state->simplex;
        case MATHNOISE_WORLEY: return &g_state->worley;
        default: return &g_state->perlin;
    }
}

/** Number of variants of a kernel */
UInt32
VariantCount( TMathBatchKernel kernel )
{
    if ( MATHBATCHKERNEL_VBAP_GAINS2D_F32 == kernel )
        return VbapLayoutCount;
    if ( MATHBATCHKERNEL_NOISE_FIELD2D_F32 == kernel || MATHBATCHKERNEL_NOISE_FIELD3D_F32 == kernel )
        return NoiseVariantCount;
    return 1;
}

/** Selects variant v of the kernel in the test state and returns the shape with the sizes that depend on it */
SKernelShape
ApplyVariant( TMathBatchKernel kernel, UInt32 variant )
{
    SKernelShape shape = KernelShapes[ kernel ];
    if ( MATHBATCHKERNEL_VBAP_GAINS2D_F32 == kernel )
    {
        g_state->layout = variant;
        shape.outSize = g_state->vbap[ variant ].GetSpeakerCount() * 4;
        shape.strides[ 0 ] = shape.outSize;
        shape.strides[ 1 ] = shape.outSize + 8;
        shape.strides[ 2 ] = shape.outSize + 24;
    }
    else if ( MATHBATCHKERNEL_NOISE_FIELD2D_F32 == kernel || MATHBATCHKERNEL_NOISE_FIELD3D_F32 == kernel )
    {
        // variants 0..3 the four noise types over ordinary coordinates, 4 Perlin far out where the lattice coordinate is clamped, 5 Perlin from NaN
        g_state->noiseType = variant < MATHNOISE_COUNT ? variant : (UInt32) MATHNOISE_PERLIN;
        g_state->originX = -5.3f;
        g_state->originY = 2.1f;
        g_state->originZ = 0.7f;
        g_state->stepX = 0.173f;
        g_state->stepY = 0.119f;
        g_state->stepZ = 0.0931f;
        if ( 4 == variant )
        {
            g_state->originX = 999999000.0f;
            g_state->originY = -1.5e9f;
            g_state->originZ = 3.0e9f;
            g_state->stepX = 100.0f;
            g_state->stepY = 33.0f;
            g_state->stepZ = 7.0f;
        }
        else if ( 5 == variant )
        {
            const UInt32 infinityBits = 0x7F800000;     // infinity: the lattice position is clamped and x - floor( x ) is NaN
            memcpy( &g_state->originX, &infinityBits, sizeof( infinityBits ) );
            g_state->originY = 0.5f;
        }
    }
    return shape;
}

/*-------------------------------------------------------------------------*/

Float32
FromBits( UInt32 bits )
{
    Float32 value;
    memcpy( &value, &bits, sizeof( value ) );
    return value;
}

/*-------------------------------------------------------------------------*/

/** As in TestSimdEquivalence: ordinary magnitudes, any finite bit pattern, special values; no NaN */
Float32
NextTestFloat( MathReference::CSeededSequence& sequence )
{
    const UInt32 kind = sequence.NextUInt32() % 100;
    const UInt32 bits = sequence.NextUInt32();
    const Float64 ordinary = sequence.NextFloat64( -100.0, 100.0 );
    if ( kind < 70 )
        return (Float32) ordinary;
    if ( kind < 90 )
    {
        UInt32 finiteBits = bits;
        if ( 0x7F800000 == ( finiteBits & 0x7F800000 ) )
            finiteBits &= 0xFF7FFFFF;
        return FromBits( finiteBits );
    }
    const Float32 specials[ 10 ] = { 0.0f, -0.0f, FromBits( 0x7F800000 ), FromBits( 0xFF800000 ), FLT_MAX, -FLT_MAX, FLT_MIN, FromBits( 0x00000001 ), 1.0f, -1.0f };
    return specials[ bits % 10 ];
}

/*-------------------------------------------------------------------------*/

/** A byte buffer whose data starts 'offset' bytes after a 16 byte boundary, filled with the sentinel */
class CTestBuffer
{
    public:

    CTestBuffer( UInt32 size, UInt32 offset )
        : m_storage( size + 32, Sentinel )
        , m_data( GUCEF_NULL )
        , m_size( size )
    {
        UInt8* raw = &m_storage[ 0 ];
        const size_t misalignment = (size_t) raw & 15;
        m_data = raw + ( 0 == misalignment ? 0 : 16 - misalignment ) + offset;
    }

    UInt8* Data( void ) { return m_data; }
    const UInt8* Data( void ) const { return m_data; }
    UInt32 Size( void ) const { return m_size; }

    void CopyFrom( const CTestBuffer& other ) { memcpy( m_data, other.m_data, m_size < other.m_size ? m_size : other.m_size ); }

    private:

    GUCEF::vector< UInt8 > m_storage;
    UInt8* m_data;
    UInt32 m_size;
};

/*-------------------------------------------------------------------------*/

/** As NextTestFloat for Float64: ordinary magnitudes, any finite bit pattern, special values; no NaN */
Float64
NextTestFloat64( MathReference::CSeededSequence& sequence )
{
    const UInt32 kind = sequence.NextUInt32() % 100;
    const UInt64 bits = sequence.NextUInt64();
    const Float64 ordinary = sequence.NextFloat64( -100.0, 100.0 );
    if ( kind < 75 )
        return ordinary;
    if ( kind < 90 )
    {
        UInt64 finiteBits = bits;
        if ( 0x7FF0000000000000ULL == ( finiteBits & 0x7FF0000000000000ULL ) )
            finiteBits &= 0xFFEFFFFFFFFFFFFFULL;
        Float64 value;
        memcpy( &value, &finiteBits, sizeof( value ) );
        return value;
    }
    const Float64 specials[ 6 ] = { 0.0, -0.0, DBL_MAX, -DBL_MAX, DBL_MIN, 1.0 };
    return specials[ bits % 6 ];
}

/** A Q16.16 raw value: mostly of moderate size so that the products stay in range, with the extremes mixed in */
Int32
NextTestFixedRaw( MathReference::CSeededSequence& sequence )
{
    const UInt32 kind = sequence.NextUInt32() % 100;
    const UInt32 bits = sequence.NextUInt32();
    if ( kind < 85 )
        return (Int32) ( bits % ( 1u << 22 ) ) - (Int32) ( 1u << 21 );
    const Int32 specials[ 6 ] = { 0, 65536, -65536, (Int32) 0x7FFFFFFF, (Int32) 0x80000000, 1 };
    return specials[ bits % 6 ];
}

/*-------------------------------------------------------------------------*/

/** Fills the elements (elementSize bytes every stride bytes) with seeded test data of the kernel's input kind */
void
FillElements( UInt8* data, UInt32 count, UInt32 stride, UInt32 elementSize, const SKernelShape& shape, MathReference::CSeededSequence& sequence )
{
    for ( UInt32 i=0; i<count; ++i )
    {
        UInt8* element = data + (size_t) i * stride;
        if ( ELEMENT_HALF == shape.element )
        {
            const UInt16 bits = (UInt16) sequence.NextUInt32();
            memcpy( element, &bits, sizeof( bits ) );
            continue;
        }
        if ( ELEMENT_FLOAT64 == shape.element )
        {
            for ( UInt32 c=0; c<elementSize / 8; ++c )
            {
                const Float64 value = NextTestFloat64( sequence );
                memcpy( element + c * 8, &value, sizeof( value ) );
            }
        }
        else if ( ELEMENT_FIXED32 == shape.element )
        {
            for ( UInt32 c=0; c<elementSize / 4; ++c )
            {
                const Int32 raw = NextTestFixedRaw( sequence );
                memcpy( element + c * 4, &raw, sizeof( raw ) );
            }
        }
        else
        {
            for ( UInt32 c=0; c<elementSize / 4; ++c )
            {
                Float32 value;
                if ( ELEMENT_BITS32 == shape.element )
                {
                    const UInt32 bits = sequence.NextUInt32();
                    value = FromBits( bits );
                }
                else
                {
                    value = NextTestFloat( sequence );
                }
                memcpy( element + c * 4, &value, sizeof( value ) );
            }
        }
        // degenerate elements for the fallback paths: zero quaternions and zero vectors
        if ( 0 == i % 37 && ELEMENT_BITS32 != shape.element && ELEMENT_FIXED32 != shape.element )
            memset( element, 0, elementSize );
        // NaN inputs: one component of every third element, any sign and payload
        if ( GUCEF_NULL != g_state && g_state->nanInputs && 1 == i % 3 && ( ELEMENT_FLOAT32 == shape.element || ELEMENT_FLOAT64 == shape.element ) )
        {
            const UInt32 width = ELEMENT_FLOAT64 == shape.element ? 8 : 4;
            const UInt32 component = sequence.NextUInt32() % ( elementSize / width );
            const UInt64 payload = sequence.NextUInt64();
            if ( 8 == width )
            {
                const UInt64 bits = ( payload & 0x800FFFFFFFFFFFFFULL ) | 0x7FF0000000000001ULL;
                memcpy( element + component * 8, &bits, sizeof( bits ) );
            }
            else
            {
                const UInt32 bits = ( (UInt32) payload & 0x807FFFFFu ) | 0x7F800001u;
                memcpy( element + component * 4, &bits, sizeof( bits ) );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
IsNaNBits( UInt32 bits )
{
    return 0x7F800000 == ( bits & 0x7F800000 ) && 0 != ( bits & 0x007FFFFF );
}

bool
IsNaNBits64( UInt64 bits )
{
    return 0x7FF0000000000000ULL == ( bits & 0x7FF0000000000000ULL ) && 0 != ( bits & 0x000FFFFFFFFFFFFFULL );
}

/**
 *  Bit identity of one output element. A NaN created by the arithmetic is NaN on every provider, but its sign and
 *  payload are not part of the bit identity: compilers may rewrite ( -a ) b as -( a b ), which flips the sign of a NaN.
 *  The conversions of a bit-exact provider compare every bit, NaN included, and so do the fixed point results (they
 *  are integers); a conversion provider without MATHPROVIDER_BITEXACT (NEON) only has to give NaN where scalar does.
 */
bool
SameElement( const UInt8* expected, const UInt8* actual, const SKernelShape& shape, bool bitExact )
{
    if ( 0 == memcmp( expected, actual, shape.outSize ) )
        return true;
    if ( !bitExact && MATHBATCHKERNEL_CONVERT_F32_TO_F16 == shape.kernel )
    {
        UInt16 a = 0;
        UInt16 b = 0;
        memcpy( &a, expected, sizeof( a ) );
        memcpy( &b, actual, sizeof( b ) );
        return CFloat16::FromBits( a ).IsNaN() && CFloat16::FromBits( b ).IsNaN();
    }
    if ( ( bitExact && MATHBATCHKERNEL_CONVERT_F32_TO_F16 == shape.kernel ) || ( bitExact && MATHBATCHKERNEL_CONVERT_F16_TO_F32 == shape.kernel ) || ELEMENT_FIXED32 == shape.element )
        return false;
    if ( ELEMENT_FLOAT64 == shape.element )
    {
        for ( UInt32 w=0; w<shape.outSize / 8; ++w )
        {
            UInt64 a = 0;
            UInt64 b = 0;
            memcpy( &a, expected + 8 * w, sizeof( a ) );
            memcpy( &b, actual + 8 * w, sizeof( b ) );
            if ( a != b && !( IsNaNBits64( a ) && IsNaNBits64( b ) ) )
                return false;
        }
        return true;
    }
    for ( UInt32 w=0; w<shape.outSize / 4; ++w )
    {
        UInt32 a = 0;
        UInt32 b = 0;
        memcpy( &a, expected + 4 * w, sizeof( a ) );
        memcpy( &b, actual + 4 * w, sizeof( b ) );
        if ( a != b && !( IsNaNBits( a ) && IsNaNBits( b ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  WebAssembly engines may set the quiet bit of a signaling NaN that is passed or returned as a Float32 value (the bits
 *  of the element operations, which only move memory, are untouched), so on that platform the conversions are compared
 *  with the quiet bit ignored for NaN results. Everywhere else every bit counts.
 */
bool
SameFloatBitsOfConversion( UInt32 expected, UInt32 actual )
{
    if ( expected == actual )
        return true;
    #if defined( __EMSCRIPTEN__ )
    return IsNaNBits( expected ) && IsNaNBits( actual ) && ( expected | 0x00400000 ) == ( actual | 0x00400000 );
    #else
    return false;
    #endif
}

bool
SameHalfBitsOfConversion( UInt16 expected, UInt16 actual )
{
    if ( expected == actual )
        return true;
    #if defined( __EMSCRIPTEN__ )
    return 0x7C00 == ( expected & 0x7C00 ) && 0 != ( expected & 0x03FF ) && 0x7C00 == ( actual & 0x7C00 ) && 0 != ( actual & 0x03FF ) && ( expected | 0x0200 ) == ( actual | 0x0200 );
    #else
    return false;
    #endif
}

/*-------------------------------------------------------------------------*/

/** Replaces every NaN of an array of floats by one quiet NaN, for hashes that do not depend on NaN signs; fixed point and conversions are left alone */
void
CanonicalizeNaNs( UInt8* data, UInt32 byteCount, const SKernelShape& shape )
{
    if ( MATHBATCHKERNEL_CONVERT_F32_TO_F16 == shape.kernel || MATHBATCHKERNEL_CONVERT_F16_TO_F32 == shape.kernel || ELEMENT_FIXED32 == shape.element )
        return;
    if ( ELEMENT_FLOAT64 == shape.element )
    {
        for ( UInt32 i=0; i<byteCount / 8; ++i )
        {
            UInt64 bits = 0;
            memcpy( &bits, data + 8 * i, sizeof( bits ) );
            if ( IsNaNBits64( bits ) )
            {
                bits = 0x7FF8000000000000ULL;
                memcpy( data + 8 * i, &bits, sizeof( bits ) );
            }
        }
        return;
    }
    for ( UInt32 i=0; i<byteCount / 4; ++i )
    {
        UInt32 bits = 0;
        memcpy( &bits, data + 4 * i, sizeof( bits ) );
        if ( IsNaNBits( bits ) )
        {
            bits = 0x7FC00000;
            memcpy( data + 4 * i, &bits, sizeof( bits ) );
        }
    }
}

/*-------------------------------------------------------------------------*/

/** Calls one provider of a kernel with the arrays given as bytes; the matrix is used by the transform kernels */
TMathStatus
CallProvider( const SMathBatchProvider& provider, TMathBatchKernel kernel, const TFloat32Matrix4D& m,
              const void* in, UInt32 inStride, const void* second, UInt32 secondStride, void* out, UInt32 outStride, UInt32 count )
{
    switch ( kernel )
    {
        case MATHBATCHKERNEL_TRANSFORM_POINTS3_F32:
        case MATHBATCHKERNEL_TRANSFORM_VECTORS3_F32:
        case MATHBATCHKERNEL_TRANSFORM_NORMALS3_F32:
        {
            SMathTransformPoints3F32Args args = { &m, in, inStride, out, outStride, count };
            return reinterpret_cast< TMathTransformPoints3F32Function >( provider.function )( provider.context, args );
        }
        case MATHBATCHKERNEL_TRANSFORM_VECTORS4_F32:
        {
            SMathTransformVectors4F32Args args = { &m, in, inStride, out, outStride, count };
            return reinterpret_cast< TMathTransformVectors4F32Function >( provider.function )( provider.context, args );
        }
        case MATHBATCHKERNEL_MULTIPLY_MATRICES4_F32:
        {
            SMathMultiplyMatrices4F32Args args = { in, inStride, second, secondStride, out, outStride, count };
            return reinterpret_cast< TMathMultiplyMatrices4F32Function >( provider.function )( provider.context, args );
        }
        case MATHBATCHKERNEL_QUATERNIONS_TO_MATRICES_F32:
        {
            SMathQuaternionsToMatricesF32Args args = { in, inStride, out, outStride, count };
            return reinterpret_cast< TMathQuaternionsToMatricesF32Function >( provider.function )( provider.context, args );
        }
        case MATHBATCHKERNEL_DOT_PRODUCTS3_F32:
        {
            SMathDotProducts3F32Args args = { in, inStride, second, secondStride, out, outStride, count };
            return reinterpret_cast< TMathDotProducts3F32Function >( provider.function )( provider.context, args );
        }
        case MATHBATCHKERNEL_NORMALIZE_VECTORS3_F32:
        {
            SMathNormalizeVectors3F32Args args = { in, inStride, out, outStride, count };
            return reinterpret_cast< TMathNormalizeVectors3F32Function >( provider.function )( provider.context, args );
        }
        case MATHBATCHKERNEL_CONVERT_F32_TO_F16:
        {
            SMathConvertF32ToF16Args args = { static_cast< const Float32* >( in ), static_cast< CFloat16* >( out ), count };
            return reinterpret_cast< TMathConvertF32ToF16Function >( provider.function )( provider.context, args );
        }
        case MATHBATCHKERNEL_CONVERT_F16_TO_F32:
        {
            SMathConvertF16ToF32Args args = { static_cast< const CFloat16* >( in ), static_cast< Float32* >( out ), count };
            return reinterpret_cast< TMathConvertF16ToF32Function >( provider.function )( provider.context, args );
        }
        case MATHBATCHKERNEL_TRANSFORM_POINTS3_F64:
        case MATHBATCHKERNEL_TRANSFORM_VECTORS3_F64:
        {
            SMathTransformPoints3F64Args args = { &g_state->matrix64, in, inStride, out, outStride, count };
            return reinterpret_cast< TMathTransformPoints3F64Function >( provider.function )( provider.context, args );
        }
        case MATHBATCHKERNEL_MULTIPLY_MATRICES4_F64:
        {
            SMathMultiplyMatrices4F64Args args = { in, inStride, second, secondStride, out, outStride, count };
            return reinterpret_cast< TMathMultiplyMatrices4F64Function >( provider.function )( provider.context, args );
        }
        case MATHBATCHKERNEL_TRANSFORM_POINTS3_FX32:
        {
            SMathTransformPoints3Fx32Args args = { &g_state->matrixFx, in, inStride, out, outStride, count };
            return reinterpret_cast< TMathTransformPoints3Fx32Function >( provider.function )( provider.context, args );
        }
        case MATHBATCHKERNEL_MULTIPLY_MATRICES4_FX32:
        {
            SMathMultiplyMatrices4Fx32Args args = { in, inStride, second, secondStride, out, outStride, count };
            return reinterpret_cast< TMathMultiplyMatrices4Fx32Function >( provider.function )( provider.context, args );
        }
        case MATHBATCHKERNEL_VBAP_GAINS2D_F32:
        {
            SMathVbapGains2DF32Args args = { &g_state->vbap[ g_state->layout ], in, inStride, g_state->targetPower, out, outStride, count };
            return reinterpret_cast< TMathVbapGains2DF32Function >( provider.function )( provider.context, args );
        }
        case MATHBATCHKERNEL_NOISE_FIELD2D_F32:
        {
            SMathNoiseField2DF32Args args = { CurrentNoise(), g_state->noiseType, g_state->originX, g_state->originY, g_state->stepX, g_state->stepY,
                                              37, 11, g_state->baseItem, static_cast< Float32* >( out ), count };
            return reinterpret_cast< TMathNoiseField2DF32Function >( provider.function )( provider.context, args );
        }
        case MATHBATCHKERNEL_NOISE_FIELD3D_F32:
        {
            SMathNoiseField3DF32Args args = { CurrentNoise(), g_state->noiseType, g_state->originX, g_state->originY, g_state->originZ, g_state->stepX, g_state->stepY, g_state->stepZ,
                                              13, 7, g_state->baseItem, static_cast< Float32* >( out ), count };
            return reinterpret_cast< TMathNoiseField3DF32Function >( provider.function )( provider.context, args );
        }
        default:
            return MATHSTATUS_INVALID_OPERAND;
    }
}

/*-------------------------------------------------------------------------*/

/** The providers to test for a kernel: those of the best set and those of the base instruction set (SSE2 on x86, the only set elsewhere), each function once */
void
CollectProviders( const CMathBatchDispatcher& best, const CMathBatchDispatcher& sse2Only, TMathBatchKernel kernel,
                  GUCEF::vector< SMathBatchProvider >& providers, SMathBatchProvider& scalar )
{
    providers.clear();
    const CMathBatchDispatcher* dispatchers[ 2 ] = { &best, &sse2Only };
    for ( UInt32 d=0; d<2; ++d )
    {
        for ( UInt32 i=0; i<dispatchers[ d ]->GetProviderCount( kernel ); ++i )
        {
            SMathBatchProvider provider;
            dispatchers[ d ]->GetProvider( kernel, i, provider );
            if ( MATHBATCHTIER_SCALAR == provider.tier )
                scalar = provider;
            bool known = false;
            for ( UInt32 k=0; k<providers.size(); ++k )
                known = known || providers[ k ].function == provider.function;
            if ( !known )
                providers.push_back( provider );
        }
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  Runs 'provider' and the scalar provider on the same seeded input for one count, layout and alignment, and compares
 *  every output element bit for bit and every byte between the elements (they must keep the sentinel). In place: the
 *  input is first copied into the output array and the provider runs with the output as input.
 */
bool
CompareWithScalar( const SMathBatchProvider& provider, const SMathBatchProvider& scalar, const SKernelShape& shape, const TFloat32Matrix4D& m,
                   UInt32 count, UInt32 inStride, UInt32 outStride, UInt32 offset, bool inPlace, UInt64 seed )
{
    MathReference::CSeededSequence sequence( seed );
    const UInt32 secondStride = shape.secondSize > 0 ? ( inStride > shape.secondSize ? inStride : shape.secondSize ) : 0;
    const UInt32 inBytes = count * inStride + 16;
    const UInt32 outBytes = count * ( outStride > inStride ? outStride : inStride ) + 16;
    CTestBuffer in( inBytes, offset );
    CTestBuffer second( count * secondStride + 16, offset );
    CTestBuffer expected( outBytes, offset );
    CTestBuffer actual( outBytes, offset );
    FillElements( in.Data(), count, inStride, shape.inSize, shape, sequence );
    if ( shape.secondSize > 0 )
        FillElements( second.Data(), count, secondStride, shape.secondSize, shape, sequence );

    if ( MATHSTATUS_OK != CallProvider( scalar, shape.kernel, m, in.Data(), inStride, second.Data(), secondStride, expected.Data(), outStride, count ) )
        return false;

    TMathStatus status = MATHSTATUS_OK;
    if ( inPlace )
    {
        // the output array holds the input (for the conversions as the packed input array); the rest keeps the sentinel
        if ( 0 == shape.strides[ 0 ] )
        {
            memcpy( actual.Data(), in.Data(), (size_t) count * shape.inSize );
        }
        else
        {
            for ( UInt32 i=0; i<count; ++i )
                memcpy( actual.Data() + (size_t) i * outStride, in.Data() + (size_t) i * inStride, shape.inSize );
        }
        status = CallProvider( provider, shape.kernel, m, actual.Data(), inStride, second.Data(), secondStride, actual.Data(), outStride, count );
    }
    else
    {
        status = CallProvider( provider, shape.kernel, m, in.Data(), inStride, second.Data(), secondStride, actual.Data(), outStride, count );
    }
    if ( MATHSTATUS_OK != status )
        return false;

    for ( UInt32 i=0; i<count; ++i )
    {
        if ( !SameElement( expected.Data() + (size_t) i * outStride, actual.Data() + (size_t) i * outStride, shape, 0 != ( provider.flags & MATHPROVIDER_BITEXACT ) ) )
        {
            UInt32 word = 0;
            while ( word + 1 < shape.outSize / 4 && 0 == memcmp( expected.Data() + (size_t) i * outStride + 4 * word, actual.Data() + (size_t) i * outStride + 4 * word, 4 ) )
                ++word;
            UInt32 expectedBits = 0;
            UInt32 actualBits = 0;
            memcpy( &expectedBits, expected.Data() + (size_t) i * outStride + 4 * word, sizeof( expectedBits ) );
            memcpy( &actualBits, actual.Data() + (size_t) i * outStride + 4 * word, sizeof( actualBits ) );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "BatchKernels: " << provider.name << " differs from scalar for kernel " << (UInt32) shape.kernel
                             << " at element " << i << " of " << count << " (seed " << CORE::ToString( seed ) << ", strides " << inStride << "/" << outStride
                             << ", offset " << offset << ( inPlace ? ", in place)" : ")" ) << ", value " << word << ": scalar bits " << expectedBits << ", provider bits " << actualBits );
            return false;
        }
        if ( !inPlace )
        {
            for ( UInt32 b=shape.outSize; b<outStride; ++b )
            {
                if ( Sentinel != actual.Data()[ (size_t) i * outStride + b ] )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "BatchKernels: " << provider.name << " wrote between the elements of kernel " << (UInt32) shape.kernel );
                    return false;
                }
            }
        }
    }
    // nothing after the last element
    if ( !inPlace && Sentinel != actual.Data()[ (size_t) count * outStride ] )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/** A matrix with general (non-affine) entries from the seeded sequence */
TFloat32Matrix4D
SeededMatrix( MathReference::CSeededSequence& sequence )
{
    Float32 values[ 16 ];
    for ( UInt32 i=0; i<16; ++i )
    {
        const Float64 value = sequence.NextFloat64( -3.0, 3.0 );
        values[ i ] = (Float32) value;
    }
    return TFloat32Matrix4D::FromColumnMajor( values );
}

/*-------------------------------------------------------------------------*/

UInt64
HashBytes( const void* data, UInt32 size, UInt64 hash )
{
    return MathReference::Fnv1a( data, size, hash );
}


/*-------------------------------------------------------------------------*/

/** Whether this build and CPU have a SIMD provider for the kernel: the registration rules of the instruction sets */
bool
ExpectSimdProvider( TMathBatchKernel kernel, UInt64 features )
{
    const bool halfConversion = MATHBATCHKERNEL_CONVERT_F32_TO_F16 == kernel || MATHBATCHKERNEL_CONVERT_F16_TO_F32 == kernel;
    const bool float64 = MATHBATCHKERNEL_TRANSFORM_POINTS3_F64 == kernel || MATHBATCHKERNEL_TRANSFORM_VECTORS3_F64 == kernel || MATHBATCHKERNEL_MULTIPLY_MATRICES4_F64 == kernel;
    const bool fixedPoint = MATHBATCHKERNEL_TRANSFORM_POINTS3_FX32 == kernel || MATHBATCHKERNEL_MULTIPLY_MATRICES4_FX32 == kernel;
    if ( fixedPoint )
        return false;
    #if ( GUCEF_MATH_SIMD_LEVEL == GUCEF_MATH_SIMD_NONE )
    (void) features; (void) halfConversion; (void) float64;
    return false;
    #elif ( GUCEF_MATH_SIMD_HAVE_SSE2 == 1 )
    if ( float64 )
        return GUCEF_MATH_SIMD_BATCH_ALLOW_AVX2 == 1 && 0 != ( features & GUCEF_CPU_ISA_AVX2 ) && 0 != ( features & GUCEF_CPU_ISA_AVX );
    if ( halfConversion )
        return GUCEF_MATH_SIMD_BATCH_ALLOW_F16C == 1 && 0 != ( features & GUCEF_CPU_ISA_F16C ) && 0 != ( features & GUCEF_CPU_ISA_AVX );
    return 0 != ( features & GUCEF_CPU_ISA_SSE2 );
    #elif ( GUCEF_MATH_SIMD_HAVE_NEON == 1 )
    (void) float64;
    return 0 != ( features & GUCEF_CPU_ISA_NEON );
    #elif ( GUCEF_MATH_SIMD_HAVE_WASM128 == 1 )
    return !halfConversion && !float64 && 0 != ( features & GUCEF_CPU_ISA_WASM_SIMD128 );
    #else
    return false;
    #endif
}

/*-------------------------------------------------------------------------*/

/** Prepares the matrices, panners and noise parameters of the test state */
void
InitializeTestState( STestState& state, MathReference::CSeededSequence& sequence )
{
    Float32 values[ 16 ];
    Float64 values64[ 16 ];
    TFixed32 valuesFx[ 16 ];
    for ( UInt32 i=0; i<16; ++i )
    {
        const Float64 value = sequence.NextFloat64( -3.0, 3.0 );
        values[ i ] = (Float32) value;
        values64[ i ] = value * 1.0000001;
        valuesFx[ i ] = TFixed32::FromRaw( (Int32) ( value * 65536.0 ) );
    }
    state.matrix = TFloat32Matrix4D::FromColumnMajor( values );
    state.matrix64 = TFloat64Matrix4D::FromColumnMajor( values64 );
    state.matrixFx = TFixed32Matrix4D::FromColumnMajor( valuesFx );

    // layout 0 a stereo pair ( the open side gives the cross-fade branch ), 1 four speakers, 2 six irregular speakers, 3 thirty-two speakers
    const Float32 stereo[ 2 ] = { -0.5235988f, 0.5235988f };
    const Float32 quad[ 4 ] = { 0.7853982f, 2.3561945f, 3.9269908f, 5.4977871f };
    const Float32 six[ 6 ] = { 0.0f, 0.5235988f, 1.9198622f, 3.1415927f, 4.3633232f, 5.7595865f };
    Float32 many[ 32 ];
    for ( UInt32 i=0; i<32; ++i )
        many[ i ] = (Float32) i * 0.19634954f;
    const Float32* layouts[ VbapLayoutCount ] = { stereo, quad, six, many };
    const UInt32 counts[ VbapLayoutCount ] = { 2, 4, 6, 32 };
    for ( UInt32 l=0; l<VbapLayoutCount; ++l )
    {
        TFloat32Radian azimuths[ 32 ];
        for ( UInt32 i=0; i<counts[ l ]; ++i )
            azimuths[ i ] = TFloat32Radian( layouts[ l ][ i ] );
        state.vbap[ l ].Prepare( azimuths, counts[ l ] );
    }
}
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformBatchKernelsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "BatchKernels" );

    GUCEF_TESTFW_TESTCASE( "BatchKernels: the dispatcher of CMathGlobal has a scalar provider for every kernel and the best SIMD set of the CPU" )
    try
    {
        const CMathBatchDispatcher& dispatcher = CMathGlobal::Instance()->GetBatchDispatcher();
        const UInt64 features = CORE::GetCpuIsaFeatures();
        ASSERT_TRUE( features == dispatcher.GetCpuIsaFeatures() );
        for ( UInt32 k=0; k<MATHBATCHKERNEL_MATH_COUNT; ++k )
        {
            const TMathBatchKernel kernel = (TMathBatchKernel) k;
            const UInt32 providerCount = dispatcher.GetProviderCount( kernel );
            ASSERT_TRUE( providerCount >= 1 && providerCount <= 2 );
            SMathBatchProvider last;
            ASSERT_TRUE( dispatcher.GetProvider( kernel, providerCount - 1, last ) );
            ASSERT_TRUE( MATHBATCHTIER_SCALAR == last.tier && 0 != ( last.flags & MATHPROVIDER_BITEXACT ) && 0 == strcmp( "Scalar", last.name ) );
            ASSERT_TRUE( GUCEF_NULL != dispatcher.GetKernelId( kernel ) && 0 == strncmp( "MATH.", dispatcher.GetKernelId( kernel ), 5 ) );
            if ( 2 == providerCount )
            {
                SMathBatchProvider simd;
                ASSERT_TRUE( dispatcher.GetProvider( kernel, 0, simd ) );
                // every SIMD provider is bit-exact except the NEON half conversions, which follow FPCR (see gucefMATH_MathBatchTypes.h)
                const bool neonConversion = 0 == strcmp( "NEON", simd.name ) && ( MATHBATCHKERNEL_CONVERT_F32_TO_F16 == kernel || MATHBATCHKERNEL_CONVERT_F16_TO_F32 == kernel );
                ASSERT_TRUE( MATHBATCHTIER_SIMD == simd.tier && ( neonConversion || 0 != ( simd.flags & MATHPROVIDER_BITEXACT ) ) );
                ASSERT_TRUE( simd.requiredCpuFeatures == ( features & simd.requiredCpuFeatures ) );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "BatchKernels: " << dispatcher.GetKernelId( kernel ) << " uses " << simd.name );
            }
            ASSERT_TRUE( providerCount == ( ExpectSimdProvider( kernel, features ) ? 2 : 1 ) );
            // the Q16.16 kernels are always deterministic
            ASSERT_TRUE( dispatcher.IsKernelAlwaysDeterministic( kernel ) == ( MATHBATCHKERNEL_TRANSFORM_POINTS3_FX32 == kernel || MATHBATCHKERNEL_MULTIPLY_MATRICES4_FX32 == kernel ) );
        }
        ASSERT_TRUE( 0 == strcmp( "MATH.TransformPoints3.F32", dispatcher.GetKernelId( MATHBATCHKERNEL_TRANSFORM_POINTS3_F32 ) ) );
        ASSERT_TRUE( 0 == strcmp( "MATH.ConvertF16ToF32.F32", dispatcher.GetKernelId( MATHBATCHKERNEL_CONVERT_F16_TO_F32 ) ) );
        ASSERT_TRUE( 0 == strcmp( "MATH.ConvertF32ToF16.F16", dispatcher.GetKernelId( MATHBATCHKERNEL_CONVERT_F32_TO_F16 ) ) );
        // every MATH kernel id has the three parts module.kernel.kind that gucefCOMPUTE kernel declarations require
        for ( UInt32 k=0; k<MATHBATCHKERNEL_MATH_COUNT; ++k )
        {
            const char* id = dispatcher.GetKernelId( (TMathBatchKernel) k );
            UInt32 dots = 0;
            for ( const char* c = id; GUCEF_NULL != c && 0 != *c; ++c )
                dots += '.' == *c ? 1 : 0;
            ASSERT_TRUE( 2 == dots );
        }
        ASSERT_TRUE( 0 == strcmp( "MATH.TransformPoints3.F64", dispatcher.GetKernelId( MATHBATCHKERNEL_TRANSFORM_POINTS3_F64 ) ) );
        ASSERT_TRUE( 0 == strcmp( "MATH.MultiplyMatrices4.FX32", dispatcher.GetKernelId( MATHBATCHKERNEL_MULTIPLY_MATRICES4_FX32 ) ) );
        ASSERT_TRUE( 0 == strcmp( "MATH.VbapGains2D.F32", dispatcher.GetKernelId( MATHBATCHKERNEL_VBAP_GAINS2D_F32 ) ) );
        ASSERT_TRUE( 0 == strcmp( "MATH.NoiseField2D.F32", dispatcher.GetKernelId( MATHBATCHKERNEL_NOISE_FIELD2D_F32 ) ) );
        ASSERT_TRUE( 0 == strcmp( "MATH.NoiseField3D.F32", dispatcher.GetKernelId( MATHBATCHKERNEL_NOISE_FIELD3D_F32 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BatchKernels: every provider equals the scalar provider for the counts, 4 byte misalignment, in place and the strides of the acceptance list" )
    try
    {
        CMathBatchDispatcher best;
        best.RegisterBuiltInProviders();
        CMathBatchDispatcher sse2Only;
        sse2Only.RegisterBuiltInProviders( ~( GUCEF_CPU_ISA_AVX | GUCEF_CPU_ISA_AVX2 | GUCEF_CPU_ISA_F16C | GUCEF_CPU_ISA_AVX512F ) );
        MathReference::CSeededSequence matrixSequence( KernelSeed );
        STestState state;
        g_state = &state;
        InitializeTestState( state, matrixSequence );
        for ( UInt32 l=0; l<VbapLayoutCount; ++l )
            ASSERT_TRUE( state.vbap[ l ].IsPrepared() );
        const TFloat32Matrix4D m = state.matrix;
        UInt32 comparisons = 0;
        for ( UInt32 k=0; k<MATHBATCHKERNEL_MATH_COUNT; ++k )
        {
            GUCEF::vector< SMathBatchProvider > providers;
            SMathBatchProvider scalar;
            memset( &scalar, 0, sizeof( scalar ) );
            CollectProviders( best, sse2Only, (TMathBatchKernel) k, providers, scalar );
            ASSERT_TRUE( GUCEF_NULL != scalar.function );
            for ( UInt32 variant=0; variant<VariantCount( (TMathBatchKernel) k ); ++variant )
            {
                const SKernelShape shape = ApplyVariant( (TMathBatchKernel) k, variant );
                for ( UInt32 p=0; p<providers.size(); ++p )
                {
                    const SMathBatchProvider& provider = providers[ p ];
                    for ( UInt32 c=0; c<TestCountCount; ++c )
                    {
                        const UInt32 count = TestCounts[ c ];
                        const UInt64 seed = KernelSeed + 1000 * k + c;
                        state.targetPower = TargetPowers[ ( c + variant ) % 4 ];
                        state.baseItem = (UInt32) ( ( seed + variant ) % 13 );
                        // contiguous arrays, 16 byte aligned and 4 bytes past a 16 byte boundary
                        ASSERT_TRUE( CompareWithScalar( provider, scalar, shape, m, count, shape.inSize, shape.outSize, 0, false, seed ) );
                        ASSERT_TRUE( CompareWithScalar( provider, scalar, shape, m, count, shape.inSize, shape.outSize, 4, false, seed ) );
                        comparisons += 2;
                        // in place: the same pointer and stride for kernels with strides, the same array for the conversions
                        if ( 0 != shape.strides[ 0 ] )
                            ASSERT_TRUE( CompareWithScalar( provider, scalar, shape, m, count, shape.strides[ 0 ], shape.strides[ 0 ], 4, true, seed ) );
                        else
                            ASSERT_TRUE( CompareWithScalar( provider, scalar, shape, m, count, shape.inSize, shape.outSize, 4, true, seed ) );
                        ++comparisons;
                        // strided input and output
                        for ( UInt32 st=0; st<3 && 0 != shape.strides[ st ]; ++st )
                        {
                            const UInt32 stride = shape.strides[ st ];
                            if ( stride < shape.inSize || stride < shape.outSize )
                                continue;
                            ASSERT_TRUE( CompareWithScalar( provider, scalar, shape, m, count, stride, stride, 4, false, seed ) );
                            ++comparisons;
                        }
                    }
                }
            }
            for ( UInt32 p=0; p<providers.size(); ++p )
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "BatchKernels: " << best.GetKernelId( (TMathBatchKernel) k ) << " provider " << providers[ p ].name << " checked" );
        }
        g_state = GUCEF_NULL;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "BatchKernels: " << comparisons << " provider comparisons" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BatchKernels: with NaN inputs every provider gives NaN exactly where the scalar provider does, every other value bit for bit" )
    try
    {
        CMathBatchDispatcher best;
        best.RegisterBuiltInProviders();
        CMathBatchDispatcher sse2Only;
        sse2Only.RegisterBuiltInProviders( ~( GUCEF_CPU_ISA_AVX | GUCEF_CPU_ISA_AVX2 | GUCEF_CPU_ISA_F16C | GUCEF_CPU_ISA_AVX512F ) );
        MathReference::CSeededSequence matrixSequence( KernelSeed ^ 0x4E614E );
        STestState state;
        g_state = &state;
        InitializeTestState( state, matrixSequence );
        state.nanInputs = true;
        const UInt32 nanCounts[ 4 ] = { 1, 7, 17, 1023 };
        UInt32 comparisons = 0;
        for ( UInt32 k=0; k<MATHBATCHKERNEL_MATH_COUNT; ++k )
        {
            const SKernelShape& kernelShape = KernelShapes[ k ];
            if ( ELEMENT_FLOAT32 != kernelShape.element && ELEMENT_FLOAT64 != kernelShape.element )
                continue;
            GUCEF::vector< SMathBatchProvider > providers;
            SMathBatchProvider scalar;
            memset( &scalar, 0, sizeof( scalar ) );
            CollectProviders( best, sse2Only, (TMathBatchKernel) k, providers, scalar );
            const SKernelShape shape = ApplyVariant( (TMathBatchKernel) k, 0 );
            if ( 0 == shape.inSize )
                continue;       // the noise fields have no input arrays
            for ( UInt32 p=0; p<providers.size(); ++p )
            {
                for ( UInt32 c=0; c<4; ++c )
                {
                    const UInt64 seed = KernelSeed + 7777 * k + c;
                    ASSERT_TRUE( CompareWithScalar( providers[ p ], scalar, shape, state.matrix, nanCounts[ c ], shape.inSize, shape.outSize, 4, false, seed ) );
                    ++comparisons;
                }
            }
        }
        g_state = GUCEF_NULL;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "BatchKernels: " << comparisons << " provider comparisons with NaN inputs" );
    }
    catch( ... )
    {
        g_state = GUCEF_NULL;
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BatchKernels: strides smaller than an element are rejected before any provider runs" )
    try
    {
        CMathBatchDispatcher dispatcher;
        dispatcher.RegisterBuiltInProviders();
        TFloat32Matrix4D m = TFloat32Matrix4D::Translation( TFloat32Vector3D( 1.0f, 2.0f, 3.0f ) );
        Float32 points[ 12 ] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
        Float32 out[ 12 ];
        for ( UInt32 i=0; i<12; ++i )
            out[ i ] = -1.0f;
        const UInt32 badStrides[ 3 ] = { 0, 4, 11 };
        for ( UInt32 b=0; b<3; ++b )
        {
            SMathTransformPoints3F32Args args = { &m, points, badStrides[ b ], out, 12, 4 };
            ASSERT_TRUE( MATHSTATUS_INVALID_OPERAND == dispatcher.Execute( MATHBATCHKERNEL_TRANSFORM_POINTS3_F32, args, 4, 0, args.in, args.out ) );
            SMathTransformPoints3F32Args outArgs = { &m, points, 12, out, badStrides[ b ], 4 };
            ASSERT_TRUE( MATHSTATUS_INVALID_OPERAND == dispatcher.Execute( MATHBATCHKERNEL_TRANSFORM_POINTS3_F32, outArgs, 4, 0, outArgs.in, outArgs.out ) );
            SMathDotProducts3F32Args dotArgs = { points, 12, points, badStrides[ b ], out, 4, 4 };
            ASSERT_TRUE( MATHSTATUS_INVALID_OPERAND == dispatcher.Execute( MATHBATCHKERNEL_DOT_PRODUCTS3_F32, dotArgs, 4, 0, dotArgs.a, dotArgs.out ) );
            SMathMultiplyMatrices4F32Args multiplyArgs = { points, 64, points, 64 * badStrides[ b ] / 12, out, 64, 2 };
            ASSERT_TRUE( MATHSTATUS_INVALID_OPERAND == dispatcher.Execute( MATHBATCHKERNEL_MULTIPLY_MATRICES4_F32, multiplyArgs, 2, 0, multiplyArgs.a, multiplyArgs.out ) );
        }
        for ( UInt32 i=0; i<12; ++i )
            ASSERT_TRUE( -1.0f == out[ i ] );       // nothing written
        // a single element needs no stride
        SMathTransformPoints3F32Args single = { &m, points, 0, out, 0, 1 };
        ASSERT_TRUE( MATHSTATUS_OK == dispatcher.Execute( MATHBATCHKERNEL_TRANSFORM_POINTS3_F32, single, 1, 0, single.in, single.out ) );
        ASSERT_TRUE( 2.0f == out[ 0 ] && 4.0f == out[ 1 ] && 6.0f == out[ 2 ] );
        // the strided batch function: a valid stride works, count 0 is fine with any stride
        ASSERT_TRUE( TransformPointsStrided( m, points, 12, out, 12, 4 ) );
        ASSERT_TRUE( TransformPointsStrided( m, points, 0, out, 0, 0 ) );
        #ifndef GUCEF_MATH_DEBUG_MODE
        // too small: rejected (debug builds assert instead)
        for ( UInt32 i=0; i<12; ++i )
            out[ i ] = -1.0f;
        ASSERT_FALSE( TransformPointsStrided( m, points, 4, out, 12, 4 ) );
        ASSERT_FALSE( TransformPointsStrided( m, points, 12, out, 0, 4 ) );
        for ( UInt32 i=0; i<12; ++i )
            ASSERT_TRUE( -1.0f == out[ i ] );
        #endif
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BatchKernels: the batch functions give the value-type result per element" )
    try
    {
        MathReference::CSeededSequence sequence( KernelSeed ^ 0x77 );
        const UInt32 count = 2053;
        const TFloat32Matrix4D m = SeededMatrix( sequence );
        GUCEF::vector< TFloat32Point3D > points( count );
        GUCEF::vector< TFloat32Vector3D > vectors( count );
        GUCEF::vector< TFloat32Vector3D > others( count );
        GUCEF::vector< TFloat32Vector3D > units( count );
        GUCEF::vector< TFloat32Vector4D > vectors4( count );
        GUCEF::vector< TFloat32Quaternion > quaternions( count );
        GUCEF::vector< TFloat32Matrix4D > matrices( count );
        GUCEF::vector< TFloat32Matrix4D > otherMatrices( count );
        for ( UInt32 i=0; i<count; ++i )
        {
            Float32 v[ 16 ];
            for ( UInt32 c=0; c<16; ++c )
            {
                const Float64 value = sequence.NextFloat64( -10.0, 10.0 );
                v[ c ] = (Float32) value;
            }
            points[ i ] = TFloat32Point3D( v[ 0 ], v[ 1 ], v[ 2 ] );
            vectors[ i ] = TFloat32Vector3D( v[ 3 ], v[ 4 ], v[ 5 ] );
            others[ i ] = TFloat32Vector3D( v[ 6 ], v[ 7 ], v[ 8 ] );
            units[ i ] = vectors[ i ].Normalized();
            vectors4[ i ] = TFloat32Vector4D( v[ 9 ], v[ 10 ], v[ 11 ], v[ 12 ] );
            quaternions[ i ] = TFloat32Quaternion::FromXYZW( v[ 13 ], v[ 14 ], v[ 15 ], v[ 0 ] );
            matrices[ i ] = TFloat32Matrix4D::FromColumnMajor( v );
            otherMatrices[ i ] = matrices[ i ].Transposed() + TFloat32Matrix4D::Translation( TFloat32Vector3D( v[ 1 ], v[ 5 ], v[ 9 ] ) );
        }
        units[ 7 ] = TFloat32Vector3D();        // passed through by TransformNormals

        const UInt32 flagSets[ 3 ] = { 0, MATHBATCH_FORCE_SCALAR, MATHBATCH_REALTIME | MATHBATCH_DETERMINISTIC };
        for ( UInt32 f=0; f<3; ++f )
        {
            const UInt32 flags = flagSets[ f ];
            GUCEF::vector< TFloat32Point3D > outPoints( count );
            GUCEF::vector< TFloat32Vector3D > outVectors( count );
            GUCEF::vector< TFloat32Vector4D > outVectors4( count );
            GUCEF::vector< TFloat32Matrix4D > outMatrices( count );
            GUCEF::vector< Float32 > outDots( count );
            bool ok = true;

            TransformPoints( m, &points[ 0 ], &outPoints[ 0 ], count, flags );
            for ( UInt32 i=0; i<count; ++i )
            {
                const TFloat32Point3D expected = m.TransformPoint( points[ i ] );
                ok = ok && 0 == memcmp( &expected, &outPoints[ i ], sizeof( expected ) );
            }
            ASSERT_TRUE( ok );

            TransformVectors( m, &vectors[ 0 ], &outVectors[ 0 ], count, flags );
            for ( UInt32 i=0; i<count; ++i )
            {
                const TFloat32Vector3D expected = m.TransformVector( vectors[ i ] );
                ok = ok && 0 == memcmp( &expected, &outVectors[ i ], sizeof( expected ) );
            }
            ASSERT_TRUE( ok );

            TransformNormals( m, &units[ 0 ], &outVectors[ 0 ], count, flags );
            for ( UInt32 i=0; i<count; ++i )
            {
                if ( units[ i ].IsZero() )
                {
                    ok = ok && 0 == memcmp( &units[ i ], &outVectors[ i ], sizeof( TFloat32Vector3D ) );
                    continue;
                }
                const TFloat32DirectionVector3D direction = TFloat32DirectionVector3D::FromNormalized( units[ i ] );
                const TFloat32Vector3D expected = m.TransformNormal( direction ).AsVector();
                ok = ok && 0 == memcmp( &expected, &outVectors[ i ], sizeof( expected ) );
            }
            ASSERT_TRUE( ok );

            TransformVectors4( m, &vectors4[ 0 ], &outVectors4[ 0 ], count, flags );
            for ( UInt32 i=0; i<count; ++i )
            {
                const TFloat32Vector4D expected = m.Transform( vectors4[ i ] );
                ok = ok && 0 == memcmp( &expected, &outVectors4[ i ], sizeof( expected ) );
            }
            ASSERT_TRUE( ok );

            MultiplyMatrices( &matrices[ 0 ], &otherMatrices[ 0 ], &outMatrices[ 0 ], count, flags );
            for ( UInt32 i=0; i<count; ++i )
            {
                const TFloat32Matrix4D expected = matrices[ i ].Multiply( otherMatrices[ i ] );
                ok = ok && 0 == memcmp( &expected, &outMatrices[ i ], sizeof( expected ) );
            }
            ASSERT_TRUE( ok );

            QuaternionsToMatrices( &quaternions[ 0 ], &outMatrices[ 0 ], count, flags );
            for ( UInt32 i=0; i<count; ++i )
            {
                const TFloat32Matrix4D expected = TFloat32Matrix4D::FromQuaternion( quaternions[ i ] );
                ok = ok && 0 == memcmp( &expected, &outMatrices[ i ], sizeof( expected ) );
            }
            ASSERT_TRUE( ok );

            DotProducts3( &vectors[ 0 ], &others[ 0 ], &outDots[ 0 ], count, flags );
            for ( UInt32 i=0; i<count; ++i )
            {
                const Float32 expected = vectors[ i ].Dot( others[ i ] );
                ok = ok && 0 == memcmp( &expected, &outDots[ i ], sizeof( expected ) );
            }
            ASSERT_TRUE( ok );

            NormalizeVectors3( &vectors[ 0 ], &outVectors[ 0 ], count, flags );
            for ( UInt32 i=0; i<count; ++i )
            {
                const TFloat32Vector3D expected = vectors[ i ].Normalized();
                ok = ok && 0 == memcmp( &expected, &outVectors[ i ], sizeof( expected ) );
            }
            ASSERT_TRUE( ok );

            // in place through the public functions
            GUCEF::vector< TFloat32Vector3D > inPlace( vectors );
            NormalizeVectors3( &inPlace[ 0 ], &inPlace[ 0 ], count, flags );
            ASSERT_TRUE( 0 == memcmp( &inPlace[ 0 ], &outVectors[ 0 ], count * sizeof( TFloat32Vector3D ) ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BatchKernels: the Float64 and Q16.16 batch functions give the value-type result per element" )
    try
    {
        MathReference::CSeededSequence sequence( KernelSeed ^ 0x1F64 );
        STestState state;
        g_state = &state;
        InitializeTestState( state, sequence );
        const UInt32 count = 1031;
        GUCEF::vector< TFloat64Point3D > points( count );
        GUCEF::vector< TFloat64Vector3D > vectors( count );
        GUCEF::vector< TFloat64Matrix4D > matrices( count );
        GUCEF::vector< TFloat64Matrix4D > others( count );
        GUCEF::vector< TFixed32Point3D > fixedPoints( count );
        GUCEF::vector< TFixed32Matrix4D > fixedMatrices( count );
        GUCEF::vector< TFixed32Matrix4D > fixedOthers( count );
        for ( UInt32 i=0; i<count; ++i )
        {
            Float64 v[ 16 ];
            TFixed32 f[ 16 ];
            TFixed32 g[ 16 ];
            for ( UInt32 c=0; c<16; ++c )
            {
                v[ c ] = sequence.NextFloat64( -10.0, 10.0 );
                f[ c ] = TFixed32::FromRaw( (Int32) ( sequence.NextFloat64( -8.0, 8.0 ) * 65536.0 ) );
                g[ c ] = TFixed32::FromRaw( (Int32) ( sequence.NextFloat64( -8.0, 8.0 ) * 65536.0 ) );
            }
            points[ i ] = TFloat64Point3D( v[ 0 ], v[ 1 ], v[ 2 ] );
            vectors[ i ] = TFloat64Vector3D( v[ 3 ], v[ 4 ], v[ 5 ] );
            matrices[ i ] = TFloat64Matrix4D::FromColumnMajor( v );
            others[ i ] = matrices[ i ].Transposed();
            fixedPoints[ i ] = TFixed32Point3D( f[ 0 ], f[ 1 ], f[ 2 ] );
            fixedMatrices[ i ] = TFixed32Matrix4D::FromColumnMajor( f );
            fixedOthers[ i ] = TFixed32Matrix4D::FromColumnMajor( g );
        }

        const UInt32 flagSets[ 3 ] = { 0, MATHBATCH_FORCE_SCALAR, MATHBATCH_REALTIME | MATHBATCH_DETERMINISTIC };
        for ( UInt32 fs=0; fs<3; ++fs )
        {
            const UInt32 flags = flagSets[ fs ];
            bool ok = true;
            GUCEF::vector< TFloat64Point3D > outPoints( count );
            GUCEF::vector< TFloat64Vector3D > outVectors( count );
            GUCEF::vector< TFloat64Matrix4D > outMatrices( count );
            TransformPoints( state.matrix64, &points[ 0 ], &outPoints[ 0 ], count, flags );
            TransformVectors( state.matrix64, &vectors[ 0 ], &outVectors[ 0 ], count, flags );
            MultiplyMatrices( &matrices[ 0 ], &others[ 0 ], &outMatrices[ 0 ], count, flags );
            for ( UInt32 i=0; i<count; ++i )
            {
                const TFloat64Point3D expectedPoint = state.matrix64.TransformPoint( points[ i ] );
                const TFloat64Vector3D expectedVector = state.matrix64.TransformVector( vectors[ i ] );
                const TFloat64Matrix4D expectedMatrix = matrices[ i ].Multiply( others[ i ] );
                ok = ok && 0 == memcmp( &expectedPoint, &outPoints[ i ], sizeof( expectedPoint ) );
                ok = ok && 0 == memcmp( &expectedVector, &outVectors[ i ], sizeof( expectedVector ) );
                ok = ok && 0 == memcmp( &expectedMatrix, &outMatrices[ i ], sizeof( expectedMatrix ) );
            }
            ASSERT_TRUE( ok );

            // in place
            GUCEF::vector< TFloat64Point3D > inPlace( points );
            TransformPoints( state.matrix64, &inPlace[ 0 ], &inPlace[ 0 ], count, flags );
            ASSERT_TRUE( 0 == memcmp( &inPlace[ 0 ], &outPoints[ 0 ], count * sizeof( TFloat64Point3D ) ) );
            GUCEF::vector< TFloat64Matrix4D > inPlaceMatrices( matrices );
            MultiplyMatrices( &inPlaceMatrices[ 0 ], &others[ 0 ], &inPlaceMatrices[ 0 ], count, flags );
            ASSERT_TRUE( 0 == memcmp( &inPlaceMatrices[ 0 ], &outMatrices[ 0 ], count * sizeof( TFloat64Matrix4D ) ) );

            // Q16.16: every call is deterministic, whatever the flags
            GUCEF::vector< TFixed32Point3D > outFixedPoints( count );
            GUCEF::vector< TFixed32Matrix4D > outFixedMatrices( count );
            TransformPoints( state.matrixFx, &fixedPoints[ 0 ], &outFixedPoints[ 0 ], count, flags );
            MultiplyMatrices( &fixedMatrices[ 0 ], &fixedOthers[ 0 ], &outFixedMatrices[ 0 ], count, flags );
            for ( UInt32 i=0; i<count; ++i )
            {
                const TFixed32Point3D expectedPoint = state.matrixFx.TransformPoint( fixedPoints[ i ] );
                const TFixed32Matrix4D expectedMatrix = fixedMatrices[ i ].Multiply( fixedOthers[ i ] );
                ok = ok && 0 == memcmp( &expectedPoint, &outFixedPoints[ i ], sizeof( expectedPoint ) );
                ok = ok && 0 == memcmp( &expectedMatrix, &outFixedMatrices[ i ], sizeof( expectedMatrix ) );
            }
            ASSERT_TRUE( ok );
        }
        g_state = GUCEF_NULL;
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BatchKernels: the VBAP gains and the noise fields equal the classes sample by sample" )
    try
    {
        MathReference::CSeededSequence sequence( KernelSeed ^ 0x7B4F );
        STestState state;
        g_state = &state;
        InitializeTestState( state, sequence );
        const UInt32 sources = 1027;
        GUCEF::vector< TFloat32Radian > azimuths( sources );
        for ( UInt32 i=0; i<sources; ++i )
        {
            const Float64 azimuth = sequence.NextFloat64( -20.0, 20.0 );
            azimuths[ i ] = TFloat32Radian( (Float32) azimuth );
        }
        azimuths[ 3 ] = TFloat32Radian( 0.0f );
        azimuths[ 11 ] = TFloat32Radian( 0.7853982f );      // at a speaker of the quad layout

        const UInt32 flagSets[ 3 ] = { 0, MATHBATCH_FORCE_SCALAR, MATHBATCH_REALTIME | MATHBATCH_DETERMINISTIC };
        for ( UInt32 fs=0; fs<3; ++fs )
        {
            const UInt32 flags = flagSets[ fs ];
            for ( UInt32 l=0; l<VbapLayoutCount; ++l )
            {
                const TFloat32Vbap2D& vbap = state.vbap[ l ];
                const UInt32 speakers = vbap.GetSpeakerCount();
                for ( UInt32 pw=0; pw<4; ++pw )
                {
                    GUCEF::vector< Float32 > gains( sources * speakers, -7.0f );
                    ComputeVbapGains2D( vbap, &azimuths[ 0 ], &gains[ 0 ], sources, TargetPowers[ pw ], flags );
                    bool ok = true;
                    for ( UInt32 i=0; i<sources; ++i )
                    {
                        Float32 expected[ TFloat32Vbap2D::MaxSpeakers ];
                        vbap.ComputeGains( azimuths[ i ], expected, TargetPowers[ pw ] );
                        ok = ok && 0 == memcmp( expected, &gains[ (size_t) i * speakers ], speakers * sizeof( Float32 ) );
                    }
                    ASSERT_TRUE( ok );
                }
            }
            // a panner that is not prepared writes nothing
            const TFloat32Vbap2D unprepared;
            Float32 untouched[ 4 ] = { 5.0f, 5.0f, 5.0f, 5.0f };
            ComputeVbapGains2D( unprepared, &azimuths[ 0 ], untouched, 4, 2.0f, flags );
            ASSERT_TRUE( 5.0f == untouched[ 0 ] && 5.0f == untouched[ 3 ] );

            // noise fields: grids of awkward width, every class, both dimensions
            const UInt32 width = 37;
            const UInt32 height = 11;
            const UInt32 depth = 5;
            GUCEF::vector< Float32 > field2( width * height );
            GUCEF::vector< Float32 > field3( width * height * depth );
            const Float32 ox = -3.7f, oy = 1.9f, oz = 0.4f, sx = 0.211f, sy = 0.137f, sz = 0.0719f;
            bool ok = true;
            for ( UInt32 t=0; t<MATHNOISE_COUNT; ++t )
            {
                switch ( t )
                {
                    case MATHNOISE_PERLIN:
                        FillNoise2D( state.perlin, ox, oy, sx, sy, width, height, &field2[ 0 ], flags );
                        FillNoise3D( state.perlin, ox, oy, oz, sx, sy, sz, width, height, depth, &field3[ 0 ], flags );
                        break;
                    case MATHNOISE_VALUE:
                        FillNoise2D( state.value, ox, oy, sx, sy, width, height, &field2[ 0 ], flags );
                        FillNoise3D( state.value, ox, oy, oz, sx, sy, sz, width, height, depth, &field3[ 0 ], flags );
                        break;
                    case MATHNOISE_OPENSIMPLEX2:
                        FillNoise2D( state.simplex, ox, oy, sx, sy, width, height, &field2[ 0 ], flags );
                        FillNoise3D( state.simplex, ox, oy, oz, sx, sy, sz, width, height, depth, &field3[ 0 ], flags );
                        break;
                    default:
                        FillNoise2D( state.worley, ox, oy, sx, sy, width, height, &field2[ 0 ], flags );
                        FillNoise3D( state.worley, ox, oy, oz, sx, sy, sz, width, height, depth, &field3[ 0 ], flags );
                        break;
                }
                for ( UInt32 iz=0; iz<depth; ++iz )
                {
                    for ( UInt32 iy=0; iy<height; ++iy )
                    {
                        for ( UInt32 ix=0; ix<width; ++ix )
                        {
                            const Float32 x = ox + (Float32) ix * sx;
                            const Float32 y = oy + (Float32) iy * sy;
                            const Float32 z = oz + (Float32) iz * sz;
                            Float32 expected2 = 0.0f;
                            Float32 expected3 = 0.0f;
                            switch ( t )
                            {
                                case MATHNOISE_PERLIN: expected2 = state.perlin.Noise2D( x, y ); expected3 = state.perlin.Noise3D( x, y, z ); break;
                                case MATHNOISE_VALUE: expected2 = state.value.Noise2D( x, y ); expected3 = state.value.Noise3D( x, y, z ); break;
                                case MATHNOISE_OPENSIMPLEX2: expected2 = state.simplex.Noise2D( x, y ); expected3 = state.simplex.Noise3D( x, y, z ); break;
                                default: expected2 = state.worley.Noise2D( x, y ); expected3 = state.worley.Noise3D( x, y, z ); break;
                            }
                            if ( 0 == iz )
                                ok = ok && 0 == memcmp( &expected2, &field2[ (size_t) iy * width + ix ], sizeof( Float32 ) );
                            ok = ok && 0 == memcmp( &expected3, &field3[ ( (size_t) iz * height + iy ) * width + ix ], sizeof( Float32 ) );
                        }
                    }
                }
                ASSERT_TRUE( ok );
            }
            // empty grids write nothing
            Float32 sentinel = 9.0f;
            FillNoise2D( state.perlin, 0.0f, 0.0f, 1.0f, 1.0f, 0, 5, &sentinel, flags );
            FillNoise3D( state.perlin, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 3, 3, 0, &sentinel, flags );
            ASSERT_TRUE( 9.0f == sentinel );
        }
        g_state = GUCEF_NULL;
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BatchKernels: half conversions equal CFloat16 for every half and sampled floats (all floats with GUCEF_TEST_MATH_EXHAUSTIVE=1)" )
    try
    {
        // every binary16 pattern, NaN included, out of place and in place
        GUCEF::vector< CFloat16 > halves( 65536 );
        for ( UInt32 h=0; h<65536; ++h )
            halves[ h ] = CFloat16::FromBits( (UInt16) h );
        GUCEF::vector< Float32 > floats( 65536 );
        ConvertFloat16ToFloat32( &halves[ 0 ], &floats[ 0 ], 65536 );
        bool ok = true;
        for ( UInt32 h=0; h<65536; ++h )
        {
            const Float32 expected = halves[ h ].ToFloat32();
            UInt32 expectedWord = 0;
            UInt32 actualWord = 0;
            memcpy( &expectedWord, &expected, sizeof( expectedWord ) );
            memcpy( &actualWord, &floats[ h ], sizeof( actualWord ) );
            const bool same = SameFloatBitsOfConversion( expectedWord, actualWord );
            if ( !same && ok )
            {
                UInt32 expectedBits = 0;
                UInt32 actualBits = 0;
                memcpy( &expectedBits, &expected, sizeof( expectedBits ) );
                memcpy( &actualBits, &floats[ h ], sizeof( actualBits ) );
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "BatchKernels: half " << h << " converts to bits " << actualBits << " in the batch, CFloat16 gives " << expectedBits );
            }
            ok = ok && same;
        }
        ASSERT_TRUE( ok );
        GUCEF::vector< Float32 > inPlace( 65536 );
        memcpy( &inPlace[ 0 ], &halves[ 0 ], 65536 * sizeof( CFloat16 ) );
        ConvertFloat16ToFloat32( reinterpret_cast< const CFloat16* >( &inPlace[ 0 ] ), &inPlace[ 0 ], 65536 );
        ASSERT_TRUE( 0 == memcmp( &inPlace[ 0 ], &floats[ 0 ], 65536 * sizeof( Float32 ) ) );

        // floats: a stride through all 2^32 bit patterns (or all of them), in blocks
        const bool exhaustive = MathReference::ExhaustiveTestsEnabled();
        const UInt64 step = exhaustive ? 1 : 4099;
        const UInt32 blockSize = 65536;
        GUCEF::vector< Float32 > block( blockSize );
        GUCEF::vector< CFloat16 > converted( blockSize );
        UInt64 checked = 0;
        UInt64 next = 0;
        while ( ok && next < 0x100000000ULL )
        {
            UInt32 n = 0;
            for ( ; n<blockSize && next < 0x100000000ULL; ++n, next += step )
                block[ n ] = FromBits( (UInt32) next );
            ConvertFloat32ToFloat16( &block[ 0 ], &converted[ 0 ], n );
            for ( UInt32 i=0; i<n; ++i )
                ok = ok && SameHalfBitsOfConversion( CFloat16::FromFloat32( block[ i ] ).GetBits(), converted[ i ].GetBits() );
            checked += n;
        }
        ASSERT_TRUE( ok );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "BatchKernels: " << CORE::ToString( checked ) << " float to half conversions checked" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BatchKernels: hashes of the batch results for the comparison with the GUCEF_MATH_SIMD=NONE build" )
    try
    {
        const CMathBatchDispatcher& dispatcher = CMathGlobal::Instance()->GetBatchDispatcher();
        MathReference::CSeededSequence matrixSequence( KernelSeed ^ 0x99 );
        STestState state;
        g_state = &state;
        InitializeTestState( state, matrixSequence );
        const TFloat32Matrix4D m = state.matrix;
        const UInt32 count = 100000;
        UInt64 total = MathReference::Fnv1aOffsetBasis();
        for ( UInt32 k=0; k<MATHBATCHKERNEL_MATH_COUNT; ++k )
        {
            const SKernelShape shape = ApplyVariant( (TMathBatchKernel) k, MATHBATCHKERNEL_VBAP_GAINS2D_F32 == k ? 2 : 0 );
            MathReference::CSeededSequence sequence( KernelSeed + 7 * k );
            CTestBuffer in( count * shape.inSize + 16, 0 );
            CTestBuffer second( count * ( shape.secondSize > 0 ? shape.secondSize : 1 ) + 16, 0 );
            CTestBuffer out( count * shape.outSize + 16, 0 );
            FillElements( in.Data(), count, shape.inSize, shape.inSize, shape, sequence );
            if ( shape.secondSize > 0 )
                FillElements( second.Data(), count, shape.secondSize, shape.secondSize, shape, sequence );
            const SMathBatchProvider* provider = dispatcher.SelectProvider( shape.kernel, count, 0, in.Data(), out.Data() );
            ASSERT_TRUE( GUCEF_NULL != provider );
            if ( GUCEF_NULL == provider )
                continue;
            ASSERT_TRUE( MATHSTATUS_OK == CallProvider( *provider, shape.kernel, m, in.Data(), shape.inSize, second.Data(), shape.secondSize, out.Data(), shape.outSize, count ) );
            CanonicalizeNaNs( out.Data(), count * shape.outSize, shape );
            const UInt64 hash = HashBytes( out.Data(), count * shape.outSize, MathReference::Fnv1aOffsetBasis() );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "BatchKernels hash " << dispatcher.GetKernelId( shape.kernel ) << " " << CORE::ToString( hash ) );
            total = MathReference::Fnv1aInt64( (Int64) hash, total );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "BatchKernels hash total " << CORE::ToString( total ) );
        g_state = GUCEF_NULL;
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BatchKernels: batch benchmarks per kernel and provider, 2^4 to 2^24 elements (opt-in: GUCEF_TEST_MATH_BENCH=1)" )
    GUCEF_TESTFW_REQUIRE_ENV( bench, "GUCEF_TEST_MATH_BENCH" );
    GUCEF_TESTFW_SKIP_IF( bench != "1", "GUCEF_TEST_MATH_BENCH is not 1" );
    try
    {
        CMathBatchDispatcher best;
        best.RegisterBuiltInProviders();
        CMathBatchDispatcher sse2Only;
        sse2Only.RegisterBuiltInProviders( ~( GUCEF_CPU_ISA_AVX | GUCEF_CPU_ISA_AVX2 | GUCEF_CPU_ISA_F16C | GUCEF_CPU_ISA_AVX512F ) );
        MathReference::CSeededSequence matrixSequence( KernelSeed );
        STestState state;
        g_state = &state;
        InitializeTestState( state, matrixSequence );
        const TFloat32Matrix4D m = state.matrix;
        for ( UInt32 k=0; k<MATHBATCHKERNEL_MATH_COUNT; ++k )
        {
            const SKernelShape shape = ApplyVariant( (TMathBatchKernel) k, MATHBATCHKERNEL_VBAP_GAINS2D_F32 == k ? 2 : 0 );
            GUCEF::vector< SMathBatchProvider > providers;
            SMathBatchProvider scalar;
            CollectProviders( best, sse2Only, shape.kernel, providers, scalar );
            // 64 byte elements stop at 2^20 (64 MB per array) to keep the memory use moderate
            const UInt32 largestPower = shape.outSize >= 64 ? 20 : 24;
            for ( UInt32 power=4; power<=largestPower; power += 4 )
            {
                const UInt32 count = (UInt32) 1 << power;
                MathReference::CSeededSequence sequence( KernelSeed + k );
                CTestBuffer in( count * shape.inSize + 16, 0 );
                CTestBuffer second( count * ( shape.secondSize > 0 ? shape.secondSize : 1 ) + 16, 0 );
                CTestBuffer out( count * shape.outSize + 16, 0 );
                FillElements( in.Data(), count, shape.inSize, shape.inSize, shape, sequence );
                if ( shape.secondSize > 0 )
                    FillElements( second.Data(), count, shape.secondSize, shape.secondSize, shape, sequence );
                const UInt32 repeats = 1 + ( 1u << 24 ) / count / 4;
                CORE::CString line = CORE::CString( best.GetKernelId( shape.kernel ) ) + " 2^" + CORE::ToString( power ) + ":";
                for ( UInt32 p=0; p<providers.size(); ++p )
                {
                    const UInt64 start = MT::PrecisionTickCount();
                    for ( UInt32 r=0; r<repeats; ++r )
                        CallProvider( providers[ p ], shape.kernel, m, in.Data(), shape.inSize, second.Data(), shape.secondSize, out.Data(), shape.outSize, count );
                    const UInt64 end = MT::PrecisionTickCount();
                    const Float64 seconds = (Float64) ( end - start ) / (Float64) MT::PrecisionTimerResolution();
                    const Float64 nanosecondsPerElement = seconds * 1.0e9 / ( (Float64) count * repeats );
                    line += CORE::CString( " " ) + providers[ p ].name + " " + CORE::ToString( nanosecondsPerElement ) + " ns";
                }
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "BatchKernels bench " << line );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
