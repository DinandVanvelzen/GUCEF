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
#include <string.h>

#ifndef GUCEFAUDIO_H
#include "gucefAUDIO.h"
#define GUCEFAUDIO_H
#endif /* GUCEFAUDIO_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#include "gucefAUDIO_CAudioGlobal.h"
#define GUCEF_AUDIO_CAUDIOGLOBAL_H
#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */

#ifndef GUCEF_AUDIO_CAUDIOBUSLAYOUT_H
#include "gucefAUDIO_CAudioBusLayout.h"
#define GUCEF_AUDIO_CAUDIOBUSLAYOUT_H
#endif /* GUCEF_AUDIO_CAUDIOBUSLAYOUT_H ? */

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

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioBusLayout.h"

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

typedef GUCEF::vector< Float32 > TSamples;

bool
Near( Float64 a, Float64 b, Float64 tolerance )
{
    return fabs( a - b ) <= tolerance;
}

AUDIO::CAudioEngineParams
NullParams( bool threaded = false )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( threaded ? AUDIO::AUDIOTHREADING_THREADED : AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.SetUpdateIntervalInMs( 1 );
    params.SetLimiterEnabled( false );
    params.GetDeviceParams().SetBufferSizeInFrames( 256 );
    params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );
    return params;
}

AUDIO::CNullAudioDevice*
NullDevice( AUDIO::CAudioEngine& engine )
{
    return engine.GetDevice()->AsNullDevice();
}

/** Renders and updates until the future is ready: the audio thread has to run for its commands */
bool
RenderUntilReady( AUDIO::CAudioEngine& engine, const CORE::CFutureResult& future, UInt32 maxBlocks = 50 )
{
    for ( UInt32 i=0; i<maxBlocks; ++i )
    {
        if ( future.IsReady() )
            return true;
        NullDevice( engine )->Render( 256 );
        engine.Update();
    }
    return future.IsReady();
}

bool
IsFailedWith( const CORE::CFutureResult& future, CORE::TTaskStatus status )
{
    if ( !future.HasNoFuture() )
        return false;
    CORE::CTaskPtr task = future.GetResult( 1000 );
    return !task.IsNULL() && status == task->GetTaskStatus();
}

/** Applies the layout and waits for the audio thread */
bool
ApplyAndWait( AUDIO::CAudioEngine& engine, const AUDIO::CAudioBusLayout& layout )
{
    CORE::CFutureResult applied = engine.ApplyBusLayout( layout );
    return !applied.HasNoFuture() && RenderUntilReady( engine, applied ) && !applied.HasNoFuture();
}

bool
SetParameter( const AUDIO::TAudioEffectPtr& effect, const char* name, Float32 value )
{
    UInt32 id = 0;
    AUDIO::TAudioEffectPtr target( effect );
    if ( target.IsNULL() || !target->TryGetParameterId( name, id ) )
        return false;
    target->SetParameter( id, value );
    return true;
}

Float32
GetParameter( const AUDIO::TAudioEffectPtr& effect, const char* name )
{
    UInt32 id = 0;
    return !effect.IsNULL() && effect->TryGetParameterId( name, id ) ? effect->GetParameter( id ) : -12345.0f;
}

AUDIO::TAudioClipPtr
MakeSine( UInt32 frames, Float64 frequency, Float32 amplitude )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( 48000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), frames );
    for ( UInt32 f=0; f<frames; ++f )
        audio.SetSampleFromFloat( f, 0, amplitude * (Float32) sin( 2.0 * 3.14159265358979323846 * frequency * (Float64) f / 48000.0 ) );
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

/**
 *  A mixing set-up like a game has it, as a layout. The buses are listed parents first, in the order the by hand version
 *  makes them, so both have the same processing order and render the same bits
 */
AUDIO::CAudioBusLayout
MakeSampleLayout( void )
{
    AUDIO::CAudioBusLayout layout;

    AUDIO::CAudioBusLayout::SBus& master = layout.AddBus( "master" );
    AUDIO::CAudioBusLayout::SEffect& filter = master.AddEffect( 1, "filter" );
    filter.SetParameter( "type", CORE::CString( "lowPass" ) );
    filter.SetParameter( "frequency", 9000.0f );

    layout.AddBus( "music", "master", -6.0f );

    AUDIO::CAudioBusLayout::SBus& sfx = layout.AddBus( "sfx", "", -3.5f );
    AUDIO::CAudioBusLayout::SEffect& compressor = sfx.AddEffect( 0, "compressor" );
    compressor.SetParameter( "threshold", -20.0f );
    compressor.SetParameter( "ratio", 3.0f );
    AUDIO::CAudioBusLayout::SEffect& equalizer = sfx.AddEffect( 2, "equalizer" );
    equalizer.SetOption( "bands", (Int32) 12 );
    equalizer.enabled = false;
    equalizer.wet = 0.5f;
    equalizer.dry = 0.5f;
    sfx.AddSend( "reverb", -10.0f, false );

    AUDIO::CAudioBusLayout::SBus& ui = layout.AddBus( "ui", "sfx", -2.0f );
    ui.AddSend( "reverb", -3.0f, true );

    AUDIO::CAudioBusLayout::SBus& reverb = layout.AddBus( "reverb", "master" );
    AUDIO::CAudioBusLayout::SEffect& delay = reverb.AddEffect( 0, "delay" );
    delay.SetOption( "maxDelay", 1000.0f );
    delay.SetParameter( "time", 40.0f );
    delay.SetParameter( "feedback", 0.4f );
    return layout;
}

/** The same set-up made with the calls of the engine, one by one */
bool
BuildByHand( AUDIO::CAudioEngine& engine )
{
    AUDIO::CAudioBus* master = engine.GetMasterBus();
    AUDIO::CAudioBus* music = engine.CreateBus( "music" );
    AUDIO::CAudioBus* sfx = engine.CreateBus( "sfx" );
    AUDIO::CAudioBus* ui = engine.CreateBus( "ui", sfx );
    AUDIO::CAudioBus* reverb = engine.CreateBus( "reverb" );
    if ( GUCEF_NULL == music || GUCEF_NULL == sfx || GUCEF_NULL == ui || GUCEF_NULL == reverb )
        return false;
    music->SetVolumeInDecibels( -6.0f );
    sfx->SetVolumeInDecibels( -3.5f );
    ui->SetVolumeInDecibels( -2.0f );

    CORE::CValueList delayOptions;
    delayOptions.Set( "maxDelay", 1000.0f );
    CORE::CValueList equalizerOptions;
    equalizerOptions.Set( "bands", (Int32) 12 );
    AUDIO::TAudioEffectPtr filter = engine.CreateEffect( "filter" );
    AUDIO::TAudioEffectPtr compressor = engine.CreateEffect( "compressor" );
    AUDIO::TAudioEffectPtr equalizer = engine.CreateEffect( "equalizer", equalizerOptions );
    AUDIO::TAudioEffectPtr delay = engine.CreateEffect( "delay", delayOptions );
    if ( filter.IsNULL() || compressor.IsNULL() || equalizer.IsNULL() || delay.IsNULL() )
        return false;
    UInt32 typeId = 0;
    Float32 lowPassValue = 0.0f;
    if ( !filter->TryGetParameterId( "type", typeId ) || !filter->GetParameterInfo( typeId )->TryGetEnumValue( "lowPass", lowPassValue ) )
        return false;
    filter->SetParameter( typeId, lowPassValue );
    SetParameter( filter, "frequency", 9000.0f );
    SetParameter( compressor, "threshold", -20.0f );
    SetParameter( compressor, "ratio", 3.0f );
    SetParameter( delay, "time", 40.0f );
    SetParameter( delay, "feedback", 0.4f );

    GUCEF::vector< CORE::CFutureResult > pending;
    pending.push_back( engine.InsertEffect( master, 1, filter ) );
    pending.push_back( engine.InsertEffect( sfx, 0, compressor ) );
    pending.push_back( engine.InsertEffect( sfx, 2, equalizer ) );
    pending.push_back( engine.InsertEffect( reverb, 0, delay ) );
    sfx->SetEffectEnabled( 2, false );
    sfx->SetEffectMix( 2, 0.5f, 0.5f );
    pending.push_back( engine.AddSend( sfx, reverb, AUDIO::CAudioBus::DecibelsToLinear( -10.0f ), false ) );
    pending.push_back( engine.AddSend( ui, reverb, AUDIO::CAudioBus::DecibelsToLinear( -3.0f ), true ) );
    for ( size_t i=0; i<pending.size(); ++i )
        if ( pending[ i ].HasNoFuture() || !RenderUntilReady( engine, pending[ i ] ) )
            return false;
    return true;
}

/** Plays a few sounds into the buses of the set-up and gives the audio the device made of them */
bool
RenderScenario( AUDIO::CAudioEngine& engine, TSamples& output )
{
    // let everything settle: parameters glide for up to 100 ms
    NullDevice( engine )->Render( 256 * 80 );
    engine.Update();
    NullDevice( engine )->ClearRenderedAudio();

    AUDIO::CAudioBus* buses[ 4 ] = { engine.FindBus( "music" ), engine.FindBus( "sfx" ), engine.FindBus( "ui" ), engine.FindBus( "reverb" ) };
    for ( int i=0; i<4; ++i )
        if ( GUCEF_NULL == buses[ i ] )
            return false;
    engine.Play( MakeSine( 20000, 220.0, 0.6f ), buses[ 0 ] );
    engine.Play( MakeSine( 20000, 1000.0, 0.9f ), buses[ 1 ] );
    engine.Play( MakeSine( 20000, 3300.0, 0.4f ), buses[ 2 ] );
    for ( UInt32 b=0; b<100; ++b )
    {
        NullDevice( engine )->Render( 256 );
        engine.Update();
    }
    const AUDIO::CAudioBuffer& rendered = NullDevice( engine )->GetRenderedAudio();
    const Float32* data = static_cast< const Float32* >( rendered.GetDataPtr() );
    output.assign( data, data + (size_t) rendered.GetFrameCount() * 2 );
    return !output.empty();
}

Float64
LargestDifference( const TSamples& a, const TSamples& b )
{
    if ( a.size() != b.size() )
        return 1.0e9;
    Float64 largest = 0.0;
    for ( size_t i=0; i<a.size(); ++i )
        if ( fabs( (Float64) a[ i ] - (Float64) b[ i ] ) > largest )
            largest = fabs( (Float64) a[ i ] - (Float64) b[ i ] );
    return largest;
}

Float64
PeakOf( const TSamples& a )
{
    Float64 peak = 0.0;
    for ( size_t i=0; i<a.size(); ++i )
        if ( fabs( (Float64) a[ i ] ) > peak )
            peak = fabs( (Float64) a[ i ] );
    return peak;
}

/**
 *  Takes a tree through the JSON codec and back, when the codec is there (it is a plugin the test application does not
 *  bring along; GUCEF_TEST_AUDIO_JSON_PLUGIN names the library to try). False when it is not, the callers then skip that part
 */
bool
TryThroughJson( const CORE::CDataNode& source, CORE::CDataNode& result )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        return false;

    CORE::CDynamicBuffer buffer;
    CORE::CDynamicBufferAccess output( buffer );
    if ( !codec->StoreDataTree( &source, &output ) )
        return false;
    CORE::CDynamicBufferAccess input( buffer );
    return codec->BuildDataTree( &result, &input );
}

/** True when the json codec was found in the plugin that the environment names */
bool
LoadJsonCodecPlugin( void )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        return true;

    const CORE::CString path = CORE::GetEnv( "GUCEF_TEST_AUDIO_JSON_PLUGIN" );
    if ( path.IsNULLOrEmpty() )
        return false;
    CORE::CCoreGlobal::Instance()->GetPluginControl().AddPluginFromDir( path, "AudioBusLayoutJson", true );
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false );
}

bool
SameDeviceParams( const AUDIO::CAudioDeviceParams& a, const AUDIO::CAudioDeviceParams& b )
{
    if ( a.GetDeviceId() != b.GetDeviceId() || a.GetDirection() != b.GetDirection() || a.GetShareMode() != b.GetShareMode() ||
         a.GetBufferSizeInFrames() != b.GetBufferSizeInFrames() || a.GetPreferredFormat() != b.GetPreferredFormat() )
        return false;
    if ( a.GetOptions().GetKeyCount() != b.GetOptions().GetKeyCount() )
        return false;
    for ( UInt32 i=0; i<a.GetOptions().GetKeyCount(); ++i )
    {
        const CORE::CString key = a.GetOptions().GetKey( i ).AsString();
        if ( a.GetOptions().GetValueAlways( key, "" ).AsString() != b.GetOptions().GetValueAlways( key, "" ).AsString() )
            return false;
    }
    return true;
}

bool
SameParams( const AUDIO::CAudioEngineParams& a, const AUDIO::CAudioEngineParams& b )
{
    return a.GetDriverName() == b.GetDriverName() && SameDeviceParams( a.GetDeviceParams(), b.GetDeviceParams() ) &&
           a.GetMaxVoices() == b.GetMaxVoices() && a.GetMaxBuses() == b.GetMaxBuses() && a.GetMaxBlockFrames() == b.GetMaxBlockFrames() &&
           a.GetCommandQueueSize() == b.GetCommandQueueSize() && a.GetEventQueueSize() == b.GetEventQueueSize() &&
           a.GetThreadingMode() == b.GetThreadingMode() && a.GetUpdateIntervalInMs() == b.GetUpdateIntervalInMs() &&
           a.GetThreadPoolName() == b.GetThreadPoolName() && a.GetStartDevice() == b.GetStartDevice() && a.GetPanLaw() == b.GetPanLaw() &&
           a.GetLimiterEnabled() == b.GetLimiterEnabled() && a.GetLimiterMode() == b.GetLimiterMode() &&
           a.GetLimiterCeilingInDecibels() == b.GetLimiterCeilingInDecibels() && a.GetLimiterLookaheadInMs() == b.GetLimiterLookaheadInMs() &&
           a.GetLimiterReleaseInMs() == b.GetLimiterReleaseInMs() && a.GetMaxEffectsPerBus() == b.GetMaxEffectsPerBus() &&
           a.GetMaxSendsPerBus() == b.GetMaxSendsPerBus() && a.GetMaxVoiceEffects() == b.GetMaxVoiceEffects() &&
           a.GetMaxVoiceSends() == b.GetMaxVoiceSends();
}

/** Parameters that differ from the defaults in every respect */
AUDIO::CAudioEngineParams
MakeChangedParams( void )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetMaxVoices( 77 );
    params.SetMaxBuses( 21 );
    params.SetMaxBlockFrames( 2048 );
    params.SetCommandQueueSize( 64 );
    params.SetEventQueueSize( 128 );
    params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.SetUpdateIntervalInMs( 9 );
    params.SetThreadPoolName( "MyPool" );
    params.SetStartDevice( false );
    params.SetPanLaw( AUDIO::AUDIOPANLAW_BALANCE );
    params.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
    params.SetLimiterEnabled( false );
    params.SetLimiterCeilingInDecibels( -2.5f );
    params.SetLimiterLookaheadInMs( 3.25f );
    params.SetLimiterReleaseInMs( 120.0f );
    params.SetMaxEffectsPerBus( 5 );
    params.SetMaxSendsPerBus( 3 );
    params.SetMaxVoiceEffects( 2 );
    params.SetMaxVoiceSends( 1 );
    params.GetDeviceParams().SetDeviceId( "hw:1,0" );
    params.GetDeviceParams().SetDirection( AUDIO::AUDIODIRECTION_CAPTURE );
    params.GetDeviceParams().SetShareMode( AUDIO::AUDIOSHAREMODE_EXCLUSIVE );
    params.GetDeviceParams().SetBufferSizeInFrames( 512 );
    params.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 44100, 6, AUDIO::SAMPLETYPE_INT16, AUDIO::AUDIOSPEAKER_FRONT_LEFT | AUDIO::AUDIOSPEAKER_FRONT_RIGHT | AUDIO::AUDIOSPEAKER_FRONT_CENTER | AUDIO::AUDIOSPEAKER_LOW_FREQUENCY | AUDIO::AUDIOSPEAKER_SIDE_LEFT | AUDIO::AUDIOSPEAKER_SIDE_RIGHT ) );
    params.GetDeviceParams().GetOptions().Set( "periods", "3" );
    params.GetDeviceParams().GetOptions().Set( "name", "my device" );
    return params;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformEngineParamsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioBusLayout: The engine parameters are saved as a section, with enumerations by name, and read back" )
    try
    {
        const AUDIO::CAudioEngineParams original = MakeChangedParams();
        CORE::CDataNode config( "Config" );
        ASSERT_TRUE( original.SaveConfig( config ) );

        const CORE::CDataNode* section = config.FindChild( AUDIO::CAudioEngineParams::ConfigSectionName );
        ASSERT_TRUE( GUCEF_NULL != section && "AudioEngine" == section->GetName() );
        ASSERT_TRUE( "cooperative" == section->GetAttributeValue( "threadingMode" ).AsString() );
        ASSERT_TRUE( "balance" == section->GetAttributeValue( "panLaw" ).AsString() );
        ASSERT_TRUE( "instant" == section->GetAttributeValue( "limiterMode" ).AsString() );
        ASSERT_FALSE( section->GetAttributeValue( "limiterEnabled" ).AsBool() );
        ASSERT_TRUE( 77 == section->GetAttributeValue( "maxVoices" ).AsUInt32() && 3 == section->GetAttributeValue( "maxSendsPerBus" ).AsUInt32() );
        const CORE::CDataNode* device = section->FindChild( "AudioDevice" );
        ASSERT_TRUE( GUCEF_NULL != device && "exclusive" == device->GetAttributeValue( "shareMode" ).AsString() && "capture" == device->GetAttributeValue( "direction" ).AsString() );
        ASSERT_TRUE( "int16" == device->GetAttributeValue( "sampleType" ).AsString() && "5.1(side)" != device->GetAttributeValue( "speakerLayout" ).AsString() );
        ASSERT_TRUE( GUCEF_NULL != device->FindChild( "Options" ) && "3" == device->FindChild( "Options" )->GetAttributeValue( "periods" ).AsString() );

        // from the node that has the section, and from the section itself
        AUDIO::CAudioEngineParams loaded;
        ASSERT_FALSE( SameParams( original, loaded ) );
        ASSERT_TRUE( loaded.LoadConfig( config ) );
        ASSERT_TRUE( SameParams( original, loaded ) );
        AUDIO::CAudioEngineParams fromSection;
        ASSERT_TRUE( fromSection.LoadConfig( *section ) );
        ASSERT_TRUE( SameParams( original, fromSection ) );

        // the limiter is off in these parameters but remembers the mode it has when it is on
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_OFF == loaded.GetLimiterMode() );
        loaded.SetLimiterEnabled( true );
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_INSTANT == loaded.GetLimiterMode() );

        // saving again replaces the values and does not pile up nodes
        AUDIO::CAudioEngineParams changedAgain( original );
        changedAgain.GetDeviceParams().GetOptions().Clear();
        changedAgain.SetMaxVoices( 5 );
        ASSERT_TRUE( changedAgain.SaveConfig( config ) );
        ASSERT_TRUE( 1 == config.GetNrOfDirectChildNodes() );
        const CORE::CDataNode* again = config.FindChild( "AudioEngine" );
        ASSERT_TRUE( GUCEF_NULL != again && 5 == again->GetAttributeValue( "maxVoices" ).AsUInt32() && 1 == again->GetNrOfDirectChildNodes() );
        ASSERT_TRUE( GUCEF_NULL == again->FindChild( "AudioDevice" )->FindChild( "Options" ) );

        // the defaults survive as well
        AUDIO::CAudioEngineParams defaults;
        CORE::CDataNode defaultConfig( "Config" );
        ASSERT_TRUE( defaults.SaveConfig( defaultConfig ) );
        AUDIO::CAudioEngineParams defaultsLoaded( original );
        defaultsLoaded.GetDeviceParams().GetOptions().Clear();              // a section without "Options" leaves the options as they are
        ASSERT_TRUE( defaultsLoaded.LoadConfig( defaultConfig ) );
        ASSERT_TRUE( defaults.GetDriverName() == defaultsLoaded.GetDriverName() );
        ASSERT_TRUE( SameDeviceParams( defaults.GetDeviceParams(), defaultsLoaded.GetDeviceParams() ) );
        ASSERT_TRUE( SameParams( defaults, defaultsLoaded ) );
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD == defaultsLoaded.GetLimiterMode() && defaultsLoaded.GetLimiterEnabled() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBusLayout: Engine parameters that a section does not have stay, names that are not valid are ignored, no section is no result" )
    try
    {
        AUDIO::CAudioEngineParams params;
        CORE::CDataNode config( "Config" );
        CORE::CDataNode* section = config.AddChild( AUDIO::CAudioEngineParams::ConfigSectionName );
        section->SetAttribute( "maxVoices", (UInt32) 9 );
        section->SetAttribute( "threadingMode", "everywhere" );
        section->SetAttribute( "panLaw", "Balance" );               // without regard for case
        section->SetAttribute( "limiterMode", "turbo" );
        CORE::CDataNode* device = section->AddChild( "AudioDevice" );
        device->SetAttribute( "shareMode", "private" );
        device->SetAttribute( "sampleType", "float99" );
        device->SetAttribute( "speakerLayout", "surround sound" );
        device->SetAttribute( "bufferFrames", (UInt32) 128 );

        ASSERT_TRUE( params.LoadConfig( config ) );
        AUDIO::CAudioEngineParams defaults;
        ASSERT_TRUE( 9 == params.GetMaxVoices() && defaults.GetMaxBuses() == params.GetMaxBuses() );
        ASSERT_TRUE( AUDIO::AUDIOTHREADING_AUTO == params.GetThreadingMode() && AUDIO::AUDIOPANLAW_BALANCE == params.GetPanLaw() );
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD == params.GetLimiterMode() );
        ASSERT_TRUE( AUDIO::AUDIOSHAREMODE_SHARED == params.GetDeviceParams().GetShareMode() && !params.GetDeviceParams().GetPreferredFormat().IsValid() );
        ASSERT_TRUE( 128 == params.GetDeviceParams().GetBufferSizeInFrames() );

        // "off" switches the limiter off, and an explicit switch has the last word
        CORE::CDataNode off( "Config" );
        off.AddChild( "AudioEngine" )->SetAttribute( "limiterMode", "off" );
        ASSERT_TRUE( params.LoadConfig( off ) && !params.GetLimiterEnabled() && AUDIO::AUDIOLIMITERMODE_OFF == params.GetLimiterMode() );
        CORE::CDataNode back( "Config" );
        back.AddChild( "AudioEngine" )->SetAttribute( "limiterEnabled", true );
        ASSERT_TRUE( params.LoadConfig( back ) && params.GetLimiterEnabled() && AUDIO::AUDIOLIMITERMODE_LOOKAHEAD == params.GetLimiterMode() );

        CORE::CDataNode empty( "Config" );
        ASSERT_FALSE( AUDIO::CAudioEngineParams().LoadConfig( empty ) );
        AUDIO::CAudioDeviceParams deviceParams;
        ASSERT_FALSE( deviceParams.LoadConfig( empty ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBusLayout: Engine parameters go through JSON as a settings file has them (when the codec is there)" )
    try
    {
        const bool codecThere = LoadJsonCodecPlugin();
        const AUDIO::CAudioEngineParams original = MakeChangedParams();
        CORE::CDataNode config( "Config" );
        ASSERT_TRUE( original.SaveConfig( config ) );
        CORE::CDataNode parsed;
        if ( codecThere )
        {
            ASSERT_TRUE( TryThroughJson( config, parsed ) );
            AUDIO::CAudioEngineParams fromJson;
            ASSERT_TRUE( fromJson.LoadConfig( parsed ) );
            ASSERT_TRUE( SameParams( original, fromJson ) );
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioBusLayout: the json codec is not there, the JSON part of the test is skipped" );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBusLayout: Parameters from a section start an engine with the capacities in it" )
    try
    {
        AUDIO::CAudioEngineParams params = NullParams();
        params.SetMaxEffectsPerBus( 3 );
        params.SetMaxSendsPerBus( 2 );
        params.SetMaxBuses( 6 );
        CORE::CDataNode config( "Config" );
        ASSERT_TRUE( params.SaveConfig( config ) );

        AUDIO::CAudioEngineParams loaded;
        ASSERT_TRUE( loaded.LoadConfig( config ) );
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( loaded ) );
        AUDIO::CAudioBus* bus = engine.CreateBus( "a" );
        ASSERT_TRUE( GUCEF_NULL != bus && 3 == bus->GetEffectSlotCount() && 2 == bus->GetSendSlotCount() );
        ASSERT_TRUE( SameParams( loaded, engine.GetParams() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformLayoutValueTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioBusLayout: A layout is a value: built up, found by name, copied, compared by what it describes" )
    try
    {
        AUDIO::CAudioBusLayout layout = MakeSampleLayout();
        ASSERT_FALSE( layout.IsEmpty() );
        ASSERT_TRUE( 5 == layout.GetBuses().size() );
        ASSERT_TRUE( GUCEF_NULL != layout.FindBus( "ui" ) && GUCEF_NULL == layout.FindBus( "UI" ) && GUCEF_NULL == layout.FindBus( "nothing" ) );
        ASSERT_TRUE( 2 == layout.FindBus( "sfx" )->effects.size() && 1 == layout.FindBus( "sfx" )->sends.size() );
        ASSERT_TRUE( layout.FindBus( "ui" )->sends[ 0 ].preFader && "sfx" == layout.FindBus( "ui" )->parent );

        AUDIO::CAudioBusLayout copy( layout );
        ASSERT_TRUE( copy == layout && !( copy != layout ) );
        AUDIO::CAudioBusLayout assigned;
        ASSERT_TRUE( assigned.IsEmpty() && assigned != layout );
        assigned = layout;
        ASSERT_TRUE( assigned == layout );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBusLayout: Layouts are the same when they describe the same tree: buses by name, effects by slot, sends as a set, decibels within a hair" )
    try
    {
        const AUDIO::CAudioBusLayout layout = MakeSampleLayout();

        // another order of the buses, a decibel value a hair off, the master bus named as parent or not
        AUDIO::CAudioBusLayout other;
        const AUDIO::CAudioBusLayout::TBusVector& buses = layout.GetBuses();
        for ( size_t i=buses.size(); i>0; --i )
            other.GetBuses().push_back( buses[ i - 1 ] );
        ASSERT_TRUE( other == layout );
        other.FindBus( "music" )->volumeInDecibels += 0.0004f;
        other.FindBus( "music" )->parent = "";
        ASSERT_TRUE( other == layout && layout == other );

        // each of these makes it another tree
        AUDIO::CAudioBusLayout changed( layout );
        changed.FindBus( "music" )->volumeInDecibels += 0.01f;
        ASSERT_TRUE( changed != layout );
        changed = layout;
        changed.FindBus( "sfx" )->muted = true;
        ASSERT_TRUE( changed != layout );
        changed = layout;
        changed.FindBus( "ui" )->parent = "music";
        ASSERT_TRUE( changed != layout );
        changed = layout;
        changed.FindBus( "ui" )->sends[ 0 ].preFader = false;
        ASSERT_TRUE( changed != layout );
        changed = layout;
        changed.FindBus( "ui" )->sends[ 0 ].levelInDecibels = -4.0f;
        ASSERT_TRUE( changed != layout );
        changed = layout;
        changed.FindBus( "sfx" )->effects[ 0 ].SetParameter( "ratio", 4.0f );
        ASSERT_TRUE( changed != layout );
        changed = layout;
        changed.FindBus( "sfx" )->effects[ 1 ].SetOption( "bands", (Int32) 10 );
        ASSERT_TRUE( changed != layout );
        changed = layout;
        changed.FindBus( "sfx" )->effects[ 1 ].slot = 3;
        ASSERT_TRUE( changed != layout );
        changed = layout;
        changed.FindBus( "sfx" )->effects[ 0 ].wet = 0.5f;
        ASSERT_TRUE( changed != layout );
        changed = layout;
        changed.FindBus( "sfx" )->effects[ 0 ].enabled = false;
        ASSERT_TRUE( changed != layout );
        changed = layout;
        changed.FindBus( "sfx" )->effects[ 0 ].typeName = "limiter";
        ASSERT_TRUE( changed != layout );
        changed = layout;
        changed.AddBus( "extra" );
        ASSERT_TRUE( changed != layout && layout != changed );

        // the same number written another way is the same parameter
        changed = layout;
        changed.FindBus( "sfx" )->effects[ 0 ].SetParameter( "ratio", CORE::CString( "3.0" ) );
        ASSERT_TRUE( changed == layout );

        // sends are a set: the order does not matter, the same send twice is not the same as once
        AUDIO::CAudioBusLayout sends;
        AUDIO::CAudioBusLayout::SBus& bus = sends.AddBus( "a" );
        bus.AddSend( "b", -1.0f ).preFader = true;
        bus.AddSend( "c", -2.0f );
        AUDIO::CAudioBusLayout swapped;
        AUDIO::CAudioBusLayout::SBus& swappedBus = swapped.AddBus( "a" );
        swappedBus.AddSend( "c", -2.0f );
        swappedBus.AddSend( "b", -1.0f ).preFader = true;
        ASSERT_TRUE( sends == swapped );
        swappedBus.sends[ 0 ] = swappedBus.sends[ 1 ];
        ASSERT_TRUE( sends != swapped );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBusLayout: A layout is written to a config section and read back, also through JSON, and a damaged one is read as far as it goes" )
    try
    {
        const bool codecThere = LoadJsonCodecPlugin();
        const AUDIO::CAudioBusLayout layout = MakeSampleLayout();
        const CORE::CDataNodeSerializableSettings settings;

        CORE::CDataNode config( "Config" );
        CORE::CDataNode* section = config.AddChild( AUDIO::CAudioBusLayout::ConfigSectionName );
        ASSERT_TRUE( layout.Serialize( *section, settings ) );
        ASSERT_TRUE( 5 == section->GetNrOfDirectChildNodes() );
        const CORE::CDataNode* sfx = section->GetChildAtIndex( 2 );
        ASSERT_TRUE( GUCEF_NULL != sfx && "sfx" == sfx->GetAttributeValue( "name" ).AsString() && 3 == sfx->GetNrOfDirectChildNodes() );
        ASSERT_TRUE( GUCEF_NULL != sfx->FindChild( "Send" ) && "reverb" == sfx->FindChild( "Send" )->GetAttributeValue( "target" ).AsString() );
        const CORE::CDataNode* compressor = sfx->FindChild( "Effect" );
        ASSERT_TRUE( GUCEF_NULL != compressor && "compressor" == compressor->GetAttributeValue( "type" ).AsString() );
        ASSERT_TRUE( GUCEF_NULL != compressor->FindChild( "Parameters" ) && 1 == compressor->GetNrOfDirectChildNodes() );

        // from the node with the section, from the section, and written twice to the same node
        AUDIO::CAudioBusLayout fromRoot, fromSection;
        ASSERT_TRUE( fromRoot.Deserialize( config, settings ) && fromRoot == layout );
        ASSERT_TRUE( fromSection.Deserialize( *section, settings ) && fromSection == layout );
        ASSERT_TRUE( layout.Serialize( *section, settings ) && 5 == section->GetNrOfDirectChildNodes() );

        // through the clone interface
        CORE::CICloneable* clone = layout.Clone();
        ASSERT_TRUE( GUCEF_NULL != clone && static_cast< AUDIO::CAudioBusLayout* >( clone )->operator==( layout ) );
        GUCEF_DELETE clone;

        if ( codecThere )
        {
            CORE::CDataNode parsed;
            ASSERT_TRUE( TryThroughJson( config, parsed ) );
            AUDIO::CAudioBusLayout fromJson;
            ASSERT_TRUE( fromJson.Deserialize( parsed, settings ) );
            ASSERT_TRUE( fromJson == layout );
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioBusLayout: the json codec is not there, the JSON part of the test is skipped" );
        }

        // a bus without a name and an effect without a type are left out, the rest is read
        CORE::CDataNode damaged( AUDIO::CAudioBusLayout::ConfigSectionName );
        damaged.AddChild( "Bus" )->SetAttribute( "volumeDb", -3.0f );
        CORE::CDataNode* good = damaged.AddChild( "Bus" );
        good->SetAttribute( "name", "good" );
        good->AddChild( "Effect" )->SetAttribute( "slot", (UInt32) 1 );
        good->AddChild( "Send" )->SetAttribute( "levelDb", -3.0f );
        CORE::CDataNode* kept = good->AddChild( "Effect" );
        kept->SetAttribute( "type", "limiter" );
        kept->SetAttribute( "slot", (UInt32) 4 );
        AUDIO::CAudioBusLayout partial;
        ASSERT_FALSE( partial.Deserialize( damaged, settings ) );
        ASSERT_TRUE( 1 == partial.GetBuses().size() && 1 == partial.FindBus( "good" )->effects.size() && 4 == partial.FindBus( "good" )->effects[ 0 ].slot );
        ASSERT_TRUE( partial.FindBus( "good" )->sends.empty() );
        ASSERT_TRUE( partial.FindBus( "good" )->effects[ 0 ].enabled && 1.0f == partial.FindBus( "good" )->effects[ 0 ].wet && 0.0f == partial.FindBus( "good" )->effects[ 0 ].dry );
        ASSERT_TRUE( 0.0f == partial.FindBus( "good" )->volumeInDecibels && !partial.FindBus( "good" )->muted );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformEngineLayoutTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioBusLayout: What GetBusLayout captures from a set-up made by hand is the layout of that set-up" )
    try
    {
        AUDIO::CAudioEngine engine;
        AUDIO::CAudioBusLayout layout;
        ASSERT_FALSE( engine.GetBusLayout( layout ) && !layout.IsEmpty() );                // not running

        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        ASSERT_TRUE( engine.GetBusLayout( layout ) && 1 == layout.GetBuses().size() && "master" == layout.GetBuses()[ 0 ].name );
        ASSERT_TRUE( layout.GetBuses()[ 0 ].parent.IsNULLOrEmpty() && layout.GetBuses()[ 0 ].effects.empty() && layout.GetBuses()[ 0 ].sends.empty() );

        ASSERT_TRUE( BuildByHand( engine ) );
        ASSERT_TRUE( engine.GetBusLayout( layout ) );
        ASSERT_TRUE( 5 == layout.GetBuses().size() );
        ASSERT_TRUE( "master" == layout.GetBuses()[ 0 ].name && "music" == layout.GetBuses()[ 1 ].name && "reverb" == layout.GetBuses()[ 4 ].name );

        // the numbers of the sample layout are in it (the layout is in dB, the engine holds linear factors: a hair of difference is fine)
        ASSERT_TRUE( Near( -6.0, layout.FindBus( "music" )->volumeInDecibels, 1e-3 ) && Near( -3.5, layout.FindBus( "sfx" )->volumeInDecibels, 1e-3 ) );
        ASSERT_TRUE( Near( 9000.0, layout.FindBus( "master" )->effects[ 0 ].config.FindChild( "Parameters" )->GetAttributeValue( "frequency" ).AsFloat32(), 1e-3 ) );
        ASSERT_TRUE( "lowPass" == layout.FindBus( "master" )->effects[ 0 ].config.FindChild( "Parameters" )->GetAttributeValue( "type" ).AsString() );

        // and has what the sample layout has not written down: all parameters, construction options, enumerations by name
        const AUDIO::CAudioBusLayout::SBus* sfx = layout.FindBus( "sfx" );
        ASSERT_TRUE( GUCEF_NULL != sfx && 2 == sfx->effects.size() && 1 == sfx->sends.size() );
        ASSERT_TRUE( 0 == sfx->effects[ 0 ].slot && "compressor" == sfx->effects[ 0 ].typeName && 2 == sfx->effects[ 1 ].slot );
        const CORE::CDataNode* parameters = sfx->effects[ 0 ].config.FindChild( "Parameters" );
        ASSERT_TRUE( GUCEF_NULL != parameters && 9 == parameters->GetAttCount() && "peak" == parameters->GetAttributeValue( "detector" ).AsString() );
        ASSERT_TRUE( Near( 3.0, parameters->GetAttributeValue( "ratio" ).AsFloat32(), 1e-6 ) );
        ASSERT_TRUE( GUCEF_NULL == sfx->effects[ 0 ].config.FindChild( "Options" ) );
        const CORE::CDataNode* options = sfx->effects[ 1 ].config.FindChild( "Options" );
        ASSERT_TRUE( GUCEF_NULL != options && 12 == options->GetAttributeValue( "bands" ).AsInt32() );
        ASSERT_TRUE( !sfx->effects[ 1 ].enabled && Near( 0.5, sfx->effects[ 1 ].wet, 1e-6 ) && Near( 0.5, sfx->effects[ 1 ].dry, 1e-6 ) );
        ASSERT_TRUE( sfx->effects[ 0 ].enabled && 1.0f == sfx->effects[ 0 ].wet && 0.0f == sfx->effects[ 0 ].dry );
        ASSERT_TRUE( Near( -10.0, sfx->sends[ 0 ].levelInDecibels, 1e-3 ) && !sfx->sends[ 0 ].preFader && "reverb" == sfx->sends[ 0 ].target );
        ASSERT_TRUE( layout.FindBus( "ui" )->sends[ 0 ].preFader && "sfx" == layout.FindBus( "ui" )->parent );

        // the effect in a slot, as the owner side sees it
        ASSERT_FALSE( engine.GetBusEffect( engine.FindBus( "sfx" ), 0 ).IsNULL() );
        ASSERT_TRUE( engine.GetBusEffect( engine.FindBus( "sfx" ), 1 ).IsNULL() && engine.GetBusEffect( engine.FindBus( "sfx" ), 99 ).IsNULL() );
        ASSERT_TRUE( engine.GetBusEffect( GUCEF_NULL, 0 ).IsNULL() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBusLayout: A layout captured, written to a config section, read back and applied to a fresh engine gives the same layout" )
    try
    {
        AUDIO::CAudioEngine original;
        ASSERT_TRUE( original.Initialize( NullParams() ) && BuildByHand( original ) );
        AUDIO::CAudioBusLayout captured;
        ASSERT_TRUE( original.GetBusLayout( captured ) );

        const CORE::CDataNodeSerializableSettings settings;
        CORE::CDataNode config( "Config" );
        ASSERT_TRUE( captured.Serialize( *config.AddChild( AUDIO::CAudioBusLayout::ConfigSectionName ), settings ) );
        AUDIO::CAudioBusLayout viaNode;
        ASSERT_TRUE( viaNode.Deserialize( config, settings ) && viaNode == captured );

        AUDIO::CAudioBusLayout viaJson;
        if ( LoadJsonCodecPlugin() )
        {
            CORE::CDataNode parsed;
            ASSERT_TRUE( TryThroughJson( config, parsed ) && viaJson.Deserialize( parsed, settings ) );
            ASSERT_TRUE( viaJson == captured );
        }
        else
            viaJson = viaNode;

        const AUDIO::CAudioBusLayout* routes[ 3 ] = { &captured, &viaNode, &viaJson };
        for ( int r=0; r<3; ++r )
        {
            AUDIO::CAudioEngine fresh;
            ASSERT_TRUE( fresh.Initialize( NullParams() ) );
            CORE::CFutureResult applied = fresh.ApplyBusLayout( *routes[ r ] );
            ASSERT_FALSE( applied.HasNoFuture() );
            ASSERT_FALSE( applied.IsReady() );                                          // the audio thread has not run yet
            ASSERT_TRUE( RenderUntilReady( fresh, applied ) && !applied.HasNoFuture() );

            AUDIO::CAudioBusLayout again;
            ASSERT_TRUE( fresh.GetBusLayout( again ) );
            ASSERT_TRUE( again == captured );
            ASSERT_TRUE( 5 == again.GetBuses().size() && "music" == again.GetBuses()[ 1 ].name && "ui" == again.GetBuses()[ 3 ].name );

            // the effects are the ones of the layout, with their construction options
            AUDIO::TAudioEffectPtr equalizer = fresh.GetBusEffect( fresh.FindBus( "sfx" ), 2 );
            ASSERT_FALSE( equalizer.IsNULL() );
            ASSERT_TRUE( "equalizer" == equalizer->GetEffectTypeName() );
            CORE::CValueList equalizerOptions, delayOptions;
            equalizer->GetConstructionOptions( equalizerOptions );
            ASSERT_TRUE( 12 == equalizerOptions.GetValueAlways( "bands", "" ).AsInt32() );
            AUDIO::TAudioEffectPtr delay = fresh.GetBusEffect( fresh.FindBus( "reverb" ), 0 );
            ASSERT_FALSE( delay.IsNULL() );
            delay->GetConstructionOptions( delayOptions );
            ASSERT_TRUE( Near( 1000.0, delayOptions.GetValueAlways( "maxDelay", "" ).AsFloat32(), 1e-3 ) );
            ASSERT_TRUE( Near( 0.4, GetParameter( delay, "feedback" ), 1e-6 ) && Near( 40.0, GetParameter( delay, "time" ), 1e-6 ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBusLayout: A layout applied to a fresh engine renders the same bits as the set-up made by hand" )
    try
    {
        AUDIO::CAudioEngine byHand;
        ASSERT_TRUE( byHand.Initialize( NullParams() ) && BuildByHand( byHand ) );
        AUDIO::CAudioEngine byLayout;
        ASSERT_TRUE( byLayout.Initialize( NullParams() ) );
        ASSERT_TRUE( ApplyAndWait( byLayout, MakeSampleLayout() ) );

        // the same tree, down to every parameter of every effect
        AUDIO::CAudioBusLayout handLayout, builtLayout;
        ASSERT_TRUE( byHand.GetBusLayout( handLayout ) && byLayout.GetBusLayout( builtLayout ) && handLayout == builtLayout );

        TSamples handOutput, layoutOutput;
        ASSERT_TRUE( RenderScenario( byHand, handOutput ) && RenderScenario( byLayout, layoutOutput ) );
        ASSERT_TRUE( handOutput.size() == layoutOutput.size() && handOutput.size() == 100 * 256 * 2 );
        ASSERT_TRUE( PeakOf( handOutput ) > 0.2 && PeakOf( handOutput ) < 1.5 );             // something was heard
        ASSERT_TRUE( 0 == memcmp( &handOutput[ 0 ], &layoutOutput[ 0 ], handOutput.size() * sizeof( Float32 ) ) );

        // a layout that was captured from the set-up (volumes went through linear factors and back) renders the same within a hair
        AUDIO::CAudioBusLayout captured;
        ASSERT_TRUE( byHand.GetBusLayout( captured ) );
        AUDIO::CAudioEngine fromCapture;
        ASSERT_TRUE( fromCapture.Initialize( NullParams() ) && ApplyAndWait( fromCapture, captured ) );
        TSamples captureOutput;
        ASSERT_TRUE( RenderScenario( fromCapture, captureOutput ) );
        ASSERT_TRUE( LargestDifference( handOutput, captureOutput ) < 1.0e-4 );

        // and a layout that is not the same sounds different
        AUDIO::CAudioBusLayout quieter = MakeSampleLayout();
        quieter.FindBus( "sfx" )->volumeInDecibels = -20.0f;
        AUDIO::CAudioEngine changed;
        ASSERT_TRUE( changed.Initialize( NullParams() ) && ApplyAndWait( changed, quieter ) );
        TSamples changedOutput;
        ASSERT_TRUE( RenderScenario( changed, changedOutput ) );
        ASSERT_TRUE( LargestDifference( handOutput, changedOutput ) > 0.01 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBusLayout: Applying the same layout again changes nothing: no commands, the same effects in the slots, the same layout" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        const AUDIO::CAudioBusLayout layout = MakeSampleLayout();
        ASSERT_TRUE( ApplyAndWait( engine, layout ) );

        AUDIO::CAudioBusLayout before;
        ASSERT_TRUE( engine.GetBusLayout( before ) );
        AUDIO::CAudioBus* sfx = engine.FindBus( "sfx" );
        AUDIO::CAudioBus* reverb = engine.FindBus( "reverb" );
        AUDIO::CAudioBus* master = engine.GetMasterBus();
        AUDIO::CAudioEffect* compressor = engine.GetBusEffect( sfx, 0 ).GetPointerAlways();
        AUDIO::CAudioEffect* equalizer = engine.GetBusEffect( sfx, 2 ).GetPointerAlways();
        AUDIO::CAudioEffect* delay = engine.GetBusEffect( reverb, 0 ).GetPointerAlways();
        AUDIO::CAudioEffect* filter = engine.GetBusEffect( master, 1 ).GetPointerAlways();
        ASSERT_TRUE( GUCEF_NULL != compressor && GUCEF_NULL != equalizer && GUCEF_NULL != delay && GUCEF_NULL != filter );

        for ( int again=0; again<2; ++again )
        {
            // nothing was sent to the audio thread, so there is nothing to wait for: no Update, no Render
            CORE::CFutureResult second = engine.ApplyBusLayout( layout );
            ASSERT_FALSE( second.HasNoFuture() );
            ASSERT_TRUE( second.IsReady() );
            AUDIO::CAudioBusLayout after;
            ASSERT_TRUE( engine.GetBusLayout( after ) && after == before );
            ASSERT_TRUE( engine.FindBus( "sfx" ) == sfx && engine.FindBus( "reverb" ) == reverb );
            ASSERT_TRUE( engine.GetBusEffect( sfx, 0 ).GetPointerAlways() == compressor && engine.GetBusEffect( sfx, 2 ).GetPointerAlways() == equalizer );
            ASSERT_TRUE( engine.GetBusEffect( reverb, 0 ).GetPointerAlways() == delay && engine.GetBusEffect( master, 1 ).GetPointerAlways() == filter );
        }

        // what GetBusLayout gave is a layout of the same tree too: applied to the engine it came from it does nothing either
        CORE::CFutureResult captured = engine.ApplyBusLayout( before );
        ASSERT_TRUE( !captured.HasNoFuture() && captured.IsReady() );
        ASSERT_TRUE( engine.GetBusEffect( sfx, 0 ).GetPointerAlways() == compressor );
        ASSERT_TRUE( 4 == sfx->GetSendSlotCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBusLayout: A layout updates what it names: kept effects keep their state, others are replaced, removed or inserted; what it does not name stays" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        ASSERT_TRUE( ApplyAndWait( engine, MakeSampleLayout() ) );

        // a bus the next layout will not know about
        AUDIO::CAudioBus* extra = engine.CreateBus( "extra" );
        ASSERT_TRUE( GUCEF_NULL != extra );
        extra->SetVolumeInDecibels( -20.0f );
        AUDIO::TAudioEffectPtr extraEffect = engine.CreateEffect( "limiter" );
        ASSERT_TRUE( RenderUntilReady( engine, engine.InsertEffect( extra, 4, extraEffect ) ) );
        ASSERT_TRUE( RenderUntilReady( engine, engine.AddSend( extra, engine.FindBus( "reverb" ), 0.5f, true ) ) );
        AUDIO::CAudioBusLayout extraBefore;
        ASSERT_TRUE( engine.GetBusLayout( extraBefore ) );

        AUDIO::CAudioBus* sfx = engine.FindBus( "sfx" );
        AUDIO::CAudioBus* reverb = engine.FindBus( "reverb" );
        AUDIO::TAudioEffectPtr compressor = engine.GetBusEffect( sfx, 0 );
        AUDIO::TAudioEffectPtr oldEqualizer = engine.GetBusEffect( sfx, 2 );
        AUDIO::TAudioEffectPtr delay = engine.GetBusEffect( reverb, 0 );
        SetParameter( compressor, "knee", 12.0f );                   // changed on the effect: a layout that does not name it puts it back

        AUDIO::CAudioBusLayout next;
        AUDIO::CAudioBusLayout::SBus& master = next.AddBus( "master" );
        master.AddEffect( 3, "limiter" ).SetParameter( "ceiling", -3.0f );
        // a child listed before its parent, a new bus below a new bus
        AUDIO::CAudioBusLayout::SBus& voice = next.AddBus( "voice", "chat", -4.0f );
        voice.AddSend( "reverb", -12.0f, false );
        next.AddBus( "chat", "sfx", -1.0f, true );
        AUDIO::CAudioBusLayout::SBus& nextSfx = next.AddBus( "sfx", "master", -3.5f );
        AUDIO::CAudioBusLayout::SEffect& nextCompressor = nextSfx.AddEffect( 0, "compressor" );
        nextCompressor.SetParameter( "ratio", 6.0f );                                  // threshold is not named: back to -18
        nextSfx.AddEffect( 2, "filter" ).SetParameter( "frequency", 2000.0f );        // another type in the slot of the equalizer
        nextSfx.AddSend( "reverb", -6.0f, false );                                    // the same send, another level
        nextSfx.AddSend( "reverb", -9.0f, true );                                     // one more, before the fader
        AUDIO::CAudioBusLayout::SBus& nextReverb = next.AddBus( "reverb", "master" ); // no effects, no sends: the delay goes
        (void) nextReverb;

        CORE::CFutureResult applied = engine.ApplyBusLayout( next );
        ASSERT_FALSE( applied.HasNoFuture() );
        // what does not need the audio thread is done when the call returns
        ASSERT_TRUE( GUCEF_NULL != engine.FindBus( "voice" ) && GUCEF_NULL != engine.FindBus( "chat" ) && engine.FindBus( "voice" )->GetParent() == engine.FindBus( "chat" ) );
        ASSERT_TRUE( engine.FindBus( "chat" )->GetParent() == sfx && engine.FindBus( "chat" )->IsMuted() );
        ASSERT_TRUE( RenderUntilReady( engine, applied ) && !applied.HasNoFuture() );

        // the compressor stayed (same object) and got the configuration, the rest of its parameters are as a new one has them
        ASSERT_TRUE( engine.GetBusEffect( sfx, 0 ).GetPointerAlways() == compressor.GetPointerAlways() );
        ASSERT_TRUE( Near( 6.0, GetParameter( compressor, "ratio" ), 1e-6 ) && Near( -18.0, GetParameter( compressor, "threshold" ), 1e-6 ) && Near( 6.0, GetParameter( compressor, "knee" ), 1e-6 ) );

        // the equalizer went and a filter took its slot, the delay is gone, the limiter is new
        AUDIO::TAudioEffectPtr filter = engine.GetBusEffect( sfx, 2 );
        ASSERT_TRUE( !filter.IsNULL() && "filter" == filter->GetEffectTypeName() && filter.GetPointerAlways() != oldEqualizer.GetPointerAlways() );
        ASSERT_TRUE( Near( 2000.0, GetParameter( filter, "frequency" ), 1e-3 ) );
        ASSERT_TRUE( engine.GetBusEffect( reverb, 0 ).IsNULL() );
        ASSERT_TRUE( engine.GetBusEffect( engine.GetMasterBus(), 1 ).IsNULL() );        // the filter that was in the master bus slot 1 is not in the layout
        AUDIO::TAudioEffectPtr limiter = engine.GetBusEffect( engine.GetMasterBus(), 3 );
        ASSERT_TRUE( !limiter.IsNULL() && "limiter" == limiter->GetEffectTypeName() && Near( -3.0, GetParameter( limiter, "ceiling" ), 1e-6 ) );

        AUDIO::CAudioBusLayout now;
        ASSERT_TRUE( engine.GetBusLayout( now ) );
        ASSERT_TRUE( 1 == now.FindBus( "master" )->effects.size() && 2 == now.FindBus( "sfx" )->sends.size() && now.FindBus( "reverb" )->effects.empty() );
        ASSERT_TRUE( 2 == now.FindBus( "sfx" )->effects.size() && 1 == now.FindBus( "voice" )->sends.size() );
        // the send that was there stays in its slot with the new level, the new one is in the next slot
        ASSERT_TRUE( Near( -6.0, now.FindBus( "sfx" )->sends[ 0 ].levelInDecibels, 1e-3 ) && !now.FindBus( "sfx" )->sends[ 0 ].preFader );
        ASSERT_TRUE( Near( -9.0, now.FindBus( "sfx" )->sends[ 1 ].levelInDecibels, 1e-3 ) && now.FindBus( "sfx" )->sends[ 1 ].preFader );
        ASSERT_TRUE( Near( -4.0, now.FindBus( "voice" )->volumeInDecibels, 1e-3 ) );

        // buses that the layout does not name: nothing about them changed, the effect of the extra bus is the same object
        const char* const unnamed[ 3 ] = { "ui", "music", "extra" };
        for ( int i=0; i<3; ++i )
        {
            ASSERT_TRUE( GUCEF_NULL != extraBefore.FindBus( unnamed[ i ] ) && GUCEF_NULL != now.FindBus( unnamed[ i ] ) );
            AUDIO::CAudioBusLayout a, b;
            a.GetBuses().push_back( *extraBefore.FindBus( unnamed[ i ] ) );
            b.GetBuses().push_back( *now.FindBus( unnamed[ i ] ) );
            ASSERT_TRUE( a == b );
        }
        ASSERT_TRUE( 1 == now.FindBus( "ui" )->sends.size() && 1 == now.FindBus( "extra" )->sends.size() );
        ASSERT_TRUE( engine.GetBusEffect( extra, 4 ).GetPointerAlways() == extraEffect.GetPointerAlways() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformRefusalTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioBusLayout: A layout that cannot be applied is refused whole, with the engine as it was" )
    try
    {
        AUDIO::CAudioEngine notRunning;
        ASSERT_TRUE( IsFailedWith( notRunning.ApplyBusLayout( MakeSampleLayout() ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );

        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        ASSERT_TRUE( ApplyAndWait( engine, MakeSampleLayout() ) );
        AUDIO::CAudioBus* extra = engine.CreateBus( "extra" );
        ASSERT_TRUE( RenderUntilReady( engine, engine.AddSend( extra, engine.FindBus( "sfx" ), 1.0f, false ) ) );     // extra feeds sfx

        AUDIO::CAudioBusLayout before;
        ASSERT_TRUE( engine.GetBusLayout( before ) );
        AUDIO::CAudioBus* sfx = engine.FindBus( "sfx" );
        AUDIO::TAudioEffectPtr compressor = engine.GetBusEffect( sfx, 0 );

        // a valid change comes first in every layout, so a layout that is applied in part would show
        struct SCase
        {
            const char* what;
            AUDIO::CAudioBusLayout layout;
        } cases[ 24 ];
        int n = 0;
        #define NEW_CASE( description ) { cases[ n ].what = description; cases[ n ].layout.AddBus( "sfx", "master", -30.0f ).AddEffect( 0, "compressor" ).SetParameter( "ratio", 9.0f ); ++n; }
        #define LAST cases[ n - 1 ].layout

        NEW_CASE( "unknown effect type" )
        LAST.AddBus( "x", "master" ).AddEffect( 0, "doesnotexist" );
        NEW_CASE( "unknown effect type in a slot of an existing bus" )
        LAST.AddBus( "music", "master" ).AddEffect( 0, "reverb9000" );
        NEW_CASE( "a send to the bus itself" )
        LAST.AddBus( "x", "master" ).AddSend( "x", 0.0f );
        NEW_CASE( "a send from a bus to its own child" )
        LAST.FindBus( "sfx" )->AddSend( "ui", 0.0f );                                             // ui is a child of sfx
        NEW_CASE( "a send to a bus that feeds it by a send of a bus the layout does not name" )
        LAST.FindBus( "sfx" )->AddSend( "extra", 0.0f );                                          // extra sends to sfx
        NEW_CASE( "two new buses that send to each other" )
        {
            LAST.AddBus( "a", "master" ).AddSend( "b", 0.0f );
            LAST.AddBus( "b", "master" ).AddSend( "a", 0.0f );
        }
        NEW_CASE( "a send to an unknown bus" )
        LAST.AddBus( "x", "master" ).AddSend( "nowhere", 0.0f );
        NEW_CASE( "a bus that would have to move" )
        LAST.AddBus( "music", "sfx" );
        NEW_CASE( "an unknown parent" )
        LAST.AddBus( "x", "nowhere" );
        NEW_CASE( "new buses that are each other's parent" )
        {
            LAST.AddBus( "a", "b" );
            LAST.AddBus( "b", "a" );
        }
        NEW_CASE( "a bus that is its own parent" )
        LAST.AddBus( "a", "a" );
        NEW_CASE( "a parent for the master bus" )
        LAST.AddBus( "master", "music" );
        NEW_CASE( "a name used twice" )
        {
            LAST.AddBus( "x", "master" );
            LAST.AddBus( "x", "master" );
        }
        NEW_CASE( "a bus without a name" )
        LAST.AddBus( "", "master" );
        NEW_CASE( "a slot beyond the slots of a bus" )
        LAST.AddBus( "x", "master" ).AddEffect( 8, "limiter" );
        NEW_CASE( "a slot used twice" )
        {
            AUDIO::CAudioBusLayout::SBus& x = LAST.AddBus( "x", "master" );
            x.AddEffect( 1, "limiter" );
            x.AddEffect( 1, "compressor" );
        }
        NEW_CASE( "more sends than a bus has slots" )
        {
            AUDIO::CAudioBusLayout::SBus& x = LAST.AddBus( "x", "master" );
            for ( int s=0; s<5; ++s )
                x.AddSend( "reverb", -3.0f );
        }
        NEW_CASE( "an effect with the configuration of another type" )
        LAST.AddBus( "x", "master" ).AddEffect( 0, "limiter" ).config.SetAttribute( "type", "compressor" );
        NEW_CASE( "an effect without a type" )
        LAST.AddBus( "x", "master" ).AddEffect( 0, "" );
        NEW_CASE( "an unknown target among valid sends" )
        {
            AUDIO::CAudioBusLayout::SBus& x = LAST.AddBus( "reverb", "master" );
            x.AddSend( "music", -3.0f );
            x.AddSend( "ghost", -3.0f );
        }
        #undef LAST
        #undef NEW_CASE

        for ( int c=0; c<n; ++c )
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AudioBusLayout: refusal case: " << cases[ c ].what );
            CORE::CFutureResult refused = engine.ApplyBusLayout( cases[ c ].layout );
            ASSERT_TRUE( IsFailedWith( refused, CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );

            // nothing changed, also not the valid part that came first, and nothing waits for the audio thread
            AUDIO::CAudioBusLayout after;
            ASSERT_TRUE( engine.GetBusLayout( after ) && after == before );
            ASSERT_TRUE( Near( -3.5, sfx->GetVolumeInDecibels(), 1e-3 ) && engine.GetBusEffect( sfx, 0 ).GetPointerAlways() == compressor.GetPointerAlways() );
            ASSERT_TRUE( Near( 3.0, GetParameter( compressor, "ratio" ), 1e-6 ) );
            ASSERT_TRUE( GUCEF_NULL == engine.FindBus( "x" ) && GUCEF_NULL == engine.FindBus( "a" ) && GUCEF_NULL == engine.FindBus( "b" ) );
        }
        ASSERT_TRUE( n >= 20 );

        // the engine went on as if nothing happened
        NullDevice( engine )->Render( 256 * 4 );
        engine.Update();
        AUDIO::CAudioBusLayout end;
        ASSERT_TRUE( engine.GetBusLayout( end ) && end == before );
        ASSERT_TRUE( 0 == engine.GetDroppedEventCount() );

        // too many buses for the engine
        AUDIO::CAudioEngineParams small = NullParams();
        small.SetMaxBuses( 4 );
        AUDIO::CAudioEngine smallEngine;
        ASSERT_TRUE( smallEngine.Initialize( small ) );
        AUDIO::CAudioBusLayout crowded;
        for ( int b=0; b<4; ++b )
            crowded.AddBus( b == 0 ? "a" : b == 1 ? "b" : b == 2 ? "c" : "d" );
        ASSERT_TRUE( IsFailedWith( smallEngine.ApplyBusLayout( crowded ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        ASSERT_TRUE( GUCEF_NULL == smallEngine.FindBus( "a" ) );
        crowded.GetBuses().pop_back();
        ASSERT_TRUE( ApplyAndWait( smallEngine, crowded ) && GUCEF_NULL != smallEngine.FindBus( "c" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBusLayout: A layout that needs more commands than the engine can have on their way is a throttle, and applies later in smaller parts" )
    try
    {
        AUDIO::CAudioEngineParams params = NullParams();
        params.SetEventQueueSize( 16 );                     // room for 5 requests on their way
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( params ) );

        AUDIO::CAudioBusLayout big;
        for ( int b=0; b<8; ++b )
            big.AddBus( CORE::CString( "bus" ) + CORE::UInt32ToString( (UInt32) b ), "master", -1.0f * (Float32) b );
        CORE::CFutureResult refused = engine.ApplyBusLayout( big );
        // a throttle: in an error status, and not an end state, it may clear
        ASSERT_TRUE( refused.HasNoFuture() && !refused.IsReady() );
        ASSERT_TRUE( GUCEF_NULL == engine.FindBus( "bus0" ) );

        AUDIO::CAudioBusLayout part;
        for ( int b=0; b<4; ++b )
            part.GetBuses().push_back( big.GetBuses()[ b ] );
        ASSERT_TRUE( ApplyAndWait( engine, part ) );
        for ( int b=4; b<8; ++b )
            part.GetBuses().push_back( big.GetBuses()[ b ] );
        ASSERT_TRUE( ApplyAndWait( engine, part ) );
        AUDIO::CAudioBusLayout captured;
        ASSERT_TRUE( engine.GetBusLayout( captured ) && 9 == captured.GetBuses().size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBusLayout: With the engine thread the future is completed there, and a shut down cancels what the audio thread did not get to" )
    try
    {
        {
            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( engine.Initialize( NullParams( true ) ) );
            ASSERT_TRUE( engine.IsThreaded() );
            CORE::CFutureResult applied = engine.ApplyBusLayout( MakeSampleLayout() );
            ASSERT_FALSE( applied.HasNoFuture() );
            for ( UInt32 i=0; i<400 && !applied.IsReady(); ++i )
            {
                NullDevice( engine )->Render( 256 );
                MT::PrecisionDelay( 1 );
            }
            ASSERT_TRUE( applied.IsReady() && !applied.HasNoFuture() );
            AUDIO::CAudioBusLayout captured;
            ASSERT_TRUE( engine.GetBusLayout( captured ) && 5 == captured.GetBuses().size() && 2 == captured.FindBus( "sfx" )->effects.size() );
        }

        CORE::CFutureResult orphan( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
        {
            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( engine.Initialize( NullParams() ) );
            orphan = engine.ApplyBusLayout( MakeSampleLayout() );
            ASSERT_FALSE( orphan.IsReady() );
            engine.Shutdown();
        }
        ASSERT_TRUE( orphan.IsReady() );
        CORE::CTaskPtr orphanTask = orphan.GetResult( 1000 );
        ASSERT_TRUE( !orphanTask.IsNULL() && CORE::TTaskStatus::TASKSTATUS_CANCELLED == orphanTask->GetTaskStatus() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAudioBusLayoutTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioBusLayout TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.BusLayout" );

    PerformEngineParamsTests();
    PerformLayoutValueTests();
    PerformEngineLayoutTests();
    PerformRefusalTests();
}
