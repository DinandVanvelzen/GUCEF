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

#ifndef GUCEF_AUDIO_CAUDIOSTREAM_H
#define GUCEF_AUDIO_CAUDIOSTREAM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_CTSPSCQUEUE_H
#include "gucefMT_CTSpscQueue.h"
#define GUCEF_MT_CTSPSCQUEUE_H
#endif /* GUCEF_MT_CTSPSCQUEUE_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_AUDIO_IAUDIOSOURCE_H
#include "gucefAUDIO_IAudioSource.h"
#define GUCEF_AUDIO_IAUDIOSOURCE_H
#endif /* GUCEF_AUDIO_IAUDIOSOURCE_H ? */

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

class CAudioGlobal;

/**
 *  Plays a source that should not be read where the audio is mixed: music and other long audio that is decoded
 *  from a file while it plays. The stream reads ahead of the playback on a task of the thread pool
 *  StreamingThreadPoolName (one task fills all streams) and keeps what it read in blocks, so the voice that plays
 *  the stream only copies memory: no IO, no decoding, no locks and no allocations on the render path.
 *
 *  The source belongs to the stream from then on, nothing else may read it.
 *
 *  Looping: a stream can loop by itself (loopCount), which is seamless because the read ahead simply continues at
 *  the start. Looping the voice instead also works, the seek it does then needs the read ahead to catch up first,
 *  which is a short gap.
 *
 *  When the read ahead falls behind the stream gives fewer frames than asked without being at its end, which the
 *  voice plays as silence, and counts an underrun.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioStream : public IAudioSource
{
    public:

    typedef CORE::CTSharedPtr< CAudioStream, MT::CMutex > TAudioStreamPtr;

    /** The pool the read ahead task runs on */
    static const CString StreamingThreadPoolName;

    static const UInt32 DefaultBufferDurationInMs = 500;

    /**
     *  @param source               what to play, the stream owns it from now on
     *  @param bufferDurationInMs   how far to read ahead
     *  @param loopCount            how often to play the source, 0 is forever (needs a seekable source)
     *  @param readAheadOnTask      false: nothing reads ahead by itself, Fill() has to be called (for platforms
     *                              without threads, and for tests that want to control every step)
     *
     *  The first part of the source is read before the constructor returns, so playback can start right away.
     */
    CAudioStream( const TAudioSourcePtr& source                           ,
                  UInt32 bufferDurationInMs = DefaultBufferDurationInMs   ,
                  UInt32 loopCount = 1                                    ,
                  bool readAheadOnTask = true                             );

    /** Stops the read ahead, waiting for it when it is busy with this stream */
    virtual ~CAudioStream() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Render side: copies what was read ahead, never waits for it */
    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsRealtimeSafe( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSeekable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The tags of the wrapped source. The source is only touched under the fill lock, like the read ahead and
     *  its seeks do, so this waits for a read ahead turn that is busy: owner side only, never the audio thread.
     */
    virtual bool GetMetaData( CORE::CValueList& metaData ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Render side: what was read ahead is dropped and the read ahead continues from the new place on its next turn */
    virtual bool SeekToFrame( UInt64 frame ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The length of the source, unknown when the stream loops forever */
    virtual UInt64 GetLengthInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Reads ahead as far as the buffer allows, on the calling thread. The read ahead task calls this, and code
     *  that made the stream without that task calls it instead. Safe to call from any thread except the render path.
     */
    void Fill( void );

    /** Times ReadFrames() had less than asked while the stream was not at its end */
    UInt64 GetUnderrunCount( void ) const;

    /** Frames that are read ahead and not played yet, an estimate while the read ahead is busy */
    UInt32 GetBufferedFrames( void ) const;

    private:

    friend class CAudioGlobal;

    /** Ends the read ahead task, at shutdown of the module */
    static void StopReadAheadTask( void );

    struct SBlock
    {
        UInt32 generation;          /**< the seek it belongs to, blocks of earlier seeks are dropped */
        UInt32 frameCount;
        bool isLast;                /**< the source ended in this block */
        GUCEF::vector< UInt8 > data;

        SBlock( void ) : generation( 0 ), frameCount( 0 ), isLast( false ), data() {}
    };

    static const UInt32 NoBlock = 0xFFFFFFFFU;

    UInt32 FillBlock( SBlock& block, bool& ended );

    CAudioStream( const CAudioStream& src );            /**< not implemented */
    CAudioStream& operator=( const CAudioStream& src ); /**< not implemented */

    private:

    TAudioSourcePtr m_source;
    CAudioFormat m_format;
    UInt32 m_bytesPerFrame;
    UInt32 m_blockFrames;
    UInt64 m_sourceLength;
    UInt32 m_loopCount;
    bool m_seekable;
    bool m_readAheadOnTask;
    GUCEF::vector< SBlock > m_blocks;
    MT::CTSpscQueue< UInt32 >* m_filledBlocks;      /**< read ahead to render side */
    MT::CTSpscQueue< UInt32 >* m_freeBlocks;        /**< render side back to read ahead */

    // published by the render side for the read ahead
    MT::CTAtomic< UInt32 > m_requestedGeneration;
    MT::CTAtomic< UInt64 > m_seekTarget;
    MT::CTAtomic< UInt64 > m_underruns;
    MT::CTAtomic< UInt32 > m_bufferedFrames;

    // render side only
    UInt32 m_generation;
    UInt32 m_currentBlock;
    UInt32 m_currentOffset;
    UInt64 m_position;
    bool m_atEnd;

    // read ahead side only, under m_fillLock
    mutable MT::CMutex m_fillLock;          /**< every access to the source is under this lock, also the const ones that ask for tags */
    UInt32 m_fillGeneration;
    UInt32 m_spareBlock;
    UInt32 m_playsDone;
    UInt64 m_framesThisPlay;
    bool m_fillAtEnd;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOSTREAM_H ? */
