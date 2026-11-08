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
#include <string.h>

#include "gucefAUDIO_CAcousticEchoCanceller.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Phases of the table of the 8 tap Lagrange interpolator of the drift compensation */
const UInt32 LagrangePhases = 512;
const UInt32 LagrangeTaps = 8;

/** Level minima kept for the drift slope: one per second */
const UInt32 DriftWindows = 16;

/** The fewest minima the slope is taken from */
const UInt32 DriftMinWindows = 4;

/** A drift below this is taken for none while the reference is read directly */
const Float64 DriftEngage = 8.0e-6;

/** The largest ratio correction */
const Float64 DriftLimit = 0.002;

/** Valin's leakage is kept within these (33 dB is as good as it is expected to get) */
const Float32 MinLeakage = 0.005f;

/** The filter counts as adapted enough for the step control by the residual echo after this */
const Float32 AdaptedLeakage = 0.03f;

/** Coherence of microphone and echo estimate below which, with the far end active, the near end talks */
const Float32 DoubleTalkCoherence = 0.6f;

/** The most blocks in a row taken for double talk (about 1.5 s at 16 kHz): longer, the filter adapts again */
const UInt32 MaxDoubleTalkBlocks = 190;

/** Smoothing per block of the coherence spectra */
const Float32 CoherenceSmoothing = 0.8f;

/** Smoothing per block of the energies of the ERLE estimate */
const Float32 ErleSmoothing = 0.9f;

/** Blocks the ERLE must stay above (below) the converged level before the state changes */
const UInt32 ConvergenceBlocks = 25;

/**
 *  Divergence: the error energy above the microphone energy by this ratio, both smoothed over about DivergenceBlocks
 *  blocks of far-end activity, after at least that many blocks since the last check fired
 */
const Float32 DivergenceRatio = 2.0f;
const UInt32 DivergenceBlocks = 12;

const Float32 TinyPower = 1.0e-20f;

/** Gives the memory of a vector back, which clear() does not */
template< typename T >
void
FreeVector( GUCEF::vector< T >& v )
{
    GUCEF::vector< T > empty;
    v.swap( empty );
}

/** The power of two block nearest below 8 ms at the rate */
UInt32
AutomaticBlock( UInt32 rate )
{
    const Float64 wanted = 0.008 * (Float64) rate;
    UInt32 block = 32;
    while ( (Float64) ( block * 2 ) <= wanted + 1.0e-9 && block < 1024 )
        block *= 2;
    return block;
}

bool
IsFinite( Float32 value )
{
    return value - value == 0.0f;
}

/*
 *  The spectra of the filter are kept split (the real parts of the B + 1 bins, then the imaginary parts) so the products
 *  over the bins are plain element loops. Their pointers do not alias (restrict), so a compiler vectorises them without
 *  run time checks (SSE, AVX, NEON), also with the cheap cost model of -O2.
 */
#if defined( __GNUC__ ) || defined( __clang__ ) || defined( _MSC_VER )
  #define GUCEF_AUDIO_AEC_RESTRICT __restrict
#else
  #define GUCEF_AUDIO_AEC_RESTRICT
#endif

/** acc += a * b (complex, split) */
void
MultiplyAccumulate( Float32* GUCEF_AUDIO_AEC_RESTRICT accRe, Float32* GUCEF_AUDIO_AEC_RESTRICT accIm,
                    const Float32* GUCEF_AUDIO_AEC_RESTRICT aRe, const Float32* GUCEF_AUDIO_AEC_RESTRICT aIm,
                    const Float32* GUCEF_AUDIO_AEC_RESTRICT bRe, const Float32* GUCEF_AUDIO_AEC_RESTRICT bIm, UInt32 count )
{
    for ( UInt32 k=0; k<count; ++k )
    {
        accRe[ k ] += aRe[ k ] * bRe[ k ] - aIm[ k ] * bIm[ k ];
        accIm[ k ] += aRe[ k ] * bIm[ k ] + aIm[ k ] * bRe[ k ];
    }
}

/** w += g * step * conj( x ) * e (complex, split) */
void
AdaptPartition( Float32* GUCEF_AUDIO_AEC_RESTRICT wRe, Float32* GUCEF_AUDIO_AEC_RESTRICT wIm, const Float32* GUCEF_AUDIO_AEC_RESTRICT step, Float32 g,
                const Float32* GUCEF_AUDIO_AEC_RESTRICT xRe, const Float32* GUCEF_AUDIO_AEC_RESTRICT xIm,
                const Float32* GUCEF_AUDIO_AEC_RESTRICT eRe, const Float32* GUCEF_AUDIO_AEC_RESTRICT eIm, UInt32 count )
{
    for ( UInt32 k=0; k<count; ++k )
    {
        const Float32 s = g * step[ k ];
        wRe[ k ] += s * ( xRe[ k ] * eRe[ k ] + xIm[ k ] * eIm[ k ] );
        wIm[ k ] += s * ( xRe[ k ] * eIm[ k ] - xIm[ k ] * eRe[ k ] );
    }
}

/** Interleaved spectrum (re, im per bin, as CRealFft writes it) to split */
void
Deinterleave( const Float32* GUCEF_AUDIO_AEC_RESTRICT interleaved, Float32* GUCEF_AUDIO_AEC_RESTRICT split, UInt32 bins )
{
    Float32* GUCEF_AUDIO_AEC_RESTRICT im = split + bins;
    for ( UInt32 k=0; k<bins; ++k )
    {
        split[ k ] = interleaved[ 2 * k ];
        im[ k ] = interleaved[ 2 * k + 1 ];
    }
}

/** Split spectrum to interleaved */
void
Interleave( const Float32* GUCEF_AUDIO_AEC_RESTRICT split, Float32* GUCEF_AUDIO_AEC_RESTRICT interleaved, UInt32 bins )
{
    const Float32* GUCEF_AUDIO_AEC_RESTRICT im = split + bins;
    for ( UInt32 k=0; k<bins; ++k )
    {
        interleaved[ 2 * k ] = split[ k ];
        interleaved[ 2 * k + 1 ] = im[ k ];
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAcousticEchoCanceller::CAcousticEchoCanceller( const CEchoCancellerSettings& settings )
    : m_settings( settings )
    , m_rate( 0 )
    , m_block( 0 )
    , m_bins( 0 )
    , m_spectrum( 0 )
    , m_partitions( 0 )
    , m_maxShift( 0 )
    , m_history( 0 )
    , m_farThreshold( 0.0f )
    , m_regularisation( 0.0f )
    , m_referenceRate( 0 )
    , m_referenceChannels( 0 )
    , m_maxReferenceBlock( 0 )
    , m_decimator()
    , m_resampler()
    , m_useResampler( false )
    , m_referenceMono()
    , m_referenceConverted()
    , m_ring()
    , m_fft()
    , m_delayEstimator()
    , m_residual()
    , m_x()
    , m_xPrevious()
    , m_d()
    , m_y()
    , m_e()
    , m_X()
    , m_W()
    , m_snapshot()
    , m_work()
    , m_Y()
    , m_E()
    , m_referencePower()
    , m_echoPower()
    , m_errorPower()
    , m_errorMean()
    , m_echoMean()
    , m_step()
    , m_crossRe()
    , m_crossIm()
    , m_micPower()
    , m_estimatePower()
    , m_proportion()
    , m_residualEcho()
    , m_lagrange()
    , m_local()
    , m_xHead( 0 )
    , m_shift( 0 )
    , m_inBlock( 0 )
    , m_blockValid( false )
    , m_blockIdle( true )
    , m_blockFarActive( false )
    , m_wasIdle( true )
    , m_synced( false )
    , m_needResync( true )
    , m_wasEnabled( false )
    , m_expectedFrame( 0 )
    , m_droppedSeen( 0 )
    , m_debt( 0 )
    , m_silentBlocks( 0 )
    , m_constrainNext( 0 )
    , m_blockCounter( 0 )
    , m_maxCaptureChunk( 0 )
    , m_adapted( false )
    , m_adaptSum( 0.0f )
    , m_powerBlocks( 0 )
    , m_leakage( 1.0f )
    , m_leakCross( 0.0f )
    , m_leakVariance( 0.0f )
    , m_micSum( 0.0f )
    , m_errorSum( 0.0f )
    , m_erleDb( 0.0f )
    , m_aboveCount( 0 )
    , m_belowCount( 0 )
    , m_converged( false )
    , m_doubleTalk( false )
    , m_doubleTalkBlocks( 0 )
    , m_divergentBlocks( 0 )
    , m_divergenceMic( 0.0f )
    , m_divergenceError( 0.0f )
    , m_hasSnapshot( false )
    , m_blocksSinceSnapshot( 0 )
    , m_compensating( false )
    , m_ratio( 1.0 )
    , m_position( 0.0 )
    , m_localCount( 0 )
    , m_windowBlocks( 1 )
    , m_windowCount( 0 )
    , m_windowMin( 0xFFFFFFFFUL )
    , m_minima()
    , m_minimaCount( 0 )
    , m_blocks( 0 )
    , m_idleBlocks( 0 )
    , m_adaptedBlocks( 0 )
    , m_transforms( 0 )
    , m_resyncs( 0 )
    , m_underruns( 0 )
    , m_divergences( 0 )
    , m_delayChanges( 0 )
    , m_captureCalls( 0 )
    , m_level( 0 )
    , m_enabled( settings.GetEnabled() )
    , m_publishedState( ECHOCANCELLERSTATE_OFF )
    , m_publishedConverged( false )
    , m_publishedDoubleTalk( false )
    , m_publishedFarActive( false )
    , m_publishedCompensating( false )
    , m_publishedHasDelay( false )
    , m_publishedBulkDelay( 0 )
    , m_publishedDelay( 0 )
    , m_publishedLevel( 0 )
    , m_maxRenderChunk( 0 )
    , m_renderCalls( 0 )
    , m_publishedCaptureCalls( 0 )
    , m_publishedBlocks( 0 )
    , m_publishedIdleBlocks( 0 )
    , m_publishedAdaptedBlocks( 0 )
    , m_publishedTransforms( 0 )
    , m_publishedResyncs( 0 )
    , m_publishedUnderruns( 0 )
    , m_publishedDivergences( 0 )
    , m_publishedDelayChanges( 0 )
    , m_publishedErle( 0.0f )
    , m_publishedResidualGain( 0.0f )
    , m_publishedLeakage( 1.0f )
    , m_publishedDrift( 0.0f )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<4; ++i )
        m_tail[ i ] = 0.0f;
}

/*-------------------------------------------------------------------------*/

CAcousticEchoCanceller::~CAcousticEchoCanceller()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::SetSettings( const CEchoCancellerSettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
    m_enabled.Store( settings.GetEnabled() );
}

/*-------------------------------------------------------------------------*/

const CEchoCancellerSettings&
CAcousticEchoCanceller::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CAcousticEchoCanceller::Prepare( UInt32 processingRate, UInt32 maxCaptureBlockFrames, UInt32 referenceRate, UInt32 referenceChannels, UInt32 maxReferenceBlockFrames )
{GUCEF_TRACE;

    Release();
    if ( processingRate < 8000 || processingRate > 48000 || 0 == maxCaptureBlockFrames )
        return false;

    const UInt32 block = 0 != m_settings.GetBlockFrames() ? m_settings.GetBlockFrames() : AutomaticBlock( processingRate );
    if ( !m_fft.Prepare( 2 * block, true, false ) )
        return false;

    m_rate = processingRate;
    m_block = block;
    m_bins = block + 1;
    m_spectrum = 2 * block + 2;
    const Float64 framesPerMs = (Float64) processingRate / 1000.0;
    m_partitions = (UInt32) ceil( (Float64) m_settings.GetFilterLengthInMs() * framesPerMs / (Float64) block - 1.0e-9 );
    if ( m_partitions < 1 )
        m_partitions = 1;
    m_maxShift = m_settings.GetDelayEstimation() ? (UInt32) ceil( (Float64) m_settings.GetMaxDelayInMs() * framesPerMs / (Float64) block ) : 0;
    m_history = m_partitions + m_maxShift;
    m_farThreshold = (Float32) pow( 10.0, (Float64) m_settings.GetFarThresholdInDecibels() / 20.0 );
    // about -90 dBFS of white reference in one bin of the transform of 2B frames
    m_regularisation = (Float32) ( 2.0 * (Float64) block * 1.0e-9 );

    m_x.assign( block, 0.0f );
    m_xPrevious.assign( block, 0.0f );
    m_d.assign( block, 0.0f );
    m_y.assign( block, 0.0f );
    m_e.assign( block, 0.0f );
    m_X.assign( (size_t) m_history * m_spectrum, 0.0f );
    m_W.assign( (size_t) m_partitions * m_spectrum, 0.0f );
    m_snapshot.assign( (size_t) m_partitions * m_spectrum, 0.0f );
    m_work.assign( m_spectrum, 0.0f );
    m_Y.assign( m_spectrum, 0.0f );
    m_E.assign( m_spectrum, 0.0f );
    m_referencePower.assign( m_bins, 0.0f );
    m_echoPower.assign( m_bins, 0.0f );
    m_errorPower.assign( m_bins, 0.0f );
    m_errorMean.assign( m_bins, 0.0f );
    m_echoMean.assign( m_bins, 0.0f );
    m_step.assign( m_bins, 0.0f );
    m_crossRe.assign( m_bins, 0.0f );
    m_crossIm.assign( m_bins, 0.0f );
    m_micPower.assign( m_bins, 0.0f );
    m_estimatePower.assign( m_bins, 0.0f );
    m_proportion.assign( m_partitions, 0.99f / (Float32) m_partitions );
    m_local.assign( 2 * block + 32, 0.0f );
    m_minima.assign( DriftWindows, 0.0f );
    m_windowBlocks = (UInt32) floor( (Float64) processingRate / (Float64) block + 0.5 );
    if ( m_windowBlocks < 1 )
        m_windowBlocks = 1;

    // 8 tap Lagrange interpolation: taps at -3 .. +4 around the integer position, one row per fraction
    m_lagrange.assign( (size_t) ( LagrangePhases + 1 ) * LagrangeTaps, 0.0f );
    for ( UInt32 phase=0; phase<=LagrangePhases; ++phase )
    {
        const Float64 position = 3.0 + (Float64) phase / (Float64) LagrangePhases;
        for ( UInt32 k=0; k<LagrangeTaps; ++k )
        {
            Float64 c = 1.0;
            for ( UInt32 m=0; m<LagrangeTaps; ++m )
                if ( m != k )
                    c *= ( position - (Float64) m ) / ( (Float64) k - (Float64) m );
            m_lagrange[ (size_t) phase * LagrangeTaps + k ] = (Float32) c;
        }
    }

    if ( m_settings.GetDelayEstimation() )
    {
        const UInt32 maxDelay = (UInt32) ( (Float64) m_settings.GetMaxDelayInMs() * framesPerMs );
        if ( !m_delayEstimator.Prepare( processingRate, maxDelay ) )
        {
            Release();
            return false;
        }
    }

    if ( m_settings.GetResidualSuppression() )
    {
        if ( !m_residual.Prepare( processingRate ) )
        {
            Release();
            return false;
        }
        m_residual.SetNoiseSuppression( m_settings.GetSuppressNoise() );
        m_residual.SetNoiseFloorGain( m_settings.GetSuppressNoise() ? (Float32) pow( 10.0, (Float64) m_settings.GetNoiseSuppressionInDecibels() / 20.0 ) : 1.0f );
        m_residual.SetEchoFloorGain( (Float32) pow( 10.0, (Float64) m_settings.GetEchoSuppressionInDecibels() / 20.0 ) );
        m_residual.SetEchoStrength( m_settings.GetResidualStrength() );
        m_residual.SetBypassBelowHz( 0.0f );
        m_residualEcho.assign( m_residual.GetBinCount(), 0.0f );
        m_residual.SetInterference( &m_residualEcho[ 0 ] );
    }

    if ( !PrepareReference( referenceRate, referenceChannels, maxReferenceBlockFrames ) )
    {
        Release();
        return false;
    }

    // the ring holds the reference ahead of the microphone: a block, the largest blocks of both sides and room for the
    // jitter of the threads; the bulk delay itself lives in the reference spectra of the filter, not here
    const UInt32 converted = (UInt32) m_referenceConverted.size();
    if ( !m_ring.Prepare( 2 * block + converted + 2 * maxCaptureBlockFrames + processingRate / 5 ) )
    {
        Release();
        return false;
    }

    m_enabled.Store( m_settings.GetEnabled() );
    m_wasEnabled = false;
    m_captureCalls = 0;
    m_renderCalls.Store( 0 );
    m_maxCaptureChunk = 0;
    m_blocks = 0;
    m_idleBlocks = 0;
    m_adaptedBlocks = 0;
    m_transforms = 0;
    m_resyncs = 0;
    m_underruns = 0;
    m_divergences = 0;
    m_delayChanges = 0;
    m_expectedFrame = 0;
    m_inBlock = 0;
    m_synced = false;
    m_needResync = true;
    ClearCaptureState();
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAcousticEchoCanceller::PrepareReference( UInt32 referenceRate, UInt32 referenceChannels, UInt32 maxReferenceBlockFrames )
{GUCEF_TRACE;

    if ( 0 == m_rate || 0 == referenceRate || 0 == referenceChannels || 0 == maxReferenceBlockFrames )
        return false;

    m_decimator.Release();
    m_useResampler = false;
    UInt32 converted = 0;
    if ( referenceRate % m_rate == 0 && referenceRate / m_rate <= 16 )
    {
        // a whole ratio (48 to 16 kHz, or none): the cheap decimator
        if ( !m_decimator.Prepare( referenceRate / m_rate ) )
            return false;
        converted = maxReferenceBlockFrames / ( referenceRate / m_rate ) + 2;
    }
    else
    {
        if ( !m_resampler.Configure( 1, (Float64) referenceRate, (Float64) m_rate ) )
            return false;
        m_resampler.Reset();
        m_useResampler = true;
        converted = (UInt32) ceil( (Float64) maxReferenceBlockFrames * (Float64) m_rate / (Float64) referenceRate ) + 8;
    }
    m_referenceRate = referenceRate;
    m_referenceChannels = referenceChannels;
    m_maxReferenceBlock = maxReferenceBlockFrames;
    m_referenceMono.assign( referenceChannels > 1 ? maxReferenceBlockFrames : 0, 0.0f );
    if ( m_referenceConverted.size() < converted )
        m_referenceConverted.assign( converted, 0.0f );
    m_maxRenderChunk.Store( 0 );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::Release( void )
{GUCEF_TRACE;

    m_rate = 0;
    m_block = 0;
    m_bins = 0;
    m_spectrum = 0;
    m_partitions = 0;
    m_history = 0;
    m_referenceRate = 0;
    m_referenceChannels = 0;
    m_decimator.Release();
    m_ring.Release();
    m_delayEstimator.Release();
    m_residual.Release();
    FreeVector( m_referenceMono );
    FreeVector( m_referenceConverted );
    FreeVector( m_x );
    FreeVector( m_xPrevious );
    FreeVector( m_d );
    FreeVector( m_y );
    FreeVector( m_e );
    FreeVector( m_X );
    FreeVector( m_W );
    FreeVector( m_snapshot );
    FreeVector( m_work );
    FreeVector( m_Y );
    FreeVector( m_E );
    FreeVector( m_referencePower );
    FreeVector( m_echoPower );
    FreeVector( m_errorPower );
    FreeVector( m_errorMean );
    FreeVector( m_echoMean );
    FreeVector( m_step );
    FreeVector( m_crossRe );
    FreeVector( m_crossIm );
    FreeVector( m_micPower );
    FreeVector( m_estimatePower );
    FreeVector( m_proportion );
    FreeVector( m_residualEcho );
    FreeVector( m_lagrange );
    FreeVector( m_local );
    FreeVector( m_minima );
    m_publishedState.Store( ECHOCANCELLERSTATE_OFF );
}

/*-------------------------------------------------------------------------*/

bool
CAcousticEchoCanceller::IsPrepared( void ) const
{GUCEF_TRACE;

    return 0 != m_block;
}

/*-------------------------------------------------------------------------*/

UInt32
CAcousticEchoCanceller::GetSampleRate( void ) const
{GUCEF_TRACE;

    return m_rate;
}

/*-------------------------------------------------------------------------*/

UInt32
CAcousticEchoCanceller::GetReferenceRate( void ) const
{GUCEF_TRACE;

    return m_referenceRate;
}

/*-------------------------------------------------------------------------*/

UInt32
CAcousticEchoCanceller::GetReferenceChannels( void ) const
{GUCEF_TRACE;

    return m_referenceChannels;
}

/*-------------------------------------------------------------------------*/

UInt32
CAcousticEchoCanceller::GetBlockFrames( void ) const
{GUCEF_TRACE;

    return m_block;
}

/*-------------------------------------------------------------------------*/

UInt32
CAcousticEchoCanceller::GetPartitionCount( void ) const
{GUCEF_TRACE;

    return m_partitions;
}

/*-------------------------------------------------------------------------*/

UInt32
CAcousticEchoCanceller::GetLatencyInFrames( void ) const
{GUCEF_TRACE;

    return m_residual.IsPrepared() ? m_residual.GetLatencyInFrames() : 0;
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::Reset( void )
{GUCEF_TRACE;

    if ( 0 == m_block )
        return;
    m_ring.Reset();
    m_decimator.Reset();
    m_resampler.Reset();
    m_residual.Reset();
    m_maxRenderChunk.Store( 0 );
    m_renderCalls.Store( 0 );
    m_synced = false;
    m_needResync = true;
    m_inBlock = 0;
    m_captureCalls = 0;
    m_maxCaptureChunk = 0;
    m_blocks = 0;
    m_idleBlocks = 0;
    m_adaptedBlocks = 0;
    m_transforms = 0;
    m_resyncs = 0;
    m_underruns = 0;
    m_divergences = 0;
    m_delayChanges = 0;
    ClearCaptureState();
    Publish();
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::ClearCaptureState( void )
{
    // the filter and everything it learnt; the alignment is found again by the caller
    if ( 0 == m_block )
        return;
    memset( &m_X[ 0 ], 0, m_X.size() * sizeof( Float32 ) );
    memset( &m_W[ 0 ], 0, m_W.size() * sizeof( Float32 ) );
    memset( &m_xPrevious[ 0 ], 0, m_xPrevious.size() * sizeof( Float32 ) );
    memset( &m_y[ 0 ], 0, m_y.size() * sizeof( Float32 ) );
    for ( UInt32 k=0; k<m_bins; ++k )
    {
        m_referencePower[ k ] = 0.0f;
        m_echoPower[ k ] = 0.0f;
        m_errorPower[ k ] = 0.0f;
        m_errorMean[ k ] = 0.0f;
        m_echoMean[ k ] = 0.0f;
        m_step[ k ] = 0.0f;
        m_crossRe[ k ] = 0.0f;
        m_crossIm[ k ] = 0.0f;
        m_micPower[ k ] = 0.0f;
        m_estimatePower[ k ] = 0.0f;
    }
    for ( UInt32 p=0; p<m_partitions; ++p )
        m_proportion[ p ] = 0.99f / (Float32) m_partitions;
    ClearResidualEcho();
    m_delayEstimator.Reset();
    m_xHead = 0;
    m_shift = 0;
    m_blockValid = false;
    m_blockIdle = true;
    m_blockFarActive = false;
    m_wasIdle = true;
    m_debt = 0;
    m_silentBlocks = 0xFFFFFFFUL;
    m_constrainNext = 0;
    m_blockCounter = 0;
    m_adapted = false;
    m_adaptSum = 0.0f;
    m_powerBlocks = 0;
    m_leakage = 1.0f;
    m_leakCross = 0.0f;
    m_leakVariance = 0.0f;
    m_micSum = 0.0f;
    m_errorSum = 0.0f;
    m_erleDb = 0.0f;
    m_aboveCount = 0;
    m_belowCount = 0;
    m_converged = false;
    m_doubleTalk = false;
    m_doubleTalkBlocks = 0;
    m_divergentBlocks = 0;
    m_divergenceMic = 0.0f;
    m_divergenceError = 0.0f;
    m_hasSnapshot = false;
    m_blocksSinceSnapshot = 0;
    m_compensating = false;
    m_ratio = 1.0;
    m_position = 0.0;
    m_localCount = 0;
    m_windowCount = 0;
    m_windowMin = 0xFFFFFFFFUL;
    m_minimaCount = 0;
    m_level = 0;
    for ( UInt32 i=0; i<4; ++i )
        m_tail[ i ] = 0.0f;
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::SetEnabled( bool enabled )
{GUCEF_TRACE;

    m_enabled.Store( enabled );
}

/*-------------------------------------------------------------------------*/

bool
CAcousticEchoCanceller::IsEnabled( void ) const
{
    // no trace macro: the real time threads use it
    return m_enabled.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::WriteReference( const Float32* interleaved, UInt32 frames ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    m_renderCalls.FetchAdd( 1, MT::MEMORDER_RELAXED );
    if ( 0 == m_referenceChannels || GUCEF_NULL == interleaved || !m_enabled.Load( MT::MEMORDER_RELAXED ) )
        return;

    const UInt32 channels = m_referenceChannels;
    while ( frames > 0 )
    {
        const UInt32 chunk = frames < m_maxReferenceBlock ? frames : m_maxReferenceBlock;

        // mono: the mean of the channels
        const Float32* mono = interleaved;
        if ( channels > 1 )
        {
            Float32* out = &m_referenceMono[ 0 ];
            const Float32 scale = 1.0f / (Float32) channels;
            const Float32* in = interleaved;
            for ( UInt32 f=0; f<chunk; ++f )
            {
                Float32 sum = 0.0f;
                for ( UInt32 c=0; c<channels; ++c )
                    sum += in[ c ];
                in += channels;
                out[ f ] = sum * scale;
            }
            mono = out;
        }

        // to the processing rate
        Float32* converted = &m_referenceConverted[ 0 ];
        const UInt32 capacity = (UInt32) m_referenceConverted.size();
        UInt32 produced = 0;
        if ( !m_useResampler )
        {
            produced = m_decimator.Process( mono, chunk, converted );
        }
        else
        {
            UInt32 offset = 0;
            while ( offset < chunk && produced < capacity )
            {
                UInt32 consumed = 0, made = 0;
                m_resampler.Process( mono + offset, chunk - offset, consumed, converted + produced, capacity - produced, made );
                offset += consumed;
                produced += made;
                if ( 0 == consumed && 0 == made )
                    break;
            }
        }

        if ( produced > 0 )
        {
            m_ring.Write( converted, produced );
            if ( produced > m_maxRenderChunk.Load( MT::MEMORDER_RELAXED ) )
                m_maxRenderChunk.Store( produced, MT::MEMORDER_RELAXED );
        }
        interleaved += (size_t) chunk * channels;
        frames -= chunk;
    }
}

/*-------------------------------------------------------------------------*/

Float32*
CAcousticEchoCanceller::PartitionX( UInt32 age )
{
    const UInt32 index = ( m_xHead + m_history - ( age % m_history ) ) % m_history;
    return &m_X[ (size_t) index * m_spectrum ];
}

/*-------------------------------------------------------------------------*/

Float32*
CAcousticEchoCanceller::PartitionW( UInt32 partition )
{
    return &m_W[ (size_t) partition * m_spectrum ];
}

/*-------------------------------------------------------------------------*/

bool
CAcousticEchoCanceller::TryResync( void )
{
    // the largest block the render side wrote: until there is one, nothing plays and there is nothing to align to
    const UInt32 renderChunk = m_maxRenderChunk.Load( MT::MEMORDER_RELAXED );
    if ( 0 == renderChunk )
        return false;

    UInt32 margin = 0;
    if ( m_settings.GetReferenceMarginInMs() > 0.0f )
        margin = (UInt32) ( (Float64) m_settings.GetReferenceMarginInMs() * (Float64) m_rate / 1000.0 );
    else
        margin = renderChunk + m_maxCaptureChunk;
    const UInt32 target = m_block + margin;
    const UInt32 available = m_ring.GetAvailable();
    if ( available < target )
        return false;

    m_ring.Skip( available - target );
    m_droppedSeen = m_ring.GetDroppedFrameCount();
    ClearCaptureState();
    m_synced = true;
    m_needResync = false;
    ++m_resyncs;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::ReadRing( Float32* destination, UInt32 count )
{
    if ( m_debt > 0 )
        m_debt -= m_ring.Skip( m_debt );
    const UInt32 got = ( 0 == m_debt ) ? m_ring.Read( destination, count ) : 0;
    if ( got < count )
    {
        // not there yet: silence in its place, and the frames are skipped when they come, so the alignment holds
        memset( destination + got, 0, (size_t) ( count - got ) * sizeof( Float32 ) );
        m_debt += count - got;
        ++m_underruns;
        if ( m_debt > m_block / 2 )
            m_needResync = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::UpdateDrift( UInt32 level )
{
    if ( level < m_windowMin )
        m_windowMin = level;
    if ( ++m_windowCount < m_windowBlocks )
        return;

    // a window is complete: its minimum level (the bottom of the saw tooth of the render and capture blocks) goes in
    if ( m_minimaCount == DriftWindows )
    {
        memmove( &m_minima[ 0 ], &m_minima[ 1 ], (size_t) ( DriftWindows - 1 ) * sizeof( Float32 ) );
        --m_minimaCount;
    }
    m_minima[ m_minimaCount++ ] = (Float32) m_windowMin;
    const UInt32 latest = m_windowMin;
    m_windowCount = 0;
    m_windowMin = 0xFFFFFFFFUL;
    if ( m_minimaCount < DriftMinWindows )
        return;

    // the slope of the minima by least squares, frames per window, as a ratio of frames per frame
    const Float64 n = (Float64) m_minimaCount;
    Float64 meanX = 0.5 * ( n - 1.0 ), meanY = 0.0;
    for ( UInt32 i=0; i<m_minimaCount; ++i )
        meanY += (Float64) m_minima[ i ];
    meanY /= n;
    Float64 sxy = 0.0, sxx = 0.0;
    for ( UInt32 i=0; i<m_minimaCount; ++i )
    {
        const Float64 dx = (Float64) i - meanX;
        sxy += dx * ( (Float64) m_minima[ i ] - meanY );
        sxx += dx * dx;
    }
    const Float64 slope = sxx > 0.0 ? sxy / sxx : 0.0;
    Float64 squares = 0.0;
    for ( UInt32 i=0; i<m_minimaCount; ++i )
    {
        const Float64 fit = meanY + slope * ( (Float64) i - meanX );
        const Float64 off = (Float64) m_minima[ i ] - fit;
        squares += off * off;
    }
    // the standard error of the slope: a slope within the jitter of the minima is no drift
    const Float64 error = ( n > 2.0 && sxx > 0.0 ) ? sqrt( squares / ( n - 2.0 ) / sxx ) : 1.0e9;
    const bool significant = fabs( slope ) > 3.0 * error;
    const Float64 residual = slope / ( (Float64) m_windowBlocks * (Float64) m_block );

    // the level runs toward too little reference for a block: lean the ratio a little so it comes back slowly
    const Float64 lean = (Float64) latest < (Float64) m_block * 1.125 ? -4.0e-6 : 0.0;

    if ( ( !significant || fabs( residual ) < DriftEngage ) && 0.0 == lean )
    {
        // nothing measurable yet: the window grows (up to DriftWindows) and the estimate gets better
        return;
    }

    Float64 ratio = m_ratio + 0.8 * ( significant ? residual : 0.0 ) + lean;
    if ( ratio > 1.0 + DriftLimit )
        ratio = 1.0 + DriftLimit;
    if ( ratio < 1.0 - DriftLimit )
        ratio = 1.0 - DriftLimit;
    m_ratio = ratio;
    if ( !m_compensating )
    {
        // from now on through the interpolator; its first output is the next frame of the ring, as it would have been
        m_local[ 0 ] = m_tail[ 1 ];
        m_local[ 1 ] = m_tail[ 2 ];
        m_local[ 2 ] = m_tail[ 3 ];
        m_localCount = 3;
        m_position = 3.0;
        m_compensating = true;
    }
    // a new ratio: the slope is measured afresh
    m_minimaCount = 0;
}

/*-------------------------------------------------------------------------*/

Float32
CAcousticEchoCanceller::ReadReferenceBlock( Float32* destination )
{
    const UInt32 block = m_block;

    // how far the reference is ahead of the microphone: in the ring, less what is owed, plus what is buffered
    Int64 level = (Int64) m_ring.GetAvailable() - (Int64) m_debt;
    if ( m_compensating )
        level += (Int64) m_localCount - (Int64) floor( m_position );
    m_level = level > 0 ? (UInt32) level : 0;
    if ( m_settings.GetDriftCompensation() )
        UpdateDrift( m_level );

    if ( !m_compensating )
    {
        ReadRing( destination, block );
        for ( UInt32 i=0; i<4; ++i )
            m_tail[ i ] = destination[ block - 4 + i ];
    }
    else
    {
        // the frames from three before the first position to four after the last one
        const Float64 ratio = m_ratio;
        const UInt32 needed = (UInt32) floor( m_position + (Float64) ( block - 1 ) * ratio ) + 5;
        if ( needed > m_localCount )
        {
            const UInt32 room = (UInt32) m_local.size();
            const UInt32 wanted = ( needed < room ? needed : room ) - m_localCount;
            ReadRing( &m_local[ m_localCount ], wanted );
            m_localCount += wanted;
        }
        const Float32* local = &m_local[ 0 ];
        const Float32* table = &m_lagrange[ 0 ];
        Float64 position = m_position;
        for ( UInt32 n=0; n<block; ++n )
        {
            UInt32 whole = (UInt32) position;
            UInt32 phase = (UInt32) ( ( position - (Float64) whole ) * (Float64) LagrangePhases + 0.5 );
            if ( phase >= LagrangePhases )
            {
                phase = 0;
                ++whole;
            }
            const Float32* c = table + (size_t) phase * LagrangeTaps;
            const Float32* s = local + whole - 3;
            destination[ n ] = ( c[ 0 ] * s[ 0 ] + c[ 1 ] * s[ 1 ] ) + ( c[ 2 ] * s[ 2 ] + c[ 3 ] * s[ 3 ] ) +
                               ( c[ 4 ] * s[ 4 ] + c[ 5 ] * s[ 5 ] ) + ( c[ 6 ] * s[ 6 ] + c[ 7 ] * s[ 7 ] );
            position += ratio;
        }
        // drop the frames that are no longer needed (three are kept before the position)
        const UInt32 drop = (UInt32) floor( position ) > 3 ? (UInt32) floor( position ) - 3 : 0;
        if ( drop > 0 && drop <= m_localCount )
        {
            memmove( &m_local[ 0 ], &m_local[ drop ], (size_t) ( m_localCount - drop ) * sizeof( Float32 ) );
            m_localCount -= drop;
            position -= (Float64) drop;
        }
        m_position = position;
    }

    Float32 peak = 0.0f;
    for ( UInt32 n=0; n<block; ++n )
    {
        Float32 x = destination[ n ];
        if ( !IsFinite( x ) )
        {
            x = 0.0f;
            destination[ n ] = 0.0f;
        }
        const Float32 magnitude = fabsf( x );
        peak = magnitude > peak ? magnitude : peak;
    }
    return peak;
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::ClearResidualEcho( void )
{
    if ( !m_residualEcho.empty() )
        memset( &m_residualEcho[ 0 ], 0, m_residualEcho.size() * sizeof( Float32 ) );
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::MapResidualEcho( void )
{
    if ( m_residualEcho.empty() )
        return;

    // The residual echo power, the leakage times the power of the echo estimate, in the bins and power units of the
    // suppressor: its transform of N frames under the square root Hann window has N/2 times the power of a frame per
    // bin, the transform of the block (B frames, no window) B times
    const UInt32 resBins = (UInt32) m_residualEcho.size();
    const UInt32 resSize = 2 * ( resBins - 1 );
    const Float32 scale = m_leakage * ( 0.5f * (Float32) resSize ) / (Float32) m_block;
    const Float32* power = &m_echoPower[ 0 ];
    const UInt32 ratio = ( 2 * m_block ) / resSize;
    for ( UInt32 j=0; j<resBins; ++j )
    {
        Float32 value = 0.0f;
        if ( ratio >= 2 )
        {
            const Int32 half = (Int32) ( ratio / 2 );
            const Int32 centre = (Int32) ( j * ratio );
            Float32 sum = 0.0f, weights = 0.0f;
            for ( Int32 i=-half; i<=half; ++i )
            {
                Int32 k = centre + i;
                if ( k < 0 )
                    k = -k;
                if ( k > (Int32) m_block )
                    k = 2 * (Int32) m_block - k;
                const Float32 w = (Float32) ( half + 1 - ( i < 0 ? -i : i ) );
                sum += w * power[ k ];
                weights += w;
            }
            value = sum / weights;
        }
        else
        {
            UInt32 k = (UInt32) ( (Float64) j * (Float64) ( 2 * m_block ) / (Float64) resSize + 0.5 );
            value = power[ k < m_bins ? k : m_bins - 1 ];
        }
        m_residualEcho[ j ] = scale * value;
    }
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::ComputeEchoEstimate( void )
{
    const UInt32 block = m_block;
    const UInt32 spectrum = m_spectrum;
    Float32* work = &m_work[ 0 ];

    // the newest reference spectrum: the previous and the current block, overlap-save
    m_xHead = ( m_xHead + 1 ) % m_history;
    const UInt32 bins = m_bins;
    memcpy( work, &m_xPrevious[ 0 ], (size_t) block * sizeof( Float32 ) );
    memcpy( work + block, &m_x[ 0 ], (size_t) block * sizeof( Float32 ) );
    m_fft.Forward( work, work );
    Deinterleave( work, PartitionX( 0 ), bins );

    // the echo estimate: the sum over the partitions of filter times reference, bin by bin (split spectra)
    Float32* acc = &m_E[ 0 ];   // used as scratch here, the error spectrum comes at the end of the block
    memset( acc, 0, (size_t) spectrum * sizeof( Float32 ) );
    for ( UInt32 p=0; p<m_partitions; ++p )
    {
        const Float32* w = PartitionW( p );
        const Float32* x = PartitionX( p + m_shift );
        MultiplyAccumulate( acc, acc + bins, w, w + bins, x, x + bins, bins );
    }
    Interleave( acc, work, bins );
    m_fft.Inverse( work, work );
    memcpy( &m_y[ 0 ], work + block, (size_t) block * sizeof( Float32 ) );

    // its spectrum as a block (zeros, then the block), for the step control and the residual echo
    memset( work, 0, (size_t) block * sizeof( Float32 ) );
    memcpy( work + block, &m_y[ 0 ], (size_t) block * sizeof( Float32 ) );
    m_fft.Forward( work, &m_Y[ 0 ] );
    const Float32* Y = &m_Y[ 0 ];
    Float32* power = &m_echoPower[ 0 ];
    for ( UInt32 k=0; k<m_bins; ++k )
        power[ k ] = Y[ 2 * k ] * Y[ 2 * k ] + Y[ 2 * k + 1 ] * Y[ 2 * k + 1 ];
    m_transforms += 3;
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::StartBlock( void )
{
    m_blockValid = false;
    m_blockIdle = true;
    m_blockFarActive = false;
    if ( !m_synced || m_needResync )
    {
        m_synced = false;
        if ( !TryResync() )
        {
            ClearResidualEcho();
            return;
        }
    }

    m_blockValid = true;
    ++m_blocks;
    const Float32 peak = ReadReferenceBlock( &m_x[ 0 ] );
    const bool farActive = peak > m_farThreshold;
    m_blockFarActive = farActive;
    if ( farActive )
        m_silentBlocks = 0;
    else
    if ( m_silentBlocks < 0xFFFFFFFUL )
        ++m_silentBlocks;

    // quiet over the whole span of the filter: the echo estimate is nothing, there is nothing to compute
    const bool idle = m_silentBlocks > m_partitions + m_shift + 1;
    m_blockIdle = idle;
    if ( idle )
    {
        if ( !m_wasIdle )
        {
            m_wasIdle = true;
            ClearResidualEcho();
        }
        ++m_idleBlocks;
        return;
    }
    if ( m_wasIdle )
    {
        // the spectra of the quiet blocks count as silence
        memset( &m_X[ 0 ], 0, m_X.size() * sizeof( Float32 ) );
        memset( &m_xPrevious[ 0 ], 0, m_xPrevious.size() * sizeof( Float32 ) );
        m_wasIdle = false;
    }
    ComputeEchoEstimate();
    MapResidualEcho();
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::UpdateProportions( void )
{
    // proportionate step per partition: partitions that hold more of the response adapt faster (the bulk of a room's
    // response is near its start); each gets at least a tenth of the largest, and they add up to 0.99
    const UInt32 spectrum = m_spectrum;
    Float32 largest = 0.0f;
    Float32* proportion = &m_proportion[ 0 ];
    for ( UInt32 p=0; p<m_partitions; ++p )
    {
        const Float32* w = PartitionW( p );
        Float32 s0 = 0.0f, s1 = 0.0f;
        for ( UInt32 i=0; i<spectrum; i+=2 )
        {
            s0 += w[ i ] * w[ i ];
            s1 += w[ i + 1 ] * w[ i + 1 ];
        }
        proportion[ p ] = sqrtf( s0 + s1 + TinyPower );
        largest = proportion[ p ] > largest ? proportion[ p ] : largest;
    }
    Float32 sum = 0.0f;
    for ( UInt32 p=0; p<m_partitions; ++p )
    {
        proportion[ p ] += 0.1f * largest;
        sum += proportion[ p ];
    }
    const Float32 scale = sum > 0.0f ? 0.99f / sum : 0.0f;
    for ( UInt32 p=0; p<m_partitions; ++p )
        proportion[ p ] = sum > 0.0f ? proportion[ p ] * scale : 0.99f / (Float32) m_partitions;
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::Constrain( UInt32 partition )
{
    // the gradient of an overlap-save block is circular: the response is kept to its first B taps
    Float32* w = PartitionW( partition );
    Float32* work = &m_work[ 0 ];
    Interleave( w, work, m_bins );
    m_fft.Inverse( work, work );
    memset( work + m_block, 0, (size_t) m_block * sizeof( Float32 ) );
    m_fft.Forward( work, work );
    Deinterleave( work, w, m_bins );
    m_transforms += 2;
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::ShiftFilter( Int32 blocks )
{
    // the response moves by whole partitions: what is learnt stays where it still fits
    const UInt32 count = m_partitions;
    const size_t size = (size_t) m_spectrum * sizeof( Float32 );
    if ( blocks > 0 )
    {
        for ( UInt32 p=0; p<count; ++p )
        {
            if ( p + (UInt32) blocks < count )
                memcpy( PartitionW( p ), PartitionW( p + (UInt32) blocks ), size );
            else
                memset( PartitionW( p ), 0, size );
        }
    }
    else
    if ( blocks < 0 )
    {
        const UInt32 by = (UInt32) ( -blocks );
        for ( UInt32 p=count; p-->0; )
        {
            if ( p >= by )
                memcpy( PartitionW( p ), PartitionW( p - by ), size );
            else
                memset( PartitionW( p ), 0, size );
        }
    }
    m_hasSnapshot = false;
    ++m_delayChanges;
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::Adapt( bool farActive )
{
    const UInt32 block = m_block;
    const UInt32 bins = m_bins;
    const UInt32 spectrum = m_spectrum;
    Float32* work = &m_work[ 0 ];

    // the error of the block as a spectrum, like the echo estimate
    memset( work, 0, (size_t) block * sizeof( Float32 ) );
    memcpy( work + block, &m_e[ 0 ], (size_t) block * sizeof( Float32 ) );
    m_fft.Forward( work, &m_E[ 0 ] );
    m_transforms += 1;

    // energies of the block
    Float32 sxx = 0.0f, see = 0.0f, syy = 0.0f, sdd = 0.0f, sey = 0.0f;
    {
        const Float32* x = &m_x[ 0 ];
        const Float32* e = &m_e[ 0 ];
        const Float32* y = &m_y[ 0 ];
        const Float32* d = &m_d[ 0 ];
        for ( UInt32 n=0; n<block; ++n )
        {
            sxx += x[ n ] * x[ n ];
            see += e[ n ] * e[ n ];
            syy += y[ n ] * y[ n ];
            sdd += d[ n ] * d[ n ];
            sey += e[ n ] * y[ n ];
        }
    }
    if ( !IsFinite( see ) || !IsFinite( syy ) || !IsFinite( sxx ) )
    {
        // never expected: start the filter over rather than carry the damage on
        memset( &m_W[ 0 ], 0, m_W.size() * sizeof( Float32 ) );
        m_adapted = false;
        m_adaptSum = 0.0f;
        m_converged = false;
        ++m_divergences;
        return;
    }

    const Float32* E = &m_E[ 0 ];
    const Float32* Y = &m_Y[ 0 ];
    const Float32* X0 = PartitionX( 0 );            // split: real parts, then imaginary parts
    const Float32* X0im = X0 + bins;
    Float32* errorPower = &m_errorPower[ 0 ];
    const Float32* echoPower = &m_echoPower[ 0 ];
    Float32* referencePower = &m_referencePower[ 0 ];

    // the reference power per bin, smoothed over about the length of the filter (a running mean over the first blocks, so
    // the first steps are not normalised by a power that is still building up)
    Float32 ss = 0.35f / (Float32) m_partitions;
    if ( m_powerBlocks < 1000 )
        ++m_powerBlocks;
    if ( 1.0f / (Float32) m_powerBlocks > ss )
        ss = 1.0f / (Float32) m_powerBlocks;
    Float32* normalisation = &m_step[ 0 ];
    for ( UInt32 k=0; k<bins; ++k )
    {
        const Float32 xp = X0[ k ] * X0[ k ] + X0im[ k ] * X0im[ k ];
        referencePower[ k ] = ( 1.0f - ss ) * referencePower[ k ] + ss * xp;
        normalisation[ k ] = referencePower[ k ];
        errorPower[ k ] = E[ 2 * k ] * E[ 2 * k ] + E[ 2 * k + 1 ] * E[ 2 * k + 1 ];
    }

    // the coherence of the microphone (error plus estimate) and the echo estimate in the band of speech: low when the
    // near end talks over the echo
    {
        const UInt32 low = (UInt32) ( 300.0 * 2.0 * (Float64) block / (Float64) m_rate );
        UInt32 high = (UInt32) ( 3500.0 * 2.0 * (Float64) block / (Float64) m_rate );
        if ( high >= bins )
            high = bins - 1;
        const Float32 a = CoherenceSmoothing;
        const Float32 b = 1.0f - a;
        Float64 numerator = 0.0, denominator = 0.0;
        for ( UInt32 k=low; k<=high; ++k )
        {
            const Float32 yr = Y[ 2 * k ], yi = Y[ 2 * k + 1 ];
            const Float32 dr = E[ 2 * k ] + yr, di = E[ 2 * k + 1 ] + yi;
            m_crossRe[ k ] = a * m_crossRe[ k ] + b * ( dr * yr + di * yi );
            m_crossIm[ k ] = a * m_crossIm[ k ] + b * ( di * yr - dr * yi );
            m_micPower[ k ] = a * m_micPower[ k ] + b * ( dr * dr + di * di );
            m_estimatePower[ k ] = a * m_estimatePower[ k ] + b * ( yr * yr + yi * yi );
            numerator += (Float64) m_crossRe[ k ] * m_crossRe[ k ] + (Float64) m_crossIm[ k ] * m_crossIm[ k ];
            denominator += (Float64) m_micPower[ k ] * m_estimatePower[ k ];
        }
        const Float32 coherence = denominator > 0.0 ? (Float32) ( numerator / denominator ) : 0.0f;

        // double talk: the microphone and the echo estimate are not alike, and the microphone carries more than the
        // estimate (a changed echo path makes them unalike as well, but not louder); counted for at most a while, after
        // which the filter adapts again whatever it is (a louder new path)
        const bool unalike = m_adapted && farActive && coherence < DoubleTalkCoherence && sdd > 2.0f * syy;
        if ( unalike && m_doubleTalkBlocks < MaxDoubleTalkBlocks )
        {
            ++m_doubleTalkBlocks;
            m_doubleTalk = true;
        }
        else
        {
            if ( !unalike )
                m_doubleTalkBlocks = 0;
            m_doubleTalk = false;
        }
    }

    // Valin's leakage: regression of the power fluctuations of the error on those of the echo estimate; not in double
    // talk, where the near end's fluctuations would pass for leakage
    if ( !m_doubleTalk )
    {
        const Float32 average = (Float32) block / (Float32) m_rate;
        Float32* errorMean = &m_errorMean[ 0 ];
        Float32* echoMean = &m_echoMean[ 0 ];
        Float64 pey = 0.0, pyy = 0.0;
        for ( UInt32 k=0; k<bins; ++k )
        {
            const Float32 eh = errorPower[ k ] - errorMean[ k ];
            const Float32 yh = echoPower[ k ] - echoMean[ k ];
            pey += (Float64) eh * (Float64) yh;
            pyy += (Float64) yh * (Float64) yh;
            errorMean[ k ] = ( 1.0f - average ) * errorMean[ k ] + average * errorPower[ k ];
            echoMean[ k ] = ( 1.0f - average ) * echoMean[ k ] + average * echoPower[ k ];
        }
        pyy = sqrt( pyy );
        const Float32 cross = pyy > 0.0 ? (Float32) ( pey / pyy ) : 0.0f;
        const Float32 beta0 = 2.0f * average;
        const Float32 betaMax = 0.5f * average;
        Float32 alpha = 0.0f;
        if ( see > TinyPower )
        {
            Float32 amount = beta0 * syy;
            if ( amount > betaMax * see )
                amount = betaMax * see;
            alpha = amount / see;
        }
        m_leakCross = ( 1.0f - alpha ) * m_leakCross + alpha * cross;
        m_leakVariance = ( 1.0f - alpha ) * m_leakVariance + alpha * (Float32) pyy;
        if ( m_leakVariance < TinyPower )
            m_leakVariance = TinyPower;
        if ( m_leakCross < MinLeakage * m_leakVariance )
            m_leakCross = MinLeakage * m_leakVariance;
        if ( m_leakCross > m_leakVariance )
            m_leakCross = m_leakVariance;
        m_leakage = m_leakCross / m_leakVariance;
    }

    // the residual to error ratio of the whole block, with a lower bound from what of the echo estimate is in the error
    Float32 rer = see > TinyPower ? ( 0.0001f * sxx + 3.0f * m_leakage * syy ) / see : 0.5f;
    const Float32 lower = ( see * syy > TinyPower ) ? ( sey * sey ) / ( see * syy ) : 0.0f;
    if ( rer < lower )
        rer = lower;
    if ( rer > 0.5f )
        rer = 0.5f;

    // the echo return loss enhancement over the blocks where only the far end is active
    if ( farActive && !m_doubleTalk && sdd > TinyPower )
    {
        m_micSum = ErleSmoothing * m_micSum + sdd;
        m_errorSum = ErleSmoothing * m_errorSum + see;
        m_erleDb = m_errorSum > TinyPower ? (Float32) ( 10.0 * log10( (Float64) m_micSum / (Float64) m_errorSum ) ) : 60.0f;
        const Float32 converged = m_settings.GetConvergedErleInDecibels();
        if ( m_erleDb >= converged )
        {
            m_belowCount = 0;
            if ( ++m_aboveCount >= ConvergenceBlocks )
                m_converged = true;
        }
        else
        if ( m_erleDb < converged - 6.0f )
        {
            m_aboveCount = 0;
            if ( ++m_belowCount >= ConvergenceBlocks )
                m_converged = false;
        }
    }

    // divergence: the output well above the microphone, over a dozen blocks, means the filter adds echo
    if ( farActive )
    {
        const Float32 d = 1.0f - 1.0f / (Float32) DivergenceBlocks;
        m_divergenceMic = d * m_divergenceMic + sdd;
        m_divergenceError = d * m_divergenceError + see;
        ++m_divergentBlocks;
        if ( m_divergentBlocks >= DivergenceBlocks && m_divergenceMic > TinyPower && m_divergenceError > DivergenceRatio * m_divergenceMic )
        {
            if ( m_hasSnapshot )
                memcpy( &m_W[ 0 ], &m_snapshot[ 0 ], m_W.size() * sizeof( Float32 ) );
            else
            {
                memset( &m_W[ 0 ], 0, m_W.size() * sizeof( Float32 ) );
                m_adapted = false;
                m_adaptSum = 0.0f;
            }
            m_converged = false;
            m_aboveCount = 0;
            m_divergentBlocks = 0;
            m_divergenceMic = 0.0f;
            m_divergenceError = 0.0f;
            ++m_divergences;
            return;
        }
    }

    // a good filter is kept, once a second, to come back to after a divergence
    if ( m_converged && farActive && !m_doubleTalk )
    {
        if ( ++m_blocksSinceSnapshot >= m_windowBlocks )
        {
            memcpy( &m_snapshot[ 0 ], &m_W[ 0 ], m_W.size() * sizeof( Float32 ) );
            m_hasSnapshot = true;
            m_blocksSinceSnapshot = 0;
        }
    }

    // adaptation: only with the far end active, not in double talk, in every n-th block
    const UInt32 every = m_settings.GetAdaptEveryBlocks();
    if ( !farActive || m_doubleTalk || ( every > 1 && 0 != ( m_blockCounter % every ) ) )
        return;
    const Float32 farEnergy = m_farThreshold * m_farThreshold * (Float32) block * 0.1f;
    if ( sxx < farEnergy )
        return;

    Float32* step = &m_step[ 0 ];
    const Float32 regularisation = m_regularisation;
    if ( !m_adapted )
    {
        // a fixed rate until the filter has some shape, lowered when the error is much louder than the reference
        Float32 rate = 0.0f;
        if ( see > TinyPower )
        {
            Float32 amount = 0.25f * sxx;
            if ( amount > 0.25f * see )
                amount = 0.25f * see;
            rate = amount / see;
        }
        for ( UInt32 k=0; k<bins; ++k )
            step[ k ] = rate / ( normalisation[ k ] + regularisation );
        m_adaptSum += rate;
        if ( m_adaptSum > (Float32) m_partitions && m_leakage > AdaptedLeakage )
            m_adapted = true;
    }
    else
    {
        // the step that minimises the residual echo: its estimate against the error, per bin
        const Float32 leakage = m_leakage;
        for ( UInt32 k=0; k<bins; ++k )
        {
            const Float32 e = errorPower[ k ] + TinyPower;
            Float32 r = leakage * echoPower[ k ];
            if ( r > 0.5f * e )
                r = 0.5f * e;
            r = 0.7f * r + 0.3f * rer * e;
            step[ k ] = r / ( e * ( normalisation[ k ] + regularisation ) );
        }
    }

    if ( 0 == ( m_blockCounter & 3 ) )
        UpdateProportions();

    // W += step * proportion * conj( X ) E, partition by partition (split spectra)
    Float32* errorSplit = &m_work[ 0 ];
    Deinterleave( E, errorSplit, bins );
    for ( UInt32 p=0; p<m_partitions; ++p )
    {
        Float32* w = PartitionW( p );
        const Float32* x = PartitionX( p + m_shift );
        AdaptPartition( w, w + bins, step, m_proportion[ p ], x, x + bins, errorSplit, errorSplit + bins, bins );
    }
    (void) spectrum;

    // the constraint on partition 0 every block and on the others in turn (alternately updated MDF)
    Constrain( 0 );
    if ( m_partitions > 1 )
    {
        Constrain( 1 + ( m_constrainNext % ( m_partitions - 1 ) ) );
        ++m_constrainNext;
    }
    ++m_adaptedBlocks;
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::EndBlock( void )
{
    if ( !m_blockValid )
        return;

    if ( m_delayEstimator.IsPrepared() )
        m_delayEstimator.Process( &m_x[ 0 ], &m_d[ 0 ], m_block, m_blockFarActive );

    if ( !m_blockIdle )
        Adapt( m_blockFarActive );

    // the filter follows the bulk delay in whole blocks, one block before the echo begins
    if ( m_delayEstimator.IsPrepared() && m_delayEstimator.HasEstimate() )
    {
        const UInt32 delay = m_delayEstimator.GetDelayInFrames();
        UInt32 wanted = delay >= m_block ? delay / m_block - 1 : 0;
        if ( wanted > m_maxShift )
            wanted = m_maxShift;
        const bool before = delay < m_shift * m_block;
        if ( wanted != m_shift && ( before || wanted > m_shift + 1 || wanted + 1 < m_shift ) )
        {
            ShiftFilter( (Int32) wanted - (Int32) m_shift );
            m_shift = wanted;
        }
    }

    memcpy( &m_xPrevious[ 0 ], &m_x[ 0 ], (size_t) m_block * sizeof( Float32 ) );
    ++m_blockCounter;
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::Process( Float32* mono, UInt32 frames, UInt64 firstFrame ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    ++m_captureCalls;
    if ( 0 == m_block || GUCEF_NULL == mono || 0 == frames )
    {
        m_publishedCaptureCalls.Store( m_captureCalls, MT::MEMORDER_RELAXED );
        return;
    }

    // switched off: the audio passes as it is and nothing is computed; back on, the alignment is found again
    if ( !m_enabled.Load( MT::MEMORDER_RELAXED ) )
    {
        if ( m_wasEnabled )
        {
            m_wasEnabled = false;
            m_synced = false;
            m_needResync = true;
            Publish();
        }
        m_publishedCaptureCalls.Store( m_captureCalls, MT::MEMORDER_RELAXED );
        return;
    }
    if ( !m_wasEnabled )
    {
        m_wasEnabled = true;
        m_synced = false;
        m_needResync = true;
    }

    if ( frames > m_maxCaptureChunk )
        m_maxCaptureChunk = frames;

    // a gap in the microphone or a gap in the reference: the alignment is lost
    if ( m_synced && firstFrame != m_expectedFrame )
        m_needResync = true;
    m_expectedFrame = firstFrame + frames;
    const UInt64 dropped = m_ring.GetDroppedFrameCount();
    if ( dropped != m_droppedSeen )
    {
        m_droppedSeen = dropped;
        if ( m_synced )
            m_needResync = true;
    }

    // blocks are aligned to the absolute position; a block that starts in the middle (after a gap) only passes
    const UInt32 block = m_block;
    const UInt32 inBlock = (UInt32) ( firstFrame % block );
    if ( inBlock != m_inBlock )
    {
        m_inBlock = inBlock;
        m_blockValid = false;
    }

    while ( frames > 0 )
    {
        if ( 0 == m_inBlock )
            StartBlock();

        UInt32 run = block - m_inBlock;
        if ( run > frames )
            run = frames;

        Float32* d = &m_d[ m_inBlock ];
        Float32* e = &m_e[ m_inBlock ];
        for ( UInt32 i=0; i<run; ++i )
        {
            const Float32 value = mono[ i ];
            d[ i ] = IsFinite( value ) ? value : 0.0f;
        }
        if ( m_blockValid && !m_blockIdle )
        {
            const Float32* y = &m_y[ m_inBlock ];
            for ( UInt32 i=0; i<run; ++i )
            {
                e[ i ] = d[ i ] - y[ i ];
                mono[ i ] = e[ i ];
            }
        }
        else
        {
            memcpy( e, d, (size_t) run * sizeof( Float32 ) );
        }

        m_inBlock += run;
        if ( m_inBlock == block )
        {
            EndBlock();
            m_inBlock = 0;
        }

        if ( m_residual.IsPrepared() )
            m_residual.Process( mono, run );

        mono += run;
        frames -= run;
    }
    Publish();
}

/*-------------------------------------------------------------------------*/

void
CAcousticEchoCanceller::Publish( void )
{
    TEchoCancellerState state = ECHOCANCELLERSTATE_OFF;
    if ( 0 != m_block && m_wasEnabled )
    {
        if ( !m_synced )
            state = ECHOCANCELLERSTATE_WAITING;
        else
        if ( m_blockIdle )
            state = ECHOCANCELLERSTATE_IDLE;
        else
            state = m_converged ? ECHOCANCELLERSTATE_CONVERGED : ECHOCANCELLERSTATE_CONVERGING;
    }
    m_publishedState.Store( (UInt32) state, MT::MEMORDER_RELAXED );
    m_publishedConverged.Store( m_converged, MT::MEMORDER_RELAXED );
    m_publishedDoubleTalk.Store( m_doubleTalk, MT::MEMORDER_RELAXED );
    m_publishedFarActive.Store( m_blockFarActive, MT::MEMORDER_RELAXED );
    m_publishedCompensating.Store( m_compensating, MT::MEMORDER_RELAXED );
    m_publishedHasDelay.Store( m_delayEstimator.IsPrepared() && m_delayEstimator.HasEstimate(), MT::MEMORDER_RELAXED );
    m_publishedDelay.Store( m_delayEstimator.IsPrepared() ? m_delayEstimator.GetDelayInFrames() : 0, MT::MEMORDER_RELAXED );
    m_publishedBulkDelay.Store( m_shift * m_block, MT::MEMORDER_RELAXED );
    m_publishedLevel.Store( m_level, MT::MEMORDER_RELAXED );
    m_publishedCaptureCalls.Store( m_captureCalls, MT::MEMORDER_RELAXED );
    m_publishedBlocks.Store( m_blocks, MT::MEMORDER_RELAXED );
    m_publishedIdleBlocks.Store( m_idleBlocks, MT::MEMORDER_RELAXED );
    m_publishedAdaptedBlocks.Store( m_adaptedBlocks, MT::MEMORDER_RELAXED );
    const UInt64 residualTransforms = m_residual.IsPrepared() ? 2 * m_residual.GetTransformedFrameCount() : 0;
    m_publishedTransforms.Store( m_transforms + residualTransforms, MT::MEMORDER_RELAXED );
    m_publishedResyncs.Store( m_resyncs, MT::MEMORDER_RELAXED );
    m_publishedUnderruns.Store( m_underruns, MT::MEMORDER_RELAXED );
    m_publishedDivergences.Store( m_divergences, MT::MEMORDER_RELAXED );
    m_publishedDelayChanges.Store( m_delayChanges, MT::MEMORDER_RELAXED );
    m_publishedErle.Store( m_erleDb );
    m_publishedResidualGain.Store( m_residual.IsPrepared() ? m_residual.GetMeanGainInDecibels() : 0.0f );
    m_publishedLeakage.Store( m_leakage );
    m_publishedDrift.Store( (Float32) ( ( m_ratio - 1.0 ) * 1.0e6 ) );
}

/*-------------------------------------------------------------------------*/

TEchoCancellerState
CAcousticEchoCanceller::GetState( void ) const
{
    // no trace macro: the real time threads use it
    return (TEchoCancellerState) m_publishedState.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

bool
CAcousticEchoCanceller::IsConverged( void ) const
{
    return m_publishedConverged.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

bool
CAcousticEchoCanceller::IsDoubleTalk( void ) const
{
    return m_publishedDoubleTalk.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

bool
CAcousticEchoCanceller::IsFarEndActive( void ) const
{
    return m_publishedFarActive.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

Float32
CAcousticEchoCanceller::GetErleInDecibels( void ) const
{
    return m_publishedErle.Load();
}

/*-------------------------------------------------------------------------*/

Float32
CAcousticEchoCanceller::GetResidualSuppressionInDecibels( void ) const
{
    return m_publishedResidualGain.Load();
}

/*-------------------------------------------------------------------------*/

Float32
CAcousticEchoCanceller::GetLeakage( void ) const
{
    return m_publishedLeakage.Load();
}

/*-------------------------------------------------------------------------*/

UInt32
CAcousticEchoCanceller::GetBulkDelayInFrames( void ) const
{
    return m_publishedBulkDelay.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt32
CAcousticEchoCanceller::GetEstimatedDelayInFrames( void ) const
{
    return m_publishedDelay.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

bool
CAcousticEchoCanceller::HasDelayEstimate( void ) const
{
    return m_publishedHasDelay.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

Float32
CAcousticEchoCanceller::GetDriftInPpm( void ) const
{
    return m_publishedDrift.Load();
}

/*-------------------------------------------------------------------------*/

bool
CAcousticEchoCanceller::IsDriftCompensating( void ) const
{
    return m_publishedCompensating.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt32
CAcousticEchoCanceller::GetReferenceLevelInFrames( void ) const
{
    return m_publishedLevel.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt64
CAcousticEchoCanceller::GetCaptureCallCount( void ) const
{
    return m_publishedCaptureCalls.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt64
CAcousticEchoCanceller::GetRenderCallCount( void ) const
{
    return m_renderCalls.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt64
CAcousticEchoCanceller::GetBlockCount( void ) const
{
    return m_publishedBlocks.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt64
CAcousticEchoCanceller::GetIdleBlockCount( void ) const
{
    return m_publishedIdleBlocks.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt64
CAcousticEchoCanceller::GetAdaptedBlockCount( void ) const
{
    return m_publishedAdaptedBlocks.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt64
CAcousticEchoCanceller::GetTransformCount( void ) const
{
    return m_publishedTransforms.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt64
CAcousticEchoCanceller::GetResyncCount( void ) const
{
    return m_publishedResyncs.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt64
CAcousticEchoCanceller::GetReferenceUnderrunCount( void ) const
{
    return m_publishedUnderruns.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt64
CAcousticEchoCanceller::GetDivergenceCount( void ) const
{
    return m_publishedDivergences.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt64
CAcousticEchoCanceller::GetDelayChangeCount( void ) const
{
    return m_publishedDelayChanges.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
