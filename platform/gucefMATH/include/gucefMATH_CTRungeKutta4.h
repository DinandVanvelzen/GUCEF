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

#ifndef GUCEF_MATH_CTRUNGEKUTTA4_H
#define GUCEF_MATH_CTRUNGEKUTTA4_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

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
 *  The classical fourth order Runge-Kutta step for the system y' = f( t, y ): halving the step divides the error by
 *  about 16. Header only; the work happens in the caller's types, so there are no exported instantiations.
 *
 *  - TState: the state vector of the system. Needs a default constructor, copy, TState + TState and TState * TTime
 *    (a CTVector2D / 3D / 4D of position and velocity, or a user struct with those operators).
 *  - TSystem: a functor class providing
 *        void Evaluate( const TTime& t, const TState& state, TState& derivative )
 *    which stores f( t, state ) in 'derivative'.
 *  - TTime: the time and step type, Float64 unless given.
 *  - Four evaluations of the system per step, no allocation; realtime-safe when the system is.
 */
template < typename TState, typename TSystem, typename TTime = Float64 >
class GUCEF_MATH_TEMPLATE_PUBLIC CTRungeKutta4
{
    public:

    /** Advances 'state' from time t to t + dt */
    static void Step( TState& state, const TTime& t, const TTime& dt, TSystem& system ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const TTime half = TTime( 0.5 ) * dt;
        TState k1, k2, k3, k4;
        system.Evaluate( t, state, k1 );
        system.Evaluate( t + half, state + k1 * half, k2 );
        system.Evaluate( t + half, state + k2 * half, k3 );
        system.Evaluate( t + dt, state + k3 * dt, k4 );
        state = state + ( k1 + k2 * TTime( 2 ) + k3 * TTime( 2 ) + k4 ) * ( dt / TTime( 6 ) );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTRUNGEKUTTA4_H ? */
