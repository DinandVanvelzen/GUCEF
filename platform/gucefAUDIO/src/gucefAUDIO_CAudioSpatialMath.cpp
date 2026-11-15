/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#include "gucefAUDIO_CAudioSpatialMath.h"

#ifndef GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#include "gucefMATH_SpatialAudioFunctions.h"
#define GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#endif /* GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H ? */

#ifndef GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H
#include "gucefMATH_SpatialAudioVectorFunctions.h"
#define GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H
#endif /* GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTORTHONORMALFRAME3D_H
#include "gucefMATH_CTOrthonormalFrame3D.h"
#define GUCEF_MATH_CTORTHONORMALFRAME3D_H
#endif /* GUCEF_MATH_CTORTHONORMALFRAME3D_H ? */

#ifndef GUCEF_MATH_SMOOTHINGFUNCTIONS_H
#include "gucefMATH_SmoothingFunctions.h"
#define GUCEF_MATH_SMOOTHINGFUNCTIONS_H
#endif /* GUCEF_MATH_SMOOTHINGFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTDEGREE_H
#include "gucefMATH_CTDegree.h"
#define GUCEF_MATH_CTDEGREE_H
#endif /* GUCEF_MATH_CTDEGREE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

// no trace macros in this file: all of it runs where the audio is mixed

const Float32 CAudioSpatialMath::NoCutoffInHz = 20000.0f;

/*-------------------------------------------------------------------------*/

void
CAudioSpatialMath::Locate( const TPoint3& listenerPosition, const TVector3& listenerForward, const TVector3& listenerUp,
                           const TPoint3& sourcePosition, bool sourceRelativeToListener, SFix& fix ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    const TVector3 p = sourceRelativeToListener ? TVector3( sourcePosition.x, sourcePosition.y, sourcePosition.z ) : ( sourcePosition - listenerPosition );
    fix.distance = p.GetLength();

    // the listener frame of gucefMATH: forward, right = forward x up, up' = right x forward, x right, y up, z forward. A source
    // relative to the listener is in this frame already: the default orientation, wherever the listener looks
    typedef MATH::CTOrthonormalFrame3D< Float32 > TFrame;
    TFrame frame;
    if ( sourceRelativeToListener || !TFrame::TryFromForwardUp( listenerForward, listenerUp, frame ) )
        frame = TFrame::FromForwardUp( TVector3::Forward(), TVector3::Up() );
    const TVector3 local = frame.ToFrameCoordinates( p );
    const Float32 x = local.x;
    const Float32 y = local.y;
    const Float32 z = local.z;

    const Float32 horizontal = sqrtf( x * x + z * z );
    if ( horizontal > 0.0f )
    {
        fix.azimuth = atan2f( x, z );
        fix.pan = x / horizontal;       // sin( atan2( x, z ) ), without the trigonometry
    }
    else
    {
        fix.azimuth = 0.0f;
        fix.pan = 0.0f;
    }
    fix.elevation = ( horizontal > 0.0f || y != 0.0f ) ? atan2f( y, horizontal ) : 0.0f;
}

/*-------------------------------------------------------------------------*/

void
CAudioSpatialMath::Locate2D( const TPoint3& listenerPosition, Float32 panWidth, const TPoint3& sourcePosition,
                             bool sourceRelativeToListener, SFix& fix ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    const Float32 dx = sourceRelativeToListener ? sourcePosition.x : sourcePosition.x - listenerPosition.x;
    const Float32 dy = sourceRelativeToListener ? sourcePosition.y : sourcePosition.y - listenerPosition.y;
    fix.distance = sqrtf( dx * dx + dy * dy );

    const Float32 width = panWidth > 0.0f ? panWidth : 1.0f;
    fix.pan = MATH::Clamp( dx / width, -1.0f, 1.0f );
    fix.azimuth = fix.pan * MATH::CTMathConstants< Float32 >::HalfPi();
    fix.elevation = 0.0f;
}

/*-------------------------------------------------------------------------*/

void
CAudioSpatialMath::EvaluateCone( const TVector3& emitterDirection, const TVector3& emitterToListener, Float32 innerAngleInDegrees,
                                 Float32 outerAngleInDegrees, Float32 outerGain, Float32 outerCutoffInHz, SConeResult& result ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    result.gain = 1.0f;
    result.cutoff = NoCutoffInHz;

    // an inner angle of a full turn is no cone, whatever the rounding of the angle of 180 degrees
    if ( innerAngleInDegrees >= 360.0f )
        return;

    TVector3 axis;
    TVector3 toListener;
    if ( !emitterDirection.TryNormalize( axis, 1.0e-12f ) || !emitterToListener.TryNormalize( toListener, 1.0e-12f ) )
        return;

    const Float32 factor = MATH::ConeInterpolationFactor( axis.Dot( toListener ), MATH::CTRadian< Float32 >( MATH::CTDegree< Float32 >( innerAngleInDegrees ) ),
                                                          MATH::CTRadian< Float32 >( MATH::CTDegree< Float32 >( outerAngleInDegrees ) ) );
    result.gain = MATH::Lerp( 1.0f, outerGain, factor );
    if ( outerCutoffInHz > 0.0f && outerCutoffInHz < NoCutoffInHz )
        result.cutoff = MATH::LogLerp( NoCutoffInHz, outerCutoffInHz, factor );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioSpatialMath::GetDirectOcclusionAmount( Float32 occlusion, Float32 obstruction ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    const Float32 amount = occlusion > obstruction ? occlusion : obstruction;
    return MATH::Clamp01( amount );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioSpatialMath::GetOcclusionGain( Float32 amount, Float32 occlusionGainInDecibels ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return MATH::Lerp( 1.0f, MATH::DecibelsToGain( occlusionGainInDecibels ), MATH::Clamp01( amount ) );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioSpatialMath::GetOcclusionCutoff( Float32 amount, Float32 occlusionCutoffInHz ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    if ( !( occlusionCutoffInHz > 0.0f ) || occlusionCutoffInHz >= NoCutoffInHz )
        return NoCutoffInHz;
    return MATH::LogLerp( NoCutoffInHz, occlusionCutoffInHz, MATH::Clamp01( amount ) );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioSpatialMath::GetDopplerRatio( const TPoint3& listenerPosition, const TVector3& listenerVelocity, const TPoint3& sourcePosition,
                                    const TVector3& sourceVelocity, Float32 speedOfSoundInUnits, Float32 dopplerFactor,
                                    Float32 minRatio, Float32 maxRatio ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    // the vector form of gucefMATH: the speeds along the axis from the source to the listener, then the OpenAL 1.1 ratio
    return MATH::DopplerRatio( listenerPosition, listenerVelocity, sourcePosition, sourceVelocity, speedOfSoundInUnits, dopplerFactor, minRatio, maxRatio );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioSpatialMath::SmoothDopplerRatio( Float32 previous, Float32 target, Float32 blockSeconds, Float32 smoothingInMs ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    if ( !( smoothingInMs > 0.0f ) )
        return target;
    return MATH::InterpTo( previous, target, blockSeconds, 1000.0f / smoothingInMs );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
