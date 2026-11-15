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

#ifndef GUCEF_MATH_CTBALLISTICS_H
#include "gucefMATH_CTBallistics.h"
#define GUCEF_MATH_CTBALLISTICS_H
#endif /* GUCEF_MATH_CTBALLISTICS_H ? */

#include "TestMathReference.h"
#include "TestBallistics.h"

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

typedef CTBallistics< Float64 > TBallistics;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Independent simulation of a trajectory in the vertical plane with plain doubles: height at the moment the
 *  horizontal distance 'x' is reached
 */
static Float64
HeightAtDistance( Float64 speed, Float64 angle, Float64 gravity, Float64 x )
{
    const Float64 vx = speed * cos( angle );
    const Float64 vy = speed * sin( angle );
    const Float64 t = x / vx;
    return vy * t - 0.5 * gravity * t * t;
}

/** The residual of the intercept equation | d + v t - g t^2 / 2 | - s t */
static Float64
InterceptResidual( const TFloat64Vector3D& d, const TFloat64Vector3D& v, const TFloat64Vector3D& g, Float64 s, Float64 t )
{
    return ( d + v * t - g * ( 0.5 * t * t ) ).GetLength() - s * t;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformBallisticsTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Ballistics TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Ballistics" );

    GUCEF_TESTFW_TESTCASE( "Ballistics: launch angles for speed 10, gravity 9.81 and a target at distance 5" )
        CTRadian< Float64 > low, high;
        ASSERT_TRUE( 2 == TBallistics::SolveLaunchAngles( 10.0, 5.0, 0.0, 9.81, low, high ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Ballistics: angles " << low.GetDegrees() << " " << high.GetDegrees() );
        ASSERT_TRUE( fabs( low.GetDegrees() - 14.69 ) < 0.01 && fabs( high.GetDegrees() - 75.31 ) < 0.01 );
        ASSERT_TRUE( fabs( low.GetDegrees() + high.GetDegrees() - 90.0 ) < 1e-9 );
        // simulating both trajectories hits the target
        ASSERT_TRUE( fabs( HeightAtDistance( 10.0, low.GetValue(), 9.81, 5.0 ) ) < 1e-6 * 5.0 );
        ASSERT_TRUE( fabs( HeightAtDistance( 10.0, high.GetValue(), 9.81, 5.0 ) ) < 1e-6 * 5.0 );
        // targets above and below
        const Float64 heights[ 5 ] = { 2.0, -3.0, 0.5, 4.0, -10.0 };
        for ( UInt32 i=0; i<5; ++i )
        {
            CTRadian< Float64 > l, h;
            const UInt32 count = TBallistics::SolveLaunchAngles( 15.0, 8.0, heights[ i ], 9.81, l, h );
            ASSERT_TRUE( 2 == count && l <= h );
            ASSERT_TRUE( fabs( HeightAtDistance( 15.0, l.GetValue(), 9.81, 8.0 ) - heights[ i ] ) < 1e-6 * 8.0 );
            ASSERT_TRUE( fabs( HeightAtDistance( 15.0, h.GetValue(), 9.81, 8.0 ) - heights[ i ] ) < 1e-6 * 8.0 );
        }
        // Float32
        CTRadian< Float32 > low32, high32;
        ASSERT_TRUE( 2 == CTBallistics< Float32 >::SolveLaunchAngles( 10.0f, 5.0f, 0.0f, 9.81f, low32, high32 ) && fabs( low32.GetDegrees() - 14.69f ) < 0.02f && fabs( high32.GetDegrees() - 75.31f ) < 0.02f );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Ballistics: out of range, the range limit and the degenerate launches" )
        CTRadian< Float64 > low( 5.0 ), high( 6.0 );
        ASSERT_TRUE( 0 == TBallistics::SolveLaunchAngles( 10.0, 50.0, 0.0, 9.81, low, high ) && 5.0 == low.GetValue() && 6.0 == high.GetValue() );
        ASSERT_TRUE( 0 == TBallistics::SolveLaunchAngles( 10.0, 5.0, 20.0, 9.81, low, high ) );
        ASSERT_TRUE( 0 == TBallistics::SolveLaunchAngles( 0.0, 5.0, 0.0, 9.81, low, high ) && 0 == TBallistics::SolveLaunchAngles( -3.0, 5.0, 0.0, 9.81, low, high ) );
        // at the range limit v^2 / g the angle is 45 degrees
        const UInt32 limit = TBallistics::SolveLaunchAngles( 10.0, 100.0 / 9.81, 0.0, 9.81, low, high );
        ASSERT_TRUE( limit >= 1 && fabs( low.GetDegrees() - 45.0 ) < 0.01 && fabs( high.GetDegrees() - 45.0 ) < 0.01 );
        ASSERT_TRUE( 0 == TBallistics::SolveLaunchAngles( 10.0, 100.0 / 9.81 * 1.0001, 0.0, 9.81, low, high ) );
        // straight up and down
        ASSERT_TRUE( 1 == TBallistics::SolveLaunchAngles( 10.0, 0.0, 4.0, 9.81, low, high ) && fabs( low.GetDegrees() - 90.0 ) < 1e-9 && fabs( high.GetDegrees() - 90.0 ) < 1e-9 );
        ASSERT_TRUE( 0 == TBallistics::SolveLaunchAngles( 10.0, 0.0, 6.0, 9.81, low, high ) );
        ASSERT_TRUE( 2 == TBallistics::SolveLaunchAngles( 10.0, 0.0, -4.0, 9.81, low, high ) && fabs( low.GetDegrees() + 90.0 ) < 1e-9 && fabs( high.GetDegrees() - 90.0 ) < 1e-9 );
        // no gravity: the straight line
        ASSERT_TRUE( 1 == TBallistics::SolveLaunchAngles( 10.0, 3.0, 4.0, 0.0, low, high ) && fabs( low.GetValue() - atan2( 4.0, 3.0 ) ) < 1e-12 && low == high );
        ASSERT_TRUE( 0 == TBallistics::SolveLaunchAngles( 10.0, 0.0, 0.0, 0.0, low, high ) );
        // failure leaves the outputs unchanged: no gravity and the target at the shooter
        CTRadian< Float64 > keepLow( 1.25 ), keepHigh( 2.5 );
        ASSERT_TRUE( 0 == TBallistics::SolveLaunchAngles( 10.0, 0.0, 0.0, 0.0, keepLow, keepHigh ) && 1.25 == keepLow.GetValue() && 2.5 == keepHigh.GetValue() );
        // a negative distance is taken as zero
        ASSERT_TRUE( 1 == TBallistics::SolveLaunchAngles( 10.0, -2.0, 4.0, 9.81, low, high ) );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Ballistics: launch velocities in 3D under any gravity direction" )
        const TFloat64Vector3D gravities[ 3 ] = { TFloat64Vector3D( 0.0, -9.81, 0.0 ), TFloat64Vector3D( 1.0, -9.0, 2.0 ), TFloat64Vector3D( 0.0, 0.0, -3.7 ) };
        const TFloat64Point3D origin( 1.0, 2.0, 3.0 );
        const TFloat64Point3D target( 11.0, 7.0, -4.0 );
        for ( UInt32 k=0; k<3; ++k )
        {
            const TFloat64Vector3D g = gravities[ k ];
            TFloat64Vector3D low, high;
            const UInt32 count = TBallistics::SolveLaunchVelocity( origin, target, 30.0, g, low, high );
            ASSERT_TRUE( 2 == count );
            Float64 times[ 2 ] = { 0.0, 0.0 };
            const TFloat64Vector3D velocities[ 2 ] = { low, high };
            for ( UInt32 i=0; i<2; ++i )
            {
                ASSERT_TRUE( fabs( velocities[ i ].GetLength() - 30.0 ) < 1e-9 );
                // the time follows from the part of the motion perpendicular to gravity
                const TFloat64Vector3D up = g * ( -1.0 / g.GetLength() );
                const TFloat64Vector3D d = target - origin;
                const TFloat64Vector3D dPerp = d - up * d.Dot( up );
                const TFloat64Vector3D vPerp = velocities[ i ] - up * velocities[ i ].Dot( up );
                const Float64 t = dPerp.GetLength() / vPerp.GetLength();
                times[ i ] = t;
                ASSERT_TRUE( ( TBallistics::GetPositionAtTime( origin, velocities[ i ], g, t ) - target ).GetLength() < 1e-9 );
            }
            ASSERT_TRUE( times[ 0 ] < times[ 1 ] );     // the flat trajectory arrives first
        }
        // out of range and without gravity
        TFloat64Vector3D low( 7, 7, 7 ), high( 8, 8, 8 );
        ASSERT_TRUE( 0 == TBallistics::SolveLaunchVelocity( origin, TFloat64Point3D( 500, 2, 3 ), 10.0, gravities[ 0 ], low, high ) && 7.0 == low.x && 8.0 == high.x );
        ASSERT_TRUE( 0 == TBallistics::SolveLaunchVelocity( origin, target, 0.0, gravities[ 0 ], low, high ) );
        ASSERT_TRUE( 1 == TBallistics::SolveLaunchVelocity( origin, target, 10.0, TFloat64Vector3D(), low, high ) && low == high && fabs( low.GetLength() - 10.0 ) < 1e-12 && low.Equals( ( target - origin ).Normalized() * 10.0, 1e-12, 1e-12 ) );
        ASSERT_TRUE( 0 == TBallistics::SolveLaunchVelocity( origin, origin, 10.0, TFloat64Vector3D(), low, high ) );
        // a target straight above
        ASSERT_TRUE( 1 == TBallistics::SolveLaunchVelocity( origin, TFloat64Point3D( 1.0, 5.0, 3.0 ), 10.0, gravities[ 0 ], low, high ) && low.Equals( TFloat64Vector3D( 0.0, 10.0, 0.0 ), 1e-9, 1e-9 ) );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Ballistics: intercept of a target moving perpendicular, checked by simulation" )
        const TFloat64Point3D shooter( 0.0, 0.0, 0.0 );
        const TFloat64Point3D target( 10.0, 0.0, 0.0 );
        const TFloat64Vector3D velocity( 0.0, 0.0, 5.0 );
        Float64 t = 0.0;
        ASSERT_TRUE( TBallistics::SolveInterceptTime( shooter, 10.0, target, velocity, t ) );
        ASSERT_TRUE( fabs( t - sqrt( 100.0 / 75.0 ) ) < 1e-12 );
        const TFloat64Point3D hit = target + velocity * t;
        ASSERT_TRUE( fabs( ( hit - shooter ).GetLength() - 10.0 * t ) < 1e-12 );
        // the projectile with the speed along the line to the intercept point arrives at the same time
        TFloat64Vector3D direction;
        ASSERT_TRUE( ( hit - shooter ).TryNormalize( direction, 0.0 ) && ( shooter + direction * ( 10.0 * t ) - hit ).GetLength() < 1e-12 );
        // a target at the shooter, a target running away faster, speed zero, negative speed
        ASSERT_TRUE( TBallistics::SolveInterceptTime( shooter, 10.0, shooter, velocity, t ) && 0.0 == t );
        Float64 untouched = 77.0;
        ASSERT_TRUE( !TBallistics::SolveInterceptTime( shooter, 10.0, target, TFloat64Vector3D( 20.0, 0.0, 0.0 ), untouched ) && 77.0 == untouched );
        ASSERT_TRUE( !TBallistics::SolveInterceptTime( shooter, 0.0, target, TFloat64Vector3D(), untouched ) );
        ASSERT_TRUE( !TBallistics::SolveInterceptTime( shooter, -1.0, target, TFloat64Vector3D(), untouched ) );
        // a target coming towards the shooter at 5: the gap of 10 closes at 15
        ASSERT_TRUE( TBallistics::SolveInterceptTime( shooter, 10.0, target, TFloat64Vector3D( -5.0, 0.0, 0.0 ), t ) && fabs( t - 10.0 / 15.0 ) < 1e-12 );
        // equal speeds: a linear equation; a target moving perpendicular at the speed of the projectile can never be caught
        const TFloat64Vector3D closing( -3.0, 0.0, 4.0 );
        ASSERT_TRUE( TBallistics::SolveInterceptTime( shooter, 5.0, target, closing, t ) && fabs( t - 100.0 / 60.0 ) < 1e-12 && fabs( ( target + closing * t - shooter ).GetLength() - 5.0 * t ) < 1e-9 );
        ASSERT_TRUE( !TBallistics::SolveInterceptTime( shooter, 5.0, target, velocity, untouched ) );
        // seeded sweep: every reported solution satisfies the equation and is the smallest non-negative root
        const UInt64 seed = 31337;
        MathReference::CSeededSequence rng( seed );
        bool ok = true;
        UInt32 solved = 0;
        for ( UInt32 i=0; i<2000; ++i )
        {
            const TFloat64Point3D s( rng.NextFloat64( -20, 20 ), rng.NextFloat64( -20, 20 ), rng.NextFloat64( -20, 20 ) );
            const TFloat64Point3D p( rng.NextFloat64( -20, 20 ), rng.NextFloat64( -20, 20 ), rng.NextFloat64( -20, 20 ) );
            const TFloat64Vector3D v( rng.NextFloat64( -12, 12 ), rng.NextFloat64( -12, 12 ), rng.NextFloat64( -12, 12 ) );
            const Float64 speed = rng.NextFloat64( 0.5, 15.0 );
            Float64 time = 0.0;
            if ( TBallistics::SolveInterceptTime( s, speed, p, v, time ) )
            {
                ++solved;
                const TFloat64Vector3D d = p - s;
                ok = ok && time >= 0.0 && fabs( InterceptResidual( d, v, TFloat64Vector3D(), speed, time ) ) < 1e-8 * ( 1.0 + time );
                // no earlier sign change of the residual: scan
                Float64 previous = InterceptResidual( d, v, TFloat64Vector3D(), speed, 0.0 );
                for ( Float64 u = time / 200.0; u < time * 0.999; u += time / 200.0 )
                {
                    const Float64 r = InterceptResidual( d, v, TFloat64Vector3D(), speed, u );
                    ok = ok && !( previous > 0.0 && r < 0.0 );
                    previous = r;
                }
            }
        }
        if ( !ok )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Ballistics: intercept sweep failed, seed " << seed );
        ASSERT_TRUE( ok && solved > 200 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Ballistics: intercept with gravity (quartic) checked by simulation" )
        const TFloat64Vector3D g( 0.0, -9.81, 0.0 );
        const TFloat64Point3D shooter( 0.0, 0.0, 0.0 );
        const TFloat64Point3D target( 20.0, 5.0, 3.0 );
        const TFloat64Vector3D velocity( 1.0, 0.0, -2.0 );
        Float64 t = 0.0;
        TFloat64Vector3D launch;
        ASSERT_TRUE( TBallistics::SolveInterceptWithGravity( shooter, 25.0, target, velocity, g, t, launch ) );
        ASSERT_TRUE( fabs( launch.GetLength() - 25.0 ) < 1e-9 );
        ASSERT_TRUE( ( TBallistics::GetPositionAtTime( shooter, launch, g, t ) - ( target + velocity * t ) ).GetLength() < 1e-9 );
        // the earliest root: no earlier crossing of the residual
        Float64 previous = InterceptResidual( target - shooter, velocity, g, 25.0, 1e-9 );
        bool earliest = true;
        for ( Float64 u = t / 400.0; u < t * 0.999; u += t / 400.0 )
        {
            const Float64 r = InterceptResidual( target - shooter, velocity, g, 25.0, u );
            earliest = earliest && !( previous > 0.0 && r < 0.0 );
            previous = r;
        }
        ASSERT_TRUE( earliest );
        // out of range, at the shooter
        Float64 untouched = 55.0;
        ASSERT_TRUE( !TBallistics::SolveInterceptWithGravity( shooter, 1.0, TFloat64Point3D( 500, 0, 0 ), velocity, g, untouched, launch ) && 55.0 == untouched );
        ASSERT_TRUE( TBallistics::SolveInterceptWithGravity( shooter, 5.0, shooter, velocity, g, t, launch ) && 0.0 == t && launch.IsZero() );
        ASSERT_TRUE( !TBallistics::SolveInterceptWithGravity( shooter, -5.0, target, velocity, g, untouched, launch ) );
        // without gravity it is the linear intercept
        Float64 tLinear = 0.0, tGravityFree = 0.0;
        TFloat64Vector3D launchFree;
        ASSERT_TRUE( TBallistics::SolveInterceptTime( shooter, 15.0, target, velocity, tLinear ) && TBallistics::SolveInterceptWithGravity( shooter, 15.0, target, velocity, TFloat64Vector3D(), tGravityFree, launchFree ) );
        ASSERT_TRUE( fabs( tLinear - tGravityFree ) < 1e-9 && fabs( launchFree.GetLength() - 15.0 ) < 1e-9 );
        // a static target on the same height agrees with the closed form for the launch angles: the shorter time is the flat trajectory
        Float64 tStatic = 0.0;
        TFloat64Vector3D launchStatic;
        ASSERT_TRUE( TBallistics::SolveInterceptWithGravity( shooter, 10.0, TFloat64Point3D( 5.0, 0.0, 0.0 ), TFloat64Vector3D(), g, tStatic, launchStatic ) );
        CTRadian< Float64 > low, high;
        TBallistics::SolveLaunchAngles( 10.0, 5.0, 0.0, 9.81, low, high );
        ASSERT_TRUE( fabs( atan2( launchStatic.y, launchStatic.x ) - low.GetValue() ) < 1e-9 && fabs( tStatic - 5.0 / ( 10.0 * cos( low.GetValue() ) ) ) < 1e-9 );
        // seeded sweep
        const UInt64 seed = 271828;
        MathReference::CSeededSequence rng( seed );
        bool ok = true;
        UInt32 solved = 0;
        for ( UInt32 i=0; i<2000; ++i )
        {
            const TFloat64Point3D s( rng.NextFloat64( -20, 20 ), rng.NextFloat64( -20, 20 ), rng.NextFloat64( -20, 20 ) );
            const TFloat64Point3D p( rng.NextFloat64( -40, 40 ), rng.NextFloat64( -40, 40 ), rng.NextFloat64( -40, 40 ) );
            const TFloat64Vector3D v( rng.NextFloat64( -8, 8 ), rng.NextFloat64( -8, 8 ), rng.NextFloat64( -8, 8 ) );
            const TFloat64Vector3D gv( rng.NextFloat64( -3, 3 ), rng.NextFloat64( -12, -6 ), rng.NextFloat64( -3, 3 ) );
            const Float64 speed = rng.NextFloat64( 5.0, 40.0 );
            Float64 time = 0.0;
            TFloat64Vector3D w;
            if ( TBallistics::SolveInterceptWithGravity( s, speed, p, v, gv, time, w ) )
            {
                ++solved;
                const TFloat64Point3D projectile = TBallistics::GetPositionAtTime( s, w, gv, time );
                ok = ok && time > 0.0 && fabs( w.GetLength() - speed ) < 1e-6 * speed && ( projectile - ( p + v * time ) ).GetLength() < 1e-6 * ( 1.0 + time );
            }
        }
        if ( !ok )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Ballistics: intercept with gravity sweep failed, seed " << seed );
        ASSERT_TRUE( ok && solved > 300 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Ballistics: apex, time of flight and position at time" )
        const TFloat64Point3D origin( 1.0, 2.0, 3.0 );
        const TFloat64Vector3D velocity( 3.0, 10.0, 0.0 );
        const TFloat64Vector3D g( 0.0, -9.81, 0.0 );
        TFloat64Point3D apex;
        Float64 t = 0.0;
        ASSERT_TRUE( TBallistics::GetApex( origin, velocity, g, apex, t ) );
        ASSERT_TRUE( fabs( t - 10.0 / 9.81 ) < 1e-12 && fabs( apex.y - ( 2.0 + 100.0 / ( 2.0 * 9.81 ) ) ) < 1e-12 && fabs( apex.x - ( 1.0 + 3.0 * t ) ) < 1e-12 );
        // a falling projectile has its apex at the origin
        ASSERT_TRUE( TBallistics::GetApex( origin, TFloat64Vector3D( 1.0, -2.0, 0.0 ), g, apex, t ) && apex == origin && 0.0 == t );
        ASSERT_TRUE( !TBallistics::GetApex( origin, velocity, TFloat64Vector3D(), apex, t ) );
        // time of flight back to the launch height and to 2 above it
        ASSERT_TRUE( TBallistics::GetTimeOfFlight( velocity, g, 0.0, t ) && fabs( t - 20.0 / 9.81 ) < 1e-12 );
        ASSERT_TRUE( TBallistics::GetTimeOfFlight( velocity, g, 2.0, t ) );
        const TFloat64Point3D at = TBallistics::GetPositionAtTime( origin, velocity, g, t );
        ASSERT_TRUE( fabs( at.y - 4.0 ) < 1e-12 && velocity.y + g.y * t < 0.0 );
        Float64 untouched = 9.0;
        ASSERT_TRUE( !TBallistics::GetTimeOfFlight( velocity, g, 100.0, untouched ) && 9.0 == untouched );
        ASSERT_TRUE( !TBallistics::GetTimeOfFlight( velocity, TFloat64Vector3D(), 1.0, untouched ) );
        // a target below the launch point that is reached on the way down; one that is lower than where we start with a downward velocity
        ASSERT_TRUE( TBallistics::GetTimeOfFlight( TFloat64Vector3D( 0.0, -5.0, 0.0 ), g, -10.0, t ) && fabs( -5.0 * t - 0.5 * 9.81 * t * t + 10.0 ) < 1e-12 );
        // position at time
        ASSERT_TRUE( TBallistics::GetPositionAtTime( origin, velocity, g, 0.0 ) == origin );
        ASSERT_TRUE( ( TBallistics::GetPositionAtTime( origin, velocity, g, 2.0 ) - TFloat64Point3D( 7.0, 2.0 + 20.0 - 0.5 * 9.81 * 4.0, 3.0 ) ).GetLength() < 1e-12 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Ballistics: Float32 low launch angle keeps its precision and agrees with Float64" )
        // speed 300, target 1 away: the low angle is tiny ( g x / ( 2 v^2 ) ), the old form cancelled to zero in Float32
        CTRadian< Float32 > low32, high32;
        ASSERT_TRUE( 2 == CTBallistics< Float32 >::SolveLaunchAngles( 300.0f, 1.0f, 0.0f, 9.81f, low32, high32 ) );
        CTRadian< Float64 > low64, high64;
        ASSERT_TRUE( 2 == TBallistics::SolveLaunchAngles( 300.0, 1.0, 0.0, 9.81, low64, high64 ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Ballistics: low angle Float32 " << low32.GetValue() << " Float64 " << low64.GetValue() );
        ASSERT_TRUE( low64.GetValue() > 5e-5 && fabs( low32.GetValue() - low64.GetValue() ) < 1e-3 * low64.GetValue() );
        ASSERT_TRUE( fabs( high32.GetValue() - high64.GetValue() ) < 1e-5 );
        // a target below: the low angle is negative
        ASSERT_TRUE( 2 == TBallistics::SolveLaunchAngles( 10.0, 5.0, -20.0, 9.81, low64, high64 ) && low64.GetValue() < 0.0 && fabs( HeightAtDistance( 10.0, low64.GetValue(), 9.81, 5.0 ) + 20.0 ) < 1e-6 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Ballistics: Float32 intercepts over long distances agree with Float64" )
        typedef CTBallistics< Float32 > TBallistics32;
        // review case: static target 16 km away under gravity, speed 475.4
        Float32 t32 = 0.0f;
        TFloat32Vector3D w32;
        const Float32 speeds[ 3 ] = { 475.4f, 600.0f, 1000.0f };
        for ( UInt32 i=0; i<3; ++i )
        {
            const bool found = TBallistics32::SolveInterceptWithGravity( TFloat32Point3D( 0, 0, 0 ), speeds[ i ], TFloat32Point3D( 16000.0f, 0, 0 ), TFloat32Vector3D(), TFloat32Vector3D( 0, -9.81f, 0 ), t32, w32 );
            Float64 t64 = 0.0;
            TFloat64Vector3D w64;
            const bool found64 = TBallistics::SolveInterceptWithGravity( TFloat64Point3D( 0, 0, 0 ), speeds[ i ], TFloat64Point3D( 16000.0, 0, 0 ), TFloat64Vector3D(), TFloat64Vector3D( 0, -9.81, 0 ), t64, w64 );
            ASSERT_TRUE( found == found64 );
            if ( found )
                ASSERT_TRUE( fabs( w32.GetLength() - speeds[ i ] ) < 1e-3f * speeds[ i ] && fabs( t32 - t64 ) < 1e-3 * t64 );
        }
        // seeded sweep to 50 km: Float32 agrees with Float64 on whether there is a solution, on the time and on the speed
        const UInt64 seed = 5150;
        MathReference::CSeededSequence rng( seed );
        UInt32 solved = 0;
        bool ok = true;
        for ( UInt32 i=0; i<1500; ++i )
        {
            const Float64 range = rng.NextFloat64( 100.0, 50000.0 );
            const Float64 speed = rng.NextFloat64( 100.0, 1500.0 );
            const Float64 px = range * rng.NextFloat64( -1.0, 1.0 ), py = range * rng.NextFloat64( -0.3, 0.3 ), pz = range * rng.NextFloat64( -1.0, 1.0 );
            const Float64 vx = rng.NextFloat64( -50, 50 ), vz = rng.NextFloat64( -50, 50 );
            Float64 t64 = 0.0, tl64 = 0.0;
            TFloat64Vector3D w64;
            const bool found64 = TBallistics::SolveInterceptWithGravity( TFloat64Point3D( 0, 0, 0 ), speed, TFloat64Point3D( px, py, pz ), TFloat64Vector3D( vx, 0, vz ), TFloat64Vector3D( 0, -9.81, 0 ), t64, w64 );
            const bool found32 = TBallistics32::SolveInterceptWithGravity( TFloat32Point3D( 0, 0, 0 ), (Float32) speed, TFloat32Point3D( (Float32) px, (Float32) py, (Float32) pz ), TFloat32Vector3D( (Float32) vx, 0, (Float32) vz ), TFloat32Vector3D( 0, -9.81f, 0 ), t32, w32 );
            if ( found32 )
            {
                const bool speedOk = fabs( w32.GetLength() - speed ) < 2e-3 * speed;      // never a wrong speed
                if ( !speedOk )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Ballistics: wrong speed " << w32.GetLength() << " for " << speed << " range " << range );
                ok = ok && speedOk;
            }
            if ( found64 && found32 )
            {
                ++solved;
                const bool timeOk = fabs( t32 - t64 ) < 5e-3 * t64;
                if ( !timeOk )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Ballistics: time " << t32 << " versus " << t64 << " range " << range << " speed " << speed );
                ok = ok && timeOk;
            }
            // the linear intercept
            const bool lin64 = TBallistics::SolveInterceptTime( TFloat64Point3D( 0, 0, 0 ), speed, TFloat64Point3D( px, py, pz ), TFloat64Vector3D( vx, 0, vz ), tl64 );
            Float32 tl32 = 0.0f;
            const bool lin32 = TBallistics32::SolveInterceptTime( TFloat32Point3D( 0, 0, 0 ), (Float32) speed, TFloat32Point3D( (Float32) px, (Float32) py, (Float32) pz ), TFloat32Vector3D( (Float32) vx, 0, (Float32) vz ), tl32 );
            if ( lin64 && lin32 )
                ok = ok && fabs( tl32 - tl64 ) < 5e-3 * tl64;
        }
        if ( !ok )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Ballistics: Float32 long range sweep failed, seed " << seed );
        ASSERT_TRUE( ok && solved > 100 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Ballistics: Float32 intercept and apex" )
        typedef CTBallistics< Float32 > TBallistics32;
        Float32 t = 0.0f;
        ASSERT_TRUE( TBallistics32::SolveInterceptTime( TFloat32Point3D( 0, 0, 0 ), 10.0f, TFloat32Point3D( 10, 0, 0 ), TFloat32Vector3D( 0, 0, 5 ), t ) && fabs( t - 1.1547005f ) < 1e-5f );
        TFloat32Point3D apex;
        ASSERT_TRUE( TBallistics32::GetApex( TFloat32Point3D( 0, 0, 0 ), TFloat32Vector3D( 0, 10, 0 ), TFloat32Vector3D( 0, -10, 0 ), apex, t ) && fabs( t - 1.0f ) < 1e-6f && fabs( apex.y - 5.0f ) < 1e-5f );
        TFloat32Vector3D launch;
        ASSERT_TRUE( TBallistics32::SolveInterceptWithGravity( TFloat32Point3D( 0, 0, 0 ), 20.0f, TFloat32Point3D( 10, 2, 0 ), TFloat32Vector3D( 1, 0, 0 ), TFloat32Vector3D( 0, -9.81f, 0 ), t, launch ) );
        ASSERT_TRUE( fabs( launch.GetLength() - 20.0f ) < 1e-3f && t > 0.0f );
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
