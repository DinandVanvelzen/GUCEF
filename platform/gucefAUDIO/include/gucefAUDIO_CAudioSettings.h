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

#ifndef GUCEF_AUDIO_CAUDIOSETTINGS_H
#define GUCEF_AUDIO_CAUDIOSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

#ifndef GUCEF_AUDIO_ETYPES_H
#include "gucefAUDIO_ETypes.h"
#define GUCEF_AUDIO_ETYPES_H
#endif /* GUCEF_AUDIO_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CAudioEngine;
class CAudioEngineParams;

/*-------------------------------------------------------------------------*/

/**
 *  The audio preferences of the user: which device is used and how it is opened, the volume and mute of the master
 *  bus and of the buses the application names, and the master limiter. It is the user's overlay on the developer
 *  configuration of CAudioEngineParams, and has no user interface of its own: a settings page shows it
 *  and applications without a user interface (tools, servers) use it as it is.
 *
 *  Two kinds of values:
 *   - device level: driver, device, capture device, sample rate, buffer size, speaker layout, limiter mode and latency
 *     profile. They take effect when the device is opened (ApplyToEngineParams() before CAudioEngine::Initialize()) or reopened. Empty and zero values
 *     mean 'the engine decides', and are left out of the engine parameters.
 *   - live: volume and mute. They take effect at once on an engine that runs (ApplyLive()).
 *
 *  Volume is kept in decibels, 0 dB is unchanged and nothing is louder. Settings pages let the user pick a percentage,
 *  PercentToDecibels() and DecibelsToPercent() are the volume law they share: 0 % is silence, 1 to 100 % are
 *  -59.4 to 0 dB in equal steps.
 *
 *  Buses are kept by name, whether or not the engine has a bus of that name right now, so the settings of a bus that
 *  a page does not show survive a save. The volume of a named bus is the user's volume: ApplyLive() replaces whatever
 *  volume the bus had, a trim designed into the bus layout belongs on a bus below it.
 *
 *  Saved as the section "AudioSettings" of a config tree. Values that a loaded tree does not have keep their value.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioSettings : public CORE::CIConfigurable
{
    public:

    static const CString ConfigSectionName;         /**< "AudioSettings" */
    static const Float32 SilenceInDecibels;         /**< -144: this volume or a lower one is a gain of 0 */

    /** What is kept for a bus */
    struct SBusSettings
    {
        Float32 volumeInDecibels;
        bool muted;

        SBusSettings( void ) : volumeInDecibels( 0.0f ), muted( false ) {}
    };
    typedef GUCEF::map< CString, SBusSettings > TBusSettingsMap;

    CAudioSettings( void );

    CAudioSettings( const CAudioSettings& src );

    virtual ~CAudioSettings() GUCEF_VIRTUAL_OVERRIDE;

    CAudioSettings& operator=( const CAudioSettings& src );

    /*
     *  Device level: take effect when the device is opened or reopened
     */

    /** The audio driver by name ("alsa", "null", ...); empty for the best one that works */
    void SetDriverName( const CString& driverName );
    const CString& GetDriverName( void ) const;

    /** The device as its driver knows it (CAudioDeviceInfo::GetId()); empty for the default device of the driver */
    void SetDeviceId( const CString& deviceId );
    const CString& GetDeviceId( void ) const;

    /** The capture device as its driver knows it (CAudioDeviceInfo::GetId()); empty for the default capture device of the driver */
    void SetCaptureDeviceId( const CString& deviceId );
    const CString& GetCaptureDeviceId( void ) const;

    /** The sample rate in Hz; 0 for the one the device prefers */
    void SetSampleRate( UInt32 sampleRate );
    UInt32 GetSampleRate( void ) const;

    /** The size of the pieces the device asks audio in, in frames; 0 for the driver default */
    void SetBufferSizeInFrames( UInt32 frames );
    UInt32 GetBufferSizeInFrames( void ) const;

    /** The speakers as a mask of ESpeakerPosition values; 0 for the layout the device prefers */
    void SetChannelLayout( UInt32 channelLayout );
    UInt32 GetChannelLayout( void ) const;

    /** The mode of the master limiter. Until it is set (or after ClearLimiterMode()) the engine parameters decide */
    void SetLimiterMode( TAudioLimiterMode mode );
    TAudioLimiterMode GetLimiterMode( void ) const;
    void ClearLimiterMode( void );
    bool HasLimiterMode( void ) const;

    /**
     *  The latency profile (CAudioEngineParams::ApplyLatencyProfile()). AUDIOLATENCYPROFILE_DEFAULT, the default, is
     *  'the engine decides': ApplyToEngineParams() leaves the parameters as they are, it does not take them back to the
     *  default values of the profile. The profile is applied before the buffer size and the limiter mode of these
     *  settings, so those two win when both are set
     */
    void SetLatencyProfile( TAudioLatencyProfile profile );
    TAudioLatencyProfile GetLatencyProfile( void ) const;

    /*
     *  Live: take effect at once on a running engine
     */

    /** The volume of the master bus; this value or a lower one (SilenceInDecibels) is silence. Above 0 dB is limited to 0 dB */
    void SetMasterVolumeInDecibels( Float32 decibels );
    Float32 GetMasterVolumeInDecibels( void ) const;

    void SetMasterMuted( bool muted );
    bool IsMasterMuted( void ) const;

    /** The volume of the bus with that name, which is added to the settings when it was not there. Same limits as the master volume */
    void SetBusVolumeInDecibels( const CString& busName, Float32 decibels );

    /** The mute of the bus with that name, which is added to the settings when it was not there */
    void SetBusMuted( const CString& busName, bool muted );

    /** False when the settings have nothing for a bus with that name */
    bool GetBusSettings( const CString& busName, SBusSettings& settings ) const;

    const TBusSettingsMap& GetAllBusSettings( void ) const;

    /** Forgets the settings of the bus with that name. False when there were none */
    bool RemoveBusSettings( const CString& busName );

    /** Whether a device level value differs from the one in 'applied': the settings the device was opened with */
    bool RequiresDeviceReopen( const CAudioSettings& applied ) const;

    /**
     *  Writes the device level values that are set into 'params': driver name, device, capture device, latency profile
     *  (with the sample rate of these settings, or else the preferred sample rate of 'params', or else 48000), sample rate,
     *  buffer size, speaker layout and limiter mode. What is not set is left as it is in 'params'. The limiter mode goes to
     *  CAudioEngineParams::SetLimiterMode(): off switches the limiter off and keeps the mode it has when on.
     */
    void ApplyToEngineParams( CAudioEngineParams& params ) const;

    /**
     *  Sets volume and mute of the master bus and of every bus in the settings that the engine has. A bus the engine does
     *  not have is skipped with a line in the log. Nothing happens for an engine that is not initialized.
     *  Returns the number of buses (the master bus included) that were set.
     */
    UInt32 ApplyLive( CAudioEngine& engine ) const;

    /** Percent of a settings page slider (0 to 100, more is 100) to decibels: 0 is SilenceInDecibels, 1 to 100 are 60 * ( percent / 100 - 1 ) */
    static Float32 PercentToDecibels( UInt32 percent );

    /** The reverse of PercentToDecibels(), rounded to the nearest percent: SilenceInDecibels or less is 0, 0 dB or more is 100, and so is anything quieter than -60 dB */
    static UInt32 DecibelsToPercent( Float32 decibels );

    /** "default" for 0, "mono", "stereo", "quad", "5.1" and "7.1" for the usual layouts of 1, 2, 4, 6 and 8 channels, else the mask as a number */
    static CString ChannelLayoutToString( UInt32 channelLayout );

    /** The reverse of ChannelLayoutToString() without regard for case; 'defaultLayout' for text that is neither a name nor a number */
    static UInt32 StringToChannelLayout( const CString& text, UInt32 defaultLayout );

    /** "off", "instant" or "lookahead" */
    static const char* LimiterModeToString( TAudioLimiterMode mode );

    /** The reverse of LimiterModeToString() without regard for case. False, and 'mode' as it was, for text that is not a mode */
    static bool StringToLimiterMode( const CString& text, TAudioLimiterMode& mode );

    /** Adds the section "AudioSettings" to the tree, replacing the one that was there */
    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the section from the tree, or the tree itself when it is that section. False when there is no such section */
    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    static Float32 LimitVolume( Float32 decibels );

    private:

    CString m_driverName;
    CString m_deviceId;
    CString m_captureDeviceId;
    UInt32 m_sampleRate;
    UInt32 m_bufferSizeInFrames;
    UInt32 m_channelLayout;
    TAudioLimiterMode m_limiterMode;
    bool m_hasLimiterMode;
    TAudioLatencyProfile m_latencyProfile;
    SBusSettings m_master;
    TBusSettingsMap m_buses;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOSETTINGS_H ? */
