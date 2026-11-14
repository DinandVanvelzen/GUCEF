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

#ifndef GUCEF_MATH_EASINGFUNCTIONS_H
#include "gucefMATH_EasingFunctions.h"
#define GUCEF_MATH_EASINGFUNCTIONS_H
#endif /* GUCEF_MATH_EASINGFUNCTIONS_H ? */

#include "TestEasing.h"

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

typedef MATH::Float64 (*TEaseFunc)( const MATH::Float64& );

/*-------------------------------------------------------------------------*/

/* The named functions in the order of the enumerators of EEasingFunction */
static const TEaseFunc NamedFunctions[ 31 ] =
{
    &MATH::EaseLinear< MATH::Float64 > ,
    &MATH::EaseInQuad< MATH::Float64 >    , &MATH::EaseOutQuad< MATH::Float64 >    , &MATH::EaseInOutQuad< MATH::Float64 >    ,
    &MATH::EaseInCubic< MATH::Float64 >   , &MATH::EaseOutCubic< MATH::Float64 >   , &MATH::EaseInOutCubic< MATH::Float64 >   ,
    &MATH::EaseInQuart< MATH::Float64 >   , &MATH::EaseOutQuart< MATH::Float64 >   , &MATH::EaseInOutQuart< MATH::Float64 >   ,
    &MATH::EaseInQuint< MATH::Float64 >   , &MATH::EaseOutQuint< MATH::Float64 >   , &MATH::EaseInOutQuint< MATH::Float64 >   ,
    &MATH::EaseInSine< MATH::Float64 >    , &MATH::EaseOutSine< MATH::Float64 >    , &MATH::EaseInOutSine< MATH::Float64 >    ,
    &MATH::EaseInExpo< MATH::Float64 >    , &MATH::EaseOutExpo< MATH::Float64 >    , &MATH::EaseInOutExpo< MATH::Float64 >    ,
    &MATH::EaseInCirc< MATH::Float64 >    , &MATH::EaseOutCirc< MATH::Float64 >    , &MATH::EaseInOutCirc< MATH::Float64 >    ,
    &MATH::EaseInBack< MATH::Float64 >    , &MATH::EaseOutBack< MATH::Float64 >    , &MATH::EaseInOutBack< MATH::Float64 >    ,
    &MATH::EaseInElastic< MATH::Float64 > , &MATH::EaseOutElastic< MATH::Float64 > , &MATH::EaseInOutElastic< MATH::Float64 > ,
    &MATH::EaseInBounce< MATH::Float64 >  , &MATH::EaseOutBounce< MATH::Float64 >  , &MATH::EaseInOutBounce< MATH::Float64 >
};

static const char* const FamilyNames[ 11 ] =
{
    "Linear", "Quad", "Cubic", "Quart", "Quint", "Sine", "Expo", "Circ", "Back", "Elastic", "Bounce"
};

/* index of the first enumerator of family f (0 = linear): 1 + 3 * ( f - 1 ) */
static MATH::UInt32 FirstOfFamily( MATH::UInt32 family )
{
    return 0 == family ? 0 : 1 + 3 * ( family - 1 );
}

/*-------------------------------------------------------------------------*/

void
PerformEasingTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Easing TESTS" );

    GUCEF_TESTFW_TESTCASE( "Easing: every function maps 0 to 0 and 1 to 1" )
        for ( MATH::UInt32 i = 0; i < 31; ++i )
        {
            const bool ok = fabs( NamedFunctions[ i ]( 0.0 ) ) <= 1e-6 && fabs( NamedFunctions[ i ]( 1.0 ) - 1.0 ) <= 1e-6;
            if ( !ok )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Easing: enumerator " << i << " gives f(0) = " << NamedFunctions[ i ]( 0.0 ) << " and f(1) = " << NamedFunctions[ i ]( 1.0 ) );
            ASSERT_TRUE( ok );
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Easing: the InOut variants give 0.5 at 0.5" )
        for ( MATH::UInt32 f = 1; f <= 10; ++f )
        {
            const MATH::UInt32 inOut = FirstOfFamily( f ) + 2;
            const bool ok = fabs( NamedFunctions[ inOut ]( 0.5 ) - 0.5 ) <= 1e-9;
            if ( !ok )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Easing: " << FamilyNames[ f ] << " InOut gives " << NamedFunctions[ inOut ]( 0.5 ) << " at 0.5" );
            ASSERT_TRUE( ok );
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Easing: the families without overshoot are monotonic on 1000 samples" )
    {
        // Linear, Quad, Cubic, Quart, Quint, Sine, Expo, Circ
        for ( MATH::UInt32 f = 0; f <= 7; ++f )
        {
            const MATH::UInt32 count = 0 == f ? 1 : 3;
            for ( MATH::UInt32 v = 0; v < count; ++v )
            {
                const MATH::UInt32 index = FirstOfFamily( f ) + v;
                bool monotonic = true;
                bool inRange = true;
                MATH::Float64 previous = NamedFunctions[ index ]( 0.0 );
                for ( MATH::UInt32 s = 1; s <= 1000; ++s )
                {
                    const MATH::Float64 value = NamedFunctions[ index ]( (MATH::Float64) s / 1000.0 );
                    if ( value < previous - 1e-12 ) monotonic = false;
                    if ( value < -1e-12 || value > 1.0 + 1e-12 ) inRange = false;
                    previous = value;
                }
                if ( !monotonic || !inRange )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Easing: " << FamilyNames[ f ] << " variant " << v << " monotonic " << monotonic << " in range " << inRange );
                ASSERT_TRUE( monotonic );
                ASSERT_TRUE( inRange );
            }
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Easing: Out( t ) = 1 - In( 1 - t ) for every family" )
        for ( MATH::UInt32 f = 1; f <= 10; ++f )
        {
            const MATH::UInt32 inIndex = FirstOfFamily( f );
            for ( MATH::UInt32 s = 1; s < 100; ++s )
            {
                const MATH::Float64 t = (MATH::Float64) s / 100.0;
                const bool ok = fabs( NamedFunctions[ inIndex + 1 ]( t ) - ( 1.0 - NamedFunctions[ inIndex ]( 1.0 - t ) ) ) <= 1e-9;
                if ( !ok )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Easing: " << FamilyNames[ f ] << " Out differs from 1 - In( 1 - t ) at t = " << t );
                ASSERT_TRUE( ok );
            }
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Easing: published values" )
        ASSERT_TRUE( MATH::EaseInQuad( 0.5 ) == 0.25 );
        ASSERT_TRUE( MATH::EaseOutBounce( 0.5 ) == 0.765625 );
        ASSERT_TRUE( MATH::EaseOutQuad( 0.5 ) == 0.75 );
        ASSERT_TRUE( MATH::EaseInCubic( 0.5 ) == 0.125 );
        ASSERT_TRUE( fabs( MATH::EaseInSine( 0.5 ) - ( 1.0 - sqrt( 0.5 ) ) ) < 1e-12 );
        // Back overshoots below 0 on the way in and above 1 on the way out, by the published constant
        ASSERT_TRUE( MATH::EaseInBack( 0.3 ) < 0.0 );
        ASSERT_TRUE( MATH::EaseOutBack( 0.7 ) > 1.0 );
        ASSERT_TRUE( fabs( MATH::EaseInBack( 0.5 ) - ( 2.70158 * 0.125 - 1.70158 * 0.25 ) ) < 1e-12 );
        // Elastic oscillates around the target: it passes 1 before it settles
        bool aboveOne = false;
        bool belowOne = false;
        for ( MATH::UInt32 s = 1; s < 100; ++s )
        {
            const MATH::Float64 value = MATH::EaseOutElastic( (MATH::Float64) s / 100.0 );
            if ( value > 1.0 ) aboveOne = true;
            if ( value < 1.0 ) belowOne = true;
        }
        ASSERT_TRUE( aboveOne && belowOne );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Easing: Ease( enumerator, t ) equals the named function for every enumerator" )
        for ( MATH::UInt32 i = 0; i < 31; ++i )
        {
            for ( MATH::UInt32 s = 0; s <= 50; ++s )
            {
                const MATH::Float64 t = (MATH::Float64) s / 50.0;
                const bool ok = MATH::Ease( (MATH::TEasingFunction) i, t ) == NamedFunctions[ i ]( t );
                if ( !ok )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Easing: Ease() differs from the named function for enumerator " << i << " at t = " << t );
                ASSERT_TRUE( ok );
            }
        }
        ASSERT_TRUE( MATH::Ease( (MATH::TEasingFunction) 99, 0.3 ) == 0.3 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Easing: the Float32 instantiation agrees with Float64" )
        for ( MATH::UInt32 i = 0; i < 31; ++i )
        {
            for ( MATH::UInt32 s = 0; s <= 20; ++s )
            {
                const MATH::Float64 t = (MATH::Float64) s / 20.0;
                const MATH::Float32 narrow = MATH::Ease( (MATH::TEasingFunction) i, (MATH::Float32) t );
                const bool ok = fabs( (MATH::Float64) narrow - NamedFunctions[ i ]( (MATH::Float64) (MATH::Float32) t ) ) <= 2e-4;
                if ( !ok )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Easing: Float32 differs for enumerator " << i << " at t = " << t );
                ASSERT_TRUE( ok );
            }
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Easing TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
