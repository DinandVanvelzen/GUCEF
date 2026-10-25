/*
 *  gucefGUI_WaylandTestApp: Tests of the Wayland window and input drivers against a fake compositor
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


#ifndef GUCEF_GUI_WAYLANDTESTAPP_TESTFAKEWAYLAND_H
#define GUCEF_GUI_WAYLANDTESTAPP_TESTFAKEWAYLAND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdarg.h>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H
#include "pluginglueWAYLAND_Protocols.h"
#define GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_RUNTIME_H
#include "guidriverWAYLAND_Runtime.h"
#define GUCEF_GUIDRIVERWAYLAND_RUNTIME_H
#endif /* GUCEF_GUIDRIVERWAYLAND_RUNTIME_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TESTWAYLAND {

/** An object of the fake compositor: what libwayland would call a proxy */
struct SFakeProxy
{
    const struct wl_interface* iface;
    UInt32 version;
    UInt32 id;
    void ( **listener )( void );
    void* data;
    bool destroyed;
    SFakeProxy* related;        /**< the surface of an xdg_surface, the xdg_surface of a toplevel, ... */
    bool configured;            /**< (surfaces) the first configure was sent */
    UInt32 outputIndex;         /**< (monitors) the order they were bound in */
    void* queue;                /**< the event queue the proxy was given with proxy_set_queue, NULL for the default one */

    SFakeProxy( void ) : iface( GUCEF_NULL ), version( 1 ), id( 0 ), listener( GUCEF_NULL ), data( GUCEF_NULL ), destroyed( false ), related( GUCEF_NULL ), configured( false ), outputIndex( 0 ), queue( GUCEF_NULL ) {}
};

/** What a monitor tells when it is bound */
struct SFakeOutputSpec
{
    Int32 width;
    Int32 height;
    Int32 refreshInMilliHertz;
    Int32 scale;
    Int32 x;
    Int32 y;
    Int32 logicalWidth;     /**< 0: no xdg_output information */
    Int32 logicalHeight;
    const char* name;

    SFakeOutputSpec( void ) : width( 3840 ), height( 1080 ), refreshInMilliHertz( 144000 ), scale( 1 ), x( 0 ), y( 0 ), logicalWidth( 0 ), logicalHeight( 0 ), name( "DP-1" ) {}
};

typedef void ( *TFakeThunk )( void* argument );

/**
 *  A compositor that lives in the test: the function table of libwayland-client over a handful of objects. It records the
 *  requests as text, answers them as a compositor does (globals, bound objects, configures), and lets a test send the events
 *  of a seat, a surface or a monitor by calling the listeners the code under test registered. Nothing connects anywhere.
 */
class CFakeWayland
{
    public:

    CFakeWayland( void );

    ~CFakeWayland();

    const SWaylandApi& GetApi( void ) const;

    // --- what the compositor offers

    /** The globals of a plain desktop compositor: compositor 6, xdg_wm_base 6, shm, seat 9 with a pointer and a keyboard, one monitor */
    void OfferDefaultGlobals( void );

    void OfferGlobal( const char* interfaceName, UInt32 version );

    /** A global that came later (the registry sends it at the next dispatch) */
    void OfferGlobalLater( const char* interfaceName, UInt32 version );

    void WithdrawGlobal( const char* interfaceName );

    /** The table loses its event queue functions, as the table of a library that has none would */
    void RemoveQueueFunctions( void );

    // --- what the compositor says

    UInt32 seatCapabilities;
    Int32 repeatRate;
    Int32 repeatDelay;
    UInt32 preferredFractionalScale;        /**< 0: the compositor sends none */
    UInt32 decorationReply;                 /**< the mode of zxdg_toplevel_decoration_v1.configure */
    Int32 firstConfigureWidth;              /**< 0 x 0: the client decides */
    Int32 firstConfigureHeight;
    UInt32 firstConfigureStates[ 4 ];
    UInt32 firstConfigureStateCount;
    bool autoConfigure;                     /**< configure a toplevel at its first commit */
    SFakeOutputSpec output;
    GUCEF::vector< SFakeOutputSpec > moreOutputs;      /**< the second, third ... monitor that is bound; the first has 'output' */
    const char* keymapText;                 /**< NULL: no keyboard events */
    CORE::CString activationToken;          /**< what xdg_activation_token_v1.done says */
    bool activationAnswers;                 /**< the compositor answers a committed token request (false: it never does) */
    bool activationFromSocket;              /**< the answer arrives on the socket (the next read brings it), else with the next dispatch of the queue */

    // --- what the code under test did

    GUCEF::vector< CORE::CString > requests;

    bool Requested( const char* text ) const;

    UInt32 CountRequestsWithPrefix( const char* prefix ) const;

    CORE::CString LastRequestWithPrefix( const char* prefix ) const;

    void ClearRequests( void );

    UInt32 prepareReadCalls;
    UInt32 readEventsCalls;
    UInt32 cancelReadCalls;
    UInt32 flushCalls;
    UInt32 rejectedListenerCount;           /**< listeners the code gave that lack a function for an event (libwayland would abort on it) */
    CORE::CString lastRejectedListener;
    bool rejectionsExpected;                /**< a test that gives a short listener on purpose says so */
    static UInt32 g_unexpectedRejectedListeners;        /**< over all fakes: listeners rejected where the test did not expect it */
    UInt32 roundtripCalls;
    UInt32 queuesCreated;
    UInt32 queuesDestroyed;
    UInt32 disconnectCalls;
    UInt32 connectCalls;
    bool flushFails;
    bool connectFails;
    int errorCode;

    // --- the objects

    SFakeProxy* FindProxy( const char* interfaceName, UInt32 index = 0 ) const;

    UInt32 CountLiveProxies( const char* interfaceName ) const;

    // --- events, delivered at once to the listener of the object

    void SendSeatCapabilities( UInt32 capabilities );
    void SendPointerEnter( UInt32 serial, SFakeProxy* surface, double x, double y );
    void SendPointerLeave( UInt32 serial, SFakeProxy* surface );
    void SendPointerMotion( UInt32 time, double x, double y );
    void SendPointerButton( UInt32 serial, UInt32 time, UInt32 button, UInt32 state );
    void SendPointerAxis( UInt32 time, UInt32 axis, double value );
    void SendPointerAxisSource( UInt32 source );
    void SendPointerAxisStop( UInt32 time, UInt32 axis );
    void SendPointerAxisDiscrete( UInt32 axis, Int32 discrete );
    void SendPointerAxisValue120( UInt32 axis, Int32 value120 );
    void SendPointerAxisRelativeDirection( UInt32 axis, UInt32 direction );
    void SendPointerFrame( void );
    void SendKeyboardKeymap( const char* text );
    void SendKeyboardEnter( UInt32 serial, SFakeProxy* surface );
    void SendKeyboardLeave( UInt32 serial, SFakeProxy* surface );
    void SendKeyboardKey( UInt32 serial, UInt32 time, UInt32 evdevKey, UInt32 state );
    void SendKeyboardModifiers( UInt32 serial, UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group );
    void SendKeyboardRepeatInfo( Int32 rate, Int32 delay );
    void SendSurfaceEnter( SFakeProxy* surface, SFakeProxy* output );
    void SendSurfaceLeave( SFakeProxy* surface, SFakeProxy* output );
    void SendPreferredBufferScale( SFakeProxy* surface, Int32 factor );
    void SendFractionalScale( UInt32 scale120 );
    void SendOutputScale( SFakeProxy* output, Int32 scale );
    void SendOutputDone( SFakeProxy* output );
    void SendToplevelConfigure( Int32 width, Int32 height, const UInt32* states, UInt32 stateCount );
    void SendXdgSurfaceConfigure( UInt32 serial );
    void SendToplevelClose( void );
    void SendPing( UInt32 serial );
    void SendFrameDone( SFakeProxy* callback );
    void SendDecorationMode( UInt32 mode );

    /** A toplevel configure and the xdg_surface configure that ends it, with the next serial */
    UInt32 Configure( Int32 width, Int32 height, const UInt32* states, UInt32 stateCount );

    // --- timing of events

    /** An event that comes with the next dispatch (Roundtrip, dispatch_pending) */
    void Post( TFakeThunk thunk, void* argument );

    /** The same for an event of an object that is on a queue of its own: only the dispatch of that queue brings it */
    void PostToQueue( TFakeThunk thunk, void* argument, void* queue, bool fromSocket );

    /** An event that arrives on the socket: the file descriptor becomes readable and the next read_events brings it */
    void PostFromSocket( TFakeThunk thunk, void* argument );

    // --- used by the function table

    struct wl_display* Connect( void );
    void Disconnect( void );
    int Roundtrip( void );
    int DispatchPending( void );
    int PrepareRead( void );
    int ReadEvents( void );
    int GetFd( void ) const;
    int DispatchQueuePending( void* queue );
    int PrepareReadQueue( void* queue );
    struct wl_proxy* Marshal( struct wl_proxy* proxy, uint32_t opcode, const struct wl_interface* iface, uint32_t version, uint32_t flags, va_list args );

    private:

    struct SQueued
    {
        TFakeThunk thunk;
        void* argument;
        void* queue;        /**< NULL: the default queue */
    };

    CFakeWayland( const CFakeWayland& src );
    CFakeWayland& operator=( const CFakeWayland& src );

    SFakeProxy* NewProxy( const struct wl_interface* iface, UInt32 version );
    void DeliverGlobals( void );
    void OnBound( SFakeProxy* proxy );
    void OnRequest( SFakeProxy* proxy, const char* name, SFakeProxy* created, const GUCEF::vector< SFakeProxy* >& objectArgs );
    void DrainQueue( void );
    void DrainQueueOf( void* queue );
    bool HasQueued( void* queue ) const;

    template< typename L > L* ListenerOf( const char* interfaceName, UInt32 index = 0 ) const;

    public:

    static CFakeWayland* g_current;

    private:

    CFakeWayland* m_previous;       /**< the fake that was current before this one: the current one is the youngest alive */
    SWaylandApi m_api;
    GUCEF::vector< SFakeProxy* > m_proxies;
    GUCEF::vector< SQueued > m_queue;
    GUCEF::vector< SQueued > m_socketQueue;
    SFakeProxy m_display;
    SFakeProxy* m_registry;
    struct SGlobal { UInt32 name; CORE::CString interfaceName; UInt32 version; bool announced; };
    GUCEF::vector< SGlobal > m_globals;
    UInt32 m_nextGlobalName;
    UInt32 m_nextId;
    UInt32 m_nextSerial;
    UInt32 m_outputsBound;
    int m_pipe[ 2 ];
    bool m_connected;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      EGL AND LIBDECOR                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

/** EGL and the window glue of the fake: counts what the window asks and hands out stand ins */
class CFakeGraphics
{
    public:

    CFakeGraphics( void );

    ~CFakeGraphics();

    /** The table the window context is given: EGL, the wayland window glue and libdecor (hasLibdecor as asked) */
    const GUIDRIVERWAYLAND::SWaylandGuiRuntime& GetRuntime( void ) const;

    void SetLibdecorAvailable( bool available );

    // what happened
    UInt32 swapCount;
    UInt32 createSurfaceCount;
    UInt32 destroySurfaceCount;
    UInt32 makeCurrentCount;
    UInt32 platformDisplayCalls;
    UInt32 lastPlatform;
    UInt32 windowCreateCount;
    UInt32 windowDestroyCount;
    UInt32 resizeCount;
    int lastWindowWidth;
    int lastWindowHeight;
    int lastViewportWidth;
    int lastViewportHeight;
    UInt32 clearCount;
    bool contextCoreFails;      /**< the 3.3 core context cannot be made: OpenGL ES is tried */
    bool noConfig;              /**< no configuration at all */
    int swapInterval;

    // libdecor
    UInt32 libdecorCreated;
    UInt32 libdecorDispatches;
    UInt32 frameUnrefs;
    CORE::CString frameTitle;
    CORE::CString frameAppId;
    bool frameMapped;
    bool frameFullscreen;
    int lastCommitWidth;
    int lastCommitHeight;
    UInt32 commitCount;
    bool decorateFails;
    int configureWidth;         /**< the content size libdecor's configuration reports; 0 for none */
    int configureHeight;
    int configureState;
    GUIDRIVERWAYLAND::SLibdecorFrameInterface frameCallbacks;
    void* frameUserData;
    struct libdecor_frame* frame;
    bool configureDelivered;    /**< the first configure was given (it comes with the first dispatch after the frame was mapped) */
    bool autoConfigure;

    static CFakeGraphics* g_current;

    private:

    CFakeGraphics* m_previous;
    GUIDRIVERWAYLAND::SWaylandGuiRuntime m_runtime;
};

} // namespace TESTWAYLAND
} // namespace GUCEF

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_WAYLANDTESTAPP_TESTFAKEWAYLAND_H ? */
