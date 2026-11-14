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

#ifndef GUCEF_MATH_CTDIRECTIVITYPATTERN_H
#define GUCEF_MATH_CTDIRECTIVITYPATTERN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <cmath>

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_MATH_SCALARFUNCTIONS_H
#include "gucefMATH_ScalarFunctions.h"
#define GUCEF_MATH_SCALARFUNCTIONS_H
#endif /* GUCEF_MATH_SCALARFUNCTIONS_H ? */


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
 *  Directivity of a sound source or a microphone of the first order family:
 *
 *      g( theta ) = | ( 1 - alpha ) + alpha cos( theta ) | ^ p
 *
 *  where theta is the angle from the axis of the pattern, alpha (0 to 1) the pattern and p the sharpness
 *  (1 gives the textbook pattern, larger values narrow it). The presets are omnidirectional (alpha 0), cardioid
 *  (0.5), supercardioid ( ( 3 - sqrt( 3 ) ) / 2 ), hypercardioid (0.75) and figure of eight (1).
 *
 *  A value type: no allocation. T is a floating point kind. Evaluate() is realtime safe.
 */
template< typename T >
class CTDirectivityPattern
{
    public:

    typedef T value_type;

    /** Omnidirectional */
    CTDirectivityPattern( void ) GUCEF_MATH_REALTIME_CONTEXT
        : m_alpha( 0 ) ,
          m_sharpness( 1 )
    {
    }

    /** The alpha is clamped to [0, 1] and the sharpness to a minimum of 0 */
    CTDirectivityPattern( const T& alpha, const T& sharpness ) GUCEF_MATH_REALTIME_CONTEXT
        : m_alpha( 0 ) ,
          m_sharpness( 1 )
    {
        SetAlpha( alpha );
        SetSharpness( sharpness );
    }

    static CTDirectivityPattern Omnidirectional( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTDirectivityPattern( T( 0 ), T( 1 ) ); }
    static CTDirectivityPattern Cardioid( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTDirectivityPattern( T( 0.5 ), T( 1 ) ); }
    static CTDirectivityPattern Supercardioid( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTDirectivityPattern( ( T( 3 ) - std::sqrt( T( 3 ) ) ) / T( 2 ), T( 1 ) ); }
    static CTDirectivityPattern Hypercardioid( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTDirectivityPattern( T( 0.75 ), T( 1 ) ); }
    static CTDirectivityPattern FigureEight( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTDirectivityPattern( T( 1 ), T( 1 ) ); }

    T GetAlpha( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_alpha; }
    T GetSharpness( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_sharpness; }

    void SetAlpha( const T& alpha ) GUCEF_MATH_REALTIME_CONTEXT
    {
        m_alpha = Clamp01( alpha );
    }

    void SetSharpness( const T& sharpness ) GUCEF_MATH_REALTIME_CONTEXT
    {
        m_sharpness = sharpness > T( 0 ) ? sharpness : T( 0 );
    }

    /**
     *  Gain for the cosine of the angle between the axis of the pattern and the direction of interest,
     *  clamped to [-1, 1]. The result is never negative: the sign of the lobes of the figure of eight is
     *  not part of the gain.
     */
    T Evaluate( const T& cosAngle ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        const T c = Clamp( cosAngle, T( -1 ), T( 1 ) );
        const T base = std::abs( ( T( 1 ) - m_alpha ) + m_alpha * c );
        if ( T( 1 ) == m_sharpness )
            return base;
        return std::pow( base, m_sharpness );
    }

    bool operator==( const CTDirectivityPattern& other ) const
    {
        return m_alpha == other.m_alpha && m_sharpness == other.m_sharpness;
    }

    bool operator!=( const CTDirectivityPattern& other ) const
    {
        return !( *this == other );
    }

    private:

    T m_alpha;
    T m_sharpness;
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTDIRECTIVITYPATTERN_H ? */
