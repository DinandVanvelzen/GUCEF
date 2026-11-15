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

#include <math.h>

#ifndef GUCEF_MATH_CQUATERNIONPACKING_H
#include "gucefMATH_CQuaternionPacking.h"
#define GUCEF_MATH_CQUATERNIONPACKING_H
#endif /* GUCEF_MATH_CQUATERNIONPACKING_H ? */

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

/** 1 / sqrt( 2 ): the largest magnitude of a component that is not the largest of a unit quaternion */
const Float64 g_inverseSqrt2 = 0.70710678118654752440084436210484903928;

/*-------------------------------------------------------------------------*/

/** Smallest-three code with 'bits' bits per kept component; the index sits above the components */
UInt64
PackSmallestThree( const Float64 ( &input )[ 4 ], UInt32 bits ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 q[ 4 ];
    Float64 length = 0.0;
    {
        Float64 sum = 0.0;
        for ( UInt32 i=0; i<4; ++i )
        {
            Float64 product = input[ i ] * input[ i ];
            sum = sum + product;
        }
        length = sqrt( sum );
    }
    if ( !( length > 0.0 ) || !( length == length ) || length - length != 0.0 )
    {
        q[ 0 ] = 0.0; q[ 1 ] = 0.0; q[ 2 ] = 0.0; q[ 3 ] = 1.0;
    }
    else
    {
        for ( UInt32 i=0; i<4; ++i )
            q[ i ] = input[ i ] / length;
    }

    UInt32 largest = 0;
    for ( UInt32 i=1; i<4; ++i )
        if ( fabs( q[ i ] ) > fabs( q[ largest ] ) )
            largest = i;
    const bool negate = q[ largest ] < 0.0;

    // codes 0 .. 2^B - 2 with the middle code 2^( B - 1 ) - 1 for 0, so that zero (the identity) is exact
    const Int64 maxCode = ( (Int64) 1 << bits ) - 2;
    const Int64 middle = maxCode / 2;
    const Float64 inverseStep = (Float64) maxCode / ( 2.0 * g_inverseSqrt2 );
    UInt64 packed = (UInt64) largest;
    for ( UInt32 i=0; i<4; ++i )
    {
        if ( i == largest )
            continue;
        const Float64 c = negate ? -q[ i ] : q[ i ];
        const Float64 scaled = c * inverseStep;
        // round half away from zero (symmetric), clamp to the code range
        Float64 rounded = scaled < 0.0 ? -floor( 0.5 - scaled ) : floor( scaled + 0.5 );
        Int64 code = (Int64) rounded + middle;
        if ( code < 0 )
            code = 0;
        if ( code > maxCode )
            code = maxCode;
        packed = ( packed << bits ) | (UInt64) code;
    }
    return packed;
}

/*-------------------------------------------------------------------------*/

/** The renormalised unit quaternion of a smallest-three code */
void
UnpackSmallestThree( UInt64 packed, UInt32 bits, Float64 ( &q )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
{
    const Int64 maxCode = ( (Int64) 1 << bits ) - 2;
    const Int64 middle = maxCode / 2;
    const Float64 step = ( 2.0 * g_inverseSqrt2 ) / (Float64) maxCode;
    const UInt32 largest = (UInt32) ( ( packed >> ( 3 * bits ) ) & 3 );
    const UInt64 mask = ( (UInt64) 1 << bits ) - 1;
    Float64 sum = 0.0;
    UInt32 shift = 3 * bits;
    for ( UInt32 i=0; i<4; ++i )
    {
        if ( i == largest )
            continue;
        shift -= bits;
        Int64 code = (Int64) ( ( packed >> shift ) & mask );
        if ( code > maxCode )
            code = maxCode;
        q[ i ] = (Float64) ( code - middle ) * step;
        const Float64 product = q[ i ] * q[ i ];
        sum = sum + product;
    }
    const Float64 remainder = 1.0 - sum;
    q[ largest ] = remainder > 0.0 ? sqrt( remainder ) : 0.0;

    Float64 lengthSquared = 0.0;
    for ( UInt32 i=0; i<4; ++i )
    {
        const Float64 product = q[ i ] * q[ i ];
        lengthSquared = lengthSquared + product;
    }
    const Float64 length = sqrt( lengthSquared );
    for ( UInt32 i=0; i<4; ++i )
        q[ i ] = q[ i ] / length;
}

/*-------------------------------------------------------------------------*/

template < typename T >
inline void
ToComponents( const CTQuaternion< T >& q, Float64 ( &out )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
{
    out[ 0 ] = (Float64) q.x;
    out[ 1 ] = (Float64) q.y;
    out[ 2 ] = (Float64) q.z;
    out[ 3 ] = (Float64) q.w;
}

/*-------------------------------------------------------------------------*/

template < typename T >
inline void
FromComponents( const Float64 ( &in )[ 4 ], CTQuaternion< T >& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    out = CTQuaternion< T >::FromXYZW( (T) in[ 0 ], (T) in[ 1 ], (T) in[ 2 ], (T) in[ 3 ] );
}

/*-------------------------------------------------------------------------*/

template < typename T >
inline UInt64
Pack( const CTQuaternion< T >& q, UInt32 bits ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 components[ 4 ];
    ToComponents( q, components );
    return PackSmallestThree( components, bits );
}

/*-------------------------------------------------------------------------*/

template < typename T >
inline void
Unpack( UInt64 packed, UInt32 bits, CTQuaternion< T >& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 components[ 4 ];
    UnpackSmallestThree( packed, bits, components );
    FromComponents( components, out );
}

}  /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32 CQuaternionPacking::Pack32( const CTQuaternion< Float32 >& q ) GUCEF_MATH_REALTIME_CONTEXT { return (UInt32) Pack( q, 10 ); }
UInt32 CQuaternionPacking::Pack32( const CTQuaternion< Float64 >& q ) GUCEF_MATH_REALTIME_CONTEXT { return (UInt32) Pack( q, 10 ); }
UInt64 CQuaternionPacking::Pack48( const CTQuaternion< Float32 >& q ) GUCEF_MATH_REALTIME_CONTEXT { return Pack( q, 15 ); }
UInt64 CQuaternionPacking::Pack48( const CTQuaternion< Float64 >& q ) GUCEF_MATH_REALTIME_CONTEXT { return Pack( q, 15 ); }
UInt64 CQuaternionPacking::Pack64( const CTQuaternion< Float32 >& q ) GUCEF_MATH_REALTIME_CONTEXT { return Pack( q, 20 ); }
UInt64 CQuaternionPacking::Pack64( const CTQuaternion< Float64 >& q ) GUCEF_MATH_REALTIME_CONTEXT { return Pack( q, 20 ); }

/*-------------------------------------------------------------------------*/

void CQuaternionPacking::Unpack32( UInt32 packed, CTQuaternion< Float32 >& out ) GUCEF_MATH_REALTIME_CONTEXT { Unpack( packed, 10, out ); }
void CQuaternionPacking::Unpack32( UInt32 packed, CTQuaternion< Float64 >& out ) GUCEF_MATH_REALTIME_CONTEXT { Unpack( packed, 10, out ); }
void CQuaternionPacking::Unpack48( UInt64 packed, CTQuaternion< Float32 >& out ) GUCEF_MATH_REALTIME_CONTEXT { Unpack( packed, 15, out ); }
void CQuaternionPacking::Unpack48( UInt64 packed, CTQuaternion< Float64 >& out ) GUCEF_MATH_REALTIME_CONTEXT { Unpack( packed, 15, out ); }
void CQuaternionPacking::Unpack64( UInt64 packed, CTQuaternion< Float32 >& out ) GUCEF_MATH_REALTIME_CONTEXT { Unpack( packed, 20, out ); }
void CQuaternionPacking::Unpack64( UInt64 packed, CTQuaternion< Float64 >& out ) GUCEF_MATH_REALTIME_CONTEXT { Unpack( packed, 20, out ); }

/*-------------------------------------------------------------------------*/

UInt32 CQuaternionPacking::GetLargestComponentIndex32( UInt32 packed ) GUCEF_MATH_REALTIME_CONTEXT { return ( packed >> 30 ) & 3; }
UInt32 CQuaternionPacking::GetLargestComponentIndex48( UInt64 packed ) GUCEF_MATH_REALTIME_CONTEXT { return (UInt32) ( ( packed >> 45 ) & 3 ); }
UInt32 CQuaternionPacking::GetLargestComponentIndex64( UInt64 packed ) GUCEF_MATH_REALTIME_CONTEXT { return (UInt32) ( ( packed >> 60 ) & 3 ); }

/*-------------------------------------------------------------------------*/

Float64
CQuaternionPacking::GetStep( UInt32 bitsPerComponent ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == bitsPerComponent || bitsPerComponent > 30 )
        return 0.0;
    return ( 2.0 * g_inverseSqrt2 ) / (Float64) ( ( (UInt64) 1 << bitsPerComponent ) - 2 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
