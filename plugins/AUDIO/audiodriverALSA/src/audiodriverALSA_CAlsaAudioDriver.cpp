/*
 *  audiodriverALSA: gucefAUDIO driver plugin for ALSA on Linux
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "audiodriverALSA_CAlsaAudioDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIODRIVER {
namespace ALSA {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const CORE::UInt32 DefaultSampleRate = 48000;
const CORE::UInt32 DefaultChannels = 2;
const CORE::UInt32 DefaultPeriodSizeInFrames = 1024;
const CORE::UInt32 DefaultPeriodCount = 4;

/** Takes a string that ALSA made and frees it with the C library as ALSA asks */
CORE::CString
TakeAlsaString( char* text )
{
    if ( GUCEF_NULL == text )
        return CORE::CString::Empty;
    CORE::CString result( text );
    ::free( text );
    return result;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAlsaAudioDevice::CAlsaAudioDevice( const AUDIO::CAudioDeviceInfo& info   ,
                                    AUDIO::IAudioDeviceCallback* callback ,
                                    const AUDIO::CAudioFormat& format     ,
                                    CORE::UInt32 periodSizeInFrames       ,
                                    CORE::UInt32 bufferSizeInFrames       ,
                                    snd_pcm_t* pcm                        ,
                                    CAlsaApi& api                         )
    : AUDIO::CAudioPushDevice( info, callback, format, periodSizeInFrames )
    , m_pcm( pcm )
    , m_api( api )
    , m_bufferSizeInFrames( bufferSizeInFrames )
    , m_bytesPerFrame( format.GetBytesPerFrame() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAlsaAudioDevice::~CAlsaAudioDevice()
{GUCEF_TRACE;

    // stopped here and not in the base class: the hooks of this class are gone by the time that destructor runs
    Stop();
    if ( GUCEF_NULL != m_pcm )
    {
        m_api.pcmClose( m_pcm );
        m_pcm = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CAlsaAudioDevice::GetLatencyInFrames( void ) const
{GUCEF_TRACE;

    return m_bufferSizeInFrames;
}

/*-------------------------------------------------------------------------*/

bool
CAlsaAudioDevice::OnPushStart( void )
{GUCEF_TRACE;

    const int result = m_api.pcmPrepare( m_pcm );
    if ( result < 0 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AlsaAudioDevice: Could not prepare \"" << GetDeviceInfo().GetId() << "\": " << m_api.ErrorText( result ) );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

AUDIO::CAudioPushDevice::TPushResult
CAlsaAudioDevice::PushFrames( const void* frames, CORE::UInt32 frameCount )
{
    // no trace macro: this runs once per period on the device thread

    const CORE::UInt8* data = static_cast< const CORE::UInt8* >( frames );
    CORE::UInt32 remaining = frameCount;
    bool recovered = false;
    while ( remaining > 0 )
    {
        const snd_pcm_sframes_t written = m_api.pcmWritei( m_pcm, data, remaining );
        if ( written >= 0 )
        {
            data += (size_t) written * m_bytesPerFrame;
            remaining -= (CORE::UInt32) written;
            continue;
        }

        // ran dry, suspended or interrupted: ALSA can recover from those, the rest of the period is written after
        const int error = (int) written;
        if ( ( -EPIPE == error || -ESTRPIPE == error || -EINTR == error ) && m_api.pcmRecover( m_pcm, error, 1 ) >= 0 )
        {
            recovered = recovered || -EINTR != error;
            continue;
        }

        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AlsaAudioDevice: \"" << GetDeviceInfo().GetId() << "\" is lost: " << m_api.ErrorText( error ) );
        return PUSHRESULT_DEVICE_LOST;
    }
    return recovered ? PUSHRESULT_UNDERRUN_RECOVERED : PUSHRESULT_OK;
}

/*-------------------------------------------------------------------------*/

void
CAlsaAudioDevice::OnPushStop( void )
{GUCEF_TRACE;

    m_api.pcmDrop( m_pcm );
}

/*-------------------------------------------------------------------------*/

CAlsaAudioDriver::CAlsaAudioDriver( CAlsaApi& api )
    : AUDIO::CAudioDriver()
    , m_api( api )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAlsaAudioDriver::~CAlsaAudioDriver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAlsaAudioDriver::GetName( void ) const
{GUCEF_TRACE;

    return "alsa";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAlsaAudioDriver::GetDescription( void ) const
{GUCEF_TRACE;

    return "ALSA, the sound system of the Linux kernel, through libasound";
}

/*-------------------------------------------------------------------------*/

CORE::Int32
CAlsaAudioDriver::GetPriority( void ) const
{GUCEF_TRACE;

    // a driver for a sound server (PipeWire, PulseAudio) can go above this one, ALSA reaches those through "default" as well
    return 10;
}

/*-------------------------------------------------------------------------*/

bool
CAlsaAudioDriver::EnumerateDevices( AUDIO::TAudioDirection direction, AUDIO::TAudioDeviceInfoVector& devices )
{GUCEF_TRACE;

    if ( AUDIO::AUDIODIRECTION_PLAYBACK != direction )
        return true;                // playback only for now

    void** hints = GUCEF_NULL;
    if ( m_api.deviceNameHint( -1, "pcm", &hints ) < 0 || GUCEF_NULL == hints )
    {
        AUDIO::CAudioDeviceInfo info( "default", "Default", AUDIO::AUDIODIRECTION_PLAYBACK );
        info.SetIsDefault( true );
        devices.push_back( info );
        return true;
    }

    for ( void** hint = hints; GUCEF_NULL != *hint; ++hint )
    {
        const CORE::CString name = TakeAlsaString( m_api.deviceNameGetHint( *hint, "NAME" ) );
        const CORE::CString description = TakeAlsaString( m_api.deviceNameGetHint( *hint, "DESC" ) );
        const CORE::CString ioid = TakeAlsaString( m_api.deviceNameGetHint( *hint, "IOID" ) );
        if ( name.IsNULLOrEmpty() || "null" == name || ( !ioid.IsNULLOrEmpty() && "Output" != ioid ) )
            continue;

        // the description has a line per detail, they read fine as one line
        AUDIO::CAudioDeviceInfo info( name, description.IsNULLOrEmpty() ? name : description.ReplaceChar( '\n', ' ' ), AUDIO::AUDIODIRECTION_PLAYBACK );
        info.SetIsDefault( "default" == name );
        devices.push_back( info );
    }
    m_api.deviceNameFreeHint( hints );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAlsaAudioDriver::Negotiate( snd_pcm_t* pcm                          ,
                             const AUDIO::CAudioDeviceParams& params ,
                             AUDIO::CAudioFormat& format             ,
                             CORE::UInt32& periodSizeInFrames        ,
                             CORE::UInt32& bufferSizeInFrames        )
{GUCEF_TRACE;

    snd_pcm_hw_params_t* hw = GUCEF_NULL;
    if ( m_api.hwParamsMalloc( &hw ) < 0 || GUCEF_NULL == hw )
        return false;

    bool success = false;
    do
    {
        int result = m_api.hwParamsAny( pcm, hw );
        if ( result < 0 )
            break;
        result = m_api.hwParamsSetAccess( pcm, hw, CAlsaApi::AccessRwInterleaved );
        if ( result < 0 )
            break;

        // the sample type asked for, else the best one the device has
        const AUDIO::CAudioFormat& preferred = params.GetPreferredFormat();
        AUDIO::TSampleType candidates[ 6 ] = { preferred.GetSampleType(), AUDIO::SAMPLETYPE_FLOAT32, AUDIO::SAMPLETYPE_INT32,
                                               AUDIO::SAMPLETYPE_INT16, AUDIO::SAMPLETYPE_INT24, AUDIO::SAMPLETYPE_UINT8 };
        AUDIO::TSampleType sampleType = AUDIO::SAMPLETYPE_UNKNOWN;
        for ( int i=0; i<6 && AUDIO::SAMPLETYPE_UNKNOWN == sampleType; ++i )
        {
            const int alsaFormat = CAlsaApi::ToAlsaFormat( candidates[ i ] );
            if ( alsaFormat >= 0 && 0 == m_api.hwParamsTestFormat( pcm, hw, alsaFormat ) && 0 == m_api.hwParamsSetFormat( pcm, hw, alsaFormat ) )
                sampleType = candidates[ i ];
        }
        if ( AUDIO::SAMPLETYPE_UNKNOWN == sampleType )
            break;

        unsigned int channels = preferred.GetChannelCount() > 0 ? preferred.GetChannelCount() : DefaultChannels;
        if ( m_api.hwParamsSetChannelsNear( pcm, hw, &channels ) < 0 )
            break;

        unsigned int rate = preferred.GetSampleRate() > 0 ? preferred.GetSampleRate() : DefaultSampleRate;
        int dir = 0;
        if ( m_api.hwParamsSetRateNear( pcm, hw, &rate, &dir ) < 0 )
            break;

        snd_pcm_uframes_t period = params.GetBufferSizeInFrames() > 0 ? params.GetBufferSizeInFrames() : DefaultPeriodSizeInFrames;
        dir = 0;
        if ( m_api.hwParamsSetPeriodSizeNear( pcm, hw, &period, &dir ) < 0 )
            break;

        CORE::UInt32 periods = params.GetOptions().GetValueAlways( "periods" ).AsUInt32( DefaultPeriodCount );
        if ( periods < 2 )
            periods = 2;
        snd_pcm_uframes_t buffer = period * periods;
        if ( m_api.hwParamsSetBufferSizeNear( pcm, hw, &buffer ) < 0 )
            break;

        result = m_api.hwParams( pcm, hw );
        if ( result < 0 )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AlsaAudioDriver: The device refused the parameters: " << m_api.ErrorText( result ) );
            break;
        }

        dir = 0;
        m_api.hwParamsGetPeriodSize( hw, &period, &dir );
        m_api.hwParamsGetBufferSize( hw, &buffer );

        format = AUDIO::CAudioFormat( rate, channels, sampleType );
        periodSizeInFrames = (CORE::UInt32) period;
        bufferSizeInFrames = (CORE::UInt32) buffer;
        success = true;
    }
    while ( false );

    m_api.hwParamsFree( hw );
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAlsaAudioDriver::OpenDevice( const AUDIO::CAudioDeviceParams& params ,
                              AUDIO::IAudioDeviceCallback* callback   ,
                              AUDIO::TAudioDevicePtr& device          )
{GUCEF_TRACE;

    if ( AUDIO::AUDIODIRECTION_PLAYBACK != params.GetDirection() || GUCEF_NULL == callback )
        return false;

    const CORE::CString pcmName = params.GetDeviceId().IsNULLOrEmpty() ? CORE::CString( "default" ) : params.GetDeviceId();
    snd_pcm_t* pcm = GUCEF_NULL;
    const int result = m_api.pcmOpen( &pcm, pcmName.C_String(), CAlsaApi::StreamPlayback, 0 );
    if ( result < 0 || GUCEF_NULL == pcm )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AlsaAudioDriver: Could not open \"" << pcmName << "\": " << m_api.ErrorText( result ) );
        return false;
    }

    AUDIO::CAudioFormat format;
    CORE::UInt32 periodSize = 0;
    CORE::UInt32 bufferSize = 0;
    if ( !Negotiate( pcm, params, format, periodSize, bufferSize ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AlsaAudioDriver: No usable format for \"" << pcmName << "\"" );
        m_api.pcmClose( pcm );
        return false;
    }

    AUDIO::CAudioDeviceInfo info( pcmName, pcmName, AUDIO::AUDIODIRECTION_PLAYBACK );
    info.SetIsDefault( "default" == pcmName );
    info.SetPreferredFormat( format );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AlsaAudioDriver: Opened \"" << pcmName << "\" at " << format.GetSampleRate() << " Hz, " <<
        format.GetChannelCount() << " channels, sample type " << (CORE::UInt32) format.GetSampleType() << ", period " << periodSize << " frames, buffer " << bufferSize << " frames" );

    device = AUDIO::TAudioDevicePtr( GUCEF_NEW CAlsaAudioDevice( info, callback, format, periodSize, bufferSize, pcm, m_api ) );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ALSA */
}; /* namespace AUDIODRIVER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
