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

#ifndef GUCEF_MATH_SMOOTHINGFUNCTIONS_H
#define GUCEF_MATH_SMOOTHINGFUNCTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <cmath>
#include <limits>

#ifndef GUCEF_MATH_CTCURVEPOINTTRAITS_H
#include "gucefMATH_CTCurvePointTraits.h"
#define GUCEF_MATH_CTCURVEPOINTTRAITS_H
#endif /* GUCEF_MATH_CTCURVEPOINTTRAITS_H ? */

#ifndef GUCEF_MATH_CTRADIAN_H
#include "gucefMATH_CTRadian.h"
#define GUCEF_MATH_CTRADIAN_H
#endif /* GUCEF_MATH_CTRADIAN_H ? */

#ifndef GUCEF_MATH_ANGLEFUNCTIONS_H
#include "gucefMATH_AngleFunctions.h"
#define GUCEF_MATH_ANGLEFUNCTIONS_H
#endif /* GUCEF_MATH_ANGLEFUNCTIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The state that SmoothDamp() carries from frame to frame: the current velocity (of type TVector of the value type:
 *  a scalar for scalars and angles, a vector for vectors and points). Starts at rest.
 */
template < typename TValue >
struct SSmoothDampState
{
    typedef CTCurvePointTraits< TValue > TTraits;

    typename TTraits::TVector velocity;

    SSmoothDampState( void ) : velocity( TTraits::Zero() ) {}
    void Reset( void ) { velocity = TTraits::Zero(); }
};

/** The state of SpringStep(): the position and velocity of a unit mass on a damped spring */
template < typename TValue >
struct SSpringState
{
    typedef CTCurvePointTraits< TValue > TTraits;

    TValue position;
    typename TTraits::TVector velocity;

    SSpringState( void ) : position(), velocity( TTraits::Zero() ) {}
    explicit SSpringState( const TValue& startPosition ) : position( startPosition ), velocity( TTraits::Zero() ) {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Smoothing of values over time, frame-rate independent where stated. The value type is a Float32 / Float64 scalar,
 *  CTVector2D / 3D / 4D or CTPoint2D / 3D (CTCurvePointTraits); angles have their own SmoothDamp overload. All are
 *  realtime-safe. Predefined kinds (scalars, 2D and 3D vectors, floats) are exported instantiations compiled without
 *  floating-point contraction; other types are instantiated by the user.
 */

/**
 *  Critically damped spring towards 'target' (the approximation of Game Programming Gems 4, chapter 1.10): reaches the
 *  target without overshoot in about 'smoothTime' seconds, whatever the time step ( 60 Hz and 240 Hz agree to 1e-3 ).
 *  'state.velocity' is updated; 'maxSpeed' limits the speed (infinity for none). smoothTime below 0.0001 is raised to it;
 *  deltaTime <= 0 returns 'current' unchanged. The result never passes the target.
 */
template < typename TValue >
TValue
SmoothDamp( const TValue& current, const TValue& target, SSmoothDampState< TValue >& state,
            const typename CTCurvePointTraits< TValue >::TScalar& smoothTime,
            const typename CTCurvePointTraits< TValue >::TScalar& maxSpeed,
            const typename CTCurvePointTraits< TValue >::TScalar& deltaTime ) GUCEF_MATH_REALTIME_CONTEXT;

/** SmoothDamp() without a speed limit */
template < typename TValue >
GUCEF_MATH_FORCE_INLINE TValue
SmoothDamp( const TValue& current, const TValue& target, SSmoothDampState< TValue >& state,
            const typename CTCurvePointTraits< TValue >::TScalar& smoothTime,
            const typename CTCurvePointTraits< TValue >::TScalar& deltaTime ) GUCEF_MATH_REALTIME_CONTEXT
{
    return SmoothDamp( current, target, state, smoothTime, std::numeric_limits< typename CTCurvePointTraits< TValue >::TScalar >::infinity(), deltaTime );
}

/** SmoothDamp() of an angle along the shortest way round; 'state.velocity' is in radians per second */
template < typename T >
CTRadian< T >
SmoothDamp( const CTRadian< T >& current, const CTRadian< T >& target, SSmoothDampState< T >& state,
            const T& smoothTime, const T& maxSpeed, const T& deltaTime ) GUCEF_MATH_REALTIME_CONTEXT;

/** SmoothDamp() of an angle without a speed limit */
template < typename T >
GUCEF_MATH_FORCE_INLINE CTRadian< T >
SmoothDamp( const CTRadian< T >& current, const CTRadian< T >& target, SSmoothDampState< T >& state,
            const T& smoothTime, const T& deltaTime ) GUCEF_MATH_REALTIME_CONTEXT
{
    return SmoothDamp( current, target, state, smoothTime, std::numeric_limits< T >::infinity(), deltaTime );
}

/**
 *  Exponential approach: moves the fraction 1 - exp( -speed deltaTime ) of the way to 'target', so the result after a
 *  time is the same for any number of steps (frame-rate independent). speed <= 0 returns 'target' (the convention of
 *  Unreal's FInterpTo); deltaTime <= 0 returns 'current'.
 */
template < typename TValue >
TValue
InterpTo( const TValue& current, const TValue& target,
          const typename CTCurvePointTraits< TValue >::TScalar& deltaTime,
          const typename CTCurvePointTraits< TValue >::TScalar& speed ) GUCEF_MATH_REALTIME_CONTEXT;

/** Moves towards 'target' at the constant speed 'speed' ( units per second ); arrives exactly, never overshoots */
template < typename TValue >
TValue
InterpConstantTo( const TValue& current, const TValue& target,
                  const typename CTCurvePointTraits< TValue >::TScalar& deltaTime,
                  const typename CTCurvePointTraits< TValue >::TScalar& speed ) GUCEF_MATH_REALTIME_CONTEXT;

/**
 *  One semi-implicit Euler step of a unit mass on a damped spring pulling towards 'target':
 *  a = -stiffness ( x - target ) - damping v; v += a dt; x += v dt. Stable for any damping ratio while
 *  deltaTime * sqrt( stiffness ) stays well below 2. Critical damping is GetCriticalDamping( stiffness ).
 */
template < typename TValue >
void
SpringStep( SSpringState< TValue >& state, const TValue& target,
            const typename CTCurvePointTraits< TValue >::TScalar& stiffness,
            const typename CTCurvePointTraits< TValue >::TScalar& damping,
            const typename CTCurvePointTraits< TValue >::TScalar& deltaTime ) GUCEF_MATH_REALTIME_CONTEXT;

/** The damping that makes SpringStep() critically damped: 2 sqrt( stiffness ) */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
GetCriticalDamping( const T& stiffness ) GUCEF_MATH_REALTIME_CONTEXT
{
    return T( 2 ) * std::sqrt( stiffness );
}

/*-------------------------------------------------------------------------*/

/*
 *  MoveTowards for vectors and points: a step of at most 'maxDelta' towards 'target', exactly 'target' when it is
 *  within reach. (The scalar form is in gucefMATH_ScalarFunctions.h.) A negative maxDelta moves away from the target.
 */

template < typename TValue >
TValue
MoveTowardsValue( const TValue& current, const TValue& target, const typename CTCurvePointTraits< TValue >::TScalar& maxDelta ) GUCEF_MATH_REALTIME_CONTEXT;

template < typename T > GUCEF_MATH_FORCE_INLINE CTVector2D< T > MoveTowards( const CTVector2D< T >& current, const CTVector2D< T >& target, const T& maxDelta ) GUCEF_MATH_REALTIME_CONTEXT { return MoveTowardsValue( current, target, maxDelta ); }
template < typename T > GUCEF_MATH_FORCE_INLINE CTVector3D< T > MoveTowards( const CTVector3D< T >& current, const CTVector3D< T >& target, const T& maxDelta ) GUCEF_MATH_REALTIME_CONTEXT { return MoveTowardsValue( current, target, maxDelta ); }
template < typename T > GUCEF_MATH_FORCE_INLINE CTVector4D< T > MoveTowards( const CTVector4D< T >& current, const CTVector4D< T >& target, const T& maxDelta ) GUCEF_MATH_REALTIME_CONTEXT { return MoveTowardsValue( current, target, maxDelta ); }
template < typename T > GUCEF_MATH_FORCE_INLINE CTPoint2D< T > MoveTowards( const CTPoint2D< T >& current, const CTPoint2D< T >& target, const T& maxDelta ) GUCEF_MATH_REALTIME_CONTEXT { return MoveTowardsValue( current, target, maxDelta ); }
template < typename T > GUCEF_MATH_FORCE_INLINE CTPoint3D< T > MoveTowards( const CTPoint3D< T >& current, const CTPoint3D< T >& target, const T& maxDelta ) GUCEF_MATH_REALTIME_CONTEXT { return MoveTowardsValue( current, target, maxDelta ); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename TValue >
GUCEF_MATH_PRECISE_INLINE TValue
SmoothDamp( const TValue& current, const TValue& target, SSmoothDampState< TValue >& state,
            const typename CTCurvePointTraits< TValue >::TScalar& smoothTime,
            const typename CTCurvePointTraits< TValue >::TScalar& maxSpeed,
            const typename CTCurvePointTraits< TValue >::TScalar& deltaTime ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTCurvePointTraits< TValue > TTraits;
    typedef typename TTraits::TScalar TScalar;
    typedef typename TTraits::TVector TVector;

    if ( !( deltaTime > TScalar( 0 ) ) )
        return current;
    const TScalar time = smoothTime > TScalar( 0.0001 ) ? smoothTime : TScalar( 0.0001 );
    const TScalar omega = TScalar( 2 ) / time;
    const TScalar x = omega * deltaTime;
    const TScalar decay = TScalar( 1 ) / ( TScalar( 1 ) + x + TScalar( 0.48 ) * x * x + TScalar( 0.235 ) * x * x * x );

    // the distance to the target, limited so that the speed stays below maxSpeed
    TVector change = TTraits::Sub( current, target );
    const TScalar maxChange = maxSpeed * time;
    const TScalar length = TTraits::Length( change );
    if ( length > maxChange && length > TScalar( 0 ) )
        change = TTraits::Scale( change, maxChange / length );
    const TValue limitedTarget = TTraits::Offset( current, TTraits::Negate( change ) );

    const TVector temp = TTraits::Scale( TTraits::Add( state.velocity, TTraits::Scale( change, omega ) ), deltaTime );
    state.velocity = TTraits::Scale( TTraits::SubVectors( state.velocity, TTraits::Scale( temp, omega ) ), decay );
    TValue output = TTraits::Offset( limitedTarget, TTraits::Scale( TTraits::Add( change, temp ), decay ) );

    // never pass the target
    const TVector toTarget = TTraits::Sub( target, current );
    const TVector pastTarget = TTraits::Sub( output, target );
    if ( TTraits::Dot( toTarget, pastTarget ) > TScalar( 0 ) )
    {
        output = target;
        state.velocity = TTraits::Zero();
    }
    return output;
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
SmoothDamp( const CTRadian< T >& current, const CTRadian< T >& target, SSmoothDampState< T >& state,
            const T& smoothTime, const T& maxSpeed, const T& deltaTime ) GUCEF_MATH_REALTIME_CONTEXT
{
    // work on the unwrapped angle: the target is moved to the representative nearest to current
    const CTRadian< T > unwrapped = current + DeltaAngle( current, target );
    return CTRadian< T >( SmoothDamp< T >( current.GetValue(), unwrapped.GetValue(), state, smoothTime, maxSpeed, deltaTime ) );
}

template < typename TValue >
GUCEF_MATH_PRECISE_INLINE TValue
InterpTo( const TValue& current, const TValue& target,
          const typename CTCurvePointTraits< TValue >::TScalar& deltaTime,
          const typename CTCurvePointTraits< TValue >::TScalar& speed ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef typename CTCurvePointTraits< TValue >::TScalar TScalar;
    if ( !( speed > TScalar( 0 ) ) )
        return target;
    if ( !( deltaTime > TScalar( 0 ) ) )
        return current;
    const TScalar fraction = TScalar( 1 ) - std::exp( -speed * deltaTime );
    return CurveLerp( current, target, fraction );
}

template < typename TValue >
GUCEF_MATH_PRECISE_INLINE TValue
MoveTowardsValue( const TValue& current, const TValue& target, const typename CTCurvePointTraits< TValue >::TScalar& maxDelta ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTCurvePointTraits< TValue > TTraits;
    typedef typename TTraits::TScalar TScalar;
    const typename TTraits::TVector delta = TTraits::Sub( target, current );
    const TScalar distance = TTraits::Length( delta );
    if ( distance <= maxDelta || distance == TScalar( 0 ) )
        return target;
    return TTraits::Offset( current, TTraits::Scale( delta, maxDelta / distance ) );
}

template < typename TValue >
GUCEF_MATH_PRECISE_INLINE TValue
InterpConstantTo( const TValue& current, const TValue& target,
                  const typename CTCurvePointTraits< TValue >::TScalar& deltaTime,
                  const typename CTCurvePointTraits< TValue >::TScalar& speed ) GUCEF_MATH_REALTIME_CONTEXT
{
    return MoveTowardsValue( current, target, speed * deltaTime );
}

template < typename TValue >
GUCEF_MATH_PRECISE_INLINE void
SpringStep( SSpringState< TValue >& state, const TValue& target,
            const typename CTCurvePointTraits< TValue >::TScalar& stiffness,
            const typename CTCurvePointTraits< TValue >::TScalar& damping,
            const typename CTCurvePointTraits< TValue >::TScalar& deltaTime ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTCurvePointTraits< TValue > TTraits;
    typedef typename TTraits::TVector TVector;
    const TVector displacement = TTraits::Sub( state.position, target );
    const TVector acceleration = TTraits::Add( TTraits::Negate( TTraits::Scale( displacement, stiffness ) ), TTraits::Negate( TTraits::Scale( state.velocity, damping ) ) );
    state.velocity = TTraits::Add( state.velocity, TTraits::Scale( acceleration, deltaTime ) );
    state.position = TTraits::Offset( state.position, TTraits::Scale( state.velocity, deltaTime ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The predefined value types of the smoothing functions: scalars, 2D and 3D vectors of both float kinds. X( kind,
 *  scalar ) is invoked per type with the kind and its scalar; used by the extern declarations here and by the
 *  instantiations in the module source.
 */
#define GUCEF_MATH_SMOOTHING_PREDEFINED( X )                    \
    X( Float32, Float32 )          X( Float64, Float64 )        \
    X( TFloat32Vector2D, Float32 ) X( TFloat64Vector2D, Float64 ) \
    X( TFloat32Vector3D, Float32 ) X( TFloat64Vector3D, Float64 )

#define GUCEF_MATH_SMOOTHING_FUNCTIONS( prefix, kind, scalar )                                                                                                     \
    prefix kind SmoothDamp< kind >( const kind&, const kind&, SSmoothDampState< kind >&, const scalar&, const scalar&, const scalar& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix kind InterpTo< kind >( const kind&, const kind&, const scalar&, const scalar& ) GUCEF_MATH_REALTIME_CONTEXT;                                             \
    prefix kind InterpConstantTo< kind >( const kind&, const kind&, const scalar&, const scalar& ) GUCEF_MATH_REALTIME_CONTEXT;                                    \
    prefix kind MoveTowardsValue< kind >( const kind&, const kind&, const scalar& ) GUCEF_MATH_REALTIME_CONTEXT;                                                   \
    prefix void SpringStep< kind >( SSpringState< kind >&, const kind&, const scalar&, const scalar&, const scalar& ) GUCEF_MATH_REALTIME_CONTEXT;

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
#define GUCEF_MATH_SMOOTHING_EXTERN( kind, scalar ) GUCEF_MATH_SMOOTHING_FUNCTIONS( GUCEF_MATH_EXTERN_TEMPLATE, kind, scalar )
GUCEF_MATH_SMOOTHING_PREDEFINED( GUCEF_MATH_SMOOTHING_EXTERN )
GUCEF_MATH_EXTERN_TEMPLATE CTRadian< Float32 > SmoothDamp< Float32 >( const CTRadian< Float32 >&, const CTRadian< Float32 >&, SSmoothDampState< Float32 >&, const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTRadian< Float64 > SmoothDamp< Float64 >( const CTRadian< Float64 >&, const CTRadian< Float64 >&, SSmoothDampState< Float64 >&, const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_SMOOTHINGFUNCTIONS_H ? */
