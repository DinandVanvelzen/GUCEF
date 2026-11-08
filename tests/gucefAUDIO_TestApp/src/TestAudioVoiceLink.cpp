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
#include <algorithm>

#ifndef GUCEFAUDIO_H
#include "gucefAUDIO.h"
#define GUCEFAUDIO_H
#endif /* GUCEFAUDIO_H ? */

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
#include "TestAudioIntercomScenario.h"
#include "TestAudioVoiceLink.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace VoiceLinkTest;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

#ifdef GUCEF_AUDIO_VOICETEST_RTSAN_BUILD
const bool IsRtsanBuild = true;
#else
const bool IsRtsanBuild = false;
#endif

const UInt32 Rate = 48000;
const UInt32 Period = 240;                  /**< 5 ms: the low latency profile */
const UInt32 ClickPeriod = 24000;           /**< a click every 500 ms */
const UInt32 ClickOffset = 4800;
const UInt32 StreamIdA = 0x0A0A0A0A;
const UInt32 StreamIdC = 0x0C0C0C0C;

typedef GUCEF::vector< Float32 > TSamples;

bool
LoadOpus( void )
{
    return VoiceCodecTest::LoadVoiceCodecPlugin( "audiovoicecodecOPUS", "audiovoicecodecopus", "opus" );
}

AUDIO::CVoiceCodecConfig
PcmCodec( void )
{
    return VoiceStreamTest::PcmConfig( Rate, 10000 );
}

AUDIO::CVoiceCodecConfig
OpusCodec( AUDIO::TVoiceCodecProfile profile )
{
    AUDIO::CVoiceCodecConfig codec = VoiceCodecTest::MakeConfig( "opus", Rate, 10000 );
    codec.SetProfileDefaults( profile );
    return codec;
}

AUDIO::CVoiceSenderConfig
SenderConfig( const AUDIO::CVoiceCodecConfig& codec, UInt32 streamId, AUDIO::TVoiceTransmitMode mode = AUDIO::VOICETRANSMITMODE_CONTINUOUS, UInt32 redundancy = 0 )
{
    AUDIO::CVoiceSenderConfig config;
    config.SetCodecConfig( codec );
    config.SetStreamId( streamId );
    config.SetFirstSequenceNumber( 100 );
    config.SetFirstTimestamp( 0 );
    config.SetTransmitMode( mode );
    config.SetRedundancy( redundancy );
    config.SetOutboundQueueFrames( 64 );
    return config;
}

AUDIO::CVoiceReceiverConfig
ReceiverConfig( const AUDIO::CVoiceCodecConfig& codec, AUDIO::TVoicePlayoutProfile profile )
{
    AUDIO::CVoiceReceiverConfig config;
    config.SetCodecConfig( codec );
    config.SetPlayoutProfile( profile );
    config.SetInboundQueueFrames( 64 );
    return config;
}

/*-------------------------------------------------------------------------*/

/** One click found in the output of a unit: when it was recorded and how long it took to the speaker */
struct SClick
{
    Float64 timeInS;
    Float64 latencyMs;
};
typedef GUCEF::vector< SClick > TClicks;

/**
 *  Finds the clicks of a CPcmClickSignal recorded by 'source' in the output of 'sink' while the scenario runs (the
 *  output ring of a unit holds the last seconds) and measures the time from the moment the microphone took the click
 *  to the moment the speaker played it: the output block made at the end of a step plays during the next period.
 *  The search is a normalised cross-correlation with the click pattern, over the whole range at first, then around
 *  the latency found before
 */
class CClickLatencyMeter : public ITestScenarioObserver
{
    public:

    CClickLatencyMeter( const CPcmClickSignal& signal, const CTestIntercomUnit& source, const CTestIntercomUnit& sink, Float64 maxLatencyMs = 400.0 )
        : clicks(), misses( 0 ), m_signal( signal ), m_source( source ), m_sink( sink ), m_maxLatencyMs( maxLatencyMs ), m_next( 0 ), m_hasPrevious( false ), m_previousOffset( 0.0 ), m_clickEnergy( 0.0 )
    {
        for ( UInt32 i=0; i<CPcmClickSignal::ClickLength; ++i )
            m_clickEnergy += (Float64) signal.GetClick()[ i ] * (Float64) signal.GetClick()[ i ];
    }

    virtual void OnStep( CTestIntercomScenario& scenario ) GUCEF_VIRTUAL_OVERRIDE
    {
        const Float64 rate = (Float64) scenario.GetSampleRate();
        const Float64 period = (Float64) scenario.GetPeriod();
        for ( ;; )
        {
            const UInt64 n = (UInt64) m_signal.GetClickOffset() + m_next * (UInt64) m_signal.GetClickPeriod();
            if ( n + CPcmClickSignal::ClickLength > m_source.GetCapturePosition() )
                return;
            const Float64 inputTime = (Float64) n / ( rate * m_source.LocalScale() );
            // the output index whose sound leaves the speaker at the input time
            const Float64 base = inputTime * rate * m_sink.LocalScale() - period;
            Float64 low = base;
            Float64 high = base + m_maxLatencyMs * rate / 1000.0;
            if ( m_hasPrevious )
            {
                low = base + m_previousOffset - 0.025 * rate;
                high = base + m_previousOffset + 0.025 * rate;
            }
            if ( low < 0.0 )
                low = 0.0;
            if ( high + (Float64) CPcmClickSignal::ClickLength >= (Float64) m_sink.GetOutputPosition() )
                return;
            ++m_next;

            Float64 bestScore = -1.0;
            UInt64 best = 0;
            for ( UInt64 m=(UInt64) low; m<=(UInt64) high; ++m )
            {
                Float64 dot = 0.0, energy = 0.0;
                for ( UInt32 i=0; i<CPcmClickSignal::ClickLength; ++i )
                {
                    const Float64 v = m_sink.OutputAt( m + i );
                    dot += v * (Float64) m_signal.GetClick()[ i ];
                    energy += v * v;
                }
                const Float64 score = energy > 0.0 ? dot / sqrt( energy * m_clickEnergy ) : 0.0;
                if ( score > bestScore )
                {
                    bestScore = score;
                    best = m;
                }
            }
            if ( bestScore < 0.7 )
            {
                ++misses;
                continue;
            }
            SClick click;
            click.timeInS = inputTime;
            click.latencyMs = ( ( (Float64) best + period ) / ( rate * m_sink.LocalScale() ) - inputTime ) * 1000.0;
            clicks.push_back( click );
            m_previousOffset = (Float64) best - base;
            m_hasPrevious = true;
        }
    }

    TClicks clicks;
    UInt32 misses;

    private:

    const CPcmClickSignal& m_signal;
    const CTestIntercomUnit& m_source;
    const CTestIntercomUnit& m_sink;
    Float64 m_maxLatencyMs;
    UInt64 m_next;
    bool m_hasPrevious;
    Float64 m_previousOffset;
    Float64 m_clickEnergy;
};

/*-------------------------------------------------------------------------*/

/** Mean, extremes and a percentile of the latencies of the clicks from 'fromS' on */
struct SLatencySummary
{
    UInt32 count;
    Float64 mean;
    Float64 minimum;
    Float64 maximum;
    Float64 p95;

    SLatencySummary( void ) : count( 0 ), mean( 0.0 ), minimum( 0.0 ), maximum( 0.0 ), p95( 0.0 ) {}
};

SLatencySummary
Summarize( const TClicks& clicks, Float64 fromS, Float64 toS = 1.0e12 )
{
    SLatencySummary summary;
    GUCEF::vector< Float64 > values;
    for ( size_t i=0; i<clicks.size(); ++i )
        if ( clicks[ i ].timeInS >= fromS && clicks[ i ].timeInS < toS )
            values.push_back( clicks[ i ].latencyMs );
    if ( values.empty() )
        return summary;
    std::sort( values.begin(), values.end() );
    summary.count = (UInt32) values.size();
    Float64 sum = 0.0;
    for ( size_t i=0; i<values.size(); ++i )
        sum += values[ i ];
    summary.mean = sum / (Float64) values.size();
    summary.minimum = values.front();
    summary.maximum = values.back();
    summary.p95 = values[ ( values.size() * 95 ) / 100 < values.size() ? ( values.size() * 95 ) / 100 : values.size() - 1 ];
    return summary;
}

/*-------------------------------------------------------------------------*/

/** Watches a receiver: the output position at which it first used the resampler, and the stats once per simulated second */
class CReceiverWatch : public ITestScenarioObserver
{
    public:

    CReceiverWatch( AUDIO::CVoiceReceiver& receiver, const CTestIntercomUnit& sink )
        : switched( false ), switchedAt( 0 ), seconds(), m_receiver( receiver ), m_sink( sink ) {}

    virtual void OnStep( CTestIntercomScenario& scenario ) GUCEF_VIRTUAL_OVERRIDE
    {
        const bool fullSecond = 0 == ( scenario.GetStepCount() * (UInt64) scenario.GetPeriod() ) % (UInt64) scenario.GetSampleRate();
        if ( switched && !fullSecond )
            return;
        AUDIO::CVoiceReceiverStats stats;
        m_receiver.GetStats( stats );
        if ( !switched && stats.resampling )
        {
            switched = true;
            switchedAt = m_sink.GetOutputPosition() - scenario.GetPeriod();
        }
        if ( fullSecond )
            seconds.push_back( stats );
    }

    bool switched;
    UInt64 switchedAt;
    GUCEF::vector< AUDIO::CVoiceReceiverStats > seconds;

    private:

    AUDIO::CVoiceReceiver& m_receiver;
    const CTestIntercomUnit& m_sink;
};

/*-------------------------------------------------------------------------*/

/** A call from unit A to unit B over one link, as most scenarios use it */
struct SCallConfig
{
    AUDIO::CVoiceCodecConfig codec;
    SLinkModel link;
    AUDIO::TVoicePlayoutProfile profile;
    Float64 driftPpmA;
    Float64 seconds;
    UInt32 redundancy;
    AUDIO::TVoiceTransmitMode mode;
    const CTestSignal* signal;          /**< A's microphone; null: the click signal */
    bool keepOutput;
    Float64 latePercentage;             /**< < 0: the profile's */
    bool fixedMargin;                   /**< the minimum margin is the initial one */
    Float64 initialMarginInMs;          /**< < 0: the profile's */
    UInt32 sampleRate;                  /**< of both units: devices, mix, capture processing (the codec's rate must be the same) */
    UInt32 period;                      /**< device period of both units */

    SCallConfig( void )
        : codec( PcmCodec() ), link( SLinkModel::Wired() ), profile( AUDIO::VOICEPLAYOUTPROFILE_WIRED ), driftPpmA( 0.0 ), seconds( 10.0 ), redundancy( 0 )
        , mode( AUDIO::VOICETRANSMITMODE_CONTINUOUS ), signal( GUCEF_NULL ), keepOutput( true ), latePercentage( -1.0 ), fixedMargin( false )
        , initialMarginInMs( -1.0 ), sampleRate( Rate ), period( Period )
    {}
};

struct SCallResult
{
    bool ok;
    TClicks clicks;
    UInt32 clickMisses;
    AUDIO::CVoiceReceiverStats stats;
    AUDIO::CVoiceSenderStats senderStats;
    SLinkStats linkStats;
    TSamples output;
    bool switched;
    UInt64 switchedAt;
    GUCEF::vector< AUDIO::CVoiceReceiverStats > seconds;
    Float64 wallInS;

    SCallResult( void ) : ok( false ), clicks(), clickMisses( 0 ), stats(), senderStats(), linkStats(), output(), switched( false ), switchedAt( 0 ), seconds(), wallInS( 0.0 ) {}
};

bool
RunCall( const SCallConfig& config, SCallResult& result )
{
    const UInt64 startTicks = MT::PrecisionTickCount();
    CPcmClickSignal clickSignal( config.sampleRate, config.sampleRate / 2, config.sampleRate / 10 );
    const CTestSignal* signal = GUCEF_NULL != config.signal ? config.signal : &clickSignal;

    CTestIntercomScenario scenario( config.period, config.sampleRate );
    SUnitConfig unitA;
    unitA.driftPpm = config.driftPpmA;
    unitA.keepOutput = false;
    SUnitConfig unitB;
    unitB.capture = false;
    unitB.keepOutput = config.keepOutput;
    CTestIntercomUnit* a = scenario.AddUnit( unitA );
    CTestIntercomUnit* b = scenario.AddUnit( unitB );
    if ( GUCEF_NULL == a || GUCEF_NULL == b )
        return false;
    a->SetSignal( signal );
    if ( !a->AttachSender( SenderConfig( config.codec, StreamIdA, config.mode, config.redundancy ) ) )
        return false;
    AUDIO::CVoiceReceiverConfig receiverConfig = ReceiverConfig( config.codec, config.profile );
    if ( config.latePercentage >= 0.0 )
        receiverConfig.SetLatePercentage( config.latePercentage );
    if ( config.initialMarginInMs >= 0.0 )
    {
        receiverConfig.SetMaxMarginInMs( config.initialMarginInMs > receiverConfig.GetMaxMarginInMs() ? config.initialMarginInMs : receiverConfig.GetMaxMarginInMs() );
        receiverConfig.SetInitialMarginInMs( config.initialMarginInMs );
    }
    if ( config.fixedMargin )
        receiverConfig.SetMinMarginInMs( receiverConfig.GetInitialMarginInMs() );
    AUDIO::CVoiceReceiver* receiver = b->AddReceiver( receiverConfig );
    if ( GUCEF_NULL == receiver )
        return false;
    CTestSimulatedVoiceLink* link = scenario.Connect( a, b, receiver, config.link );

    CClickLatencyMeter meter( clickSignal, *a, *b );
    CReceiverWatch watch( *receiver, *b );
    if ( signal == &clickSignal )
        scenario.AddObserver( &meter );
    scenario.AddObserver( &watch );
    scenario.Run( config.seconds );

    receiver->GetStats( result.stats );
    a->sender->GetStats( result.senderStats );
    result.linkStats = link->GetStats();
    result.clicks = meter.clicks;
    result.clickMisses = meter.misses;
    result.output = b->output;
    result.switched = watch.switched;
    result.switchedAt = watch.switchedAt;
    result.seconds = watch.seconds;
    result.wallInS = (Float64) ( MT::PrecisionTickCount() - startTicks ) / (Float64) MT::PrecisionTimerResolution();
    result.ok = true;
    return true;
}

/*-------------------------------------------------------------------------*/

/** What a float device keeps of a sample: full scale at most */
Float32
Clipped( Float32 value )
{
    return value > 1.0f ? 1.0f : ( value < -1.0f ? -1.0f : value );
}

/** RMS in dB of a range */
Float64
RangeRmsDb( const TSamples& x, size_t from, size_t to )
{
    Float64 sum = 0.0;
    size_t count = 0;
    for ( size_t n=from; n<to && n<x.size(); ++n, ++count )
        sum += (Float64) x[ n ] * (Float64) x[ n ];
    return count > 0 && sum > 0.0 ? 10.0 * log10( sum / (Float64) count ) : -200.0;
}

/** RMS in dB of a range without its mean */
Float64
AcRmsDb( const TSamples& x, size_t from, size_t to )
{
    Float64 sum = 0.0, sumSquares = 0.0;
    size_t count = 0;
    for ( size_t n=from; n<to && n<x.size(); ++n, ++count )
    {
        sum += (Float64) x[ n ];
        sumSquares += (Float64) x[ n ] * (Float64) x[ n ];
    }
    if ( 0 == count )
        return -200.0;
    const Float64 mean = sum / (Float64) count;
    const Float64 variance = sumSquares / (Float64) count - mean * mean;
    return variance > 0.0 ? 10.0 * log10( variance ) : -200.0;
}

/** The output from 'delay' on compared with the signal from its start: how many samples match exactly, up to 'limit' */
size_t
MatchingSamples( const TSamples& output, size_t delay, const CTestSignal& signal, size_t limit )
{
    size_t n = 0;
    while ( delay + n < output.size() && n < limit && output[ delay + n ] == signal.Sample( n ) )
        ++n;
    return n;
}

/** Normalised correlation of the output from 'delay' on with the signal, over 'count' samples from 'from' (of the signal) */
Float64
Correlation( const TSamples& output, size_t delay, const CTestSignal& signal, size_t from, size_t count )
{
    Float64 xy = 0.0, xx = 0.0, yy = 0.0;
    for ( size_t n=from; n<from+count && delay+n<output.size(); ++n )
    {
        const Float64 x = signal.Sample( n );
        const Float64 y = output[ delay + n ];
        xy += x * y;
        xx += x * x;
        yy += y * y;
    }
    return xx > 0.0 && yy > 0.0 ? xy / sqrt( xx * yy ) : 0.0;
}

/**
 *  The output position from which 'count' samples equal 'scale' times the signal from sample 'n0' on, searched from
 *  'from' to 'to'; the size of the output when there is none
 */
size_t
FindLag( const TSamples& output, const CTestSignal& signal, UInt64 n0, size_t count, size_t from, size_t to, Float32 scale = 1.0f )
{
    for ( size_t m=from; m<to && m + count <= output.size(); ++m )
    {
        size_t i = 0;
        while ( i < count && output[ m + i ] == scale * signal.Sample( n0 + i ) )
            ++i;
        if ( i == count )
            return m;
    }
    return output.size();
}

/** The output position where 'count' samples of the signal from 'n0' on correlate best, searched from 'from' to 'to' */
size_t
FindLagByCorrelation( const TSamples& output, const CTestSignal& signal, UInt64 n0, size_t count, size_t from, size_t to, Float64& score )
{
    score = -1.0;
    size_t best = output.size();
    for ( size_t m=from; m<to && m + count <= output.size(); ++m )
    {
        Float64 xy = 0.0, xx = 0.0, yy = 0.0;
        for ( size_t i=0; i<count; ++i )
        {
            const Float64 x = signal.Sample( n0 + i );
            const Float64 y = output[ m + i ];
            xy += x * y;
            xx += x * x;
            yy += y * y;
        }
        const Float64 c = xx > 0.0 && yy > 0.0 ? xy / sqrt( xx * yy ) : 0.0;
        if ( c > score )
        {
            score = c;
            best = m;
        }
    }
    return best;
}

/** The output position of the first sound */
size_t
FirstSound( const TSamples& output )
{
    for ( size_t n=0; n<output.size(); ++n )
        if ( 0.0f != output[ n ] )
            return n;
    return output.size();
}

/*-------------------------------------------------------------------------*/

void
PerformLinkModelTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceLink: The simulated link carries RTP packets with the delay, loss, reordering, duplication and queue of its model" )
    try
    {
        AUDIO::CVoiceFrame frame;
        UInt8 payload[ 960 ];
        for ( UInt32 i=0; i<sizeof( payload ); ++i )
            payload[ i ] = (UInt8) ( i * 7 );
        ASSERT_TRUE( frame.SetPayload( payload, sizeof( payload ) ) );
        frame.SetStreamId( 77 );
        frame.SetPayloadType( 112 );

        // Wi-Fi delay: 3 ms plus a Pareto delay of median 2 ms with 1 % above 30 ms; 1 % loss
        {
            CTestSimulatedVoiceLink link( SLinkModel::WiFi( 4711 ) );
            const UInt32 count = 200000;
            GUCEF::vector< Float64 > delays;
            for ( UInt32 k=0; k<count; ++k )
            {
                frame.SetSequenceNumber( (UInt16) k );
                frame.SetTimestamp( k * 480 );
                link.Send( frame, (Float64) k * 10000.0 );
                AUDIO::CVoiceFrame out;
                Float64 arrival = 0.0, send = 0.0;
                while ( link.PopArrived( (Float64) k * 10000.0 + 5000.0, out, arrival, send ) )
                    delays.push_back( arrival - send );
            }
            AUDIO::CVoiceFrame out;
            Float64 arrival = 0.0, send = 0.0;
            while ( link.PopArrived( 1.0e15, out, arrival, send ) )
            {
                delays.push_back( arrival - send );
                ASSERT_TRUE( out.GetPayloadSize() == sizeof( payload ) && 0 == memcmp( out.GetPayload(), payload, sizeof( payload ) ) && 77 == out.GetStreamId() );
            }
            std::sort( delays.begin(), delays.end() );
            const Float64 median = delays[ delays.size() / 2 ];
            UInt32 above = 0;
            for ( size_t i=0; i<delays.size(); ++i )
                if ( delays[ i ] > 33000.0 )
                    ++above;
            const SLinkStats& stats = link.GetStats();
            const Float64 lossShare = (Float64) stats.lost / (Float64) stats.sent;
            const Float64 aboveShare = (Float64) above / (Float64) delays.size();
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceLink: Wi-Fi model: median delay " << median / 1000.0 << " ms, " << 100.0 * aboveShare << " % above 33 ms, loss " << 100.0 * lossShare <<
                       " %, out of order " << stats.deliveredOutOfOrder );
            ASSERT_TRUE( fabs( median - 5000.0 ) < 100.0 );
            ASSERT_TRUE( aboveShare > 0.009 && aboveShare < 0.011 );
            ASSERT_TRUE( lossShare > 0.009 && lossShare < 0.011 );
            ASSERT_TRUE( stats.delivered + stats.lost == stats.sent );
        }

        // Gilbert-Elliott: p 0.01, r 1/3: loss 0.01 / ( 0.01 + 1/3 ) = 2.9 %, bursts of 3 on average; reordering, duplicates; a queue
        {
            SLinkModel model;
            model.geGoodToBad = 0.01;
            model.geBadToGood = 1.0 / 3.0;
            model.reorderProbability = 0.05;
            model.reorderMaxSteps = 2;
            model.duplicateProbability = 0.01;
            model.seed = 99;
            CTestSimulatedVoiceLink link( model );
            const UInt32 count = 100000;
            GUCEF::vector< UInt8 > received( count, 0 );
            UInt32 bursts = 0, burstFrames = 0, run = 0;
            UInt32 copies = 0;
            for ( UInt32 k=0; k<count; ++k )
            {
                frame.SetSequenceNumber( (UInt16) k );
                frame.SetTimestamp( k );
                link.Send( frame, (Float64) k * 10000.0 );
                AUDIO::CVoiceFrame out;
                Float64 arrival = 0.0, send = 0.0;
                while ( link.PopArrived( (Float64) k * 10000.0, out, arrival, send ) )
                {
                    const UInt32 index = out.GetTimestamp();
                    if ( received[ index ] )
                        ++copies;
                    received[ index ] = 1;
                }
            }
            AUDIO::CVoiceFrame out;
            Float64 arrival = 0.0, send = 0.0;
            while ( link.PopArrived( 1.0e15, out, arrival, send ) )
            {
                if ( received[ out.GetTimestamp() ] )
                    ++copies;
                received[ out.GetTimestamp() ] = 1;
            }
            UInt32 lost = 0;
            for ( UInt32 k=0; k<count; ++k )
            {
                if ( !received[ k ] )
                {
                    ++lost;
                    ++run;
                }
                else
                if ( run > 0 )
                {
                    ++bursts;
                    burstFrames += run;
                    run = 0;
                }
            }
            const SLinkStats& stats = link.GetStats();
            const Float64 meanBurst = bursts > 0 ? (Float64) burstFrames / (Float64) bursts : 0.0;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceLink: Gilbert-Elliott: loss " << 100.0 * (Float64) lost / (Float64) count << " %, mean burst " << meanBurst << " frames, reordered " << stats.reordered <<
                       " (" << stats.deliveredOutOfOrder << " delivered out of order), copies " << copies );
            ASSERT_TRUE( lost == stats.lost );
            ASSERT_TRUE( fabs( (Float64) lost / (Float64) count - 0.01 / ( 0.01 + 1.0 / 3.0 ) ) < 0.004 );
            ASSERT_TRUE( meanBurst > 2.6 && meanBurst < 3.4 );
            ASSERT_TRUE( copies == stats.duplicated && copies > 700 && copies < 1300 );
            // a held packet is overtaken unless the ones after it were lost or held as well
            ASSERT_TRUE( stats.reordered > 4000 && stats.reordered < 6000 && (Float64) stats.deliveredOutOfOrder >= 0.9 * (Float64) stats.reordered );
        }

        // bandwidth and queue: 1500 byte packets every 5 ms (2.4 Mbit/s) into 1 Mbit/s with a queue of 8
        {
            SLinkModel model;
            model.baseDelayUs = 1000.0;
            model.bandwidthBitsPerSecond = 1000000.0;
            model.queueLimitPackets = 8;
            CTestSimulatedVoiceLink link( model );
            AUDIO::CVoiceFrame big;
            UInt8 bytes[ 1440 ];
            memset( bytes, 0x55, sizeof( bytes ) );
            ASSERT_TRUE( big.SetPayload( bytes, sizeof( bytes ) ) );
            big.SetPayloadType( 112 );
            Float64 lastArrival = 0.0;
            for ( UInt32 k=0; k<1000; ++k )
            {
                big.SetSequenceNumber( (UInt16) k );
                link.Send( big, (Float64) k * 5000.0 );
            }
            AUDIO::CVoiceFrame out;
            Float64 arrival = 0.0, send = 0.0;
            UInt64 delivered = 0;
            while ( link.PopArrived( 1.0e15, out, arrival, send ) )
            {
                ++delivered;
                lastArrival = arrival;
            }
            const SLinkStats& stats = link.GetStats();
            // the link carries 1 Mbit/s: in 5 s about 5e6 / ( 1452 * 8 ) = 430 packets
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceLink: queue: " << delivered << " delivered, " << stats.queueDropped << " dropped at the queue, last arrival " << lastArrival / 1000.0 << " ms, max delay " << stats.maxDelayUs / 1000.0 << " ms" );
            ASSERT_TRUE( delivered + stats.queueDropped == 1000 );
            ASSERT_TRUE( delivered > 400 && delivered < 460 );
            ASSERT_TRUE( stats.maxDelayUs <= 1000.0 + 9.0 * 1452.0 * 8.0 + 1.0 );
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
PerformLatencyTests( void )
{GUCEF_TRACE;

    const bool opus = LoadOpus();

    GUCEF_TESTFW_TESTCASE( "AudioVoiceLink: Wired (0.3 ms + 0.2 ms jitter): end to end at most 40 ms with PCM and Opus low delay, PCM bit for bit until the resampler, Opus level within 0.5 dB" )
    try
    {
        for ( UInt32 c=0; c<( opus ? 3 : 2 ); ++c )
        {
            SCallConfig config;
            config.seconds = 12.0;
            if ( 1 == c )
            {
                // a constant delay: the PCM output stays bit for bit for the whole run
                config.link.jitter = SLinkModel::JITTER_NONE;
                config.fixedMargin = true;
            }
            if ( 2 == c )
                config.codec = OpusCodec( AUDIO::VOICECODECPROFILE_LOWDELAY );
            SCallResult result;
            ASSERT_TRUE( RunCall( config, result ) );
            const SLatencySummary latency = Summarize( result.clicks, 1.0 );
            CPcmClickSignal signal( Rate, ClickPeriod, ClickOffset );
            const size_t delay = FirstSound( result.output );
            size_t matching = 0;
            Float64 levelError = 0.0;
            if ( 2 != c )
                matching = MatchingSamples( result.output, delay, signal, result.output.size() );
            else
            {
                // the level of the tones between the clicks (without the offset, which Opus does not keep)
                const size_t lag = (size_t) ( latency.mean / 1000.0 * Rate + 0.5 ) - Period;
                const size_t from = 2 * Rate + 2000;    // between the clicks at 2.1 s and 2.6 s
                levelError = AcRmsDb( result.output, lag + from, lag + from + 19000 ) - AcRmsDb( VoiceStreamTest::MakePcmSignal( 19000, Rate, from ), 0, 19000 );
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceLink: wired, " << ( 2 == c ? "opus" : "pcm" ) << ( 1 == c ? ", constant delay" : "" ) << ": end to end " << latency.mean << " ms (" << latency.minimum << " to " << latency.maximum <<
                       ", " << latency.count << " clicks, " << result.clickMisses << " missed), margin " << result.stats.marginInMs << " ms, concealed " << result.stats.framesConcealed << ", late " << result.stats.lateFrames <<
                       ", resampler from output " << ( result.switched ? (Int64) result.switchedAt : -1 ) << ", " << matching << " samples bit for bit from " << delay << ", level error " << levelError << " dB, " << result.wallInS << " s" );
            ASSERT_TRUE( latency.count >= 20 && 0 == result.clickMisses );
            ASSERT_TRUE( latency.maximum <= 40.0 );
            ASSERT_TRUE( 0 == result.stats.framesConcealed && 0 == result.stats.lateFrames );
            if ( 0 == c )
                ASSERT_TRUE( delay + matching >= ( result.switched ? (size_t) result.switchedAt : result.output.size() ) );
            if ( 1 == c )
                ASSERT_TRUE( !result.switched && delay + matching == result.output.size() );
            if ( 2 == c )
                ASSERT_TRUE( fabs( levelError ) <= 0.5 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceLink: Wi-Fi (3 ms + Pareto, 1 % loss): end to end at most 80 ms once the margin adapted, late frames within 1 %; the margin rule before and after" )
    try
    {
        // 0: PCM with the margin rule of the wireless profile; 1: Opus low delay with it; 2: PCM with the rule of step 12
        // (late percentage 0: the margin grows by the lateness of every late frame), for comparison
        for ( UInt32 c=0; c<3; ++c )
        {
            if ( 1 == c && !opus )
                continue;
            SCallConfig config;
            config.link = SLinkModel::WiFi();
            config.profile = AUDIO::VOICEPLAYOUTPROFILE_WIRELESS;
            config.seconds = 120.0;
            config.keepOutput = false;
            if ( 1 == c )
                config.codec = OpusCodec( AUDIO::VOICECODECPROFILE_LOWDELAY );
            if ( 2 == c )
                config.latePercentage = 0.0;
            SCallResult result;
            ASSERT_TRUE( RunCall( config, result ) );
            const SLatencySummary latency = Summarize( result.clicks, 10.0 );
            const AUDIO::CVoiceReceiverStats& early = result.seconds.size() > 10 ? result.seconds[ 9 ] : result.stats;
            const Float64 frames = (Float64) ( result.stats.framesExpected - early.framesExpected );
            const Float64 lost = (Float64) ( result.stats.cumulativeLost - early.cumulativeLost );
            const Float64 late = (Float64) ( result.stats.lateFrames - early.lateFrames + result.stats.framesStretched - early.framesStretched );
            // what stood in for missing frames: concealment, and for Opus the decoder's own (DecodeFec() on a packet
            // without forward error correction data conceals and counts as recovered)
            const Float64 concealed = (Float64) ( result.stats.framesConcealed - early.framesConcealed + result.stats.framesRecovered - early.framesRecovered );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceLink: Wi-Fi, " << ( 1 == c ? "opus" : "pcm" ) << ( 2 == c ? ", rule of step 12 (late 0 %)" : "" ) << ": end to end mean " << latency.mean << " ms, 95 % " << latency.p95 <<
                       " ms, " << latency.minimum << " to " << latency.maximum << " ms (" << latency.count << " clicks, " << result.clickMisses << " missed); after 10 s: lost " << 100.0 * lost / frames << " %, late " <<
                       100.0 * late / frames << " %, concealed " << 100.0 * concealed / frames << " %; margin " << result.stats.marginInMs << " ms, " << result.wallInS << " s" );
            ASSERT_TRUE( latency.count > 200 );
            if ( 2 != c )
            {
                ASSERT_TRUE( latency.mean <= 80.0 );
                ASSERT_TRUE( late <= 0.01 * frames );
                ASSERT_TRUE( concealed >= lost && concealed <= 1.1 * ( lost + late ) + 10.0 );
            }
            else
            {
                ASSERT_TRUE( latency.mean > 150.0 );    // what the rule of step 12 gives: the margin at its maximum
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

/**
 *  Correlation of the output with the signal, aligned per click: the stretch after every click is compared at the
 *  latency of that click (the playout point moves with the controller, so one lag does not fit a whole run)
 */
Float64
CorrelationByClicks( const TSamples& output, const CPcmClickSignal& signal, const TClicks& clicks, Float64 fromS )
{
    Float64 xy = 0.0, xx = 0.0, yy = 0.0;
    for ( size_t k=0; k<clicks.size(); ++k )
    {
        if ( clicks[ k ].timeInS < fromS )
            continue;
        const UInt64 n0 = (UInt64) ( clicks[ k ].timeInS * Rate + 0.5 );
        const Int64 lag = (Int64) floor( clicks[ k ].latencyMs / 1000.0 * Rate + 0.5 ) - (Int64) Period;
        // from 10 ms after the click to 10 ms before the next
        for ( UInt64 n=n0 + 480; n<n0 + ClickPeriod - 480; ++n )
        {
            const Int64 m = (Int64) n + lag;
            if ( m < 0 || (UInt64) m >= output.size() )
                continue;
            const Float64 x = signal.Sample( n );
            const Float64 y = output[ (size_t) m ];
            xy += x * y;
            xx += x * x;
            yy += y * y;
        }
    }
    return xx > 0.0 && yy > 0.0 ? xy / sqrt( xx * yy ) : 0.0;
}

/*-------------------------------------------------------------------------*/

void
PerformLossTests( void )
{GUCEF_TRACE;

    const bool opus = LoadOpus();

    GUCEF_TESTFW_TESTCASE( "AudioVoiceLink: Burst loss (Gilbert-Elliott, 3 %, bursts of 3): concealment, redundancy and Opus forward error correction make up for the lost frames; redundancy recovers more and the output follows the input closer" )
    try
    {
        // 0: PCM in 5 ms frames, concealment; 1: the same with RFC 2198 redundancy (a 5 ms PCM frame and its copy fit a
        // packet); 2: Opus robust (in-band FEC); 3: Opus robust with redundancy
        const char* names[] = { "pcm", "pcm with redundancy", "opus robust", "opus robust with redundancy" };
        Float64 correlations[ 4 ] = { 0.0, 0.0, 0.0, 0.0 };
        UInt64 recovered[ 4 ] = { 0, 0, 0, 0 };
        for ( UInt32 c=0; c<4; ++c )
        {
            if ( c >= 2 && !opus )
                continue;
            SCallConfig config;
            config.link = SLinkModel::Wired( 31 );
            config.link.geGoodToBad = 0.01;
            config.link.geBadToGood = 1.0 / 3.0;
            config.profile = AUDIO::VOICEPLAYOUTPROFILE_WIRELESS;
            config.seconds = 60.0;
            config.codec = c < 2 ? VoiceStreamTest::PcmConfig( Rate, 5000 ) : OpusCodec( AUDIO::VOICECODECPROFILE_ROBUST );
            config.redundancy = ( 1 == c || 3 == c ) ? 1 : 0;
            if ( c < 2 )
            {
                // PCM at a constant delay and a fixed margin longer than the bursts: the playout runs at ratio 1 and never
                // runs dry, so the output is the input bit for bit except where frames were lost, and the correlation tells
                // how well they were made up for
                config.link.jitter = SLinkModel::JITTER_NONE;
                config.fixedMargin = true;
                config.initialMarginInMs = 100.0;
            }
            SCallResult result;
            ASSERT_TRUE( RunCall( config, result ) );
            CPcmClickSignal signal( Rate, ClickPeriod, ClickOffset );
            if ( c < 2 )
            {
                const size_t found = FindLag( result.output, signal, Rate / 4, 480, Rate / 4, Rate / 2 );
                ASSERT_TRUE( found < result.output.size() && !result.switched );
                const size_t lag = found < result.output.size() ? found - Rate / 4 : 0;
                correlations[ c ] = Correlation( result.output, lag, signal, 2 * Rate, result.output.size() - lag - 2 * Rate );
            }
            else
                correlations[ c ] = CorrelationByClicks( result.output, signal, result.clicks, 2.0 );
            recovered[ c ] = result.stats.framesRecovered + result.stats.redundantStored;
            const SLatencySummary latency = Summarize( result.clicks, 2.0 );
            const UInt64 madeUp = result.stats.framesConcealed + result.stats.framesRecovered + result.stats.redundantStored;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceLink: burst loss, " << names[ c ] << ": lost " << result.linkStats.lost << " of " << result.linkStats.sent << " (" << 100.0 * (Float64) result.linkStats.lost / (Float64) result.linkStats.sent <<
                       " %), concealed " << result.stats.framesConcealed << ", recovered by FEC " << result.stats.framesRecovered << ", from redundancy " << result.stats.redundantStored << ", late " << result.stats.lateFrames <<
                       ", talkspurts " << result.stats.talkspurts << ", counted lost " << result.stats.cumulativeLost << ", stretched " << result.stats.framesStretched << ", accelerated " << result.stats.framesAccelerated <<
                       ", Lmin " << result.stats.minimumLevelInMs << " ms, ratio " << result.stats.ratioInPpm << " ppm, correlation with the input " << correlations[ c ] << ", margin " << result.stats.marginInMs << " ms, end to end " << latency.mean << " ms (" << latency.minimum <<
                       " to " << latency.maximum << ")" );
            ASSERT_TRUE( result.linkStats.lost > 80 );
            // every lost frame is made up for, but where a burst outlasts the concealment limit (the talkspurt ends there)
            ASSERT_TRUE( (Float64) madeUp >= 0.75 * (Float64) result.stats.cumulativeLost && madeUp <= (UInt64) result.stats.cumulativeLost + result.stats.lateFrames + 2 );
            if ( 0 == c )
                ASSERT_TRUE( 0 == result.stats.framesRecovered && 0 == result.stats.redundantStored );
            if ( 2 == c )
                ASSERT_TRUE( (Float64) result.stats.framesRecovered >= 0.25 * (Float64) result.stats.cumulativeLost );
            if ( 1 == c || 3 == c )
                ASSERT_TRUE( result.stats.redundantStored > 0 );
        }
        ASSERT_TRUE( recovered[ 1 ] > recovered[ 0 ] && correlations[ 1 ] > correlations[ 0 ] );
        if ( opus )
            ASSERT_TRUE( recovered[ 3 ] > recovered[ 2 ] );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceLink: Reordering of 5 % by up to two frames and 1 % duplicates: once the margin covers them every frame plays in order, nothing is concealed" )
    try
    {
        SCallConfig config;
        config.link = SLinkModel::Wired( 77 );
        config.link.reorderProbability = 0.05;
        config.link.reorderMaxSteps = 2;
        config.link.duplicateProbability = 0.01;
        config.profile = AUDIO::VOICEPLAYOUTPROFILE_WIRELESS;
        config.seconds = 40.0;
        config.keepOutput = false;
        SCallResult result;
        ASSERT_TRUE( RunCall( config, result ) );
        const AUDIO::CVoiceReceiverStats& early = result.seconds[ 9 ];
        const SLatencySummary latency = Summarize( result.clicks, 10.0 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceLink: reordering: link reordered " << result.linkStats.reordered << " (" << result.linkStats.deliveredOutOfOrder << " out of order), duplicated " << result.linkStats.duplicated <<
                   "; receiver reordered " << result.stats.reordered << ", duplicates " << result.stats.duplicates << ", concealed " << result.stats.framesConcealed << " (" << early.framesConcealed << " in the first 10 s), late " <<
                   result.stats.lateFrames << ", stretched " << result.stats.framesStretched << ", margin " << result.stats.marginInMs << " ms, end to end " << latency.mean << " ms (" << latency.minimum << " to " << latency.maximum << ")" );
        ASSERT_TRUE( result.linkStats.reordered > 100 && result.stats.reordered > 0 );
        ASSERT_TRUE( result.linkStats.duplicated > 10 && result.stats.duplicates + result.stats.lateDropped >= result.linkStats.duplicated );
        ASSERT_TRUE( result.stats.framesConcealed == early.framesConcealed && result.stats.lateFrames == early.lateFrames && result.stats.framesStretched == early.framesStretched );
        ASSERT_TRUE( 0 == result.stats.cumulativeLost );
        ASSERT_TRUE( latency.count > 50 && latency.maximum <= 60.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformDriftTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceLink: Clock drift between the units (+-100 to +-300 ppm, 10 simulated minutes at +-200 ppm): no underrun after the start, the latency stays within 3 ms of its value at 30 s" )
    try
    {
        struct SCase { Float64 ppm; Float64 seconds; };
        const SCase cases[] = { { 200.0, 600.0 }, { -200.0, 600.0 }, { 100.0, 120.0 }, { -100.0, 120.0 }, { 300.0, 120.0 }, { -300.0, 120.0 } };
        for ( UInt32 c=0; c<6; ++c )
        {
            SCallConfig config;
            config.driftPpmA = cases[ c ].ppm;
            config.seconds = cases[ c ].seconds;
            config.keepOutput = false;
            SCallResult result;
            ASSERT_TRUE( RunCall( config, result ) );
            const AUDIO::CVoiceReceiverStats& early = result.seconds[ 4 ];
            const SLatencySummary at30 = Summarize( result.clicks, 29.5, 30.5 );
            const SLatencySummary after = Summarize( result.clicks, 30.0 );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceLink: drift " << cases[ c ].ppm << " ppm over " << cases[ c ].seconds << " s: end to end at 30 s " << at30.mean << " ms, after it " << after.minimum << " to " << after.maximum <<
                       " ms; underruns " << result.stats.underruns << " (" << early.underruns << " in the first 5 s), concealed " << result.stats.framesConcealed << ", integral " << result.stats.integralInPpm << " ppm, " << result.wallInS << " s" );
            ASSERT_TRUE( result.stats.underruns == early.underruns && result.stats.framesConcealed == early.framesConcealed );
            ASSERT_TRUE( at30.count > 0 && after.minimum >= at30.mean - 3.0 && after.maximum <= at30.mean + 3.0 );
            ASSERT_TRUE( fabs( result.stats.integralInPpm - cases[ c ].ppm ) < 20.0 );
            ASSERT_TRUE( after.maximum <= 40.0 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceLink: Clock drift of +-0.5 %, the most the playout corrects: the ratio has headroom left, the latency returns to where it was" )
    try
    {
        const Float64 drifts[] = { 5000.0, -5000.0 };
        for ( UInt32 c=0; c<2; ++c )
        {
            SCallConfig config;
            config.driftPpmA = drifts[ c ];
            config.seconds = 300.0;
            config.keepOutput = false;
            SCallResult result;
            ASSERT_TRUE( RunCall( config, result ) );
            const AUDIO::CVoiceReceiverStats& settled = result.seconds[ 119 ];
            const SLatencySummary late = Summarize( result.clicks, 120.0 );
            const SLatencySummary lastMinute = Summarize( result.clicks, 240.0 );
            const SLatencySummary start = Summarize( result.clicks, 0.0, 5.0 );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceLink: drift " << drifts[ c ] / 10000.0 << " %: end to end in the first 5 s " << start.mean << " ms, after 2 minutes " << late.minimum << " to " << late.maximum <<
                       " ms, in the last minute " << lastMinute.minimum << " to " << lastMinute.maximum << " ms; underruns " << result.stats.underruns << " (" << settled.underruns << " in the first 2 minutes), integral " <<
                       result.stats.integralInPpm << " ppm, ratio " << result.stats.ratioInPpm << " ppm, margin " << result.stats.marginInMs << " ms" );
            // a slower sender runs the buffer dry at the start (stretches, which grow the margin); the margin decays again
            // at 1 ms per 10 s and the ratio, with headroom left, follows it down
            ASSERT_TRUE( result.stats.underruns == settled.underruns && result.stats.lateFrames == settled.lateFrames );
            ASSERT_TRUE( fabs( result.stats.integralInPpm - drifts[ c ] ) < 150.0 );
            ASSERT_TRUE( late.count > 100 && late.maximum <= 40.0 );
            ASSERT_TRUE( lastMinute.count > 100 && lastMinute.maximum - lastMinute.minimum <= 2.0 );
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
PerformTalkspurtTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceLink: Talkspurts gated by the real voice activity detector: every talkspurt plays, the pauses are silent, the first syllable is kept" )
    try
    {
        TestVoiceSignals::SSpeechSpec spec;
        spec.sampleRate = Rate;
        spec.seconds = 20.0;
        spec.seed = 14;
        spec.activeRms = 0.1;
        TestVoiceSignals::SSpeech speech;
        TestVoiceSignals::MakeSpeech( spec, speech );
        // 16 bit exact, so PCM passes it bit for bit where the playout runs at ratio 1
        for ( size_t n=0; n<speech.samples.size(); ++n )
            speech.samples[ n ] = (Float32) floor( speech.samples[ n ] * 32768.0f + 0.5f ) / 32768.0f;
        CBufferSignal signal( speech.samples );

        SCallConfig config;
        config.signal = &signal;
        config.mode = AUDIO::VOICETRANSMITMODE_VOICE_ACTIVATED;
        config.seconds = 21.0;
        config.link.jitter = SLinkModel::JITTER_NONE;
        config.fixedMargin = true;
        SCallResult result;
        ASSERT_TRUE( RunCall( config, result ) );

        // the fixed delay of the playout: where the middle of the first talkspurt is found in the output
        const UInt64 probe = speech.talkspurts[ 0 ].start + speech.talkspurts[ 0 ].length / 2;
        Float64 lagScore = 0.0;
        const size_t found = FindLagByCorrelation( result.output, signal, probe, 2400, (size_t) probe, (size_t) probe + Rate / 10, lagScore );
        ASSERT_TRUE( found < result.output.size() && lagScore > 0.99 );
        const size_t fixedLag = found < result.output.size() ? found - (size_t) probe : 0;
        const Float64 latencyMs = (Float64) ( fixedLag + Period ) * 1000.0 / Rate;

        // the first sound of every talkspurt against its start (the burst rises over its first 8 ms), and the pauses
        UInt32 spurtsHeard = 0, pauseSamplesSilent = 0, pauseSamples = 0;
        Float64 worstOnsetMs = -1.0e9, bestOnsetMs = 1.0e9;
        for ( size_t s=0; s<speech.talkspurts.size(); ++s )
        {
            const TestVoiceSignals::SSpan& spurt = speech.talkspurts[ s ];
            const size_t expected = (size_t) spurt.start + fixedLag;
            size_t first = result.output.size();
            for ( size_t m=( expected > 4800 ? expected - 4800 : 0 ); m<expected + 4800 && m<result.output.size(); ++m )
            {
                if ( 0.0f != result.output[ m ] )
                {
                    first = m;
                    break;
                }
            }
            if ( first < result.output.size() )
            {
                ++spurtsHeard;
                const Float64 onsetMs = ( (Float64) first - (Float64) expected ) * 1000.0 / Rate;
                if ( onsetMs > worstOnsetMs )
                    worstOnsetMs = onsetMs;
                if ( onsetMs < bestOnsetMs )
                    bestOnsetMs = onsetMs;
            }
            // the pause after the talkspurt, once the hangovers (200 + 300 ms) and the concealment (60 ms) are over
            const size_t pauseStart = (size_t) spurt.End() + (size_t) ( 0.56 * Rate ) + fixedLag;
            const size_t pauseEnd = s + 1 < speech.talkspurts.size() ? (size_t) speech.talkspurts[ s + 1 ].start + fixedLag - (size_t) ( 0.05 * Rate ) : result.output.size();
            for ( size_t m=pauseStart; m<pauseEnd && m<result.output.size(); ++m )
            {
                ++pauseSamples;
                if ( 0.0f == result.output[ m ] )
                    ++pauseSamplesSilent;
            }
        }
        // the level of the speech that came through, against the input over the same talkspurts
        Float64 inputEnergy = 0.0, outputEnergy = 0.0;
        for ( size_t s=0; s<speech.talkspurts.size(); ++s )
        {
            const size_t lag = fixedLag;
            for ( size_t n=(size_t) speech.talkspurts[ s ].start; n<(size_t) speech.talkspurts[ s ].End() && n + lag < result.output.size(); ++n )
            {
                inputEnergy += (Float64) speech.samples[ n ] * (Float64) speech.samples[ n ];
                outputEnergy += (Float64) result.output[ n + lag ] * (Float64) result.output[ n + lag ];
            }
        }
        const Float64 levelDb = 10.0 * log10( outputEnergy / inputEnergy );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceLink: talkspurts: " << speech.talkspurts.size() << " in the signal, " << result.senderStats.talkspurts << " sent, " << result.stats.talkspurts << " played, " << spurtsHeard <<
                   " heard; end to end " << latencyMs << " ms; first sound " << bestOnsetMs << " to " << worstOnsetMs << " ms after the start of the talkspurt; pauses silent " << 100.0 * (Float64) pauseSamplesSilent / (Float64) pauseSamples << " %; level " << levelDb <<
                   " dB; frames sent " << result.senderStats.framesSent << " of " << result.senderStats.framesCaptured << " (pre-roll " << result.senderStats.preRollFramesSent << ")" );
        ASSERT_TRUE( speech.talkspurts.size() >= 6 );
        ASSERT_TRUE( result.senderStats.talkspurts == speech.talkspurts.size() && result.stats.talkspurts == speech.talkspurts.size() && spurtsHeard == speech.talkspurts.size() );
        // the detector needs 10 to 30 ms, the pre-roll sends the 2 frames before: at most the rise of the burst is lost
        ASSERT_TRUE( worstOnsetMs <= 10.0 );
        ASSERT_TRUE( pauseSamplesSilent == pauseSamples );
        ASSERT_TRUE( fabs( levelDb ) < 0.2 );
        ASSERT_TRUE( result.senderStats.framesSent < result.senderStats.framesCaptured * 3 / 4 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformMultiStreamTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceLink: A push-to-talk announcement to four receivers (two engines with one, one engine with two): chimes first, then the voice, each output as sent" )
    try
    {
        CPcmClickSignal signal( Rate, 0, 0 );
        CTestIntercomScenario scenario( Period, Rate );
        SUnitConfig talkerConfig;
        talkerConfig.keepOutput = false;
        SUnitConfig listenerConfig;
        listenerConfig.capture = false;
        CTestIntercomUnit* talker = scenario.AddUnit( talkerConfig );
        CTestIntercomUnit* b = scenario.AddUnit( listenerConfig );
        CTestIntercomUnit* c = scenario.AddUnit( listenerConfig );
        CTestIntercomUnit* de = scenario.AddUnit( listenerConfig );
        ASSERT_TRUE( GUCEF_NULL != talker && GUCEF_NULL != b && GUCEF_NULL != c && GUCEF_NULL != de );
        if ( GUCEF_NULL == talker || GUCEF_NULL == b || GUCEF_NULL == c || GUCEF_NULL == de )
            return;
        talker->SetSignal( &signal );
        ASSERT_TRUE( talker->AttachSender( SenderConfig( PcmCodec(), StreamIdA, AUDIO::VOICETRANSMITMODE_PUSH_TO_TALK ) ) );
        CTestIntercomUnit* listeners[ 3 ] = { b, c, de };
        AUDIO::CVoiceReceiver* receivers[ 4 ];
        for ( UInt32 r=0; r<4; ++r )
        {
            CTestIntercomUnit* unit = listeners[ r < 2 ? r : 2 ];
            AUDIO::CVoiceReceiverConfig config = ReceiverConfig( PcmCodec(), AUDIO::VOICEPLAYOUTPROFILE_WIRED );
            config.SetMinMarginInMs( config.GetInitialMarginInMs() );
            AUDIO::CAudioBus* announce = unit->engine->FindBus( "announce" );
            if ( GUCEF_NULL == announce )
                announce = unit->engine->CreateBus( "announce" );
            receivers[ r ] = unit->AddReceiver( config, announce );
            ASSERT_TRUE( GUCEF_NULL != receivers[ r ] );
            if ( GUCEF_NULL == receivers[ r ] )
                return;
            SLinkModel model = SLinkModel::Wired();
            model.jitter = SLinkModel::JITTER_NONE;
            scenario.Connect( talker, unit, receivers[ r ], model );
        }

        // the announcement starts: every listener plays the chime (700 ms); the talker sends once its own chime ended
        scenario.Run( 0.2 );
        const UInt64 chimeAt = b->GetOutputPosition();
        for ( UInt32 u=0; u<3; ++u )
        {
            AUDIO::TAudioVoicePtr chime = listeners[ u ]->engine->CreateVoice( AUDIO::CToneSequenceSource::MakeChime( Rate, 1 ) );
            ASSERT_TRUE( !chime.IsNULL() && !listeners[ u ]->engine->Start( chime ).HasNoFuture() );
        }
        scenario.Run( 0.7 );
        talker->sender->SetTransmitting( true );
        const UInt64 keyDown = talker->GetCapturePosition();
        scenario.Run( 2.0 );
        talker->sender->SetTransmitting( false );
        const UInt64 keyUp = talker->GetCapturePosition();
        scenario.Run( 0.5 );

        // B and C: the chime, then the voice bit for bit; D and E on one engine: twice the voice
        const UInt64 chimeEnd = chimeAt + (UInt64) ( 0.7 * Rate );
        const UInt64 firstSample = ( keyDown / 480 ) * 480;     // the voice starts with the frame in which the key went down
        UInt64 voiceStart[ 3 ] = { 0, 0, 0 };
        for ( UInt32 u=0; u<3; ++u )
        {
            const TSamples& out = listeners[ u ]->output;
            const Float64 chimeDb = RangeRmsDb( out, (size_t) chimeAt + 2400, (size_t) chimeAt + 24000 );
            const Float32 scale = 2 == u ? 2.0f : 1.0f;
            // D and E: the mix of two voices, within 1e-6, and clipped at full scale by the device format
            size_t first = out.size();
            for ( size_t m=(size_t) chimeEnd; m<(size_t) chimeEnd + Rate / 5 && first == out.size(); ++m )
            {
                size_t i = 0;
                while ( i < 480 && m + i < out.size() && fabs( out[ m + i ] - Clipped( scale * signal.Sample( firstSample + i ) ) ) <= 1.0e-6 )
                    ++i;
                if ( 480 == i )
                    first = m;
            }
            voiceStart[ u ] = first;
            size_t matching = 0;
            while ( first + matching < out.size() && firstSample + matching < keyUp && fabs( out[ first + matching ] - Clipped( scale * signal.Sample( firstSample + matching ) ) ) <= 1.0e-6 )
                ++matching;
            // between the end of the chime and the voice: silence (the chime decays to nothing)
            const Float64 gapDb = RangeRmsDb( out, (size_t) chimeEnd, first );
            AUDIO::CVoiceReceiverStats stats;
            receivers[ u < 2 ? u : 2 ]->GetStats( stats );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceLink: announcement, listener " << u << ": chime " << chimeDb << " dBFS, voice from output " << first << " (" << ( (Float64) first - (Float64) chimeEnd ) * 1000.0 / Rate <<
                       " ms after the chime ended, " << ( (Float64) first + Period - (Float64) keyDown ) * 1000.0 / Rate << " ms after the key went down), " << matching << " samples as sent of " << keyUp - firstSample <<
                       ", between them " << gapDb << " dBFS, talkspurts " << stats.talkspurts << ", concealed " << stats.framesConcealed );
            ASSERT_TRUE( chimeDb > -20.0 );
            ASSERT_TRUE( first < out.size() && first >= chimeEnd );
            ASSERT_TRUE( matching + 480 >= keyUp - firstSample );
            ASSERT_TRUE( 1 == stats.talkspurts );
        }
        ASSERT_TRUE( voiceStart[ 0 ] == voiceStart[ 1 ] && voiceStart[ 1 ] == voiceStart[ 2 ] );
        AUDIO::CVoiceSenderStats senderStats;
        talker->sender->GetStats( senderStats );
        ASSERT_TRUE( 1 == senderStats.talkspurts );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceLink: A sender that restarts (new numbering) is followed by the receiver after the probation of the jitter buffer" )
    try
    {
        CPcmClickSignal signal( Rate, ClickPeriod, ClickOffset );
        CTestIntercomScenario scenario( Period, Rate );
        SUnitConfig aConfig;
        aConfig.keepOutput = false;
        SUnitConfig bConfig;
        bConfig.capture = false;
        bConfig.keepOutput = false;
        CTestIntercomUnit* a = scenario.AddUnit( aConfig );
        CTestIntercomUnit* b = scenario.AddUnit( bConfig );
        ASSERT_TRUE( GUCEF_NULL != a && GUCEF_NULL != b );
        if ( GUCEF_NULL == a || GUCEF_NULL == b )
            return;
        a->SetSignal( &signal );
        ASSERT_TRUE( a->AttachSender( SenderConfig( PcmCodec(), StreamIdA ) ) );
        AUDIO::CVoiceReceiver* receiver = b->AddReceiver( ReceiverConfig( PcmCodec(), AUDIO::VOICEPLAYOUTPROFILE_WIRED ) );
        ASSERT_TRUE( GUCEF_NULL != receiver );
        if ( GUCEF_NULL == receiver )
            return;
        scenario.Connect( a, b, receiver, SLinkModel::Wired() );
        CClickLatencyMeter meter( signal, *a, *b );
        scenario.AddObserver( &meter );
        scenario.Run( 5.0 );
        AUDIO::CVoiceReceiverStats before;
        receiver->GetStats( before );

        // the unit restarts its stream: the same stream id, a sequence number and timestamp far from the old ones
        AUDIO::CVoiceSenderConfig restarted = SenderConfig( PcmCodec(), StreamIdA );
        restarted.SetFirstSequenceNumber( 30000 );
        restarted.SetFirstTimestamp( 0 );
        ASSERT_TRUE( a->AttachSender( restarted ) );
        scenario.Run( 5.0 );
        AUDIO::CVoiceReceiverStats after;
        receiver->GetStats( after );
        const SLatencySummary first = Summarize( meter.clicks, 0.5, 5.0 );
        const SLatencySummary second = Summarize( meter.clicks, 5.5, 10.0 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceLink: restart: restarts " << after.restarts << ", talkspurts " << after.talkspurts << ", decoded " << before.framesDecoded << " then " << after.framesDecoded <<
                   ", concealed " << after.framesConcealed << ", end to end before " << first.mean << " ms, after " << second.mean << " ms (" << second.count << " clicks, " << meter.misses << " missed)" );
        ASSERT_TRUE( 1 == after.restarts && after.talkspurts >= 2 );
        ASSERT_TRUE( after.framesDecoded > before.framesDecoded + 450 );
        ASSERT_TRUE( after.framesConcealed <= 8 );
        ASSERT_TRUE( second.count >= 8 && second.maximum <= 40.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceLink: Two units talk to a third at once: two receivers on its bus 'call', the mix is the sum of both streams" )
    try
    {
        CPcmClickSignal signalA( Rate, 0, 0 );
        // C talks from 1 s on: a tone, 16 bit exact
        TSamples tone = TestVoiceSignals::MakeTone( 12 * Rate, Rate, 700.0, 0.1, 0.3 );
        for ( size_t n=0; n<tone.size(); ++n )
            tone[ n ] = n < Rate ? 0.0f : (Float32) floor( tone[ n ] * 32768.0f + 0.5f ) / 32768.0f;
        CBufferSignal signalC( tone );
        CTestIntercomScenario scenario( Period, Rate );
        SUnitConfig senderConfig;
        senderConfig.keepOutput = false;
        SUnitConfig listenerConfig;
        listenerConfig.capture = false;
        CTestIntercomUnit* a = scenario.AddUnit( senderConfig );
        CTestIntercomUnit* c = scenario.AddUnit( senderConfig );
        CTestIntercomUnit* b = scenario.AddUnit( listenerConfig );
        ASSERT_TRUE( GUCEF_NULL != a && GUCEF_NULL != b && GUCEF_NULL != c );
        if ( GUCEF_NULL == a || GUCEF_NULL == b || GUCEF_NULL == c )
            return;
        a->SetSignal( &signalA );
        c->SetSignal( &signalC );
        ASSERT_TRUE( a->AttachSender( SenderConfig( PcmCodec(), StreamIdA ) ) );
        ASSERT_TRUE( c->AttachSender( SenderConfig( PcmCodec(), StreamIdC ) ) );
        AUDIO::CAudioBus* call = b->engine->CreateBus( "call" );
        AUDIO::CVoiceReceiverConfig config = ReceiverConfig( PcmCodec(), AUDIO::VOICEPLAYOUTPROFILE_WIRED );
        config.SetMinMarginInMs( config.GetInitialMarginInMs() );
        AUDIO::CVoiceReceiver* fromA = b->AddReceiver( config, call );
        AUDIO::CVoiceReceiver* fromC = b->AddReceiver( config, call );
        ASSERT_TRUE( GUCEF_NULL != call && GUCEF_NULL != fromA && GUCEF_NULL != fromC );
        if ( GUCEF_NULL == fromA || GUCEF_NULL == fromC )
            return;
        SLinkModel fast = SLinkModel::Wired();
        fast.jitter = SLinkModel::JITTER_NONE;
        SLinkModel slow = fast;
        slow.baseDelayUs = 2300.0;      // C's packets take 2 ms longer
        scenario.Connect( a, b, fromA, fast );
        scenario.Connect( c, b, fromC, slow );
        scenario.Run( 10.0 );

        // each stream has its own fixed delay: A's from the first second (A alone), C's from where the mix first differs from A
        AUDIO::CVoiceReceiverStats statsA, statsC;
        fromA->GetStats( statsA );
        fromC->GetStats( statsC );
        const size_t foundA = FindLag( b->output, signalA, Rate / 4, 480, Rate / 4, Rate / 4 + Rate / 10 );
        ASSERT_TRUE( foundA < b->output.size() );
        const size_t lagA = foundA < b->output.size() ? foundA - Rate / 4 : 0;
        size_t firstC = b->output.size();
        for ( size_t m=Rate; m<b->output.size(); ++m )
        {
            if ( b->output[ m ] != signalA.Sample( m - lagA ) )
            {
                firstC = m;
                break;
            }
        }
        size_t toneStart = Rate;
        while ( toneStart < tone.size() && 0.0f == tone[ toneStart ] )
            ++toneStart;
        const size_t lagC = firstC - toneStart;
        Float64 worst = 0.0;
        for ( size_t m=lagC + Rate; m<b->output.size(); ++m )
        {
            const Float64 expected = (Float64) signalA.Sample( m - lagA ) + (Float64) signalC.Sample( m - lagC );
            const Float64 error = fabs( (Float64) b->output[ m ] - expected );
            if ( error > worst )
                worst = error;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceLink: two streams on one bus: decoded " << statsA.framesDecoded << " and " << statsC.framesDecoded << ", resampling " << statsA.resampling << " / " << statsC.resampling <<
                   ", delays " << lagA << " and " << lagC << " samples, largest difference of the mix from the sum of both streams " << worst << ", bus level " << call->GetMeter().GetRmsInDecibels( 0 ) << " dBFS" );
        ASSERT_TRUE( !statsA.resampling && !statsC.resampling );
        ASSERT_TRUE( lagC == lagA + (size_t) ( 0.002 * Rate ) );
        ASSERT_TRUE( worst <= 1.0e-6 );
        ASSERT_TRUE( 0 == statsA.framesConcealed && 0 == statsC.framesConcealed );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformLongRunTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceLink: Long run: one simulated hour of a call over Wi-Fi with jitter, loss, duplicates and drift stays stable" )
    try
    {
        // 16 kHz with 20 ms periods and frames keeps the hour short in wall time; the stalls are left to the Wi-Fi tests
        // (the ratio removes the delay each stretch added, so with stalls the integral holds that and not the drift)
        SCallConfig config;
        config.sampleRate = 16000;
        config.period = 320;
        config.codec = VoiceStreamTest::PcmConfig( 16000, 20000 );
        config.link = SLinkModel::Wired( 2026 );
        config.link.baseDelayUs = 3000.0;
        config.link.uniformJitterUs = 8000.0;
        config.link.randomLoss = 0.005;
        config.link.duplicateProbability = 0.01;
        config.profile = AUDIO::VOICEPLAYOUTPROFILE_WIRELESS;
        config.driftPpmA = 80.0;
        config.seconds = 3600.0;
        config.keepOutput = false;
        SCallResult result;
        ASSERT_TRUE( RunCall( config, result ) );
        const SLatencySummary second = Summarize( result.clicks, 1800.0, 2400.0 );
        const SLatencySummary last = Summarize( result.clicks, 3000.0 );
        Float64 marginMax = 0.0;
        for ( size_t i=60; i<result.seconds.size(); ++i )
            if ( result.seconds[ i ].marginInMs > marginMax )
                marginMax = result.seconds[ i ].marginInMs;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceLink: one hour in " << result.wallInS << " s: " << result.linkStats.sent << " frames, lost " << result.stats.cumulativeLost << ", duplicates " << result.stats.duplicates <<
                   ", stalled " << result.linkStats.stalled << ", concealed " << result.stats.framesConcealed << ", late " << result.stats.lateFrames << ", stretched " << result.stats.framesStretched <<
                   ", sequence cycles " << ( result.stats.extendedHighestSequenceNumber >> 16 ) << ", integral " << result.stats.integralInPpm << " ppm, margin up to " << marginMax <<
                   " ms, end to end in minutes 30 to 40 " << second.mean << " ms (" << second.minimum << " to " << second.maximum << "), in the last 10 " << last.mean << " ms (" << last.minimum << " to " << last.maximum << ")" );
        ASSERT_TRUE( ( result.stats.extendedHighestSequenceNumber >> 16 ) >= 2 );
        ASSERT_TRUE( fabs( result.stats.integralInPpm - 80.0 ) < 20.0 );
        ASSERT_TRUE( fabs( second.mean - last.mean ) <= 3.0 && last.maximum <= 80.0 );
        ASSERT_TRUE( (Float64) ( result.stats.lateFrames + result.stats.framesStretched ) <= 0.01 * (Float64) result.linkStats.sent );
        ASSERT_TRUE( marginMax <= 100.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** A thread that runs a function of a scenario part in a loop until stopped */
class CLoopThread : public MT::CActiveObject
{
    public:

    enum EJob { JOB_CAPTURE, JOB_NETWORK, JOB_PLAYBACK };

    CLoopThread( EJob job, CTestIntercomUnit& a, CTestIntercomUnit& b, CTestSimulatedVoiceLink& link )
        : MT::CActiveObject(), stop( 0 ), cycles( 0 ), m_job( job ), m_a( a ), m_b( b ), m_link( link ), m_sent() {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        while ( 0 == stop.Load() )
        {
            switch ( m_job )
            {
                case JOB_CAPTURE:
                    // the microphone: a period of the signal through the capture device
                    m_a.Capture( Period );
                    MT::PrecisionDelay( 4 );
                    break;
                case JOB_NETWORK:
                {
                    // the network task: polls the sender, moves packets over the link in system time, pushes to the receiver
                    m_sent.clear();
                    AUDIO::CVoiceFrame frame;
                    while ( m_a.sender->PopFrame( frame ) )
                        m_link.Send( frame, (Float64) AUDIO::CVoiceClock::GetSystemTimeInUs() );
                    AUDIO::CVoiceFrame arrived;
                    Float64 arrival = 0.0, send = 0.0;
                    while ( m_link.PopArrived( (Float64) AUDIO::CVoiceClock::GetSystemTimeInUs(), arrived, arrival, send ) )
                        m_b.receivers[ 0 ]->PushFrame( arrived, (UInt64) arrival );
                    MT::PrecisionDelay( 1 );
                    break;
                }
                case JOB_PLAYBACK:
                default:
                    m_b.GetPlaybackDevice()->Render( Period );
                    m_b.GetPlaybackDevice()->ClearRenderedAudio();
                    MT::PrecisionDelay( 4 );
                    break;
            }
            cycles.FetchAdd( 1 );
        }
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    MT::CTAtomic< UInt32 > stop;
    MT::CTAtomic< UInt32 > cycles;

    private:

    EJob m_job;
    CTestIntercomUnit& m_a;
    CTestIntercomUnit& m_b;
    CTestSimulatedVoiceLink& m_link;
    TSentFrames m_sent;
};

void
PerformThreadTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceLink: Threaded engines: a capture thread, a network thread and a playback thread with the owner changing controls (TSan)" )
    try
    {
        CPcmClickSignal signal( Rate, ClickPeriod, ClickOffset );
        SUnitConfig aConfig;
        aConfig.threading = AUDIO::AUDIOTHREADING_THREADED;
        aConfig.keepOutput = false;
        SUnitConfig bConfig;
        bConfig.threading = AUDIO::AUDIOTHREADING_THREADED;
        bConfig.capture = false;
        bConfig.keepOutput = false;
        CTestIntercomUnit a( aConfig );
        CTestIntercomUnit b( bConfig );
        ASSERT_TRUE( a.Init() && b.Init() );
        a.SetSignal( &signal );
        ASSERT_TRUE( a.AttachSender( SenderConfig( PcmCodec(), StreamIdA ) ) );
        AUDIO::CVoiceReceiverConfig config = ReceiverConfig( PcmCodec(), AUDIO::VOICEPLAYOUTPROFILE_WIRELESS );
        AUDIO::CVoiceReceiver* receiver = GUCEF_NEW AUDIO::CVoiceReceiver( *b.engine, config );
        ASSERT_TRUE( receiver->Prepare() );
        b.receivers.push_back( receiver );
        // the voice starts with the first render of the playback thread
        CORE::CFutureResult started = receiver->Start();
        SLinkModel model = SLinkModel::Wired();
        model.baseDelayUs = 2000.0;
        model.uniformJitterUs = 2000.0;
        CTestSimulatedVoiceLink link( model );

        CLoopThread capture( CLoopThread::JOB_CAPTURE, a, b, link );
        CLoopThread network( CLoopThread::JOB_NETWORK, a, b, link );
        CLoopThread playback( CLoopThread::JOB_PLAYBACK, a, b, link );
        ASSERT_TRUE( capture.Activate() && network.Activate() && playback.Activate() );
        for ( UInt32 i=0; i<40; ++i )
        {
            MT::PrecisionDelay( 25 );
            receiver->SetVolume( 0 == i % 2 ? 0.5f : 1.0f );
            a.sender->SetMuted( 7 == i % 8 );
            a.engine->SetCaptureMuted( 5 == i % 10 );
            if ( 20 == i )
                receiver->Reset();
            AUDIO::CVoiceReceiverStats stats;
            receiver->GetStats( stats );
            AUDIO::CVoiceSenderStats senderStats;
            a.sender->GetStats( senderStats );
            (void) receiver->IsPlaying();
            (void) receiver->GetLevelInDecibels();
        }
        capture.stop.Store( 1 );
        network.stop.Store( 1 );
        playback.stop.Store( 1 );
        capture.WaitForThreadToFinish( 10000 );
        network.WaitForThreadToFinish( 10000 );
        playback.WaitForThreadToFinish( 10000 );
        AUDIO::CVoiceReceiverStats stats;
        receiver->GetStats( stats );
        AUDIO::CVoiceSenderStats senderStats;
        a.sender->GetStats( senderStats );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceLink: threads: " << capture.cycles.Load() << " capture, " << network.cycles.Load() << " network and " << playback.cycles.Load() << " playback cycles; sent " <<
                   senderStats.framesSent << ", pushed " << stats.framesPushed << ", decoded " << stats.framesDecoded << ", resets " << stats.resets );
        ASSERT_TRUE( started.IsReady() );
        ASSERT_TRUE( senderStats.framesSent > 20 && stats.framesPushed > 20 && stats.framesDecoded > 10 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformRealtimeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceLink: Both audio threads allocate nothing and take no lock in a call with Opus, loss, redundancy, drift, push-to-talk, voice activation and a reset (RealtimeSanitizer build)" )
    if ( !IsRtsanBuild )
    {
        GUCEF_TESTFW_SKIP( "not a RealtimeSanitizer build (-fsanitize=realtime)" );
    }
    GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_RTSAN_CLEAN" ) == "0", "switched off with GUCEF_TEST_AUDIO_RTSAN_CLEAN=0" );
    try
    {
        // every capture callback and every render of the engines below is a real time context in this build
        const bool opus = LoadOpus();
        CPcmClickSignal signal( Rate, ClickPeriod, ClickOffset );
        CTestIntercomScenario scenario( Period, Rate );
        SUnitConfig aConfig;
        aConfig.driftPpm = 150.0;
        aConfig.keepOutput = false;
        SUnitConfig bConfig;
        bConfig.keepOutput = false;
        CTestIntercomUnit* a = scenario.AddUnit( aConfig );
        CTestIntercomUnit* b = scenario.AddUnit( bConfig );
        ASSERT_TRUE( GUCEF_NULL != a && GUCEF_NULL != b );
        if ( GUCEF_NULL == a || GUCEF_NULL == b )
            return;
        const AUDIO::CVoiceCodecConfig codec = opus ? OpusCodec( AUDIO::VOICECODECPROFILE_ROBUST ) : VoiceStreamTest::PcmConfig( Rate, 5000 );
        a->SetSignal( &signal );
        b->SetSignal( &signal );
        ASSERT_TRUE( a->AttachSender( SenderConfig( codec, StreamIdA, AUDIO::VOICETRANSMITMODE_CONTINUOUS, 1 ) ) );
        AUDIO::CVoiceReceiver* receiver = b->AddReceiver( ReceiverConfig( codec, AUDIO::VOICEPLAYOUTPROFILE_WIRELESS ) );
        ASSERT_TRUE( GUCEF_NULL != receiver );
        if ( GUCEF_NULL == receiver )
            return;
        SLinkModel model = SLinkModel::WiFi( 5 );
        model.geGoodToBad = 0.01;
        model.geBadToGood = 0.5;
        model.duplicateProbability = 0.01;
        model.reorderProbability = 0.02;
        scenario.Connect( a, b, receiver, model );
        scenario.Run( 3.0 );
        a->sender->SetTransmitMode( AUDIO::VOICETRANSMITMODE_PUSH_TO_TALK );
        scenario.Run( 1.0 );
        a->sender->SetTransmitting( true );
        scenario.Run( 2.0 );
        AUDIO::CVoiceReceiverStats before;
        receiver->GetStats( before );
        a->sender->SetTransmitMode( AUDIO::VOICETRANSMITMODE_VOICE_ACTIVATED );
        receiver->Reset();
        scenario.Run( 2.0 );
        AUDIO::CVoiceReceiverStats stats;
        receiver->GetStats( stats );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceLink: real time: decoded " << before.framesDecoded << " before the reset, " << stats.framesDecoded << " after, concealed " << stats.framesConcealed << ", recovered " << stats.framesRecovered << ", resets " << stats.resets );
        ASSERT_TRUE( before.framesDecoded > 100 && stats.resets >= 1 );
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
PerformAudioVoiceLinkTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioVoiceLink TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.VoiceLink" );

    PerformLinkModelTests();
    PerformLatencyTests();
    PerformLossTests();
    PerformDriftTests();
    PerformTalkspurtTests();
    PerformMultiStreamTests();
    PerformLongRunTests();
    PerformThreadTests();
    PerformRealtimeTests();
}

/*-------------------------------------------------------------------------*/
