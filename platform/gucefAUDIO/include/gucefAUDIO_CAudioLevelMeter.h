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

#ifndef GUCEF_AUDIO_CAUDIOLEVELMETER_H
#define GUCEF_AUDIO_CAUDIOLEVELMETER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CATOMICFLOAT_H
#include "gucefAUDIO_CAtomicFloat.h"
#define GUCEF_AUDIO_CATOMICFLOAT_H
#endif /* GUCEF_AUDIO_CATOMICFLOAT_H ? */

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
 *  Level of a signal per channel, for meters in a user interface:
 *   - peak: the largest absolute sample of the last block
 *   - RMS: root of a running mean square (one pole, 300 ms)
 *   - peak hold: the highest peak, falling back exponentially (time constant 1.5 s) but never below the current peak
 *
 *  Update() runs on the audio thread (no allocation, no locks); the values are published as atomics and can be read
 *  from any thread at any time. Room for MaxChannels channels is part of the object, so a change of format (a device
 *  reopen) never frees memory that a reader might be looking at. Levels are linear, the decibel getters give dBFS.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioLevelMeter
{
    public:

    enum { MaxChannels = 64 };

    CAudioLevelMeter( void );

    ~CAudioLevelMeter();

    /** Owner thread, while Update() is not called: channel count and rate. Clears the levels */
    void Prepare( UInt32 channels, UInt32 sampleRate );

    /** Clears the levels */
    void Reset( void );

    /** Audio thread: measures a block of interleaved float audio */
    void Update( const Float32* interleaved, UInt32 frames );

    /** Any thread; the channel count, peak and RMS are real time safe (atomics, no trace): audio threads read them for keys */
    UInt32 GetChannelCount( void ) const;
    Float32 GetPeak( UInt32 channel ) const;
    Float32 GetRms( UInt32 channel ) const;
    Float32 GetPeakHold( UInt32 channel ) const;

    /** Any thread, in dBFS. Silence gives MinimumDecibels */
    Float32 GetPeakInDecibels( UInt32 channel ) const;
    Float32 GetRmsInDecibels( UInt32 channel ) const;
    Float32 GetPeakHoldInDecibels( UInt32 channel ) const;

    /** The level reported for silence */
    static const Float32 MinimumDecibels;

    static Float32 ToDecibels( Float32 linear );

    private:

    CAudioLevelMeter( const CAudioLevelMeter& src );              /**< not implemented */
    CAudioLevelMeter& operator=( const CAudioLevelMeter& src );   /**< not implemented */

    private:

    MT::CTAtomic< UInt32 > m_channels;
    CAtomicFloat m_peak[ MaxChannels ];
    CAtomicFloat m_rms[ MaxChannels ];
    CAtomicFloat m_peakHold[ MaxChannels ];

    // audio thread
    Float64 m_meanSquare[ MaxChannels ];
    Float32 m_hold[ MaxChannels ];
    Float64 m_rmsCoefficient;           /**< per frame */
    Float64 m_holdDecayPerFrame;        /**< factor per frame */
    UInt32 m_sampleRate;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOLEVELMETER_H ? */
