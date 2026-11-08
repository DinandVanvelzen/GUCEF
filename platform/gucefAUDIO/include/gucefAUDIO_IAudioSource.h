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

#ifndef GUCEF_AUDIO_IAUDIOSOURCE_H
#define GUCEF_AUDIO_IAUDIOSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

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
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Returned as length of a source that does not know how long it is (a live stream) */
#define GUCEF_AUDIO_LENGTH_UNKNOWN  GUCEF_UINT64MAX

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
 *  Where audio comes from: something that hands out frames of one fixed format when asked, one source per
 *  playing sound. Decoded files, clips in memory, generated tones and microphones all look the same to the engine.
 *
 *  Pull model: the consumer asks for as many frames as it has room for. A source does not have to be fast
 *  (it may decode while it is asked) but it is never asked from more than one thread at a time, and the engine
 *  makes sure it is not asked from the real time audio thread when it is slow (see CAudioStream).
 */
class GUCEF_AUDIO_PUBLIC_CPP IAudioSource
{
    public:

    typedef CORE::CTSharedPtr< IAudioSource, MT::CMutex > TAudioSourcePtr;

    virtual ~IAudioSource();

    /** The format of the frames that ReadFrames() produces. Does not change during the life of the source */
    virtual const CAudioFormat& GetFormat( void ) const = 0;

    /**
     *  Fills 'destination' with up to 'frameCapacity' frames in GetFormat() and returns how many it made.
     *  Fewer than asked means the source is at its end or has nothing more right now (a live source):
     *  check IsEndOfStream() to tell them apart.
     */
    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) = 0;

    /** True when there will never be more frames, unless the source is rewound or sought */
    virtual bool IsEndOfStream( void ) const = 0;

    /**
     *  True when ReadFrames() never allocates, locks, waits or logs, so that it may be called from the real time
     *  audio thread. False by default: a source must opt in. A voice on a source that is not real time safe
     *  causes a warning when it is created (CAudioEngine::CreateVoice), since it can make the audio glitch.
     */
    virtual bool IsRealtimeSafe( void ) const;

    /** True when SeekToFrame() works */
    virtual bool IsSeekable( void ) const;

    /** Moves to the given frame. Fails (and stays where it was) for a source that is not seekable or a frame beyond the end */
    virtual bool SeekToFrame( UInt64 frame );

    /**
     *  Adds the tags of the source (loop points, title and so on) to 'metaData'. Returns false, and adds nothing, when
     *  the source has no tags. False by default. Meant for the owner of the source, the side that opens and decodes
     *  it: never call this from the real time audio thread, it may lock and allocate.
     */
    virtual bool GetMetaData( CORE::CValueList& metaData ) const;

    /** Number of frames read so far from the start of the audio */
    virtual UInt64 GetPositionInFrames( void ) const = 0;

    /** Total frames in the audio, GUCEF_AUDIO_LENGTH_UNKNOWN when the source cannot tell */
    virtual UInt64 GetLengthInFrames( void ) const;

    /** Goes back to the start. Same as seeking to frame 0 */
    bool Rewind( void );
};

typedef IAudioSource::TAudioSourcePtr TAudioSourcePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_IAUDIOSOURCE_H ? */
