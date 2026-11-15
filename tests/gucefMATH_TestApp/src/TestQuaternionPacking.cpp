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
#include <math.h>
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_MATH_CQUATERNIONPACKING_H
#include "gucefMATH_CQuaternionPacking.h"
#define GUCEF_MATH_CQUATERNIONPACKING_H
#endif /* GUCEF_MATH_CQUATERNIONPACKING_H ? */

#include "TestMathReference.h"
#include "TestQuaternionPacking.h"

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

static TFloat64Quaternion
RandomUnitQuaternion( MathReference::CSeededSequence& generator )
{
    for ( ;; )
    {
        Float64 c[ 4 ];
        Float64 sum = 0.0;
        for ( UInt32 i=0; i<4; ++i )
        {
            c[ i ] = generator.NextFloat64( -1.0, 1.0 );
            sum += c[ i ] * c[ i ];
        }
        if ( sum > 1e-4 && sum <= 1.0 )
        {
            Float64 n = sqrt( sum );
            return TFloat64Quaternion::FromXYZW( c[ 0 ] / n, c[ 1 ] / n, c[ 2 ] / n, c[ 3 ] / n );
        }
    }
}

/*-------------------------------------------------------------------------*/

/** The index of the largest magnitude component ( 0 = x ... 3 = w ), lowest index on ties */
static UInt32
LargestIndex( const TFloat64Quaternion& q )
{
    const Float64 c[ 4 ] = { fabs( q.x ), fabs( q.y ), fabs( q.z ), fabs( q.w ) };
    UInt32 largest = 0;
    for ( UInt32 i=1; i<4; ++i )
        if ( c[ i ] > c[ largest ] )
            largest = i;
    return largest;
}

/*-------------------------------------------------------------------------*/

static Float64
Component( const TFloat64Quaternion& q, UInt32 index )
{
    const Float64 c[ 4 ] = { q.x, q.y, q.z, q.w };
    return c[ index ];
}

/*-------------------------------------------------------------------------*/

/** Calls the packing functions from a realtime context */
static UInt64
RealtimePackingWork( const TFloat32Quaternion& q ) GUCEF_MATH_REALTIME_CONTEXT
{
    TFloat32Quaternion a;
    TFloat64Quaternion b;
    CQuaternionPacking::Unpack32( CQuaternionPacking::Pack32( q ), a );
    CQuaternionPacking::Unpack48( CQuaternionPacking::Pack48( a ), b );
    CQuaternionPacking::Unpack64( CQuaternionPacking::Pack64( b ), a );
    return CQuaternionPacking::Pack64( a ) + CQuaternionPacking::GetLargestComponentIndex64( CQuaternionPacking::Pack64( b ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformQuaternionPackingTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "QuaternionPacking" );

    GUCEF_TESTFW_TESTCASE( "QuaternionPacking: layout of the codes, steps, identity and zero input" )
    try
    {
        ASSERT_TRUE( fabs( CQuaternionPacking::GetStep( 10 ) - sqrt( 2.0 ) / 1022.0 ) < 1e-18 && fabs( CQuaternionPacking::GetStep( 20 ) - sqrt( 2.0 ) / 1048574.0 ) < 1e-20 );
        // identity: index 3 (w), the kept components 0 encode as the middle code 2^( B - 1 ) - 1
        const UInt32 identity32 = ( 3u << 30 ) | ( 511u << 20 ) | ( 511u << 10 ) | 511u;
        ASSERT_TRUE( identity32 == CQuaternionPacking::Pack32( TFloat64Quaternion() ) && identity32 == CQuaternionPacking::Pack32( TFloat32Quaternion() ) );
        const UInt64 half15 = 16383;
        ASSERT_TRUE( ( ( (UInt64) 3 << 45 ) | ( half15 << 30 ) | ( half15 << 15 ) | half15 ) == CQuaternionPacking::Pack48( TFloat64Quaternion() ) );
        const UInt64 half20 = 524287;
        ASSERT_TRUE( ( ( (UInt64) 3 << 60 ) | ( half20 << 40 ) | ( half20 << 20 ) | half20 ) == CQuaternionPacking::Pack64( TFloat64Quaternion() ) );
        // the zero quaternion packs as the identity; a non-unit quaternion as its normalisation
        ASSERT_TRUE( identity32 == CQuaternionPacking::Pack32( TFloat64Quaternion::FromXYZW( 0, 0, 0, 0 ) ) );
        TFloat64Quaternion q = TFloat64Quaternion::FromXYZW( 0.1, -0.7, 0.3, 0.2 );
        ASSERT_TRUE( CQuaternionPacking::Pack64( q ) == CQuaternionPacking::Pack64( q.Normalized() ) );
        ASSERT_TRUE( 1 == CQuaternionPacking::GetLargestComponentIndex64( CQuaternionPacking::Pack64( q ) ) );
        TFloat64Quaternion unpacked;
        CQuaternionPacking::Unpack32( identity32, unpacked );
        ASSERT_TRUE( unpacked == TFloat64Quaternion() );
        // the extreme codes: +-1/sqrt( 2 ) in two components
        CQuaternionPacking::Unpack32( ( 3u << 30 ) | ( 1022u << 20 ) | ( 511u << 10 ) | 0u, unpacked );
        ASSERT_TRUE( fabs( unpacked.x - sqrt( 0.5 ) ) < 1e-15 && fabs( unpacked.z + sqrt( 0.5 ) ) < 1e-15 && fabs( unpacked.w ) < 1e-7 );
        TFloat64Quaternion clamped;
        CQuaternionPacking::Unpack32( ( 3u << 30 ) | ( 1023u << 20 ) | ( 511u << 10 ) | 0u, clamped );
        ASSERT_TRUE( clamped == unpacked );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "QuaternionPacking: 10^5 seeded quaternions within 4 steps, index and sign recovered" )
    try
    {
        // acceptance 9
        const UInt64 seed = 0x9AC5;
        MathReference::CSeededSequence generator( seed );
        const Float64 step32 = CQuaternionPacking::GetStep( 10 );
        const Float64 step48 = CQuaternionPacking::GetStep( 15 );
        const Float64 step64 = CQuaternionPacking::GetStep( 20 );
        UInt32 failures = 0;
        Float64 worst[ 4 ] = { 0, 0, 0, 0 };
        for ( UInt32 i=0; i<100000; ++i )
        {
            TFloat64Quaternion q = RandomUnitQuaternion( generator );
            const UInt32 largest = LargestIndex( q );
            // the sign-folded representative: its largest component is positive
            const TFloat64Quaternion folded = Component( q, largest ) < 0.0 ? -q : q;

            const UInt32 code32 = CQuaternionPacking::Pack32( q );
            const UInt64 code48 = CQuaternionPacking::Pack48( q );
            const UInt64 code64 = CQuaternionPacking::Pack64( q );
            if ( largest != CQuaternionPacking::GetLargestComponentIndex32( code32 ) || largest != CQuaternionPacking::GetLargestComponentIndex48( code48 ) ||
                 largest != CQuaternionPacking::GetLargestComponentIndex64( code64 ) || 0 != ( code48 >> 47 ) || 0 != ( code64 >> 62 ) )
                ++failures;

            TFloat64Quaternion u32;
            TFloat64Quaternion u48;
            TFloat64Quaternion u64;
            TFloat32Quaternion f32;
            CQuaternionPacking::Unpack32( code32, u32 );
            CQuaternionPacking::Unpack48( code48, u48 );
            CQuaternionPacking::Unpack64( code64, u64 );
            CQuaternionPacking::Unpack32( CQuaternionPacking::Pack32( TFloat32Quaternion( q ) ), f32 );
            const Float64 errors[ 4 ] = { q.GetAngleTo( u32 ).GetValue(), q.GetAngleTo( u48 ).GetValue(), q.GetAngleTo( u64 ).GetValue(), q.GetAngleTo( TFloat64Quaternion( f32 ) ).GetValue() };
            for ( UInt32 n=0; n<4; ++n )
                if ( errors[ n ] > worst[ n ] )
                    worst[ n ] = errors[ n ];
            if ( errors[ 0 ] > 4.0 * step32 || errors[ 1 ] > 4.0 * step48 || errors[ 2 ] > 4.0 * step64 || errors[ 3 ] > 4.0 * step32 )
                ++failures;
            // the unpacked quaternion is the sign-folded one: largest component positive, components within two steps
            // (half a step per kept component; the restored largest component and the renormalisation add to that)
            if ( !( Component( u64, largest ) > 0.0 ) || !u64.Equals( folded, 2.0 * step64, 0.0 ) || !u32.Equals( folded, 2.0 * step32, 0.0 ) )
                ++failures;
            if ( !( fabs( u32.GetLength() - 1.0 ) < 1e-15 ) || !( fabs( f32.GetLength() - 1.0f ) < 1e-6f ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "QuaternionPacking: largest angle in steps: Pack32 " << worst[ 0 ] / step32 << ", Pack48 " << worst[ 1 ] / step48 <<
                   ", Pack64 " << worst[ 2 ] / step64 << ", Pack32 of Float32 " << worst[ 3 ] / step32 );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "QuaternionPacking: " << failures << " failures, seed " << seed );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "QuaternionPacking: realtime-safe functions run in a realtime context" )
    try
    {
        UInt64 code = RealtimePackingWork( TFloat32Quaternion::FromAxisAngle( TFloat32DirectionVector3D::UnitZ(), TFloat32Radian( 1.0f ) ) );
        ASSERT_TRUE( 0 != code );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
