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

#ifndef GUCEF_AUDIO_CFIRDECIMATOR_H
#define GUCEF_AUDIO_CFIRDECIMATOR_H

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
 *  Rate reduction of mono audio by a whole factor (48 to 16 kHz: 3) with a linear phase FIR low pass (a Kaiser windowed
 *  sinc), computed only at the output instants: 'factor' times 'tapsPerPhase' multiplications per output frame. A cheap
 *  DSP block for signals that need a good pass band rather than a steep edge, such as the reference of the echo
 *  canceller; CSincResampler is the general converter (any ratio, more taps, more cost).
 *
 *  Silence costs nothing: once the filter holds only zeros, blocks of zeros give zeros without arithmetic. The output
 *  is delayed by GetLatencyInFrames() output frames. Prepare() allocates; Process() allocates nothing and takes no lock.
 */
class GUCEF_AUDIO_PUBLIC_CPP CFirDecimator
{
    public:

    CFirDecimator( void );

    ~CFirDecimator();

    /**
     *  Owner: a decimator by 'factor' (1 to 16; 1 passes the audio through as it is) with 'tapsPerPhase' taps per output
     *  phase (4 to 64) and a cut-off of 'cutoff' times the output rate (0.1 to 0.5; 0.42 keeps 6.7 kHz of a 16 kHz
     *  output), stop band attenuation set by the Kaiser 'beta' (6: about 60 dB). Allocates
     */
    bool Prepare( UInt32 factor, UInt32 tapsPerPhase = 24, Float64 cutoff = 0.42, Float64 beta = 6.0 );

    void Release( void );

    bool IsPrepared( void ) const;

    UInt32 GetFactor( void ) const;

    UInt32 GetTapCount( void ) const;

    /** The delay of the output, in output frames (may be fractional for an even tap count: rounded down) */
    UInt32 GetLatencyInFrames( void ) const;

    /** Clears the history: the next input frame is the first of a stream */
    void Reset( void );

    /** The output frames the next 'inputFrames' input frames give */
    UInt32 GetOutputCount( UInt32 inputFrames ) const;

    /** Takes 'frames' input frames, writes the outputs (GetOutputCount( frames ) of them) and returns their number */
    UInt32 Process( const Float32* input, UInt32 frames, Float32* output );

    private:

    CFirDecimator( const CFirDecimator& src );              /**< not implemented */
    CFirDecimator& operator=( const CFirDecimator& src );   /**< not implemented */

    private:

    UInt32 m_factor;
    UInt32 m_taps;
    Float32* m_coefficients;
    Float32* m_history;         /**< two copies of the last m_taps inputs, so a window is always contiguous */
    UInt32 m_write;             /**< where the next input goes, 0 .. m_taps - 1 */
    UInt32 m_phase;             /**< inputs since the last output, 0 .. m_factor - 1 */
    UInt32 m_zeroRun;           /**< consecutive zero inputs, up to m_taps */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CFIRDECIMATOR_H ? */
