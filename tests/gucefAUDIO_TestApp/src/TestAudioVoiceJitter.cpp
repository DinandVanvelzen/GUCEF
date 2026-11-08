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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_AUDIO_CSINCRESAMPLER_H
#include "gucefAUDIO_CSincResampler.h"
#define GUCEF_AUDIO_CSINCRESAMPLER_H
#endif /* GUCEF_AUDIO_CSINCRESAMPLER_H ? */

#ifndef GUCEF_AUDIO_CVOICEFRAME_H
#include "gucefAUDIO_CVoiceFrame.h"
#define GUCEF_AUDIO_CVOICEFRAME_H
#endif /* GUCEF_AUDIO_CVOICEFRAME_H ? */

#ifndef GUCEF_AUDIO_CVOICEJITTERBUFFER_H
#include "gucefAUDIO_CVoiceJitterBuffer.h"
#define GUCEF_AUDIO_CVOICEJITTERBUFFER_H
#endif /* GUCEF_AUDIO_CVOICEJITTERBUFFER_H ? */

#ifndef GUCEF_AUDIO_CVOICEPLAYOUTCONTROLLER_H
#include "gucefAUDIO_CVoicePlayoutController.h"
#define GUCEF_AUDIO_CVOICEPLAYOUTCONTROLLER_H
#endif /* GUCEF_AUDIO_CVOICEPLAYOUTCONTROLLER_H ? */

#ifndef GUCEF_AUDIO_CSLIDINGQUANTILE_H
#include "gucefAUDIO_CSlidingQuantile.h"
#define GUCEF_AUDIO_CSLIDINGQUANTILE_H
#endif /* GUCEF_AUDIO_CSLIDINGQUANTILE_H ? */

#include "TestAudioVoiceCodecSupport.h"
#include "TestAudioVoiceJitter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef AUDIO::CVoiceJitterBuffer TJitterBuffer;
typedef AUDIO::CVoicePlayoutController TController;

const UInt32 Rate = 48000;
const UInt32 FrameSize = 480;           /**< 10 ms */
const UInt64 FrameUs = 10000;
const UInt32 BlockFrames = 240;         /**< 5 ms render requests */
const UInt64 BlockUs = 5000;
const UInt32 RampPeriod = 1 << 21;      /**< the marker decoder's ramp wraps every 2^21 samples */
const UInt32 Lookahead = 16;            /**< CSincResampler::HALF_WIDTH: the input a resampled request needs beyond its output */
const Float64 ResampledBlockUs = (Float64) ( BlockFrames + Lookahead ) * 1000000.0 / (Float64) Rate;

#ifdef GUCEF_AUDIO_VOICETEST_RTSAN_BUILD
const bool IsRtsanBuild = true;
#else
const bool IsRtsanBuild = false;
#endif

/*-------------------------------------------------------------------------*/

inline Float32
Ramp( UInt64 mediaSample )
{
    return 0.25f + 0.5f * (Float32) ( mediaSample & ( RampPeriod - 1 ) ) / (Float32) RampPeriod;
}

/*-------------------------------------------------------------------------*/

/** Payload of the marker codec: the frame index in 4 bytes, big endian, and a flag byte (1 = the frame is silence) */
inline void
WriteMarkerPayload( AUDIO::CVoiceFrame& frame, UInt64 frameIndex, bool silent )
{
    UInt8* payload = frame.GetPayloadBuffer();
    payload[ 0 ] = (UInt8) ( ( frameIndex >> 24 ) & 0xFF );
    payload[ 1 ] = (UInt8) ( ( frameIndex >> 16 ) & 0xFF );
    payload[ 2 ] = (UInt8) ( ( frameIndex >> 8 ) & 0xFF );
    payload[ 3 ] = (UInt8) ( frameIndex & 0xFF );
    payload[ 4 ] = silent ? 1 : 0;
    frame.SetPayloadSize( 5 );
}

inline UInt32
ReadMarkerIndex( const UInt8* payload )
{
    return ( (UInt32) payload[ 0 ] << 24 ) | ( (UInt32) payload[ 1 ] << 16 ) | ( (UInt32) payload[ 2 ] << 8 ) | (UInt32) payload[ 3 ];
}

/*-------------------------------------------------------------------------*/

/**
 *  A decoder for the tests: a frame decodes to a ramp that encodes its media sample position (or to silence), so
 *  the output tells which frame and sample plays; concealment is silence; forward error correction (when switched on)
 *  rebuilds the frame before the given one. Real time safe like a real decoder.
 */
class CMarkerVoiceDecoder : public AUDIO::CVoiceDecoder
{
    public:

    CMarkerVoiceDecoder( bool fec )
        : AUDIO::CVoiceDecoder()
        , m_config( VoiceCodecTest::MakeConfig( "marker", Rate, 10000 ) )
        , m_fec( fec )
        , decoded( 0 )
        , concealed( 0 )
        , recovered( 0 )
        , resets( 0 )
    {}

    virtual const AUDIO::CVoiceCodecConfig& GetConfig( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_config; }
    virtual UInt32 GetOutputRate( void ) const GUCEF_VIRTUAL_OVERRIDE { return Rate; }

    virtual Int32 Decode( const UInt8* payload, UInt32 size, Float32* pcm, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( 5 != size || frameCapacity < FrameSize )
            return -1;
        ++decoded;
        Write( ReadMarkerIndex( payload ), 0 != payload[ 4 ], pcm );
        return (Int32) FrameSize;
    }

    virtual Int32 Conceal( Float32* pcm, UInt32 frames ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++concealed;
        memset( pcm, 0, frames * sizeof( Float32 ) );
        return (Int32) frames;
    }

    virtual bool SupportsFec( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_fec; }

    virtual Int32 DecodeFec( const UInt8* nextPayload, UInt32 size, Float32* pcm, UInt32 frames ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !m_fec || 5 != size || frames != FrameSize )
            return -1;
        const UInt32 next = ReadMarkerIndex( nextPayload );
        if ( 0 == next )
            return -1;
        ++recovered;
        Write( next - 1, false, pcm );
        return (Int32) FrameSize;
    }

    virtual void Reset( void ) GUCEF_VIRTUAL_OVERRIDE { ++resets; }

    UInt64 decoded;
    UInt64 concealed;
    UInt64 recovered;
    UInt64 resets;

    private:

    void Write( UInt64 index, bool silent, Float32* pcm )
    {
        for ( UInt32 i=0; i<FrameSize; ++i )
            pcm[ i ] = silent ? 0.0f : Ramp( index * FrameSize + i );
    }

    AUDIO::CVoiceCodecConfig m_config;
    bool m_fec;
};

/*-------------------------------------------------------------------------*/

/** The PCM test signal, exactly representable in 16 bits: two tones that never fall silent for a frame */
inline Float32
PcmSignal( UInt64 mediaSample )
{
    const Float64 t = (Float64) mediaSample / (Float64) Rate;
    const Float64 v = 9000.0 * sin( 2.0 * AudioEffectTest::Pi * 211.0 * t ) + 5000.0 * sin( 2.0 * AudioEffectTest::Pi * 1033.0 * t ) + 3000.0;
    return (Float32) floor( v + 0.5 ) / 32768.0f;
}

/*-------------------------------------------------------------------------*/

enum EConsumption
{
    CONSUME_DIRECT = 0 ,    /**< the output is the decoded audio as it is (only while the ratio is exactly 1) */
    CONSUME_SINC ,          /**< through CSincResampler at the controller's ratio */
    CONSUME_VIRTUAL         /**< the consumption of a resampler at the ratio is counted, no audio is made (long runs) */
};

enum EPayloadKind
{
    PAYLOAD_MARKER = 0 ,
    PAYLOAD_PCM
};

/** The simulated network between sender and receiver */
struct SLinkConfig
{
    Float64 baseDelayUs;
    Float64 uniformJitterUs;            /**< plus U( 0, this ) */
    Float64 paretoProbability;          /**< with this probability a frame gets a Pareto distributed extra delay */
    Float64 paretoScaleUs;
    Float64 paretoShape;
    Float64 spikeProbability;           /**< per frame: a stall starts, frames sent during it arrive together at its end */
    Float64 spikeDurationUs;
    Float64 randomLoss;
    Float64 geGoodToBad;                /**< Gilbert-Elliott: everything is lost in the bad state */
    Float64 geBadToGood;
    Float64 duplicateProbability;
    UInt32 seed;

    SLinkConfig( void )
        : baseDelayUs( 1000.0 ), uniformJitterUs( 0.0 ), paretoProbability( 0.0 ), paretoScaleUs( 0.0 ), paretoShape( 2.0 )
        , spikeProbability( 0.0 ), spikeDurationUs( 0.0 ), randomLoss( 0.0 ), geGoodToBad( 0.0 ), geBadToGood( 1.0 )
        , duplicateProbability( 0.0 ), seed( 0x1234567 )
    {}
};

struct SSimConfig
{
    TController::SSettings settings;
    EConsumption consumption;
    EPayloadKind payload;
    bool fec;
    bool redundancy;
    Float64 driftPpm;                   /**< the sender's clock is this much faster than the receiver's */
    UInt16 firstSequenceNumber;
    UInt32 firstTimestamp;
    UInt32 speakFrames;                 /**< talkspurts of this many frames ... */
    UInt32 silentFrames;                /**< ... and gaps of this many (0: continuous) */
    UInt32 silentContentPeriod;         /**< frames with ( k mod period ) >= period - length carry silence (0: none) */
    UInt32 silentContentLength;
    UInt32 slotCount;
    bool recordOutput;
    bool recordProduced;
    SLinkConfig link;
    GUCEF::map< UInt64, Int64 > extraDelayUs;   /**< per frame index */
    GUCEF::set< UInt64 > lostFrames;            /**< per frame index */
    UInt64 restartAtFrame;                      /**< the sender restarts its numbering here (0: never) */
    UInt64 delayChangeAtFrame;                  /**< the base delay changes here (0: never) ... */
    Float64 delayAfterChangeUs;                 /**< ... to this */

    SSimConfig( void )
        : settings(), consumption( CONSUME_VIRTUAL ), payload( PAYLOAD_MARKER ), fec( false ), redundancy( false ), driftPpm( 0.0 )
        , firstSequenceNumber( 1000 ), firstTimestamp( 123456 ), speakFrames( 0 ), silentFrames( 0 ), silentContentPeriod( 0 )
        , silentContentLength( 0 ), slotCount( 64 ), recordOutput( false ), recordProduced( false ), link(), extraDelayUs(), lostFrames()
        , restartAtFrame( 0 ), delayChangeAtFrame( 0 ), delayAfterChangeUs( 0.0 )
    {}
};

/*-------------------------------------------------------------------------*/

/** A piece of decoded audio in the stream the controller produced */
struct SChunk
{
    Float64 start;
    UInt32 length;
    UInt32 kind;
    UInt64 timestamp;
    UInt64 arrivalUs;
};

/** One produced piece, for the logs of the tests */
struct SProduced
{
    UInt32 kind;
    UInt64 timestamp;
    UInt64 sequenceNumber;
    Float64 outputTimeUs;
    Float64 delayUs;            /**< output of the first sample minus arrival, for decoded frames */
    Float64 sendDelayUs;        /**< output of the first sample minus the time the sender had the frame complete, for decoded frames */
};

const UInt32 MaxArrivalsPerStep = 64;
const UInt32 ChunkCapacity = 64;
const UInt32 MaxProducedPerStep = 16;

/**
 *  Everything one render request needs, preallocated: the step runs as a real time context in a RealtimeSanitizer
 *  build, so the whole playout path of every simulation in this file is checked for allocation, locks and blocking
 */
struct SPlayoutStep
{
    TController* controller;
    AUDIO::CSincResampler* resampler;
    EConsumption consumption;
    UInt32 lookahead;

    UInt64 nowUs;
    const AUDIO::CVoiceFrame* arrivals;
    const UInt64* arrivalTimes;
    UInt32 arrivalCount;

    Float32* fifo;
    UInt32 fifoCapacity;
    UInt32 fifoCount;
    Float32* scratch;
    UInt32 scratchCapacity;
    Float32* output;
    Float64 produced;           /**< stream position after the last produced sample */
    Float64 position;           /**< stream position of the next output sample */
    SChunk* chunks;
    UInt32 chunkHead;
    UInt32 chunkCount;

    // results of the step
    Float64 ratio;
    bool hasDelay;
    Float64 delayUs;
    bool ratioNotOne;
    bool fifoOverflow;
    UInt32 insertResults[ 8 ];
    SProduced producedLog[ MaxProducedPerStep ];
    UInt32 producedCount;
};

/*-------------------------------------------------------------------------*/

void
AddChunk( SPlayoutStep& s, UInt32 length, const TController::SProduceInfo& info ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    // forget chunks that were output long ago
    while ( s.chunkCount > 0 )
    {
        const SChunk& oldest = s.chunks[ s.chunkHead ];
        if ( oldest.start + (Float64) oldest.length + 4.0 * FrameSize < s.position || s.chunkCount == ChunkCapacity )
        {
            s.chunkHead = ( s.chunkHead + 1 ) % ChunkCapacity;
            --s.chunkCount;
        }
        else
            break;
    }
    SChunk& chunk = s.chunks[ ( s.chunkHead + s.chunkCount ) % ChunkCapacity ];
    chunk.start = s.produced;
    chunk.length = length;
    chunk.kind = (UInt32) info.kind;
    chunk.timestamp = info.extendedTimestamp;
    chunk.arrivalUs = info.arrivalTimeInUs;
    ++s.chunkCount;
}

/*-------------------------------------------------------------------------*/

bool
ProduceMore( SPlayoutStep& s, UInt32 deficit ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    TController::SProduceInfo info;
    UInt32 offer = deficit < s.scratchCapacity ? deficit : s.scratchCapacity;
    if ( AUDIO::VOICEPLAYOUTSTATE_PLAYING == s.controller->GetState() || offer < 1 )
        offer = FrameSize;
    UInt32 n = s.controller->Produce( s.scratch, offer, &info );
    if ( 0 == n )
        n = s.controller->Produce( s.scratch, FrameSize > offer ? FrameSize : offer, &info );
    if ( 0 == n )
        return false;

    if ( s.producedCount < MaxProducedPerStep )
    {
        SProduced& log = s.producedLog[ s.producedCount++ ];
        log.kind = (UInt32) info.kind;
        log.timestamp = info.extendedTimestamp;
        log.sequenceNumber = info.extendedSequenceNumber;
        log.outputTimeUs = (Float64) s.nowUs + ( s.produced - s.position ) * 1000000.0 / (Float64) Rate / s.ratio;
        log.delayUs = log.outputTimeUs - (Float64) info.arrivalTimeInUs;
        log.sendDelayUs = log.outputTimeUs - (Float64) info.captureTickCount;
    }
    AddChunk( s, n, info );
    if ( CONSUME_VIRTUAL != s.consumption )
    {
        if ( s.fifoCount + n > s.fifoCapacity )
        {
            s.fifoOverflow = true;
            return false;
        }
        memcpy( s.fifo + s.fifoCount, s.scratch, n * sizeof( Float32 ) );
        s.fifoCount += n;
    }
    s.produced += (Float64) n;
    return true;
}

/*-------------------------------------------------------------------------*/

void
DropFromFifo( SPlayoutStep& s, UInt32 frames ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    if ( frames >= s.fifoCount )
    {
        s.fifoCount = 0;
        return;
    }
    memmove( s.fifo, s.fifo + frames, ( s.fifoCount - frames ) * sizeof( Float32 ) );
    s.fifoCount -= frames;
}

/*-------------------------------------------------------------------------*/

/** One render request of the playout source as step 13 will make it: real time context in a RealtimeSanitizer build */
void
RunPlayoutStep( SPlayoutStep& s ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    s.hasDelay = false;
    s.producedCount = 0;
    for ( UInt32 i=0; i<8; ++i )
        s.insertResults[ i ] = 0;

    const Float64 pendingFloat = s.produced - s.position;
    const UInt32 pending = pendingFloat > 0.0 ? (UInt32) floor( pendingFloat ) : 0;
    s.controller->BeginRequest( s.nowUs, BlockFrames + s.lookahead, pending );
    for ( UInt32 i=0; i<s.arrivalCount; ++i )
    {
        const TJitterBuffer::EInsertResult result = s.controller->Insert( s.arrivals[ i ], s.arrivalTimes[ i ] );
        ++s.insertResults[ (UInt32) result & 7 ];
    }
    s.ratio = s.controller->UpdateRatio();
    if ( CONSUME_DIRECT == s.consumption && 1.0 != s.ratio )
        s.ratioNotOne = true;

    // what plays at the start of this request
    for ( UInt32 c=0; c<s.chunkCount; ++c )
    {
        const SChunk& chunk = s.chunks[ ( s.chunkHead + c ) % ChunkCapacity ];
        if ( s.position >= chunk.start && s.position < chunk.start + (Float64) chunk.length )
        {
            if ( (UInt32) TController::PRODUCED_DECODED == chunk.kind )
            {
                s.hasDelay = true;
                s.delayUs = (Float64) s.nowUs - ( s.position - chunk.start ) * 1000000.0 / (Float64) Rate / s.ratio - (Float64) chunk.arrivalUs;
            }
            break;
        }
    }

    const Float64 step = CONSUME_DIRECT == s.consumption ? 1.0 : s.ratio;
    const Float64 target = s.position + (Float64) BlockFrames * step + (Float64) s.lookahead;
    while ( s.produced < target - 1.0e-9 )
    {
        if ( !ProduceMore( s, (UInt32) ceil( target - s.produced ) ) )
            break;
    }

    if ( CONSUME_DIRECT == s.consumption )
    {
        memcpy( s.output, s.fifo, BlockFrames * sizeof( Float32 ) );
        DropFromFifo( s, BlockFrames );
        s.position += (Float64) BlockFrames;
    }
    else
    if ( CONSUME_SINC == s.consumption )
    {
        s.resampler->SetRates( s.ratio * (Float64) Rate, (Float64) Rate );
        UInt32 made = 0;
        UInt32 guard = 0;
        while ( made < BlockFrames && guard++ < 8 )
        {
            UInt32 consumed = 0, producedNow = 0;
            s.resampler->Process( s.fifo, s.fifoCount, consumed, s.output + made, BlockFrames - made, producedNow );
            DropFromFifo( s, consumed );
            made += producedNow;
            if ( made < BlockFrames && !ProduceMore( s, FrameSize ) )
                break;
        }
        s.position += (Float64) BlockFrames * s.ratio;
    }
    else
    {
        s.position += (Float64) BlockFrames * s.ratio;
    }
}

/*-------------------------------------------------------------------------*/

struct SArrivalEvent
{
    UInt64 arrivalUs;
    UInt64 order;
    UInt64 frameIndex;
    UInt16 sequenceNumber;
    UInt32 timestamp;
    bool marker;

    bool operator<( const SArrivalEvent& other ) const
    {
        // a max-heap on 'later', so the earliest is at the front
        if ( arrivalUs != other.arrivalUs )
            return arrivalUs > other.arrivalUs;
        return order > other.order;
    }
};

/** A sample of the control values, once per simulated second */
struct SSecond
{
    Float64 timeInS;
    bool hasMinimum;
    Float64 minimumMs;
    Float64 marginMs;
    Float64 levelMs;
    Float64 ratio;
    Float64 integralPpm;
    bool hasDelay;
    Float64 delayMs;
    UInt64 concealed;
    UInt64 late;
    UInt64 underruns;
};

/*-------------------------------------------------------------------------*/

/**
 *  A sender, a simulated network and the receiving playout (jitter buffer, controller, decoder, FIFO and resampler)
 *  in one simulated clock. The receiver makes a render request of 5 ms every 5 ms of receiver time; the sender makes
 *  a 10 ms frame every 10 ms of its own clock, which runs 'driftPpm' faster.
 */
class CPlayoutSim
{
    public:

    explicit CPlayoutSim( const SSimConfig& config )
        : m_config( config )
        , m_buffer()
        , m_controller()
        , m_marker( config.fec )
        , m_pcmDecoder()
        , m_pcmEncoder()
        , m_resampler()
        , m_ready( false )
        , m_random( config.link.seed )
        , m_events()
        , m_order( 0 )
        , m_nextFrame( 0 )
        , m_sentCount( 0 )
        , m_seqOffset( 0 )
        , m_tsOffset( 0 )
        , m_geBad( false )
        , m_spikeEndUs( 0.0 )
        , m_lastSentFrame( (UInt64) -1 )
        , m_previousPayloadSize( 0 )
        , m_step()
        , m_arrivalFrames( MaxArrivalsPerStep )
        , m_arrivalTimes( MaxArrivalsPerStep, 0 )
        , m_fifo( 8 * FrameSize, 0.0f )
        , m_scratch( 4 * FrameSize, 0.0f )
        , m_block( BlockFrames, 0.0f )
        , m_chunks( ChunkCapacity )
        , m_stepIndex( 0 )
        , framesSent( 0 )
        , framesLost( 0 )
        , framesDuplicated( 0 )
        , ratioMin( 1.0 )
        , ratioMax( 1.0 )
        , output()
        , produced()
        , seconds()
        , delays()
        , lostFrames()
    {
        AUDIO::CVoiceDecoder* decoder = &m_marker;
        if ( PAYLOAD_PCM == config.payload )
        {
            const AUDIO::CVoiceCodecConfig codec = VoiceCodecTest::MakeConfig( "pcm", Rate, 10000 );
            AUDIO::CVoiceCodecRegistry& registry = AUDIO::CAudioGlobal::Instance()->GetVoiceCodecRegistry();
            if ( !registry.CreateEncoder( codec, m_pcmEncoder ) || !registry.CreateDecoder( codec, m_pcmDecoder ) )
                return;
            decoder = m_pcmDecoder.GetPointerAlways();
        }
        if ( !m_buffer.Prepare( config.slotCount, FrameSize, Rate ) )
            return;
        if ( !m_controller.Prepare( config.settings, m_buffer, *decoder ) )
            return;
        if ( CONSUME_SINC == config.consumption && !m_resampler.Configure( 1, (Float64) Rate, (Float64) Rate ) )
            return;

        memset( &m_step, 0, sizeof( m_step ) );
        m_step.controller = &m_controller;
        m_step.resampler = &m_resampler;
        m_step.consumption = config.consumption;
        m_step.lookahead = CONSUME_DIRECT == config.consumption ? 0 : Lookahead;
        m_step.arrivals = &m_arrivalFrames[ 0 ];
        m_step.arrivalTimes = &m_arrivalTimes[ 0 ];
        m_step.fifo = &m_fifo[ 0 ];
        m_step.fifoCapacity = (UInt32) m_fifo.size();
        m_step.scratch = &m_scratch[ 0 ];
        m_step.scratchCapacity = (UInt32) m_scratch.size();
        m_step.output = &m_block[ 0 ];
        m_step.chunks = &m_chunks[ 0 ];
        m_step.ratio = 1.0;
        m_ready = true;
    }

    bool IsReady( void ) const { return m_ready; }
    TController& Controller( void ) { return m_controller; }
    TJitterBuffer& Buffer( void ) { return m_buffer; }
    CMarkerVoiceDecoder& Marker( void ) { return m_marker; }
    UInt64 NowUs( void ) const { return m_stepIndex * BlockUs; }
    Float64 NowInS( void ) const { return (Float64) NowUs() / 1000000.0; }
    bool RatioWasNotOne( void ) const { return m_step.ratioNotOne; }
    bool FifoOverflowed( void ) const { return m_step.fifoOverflow; }
    bool HasDelay( void ) const { return m_step.hasDelay; }
    Float64 DelayUs( void ) const { return m_step.delayUs; }
    const UInt32* InsertResults( void ) const { return m_step.insertResults; }

    /** The time at which the sender has frame k complete (its capture time plus a frame), in receiver microseconds */
    Float64 SendTimeUs( UInt64 k ) const
    {
        return (Float64) ( k + 1 ) * (Float64) FrameUs / ( 1.0 + m_config.driftPpm * 1.0e-6 );
    }

    void ResetRatioTracking( void )
    {
        ratioMin = ratioMax = m_controller.GetRatio();
    }

    /** One render request */
    void Step( void )
    {
        const UInt64 now = NowUs();
        GenerateUntil( now );

        UInt32 count = 0;
        while ( !m_events.empty() && m_events.front().arrivalUs <= now && count < MaxArrivalsPerStep )
        {
            std::pop_heap( m_events.begin(), m_events.end() );
            const SArrivalEvent event = m_events.back();
            m_events.pop_back();
            BuildFrame( event, m_arrivalFrames[ count ] );
            m_arrivalTimes[ count ] = event.arrivalUs;
            ++count;
        }
        m_step.nowUs = now;
        m_step.arrivalCount = count;

        RunPlayoutStep( m_step );

        if ( m_step.ratio < ratioMin )
            ratioMin = m_step.ratio;
        if ( m_step.ratio > ratioMax )
            ratioMax = m_step.ratio;
        if ( m_config.recordOutput && CONSUME_VIRTUAL != m_config.consumption )
            output.insert( output.end(), m_block.begin(), m_block.end() );
        if ( m_config.recordProduced )
            for ( UInt32 i=0; i<m_step.producedCount; ++i )
                produced.push_back( m_step.producedLog[ i ] );

        ++m_stepIndex;
        if ( 0 == m_stepIndex % ( 1000000 / BlockUs ) )
            SampleSecond();
    }

    void RunUntil( Float64 seconds )
    {
        const UInt64 end = (UInt64) ( seconds * 1000000.0 / (Float64) BlockUs + 0.5 );
        while ( m_stepIndex < end )
            Step();
    }

    void SampleSecond( void )
    {
        TController::SStatistics stats;
        m_controller.GetStatistics( stats );
        SSecond second;
        second.timeInS = NowInS();
        second.hasMinimum = stats.hasMinimumLevel;
        second.minimumMs = stats.minimumLevelInS * 1000.0;
        second.marginMs = stats.marginInS * 1000.0;
        second.levelMs = stats.levelInS * 1000.0;
        second.ratio = stats.ratio;
        second.integralPpm = stats.integral * 1.0e6;
        second.hasDelay = m_step.hasDelay;
        second.delayMs = m_step.delayUs / 1000.0;
        second.concealed = stats.framesConcealed;
        second.late = stats.lateFrames;
        second.underruns = stats.underruns;
        seconds.push_back( second );
    }

    UInt64 framesSent;
    UInt64 framesLost;
    UInt64 framesDuplicated;
    Float64 ratioMin;
    Float64 ratioMax;
    GUCEF::vector< Float32 > output;
    GUCEF::vector< SProduced > produced;
    GUCEF::vector< SSecond > seconds;
    GUCEF::vector< Float64 > delays;
    GUCEF::vector< UInt64 > lostFrames;

    /** Lost frames after the first frame that came and before frame 'k' (what RFC 3550 A.3 can know of) */
    UInt64 LostBefore( UInt64 k ) const
    {
        UInt64 firstReceived = 0;
        for ( size_t i=0; i<lostFrames.size() && lostFrames[ i ] == firstReceived; ++i )
            ++firstReceived;
        UInt64 count = 0;
        for ( size_t i=0; i<lostFrames.size(); ++i )
            if ( lostFrames[ i ] > firstReceived && lostFrames[ i ] < k )
                ++count;
        return count;
    }

    /**
     *  Of the lost frames before frame 'k': the frames beyond the first 'limit' of each burst, and the number of bursts
     *  longer than 'limit' (after that many concealed frames the playout ends the talkspurt and plays silence)
     */
    UInt64 LostBeyondBurstLimit( UInt64 k, UInt32 limit, UInt64& longBursts ) const
    {
        UInt64 beyond = 0;
        longBursts = 0;
        size_t i = 0;
        while ( i < lostFrames.size() && lostFrames[ i ] < k )
        {
            size_t j = i + 1;
            while ( j < lostFrames.size() && lostFrames[ j ] == lostFrames[ j - 1 ] + 1 )
                ++j;
            const UInt64 length = (UInt64) ( j - i );
            if ( length > limit )
            {
                beyond += length - limit;
                ++longBursts;
            }
            i = j;
        }
        return beyond;
    }

    /** The frame index of the newest frame the buffer has seen (a stream without restart and talkspurts) */
    UInt64 HighestReceivedFrame( void ) const
    {
        TJitterBuffer::SStatistics stats;
        m_buffer.GetStatistics( stats );
        return stats.extendedHighestSequenceNumber - ( 65536 + (UInt64) m_config.firstSequenceNumber );
    }

    private:

    bool IsSent( UInt64 k ) const
    {
        if ( 0 == m_config.silentFrames )
            return true;
        return ( k % ( m_config.speakFrames + m_config.silentFrames ) ) < m_config.speakFrames;
    }

    bool IsSilentContent( UInt64 k ) const
    {
        if ( 0 == m_config.silentContentPeriod )
            return false;
        return ( k % m_config.silentContentPeriod ) >= m_config.silentContentPeriod - m_config.silentContentLength;
    }

    UInt32 TimestampOf( UInt64 k ) const
    {
        return (UInt32) ( m_config.firstTimestamp + (UInt32) ( k * FrameSize ) + m_tsOffset );
    }

    void GenerateUntil( UInt64 nowUs )
    {
        const SLinkConfig& link = m_config.link;
        while ( SendTimeUs( m_nextFrame ) <= (Float64) nowUs )
        {
            const UInt64 k = m_nextFrame++;
            if ( !IsSent( k ) )
                continue;
            if ( 0 != m_config.restartAtFrame && k == m_config.restartAtFrame )
            {
                m_seqOffset += 20000;
                m_tsOffset += 7000000;
            }
            const bool marker = 0 == k || !IsSent( k - 1 );
            const UInt16 seq = (UInt16) ( m_config.firstSequenceNumber + m_sentCount + m_seqOffset );
            ++m_sentCount;
            ++framesSent;

            // loss
            bool lost = m_config.lostFrames.end() != m_config.lostFrames.find( k );
            if ( link.geGoodToBad > 0.0 )
            {
                if ( m_geBad )
                    m_geBad = m_random.NextUnit() >= link.geBadToGood;
                else
                    m_geBad = m_random.NextUnit() < link.geGoodToBad;
                lost = lost || m_geBad;
            }
            if ( link.randomLoss > 0.0 && m_random.NextUnit() < link.randomLoss )
                lost = true;

            // delay
            const Float64 sendUs = SendTimeUs( k );
            Float64 delay = ( 0 != m_config.delayChangeAtFrame && k >= m_config.delayChangeAtFrame ) ? m_config.delayAfterChangeUs : link.baseDelayUs;
            if ( link.uniformJitterUs > 0.0 )
                delay += m_random.NextUnit() * link.uniformJitterUs;
            if ( link.paretoProbability > 0.0 && m_random.NextUnit() < link.paretoProbability )
            {
                Float64 u = m_random.NextUnit();
                if ( u < 1.0e-12 )
                    u = 1.0e-12;
                Float64 extra = link.paretoScaleUs / pow( u, 1.0 / link.paretoShape );
                if ( extra > 2000000.0 )
                    extra = 2000000.0;
                delay += extra;
            }
            if ( link.spikeProbability > 0.0 && sendUs >= m_spikeEndUs && m_random.NextUnit() < link.spikeProbability )
                m_spikeEndUs = sendUs + link.spikeDurationUs;
            GUCEF::map< UInt64, Int64 >::const_iterator extra = m_config.extraDelayUs.find( k );
            if ( m_config.extraDelayUs.end() != extra )
                delay += (Float64) extra->second;
            Float64 arrival = sendUs + delay;
            if ( sendUs < m_spikeEndUs && arrival < m_spikeEndUs )
                arrival = m_spikeEndUs + 50.0 * (Float64) ( k % 16 );

            if ( lost )
            {
                ++framesLost;
                lostFrames.push_back( k );
                continue;
            }
            Push( arrival, k, seq, marker );
            if ( link.duplicateProbability > 0.0 && m_random.NextUnit() < link.duplicateProbability )
            {
                ++framesDuplicated;
                Push( arrival + 100.0 + m_random.NextUnit() * 3000.0, k, seq, marker );
            }
        }
    }

    void Push( Float64 arrivalUs, UInt64 k, UInt16 seq, bool marker )
    {
        SArrivalEvent event;
        event.arrivalUs = (UInt64) ( arrivalUs + 0.5 );
        event.order = m_order++;
        event.frameIndex = k;
        event.sequenceNumber = seq;
        event.timestamp = TimestampOf( k );
        event.marker = marker;
        m_events.push_back( event );
        std::push_heap( m_events.begin(), m_events.end() );
    }

    void BuildFrame( const SArrivalEvent& event, AUDIO::CVoiceFrame& frame )
    {
        frame.Clear();
        frame.SetStreamId( 0x5EED );
        frame.SetSequenceNumber( event.sequenceNumber );
        frame.SetTimestamp( event.timestamp );
        frame.SetMarker( event.marker );
        frame.SetPayloadType( 112 );
        frame.SetCaptureTickCount( (UInt64) ( SendTimeUs( event.frameIndex ) ) );
        if ( PAYLOAD_PCM == m_config.payload )
        {
            Float32 samples[ FrameSize ];
            for ( UInt32 i=0; i<FrameSize; ++i )
                samples[ i ] = PcmSignal( event.frameIndex * FrameSize + i );
            const Int32 size = m_pcmEncoder->Encode( samples, frame.GetPayloadBuffer(), AUDIO::CVoiceFrame::MaxPayloadSize );
            frame.SetPayloadSize( size > 0 ? (UInt32) size : 0 );
        }
        else
        {
            WriteMarkerPayload( frame, event.frameIndex, IsSilentContent( event.frameIndex ) );
        }
        if ( m_config.redundancy && event.frameIndex > 0 && IsSent( event.frameIndex - 1 ) && PAYLOAD_MARKER == m_config.payload )
        {
            UInt8 previous[ 5 ];
            AUDIO::CVoiceFrame scratch;
            WriteMarkerPayload( scratch, event.frameIndex - 1, IsSilentContent( event.frameIndex - 1 ) );
            memcpy( previous, scratch.GetPayload(), 5 );
            frame.SetRedundantBlock( (UInt16) FrameSize, 112, previous, 5 );
        }
    }

    SSimConfig m_config;
    TJitterBuffer m_buffer;
    TController m_controller;
    CMarkerVoiceDecoder m_marker;
    AUDIO::TVoiceDecoderPtr m_pcmDecoder;
    AUDIO::TVoiceEncoderPtr m_pcmEncoder;
    AUDIO::CSincResampler m_resampler;
    bool m_ready;
    AudioEffectTest::CXorShift m_random;
    GUCEF::vector< SArrivalEvent > m_events;
    UInt64 m_order;
    UInt64 m_nextFrame;
    UInt64 m_sentCount;
    UInt32 m_seqOffset;
    UInt32 m_tsOffset;
    bool m_geBad;
    Float64 m_spikeEndUs;
    UInt64 m_lastSentFrame;
    UInt32 m_previousPayloadSize;
    SPlayoutStep m_step;
    GUCEF::vector< AUDIO::CVoiceFrame > m_arrivalFrames;
    GUCEF::vector< UInt64 > m_arrivalTimes;
    GUCEF::vector< Float32 > m_fifo;
    GUCEF::vector< Float32 > m_scratch;
    GUCEF::vector< Float32 > m_block;
    GUCEF::vector< SChunk > m_chunks;
    UInt64 m_stepIndex;
};

/*-------------------------------------------------------------------------*/

TController::SSettings
ProfileSettings( AUDIO::TVoicePlayoutProfile profile )
{
    TController::SSettings settings;
    settings.SetProfile( profile );
    return settings;
}

/*-------------------------------------------------------------------------*/

AUDIO::CVoiceFrame
MakeFrame( UInt16 seq, UInt32 ts, UInt64 index, bool marker = false )
{
    AUDIO::CVoiceFrame frame;
    frame.SetStreamId( 77 );
    frame.SetSequenceNumber( seq );
    frame.SetTimestamp( ts );
    frame.SetMarker( marker );
    frame.SetPayloadType( 112 );
    WriteMarkerPayload( frame, index, false );
    return frame;
}

/** The frame index in a marker payload */
UInt32
IndexOf( const TJitterBuffer::SFrameInfo& info )
{
    return GUCEF_NULL != info.frame ? ReadMarkerIndex( info.frame->GetPayload() ) : 0xFFFFFFFF;
}

/*-------------------------------------------------------------------------*/

/** Calls of the jitter buffer as the render thread makes them: a real time context in a RealtimeSanitizer build */
TJitterBuffer::EInsertResult
InsertRealtime( TJitterBuffer& buffer, const AUDIO::CVoiceFrame& frame, UInt64 arrivalUs, TJitterBuffer::SInsertInfo& info ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return buffer.Insert( frame, arrivalUs, info );
}

TJitterBuffer::EPullResult
PullRealtime( TJitterBuffer& buffer, UInt64 playoutTimestamp, UInt64 oldestWanted, TJitterBuffer::SPullInfo& info ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return buffer.Pull( playoutTimestamp, oldestWanted, info );
}

void
ResetRealtime( TJitterBuffer& buffer, TController& controller ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    buffer.Reset();
    controller.Reset();
}

/*-------------------------------------------------------------------------*/

Float64
Absolute( Float64 v )
{
    return v < 0.0 ? -v : v;
}

/*-------------------------------------------------------------------------*/

/** Settling: the first second from which on |Lmin - M| stays within 'tolerance' ms. -1 when never */
Float64
SettlingTime( const GUCEF::vector< SSecond >& seconds, Float64 toleranceMs )
{
    Float64 settled = -1.0;
    for ( size_t i=0; i<seconds.size(); ++i )
    {
        const bool inside = seconds[ i ].hasMinimum && Absolute( seconds[ i ].minimumMs - seconds[ i ].marginMs ) <= toleranceMs;
        if ( !inside )
            settled = -1.0;
        else
        if ( settled < 0.0 )
            settled = seconds[ i ].timeInS;
    }
    return settled;
}

/** The range (max - min) of the measured delay over the samples from 'fromS' on */
Float64
DelayRangeMs( const GUCEF::vector< SSecond >& seconds, Float64 fromS, Float64& minimum, Float64& maximum )
{
    minimum = 1.0e9;
    maximum = -1.0e9;
    for ( size_t i=0; i<seconds.size(); ++i )
    {
        if ( seconds[ i ].timeInS < fromS || !seconds[ i ].hasDelay )
            continue;
        if ( seconds[ i ].delayMs < minimum )
            minimum = seconds[ i ].delayMs;
        if ( seconds[ i ].delayMs > maximum )
            maximum = seconds[ i ].delayMs;
    }
    return maximum >= minimum ? maximum - minimum : 0.0;
}

/** The largest |Lmin - M| over the samples from 'fromS' on */
Float64
MaxMinimumError( const GUCEF::vector< SSecond >& seconds, Float64 fromS )
{
    Float64 worst = 0.0;
    for ( size_t i=0; i<seconds.size(); ++i )
    {
        if ( seconds[ i ].timeInS < fromS )
            continue;
        const Float64 e = seconds[ i ].hasMinimum ? Absolute( seconds[ i ].minimumMs - seconds[ i ].marginMs ) : 1.0e9;
        if ( e > worst )
            worst = e;
    }
    return worst;
}

/** The sample at or just after 'timeInS' */
const SSecond*
SecondAt( const GUCEF::vector< SSecond >& seconds, Float64 timeInS )
{
    for ( size_t i=0; i<seconds.size(); ++i )
        if ( seconds[ i ].timeInS >= timeInS - 1.0e-9 )
            return &seconds[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
PerformJitterBufferTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: The sliding quantile of the slack window equals a sorted copy of the last N values: minimum, ranks, quantiles, counts below a limit" )
    try
    {
        AUDIO::CSlidingQuantile empty;
        ASSERT_FALSE( empty.Prepare( 0 ) );
        ASSERT_TRUE( 0.0 == empty.GetMinimum() && 0.0 == empty.GetQuantile( 0.5 ) && 0 == empty.CountBelow( 1.0 ) );
        const UInt32 sizes[] = { 1, 2, 7, 400 };
        AudioEffectTest::CXorShift random( 31337 );
        for ( UInt32 c=0; c<4; ++c )
        {
            AUDIO::CSlidingQuantile window;
            ASSERT_TRUE( window.Prepare( sizes[ c ] ) && sizes[ c ] == window.GetWindowSize() );
            GUCEF::vector< Float64 > history;
            bool same = true;
            for ( UInt32 i=0; i<3000; ++i )
            {
                // values with repeats, so equal values come and go
                const Float64 value = (Float64) ( random.Next() % 50 ) - 25.0 + ( 0 == i % 3 ? 0.5 : 0.0 );
                window.Push( value );
                history.push_back( value );
                const size_t count = history.size() < sizes[ c ] ? history.size() : sizes[ c ];
                GUCEF::vector< Float64 > sorted( history.end() - count, history.end() );
                std::sort( sorted.begin(), sorted.end() );
                same = same && window.GetCount() == count && window.GetMinimum() == sorted.front() && window.GetRanked( 1000000 ) == sorted.back();
                const Float64 fractions[] = { 0.0, 0.01, 0.25, 0.5, 0.999 };
                for ( UInt32 q=0; q<5; ++q )
                    same = same && window.GetQuantile( fractions[ q ] ) == sorted[ (size_t) ( fractions[ q ] * (Float64) count ) ];
                const UInt32 below = (UInt32) ( std::lower_bound( sorted.begin(), sorted.end(), 0.0 ) - sorted.begin() );
                same = same && window.CountBelow( 0.0 ) == below;
            }
            ASSERT_TRUE( same );
            window.Push( sqrt( -1.0 ) );    // not a number counts as 0 and keeps the order
            ASSERT_TRUE( window.GetMinimum() <= 0.0 || 1 == sizes[ c ] );
            window.Reset();
            ASSERT_TRUE( 0 == window.GetCount() && 0.0 == window.GetMinimum() );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: In-order frames are handed out at their timestamps; Prepare refuses bad sizes" )
    try
    {
        TJitterBuffer buffer;
        ASSERT_FALSE( buffer.Prepare( 63, 480, 48000 ) );
        ASSERT_FALSE( buffer.Prepare( 1, 480, 48000 ) );
        ASSERT_FALSE( buffer.Prepare( 64, 0, 48000 ) );
        ASSERT_FALSE( buffer.Prepare( 64, 480, 0 ) );
        ASSERT_FALSE( buffer.IsPrepared() );
        ASSERT_TRUE( TJitterBuffer::INSERT_NOT_PREPARED == buffer.Insert( MakeFrame( 1, 1, 1 ), 0 ) );
        ASSERT_TRUE( buffer.Prepare( 64, 480, 48000 ) );

        TJitterBuffer::SPullInfo pull;
        ASSERT_TRUE( TJitterBuffer::PULL_NOT_PLAYING == buffer.Pull( 0, pull ) );
        ASSERT_FALSE( buffer.StartPlayout() );
        TJitterBuffer::SInsertInfo first;
        for ( UInt32 k=0; k<10; ++k )
        {
            TJitterBuffer::SInsertInfo info;
            ASSERT_TRUE( TJitterBuffer::INSERT_STORED == buffer.Insert( MakeFrame( (UInt16) ( 100 + k ), 1000 + 480 * k, k ), 10000 * k, info ) );
            if ( 0 == k )
                first = info;
            ASSERT_TRUE( info.extendedSequenceNumber == first.extendedSequenceNumber + k );
            ASSERT_TRUE( info.extendedTimestamp == first.extendedTimestamp + 480 * k );
        }
        ASSERT_TRUE( 10 == buffer.GetStoredFrameCount() );
        ASSERT_TRUE( first.extendedTimestamp + 4800 == buffer.GetBufferedEndTimestamp() );
        ASSERT_TRUE( buffer.StartPlayout() );
        for ( UInt32 k=0; k<10; ++k )
        {
            ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( first.extendedTimestamp + 480 * k, pull ) );
            ASSERT_TRUE( k == IndexOf( pull.frame ) );
            ASSERT_TRUE( 0 == pull.latenessInTimestampUnits && 0 == pull.skippedFrames );
            ASSERT_TRUE( 10000 * k == pull.frame.arrivalTimeInUs );
        }
        ASSERT_TRUE( TJitterBuffer::PULL_EMPTY == buffer.Pull( first.extendedTimestamp + 4800, pull ) );

        TJitterBuffer::SStatistics stats;
        buffer.GetStatistics( stats );
        ASSERT_TRUE( 10 == stats.framesReceived && 10 == stats.framesExpected && 0 == stats.cumulativeLost );
        ASSERT_TRUE( 0 == stats.reordered && 0 == stats.duplicates && 0 == stats.late && 0 == stats.bufferedFrames );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: Duplicates (stored or played), reordering and late frames (by sequence and by timestamp) are told apart" )
    try
    {
        TJitterBuffer buffer;
        ASSERT_TRUE( buffer.Prepare( 64, 480, 48000 ) );
        TJitterBuffer::SInsertInfo info;
        ASSERT_TRUE( TJitterBuffer::INSERT_STORED == buffer.Insert( MakeFrame( 10, 0, 0 ), 100 ) );
        ASSERT_TRUE( TJitterBuffer::INSERT_STORED == buffer.Insert( MakeFrame( 12, 960, 2 ), 200 ) );
        ASSERT_TRUE( TJitterBuffer::INSERT_REORDERED == buffer.Insert( MakeFrame( 11, 480, 1 ), 300, info ) );
        const UInt64 base = info.extendedTimestamp - 480;
        ASSERT_TRUE( TJitterBuffer::INSERT_DUPLICATE == buffer.Insert( MakeFrame( 12, 960, 2 ), 400 ) );
        ASSERT_TRUE( buffer.StartPlayout() );
        TJitterBuffer::SPullInfo pull;
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( base, pull ) && 0 == IndexOf( pull.frame ) );
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( base + 480, pull ) && 1 == IndexOf( pull.frame ) );
        ASSERT_TRUE( TJitterBuffer::INSERT_DUPLICATE == buffer.Insert( MakeFrame( 11, 480, 1 ), 500 ) );     // played already
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( base + 960, pull ) && 2 == IndexOf( pull.frame ) );

        // 13 and 14 do not come in time, 15 does
        ASSERT_TRUE( TJitterBuffer::INSERT_STORED == buffer.Insert( MakeFrame( 15, 480 * 5, 5 ), 600 ) );
        ASSERT_TRUE( TJitterBuffer::PULL_MISSING == buffer.Pull( base + 480 * 3, pull ) );
        ASSERT_TRUE( GUCEF_NULL == pull.frame.frame );
        ASSERT_TRUE( TJitterBuffer::PULL_MISSING == buffer.Pull( base + 480 * 4, pull ) );
        ASSERT_TRUE( 5 == IndexOf( pull.frame ) );                                                            // the next frame, for forward error correction
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( base + 480 * 5, pull ) && 5 == IndexOf( pull.frame ) );
        ASSERT_TRUE( 2 == pull.skippedFrames );
        ASSERT_TRUE( TJitterBuffer::INSERT_LATE == buffer.Insert( MakeFrame( 13, 480 * 3, 3 ), 700 ) );          // behind in sequence
        ASSERT_TRUE( TJitterBuffer::INSERT_DUPLICATE == buffer.Insert( MakeFrame( 13, 480 * 3, 3 ), 710 ) );     // and once it came, it came
        ASSERT_TRUE( TJitterBuffer::INSERT_LATE == buffer.Insert( MakeFrame( 16, 480 * 4, 4 ), 800 ) );          // ahead in sequence, behind in time

        TJitterBuffer::SStatistics stats;
        buffer.GetStatistics( stats );
        ASSERT_TRUE( 3 == stats.duplicates );
        ASSERT_TRUE( 2 == stats.late );
        ASSERT_TRUE( 2 == stats.reordered );        // 11, and 13 after 15
        ASSERT_TRUE( 6 == stats.framesReceived );   // 10, 12, 11, 15, 13, 16
        ASSERT_TRUE( 7 == stats.framesExpected );   // 10 to 16
        ASSERT_TRUE( 1 == stats.cumulativeLost );   // 14
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: Sequence numbers wrap at 65535 and timestamps at 2^32 without a break in the extended numbers, also out of order" )
    try
    {
        TJitterBuffer buffer;
        ASSERT_TRUE( buffer.Prepare( 64, 480, 48000 ) );
        const UInt32 firstTs = 0xFFFFFFFFu - 480 * 3 + 1;
        TJitterBuffer::SInsertInfo first, info;
        for ( UInt32 k=0; k<12; ++k )
        {
            const UInt32 order = ( 6 == k ) ? 7 : ( 7 == k ? 6 : k );      // 65536 + 0 and 65535 swapped: the wrap out of order
            const UInt16 seq = (UInt16) ( 65530 + order );
            const TJitterBuffer::EInsertResult result = buffer.Insert( MakeFrame( seq, firstTs + 480 * order, order ), 1000 * k, info );
            ASSERT_TRUE( ( 7 == k ? TJitterBuffer::INSERT_REORDERED : TJitterBuffer::INSERT_STORED ) == result );
            if ( 0 == k )
                first = info;
            ASSERT_TRUE( info.extendedSequenceNumber == first.extendedSequenceNumber + order );
            ASSERT_TRUE( info.extendedTimestamp == first.extendedTimestamp + 480 * order );
        }
        ASSERT_TRUE( buffer.StartPlayout() );
        TJitterBuffer::SPullInfo pull;
        for ( UInt32 k=0; k<12; ++k )
        {
            ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( first.extendedTimestamp + 480 * k, pull ) );
            ASSERT_TRUE( k == IndexOf( pull.frame ) );
            ASSERT_TRUE( (UInt16) ( 65530 + k ) == pull.frame.frame->GetSequenceNumber() );
        }
        TJitterBuffer::SStatistics stats;
        buffer.GetStatistics( stats );
        ASSERT_TRUE( stats.extendedHighestSequenceNumber == 2 * 65536 + 5 );    // one cycle up from the start cycle
        ASSERT_TRUE( 12 == stats.framesExpected && 0 == stats.cumulativeLost && 1 == stats.reordered );

        // a stream that starts at the very top of both ranges
        ASSERT_TRUE( buffer.Prepare( 16, 480, 48000 ) );
        ASSERT_TRUE( TJitterBuffer::INSERT_STORED == buffer.Insert( MakeFrame( 65535, 0xFFFFFFFFu, 0 ), 0, first ) );
        ASSERT_TRUE( TJitterBuffer::INSERT_STORED == buffer.Insert( MakeFrame( 0, 479, 1 ), 10000, info ) );
        ASSERT_TRUE( info.extendedSequenceNumber == first.extendedSequenceNumber + 1 && info.extendedTimestamp == first.extendedTimestamp + 480 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: A jump of the sequence number restarts the stream after two sequential frames, a single stray frame is dropped" )
    try
    {
        TJitterBuffer buffer;
        ASSERT_TRUE( buffer.Prepare( 64, 480, 48000 ) );
        for ( UInt32 k=0; k<5; ++k )
            ASSERT_TRUE( TJitterBuffer::INSERT_STORED == buffer.Insert( MakeFrame( (UInt16) ( 10 + k ), 480 * k, k ), 1000 * k ) );
        ASSERT_TRUE( buffer.StartPlayout() );
        TJitterBuffer::SPullInfo pull;
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( buffer.GetNextTimestamp(), pull ) );

        // a jump of more than MAX_DROPOUT: held
        ASSERT_TRUE( TJitterBuffer::INSERT_PROBATION == buffer.Insert( MakeFrame( 40000, 9000000, 100 ), 6000 ) );
        ASSERT_TRUE( 4 == buffer.GetStoredFrameCount() && buffer.IsPlaying() );
        // the old stream goes on: still the old stream
        ASSERT_TRUE( TJitterBuffer::INSERT_STORED == buffer.Insert( MakeFrame( 15, 480 * 5, 5 ), 7000 ) );
        // a dropout below MAX_DROPOUT is no jump: the same stream, far beyond the window
        ASSERT_TRUE( TJitterBuffer::INSERT_OVERFLOW == buffer.Insert( MakeFrame( 15 + 2999, 480 * 3004, 6 ), 7500 ) );
        ASSERT_FALSE( buffer.IsPlaying() );
        // the frame after the held one confirms the new stream
        ASSERT_TRUE( TJitterBuffer::INSERT_PROBATION == buffer.Insert( MakeFrame( 40000, 9000000, 100 ), 8000 ) );
        ASSERT_TRUE( TJitterBuffer::INSERT_RESTARTED == buffer.Insert( MakeFrame( 40001, 9000480, 101 ), 9000 ) );
        ASSERT_FALSE( buffer.IsPlaying() );
        ASSERT_FALSE( buffer.HasCursor() );
        ASSERT_TRUE( 2 == buffer.GetStoredFrameCount() );
        ASSERT_TRUE( buffer.StartPlayout() );
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( buffer.GetNextTimestamp(), pull ) && 100 == IndexOf( pull.frame ) );
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( buffer.GetNextTimestamp(), pull ) && 101 == IndexOf( pull.frame ) );

        // a single stray frame far away, then the stream goes on: the stray one is dropped when the next stray one comes
        ASSERT_TRUE( TJitterBuffer::INSERT_PROBATION == buffer.Insert( MakeFrame( 7, 123, 999 ), 10000 ) );
        ASSERT_TRUE( TJitterBuffer::INSERT_STORED == buffer.Insert( MakeFrame( 40002, 9000960, 102 ), 11000 ) );
        ASSERT_TRUE( TJitterBuffer::INSERT_PROBATION == buffer.Insert( MakeFrame( 20000, 456, 998 ), 12000 ) );
        // a frame within MAX_MISORDER behind the newest is late, not a restart
        ASSERT_TRUE( TJitterBuffer::INSERT_LATE == buffer.Insert( MakeFrame( 40002 - 90, 9000960 - 480 * 90, 3 ), 13000 ) );

        TJitterBuffer::SStatistics stats;
        buffer.GetStatistics( stats );
        ASSERT_TRUE( 1 == stats.restarts );
        ASSERT_TRUE( 2 == stats.probationDropped );     // the first 40000 (replaced by the second) and 7
        // expected: 10 to 3014 in the first stream, 40000 to 40002 in the second
        ASSERT_TRUE( 3005 + 3 == stats.framesExpected );
        ASSERT_TRUE( 7 + 4 == stats.framesReceived );   // 10..15, 3014; 40000, 40001, 40002, 39912
        ASSERT_TRUE( (Int64) stats.framesExpected - (Int64) stats.framesReceived == stats.cumulativeLost );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: A frame beyond the window pushes the oldest out, stops the playout and moves the cursor" )
    try
    {
        TJitterBuffer buffer;
        ASSERT_TRUE( buffer.Prepare( 8, 480, 48000 ) );
        for ( UInt32 k=0; k<8; ++k )
            ASSERT_TRUE( TJitterBuffer::INSERT_STORED == buffer.Insert( MakeFrame( (UInt16) k, 480 * k, k ), 1000 * k ) );
        ASSERT_TRUE( buffer.StartPlayout() );
        TJitterBuffer::SPullInfo pull;
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( buffer.GetNextTimestamp(), pull ) );
        ASSERT_TRUE( TJitterBuffer::INSERT_OVERFLOW == buffer.Insert( MakeFrame( 20, 480 * 20, 20 ), 9000 ) );
        ASSERT_FALSE( buffer.IsPlaying() );
        ASSERT_TRUE( 1 == buffer.GetStoredFrameCount() );
        TJitterBuffer::SStatistics stats;
        buffer.GetStatistics( stats );
        ASSERT_TRUE( 7 == stats.overflowDropped );
        // frames before the new window are late now
        ASSERT_TRUE( TJitterBuffer::INSERT_LATE == buffer.Insert( MakeFrame( 12, 480 * 12, 12 ), 9500 ) );
        ASSERT_TRUE( TJitterBuffer::INSERT_REORDERED == buffer.Insert( MakeFrame( 13, 480 * 13, 13 ), 9600 ) );
        ASSERT_TRUE( buffer.StartPlayout() );
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( buffer.GetNextTimestamp(), pull ) && 13 == IndexOf( pull.frame ) );

        // before playout the window reaches back as far as the newest frame allows
        ASSERT_TRUE( buffer.Prepare( 8, 480, 48000 ) );
        ASSERT_TRUE( TJitterBuffer::INSERT_STORED == buffer.Insert( MakeFrame( 100, 480 * 100, 100 ), 0 ) );
        ASSERT_TRUE( TJitterBuffer::INSERT_REORDERED == buffer.Insert( MakeFrame( 95, 480 * 95, 95 ), 10 ) );
        ASSERT_TRUE( TJitterBuffer::INSERT_LATE == buffer.Insert( MakeFrame( 92, 480 * 92, 92 ), 20 ) );
        ASSERT_TRUE( buffer.StartPlayout() );
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( buffer.GetNextTimestamp(), pull ) && 95 == IndexOf( pull.frame ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: An RFC 2198 redundant block fills an empty slot only, and the primary frame replaces it" )
    try
    {
        TJitterBuffer buffer;
        ASSERT_TRUE( buffer.Prepare( 64, 480, 48000 ) );
        AUDIO::CVoiceFrame previous = MakeFrame( 0, 0, 0 );

        // frame 1 carries frame 0: 0 is filled in from it
        AUDIO::CVoiceFrame frame = MakeFrame( 1, 480, 1 );
        ASSERT_TRUE( frame.SetRedundantBlock( 480, 112, previous.GetPayload(), previous.GetPayloadSize() ) );
        TJitterBuffer::SInsertInfo info;
        ASSERT_TRUE( TJitterBuffer::INSERT_STORED == buffer.Insert( frame, 1000, info ) );
        ASSERT_TRUE( info.redundantStored );
        ASSERT_TRUE( 2 == buffer.GetStoredFrameCount() );
        TJitterBuffer::SFrameInfo oldest;
        ASSERT_TRUE( buffer.PeekOldest( oldest ) && oldest.isRedundant && 0 == IndexOf( oldest ) );
        ASSERT_TRUE( 0 == oldest.frame->GetSequenceNumber() && 0 == oldest.frame->GetTimestamp() && !oldest.frame->HasRedundantBlock() );

        // the primary frame 0 arrives late after all: it takes the slot of the copy
        ASSERT_TRUE( TJitterBuffer::INSERT_REORDERED == buffer.Insert( previous, 1100 ) );
        ASSERT_TRUE( buffer.PeekOldest( oldest ) && !oldest.isRedundant && 1100 == oldest.arrivalTimeInUs );
        ASSERT_TRUE( 2 == buffer.GetStoredFrameCount() );

        // frame 2 carries frame 1, which is there: not used
        AUDIO::CVoiceFrame frame2 = MakeFrame( 2, 960, 2 );
        ASSERT_TRUE( frame2.SetRedundantBlock( 480, 112, frame.GetPayload(), frame.GetPayloadSize() ) );
        ASSERT_TRUE( TJitterBuffer::INSERT_STORED == buffer.Insert( frame2, 2000, info ) );
        ASSERT_FALSE( info.redundantStored );

        // play 0 to 2; frame 4 carries frame 3, which was never there and is not late: used
        ASSERT_TRUE( buffer.StartPlayout() );
        TJitterBuffer::SPullInfo pull;
        for ( UInt32 k=0; k<3; ++k )
            ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( buffer.GetNextTimestamp(), pull ) && k == IndexOf( pull.frame ) );
        AUDIO::CVoiceFrame frame3 = MakeFrame( 3, 1440, 3 );
        AUDIO::CVoiceFrame frame4 = MakeFrame( 4, 1920, 4 );
        ASSERT_TRUE( frame4.SetRedundantBlock( 480, 112, frame3.GetPayload(), frame3.GetPayloadSize() ) );
        ASSERT_TRUE( TJitterBuffer::INSERT_STORED == buffer.Insert( frame4, 4000, info ) );
        ASSERT_TRUE( info.redundantStored );
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( buffer.GetNextTimestamp(), pull ) && 3 == IndexOf( pull.frame ) && pull.frame.isRedundant );
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( buffer.GetNextTimestamp(), pull ) && 4 == IndexOf( pull.frame ) );
        // frame 5 carries frame 4, which was played: not used
        AUDIO::CVoiceFrame frame5 = MakeFrame( 5, 2400, 5 );
        ASSERT_TRUE( frame5.SetRedundantBlock( 480, 112, frame4.GetPayload(), frame4.GetPayloadSize() ) );
        ASSERT_TRUE( TJitterBuffer::INSERT_STORED == buffer.Insert( frame5, 5000, info ) );
        ASSERT_FALSE( info.redundantStored );

        TJitterBuffer::SStatistics stats;
        buffer.GetStatistics( stats );
        ASSERT_TRUE( 2 == stats.redundantStored && 2 == stats.redundantUnused );
        ASSERT_TRUE( 5 == stats.framesReceived && 6 == stats.framesExpected && 1 == stats.cumulativeLost );   // 3 came only as a copy
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: Pull tells a missing frame, an empty buffer and a new talkspurt apart, and hands out a late frame only when it is wanted" )
    try
    {
        TJitterBuffer buffer;
        ASSERT_TRUE( buffer.Prepare( 64, 480, 48000 ) );
        TJitterBuffer::SInsertInfo info;
        buffer.Insert( MakeFrame( 0, 0, 0, true ), 0, info );
        const UInt64 base = info.extendedTimestamp;
        buffer.Insert( MakeFrame( 1, 480, 1 ), 10 );
        buffer.Insert( MakeFrame( 3, 1440, 3 ), 30 );          // 2 lost
        buffer.Insert( MakeFrame( 4, 1920, 4 ), 40 );
        buffer.Insert( MakeFrame( 5, 1920 + 480 * 4, 5, true ), 50 );     // the sender paused 3 frames: a new talkspurt
        ASSERT_TRUE( buffer.StartPlayout() );
        TJitterBuffer::SPullInfo pull;
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( base, pull ) && 0 == IndexOf( pull.frame ) );
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( base + 480, pull ) && 1 == IndexOf( pull.frame ) );
        ASSERT_TRUE( TJitterBuffer::PULL_MISSING == buffer.Pull( base + 960, pull ) && 3 == IndexOf( pull.frame ) );
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( base + 1440, pull ) && 3 == IndexOf( pull.frame ) && 1 == pull.skippedFrames );
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( base + 1920, pull ) && 4 == IndexOf( pull.frame ) );
        ASSERT_TRUE( TJitterBuffer::PULL_TALKSPURT_END == buffer.Pull( base + 2400, pull ) );
        ASSERT_FALSE( buffer.IsPlaying() );
        ASSERT_TRUE( buffer.StartPlayout() );
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( base + 1920 + 480 * 4, pull ) && 5 == IndexOf( pull.frame ) );
        const UInt64 t6 = base + 1920 + 480 * 5;
        ASSERT_TRUE( TJitterBuffer::PULL_EMPTY == buffer.Pull( t6, pull ) );

        // concealment stood in for 6 with nothing buffered: 6 is still wanted when it comes during the concealment
        ASSERT_TRUE( TJitterBuffer::INSERT_STORED == buffer.Insert( MakeFrame( 6, (UInt32) ( 1920 + 480 * 5 ), 6 ), 60 ) );
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == buffer.Pull( t6 + 480, t6, pull ) && 6 == IndexOf( pull.frame ) );
        ASSERT_TRUE( 480 == pull.latenessInTimestampUnits );
        // 7 comes after two concealed frames, but the caller wants nothing before t7 + 960: dropped as late
        ASSERT_TRUE( TJitterBuffer::INSERT_STORED == buffer.Insert( MakeFrame( 7, (UInt32) ( 1920 + 480 * 6 ), 7 ), 70 ) );
        ASSERT_TRUE( TJitterBuffer::PULL_EMPTY == buffer.Pull( t6 + 480 * 3, t6 + 480 * 3, pull ) );
        ASSERT_TRUE( 1 == pull.lateDiscarded && 960 == pull.maxLatenessDiscarded );

        TJitterBuffer::SStatistics stats;
        buffer.GetStatistics( stats );
        ASSERT_TRUE( 1 == stats.talkspurtEnds && 1 == stats.late );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: The RFC 3550 interarrival jitter matches a hand computed sequence and loss follows appendix A.3" )
    try
    {
        // 8 kHz timestamps: one unit is 125 us, so arrival times are exact in microseconds
        TJitterBuffer buffer;
        ASSERT_TRUE( buffer.Prepare( 64, 80, 8000 ) );
        const UInt32 transit[] = { 1000, 1480, 1240, 2200, 2200, 1000 };
        for ( UInt32 k=0; k<6; ++k )
        {
            const UInt64 arrivalUnits = 80 * k + transit[ k ];
            buffer.Insert( MakeFrame( (UInt16) ( 500 + k ), 77777 + 80 * k, k ), arrivalUnits * 125 );
        }
        // D = 480, 240, 960, 0, 1200: J = 30, 43.125, 100.4296875, 94.15283203125, 163.268280029296875
        const Float64 expected = 163.268280029296875;
        ASSERT_TRUE( Absolute( buffer.GetInterarrivalJitter() - expected ) < 1.0 );
        ASSERT_TRUE( Absolute( buffer.GetInterarrivalJitter() - expected ) < 1.0e-6 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceJitter: interarrival jitter " << buffer.GetInterarrivalJitter() << " against " << expected << " by hand" );

        // A.3: a missing frame and a duplicate
        buffer.Insert( MakeFrame( 507, 77777 + 80 * 7, 7 ), 1000000 );
        buffer.Insert( MakeFrame( 507, 77777 + 80 * 7, 7 ), 1000100 );
        TJitterBuffer::SStatistics stats;
        buffer.GetStatistics( stats );
        ASSERT_TRUE( 8 == stats.framesExpected && 7 == stats.framesReceived && 1 == stats.cumulativeLost && 1 == stats.duplicates );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: After an idle time of more than half the timestamp range the next talkspurt's timestamp counts forward" )
    try
    {
        TJitterBuffer buffer;
        ASSERT_TRUE( buffer.Prepare( 64, 480, 48000 ) );
        TJitterBuffer::SInsertInfo first, info;
        buffer.Insert( MakeFrame( 1, 1000, 0 ), 0, first );
        // the same timestamp step without the idle time is a step back
        buffer.Insert( MakeFrame( 2, 1000 + 0x90000000u, 1 ), 10000, info );
        ASSERT_TRUE( info.extendedTimestamp < first.extendedTimestamp );

        ASSERT_TRUE( buffer.Prepare( 64, 480, 48000 ) );
        buffer.Insert( MakeFrame( 1, 1000, 0 ), 0, first );
        // 2^31 + 2^28 units at 48 kHz is about 13.9 hours: the sender's timestamp ran on while it was silent
        const UInt64 idleUs = (UInt64) ( (Float64) 0x90000000u / 48000.0 * 1000000.0 );
        buffer.Insert( MakeFrame( 2, 1000 + 0x90000000u, 1, true ), idleUs, info );
        ASSERT_TRUE( info.extendedTimestamp == first.extendedTimestamp + 0x90000000u );
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

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: Constant delay: the PCM output is bit-exact, delayed by transit plus margin plus one block, at ratio 1 exactly" )
    try
    {
        const AUDIO::TVoicePlayoutProfile profiles[] = { AUDIO::VOICEPLAYOUTPROFILE_WIRED, AUDIO::VOICEPLAYOUTPROFILE_WIRELESS };
        const Float64 transits[] = { 1000.0, 3000.0 };
        for ( UInt32 p=0; p<2; ++p )
        {
            for ( UInt32 wrap=0; wrap<2; ++wrap )
            {
                SSimConfig config;
                config.settings = ProfileSettings( profiles[ p ] );
                config.consumption = CONSUME_DIRECT;
                config.payload = PAYLOAD_PCM;
                config.recordOutput = true;
                config.link.baseDelayUs = transits[ p ];
                if ( 1 == wrap )
                {
                    // both numbers wrap during the run
                    config.firstSequenceNumber = 65000;
                    config.firstTimestamp = 0xFFFFFFFFu - 480 * 500;
                }
                // the margin stays where it starts: the run is shorter than the decay interval of the wired profile (10 s),
                // and for the wireless profile, whose margin decays every second, the minimum is the initial margin
                config.settings.minMarginInMs = config.settings.initialMarginInMs;
                CPlayoutSim sim( config );
                ASSERT_TRUE( sim.IsReady() );
                if ( !sim.IsReady() )
                    continue;
                sim.RunUntil( 9.5 );

                const UInt32 delay = (UInt32) ( ( (Float64) FrameUs + transits[ p ] + config.settings.initialMarginInMs * 1000.0 + (Float64) BlockUs ) * (Float64) Rate / 1000000.0 + 0.5 );
                UInt32 mismatches = 0;
                for ( size_t n=0; n<sim.output.size(); ++n )
                {
                    const Float32 expected = n < delay ? 0.0f : PcmSignal( n - delay );
                    if ( sim.output[ n ] != expected )
                        ++mismatches;
                }
                TController::SStatistics stats;
                sim.Controller().GetStatistics( stats );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceJitter: constant delay, profile " << p << ( wrap ? " with wrap" : "" ) << ": " << mismatches << " samples differ, delay " << delay <<
                           " samples, mean playout delay " << stats.meanPlayoutDelayInS * 1000.0 << " ms, Lmin " << stats.minimumLevelInS * 1000.0 << " ms, margin " << stats.marginInS * 1000.0 << " ms" );
                ASSERT_TRUE( 0 == mismatches );
                ASSERT_TRUE( sim.output.size() > 400000 );
                ASSERT_FALSE( sim.RatioWasNotOne() );
                ASSERT_TRUE( 0 == stats.framesConcealed && 0 == stats.lateFrames && 1 == stats.talkspurts );
                ASSERT_TRUE( Absolute( stats.meanPlayoutDelayInS - ( stats.marginInS + (Float64) BlockUs / 1000000.0 ) ) < 1.0 / Rate );
                ASSERT_TRUE( stats.hasMinimumLevel && Absolute( stats.minimumLevelInS - stats.marginInS ) <= 1.0 / Rate );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: Reordering by one and two frames within the margin plays every frame in order without concealment" )
    try
    {
        SSimConfig config;
        config.settings = ProfileSettings( AUDIO::VOICEPLAYOUTPROFILE_WIRELESS );
        config.settings.initialMarginInMs = 30.0;
        config.consumption = CONSUME_VIRTUAL;
        config.recordProduced = true;
        config.link.baseDelayUs = 3000.0;
        for ( UInt64 k=50; k<1000; k+=20 )
        {
            config.extraDelayUs[ k ] = 15000;           // after its successor
            config.extraDelayUs[ k + 10 ] = 25000;      // after the two that follow it
        }
        CPlayoutSim sim( config );
        ASSERT_TRUE( sim.IsReady() );
        sim.RunUntil( 11.0 );

        UInt64 decoded = 0, others = 0, outOfOrder = 0;
        UInt64 previous = 0;
        for ( size_t i=0; i<sim.produced.size(); ++i )
        {
            const SProduced& entry = sim.produced[ i ];
            if ( (UInt32) TController::PRODUCED_DECODED == entry.kind )
            {
                if ( decoded > 0 && entry.timestamp != previous + FrameSize )
                    ++outOfOrder;
                previous = entry.timestamp;
                ++decoded;
            }
            else
            if ( (UInt32) TController::PRODUCED_SILENCE != entry.kind )
                ++others;
        }
        TController::SStatistics stats;
        sim.Controller().GetStatistics( stats );
        TJitterBuffer::SStatistics bufferStats;
        sim.Buffer().GetStatistics( bufferStats );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceJitter: reordering within the margin: " << decoded << " decoded, " << bufferStats.reordered << " reordered, margin " << stats.marginInS * 1000.0 << " ms" );
        ASSERT_TRUE( decoded > 1050 );
        ASSERT_TRUE( 0 == others && 0 == outOfOrder );
        ASSERT_TRUE( bufferStats.reordered >= 90 );
        ASSERT_TRUE( 0 == stats.framesConcealed && 0 == stats.lateFrames && 0 == bufferStats.late );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: Reordering beyond the margin: concealment, then the late frame is dropped (newer frame there) or played with a stretch (none there); the margin grows by the lateness" )
    try
    {
        for ( UInt32 variant=0; variant<2; ++variant )
        {
            SSimConfig config;
            config.settings = ProfileSettings( AUDIO::VOICEPLAYOUTPROFILE_WIRED );     // M = 4 ms
            config.consumption = CONSUME_DIRECT;
            config.recordProduced = true;
            config.link.baseDelayUs = 1000.0;
            // 0: 20 ms late, it comes after its successor was played: dropped. 1: 12 ms late, it comes while concealment stands in for it
            config.extraDelayUs[ 200 ] = 0 == variant ? 20000 : 12000;
            CPlayoutSim sim( config );
            ASSERT_TRUE( sim.IsReady() );
            sim.RunUntil( 5.0 );

            TController::SStatistics stats;
            sim.Controller().GetStatistics( stats );
            TJitterBuffer::SStatistics bufferStats;
            sim.Buffer().GetStatistics( bufferStats );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceJitter: frame late by " << ( 0 == variant ? 20 : 12 ) << " ms: concealed " << stats.framesConcealed << ", late " << stats.lateFrames <<
                       ", stretched " << stats.framesStretched << ", underruns " << stats.underruns << ", margin " << stats.marginInS * 1000.0 << " ms" );
            ASSERT_TRUE( 1 == stats.framesConcealed );
            if ( 0 == variant )
            {
                // slack 4 - 20 = -16 ms: two frames
                ASSERT_TRUE( 1 == stats.lateFrames && 1 == bufferStats.late && 0 == stats.framesStretched );
                ASSERT_TRUE( Absolute( stats.marginInS - 0.024 ) < 1.0e-9 );
            }
            else
            {
                // played after one concealed frame: one frame of stretch
                ASSERT_TRUE( 0 == stats.lateFrames && 1 == stats.framesStretched && 1 == stats.underruns );
                ASSERT_TRUE( Absolute( stats.marginInS - 0.014 ) < 1.0e-9 );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: Losses: forward error correction where the next frame is there, concealment otherwise; duplicates are dropped" )
    try
    {
        for ( UInt32 fec=0; fec<2; ++fec )
        {
            SSimConfig config;
            config.settings = ProfileSettings( AUDIO::VOICEPLAYOUTPROFILE_WIRELESS );   // M = 20 ms: the next frame is there in time
            config.fec = 1 == fec;
            config.link.baseDelayUs = 3000.0;
            config.link.duplicateProbability = 0.05;
            config.lostFrames.insert( 100 );
            config.lostFrames.insert( 300 );
            config.lostFrames.insert( 500 );
            config.lostFrames.insert( 501 );
            config.lostFrames.insert( 502 );
            CPlayoutSim sim( config );
            ASSERT_TRUE( sim.IsReady() );
            sim.RunUntil( 8.0 );

            TController::SStatistics stats;
            sim.Controller().GetStatistics( stats );
            TJitterBuffer::SStatistics bufferStats;
            sim.Buffer().GetStatistics( bufferStats );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceJitter: losses with" << ( fec ? "" : "out" ) << " FEC: recovered " << stats.framesRecovered << ", concealed " << stats.framesConcealed <<
                       ", duplicates " << bufferStats.duplicates << " of " << sim.framesDuplicated );
            if ( 1 == fec )
            {
                // 100, 300 and the last of the burst come from the next frame; 500 and 501 have no next frame
                ASSERT_TRUE( 3 == stats.framesRecovered && 2 == stats.framesConcealed );
                ASSERT_TRUE( 3 == sim.Marker().recovered && 2 == sim.Marker().concealed );
            }
            else
            {
                ASSERT_TRUE( 0 == stats.framesRecovered && 5 == stats.framesConcealed );
            }
            ASSERT_TRUE( sim.framesDuplicated > 20 && bufferStats.duplicates == sim.framesDuplicated );
            ASSERT_TRUE( 5 == bufferStats.cumulativeLost && 0 == stats.lateFrames );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: A stream restart (sequence jump) is taken after probation and the playout starts over with the margin" )
    try
    {
        SSimConfig config;
        config.settings = ProfileSettings( AUDIO::VOICEPLAYOUTPROFILE_WIRED );
        config.recordProduced = true;
        config.link.baseDelayUs = 1000.0;
        config.restartAtFrame = 300;
        CPlayoutSim sim( config );
        ASSERT_TRUE( sim.IsReady() );
        sim.RunUntil( 6.0 );

        TController::SStatistics stats;
        sim.Controller().GetStatistics( stats );
        TJitterBuffer::SStatistics bufferStats;
        sim.Buffer().GetStatistics( bufferStats );
        // every frame of the new stream is played, its delay starts at the margin plus a block (within the frame the probation held it)
        UInt64 newDecoded = 0;
        Float64 firstDelay = -1.0, lastDelay = -1.0;
        for ( size_t i=0; i<sim.produced.size(); ++i )
        {
            const SProduced& entry = sim.produced[ i ];
            if ( (UInt32) TController::PRODUCED_DECODED == entry.kind && entry.outputTimeUs > 3.0e6 + 100000.0 )
            {
                if ( firstDelay < 0.0 )
                    firstDelay = entry.delayUs;
                lastDelay = entry.delayUs;
                ++newDecoded;
            }
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceJitter: restart: restarts " << bufferStats.restarts << ", talkspurts " << stats.talkspurts << ", concealed " << stats.framesConcealed <<
                   ", decoder resets " << sim.Marker().resets << ", playout delay after the restart " << firstDelay / 1000.0 << " to " << lastDelay / 1000.0 << " ms" );
        ASSERT_TRUE( 1 == bufferStats.restarts && 2 == stats.talkspurts );
        ASSERT_TRUE( sim.Marker().resets >= 2 );    // Prepare and the restart
        ASSERT_TRUE( newDecoded > 280 );
        const Float64 start = 4000.0 + ResampledBlockUs;
        ASSERT_TRUE( firstDelay >= start - 25.0 && firstDelay <= start + 10000.0 );
        ASSERT_TRUE( lastDelay >= start - 25.0 && lastDelay <= start + 10000.0 );
        ASSERT_TRUE( stats.framesConcealed <= 2 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: Talkspurts: idle after the concealment limit, a new start at the margin for every talkspurt, and short pauses found by their timestamps" )
    try
    {
        // 1 s of speech, 0.5 s of silence
        SSimConfig config;
        config.settings = ProfileSettings( AUDIO::VOICEPLAYOUTPROFILE_WIRED );
        config.recordProduced = true;
        config.link.baseDelayUs = 1000.0;
        config.speakFrames = 100;
        config.silentFrames = 50;
        CPlayoutSim sim( config );
        ASSERT_TRUE( sim.IsReady() );
        UInt32 idleSeen = 0;
        while ( sim.NowInS() < 9.0 )
        {
            sim.Step();
            const Float64 cycle = fmod( sim.NowInS(), 1.5 );
            if ( cycle > 1.3 && cycle < 1.45 && AUDIO::VOICEPLAYOUTSTATE_IDLE == sim.Controller().GetState() )
                ++idleSeen;
        }
        TController::SStatistics stats;
        sim.Controller().GetStatistics( stats );
        UInt32 starts = 0, startsAtMargin = 0;
        UInt32 previousKind = (UInt32) TController::PRODUCED_SILENCE;
        for ( size_t i=0; i<sim.produced.size(); ++i )
        {
            const SProduced& entry = sim.produced[ i ];
            if ( (UInt32) TController::PRODUCED_DECODED == entry.kind && (UInt32) TController::PRODUCED_SILENCE == previousKind )
            {
                ++starts;
                if ( Absolute( entry.delayUs - ( 4000.0 + ResampledBlockUs ) ) <= 25.0 )
                    ++startsAtMargin;
            }
            previousKind = entry.kind;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceJitter: talkspurts: " << stats.talkspurts << " starts, " << startsAtMargin << " at the margin, concealed " << stats.framesConcealed << ", idle seen " << idleSeen );
        ASSERT_TRUE( 6 == stats.talkspurts && 6 == starts && 6 == startsAtMargin );
        ASSERT_TRUE( 6 * config.settings.concealLimit == stats.framesConcealed );    // the tail of every talkspurt
        ASSERT_TRUE( idleSeen > 0 );

        // pauses of 20 ms, shorter than the concealment limit: found by the timestamp jump at the next frame
        SSimConfig shortConfig = config;
        shortConfig.silentFrames = 2;
        CPlayoutSim shortSim( shortConfig );
        ASSERT_TRUE( shortSim.IsReady() );
        shortSim.RunUntil( 5.1 );
        shortSim.Controller().GetStatistics( stats );
        TJitterBuffer::SStatistics bufferStats;
        shortSim.Buffer().GetStatistics( bufferStats );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceJitter: short pauses: " << stats.talkspurts << " talkspurts, " << bufferStats.talkspurtEnds << " found by their timestamps, concealed " << stats.framesConcealed );
        ASSERT_TRUE( 5 == stats.talkspurts && 4 == bufferStats.talkspurtEnds );
        ASSERT_TRUE( stats.framesConcealed <= 4 * 3 );
        ASSERT_TRUE( 0 == stats.lateFrames && 0 == bufferStats.late );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: Uniform jitter: no late frames after start-up, the margin decays to its floor and the playout delay settles near the design target" )
    try
    {
        struct SCase { AUDIO::TVoicePlayoutProfile profile; Float64 base; Float64 jitter; Float64 maxDelayMs; };
        // the latency budget (3.4): jitter buffer margin 3 to 7 ms on wired, 10 to 40 ms on Wi-Fi, beyond one block of 5 ms
        const SCase cases[] = { { AUDIO::VOICEPLAYOUTPROFILE_WIRED, 300.0, 200.0, 5.0 + 7.0 }, { AUDIO::VOICEPLAYOUTPROFILE_WIRELESS, 3000.0, 10000.0, 5.0 + 40.0 } };
        for ( UInt32 c=0; c<2; ++c )
        {
            SSimConfig config;
            config.settings = ProfileSettings( cases[ c ].profile );
            config.link.baseDelayUs = cases[ c ].base;
            config.link.uniformJitterUs = cases[ c ].jitter;
            config.link.seed = 777 + c;
            CPlayoutSim sim( config );
            ASSERT_TRUE( sim.IsReady() );
            sim.RunUntil( 10.0 );
            TController::SStatistics early;
            sim.Controller().GetStatistics( early );
            sim.ResetRatioTracking();
            sim.RunUntil( 180.0 );
            TController::SStatistics stats;
            sim.Controller().GetStatistics( stats );
            Float64 low = 0.0, high = 0.0;
            DelayRangeMs( sim.seconds, 120.0, low, high );
            // the floor of the profile: wired 3 J, wireless none beyond the minimum (its 1 % quantile of the slack covers the jitter)
            const Float64 jitterFloorMs = config.settings.jitterMarginFactor * stats.jitterInS * 1000.0;
            const Float64 floorMs = config.settings.minMarginInMs > jitterFloorMs ? config.settings.minMarginInMs : jitterFloorMs;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceJitter: uniform jitter " << cases[ c ].jitter / 1000.0 << " ms: margin " << stats.marginInS * 1000.0 << " ms (floor " << floorMs << "), Lmin " <<
                       stats.minimumLevelInS * 1000.0 << " ms, jitter " << stats.jitterInS * 1000.0 << " ms, playout delay " << low << " to " << high << " ms, late " << stats.lateFrames - early.lateFrames <<
                       ", ratio " << sim.ratioMin << " to " << sim.ratioMax );
            ASSERT_TRUE( stats.lateFrames == early.lateFrames && stats.framesConcealed == early.framesConcealed );
            ASSERT_TRUE( Absolute( stats.marginInS * 1000.0 - floorMs ) < 1.0 );
            ASSERT_TRUE( MaxMinimumError( sim.seconds, 120.0 ) < 2.0 );
            ASSERT_TRUE( high <= cases[ c ].maxDelayMs + cases[ c ].jitter / 1000.0 && low >= stats.marginInS * 1000.0 );
            ASSERT_TRUE( sim.ratioMin >= 1.0 - config.settings.maxRatioDeviation && sim.ratioMax <= 1.0 + config.settings.maxRatioDeviation );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: Bursty Wi-Fi delay (Pareto tail, stalls) and 1 % loss: late frames within the quantile of the profile, the delay follows the bulk of the distribution, stalls are waited for" )
    try
    {
        // The design asked for at most 0.2 % late frames together with an end-to-end delay of at most 80 ms. With this
        // delay model (independent Pareto delays, 1 % above 30 ms) the 99.8 % point of the network delay alone is 94 ms,
        // so both cannot hold: the wireless profile keeps the margin beyond the 1 % quantile of the slack and accepts
        // about 1 % late frames (concealed) for a delay that follows the bulk of the distribution
        for ( UInt32 variant=0; variant<2; ++variant )
        {
            SSimConfig config;
            config.settings = ProfileSettings( AUDIO::VOICEPLAYOUTPROFILE_WIRELESS );
            config.recordProduced = true;
            config.link.baseDelayUs = 3000.0;
            config.link.randomLoss = 0.01;
            config.link.seed = 4242 + variant;
            if ( 0 == variant )
            {
                // Pareto: median 2 ms, 1 % above 30 ms
                config.link.paretoProbability = 1.0;
                config.link.paretoShape = log( 50.0 ) / log( 15.0 );
                config.link.paretoScaleUs = 2000.0 / pow( 2.0, 1.0 / config.link.paretoShape );
            }
            else
            {
                // a stall of 60 ms every 10 s on average on top of 2 ms of jitter
                config.link.uniformJitterUs = 2000.0;
                config.link.spikeProbability = 0.001;
                config.link.spikeDurationUs = 60000.0;
            }
            CPlayoutSim sim( config );
            ASSERT_TRUE( sim.IsReady() );
            sim.RunUntil( 10.0 );
            TController::SStatistics early;
            sim.Controller().GetStatistics( early );
            const UInt64 sentEarly = sim.framesSent, lostEarly = sim.framesLost;
            const size_t producedEarly = sim.produced.size();
            sim.ResetRatioTracking();
            sim.RunUntil( 300.0 );
            TController::SStatistics stats;
            sim.Controller().GetStatistics( stats );
            const UInt64 sent = sim.framesSent - sentEarly;
            const UInt64 lost = sim.framesLost - lostEarly;
            const UInt64 late = stats.lateFrames - early.lateFrames + stats.framesStretched - early.framesStretched;
            const UInt64 concealed = stats.framesConcealed - early.framesConcealed;
            Float64 low = 0.0, high = 0.0;
            DelayRangeMs( sim.seconds, 10.0, low, high );
            // from the moment the sender had a frame complete to the output of its first sample: network plus playout delay
            GUCEF::vector< Float64 > sendDelays;
            for ( size_t i=producedEarly; i<sim.produced.size(); ++i )
                if ( (UInt32) TController::PRODUCED_DECODED == sim.produced[ i ].kind )
                    sendDelays.push_back( sim.produced[ i ].sendDelayUs / 1000.0 );
            std::sort( sendDelays.begin(), sendDelays.end() );
            Float64 meanSendDelay = 0.0;
            for ( size_t i=0; i<sendDelays.size(); ++i )
                meanSendDelay += sendDelays[ i ];
            meanSendDelay /= sendDelays.empty() ? 1.0 : (Float64) sendDelays.size();
            const Float64 p99SendDelay = sendDelays.empty() ? 0.0 : sendDelays[ sendDelays.size() * 99 / 100 ];
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceJitter: bursty " << ( 0 == variant ? "Pareto" : "stalls" ) << ": " << sent << " frames, " << lost << " lost, " << late << " late (" << 100.0 * (Float64) late / (Float64) sent <<
                       " %), concealed " << concealed << ", margin " << stats.marginInS * 1000.0 << " ms, playout delay " << low << " to " << high << " ms, send to output mean " << meanSendDelay <<
                       " ms (99 % " << p99SendDelay << " ms), ratio " << sim.ratioMin << " to " << sim.ratioMax );
            ASSERT_TRUE( (Float64) late <= config.settings.lateQuantile * (Float64) sent );
            ASSERT_TRUE( concealed >= lost && (Float64) concealed <= 1.1 * (Float64) ( lost + late ) + 10.0 );
            // network and playout within 60 ms on average: with the frame and the device buffer within the 80 ms of 3.4.
            // Stalls that empty the buffer are waited for: the margin grows with each, and decays between them
            if ( 0 == variant )
                ASSERT_TRUE( meanSendDelay <= 60.0 );
            else
                ASSERT_TRUE( late <= 30 && stats.marginInS > 0.03 );
            ASSERT_TRUE( sim.ratioMin >= 1.0 - config.settings.maxRatioDeviation && sim.ratioMax <= 1.0 + config.settings.maxRatioDeviation );
            ASSERT_TRUE( stats.marginInS <= 0.2 + 1.0e-9 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: Gilbert-Elliott burst loss: RFC 3550 loss count exact, concealment and recovery account for every lost frame, redundancy fills single losses" )
    try
    {
        for ( UInt32 variant=0; variant<3; ++variant )
        {
            SSimConfig config;
            config.settings = ProfileSettings( AUDIO::VOICEPLAYOUTPROFILE_WIRELESS );
            config.link.baseDelayUs = 3000.0;
            config.link.uniformJitterUs = 1000.0;
            config.link.geGoodToBad = 0.01;          // mean burst 3 frames, about 3 % loss
            config.link.geBadToGood = 1.0 / 3.0;
            config.link.seed = 99;
            config.fec = 1 == variant;
            config.redundancy = 2 == variant;
            CPlayoutSim sim( config );
            ASSERT_TRUE( sim.IsReady() );
            sim.RunUntil( 120.0 );
            TController::SStatistics stats;
            sim.Controller().GetStatistics( stats );
            TJitterBuffer::SStatistics bufferStats;
            sim.Buffer().GetStatistics( bufferStats );
            const UInt64 filled = stats.framesConcealed + stats.framesRecovered + bufferStats.redundantStored;
            const UInt64 highest = sim.HighestReceivedFrame();
            UInt64 longBursts = 0;
            const UInt64 beyond = sim.LostBeyondBurstLimit( highest, config.settings.concealLimit, longBursts );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceJitter: Gilbert-Elliott " << ( 0 == variant ? "plain" : ( 1 == variant ? "FEC" : "redundancy" ) ) << ": " << sim.framesLost << " of " << sim.framesSent <<
                       " lost (" << 100.0 * (Float64) sim.framesLost / (Float64) sim.framesSent << " %), RFC 3550 lost " << bufferStats.cumulativeLost << ", concealed " << stats.framesConcealed <<
                       ", recovered " << stats.framesRecovered << ", from redundancy " << bufferStats.redundantStored << ", late " << stats.lateFrames << ", bursts longer than the concealment limit " <<
                       longBursts << " (" << beyond << " frames beyond it), talkspurts " << stats.talkspurts );
            // RFC 3550 counts from the first frame received up to the newest; the frames lost after it are not known yet
            const UInt64 lost = sim.LostBefore( highest );
            ASSERT_TRUE( (Int64) lost == bufferStats.cumulativeLost );
            ASSERT_TRUE( sim.framesLost > 200 );
            ASSERT_TRUE( 0 == stats.lateFrames && 0 == stats.framesStretched );
            // every lost frame was made up for, except where a burst outlasted the concealment limit: the talkspurt ended
            // there (silence) and started over with the next frame
            ASSERT_TRUE( filled <= lost && filled + beyond >= sim.LostBefore( highest - 3 ) );
            ASSERT_TRUE( stats.talkspurts <= 1 + longBursts );
            if ( 1 == variant )
                ASSERT_TRUE( stats.framesRecovered > 0 );
            if ( 2 == variant )
                ASSERT_TRUE( bufferStats.redundantStored > 0 && stats.framesConcealed < sim.framesLost );
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
PerformDriftTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: Clock drift of -300 to +300 ppm over 10 minutes: no underrun after 5 s, Lmin within 2 ms of M, integral within 5 ppm of the drift after 2 minutes" )
    try
    {
        const Float64 drifts[] = { -300.0, -200.0, -100.0, 100.0, 200.0, 300.0 };
        for ( UInt32 d=0; d<6; ++d )
        {
            SSimConfig config;
            config.settings = ProfileSettings( AUDIO::VOICEPLAYOUTPROFILE_WIRED );
            config.driftPpm = drifts[ d ];
            config.link.baseDelayUs = 1000.0;
            config.link.uniformJitterUs = 200.0;
            config.link.seed = 31 + d;
            CPlayoutSim sim( config );
            ASSERT_TRUE( sim.IsReady() );
            sim.RunUntil( 5.0 );
            TController::SStatistics early;
            sim.Controller().GetStatistics( early );
            sim.ResetRatioTracking();
            sim.RunUntil( 600.0 );
            TController::SStatistics stats;
            sim.Controller().GetStatistics( stats );

            Float64 worstIntegral = 0.0;
            for ( size_t i=0; i<sim.seconds.size(); ++i )
                if ( sim.seconds[ i ].timeInS >= 120.0 && Absolute( sim.seconds[ i ].integralPpm - drifts[ d ] ) > worstIntegral )
                    worstIntegral = Absolute( sim.seconds[ i ].integralPpm - drifts[ d ] );
            Float64 low = 0.0, high = 0.0;
            const Float64 range = DelayRangeMs( sim.seconds, 300.0, low, high );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceJitter: drift " << drifts[ d ] << " ppm: settled (|Lmin - M| < 0.5 ms) at " << SettlingTime( sim.seconds, 0.5 ) << " s, worst |Lmin - M| after 2 min " <<
                       MaxMinimumError( sim.seconds, 120.0 ) << " ms, integral " << stats.integral * 1.0e6 << " ppm (worst error after 2 min " << worstIntegral << " ppm), delay over the last 5 min " << low <<
                       " to " << high << " ms (range " << range << "), margin " << stats.marginInS * 1000.0 << " ms, ratio " << sim.ratioMin << " to " << sim.ratioMax );
            ASSERT_TRUE( stats.underruns == early.underruns && stats.lateFrames == early.lateFrames && stats.framesConcealed == early.framesConcealed );
            ASSERT_TRUE( MaxMinimumError( sim.seconds, 120.0 ) <= 2.0 );
            ASSERT_TRUE( worstIntegral <= 5.0 );
            ASSERT_TRUE( sim.ratioMin >= 1.0 - config.settings.maxRatioDeviation && sim.ratioMax <= 1.0 + config.settings.maxRatioDeviation );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: Clock drift of +-0.5 %: the controller converges to a stable level without underruns, the ratio within its bound" )
    try
    {
        // The bound of the ratio (0.6 %) leaves headroom at 0.5 %: the level returns to the margin as at 0.45 %. With the
        // bound of the design (0.5 %) the drift took the whole bound at 0.5 %, and whatever excess or shortfall the
        // start-up left (up to 47 ms) could only be held, not removed
        const Float64 drifts[] = { -5000.0, 5000.0, -4500.0, 4500.0 };
        const AUDIO::TVoicePlayoutProfile profiles[] = { AUDIO::VOICEPLAYOUTPROFILE_WIRED, AUDIO::VOICEPLAYOUTPROFILE_WIRELESS };
        for ( UInt32 p=0; p<2; ++p )
        {
            for ( UInt32 d=0; d<4; ++d )
            {
                SSimConfig config;
                config.settings = ProfileSettings( profiles[ p ] );
                config.driftPpm = drifts[ d ];
                config.link.baseDelayUs = 0 == p ? 1000.0 : 3000.0;
                config.link.uniformJitterUs = 0 == p ? 200.0 : 4000.0;
                config.link.seed = 55 + d + 4 * p;
                CPlayoutSim sim( config );
                ASSERT_TRUE( sim.IsReady() );
                sim.RunUntil( 120.0 );
                TController::SStatistics settled;
                sim.Controller().GetStatistics( settled );
                sim.ResetRatioTracking();
                sim.RunUntil( 600.0 );
                TController::SStatistics stats;
                sim.Controller().GetStatistics( stats );
                Float64 low = 0.0, high = 0.0;
                const Float64 range = DelayRangeMs( sim.seconds, 300.0, low, high );
                Float64 lastUnderrunAt = 0.0;
                for ( size_t i=1; i<sim.seconds.size(); ++i )
                    if ( sim.seconds[ i ].underruns + sim.seconds[ i ].late != sim.seconds[ i - 1 ].underruns + sim.seconds[ i - 1 ].late )
                        lastUnderrunAt = sim.seconds[ i ].timeInS;
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceJitter: drift " << drifts[ d ] / 10000.0 << " %, profile " << p << ": settled at " << SettlingTime( sim.seconds, 1.0 ) << " s, last underrun or late frame in second " << lastUnderrunAt << ", start-up underruns " << settled.underruns <<
                           ", late " << settled.lateFrames << ", concealed " << settled.framesConcealed << "; after 2 min: underruns " << stats.underruns - settled.underruns << ", late " << stats.lateFrames - settled.lateFrames <<
                           ", integral " << stats.integral * 100.0 << " %, worst |Lmin - M| " << MaxMinimumError( sim.seconds, 120.0 ) << " ms, delay over the last 5 min " << low << " to " << high << " ms (range " << range <<
                           "), margin " << stats.marginInS * 1000.0 << " ms, ratio " << sim.ratioMin << " to " << sim.ratioMax );
                const SSecond* fiveMinutes = SecondAt( sim.seconds, 300.0 );
                ASSERT_TRUE( GUCEF_NULL != fiveMinutes );
                ASSERT_TRUE( stats.underruns == settled.underruns && stats.lateFrames == settled.lateFrames && stats.framesConcealed == settled.framesConcealed );
                ASSERT_TRUE( sim.ratioMin >= 1.0 - config.settings.maxRatioDeviation - 1.0e-12 && sim.ratioMax <= 1.0 + config.settings.maxRatioDeviation + 1.0e-12 );
                ASSERT_TRUE( Absolute( stats.integral * 1.0e6 - drifts[ d ] ) <= 50.0 );
                ASSERT_TRUE( range <= 2.0 + ( 0 == p ? 0.2 : 4.0 ) );
                ASSERT_TRUE( MaxMinimumError( sim.seconds, 120.0 ) <= 2.0 );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: Drift through the real sinc resampler: +300 ppm for 3 minutes, delay held, ratio within bounds, output finite" )
    try
    {
        SSimConfig config;
        config.settings = ProfileSettings( AUDIO::VOICEPLAYOUTPROFILE_WIRED );
        config.consumption = CONSUME_SINC;
        config.driftPpm = 300.0;
        config.link.baseDelayUs = 1000.0;
        config.link.uniformJitterUs = 200.0;
        CPlayoutSim sim( config );
        ASSERT_TRUE( sim.IsReady() );
        sim.RunUntil( 5.0 );
        TController::SStatistics early;
        sim.Controller().GetStatistics( early );
        sim.RunUntil( 180.0 );
        TController::SStatistics stats;
        sim.Controller().GetStatistics( stats );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceJitter: sinc resampler, +300 ppm: integral " << stats.integral * 1.0e6 << " ppm, worst |Lmin - M| after 2 min " << MaxMinimumError( sim.seconds, 120.0 ) <<
                   " ms, margin " << stats.marginInS * 1000.0 << " ms, ratio " << sim.ratioMin << " to " << sim.ratioMax );
        ASSERT_FALSE( sim.FifoOverflowed() );
        ASSERT_TRUE( stats.underruns == early.underruns && stats.framesConcealed == early.framesConcealed );
        ASSERT_TRUE( Absolute( stats.integral * 1.0e6 - 300.0 ) <= 10.0 );
        ASSERT_TRUE( MaxMinimumError( sim.seconds, 120.0 ) <= 2.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: After a burst of late frames the margin returns to its minimum at 1 ms per decay interval" )
    try
    {
        SSimConfig config;
        config.settings = ProfileSettings( AUDIO::VOICEPLAYOUTPROFILE_WIRED );
        config.link.baseDelayUs = 1000.0;
        // frames 2000 to 2009 (20 s) arrive 35 ms late
        for ( UInt64 k=2000; k<2010; ++k )
            config.extraDelayUs[ k ] = 35000;
        CPlayoutSim sim( config );
        ASSERT_TRUE( sim.IsReady() );
        Float64 lastLateAt = 0.0;
        UInt64 lastLate = 0;
        Float64 peak = 0.0;
        while ( sim.NowInS() < 120.0 )
        {
            sim.Step();
            TController::SStatistics stats;
            sim.Controller().GetStatistics( stats );
            if ( stats.lateFrames + stats.framesStretched != lastLate )
            {
                lastLate = stats.lateFrames + stats.framesStretched;
                lastLateAt = sim.NowInS();
            }
            if ( stats.marginInS > peak )
                peak = stats.marginInS;
        }
        // M at whole decay intervals after the last late frame
        bool steps = true;
        for ( UInt32 i=1; i<9; ++i )
        {
            const SSecond* second = SecondAt( sim.seconds, lastLateAt + 10.0 * i + 0.5 );
            if ( GUCEF_NULL == second )
                continue;
            Float64 expected = peak * 1000.0 - (Float64) i;
            if ( expected < 2.0 )
                expected = 2.0;
            if ( Absolute( second->marginMs - expected ) > 1.0e-6 )
                steps = false;
        }
        TController::SStatistics stats;
        sim.Controller().GetStatistics( stats );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceJitter: margin decay: peak " << peak * 1000.0 << " ms after the late frames (last at " << lastLateAt << " s), " << stats.marginInS * 1000.0 << " ms at " << sim.NowInS() << " s" );
        ASSERT_TRUE( lastLate > 0 && peak > 0.02 );
        ASSERT_TRUE( steps );
        ASSERT_TRUE( lastLateAt + 10.0 * ( peak * 1000.0 - 2.0 ) < 120.0 ? Absolute( stats.marginInS - 0.002 ) < 1.0e-9 : true );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: A large excess is removed by dropping silent frames, far faster than by the ratio alone" )
    try
    {
        Float64 times[ 2 ] = { -1.0, -1.0 };
        UInt64 dropped[ 2 ] = { 0, 0 };
        for ( UInt32 silent=0; silent<2; ++silent )
        {
            SSimConfig config;
            config.settings = ProfileSettings( AUDIO::VOICEPLAYOUTPROFILE_WIRELESS );
            config.link.baseDelayUs = 90000.0;
            config.delayChangeAtFrame = 500;         // the route changes at 5 s: 87 ms less transit
            config.delayAfterChangeUs = 3000.0;
            if ( 1 == silent )
            {
                // pauses within the speech: 4 of every 10 frames are silence
                config.silentContentPeriod = 10;
                config.silentContentLength = 4;
            }
            CPlayoutSim sim( config );
            ASSERT_TRUE( sim.IsReady() );
            Float64 peak = 0.0;
            while ( sim.NowInS() < 90.0 )
            {
                sim.Step();
                if ( !sim.HasDelay() || sim.NowInS() < 5.2 )
                    continue;
                // the delay from arrival to output jumped by 87 ms when the route changed
                const Float64 target = sim.Controller().GetMarginInS() * 1000000.0 + ResampledBlockUs + 2.0 * (Float64) FrameUs;
                if ( sim.DelayUs() > peak )
                    peak = sim.DelayUs();
                if ( times[ silent ] < 0.0 && peak > target + 50000.0 && sim.DelayUs() <= target )
                    times[ silent ] = sim.NowInS() - 5.0;
            }
            TController::SStatistics stats;
            sim.Controller().GetStatistics( stats );
            dropped[ silent ] = stats.framesAccelerated;
            ASSERT_TRUE( 0 == stats.lateFrames );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceJitter: an excess of 87 ms comes down to M + 2 frames in " << times[ 0 ] << " s by the ratio alone and in " << times[ 1 ] << " s with " << dropped[ 1 ] << " silent frames dropped" );
        ASSERT_TRUE( 0 == dropped[ 0 ] && dropped[ 1 ] > 0 );
        ASSERT_TRUE( times[ 1 ] > 0.0 && times[ 0 ] > 0.0 );
        ASSERT_TRUE( times[ 1 ] * 3.0 < times[ 0 ] );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: Long run: 3 simulated hours of jitter, loss, duplicates, drift and wrap-around stay stable" )
    try
    {
        SSimConfig config;
        config.settings = ProfileSettings( AUDIO::VOICEPLAYOUTPROFILE_WIRELESS );
        config.driftPpm = 80.0;
        config.firstSequenceNumber = 60000;
        config.firstTimestamp = 0xFFFFFFFFu - 48000 * 60;      // the timestamp wraps after a minute
        config.link.baseDelayUs = 3000.0;
        config.link.uniformJitterUs = 8000.0;
        config.link.randomLoss = 0.005;
        config.link.duplicateProbability = 0.01;
        config.link.seed = 2026;
        CPlayoutSim sim( config );
        ASSERT_TRUE( sim.IsReady() );
        const UInt64 startTicks = MT::PrecisionTickCount();
        sim.RunUntil( 60.0 );
        TController::SStatistics early;
        sim.Controller().GetStatistics( early );
        sim.ResetRatioTracking();
        sim.RunUntil( 3.0 * 3600.0 );
        const Float64 wallInS = (Float64) ( MT::PrecisionTickCount() - startTicks ) / (Float64) MT::PrecisionTimerResolution();
        TController::SStatistics stats;
        sim.Controller().GetStatistics( stats );
        TJitterBuffer::SStatistics bufferStats;
        sim.Buffer().GetStatistics( bufferStats );
        Float64 firstLow = 0.0, firstHigh = 0.0, low = 0.0, high = 0.0;
        DelayRangeMs( sim.seconds, 3600.0, firstLow, firstHigh );
        DelayRangeMs( sim.seconds, 2.0 * 3600.0, low, high );
        Float64 marginMin = 1.0e9, marginMax = 0.0;
        for ( size_t i=0; i<sim.seconds.size(); ++i )
        {
            if ( sim.seconds[ i ].timeInS < 60.0 )
                continue;
            if ( sim.seconds[ i ].marginMs < marginMin )
                marginMin = sim.seconds[ i ].marginMs;
            if ( sim.seconds[ i ].marginMs > marginMax )
                marginMax = sim.seconds[ i ].marginMs;
        }
        const UInt64 late = stats.lateFrames - early.lateFrames + stats.framesStretched - early.framesStretched;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceJitter: 3 hours in " << wallInS << " s: " << sim.framesSent << " frames, sequence cycles " << ( bufferStats.extendedHighestSequenceNumber >> 16 ) - 1 <<
                   ", lost " << bufferStats.cumulativeLost << ", late " << late << ", concealed " << stats.framesConcealed << ", margin " << marginMin << " to " << marginMax << " ms, integral " << stats.integral * 1.0e6 <<
                   " ppm, delay in the 2nd hour " << firstLow << " to " << firstHigh << " ms, in the 3rd " << low << " to " << high << " ms, ratio " << sim.ratioMin << " to " << sim.ratioMax );
        ASSERT_TRUE( ( bufferStats.extendedHighestSequenceNumber >> 16 ) - 1 >= 16 );
        ASSERT_TRUE( (Int64) sim.LostBefore( sim.HighestReceivedFrame() ) == bufferStats.cumulativeLost );
        ASSERT_TRUE( (Float64) late <= 0.0005 * (Float64) sim.framesSent );
        ASSERT_TRUE( sim.ratioMin >= 1.0 - config.settings.maxRatioDeviation && sim.ratioMax <= 1.0 + config.settings.maxRatioDeviation );
        ASSERT_TRUE( Absolute( stats.integral * 1.0e6 - 80.0 ) <= 10.0 );
        ASSERT_TRUE( marginMax <= 40.0 );
        ASSERT_TRUE( Absolute( ( high + low ) - ( firstHigh + firstLow ) ) / 2.0 <= 3.0 );
        ASSERT_TRUE( high - low <= 12.0 );
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

    GUCEF_TESTFW_TESTCASE( "AudioVoiceJitter: The playout path allocates nothing and takes no lock (RealtimeSanitizer build)" )
    if ( !IsRtsanBuild )
    {
        GUCEF_TESTFW_SKIP( "not a RealtimeSanitizer build (-fsanitize=realtime)" );
    }
    GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_RTSAN_CLEAN" ) == "0", "switched off with GUCEF_TEST_AUDIO_RTSAN_CLEAN=0" );
    try
    {
        // every render request of the simulation runs as a real time context: PCM through the sinc resampler with
        // jitter, loss, duplicates, talkspurts, a restart, a window overflow and drift
        SSimConfig config;
        config.settings = ProfileSettings( AUDIO::VOICEPLAYOUTPROFILE_WIRELESS );
        config.consumption = CONSUME_SINC;
        config.payload = PAYLOAD_PCM;
        config.slotCount = 16;
        config.driftPpm = 400.0;
        config.speakFrames = 300;
        config.silentFrames = 40;
        config.restartAtFrame = 700;
        config.link.baseDelayUs = 3000.0;
        config.link.uniformJitterUs = 15000.0;
        config.link.randomLoss = 0.03;
        config.link.duplicateProbability = 0.03;
        config.link.spikeProbability = 0.005;
        config.link.spikeDurationUs = 250000.0;      // longer than 16 slots: the window overflows
        CPlayoutSim sim( config );
        ASSERT_TRUE( sim.IsReady() );
        sim.RunUntil( 30.0 );
        TJitterBuffer::SStatistics bufferStats;
        sim.Buffer().GetStatistics( bufferStats );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceJitter: real time run: restarts " << bufferStats.restarts << ", overflow drops " << bufferStats.overflowDropped << ", talkspurt ends " << bufferStats.talkspurtEnds );
        ASSERT_TRUE( 1 == bufferStats.restarts );

        // the buffer on its own, with forward error correction and redundancy, and resets
        CMarkerVoiceDecoder decoder( true );
        TJitterBuffer buffer;
        TController controller;
        ASSERT_TRUE( buffer.Prepare( 64, FrameSize, Rate ) );
        ASSERT_TRUE( controller.Prepare( ProfileSettings( AUDIO::VOICEPLAYOUTPROFILE_WIRED ), buffer, decoder ) );
        AUDIO::CVoiceFrame frame = MakeFrame( 1, 480, 1 );
        AUDIO::CVoiceFrame previous = MakeFrame( 0, 0, 0 );
        ASSERT_TRUE( frame.SetRedundantBlock( 480, 112, previous.GetPayload(), previous.GetPayloadSize() ) );
        TJitterBuffer::SInsertInfo info;
        TJitterBuffer::SPullInfo pull;
        ASSERT_TRUE( TJitterBuffer::INSERT_STORED == InsertRealtime( buffer, frame, 1000, info ) );
        ASSERT_TRUE( buffer.StartPlayout() );
        ASSERT_TRUE( TJitterBuffer::PULL_FRAME == PullRealtime( buffer, buffer.GetNextTimestamp(), buffer.GetNextTimestamp(), pull ) );
        ResetRealtime( buffer, controller );
        ASSERT_TRUE( 0 == buffer.GetStoredFrameCount() );
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
PerformAudioVoiceJitterTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioVoiceJitter TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.VoiceJitter" );

    PerformJitterBufferTests();
    PerformPlayoutTests();
    PerformDriftTests();
    PerformRealtimeTests();
}

/*-------------------------------------------------------------------------*/
