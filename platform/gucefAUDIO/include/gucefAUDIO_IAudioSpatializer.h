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

#ifndef GUCEF_AUDIO_IAUDIOSPATIALIZER_H
#define GUCEF_AUDIO_IAUDIOSPATIALIZER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_AUDIO_CAUDIOFORMAT_H
#include "gucefAUDIO_CAudioFormat.h"
#define GUCEF_AUDIO_CAUDIOFORMAT_H
#endif /* GUCEF_AUDIO_CAUDIOFORMAT_H ? */

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

class CAudioSpatializerFactory;

/**
 *  What a spatializer needs to know about a spatial voice for one block (design 5.5 and 5.9): where the sound is heard
 *  from and how loud it is. Made by the voice on the audio thread once per block from the listener, emitter and world
 *  snapshots. A plain value.
 *
 *  The values of the previous block are not part of the frame: a spatializer keeps what it needs to interpolate from one
 *  block to the next itself (the panning spatializer its speaker gains), in its own domain, and forgets it in Reset().
 */
struct GUCEF_AUDIO_PUBLIC_CPP SSpatialFrame
{
    Float32 azimuth;            /**< radians, 0 straight ahead, positive to the right, -pi .. pi */
    Float32 elevation;          /**< radians, positive up (spatializers that render height; the panning spatializer does not) */
    Float32 distance;           /**< world units from the listener */
    Float32 pan;                /**< -1 .. 1, what the stereo law uses: sin( azimuth ) in 3D, the pan of the 2D mode */
    Float32 spreadFactor;       /**< 0 .. 1: 1 is a point source, below 1 the sound widens toward all speakers, 0 is even on all of them */
    Float32 gain;               /**< linear gain on every speaker: the voice volume, times the gain of the direct path when the voice gives it to the spatializer */
    Float32 rearGain;           /**< linear gain of a source straight behind a stereo listener (front and back cue), 1 for none */
    Float32 lfeGain;            /**< linear level at which the low frequency channel gets the sound (before 'gain'), 0 for none */
    bool useCenterSpeaker;      /**< the centre speakers of a surround layout take part in the panning */

    SSpatialFrame( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  Renders the mono working signal of a spatial voice to the speakers of the mix (design 5.9). One per spatial voice: it
 *  may keep state from block to block (gains to interpolate from, filter or convolution state). CPanningSpatializer is the
 *  built-in implementation; others (HRTF for headphones) are registered in CAudioSpatializerRegistry and chosen per engine
 *  with CAudioSpatialParams::SetSpatializerName().
 *
 *  Threads: Prepare() on the owner thread (it may allocate), before the voice is mixed; Reset() and Render() on the audio
 *  thread only, realtime safe: no allocation, no locks, no logging, no GUCEF_TRACE.
 *
 *  A spatializer made by a factory is counted by it (CAudioSpatializerFactory::GetLiveInstanceCount()), so a plugin is not
 *  unloaded while its code is in use.
 */
class GUCEF_AUDIO_PUBLIC_CPP IAudioSpatializer
{
    public:

    typedef CORE::CTSharedPtr< IAudioSpatializer, MT::CMutex > TAudioSpatializerPtr;

    IAudioSpatializer( void );

    virtual ~IAudioSpatializer();

    /** The type name, the key of its factory in the registry */
    virtual const CString& GetSpatializerTypeName( void ) const = 0;

    /** Owner thread: sets up for the speakers of the mix and the largest block, and forgets the previous blocks. May allocate. False when the format is not supported */
    virtual bool Prepare( const CAudioFormat& mixFormat, UInt32 maxBlockFrames ) = 0;

    /** Audio thread: forgets the previous blocks, so the next Render() starts without interpolation from earlier values */
    virtual void Reset( void ) GUCEF_AUDIO_REALTIME_CONTEXT = 0;

    /**
     *  Audio thread: adds 'frames' frames of the mono signal, placed as 'frame' says, to 'output' (interleaved, the channels of
     *  the mix format). Changes from the previous block are interpolated over the block so they do not click; the first block
     *  after Prepare() or Reset() starts at the values of its frame. 'frames' is at most the largest block of Prepare()
     */
    virtual void Render( const Float32* mono, UInt32 frames, const SSpatialFrame& frame, Float32* output ) GUCEF_AUDIO_REALTIME_CONTEXT = 0;

    /** The factory that made it, null when it was made directly */
    CAudioSpatializerFactory* GetFactory( void ) const;

    private:
    friend class CAudioSpatializerFactory;

    IAudioSpatializer( const IAudioSpatializer& src );              /**< not implemented */
    IAudioSpatializer& operator=( const IAudioSpatializer& src );   /**< not implemented */

    private:

    CAudioSpatializerFactory* m_factory;
};

/*-------------------------------------------------------------------------*/

typedef IAudioSpatializer::TAudioSpatializerPtr TAudioSpatializerPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_IAUDIOSPATIALIZER_H ? */
