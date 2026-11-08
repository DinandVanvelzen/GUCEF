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

#ifndef GUCEF_AUDIO_CAUDIOFRAMERING_H
#define GUCEF_AUDIO_CAUDIOFRAMERING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

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
 *  A lock-free ring of mono Float32 frames between one writer thread and one reader thread, with absolute positions:
 *  the writer's position counts the frames stored since Prepare() or Reset(), the reader's the frames taken. The render
 *  tap of the echo canceller is one: the render thread writes what the speaker will play, the capture thread reads it as
 *  the reference.
 *
 *  The writer never waits and never overwrites what the reader has not taken: what does not fit is dropped and counted
 *  (GetDroppedFrameCount()); the reader sees the count go up and knows the stream has a gap. The reader takes frames in
 *  order (Read(), Skip()); GetAvailable() is what it can take now. Positions and counts are atomics with release and
 *  acquire ordering, the frames themselves are plain memory owned by one side at a time.
 *
 *  Prepare() and Reset() are for the owner while neither side runs; everything else allocates nothing, takes no lock and
 *  fits the real time threads.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioFrameRing
{
    public:

    CAudioFrameRing( void );

    ~CAudioFrameRing();

    /** Owner: room for at least 'capacityFrames' frames (rounded up to a power of two). Allocates. False for 0 */
    bool Prepare( UInt32 capacityFrames );

    /** Owner: frees the memory */
    void Release( void );

    bool IsPrepared( void ) const;

    /** The frames the ring holds at most */
    UInt32 GetCapacity( void ) const;

    /** Owner, while neither side runs: empty, positions and counts at 0 */
    void Reset( void );

    /** Writer: stores what fits of 'count' frames and drops the rest (counted). Returns the frames stored */
    UInt32 Write( const Float32* frames, UInt32 count );

    /** Writer: stores what fits of 'count' frames of silence, the rest is dropped (counted). Returns the frames stored */
    UInt32 WriteSilence( UInt32 count );

    /** Writer: the room there is now */
    UInt32 GetFree( void ) const;

    /** Reader: the frames there are to take now */
    UInt32 GetAvailable( void ) const;

    /** Reader: takes up to 'count' frames in order. Returns the frames taken */
    UInt32 Read( Float32* destination, UInt32 count );

    /** Reader: drops up to 'count' frames. Returns the frames dropped */
    UInt32 Skip( UInt32 count );

    /** Any thread: the absolute position of the next frame written and of the next frame read */
    UInt64 GetWritePosition( void ) const;
    UInt64 GetReadPosition( void ) const;

    /** Any thread: frames the writer had to drop because the ring was full */
    UInt64 GetDroppedFrameCount( void ) const;

    private:

    CAudioFrameRing( const CAudioFrameRing& src );              /**< not implemented */
    CAudioFrameRing& operator=( const CAudioFrameRing& src );   /**< not implemented */

    private:

    Float32* m_frames;
    UInt32 m_capacity;                  /**< a power of two */
    MT::CTAtomic< UInt64 > m_write;
    MT::CTAtomic< UInt64 > m_read;
    MT::CTAtomic< UInt64 > m_dropped;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOFRAMERING_H ? */
