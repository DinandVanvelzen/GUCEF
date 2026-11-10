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
}

/*-------------------------------------------------------------------------*/
