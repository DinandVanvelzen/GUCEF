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

#ifndef GUCEF_AUDIO_CECHODELAYESTIMATOR_H
#define GUCEF_AUDIO_CECHODELAYESTIMATOR_H

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
 *  The bulk delay of an echo: how much later than the reference (what the speaker plays) its echo arrives in the
 *  microphone signal, on a coarse grid. A DSP block of the echo canceller, which moves its adaptive filter to the
 *  delay so the filter only has to cover the room's response.
 *
 *  Method: the envelopes of both signals (the level in dB of each stretch of GetResolutionInFrames() frames, 2 ms, low
 *  limited at -80 dB, with their slow mean removed) are cross-correlated over the lags 0 to the maximum delay, with an
 *  exponential memory of CorrelationSeconds; the correlation is only updated while the far end is active (the caller
 *  says so). Every EvaluationSeconds of active far end the lag of the highest normalised correlation is taken as a
 *  candidate when that correlation is at least MinimumCorrelation; a candidate that ConsistentEstimates evaluations in a
 *  row agree on (within one step of the grid) becomes the estimate. Cost: two multiplications per lag and step of the
 *  grid (250 lags per 2 ms for 250 ms), plus the comparison every EvaluationSeconds; nothing while the far end is quiet.
 *
 *  Prepare() allocates; nothing else allocates, locks or logs. One thread uses one object.
 */
class GUCEF_AUDIO_PUBLIC_CPP CEchoDelayEstimator
{
    public:

    static const Float64 ResolutionSeconds;         /**< 0.002 */
    static const Float64 CorrelationSeconds;        /**< 2.0 */
    static const Float64 EvaluationSeconds;         /**< 0.25 */
    static const Float32 MinimumCorrelation;        /**< 0.4 */
    enum { CONSISTENT_ESTIMATES = 3 };

    CEchoDelayEstimator( void );

    ~CEchoDelayEstimator();

    /** Owner: for the rate (8000 to 192000 Hz) and delays up to 'maxDelayFrames'. Allocates. False for a rate out of range or a delay of 0 */
    bool Prepare( UInt32 sampleRate, UInt32 maxDelayFrames );

    void Release( void );

    bool IsPrepared( void ) const;

    /** No estimate, the correlation forgotten */
    void Reset( void );

    /**
     *  'frames' frames of the reference and of the microphone signal, aligned (frame n of both at the same moment of
     *  the stream), and whether the far end is active in them
     */
    void Process( const Float32* reference, const Float32* microphone, UInt32 frames, bool farActive );

    /** True once ConsistentEstimates evaluations agreed */
    bool HasEstimate( void ) const;

    /** The estimate: the delay of the echo in frames, a multiple of GetResolutionInFrames() */
    UInt32 GetDelayInFrames( void ) const;

    /** The normalised correlation at the lag of the last evaluation, 0 to 1 */
    Float32 GetCorrelation( void ) const;

    UInt32 GetResolutionInFrames( void ) const;
    UInt32 GetMaxDelayInFrames( void ) const;

    /** Evaluations so far, and how often the estimate was set or changed */
    UInt64 GetEvaluationCount( void ) const;
    UInt64 GetEstimateChangeCount( void ) const;

    private:

    CEchoDelayEstimator( const CEchoDelayEstimator& src );              /**< not implemented */
    CEchoDelayEstimator& operator=( const CEchoDelayEstimator& src );   /**< not implemented */

    void AddEnvelope( Float32 reference, Float32 microphone, bool farActive );
    void Evaluate( void );

    private:

    UInt32 m_resolution;
    UInt32 m_lags;                      /**< lags 0 .. m_lags - 1 */
    Float32 m_memory;                   /**< per step coefficient of the correlation */
    Float32 m_meanCoefficient;          /**< per step coefficient of the slow mean that is removed */
    UInt32 m_evaluationSteps;

    Float32* m_history;                 /**< reference envelope, two copies of the last m_lags values (contiguous windows) */
    Float32* m_correlation;             /**< per lag */
    Float32* m_referenceEnergy;         /**< per lag */
    UInt32 m_historyWrite;

    Float64 m_referenceSum;             /**< of the squares in the current step */
    Float64 m_microphoneSum;
    UInt32 m_inStep;
    Float32 m_referenceMean;
    Float32 m_microphoneMean;
    Float32 m_microphoneEnergy;
    bool m_meansStarted;
    UInt32 m_activeSteps;               /**< since the last evaluation */
    UInt32 m_filledSteps;               /**< history values written, up to m_lags */

    Int32 m_candidate;
    UInt32 m_agreements;
    bool m_hasEstimate;
    UInt32 m_estimate;                  /**< in steps */
    Float32 m_lastCorrelation;
    UInt64 m_evaluations;
    UInt64 m_changes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CECHODELAYESTIMATOR_H ? */
