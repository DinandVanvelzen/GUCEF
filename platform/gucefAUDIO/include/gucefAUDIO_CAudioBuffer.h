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

#ifndef GUCEF_AUDIO_CAUDIOBUFFER_H
#define GUCEF_AUDIO_CAUDIOBUFFER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

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

/**
 *  A block of interleaved audio frames with the format that describes them. Like CPixelMap in gucefIMAGE the
 *  sample type is a property of the buffer at run time and not of its C++ type, so one class holds 16 bit
 *  recordings and float mix buffers alike, and ConvertFormatTo() turns one into the other.
 *  New buffers are filled with silence.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioBuffer
{
    public:

    typedef CORE::CTSharedPtr< CAudioBuffer, MT::CMutex > TAudioBufferPtr;

    CAudioBuffer( void );

    /** A buffer of 'frameCount' frames of silence */
    explicit CAudioBuffer( const CAudioFormat& format, UInt32 frameCount = 0 );

    /** A buffer with a copy of the given frames, which must be in the given format */
    CAudioBuffer( const CAudioFormat& format, const void* frames, UInt32 frameCount );

    CAudioBuffer( const CAudioBuffer& src );

    CAudioBuffer& operator=( const CAudioBuffer& src );

    ~CAudioBuffer();

    const CAudioFormat& GetFormat( void ) const;

    UInt32 GetFrameCount( void ) const;

    UInt32 GetSizeInBytes( void ) const;

    bool IsEmpty( void ) const;

    /** Length of the audio in seconds */
    Float64 GetDurationInSeconds( void ) const;

    /** Pointer to the bytes of the given frame. Valid until the buffer is resized */
    void* GetDataPtr( UInt32 frame = 0 );

    const void* GetDataPtr( UInt32 frame = 0 ) const;

    /** Changes the number of frames. New frames are silence, existing ones are kept */
    bool Resize( UInt32 frameCount );

    /** Removes all frames, the format stays */
    void Clear( void );

    /** Sets frames to silence ('count' of zero means until the end) */
    void FillSilence( UInt32 firstFrame = 0, UInt32 count = 0 );

    /** Adds the frames of another buffer at the end. The formats have to be the same */
    bool Append( const CAudioBuffer& other );

    /** Reads one sample as a float in the nominal range -1.0 to 1.0 */
    Float32 GetSampleAsFloat( UInt32 frame, UInt32 channel ) const;

    void SetSampleFromFloat( UInt32 frame, UInt32 channel, Float32 value );

    /**
     *  Converts the buffer to the given format: sample type, channel layout and sample rate, as far as they differ.
     *  The buffer holds the converted audio afterwards. Returns false (buffer unchanged) when the conversion is not possible.
     */
    bool ConvertFormatTo( const CAudioFormat& format );

    /** Same, leaving this buffer as it is and giving the converted audio as a new buffer */
    bool ConvertFormatTo( const CAudioFormat& format, TAudioBufferPtr& newBuffer ) const;

    /** Same sample rate and channels, other sample type */
    bool ConvertFormatTo( TSampleType sampleType );

    private:

    void RefreshDataBase( void );

    private:

    CAudioFormat m_format;
    CORE::CDynamicBuffer m_data;
    UInt8* m_dataBase;          /**< where the frames start, kept up to date by every change of the buffer: the audio thread reaches the frames without calling the dynamic buffer */
    UInt32 m_frameCount;
};

typedef CAudioBuffer::TAudioBufferPtr TAudioBufferPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOBUFFER_H ? */
