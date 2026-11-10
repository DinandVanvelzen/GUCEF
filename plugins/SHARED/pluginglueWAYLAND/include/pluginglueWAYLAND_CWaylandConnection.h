/*
 *  pluginglueWAYLAND: Library that owns the connection to the Wayland compositor for the plugins that share it
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


#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PLUGINGLUE_WAYLAND_MACROS_H
#include "pluginglueWAYLAND_macros.h"
#define GUCEF_PLUGINGLUE_WAYLAND_MACROS_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_MACROS_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H
#include "pluginglueWAYLAND_Protocols.h"
#define GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDAVAILABILITY_H
#include "pluginglueWAYLAND_CWaylandAvailability.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDAVAILABILITY_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDAVAILABILITY_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_IWAYLANDSEATLISTENER_H
#include "pluginglueWAYLAND_IWaylandSeatListener.h"
#define GUCEF_PLUGINGLUE_WAYLAND_IWAYLANDSEATLISTENER_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_IWAYLANDSEATLISTENER_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCURSOR_H
#include "pluginglueWAYLAND_CWaylandCursor.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCURSOR_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCURSOR_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace WAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A display mode a monitor advertises (wl_output.mode) */
struct SWaylandOutputMode
{
    Int32 width;
    Int32 height;
    Int32 refreshInMilliHertz;
    UInt32 flags;                   /**< WL_OUTPUT_MODE_CURRENT and WL_OUTPUT_MODE_PREFERRED bits */

    SWaylandOutputMode( void ) : width( 0 ), height( 0 ), refreshInMilliHertz( 0 ), flags( 0 ) {}
};

/** A monitor: what wl_output and zxdg_output_v1 say about it */
struct GUCEF_PLUGINGLUE_WAYLAND_EXPORT_CPP SWaylandOutput
{
    UInt32 globalName;
    struct wl_output* proxy;
    struct zxdg_output_v1* xdgOutput;
    Int32 x;
    Int32 y;
    Int32 physicalWidthInMm;
    Int32 physicalHeightInMm;
    Int32 transform;
    Int32 scale;                    /**< whole scale factor of wl_output.scale */
    CORE::CString make;
    CORE::CString model;
    CORE::CString name;
    CORE::CString description;
    GUCEF::vector< SWaylandOutputMode > modes;
    bool hasLogicalSize;            /**< zxdg_output_v1 told the size in logical units */
    Int32 logicalX;
    Int32 logicalY;
    Int32 logicalWidth;
    Int32 logicalHeight;
    bool done;                      /**< the compositor sent everything it knows (wl_output.done) */
    CWaylandConnection* owner;

    SWaylandOutput( void )
        : globalName( 0 ), proxy( GUCEF_NULL ), xdgOutput( GUCEF_NULL ), x( 0 ), y( 0 ), physicalWidthInMm( 0 ), physicalHeightInMm( 0 )
        , transform( 0 ), scale( 1 ), make(), model(), name(), description(), modes(), hasLogicalSize( false )
        , logicalX( 0 ), logicalY( 0 ), logicalWidth( 0 ), logicalHeight( 0 ), done( false ), owner( GUCEF_NULL ) {}

    /** The mode of the monitor now, NULL when the compositor did not say */
    const SWaylandOutputMode* GetCurrentMode( void ) const;
};

/** What the plugins know of a wl_surface of the application: the input driver finds the surface of a window context by its id and the scale of it */
struct SWaylandSurfaceInfo
{
    UInt32 id;
    struct wl_surface* surface;
    UInt32 scale120;                /**< the scale the window draws at, 120 is 1.0; pointer coordinates are multiplied by it */

    SWaylandSurfaceInfo( void ) : id( 0 ), surface( GUCEF_NULL ), scale120( 120 ) {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The one connection to the compositor of the process, with what the plugins share of it: the registry and the objects
 *  bound from it, the monitors, the seat with its pointer and keyboard, the pointer image and the surfaces of the windows.
 *
 *  It is pumped from the pulse generator of the application and never blocks: on each pulse the requests are flushed,
 *  the socket is read when it has data and the events are dispatched, all on the thread of the pulse generator, so the
 *  listeners and the callbacks of the plugins need no locks. The window and input plugins get the shared connection with
 *  Acquire() and give it back with Release(); the last release closes the connection. A test makes its own connection
 *  with a table of functions of its own, which is how the protocol is faked.
 */
class GUCEF_PLUGINGLUE_WAYLAND_EXPORT_CPP CWaylandConnection : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent ConnectionLostEvent;      /**< the socket broke or the compositor sent a protocol error: nothing works any more */
    static const CORE::CEvent OutputsChangedEvent;      /**< a monitor came, went or changed (wl_output.done), the scale of a window may have changed with it */

    static void RegisterEvents( void );

    /** The time between two pumps of the connection, in milliseconds */
    static const UInt32 PumpIntervalInMs = 4;

    /** The shared connection of the process, connected; NULL (with the reason in 'problem') when Wayland cannot be used. Pair with Release() */
    static CWaylandConnection* Acquire( CORE::CString* problem = GUCEF_NULL );

    static void Release( CWaylandConnection* connection );

    /** Closes the shared connection whatever the count (the library unloads) */
    static void Deinstance( void );

    /** A connection over the functions given, to the socket 'displayName' names (NULL: as WAYLAND_DISPLAY says). The table must outlive the connection */
    CWaylandConnection( const SWaylandApi& api, const char* displayName = GUCEF_NULL );

    virtual ~CWaylandConnection();

    /** Whether the connection pumps itself on the pulses of the application (true); a test pumps by hand */
    void SetPumpOnPulse( bool pumpOnPulse );

    bool Connect( CORE::CString* problem = GUCEF_NULL );

    void Disconnect( void );

    bool IsConnected( void ) const;

    /** False once the connection broke */
    bool IsHealthy( void ) const;

    const SWaylandApi& GetApi( void ) const;

    struct wl_display* GetDisplay( void ) const;

    /** Sends the requests, takes in the events that are waiting and dispatches them; false when the connection broke */
    bool Pump( void );

    /** Waits for the compositor to have handled the requests so far; at startup and in tests, not for use per frame */
    bool Roundtrip( void );

    // --- the globals

    const TWaylandGlobalList& GetGlobals( void ) const;

    bool HasGlobal( const char* interfaceName, UInt32 minimumVersion = 1 ) const;

    struct wl_compositor* GetCompositor( void ) const;
    UInt32 GetCompositorVersion( void ) const;
    struct xdg_wm_base* GetWmBase( void ) const;
    struct wl_shm* GetShm( void ) const;
    struct wp_viewporter* GetViewporter( void ) const;
    struct wp_fractional_scale_manager_v1* GetFractionalScaleManager( void ) const;
    struct zxdg_decoration_manager_v1* GetDecorationManager( void ) const;
    struct wp_cursor_shape_manager_v1* GetCursorShapeManager( void ) const;
    struct zxdg_output_manager_v1* GetOutputManager( void ) const;
    struct xdg_activation_v1* GetActivationManager( void ) const;

    // --- the monitors

    UInt32 GetOutputCount( void ) const;

    const SWaylandOutput* GetOutput( UInt32 index ) const;

    /** The monitor a wl_output proxy is, NULL when unknown */
    const SWaylandOutput* FindOutput( const struct wl_output* proxy ) const;

    // --- the seat

    struct wl_seat* GetSeat( void ) const;

    /** The version the seat is bound at: pointers of version 5 and newer send frame events after a group of axis events */
    UInt32 GetSeatVersion( void ) const;

    UInt32 GetSeatCapabilities( void ) const;

    struct wl_pointer* GetPointer( void ) const;

    struct wl_keyboard* GetKeyboard( void ) const;

    /** The serial of the last event that came from the user: what move, resize and popup requests need */
    UInt32 GetLastInputSerial( void ) const;

    /** The serial of the last pointer button or key event, 0 when there was none: what an activation token is made from (xdg_activation_token_v1.set_serial) */
    UInt32 GetLastActivationSerial( void ) const;

    UInt32 GetPointerEnterSerial( void ) const;

    struct wl_surface* GetPointerFocus( void ) const;

    struct wl_surface* GetKeyboardFocus( void ) const;

    void AddSeatListener( IWaylandSeatListener* listener );

    void RemoveSeatListener( IWaylandSeatListener* listener );

    CWaylandCursor& GetCursor( void );

    // --- the surfaces of windows

    /** Makes known that a surface is a window of the application; the id is what the input driver is given to find it */
    UInt32 RegisterSurface( struct wl_surface* surface );

    void UnregisterSurface( UInt32 id );

    SWaylandSurfaceInfo* FindSurface( UInt32 id );

    SWaylandSurfaceInfo* FindSurfaceByProxy( const struct wl_surface* surface );

    void SetSurfaceScale( UInt32 id, UInt32 scale120 );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The newest versions of the globals the connection binds: the newest the code has handlers for (each listener covers every event up to there) */
    static const UInt32 MaxCompositorVersion = 6;
    static const UInt32 MaxWmBaseVersion = 6;
    static const UInt32 MaxSeatVersion = 9;
    static const UInt32 MaxOutputVersion = 4;
    static const UInt32 MaxOutputManagerVersion = 3;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier          ,
                           const CORE::CEvent& eventId        ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend struct SWaylandConnectionCallbacks;

    /** Events go to the listeners with the list held in place: listeners that left meanwhile are removed when the last dispatch ends, also when a listener throws */
    class CDispatchScope
    {
        public:

        explicit CDispatchScope( CWaylandConnection& connection ) : m_connection( connection ) { ++m_connection.m_dispatchDepth; }
        ~CDispatchScope() { --m_connection.m_dispatchDepth; m_connection.CompactListeners(); }

        private:

        CDispatchScope( const CDispatchScope& src );
        CDispatchScope& operator=( const CDispatchScope& src );

        CWaylandConnection& m_connection;
    };

    CWaylandConnection( const CWaylandConnection& src );
    CWaylandConnection& operator=( const CWaylandConnection& src );

    void OnGlobal( UInt32 name, const char* interfaceName, UInt32 version );
    void OnGlobalRemoved( UInt32 name );
    void BindGlobal( const SWaylandGlobal& global );
    void AttachOutputExtension( SWaylandOutput& output );
    void RemoveOutput( UInt32 index );
    void OnSeatCapabilitiesEvent( UInt32 capabilities );
    void ReleaseSeat( void );
    void ReleasePointer( void );
    void ReleaseKeyboard( void );
    void OnFatalError( void );
    void CompactListeners( void );

    private:

    typedef GUCEF::vector< SWaylandOutput* > TOutputVector;
    typedef GUCEF::vector< IWaylandSeatListener* > TListenerVector;
    typedef GUCEF::map< UInt32, SWaylandSurfaceInfo* > TSurfaceMap;

    const SWaylandApi& m_api;
    CORE::CString m_displayName;
    struct wl_display* m_display;
    struct wl_registry* m_registry;
    TWaylandGlobalList m_globals;
    bool m_pumpOnPulse;
    bool m_pumping;
    bool m_healthy;
    bool m_pulsing;

    struct wl_compositor* m_compositor;
    UInt32 m_compositorVersion;
    struct xdg_wm_base* m_wmBase;
    struct wl_shm* m_shm;
    struct wp_viewporter* m_viewporter;
    struct wp_fractional_scale_manager_v1* m_fractionalScaleManager;
    struct zxdg_decoration_manager_v1* m_decorationManager;
    struct wp_cursor_shape_manager_v1* m_cursorShapeManager;
    struct zxdg_output_manager_v1* m_outputManager;
    struct xdg_activation_v1* m_activationManager;
    TOutputVector m_outputs;

    struct wl_seat* m_seat;
    UInt32 m_seatVersion;
    UInt32 m_seatName;
    UInt32 m_seatCapabilities;
    struct wl_pointer* m_pointer;
    struct wl_keyboard* m_keyboard;
    UInt32 m_lastInputSerial;
    UInt32 m_lastActivationSerial;
    UInt32 m_pointerEnterSerial;
    struct wl_surface* m_pointerFocus;
    struct wl_surface* m_keyboardFocus;
    wl_fixed_t m_pointerX;
    wl_fixed_t m_pointerY;
    GUCEF::vector< char > m_keymap;
    UInt32 m_keymapFormat;
    bool m_hasRepeatInfo;
    Int32 m_repeatRate;
    Int32 m_repeatDelay;
    TListenerVector m_listeners;
    UInt32 m_dispatchDepth;
    bool m_compactListeners;
    CWaylandCursor m_cursor;

    TSurfaceMap m_surfaces;
    UInt32 m_nextSurfaceId;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WAYLAND */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H ? */
