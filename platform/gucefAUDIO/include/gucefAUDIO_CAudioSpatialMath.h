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

#ifndef GUCEF_AUDIO_CAUDIOSPATIALMATH_H
#define GUCEF_AUDIO_CAUDIOSPATIALMATH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The geometry of a spatial voice as pure functions of plain values: where the sound is heard from, the cone, the
 *  occlusion mapping and the Doppler ratio. These are the steps 2 to 4 and 7 of the pipeline of a spatial voice (design
 *  5.5): the vector work is done with the gucefMATH vector types and the scalar formulas are those of gucefMATH
 *  (cone interpolation, Doppler ratio, gain conversion, interpolation), so nothing is calculated twice. The distance gain
 *  is CAudioAttenuation, the speaker gains are CAudioPanner.
 *
 *  Coordinates: right handed, +Y up, -Z forward, in world units. All functions are realtime safe: no allocation, no locks,
 *  nothing but arithmetic and a few trigonometric calls.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioSpatialMath
{
    public:

    typedef MATH::TFloat32Vector3D TVector3;
    typedef MATH::TFloat32Point3D TPoint3;

    /** The cutoff in Hz that means "no low pass": the top of the audible range */
    static const Float32 NoCutoffInHz;

    /** Where a sound is heard from, relative to the listener */
    struct SFix
    {
        Float32 distance;       /**< distance in world units (2D mode: in the XY plane) */
        Float32 azimuth;        /**< radians, 0 straight ahead, positive to the right, -pi .. pi (2D mode: pan times a quarter turn) */
        Float32 elevation;      /**< radians, positive up, -pi/2 .. pi/2 (2D mode: 0) */
        Float32 pan;            /**< -1 .. 1 left to right: sin( azimuth ) in 3D, the pan of the 2D mode (front and back fold onto the same position) */

        SFix( void ) : distance( 0.0f ), azimuth( 0.0f ), elevation( 0.0f ), pan( 0.0f ) {}
    };

    /**
     *  Locates a source for a listener in 3D mode (design 5.5, steps 2 and 3): with p the vector from the listener to the
     *  source (the position of the source itself when 'sourceRelativeToListener' is set), the listener frame is
     *  r = normalise( forward x up ), u = r x forward, f = normalise( forward ); x = p.r, y = p.u, z = p.f; the azimuth is atan2( x, z ), the
     *  elevation atan2( y, sqrt( x x + z z ) ). A forward that is zero or parallel to up falls back to the default
     *  orientation (forward -Z, up +Y); a source at the position of the listener has azimuth, elevation and pan 0.
     *  A source that is relative to the listener (attached to it: the sounds of an interface, of the first person) is given in the frame of
     *  the listener itself, x to the right, y up, -z ahead, so it does not depend on the position or the orientation of the listener
     */
    static void Locate( const TPoint3& listenerPosition, const TVector3& listenerForward, const TVector3& listenerUp,
                        const TPoint3& sourcePosition, bool sourceRelativeToListener, SFix& fix ) GUCEF_AUDIO_REALTIME_CONTEXT;

    /**
     *  The 2D mode of the screen space games (design 5.7): the distance is measured in the XY plane, the pan is
     *  clamp( ( xS - xL ) / panWidth, -1, 1 ) (a pan width that is not positive counts as 1) and the azimuth is the pan times a quarter turn. The
     *  orientation of the listener is not used
     */
    static void Locate2D( const TPoint3& listenerPosition, Float32 panWidth, const TPoint3& sourcePosition,
                          bool sourceRelativeToListener, SFix& fix ) GUCEF_AUDIO_REALTIME_CONTEXT;

    /** The result of the cone: the gain and the cutoff of the low pass that goes with it */
    struct SConeResult
    {
        Float32 gain;           /**< 1 inside the inner cone, the outer gain outside the outer cone, linear in the angle between */
        Float32 cutoff;         /**< in Hz, interpolated logarithmically in the same way, NoCutoffInHz when the outer cutoff is off */

        SConeResult( void ) : gain( 1.0f ), cutoff( 20000.0f ) {}
    };

    /**
     *  The cone of an emitter (design 5.6.2): 'emitterDirection' is the axis of the cone (a zero vector is an omnidirectional
     *  emitter: gain 1, no cutoff), 'emitterToListener' the vector from the emitter to the listener (a listener at the position
     *  of the emitter hears gain 1). The angles are the full opening angles in degrees, an inner angle of 360 is no cone. 'outerGain' is
     *  linear, 'outerCutoffInHz' 0 or less is off
     */
    static void EvaluateCone( const TVector3& emitterDirection, const TVector3& emitterToListener, Float32 innerAngleInDegrees,
                              Float32 outerAngleInDegrees, Float32 outerGain, Float32 outerCutoffInHz, SConeResult& result ) GUCEF_AUDIO_REALTIME_CONTEXT;

    /**
     *  The amount of occlusion of the direct path, max( occlusion, obstruction ), each 0 .. 1 (design 5.6.5). The sends of a voice
     *  only use the occlusion
     */
    static Float32 GetDirectOcclusionAmount( Float32 occlusion, Float32 obstruction ) GUCEF_AUDIO_REALTIME_CONTEXT;

    /** lerp( 1, 10 ^ ( occlusionGainInDecibels / 20 ), amount ), the gain of a path with that amount of occlusion */
    static Float32 GetOcclusionGain( Float32 amount, Float32 occlusionGainInDecibels ) GUCEF_AUDIO_REALTIME_CONTEXT;

    /** exp( lerp( ln 20000, ln occlusionCutoff, amount ) ), the low pass cutoff in Hz of a path with that amount of occlusion; NoCutoffInHz when the cutoff is off (0 or at least 20000) */
    static Float32 GetOcclusionCutoff( Float32 amount, Float32 occlusionCutoffInHz ) GUCEF_AUDIO_REALTIME_CONTEXT;

    /**
     *  The Doppler pitch ratio of OpenAL 1.1 (design 5.6.6) from positions and velocities in world units: the speeds along the axis
     *  from the source to the listener are derived here, the formula is MATH::DopplerRatio(). 1 when the listener and the source are at the same place, when the
     *  factor is 0 or the speed of sound not positive. For a listener relative emitter pass the origin and a zero velocity for the listener
     */
    static Float32 GetDopplerRatio( const TPoint3& listenerPosition, const TVector3& listenerVelocity, const TPoint3& sourcePosition,
                                    const TVector3& sourceVelocity, Float32 speedOfSoundInUnits, Float32 dopplerFactor,
                                    Float32 minRatio, Float32 maxRatio ) GUCEF_AUDIO_REALTIME_CONTEXT;

    /**
     *  One step of the smoothing of the Doppler ratio, a one pole filter that is the same for any block size: the part
     *  1 - exp( -blockSeconds / time constant ) of the way from 'previous' to 'target'. A time constant of 0 gives 'target'
     */
    static Float32 SmoothDopplerRatio( Float32 previous, Float32 target, Float32 blockSeconds, Float32 smoothingInMs ) GUCEF_AUDIO_REALTIME_CONTEXT;

    private:

    CAudioSpatialMath( void );      /**< not implemented: only static functions */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOSPATIALMATH_H ? */
