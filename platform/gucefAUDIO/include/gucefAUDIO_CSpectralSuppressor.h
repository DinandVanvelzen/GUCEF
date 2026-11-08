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

#ifndef GUCEF_AUDIO_CSPECTRALSUPPRESSOR_H
#define GUCEF_AUDIO_CSPECTRALSUPPRESSOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CREALFFT_H
#include "gucefAUDIO_CRealFft.h"
#define GUCEF_AUDIO_CREALFFT_H
#endif /* GUCEF_AUDIO_CREALFFT_H ? */

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
 *  Spectral suppression of noise and of residual echo in one mono signal: a DSP block, the core of the effect
 *  CNoiseSuppressorEffect and the residual echo suppressor of CAcousticEchoCanceller.
 *
 *  Short-time Fourier transform: frames of N samples with a square root Hann window for analysis and synthesis and a
 *  hop of N/2, so the windows add up to exactly 1 (overlap-add). N is the smallest power of two that covers 5.3 ms
 *  (256 at 44.1 and 48 kHz, 128 at 16 to 24 kHz, 64 at 8 kHz). The output is the input delayed by GetLatencyInFrames()
 *  = N frames; what the frames are cut into does not matter: the hops are aligned to the count of frames processed.
 *
 *  Per frame and bin:
 *   - the noise power: minimum statistics (Martin, 2001): the power smoothed over SmoothingSeconds, its minimum over
 *     1.5 s in six sub-windows of 0.25 s, times a bias compensation
 *   - the interference: the noise power times 'noiseStrength' when noise is suppressed, plus the residual echo power
 *     given by the owner (SetInterference()) times 'echoStrength'
 *   - a priori signal to interference ratio by the decision-directed estimator (Ephraim and Malah, 1984) with alpha 0.98,
 *     Wiener gain G = xi / ( 1 + xi )
 *   - a floor: the gains of noise and echo ('noiseFloorGain', 'echoFloorGain') weighted by their share of the
 *     interference, so the gain never goes below the floor of what is there (against musical noise); when noise is not
 *     suppressed, the gain also never takes the output below the noise power (a comfort floor that keeps the background
 *     of the near end while echo is suppressed)
 *   - smoothing over three bins (1/4, 1/2, 1/4) and over time (one pole of GainSmoothingSeconds); the bins below
 *     'bypassBelowHz' keep a gain of 1
 *  A frame needs no transform when nothing is to be suppressed (noise suppression off or its floor at 0 dB, and no echo
 *  power given) or when its input is digital silence: it is then windowed into the output without FFT, which is the
 *  same as a gain of 1, so the switch between the two is seamless. Only frames that need it cost two FFTs.
 *
 *  Single precision; the state is kept above 1e-20 (no denormals); not a number and infinite input count as 0 for the
 *  estimates. Prepare() allocates; nothing else allocates, locks or logs, so Process() is fit for the audio and capture
 *  threads. One thread uses one object.
 */
class GUCEF_AUDIO_PUBLIC_CPP CSpectralSuppressor
{
    public:

    /** The time constant of the power smoothing of the noise tracker */
    static const Float64 SmoothingSeconds;

    /** The time constant of the smoothing of the gains over time */
    static const Float64 GainSmoothingSeconds;

    /** The window of the minimum statistics, and the number of its sub-windows */
    static const Float64 MinimumWindowSeconds;
    enum { MINIMUM_SUBWINDOWS = 6 };

    /** The bias compensation of the minimum statistics (the minimum of the smoothed power against the mean noise power) */
    static const Float32 MinimumBias;

    CSpectralSuppressor( void );

    ~CSpectralSuppressor();

    /** Owner: allocates for the rate (8000 to 192000 Hz) and clears the state. False for another rate */
    bool Prepare( UInt32 sampleRate );

    /** Owner: frees the memory */
    void Release( void );

    bool IsPrepared( void ) const;
    UInt32 GetSampleRate( void ) const;

    /** N, the frame of the transform (0 before Prepare()) */
    UInt32 GetFrameSize( void ) const;

    /** The hop between frames, N/2 */
    UInt32 GetHopSize( void ) const;

    /** N/2 + 1 */
    UInt32 GetBinCount( void ) const;

    /** The output is the input delayed by this many frames: N */
    UInt32 GetLatencyInFrames( void ) const;

    /** The frame size the class picks for a rate (0 for a rate it does not support) */
    static UInt32 FrameSizeForRate( UInt32 sampleRate );

    /** Clears the state, as just after Prepare(). No allocation */
    void Reset( void );

    /** Suppress noise (true) or only the interference of SetInterference() with the comfort floor (false). Default true */
    void SetNoiseSuppression( bool suppressNoise );
    bool GetNoiseSuppression( void ) const;

    /** The lowest gain where noise dominates, linear 0..1 (1: noise is not suppressed). Default -15 dB */
    void SetNoiseFloorGain( Float32 gain );
    Float32 GetNoiseFloorGain( void ) const;

    /** The lowest gain where the echo power of SetInterference() dominates, linear 0..1. Default -40 dB */
    void SetEchoFloorGain( Float32 gain );
    Float32 GetEchoFloorGain( void ) const;

    /** Factor on the noise power estimate (over-subtraction), 0.25 to 8. Default 1 */
    void SetNoiseStrength( Float32 strength );
    Float32 GetNoiseStrength( void ) const;

    /** Factor on the echo power of SetInterference(), 0.25 to 8. Default 1 */
    void SetEchoStrength( Float32 strength );
    Float32 GetEchoStrength( void ) const;

    /** Bins below this frequency keep a gain of 1. Default 0 */
    void SetBypassBelowHz( Float32 frequency );
    Float32 GetBypassBelowHz( void ) const;

    /**
     *  The residual echo power per bin (GetBinCount() values, in the power units of this transform: |X|^2 of an FFT of N
     *  samples under the square root Hann window), read at every frame: the owner updates the memory between frames, on
     *  the same thread. Null: none (the default). An all-zero array costs nothing
     */
    void SetInterference( const Float32* powerPerBin );

    /** Processes mono audio in place, delayed by GetLatencyInFrames(). Unprepared: the audio passes unchanged */
    void Process( Float32* inOut, UInt32 frames );

    /** The noise power estimate of a bin, in the power units of SetInterference() */
    Float32 GetNoisePower( UInt32 bin ) const;

    /** The gain applied to a bin in the last frame that was transformed */
    Float32 GetGain( UInt32 bin ) const;

    /** The gain over all bins in the last frame that was transformed, weighted by the power of the input, in dB (0 before any) */
    Float32 GetMeanGainInDecibels( void ) const;

    /** Frames transformed (two FFTs each) and frames that passed without a transform, since Prepare() or Reset() */
    UInt64 GetTransformedFrameCount( void ) const;
    UInt64 GetBypassedFrameCount( void ) const;

    /** Frames processed since Prepare() or Reset(): the position the hops are aligned to */
    UInt64 GetProcessedFrameCount( void ) const;

    private:

    CSpectralSuppressor( const CSpectralSuppressor& src );              /**< not implemented */
    CSpectralSuppressor& operator=( const CSpectralSuppressor& src );   /**< not implemented */

    void ProcessFrame( void );
    bool NeedsTransform( bool silent ) const;
    void TransformFrame( void );
    void UpdateNoise( void );
    void ComputeGains( void );

    private:

    UInt32 m_sampleRate;
    UInt32 m_size;
    UInt32 m_hop;
    UInt32 m_bins;
    CRealFft m_fft;

    // settings
    bool m_suppressNoise;
    Float32 m_noiseFloorGain;
    Float32 m_echoFloorGain;
    Float32 m_noiseStrength;
    Float32 m_echoStrength;
    Float32 m_bypassBelowHz;
    UInt32 m_bypassBins;
    const Float32* m_interference;

    // derived at Prepare()
    Float32 m_smoothing;                        /**< per frame coefficient of the power smoothing */
    Float32 m_gainSmoothing;                    /**< per frame coefficient of the gain smoothing */
    UInt32 m_subwindowFrames;

    // state
    GUCEF::vector< Float32 > m_window;          /**< square root Hann, N */
    GUCEF::vector< Float32 > m_hann;            /**< analysis times synthesis window, N */
    GUCEF::vector< Float32 > m_input;           /**< the last N input samples */
    GUCEF::vector< Float32 > m_overlap;         /**< overlap-add accumulator, N */
    GUCEF::vector< Float32 > m_output;          /**< the output of the current hop, N/2 */
    GUCEF::vector< Float32 > m_work;            /**< N + 2 */
    GUCEF::vector< Float32 > m_power;           /**< |X|^2 of the current frame per bin */
    GUCEF::vector< Float32 > m_smoothed;        /**< smoothed power per bin */
    GUCEF::vector< Float32 > m_noise;           /**< noise power estimate per bin */
    GUCEF::vector< Float32 > m_subwindowMin;    /**< minimum of the current sub-window per bin */
    GUCEF::vector< Float32 > m_minima;          /**< MINIMUM_SUBWINDOWS rows of the minima of the sub-windows before */
    GUCEF::vector< Float32 > m_storedMinimum;   /**< the minimum over those rows per bin */
    GUCEF::vector< Float32 > m_previousClean;   /**< G^2 |X|^2 of the frame before, for the decision-directed estimate */
    GUCEF::vector< Float32 > m_rawGain;
    GUCEF::vector< Float32 > m_gain;            /**< smoothed gain per bin, the one applied */
    UInt32 m_fill;                              /**< input samples of the current hop */
    UInt32 m_subwindowCount;                    /**< frames in the current sub-window */
    UInt32 m_subwindowIndex;                    /**< the row of m_minima the current sub-window goes to */
    bool m_noiseStarted;
    bool m_transformedBefore;                   /**< the frame before was transformed (the decision-directed state is valid) */
    Float32 m_meanGainDb;
    UInt64 m_transformed;
    UInt64 m_bypassed;
    UInt64 m_processed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CSPECTRALSUPPRESSOR_H ? */
