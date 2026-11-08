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

#ifndef GUCEF_AUDIO_CSVFFILTER_H
#define GUCEF_AUDIO_CSVFFILTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

#ifndef GUCEF_AUDIO_EFILTERRESPONSE_H
#include "gucefAUDIO_EFilterResponse.h"
#define GUCEF_AUDIO_EFILTERRESPONSE_H
#endif /* GUCEF_AUDIO_EFILTERRESPONSE_H ? */

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
 *  Second order filter for any number of channels, as a linear trapezoidal integrated state variable filter after
 *  A. Simper ("Linear Trapezoidal Integrated SVF", Cytomic technical paper). It has the same magnitude response
 *  as the matching biquad of the "Audio EQ Cookbook" (see CBiquadDesign) for all the responses of TFilterResponse,
 *  but unlike a direct form biquad it stays stable and free of clicks when frequency, Q or gain are changed
 *  from one block to the next (or one sample to the next), and it keeps its precision at low frequencies.
 *  That makes it the filter to use for anything that is modulated: swept equalizers, distance low pass filters.
 *
 *  Usage: Prepare() once with the channel count (allocates), SetResponse() whenever the settings change (cheap,
 *  keeps the state of the filter), then Process() / ProcessInterleaved() for the audio. Processing does not
 *  allocate or lock and may be done on a real time thread, as may SetResponse() (it makes one call to tan()).
 *  The state is kept in Float64.
 *
 *  Feedback state decays towards zero in silence: call FlushDenormals() once per block on platforms where no
 *  flush to zero mode is available (see CDenormalFlushScope).
 *
 *  Not thread safe: one thread at a time uses one filter.
 */
class GUCEF_AUDIO_PUBLIC_CPP CSvfFilter
{
    public:

    CSvfFilter( void );

    ~CSvfFilter();

    /**
     *  Sets the number of channels and clears the state. Allocates: not for the real time thread.
     *  The response starts as pass through until SetResponse() is called.
     */
    bool Prepare( UInt32 channelCount );

    UInt32 GetChannelCount( void ) const;

    /** Clears the state of all channels (the response stays) */
    void Reset( void );

    /**
     *  Sets the response. The state of the filter is kept, so this can be done at any time during processing.
     *  @param response         shape of the response
     *  @param sampleRate       sample rate in Hz
     *  @param frequency        corner or center frequency in Hz, limited to a little below half the sample rate
     *  @param q                quality factor, limited to a small positive value
     *  @param gainInDecibels   boost or cut of peaking and shelving responses, ignored by the others
     *  @return false when the sample rate is not positive, the response is then not changed
     */
    bool SetResponse( TFilterResponse response   ,
                      Float64 sampleRate         ,
                      Float64 frequency          ,
                      Float64 q                  ,
                      Float64 gainInDecibels = 0.0 );

    /**
     *  Filters one sample of one channel. 'channel' must be below GetChannelCount().
     *  Samples of different channels may be given in any order, but those of one channel in time order.
     */
    inline Float32 Process( UInt32 channel, Float32 input )
    {
        Float64& ic1 = m_state[ (size_t) channel * 2 ];
        Float64& ic2 = m_state[ (size_t) channel * 2 + 1 ];
        const Float64 v0 = (Float64) input;

        const Float64 v3 = v0 - ic2;
        const Float64 v1 = m_a1 * ic1 + m_a2 * v3;
        const Float64 v2 = ic2 + m_a2 * ic1 + m_a3 * v3;
        ic1 = 2.0 * v1 - ic1;
        ic2 = 2.0 * v2 - ic2;

        return (Float32) ( m_m0 * v0 + m_m1 * v1 + m_m2 * v2 );
    }

    /**
     *  Filters 'frames' frames of interleaved audio with GetChannelCount() channels. 'input' and 'output' may be the same buffer
     */
    void ProcessInterleaved( const Float32* input, Float32* output, UInt32 frames );

    /**
     *  Filters 'frames' samples of one channel out of interleaved buffers. 'stride' is the number of floats from one
     *  sample of the channel to the next (the channel count of the buffers); the pointers point at the first sample of the channel.
     *  'input' and 'output' may be the same buffer.
     */
    void ProcessChannel( UInt32 channel, const Float32* input, Float32* output, UInt32 frames, UInt32 stride = 1 );

    /** Sets internal state magnitudes that are smaller than 1e-15 to zero, to keep decaying state out of the denormal range */
    void FlushDenormals( void );

    private:

    CSvfFilter( const CSvfFilter& src );                /**< not implemented */
    CSvfFilter& operator=( const CSvfFilter& src );     /**< not implemented */

    private:

    // coefficients
    Float64 m_a1;
    Float64 m_a2;
    Float64 m_a3;
    Float64 m_m0;
    Float64 m_m1;
    Float64 m_m2;

    // two integrator states per channel: ic1eq, ic2eq
    GUCEF::vector< Float64 > m_state;
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

#endif /* GUCEF_AUDIO_CSVFFILTER_H ? */
