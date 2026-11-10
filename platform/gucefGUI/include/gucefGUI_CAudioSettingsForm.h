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

#ifndef GUCEF_GUI_CAUDIOSETTINGSFORM_H
#define GUCEF_GUI_CAUDIOSETTINGSFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_AUDIO_CAUDIOSETTINGS_H
#include "gucefAUDIO_CAudioSettings.h"
#define GUCEF_AUDIO_CAUDIOSETTINGS_H
#endif /* GUCEF_AUDIO_CAUDIOSETTINGS_H ? */

#ifndef GUCEF_AUDIO_CAUDIODEVICEINFO_H
#include "gucefAUDIO_CAudioDeviceInfo.h"
#define GUCEF_AUDIO_CAUDIODEVICEINFO_H
#endif /* GUCEF_AUDIO_CAUDIODEVICEINFO_H ? */

#ifndef GUCEF_AUDIO_CAUDIOVOICE_H
#include "gucefAUDIO_CAudioVoice.h"
#define GUCEF_AUDIO_CAUDIOVOICE_H
#endif /* GUCEF_AUDIO_CAUDIOVOICE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOENGINE_H
#include "gucefAUDIO_CAudioEngine.h"
#define GUCEF_AUDIO_CAUDIOENGINE_H
#endif /* GUCEF_AUDIO_CAUDIOENGINE_H ? */

#ifndef GUCEF_GUI_CSETTINGSPAGEFORM_H
#include "gucefGUI_CSettingsPageForm.h"
#define GUCEF_GUI_CSETTINGSPAGEFORM_H
#endif /* GUCEF_GUI_CSETTINGSPAGEFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The audio settings: a prebaked page that shows an AUDIO::CAudioSettings and applies it. Registered as the settings
 *  page "Audio". The rows, in a scroll panel (Body) above a status line (Status), are named by their widgets:
 *
 *      Driver          "Automatic" and the audio drivers there are; the null driver is "None (silent)". The row is
 *                      hidden when there is at most one driver besides the null driver
 *      OutputDevice    "Default device" and the output devices of the driver that is chosen or in use
 *      SampleRate      the device default and the usual sample rates
 *      BufferSize      the driver default and sizes in frames, each with its duration at the chosen sample rate
 *      Latency         the latency of the device that is open, with the latency of the master limiter next to it
 *                      when it looks ahead (read only)
 *      SpeakerLayout   the device default, mono, stereo, quadraphonic, 5.1 and 7.1 as far as the device has the speakers
 *      MasterVolume    with MasterPercent and MasterMute: slider 0 to 100 % of the master bus
 *      Bus<n>Volume    the same for each bus declared with AddBusRow(): Bus<n>Label, Bus<n>Percent, Bus<n>Mute
 *      Limiter         the master limiter: off, instant or lookahead. Until the user picks one the row shows the mode
 *                      the running engine uses, or the engine default without an engine
 *      TestSignal      with TestSound, the button that plays the signal on the test bus and stops it again
 *      MasterMeter     with Bus<n>Meter: progress bars with the peak level of the master bus and of the buses, from
 *                      -60 to 0 dBFS, as a thin bar under the sliders. Shown when there is a running engine
 *
 *  Volumes follow the law of CAudioSettings::PercentToDecibels(). Nothing reaches the engine before Apply, there is no
 *  preview. Apply takes the values into the settings and sets volume and mute of the master bus and of the buses on the
 *  engine that was given with SetAudioEngine(). When a device level value changed (driver, device, sample rate, buffer
 *  size, speaker layout, limiter) the page opens the device again itself when the engine runs, with
 *  CAudioEngine::ReopenDeviceAsync(): buses, voices and effects stay valid. The Status line says that the device is
 *  being opened and the device rows are disabled meanwhile. The page checks the outcome once per frame and never waits
 *  for it: the application keeps calling CAudioEngine::Update() when the engine is cooperative. On success the latency
 *  and the device lists are shown again; on failure the device level values return to the ones in use, the rows show
 *  them and Status says that the previous device remains in use. Both end in DeviceReopenCompletedEvent. Without a
 *  running engine Apply only updates the settings and, when a device level value changed, notifies
 *  DeviceReopenRequiredEvent: the application opens the device (CAudioSettings::ApplyToEngineParams(),
 *  CAudioEngine::Initialize() and its buses, CAudioSettings::ApplyLive(), SetAudioEngine()). The page does not save the
 *  settings: the application does, on SettingsAppliedEvent and DeviceReopenCompletedEvent.
 *
 *  The meters are read from the CAudioLevelMeter of the buses once per frame, only while the page is shown, and a
 *  widget is only touched when the whole dB value it shows changes.
 *
 *  Use, with the page added to a CSettingsForm before Build():
 *
 *      CAudioSettingsForm* page = static_cast< CAudioSettingsForm* >( settingsForm.AddPage( "Audio" ) );
 *      page->AddBusRow( "music", "Music" );
 *      page->SetAudioSettings( audioSettings );
 *      page->SetAudioEngine( &engine );
 *
 *  Thread: the GUI thread, which also has to be the owner thread of the engine (the thread that creates its voices
 *  and runs Update()). The page never waits for audio work: futures are only asked whether they are ready.
 */
class GUCEF_GUI_PUBLIC_CPP CAudioSettingsForm : public CSettingsPageForm
{
    public:

    static const CORE::CEvent DeviceReopenRequiredEvent;    /**< Apply changed a device level value and the page has no running engine to reopen: the application opens the device again. No data */
    static const CORE::CEvent DeviceReopenCompletedEvent;   /**< A device reopen the page started has ended. Data: CORE::TCloneableBool, true for success. The settings are the ones in use then */

    static const CString AutomaticDriverChoice;
    static const CString NullDriverChoice;
    static const CString DefaultDeviceChoice;
    static const CString DefaultDriverBufferChoice;
    static const CString DefaultSpeakersChoice;
    static const CString PlayTestSoundText;
    static const CString StopTestSoundText;
    static const CString DeviceLostText;
    static const CString OpeningDeviceText;
    static const CString DeviceOpenFailedText;
    static const CString DeviceBusyText;

    static void RegisterEvents( void );

    CAudioSettingsForm( void );

    virtual ~CAudioSettingsForm() GUCEF_VIRTUAL_OVERRIDE;

    /** The settings in use; ReadSettings() shows them */
    void SetAudioSettings( const AUDIO::CAudioSettings& settings );

    const AUDIO::CAudioSettings& GetAudioSettings( void ) const;

    /** The engine whose buses are set at Apply and where the test sound plays; not owned, set to NULL before the engine is destroyed */
    void SetAudioEngine( AUDIO::CAudioEngine* engine );

    AUDIO::CAudioEngine* GetAudioEngine( void ) const;

    /** Declares a row for the volume of the bus with that name, before the form is built; the order of the calls is the order of the rows. False for a name that is empty or already has a row, or when the page is built */
    bool AddBusRow( const CString& busName, const CString& label );

    /** The bus the test sound plays on; empty for the master bus, which is also where it plays when the engine has no such bus */
    void SetTestBus( const CString& busName );

    const CString& GetTestBus( void ) const;

    /** The settings as the widgets have them now */
    AUDIO::CAudioSettings GetSettingsFromWidgets( void );

    /** Starts the signal chosen in TestSignal on the test bus. False without a running engine, when it plays already or when it cannot be started */
    bool PlayTestSound( void );

    /** Stops the test sound, if there is one */
    void StopTestSound( void );

    bool IsTestSoundPlaying( void ) const;

    /** True from the Apply that started a device reopen until the frame that saw it end */
    bool IsDeviceReopenPending( void ) const;

    /** What the Status line says */
    CString GetStatusText( void );

    virtual CString GetTitle( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void ReadSettings( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ApplySettings( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void BuildLayout( CORE::CDataNode& layout ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnValueChanged( CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CAudioSettingsForm > TEventCallback;

    /** One choice of a list: what is shown, the name or identifier it stands for and the number it stands for */
    struct SChoice
    {
        CString text;
        CString id;
        UInt32 number;

        SChoice( void ) : text(), id(), number( 0 ) {}
        SChoice( const CString& choiceText, const CString& choiceId, UInt32 choiceNumber ) : text( choiceText ), id( choiceId ), number( choiceNumber ) {}
    };
    typedef GUCEF::vector< SChoice > TChoiceVector;

    /** The bus rows as declared */
    struct SBusRow
    {
        CString busName;
        CString label;
    };
    typedef GUCEF::vector< SBusRow > TBusRowVector;

    /** The device level values as the lists have them selected */
    struct SDeviceSelection
    {
        CString driverName;
        CString deviceId;
        UInt32 sampleRate;
        UInt32 bufferSize;
        UInt32 channelLayout;

        SDeviceSelection( void ) : driverName(), deviceId(), sampleRate( 0 ), bufferSize( 0 ), channelLayout( 0 ) {}
    };

    typedef GUCEF::vector< CString > TStringVector;

    void OnTestSoundActivated( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData );

    void OnDeviceLost( CORE::CNotifier* notifier    ,
                       const CORE::CEvent& eventId  ,
                       CORE::CICloneable* eventData );

    void OnFrameStarted( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    /** Applying: false when the reopen could not be started. The page never waits for it, see OnFrameStarted() */
    bool StartDeviceReopen( const AUDIO::CAudioSettings& previous );

    /** The reopen has ended: the outcome goes to the rows, the status and DeviceReopenCompletedEvent */
    void FinishDeviceReopen( bool success );

    /** Forgets a reopen without an outcome (the engine of the page changed) */
    void DropDeviceReopen( void );

    /** The device level values of 'from' into 'to' */
    static void CopyDeviceValues( const AUDIO::CAudioSettings& from, AUDIO::CAudioSettings& to );

    /** Shows the device level rows (lists, latency, limiter) for the settings in use */
    void ReadDeviceRows( void );

    /** Meters of the master bus and the bus rows. With 'force' every widget is set, else only the ones whose value changed */
    void UpdateMeters( bool force );

    void SetMeterLevel( UInt32 index, const CString& widgetName, Int32 level );

    /** The whole dB value of the loudest channel of the meter of the bus, between the floor of the meter and 0; the floor for no bus */
    static Int32 GetMeterLevel( const AUDIO::CAudioBus* bus );

    /** The lists of the device rows, each depending on the ones before it: from the driver on, from the device on, from the sample rate on */
    void FillDriverList( const SDeviceSelection& selection );
    void FillDeviceList( const SDeviceSelection& selection );
    void FillSampleRateList( const SDeviceSelection& selection );
    void FillBufferSizeList( const SDeviceSelection& selection );
    void FillSpeakerLayoutList( const SDeviceSelection& selection );

    SDeviceSelection GetDeviceSelectionFromWidgets( void );

    /** The name of the driver that "Automatic" stands for: the one in use, else the best there is */
    CString ResolveDriverName( const CString& driverName ) const;

    /** The information the device list has about the device with that identifier, the default device for an empty one */
    bool FindDeviceInfo( const CString& deviceId, AUDIO::CAudioDeviceInfo& info ) const;

    void FillCombobox( const CString& widgetName, const TChoiceVector& choices, Int32 selectedIndex );

    Int32 GetSelectedIndex( const CString& widgetName );

    /** The value of a bus row as the page shows it: what the settings have, else what the engine has */
    void GetShownBusValues( const CString& busName, Float32& decibels, bool& muted ) const;

    void ReadVolumeRow( const CString& rowId, const CString& busName, bool master );

    void UpdatePercentLabel( const CString& sliderName );

    void UpdateLatency( void );

    void UpdateEnabledRows( void );

    void SetRowEnabled( const CString& rowId, bool enabled );

    /** Puts the rows below each other, the ones that are not shown take no room */
    void PositionRows( void );

    void SetStatusText( const CString& text );

    void UpdateTestSoundButton( void );

    void SetEngineSubscription( AUDIO::CAudioEngine* engine );

    static CString RowName( const CString& rowId );

    static CString BusRowId( UInt32 index );

    static CString DurationText( UInt32 frames, UInt32 sampleRate );

    static CString MillisecondsText( UInt32 frames, UInt32 sampleRate );

    CAudioSettingsForm( const CAudioSettingsForm& src );
    CAudioSettingsForm& operator=( const CAudioSettingsForm& src );

    private:

    AUDIO::CAudioSettings m_settings;
    AUDIO::CAudioEngine* m_engine;
    TBusRowVector m_busRows;
    CString m_testBus;

    TStringVector m_rowIds;                         /**< the rows in the order they are shown */
    TChoiceVector m_driverChoices;
    TChoiceVector m_deviceChoices;
    TChoiceVector m_sampleRateChoices;
    TChoiceVector m_bufferSizeChoices;
    TChoiceVector m_speakerLayoutChoices;
    TChoiceVector m_limiterChoices;
    AUDIO::TAudioDeviceInfoVector m_devices;        /**< the output devices of the driver the device list shows */
    AUDIO::CAudioDeviceInfo m_defaultDevice;
    bool m_hasDefaultDevice;
    bool m_driverRowShown;
    Int32 m_shownLimiterIndex;                      /**< the entry of the limiter list that ReadSettings() chose */

    AUDIO::TAudioVoicePtr m_testVoice;
    CORE::CFutureResult m_testEnded;
    MT::CTAtomic< UInt32 > m_deviceLost;

    CORE::CFutureResult m_reopen;                   /**< the reopen the page started; only asked whether it is ready */
    AUDIO::CAudioSettings m_reopenPrevious;         /**< the settings in use before the Apply that started it */
    bool m_reopenPending;
    GUCEF::vector< Int32 > m_meterLevels;           /**< what the meters show: master first, then the bus rows */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CAUDIOSETTINGSFORM_H ? */
