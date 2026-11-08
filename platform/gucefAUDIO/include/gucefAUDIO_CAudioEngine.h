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

#ifndef GUCEF_AUDIO_CAUDIOENGINE_H
#define GUCEF_AUDIO_CAUDIOENGINE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_MT_CTSPSCQUEUE_H
#include "gucefMT_CTSpscQueue.h"
#define GUCEF_MT_CTSPSCQUEUE_H
#endif /* GUCEF_MT_CTSPSCQUEUE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_AUDIO_CAUDIOENGINEPARAMS_H
#include "gucefAUDIO_CAudioEngineParams.h"
#define GUCEF_AUDIO_CAUDIOENGINEPARAMS_H
#endif /* GUCEF_AUDIO_CAUDIOENGINEPARAMS_H ? */

#ifndef GUCEF_AUDIO_CAUDIOMIXER_H
#include "gucefAUDIO_CAudioMixer.h"
#define GUCEF_AUDIO_CAUDIOMIXER_H
#endif /* GUCEF_AUDIO_CAUDIOMIXER_H ? */

#ifndef GUCEF_AUDIO_CAUDIODEVICE_H
#include "gucefAUDIO_CAudioDevice.h"
#define GUCEF_AUDIO_CAUDIODEVICE_H
#endif /* GUCEF_AUDIO_CAUDIODEVICE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCLIP_H
#include "gucefAUDIO_CAudioClip.h"
#define GUCEF_AUDIO_CAUDIOCLIP_H
#endif /* GUCEF_AUDIO_CAUDIOCLIP_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECT_H
#include "gucefAUDIO_CAudioEffect.h"
#define GUCEF_AUDIO_CAUDIOEFFECT_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECT_H ? */

#ifndef GUCEF_AUDIO_CAUDIOBUSROUTING_H
#include "gucefAUDIO_CAudioBusRouting.h"
#define GUCEF_AUDIO_CAUDIOBUSROUTING_H
#endif /* GUCEF_AUDIO_CAUDIOBUSROUTING_H ? */

#ifndef GUCEF_AUDIO_CAUDIOBUSLAYOUT_H
#include "gucefAUDIO_CAudioBusLayout.h"
#define GUCEF_AUDIO_CAUDIOBUSLAYOUT_H
#endif /* GUCEF_AUDIO_CAUDIOBUSLAYOUT_H ? */

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

class CAudioEngineUpdateTask;
class CAudioEngineReopenRequest;

/*-------------------------------------------------------------------------*/

/**
 *  Plays sounds: owns an audio device, the mixer with its buses and the voices that play, and keeps the
 *  thread of the application and the real time audio thread apart.
 *
 *  Calls are of three kinds, named by how they work:
 *   - inline: done when the call returns (CreateVoice(), CreateBus(), CreateEffect(), the settings of voices, buses and
 *     effect slots)
 *   - commands for the audio thread, which return a CFutureResult that is ready when the audio thread has
 *     carried the command out: Start(), Stop(), Pause(), Resume(), InsertEffect(), RemoveEffect(), AddSend(), RemoveSend(),
 *     ApplyBusLayout()
 *   - events, raised as notifier events when something happens in the audio thread: VoiceEndedEvent, DeviceLostEvent
 *
 *  The audio thread never waits for the application, never allocates, never locks and never touches a reference count:
 *  commands carry plain pointers, and what the audio thread lets go of (an effect taken out of a slot, a bus order table
 *  it no longer uses) is released on the owner side, when its event comes back. The application never waits for the
 *  audio thread, except in WaitFor() which is for tests and simple programs.
 *
 *  In threaded mode a dedicated thread, known to the task manager, completes futures and raises events as they
 *  happen. In cooperative mode that is done in Update(), which the application calls regularly (every frame, say).
 *  Everything else works the same, and the audio itself never depends on Update() being called.
 *
 *  All member functions can be called from any thread.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioEngine : public CORE::CTSGNotifier ,
                                            private IAudioDeviceCallback ,
                                            private IAudioMixerListener
{
    public:

    /** Raised (from Update, in the thread that runs it, or passed on to the main thread) when a voice has ended. Data: a TVoiceEventData */
    static const CORE::CEvent VoiceEndedEvent;

    /** Raised when the device is gone (unplugged, driver failure). The engine stays as it is, a new engine can be started for another device */
    static const CORE::CEvent DeviceLostEvent;

    typedef CORE::CTCloneableObj< TAudioVoicePtr > TVoiceEventData;

    static void RegisterEvents( void );

    CAudioEngine( void );

    virtual ~CAudioEngine() GUCEF_VIRTUAL_OVERRIDE;

    /** Opens the device and starts the engine. False if no device could be opened or the engine is running already */
    bool Initialize( const CAudioEngineParams& params = CAudioEngineParams() );

    /** Stops everything. Futures that are not done yet are cancelled. Called by the destructor as well */
    void Shutdown( void );

    bool IsInitialized( void ) const;

    /** Name of the driver that is in use, empty when the engine is not initialized */
    CString GetDriverName( void ) const;

    /**
     *  The parameters in use: those given to Initialize() (those of the last Initialize() when the engine is not running,
     *  the defaults before the first), with the driver, device parameters and limiter of the last successful device
     *  reopen put over them. A reopen that failed leaves them as they were. A copy, taken under the engine lock, so it can
     *  be called from any thread, also while a reopen is swapped in on the engine thread. Meant for deriving the
     *  parameters of a reopen: change what should differ and give the result to ReopenDevice() or ReopenDeviceAsync()
     */
    CAudioEngineParams GetParams( void ) const;

    /** The format the device plays in, and the format the mixer works in: float, with the speakers and rate of the device */
    CAudioFormat GetDeviceFormat( void ) const;
    CAudioFormat GetMixFormat( void ) const;

    TAudioDevicePtr GetDevice( void ) const;

    bool IsThreaded( void ) const;

    bool StartDevice( void );
    bool StopDevice( void );

    /** The top of the bus tree. Valid until Shutdown() */
    CAudioBus* GetMasterBus( void );

    /** Makes a bus below 'parent' (the master bus when null). Null when the limit of buses is reached or the parent is not a bus of this engine */
    CAudioBus* CreateBus( const CString& name, CAudioBus* parent = GUCEF_NULL );

    CAudioBus* FindBus( const CString& name );

    /**
     *  A voice for a source (or a clip), to play into 'bus' (the master bus when null). Nothing is audible until Start().
     *  The voice can have as many effects and sends as the engine parameters allow (maxVoiceEffects, maxVoiceSends)
     */
    TAudioVoicePtr CreateVoice( const TAudioSourcePtr& source, CAudioBus* bus = GUCEF_NULL );
    TAudioVoicePtr CreateVoice( const TAudioClipPtr& clip, CAudioBus* bus = GUCEF_NULL );

    /**
     *  Starts a voice. The future is ready when the audio thread has started mixing it. When it cannot be started the
     *  future is in an error status, so HasNoFuture() is true:
     *   - TASKSTATUS_RESOURCE_LIMIT_REACHED at once when too many voices are on their way or playing: a throttle, it is
     *     worth trying again later, and the future stays pending as for the thread pool
     *   - TASKSTATUS_QUEUEING_FAILED at once when the command queue to the audio thread is full
     *   - TASKSTATUS_SETUP_FAILED / TASKSTATUS_TASK_WITH_TYPE_EXISTS at once for an engine that is not running, a voice that
     *     cannot be prepared, or one that was started before
     *   - TASKSTATUS_SETUP_FAILED at once as well for a voice with more effects or a send in a higher slot than the engine
     *     parameters allow (maxVoiceEffects, maxVoiceSends), a send to a bus that is not of this engine, an effect that is in
     *     a bus slot or on another voice, or an effect that cannot be prepared for the mix format
     *   - TASKSTATUS_STARTUP_FAILED later, when the mixer on the audio thread had no free voice
     *  The effects of the voice (CAudioVoice::AddEffect()) are prepared for the mix format here, on the calling thread, when
     *  they are not already. The engine lets go of them, on the owner side (the engine thread, or Update()), once the voice
     *  has ended or failed to start and that is reported; a device reopen with another format prepares them again.
     */
    CORE::CFutureResult Start( const TAudioVoicePtr& voice );

    /** Makes a voice and starts it. The voice is given back when asked for */
    CORE::CFutureResult Play( const TAudioClipPtr& clip, CAudioBus* bus = GUCEF_NULL, TAudioVoicePtr* voice = GUCEF_NULL );

    /** Ends a voice. The future is ready when the voice has ended. For a voice that is not playing it is ready at once */
    CORE::CFutureResult Stop( const TAudioVoicePtr& voice );

    /** The future is ready when the voice is paused (or resumed). Needs a running device to take effect */
    CORE::CFutureResult Pause( const TAudioVoicePtr& voice );
    CORE::CFutureResult Resume( const TAudioVoicePtr& voice );

    /** A future that is ready when the voice has ended, for a voice that was started. Failed for one that was not */
    CORE::CFutureResult GetEndedResult( const TAudioVoicePtr& voice );

    /**
     *  Makes an effect of a registered type (CAudioEffectRegistry) and prepares it for the mix format, ready to be
     *  inserted. Inline. Null for an unknown type, options it does not accept, or an engine that is not running
     */
    TAudioEffectPtr CreateEffect( const CString& typeName, const CORE::CValueList& options = CORE::CValueList() );

    /**
     *  Places an effect in an insert slot of a bus. The engine keeps the effect alive while the audio thread may use it.
     *  The future is ready when the audio thread has placed it. Errors, at once:
     *   - TASKSTATUS_SETUP_FAILED for an engine that is not running, a bus that is not of this engine, an effect that is
     *     not prepared for the mix format (CreateEffect() does that) or is in a slot already or on a voice that was started,
     *     an occupied or unknown slot
     *   - TASKSTATUS_RESOURCE_LIMIT_REACHED when too many commands are on their way (a throttle, try again later)
     *   - TASKSTATUS_QUEUEING_FAILED when the command queue to the audio thread is full
     */
    CORE::CFutureResult InsertEffect( CAudioBus* bus, UInt32 slot, const TAudioEffectPtr& effect );

    /**
     *  Takes an effect out of its slot. The future is ready when the audio thread has let go of it and the engine has
     *  released its reference, on the owner side (the engine thread, or Update()). The slot can be used again at once.
     *  Errors as for InsertEffect(); TASKSTATUS_SETUP_FAILED also for an effect that is not in a slot of this bus
     */
    CORE::CFutureResult RemoveEffect( CAudioBus* bus, const TAudioEffectPtr& effect );

    /**
     *  Sends the audio of bus 'from' to bus 'to' as well, at a linear level, taken before or after the fader of 'from'.
     *  The index of the send slot used is given back (for SetSendLevel() and RemoveSend()). The future is ready when the
     *  audio thread processes with the new routing. Errors as for InsertEffect(); TASKSTATUS_SETUP_FAILED also when no
     *  send slot is free, or when the send would make a cycle (a bus feeding itself): nothing is changed then
     */
    CORE::CFutureResult AddSend( CAudioBus* from, CAudioBus* to, Float32 level, bool preFader, UInt32* sendIndex = GUCEF_NULL );

    /** Removes a send. The future is ready when the audio thread processes without it. Errors as for AddSend() */
    CORE::CFutureResult RemoveSend( CAudioBus* from, UInt32 sendIndex );

    /**
     *  The effect in an insert slot of a bus as the owner side sees it, which is how the slot will be once the commands on
     *  their way have been carried out. Null for an empty slot, a slot that does not exist and a bus of another engine
     */
    TAudioEffectPtr GetBusEffect( const CAudioBus* bus, UInt32 slot ) const;

    /**
     *  Brings the bus tree to what the layout describes, with the steps of CreateBus(), InsertEffect(), RemoveEffect(),
     *  AddSend() and RemoveSend(). It does not matter which thread calls it; the engine is locked while the layout is
     *  worked out and the commands are given, so no other change gets in between, and nothing waits for the audio thread,
     *  which is only ever sent commands. Making effects for the layout (allocation) happens in the call as well.
     *
     *  What the layout names is made so:
     *   - Buses. A bus that the layout names and the engine does not have is created below its parent, in the order of the
     *     layout as far as the parents allow (the layout may list a child before its parent). A bus that the engine has is
     *     kept as it is; only its volume and mute are set. The master bus is the bus named "master", it takes part like any
     *     bus (volume, mute, effects, sends) and has no parent. A parent name that is empty means the master bus. An
     *     existing bus is not moved: a layout that gives it another parent than it has is refused.
     *   - Volume and mute of every named bus are set to the values of the layout.
     *   - Effects. The insert chain of a named bus becomes the chain of the layout, slot by slot. An effect that already is
     *     in that slot with the same type and the same construction options stays (its state and tail are not touched) and
     *     gets the configuration of the layout: parameters that the layout does not name go to the value a new effect of
     *     that kind has, so the result depends on the layout alone, and parameters that already have the value are not set at all. In every other case the
     *     effect in the slot is removed, if there is one, and a new one made from the registry is inserted. The slot's
     *     enabled, wet and dry settings are set to the layout's. An effect in a slot that the layout does not list is removed
     *     (so a layout that names a bus but no effects empties its chain).
     *   - Sends. The sends of a named bus become the sends of the layout. A send to the same target on the same side of the
     *     fader stays, and gets the level of the layout; a send that the layout does not list is removed; a send that
     *     the bus does not have is added. The same pair may be listed more than once.
     *
     *  What the layout does not name is left alone: buses with their volume, mute, effects and sends, voices. Buses are
     *  never removed, there is no removal of buses in the engine, so applying a layout can grow the tree and change the
     *  named parts of it but not shrink it. Effects of voices are not part of a layout.
     *
     *  Refusing. The layout is checked completely before anything is changed, and a layout that cannot be applied as a
     *  whole is refused with the engine exactly as it was. The future is then in an error status at once (HasNoFuture()):
     *   - TASKSTATUS_SETUP_FAILED for an engine that is not running; a bus without a name or a name given twice, or a name
     *     that more than one bus of the engine has; an unknown parent, a parent cycle among new buses, a bus that would
     *     have to move, a parent name for the master bus; more buses than the engine has room for; an effect slot beyond
     *     the capacity of the bus or used twice; an effect type that is not registered, options it does not accept, or one
     *     that cannot be prepared for the mix format; a send to an unknown bus, more sends than a bus has slots, or a send
     *     that closes a cycle (also a send to the bus itself), the sends that stay on buses the layout does not name included
     *   - TASKSTATUS_RESOURCE_LIMIT_REACHED for a layout that needs more commands than the engine can have on their way at
     *     once, a throttle: it may work later, when the audio thread has caught up
     *  Once the layout is accepted every step has been checked and none is expected to fail. If one does anyway (a command
     *  that cannot be queued after all) the steps after it are not taken, what was done stays, nothing is rolled back and
     *  the future is in TASKSTATUS_STARTUP_FAILED at once. If the audio thread reports that it could not carry a command out
     *  the other commands are carried out, nothing is rolled back and the future ends in TASKSTATUS_STARTUP_FAILED when all
     *  are done. When the engine is shut down before the audio thread got to the commands the future is cancelled.
     *
     *  The future is ready when the audio thread has carried out all commands of the layout (completed by the engine thread
     *  or in Update(), like the futures of the single calls). Everything that does not need the audio thread, like the volume
     *  of a bus, is changed when the call returns. A layout that changes nothing gives a future that is ready at once and
     *  sends no command.
     *
     *  Applying the same layout twice changes nothing the second time: no bus, effect or send is made or removed and no
     *  parameter is set, the same effects stay in their slots. Applying what GetBusLayout() gave to a fresh engine builds
     *  the same tree.
     */
    CORE::CFutureResult ApplyBusLayout( const CAudioBusLayout& layout );

    /**
     *  Captures the bus tree: every bus (the master bus first, then in the order they were made, so a parent is always
     *  listed before its children) with parent, volume, mute, sends and the effects in its slots, each with its type,
     *  switch, wet and dry levels and its configuration (SaveConfig(), construction options included). It is the
     *  state as the owner side sees it, which is how the audio thread will have it once the commands on their way are carried
     *  out. Volumes are given back in decibels from the linear factor the bus holds, they can differ a hair from the
     *  decibels they were set from. False, with the layout empty, for an engine that is not running and for an engine that
     *  has buses with the same name or without a name, which a layout cannot describe
     */
    bool GetBusLayout( CAudioBusLayout& layout ) const;

    /**
     *  Replaces the device, for another driver, device or format, while buses, voices and effects stay valid: playing
     *  voices go on after a short gap and streams keep their positions. Owner thread.
     *  Used from 'params': driver name, device parameters and the limiter (mode, ceiling, lookahead, release). Capacities,
     *  block size, queue sizes, threading mode and pool name stay as initialized (they size what exists); differences are
     *  logged and ignored. A reopen is the way to change the limiter of a running engine: the limiter starts afresh in the
     *  gap between the devices, where a change of its latency is not heard as a jump.
     *  The new device is opened on the calling thread before the old one is touched. False, with the old device still
     *  in use, when it cannot be opened or started, when the engine is not running, or while another reopen is pending
     */
    bool ReopenDevice( const CAudioEngineParams& params );

    /**
     *  The same, with the new device opened on a task of the engine's thread pool. The swap happens on the owner side:
     *  on the engine thread in threaded mode, in Update() in cooperative mode. The future is ready after the swap. Errors:
     *   - TASKSTATUS_SETUP_FAILED at once for an engine that is not running or while another reopen is pending
     *   - TASKSTATUS_STARTUP_FAILED later when the new device could not be opened or started: the old device stays
     */
    CORE::CFutureResult ReopenDeviceAsync( const CAudioEngineParams& params );

    /**
     *  Frames by which the mix comes out later than it went in, at the mix rate: the lookahead of the master limiter in
     *  lookahead mode, 0 otherwise (and when the engine is not running). The latency of the device itself comes on top of
     *  it (CAudioDevice::GetLatencyInFrames(), at the same rate). Any thread
     */
    UInt32 GetMixLatencyInFrames( void ) const;

    /** Master volume, same as on the master bus */
    void SetMasterVolume( Float32 volume );
    Float32 GetMasterVolume( void ) const;

    /**
     *  Completes the futures and raises the events that the audio thread has made ready. Done by the engine thread
     *  in threaded mode, so calling it there is optional (and harmless). Returns the number of events that were handled.
     */
    UInt32 Update( void );

    /**
     *  Waits until the future is ready or the time is up. In cooperative mode it keeps calling Update() while it waits, so it is
     *  the way to wait for audio work on the thread that would otherwise have to pump. True when the future is ready.
     */
    bool WaitFor( const CORE::CFutureResult& future, UInt32 timeoutInMs );

    /** Number of voices that are started and not yet reported ended */
    UInt32 GetActiveVoiceCount( void ) const;

    /** Events from the audio thread that were dropped because the event queue was full. Should be 0, a non zero count is a sign the queue sizes are too small */
    UInt64 GetDroppedEventCount( void ) const;

    private:
    friend class CAudioEngineUpdateTask;

    CAudioEngine( const CAudioEngine& src );            /**< not implemented */
    CAudioEngine& operator=( const CAudioEngine& src ); /**< not implemented */

    typedef CORE::CTSharedPtr< CORE::CFutureResultSource, MT::CMutex > TSourcePtr;

    /** A processing order of the buses for the mixer, made on the owner side and released there after use */
    struct SOrderTable
    {
        GUCEF::vector< CAudioBus* > buses;
        UInt32 count;
        SOrderTable( void ) : buses(), count( 0 ) {}
    };

    /** What the audio thread is asked to do. Plain pointers only: copying a command never locks or allocates */
    struct SCommand
    {
        enum EType { COMMAND_NONE = 0, COMMAND_START_VOICE, COMMAND_ATTACH_BUS, COMMAND_INSERT_EFFECT, COMMAND_REMOVE_EFFECT, COMMAND_SET_SEND };
        EType type;
        CAudioVoice* voice;
        CAudioBus* bus;
        CAudioBus* target;          /**< send target, null to free the send slot */
        CAudioEffect* effect;
        SOrderTable* order;         /**< new bus order that comes with a bus or a send */
        UInt32 index;               /**< effect slot or send slot */
        UInt32 token;               /**< the request on the owner side that waits for this command, 0 for none */
        bool preFader;
        SCommand( void ) : type( COMMAND_NONE ), voice( GUCEF_NULL ), bus( GUCEF_NULL ), target( GUCEF_NULL ), effect( GUCEF_NULL ), order( GUCEF_NULL ), index( 0 ), token( 0 ), preFader( false ) {}
    };

    struct SEvent
    {
        enum EType { EVENT_NONE = 0, EVENT_VOICE_STARTED, EVENT_VOICE_START_FAILED, EVENT_VOICE_ENDED, EVENT_COMMAND_DONE, EVENT_COMMAND_FAILED, EVENT_EFFECT_RELEASED, EVENT_ROUTING_APPLIED };
        EType type;
        CAudioVoice* voice;
        CAudioEffect* effect;       /**< an effect the audio thread no longer uses */
        SOrderTable* order;         /**< a bus order the audio thread no longer uses */
        UInt32 token;
        SEvent( void ) : type( EVENT_NONE ), voice( GUCEF_NULL ), effect( GUCEF_NULL ), order( GUCEF_NULL ), token( 0 ) {}
    };

    struct SVoiceEntry
    {
        TAudioVoicePtr voice;
        TSourcePtr startSource;
        TSourcePtr stopSource;
        TSourcePtr pauseSource;
        TSourcePtr endedSource;
        bool pauseTarget;               /**< what the pending pause future waits for: paused (true) or playing again (false) */
        SVoiceEntry( void ) : voice(), startSource(), stopSource(), pauseSource(), endedSource(), pauseTarget( false ) {}
    };

    /** An effect the audio thread may use: the engine's reference keeps it alive until the audio thread let go of it */
    struct SEffectEntry
    {
        TAudioEffectPtr effect;
        CAudioBus* bus;
        UInt32 slot;
        bool removing;
        SEffectEntry( void ) : effect(), bus( GUCEF_NULL ), slot( 0 ), removing( false ) {}
    };

    /** Owner side picture of what is in the slots of a bus, as it will be once the audio thread has caught up */
    struct SBusEntry
    {
        GUCEF::vector< CAudioEffect* > effects;
        GUCEF::vector< CAudioBus* > sends;
        GUCEF::vector< UInt8 > sendPreFader;    /**< per send slot: 1 when the send is taken before the fader */
        SBusEntry( void ) : effects(), sends(), sendPreFader() {}
    };

    /** A bus layout whose commands are on their way: the future that ApplyBusLayout() gave is completed when all parts are done */
    struct SLayoutApply
    {
        TSourcePtr source;
        CORE::CFutureResult::TFutureResultVector parts;
        SLayoutApply( void ) : source(), parts() {}
    };

    struct SLayoutPlan;

    typedef GUCEF::map< CAudioEffect*, SEffectEntry > TEffectMap;
    typedef GUCEF::map< CAudioBus*, SBusEntry > TBusEntryMap;
    typedef GUCEF::map< UInt32, TSourcePtr > TPendingMap;
    typedef GUCEF::map< CAudioVoice*, SVoiceEntry > TVoiceMap;
    typedef GUCEF::set< CAudioEffect* > TEffectSet;

    // IAudioDeviceCallback: the audio thread
    virtual void OnRenderAudio( void* buffer, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnCaptureAudio( const void* buffer, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnDeviceLost( void ) GUCEF_VIRTUAL_OVERRIDE;

    // IAudioMixerListener: also the audio thread
    virtual void OnVoiceEnded( CAudioVoice* voice, TAudioVoiceEndReason reason ) GUCEF_VIRTUAL_OVERRIDE;

    void ProcessCommands( void );
    void PostEvent( SEvent::EType type, CAudioVoice* voice );
    void PostEvent( const SEvent& event );
    void ApplyBusOrder( SOrderTable* order, UInt32 token );
    void HandleEvent( const SEvent& event );
    void HandleCommandEvent( const SEvent& event );
    void CompletePending( UInt32 token, bool success );
    bool CheckVoiceRouting( CAudioVoice* voice ) const;
    void ReleaseVoiceEffects( CAudioVoice* voice );
    bool HasRoomForRequest( void ) const;
    UInt32 NextToken( void );
    SOrderTable* ComputeBusOrder( void );
    void RecycleOrderTable( SOrderTable* order );
    void FreeOrderTables( void );
    CORE::CFutureResult PushRequest( SCommand& command );
    CAudioBus* CreateBusInternal( const CString& name, CAudioBus* parent, CORE::CFutureResult* future );
    CAudioBus* FindBusUnique( const CString& name, bool& ambiguous ) const;
    bool PlanBusLayout( const CAudioBusLayout& layout, SLayoutPlan& plan, CORE::TTaskStatus& refusal );
    bool ExecuteBusLayout( const CAudioBusLayout& layout, SLayoutPlan& plan, CORE::CFutureResult::TFutureResultVector& parts );
    void CompleteLayoutApplies( void );
    void FinishReopenWhenOpened( void );
    bool SwapDevice( TAudioDevicePtr& newDevice, const CString& usedDriverName, const CAudioEngineParams& params );
    void PrepareForMixFormat( const CAudioFormat& mixFormat );
    void ApplyDeviceFormat( const CAudioFormat& deviceFormat );
    void ApplyLimiterParams( const CAudioEngineParams& params );
    void AbandonReopen( void );
    void CompleteAllPending( void );
    CORE::CFutureResult MakeFailedFuture( CORE::TTaskStatus status );
    CORE::CFutureResult MakeReadyFuture( void );
    void StopUpdateThread( void );

    private:

    mutable MT::CMutex m_lock;
    bool m_initialized;
    CAudioEngineParams m_params;
    CString m_usedDriverName;
    TAudioDevicePtr m_device;
    CAudioFormat m_deviceFormat;
    CAudioMixer* m_mixer;
    MT::CTSpscQueue< SCommand >* m_commands;
    MT::CTSpscQueue< SEvent >* m_events;
    GUCEF::vector< Float32 > m_scratch;
    UInt32 m_maxBlock;
    bool m_directFloat;
    MT::CTAtomic< bool > m_deviceLost;
    MT::CTAtomic< UInt64 > m_droppedEvents;
    TVoiceMap m_voices;
    UInt32 m_maxOutstanding;
    GUCEF::vector< TAudioBusPtr > m_buses;
    bool m_threaded;
    CORE::CTSharedPtr< CAudioEngineUpdateTask, MT::CMutex > m_updateTask;

    // owner side of effects, sends and routing
    CAudioBusRouting* m_routing;
    TBusEntryMap m_busEntries;
    TEffectMap m_effects;
    TEffectSet m_voiceEffects;                  /**< the effects of the voices that were started and not reported ended yet */
    TPendingMap m_pending;                      /**< requests waiting for the audio thread, by token. A null source when nobody waits on a future */
    GUCEF::vector< SLayoutApply > m_layoutApplies;      /**< bus layouts that wait for the audio thread */
    UInt32 m_lastToken;
    GUCEF::vector< SOrderTable* > m_orderTables;        /**< every order table, owned here */
    GUCEF::vector< SOrderTable* > m_freeOrderTables;    /**< tables the audio thread does not use */

    // audio thread side
    SOrderTable* m_audioOrderTable;             /**< the bus order the mixer uses now */

    // device reopen
    bool m_deviceRunWanted;                     /**< the device should play: started by Initialize() or StartDevice(), not stopped by StopDevice() */
    bool m_reopenPending;
    bool m_reopenOnOwner;                       /**< no task could open the new device: Update() does it */
    CORE::CTSharedPtr< CAudioEngineReopenRequest, MT::CMutex > m_reopenRequest;
    TSourcePtr m_reopenSource;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOENGINE_H ? */
