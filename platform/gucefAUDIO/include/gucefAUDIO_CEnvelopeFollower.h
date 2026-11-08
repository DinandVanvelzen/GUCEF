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

#ifndef GUCEF_AUDIO_CENVELOPEFOLLOWER_H
#define GUCEF_AUDIO_CENVELOPEFOLLOWER_H

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What the envelope follower measures */
enum EEnvelopeMode
{
    ENVELOPEMODE_PEAK = 0   ,   /**< follows the absolute value of the signal, rising with the attack time and falling with the release time */
    ENVELOPEMODE_RMS            /**< root mean square of the signal over a window (a one pole average of the squares) */
};
typedef enum EEnvelopeMode TEnvelopeMode;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Level detector for one signal: follows how loud a signal is, the way the detector of a compressor or a
 *  level meter does.
 *
 *  Time constants are those of a one pole smoother, so after the stated time a step in the input has been followed
 *  for 63.2 % (1 - 1/e) of its size. The coefficient is exp( -1 / ( time * sampleRate ) ); a time of 0 follows instantly.
 *  - Peak mode: the envelope moves to the absolute value of the input with the attack time when the input is above
 *    it and with the release time when it is below it
 *  - RMS mode: the envelope is the square root of the one pole average of the squared input, with the window
 *    time as time constant for rising and falling alike (attack and release times are not used)
 *
 *  Processing does not allocate or lock. Not thread safe: one thread at a time uses one follower, and one follower
 *  follows one channel (use one per channel, or feed it the linked peak of the channels).
 *  Call FlushDenormals() once per block where no flush to zero mode is available (see CDenormalFlushScope).
 */
class GUCEF_AUDIO_PUBLIC_CPP CEnvelopeFollower
{
    public:

    CEnvelopeFollower( void );

    ~CEnvelopeFollower();

    /** Sets the sample rate that the times are converted with and clears the envelope. The times set before are kept */
    bool Prepare( Float64 sampleRate );

    void SetMode( TEnvelopeMode mode );
    TEnvelopeMode GetMode( void ) const;

    void SetAttackTimeInMs( Float64 milliseconds );
    void SetReleaseTimeInMs( Float64 milliseconds );
    void SetRmsWindowInMs( Float64 milliseconds );

    /** Sets the envelope to zero */
    void Reset( void );

    /** The current envelope, as last returned by Process() */
    inline Float32 GetEnvelope( void ) const
    {
        return ENVELOPEMODE_RMS == m_mode ? (Float32) sqrt( m_state ) : (Float32) m_state;
    }

    /** Feeds one sample, returns the envelope after it */
    inline Float32 Process( Float32 input )
    {
        if ( ENVELOPEMODE_RMS == m_mode )
        {
            const Float64 square = (Float64) input * (Float64) input;
            m_state = square + m_rmsCoefficient * ( m_state - square );
            return (Float32) sqrt( m_state );
        }

        const Float64 magnitude = fabs( (Float64) input );
        const Float64 coefficient = magnitude > m_state ? m_attackCoefficient : m_releaseCoefficient;
        m_state = magnitude + coefficient * ( m_state - magnitude );
        return (Float32) m_state;
    }

    /** Feeds 'frames' samples that are 'stride' apart, stores the envelope after each in 'envelope' (may be GUCEF_NULL) */
    void ProcessBlock( const Float32* input, Float32* envelope, UInt32 frames, UInt32 stride = 1 );

    /** Sets a state that has decayed to a negligible level to zero, to keep it out of the denormal range */
    void FlushDenormals( void );

    private:

    CEnvelopeFollower( const CEnvelopeFollower& src );              /**< not implemented */
    CEnvelopeFollower& operator=( const CEnvelopeFollower& src );   /**< not implemented */

    void UpdateCoefficients( void );

    static Float64 CoefficientForMs( Float64 milliseconds, Float64 sampleRate );

    private:

    TEnvelopeMode m_mode;
    Float64 m_sampleRate;
    Float64 m_attackMs;
    Float64 m_releaseMs;
    Float64 m_rmsWindowMs;
    Float64 m_attackCoefficient;
    Float64 m_releaseCoefficient;
    Float64 m_rmsCoefficient;
    Float64 m_state;                /**< the envelope, in RMS mode the mean square */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CENVELOPEFOLLOWER_H ? */
