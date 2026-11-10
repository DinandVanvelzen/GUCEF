/*
 *  gucefGUI_TestApp: GUCEF test application for the GUI module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEFAUDIO_H
#include "gucefAUDIO.h"
#define GUCEFAUDIO_H
#endif /* GUCEFAUDIO_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "gucefGUI_CScrollPanel.h"
#include "gucefGUI_CProgressBar.h"
#include "gucefGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/** Counts one event of one notifier */
class CAudioEventCount : public CORE::CObserver
{
    public:

    CORE::CEvent eventId;
    UInt32 count;

    CAudioEventCount( CORE::CNotifier* notifier, const CORE::CEvent& id )
        : CORE::CObserver(), eventId( id ), count( 0 )
    {
        SubscribeTo( notifier, id );
    }

    virtual ~CAudioEventCount() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( id == eventId )
            ++count;
    }
};

/** Counts the end of the reopens a page started and keeps how the last one ended */
class CReopenCompletedRecorder : public CORE::CObserver
{
    public:

    UInt32 count;
    bool lastSuccess;

    CReopenCompletedRecorder( CORE::CNotifier* notifier )
        : CORE::CObserver(), count( 0 ), lastSuccess( false )
    {
        SubscribeTo( notifier, GUI::CAudioSettingsForm::DeviceReopenCompletedEvent );
    }

    virtual ~CReopenCompletedRecorder() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( id == GUI::CAudioSettingsForm::DeviceReopenCompletedEvent )
        {
            ++count;
            const CORE::TCloneableBool* success = static_cast< const CORE::TCloneableBool* >( eventData );
            lastSuccess = GUCEF_NULL != success && success->GetData();
        }
    }
};

bool
Near( Float32 a, Float32 b, Float32 tolerance = 0.01f )
{
    return fabsf( a - b ) <= tolerance;
}

/** A context of the null driver in the dark theme, with the fixed width test font */
GUI::TGuiContextPtr
CreateAudioContext( Int32 width, Int32 height )
{
    GUI::TGuiContextPtr contextPtr = CreateNullContext();
    GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
    if ( GUCEF_NULL != context )
    {
        context->SetLogicalResolution( width, height );
        context->RegisterFont( "Test", MakeTestFont( 8, 10 ) );
        CORE::CDataNode theme;
        GUI::BuildDarkTheme( theme, "Test" );
        context->SetTheme( theme );
    }
    return contextPtr;
}

/** An engine on the null driver in cooperative mode that keeps what it plays, with the buses music and effects */
class CAudioFixture
{
    public:

    AUDIO::CAudioEngine engine;
    AUDIO::CAudioBus* music;
    AUDIO::CAudioBus* effects;

    CAudioFixture( void ) : engine(), music( GUCEF_NULL ), effects( GUCEF_NULL ) {}

    bool Start( void )
    {
        AUDIO::CAudioEngineParams params;
        params.SetDriverName( "null" );
        params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
        params.GetDeviceParams().SetBufferSizeInFrames( 256 );
        params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );
        if ( !engine.Initialize( params ) )
            return false;
        music = engine.CreateBus( "music" );
        effects = engine.CreateBus( "effects" );
        return GUCEF_NULL != music && GUCEF_NULL != effects;
    }

    AUDIO::CNullAudioDevice* Device( void )
    {
        AUDIO::TAudioDevicePtr device = engine.GetDevice();
        return device.IsNULL() ? GUCEF_NULL : device->AsNullDevice();
    }

    /** The audio thread does its work (the null device is driven by hand) and then the application side catches up */
    void Run( UInt32 frames )
    {
        if ( GUCEF_NULL != Device() )
            Device()->Render( frames );
        engine.Update();
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
    }
};

/**
 *  The application side of a cooperative engine and the frames of the GUI go on, until the page has no reopen pending.
 *  The page itself never waits: this loop is what a game loop does
 */
bool
RunUntilReopened( CAudioFixture& fixture, GUI::CGUIContext* context, GUI::CAudioSettingsForm* audio, UInt32 timeoutInMs = 10000 )
{
    for ( UInt32 waited=0; waited<timeoutInMs && audio->IsDeviceReopenPending(); ++waited )
    {
        fixture.Run( 256 );
        context->RenderFrame( 16 );
        if ( audio->IsDeviceReopenPending() )
            MT::ThreadDelay( 1 );
    }
    return !audio->IsDeviceReopenPending();
}

/** The level a meter shows in dB, a value outside the scale when there is no such meter */
Float32
MeterValue( GUI::CForm* form, const CORE::CString& name )
{
    GUI::CProgressBar* meter = form->GetWidgetAs< GUI::CProgressBar >( name );
    return GUCEF_NULL != meter ? meter->GetValue() : 1000.0f;
}

/** The page in a settings form of its own with the buses music and effects declared */
GUI::CAudioSettingsForm*
MakeAudioPage( GUI::CSettingsForm& form, bool declareBuses = true )
{
    GUI::CAudioSettingsForm* audio = static_cast< GUI::CAudioSettingsForm* >( form.AddPage( "Audio" ) );
    if ( GUCEF_NULL != audio && declareBuses )
    {
        audio->AddBusRow( "music", "Music" );
        audio->AddBusRow( "effects", "Sound effects" );
    }
    return audio;
}

CORE::CString
ComboText( GUI::CSettingsPageForm* page, const CORE::CString& name )
{
    GUI::CCombobox* combobox = page->GetWidgetAs< GUI::CCombobox >( name );
    return GUCEF_NULL != combobox ? combobox->GetText() : CORE::CString::Empty;
}

bool
ComboHas( GUI::CSettingsPageForm* page, const CORE::CString& name, const CORE::CString& item )
{
    GUI::CCombobox* combobox = page->GetWidgetAs< GUI::CCombobox >( name );
    if ( GUCEF_NULL == combobox )
        return false;
    GUI::CCombobox::TStringVector items;
    combobox->GetListItems( items );
    for ( UInt32 i=0; i<(UInt32)items.size(); ++i )
        if ( items[ i ] == item )
            return true;
    return false;
}

UInt32
ComboCount( GUI::CSettingsPageForm* page, const CORE::CString& name )
{
    GUI::CCombobox* combobox = page->GetWidgetAs< GUI::CCombobox >( name );
    GUI::CCombobox::TStringVector items;
    if ( GUCEF_NULL != combobox )
        combobox->GetListItems( items );
    return (UInt32) items.size();
}

bool
WidgetEnabled( GUI::CForm* form, const CORE::CString& name )
{
    GUI::CWidget* widget = form->GetWidget( name );
    return GUCEF_NULL != widget && widget->GetBoolProperty( "Enabled" );
}

bool
WidgetVisible( GUI::CForm* form, const CORE::CString& name )
{
    GUI::CWidget* widget = form->GetWidget( name );
    return GUCEF_NULL != widget && widget->IsVisible();
}

/** The user moves a slider to a percentage */
bool
SetSlider( GUI::CForm* form, const CORE::CString& name, UInt32 percent )
{
    GUI::CWidget* slider = form->GetWidget( name );
    return GUCEF_NULL != slider && slider->ChangeValueByUser( "Value", CORE::UInt32ToString( percent ) );
}

bool
SetCheck( GUI::CForm* form, const CORE::CString& name, bool checked )
{
    GUI::CWidget* check = form->GetWidget( name );
    return GUCEF_NULL != check && check->ChangeValueByUser( "Checked", checked ? "true" : "false" );
}

bool
PickIndex( GUI::CForm* form, const CORE::CString& name, Int32 index )
{
    GUI::CWidget* combo = form->GetWidget( name );
    return GUCEF_NULL != combo && combo->ChangeValueByUser( "SelectedIndex", CORE::Int32ToString( index ) );
}

void
Click( GUI::CGUIContext* context, GUI::CWidget* widget )
{
    context->FlushChanges();
    const GUI::CRect& rect = widget->GetRect();
    context->InjectMouseMove( rect.left + rect.width / 2, rect.top + rect.height / 2 );
    context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
    context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
}

/** True when some of the rendered audio is not silence */
bool
HeardSound( AUDIO::CNullAudioDevice* device, UInt32 fromFrame )
{
    if ( GUCEF_NULL == device )
        return false;
    const AUDIO::CAudioBuffer& heard = device->GetRenderedAudio();
    for ( UInt32 f=fromFrame; f<heard.GetFrameCount(); ++f )
        if ( fabsf( heard.GetSampleAsFloat( f, 0 ) ) > 0.0001f )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------*/

void
PerformGuiAudioSettingsFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.AudioSettings" );

    GUCEF_TESTFW_TESTCASE( "AudioSettings: The page type Audio is registered at start, gone after unregistering, and builds with the other pages" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            CORE::CString consoleLoggerEnv = CORE::GetEnv( "GUCEF_CONSOLE_LOGGER" );
            CORE::SetEnv( "GUCEF_CONSOLE_LOGGER", "none" );
            {
                GUI::CSettingsForm form( context );
                GUI::CSettingsPageForm* video = form.AddPage( "Video" );
                GUI::CSettingsPageForm* audio = form.AddPage( "Audio" );
                GUI::CSettingsPageForm* general = form.AddPage( "General" );
                ASSERT_TRUE( GUCEF_NULL != video && GUCEF_NULL != audio && GUCEF_NULL != general );
                ASSERT_TRUE( "GUCEF::GUI::CAudioSettingsForm" == audio->GetClassTypeName() && "Audio" == audio->GetTitle() );
                ASSERT_TRUE( audio == form.GetPage( "Audio" ) );
                ASSERT_TRUE( form.Build() && form.Open() );
                ASSERT_TRUE( "Video\nAudio\nGeneral" == form.GetWidget( "Pages" )->GetProperty( "Tabs" ) );
                form.Cancel();
            }

            GUI::CSettingsForm::UnregisterStandardPages();
            {
                GUI::CSettingsForm form( context );
                ASSERT_TRUE( GUCEF_NULL == form.AddPage( "Audio" ) );
            }
            GUI::CSettingsForm::RegisterStandardPages();
            {
                GUI::CSettingsForm form( context );
                ASSERT_TRUE( GUCEF_NULL != form.AddPage( "Audio" ) );
            }
            CORE::SetEnv( "GUCEF_CONSOLE_LOGGER", consoleLoggerEnv );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: ReadSettings shows the settings and the page has no changes, a changed widget is a change, Cancel puts back" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        CAudioFixture fixture;
        ASSERT_TRUE( fixture.Start() );
        if ( GUCEF_NULL != context )
        {
            AUDIO::CAudioSettings settings;
            settings.SetMasterVolumeInDecibels( AUDIO::CAudioSettings::PercentToDecibels( 80 ) );
            settings.SetBusVolumeInDecibels( "music", AUDIO::CAudioSettings::PercentToDecibels( 40 ) );
            settings.SetBusMuted( "music", true );
            settings.SetSampleRate( 44100 );
            settings.SetBufferSizeInFrames( 512 );
            settings.SetChannelLayout( AUDIO::CAudioFormat::GetDefaultChannelLayout( 2 ) );
            settings.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_OFF );

            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            ASSERT_TRUE( GUCEF_NULL != audio );
            audio->SetAudioSettings( settings );
            audio->SetAudioEngine( &fixture.engine );
            fixture.music->SetVolume( 0.3f );
            ASSERT_TRUE( form.Build() && form.Open() );

            ASSERT_TRUE( 80.0f == audio->GetWidget( "MasterVolume" )->GetFloatProperty( "Value" ) && "80 %" == audio->GetWidget( "MasterPercent" )->GetProperty( "Text" ) );
            ASSERT_FALSE( audio->GetWidget( "MasterMute" )->GetBoolProperty( "Checked" ) );
            ASSERT_TRUE( 40.0f == audio->GetWidget( "Bus0Volume" )->GetFloatProperty( "Value" ) && audio->GetWidget( "Bus0Mute" )->GetBoolProperty( "Checked" ) );
            ASSERT_TRUE( "Music" == audio->GetWidget( "Bus0Label" )->GetProperty( "Text" ) && "Sound effects" == audio->GetWidget( "Bus1Label" )->GetProperty( "Text" ) );

            // a bus the settings know nothing about shows what the engine has for it
            fixture.effects->SetVolumeInDecibels( AUDIO::CAudioSettings::PercentToDecibels( 25 ) );
            audio->ReadSettings();
            ASSERT_TRUE( 25.0f == audio->GetWidget( "Bus1Volume" )->GetFloatProperty( "Value" ) );

            ASSERT_TRUE( "44100 Hz" == ComboText( audio, "SampleRate" ) );
            ASSERT_TRUE( ComboText( audio, "BufferSize" ).HasSubstr( "512 frames, 11.6 ms" ) >= 0 );
            ASSERT_TRUE( "Stereo" == ComboText( audio, "SpeakerLayout" ) && "Off" == ComboText( audio, "Limiter" ) );
            ASSERT_TRUE( ComboText( audio, "Driver" ) == GUI::CAudioSettingsForm::AutomaticDriverChoice );
            ASSERT_TRUE( GUI::CAudioSettingsForm::DefaultDeviceChoice == ComboText( audio, "OutputDevice" ) );
            ASSERT_TRUE( GUI::CAudioSettingsForm::PlayTestSoundText == audio->GetWidget( "TestSound" )->GetProperty( "Text" ) );
            ASSERT_FALSE( audio->HasChanges() || form.HasChanges() );

            // the user moves the sliders and picks: the page and the form have changes, the engine does not
            ASSERT_TRUE( SetSlider( audio, "MasterVolume", 55 ) );
            ASSERT_TRUE( audio->HasChanges() && form.HasChanges() && WidgetEnabled( &form, "Apply" ) );
            ASSERT_TRUE( "55 %" == audio->GetWidget( "MasterPercent" )->GetProperty( "Text" ) );
            ASSERT_TRUE( SetCheck( audio, "Bus1Mute", true ) );
            ASSERT_TRUE( PickIndex( audio, "Limiter", 1 ) );
            ASSERT_TRUE( Near( 0.3f, fixture.music->GetVolume() ) && !fixture.effects->IsMuted() && Near( 1.0f, fixture.engine.GetMasterBus()->GetVolume() ) );
            ASSERT_TRUE( 55 == AUDIO::CAudioSettings::DecibelsToPercent( audio->GetSettingsFromWidgets().GetMasterVolumeInDecibels() ) );

            // Cancel: the widgets show the settings again, nothing reached the engine, nothing is changed
            form.Cancel();
            ASSERT_FALSE( audio->HasChanges() || form.HasChanges() );
            ASSERT_TRUE( 80.0f == audio->GetWidget( "MasterVolume" )->GetFloatProperty( "Value" ) && "80 %" == audio->GetWidget( "MasterPercent" )->GetProperty( "Text" ) );
            ASSERT_FALSE( audio->GetWidget( "Bus1Mute" )->GetBoolProperty( "Checked" ) );
            ASSERT_TRUE( "Off" == ComboText( audio, "Limiter" ) );
            ASSERT_TRUE( Near( 0.3f, fixture.music->GetVolume() ) && !fixture.effects->IsMuted() && Near( 1.0f, fixture.engine.GetMasterBus()->GetVolume() ) );
            ASSERT_TRUE( Near( AUDIO::CAudioSettings::PercentToDecibels( 80 ), audio->GetAudioSettings().GetMasterVolumeInDecibels() ) );

            audio->SetAudioEngine( GUCEF_NULL );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: Apply with volumes and mutes sets the engine and asks for no device reopen" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        CAudioFixture fixture;
        ASSERT_TRUE( fixture.Start() );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            ASSERT_TRUE( GUCEF_NULL != audio );
            audio->SetAudioEngine( &fixture.engine );
            CAudioEventCount reopen( audio, GUI::CAudioSettingsForm::DeviceReopenRequiredEvent );
            CAudioEventCount pageApplied( audio, GUI::CSettingsPageForm::SettingsAppliedEvent );
            CAudioEventCount formApplied( &form, GUI::CSettingsForm::SettingsAppliedEvent );
            ASSERT_TRUE( form.Build() && form.Open() );

            ASSERT_TRUE( SetSlider( audio, "MasterVolume", 50 ) );
            ASSERT_TRUE( SetSlider( audio, "Bus0Volume", 20 ) );
            ASSERT_TRUE( SetCheck( audio, "Bus0Mute", true ) );
            ASSERT_TRUE( SetCheck( audio, "MasterMute", true ) );
            ASSERT_TRUE( SetSlider( audio, "Bus1Volume", 0 ) );

            ASSERT_TRUE( form.Apply() );
            ASSERT_TRUE( 1 == pageApplied.count && 1 == formApplied.count && 0 == reopen.count );
            ASSERT_FALSE( audio->HasChanges() );

            AUDIO::CAudioBus* master = fixture.engine.GetMasterBus();
            ASSERT_TRUE( Near( -30.0f, master->GetVolumeInDecibels() ) && master->IsMuted() );
            ASSERT_TRUE( Near( -48.0f, fixture.music->GetVolumeInDecibels() ) && fixture.music->IsMuted() );
            ASSERT_TRUE( 0.0f == fixture.effects->GetVolume() && !fixture.effects->IsMuted() );

            const AUDIO::CAudioSettings& applied = audio->GetAudioSettings();
            ASSERT_TRUE( Near( -30.0f, applied.GetMasterVolumeInDecibels() ) && applied.IsMasterMuted() );
            AUDIO::CAudioSettings::SBusSettings bus;
            ASSERT_TRUE( applied.GetBusSettings( "music", bus ) && bus.muted && Near( -48.0f, bus.volumeInDecibels ) );
            ASSERT_TRUE( applied.GetBusSettings( "effects", bus ) && !bus.muted && AUDIO::CAudioSettings::SilenceInDecibels == bus.volumeInDecibels );

            // applying again without changes changes nothing, also not through rounding
            fixture.music->SetVolumeInDecibels( -47.5f );
            ASSERT_TRUE( audio->ApplySettings() );
            ASSERT_TRUE( Near( -48.0f, fixture.music->GetVolumeInDecibels() ) && 0 == reopen.count );
            form.Cancel();
            audio->SetAudioEngine( GUCEF_NULL );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: A volume of the settings that is not a whole percentage is not rounded by applying the page" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            AUDIO::CAudioSettings settings;
            settings.SetBusVolumeInDecibels( "music", -9.3f );
            settings.SetMasterVolumeInDecibels( -4.4f );
            audio->SetAudioSettings( settings );
            ASSERT_TRUE( form.Build() && form.Open() );

            // only the mute of the bus is touched
            ASSERT_TRUE( SetCheck( audio, "Bus0Mute", true ) );
            AUDIO::CAudioSettings fromWidgets = audio->GetSettingsFromWidgets();
            AUDIO::CAudioSettings::SBusSettings bus;
            ASSERT_TRUE( fromWidgets.GetBusSettings( "music", bus ) && bus.muted && Near( -9.3f, bus.volumeInDecibels ) );
            ASSERT_TRUE( Near( -4.4f, fromWidgets.GetMasterVolumeInDecibels() ) );
            ASSERT_FALSE( fromWidgets.GetBusSettings( "effects", bus ) );
            form.Cancel();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: Without a running engine Apply with a device level change asks for a device reopen, once, and the limiter stays unset until picked" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        CAudioFixture fixture;
        ASSERT_TRUE( fixture.Start() );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            audio->SetAudioEngine( &fixture.engine );
            fixture.engine.Shutdown();                  // an engine that does not run: the application opens the device
            CAudioEventCount reopen( audio, GUI::CAudioSettingsForm::DeviceReopenRequiredEvent );
            CAudioEventCount applied( audio, GUI::CSettingsPageForm::SettingsAppliedEvent );
            ASSERT_TRUE( form.Build() && form.Open() );

            // nothing picked of the device: applying sets nothing of it
            ASSERT_TRUE( SetSlider( audio, "MasterVolume", 90 ) );
            ASSERT_TRUE( form.Apply() );
            ASSERT_TRUE( 0 == reopen.count && !audio->GetAudioSettings().HasLimiterMode() );
            ASSERT_TRUE( 0 == audio->GetAudioSettings().GetSampleRate() && 0 == audio->GetAudioSettings().GetBufferSizeInFrames() );

            // the sample rate: item 0 is the device default, then 44100, 48000
            ASSERT_TRUE( PickIndex( audio, "SampleRate", 2 ) );
            ASSERT_TRUE( ComboText( audio, "SampleRate" ) == "48000 Hz" );
            ASSERT_TRUE( form.Apply() );
            ASSERT_TRUE( 1 == reopen.count && 2 == applied.count && 48000 == audio->GetAudioSettings().GetSampleRate() );

            // the same again: no change, no reopen
            ASSERT_TRUE( audio->ApplySettings() );
            ASSERT_TRUE( 1 == reopen.count );

            // buffer size, speakers and limiter each need one
            ASSERT_TRUE( PickIndex( audio, "BufferSize", 2 ) );
            ASSERT_TRUE( form.Apply() );
            ASSERT_TRUE( 2 == reopen.count && 256 == audio->GetAudioSettings().GetBufferSizeInFrames() );
            ASSERT_TRUE( ComboHas( audio, "SpeakerLayout", "5.1 surround" ) );
            ASSERT_TRUE( PickIndex( audio, "SpeakerLayout", 4 ) );
            ASSERT_TRUE( form.Apply() );
            ASSERT_TRUE( 3 == reopen.count && "5.1" == AUDIO::CAudioSettings::ChannelLayoutToString( audio->GetAudioSettings().GetChannelLayout() ) );
            ASSERT_TRUE( PickIndex( audio, "Limiter", 0 ) );
            ASSERT_TRUE( form.Apply() );
            ASSERT_TRUE( 4 == reopen.count && audio->GetAudioSettings().HasLimiterMode() && AUDIO::AUDIOLIMITERMODE_OFF == audio->GetAudioSettings().GetLimiterMode() );

            ASSERT_FALSE( audio->IsDeviceReopenPending() );
            form.Cancel();
            audio->SetAudioEngine( GUCEF_NULL );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: Without an engine Apply updates the settings, the test sound row is disabled and the bus rows are not" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            CAudioEventCount applied( audio, GUI::CSettingsPageForm::SettingsAppliedEvent );
            ASSERT_TRUE( form.Build() && form.Open() );

            ASSERT_TRUE( "unknown" == audio->GetWidget( "Latency" )->GetProperty( "Text" ) );
            ASSERT_FALSE( WidgetEnabled( audio, "TestSound" ) || WidgetEnabled( audio, "TestSignal" ) );
            ASSERT_TRUE( WidgetEnabled( audio, "Bus0Volume" ) && WidgetEnabled( audio, "Bus1Mute" ) && WidgetEnabled( audio, "MasterVolume" ) );
            ASSERT_FALSE( audio->PlayTestSound() || audio->IsTestSoundPlaying() );

            ASSERT_TRUE( SetSlider( audio, "Bus1Volume", 10 ) );
            ASSERT_TRUE( form.Apply() && 1 == applied.count );
            AUDIO::CAudioSettings::SBusSettings bus;
            ASSERT_TRUE( audio->GetAudioSettings().GetBusSettings( "effects", bus ) && Near( AUDIO::CAudioSettings::PercentToDecibels( 10 ), bus.volumeInDecibels ) );
            form.Cancel();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: Bus rows come in the order they were declared, a duplicate is refused, a bus the engine lacks is disabled" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        CAudioFixture fixture;
        ASSERT_TRUE( fixture.Start() );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form, false );
            ASSERT_TRUE( audio->AddBusRow( "effects", "Effects" ) && audio->AddBusRow( "voice", "Voice" ) && audio->AddBusRow( "music", "Music" ) );
            ASSERT_FALSE( audio->AddBusRow( "voice", "Voices" ) );
            ASSERT_FALSE( audio->AddBusRow( "", "Nothing" ) );
            audio->SetAudioEngine( &fixture.engine );
            ASSERT_TRUE( form.Build() && form.Open() );
            context->FlushChanges();

            ASSERT_FALSE( audio->AddBusRow( "late", "Late" ) );
            ASSERT_TRUE( "Effects" == audio->GetWidget( "Bus0Label" )->GetProperty( "Text" ) );
            ASSERT_TRUE( "Voice" == audio->GetWidget( "Bus1Label" )->GetProperty( "Text" ) );
            ASSERT_TRUE( "Music" == audio->GetWidget( "Bus2Label" )->GetProperty( "Text" ) );
            ASSERT_TRUE( GUCEF_NULL == audio->GetWidget( "Bus3Volume" ) );
            ASSERT_TRUE( audio->GetWidget( "Bus0Row" )->GetRect().top < audio->GetWidget( "Bus1Row" )->GetRect().top );
            ASSERT_TRUE( audio->GetWidget( "Bus1Row" )->GetRect().top < audio->GetWidget( "Bus2Row" )->GetRect().top );

            // the engine has no voice bus
            ASSERT_TRUE( WidgetEnabled( audio, "Bus0Volume" ) && WidgetEnabled( audio, "Bus2Mute" ) );
            ASSERT_FALSE( WidgetEnabled( audio, "Bus1Volume" ) || WidgetEnabled( audio, "Bus1Mute" ) );

            // settings of a bus without a row are kept
            AUDIO::CAudioSettings settings;
            settings.SetBusVolumeInDecibels( "ui", -6.0f );
            audio->SetAudioSettings( settings );
            audio->ReadSettings();
            ASSERT_TRUE( SetSlider( audio, "Bus0Volume", 60 ) );
            ASSERT_TRUE( form.Apply() );
            AUDIO::CAudioSettings::SBusSettings bus;
            ASSERT_TRUE( audio->GetAudioSettings().GetBusSettings( "ui", bus ) && Near( -6.0f, bus.volumeInDecibels ) );
            ASSERT_TRUE( Near( -24.0f, fixture.effects->GetVolumeInDecibels() ) );
            form.Cancel();
            audio->SetAudioEngine( GUCEF_NULL );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: The device lists show the default device and the device of the null driver, and the driver row is hidden with one driver" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            AUDIO::CAudioSettings settings;
            settings.SetDriverName( "null" );
            audio->SetAudioSettings( settings );
            ASSERT_TRUE( form.Build() && form.Open() );

            // only the null driver is there in this program
            ASSERT_FALSE( WidgetVisible( audio, "DriverRow" ) );
            ASSERT_TRUE( 2 == ComboCount( audio, "Driver" ) );
            ASSERT_TRUE( ComboHas( audio, "Driver", "Automatic" ) && ComboHas( audio, "Driver", "None (silent)" ) && "None (silent)" == ComboText( audio, "Driver" ) );
            ASSERT_TRUE( 2 == ComboCount( audio, "OutputDevice" ) );
            ASSERT_TRUE( ComboHas( audio, "OutputDevice", "Default device" ) && ComboHas( audio, "OutputDevice", "Null audio device" ) );
            ASSERT_TRUE( WidgetVisible( audio, "OutputDeviceRow" ) );

            // the rows below the hidden driver row start at the top
            context->FlushChanges();
            ASSERT_TRUE( audio->GetWidget( "OutputDeviceRow" )->GetRect().top == audio->GetWidget( "Body" )->GetRect().top + 8 );

            // the device default rate is the one of the device
            ASSERT_TRUE( ComboText( audio, "SampleRate" ).HasSubstr( "Device default (48000 Hz)" ) >= 0 );
            ASSERT_TRUE( ComboHas( audio, "BufferSize", "Driver default" ) && ComboHas( audio, "BufferSize", "256 frames, 5.3 ms" ) );

            // another sample rate changes the durations of the buffer sizes, the choice stays
            ASSERT_TRUE( PickIndex( audio, "BufferSize", 2 ) );
            ASSERT_TRUE( PickIndex( audio, "SampleRate", 1 ) );
            ASSERT_TRUE( "256 frames, 5.8 ms" == ComboText( audio, "BufferSize" ) );

            // the device list follows the driver: an unknown device of the settings stays in the list as it was
            form.Cancel();
            settings.SetDeviceId( "gone" );
            audio->SetAudioSettings( settings );
            ASSERT_TRUE( form.Open() );
            ASSERT_TRUE( "gone" == ComboText( audio, "OutputDevice" ) && 3 == ComboCount( audio, "OutputDevice" ) );
            ASSERT_TRUE( "gone" == audio->GetSettingsFromWidgets().GetDeviceId() );
            form.Cancel();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: The latency shows what the open device says, and the status shows a lost device after the next frame" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        CAudioFixture fixture;
        ASSERT_TRUE( fixture.Start() );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            audio->SetAudioEngine( &fixture.engine );
            ASSERT_TRUE( form.Build() && form.Open() );

            // the null device does not know its latency, the limiter of the engine default looks ahead
            ASSERT_TRUE( 0 == fixture.engine.GetDevice()->GetLatencyInFrames() && fixture.engine.GetMixLatencyInFrames() > 0 );
            ASSERT_TRUE( audio->GetWidget( "Latency" )->GetProperty( "Text" ).HasSubstr( "device unknown + limiter " ) == 0 );
            ASSERT_TRUE( audio->GetStatusText().IsNULLOrEmpty() );

            fixture.Device()->SimulateDeviceLoss();
            fixture.engine.Update();
            CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            ASSERT_TRUE( audio->GetStatusText().IsNULLOrEmpty() );      // the GUI thread has not had a frame yet
            context->RenderFrame( 16 );
            ASSERT_TRUE( GUI::CAudioSettingsForm::DeviceLostText == audio->GetStatusText() );

            // reading the settings again clears it
            audio->ReadSettings();
            ASSERT_TRUE( audio->GetStatusText().IsNULLOrEmpty() );
            context->RenderFrame( 16 );
            ASSERT_TRUE( audio->GetStatusText().IsNULLOrEmpty() );
            form.Cancel();
            audio->SetAudioEngine( GUCEF_NULL );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: The test sound plays on the engine, a second press stops it, it ends by itself and Cancel and Apply stop it" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        CAudioFixture fixture;
        ASSERT_TRUE( fixture.Start() );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            audio->SetAudioEngine( &fixture.engine );
            ASSERT_TRUE( form.Build() && form.Open() );
            ASSERT_TRUE( WidgetEnabled( audio, "TestSound" ) && WidgetEnabled( audio, "TestSignal" ) );
            ASSERT_TRUE( 4 == ComboCount( audio, "TestSignal" ) && ComboHas( audio, "TestSignal", "Tone ladder" ) && ComboHas( audio, "TestSignal", "Speaker check" ) );
            ASSERT_TRUE( ComboHas( audio, "TestSignal", "Sweep" ) && ComboHas( audio, "TestSignal", "Noise" ) );

            // the button starts a voice that is heard on the master bus
            ASSERT_TRUE( 0 == fixture.engine.GetActiveVoiceCount() );
            Click( context, audio->GetWidget( "TestSound" ) );
            ASSERT_TRUE( audio->IsTestSoundPlaying() && 1 == fixture.engine.GetActiveVoiceCount() );
            ASSERT_TRUE( GUI::CAudioSettingsForm::StopTestSoundText == audio->GetWidget( "TestSound" )->GetProperty( "Text" ) );
            ASSERT_FALSE( audio->PlayTestSound() );
            fixture.Run( 4800 );
            ASSERT_TRUE( HeardSound( fixture.Device(), 0 ) );

            // a second press stops it and it falls silent
            Click( context, audio->GetWidget( "TestSound" ) );
            ASSERT_FALSE( audio->IsTestSoundPlaying() );
            ASSERT_TRUE( GUI::CAudioSettingsForm::PlayTestSoundText == audio->GetWidget( "TestSound" )->GetProperty( "Text" ) );
            fixture.Run( 512 );
            ASSERT_TRUE( 0 == fixture.engine.GetActiveVoiceCount() );
            UInt32 silentFrom = fixture.Device()->GetRenderedAudio().GetFrameCount();
            fixture.Run( 2048 );
            ASSERT_FALSE( HeardSound( fixture.Device(), silentFrom ) );

            // the noise ends by itself (two seconds) and the button is back at its idle text after the next frame
            ASSERT_TRUE( PickIndex( audio, "TestSignal", 3 ) );
            ASSERT_TRUE( audio->PlayTestSound() );
            for ( UInt32 i=0; i<30 && audio->IsTestSoundPlaying(); ++i )
                fixture.Run( 4800 );
            ASSERT_FALSE( audio->IsTestSoundPlaying() );
            context->RenderFrame( 16 );
            ASSERT_TRUE( GUI::CAudioSettingsForm::PlayTestSoundText == audio->GetWidget( "TestSound" )->GetProperty( "Text" ) );
            ASSERT_TRUE( 0 == fixture.engine.GetActiveVoiceCount() );

            // the signals that need the speakers play as well
            for ( Int32 signal = 0; signal < 3; ++signal )
            {
                ASSERT_TRUE( PickIndex( audio, "TestSignal", signal ) );
                ASSERT_TRUE( audio->PlayTestSound() );
                fixture.Run( 480 );
                audio->StopTestSound();
                fixture.Run( 480 );
                ASSERT_TRUE( 0 == fixture.engine.GetActiveVoiceCount() );
            }

            // Cancel stops it
            ASSERT_TRUE( audio->PlayTestSound() );
            form.Cancel();
            ASSERT_FALSE( audio->IsTestSoundPlaying() );
            fixture.Run( 512 );
            ASSERT_TRUE( 0 == fixture.engine.GetActiveVoiceCount() );

            // and so does Apply
            ASSERT_TRUE( form.Open() );
            ASSERT_TRUE( audio->PlayTestSound() );
            ASSERT_TRUE( SetSlider( audio, "MasterVolume", 70 ) );
            ASSERT_TRUE( form.Apply() );
            ASSERT_FALSE( audio->IsTestSoundPlaying() );
            fixture.Run( 512 );
            ASSERT_TRUE( 0 == fixture.engine.GetActiveVoiceCount() );
            form.Cancel();
            audio->SetAudioEngine( GUCEF_NULL );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: The test sound plays on the bus the application names and on the master bus when the engine lacks it" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        CAudioFixture fixture;
        ASSERT_TRUE( fixture.Start() );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            audio->SetAudioEngine( &fixture.engine );
            ASSERT_TRUE( form.Build() && form.Open() );
            ASSERT_TRUE( audio->GetTestBus().IsNULLOrEmpty() );

            // the bus is muted: nothing is heard
            fixture.music->SetMuted( true );
            audio->SetTestBus( "music" );
            ASSERT_TRUE( "music" == audio->GetTestBus() && audio->PlayTestSound() );
            fixture.Run( 4800 );
            fixture.Run( 4800 );
            ASSERT_FALSE( HeardSound( fixture.Device(), 480 ) );
            audio->StopTestSound();
            fixture.Run( 512 );

            // a bus that is not there: the master bus
            audio->SetTestBus( "nosuchbus" );
            ASSERT_TRUE( audio->PlayTestSound() );
            UInt32 from = fixture.Device()->GetRenderedAudio().GetFrameCount();
            fixture.Run( 4800 );
            ASSERT_TRUE( HeardSound( fixture.Device(), from ) );
            audio->StopTestSound();
            fixture.Run( 512 );
            form.Cancel();
            audio->SetAudioEngine( GUCEF_NULL );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: The engine can be swapped and taken away without the page keeping its test sound" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            ASSERT_TRUE( form.Build() && form.Open() );
            {
                CAudioFixture first;
                ASSERT_TRUE( first.Start() );
                audio->SetAudioEngine( &first.engine );
                ASSERT_TRUE( audio->GetAudioEngine() == &first.engine );
                ASSERT_TRUE( WidgetEnabled( audio, "TestSound" ) );
                ASSERT_TRUE( audio->PlayTestSound() );
                audio->SetAudioEngine( GUCEF_NULL );
                ASSERT_FALSE( audio->IsTestSoundPlaying() );
                ASSERT_FALSE( WidgetEnabled( audio, "TestSound" ) );
                first.Run( 512 );
                ASSERT_TRUE( 0 == first.engine.GetActiveVoiceCount() );

                // a lost device of an engine the page is not attached to means nothing
                first.Device()->SimulateDeviceLoss();
                first.engine.Update();
                CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
                context->RenderFrame( 16 );
                ASSERT_TRUE( audio->GetStatusText().IsNULLOrEmpty() );
            }
            {
                // a page destroyed with a sound playing stops the sound
                CAudioFixture second;
                ASSERT_TRUE( second.Start() );
                {
                    GUI::CSettingsForm otherForm( context );
                    GUI::CAudioSettingsForm* other = MakeAudioPage( otherForm );
                    other->SetAudioEngine( &second.engine );
                    ASSERT_TRUE( otherForm.Build() && otherForm.Open() );
                    ASSERT_TRUE( other->PlayTestSound() );
                    ASSERT_TRUE( 1 == second.engine.GetActiveVoiceCount() );
                    otherForm.Cancel();
                    ASSERT_TRUE( other->PlayTestSound() );
                    other->SetAudioEngine( GUCEF_NULL );
                }
                second.Run( 512 );
                ASSERT_TRUE( 0 == second.engine.GetActiveVoiceCount() );
            }
            form.Cancel();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: Apply with a device level change reopens the device of a running engine without waiting, and the buses stay valid" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        CAudioFixture fixture;
        ASSERT_TRUE( fixture.Start() );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            audio->SetAudioEngine( &fixture.engine );
            CAudioEventCount required( audio, GUI::CAudioSettingsForm::DeviceReopenRequiredEvent );
            CAudioEventCount applied( audio, GUI::CSettingsPageForm::SettingsAppliedEvent );
            CReopenCompletedRecorder completed( audio );
            ASSERT_TRUE( form.Build() && form.Open() );
            ASSERT_FALSE( audio->IsDeviceReopenPending() );
            ASSERT_TRUE( 48000 == fixture.engine.GetMixFormat().GetSampleRate() );
            AUDIO::TAudioDevicePtr oldDevice = fixture.engine.GetDevice();

            // the sample rate: item 0 is the device default, then 44100, 48000
            ASSERT_TRUE( SetSlider( audio, "MasterVolume", 80 ) );
            ASSERT_TRUE( PickIndex( audio, "SampleRate", 1 ) );
            ASSERT_TRUE( ComboText( audio, "SampleRate" ) == "44100 Hz" );
            ASSERT_TRUE( form.Apply() );

            // started, not finished: the owner side of the engine has not had its turn, the page does not give it one
            ASSERT_TRUE( audio->IsDeviceReopenPending() );
            ASSERT_TRUE( GUI::CAudioSettingsForm::OpeningDeviceText == audio->GetStatusText() );
            ASSERT_TRUE( 0 == required.count && 1 == applied.count && 0 == completed.count );
            ASSERT_FALSE( WidgetEnabled( audio, "SampleRate" ) || WidgetEnabled( audio, "OutputDevice" ) || WidgetEnabled( audio, "BufferSize" ) );
            ASSERT_FALSE( WidgetEnabled( audio, "SpeakerLayout" ) || WidgetEnabled( audio, "Limiter" ) );
            ASSERT_TRUE( WidgetEnabled( audio, "MasterVolume" ) && WidgetEnabled( audio, "Bus0Volume" ) );
            ASSERT_TRUE( Near( AUDIO::CAudioSettings::PercentToDecibels( 80 ), fixture.engine.GetMasterBus()->GetVolumeInDecibels() ) );
            for ( UInt32 i=0; i<3; ++i )
                context->RenderFrame( 16 );
            ASSERT_TRUE( audio->IsDeviceReopenPending() && 0 == completed.count );
            ASSERT_TRUE( 48000 == fixture.engine.GetMixFormat().GetSampleRate() && fixture.engine.GetDevice() == oldDevice );

            // the application keeps the engine going, the frames see the end
            ASSERT_TRUE( RunUntilReopened( fixture, context, audio ) );
            ASSERT_TRUE( 1 == completed.count && completed.lastSuccess );
            ASSERT_TRUE( 0 == required.count && 1 == applied.count );
            ASSERT_TRUE( audio->GetStatusText().IsNULLOrEmpty() );
            ASSERT_TRUE( WidgetEnabled( audio, "SampleRate" ) && WidgetEnabled( audio, "OutputDevice" ) && WidgetEnabled( audio, "Limiter" ) );
            ASSERT_TRUE( 44100 == fixture.engine.GetMixFormat().GetSampleRate() && fixture.engine.GetDevice() != oldDevice );
            ASSERT_TRUE( 44100 == audio->GetAudioSettings().GetSampleRate() && ComboText( audio, "SampleRate" ) == "44100 Hz" );
            ASSERT_FALSE( audio->HasChanges() );
            ASSERT_TRUE( fixture.engine.FindBus( "music" ) == fixture.music && fixture.engine.FindBus( "effects" ) == fixture.effects );

            // and the engine plays on the new device, the page plays through it
            ASSERT_TRUE( audio->PlayTestSound() );
            fixture.Run( 4800 );
            ASSERT_TRUE( HeardSound( fixture.Device(), 0 ) && 1 == fixture.engine.GetActiveVoiceCount() );
            audio->StopTestSound();
            fixture.Run( 512 );

            // a volume only change after that asks for nothing
            ASSERT_TRUE( SetSlider( audio, "MasterVolume", 60 ) );
            ASSERT_TRUE( form.Apply() );
            ASSERT_FALSE( audio->IsDeviceReopenPending() );
            ASSERT_TRUE( 1 == completed.count && 0 == required.count && 2 == applied.count );

            // the limiter is part of the device settings as well
            ASSERT_TRUE( PickIndex( audio, "Limiter", 0 ) );
            ASSERT_TRUE( form.Apply() );
            ASSERT_TRUE( audio->IsDeviceReopenPending() );
            ASSERT_TRUE( RunUntilReopened( fixture, context, audio ) );
            ASSERT_TRUE( 2 == completed.count && completed.lastSuccess && ComboText( audio, "Limiter" ) == "Off" );
            form.Cancel();
            audio->SetAudioEngine( GUCEF_NULL );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: A reopen that fails puts the device rows and the settings back and says so, and the old device plays on" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        CAudioFixture fixture;
        ASSERT_TRUE( fixture.Start() );
        if ( GUCEF_NULL != context )
        {
            // a driver in the settings that this machine does not have: opening a device with it fails
            AUDIO::CAudioSettings settings;
            settings.SetDriverName( "nosuchdriver" );

            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            audio->SetAudioSettings( settings );
            audio->SetAudioEngine( &fixture.engine );
            CAudioEventCount required( audio, GUI::CAudioSettingsForm::DeviceReopenRequiredEvent );
            CReopenCompletedRecorder completed( audio );
            ASSERT_TRUE( form.Build() && form.Open() );
            AUDIO::TAudioDevicePtr oldDevice = fixture.engine.GetDevice();

            ASSERT_TRUE( SetSlider( audio, "Bus0Volume", 25 ) );
            ASSERT_TRUE( PickIndex( audio, "SampleRate", 1 ) );
            ASSERT_TRUE( PickIndex( audio, "BufferSize", 3 ) );
            ASSERT_TRUE( PickIndex( audio, "Limiter", 0 ) );
            ASSERT_TRUE( form.Apply() );
            ASSERT_TRUE( audio->IsDeviceReopenPending() && 44100 == audio->GetAudioSettings().GetSampleRate() );
            ASSERT_TRUE( RunUntilReopened( fixture, context, audio ) );

            // the page is as it was before the Apply as far as the device goes, and says what happened
            ASSERT_TRUE( 1 == completed.count && !completed.lastSuccess && 0 == required.count );
            ASSERT_TRUE( GUI::CAudioSettingsForm::DeviceOpenFailedText == audio->GetStatusText() );
            ASSERT_TRUE( 0 == audio->GetAudioSettings().GetSampleRate() && 0 == audio->GetAudioSettings().GetBufferSizeInFrames() );
            ASSERT_TRUE( ! audio->GetAudioSettings().HasLimiterMode() && "nosuchdriver" == audio->GetAudioSettings().GetDriverName() );
            ASSERT_TRUE( 0 == audio->GetWidgetAs< GUI::CCombobox >( "SampleRate" )->GetSelectedIndex() );
            ASSERT_TRUE( 0 == audio->GetWidgetAs< GUI::CCombobox >( "BufferSize" )->GetSelectedIndex() );
            ASSERT_TRUE( 2 == audio->GetWidgetAs< GUI::CCombobox >( "Limiter" )->GetSelectedIndex() );
            ASSERT_TRUE( ComboText( audio, "Driver" ) == "nosuchdriver" );
            ASSERT_TRUE( WidgetEnabled( audio, "SampleRate" ) && WidgetEnabled( audio, "BufferSize" ) && WidgetEnabled( audio, "Limiter" ) );
            ASSERT_FALSE( audio->IsDeviceReopenPending() || audio->HasChanges() );

            // what is not the device stays applied
            AUDIO::CAudioSettings::SBusSettings bus;
            ASSERT_TRUE( audio->GetAudioSettings().GetBusSettings( "music", bus ) && Near( AUDIO::CAudioSettings::PercentToDecibels( 25 ), bus.volumeInDecibels ) );
            ASSERT_TRUE( Near( AUDIO::CAudioSettings::PercentToDecibels( 25 ), fixture.music->GetVolumeInDecibels() ) );

            // the engine did not notice: same device and format, buses and sound
            ASSERT_TRUE( fixture.engine.GetDevice() == oldDevice && 48000 == fixture.engine.GetMixFormat().GetSampleRate() );
            ASSERT_TRUE( fixture.engine.FindBus( "music" ) == fixture.music );
            ASSERT_TRUE( audio->PlayTestSound() );
            fixture.Run( 4800 );
            ASSERT_TRUE( HeardSound( fixture.Device(), 0 ) );
            audio->StopTestSound();

            // the user tries again with the next Apply and the page asks for a reopen again
            ASSERT_TRUE( PickIndex( audio, "SampleRate", 2 ) );
            ASSERT_TRUE( form.Apply() && audio->IsDeviceReopenPending() );
            ASSERT_TRUE( RunUntilReopened( fixture, context, audio ) );
            ASSERT_TRUE( 2 == completed.count && !completed.lastSuccess && 0 == audio->GetAudioSettings().GetSampleRate() );
            form.Cancel();
            audio->SetAudioEngine( GUCEF_NULL );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: A reopen the engine refuses at once fails the Apply, and a reopen that is cancelled by a shutdown is a failed one" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        CAudioFixture fixture;
        ASSERT_TRUE( fixture.Start() );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            audio->SetAudioEngine( &fixture.engine );
            CAudioEventCount required( audio, GUI::CAudioSettingsForm::DeviceReopenRequiredEvent );
            CAudioEventCount applied( audio, GUI::CSettingsPageForm::SettingsAppliedEvent );
            CReopenCompletedRecorder completed( audio );
            ASSERT_TRUE( form.Build() && form.Open() );

            // the application has a reopen of its own going: the engine does one at a time
            AUDIO::CAudioEngineParams own( fixture.engine.GetParams() );
            own.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 32000, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );
            CORE::CFutureResult ownReopen = fixture.engine.ReopenDeviceAsync( own );
            ASSERT_FALSE( ownReopen.HasNoFuture() );

            ASSERT_TRUE( SetSlider( audio, "MasterVolume", 70 ) );
            ASSERT_TRUE( PickIndex( audio, "SampleRate", 2 ) );
            ASSERT_FALSE( audio->ApplySettings() );
            ASSERT_FALSE( audio->IsDeviceReopenPending() );
            ASSERT_TRUE( GUI::CAudioSettingsForm::DeviceOpenFailedText == audio->GetStatusText() );
            ASSERT_TRUE( 0 == audio->GetAudioSettings().GetSampleRate() && 0 == audio->GetWidgetAs< GUI::CCombobox >( "SampleRate" )->GetSelectedIndex() );
            ASSERT_TRUE( 0 == completed.count && 0 == required.count && 0 == applied.count );
            ASSERT_TRUE( Near( AUDIO::CAudioSettings::PercentToDecibels( 70 ), fixture.engine.GetMasterBus()->GetVolumeInDecibels() ) );
            while ( !ownReopen.IsReady() )
            {
                fixture.engine.Update();
                MT::ThreadDelay( 1 );
            }

            // the engine goes away while the reopen is pending: nothing to wait for, a failure to show
            ASSERT_TRUE( PickIndex( audio, "SampleRate", 2 ) );
            ASSERT_TRUE( audio->ApplySettings() && audio->IsDeviceReopenPending() );
            fixture.engine.Shutdown();
            context->RenderFrame( 16 );
            ASSERT_FALSE( audio->IsDeviceReopenPending() );
            ASSERT_TRUE( 1 == completed.count && !completed.lastSuccess && 0 == audio->GetAudioSettings().GetSampleRate() );
            ASSERT_TRUE( GUI::CAudioSettingsForm::DeviceOpenFailedText == audio->GetStatusText() );
            form.Cancel();
            audio->SetAudioEngine( GUCEF_NULL );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: A second Apply while a reopen is pending applies the volumes, refuses other device changes, and the first reopen completes" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        CAudioFixture fixture;
        ASSERT_TRUE( fixture.Start() );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            audio->SetAudioEngine( &fixture.engine );
            CAudioEventCount required( audio, GUI::CAudioSettingsForm::DeviceReopenRequiredEvent );
            CAudioEventCount applied( audio, GUI::CSettingsPageForm::SettingsAppliedEvent );
            CReopenCompletedRecorder completed( audio );
            ASSERT_TRUE( form.Build() && form.Open() );

            ASSERT_TRUE( PickIndex( audio, "SampleRate", 1 ) );
            ASSERT_TRUE( form.Apply() && audio->IsDeviceReopenPending() );
            ASSERT_TRUE( 1 == applied.count );

            // the volumes are not device settings: they are applied at once, the page stays pending
            ASSERT_TRUE( SetSlider( audio, "Bus1Volume", 33 ) );
            ASSERT_TRUE( SetCheck( audio, "Bus0Mute", true ) );
            ASSERT_TRUE( form.Apply() );
            ASSERT_TRUE( audio->IsDeviceReopenPending() && 2 == applied.count );
            ASSERT_TRUE( Near( AUDIO::CAudioSettings::PercentToDecibels( 33 ), fixture.effects->GetVolumeInDecibels() ) && fixture.music->IsMuted() );
            ASSERT_TRUE( GUI::CAudioSettingsForm::OpeningDeviceText == audio->GetStatusText() );

            // another device value, with the rows disabled this can only be done by a program: not taken, no second reopen
            ASSERT_TRUE( PickIndex( audio, "BufferSize", 3 ) );
            ASSERT_FALSE( audio->ApplySettings() );
            ASSERT_TRUE( GUI::CAudioSettingsForm::DeviceBusyText == audio->GetStatusText() );
            ASSERT_TRUE( audio->IsDeviceReopenPending() && 44100 == audio->GetAudioSettings().GetSampleRate() && 0 == audio->GetAudioSettings().GetBufferSizeInFrames() );
            ASSERT_TRUE( 0 == audio->GetWidgetAs< GUI::CCombobox >( "BufferSize" )->GetSelectedIndex() );
            ASSERT_TRUE( 2 == applied.count && 0 == required.count && 0 == completed.count );

            // the one reopen that was started completes, once
            ASSERT_TRUE( RunUntilReopened( fixture, context, audio ) );
            for ( UInt32 i=0; i<5; ++i )
            {
                fixture.Run( 256 );
                context->RenderFrame( 16 );
            }
            ASSERT_TRUE( 1 == completed.count && completed.lastSuccess );
            ASSERT_TRUE( 44100 == fixture.engine.GetMixFormat().GetSampleRate() && 44100 == audio->GetAudioSettings().GetSampleRate() );
            ASSERT_TRUE( audio->GetStatusText().IsNULLOrEmpty() && 2 == applied.count );
            ASSERT_TRUE( Near( AUDIO::CAudioSettings::PercentToDecibels( 33 ), fixture.effects->GetVolumeInDecibels() ) );

            // Cancel and a new Apply while pending: the reopen goes on, the status does not lie
            ASSERT_TRUE( PickIndex( audio, "SampleRate", 2 ) );
            ASSERT_TRUE( form.Apply() && audio->IsDeviceReopenPending() );
            form.Cancel();
            ASSERT_TRUE( audio->IsDeviceReopenPending() && GUI::CAudioSettingsForm::OpeningDeviceText == audio->GetStatusText() );
            ASSERT_FALSE( WidgetEnabled( audio, "SampleRate" ) );
            ASSERT_TRUE( RunUntilReopened( fixture, context, audio ) );
            ASSERT_TRUE( 2 == completed.count && completed.lastSuccess && 48000 == fixture.engine.GetMixFormat().GetSampleRate() );

            // the engine of the page is swapped while pending: no outcome is shown for the old one
            ASSERT_TRUE( form.Open() );
            ASSERT_TRUE( PickIndex( audio, "SampleRate", 1 ) );
            ASSERT_TRUE( form.Apply() && audio->IsDeviceReopenPending() );
            audio->SetAudioEngine( GUCEF_NULL );
            ASSERT_FALSE( audio->IsDeviceReopenPending() );
            ASSERT_TRUE( WidgetEnabled( audio, "SampleRate" ) && audio->GetStatusText().IsNULLOrEmpty() );
            for ( UInt32 i=0; i<3; ++i )
            {
                fixture.engine.Update();
                context->RenderFrame( 16 );
            }
            ASSERT_TRUE( 2 == completed.count );
            form.Cancel();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: The limiter row offers off, instant and lookahead, shows the mode in effect, and the latency label adds the limiter" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        CAudioFixture fixture;
        ASSERT_TRUE( fixture.Start() );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            ASSERT_TRUE( form.Build() && form.Open() );

            // no engine, no mode chosen: the engine default is shown, the three modes are offered
            ASSERT_TRUE( 3 == ComboCount( audio, "Limiter" ) );
            ASSERT_TRUE( ComboHas( audio, "Limiter", "Off" ) && ComboHas( audio, "Limiter", "Instant" ) && ComboHas( audio, "Limiter", "Lookahead" ) );
            ASSERT_TRUE( "Lookahead" == ComboText( audio, "Limiter" ) && ! audio->GetAudioSettings().HasLimiterMode() );
            ASSERT_FALSE( audio->HasChanges() );
            ASSERT_FALSE( audio->GetSettingsFromWidgets().HasLimiterMode() );

            // a running engine on the default shows lookahead and its latency next to the device latency
            audio->SetAudioEngine( &fixture.engine );
            ASSERT_TRUE( "Lookahead" == ComboText( audio, "Limiter" ) && ! audio->GetAudioSettings().HasLimiterMode() );
            UInt32 mixFrames = fixture.engine.GetMixLatencyInFrames();
            ASSERT_TRUE( mixFrames > 0 );
            const CORE::CString lookaheadText = audio->GetWidget( "Latency" )->GetProperty( "Text" );
            ASSERT_TRUE( lookaheadText.HasSubstr( "device " ) == 0 && lookaheadText.HasSubstr( " + limiter 1.5 ms" ) > 0 );

            // each mode is chosen, applied through the reopen, kept by Cancel and ReadSettings
            const char* const names[] = { "Off", "Instant", "Lookahead" };
            const AUDIO::TAudioLimiterMode modes[] = { AUDIO::AUDIOLIMITERMODE_OFF, AUDIO::AUDIOLIMITERMODE_INSTANT, AUDIO::AUDIOLIMITERMODE_LOOKAHEAD };
            const Int32 order[] = { 0, 1, 2, 0 };
            for ( UInt32 i=0; i<4; ++i )
            {
                const Int32 m = order[ i ];
                ASSERT_TRUE( PickIndex( audio, "Limiter", m ) );
                if ( 2 == i )
                {
                    // lookahead after instant is a change as well
                    ASSERT_TRUE( audio->HasChanges() );
                }
                ASSERT_TRUE( form.Apply() );
                ASSERT_TRUE( audio->IsDeviceReopenPending() );
                ASSERT_TRUE( RunUntilReopened( fixture, context, audio ) );
                ASSERT_TRUE( modes[ m ] == fixture.engine.GetParams().GetLimiterMode() );
                ASSERT_TRUE( audio->GetAudioSettings().HasLimiterMode() && modes[ m ] == audio->GetAudioSettings().GetLimiterMode() );
                ASSERT_TRUE( CORE::CString( names[ m ] ) == ComboText( audio, "Limiter" ) );

                const CORE::CString latencyText = audio->GetWidget( "Latency" )->GetProperty( "Text" );
                if ( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD == modes[ m ] )
                {
                    ASSERT_TRUE( fixture.engine.GetMixLatencyInFrames() > 0 && latencyText.HasSubstr( " + limiter 1.5 ms" ) > 0 );
                }
                else
                {
                    ASSERT_TRUE( 0 == fixture.engine.GetMixLatencyInFrames() && latencyText.HasSubstr( "limiter" ) < 0 );
                    ASSERT_TRUE( "unknown" == latencyText );
                }

                // Cancel and ReadSettings show the mode that is in effect
                form.Cancel();
                ASSERT_TRUE( CORE::CString( names[ m ] ) == ComboText( audio, "Limiter" ) );
                audio->ReadSettings();
                ASSERT_TRUE( CORE::CString( names[ m ] ) == ComboText( audio, "Limiter" ) && ! audio->HasChanges() );
            }

            audio->SetAudioEngine( GUCEF_NULL );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: With no mode chosen the limiter row shows the mode of the running engine, not the default" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        AUDIO::CAudioEngine engine;
        AUDIO::CAudioEngineParams params;
        params.SetDriverName( "null" );
        params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
        params.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
        ASSERT_TRUE( engine.Initialize( params ) );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            audio->SetAudioEngine( &engine );
            ASSERT_TRUE( form.Build() && form.Open() );
            ASSERT_TRUE( "Instant" == ComboText( audio, "Limiter" ) && ! audio->GetAudioSettings().HasLimiterMode() );
            ASSERT_FALSE( audio->GetSettingsFromWidgets().HasLimiterMode() );
            ASSERT_TRUE( "unknown" == audio->GetWidget( "Latency" )->GetProperty( "Text" ) );
            form.Cancel();
            audio->SetAudioEngine( GUCEF_NULL );
        }
        engine.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: The level meters follow a playing test sound per bus, fall back when it stops, and are hidden without an engine" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        CAudioFixture fixture;
        ASSERT_TRUE( fixture.Start() );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            ASSERT_TRUE( form.Build() && form.Open() );

            // no engine: nothing to meter
            ASSERT_FALSE( WidgetVisible( audio, "MasterMeter" ) || WidgetVisible( audio, "Bus0Meter" ) || WidgetVisible( audio, "Bus1Meter" ) );

            audio->SetAudioEngine( &fixture.engine );
            audio->SetTestBus( "music" );
            ASSERT_TRUE( WidgetVisible( audio, "MasterMeter" ) && WidgetVisible( audio, "Bus0Meter" ) && WidgetVisible( audio, "Bus1Meter" ) );
            context->RenderFrame( 16 );
            ASSERT_TRUE( Near( -60.0f, MeterValue( audio, "MasterMeter" ) ) && Near( -60.0f, MeterValue( audio, "Bus0Meter" ) ) && Near( -60.0f, MeterValue( audio, "Bus1Meter" ) ) );
            GUI::CProgressBar* master = audio->GetWidgetAs< GUI::CProgressBar >( "MasterMeter" );
            ASSERT_TRUE( GUCEF_NULL != master && Near( -60.0f, master->GetFloatProperty( GUI::EWidgetProperty_Min ) ) && Near( 0.0f, master->GetFloatProperty( GUI::EWidgetProperty_Max ) ) );

            // the tone goes through the music bus (row 0) into the master bus; the effects bus (row 1) stays quiet
            ASSERT_TRUE( audio->PlayTestSound() );
            fixture.Run( 4800 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( MeterValue( audio, "MasterMeter" ) > -50.0f && MeterValue( audio, "MasterMeter" ) <= 0.0f );
            ASSERT_TRUE( MeterValue( audio, "Bus0Meter" ) > -50.0f && MeterValue( audio, "Bus0Meter" ) <= 0.0f );
            ASSERT_TRUE( Near( -60.0f, MeterValue( audio, "Bus1Meter" ) ) );
            ASSERT_FALSE( audio->HasChanges() );

            // the meter shows what the bus has: the same whole dB value as the meter of the engine says
            const AUDIO::CAudioLevelMeter& busMeter = fixture.music->GetMeter();
            Float32 loudest = 0.0f;
            for ( UInt32 c=0; c<busMeter.GetChannelCount(); ++c )
                loudest = busMeter.GetPeak( c ) > loudest ? busMeter.GetPeak( c ) : loudest;
            ASSERT_TRUE( Near( AUDIO::CAudioLevelMeter::ToDecibels( loudest ), MeterValue( audio, "Bus0Meter" ), 0.51f ) );

            // another bus
            audio->StopTestSound();
            fixture.Run( 512 );
            audio->SetTestBus( "effects" );
            ASSERT_TRUE( audio->PlayTestSound() );
            fixture.Run( 4800 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( MeterValue( audio, "Bus1Meter" ) > -50.0f && MeterValue( audio, "MasterMeter" ) > -50.0f );

            // it stops and the meters fall back to the floor
            audio->StopTestSound();
            fixture.Run( 512 );
            fixture.Run( 1024 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( Near( -60.0f, MeterValue( audio, "MasterMeter" ) ) && Near( -60.0f, MeterValue( audio, "Bus0Meter" ) ) && Near( -60.0f, MeterValue( audio, "Bus1Meter" ) ) );

            // the engine goes: the meters are at the floor and stay there
            audio->SetAudioEngine( GUCEF_NULL );
            ASSERT_FALSE( WidgetVisible( audio, "MasterMeter" ) );
            context->RenderFrame( 16 );
            ASSERT_TRUE( Near( -60.0f, MeterValue( audio, "MasterMeter" ) ) );
            form.Cancel();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: The level meters are not updated while the page is not shown and catch up when it is" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateAudioContext( 1280, 960 );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        CAudioFixture fixture;
        ASSERT_TRUE( fixture.Start() );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            form.AddPage( "Video" );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            audio->SetAudioEngine( &fixture.engine );
            ASSERT_TRUE( form.Build() && form.Open() );
            ASSERT_TRUE( form.SelectPage( "Audio" ) );
            context->FlushChanges();
            context->RenderFrame( 16 );
            ASSERT_TRUE( audio->GetRootWidget()->IsEffectivelyVisible() );
            ASSERT_TRUE( Near( -60.0f, MeterValue( audio, "MasterMeter" ) ) );

            // another page is shown: the sound is there, the meter is not touched
            ASSERT_TRUE( form.SelectPage( "Video" ) );
            context->FlushChanges();
            ASSERT_FALSE( audio->GetRootWidget()->IsEffectivelyVisible() );
            ASSERT_TRUE( audio->PlayTestSound() );
            fixture.Run( 4800 );
            context->RenderFrame( 16 );
            fixture.Run( 480 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( fixture.engine.GetMasterBus()->GetMeter().GetPeak( 0 ) > 0.01f );
            ASSERT_TRUE( Near( -60.0f, MeterValue( audio, "MasterMeter" ) ) && Near( -60.0f, MeterValue( audio, "Bus0Meter" ) ) );

            // back on the page: the first frame brings them up to date
            ASSERT_TRUE( form.SelectPage( "Audio" ) );
            context->FlushChanges();
            fixture.Run( 480 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( MeterValue( audio, "MasterMeter" ) > -50.0f );

            // the whole form is closed: the same
            form.Hide();
            context->FlushChanges();
            audio->StopTestSound();
            fixture.Run( 512 );
            fixture.Run( 1024 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( MeterValue( audio, "MasterMeter" ) > -50.0f );
            ASSERT_TRUE( form.Open() );
            context->FlushChanges();
            context->RenderFrame( 16 );
            ASSERT_TRUE( Near( -60.0f, MeterValue( audio, "MasterMeter" ) ) );
            form.Cancel();
            audio->SetAudioEngine( GUCEF_NULL );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: The meters sit under the slider of their row, within the row, at every size" )
    try
    {
        const Int32 sizes[][2] = { { 800, 600 }, { 1242, 952 }, { 1920, 1080 }, { 3440, 1440 }, { 5120, 1440 }, { 720, 1280 } };
        for ( UInt32 s=0; s<6; ++s )
        {
            GUI::TGuiContextPtr contextPtr = CreateAudioContext( sizes[ s ][ 0 ], sizes[ s ][ 1 ] );
            GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                continue;
            CAudioFixture fixture;
            ASSERT_TRUE( fixture.Start() );

            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            audio->SetAudioEngine( &fixture.engine );
            ASSERT_TRUE( form.Build() && form.Open() );
            context->FlushChanges();

            const char* ids[] = { "Master", "Bus0", "Bus1" };
            for ( UInt32 i=0; i<3; ++i )
            {
                GUI::CWidget* slider = audio->GetWidget( CORE::CString( ids[ i ] ) + "Volume" );
                GUI::CWidget* meter = audio->GetWidget( CORE::CString( ids[ i ] ) + "Meter" );
                GUI::CWidget* percent = audio->GetWidget( CORE::CString( ids[ i ] ) + "Percent" );
                GUI::CWidget* mute = audio->GetWidget( CORE::CString( ids[ i ] ) + "Mute" );
                ASSERT_TRUE( GUCEF_NULL != slider && GUCEF_NULL != meter && GUCEF_NULL != percent && GUCEF_NULL != mute );
                if ( GUCEF_NULL == slider || GUCEF_NULL == meter || GUCEF_NULL == percent || GUCEF_NULL == mute )
                    continue;
                const GUI::CRect sliderRect = slider->GetRect();
                const GUI::CRect meterRect = meter->GetRect();
                const GUI::CRect rowRect = meter->GetParentWidget()->GetRect();
                ASSERT_TRUE( meter->IsEffectivelyVisible() && meterRect.width >= 64 && meterRect.height >= 4 );
                ASSERT_TRUE( sliderRect.width >= 80 );
                ASSERT_TRUE( sliderRect.Right() <= percent->GetRect().left && percent->GetRect().Right() <= mute->GetRect().left );
                ASSERT_TRUE( meterRect.left == sliderRect.left && meterRect.width == sliderRect.width );
                ASSERT_TRUE( meterRect.top >= sliderRect.Bottom() - 4 && meterRect.Bottom() <= rowRect.Bottom() && meterRect.Right() <= sizes[ s ][ 0 ] );
                ASSERT_TRUE( meterRect.left >= rowRect.left && meterRect.Right() <= rowRect.Right() );
            }
            form.Cancel();
            audio->SetAudioEngine( GUCEF_NULL );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: The rows follow the real size of the context, centred in a column of at most 960 pixels, scrolling when they do not fit" )
    try
    {
        const Int32 sizes[][2] = { { 800, 600 }, { 1242, 952 }, { 1920, 1080 }, { 3440, 1440 }, { 5120, 1440 }, { 720, 1280 } };
        for ( UInt32 s=0; s<6; ++s )
        {
            GUI::TGuiContextPtr contextPtr = CreateAudioContext( sizes[ s ][ 0 ], sizes[ s ][ 1 ] );
            GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                continue;

            CAudioFixture fixture;
            ASSERT_TRUE( fixture.Start() );
            GUI::CSettingsForm form( context );
            GUI::CAudioSettingsForm* audio = MakeAudioPage( form );
            // the engine default shows the longest latency text, device and limiter
            audio->SetAudioEngine( &fixture.engine );
            ASSERT_TRUE( form.Build() && form.Open() );
            ASSERT_TRUE( audio->GetWidget( "Latency" )->GetProperty( "Text" ).HasSubstr( " + limiter " ) > 0 );
            context->FlushChanges();

            GUI::CScrollPanel* body = static_cast< GUI::CScrollPanel* >( audio->GetWidget( "Body" ) );
            ASSERT_TRUE( GUCEF_NULL != body );
            if ( GUCEF_NULL == body )
                continue;
            const GUI::CRect bodyRect = body->GetRect();
            const GUI::CRect column = body->GetColumnRect();
            ASSERT_TRUE( column.width <= 960 && column.width > 0 );
            ASSERT_TRUE( Near( (Float32) column.left, (Float32) ( bodyRect.width - column.Right() ), 2.0f ) );
            ASSERT_TRUE( bodyRect.Right() <= sizes[ s ][ 0 ] && bodyRect.Bottom() <= sizes[ s ][ 1 ] );

            const char* rows[] = { "OutputDeviceRow", "SampleRateRow", "BufferSizeRow", "LatencyRow", "SpeakerLayoutRow", "MasterRow", "Bus0Row", "Bus1Row", "LimiterRow", "TestSoundRow" };
            for ( UInt32 r=0; r<10; ++r )
            {
                GUI::CWidget* row = audio->GetWidget( rows[ r ] );
                ASSERT_TRUE( GUCEF_NULL != row );
                if ( GUCEF_NULL == row )
                    continue;
                const GUI::CRect rowRect = row->GetRect();
                ASSERT_TRUE( rowRect.left >= bodyRect.left && rowRect.Right() <= bodyRect.Right() );
                ASSERT_TRUE( rowRect.width <= 960 && rowRect.Right() <= sizes[ s ][ 0 ] );
            }

            // controls do not leave their row
            const char* controls[] = { "OutputDevice", "SampleRate", "MasterVolume", "MasterMute", "MasterPercent", "Bus1Volume", "TestSignal", "TestSound", "Latency", "Limiter" };
            for ( UInt32 c=0; c<10; ++c )
            {
                GUI::CWidget* control = audio->GetWidget( controls[ c ] );
                GUI::CWidget* parent = GUCEF_NULL == control ? GUCEF_NULL : control->GetParentWidget();
                ASSERT_TRUE( GUCEF_NULL != parent );
                if ( GUCEF_NULL == parent )
                    continue;
                ASSERT_TRUE( control->GetRect().left >= parent->GetRect().left && control->GetRect().Right() <= parent->GetRect().Right() );
            }

            // the status line is below the body, the panel scrolls where the rows do not fit
            GUI::CWidget* status = audio->GetWidget( "Status" );
            ASSERT_TRUE( GUCEF_NULL != status && status->GetRect().top >= bodyRect.Bottom() - 1 && status->GetRect().Bottom() <= sizes[ s ][ 1 ] );
            if ( 800 == sizes[ s ][ 0 ] )
            {
                ASSERT_TRUE( body->IsScrollable() );
            }
            else
            if ( sizes[ s ][ 1 ] >= 1080 )
            {
                ASSERT_FALSE( body->IsScrollable() );
            }
            form.Cancel();
            audio->SetAudioEngine( GUCEF_NULL );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSettings: Audio settings saved through the JSON codec are read back" )
    try
    {
        if ( LoadLayoutCodecs() )
        {
            CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) );

            AUDIO::CAudioSettings original;
            original.SetDriverName( "alsa" );
            original.SetDeviceId( "hw:1,0" );
            original.SetSampleRate( 96000 );
            original.SetBufferSizeInFrames( 128 );
            original.SetChannelLayout( AUDIO::CAudioFormat::GetDefaultChannelLayout( 8 ) );
            original.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_OFF );
            original.SetMasterVolumeInDecibels( -6.0f );
            original.SetMasterMuted( true );
            original.SetBusVolumeInDecibels( "music", -9.0f );
            original.SetBusMuted( "effects", true );

            CORE::CDataNode config( "Config" );
            ASSERT_TRUE( original.SaveConfig( config ) );
            CORE::CDynamicBuffer buffer;
            CORE::CDynamicBufferAccess output( buffer );
            ASSERT_TRUE( codec->StoreDataTree( &config, &output ) );
            CORE::CDynamicBufferAccess input( buffer );
            CORE::CDataNode parsed;
            ASSERT_TRUE( codec->BuildDataTree( &parsed, &input ) );

            AUDIO::CAudioSettings loaded;
            ASSERT_TRUE( loaded.LoadConfig( parsed ) );
            ASSERT_TRUE( "alsa" == loaded.GetDriverName() && "hw:1,0" == loaded.GetDeviceId() && 96000 == loaded.GetSampleRate() && 128 == loaded.GetBufferSizeInFrames() );
            ASSERT_TRUE( original.GetChannelLayout() == loaded.GetChannelLayout() && loaded.HasLimiterMode() && AUDIO::AUDIOLIMITERMODE_OFF == loaded.GetLimiterMode() );
            ASSERT_TRUE( Near( -6.0f, loaded.GetMasterVolumeInDecibels() ) && loaded.IsMasterMuted() );
            AUDIO::CAudioSettings::SBusSettings bus;
            ASSERT_TRUE( 2 == loaded.GetAllBusSettings().size() );
            ASSERT_TRUE( loaded.GetBusSettings( "music", bus ) && Near( -9.0f, bus.volumeInDecibels ) && !bus.muted );
            ASSERT_TRUE( loaded.GetBusSettings( "effects", bus ) && bus.muted );

            // a single bus is an object in JSON, not an array: it reads back as well
            AUDIO::CAudioSettings single;
            single.SetBusVolumeInDecibels( "music", -3.0f );
            CORE::CDataNode singleConfig( "Config" );
            ASSERT_TRUE( single.SaveConfig( singleConfig ) );
            CORE::CDynamicBuffer singleBuffer;
            CORE::CDynamicBufferAccess singleOutput( singleBuffer );
            ASSERT_TRUE( codec->StoreDataTree( &singleConfig, &singleOutput ) );
            CORE::CDynamicBufferAccess singleInput( singleBuffer );
            CORE::CDataNode singleParsed;
            ASSERT_TRUE( codec->BuildDataTree( &singleParsed, &singleInput ) );
            AUDIO::CAudioSettings singleLoaded;
            ASSERT_TRUE( singleLoaded.LoadConfig( singleParsed ) );
            ASSERT_TRUE( 1 == singleLoaded.GetAllBusSettings().size() && singleLoaded.GetBusSettings( "music", bus ) && Near( -3.0f, bus.volumeInDecibels ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
