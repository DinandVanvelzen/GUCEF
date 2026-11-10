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

#ifndef GUCEF_AUDIO_CAUDIOFORMAT_H
#define GUCEF_AUDIO_CAUDIOFORMAT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  Describes how audio is laid out in memory or on a device: sample rate, number of channels, how a sample is
 *  stored and which speaker each channel belongs to. Audio is always interleaved: one frame is one sample of
 *  every channel, frames follow each other.
 *
 *  A format that was not set up is 'invalid' (no rate, no channels). It is used to say 'let the device decide'
 *  when asking for a device format.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioFormat
{
    public:

    CAudioFormat( void );

    /**
     *  @param channelLayout bit mask of ESpeakerPosition values, 0 for the usual layout of the channel count (see GetDefaultChannelLayout)
     */
    CAudioFormat( UInt32 sampleRate         ,
                  UInt32 channelCount       ,
                  TSampleType sampleType    ,
                  UInt32 channelLayout = 0  );

    CAudioFormat( const CAudioFormat& src );

    CAudioFormat& operator=( const CAudioFormat& src );

    ~CAudioFormat();

    void SetSampleRate( UInt32 sampleRate );

    UInt32 GetSampleRate( void ) const;

    /** Setting the channel count drops an explicit channel layout when it does not have that many channels */
    void SetChannelCount( UInt32 channelCount );

    UInt32 GetChannelCount( void ) const;

    void SetSampleType( TSampleType sampleType );

    TSampleType GetSampleType( void ) const;

    /** Sets the speakers of the channels, the channel count follows from it. Zero goes back to the default layout */
    void SetChannelLayout( UInt32 channelLayout );

    /** The speaker positions of the channels: the explicit layout, or the usual one for the channel count */
    UInt32 GetChannelLayout( void ) const;

    bool HasExplicitChannelLayout( void ) const;

    UInt32 GetBytesPerSample( void ) const;

    UInt32 GetBytesPerFrame( void ) const;

    /** True when rate and channels are set and the sample type is known */
    bool IsValid( void ) const;

    bool operator==( const CAudioFormat& other ) const;

    bool operator!=( const CAudioFormat& other ) const;

    /** Text for logs, like "44100 Hz, 2 ch, float32" */
    CString ToString( void ) const;

    /** Size in bytes of one sample of the given type, 0 for unknown */
    static UInt32 GetBytesPerSample( TSampleType sampleType );

    static const char* SampleTypeToString( TSampleType sampleType );

    /** The reverse of SampleTypeToString(), without regard for case. SAMPLETYPE_UNKNOWN for text that is not a sample type */
    static TSampleType SampleTypeFromString( const CString& text );

    /** The speaker layout normally used for a number of channels: 1 mono, 2 stereo, 4 quad, 6 is 5.1, 8 is 7.1 and so on. 0 for no channels */
    static UInt32 GetDefaultChannelLayout( UInt32 channelCount );

    /** Number of speakers in a layout mask */
    static UInt32 GetChannelCountOfLayout( UInt32 channelLayout );

    private:

    UInt32 m_sampleRate;
    UInt32 m_channelCount;
    TSampleType m_sampleType;
    UInt32 m_channelLayout;     /**< 0 when not explicit */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOFORMAT_H ? */
