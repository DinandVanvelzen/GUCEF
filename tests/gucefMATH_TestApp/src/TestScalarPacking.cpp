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

#include <math.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_PACKINGFUNCTIONS_H
#include "gucefMATH_PackingFunctions.h"
#define GUCEF_MATH_PACKINGFUNCTIONS_H
#endif /* GUCEF_MATH_PACKINGFUNCTIONS_H ? */

#include "TestScalarPacking.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformScalarPackingTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING ScalarPacking TESTS" );

    GUCEF_TESTFW_TESTCASE( "ScalarPacking: unorm and snorm values" )
    {
        // 0.5 * 255 = 127.5: the tie goes to the even code 128
        ASSERT_TRUE( MATH::PackUnorm8( 0.5 ) == 128 );
        ASSERT_TRUE( MATH::PackUnorm8( 0.0 ) == 0 );
        ASSERT_TRUE( MATH::PackUnorm8( 1.0 ) == 255 );
        ASSERT_TRUE( MATH::PackUnorm8( -3.0 ) == 0 );
        ASSERT_TRUE( MATH::PackUnorm8( 7.0 ) == 255 );
        ASSERT_TRUE( MATH::PackUnorm8( 1.5f / 255.0f ) == 2 );
        ASSERT_TRUE( MATH::PackUnorm8( 2.5 / 255.0 ) == 2 );      // tie goes to the even code
        ASSERT_TRUE( MATH::PackUnorm8( 3.5 / 255.0 ) == 4 );
        ASSERT_TRUE( MATH::PackUnorm16( 0.5 ) == 32768 );         // 32767.5 ties to even 32768
        ASSERT_TRUE( MATH::PackUnorm16( 1.0 ) == 65535 );
        ASSERT_TRUE( MATH::PackUnorm16( -1.0 ) == 0 );

        ASSERT_TRUE( MATH::PackSnorm8( -1.0 ) == -127 );
        ASSERT_TRUE( MATH::PackSnorm8( 1.0 ) == 127 );
        ASSERT_TRUE( MATH::PackSnorm8( 0.0 ) == 0 );
        ASSERT_TRUE( MATH::PackSnorm8( -5.0 ) == -127 );
        ASSERT_TRUE( MATH::PackSnorm8( 0.5 ) == 64 );             // 63.5 ties to even 64
        ASSERT_TRUE( MATH::PackSnorm8( -0.5 ) == -64 );
        ASSERT_TRUE( MATH::PackSnorm16( -1.0 ) == -32767 );
        ASSERT_TRUE( MATH::PackSnorm16( 1.0 ) == 32767 );

        ASSERT_TRUE( MATH::UnpackSnorm8< MATH::Float64 >( -128 ) == -1.0 );
        ASSERT_TRUE( MATH::UnpackSnorm8< MATH::Float64 >( -127 ) == -1.0 );
        ASSERT_TRUE( MATH::UnpackSnorm8< MATH::Float64 >( 127 ) == 1.0 );
        ASSERT_TRUE( MATH::UnpackSnorm16< MATH::Float64 >( -32768 ) == -1.0 );
        ASSERT_TRUE( MATH::UnpackSnorm16< MATH::Float64 >( 32767 ) == 1.0 );
        ASSERT_TRUE( MATH::UnpackUnorm8< MATH::Float64 >( 255 ) == 1.0 );
        ASSERT_TRUE( MATH::UnpackUnorm8< MATH::Float64 >( 0 ) == 0.0 );
        ASSERT_TRUE( MATH::UnpackUnorm16< MATH::Float64 >( 65535 ) == 1.0 );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScalarPacking: NaN packs to 0" )
    {
        volatile MATH::Float64 zero = 0.0;
        const MATH::Float64 nan = zero / zero;
        ASSERT_TRUE( MATH::PackUnorm8( nan ) == 0 );
        ASSERT_TRUE( MATH::PackUnorm16( nan ) == 0 );
        ASSERT_TRUE( MATH::PackSnorm8( nan ) == 0 );
        ASSERT_TRUE( MATH::PackSnorm16( nan ) == 0 );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScalarPacking: every code decodes and re-encodes to itself (Float32 and Float64)" )
    {
        bool ok = true;
        for ( MATH::UInt32 c = 0; c < 256; ++c )
        {
            if ( MATH::PackUnorm8( MATH::UnpackUnorm8< MATH::Float64 >( (MATH::UInt8) c ) ) != c ) ok = false;
            if ( MATH::PackUnorm8( MATH::UnpackUnorm8< MATH::Float32 >( (MATH::UInt8) c ) ) != c ) ok = false;
            const MATH::Int8 s = (MATH::Int8) (MATH::Int32) ( (MATH::Int32) c - 128 );
            if ( s != -128 )
            {
                if ( MATH::PackSnorm8( MATH::UnpackSnorm8< MATH::Float64 >( s ) ) != s ) ok = false;
                if ( MATH::PackSnorm8( MATH::UnpackSnorm8< MATH::Float32 >( s ) ) != s ) ok = false;
            }
        }
        ASSERT_TRUE( ok );

        bool ok16 = true;
        for ( MATH::UInt32 c = 0; c < 65536; ++c )
        {
            if ( MATH::PackUnorm16( MATH::UnpackUnorm16< MATH::Float64 >( (MATH::UInt16) c ) ) != c ) ok16 = false;
            if ( MATH::PackUnorm16( MATH::UnpackUnorm16< MATH::Float32 >( (MATH::UInt16) c ) ) != c ) ok16 = false;
            const MATH::Int16 s = (MATH::Int16) (MATH::Int32) ( (MATH::Int32) c - 32768 );
            if ( s != -32768 )
            {
                if ( MATH::PackSnorm16( MATH::UnpackSnorm16< MATH::Float64 >( s ) ) != s ) ok16 = false;
                if ( MATH::PackSnorm16( MATH::UnpackSnorm16< MATH::Float32 >( s ) ) != s ) ok16 = false;
            }
        }
        ASSERT_TRUE( ok16 );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScalarPacking: quantisation error is at most half a step, packing is monotonic" )
    {
        bool errorOk = true;
        bool monotonic = true;
        MATH::UInt8 previous = 0;
        MATH::Int16 previousSnorm = -32767;
        for ( MATH::UInt32 s = 0; s <= 100000; ++s )
        {
            const MATH::Float64 x = (MATH::Float64) s / 100000.0;
            const MATH::UInt8 code = MATH::PackUnorm8( x );
            if ( fabs( MATH::UnpackUnorm8< MATH::Float64 >( code ) - x ) > 0.5 / 255.0 + 1e-12 ) errorOk = false;
            if ( code < previous ) monotonic = false;
            previous = code;

            const MATH::Float64 y = x * 2.0 - 1.0;
            const MATH::Int16 snorm = MATH::PackSnorm16( y );
            if ( fabs( MATH::UnpackSnorm16< MATH::Float64 >( snorm ) - y ) > 0.5 / 32767.0 + 1e-12 ) errorOk = false;
            if ( snorm < previousSnorm ) monotonic = false;
            previousSnorm = snorm;
        }
        ASSERT_TRUE( errorOk );
        ASSERT_TRUE( monotonic );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ScalarPacking TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
