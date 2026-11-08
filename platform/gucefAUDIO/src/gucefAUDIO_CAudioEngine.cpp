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

#include <string.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CDENORMALFLUSHSCOPE_H
#include "gucefCORE_CDenormalFlushScope.h"
#define GUCEF_CORE_CDENORMALFLUSHSCOPE_H
#endif /* GUCEF_CORE_CDENORMALFLUSHSCOPE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#include "gucefAUDIO_CAudioGlobal.h"
#define GUCEF_AUDIO_CAUDIOGLOBAL_H
#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */

#ifndef GUCEF_AUDIO_CAUDIODRIVERREGISTRY_H
#include "gucefAUDIO_CAudioDriverRegistry.h"
#define GUCEF_AUDIO_CAUDIODRIVERREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIODRIVERREGISTRY_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCONVERTER_H
#include "gucefAUDIO_CAudioConverter.h"
#define GUCEF_AUDIO_CAUDIOCONVERTER_H
#endif /* GUCEF_AUDIO_CAUDIOCONVERTER_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#include "gucefAUDIO_CAudioEffectRegistry.h"
#define GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H ? */

#ifndef GUCEF_AUDIO_CAUDIOBUSROUTING_H
#include "gucefAUDIO_CAudioBusRouting.h"
#define GUCEF_AUDIO_CAUDIOBUSROUTING_H
#endif /* GUCEF_AUDIO_CAUDIOBUSROUTING_H ? */

#ifndef GUCEF_AUDIO_CAUDIOPUSHDEVICE_H
#include "gucefAUDIO_CAudioPushDevice.h"
#define GUCEF_AUDIO_CAUDIOPUSHDEVICE_H
#endif /* GUCEF_AUDIO_CAUDIOPUSHDEVICE_H ? */

#include "gucefAUDIO_CAudioEngine.h"

#ifndef GUCEF_AUDIO_CHALFDUPLEXCONTROLLER_H
#include "gucefAUDIO_CHalfDuplexController.h"
#define GUCEF_AUDIO_CHALFDUPLEXCONTROLLER_H
#endif /* GUCEF_AUDIO_CHALFDUPLEXCONTROLLER_H ? */

#ifndef GUCEF_AUDIO_CACOUSTICECHOCANCELLER_H
#include "gucefAUDIO_CAcousticEchoCanceller.h"
#define GUCEF_AUDIO_CACOUSTICECHOCANCELLER_H
#endif /* GUCEF_AUDIO_CACOUSTICECHOCANCELLER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CAudioEngine::VoiceEndedEvent = "GUCEF::AUDIO::CAudioEngine::VoiceEndedEvent";
const CORE::CEvent CAudioEngine::DeviceLostEvent = "GUCEF::AUDIO::CAudioEngine::DeviceLostEvent";
const CORE::CEvent CAudioEngine::CaptureDeviceLostEvent = "GUCEF::AUDIO::CAudioEngine::CaptureDeviceLostEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The mode the limiter of 'params' works in when it is on, also while it is off */
TAudioLimiterMode
LimiterModeWhenEnabled( const CAudioEngineParams& params )
{
    if ( params.GetLimiterEnabled() )
        return params.GetLimiterMode();
    CAudioEngineParams on( params );
    on.SetLimiterEnabled( true );
    return on.GetLimiterMode();
}

/** The plain pointer of a voice that is held by a shared pointer, which is what the engine tracks voices by */
CAudioVoice*
RawVoice( const TAudioVoicePtr& voice )
{
    TAudioVoicePtr copy( voice );
    return copy.GetPointerAlways();
}

/** The same for an effect */
CAudioEffect*
RawEffect( const TAudioEffectPtr& effect )
{
    TAudioEffectPtr copy( effect );
    return copy.GetPointerAlways();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The thread of the engine in threaded mode: a task of the task manager that keeps a dedicated thread, which is
 *  how the task manager gets to know about every thread. It does one thing, call Update().
 */
class CAudioEngineUpdateTask : public CORE::CTaskConsumer
{
    public:

    CAudioEngineUpdateTask( CAudioEngine* engine, UInt32 intervalInMs )
        : CORE::CTaskConsumer(), m_engine( engine ), m_intervalInMs( intervalInMs ), m_lock(), m_stopped( false ), m_ended( false )
    {GUCEF_TRACE;
    }

    virtual ~CAudioEngineUpdateTask() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
    }

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return "GUCEF::AUDIO::CAudioEngineUpdateTask";
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        static const CORE::CString typeName = "GUCEF::AUDIO::CAudioEngineUpdateTask";
        return typeName;
    }

    virtual bool OnTaskCycle( CORE::CTaskPtr ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        // The lock is held while the engine is updated, so once Disconnect() has the lock the engine is never touched again
        MT::CScopeMutex lock( m_lock );
        if ( m_stopped )
            return true;
        m_engine->Update();
        RequestTaskCycleDelayInMs( m_intervalInMs );
        return false;           // keep going until asked to stop
    }

    virtual void OnTaskEnded( CORE::CTaskPtr, bool ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_ended.Store( true );
    }

    /** After this returns the task will not call the engine any more, and ends itself at its next cycle */
    void Disconnect( void )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        m_stopped = true;
    }

    bool HasEnded( void ) const
    {GUCEF_TRACE;

        return m_ended.Load();
    }

    private:

    CAudioEngine* m_engine;
    UInt32 m_intervalInMs;
    MT::CMutex m_lock;
    bool m_stopped;
    MT::CTAtomic< bool > m_ended;
};

/*-------------------------------------------------------------------------*/

/**
 *  A device reopen on its way: the new device is opened by a pool task (or, where no task could be started, on the owner
 *  side in Update()), the engine picks it up on the owner side. Shared by the engine and the task, so either can go first.
 */
class CAudioEngineReopenRequest
{
    public:

    CAudioEngineReopenRequest( const CAudioEngineParams& params, IAudioDeviceCallback* callback )
        : m_params( params ), m_callback( callback ), m_lock(), m_done( false ), m_opened( false ), m_abandoned( false ), m_device(), m_usedDriverName()
    {GUCEF_TRACE;
    }

    const CAudioEngineParams& GetParams( void ) const
    {GUCEF_TRACE;
        return m_params;
    }

    /** Opens the device (not started). Whichever thread does it */
    void Open( void )
    {GUCEF_TRACE;

        TAudioDevicePtr device;
        CString usedDriverName;
        const bool opened = CAudioGlobal::Instance()->GetAudioDriverRegistry().OpenDevice( m_params.GetDriverName(), m_params.GetDeviceParams(), m_callback, device, usedDriverName ) && !device.IsNULL();

        TAudioDevicePtr unwanted;
        {
            MT::CScopeMutex lock( m_lock );
            if ( m_abandoned )
                unwanted = device;          // the engine is gone or no longer interested: closed below, outside the lock
            else
            {
                m_device = device;
                m_usedDriverName = usedDriverName;
                m_opened = opened;
            }
            m_done = true;
        }
    }

    bool IsDone( void ) const
    {GUCEF_TRACE;
        MT::CScopeMutex lock( m_lock );
        return m_done;
    }

    /** Owner side, once done: the device that was opened (null when none) */
    bool TakeDevice( TAudioDevicePtr& device, CString& usedDriverName )
    {GUCEF_TRACE;
        MT::CScopeMutex lock( m_lock );
        device = m_device;
        usedDriverName = m_usedDriverName;
        m_device.Unlink();
        return m_opened && !device.IsNULL();
    }

    /** The engine gives up on the request: a device opened later is closed by the opener */
    void Abandon( void )
    {GUCEF_TRACE;
        TAudioDevicePtr unwanted;
        {
            MT::CScopeMutex lock( m_lock );
            m_abandoned = true;
            unwanted = m_device;
            m_device.Unlink();
        }
    }

    private:

    CAudioEngineParams m_params;
    IAudioDeviceCallback* m_callback;
    MT::CMutex m_lock;
    bool m_done;
    bool m_opened;
    bool m_abandoned;
    TAudioDevicePtr m_device;
    CString m_usedDriverName;
};

typedef CORE::CTSharedPtr< CAudioEngineReopenRequest, MT::CMutex > TAudioEngineReopenRequestPtr;

/*-------------------------------------------------------------------------*/

/**
 *  What the capture device calls. The engine is the callback of the playback device; the capture device has this one, so
 *  that the loss of the one is not taken for the loss of the other. Its calls come from the capture thread (the loss from
 *  any thread) and only hand over to the engine
 */
class CAudioEngineCaptureCallback : public IAudioDeviceCallback
{
    public:

    explicit CAudioEngineCaptureCallback( CAudioEngine* engine )
        : IAudioDeviceCallback(), m_engine( engine )
    {GUCEF_TRACE;
    }

    virtual ~CAudioEngineCaptureCallback() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
    }

    virtual void OnRenderAudio( void*, UInt32 ) GUCEF_VIRTUAL_OVERRIDE
    {
        // a capture device renders nothing
    }

    virtual void OnCaptureAudio( const void* buffer, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_engine->OnCaptureAudio( buffer, frameCount );
    }

    virtual void OnDeviceLost( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_engine->OnCaptureDeviceLost();
    }

    private:

    CAudioEngineCaptureCallback( const CAudioEngineCaptureCallback& src );              /**< not implemented */
    CAudioEngineCaptureCallback& operator=( const CAudioEngineCaptureCallback& src );   /**< not implemented */

    CAudioEngine* m_engine;
};

/*-------------------------------------------------------------------------*/

/** One-shot task of the engine's thread pool that opens the device of a reopen request */
class CAudioEngineReopenTask : public CORE::CTaskConsumer
{
    public:

    explicit CAudioEngineReopenTask( const TAudioEngineReopenRequestPtr& request )
        : CORE::CTaskConsumer(), m_request( request )
    {GUCEF_TRACE;
    }

    virtual ~CAudioEngineReopenTask() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
    }

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return "GUCEF::AUDIO::CAudioEngineReopenTask";
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        static const CORE::CString typeName = "GUCEF::AUDIO::CAudioEngineReopenTask";
        return typeName;
    }

    virtual bool OnTaskCycle( CORE::CTaskPtr ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_request->Open();
        return true;            // done
    }

    virtual void OnTaskEnded( CORE::CTaskPtr, bool ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
    }

    private:

    TAudioEngineReopenRequestPtr m_request;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CAudioEngine::RegisterEvents( void )
{GUCEF_TRACE;

    VoiceEndedEvent.Initialize();
    DeviceLostEvent.Initialize();
    CaptureDeviceLostEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAudioEngine::CAudioEngine( void )
    : CORE::CTSGNotifier()
    , m_lock()
    , m_initialized( false )
    , m_params()
    , m_usedDriverName()
    , m_device()
    , m_deviceFormat()
    , m_mixer( GUCEF_NULL )
    , m_commands( GUCEF_NULL )
    , m_events( GUCEF_NULL )
    , m_scratch()
    , m_maxBlock( 0 )
    , m_directFloat( false )
    , m_deviceLost( false )
    , m_droppedEvents( 0 )
    , m_voices()
    , m_maxOutstanding( 0 )
    , m_buses()
    , m_threaded( false )
    , m_updateTask()
    , m_routing( GUCEF_NULL )
    , m_busEntries()
    , m_effects()
    , m_voiceEffects()
    , m_pending()
    , m_lastToken( 0 )
    , m_orderTables()
    , m_freeOrderTables()
    , m_audioOrderTable( GUCEF_NULL )
    , m_echoReference( GUCEF_NULL )
    , m_deviceRunWanted( false )
    , m_reopenPending( false )
    , m_reopenOnOwner( false )
    , m_reopenRequest()
    , m_reopenSource()
    , m_capturePath()
    , m_captureCallback( GUCEF_NULL )
    , m_captureDevice()
    , m_capturePushDevice( GUCEF_NULL )
    , m_captureAvailable( false )
    , m_captureRunning( false )
    , m_captureDeviceLost( false )
    , m_captureEffects()
    , m_captureSlots()
    , m_captureConsumers()
    , m_captureConsumersLeaving()
    , m_captureHalfDuplex( GUCEF_NULL )
    , m_captureHalfDuplexLeaving()
    , m_capturePending()
    , m_echoCanceller( GUCEF_NULL )
    , m_echoCancellersLeaving()
    , m_playbackSchedulingReported( false )
    , m_captureSchedulingReported( false )
{GUCEF_TRACE;

    m_captureCallback = GUCEF_NEW CAudioEngineCaptureCallback( this );
}

/*-------------------------------------------------------------------------*/

CAudioEngine::~CAudioEngine()
{GUCEF_TRACE;

    Shutdown();
    GUCEF_DELETE m_captureCallback;
    m_captureCallback = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngine::Initialize( const CAudioEngineParams& params )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_initialized )
        return false;

    m_params = params;
    m_params.GetDeviceParams().SetDirection( AUDIODIRECTION_PLAYBACK );

    // Everything the audio thread will touch is made before the device can call it
    m_commands = GUCEF_NEW MT::CTSpscQueue< SCommand >( params.GetCommandQueueSize() );
    m_events = GUCEF_NEW MT::CTSpscQueue< SEvent >( params.GetEventQueueSize() );
    m_maxOutstanding = m_events->GetCapacity() / 3;     // a voice makes at most two events that must not be lost: this keeps the queue from overflowing
    m_deviceLost.Store( false );
    m_droppedEvents.Store( 0 );

    if ( !CAudioGlobal::Instance()->GetAudioDriverRegistry().OpenDevice( m_params.GetDriverName(), m_params.GetDeviceParams(), this, m_device, m_usedDriverName ) || m_device.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: no audio device could be opened" );
        GUCEF_DELETE m_commands; m_commands = GUCEF_NULL;
        GUCEF_DELETE m_events; m_events = GUCEF_NULL;
        return false;
    }

    m_deviceFormat = m_device->GetFormat();
    if ( !m_deviceFormat.IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: the audio device reports no usable format" );
        m_device.Unlink();
        GUCEF_DELETE m_commands; m_commands = GUCEF_NULL;
        GUCEF_DELETE m_events; m_events = GUCEF_NULL;
        return false;
    }

    // The mixer works in float in the speakers and rate of the device, the conversion to the device sample type is the last step
    m_maxBlock = ( m_params.GetMaxBlockFrames() > 0 ) ? m_params.GetMaxBlockFrames() : 4096;
    m_mixer = GUCEF_NEW CAudioMixer( m_deviceFormat, m_maxBlock, m_params.GetMaxVoices(), m_params.GetMaxBuses(), m_params.GetMaxEffectsPerBus(), m_params.GetMaxSendsPerBus() );
    m_mixer->SetListener( this );
    m_mixer->SetPanLaw( m_params.GetPanLaw() );
    ApplyLimiterParams( m_params );
    ApplyDeviceFormat( m_deviceFormat );

    // The routing starts with the master bus; every bus and send that is added comes with a new bus order for the mixer
    m_buses.clear();
    m_routing = GUCEF_NEW CAudioBusRouting( m_mixer->GetMaxBuses() );
    CAudioBus* master = m_mixer->GetMasterBus();
    m_routing->AddBus( master, GUCEF_NULL );
    SBusEntry& masterEntry = m_busEntries[ master ];
    masterEntry.effects.assign( m_mixer->GetMaxEffectsPerBus(), GUCEF_NULL );
    masterEntry.sends.assign( m_mixer->GetMaxSendsPerBus(), GUCEF_NULL );
    masterEntry.sendPreFader.assign( m_mixer->GetMaxSendsPerBus(), 0 );
    m_layoutApplies.clear();
    m_audioOrderTable = GUCEF_NULL;
    m_echoReference = GUCEF_NULL;
    m_lastToken = 0;
    m_reopenPending = false;
    m_initialized = true;

    // capture, when asked for: next to the playback device, and the engine plays whether it opens or not
    OpenCaptureDevice();

    // Threaded: a dedicated task keeps the futures and events moving
    m_threaded = false;
    TAudioThreadingMode mode = m_params.GetThreadingMode();
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    if ( AUDIOTHREADING_AUTO == mode )
        mode = AUDIOTHREADING_COOPERATIVE;
    #endif
    if ( AUDIOTHREADING_COOPERATIVE != mode )
    {
        m_updateTask = CORE::CTSharedPtr< CAudioEngineUpdateTask, MT::CMutex >( GUCEF_NEW CAudioEngineUpdateTask( this, m_params.GetUpdateIntervalInMs() ) );
        CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( m_params.GetThreadPoolName() );
        CORE::CFutureResult started = pool.IsNULL() ? CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) : pool->StartTaskWithConsumer( m_updateTask );
        if ( started.HasNoFuture() || started.IsReady() )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: could not start the engine thread, continuing in cooperative mode" );
            m_updateTask.Unlink();
        }
        else
            m_threaded = true;
    }

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: initialized with driver \"" + m_usedDriverName + "\", device format " + m_deviceFormat.ToString() +
                      ( m_threaded ? ", threaded" : ", cooperative" ) );

    m_deviceRunWanted = m_params.GetStartDevice();
    m_playbackSchedulingReported = false;
    if ( m_deviceRunWanted )
    {
        m_device->Start();
        StartCaptureDevice();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::StopUpdateThread( void )
{GUCEF_TRACE;

    if ( !m_updateTask.IsNULL() )
    {
        CORE::CTSharedPtr< CAudioEngineUpdateTask, MT::CMutex > task = m_updateTask;
        m_updateTask.Unlink();
        m_threaded = false;

        // Not under the engine lock: the thread takes it in Update(). Disconnect() waits for an update in progress
        // and guarantees there is none after it, even if the thread has not got round to starting yet
        m_lock.Unlock();
        task->Disconnect();
        task->RequestTaskToStop( false );
        UInt32 waitedInMs = 0;
        while ( !task->HasEnded() && waitedInMs < 3000 )
        {
            MT::PrecisionDelay( 1 );
            ++waitedInMs;
        }
        m_lock.Lock( GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    }
    m_threaded = false;
}

void
CAudioEngine::Shutdown( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized )
        return;

    StopUpdateThread();
    if ( !m_initialized )
        return;                 // somebody else did it while the lock was open
    m_initialized = false;

    // When Stop() returns the audio thread is no longer calling us. The capture device goes first: once it is stopped no
    // capture consumer is called any more
    StopCaptureDevice();
    if ( !m_device.IsNULL() )
    {
        m_device->Stop();
        m_device.Unlink();
    }
    AbandonReopen();

    // The audio thread is gone, so everything it might have used can be released here
    CompleteAllPending();
    for ( TVoiceMap::iterator v=m_voices.begin(); v!=m_voices.end(); ++v )
        (*v).first->ReleaseEffects();
    m_voices.clear();
    m_voiceEffects.clear();
    m_effects.clear();
    ReleaseCapture();
    m_busEntries.clear();
    m_buses.clear();
    m_mixer->SetBusOrder( GUCEF_NULL, 0 );
    m_audioOrderTable = GUCEF_NULL;
    m_echoReference = GUCEF_NULL;
    FreeOrderTables();
    GUCEF_DELETE m_routing;     m_routing = GUCEF_NULL;

    GUCEF_DELETE m_mixer;       m_mixer = GUCEF_NULL;
    GUCEF_DELETE m_commands;    m_commands = GUCEF_NULL;
    GUCEF_DELETE m_events;      m_events = GUCEF_NULL;
    m_scratch.clear();
    m_usedDriverName.Clear();
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: shut down" );
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::CompleteAllPending( void )
{GUCEF_TRACE;

    // Whatever somebody is still waiting for will never happen now
    for ( TVoiceMap::iterator i=m_voices.begin(); i!=m_voices.end(); ++i )
    {
        SVoiceEntry& entry = (*i).second;
        if ( !entry.startSource.IsNULL() )  entry.startSource->Cancel();
        if ( !entry.stopSource.IsNULL() )   entry.stopSource->Cancel();
        if ( !entry.pauseSource.IsNULL() )  entry.pauseSource->Cancel();
        if ( !entry.endedSource.IsNULL() )  entry.endedSource->Cancel();
    }
    for ( TPendingMap::iterator i=m_pending.begin(); i!=m_pending.end(); ++i )
        if ( !(*i).second.IsNULL() )
            (*i).second->Cancel();
    m_pending.clear();
    for ( size_t i=0; i<m_layoutApplies.size(); ++i )
        if ( !m_layoutApplies[ i ].source.IsNULL() )
            m_layoutApplies[ i ].source->Cancel();
    m_layoutApplies.clear();
    for ( TPendingMap::iterator i=m_capturePending.begin(); i!=m_capturePending.end(); ++i )
        if ( !(*i).second.IsNULL() )
            (*i).second->Cancel();
    m_capturePending.clear();
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngine::IsInitialized( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_initialized;
}

/*-------------------------------------------------------------------------*/

CString
CAudioEngine::GetDriverName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_usedDriverName;
}

/*-------------------------------------------------------------------------*/

CAudioEngineParams
CAudioEngine::GetParams( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_params;
}

/*-------------------------------------------------------------------------*/

CAudioFormat
CAudioEngine::GetDeviceFormat( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_deviceFormat;
}

/*-------------------------------------------------------------------------*/

CAudioFormat
CAudioEngine::GetMixFormat( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_initialized ? m_mixer->GetMixFormat() : CAudioFormat();
}

/*-------------------------------------------------------------------------*/

TAudioDevicePtr
CAudioEngine::GetDevice( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_device;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngine::IsThreaded( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_threaded;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngine::StartDevice( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized || !m_device->Start() )
        return false;
    m_deviceRunWanted = true;
    m_playbackSchedulingReported = false;
    StartCaptureDevice();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngine::StopDevice( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized )
        return false;
    m_deviceRunWanted = false;
    StopCaptureDevice();
    return m_device->Stop();
}

/*-------------------------------------------------------------------------*/

CAudioBus*
CAudioEngine::GetMasterBus( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_initialized ? m_mixer->GetMasterBus() : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CAudioBus*
CAudioEngine::CreateBus( const CString& name, CAudioBus* parent )
{GUCEF_TRACE;

    return CreateBusInternal( name, parent, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

CAudioBus*
CAudioEngine::CreateBusInternal( const CString& name, CAudioBus* parent, CORE::CFutureResult* future )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized || m_buses.size() + 1 >= m_params.GetMaxBuses() )
        return GUCEF_NULL;

    CAudioBus* master = m_mixer->GetMasterBus();
    if ( GUCEF_NULL == parent )
        parent = master;
    bool known = ( parent == master );
    for ( size_t i=0; !known && i<m_buses.size(); ++i )
        known = ( m_buses[ i ].GetPointerAlways() == parent );
    if ( !known )
        return GUCEF_NULL;

    if ( !HasRoomForRequest() )
        return GUCEF_NULL;

    // The bus is made here and handed over to the audio thread, which links it into the tree together with the new bus
    // order. Commands arrive in order, so a voice that is started on this bus afterwards finds it there. The engine keeps
    // the bus alive; the command carries a plain pointer
    TAudioBusPtr bus( GUCEF_NEW CAudioBus( name, parent ) );
    CAudioBus* raw = bus.GetPointerAlways();
    if ( !m_mixer->PrepareBus( raw ) || !m_routing->AddBus( raw, parent ) )
        return GUCEF_NULL;
    SOrderTable* order = ComputeBusOrder();
    if ( GUCEF_NULL == order )
    {
        m_routing->RemoveBus( raw );
        return GUCEF_NULL;
    }

    SCommand command;
    command.type = SCommand::COMMAND_ATTACH_BUS;
    command.bus = raw;
    command.order = order;
    command.token = NextToken();
    if ( !m_commands->Push( command ) )
    {
        m_routing->RemoveBus( raw );
        RecycleOrderTable( order );
        return GUCEF_NULL;
    }
    if ( GUCEF_NULL != future )
    {
        // a caller that wants to know when the audio thread has the bus
        TSourcePtr source( GUCEF_NEW CORE::CFutureResultSource() );
        m_pending[ command.token ] = source;
        *future = source->GetFuture();
    }
    else
        m_pending[ command.token ] = TSourcePtr();

    m_buses.push_back( bus );
    SBusEntry& entry = m_busEntries[ raw ];
    entry.effects.assign( m_mixer->GetMaxEffectsPerBus(), GUCEF_NULL );
    entry.sends.assign( m_mixer->GetMaxSendsPerBus(), GUCEF_NULL );
    entry.sendPreFader.assign( m_mixer->GetMaxSendsPerBus(), 0 );
    return raw;
}

/*-------------------------------------------------------------------------*/

CAudioBus*
CAudioEngine::FindBus( const CString& name )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized )
        return GUCEF_NULL;
    if ( name == m_mixer->GetMasterBus()->GetName() )
        return m_mixer->GetMasterBus();
    for ( size_t i=0; i<m_buses.size(); ++i )
        if ( m_buses[ i ]->GetName() == name )
            return m_buses[ i ].GetPointerAlways();
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

TAudioVoicePtr
CAudioEngine::CreateVoice( const TAudioSourcePtr& source, CAudioBus* bus )
{GUCEF_TRACE;

    if ( !source.IsNULL() && !source->IsRealtimeSafe() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "AudioEngine: Creating a voice for a source that is not real time safe, it may glitch the audio when it blocks or allocates while being read on the audio thread. Wrap it in a CAudioStream or decode it into a CAudioClip" );
    }

    MT::CScopeMutex lock( m_lock );
    TAudioVoicePtr voice( GUCEF_NEW CAudioVoice( source, m_params.GetMaxVoiceEffects(), m_params.GetMaxVoiceSends() ) );
    if ( m_initialized )
        voice->SetBus( GUCEF_NULL != bus ? bus : m_mixer->GetMasterBus() );
    return voice;
}

/*-------------------------------------------------------------------------*/

TAudioVoicePtr
CAudioEngine::CreateVoice( const TAudioClipPtr& clip, CAudioBus* bus )
{GUCEF_TRACE;

    if ( clip.IsNULL() )
        return TAudioVoicePtr();
    return CreateVoice( clip->CreateSource(), bus );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::MakeFailedFuture( CORE::TTaskStatus status )
{GUCEF_TRACE;

    return CORE::CFutureResult( status );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::MakeReadyFuture( void )
{GUCEF_TRACE;

    CORE::CFutureResultSource done;
    done.Complete();
    return done.GetFuture();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::Start( const TAudioVoicePtr& voice )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized || voice.IsNULL() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    if ( AUDIOVOICESTATE_IDLE != voice->GetState() || m_voices.find( RawVoice( voice ) ) != m_voices.end() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_TASK_WITH_TYPE_EXISTS );     // already started: a voice plays once
    if ( m_voices.size() >= m_maxOutstanding )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    CAudioVoice* raw = RawVoice( voice );
    if ( GUCEF_NULL == raw->GetBus() )
        raw->SetBus( m_mixer->GetMasterBus() );
    if ( !CheckVoiceRouting( raw ) || !m_mixer->PrepareVoice( raw ) )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    SCommand command;
    command.type = SCommand::COMMAND_START_VOICE;
    command.voice = raw;
    if ( !m_commands->Push( command ) )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );

    // from here on the audio thread may use the voice's effects: they are the voice's until it is reported ended
    for ( UInt32 e=0; e<raw->GetEffectCount(); ++e )
        m_voiceEffects.insert( raw->GetEffectPointer( e ) );

    SVoiceEntry& entry = m_voices[ raw ];
    entry.voice = voice;
    entry.startSource = TSourcePtr( GUCEF_NEW CORE::CFutureResultSource() );
    return entry.startSource->GetFuture();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::Play( const TAudioClipPtr& clip, CAudioBus* bus, TAudioVoicePtr* voiceOut )
{GUCEF_TRACE;

    TAudioVoicePtr voice = CreateVoice( clip, bus );
    if ( GUCEF_NULL != voiceOut )
        *voiceOut = voice;
    if ( voice.IsNULL() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    return Start( voice );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::Stop( const TAudioVoicePtr& voice )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( voice.IsNULL() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    TVoiceMap::iterator i = m_voices.find( RawVoice( voice ) );
    if ( i == m_voices.end() )
        return MakeReadyFuture();               // not playing (never started, or over already)

    SVoiceEntry& entry = (*i).second;
    RawVoice( voice )->RequestStop();
    if ( entry.stopSource.IsNULL() )
        entry.stopSource = TSourcePtr( GUCEF_NEW CORE::CFutureResultSource() );
    return entry.stopSource->GetFuture();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::Pause( const TAudioVoicePtr& voice )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( voice.IsNULL() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    TVoiceMap::iterator i = m_voices.find( RawVoice( voice ) );
    if ( i == m_voices.end() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    SVoiceEntry& entry = (*i).second;
    if ( !entry.pauseSource.IsNULL() )
        entry.pauseSource->Cancel();            // a newer request replaces the one that was waiting
    RawVoice( voice )->RequestPause( true );
    entry.pauseTarget = true;
    entry.pauseSource = TSourcePtr( GUCEF_NEW CORE::CFutureResultSource() );
    return entry.pauseSource->GetFuture();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::Resume( const TAudioVoicePtr& voice )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( voice.IsNULL() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    TVoiceMap::iterator i = m_voices.find( RawVoice( voice ) );
    if ( i == m_voices.end() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    SVoiceEntry& entry = (*i).second;
    if ( !entry.pauseSource.IsNULL() )
        entry.pauseSource->Cancel();
    RawVoice( voice )->RequestPause( false );
    entry.pauseTarget = false;
    entry.pauseSource = TSourcePtr( GUCEF_NEW CORE::CFutureResultSource() );
    return entry.pauseSource->GetFuture();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::GetEndedResult( const TAudioVoicePtr& voice )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( voice.IsNULL() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    TVoiceMap::iterator i = m_voices.find( RawVoice( voice ) );
    if ( i == m_voices.end() )
        return AUDIOVOICESTATE_ENDED == voice->GetState() ? MakeReadyFuture() : MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    SVoiceEntry& entry = (*i).second;
    if ( entry.endedSource.IsNULL() )
        entry.endedSource = TSourcePtr( GUCEF_NEW CORE::CFutureResultSource() );
    return entry.endedSource->GetFuture();
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngine::HasRoomForRequest( void ) const
{GUCEF_TRACE;

    // every request makes one event that must not be lost; voices have their own share of the event queue
    return m_pending.size() < m_maxOutstanding;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEngine::NextToken( void )
{GUCEF_TRACE;

    ++m_lastToken;
    if ( 0 == m_lastToken )
        m_lastToken = 1;            // 0 means "nobody waits"
    return m_lastToken;
}

/*-------------------------------------------------------------------------*/

CAudioEngine::SOrderTable*
CAudioEngine::ComputeBusOrder( void )
{GUCEF_TRACE;

    SOrderTable* order = GUCEF_NULL;
    if ( !m_freeOrderTables.empty() )
    {
        order = m_freeOrderTables.back();
        m_freeOrderTables.pop_back();
    }
    else
    {
        // normally two tables take turns; more only while several routing changes are on their way at once
        order = GUCEF_NEW SOrderTable();
        order->buses.assign( m_mixer->GetMaxBuses(), GUCEF_NULL );
        m_orderTables.push_back( order );
    }

    if ( !m_routing->ComputeOrder( &order->buses[ 0 ], (UInt32) order->buses.size(), order->count ) )
    {
        RecycleOrderTable( order );
        return GUCEF_NULL;
    }
    return order;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::RecycleOrderTable( SOrderTable* order )
{GUCEF_TRACE;

    if ( GUCEF_NULL != order )
        m_freeOrderTables.push_back( order );
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::FreeOrderTables( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_orderTables.size(); ++i )
        GUCEF_DELETE m_orderTables[ i ];
    m_orderTables.clear();
    m_freeOrderTables.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::PushRequest( SCommand& command )
{GUCEF_TRACE;

    command.token = NextToken();
    if ( !m_commands->Push( command ) )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );

    TSourcePtr source( GUCEF_NEW CORE::CFutureResultSource() );
    m_pending[ command.token ] = source;
    return source->GetFuture();
}

/*-------------------------------------------------------------------------*/

TAudioEffectPtr
CAudioEngine::CreateEffect( const CString& typeName, const CORE::CValueList& options )
{GUCEF_TRACE;

    CAudioFormat mixFormat;
    UInt32 maxBlock = 0;
    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_initialized )
            return TAudioEffectPtr();
        mixFormat = m_mixer->GetMixFormat();
        maxBlock = m_maxBlock;
    }

    // made and prepared without the engine lock: preparing allocates and may take a while (a reverb)
    TAudioEffectPtr effect = CAudioGlobal::Instance()->GetAudioEffectRegistry().CreateEffect( typeName, options );
    if ( effect.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: no effect of type \"" << typeName << "\" could be made" );
        return TAudioEffectPtr();
    }
    if ( !effect->Prepare( mixFormat, maxBlock ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: the effect of type \"" << typeName << "\" could not be prepared for " << mixFormat.ToString() );
        return TAudioEffectPtr();
    }
    return effect;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::InsertEffect( CAudioBus* bus, UInt32 slot, const TAudioEffectPtr& effect )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized || effect.IsNULL() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    TBusEntryMap::iterator b = m_busEntries.find( bus );
    if ( b == m_busEntries.end() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    SBusEntry& busEntry = (*b).second;
    if ( slot >= busEntry.effects.size() || GUCEF_NULL != busEntry.effects[ slot ] )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    CAudioEffect* raw = RawEffect( effect );
    if ( IsEffectInUse( raw ) || !raw->IsPreparedFor( m_mixer->GetMixFormat(), m_maxBlock ) )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    if ( !raw->ResolveReferences( *this ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: an effect of type \"" << raw->GetEffectTypeName() << "\" could not resolve what it refers to, it is not inserted" );
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    }
    if ( !HasRoomForRequest() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    SCommand command;
    command.type = SCommand::COMMAND_INSERT_EFFECT;
    command.bus = bus;
    command.index = slot;
    command.effect = raw;
    CORE::CFutureResult future = PushRequest( command );
    if ( future.HasNoFuture() )
        return future;

    busEntry.effects[ slot ] = raw;
    SEffectEntry& entry = m_effects[ raw ];
    entry.effect = effect;
    entry.bus = bus;
    entry.slot = slot;
    entry.removing = false;
    return future;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::RemoveEffect( CAudioBus* bus, const TAudioEffectPtr& effect )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized || effect.IsNULL() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    TEffectMap::iterator e = m_effects.find( RawEffect( effect ) );
    if ( e == m_effects.end() || (*e).second.bus != bus || (*e).second.removing )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    if ( !HasRoomForRequest() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    SEffectEntry& entry = (*e).second;
    SCommand command;
    command.type = SCommand::COMMAND_REMOVE_EFFECT;
    command.bus = bus;
    command.index = entry.slot;
    command.effect = (*e).first;
    CORE::CFutureResult future = PushRequest( command );
    if ( future.HasNoFuture() )
        return future;

    // the slot is free for a new effect at once: commands are carried out in order. The reference stays until the
    // audio thread reports it let go of the effect
    entry.removing = true;
    m_busEntries[ bus ].effects[ entry.slot ] = GUCEF_NULL;
    return future;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::AddSend( CAudioBus* from, CAudioBus* to, Float32 level, bool preFader, UInt32* sendIndex )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    TBusEntryMap::iterator b = m_busEntries.find( from );
    if ( b == m_busEntries.end() || m_busEntries.find( to ) == m_busEntries.end() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    SBusEntry& busEntry = (*b).second;

    UInt32 index = 0;
    while ( index < busEntry.sends.size() && GUCEF_NULL != busEntry.sends[ index ] )
        ++index;
    if ( index >= busEntry.sends.size() || !m_routing->CanAddSend( from, to ) )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    if ( !HasRoomForRequest() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    m_routing->AddSend( from, to );
    SOrderTable* order = ComputeBusOrder();
    if ( GUCEF_NULL == order )
    {
        m_routing->RemoveSend( from, to );
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    }

    // the level is there before the audio thread sees the send
    from->SetSendLevel( index, level );

    SCommand command;
    command.type = SCommand::COMMAND_SET_SEND;
    command.bus = from;
    command.target = to;
    command.index = index;
    command.preFader = preFader;
    command.order = order;
    CORE::CFutureResult future = PushRequest( command );
    if ( future.HasNoFuture() )
    {
        m_routing->RemoveSend( from, to );
        RecycleOrderTable( order );
        return future;
    }

    busEntry.sends[ index ] = to;
    busEntry.sendPreFader[ index ] = preFader ? 1 : 0;
    if ( GUCEF_NULL != sendIndex )
        *sendIndex = index;
    return future;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::RemoveSend( CAudioBus* from, UInt32 sendIndex )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    TBusEntryMap::iterator b = m_busEntries.find( from );
    if ( b == m_busEntries.end() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    SBusEntry& busEntry = (*b).second;
    if ( sendIndex >= busEntry.sends.size() || GUCEF_NULL == busEntry.sends[ sendIndex ] )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    if ( !HasRoomForRequest() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    CAudioBus* to = busEntry.sends[ sendIndex ];
    m_routing->RemoveSend( from, to );
    SOrderTable* order = ComputeBusOrder();
    if ( GUCEF_NULL == order )
    {
        m_routing->AddSend( from, to );
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    }

    SCommand command;
    command.type = SCommand::COMMAND_SET_SEND;
    command.bus = from;
    command.target = GUCEF_NULL;
    command.index = sendIndex;
    command.order = order;
    CORE::CFutureResult future = PushRequest( command );
    if ( future.HasNoFuture() )
    {
        m_routing->AddSend( from, to );
        RecycleOrderTable( order );
        return future;
    }

    busEntry.sends[ sendIndex ] = GUCEF_NULL;
    busEntry.sendPreFader[ sendIndex ] = 0;
    return future;
}

/*-------------------------------------------------------------------------*/

TAudioEffectPtr
CAudioEngine::GetBusEffect( const CAudioBus* bus, UInt32 slot ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TBusEntryMap::const_iterator b = m_busEntries.find( const_cast< CAudioBus* >( bus ) );
    if ( !m_initialized || b == m_busEntries.end() || slot >= (*b).second.effects.size() || GUCEF_NULL == (*b).second.effects[ slot ] )
        return TAudioEffectPtr();

    TEffectMap::const_iterator e = m_effects.find( (*b).second.effects[ slot ] );
    return e == m_effects.end() ? TAudioEffectPtr() : (*e).second.effect;
}

/*-------------------------------------------------------------------------*/

CAudioBus*
CAudioEngine::FindBusUnique( const CString& name, bool& ambiguous ) const
{GUCEF_TRACE;

    ambiguous = false;
    CAudioBus* found = GUCEF_NULL;
    CAudioBus* master = m_mixer->GetMasterBus();
    if ( master->GetName() == name )
        found = master;
    for ( size_t i=0; i<m_buses.size(); ++i )
    {
        if ( m_buses[ i ]->GetName() == name )
        {
            if ( GUCEF_NULL != found )
                ambiguous = true;
            found = const_cast< CAudioBus* >( m_buses[ i ].GetPointerAlways() );
        }
    }
    return found;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngine::GetBusLayout( CAudioBusLayout& layout ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    layout.Clear();
    if ( !m_initialized )
        return false;

    GUCEF::vector< CAudioBus* > buses;
    buses.push_back( m_mixer->GetMasterBus() );
    for ( size_t i=0; i<m_buses.size(); ++i )
        buses.push_back( const_cast< CAudioBus* >( m_buses[ i ].GetPointerAlways() ) );

    // a layout names its buses: buses without a name or with the same name cannot be told apart in it
    for ( size_t i=0; i<buses.size(); ++i )
    {
        bool unique = !buses[ i ]->GetName().IsNULLOrEmpty();
        for ( size_t j=0; unique && j<i; ++j )
            unique = buses[ j ]->GetName() != buses[ i ]->GetName();
        if ( !unique )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: the buses cannot be described as a layout, bus " << i << " has no name or the name of another bus" );
            return false;
        }
    }

    for ( size_t i=0; i<buses.size(); ++i )
    {
        CAudioBus* bus = buses[ i ];
        CAudioBusLayout::SBus& described = layout.AddBus( bus->GetName(), GUCEF_NULL == bus->GetParent() ? CString::Empty : bus->GetParent()->GetName(), bus->GetVolumeInDecibels(), bus->IsMuted() );

        TBusEntryMap::const_iterator b = m_busEntries.find( bus );
        if ( b == m_busEntries.end() )
            continue;
        const SBusEntry& entry = (*b).second;

        for ( size_t s=0; s<entry.sends.size(); ++s )
        {
            if ( GUCEF_NULL == entry.sends[ s ] )
                continue;
            described.AddSend( entry.sends[ s ]->GetName(), CAudioBus::LinearToDecibels( bus->GetSendLevel( (UInt32) s ) ), 0 != entry.sendPreFader[ s ] );
        }

        for ( size_t slot=0; slot<entry.effects.size(); ++slot )
        {
            if ( GUCEF_NULL == entry.effects[ slot ] )
                continue;
            TEffectMap::const_iterator e = m_effects.find( entry.effects[ slot ] );
            if ( e == m_effects.end() )
                continue;
            const TAudioEffectPtr& effect = (*e).second.effect;
            CAudioBusLayout::SEffect& capturedEffect = described.AddEffect( (UInt32) slot, effect->GetEffectTypeName() );
            capturedEffect.enabled = bus->IsEffectEnabled( (UInt32) slot );
            capturedEffect.wet = bus->GetEffectWet( (UInt32) slot );
            capturedEffect.dry = bus->GetEffectDry( (UInt32) slot );
            effect->SaveConfig( capturedEffect.config );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** What ApplyBusLayout() worked out before it changes anything */
struct CAudioEngine::SLayoutPlan
{
    enum EEffectAction { EFFECT_KEEP = 0, EFFECT_INSERT, EFFECT_REPLACE };

    struct SEffectStep
    {
        UInt32 slot;
        EEffectAction action;
        TAudioEffectPtr effect;         /**< keep: the effect that is in the slot, insert and replace: the new one, prepared and configured */
        TAudioEffectPtr oldEffect;      /**< replace: the effect that goes out of the slot */
        TAudioEffectPtr baseline;       /**< keep: an effect as a new one is, what the parameters that the layout does not name go back to */
        const CAudioBusLayout::SEffect* wanted;
        SEffectStep( void ) : slot( 0 ), action( EFFECT_KEEP ), effect(), oldEffect(), wanted( GUCEF_NULL ) {}
    };

    struct SSendStep
    {
        Int32 targetIndex;              /**< index of the target in the layout, -1 for a bus that is not in the layout */
        CAudioBus* targetBus;           /**< a target the engine has already, null for a bus the layout makes */
        UInt32 slot;                    /**< kept sends: the slot they are in */
        const CAudioBusLayout::SSend* wanted;
        SSendStep( void ) : targetIndex( -1 ), targetBus( GUCEF_NULL ), slot( 0 ), wanted( GUCEF_NULL ) {}
    };

    struct SBusStep
    {
        const CAudioBusLayout::SBus* wanted;
        CAudioBus* bus;                 /**< the bus of the engine, null for a bus that is to be made, set when it is */
        bool isNew;
        Int32 parentIndex;              /**< index of the parent in the layout, -1 for a bus the engine has */
        CAudioBus* parentBus;           /**< a parent that the engine has already, null when the layout makes it */
        GUCEF::vector< SEffectStep > effects;
        GUCEF::vector< TAudioEffectPtr > removedEffects;
        GUCEF::vector< UInt32 > removedSends;
        GUCEF::vector< SSendStep > keptSends;
        GUCEF::vector< SSendStep > addedSends;
        SBusStep( void ) : wanted( GUCEF_NULL ), bus( GUCEF_NULL ), isNew( false ), parentIndex( -1 ), parentBus( GUCEF_NULL ), effects(), removedEffects(), removedSends(), keptSends(), addedSends() {}
    };

    GUCEF::vector< SBusStep > buses;
    GUCEF::vector< UInt32 > createOrder;        /**< indices of the new buses, every one after its parent */
    UInt32 commandCount;

    SLayoutPlan( void ) : buses(), createOrder(), commandCount( 0 ) {}
};

/*-------------------------------------------------------------------------*/

namespace {

/** The buses that stand in for the buses a layout makes while its sends are checked for cycles, taken away afterwards */
class CTemporaryBuses
{
    public:

    CTemporaryBuses( void ) : m_buses() {}

    ~CTemporaryBuses()
    {
        for ( size_t i=0; i<m_buses.size(); ++i )
            GUCEF_DELETE m_buses[ i ];
    }

    CAudioBus* Make( const CString& name, CAudioBus* parent )
    {
        CAudioBus* bus = GUCEF_NEW CAudioBus( name, parent );
        m_buses.push_back( bus );
        return bus;
    }

    private:

    CTemporaryBuses( const CTemporaryBuses& src );              /**< not implemented */
    CTemporaryBuses& operator=( const CTemporaryBuses& src );   /**< not implemented */

    GUCEF::vector< CAudioBus* > m_buses;
};

/** The construction options in the configuration of an effect, node "Options" */
void
OptionsOfConfig( const CORE::CDataNode& config, CORE::CValueList& options )
{
    const CORE::CDataNode* node = config.FindChild( "Options" );
    if ( GUCEF_NULL == node )
        return;
    for ( CORE::CDataNode::TAttributeMap::const_iterator a=node->AttributeBegin(); a!=node->AttributeEnd(); ++a )
        options.Set( (*a).first, (*a).second );
}

bool
SameOptions( const CORE::CValueList& a, const CORE::CValueList& b )
{
    if ( a.GetKeyCount() != b.GetKeyCount() )
        return false;
    for ( UInt32 i=0; i<a.GetKeyCount(); ++i )
    {
        const CString key = a.GetKey( i ).AsString();
        CORE::CVariant other;
        if ( !b.TryGetValue( key, other ) || other.AsString() != a.GetValueAlways( key, "" ).AsString() )
            return false;
    }
    return true;
}

}

/*-------------------------------------------------------------------------*/

bool
CAudioEngine::PlanBusLayout( const CAudioBusLayout& layout, SLayoutPlan& plan, CORE::TTaskStatus& refusal )
{GUCEF_TRACE;

    refusal = CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED;

    const CAudioBusLayout::TBusVector& wantedBuses = layout.GetBuses();
    const size_t count = wantedBuses.size();
    CAudioBus* master = m_mixer->GetMasterBus();
    const CString masterName = master->GetName();
    const UInt32 maxEffects = m_mixer->GetMaxEffectsPerBus();
    const UInt32 maxSends = m_mixer->GetMaxSendsPerBus();

    // 1. the buses: names, which ones exist
    typedef GUCEF::map< CString, size_t > TNameMap;
    TNameMap indexByName;
    plan.buses.resize( count );
    for ( size_t i=0; i<count; ++i )
    {
        const CAudioBusLayout::SBus& wanted = wantedBuses[ i ];
        SLayoutPlan::SBusStep& step = plan.buses[ i ];
        step.wanted = &wanted;
        if ( wanted.name.IsNULLOrEmpty() || indexByName.find( wanted.name ) != indexByName.end() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: bus layout refused, the bus at position " << i << " has no name or a name that is used already" );
            return false;
        }
        indexByName[ wanted.name ] = i;

        bool ambiguous = false;
        step.bus = FindBusUnique( wanted.name, ambiguous );
        if ( ambiguous )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: bus layout refused, more than one bus of the engine is named \"" << wanted.name << "\"" );
            return false;
        }
        step.isNew = GUCEF_NULL == step.bus;
    }

    // 2. the parents: they have to exist or be made, and a bus that exists stays where it is
    UInt32 newCount = 0;
    for ( size_t i=0; i<count; ++i )
    {
        SLayoutPlan::SBusStep& step = plan.buses[ i ];
        const CAudioBusLayout::SBus& wanted = *step.wanted;
        if ( step.isNew )
            ++newCount;

        if ( step.bus == master )
        {
            if ( !wanted.parent.IsNULLOrEmpty() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: bus layout refused, the master bus has no parent but the layout gives it \"" << wanted.parent << "\"" );
                return false;
            }
            continue;
        }

        const CString parentName = wanted.parent.IsNULLOrEmpty() ? masterName : wanted.parent;
        if ( parentName == wanted.name )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: bus layout refused, bus \"" << wanted.name << "\" is its own parent" );
            return false;
        }

        TNameMap::const_iterator p = indexByName.find( parentName );
        if ( p != indexByName.end() )
        {
            step.parentIndex = (Int32) (*p).second;
            step.parentBus = plan.buses[ (*p).second ].bus;
        }
        else
        {
            bool ambiguous = false;
            step.parentBus = FindBusUnique( parentName, ambiguous );
            if ( ambiguous || GUCEF_NULL == step.parentBus )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: bus layout refused, the parent \"" << parentName << "\" of bus \"" << wanted.name << "\" is unknown or not unique" );
                return false;
            }
        }

        if ( !step.isNew && step.parentBus != step.bus->GetParent() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: bus layout refused, bus \"" << wanted.name << "\" exists with another parent and a bus cannot be moved" );
            return false;
        }
    }

    // 3. the order in which the new buses are made: parents first
    {
        GUCEF::vector< bool > placed( count, false );
        UInt32 remaining = 0;
        for ( size_t i=0; i<count; ++i )
        {
            placed[ i ] = !plan.buses[ i ].isNew;
            if ( plan.buses[ i ].isNew )
                ++remaining;
        }
        while ( remaining > 0 )
        {
            bool progress = false;
            for ( size_t i=0; i<count; ++i )
            {
                const SLayoutPlan::SBusStep& step = plan.buses[ i ];
                if ( placed[ i ] || ( step.parentIndex >= 0 && !placed[ (size_t) step.parentIndex ] ) )
                    continue;
                plan.createOrder.push_back( (UInt32) i );
                placed[ i ] = true;
                --remaining;
                progress = true;
            }
            if ( !progress )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: bus layout refused, the parents of the new buses form a cycle" );
                return false;
            }
        }
    }
    if ( newCount > 0 && m_buses.size() + newCount >= m_params.GetMaxBuses() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: bus layout refused, " << newCount << " new buses do not fit in the " << m_params.GetMaxBuses() << " buses of the engine" );
        return false;
    }
    plan.commandCount += newCount;

    // 4. stand-ins for the new buses, so the sends can be checked on a copy of the routing
    CTemporaryBuses temporaryBuses;
    GUCEF::vector< CAudioBus* > scratchBus( count, GUCEF_NULL );
    CAudioBusRouting scratch( m_mixer->GetMaxBuses() );
    scratch.AddBus( master, GUCEF_NULL );
    for ( size_t i=0; i<m_buses.size(); ++i )
    {
        CAudioBus* existing = const_cast< CAudioBus* >( m_buses[ i ].GetPointerAlways() );
        scratch.AddBus( existing, existing->GetParent() );
    }
    for ( size_t i=0; i<count; ++i )
        scratchBus[ i ] = plan.buses[ i ].bus;
    for ( size_t n=0; n<plan.createOrder.size(); ++n )
    {
        const size_t i = plan.createOrder[ n ];
        const SLayoutPlan::SBusStep& step = plan.buses[ i ];
        CAudioBus* parent = step.parentIndex >= 0 ? scratchBus[ (size_t) step.parentIndex ] : step.parentBus;
        scratchBus[ i ] = temporaryBuses.Make( step.wanted->name, parent );
        if ( !scratch.AddBus( scratchBus[ i ], parent ) )
            return false;
    }

    // what the buses that the layout does not name send stays as it is
    {
        GUCEF::vector< CAudioBus* > existing;
        existing.push_back( master );
        for ( size_t i=0; i<m_buses.size(); ++i )
            existing.push_back( const_cast< CAudioBus* >( m_buses[ i ].GetPointerAlways() ) );
        for ( size_t i=0; i<existing.size(); ++i )
        {
            if ( indexByName.find( existing[ i ]->GetName() ) != indexByName.end() )
                continue;
            TBusEntryMap::const_iterator b = m_busEntries.find( existing[ i ] );
            if ( b == m_busEntries.end() )
                continue;
            for ( size_t s=0; s<(*b).second.sends.size(); ++s )
                if ( GUCEF_NULL != (*b).second.sends[ s ] )
                    scratch.AddSend( existing[ i ], (*b).second.sends[ s ] );
        }
    }

    // 5. effects and sends of every named bus
    CAudioEffectRegistry& registry = CAudioGlobal::Instance()->GetAudioEffectRegistry();
    const CAudioFormat mixFormat = m_mixer->GetMixFormat();
    for ( size_t i=0; i<count; ++i )
    {
        SLayoutPlan::SBusStep& step = plan.buses[ i ];
        const CAudioBusLayout::SBus& wanted = *step.wanted;
        const SBusEntry* entry = GUCEF_NULL;
        if ( !step.isNew )
        {
            TBusEntryMap::const_iterator b = m_busEntries.find( step.bus );
            if ( b != m_busEntries.end() )
                entry = &(*b).second;
        }

        // effects
        GUCEF::vector< bool > slotWanted( maxEffects, false );
        for ( size_t e=0; e<wanted.effects.size(); ++e )
        {
            const CAudioBusLayout::SEffect& wantedEffect = wanted.effects[ e ];
            if ( wantedEffect.slot >= maxEffects || slotWanted[ wantedEffect.slot ] || wantedEffect.typeName.IsNULLOrEmpty() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: bus layout refused, an effect of bus \"" << wanted.name << "\" has no type, a slot beyond the " << maxEffects << " the bus has, or a slot that is used twice" );
                return false;
            }
            slotWanted[ wantedEffect.slot ] = true;

            const CString configType = wantedEffect.config.GetAttributeValueOrChildValueByName( "type" ).AsString();
            if ( !configType.IsNULLOrEmpty() && configType.Lowercase() != wantedEffect.typeName.Lowercase() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: bus layout refused, the effect in slot " << wantedEffect.slot << " of bus \"" << wanted.name << "\" is of type \"" << wantedEffect.typeName << "\" but has the configuration of type \"" << configType << "\"" );
                return false;
            }

            SLayoutPlan::SEffectStep effectStep;
            effectStep.slot = wantedEffect.slot;
            effectStep.wanted = &wantedEffect;

            TAudioEffectPtr live;
            if ( GUCEF_NULL != entry && GUCEF_NULL != entry->effects[ wantedEffect.slot ] )
            {
                TEffectMap::const_iterator le = m_effects.find( entry->effects[ wantedEffect.slot ] );
                if ( le != m_effects.end() )
                    live = (*le).second.effect;
            }

            CORE::CValueList wantedOptions;
            OptionsOfConfig( wantedEffect.config, wantedOptions );

            TAudioEffectPtr candidate;
            bool sameKind = !live.IsNULL() && live->GetEffectTypeName().Lowercase() == wantedEffect.typeName.Lowercase();
            if ( sameKind )
            {
                // made without being prepared: it tells which options the layout's make, and what a new effect is like
                candidate = registry.CreateEffect( wantedEffect.typeName, wantedOptions );
                if ( candidate.IsNULL() )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: bus layout refused, no effect of type \"" << wantedEffect.typeName << "\" can be made with the options of the layout" );
                    return false;
                }
                CORE::CValueList liveOptions, candidateOptions;
                live->GetConstructionOptions( liveOptions );
                candidate->GetConstructionOptions( candidateOptions );
                sameKind = SameOptions( candidateOptions, liveOptions );
            }

            if ( sameKind )
            {
                effectStep.action = SLayoutPlan::EFFECT_KEEP;
                effectStep.effect = live;
                effectStep.baseline = candidate;
            }
            else
            {
                if ( candidate.IsNULL() )
                    candidate = registry.CreateEffect( wantedEffect.typeName, wantedOptions );
                if ( candidate.IsNULL() )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: bus layout refused, no effect of type \"" << wantedEffect.typeName << "\" can be made" );
                    return false;
                }
                if ( !candidate->Prepare( mixFormat, m_maxBlock ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: bus layout refused, the effect of type \"" << wantedEffect.typeName << "\" could not be prepared for " << mixFormat.ToString() );
                    return false;
                }
                if ( !candidate->ApplyConfig( wantedEffect.config, GUCEF_NULL ) )
                    return false;
                effectStep.action = live.IsNULL() ? SLayoutPlan::EFFECT_INSERT : SLayoutPlan::EFFECT_REPLACE;
                effectStep.effect = candidate;
                effectStep.oldEffect = live;
                plan.commandCount += live.IsNULL() ? 1 : 2;
            }
            step.effects.push_back( effectStep );
        }

        if ( GUCEF_NULL != entry )
        {
            for ( size_t slot=0; slot<entry->effects.size(); ++slot )
            {
                if ( GUCEF_NULL == entry->effects[ slot ] || ( slot < maxEffects && slotWanted[ slot ] ) )
                    continue;
                TEffectMap::const_iterator le = m_effects.find( entry->effects[ slot ] );
                if ( le != m_effects.end() )
                {
                    step.removedEffects.push_back( (*le).second.effect );
                    ++plan.commandCount;
                }
            }
        }

        // sends
        if ( wanted.sends.size() > maxSends )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: bus layout refused, bus \"" << wanted.name << "\" has " << wanted.sends.size() << " sends and the engine allows " << maxSends << " per bus" );
            return false;
        }
        GUCEF::vector< bool > slotKept( GUCEF_NULL == entry ? 0 : entry->sends.size(), false );
        for ( size_t s=0; s<wanted.sends.size(); ++s )
        {
            const CAudioBusLayout::SSend& wantedSend = wanted.sends[ s ];
            SLayoutPlan::SSendStep sendStep;
            sendStep.wanted = &wantedSend;

            TNameMap::const_iterator t = indexByName.find( wantedSend.target );
            if ( t != indexByName.end() )
            {
                sendStep.targetIndex = (Int32) (*t).second;
                sendStep.targetBus = plan.buses[ (*t).second ].bus;
            }
            else
            {
                bool ambiguous = false;
                sendStep.targetBus = FindBusUnique( wantedSend.target, ambiguous );
                if ( ambiguous || GUCEF_NULL == sendStep.targetBus )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: bus layout refused, the target \"" << wantedSend.target << "\" of a send of bus \"" << wanted.name << "\" is unknown or not unique" );
                    return false;
                }
            }

            CAudioBus* scratchTarget = sendStep.targetIndex >= 0 ? scratchBus[ (size_t) sendStep.targetIndex ] : sendStep.targetBus;
            if ( !scratch.AddSend( scratchBus[ i ], scratchTarget ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: bus layout refused, the send from \"" << wanted.name << "\" to \"" << wantedSend.target << "\" would feed a bus into itself" );
                return false;
            }

            // a send of the engine to the same target on the same side of the fader is kept
            bool kept = false;
            for ( size_t slot=0; !kept && GUCEF_NULL != entry && slot<entry->sends.size(); ++slot )
            {
                if ( !slotKept[ slot ] && GUCEF_NULL != sendStep.targetBus && entry->sends[ slot ] == sendStep.targetBus && ( 0 != entry->sendPreFader[ slot ] ) == wantedSend.preFader )
                {
                    slotKept[ slot ] = true;
                    sendStep.slot = (UInt32) slot;
                    kept = true;
                }
            }
            if ( kept )
                step.keptSends.push_back( sendStep );
            else
            {
                step.addedSends.push_back( sendStep );
                ++plan.commandCount;
            }
        }
        for ( size_t slot=0; slot<slotKept.size(); ++slot )
        {
            if ( !slotKept[ slot ] && GUCEF_NULL != entry->sends[ slot ] )
            {
                step.removedSends.push_back( (UInt32) slot );
                ++plan.commandCount;
            }
        }
    }

    // 6. room for the commands
    if ( m_pending.size() + plan.commandCount > m_maxOutstanding || plan.commandCount > m_commands->GetCapacity() - m_commands->GetSize() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: bus layout needs " << plan.commandCount << " commands and the engine has no room for that many right now, try again later" );
        refusal = CORE::TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngine::ExecuteBusLayout( const CAudioBusLayout& layout, SLayoutPlan& plan, CORE::CFutureResult::TFutureResultVector& parts )
{GUCEF_TRACE;

    // 1. the new buses, parents first
    for ( size_t n=0; n<plan.createOrder.size(); ++n )
    {
        SLayoutPlan::SBusStep& step = plan.buses[ plan.createOrder[ n ] ];
        CAudioBus* parent = step.parentIndex >= 0 ? plan.buses[ (size_t) step.parentIndex ].bus : step.parentBus;
        CORE::CFutureResult made = MakeReadyFuture();
        step.bus = CreateBusInternal( step.wanted->name, parent, &made );
        if ( GUCEF_NULL == step.bus )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: bus layout: bus \"" << step.wanted->name << "\" could not be made" );
            return false;
        }
        parts.push_back( made );
    }

    // 2. what is changed without the audio thread
    for ( size_t i=0; i<plan.buses.size(); ++i )
    {
        plan.buses[ i ].bus->SetVolumeInDecibels( plan.buses[ i ].wanted->volumeInDecibels );
        plan.buses[ i ].bus->SetMuted( plan.buses[ i ].wanted->muted );
    }

    // 3. effects: out first, so the slots are free for the ones that come in
    for ( size_t i=0; i<plan.buses.size(); ++i )
    {
        SLayoutPlan::SBusStep& step = plan.buses[ i ];
        for ( size_t r=0; r<step.removedEffects.size(); ++r )
        {
            CORE::CFutureResult done = RemoveEffect( step.bus, step.removedEffects[ r ] );
            if ( done.HasNoFuture() )
                return false;
            parts.push_back( done );
        }
        for ( size_t e=0; e<step.effects.size(); ++e )
        {
            if ( SLayoutPlan::EFFECT_REPLACE == step.effects[ e ].action )
            {
                CORE::CFutureResult done = RemoveEffect( step.bus, step.effects[ e ].oldEffect );
                if ( done.HasNoFuture() )
                    return false;
                parts.push_back( done );
            }
        }
        for ( size_t e=0; e<step.effects.size(); ++e )
        {
            SLayoutPlan::SEffectStep& effectStep = step.effects[ e ];
            if ( SLayoutPlan::EFFECT_KEEP == effectStep.action )
            {
                if ( !effectStep.effect->ApplyConfig( effectStep.wanted->config, effectStep.baseline.GetPointerAlways() ) )
                    return false;
            }
            else
            {
                CORE::CFutureResult done = InsertEffect( step.bus, effectStep.slot, effectStep.effect );
                if ( done.HasNoFuture() )
                    return false;
                parts.push_back( done );
            }
            step.bus->SetEffectEnabled( effectStep.slot, effectStep.wanted->enabled );
            step.bus->SetEffectMix( effectStep.slot, effectStep.wanted->wet, effectStep.wanted->dry );
        }
    }

    // 4. sends: all that go before any that come, so the routing is never more than the old and the new together
    for ( size_t i=0; i<plan.buses.size(); ++i )
    {
        SLayoutPlan::SBusStep& step = plan.buses[ i ];
        for ( size_t r=0; r<step.removedSends.size(); ++r )
        {
            CORE::CFutureResult done = RemoveSend( step.bus, step.removedSends[ r ] );
            if ( done.HasNoFuture() )
                return false;
            parts.push_back( done );
        }
    }
    for ( size_t i=0; i<plan.buses.size(); ++i )
    {
        SLayoutPlan::SBusStep& step = plan.buses[ i ];
        for ( size_t k=0; k<step.keptSends.size(); ++k )
            step.bus->SetSendLevel( step.keptSends[ k ].slot, CAudioBus::DecibelsToLinear( step.keptSends[ k ].wanted->levelInDecibels ) );
        for ( size_t a=0; a<step.addedSends.size(); ++a )
        {
            SLayoutPlan::SSendStep& sendStep = step.addedSends[ a ];
            CAudioBus* target = sendStep.targetIndex >= 0 ? plan.buses[ (size_t) sendStep.targetIndex ].bus : sendStep.targetBus;
            CORE::CFutureResult done = AddSend( step.bus, target, CAudioBus::DecibelsToLinear( sendStep.wanted->levelInDecibels ), sendStep.wanted->preFader );
            if ( done.HasNoFuture() )
                return false;
            parts.push_back( done );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::ApplyBusLayout( const CAudioBusLayout& layout )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    SLayoutPlan plan;
    CORE::TTaskStatus refusal = CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED;
    if ( !PlanBusLayout( layout, plan, refusal ) )
        return MakeFailedFuture( refusal );

    CORE::CFutureResult::TFutureResultVector parts;
    if ( !ExecuteBusLayout( layout, plan, parts ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: a step of a bus layout that was checked beforehand failed, the layout is applied in part" );
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED );
    }
    if ( parts.empty() )
        return MakeReadyFuture();

    SLayoutApply apply;
    apply.source = TSourcePtr( GUCEF_NEW CORE::CFutureResultSource() );
    apply.parts = parts;
    m_layoutApplies.push_back( apply );
    return apply.source->GetFuture();
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::CompleteLayoutApplies( void )
{GUCEF_TRACE;

    size_t i = 0;
    while ( i < m_layoutApplies.size() )
    {
        SLayoutApply& apply = m_layoutApplies[ i ];
        bool allReady = true;
        bool anyFailed = false;
        for ( size_t p=0; p<apply.parts.size(); ++p )
        {
            if ( !apply.parts[ p ].IsReady() )
            {
                allReady = false;
                break;
            }
            if ( apply.parts[ p ].HasNoFuture() )
                anyFailed = true;
        }
        if ( !allReady )
        {
            ++i;
            continue;
        }

        if ( anyFailed )
            apply.source->Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "the audio thread could not carry out a command of the bus layout" );
        else
            apply.source->Complete();
        m_layoutApplies.erase( m_layoutApplies.begin() + i );
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::ApplyDeviceFormat( const CAudioFormat& deviceFormat )
{GUCEF_TRACE;

    m_deviceFormat = deviceFormat;
    m_directFloat = SAMPLETYPE_FLOAT32 == m_deviceFormat.GetSampleType();
    if ( !m_directFloat )
        m_scratch.assign( (size_t) m_maxBlock * m_deviceFormat.GetChannelCount(), 0.0f );
    else
        m_scratch.clear();
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::PrepareForMixFormat( const CAudioFormat& mixFormat )
{GUCEF_TRACE;

    // Nothing renders while this runs (both devices are stopped), so memory can be replaced freely
    m_mixer->SetMixFormat( mixFormat );
    const CAudioFormat& newMix = m_mixer->GetMixFormat();

    // the render tap of the echo canceller: the one in use and the ones the render thread may still feed
    if ( GUCEF_NULL != m_echoCanceller && !m_echoCanceller->PrepareReference( newMix.GetSampleRate(), newMix.GetChannelCount(), m_maxBlock ) )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: the echo canceller cannot take its reference in " << newMix.ToString() );
    for ( size_t i=0; i<m_echoCancellersLeaving.size(); ++i )
        m_echoCancellersLeaving[ i ]->PrepareReference( newMix.GetSampleRate(), newMix.GetChannelCount(), m_maxBlock );

    // buses whose attach command is still on its way are prepared as well: they will be mixed in this format
    for ( size_t b=0; b<m_buses.size(); ++b )
        m_mixer->PrepareBus( m_buses[ b ].GetPointerAlways() );

    // voices that play, and voices whose start command is still on its way, keep their position
    for ( TVoiceMap::iterator v=m_voices.begin(); v!=m_voices.end(); ++v )
    {
        if ( !m_mixer->ReprepareVoice( (*v).first ) )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: a voice cannot be mixed in " << newMix.ToString() << ", it will end" );
    }

    // effects in slots, and effects on their way into one
    for ( TEffectMap::iterator e=m_effects.begin(); e!=m_effects.end(); ++e )
    {
        CAudioEffect* effect = (*e).first;
        if ( !effect->Prepare( newMix, m_maxBlock ) )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: an effect of type \"" << effect->GetEffectTypeName() << "\" could not be prepared for " << newMix.ToString() << ", it passes the audio unchanged" );
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::ApplyLimiterParams( const CAudioEngineParams& params )
{GUCEF_TRACE;

    // the mode the limiter has when it is on comes first, then whether it is on: a limiter that is off keeps that mode
    m_mixer->SetLimiterCeilingInDecibels( params.GetLimiterCeilingInDecibels() );
    m_mixer->SetLimiterLookaheadInMs( params.GetLimiterLookaheadInMs() );
    m_mixer->SetLimiterReleaseInMs( params.GetLimiterReleaseInMs() );
    m_mixer->SetLimiterMode( LimiterModeWhenEnabled( params ) );
    m_mixer->SetLimiterEnabled( params.GetLimiterEnabled() );
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEngine::GetMixLatencyInFrames( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_initialized ? m_mixer->GetLatencyInFrames() : 0;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngine::SwapDevice( TAudioDevicePtr& newDevice, const CString& usedDriverName, const CAudioEngineParams& params )
{GUCEF_TRACE;

    const CAudioFormat newDeviceFormat = newDevice->GetFormat();
    if ( !newDeviceFormat.IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: the new audio device reports no usable format, the current device stays in use" );
        newDevice.Unlink();
        return false;
    }

    CAudioFormat newMix( newDeviceFormat );
    newMix.SetSampleType( SAMPLETYPE_FLOAT32 );
    const CAudioFormat oldMix = m_mixer->GetMixFormat();
    const CAudioFormat oldDeviceFormat = m_deviceFormat;
    const bool mixChanges = !( newMix == oldMix );

    // From here on no render callback runs: the old device is stopped and the new one is not started yet
    if ( !m_device.IsNULL() )
        m_device->Stop();

    if ( mixChanges )
        PrepareForMixFormat( newMix );
    ApplyDeviceFormat( newDeviceFormat );
    ApplyLimiterParams( params );

    if ( m_deviceRunWanted && !newDevice->Start() )
    {
        // back to the old device, which is still open
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: the new audio device could not be started, the previous device is used again" );
        newDevice.Unlink();
        if ( mixChanges )
            PrepareForMixFormat( oldMix );
        ApplyDeviceFormat( oldDeviceFormat );
        ApplyLimiterParams( m_params );
        if ( !m_device.IsNULL() )
            m_device->Start();
        return false;
    }

    // what the reopen does not change is reported, so a caller does not expect it to
    if ( params.GetMaxVoices() != m_params.GetMaxVoices() || params.GetMaxBuses() != m_params.GetMaxBuses() ||
         params.GetMaxBlockFrames() != m_params.GetMaxBlockFrames() || params.GetThreadingMode() != m_params.GetThreadingMode() ||
         params.GetThreadPoolName() != m_params.GetThreadPoolName() || params.GetMaxEffectsPerBus() != m_params.GetMaxEffectsPerBus() ||
         params.GetMaxSendsPerBus() != m_params.GetMaxSendsPerBus() || params.GetCommandQueueSize() != m_params.GetCommandQueueSize() ||
         params.GetEventQueueSize() != m_params.GetEventQueueSize() || params.GetMaxVoiceEffects() != m_params.GetMaxVoiceEffects() ||
         params.GetMaxVoiceSends() != m_params.GetMaxVoiceSends() || params.GetCaptureEnabled() != m_params.GetCaptureEnabled() ||
         params.GetCaptureProcessingRate() != m_params.GetCaptureProcessingRate() || params.GetCaptureChannel() != m_params.GetCaptureChannel() ||
         params.GetMaxCaptureConsumers() != m_params.GetMaxCaptureConsumers() || params.GetMaxCaptureEffects() != m_params.GetMaxCaptureEffects() ||
         params.GetCaptureCommandQueueSize() != m_params.GetCaptureCommandQueueSize() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: a device reopen keeps the capacities, block size, queues, threading and capture the engine was initialized with; the different values asked for are ignored" );
    }

    TAudioDevicePtr oldDevice = m_device;
    m_device = newDevice;
    newDevice.Unlink();
    m_usedDriverName = usedDriverName;
    m_params.SetDriverName( params.GetDriverName() );
    m_params.GetDeviceParams() = params.GetDeviceParams();
    m_params.GetDeviceParams().SetDirection( AUDIODIRECTION_PLAYBACK );
    m_params.SetLimiterMode( LimiterModeWhenEnabled( params ) );
    m_params.SetLimiterEnabled( params.GetLimiterEnabled() );
    m_params.SetLimiterCeilingInDecibels( params.GetLimiterCeilingInDecibels() );
    m_params.SetLimiterLookaheadInMs( params.GetLimiterLookaheadInMs() );
    m_params.SetLimiterReleaseInMs( params.GetLimiterReleaseInMs() );

    // the old device is closed last, when nothing refers to it any more
    oldDevice.Unlink();
    m_playbackSchedulingReported = false;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: device reopened with driver \"" << m_usedDriverName << "\", device format " << m_deviceFormat.ToString() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngine::ReopenDevice( const CAudioEngineParams& params )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_initialized || m_reopenPending )
            return false;
        m_reopenPending = true;
    }

    // opened on this thread without the engine lock: opening a device can take a while, the audio goes on meanwhile
    CAudioEngineParams wanted( params );
    wanted.GetDeviceParams().SetDirection( AUDIODIRECTION_PLAYBACK );
    TAudioDevicePtr device;
    CString usedDriverName;
    const bool opened = CAudioGlobal::Instance()->GetAudioDriverRegistry().OpenDevice( wanted.GetDriverName(), wanted.GetDeviceParams(), this, device, usedDriverName ) && !device.IsNULL();

    MT::CScopeMutex lock( m_lock );
    m_reopenPending = false;
    if ( !opened )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: the new audio device could not be opened, the current device stays in use" );
        return false;
    }
    if ( !m_initialized )
        return false;
    return SwapDevice( device, usedDriverName, wanted );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::ReopenDeviceAsync( const CAudioEngineParams& params )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized || m_reopenPending )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    CAudioEngineParams wanted( params );
    wanted.GetDeviceParams().SetDirection( AUDIODIRECTION_PLAYBACK );
    TAudioEngineReopenRequestPtr request( GUCEF_NEW CAudioEngineReopenRequest( wanted, this ) );

    // the device is opened by a task of the engine's pool. Where none can be started the owner side opens it in Update()
    bool started = false;
    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( m_params.GetThreadPoolName() );
    if ( !pool.IsNULL() )
    {
        CORE::CTSharedPtr< CAudioEngineReopenTask, MT::CMutex > task( GUCEF_NEW CAudioEngineReopenTask( request ) );
        CORE::CFutureResult taskFuture = pool->StartTaskWithConsumer( task );
        started = !taskFuture.HasNoFuture();
    }
    #endif
    if ( !started )
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: no task could be started to open the new device, it is opened on the owner side" );

    m_reopenPending = true;
    m_reopenRequest = request;
    m_reopenSource = TSourcePtr( GUCEF_NEW CORE::CFutureResultSource() );
    m_reopenOnOwner = !started;
    return m_reopenSource->GetFuture();
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::FinishReopenWhenOpened( void )
{GUCEF_TRACE;

    if ( m_reopenRequest.IsNULL() )
        return;
    if ( m_reopenOnOwner )
    {
        m_reopenOnOwner = false;
        m_reopenRequest->Open();
    }
    if ( !m_reopenRequest->IsDone() )
        return;

    TAudioEngineReopenRequestPtr request = m_reopenRequest;
    TSourcePtr source = m_reopenSource;
    m_reopenRequest.Unlink();
    m_reopenSource.Unlink();
    m_reopenPending = false;

    TAudioDevicePtr device;
    CString usedDriverName;
    if ( !request->TakeDevice( device, usedDriverName ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: the new audio device could not be opened, the current device stays in use" );
        source->Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "the new audio device could not be opened" );
        return;
    }
    if ( SwapDevice( device, usedDriverName, request->GetParams() ) )
        source->Complete();
    else
        source->Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "the new audio device could not be used" );
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::AbandonReopen( void )
{GUCEF_TRACE;

    if ( !m_reopenRequest.IsNULL() )
        m_reopenRequest->Abandon();
    if ( !m_reopenSource.IsNULL() )
        m_reopenSource->Cancel();
    m_reopenRequest.Unlink();
    m_reopenSource.Unlink();
    m_reopenPending = false;
    m_reopenOnOwner = false;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::SetMasterVolume( Float32 volume )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_initialized )
        m_mixer->GetMasterBus()->SetVolume( volume );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioEngine::GetMasterVolume( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_initialized ? m_mixer->GetMasterBus()->GetVolume() : 0.0f;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEngine::GetActiveVoiceCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_voices.size();
}

/*-------------------------------------------------------------------------*/

UInt64
CAudioEngine::GetDroppedEventCount( void ) const
{GUCEF_TRACE;

    return m_droppedEvents.Load();
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngine::IsEffectInUse( CAudioEffect* effect ) const
{GUCEF_TRACE;

    // an effect has one owner at a time: a bus slot, a capture slot or a voice
    return m_effects.find( effect ) != m_effects.end() ||
           m_voiceEffects.find( effect ) != m_voiceEffects.end() ||
           m_captureEffects.find( effect ) != m_captureEffects.end();
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::OpenCaptureDevice( void )
{GUCEF_TRACE;

    m_captureAvailable = false;
    m_captureRunning = false;
    m_captureDeviceLost.Store( false );
    m_capturePushDevice = GUCEF_NULL;
    m_captureSchedulingReported = false;
    if ( !m_params.GetCaptureEnabled() )
        return;

    const UInt32 processingRate = m_params.GetCaptureProcessingRate();
    if ( 0 != processingRate && ( processingRate < 8000 || processingRate > 48000 ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: the capture processing rate " << processingRate << " is not 0 or 8000 to 48000, the engine plays without capture" );
        return;
    }

    // the capture device comes from the driver of the playback device, always in the capture direction
    m_params.GetCaptureDeviceParams().SetDirection( AUDIODIRECTION_CAPTURE );
    CString usedDriverName;
    if ( !CAudioGlobal::Instance()->GetAudioDriverRegistry().OpenDevice( m_usedDriverName, m_params.GetCaptureDeviceParams(), m_captureCallback, m_captureDevice, usedDriverName ) || m_captureDevice.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: no capture device \"" << m_params.GetCaptureDeviceParams().GetDeviceId() << "\" could be opened with driver \"" << m_usedDriverName << "\", the engine plays without capture" );
        m_captureDevice.Unlink();
        return;
    }

    const CAudioFormat captureFormat = m_captureDevice->GetFormat();
    m_capturePath.SetVoiceActivitySettings( m_params.GetVoiceActivitySettings() );
    if ( AUDIODIRECTION_CAPTURE != m_captureDevice->GetDirection() ||
         !m_capturePath.Prepare( captureFormat, processingRate, m_params.GetCaptureChannel(), m_maxBlock, m_params.GetMaxCaptureConsumers(), m_params.GetMaxCaptureEffects(), m_params.GetCaptureCommandQueueSize() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: the capture device with format " << captureFormat.ToString() << " cannot be used with capture channel " <<
                         m_params.GetCaptureChannel() << " and processing rate " << processingRate << ", the engine plays without capture" );
        m_captureDevice.Unlink();
        m_capturePath.Release();
        return;
    }

    m_capturePushDevice = m_captureDevice->AsPushDevice();
    m_captureSlots.assign( (size_t) AUDIOCAPTURESTAGE_COUNT * m_capturePath.GetMaxEffectsPerStage(), GUCEF_NULL );
    m_captureConsumers.clear();
    m_captureConsumersLeaving.clear();
    m_captureHalfDuplex = GUCEF_NULL;
    m_captureHalfDuplexLeaving.clear();
    m_captureAvailable = true;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: capture device opened with format " << captureFormat.ToString() << ", processing format " << m_capturePath.GetProcessingFormat().ToString() );
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::StartCaptureDevice( void )
{GUCEF_TRACE;

    if ( !m_captureAvailable || m_captureRunning )
        return;

    // the owner side stops carrying out capture commands before the capture thread can start doing so: both happen under
    // the engine lock, and starting the device thread orders everything done here before it
    m_captureRunning = true;
    m_captureSchedulingReported = false;
    if ( !m_captureDevice->Start() )
    {
        m_captureRunning = false;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: the capture device could not be started, the engine plays without recording" );
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::StopCaptureDevice( void )
{GUCEF_TRACE;

    if ( !m_captureAvailable || m_captureDevice.IsNULL() )
        return;

    // when Stop() returns the capture thread does not use the path any more: the owner side takes over its commands
    m_captureDevice->Stop();
    m_captureRunning = false;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::ReleaseCapture( void )
{GUCEF_TRACE;

    // the capture device is stopped: nothing on the capture thread uses the effects or the consumers any more
    m_captureRunning = false;
    m_captureAvailable = false;
    m_capturePushDevice = GUCEF_NULL;
    m_captureDevice.Unlink();
    m_capturePath.Release();
    m_captureEffects.clear();
    m_captureSlots.clear();
    m_captureConsumers.clear();
    m_captureConsumersLeaving.clear();
    m_captureHalfDuplex = GUCEF_NULL;
    m_captureHalfDuplexLeaving.clear();
    m_echoCanceller = GUCEF_NULL;
    m_echoCancellersLeaving.clear();
    m_captureDeviceLost.Store( false );
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngine::IsCaptureAvailable( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_initialized && m_captureAvailable;
}

/*-------------------------------------------------------------------------*/

TAudioDevicePtr
CAudioEngine::GetCaptureDevice( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_captureDevice;
}

/*-------------------------------------------------------------------------*/

CAudioFormat
CAudioEngine::GetCaptureProcessingFormat( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return ( m_initialized && m_captureAvailable ) ? m_capturePath.GetProcessingFormat() : CAudioFormat();
}

/*-------------------------------------------------------------------------*/

TAudioEffectPtr
CAudioEngine::CreateCaptureEffect( const CString& typeName, const CORE::CValueList& options )
{GUCEF_TRACE;

    CAudioFormat processingFormat;
    UInt32 maxBlock = 0;
    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_initialized || !m_captureAvailable )
            return TAudioEffectPtr();
        processingFormat = m_capturePath.GetProcessingFormat();
        maxBlock = m_capturePath.GetMaxBlockFrames();
    }

    // made and prepared without the engine lock, as CreateEffect() does
    TAudioEffectPtr effect = CAudioGlobal::Instance()->GetAudioEffectRegistry().CreateEffect( typeName, options );
    if ( effect.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: no capture effect of type \"" << typeName << "\" could be made" );
        return TAudioEffectPtr();
    }
    if ( !effect->Prepare( processingFormat, maxBlock ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: the capture effect of type \"" << typeName << "\" could not be prepared for " << processingFormat.ToString() );
        return TAudioEffectPtr();
    }
    return effect;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::PushCaptureRequest( CAudioCapturePath::SCommand& command )
{GUCEF_TRACE;

    // every command makes one event that must not be lost: no more on their way than the queues hold
    if ( m_capturePending.size() >= m_capturePath.GetQueueCapacity() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );

    command.token = NextToken();
    if ( !m_capturePath.PushCommand( command ) )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );

    TSourcePtr source( GUCEF_NEW CORE::CFutureResultSource() );
    m_capturePending[ command.token ] = source;
    return source->GetFuture();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::InsertCaptureEffect( TAudioCaptureStage stage, UInt32 slot, const TAudioEffectPtr& effect )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized || !m_captureAvailable || effect.IsNULL() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    const UInt32 maxEffects = m_capturePath.GetMaxEffectsPerStage();
    if ( ( AUDIOCAPTURESTAGE_PRE_ECHO != stage && AUDIOCAPTURESTAGE_POST_ECHO != stage ) || slot >= maxEffects )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    const size_t index = (size_t) stage * maxEffects + slot;
    if ( GUCEF_NULL != m_captureSlots[ index ] )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    CAudioEffect* raw = RawEffect( effect );
    if ( IsEffectInUse( raw ) || !raw->IsPreparedFor( m_capturePath.GetProcessingFormat(), m_capturePath.GetMaxBlockFrames() ) )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    if ( !raw->ResolveReferences( *this ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: a capture effect of type \"" << raw->GetEffectTypeName() << "\" could not resolve what it refers to, it is not inserted" );
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    }

    CAudioCapturePath::SCommand command;
    command.type = CAudioCapturePath::SCommand::COMMAND_INSERT_EFFECT;
    command.stage = stage;
    command.slot = slot;
    command.effect = raw;
    CORE::CFutureResult future = PushCaptureRequest( command );
    if ( future.HasNoFuture() )
        return future;

    m_captureSlots[ index ] = raw;
    SCaptureEffectEntry& entry = m_captureEffects[ raw ];
    entry.effect = effect;
    entry.stage = stage;
    entry.slot = slot;
    entry.removing = false;
    return future;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::RemoveCaptureEffect( const TAudioEffectPtr& effect )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized || !m_captureAvailable || effect.IsNULL() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    TCaptureEffectMap::iterator e = m_captureEffects.find( RawEffect( effect ) );
    if ( e == m_captureEffects.end() || (*e).second.removing )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    SCaptureEffectEntry& entry = (*e).second;
    CAudioCapturePath::SCommand command;
    command.type = CAudioCapturePath::SCommand::COMMAND_REMOVE_EFFECT;
    command.stage = entry.stage;
    command.slot = entry.slot;
    command.effect = (*e).first;
    CORE::CFutureResult future = PushCaptureRequest( command );
    if ( future.HasNoFuture() )
        return future;

    // the slot is free for a new effect at once: commands are carried out in order. The reference stays until the
    // capture thread reports it let go of the effect
    entry.removing = true;
    m_captureSlots[ (size_t) entry.stage * m_capturePath.GetMaxEffectsPerStage() + entry.slot ] = GUCEF_NULL;
    return future;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::SetCaptureEffectEnabled( TAudioCaptureStage stage, UInt32 slot, bool enabled )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_initialized && m_captureAvailable )
        m_capturePath.SetEffectEnabled( stage, slot, enabled );
}

/*-------------------------------------------------------------------------*/

TAudioEffectPtr
CAudioEngine::GetCaptureEffect( TAudioCaptureStage stage, UInt32 slot ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    const UInt32 maxEffects = m_capturePath.GetMaxEffectsPerStage();
    if ( !m_initialized || !m_captureAvailable || ( AUDIOCAPTURESTAGE_PRE_ECHO != stage && AUDIOCAPTURESTAGE_POST_ECHO != stage ) || slot >= maxEffects )
        return TAudioEffectPtr();
    CAudioEffect* raw = m_captureSlots[ (size_t) stage * maxEffects + slot ];
    TCaptureEffectMap::const_iterator e = m_captureEffects.find( raw );
    return ( GUCEF_NULL == raw || e == m_captureEffects.end() ) ? TAudioEffectPtr() : (*e).second.effect;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::AddCaptureConsumer( IAudioCaptureConsumer* consumer )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized || !m_captureAvailable || GUCEF_NULL == consumer )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    // a consumer that is registered, or whose removal has not reached the capture thread yet, may still be called there:
    // it is not prepared again
    for ( size_t i=0; i<m_captureConsumers.size(); ++i )
        if ( m_captureConsumers[ i ] == consumer )
            return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    for ( size_t i=0; i<m_captureConsumersLeaving.size(); ++i )
        if ( m_captureConsumersLeaving[ i ] == consumer )
            return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    if ( m_captureConsumers.size() >= m_capturePath.GetMaxConsumers() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    if ( m_capturePending.size() >= m_capturePath.GetQueueCapacity() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );

    if ( !consumer->PrepareForCapture( m_capturePath.GetProcessingFormat(), m_capturePath.GetMaxBlockFrames() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: a capture consumer could not prepare for " << m_capturePath.GetProcessingFormat().ToString() << ", it is not added" );
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    }

    CAudioCapturePath::SCommand command;
    command.type = CAudioCapturePath::SCommand::COMMAND_ADD_CONSUMER;
    command.consumer = consumer;
    CORE::CFutureResult future = PushCaptureRequest( command );
    if ( future.HasNoFuture() )
        return future;
    m_captureConsumers.push_back( consumer );
    return future;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::RemoveCaptureConsumer( IAudioCaptureConsumer* consumer )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized || !m_captureAvailable || GUCEF_NULL == consumer )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    size_t index = m_captureConsumers.size();
    for ( size_t i=0; i<m_captureConsumers.size(); ++i )
        if ( m_captureConsumers[ i ] == consumer )
            index = i;
    if ( index == m_captureConsumers.size() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    CAudioCapturePath::SCommand command;
    command.type = CAudioCapturePath::SCommand::COMMAND_REMOVE_CONSUMER;
    command.consumer = consumer;
    CORE::CFutureResult future = PushCaptureRequest( command );
    if ( future.HasNoFuture() )
        return future;

    // its place in the table is free for another consumer at once: commands are carried out in order
    m_captureConsumers.erase( m_captureConsumers.begin() + index );
    m_captureConsumersLeaving.push_back( consumer );
    return future;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::SetCaptureHalfDuplexController( CHalfDuplexController* controller )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized || !m_captureAvailable )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    if ( controller == m_captureHalfDuplex )
        return MakeReadyFuture();

    // a controller the capture thread may still be using is not prepared again
    for ( size_t i=0; GUCEF_NULL != controller && i<m_captureHalfDuplexLeaving.size(); ++i )
        if ( m_captureHalfDuplexLeaving[ i ] == controller )
            return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    if ( m_capturePending.size() >= m_capturePath.GetQueueCapacity() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );

    if ( GUCEF_NULL != controller )
    {
        if ( !controller->ResolveReferences( *this ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: the half-duplex controller could not resolve its far-end bus, it is not set" );
            return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
        }
        if ( !controller->Prepare( m_capturePath.GetProcessingFormat().GetSampleRate() ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: the half-duplex controller could not prepare for " << m_capturePath.GetProcessingFormat().ToString() << ", it is not set" );
            return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
        }
    }

    CAudioCapturePath::SCommand command;
    command.type = CAudioCapturePath::SCommand::COMMAND_SET_HALF_DUPLEX;
    command.halfDuplex = controller;
    CORE::CFutureResult future = PushCaptureRequest( command );
    if ( future.HasNoFuture() )
        return future;

    // the one before stays in use until the capture thread reports that it let go of it
    if ( GUCEF_NULL != m_captureHalfDuplex )
        m_captureHalfDuplexLeaving.push_back( m_captureHalfDuplex );
    m_captureHalfDuplex = controller;
    return future;
}

/*-------------------------------------------------------------------------*/

CHalfDuplexController*
CAudioEngine::GetCaptureHalfDuplexController( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_captureHalfDuplex;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAudioEngine::SetEchoCanceller( CAcousticEchoCanceller* canceller )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized || !m_captureAvailable )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
    if ( canceller == m_echoCanceller )
        return MakeReadyFuture();

    // a canceller that either thread may still be using is not prepared again
    for ( size_t i=0; GUCEF_NULL != canceller && i<m_echoCancellersLeaving.size(); ++i )
        if ( m_echoCancellersLeaving[ i ] == canceller )
            return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );

    // one command for each thread: there must be room for both before either is given
    if ( m_capturePending.size() >= m_capturePath.GetQueueCapacity() || !HasRoomForRequest() )
        return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );

    if ( GUCEF_NULL != canceller )
    {
        const CAudioFormat processing = m_capturePath.GetProcessingFormat();
        const CAudioFormat& mix = m_mixer->GetMixFormat();
        if ( !canceller->Prepare( processing.GetSampleRate(), m_capturePath.GetMaxBlockFrames(), mix.GetSampleRate(), mix.GetChannelCount(), m_maxBlock ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: the echo canceller could not prepare for " << processing.ToString() << " with a reference in " << mix.ToString() << ", it is not set" );
            return MakeFailedFuture( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED );
        }
    }

    SCommand renderCommand;
    renderCommand.type = SCommand::COMMAND_SET_ECHO_REFERENCE;
    renderCommand.echoCanceller = canceller;
    CORE::CFutureResult renderPart = PushRequest( renderCommand );
    if ( renderPart.HasNoFuture() )
        return renderPart;

    CAudioCapturePath::SCommand captureCommand;
    captureCommand.type = CAudioCapturePath::SCommand::COMMAND_SET_ECHO_CANCELLER;
    captureCommand.echoCanceller = canceller;
    CORE::CFutureResult capturePart = PushCaptureRequest( captureCommand );
    if ( capturePart.HasNoFuture() )
    {
        // never expected (the room was checked): the render thread gets the canceller, the capture thread does not, so
        // the render thread is told to let go of it again
        SCommand undo;
        undo.type = SCommand::COMMAND_SET_ECHO_REFERENCE;
        undo.echoCanceller = m_echoCanceller;
        PushRequest( undo );
        if ( GUCEF_NULL != canceller )
            m_echoCancellersLeaving.push_back( canceller );
        return capturePart;
    }

    // the one before stays in use until both threads reported that they let go of it
    if ( GUCEF_NULL != m_echoCanceller )
    {
        m_echoCancellersLeaving.push_back( m_echoCanceller );
        m_echoCancellersLeaving.push_back( m_echoCanceller );
    }
    m_echoCanceller = canceller;

    SLayoutApply apply;
    apply.source = TSourcePtr( GUCEF_NEW CORE::CFutureResultSource() );
    apply.parts.push_back( renderPart );
    apply.parts.push_back( capturePart );
    m_layoutApplies.push_back( apply );
    return apply.source->GetFuture();
}

/*-------------------------------------------------------------------------*/

CAcousticEchoCanceller*
CAudioEngine::GetEchoCanceller( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_echoCanceller;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::ReleaseLeavingEchoCanceller( CAcousticEchoCanceller* canceller )
{GUCEF_TRACE;

    // one thread let go of it
    for ( size_t i=0; i<m_echoCancellersLeaving.size(); ++i )
    {
        if ( m_echoCancellersLeaving[ i ] == canceller )
        {
            m_echoCancellersLeaving.erase( m_echoCancellersLeaving.begin() + i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::SetCaptureMuted( bool muted )
{GUCEF_TRACE;

    m_capturePath.SetMuted( muted );
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngine::IsCaptureMuted( void ) const
{GUCEF_TRACE;

    return m_capturePath.IsMuted();
}

/*-------------------------------------------------------------------------*/

const CAudioLevelMeter&
CAudioEngine::GetCaptureMeter( void ) const
{GUCEF_TRACE;

    return m_capturePath.GetMeter();
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngine::IsCaptureVoiceActive( void ) const
{GUCEF_TRACE;

    return m_capturePath.IsVoiceActive();
}

/*-------------------------------------------------------------------------*/

UInt64
CAudioEngine::GetCaptureOverrunCount( void ) const
{GUCEF_TRACE;

    return m_capturePath.GetOverrunCount();
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::CompletePendingIn( TPendingMap& pending, UInt32 token, bool success )
{GUCEF_TRACE;

    TPendingMap::iterator i = pending.find( token );
    if ( i == pending.end() )
        return;
    TSourcePtr source = (*i).second;
    pending.erase( i );
    if ( source.IsNULL() )
        return;
    if ( success )
        source->Complete();
    else
        source->Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "the capture thread could not carry out the command" );
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::HandleCaptureEvent( const CAudioCapturePath::SEvent& event )
{GUCEF_TRACE;

    switch ( event.type )
    {
        case CAudioCapturePath::SEvent::EVENT_COMMAND_DONE :
        {
            // a half-duplex controller or echo canceller that was replaced is not used any more by the capture thread
            if ( GUCEF_NULL != event.echoCanceller )
                ReleaseLeavingEchoCanceller( event.echoCanceller );
            for ( size_t i=0; GUCEF_NULL != event.halfDuplex && i<m_captureHalfDuplexLeaving.size(); ++i )
            {
                if ( m_captureHalfDuplexLeaving[ i ] == event.halfDuplex )
                {
                    m_captureHalfDuplexLeaving.erase( m_captureHalfDuplexLeaving.begin() + i );
                    break;
                }
            }
            CompletePendingIn( m_capturePending, event.token, true );
            break;
        }
        case CAudioCapturePath::SEvent::EVENT_COMMAND_FAILED :
        {
            // what did not get its place is not used by the capture thread: the engine lets go of it
            TCaptureEffectMap::iterator e = m_captureEffects.find( event.effect );
            if ( GUCEF_NULL != event.effect && e != m_captureEffects.end() )
            {
                const size_t index = (size_t) (*e).second.stage * m_capturePath.GetMaxEffectsPerStage() + (*e).second.slot;
                if ( index < m_captureSlots.size() && m_captureSlots[ index ] == event.effect )
                    m_captureSlots[ index ] = GUCEF_NULL;
                m_captureEffects.erase( e );
            }
            for ( size_t i=0; GUCEF_NULL != event.consumer && i<m_captureConsumers.size(); ++i )
            {
                if ( m_captureConsumers[ i ] == event.consumer )
                {
                    m_captureConsumers.erase( m_captureConsumers.begin() + i );
                    break;
                }
            }
            CompletePendingIn( m_capturePending, event.token, false );
            break;
        }
        case CAudioCapturePath::SEvent::EVENT_EFFECT_RELEASED :
        {
            // the last use by the capture thread is over: the engine's reference goes, here on the owner side
            TAudioEffectPtr effect;
            TCaptureEffectMap::iterator e = m_captureEffects.find( event.effect );
            if ( e != m_captureEffects.end() )
            {
                effect = (*e).second.effect;
                m_captureEffects.erase( e );
            }
            CompletePendingIn( m_capturePending, event.token, true );
            effect.Unlink();
            break;
        }
        case CAudioCapturePath::SEvent::EVENT_CONSUMER_RELEASED :
        {
            for ( size_t i=0; i<m_captureConsumersLeaving.size(); ++i )
            {
                if ( m_captureConsumersLeaving[ i ] == event.consumer )
                {
                    m_captureConsumersLeaving.erase( m_captureConsumersLeaving.begin() + i );
                    break;
                }
            }
            CompletePendingIn( m_capturePending, event.token, true );
            break;
        }
        default :
            break;
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::UpdateCapture( UInt32& handled )
{GUCEF_TRACE;

    if ( !m_captureAvailable )
        return;

    // a lost capture device calls no more; once it is stopped the owner side carries out the capture commands, so the
    // futures of removals still complete
    bool lost = false;
    if ( m_captureDeviceLost.Exchange( false ) )
    {
        m_captureDevice->Stop();
        m_captureRunning = false;
        lost = true;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: the capture device is lost, the engine plays on without recording" );
    }
    if ( !m_captureRunning )
        m_capturePath.ProcessCommands();

    CAudioCapturePath::SEvent event;
    while ( m_capturePath.PopEvent( event ) )
    {
        HandleCaptureEvent( event );
        ++handled;
    }

    if ( m_captureRunning )
        ReportRealtimeScheduling( m_captureDevice, "capture", m_captureSchedulingReported );

    if ( lost )
    {
        ++handled;
        NotifyObserversFromThread( CaptureDeviceLostEvent );
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::ReportRealtimeScheduling( const TAudioDevicePtr& device, const char* what, bool& reported )
{GUCEF_TRACE;

    // a device thread that asked for real-time scheduling knows whether it got it once its first callback ran; it never
    // logs itself, the engine does it once per start of the device
    if ( reported || device.IsNULL() )
        return;
    TAudioDevicePtr deviceCopy( device );
    CAudioPushDevice* pushDevice = deviceCopy->AsPushDevice();
    if ( GUCEF_NULL == pushDevice || 0 == pushDevice->GetRealtimePriority() )
    {
        reported = true;
        return;
    }
    if ( pushDevice->IsRealtimeSchedulingActive() )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: the " << what << " device thread runs with real-time scheduling, priority " << pushDevice->GetRealtimePriority() );
        reported = true;
    }
    else
    if ( 0 != pushDevice->GetRealtimeSchedulingError() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AudioEngine: the " << what << " device thread did not get real-time scheduling (error " << pushDevice->GetRealtimeSchedulingError() <<
                           "), it runs at normal priority. On Linux the user needs rtprio in /etc/security/limits.d or LimitRTPRIO in the service unit" );
        reported = true;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEngine::Update( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized )
        return 0;

    UInt32 handled = 0;
    SEvent event;
    while ( m_events->Pop( event ) )
    {
        HandleEvent( event );
        ++handled;
    }

    // Pausing and resuming are state changes of the voice, noticed here without any event from the audio thread
    for ( TVoiceMap::iterator i=m_voices.begin(); i!=m_voices.end(); ++i )
    {
        SVoiceEntry& entry = (*i).second;
        if ( !entry.pauseSource.IsNULL() )
        {
            const TAudioVoiceState state = entry.voice->GetState();
            if ( ( entry.pauseTarget && AUDIOVOICESTATE_PAUSED == state ) || ( !entry.pauseTarget && AUDIOVOICESTATE_PLAYING == state ) )
            {
                entry.pauseSource->Complete();
                entry.pauseSource.Unlink();
            }
        }
    }

    // bus layouts whose commands have all been carried out are finished here
    CompleteLayoutApplies();

    // a device reopen whose new device is open is finished here, on the owner side
    FinishReopenWhenOpened();

    // the capture thread's events, the loss of the capture device
    UpdateCapture( handled );

    // requests with a part on each thread (the echo canceller) may have completed with the capture events
    CompleteLayoutApplies();

    if ( m_deviceRunWanted )
        ReportRealtimeScheduling( m_device, "playback", m_playbackSchedulingReported );

    if ( m_deviceLost.Exchange( false ) )
    {
        ++handled;
        NotifyObserversFromThread( DeviceLostEvent );
    }
    return handled;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::CompletePending( UInt32 token, bool success )
{GUCEF_TRACE;

    TPendingMap::iterator i = m_pending.find( token );
    if ( i == m_pending.end() )
        return;
    TSourcePtr source = (*i).second;
    m_pending.erase( i );
    if ( source.IsNULL() )
        return;
    if ( success )
        source->Complete();
    else
        source->Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "the audio thread could not carry out the command" );
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::HandleCommandEvent( const SEvent& event )
{GUCEF_TRACE;

    switch ( event.type )
    {
        case SEvent::EVENT_COMMAND_DONE :
        {
            if ( GUCEF_NULL != event.echoCanceller )
                ReleaseLeavingEchoCanceller( event.echoCanceller );
            CompletePending( event.token, true );
            break;
        }
        case SEvent::EVENT_COMMAND_FAILED :
        {
            // an effect that did not get its slot is not used by the audio thread: the engine lets go of it
            TEffectMap::iterator e = m_effects.find( event.effect );
            if ( e != m_effects.end() )
            {
                TBusEntryMap::iterator b = m_busEntries.find( (*e).second.bus );
                if ( b != m_busEntries.end() && (*e).second.slot < (*b).second.effects.size() && (*b).second.effects[ (*e).second.slot ] == event.effect )
                    (*b).second.effects[ (*e).second.slot ] = GUCEF_NULL;
                m_effects.erase( e );
            }
            CompletePending( event.token, false );
            break;
        }
        case SEvent::EVENT_EFFECT_RELEASED :
        {
            // the last use by the audio thread is over: the engine's reference goes, here on the owner side
            TEffectMap::iterator e = m_effects.find( event.effect );
            if ( e != m_effects.end() )
            {
                TAudioEffectPtr effect = (*e).second.effect;
                m_effects.erase( e );
                CompletePending( event.token, true );
                effect.Unlink();
            }
            else
                CompletePending( event.token, true );
            break;
        }
        case SEvent::EVENT_ROUTING_APPLIED :
        {
            RecycleOrderTable( event.order );
            CompletePending( event.token, true );
            break;
        }
        default :
            break;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngine::CheckVoiceRouting( CAudioVoice* voice ) const
{GUCEF_TRACE;

    // what the engine was made for: the voice may have been made with other capacities than those of the engine
    if ( voice->GetEffectCount() > m_params.GetMaxVoiceEffects() )
        return false;
    for ( UInt32 s=0; s<voice->GetMaxSends(); ++s )
    {
        CAudioBus* target = voice->GetSendBus( s );
        if ( GUCEF_NULL == target )
            continue;
        if ( s >= m_params.GetMaxVoiceSends() || m_busEntries.find( target ) == m_busEntries.end() )
            return false;
    }

    // an effect has one owner at a time: one bus slot or one voice
    for ( UInt32 e=0; e<voice->GetEffectCount(); ++e )
    {
        if ( IsEffectInUse( voice->GetEffectPointer( e ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::ReleaseVoiceEffects( CAudioVoice* voice )
{GUCEF_TRACE;

    for ( UInt32 e=0; e<voice->GetEffectCount(); ++e )
        m_voiceEffects.erase( voice->GetEffectPointer( e ) );
    voice->ReleaseEffects();
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::HandleEvent( const SEvent& event )
{GUCEF_TRACE;

    if ( event.type >= SEvent::EVENT_COMMAND_DONE )
    {
        HandleCommandEvent( event );
        return;
    }

    TVoiceMap::iterator i = m_voices.find( event.voice );
    if ( i == m_voices.end() )
        return;
    SVoiceEntry& entry = (*i).second;

    switch ( event.type )
    {
        case SEvent::EVENT_VOICE_STARTED :
        {
            if ( !entry.startSource.IsNULL() )
            {
                entry.startSource->Complete();
                entry.startSource.Unlink();
            }
            break;
        }
        case SEvent::EVENT_VOICE_START_FAILED :
        {
            ReleaseVoiceEffects( event.voice );
            if ( !entry.startSource.IsNULL() )
                entry.startSource->Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "no free voice in the mixer" );
            if ( !entry.stopSource.IsNULL() )   entry.stopSource->Complete();
            if ( !entry.pauseSource.IsNULL() )  entry.pauseSource->Cancel();
            if ( !entry.endedSource.IsNULL() )  entry.endedSource->Cancel();
            m_voices.erase( i );
            break;
        }
        case SEvent::EVENT_VOICE_ENDED :
        {
            // the audio thread has taken the voice out: its effects are let go of here, on the owner side
            ReleaseVoiceEffects( event.voice );
            TAudioVoicePtr voice = entry.voice;
            if ( !entry.startSource.IsNULL() )  entry.startSource->Complete();
            if ( !entry.stopSource.IsNULL() )   entry.stopSource->Complete();
            if ( !entry.pauseSource.IsNULL() )  entry.pauseSource->Cancel();
            if ( !entry.endedSource.IsNULL() )  entry.endedSource->Complete();
            m_voices.erase( i );

            TVoiceEventData data( voice );
            NotifyObserversFromThread( VoiceEndedEvent, &data );
            break;
        }
        default :
            break;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAudioEngine::WaitFor( const CORE::CFutureResult& future, UInt32 timeoutInMs )
{GUCEF_TRACE;

    const UInt64 startTicks = MT::PrecisionTickCount();
    for ( ;; )
    {
        if ( future.IsReady() )
            return true;
        if ( !IsThreaded() )
            Update();
        if ( future.IsReady() )
            return true;
        if ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks ) >= timeoutInMs )
            return false;
        MT::PrecisionDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::ProcessCommands( void )
{
    // Commands hold plain pointers only: popping one copies a few words, no lock, no reference count
    SCommand command;
    while ( m_commands->Pop( command ) )
    {
        switch ( command.type )
        {
            case SCommand::COMMAND_START_VOICE :
            {
                if ( m_mixer->AddVoice( command.voice ) )
                    PostEvent( SEvent::EVENT_VOICE_STARTED, command.voice );
                else
                    PostEvent( SEvent::EVENT_VOICE_START_FAILED, command.voice );
                break;
            }
            case SCommand::COMMAND_ATTACH_BUS :
            {
                m_mixer->AttachBus( command.bus );
                ApplyBusOrder( command.order, command.token );
                break;
            }
            case SCommand::COMMAND_INSERT_EFFECT :
            {
                SEvent event;
                event.token = command.token;
                if ( GUCEF_NULL == m_mixer->GetBusEffect( command.bus, command.index ) && m_mixer->SetBusEffect( command.bus, command.index, command.effect ) )
                    event.type = SEvent::EVENT_COMMAND_DONE;
                else
                {
                    event.type = SEvent::EVENT_COMMAND_FAILED;
                    event.effect = command.effect;
                }
                PostEvent( event );
                break;
            }
            case SCommand::COMMAND_REMOVE_EFFECT :
            {
                if ( m_mixer->GetBusEffect( command.bus, command.index ) == command.effect )
                    m_mixer->SetBusEffect( command.bus, command.index, GUCEF_NULL );
                SEvent event;
                event.type = SEvent::EVENT_EFFECT_RELEASED;
                event.effect = command.effect;
                event.token = command.token;
                PostEvent( event );
                break;
            }
            case SCommand::COMMAND_SET_SEND :
            {
                m_mixer->SetBusSend( command.bus, command.index, command.target, command.preFader );
                ApplyBusOrder( command.order, command.token );
                break;
            }
            case SCommand::COMMAND_SET_ECHO_REFERENCE :
            {
                // the event carries the canceller the tap no longer feeds
                SEvent event;
                event.type = SEvent::EVENT_COMMAND_DONE;
                event.echoCanceller = m_echoReference;
                event.token = command.token;
                m_echoReference = command.echoCanceller;
                PostEvent( event );
                break;
            }
            default :
                break;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::ApplyBusOrder( SOrderTable* order, UInt32 token )
{
    // the table that was in use goes back to the owner side, which recycles it
    SEvent event;
    event.type = SEvent::EVENT_ROUTING_APPLIED;
    event.order = m_audioOrderTable;
    event.token = token;
    if ( GUCEF_NULL != order )
    {
        m_mixer->SetBusOrder( &order->buses[ 0 ], order->count );
        m_audioOrderTable = order;
    }
    PostEvent( event );
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::PostEvent( SEvent::EType type, CAudioVoice* voice )
{
    SEvent event;
    event.type = type;
    event.voice = voice;
    PostEvent( event );
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::PostEvent( const SEvent& event )
{
    if ( !m_events->Push( event ) )
        m_droppedEvents.FetchAdd( 1, MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::OnRenderAudio( void* buffer, UInt32 frameCount ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    // denormals are flushed for the duration of the render: a decaying tail must not cost more cpu than an active signal
    CORE::CDenormalFlushScope denormalFlushScope;

    ProcessCommands();

    const UInt32 channels = m_deviceFormat.GetChannelCount();
    const UInt32 frameBytes = m_deviceFormat.GetBytesPerFrame();
    UInt8* out = static_cast< UInt8* >( buffer );
    while ( frameCount > 0 )
    {
        const UInt32 block = ( frameCount < m_maxBlock ) ? frameCount : m_maxBlock;
        if ( m_directFloat )
        {
            m_mixer->Mix( reinterpret_cast< Float32* >( out ), block );
            if ( GUCEF_NULL != m_echoReference )
                m_echoReference->WriteReference( reinterpret_cast< const Float32* >( out ), block );
        }
        else
        {
            m_mixer->Mix( &m_scratch[ 0 ], block );
            // the render tap: what is played, after the limiter, for the echo canceller
            if ( GUCEF_NULL != m_echoReference )
                m_echoReference->WriteReference( &m_scratch[ 0 ], block );
            CAudioConverter::ConvertFromFloat32( &m_scratch[ 0 ], out, m_deviceFormat.GetSampleType(), (UInt64) block * channels );
        }
        out += (size_t) block * frameBytes;
        frameCount -= block;
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::OnCaptureAudio( const void* buffer, UInt32 frameCount ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    // the capture thread, through CAudioEngineCaptureCallback. The clock is read first: the block info carries the time the
    // callback started. Denormals are flushed for the duration, as for a render
    const UInt64 tickCount = MT::PrecisionTickCount();
    CORE::CDenormalFlushScope denormalFlushScope;

    // the overrun count goes up before the block after the gap is delivered (CAudioPushDevice), so it belongs to this block
    const UInt64 overruns = ( GUCEF_NULL != m_capturePushDevice ) ? m_capturePushDevice->GetOverrunCount() : 0;
    m_capturePath.ProcessCommands();
    m_capturePath.Process( buffer, frameCount, overruns, tickCount );
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::OnCaptureDeviceLost( void )
{
    // may come from any thread: only noted here, Update() makes something of it
    m_captureDeviceLost.Store( true );
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::OnDeviceLost( void )
{
    // may come from any thread: only noted here, Update() makes something of it
    m_deviceLost.Store( true );
}

/*-------------------------------------------------------------------------*/

void
CAudioEngine::OnVoiceEnded( CAudioVoice* voice, TAudioVoiceEndReason )
{
    PostEvent( SEvent::EVENT_VOICE_ENDED, voice );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
