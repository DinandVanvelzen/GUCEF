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

#ifndef GUCEF_AUDIO_CPCMSTREAMSOURCE_H
#define GUCEF_AUDIO_CPCMSTREAMSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

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

/**
 *  A source for audio that is stored as plain samples: frames of one format, one after the other, from a place
 *  in a CIOAccess on. This is what WAV files and headerless sound data are once the header is read, so it is
 *  the building block for those codecs. Samples are converted to the byte order of this machine as they are read.
 */
class GUCEF_AUDIO_PUBLIC_CPP CPcmStreamSource : public IAudioSource
{
    public:

    /**
     *  @param input            where the samples are read from, it has to stay valid as long as the source exists
     *  @param owner            optional: a shared pointer that keeps the input alive, the source holds on to it
     *  @param format           the format of the samples as stored (the sample type is the type in the byte order given below)
     *  @param dataOffset       where in the input the first frame is
     *  @param lengthInFrames   number of frames, GUCEF_AUDIO_LENGTH_UNKNOWN to read until the input ends
     *  @param bigEndian        true when the stored multi byte samples have the most significant byte first
     */
    CPcmStreamSource( CORE::CIOAccess* input                         ,
                      const CORE::CIOAccess::IOAccessPtr& owner      ,
                      const CAudioFormat& format                     ,
                      UInt64 dataOffset                              ,
                      UInt64 lengthInFrames = GUCEF_AUDIO_LENGTH_UNKNOWN ,
                      bool bigEndian = false                         );

    virtual ~CPcmStreamSource() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSeekable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SeekToFrame( UInt64 frame ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 GetLengthInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** True when this machine stores the most significant byte first */
    static bool IsHostBigEndian( void );

    /** Reverses the bytes of every sample in the given frames, for converting between byte orders */
    static void SwapSampleBytes( void* frames, UInt32 frameCount, const CAudioFormat& format );

    private:

    CPcmStreamSource( const CPcmStreamSource& src );            /**< not implemented */
    CPcmStreamSource& operator=( const CPcmStreamSource& src ); /**< not implemented */

    private:

    CORE::CIOAccess* m_input;
    CORE::CIOAccess::IOAccessPtr m_owner;
    CAudioFormat m_format;
    UInt64 m_dataOffset;
    UInt64 m_length;
    UInt64 m_position;
    bool m_swapBytes;
    bool m_atEnd;
    bool m_canSeek;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CPCMSTREAMSOURCE_H ? */
