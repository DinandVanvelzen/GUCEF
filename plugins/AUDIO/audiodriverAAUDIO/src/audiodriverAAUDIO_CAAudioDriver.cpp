/*
 *  audiodriverAAUDIO: gucefAUDIO driver plugin for AAudio on Android
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "audiodriverAAUDIO_CAAudioDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIODRIVER {
namespace AAUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const CORE::UInt32 DefaultBurstCount = 2;
static const CORE::Int64 WriteTimeoutInNs = 100 * 1000 * 1000;     // so a stop request is seen within 100 ms
static const CORE::UInt32 MaxStalls = 10;                           // write time outs in a row before the device counts as lost

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAAudioDevice::CAAudioDevice( const AUDIO::CAudioDeviceInfo& info   ,
                              AUDIO::IAudioDeviceCallback* callback ,
                              const AUDIO::CAudioFormat& format     ,
                              CORE::UInt32 periodSizeInFrames       ,
                              CORE::UInt32 bufferSizeInFrames       ,
                              AAudioStream* stream                  ,
                              CAAudioApi& api                       )
    : AUDIO::CAudioPushDevice( info, callback, format, periodSizeInFrames )
    , m_stream( stream )
    , m_api( api )
    , m_bufferSizeInFrames( bufferSizeInFrames )
    , m_bytesPerFrame( format.GetBytesPerFrame() )
    , m_xruns( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAAudioDevice::~CAAudioDevice()
{GUCEF_TRACE;

    // stopped here and not in the base class: the hooks of this class are gone by the time that destructor runs
    Stop();
    if ( GUCEF_NULL != m_stream )
    {
        m_api.streamClose( m_stream );
        m_stream = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CAAudioDevice::GetLatencyInFrames( void ) const
{GUCEF_TRACE;

    return m_bufferSizeInFrames;
}

/*-------------------------------------------------------------------------*/

bool
CAAudioDevice::OnPushStart( void )
{GUCEF_TRACE;

    const aaudio_result_t result = m_api.streamRequestStart( m_stream );
    if ( CAAudioApi::Ok != result )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AAudioDevice: Could not start \"" << GetDeviceInfo().GetId() << "\": " << m_api.ResultText( result ) );
        return false;
    }
    m_xruns = m_api.streamGetXRunCount( m_stream );
    return true;
}

/*-------------------------------------------------------------------------*/

AUDIO::CAudioPushDevice::TPushResult
CAAudioDevice::PushFrames( const void* frames, CORE::UInt32 frameCount )
{
    // no trace macro: this runs once per period on the device thread

    const CORE::UInt8* data = static_cast< const CORE::UInt8* >( frames );
    CORE::UInt32 remaining = frameCount;
    CORE::UInt32 stalls = 0;
    while ( remaining > 0 )
    {
        // returns the frames written, fewer (also 0) when the time is up, or an error
        const aaudio_result_t written = m_api.streamWrite( m_stream, data, (CORE::Int32) remaining, WriteTimeoutInNs );
        if ( written < 0 )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AAudioDevice: \"" << GetDeviceInfo().GetId() << "\" is lost: " << m_api.ResultText( written ) );
            return PUSHRESULT_DEVICE_LOST;
        }
        if ( 0 == written )
        {
            // the stream did not move for the whole time out; a second of that and it is not coming back
            if ( ++stalls >= MaxStalls )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AAudioDevice: \"" << GetDeviceInfo().GetId() << "\" stopped taking audio" );
                return PUSHRESULT_DEVICE_LOST;
            }
            continue;
        }
        stalls = 0;

        data += (size_t) written * m_bytesPerFrame;
        remaining -= (CORE::UInt32) written;
    }

    // AAudio recovers from running dry by itself, it only counts them
    const CORE::Int32 xruns = m_api.streamGetXRunCount( m_stream );
    if ( xruns != m_xruns )
    {
        m_xruns = xruns;
        return PUSHRESULT_UNDERRUN_RECOVERED;
    }
    return PUSHRESULT_OK;
}

/*-------------------------------------------------------------------------*/

void
CAAudioDevice::OnPushStop( void )
{GUCEF_TRACE;

    m_api.streamRequestStop( m_stream );
}

/*-------------------------------------------------------------------------*/

CAAudioDriver::CAAudioDriver( CAAudioApi& api )
    : AUDIO::CAudioDriver()
    , m_api( api )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAAudioDriver::~CAAudioDriver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAAudioDriver::GetName( void ) const
{GUCEF_TRACE;

    return "aaudio";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAAudioDriver::GetDescription( void ) const
{GUCEF_TRACE;

    return "AAudio, the native audio interface of Android 8 and newer";
}

/*-------------------------------------------------------------------------*/

CORE::Int32
CAAudioDriver::GetPriority( void ) const
{GUCEF_TRACE;

    return 10;
}

/*-------------------------------------------------------------------------*/

bool
CAAudioDriver::EnumerateDevices( AUDIO::TAudioDirection direction, AUDIO::TAudioDeviceInfoVector& devices )
{GUCEF_TRACE;

    if ( AUDIO::AUDIODIRECTION_PLAYBACK != direction )
        return true;                // playback only for now

    AUDIO::CAudioDeviceInfo info( "default", "Default", AUDIO::AUDIODIRECTION_PLAYBACK );
    info.SetIsDefault( true );
    devices.push_back( info );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAAudioDriver::OpenDevice( const AUDIO::CAudioDeviceParams& params ,
                           AUDIO::IAudioDeviceCallback* callback   ,
                           AUDIO::TAudioDevicePtr& device          )
{GUCEF_TRACE;

    if ( AUDIO::AUDIODIRECTION_PLAYBACK != params.GetDirection() || GUCEF_NULL == callback )
        return false;

    const CORE::CString deviceId = params.GetDeviceId().IsNULLOrEmpty() ? CORE::CString( "default" ) : params.GetDeviceId();
    const CORE::Int32 aaudioDeviceId = "default" == deviceId ? CAAudioApi::Unspecified : CORE::StringToInt32( deviceId );

    AAudioStreamBuilder* builder = GUCEF_NULL;
    aaudio_result_t result = m_api.createStreamBuilder( &builder );
    if ( CAAudioApi::Ok != result || GUCEF_NULL == builder )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AAudioDriver: Could not make a stream builder: " << m_api.ResultText( result ) );
        return false;
    }

    // what is not asked for is left to AAudio, which then picks what suits the device best
    const AUDIO::CAudioFormat& preferred = params.GetPreferredFormat();
    m_api.builderSetDeviceId( builder, aaudioDeviceId );
    m_api.builderSetDirection( builder, CAAudioApi::DirectionOutput );
    m_api.builderSetSharingMode( builder, AUDIO::AUDIOSHAREMODE_EXCLUSIVE == params.GetShareMode() ? CAAudioApi::SharingModeExclusive : CAAudioApi::SharingModeShared );
    if ( preferred.IsValid() )
    {
        m_api.builderSetSampleRate( builder, (CORE::Int32) preferred.GetSampleRate() );
        m_api.builderSetChannelCount( builder, (CORE::Int32) preferred.GetChannelCount() );
        const CORE::Int32 format = CAAudioApi::ToAAudioFormat( preferred.GetSampleType() );
        m_api.builderSetFormat( builder, CAAudioApi::Unspecified != format ? format : CAAudioApi::FormatPcmFloat );
    }
    if ( params.GetBufferSizeInFrames() > 0 )
        m_api.builderSetBufferCapacityInFrames( builder, (CORE::Int32) params.GetBufferSizeInFrames() * 2 );
    const bool lowLatency = params.GetOptions().GetValueAlways( "lowLatency" ).AsBool( true );
    m_api.builderSetPerformanceMode( builder, lowLatency ? CAAudioApi::PerformanceModeLowLatency : CAAudioApi::PerformanceModeNone );
    if ( GUCEF_NULL != m_api.builderSetUsage )
        m_api.builderSetUsage( builder, CAAudioApi::UsageGame );

    AAudioStream* stream = GUCEF_NULL;
    result = m_api.builderOpenStream( builder, &stream );
    m_api.builderDelete( builder );
    if ( CAAudioApi::Ok != result || GUCEF_NULL == stream )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AAudioDriver: Could not open \"" << deviceId << "\": " << m_api.ResultText( result ) );
        return false;
    }

    // what we got, which can differ from what was asked
    const AUDIO::TSampleType sampleType = CAAudioApi::FromAAudioFormat( m_api.streamGetFormat( stream ) );
    const AUDIO::CAudioFormat format( (CORE::UInt32) m_api.streamGetSampleRate( stream ), (CORE::UInt32) m_api.streamGetChannelCount( stream ), sampleType );
    if ( !format.IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AAudioDriver: \"" << deviceId << "\" opened with a format that cannot be used" );
        m_api.streamClose( stream );
        return false;
    }

    // a period is a burst, the unit the device works in; the buffer holds a few of them
    CORE::Int32 burst = m_api.streamGetFramesPerBurst( stream );
    if ( burst <= 0 )
        burst = 192;
    CORE::UInt32 bursts = params.GetOptions().GetValueAlways( "bursts" ).AsUInt32( DefaultBurstCount );
    if ( bursts < 1 )
        bursts = 1;
    m_api.streamSetBufferSizeInFrames( stream, (CORE::Int32) ( bursts * burst ) );
    const CORE::Int32 bufferSize = m_api.streamGetBufferSizeInFrames( stream );

    AUDIO::CAudioDeviceInfo info( deviceId, "default" == deviceId ? CORE::CString( "Default" ) : deviceId, AUDIO::AUDIODIRECTION_PLAYBACK );
    info.SetIsDefault( "default" == deviceId );
    info.SetPreferredFormat( format );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AAudioDriver: Opened \"" << deviceId << "\" at " << format.GetSampleRate() << " Hz, " <<
        format.GetChannelCount() << " channels, sample type " << (CORE::UInt32) format.GetSampleType() << ", burst " << burst << " frames, buffer " << bufferSize << " frames" );

    device = AUDIO::TAudioDevicePtr( GUCEF_NEW CAAudioDevice( info, callback, format, (CORE::UInt32) burst, (CORE::UInt32) ( bufferSize > 0 ? bufferSize : burst ), stream, m_api ) );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AAUDIO */
}; /* namespace AUDIODRIVER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
