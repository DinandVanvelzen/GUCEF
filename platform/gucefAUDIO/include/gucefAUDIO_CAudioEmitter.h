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

#ifndef GUCEF_AUDIO_CAUDIOEMITTER_H
#define GUCEF_AUDIO_CAUDIOEMITTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTSLOTDATAEXCHANGE_H
#include "gucefMT_CTSlotDataExchange.h"
#define GUCEF_MT_CTSLOTDATAEXCHANGE_H
#endif /* GUCEF_MT_CTSLOTDATAEXCHANGE_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_AUDIO_CAUDIOSPATIALMATH_H
#include "gucefAUDIO_CAudioSpatialMath.h"
#define GUCEF_AUDIO_CAUDIOSPATIALMATH_H
#endif /* GUCEF_AUDIO_CAUDIOSPATIALMATH_H ? */

#ifndef GUCEF_AUDIO_CAUDIOATTENUATION_H
#include "gucefAUDIO_CAudioAttenuation.h"
#define GUCEF_AUDIO_CAUDIOATTENUATION_H
#endif /* GUCEF_AUDIO_CAUDIOATTENUATION_H ? */

#ifndef GUCEF_AUDIO_CAUDIOSPATIALPARAMS_H
#include "gucefAUDIO_CAudioSpatialParams.h"
#define GUCEF_AUDIO_CAUDIOSPATIALPARAMS_H
#endif /* GUCEF_AUDIO_CAUDIOSPATIALPARAMS_H ? */

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

/** How an emitter is placed in the world */
enum EAudioSpatialMode
{
    AUDIOSPATIAL_2D = 0 ,       /**< screen space: the distance is measured in the XY plane, the pan comes from the horizontal offset (design 5.7) */
    AUDIOSPATIAL_3D             /**< the sound has a place in the three dimensional world of the listener */
};
typedef enum EAudioSpatialMode TAudioSpatialMode;

/*-------------------------------------------------------------------------*/

/**
 *  Everything the audio thread needs to know about an emitter, from one update: a plain value, copied through a
 *  MT::CTSlotDataExchange. All properties of 5.4 of the design; the angles are in degrees, gains linear, cutoffs in Hz,
 *  distances and velocities in world units.
 */
struct GUCEF_AUDIO_PUBLIC_CPP SEmitterState
{
    TAudioSpatialMode mode;                         /**< default 3D */
    CAudioSpatialMath::TPoint3 position;
    CAudioSpatialMath::TVector3 velocity;
    CAudioSpatialMath::TVector3 direction;          /**< the axis of the cone, a unit vector; the zero vector is an omnidirectional emitter */
    bool listenerRelative;                          /**< the position is relative to the listener: attached sounds of the interface or of the first person */
    bool useWorldAttenuation;                       /**< true (default): the attenuation of the world settings; false: the one of this emitter */
    CAudioAttenuation::TEvaluator attenuation;      /**< the attenuation of this emitter, used when 'useWorldAttenuation' is false */
    Float32 coneInnerAngle;                         /**< degrees, 0 .. 360, default 360 */
    Float32 coneOuterAngle;                         /**< degrees, from the inner angle to 360, default 360 */
    Float32 coneOuterGain;                          /**< linear, 0 .. 1, default 0 */
    Float32 coneOuterCutoff;                        /**< Hz, 0 for none (default) */
    Float32 spreadRadius;                           /**< distance below which the sound widens to all speakers; 0 (default) is the minimum distance of the attenuation */
    Float32 distanceCutoff;                         /**< Hz of the low pass at the maximum distance, 0 for none (default) */
    Float32 occlusion;                              /**< 0 .. 1, affects the direct path and the sends */
    Float32 obstruction;                            /**< 0 .. 1, affects the direct path only */
    Float32 occlusionGainInDecibels;                /**< gain at an occlusion of 1, default -12 */
    Float32 occlusionCutoff;                        /**< Hz at an occlusion of 1, default 1500 */
    Float32 dopplerFactor;                          /**< multiplies the Doppler factor of the world, default 1 */
    bool sendsFollowAttenuation;                    /**< true (default): the voice sends follow the distance attenuation */
    bool deriveVelocity;                            /**< derive the velocity from the successive positions instead of using 'velocity' (default false, see CAudioEmitter::SetDeriveVelocity()) */

    SEmitterState( void );

    /** The attenuation in effect: that of the emitter or, when it uses the world's, the one of the world settings. Realtime safe */
    const CAudioAttenuation::TEvaluator& GetEffectiveAttenuation( const SSpatialWorldState& world ) const GUCEF_AUDIO_REALTIME_CONTEXT;

    /** The spread radius in effect: the one set, or the minimum distance of the attenuation in effect. Realtime safe */
    Float32 GetEffectiveSpreadRadius( const SSpatialWorldState& world ) const GUCEF_AUDIO_REALTIME_CONTEXT;

    bool operator==( const SEmitterState& other ) const;
    bool operator!=( const SEmitterState& other ) const;
};

/*-------------------------------------------------------------------------*/

/**
 *  A place in the world that sounds come from (design 5.4): shared by any number of voices (an engine and a horn on one
 *  vehicle). The application sets properties from any thread; every setter publishes the complete state as one snapshot
 *  through a triple buffer (MT::CTSlotDataExchange, latest only, one reader), so the audio thread, which takes the newest
 *  snapshot once per block (ReadState()), never sees a position from one update with a velocity from another. Writers are serialised by a
 *  mutex that the audio thread never takes.
 *
 *  Values that are not valid (not finite, out of range where there is no sensible limit) are refused: the setter returns false
 *  and nothing changes. Values with a sensible limit are limited to it.
 *
 *  Not copyable: the object is the owner of the buffer; share it with TAudioEmitterPtr.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioEmitter
{
    public:

    typedef CORE::CTSharedPtr< CAudioEmitter, MT::CMutex > TAudioEmitterPtr;
    typedef CAudioSpatialMath::TPoint3 TPoint3;
    typedef CAudioSpatialMath::TVector3 TVector3;

    CAudioEmitter( void );

    ~CAudioEmitter();

    bool SetMode( TAudioSpatialMode mode );

    bool SetPosition( const TPoint3& position );

    bool SetVelocity( const TVector3& velocity );

    /** Position and velocity in one update */
    bool SetPositionAndVelocity( const TPoint3& position, const TVector3& velocity );

    /** The axis of the cone: it is normalised when stored, the zero vector makes the emitter omnidirectional */
    bool SetDirection( const TVector3& direction );

    /** Position, velocity and cone axis in one update: what an emitter that follows a node of a transform tree needs (see gucefAUDIO_SpatialSnapshotAdapters.h) */
    bool SetPlacement( const TPoint3& position, const TVector3& velocity, const TVector3& direction );

    bool SetListenerRelative( bool relative );

    /** An attenuation of its own for this emitter */
    bool SetAttenuation( const CAudioAttenuation& attenuation );

    /** Back to the attenuation of the world settings (the default) */
    bool UseWorldAttenuation( void );

    /**
     *  The cone: the angles are the full opening angles in degrees (inner 0 .. 360, outer from the inner angle up to 360, a value outside is limited),
     *  the outer gain is linear 0 .. 1, the outer cutoff in Hz, 0 for none. The default, 360 and 360, is no cone
     */
    bool SetCone( Float32 innerAngleInDegrees, Float32 outerAngleInDegrees, Float32 outerGain, Float32 outerCutoffInHz );

    /** The distance below which the sound widens toward all speakers; 0 for the minimum distance of the attenuation in effect */
    bool SetSpreadRadius( Float32 radius );

    /** The cutoff of the low pass at the maximum distance in Hz, 20 .. 20000; 0 for none */
    bool SetDistanceLowPass( Float32 cutoffInHz );

    /** 0 .. 1, set by the game from its own geometry queries: affects the direct path and the sends */
    bool SetOcclusion( Float32 occlusion );

    /** 0 .. 1: affects the direct path only */
    bool SetObstruction( Float32 obstruction );

    /** Occlusion and obstruction in one update, as a pair of estimates from the same capture of the geometry */
    bool SetOcclusionAndObstruction( Float32 occlusion, Float32 obstruction );

    /** What an occlusion of 1 does: the gain in dB, -96 .. 0 (default -12), and the cutoff in Hz, 20 .. 20000 (default 1500), 0 for no low pass */
    bool SetOcclusionResponse( Float32 gainInDecibels, Float32 cutoffInHz );

    /** Multiplies the Doppler factor of the world, 0 .. 10 (default 1) */
    bool SetDopplerFactor( Float32 factor );

    bool SetSendsFollowAttenuation( bool follow );

    /**
     *  On: the velocity is derived from successive positions and the time between their updates (a precision clock read
     *  when the position is set), instead of being set by the game; physics velocities are preferred where the game has them.
     *  Every update of the position (SetPosition(), SetPositionAndVelocity(), SetState()) at least 1 ms after the one the
     *  last velocity was derived from gives velocity = distance moved / time passed; closer updates move the position and
     *  keep the velocity. Turning it on keeps the velocity it had until the first such update. A game that uses it sets the
     *  position every frame, also while the emitter stands still, so the velocity goes back to 0. A velocity that is given
     *  (SetVelocity(), the velocity of SetPositionAndVelocity() and SetState()) is replaced by the derived one while it is on.
     *  Off leaves the velocity as it was last derived
     */
    bool SetDeriveVelocity( bool derive );

    /** The whole state in one update; refused as a whole when anything in it is not valid, the values with a limit are limited */
    bool SetState( const SEmitterState& state );

    /** The state as it was last set, from any thread */
    void GetState( SEmitterState& state ) const;

    /**
     *  Audio thread (the one reader): copies the newest snapshot into 'state'. True when it was updated since the previous call,
     *  false when it is the same as the one before. Never waits, never allocates
     */
    bool ReadState( SEmitterState& state ) GUCEF_AUDIO_REALTIME_CONTEXT;

    private:

    CAudioEmitter( const CAudioEmitter& src );                /**< not implemented */
    CAudioEmitter& operator=( const CAudioEmitter& src );     /**< not implemented */

    typedef MT::CTSlotDataExchange< SEmitterState, MT::SSlotDeliveryLatestOnly, MT::SSlotFullFail, 1 > TExchange;

    /** Limits and checks a state; false when something is not valid */
    static bool Sanitise( SEmitterState& state );

    void Publish( void );

    /** Under the writer lock: the derived velocity of a new state, when the state asks for it (SetDeriveVelocity()) */
    void DeriveVelocity( SEmitterState& candidate );

    private:

    mutable MT::CMutex m_writerLock;
    SEmitterState m_state;              /**< owner side picture, under the writer lock */
    TPoint3 m_derivedFrom;              /**< the position the last derived velocity was measured from, under the writer lock */
    UInt64 m_derivedFromTicks;          /**< precision clock of that position */
    bool m_derivedFromKnown;
    TExchange m_exchange;
};

typedef CAudioEmitter::TAudioEmitterPtr TAudioEmitterPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOEMITTER_H ? */
