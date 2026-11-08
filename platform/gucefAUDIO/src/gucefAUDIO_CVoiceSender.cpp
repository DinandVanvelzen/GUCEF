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

#include <string.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUDIO_CRTPHEADER_H
#include "gucefAUDIO_CRtpHeader.h"
#define GUCEF_AUDIO_CRTPHEADER_H
#endif /* GUCEF_AUDIO_CRTPHEADER_H ? */

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#include "gucefAUDIO_CAudioGlobal.h"
#define GUCEF_AUDIO_CAUDIOGLOBAL_H
#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */

#ifndef GUCEF_AUDIO_CVOICECODECREGISTRY_H
#include "gucefAUDIO_CVoiceCodecRegistry.h"
#define GUCEF_AUDIO_CVOICECODECREGISTRY_H
#endif /* GUCEF_AUDIO_CVOICECODECREGISTRY_H ? */

#ifndef GUCEF_AUDIO_CVOICECLOCK_H
#include "gucefAUDIO_CVoiceClock.h"
#define GUCEF_AUDIO_CVOICECLOCK_H
#endif /* GUCEF_AUDIO_CVOICECLOCK_H ? */

#include "gucefAUDIO_CVoiceSender.h"

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

/** RFC 2198: 4 bytes of header for the redundant block and 1 for the primary block */
const UInt32 RedHeaderBytes = 5;

const Int32 NoPendingSlot = -1;

/** A random value for the RTP identity of a stream (RFC 3550 asks for random SSRC, sequence and timestamp starts) */
UInt64
RandomIdentityValue( const void* object )
{
    static MT::CTAtomic< UInt64 > counter( 0 );
    UInt64 z = MT::PrecisionTickCount() ^ ( (UInt64) (size_t) object << 16 ) ^ ( counter.FetchAdd( 1 ) * 0x9E3779B97F4A7C15ULL );
    // splitmix64 finaliser
    z += 0x9E3779B97F4A7C15ULL;
    z = ( z ^ ( z >> 30 ) ) * 0xBF58476D1CE4E5B9ULL;
    z = ( z ^ ( z >> 27 ) ) * 0x94D049BB133111EBULL;
    return z ^ ( z >> 31 );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVoiceSender::SCodecSlot::SCodecSlot( void )
    : encoderRef()
    , encoder( GUCEF_NULL )
    , config()
    , sampleRate( 0 )
    , frameSize( 0 )
    , timestampRate( 0 )
    , timestampUnitsPerFrame( 0 )
    , hangoverFrames( 0 )
    , payloadType( 0 )
    , accumulator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVoiceSender::CVoiceSender( const CVoiceSenderConfig& config )
    : IAudioCaptureConsumer()
    , m_config( config )
    , m_streamId( 0 )
    , m_firstSequenceNumber( 0 )
    , m_firstTimestamp( 0 )
    , m_queueLimit( config.GetOutboundQueueFrames() )
    , m_preRollCapacity( config.GetPreRollFrames() )
    , m_redundancy( config.GetRedundancy() )
    , m_prepared( false )
    , m_ownerSlot( 0 )
    , m_processingRate( 0 )
    , m_ready( false )
    , m_pendingSlot( NoPendingSlot )
    , m_transmitMode( (UInt32) config.GetTransmitMode() )
    , m_talk( false )
    , m_releaseLatch( false )
    , m_muted( false )
    , m_transmitting( false )
    , m_requestedBitrate( config.GetCodecConfig().GetBitrate() )
    , m_bitrateGeneration( 0 )
    , m_requestedLoss( config.GetCodecConfig().GetExpectedLossPercentage() )
    , m_lossGeneration( 0 )
    , m_activeSlot( 0 )
    , m_appliedBitrateGeneration( 0 )
    , m_appliedLossGeneration( 0 )
    , m_fill( 0 )
    , m_hasPosition( false )
    , m_nextPosition( 0 )
    , m_frameStartPosition( 0 )
    , m_frameTickCount( 0 )
    , m_hasTimeline( false )
    , m_positionBase( 0 )
    , m_timestampBase( 0 )
    , m_lastFrameTimestamp( 0 )
    , m_nextSequenceNumber( 0 )
    , m_previousFrameSent( false )
    , m_hangoverLeft( 0 )
    , m_work()
    , m_previousPayloadSize( 0 )
    , m_previousPayloadType( 0 )
    , m_preRoll()
    , m_preRollHead( 0 )
    , m_preRollCount( 0 )
    , m_encodeTimeSum( 0.0 )
    , m_stats()
    , m_queue( config.GetOutboundQueueFrames() )
    , m_statsExchange()
    , m_statsReadLock()
    , m_statsRead()
{GUCEF_TRACE;

    // the identity of the stream: random where the configuration leaves it open (RFC 3550 section 5.1)
    m_streamId = config.GetStreamId();
    while ( 0 == m_streamId )
        m_streamId = (UInt32) RandomIdentityValue( this );
    m_firstSequenceNumber = config.HasFirstSequenceNumber() ? config.GetFirstSequenceNumber() : (UInt16) RandomIdentityValue( this );
    m_firstTimestamp = config.HasFirstTimestamp() ? config.GetFirstTimestamp() : (UInt32) RandomIdentityValue( this );
    m_nextSequenceNumber = m_firstSequenceNumber;

    m_preRoll.assign( m_preRollCapacity > 0 ? m_preRollCapacity : 1, CVoiceFrame() );
    memset( m_previousPayload, 0, sizeof( m_previousPayload ) );
    m_stats.streamId = m_streamId;
    m_statsRead.streamId = m_streamId;
}

/*-------------------------------------------------------------------------*/

CVoiceSender::~CVoiceSender()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CVoiceSenderConfig&
CVoiceSender::GetConfig( void ) const
{GUCEF_TRACE;

    return m_config;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceSender::PrepareSlot( SCodecSlot& slot, const CVoiceCodecConfig& codecConfig )
{GUCEF_TRACE;

    TVoiceEncoderPtr encoder;
    if ( !CAudioGlobal::Instance()->GetVoiceCodecRegistry().CreateEncoder( codecConfig, encoder ) || encoder.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VoiceSender: No encoder for codec \"" << codecConfig.GetCodec() << "\" at " <<
                         codecConfig.GetSampleRate() << " Hz with frames of " << codecConfig.GetFrameDurationInUs() << " us" );
        return false;
    }

    const UInt32 frameSize = encoder->GetFrameSizeInFrames();
    const UInt32 timestampRate = encoder->GetTimestampRate();
    const UInt64 durationInUs = codecConfig.GetFrameDurationInUs();
    if ( 0 == frameSize || 0 == timestampRate || 0 == durationInUs || 0 != ( (UInt64) timestampRate * durationInUs ) % 1000000ULL )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VoiceSender: The encoder of codec \"" << codecConfig.GetCodec() << "\" has frames that are no whole number of timestamp units" );
        return false;
    }

    slot.encoderRef = encoder;
    slot.encoder = encoder.GetPointerAlways();
    slot.config = codecConfig;
    slot.sampleRate = codecConfig.GetSampleRate();
    slot.frameSize = frameSize;
    slot.timestampRate = timestampRate;
    slot.timestampUnitsPerFrame = (UInt32) ( (UInt64) timestampRate * durationInUs / 1000000ULL );
    slot.hangoverFrames = (UInt32) ( ( (UInt64) m_config.GetVadHangoverInMs() * 1000ULL + durationInUs - 1 ) / durationInUs );
    slot.payloadType = codecConfig.GetPayloadType();
    slot.accumulator.assign( frameSize, 0.0f );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceSender::Prepare( void )
{GUCEF_TRACE;

    SCodecSlot& slot = m_slots[ 0 ];
    if ( !PrepareSlot( slot, m_config.GetCodecConfig() ) )
        return false;

    // nothing uses the sender now (the owner's contract): start from slot 0, forget a change that was waiting
    m_slots[ 1 ] = SCodecSlot();
    m_pendingSlot.Store( NoPendingSlot, MT::MEMORDER_RELEASE );
    m_ownerSlot = 0;
    m_activeSlot = 0;
    m_appliedBitrateGeneration = m_bitrateGeneration.Load( MT::MEMORDER_ACQUIRE );
    m_appliedLossGeneration = m_lossGeneration.Load( MT::MEMORDER_ACQUIRE );
    ResetCaptureState();
    m_prepared = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceSender::IsPrepared( void ) const
{GUCEF_TRACE;

    return m_prepared;
}

/*-------------------------------------------------------------------------*/

void
CVoiceSender::ResetCaptureState( void )
{GUCEF_TRACE;

    m_fill = 0;
    m_hasPosition = false;
    m_nextPosition = 0;
    m_previousFrameSent = false;
    m_hangoverLeft = 0;
    m_previousPayloadSize = 0;
    m_preRollHead = 0;
    m_preRollCount = 0;
}

/*-------------------------------------------------------------------------*/

void
CVoiceSender::SetTransmitMode( TVoiceTransmitMode mode )
{GUCEF_TRACE;

    if ( VOICETRANSMITMODE_CONTINUOUS == mode || VOICETRANSMITMODE_VOICE_ACTIVATED == mode || VOICETRANSMITMODE_PUSH_TO_TALK == mode )
        m_transmitMode.Store( (UInt32) mode, MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

TVoiceTransmitMode
CVoiceSender::GetTransmitMode( void ) const
{GUCEF_TRACE;

    return (TVoiceTransmitMode) m_transmitMode.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CVoiceSender::SetTransmitting( bool transmitting )
{GUCEF_TRACE;

    if ( transmitting )
        m_talk.Store( true );
    else
    if ( m_talk.Exchange( false ) )
        m_releaseLatch.Store( true );
}

/*-------------------------------------------------------------------------*/

bool
CVoiceSender::IsTalkKeyDown( void ) const
{GUCEF_TRACE;

    return m_talk.Load();
}

/*-------------------------------------------------------------------------*/

void
CVoiceSender::SetMuted( bool muted )
{GUCEF_TRACE;

    m_muted.Store( muted );
}

/*-------------------------------------------------------------------------*/

bool
CVoiceSender::IsMuted( void ) const
{GUCEF_TRACE;

    return m_muted.Load();
}

/*-------------------------------------------------------------------------*/

bool
CVoiceSender::IsTransmitting( void ) const
{GUCEF_TRACE;

    return m_transmitting.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

void
CVoiceSender::SetBitrate( UInt32 bitsPerSecond )
{GUCEF_TRACE;

    m_requestedBitrate.Store( bitsPerSecond, MT::MEMORDER_RELEASE );
    m_bitrateGeneration.FetchAdd( 1 );
}

/*-------------------------------------------------------------------------*/

void
CVoiceSender::SetExpectedLossPercentage( UInt32 percentage )
{GUCEF_TRACE;

    m_requestedLoss.Store( percentage, MT::MEMORDER_RELEASE );
    m_lossGeneration.FetchAdd( 1 );
}

/*-------------------------------------------------------------------------*/

bool
CVoiceSender::SetCodecConfig( const CVoiceCodecConfig& codecConfig )
{GUCEF_TRACE;

    if ( !m_prepared )
    {
        // not running: the new configuration is simply the one to prepare
        m_config.SetCodecConfig( codecConfig );
        return true;
    }
    if ( NoPendingSlot != m_pendingSlot.Load( MT::MEMORDER_ACQUIRE ) )
        return false;
    if ( 0 != m_processingRate && codecConfig.GetSampleRate() != m_processingRate )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VoiceSender: A codec at " << codecConfig.GetSampleRate() << " Hz does not fit the processing rate of " << m_processingRate << " Hz" );
        return false;
    }

    // the other slot is not used by the capture thread: the last change was taken (no pending slot)
    const UInt32 slotIndex = 1 - m_ownerSlot;
    SCodecSlot& slot = m_slots[ slotIndex ];
    if ( !PrepareSlot( slot, codecConfig ) )
        return false;
    m_ownerSlot = slotIndex;
    m_pendingSlot.Store( (Int32) slotIndex, MT::MEMORDER_RELEASE );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceSender::IsCodecChangePending( void ) const
{GUCEF_TRACE;

    return NoPendingSlot != m_pendingSlot.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

const CVoiceCodecConfig&
CVoiceSender::GetCodecConfig( void ) const
{GUCEF_TRACE;

    if ( !m_prepared )
        return m_config.GetCodecConfig();
    return m_slots[ m_ownerSlot ].config;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceSender::GetStreamId( void ) const
{GUCEF_TRACE;

    return m_streamId;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceSender::PopFrame( CVoiceFrame& frame )
{GUCEF_TRACE;

    return m_queue.Read( frame );
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceSender::GetPendingFrameCount( void ) const
{GUCEF_TRACE;

    return m_queue.GetSize();
}

/*-------------------------------------------------------------------------*/

void
CVoiceSender::GetStats( CVoiceSenderStats& stats ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_statsReadLock );
    m_statsExchange.Read( m_statsRead );
    stats = m_statsRead;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceSender::PrepareForCapture( const CAudioFormat& processingFormat, UInt32 maxBlockFrames )
{GUCEF_TRACE;

    if ( !m_prepared && !Prepare() )
        return false;

    // nothing captures into the sender now: a change that waits is taken here
    const Int32 pending = m_pendingSlot.Load( MT::MEMORDER_ACQUIRE );
    if ( NoPendingSlot != pending )
    {
        m_activeSlot = (UInt32) pending;
        m_pendingSlot.Store( NoPendingSlot, MT::MEMORDER_RELEASE );
        ++m_stats.codecSwitches;
    }

    const SCodecSlot& slot = m_slots[ m_activeSlot ];
    if ( 1 != processingFormat.GetChannelCount() || SAMPLETYPE_FLOAT32 != processingFormat.GetSampleType() ||
         processingFormat.GetSampleRate() != slot.sampleRate )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VoiceSender: The capture processing format (" << processingFormat.GetSampleRate() << " Hz, " <<
                         processingFormat.GetChannelCount() << " channels) does not fit the codec at " << slot.sampleRate << " Hz mono" );
        return false;
    }

    m_processingRate = processingFormat.GetSampleRate();
    ResetCaptureState();
    m_ready.Store( true, MT::MEMORDER_RELEASE );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceSender::TimestampAt( UInt64 position ) const
{
    // no trace macro: capture thread
    const SCodecSlot& slot = m_slots[ m_activeSlot ];
    const UInt64 units = ( ( position - m_positionBase ) * (UInt64) slot.timestampRate ) / (UInt64) slot.sampleRate;
    return m_timestampBase + (UInt32) units;
}

/*-------------------------------------------------------------------------*/

void
CVoiceSender::ApplyPendingChanges( void )
{
    // no trace macro: capture thread

    const Int32 pending = m_pendingSlot.Load( MT::MEMORDER_ACQUIRE );
    if ( NoPendingSlot != pending )
    {
        // the timestamps go on at the position of the next frame, from there in the rate of the new codec
        if ( m_hasPosition )
        {
            m_timestampBase = TimestampAt( m_frameStartPosition );
            m_positionBase = m_frameStartPosition;
        }
        m_activeSlot = (UInt32) pending;
        m_previousFrameSent = false;            // a new talkspurt: marker, no redundancy across codecs
        m_previousPayloadSize = 0;
        ClearPreRoll();
        m_appliedBitrateGeneration = m_bitrateGeneration.Load( MT::MEMORDER_ACQUIRE );
        m_appliedLossGeneration = m_lossGeneration.Load( MT::MEMORDER_ACQUIRE );
        ++m_stats.codecSwitches;
        m_pendingSlot.Store( NoPendingSlot, MT::MEMORDER_RELEASE );
    }

    CVoiceEncoder* encoder = m_slots[ m_activeSlot ].encoder;
    const UInt32 bitrateGeneration = m_bitrateGeneration.Load( MT::MEMORDER_ACQUIRE );
    if ( bitrateGeneration != m_appliedBitrateGeneration )
    {
        m_appliedBitrateGeneration = bitrateGeneration;
        encoder->SetBitrate( m_requestedBitrate.Load( MT::MEMORDER_ACQUIRE ) );
    }
    const UInt32 lossGeneration = m_lossGeneration.Load( MT::MEMORDER_ACQUIRE );
    if ( lossGeneration != m_appliedLossGeneration )
    {
        m_appliedLossGeneration = lossGeneration;
        encoder->SetExpectedLossPercentage( m_requestedLoss.Load( MT::MEMORDER_ACQUIRE ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CVoiceSender::OnCapturedAudio( const Float32* mono, UInt32 frameCount, const SCaptureBlockInfo& info )
{
    // no trace macro: capture thread

    if ( GUCEF_NULL == mono || 0 == frameCount || !m_ready.Load( MT::MEMORDER_ACQUIRE ) )
        return;

    if ( !m_hasPosition )
    {
        // a new stream section: the timestamps go on where they were, or start at the first timestamp
        ApplyPendingChanges();
        m_positionBase = info.firstFrame;
        m_timestampBase = m_hasTimeline ? m_lastFrameTimestamp + m_slots[ m_activeSlot ].timestampUnitsPerFrame : m_firstTimestamp;
        m_hasTimeline = true;
        m_hasPosition = true;
    }
    else
    if ( info.discontinuity || info.firstFrame != m_nextPosition )
    {
        // audio was lost: so is the frame in progress. The next frame starts where the audio resumes, its timestamp
        // shows the gap (a position that went back, which no capture path makes, starts a new time line)
        m_fill = 0;
        FrameNotSent();
        ++m_stats.discontinuities;
        if ( info.firstFrame < m_nextPosition )
        {
            m_timestampBase = m_lastFrameTimestamp + m_slots[ m_activeSlot ].timestampUnitsPerFrame;
            m_positionBase = info.firstFrame;
        }
    }
    m_nextPosition = info.firstFrame + frameCount;

    UInt32 offset = 0;
    while ( offset < frameCount )
    {
        if ( 0 == m_fill )
        {
            m_frameStartPosition = info.firstFrame + offset;
            m_frameTickCount = info.tickCount;
            ApplyPendingChanges();
        }

        SCodecSlot& slot = m_slots[ m_activeSlot ];
        const UInt32 room = slot.frameSize - m_fill;
        const UInt32 take = ( frameCount - offset < room ) ? frameCount - offset : room;
        memcpy( &slot.accumulator[ m_fill ], mono + offset, (size_t) take * sizeof( Float32 ) );
        m_fill += take;
        offset += take;
        if ( m_fill == slot.frameSize )
        {
            m_fill = 0;
            CompleteFrame( info.voiceActive );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CVoiceSender::ClearPreRoll( void )
{
    // no trace macro: capture thread
    m_preRollHead = 0;
    m_preRollCount = 0;
}

/*-------------------------------------------------------------------------*/

void
CVoiceSender::FrameNotSent( void )
{
    // no trace macro: capture thread
    m_previousFrameSent = false;
    ClearPreRoll();
}

/*-------------------------------------------------------------------------*/

void
CVoiceSender::CompleteFrame( bool voiceActive )
{
    // no trace macro: capture thread

    SCodecSlot& slot = m_slots[ m_activeSlot ];
    ++m_stats.framesCaptured;
    const UInt32 timestamp = TimestampAt( m_frameStartPosition );
    m_lastFrameTimestamp = timestamp;

    // the gate: does this frame go out
    const bool released = m_releaseLatch.Exchange( false );
    const bool muted = m_muted.Load( MT::MEMORDER_RELAXED );
    const TVoiceTransmitMode mode = (TVoiceTransmitMode) m_transmitMode.Load( MT::MEMORDER_RELAXED );
    bool transmit = false;
    switch ( mode )
    {
        case VOICETRANSMITMODE_VOICE_ACTIVATED:
        {
            if ( voiceActive )
            {
                m_hangoverLeft = slot.hangoverFrames;
                transmit = true;
            }
            else
            if ( m_hangoverLeft > 0 )
            {
                --m_hangoverLeft;
                transmit = true;
            }
            break;
        }
        case VOICETRANSMITMODE_PUSH_TO_TALK:
        {
            transmit = m_talk.Load() || released;
            break;
        }
        case VOICETRANSMITMODE_CONTINUOUS:
        default:
        {
            transmit = true;
            break;
        }
    }
    if ( muted )
        transmit = false;
    if ( VOICETRANSMITMODE_VOICE_ACTIVATED != mode )
        m_hangoverLeft = 0;

    // voice activated mode encodes every frame, so the encoder state runs on and the pre-roll holds real frames
    const bool encode = transmit || ( !muted && VOICETRANSMITMODE_VOICE_ACTIVATED == mode );
    m_transmitting.Store( transmit, MT::MEMORDER_RELEASE );
    if ( !encode )
    {
        FrameNotSent();
        m_stats.transmitting = false;
        m_statsExchange.Write( m_stats );
        return;
    }

    const UInt64 before = MT::PrecisionTickCount();
    const Int32 bytes = slot.encoder->Encode( &slot.accumulator[ 0 ], m_work.GetPayloadBuffer(), CVoiceFrame::MaxPayloadSize );
    const UInt64 after = MT::PrecisionTickCount();
    const Float64 encodeTimeInUs = (Float64) CVoiceClock::TicksToMicroseconds( after >= before ? after - before : 0 );
    m_encodeTimeSum += encodeTimeInUs;
    if ( encodeTimeInUs > m_stats.maxEncodeTimeInUs )
        m_stats.maxEncodeTimeInUs = encodeTimeInUs;

    if ( bytes <= 0 || (UInt32) bytes > CVoiceFrame::MaxPayloadSize )
    {
        // nothing to send: a gap in the timestamps, the next frame sent starts a talkspurt
        if ( bytes < 0 || (UInt32) bytes > CVoiceFrame::MaxPayloadSize )
            ++m_stats.encodeErrors;
        else
            ++m_stats.framesEmpty;
        FrameNotSent();
        m_transmitting.Store( false, MT::MEMORDER_RELEASE );
        m_stats.transmitting = false;
        m_statsExchange.Write( m_stats );
        return;
    }

    ++m_stats.framesEncoded;
    m_stats.meanEncodeTimeInUs = m_encodeTimeSum / (Float64) m_stats.framesEncoded;
    m_work.SetPayloadSize( (UInt32) bytes );
    m_work.SetStreamId( m_streamId );
    m_work.SetTimestamp( timestamp );
    m_work.SetPayloadType( slot.payloadType );
    m_work.SetCaptureTickCount( m_frameTickCount );
    m_work.SetMarker( false );
    m_work.ClearRedundantBlock();

    if ( transmit )
    {
        // at the start of a talkspurt the pre-roll goes first: the frames right before this one
        if ( !m_previousFrameSent )
        {
            const UInt32 capacity = (UInt32) m_preRoll.size();
            for ( UInt32 i=0; i<m_preRollCount; ++i )
                SendFrame( m_preRoll[ ( m_preRollHead + i ) % capacity ], true );
        }
        ClearPreRoll();
        SendFrame( m_work, false );
    }
    else
    {
        // held for an onset; the oldest goes when the ring is full
        const UInt32 capacity = (UInt32) m_preRoll.size();
        if ( 0 == m_preRollCapacity )
        {
            // no pre-roll: the frame was only encoded to keep the encoder's state running
        }
        else
        if ( m_preRollCount < m_preRollCapacity )
        {
            m_preRoll[ ( m_preRollHead + m_preRollCount ) % capacity ] = m_work;
            ++m_preRollCount;
        }
        else
        {
            m_preRoll[ m_preRollHead ] = m_work;
            m_preRollHead = ( m_preRollHead + 1 ) % capacity;
        }
        m_previousFrameSent = false;
    }

    m_stats.transmitting = transmit;
    m_statsExchange.Write( m_stats );
}

/*-------------------------------------------------------------------------*/

void
CVoiceSender::SendFrame( CVoiceFrame& frame, bool preRoll )
{
    // no trace macro: capture thread

    const SCodecSlot& slot = m_slots[ m_activeSlot ];
    const bool marker = !m_previousFrameSent;
    frame.SetSequenceNumber( m_nextSequenceNumber++ );
    frame.SetMarker( marker );
    frame.ClearRedundantBlock();

    // the previous frame along as RFC 2198 block, when it was sent right before this one and the packet stays small enough
    UInt32 octets = frame.GetPayloadSize();
    if ( m_redundancy > 0 && !marker && m_previousPayloadSize > 0 )
    {
        const UInt32 packetSize = CRtpHeader::FixedHeaderSize + RedHeaderBytes + m_previousPayloadSize + frame.GetPayloadSize();
        if ( packetSize <= MaxPacketSize &&
             frame.SetRedundantBlock( (UInt16) slot.timestampUnitsPerFrame, m_previousPayloadType, m_previousPayload, m_previousPayloadSize ) )
        {
            ++m_stats.redundantBlocksSent;
            octets += RedHeaderBytes + m_previousPayloadSize;
        }
        else
        {
            ++m_stats.redundancyOmitted;
        }
    }

    // what the next frame may carry
    memcpy( m_previousPayload, frame.GetPayload(), frame.GetPayloadSize() );
    m_previousPayloadSize = frame.GetPayloadSize();
    m_previousPayloadType = frame.GetPayloadType();
    m_previousFrameSent = true;

    if ( marker )
        ++m_stats.talkspurts;
    if ( preRoll )
        ++m_stats.preRollFramesSent;

    // the queue rounds its size up to a power of two: the configured size is the limit. The size only shrinks while
    // this thread looks at it, so the check never lets more in than the limit
    if ( m_queue.GetSize() >= m_queueLimit || !m_queue.Write( frame ) )
    {
        ++m_stats.framesDropped;
    }
    else
    {
        ++m_stats.framesSent;
        m_stats.octetsSent += octets;
    }
    m_stats.hasLastFrame = true;
    m_stats.lastTimestamp = frame.GetTimestamp();
    m_stats.lastCaptureTickCount = frame.GetCaptureTickCount();
    m_stats.lastSequenceNumber = frame.GetSequenceNumber();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
