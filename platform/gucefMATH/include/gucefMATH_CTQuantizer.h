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

#ifndef GUCEF_MATH_CTQUANTIZER_H
#define GUCEF_MATH_CTQUANTIZER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_PACKINGFUNCTIONS_H
#include "gucefMATH_PackingFunctions.h"
#define GUCEF_MATH_PACKINGFUNCTIONS_H
#endif /* GUCEF_MATH_PACKINGFUNCTIONS_H ? */

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

/**
 *  Uniform quantisation of a value range to an unsigned integer code of 1 to 32 bits (section 4.6 of the design):
 *  the building block for replicating positions, velocities and other bounded values; the wire format is decided by
 *  the replication layer.
 *
 *  - The range [minValue, maxValue] is divided into 2^bits - 1 equal steps ( GetStep() ): code 0 is minValue and the
 *    largest code, 2^bits - 1, is maxValue. Encode() rounds to the nearest code, ties to even; the error of a value in
 *    range is at most half a step ( up to the rounding of the arithmetic, which is done in Float64 ). Values outside
 *    the range clamp to the end codes; NaN encodes as 0.
 *  - Decode() returns the value of a code; codes above the largest code decode as maxValue.
 *  - A reversed range is swapped; an empty range ( min == max ) has step 0, every value encodes as 0 and decodes as min.
 *    Bits are limited to [1, 32]; 32 bits are fully supported ( the codes are UInt32 ).
 *  - T is Float32 or Float64; predefined kinds are exported instantiations compiled without floating-point
 *    contraction. A value type: copyable, no heap, concurrent const use is safe. Realtime-safe.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTQuantizer
{
    public:

    typedef T TValue;

    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::IsFloatingPoint, Quantizers_require_a_floating_point_kind );

    /** The range [0, 1] with 8 bits */
    CTQuantizer( void ) GUCEF_MATH_REALTIME_CONTEXT
        : m_minimum( 0.0 ) , m_maximum( 1.0 ) , m_bits( 8 ) , m_step( 0.0 ) , m_inverseStep( 0.0 )
    {
        Setup( 0.0, 1.0, 8 );
    }

    CTQuantizer( const T& minValue, const T& maxValue, UInt32 bits ) GUCEF_MATH_REALTIME_CONTEXT
        : m_minimum( 0.0 ) , m_maximum( 1.0 ) , m_bits( 8 ) , m_step( 0.0 ) , m_inverseStep( 0.0 )
    {
        Setup( (Float64) minValue, (Float64) maxValue, bits );
    }

    GUCEF_MATH_FORCE_INLINE T GetMinimum( void ) const GUCEF_MATH_REALTIME_CONTEXT { return (T) m_minimum; }
    GUCEF_MATH_FORCE_INLINE T GetMaximum( void ) const GUCEF_MATH_REALTIME_CONTEXT { return (T) m_maximum; }
    GUCEF_MATH_FORCE_INLINE UInt32 GetBits( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_bits; }

    /** The largest code: 2^bits - 1 */
    GUCEF_MATH_FORCE_INLINE UInt32 GetMaxCode( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_bits >= 32 ? 0xFFFFFFFFU : ( ( 1U << m_bits ) - 1U ); }

    /** The distance between neighbouring codes: ( max - min ) / ( 2^bits - 1 ); in the kind T ( the step of a Float32 quantizer of 32 bits is rounded to Float32 ) */
    T GetStep( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The code of the value nearest to 'value'; end codes for values out of range, 0 for NaN */
    UInt32 Encode( const T& value ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The value of a code; maxValue for codes above the largest code */
    T Decode( UInt32 code ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The three components of 'v' as codes of this quantizer */
    void EncodeVector3( const CTVector3D< T >& v, UInt32* out3 ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The vector of three codes */
    CTVector3D< T > DecodeVector3( const UInt32* in3 ) const GUCEF_MATH_REALTIME_CONTEXT;

    private:

    void Setup( Float64 minValue, Float64 maxValue, UInt32 bits ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( maxValue < minValue )
        {
            const Float64 swapped = minValue;
            minValue = maxValue;
            maxValue = swapped;
        }
        m_bits = bits < 1 ? 1 : ( bits > 32 ? 32 : bits );
        m_minimum = minValue;
        m_maximum = maxValue;
        const Float64 steps = (Float64) GetMaxCode();
        const Float64 span = maxValue - minValue;
        if ( span > 0.0 && span - span == 0.0 )
        {
            m_step = span / steps;
            m_inverseStep = steps / span;
        }
        else
        {
            m_step = 0.0;
            m_inverseStep = 0.0;
        }
    }

    Float64 m_minimum;
    Float64 m_maximum;
    UInt32 m_bits;
    Float64 m_step;
    Float64 m_inverseStep;
};

/*-------------------------------------------------------------------------*/

typedef CTQuantizer< Float32 > TFloat32Quantizer;
typedef CTQuantizer< Float64 > TFloat64Quantizer;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTQuantizer< T >::GetStep( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return (T) m_step;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE UInt32
CTQuantizer< T >::Encode( const T& value ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const Float64 v = (Float64) value;
    if ( !( v > m_minimum ) || !( m_inverseStep > 0.0 ) )        // below the range, at its start, NaN, or an empty range
        return 0;
    if ( !( v < m_maximum ) )
        return GetMaxCode();
    const Float64 code = PackingRoundHalfEven( ( v - m_minimum ) * m_inverseStep );
    const Float64 maxCode = (Float64) GetMaxCode();
    return (UInt32) ( code < maxCode ? code : maxCode );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTQuantizer< T >::Decode( UInt32 code ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( code >= GetMaxCode() )
        return (T) ( m_step > 0.0 ? m_maximum : m_minimum );
    return (T) ( m_minimum + (Float64) code * m_step );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE void
CTQuantizer< T >::EncodeVector3( const CTVector3D< T >& v, UInt32* out3 ) const GUCEF_MATH_REALTIME_CONTEXT
{
    out3[ 0 ] = Encode( v.x );
    out3[ 1 ] = Encode( v.y );
    out3[ 2 ] = Encode( v.z );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
CTQuantizer< T >::DecodeVector3( const UInt32* in3 ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return CTVector3D< T >( Decode( in3[ 0 ] ), Decode( in3[ 1 ] ), Decode( in3[ 2 ] ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MATH_CTQUANTIZER_MEMBERS( prefix, kind, vector3 )                                                                \
    prefix kind CTQuantizer< kind >::GetStep( void ) const GUCEF_MATH_REALTIME_CONTEXT;                                        \
    prefix UInt32 CTQuantizer< kind >::Encode( const kind& ) const GUCEF_MATH_REALTIME_CONTEXT;                                \
    prefix kind CTQuantizer< kind >::Decode( UInt32 ) const GUCEF_MATH_REALTIME_CONTEXT;                                       \
    prefix void CTQuantizer< kind >::EncodeVector3( const vector3&, UInt32* ) const GUCEF_MATH_REALTIME_CONTEXT;               \
    prefix vector3 CTQuantizer< kind >::DecodeVector3( const UInt32* ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTQUANTIZER_PREDEFINED( prefix )                                \
    GUCEF_MATH_CTQUANTIZER_MEMBERS( prefix, Float32, TFloat32Vector3D )            \
    GUCEF_MATH_CTQUANTIZER_MEMBERS( prefix, Float64, TFloat64Vector3D )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
GUCEF_MATH_CTQUANTIZER_PREDEFINED( GUCEF_MATH_EXTERN_TEMPLATE )
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTQUANTIZER_H ? */
