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

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioVoiceStreamSupport.h"
#include "TestAudioVoiceReceiver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace VoiceStreamTest;

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

const UInt32 StreamId = 0x0BADCAFE;

bool
LoadOpus( void )
{
    return VoiceCodecTest::LoadVoiceCodecPlugin( "audiovoicecodecOPUS", "audiovoicecodecopus", "opus" );
}

AUDIO::CVoiceReceiverConfig
ReceiverConfig( const AUDIO::CVoiceCodecConfig& codecConfig, AUDIO::TVoicePlayoutProfile profile )
{
    AUDIO::CVoiceReceiverConfig config;
    config.SetCodecConfig( codecConfig );
    config.SetPlayoutProfile( profile );
    return config;
}

AUDIO::CAudioEngineParams
PlaybackParams( UInt32 rate, UInt32 period )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_OFF );
    params.GetDeviceParams().SetPreferredFormat( MonoFormat( rate ) );
    params.GetDeviceParams().SetBufferSizeInFrames( period );
    params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );
    return params;
}

/*-------------------------------------------------------------------------*/

/** How a loop of a sender, a simulated network and a receiver is set up */
struct SLoopConfig
{
    AUDIO::CVoiceCodecConfig codec;
    UInt32 period;                  /**< frames per render request */
    bool randomRequests;            /**< render requests of 1 to 3 periods at random (the source alone) */
    Float64 transitInUs;
    Float64 jitterInUs;             /**< plus U( 0, this ) per frame */
    Float64 driftPpm;               /**< the sender's clock is this much faster */
    UInt32 lossEvery;               /**< frame k is lost when k % lossEvery == lossEvery - 1 (0: none) */
    UInt32 redundancy;
    AUDIO::TVoicePlayoutProfile profile;
    bool useEngine;                 /**< through a CVoiceReceiver and an engine on the null device, else the playout source alone */
    bool fixedMargin;               /**< the minimum margin is the initial one: the margin never decays (a stream on time then plays at ratio 1) */
    UInt32 seed;

    SLoopConfig( void )
        : codec( PcmConfig( 48000 ) ), period( 240 ), randomRequests( false ), transitInUs( 1000.0 ), jitterInUs( 0.0 ), driftPpm( 0.0 )
        , lossEvery( 0 ), redundancy( 0 ), profile( AUDIO::VOICEPLAYOUTPROFILE_WIRED ), useEngine( false ), fixedMargin( false ), seed( 1 )
    {}
};

/** A frame on its way through the simulated network */
struct SInFlight
{
    UInt64 arrivalInUs;
    AUDIO::CVoiceFrame frame;
};

/**
 *  A sender fed with the PCM test signal, a simulated network of constant transit (plus jitter and loss) and the
 *  receiving side, in one simulated clock. Each step is one render request of the receiver
 */
class CVoiceLoop
{
    public:

    explicit CVoiceLoop( const SLoopConfig& config )
        : m_config( config ), clock(), engine( GUCEF_NULL ), receiver( GUCEF_NULL ), source( GUCEF_NULL ), sender( GUCEF_NULL )
        , output(), framesSent( 0 ), framesLost( 0 ), switchedAt( 0 ), switched( false ), m_rate( config.codec.GetSampleRate() ), m_position( 0 )
        , m_carry( 0.0 ), m_random( config.seed ), m_inFlight(), m_capture(), m_request(), m_ownSource( GUCEF_NULL ), m_decoder()
        , m_startInUs( clock.GetTimeInUs() ), m_senderBase( 0 ), m_senderFrames( 0 )
    {}

    ~CVoiceLoop()
    {
        GUCEF_DELETE receiver;
        if ( GUCEF_NULL != engine )
            engine->Shutdown();
        GUCEF_DELETE engine;
        GUCEF_DELETE sender;
        GUCEF_DELETE m_ownSource;
    }

    /** The sender of the loop: from now on this one captures (the stream of the old one ends here) */
    bool ReplaceSender( const AUDIO::CVoiceSenderConfig& senderConfig )
    {
        GUCEF_DELETE sender;
        sender = GUCEF_NEW AUDIO::CVoiceSender( senderConfig );
        m_senderBase = m_position;
        m_senderFrames = 0;
        return sender->PrepareForCapture( MonoFormat( m_rate ), 8192 );
    }

    AUDIO::CVoiceSenderConfig SenderConfig( void ) const
    {
        AUDIO::CVoiceSenderConfig senderConfig;
        senderConfig.SetCodecConfig( m_config.codec );
        senderConfig.SetStreamId( StreamId );
        senderConfig.SetFirstSequenceNumber( 1000 );
        senderConfig.SetFirstTimestamp( 0 );
        senderConfig.SetRedundancy( m_config.redundancy );
        senderConfig.SetOutboundQueueFrames( 64 );
        return senderConfig;
    }

    bool Init( void )
    {
        if ( !ReplaceSender( SenderConfig() ) )
            return false;

        AUDIO::CVoiceReceiverConfig receiverConfig = ReceiverConfig( m_config.codec, m_config.profile );
        if ( m_config.fixedMargin )
            receiverConfig.SetMinMarginInMs( receiverConfig.GetInitialMarginInMs() );
        if ( m_config.useEngine )
        {
            engine = GUCEF_NEW AUDIO::CAudioEngine();
            if ( !engine->Initialize( PlaybackParams( m_rate, m_config.period ) ) )
                return false;
            receiver = GUCEF_NEW AUDIO::CVoiceReceiver( *engine, receiverConfig );
            receiver->SetClock( clock );
            if ( !receiver->Prepare() )
                return false;
            const CORE::CFutureResult started = receiver->Start();
            engine->Update();
            if ( started.HasNoFuture() )
                return false;
            source = &receiver->GetPlayoutSource();
        }
        else
        {
            if ( !AUDIO::CAudioGlobal::Instance()->GetVoiceCodecRegistry().CreateDecoder( m_config.codec, m_decoder ) )
                return false;
            m_ownSource = GUCEF_NEW AUDIO::CVoicePlayoutSource();
            if ( !m_ownSource->Prepare( receiverConfig, m_decoder, clock ) )
                return false;
            source = m_ownSource;
        }
        m_capture.assign( 16384, 0.0f );
        m_request.assign( 16384, 0.0f );
        return true;
    }

    /** One render request: the sender captures the same time (times its drift), the network delivers, the receiver renders */
    void Step( void )
    {
        UInt32 frames = m_config.period;
        if ( m_config.randomRequests )
            frames = m_config.period * ( 1 + m_random.Next() % 3 ) - m_random.Next() % 50;
        const UInt64 nowInUs = clock.GetTimeInUs();
        const Float64 stepInUs = (Float64) frames * 1000000.0 / (Float64) m_rate;

        // the sender captures the same span of its own (drifting) time
        m_carry += (Float64) frames * ( 1.0 + m_config.driftPpm * 1.0e-6 );
        const UInt32 captured = (UInt32) floor( m_carry );
        m_carry -= (Float64) captured;
        for ( UInt32 n=0; n<captured; ++n )
            m_capture[ n ] = PcmSample( m_position + n, m_rate );
        AUDIO::IAudioCaptureConsumer::SCaptureBlockInfo info;
        info.firstFrame = m_position;
        info.tickCount = nowInUs;
        sender->OnCapturedAudio( &m_capture[ 0 ], captured, info );
        m_position += captured;
        AUDIO::CVoiceFrame frame;
        while ( sender->PopFrame( frame ) )
        {
            const UInt64 index = framesSent++;
            // the frame is complete when its last sample was captured, in the time of the receiver
            const UInt64 endPosition = m_senderBase + ( ++m_senderFrames ) * (UInt64) m_config.codec.GetFrameSizeInFrames();
            const Float64 completedInUs = (Float64) m_startInUs + (Float64) endPosition * 1000000.0 / ( (Float64) m_rate * ( 1.0 + m_config.driftPpm * 1.0e-6 ) );
            if ( 0 != m_config.lossEvery && m_config.lossEvery - 1 == index % m_config.lossEvery )
            {
                ++framesLost;
                continue;
            }
            SInFlight flight;
            flight.arrivalInUs = (UInt64) ( completedInUs + m_config.transitInUs + m_config.jitterInUs * m_random.NextUnit() );
            flight.frame = frame;
            size_t at = m_inFlight.size();
            while ( at > 0 && m_inFlight[ at - 1 ].arrivalInUs > flight.arrivalInUs )
                --at;
            m_inFlight.insert( m_inFlight.begin() + at, flight );
        }

        // time passes, the network delivers what has arrived, the receiver renders
        clock.SetTimeInUs( nowInUs + (UInt64) stepInUs );
        Deliver();
        Render( frames );
    }

    void Deliver( void )
    {
        const UInt64 nowInUs = clock.GetTimeInUs();
        size_t delivered = 0;
        while ( delivered < m_inFlight.size() && m_inFlight[ delivered ].arrivalInUs <= nowInUs )
        {
            if ( GUCEF_NULL != receiver )
                receiver->PushFrame( m_inFlight[ delivered ].frame, m_inFlight[ delivered ].arrivalInUs );
            else
                source->PushFrame( m_inFlight[ delivered ].frame, m_inFlight[ delivered ].arrivalInUs );
            ++delivered;
        }
        m_inFlight.erase( m_inFlight.begin(), m_inFlight.begin() + delivered );
    }

    void Render( UInt32 frames )
    {
        if ( GUCEF_NULL != engine )
        {
            AUDIO::CNullAudioDevice* device = engine->GetDevice()->AsNullDevice();
            device->Render( frames );
            engine->Update();
            const AUDIO::CAudioBuffer& rendered = device->GetRenderedAudio();
            const Float32* data = static_cast< const Float32* >( rendered.GetDataPtr( 0 ) );
            output.insert( output.end(), data, data + rendered.GetFrameCount() );
            device->ClearRenderedAudio();
        }
        else
        {
            source->ReadFrames( &m_request[ 0 ], frames );
            output.insert( output.end(), m_request.begin(), m_request.begin() + frames );
        }
        if ( !switched )
        {
            AUDIO::CVoiceReceiverStats stats;
            source->GetStats( stats );
            if ( stats.resampling )
            {
                switched = true;
                switchedAt = output.size() - frames;
            }
        }
    }

    void Run( Float64 seconds )
    {
        const UInt64 end = clock.GetTimeInUs() + (UInt64) ( seconds * 1000000.0 );
        while ( clock.GetTimeInUs() < end )
            Step();
    }

    void GetStats( AUDIO::CVoiceReceiverStats& stats ) const
    {
        if ( GUCEF_NULL != receiver )
            receiver->GetStats( stats );
        else
            source->GetStats( stats );
    }

    /** The output position of the first sample of the stream (the signal never is 0 at its start) */
    size_t FirstSound( void ) const
    {
        for ( size_t n=0; n<output.size(); ++n )
            if ( 0.0f != output[ n ] )
                return n;
        return output.size();
    }

    /** How many output samples from 'delay' on equal the signal from its start, up to 'limit' */
    size_t MatchingSamples( size_t delay, size_t limit ) const
    {
        size_t n = 0;
        while ( delay + n < output.size() && n < limit && output[ delay + n ] == PcmSample( n, m_rate ) )
            ++n;
        return n;
    }

    UInt32 GetRate( void ) const { return m_rate; }

    private:

    SLoopConfig m_config;

    public:

    CSimulatedVoiceClock clock;
    AUDIO::CAudioEngine* engine;
    AUDIO::CVoiceReceiver* receiver;
    AUDIO::CVoicePlayoutSource* source;
    AUDIO::CVoiceSender* sender;
    TSamples output;
    UInt64 framesSent;
    UInt64 framesLost;
    size_t switchedAt;
    bool switched;

    private:

    UInt32 m_rate;
    UInt64 m_position;
    Float64 m_carry;
    AudioEffectTest::CXorShift m_random;
    GUCEF::vector< SInFlight > m_inFlight;
    TSamples m_capture;
    TSamples m_request;
    AUDIO::CVoicePlayoutSource* m_ownSource;
    AUDIO::TVoiceDecoderPtr m_decoder;
    UInt64 m_startInUs;
    UInt64 m_senderBase;
    UInt64 m_senderFrames;
};

/*-------------------------------------------------------------------------*/

/** The largest step from one sample to the next in a range */
Float32
MaxStep( const TSamples& signal, size_t from, size_t to )
{
    Float32 largest = 0.0f;
    for ( size_t n=from + 1; n<to && n<signal.size(); ++n )
    {
        const Float32 d = fabsf( signal[ n ] - signal[ n - 1 ] );
        if ( d > largest )
            largest = d;
    }
    return largest;
}

bool
IsFinite( const TSamples& signal )
{
    for ( size_t n=0; n<signal.size(); ++n )
        if ( !( signal[ n ] == signal[ n ] ) || fabsf( signal[ n ] ) > 4.0f )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/** A playback push device with a thread of its own that renders a little faster than real time */
class CPacedPlaybackDevice : public AUDIO::CAudioPushDevice
{
    public:

    explicit CPacedPlaybackDevice( AUDIO::IAudioDeviceCallback* callback )
        : AUDIO::CAudioPushDevice( AUDIO::CAudioDeviceInfo( "receiverplayback", "Paced playback for the receiver tests", AUDIO::AUDIODIRECTION_PLAYBACK ),
                                   callback, MonoFormat( 48000 ), 240 )
    {}

    virtual ~CPacedPlaybackDevice() GUCEF_VIRTUAL_OVERRIDE
    {
        Stop();
    }

    protected:

    virtual bool OnPushStart( void ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual void OnPushStop( void ) GUCEF_VIRTUAL_OVERRIDE {}

    virtual TPushResult PushFrames( const void*, UInt32 ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::PrecisionDelay( 4 );
        return PUSHRESULT_OK;
    }
};

class CPacedTestDriver : public AUDIO::CAudioDriver
{
    public:

    CPacedTestDriver( void ) : AUDIO::CAudioDriver() {}

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "receivertest"; }
    virtual Int32 GetPriority( void ) const GUCEF_VIRTUAL_OVERRIDE { return -2000; }
    virtual bool EnumerateDevices( AUDIO::TAudioDirection, AUDIO::TAudioDeviceInfoVector& ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OpenDevice( const AUDIO::CAudioDeviceParams& params, AUDIO::IAudioDeviceCallback* callback, AUDIO::TAudioDevicePtr& device ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( AUDIO::AUDIODIRECTION_CAPTURE == params.GetDirection() )
            return false;
        device = AUDIO::TAudioDevicePtr( GUCEF_NEW CPacedPlaybackDevice( callback ) );
        return true;
    }
};

/** The network thread: pushes the frames of several streams at their pace in the system clock */
class CPushThread : public MT::CActiveObject
{
    public:

    CPushThread( AUDIO::CVoiceReceiver** receivers, const TFrames* streams, UInt32 streamCount )
        : MT::CActiveObject(), m_receivers( receivers ), m_streams( streams ), m_streamCount( streamCount ), stop( 0 ), pushed( 0 ) {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        size_t index = 0;
        while ( 0 == stop.Load() )
        {
            for ( UInt32 s=0; s<m_streamCount; ++s )
            {
                const TFrames& frames = m_streams[ s ];
                AUDIO::CVoiceFrame frame = frames[ index % frames.size() ];
                // the stream loops: the numbers go on
                frame.SetSequenceNumber( (UInt16) ( frame.GetSequenceNumber() + ( index / frames.size() ) * frames.size() ) );
                frame.SetTimestamp( frame.GetTimestamp() + (UInt32) ( ( index / frames.size() ) * frames.size() * 480 ) );
                if ( m_receivers[ s ]->PushFrame( frame, AUDIO::CVoiceClock::GetSystemTimeInUs() ) )
                    pushed.FetchAdd( 1 );
            }
            ++index;
            MT::PrecisionDelay( 8 );
        }
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    private:

    AUDIO::CVoiceReceiver** m_receivers;
    const TFrames* m_streams;
    UInt32 m_streamCount;

    public:

    MT::CTAtomic< UInt32 > stop;
    MT::CTAtomic< UInt32 > pushed;
};

/** Frames of the PCM test signal at 48 kHz, 10 ms, from a sender */
void
MakeStream( UInt32 streamId, UInt32 frameCount, TFrames& frames )
{
    AUDIO::CVoiceSenderConfig config;
    config.SetCodecConfig( PcmConfig( 48000 ) );
    config.SetStreamId( streamId );
    config.SetOutboundQueueFrames( 4096 );
    AUDIO::CVoiceSender sender( config );
    sender.PrepareForCapture( MonoFormat( 48000 ), 8192 );
    const TSamples signal = MakePcmSignal( frameCount * 480, 48000 );
    UInt64 position = 0;
    FeedSender( sender, signal, 0, signal.size(), position, 48000, 480, false );
    PopAll( sender, frames );
}

/*-------------------------------------------------------------------------*/

void
PerformConfigTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceReceiver: The configuration has the defaults of the design, the profiles set their margins, values are limited and survive a save and load" )
    try
    {
        AUDIO::CVoiceReceiverConfig config;
        ASSERT_TRUE( AUDIO::VOICEPLAYOUTPROFILE_WIRELESS == config.GetPlayoutProfile() );
        ASSERT_TRUE( 20.0 == config.GetInitialMarginInMs() && 6.0 == config.GetMinMarginInMs() && 200.0 == config.GetMaxMarginInMs() && 4.0 == config.GetWindowInS() );
        ASSERT_TRUE( 32 == config.GetInboundQueueFrames() && 64 == config.GetSlotCount() && 6 == config.GetConcealLimit() );
        ASSERT_TRUE( 0 == config.GetStreamId() && 113 == config.GetRedPayloadType() );
        ASSERT_TRUE( 1.0 == config.GetLatePercentage() );
        config.SetPlayoutProfile( AUDIO::VOICEPLAYOUTPROFILE_WIRED );
        ASSERT_TRUE( 4.0 == config.GetInitialMarginInMs() && 2.0 == config.GetMinMarginInMs() && 60.0 == config.GetMaxMarginInMs() && 2.0 == config.GetWindowInS() );
        ASSERT_TRUE( 0.0 == config.GetLatePercentage() );
        config.SetLatePercentage( 50.0 );
        ASSERT_TRUE( 20.0 == config.GetLatePercentage() );
        config.SetLatePercentage( -1.0 );
        ASSERT_TRUE( 0.0 == config.GetLatePercentage() );

        config.SetSlotCount( 100 );
        ASSERT_TRUE( 128 == config.GetSlotCount() );
        config.SetSlotCount( 1 );
        ASSERT_TRUE( 2 == config.GetSlotCount() );
        config.SetInboundQueueFrames( 0 );
        config.SetConcealLimit( 0 );
        ASSERT_TRUE( 2 == config.GetInboundQueueFrames() && 1 == config.GetConcealLimit() );
        config.SetInitialMarginInMs( 500.0 );
        ASSERT_TRUE( 60.0 == config.GetInitialMarginInMs() );
        config.SetMinMarginInMs( 80.0 );
        ASSERT_TRUE( 80.0 == config.GetMinMarginInMs() && 80.0 == config.GetMaxMarginInMs() && 80.0 == config.GetInitialMarginInMs() );

        AUDIO::TVoicePlayoutProfile profile = AUDIO::VOICEPLAYOUTPROFILE_WIRELESS;
        ASSERT_TRUE( AUDIO::CVoiceReceiverConfig::StringToPlayoutProfile( "Wired", profile ) && AUDIO::VOICEPLAYOUTPROFILE_WIRED == profile );
        ASSERT_FALSE( AUDIO::CVoiceReceiverConfig::StringToPlayoutProfile( "cable", profile ) );

        AUDIO::CVoiceReceiverConfig saved = ReceiverConfig( PcmConfig( 16000, 20000 ), AUDIO::VOICEPLAYOUTPROFILE_WIRED );
        saved.SetInitialMarginInMs( 7.5 );
        saved.SetMaxMarginInMs( 45.0 );
        saved.SetWindowInS( 3.0 );
        saved.SetInboundQueueFrames( 48 );
        saved.SetSlotCount( 32 );
        saved.SetConcealLimit( 9 );
        saved.SetStreamId( StreamId );
        saved.SetRedPayloadType( 98 );
        saved.SetLatePercentage( 0.5 );
        CORE::CDataNode tree;
        ASSERT_TRUE( saved.SaveConfig( tree ) && GUCEF_NULL != tree.FindChild( "VoiceReceiver" ) );
        AUDIO::CVoiceReceiverConfig loaded;
        ASSERT_TRUE( loaded.LoadConfig( tree ) && loaded == saved );
        AUDIO::CVoiceReceiverConfig fromSection;
        ASSERT_TRUE( fromSection.LoadConfig( *tree.FindChild( "VoiceReceiver" ) ) && fromSection == saved );
        CORE::CDataNode empty;
        ASSERT_FALSE( loaded.LoadConfig( empty ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformPlayoutTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceReceiver: PCM at a constant delay plays bit for bit, the ratio stays exactly 1 and the resampler is never used" )
    try
    {
        struct SCase { UInt32 rate; UInt32 duration; UInt32 period; bool randomRequests; AUDIO::TVoicePlayoutProfile profile; };
        const SCase cases[ 4 ] = { { 48000, 10000, 240, false, AUDIO::VOICEPLAYOUTPROFILE_WIRED },
                                   { 16000, 20000, 80, false, AUDIO::VOICEPLAYOUTPROFILE_WIRELESS },
                                   { 48000, 5000, 1500, false, AUDIO::VOICEPLAYOUTPROFILE_WIRED },
                                   { 48000, 10000, 256, true, AUDIO::VOICEPLAYOUTPROFILE_WIRED } };
        for ( int c=0; c<4; ++c )
        {
            SLoopConfig config;
            config.codec = PcmConfig( cases[ c ].rate, cases[ c ].duration );
            config.period = cases[ c ].period;
            config.randomRequests = cases[ c ].randomRequests;
            config.profile = cases[ c ].profile;
            config.fixedMargin = true;
            CVoiceLoop loop( config );
            ASSERT_TRUE( loop.Init() );
            loop.Run( 20.0 );

            const size_t delay = loop.FirstSound();
            const size_t matching = loop.MatchingSamples( delay, loop.output.size() );
            AUDIO::CVoiceReceiverStats stats;
            loop.GetStats( stats );
            const Float64 delayInMs = (Float64) delay * 1000.0 / (Float64) cases[ c ].rate;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceReceiver: pcm " << cases[ c ].rate << " Hz, " << cases[ c ].duration << " us frames, requests of " << cases[ c ].period <<
                       ( cases[ c ].randomRequests ? " (random)" : "" ) << ": delay " << delayInMs << " ms, " << matching << " of " << loop.output.size() - delay <<
                       " samples bit-exact, margin " << stats.marginInMs << " ms, Lmin " << stats.minimumLevelInMs << " ms, ratio " << stats.ratioInPpm << " ppm, playout delay " << stats.meanPlayoutDelayInMs <<
                       " ms, one-way " << stats.meanOneWayDelayInMs << " ms" );
            ASSERT_TRUE( delay < loop.output.size() );
            ASSERT_TRUE( 0 == stats.framesConcealed && 0 == stats.underruns && 0 == stats.lateFrames && 0 == stats.outputShortfalls );
            if ( cases[ c ].randomRequests )
            {
                // requests of changing size: the margin rules see a changing block, the ratio moves; the audio stays whole and smooth
                const Float32 signalStep = MaxStep( MakePcmSignal( 48000, 48000 ), 0, 48000 );
                ASSERT_TRUE( IsFinite( loop.output ) && MaxStep( loop.output, delay + 100, loop.output.size() ) <= signalStep * 1.02f );
                ASSERT_TRUE( stats.framesDecoded + 5 >= loop.framesSent );
                continue;
            }
            ASSERT_TRUE( matching + 2 * cases[ c ].rate / 10 >= loop.output.size() - delay );     // all but the frames still on their way
            ASSERT_TRUE( !stats.resampling && 0 == stats.resamplerSwitches && 0.0 == stats.ratioInPpm );
            ASSERT_TRUE( 1 == stats.talkspurts && AUDIO::VOICEPLAYOUTSTATE_PLAYING == stats.state && stats.framesDecoded + 3 >= loop.framesSent );
            // the delay: the transit, a frame, the margin and a request, give or take a request
            const Float64 expectedInMs = 1.0 + (Float64) cases[ c ].duration / 1000.0 + stats.marginInMs + (Float64) cases[ c ].period * 1000.0 / (Float64) cases[ c ].rate;
            ASSERT_TRUE( fabs( delayInMs - expectedInMs ) <= 3.0 * (Float64) cases[ c ].period * 1000.0 / (Float64) cases[ c ].rate + 1.0 );
            ASSERT_TRUE( stats.oneWayDelayCount > 0 );
            if ( c < 2 )
                ASSERT_TRUE( fabs( stats.meanOneWayDelayInMs - stats.maxOneWayDelayInMs ) < 0.01 );    // frames begin at the same place in the requests
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceReceiver: End to end through the engine on the null device: PCM bit for bit at a constant delay, level and state readable" )
    try
    {
        SLoopConfig config;
        config.useEngine = true;
        config.fixedMargin = true;
        CVoiceLoop loop( config );
        ASSERT_TRUE( loop.Init() );
        ASSERT_FALSE( loop.receiver->IsPlaying() );
        loop.Run( 10.0 );
        const size_t delay = loop.FirstSound();
        const size_t matching = loop.MatchingSamples( delay, loop.output.size() );
        AUDIO::CVoiceReceiverStats stats;
        loop.GetStats( stats );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceReceiver: engine: delay " << delay << " samples, " << matching << " of " << loop.output.size() - delay << " bit-exact, level " << loop.receiver->GetLevelInDecibels() << " dB" );
        ASSERT_TRUE( matching + 9600 >= loop.output.size() - delay && matching > 400000 );
        ASSERT_TRUE( loop.receiver->IsPlaying() && loop.receiver->GetLevelInDecibels() > -30.0f );
        ASSERT_TRUE( StreamId == stats.streamId && stats.framesPushed == stats.framesQueued && 0 == stats.wrongStream );

        // the volume reaches the voice
        loop.receiver->SetVolume( 0.5f );
        ASSERT_TRUE( 0.5f == loop.receiver->GetVolume() && 0.5f == loop.receiver->GetVoice()->GetVolume() );
        CORE::CFutureResult stopped = loop.receiver->Stop();
        loop.Step();
        ASSERT_TRUE( stopped.IsReady() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceReceiver: Lost frames are concealed (PCM), recovered from the next frame (Opus robust) or filled in from redundancy (bit for bit)" )
    try
    {
        // PCM: concealment
        {
            SLoopConfig config;
            config.lossEvery = 7;
            CVoiceLoop loop( config );
            ASSERT_TRUE( loop.Init() );
            loop.Run( 10.0 );
            AUDIO::CVoiceReceiverStats stats;
            loop.GetStats( stats );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceReceiver: pcm, every 7th frame lost: " << loop.framesLost << " lost, " << stats.framesConcealed << " concealed, " << stats.cumulativeLost << " counted lost" );
            ASSERT_TRUE( IsFinite( loop.output ) );
            ASSERT_TRUE( stats.framesConcealed + 1 >= loop.framesLost && stats.framesConcealed <= loop.framesLost + 1 && 0 == stats.framesRecovered );
            ASSERT_TRUE( (Int64) loop.framesLost >= stats.cumulativeLost && stats.cumulativeLost + 1 >= (Int64) loop.framesLost );
        }
        // PCM with redundancy: every lost frame comes from the next packet, the output is the input bit for bit
        {
            SLoopConfig config;
            config.codec = PcmConfig( 16000 );
            config.period = 80;
            config.lossEvery = 5;
            config.redundancy = 1;
            config.transitInUs = 12000.0;
            config.profile = AUDIO::VOICEPLAYOUTPROFILE_WIRELESS;
            config.fixedMargin = true;      // the wireless margin decays every second: a ratio other than 1 would end the bit for bit output
            CVoiceLoop loop( config );
            ASSERT_TRUE( loop.Init() );
            loop.Run( 10.0 );
            AUDIO::CVoiceReceiverStats stats;
            loop.GetStats( stats );
            const size_t delay = loop.FirstSound();
            const size_t matching = loop.MatchingSamples( delay, loop.output.size() );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceReceiver: pcm with redundancy, every 5th frame lost: " << loop.framesLost << " lost, " << stats.redundantStored << " filled in, " << stats.framesConcealed << " concealed, " << matching << " samples bit-exact" );
            ASSERT_TRUE( stats.redundantStored + 1 >= loop.framesLost && 0 == stats.framesConcealed );
            ASSERT_TRUE( matching + 3200 >= loop.output.size() - delay );
        }
        // Opus robust: forward error correction
        if ( LoadOpus() )
        {
            SLoopConfig config;
            config.codec = VoiceCodecTest::MakeConfig( "opus", 48000, 20000 );
            config.codec.SetProfileDefaults( AUDIO::VOICECODECPROFILE_ROBUST );
            config.lossEvery = 5;
            config.transitInUs = 25000.0;
            config.profile = AUDIO::VOICEPLAYOUTPROFILE_WIRELESS;
            CVoiceLoop loop( config );
            ASSERT_TRUE( loop.Init() );
            loop.Run( 10.0 );
            AUDIO::CVoiceReceiverStats stats;
            loop.GetStats( stats );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceReceiver: opus robust, every 5th frame lost: " << loop.framesLost << " lost, " << stats.framesRecovered << " recovered, " << stats.framesConcealed << " concealed" );
            ASSERT_TRUE( IsFinite( loop.output ) );
            ASSERT_TRUE( stats.framesRecovered > loop.framesLost / 2 && stats.framesRecovered + stats.framesConcealed + 1 >= loop.framesLost );
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceReceiver: the Opus plugin is not available, forward error correction is not tested" );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceReceiver: The statistics carry what RTCP needs: loss, fraction lost per interval, jitter; and the drops of the network side" )
    try
    {
        SLoopConfig config;
        config.useEngine = true;
        config.lossEvery = 10;
        config.jitterInUs = 4000.0;
        config.transitInUs = 3000.0;
        config.profile = AUDIO::VOICEPLAYOUTPROFILE_WIRELESS;
        CVoiceLoop loop( config );
        ASSERT_TRUE( loop.Init() );
        loop.Run( 5.0 );
        AUDIO::CVoiceReceiverStats stats;
        loop.GetStats( stats );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceReceiver: statistics: expected " << stats.framesExpected << ", received " << stats.framesReceived << ", lost " << stats.cumulativeLost <<
                   ", fraction " << (UInt32) stats.fractionLost << "/256, jitter " << stats.interarrivalJitter << " units = " << stats.interarrivalJitterInMs << " ms, highest " << stats.extendedHighestSequenceNumber );
        ASSERT_TRUE( stats.framesExpected == stats.framesReceived + (UInt64) stats.cumulativeLost );
        ASSERT_TRUE( stats.cumulativeLost >= (Int64) loop.framesLost - 1 && stats.cumulativeLost <= (Int64) loop.framesLost );
        ASSERT_TRUE( stats.fractionLost >= 23 && stats.fractionLost <= 28 );                    // 1 in 10 is 25.6 / 256
        ASSERT_TRUE( stats.interarrivalJitterInMs > 0.3 && stats.interarrivalJitterInMs < 4.0 );
        ASSERT_TRUE( stats.extendedHighestSequenceNumber < 65536 * 2 );
        AUDIO::CVoiceReceiverStats again;
        loop.GetStats( again );
        ASSERT_TRUE( 0 == again.fractionLost );                                                 // nothing new since the previous call

        // frames of another stream and of another payload type are dropped and counted
        AUDIO::CVoiceFrame other;
        ASSERT_TRUE( other.SetPayload( (const UInt8*) "\0\0\0\0", 4 ) );
        other.SetStreamId( StreamId + 1 );
        other.SetPayloadType( 112 );
        ASSERT_FALSE( loop.receiver->PushFrame( other, loop.clock.GetTimeInUs() ) );
        other.SetStreamId( StreamId );
        other.SetPayloadType( 96 );
        ASSERT_FALSE( loop.receiver->PushFrame( other, loop.clock.GetTimeInUs() ) );
        // a full inbound queue (32 frames)
        other.SetPayloadType( 112 );
        UInt32 accepted = 0;
        for ( UInt32 i=0; i<40; ++i )
            accepted += loop.receiver->PushFrame( other, loop.clock.GetTimeInUs() ) ? 1 : 0;
        loop.receiver->GetStats( stats );
        ASSERT_TRUE( 1 == stats.wrongStream && 1 == stats.wrongPayloadType && 32 == accepted && 8 == stats.inboundQueueFull );
        ASSERT_TRUE( stats.framesPushed == stats.framesQueued + 10 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceReceiver: Clock drift engages the resampler: bit for bit before, no step at the switch, the drift found, no underruns" )
    try
    {
        const Float64 drifts[ 2 ] = { 300.0, -300.0 };
        for ( int c=0; c<2; ++c )
        {
            SLoopConfig config;
            config.driftPpm = drifts[ c ];
            config.profile = AUDIO::VOICEPLAYOUTPROFILE_WIRED;
            config.fixedMargin = true;
            CVoiceLoop loop( config );
            ASSERT_TRUE( loop.Init() );
            loop.Run( 120.0 );
            AUDIO::CVoiceReceiverStats stats;
            loop.GetStats( stats );
            ASSERT_TRUE( loop.switched );
            const size_t delay = loop.FirstSound();
            const size_t matching = loop.MatchingSamples( delay, loop.output.size() );
            // the step at the switch against the largest step of the signal itself
            const Float32 signalStep = MaxStep( MakePcmSignal( 48000, 48000 ), 0, 48000 );
            const size_t from = loop.switchedAt > 480 ? loop.switchedAt - 480 : 0;
            const Float32 switchStep = MaxStep( loop.output, from, loop.switchedAt + 960 );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceReceiver: drift " << drifts[ c ] << " ppm: resampler from sample " << loop.switchedAt << " (" << (Float64) loop.switchedAt / 48000.0 <<
                       " s), bit-exact before it " << matching << " samples from " << delay << ", largest step there " << switchStep << " (signal " << signalStep << "), integral " << stats.integralInPpm <<
                       " ppm, Lmin " << stats.minimumLevelInMs << " ms, margin " << stats.marginInMs << " ms, underruns " << stats.underruns << ", concealed " << stats.framesConcealed );
            ASSERT_TRUE( delay + matching >= loop.switchedAt );
            ASSERT_TRUE( switchStep <= signalStep * 1.02f );
            ASSERT_TRUE( stats.resampling && 1 == stats.resamplerSwitches && 0 == stats.outputShortfalls );
            ASSERT_TRUE( 0 == stats.underruns && 0 == stats.framesConcealed && 0 == stats.lateFrames );
            ASSERT_TRUE( fabs( stats.integralInPpm - drifts[ c ] ) < 10.0 && fabs( stats.minimumLevelInMs - stats.marginInMs ) < 0.5 );
            ASSERT_TRUE( IsFinite( loop.output ) && MaxStep( loop.output, delay + 100, loop.output.size() ) <= signalStep * 1.02f );
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
PerformStreamTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceReceiver: Reset() starts over, a restarted sender is followed, another stream needs a Reset() to be accepted" )
    try
    {
        SLoopConfig config;
        config.useEngine = true;
        CVoiceLoop loop( config );
        ASSERT_TRUE( loop.Init() );
        loop.Run( 2.0 );
        loop.receiver->Reset();
        loop.Run( 2.0 );
        AUDIO::CVoiceReceiverStats stats;
        loop.GetStats( stats );
        ASSERT_TRUE( 2 == stats.resets && AUDIO::VOICEPLAYOUTSTATE_PLAYING == stats.state && 1 == stats.talkspurts );     // Start() and Reset()
        ASSERT_TRUE( stats.framesDecoded > 150 && 0 == stats.wrongStream );

        // the sender starts over with new numbers (the same stream id): the jitter buffer restarts the stream
        AUDIO::CVoiceSenderConfig senderConfig = loop.SenderConfig();
        senderConfig.SetFirstSequenceNumber( 40000 );
        senderConfig.ClearFirstTimestamp();
        ASSERT_TRUE( loop.ReplaceSender( senderConfig ) );
        loop.Run( 2.0 );
        loop.GetStats( stats );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceReceiver: restarted sender: restarts " << stats.restarts << ", talkspurts " << stats.talkspurts << ", decoded " << stats.framesDecoded << ", state " << (UInt32) stats.state );
        ASSERT_TRUE( stats.restarts >= 1 && AUDIO::VOICEPLAYOUTSTATE_PLAYING == stats.state );

        // a sender with another stream id: dropped until Reset()
        senderConfig.SetStreamId( StreamId + 7 );
        ASSERT_TRUE( loop.ReplaceSender( senderConfig ) );
        loop.Run( 1.0 );
        loop.GetStats( stats );
        ASSERT_TRUE( stats.wrongStream > 90 && StreamId == stats.streamId && !loop.receiver->IsPlaying() );
        loop.receiver->Reset();
        loop.Run( 1.0 );
        loop.GetStats( stats );
        ASSERT_TRUE( StreamId + 7 == stats.streamId && loop.receiver->IsPlaying() && 3 == stats.resets );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceReceiver: Receivers are added, stopped and started again while the engine plays; each plays its own stream into the mix" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( PlaybackParams( 48000, 240 ) ) );
        AUDIO::CNullAudioDevice* device = engine.GetDevice()->AsNullDevice();
        CSimulatedVoiceClock clock;
        const UInt32 count = 4;
        AUDIO::CVoiceReceiver* receivers[ count ];
        TFrames streams[ count ];
        for ( UInt32 r=0; r<count; ++r )
        {
            MakeStream( 100 + r, 300, streams[ r ] );
            receivers[ r ] = GUCEF_NEW AUDIO::CVoiceReceiver( engine, ReceiverConfig( PcmConfig( 48000 ), AUDIO::VOICEPLAYOUTPROFILE_WIRED ) );
            receivers[ r ]->SetClock( clock );
            ASSERT_TRUE( receivers[ r ]->Prepare() );
        }
        AUDIO::CAudioBus* bus = engine.CreateBus( "intercom" );
        ASSERT_TRUE( GUCEF_NULL != bus );

        const CORE::CFutureResult none( CORE::TASKSTATUS_SETUP_FAILED );
        GUCEF::vector< CORE::CFutureResult > starts( count, none );
        CORE::CFutureResult stop( none );
        CORE::CFutureResult restart( none );
        UInt64 decodedAtStop = 0;
        for ( UInt32 step=0; step<600; ++step )
        {
            // receiver r starts at step 50 r; receiver 1 stops at 250 and starts again at 400
            for ( UInt32 r=0; r<count; ++r )
                if ( step == 50 * r )
                    starts[ r ] = receivers[ r ]->Start( bus );
            if ( 250 == step )
            {
                stop = receivers[ 1 ]->Stop();
                ASSERT_TRUE( receivers[ 1 ]->Start( bus ).HasNoFuture() );         // its voice has not ended yet
            }
            if ( 400 == step )
                restart = receivers[ 1 ]->Start( bus );

            // a frame of each stream every second step (10 ms frames, 5 ms steps)
            if ( 0 == step % 2 )
                for ( UInt32 r=0; r<count; ++r )
                    receivers[ r ]->PushFrame( streams[ r ][ ( step / 2 ) % 300 ], clock.GetTimeInUs() );
            clock.Advance( 5000 );
            device->Render( 240 );
            engine.Update();
            device->ClearRenderedAudio();

            if ( 260 == step )
            {
                AUDIO::CVoiceReceiverStats stats;
                receivers[ 1 ]->GetStats( stats );
                decodedAtStop = stats.framesDecoded;
            }
            if ( 390 == step )
            {
                AUDIO::CVoiceReceiverStats stats;
                receivers[ 1 ]->GetStats( stats );
                ASSERT_TRUE( stats.framesDecoded == decodedAtStop );                // stopped: nobody reads its source
                ASSERT_TRUE( 3 == engine.GetActiveVoiceCount() );
            }
        }
        ASSERT_TRUE( stop.IsReady() && !restart.HasNoFuture() && restart.IsReady() );
        ASSERT_TRUE( 4 == engine.GetActiveVoiceCount() );
        for ( UInt32 r=0; r<count; ++r )
        {
            AUDIO::CVoiceReceiverStats stats;
            receivers[ r ]->GetStats( stats );
            ASSERT_TRUE( starts[ r ].IsReady() && receivers[ r ]->IsPlaying() && 100 + r == stats.streamId );
            ASSERT_TRUE( stats.framesDecoded > 50 && 0 == stats.wrongStream );
        }
        // the mix holds the voices: louder than one stream alone
        device->Render( 480 );
        const AUDIO::CAudioBuffer& rendered = device->GetRenderedAudio();
        TSamples mix( static_cast< const Float32* >( rendered.GetDataPtr( 0 ) ), static_cast< const Float32* >( rendered.GetDataPtr( 0 ) ) + rendered.GetFrameCount() );
        ASSERT_TRUE( 480 == mix.size() && VoiceCodecTest::Rms( mix, 0, mix.size() ) > 1.5 * VoiceCodecTest::Rms( MakePcmSignal( 48000, 48000 ), 0, 48000 ) );

        for ( UInt32 r=0; r<count; ++r )
        {
            CORE::CFutureResult stopped = receivers[ r ]->Stop();
            device->Render( 240 );
            engine.Update();
            ASSERT_TRUE( stopped.IsReady() );
            GUCEF_DELETE receivers[ r ];
        }
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformThreadTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceReceiver: A network thread pushes, the audio thread plays and the owner starts, stops, resets and reads receivers (TSan)" )
    try
    {
        AUDIO::CAudioDriverRegistry& drivers = AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry();
        drivers.TryRegister( "receivertest", AUDIO::TAudioDriverPtr( GUCEF_NEW CPacedTestDriver() ) );
        AUDIO::CAudioEngineParams params;
        params.SetDriverName( "receivertest" );
        params.SetThreadingMode( AUDIO::AUDIOTHREADING_THREADED );
        params.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( params ) );

        const UInt32 count = 3;
        AUDIO::CVoiceReceiver* receivers[ count ];
        TFrames streams[ count ];
        for ( UInt32 r=0; r<count; ++r )
        {
            MakeStream( 200 + r, 100, streams[ r ] );
            receivers[ r ] = GUCEF_NEW AUDIO::CVoiceReceiver( engine, ReceiverConfig( PcmConfig( 48000 ), AUDIO::VOICEPLAYOUTPROFILE_WIRELESS ) );
            ASSERT_TRUE( receivers[ r ]->Prepare() );
            ASSERT_TRUE( engine.WaitFor( receivers[ r ]->Start(), 10000 ) );
        }
        CPushThread* network = GUCEF_NEW CPushThread( receivers, streams, count );
        ASSERT_TRUE( network->Activate() );
        UInt32 playingSeen = 0;
        for ( UInt32 round=0; round<300; ++round )
        {
            AUDIO::CVoiceReceiver* receiver = receivers[ round % count ];
            AUDIO::CVoiceReceiverStats stats;
            receiver->GetStats( stats );
            playingSeen += receiver->IsPlaying() ? 1 : 0;
            receiver->GetLevelInDecibels();
            receiver->SetVolume( 0.5f + 0.1f * (Float32) ( round % 5 ) );
            if ( 17 == round % 40 )
                receiver->Reset();
            if ( 29 == round % 60 )
            {
                ASSERT_TRUE( engine.WaitFor( receiver->Stop(), 10000 ) );
                ASSERT_TRUE( engine.WaitFor( receiver->Start(), 10000 ) );
            }
            MT::PrecisionDelay( 2 );
        }
        network->stop.Store( 1 );
        network->WaitForThreadToFinish( 10000 );
        UInt64 decoded = 0;
        for ( UInt32 r=0; r<count; ++r )
        {
            AUDIO::CVoiceReceiverStats stats;
            receivers[ r ]->GetStats( stats );
            decoded += stats.framesDecoded;
            ASSERT_TRUE( engine.WaitFor( receivers[ r ]->Stop(), 10000 ) );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceReceiver: threads: " << network->pushed.Load() << " frames pushed, playing seen " << playingSeen << " times, " << decoded << " frames decoded in the last runs" );
        ASSERT_TRUE( network->pushed.Load() > 100 && playingSeen > 0 );
        for ( UInt32 r=0; r<count; ++r )
            GUCEF_DELETE receivers[ r ];
        GUCEF_DELETE network;
        engine.Shutdown();
        drivers.Unregister( "receivertest" );
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

    GUCEF_TESTFW_TESTCASE( "AudioVoiceReceiver: The playout allocates nothing and takes no lock: loss, drift, the resampler, resets, a full queue, Opus (RealtimeSanitizer build)" )
    if ( !IsRtsanBuild )
    {
        GUCEF_TESTFW_SKIP( "not a RealtimeSanitizer build (-fsanitize=realtime)" );
    }
    GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_RTSAN_CLEAN" ) == "0", "switched off with GUCEF_TEST_AUDIO_RTSAN_CLEAN=0" );
    try
    {
        // ReadFrames() is a real time context: every render request below is checked, and so is the audio thread of the engine
        const bool opus = LoadOpus();
        for ( int c=0; c<( opus ? 3 : 2 ); ++c )
        {
            SLoopConfig config;
            config.lossEvery = 9;
            config.driftPpm = 400.0;
            config.jitterInUs = 6000.0;
            config.randomRequests = 0 == c;
            config.useEngine = 1 == c;
            config.redundancy = 1;
            config.codec = PcmConfig( 16000 );
            config.period = 80;
            if ( 2 == c )
            {
                config.codec = VoiceCodecTest::MakeConfig( "opus", 48000, 20000 );
                config.codec.SetProfileDefaults( AUDIO::VOICECODECPROFILE_ROBUST );
                config.period = 240;
                config.redundancy = 0;
            }
            CVoiceLoop loop( config );
            ASSERT_TRUE( loop.Init() );
            loop.Run( 4.0 );
            if ( GUCEF_NULL != loop.receiver )
                loop.receiver->Reset();
            else
                loop.source->RequestReset();
            loop.Run( 2.0 );
            AUDIO::CVoiceFrame frame;
            ASSERT_TRUE( frame.SetPayload( (const UInt8*) "\0\0\0\0", 4 ) );
            frame.SetStreamId( StreamId );
            frame.SetPayloadType( config.codec.GetPayloadType() );
            for ( UInt32 i=0; i<40; ++i )
                loop.source->PushFrame( frame, loop.clock.GetTimeInUs() );
            loop.Run( 1.0 );
            AUDIO::CVoiceReceiverStats stats;
            loop.GetStats( stats );
            ASSERT_TRUE( stats.resampling && stats.framesDecoded > 100 && stats.resets >= 1 && stats.inboundQueueFull > 0 );
        }
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
PerformAudioVoiceReceiverTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioVoiceReceiver TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.VoiceReceiver" );

    PerformConfigTests();
    PerformPlayoutTests();
    PerformStreamTests();
    PerformThreadTests();
    PerformRealtimeTests();
}

/*-------------------------------------------------------------------------*/
