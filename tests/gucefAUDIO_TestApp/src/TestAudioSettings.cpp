/*
 *  gucefAUDIO_TestApp: GUCEF test application for the AUDIO module
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

#ifndef GUCEFAUDIO_H
#include "gucefAUDIO.h"
#define GUCEFAUDIO_H
#endif /* GUCEFAUDIO_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
Near( Float32 a, Float32 b, Float32 tolerance = 1e-4f )
{
    return fabsf( a - b ) <= tolerance;
}

/** Settings with every value away from its default */
AUDIO::CAudioSettings
MakeChangedSettings( void )
{
    AUDIO::CAudioSettings settings;
    settings.SetDriverName( "alsa" );
    settings.SetDeviceId( "hw:1,0" );
    settings.SetCaptureDeviceId( "hw:2,0" );
    settings.SetLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_LOW );
    settings.SetSampleRate( 44100 );
    settings.SetBufferSizeInFrames( 256 );
    settings.SetChannelLayout( AUDIO::CAudioFormat::GetDefaultChannelLayout( 6 ) );
    settings.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_OFF );
    settings.SetMasterVolumeInDecibels( -6.0f );
    settings.SetMasterMuted( true );
    settings.SetBusVolumeInDecibels( "music", -9.0f );
    settings.SetBusMuted( "music", false );
    settings.SetBusVolumeInDecibels( "effects", -3.0f );
    settings.SetBusMuted( "effects", true );
    return settings;
}

bool
SameSettings( const AUDIO::CAudioSettings& a, const AUDIO::CAudioSettings& b )
{
    if ( a.GetDriverName() != b.GetDriverName() || a.GetDeviceId() != b.GetDeviceId() ||
         a.GetCaptureDeviceId() != b.GetCaptureDeviceId() || a.GetLatencyProfile() != b.GetLatencyProfile() ||
         a.GetSampleRate() != b.GetSampleRate() || a.GetBufferSizeInFrames() != b.GetBufferSizeInFrames() ||
         a.GetChannelLayout() != b.GetChannelLayout() || a.HasLimiterMode() != b.HasLimiterMode() ||
         ( a.HasLimiterMode() && a.GetLimiterMode() != b.GetLimiterMode() ) ||
         !Near( a.GetMasterVolumeInDecibels(), b.GetMasterVolumeInDecibels() ) || a.IsMasterMuted() != b.IsMasterMuted() )
    {
        return false;
    }
    const AUDIO::CAudioSettings::TBusSettingsMap& busesA = a.GetAllBusSettings();
    const AUDIO::CAudioSettings::TBusSettingsMap& busesB = b.GetAllBusSettings();
    if ( busesA.size() != busesB.size() )
        return false;
    AUDIO::CAudioSettings::TBusSettingsMap::const_iterator i = busesA.begin();
    while ( i != busesA.end() )
    {
        AUDIO::CAudioSettings::SBusSettings other;
        if ( !b.GetBusSettings( (*i).first, other ) ||
             !Near( other.volumeInDecibels, (*i).second.volumeInDecibels ) || other.muted != (*i).second.muted )
        {
            return false;
        }
        ++i;
    }
    return true;
}

}

/*-------------------------------------------------------------------------*/

void
PerformAudioSettingsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Settings" );

    GUCEF_TESTFW_TESTCASE( "AudioSettings: The defaults leave everything to the engine and are full volume" )
    try
    {
        AUDIO::CAudioSettings settings;
        ASSERT_TRUE( settings.GetDriverName().IsNULLOrEmpty() && settings.GetDeviceId().IsNULLOrEmpty() );
        ASSERT_TRUE( 0 == settings.GetSampleRate() && 0 == settings.GetBufferSizeInFrames() && 0 == settings.GetChannelLayout() );
        ASSERT_FALSE( settings.HasLimiterMode() );
        ASSERT_TRUE( settings.GetCaptureDeviceId().IsNULLOrEmpty() && AUDIO::AUDIOLATENCYPROFILE_DEFAULT == settings.GetLatencyProfile() );
        ASSERT_TRUE( Near( 0.0f, settings.GetMasterVolumeInDecibels() ) && !settings.IsMasterMuted() );
        ASSERT_TRUE( settings.GetAllBusSettings().empty() );
        ASSERT_TRUE( "AudioSettings" == AUDIO::CAudioSettings::ConfigSectionName );

        // the limiter mode can be set and taken back
        settings.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD );
        ASSERT_TRUE( settings.HasLimiterMode() && AUDIO::AUDIOLIMITERMODE_LOOKAHEAD == settings.GetLimiterMode() );
        settings.ClearLimiterMode();
        ASSERT_FALSE( settings.HasLimiterMode() );

        // no gain above 0 dB, and not a number is silence
        settings.SetMasterVolumeInDecibels( 12.0f );
        ASSERT_TRUE( Near( 0.0f, settings.GetMasterVolumeInDecibels() ) );
        settings.SetBusVolumeInDecibels( "music", -1000.0f );
        AUDIO::CAudioSettings::SBusSettings bus;
        ASSERT_TRUE( settings.GetBusSettings( "music", bus ) && Near( AUDIO::CAudioSettings::SilenceInDecibels, bus.volumeInDecibels ) && !bus.muted );
        ASSERT_FALSE( settings.GetBusSettings( "effects", bus ) );
        ASSERT_TRUE( settings.RemoveBusSettings( "music" ) && !settings.RemoveBusSettings( "music" ) );

        // copies are independent
        AUDIO::CAudioSettings changed = MakeChangedSettings();
        AUDIO::CAudioSettings copy( changed );
        ASSERT_TRUE( SameSettings( changed, copy ) );
        copy.SetBusMuted( "voice", true );
        ASSERT_FALSE( SameSettings( changed, copy ) );
        AUDIO::CAudioSettings assigned;
        assigned = changed;
        ASSERT_TRUE( SameSettings( changed, assigned ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: The volume law has silence at 0 percent and 0 dB at 100 percent" )
    try
    {
        ASSERT_TRUE( Near( AUDIO::CAudioSettings::SilenceInDecibels, AUDIO::CAudioSettings::PercentToDecibels( 0 ) ) );
        ASSERT_TRUE( Near( 0.0f, AUDIO::CAudioSettings::PercentToDecibels( 100 ) ) );
        ASSERT_TRUE( Near( -59.4f, AUDIO::CAudioSettings::PercentToDecibels( 1 ) ) );
        ASSERT_TRUE( Near( -30.0f, AUDIO::CAudioSettings::PercentToDecibels( 50 ) ) );
        ASSERT_TRUE( Near( 0.0f, AUDIO::CAudioSettings::PercentToDecibels( 250 ) ) );

        // louder with every step, and the way back gives the same percentage
        Float32 previous = AUDIO::CAudioSettings::PercentToDecibels( 0 );
        for ( UInt32 p=0; p<=100; ++p )
        {
            Float32 decibels = AUDIO::CAudioSettings::PercentToDecibels( p );
            if ( p > 0 )
                ASSERT_TRUE( decibels > previous );
            previous = decibels;
            ASSERT_TRUE( p == AUDIO::CAudioSettings::DecibelsToPercent( decibels ) );
        }

        // clamped
        ASSERT_TRUE( 0 == AUDIO::CAudioSettings::DecibelsToPercent( AUDIO::CAudioSettings::SilenceInDecibels ) );
        ASSERT_TRUE( 0 == AUDIO::CAudioSettings::DecibelsToPercent( -60.0f ) );
        ASSERT_TRUE( 0 == AUDIO::CAudioSettings::DecibelsToPercent( -500.0f ) );
        ASSERT_TRUE( 100 == AUDIO::CAudioSettings::DecibelsToPercent( 0.0f ) );
        ASSERT_TRUE( 100 == AUDIO::CAudioSettings::DecibelsToPercent( 6.0f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: Speaker layouts and limiter modes have names, in both directions" )
    try
    {
        const UInt32 defaults[] = { 1, 2, 4, 6, 8 };
        const char* names[] = { "mono", "stereo", "quad", "5.1", "7.1" };
        for ( UInt32 i=0; i<5; ++i )
        {
            UInt32 layout = AUDIO::CAudioFormat::GetDefaultChannelLayout( defaults[ i ] );
            ASSERT_TRUE( names[ i ] == AUDIO::CAudioSettings::ChannelLayoutToString( layout ) );
            ASSERT_TRUE( layout == AUDIO::CAudioSettings::StringToChannelLayout( names[ i ], 77 ) );
        }
        ASSERT_TRUE( "default" == AUDIO::CAudioSettings::ChannelLayoutToString( 0 ) );
        ASSERT_TRUE( 0 == AUDIO::CAudioSettings::StringToChannelLayout( "default", 77 ) );
        ASSERT_TRUE( AUDIO::CAudioFormat::GetDefaultChannelLayout( 2 ) == AUDIO::CAudioSettings::StringToChannelLayout( "Stereo", 77 ) );

        // a layout without a name is its mask as a number, which reads back
        UInt32 side = AUDIO::AUDIOSPEAKER_FRONT_LEFT | AUDIO::AUDIOSPEAKER_FRONT_RIGHT | AUDIO::AUDIOSPEAKER_SIDE_LEFT | AUDIO::AUDIOSPEAKER_SIDE_RIGHT;
        CORE::CString sideText = AUDIO::CAudioSettings::ChannelLayoutToString( side );
        ASSERT_TRUE( CORE::UInt32ToString( side ) == sideText );
        ASSERT_TRUE( side == AUDIO::CAudioSettings::StringToChannelLayout( sideText, 77 ) );

        // unknown text gives the default that was passed
        ASSERT_TRUE( 77 == AUDIO::CAudioSettings::StringToChannelLayout( "surround", 77 ) );
        ASSERT_TRUE( 77 == AUDIO::CAudioSettings::StringToChannelLayout( "", 77 ) );
        ASSERT_TRUE( 77 == AUDIO::CAudioSettings::StringToChannelLayout( "12x", 77 ) );
        ASSERT_TRUE( 77 == AUDIO::CAudioSettings::StringToChannelLayout( "99999999999", 77 ) );

        AUDIO::TAudioLimiterMode modes[] = { AUDIO::AUDIOLIMITERMODE_OFF, AUDIO::AUDIOLIMITERMODE_INSTANT, AUDIO::AUDIOLIMITERMODE_LOOKAHEAD };
        const char* modeNames[] = { "off", "instant", "lookahead" };
        for ( UInt32 i=0; i<3; ++i )
        {
            ASSERT_TRUE( CORE::CString( modeNames[ i ] ) == AUDIO::CAudioSettings::LimiterModeToString( modes[ i ] ) );
            AUDIO::TAudioLimiterMode mode = AUDIO::AUDIOLIMITERMODE_OFF == modes[ i ] ? AUDIO::AUDIOLIMITERMODE_INSTANT : AUDIO::AUDIOLIMITERMODE_OFF;
            ASSERT_TRUE( AUDIO::CAudioSettings::StringToLimiterMode( modeNames[ i ], mode ) && modes[ i ] == mode );
        }
        AUDIO::TAudioLimiterMode mode = AUDIO::AUDIOLIMITERMODE_LOOKAHEAD;
        ASSERT_TRUE( AUDIO::CAudioSettings::StringToLimiterMode( "OFF", mode ) && AUDIO::AUDIOLIMITERMODE_OFF == mode );
        ASSERT_FALSE( AUDIO::CAudioSettings::StringToLimiterMode( "loud", mode ) );
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_OFF == mode );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: Saved as a config section and read back, also through JSON" )
    try
    {
        AUDIO::CAudioSettings settings = MakeChangedSettings();
        CORE::CDataNode config( "Config" );
        ASSERT_TRUE( settings.SaveConfig( config ) );
        const CORE::CDataNode* section = config.FindChild( AUDIO::CAudioSettings::ConfigSectionName );
        ASSERT_TRUE( GUCEF_NULL != section && "5.1" == section->GetAttributeValue( "speakerLayout" ).AsString() );

        // from the node that has the section and from the section itself
        AUDIO::CAudioSettings loaded;
        ASSERT_FALSE( SameSettings( settings, loaded ) );
        ASSERT_TRUE( loaded.LoadConfig( config ) );
        ASSERT_TRUE( SameSettings( settings, loaded ) );
        AUDIO::CAudioSettings fromSection;
        ASSERT_TRUE( GUCEF_NULL != section && fromSection.LoadConfig( *section ) );
        ASSERT_TRUE( SameSettings( settings, fromSection ) );

        // saving again replaces the section
        settings.RemoveBusSettings( "effects" );
        ASSERT_TRUE( settings.SaveConfig( config ) );
        ASSERT_TRUE( 1 == config.GetNrOfDirectChildNodes() );
        AUDIO::CAudioSettings reloaded;
        ASSERT_TRUE( reloaded.LoadConfig( config ) );
        ASSERT_TRUE( 1 == reloaded.GetAllBusSettings().size() );

        // no section, no settings
        CORE::CDataNode empty( "Config" );
        ASSERT_FALSE( AUDIO::CAudioSettings().LoadConfig( empty ) );

        // no limiter mode is no limiter mode after a round trip
        AUDIO::CAudioSettings noLimiter;
        CORE::CDataNode plain( "Config" );
        ASSERT_TRUE( noLimiter.SaveConfig( plain ) );
        AUDIO::CAudioSettings plainLoaded;
        plainLoaded.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_OFF );
        ASSERT_TRUE( plainLoaded.LoadConfig( plain ) );
        ASSERT_TRUE( plainLoaded.HasLimiterMode() );    // missing values keep their value

        // through JSON, as an application config file has it, when the JSON codec is there
        CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
        if ( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        {
            AUDIO::CAudioSettings original = MakeChangedSettings();
            CORE::CDataNode jsonConfig( "Config" );
            ASSERT_TRUE( original.SaveConfig( jsonConfig ) );
            CORE::CDynamicBuffer buffer;
            CORE::CDynamicBufferAccess output( buffer );
            ASSERT_TRUE( codec->StoreDataTree( &jsonConfig, &output ) );
            CORE::CDynamicBufferAccess input( buffer );
            CORE::CDataNode parsed;
            ASSERT_TRUE( codec->BuildDataTree( &parsed, &input ) );
            AUDIO::CAudioSettings fromJson;
            ASSERT_TRUE( fromJson.LoadConfig( parsed ) );
            ASSERT_TRUE( SameSettings( original, fromJson ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: What a loaded section does not have stays, unknown names are ignored, buses nobody shows survive" )
    try
    {
        CORE::CDataNode config( "Config" );
        CORE::CDataNode* section = config.AddChild( AUDIO::CAudioSettings::ConfigSectionName );
        section->SetAttribute( "sampleRate", (UInt32) 96000 );
        section->SetAttribute( "speakerLayout", "surround sound" );
        section->SetAttribute( "limiterMode", "loud" );
        CORE::CDataNode* nameless = section->AddChild( "Bus" );
        nameless->SetAttribute( "volumeDb", -5.0f );
        nameless->SetAttribute( "muted", true );
        CORE::CDataNode* voice = section->AddChild( "Bus" );
        voice->SetAttribute( "name", "voice" );
        voice->SetAttribute( "volumeDb", -12.0f );

        AUDIO::CAudioSettings settings = MakeChangedSettings();
        ASSERT_TRUE( settings.LoadConfig( config ) );
        ASSERT_TRUE( 96000 == settings.GetSampleRate() );
        ASSERT_TRUE( "alsa" == settings.GetDriverName() && "hw:1,0" == settings.GetDeviceId() && 256 == settings.GetBufferSizeInFrames() );
        ASSERT_TRUE( "5.1" == AUDIO::CAudioSettings::ChannelLayoutToString( settings.GetChannelLayout() ) );
        ASSERT_TRUE( settings.HasLimiterMode() && AUDIO::AUDIOLIMITERMODE_OFF == settings.GetLimiterMode() );
        ASSERT_TRUE( Near( -6.0f, settings.GetMasterVolumeInDecibels() ) && settings.IsMasterMuted() );

        // the bus entry without a name is skipped; the others stay, the new one is added
        ASSERT_TRUE( 3 == settings.GetAllBusSettings().size() );
        AUDIO::CAudioSettings::SBusSettings bus;
        ASSERT_TRUE( settings.GetBusSettings( "voice", bus ) && Near( -12.0f, bus.volumeInDecibels ) && !bus.muted );
        ASSERT_TRUE( settings.GetBusSettings( "music", bus ) && Near( -9.0f, bus.volumeInDecibels ) );

        // a bus that is in the config and in the settings: only the values it has change
        CORE::CDataNode second( AUDIO::CAudioSettings::ConfigSectionName );
        CORE::CDataNode* music = second.AddChild( "Bus" );
        music->SetAttribute( "name", "music" );
        music->SetAttribute( "muted", true );
        ASSERT_TRUE( settings.LoadConfig( second ) );
        ASSERT_TRUE( settings.GetBusSettings( "music", bus ) && Near( -9.0f, bus.volumeInDecibels ) && bus.muted );
        ASSERT_TRUE( 96000 == settings.GetSampleRate() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: A device level difference needs a device reopen, volume and mute do not" )
    try
    {
        AUDIO::CAudioSettings applied = MakeChangedSettings();
        ASSERT_FALSE( applied.RequiresDeviceReopen( applied ) );

        AUDIO::CAudioSettings changed( applied );
        changed.SetDriverName( "null" );
        ASSERT_TRUE( changed.RequiresDeviceReopen( applied ) );
        changed = applied;
        changed.SetDeviceId( "hw:2,0" );
        ASSERT_TRUE( changed.RequiresDeviceReopen( applied ) );
        changed = applied;
        changed.SetCaptureDeviceId( "hw:3,0" );
        ASSERT_TRUE( changed.RequiresDeviceReopen( applied ) );
        changed = applied;
        changed.SetCaptureDeviceId( CORE::CString::Empty );
        ASSERT_TRUE( changed.RequiresDeviceReopen( applied ) );
        changed = applied;
        changed.SetLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_DEFAULT );
        ASSERT_TRUE( changed.RequiresDeviceReopen( applied ) );
        changed = applied;
        changed.SetSampleRate( 48000 );
        ASSERT_TRUE( changed.RequiresDeviceReopen( applied ) );
        changed = applied;
        changed.SetBufferSizeInFrames( 512 );
        ASSERT_TRUE( changed.RequiresDeviceReopen( applied ) );
        changed = applied;
        changed.SetChannelLayout( 0 );
        ASSERT_TRUE( changed.RequiresDeviceReopen( applied ) );
        changed = applied;
        changed.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
        ASSERT_TRUE( changed.RequiresDeviceReopen( applied ) );
        changed = applied;
        changed.ClearLimiterMode();
        ASSERT_TRUE( changed.RequiresDeviceReopen( applied ) );

        // the same limiter mode, volumes and mutes, and the buses
        changed = applied;
        changed.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_OFF );
        changed.SetMasterVolumeInDecibels( -40.0f );
        changed.SetMasterMuted( false );
        changed.SetBusVolumeInDecibels( "music", -20.0f );
        changed.SetBusMuted( "music", true );
        changed.SetBusVolumeInDecibels( "new bus", -20.0f );
        ASSERT_FALSE( changed.RequiresDeviceReopen( applied ) );

        // a limiter mode that is not set on both sides is the same
        AUDIO::CAudioSettings a, b;
        ASSERT_FALSE( a.RequiresDeviceReopen( b ) );
        a.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD );
        b.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD );
        ASSERT_FALSE( a.RequiresDeviceReopen( b ) );

        // the same capture device and latency profile are the same
        a.SetCaptureDeviceId( "hw:2,0" );
        b.SetCaptureDeviceId( "hw:2,0" );
        a.SetLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_LOW );
        b.SetLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_LOW );
        ASSERT_FALSE( a.RequiresDeviceReopen( b ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: The engine parameters take the values that are set and keep the rest" )
    try
    {
        AUDIO::CAudioEngineParams params;
        params.SetDriverName( "keepme" );
        params.GetDeviceParams().SetDeviceId( "keepdevice" );
        params.GetDeviceParams().SetBufferSizeInFrames( 1024 );
        params.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 32000, 1, AUDIO::SAMPLETYPE_FLOAT32 ) );
        params.SetLimiterEnabled( false );
        params.SetMaxVoices( 17 );

        // nothing set: nothing changes
        AUDIO::CAudioSettings none;
        none.ApplyToEngineParams( params );
        ASSERT_TRUE( "keepme" == params.GetDriverName() && "keepdevice" == params.GetDeviceParams().GetDeviceId() );
        ASSERT_TRUE( 1024 == params.GetDeviceParams().GetBufferSizeInFrames() && !params.GetLimiterEnabled() && 17 == params.GetMaxVoices() );
        ASSERT_TRUE( AUDIO::CAudioFormat( 32000, 1, AUDIO::SAMPLETYPE_FLOAT32 ) == params.GetDeviceParams().GetPreferredFormat() );

        // only a sample rate: the other parts of the format stay
        AUDIO::CAudioSettings rateOnly;
        rateOnly.SetSampleRate( 44100 );
        rateOnly.ApplyToEngineParams( params );
        ASSERT_TRUE( 44100 == params.GetDeviceParams().GetPreferredFormat().GetSampleRate() && 1 == params.GetDeviceParams().GetPreferredFormat().GetChannelCount() );
        ASSERT_TRUE( "keepme" == params.GetDriverName() && 1024 == params.GetDeviceParams().GetBufferSizeInFrames() );

        // everything
        AUDIO::CAudioSettings all = MakeChangedSettings();
        all.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
        all.ApplyToEngineParams( params );
        ASSERT_TRUE( "alsa" == params.GetDriverName() && "hw:1,0" == params.GetDeviceParams().GetDeviceId() );
        ASSERT_TRUE( 256 == params.GetDeviceParams().GetBufferSizeInFrames() );
        ASSERT_TRUE( 44100 == params.GetDeviceParams().GetPreferredFormat().GetSampleRate() );
        ASSERT_TRUE( 6 == params.GetDeviceParams().GetPreferredFormat().GetChannelCount() );
        ASSERT_TRUE( AUDIO::CAudioFormat::GetDefaultChannelLayout( 6 ) == params.GetDeviceParams().GetPreferredFormat().GetChannelLayout() );
        ASSERT_TRUE( params.GetLimiterEnabled() && AUDIO::AUDIOLIMITERMODE_INSTANT == params.GetLimiterMode() && 17 == params.GetMaxVoices() );

        // the limiter mode goes through as it is: off switches the limiter off and keeps the mode it has when on
        AUDIO::CAudioSettings off;
        off.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_OFF );
        off.ApplyToEngineParams( params );
        ASSERT_TRUE( !params.GetLimiterEnabled() && AUDIO::AUDIOLIMITERMODE_OFF == params.GetLimiterMode() );
        AUDIO::CAudioEngineParams onAgain( params );
        onAgain.SetLimiterEnabled( true );
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_INSTANT == onAgain.GetLimiterMode() );
        AUDIO::CAudioSettings lookahead;
        lookahead.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD );
        lookahead.ApplyToEngineParams( params );
        ASSERT_TRUE( params.GetLimiterEnabled() && AUDIO::AUDIOLIMITERMODE_LOOKAHEAD == params.GetLimiterMode() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: A running engine gets the volumes and mutes, a bus it does not have is skipped" )
    try
    {
        AUDIO::CAudioEngine engine;
        AUDIO::CAudioSettings settings;

        // an engine that is not running is left alone
        ASSERT_TRUE( 0 == settings.ApplyLive( engine ) );

        AUDIO::CAudioEngineParams params;
        params.SetDriverName( "null" );
        params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
        settings.ApplyToEngineParams( params );
        ASSERT_TRUE( engine.Initialize( params ) );
        AUDIO::CAudioBus* music = engine.CreateBus( "music" );
        AUDIO::CAudioBus* effects = engine.CreateBus( "effects" );
        ASSERT_TRUE( GUCEF_NULL != music && GUCEF_NULL != effects );

        settings.SetMasterVolumeInDecibels( -6.0f );
        settings.SetMasterMuted( true );
        settings.SetBusVolumeInDecibels( "music", -9.0f );
        settings.SetBusMuted( "music", true );
        settings.SetBusVolumeInDecibels( "effects", -3.0f );
        settings.SetBusVolumeInDecibels( "nosuchbus", -3.0f );
        ASSERT_TRUE( 3 == settings.ApplyLive( engine ) );

        AUDIO::CAudioBus* master = engine.GetMasterBus();
        ASSERT_TRUE( GUCEF_NULL != master );
        ASSERT_TRUE( Near( -6.0f, master->GetVolumeInDecibels(), 0.01f ) && master->IsMuted() );
        ASSERT_TRUE( Near( -9.0f, music->GetVolumeInDecibels(), 0.01f ) && music->IsMuted() );
        ASSERT_TRUE( Near( -3.0f, effects->GetVolumeInDecibels(), 0.01f ) && !effects->IsMuted() );
        ASSERT_TRUE( GUCEF_NULL == engine.FindBus( "nosuchbus" ) );

        // the user volume replaces the volume the bus had, silence is a gain of 0
        music->SetVolume( 0.25f );
        settings.SetBusVolumeInDecibels( "music", AUDIO::CAudioSettings::SilenceInDecibels );
        settings.SetBusMuted( "music", false );
        settings.SetMasterVolumeInDecibels( 0.0f );
        settings.SetMasterMuted( false );
        settings.ApplyLive( engine );
        ASSERT_TRUE( 0.0f == music->GetVolume() && !music->IsMuted() );
        ASSERT_TRUE( Near( 1.0f, master->GetVolume() ) && !master->IsMuted() );

        engine.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: Settings applied to the engine parameters open the device that was asked for" )
    try
    {
        AUDIO::CAudioSettings settings;
        settings.SetDriverName( "null" );
        settings.SetSampleRate( 44100 );
        settings.SetChannelLayout( AUDIO::CAudioFormat::GetDefaultChannelLayout( 6 ) );
        settings.SetBufferSizeInFrames( 128 );
        settings.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_OFF );

        AUDIO::CAudioEngineParams params;
        params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
        settings.ApplyToEngineParams( params );
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( params ) );
        ASSERT_TRUE( "null" == engine.GetDriverName() );
        ASSERT_TRUE( 44100 == engine.GetDeviceFormat().GetSampleRate() && 6 == engine.GetDeviceFormat().GetChannelCount() );
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_OFF == engine.GetParams().GetLimiterMode() && 0 == engine.GetMixLatencyInFrames() );
        engine.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: Each limiter mode goes through a save and load into the engine, which limits in that mode" )
    try
    {
        const AUDIO::TAudioLimiterMode modes[ 3 ] = { AUDIO::AUDIOLIMITERMODE_OFF, AUDIO::AUDIOLIMITERMODE_INSTANT, AUDIO::AUDIOLIMITERMODE_LOOKAHEAD };
        const UInt32 latencies[ 3 ] = { 0, 0, 72 };                 // 1.5 ms at 48 kHz in lookahead mode
        for ( UInt32 i=0; i<3; ++i )
        {
            AUDIO::CAudioSettings settings;
            settings.SetDriverName( "null" );
            settings.SetLimiterMode( modes[ i ] );
            CORE::CDataNode config( "Config" );
            ASSERT_TRUE( settings.SaveConfig( config ) );
            AUDIO::CAudioSettings loaded;
            ASSERT_TRUE( loaded.LoadConfig( config ) );
            ASSERT_TRUE( loaded.HasLimiterMode() && modes[ i ] == loaded.GetLimiterMode() );

            // over engine parameters that have the other on mode, so the mode is seen to come from the settings
            AUDIO::CAudioEngineParams params;
            params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
            params.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );
            params.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD == modes[ i ] ? AUDIO::AUDIOLIMITERMODE_INSTANT : AUDIO::AUDIOLIMITERMODE_LOOKAHEAD );
            loaded.ApplyToEngineParams( params );
            ASSERT_TRUE( modes[ i ] == params.GetLimiterMode() );

            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( engine.Initialize( params ) );
            ASSERT_TRUE( 48000 == engine.GetMixFormat().GetSampleRate() );
            ASSERT_TRUE( modes[ i ] == engine.GetParams().GetLimiterMode() );
            ASSERT_TRUE( latencies[ i ] == engine.GetMixLatencyInFrames() );
            engine.Shutdown();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LatencyProfile: The low profile writes 5 ms periods, 3 periods, priority 70 and the instant limiter for both devices" )
    try
    {
        AUDIO::CAudioEngineParams params;
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD == params.GetLimiterMode() );
        params.ApplyLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_LOW );

        const AUDIO::CAudioDeviceParams* devices[ 2 ] = { &params.GetDeviceParams(), &params.GetCaptureDeviceParams() };
        for ( UInt32 i=0; i<2; ++i )
        {
            ASSERT_TRUE( 240 == devices[ i ]->GetBufferSizeInFrames() );
            ASSERT_TRUE( 3 == devices[ i ]->GetOptions().GetValueAlways( "periods" ).AsUInt32( 0 ) );
            ASSERT_TRUE( 70 == devices[ i ]->GetOptions().GetValueAlways( "realtimePriority" ).AsUInt32( 0 ) );
        }
        ASSERT_TRUE( AUDIO::AUDIODIRECTION_CAPTURE == params.GetCaptureDeviceParams().GetDirection() );
        ASSERT_TRUE( params.GetLimiterEnabled() && AUDIO::AUDIOLIMITERMODE_INSTANT == params.GetLimiterMode() );

        // the period follows the rate, to the nearest whole frame
        params.ApplyLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_LOW, 16000 );
        ASSERT_TRUE( 80 == params.GetDeviceParams().GetBufferSizeInFrames() && 80 == params.GetCaptureDeviceParams().GetBufferSizeInFrames() );
        params.ApplyLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_LOW, 44100 );
        ASSERT_TRUE( 221 == params.GetDeviceParams().GetBufferSizeInFrames() );
        params.ApplyLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_LOW, 8000 );
        ASSERT_TRUE( 40 == params.GetDeviceParams().GetBufferSizeInFrames() );
        params.ApplyLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_LOW, 0 );
        ASSERT_TRUE( 240 == params.GetDeviceParams().GetBufferSizeInFrames() );
        ASSERT_TRUE( "periods" == AUDIO::CAudioEngineParams::PeriodsOptionName );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LatencyProfile: The default profile gives the driver its choice back, nothing else is touched" )
    try
    {
        AUDIO::CAudioEngineParams params;
        params.GetDeviceParams().SetDeviceId( "keepdevice" );
        params.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 32000, 1, AUDIO::SAMPLETYPE_FLOAT32 ) );
        params.GetDeviceParams().GetOptions().Set( "keepOption", "keep" );
        params.GetCaptureDeviceParams().SetDeviceId( "keepcapture" );
        params.SetMaxBlockFrames( 2048 );
        params.SetCommandQueueSize( 77 );
        params.SetCaptureCommandQueueSize( 33 );
        params.SetMaxVoices( 17 );

        params.ApplyLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_LOW );
        ASSERT_TRUE( "keepdevice" == params.GetDeviceParams().GetDeviceId() && "keepcapture" == params.GetCaptureDeviceParams().GetDeviceId() );
        ASSERT_TRUE( AUDIO::CAudioFormat( 32000, 1, AUDIO::SAMPLETYPE_FLOAT32 ) == params.GetDeviceParams().GetPreferredFormat() );
        ASSERT_TRUE( "keep" == params.GetDeviceParams().GetOptions().GetValueAlways( "keepOption" ).AsString( CORE::CString::Empty ) );
        ASSERT_TRUE( 2048 == params.GetMaxBlockFrames() && 77 == params.GetCommandQueueSize() && 33 == params.GetCaptureCommandQueueSize() && 17 == params.GetMaxVoices() );

        params.ApplyLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_DEFAULT );
        const AUDIO::CAudioDeviceParams* devices[ 2 ] = { &params.GetDeviceParams(), &params.GetCaptureDeviceParams() };
        for ( UInt32 i=0; i<2; ++i )
        {
            ASSERT_TRUE( 0 == devices[ i ]->GetBufferSizeInFrames() );
            ASSERT_FALSE( devices[ i ]->GetOptions().HasKey( "periods" ) );
            ASSERT_TRUE( 0 == devices[ i ]->GetOptions().GetValueAlways( "realtimePriority" ).AsUInt32( 99 ) );
        }
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD == params.GetLimiterMode() );
        ASSERT_TRUE( "keep" == params.GetDeviceParams().GetOptions().GetValueAlways( "keepOption" ).AsString( CORE::CString::Empty ) );
        ASSERT_TRUE( "keepdevice" == params.GetDeviceParams().GetDeviceId() && 2048 == params.GetMaxBlockFrames() && 17 == params.GetMaxVoices() );

        // a limiter that is off stays off, only the mode it has when on changes
        AUDIO::CAudioEngineParams quiet;
        quiet.SetLimiterEnabled( false );
        quiet.ApplyLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_LOW );
        ASSERT_FALSE( quiet.GetLimiterEnabled() );
        quiet.SetLimiterEnabled( true );
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_INSTANT == quiet.GetLimiterMode() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LatencyProfile: The names go both ways, a profile survives a save and load of the engine parameters" )
    try
    {
        ASSERT_TRUE( "default" == CORE::CString( AUDIO::CAudioEngineParams::LatencyProfileToString( AUDIO::AUDIOLATENCYPROFILE_DEFAULT ) ) );
        ASSERT_TRUE( "low" == CORE::CString( AUDIO::CAudioEngineParams::LatencyProfileToString( AUDIO::AUDIOLATENCYPROFILE_LOW ) ) );
        AUDIO::TAudioLatencyProfile profile = AUDIO::AUDIOLATENCYPROFILE_DEFAULT;
        ASSERT_TRUE( AUDIO::CAudioEngineParams::StringToLatencyProfile( "LOW", profile ) && AUDIO::AUDIOLATENCYPROFILE_LOW == profile );
        ASSERT_TRUE( AUDIO::CAudioEngineParams::StringToLatencyProfile( "Default", profile ) && AUDIO::AUDIOLATENCYPROFILE_DEFAULT == profile );
        profile = AUDIO::AUDIOLATENCYPROFILE_LOW;
        ASSERT_FALSE( AUDIO::CAudioEngineParams::StringToLatencyProfile( "ultra", profile ) );
        ASSERT_TRUE( AUDIO::AUDIOLATENCYPROFILE_LOW == profile );

        // the values are explicit in the saved configuration, so loading needs no knowledge of the profile
        AUDIO::CAudioEngineParams original;
        original.ApplyLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_LOW, 16000 );
        CORE::CDataNode config( "Config" );
        ASSERT_TRUE( original.SaveConfig( config ) );
        const CORE::CDataNode* section = config.FindChild( AUDIO::CAudioEngineParams::ConfigSectionName );
        ASSERT_TRUE( GUCEF_NULL != section );
        ASSERT_TRUE( GUCEF_NULL != section->FindChild( AUDIO::CAudioEngineParams::CaptureDeviceSectionName ) );

        AUDIO::CAudioEngineParams loaded;
        ASSERT_TRUE( loaded.LoadConfig( config ) );
        const AUDIO::CAudioDeviceParams* devices[ 2 ] = { &loaded.GetDeviceParams(), &loaded.GetCaptureDeviceParams() };
        for ( UInt32 i=0; i<2; ++i )
        {
            ASSERT_TRUE( 80 == devices[ i ]->GetBufferSizeInFrames() );
            ASSERT_TRUE( 3 == devices[ i ]->GetOptions().GetValueAlways( "periods" ).AsUInt32( 0 ) );
            ASSERT_TRUE( 70 == devices[ i ]->GetOptions().GetValueAlways( "realtimePriority" ).AsUInt32( 0 ) );
        }
        ASSERT_TRUE( loaded.GetLimiterEnabled() && AUDIO::AUDIOLIMITERMODE_INSTANT == loaded.GetLimiterMode() );

        // and an edited value stays edited
        loaded.GetDeviceParams().GetOptions().Delete( "periods" );
        loaded.GetDeviceParams().GetOptions().Set( "periods", "2" );
        CORE::CDataNode edited( "Config" );
        ASSERT_TRUE( loaded.SaveConfig( edited ) );
        AUDIO::CAudioEngineParams reloaded;
        ASSERT_TRUE( reloaded.LoadConfig( edited ) );
        ASSERT_TRUE( 2 == reloaded.GetDeviceParams().GetOptions().GetValueAlways( "periods" ).AsUInt32( 0 ) );
        ASSERT_TRUE( 3 == reloaded.GetCaptureDeviceParams().GetOptions().GetValueAlways( "periods" ).AsUInt32( 0 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LatencyProfile: The settings keep the capture device and the profile by name through a save and load" )
    try
    {
        AUDIO::CAudioSettings original = MakeChangedSettings();
        CORE::CDataNode config( "Config" );
        ASSERT_TRUE( original.SaveConfig( config ) );
        const CORE::CDataNode* section = config.FindChild( AUDIO::CAudioSettings::ConfigSectionName );
        ASSERT_TRUE( GUCEF_NULL != section );
        ASSERT_TRUE( "low" == section->GetAttributeValueOrChildValueByName( "latencyProfile" ).AsString( CORE::CString::Empty ) );
        ASSERT_TRUE( "hw:2,0" == section->GetAttributeValueOrChildValueByName( "captureDevice" ).AsString( CORE::CString::Empty ) );

        AUDIO::CAudioSettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( config ) );
        ASSERT_TRUE( SameSettings( original, loaded ) );
        ASSERT_TRUE( "hw:2,0" == loaded.GetCaptureDeviceId() && AUDIO::AUDIOLATENCYPROFILE_LOW == loaded.GetLatencyProfile() );

        // the default profile is written too; it loads over another profile, as the names of the other settings do
        AUDIO::CAudioSettings plain;
        CORE::CDataNode plainConfig( "Config" );
        ASSERT_TRUE( plain.SaveConfig( plainConfig ) );
        const CORE::CDataNode* plainSection = plainConfig.FindChild( AUDIO::CAudioSettings::ConfigSectionName );
        ASSERT_TRUE( GUCEF_NULL != plainSection );
        ASSERT_TRUE( "default" == plainSection->GetAttributeValueOrChildValueByName( "latencyProfile" ).AsString( CORE::CString::Empty ) );
        AUDIO::CAudioSettings back = original;
        ASSERT_TRUE( back.LoadConfig( plainConfig ) );
        ASSERT_TRUE( AUDIO::AUDIOLATENCYPROFILE_DEFAULT == back.GetLatencyProfile() );
        AUDIO::CAudioSettings fresh;
        ASSERT_TRUE( fresh.LoadConfig( plainConfig ) );
        ASSERT_TRUE( fresh.GetCaptureDeviceId().IsNULLOrEmpty() && AUDIO::AUDIOLATENCYPROFILE_DEFAULT == fresh.GetLatencyProfile() );

        // a section without them keeps them, a profile name that is not one is ignored
        CORE::CDataNode partial( AUDIO::CAudioSettings::ConfigSectionName );
        partial.SetAttribute( "sampleRate", (UInt32) 16000 );
        AUDIO::CAudioSettings kept = original;
        ASSERT_TRUE( kept.LoadConfig( partial ) );
        ASSERT_TRUE( "hw:2,0" == kept.GetCaptureDeviceId() && AUDIO::AUDIOLATENCYPROFILE_LOW == kept.GetLatencyProfile() && 16000 == kept.GetSampleRate() );
        CORE::CDataNode unknown( AUDIO::CAudioSettings::ConfigSectionName );
        unknown.SetAttribute( "latencyProfile", "ultra" );
        ASSERT_TRUE( kept.LoadConfig( unknown ) );
        ASSERT_TRUE( AUDIO::AUDIOLATENCYPROFILE_LOW == kept.GetLatencyProfile() );

        // through JSON, as an application config file has it, when the JSON codec is there
        CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
        if ( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        {
            CORE::CDynamicBuffer buffer;
            CORE::CDynamicBufferAccess output( buffer );
            ASSERT_TRUE( codec->StoreDataTree( &config, &output ) );
            CORE::CDynamicBufferAccess input( buffer );
            CORE::CDataNode parsed;
            ASSERT_TRUE( codec->BuildDataTree( &parsed, &input ) );
            AUDIO::CAudioSettings fromJson;
            ASSERT_TRUE( fromJson.LoadConfig( parsed ) );
            ASSERT_TRUE( SameSettings( original, fromJson ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LatencyProfile: The settings map the profile and the capture device onto the engine parameters" )
    try
    {
        AUDIO::CAudioEngineParams params;
        params.GetDeviceParams().SetBufferSizeInFrames( 1024 );
        params.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 16000, 1, AUDIO::SAMPLETYPE_FLOAT32 ) );
        params.GetDeviceParams().GetOptions().Set( "periods", "6" );
        params.GetCaptureDeviceParams().SetDeviceId( "keepcapture" );

        // the default profile and an empty capture device change nothing
        AUDIO::CAudioSettings none;
        none.ApplyToEngineParams( params );
        ASSERT_TRUE( 1024 == params.GetDeviceParams().GetBufferSizeInFrames() && "keepcapture" == params.GetCaptureDeviceParams().GetDeviceId() );
        ASSERT_TRUE( 6 == params.GetDeviceParams().GetOptions().GetValueAlways( "periods" ).AsUInt32( 0 ) );
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD == params.GetLimiterMode() );

        // the capture device alone
        AUDIO::CAudioSettings captureOnly;
        captureOnly.SetCaptureDeviceId( "hw:2,0" );
        captureOnly.ApplyToEngineParams( params );
        ASSERT_TRUE( "hw:2,0" == params.GetCaptureDeviceParams().GetDeviceId() && AUDIO::AUDIODIRECTION_CAPTURE == params.GetCaptureDeviceParams().GetDirection() );
        ASSERT_TRUE( 1024 == params.GetDeviceParams().GetBufferSizeInFrames() );

        // the low profile at the rate the engine parameters prefer
        AUDIO::CAudioSettings low;
        low.SetLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_LOW );
        low.ApplyToEngineParams( params );
        ASSERT_TRUE( 80 == params.GetDeviceParams().GetBufferSizeInFrames() && 80 == params.GetCaptureDeviceParams().GetBufferSizeInFrames() );
        ASSERT_TRUE( 3 == params.GetDeviceParams().GetOptions().GetValueAlways( "periods" ).AsUInt32( 0 ) );
        ASSERT_TRUE( 70 == params.GetCaptureDeviceParams().GetOptions().GetValueAlways( "realtimePriority" ).AsUInt32( 0 ) );
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_INSTANT == params.GetLimiterMode() );
        ASSERT_TRUE( "hw:2,0" == params.GetCaptureDeviceParams().GetDeviceId() );

        // the sample rate of the settings decides the period
        low.SetSampleRate( 48000 );
        low.ApplyToEngineParams( params );
        ASSERT_TRUE( 240 == params.GetDeviceParams().GetBufferSizeInFrames() && 240 == params.GetCaptureDeviceParams().GetBufferSizeInFrames() );
        ASSERT_TRUE( 48000 == params.GetDeviceParams().GetPreferredFormat().GetSampleRate() );

        // no rate anywhere: 48 kHz
        AUDIO::CAudioEngineParams bare;
        AUDIO::CAudioSettings lowOnly;
        lowOnly.SetLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_LOW );
        lowOnly.ApplyToEngineParams( bare );
        ASSERT_TRUE( 240 == bare.GetDeviceParams().GetBufferSizeInFrames() );

        // the buffer size and the limiter mode of the settings win over the values of the profile
        AUDIO::CAudioSettings explicitValues;
        explicitValues.SetLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_LOW );
        explicitValues.SetBufferSizeInFrames( 512 );
        explicitValues.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD );
        AUDIO::CAudioEngineParams wins;
        explicitValues.ApplyToEngineParams( wins );
        ASSERT_TRUE( 512 == wins.GetDeviceParams().GetBufferSizeInFrames() && 240 == wins.GetCaptureDeviceParams().GetBufferSizeInFrames() );
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD == wins.GetLimiterMode() );
        ASSERT_TRUE( 70 == wins.GetDeviceParams().GetOptions().GetValueAlways( "realtimePriority" ).AsUInt32( 0 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LatencyProfile: An engine opened with the low profile on the null driver runs with the instant limiter and the 5 ms period" )
    try
    {
        AUDIO::CAudioSettings settings;
        settings.SetDriverName( "null" );
        settings.SetSampleRate( 48000 );
        settings.SetLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_LOW );

        AUDIO::CAudioEngineParams params;
        params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
        settings.ApplyToEngineParams( params );

        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( params ) );
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_INSTANT == engine.GetParams().GetLimiterMode() && 0 == engine.GetMixLatencyInFrames() );
        ASSERT_TRUE( 240 == engine.GetParams().GetDeviceParams().GetBufferSizeInFrames() );
        ASSERT_TRUE( 70 == engine.GetParams().GetDeviceParams().GetOptions().GetValueAlways( "realtimePriority" ).AsUInt32( 0 ) );
        engine.Shutdown();

        // a change of profile is a change of the device
        AUDIO::CAudioSettings other( settings );
        other.SetLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_DEFAULT );
        ASSERT_TRUE( other.RequiresDeviceReopen( settings ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
