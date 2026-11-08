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

#ifndef GUCEF_AUDIO_CAUDIOCONVERTER_H
#include "gucefAUDIO_CAudioConverter.h"
#define GUCEF_AUDIO_CAUDIOCONVERTER_H
#endif /* GUCEF_AUDIO_CAUDIOCONVERTER_H ? */

#include "gucefAUDIO_CNullAudioDriver.h"

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

CNullAudioDevice::CNullAudioDevice( const CAudioDeviceInfo& info, IAudioDeviceCallback* callback, const CAudioFormat& format, UInt32 periodSizeInFrames, bool keepOutput )
    : CAudioDevice( info, callback )
    , m_format( format )
    , m_periodSize( periodSizeInFrames )
    , m_keepOutput( keepOutput )
    , m_started( false )
    , m_totalFrames( 0 )
    , m_rendered( format )
    , m_scratch( format, periodSizeInFrames )
    , m_captureSource()
    , m_captureSourceRaw( GUCEF_NULL )
    , m_captureConverts( false )
    , m_captureRemixes( false )
    , m_captureResamples( false )
    , m_captureRead()
    , m_captureFloat()
    , m_capturePending()
    , m_capturePendingOffset( 0 )
    , m_capturePendingFrames( 0 )
    , m_captureOut()
    , m_captureMatrix()
    , m_captureResampler()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CNullAudioDevice::~CNullAudioDevice()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

const CAudioFormat&
CNullAudioDevice::GetFormat( void ) const
{GUCEF_TRACE;

    return m_format;
}

/*-------------------------------------------------------------------------*/

bool
CNullAudioDevice::Start( void )
{GUCEF_TRACE;

    m_started = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNullAudioDevice::Stop( void )
{GUCEF_TRACE;

    m_started = false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNullAudioDevice::IsStarted( void ) const
{GUCEF_TRACE;

    return m_started;
}

/*-------------------------------------------------------------------------*/

CNullAudioDevice*
CNullAudioDevice::AsNullDevice( void )
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

UInt32
CNullAudioDevice::GetPeriodSizeInFrames( void ) const
{GUCEF_TRACE;

    return m_periodSize;
}

/*-------------------------------------------------------------------------*/

UInt64
CNullAudioDevice::Render( UInt64 frameCount )
{GUCEF_TRACE;

    IAudioDeviceCallback* callback = GetCallback();
    if ( !m_started || GUCEF_NULL == callback )
        return 0;

    UInt64 done = 0;
    while ( done < frameCount )
    {
        const UInt32 piece = ( frameCount - done < m_periodSize ) ? (UInt32) ( frameCount - done ) : m_periodSize;
        if ( AUDIODIRECTION_PLAYBACK == GetDirection() )
        {
            // a real device hands over whatever is in its buffer, so the buffer starts as silence
            m_scratch.FillSilence();
            callback->OnRenderAudio( m_scratch.GetDataPtr(), piece );
            if ( m_keepOutput )
            {
                CAudioBuffer made( m_format, m_scratch.GetDataPtr(), piece );
                m_rendered.Append( made );
            }
        }
        else
        {
            if ( GUCEF_NULL == m_captureSourceRaw )
                m_scratch.FillSilence();
            else
                ReadCaptureSource( piece );
            callback->OnCaptureAudio( m_scratch.GetDataPtr(), piece );
        }
        done += piece;
    }
    m_totalFrames += done;
    return done;
}

/*-------------------------------------------------------------------------*/

bool
CNullAudioDevice::SetCaptureSource( const TAudioSourcePtr& source )
{GUCEF_TRACE;

    if ( AUDIODIRECTION_CAPTURE != GetDirection() )
        return false;

    if ( source.IsNULL() )
    {
        m_captureSourceRaw = GUCEF_NULL;
        m_captureSource.Unlink();
        m_capturePendingOffset = 0;
        m_capturePendingFrames = 0;
        return true;
    }

    const CAudioFormat& sourceFormat = source->GetFormat();
    if ( !sourceFormat.IsValid() || !m_format.IsValid() )
        return false;

    const UInt32 sourceChannels = sourceFormat.GetChannelCount();
    const UInt32 deviceChannels = m_format.GetChannelCount();
    const bool remixes = ( sourceChannels != deviceChannels ) || ( sourceFormat.GetChannelLayout() != m_format.GetChannelLayout() );
    const bool resamples = sourceFormat.GetSampleRate() != m_format.GetSampleRate();

    GUCEF::vector< Float32 > matrix;
    if ( remixes && !CAudioConverter::BuildChannelMatrix( sourceFormat, m_format, matrix ) )
        return false;
    if ( resamples && !m_captureResampler.Configure( deviceChannels, (Float64) sourceFormat.GetSampleRate(), (Float64) m_format.GetSampleRate() ) )
        return false;
    m_captureResampler.Reset();

    // everything Render() needs is made now: one period of the source at a time
    m_captureMatrix = matrix;
    m_captureRemixes = remixes;
    m_captureResamples = resamples;
    m_captureConverts = remixes || resamples || ( sourceFormat.GetSampleType() != m_format.GetSampleType() );
    if ( m_captureConverts )
    {
        m_captureRead = CAudioBuffer( sourceFormat, m_periodSize );
        m_captureFloat.assign( remixes ? (size_t) m_periodSize * sourceChannels : 0, 0.0f );
        m_capturePending.assign( (size_t) m_periodSize * deviceChannels, 0.0f );
        m_captureOut.assign( (size_t) m_periodSize * deviceChannels, 0.0f );
    }
    else
    {
        m_captureRead = CAudioBuffer();
        m_captureFloat.clear();
        m_capturePending.clear();
        m_captureOut.clear();
    }
    m_capturePendingOffset = 0;
    m_capturePendingFrames = 0;
    m_captureSource = source;
    m_captureSourceRaw = m_captureSource.GetPointerAlways();
    return true;
}

/*-------------------------------------------------------------------------*/

const TAudioSourcePtr&
CNullAudioDevice::GetCaptureSource( void ) const
{GUCEF_TRACE;

    return m_captureSource;
}

/*-------------------------------------------------------------------------*/

void
CNullAudioDevice::ReadCaptureSource( UInt32 frameCount ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    // no trace macro: this runs where a real device records, on the thread that drives the device

    IAudioSource* source = m_captureSourceRaw;
    UInt32 produced = 0;

    if ( !m_captureConverts )
    {
        // the source has the format of the device: straight in, bit for bit
        while ( produced < frameCount )
        {
            const UInt32 got = source->ReadFrames( m_scratch.GetDataPtr( produced ), frameCount - produced );
            if ( 0 == got )
                break;
            produced += got;
        }
    }
    else
    {
        const CAudioFormat& sourceFormat = source->GetFormat();
        const UInt32 sourceChannels = sourceFormat.GetChannelCount();
        const UInt32 deviceChannels = m_format.GetChannelCount();
        Float32* pendingBase = &m_capturePending[ 0 ];
        Float32* out = &m_captureOut[ 0 ];

        while ( produced < frameCount )
        {
            if ( 0 == m_capturePendingFrames )
            {
                const UInt32 got = source->ReadFrames( m_captureRead.GetDataPtr(), m_periodSize );
                if ( 0 == got )
                    break;
                if ( m_captureRemixes )
                {
                    CAudioConverter::ConvertToFloat32( m_captureRead.GetDataPtr(), sourceFormat.GetSampleType(), &m_captureFloat[ 0 ], (UInt64) got * sourceChannels );
                    CAudioConverter::RemixChannels( &m_captureFloat[ 0 ], sourceChannels, pendingBase, deviceChannels, got, m_captureMatrix );
                }
                else
                {
                    CAudioConverter::ConvertToFloat32( m_captureRead.GetDataPtr(), sourceFormat.GetSampleType(), pendingBase, (UInt64) got * sourceChannels );
                }
                m_capturePendingOffset = 0;
                m_capturePendingFrames = got;
            }

            const Float32* pending = pendingBase + (size_t) m_capturePendingOffset * deviceChannels;
            if ( m_captureResamples )
            {
                UInt32 consumed = 0;
                UInt32 made = 0;
                m_captureResampler.Process( pending, m_capturePendingFrames, consumed, out + (size_t) produced * deviceChannels, frameCount - produced, made );
                m_capturePendingOffset += consumed;
                m_capturePendingFrames -= consumed;
                produced += made;
                if ( 0 == consumed && 0 == made )
                    break;      // a resampler that neither takes nor gives would loop forever
            }
            else
            {
                const UInt32 take = ( m_capturePendingFrames < frameCount - produced ) ? m_capturePendingFrames : frameCount - produced;
                memcpy( out + (size_t) produced * deviceChannels, pending, (size_t) take * deviceChannels * sizeof( Float32 ) );
                m_capturePendingOffset += take;
                m_capturePendingFrames -= take;
                produced += take;
            }
        }

        CAudioConverter::ConvertFromFloat32( out, m_scratch.GetDataPtr(), m_format.GetSampleType(), (UInt64) produced * deviceChannels );
    }

    // after the end of the source, or while it has nothing, the device records silence
    if ( produced < frameCount )
        m_scratch.FillSilence( produced, frameCount - produced );
}

/*-------------------------------------------------------------------------*/

const CAudioBuffer&
CNullAudioDevice::GetRenderedAudio( void ) const
{GUCEF_TRACE;

    return m_rendered;
}

/*-------------------------------------------------------------------------*/

void
CNullAudioDevice::ClearRenderedAudio( void )
{GUCEF_TRACE;

    m_rendered.Clear();
}

/*-------------------------------------------------------------------------*/

UInt64
CNullAudioDevice::GetTotalFramesRendered( void ) const
{GUCEF_TRACE;

    return m_totalFrames;
}

/*-------------------------------------------------------------------------*/

void
CNullAudioDevice::SimulateDeviceLoss( void )
{GUCEF_TRACE;

    m_started = false;
    IAudioDeviceCallback* callback = GetCallback();
    if ( GUCEF_NULL != callback )
        callback->OnDeviceLost();
}

/*-------------------------------------------------------------------------*/

CNullAudioDriver::CNullAudioDriver( void )
    : CAudioDriver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CNullAudioDriver::~CNullAudioDriver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CNullAudioDriver::GetName( void ) const
{GUCEF_TRACE;

    return "null";
}

/*-------------------------------------------------------------------------*/

CString
CNullAudioDriver::GetDescription( void ) const
{GUCEF_TRACE;

    return "No audio hardware: plays to nowhere and records silence";
}

/*-------------------------------------------------------------------------*/

Int32
CNullAudioDriver::GetPriority( void ) const
{GUCEF_TRACE;

    return -1000;
}

/*-------------------------------------------------------------------------*/

CAudioFormat
CNullAudioDriver::GetDefaultFormat( void )
{GUCEF_TRACE;

    return CAudioFormat( 48000, 2, SAMPLETYPE_FLOAT32 );
}

/*-------------------------------------------------------------------------*/

bool
CNullAudioDriver::EnumerateDevices( TAudioDirection direction, TAudioDeviceInfoVector& devices )
{GUCEF_TRACE;

    CAudioDeviceInfo info( "null", "Null audio device", direction );
    info.SetIsDefault( true );
    info.SetPreferredFormat( GetDefaultFormat() );
    info.SetMaxChannelCount( 32 );
    devices.push_back( info );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNullAudioDriver::OpenDevice( const CAudioDeviceParams& params, IAudioDeviceCallback* callback, TAudioDevicePtr& device )
{GUCEF_TRACE;

    if ( !params.GetDeviceId().IsNULLOrEmpty() && params.GetDeviceId() != "null" )
        return false;

    // The null device is happy with any format, the parts that are not asked for are filled in
    CAudioFormat format = GetDefaultFormat();
    const CAudioFormat& wanted = params.GetPreferredFormat();
    if ( wanted.GetSampleRate() > 0 )
        format.SetSampleRate( wanted.GetSampleRate() );
    if ( wanted.GetChannelCount() > 0 )
    {
        if ( wanted.HasExplicitChannelLayout() )
            format.SetChannelLayout( wanted.GetChannelLayout() );
        else
            format.SetChannelCount( wanted.GetChannelCount() );
    }
    if ( SAMPLETYPE_UNKNOWN != wanted.GetSampleType() )
        format.SetSampleType( wanted.GetSampleType() );

    const UInt32 period = ( params.GetBufferSizeInFrames() > 0 ) ? params.GetBufferSizeInFrames() : 512;
    const bool keep = params.GetOptions().GetValueAlways( "keepOutput" ).AsBool( false );

    CAudioDeviceInfo info( "null", "Null audio device", params.GetDirection() );
    info.SetIsDefault( true );
    info.SetPreferredFormat( format );
    device = TAudioDevicePtr( GUCEF_NEW CNullAudioDevice( info, callback, format, period, keep ) );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
