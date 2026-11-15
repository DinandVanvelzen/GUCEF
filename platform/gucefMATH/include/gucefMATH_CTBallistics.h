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

#ifndef GUCEF_MATH_CTBALLISTICS_H
#define GUCEF_MATH_CTBALLISTICS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <cmath>
#include <limits>

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_CTRADIAN_H
#include "gucefMATH_CTRadian.h"
#define GUCEF_MATH_CTRADIAN_H
#endif /* GUCEF_MATH_CTRADIAN_H ? */

#ifndef GUCEF_MATH_ANGLEFUNCTIONS_H
#include "gucefMATH_AngleFunctions.h"
#define GUCEF_MATH_ANGLEFUNCTIONS_H
#endif /* GUCEF_MATH_ANGLEFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTPOLYNOMIALSOLVER_H
#include "gucefMATH_CTPolynomialSolver.h"
#define GUCEF_MATH_CTPOLYNOMIALSOLVER_H
#endif /* GUCEF_MATH_CTPOLYNOMIALSOLVER_H ? */

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
 *  Projectile kinematics under constant acceleration (gravity) without drag: aiming at a point, intercepting a
 *  moving target, and the basic questions about a trajectory. All functions are static and realtime-safe.
 *
 *  - T: Float32 or Float64. Positions are CTPoint3D, velocities and gravity CTVector3D. Gravity is a vector
 *    (for example ( 0, -9.81, 0 ) with +Y up): "up" is the direction opposite to it; the 2D function
 *    SolveLaunchAngles takes the magnitude of gravity. A zero gravity gives straight line solutions.
 *  - The trajectory is P( t ) = origin + velocity t + gravity t^2 / 2.
 *  - Results that can fail return a bool or a solution count (0, 1 or 2); outputs are left unchanged on failure.
 *    There are no sentinel values.
 *  - Degenerate input: a speed of zero or less, or a target at the shooter, have the results documented per function.
 *  - Intercept solutions use the polynomial solvers of gucefMATH_CTPolynomialSolver.h (quadratic, quartic).
 *  - The precision-sensitive functions are exported instantiations compiled without floating-point contraction.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTBallistics
{
    public:

    typedef T TScalar;
    typedef CTVector3D< T > TVector;
    typedef CTPoint3D< T > TPoint;

    /**
     *  The launch angles above the horizontal that make a projectile of speed 'speed' hit a target at horizontal distance
     *  'horizontalDistance' ( >= 0 ) and height 'heightDifference' ( positive above the shooter ), under gravity of
     *  magnitude 'gravity' ( > 0 ). Returns the number of solutions: 0 (out of range, or speed <= 0), 1 (at the range
     *  limit, straight line without gravity, or straight up for a target directly above) or 2; with two, 'lowAngle'
     *  is the flat (quick) trajectory and 'highAngle' the lobbed one, lowAngle <= highAngle; with one both are equal.
     *  A target directly below or at the shooter (distance 0, height <= 0) has the two vertical solutions -90 and +90 degrees.
     */
    static UInt32 SolveLaunchAngles( const T& speed, const T& horizontalDistance, const T& heightDifference, const T& gravity,
                                     CTRadian< T >& lowAngle, CTRadian< T >& highAngle ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The launch velocities of the given speed that carry a projectile from 'origin' to 'target' under 'gravityVector'
     *  (3D form of SolveLaunchAngles: the horizontal direction is towards the target, "up" opposes gravity). Returns
     *  the number of solutions 0, 1 or 2 with the flat trajectory in 'velocityLow' and the lobbed one in 'velocityHigh'
     *  (equal when there is one).
     */
    static UInt32 SolveLaunchVelocity( const TPoint& origin, const TPoint& target, const T& speed, const TVector& gravityVector,
                                       TVector& velocityLow, TVector& velocityHigh ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The earliest time t >= 0 at which a projectile of constant speed, fired in a straight line (no gravity) from
     *  'shooterPosition', meets a target that moves with constant velocity: | target + v t - shooter | = speed t, a
     *  quadratic. False when it never does (the target is faster and moving away, or speed < 0). A target at the
     *  shooter is hit at time 0.
     */
    static bool SolveInterceptTime( const TPoint& shooterPosition, const T& projectileSpeed, const TPoint& targetPosition,
                                    const TVector& targetVelocity, T& time ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  As SolveInterceptTime with gravity: the earliest time t > 0 and the launch velocity (of length 'projectileSpeed')
     *  that hit a target moving with constant velocity, a quartic in t. False when no real solution exists.
     */
    static bool SolveInterceptWithGravity( const TPoint& shooterPosition, const T& projectileSpeed, const TPoint& targetPosition,
                                           const TVector& targetVelocity, const TVector& gravityVector,
                                           T& time, TVector& launchVelocity ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The highest point of the trajectory: its position and the time to reach it. A projectile that does not rise
     *  (velocity not against gravity) has its apex at the origin at time 0. False without gravity.
     */
    static bool GetApex( const TPoint& origin, const TVector& velocity, const TVector& gravityVector, TPoint& apex, T& time ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The time at which the projectile is at 'heightDifference' above its origin (along "up") on its way down: the
     *  later of the two crossings. False without gravity or when the projectile never gets that high.
     */
    static bool GetTimeOfFlight( const TVector& velocity, const TVector& gravityVector, const T& heightDifference, T& time ) GUCEF_MATH_REALTIME_CONTEXT;

    /** origin + velocity t + gravity t^2 / 2 */
    static TPoint GetPositionAtTime( const TPoint& origin, const TVector& velocity, const TVector& gravityVector, const T& time ) GUCEF_MATH_REALTIME_CONTEXT;
};

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE UInt32
CTBallistics< T >::SolveLaunchAngles( const T& speed, const T& horizontalDistance, const T& heightDifference, const T& gravity,
                                      CTRadian< T >& lowAngle, CTRadian< T >& highAngle ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::IsFloatingPoint, CTBallistics_requires_a_floating_point_kind );
    const T halfPi = CTMathConstants< T >::Pi() / T( 2 );
    const T x = horizontalDistance < T( 0 ) ? T( 0 ) : horizontalDistance;
    const T y = heightDifference;
    if ( !( speed > T( 0 ) ) )
        return 0;

    if ( !( gravity > T( 0 ) ) )
    {
        // no gravity: the straight line to the target
        if ( x == T( 0 ) && y == T( 0 ) )
            return 0;
        const CTRadian< T > angle = Atan2( y, x );
        lowAngle = angle;
        highAngle = angle;
        return 1;
    }

    const T v2 = speed * speed;
    if ( x == T( 0 ) )
    {
        if ( y > T( 0 ) )
        {
            if ( v2 < T( 2 ) * gravity * y )
                return 0;
            lowAngle = CTRadian< T >( halfPi );
            highAngle = lowAngle;
            return 1;
        }
        lowAngle = CTRadian< T >( -halfPi );
        highAngle = CTRadian< T >( halfPi );
        return 2;
    }

    const T discriminant = v2 * v2 - gravity * ( gravity * x * x + T( 2 ) * y * v2 );
    if ( discriminant < T( 0 ) )
        return 0;
    const T root = std::sqrt( discriminant );
    const T denominator = gravity * x;
    // ( v2 - root ) / ( g x ) rewritten without the cancellation: v2^2 - root^2 = g ( g x^2 + 2 y v2 )
    lowAngle = Atan( ( gravity * x * x + T( 2 ) * y * v2 ) / ( x * ( v2 + root ) ) );
    highAngle = Atan( ( v2 + root ) / denominator );
    return discriminant == T( 0 ) ? 1 : 2;
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE UInt32
CTBallistics< T >::SolveLaunchVelocity( const TPoint& origin, const TPoint& target, const T& speed, const TVector& gravityVector,
                                        TVector& velocityLow, TVector& velocityHigh ) GUCEF_MATH_REALTIME_CONTEXT
{
    const TVector delta = target - origin;
    const T g = gravityVector.GetLength();
    if ( !( speed > T( 0 ) ) )
        return 0;
    if ( !( g > T( 0 ) ) )
    {
        TVector direction;
        if ( !delta.TryNormalize( direction, T( 0 ) ) )
            return 0;
        velocityLow = direction * speed;
        velocityHigh = velocityLow;
        return 1;
    }

    const TVector up = gravityVector * ( T( -1 ) / g );
    const T height = delta.Dot( up );
    const TVector horizontal = delta - up * height;
    const T distance = horizontal.GetLength();
    TVector horizontalDirection;
    if ( distance > T( 0 ) )
        horizontalDirection = horizontal * ( T( 1 ) / distance );

    CTRadian< T > low, high;
    const UInt32 count = SolveLaunchAngles( speed, distance, height, g, low, high );
    if ( 0 == count )
        return 0;
    velocityLow = horizontalDirection * ( speed * Cos( low ) ) + up * ( speed * Sin( low ) );
    velocityHigh = horizontalDirection * ( speed * Cos( high ) ) + up * ( speed * Sin( high ) );
    return count;
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTBallistics< T >::SolveInterceptTime( const TPoint& shooterPosition, const T& projectileSpeed, const TPoint& targetPosition,
                                       const TVector& targetVelocity, T& time ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( projectileSpeed < T( 0 ) )
        return false;
    const TVector d = targetPosition - shooterPosition;
    const T distance = d.GetLength();
    if ( distance == T( 0 ) )
    {
        time = T( 0 );
        return true;
    }
    // dimensionless form: lengths in units of the distance, time in units of distance / speed, so the
    // coefficients of the quadratic are of order one whatever the scale of the problem
    const T timeScale = projectileSpeed > T( 0 ) ? distance / projectileSpeed : distance;
    const T speedScale = timeScale / distance;
    const TVector dn = d * ( T( 1 ) / distance );
    const TVector vn = targetVelocity * speedScale;
    const T sn = projectileSpeed * speedScale;
    const T a = vn.Dot( vn ) - sn * sn;
    const T b = T( 2 ) * dn.Dot( vn );
    T roots[ 2 ];
    const UInt32 count = CTPolynomialSolver< T >::SolveQuadratic( a, b, T( 1 ), roots );
    bool found = false;
    T best = T( 0 );
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( roots[ i ] >= T( 0 ) && ( !found || roots[ i ] < best ) )
        {
            best = roots[ i ];
            found = true;
        }
    }
    if ( found )
        time = best * timeScale;
    return found;
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTBallistics< T >::SolveInterceptWithGravity( const TPoint& shooterPosition, const T& projectileSpeed, const TPoint& targetPosition,
                                              const TVector& targetVelocity, const TVector& gravityVector,
                                              T& time, TVector& launchVelocity ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( projectileSpeed < T( 0 ) )
        return false;
    const TVector d = targetPosition - shooterPosition;
    const T distance = d.GetLength();
    if ( distance == T( 0 ) )
    {
        time = T( 0 );
        launchVelocity = TVector();
        return true;
    }
    // | d + v t - g t^2 / 2 | = s t in dimensionless form (lengths in units of the distance, time in units of
    // distance / speed): the solver drops a leading coefficient that is small compared to the others, which would
    // otherwise depend on the scale of the problem
    const T timeScale = projectileSpeed > T( 0 ) ? distance / projectileSpeed : distance;
    const T speedScale = timeScale / distance;
    const TVector dn = d * ( T( 1 ) / distance );
    const TVector vn = targetVelocity * speedScale;
    const TVector gn = gravityVector * ( speedScale * timeScale );
    const T sn = projectileSpeed * speedScale;
    const T gg = gn.Dot( gn );
    const T vg = vn.Dot( gn );
    const T vv = vn.Dot( vn );
    const T dg = dn.Dot( gn );
    const T dv = dn.Dot( vn );
    // candidates: the roots of the quartic and, as starting points that stay accurate when gravity is weak compared
    // to the speed, the roots of the gravity free quadratic; each is polished on the real equation and verified
    T candidates[ 6 ];
    UInt32 candidateCount = CTPolynomialSolver< T >::SolveQuartic( T( 0.25 ) * gg, -vg, vv - dg - sn * sn, T( 2 ) * dv, T( 1 ), candidates );
    candidateCount += CTPolynomialSolver< T >::SolveQuadratic( vv - sn * sn, T( 2 ) * dv, T( 1 ), candidates + candidateCount );
    const T tolerance = std::sqrt( std::numeric_limits< T >::epsilon() );
    bool found = false;
    T best = T( 0 );
    for ( UInt32 i=0; i<candidateCount; ++i )
    {
        T tau = candidates[ i ];
        if ( !( tau > T( 0 ) ) )
            continue;
        // Newton steps on f( tau ) = | q( tau ) |^2 - ( sn tau )^2, kept only where |f| shrinks
        for ( UInt32 step=0; step<8; ++step )
        {
            const TVector q = dn + vn * tau - gn * ( T( 0.5 ) * tau * tau );
            const TVector dq = vn - gn * tau;
            const T f = q.Dot( q ) - sn * sn * tau * tau;
            const T slope = T( 2 ) * ( q.Dot( dq ) - sn * sn * tau );
            if ( !( slope != T( 0 ) ) )
                break;
            const T next = tau - f / slope;
            if ( !( next > T( 0 ) ) )
                break;
            const TVector qNext = dn + vn * next - gn * ( T( 0.5 ) * next * next );
            const T fNext = qNext.Dot( qNext ) - sn * sn * next * next;
            if ( !( std::fabs( fNext ) < std::fabs( f ) ) )
                break;
            tau = next;
        }
        const TVector qn = dn + vn * tau - gn * ( T( 0.5 ) * tau * tau );
        if ( projectileSpeed > T( 0 ) && std::fabs( qn.GetLength() - sn * tau ) > tolerance * sn * tau )
            continue;
        if ( !found || tau < best )
        {
            best = tau;
            found = true;
        }
    }
    if ( !found )
        return false;
    time = best * timeScale;
    const TVector qBest = dn + vn * best - gn * ( T( 0.5 ) * best * best );
    launchVelocity = qBest * ( distance / time );
    return true;
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTBallistics< T >::GetApex( const TPoint& origin, const TVector& velocity, const TVector& gravityVector, TPoint& apex, T& time ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T g = gravityVector.GetLength();
    if ( !( g > T( 0 ) ) )
        return false;
    const T rise = velocity.Dot( gravityVector ) * ( T( -1 ) / g );    // speed along "up"
    if ( !( rise > T( 0 ) ) )
    {
        apex = origin;
        time = T( 0 );
        return true;
    }
    time = rise / g;
    apex = GetPositionAtTime( origin, velocity, gravityVector, time );
    return true;
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTBallistics< T >::GetTimeOfFlight( const TVector& velocity, const TVector& gravityVector, const T& heightDifference, T& time ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T g = gravityVector.GetLength();
    if ( !( g > T( 0 ) ) )
        return false;
    const T rise = velocity.Dot( gravityVector ) * ( T( -1 ) / g );
    const T discriminant = rise * rise - T( 2 ) * g * heightDifference;
    if ( discriminant < T( 0 ) )
        return false;
    const T result = ( rise + std::sqrt( discriminant ) ) / g;
    if ( result < T( 0 ) )
        return false;
    time = result;
    return true;
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTPoint3D< T >
CTBallistics< T >::GetPositionAtTime( const TPoint& origin, const TVector& velocity, const TVector& gravityVector, const T& time ) GUCEF_MATH_REALTIME_CONTEXT
{
    return origin + ( velocity * time + gravityVector * ( T( 0.5 ) * time * time ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTBallistics< Float32 > )
GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTBallistics< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTBALLISTICS_H ? */
