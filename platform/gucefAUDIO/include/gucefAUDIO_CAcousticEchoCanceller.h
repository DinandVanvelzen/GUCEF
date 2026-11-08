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

#ifndef GUCEF_AUDIO_CACOUSTICECHOCANCELLER_H
#define GUCEF_AUDIO_CACOUSTICECHOCANCELLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_AUDIO_ETYPES_H
#include "gucefAUDIO_ETypes.h"
#define GUCEF_AUDIO_ETYPES_H
#endif /* GUCEF_AUDIO_ETYPES_H ? */

#ifndef GUCEF_AUDIO_CATOMICFLOAT_H
#include "gucefAUDIO_CAtomicFloat.h"
#define GUCEF_AUDIO_CATOMICFLOAT_H
#endif /* GUCEF_AUDIO_CATOMICFLOAT_H ? */

#ifndef GUCEF_AUDIO_CAUDIOFRAMERING_H
#include "gucefAUDIO_CAudioFrameRing.h"
#define GUCEF_AUDIO_CAUDIOFRAMERING_H
#endif /* GUCEF_AUDIO_CAUDIOFRAMERING_H ? */

#ifndef GUCEF_AUDIO_CECHOCANCELLERSETTINGS_H
#include "gucefAUDIO_CEchoCancellerSettings.h"
#define GUCEF_AUDIO_CECHOCANCELLERSETTINGS_H
#endif /* GUCEF_AUDIO_CECHOCANCELLERSETTINGS_H ? */

#ifndef GUCEF_AUDIO_CECHODELAYESTIMATOR_H
#include "gucefAUDIO_CEchoDelayEstimator.h"
#define GUCEF_AUDIO_CECHODELAYESTIMATOR_H
#endif /* GUCEF_AUDIO_CECHODELAYESTIMATOR_H ? */

#ifndef GUCEF_AUDIO_CFIRDECIMATOR_H
#include "gucefAUDIO_CFirDecimator.h"
#define GUCEF_AUDIO_CFIRDECIMATOR_H
#endif /* GUCEF_AUDIO_CFIRDECIMATOR_H ? */

#ifndef GUCEF_AUDIO_CREALFFT_H
#include "gucefAUDIO_CRealFft.h"
#define GUCEF_AUDIO_CREALFFT_H
#endif /* GUCEF_AUDIO_CREALFFT_H ? */

#ifndef GUCEF_AUDIO_CSINCRESAMPLER_H
#include "gucefAUDIO_CSincResampler.h"
#define GUCEF_AUDIO_CSINCRESAMPLER_H
#endif /* GUCEF_AUDIO_CSINCRESAMPLER_H ? */

#ifndef GUCEF_AUDIO_CSPECTRALSUPPRESSOR_H
#include "gucefAUDIO_CSpectralSuppressor.h"
#define GUCEF_AUDIO_CSPECTRALSUPPRESSOR_H
#endif /* GUCEF_AUDIO_CSPECTRALSUPPRESSOR_H ? */

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
 *  Acoustic echo canceller for a unit whose microphone hears its own speaker (stage 2 of the echo handling, section 4.13
 *  of the intercom design): the echo stage of the capture path (CAudioEngine::SetEchoCanceller()), between the effect
 *  stages AUDIOCAPTURESTAGE_PRE_ECHO and AUDIOCAPTURESTAGE_POST_ECHO.
 *
 *  Reference (render thread, WriteReference()): what the engine plays, after the master limiter, mixed down to mono,
 *  brought to the processing rate of the capture path (a CFirDecimator for a whole ratio such as 48 to 16 kHz, a
 *  CSincResampler otherwise) and written into a CAudioFrameRing: the render tap.
 *
 *  Capture thread (Process()), in blocks of B frames aligned to the absolute frame position of the capture path:
 *   - alignment: the reference is read one frame per microphone frame, kept a little more than a block ahead of the
 *     microphone (the largest blocks of both sides plus a block, or the margin of the settings), so the echo estimate of
 *     a block is known before its first microphone frame: the filter adds no latency. When the playback and capture
 *     clocks differ the reference level drifts: its minimum over every second gives the slope, the reference is then read
 *     at that ratio through an 8 tap Lagrange interpolator (no cost while the clocks agree). Gaps of either stream
 *     (dropped reference, a capture overrun) start the alignment over
 *   - bulk delay: CEchoDelayEstimator finds the delay of the echo; the filter is moved to it in whole blocks (its
 *     partitions shifted, nothing learnt is lost), so it only covers the room
 *   - the adaptive filter: partitioned block frequency-domain (multidelay) filter after Soo and Pang (1990), P partitions
 *     of B (filterLengthMs), FFT of 2B with overlap-save, the gradient constrained in partition 0 and one more partition
 *     per block in turn (alternately updated MDF), a proportionate step per partition, the step per bin normalised by the
 *     reference power and set from the estimated residual echo against the error (Valin, 2007: the leakage of the echo
 *     estimate into the error, found by regression of their power fluctuations). Double talk: the coherence of the
 *     microphone and the echo estimate (smoothed cross spectra over 300 to 3500 Hz) below 0.6 while the microphone
 *     carries twice the energy of the estimate; it freezes the adaptation and the leakage regression for up to 1.5 s at a
 *     time (longer, it is taken for a louder new echo path and the filter adapts again). Instead of a second (background)
 *     filter, a snapshot of the filter is kept once a second while it is converged and restored when the output energy
 *     grows to twice the microphone's (divergence); this costs a copy instead of a second filter output per block
 *   - residual echo suppression (residualSuppression): a CSpectralSuppressor on the output, whose interference is the
 *     residual echo power (the leakage times the echo estimate's spectrum), floor echoSuppressionDb, with the comfort floor
 *     of the near end's noise; it may also suppress noise (suppressNoise). It delays the output by its frame
 *  Nothing is computed while the far end has been quiet for the length of the filter (state IDLE: no FFT, the audio passes
 *  and the residual suppressor passes without a transform); switched off (SetEnabled( false ) or the settings) the
 *  microphone passes bit for bit and neither side computes anything.
 *
 *  Threads: SetSettings(), Prepare(), PrepareReference(), Release() and Reset() belong to the owner while neither the
 *  render nor the capture side uses the canceller. WriteReference() is the render thread's, Process() the capture
 *  thread's; neither allocates, locks or logs. SetEnabled() and every getter of the state and the statistics work from
 *  any thread (atomics, values of the last block).
 */
class GUCEF_AUDIO_PUBLIC_CPP CAcousticEchoCanceller
{
    public:

    explicit CAcousticEchoCanceller( const CEchoCancellerSettings& settings = CEchoCancellerSettings() );

    ~CAcousticEchoCanceller();

    /** Owner: the settings; their enabled switch becomes the current one. Take effect at Prepare() */
    void SetSettings( const CEchoCancellerSettings& settings );
    const CEchoCancellerSettings& GetSettings( void ) const;

    /**
     *  Owner: prepares for the processing rate of the capture path (8000 to 48000 Hz) and blocks of up to
     *  'maxCaptureBlockFrames', and for the reference (PrepareReference()). Allocates. False, unprepared, for a rate out of
     *  range or a reference that cannot be converted
     */
    bool Prepare( UInt32 processingRate, UInt32 maxCaptureBlockFrames, UInt32 referenceRate, UInt32 referenceChannels, UInt32 maxReferenceBlockFrames );

    /**
     *  Owner, while the render side does not use the canceller: the format of what WriteReference() gets (a device
     *  reopen of the engine changes it). Allocates. The capture side may go on: the reference has a gap it notices
     */
    bool PrepareReference( UInt32 referenceRate, UInt32 referenceChannels, UInt32 maxReferenceBlockFrames );

    void Release( void );

    bool IsPrepared( void ) const;
    UInt32 GetSampleRate( void ) const;
    UInt32 GetReferenceRate( void ) const;
    UInt32 GetReferenceChannels( void ) const;

    /** B: frames per block and partition */
    UInt32 GetBlockFrames( void ) const;

    /** P: partitions of the adaptive filter */
    UInt32 GetPartitionCount( void ) const;

    /** The delay of the output against the microphone: the frame of the residual echo suppressor, 0 without it */
    UInt32 GetLatencyInFrames( void ) const;

    /** Owner, while neither side runs: the filter forgotten, the alignment to be found again */
    void Reset( void );

    /**
     *  Render thread: a block of what is played, interleaved Float32 in the reference format of PrepareReference(). Does
     *  nothing while switched off or unprepared
     */
    void WriteReference( const Float32* interleaved, UInt32 frames ) GUCEF_AUDIO_REALTIME_CONTEXT;

    /**
     *  Capture thread: removes the echo from a block of the microphone (mono, at the processing rate, in place) whose first
     *  frame has the absolute position 'firstFrame' (CAudioCapturePath's count; a jump is a gap)
     */
    void Process( Float32* mono, UInt32 frames, UInt64 firstFrame ) GUCEF_AUDIO_REALTIME_CONTEXT;

    /** Any thread: on or off. Off, the microphone passes bit for bit and nothing is computed; back on, the alignment is found again */
    void SetEnabled( bool enabled );
    bool IsEnabled( void ) const;

    /** Any thread: the state after the last block */
    TEchoCancellerState GetState( void ) const;

    /** Any thread: converged (the echo return loss enhancement held above the converged level of the settings) */
    bool IsConverged( void ) const;

    /** Any thread: the coherence detector saw near-end speech over the echo in the last active block */
    bool IsDoubleTalk( void ) const;

    /** Any thread: the far end was active in the last block */
    bool IsFarEndActive( void ) const;

    /** Any thread: echo return loss enhancement of the adaptive filter (microphone against its output, while only the far end is active), dB */
    Float32 GetErleInDecibels( void ) const;

    /** Any thread: the gain of the residual echo suppressor in its last transformed frame, power weighted, dB (0 without one) */
    Float32 GetResidualSuppressionInDecibels( void ) const;

    /** Any thread: the leakage of the echo estimate into the output (Valin's estimate), 0.005 to 1 */
    Float32 GetLeakage( void ) const;

    /** Any thread: the bulk delay the filter starts at, frames at the processing rate, and the estimate of the delay of the echo (0 without one) */
    UInt32 GetBulkDelayInFrames( void ) const;
    UInt32 GetEstimatedDelayInFrames( void ) const;
    bool HasDelayEstimate( void ) const;

    /** Any thread: the reference read rate against the microphone, ( ratio - 1 ) in ppm, and whether it is resampled */
    Float32 GetDriftInPpm( void ) const;
    bool IsDriftCompensating( void ) const;

    /** Any thread: frames of reference ahead of the microphone at the start of the last block */
    UInt32 GetReferenceLevelInFrames( void ) const;

    /** Any thread: counters since Prepare() or Reset() */
    UInt64 GetCaptureCallCount( void ) const;       /**< calls of Process() */
    UInt64 GetRenderCallCount( void ) const;        /**< calls of WriteReference() */
    UInt64 GetBlockCount( void ) const;             /**< blocks with an aligned reference */
    UInt64 GetIdleBlockCount( void ) const;         /**< of those, blocks in which nothing was computed */
    UInt64 GetAdaptedBlockCount( void ) const;      /**< blocks in which the filter adapted */
    UInt64 GetTransformCount( void ) const;         /**< FFTs of the filter and of the residual suppressor */
    UInt64 GetResyncCount( void ) const;            /**< alignments found (the first one included) */
    UInt64 GetReferenceUnderrunCount( void ) const; /**< blocks whose reference was not all there */
    UInt64 GetDivergenceCount( void ) const;        /**< divergences caught (filter restored or cleared) */
    UInt64 GetDelayChangeCount( void ) const;       /**< moves of the filter to a new bulk delay */

    private:

    CAcousticEchoCanceller( const CAcousticEchoCanceller& src );                /**< not implemented */
    CAcousticEchoCanceller& operator=( const CAcousticEchoCanceller& src );     /**< not implemented */

    void ClearCaptureState( void );
    bool TryResync( void );
    void StartBlock( void );
    void EndBlock( void );
    Float32 ReadReferenceBlock( Float32* destination );
    void ReadRing( Float32* destination, UInt32 count );
    void UpdateDrift( UInt32 level );
    void ComputeEchoEstimate( void );
    void Adapt( bool farActive );
    void UpdateProportions( void );
    void Constrain( UInt32 partition );
    void ShiftFilter( Int32 blocks );
    void MapResidualEcho( void );
    void ClearResidualEcho( void );
    void Publish( void );
    Float32* PartitionX( UInt32 age );
    Float32* PartitionW( UInt32 partition );

    private:

    CEchoCancellerSettings m_settings;

    // sizes, from Prepare()
    UInt32 m_rate;
    UInt32 m_block;                         /**< B */
    UInt32 m_bins;                          /**< B + 1 */
    UInt32 m_spectrum;                      /**< values of one spectrum: 2B + 2 */
    UInt32 m_partitions;                    /**< P */
    UInt32 m_maxShift;                      /**< blocks the filter may be moved by */
    UInt32 m_history;                       /**< reference spectra kept: P + m_maxShift */
    Float32 m_farThreshold;                 /**< linear peak */
    Float32 m_regularisation;               /**< added to the reference power per bin */

    // render side
    UInt32 m_referenceRate;
    UInt32 m_referenceChannels;
    UInt32 m_maxReferenceBlock;
    CFirDecimator m_decimator;
    CSincResampler m_resampler;
    bool m_useResampler;
    GUCEF::vector< Float32 > m_referenceMono;
    GUCEF::vector< Float32 > m_referenceConverted;
    CAudioFrameRing m_ring;

    // capture side
    CRealFft m_fft;
    CEchoDelayEstimator m_delayEstimator;
    CSpectralSuppressor m_residual;
    GUCEF::vector< Float32 > m_x;           /**< the current reference block */
    GUCEF::vector< Float32 > m_xPrevious;
    GUCEF::vector< Float32 > m_d;           /**< the microphone of the block */
    GUCEF::vector< Float32 > m_y;           /**< the echo estimate of the block */
    GUCEF::vector< Float32 > m_e;           /**< the error (output of the filter) of the block */
    GUCEF::vector< Float32 > m_X;           /**< m_history spectra of the reference, newest at m_xHead */
    GUCEF::vector< Float32 > m_W;           /**< P spectra of the filter */
    GUCEF::vector< Float32 > m_snapshot;    /**< a filter that was good */
    GUCEF::vector< Float32 > m_work;
    GUCEF::vector< Float32 > m_Y;           /**< spectrum of the echo estimate block */
    GUCEF::vector< Float32 > m_E;           /**< spectrum of the error block */
    GUCEF::vector< Float32 > m_referencePower;  /**< smoothed |X|^2 per bin */
    GUCEF::vector< Float32 > m_echoPower;       /**< |Y|^2 per bin of the block */
    GUCEF::vector< Float32 > m_errorPower;      /**< |E|^2 per bin of the block */
    GUCEF::vector< Float32 > m_errorMean;       /**< smoothed |E|^2, for the leakage regression */
    GUCEF::vector< Float32 > m_echoMean;        /**< smoothed |Y|^2 */
    GUCEF::vector< Float32 > m_step;            /**< the step per bin of the block */
    GUCEF::vector< Float32 > m_crossRe;         /**< coherence: smoothed D Y* */
    GUCEF::vector< Float32 > m_crossIm;
    GUCEF::vector< Float32 > m_micPower;        /**< smoothed |D|^2 */
    GUCEF::vector< Float32 > m_estimatePower;   /**< smoothed |Y|^2 */
    GUCEF::vector< Float32 > m_proportion;      /**< per partition */
    GUCEF::vector< Float32 > m_residualEcho;    /**< per bin of the residual suppressor */
    GUCEF::vector< Float32 > m_lagrange;        /**< the 8 tap interpolator, LagrangePhases + 1 rows */
    GUCEF::vector< Float32 > m_local;           /**< reference frames around the read position while resampling */
    UInt32 m_xHead;
    UInt32 m_shift;                         /**< blocks the filter starts after the aligned reference */
    UInt32 m_inBlock;                       /**< frames of the current block done */
    bool m_blockValid;                      /**< the current block has an aligned reference */
    bool m_blockIdle;
    bool m_blockFarActive;
    bool m_wasIdle;
    bool m_synced;
    bool m_needResync;
    bool m_wasEnabled;
    UInt64 m_expectedFrame;
    UInt64 m_droppedSeen;
    UInt32 m_debt;                          /**< reference frames to skip once they arrive (after an underrun) */
    UInt32 m_silentBlocks;
    UInt32 m_constrainNext;                 /**< the partition constrained next, besides partition 0 */
    UInt32 m_blockCounter;
    UInt32 m_maxCaptureChunk;
    bool m_adapted;
    Float32 m_adaptSum;
    UInt32 m_powerBlocks;                   /**< blocks the reference power was averaged over, up to 1000 */
    Float32 m_leakage;
    Float32 m_leakCross;                    /**< the regression of the leakage, smoothed */
    Float32 m_leakVariance;
    Float32 m_micSum;                       /**< ERLE: smoothed microphone and error energy of far-only blocks */
    Float32 m_errorSum;
    Float32 m_erleDb;
    UInt32 m_aboveCount;
    UInt32 m_belowCount;
    bool m_converged;
    bool m_doubleTalk;
    UInt32 m_doubleTalkBlocks;              /**< blocks in a row taken for double talk */
    UInt32 m_divergentBlocks;               /**< far-active blocks since the last divergence */
    Float32 m_divergenceMic;                /**< smoothed energies of the divergence check */
    Float32 m_divergenceError;
    bool m_hasSnapshot;
    UInt32 m_blocksSinceSnapshot;

    // drift
    bool m_compensating;
    Float64 m_ratio;                        /**< reference frames per microphone frame */
    Float64 m_position;                     /**< read position in m_local */
    UInt32 m_localCount;
    UInt32 m_windowBlocks;                  /**< blocks per window of the level minimum */
    UInt32 m_windowCount;
    UInt32 m_windowMin;
    GUCEF::vector< Float32 > m_minima;      /**< level minima of the last windows */
    UInt32 m_minimaCount;
    Float32 m_tail[ 4 ];                    /**< the last frames read directly, for the start of the interpolation */

    // counters (capture side)
    UInt64 m_blocks;
    UInt64 m_idleBlocks;
    UInt64 m_adaptedBlocks;
    UInt64 m_transforms;
    UInt64 m_resyncs;
    UInt64 m_underruns;
    UInt64 m_divergences;
    UInt64 m_delayChanges;
    UInt64 m_captureCalls;
    UInt32 m_level;

    // any thread
    MT::CTAtomic< bool > m_enabled;
    MT::CTAtomic< UInt32 > m_publishedState;
    MT::CTAtomic< bool > m_publishedConverged;
    MT::CTAtomic< bool > m_publishedDoubleTalk;
    MT::CTAtomic< bool > m_publishedFarActive;
    MT::CTAtomic< bool > m_publishedCompensating;
    MT::CTAtomic< bool > m_publishedHasDelay;
    MT::CTAtomic< UInt32 > m_publishedBulkDelay;
    MT::CTAtomic< UInt32 > m_publishedDelay;
    MT::CTAtomic< UInt32 > m_publishedLevel;
    MT::CTAtomic< UInt32 > m_maxRenderChunk;    /**< the largest block of reference written, at the processing rate */
    MT::CTAtomic< UInt64 > m_renderCalls;
    MT::CTAtomic< UInt64 > m_publishedCaptureCalls;
    MT::CTAtomic< UInt64 > m_publishedBlocks;
    MT::CTAtomic< UInt64 > m_publishedIdleBlocks;
    MT::CTAtomic< UInt64 > m_publishedAdaptedBlocks;
    MT::CTAtomic< UInt64 > m_publishedTransforms;
    MT::CTAtomic< UInt64 > m_publishedResyncs;
    MT::CTAtomic< UInt64 > m_publishedUnderruns;
    MT::CTAtomic< UInt64 > m_publishedDivergences;
    MT::CTAtomic< UInt64 > m_publishedDelayChanges;
    CAtomicFloat m_publishedErle;
    CAtomicFloat m_publishedResidualGain;
    CAtomicFloat m_publishedLeakage;
    CAtomicFloat m_publishedDrift;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CACOUSTICECHOCANCELLER_H ? */
