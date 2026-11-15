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

#ifndef GUCEF_MATH_INTEGRATORFUNCTIONS_H
#include "gucefMATH_IntegratorFunctions.h"
#define GUCEF_MATH_INTEGRATORFUNCTIONS_H
#endif /* GUCEF_MATH_INTEGRATORFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTRUNGEKUTTA4_H
#include "gucefMATH_CTRungeKutta4.h"
#define GUCEF_MATH_CTRUNGEKUTTA4_H
#endif /* GUCEF_MATH_CTRUNGEKUTTA4_H ? */

#include "TestIntegrators.h"

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

/** x'' = -x as an acceleration functor on a scalar position */
struct SHarmonicAcceleration
{
    UInt32 evaluations;
    SHarmonicAcceleration( void ) : evaluations( 0 ) {}
    void Evaluate( const Float64& position, Float64& acceleration ) { ++evaluations; acceleration = -position; }
};

/** x'' = -x for a vector position; the pull is towards the origin */
struct SCentralAcceleration
{
    void Evaluate( const TFloat64Vector3D& position, TFloat64Vector3D& acceleration ) { acceleration = -position; }
};

/** The same pull for a point position, centred on a given point */
struct SPointAcceleration
{
    TFloat64Point3D centre;
    void Evaluate( const TFloat64Point3D& position, TFloat64Vector3D& acceleration ) { acceleration = centre - position; }
};

/** Constant gravity */
struct SGravity
{
    TFloat64Vector3D g;
    void Evaluate( const TFloat64Point3D&, TFloat64Vector3D& acceleration ) { acceleration = g; }
};

/** x' = -x */
struct SDecay
{
    void Evaluate( const Float64&, const Float64& state, Float64& derivative ) { derivative = -state; }
};

/** x' = -x in single precision */
struct SDecay32
{
    void Evaluate( const Float32&, const Float32& state, Float32& derivative ) { derivative = -state; }
};

/** ( x, v )' = ( v, -x ) with the state as a 2D vector */
struct SOscillatorSystem
{
    void Evaluate( const Float64&, const TFloat64Vector2D& state, TFloat64Vector2D& derivative ) { derivative = TFloat64Vector2D( state.y, -state.x ); }
};

/** x' = cos( t ): depends on the time argument */
struct SCosineSystem
{
    void Evaluate( const Float64& t, const Float64&, Float64& derivative ) { derivative = cos( t ); }
};

/** The position error of velocity Verlet on x'' = -x, x( 0 ) = 1, v( 0 ) = 0 at time 'end' */
static Float64
VerletError( Float64 dt, Float64 end )
{
    SVelocityVerletState< Float64 > state( 1.0, 0.0 );
    SHarmonicAcceleration f;
    const UInt32 steps = (UInt32) ( end / dt + 0.5 );
    for ( UInt32 i=0; i<steps; ++i )
        VelocityVerletStep( state, f, dt );
    return fabs( state.position - cos( end ) );
}

/** The error of RK4 on x' = -x, x( 0 ) = 1 at time 'end' */
static Float64
Rk4Error( Float64 dt, Float64 end )
{
    Float64 x = 1.0;
    SDecay system;
    const UInt32 steps = (UInt32) ( end / dt + 0.5 );
    for ( UInt32 i=0; i<steps; ++i )
        CTRungeKutta4< Float64, SDecay >::Step( x, i * dt, dt, system );
    return fabs( x - exp( -end ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformIntegratorsTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Integrators TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Integrators" );

    GUCEF_TESTFW_TESTCASE( "Integrators: semi-implicit Euler keeps the energy of an oscillator within 1% over 100000 steps" )
        Float64 x = 1.0;
        Float64 v = 0.0;
        const Float64 dt = 0.01;
        const Float64 e0 = 0.5 * ( x * x + v * v );
        Float64 worst = 0.0;
        for ( UInt32 i=0; i<100000; ++i )
        {
            SemiImplicitEulerStep( x, v, -x, dt );
            const Float64 drift = fabs( 0.5 * ( x * x + v * v ) - e0 ) / e0;
            if ( drift > worst ) worst = drift;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Integrators: semi-implicit Euler worst energy drift " << worst );
        ASSERT_TRUE( worst < 0.01 );
        // the exact update for constant acceleration: v += a dt, x += v dt
        Float64 px = 0.0, pv = 1.0;
        SemiImplicitEulerStep( px, pv, 2.0, 0.5 );
        ASSERT_TRUE( 2.0 == pv && 1.0 == px );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Integrators: semi-implicit Euler on vectors, points and Float32" )
        TFloat64Vector3D position( 1.0, 2.0, 3.0 );
        TFloat64Vector3D velocity( 0.0, -1.0, 0.5 );
        SemiImplicitEulerStep( position, velocity, TFloat64Vector3D( 0.0, -10.0, 0.0 ), 0.1 );
        ASSERT_TRUE( velocity.Equals( TFloat64Vector3D( 0.0, -2.0, 0.5 ) ) && position.Equals( TFloat64Vector3D( 1.0, 1.8, 3.05 ) ) );
        TFloat32Point2D p( 0.0f, 0.0f );
        TFloat32Vector2D pv( 1.0f, 0.0f );
        for ( UInt32 i=0; i<10; ++i )
            SemiImplicitEulerStep( p, pv, TFloat32Vector2D( 0.0f, 1.0f ), 0.1f );
        ASSERT_TRUE( fabs( pv.y - 1.0f ) < 1e-5f && fabs( p.x - 1.0f ) < 1e-5f && p.y > 0.4f && p.y < 0.7f );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Integrators: velocity Verlet is second order (halving dt divides the error by 4)" )
        const Float64 steps[ 3 ] = { 0.02, 0.01, 0.005 };
        Float64 errors[ 3 ];
        for ( UInt32 i=0; i<3; ++i )
            errors[ i ] = VerletError( steps[ i ], 10.0 );
        const Float64 ratioA = errors[ 0 ] / errors[ 1 ];
        const Float64 ratioB = errors[ 1 ] / errors[ 2 ];
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Integrators: Verlet errors " << errors[ 0 ] << " " << errors[ 1 ] << " " << errors[ 2 ] << " ratios " << ratioA << " " << ratioB );
        ASSERT_TRUE( ratioA > 3.2 && ratioA < 4.8 && ratioB > 3.2 && ratioB < 4.8 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Integrators: velocity Verlet needs one evaluation per step and is exact for constant acceleration" )
        SVelocityVerletState< Float64 > state( 1.0, 0.0 );
        SHarmonicAcceleration f;
        for ( UInt32 i=0; i<10; ++i )
            VelocityVerletStep( state, f, 0.1 );
        ASSERT_TRUE( 11 == f.evaluations );       // one for the start acceleration, one per step
        // projectile: ballistic motion is quadratic, Verlet reproduces it exactly
        SVelocityVerletState< TFloat64Point3D > shot( TFloat64Point3D( 0, 0, 0 ), TFloat64Vector3D( 3.0, 10.0, 0.0 ) );
        SGravity gravity;
        gravity.g = TFloat64Vector3D( 0.0, -9.81, 0.0 );
        for ( UInt32 i=0; i<100; ++i )
            VelocityVerletStep( shot, gravity, 0.01 );
        ASSERT_TRUE( fabs( shot.position.x - 3.0 ) < 1e-12 && fabs( shot.position.y - ( 10.0 - 0.5 * 9.81 ) ) < 1e-12 && fabs( shot.velocity.y - ( 10.0 - 9.81 ) ) < 1e-12 );
        // resetting the validity flag after a teleport recomputes the acceleration
        shot.position = TFloat64Point3D( 100, 100, 100 );
        shot.accelerationValid = false;
        VelocityVerletStep( shot, gravity, 0.01 );
        ASSERT_TRUE( shot.accelerationValid && fabs( shot.position.x - 100.03 ) < 1e-9 );
        // vector and point forms of the oscillator conserve energy well
        SVelocityVerletState< TFloat64Vector3D > vec( TFloat64Vector3D( 1, 0, 0 ), TFloat64Vector3D( 0, 1, 0 ) );
        SCentralAcceleration central;
        for ( UInt32 i=0; i<10000; ++i )
            VelocityVerletStep( vec, central, 0.01 );
        ASSERT_TRUE( fabs( 0.5 * ( vec.position.Dot( vec.position ) + vec.velocity.Dot( vec.velocity ) ) - 1.0 ) < 1e-4 );
        SVelocityVerletState< TFloat64Point3D > pnt( TFloat64Point3D( 6, 5, 5 ), TFloat64Vector3D( 0, 1, 0 ) );
        SPointAcceleration pull;
        pull.centre = TFloat64Point3D( 5, 5, 5 );
        for ( UInt32 i=0; i<10000; ++i )
            VelocityVerletStep( pnt, pull, 0.01 );
        ASSERT_TRUE( fabs( ( pnt.position - pull.centre ).GetLength() - 1.0 ) < 1e-3 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Integrators: Runge-Kutta 4 is fourth order (halving dt divides the error by 16)" )
        const Float64 e1 = Rk4Error( 0.2, 2.0 );
        const Float64 e2 = Rk4Error( 0.1, 2.0 );
        const Float64 e3 = Rk4Error( 0.05, 2.0 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Integrators: RK4 errors " << e1 << " " << e2 << " " << e3 << " ratios " << e1 / e2 << " " << e2 / e3 );
        ASSERT_TRUE( e1 / e2 > 12.8 && e1 / e2 < 19.2 && e2 / e3 > 12.8 && e2 / e3 < 19.2 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Integrators: Runge-Kutta 4 with a vector state, a time dependent system and Float32 time" )
        TFloat64Vector2D state( 1.0, 0.0 );
        SOscillatorSystem oscillator;
        for ( UInt32 i=0; i<1000; ++i )
            CTRungeKutta4< TFloat64Vector2D, SOscillatorSystem >::Step( state, i * 0.01, 0.01, oscillator );
        ASSERT_TRUE( fabs( state.x - cos( 10.0 ) ) < 1e-9 && fabs( state.y + sin( 10.0 ) ) < 1e-9 );
        Float64 s = 0.0;
        SCosineSystem cosine;
        for ( UInt32 i=0; i<100; ++i )
            CTRungeKutta4< Float64, SCosineSystem >::Step( s, i * 0.02, 0.02, cosine );
        ASSERT_TRUE( fabs( s - sin( 2.0 ) ) < 1e-9 );
        Float32 x = 1.0f;
        SDecay32 decay32;
        for ( UInt32 i=0; i<20; ++i )
            CTRungeKutta4< Float32, SDecay32, Float32 >::Step( x, i * 0.05f, 0.05f, decay32 );
        ASSERT_TRUE( fabs( x - expf( -1.0f ) ) < 1e-5f );
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
