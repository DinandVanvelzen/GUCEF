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

#ifndef GUCEF_AUDIO_CDELAYLINE_H
#define GUCEF_AUDIO_CDELAYLINE_H

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
 *  The history of the last samples of one or more channels in a circular buffer, with reads at a delay that is
 *  a whole number of frames or has a fraction (linear or cubic Hermite interpolation). The building block of
 *  echoes, reverbs, chorus and lookahead.
 *
 *  Every channel has a ring of its own of a power of two size, allocated in Prepare() for the longest delay
 *  needed. Nothing allocates or locks after that. Not thread safe: one thread at a time uses one delay line.
 *
 *  Time is counted in pushes. Push() stores a sample as the newest one of a channel. The delay of a read counts
 *  back from the newest sample:
 *  - delay 0 is the sample that was pushed last, delay 1 the one pushed before that, and so on
 *  - to delay a signal by D frames: Push( x[n] ), then Read( D ) gives x[n-D]
 *  - for a delay that feeds back into itself, read first: before the Push of x[n] the newest sample is x[n-1],
 *    so x[n-D] is Read( D - 1 ), after which the result is mixed with the input and pushed
 *
 *  Reads below the shortest delay (0 for integer and linear reads, 1 for cubic reads, which need the next
 *  newer sample too) and above the maximum delay given to Prepare() are limited to those. History that was
 *  not written yet reads as silence.
 */
class GUCEF_AUDIO_PUBLIC_CPP CDelayLine
{
    public:

    CDelayLine( void );

    ~CDelayLine();

    /**
     *  Allocates the buffers and clears them. Allocates: not for the real time thread.
     *  @param channelCount         number of channels
     *  @param maxDelayInFrames     longest delay (of Read(), the fractional reads can go up to this too) that will be asked for
     */
    bool Prepare( UInt32 channelCount, UInt32 maxDelayInFrames );

    UInt32 GetChannelCount( void ) const;

    UInt32 GetMaxDelayInFrames( void ) const;

    /** Clears the history of all channels */
    void Reset( void );

    /** Stores a sample as the newest one of a channel */
    inline void Push( UInt32 channel, Float32 sample )
    {
        UInt32& position = m_writePositions[ channel ];
        m_buffers[ (size_t) channel * m_capacity + ( position & m_mask ) ] = sample;
        ++position;
    }

    /** Sample at a whole number of frames back from the newest one */
    inline Float32 Read( UInt32 channel, UInt32 delay ) const
    {
        if ( delay > m_maxDelay ) delay = m_maxDelay;
        return Peek( channel, delay );
    }

    /** Sample at a delay with a fraction, linear interpolation between the two nearest samples */
    inline Float32 ReadLinear( UInt32 channel, Float32 delay ) const
    {
        if ( !( delay > 0.0f ) ) delay = 0.0f;
        const Float32 maxDelay = (Float32) m_maxDelay;
        if ( delay > maxDelay ) delay = maxDelay;
        const UInt32 whole = (UInt32) delay;
        const Float32 fraction = delay - (Float32) whole;
        const Float32 newer = Peek( channel, whole );
        const Float32 older = Peek( channel, whole + 1 );
        return newer + fraction * ( older - newer );
    }

    /**
     *  Sample at a delay with a fraction, cubic Hermite (four point, third order) interpolation: smoother than
     *  linear, with no loss of high frequencies worth speaking of. Needs a delay of at least 1.
     */
    inline Float32 ReadCubic( UInt32 channel, Float32 delay ) const
    {
        if ( !( delay > 1.0f ) ) delay = 1.0f;
        const Float32 maxDelay = (Float32) m_maxDelay;
        if ( delay > maxDelay ) delay = maxDelay;
        const UInt32 whole = (UInt32) delay;
        const Float32 t = delay - (Float32) whole;

        // four samples around the position, in time order from the older side: the one a frame newer than 'whole', 'whole', and two older
        const Float32 xm1 = Peek( channel, whole - 1 );
        const Float32 x0 = Peek( channel, whole );
        const Float32 x1 = Peek( channel, whole + 1 );
        const Float32 x2 = Peek( channel, whole + 2 );

        const Float32 c1 = 0.5f * ( x1 - xm1 );
        const Float32 c2 = xm1 - 2.5f * x0 + 2.0f * x1 - 0.5f * x2;
        const Float32 c3 = 0.5f * ( x2 - xm1 ) + 1.5f * ( x0 - x1 );
        return ( ( c3 * t + c2 ) * t + c1 ) * t + x0;
    }

    private:

    CDelayLine( const CDelayLine& src );                /**< not implemented */
    CDelayLine& operator=( const CDelayLine& src );     /**< not implemented */

    /** No limits applied: 'delay' + 2 must be below the capacity */
    inline Float32 Peek( UInt32 channel, UInt32 delay ) const
    {
        const UInt32 position = m_writePositions[ channel ] - 1 - delay;
        return m_buffers[ (size_t) channel * m_capacity + ( position & m_mask ) ];
    }

    private:

    GUCEF::vector< Float32 > m_buffers;         /**< channel after channel, 'm_capacity' samples each */
    GUCEF::vector< UInt32 > m_writePositions;   /**< per channel: running count of pushes, wraps harmlessly with the power of two size */
    UInt32 m_capacity;
    UInt32 m_mask;
    UInt32 m_maxDelay;
    UInt32 m_channelCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CDELAYLINE_H ? */
