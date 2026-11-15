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

#ifndef GUCEF_MATH_CQUATERNIONPACKING_H
#define GUCEF_MATH_CQUATERNIONPACKING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTQUATERNION_H
#include "gucefMATH_CTQuaternion.h"
#define GUCEF_MATH_CTQUATERNION_H
#endif /* GUCEF_MATH_CTQUATERNION_H ? */

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
 *  Smallest-three packing of unit quaternions (section 4.6 of the design): the building blocks the DRN replication
 *  item chooses from; the wire formats themselves are decided there.
 *
 *  - The input is normalised; the component with the largest magnitude (lowest index on ties, 0 = x ... 3 = w) is
 *    dropped and its index stored in 2 bits; the quaternion is negated when that component is negative (q and -q are
 *    the same rotation), so the dropped component is >= 0 and is restored as sqrt( 1 - a^2 - b^2 - c^2 ).
 *  - The other three components lie in [-1/sqrt( 2 ), 1/sqrt( 2 )] and are quantised uniformly with B bits each:
 *    code = round( c / step ) + 2^( B - 1 ) - 1 (half away from zero, clamped to 0 .. 2^B - 2) with
 *    step = sqrt( 2 ) / ( 2^B - 2 ) (GetStep): the range is symmetric and 0 is a code, so the identity is exact. The
 *    code 2^B - 1 is not produced and decodes as 2^B - 2.
 *  - Layout, from the most significant used bit down: the index (2 bits), then the three kept components in
 *    ascending component order (B bits each). Pack32: B = 10, bits 31..0; Pack48: B = 15, bits 46..0 of the UInt64
 *    (bit 47 and above zero); Pack64: B = 20, bits 61..0 (bits 63, 62 zero).
 *  - Unpacking renormalises. The zero quaternion packs as the identity.
 *  - Float32 and Float64 overloads, compiled in the module (no floating-point contraction): identical codes on every
 *    platform. Realtime-safe.
 */
class GUCEF_MATH_PUBLIC_CPP CQuaternionPacking
{
    public:

    static UInt32 Pack32( const CTQuaternion< Float32 >& q ) GUCEF_MATH_REALTIME_CONTEXT;
    static UInt32 Pack32( const CTQuaternion< Float64 >& q ) GUCEF_MATH_REALTIME_CONTEXT;

    static UInt64 Pack48( const CTQuaternion< Float32 >& q ) GUCEF_MATH_REALTIME_CONTEXT;
    static UInt64 Pack48( const CTQuaternion< Float64 >& q ) GUCEF_MATH_REALTIME_CONTEXT;

    static UInt64 Pack64( const CTQuaternion< Float32 >& q ) GUCEF_MATH_REALTIME_CONTEXT;
    static UInt64 Pack64( const CTQuaternion< Float64 >& q ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The unit quaternion of a Pack32 code */
    static void Unpack32( UInt32 packed, CTQuaternion< Float32 >& out ) GUCEF_MATH_REALTIME_CONTEXT;
    static void Unpack32( UInt32 packed, CTQuaternion< Float64 >& out ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The unit quaternion of a Pack48 code (bits above 47 are ignored) */
    static void Unpack48( UInt64 packed, CTQuaternion< Float32 >& out ) GUCEF_MATH_REALTIME_CONTEXT;
    static void Unpack48( UInt64 packed, CTQuaternion< Float64 >& out ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The unit quaternion of a Pack64 code (bits 63 and 62 are ignored) */
    static void Unpack64( UInt64 packed, CTQuaternion< Float32 >& out ) GUCEF_MATH_REALTIME_CONTEXT;
    static void Unpack64( UInt64 packed, CTQuaternion< Float64 >& out ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The index ( 0 = x ... 3 = w ) of the dropped largest component stored in a code */
    static UInt32 GetLargestComponentIndex32( UInt32 packed ) GUCEF_MATH_REALTIME_CONTEXT;
    static UInt32 GetLargestComponentIndex48( UInt64 packed ) GUCEF_MATH_REALTIME_CONTEXT;
    static UInt32 GetLargestComponentIndex64( UInt64 packed ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The quantisation step sqrt( 2 ) / ( 2^bitsPerComponent - 2 ) of a kept component (10, 15 or 20 bits) */
    static Float64 GetStep( UInt32 bitsPerComponent ) GUCEF_MATH_REALTIME_CONTEXT;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CQUATERNIONPACKING_H ? */
