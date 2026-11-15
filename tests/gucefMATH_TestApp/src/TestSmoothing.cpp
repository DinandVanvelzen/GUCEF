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

#ifndef GUCEF_MATH_SMOOTHINGFUNCTIONS_H
#include "gucefMATH_SmoothingFunctions.h"
#define GUCEF_MATH_SMOOTHINGFUNCTIONS_H
#endif /* GUCEF_MATH_SMOOTHINGFUNCTIONS_H ? */

#ifndef GUCEF_MATH_SCALARFUNCTIONS_H
#include "gucefMATH_ScalarFunctions.h"
#define GUCEF_MATH_SCALARFUNCTIONS_H
#endif /* GUCEF_MATH_SCALARFUNCTIONS_H ? */

#include "TestSmoothing.h"

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
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** SmoothDamp from 'start' to 'target' for 'seconds' at 'rate' steps per second; value at the end, monotonicity in 'monotone' */
static Float64
RunSmoothDamp( Float64 start, Float64 target, Float64 smoothTime, Float64 seconds, UInt32 rate, bool& monotone, Float64* at1 = GUCEF_NULL, Float64* at2 = GUCEF_NULL )
{
    SSmoothDampState< Float64 > state;
    Float64 value = start;
    const Float64 dt = 1.0 / rate;
    const UInt32 steps = (UInt32) ( seconds * rate + 0.5 );
    monotone = true;
    for ( UInt32 i=0; i<steps; ++i )
    {
        const Float64 next = SmoothDamp( value, target, state, smoothTime, dt );
        if ( ( start > target && next > value + 1e-15 ) || ( start < target && next < value - 1e-15 ) || ( start > target && next < target - 1e-12 ) || ( start < target && next > target + 1e-12 ) )
            monotone = false;
        value = next;
        if ( GUCEF_NULL != at1 && i + 1 == rate )
            *at1 = value;
        if ( GUCEF_NULL != at2 && i + 1 == 2 * rate )
            *at2 = value;
    }
    return value;
}

/*-------------------------------------------------------------------------*/

void
PerformSmoothingTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Smoothing TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Smoothing" );

    GUCEF_TESTFW_TESTCASE( "Smoothing: SmoothDamp from 1 to 0 with smooth time 1 is below 1% after 4 time units without overshoot" )
        bool monotone = false;
        const Float64 end = RunSmoothDamp( 1.0, 0.0, 1.0, 4.0, 60, monotone );
        ASSERT_TRUE( end >= 0.0 && end < 0.01 );
        ASSERT_TRUE( monotone );
        bool monotone240 = false;
        const Float64 end240 = RunSmoothDamp( 1.0, 0.0, 1.0, 4.0, 240, monotone240 );
        ASSERT_TRUE( end240 >= 0.0 && end240 < 0.01 && monotone240 );
        // rising direction
        bool monotoneUp = false;
        const Float64 endUp = RunSmoothDamp( -3.0, 5.0, 0.5, 6.0, 60, monotoneUp );
        ASSERT_TRUE( monotoneUp && fabs( endUp - 5.0 ) < 1e-3 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Smoothing: SmoothDamp gives the same trajectory at 60 Hz and 240 Hz within 1e-3" )
        bool monotone = false;
        Float64 a1 = 0, a2 = 0, b1 = 0, b2 = 0;
        const Float64 ea = RunSmoothDamp( 1.0, 0.0, 1.0, 4.0, 60, monotone, &a1, &a2 );
        const Float64 eb = RunSmoothDamp( 1.0, 0.0, 1.0, 4.0, 240, monotone, &b1, &b2 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Smoothing: SmoothDamp 60 Hz / 240 Hz differences " << fabs( a1 - b1 ) << " " << fabs( a2 - b2 ) << " " << fabs( ea - eb ) );
        ASSERT_TRUE( fabs( a1 - b1 ) < 1e-3 && fabs( a2 - b2 ) < 1e-3 && fabs( ea - eb ) < 1e-3 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Smoothing: SmoothDamp limits the speed, ignores a zero step and keeps the state at rest on the target" )
        // from 0 towards 100 with maxSpeed 2 and smooth time 1: the speed never exceeds about 2
        SSmoothDampState< Float64 > state;
        Float64 value = 0.0;
        Float64 fastest = 0.0;
        for ( UInt32 i=0; i<120; ++i )
        {
            const Float64 next = SmoothDamp( value, 100.0, state, 1.0, 2.0, 1.0 / 60.0 );
            const Float64 speed = ( next - value ) * 60.0;
            if ( speed > fastest ) fastest = speed;
            value = next;
        }
        ASSERT_TRUE( fastest <= 2.0 * 1.05 && fastest > 1.0 && value > 2.0 );
        SSmoothDampState< Float64 > rest;
        ASSERT_TRUE( 3.0 == SmoothDamp( 3.0, 9.0, rest, 1.0, 0.0 ) );
        ASSERT_TRUE( 0.0 == rest.velocity );
        ASSERT_TRUE( 7.0 == SmoothDamp( 7.0, 7.0, rest, 1.0, 0.1 ) && 0.0 == rest.velocity );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Smoothing: SmoothDamp of vectors, points and Float32 reaches the target without passing it" )
        {
            SSmoothDampState< TFloat64Vector3D > state;
            TFloat64Vector3D value( 5.0, -2.0, 1.0 );
            const TFloat64Vector3D target( -1.0, 4.0, 0.5 );
            const TFloat64Vector3D direction = ( target - value ).Normalized();
            bool passed = false;
            for ( UInt32 i=0; i<600; ++i )
            {
                value = SmoothDamp( value, target, state, 0.7, 1.0 / 120.0 );
                if ( ( target - value ).Dot( direction ) < -1e-9 ) passed = true;
            }
            ASSERT_TRUE( !passed && ( value - target ).GetLength() < 1e-3 );
        }
        {
            SSmoothDampState< TFloat64Point3D > state;
            TFloat64Point3D value( 5.0, -2.0, 1.0 );
            const TFloat64Point3D target( -1.0, 4.0, 0.5 );
            for ( UInt32 i=0; i<600; ++i )
                value = SmoothDamp( value, target, state, 0.7, 1.0 / 120.0 );
            ASSERT_TRUE( ( value - target ).GetLength() < 1e-3 );
        }
        {
            SSmoothDampState< Float32 > state;
            Float32 value = 1.0f;
            for ( UInt32 i=0; i<240; ++i )
                value = SmoothDamp( value, 0.0f, state, 1.0f, 1.0f / 60.0f );
            ASSERT_TRUE( value >= 0.0f && value < 0.01f );
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Smoothing: SmoothDamp of an angle takes the short way round" )
        SSmoothDampState< Float64 > state;
        CTRadian< Float64 > angle = Degrees( 350.0 );
        const CTRadian< Float64 > target = Degrees( 10.0 );
        const CTRadian< Float64 > first = SmoothDamp( angle, target, state, 0.5, 1.0 / 60.0 );
        // 350 degrees moves up (towards 370), not down to 10
        ASSERT_TRUE( first.GetValue() > angle.GetValue() );
        for ( UInt32 i=0; i<600; ++i )
            angle = SmoothDamp( angle, target, state, 0.5, std::numeric_limits< Float64 >::infinity(), 1.0 / 60.0 );
        ASSERT_TRUE( fabs( DeltaAngle( angle, target ).GetValue() ) < 1e-4 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Smoothing: InterpTo is frame-rate independent and handles its degenerate input" )
        Float64 once = InterpTo( 0.0, 10.0, 1.0, 3.0 );
        Float64 many = 0.0;
        for ( UInt32 i=0; i<240; ++i )
            many = InterpTo( many, 10.0, 1.0 / 240.0, 3.0 );
        ASSERT_TRUE( fabs( once - many ) < 1e-12 );
        ASSERT_TRUE( fabs( once - 10.0 * ( 1.0 - exp( -3.0 ) ) ) < 1e-12 );
        ASSERT_TRUE( 10.0 == InterpTo( 0.0, 10.0, 0.5, 0.0 ) && 10.0 == InterpTo( 0.0, 10.0, 0.5, -1.0 ) );
        ASSERT_TRUE( 2.0 == InterpTo( 2.0, 10.0, 0.0, 3.0 ) && 2.0 == InterpTo( 2.0, 10.0, -1.0, 3.0 ) );
        const TFloat64Vector3D v = InterpTo( TFloat64Vector3D( 0.0, 0.0, 0.0 ), TFloat64Vector3D( 10.0, 20.0, 30.0 ), 1.0, 3.0 );
        ASSERT_TRUE( fabs( v.y - 2.0 * once ) < 1e-12 && fabs( v.z - 3.0 * once ) < 1e-12 );
        const TFloat32Point2D p = InterpTo( TFloat32Point2D( 0.0f, 0.0f ), TFloat32Point2D( 4.0f, 8.0f ), 1.0f, 100.0f );
        ASSERT_TRUE( fabs( p.x - 4.0f ) < 1e-4f && fabs( p.y - 8.0f ) < 1e-4f );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Smoothing: MoveTowards and InterpConstantTo arrive exactly and never overshoot" )
        ASSERT_TRUE( 5.0 == MoveTowards( 4.5, 5.0, 1.0 ) && 5.0 == MoveTowards( 5.0, 5.0, 0.0 ) );
        const TFloat64Vector3D from( 0.0, 0.0, 0.0 );
        const TFloat64Vector3D to( 3.0, 4.0, 0.0 );
        ASSERT_TRUE( to == MoveTowards( from, to, 5.0 ) && to == MoveTowards( from, to, 7.0 ) );
        const TFloat64Vector3D step = MoveTowards( from, to, 2.0 );
        ASSERT_TRUE( fabs( step.GetLength() - 2.0 ) < 1e-12 && fabs( step.x - 1.2 ) < 1e-12 && fabs( step.y - 1.6 ) < 1e-12 );
        const TFloat64Point3D pFrom( 1.0, 1.0, 1.0 );
        const TFloat64Point3D pTo( 1.0, 1.0, 4.0 );
        ASSERT_TRUE( pTo == MoveTowards( pFrom, pTo, 3.0 ) && pTo == MoveTowards( pFrom, pTo, 100.0 ) );
        ASSERT_TRUE( fabs( MoveTowards( pFrom, pTo, 1.0 ).z - 2.0 ) < 1e-12 );
        const TFloat64Vector2D v2 = MoveTowards( TFloat64Vector2D( 0.0, 0.0 ), TFloat64Vector2D( 0.0, 10.0 ), 4.0 );
        ASSERT_TRUE( 0.0 == v2.x && fabs( v2.y - 4.0 ) < 1e-12 );
        // constant speed motion: 10 units at 2 per second takes 5 seconds of 1/60 steps and then stays put
        Float64 value = 0.0;
        for ( UInt32 i=0; i<300; ++i )
            value = InterpConstantTo( value, 10.0, 1.0 / 60.0, 2.0 );
        ASSERT_TRUE( fabs( value - 10.0 ) < 1e-9 );
        for ( UInt32 i=0; i<60; ++i )
            value = InterpConstantTo( value, 10.0, 1.0 / 60.0, 2.0 );
        ASSERT_TRUE( 10.0 == value );
        ASSERT_TRUE( 3.0 == InterpConstantTo( 0.0, 3.0, 1.0, 100.0 ) );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Smoothing: SpringStep converges critically damped and oscillates when underdamped" )
        {
            const Float64 stiffness = 100.0;
            SSpringState< Float64 > state( 1.0 );
            const Float64 damping = GetCriticalDamping( stiffness );
            ASSERT_TRUE( fabs( damping - 20.0 ) < 1e-12 );
            Float64 lowest = 1.0;
            for ( UInt32 i=0; i<2000; ++i )
            {
                SpringStep( state, 0.0, stiffness, damping, 0.001 );
                if ( state.position < lowest ) lowest = state.position;
            }
            ASSERT_TRUE( fabs( state.position ) < 1e-3 && lowest > -1e-3 );
        }
        {
            SSpringState< Float64 > state( 1.0 );
            Float64 lowest = 1.0;
            for ( UInt32 i=0; i<2000; ++i )
            {
                SpringStep( state, 0.0, 100.0, 2.0, 0.001 );
                if ( state.position < lowest ) lowest = state.position;
            }
            ASSERT_TRUE( lowest < -0.5 );
        }
        {
            // vector valued, a spring pulling towards a moved target: ends at rest on the target
            SSpringState< TFloat32Vector3D > state( TFloat32Vector3D( 1.0f, 2.0f, 3.0f ) );
            const TFloat32Vector3D target( -2.0f, 0.5f, 1.0f );
            for ( UInt32 i=0; i<4000; ++i )
                SpringStep( state, target, 50.0f, 14.0f, 0.001f );
            ASSERT_TRUE( ( state.position - target ).GetLength() < 1e-3f && state.velocity.GetLength() < 1e-3f );
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
