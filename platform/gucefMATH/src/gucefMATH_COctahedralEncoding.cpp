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

#ifndef GUCEF_MATH_PACKINGFUNCTIONS_H
#include "gucefMATH_PackingFunctions.h"
#define GUCEF_MATH_PACKINGFUNCTIONS_H
#endif /* GUCEF_MATH_PACKINGFUNCTIONS_H ? */

#include "gucefMATH_COctahedralEncoding.h"

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

/** -1 for negative values and negative zero, +1 otherwise: the sign used when folding ( never 0 ) */
GUCEF_MATH_FORCE_INLINE Float64
FoldSign( Float64 v )
{
    return v < 0.0 ? -1.0 : 1.0;
}

GUCEF_MATH_FORCE_INLINE Float64
Absolute( Float64 v )
{
    return v < 0.0 ? -v : v;
}

GUCEF_MATH_FORCE_INLINE Float64
LimitUnit( Float64 v )
{
    return v > 1.0 ? 1.0 : ( v < -1.0 ? -1.0 : v );
}

/** The octahedral components of the direction ( x, y, z ): u, w in [-1, 1] */
void
ProjectDirection( Float64 x, Float64 y, Float64 z, Float64& u, Float64& w )
{
    const Float64 length = Absolute( x ) + Absolute( y ) + Absolute( z );
    if ( !( length > 0.0 ) || length - length != 0.0 )
    {
        u = 0.0;
        w = 0.0;
        return;
    }
    const Float64 px = x / length;
    const Float64 py = y / length;
    if ( z < 0.0 )
    {
        u = ( 1.0 - Absolute( py ) ) * FoldSign( px );
        w = ( 1.0 - Absolute( px ) ) * FoldSign( py );
    }
    else
    {
        u = px;
        w = py;
    }
}

/** The unit direction of the octahedral components */
void
UnprojectDirection( Float64 u, Float64 w, Float64& x, Float64& y, Float64& z )
{
    u = LimitUnit( u );
    w = LimitUnit( w );
    x = u;
    y = w;
    z = 1.0 - Absolute( u ) - Absolute( w );
    if ( z < 0.0 )
    {
        x = ( 1.0 - Absolute( w ) ) * FoldSign( u );
        y = ( 1.0 - Absolute( u ) ) * FoldSign( w );
    }
    const Float64 length = sqrt( x * x + y * y + z * z );
    x = x / length;
    y = y / length;
    z = z / length;
}

/** The code of a component for 'maxCode' ( 127 or 32767 ) as an integer in [-maxCode, maxCode] */
GUCEF_MATH_FORCE_INLINE Int32
Quantise( Float64 component, Float64 maxCode )
{
    return (Int32) PackingRoundHalfEven( LimitUnit( component ) * maxCode );
}

GUCEF_MATH_FORCE_INLINE Int32
LimitCode( Int32 code, Int32 maxCode )
{
    return code > maxCode ? maxCode : ( code < -maxCode ? -maxCode : code );
}

GUCEF_MATH_FORCE_INLINE UInt32
PackCodes( Int32 first, Int32 second, UInt32 bits )
{
    const UInt32 mask = ( 1U << bits ) - 1U;
    return ( ( (UInt32) first & mask ) << bits ) | ( (UInt32) second & mask );
}

GUCEF_MATH_FORCE_INLINE Int32
SignExtend( UInt32 value, UInt32 bits )
{
    const UInt32 sign = 1U << ( bits - 1 );
    return (Int32) ( ( value ^ sign ) - sign );
}

/** Encoding with 'bits' bits per component; 'precise' also tries the neighbouring codes */
UInt32
EncodeDirection( Float64 x, Float64 y, Float64 z, UInt32 bits, bool precise )
{
    const Int32 maxCode = (Int32) ( ( 1U << ( bits - 1 ) ) - 1U );
    const Float64 scale = (Float64) maxCode;
    Float64 u, w;
    ProjectDirection( x, y, z, u, w );
    u = LimitUnit( u );
    w = LimitUnit( w );
    if ( !precise )
        return PackCodes( Quantise( u, scale ), Quantise( w, scale ), bits );

    // the input direction normalised, to compare angles
    const Float64 length = sqrt( x * x + y * y + z * z );
    const Float64 nx = length > 0.0 ? x / length : 0.0;
    const Float64 ny = length > 0.0 ? y / length : 0.0;
    const Float64 nz = length > 0.0 ? z / length : 1.0;

    const Int32 baseU = (Int32) floor( u * scale );
    const Int32 baseW = (Int32) floor( w * scale );
    Int32 bestU = Quantise( u, scale );
    Int32 bestW = Quantise( w, scale );
    Float64 bestDot = -2.0;
    for ( UInt32 candidate=0; candidate<4; ++candidate )
    {
        const Int32 cu = LimitCode( baseU + (Int32) ( candidate & 1 ), maxCode );
        const Int32 cw = LimitCode( baseW + (Int32) ( candidate >> 1 ), maxCode );
        Float64 dx, dy, dz;
        UnprojectDirection( (Float64) cu / scale, (Float64) cw / scale, dx, dy, dz );
        const Float64 dot = dx * nx + dy * ny + dz * nz;
        if ( dot > bestDot )
        {
            bestDot = dot;
            bestU = cu;
            bestW = cw;
        }
    }
    return PackCodes( bestU, bestW, bits );
}

void
DecodeDirection( UInt32 code, UInt32 bits, Float64& x, Float64& y, Float64& z )
{
    const Int32 maxCode = (Int32) ( ( 1U << ( bits - 1 ) ) - 1U );
    const UInt32 mask = ( 1U << bits ) - 1U;
    const Int32 first = SignExtend( ( code >> bits ) & mask, bits );
    const Int32 second = SignExtend( code & mask, bits );
    const Float64 scale = (Float64) maxCode;
    // the code -( maxCode + 1 ) is never produced; it decodes like -maxCode
    UnprojectDirection( (Float64) LimitCode( first, maxCode ) / scale, (Float64) LimitCode( second, maxCode ) / scale, x, y, z );
}

}

/*-------------------------------------------------------------------------*/

void
COctahedralEncoding::Project( const CTVector3D< Float32 >& v, Float32& u, Float32& w ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 a, b;
    ProjectDirection( v.x, v.y, v.z, a, b );
    u = (Float32) a;
    w = (Float32) b;
}

void
COctahedralEncoding::Project( const CTVector3D< Float64 >& v, Float64& u, Float64& w ) GUCEF_MATH_REALTIME_CONTEXT
{
    ProjectDirection( v.x, v.y, v.z, u, w );
}

void
COctahedralEncoding::Unproject( Float32 u, Float32 w, CTVector3D< Float32 >& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 x, y, z;
    UnprojectDirection( u, w, x, y, z );
    out = CTVector3D< Float32 >( (Float32) x, (Float32) y, (Float32) z );
}

void
COctahedralEncoding::Unproject( Float64 u, Float64 w, CTVector3D< Float64 >& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 x, y, z;
    UnprojectDirection( u, w, x, y, z );
    out = CTVector3D< Float64 >( x, y, z );
}

UInt16 COctahedralEncoding::Encode16( const CTVector3D< Float32 >& v ) GUCEF_MATH_REALTIME_CONTEXT { return (UInt16) EncodeDirection( v.x, v.y, v.z, 8, false ); }
UInt16 COctahedralEncoding::Encode16( const CTVector3D< Float64 >& v ) GUCEF_MATH_REALTIME_CONTEXT { return (UInt16) EncodeDirection( v.x, v.y, v.z, 8, false ); }
UInt32 COctahedralEncoding::Encode32( const CTVector3D< Float32 >& v ) GUCEF_MATH_REALTIME_CONTEXT { return EncodeDirection( v.x, v.y, v.z, 16, false ); }
UInt32 COctahedralEncoding::Encode32( const CTVector3D< Float64 >& v ) GUCEF_MATH_REALTIME_CONTEXT { return EncodeDirection( v.x, v.y, v.z, 16, false ); }
UInt16 COctahedralEncoding::EncodePrecise16( const CTVector3D< Float32 >& v ) GUCEF_MATH_REALTIME_CONTEXT { return (UInt16) EncodeDirection( v.x, v.y, v.z, 8, true ); }
UInt16 COctahedralEncoding::EncodePrecise16( const CTVector3D< Float64 >& v ) GUCEF_MATH_REALTIME_CONTEXT { return (UInt16) EncodeDirection( v.x, v.y, v.z, 8, true ); }
UInt32 COctahedralEncoding::EncodePrecise32( const CTVector3D< Float32 >& v ) GUCEF_MATH_REALTIME_CONTEXT { return EncodeDirection( v.x, v.y, v.z, 16, true ); }
UInt32 COctahedralEncoding::EncodePrecise32( const CTVector3D< Float64 >& v ) GUCEF_MATH_REALTIME_CONTEXT { return EncodeDirection( v.x, v.y, v.z, 16, true ); }

void
COctahedralEncoding::Decode16( UInt16 code, CTVector3D< Float32 >& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 x, y, z;
    DecodeDirection( code, 8, x, y, z );
    out = CTVector3D< Float32 >( (Float32) x, (Float32) y, (Float32) z );
}

void
COctahedralEncoding::Decode16( UInt16 code, CTVector3D< Float64 >& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 x, y, z;
    DecodeDirection( code, 8, x, y, z );
    out = CTVector3D< Float64 >( x, y, z );
}

void
COctahedralEncoding::Decode32( UInt32 code, CTVector3D< Float32 >& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 x, y, z;
    DecodeDirection( code, 16, x, y, z );
    out = CTVector3D< Float32 >( (Float32) x, (Float32) y, (Float32) z );
}

void
COctahedralEncoding::Decode32( UInt32 code, CTVector3D< Float64 >& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 x, y, z;
    DecodeDirection( code, 16, x, y, z );
    out = CTVector3D< Float64 >( x, y, z );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
