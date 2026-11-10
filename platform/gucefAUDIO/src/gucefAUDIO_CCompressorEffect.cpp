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

#include "gucefAUDIO_CCompressorEffect.h"

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

/** 20 * log10( 2 ): decibels per doubling of the amplitude */
const Float32 DecibelsPerOctave = 6.0205999132796239f;

/** A smoothed reduction below this (in dB) is none at all */
const Float64 ReductionFlushThreshold = 1.0e-9;

/** Detector input beyond this is not audio; it is brought back so the detector state stays finite */
const Float32 MaxDetectorMagnitude = 1.0e15f;

/** Smoothing time of threshold, ratio and knee */
const Float64 CurveSmoothingSeconds = 0.020;

inline Float32
DetectorInput( Float32 sample )
{
    if ( !( fabsf( sample ) <= MaxDetectorMagnitude ) )
        return sample != sample ? 0.0f : ( sample > 0.0f ? MaxDetectorMagnitude : -MaxDetectorMagnitude );
    return sample;
}

inline Float64
CoefficientFor( Float32 milliseconds, Float64 sampleRate )
{
    return exp( -1000.0 / ( (Float64) milliseconds * sampleRate ) );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString&
CCompressorEffect::GetStaticEffectTypeName( void )
{GUCEF_TRACE;

    static const CString typeName( "compressor" );
    return typeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CCompressorEffect::GetStaticParameterCount( void )
{GUCEF_TRACE;

    return PARAMETER_COUNT;
}

/*-------------------------------------------------------------------------*/

const CAudioEffectParameterInfo*
CCompressorEffect::GetStaticParameterInfos( void )
{GUCEF_TRACE;

    static const char* const detectorLabels[ 2 ] = { "peak", "rms" };
    static const CAudioEffectParameterInfo infos[ PARAMETER_COUNT ] =
    {
        CAudioEffectParameterInfo( PARAMETER_THRESHOLD, "threshold", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, -60.0f, 0.0f, -18.0f ),
        CAudioEffectParameterInfo( PARAMETER_RATIO, "ratio", AUDIOPARAMUNIT_RATIO, AUDIOPARAMSCALE_LOGARITHMIC, 1.0f, 20.0f, 4.0f ),
        CAudioEffectParameterInfo( PARAMETER_KNEE, "knee", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, 0.0f, 24.0f, 6.0f ),
        CAudioEffectParameterInfo( PARAMETER_ATTACK, "attack", AUDIOPARAMUNIT_MILLISECONDS, AUDIOPARAMSCALE_LOGARITHMIC, 0.1f, 200.0f, 10.0f ),
        CAudioEffectParameterInfo( PARAMETER_RELEASE, "release", AUDIOPARAMUNIT_MILLISECONDS, AUDIOPARAMSCALE_LOGARITHMIC, 10.0f, 3000.0f, 150.0f ),
        CAudioEffectParameterInfo( PARAMETER_MAKEUP, "makeup", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, 0.0f, 24.0f, 0.0f ),
        CAudioEffectParameterInfo( PARAMETER_DETECTOR, "detector", detectorLabels, 2, 0 ),
        CAudioEffectParameterInfo( PARAMETER_RMS_WINDOW, "rmsWindow", AUDIOPARAMUNIT_MILLISECONDS, AUDIOPARAMSCALE_LOGARITHMIC, 1.0f, 300.0f, 10.0f ),
        CAudioEffectParameterInfo( PARAMETER_STEREO_LINK, "stereoLink", AUDIOPARAMUNIT_NONE, AUDIOPARAMSCALE_LINEAR, 0.0f, 1.0f, 1.0f )
    };
    return infos;
}

/*-------------------------------------------------------------------------*/

CCompressorEffect::CCompressorEffect( const CORE::CValueList& options )
    : CAudioEffect( GetStaticParameterInfos(), PARAMETER_COUNT )
    , m_channels( 0 )
    , m_sampleRate( 48000.0 )
    , m_threshold()
    , m_ratio()
    , m_knee()
    , m_attackCoefficient( 0.0 )
    , m_releaseCoefficient( 0.0 )
    , m_makeupGain( 1.0f )
    , m_makeupTarget( 1.0f )
    , m_detector( ENVELOPEMODE_PEAK )
    , m_rmsWindow( 10.0f )
    , m_link( 1.0f )
    , m_curveThreshold( -18.0f )
    , m_curveKnee( 6.0f )
    , m_curveSlope( 0.75f )
    , m_curveFloor( 0.0f )
    , m_followers()
    , m_smoothed()
    , m_reduction()
    , m_gainReduction( 0.0f )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCompressorEffect::~CCompressorEffect()
{GUCEF_TRACE;

    ReleaseFollowers();
}

/*-------------------------------------------------------------------------*/

const CString&
CCompressorEffect::GetEffectTypeName( void ) const
{GUCEF_TRACE;

    return GetStaticEffectTypeName();
}

/*-------------------------------------------------------------------------*/

void
CCompressorEffect::SetThresholdInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetParameter( PARAMETER_THRESHOLD, decibels );
}

/*-------------------------------------------------------------------------*/

void
CCompressorEffect::SetRatio( Float32 ratio )
{GUCEF_TRACE;

    SetParameter( PARAMETER_RATIO, ratio );
}

/*-------------------------------------------------------------------------*/

void
CCompressorEffect::SetKneeInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetParameter( PARAMETER_KNEE, decibels );
}

/*-------------------------------------------------------------------------*/

void
CCompressorEffect::SetAttackInMs( Float32 milliseconds )
{GUCEF_TRACE;

    SetParameter( PARAMETER_ATTACK, milliseconds );
}

/*-------------------------------------------------------------------------*/

void
CCompressorEffect::SetReleaseInMs( Float32 milliseconds )
{GUCEF_TRACE;

    SetParameter( PARAMETER_RELEASE, milliseconds );
}

/*-------------------------------------------------------------------------*/

void
CCompressorEffect::SetMakeupInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetParameter( PARAMETER_MAKEUP, decibels );
}

/*-------------------------------------------------------------------------*/

void
CCompressorEffect::SetDetector( TEnvelopeMode mode )
{GUCEF_TRACE;

    SetParameter( PARAMETER_DETECTOR, (Float32) mode );
}

/*-------------------------------------------------------------------------*/

void
CCompressorEffect::SetRmsWindowInMs( Float32 milliseconds )
{GUCEF_TRACE;

    SetParameter( PARAMETER_RMS_WINDOW, milliseconds );
}

/*-------------------------------------------------------------------------*/

void
CCompressorEffect::SetStereoLink( Float32 link )
{GUCEF_TRACE;

    SetParameter( PARAMETER_STEREO_LINK, link );
}

/*-------------------------------------------------------------------------*/

Float32
CCompressorEffect::GetGainReductionInDecibels( void ) const
{GUCEF_TRACE;

    return m_gainReduction.Load();
}

/*-------------------------------------------------------------------------*/

void
CCompressorEffect::ReleaseFollowers( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_followers.size(); ++i )
        GUCEF_DELETE m_followers[ i ];
    m_followers.clear();
}

/*-------------------------------------------------------------------------*/

bool
CCompressorEffect::OnPrepare( const CAudioFormat& format, UInt32 maxBlockFrames )
{GUCEF_TRACE;

    m_channels = format.GetChannelCount();
    m_sampleRate = (Float64) format.GetSampleRate();

    ReleaseFollowers();
    for ( UInt32 c=0; c<m_channels; ++c )
    {
        CEnvelopeFollower* follower = GUCEF_NEW CEnvelopeFollower();
        m_followers.push_back( follower );
        if ( !follower->Prepare( m_sampleRate ) )
            return false;
    }
    m_smoothed.assign( m_channels, 0.0 );
    m_reduction.assign( m_channels, 0.0f );

    m_threshold.Prepare( AUDIOSMOOTHING_ONE_POLE, CurveSmoothingSeconds, m_sampleRate, false, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    m_ratio.Prepare( AUDIOSMOOTHING_ONE_POLE, CurveSmoothingSeconds, m_sampleRate, false, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    m_knee.Prepare( AUDIOSMOOTHING_ONE_POLE, CurveSmoothingSeconds, m_sampleRate, false, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCompressorEffect::OnReset( void )
{
    HaveParametersChanged();
    ApplyParameters( true );
    for ( UInt32 c=0; c<m_channels; ++c )
    {
        m_followers[ c ]->Reset();
        m_smoothed[ c ] = 0.0;
    }
    m_gainReduction.Store( 0.0f );
}

/*-------------------------------------------------------------------------*/

void
CCompressorEffect::ApplyParameters( bool jump )
{
    if ( jump )
    {
        m_threshold.Reset( ReadParameter( PARAMETER_THRESHOLD ) );
        m_ratio.Reset( ReadParameter( PARAMETER_RATIO ) );
        m_knee.Reset( ReadParameter( PARAMETER_KNEE ) );
    }
    else
    {
        m_threshold.SetTarget( ReadParameter( PARAMETER_THRESHOLD ) );
        m_ratio.SetTarget( ReadParameter( PARAMETER_RATIO ) );
        m_knee.SetTarget( ReadParameter( PARAMETER_KNEE ) );
    }
    UpdateCurve();

    m_attackCoefficient = CoefficientFor( ReadParameter( PARAMETER_ATTACK ), m_sampleRate );
    m_releaseCoefficient = CoefficientFor( ReadParameter( PARAMETER_RELEASE ), m_sampleRate );
    m_link = ReadParameter( PARAMETER_STEREO_LINK );

    m_makeupTarget = (Float32) pow( 10.0, (Float64) ReadParameter( PARAMETER_MAKEUP ) / 20.0 );
    if ( jump )
        m_makeupGain = m_makeupTarget;

    const TEnvelopeMode detector = (TEnvelopeMode) (Int32) ReadParameter( PARAMETER_DETECTOR );
    const Float32 window = ReadParameter( PARAMETER_RMS_WINDOW );
    if ( jump || detector != m_detector || window != m_rmsWindow )
    {
        m_detector = detector;
        m_rmsWindow = window;
        for ( size_t c=0; c<m_followers.size(); ++c )
        {
            m_followers[ c ]->SetMode( detector );
            m_followers[ c ]->SetRmsWindowInMs( (Float64) window );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CCompressorEffect::UpdateCurve( void )
{
    const Float32 threshold = m_threshold.GetValue();
    const Float32 knee = m_knee.GetValue();
    const Float32 ratio = m_ratio.GetValue();
    m_curveThreshold = threshold;
    m_curveKnee = knee;
    m_curveSlope = ratio > 1.0f ? 1.0f - 1.0f / ratio : 0.0f;
    m_curveFloor = (Float32) pow( 10.0, (Float64) ( threshold - 0.5f * knee ) / 20.0 );
}

/*-------------------------------------------------------------------------*/

void
CCompressorEffect::StepSubBlock( void )
{
    if ( m_threshold.IsSmoothing() || m_ratio.IsSmoothing() || m_knee.IsSmoothing() )
    {
        m_threshold.Step();
        m_ratio.Step();
        m_knee.Step();
        UpdateCurve();
    }
    for ( size_t c=0; c<m_followers.size(); ++c )
        m_followers[ c ]->FlushDenormals();
}

/*-------------------------------------------------------------------------*/

Float32
CCompressorEffect::ReductionFor( Float32 level ) const
{
    // level is above the floor of the curve here
    const Float32 over = DecibelsPerOctave * log2f( level ) - m_curveThreshold;
    const Float32 knee = m_curveKnee;
    if ( knee > 0.0f )
    {
        if ( 2.0f * over < -knee )
            return 0.0f;
        if ( 2.0f * over <= knee )
        {
            const Float32 inKnee = over + 0.5f * knee;
            return m_curveSlope * inKnee * inKnee / ( 2.0f * knee );
        }
    }
    else
    if ( over <= 0.0f )
        return 0.0f;
    return m_curveSlope * over;
}

/*-------------------------------------------------------------------------*/

void
CCompressorEffect::OnProcess( Float32* inOut, UInt32 frames )
{
    if ( HaveParametersChanged() )
        ApplyParameters( false );

    const UInt32 channels = m_channels;
    const bool rms = ENVELOPEMODE_RMS == m_detector;
    const Float64 attack = m_attackCoefficient;
    const Float64 release = m_releaseCoefficient;
    const Float32 link = m_link;
    Float32* reduction = &m_reduction[ 0 ];
    Float64* smoothed = &m_smoothed[ 0 ];

    Float32 makeup = m_makeupGain;
    const Float32 makeupTarget = m_makeupTarget;
    const Float32 makeupStep = makeup != makeupTarget ? ( makeupTarget - makeup ) / (Float32) frames : 0.0f;

    UInt64 position = GetProcessedFrameCount();
    UInt32 done = 0;
    while ( done < frames )
    {
        const UInt32 inSubBlock = (UInt32) ( position % GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
        if ( 0 == inSubBlock )
            StepSubBlock();
        UInt32 run = GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES - inSubBlock;
        if ( run > frames - done )
            run = frames - done;

        for ( UInt32 f=0; f<run; ++f )
        {
            Float32* frame = inOut + (size_t) ( done + f ) * channels;
            if ( makeup != makeupTarget )
                makeup = ( done + f + 1 == frames ) ? makeupTarget : makeup + makeupStep;

            // what the gain computer asks for, per channel
            const Float32 floor = m_curveFloor;
            Float32 strongest = 0.0f;
            for ( UInt32 c=0; c<channels; ++c )
            {
                const Float32 input = DetectorInput( frame[ c ] );
                const Float32 level = rms ? m_followers[ c ]->Process( input ) : fabsf( input );
                const Float32 asked = level > floor ? ReductionFor( level ) : 0.0f;
                reduction[ c ] = asked;
                if ( asked > strongest )
                    strongest = asked;
            }

            // smooth with attack and release, apply
            Float32 lastReduction = -1.0f;
            Float32 lastGain = 1.0f;
            for ( UInt32 c=0; c<channels; ++c )
            {
                const Float64 asked = link >= 1.0f ? (Float64) strongest : (Float64) ( reduction[ c ] + link * ( strongest - reduction[ c ] ) );
                Float64 state = smoothed[ c ];
                state = asked + ( asked > state ? attack : release ) * ( state - asked );
                if ( state < ReductionFlushThreshold )
                    state = 0.0;
                smoothed[ c ] = state;

                if ( 0.0 == state )
                {
                    if ( makeup != 1.0f )
                        frame[ c ] *= makeup;
                }
                else
                {
                    const Float32 current = (Float32) state;
                    if ( current != lastReduction )
                    {
                        lastReduction = current;
                        lastGain = exp2f( -current / DecibelsPerOctave );
                    }
                    frame[ c ] *= lastGain * makeup;
                }
            }
        }

        done += run;
        position += run;
    }

    m_makeupGain = makeupTarget;
    Float64 strongestState = 0.0;
    for ( UInt32 c=0; c<channels; ++c )
        if ( smoothed[ c ] > strongestState )
            strongestState = smoothed[ c ];
    m_gainReduction.Store( (Float32) strongestState );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
