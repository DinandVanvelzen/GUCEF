/*
 *  gucefAUDIO_TestApp: GUCEF test application for the AUDIO module
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

#ifndef GUCEFAUDIO_H
#include "gucefAUDIO.h"
#define GUCEFAUDIO_H
#endif /* GUCEFAUDIO_H ? */

#ifndef GUCEF_AUDIO_CFIRDECIMATOR_H
#include "gucefAUDIO_CFirDecimator.h"
#define GUCEF_AUDIO_CFIRDECIMATOR_H
#endif /* GUCEF_AUDIO_CFIRDECIMATOR_H ? */

#ifndef GUCEF_AUDIO_CECHODELAYESTIMATOR_H
#include "gucefAUDIO_CEchoDelayEstimator.h"
#define GUCEF_AUDIO_CECHODELAYESTIMATOR_H
#endif /* GUCEF_AUDIO_CECHODELAYESTIMATOR_H ? */

#ifndef GUCEF_AUDIO_CDUCKEREFFECT_H
#include "gucefAUDIO_CDuckerEffect.h"
#define GUCEF_AUDIO_CDUCKEREFFECT_H
#endif /* GUCEF_AUDIO_CDUCKEREFFECT_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioVoiceSignals.h"
#include "TestAudioEchoSimulation.h"
#include "TestAudioIntercomScenario.h"
#include "TestAudioEchoCanceller.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace EchoTest;

namespace TVS = TestVoiceSignals;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

#if defined( __has_feature )
  #if __has_feature( realtime_sanitizer )
    #define GUCEF_AUDIO_TEST_ECHO_RTSAN_BUILD
  #endif
#endif

#ifdef GUCEF_AUDIO_TEST_ECHO_RTSAN_BUILD
const bool IsRtsanBuild = true;
#else
const bool IsRtsanBuild = false;
#endif

typedef AUDIO::CAcousticEchoCanceller TCanceller;
typedef AUDIO::CEchoCancellerSettings TSettings;

const UInt32 Rate = 16000;

Float64
DbToGain( Float64 decibels )
{
    return pow( 10.0, decibels / 20.0 );
}

/** One run of the canceller in the rig: what went in and what came out */
struct SEchoRun
{
    TSamples far;           /**< the reference, at the render clock */
    TSamples echo;          /**< the echo in the microphone */
    TSamples near;          /**< the near end in the microphone: talker and noise */
    TSamples mic;
    TSamples out;
    UInt32 latency;         /**< of the output against the microphone */
    SEchoRun( void ) : far(), echo(), near(), mic(), out(), latency( 0 ) {}

    /** What is left of the echo in the output over [from, to) of the microphone (the output moved back by the latency) */
    TSamples Residual( void ) const
    {
        TSamples residual( mic.size(), 0.0f );
        for ( size_t i=0; i<mic.size(); ++i )
        {
            const size_t j = i + latency;
            residual[ i ] = ( j < out.size() ? out[ j ] : 0.0f ) - near[ i ];
        }
        return residual;
    }

    /** The output moved back by the latency */
    TSamples Aligned( void ) const
    {
        TSamples aligned( mic.size(), 0.0f );
        for ( size_t i=0; i<mic.size() && i + latency<out.size(); ++i )
            aligned[ i ] = out[ i + latency ];
        return aligned;
    }
};

/**
 *  Runs 'canceller' (prepared by the rig) on a far talker through 'response' with the near end 'near' (may be empty) and
 *  white noise at 'noiseDb' in the microphone
 */
bool
RunEcho( TCanceller& canceller, const SEchoRigConfig& rig, const TSamples& far, const TResponse& response, const TSamples& near, Float64 noiseDb, SEchoRun& run )
{
    CEchoRig driver( canceller, rig );
    if ( !driver.Prepare() )
        return false;
    run.far = far;
    run.echo = Convolve( Played( far, rig, far.size() ), response );
    run.near = TVS::MakeWhiteNoise( far.size(), DbToGain( noiseDb ), 99 );
    for ( size_t i=0; i<near.size() && i<run.near.size(); ++i )
        run.near[ i ] += near[ i ];
    run.mic = Sum( run.echo, run.near );
    // room for the output to come out of the latency
    TSamples mic = run.mic;
    mic.resize( mic.size() + 512, 0.0f );
    driver.Run( far, mic, run.out );
    run.latency = canceller.GetLatencyInFrames();
    return true;
}

/** Settings for the linear filter alone (no residual suppression), so the echo return loss enhancement is that of the filter */
TSettings
LinearSettings( void )
{
    TSettings settings;
    settings.SetResidualSuppression( false );
    return settings;
}

/** ERLE per window of 'windowSeconds' from 'fromSeconds', logged in one line */
void
LogErleTrack( const char* what, const SEchoRun& run, Float64 fromSeconds, Float64 toSeconds, Float64 windowSeconds )
{
    const TSamples residual = run.Residual();
    CORE::CString line;
    for ( Float64 t=fromSeconds; t + windowSeconds <= toSeconds + 1.0e-9; t += windowSeconds )
    {
        const Float64 erle = ErleDb( run.echo, residual, (size_t) ( t * Rate ), (size_t) ( ( t + windowSeconds ) * Rate ) );
        line += CORE::CString( " " ) + CORE::ToString( floor( erle * 10.0 + 0.5 ) / 10.0 );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: " << what << ": ERLE per " << windowSeconds << " s from " << fromSeconds << " s:" << line );
}

/** The first time (s) from which every window of 'windowSeconds' with echo has an ERLE of at least 'level' up to 'toSeconds' */
Float64
ConvergenceSeconds( const SEchoRun& run, Float64 level, Float64 windowSeconds, Float64 toSeconds )
{
    const TSamples residual = run.Residual();
    Float64 since = -1.0;
    for ( Float64 t=0.0; t + windowSeconds <= toSeconds + 1.0e-9; t += windowSeconds )
    {
        const size_t a = (size_t) ( t * Rate ), b = (size_t) ( ( t + windowSeconds ) * Rate );
        if ( TVS::Rms( run.echo, a, b - a ) < 1.0e-5 )
            continue;   // a pause of the far end
        const Float64 erle = ErleDb( run.echo, residual, a, b );
        if ( erle >= level )
        {
            if ( since < 0.0 )
                since = t;
        }
        else
            since = -1.0;
    }
    return since;
}

/*-------------------------------------------------------------------------*/

void
PerformComponentTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: The render tap ring: in order, wraps, drops what does not fit and counts it, positions, skip, silence" )
    try
    {
        AUDIO::CAudioFrameRing ring;
        ASSERT_FALSE( ring.Prepare( 0 ) );
        ASSERT_TRUE( ring.Prepare( 100 ) );
        ASSERT_TRUE( 128 == ring.GetCapacity() && 128 == ring.GetFree() && 0 == ring.GetAvailable() );
        Float32 in[ 300 ], out[ 300 ];
        for ( UInt32 i=0; i<300; ++i )
            in[ i ] = (Float32) i;
        ASSERT_TRUE( 100 == ring.Write( in, 100 ) );
        ASSERT_TRUE( 60 == ring.Read( out, 60 ) && 59.0f == out[ 59 ] );
        // 88 fit, 12 are dropped
        ASSERT_TRUE( 88 == ring.Write( in + 100, 100 ) );
        ASSERT_TRUE( 12 == ring.GetDroppedFrameCount() && 0 == ring.GetFree() && 128 == ring.GetAvailable() );
        ASSERT_TRUE( 10 == ring.Skip( 10 ) );
        ASSERT_TRUE( 118 == ring.Read( out, 300 ) );
        bool inOrder = true;
        for ( UInt32 i=0; i<118; ++i )
            inOrder = inOrder && ( out[ i ] == (Float32) ( 70 + i ) );
        ASSERT_TRUE( inOrder );
        ASSERT_TRUE( 188 == ring.GetWritePosition() && 188 == ring.GetReadPosition() );
        ASSERT_TRUE( 5 == ring.WriteSilence( 5 ) && 5 == ring.Read( out, 5 ) && 0.0f == out[ 4 ] );
        ring.Reset();
        ASSERT_TRUE( 0 == ring.GetWritePosition() && 0 == ring.GetDroppedFrameCount() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: The reference decimator: 48 to 16 kHz flat to 3 kHz and within 1 dB at 6 kHz, aliases suppressed, block size invariant, silence costs nothing and stays exact" )
    try
    {
        AUDIO::CFirDecimator decimator;
        ASSERT_FALSE( decimator.Prepare( 0 ) );
        ASSERT_TRUE( decimator.Prepare( 3 ) );
        ASSERT_TRUE( 72 == decimator.GetTapCount() && 11 == decimator.GetLatencyInFrames() );
        const Float64 frequencies[] = { 200.0, 1000.0, 3000.0, 6000.0, 10000.0, 12000.0, 15000.0 };
        Float64 gains[ 7 ];
        for ( UInt32 f=0; f<7; ++f )
        {
            const TSamples tone = TVS::MakeTone( 48000, 48000, frequencies[ f ], 0.5 );
            TSamples out( 16010, 0.0f );
            decimator.Reset();
            const UInt32 produced = decimator.Process( &tone[ 0 ], (UInt32) tone.size(), &out[ 0 ] );
            ASSERT_TRUE( 16000 == produced );
            gains[ f ] = TVS::ToDecibels( TVS::Rms( out, 2000, 12000 ) / 0.5 );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: decimator 48 to 16 kHz: gain at 200 Hz " << gains[ 0 ] << ", 1 kHz " << gains[ 1 ] << ", 3 kHz " << gains[ 2 ] << ", 6 kHz " << gains[ 3 ] <<
                   " dB; aliases of 10, 12 and 15 kHz at " << gains[ 4 ] << ", " << gains[ 5 ] << ", " << gains[ 6 ] << " dB" );
        ASSERT_TRUE( fabs( gains[ 0 ] ) < 0.05 && fabs( gains[ 1 ] ) < 0.05 && fabs( gains[ 2 ] ) < 0.05 && fabs( gains[ 3 ] ) < 1.0 );
        ASSERT_TRUE( gains[ 4 ] < -50.0 && gains[ 5 ] < -50.0 && gains[ 6 ] < -50.0 );

        // the same output whatever the blocks
        const TSamples noise = TVS::MakeWhiteNoise( 9000, 0.3, 5 );
        TSamples a( 3100, 0.0f ), b( 3100, 0.0f );
        decimator.Reset();
        UInt32 na = decimator.Process( &noise[ 0 ], 9000, &a[ 0 ] );
        decimator.Reset();
        UInt32 nb = 0;
        for ( UInt32 at=0, step=1; at<9000; at += step, step = ( step * 7 + 3 ) % 500 + 1 )
        {
            const UInt32 count = at + step > 9000 ? 9000 - at : step;
            nb += decimator.Process( &noise[ at ], count, &b[ nb ] );
        }
        ASSERT_TRUE( na == 3000 && nb == 3000 );
        ASSERT_TRUE( 0 == memcmp( &a[ 0 ], &b[ 0 ], 3000 * sizeof( Float32 ) ) );

        // silence after the history ran out: zeros, and the phase goes on (a following tone comes out as from a fresh stream)
        TSamples silence( 6000, 0.0f ), zeros( 2100, 1.0f );
        const UInt32 nz = decimator.Process( &silence[ 0 ], 6000, &zeros[ 0 ] );
        ASSERT_TRUE( 2000 == nz );
        Float32 peak = 0.0f;
        for ( UInt32 i=100; i<2000; ++i )
            peak = fabsf( zeros[ i ] ) > peak ? fabsf( zeros[ i ] ) : peak;
        ASSERT_TRUE( 0.0f == peak );
        AUDIO::CFirDecimator one;
        ASSERT_TRUE( one.Prepare( 1 ) );
        TSamples copy( 100, 0.0f );
        ASSERT_TRUE( 100 == one.Process( &noise[ 0 ], 100, &copy[ 0 ] ) && 0 == memcmp( &copy[ 0 ], &noise[ 0 ], 100 * sizeof( Float32 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: The delay estimator finds bulk delays of 5 to 220 ms within one block (8 ms) after three consistent evaluations, and nothing on unrelated signals" )
    try
    {
        const Float64 delays[] = { 5.0, 20.0, 64.0, 100.0, 150.0, 220.0 };
        for ( UInt32 d=0; d<6; ++d )
        {
            AUDIO::CEchoDelayEstimator estimator;
            ASSERT_TRUE( estimator.Prepare( Rate, Rate / 4 ) );
            const TSamples far = MakeFarTalker( Rate, 8.0, 3 + d, -20.0 );
            const TResponse h = MakeEarlyReflections( Rate, delays[ d ], -15.0, 5 + d );
            TSamples mic = Convolve( far, h );
            TVS::MixInto( mic, TVS::MakeWhiteNoise( mic.size(), DbToGain( -65.0 ), 7 ) );
            Float64 foundAt = -1.0;
            for ( size_t at=0; at + 128 <= far.size(); at += 128 )
            {
                Float32 peak = 0.0f;
                for ( UInt32 i=0; i<128; ++i )
                    peak = fabsf( far[ at + i ] ) > peak ? fabsf( far[ at + i ] ) : peak;
                estimator.Process( &far[ at ], &mic[ at ], 128, peak > 0.0003f );
                if ( foundAt < 0.0 && estimator.HasEstimate() )
                    foundAt = (Float64) ( at + 128 ) / Rate;
            }
            const Float64 found = (Float64) estimator.GetDelayInFrames() * 1000.0 / Rate;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: delay estimator: " << delays[ d ] << " ms found as " << found << " ms after " << foundAt << " s, correlation " << estimator.GetCorrelation() <<
                       ", " << estimator.GetEstimateChangeCount() << " changes" );
            ASSERT_TRUE( estimator.HasEstimate() );
            ASSERT_TRUE( found >= delays[ d ] - 2.0 && found <= delays[ d ] + 8.0 );
            ASSERT_TRUE( foundAt > 0.0 && foundAt < 4.0 );
        }

        // a microphone that does not hear the reference
        AUDIO::CEchoDelayEstimator estimator;
        ASSERT_TRUE( estimator.Prepare( Rate, Rate / 4 ) );
        const TSamples far = MakeFarTalker( Rate, 8.0, 11, -20.0 );
        const TSamples other = MakeFarTalker( Rate, 8.0, 12, -20.0 );
        for ( size_t at=0; at + 128 <= far.size(); at += 128 )
            estimator.Process( &far[ at ], &other[ at ], 128, true );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: delay estimator on unrelated signals: estimate " << ( estimator.HasEstimate() ? "found" : "none" ) << ", correlation " << estimator.GetCorrelation() );
        ASSERT_FALSE( estimator.HasEstimate() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: The settings have the defaults of the design, limit their values and survive a save and load" )
    try
    {
        TSettings settings;
        ASSERT_TRUE( settings.GetEnabled() && 128.0f == settings.GetFilterLengthInMs() && 0 == settings.GetBlockFrames() && 1 == settings.GetAdaptEveryBlocks() );
        ASSERT_TRUE( settings.GetResidualSuppression() && -40.0f == settings.GetEchoSuppressionInDecibels() && 2.0f == settings.GetResidualStrength() );
        ASSERT_TRUE( !settings.GetSuppressNoise() && -15.0f == settings.GetNoiseSuppressionInDecibels() );
        ASSERT_TRUE( settings.GetDelayEstimation() && 250.0f == settings.GetMaxDelayInMs() && -70.0f == settings.GetFarThresholdInDecibels() );
        ASSERT_TRUE( settings.GetDriftCompensation() && 12.0f == settings.GetConvergedErleInDecibels() && 0.0f == settings.GetReferenceMarginInMs() );
        settings.SetFilterLengthInMs( 2000.0f );
        settings.SetBlockFrames( 100 );
        settings.SetAdaptEveryBlocks( 9 );
        const Float32 zero = 0.0f;
        settings.SetEchoSuppressionInDecibels( zero / zero );
        ASSERT_TRUE( 512.0f == settings.GetFilterLengthInMs() && 64 == settings.GetBlockFrames() && 4 == settings.GetAdaptEveryBlocks() && -40.0f == settings.GetEchoSuppressionInDecibels() );
        settings.SetBlockFrames( 5000 );
        ASSERT_TRUE( 1024 == settings.GetBlockFrames() );
        settings.SetBlockFrames( 0 );
        settings.SetFilterLengthInMs( 96.0f );
        settings.SetSuppressNoise( true );
        settings.SetMaxDelayInMs( 180.0f );
        settings.SetReferenceMarginInMs( 12.0f );
        settings.SetEnabled( false );
        CORE::CDataNode tree;
        ASSERT_TRUE( settings.SaveConfig( tree ) );
        TSettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( tree ) );
        ASSERT_TRUE( loaded == settings );
        ASSERT_TRUE( loaded != TSettings() );
        ASSERT_FALSE( loaded.LoadConfig( CORE::CDataNode( "Other" ) ) );

        AUDIO::CHalfDuplexSettings halfDuplex;
        ASSERT_TRUE( halfDuplex.GetEchoCancellerHandOver() && 18.0f == halfDuplex.GetHandOverErleInDecibels() );
        halfDuplex.SetHandOverErleInDecibels( 99.0f );
        halfDuplex.SetEchoCancellerHandOver( false );
        CORE::CDataNode hdTree;
        ASSERT_TRUE( halfDuplex.SaveConfig( hdTree ) );
        AUDIO::CHalfDuplexSettings hdLoaded;
        ASSERT_TRUE( hdLoaded.LoadConfig( hdTree ) && hdLoaded == halfDuplex && 40.0f == hdLoaded.GetHandOverErleInDecibels() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** Watches a canceller during a run: double talk, convergence and the drift, per span of the microphone */
class CCancellerProbe : public IEchoRigObserver
{
    public:

    CCancellerProbe( const TCanceller& canceller, UInt64 from, UInt64 to )
        : periods( 0 ), doubleTalk( 0 ), converged( 0 ), compensating( 0 ), m_canceller( canceller ), m_from( from ), m_to( to )
    {}

    virtual void OnPeriod( UInt64 captured ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( captured <= m_from || captured > m_to )
            return;
        ++periods;
        doubleTalk += m_canceller.IsDoubleTalk() ? 1 : 0;
        converged += m_canceller.IsConverged() ? 1 : 0;
        compensating += m_canceller.IsDriftCompensating() ? 1 : 0;
    }

    UInt64 periods;
    UInt64 doubleTalk;
    UInt64 converged;
    UInt64 compensating;

    private:

    const TCanceller& m_canceller;
    UInt64 m_from;
    UInt64 m_to;
};

/** RunEcho with a probe and a loudspeaker that may clip, and a response that changes at 'changeAt' seconds (0: never) */
bool
RunEchoWith( TCanceller& canceller, const SEchoRigConfig& rig, const TSamples& far, const TResponse& response, const TResponse& changed, Float64 changeAt,
             const TSamples& near, Float64 noiseDb, bool softClip, IEchoRigObserver* probe, SEchoRun& run )
{
    CEchoRig driver( canceller, rig, probe );
    if ( !driver.Prepare() )
        return false;
    run.far = far;
    TSamples played = Played( far, rig, far.size() );
    if ( softClip )
        played = SoftClip( played );
    run.echo = Convolve( played, response );
    if ( changeAt > 0.0 )
    {
        const TSamples after = Convolve( played, changed );
        for ( size_t i=(size_t) ( changeAt * Rate ); i<run.echo.size(); ++i )
            run.echo[ i ] = after[ i ];
    }
    run.near = TVS::MakeWhiteNoise( far.size(), DbToGain( noiseDb ), 99 );
    for ( size_t i=0; i<near.size() && i<run.near.size(); ++i )
        run.near[ i ] += near[ i ];
    run.mic = Sum( run.echo, run.near );
    TSamples mic = run.mic;
    mic.resize( mic.size() + 512, 0.0f );
    driver.Run( far, mic, run.out );
    run.latency = canceller.GetLatencyInFrames();
    return true;
}

/**
 *  The convergence time: the first moment (on a grid of 0.25 s from 'fromSeconds') from which the ERLE of every following
 *  second with echo is at least 'level' - 6 dB and that of all the rest of the run at least 'level'. -1 when there is none
 */
Float64
ConvergedFrom( const SEchoRun& run, Float64 level, Float64 fromSeconds, Float64 toSeconds )
{
    const TSamples residual = run.Residual();
    const size_t end = (size_t) ( toSeconds * Rate );
    for ( Float64 t=fromSeconds; t + 1.0 <= toSeconds + 1.0e-9; t += 0.25 )
    {
        if ( ErleDb( run.echo, residual, (size_t) ( t * Rate ), end ) < level )
            continue;
        bool every = true;
        for ( Float64 w=t; w + 1.0 <= toSeconds + 1.0e-9 && every; w += 0.5 )
        {
            const size_t a = (size_t) ( w * Rate ), b = (size_t) ( ( w + 1.0 ) * Rate );
            if ( TVS::Rms( run.echo, a, b - a ) < 0.003 )
                continue;   // hardly any echo: a pause of the far end
            every = ErleDb( run.echo, residual, a, b ) >= level - 6.0;
        }
        if ( every )
            return t;
    }
    return -1.0;
}

/*-------------------------------------------------------------------------*/

void
PerformConvergenceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: ERLE of the filter for echo paths of 5 to 150 ms and -6, -15 and -30 dB with early reflections, far end speech-like; convergence time" )
    try
    {
        const Float64 delays[] = { 5.0, 20.0, 50.0, 100.0, 150.0 };
        const Float64 gains[] = { -6.0, -15.0, -30.0 };
        const TSamples far = MakeFarTalker( Rate, 12.0, 21, -20.0 );
        for ( UInt32 d=0; d<5; ++d )
        {
            for ( UInt32 g=0; g<3; ++g )
            {
                if ( d != 1 && g != 1 )
                    continue;   // every delay at -15 dB, every attenuation at 20 ms
                TCanceller canceller( LinearSettings() );
                SEchoRun run;
                ASSERT_TRUE( RunEcho( canceller, SEchoRigConfig(), far, MakeEarlyReflections( Rate, delays[ d ], gains[ g ], 31 + d ), TSamples(), -80.0, run ) );
                const Float64 erle3 = ErleDb( run.echo, run.Residual(), 3 * Rate, 4 * Rate );
                const Float64 erle = ErleDb( run.echo, run.Residual(), 6 * Rate, 12 * Rate );
                const Float64 converged = ConvergedFrom( run, 20.0, 0.0, 12.0 );
                CORE::CString what = CORE::CString( "path " ) + CORE::ToString( delays[ d ] ) + " ms " + CORE::ToString( gains[ g ] ) + " dB";
                LogErleTrack( what.C_String(), run, 0.0, 12.0, 0.5 );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: " << what << ": ERLE 3 to 4 s " << erle3 << " dB, 6 to 12 s " << erle << " dB, converged to 20 dB (and every second above 14) from " << converged <<
                           " s, bulk delay " << canceller.GetBulkDelayInFrames() << " frames (estimate " << canceller.GetEstimatedDelayInFrames() << "), converged " << canceller.IsConverged() <<
                           ", divergences " << canceller.GetDivergenceCount() << ", ERLE estimate of the canceller " << canceller.GetErleInDecibels() << " dB" );
                ASSERT_TRUE( erle >= 24.0 );
                ASSERT_TRUE( converged >= 0.0 && converged <= 7.0 );
                ASSERT_TRUE( canceller.IsConverged() );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: Convergence on a broadband far end (white noise): 20 dB within 2 s, 35 dB after 6 s" )
    try
    {
        const TSamples far = TVS::MakeWhiteNoise( 8 * Rate, 0.05, 3 );
        TCanceller canceller( LinearSettings() );
        SEchoRun run;
        ASSERT_TRUE( RunEcho( canceller, SEchoRigConfig(), far, MakeEarlyReflections( Rate, 20.0, -15.0, 5 ), TSamples(), -90.0, run ) );
        const Float64 converged = ConvergenceSeconds( run, 20.0, 0.25, 8.0 );
        const Float64 erle = ErleDb( run.echo, run.Residual(), 6 * Rate, 8 * Rate );
        LogErleTrack( "white noise far end", run, 0.0, 8.0, 0.5 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: white noise far end: at 20 dB from " << converged << " s, ERLE 6 to 8 s " << erle << " dB" );
        ASSERT_TRUE( converged >= 0.0 && converged <= 2.0 );
        ASSERT_TRUE( erle >= 35.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: Reverberant rooms (T60 150 and 300 ms, 20 ms bulk delay) with filters of 128 and 256 ms, and a loudspeaker that clips (residual suppression on)" )
    try
    {
        const TSamples far = MakeFarTalker( Rate, 14.0, 23, -20.0 );
        const Float64 t60s[] = { 150.0, 300.0 };
        const Float32 lengths[] = { 128.0f, 256.0f };
        for ( UInt32 t=0; t<2; ++t )
        {
            for ( UInt32 l=0; l<2; ++l )
            {
                TSettings settings = LinearSettings();
                settings.SetFilterLengthInMs( lengths[ l ] );
                TCanceller canceller( settings );
                SEchoRun run;
                ASSERT_TRUE( RunEcho( canceller, SEchoRigConfig(), far, MakeReverberantRoom( Rate, 20.0, -15.0, t60s[ t ], 41 + t ), TSamples(), -80.0, run ) );
                const Float64 erle = ErleDb( run.echo, run.Residual(), 8 * Rate, 14 * Rate );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: room T60 " << t60s[ t ] << " ms, filter " << lengths[ l ] << " ms: ERLE 8 to 14 s " << erle << " dB" );
                ASSERT_TRUE( erle >= ( t60s[ t ] < 200.0 ? 20.0 : 15.0 ) );
            }
        }

        // a loudspeaker that compresses its peaks: the filter cannot follow it, the residual suppressor takes the rest
        const TResponse room = MakeEarlyReflections( Rate, 20.0, -15.0, 47 );
        TCanceller linear( LinearSettings() );
        SEchoRun linearRun;
        ASSERT_TRUE( RunEchoWith( linear, SEchoRigConfig(), far, room, TResponse(), 0.0, TSamples(), -80.0, true, GUCEF_NULL, linearRun ) );
        TCanceller full;
        SEchoRun fullRun;
        ASSERT_TRUE( RunEchoWith( full, SEchoRigConfig(), far, room, TResponse(), 0.0, TSamples(), -80.0, true, GUCEF_NULL, fullRun ) );
        const Float64 erleLinear = ErleDb( linearRun.echo, linearRun.Residual(), 8 * Rate, 14 * Rate );
        const Float64 total = ErleDb( fullRun.echo, fullRun.Residual(), 8 * Rate, 14 * Rate );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: clipping loudspeaker: ERLE of the filter " << erleLinear << " dB, with the residual suppressor " << total << " dB" );
        ASSERT_TRUE( erleLinear >= 10.0 );
        ASSERT_TRUE( total >= 15.0 && total >= erleLinear + 5.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: The residual echo suppressor: echo of the far end alone down by at least 35 dB in all, its latency as reported, and a 48 kHz reference through the render tap's decimator" )
    try
    {
        const TSamples far = MakeFarTalker( Rate, 12.0, 25, -20.0 );
        TCanceller canceller;
        ASSERT_TRUE( canceller.GetSettings().GetResidualSuppression() );
        SEchoRun run;
        ASSERT_TRUE( RunEcho( canceller, SEchoRigConfig(), far, MakeEarlyReflections( Rate, 20.0, -15.0, 51 ), TSamples(), -90.0, run ) );
        ASSERT_TRUE( 128 == run.latency && 128 == canceller.GetLatencyInFrames() );
        const Float64 total = ErleDb( run.echo, run.Residual(), 6 * Rate, 12 * Rate );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: residual suppressor: echo down by " << total << " dB in all (6 to 12 s), suppressor gain of its last frame " << canceller.GetResidualSuppressionInDecibels() << " dB" );
        ASSERT_TRUE( total >= 35.0 );

        // the reference at 48 kHz: the canceller decimates it to 16 kHz (the microphone's own path is the reference computation)
        AUDIO::CSincResampler down;
        ASSERT_TRUE( down.Configure( 1, 48000.0, 16000.0 ) );
        const TSamples far48 = MakeFarTalker( 48000, 12.0, 25, -20.0 );
        TSamples far16( far48.size() / 3 + 64, 0.0f );
        UInt32 consumed = 0, produced = 0;
        down.Process( &far48[ 0 ], (UInt32) far48.size(), consumed, &far16[ 0 ], (UInt32) far16.size(), produced );
        far16.resize( produced );
        // the decimator of the canceller and the sinc resampler differ in delay: the filter learns that too
        TCanceller decimating( LinearSettings() );
        ASSERT_TRUE( decimating.Prepare( Rate, 4096, 48000, 1, 4096 ) );
        SEchoRigConfig rig;
        const TSamples echo = Convolve( Played( far16, rig, far16.size() ), MakeEarlyReflections( Rate, 20.0, -15.0, 53 ) );
        TSamples out = echo;
        TVS::MixInto( out, TVS::MakeWhiteNoise( out.size(), DbToGain( -80.0 ), 5 ) );
        const TSamples mic = out;
        for ( size_t at=0; at + 80 <= mic.size() && 3 * ( at + 80 ) <= far48.size(); at += 80 )
        {
            decimating.WriteReference( &far48[ 3 * at ], 240 );
            decimating.Process( &out[ at ], 80, at );
        }
        const size_t end = mic.size() - mic.size() % 80;
        const Float64 erle48 = ErleDb( echo, Difference( out, Difference( mic, echo ) ), 6 * Rate, end );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: reference at 48 kHz through the decimator: ERLE 6 to 12 s " << erle48 << " dB, render calls " << decimating.GetRenderCallCount() );
        ASSERT_TRUE( erle48 >= 20.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformDoubleTalkAndChangeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: Double talk: the near end is preserved (level within 3 dB with the filter, measured with and without the residual suppressor), no divergence, the detector sees it, the ERLE is back after it" )
    try
    {
        const TSamples far = MakeFarTalker( Rate, 16.0, 61, -20.0, 0.5, 0.4 );
        const TResponse room = MakeEarlyReflections( Rate, 20.0, -15.0, 63 );
        // the near end talks from 8 to 11 s, 6 dB below the echo's far end level... at the microphone about as loud as the echo
        TSamples near( far.size(), 0.0f );
        const TSamples talk = MakeFarTalker( Rate, 3.0, 65, -30.0, 0.0, 0.2 );
        for ( size_t i=0; i<talk.size(); ++i )
            near[ 8 * Rate + i ] = talk[ i ];
        const bool residuals[] = { false, true };
        for ( UInt32 r=0; r<2; ++r )
        {
            TSettings settings;
            settings.SetResidualSuppression( residuals[ r ] );
            TCanceller canceller( settings );
            CCancellerProbe during( canceller, 8 * Rate, 11 * Rate );
            SEchoRun run;
            ASSERT_TRUE( RunEchoWith( canceller, SEchoRigConfig(), far, room, TResponse(), 0.0, near, -80.0, false, &during, run ) );
            const TSamples aligned = run.Aligned();
            const Float64 nearIn = TVS::ToDecibels( TVS::Rms( near, 8 * Rate, 3 * Rate ) );
            // the near end in the output: the output less the residual echo is not known with the suppressor, so the level of
            // the output along the near end (projection) is taken
            Float64 xy = 0.0, xx = 0.0;
            for ( size_t i=8 * Rate; i<11 * Rate; ++i )
            {
                xy += (Float64) aligned[ i ] * (Float64) near[ i ];
                xx += (Float64) near[ i ] * (Float64) near[ i ];
            }
            const Float64 kept = TVS::ToDecibels( xy / xx );
            const Float64 before = ErleDb( run.echo, run.Residual(), 6 * Rate, 8 * Rate );
            const Float64 after = ErleDb( run.echo, run.Residual(), 12 * Rate, 16 * Rate );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: double talk " << ( residuals[ r ] ? "with" : "without" ) << " the residual suppressor: near end " << nearIn << " dB, kept " << kept <<
                       " dB along itself; ERLE (all) " << before << " dB before, " << after << " dB after; double talk seen in " << during.doubleTalk << " of " << during.periods << " periods; divergences " <<
                       canceller.GetDivergenceCount() );
            ASSERT_TRUE( kept >= ( residuals[ r ] ? -6.0 : -3.0 ) && kept <= 1.0 );
            ASSERT_TRUE( after >= before - 3.0 || after >= 25.0 );
            ASSERT_TRUE( 0 == canceller.GetDivergenceCount() );
            ASSERT_TRUE( during.doubleTalk * 4 >= during.periods );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: A change of the echo path (another room, another delay) is followed: the ERLE drops and comes back to 20 dB" )
    try
    {
        const TSamples far = MakeFarTalker( Rate, 18.0, 71, -20.0, 0.5, 0.4 );
        const TResponse first = MakeEarlyReflections( Rate, 20.0, -15.0, 73 );
        const TResponse second = MakeEarlyReflections( Rate, 45.0, -12.0, 75 );
        TCanceller canceller( LinearSettings() );
        SEchoRun run;
        ASSERT_TRUE( RunEchoWith( canceller, SEchoRigConfig(), far, first, second, 9.0, TSamples(), -80.0, false, GUCEF_NULL, run ) );
        LogErleTrack( "path change at 9 s", run, 6.0, 18.0, 0.5 );
        const Float64 before = ErleDb( run.echo, run.Residual(), 7 * Rate, 9 * Rate );
        const Float64 recovered = ConvergedFrom( run, 20.0, 9.0, 18.0 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: path change: ERLE " << before << " dB before, converged to 20 dB again " << recovered - 9.0 << " s after the change; bulk delay " <<
                   canceller.GetBulkDelayInFrames() << " (estimate " << canceller.GetEstimatedDelayInFrames() << "), " << canceller.GetDelayChangeCount() << " moves, divergences " << canceller.GetDivergenceCount() );
        ASSERT_TRUE( before >= 20.0 );
        ASSERT_TRUE( recovered >= 9.0 && recovered - 9.0 <= 4.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: Background noise at the near end (white and pink, -50 dBFS): the filter still converges, the residual suppressor keeps the noise (comfort floor)" )
    try
    {
        const TSamples far = MakeFarTalker( Rate, 12.0, 81, -20.0, 0.5, 0.4 );
        const TResponse room = MakeEarlyReflections( Rate, 20.0, -15.0, 83 );
        for ( UInt32 pink=0; pink<2; ++pink )
        {
            const TSamples noise = pink ? TVS::MakePinkNoise( far.size(), DbToGain( -50.0 ), 85 ) : TVS::MakeWhiteNoise( far.size(), DbToGain( -50.0 ), 85 );
            TCanceller canceller;
            SEchoRun run;
            ASSERT_TRUE( RunEchoWith( canceller, SEchoRigConfig(), far, room, TResponse(), 0.0, noise, -120.0, false, GUCEF_NULL, run ) );
            const TSamples aligned = run.Aligned();
            const Float64 noiseIn = TVS::ToDecibels( TVS::Rms( noise, 6 * Rate, 6 * Rate ) );
            const Float64 noiseOut = TVS::ToDecibels( TVS::Rms( aligned, 6 * Rate, 6 * Rate ) );
            const Float64 echoLeft = ErleDb( run.echo, run.Residual(), 6 * Rate, 12 * Rate );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: " << ( pink ? "pink" : "white" ) << " noise at -50 dBFS: the output (echo and noise) " << noiseOut << " dB against the noise " << noiseIn <<
                       " dB; echo and its residual " << echoLeft << " dB apart; ERLE estimate " << canceller.GetErleInDecibels() << " dB, converged " << canceller.IsConverged() );
            ASSERT_TRUE( fabs( noiseOut - noiseIn ) <= 3.0 );
            ASSERT_TRUE( canceller.IsConverged() );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformDriftAndLongRunTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: Clock drift between playback and capture of +-100 ppm (10 minutes) and +-300 ppm (3 minutes): the drift is found, the reference resampled, the ERLE held" )
    try
    {
        const Float64 drifts[] = { 100.0, -100.0, 300.0, -300.0 };
        const Float64 minutes[] = { 10.0, 10.0, 3.0, 3.0 };
        for ( UInt32 d=0; d<4; ++d )
        {
            const Float64 seconds = 60.0 * minutes[ d ];
            const TSamples far = MakeFarTalker( Rate, seconds, 91 + d, -20.0, 0.5, 0.6 );
            SEchoRigConfig rig;
            rig.driftPpm = drifts[ d ];
            TCanceller canceller( LinearSettings() );
            SEchoRun run;
            ASSERT_TRUE( RunEcho( canceller, rig, far, MakeEarlyReflections( Rate, 20.0, -15.0, 93 ), TSamples(), -80.0, run ) );
            const TSamples residual = run.Residual();
            Float64 worst = 200.0;
            for ( Float64 t=60.0; t + 30.0 <= seconds + 1.0e-9; t += 30.0 )
            {
                const Float64 erle = ErleDb( run.echo, residual, (size_t) ( t * Rate ), (size_t) ( ( t + 30.0 ) * Rate ) );
                worst = erle < worst ? erle : worst;
            }
            const Float64 last = ErleDb( run.echo, residual, (size_t) ( ( seconds - 60.0 ) * Rate ), (size_t) ( seconds * Rate ) );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: drift " << drifts[ d ] << " ppm for " << minutes[ d ] << " minutes: found " << canceller.GetDriftInPpm() << " ppm, resampling " <<
                       canceller.IsDriftCompensating() << "; ERLE of the last minute " << last << " dB, worst 30 s after the first minute " << worst << " dB; reference level " <<
                       canceller.GetReferenceLevelInFrames() << " frames, underruns " << canceller.GetReferenceUnderrunCount() << ", alignments " << canceller.GetResyncCount() << ", divergences " << canceller.GetDivergenceCount() );
            ASSERT_TRUE( canceller.IsDriftCompensating() );
            ASSERT_TRUE( fabs( canceller.GetDriftInPpm() - drifts[ d ] ) <= 0.1 * fabs( drifts[ d ] ) );
            ASSERT_TRUE( last >= 20.0 );
            ASSERT_TRUE( worst >= 15.0 );
            ASSERT_TRUE( 1 == canceller.GetResyncCount() );
        }

        // the same clock: the reference is never resampled
        const TSamples far = MakeFarTalker( Rate, 60.0, 99, -20.0 );
        TCanceller same( LinearSettings() );
        SEchoRun run;
        ASSERT_TRUE( RunEcho( same, SEchoRigConfig(), far, MakeEarlyReflections( Rate, 20.0, -15.0, 93 ), TSamples(), -80.0, run ) );
        ASSERT_FALSE( same.IsDriftCompensating() );
        ASSERT_TRUE( 0.0f == same.GetDriftInPpm() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: One hour of simulated call (a far talker, double talk bursts every two minutes, noise): no divergence, the ERLE held to the end" )
    try
    {
        // GUCEF_TEST_AUDIO_ECHO_LONG_MINUTES shortens it for a quick run
        Float64 minutes = 60.0;
        const CORE::CString asked = GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_ECHO_LONG_MINUTES" ).c_str();
        if ( !asked.IsNULLOrEmpty() )
            minutes = CORE::StringToDouble( asked, 60.0 );
        const UInt64 total = (UInt64) ( minutes * 60.0 * Rate );

        // a minute of far talker, repeated; a sparse echo path; near bursts of 5 s; generated as the call goes
        const TSamples far = MakeFarTalker( Rate, 60.0, 101, -20.0, 0.5, 0.6 );
        const TSamples burst = MakeFarTalker( Rate, 5.0, 103, -28.0, 0.0, 0.2 );
        const TSamples noise = TVS::MakeWhiteNoise( 7 * Rate + 13, DbToGain( -65.0 ), 105 );
        const UInt32 taps[] = { 560, 571, 640, 760 };      // the playback buffer of 240 frames and a room of 20 ms
        const Float32 gains[] = { 0.18f, -0.08f, 0.05f, -0.03f };
        TCanceller canceller( LinearSettings() );
        ASSERT_TRUE( canceller.Prepare( Rate, 4096, Rate, 1, 4096 ) );
        Float32 reference[ 80 ], mic[ 80 ], echo[ 80 ];
        Float64 minuteEcho = 0.0, minuteResidual = 0.0, worst = 200.0, last = 0.0;
        UInt32 minuteCount = 0;
        for ( UInt64 at=0; at<total; at += 80 )
        {
            for ( UInt32 i=0; i<80; ++i )
            {
                const UInt64 n = at + i;
                reference[ i ] = far[ (size_t) ( n % far.size() ) ];
                Float32 e = 0.0f;
                for ( UInt32 t=0; t<4; ++t )
                    if ( n >= taps[ t ] )
                        e += gains[ t ] * far[ (size_t) ( ( n - taps[ t ] ) % far.size() ) ];
                echo[ i ] = e;
                const UInt64 inMinute = n % ( 120 * Rate );
                const Float32 near = ( inMinute >= 60 * Rate && inMinute < 65 * Rate ) ? burst[ (size_t) ( inMinute - 60 * Rate ) ] : 0.0f;
                mic[ i ] = e + near + noise[ (size_t) ( n % noise.size() ) ];
            }
            const Float32 nearAndNoise0 = mic[ 0 ] - echo[ 0 ];
            (void) nearAndNoise0;
            Float32 nearPart[ 80 ];
            for ( UInt32 i=0; i<80; ++i )
                nearPart[ i ] = mic[ i ] - echo[ i ];
            canceller.WriteReference( reference, 80 );
            canceller.Process( mic, 80, at );
            for ( UInt32 i=0; i<80; ++i )
            {
                const Float64 r = (Float64) mic[ i ] - (Float64) nearPart[ i ];
                minuteEcho += (Float64) echo[ i ] * (Float64) echo[ i ];
                minuteResidual += r * r;
            }
            if ( 0 == ( at + 80 ) % ( 60 * Rate ) )
            {
                ++minuteCount;
                const Float64 erle = minuteResidual > 0.0 ? 10.0 * log10( minuteEcho / minuteResidual ) : 200.0;
                if ( minuteCount > 1 && erle < worst )
                    worst = erle;
                last = erle;
                minuteEcho = 0.0;
                minuteResidual = 0.0;
            }
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: " << minutes << " minutes of call: ERLE of the worst minute after the first " << worst << " dB, of the last " << last << " dB; divergences " <<
                   canceller.GetDivergenceCount() << ", alignments " << canceller.GetResyncCount() << ", underruns " << canceller.GetReferenceUnderrunCount() << ", blocks " << canceller.GetBlockCount() <<
                   " (adapted " << canceller.GetAdaptedBlockCount() << ", idle " << canceller.GetIdleBlockCount() << ")" );
        ASSERT_TRUE( 0 == canceller.GetDivergenceCount() );
        ASSERT_TRUE( 1 == canceller.GetResyncCount() );
        ASSERT_TRUE( last >= 20.0 && worst >= 15.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** Real time contexts for the canceller's two sides, so a RealtimeSanitizer build checks them */
void
WriteReferenceRealtime( TCanceller& canceller, const Float32* frames, UInt32 count ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    canceller.WriteReference( frames, count );
}

void
ProcessRealtime( TCanceller& canceller, Float32* frames, UInt32 count, UInt64 first ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    canceller.Process( frames, count, first );
}

void
PerformCostAndBypassTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: Switched off: the microphone passes bit for bit, neither side computes anything (no FFT, nothing in the render tap); back on it aligns again and cancels" )
    try
    {
        const TSamples far = MakeFarTalker( Rate, 12.0, 111, -20.0 );
        SEchoRigConfig rig;
        const TSamples echo = Convolve( Played( far, rig, far.size() ), MakeEarlyReflections( Rate, 20.0, -15.0, 113 ) );
        TCanceller canceller( LinearSettings() );
        ASSERT_TRUE( canceller.Prepare( Rate, 4096, Rate, 1, 4096 ) );
        canceller.SetEnabled( false );
        ASSERT_FALSE( canceller.IsEnabled() );
        TSamples out = echo;
        for ( size_t at=0; at + 80 <= 4 * Rate; at += 80 )
        {
            canceller.WriteReference( &far[ at ], 80 );
            canceller.Process( &out[ at ], 80, at );
        }
        ASSERT_TRUE( 0 == memcmp( &out[ 0 ], &echo[ 0 ], 4 * Rate * sizeof( Float32 ) ) );
        ASSERT_TRUE( 0 == canceller.GetTransformCount() && 0 == canceller.GetBlockCount() && 0 == canceller.GetResyncCount() );
        ASSERT_TRUE( 4 * Rate / 80 == canceller.GetRenderCallCount() && 4 * Rate / 80 == canceller.GetCaptureCallCount() );
        ASSERT_TRUE( AUDIO::ECHOCANCELLERSTATE_OFF == canceller.GetState() );

        canceller.SetEnabled( true );
        for ( size_t at=4 * Rate; at + 80 <= out.size(); at += 80 )
        {
            canceller.WriteReference( &far[ at ], 80 );
            canceller.Process( &out[ at ], 80, at );
        }
        const Float64 erle = ErleDb( echo, out, 10 * Rate, 12 * Rate );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: back on after 4 s off: " << canceller.GetResyncCount() << " alignment, ERLE 10 to 12 s " << erle << " dB, state " << (UInt32) canceller.GetState() );
        ASSERT_TRUE( 1 == canceller.GetResyncCount() );
        ASSERT_TRUE( erle >= 15.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: Far end silent (near end only): nothing is computed (no FFT, idle blocks); without the residual suppressor the microphone passes bit for bit, with it delayed by its latency" )
    try
    {
        // 4 s with the far end quiet (the near end talks), then the far end
        const TSamples talk = MakeFarTalker( Rate, 8.0, 121, -20.0 );
        TSamples far( talk.size(), 0.0f );
        for ( size_t i=4 * Rate; i<far.size(); ++i )
            far[ i ] = talk[ i - 4 * Rate ];
        const TSamples near = MakeFarTalker( Rate, 4.0, 123, -26.0, 0.0, 0.3 );
        for ( UInt32 residual=0; residual<2; ++residual )
        {
            TSettings settings;
            settings.SetResidualSuppression( 1 == residual );
            TCanceller canceller( settings );
            ASSERT_TRUE( canceller.Prepare( Rate, 4096, Rate, 1, 4096 ) );
            const UInt32 latency = canceller.GetLatencyInFrames();
            TSamples mic( far.size(), 0.0f );
            for ( size_t i=0; i<near.size(); ++i )
                mic[ i ] = near[ i ];
            TSamples out = mic;
            UInt64 transformsQuiet = 0, blocksQuiet = 0, idleQuiet = 0;
            for ( size_t at=0; at + 80 <= out.size(); at += 80 )
            {
                WriteReferenceRealtime( canceller, &far[ at ], 80 );
                ProcessRealtime( canceller, &out[ at ], 80, at );
                if ( at + 80 == 4 * Rate )
                {
                    transformsQuiet = canceller.GetTransformCount();
                    blocksQuiet = canceller.GetBlockCount();
                    idleQuiet = canceller.GetIdleBlockCount();
                    ASSERT_TRUE( AUDIO::ECHOCANCELLERSTATE_IDLE == canceller.GetState() );
                }
            }
            Float64 worst = 0.0;
            for ( size_t i=0; i + latency<4 * Rate; ++i )
            {
                const Float64 d = fabs( (Float64) out[ i + latency ] - (Float64) mic[ i ] );
                worst = d > worst ? d : worst;
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: far end silent " << ( residual ? "with" : "without" ) << " the residual suppressor: " << transformsQuiet << " FFTs and " << idleQuiet <<
                       " idle of " << blocksQuiet << " blocks in 4 s, the output differs from the delayed microphone by at most " << worst << "; FFTs once the far end talks " <<
                       canceller.GetTransformCount() - transformsQuiet );
            ASSERT_TRUE( 0 == transformsQuiet );
            ASSERT_TRUE( idleQuiet == blocksQuiet && blocksQuiet > 400 );
            if ( 0 == residual )
                ASSERT_TRUE( 0 == memcmp( &out[ 0 ], &mic[ 0 ], 4 * Rate * sizeof( Float32 ) ) );
            else
                ASSERT_TRUE( worst < 1.0e-6 );
            ASSERT_TRUE( canceller.GetTransformCount() > transformsQuiet );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: Block size invariance: capture calls of 1, 7, 33, 40, 64 and 80 frames within each period of 80 give the same output bit for bit" )
    try
    {
        const TSamples far = MakeFarTalker( Rate, 6.0, 131, -20.0 );
        const TResponse room = MakeEarlyReflections( Rate, 20.0, -15.0, 133 );
        TSettings settings;
        settings.SetReferenceMarginInMs( 40.0f );
        // every period of 80 frames is processed whole, in calls of up to this many frames (blocks are 128)
        const UInt32 chunks[] = { 80, 1, 7, 33, 40, 64 };
        TSamples first;
        for ( UInt32 c=0; c<6; ++c )
        {
            SEchoRigConfig rig;
            rig.captureChunk = chunks[ c ];
            TCanceller canceller( settings );
            SEchoRun run;
            ASSERT_TRUE( RunEcho( canceller, rig, far, room, TSamples(), -70.0, run ) );
            const TSamples out( run.out.begin(), run.out.begin() + 5 * Rate );
            if ( 0 == c )
                first = out;
            else
            {
                size_t differ = 0;
                while ( differ < out.size() && out[ differ ] == first[ differ ] )
                    ++differ;
                if ( differ < out.size() )
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: block size invariance: calls of " << chunks[ c ] << " frames differ from frame " << differ << " on; drift " << canceller.GetDriftInPpm() << " ppm, alignments " << canceller.GetResyncCount() );
                ASSERT_TRUE( differ == out.size() );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: No reference yet (the capture side runs first, nothing plays): the microphone passes, the canceller waits; a gap in the microphone or a dropped reference aligns again" )
    try
    {
        TCanceller canceller( LinearSettings() );
        ASSERT_TRUE( canceller.Prepare( Rate, 4096, Rate, 1, 4096 ) );
        TSamples mic = TVS::MakeWhiteNoise( Rate, 0.1, 3 );
        const TSamples original = mic;
        for ( size_t at=0; at + 80 <= mic.size(); at += 80 )
            ProcessRealtime( canceller, &mic[ at ], 80, at );
        ASSERT_TRUE( 0 == memcmp( &mic[ 0 ], &original[ 0 ], mic.size() * sizeof( Float32 ) ) );
        ASSERT_TRUE( AUDIO::ECHOCANCELLERSTATE_WAITING == canceller.GetState() && 0 == canceller.GetResyncCount() );

        // the reference starts: aligned once
        const TSamples far = MakeFarTalker( Rate, 4.0, 141, -20.0 );
        TSamples work = TVS::MakeWhiteNoise( far.size(), 0.01, 5 );
        UInt64 position = Rate;
        for ( size_t at=0; at + 80 <= 2 * Rate; at += 80, position += 80 )
        {
            WriteReferenceRealtime( canceller, &far[ at ], 80 );
            ProcessRealtime( canceller, &work[ at ], 80, position );
        }
        ASSERT_TRUE( 1 == canceller.GetResyncCount() );
        // a gap of the microphone (an overrun skipped 1000 frames)
        position += 1000;
        for ( size_t at=2 * Rate; at + 80 <= 3 * Rate; at += 80, position += 80 )
        {
            WriteReferenceRealtime( canceller, &far[ at ], 80 );
            ProcessRealtime( canceller, &work[ at ], 80, position );
        }
        ASSERT_TRUE( 2 == canceller.GetResyncCount() );
        // the capture side stalls while the render side goes on: the ring overflows, the reference has a gap
        for ( UInt32 i=0; i<400; ++i )
            WriteReferenceRealtime( canceller, &far[ 0 ], 80 );
        for ( size_t at=3 * Rate; at + 80 <= 4 * Rate; at += 80, position += 80 )
        {
            WriteReferenceRealtime( canceller, &far[ at ], 80 );
            ProcessRealtime( canceller, &work[ at ], 80, position );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: alignments after the start, a capture gap and a reference overflow: " << canceller.GetResyncCount() );
        ASSERT_TRUE( 3 == canceller.GetResyncCount() );
        canceller.Reset();
        ASSERT_TRUE( 0 == canceller.GetResyncCount() && 0 == canceller.GetTransformCount() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: Real time safety of both sides: the render tap (with decimation) and the capture side in every state allocate, lock and block nowhere (RealtimeSanitizer builds)" )
    if ( !IsRtsanBuild )
    {
        GUCEF_TESTFW_SKIP( "not a RealtimeSanitizer build (-fsanitize=realtime)" );
    }
    try
    {
        // 48 kHz stereo reference, drift (the interpolator), double talk, the residual suppressor with noise suppression
        TSettings settings;
        settings.SetSuppressNoise( true );
        TCanceller canceller( settings );
        ASSERT_TRUE( canceller.Prepare( Rate, 4096, 48000, 2, 1024 ) );
        const TSamples far = MakeFarTalker( Rate, 20.0, 151, -20.0 );
        SEchoRigConfig rig;
        rig.driftPpm = 400.0;
        TSamples mic = Convolve( Played( far, rig, far.size() ), MakeEarlyReflections( Rate, 20.0, -15.0, 153 ) );
        TVS::MixInto( mic, MakeFarTalker( Rate, 20.0, 155, -30.0, 6.0, 1.0 ) );
        GUCEF::vector< Float32 > stereo( 2 * 3 * 81, 0.0f );
        Float64 carry = 0.0;
        size_t rendered = 0;
        for ( size_t at=0; at + 80 <= mic.size(); at += 80 )
        {
            carry += 80.0 * ( 1.0 + rig.driftPpm * 1.0e-6 );
            const UInt32 frames = (UInt32) floor( carry );
            carry -= frames;
            for ( UInt32 i=0; i<3 * frames; ++i )
            {
                const size_t r = ( rendered * 3 + i ) / 3;
                stereo[ 2 * i ] = stereo[ 2 * i + 1 ] = r < far.size() ? far[ r ] : 0.0f;
            }
            rendered += frames;
            WriteReferenceRealtime( canceller, &stereo[ 0 ], 3 * frames );
            ProcessRealtime( canceller, &mic[ at ], 80, at );
            if ( at == 10 * Rate )
                canceller.SetEnabled( false );
            if ( at == 11 * Rate )
                canceller.SetEnabled( true );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: real time run: " << canceller.GetBlockCount() << " blocks, drift " << canceller.GetDriftInPpm() << " ppm, resampling " << canceller.IsDriftCompensating() );
        ASSERT_TRUE( canceller.GetAdaptedBlockCount() > 0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** A mono clip of a signal at 16 kHz, to play on an engine */
AUDIO::TAudioClipPtr
MakeClip( const TSamples& samples )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( Rate, 1, AUDIO::SAMPLETYPE_FLOAT32 ), (UInt32) samples.size() );
    for ( UInt32 f=0; f<(UInt32) samples.size(); ++f )
        audio.SetSampleFromFloat( f, 0, samples[ f ] );
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

/** A unit at 16 kHz with 5 ms periods (80 frames) */
VoiceLinkTest::SUnitConfig
UnitConfig16( void )
{
    VoiceLinkTest::SUnitConfig config;
    config.sampleRate = Rate;
    config.period = 80;
    config.keepOutput = false;
    return config;
}

/** One period of a unit: capture, play, update */
void
StepUnit( VoiceLinkTest::CTestIntercomUnit& unit )
{
    unit.Capture( 80 );
    unit.Play( 80 );
    unit.engine->Update();
}

/** Plays the device of a threaded engine: render periods until told to stop */
class CRenderThread : public MT::CActiveObject
{
    public:

    explicit CRenderThread( AUDIO::CNullAudioDevice& device ) : MT::CActiveObject(), stop( 0 ), periods( 0 ), m_device( device ) {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        while ( 0 == stop.Load() )
        {
            m_device.Render( 80 );
            m_device.ClearRenderedAudio();
            periods.FetchAdd( 1 );
            MT::PrecisionDelay( 1 );
        }
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    MT::CTAtomic< UInt32 > stop;
    MT::CTAtomic< UInt32 > periods;

    private:

    AUDIO::CNullAudioDevice& m_device;
};

/** Records with the capture device of a threaded engine: a feed of speech-like sound, period by period, until told to stop */
class CCaptureThread : public MT::CActiveObject
{
    public:

    CCaptureThread( AUDIO::CNullAudioDevice& device, VoiceLinkTest::CTestFeedSource& feed, const TSamples& sound )
        : MT::CActiveObject(), stop( 0 ), periods( 0 ), m_device( device ), m_feed( feed ), m_sound( sound ), m_at( 0 )
    {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        while ( 0 == stop.Load() )
        {
            for ( UInt32 i=0; i<80; ++i )
                m_feed.Push( m_sound[ ( m_at++ ) % m_sound.size() ] );
            m_device.Render( 80 );
            periods.FetchAdd( 1 );
            MT::PrecisionDelay( 1 );
        }
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    MT::CTAtomic< UInt32 > stop;
    MT::CTAtomic< UInt32 > periods;

    private:

    AUDIO::CNullAudioDevice& m_device;
    VoiceLinkTest::CTestFeedSource& m_feed;
    const TSamples& m_sound;
    size_t m_at;
};

void
PerformEngineTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: In the engine: set, replace and remove with futures (both threads), refusals, the render tap fed once per mix and the canceller called once per capture period: no wake-ups of its own; nothing computed while nothing plays" )
    try
    {
        // no capture: refused
        VoiceLinkTest::SUnitConfig playOnly = UnitConfig16();
        playOnly.capture = false;
        VoiceLinkTest::CTestIntercomUnit silent( playOnly );
        ASSERT_TRUE( silent.Init() );
        TCanceller refused;
        ASSERT_TRUE( silent.engine->SetEchoCanceller( &refused ).HasNoFuture() );

        VoiceLinkTest::CTestIntercomUnit unit( UnitConfig16() );
        ASSERT_TRUE( unit.Init() );
        TCanceller a, b;
        CORE::CFutureResult set = unit.engine->SetEchoCanceller( &a );
        ASSERT_FALSE( set.HasNoFuture() );
        ASSERT_FALSE( set.IsReady() );
        ASSERT_TRUE( unit.engine->GetEchoCanceller() == &a );
        ASSERT_TRUE( unit.engine->SetEchoCanceller( &a ).IsReady() );       // the one in use
        ASSERT_TRUE( a.IsPrepared() && Rate == a.GetSampleRate() && Rate == a.GetReferenceRate() && 1 == a.GetReferenceChannels() );
        StepUnit( unit );
        ASSERT_TRUE( set.IsReady() && !set.HasNoFuture() );

        // replaced: the one before is leaving until both threads let go of it
        CORE::CFutureResult replaced = unit.engine->SetEchoCanceller( &b );
        ASSERT_FALSE( replaced.HasNoFuture() );
        ASSERT_TRUE( unit.engine->SetEchoCanceller( &a ).HasNoFuture() );
        StepUnit( unit );
        ASSERT_TRUE( replaced.IsReady() );
        CORE::CFutureResult removed = unit.engine->SetEchoCanceller( GUCEF_NULL );
        StepUnit( unit );
        ASSERT_TRUE( removed.IsReady() && GUCEF_NULL == unit.engine->GetEchoCanceller() );

        // wake-ups: one call of each side per device period, nothing else; nothing plays: no FFT
        CORE::CFutureResult again = unit.engine->SetEchoCanceller( &a );
        StepUnit( unit );
        ASSERT_TRUE( again.IsReady() && !again.HasNoFuture() );
        const UInt64 renderBefore = a.GetRenderCallCount();
        const UInt64 captureBefore = a.GetCaptureCallCount();
        for ( UInt32 i=0; i<1000; ++i )
            StepUnit( unit );
        const UInt64 renderCalls = a.GetRenderCallCount() - renderBefore;
        const UInt64 captureCalls = a.GetCaptureCallCount() - captureBefore;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: in the engine, 5 s with nothing playing: " << renderCalls << " render tap calls and " << captureCalls << " capture calls for 1000 periods of each device (" <<
                   ( renderCalls + captureCalls ) / 5 << " calls per second, the device callbacks), " << a.GetTransformCount() << " FFTs, state " << (UInt32) a.GetState() );
        ASSERT_TRUE( 1000 == renderCalls && 1000 == captureCalls );
        ASSERT_TRUE( 0 == a.GetTransformCount() );

        // the far end plays: the canceller works, still once per period
        unit.SetEcho( DbToGain( -15.0 ), 320 );
        ASSERT_FALSE( unit.engine->Play( MakeClip( MakeFarTalker( Rate, 6.0, 161, -20.0 ) ) ).HasNoFuture() );
        for ( UInt32 i=0; i<1200; ++i )
            StepUnit( unit );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: in the engine, 6 s of far end: " << a.GetTransformCount() << " FFTs, " << a.GetBlockCount() << " blocks, ERLE estimate " << a.GetErleInDecibels() <<
                   " dB, converged " << a.IsConverged() << ", bulk delay " << a.GetBulkDelayInFrames() << " (estimate " << a.GetEstimatedDelayInFrames() << ")" );
        ASSERT_TRUE( 2200 == a.GetRenderCallCount() - renderBefore && 2200 == a.GetCaptureCallCount() - captureBefore );
        ASSERT_TRUE( a.GetTransformCount() > 0 && a.GetAdaptedBlockCount() > 0 );
        ASSERT_TRUE( a.GetErleInDecibels() >= 15.0 );
        unit.Shutdown();
        ASSERT_TRUE( GUCEF_NULL == unit.engine );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: Threaded engine: a render thread and a capture thread run the canceller while the owner replaces it, switches it and reads it (ThreadSanitizer)" )
    try
    {
        VoiceLinkTest::SUnitConfig config = UnitConfig16();
        config.threading = AUDIO::AUDIOTHREADING_THREADED;
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( VoiceLinkTest::CTestIntercomUnit::MakeEngineParams( config ) ) );
        AUDIO::CNullAudioDevice* playback = engine.GetDevice()->AsNullDevice();
        AUDIO::CNullAudioDevice* capture = engine.GetCaptureDevice()->AsNullDevice();
        ASSERT_TRUE( GUCEF_NULL != playback && GUCEF_NULL != capture );
        VoiceLinkTest::CTestFeedSource* feed = GUCEF_NEW VoiceLinkTest::CTestFeedSource( Rate );
        AUDIO::TAudioSourcePtr feedPtr( feed );
        ASSERT_TRUE( capture->SetCaptureSource( feedPtr ) );
        const TSamples far = MakeFarTalker( Rate, 30.0, 171, -20.0 );
        TSamples sound = TVS::MakeWhiteNoise( Rate, 0.01, 3 );
        TVS::MixInto( sound, TSamples( far.begin(), far.begin() + Rate ), 0.2 );
        ASSERT_FALSE( engine.Play( MakeClip( far ) ).HasNoFuture() );

        TCanceller cancellers[ 2 ];
        CORE::CFutureResult set = engine.SetEchoCanceller( &cancellers[ 0 ] );
        CRenderThread render( *playback );
        CCaptureThread record( *capture, *feed, sound );
        ASSERT_TRUE( render.Activate() );
        ASSERT_TRUE( record.Activate() );
        ASSERT_TRUE( engine.WaitFor( set, 5000 ) );
        UInt32 replaced = 0;
        for ( UInt32 i=0; i<60; ++i )
        {
            MT::PrecisionDelay( 10 );
            TCanceller* current = engine.GetEchoCanceller();
            if ( GUCEF_NULL != current )
            {
                current->SetEnabled( 0 != i % 9 );
                (void) current->GetState();
                (void) current->GetErleInDecibels();
                (void) current->GetBulkDelayInFrames();
                (void) current->IsDoubleTalk();
                (void) current->GetReferenceLevelInFrames();
                (void) current->GetTransformCount();
                (void) current->GetDriftInPpm();
            }
            if ( 0 == i % 15 )
            {
                TCanceller* next = current == &cancellers[ 0 ] ? &cancellers[ 1 ] : &cancellers[ 0 ];
                if ( engine.WaitFor( engine.SetEchoCanceller( next ), 5000 ) )
                    ++replaced;
            }
        }
        ASSERT_TRUE( engine.WaitFor( engine.SetEchoCanceller( GUCEF_NULL ), 5000 ) );
        record.stop.Store( 1 );
        render.stop.Store( 1 );
        record.WaitForThreadToFinish( 10000 );
        render.WaitForThreadToFinish( 10000 );
        engine.Shutdown();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: threads: " << render.periods.Load() << " render and " << record.periods.Load() << " capture periods, " << replaced << " replacements; blocks " <<
                   cancellers[ 0 ].GetBlockCount() << " and " << cancellers[ 1 ].GetBlockCount() );
        ASSERT_TRUE( replaced >= 3 && render.periods.Load() > 100 && record.periods.Load() > 100 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** What the intercom call measures in the output of B */
struct SCallResult
{
    Float64 echoDb;             /**< B's own voice coming back (level of B's output against B's voice) where only B talks, late in the call */
    Float64 echoAlongDb;        /**< the same as the part along B's voice (projection) */
    Float64 doubleTalkEchoDb;   /**< B's voice coming back while A talks over B (level along B's voice) */
    Float64 aloneDb;            /**< A's voice at B where A talks alone */
    Float64 doubleTalkDb;       /**< A's voice at B where A talks over B */
    UInt64 handOvers;
    bool handedOver;
    Float64 erleDb;
    SCallResult( void ) : echoDb( 0.0 ), echoAlongDb( 0.0 ), doubleTalkEchoDb( 0.0 ), aloneDb( 0.0 ), doubleTalkDb( 0.0 ), handOvers( 0 ), handedOver( false ), erleDb( 0.0 ) {}
};

/** Gain of 'signal' delayed by 'lag' in 'output' over [from, to) of the signal (projection) */
Float64
Along( const TSamples& output, const TSamples& signal, size_t lag, size_t from, size_t to )
{
    Float64 xy = 0.0, xx = 0.0;
    for ( size_t n=from; n<to && n<signal.size() && n + lag<output.size(); ++n )
    {
        xy += (Float64) output[ n + lag ] * (Float64) signal[ n ];
        xx += (Float64) signal[ n ] * (Float64) signal[ n ];
    }
    return xx > 0.0 ? xy / xx : 0.0;
}

/** A talker of speech-like bursts in the given stretches, the active RMS -20 dBFS, exact in 16 bit PCM */
TSamples
MakeCallTalker( Float64 seconds, const Float64 spans[][ 2 ], UInt32 count, UInt64 seed )
{
    TSamples x( (size_t) ( seconds * Rate ), 0.0f );
    TVS::TMask active( x.size(), 0 );
    for ( UInt32 s=0; s<count; ++s )
    {
        const size_t start = (size_t) ( spans[ s ][ 0 ] * Rate );
        const size_t length = (size_t) ( ( spans[ s ][ 1 ] - spans[ s ][ 0 ] ) * Rate );
        TVS::AddVoicedBurst( x, &active, Rate, start, length, 110.0 + 30.0 * s, 160.0 + 20.0 * s, 1.0, 4.0, (size_t) ( 0.008 * Rate ), seed + s );
    }
    // the unvoiced part, as for the far talker
    const TSamples noise = TVS::MakeWhiteNoise( x.size(), 1.0, seed + 99 );
    const Float64 voiced = TVS::MaskedRms( x, active );
    for ( size_t i=0; i<x.size(); ++i )
        x[ i ] += active[ i ] ? (Float32) ( noise[ i ] * voiced * 0.18 ) : 0.0f;
    TVS::Scale( x, 0.1 / TVS::MaskedRms( x, active ) );
    for ( size_t i=0; i<x.size(); ++i )
        x[ i ] = (Float32) floor( x[ i ] * 32768.0f + 0.5f ) / 32768.0f;
    return x;
}

/**
 *  The call of section 6.2 at 16 kHz: units A and B (5 ms periods), PCM over a wired link at a constant delay. A's speaker
 *  reaches A's microphone at -15 dB after 20 ms; A's receiver plays on the bus "call" with a ducker. B talks most of the
 *  time, A talks alone once and over B three times. A has an echo canceller and/or a half-duplex controller (hand-over
 *  on) or neither
 */
bool
RunCall( bool withCanceller, bool withHalfDuplex, SCallResult& result )
{
    using namespace VoiceLinkTest;
    const Float64 seconds = 24.0;
    const Float64 bSpans[][ 2 ] = { { 1.0, 5.0 }, { 6.5, 10.0 }, { 11.5, 15.5 }, { 17.0, 22.0 } };
    const Float64 aSpans[][ 2 ] = { { 5.2, 6.3 }, { 8.0, 9.2 }, { 13.0, 14.2 }, { 19.5, 20.7 } };
    const TSamples bVoice = MakeCallTalker( seconds, bSpans, 4, 181 );
    const TSamples aVoice = MakeCallTalker( seconds, aSpans, 4, 191 );
    TSamples aRoom = TVS::MakeWhiteNoise( aVoice.size(), DbToGain( -80.0 ), 81 );
    for ( size_t n=0; n<aRoom.size(); ++n )
        aRoom[ n ] += aVoice[ n ];
    CBufferSignal aSignal( aRoom );
    CBufferSignal bSignal( bVoice );

    CTestIntercomScenario scenario( 80, Rate );
    SUnitConfig aConfig = UnitConfig16();
    aConfig.echoGain = DbToGain( -15.0 );
    aConfig.echoDelayFrames = 320;
    SUnitConfig bConfig = UnitConfig16();
    bConfig.keepOutput = true;
    CTestIntercomUnit* a = scenario.AddUnit( aConfig );
    CTestIntercomUnit* b = scenario.AddUnit( bConfig );
    if ( GUCEF_NULL == a || GUCEF_NULL == b )
        return false;
    a->SetSignal( &aSignal );
    b->SetSignal( &bSignal );

    const AUDIO::CVoiceCodecConfig codec = VoiceStreamTest::PcmConfig( Rate, 10000 );
    AUDIO::CVoiceSenderConfig senderConfig;
    senderConfig.SetCodecConfig( codec );
    senderConfig.SetFirstSequenceNumber( 1 );
    senderConfig.SetFirstTimestamp( 0 );
    senderConfig.SetStreamId( 0xA );
    if ( !a->AttachSender( senderConfig ) )
        return false;
    senderConfig.SetStreamId( 0xB );
    if ( !b->AttachSender( senderConfig ) )
        return false;

    AUDIO::CVoiceReceiverConfig receiverConfig;
    receiverConfig.SetCodecConfig( codec );
    receiverConfig.SetPlayoutProfile( AUDIO::VOICEPLAYOUTPROFILE_WIRED );
    receiverConfig.SetMinMarginInMs( receiverConfig.GetInitialMarginInMs() );
    AUDIO::CAudioBus* aCall = a->engine->CreateBus( "call" );
    AUDIO::CAudioBus* bCall = b->engine->CreateBus( "call" );
    AUDIO::TAudioEffectPtr ducker = a->engine->CreateEffect( "ducker" );
    if ( GUCEF_NULL == aCall || GUCEF_NULL == bCall || ducker.IsNULL() )
        return false;
    static_cast< AUDIO::CDuckerEffect* >( ducker.GetPointerAlways() )->SetDepthInDecibels( -20.0f );
    CORE::CFutureResult inserted = a->engine->InsertEffect( aCall, 0, ducker );
    AUDIO::CVoiceReceiver* atA = a->AddReceiver( receiverConfig, aCall );
    AUDIO::CVoiceReceiver* atB = b->AddReceiver( receiverConfig, bCall );
    if ( inserted.HasNoFuture() || GUCEF_NULL == atA || GUCEF_NULL == atB )
        return false;
    SLinkModel model = SLinkModel::Wired();
    model.jitter = SLinkModel::JITTER_NONE;
    scenario.Connect( a, b, atB, model );
    scenario.Connect( b, a, atA, model );

    AUDIO::CHalfDuplexSettings hdSettings;
    hdSettings.SetFarEndBusName( "call" );
    AUDIO::CHalfDuplexController halfDuplex( hdSettings );
    TCanceller canceller;
    if ( withHalfDuplex )
    {
        if ( !halfDuplex.SetDucker( ducker ) || a->engine->SetCaptureHalfDuplexController( &halfDuplex ).HasNoFuture() )
            return false;
    }
    if ( withCanceller && a->engine->SetEchoCanceller( &canceller ).HasNoFuture() )
        return false;

    scenario.Run( seconds );
    result.handOvers = halfDuplex.GetHandOverCount();
    result.handedOver = halfDuplex.IsHandedOver();
    result.erleDb = canceller.GetErleInDecibels();

    // the lags: one way A to B (A alone), and B's voice round trip (B to A, the acoustic path, A to B) plus the canceller's latency
    const TSamples& out = b->output;
    size_t oneWay = 0;
    Float64 best = -1.0;
    for ( size_t lag=150; lag<500; ++lag )
    {
        const Float64 c = Along( out, aVoice, lag, (size_t) ( 5.3 * Rate ), (size_t) ( 6.2 * Rate ) );
        if ( c > best )
        {
            best = c;
            oneWay = lag;
        }
    }
    const size_t roundTrip = oneWay + ( oneWay - ( withCanceller ? canceller.GetLatencyInFrames() : 0 ) ) + 80 + 320;

    // late in the call, where only B talks (17 to 19.5 s) and where A talks over B (19.5 to 20.7 s)
    const size_t from = (size_t) ( 17.3 * Rate ), to = (size_t) ( 19.4 * Rate );
    Float64 outPower = 0.0, voicePower = 0.0;
    for ( size_t n=from; n<to; ++n )
    {
        outPower += (Float64) out[ n + roundTrip ] * (Float64) out[ n + roundTrip ];
        voicePower += (Float64) bVoice[ n ] * (Float64) bVoice[ n ];
    }
    result.echoDb = 10.0 * log10( ( outPower + 1.0e-30 ) / voicePower );
    result.echoAlongDb = TVS::ToDecibels( fabs( Along( out, bVoice, roundTrip, from, to ) ) + 1.0e-15 );
    result.doubleTalkEchoDb = TVS::ToDecibels( fabs( Along( out, bVoice, roundTrip, (size_t) ( 19.6 * Rate ), (size_t) ( 20.6 * Rate ) ) ) + 1.0e-15 );
    result.aloneDb = TVS::ToDecibels( fabs( Along( out, aVoice, oneWay, (size_t) ( 5.3 * Rate ), (size_t) ( 6.2 * Rate ) ) ) + 1.0e-15 );
    result.doubleTalkDb = TVS::ToDecibels( fabs( Along( out, aVoice, oneWay, (size_t) ( 19.6 * Rate ), (size_t) ( 20.6 * Rate ) ) ) + 1.0e-15 );

    if ( withCanceller )
        a->engine->SetEchoCanceller( GUCEF_NULL );
    if ( withHalfDuplex )
        a->engine->SetCaptureHalfDuplexController( GUCEF_NULL );
    scenario.Step();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: call " << ( withCanceller ? "with" : "without" ) << " the canceller, " << ( withHalfDuplex ? "with" : "without" ) << " half duplex: lags " << oneWay << " and " << roundTrip <<
               "; canceller: blocks " << canceller.GetBlockCount() << ", adapted " << canceller.GetAdaptedBlockCount() << ", idle " << canceller.GetIdleBlockCount() << ", alignments " << canceller.GetResyncCount() <<
               ", underruns " << canceller.GetReferenceUnderrunCount() << ", divergences " << canceller.GetDivergenceCount() << ", bulk delay " << canceller.GetBulkDelayInFrames() << " (estimate " <<
               canceller.GetEstimatedDelayInFrames() << "), level " << canceller.GetReferenceLevelInFrames() << ", drift " << canceller.GetDriftInPpm() << " ppm" );
    return true;
}

void
PerformIntercomTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: Intercom call (section 6.2, 16 kHz): with the canceller instead of half duplex B hears its own voice 40 dB down while A talks over B at full level; with both, half duplex hands over once the canceller converged" )
    try
    {
        SCallResult none, halfDuplex, canceller, both;
        ASSERT_TRUE( RunCall( false, false, none ) );
        ASSERT_TRUE( RunCall( false, true, halfDuplex ) );
        ASSERT_TRUE( RunCall( true, false, canceller ) );
        ASSERT_TRUE( RunCall( true, true, both ) );
        const SCallResult* results[] = { &none, &halfDuplex, &canceller, &both };
        const char* names[] = { "nothing", "half duplex", "the canceller", "the canceller and half duplex (hand-over)" };
        for ( UInt32 r=0; r<4; ++r )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEchoCanceller: call with " << names[ r ] << ": B's own voice back at B " << results[ r ]->echoDb << " dB (level; " << results[ r ]->echoAlongDb <<
                       " dB along it), during double talk " << results[ r ]->doubleTalkEchoDb << " dB; A at B alone " << results[ r ]->aloneDb << " dB, over B " << results[ r ]->doubleTalkDb <<
                       " dB; hand-overs " << results[ r ]->handOvers << ( results[ r ]->handedOver ? " (handed over at the end)" : "" ) << ", ERLE estimate " << results[ r ]->erleDb << " dB" );
        }
        ASSERT_TRUE( fabs( none.echoAlongDb + 15.0 ) <= 1.0 );
        ASSERT_TRUE( canceller.echoDb <= -40.0 );
        ASSERT_TRUE( canceller.doubleTalkDb >= -3.0 && fabs( canceller.aloneDb ) <= 1.0 );
        ASSERT_TRUE( canceller.doubleTalkEchoDb <= -25.0 );
        ASSERT_TRUE( halfDuplex.doubleTalkDb < canceller.doubleTalkDb || halfDuplex.echoDb <= -40.0 );
        ASSERT_TRUE( both.handOvers >= 1 );
        ASSERT_TRUE( both.echoDb <= -40.0 && both.doubleTalkDb >= -3.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEchoCanceller: The call with the canceller and half duplex allocates nothing and takes no lock on the render and capture threads (RealtimeSanitizer build)" )
    if ( !IsRtsanBuild )
    {
        GUCEF_TESTFW_SKIP( "not a RealtimeSanitizer build (-fsanitize=realtime)" );
    }
    GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_RTSAN_CLEAN" ) == "0", "switched off with GUCEF_TEST_AUDIO_RTSAN_CLEAN=0" );
    try
    {
        SCallResult result;
        ASSERT_TRUE( RunCall( true, true, result ) );
        ASSERT_TRUE( result.erleDb > 0.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------*/

void
PerformAudioEchoCancellerTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioEchoCanceller TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.EchoCanceller" );

    PerformComponentTests();
    PerformConvergenceTests();
    PerformDoubleTalkAndChangeTests();
    PerformDriftAndLongRunTests();
    PerformCostAndBypassTests();
    PerformEngineTests();
    PerformIntercomTests();
}

/*-------------------------------------------------------------------------*/
