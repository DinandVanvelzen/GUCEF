/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#include "gucefAUDIO_CAudioGlobal.h"
#define GUCEF_AUDIO_CAUDIOGLOBAL_H
#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */

#ifndef GUCEF_AUDIO_CAUDIODRIVERREGISTRY_H
#include "gucefAUDIO_CAudioDriverRegistry.h"
#define GUCEF_AUDIO_CAUDIODRIVERREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIODRIVERREGISTRY_H ? */

#ifndef GUCEF_AUDIO_CAUDIODRIVER_H
#include "gucefAUDIO_CAudioDriver.h"
#define GUCEF_AUDIO_CAUDIODRIVER_H
#endif /* GUCEF_AUDIO_CAUDIODRIVER_H ? */

#ifndef GUCEF_AUDIO_CAUDIODEVICE_H
#include "gucefAUDIO_CAudioDevice.h"
#define GUCEF_AUDIO_CAUDIODEVICE_H
#endif /* GUCEF_AUDIO_CAUDIODEVICE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOBUS_H
#include "gucefAUDIO_CAudioBus.h"
#define GUCEF_AUDIO_CAUDIOBUS_H
#endif /* GUCEF_AUDIO_CAUDIOBUS_H ? */

#ifndef GUCEF_AUDIO_CTONESEQUENCESOURCE_H
#include "gucefAUDIO_CToneSequenceSource.h"
#define GUCEF_AUDIO_CTONESEQUENCESOURCE_H
#endif /* GUCEF_AUDIO_CTONESEQUENCESOURCE_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_GUI_CPROGRESSBAR_H
#include "gucefGUI_CProgressBar.h"
#define GUCEF_GUI_CPROGRESSBAR_H
#endif /* GUCEF_GUI_CPROGRESSBAR_H ? */

#include "gucefGUI_CAudioSettingsForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CAudioSettingsForm::DeviceReopenRequiredEvent = "GUCEF::GUI::CAudioSettingsForm::DeviceReopenRequiredEvent";
const CORE::CEvent CAudioSettingsForm::DeviceReopenCompletedEvent = "GUCEF::GUI::CAudioSettingsForm::DeviceReopenCompletedEvent";

const CString CAudioSettingsForm::AutomaticDriverChoice = "Automatic";
const CString CAudioSettingsForm::NullDriverChoice = "None (silent)";
const CString CAudioSettingsForm::DefaultDeviceChoice = "Default device";
const CString CAudioSettingsForm::DefaultDriverBufferChoice = "Driver default";
const CString CAudioSettingsForm::DefaultSpeakersChoice = "Device default";
const CString CAudioSettingsForm::PlayTestSoundText = "Play";
const CString CAudioSettingsForm::StopTestSoundText = "Stop";
const CString CAudioSettingsForm::DeviceLostText = "The audio device is no longer available";
const CString CAudioSettingsForm::OpeningDeviceText = "Opening the audio device";
const CString CAudioSettingsForm::DeviceOpenFailedText = "The audio device could not be opened; the previous device remains in use";
const CString CAudioSettingsForm::DeviceBusyText = "The audio device is still being opened; change the device settings when that is done";

namespace {

const char* const NullDriverName = "null";

/** The rows of the page: a label on the left, the control on the right, in a column of at most 960 pixels */
const Int32 RowStep = 48;
const Int32 RowHeight = 44;
const Int32 StatusHeight = 44;
const Int32 MaxColumnWidth = 960;

const char* const LabelPosition = "0,16 0,8";
const char* const LabelSize = "0.38,-16 0,28";
/** The level meters show the peak from this level up to full scale (0 dBFS) */
const Int32 MeterFloorInDecibels = -60;

const char* const ControlPosition = "0.4,0 0,4";
const char* const ComboSize = "0.6,-16 0,36";

CString
RowPosition( Int32 row )
{
    return "0,0 0," + CORE::Int32ToString( 8 + row * RowStep );
}

/** A row: the panel that holds its widgets, with the label on the left. The widgets of the control are added to the row */
CORE::CDataNode*
AddRow( CORE::CDataNode& body, const CString& rowName, const CString& labelName, const CString& labelText, Int32 row )
{
    CORE::CDataNode* rowNode = CFormLayout::AddWidgetNode( body, "Panel", rowName, RowPosition( row ), CString( "1,0 0," ) + CORE::Int32ToString( RowHeight ) );
    CORE::CDataNode* label = CFormLayout::AddWidgetNode( *rowNode, "Label", labelName, LabelPosition, LabelSize );
    CFormLayout::SetWidgetNodeProperty( *label, EWidgetProperty_Text, labelText );
    return rowNode;
}

CORE::CDataNode*
AddCombo( CORE::CDataNode& rowNode, const CString& name )
{
    CORE::CDataNode* combo = CFormLayout::AddWidgetNode( rowNode, "Combobox", name, ControlPosition, ComboSize );
    CFormLayout::SetWidgetNodeProperty( *combo, EWidgetProperty_DropDownRows, "10" );
    return combo;
}

/** The volume of the master or of a bus: slider with the level meter as a thin bar under it, the percentage as text and the mute check box */
void
AddVolumeControls( CORE::CDataNode& rowNode, const CString& id )
{
    CORE::CDataNode* slider = CFormLayout::AddWidgetNode( rowNode, "Slider", id + "Volume", "0.4,0 0,4", "0.6,-212 0,28" );
    CFormLayout::SetWidgetNodeProperty( *slider, EWidgetProperty_Min, "0" );
    CFormLayout::SetWidgetNodeProperty( *slider, EWidgetProperty_Max, "100" );
    CFormLayout::SetWidgetNodeProperty( *slider, EWidgetProperty_Step, "1" );
    CORE::CDataNode* meter = CFormLayout::AddWidgetNode( rowNode, "ProgressBar", id + "Meter", "0.4,0 0,35", "0.6,-212 0,6" );
    CFormLayout::SetWidgetNodeProperty( *meter, EWidgetProperty_Min, CORE::Int32ToString( MeterFloorInDecibels ) );
    CFormLayout::SetWidgetNodeProperty( *meter, EWidgetProperty_Max, "0" );
    CFormLayout::SetWidgetNodeProperty( *meter, EWidgetProperty_Value, CORE::Int32ToString( MeterFloorInDecibels ) );
    CFormLayout::SetWidgetNodeProperty( *meter, EWidgetProperty_Visible, "false" );
    CFormLayout::SetWidgetNodeProperty( *meter, EWidgetProperty_Tooltip, "Peak level of the sound that goes through, in dB" );
    CORE::CDataNode* percent = CFormLayout::AddWidgetNode( rowNode, "Label", id + "Percent", "1,-204 0,10", "0,64 0,24" );
    CFormLayout::SetWidgetNodeProperty( *percent, EWidgetProperty_Text, "100 %" );
    CORE::CDataNode* mute = CFormLayout::AddWidgetNode( rowNode, "Checkbox", id + "Mute", "1,-132 0,4", "0,116 0,36" );
    CFormLayout::SetWidgetNodeProperty( *mute, EWidgetProperty_Text, "Mute" );
}

bool
EndsWith( const CString& text, const CString& ending )
{
    return text.Length() >= ending.Length() && text.SubstrFromRange( text.Length() - ending.Length(), text.Length() ) == ending;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CAudioSettingsForm::RegisterEvents( void )
{GUCEF_TRACE;

    DeviceReopenRequiredEvent.Initialize();
    DeviceReopenCompletedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAudioSettingsForm::CAudioSettingsForm( void )
    : CSettingsPageForm()
    , m_settings()
    , m_engine( GUCEF_NULL )
    , m_busRows()
    , m_testBus()
    , m_rowIds()
    , m_driverChoices()
    , m_deviceChoices()
    , m_sampleRateChoices()
    , m_bufferSizeChoices()
    , m_speakerLayoutChoices()
    , m_limiterChoices()
    , m_devices()
    , m_defaultDevice()
    , m_hasDefaultDevice( false )
    , m_driverRowShown( false )
    , m_shownLimiterIndex( -1 )
    , m_testVoice()
    , m_testEnded( CORE::TASKSTATUS_UNDEFINED )
    , m_deviceLost()
    , m_reopen( CORE::TASKSTATUS_UNDEFINED )
    , m_reopenPrevious()
    , m_reopenPending( false )
    , m_meterLevels()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CAudioSettingsForm::~CAudioSettingsForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();

    // a sound of the engine does not play on; the widgets may be gone already, so they are left alone
    if ( !m_testVoice.IsNULL() && GUCEF_NULL != m_engine )
        m_engine->Stop( m_testVoice );
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::SetAudioSettings( const AUDIO::CAudioSettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
}

/*-------------------------------------------------------------------------*/

const AUDIO::CAudioSettings&
CAudioSettingsForm::GetAudioSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::SetEngineSubscription( AUDIO::CAudioEngine* engine )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_engine )
        UnsubscribeFrom( m_engine );
    if ( GUCEF_NULL != engine )
    {
        // raised in the thread that runs the Update() of the engine: the handler only sets a flag
        TEventCallback deviceLostCallback( this, &CAudioSettingsForm::OnDeviceLost );
        SubscribeTo( engine, AUDIO::CAudioEngine::DeviceLostEvent, deviceLostCallback );
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::SetAudioEngine( AUDIO::CAudioEngine* engine )
{GUCEF_TRACE;

    if ( engine == m_engine )
        return;

    // a sound of the engine that goes does not play on, and a reopen of the engine that goes has no outcome to show
    StopTestSound();
    DropDeviceReopen();
    SetEngineSubscription( engine );
    m_engine = engine;
    m_deviceLost.Store( 0 );
    if ( GUCEF_NULL != GetRootWidget() )
    {
        UpdateLatency();
        UpdateEnabledRows();
        UpdateTestSoundButton();
        UpdateMeters( true );
    }
}

/*-------------------------------------------------------------------------*/

AUDIO::CAudioEngine*
CAudioSettingsForm::GetAudioEngine( void ) const
{GUCEF_TRACE;

    return m_engine;
}

/*-------------------------------------------------------------------------*/

bool
CAudioSettingsForm::AddBusRow( const CString& busName, const CString& label )
{GUCEF_TRACE;

    if ( busName.IsNULLOrEmpty() || GUCEF_NULL != GetRootWidget() )
        return false;
    for ( UInt32 i=0; i<(UInt32)m_busRows.size(); ++i )
        if ( m_busRows[ i ].busName == busName )
            return false;

    SBusRow row;
    row.busName = busName;
    row.label = label.IsNULLOrEmpty() ? busName : label;
    m_busRows.push_back( row );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::SetTestBus( const CString& busName )
{GUCEF_TRACE;

    m_testBus = busName;
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioSettingsForm::GetTestBus( void ) const
{GUCEF_TRACE;

    return m_testBus;
}

/*-------------------------------------------------------------------------*/

CString
CAudioSettingsForm::GetTitle( void ) const
{GUCEF_TRACE;

    return "Audio";
}

/*-------------------------------------------------------------------------*/

CString
CAudioSettingsForm::RowName( const CString& rowId )
{GUCEF_TRACE;

    return rowId + "Row";
}

/*-------------------------------------------------------------------------*/

CString
CAudioSettingsForm::BusRowId( UInt32 index )
{GUCEF_TRACE;

    return "Bus" + CORE::UInt32ToString( index );
}

/*-------------------------------------------------------------------------*/

CString
CAudioSettingsForm::MillisecondsText( UInt32 frames, UInt32 sampleRate )
{GUCEF_TRACE;

    UInt64 tenths = ( (UInt64) frames * 10000 + sampleRate / 2 ) / sampleRate;
    return CORE::UInt64ToString( tenths / 10 ) + "." + CORE::UInt64ToString( tenths % 10 ) + " ms";
}

/*-------------------------------------------------------------------------*/

CString
CAudioSettingsForm::DurationText( UInt32 frames, UInt32 sampleRate )
{GUCEF_TRACE;

    if ( 0 == sampleRate )
        return CORE::UInt32ToString( frames ) + " frames";
    return CORE::UInt32ToString( frames ) + " frames, " + MillisecondsText( frames, sampleRate );
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    m_rowIds.clear();

    // the driver row is shown when there is a choice, which is known when the page is built and read
    CORE::CDataNode* page = CFormLayout::AddWidgetNode( layout, "Panel", "AudioSettings", "0,0 0,0", "1,0 1,0" );
    CORE::CDataNode* body = CFormLayout::AddWidgetNode( *page, "ScrollPanel", "Body", "0,0 0,0", CString( "1,0 1,-" ) + CORE::Int32ToString( StatusHeight ) );
    CFormLayout::SetWidgetNodeProperty( *body, EWidgetProperty_MaxColumnWidth, CORE::Int32ToString( MaxColumnWidth ) );
    CFormLayout::SetWidgetNodeProperty( *body, EWidgetProperty_BottomPadding, "16" );

    Int32 row = 0;

    CORE::CDataNode* rowNode = AddRow( *body, RowName( "Driver" ), "DriverLabel", "Audio system", row++ );
    AddCombo( *rowNode, "Driver" );
    m_rowIds.push_back( "Driver" );

    rowNode = AddRow( *body, RowName( "OutputDevice" ), "OutputDeviceLabel", "Output device", row++ );
    AddCombo( *rowNode, "OutputDevice" );
    m_rowIds.push_back( "OutputDevice" );

    rowNode = AddRow( *body, RowName( "SampleRate" ), "SampleRateLabel", "Sample rate", row++ );
    AddCombo( *rowNode, "SampleRate" );
    m_rowIds.push_back( "SampleRate" );

    rowNode = AddRow( *body, RowName( "BufferSize" ), "BufferSizeLabel", "Buffer size", row++ );
    AddCombo( *rowNode, "BufferSize" );
    m_rowIds.push_back( "BufferSize" );

    rowNode = AddRow( *body, RowName( "Latency" ), "LatencyLabel", "Latency", row++ );
    CORE::CDataNode* latency = CFormLayout::AddWidgetNode( *rowNode, "Label", "Latency", "0.4,0 0,8", "0.6,-16 0,28" );
    CFormLayout::SetWidgetNodeProperty( *latency, EWidgetProperty_Text, "unknown" );
    m_rowIds.push_back( "Latency" );

    rowNode = AddRow( *body, RowName( "SpeakerLayout" ), "SpeakerLayoutLabel", "Speakers", row++ );
    AddCombo( *rowNode, "SpeakerLayout" );
    m_rowIds.push_back( "SpeakerLayout" );

    rowNode = AddRow( *body, RowName( "Master" ), "MasterLabel", "Master volume", row++ );
    AddVolumeControls( *rowNode, "Master" );
    m_rowIds.push_back( "Master" );

    for ( UInt32 i=0; i<(UInt32)m_busRows.size(); ++i )
    {
        CString id = BusRowId( i );
        rowNode = AddRow( *body, RowName( id ), id + "Label", m_busRows[ i ].label, row++ );
        AddVolumeControls( *rowNode, id );
        m_rowIds.push_back( id );
    }

    rowNode = AddRow( *body, RowName( "Limiter" ), "LimiterLabel", "Limiter", row++ );
    CORE::CDataNode* limiter = AddCombo( *rowNode, "Limiter" );
    CFormLayout::SetWidgetNodeProperty( *limiter, EWidgetProperty_Tooltip, "Keeps the sound from going over full level. Changing it opens the audio device again" );
    m_rowIds.push_back( "Limiter" );

    rowNode = AddRow( *body, RowName( "TestSound" ), "TestSoundLabel", "Test sound", row++ );
    CORE::CDataNode* signal = CFormLayout::AddWidgetNode( *rowNode, "Combobox", "TestSignal", ControlPosition, "0.6,-164 0,36" );
    CFormLayout::SetWidgetNodeProperty( *signal, EWidgetProperty_DropDownRows, "10" );
    CORE::CDataNode* play = CFormLayout::AddWidgetNode( *rowNode, "Button", "TestSound", "1,-148 0,4", "0,132 0,36" );
    CFormLayout::SetWidgetNodeProperty( *play, EWidgetProperty_Text, PlayTestSoundText );
    CFormLayout::SetWidgetNodeProperty( *play, EWidgetProperty_Tooltip, "Plays with the settings that were applied last" );
    m_rowIds.push_back( "TestSound" );

    CORE::CDataNode* status = CFormLayout::AddWidgetNode( *page, "Label", "Status", CString( "0,24 1,-" ) + CORE::Int32ToString( StatusHeight - 8 ), "1,-48 0,28" );
    CFormLayout::SetWidgetNodeProperty( *status, EWidgetProperty_Text, "" );
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CSettingsPageForm::OnPostLayoutLoad();

    CWidget* testSound = GetWidget( "TestSound" );
    if ( GUCEF_NULL != testSound )
    {
        TEventCallback activatedCallback( this, &CAudioSettingsForm::OnTestSoundActivated );
        SubscribeTo( testSound, CWidget::ActivatedEvent, activatedCallback );
    }
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context )
    {
        TEventCallback frameStartedCallback( this, &CAudioSettingsForm::OnFrameStarted );
        SubscribeTo( context, CGUIContext::FrameStartedEvent, frameStartedCallback );
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::OnTestSoundActivated( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsTestSoundPlaying() )
    {
        StopTestSound();
    }
    else
    if ( !PlayTestSound() )
    {
        SetStatusText( "The test sound could not be started" );
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::OnDeviceLost( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // not the thread of the GUI: the frame handler does the rest
    m_deviceLost.Store( 1 );
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::OnFrameStarted( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( 0 != m_deviceLost.Exchange( 0 ) )
        SetStatusText( DeviceLostText );

    // a reopen that has ended: only asked, never waited for
    if ( m_reopenPending && m_reopen.IsReady() )
    {
        bool success = false;
        if ( !m_reopen.HasNoFuture() )
        {
            // ready: this returns at once
            CORE::CTaskPtr task = m_reopen.GetResult( 0 );
            success = !task.IsNULL() && CORE::TASKSTATUS_FINISHED == task->GetTaskStatus();
        }
        FinishDeviceReopen( success );
    }

    // a test sound that played to its end
    if ( !m_testVoice.IsNULL() && m_testEnded.IsReady() )
    {
        m_testVoice = AUDIO::TAudioVoicePtr();
        m_testEnded = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
        UpdateTestSoundButton();
    }

    UpdateMeters( false );
}

/*-------------------------------------------------------------------------*/

CString
CAudioSettingsForm::ResolveDriverName( const CString& driverName ) const
{GUCEF_TRACE;

    if ( !driverName.IsNULLOrEmpty() )
        return driverName;
    if ( GUCEF_NULL != m_engine && m_engine->IsInitialized() )
        return m_engine->GetDriverName();

    GUCEF::vector< AUDIO::CAudioDriverRegistry::TAudioDriverPtr > drivers;
    AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry().GetDriversByPriority( drivers );
    for ( UInt32 i=0; i<(UInt32)drivers.size(); ++i )
        if ( !drivers[ i ].IsNULL() && drivers[ i ]->GetName().Lowercase() != NullDriverName )
            return drivers[ i ]->GetName();
    return NullDriverName;
}

/*-------------------------------------------------------------------------*/

bool
CAudioSettingsForm::FindDeviceInfo( const CString& deviceId, AUDIO::CAudioDeviceInfo& info ) const
{GUCEF_TRACE;

    if ( deviceId.IsNULLOrEmpty() )
    {
        if ( !m_hasDefaultDevice )
            return false;
        info = m_defaultDevice;
        return true;
    }
    for ( UInt32 i=0; i<(UInt32)m_devices.size(); ++i )
    {
        if ( m_devices[ i ].GetId() == deviceId )
        {
            info = m_devices[ i ];
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::FillCombobox( const CString& widgetName, const TChoiceVector& choices, Int32 selectedIndex )
{GUCEF_TRACE;

    CCombobox* list = GetWidgetAs< CCombobox >( widgetName );
    if ( GUCEF_NULL == list )
        return;
    CCombobox::TStringVector items;
    for ( UInt32 i=0; i<(UInt32)choices.size(); ++i )
        items.push_back( choices[ i ].text );
    list->SetListItems( items );
    list->SetSelectedIndex( selectedIndex );
}

/*-------------------------------------------------------------------------*/

Int32
CAudioSettingsForm::GetSelectedIndex( const CString& widgetName )
{GUCEF_TRACE;

    CCombobox* list = GetWidgetAs< CCombobox >( widgetName );
    return GUCEF_NULL == list ? -1 : list->GetSelectedIndex();
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::FillDriverList( const SDeviceSelection& selection )
{GUCEF_TRACE;

    m_driverChoices.clear();
    m_driverChoices.push_back( SChoice( AutomaticDriverChoice, CString::Empty, 0 ) );

    AUDIO::CAudioDriverRegistry& registry = AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry();
    GUCEF::vector< AUDIO::CAudioDriverRegistry::TAudioDriverPtr > drivers;
    registry.GetDriversByPriority( drivers );
    UInt32 realDrivers = 0;
    bool hasNullDriver = false;
    for ( UInt32 i=0; i<(UInt32)drivers.size(); ++i )
    {
        if ( drivers[ i ].IsNULL() )
            continue;
        CString name = drivers[ i ]->GetName();
        if ( name.Lowercase() == NullDriverName )
        {
            hasNullDriver = true;
            continue;
        }
        m_driverChoices.push_back( SChoice( name, name, 0 ) );
        ++realDrivers;
    }
    if ( hasNullDriver )
        m_driverChoices.push_back( SChoice( NullDriverChoice, NullDriverName, 0 ) );

    // there is something to choose when there are two drivers that play sound
    m_driverRowShown = realDrivers > 1;

    Int32 selected = 0;
    if ( !selection.driverName.IsNULLOrEmpty() )
    {
        selected = -1;
        for ( UInt32 i=0; i<(UInt32)m_driverChoices.size(); ++i )
            if ( m_driverChoices[ i ].id.Lowercase() == selection.driverName.Lowercase() )
                selected = (Int32) i;
        if ( selected < 0 )
        {
            // a driver of the settings that is not here now: it stays what the settings say until the user picks another
            m_driverChoices.push_back( SChoice( selection.driverName, selection.driverName, 0 ) );
            selected = (Int32) m_driverChoices.size() - 1;
        }
    }
    FillCombobox( "Driver", m_driverChoices, selected );
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::FillDeviceList( const SDeviceSelection& selection )
{GUCEF_TRACE;

    m_devices.clear();
    m_hasDefaultDevice = false;

    AUDIO::CAudioDriverRegistry::TAudioDriverPtr driver;
    if ( AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry().TryFindDriver( ResolveDriverName( selection.driverName ), driver ) && !driver.IsNULL() )
    {
        driver->EnumerateDevices( AUDIO::AUDIODIRECTION_PLAYBACK, m_devices );
        for ( UInt32 i=0; i<(UInt32)m_devices.size() && !m_hasDefaultDevice; ++i )
        {
            if ( m_devices[ i ].IsDefault() )
            {
                m_defaultDevice = m_devices[ i ];
                m_hasDefaultDevice = true;
            }
        }
        if ( !m_hasDefaultDevice && !m_devices.empty() )
        {
            m_defaultDevice = m_devices[ 0 ];
            m_hasDefaultDevice = true;
        }
    }

    m_deviceChoices.clear();
    m_deviceChoices.push_back( SChoice( DefaultDeviceChoice, CString::Empty, 0 ) );
    Int32 selected = 0;
    for ( UInt32 i=0; i<(UInt32)m_devices.size(); ++i )
    {
        m_deviceChoices.push_back( SChoice( m_devices[ i ].GetName(), m_devices[ i ].GetId(), 0 ) );
        if ( !selection.deviceId.IsNULLOrEmpty() && m_devices[ i ].GetId() == selection.deviceId )
            selected = (Int32) m_deviceChoices.size() - 1;
    }
    if ( !selection.deviceId.IsNULLOrEmpty() && 0 == selected )
    {
        // a device of the settings that is not there now (unplugged, another driver): it is kept as it is
        m_deviceChoices.push_back( SChoice( selection.deviceId, selection.deviceId, 0 ) );
        selected = (Int32) m_deviceChoices.size() - 1;
    }
    FillCombobox( "OutputDevice", m_deviceChoices, selected );
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::FillSampleRateList( const SDeviceSelection& selection )
{GUCEF_TRACE;

    AUDIO::CAudioDeviceInfo info;
    UInt32 defaultRate = 0;
    if ( FindDeviceInfo( selection.deviceId, info ) )
        defaultRate = info.GetPreferredFormat().GetSampleRate();

    m_sampleRateChoices.clear();
    m_sampleRateChoices.push_back( SChoice( defaultRate > 0 ? CString( "Device default (" ) + CORE::UInt32ToString( defaultRate ) + " Hz)" : CString( "Device default" ), CString::Empty, 0 ) );
    const UInt32 rates[] = { 44100, 48000, 88200, 96000 };
    Int32 selected = 0;
    for ( UInt32 i=0; i<4; ++i )
    {
        m_sampleRateChoices.push_back( SChoice( CORE::UInt32ToString( rates[ i ] ) + " Hz", CString::Empty, rates[ i ] ) );
        if ( rates[ i ] == selection.sampleRate )
            selected = (Int32) m_sampleRateChoices.size() - 1;
    }
    if ( selection.sampleRate > 0 && 0 == selected )
    {
        m_sampleRateChoices.push_back( SChoice( CORE::UInt32ToString( selection.sampleRate ) + " Hz", CString::Empty, selection.sampleRate ) );
        selected = (Int32) m_sampleRateChoices.size() - 1;
    }
    FillCombobox( "SampleRate", m_sampleRateChoices, selected );
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::FillBufferSizeList( const SDeviceSelection& selection )
{GUCEF_TRACE;

    // the duration of a size depends on the rate that is chosen, else on the one of the device
    UInt32 rate = selection.sampleRate;
    if ( 0 == rate )
    {
        AUDIO::CAudioDeviceInfo info;
        if ( FindDeviceInfo( selection.deviceId, info ) )
            rate = info.GetPreferredFormat().GetSampleRate();
    }
    if ( 0 == rate )
        rate = 48000;

    m_bufferSizeChoices.clear();
    m_bufferSizeChoices.push_back( SChoice( DefaultDriverBufferChoice, CString::Empty, 0 ) );
    const UInt32 sizes[] = { 128, 256, 512, 1024, 2048, 4096 };
    Int32 selected = 0;
    for ( UInt32 i=0; i<6; ++i )
    {
        m_bufferSizeChoices.push_back( SChoice( DurationText( sizes[ i ], rate ), CString::Empty, sizes[ i ] ) );
        if ( sizes[ i ] == selection.bufferSize )
            selected = (Int32) m_bufferSizeChoices.size() - 1;
    }
    if ( selection.bufferSize > 0 && 0 == selected )
    {
        m_bufferSizeChoices.push_back( SChoice( DurationText( selection.bufferSize, rate ), CString::Empty, selection.bufferSize ) );
        selected = (Int32) m_bufferSizeChoices.size() - 1;
    }
    FillCombobox( "BufferSize", m_bufferSizeChoices, selected );
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::FillSpeakerLayoutList( const SDeviceSelection& selection )
{GUCEF_TRACE;

    UInt32 maxChannels = 0;
    AUDIO::CAudioDeviceInfo info;
    if ( FindDeviceInfo( selection.deviceId, info ) )
        maxChannels = info.GetMaxChannelCount();

    m_speakerLayoutChoices.clear();
    m_speakerLayoutChoices.push_back( SChoice( DefaultSpeakersChoice, CString::Empty, 0 ) );

    // a layout the device has too few channels for is not offered, when the device says how many it has
    struct SLayout { UInt32 channels; const char* text; };
    const SLayout layouts[] = { { 1, "Mono" }, { 2, "Stereo" }, { 4, "Quadraphonic" }, { 6, "5.1 surround" }, { 8, "7.1 surround" } };
    Int32 selected = 0;
    for ( UInt32 i=0; i<5; ++i )
    {
        if ( maxChannels > 0 && layouts[ i ].channels > maxChannels )
            continue;
        UInt32 mask = AUDIO::CAudioFormat::GetDefaultChannelLayout( layouts[ i ].channels );
        m_speakerLayoutChoices.push_back( SChoice( layouts[ i ].text, CString::Empty, mask ) );
        if ( mask == selection.channelLayout )
            selected = (Int32) m_speakerLayoutChoices.size() - 1;
    }
    if ( selection.channelLayout != 0 && 0 == selected )
    {
        m_speakerLayoutChoices.push_back( SChoice( AUDIO::CAudioSettings::ChannelLayoutToString( selection.channelLayout ), CString::Empty, selection.channelLayout ) );
        selected = (Int32) m_speakerLayoutChoices.size() - 1;
    }
    FillCombobox( "SpeakerLayout", m_speakerLayoutChoices, selected );
}

/*-------------------------------------------------------------------------*/

CAudioSettingsForm::SDeviceSelection
CAudioSettingsForm::GetDeviceSelectionFromWidgets( void )
{GUCEF_TRACE;

    SDeviceSelection selection;
    selection.driverName = m_settings.GetDriverName();
    selection.deviceId = m_settings.GetDeviceId();
    selection.sampleRate = m_settings.GetSampleRate();
    selection.bufferSize = m_settings.GetBufferSizeInFrames();
    selection.channelLayout = m_settings.GetChannelLayout();

    Int32 index = GetSelectedIndex( "Driver" );
    if ( index >= 0 && index < (Int32) m_driverChoices.size() )
        selection.driverName = m_driverChoices[ index ].id;
    index = GetSelectedIndex( "OutputDevice" );
    if ( index >= 0 && index < (Int32) m_deviceChoices.size() )
        selection.deviceId = m_deviceChoices[ index ].id;
    index = GetSelectedIndex( "SampleRate" );
    if ( index >= 0 && index < (Int32) m_sampleRateChoices.size() )
        selection.sampleRate = m_sampleRateChoices[ index ].number;
    index = GetSelectedIndex( "BufferSize" );
    if ( index >= 0 && index < (Int32) m_bufferSizeChoices.size() )
        selection.bufferSize = m_bufferSizeChoices[ index ].number;
    index = GetSelectedIndex( "SpeakerLayout" );
    if ( index >= 0 && index < (Int32) m_speakerLayoutChoices.size() )
        selection.channelLayout = m_speakerLayoutChoices[ index ].number;
    return selection;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::GetShownBusValues( const CString& busName, Float32& decibels, bool& muted ) const
{GUCEF_TRACE;

    decibels = 0.0f;
    muted = false;
    AUDIO::CAudioSettings::SBusSettings bus;
    if ( busName.IsNULLOrEmpty() )
    {
        decibels = m_settings.GetMasterVolumeInDecibels();
        muted = m_settings.IsMasterMuted();
    }
    else
    if ( m_settings.GetBusSettings( busName, bus ) )
    {
        decibels = bus.volumeInDecibels;
        muted = bus.muted;
    }
    else
    if ( GUCEF_NULL != m_engine && m_engine->IsInitialized() )
    {
        // nothing of the user for this bus yet: what the bus has now
        AUDIO::CAudioBus* engineBus = m_engine->FindBus( busName );
        if ( GUCEF_NULL != engineBus )
        {
            decibels = engineBus->GetVolumeInDecibels();
            muted = engineBus->IsMuted();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::ReadVolumeRow( const CString& rowId, const CString& busName, bool master )
{GUCEF_TRACE;

    Float32 decibels = 0.0f;
    bool muted = false;
    GetShownBusValues( master ? CString::Empty : busName, decibels, muted );
    UInt32 percent = AUDIO::CAudioSettings::DecibelsToPercent( decibels );

    CWidget* slider = GetWidget( rowId + "Volume" );
    if ( GUCEF_NULL != slider )
        slider->SetFloatProperty( EWidgetProperty_Value, (Float32) percent );
    SetWidgetProperty( rowId + "Mute", EWidgetProperty_Checked, CORE::BoolToString( muted ) );
    UpdatePercentLabel( rowId + "Volume" );
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::UpdatePercentLabel( const CString& sliderName )
{GUCEF_TRACE;

    CWidget* slider = GetWidget( sliderName );
    if ( GUCEF_NULL == slider || !EndsWith( sliderName, "Volume" ) )
        return;
    CString base = sliderName.SubstrFromRange( 0, sliderName.Length() - 6 );
    UInt32 percent = (UInt32) ( slider->GetFloatProperty( EWidgetProperty_Value ) + 0.5f );
    SetWidgetProperty( base + "Percent", EWidgetProperty_Text, CORE::UInt32ToString( percent ) + " %" );
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::UpdateLatency( void )
{GUCEF_TRACE;

    CString text = "unknown";
    if ( GUCEF_NULL != m_engine && m_engine->IsInitialized() )
    {
        AUDIO::TAudioDevicePtr device = m_engine->GetDevice();
        if ( !device.IsNULL() )
        {
            UInt32 frames = device->GetLatencyInFrames();
            UInt32 rate = device->GetFormat().GetSampleRate();
            // the limiter delays the mix before it reaches the device: both add up
            UInt32 mixFrames = m_engine->GetMixLatencyInFrames();
            UInt32 mixRate = m_engine->GetMixFormat().GetSampleRate();
            const bool deviceKnown = frames > 0 && rate > 0;
            if ( mixFrames > 0 && mixRate > 0 )
            {
                CString deviceText = deviceKnown ? MillisecondsText( frames, rate ) : CString( "unknown" );
                text = "device " + deviceText + " + limiter " + MillisecondsText( mixFrames, mixRate );
            }
            else
            if ( deviceKnown )
                text = DurationText( frames, rate );
        }
    }
    SetWidgetProperty( "Latency", EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::SetRowEnabled( const CString& rowId, bool enabled )
{GUCEF_TRACE;

    CWidget* row = GetWidget( RowName( rowId ) );
    if ( GUCEF_NULL == row )
        return;
    const CWidget::TWidgetVector& children = row->GetChildren();
    for ( UInt32 i=0; i<(UInt32)children.size(); ++i )
        children[ i ]->SetEnabled( enabled );
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::UpdateEnabledRows( void )
{GUCEF_TRACE;

    const bool running = GUCEF_NULL != m_engine && m_engine->IsInitialized();

    // without an engine the volumes are only settings; with one, a bus the engine does not have has nothing to set
    for ( UInt32 i=0; i<(UInt32)m_busRows.size(); ++i )
    {
        bool enabled = GUCEF_NULL == m_engine || ( running && GUCEF_NULL != m_engine->FindBus( m_busRows[ i ].busName ) );
        SetRowEnabled( BusRowId( i ), enabled );
    }
    SetRowEnabled( "TestSound", running );

    // the device is being replaced: its rows wait for the outcome
    const char* const deviceRows[] = { "Driver", "OutputDevice", "SampleRate", "BufferSize", "SpeakerLayout", "Limiter" };
    for ( UInt32 i=0; i<6; ++i )
        SetRowEnabled( deviceRows[ i ], !m_reopenPending );

    // meters have something to show when an engine runs
    CWidget* masterMeter = GetWidget( "MasterMeter" );
    if ( GUCEF_NULL != masterMeter )
        masterMeter->SetVisibility( running );
    for ( UInt32 i=0; i<(UInt32)m_busRows.size(); ++i )
    {
        CWidget* busMeter = GetWidget( BusRowId( i ) + "Meter" );
        if ( GUCEF_NULL != busMeter )
            busMeter->SetVisibility( running );
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::PositionRows( void )
{GUCEF_TRACE;

    Int32 y = 8;
    for ( UInt32 i=0; i<(UInt32)m_rowIds.size(); ++i )
    {
        CWidget* row = GetWidget( RowName( m_rowIds[ i ] ) );
        if ( GUCEF_NULL == row )
            continue;
        bool shown = m_rowIds[ i ] != "Driver" || m_driverRowShown;
        row->SetVisibility( shown );
        if ( shown )
        {
            row->SetProperty( EWidgetProperty_Position, "0,0 0," + CORE::Int32ToString( y ) );
            y += RowStep;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::SetStatusText( const CString& text )
{GUCEF_TRACE;

    SetWidgetProperty( "Status", EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

CString
CAudioSettingsForm::GetStatusText( void )
{GUCEF_TRACE;

    return GetWidgetProperty( "Status", EWidgetProperty_Text );
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::UpdateTestSoundButton( void )
{GUCEF_TRACE;

    SetWidgetProperty( "TestSound", EWidgetProperty_Text, IsTestSoundPlaying() ? StopTestSoundText : PlayTestSoundText );
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::ReadDeviceRows( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() )
        return;

    CReadingSettingsScope readingSettings( *this );

    SDeviceSelection selection;
    selection.driverName = m_settings.GetDriverName();
    selection.deviceId = m_settings.GetDeviceId();
    selection.sampleRate = m_settings.GetSampleRate();
    selection.bufferSize = m_settings.GetBufferSizeInFrames();
    selection.channelLayout = m_settings.GetChannelLayout();
    FillDriverList( selection );
    FillDeviceList( selection );
    FillSampleRateList( selection );
    FillBufferSizeList( selection );
    FillSpeakerLayoutList( selection );
    UpdateLatency();

    // the limiter; the settings decide once they hold a mode, otherwise the mode in effect is shown: the one of the
    // running engine, or the engine default when there is none
    m_limiterChoices.clear();
    m_limiterChoices.push_back( SChoice( "Off", AUDIO::CAudioSettings::LimiterModeToString( AUDIO::AUDIOLIMITERMODE_OFF ), (UInt32) AUDIO::AUDIOLIMITERMODE_OFF ) );
    m_limiterChoices.push_back( SChoice( "Instant", AUDIO::CAudioSettings::LimiterModeToString( AUDIO::AUDIOLIMITERMODE_INSTANT ), (UInt32) AUDIO::AUDIOLIMITERMODE_INSTANT ) );
    m_limiterChoices.push_back( SChoice( "Lookahead", AUDIO::CAudioSettings::LimiterModeToString( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD ), (UInt32) AUDIO::AUDIOLIMITERMODE_LOOKAHEAD ) );

    AUDIO::TAudioLimiterMode shownMode = AUDIO::AUDIOLIMITERMODE_LOOKAHEAD;
    if ( m_settings.HasLimiterMode() )
        shownMode = m_settings.GetLimiterMode();
    else
    if ( GUCEF_NULL != m_engine && m_engine->IsInitialized() )
        shownMode = m_engine->GetParams().GetLimiterMode();
    else
        shownMode = AUDIO::CAudioEngineParams().GetLimiterMode();

    m_shownLimiterIndex = 2;
    for ( UInt32 i = 0; i < m_limiterChoices.size(); ++i )
    {
        if ( m_limiterChoices[ i ].number == (UInt32) shownMode )
            m_shownLimiterIndex = (Int32) i;
    }
    FillCombobox( "Limiter", m_limiterChoices, m_shownLimiterIndex );

    // the driver row is there when there is a choice
    PositionRows();
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::ReadSettings( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() )
        return;

    {
        CReadingSettingsScope readingSettings( *this );

        StopTestSound();

        ReadDeviceRows();

        ReadVolumeRow( "Master", CString::Empty, true );
        for ( UInt32 i=0; i<(UInt32)m_busRows.size(); ++i )
            ReadVolumeRow( BusRowId( i ), m_busRows[ i ].busName, false );

        CCombobox* signals = GetWidgetAs< CCombobox >( "TestSignal" );
        if ( GUCEF_NULL != signals )
        {
            Int32 signal = signals->GetSelectedIndex();
            CCombobox::TStringVector items;
            items.push_back( "Tone ladder" );
            items.push_back( "Speaker check" );
            items.push_back( "Sweep" );
            items.push_back( "Noise" );
            signals->SetListItems( items );
            signals->SetSelectedIndex( signal >= 0 && signal < (Int32) items.size() ? signal : 0 );
        }
        UpdateTestSoundButton();

        PositionRows();
        UpdateEnabledRows();

        // a device that is being opened is still being opened, the rows only show what the settings have
        SetStatusText( m_reopenPending ? OpeningDeviceText : CString::Empty );

        // the meters are set by the next frame that shows the page
        m_meterLevels.assign( m_busRows.size() + 1, 1 );
    }
    SetHasChanges( false );
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::OnValueChanged( CWidget& widget )
{GUCEF_TRACE;

    const CString& name = widget.GetName();
    if ( EndsWith( name, "Volume" ) )
    {
        UpdatePercentLabel( name );
        return;
    }

    // the lists of the device rows follow the choices before them
    const bool isDriver = name == "Driver";
    const bool isDevice = name == "OutputDevice";
    const bool isRate = name == "SampleRate";
    if ( !isDriver && !isDevice && !isRate )
        return;
    if ( IsReadingSettings() )
        return;

    CReadingSettingsScope readingSettings( *this );
    SDeviceSelection selection = GetDeviceSelectionFromWidgets();
    if ( isDriver )
    {
        FillDeviceList( selection );
        selection = GetDeviceSelectionFromWidgets();
    }
    if ( isDriver || isDevice )
    {
        FillSampleRateList( selection );
        selection = GetDeviceSelectionFromWidgets();
    }
    FillBufferSizeList( selection );
    if ( isDriver || isDevice )
        FillSpeakerLayoutList( selection );
}

/*-------------------------------------------------------------------------*/

AUDIO::CAudioSettings
CAudioSettingsForm::GetSettingsFromWidgets( void )
{GUCEF_TRACE;

    AUDIO::CAudioSettings settings( m_settings );
    if ( GUCEF_NULL == GetRootWidget() )
        return settings;

    SDeviceSelection selection = GetDeviceSelectionFromWidgets();
    settings.SetDriverName( selection.driverName );
    settings.SetDeviceId( selection.deviceId );
    settings.SetSampleRate( selection.sampleRate );
    settings.SetBufferSizeInFrames( selection.bufferSize );
    settings.SetChannelLayout( selection.channelLayout );

    // the limiter mode stays unset as long as the user leaves the list where ReadSettings() put it
    Int32 limiter = GetSelectedIndex( "Limiter" );
    if ( limiter >= 0 && limiter < (Int32) m_limiterChoices.size() && limiter != m_shownLimiterIndex )
        settings.SetLimiterMode( (AUDIO::TAudioLimiterMode) m_limiterChoices[ limiter ].number );

    // a volume is only taken when the user moved its slider or mute: a volume that is not a whole percentage (set by
    // the application, or the volume a bus has in the engine) is not rounded by saving the page
    Float32 shownDecibels = 0.0f;
    bool shownMuted = false;
    GetShownBusValues( CString::Empty, shownDecibels, shownMuted );
    CWidget* masterSlider = GetWidget( "MasterVolume" );
    if ( GUCEF_NULL != masterSlider )
    {
        UInt32 percent = (UInt32) ( masterSlider->GetFloatProperty( EWidgetProperty_Value ) + 0.5f );
        if ( percent != AUDIO::CAudioSettings::DecibelsToPercent( shownDecibels ) )
            settings.SetMasterVolumeInDecibels( AUDIO::CAudioSettings::PercentToDecibels( percent ) );
    }
    settings.SetMasterMuted( CORE::StringToBool( GetWidgetProperty( "MasterMute", EWidgetProperty_Checked ), shownMuted ) );

    for ( UInt32 i=0; i<(UInt32)m_busRows.size(); ++i )
    {
        const CString id = BusRowId( i );
        CWidget* slider = GetWidget( id + "Volume" );
        if ( GUCEF_NULL == slider )
            continue;
        GetShownBusValues( m_busRows[ i ].busName, shownDecibels, shownMuted );
        UInt32 percent = (UInt32) ( slider->GetFloatProperty( EWidgetProperty_Value ) + 0.5f );
        bool muted = CORE::StringToBool( GetWidgetProperty( id + "Mute", EWidgetProperty_Checked ), shownMuted );
        bool volumeChanged = percent != AUDIO::CAudioSettings::DecibelsToPercent( shownDecibels );
        if ( volumeChanged || muted != shownMuted )
        {
            settings.SetBusVolumeInDecibels( m_busRows[ i ].busName, volumeChanged ? AUDIO::CAudioSettings::PercentToDecibels( percent ) : shownDecibels );
            settings.SetBusMuted( m_busRows[ i ].busName, muted );
        }
    }
    return settings;
}

/*-------------------------------------------------------------------------*/

bool
CAudioSettingsForm::ApplySettings( void )
{GUCEF_TRACE;

    const AUDIO::CAudioSettings previous( m_settings );
    m_settings = GetSettingsFromWidgets();
    SetHasChanges( false );

    // the test sound is over: it was heard with the settings of before
    StopTestSound();

    if ( GUCEF_NULL != m_engine )
        m_settings.ApplyLive( *m_engine );

    if ( m_settings.RequiresDeviceReopen( previous ) )
    {
        if ( m_reopenPending )
        {
            // one device at a time: what was asked for meanwhile is not taken, the rows show what is being opened
            CopyDeviceValues( previous, m_settings );
            ReadDeviceRows();
            SetStatusText( DeviceBusyText );
            return false;
        }
        if ( GUCEF_NULL != m_engine && m_engine->IsInitialized() )
        {
            if ( !StartDeviceReopen( previous ) )
            {
                CopyDeviceValues( previous, m_settings );
                ReadDeviceRows();
                SetStatusText( DeviceOpenFailedText );
                return false;
            }
        }
        else
        {
            // no engine that runs: the device is the application's to open, the page only says that a value of it changed
            NotifyObservers( DeviceReopenRequiredEvent );
        }
    }

    NotifyObservers( SettingsAppliedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::CopyDeviceValues( const AUDIO::CAudioSettings& from, AUDIO::CAudioSettings& to )
{GUCEF_TRACE;

    to.SetDriverName( from.GetDriverName() );
    to.SetDeviceId( from.GetDeviceId() );
    to.SetSampleRate( from.GetSampleRate() );
    to.SetBufferSizeInFrames( from.GetBufferSizeInFrames() );
    to.SetChannelLayout( from.GetChannelLayout() );
    if ( from.HasLimiterMode() )
        to.SetLimiterMode( from.GetLimiterMode() );
    else
        to.ClearLimiterMode();
}

/*-------------------------------------------------------------------------*/

bool
CAudioSettingsForm::StartDeviceReopen( const AUDIO::CAudioSettings& previous )
{GUCEF_TRACE;

    // what the engine runs with (capacities, threading, device options stay as they are), the device level values of the settings over it
    AUDIO::CAudioEngineParams params( m_engine->GetParams() );
    m_settings.ApplyToEngineParams( params );

    // the new device is opened on a task of the engine and swapped in by its owner side; neither is waited for here
    CORE::CFutureResult reopen = m_engine->ReopenDeviceAsync( params );
    if ( reopen.HasNoFuture() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioSettingsForm: the audio engine did not accept the device reopen" );
        return false;
    }

    m_reopen = reopen;
    m_reopenPrevious = previous;
    m_reopenPending = true;
    UpdateEnabledRows();
    SetStatusText( OpeningDeviceText );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::FinishDeviceReopen( bool success )
{GUCEF_TRACE;

    m_reopen = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    m_reopenPending = false;

    // the device values of the settings are the ones in use again when the new device did not open
    if ( !success )
        CopyDeviceValues( m_reopenPrevious, m_settings );
    m_reopenPrevious = AUDIO::CAudioSettings();

    ReadDeviceRows();
    UpdateEnabledRows();
    SetStatusText( success ? CString::Empty : DeviceOpenFailedText );

    CORE::TCloneableBool eventData( success );
    NotifyObservers( DeviceReopenCompletedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::DropDeviceReopen( void )
{GUCEF_TRACE;

    if ( !m_reopenPending )
        return;

    m_reopen = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    m_reopenPrevious = AUDIO::CAudioSettings();
    m_reopenPending = false;
    if ( GUCEF_NULL != GetRootWidget() )
    {
        UpdateEnabledRows();
        SetStatusText( CString::Empty );
    }
}

/*-------------------------------------------------------------------------*/

bool
CAudioSettingsForm::IsDeviceReopenPending( void ) const
{GUCEF_TRACE;

    return m_reopenPending;
}

/*-------------------------------------------------------------------------*/

Int32
CAudioSettingsForm::GetMeterLevel( const AUDIO::CAudioBus* bus )
{GUCEF_TRACE;

    if ( GUCEF_NULL == bus )
        return MeterFloorInDecibels;

    // the loudest channel; atomics, so this can be done at any time
    const AUDIO::CAudioLevelMeter& meter = bus->GetMeter();
    Float32 peak = 0.0f;
    const UInt32 channels = meter.GetChannelCount();
    for ( UInt32 c=0; c<channels; ++c )
    {
        Float32 channelPeak = meter.GetPeak( c );
        if ( channelPeak > peak )
            peak = channelPeak;
    }
    if ( peak <= 0.0f )
        return MeterFloorInDecibels;

    Float32 decibels = AUDIO::CAudioLevelMeter::ToDecibels( peak );
    if ( decibels <= (Float32) MeterFloorInDecibels )
        return MeterFloorInDecibels;
    if ( decibels >= 0.0f )
        return 0;
    return (Int32) ( decibels - 0.5f );     // rounded to the whole dB below zero
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::SetMeterLevel( UInt32 index, const CString& widgetName, Int32 level )
{GUCEF_TRACE;

    if ( index >= (UInt32) m_meterLevels.size() || m_meterLevels[ index ] == level )
        return;
    m_meterLevels[ index ] = level;
    CProgressBar* meter = GetWidgetAs< CProgressBar >( widgetName );
    if ( GUCEF_NULL != meter )
        meter->SetValue( (Float32) level );
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::UpdateMeters( bool force )
{GUCEF_TRACE;

    // nothing to show while the page is not: the values are caught up with in the first frame it is shown again. The page
    // is hidden by its tab not being the selected one, and by the form that holds it not being a shown root of the context
    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL == root || !root->IsEffectivelyVisible() )
        return;
    const CWidget* top = root;
    while ( GUCEF_NULL != top->GetParentWidget() )
        top = top->GetParentWidget();
    if ( GUCEF_NULL != top->GetContext() && !top->GetContext()->IsRootShown( top ) )
        return;

    const UInt32 meterCount = (UInt32) m_busRows.size() + 1;
    if ( force || (UInt32) m_meterLevels.size() != meterCount )
        m_meterLevels.assign( meterCount, 1 );

    AUDIO::CAudioBus* master = GUCEF_NULL;
    const bool running = GUCEF_NULL != m_engine && m_engine->IsInitialized();
    if ( running )
        master = m_engine->GetMasterBus();
    SetMeterLevel( 0, "MasterMeter", GetMeterLevel( master ) );
    for ( UInt32 i=0; i<(UInt32)m_busRows.size(); ++i )
        SetMeterLevel( i + 1, BusRowId( i ) + "Meter", GetMeterLevel( running ? m_engine->FindBus( m_busRows[ i ].busName ) : GUCEF_NULL ) );
}

/*-------------------------------------------------------------------------*/

bool
CAudioSettingsForm::PlayTestSound( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_engine || !m_engine->IsInitialized() )
        return false;

    // a sound that ended by itself is over
    if ( !m_testVoice.IsNULL() && m_testEnded.IsReady() )
    {
        m_testVoice = AUDIO::TAudioVoicePtr();
        m_testEnded = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    }
    if ( !m_testVoice.IsNULL() )
        return false;

    // made in the format the engine mixes in
    const AUDIO::CAudioFormat mixFormat = m_engine->GetMixFormat();
    const UInt32 rate = mixFormat.GetSampleRate();
    const UInt32 channels = mixFormat.GetChannelCount();
    AUDIO::TAudioSourcePtr source;
    switch ( GetSelectedIndex( "TestSignal" ) )
    {
        case 1: source = AUDIO::CAudioTestSignals::MakeChannelCheck( rate, channels ); break;
        case 2: source = AUDIO::CAudioTestSignals::MakeSweep( rate, channels ); break;
        case 3: source = AUDIO::CAudioTestSignals::MakeNoise( rate, channels ); break;
        case 0:
        default: source = AUDIO::CAudioTestSignals::MakeToneLadder( rate, channels ); break;
    }
    if ( source.IsNULL() )
        return false;

    // the master bus unless the application names another and the engine has it
    AUDIO::CAudioBus* bus = m_testBus.IsNULLOrEmpty() ? GUCEF_NULL : m_engine->FindBus( m_testBus );
    AUDIO::TAudioVoicePtr voice = m_engine->CreateVoice( source, bus );
    if ( voice.IsNULL() )
        return false;
    CORE::CFutureResult started = m_engine->Start( voice );
    if ( started.HasNoFuture() )
        return false;
    CORE::CFutureResult ended = m_engine->GetEndedResult( voice );
    if ( ended.HasNoFuture() )
    {
        m_engine->Stop( voice );
        return false;
    }

    m_testVoice = voice;
    m_testEnded = ended;
    UpdateTestSoundButton();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioSettingsForm::StopTestSound( void )
{GUCEF_TRACE;

    if ( m_testVoice.IsNULL() )
        return;

    AUDIO::TAudioVoicePtr voice = m_testVoice;
    m_testVoice = AUDIO::TAudioVoicePtr();
    m_testEnded = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    if ( GUCEF_NULL != m_engine )
        m_engine->Stop( voice );
    if ( GUCEF_NULL != GetRootWidget() )
        UpdateTestSoundButton();
}

/*-------------------------------------------------------------------------*/

bool
CAudioSettingsForm::IsTestSoundPlaying( void ) const
{GUCEF_TRACE;

    return !m_testVoice.IsNULL() && !m_testEnded.IsReady();
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioSettingsForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CAudioSettingsForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
