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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioVoiceSignals.h"
#include "TestAudioVoiceActivity.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

#if defined( __has_feature )
  #if __has_feature( realtime_sanitizer )
    #define GUCEF_AUDIO_TEST_VOICEACTIVITY_RTSAN_BUILD
  #endif
#endif

using namespace GUCEF;

namespace Sig = TestVoiceSignals;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef Sig::TSamples TSamples;

/** What a detector said after each completed analysis frame */
struct SFrameLog
{
    GUCEF::vector< UInt8 > active;
    GUCEF::vector< Float32 > energy;
    GUCEF::vector< Float32 > floor;
    void Note( const AUDIO::CVoiceActivityDetector& detector )
    {
        active.push_back( detector.IsActive() ? 1 : 0 );
        energy.push_back( detector.GetEnergyInDecibels() );
        floor.push_back( detector.GetNoiseFloorInDecibels() );
    }
};

/** The first sample after analysis frame 'k' */
inline UInt64
FrameEnd( UInt32 rate, UInt64 k )
{
    return ( ( k + 1 ) * (UInt64) rate ) / 100;
}

inline UInt64
FrameStart( UInt32 rate, UInt64 k )
{
    return ( k * (UInt64) rate ) / 100;
}

/** Feeds x[ from .. from+count ) as the stream positions from 'absoluteStart' on, in pieces that end at analysis frame boundaries, noting the state after each frame */
void
FeedAligned( AUDIO::CVoiceActivityDetector& detector, const TSamples& x, UInt32 rate, size_t from, size_t count, UInt64 absoluteStart, SFrameLog& log )
{
    UInt64 position = absoluteStart;
    size_t done = 0;
    while ( done < count )
    {
        UInt64 k = ( position * 100 ) / rate;
        UInt64 end = FrameEnd( rate, k );
        while ( end <= position )
            end = FrameEnd( rate, ++k );
        size_t piece = (size_t) ( end - position );
        if ( piece > count - done )
            piece = count - done;
        const UInt64 before = detector.GetAnalyzedFrameCount();
        detector.Process( &x[ from + done ], (UInt32) piece, position );
        if ( detector.GetAnalyzedFrameCount() > before )
            log.Note( detector );
        position += piece;
        done += piece;
    }
}

/** A detector for 'rate' with the settings, run over x by frames */
void
RunByFrames( const TSamples& x, UInt32 rate, const AUDIO::CVoiceActivitySettings& settings, SFrameLog& log, AUDIO::CVoiceActivityDetector& detector )
{
    detector.SetSettings( settings );
    detector.Prepare( rate );
    FeedAligned( detector, x, rate, 0, x.size(), 0, log );
}

void
RunByFrames( const TSamples& x, UInt32 rate, SFrameLog& log )
{
    AUDIO::CVoiceActivityDetector detector;
    RunByFrames( x, rate, AUDIO::CVoiceActivitySettings(), log, detector );
}

/** How a log fits the truth of a speech signal */
struct SScore
{
    UInt32 truthFrames;         /**< frames that are mostly active in the truth */
    UInt32 hits;                /**< of those, frames the detector called active */
    UInt32 gapFrames;           /**< frames with no active sample in them or in the 300 ms before */
    UInt32 falseAlarms;         /**< of those, frames the detector called active */
    UInt32 activeRuns;          /**< stretches of frames the detector called active */
    UInt32 shortestRunFrames;   /**< the shortest of them */
    UInt32 talkspursDetected;   /**< talkspurts with at least one active frame while they are on */
    Float64 meanLatencyInMs;    /**< from the first active sample of a talkspurt to the end of the first frame called active (negative: before) */
    Float64 maxLatencyInMs;
    Float64 HitRate( void ) const { return truthFrames > 0 ? (Float64) hits / (Float64) truthFrames : 0.0; }
    Float64 FalseAlarmRate( void ) const { return gapFrames > 0 ? (Float64) falseAlarms / (Float64) gapFrames : 0.0; }
    SScore( void ) : truthFrames( 0 ), hits( 0 ), gapFrames( 0 ), falseAlarms( 0 ), activeRuns( 0 ), shortestRunFrames( 0xFFFFFFFF ), talkspursDetected( 0 ), meanLatencyInMs( 0.0 ), maxLatencyInMs( -1.0e9 ) {}
};

SScore
ScoreAgainstTruth( const Sig::SSpeech& speech, const SFrameLog& log, UInt32 rate )
{
    SScore score;
    GUCEF::vector< UInt32 > prefix( speech.active.size() + 1, 0 );
    for ( size_t i=0; i<speech.active.size(); ++i )
        prefix[ i + 1 ] = prefix[ i ] + ( speech.active[ i ] ? 1 : 0 );
    const UInt64 lookBack = (UInt64) ( 0.3 * (Float64) rate );

    UInt32 run = 0;
    for ( size_t k=0; k<log.active.size(); ++k )
    {
        const UInt64 start = FrameStart( rate, k );
        const UInt64 end = FrameEnd( rate, k );
        if ( end > speech.active.size() )
            break;
        const UInt32 activeSamples = prefix[ end ] - prefix[ start ];
        const UInt64 from = ( start > lookBack ) ? start - lookBack : 0;
        const UInt32 nearActive = prefix[ end ] - prefix[ from ];
        const bool called = 0 != log.active[ k ];
        if ( activeSamples * 2 >= end - start )
        {
            ++score.truthFrames;
            if ( called )
                ++score.hits;
        }
        if ( 0 == nearActive )
        {
            ++score.gapFrames;
            if ( called )
                ++score.falseAlarms;
        }
        if ( called )
            ++run;
        if ( ( !called || k + 1 == log.active.size() ) && run > 0 )
        {
            ++score.activeRuns;
            if ( run < score.shortestRunFrames )
                score.shortestRunFrames = run;
            run = 0;
        }
    }

    // the onset of each talkspurt
    UInt32 measured = 0;
    for ( size_t t=0; t<speech.talkspurts.size(); ++t )
    {
        const Sig::SSpan& spurt = speech.talkspurts[ t ];
        UInt64 firstActive = spurt.start;
        while ( firstActive < spurt.End() && 0 == speech.active[ firstActive ] )
            ++firstActive;
        for ( size_t k=(size_t) ( ( spurt.start * 100 ) / rate ); k<log.active.size() && FrameStart( rate, k ) < spurt.End(); ++k )
        {
            if ( 0 != log.active[ k ] && FrameEnd( rate, k ) > spurt.start )
            {
                const Float64 latency = 1000.0 * ( (Float64) FrameEnd( rate, k ) - (Float64) firstActive ) / (Float64) rate;
                score.meanLatencyInMs += latency;
                if ( latency > score.maxLatencyInMs )
                    score.maxLatencyInMs = latency;
                ++measured;
                ++score.talkspursDetected;
                break;
            }
        }
    }
    if ( measured > 0 )
        score.meanLatencyInMs /= (Float64) measured;
    return score;
}

/** The speech-like signal of the tests: 30 s at 'rate', active RMS 0.1 (-20 dBFS) */
const Sig::SSpeech&
TestSpeech( UInt32 rate )
{
    static Sig::SSpeech speeches[ 4 ];
    static UInt32 rates[ 4 ] = { 0, 0, 0, 0 };
    for ( int i=0; i<4; ++i )
    {
        if ( rates[ i ] == rate )
            return speeches[ i ];
        if ( 0 == rates[ i ] )
        {
            Sig::SSpeechSpec spec;
            spec.sampleRate = rate;
            spec.seconds = 30.0;
            spec.seed = 11;
            Sig::MakeSpeech( spec, speeches[ i ] );
            rates[ i ] = rate;
            return speeches[ i ];
        }
    }
    return speeches[ 3 ];
}

TSamples
MakeNoise( bool pink, size_t count, Float64 rms, UInt64 seed )
{
    return pink ? Sig::MakePinkNoise( count, rms, seed ) : Sig::MakeWhiteNoise( count, rms, seed );
}

/** Fraction of the frames from 'first' that are active */
Float64
ActiveFraction( const SFrameLog& log, size_t first )
{
    size_t active = 0;
    for ( size_t k=first; k<log.active.size(); ++k )
        active += log.active[ k ] ? 1 : 0;
    return log.active.size() > first ? (Float64) active / (Float64) ( log.active.size() - first ) : 0.0;
}

/**
 *  A steady tone of 1500 Hz with the RMS 'bedRms' (the noise floor: constant in every analysis frame) plus a tone of 1000 Hz
 *  that is 'levelsInDecibels[ s ]' above the bed in the energy for 'seconds[ s ]' (a level of 0 or less leaves the bed alone).
 *  The two tones are orthogonal over a frame at 16 kHz, so the energies add exactly
 */
TSamples
MakeToneSteps( UInt32 rate, Float64 bedRms, const Float64* levelsInDecibels, const Float64* seconds, size_t stepCount )
{
    size_t total = 0;
    for ( size_t s=0; s<stepCount; ++s )
        total += (size_t) ( seconds[ s ] * (Float64) rate + 0.5 );
    TSamples x = Sig::MakeTone( total, rate, 1500.0, bedRms );
    size_t at = 0;
    for ( size_t s=0; s<stepCount; ++s )
    {
        const size_t length = (size_t) ( seconds[ s ] * (Float64) rate + 0.5 );
        if ( levelsInDecibels[ s ] > 0.0 )
        {
            const Float64 rms = bedRms * sqrt( pow( 10.0, levelsInDecibels[ s ] / 10.0 ) - 1.0 );
            const Float64 amplitude = rms * sqrt( 2.0 );
            for ( size_t i=0; i<length; ++i )
                x[ at + i ] = (Float32) ( (Float64) x[ at + i ] + amplitude * sin( 2.0 * Sig::Pi * 1000.0 * (Float64) ( at + i ) / (Float64) rate ) );
        }
        at += length;
    }
    return x;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TEST SIGNALS                                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformVoiceSignalTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: The test signals are deterministic and have the levels, ratios and truth they are asked for" )
    try
    {
        Sig::SSpeechSpec spec;
        spec.sampleRate = 16000;
        spec.seconds = 12.0;
        spec.seed = 5;
        spec.activeRms = 0.05;
        Sig::SSpeech a, b;
        Sig::MakeSpeech( spec, a );
        Sig::MakeSpeech( spec, b );
        ASSERT_TRUE( a.samples.size() == 12 * 16000 && a.active.size() == a.samples.size() );
        ASSERT_TRUE( 0 == memcmp( &a.samples[ 0 ], &b.samples[ 0 ], a.samples.size() * sizeof( Float32 ) ) );
        ASSERT_TRUE( a.active == b.active );
        spec.seed = 6;
        Sig::SSpeech c;
        Sig::MakeSpeech( spec, c );
        ASSERT_FALSE( 0 == memcmp( &a.samples[ 0 ], &c.samples[ 0 ], a.samples.size() * sizeof( Float32 ) ) );

        // the level over the active samples is as asked, the pauses are silent, the talkspurts keep their distance
        ASSERT_TRUE( fabs( Sig::MaskedRms( a.samples, a.active ) - 0.05 ) < 1.0e-6 );
        ASSERT_TRUE( a.talkspurts.size() >= 3 );
        size_t activeCount = 0;
        for ( size_t i=0; i<a.active.size(); ++i )
            activeCount += a.active[ i ];
        const Float64 activeFraction = (Float64) activeCount / (Float64) a.active.size();
        ASSERT_TRUE( activeFraction > 0.15 && activeFraction < 0.8 );
        Float32 peak = 0.0f;
        for ( size_t i=0; i<a.samples.size(); ++i )
            peak = ( fabsf( a.samples[ i ] ) > peak ) ? fabsf( a.samples[ i ] ) : peak;
        ASSERT_TRUE( peak < 0.95f );
        for ( size_t t=0; t<a.talkspurts.size(); ++t )
        {
            const Sig::SSpan& spurt = a.talkspurts[ t ];
            ASSERT_TRUE( spurt.length >= (UInt64) ( 0.8 * 16000.0 ) - 1 && spurt.length <= (UInt64) ( 2.5 * 16000.0 ) + 1 );
            ASSERT_TRUE( spurt.start >= 16000 );
            if ( t > 0 )
                ASSERT_TRUE( spurt.start >= a.talkspurts[ t - 1 ].End() + (UInt64) ( 0.5 * 16000.0 ) - 1 );
            ASSERT_TRUE( 0.0 == Sig::Rms( a.samples, (size_t) ( spurt.End() + 4 ), 160 ) || t + 1 == a.talkspurts.size() || spurt.End() + 164 >= a.talkspurts[ t + 1 ].start );
        }
        ASSERT_TRUE( 0.0 == Sig::Rms( a.samples, 0, 16000 ) );                  // the leading silence

        // syllables: the envelope falls to (almost) nothing about four times per second inside a talkspurt
        const Sig::SSpan& longest = a.talkspurts[ 0 ];
        UInt32 dips = 0;
        bool inDip = false;
        for ( UInt64 i=longest.start+400; i+400<longest.End(); i+=80 )
        {
            const Float64 level = Sig::Rms( a.samples, (size_t) i, 80 );
            if ( level < 0.1 * 0.05 && !inDip ) { ++dips; inDip = true; }
            if ( level > 0.5 * 0.05 ) inDip = false;
        }
        const Float64 spurtSeconds = (Float64) ( longest.length - 800 ) / 16000.0;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceActivity: the first talkspurt of " << spurtSeconds << " s has " << dips << " syllable dips" );
        ASSERT_TRUE( (Float64) dips >= 3.0 * spurtSeconds - 2.0 && (Float64) dips <= 5.0 * spurtSeconds + 2.0 );

        // noise at a ratio: the ratio of the active speech to the noise over the whole signal is what was asked
        const TSamples white = Sig::MakeWhiteNoise( a.samples.size(), 0.3, 9 );
        ASSERT_TRUE( fabs( Sig::Rms( white, 0, white.size() ) - 0.3 ) < 1.0e-6 );
        const TSamples pink = Sig::MakePinkNoise( a.samples.size(), 0.3, 9 );
        ASSERT_TRUE( fabs( Sig::Rms( pink, 0, pink.size() ) - 0.3 ) < 1.0e-6 );
        const Float64 whiteSlope = Sig::Rms( white, 0, white.size() );
        TSamples whiteDiff( white.size() - 1 ), pinkDiff( pink.size() - 1 );
        for ( size_t i=1; i<white.size(); ++i )
        {
            whiteDiff[ i - 1 ] = white[ i ] - white[ i - 1 ];
            pinkDiff[ i - 1 ] = pink[ i ] - pink[ i - 1 ];
        }
        ASSERT_TRUE( fabs( Sig::Rms( whiteDiff, 0, whiteDiff.size() ) / whiteSlope - sqrt( 2.0 ) ) < 0.03 );   // white: the difference has twice the power
        ASSERT_TRUE( Sig::Rms( pinkDiff, 0, pinkDiff.size() ) / 0.3 < 0.7 );                                    // pink: much of the power is at low frequencies
        for ( int snr=-5; snr<=25; snr+=10 )
        {
            const TSamples mixed = Sig::MixAtSnr( a, white, (Float64) snr );
            TSamples noiseOnly = mixed;
            for ( size_t i=0; i<noiseOnly.size(); ++i )
                noiseOnly[ i ] -= a.samples[ i ];
            const Float64 measured = Sig::ToDecibels( Sig::MaskedRms( a.samples, a.active ) ) - Sig::ToDecibels( Sig::Rms( noiseOnly, 0, noiseOnly.size() ) );
            ASSERT_TRUE( fabs( measured - (Float64) snr ) < 0.01 );
        }

        // hum: periodic with the mains period, exact level; tone; clicks at their positions
        const TSamples hum = Sig::MakeHum( 32000, 16000, 50.0, 6, 0.02 );
        ASSERT_TRUE( fabs( Sig::Rms( hum, 0, hum.size() ) - 0.02 ) < 1.0e-6 );
        for ( size_t i=0; i+320<hum.size(); i+=97 )
            ASSERT_TRUE( fabs( hum[ i ] - hum[ i + 320 ] ) < 1.0e-5f );
        const TSamples tone = Sig::MakeTone( 16000, 16000, 1000.0, 0.1 );
        ASSERT_TRUE( fabs( Sig::Rms( tone, 0, tone.size() ) - 0.1 ) < 1.0e-4 );
        Sig::TSpans positions;
        const TSamples clicks = Sig::MakeClicks( 16000 * 10, 16000, 0.5, 0.4, 1, 3, &positions );
        ASSERT_TRUE( positions.size() >= 14 && positions.size() <= 26 );
        for ( size_t p=0; p<positions.size(); ++p )
            ASSERT_TRUE( fabsf( clicks[ (size_t) positions[ p ].start ] ) == 0.4f );
        size_t nonZero = 0;
        for ( size_t i=0; i<clicks.size(); ++i )
            nonZero += ( 0.0f != clicks[ i ] ) ? 1 : 0;
        ASSERT_TRUE( nonZero == positions.size() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      SETTINGS                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformVoiceActivitySettingsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: The settings have the defaults of the design, limit what is set and keep the release margin below the onset margin" )
    try
    {
        AUDIO::CVoiceActivitySettings settings;
        ASSERT_TRUE( 9.0f == settings.GetOnsetInDecibels() && 6.0f == settings.GetReleaseInDecibels() );
        ASSERT_TRUE( 200.0f == settings.GetHangoverInMs() && -65.0f == settings.GetMinimumLevelInDecibels() && 3.0f == settings.GetFloorRiseInDecibelsPerSecond() );

        settings.SetOnsetInDecibels( 100.0f );
        ASSERT_TRUE( 60.0f == settings.GetOnsetInDecibels() );
        settings.SetOnsetInDecibels( -3.0f );
        ASSERT_TRUE( 0.0f == settings.GetOnsetInDecibels() && 0.0f == settings.GetReleaseInDecibels() );       // the release follows the onset down
        settings.SetOnsetInDecibels( 12.0f );
        settings.SetReleaseInDecibels( 20.0f );
        ASSERT_TRUE( 12.0f == settings.GetReleaseInDecibels() );
        settings.SetReleaseInDecibels( 4.0f );
        ASSERT_TRUE( 4.0f == settings.GetReleaseInDecibels() );
        settings.SetHangoverInMs( 99999.0f );
        ASSERT_TRUE( 5000.0f == settings.GetHangoverInMs() );
        settings.SetHangoverInMs( -1.0f );
        ASSERT_TRUE( 0.0f == settings.GetHangoverInMs() );
        settings.SetMinimumLevelInDecibels( -500.0f );
        ASSERT_TRUE( -120.0f == settings.GetMinimumLevelInDecibels() );
        settings.SetMinimumLevelInDecibels( 5.0f );
        ASSERT_TRUE( 0.0f == settings.GetMinimumLevelInDecibels() );
        settings.SetFloorRiseInDecibelsPerSecond( 0.0f );
        ASSERT_TRUE( 0.1f == settings.GetFloorRiseInDecibelsPerSecond() );
        settings.SetFloorRiseInDecibelsPerSecond( 1000.0f );
        ASSERT_TRUE( 60.0f == settings.GetFloorRiseInDecibelsPerSecond() );

        // not a number: ignored
        volatile Float32 zero = 0.0f;
        const Float32 notANumber = zero / zero;
        settings.SetOnsetInDecibels( notANumber );
        ASSERT_TRUE( 12.0f == settings.GetOnsetInDecibels() );

        AUDIO::CVoiceActivitySettings other( settings );
        ASSERT_TRUE( other == settings );
        other.SetHangoverInMs( 321.0f );
        ASSERT_TRUE( other != settings );
        other = settings;
        ASSERT_TRUE( other == settings );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: The settings save and load as the section VoiceActivity, also as a child of the engine parameters" )
    try
    {
        AUDIO::CVoiceActivitySettings settings;
        settings.SetOnsetInDecibels( 14.5f );
        settings.SetReleaseInDecibels( 7.25f );
        settings.SetHangoverInMs( 350.0f );
        settings.SetMinimumLevelInDecibels( -58.0f );
        settings.SetFloorRiseInDecibelsPerSecond( 5.5f );

        CORE::CDataNode tree;
        ASSERT_TRUE( settings.SaveConfig( tree ) );
        ASSERT_TRUE( GUCEF_NULL != tree.FindChild( "VoiceActivity" ) );
        AUDIO::CVoiceActivitySettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( tree ) );
        ASSERT_TRUE( loaded == settings );
        ASSERT_TRUE( loaded.LoadConfig( *tree.FindChild( "VoiceActivity" ) ) );                 // the section itself will do

        // a section with part of the values keeps the rest, values out of range are limited, no section is no success
        CORE::CDataNode partial;
        CORE::CDataNode* section = partial.AddChild( "VoiceActivity" );
        section->SetAttribute( "hangoverMs", 80 );
        section->SetAttribute( "onsetDb", 500 );
        AUDIO::CVoiceActivitySettings defaults;
        ASSERT_TRUE( defaults.LoadConfig( partial ) );
        ASSERT_TRUE( 80.0f == defaults.GetHangoverInMs() && 60.0f == defaults.GetOnsetInDecibels() && 6.0f == defaults.GetReleaseInDecibels() && -65.0f == defaults.GetMinimumLevelInDecibels() );
        CORE::CDataNode empty;
        ASSERT_FALSE( AUDIO::CVoiceActivitySettings().LoadConfig( empty ) );

        // part of the parameters of the engine
        AUDIO::CAudioEngineParams params;
        ASSERT_TRUE( params.GetVoiceActivitySettings() == AUDIO::CVoiceActivitySettings() );
        CORE::CDataNode defaultTree;
        ASSERT_TRUE( params.SaveConfig( defaultTree ) );
        ASSERT_TRUE( GUCEF_NULL == defaultTree.FindChild( "AudioEngine" )->FindChild( "VoiceActivity" ) );       // the defaults are not written
        params.GetVoiceActivitySettings() = settings;
        CORE::CDataNode engineTree;
        ASSERT_TRUE( params.SaveConfig( engineTree ) );
        const CORE::CDataNode* engineSection = engineTree.FindChild( "AudioEngine" );
        ASSERT_TRUE( GUCEF_NULL != engineSection && GUCEF_NULL != engineSection->FindChild( "VoiceActivity" ) );
        AUDIO::CAudioEngineParams loadedParams;
        ASSERT_TRUE( loadedParams.LoadConfig( engineTree ) );
        ASSERT_TRUE( loadedParams.GetVoiceActivitySettings() == settings );
        params.GetVoiceActivitySettings() = AUDIO::CVoiceActivitySettings();                                    // saving defaults over a tree that has the child removes it
        ASSERT_TRUE( params.SaveConfig( engineTree ) );
        ASSERT_TRUE( GUCEF_NULL == engineTree.FindChild( "AudioEngine" )->FindChild( "VoiceActivity" ) );
        params.GetVoiceActivitySettings() = settings;
        AUDIO::CAudioEngineParams copied( params );
        ASSERT_TRUE( copied.GetVoiceActivitySettings() == settings );
        AUDIO::CAudioEngineParams assigned;
        assigned = params;
        ASSERT_TRUE( assigned.GetVoiceActivitySettings() == settings );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      DETECTOR                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformVoiceDetectorDetectionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: The detector takes the rates it supports, does nothing before it is prepared and keeps its settings through Prepare() and Release()" )
    try
    {
        AUDIO::CVoiceActivityDetector detector;
        const TSamples noise = Sig::MakeWhiteNoise( 1600, 0.1, 1 );
        ASSERT_FALSE( detector.IsPrepared() );
        ASSERT_FALSE( detector.Process( &noise[ 0 ], 1600 ) );
        ASSERT_FALSE( detector.IsActive() );
        ASSERT_TRUE( -120.0f == detector.GetEnergyInDecibels() && -120.0f == detector.GetNoiseFloorInDecibels() && 0 == detector.GetAnalyzedFrameCount() );
        ASSERT_FALSE( detector.Prepare( 7999 ) );
        ASSERT_FALSE( detector.Prepare( 192001 ) );
        ASSERT_FALSE( detector.IsPrepared() );
        const UInt32 rates[ 8 ] = { 8000, 11025, 16000, 22050, 32000, 44100, 48000, 192000 };
        for ( int r=0; r<8; ++r )
        {
            ASSERT_TRUE( detector.Prepare( rates[ r ] ) );
            ASSERT_TRUE( detector.IsPrepared() && rates[ r ] == detector.GetSampleRate() );
            detector.Process( &noise[ 0 ], 1600 );
            ASSERT_TRUE( detector.GetAnalyzedFrameCount() == (UInt64) ( 1600 * 100 / rates[ r ] ) );
        }

        AUDIO::CVoiceActivitySettings settings;
        settings.SetHangoverInMs( 400.0f );
        detector.SetSettings( settings );
        ASSERT_TRUE( detector.Prepare( 16000 ) );
        ASSERT_TRUE( detector.GetSettings() == settings );
        detector.Release();
        ASSERT_FALSE( detector.IsPrepared() );
        ASSERT_TRUE( detector.GetSettings() == settings );
        ASSERT_TRUE( 0 == detector.GetAnalyzedFrameCount() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // Speech-like bursts in white and pink noise. The signal to noise ratio is that of the active speech (-20 dBFS) against the
    // noise over the whole band; the detector only looks at 200 to 4000 Hz, where white noise has 47 % of its power at 16 kHz
    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: Speech-like bursts are detected in white and pink noise at signal to noise ratios of 20, 10 and 5 dB: hit rate and false alarm rate measured" )
    try
    {
        const Sig::SSpeech& speech = TestSpeech( 16000 );
        const int snrs[ 3 ] = { 20, 10, 5 };
        const Float64 requiredHitRate[ 3 ] = { 0.95, 0.95, 0.90 };
        for ( int pink=0; pink<2; ++pink )
        {
            for ( int s=0; s<3; ++s )
            {
                const TSamples noise = MakeNoise( 0 != pink, speech.samples.size(), 1.0, 100 + (UInt64) s );
                const TSamples mixed = Sig::MixAtSnr( speech, noise, (Float64) snrs[ s ] );
                SFrameLog log;
                RunByFrames( mixed, 16000, log );
                const SScore score = ScoreAgainstTruth( speech, log, 16000 );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceActivity: " << ( pink ? "pink" : "white" ) << " noise, SNR " << snrs[ s ] << " dB: hit rate " << 100.0 * score.HitRate() << " % of " << score.truthFrames <<
                           " active frames, false alarm rate " << 100.0 * score.FalseAlarmRate() << " % of " << score.gapFrames << " pause frames, talkspurts detected " << score.talkspursDetected << " of " << speech.talkspurts.size() <<
                           ", onset latency (to the end of the first active frame, from the first sample with an envelope of 0.3) mean " << score.meanLatencyInMs << " ms, max " << score.maxLatencyInMs << " ms, active runs " << score.activeRuns );
                ASSERT_TRUE( score.truthFrames > 500 && score.gapFrames > 500 );
                ASSERT_TRUE( score.HitRate() >= requiredHitRate[ s ] );
                ASSERT_TRUE( score.FalseAlarmRate() < 0.01 );
                ASSERT_TRUE( score.talkspursDetected == speech.talkspurts.size() );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: Speech-like bursts are detected at 10 dB signal to noise ratio at the processing rates 8, 16, 32 and 48 kHz" )
    try
    {
        const UInt32 rates[ 4 ] = { 8000, 16000, 32000, 48000 };
        for ( int r=0; r<4; ++r )
        {
            const Sig::SSpeech& speech = TestSpeech( rates[ r ] );
            const TSamples noise = Sig::MakeWhiteNoise( speech.samples.size(), 1.0, 200 + (UInt64) r );
            const TSamples mixed = Sig::MixAtSnr( speech, noise, 10.0 );
            SFrameLog log;
            RunByFrames( mixed, rates[ r ], log );
            const SScore score = ScoreAgainstTruth( speech, log, rates[ r ] );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceActivity: rate " << rates[ r ] << " Hz, SNR 10 dB: hit rate " << 100.0 * score.HitRate() << " %, false alarm rate " << 100.0 * score.FalseAlarmRate() << " %, onset latency mean " << score.meanLatencyInMs << " ms" );
            ASSERT_TRUE( score.HitRate() >= 0.95 );
            ASSERT_TRUE( score.FalseAlarmRate() < 0.01 );
            ASSERT_TRUE( score.talkspursDetected == speech.talkspurts.size() );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: Steady white and pink noise, hum with white noise and digital silence are never taken for voice" )
    try
    {
        const size_t count = 16000 * 30;
        struct { const char* name; TSamples signal; } cases[ 6 ];
        cases[ 0 ].name = "white noise -40 dBFS";    cases[ 0 ].signal = Sig::MakeWhiteNoise( count, Sig::FromDecibels( -40.0 ), 1 );
        cases[ 1 ].name = "pink noise -40 dBFS";     cases[ 1 ].signal = Sig::MakePinkNoise( count, Sig::FromDecibels( -40.0 ), 2 );
        cases[ 2 ].name = "white noise -75 dBFS";    cases[ 2 ].signal = Sig::MakeWhiteNoise( count, Sig::FromDecibels( -75.0 ), 3 );
        cases[ 3 ].name = "50 Hz hum -30 dBFS with white noise -50 dBFS";
        cases[ 3 ].signal = Sig::MakeHum( count, 16000, 50.0, 8, Sig::FromDecibels( -30.0 ) );
        Sig::MixInto( cases[ 3 ].signal, Sig::MakeWhiteNoise( count, Sig::FromDecibels( -50.0 ), 4 ) );
        cases[ 4 ].name = "60 Hz hum -35 dBFS with pink noise -45 dBFS";
        cases[ 4 ].signal = Sig::MakeHum( count, 16000, 60.0, 8, Sig::FromDecibels( -35.0 ) );
        Sig::MixInto( cases[ 4 ].signal, Sig::MakePinkNoise( count, Sig::FromDecibels( -45.0 ), 5 ) );
        cases[ 5 ].name = "digital silence";         cases[ 5 ].signal = Sig::MakeSilence( count );
        for ( int c=0; c<6; ++c )
        {
            SFrameLog log;
            RunByFrames( cases[ c ].signal, 16000, log );
            const Float64 fraction = ActiveFraction( log, 0 );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceActivity: " << cases[ c ].name << ": " << 100.0 * fraction << " % of " << log.active.size() << " frames active" );
            ASSERT_TRUE( log.active.size() == 3000 );
            ASSERT_TRUE( fraction < 0.01 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // A click is one frame of energy far above the floor, or two when it lies across the boundary of two frames; the detector
    // needs two frames for an onset, so some clicks start the voice. What it must not do is stay active
    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: An isolated click or tick never keeps the voice active beyond the onset, the hangover and one frame" )
    try
    {
        const size_t count = 16000 * 20;
        const UInt32 widths[ 2 ] = { 1, 16 };
        const Float64 peaks[ 3 ] = { 0.02, 0.1, 0.5 };
        for ( int w=0; w<2; ++w )
        {
            for ( int p=0; p<3; ++p )
            {
                TSamples signal = Sig::MakeWhiteNoise( count, Sig::FromDecibels( -55.0 ), 7 );
                Sig::TSpans positions;
                Sig::MixInto( signal, Sig::MakeClicks( count, 16000, 0.7, peaks[ p ], widths[ w ], 8, &positions ) );
                SFrameLog log;
                RunByFrames( signal, 16000, log );
                UInt32 runs = 0, longest = 0, run = 0;
                for ( size_t k=0; k<log.active.size(); ++k )
                {
                    if ( log.active[ k ] )
                        ++run;
                    if ( ( !log.active[ k ] || k + 1 == log.active.size() ) && run > 0 )
                    {
                        ++runs;
                        longest = ( run > longest ) ? run : longest;
                        run = 0;
                    }
                }
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceActivity: " << positions.size() << " clicks of width " << widths[ w ] << " and peak " << peaks[ p ] << " (" << Sig::ToDecibels( peaks[ p ] ) << " dBFS) on white noise at -55 dBFS: " << runs << " activations, longest " << 10 * longest << " ms" );
                ASSERT_TRUE( longest <= 2 + 20 + 1 );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformVoiceDetectorTimingTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: The noise floor follows a fall of the noise level within 2 s and a rise of the noise level at the rate of the settings" )
    try
    {
        const UInt32 rate = 16000;
        struct SStep { Float64 fromDb; Float64 toDb; Float64 beforeSeconds; Float64 afterSeconds; const char* name; };
        const SStep steps[ 3 ] = {
            { -40.0, -60.0, 6.0, 8.0, "fall of 20 dB" },
            { -50.0, -44.0, 6.0, 10.0, "rise of 6 dB" },
            { -50.0, -38.0, 6.0, 14.0, "rise of 12 dB" } };
        for ( int s=0; s<3; ++s )
        {
            const SStep& step = steps[ s ];
            TSamples signal = Sig::MakeWhiteNoise( (size_t) ( step.beforeSeconds * rate ), Sig::FromDecibels( step.fromDb ), 30 + (UInt64) s );
            Sig::Append( signal, Sig::MakeWhiteNoise( (size_t) ( step.afterSeconds * rate ), Sig::FromDecibels( step.toDb ), 40 + (UInt64) s ) );
            SFrameLog log;
            RunByFrames( signal, rate, log );
            const size_t stepFrame = (size_t) ( step.beforeSeconds * 100.0 );
            const Float32 before = log.floor[ stepFrame - 1 ];
            const Float32 settled = log.floor[ log.floor.size() - 1 ];
            size_t reached = 0;
            for ( size_t k=stepFrame; k<log.floor.size(); ++k )
            {
                if ( fabsf( log.floor[ k ] - settled ) <= 1.5f )
                {
                    reached = k;
                    break;
                }
            }
            size_t lastActive = 0;
            for ( size_t k=stepFrame; k<log.active.size(); ++k )
                lastActive = log.active[ k ] ? k : lastActive;
            const Float64 trackSeconds = (Float64) ( reached - stepFrame ) / 100.0;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceActivity: " << step.name << ": floor " << before << " dB before, " << settled << " dB settled (" << settled - before << " dB), within 1.5 dB after " << trackSeconds <<
                       " s, active until " << (Float64) ( lastActive > stepFrame ? lastActive - stepFrame : 0 ) / 100.0 << " s after the step" );
            ASSERT_TRUE( fabs( ( settled - before ) - ( step.toDb - step.fromDb ) ) < 2.0 );
            if ( s == 0 )
                ASSERT_TRUE( trackSeconds <= 2.0 );
            else
                ASSERT_TRUE( trackSeconds <= 1.5 + 2.0 + (Float64) ( step.toDb - step.fromDb ) / 3.0 );        // the window, the smoothing and the rate limit
            if ( s == 0 )
                ASSERT_TRUE( 0 == lastActive || lastActive < stepFrame );                                        // a fall of the noise never looks like voice
            else
                ASSERT_TRUE( (Float64) ( lastActive > stepFrame ? lastActive - stepFrame : 0 ) / 100.0 <= 1.5 + ( step.toDb - step.fromDb ) / 3.0 + 1.0 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // A steady floor makes the margins exact: a tone of 1500 Hz is the floor, a second tone of 1000 Hz adds a known number of dB
    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: Hysteresis: between the release and the onset margin the voice neither starts nor stops; the onset needs 9 dB over two frames, the end follows the hangover" )
    try
    {
        const UInt32 rate = 16000;
        const Float64 bedRms = Sig::FromDecibels( -50.0 );

        // 7.5 dB over the floor, between the margins: never starts
        {
            const Float64 levels[ 2 ] = { 0.0, 7.5 };
            const Float64 seconds[ 2 ] = { 3.0, 1.2 };
            SFrameLog log;
            RunByFrames( MakeToneSteps( rate, bedRms, levels, seconds, 2 ), rate, log );
            ASSERT_TRUE( 0.0 == ActiveFraction( log, 0 ) );
            ASSERT_TRUE( fabsf( log.floor[ 100 ] - log.floor[ log.floor.size() - 1 ] ) < 0.1f );
        }

        // 15 dB starts it, 7.5 dB keeps it, 4 dB ends it after the hangover
        {
            const Float64 levels[ 5 ] = { 0.0, 15.0, 7.5, 4.0, 0.0 };
            const Float64 seconds[ 5 ] = { 3.0, 0.4, 0.8, 0.8, 1.5 };
            SFrameLog log;
            AUDIO::CVoiceActivityDetector detector;
            RunByFrames( MakeToneSteps( rate, bedRms, levels, seconds, 5 ), rate, AUDIO::CVoiceActivitySettings(), log, detector );
            size_t firstActive = 0;
            while ( firstActive < log.active.size() && !log.active[ firstActive ] )
                ++firstActive;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceActivity: a step of 15 dB at 3.00 s: active at the end of frame " << firstActive << " (" << 10 * ( firstActive + 1 ) << " ms)" );
            ASSERT_TRUE( 301 == firstActive );                                               // frames 300 and 301 are over the margin, the decision follows the second
            for ( size_t k=firstActive; k<420; ++k )
                ASSERT_TRUE( 0 != log.active[ k ] );                                         // 7.5 dB keeps it active
            size_t firstInactive = 420;
            while ( firstInactive < log.active.size() && log.active[ firstInactive ] )
                ++firstInactive;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceActivity: the level drops to 4 dB at 4.20 s: inactive at the end of frame " << firstInactive << " (" << 10 * ( firstInactive + 1 ) << " ms)" );
            ASSERT_TRUE( 439 == firstInactive );                                             // 20 frames below the release margin: frames 420 to 439, the decision follows the last
            for ( size_t k=firstInactive; k<log.active.size(); ++k )
                ASSERT_TRUE( 0 == log.active[ k ] );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: The voice ends the hangover after the end of the burst, for hangovers from 0 to 500 ms and burst ends at every position in a frame" )
    try
    {
        const UInt32 rate = 16000;
        const Float64 bedRms = Sig::FromDecibels( -50.0 );
        const Float32 hangovers[ 5 ] = { 0.0f, 50.0f, 100.0f, 200.0f, 500.0f };
        for ( int h=0; h<5; ++h )
        {
            Float64 minimum = 1.0e9, maximum = -1.0e9;
            for ( int j=0; j<10; ++j )
            {
                const Float64 levels[ 3 ] = { 0.0, 20.0, 0.0 };
                const Float64 seconds[ 3 ] = { 3.0, 1.0 + 0.001 * (Float64) j, 2.0 };
                AUDIO::CVoiceActivitySettings settings;
                settings.SetHangoverInMs( hangovers[ h ] );
                SFrameLog log;
                AUDIO::CVoiceActivityDetector detector;
                RunByFrames( MakeToneSteps( rate, bedRms, levels, seconds, 3 ), rate, settings, log, detector );
                size_t firstInactive = 400;
                while ( firstInactive < log.active.size() && log.active[ firstInactive ] )
                    ++firstInactive;
                // from the end of the burst to the end of the last frame that was still active
                const Float64 burstEnd = 4.0 + 0.001 * (Float64) j;
                const Float64 hangoverSeen = 0.01 * (Float64) ( firstInactive + 1 ) - burstEnd;
                minimum = ( hangoverSeen < minimum ) ? hangoverSeen : minimum;
                maximum = ( hangoverSeen > maximum ) ? hangoverSeen : maximum;
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceActivity: hangover " << hangovers[ h ] << " ms: the voice ended " << 1000.0 * minimum << " to " << 1000.0 * maximum << " ms after the end of the burst" );
            ASSERT_TRUE( minimum * 1000.0 >= (Float64) hangovers[ h ] - 11.0 );
            ASSERT_TRUE( maximum * 1000.0 <= (Float64) hangovers[ h ] + 21.0 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: Onset latency: a voiced burst with an abrupt start is reported within 30 ms at 20 and 10 dB signal to noise ratio and 50 ms at 5 dB, at every position in the analysis frame" )
    try
    {
        const UInt32 rate = 16000;
        const int snrs[ 3 ] = { 20, 10, 5 };
        for ( int s=0; s<3; ++s )
        {
            Float64 minimum = 1.0e9, maximum = -1.0e9, sum = 0.0;
            const int offsets = 20;
            for ( int j=0; j<offsets; ++j )
            {
                const size_t total = rate * 6;
                const size_t start = rate * 3 + (size_t) ( 0.0005 * (Float64) j * rate );
                TSamples burst( total, 0.0f );
                Sig::AddVoicedBurst( burst, GUCEF_NULL, rate, start, (size_t) ( 0.4 * rate ), 120.0, 140.0, 1.0, 0.0, 32, 77 );
                // the flat burst has the RMS the ratio asks for against white noise of RMS 0.01
                const Float64 burstRms = Sig::Rms( burst, start + 64, (size_t) ( 0.4 * rate ) - 128 );
                Sig::Scale( burst, 0.01 * Sig::FromDecibels( (Float64) snrs[ s ] ) / burstRms );
                Sig::MixInto( burst, Sig::MakeWhiteNoise( total, 0.01, 50 + (UInt64) j ) );
                SFrameLog log;
                RunByFrames( burst, rate, log );
                size_t first = 0;
                for ( size_t k=(size_t) ( start * 100 / rate ); k<log.active.size(); ++k )
                {
                    if ( log.active[ k ] )
                    {
                        first = k;
                        break;
                    }
                }
                ASSERT_TRUE( first > 0 );
                const Float64 latency = 1000.0 * ( (Float64) FrameEnd( rate, first ) - (Float64) start ) / (Float64) rate;
                minimum = ( latency < minimum ) ? latency : minimum;
                maximum = ( latency > maximum ) ? latency : maximum;
                sum += latency;
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceActivity: onset latency of an abrupt voiced burst at SNR " << snrs[ s ] << " dB: " << minimum << " to " << maximum << " ms, mean " << sum / (Float64) offsets << " ms (" << offsets << " positions in the frame)" );
            if ( snrs[ s ] >= 10 )
                ASSERT_TRUE( maximum <= 30.0 );
            else
                ASSERT_TRUE( maximum <= 50.0 );                                                                  // at 5 dB a weak first frame can cost two more
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: No chatter: at 3 dB signal to noise ratio every stretch of voice lasts at least the hangover and the syllables of a talkspurt are one stretch" )
    try
    {
        const Sig::SSpeech& speech = TestSpeech( 16000 );
        const TSamples mixed = Sig::MixAtSnr( speech, Sig::MakeWhiteNoise( speech.samples.size(), 1.0, 61 ), 3.0 );
        SFrameLog log;
        RunByFrames( mixed, 16000, log );
        const SScore score = ScoreAgainstTruth( speech, log, 16000 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceActivity: SNR 3 dB: " << score.activeRuns << " stretches of voice for " << speech.talkspurts.size() << " talkspurts, the shortest " << 10 * score.shortestRunFrames << " ms, hit rate " << 100.0 * score.HitRate() << " %, false alarm rate " << 100.0 * score.FalseAlarmRate() << " %" );
        ASSERT_TRUE( score.activeRuns >= 1 );
        ASSERT_TRUE( score.shortestRunFrames >= 20 );
        ASSERT_TRUE( score.activeRuns <= speech.talkspurts.size() + 2 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformVoiceDetectorStreamTests( void )
{GUCEF_TRACE;

    // The decisions depend on the audio and its position and on nothing else. The reference is fed in pieces that end at
    // analysis frame boundaries, so every frame can be seen; the others are fed in blocks of the sizes below and are
    // compared with the reference at every point where they show a new frame
    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: Block size invariance: decisions, energies and floors are bit for bit those of the reference for block sizes from 1 to 7919 frames, also at rates with a fraction of a frame per 10 ms" )
    try
    {
        const UInt32 rates[ 5 ] = { 8000, 16000, 22050, 44100, 48000 };
        const UInt32 blockSizes[ 13 ] = { 1, 3, 17, 64, 100, 160, 333, 441, 480, 1000, 1600, 4096, 7919 };
        for ( int r=0; r<5; ++r )
        {
            Sig::SSpeechSpec spec;
            spec.sampleRate = rates[ r ];
            spec.seconds = 9.0;
            spec.seed = 21 + (UInt64) r;
            Sig::SSpeech speech;
            Sig::MakeSpeech( spec, speech );
            const TSamples mixed = Sig::MixAtSnr( speech, Sig::MakeWhiteNoise( speech.samples.size(), 1.0, 70 + (UInt64) r ), 8.0 );

            SFrameLog reference;
            RunByFrames( mixed, rates[ r ], reference );
            ASSERT_TRUE( reference.active.size() == 900 );
            size_t activeFrames = 0;
            for ( size_t k=0; k<reference.active.size(); ++k )
                activeFrames += reference.active[ k ];
            ASSERT_TRUE( activeFrames > 100 && activeFrames < 800 );

            for ( int b=0; b<13; ++b )
            {
                AUDIO::CVoiceActivityDetector detector;
                ASSERT_TRUE( detector.Prepare( rates[ r ] ) );
                size_t position = 0;
                UInt32 compared = 0;
                while ( position < mixed.size() )
                {
                    size_t block = blockSizes[ b ];
                    if ( block > mixed.size() - position )
                        block = mixed.size() - position;
                    detector.Process( &mixed[ position ], (UInt32) block );
                    position += block;
                    const UInt64 seen = detector.GetAnalyzedFrameCount();
                    if ( seen > 0 )
                    {
                        const size_t k = (size_t) seen - 1;
                        const Float32 energy = detector.GetEnergyInDecibels();
                        const Float32 floor = detector.GetNoiseFloorInDecibels();
                        ASSERT_TRUE( ( detector.IsActive() ? 1 : 0 ) == reference.active[ k ] );
                        ASSERT_TRUE( 0 == memcmp( &energy, &reference.energy[ k ], sizeof( Float32 ) ) );
                        ASSERT_TRUE( 0 == memcmp( &floor, &reference.floor[ k ], sizeof( Float32 ) ) );
                        ++compared;
                    }
                }
                ASSERT_TRUE( 900 == detector.GetAnalyzedFrameCount() && (UInt64) mixed.size() == detector.GetPosition() );
                ASSERT_TRUE( compared > 0 );
            }

            // the same audio again after Reset() gives the same log
            AUDIO::CVoiceActivityDetector detector;
            detector.Prepare( rates[ r ] );
            SFrameLog first, second;
            FeedAligned( detector, mixed, rates[ r ], 0, mixed.size(), 0, first );
            detector.Reset();
            ASSERT_TRUE( 0 == detector.GetAnalyzedFrameCount() && !detector.IsActive() );
            FeedAligned( detector, mixed, rates[ r ], 0, mixed.size(), 0, second );
            ASSERT_TRUE( first.active == second.active && first.energy == second.energy && first.floor == second.floor );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceActivity: block size invariance checked for 5 rates and 13 block sizes, 900 frames each" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: A gap in the stream drops the analysis frame it falls in, keeps the frames aligned to the absolute position and the decisions independent of the block size" )
    try
    {
        const UInt32 rates[ 2 ] = { 16000, 22050 };
        for ( int r=0; r<2; ++r )
        {
            const UInt32 rate = rates[ r ];
            Sig::SSpeechSpec spec;
            spec.sampleRate = rate;
            spec.seconds = 8.0;
            spec.seed = 33;
            Sig::SSpeech speech;
            Sig::MakeSpeech( spec, speech );
            const TSamples mixed = Sig::MixAtSnr( speech, Sig::MakeWhiteNoise( speech.samples.size(), 1.0, 80 ), 10.0 );
            const size_t part = rate * 3;
            const UInt64 gap = 12345;                       // frames lost, not a multiple of an analysis frame

            AUDIO::CVoiceActivityDetector referenceDetector;
            referenceDetector.Prepare( rate );
            SFrameLog reference;
            FeedAligned( referenceDetector, mixed, rate, 0, part, 0, reference );
            const UInt64 framesBefore = referenceDetector.GetAnalyzedFrameCount();
            FeedAligned( referenceDetector, mixed, rate, part, mixed.size() - part, (UInt64) part + gap, reference );
            const UInt64 resumeFrame = ( ( (UInt64) part + gap ) * 100 ) / rate;
            const UInt64 lastFrame = ( ( (UInt64) mixed.size() + gap ) * 100 ) / rate;       // frames that completed
            // frames before the gap: 300; the frame that holds the resume position is dropped, the others after it are complete
            ASSERT_TRUE( 300 == framesBefore );
            ASSERT_TRUE( reference.active.size() == (size_t) ( framesBefore + ( lastFrame - ( resumeFrame + 1 ) ) ) );

            const UInt32 blockSizes[ 4 ] = { 7, 160, 1000, 4096 };
            for ( int b=0; b<4; ++b )
            {
                AUDIO::CVoiceActivityDetector detector;
                detector.Prepare( rate );
                UInt64 position = 0;
                size_t done = 0;
                while ( done < mixed.size() )
                {
                    if ( done == part )
                        position = (UInt64) part + gap;
                    size_t block = blockSizes[ b ];
                    const size_t limit = ( done < part ) ? part : mixed.size();
                    if ( block > limit - done )
                        block = limit - done;
                    detector.Process( &mixed[ done ], (UInt32) block, position );
                    done += block;
                    position += block;
                    const UInt64 seen = detector.GetAnalyzedFrameCount();
                    if ( seen > 0 )
                    {
                        ASSERT_TRUE( seen <= reference.active.size() );
                        const size_t k = (size_t) seen - 1;
                        const Float32 energy = detector.GetEnergyInDecibels();
                        ASSERT_TRUE( ( detector.IsActive() ? 1 : 0 ) == reference.active[ k ] );
                        ASSERT_TRUE( 0 == memcmp( &energy, &reference.energy[ k ], sizeof( Float32 ) ) );
                    }
                }
                ASSERT_TRUE( reference.active.size() == (size_t) detector.GetAnalyzedFrameCount() );
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceActivity: rate " << rate << " Hz: a gap of " << gap << " frames after 3 s: " << framesBefore << " frames before it, " << reference.active.size() - framesBefore << " after it" );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: Silence, tiny and denormal samples, and samples that are not numbers leave the detector sane" )
    try
    {
        // digital silence and denormals: nothing is detected, the energy reads -120 dB and the floor stops at the minimum level
        {
            AUDIO::CVoiceActivityDetector detector;
            ASSERT_TRUE( detector.Prepare( 16000 ) );
            TSamples silence = Sig::MakeSilence( 16000 * 20 );
            ASSERT_FALSE( detector.Process( &silence[ 0 ], (UInt32) silence.size() ) );
            ASSERT_TRUE( -120.0f == detector.GetEnergyInDecibels() && -65.0f == detector.GetNoiseFloorInDecibels() );
            for ( size_t i=0; i<silence.size(); ++i )
                silence[ i ] = ( 0 == ( i & 1 ) ) ? 1.0e-40f : -1.0e-40f;
            ASSERT_FALSE( detector.Process( &silence[ 0 ], (UInt32) silence.size() ) );
            ASSERT_TRUE( -120.0f == detector.GetEnergyInDecibels() );

            // a burst after a long silence: energy reads, then silence again brings the energy back to -120 dB (the filter state decays)
            const TSamples burst = Sig::MakeTone( 16000, 16000, 1000.0, 0.1 );
            detector.Process( &burst[ 0 ], (UInt32) burst.size() );
            ASSERT_TRUE( detector.GetEnergyInDecibels() > -30.0f );
            ASSERT_TRUE( detector.IsActive() );
            const TSamples after = Sig::MakeSilence( 16000 * 2 );
            detector.Process( &after[ 0 ], (UInt32) after.size() );
            ASSERT_FALSE( detector.IsActive() );
            ASSERT_TRUE( -120.0f == detector.GetEnergyInDecibels() );
        }

        // not numbers and infinities are taken as silence and do not poison the filters
        {
            const TSamples noise = Sig::MakeWhiteNoise( 16000 * 3, Sig::FromDecibels( -40.0 ), 90 );
            AUDIO::CVoiceActivityDetector clean, damaged;
            clean.Prepare( 16000 );
            damaged.Prepare( 16000 );
            TSamples bad = noise;
            volatile Float32 zero = 0.0f;
            for ( size_t i=16000; i<16160; ++i )
                bad[ i ] = ( 0 == ( i % 3 ) ) ? zero / zero : ( ( 1 == ( i % 3 ) ) ? 1.0f / zero : -1.0f / zero );
            clean.Process( &noise[ 0 ], (UInt32) noise.size() );
            damaged.Process( &bad[ 0 ], (UInt32) bad.size() );
            const Float32 energy = damaged.GetEnergyInDecibels();
            ASSERT_TRUE( energy == energy && energy > -80.0f && energy < -30.0f );
            const Float32 floor = damaged.GetNoiseFloorInDecibels();
            ASSERT_TRUE( floor == floor && fabsf( floor - clean.GetNoiseFloorInDecibels() ) < 6.0f );
            ASSERT_FALSE( damaged.IsActive() );
        }

        // settings that change while the stream runs act from the next frame: an onset margin of 60 dB is never reached
        {
            const Sig::SSpeech& speech = TestSpeech( 16000 );
            const TSamples mixed = Sig::MixAtSnr( speech, Sig::MakeWhiteNoise( speech.samples.size(), 1.0, 95 ), 15.0 );
            AUDIO::CVoiceActivitySettings strict;
            strict.SetOnsetInDecibels( 60.0f );
            SFrameLog strictLog, normalLog;
            AUDIO::CVoiceActivityDetector detector;
            RunByFrames( mixed, 16000, strict, strictLog, detector );
            RunByFrames( mixed, 16000, normalLog );
            ASSERT_TRUE( 0.0 == ActiveFraction( strictLog, 0 ) );
            ASSERT_TRUE( ActiveFraction( normalLog, 0 ) > 0.2 );

            // a minimum level above the speech keeps it silent as well
            AUDIO::CVoiceActivitySettings deaf;
            deaf.SetMinimumLevelInDecibels( -10.0f );
            SFrameLog deafLog;
            RunByFrames( mixed, 16000, deaf, deafLog, detector );
            ASSERT_TRUE( 0.0 == ActiveFraction( deafLog, 0 ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CAPTURE PATH OF THE ENGINE                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 VoicePushPeriod = 160;

/** A source that plays the frames of a buffer once and then has nothing; a copy per read */
class CVoiceBufferSource : public AUDIO::IAudioSource
{
    public:

    explicit CVoiceBufferSource( const AUDIO::CAudioBuffer& buffer ) : m_buffer( buffer ), m_position( 0 ) {}

    virtual const AUDIO::CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_buffer.GetFormat(); }

    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt32 left = ( m_position < m_buffer.GetFrameCount() ) ? m_buffer.GetFrameCount() - (UInt32) m_position : 0;
        const UInt32 count = ( left < frameCapacity ) ? left : frameCapacity;
        if ( count > 0 )
            memcpy( destination, m_buffer.GetDataPtr( (UInt32) m_position ), (size_t) count * m_buffer.GetFormat().GetBytesPerFrame() );
        m_position += count;
        return count;
    }

    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_position >= m_buffer.GetFrameCount(); }
    virtual bool IsRealtimeSafe( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_position; }
    virtual UInt64 GetLengthInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_buffer.GetFrameCount(); }

    private:

    AUDIO::CAudioBuffer m_buffer;
    UInt64 m_position;
};

/** Keeps what the capture path delivers in memory made beforehand: the samples and, per block, its position, size and voice decision */
class CVoiceConsumer : public AUDIO::IAudioCaptureConsumer
{
    public:

    struct SBlock
    {
        UInt64 firstFrame;
        UInt32 frames;
        bool voiceActive;
        SBlock( void ) : firstFrame( 0 ), frames( 0 ), voiceActive( false ) {}
    };

    CVoiceConsumer( size_t capacityInFrames, size_t maxBlocks )
        : samples(), frames( 0 ), blocks(), blockCount( 0 ), calls( 0 ), activeBlocks( 0 )
    {
        samples.assign( capacityInFrames, 0.0f );
        blocks.assign( maxBlocks, SBlock() );
    }

    virtual bool PrepareForCapture( const AUDIO::CAudioFormat&, UInt32 ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual void OnCapturedAudio( const Float32* mono, UInt32 frameCount, const SCaptureBlockInfo& info ) GUCEF_VIRTUAL_OVERRIDE GUCEF_AUDIO_REALTIME_CONTEXT
    {
        const size_t room = samples.size() - frames;
        const size_t count = ( frameCount < room ) ? frameCount : room;
        if ( count > 0 )
            memcpy( &samples[ frames ], mono, count * sizeof( Float32 ) );
        frames += count;
        if ( blockCount < blocks.size() )
        {
            blocks[ blockCount ].firstFrame = info.firstFrame;
            blocks[ blockCount ].frames = frameCount;
            blocks[ blockCount ].voiceActive = info.voiceActive;
        }
        ++blockCount;
        if ( info.voiceActive )
            activeBlocks.FetchAdd( 1 );
        calls.FetchAdd( 1 );
    }

    TSamples samples;
    size_t frames;
    GUCEF::vector< SBlock > blocks;
    size_t blockCount;
    MT::CTAtomic< UInt32 > calls;
    MT::CTAtomic< UInt32 > activeBlocks;
};

AUDIO::CAudioEngineParams
VoiceCaptureParams( UInt32 deviceRate, UInt32 processingRate, UInt32 deviceBuffer )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
    params.GetDeviceParams().SetBufferSizeInFrames( 256 );
    params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );
    params.SetCaptureEnabled( true );
    params.SetCaptureProcessingRate( processingRate );
    params.GetCaptureDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( deviceRate, 1, AUDIO::SAMPLETYPE_FLOAT32 ) );
    params.GetCaptureDeviceParams().SetBufferSizeInFrames( deviceBuffer );
    return params;
}

AUDIO::CNullAudioDevice*
VoiceCaptureNull( AUDIO::CAudioEngine& engine )
{
    AUDIO::TAudioDevicePtr device = engine.GetCaptureDevice();
    return device.IsNULL() ? GUCEF_NULL : device->AsNullDevice();
}

AUDIO::TAudioSourcePtr
MakeVoiceSource( const AUDIO::CAudioFormat& format, const TSamples& signal )
{
    AUDIO::CAudioBuffer buffer( format, (UInt32) signal.size() );
    for ( size_t f=0; f<signal.size(); ++f )
        buffer.SetSampleFromFloat( (UInt32) f, 0, signal[ f ] );
    return AUDIO::TAudioSourcePtr( GUCEF_NEW CVoiceBufferSource( buffer ) );
}

/** What an engine run delivered: the processed audio at the processing rate and the decision per analysis frame as the blocks showed it (-1: no block ended in that frame) */
struct SEngineRun
{
    TSamples recorded;
    GUCEF::vector< Int8 > decisionAtFrame;
    UInt32 processingRate;
    bool activeAtEnd;
    SEngineRun( void ) : recorded(), decisionAtFrame(), processingRate( 0 ), activeAtEnd( false ) {}
};

/**
 *  Plays 'input' (at 'deviceRate') into an engine whose capture path is set up as given (device periods of 'deviceBuffer'
 *  frames, the settings of 'settings' when given) and renders 'tailSeconds' of silence more
 */
bool
RunEngineCapture( const TSamples& input, UInt32 deviceRate, UInt32 processingRate, UInt32 deviceBuffer, const AUDIO::CVoiceActivitySettings* settings, Float64 tailSeconds, SEngineRun& run )
{
    AUDIO::CAudioEngineParams params = VoiceCaptureParams( deviceRate, processingRate, deviceBuffer );
    if ( GUCEF_NULL != settings )
        params.GetVoiceActivitySettings() = *settings;
    AUDIO::CAudioEngine engine;
    if ( !engine.Initialize( params ) || !engine.IsCaptureAvailable() || GUCEF_NULL == VoiceCaptureNull( engine ) )
        return false;
    run.processingRate = engine.GetCaptureProcessingFormat().GetSampleRate();
    const AUDIO::CAudioFormat deviceFormat = engine.GetCaptureDevice()->GetFormat();
    if ( 1 != deviceFormat.GetChannelCount() || deviceFormat.GetSampleRate() != deviceRate )
        return false;
    if ( !VoiceCaptureNull( engine )->SetCaptureSource( MakeVoiceSource( deviceFormat, input ) ) )
        return false;

    const UInt64 totalDeviceFrames = (UInt64) input.size() + (UInt64) ( tailSeconds * (Float64) deviceRate );
    const size_t capacity = (size_t) ( totalDeviceFrames * run.processingRate / deviceRate ) + 8192;
    CVoiceConsumer consumer( capacity, capacity / 16 + 1024 );
    CORE::CFutureResult added = engine.AddCaptureConsumer( &consumer );
    if ( added.HasNoFuture() )
        return false;
    UInt64 rendered = 0;
    while ( rendered < totalDeviceFrames )
    {
        const UInt64 step = ( totalDeviceFrames - rendered < deviceRate / 10 ) ? totalDeviceFrames - rendered : deviceRate / 10;
        VoiceCaptureNull( engine )->Render( step );
        engine.Update();
        rendered += step;
    }
    run.activeAtEnd = engine.IsCaptureVoiceActive();
    CORE::CFutureResult removed = engine.RemoveCaptureConsumer( &consumer );
    VoiceCaptureNull( engine )->Render( deviceBuffer );
    engine.Update();
    if ( !added.IsReady() || !removed.IsReady() || consumer.blockCount > consumer.blocks.size() )
        return false;

    run.recorded.assign( consumer.samples.begin(), consumer.samples.begin() + consumer.frames );
    run.decisionAtFrame.assign( run.recorded.size() * 100 / run.processingRate + 2, -1 );
    for ( size_t b=0; b<consumer.blockCount; ++b )
    {
        const UInt64 end = consumer.blocks[ b ].firstFrame + consumer.blocks[ b ].frames;
        const UInt64 completed = ( end * 100 ) / run.processingRate;
        if ( completed > 0 && completed - 1 < run.decisionAtFrame.size() )
            run.decisionAtFrame[ (size_t) completed - 1 ] = consumer.blocks[ b ].voiceActive ? 1 : 0;
    }
    engine.Shutdown();
    return true;
}

/** The compared frames of two runs: how many have a decision in both, and how many of those differ */
void
CompareDecisions( const SEngineRun& a, const SEngineRun& b, UInt32& shared, UInt32& different )
{
    shared = 0;
    different = 0;
    const size_t count = ( a.decisionAtFrame.size() < b.decisionAtFrame.size() ) ? a.decisionAtFrame.size() : b.decisionAtFrame.size();
    for ( size_t k=0; k<count; ++k )
    {
        if ( a.decisionAtFrame[ k ] >= 0 && b.decisionAtFrame[ k ] >= 0 )
        {
            ++shared;
            different += ( a.decisionAtFrame[ k ] != b.decisionAtFrame[ k ] ) ? 1 : 0;
        }
    }
}

/** The decisions that a standalone detector makes on the audio an engine run recorded, against the decisions the blocks showed */
void
CompareWithStandalone( const SEngineRun& run, UInt32& shared, UInt32& different )
{
    SFrameLog log;
    RunByFrames( run.recorded, run.processingRate, log );
    shared = 0;
    different = 0;
    for ( size_t k=0; k<log.active.size() && k<run.decisionAtFrame.size(); ++k )
    {
        if ( run.decisionAtFrame[ k ] >= 0 )
        {
            ++shared;
            different += ( run.decisionAtFrame[ k ] != log.active[ k ] ) ? 1 : 0;
        }
    }
}

/** A capture push device on a thread of its own that records a signal, 10 times faster than real time, and zeros after the end */
class CVoicePushCaptureDevice : public AUDIO::CAudioPushDevice
{
    public:

    CVoicePushCaptureDevice( AUDIO::IAudioDeviceCallback* callback, const TSamples* signal )
        : AUDIO::CAudioPushDevice( AUDIO::CAudioDeviceInfo( "voicecapture", "Voice capture push device", AUDIO::AUDIODIRECTION_CAPTURE ), callback, AUDIO::CAudioFormat( 16000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), VoicePushPeriod )
        , pulls( 0 ), m_signal( signal ), m_position( 0 )
    {}

    virtual ~CVoicePushCaptureDevice() GUCEF_VIRTUAL_OVERRIDE
    {
        Stop();
    }

    MT::CTAtomic< UInt32 > pulls;

    protected:

    virtual bool SupportsCapture( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool OnPushStart( void ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual void OnPushStop( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual TPushResult PushFrames( const void*, UInt32 ) GUCEF_VIRTUAL_OVERRIDE { return PUSHRESULT_OK; }

    virtual TPushResult PullFrames( void* frames, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::PrecisionDelay( 1 );
        Float32* out = static_cast< Float32* >( frames );
        for ( UInt32 f=0; f<frameCount; ++f )
            out[ f ] = ( m_position < m_signal->size() ) ? ( *m_signal )[ m_position++ ] : 0.0f;
        pulls.FetchAdd( 1 );
        return PUSHRESULT_OK;
    }

    private:

    const TSamples* m_signal;
    size_t m_position;
};

class CVoicePacedPlaybackPushDevice : public AUDIO::CAudioPushDevice
{
    public:

    explicit CVoicePacedPlaybackPushDevice( AUDIO::IAudioDeviceCallback* callback )
        : AUDIO::CAudioPushDevice( AUDIO::CAudioDeviceInfo( "voiceplayback", "Voice playback push device", AUDIO::AUDIODIRECTION_PLAYBACK ), callback, AUDIO::CNullAudioDriver::GetDefaultFormat(), 240 )
    {}

    virtual ~CVoicePacedPlaybackPushDevice() GUCEF_VIRTUAL_OVERRIDE
    {
        Stop();
    }

    protected:

    virtual bool OnPushStart( void ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual void OnPushStop( void ) GUCEF_VIRTUAL_OVERRIDE {}

    virtual TPushResult PushFrames( const void*, UInt32 ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::PrecisionDelay( 1 );
        return PUSHRESULT_OK;
    }
};

/** The signal the voice test driver records; set by the test */
const TSamples*&
VoiceTestDriverSignal( void )
{
    static const TSamples* signal = GUCEF_NULL;
    return signal;
}

class CVoiceTestDriver : public AUDIO::CAudioDriver
{
    public:

    CVoiceTestDriver( void ) : AUDIO::CAudioDriver() {}

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "voicetest"; }
    virtual Int32 GetPriority( void ) const GUCEF_VIRTUAL_OVERRIDE { return -2000; }

    virtual bool EnumerateDevices( AUDIO::TAudioDirection, AUDIO::TAudioDeviceInfoVector& ) GUCEF_VIRTUAL_OVERRIDE
    {
        return true;
    }

    virtual bool OpenDevice( const AUDIO::CAudioDeviceParams& params, AUDIO::IAudioDeviceCallback* callback, AUDIO::TAudioDevicePtr& device ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( AUDIO::AUDIODIRECTION_CAPTURE == params.GetDirection() )
            device = AUDIO::TAudioDevicePtr( GUCEF_NEW CVoicePushCaptureDevice( callback, VoiceTestDriverSignal() ) );
        else
            device = AUDIO::TAudioDevicePtr( GUCEF_NEW CVoicePacedPlaybackPushDevice( callback ) );
        return true;
    }
};

}

/*-------------------------------------------------------------------------*/

static void
PerformVoiceActivityEngineTests( void )
{GUCEF_TRACE;

    // The speech at 10 dB over white noise goes in at the device, through conversion, resampling and the stages, to the detector
    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: The capture path reports the voice: the decisions of the blocks are those of a detector run on the processed audio, for device rates 16 and 48 kHz and processing rates 16 and 8 kHz" )
    try
    {
        const UInt32 deviceRates[ 3 ] = { 16000, 48000, 48000 };
        const UInt32 processingRates[ 3 ] = { 16000, 16000, 8000 };
        for ( int c=0; c<3; ++c )
        {
            Sig::SSpeechSpec spec;
            spec.sampleRate = deviceRates[ c ];
            spec.seconds = ( 16000 == deviceRates[ c ] ) ? 12.0 : 7.0;
            spec.seed = 40 + (UInt64) c;
            Sig::SSpeech speech;
            Sig::MakeSpeech( spec, speech );
            const TSamples mixed = Sig::MixAtSnr( speech, Sig::MakeWhiteNoise( speech.samples.size(), 1.0, 120 + (UInt64) c ), 10.0 );

            SEngineRun run;
            ASSERT_TRUE( RunEngineCapture( mixed, deviceRates[ c ], processingRates[ c ], 256, GUCEF_NULL, 3.0, run ) );
            ASSERT_TRUE( run.processingRate == processingRates[ c ] );
            ASSERT_TRUE( run.recorded.size() + 64 >= (size_t) ( ( spec.seconds + 3.0 ) * (Float64) processingRates[ c ] ) );

            UInt32 shared = 0, different = 0;
            CompareWithStandalone( run, shared, different );
            size_t activeBlocks = 0;
            for ( size_t k=0; k<run.decisionAtFrame.size(); ++k )
                activeBlocks += ( 1 == run.decisionAtFrame[ k ] ) ? 1 : 0;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceActivity: engine, device " << deviceRates[ c ] << " Hz, processing " << processingRates[ c ] << " Hz: " << shared << " frames with a decision, " << different << " different from the detector run on the processed audio, " << activeBlocks << " active" );
            ASSERT_TRUE( shared > 300 && 0 == different );
            ASSERT_TRUE( activeBlocks > 100 );
            ASSERT_FALSE( run.activeAtEnd );                                    // three seconds of silence at the end

            // the voice shows up where the talkspurts are: mostly active in the middle of the talkspurts, never in the pauses
            UInt32 insideFrames = 0, insideActive = 0, pauseFrames = 0, pauseActive = 0;
            for ( size_t t=0; t<speech.talkspurts.size(); ++t )
            {
                const Sig::SSpan& spurt = speech.talkspurts[ t ];
                const size_t from = (size_t) ( ( spurt.start * 100 ) / deviceRates[ c ] ) + 20;
                const size_t to = (size_t) ( ( spurt.End() * 100 ) / deviceRates[ c ] );
                for ( size_t k=from; k<to && k<run.decisionAtFrame.size(); ++k )
                {
                    if ( run.decisionAtFrame[ k ] >= 0 )
                    {
                        ++insideFrames;
                        insideActive += run.decisionAtFrame[ k ];
                    }
                }
                const size_t pauseFrom = to + 40;
                const size_t pauseTo = ( t + 1 < speech.talkspurts.size() ) ? (size_t) ( ( speech.talkspurts[ t + 1 ].start * 100 ) / deviceRates[ c ] ) : 0;
                for ( size_t k=pauseFrom; k<pauseTo && k<run.decisionAtFrame.size(); ++k )
                {
                    if ( run.decisionAtFrame[ k ] >= 0 )
                    {
                        ++pauseFrames;
                        pauseActive += run.decisionAtFrame[ k ];
                    }
                }
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceActivity: engine: " << insideActive << " of " << insideFrames << " frames inside talkspurts active, " << pauseActive << " of " << pauseFrames << " frames in the pauses active" );
            ASSERT_TRUE( insideFrames > 100 && (Float64) insideActive >= 0.9 * (Float64) insideFrames );
            ASSERT_TRUE( pauseFrames > 20 && 0 == pauseActive );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: The decisions of the capture path do not depend on the size of the device periods" )
    try
    {
        Sig::SSpeechSpec spec;
        spec.sampleRate = 16000;
        spec.seconds = 10.0;
        spec.seed = 45;
        Sig::SSpeech speech;
        Sig::MakeSpeech( spec, speech );
        const TSamples mixed = Sig::MixAtSnr( speech, Sig::MakeWhiteNoise( speech.samples.size(), 1.0, 130 ), 10.0 );
        const UInt32 periods[ 4 ] = { 64, 256, 480, 1000 };
        SEngineRun runs[ 4 ];
        for ( int p=0; p<4; ++p )
            ASSERT_TRUE( RunEngineCapture( mixed, 16000, 16000, periods[ p ], GUCEF_NULL, 1.0, runs[ p ] ) );
        for ( int p=1; p<4; ++p )
        {
            UInt32 shared = 0, different = 0;
            CompareDecisions( runs[ 0 ], runs[ p ], shared, different );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceActivity: engine, device periods 64 against " << periods[ p ] << " frames: " << shared << " frames compared, " << different << " different" );
            ASSERT_TRUE( shared > 150 && 0 == different );
            ASSERT_TRUE( runs[ 0 ].recorded.size() == runs[ p ].recorded.size() );
            ASSERT_TRUE( 0 == memcmp( &runs[ 0 ].recorded[ 0 ], &runs[ p ].recorded[ 0 ], runs[ 0 ].recorded.size() * sizeof( Float32 ) ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: The voice activity settings of the engine parameters reach the capture path, and muting the microphone ends the voice" )
    try
    {
        Sig::SSpeechSpec spec;
        spec.sampleRate = 16000;
        spec.seconds = 8.0;
        spec.seed = 46;
        spec.minSpurtSeconds = 2.0;
        spec.maxSpurtSeconds = 2.5;
        Sig::SSpeech speech;
        Sig::MakeSpeech( spec, speech );
        const TSamples mixed = Sig::MixAtSnr( speech, Sig::MakeWhiteNoise( speech.samples.size(), 1.0, 131 ), 15.0 );

        AUDIO::CVoiceActivitySettings strict;
        strict.SetOnsetInDecibels( 60.0f );
        SEngineRun strictRun, normalRun;
        ASSERT_TRUE( RunEngineCapture( mixed, 16000, 16000, 256, &strict, 1.0, strictRun ) );
        ASSERT_TRUE( RunEngineCapture( mixed, 16000, 16000, 256, GUCEF_NULL, 1.0, normalRun ) );
        UInt32 strictActive = 0, normalActive = 0;
        for ( size_t k=0; k<strictRun.decisionAtFrame.size(); ++k )
            strictActive += ( 1 == strictRun.decisionAtFrame[ k ] ) ? 1 : 0;
        for ( size_t k=0; k<normalRun.decisionAtFrame.size(); ++k )
            normalActive += ( 1 == normalRun.decisionAtFrame[ k ] ) ? 1 : 0;
        ASSERT_TRUE( 0 == strictActive && normalActive > 100 );

        // mute in the middle of the first talkspurt
        ASSERT_FALSE( speech.talkspurts.empty() );
        const Sig::SSpan& spurt = speech.talkspurts[ 0 ];
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( VoiceCaptureParams( 16000, 16000, 256 ) ) );
        ASSERT_TRUE( VoiceCaptureNull( engine )->SetCaptureSource( MakeVoiceSource( engine.GetCaptureDevice()->GetFormat(), mixed ) ) );
        CVoiceConsumer consumer( 16000 * 10, 4096 );
        CORE::CFutureResult added = engine.AddCaptureConsumer( &consumer );
        UInt64 rendered = 0;
        while ( rendered < spurt.start + 16000 )
        {
            VoiceCaptureNull( engine )->Render( 160 );
            engine.Update();
            rendered += 160;
        }
        ASSERT_TRUE( added.IsReady() );
        ASSERT_TRUE( engine.IsCaptureVoiceActive() );
        engine.SetCaptureMuted( true );
        while ( rendered < spurt.start + 16000 + 8000 )
        {
            VoiceCaptureNull( engine )->Render( 160 );
            engine.Update();
            rendered += 160;
        }
        ASSERT_FALSE( engine.IsCaptureVoiceActive() );
        engine.SetCaptureMuted( false );
        CORE::CFutureResult removed = engine.RemoveCaptureConsumer( &consumer );
        VoiceCaptureNull( engine )->Render( 256 );
        engine.Update();
        ASSERT_TRUE( removed.IsReady() );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: The decision is read from another thread while a capture device thread produces it, and the blocks carry it" )
    try
    {
        AUDIO::CAudioDriverRegistry& drivers = AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry();
        drivers.TryRegister( "voicetest", AUDIO::TAudioDriverPtr( GUCEF_NEW CVoiceTestDriver() ) );

        Sig::SSpeechSpec spec;
        spec.sampleRate = 16000;
        spec.seconds = 8.0;
        spec.seed = 47;
        Sig::SSpeech speech;
        Sig::MakeSpeech( spec, speech );
        const TSamples mixed = Sig::MixAtSnr( speech, Sig::MakeWhiteNoise( speech.samples.size(), 1.0, 132 ), 10.0 );
        VoiceTestDriverSignal() = &mixed;

        AUDIO::CAudioEngineParams params;
        params.SetDriverName( "voicetest" );
        params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
        params.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
        params.SetCaptureEnabled( true );
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( params ) );
        ASSERT_TRUE( engine.IsCaptureAvailable() );
        CVoiceConsumer consumer( 16000 * 12, 8192 );
        CORE::CFutureResult added = engine.AddCaptureConsumer( &consumer );
        ASSERT_FALSE( added.HasNoFuture() );
        ASSERT_TRUE( engine.WaitFor( added, 10000 ) && !added.HasNoFuture() );

        UInt32 seenActive = 0, seenInactive = 0, changes = 0;
        bool last = false;
        UInt32 waited = 0;
        while ( consumer.calls.Load() < 8 * 100 + 50 && waited < 20000 )
        {
            const bool now = engine.IsCaptureVoiceActive();
            ( now ? seenActive : seenInactive ) += 1;
            changes += ( now != last ) ? 1 : 0;
            last = now;
            MT::PrecisionDelay( 1 );
            ++waited;
        }
        ASSERT_TRUE( engine.StopDevice() );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceActivity: engine on a capture device thread: another thread saw the voice active " << seenActive << " times, inactive " << seenInactive << " times, " << changes << " changes; " << consumer.activeBlocks.Load() << " of " << consumer.calls.Load() << " blocks carried it" );
        ASSERT_TRUE( seenActive > 0 && seenInactive > 0 && changes >= 2 );
        ASSERT_TRUE( consumer.activeBlocks.Load() > 50 && consumer.activeBlocks.Load() < consumer.calls.Load() );
        engine.Shutdown();
        VoiceTestDriverSignal() = GUCEF_NULL;
        drivers.Unregister( "voicetest" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // The capture callback is a real time context in a RealtimeSanitizer build: the detector is part of it, and so is
    // the call of the detector itself, which is marked as such
    GUCEF_TESTFW_TESTCASE( "AudioVoiceActivity: The detector and the capture path with it do nothing a real time thread must not do (RealtimeSanitizer build)" )
    #ifndef GUCEF_AUDIO_TEST_VOICEACTIVITY_RTSAN_BUILD
    GUCEF_TESTFW_SKIP( "not a RealtimeSanitizer build (-fsanitize=realtime)" );
    #endif
    GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_RTSAN_CLEAN" ) == "0", "switched off with GUCEF_TEST_AUDIO_RTSAN_CLEAN=0" );
    try
    {
        Sig::SSpeechSpec spec;
        spec.sampleRate = 48000;
        spec.seconds = 5.0;
        spec.seed = 48;
        Sig::SSpeech speech;
        Sig::MakeSpeech( spec, speech );
        const TSamples mixed = Sig::MixAtSnr( speech, Sig::MakeWhiteNoise( speech.samples.size(), 1.0, 133 ), 10.0 );
        SEngineRun run;
        ASSERT_TRUE( RunEngineCapture( mixed, 48000, 16000, 256, GUCEF_NULL, 1.0, run ) );
        ASSERT_TRUE( run.recorded.size() > 16000 * 5 );

        AUDIO::CVoiceActivityDetector detector;
        ASSERT_TRUE( detector.Prepare( 16000 ) );
        const TSamples& audio = run.recorded;
        UInt64 position = 0;
        while ( position + 333 <= audio.size() )
        {
            detector.Process( &audio[ (size_t) position ], 333, position );
            position += 333;
        }
        ASSERT_TRUE( detector.GetAnalyzedFrameCount() > 400 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioVoiceActivityTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioVoiceActivity TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.VoiceActivity" );

    PerformVoiceSignalTests();
    PerformVoiceActivitySettingsTests();
    PerformVoiceDetectorDetectionTests();
    PerformVoiceDetectorTimingTests();
    PerformVoiceDetectorStreamTests();
    PerformVoiceActivityEngineTests();
}

/*-------------------------------------------------------------------------*/
