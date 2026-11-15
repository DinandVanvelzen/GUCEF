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

#ifndef GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H
#define GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H

/*
 *  The spatial audio functions that take positions and velocities (section 4.7.3 of the design): the vector form of the
 *  Doppler ratio. It lives in its own header so that the scalar functions of gucefMATH_SpatialAudioFunctions.h do not pull
 *  in the vector types. T is a floating point kind; the function is realtime-safe.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#include "gucefMATH_SpatialAudioFunctions.h"
#define GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#endif /* GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTVECTOR3D_H
#include "gucefMATH_CTVector3D.h"
#define GUCEF_MATH_CTVECTOR3D_H
#endif /* GUCEF_MATH_CTVECTOR3D_H ? */

#ifndef GUCEF_MATH_CTPOINT3D_H
#include "gucefMATH_CTPoint3D.h"
#define GUCEF_MATH_CTPOINT3D_H
#endif /* GUCEF_MATH_CTPOINT3D_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  OpenAL 1.1 Doppler ratio from positions and velocities: with SL = listenerPosition - sourcePosition the speeds along the
 *  axis from the source to the listener are vls = ( listenerVelocity . SL ) / |SL| and vss = ( sourceVelocity . SL ) / |SL|,
 *  and the ratio is the scalar DopplerRatio( vls, vss, speedOfSound, dopplerFactor, minRatio, maxRatio ). The ratio is 1
 *  when the listener and the source are at the same position (or the positions are not finite), when the Doppler factor
 *  or the speed of sound is not positive. Use the same length unit for positions, velocities and the speed of sound
 *  (gucefAUDIO multiplies the speed of sound by its units per metre).
 */
template < typename T >
GUCEF_MATH_PRECISE_INLINE T
DopplerRatio( const CTPoint3D< T >& listenerPosition, const CTVector3D< T >& listenerVelocity,
              const CTPoint3D< T >& sourcePosition, const CTVector3D< T >& sourceVelocity,
              const T& speedOfSound, const T& dopplerFactor, const T& minRatio, const T& maxRatio ) GUCEF_MATH_REALTIME_CONTEXT
{
    CTVector3D< T > axis;
    if ( !( listenerPosition - sourcePosition ).TryNormalize( axis, CTNumericTraits< T >::Zero() ) )
        return CTNumericTraits< T >::One();
    return DopplerRatio( listenerVelocity.Dot( axis ), sourceVelocity.Dot( axis ), speedOfSound, dopplerFactor, minRatio, maxRatio );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* For Float32 and Float64 the module's instantiations (gucefMATH_SphericalInstantiations.cpp) are used, section 5.10 */

#define GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_KIND( prefix, kind ) \
    prefix kind DopplerRatio< kind >( const CTPoint3D< kind >&, const CTVector3D< kind >&, const CTPoint3D< kind >&, const CTVector3D< kind >&, const kind&, const kind&, const kind&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_KIND( prefix, Float32 ) \
    GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_KIND( prefix, Float64 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H ? */
