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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#include "gucefAUDIO_CToneSequenceSource.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CToneSequenceSource::AllChannels;

namespace {

const Float64 Pi = 3.14159265358979323846;

/** Default length of the fades at the edges of a segment */
const UInt32 DefaultFadeInMs = 5;

/** A well mixed 64 bit hash, so noise is the same for the same frame without keeping any state */
UInt64
MixBits( UInt64 x )
{
    x += 0x9E3779B97F4A7C15ULL;
    x = ( x ^ ( x >> 30 ) ) * 0xBF58476D1CE4E5B9ULL;
    x = ( x ^ ( x >> 27 ) ) * 0x94D049BB133111EBULL;
    return x ^ ( x >> 31 );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CToneSequenceSource::SSegment::SSegment( void )
    : waveform( WAVEFORM_SILENCE )
    , frequency( 0.0 )
    , endFrequency( 0.0 )
    , amplitude( 0.0f )
    , durationInMs( 0 )
    , channelMask( AllChannels )
    , fadeMs( DefaultFadeInMs )
    , decayInMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CToneSequenceSource::SSegment::SSegment( TWaveform waveform_, Float64 frequency_, UInt32 durationInMs_, Float32 amplitude_, UInt32 channelMask_ )
    : waveform( waveform_ )
    , frequency( frequency_ )
    , endFrequency( frequency_ )
    , amplitude( amplitude_ )
    , durationInMs( durationInMs_ )
    , channelMask( channelMask_ )
    , fadeMs( DefaultFadeInMs )
    , decayInMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CToneSequenceSource::CToneSequenceSource( UInt32 sampleRate              ,
                                          UInt32 channels                ,
                                          const TSegmentVector& segments ,
                                          UInt32 loopCount               )
    : IAudioSource()
    , m_format( sampleRate, channels, SAMPLETYPE_FLOAT32 )
    , m_segments( segments )
    , m_segmentStarts()
    , m_sequenceLength( 0 )
    , m_loopCount( loopCount )
    , m_position( 0 )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_segments.size(); ++i )
    {
        m_segmentStarts.push_back( m_sequenceLength );
        m_sequenceLength += ( (UInt64) m_segments[ i ].durationInMs * sampleRate ) / 1000;
    }
}

/*-------------------------------------------------------------------------*/

CToneSequenceSource::~CToneSequenceSource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CAudioFormat&
CToneSequenceSource::GetFormat( void ) const
{GUCEF_TRACE;

    return m_format;
}

/*-------------------------------------------------------------------------*/

bool
CToneSequenceSource::GetSegmentAt( UInt64 frameInSequence, UInt32& segmentIndex, UInt64& frameInSegment ) const
{
    // no trace macro: this runs where the audio is mixed

    if ( frameInSequence >= m_sequenceLength )
        return false;

    // the segment lists of test signals are short, a linear look is fine
    for ( size_t i=m_segmentStarts.size(); i>0; --i )
    {
        if ( m_segmentStarts[ i-1 ] <= frameInSequence )
        {
            segmentIndex = (UInt32) ( i-1 );
            frameInSegment = frameInSequence - m_segmentStarts[ i-1 ];
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

Float32
CToneSequenceSource::SampleAt( const SSegment& segment, UInt64 segmentLength, UInt64 frameInSegment ) const
{
    const Float64 rate = (Float64) m_format.GetSampleRate();
    const Float64 t = (Float64) frameInSegment / rate;
    const Float64 cycles = segment.frequency * t;
    const Float64 phase = cycles - floor( cycles );

    Float64 value = 0.0;
    switch ( segment.waveform )
    {
        case WAVEFORM_SINE:         { value = sin( 2.0 * Pi * phase ); break; }
        case WAVEFORM_SQUARE:       { value = phase < 0.5 ? 1.0 : -1.0; break; }
        case WAVEFORM_TRIANGLE:     { value = phase < 0.5 ? 4.0 * phase - 1.0 : 3.0 - 4.0 * phase; break; }
        case WAVEFORM_SAWTOOTH:     { value = 2.0 * phase - 1.0; break; }
        case WAVEFORM_SWEEP:
        {
            // an exponential sweep: the phase is the integral of f0 * ( f1 / f0 )^( t / T )
            const Float64 duration = (Float64) segmentLength / rate;
            const Float64 k = ( segment.frequency > 0.0 && segment.endFrequency > 0.0 ) ? log( segment.endFrequency / segment.frequency ) : 0.0;
            if ( 0.0 == k || duration <= 0.0 )
            {
                value = sin( 2.0 * Pi * phase );
            }
            else
            {
                const Float64 sweepCycles = segment.frequency * duration / k * ( exp( t * k / duration ) - 1.0 );
                value = sin( 2.0 * Pi * ( sweepCycles - floor( sweepCycles ) ) );
            }
            break;
        }
        case WAVEFORM_WHITE_NOISE:
        {
            const UInt64 bits = MixBits( frameInSegment ^ ( (UInt64) segment.durationInMs << 40 ) );
            value = ( (Float64) ( bits >> 11 ) / (Float64) ( 1ULL << 53 ) ) * 2.0 - 1.0;
            break;
        }
        case WAVEFORM_SILENCE:
        default:
        {
            return 0.0f;
        }
    }

    // fades at both edges, raised cosine
    UInt64 fadeFrames = ( (UInt64) segment.fadeMs * m_format.GetSampleRate() ) / 1000;
    if ( fadeFrames > segmentLength / 2 )
        fadeFrames = segmentLength / 2;
    Float64 envelope = 1.0;
    if ( fadeFrames > 0 )
    {
        if ( frameInSegment < fadeFrames )
            envelope = 0.5 - 0.5 * cos( Pi * (Float64) frameInSegment / (Float64) fadeFrames );
        else
        if ( frameInSegment >= segmentLength - fadeFrames )
            envelope = 0.5 - 0.5 * cos( Pi * (Float64) ( segmentLength - 1 - frameInSegment ) / (Float64) fadeFrames );
    }

    // exponential decay from the end of the fade in: no step where the fade in ends, the fade out at the end still applies
    if ( segment.decayInMs > 0 && frameInSegment > fadeFrames )
        envelope *= exp( -( (Float64) ( frameInSegment - fadeFrames ) / rate ) / ( (Float64) segment.decayInMs * 0.001 ) );

    return (Float32) ( value * envelope * segment.amplitude );
}

/*-------------------------------------------------------------------------*/

UInt32
CToneSequenceSource::ReadFrames( void* destination, UInt32 frameCapacity )
{
    // no trace macro: this runs where the audio is mixed

    Float32* out = static_cast< Float32* >( destination );
    const UInt32 channels = m_format.GetChannelCount();
    UInt32 done = 0;
    while ( done < frameCapacity && !IsEndOfStream() && m_sequenceLength > 0 )
    {
        UInt32 segmentIndex = 0;
        UInt64 frameInSegment = 0;
        if ( !GetSegmentAt( m_position % m_sequenceLength, segmentIndex, frameInSegment ) )
            break;

        const SSegment& segment = m_segments[ segmentIndex ];
        const UInt64 segmentLength = ( (UInt64) segment.durationInMs * m_format.GetSampleRate() ) / 1000;
        const Float32 sample = SampleAt( segment, segmentLength, frameInSegment );
        for ( UInt32 c=0; c<channels; ++c )
        {
            const bool sounds = c < 32 ? 0 != ( segment.channelMask & ( 1U << c ) ) : AllChannels == segment.channelMask;
            *out++ = sounds ? sample : 0.0f;
        }
        ++m_position;
        ++done;
    }
    return done;
}

/*-------------------------------------------------------------------------*/

bool
CToneSequenceSource::IsEndOfStream( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return 0 != m_loopCount && m_position >= m_sequenceLength * m_loopCount;
}

/*-------------------------------------------------------------------------*/

bool
CToneSequenceSource::IsRealtimeSafe( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CToneSequenceSource::IsSeekable( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CToneSequenceSource::SeekToFrame( UInt64 frame )
{
    // no trace macro: this runs where the audio is mixed

    if ( 0 != m_loopCount && frame > m_sequenceLength * m_loopCount )
        return false;
    m_position = frame;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
CToneSequenceSource::GetPositionInFrames( void ) const
{GUCEF_TRACE;

    return m_position;
}

/*-------------------------------------------------------------------------*/

UInt64
CToneSequenceSource::GetLengthInFrames( void ) const
{GUCEF_TRACE;

    return 0 == m_loopCount ? GUCEF_AUDIO_LENGTH_UNKNOWN : m_sequenceLength * m_loopCount;
}

/*-------------------------------------------------------------------------*/

UInt64
CToneSequenceSource::GetSequenceLengthInFrames( void ) const
{GUCEF_TRACE;

    return m_sequenceLength;
}

/*-------------------------------------------------------------------------*/

const CToneSequenceSource::TSegmentVector&
CToneSequenceSource::GetSegments( void ) const
{GUCEF_TRACE;

    return m_segments;
}

/*-------------------------------------------------------------------------*/

CToneSequenceSource::TAudioSourcePtr
CToneSequenceSource::MakeChime( UInt32 sampleRate, UInt32 channels )
{GUCEF_TRACE;

    TSegmentVector segments;
    static const Float64 notes[ 2 ] = { 784.0, 659.0 };
    for ( UInt32 i=0; i<2; ++i )
    {
        SSegment note( WAVEFORM_SINE, notes[ i ], 350, 0.4f );
        note.decayInMs = 250;
        segments.push_back( note );
    }
    return TAudioSourcePtr( GUCEF_NEW CToneSequenceSource( sampleRate, channels, segments ) );
}

/*-------------------------------------------------------------------------*/

CAudioTestSignals::TAudioSourcePtr
CAudioTestSignals::MakeTone( UInt32 sampleRate, UInt32 channels, Float64 frequency, UInt32 durationInMs, Float32 amplitude )
{GUCEF_TRACE;

    CToneSequenceSource::TSegmentVector segments;
    segments.push_back( CToneSequenceSource::SSegment( CToneSequenceSource::WAVEFORM_SINE, frequency, durationInMs, amplitude ) );
    return TAudioSourcePtr( GUCEF_NEW CToneSequenceSource( sampleRate, channels, segments ) );
}

/*-------------------------------------------------------------------------*/

CAudioTestSignals::TAudioSourcePtr
CAudioTestSignals::MakeToneLadder( UInt32 sampleRate, UInt32 channels, UInt32 stepInMs, Float32 amplitude )
{GUCEF_TRACE;

    CToneSequenceSource::TSegmentVector segments;
    for ( Float64 frequency = 110.0; frequency <= 7040.0 && frequency < sampleRate / 2; frequency *= 2.0 )
    {
        segments.push_back( CToneSequenceSource::SSegment( CToneSequenceSource::WAVEFORM_SINE, frequency, stepInMs, amplitude ) );
        segments.push_back( CToneSequenceSource::SSegment( CToneSequenceSource::WAVEFORM_SILENCE, 0.0, stepInMs / 4 ) );
    }
    return TAudioSourcePtr( GUCEF_NEW CToneSequenceSource( sampleRate, channels, segments ) );
}

/*-------------------------------------------------------------------------*/

CAudioTestSignals::TAudioSourcePtr
CAudioTestSignals::MakeChannelCheck( UInt32 sampleRate, UInt32 channels, Float32 amplitude )
{GUCEF_TRACE;

    static const Float64 steps[ 3 ] = { 440.0, 554.37, 659.26 };    // A, C sharp, E: a major chord, one note at a time

    CToneSequenceSource::TSegmentVector segments;
    for ( UInt32 c=0; c<channels && c<32; ++c )
    {
        for ( UInt32 s=0; s<3; ++s )
            segments.push_back( CToneSequenceSource::SSegment( CToneSequenceSource::WAVEFORM_SINE, steps[ s ], 250, amplitude, 1U << c ) );
        segments.push_back( CToneSequenceSource::SSegment( CToneSequenceSource::WAVEFORM_SILENCE, 0.0, 500 ) );
    }
    return TAudioSourcePtr( GUCEF_NEW CToneSequenceSource( sampleRate, channels, segments ) );
}

/*-------------------------------------------------------------------------*/

CAudioTestSignals::TAudioSourcePtr
CAudioTestSignals::MakeSweep( UInt32 sampleRate, UInt32 channels, UInt32 durationInMs, Float32 amplitude )
{GUCEF_TRACE;

    CToneSequenceSource::SSegment sweep( CToneSequenceSource::WAVEFORM_SWEEP, 20.0, durationInMs, amplitude );
    sweep.endFrequency = 20000.0;
    if ( sweep.endFrequency > 0.45 * sampleRate )
        sweep.endFrequency = 0.45 * sampleRate;
    CToneSequenceSource::TSegmentVector segments;
    segments.push_back( sweep );
    return TAudioSourcePtr( GUCEF_NEW CToneSequenceSource( sampleRate, channels, segments ) );
}

/*-------------------------------------------------------------------------*/

CAudioTestSignals::TAudioSourcePtr
CAudioTestSignals::MakeNoise( UInt32 sampleRate, UInt32 channels, UInt32 durationInMs, Float32 amplitude )
{GUCEF_TRACE;

    CToneSequenceSource::TSegmentVector segments;
    segments.push_back( CToneSequenceSource::SSegment( CToneSequenceSource::WAVEFORM_WHITE_NOISE, 0.0, durationInMs, amplitude ) );
    return TAudioSourcePtr( GUCEF_NEW CToneSequenceSource( sampleRate, channels, segments ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
