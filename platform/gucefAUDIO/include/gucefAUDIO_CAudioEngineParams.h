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

#ifndef GUCEF_AUDIO_CAUDIOENGINEPARAMS_H
#define GUCEF_AUDIO_CAUDIOENGINEPARAMS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_AUDIO_CAUDIODEVICEPARAMS_H
#include "gucefAUDIO_CAudioDeviceParams.h"
#define GUCEF_AUDIO_CAUDIODEVICEPARAMS_H
#endif /* GUCEF_AUDIO_CAUDIODEVICEPARAMS_H ? */

#ifndef GUCEF_AUDIO_CAUDIOVOICE_H
#include "gucefAUDIO_CAudioVoice.h"
#define GUCEF_AUDIO_CAUDIOVOICE_H
#endif /* GUCEF_AUDIO_CAUDIOVOICE_H ? */

#ifndef GUCEF_AUDIO_CVOICEACTIVITYSETTINGS_H
#include "gucefAUDIO_CVoiceActivitySettings.h"
#define GUCEF_AUDIO_CVOICEACTIVITYSETTINGS_H
#endif /* GUCEF_AUDIO_CVOICEACTIVITYSETTINGS_H ? */

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

/** How the engine gets its work done on threads */
enum EAudioThreadingMode
{
    AUDIOTHREADING_AUTO = 0 ,       /**< threaded where the platform has threads, cooperative where it has not */
    AUDIOTHREADING_THREADED ,       /**< a dedicated thread (known to the task manager) completes futures and raises events by itself */
    AUDIOTHREADING_COOPERATIVE      /**< nothing runs but the audio thread of the device: the owner calls CAudioEngine::Update() regularly for futures and events */
};
typedef enum EAudioThreadingMode TAudioThreadingMode;

/**
 *  What the engine is started with. All of it has a default that works, a program that does not care
 *  about the details sets nothing.
 *
 *  The parameters can be saved to and loaded from a configuration tree (CORE::CIConfigurable), the section "AudioEngine"
 *  of an application's settings: every parameter has an attribute, the device parameters are the child "AudioDevice"
 *  (CAudioDeviceParams), the capture device parameters the child "CaptureDevice". Enumerations are written by name.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioEngineParams : public CORE::CIConfigurable
{
    public:

    static const CString ConfigSectionName;         /**< "AudioEngine" */
    static const CString CaptureDeviceSectionName;  /**< "CaptureDevice", the child with the capture device parameters */
    static const CString PeriodsOptionName;         /**< "periods", the device option with the number of periods in the device buffer (ALSA) */

    CAudioEngineParams( void );

    CAudioEngineParams( const CAudioEngineParams& src );

    CAudioEngineParams& operator=( const CAudioEngineParams& src );

    virtual ~CAudioEngineParams() GUCEF_VIRTUAL_OVERRIDE;

    /** The audio driver to use ("alsa", "null", ...), empty for the best one that works on this machine */
    const CString& GetDriverName( void ) const;
    void SetDriverName( const CString& driverName );

    /** What is asked of the device. The engine adapts to the format it gets */
    const CAudioDeviceParams& GetDeviceParams( void ) const;
    CAudioDeviceParams& GetDeviceParams( void );

    /** Number of sounds that can play at once */
    UInt32 GetMaxVoices( void ) const;
    void SetMaxVoices( UInt32 maxVoices );

    /** Number of buses, the master bus included */
    UInt32 GetMaxBuses( void ) const;
    void SetMaxBuses( UInt32 maxBuses );

    /** Largest piece of audio mixed in one go, in frames. Memory per sound and bus depends on it */
    UInt32 GetMaxBlockFrames( void ) const;
    void SetMaxBlockFrames( UInt32 frames );

    /** Room for commands on their way to the audio thread, and for events on their way back. Both are fixed for the life of the engine */
    UInt32 GetCommandQueueSize( void ) const;
    void SetCommandQueueSize( UInt32 size );
    UInt32 GetEventQueueSize( void ) const;
    void SetEventQueueSize( UInt32 size );

    TAudioThreadingMode GetThreadingMode( void ) const;
    void SetThreadingMode( TAudioThreadingMode mode );

    /** Threaded mode: how often the engine thread looks for futures to complete and events to raise */
    UInt32 GetUpdateIntervalInMs( void ) const;
    void SetUpdateIntervalInMs( UInt32 milliseconds );

    /** Threaded mode: the name of the task manager thread pool the engine thread lives in */
    const CString& GetThreadPoolName( void ) const;
    void SetThreadPoolName( const CString& name );

    /** True (default) to start the device when the engine is initialized */
    bool GetStartDevice( void ) const;
    void SetStartDevice( bool start );

    TAudioPanLaw GetPanLaw( void ) const;
    void SetPanLaw( TAudioPanLaw panLaw );

    /**
     *  The mode of the master limiter stage (see CAudioMixer::SetLimiterMode): lookahead by default. Off switches the
     *  limiter off and keeps the mode it had when on (SetLimiterEnabled( true ) brings that back); instant or lookahead
     *  switches it on in that mode. While the engine runs the mode changes with a device reopen (CAudioEngine::ReopenDevice())
     */
    TAudioLimiterMode GetLimiterMode( void ) const;
    void SetLimiterMode( TAudioLimiterMode mode );

    /**
     *  Whether the master limiter is on (default true). True switches it on in the mode last set other than off
     *  (lookahead unless SetLimiterMode() chose instant), false switches it off: the same as SetLimiterMode( off )
     */
    bool GetLimiterEnabled( void ) const;
    void SetLimiterEnabled( bool enabled );

    /** Ceiling of the lookahead limiter in dBFS, -12 .. 0 (default -0.3). The instant limiter always has 0.98 linear */
    Float32 GetLimiterCeilingInDecibels( void ) const;
    void SetLimiterCeilingInDecibels( Float32 decibels );

    /** Lookahead of the lookahead limiter in milliseconds, 0.5 .. 10 (default 1.5): also the latency it adds */
    Float32 GetLimiterLookaheadInMs( void ) const;
    void SetLimiterLookaheadInMs( Float32 milliseconds );

    /** Release time constant of the limiter in milliseconds, 10 .. 1000 (default 50) */
    Float32 GetLimiterReleaseInMs( void ) const;
    void SetLimiterReleaseInMs( Float32 milliseconds );

    /** Insert effect slots per bus (default 8). Fixed for the life of the engine: the slots are allocated with the bus */
    UInt32 GetMaxEffectsPerBus( void ) const;
    void SetMaxEffectsPerBus( UInt32 count );

    /** Send slots per bus (default 4). Fixed for the life of the engine */
    UInt32 GetMaxSendsPerBus( void ) const;
    void SetMaxSendsPerBus( UInt32 count );

    /** Effect slots per voice (default 4), for the voice effect chains. Fixed for the life of the engine */
    UInt32 GetMaxVoiceEffects( void ) const;
    void SetMaxVoiceEffects( UInt32 count );

    /** Send slots per voice (default 2), for the voice sends. Fixed for the life of the engine */
    UInt32 GetMaxVoiceSends( void ) const;
    void SetMaxVoiceSends( UInt32 count );

    /**
     *  Whether the engine opens and starts a capture device next to the playback device (default false). A capture device
     *  that cannot be opened is logged and leaves capture unavailable; the engine still plays
     */
    bool GetCaptureEnabled( void ) const;
    void SetCaptureEnabled( bool enabled );

    /**
     *  What is asked of the capture device: the default device unless an id is set. It is opened with the driver of the
     *  playback device, always in the capture direction whatever the direction says
     */
    const CAudioDeviceParams& GetCaptureDeviceParams( void ) const;
    CAudioDeviceParams& GetCaptureDeviceParams( void );

    /** Sample rate of the capture path: 0 (default) for the rate of the capture device, otherwise 8000 to 48000 */
    UInt32 GetCaptureProcessingRate( void ) const;
    void SetCaptureProcessingRate( UInt32 sampleRate );

    /** The channel of the capture device the capture path takes, from 0; -1 (default) for the mean of all channels */
    Int32 GetCaptureChannel( void ) const;
    void SetCaptureChannel( Int32 channel );

    /** Room for capture consumers (default 4). Fixed for the life of the engine */
    UInt32 GetMaxCaptureConsumers( void ) const;
    void SetMaxCaptureConsumers( UInt32 count );

    /** Effect slots per capture stage (default 8). Fixed for the life of the engine */
    UInt32 GetMaxCaptureEffects( void ) const;
    void SetMaxCaptureEffects( UInt32 count );

    /** Room for commands on their way to the capture thread, and for their events (default 64) */
    UInt32 GetCaptureCommandQueueSize( void ) const;
    void SetCaptureCommandQueueSize( UInt32 size );

    /** The settings of the voice activity detection of the capture path (the child "VoiceActivity" of the configuration), applied when the capture device is opened */
    const CVoiceActivitySettings& GetVoiceActivitySettings( void ) const;
    CVoiceActivitySettings& GetVoiceActivitySettings( void );

    /**
     *  Writes the values of a latency profile into the parameters, so a saved configuration has them explicitly and
     *  they can be edited afterwards. It sets, for the playback device and in the same way for the capture device:
     *   - AUDIOLATENCYPROFILE_LOW: the device period (CAudioDeviceParams::SetBufferSizeInFrames()) to 5 ms at 'sampleRate'
     *     (240 frames at 48 kHz, 80 at 16 kHz), the option "periods" to 3, the option "realtimePriority" to 70 (the
     *     default of MT::CRealtimeSchedulingScope); and for the engine the limiter mode to instant, which adds no latency.
     *   - AUDIOLATENCYPROFILE_DEFAULT: the device period to 0 and the option "periods" removed (the driver chooses), the
     *     option "realtimePriority" to 0, and the limiter mode to lookahead.
     *  A limiter that is switched off stays off, only the mode it has when it is on changes. Nothing else is touched:
     *  other options, the device id, the preferred format, block sizes and queue sizes keep their values. 'sampleRate'
     *  0 means 48000.
     */
    void ApplyLatencyProfile( TAudioLatencyProfile profile, UInt32 sampleRate = 48000 );

    /** "default" or "low" */
    static const char* LatencyProfileToString( TAudioLatencyProfile profile );

    /** The reverse of LatencyProfileToString() without regard for case. False, and 'profile' as it was, for text that is not a profile */
    static bool StringToLatencyProfile( const CString& text, TAudioLatencyProfile& profile );

    /** "auto", "threaded" or "cooperative" */
    static const char* ThreadingModeToString( TAudioThreadingMode mode );

    /** The reverse of ThreadingModeToString() without regard for case. False, and 'mode' as it was, for text that is not a mode */
    static bool StringToThreadingMode( const CString& text, TAudioThreadingMode& mode );

    /** "constantPower" or "balance" */
    static const char* PanLawToString( TAudioPanLaw panLaw );

    /** The reverse of PanLawToString() without regard for case. False, and 'panLaw' as it was, for text that is not a pan law */
    static bool StringToPanLaw( const CString& text, TAudioPanLaw& panLaw );

    /**
     *  Writes the section "AudioEngine" (kept when it is there): attributes driver, maxVoices, maxBuses, maxBlockFrames,
     *  commandQueueSize, eventQueueSize, threadingMode, updateIntervalMs, threadPool, startDevice, panLaw, limiterEnabled,
     *  limiterMode (the mode the limiter works in when it is on: instant or lookahead), limiterCeilingDb, limiterLookaheadMs,
     *  limiterReleaseMs, maxEffectsPerBus, maxSendsPerBus, maxVoiceEffects, maxVoiceSends, captureEnabled,
     *  captureProcessingRate, captureChannel, maxCaptureConsumers, maxCaptureEffects and captureCommandQueueSize; the child
     *  "AudioDevice" with the device parameters, and the child "CaptureDevice" with the capture device parameters (as its
     *  own child "AudioDevice"), and the child "VoiceActivity" with the settings of the voice activity detection (only when they are not the defaults)
     */
    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Reads the section from the tree, or the tree itself when it is that section. False when there is no such section.
     *  What the section does not have keeps its value, names that are not valid are logged and ignored. A limiterMode of
     *  "off" switches the limiter off (as SetLimiterMode( AUDIOLIMITERMODE_OFF )), limiterEnabled, when present, has the last say
     */
    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CString m_driverName;
    CAudioDeviceParams m_deviceParams;
    UInt32 m_maxVoices;
    UInt32 m_maxBuses;
    UInt32 m_maxBlockFrames;
    UInt32 m_commandQueueSize;
    UInt32 m_eventQueueSize;
    TAudioThreadingMode m_threadingMode;
    UInt32 m_updateIntervalInMs;
    CString m_threadPoolName;
    bool m_startDevice;
    TAudioPanLaw m_panLaw;
    bool m_limiterEnabled;
    TAudioLimiterMode m_limiterOnMode;      /**< the mode of the limiter when it is enabled: instant or lookahead */
    Float32 m_limiterCeilingInDecibels;
    Float32 m_limiterLookaheadInMs;
    Float32 m_limiterReleaseInMs;
    UInt32 m_maxEffectsPerBus;
    UInt32 m_maxSendsPerBus;
    UInt32 m_maxVoiceEffects;
    UInt32 m_maxVoiceSends;
    bool m_captureEnabled;
    CAudioDeviceParams m_captureDeviceParams;
    UInt32 m_captureProcessingRate;
    Int32 m_captureChannel;
    UInt32 m_maxCaptureConsumers;
    UInt32 m_maxCaptureEffects;
    UInt32 m_captureCommandQueueSize;
    CVoiceActivitySettings m_voiceActivitySettings;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOENGINEPARAMS_H ? */
