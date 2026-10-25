/*
 *  guidriverWAYLAND: GUCEF window driver for Wayland: native windows with OpenGL through EGL
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


#ifndef GUCEF_GUIDRIVERWAYLAND_CWAYLANDWINDOWCONTEXT_H
#define GUCEF_GUIDRIVERWAYLAND_CWAYLANDWINDOWCONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CWINDOWCONTEXT_H
#include "gucefGUI_CWindowContext.h"
#define GUCEF_GUI_CWINDOWCONTEXT_H
#endif /* GUCEF_GUI_CWINDOWCONTEXT_H ? */

#ifndef GUCEF_GUI_CVIDEOSETTINGS_H
#include "gucefGUI_CVideoSettings.h"
#define GUCEF_GUI_CVIDEOSETTINGS_H
#endif /* GUCEF_GUI_CVIDEOSETTINGS_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H
#include "pluginglueWAYLAND_CWaylandConnection.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDSCALE_H
#include "pluginglueWAYLAND_CWaylandScale.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDSCALE_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDSCALE_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDACTIVATION_H
#include "pluginglueWAYLAND_CWaylandActivation.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDACTIVATION_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDACTIVATION_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_MACROS_H
#include "guidriverWAYLAND_macros.h"
#define GUCEF_GUIDRIVERWAYLAND_MACROS_H
#endif /* GUCEF_GUIDRIVERWAYLAND_MACROS_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_RUNTIME_H
#include "guidriverWAYLAND_Runtime.h"
#define GUCEF_GUIDRIVERWAYLAND_RUNTIME_H
#endif /* GUCEF_GUIDRIVERWAYLAND_RUNTIME_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_CXDGCONFIGURESTATE_H
#include "guidriverWAYLAND_CXdgConfigureState.h"
#define GUCEF_GUIDRIVERWAYLAND_CXDGCONFIGURESTATE_H
#endif /* GUCEF_GUIDRIVERWAYLAND_CXDGCONFIGURESTATE_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_CWAYLANDFRAMEPACER_H
#include "guidriverWAYLAND_CWaylandFramePacer.h"
#define GUCEF_GUIDRIVERWAYLAND_CWAYLANDFRAMEPACER_H
#endif /* GUCEF_GUIDRIVERWAYLAND_CWAYLANDFRAMEPACER_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_CDECORATIONPOLICY_H
#include "guidriverWAYLAND_CDecorationPolicy.h"
#define GUCEF_GUIDRIVERWAYLAND_CDECORATIONPOLICY_H
#endif /* GUCEF_GUIDRIVERWAYLAND_CDECORATIONPOLICY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What the window is made from besides the title and the video settings */
struct SWaylandWindowParams
{
    CORE::CString appId;                        /**< what the desktop knows the application by (the desktop file name, without .desktop) */
    EDecorationPreference decorations;
    Int32 minWidth;                             /**< in surface units, 0 for none */
    Int32 minHeight;
    Int32 maxWidth;
    Int32 maxHeight;

    SWaylandWindowParams( void ) : appId(), decorations( DECORATIONPREFERENCE_AUTO ), minWidth( 0 ), minHeight( 0 ), maxWidth( 0 ), maxHeight( 0 ) {}
};

/**
 *  A window on a Wayland compositor with an OpenGL context through EGL.
 *
 *  The size the window shows to the GUI (GetWidth(), GetHeight()) is the size of the drawing buffer in pixels, as it is for
 *  every window driver, and pointer coordinates of the input driver are in those pixels too. On a monitor with a scale of
 *  1.5 a window of 1280 x 720 surface units has a buffer of 1920 x 1080 pixels. The properties SCALE, LOGICAL_WIDTH and
 *  LOGICAL_HEIGHT give the scale and the size in surface units for a GUI that wants to draw at that size.
 *
 *  Everything runs on the thread that pumps the connection (the pulse generator of the application): the events of the
 *  compositor arrive in the pump of the connection and the frames are drawn on the pulses.
 */
class GUIDRIVERWAYLAND_PUBLIC_CPP CWaylandWindowContext : public GUI::CWindowContext
{
    public:

    CWaylandWindowContext( PLUGINGLUE::WAYLAND::CWaylandConnection& connection ,
                           const SWaylandGuiRuntime& runtime                   );

    virtual ~CWaylandWindowContext();

    virtual void SetGuiContext( GUI::TGuiContextPtr& context ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::TGuiContextPtr GetGuiContext( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** The size of the drawing buffer in pixels */
    virtual GUI::UInt32 GetWidth( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::UInt32 GetHeight( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::UInt32 GetID( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the compositor says the window has the focus (the activated state) */
    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::CString GetProperty( const GUI::CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetSystemCursorVisible( bool visible ) GUCEF_VIRTUAL_OVERRIDE;

    void SetCursorShape( PLUGINGLUE::WAYLAND::CWaylandCursor::ECursorShape shape );

    bool Initialize( const GUI::CString& title                ,
                     const GUI::CVideoSettings& videoSettings ,
                     const SWaylandWindowParams& params       );

    void Shutdown( void );

    /**
     *  Vertical sync as the settings say; full screen or windowed (a request: the compositor decides and the window shows
     *  what it gets in its size events, there is no switch of the mode of the monitor); in a floating window the size of the settings
     */
    bool ApplyVideoSettings( const GUI::CVideoSettings& videoSettings );

    /** One step of the window, on each pulse: libdecor gets its turn and a frame is drawn when the pace says it is due; 'nowInMs' is a tick count in milliseconds */
    void Update( UInt32 nowInMs );

    void SetMinimumSize( Int32 width, Int32 height );

    void SetMaximumSize( Int32 width, Int32 height );

    /** The window asks the compositor to move it with the pointer, for a window without decorations that draws its own title bar; the last button press must be what asks. False without decorations support from the compositor side or without a seat */
    bool BeginInteractiveMove( void );

    /** The same for resizing at an edge (a combination of the xdg_toplevel resize_edge values) */
    bool BeginInteractiveResize( UInt32 edges );

    /** The connection was acquired for this window: give it back when the window goes */
    void SetReleaseConnectionOnShutdown( bool release );

    EDecorationMode GetDecorationMode( void ) const;

    UInt32 GetScale120( void ) const;

    UInt32 GetSurfaceId( void ) const;

    UInt32 GetLogicalWidth( void ) const;

    UInt32 GetLogicalHeight( void ) const;

    bool IsFullscreen( void ) const;

    bool IsMaximized( void ) const;

    bool IsConfigured( void ) const;

    UInt32 GetFramesDrawn( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend struct SWaylandWindowCallbacks;

    typedef CORE::CTEventHandlerFunctor< CWaylandWindowContext > TEventCallback;

    void OnPulseEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnConnectionLostEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnOutputsChangedEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CWaylandWindowContext( const CWaylandWindowContext& src );
    CWaylandWindowContext& operator=( const CWaylandWindowContext& src );

    bool InitializeXdg( EDecorationMode mode );
    bool InitializeLibdecor( void );
    bool InitializeEgl( void );
    void ShutdownEgl( void );

    void OnSurfaceConfigureEvent( UInt32 serial );
    void OnLibdecorConfigure( struct libdecor_configuration* configuration );
    void ApplyConfigure( const SConfigureResult& result );
    void ApplySizeAndScale( void );
    void OnScaleChanged( void );
    void OnOutputsChanged( void );
    void OnFrameCallbackDone( struct wl_callback* callback );
    void DrawFrame( UInt32 now );
    void MakeCurrent( void );
    void RequestCloseOnce( void );

    private:

    PLUGINGLUE::WAYLAND::CWaylandConnection& m_connection;
    SWaylandGuiRuntime m_runtime;
    GUI::TGuiContextPtr m_guiContext;
    GUI::CString m_name;
    CORE::CString m_appId;
    bool m_releaseConnection;
    bool m_initialized;
    bool m_ready;                   /**< Initialize() is done: size changes are announced from then on */
    bool m_shutDown;                /**< Shutdown() ran: the connection may be gone (this window gave back its reference), it is not touched again */
    bool m_closeReported;

    // protocol objects
    struct wl_surface* m_surface;
    UInt32 m_surfaceId;
    PLUGINGLUE::WAYLAND::CWaylandActivationTokenProvider* m_activation;     /**< gives the launcher of the browser its token while this window is the one that has none; NULL when another window has installed its own */
    struct xdg_surface* m_xdgSurface;
    struct xdg_toplevel* m_toplevel;
    struct zxdg_toplevel_decoration_v1* m_decoration;
    struct wp_viewport* m_viewport;
    struct wp_fractional_scale_v1* m_fractionalScale;
    struct wl_callback* m_frameCallback;
    struct libdecor* m_libdecor;
    struct libdecor_frame* m_frame;
    struct SLibdecorInterface m_libdecorInterface;
    struct SLibdecorFrameInterface m_libdecorFrameInterface;

    // EGL
    void* m_eglDisplay;
    void* m_eglConfig;
    void* m_eglContext;
    void* m_eglSurface;
    struct wl_egl_window* m_eglWindow;
    bool m_isGles;
    bool m_isCoreProfile;
    bool m_current;
    void ( *m_glViewport )( int x, int y, int width, int height );
    void ( *m_glClearColor )( float red, float green, float blue, float alpha );
    void ( *m_glClear )( unsigned int mask );

    // negotiated state
    CXdgConfigureState m_configure;
    PLUGINGLUE::WAYLAND::CWaylandScaleTracker m_scale;
    CWaylandFramePacer m_pacer;
    EDecorationMode m_decorationMode;
    EDecorationMode m_chosenDecoration;
    GUCEF::vector< struct wl_output* > m_enteredOutputs;
    Int32 m_logicalWidth;
    Int32 m_logicalHeight;
    Int32 m_bufferWidth;
    Int32 m_bufferHeight;
    Int32 m_appliedBufferScale;
    bool m_active;
    bool m_wantFullscreen;
    bool m_pulsing;
    bool m_subscribedToConnection;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CWaylandWindowContext, MT::CMutex > TWaylandWindowContextPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERWAYLAND */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERWAYLAND_CWAYLANDWINDOWCONTEXT_H ? */
