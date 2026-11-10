/*
 *  gucefAUDIO_TestApp: GUCEF test application for the AUDIO module
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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOEFFECTSUPPORT_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOEFFECTSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEF_AUDIO_CAUDIOEFFECT_H
#include "gucefAUDIO_CAudioEffect.h"
#define GUCEF_AUDIO_CAUDIOEFFECT_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Helpers shared by the tests of the built-in effects (limiter, compressor, equalizer, filter, delay): reproducible
 *  signals, running an effect in blocks as the audio thread does, and measurements. Header only, inline.
 */
namespace AudioEffectTest {

const GUCEF::Float64 Pi = 3.14159265358979323846;

/** White noise from a fixed seed, never rand() */
class CXorShift
{
    public:

    explicit CXorShift( GUCEF::UInt32 seed = 2463534242U ) : m_state( seed ) {}

    GUCEF::UInt32 Next( void )
    {
        m_state ^= m_state << 13;
        m_state ^= m_state >> 17;
        m_state ^= m_state << 5;
        return m_state;
    }

    /** -1 to 1 */
    GUCEF::Float32 NextFloat( void ) { return (GUCEF::Float32) ( (GUCEF::Float64) Next() / 2147483648.0 - 1.0 ); }

    /** 0 to 1 */
    GUCEF::Float64 NextUnit( void ) { return (GUCEF::Float64) Next() / 4294967296.0; }

    private:

    GUCEF::UInt32 m_state;
};

typedef GUCEF::vector< GUCEF::Float32 > TSamples;

inline TSamples
MakeNoise( GUCEF::UInt32 frames, GUCEF::UInt32 channels, GUCEF::UInt32 seed, GUCEF::Float32 amplitude )
{
    CXorShift random( seed );
    TSamples noise( (size_t) frames * channels, 0.0f );
    for ( size_t i=0; i<noise.size(); ++i )
        noise[ i ] = amplitude * random.NextFloat();
    return noise;
}

/** The same sine on every channel */
inline TSamples
MakeSine( GUCEF::UInt32 frames, GUCEF::UInt32 channels, GUCEF::Float64 frequency, GUCEF::Float32 amplitude, GUCEF::Float64 sampleRate = 48000.0 )
{
    TSamples sine( (size_t) frames * channels, 0.0f );
    for ( GUCEF::UInt32 f=0; f<frames; ++f )
        for ( GUCEF::UInt32 c=0; c<channels; ++c )
            sine[ (size_t) f * channels + c ] = amplitude * (GUCEF::Float32) sin( 2.0 * Pi * frequency * (GUCEF::Float64) f / sampleRate );
    return sine;
}

inline TSamples
MakeConstant( GUCEF::UInt32 frames, GUCEF::UInt32 channels, GUCEF::Float32 value )
{
    return TSamples( (size_t) frames * channels, value );
}

inline GUCEF::Float64
DecibelsOf( GUCEF::Float64 linear )
{
    return 20.0 * log10( linear );
}

inline GUCEF::Float64
LinearOf( GUCEF::Float64 decibels )
{
    return pow( 10.0, decibels / 20.0 );
}

/** Marks the call as a real time context in a RealtimeSanitizer build: an allocation, lock or blocking call in the effect aborts the run there */
inline void
ProcessRealtime( GUCEF::AUDIO::CAudioEffect& effect, GUCEF::Float32* data, GUCEF::UInt32 frames ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    effect.Process( data, frames );
}

inline void
ResetRealtime( GUCEF::AUDIO::CAudioEffect& effect ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    effect.Reset();
}

/** Runs the effect over the audio in place in calls of 'blockFrames' frames (the last one may be shorter) */
inline void
Run( GUCEF::AUDIO::CAudioEffect& effect, TSamples& audio, GUCEF::UInt32 channels, GUCEF::UInt32 blockFrames )
{
    const size_t frames = audio.size() / channels;
    size_t done = 0;
    while ( done < frames )
    {
        const size_t block = frames - done < blockFrames ? frames - done : blockFrames;
        ProcessRealtime( effect, &audio[ done * channels ], (GUCEF::UInt32) block );
        done += block;
    }
}

/** The same, with block sizes drawn from a generator, 1 to 'largestBlock' frames */
inline void
RunInRandomBlocks( GUCEF::AUDIO::CAudioEffect& effect, TSamples& audio, GUCEF::UInt32 channels, GUCEF::UInt32 largestBlock, GUCEF::UInt32 seed )
{
    CXorShift random( seed );
    const size_t frames = audio.size() / channels;
    size_t done = 0;
    while ( done < frames )
    {
        size_t block = 1 + random.Next() % largestBlock;
        if ( block > frames - done )
            block = frames - done;
        ProcessRealtime( effect, &audio[ done * channels ], (GUCEF::UInt32) block );
        done += block;
    }
}

inline GUCEF::Float32
PeakOf( const TSamples& audio, size_t from = 0, size_t to = 0 )
{
    if ( 0 == to || to > audio.size() )
        to = audio.size();
    GUCEF::Float32 peak = 0.0f;
    for ( size_t i=from; i<to; ++i )
        if ( fabsf( audio[ i ] ) > peak )
            peak = fabsf( audio[ i ] );
    return peak;
}

/** Root mean square of one channel over a frame range */
inline GUCEF::Float64
RmsOfChannel( const TSamples& audio, GUCEF::UInt32 channels, GUCEF::UInt32 channel, size_t fromFrame, size_t toFrame )
{
    GUCEF::Float64 sum = 0.0;
    for ( size_t f=fromFrame; f<toFrame; ++f )
    {
        const GUCEF::Float64 s = (GUCEF::Float64) audio[ f * channels + channel ];
        sum += s * s;
    }
    return toFrame > fromFrame ? sqrt( sum / (GUCEF::Float64) ( toFrame - fromFrame ) ) : 0.0;
}

/** Largest difference between two consecutive samples of one channel */
inline GUCEF::Float32
MaxStep( const TSamples& audio, GUCEF::UInt32 channels, GUCEF::UInt32 channel, size_t fromFrame = 0, size_t toFrame = 0 )
{
    const size_t frames = audio.size() / channels;
    if ( 0 == toFrame || toFrame > frames )
        toFrame = frames;
    GUCEF::Float32 largest = 0.0f;
    for ( size_t f=fromFrame+1; f<toFrame; ++f )
    {
        const GUCEF::Float32 d = fabsf( audio[ f * channels + channel ] - audio[ ( f - 1 ) * channels + channel ] );
        if ( d > largest )
            largest = d;
    }
    return largest;
}

inline bool
IsSame( const TSamples& a, const TSamples& b )
{
    return a.size() == b.size() && 0 == memcmp( &a[ 0 ], &b[ 0 ], a.size() * sizeof( GUCEF::Float32 ) );
}

inline bool
AllFiniteAndBelow( const TSamples& audio, GUCEF::Float32 bound )
{
    for ( size_t i=0; i<audio.size(); ++i )
        if ( !( fabsf( audio[ i ] ) <= bound ) )
            return false;
    return true;
}

inline size_t
CountSubnormal( const TSamples& audio )
{
    size_t count = 0;
    for ( size_t i=0; i<audio.size(); ++i )
        if ( FP_SUBNORMAL == fpclassify( audio[ i ] ) )
            ++count;
    return count;
}

/** Magnitude in decibels at 'frequency' of an impulse response (long enough to have decayed) */
inline GUCEF::Float64
MagnitudeOfImpulseResponse( const GUCEF::vector< GUCEF::Float64 >& response, GUCEF::Float64 frequency, GUCEF::Float64 sampleRate )
{
    const GUCEF::Float64 w = 2.0 * Pi * frequency / sampleRate;
    const GUCEF::Float64 stepRe = cos( w ), stepIm = -sin( w );
    GUCEF::Float64 phaseRe = 1.0, phaseIm = 0.0;
    GUCEF::Float64 sumRe = 0.0, sumIm = 0.0;
    for ( size_t n=0; n<response.size(); ++n )
    {
        sumRe += response[ n ] * phaseRe;
        sumIm += response[ n ] * phaseIm;
        const GUCEF::Float64 nextRe = phaseRe * stepRe - phaseIm * stepIm;
        phaseIm = phaseRe * stepIm + phaseIm * stepRe;
        phaseRe = nextRe;
        if ( 0 == ( n & 1023 ) )
        {
            // keep the rotation on the unit circle
            const GUCEF::Float64 norm = 1.0 / sqrt( phaseRe * phaseRe + phaseIm * phaseIm );
            phaseRe *= norm;
            phaseIm *= norm;
        }
    }
    return 20.0 * log10( sqrt( sumRe * sumRe + sumIm * sumIm ) );
}

/** The impulse response of channel 0 of an effect that was prepared for 'channels' channels: a unit impulse on every channel, run in blocks */
inline GUCEF::vector< GUCEF::Float64 >
ImpulseResponseOf( GUCEF::AUDIO::CAudioEffect& effect, GUCEF::UInt32 channels, GUCEF::UInt32 frames, GUCEF::UInt32 blockFrames = 256 )
{
    TSamples audio( (size_t) frames * channels, 0.0f );
    for ( GUCEF::UInt32 c=0; c<channels; ++c )
        audio[ c ] = 1.0f;
    Run( effect, audio, channels, blockFrames );
    GUCEF::vector< GUCEF::Float64 > response( frames );
    for ( GUCEF::UInt32 f=0; f<frames; ++f )
        response[ f ] = (GUCEF::Float64) audio[ (size_t) f * channels ];
    return response;
}

inline bool
Near( GUCEF::Float64 a, GUCEF::Float64 b, GUCEF::Float64 tolerance )
{
    return fabs( a - b ) <= tolerance;
}

}

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOEFFECTSUPPORT_H ? */
