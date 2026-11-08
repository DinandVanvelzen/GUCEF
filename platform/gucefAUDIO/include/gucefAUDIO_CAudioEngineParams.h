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
 *  (CAudioDeviceParams). Enumerations are written by name.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioEngineParams : public CORE::CIConfigurable
{
    public:

    static const CString ConfigSectionName;         /**< "AudioEngine" */

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
     *  limiterReleaseMs, maxEffectsPerBus, maxSendsPerBus, maxVoiceEffects and maxVoiceSends; and the child "AudioDevice"
     *  with the device parameters
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
