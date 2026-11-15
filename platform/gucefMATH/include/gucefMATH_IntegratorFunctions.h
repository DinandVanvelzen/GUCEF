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

#ifndef GUCEF_MATH_INTEGRATORFUNCTIONS_H
#define GUCEF_MATH_INTEGRATORFUNCTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTCURVEPOINTTRAITS_H
#include "gucefMATH_CTCurvePointTraits.h"
#define GUCEF_MATH_CTCURVEPOINTTRAITS_H
#endif /* GUCEF_MATH_CTCURVEPOINTTRAITS_H ? */

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
 *  The state of a velocity Verlet integration: position, velocity and the acceleration at the position. The
 *  acceleration is valid only after the first step (or when the caller sets it and 'accelerationValid'); VelocityVerletStep()
 *  computes it from the functor when it is not valid, so a default state works. Changing 'position' by hand
 *  (teleporting) requires 'accelerationValid = false'.
 */
template < typename TValue >
struct SVelocityVerletState
{
    typedef CTCurvePointTraits< TValue > TTraits;

    TValue position;
    typename TTraits::TVector velocity;
    typename TTraits::TVector acceleration;
    bool accelerationValid;

    SVelocityVerletState( void ) : position(), velocity( TTraits::Zero() ), acceleration( TTraits::Zero() ), accelerationValid( false ) {}
    SVelocityVerletState( const TValue& startPosition, const typename TTraits::TVector& startVelocity )
        : position( startPosition ), velocity( startVelocity ), acceleration( TTraits::Zero() ), accelerationValid( false ) {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Fixed step integrators for particles and simple dynamics. The value type is a Float32 / Float64 scalar,
 *  CTVector2D / 3D / 4D or CTPoint2D / 3D (CTCurvePointTraits); velocities and accelerations are the matching
 *  displacement type. All are realtime-safe.
 */

/**
 *  Semi-implicit (symplectic) Euler: v += a dt, then x += v dt. First order, but the energy of an oscillator stays
 *  bounded (a drift of well under 1 % over 100 000 steps of 0.01 on x'' = -x), which makes it the default for game
 *  physics where the force does not depend on the velocity.
 */
template < typename TValue >
void
SemiImplicitEulerStep( TValue& position, typename CTCurvePointTraits< TValue >::TVector& velocity,
                       const typename CTCurvePointTraits< TValue >::TVector& acceleration,
                       const typename CTCurvePointTraits< TValue >::TScalar& deltaTime ) GUCEF_MATH_REALTIME_CONTEXT;

/**
 *  Velocity Verlet: x += v dt + a dt^2 / 2, a' = a( x ), v += ( a + a' ) dt / 2. Second order (halving dt divides the
 *  error by about 4), time reversible, needs one evaluation of the acceleration per step. The acceleration must depend
 *  on the position only. TAccelerationFunctor provides
 *      void Evaluate( const TValue& position, TVector& acceleration )
 *  (a functor object, resolved at compile time; no std::function).
 */
template < typename TValue, typename TAccelerationFunctor >
void
VelocityVerletStep( SVelocityVerletState< TValue >& state, TAccelerationFunctor& acceleration,
                    const typename CTCurvePointTraits< TValue >::TScalar& deltaTime ) GUCEF_MATH_REALTIME_CONTEXT;

/*-------------------------------------------------------------------------*/

template < typename TValue >
GUCEF_MATH_PRECISE_INLINE void
SemiImplicitEulerStep( TValue& position, typename CTCurvePointTraits< TValue >::TVector& velocity,
                       const typename CTCurvePointTraits< TValue >::TVector& acceleration,
                       const typename CTCurvePointTraits< TValue >::TScalar& deltaTime ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTCurvePointTraits< TValue > TTraits;
    velocity = TTraits::Add( velocity, TTraits::Scale( acceleration, deltaTime ) );
    position = TTraits::Offset( position, TTraits::Scale( velocity, deltaTime ) );
}

template < typename TValue, typename TAccelerationFunctor >
void
VelocityVerletStep( SVelocityVerletState< TValue >& state, TAccelerationFunctor& acceleration,
                    const typename CTCurvePointTraits< TValue >::TScalar& deltaTime ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTCurvePointTraits< TValue > TTraits;
    typedef typename TTraits::TScalar TScalar;
    typedef typename TTraits::TVector TVector;

    if ( !state.accelerationValid )
    {
        acceleration.Evaluate( state.position, state.acceleration );
        state.accelerationValid = true;
    }
    const TVector step = TTraits::Add( TTraits::Scale( state.velocity, deltaTime ), TTraits::Scale( state.acceleration, TScalar( 0.5 ) * deltaTime * deltaTime ) );
    state.position = TTraits::Offset( state.position, step );
    TVector next = TTraits::Zero();
    acceleration.Evaluate( state.position, next );
    state.velocity = TTraits::Add( state.velocity, TTraits::Scale( TTraits::Add( state.acceleration, next ), TScalar( 0.5 ) * deltaTime ) );
    state.acceleration = next;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The predefined value types of the integrator functions: X( kind, scalar ) per type (the functor based steps can not be exported) */
#define GUCEF_MATH_INTEGRATOR_PREDEFINED( X )                     \
    X( Float32, Float32 )          X( Float64, Float64 )          \
    X( TFloat32Vector2D, Float32 ) X( TFloat64Vector2D, Float64 ) \
    X( TFloat32Vector3D, Float32 ) X( TFloat64Vector3D, Float64 )

#define GUCEF_MATH_INTEGRATOR_FUNCTIONS( prefix, kind, scalar )                                                                                                      \
    prefix void SemiImplicitEulerStep< kind >( kind&, CTCurvePointTraits< kind >::TVector&, const CTCurvePointTraits< kind >::TVector&, const scalar& ) GUCEF_MATH_REALTIME_CONTEXT;

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
#define GUCEF_MATH_INTEGRATOR_EXTERN( kind, scalar ) GUCEF_MATH_INTEGRATOR_FUNCTIONS( GUCEF_MATH_EXTERN_TEMPLATE, kind, scalar )
GUCEF_MATH_INTEGRATOR_PREDEFINED( GUCEF_MATH_INTEGRATOR_EXTERN )
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_INTEGRATORFUNCTIONS_H ? */
