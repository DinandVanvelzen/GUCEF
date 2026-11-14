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

#ifndef GUCEF_MATH_PACKINGFUNCTIONS_H
#define GUCEF_MATH_PACKINGFUNCTIONS_H

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
 *  Scalar packing of normalised values into 8 and 16 bit integers, with the conversion rules of Vulkan and
 *  Direct3D. The functions are templates over the floating point kind, the arithmetic is done in Float64 so every
 *  code survives a round trip through Float32. A NaN packs to 0. All functions are realtime safe.
 */

/*-------------------------------------------------------------------------*/

/** Rounds to the nearest integer, ties to the even integer. The input is within +-2^32 */
GUCEF_MATH_FORCE_INLINE Float64
PackingRoundHalfEven( Float64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 floored = floor( value );
    Float64 fraction = value - floored;
    if ( fraction < 0.5 )
        return floored;
    if ( fraction > 0.5 )
        return floored + 1.0;
    // tie: the even neighbour
    return ( fmod( floored, 2.0 ) == 0.0 ) ? floored : floored + 1.0;
}

/*-------------------------------------------------------------------------*/

/** Clamps to [0, 1] and packs as round( x * 255 ) */
template< typename T >
GUCEF_MATH_FORCE_INLINE UInt8
PackUnorm8( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 x = (Float64) value;
    if ( !( x > 0.0 ) ) return 0;          // also NaN
    if ( x >= 1.0 ) return 255;
    return (UInt8) PackingRoundHalfEven( x * 255.0 );
}

/*-------------------------------------------------------------------------*/

/** Clamps to [0, 1] and packs as round( x * 65535 ) */
template< typename T >
GUCEF_MATH_FORCE_INLINE UInt16
PackUnorm16( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 x = (Float64) value;
    if ( !( x > 0.0 ) ) return 0;
    if ( x >= 1.0 ) return 65535;
    return (UInt16) PackingRoundHalfEven( x * 65535.0 );
}

/*-------------------------------------------------------------------------*/

/** code / 255 */
template< typename T >
GUCEF_MATH_FORCE_INLINE T
UnpackUnorm8( UInt8 code ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (T) ( (Float64) code / 255.0 );
}

/*-------------------------------------------------------------------------*/

/** code / 65535 */
template< typename T >
GUCEF_MATH_FORCE_INLINE T
UnpackUnorm16( UInt16 code ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (T) ( (Float64) code / 65535.0 );
}

/*-------------------------------------------------------------------------*/

/** Clamps to [-1, 1] and packs as round( x * 127 ); the code -128 is never produced */
template< typename T >
GUCEF_MATH_FORCE_INLINE Int8
PackSnorm8( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 x = (Float64) value;
    if ( x != x ) return 0;
    if ( x <= -1.0 ) return -127;
    if ( x >= 1.0 ) return 127;
    return (Int8) PackingRoundHalfEven( x * 127.0 );
}

/*-------------------------------------------------------------------------*/

/** Clamps to [-1, 1] and packs as round( x * 32767 ); the code -32768 is never produced */
template< typename T >
GUCEF_MATH_FORCE_INLINE Int16
PackSnorm16( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 x = (Float64) value;
    if ( x != x ) return 0;
    if ( x <= -1.0 ) return -32767;
    if ( x >= 1.0 ) return 32767;
    return (Int16) PackingRoundHalfEven( x * 32767.0 );
}

/*-------------------------------------------------------------------------*/

/** max( code / 127, -1 ): the code -128 decodes to -1 as well */
template< typename T >
GUCEF_MATH_FORCE_INLINE T
UnpackSnorm8( Int8 code ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 x = (Float64) code / 127.0;
    return (T) ( x < -1.0 ? -1.0 : x );
}

/*-------------------------------------------------------------------------*/

/** max( code / 32767, -1 ): the code -32768 decodes to -1 as well */
template< typename T >
GUCEF_MATH_FORCE_INLINE T
UnpackSnorm16( Int16 code ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 x = (Float64) code / 32767.0;
    return (T) ( x < -1.0 ? -1.0 : x );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_PACKINGFUNCTIONS_H ? */
