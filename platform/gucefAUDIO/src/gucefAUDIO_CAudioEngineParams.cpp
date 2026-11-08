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

#include "gucefAUDIO_CAudioEngineParams.h"

#ifndef GUCEF_MT_CREALTIMESCHEDULINGSCOPE_H
#include "gucefMT_CRealtimeSchedulingScope.h"
#define GUCEF_MT_CREALTIMESCHEDULINGSCOPE_H
#endif /* GUCEF_MT_CREALTIMESCHEDULINGSCOPE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOPUSHDEVICE_H
#include "gucefAUDIO_CAudioPushDevice.h"
#define GUCEF_AUDIO_CAUDIOPUSHDEVICE_H
#endif /* GUCEF_AUDIO_CAUDIOPUSHDEVICE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUDIO_CAUDIOSETTINGS_H
#include "gucefAUDIO_CAudioSettings.h"
#define GUCEF_AUDIO_CAUDIOSETTINGS_H
#endif /* GUCEF_AUDIO_CAUDIOSETTINGS_H ? */

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

/** Reads a text value when the section has it, an empty text included: saved as empty it has to load as empty */
void
LoadText( const CORE::CDataNode& section, const char* name, CString& target )
{
    const CORE::CVariant value = section.GetAttributeValueOrChildValueByName( name );
    if ( value.IsInitialized() && !value.IsNULL() )
        target = value.AsString( CString::Empty, true );
}

/** The latency profile values of one device: the period, the number of periods (0 for the driver's choice) and the real-time priority */
void
ApplyLatencyToDevice( CAudioDeviceParams& deviceParams, UInt32 periodInFrames, UInt32 periods, UInt32 realtimePriority )
{
    // Set() adds a value to the ones a key has, so the key goes first: the options end up with exactly these values
    deviceParams.SetBufferSizeInFrames( periodInFrames );
    deviceParams.GetOptions().Delete( CAudioEngineParams::PeriodsOptionName );
    if ( periods > 0 )
        deviceParams.GetOptions().Set( CAudioEngineParams::PeriodsOptionName, CORE::UInt32ToString( periods ) );
    deviceParams.GetOptions().Delete( CAudioPushDevice::RealtimePriorityOptionName );
    deviceParams.GetOptions().Set( CAudioPushDevice::RealtimePriorityOptionName, CORE::UInt32ToString( realtimePriority ) );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CAudioEngineParams::ConfigSectionName = "AudioEngine";
const CString CAudioEngineParams::CaptureDeviceSectionName = "CaptureDevice";
const CString CAudioEngineParams::PeriodsOptionName = "periods";

/*-------------------------------------------------------------------------*/

CAudioEngineParams::CAudioEngineParams( void )
    : CORE::CIConfigurable()
    , m_driverName()
    , m_deviceParams()
    , m_maxVoices( 128 )
    , m_maxBuses( 32 )
    , m_maxBlockFrames( 4096 )
    , m_commandQueueSize( 1024 )
    , m_eventQueueSize( 2048 )
    , m_threadingMode( AUDIOTHREADING_AUTO )
    , m_updateIntervalInMs( 5 )
    , m_threadPoolName( "AudioEngine" )
    , m_startDevice( true )
    , m_panLaw( AUDIOPANLAW_CONSTANT_POWER )
    , m_limiterEnabled( true )
    , m_limiterOnMode( AUDIOLIMITERMODE_LOOKAHEAD )
    , m_limiterCeilingInDecibels( -0.3f )
    , m_limiterLookaheadInMs( 1.5f )
    , m_limiterReleaseInMs( 50.0f )
    , m_maxEffectsPerBus( 8 )
    , m_maxSendsPerBus( 4 )
    , m_maxVoiceEffects( 4 )
    , m_maxVoiceSends( 2 )
    , m_captureEnabled( false )
    , m_captureDeviceParams()
    , m_captureProcessingRate( 0 )
    , m_captureChannel( -1 )
    , m_maxCaptureConsumers( 4 )
    , m_maxCaptureEffects( 8 )
    , m_captureCommandQueueSize( 64 )
    , m_voiceActivitySettings()
{GUCEF_TRACE;

    m_captureDeviceParams.SetDirection( AUDIODIRECTION_CAPTURE );
}

/*-------------------------------------------------------------------------*/

CAudioEngineParams::CAudioEngineParams( const CAudioEngineParams& src )
    : CORE::CIConfigurable( src )
    , m_driverName( src.m_driverName )
    , m_deviceParams( src.m_deviceParams )
    , m_maxVoices( src.m_maxVoices )
    , m_maxBuses( src.m_maxBuses )
    , m_maxBlockFrames( src.m_maxBlockFrames )
    , m_commandQueueSize( src.m_commandQueueSize )
    , m_eventQueueSize( src.m_eventQueueSize )
    , m_threadingMode( src.m_threadingMode )
    , m_updateIntervalInMs( src.m_updateIntervalInMs )
    , m_threadPoolName( src.m_threadPoolName )
    , m_startDevice( src.m_startDevice )
    , m_panLaw( src.m_panLaw )
    , m_limiterEnabled( src.m_limiterEnabled )
    , m_limiterOnMode( src.m_limiterOnMode )
    , m_limiterCeilingInDecibels( src.m_limiterCeilingInDecibels )
    , m_limiterLookaheadInMs( src.m_limiterLookaheadInMs )
    , m_limiterReleaseInMs( src.m_limiterReleaseInMs )
    , m_maxEffectsPerBus( src.m_maxEffectsPerBus )
    , m_maxSendsPerBus( src.m_maxSendsPerBus )
    , m_maxVoiceEffects( src.m_maxVoiceEffects )
    , m_maxVoiceSends( src.m_maxVoiceSends )
    , m_captureEnabled( src.m_captureEnabled )
    , m_captureDeviceParams( src.m_captureDeviceParams )
    , m_captureProcessingRate( src.m_captureProcessingRate )
    , m_captureChannel( src.m_captureChannel )
    , m_maxCaptureConsumers( src.m_maxCaptureConsumers )
    , m_maxCaptureEffects( src.m_maxCaptureEffects )
    , m_captureCommandQueueSize( src.m_captureCommandQueueSize )
    , m_voiceActivitySettings( src.m_voiceActivitySettings )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioEngineParams&
CAudioEngineParams::operator=( const CAudioEngineParams& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_driverName = src.m_driverName;
        m_deviceParams = src.m_deviceParams;
        m_maxVoices = src.m_maxVoices;
        m_maxBuses = src.m_maxBuses;
        m_maxBlockFrames = src.m_maxBlockFrames;
        m_commandQueueSize = src.m_commandQueueSize;
        m_eventQueueSize = src.m_eventQueueSize;
        m_threadingMode = src.m_threadingMode;
        m_updateIntervalInMs = src.m_updateIntervalInMs;
        m_threadPoolName = src.m_threadPoolName;
        m_startDevice = src.m_startDevice;
        m_panLaw = src.m_panLaw;
        m_limiterEnabled = src.m_limiterEnabled;
        m_limiterOnMode = src.m_limiterOnMode;
        m_limiterCeilingInDecibels = src.m_limiterCeilingInDecibels;
        m_limiterLookaheadInMs = src.m_limiterLookaheadInMs;
        m_limiterReleaseInMs = src.m_limiterReleaseInMs;
        m_maxEffectsPerBus = src.m_maxEffectsPerBus;
        m_maxSendsPerBus = src.m_maxSendsPerBus;
        m_maxVoiceEffects = src.m_maxVoiceEffects;
        m_maxVoiceSends = src.m_maxVoiceSends;
        m_captureEnabled = src.m_captureEnabled;
        m_captureDeviceParams = src.m_captureDeviceParams;
        m_captureProcessingRate = src.m_captureProcessingRate;
        m_captureChannel = src.m_captureChannel;
        m_maxCaptureConsumers = src.m_maxCaptureConsumers;
        m_maxCaptureEffects = src.m_maxCaptureEffects;
        m_captureCommandQueueSize = src.m_captureCommandQueueSize;
        m_voiceActivitySettings = src.m_voiceActivitySettings;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CAudioEngineParams::~CAudioEngineParams()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CAudioEngineParams::GetDriverName( void ) const
{GUCEF_TRACE;

    return m_driverName;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetDriverName( const CString& driverName )
{GUCEF_TRACE;

    m_driverName = driverName;
}

/*-------------------------------------------------------------------------*/

const CAudioDeviceParams&
CAudioEngineParams::GetDeviceParams( void ) const
{GUCEF_TRACE;

    return m_deviceParams;
}

/*-------------------------------------------------------------------------*/

CAudioDeviceParams&
CAudioEngineParams::GetDeviceParams( void )
{GUCEF_TRACE;

    return m_deviceParams;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEngineParams::GetMaxVoices( void ) const
{GUCEF_TRACE;

    return m_maxVoices;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetMaxVoices( UInt32 maxVoices )
{GUCEF_TRACE;

    m_maxVoices = maxVoices;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEngineParams::GetMaxBuses( void ) const
{GUCEF_TRACE;

    return m_maxBuses;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetMaxBuses( UInt32 maxBuses )
{GUCEF_TRACE;

    m_maxBuses = maxBuses;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEngineParams::GetMaxBlockFrames( void ) const
{GUCEF_TRACE;

    return m_maxBlockFrames;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetMaxBlockFrames( UInt32 frames )
{GUCEF_TRACE;

    m_maxBlockFrames = frames;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEngineParams::GetCommandQueueSize( void ) const
{GUCEF_TRACE;

    return m_commandQueueSize;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetCommandQueueSize( UInt32 size )
{GUCEF_TRACE;

    m_commandQueueSize = size;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEngineParams::GetEventQueueSize( void ) const
{GUCEF_TRACE;

    return m_eventQueueSize;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetEventQueueSize( UInt32 size )
{GUCEF_TRACE;

    m_eventQueueSize = size;
}

/*-------------------------------------------------------------------------*/

TAudioThreadingMode
CAudioEngineParams::GetThreadingMode( void ) const
{GUCEF_TRACE;

    return m_threadingMode;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetThreadingMode( TAudioThreadingMode mode )
{GUCEF_TRACE;

    m_threadingMode = mode;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEngineParams::GetUpdateIntervalInMs( void ) const
{GUCEF_TRACE;

    return m_updateIntervalInMs;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetUpdateIntervalInMs( UInt32 milliseconds )
{GUCEF_TRACE;

    m_updateIntervalInMs = milliseconds;
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioEngineParams::GetThreadPoolName( void ) const
{GUCEF_TRACE;

    return m_threadPoolName;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetThreadPoolName( const CString& name )
{GUCEF_TRACE;

    m_threadPoolName = name;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngineParams::GetStartDevice( void ) const
{GUCEF_TRACE;

    return m_startDevice;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetStartDevice( bool start )
{GUCEF_TRACE;

    m_startDevice = start;
}

/*-------------------------------------------------------------------------*/

TAudioPanLaw
CAudioEngineParams::GetPanLaw( void ) const
{GUCEF_TRACE;

    return m_panLaw;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetPanLaw( TAudioPanLaw panLaw )
{GUCEF_TRACE;

    m_panLaw = panLaw;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngineParams::GetLimiterEnabled( void ) const
{GUCEF_TRACE;

    return m_limiterEnabled;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetLimiterEnabled( bool enabled )
{GUCEF_TRACE;

    m_limiterEnabled = enabled;
}

/*-------------------------------------------------------------------------*/

TAudioLimiterMode
CAudioEngineParams::GetLimiterMode( void ) const
{GUCEF_TRACE;

    return m_limiterEnabled ? m_limiterOnMode : AUDIOLIMITERMODE_OFF;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetLimiterMode( TAudioLimiterMode mode )
{GUCEF_TRACE;

    if ( AUDIOLIMITERMODE_INSTANT == mode || AUDIOLIMITERMODE_LOOKAHEAD == mode )
    {
        m_limiterOnMode = mode;
        m_limiterEnabled = true;
    }
    else
        m_limiterEnabled = false;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioEngineParams::GetLimiterCeilingInDecibels( void ) const
{GUCEF_TRACE;

    return m_limiterCeilingInDecibels;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetLimiterCeilingInDecibels( Float32 decibels )
{GUCEF_TRACE;

    m_limiterCeilingInDecibels = decibels;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioEngineParams::GetLimiterLookaheadInMs( void ) const
{GUCEF_TRACE;

    return m_limiterLookaheadInMs;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetLimiterLookaheadInMs( Float32 milliseconds )
{GUCEF_TRACE;

    m_limiterLookaheadInMs = milliseconds;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioEngineParams::GetLimiterReleaseInMs( void ) const
{GUCEF_TRACE;

    return m_limiterReleaseInMs;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetLimiterReleaseInMs( Float32 milliseconds )
{GUCEF_TRACE;

    m_limiterReleaseInMs = milliseconds;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEngineParams::GetMaxEffectsPerBus( void ) const
{GUCEF_TRACE;

    return m_maxEffectsPerBus;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetMaxEffectsPerBus( UInt32 count )
{GUCEF_TRACE;

    m_maxEffectsPerBus = count;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEngineParams::GetMaxSendsPerBus( void ) const
{GUCEF_TRACE;

    return m_maxSendsPerBus;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetMaxSendsPerBus( UInt32 count )
{GUCEF_TRACE;

    m_maxSendsPerBus = count;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEngineParams::GetMaxVoiceEffects( void ) const
{GUCEF_TRACE;

    return m_maxVoiceEffects;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetMaxVoiceEffects( UInt32 count )
{GUCEF_TRACE;

    m_maxVoiceEffects = count;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEngineParams::GetMaxVoiceSends( void ) const
{GUCEF_TRACE;

    return m_maxVoiceSends;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetMaxVoiceSends( UInt32 count )
{GUCEF_TRACE;

    m_maxVoiceSends = count;
}


/*-------------------------------------------------------------------------*/

const char*
CAudioEngineParams::ThreadingModeToString( TAudioThreadingMode mode )
{GUCEF_TRACE;

    switch ( mode )
    {
        case AUDIOTHREADING_THREADED    : return "threaded";
        case AUDIOTHREADING_COOPERATIVE : return "cooperative";
        default                         : return "auto";
    }
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngineParams::StringToThreadingMode( const CString& text, TAudioThreadingMode& mode )
{GUCEF_TRACE;

    const CString name = text.Lowercase();
    if ( name == "auto" )           { mode = AUDIOTHREADING_AUTO; return true; }
    if ( name == "threaded" )       { mode = AUDIOTHREADING_THREADED; return true; }
    if ( name == "cooperative" )    { mode = AUDIOTHREADING_COOPERATIVE; return true; }
    return false;
}

/*-------------------------------------------------------------------------*/

const char*
CAudioEngineParams::PanLawToString( TAudioPanLaw panLaw )
{GUCEF_TRACE;

    return AUDIOPANLAW_BALANCE == panLaw ? "balance" : "constantPower";
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngineParams::StringToPanLaw( const CString& text, TAudioPanLaw& panLaw )
{GUCEF_TRACE;

    const CString name = text.Lowercase();
    if ( name == "constantpower" )  { panLaw = AUDIOPANLAW_CONSTANT_POWER; return true; }
    if ( name == "balance" )        { panLaw = AUDIOPANLAW_BALANCE; return true; }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngineParams::GetCaptureEnabled( void ) const
{GUCEF_TRACE;

    return m_captureEnabled;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetCaptureEnabled( bool enabled )
{GUCEF_TRACE;

    m_captureEnabled = enabled;
}

/*-------------------------------------------------------------------------*/

const CAudioDeviceParams&
CAudioEngineParams::GetCaptureDeviceParams( void ) const
{GUCEF_TRACE;

    return m_captureDeviceParams;
}

/*-------------------------------------------------------------------------*/

CAudioDeviceParams&
CAudioEngineParams::GetCaptureDeviceParams( void )
{GUCEF_TRACE;

    return m_captureDeviceParams;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEngineParams::GetCaptureProcessingRate( void ) const
{GUCEF_TRACE;

    return m_captureProcessingRate;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetCaptureProcessingRate( UInt32 sampleRate )
{GUCEF_TRACE;

    m_captureProcessingRate = sampleRate;
}

/*-------------------------------------------------------------------------*/

Int32
CAudioEngineParams::GetCaptureChannel( void ) const
{GUCEF_TRACE;

    return m_captureChannel;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetCaptureChannel( Int32 channel )
{GUCEF_TRACE;

    m_captureChannel = channel;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEngineParams::GetMaxCaptureConsumers( void ) const
{GUCEF_TRACE;

    return m_maxCaptureConsumers;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetMaxCaptureConsumers( UInt32 count )
{GUCEF_TRACE;

    m_maxCaptureConsumers = count;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEngineParams::GetMaxCaptureEffects( void ) const
{GUCEF_TRACE;

    return m_maxCaptureEffects;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetMaxCaptureEffects( UInt32 count )
{GUCEF_TRACE;

    m_maxCaptureEffects = count;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEngineParams::GetCaptureCommandQueueSize( void ) const
{GUCEF_TRACE;

    return m_captureCommandQueueSize;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::SetCaptureCommandQueueSize( UInt32 size )
{GUCEF_TRACE;

    m_captureCommandQueueSize = size;
}

/*-------------------------------------------------------------------------*/

const CVoiceActivitySettings&
CAudioEngineParams::GetVoiceActivitySettings( void ) const
{GUCEF_TRACE;

    return m_voiceActivitySettings;
}

/*-------------------------------------------------------------------------*/

CVoiceActivitySettings&
CAudioEngineParams::GetVoiceActivitySettings( void )
{GUCEF_TRACE;

    return m_voiceActivitySettings;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngineParams::ApplyLatencyProfile( TAudioLatencyProfile profile, UInt32 sampleRate )
{GUCEF_TRACE;

    UInt32 periodInFrames = 0;
    UInt32 periods = 0;
    UInt32 realtimePriority = 0;
    TAudioLimiterMode limiterMode = AUDIOLIMITERMODE_LOOKAHEAD;

    if ( AUDIOLATENCYPROFILE_LOW == profile )
    {
        // 5 ms, rounded to a whole frame
        if ( 0 == sampleRate )
            sampleRate = 48000;
        periodInFrames = (UInt32) ( ( (UInt64) sampleRate * 5 + 500 ) / 1000 );
        if ( 0 == periodInFrames )
            periodInFrames = 1;
        periods = 3;
        realtimePriority = MT::CRealtimeSchedulingScope::DefaultPriority;
        limiterMode = AUDIOLIMITERMODE_INSTANT;
    }

    ApplyLatencyToDevice( m_deviceParams, periodInFrames, periods, realtimePriority );
    ApplyLatencyToDevice( m_captureDeviceParams, periodInFrames, periods, realtimePriority );

    // the mode the limiter works in when it is on: a limiter that was off stays off
    const bool limiterWasEnabled = m_limiterEnabled;
    SetLimiterMode( limiterMode );
    if ( !limiterWasEnabled )
        SetLimiterEnabled( false );
}

/*-------------------------------------------------------------------------*/

const char*
CAudioEngineParams::LatencyProfileToString( TAudioLatencyProfile profile )
{GUCEF_TRACE;

    switch ( profile )
    {
        case AUDIOLATENCYPROFILE_LOW: return "low";
        case AUDIOLATENCYPROFILE_DEFAULT:
        default: return "default";
    }
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngineParams::StringToLatencyProfile( const CString& text, TAudioLatencyProfile& profile )
{GUCEF_TRACE;

    CString name = text.Lowercase();
    if ( "default" == name )
        profile = AUDIOLATENCYPROFILE_DEFAULT;
    else
    if ( "low" == name )
        profile = AUDIOLATENCYPROFILE_LOW;
    else
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngineParams::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = tree.Structure( ConfigSectionName, '/' );
    if ( GUCEF_NULL == section )
        return false;

    section->SetAttribute( "driver", m_driverName );
    section->SetAttribute( "maxVoices", m_maxVoices );
    section->SetAttribute( "maxBuses", m_maxBuses );
    section->SetAttribute( "maxBlockFrames", m_maxBlockFrames );
    section->SetAttribute( "commandQueueSize", m_commandQueueSize );
    section->SetAttribute( "eventQueueSize", m_eventQueueSize );
    section->SetAttribute( "threadingMode", CString( ThreadingModeToString( m_threadingMode ) ) );
    section->SetAttribute( "updateIntervalMs", m_updateIntervalInMs );
    section->SetAttribute( "threadPool", m_threadPoolName );
    section->SetAttribute( "startDevice", m_startDevice );
    section->SetAttribute( "panLaw", CString( PanLawToString( m_panLaw ) ) );
    section->SetAttribute( "limiterEnabled", m_limiterEnabled );
    section->SetAttribute( "limiterMode", CString( CAudioSettings::LimiterModeToString( m_limiterOnMode ) ) );
    section->SetAttribute( "limiterCeilingDb", m_limiterCeilingInDecibels );
    section->SetAttribute( "limiterLookaheadMs", m_limiterLookaheadInMs );
    section->SetAttribute( "limiterReleaseMs", m_limiterReleaseInMs );
    section->SetAttribute( "maxEffectsPerBus", m_maxEffectsPerBus );
    section->SetAttribute( "maxSendsPerBus", m_maxSendsPerBus );
    section->SetAttribute( "maxVoiceEffects", m_maxVoiceEffects );
    section->SetAttribute( "maxVoiceSends", m_maxVoiceSends );
    section->SetAttribute( "captureEnabled", m_captureEnabled );
    section->SetAttribute( "captureProcessingRate", m_captureProcessingRate );
    section->SetAttribute( "captureChannel", m_captureChannel );
    section->SetAttribute( "maxCaptureConsumers", m_maxCaptureConsumers );
    section->SetAttribute( "maxCaptureEffects", m_maxCaptureEffects );
    section->SetAttribute( "captureCommandQueueSize", m_captureCommandQueueSize );

    CORE::CDataNode* capture = section->Structure( CaptureDeviceSectionName, '/' );
    if ( GUCEF_NULL == capture || !m_captureDeviceParams.SaveConfig( *capture ) )
        return false;

    // the settings of the voice activity detection are only written when they are not the defaults
    section->DelChild( CVoiceActivitySettings::ConfigSectionName );
    if ( m_voiceActivitySettings != CVoiceActivitySettings() && !m_voiceActivitySettings.SaveConfig( *section ) )
        return false;
    return m_deviceParams.SaveConfig( *section );
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngineParams::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &treeroot;
    if ( treeroot.GetName() != ConfigSectionName )
        section = treeroot.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    LoadText( *section, "driver", m_driverName );
    m_maxVoices = section->GetAttributeValueOrChildValueByName( "maxVoices" ).AsUInt32( m_maxVoices, true );
    m_maxBuses = section->GetAttributeValueOrChildValueByName( "maxBuses" ).AsUInt32( m_maxBuses, true );
    m_maxBlockFrames = section->GetAttributeValueOrChildValueByName( "maxBlockFrames" ).AsUInt32( m_maxBlockFrames, true );
    m_commandQueueSize = section->GetAttributeValueOrChildValueByName( "commandQueueSize" ).AsUInt32( m_commandQueueSize, true );
    m_eventQueueSize = section->GetAttributeValueOrChildValueByName( "eventQueueSize" ).AsUInt32( m_eventQueueSize, true );
    m_updateIntervalInMs = section->GetAttributeValueOrChildValueByName( "updateIntervalMs" ).AsUInt32( m_updateIntervalInMs, true );
    LoadText( *section, "threadPool", m_threadPoolName );
    m_startDevice = section->GetAttributeValueOrChildValueByName( "startDevice" ).AsBool( m_startDevice, true );
    m_limiterCeilingInDecibels = section->GetAttributeValueOrChildValueByName( "limiterCeilingDb" ).AsFloat32( m_limiterCeilingInDecibels, true );
    m_limiterLookaheadInMs = section->GetAttributeValueOrChildValueByName( "limiterLookaheadMs" ).AsFloat32( m_limiterLookaheadInMs, true );
    m_limiterReleaseInMs = section->GetAttributeValueOrChildValueByName( "limiterReleaseMs" ).AsFloat32( m_limiterReleaseInMs, true );
    m_maxEffectsPerBus = section->GetAttributeValueOrChildValueByName( "maxEffectsPerBus" ).AsUInt32( m_maxEffectsPerBus, true );
    m_maxSendsPerBus = section->GetAttributeValueOrChildValueByName( "maxSendsPerBus" ).AsUInt32( m_maxSendsPerBus, true );
    m_maxVoiceEffects = section->GetAttributeValueOrChildValueByName( "maxVoiceEffects" ).AsUInt32( m_maxVoiceEffects, true );
    m_maxVoiceSends = section->GetAttributeValueOrChildValueByName( "maxVoiceSends" ).AsUInt32( m_maxVoiceSends, true );
    m_captureEnabled = section->GetAttributeValueOrChildValueByName( "captureEnabled" ).AsBool( m_captureEnabled, true );
    m_captureProcessingRate = section->GetAttributeValueOrChildValueByName( "captureProcessingRate" ).AsUInt32( m_captureProcessingRate, true );
    m_captureChannel = section->GetAttributeValueOrChildValueByName( "captureChannel" ).AsInt32( m_captureChannel, true );
    m_maxCaptureConsumers = section->GetAttributeValueOrChildValueByName( "maxCaptureConsumers" ).AsUInt32( m_maxCaptureConsumers, true );
    m_maxCaptureEffects = section->GetAttributeValueOrChildValueByName( "maxCaptureEffects" ).AsUInt32( m_maxCaptureEffects, true );
    m_captureCommandQueueSize = section->GetAttributeValueOrChildValueByName( "captureCommandQueueSize" ).AsUInt32( m_captureCommandQueueSize, true );

    const CString threading = section->GetAttributeValueOrChildValueByName( "threadingMode" ).AsString( CString::Empty, true );
    if ( !threading.IsNULLOrEmpty() && !StringToThreadingMode( threading, m_threadingMode ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AudioEngineParams: Ignoring the unknown threading mode \"" << threading << "\"" );
    }

    const CString panLaw = section->GetAttributeValueOrChildValueByName( "panLaw" ).AsString( CString::Empty, true );
    if ( !panLaw.IsNULLOrEmpty() && !StringToPanLaw( panLaw, m_panLaw ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AudioEngineParams: Ignoring the unknown pan law \"" << panLaw << "\"" );
    }

    const CString limiter = section->GetAttributeValueOrChildValueByName( "limiterMode" ).AsString( CString::Empty, true );
    if ( !limiter.IsNULLOrEmpty() )
    {
        TAudioLimiterMode mode = AUDIOLIMITERMODE_OFF;
        if ( CAudioSettings::StringToLimiterMode( limiter, mode ) )
            SetLimiterMode( mode );
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AudioEngineParams: Ignoring the unknown limiter mode \"" << limiter << "\"" );
        }
    }
    m_limiterEnabled = section->GetAttributeValueOrChildValueByName( "limiterEnabled" ).AsBool( m_limiterEnabled, true );

    m_deviceParams.LoadConfig( *section );
    const CORE::CDataNode* capture = section->FindChild( CaptureDeviceSectionName );
    if ( GUCEF_NULL != capture )
        m_captureDeviceParams.LoadConfig( *capture );
    m_voiceActivitySettings.LoadConfig( *section );
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioEngineParams::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::AUDIO::CAudioEngineParams";
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
