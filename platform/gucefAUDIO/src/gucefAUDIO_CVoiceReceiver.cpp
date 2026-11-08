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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUDIO_CAUDIOENGINE_H
#include "gucefAUDIO_CAudioEngine.h"
#define GUCEF_AUDIO_CAUDIOENGINE_H
#endif /* GUCEF_AUDIO_CAUDIOENGINE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#include "gucefAUDIO_CAudioGlobal.h"
#define GUCEF_AUDIO_CAUDIOGLOBAL_H
#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */

#ifndef GUCEF_AUDIO_CVOICECODECREGISTRY_H
#include "gucefAUDIO_CVoiceCodecRegistry.h"
#define GUCEF_AUDIO_CVOICECODECREGISTRY_H
#endif /* GUCEF_AUDIO_CVOICECODECREGISTRY_H ? */

#include "gucefAUDIO_CVoiceReceiver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVoiceReceiver::CVoiceReceiver( CAudioEngine& engine, const CVoiceReceiverConfig& config )
    : m_engine( &engine )
    , m_config( config )
    , m_clock( &CVoiceClock::GetSystemClock() )
    , m_playout( GUCEF_NULL )
    , m_source()
    , m_decoder()
    , m_prepared( false )
    , m_lock()
    , m_voice()
    , m_volume( 1.0f )
    , m_ready( false )
    , m_acceptedStreamId( config.GetStreamId() )
    , m_seenResetGeneration( 0 )
    , m_resetGeneration( 0 )
    , m_currentStreamId( config.GetStreamId() )
    , m_framesPushed( 0 )
    , m_wrongStream( 0 )
    , m_wrongPayloadType( 0 )
    , m_statsLock()
    , m_priorExpected( 0 )
    , m_priorReceived( 0 )
{GUCEF_TRACE;

    m_playout = GUCEF_NEW CVoicePlayoutSource();
    m_source = TAudioSourcePtr( m_playout );
}

/*-------------------------------------------------------------------------*/

CVoiceReceiver::~CVoiceReceiver()
{GUCEF_TRACE;

    TAudioVoicePtr voice;
    {
        MT::CScopeMutex lock( m_lock );
        voice = m_voice;
        m_voice.Unlink();
    }
    if ( !voice.IsNULL() && AUDIOVOICESTATE_ENDED != voice->GetState() )
        m_engine->Stop( voice );
}

/*-------------------------------------------------------------------------*/

const CVoiceReceiverConfig&
CVoiceReceiver::GetConfig( void ) const
{GUCEF_TRACE;

    return m_config;
}

/*-------------------------------------------------------------------------*/

void
CVoiceReceiver::SetClock( const CVoiceClock& clock )
{GUCEF_TRACE;

    m_clock = &clock;
}

/*-------------------------------------------------------------------------*/

const CVoiceClock&
CVoiceReceiver::GetClock( void ) const
{GUCEF_TRACE;

    return *m_clock;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceReceiver::Prepare( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_voice.IsNULL() && AUDIOVOICESTATE_ENDED != m_voice->GetState() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VoiceReceiver: Prepare() while the receiver plays" );
            return false;
        }
    }

    m_ready.Store( false, MT::MEMORDER_RELEASE );
    m_prepared = false;

    TVoiceDecoderPtr decoder;
    const CVoiceCodecConfig& codecConfig = m_config.GetCodecConfig();
    if ( !CAudioGlobal::Instance()->GetVoiceCodecRegistry().CreateDecoder( codecConfig, decoder ) || decoder.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VoiceReceiver: No decoder for codec \"" << codecConfig.GetCodec() << "\" at " <<
                         codecConfig.GetSampleRate() << " Hz with frames of " << codecConfig.GetFrameDurationInUs() << " us" );
        return false;
    }
    if ( !m_playout->Prepare( m_config, decoder, *m_clock ) )
        return false;

    m_decoder = decoder;
    m_acceptedStreamId = m_config.GetStreamId();
    m_seenResetGeneration = m_resetGeneration.Load( MT::MEMORDER_ACQUIRE );
    m_currentStreamId.Store( m_acceptedStreamId, MT::MEMORDER_RELAXED );
    {
        MT::CScopeMutex lock( m_statsLock );
        m_priorExpected = 0;
        m_priorReceived = 0;
    }
    m_prepared = true;
    m_ready.Store( true, MT::MEMORDER_RELEASE );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceReceiver::IsPrepared( void ) const
{GUCEF_TRACE;

    return m_prepared;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVoiceReceiver::Start( CAudioBus* bus )
{GUCEF_TRACE;

    if ( !m_prepared )
        return CORE::CFutureResult( CORE::TASKSTATUS_SETUP_FAILED );

    MT::CScopeMutex lock( m_lock );
    if ( !m_voice.IsNULL() && AUDIOVOICESTATE_ENDED != m_voice->GetState() )
        return CORE::CFutureResult( CORE::TASKSTATUS_SETUP_FAILED );

    // a voice reads ahead when it has to resample the source, which costs latency: the codec rate should be the mix rate
    const CAudioFormat mixFormat = m_engine->GetMixFormat();
    if ( mixFormat.IsValid() && mixFormat.GetSampleRate() != m_playout->GetFormat().GetSampleRate() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VoiceReceiver: The decoder rate of " << m_playout->GetFormat().GetSampleRate() << " Hz differs from the mix rate of " <<
                           mixFormat.GetSampleRate() << " Hz: the voice resamples and reads ahead, which adds latency" );
    }

    m_playout->RequestReset();
    m_voice = m_engine->CreateVoice( m_source, bus );
    if ( m_voice.IsNULL() )
        return CORE::CFutureResult( CORE::TASKSTATUS_SETUP_FAILED );
    m_voice->SetVolume( m_volume );
    return m_engine->Start( m_voice );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVoiceReceiver::Stop( void )
{GUCEF_TRACE;

    TAudioVoicePtr voice;
    {
        MT::CScopeMutex lock( m_lock );
        voice = m_voice;
    }
    if ( voice.IsNULL() )
    {
        CORE::CFutureResultSource done;
        done.Complete();
        return done.GetFuture();
    }
    return m_engine->Stop( voice );
}

/*-------------------------------------------------------------------------*/

TAudioVoicePtr
CVoiceReceiver::GetVoice( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_voice;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceReceiver::PushFrame( const CVoiceFrame& frame, UInt64 arrivalTimeInMicroseconds )
{GUCEF_TRACE;

    if ( !m_ready.Load( MT::MEMORDER_ACQUIRE ) )
        return false;
    m_framesPushed.FetchAdd( 1, MT::MEMORDER_RELAXED );

    // a reset forgets the stream that was accepted
    const UInt32 generation = m_resetGeneration.Load( MT::MEMORDER_ACQUIRE );
    if ( generation != m_seenResetGeneration )
    {
        m_seenResetGeneration = generation;
        m_acceptedStreamId = m_config.GetStreamId();
        m_currentStreamId.Store( m_acceptedStreamId, MT::MEMORDER_RELAXED );
    }

    if ( frame.GetPayloadType() != m_config.GetCodecConfig().GetPayloadType() )
    {
        m_wrongPayloadType.FetchAdd( 1, MT::MEMORDER_RELAXED );
        return false;
    }
    if ( 0 == m_acceptedStreamId )
    {
        m_acceptedStreamId = frame.GetStreamId();
        m_currentStreamId.Store( m_acceptedStreamId, MT::MEMORDER_RELAXED );
    }
    else
    if ( frame.GetStreamId() != m_acceptedStreamId )
    {
        m_wrongStream.FetchAdd( 1, MT::MEMORDER_RELAXED );
        return false;
    }
    return m_playout->PushFrame( frame, arrivalTimeInMicroseconds );
}

/*-------------------------------------------------------------------------*/

void
CVoiceReceiver::Reset( void )
{GUCEF_TRACE;

    m_resetGeneration.FetchAdd( 1 );
    m_playout->RequestReset();
}

/*-------------------------------------------------------------------------*/

void
CVoiceReceiver::SetVolume( Float32 volume )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_volume = volume;
    if ( !m_voice.IsNULL() )
        m_voice->SetVolume( volume );
}

/*-------------------------------------------------------------------------*/

Float32
CVoiceReceiver::GetVolume( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_volume;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceReceiver::IsPlaying( void ) const
{GUCEF_TRACE;

    return m_playout->IsPlaying();
}

/*-------------------------------------------------------------------------*/

Float32
CVoiceReceiver::GetLevelInDecibels( void ) const
{GUCEF_TRACE;

    return m_playout->GetLevelInDecibels();
}

/*-------------------------------------------------------------------------*/

void
CVoiceReceiver::GetStats( CVoiceReceiverStats& stats ) const
{GUCEF_TRACE;

    m_playout->GetStats( stats );
    stats.framesPushed = m_framesPushed.Load( MT::MEMORDER_RELAXED );
    stats.wrongStream = m_wrongStream.Load( MT::MEMORDER_RELAXED );
    stats.wrongPayloadType = m_wrongPayloadType.Load( MT::MEMORDER_RELAXED );
    stats.streamId = m_currentStreamId.Load( MT::MEMORDER_RELAXED );

    // RFC 3550 A.3: the fraction lost over the interval since the previous call (a reset starts the counts over)
    MT::CScopeMutex lock( m_statsLock );
    if ( stats.framesExpected < m_priorExpected || stats.framesReceived < m_priorReceived )
    {
        m_priorExpected = 0;
        m_priorReceived = 0;
    }
    const Int64 expectedInterval = (Int64) ( stats.framesExpected - m_priorExpected );
    const Int64 receivedInterval = (Int64) ( stats.framesReceived - m_priorReceived );
    const Int64 lostInterval = expectedInterval - receivedInterval;
    if ( 0 == expectedInterval || lostInterval <= 0 )
        stats.fractionLost = 0;
    else
        stats.fractionLost = (UInt8) ( ( lostInterval << 8 ) / expectedInterval > 255 ? 255 : ( lostInterval << 8 ) / expectedInterval );
    m_priorExpected = stats.framesExpected;
    m_priorReceived = stats.framesReceived;
}

/*-------------------------------------------------------------------------*/

CVoicePlayoutSource&
CVoiceReceiver::GetPlayoutSource( void )
{GUCEF_TRACE;

    return *m_playout;
}

/*-------------------------------------------------------------------------*/

const TAudioSourcePtr&
CVoiceReceiver::GetSource( void ) const
{GUCEF_TRACE;

    return m_source;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
