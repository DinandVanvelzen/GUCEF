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

#include "gucefAUDIO_CEchoDelayEstimator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const Float64 CEchoDelayEstimator::ResolutionSeconds = 0.002;
const Float64 CEchoDelayEstimator::CorrelationSeconds = 2.0;
const Float64 CEchoDelayEstimator::EvaluationSeconds = 0.25;
const Float32 CEchoDelayEstimator::MinimumCorrelation = 0.4f;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The slow mean removed from the envelopes */
const Float64 MeanSeconds = 0.5;

/** The lowest envelope level, dB */
const Float32 EnvelopeFloorDb = -80.0f;

Float32
EnvelopeDb( Float64 sumOfSquares, UInt32 frames )
{
    const Float64 meanSquare = sumOfSquares / (Float64) frames;
    if ( !( meanSquare > 1.0e-8 ) )         // also not a number
        return EnvelopeFloorDb;
    const Float32 level = (Float32) ( 10.0 * log10( meanSquare ) );
    return level > EnvelopeFloorDb ? level : EnvelopeFloorDb;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEchoDelayEstimator::CEchoDelayEstimator( void )
    : m_resolution( 0 )
    , m_lags( 0 )
    , m_memory( 0.0f )
    , m_meanCoefficient( 0.0f )
    , m_evaluationSteps( 1 )
    , m_history( GUCEF_NULL )
    , m_correlation( GUCEF_NULL )
    , m_referenceEnergy( GUCEF_NULL )
    , m_historyWrite( 0 )
    , m_referenceSum( 0.0 )
    , m_microphoneSum( 0.0 )
    , m_inStep( 0 )
    , m_referenceMean( 0.0f )
    , m_microphoneMean( 0.0f )
    , m_microphoneEnergy( 0.0f )
    , m_meansStarted( false )
    , m_activeSteps( 0 )
    , m_filledSteps( 0 )
    , m_candidate( -1 )
    , m_agreements( 0 )
    , m_hasEstimate( false )
    , m_estimate( 0 )
    , m_lastCorrelation( 0.0f )
    , m_evaluations( 0 )
    , m_changes( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEchoDelayEstimator::~CEchoDelayEstimator()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

bool
CEchoDelayEstimator::Prepare( UInt32 sampleRate, UInt32 maxDelayFrames )
{GUCEF_TRACE;

    Release();
    if ( sampleRate < 8000 || sampleRate > 192000 || 0 == maxDelayFrames )
        return false;

    m_resolution = (UInt32) floor( ResolutionSeconds * (Float64) sampleRate + 0.5 );
    m_lags = maxDelayFrames / m_resolution + 1;
    const Float64 stepsPerSecond = (Float64) sampleRate / (Float64) m_resolution;
    m_memory = (Float32) exp( -1.0 / ( CorrelationSeconds * stepsPerSecond ) );
    m_meanCoefficient = (Float32) exp( -1.0 / ( MeanSeconds * stepsPerSecond ) );
    m_evaluationSteps = (UInt32) floor( EvaluationSeconds * stepsPerSecond + 0.5 );
    if ( m_evaluationSteps < 1 )
        m_evaluationSteps = 1;

    m_history = GUCEF_NEW Float32[ 2 * m_lags ];
    m_correlation = GUCEF_NEW Float32[ m_lags ];
    m_referenceEnergy = GUCEF_NEW Float32[ m_lags ];
    Reset();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEchoDelayEstimator::Release( void )
{GUCEF_TRACE;

    GUCEF_DELETE[] m_history;
    m_history = GUCEF_NULL;
    GUCEF_DELETE[] m_correlation;
    m_correlation = GUCEF_NULL;
    GUCEF_DELETE[] m_referenceEnergy;
    m_referenceEnergy = GUCEF_NULL;
    m_resolution = 0;
    m_lags = 0;
}

/*-------------------------------------------------------------------------*/

bool
CEchoDelayEstimator::IsPrepared( void ) const
{
    // no trace macro: the real time threads use it
    return 0 != m_lags;
}

/*-------------------------------------------------------------------------*/

void
CEchoDelayEstimator::Reset( void )
{
    // no trace macro: the capture thread resets
    if ( 0 != m_lags )
    {
        memset( m_history, 0, (size_t) 2 * m_lags * sizeof( Float32 ) );
        memset( m_correlation, 0, (size_t) m_lags * sizeof( Float32 ) );
        memset( m_referenceEnergy, 0, (size_t) m_lags * sizeof( Float32 ) );
    }
    m_historyWrite = 0;
    m_referenceSum = 0.0;
    m_microphoneSum = 0.0;
    m_inStep = 0;
    m_referenceMean = 0.0f;
    m_microphoneMean = 0.0f;
    m_microphoneEnergy = 0.0f;
    m_meansStarted = false;
    m_activeSteps = 0;
    m_filledSteps = 0;
    m_candidate = -1;
    m_agreements = 0;
    m_hasEstimate = false;
    m_estimate = 0;
    m_lastCorrelation = 0.0f;
    m_evaluations = 0;
    m_changes = 0;
}

/*-------------------------------------------------------------------------*/

void
CEchoDelayEstimator::Process( const Float32* reference, const Float32* microphone, UInt32 frames, bool farActive )
{
    if ( 0 == m_lags || GUCEF_NULL == reference || GUCEF_NULL == microphone )
        return;

    UInt32 done = 0;
    while ( done < frames )
    {
        UInt32 run = m_resolution - m_inStep;
        if ( run > frames - done )
            run = frames - done;
        Float64 r = 0.0, m = 0.0;
        for ( UInt32 i=0; i<run; ++i )
        {
            const Float64 x = (Float64) reference[ done + i ];
            const Float64 y = (Float64) microphone[ done + i ];
            r += x * x;
            m += y * y;
        }
        m_referenceSum += r;
        m_microphoneSum += m;
        m_inStep += run;
        done += run;
        if ( m_inStep == m_resolution )
        {
            AddEnvelope( EnvelopeDb( m_referenceSum, m_resolution ), EnvelopeDb( m_microphoneSum, m_resolution ), farActive );
            m_referenceSum = 0.0;
            m_microphoneSum = 0.0;
            m_inStep = 0;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CEchoDelayEstimator::AddEnvelope( Float32 reference, Float32 microphone, bool farActive )
{
    if ( !m_meansStarted )
    {
        m_referenceMean = reference;
        m_microphoneMean = microphone;
        m_meansStarted = true;
    }
    const Float32 c = m_meanCoefficient;
    m_referenceMean = c * m_referenceMean + ( 1.0f - c ) * reference;
    m_microphoneMean = c * m_microphoneMean + ( 1.0f - c ) * microphone;
    const Float32 r = reference - m_referenceMean;
    const Float32 m = microphone - m_microphoneMean;

    // the reference envelope goes on in time, active or not
    const UInt32 lags = m_lags;
    m_history[ m_historyWrite ] = r;
    m_history[ m_historyWrite + lags ] = r;
    if ( ++m_historyWrite == lags )
        m_historyWrite = 0;
    if ( m_filledSteps < lags )
        ++m_filledSteps;

    if ( !farActive )
        return;

    // index j of the window is lag lags - 1 - j: the window runs from the oldest value to the newest, contiguous
    const Float32* window = m_history + m_historyWrite;
    const Float32 memory = m_memory;
    Float32* correlation = m_correlation;
    Float32* energy = m_referenceEnergy;
    for ( UInt32 j=0; j<lags; ++j )
    {
        const Float32 x = window[ j ];
        correlation[ j ] = memory * correlation[ j ] + m * x;
        energy[ j ] = memory * energy[ j ] + x * x;
    }
    m_microphoneEnergy = memory * m_microphoneEnergy + m * m;

    if ( ++m_activeSteps >= m_evaluationSteps )
    {
        m_activeSteps = 0;
        Evaluate();
    }
}

/*-------------------------------------------------------------------------*/

void
CEchoDelayEstimator::Evaluate( void )
{
    ++m_evaluations;
    const UInt32 lags = m_lags;
    Float32 best = 0.0f;
    Int32 bestLag = -1;
    for ( UInt32 lag=0; lag<lags && lag<m_filledSteps; ++lag )
    {
        const UInt32 j = lags - 1 - lag;
        const Float32 c = m_correlation[ j ];
        if ( c <= 0.0f )
            continue;
        const Float32 norm = m_microphoneEnergy * m_referenceEnergy[ j ];
        if ( !( norm > 1.0e-20f ) )
            continue;
        const Float32 ncc = c / sqrtf( norm );
        if ( ncc > best )
        {
            best = ncc;
            bestLag = (Int32) lag;
        }
    }
    m_lastCorrelation = best > 1.0f ? 1.0f : best;

    if ( bestLag < 0 || best < MinimumCorrelation )
    {
        m_agreements = 0;
        return;
    }
    const Int32 difference = bestLag - m_candidate;
    if ( m_candidate >= 0 && difference >= -1 && difference <= 1 )
        ++m_agreements;
    else
        m_agreements = 1;
    m_candidate = bestLag;
    if ( m_agreements >= CONSISTENT_ESTIMATES )
    {
        if ( !m_hasEstimate || m_estimate != (UInt32) bestLag )
            ++m_changes;
        m_estimate = (UInt32) bestLag;
        m_hasEstimate = true;
    }
}

/*-------------------------------------------------------------------------*/

bool
CEchoDelayEstimator::HasEstimate( void ) const
{
    return m_hasEstimate;
}

/*-------------------------------------------------------------------------*/

UInt32
CEchoDelayEstimator::GetDelayInFrames( void ) const
{
    return m_estimate * m_resolution;
}

/*-------------------------------------------------------------------------*/

Float32
CEchoDelayEstimator::GetCorrelation( void ) const
{
    return m_lastCorrelation;
}

/*-------------------------------------------------------------------------*/

UInt32
CEchoDelayEstimator::GetResolutionInFrames( void ) const
{
    return m_resolution;
}

/*-------------------------------------------------------------------------*/

UInt32
CEchoDelayEstimator::GetMaxDelayInFrames( void ) const
{
    return 0 == m_lags ? 0 : ( m_lags - 1 ) * m_resolution;
}

/*-------------------------------------------------------------------------*/

UInt64
CEchoDelayEstimator::GetEvaluationCount( void ) const
{
    return m_evaluations;
}

/*-------------------------------------------------------------------------*/

UInt64
CEchoDelayEstimator::GetEstimateChangeCount( void ) const
{
    return m_changes;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
