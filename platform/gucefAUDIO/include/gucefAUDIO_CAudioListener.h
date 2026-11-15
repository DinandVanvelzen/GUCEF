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

#ifndef GUCEF_AUDIO_CAUDIOLISTENER_H
#define GUCEF_AUDIO_CAUDIOLISTENER_H

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

#ifndef GUCEF_AUDIO_CAUDIOSPATIALMATH_H
#include "gucefAUDIO_CAudioSpatialMath.h"
#define GUCEF_AUDIO_CAUDIOSPATIALMATH_H
#endif /* GUCEF_AUDIO_CAUDIOSPATIALMATH_H ? */

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
 *  Everything the audio thread needs to know about the listener, from one update: a plain value, copied through a
 *  MT::CTSlotDataExchange. Position, orientation and velocity in one snapshot are always from the same update.
 *  Right handed, +Y up, -Z forward, in world units.
 */
struct GUCEF_AUDIO_PUBLIC_CPP SListenerState
{
    CAudioSpatialMath::TPoint3 position;
    CAudioSpatialMath::TVector3 forward;        /**< unit vector, default -Z */
    CAudioSpatialMath::TVector3 up;             /**< unit vector, default +Y */
    CAudioSpatialMath::TVector3 velocity;       /**< world units per second */
    Float32 panWidth2D;                         /**< 2D mode: the horizontal distance, in world units, from the listener to full left or right */

    SListenerState( void );

    bool operator==( const SListenerState& other ) const;
    bool operator!=( const SListenerState& other ) const;
};

/*-------------------------------------------------------------------------*/

/**
 *  The listener of the engine: where and in which direction the spatial sounds are heard from (design 5.3). The
 *  application sets the state from any thread, the audio thread takes the newest complete snapshot once per block
 *  (ReadState()) through a triple buffer (MT::CTSlotDataExchange, latest only, one reader), without waiting. Writers are
 *  serialised by a mutex that the audio thread never takes. One listener per engine; the spatial functions take the
 *  listener as an argument so several can follow.
 *
 *  The orientation is given as a forward and an up vector (OpenAL style). A vector that is not finite, or a
 *  forward that is zero or parallel to up, is refused: the setter returns false and nothing changes. The vectors
 *  are normalised when they are stored. Positions and velocities that are not finite are refused as well.
 *
 *  Not copyable: the object is the owner of the buffer.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioListener
{
    public:

    typedef CAudioSpatialMath::TPoint3 TPoint3;
    typedef CAudioSpatialMath::TVector3 TVector3;

    /** The pan width of the 2D mode when none is set: 10 world units */
    static const Float32 DefaultPanWidth2D;

    CAudioListener( void );

    ~CAudioListener();

    /** Position, forward and up in one update */
    bool SetPose( const TPoint3& position, const TVector3& forward, const TVector3& up );

    bool SetPosition( const TPoint3& position );

    bool SetOrientation( const TVector3& forward, const TVector3& up );

    bool SetVelocity( const TVector3& velocity );

    /** Position and velocity in one update */
    bool SetPositionAndVelocity( const TPoint3& position, const TVector3& velocity );

    /** The distance to full left or right of the 2D mode, in world units, more than 0 */
    bool Set2DPanWidth( Float32 units );

    /** The whole state in one update; refused as a whole when anything in it is not valid */
    bool SetState( const SListenerState& state );

    /** The state as it was last set, from any thread */
    void GetState( SListenerState& state ) const;

    /**
     *  Audio thread (the one reader): copies the newest snapshot into 'state'. True when it was updated since the previous call,
     *  false when it is the same as the one before. Never waits, never allocates
     */
    bool ReadState( SListenerState& state ) GUCEF_AUDIO_REALTIME_CONTEXT;

    private:

    CAudioListener( const CAudioListener& src );                /**< not implemented */
    CAudioListener& operator=( const CAudioListener& src );     /**< not implemented */

    typedef MT::CTSlotDataExchange< SListenerState, MT::SSlotDeliveryLatestOnly, MT::SSlotFullFail, 1 > TExchange;

    private:

    mutable MT::CMutex m_writerLock;
    SListenerState m_state;             /**< owner side picture, under the writer lock */
    TExchange m_exchange;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOLISTENER_H ? */
