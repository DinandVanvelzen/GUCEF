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

#ifndef GUCEF_AUDIO_CPACKETLOSSCONCEALER_H
#define GUCEF_AUDIO_CPACKETLOSSCONCEALER_H

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
 *  Waveform substitution for a lost frame of speech, after ITU-T G.711 Appendix I generalised to any sample rate, for
 *  codecs without a concealment of their own.
 *
 *  It keeps the last 48.75 ms of decoded audio. At the first lost frame it estimates the pitch period: normalised
 *  cross-correlation of the last 20 ms against the history over periods of 5 to 15 ms, coarse on a signal decimated
 *  to about 8 kHz and refined at the full rate. The lost audio is the last period repeated (the last 2 and 3 periods
 *  after 10 and 20 ms, which avoids a buzz), at full level for 10 ms and then turned down by 20 % of the level per
 *  10 ms, so it is silent after 60 ms. The start of the synthetic signal is matched to the last decoded sample over a
 *  quarter of a period. The first good frame after a loss is cross-faded from the synthetic signal into the decoded
 *  one over a quarter period (half a period after 10 ms of loss, 10 ms after 20 ms of loss), so there is no click.
 *
 *  Prepare() allocates and runs on the owner thread. Everything else is real time safe: no allocation, no lock, a
 *  bounded time per call (the pitch search at the first lost frame is the most expensive one, about 25 thousand
 *  multiply-adds). One thread at a time.
 */
class GUCEF_AUDIO_PUBLIC_CPP CPacketLossConcealer
{
    public:

    CPacketLossConcealer( void );

    ~CPacketLossConcealer();

    /** Allocates for a sample rate of 8000 to 192000 Hz and forgets everything. False for another rate */
    bool Prepare( UInt32 sampleRate );

    bool IsPrepared( void ) const;

    UInt32 GetSampleRate( void ) const;

    /** Forgets the history and any loss in progress. Real time safe */
    void Reset( void );

    /**
     *  Tells the concealer a frame was decoded. Real time safe. The frame is changed in place when frames were
     *  concealed just before it (the cross-fade); the result is what goes to the output. It is also added to the history
     */
    void OnGoodFrame( Float32* pcm, UInt32 frames );

    /** Makes the audio for 'frames' lost samples (one lost frame, or part of one). Real time safe */
    void Conceal( Float32* pcm, UInt32 frames );

    /** True between the first concealed sample and the next good frame */
    bool IsConcealing( void ) const;

    /** Samples concealed in a row so far */
    UInt32 GetConcealedSampleCount( void ) const;

    /** The pitch period in samples used for the loss in progress, or for the last one; 0 before the first */
    UInt32 GetPitchPeriod( void ) const;

    private:

    CPacketLossConcealer( const CPacketLossConcealer& src );                /**< not implemented */
    CPacketLossConcealer& operator=( const CPacketLossConcealer& src );     /**< not implemented */

    UInt32 EstimatePitch( void );
    void StartLoss( void );
    void Synthesize( Float32* out, UInt32 count );
    void AppendToHistory( const Float32* pcm, UInt32 frames );

    private:

    UInt32 m_sampleRate;
    UInt32 m_historySize;
    UInt32 m_valid;                         /**< samples of the history that were set, counted from its end */
    UInt32 m_decimation;
    UInt32 m_window;                        /**< 20 ms */
    UInt32 m_pitchMin;                      /**< 5 ms */
    UInt32 m_pitchMax;                      /**< 15 ms */
    UInt32 m_tenMs;
    GUCEF::vector< Float32 > m_history;
    GUCEF::vector< Float32 > m_decimated;
    GUCEF::vector< Float32 > m_correlation;
    GUCEF::vector< Float32 > m_olaScratch;

    bool m_inLoss;
    UInt32 m_lost;                          /**< samples made since the first lost one (saturates) */
    UInt32 m_pitch;
    UInt32 m_cycleLength;
    UInt32 m_cyclePos;
    Float32 m_seamOffset;
    UInt32 m_seamLength;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CPACKETLOSSCONCEALER_H ? */
