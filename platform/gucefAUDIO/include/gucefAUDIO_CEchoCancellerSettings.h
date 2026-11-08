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

#ifndef GUCEF_AUDIO_CECHOCANCELLERSETTINGS_H
#define GUCEF_AUDIO_CECHOCANCELLERSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

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
 *  The settings of a CAcousticEchoCanceller (section 4.13 of the intercom design). Every setter limits its value to the
 *  range given. Saved as the section "EchoCanceller" with the attributes enabled, filterLengthMs, blockFrames,
 *  adaptEveryBlocks, residualSuppression, echoSuppressionDb, residualStrength, suppressNoise, noiseSuppressionDb,
 *  delayEstimation, maxDelayMs, farThresholdDb, driftCompensation, convergedErleDb and referenceMarginMs.
 *
 *  Cost and quality (at 16 kHz, measured in section 7 of the intercom design): the cost of the adaptive filter grows
 *  with filterLengthMs / blockFrames per second and its FFTs with 1 / blockFrames; the residual echo suppressor adds two
 *  small FFTs per 4 ms while there is echo to suppress. A shorter filter cancels less of a reverberant room's tail, a
 *  larger block is cheaper per second and needs the reference that much earlier, adaptEveryBlocks above 1 halves (or
 *  more) the cost of the adaptation and slows the convergence by the same factor.
 */
class GUCEF_AUDIO_PUBLIC_CPP CEchoCancellerSettings : public CORE::CIConfigurable
{
    public:

    static const CString ConfigSectionName;         /**< "EchoCanceller" */

    CEchoCancellerSettings( void );

    CEchoCancellerSettings( const CEchoCancellerSettings& src );

    CEchoCancellerSettings& operator=( const CEchoCancellerSettings& src );

    virtual ~CEchoCancellerSettings() GUCEF_VIRTUAL_OVERRIDE;

    bool operator==( const CEchoCancellerSettings& other ) const;
    bool operator!=( const CEchoCancellerSettings& other ) const;

    /** Whether the canceller acts (default true); off, the microphone passes bit for bit and nothing is computed */
    bool GetEnabled( void ) const;
    void SetEnabled( bool enabled );

    /** The length of the room response the adaptive filter covers beyond the bulk delay, 16 to 512 ms (default 128) */
    Float32 GetFilterLengthInMs( void ) const;
    void SetFilterLengthInMs( Float32 milliseconds );

    /**
     *  Frames per block and partition of the filter: 0 (default) for the power of two nearest 8 ms (128 at 16 kHz, 256
     *  at 32 and 48 kHz, 64 at 8 kHz), else a power of two from 32 to 1024
     */
    UInt32 GetBlockFrames( void ) const;
    void SetBlockFrames( UInt32 frames );

    /** The filter adapts in every n-th block of far-end activity, 1 to 4 (default 1) */
    UInt32 GetAdaptEveryBlocks( void ) const;
    void SetAdaptEveryBlocks( UInt32 blocks );

    /** Whether the residual echo suppressor runs after the filter (default true); it delays the output by its frame (8 ms at 16 kHz) */
    bool GetResidualSuppression( void ) const;
    void SetResidualSuppression( bool enabled );

    /** The deepest attenuation of residual echo, -80 to 0 dB (default -40) */
    Float32 GetEchoSuppressionInDecibels( void ) const;
    void SetEchoSuppressionInDecibels( Float32 decibels );

    /** Factor on the residual echo estimate, 0.25 to 8 (default 2): higher suppresses more of the echo and more of the near end in double talk */
    Float32 GetResidualStrength( void ) const;
    void SetResidualStrength( Float32 strength );

    /** Whether the residual echo suppressor also suppresses stationary noise (default false: use the noiseSuppressor effect, or this to save its transform) */
    bool GetSuppressNoise( void ) const;
    void SetSuppressNoise( bool suppress );

    /** The deepest attenuation of noise when it is suppressed, -40 to 0 dB (default -15) */
    Float32 GetNoiseSuppressionInDecibels( void ) const;
    void SetNoiseSuppressionInDecibels( Float32 decibels );

    /** Whether the bulk delay is estimated and the filter moved to it (default true) */
    bool GetDelayEstimation( void ) const;
    void SetDelayEstimation( bool enabled );

    /** The longest bulk delay looked for, 10 to 1000 ms (default 250) */
    Float32 GetMaxDelayInMs( void ) const;
    void SetMaxDelayInMs( Float32 milliseconds );

    /** The far end is active in a block whose reference peaks above this, -100 to -20 dBFS (default -70) */
    Float32 GetFarThresholdInDecibels( void ) const;
    void SetFarThresholdInDecibels( Float32 decibels );

    /** Whether a drift between the clocks of playback and capture is measured and the reference resampled (default true) */
    bool GetDriftCompensation( void ) const;
    void SetDriftCompensation( bool enabled );

    /** The canceller counts as converged above this echo return loss enhancement, 3 to 40 dB (default 12) */
    Float32 GetConvergedErleInDecibels( void ) const;
    void SetConvergedErleInDecibels( Float32 decibels );

    /**
     *  How far the reference is kept ahead of the microphone beyond one block, 0 to 100 ms; 0 (default) takes the largest
     *  blocks of the render and the capture side as they come
     */
    Float32 GetReferenceMarginInMs( void ) const;
    void SetReferenceMarginInMs( Float32 milliseconds );

    /** Writes the section "EchoCanceller" (kept when it is there) */
    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the section from the tree, or the tree itself when it is that section. False when there is none; what it does not have keeps its value */
    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    bool m_enabled;
    Float32 m_filterLengthInMs;
    UInt32 m_blockFrames;
    UInt32 m_adaptEveryBlocks;
    bool m_residualSuppression;
    Float32 m_echoSuppressionInDecibels;
    Float32 m_residualStrength;
    bool m_suppressNoise;
    Float32 m_noiseSuppressionInDecibels;
    bool m_delayEstimation;
    Float32 m_maxDelayInMs;
    Float32 m_farThresholdInDecibels;
    bool m_driftCompensation;
    Float32 m_convergedErleInDecibels;
    Float32 m_referenceMarginInMs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CECHOCANCELLERSETTINGS_H ? */
