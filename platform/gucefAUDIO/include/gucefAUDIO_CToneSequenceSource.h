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

#ifndef GUCEF_AUDIO_CTONESEQUENCESOURCE_H
#define GUCEF_AUDIO_CTONESEQUENCESOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  Generated audio: a sequence of segments, each a waveform at a frequency and level on a chosen set of channels.
 *  Meant for checking that sound comes out, out of the right speaker and at the right pitch ("test my audio"),
 *  and as a known signal for tests. Every frame is computed from its position, so the source is seekable and the
 *  same frame always has the same value. Segments fade in and out over a few milliseconds so they do not click.
 *  A segment can also decay exponentially, which turns a sine into a bell like tone (see MakeChime()).
 *  See CAudioTestSignals for ready made sequences.
 */
class GUCEF_AUDIO_PUBLIC_CPP CToneSequenceSource : public IAudioSource
{
    public:

    enum EWaveform
    {
        WAVEFORM_SILENCE    = 0 ,
        WAVEFORM_SINE           ,
        WAVEFORM_SQUARE         ,
        WAVEFORM_TRIANGLE       ,
        WAVEFORM_SAWTOOTH       ,
        WAVEFORM_SWEEP          ,   /**< a sine that glides from the frequency to the end frequency, evenly per octave */
        WAVEFORM_WHITE_NOISE
    };
    typedef enum EWaveform TWaveform;

    /** All channels */
    static const UInt32 AllChannels = 0xFFFFFFFFU;

    struct GUCEF_AUDIO_PUBLIC_CPP SSegment
    {
        TWaveform waveform;
        Float64 frequency;          /**< Hz */
        Float64 endFrequency;       /**< Hz, for WAVEFORM_SWEEP */
        Float32 amplitude;          /**< peak level, 0 to 1 */
        UInt32 durationInMs;
        UInt32 channelMask;         /**< bit n set: channel n sounds, the others are silent. AllChannels for all */
        UInt32 fadeMs;              /**< ramps at the start and end of the segment against clicks */
        UInt32 decayInMs;           /**< time constant of an exponential decay exp( -t / decay ) that starts at the end of the fade in. 0 is flat (no decay) */

        SSegment( void );
        SSegment( TWaveform waveform, Float64 frequency, UInt32 durationInMs, Float32 amplitude = 0.5f, UInt32 channelMask = AllChannels );
    };
    typedef GUCEF::vector< SSegment > TSegmentVector;

    /**
     *  @param sampleRate   rate of the frames made, in Hz
     *  @param channels     channels of the frames made, float32 samples
     *  @param segments     what to play, in order
     *  @param loopCount    how often to play the sequence, 0 is forever
     */
    CToneSequenceSource( UInt32 sampleRate               ,
                         UInt32 channels                 ,
                         const TSegmentVector& segments  ,
                         UInt32 loopCount = 1            );

    virtual ~CToneSequenceSource() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsRealtimeSafe( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSeekable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SeekToFrame( UInt64 frame ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Frames of all plays together, unknown when the sequence loops forever */
    virtual UInt64 GetLengthInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Frames of one play of the sequence */
    UInt64 GetSequenceLengthInFrames( void ) const;

    /** The segment that plays at the given frame of a play, and the frame within it. False past the end */
    bool GetSegmentAt( UInt64 frameInSequence, UInt32& segmentIndex, UInt64& frameInSegment ) const;

    const TSegmentVector& GetSegments( void ) const;

    /**
     *  The default two tone chime that announces something: 784 Hz (G5) and then 659 Hz (E5), 350 ms each, a sine
     *  with a decay time constant of 250 ms at amplitude 0.4, on all channels, played once. 700 ms long.
     *  Float32 at the given rate and channel count, so it plays without conversion on a device with that format.
     */
    static TAudioSourcePtr MakeChime( UInt32 sampleRate, UInt32 channels );

    private:

    Float32 SampleAt( const SSegment& segment, UInt64 segmentLength, UInt64 frameInSegment ) const;

    CToneSequenceSource( const CToneSequenceSource& src );              /**< not implemented */
    CToneSequenceSource& operator=( const CToneSequenceSource& src );   /**< not implemented */

    private:

    CAudioFormat m_format;
    TSegmentVector m_segments;
    GUCEF::vector< UInt64 > m_segmentStarts;    /**< first frame of each segment within a play */
    UInt64 m_sequenceLength;
    UInt32 m_loopCount;
    UInt64 m_position;
};

/*-------------------------------------------------------------------------*/

/**
 *  Ready made signals for checking audio output, as the "test my audio" facility of an application.
 *  All are float32 at the given rate and channel count, so they play without conversion on a device with that format.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioTestSignals
{
    public:

    typedef IAudioSource::TAudioSourcePtr TAudioSourcePtr;

    /** One tone */
    static TAudioSourcePtr MakeTone( UInt32 sampleRate, UInt32 channels, Float64 frequency, UInt32 durationInMs, Float32 amplitude = 0.5f );

    /**
     *  A tone ladder: sine tones going up by octaves from 110 Hz to 7040 Hz, each 'stepInMs' long with a short pause
     *  in between, on all channels. Checks that sound comes out and that low and high pitches are heard.
     */
    static TAudioSourcePtr MakeToneLadder( UInt32 sampleRate, UInt32 channels, UInt32 stepInMs = 400, Float32 amplitude = 0.4f );

    /**
     *  Each channel in turn: a short ladder of three tones on channel 0 only, then on channel 1 only, and so on, with
     *  a pause in between. Checks that every speaker works and is where it should be (left before right).
     */
    static TAudioSourcePtr MakeChannelCheck( UInt32 sampleRate, UInt32 channels, Float32 amplitude = 0.4f );

    /** A sine that glides from 20 Hz to 20 kHz (capped below half the sample rate), evenly per octave */
    static TAudioSourcePtr MakeSweep( UInt32 sampleRate, UInt32 channels, UInt32 durationInMs = 10000, Float32 amplitude = 0.3f );

    /** White noise, for levels */
    static TAudioSourcePtr MakeNoise( UInt32 sampleRate, UInt32 channels, UInt32 durationInMs = 2000, Float32 amplitude = 0.2f );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CTONESEQUENCESOURCE_H ? */
