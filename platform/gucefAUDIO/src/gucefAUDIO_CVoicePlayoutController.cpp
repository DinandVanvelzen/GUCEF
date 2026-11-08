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

#ifndef GUCEF_AUDIO_CVOICEDECODER_H
#include "gucefAUDIO_CVoiceDecoder.h"
#define GUCEF_AUDIO_CVOICEDECODER_H
#endif /* GUCEF_AUDIO_CVOICEDECODER_H ? */

#ifndef GUCEF_AUDIO_CVOICECODECCONFIG_H
#include "gucefAUDIO_CVoiceCodecConfig.h"
#define GUCEF_AUDIO_CVOICECODECCONFIG_H
#endif /* GUCEF_AUDIO_CVOICECODECCONFIG_H ? */

#include "gucefAUDIO_CVoicePlayoutController.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

inline Float64
Clamp( Float64 value, Float64 low, Float64 high )
{
    return value < low ? low : ( value > high ? high : value );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVoicePlayoutController::SSettings::SSettings( void )
    : initialMarginInMs( 20.0 )
    , minMarginInMs( 6.0 )
    , maxMarginInMs( 200.0 )
    , windowInS( 4.0 )
    , decayIntervalInS( 1.0 )
    , decayStepInMs( 1.0 )
    , jitterMarginFactor( 0.0 )
    , concealLimit( 6 )
    , proportionalGain( 0.2 )
    , integralTimeInS( 30.0 )
    , maxRatioDeviation( 0.006 )
    , integralLimit( 0.006 )
    , maxRatioStepPerRequest( 0.00001 )
    , silenceThresholdInDb( -60.0 )
    , excessFramesForDrop( 2.0 )
    , lateQuantile( 0.01 )
{

}

/*-------------------------------------------------------------------------*/

void
CVoicePlayoutController::SSettings::SetProfile( TVoicePlayoutProfile profile )
{
    if ( VOICEPLAYOUTPROFILE_WIRED == profile )
    {
        initialMarginInMs = 4.0;
        minMarginInMs = 2.0;
        maxMarginInMs = 60.0;
        windowInS = 2.0;
        decayIntervalInS = 10.0;
        jitterMarginFactor = 3.0;
        lateQuantile = 0.0;
    }
    else
    {
        initialMarginInMs = 20.0;
        minMarginInMs = 6.0;
        maxMarginInMs = 200.0;
        windowInS = 4.0;
        decayIntervalInS = 1.0;
        jitterMarginFactor = 0.0;
        lateQuantile = 0.01;
    }
}

/*-------------------------------------------------------------------------*/

CVoicePlayoutController::SProduceInfo::SProduceInfo( void )
    : kind( PRODUCED_SILENCE )
    , extendedTimestamp( 0 )
    , extendedSequenceNumber( 0 )
    , arrivalTimeInUs( 0 )
    , captureTickCount( 0 )
{

}

/*-------------------------------------------------------------------------*/

CVoicePlayoutController::SStatistics::SStatistics( void )
    : state( VOICEPLAYOUTSTATE_IDLE )
    , framesDecoded( 0 )
    , framesConcealed( 0 )
    , framesRecovered( 0 )
    , framesAccelerated( 0 )
    , framesStretched( 0 )
    , underruns( 0 )
    , lateFrames( 0 )
    , decodeErrors( 0 )
    , talkspurts( 0 )
    , ratio( 1.0 )
    , integral( 0.0 )
    , marginInS( 0.0 )
    , minimumLevelInS( 0.0 )
    , hasMinimumLevel( false )
    , levelInS( 0.0 )
    , lastPlayoutDelayInS( 0.0 )
    , meanPlayoutDelayInS( 0.0 )
    , maxPlayoutDelayInS( 0.0 )
    , jitterInS( 0.0 )
{

}

/*-------------------------------------------------------------------------*/

CVoicePlayoutController::CVoicePlayoutController( void )
    : m_settings()
    , m_buffer( GUCEF_NULL )
    , m_decoder( GUCEF_NULL )
    , m_outputRate( 0 )
    , m_frameSize( 0 )
    , m_timestampUnitsPerFrame( 0 )
    , m_timestampRate( 0 )
    , m_frameDurationInS( 0.0 )
    , m_kp( 0.0 )
    , m_ki( 0.0 )
    , m_silenceThreshold( 0.0 )
    , m_slackWindow()
    , m_state( VOICEPLAYOUTSTATE_IDLE )
    , m_concealed( 0 )
    , m_concealedForEmpty( false )
    , m_emptySince( 0 )
    , m_hasSlack( false )
    , m_timelineShift( 0.0 )
    , m_margin( 0.0 )
    , m_integral( 0.0 )
    , m_ratio( 1.0 )
    , m_sinceLateInS( 0.0 )
    , m_redundancyFloor( 0.0 )
    , m_sinceRedundancyInS( 0.0 )
    , m_fecFloor( 0.0 )
    , m_level( 0.0 )
    , m_hasRequest( false )
    , m_nowInUs( 0 )
    , m_blockInS( 0.0 )
    , m_pending( 0 )
    , m_produced( 0 )
    , m_elapsedInS( 0.0 )
    , m_delaySum( 0.0 )
    , m_delayCount( 0 )
    , m_stats()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVoicePlayoutController::~CVoicePlayoutController()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CVoicePlayoutController::Prepare( const SSettings& settings, CVoiceJitterBuffer& buffer, CVoiceDecoder& decoder )
{GUCEF_TRACE;

    if ( !buffer.IsPrepared() )
        return false;
    if ( !( settings.windowInS > 0.0 ) || !( settings.decayIntervalInS > 0.0 ) || settings.minMarginInMs < 0.0 ||
         settings.maxMarginInMs < settings.minMarginInMs || settings.maxRatioDeviation < 0.0 || settings.integralLimit < 0.0 ||
         settings.maxRatioStepPerRequest < 0.0 || settings.proportionalGain < 0.0 || 0 == settings.concealLimit ||
         !( settings.lateQuantile >= 0.0 && settings.lateQuantile < 0.5 ) )
        return false;

    const UInt64 outputRate = decoder.GetOutputRate();
    const UInt64 durationInUs = decoder.GetConfig().GetFrameDurationInUs();
    if ( 0 == outputRate || 0 == durationInUs || 0 != ( outputRate * durationInUs ) % 1000000 )
        return false;

    m_settings = settings;
    m_buffer = &buffer;
    m_decoder = &decoder;
    m_outputRate = (UInt32) outputRate;
    m_frameSize = (UInt32) ( outputRate * durationInUs / 1000000 );
    m_timestampUnitsPerFrame = buffer.GetTimestampUnitsPerFrame();
    m_timestampRate = buffer.GetTimestampRate();
    m_frameDurationInS = (Float64) durationInUs / 1000000.0;
    m_kp = settings.proportionalGain;
    m_ki = settings.integralTimeInS > 0.0 ? settings.proportionalGain / settings.integralTimeInS : 0.0;
    m_silenceThreshold = pow( 10.0, settings.silenceThresholdInDb / 20.0 );

    // in-band forward error correction recovers a lost frame from the next one, which must be there by then
    m_fecFloor = 0.0;
    if ( decoder.SupportsFec() && decoder.GetConfig().GetInbandFec() )
        m_fecFloor = m_frameDurationInS < settings.maxMarginInMs / 1000.0 ? m_frameDurationInS : settings.maxMarginInMs / 1000.0;

    // one slack per arriving frame: the window holds the frames of 'windowInS'
    const UInt32 windowValues = (UInt32) ceil( settings.windowInS / m_frameDurationInS - 1.0e-9 );
    if ( !m_slackWindow.Prepare( windowValues < 1 ? 1 : windowValues ) )
        return false;

    Reset();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVoicePlayoutController::IsPrepared( void ) const
{
    return GUCEF_NULL != m_buffer;
}

/*-------------------------------------------------------------------------*/

const CVoicePlayoutController::SSettings&
CVoicePlayoutController::GetSettings( void ) const
{
    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CVoicePlayoutController::Reset( void )
{
    if ( GUCEF_NULL == m_buffer )
        return;

    m_buffer->Reset();
    m_decoder->Reset();
    m_state = VOICEPLAYOUTSTATE_IDLE;
    m_concealed = 0;
    m_concealedForEmpty = false;
    m_slackWindow.Reset();
    m_hasSlack = false;
    m_timelineShift = 0.0;
    m_margin = Clamp( m_settings.initialMarginInMs, m_settings.minMarginInMs, m_settings.maxMarginInMs ) / 1000.0;
    if ( m_margin < m_fecFloor )
        m_margin = m_fecFloor;
    m_integral = 0.0;
    m_ratio = 1.0;
    m_sinceLateInS = 0.0;
    m_redundancyFloor = 0.0;
    m_sinceRedundancyInS = 0.0;
    m_level = 0.0;
    m_hasRequest = false;
    m_nowInUs = 0;
    m_blockInS = 0.0;
    m_pending = 0;
    m_produced = 0;
    m_elapsedInS = 0.0;
    m_delaySum = 0.0;
    m_delayCount = 0;
    m_stats = SStatistics();
}

/*-------------------------------------------------------------------------*/

void
CVoicePlayoutController::BeginRequest( UInt64 nowInUs, UInt32 requestFrames, UInt32 pendingFrames )
{
    if ( 0 == m_outputRate )
        return;

    const Float64 block = (Float64) requestFrames / (Float64) m_outputRate;
    if ( m_hasRequest && nowInUs >= m_nowInUs )
        m_elapsedInS = (Float64) ( nowInUs - m_nowInUs ) / 1000000.0;
    else
        m_elapsedInS = block;
    m_hasRequest = true;
    m_nowInUs = nowInUs;
    m_blockInS = block;
    m_pending = pendingFrames;
    m_produced = 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CVoicePlayoutController::GetPlayoutTimestamp( void ) const
{
    return m_buffer->GetNextTimestamp() + (UInt64) m_concealed * (UInt64) m_timestampUnitsPerFrame;
}

/*-------------------------------------------------------------------------*/

Float64
CVoicePlayoutController::GetOutputPositionTimestamp( void ) const
{
    // the timestamp that is output at the start of the current request: the next frame to make, minus what was made but not output
    const Float64 unitsPerSample = (Float64) m_timestampRate / (Float64) m_outputRate;
    return (Float64) GetPlayoutTimestamp() - (Float64) ( m_pending + m_produced ) * unitsPerSample;
}

/*-------------------------------------------------------------------------*/

Float64
CVoicePlayoutController::GetSlack( UInt64 extendedTimestamp, UInt64 arrivalTimeInUs ) const
{
    // the frame is output at now + ( its timestamp - the output position ) / rate / ratio; the first request that can
    // need it starts one block before that; the slack is how long before that it arrived
    const Float64 ahead = ( (Float64) extendedTimestamp - GetOutputPositionTimestamp() ) / (Float64) m_timestampRate / m_ratio;
    const Float64 sinceArrival = ( (Float64) (Int64) ( m_nowInUs - arrivalTimeInUs ) ) / 1000000.0;
    return ahead - m_blockInS + sinceArrival;
}

/*-------------------------------------------------------------------------*/

void
CVoicePlayoutController::PushSlack( Float64 slackInS )
{
    // the window holds the slack relative to the timeline at the start of the playout (Float64: the shift may grow)
    m_slackWindow.Push( slackInS - m_timelineShift );
    m_hasSlack = true;
}

/*-------------------------------------------------------------------------*/

void
CVoicePlayoutController::ShiftTimeline( Float64 seconds )
{
    m_timelineShift += seconds;
}

/*-------------------------------------------------------------------------*/

void
CVoicePlayoutController::GrowMargin( Float64 latenessInS, bool stretched )
{
    // With a quantile above 0 a late frame that was dropped (a newer one was there) is expected: its slack is part of
    // the window, so it lowers the level and the ratio adds the delay it asks for; growing the margin as well would
    // count it twice and pins the margin at its maximum on a heavy-tailed delay. A stretch (the buffer ran dry) is a
    // shortfall of the whole playout and grows the margin as it does with a quantile of 0, by the lateness rounded up
    // to a frame: the error the ratio sees then stays where it was instead of jumping with the stretch, so the integral
    // keeps learning the drift (the margin decays again afterwards)
    if ( m_settings.lateQuantile > 0.0 && !stretched )
        return;

    Float64 frames = ceil( latenessInS / m_frameDurationInS - 1.0e-6 );
    if ( frames < 1.0 )
        frames = 1.0;
    m_margin += frames * m_frameDurationInS;
    const Float64 maxMargin = m_settings.maxMarginInMs / 1000.0;
    if ( m_margin > maxMargin )
        m_margin = maxMargin;
    m_sinceLateInS = 0.0;
}

/*-------------------------------------------------------------------------*/

bool
CVoicePlayoutController::GetMinimumLevel( Float64& levelInS ) const
{
    if ( !m_hasSlack )
        return false;
    levelInS = m_slackWindow.GetQuantile( m_settings.lateQuantile ) + m_timelineShift;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVoicePlayoutController::StopPlayout( TVoicePlayoutState nextState )
{
    m_buffer->StopPlayout();
    m_state = nextState;
    m_concealed = 0;
    m_concealedForEmpty = false;
    m_emptySince = 0;
}

/*-------------------------------------------------------------------------*/

CVoiceJitterBuffer::EInsertResult
CVoicePlayoutController::Insert( const CVoiceFrame& frame, UInt64 arrivalTimeInUs )
{
    if ( GUCEF_NULL == m_buffer )
        return CVoiceJitterBuffer::INSERT_NOT_PREPARED;

    // A stream with RFC 2198 redundancy brings the copy of a lost frame with the next one, a timestamp offset later:
    // while it does, the margin is at least that offset, or the copy would come too late to be of use
    if ( frame.HasRedundantBlock() && m_timestampRate > 0 )
    {
        const Float64 offset = (Float64) frame.GetRedundantTimestampOffset() / (Float64) m_timestampRate;
        const Float64 floor = offset < m_settings.maxMarginInMs / 1000.0 ? offset : m_settings.maxMarginInMs / 1000.0;
        if ( floor > m_redundancyFloor )
            m_redundancyFloor = floor;
        if ( m_margin < m_redundancyFloor )
            m_margin = m_redundancyFloor;
        m_sinceRedundancyInS = 0.0;
    }

    CVoiceJitterBuffer::SInsertInfo info;
    const CVoiceJitterBuffer::EInsertResult result = m_buffer->Insert( frame, arrivalTimeInUs, info );
    switch ( result )
    {
        case CVoiceJitterBuffer::INSERT_STORED:
        case CVoiceJitterBuffer::INSERT_REORDERED:
        case CVoiceJitterBuffer::INSERT_OVERFLOW:
        {
            if ( VOICEPLAYOUTSTATE_PLAYING == m_state )
            {
                if ( !m_buffer->IsPlaying() )
                    StopPlayout( VOICEPLAYOUTSTATE_PREBUFFER );     // the window moved under the playout: start over
                else
                    PushSlack( GetSlack( info.extendedTimestamp, arrivalTimeInUs ) );
            }
            if ( VOICEPLAYOUTSTATE_IDLE == m_state )
                m_state = VOICEPLAYOUTSTATE_PREBUFFER;
            break;
        }
        case CVoiceJitterBuffer::INSERT_LATE:
        {
            if ( VOICEPLAYOUTSTATE_PLAYING == m_state )
            {
                // an underrun the margin did not cover: concealment stood in for the frame
                const Float64 slack = GetSlack( info.extendedTimestamp, arrivalTimeInUs );
                ++m_stats.lateFrames;
                GrowMargin( -slack, false );
                PushSlack( slack );
            }
            break;
        }
        case CVoiceJitterBuffer::INSERT_RESTARTED:
        {
            // a new stream: its timeline has nothing to do with the old one
            StopPlayout( VOICEPLAYOUTSTATE_PREBUFFER );
            m_decoder->Reset();
            m_slackWindow.Reset();
            m_hasSlack = false;
            m_timelineShift = 0.0;
            break;
        }
        default:
            break;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

Float64
CVoicePlayoutController::UpdateRatio( void )
{
    if ( GUCEF_NULL == m_buffer )
        return 1.0;

    const Float64 dt = m_elapsedInS;

    // the redundancy floor ends a window after the last frame with a redundant block
    m_sinceRedundancyInS += dt;
    if ( m_sinceRedundancyInS > m_settings.windowInS )
        m_redundancyFloor = 0.0;

    // margin decay: one step per interval without a late frame, not below the minimum, the jitter nor the redundancy
    m_sinceLateInS += dt;
    while ( m_sinceLateInS >= m_settings.decayIntervalInS )
    {
        m_sinceLateInS -= m_settings.decayIntervalInS;
        const Float64 jitterInS = m_buffer->GetInterarrivalJitter() / (Float64) m_timestampRate;
        Float64 floor = m_settings.minMarginInMs / 1000.0;
        if ( m_settings.jitterMarginFactor * jitterInS > floor )
            floor = m_settings.jitterMarginFactor * jitterInS;
        if ( m_redundancyFloor > floor )
            floor = m_redundancyFloor;
        if ( m_fecFloor > floor )
            floor = m_fecFloor;
        const Float64 lowered = m_margin - m_settings.decayStepInMs / 1000.0;
        if ( lowered >= floor )
            m_margin = lowered;
        else
        if ( m_margin > floor )
            m_margin = floor;
    }

    Float64 target = 1.0 + Clamp( m_integral, -m_settings.maxRatioDeviation, m_settings.maxRatioDeviation );
    if ( VOICEPLAYOUTSTATE_PLAYING == m_state )
    {
        // the audio there is beyond the current block
        const UInt64 end = m_buffer->GetStoredFrameCount() > 0 ? m_buffer->GetBufferedEndTimestamp() : GetPlayoutTimestamp();
        m_level = ( (Float64) end - GetOutputPositionTimestamp() ) / (Float64) m_timestampRate / m_ratio - m_blockInS;

        Float64 minimum = 0.0;
        if ( GetMinimumLevel( minimum ) )
        {
            Float64 error = minimum - m_margin;
            if ( fabs( error ) <= 1.0 / (Float64) m_outputRate )
                error = 0.0;     // the dead band of one sample: a stream on time is played at exactly ratio 1
            m_integral = Clamp( m_integral + m_ki * error * dt, -m_settings.integralLimit, m_settings.integralLimit );
            target = 1.0 + Clamp( m_kp * error + m_integral, -m_settings.maxRatioDeviation, m_settings.maxRatioDeviation );
        }
    }
    else
    {
        m_level = 0.0;
    }

    const Float64 step = m_settings.maxRatioStepPerRequest;
    if ( target > m_ratio + step )
        m_ratio += step;
    else
    if ( target < m_ratio - step )
        m_ratio -= step;
    else
        m_ratio = target;
    return m_ratio;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoicePlayoutController::WriteSilence( Float32* pcm, UInt32 frames )
{
    memset( pcm, 0, (size_t) frames * sizeof( Float32 ) );
    m_produced += frames;
    return frames;
}

/*-------------------------------------------------------------------------*/

bool
CVoicePlayoutController::Conceal( Float32* pcm )
{
    ++m_stats.framesConcealed;
    if ( (Int32) m_frameSize != m_decoder->Conceal( pcm, m_frameSize ) )
    {
        memset( pcm, 0, (size_t) m_frameSize * sizeof( Float32 ) );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVoicePlayoutController::IsSilent( const Float32* pcm ) const
{
    Float64 energy = 0.0;
    for ( UInt32 n=0; n<m_frameSize; ++n )
        energy += (Float64) pcm[ n ] * (Float64) pcm[ n ];
    return energy < m_silenceThreshold * m_silenceThreshold * (Float64) m_frameSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoicePlayoutController::Produce( Float32* pcm, UInt32 maxFrames, SProduceInfo* info )
{
    SProduceInfo ignored;
    SProduceInfo& produced = GUCEF_NULL != info ? *info : ignored;
    produced = SProduceInfo();
    if ( GUCEF_NULL == m_buffer || GUCEF_NULL == pcm || 0 == maxFrames )
        return 0;
    if ( VOICEPLAYOUTSTATE_PLAYING == m_state )
        return ProducePlaying( pcm, maxFrames, produced );
    return ProduceWhileWaiting( pcm, maxFrames, produced );
}

/*-------------------------------------------------------------------------*/

UInt32
CVoicePlayoutController::ProduceWhileWaiting( Float32* pcm, UInt32 maxFrames, SProduceInfo& produced )
{
    CVoiceJitterBuffer::SFrameInfo oldest;
    if ( !m_buffer->PeekOldest( oldest ) )
    {
        m_state = VOICEPLAYOUTSTATE_IDLE;
        return WriteSilence( pcm, maxFrames );
    }
    m_state = VOICEPLAYOUTSTATE_PREBUFFER;

    if ( m_hasRequest )
    {
        // the first sample of the talkspurt goes out at its arrival plus the margin plus one block
        const Float64 start = (Float64) oldest.arrivalTimeInUs / 1000000.0 + m_margin + m_blockInS;
        const Float64 nextOutput = (Float64) m_nowInUs / 1000000.0 + (Float64) ( m_pending + m_produced ) / (Float64) m_outputRate / m_ratio;
        const Float64 pad = floor( ( start - nextOutput ) * (Float64) m_outputRate + 0.5 );
        if ( pad >= (Float64) maxFrames )
            return WriteSilence( pcm, maxFrames );
        if ( pad >= 1.0 )
            return WriteSilence( pcm, (UInt32) pad );
    }

    if ( maxFrames < m_frameSize )
        return 0;   // the talkspurt starts now: there must be room for a frame
    if ( !StartTalkspurt() )
        return WriteSilence( pcm, maxFrames );
    return ProducePlaying( pcm, maxFrames, produced );
}

/*-------------------------------------------------------------------------*/

bool
CVoicePlayoutController::StartTalkspurt( void )
{
    if ( !m_buffer->StartPlayout() )
        return false;

    m_state = VOICEPLAYOUTSTATE_PLAYING;
    m_concealed = 0;
    m_concealedForEmpty = false;
    ++m_stats.talkspurts;
    m_delaySum = 0.0;
    m_delayCount = 0;

    // a new timeline: the slack of every frame there is measured against it
    m_slackWindow.Reset();
    m_hasSlack = false;
    m_timelineShift = 0.0;
    CVoiceJitterBuffer::SFrameInfo info;
    bool more = m_buffer->PeekOldest( info );
    while ( more )
    {
        PushSlack( GetSlack( info.extendedTimestamp, info.arrivalTimeInUs ) );
        more = m_buffer->PeekNext( info.extendedSequenceNumber, info );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoicePlayoutController::ProducePlaying( Float32* pcm, UInt32 maxFrames, SProduceInfo& produced )
{
    if ( maxFrames < m_frameSize )
        return 0;

    bool dropped = false;
    for ( ;; )
    {
        Float64 minimum = 0.0;
        const bool excess = !dropped && GetMinimumLevel( minimum ) &&
                            minimum > m_margin + m_settings.excessFramesForDrop * m_frameDurationInS;

        // frames that concealment stood in for are still wanted when the buffer was empty then (they are late, the
        // playout stretches for them); when a newer frame was there the concealment replaced them for good
        const UInt64 playoutTimestamp = GetPlayoutTimestamp();
        const UInt64 oldestWanted = m_concealedForEmpty ?
                m_buffer->GetNextTimestamp() + (UInt64) m_emptySince * (UInt64) m_timestampUnitsPerFrame : playoutTimestamp;

        CVoiceJitterBuffer::SPullInfo info;
        const CVoiceJitterBuffer::EPullResult result = m_buffer->Pull( playoutTimestamp, oldestWanted, info );
        if ( info.lateDiscarded > 0 )
        {
            m_stats.lateFrames += info.lateDiscarded;
            GrowMargin( (Float64) info.maxLatenessDiscarded / (Float64) m_timestampRate, false );
        }
        switch ( result )
        {
            case CVoiceJitterBuffer::PULL_FRAME:
            {
                const CVoiceFrame& frame = *info.frame.frame;
                const bool decoded = (Int32) m_frameSize == m_decoder->Decode( frame.GetPayload(), frame.GetPayloadSize(), pcm, m_frameSize );
                if ( decoded )
                    ++m_stats.framesDecoded;
                else
                {
                    ++m_stats.decodeErrors;
                    Conceal( pcm );
                }

                if ( info.latenessInTimestampUnits > 0 )
                {
                    // concealment stood in for this frame: the playout stretched by that much, and the margin grows by it
                    const Float64 lateness = (Float64) info.latenessInTimestampUnits / (Float64) m_timestampRate;
                    ++m_stats.framesStretched;
                    if ( m_concealedForEmpty )
                        ++m_stats.underruns;
                    ShiftTimeline( lateness );
                    GrowMargin( lateness, true );
                }
                m_concealed = 0;
                m_concealedForEmpty = false;

                const Float64 outputTime = (Float64) m_nowInUs / 1000000.0 + (Float64) ( m_pending + m_produced ) / (Float64) m_outputRate / m_ratio;
                const Float64 delay = outputTime - (Float64) info.frame.arrivalTimeInUs / 1000000.0;
                m_stats.lastPlayoutDelayInS = delay;
                m_delaySum += delay;
                ++m_delayCount;
                if ( delay > m_stats.maxPlayoutDelayInS )
                    m_stats.maxPlayoutDelayInS = delay;

                if ( excess && decoded && IsSilent( pcm ) )
                {
                    ++m_stats.framesAccelerated;
                    ShiftTimeline( -m_frameDurationInS );
                    dropped = true;
                    continue;
                }
                produced.kind = decoded ? PRODUCED_DECODED : PRODUCED_CONCEALED;
                produced.extendedTimestamp = info.frame.extendedTimestamp;
                produced.extendedSequenceNumber = info.frame.extendedSequenceNumber;
                produced.arrivalTimeInUs = info.frame.arrivalTimeInUs;
                produced.captureTickCount = frame.GetCaptureTickCount();
                m_produced += m_frameSize;
                return m_frameSize;
            }
            case CVoiceJitterBuffer::PULL_MISSING:
            {
                bool recovered = false;
                if ( GUCEF_NULL != info.frame.frame && m_decoder->SupportsFec() )
                {
                    const CVoiceFrame& next = *info.frame.frame;
                    recovered = (Int32) m_frameSize == m_decoder->DecodeFec( next.GetPayload(), next.GetPayloadSize(), pcm, m_frameSize );
                    if ( recovered )
                        ++m_stats.framesRecovered;
                }
                if ( !recovered )
                    Conceal( pcm );
                ++m_concealed;
                produced.kind = recovered ? PRODUCED_RECOVERED : PRODUCED_CONCEALED;
                produced.extendedTimestamp = playoutTimestamp;

                if ( excess && !recovered )
                {
                    ++m_stats.framesAccelerated;
                    ShiftTimeline( -m_frameDurationInS );
                    dropped = true;
                    continue;
                }
                m_produced += m_frameSize;
                return m_frameSize;
            }
            case CVoiceJitterBuffer::PULL_EMPTY:
            {
                if ( m_concealed >= m_settings.concealLimit )
                {
                    // the concealment has faded out and nothing came: the talkspurt is over
                    StopPlayout( VOICEPLAYOUTSTATE_IDLE );
                    return ProduceWhileWaiting( pcm, maxFrames, produced );
                }
                if ( !m_concealedForEmpty )
                {
                    m_concealedForEmpty = true;
                    m_emptySince = m_concealed;
                }
                Conceal( pcm );
                ++m_concealed;
                produced.kind = PRODUCED_CONCEALED;
                produced.extendedTimestamp = playoutTimestamp;
                m_produced += m_frameSize;
                return m_frameSize;
            }
            case CVoiceJitterBuffer::PULL_TALKSPURT_END:
            case CVoiceJitterBuffer::PULL_NOT_PLAYING:
            default:
            {
                StopPlayout( VOICEPLAYOUTSTATE_PREBUFFER );
                return ProduceWhileWaiting( pcm, maxFrames, produced );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

TVoicePlayoutState
CVoicePlayoutController::GetState( void ) const
{
    return m_state;
}

/*-------------------------------------------------------------------------*/

Float64
CVoicePlayoutController::GetRatio( void ) const
{
    return m_ratio;
}

/*-------------------------------------------------------------------------*/

Float64
CVoicePlayoutController::GetMarginInS( void ) const
{
    return m_margin;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoicePlayoutController::GetFrameSizeInFrames( void ) const
{
    return m_frameSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoicePlayoutController::GetOutputRate( void ) const
{
    return m_outputRate;
}

/*-------------------------------------------------------------------------*/

void
CVoicePlayoutController::GetStatistics( SStatistics& stats ) const
{
    stats = m_stats;
    stats.state = m_state;
    stats.ratio = m_ratio;
    stats.integral = m_integral;
    stats.marginInS = m_margin;
    stats.hasMinimumLevel = GetMinimumLevel( stats.minimumLevelInS );
    stats.levelInS = m_level;
    stats.meanPlayoutDelayInS = m_delayCount > 0 ? m_delaySum / (Float64) m_delayCount : 0.0;
    stats.jitterInS = GUCEF_NULL != m_buffer && m_timestampRate > 0 ? m_buffer->GetInterarrivalJitter() / (Float64) m_timestampRate : 0.0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
