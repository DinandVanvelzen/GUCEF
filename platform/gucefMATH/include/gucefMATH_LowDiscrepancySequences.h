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

#ifndef GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H
#define GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <limits>

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

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
 *  Low discrepancy sequences, section 4.9 of the design: deterministic point sets that cover [0, 1) more evenly than
 *  random numbers. Indices start at 0 for the first point of Sobol and Hammersley and at 1 for Halton (index 0 gives
 *  0, the customary skipped first point). All results are in [0, 1) for Float32 and Float64; integer arithmetic and
 *  one exact power-of-two scaling or division make them bit identical on every platform. Realtime-safe.
 */

/** Largest number of dimensions supported by Sobol() */
enum { GUCEF_MATH_SOBOL_DIMENSION_COUNT = 4 };

namespace LowDiscrepancyDetail {

/** The value of an integer fraction in [0, 1) of the kind T: a rounding up to 1 (Float32) is pulled back below 1 */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
ToUnit( Float64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    T result = (T) value;
    if ( !( result < T( 1 ) ) )
        result = T( 1 ) - T( 0.5 ) * std::numeric_limits< T >::epsilon();
    return result;
}

/** Direction numbers of Sobol dimensions 1 to 3 (Joe and Kuo, new-joe-kuo-6.21201 rows d = 2, 3, 4: s = 1 a = 0 m = 1; s = 2 a = 1 m = 1 3; s = 3 a = 1 m = 1 3 1), 32 bit scaled; dimension 0 is the bit reversal */
GUCEF_MATH_FORCE_INLINE const UInt32*
GetSobolDirectionNumbers( UInt32 dimension ) GUCEF_MATH_REALTIME_CONTEXT
{
    static const UInt32 directions[ 3 ][ 32 ] =
    {
        {
            0x80000000U, 0xC0000000U, 0xA0000000U, 0xF0000000U, 0x88000000U, 0xCC000000U, 0xAA000000U, 0xFF000000U,
            0x80800000U, 0xC0C00000U, 0xA0A00000U, 0xF0F00000U, 0x88880000U, 0xCCCC0000U, 0xAAAA0000U, 0xFFFF0000U,
            0x80008000U, 0xC000C000U, 0xA000A000U, 0xF000F000U, 0x88008800U, 0xCC00CC00U, 0xAA00AA00U, 0xFF00FF00U,
            0x80808080U, 0xC0C0C0C0U, 0xA0A0A0A0U, 0xF0F0F0F0U, 0x88888888U, 0xCCCCCCCCU, 0xAAAAAAAAU, 0xFFFFFFFFU
        },
        {
            0x80000000U, 0xC0000000U, 0x60000000U, 0x90000000U, 0xE8000000U, 0x5C000000U, 0x8E000000U, 0xC5000000U,
            0x68800000U, 0x9CC00000U, 0xEE600000U, 0x55900000U, 0x80680000U, 0xC09C0000U, 0x60EE0000U, 0x90550000U,
            0xE8808000U, 0x5CC0C000U, 0x8E606000U, 0xC5909000U, 0x6868E800U, 0x9C9C5C00U, 0xEEEE8E00U, 0x5555C500U,
            0x8000E880U, 0xC0005CC0U, 0x60008E60U, 0x9000C590U, 0xE8006868U, 0x5C009C9CU, 0x8E00EEEEU, 0xC5005555U
        },
        {
            0x80000000U, 0xC0000000U, 0x20000000U, 0x50000000U, 0xF8000000U, 0x74000000U, 0xA2000000U, 0x93000000U,
            0xD8800000U, 0x25400000U, 0x59E00000U, 0xE6D00000U, 0x78080000U, 0xB40C0000U, 0x82020000U, 0xC3050000U,
            0x208F8000U, 0x51474000U, 0xFBEA2000U, 0x75D93000U, 0xA0858800U, 0x914E5400U, 0xDBE79E00U, 0x25DB6D00U,
            0x58800080U, 0xE54000C0U, 0x79E00020U, 0xB6D00050U, 0x800800F8U, 0xC00C0074U, 0x200200A2U, 0x50050093U
        }
    };
    return directions[ dimension - 1 ];
}

}

/*-------------------------------------------------------------------------*/

/** The van der Corput sequence in base 2: the bits of 'index' mirrored around the binary point; 0 for index 0, 0.5 for 1, 0.25 for 2, 0.75 for 3 */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
RadicalInverseBase2( UInt32 index ) GUCEF_MATH_REALTIME_CONTEXT
{
    index = ( index << 16 ) | ( index >> 16 );
    index = ( ( index & 0x55555555U ) << 1 ) | ( ( index & 0xAAAAAAAAU ) >> 1 );
    index = ( ( index & 0x33333333U ) << 2 ) | ( ( index & 0xCCCCCCCCU ) >> 2 );
    index = ( ( index & 0x0F0F0F0FU ) << 4 ) | ( ( index & 0xF0F0F0F0U ) >> 4 );
    index = ( ( index & 0x00FF00FFU ) << 8 ) | ( ( index & 0xFF00FF00U ) >> 8 );
    return LowDiscrepancyDetail::ToUnit< T >( (Float64) index * 2.3283064365386962890625e-10 );    // 2^-32
}

/*-------------------------------------------------------------------------*/

/**
 *  The radical inverse of 'index' in 'base': the digits of the index mirrored around the point. Halton( i, 2 ) =
 *  1/2, 1/4, 3/4, 1/8, 5/8 for i = 1 .. 5; Halton( i, 3 ) = 1/3, 2/3, 1/9 for i = 1 .. 3. A halton point set in d
 *  dimensions uses a different prime base per dimension. A base below 2 gives 0.
 */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
Halton( UInt32 index, UInt32 base ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( base < 2 )
        return T( 0 );
    UInt64 reversed = 0;
    UInt64 scale = 1;
    while ( index != 0 )
    {
        reversed = reversed * base + ( index % base );
        scale *= base;
        index /= base;
    }
    return LowDiscrepancyDetail::ToUnit< T >( (Float64) reversed / (Float64) scale );
}

/*-------------------------------------------------------------------------*/

/**
 *  Sobol point 'index' of 'dimension' ( 0 to 3 ): dimension 0 equals RadicalInverseBase2( index ); dimensions 1 to 3
 *  use the Joe and Kuo direction numbers, so any two dimensions together form a ( t, m, 2 )-net in base 2 over the
 *  first 2^m points. Points come in natural index order (not Gray code order). A dimension of 4 or more gives 0.
 */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
Sobol( UInt32 index, UInt32 dimension ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( dimension == 0 )
        return RadicalInverseBase2< T >( index );
    if ( dimension >= (UInt32) GUCEF_MATH_SOBOL_DIMENSION_COUNT )
        return T( 0 );
    const UInt32* directions = LowDiscrepancyDetail::GetSobolDirectionNumbers( dimension );
    UInt32 result = 0;
    for ( UInt32 bit = 0; index != 0; ++bit, index >>= 1 )
        if ( index & 1 )
            result ^= directions[ bit ];
    return LowDiscrepancyDetail::ToUnit< T >( (Float64) result * 2.3283064365386962890625e-10 );
}

/*-------------------------------------------------------------------------*/

/** The Hammersley point 'index' of 'count': ( index / count, RadicalInverseBase2( index ) ); the zero point for count 0 */
template < typename T >
GUCEF_MATH_FORCE_INLINE CTVector2D< T >
Hammersley2D( UInt32 index, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( count == 0 )
        return CTVector2D< T >( T( 0 ), T( 0 ) );
    return CTVector2D< T >( LowDiscrepancyDetail::ToUnit< T >( (Float64) index / (Float64) count ), RadicalInverseBase2< T >( index ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  Point 'index' of 'count' of the Fibonacci spiral on the unit sphere, half-offset construction: z = 1 - ( 2 index + 1 ) / count,
 *  angle = index times the golden angle. Unit length, nearly uniform, with a minimum distance of about 3.09 / sqrt( count )
 *  between points. The pole ( 0, 0, 1 ) for count 0; index is taken modulo count.
 */
template < typename T >
CTVector3D< T >
FibonacciSphere( UInt32 index, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT;

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
FibonacciSphere( UInt32 index, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( count == 0 )
        return CTVector3D< T >( T( 0 ), T( 0 ), T( 1 ) );
    index = index % count;
    const Float64 z = 1.0 - ( 2.0 * (Float64) index + 1.0 ) / (Float64) count;
    const Float64 radiusSquared = 1.0 - z * z;
    const Float64 radius = radiusSquared > 0.0 ? sqrt( radiusSquared ) : 0.0;
    const Float64 twoPi = 6.28318530717958647692528676655900577;
    const Float64 angle = fmod( (Float64) index * 2.39996322972865332223155550663361385, twoPi );    // the golden angle
    return CTVector3D< T >( (T) ( radius * cos( angle ) ), (T) ( radius * sin( angle ) ), (T) z );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MATH_LOWDISCREPANCY_PREDEFINED( X )  \
    X( Float32, TFloat32Vector3D ) X( Float64, TFloat64Vector3D )

#define GUCEF_MATH_LOWDISCREPANCY_FUNCTIONS( prefix, kind, vector3 )                                    \
    prefix vector3 FibonacciSphere< kind >( UInt32, UInt32 ) GUCEF_MATH_REALTIME_CONTEXT;

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
#define GUCEF_MATH_LOWDISCREPANCY_EXTERN( kind, vector3 ) GUCEF_MATH_LOWDISCREPANCY_FUNCTIONS( GUCEF_MATH_EXTERN_TEMPLATE, kind, vector3 )
GUCEF_MATH_LOWDISCREPANCY_PREDEFINED( GUCEF_MATH_LOWDISCREPANCY_EXTERN )
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H ? */
