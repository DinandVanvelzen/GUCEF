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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefAUDIO_CVoicePlayoutSource.h"

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

const UInt32 HistoryFrames = CSincResampler::HALF_WIDTH;

/** The jitter buffer counts sequence cycles from 1 (so a frame before the first never goes below 0); RTCP from 0 */
const UInt64 FirstSequenceCycle = 65536;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVoicePlayoutSource::SInboundFrame::SInboundFrame( void )
    : frame()
    , arrivalTimeInUs( 0 )
    , generation( 0 )
{

}

/*-------------------------------------------------------------------------*/

CVoicePlayoutSource::CVoicePlayoutSource( void )
    : IAudioSource()
    , m_prepared( false )
    , m_format()
    , m_rate( 0 )
    , m_frameSize( 0 )
    , m_lookahead( 0 )
    , m_timestampRate( 0 )
    , m_clock( &CVoiceClock::GetSystemClock() )
    , m_decoderRef()
    , m_inbound( GUCEF_NULL )
    , m_inboundLimit( 0 )
    , m_generation( 0 )
    , m_state( (UInt32) VOICEPLAYOUTSTATE_IDLE )
    , m_position( 0 )
    , m_framesQueued( 0 )
    , m_inboundQueueFull( 0 )
    , m_meter()
    , m_pushItem()
    , m_buffer()
    , m_controller()
    , m_resampler()
    , m_fifo()
    , m_fifoCount( 0 )
    , m_readOffset( 0.0 )
    , m_resamplerOffset( 0 )
    , m_resampling( false )
    , m_appliedGeneration( 0 )
    , m_popItem()
    , m_hasRequest( false )
    , m_lastClockUs( 0 )
    , m_lastNowUs( 0.0 )
    , m_lastDurationUs( 0.0 )
    , m_requestNowUs( 0 )
    , m_requestFrames( 0 )
    , m_oneWayDelaySum( 0.0 )
    , m_stats()
    , m_controllerStats()
    , m_bufferStats()
    , m_statsExchange()
    , m_statsReadLock()
    , m_statsRead()
{GUCEF_TRACE;

    memset( m_primeScratch, 0, sizeof( m_primeScratch ) );
}

/*-------------------------------------------------------------------------*/

CVoicePlayoutSource::~CVoicePlayoutSource()
{GUCEF_TRACE;

    GUCEF_DELETE m_inbound;
    m_inbound = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CVoicePlayoutSource::Prepare( const CVoiceReceiverConfig& config, const TVoiceDecoderPtr& decoder, const CVoiceClock& clock )
{GUCEF_TRACE;

    m_prepared = false;
    if ( decoder.IsNULL() )
        return false;

    const UInt64 rate = decoder->GetOutputRate();
    const UInt64 timestampRate = decoder->GetTimestampRate();
    const UInt64 durationInUs = decoder->GetConfig().GetFrameDurationInUs();
    if ( 0 == rate || 0 == timestampRate || 0 == durationInUs ||
         0 != ( rate * durationInUs ) % 1000000ULL || 0 != ( timestampRate * durationInUs ) % 1000000ULL )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VoicePlayoutSource: The decoder has frames that are no whole number of samples or timestamp units" );
        return false;
    }
    const UInt32 frameSize = (UInt32) ( rate * durationInUs / 1000000ULL );
    const UInt32 unitsPerFrame = (UInt32) ( timestampRate * durationInUs / 1000000ULL );

    if ( !m_buffer.Prepare( config.GetSlotCount(), unitsPerFrame, (UInt32) timestampRate ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VoicePlayoutSource: The jitter buffer refused " << config.GetSlotCount() << " slots" );
        return false;
    }

    CVoicePlayoutController::SSettings settings;
    settings.SetProfile( config.GetPlayoutProfile() );
    settings.initialMarginInMs = config.GetInitialMarginInMs();
    settings.minMarginInMs = config.GetMinMarginInMs();
    settings.maxMarginInMs = config.GetMaxMarginInMs();
    settings.windowInS = config.GetWindowInS();
    settings.lateQuantile = config.GetLatePercentage() / 100.0;
    settings.concealLimit = config.GetConcealLimit();
    TVoiceDecoderPtr decoderRef( decoder );
    if ( !m_controller.Prepare( settings, m_buffer, *decoderRef.GetPointerAlways() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VoicePlayoutSource: The playout controller refused its settings" );
        return false;
    }
    if ( !m_resampler.Configure( 1, (Float64) rate, (Float64) rate ) )
        return false;

    // the resampler needs this much input beyond its output at the largest ratio; the request asks for it in both paths,
    // so a change between the paths does not move the margin
    m_lookahead = (UInt32) ceil( (Float64) CSincResampler::HALF_WIDTH * ( 1.0 + settings.maxRatioDeviation ) );
    const UInt32 fifoCapacity = HistoryFrames + (UInt32) ceil( (Float64) MaxRequestFrames * ( 1.0 + settings.maxRatioDeviation ) ) +
                                2 * m_lookahead + 2 * frameSize + 64;
    m_fifo.assign( fifoCapacity, 0.0f );

    GUCEF_DELETE m_inbound;
    m_inbound = GUCEF_NEW TInboundQueue( config.GetInboundQueueFrames() );
    m_inboundLimit = config.GetInboundQueueFrames();
    m_decoderRef = decoder;
    m_clock = &clock;
    m_rate = (UInt32) rate;
    m_frameSize = frameSize;
    m_timestampRate = (UInt32) timestampRate;
    m_format = CAudioFormat( m_rate, 1, SAMPLETYPE_FLOAT32 );
    m_meter.Prepare( 1, m_rate );
    m_appliedGeneration = m_generation.Load( MT::MEMORDER_ACQUIRE );
    m_pushItem.generation = m_appliedGeneration;
    m_stats.Clear();
    m_oneWayDelaySum = 0.0;
    ResetPlayout();
    m_prepared = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVoicePlayoutSource::IsPrepared( void ) const
{GUCEF_TRACE;

    return m_prepared;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoicePlayoutSource::GetTimestampRate( void ) const
{GUCEF_TRACE;

    return m_timestampRate;
}

/*-------------------------------------------------------------------------*/

bool
CVoicePlayoutSource::PushFrame( const CVoiceFrame& frame, UInt64 arrivalTimeInUs )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_inbound )
        return false;

    // the queue rounds its size up to a power of two: the configured size is the limit (the size only shrinks while
    // this thread looks at it)
    if ( m_inbound->GetSize() >= m_inboundLimit )
    {
        m_inboundQueueFull.FetchAdd( 1, MT::MEMORDER_RELAXED );
        return false;
    }
    m_pushItem.frame = frame;
    m_pushItem.arrivalTimeInUs = arrivalTimeInUs;
    m_pushItem.generation = m_generation.Load( MT::MEMORDER_ACQUIRE );
    if ( !m_inbound->Write( m_pushItem ) )
    {
        m_inboundQueueFull.FetchAdd( 1, MT::MEMORDER_RELAXED );
        return false;
    }
    m_framesQueued.FetchAdd( 1, MT::MEMORDER_RELAXED );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVoicePlayoutSource::RequestReset( void )
{GUCEF_TRACE;

    m_generation.FetchAdd( 1 );
}

/*-------------------------------------------------------------------------*/

TVoicePlayoutState
CVoicePlayoutSource::GetPlayoutState( void ) const
{GUCEF_TRACE;

    return (TVoicePlayoutState) m_state.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

bool
CVoicePlayoutSource::IsPlaying( void ) const
{GUCEF_TRACE;

    return VOICEPLAYOUTSTATE_PLAYING == GetPlayoutState();
}

/*-------------------------------------------------------------------------*/

Float32
CVoicePlayoutSource::GetLevelInDecibels( void ) const
{GUCEF_TRACE;

    return m_meter.GetRmsInDecibels( 0 );
}

/*-------------------------------------------------------------------------*/

UInt32
CVoicePlayoutSource::GetPendingFrameCount( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_inbound ? m_inbound->GetSize() : 0;
}

/*-------------------------------------------------------------------------*/

void
CVoicePlayoutSource::GetStats( CVoiceReceiverStats& stats ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_statsReadLock );
    m_statsExchange.Read( m_statsRead );
    stats = m_statsRead;
    stats.framesQueued = m_framesQueued.Load( MT::MEMORDER_RELAXED );
    stats.inboundQueueFull = m_inboundQueueFull.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

const CAudioFormat&
CVoicePlayoutSource::GetFormat( void ) const
{
    return m_format;
}

/*-------------------------------------------------------------------------*/

bool
CVoicePlayoutSource::IsEndOfStream( void ) const
{
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVoicePlayoutSource::IsRealtimeSafe( void ) const
{
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
CVoicePlayoutSource::GetPositionInFrames( void ) const
{
    return m_position.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CVoicePlayoutSource::ResetPlayout( void )
{
    // no trace macro: audio thread

    m_controller.Reset();
    m_resampler.Reset();
    m_resampling = false;

    // the FIFO starts with a history of silence, so the resampler can always be primed from it
    memset( &m_fifo[ 0 ], 0, (size_t) HistoryFrames * sizeof( Float32 ) );
    m_fifoCount = HistoryFrames;
    m_readOffset = (Float64) HistoryFrames;
    m_resamplerOffset = 0;
    m_hasRequest = false;
    m_oneWayDelaySum = 0.0;
    m_stats.oneWayDelayCount = 0;
    m_stats.lastOneWayDelayInMs = 0.0;
    m_stats.meanOneWayDelayInMs = 0.0;
    m_stats.maxOneWayDelayInMs = 0.0;
}

/*-------------------------------------------------------------------------*/

UInt64
CVoicePlayoutSource::RequestTime( UInt32 frames )
{
    // no trace macro: audio thread

    // A piece that follows the previous one within the same callback of the device (the clock moved less than half of
    // the previous piece) is output right after it; otherwise the request is at the time of the clock
    const UInt64 clockNow = m_clock->GetTimeInUs();
    Float64 now = (Float64) clockNow;
    if ( m_hasRequest && clockNow >= m_lastClockUs && (Float64) ( clockNow - m_lastClockUs ) < 0.5 * m_lastDurationUs )
        now = m_lastNowUs + m_lastDurationUs;
    m_hasRequest = true;
    m_lastClockUs = clockNow;
    m_lastNowUs = now;
    m_lastDurationUs = (Float64) frames * 1000000.0 / (Float64) m_rate;
    return (UInt64) ( now + 0.5 );
}

/*-------------------------------------------------------------------------*/

void
CVoicePlayoutSource::DrainInbound( void )
{
    // no trace macro: audio thread

    const UInt32 limit = m_inbound->GetCapacity();
    for ( UInt32 i=0; i<limit; ++i )
    {
        if ( !m_inbound->Read( m_popItem ) )
            break;

        if ( m_popItem.generation != m_appliedGeneration )
        {
            if ( (Int32) ( m_popItem.generation - m_appliedGeneration ) < 0 )
                continue;           // pushed before a reset: not part of the new stream

            // a reset came between the start of this request and this frame: carry it out and start the request again
            m_appliedGeneration = m_popItem.generation;
            ResetPlayout();
            ++m_stats.resets;
            const UInt32 pending = (Float64) m_fifoCount > m_readOffset ? (UInt32) floor( (Float64) m_fifoCount - m_readOffset ) : 0;
            m_controller.BeginRequest( m_requestNowUs, m_requestFrames, pending );
        }

        // a redundant block of another payload type is no frame of this codec
        CVoiceFrame& frame = m_popItem.frame;
        if ( frame.HasRedundantBlock() && frame.GetRedundantPayloadType() != frame.GetPayloadType() )
            frame.ClearRedundantBlock();
        m_controller.Insert( frame, m_popItem.arrivalTimeInUs );
    }
}

/*-------------------------------------------------------------------------*/

bool
CVoicePlayoutSource::ProduceMore( UInt32 deficit, UInt64 nowInUs, Float64 ratio )
{
    // no trace macro: audio thread

    const UInt32 room = (UInt32) m_fifo.size() - m_fifoCount;
    if ( room < m_frameSize )
        return false;

    UInt32 offer = deficit < room ? deficit : room;
    if ( VOICEPLAYOUTSTATE_PLAYING == m_controller.GetState() || offer < 1 )
        offer = m_frameSize;

    CVoicePlayoutController::SProduceInfo info;
    Float32* pcm = &m_fifo[ m_fifoCount ];
    UInt32 made = m_controller.Produce( pcm, offer, &info );
    if ( 0 == made )
        made = m_controller.Produce( pcm, offer > m_frameSize ? offer : m_frameSize, &info );
    if ( 0 == made )
        return false;

    if ( CVoicePlayoutController::PRODUCED_DECODED == info.kind && 0 != info.captureTickCount )
    {
        // one-way delay: from the capture of the frame's first sample to its output, where the clocks compare
        UInt64 captureInUs = 0;
        if ( m_clock->CaptureTickToTimeInUs( info.captureTickCount, captureInUs ) )
        {
            const Float64 outputInUs = (Float64) nowInUs + ( (Float64) m_fifoCount - m_readOffset ) * 1000000.0 / (Float64) m_rate / ratio;
            const Float64 delayInMs = ( outputInUs - (Float64) captureInUs ) / 1000.0;
            ++m_stats.oneWayDelayCount;
            m_oneWayDelaySum += delayInMs;
            m_stats.lastOneWayDelayInMs = delayInMs;
            m_stats.meanOneWayDelayInMs = m_oneWayDelaySum / (Float64) m_stats.oneWayDelayCount;
            if ( 1 == m_stats.oneWayDelayCount || delayInMs > m_stats.maxOneWayDelayInMs )
                m_stats.maxOneWayDelayInMs = delayInMs;
        }
    }
    m_fifoCount += made;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVoicePlayoutSource::PrimeResampler( void )
{
    // no trace macro: audio thread

    // The resampler starts at the next output sample R (whole, the direct path never leaves a fraction) with the frames
    // before it as history: it is reset and given the HistoryFrames frames before R and as many after it, and the
    // outputs of the frames before R are thrown away. Its next output is then the one at R
    const UInt32 start = (UInt32) m_readOffset;
    while ( m_fifoCount < start + HistoryFrames && m_fifoCount < (UInt32) m_fifo.size() )
        m_fifo[ m_fifoCount++ ] = 0.0f;

    m_resampler.Reset();
    m_resampler.SetRates( (Float64) m_rate, (Float64) m_rate );
    UInt32 consumed = 0, made = 0;
    m_resampler.Process( &m_fifo[ start - HistoryFrames ], 2 * HistoryFrames, consumed, m_primeScratch, HistoryFrames, made );
    m_resamplerOffset = start - HistoryFrames + consumed;
    m_resampling = true;
    ++m_stats.resamplerSwitches;
}


/*-------------------------------------------------------------------------*/

void
CVoicePlayoutSource::CompactFifo( void )
{
    // no trace macro: audio thread

    const Float64 keep = floor( m_readOffset ) - (Float64) HistoryFrames;
    UInt32 drop = keep > 0.0 ? (UInt32) keep : 0;
    if ( m_resampling && m_resamplerOffset < drop )
        drop = m_resamplerOffset;
    if ( drop > m_fifoCount )
        drop = m_fifoCount;
    if ( 0 == drop )
        return;

    memmove( &m_fifo[ 0 ], &m_fifo[ drop ], (size_t) ( m_fifoCount - drop ) * sizeof( Float32 ) );
    m_fifoCount -= drop;
    m_readOffset -= (Float64) drop;
    m_resamplerOffset = m_resampling ? m_resamplerOffset - drop : 0;
}

/*-------------------------------------------------------------------------*/

void
CVoicePlayoutSource::RenderPiece( Float32* output, UInt32 frames, UInt32 remaining, bool first )
{
    // no trace macro: audio thread

    // 1. the request: what the device asks for is all of the rest of the call, which must be there now (the margin rules
    //    count it as the block); this piece starts where the previous one ended
    const UInt64 now = RequestTime( frames );
    const UInt32 pending = (Float64) m_fifoCount > m_readOffset ? (UInt32) floor( (Float64) m_fifoCount - m_readOffset ) : 0;
    m_requestNowUs = now;
    m_requestFrames = remaining + m_lookahead;
    m_controller.BeginRequest( now, m_requestFrames, pending );

    // 2. what arrived
    if ( first )
        DrainInbound();

    // 3. the control step
    const Float64 ratio = m_controller.UpdateRatio();
    const bool resample = m_resampling || 1.0 != ratio;

    // 4. decoded audio for the request and the lookahead of the resampler
    const Float64 step = resample ? ratio : 1.0;
    const Float64 target = m_readOffset + (Float64) frames * step + (Float64) m_lookahead;
    while ( (Float64) m_fifoCount < target - 1.0e-9 )
    {
        if ( !ProduceMore( (UInt32) ceil( target - (Float64) m_fifoCount ), now, ratio ) )
            break;
    }

    // 5. the output
    if ( !resample )
    {
        // ratio exactly 1: the decoded audio as it is, bit for bit
        const UInt32 start = (UInt32) m_readOffset;
        const UInt32 available = m_fifoCount > start ? m_fifoCount - start : 0;
        const UInt32 copied = available < frames ? available : frames;
        if ( copied > 0 )
            memcpy( output, &m_fifo[ start ], (size_t) copied * sizeof( Float32 ) );
        if ( copied < frames )
        {
            memset( output + copied, 0, (size_t) ( frames - copied ) * sizeof( Float32 ) );
            ++m_stats.outputShortfalls;
        }
        m_readOffset += (Float64) copied;
    }
    else
    {
        if ( !m_resampling )
            PrimeResampler();
        m_resampler.SetRates( ratio * (Float64) m_rate, (Float64) m_rate );

        UInt32 made = 0;
        UInt32 attempts = 0;
        while ( made < frames && attempts++ < 8 )
        {
            UInt32 consumed = 0, producedNow = 0;
            const UInt32 available = m_fifoCount > m_resamplerOffset ? m_fifoCount - m_resamplerOffset : 0;
            if ( available > 0 )
            {
                m_resampler.Process( &m_fifo[ m_resamplerOffset ], available, consumed, output + made, frames - made, producedNow );
                m_resamplerOffset += consumed;
                made += producedNow;
            }
            if ( made < frames && !ProduceMore( m_frameSize, now, ratio ) )
                break;
        }
        if ( made < frames )
        {
            memset( output + made, 0, (size_t) ( frames - made ) * sizeof( Float32 ) );
            ++m_stats.outputShortfalls;
        }
        m_readOffset += (Float64) made * ratio;
    }

    CompactFifo();
}

/*-------------------------------------------------------------------------*/

UInt32
CVoicePlayoutSource::ReadFrames( void* destination, UInt32 frameCapacity )
{
    // no trace macro: audio thread

    Float32* output = static_cast< Float32* >( destination );
    if ( GUCEF_NULL == output || 0 == frameCapacity )
        return 0;
    if ( !m_prepared )
    {
        memset( output, 0, (size_t) frameCapacity * sizeof( Float32 ) );
        return frameCapacity;
    }

    // a reset asked for since the last request
    const UInt32 generation = m_generation.Load( MT::MEMORDER_ACQUIRE );
    if ( generation != m_appliedGeneration )
    {
        m_appliedGeneration = generation;
        ResetPlayout();
        ++m_stats.resets;
    }

    UInt32 done = 0;
    bool first = true;
    while ( done < frameCapacity )
    {
        const UInt32 piece = ( frameCapacity - done < MaxRequestFrames ) ? frameCapacity - done : MaxRequestFrames;
        RenderPiece( output + done, piece, frameCapacity - done, first );
        first = false;
        done += piece;
    }

    m_position.Store( m_position.Load( MT::MEMORDER_RELAXED ) + frameCapacity, MT::MEMORDER_RELAXED );
    ++m_stats.renderRequests;
    m_stats.framesOutput += frameCapacity;
    m_meter.Update( output, frameCapacity );
    PublishStats();
    return frameCapacity;
}

/*-------------------------------------------------------------------------*/

void
CVoicePlayoutSource::PublishStats( void )
{
    // no trace macro: audio thread

    m_controller.GetStatistics( m_controllerStats );
    m_buffer.GetStatistics( m_bufferStats );

    m_stats.framesReceived = m_bufferStats.framesReceived;
    m_stats.framesExpected = m_bufferStats.framesExpected;
    m_stats.cumulativeLost = m_bufferStats.cumulativeLost;
    m_stats.extendedHighestSequenceNumber = m_bufferStats.extendedHighestSequenceNumber >= FirstSequenceCycle ?
                                                m_bufferStats.extendedHighestSequenceNumber - FirstSequenceCycle : m_bufferStats.extendedHighestSequenceNumber;
    m_stats.interarrivalJitter = m_bufferStats.interarrivalJitter;
    m_stats.interarrivalJitterInMs = m_timestampRate > 0 ? m_bufferStats.interarrivalJitter * 1000.0 / (Float64) m_timestampRate : 0.0;
    m_stats.duplicates = m_bufferStats.duplicates;
    m_stats.reordered = m_bufferStats.reordered;
    m_stats.lateDropped = m_bufferStats.late;
    m_stats.redundantStored = m_bufferStats.redundantStored;
    m_stats.overflowDropped = m_bufferStats.overflowDropped;
    m_stats.restarts = m_bufferStats.restarts;
    m_stats.bufferedFrames = m_bufferStats.bufferedFrames;

    m_stats.state = m_controllerStats.state;
    m_stats.framesDecoded = m_controllerStats.framesDecoded;
    m_stats.framesConcealed = m_controllerStats.framesConcealed;
    m_stats.framesRecovered = m_controllerStats.framesRecovered;
    m_stats.framesAccelerated = m_controllerStats.framesAccelerated;
    m_stats.framesStretched = m_controllerStats.framesStretched;
    m_stats.lateFrames = m_controllerStats.lateFrames;
    m_stats.underruns = m_controllerStats.underruns;
    m_stats.decodeErrors = m_controllerStats.decodeErrors;
    m_stats.talkspurts = m_controllerStats.talkspurts;
    m_stats.marginInMs = m_controllerStats.marginInS * 1000.0;
    m_stats.hasMinimumLevel = m_controllerStats.hasMinimumLevel;
    m_stats.minimumLevelInMs = m_controllerStats.minimumLevelInS * 1000.0;
    m_stats.levelInMs = m_controllerStats.levelInS * 1000.0;
    m_stats.ratioInPpm = ( m_controllerStats.ratio - 1.0 ) * 1.0e6;
    m_stats.integralInPpm = m_controllerStats.integral * 1.0e6;
    m_stats.lastPlayoutDelayInMs = m_controllerStats.lastPlayoutDelayInS * 1000.0;
    m_stats.meanPlayoutDelayInMs = m_controllerStats.meanPlayoutDelayInS * 1000.0;
    m_stats.maxPlayoutDelayInMs = m_controllerStats.maxPlayoutDelayInS * 1000.0;
    m_stats.resampling = m_resampling;

    m_statsExchange.Write( m_stats );
    m_state.Store( (UInt32) m_controllerStats.state, MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
