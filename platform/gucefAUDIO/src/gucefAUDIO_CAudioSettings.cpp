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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUDIO_CAUDIOENGINEPARAMS_H
#include "gucefAUDIO_CAudioEngineParams.h"
#define GUCEF_AUDIO_CAUDIOENGINEPARAMS_H
#endif /* GUCEF_AUDIO_CAUDIOENGINEPARAMS_H ? */

#ifndef GUCEF_AUDIO_CAUDIOENGINE_H
#include "gucefAUDIO_CAudioEngine.h"
#define GUCEF_AUDIO_CAUDIOENGINE_H
#endif /* GUCEF_AUDIO_CAUDIOENGINE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOBUS_H
#include "gucefAUDIO_CAudioBus.h"
#define GUCEF_AUDIO_CAUDIOBUS_H
#endif /* GUCEF_AUDIO_CAUDIOBUS_H ? */

#ifndef GUCEF_AUDIO_CAUDIOFORMAT_H
#include "gucefAUDIO_CAudioFormat.h"
#define GUCEF_AUDIO_CAUDIOFORMAT_H
#endif /* GUCEF_AUDIO_CAUDIOFORMAT_H ? */

#include "gucefAUDIO_CAudioSettings.h"

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

const CString CAudioSettings::ConfigSectionName = "AudioSettings";
const Float32 CAudioSettings::SilenceInDecibels = -144.0f;

namespace {

const Float32 FloorInDecibels = -60.0f;     /**< the volume law: the quietest sound above silence is just above this */

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioSettings::CAudioSettings( void )
    : CORE::CIConfigurable()
    , m_driverName()
    , m_deviceId()
    , m_captureDeviceId()
    , m_sampleRate( 0 )
    , m_bufferSizeInFrames( 0 )
    , m_channelLayout( 0 )
    , m_limiterMode( AUDIOLIMITERMODE_INSTANT )
    , m_hasLimiterMode( false )
    , m_latencyProfile( AUDIOLATENCYPROFILE_DEFAULT )
    , m_master()
    , m_buses()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioSettings::CAudioSettings( const CAudioSettings& src )
    : CORE::CIConfigurable()
    , m_driverName( src.m_driverName )
    , m_deviceId( src.m_deviceId )
    , m_captureDeviceId( src.m_captureDeviceId )
    , m_sampleRate( src.m_sampleRate )
    , m_bufferSizeInFrames( src.m_bufferSizeInFrames )
    , m_channelLayout( src.m_channelLayout )
    , m_limiterMode( src.m_limiterMode )
    , m_hasLimiterMode( src.m_hasLimiterMode )
    , m_latencyProfile( src.m_latencyProfile )
    , m_master( src.m_master )
    , m_buses( src.m_buses )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioSettings::~CAudioSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioSettings&
CAudioSettings::operator=( const CAudioSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_driverName = src.m_driverName;
        m_deviceId = src.m_deviceId;
        m_captureDeviceId = src.m_captureDeviceId;
        m_sampleRate = src.m_sampleRate;
        m_bufferSizeInFrames = src.m_bufferSizeInFrames;
        m_channelLayout = src.m_channelLayout;
        m_limiterMode = src.m_limiterMode;
        m_hasLimiterMode = src.m_hasLimiterMode;
        m_latencyProfile = src.m_latencyProfile;
        m_master = src.m_master;
        m_buses = src.m_buses;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettings::SetDriverName( const CString& driverName )
{GUCEF_TRACE;

    m_driverName = driverName;
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioSettings::GetDriverName( void ) const
{GUCEF_TRACE;

    return m_driverName;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettings::SetDeviceId( const CString& deviceId )
{GUCEF_TRACE;

    m_deviceId = deviceId;
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioSettings::GetDeviceId( void ) const
{GUCEF_TRACE;

    return m_deviceId;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettings::SetCaptureDeviceId( const CString& deviceId )
{GUCEF_TRACE;

    m_captureDeviceId = deviceId;
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioSettings::GetCaptureDeviceId( void ) const
{GUCEF_TRACE;

    return m_captureDeviceId;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettings::SetSampleRate( UInt32 sampleRate )
{GUCEF_TRACE;

    m_sampleRate = sampleRate;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioSettings::GetSampleRate( void ) const
{GUCEF_TRACE;

    return m_sampleRate;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettings::SetBufferSizeInFrames( UInt32 frames )
{GUCEF_TRACE;

    m_bufferSizeInFrames = frames;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioSettings::GetBufferSizeInFrames( void ) const
{GUCEF_TRACE;

    return m_bufferSizeInFrames;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettings::SetChannelLayout( UInt32 channelLayout )
{GUCEF_TRACE;

    m_channelLayout = channelLayout;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioSettings::GetChannelLayout( void ) const
{GUCEF_TRACE;

    return m_channelLayout;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettings::SetLimiterMode( TAudioLimiterMode mode )
{GUCEF_TRACE;

    m_limiterMode = mode;
    m_hasLimiterMode = true;
}

/*-------------------------------------------------------------------------*/

TAudioLimiterMode
CAudioSettings::GetLimiterMode( void ) const
{GUCEF_TRACE;

    return m_limiterMode;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettings::ClearLimiterMode( void )
{GUCEF_TRACE;

    m_limiterMode = AUDIOLIMITERMODE_INSTANT;
    m_hasLimiterMode = false;
}

/*-------------------------------------------------------------------------*/

bool
CAudioSettings::HasLimiterMode( void ) const
{GUCEF_TRACE;

    return m_hasLimiterMode;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettings::SetLatencyProfile( TAudioLatencyProfile profile )
{GUCEF_TRACE;

    m_latencyProfile = profile;
}

/*-------------------------------------------------------------------------*/

TAudioLatencyProfile
CAudioSettings::GetLatencyProfile( void ) const
{GUCEF_TRACE;

    return m_latencyProfile;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioSettings::LimitVolume( Float32 decibels )
{GUCEF_TRACE;

    // not a number is silence, a gain above 0 dB is not offered
    if ( decibels != decibels )
        return SilenceInDecibels;
    if ( decibels > 0.0f )
        return 0.0f;
    if ( decibels < SilenceInDecibels )
        return SilenceInDecibels;
    return decibels;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettings::SetMasterVolumeInDecibels( Float32 decibels )
{GUCEF_TRACE;

    m_master.volumeInDecibels = LimitVolume( decibels );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioSettings::GetMasterVolumeInDecibels( void ) const
{GUCEF_TRACE;

    return m_master.volumeInDecibels;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettings::SetMasterMuted( bool muted )
{GUCEF_TRACE;

    m_master.muted = muted;
}

/*-------------------------------------------------------------------------*/

bool
CAudioSettings::IsMasterMuted( void ) const
{GUCEF_TRACE;

    return m_master.muted;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettings::SetBusVolumeInDecibels( const CString& busName, Float32 decibels )
{GUCEF_TRACE;

    m_buses[ busName ].volumeInDecibels = LimitVolume( decibels );
}

/*-------------------------------------------------------------------------*/

void
CAudioSettings::SetBusMuted( const CString& busName, bool muted )
{GUCEF_TRACE;

    m_buses[ busName ].muted = muted;
}

/*-------------------------------------------------------------------------*/

bool
CAudioSettings::GetBusSettings( const CString& busName, SBusSettings& settings ) const
{GUCEF_TRACE;

    TBusSettingsMap::const_iterator i = m_buses.find( busName );
    if ( i == m_buses.end() )
        return false;
    settings = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

const CAudioSettings::TBusSettingsMap&
CAudioSettings::GetAllBusSettings( void ) const
{GUCEF_TRACE;

    return m_buses;
}

/*-------------------------------------------------------------------------*/

bool
CAudioSettings::RemoveBusSettings( const CString& busName )
{GUCEF_TRACE;

    return 0 != m_buses.erase( busName );
}

/*-------------------------------------------------------------------------*/

bool
CAudioSettings::RequiresDeviceReopen( const CAudioSettings& applied ) const
{GUCEF_TRACE;

    if ( m_driverName != applied.m_driverName ||
         m_deviceId != applied.m_deviceId ||
         m_captureDeviceId != applied.m_captureDeviceId ||
         m_latencyProfile != applied.m_latencyProfile ||
         m_sampleRate != applied.m_sampleRate ||
         m_bufferSizeInFrames != applied.m_bufferSizeInFrames ||
         m_channelLayout != applied.m_channelLayout ||
         m_hasLimiterMode != applied.m_hasLimiterMode )
    {
        return true;
    }
    return m_hasLimiterMode && m_limiterMode != applied.m_limiterMode;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettings::ApplyToEngineParams( CAudioEngineParams& params ) const
{GUCEF_TRACE;

    if ( !m_driverName.IsNULLOrEmpty() )
        params.SetDriverName( m_driverName );

    CAudioDeviceParams& deviceParams = params.GetDeviceParams();
    if ( !m_deviceId.IsNULLOrEmpty() )
        deviceParams.SetDeviceId( m_deviceId );

    if ( !m_captureDeviceId.IsNULLOrEmpty() )
        params.GetCaptureDeviceParams().SetDeviceId( m_captureDeviceId );

    if ( m_sampleRate > 0 || m_channelLayout != 0 )
    {
        CAudioFormat format( deviceParams.GetPreferredFormat() );
        if ( m_sampleRate > 0 )
            format.SetSampleRate( m_sampleRate );
        if ( m_channelLayout != 0 )
            format.SetChannelLayout( m_channelLayout );
        deviceParams.SetPreferredFormat( format );
    }

    // the profile first: the explicit buffer size and limiter mode of the settings win over its values
    if ( AUDIOLATENCYPROFILE_DEFAULT != m_latencyProfile )
    {
        UInt32 rate = m_sampleRate;
        if ( 0 == rate )
            rate = deviceParams.GetPreferredFormat().GetSampleRate();
        params.ApplyLatencyProfile( m_latencyProfile, 0 == rate ? 48000 : rate );
    }

    if ( m_bufferSizeInFrames > 0 )
        deviceParams.SetBufferSizeInFrames( m_bufferSizeInFrames );

    if ( m_hasLimiterMode )
        params.SetLimiterMode( m_limiterMode );
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioSettings::ApplyLive( CAudioEngine& engine ) const
{GUCEF_TRACE;

    if ( !engine.IsInitialized() )
        return 0;

    UInt32 applied = 0;
    CAudioBus* master = engine.GetMasterBus();
    if ( GUCEF_NULL != master )
    {
        if ( m_master.volumeInDecibels <= SilenceInDecibels )
            master->SetVolume( 0.0f );
        else
            master->SetVolumeInDecibels( m_master.volumeInDecibels );
        master->SetMuted( m_master.muted );
        ++applied;
    }

    TBusSettingsMap::const_iterator i = m_buses.begin();
    while ( i != m_buses.end() )
    {
        CAudioBus* bus = engine.FindBus( (*i).first );
        if ( GUCEF_NULL == bus )
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AudioSettings: The engine has no bus named \"" << (*i).first << "\", its settings are skipped" );
        }
        else
        {
            if ( (*i).second.volumeInDecibels <= SilenceInDecibels )
                bus->SetVolume( 0.0f );
            else
                bus->SetVolumeInDecibels( (*i).second.volumeInDecibels );
            bus->SetMuted( (*i).second.muted );
            ++applied;
        }
        ++i;
    }
    return applied;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioSettings::PercentToDecibels( UInt32 percent )
{GUCEF_TRACE;

    if ( 0 == percent )
        return SilenceInDecibels;
    if ( percent >= 100 )
        return 0.0f;
    return -FloorInDecibels * ( (Float32) percent / 100.0f - 1.0f );
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioSettings::DecibelsToPercent( Float32 decibels )
{GUCEF_TRACE;

    if ( decibels != decibels || decibels <= FloorInDecibels )
        return 0;
    if ( decibels >= 0.0f )
        return 100;
    Float64 percent = 100.0 * ( 1.0 + (Float64) decibels / (Float64) -FloorInDecibels );
    if ( percent < 0.0 )
        return 0;
    return (UInt32) ( percent + 0.5 );
}

/*-------------------------------------------------------------------------*/

CString
CAudioSettings::ChannelLayoutToString( UInt32 channelLayout )
{GUCEF_TRACE;

    if ( 0 == channelLayout )
        return "default";
    if ( channelLayout == CAudioFormat::GetDefaultChannelLayout( 1 ) )
        return "mono";
    if ( channelLayout == CAudioFormat::GetDefaultChannelLayout( 2 ) )
        return "stereo";
    if ( channelLayout == CAudioFormat::GetDefaultChannelLayout( 4 ) )
        return "quad";
    if ( channelLayout == CAudioFormat::GetDefaultChannelLayout( 6 ) )
        return "5.1";
    if ( channelLayout == CAudioFormat::GetDefaultChannelLayout( 8 ) )
        return "7.1";
    return CORE::UInt32ToString( channelLayout );
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioSettings::StringToChannelLayout( const CString& text, UInt32 defaultLayout )
{GUCEF_TRACE;

    CString name = text.Lowercase().Trim( true ).Trim( false );
    if ( "default" == name )
        return 0;
    if ( "mono" == name )
        return CAudioFormat::GetDefaultChannelLayout( 1 );
    if ( "stereo" == name )
        return CAudioFormat::GetDefaultChannelLayout( 2 );
    if ( "quad" == name )
        return CAudioFormat::GetDefaultChannelLayout( 4 );
    if ( "5.1" == name )
        return CAudioFormat::GetDefaultChannelLayout( 6 );
    if ( "7.1" == name )
        return CAudioFormat::GetDefaultChannelLayout( 8 );

    // a mask as a number: digits only
    if ( name.IsNULLOrEmpty() || name.Length() > 10 )
        return defaultLayout;
    UInt64 mask = 0;
    for ( UInt32 i=0; i<name.Length(); ++i )
    {
        UInt32 c = (UInt32) name[ i ];
        if ( c < (UInt32) '0' || c > (UInt32) '9' )
            return defaultLayout;
        mask = mask * 10 + (UInt64) ( c - (UInt32) '0' );
    }
    if ( mask > 0xFFFFFFFFULL )
        return defaultLayout;
    return (UInt32) mask;
}

/*-------------------------------------------------------------------------*/

const char*
CAudioSettings::LimiterModeToString( TAudioLimiterMode mode )
{GUCEF_TRACE;

    switch ( mode )
    {
        case AUDIOLIMITERMODE_OFF: return "off";
        case AUDIOLIMITERMODE_LOOKAHEAD: return "lookahead";
        case AUDIOLIMITERMODE_INSTANT:
        default: return "instant";
    }
}

/*-------------------------------------------------------------------------*/

bool
CAudioSettings::StringToLimiterMode( const CString& text, TAudioLimiterMode& mode )
{GUCEF_TRACE;

    CString name = text.Lowercase();
    if ( "off" == name )
        mode = AUDIOLIMITERMODE_OFF;
    else
    if ( "instant" == name )
        mode = AUDIOLIMITERMODE_INSTANT;
    else
    if ( "lookahead" == name )
        mode = AUDIOLIMITERMODE_LOOKAHEAD;
    else
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioSettings::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    // replaces the section that was there
    tree.DelChild( ConfigSectionName );
    CORE::CDataNode* section = tree.AddChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    section->SetAttribute( "driver", m_driverName );
    section->SetAttribute( "device", m_deviceId );
    section->SetAttribute( "captureDevice", m_captureDeviceId );
    section->SetAttribute( "sampleRate", m_sampleRate );
    section->SetAttribute( "bufferFrames", m_bufferSizeInFrames );
    section->SetAttribute( "speakerLayout", ChannelLayoutToString( m_channelLayout ) );
    if ( m_hasLimiterMode )
        section->SetAttribute( "limiterMode", CString( LimiterModeToString( m_limiterMode ) ) );
    section->SetAttribute( "latencyProfile", CString( CAudioEngineParams::LatencyProfileToString( m_latencyProfile ) ) );

    CORE::CDataNode* master = section->AddChild( "Master" );
    if ( GUCEF_NULL != master )
    {
        master->SetAttribute( "volumeDb", m_master.volumeInDecibels );
        master->SetAttribute( "muted", m_master.muted );
    }

    TBusSettingsMap::const_iterator i = m_buses.begin();
    while ( i != m_buses.end() )
    {
        CORE::CDataNode* bus = section->AddChild( "Bus" );
        if ( GUCEF_NULL != bus )
        {
            bus->SetAttribute( "name", (*i).first );
            bus->SetAttribute( "volumeDb", (*i).second.volumeInDecibels );
            bus->SetAttribute( "muted", (*i).second.muted );
        }
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioSettings::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &treeroot;
    if ( treeroot.GetName() != ConfigSectionName )
        section = treeroot.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    m_driverName = section->GetAttributeValueOrChildValueByName( "driver" ).AsString( m_driverName, true );
    m_deviceId = section->GetAttributeValueOrChildValueByName( "device" ).AsString( m_deviceId, true );
    m_captureDeviceId = section->GetAttributeValueOrChildValueByName( "captureDevice" ).AsString( m_captureDeviceId, true );
    m_sampleRate = section->GetAttributeValueOrChildValueByName( "sampleRate" ).AsUInt32( m_sampleRate, true );
    m_bufferSizeInFrames = section->GetAttributeValueOrChildValueByName( "bufferFrames" ).AsUInt32( m_bufferSizeInFrames, true );

    CString layout = section->GetAttributeValueOrChildValueByName( "speakerLayout" ).AsString( CString::Empty, true );
    if ( !layout.IsNULLOrEmpty() )
    {
        const UInt32 unknown = 0xFFFFFFFFU;
        UInt32 parsed = StringToChannelLayout( layout, unknown );
        if ( unknown == parsed )
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AudioSettings: Ignoring the unknown speaker layout \"" << layout << "\"" );
        }
        else
        {
            m_channelLayout = parsed;
        }
    }

    CString limiter = section->GetAttributeValueOrChildValueByName( "limiterMode" ).AsString( CString::Empty, true );
    if ( !limiter.IsNULLOrEmpty() )
    {
        TAudioLimiterMode mode = m_limiterMode;
        if ( StringToLimiterMode( limiter, mode ) )
        {
            SetLimiterMode( mode );
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AudioSettings: Ignoring the unknown limiter mode \"" << limiter << "\"" );
        }
    }

    CString profile = section->GetAttributeValueOrChildValueByName( "latencyProfile" ).AsString( CString::Empty, true );
    if ( !profile.IsNULLOrEmpty() )
    {
        TAudioLatencyProfile parsed = m_latencyProfile;
        if ( CAudioEngineParams::StringToLatencyProfile( profile, parsed ) )
        {
            m_latencyProfile = parsed;
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AudioSettings: Ignoring the unknown latency profile \"" << profile << "\"" );
        }
    }

    const CORE::CDataNode* master = section->FindChild( "Master" );
    if ( GUCEF_NULL != master )
    {
        SetMasterVolumeInDecibels( master->GetAttributeValueOrChildValueByName( "volumeDb" ).AsFloat32( m_master.volumeInDecibels, true ) );
        m_master.muted = master->GetAttributeValueOrChildValueByName( "muted" ).AsBool( m_master.muted, true );
    }

    CORE::CDataNode::const_iterator i = section->ConstBegin();
    while ( i != section->ConstEnd() )
    {
        const CORE::CDataNode* node = (*i);
        ++i;
        if ( node->GetName() != "Bus" )
            continue;
        CString name = node->GetAttributeValueOrChildValueByName( "name" ).AsString( CString::Empty, true );
        if ( name.IsNULLOrEmpty() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AudioSettings: Ignoring a bus entry without a name" );
            continue;
        }
        SBusSettings bus;
        GetBusSettings( name, bus );
        SetBusVolumeInDecibels( name, node->GetAttributeValueOrChildValueByName( "volumeDb" ).AsFloat32( bus.volumeInDecibels, true ) );
        SetBusMuted( name, node->GetAttributeValueOrChildValueByName( "muted" ).AsBool( bus.muted, true ) );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::AUDIO::CAudioSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
