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
            m_scratch.FillSilence();
            callback->OnCaptureAudio( m_scratch.GetDataPtr(), piece );
        }
        done += piece;
    }
    m_totalFrames += done;
    return done;
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
