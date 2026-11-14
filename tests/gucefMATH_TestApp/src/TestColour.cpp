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

#ifndef GUCEF_MATH_CTLINEARCOLOUR_H
#include "gucefMATH_CTLinearColour.h"
#define GUCEF_MATH_CTLINEARCOLOUR_H
#endif /* GUCEF_MATH_CTLINEARCOLOUR_H ? */

#ifndef GUCEF_MATH_COLOURFUNCTIONS_H
#include "gucefMATH_ColourFunctions.h"
#define GUCEF_MATH_COLOURFUNCTIONS_H
#endif /* GUCEF_MATH_COLOURFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CPCG32_H
#include "gucefMATH_CPcg32.h"
#define GUCEF_MATH_CPCG32_H
#endif /* GUCEF_MATH_CPCG32_H ? */

#include "TestColour.h"

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


typedef MATH::Float32 F32;
typedef MATH::Float64 F64;

/*-------------------------------------------------------------------------*/

static F64
NextUnit( MATH::CPcg32& random )
{
    return (F64) random.NextUInt32() / 4294967296.0;
}

/*-------------------------------------------------------------------------*/

static void
PerformTransferFunctionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Colour: sRGB transfer functions match the published values, are continuous at the thresholds and invert each other" )
    {
        ASSERT_TRUE( fabs( MATH::SrgbToLinear( 0.5f ) - 0.21404114f ) < 1e-7f );
        ASSERT_TRUE( fabs( MATH::SrgbToLinear( 0.5 ) - 0.21404114048223255 ) < 1e-12 );
        ASSERT_TRUE( 0.0 == MATH::SrgbToLinear( 0.0 ) && fabs( MATH::SrgbToLinear( 1.0 ) - 1.0 ) < 1e-15 );
        ASSERT_TRUE( 0.0 == MATH::LinearToSrgb( 0.0 ) && fabs( MATH::LinearToSrgb( 1.0 ) - 1.0 ) < 1e-15 );

        // Continuity at the thresholds: both branches agree to the precision of the constants of the standard
        const F64 below = 0.04045 / 12.92;
        const F64 above = pow( ( 0.04045 + 0.055 ) / 1.055, 2.4 );
        ASSERT_TRUE( fabs( below - above ) < 1e-6 );
        ASSERT_TRUE( fabs( MATH::SrgbToLinear( 0.04045 ) - MATH::SrgbToLinear( 0.04045 + 1e-12 ) ) < 1e-7 );   // the constants of the standard leave a jump of about 1e-9
        ASSERT_TRUE( fabs( MATH::LinearToSrgb( 0.0031308 ) - MATH::LinearToSrgb( 0.0031308 + 1e-12 ) ) < 1e-6 );
        ASSERT_TRUE( fabs( MATH::LinearToSrgb( 0.0031308 ) - 0.04045 ) < 1e-5 );

        // The negative side mirrors the positive side
        ASSERT_TRUE( MATH::SrgbToLinear( -0.5 ) == -MATH::SrgbToLinear( 0.5 ) );
        ASSERT_TRUE( MATH::LinearToSrgb( -0.2 ) == -MATH::LinearToSrgb( 0.2 ) );

        MATH::CPcg32 random( 3, 3 );
        bool ok = true;
        for ( int n = 0; n < 10000 && ok; ++n )
        {
            const F64 x = NextUnit( random );
            ok = fabs( MATH::LinearToSrgb( MATH::SrgbToLinear( x ) ) - x ) < 1e-12 && fabs( MATH::SrgbToLinear( MATH::LinearToSrgb( x ) ) - x ) < 1e-12;
        }
        ASSERT_TRUE( ok );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Colour: all 256 8 bit codes round trip through linear Float32 and Float64, alpha is not gamma encoded" )
    {
        bool ok32 = true;
        bool ok64 = true;
        for ( int code = 0; code < 256; ++code )
        {
            const MATH::CTLinearColour< F32 > c32 = MATH::CTLinearColour< F32 >::FromRgba8( (MATH::UInt8) code, (MATH::UInt8) ( 255 - code ), (MATH::UInt8) code, (MATH::UInt8) code );
            MATH::UInt8 r, g, b, a;
            c32.ToRgba8( r, g, b, a );
            if ( r != code || g != 255 - code || b != code || a != code )
                ok32 = false;
            const MATH::CTLinearColour< F64 > c64 = MATH::CTLinearColour< F64 >::FromRgba8( (MATH::UInt8) code, (MATH::UInt8) code, (MATH::UInt8) code, (MATH::UInt8) code );
            c64.ToRgba8( r, g, b, a );
            if ( r != code || g != code || b != code || a != code )
                ok64 = false;
        }
        ASSERT_TRUE( ok32 );
        ASSERT_TRUE( ok64 );

        const MATH::CTLinearColour< F32 > half = MATH::CTLinearColour< F32 >::FromRgba8( 0, 0, 0, 255 );
        ASSERT_TRUE( 1.0f == half.a );
        const MATH::CTLinearColour< F32 > mid = MATH::CTLinearColour< F32 >::FromRgba8( 128, 128, 128, 128 );
        ASSERT_TRUE( fabs( mid.a - 128.0f / 255.0f ) < 1e-7f );
        ASSERT_TRUE( mid.r < 0.25f && mid.r > 0.2f );
        ASSERT_TRUE( sizeof( MATH::CTLinearColour< F32 > ) == 4 * sizeof( F32 ) );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Colour: quantisation rounds half to even, clamps and maps not a number to 0" )
    {
        // four steps, so that the products are exact: 0.5 -> 0, 1.5 -> 2, 2.5 -> 2, 3.5 -> 4
        ASSERT_TRUE( 0 == MATH::ColourQuantize( 0.125, 4 ) );
        ASSERT_TRUE( 2 == MATH::ColourQuantize( 0.375, 4 ) );
        ASSERT_TRUE( 2 == MATH::ColourQuantize( 0.625, 4 ) );
        ASSERT_TRUE( 4 == MATH::ColourQuantize( 0.875, 4 ) );
        ASSERT_TRUE( 1 == MATH::ColourQuantize( 0.26, 4 ) && 3 == MATH::ColourQuantize( 0.74, 4 ) );
        ASSERT_TRUE( 0 == MATH::ColourQuantize( -1.0, 255 ) && 255 == MATH::ColourQuantize( 2.0, 255 ) );
        volatile F64 zero = 0.0;
        ASSERT_TRUE( 0 == MATH::ColourQuantize( zero / zero, 255 ) );

        MATH::CTLinearColour< F64 > hdr( 4.0, -1.0, 0.5, 2.0 );
        MATH::UInt8 r, g, b, a;
        hdr.ToRgba8( r, g, b, a );
        ASSERT_TRUE( 255 == r && 0 == g && 255 == a && b > 180 && b < 190 );
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformSpaceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Colour: HSV and HSL of the primaries and of greys, and round trips for 10^4 seeded colours" )
    {
        MATH::CTDegree< F64 > hue;
        F64 s, v;
        MATH::RgbToHsv( 1.0, 0.0, 0.0, hue, s, v );
        ASSERT_TRUE( 0.0 == hue.GetValue() && 1.0 == s && 1.0 == v );
        MATH::RgbToHsv( 0.0, 1.0, 0.0, hue, s, v );
        ASSERT_TRUE( 120.0 == hue.GetValue() && 1.0 == s && 1.0 == v );
        MATH::RgbToHsv( 0.0, 0.0, 1.0, hue, s, v );
        ASSERT_TRUE( 240.0 == hue.GetValue() && 1.0 == s && 1.0 == v );
        MATH::RgbToHsv( 1.0, 1.0, 0.0, hue, s, v );
        ASSERT_TRUE( 60.0 == hue.GetValue() );
        MATH::RgbToHsv( 1.0, 0.0, 1.0, hue, s, v );
        ASSERT_TRUE( 300.0 == hue.GetValue() );
        MATH::RgbToHsv( 1.0, 0.0, 0.5, hue, s, v );
        ASSERT_TRUE( hue.GetValue() >= 0.0 && hue.GetValue() < 360.0 && 330.0 == hue.GetValue() );
        MATH::RgbToHsv( 0.5, 0.5, 0.5, hue, s, v );
        ASSERT_TRUE( 0.0 == hue.GetValue() && 0.0 == s && 0.5 == v );
        MATH::RgbToHsv( 0.0, 0.0, 0.0, hue, s, v );
        ASSERT_TRUE( 0.0 == hue.GetValue() && 0.0 == s && 0.0 == v );

        F64 l;
        MATH::RgbToHsl( 1.0, 0.0, 0.0, hue, s, l );
        ASSERT_TRUE( 0.0 == hue.GetValue() && 1.0 == s && 0.5 == l );
        MATH::RgbToHsl( 0.0, 1.0, 0.0, hue, s, l );
        ASSERT_TRUE( 120.0 == hue.GetValue() );
        MATH::RgbToHsl( 0.25, 0.25, 0.25, hue, s, l );
        ASSERT_TRUE( 0.0 == hue.GetValue() && 0.0 == s && 0.25 == l );
        MATH::RgbToHsl( 1.0, 1.0, 1.0, hue, s, l );
        ASSERT_TRUE( 0.0 == s && 1.0 == l );

        F64 r, g, b;
        MATH::HsvToRgb( MATH::CTDegree< F64 >( 120.0 ), 1.0, 1.0, r, g, b );
        ASSERT_TRUE( 0.0 == r && 1.0 == g && 0.0 == b );
        MATH::HsvToRgb( MATH::CTDegree< F64 >( 480.0 ), 1.0, 1.0, r, g, b );               // wraps to 120
        ASSERT_TRUE( 0.0 == r && 1.0 == g && 0.0 == b );
        MATH::HsvToRgb( MATH::CTDegree< F64 >( -60.0 ), 1.0, 1.0, r, g, b );               // wraps to 300
        ASSERT_TRUE( 1.0 == r && 0.0 == g && 1.0 == b );
        MATH::HslToRgb( MATH::CTDegree< F64 >( 0.0 ), 1.0, 0.5, r, g, b );
        ASSERT_TRUE( 1.0 == r && 0.0 == g && 0.0 == b );

        MATH::CPcg32 random( 11, 1 );
        bool ok = true;
        for ( int n = 0; n < 10000 && ok; ++n )
        {
            const F64 r0 = NextUnit( random );
            const F64 g0 = NextUnit( random );
            const F64 b0 = NextUnit( random );
            MATH::RgbToHsv( r0, g0, b0, hue, s, v );
            MATH::HsvToRgb( hue, s, v, r, g, b );
            ok = fabs( r - r0 ) < 1e-12 && fabs( g - g0 ) < 1e-12 && fabs( b - b0 ) < 1e-12 && hue.GetValue() >= 0.0 && hue.GetValue() < 360.0;
            MATH::RgbToHsl( r0, g0, b0, hue, s, l );
            MATH::HslToRgb( hue, s, l, r, g, b );
            ok = ok && fabs( r - r0 ) < 1e-12 && fabs( g - g0 ) < 1e-12 && fabs( b - b0 ) < 1e-12;
        }
        ASSERT_TRUE( ok );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Colour: OKLab of linear red and white match the published values, round trips for 10^4 seeded colours, LCH" )
    {
        F64 lightness, a, b;
        MATH::LinearRgbToOkLab( 1.0, 0.0, 0.0, lightness, a, b );
        ASSERT_TRUE( fabs( lightness - 0.62795536 ) < 1e-6 && fabs( a - 0.22486306 ) < 1e-6 && fabs( b - 0.12584630 ) < 1e-6 );
        MATH::LinearRgbToOkLab( 1.0, 1.0, 1.0, lightness, a, b );
        ASSERT_TRUE( fabs( lightness - 1.0 ) < 1e-4 && fabs( a ) < 1e-4 && fabs( b ) < 1e-4 );
        MATH::LinearRgbToOkLab( 0.0, 0.0, 0.0, lightness, a, b );
        ASSERT_TRUE( 0.0 == lightness && 0.0 == a && 0.0 == b );

        // the same in Float32
        F32 l32, a32, b32;
        MATH::LinearRgbToOkLab( 1.0f, 0.0f, 0.0f, l32, a32, b32 );
        ASSERT_TRUE( fabs( l32 - 0.62795536f ) < 1e-5f && fabs( a32 - 0.22486306f ) < 1e-5f && fabs( b32 - 0.12584630f ) < 1e-5f );

        MATH::CPcg32 random( 21, 1 );
        bool ok = true;
        for ( int n = 0; n < 10000 && ok; ++n )
        {
            const F64 r0 = NextUnit( random );
            const F64 g0 = NextUnit( random );
            const F64 b0 = NextUnit( random );
            F64 lab[ 3 ];
            F64 back[ 3 ];
            MATH::LinearRgbToOkLab( r0, g0, b0, lab[ 0 ], lab[ 1 ], lab[ 2 ] );
            MATH::OkLabToLinearRgb( lab[ 0 ], lab[ 1 ], lab[ 2 ], back[ 0 ], back[ 1 ], back[ 2 ] );
            ok = fabs( back[ 0 ] - r0 ) < 1e-6 && fabs( back[ 1 ] - g0 ) < 1e-6 && fabs( back[ 2 ] - b0 ) < 1e-6;

            F64 lch[ 2 ];
            MATH::CTDegree< F64 > hue;
            MATH::OkLabToOkLch( lab[ 0 ], lab[ 1 ], lab[ 2 ], lch[ 0 ], lch[ 1 ], hue );
            F64 again[ 3 ];
            MATH::OkLchToOkLab( lch[ 0 ], lch[ 1 ], hue, again[ 0 ], again[ 1 ], again[ 2 ] );
            ok = ok && fabs( again[ 1 ] - lab[ 1 ] ) < 1e-12 && fabs( again[ 2 ] - lab[ 2 ] ) < 1e-12 && hue.GetValue() >= 0.0 && hue.GetValue() < 360.0;
        }
        ASSERT_TRUE( ok );

        F64 lightness2, chroma;
        MATH::CTDegree< F64 > hue;
        MATH::OkLabToOkLch( 0.5, 0.0, 0.1, lightness2, chroma, hue );
        ASSERT_TRUE( 0.5 == lightness2 && fabs( chroma - 0.1 ) < 1e-15 && fabs( hue.GetValue() - 90.0 ) < 1e-12 );
        MATH::OkLabToOkLch( 0.5, 0.0, 0.0, lightness2, chroma, hue );
        ASSERT_TRUE( 0.0 == chroma && 0.0 == hue.GetValue() );

        ASSERT_TRUE( fabs( MATH::GetRelativeLuminance( 1.0, 1.0, 1.0 ) - 1.0 ) < 1e-15 );
        ASSERT_TRUE( fabs( MATH::GetRelativeLuminance( 0.0, 1.0, 0.0 ) - 0.7152 ) < 1e-15 );
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformColourClassTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Colour: Lerp, premultiply and arithmetic of the linear colour" )
    {
        typedef MATH::CTLinearColour< F64 > TColour;
        const TColour black( 0.0, 0.0, 0.0, 1.0 );
        const TColour white( 1.0, 1.0, 1.0, 0.0 );
        const TColour mid = TColour::Lerp( black, white, 0.5 );
        ASSERT_TRUE( 0.5 == mid.r && 0.5 == mid.g && 0.5 == mid.b && 0.5 == mid.a );
        ASSERT_TRUE( TColour::Lerp( black, white, 0.0 ) == black && TColour::Lerp( black, white, 1.0 ) == white );

        const TColour colour( 0.8, 0.4, 0.2, 0.5 );
        const TColour premultiplied = colour.Premultiplied();
        ASSERT_TRUE( 0.4 == premultiplied.r && 0.2 == premultiplied.g && 0.1 == premultiplied.b && 0.5 == premultiplied.a );
        const TColour restored = premultiplied.Unpremultiplied();
        ASSERT_TRUE( fabs( restored.r - 0.8 ) < 1e-15 && fabs( restored.g - 0.4 ) < 1e-15 && fabs( restored.b - 0.2 ) < 1e-15 );
        ASSERT_TRUE( TColour( 1.0, 1.0, 1.0, 0.0 ).Unpremultiplied() == TColour() );

        const TColour sum = colour + colour;
        ASSERT_TRUE( 1.6 == sum.r && 1.0 == sum.a );
        ASSERT_TRUE( ( sum - colour ) == colour );
        ASSERT_TRUE( ( colour * 2.0 ) == sum && ( 2.0 * colour ) == sum );
        ASSERT_TRUE( TColour() == TColour( 0.0, 0.0, 0.0, 0.0 ) && colour != TColour() );
        ASSERT_TRUE( fabs( TColour( 1.0, 1.0, 1.0, 1.0 ).GetRelativeLuminance() - 1.0 ) < 1e-15 );
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformColourTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Colour TESTS" );

    PerformTransferFunctionTests();
    PerformSpaceTests();
    PerformColourClassTests();

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Colour TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
