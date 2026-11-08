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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCONVERTER_H
#include "gucefAUDIO_CAudioConverter.h"
#define GUCEF_AUDIO_CAUDIOCONVERTER_H
#endif /* GUCEF_AUDIO_CAUDIOCONVERTER_H ? */

#include "gucefAUDIO_CAudioCapturePath.h"

#ifndef GUCEF_AUDIO_CHALFDUPLEXCONTROLLER_H
#include "gucefAUDIO_CHalfDuplexController.h"
#define GUCEF_AUDIO_CHALFDUPLEXCONTROLLER_H
#endif /* GUCEF_AUDIO_CHALFDUPLEXCONTROLLER_H ? */

#ifndef GUCEF_AUDIO_CACOUSTICECHOCANCELLER_H
#include "gucefAUDIO_CAcousticEchoCanceller.h"
#define GUCEF_AUDIO_CACOUSTICECHOCANCELLER_H
#endif /* GUCEF_AUDIO_CACOUSTICECHOCANCELLER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const Float64 CAudioCapturePath::MuteRampInSeconds = 0.010;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Gives the memory of a vector back, which clear() does not */
template< typename T >
void
FreeVector( GUCEF::vector< T >& v )
{
    GUCEF::vector< T > empty;
    v.swap( empty );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioCapturePath::CAudioCapturePath( void )
    : m_prepared( false )
    , m_deviceFormat()
    , m_processingFormat()
    , m_channel( -1 )
    , m_maxBlockFrames( 0 )
    , m_maxEffectsPerStage( 0 )
    , m_resampling( false )
    , m_ticksPerSecond( 0.0 )
    , m_processingFramesPerDeviceFrame( 1.0 )
    , m_commands( GUCEF_NULL )
    , m_events( GUCEF_NULL )
    , m_preEcho()
    , m_postEcho()
    , m_meter()
    , m_resampler()
    , m_voiceDetector()
    , m_muted( false )
    , m_voiceActive( false )
    , m_overruns( 0 )
    , m_framePosition( 0 )
    , m_consumers()
    , m_halfDuplex( GUCEF_NULL )
    , m_echoCanceller( GUCEF_NULL )
    , m_deviceFloat()
    , m_mono()
    , m_resampled()
    , m_gain( 1.0f )
    , m_gainStep( 1.0f )
    , m_nextFrame( 0 )
    , m_seenOverruns( 0 )
    , m_lastTickCount( 0 )
    , m_discontinuityPending( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioCapturePath::~CAudioCapturePath()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

bool
CAudioCapturePath::Prepare( const CAudioFormat& deviceFormat     ,
                            UInt32 processingRate                ,
                            Int32 channel                        ,
                            UInt32 maxBlockFrames                ,
                            UInt32 maxConsumers                  ,
                            UInt32 maxEffectsPerStage            ,
                            UInt32 queueSize                     )
{GUCEF_TRACE;

    Release();

    const UInt32 channels = deviceFormat.GetChannelCount();
    if ( !deviceFormat.IsValid() || 0 == channels || 0 == maxBlockFrames || 0 == queueSize )
        return false;
    if ( channel >= 0 && (UInt32) channel >= channels )
        return false;

    const UInt32 deviceRate = deviceFormat.GetSampleRate();
    const UInt32 rate = ( processingRate > 0 ) ? processingRate : deviceRate;
    m_resampling = ( rate != deviceRate );
    if ( m_resampling )
    {
        if ( !m_resampler.Configure( 1, (Float64) deviceRate, (Float64) rate ) )
            return false;
        m_resampler.Reset();
    }

    m_deviceFormat = deviceFormat;
    m_processingFormat = CAudioFormat( rate, 1, SAMPLETYPE_FLOAT32 );
    m_channel = channel;
    m_maxBlockFrames = maxBlockFrames;
    m_maxEffectsPerStage = maxEffectsPerStage;
    m_processingFramesPerDeviceFrame = (Float64) rate / (Float64) deviceRate;
    m_ticksPerSecond = (Float64) MT::PrecisionTimerResolution();

    m_commands = GUCEF_NEW MT::CTSpscQueue< SCommand >( queueSize );
    m_events = GUCEF_NEW MT::CTSpscQueue< SEvent >( queueSize );

    // fresh slots: the switches of a path that was used before do not carry over
    m_preEcho.SetSlotCapacity( 0 );
    m_postEcho.SetSlotCapacity( 0 );
    if ( !m_preEcho.Prepare( maxEffectsPerStage, m_processingFormat, maxBlockFrames ) ||
         !m_postEcho.Prepare( maxEffectsPerStage, m_processingFormat, maxBlockFrames ) )
    {
        Release();
        return false;
    }
    m_meter.Prepare( 1, rate );
    if ( !m_voiceDetector.Prepare( rate ) )
    {
        Release();
        return false;
    }

    m_consumers.assign( maxConsumers, GUCEF_NULL );
    if ( SAMPLETYPE_FLOAT32 != deviceFormat.GetSampleType() )
        m_deviceFloat.assign( (size_t) maxBlockFrames * channels, 0.0f );
    m_mono.assign( maxBlockFrames, 0.0f );
    if ( m_resampling )
        m_resampled.assign( maxBlockFrames, 0.0f );

    m_gain = m_muted.Load() ? 0.0f : 1.0f;
    m_gainStep = (Float32) ( 1.0 / ( MuteRampInSeconds * (Float64) rate ) );
    m_nextFrame = 0;
    m_seenOverruns = 0;
    m_lastTickCount = 0;
    m_discontinuityPending = false;
    m_voiceActive.Store( false );
    m_overruns.Store( 0 );
    m_framePosition.Store( 0 );
    m_prepared = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioCapturePath::Release( void )
{GUCEF_TRACE;

    m_prepared = false;
    GUCEF_DELETE m_commands;
    m_commands = GUCEF_NULL;
    GUCEF_DELETE m_events;
    m_events = GUCEF_NULL;

    for ( UInt32 s=0; s<m_maxEffectsPerStage; ++s )
    {
        m_preEcho.SetSlotEffect( s, GUCEF_NULL );
        m_postEcho.SetSlotEffect( s, GUCEF_NULL );
    }
    m_preEcho.SetSlotCapacity( 0 );
    m_postEcho.SetSlotCapacity( 0 );
    m_maxEffectsPerStage = 0;
    m_meter.Reset();
    m_resampler.Reset();
    m_resampling = false;
    m_voiceDetector.Release();

    FreeVector( m_consumers );
    m_halfDuplex = GUCEF_NULL;
    m_echoCanceller = GUCEF_NULL;
    FreeVector( m_deviceFloat );
    FreeVector( m_mono );
    FreeVector( m_resampled );
    m_voiceActive.Store( false );
}

/*-------------------------------------------------------------------------*/

bool
CAudioCapturePath::IsPrepared( void ) const
{GUCEF_TRACE;

    return m_prepared;
}

/*-------------------------------------------------------------------------*/

const CAudioFormat&
CAudioCapturePath::GetDeviceFormat( void ) const
{GUCEF_TRACE;

    return m_deviceFormat;
}

/*-------------------------------------------------------------------------*/

const CAudioFormat&
CAudioCapturePath::GetProcessingFormat( void ) const
{GUCEF_TRACE;

    return m_processingFormat;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioCapturePath::GetMaxBlockFrames( void ) const
{GUCEF_TRACE;

    return m_maxBlockFrames;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioCapturePath::GetMaxConsumers( void ) const
{GUCEF_TRACE;

    return (UInt32) m_consumers.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioCapturePath::GetMaxEffectsPerStage( void ) const
{GUCEF_TRACE;

    return m_maxEffectsPerStage;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioCapturePath::GetQueueCapacity( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_commands || GUCEF_NULL == m_events )
        return 0;
    const UInt32 commands = m_commands->GetCapacity();
    const UInt32 events = m_events->GetCapacity();
    return commands < events ? commands : events;
}

/*-------------------------------------------------------------------------*/

bool
CAudioCapturePath::PushCommand( const SCommand& command )
{GUCEF_TRACE;

    return GUCEF_NULL != m_commands && m_commands->Push( command );
}

/*-------------------------------------------------------------------------*/

bool
CAudioCapturePath::PopEvent( SEvent& event )
{GUCEF_TRACE;

    return GUCEF_NULL != m_events && m_events->Pop( event );
}

/*-------------------------------------------------------------------------*/

void
CAudioCapturePath::SetMuted( bool muted )
{GUCEF_TRACE;

    m_muted.Store( muted, MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

bool
CAudioCapturePath::IsMuted( void ) const
{GUCEF_TRACE;

    return m_muted.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

const CAudioEffectChain*
CAudioCapturePath::FindChain( TAudioCaptureStage stage ) const
{GUCEF_TRACE;

    switch ( stage )
    {
        case AUDIOCAPTURESTAGE_PRE_ECHO :   return &m_preEcho;
        case AUDIOCAPTURESTAGE_POST_ECHO :  return &m_postEcho;
        default :                           return GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

CAudioEffectChain&
CAudioCapturePath::Chain( TAudioCaptureStage stage )
{
    return ( AUDIOCAPTURESTAGE_POST_ECHO == stage ) ? m_postEcho : m_preEcho;
}

/*-------------------------------------------------------------------------*/

void
CAudioCapturePath::SetEffectEnabled( TAudioCaptureStage stage, UInt32 slot, bool enabled )
{GUCEF_TRACE;

    if ( GUCEF_NULL != FindChain( stage ) && slot < m_maxEffectsPerStage )
        Chain( stage ).SetEnabled( slot, enabled );
}

/*-------------------------------------------------------------------------*/

bool
CAudioCapturePath::IsEffectEnabled( TAudioCaptureStage stage, UInt32 slot ) const
{GUCEF_TRACE;

    const CAudioEffectChain* chain = FindChain( stage );
    return GUCEF_NULL != chain && slot < m_maxEffectsPerStage && chain->IsEnabled( slot );
}

/*-------------------------------------------------------------------------*/

const CAudioLevelMeter&
CAudioCapturePath::GetMeter( void ) const
{GUCEF_TRACE;

    return m_meter;
}

/*-------------------------------------------------------------------------*/

bool
CAudioCapturePath::IsVoiceActive( void ) const
{GUCEF_TRACE;

    return m_voiceActive.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CAudioCapturePath::SetVoiceActivitySettings( const CVoiceActivitySettings& settings )
{GUCEF_TRACE;

    m_voiceDetector.SetSettings( settings );
}

/*-------------------------------------------------------------------------*/

const CVoiceActivitySettings&
CAudioCapturePath::GetVoiceActivitySettings( void ) const
{GUCEF_TRACE;

    return m_voiceDetector.GetSettings();
}

/*-------------------------------------------------------------------------*/

Float32
CAudioCapturePath::GetVoiceEnergyInDecibels( void ) const
{GUCEF_TRACE;

    return m_voiceDetector.GetEnergyInDecibels();
}

/*-------------------------------------------------------------------------*/

Float32
CAudioCapturePath::GetVoiceNoiseFloorInDecibels( void ) const
{GUCEF_TRACE;

    return m_voiceDetector.GetNoiseFloorInDecibels();
}

/*-------------------------------------------------------------------------*/

UInt64
CAudioCapturePath::GetOverrunCount( void ) const
{GUCEF_TRACE;

    return m_overruns.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt64
CAudioCapturePath::GetFramePosition( void ) const
{GUCEF_TRACE;

    return m_framePosition.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CAudioCapturePath::PostEvent( SEvent::EType type, const SCommand& command )
{
    // the owner never has more commands on their way than the queue holds, and every command makes one event: this fits
    SEvent event;
    event.type = type;
    event.effect = command.effect;
    event.consumer = command.consumer;
    event.halfDuplex = command.halfDuplex;
    event.echoCanceller = command.echoCanceller;
    event.token = command.token;
    m_events->Push( event );
}

/*-------------------------------------------------------------------------*/

void
CAudioCapturePath::ProcessCommands( void ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    if ( !m_prepared )
        return;

    // Commands hold plain pointers only: popping one copies a few words, no lock, no reference count
    SCommand command;
    while ( m_commands->Pop( command ) )
    {
        const bool knownStage = ( AUDIOCAPTURESTAGE_PRE_ECHO == command.stage || AUDIOCAPTURESTAGE_POST_ECHO == command.stage );
        switch ( command.type )
        {
            case SCommand::COMMAND_INSERT_EFFECT :
            {
                if ( knownStage && command.slot < m_maxEffectsPerStage && GUCEF_NULL != command.effect &&
                     GUCEF_NULL == Chain( command.stage ).GetSlotEffect( command.slot ) &&
                     Chain( command.stage ).SetSlotEffect( command.slot, command.effect ) )
                    PostEvent( SEvent::EVENT_COMMAND_DONE, command );
                else
                    PostEvent( SEvent::EVENT_COMMAND_FAILED, command );
                break;
            }
            case SCommand::COMMAND_REMOVE_EFFECT :
            {
                if ( knownStage && command.slot < m_maxEffectsPerStage && Chain( command.stage ).GetSlotEffect( command.slot ) == command.effect )
                    Chain( command.stage ).SetSlotEffect( command.slot, GUCEF_NULL );
                PostEvent( SEvent::EVENT_EFFECT_RELEASED, command );
                break;
            }
            case SCommand::COMMAND_ADD_CONSUMER :
            {
                size_t free = m_consumers.size();
                bool present = false;
                for ( size_t i=0; i<m_consumers.size(); ++i )
                {
                    if ( m_consumers[ i ] == command.consumer )
                        present = true;
                    else
                    if ( GUCEF_NULL == m_consumers[ i ] && free == m_consumers.size() )
                        free = i;
                }
                if ( GUCEF_NULL != command.consumer && !present && free < m_consumers.size() )
                {
                    m_consumers[ free ] = command.consumer;
                    PostEvent( SEvent::EVENT_COMMAND_DONE, command );
                }
                else
                    PostEvent( SEvent::EVENT_COMMAND_FAILED, command );
                break;
            }
            case SCommand::COMMAND_REMOVE_CONSUMER :
            {
                for ( size_t i=0; i<m_consumers.size(); ++i )
                    if ( m_consumers[ i ] == command.consumer )
                        m_consumers[ i ] = GUCEF_NULL;
                PostEvent( SEvent::EVENT_CONSUMER_RELEASED, command );
                break;
            }
            case SCommand::COMMAND_SET_HALF_DUPLEX :
            {
                // the event carries the controller let go of
                SCommand done = command;
                done.halfDuplex = m_halfDuplex;
                m_halfDuplex = command.halfDuplex;
                PostEvent( SEvent::EVENT_COMMAND_DONE, done );
                break;
            }
            case SCommand::COMMAND_SET_ECHO_CANCELLER :
            {
                // the event carries the canceller let go of
                SCommand done = command;
                done.echoCanceller = m_echoCanceller;
                m_echoCanceller = command.echoCanceller;
                PostEvent( SEvent::EVENT_COMMAND_DONE, done );
                break;
            }
            default :
                break;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioCapturePath::ToMono( const Float32* interleaved, UInt32 frames, Float32* mono ) const
{
    const UInt32 channels = m_deviceFormat.GetChannelCount();
    if ( 1 == channels )
    {
        memcpy( mono, interleaved, (size_t) frames * sizeof( Float32 ) );
        return;
    }
    if ( m_channel >= 0 )
    {
        const Float32* in = interleaved + m_channel;
        for ( UInt32 f=0; f<frames; ++f, in += channels )
            mono[ f ] = *in;
        return;
    }

    // the mean of all channels
    const Float32 scale = 1.0f / (Float32) channels;
    const Float32* in = interleaved;
    for ( UInt32 f=0; f<frames; ++f )
    {
        Float32 sum = 0.0f;
        for ( UInt32 c=0; c<channels; ++c )
            sum += *in++;
        mono[ f ] = sum * scale;
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioCapturePath::ApplyMicrophoneGain( Float32* mono, UInt32 frames, Float32 halfDuplexGain )
{
    // mute wins; otherwise the gain the half-duplex controller asks for (1 without one)
    const Float32 target = m_muted.Load( MT::MEMORDER_RELAXED ) ? 0.0f : halfDuplexGain;
    if ( m_gain == target )
    {
        // open: the audio passes exactly as it is; closed: silence; attenuated: the steady gain
        if ( 0.0f == target )
            memset( mono, 0, (size_t) frames * sizeof( Float32 ) );
        else
        if ( 1.0f != target )
        {
            for ( UInt32 f=0; f<frames; ++f )
                mono[ f ] *= target;
        }
        return;
    }

    // a linear ramp frame by frame towards the target, so its length does not depend on how the audio is cut into blocks
    for ( UInt32 f=0; f<frames; ++f )
    {
        if ( m_gain < target )
        {
            m_gain += m_gainStep;
            if ( m_gain > target )
                m_gain = target;
        }
        else
        if ( m_gain > target )
        {
            m_gain -= m_gainStep;
            if ( m_gain < target )
                m_gain = target;
        }
        mono[ f ] *= m_gain;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAudioCapturePath::DetectVoiceActivity( const Float32* mono, UInt32 frames, UInt64 firstFrame )
{
    // The detector looks at the processed audio and does not change it. Its analysis frames are aligned to the absolute
    // position, which jumps forward after an overrun: the detector treats that as a gap
    const bool active = m_voiceDetector.Process( mono, frames, firstFrame );
    m_voiceActive.Store( active, MT::MEMORDER_RELAXED );
    return active;
}

/*-------------------------------------------------------------------------*/

void
CAudioCapturePath::ProcessBlock( Float32* mono, UInt32 frames, UInt64 tickCount )
{
    m_preEcho.Process( mono, frames );

    // the echo stage: the echo canceller with the render tap as its reference
    if ( GUCEF_NULL != m_echoCanceller )
        m_echoCanceller->Process( mono, frames, m_nextFrame );

    m_postEcho.Process( mono, frames );

    // the half-duplex controller hears whether the canceller holds the echo down: then it may stand aside
    Float32 halfDuplexGain = 1.0f;
    if ( GUCEF_NULL != m_halfDuplex )
    {
        const bool cancelling = GUCEF_NULL != m_echoCanceller && m_echoCanceller->IsEnabled() && m_echoCanceller->IsConverged();
        m_halfDuplex->SetEchoCancellation( cancelling, cancelling ? m_echoCanceller->GetErleInDecibels() : 0.0f );
        halfDuplexGain = m_halfDuplex->Process( mono, frames, m_nextFrame );
    }
    ApplyMicrophoneGain( mono, frames, halfDuplexGain );
    const bool voiceActive = DetectVoiceActivity( mono, frames, m_nextFrame );
    m_meter.Update( mono, frames );

    Float64 sumOfSquares = 0.0;
    for ( UInt32 f=0; f<frames; ++f )
        sumOfSquares += (Float64) mono[ f ] * (Float64) mono[ f ];
    Float32 level = CAudioLevelMeter::MinimumDecibels;
    if ( sumOfSquares > 0.0 && frames > 0 )
    {
        level = (Float32) ( 10.0 * log10( sumOfSquares / (Float64) frames ) );
        if ( level < CAudioLevelMeter::MinimumDecibels )
            level = CAudioLevelMeter::MinimumDecibels;
    }

    IAudioCaptureConsumer::SCaptureBlockInfo info;
    info.firstFrame = m_nextFrame;
    info.tickCount = tickCount;
    info.discontinuity = m_discontinuityPending;
    info.voiceActive = voiceActive;
    info.levelInDecibels = level;
    for ( size_t i=0; i<m_consumers.size(); ++i )
    {
        IAudioCaptureConsumer* consumer = m_consumers[ i ];
        if ( GUCEF_NULL != consumer )
            consumer->OnCapturedAudio( mono, frames, info );
    }

    m_discontinuityPending = false;
    m_nextFrame += frames;
    m_framePosition.Store( m_nextFrame, MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CAudioCapturePath::Process( const void* frames, UInt32 frameCount, UInt64 deviceOverrunCount, UInt64 tickCount ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    if ( !m_prepared || GUCEF_NULL == frames )
        return;

    const UInt32 deviceRate = m_deviceFormat.GetSampleRate();

    // An overrun of the device: the next block is a discontinuity, and the frame position moves on by the frames that
    // were lost. Devices do not report that number, so it comes from the clock: the time since the last callback started,
    // less the time it took to record this callback's frames
    if ( deviceOverrunCount > m_seenOverruns )
    {
        m_overruns.FetchAdd( deviceOverrunCount - m_seenOverruns, MT::MEMORDER_RELAXED );
        m_seenOverruns = deviceOverrunCount;
        m_discontinuityPending = true;
        if ( 0 != m_lastTickCount && tickCount > m_lastTickCount && m_ticksPerSecond > 0.0 )
        {
            const Float64 elapsedFrames = (Float64) ( tickCount - m_lastTickCount ) / m_ticksPerSecond * (Float64) deviceRate;
            const Float64 lostFrames = elapsedFrames - (Float64) frameCount;
            if ( lostFrames > 0.0 )
                m_nextFrame += (UInt64) ( lostFrames * m_processingFramesPerDeviceFrame + 0.5 );
        }
    }
    m_lastTickCount = tickCount;

    const UInt32 channels = m_deviceFormat.GetChannelCount();
    const UInt32 frameBytes = m_deviceFormat.GetBytesPerFrame();
    const bool isFloat = m_deviceFloat.empty();
    const UInt8* in = static_cast< const UInt8* >( frames );
    UInt32 left = frameCount;
    while ( left > 0 )
    {
        const UInt32 block = ( left < m_maxBlockFrames ) ? left : m_maxBlockFrames;

        const Float32* interleaved = reinterpret_cast< const Float32* >( in );
        if ( !isFloat )
        {
            CAudioConverter::ConvertToFloat32( in, m_deviceFormat.GetSampleType(), &m_deviceFloat[ 0 ], (UInt64) block * channels );
            interleaved = &m_deviceFloat[ 0 ];
        }
        ToMono( interleaved, block, &m_mono[ 0 ] );

        if ( !m_resampling )
            ProcessBlock( &m_mono[ 0 ], block, tickCount );
        else
        {
            // the output of one piece of input may be more than a block (upsampling): the resampler stops when the
            // block is full and takes the rest of the input in the next round
            UInt32 offset = 0;
            while ( offset < block )
            {
                UInt32 consumed = 0;
                UInt32 produced = 0;
                m_resampler.Process( &m_mono[ offset ], block - offset, consumed, &m_resampled[ 0 ], m_maxBlockFrames, produced );
                if ( produced > 0 )
                    ProcessBlock( &m_resampled[ 0 ], produced, tickCount );
                offset += consumed;
                if ( 0 == consumed && 0 == produced )
                    break;
            }
        }

        in += (size_t) block * frameBytes;
        left -= block;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
