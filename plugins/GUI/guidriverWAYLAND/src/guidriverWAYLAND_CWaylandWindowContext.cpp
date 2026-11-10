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


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H
#include "pluginglueWAYLAND_CEventGuard.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H ? */

#include "guidriverWAYLAND_CWaylandWindowContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWAYLAND {

using namespace PLUGINGLUE::WAYLAND;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

void* ( *g_getProcAddress )( const char* ) = GUCEF_NULL;

/** The pointer to eglGetProcAddress of the runtime as GL_GETPROCADDRESS hands it out */
void*
GetGlProcAddress( const char* name )
{
    return GUCEF_NULL != g_getProcAddress ? g_getProcAddress( name ) : GUCEF_NULL;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CALLBACKS OF THE COMPOSITOR AND OF LIBDECOR                        //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SWaylandWindowCallbacks
{
    static CWaylandWindowContext* W( void* data ) { return static_cast< CWaylandWindowContext* >( data ); }

    // wl_surface

    static void SurfaceEnter( void* data, struct wl_surface*, struct wl_output* output )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandWindowContext* w = W( data );
        w->m_enteredOutputs.push_back( output );
        const SWaylandOutput* info = w->m_connection.FindOutput( output );
        w->m_scale.OnOutputEntered( output, GUCEF_NULL != info ? info->scale : 1 );
        w->OnScaleChanged();
        GUCEF_WL_GUARD_END
    }

    static void SurfaceLeave( void* data, struct wl_surface*, struct wl_output* output )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandWindowContext* w = W( data );
        for ( UInt32 i=0; i<w->m_enteredOutputs.size(); ++i )
        {
            if ( w->m_enteredOutputs[ i ] == output )
            {
                w->m_enteredOutputs.erase( w->m_enteredOutputs.begin() + i );
                break;
            }
        }
        w->m_scale.OnOutputLeft( output );
        w->OnScaleChanged();
        GUCEF_WL_GUARD_END
    }

    static void SurfacePreferredBufferScale( void* data, struct wl_surface*, int32_t factor )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandWindowContext* w = W( data );
        w->m_scale.OnPreferredBufferScale( factor );
        w->OnScaleChanged();
        GUCEF_WL_GUARD_END
    }

    static void SurfacePreferredBufferTransform( void*, struct wl_surface*, uint32_t ) {}

    // xdg_surface, xdg_toplevel

    static void XdgSurfaceConfigure( void* data, struct xdg_surface*, uint32_t serial )
    {
        GUCEF_WL_GUARD_BEGIN
        W( data )->OnSurfaceConfigureEvent( serial );
        GUCEF_WL_GUARD_END
    }

    static void ToplevelConfigure( void* data, struct xdg_toplevel*, int32_t width, int32_t height, struct wl_array* states )
    {
        GUCEF_WL_GUARD_BEGIN
        const UInt32* list = GUCEF_NULL != states ? static_cast< const UInt32* >( states->data ) : GUCEF_NULL;
        const UInt32 count = GUCEF_NULL != states ? (UInt32) ( states->size / sizeof( UInt32 ) ) : 0;
        W( data )->m_configure.OnToplevelConfigure( width, height, CXdgConfigureState::StatesFromProtocol( list, count ) );
        GUCEF_WL_GUARD_END
    }

    static void ToplevelClose( void* data, struct xdg_toplevel* )
    {
        GUCEF_WL_GUARD_BEGIN
        // the user asked to close the window: the application decides
        W( data )->NotifyObservers( GUI::CWindowContext::WindowContextCloseRequestEvent );
        GUCEF_WL_GUARD_END
    }

    static void ToplevelConfigureBounds( void* data, struct xdg_toplevel*, int32_t width, int32_t height )
    {
        GUCEF_WL_GUARD_BEGIN
        W( data )->m_configure.OnToplevelBounds( width, height );
        GUCEF_WL_GUARD_END
    }

    static void ToplevelWmCapabilities( void* data, struct xdg_toplevel*, struct wl_array* capabilities )
    {
        GUCEF_WL_GUARD_BEGIN
        const UInt32* list = GUCEF_NULL != capabilities ? static_cast< const UInt32* >( capabilities->data ) : GUCEF_NULL;
        const UInt32 count = GUCEF_NULL != capabilities ? (UInt32) ( capabilities->size / sizeof( UInt32 ) ) : 0;
        W( data )->m_configure.OnCapabilities( list, count );
        GUCEF_WL_GUARD_END
    }

    // zxdg_toplevel_decoration_v1

    static void DecorationConfigure( void* data, struct zxdg_toplevel_decoration_v1*, uint32_t mode )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandWindowContext* w = W( data );
        const bool clientSide = GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE == mode;
        w->m_decorationMode = CDecorationPolicy::Effective( w->m_chosenDecoration, clientSide );
        if ( clientSide )
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandWindowContext: The compositor wants the application to draw the window decorations: the window has none" );
        GUCEF_WL_GUARD_END
    }

    // wp_fractional_scale_v1

    static void FractionalPreferredScale( void* data, struct wp_fractional_scale_v1*, uint32_t scale )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandWindowContext* w = W( data );
        w->m_scale.OnFractionalScale( scale );
        w->OnScaleChanged();
        GUCEF_WL_GUARD_END
    }

    // wl_callback (frame)

    static void FrameDone( void* data, struct wl_callback* callback, uint32_t )
    {
        GUCEF_WL_GUARD_BEGIN
        W( data )->OnFrameCallbackDone( callback );
        GUCEF_WL_GUARD_END
    }

    // libdecor

    static void LibdecorError( struct libdecor*, int error, const char* message )
    {
        GUCEF_WL_GUARD_BEGIN
        const char* reported = GUCEF_NULL != message ? message : "";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CWaylandWindowContext: libdecor reports error " << error << ": " << reported );
        GUCEF_WL_GUARD_END
    }

    static void LibdecorConfigure( struct libdecor_frame*, struct libdecor_configuration* configuration, void* data )
    {
        GUCEF_WL_GUARD_BEGIN
        W( data )->OnLibdecorConfigure( configuration );
        GUCEF_WL_GUARD_END
    }

    static void LibdecorClose( struct libdecor_frame*, void* data )
    {
        GUCEF_WL_GUARD_BEGIN
        W( data )->NotifyObservers( GUI::CWindowContext::WindowContextCloseRequestEvent );
        GUCEF_WL_GUARD_END
    }

    static void LibdecorCommit( struct libdecor_frame*, void* data )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandWindowContext* w = W( data );
        if ( GUCEF_NULL != w->m_surface )
            gucef_wl_surface_commit( &w->m_connection.GetApi(), w->m_surface );
        GUCEF_WL_GUARD_END
    }

    static void LibdecorDismissPopup( struct libdecor_frame*, const char*, void* ) {}
};

namespace {

const struct gucef_wl_surface_listener g_surfaceListener = {
    SWaylandWindowCallbacks::SurfaceEnter, SWaylandWindowCallbacks::SurfaceLeave,
    SWaylandWindowCallbacks::SurfacePreferredBufferScale, SWaylandWindowCallbacks::SurfacePreferredBufferTransform };

const struct gucef_xdg_surface_listener g_xdgSurfaceListener = { SWaylandWindowCallbacks::XdgSurfaceConfigure };

const struct gucef_xdg_toplevel_listener g_toplevelListener = {
    SWaylandWindowCallbacks::ToplevelConfigure, SWaylandWindowCallbacks::ToplevelClose,
    SWaylandWindowCallbacks::ToplevelConfigureBounds, SWaylandWindowCallbacks::ToplevelWmCapabilities };

const struct gucef_zxdg_toplevel_decoration_v1_listener g_decorationListener = { SWaylandWindowCallbacks::DecorationConfigure };

const struct gucef_wp_fractional_scale_v1_listener g_fractionalScaleListener = { SWaylandWindowCallbacks::FractionalPreferredScale };

const struct gucef_wl_callback_listener g_frameListener = { SWaylandWindowCallbacks::FrameDone };

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWaylandWindowContext::CWaylandWindowContext( CWaylandConnection& connection, const SWaylandGuiRuntime& runtime )
    : GUI::CWindowContext()
    , m_connection( connection )
    , m_runtime( runtime )
    , m_guiContext()
    , m_name()
    , m_appId()
    , m_releaseConnection( false )
    , m_initialized( false )
    , m_ready( false )
    , m_shutDown( false )
    , m_closeReported( false )
    , m_surface( GUCEF_NULL )
    , m_surfaceId( 0 )
    , m_activation( GUCEF_NULL )
    , m_xdgSurface( GUCEF_NULL )
    , m_toplevel( GUCEF_NULL )
    , m_decoration( GUCEF_NULL )
    , m_viewport( GUCEF_NULL )
    , m_fractionalScale( GUCEF_NULL )
    , m_frameCallback( GUCEF_NULL )
    , m_libdecor( GUCEF_NULL )
    , m_frame( GUCEF_NULL )
    , m_eglDisplay( GUCEF_NULL )
    , m_eglConfig( GUCEF_NULL )
    , m_eglContext( GUCEF_NULL )
    , m_eglSurface( GUCEF_NULL )
    , m_eglWindow( GUCEF_NULL )
    , m_isGles( false )
    , m_isCoreProfile( false )
    , m_current( false )
    , m_glViewport( GUCEF_NULL )
    , m_glClearColor( GUCEF_NULL )
    , m_glClear( GUCEF_NULL )
    , m_configure()
    , m_scale()
    , m_pacer()
    , m_decorationMode( DECORATION_NONE )
    , m_chosenDecoration( DECORATION_NONE )
    , m_enteredOutputs()
    , m_logicalWidth( 0 )
    , m_logicalHeight( 0 )
    , m_bufferWidth( 0 )
    , m_bufferHeight( 0 )
    , m_appliedBufferScale( 1 )
    , m_active( false )
    , m_wantFullscreen( false )
    , m_pulsing( false )
    , m_subscribedToConnection( false )
{GUCEF_TRACE;

    memset( &m_libdecorInterface, 0, sizeof( m_libdecorInterface ) );
    memset( &m_libdecorFrameInterface, 0, sizeof( m_libdecorFrameInterface ) );
}

/*-------------------------------------------------------------------------*/

CWaylandWindowContext::~CWaylandWindowContext()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::SetGuiContext( GUI::TGuiContextPtr& context )
{GUCEF_TRACE;

    m_guiContext = context;
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CWaylandWindowContext::GetGuiContext( void )
{GUCEF_TRACE;

    return m_guiContext;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CWaylandWindowContext::GetWidth( void ) const
{GUCEF_TRACE;

    return m_bufferWidth > 0 ? (GUI::UInt32) m_bufferWidth : 0;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CWaylandWindowContext::GetHeight( void ) const
{GUCEF_TRACE;

    return m_bufferHeight > 0 ? (GUI::UInt32) m_bufferHeight : 0;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CWaylandWindowContext::GetID( void ) const
{GUCEF_TRACE;

    return m_surfaceId;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandWindowContext::IsActive( void ) const
{GUCEF_TRACE;

    return m_active;
}

/*-------------------------------------------------------------------------*/

GUI::CString
CWaylandWindowContext::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

GUI::CString
CWaylandWindowContext::GetProperty( const GUI::CString& propertyName ) const
{GUCEF_TRACE;

    // the surface id is what the input driver is given to find the surface of this window
    if ( propertyName == "WINDOW" )
        return CORE::UInt32ToString( m_surfaceId );
    if ( propertyName == "WINDOW_SYSTEM" )
        return "wayland";
    if ( propertyName == "DISPLAY" )
        return CORE::PointerToString( m_connection.GetDisplay() );
    if ( propertyName == "WL_SURFACE" )
        return CORE::PointerToString( m_surface );
    if ( propertyName == "GL_GETPROCADDRESS" )
        return CORE::PointerToString( (const void*) &GetGlProcAddress );
    if ( propertyName == "GL_FRAMEBUFFER" )
        return "0";
    if ( propertyName == "GL_API" )
        return m_isGles ? "GLES" : "GL";
    if ( propertyName == "GL_VERSION" )
        return m_isGles ? "3.0" : ( m_isCoreProfile ? "3.3" : "" );
    if ( propertyName == "SCALE" )
        return CWaylandScale::ToString( GetScale120() );
    if ( propertyName == "LOGICAL_WIDTH" )
        return CORE::Int32ToString( m_logicalWidth );
    if ( propertyName == "LOGICAL_HEIGHT" )
        return CORE::Int32ToString( m_logicalHeight );
    if ( propertyName == "DECORATIONS" )
        return CDecorationPolicy::ToString( m_decorationMode );
    return GUI::CString();
}

/*-------------------------------------------------------------------------*/

bool
CWaylandWindowContext::SetSystemCursorVisible( bool visible )
{GUCEF_TRACE;

    if ( !m_initialized )
        return false;
    m_connection.GetCursor().SetVisible( visible );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::SetCursorShape( CWaylandCursor::ECursorShape shape )
{GUCEF_TRACE;

    m_connection.GetCursor().SetShape( shape );
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::SetReleaseConnectionOnShutdown( bool release )
{GUCEF_TRACE;

    m_releaseConnection = release;
}

/*-------------------------------------------------------------------------*/

EDecorationMode CWaylandWindowContext::GetDecorationMode( void ) const { return m_decorationMode; }
UInt32 CWaylandWindowContext::GetScale120( void ) const { return m_scale.GetScale120(); }
UInt32 CWaylandWindowContext::GetSurfaceId( void ) const { return m_surfaceId; }
UInt32 CWaylandWindowContext::GetLogicalWidth( void ) const { return m_logicalWidth > 0 ? (UInt32) m_logicalWidth : 0; }
UInt32 CWaylandWindowContext::GetLogicalHeight( void ) const { return m_logicalHeight > 0 ? (UInt32) m_logicalHeight : 0; }
bool CWaylandWindowContext::IsFullscreen( void ) const { return 0 != ( m_configure.GetStates() & TOPLEVELSTATE_FULLSCREEN ); }
bool CWaylandWindowContext::IsMaximized( void ) const { return 0 != ( m_configure.GetStates() & TOPLEVELSTATE_MAXIMIZED ); }
bool CWaylandWindowContext::IsConfigured( void ) const { return m_configure.IsConfigured(); }
UInt32 CWaylandWindowContext::GetFramesDrawn( void ) const { return m_pacer.GetFramesDrawn(); }

/*-------------------------------------------------------------------------*/

bool
CWaylandWindowContext::Initialize( const GUI::CString& title                ,
                                   const GUI::CVideoSettings& videoSettings ,
                                   const SWaylandWindowParams& params       )
{GUCEF_TRACE;

    Shutdown();
    m_shutDown = false;

    const SWaylandApi& api = m_connection.GetApi();
    if ( !m_connection.IsHealthy() || GUCEF_NULL == m_connection.GetCompositor() || GUCEF_NULL == m_connection.GetWmBase() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandWindowContext: The compositor does not offer wl_compositor and xdg_wm_base" );
        return false;
    }
    if ( !m_runtime.hasEgl || !m_runtime.hasEglWindow )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandWindowContext: EGL (libEGL.so.1 and libwayland-egl.so.1) is not available" );
        return false;
    }

    m_name = title;
    m_appId = params.appId;
    m_wantFullscreen = videoSettings.GetFullscreenState();
    g_getProcAddress = m_runtime.egl.getProcAddress;

    m_logicalWidth = (Int32) videoSettings.GetResolutionWidthInPixels();
    m_logicalHeight = (Int32) videoSettings.GetResolutionHeightInPixels();
    if ( m_logicalWidth <= 0 ) m_logicalWidth = 1280;
    if ( m_logicalHeight <= 0 ) m_logicalHeight = 720;
    m_configure.Reset();
    m_configure.SetRequestedSize( m_logicalWidth, m_logicalHeight );
    m_pacer = CWaylandFramePacer();
    m_pacer.SetVerticalSync( videoSettings.GetVSyncState() );
    m_pacer.SetFrequency( videoSettings.GetFrequency() );

    m_surface = gucef_wl_compositor_create_surface( &api, m_connection.GetCompositor() );
    if ( GUCEF_NULL == m_surface )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandWindowContext: The compositor made no surface" );
        return false;
    }
    gucef_wl_surface_add_listener( &api, m_surface, &g_surfaceListener, this );
    m_surfaceId = m_connection.RegisterSurface( m_surface );

    // the browser that a login opens gets what it takes to come to the front instead of opening behind the window
    if ( GUCEF_NULL == CORE::GetActivationTokenProvider() )
    {
        m_activation = new PLUGINGLUE::WAYLAND::CWaylandActivationTokenProvider( m_connection, m_surfaceId, m_appId );
        CORE::SetActivationTokenProvider( m_activation );
    }

    // a fractional scale is applied through a viewport; both have to be there
    const bool fractional = GUCEF_NULL != m_connection.GetViewporter() && GUCEF_NULL != m_connection.GetFractionalScaleManager();
    m_scale = CWaylandScaleTracker();
    m_scale.SetFractionalCapable( fractional );
    if ( fractional )
    {
        m_viewport = gucef_wp_viewporter_get_viewport( &api, m_connection.GetViewporter(), m_surface );
        m_fractionalScale = gucef_wp_fractional_scale_manager_v1_get_fractional_scale( &api, m_connection.GetFractionalScaleManager(), m_surface );
        gucef_wp_fractional_scale_v1_add_listener( &api, m_fractionalScale, &g_fractionalScaleListener, this );
    }

    // who draws the decorations
    m_chosenDecoration = CDecorationPolicy::Choose( params.decorations, GUCEF_NULL != m_connection.GetDecorationManager(), m_runtime.hasLibdecor );
    m_decorationMode = m_chosenDecoration;
    bool created = false;
    if ( DECORATION_LIBDECOR == m_chosenDecoration )
    {
        created = InitializeLibdecor();
        if ( !created )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandWindowContext: libdecor could not make decorations (is a libdecor plugin installed?): the window has none" );
            m_chosenDecoration = DECORATION_NONE;
            m_decorationMode = DECORATION_NONE;
        }
    }
    if ( !created )
        created = InitializeXdg( m_chosenDecoration );
    if ( !created )
    {
        Shutdown();
        return false;
    }
    SetMinimumSize( params.minWidth, params.minHeight );
    SetMaximumSize( params.maxWidth, params.maxHeight );
    m_initialized = true;

    // the first configure (and the scale of the surface) come in response to the first commit: waiting for them here
    // lets the window have its real size when this returns
    for ( int attempt=0; attempt<4; ++attempt )
    {
        const bool scaleKnown = !fractional || CWaylandScaleTracker::SCALESOURCE_FRACTIONAL == m_scale.GetSource();
        if ( m_configure.IsConfigured() && scaleKnown )
            break;
        if ( GUCEF_NULL != m_libdecor )
            m_runtime.libdecor.dispatch( m_libdecor, 0 );
        if ( !m_connection.Roundtrip() )
            break;
    }
    if ( !m_configure.IsConfigured() )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandWindowContext: The compositor has not configured the window yet: it is drawn once it does" );

    ApplySizeAndScale();
    if ( !InitializeEgl() )
    {
        Shutdown();
        return false;
    }
    ApplySizeAndScale();

    TEventCallback connectionLost( this, &CWaylandWindowContext::OnConnectionLostEvent );
    SubscribeTo( &m_connection, CWaylandConnection::ConnectionLostEvent, connectionLost );
    TEventCallback outputsChanged( this, &CWaylandWindowContext::OnOutputsChangedEvent );
    SubscribeTo( &m_connection, CWaylandConnection::OutputsChangedEvent, outputsChanged );
    m_subscribedToConnection = true;
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    TEventCallback pulse( this, &CWaylandWindowContext::OnPulseEvent );
    SubscribeTo( pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, pulse );
    pulseGenerator->RequestPeriodicPulses( this, CWaylandConnection::PumpIntervalInMs );
    m_pulsing = true;

    m_ready = true;
    const char* apiName = m_isGles ? "OpenGL ES 3" : "OpenGL 3.3 core";
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandWindowContext: Window \"" << title << "\" of " << (UInt32) m_logicalWidth << " x " << (UInt32) m_logicalHeight
        << " units, " << (UInt32) m_bufferWidth << " x " << (UInt32) m_bufferHeight << " pixels at scale " << CWaylandScale::ToString( GetScale120() )
        << ", decorations: " << CDecorationPolicy::ToString( m_decorationMode ) << ", " << apiName );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandWindowContext::InitializeXdg( EDecorationMode mode )
{GUCEF_TRACE;

    const SWaylandApi& api = m_connection.GetApi();
    m_xdgSurface = gucef_xdg_wm_base_get_xdg_surface( &api, m_connection.GetWmBase(), m_surface );
    if ( GUCEF_NULL == m_xdgSurface )
        return false;
    gucef_xdg_surface_add_listener( &api, m_xdgSurface, &g_xdgSurfaceListener, this );
    m_toplevel = gucef_xdg_surface_get_toplevel( &api, m_xdgSurface );
    if ( GUCEF_NULL == m_toplevel )
        return false;
    gucef_xdg_toplevel_add_listener( &api, m_toplevel, &g_toplevelListener, this );
    gucef_xdg_toplevel_set_title( &api, m_toplevel, m_name.C_String() );
    if ( !m_appId.IsNULLOrEmpty() )
        gucef_xdg_toplevel_set_app_id( &api, m_toplevel, m_appId.C_String() );

    if ( DECORATION_SERVER == mode && GUCEF_NULL != m_connection.GetDecorationManager() )
    {
        m_decoration = gucef_zxdg_decoration_manager_v1_get_toplevel_decoration( &api, m_connection.GetDecorationManager(), m_toplevel );
        gucef_zxdg_toplevel_decoration_v1_add_listener( &api, m_decoration, &g_decorationListener, this );
        gucef_zxdg_toplevel_decoration_v1_set_mode( &api, m_decoration, GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE );
    }
    if ( m_wantFullscreen )
        gucef_xdg_toplevel_set_fullscreen( &api, m_toplevel, GUCEF_NULL );

    // the first commit, without a buffer, asks the compositor to configure the window
    gucef_wl_surface_commit( &api, m_surface );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandWindowContext::InitializeLibdecor( void )
{GUCEF_TRACE;

    const SLibdecorApi& decor = m_runtime.libdecor;
    m_libdecorInterface.error = SWaylandWindowCallbacks::LibdecorError;
    m_libdecor = decor.create( m_connection.GetDisplay(), &m_libdecorInterface );
    if ( GUCEF_NULL == m_libdecor )
        return false;

    m_libdecorFrameInterface.configure = SWaylandWindowCallbacks::LibdecorConfigure;
    m_libdecorFrameInterface.close = SWaylandWindowCallbacks::LibdecorClose;
    m_libdecorFrameInterface.commit = SWaylandWindowCallbacks::LibdecorCommit;
    m_libdecorFrameInterface.dismiss_popup = SWaylandWindowCallbacks::LibdecorDismissPopup;
    m_frame = decor.decorate( m_libdecor, m_surface, &m_libdecorFrameInterface, this );
    if ( GUCEF_NULL == m_frame )
    {
        decor.unref( m_libdecor );
        m_libdecor = GUCEF_NULL;
        return false;
    }
    decor.frame_set_title( m_frame, m_name.C_String() );
    if ( !m_appId.IsNULLOrEmpty() && GUCEF_NULL != decor.frame_set_app_id )
        decor.frame_set_app_id( m_frame, m_appId.C_String() );
    if ( m_wantFullscreen && GUCEF_NULL != decor.frame_set_fullscreen )
        decor.frame_set_fullscreen( m_frame, GUCEF_NULL );
    decor.frame_map( m_frame );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandWindowContext::InitializeEgl( void )
{GUCEF_TRACE;

    const SEglApi& egl = m_runtime.egl;

    void* display = GUCEF_NULL;
    if ( GUCEF_NULL != egl.getPlatformDisplay )
        display = egl.getPlatformDisplay( GUCEF_EGL_PLATFORM_WAYLAND_KHR, m_connection.GetDisplay(), GUCEF_NULL );
    else
    if ( GUCEF_NULL != egl.getPlatformDisplayEXT )
        display = egl.getPlatformDisplayEXT( GUCEF_EGL_PLATFORM_WAYLAND_KHR, m_connection.GetDisplay(), GUCEF_NULL );
    if ( GUCEF_NULL == display && GUCEF_NULL != egl.getDisplay )
        display = egl.getDisplay( m_connection.GetDisplay() );
    int major = 0, minor = 0;
    if ( GUCEF_NULL == display || GUCEF_EGL_TRUE != egl.initialize( display, &major, &minor ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandWindowContext: No EGL display could be made on the Wayland connection" );
        return false;
    }
    m_eglDisplay = display;

    // OpenGL 3.3 core first, OpenGL ES 3 when the driver has no desktop OpenGL
    void* config = GUCEF_NULL;
    void* context = GUCEF_NULL;
    for ( int attempt=0; attempt<2 && GUCEF_NULL == context; ++attempt )
    {
        const bool gles = 1 == attempt;
        if ( GUCEF_EGL_TRUE != egl.bindApi( gles ? GUCEF_EGL_OPENGL_ES_API : GUCEF_EGL_OPENGL_API ) )
            continue;
        const int configAttributes[] = { GUCEF_EGL_SURFACE_TYPE, GUCEF_EGL_WINDOW_BIT,
                                         GUCEF_EGL_RENDERABLE_TYPE, gles ? GUCEF_EGL_OPENGL_ES3_BIT : GUCEF_EGL_OPENGL_BIT,
                                         GUCEF_EGL_RED_SIZE, 8, GUCEF_EGL_GREEN_SIZE, 8, GUCEF_EGL_BLUE_SIZE, 8, GUCEF_EGL_ALPHA_SIZE, 8,
                                         GUCEF_EGL_STENCIL_SIZE, 8, GUCEF_EGL_NONE };
        int configCount = 0;
        if ( GUCEF_EGL_TRUE != egl.chooseConfig( display, configAttributes, &config, 1, &configCount ) || configCount < 1 )
            continue;
        const int glAttributes[] = { GUCEF_EGL_CONTEXT_MAJOR_VERSION, 3, GUCEF_EGL_CONTEXT_MINOR_VERSION, 3,
                                     GUCEF_EGL_CONTEXT_OPENGL_PROFILE_MASK, GUCEF_EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT, GUCEF_EGL_NONE };
        const int glesAttributes[] = { GUCEF_EGL_CONTEXT_MAJOR_VERSION, 3, GUCEF_EGL_NONE };
        context = egl.createContext( display, config, GUCEF_NULL, gles ? glesAttributes : glAttributes );
        m_isGles = gles;
        m_isCoreProfile = !gles;
    }
    if ( GUCEF_NULL == context )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandWindowContext: Neither an OpenGL 3.3 nor an OpenGL ES 3 context could be made" );
        return false;
    }
    m_eglConfig = config;
    m_eglContext = context;

    // the window of the buffer, at the size in pixels
    m_eglWindow = m_runtime.eglWindow.window_create( m_surface, m_bufferWidth > 0 ? m_bufferWidth : 1, m_bufferHeight > 0 ? m_bufferHeight : 1 );
    if ( GUCEF_NULL == m_eglWindow )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandWindowContext: libwayland-egl made no window" );
        return false;
    }
    m_eglSurface = egl.createWindowSurface( display, config, m_eglWindow, GUCEF_NULL );
    if ( GUCEF_NULL == m_eglSurface )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandWindowContext: EGL made no surface for the window, error " << egl.getError() );
        return false;
    }
    MakeCurrent();
    if ( !m_current )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandWindowContext: The OpenGL context cannot be made current, error " << egl.getError() );
        return false;
    }

    // the compositor paces the window with frame callbacks: a swap that waits for the monitor as well would wait twice
    egl.swapInterval( display, 0 );

    m_glViewport = (void ( * )( int, int, int, int )) egl.getProcAddress( "glViewport" );
    m_glClearColor = (void ( * )( float, float, float, float )) egl.getProcAddress( "glClearColor" );
    m_glClear = (void ( * )( unsigned int )) egl.getProcAddress( "glClear" );
    if ( GUCEF_NULL != m_glViewport ) m_glViewport( 0, 0, m_bufferWidth, m_bufferHeight );
    if ( GUCEF_NULL != m_glClearColor ) m_glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::MakeCurrent( void )
{GUCEF_TRACE;

    if ( m_current || GUCEF_NULL == m_eglSurface )
        return;
    m_current = GUCEF_EGL_TRUE == m_runtime.egl.makeCurrent( m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext );
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::ShutdownEgl( void )
{GUCEF_TRACE;

    const SEglApi& egl = m_runtime.egl;
    if ( GUCEF_NULL != m_eglDisplay )
    {
        // the order of the EGL platform for Wayland: the surface, then the window it was made for (the driver is still told about the window until then),
        // then the context, and the display last, which is still a working connection to the compositor until the connection is closed after this
        egl.makeCurrent( m_eglDisplay, GUCEF_NULL, GUCEF_NULL, GUCEF_NULL );
        if ( GUCEF_NULL != m_eglSurface ) egl.destroySurface( m_eglDisplay, m_eglSurface );
        if ( GUCEF_NULL != m_eglWindow ) m_runtime.eglWindow.window_destroy( m_eglWindow );
        m_eglWindow = GUCEF_NULL;
        if ( GUCEF_NULL != m_eglContext ) egl.destroyContext( m_eglDisplay, m_eglContext );
        if ( GUCEF_NULL != egl.terminate ) egl.terminate( m_eglDisplay );
    }
    if ( GUCEF_NULL != m_eglWindow )
        m_runtime.eglWindow.window_destroy( m_eglWindow );
    m_eglSurface = GUCEF_NULL;
    m_eglContext = GUCEF_NULL;
    m_eglWindow = GUCEF_NULL;
    m_eglDisplay = GUCEF_NULL;
    m_current = false;
    m_glViewport = GUCEF_NULL;
    m_glClearColor = GUCEF_NULL;
    m_glClear = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::Shutdown( void )
{GUCEF_TRACE;

    // the second call (the destructor after DestroyWindowContext()) has nothing left and the connection may be gone
    if ( m_shutDown )
        return;
    m_shutDown = true;

    if ( m_pulsing )
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        pulseGenerator->RequestStopOfPeriodicUpdates( this );
        UnsubscribeFrom( pulseGenerator.GetPointerAlways() );
        m_pulsing = false;
    }
    if ( m_subscribedToConnection )
    {
        UnsubscribeFrom( &m_connection );
        m_subscribedToConnection = false;
    }
    m_initialized = false;
    m_ready = false;

    ShutdownEgl();

    if ( GUCEF_NULL != m_activation )
    {
        if ( CORE::GetActivationTokenProvider() == m_activation )
            CORE::SetActivationTokenProvider( GUCEF_NULL );
        delete m_activation;
        m_activation = GUCEF_NULL;
    }

    const SWaylandApi& api = m_connection.GetApi();
    if ( m_connection.IsConnected() )
    {
        if ( GUCEF_NULL != m_frameCallback ) gucef_wl_callback_destroy_proxy( &api, m_frameCallback );
        if ( GUCEF_NULL != m_decoration ) gucef_zxdg_toplevel_decoration_v1_destroy( &api, m_decoration );
        if ( GUCEF_NULL != m_fractionalScale ) gucef_wp_fractional_scale_v1_destroy( &api, m_fractionalScale );
        if ( GUCEF_NULL != m_viewport ) gucef_wp_viewport_destroy( &api, m_viewport );
        if ( GUCEF_NULL != m_frame ) m_runtime.libdecor.frame_unref( m_frame );
        if ( GUCEF_NULL != m_libdecor ) m_runtime.libdecor.unref( m_libdecor );
        if ( GUCEF_NULL != m_toplevel ) gucef_xdg_toplevel_destroy( &api, m_toplevel );
        if ( GUCEF_NULL != m_xdgSurface ) gucef_xdg_surface_destroy( &api, m_xdgSurface );
        if ( 0 != m_surfaceId ) m_connection.UnregisterSurface( m_surfaceId );
        if ( GUCEF_NULL != m_surface ) gucef_wl_surface_destroy( &api, m_surface );
        if ( GUCEF_NULL != m_connection.GetDisplay() ) api.display_flush( m_connection.GetDisplay() );
    }
    m_frameCallback = GUCEF_NULL;
    m_decoration = GUCEF_NULL;
    m_fractionalScale = GUCEF_NULL;
    m_viewport = GUCEF_NULL;
    m_frame = GUCEF_NULL;
    m_libdecor = GUCEF_NULL;
    m_toplevel = GUCEF_NULL;
    m_xdgSurface = GUCEF_NULL;
    m_surface = GUCEF_NULL;
    m_surfaceId = 0;
    m_enteredOutputs.clear();
    m_bufferWidth = 0;
    m_bufferHeight = 0;

    if ( m_releaseConnection )
    {
        m_releaseConnection = false;
        CWaylandConnection::Release( &m_connection );
    }
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::SetMinimumSize( Int32 width, Int32 height )
{GUCEF_TRACE;

    if ( width < 0 ) width = 0;
    if ( height < 0 ) height = 0;
    if ( GUCEF_NULL != m_frame && GUCEF_NULL != m_runtime.libdecor.frame_set_min_content_size )
        m_runtime.libdecor.frame_set_min_content_size( m_frame, width, height );
    else
    if ( GUCEF_NULL != m_toplevel )
        gucef_xdg_toplevel_set_min_size( &m_connection.GetApi(), m_toplevel, width, height );
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::SetMaximumSize( Int32 width, Int32 height )
{GUCEF_TRACE;

    if ( width < 0 ) width = 0;
    if ( height < 0 ) height = 0;
    if ( GUCEF_NULL != m_frame && GUCEF_NULL != m_runtime.libdecor.frame_set_max_content_size )
        m_runtime.libdecor.frame_set_max_content_size( m_frame, width, height );
    else
    if ( GUCEF_NULL != m_toplevel )
        gucef_xdg_toplevel_set_max_size( &m_connection.GetApi(), m_toplevel, width, height );
}

/*-------------------------------------------------------------------------*/

bool
CWaylandWindowContext::BeginInteractiveMove( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_toplevel || GUCEF_NULL == m_connection.GetSeat() )
        return false;
    gucef_xdg_toplevel_move( &m_connection.GetApi(), m_toplevel, m_connection.GetSeat(), m_connection.GetLastInputSerial() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandWindowContext::BeginInteractiveResize( UInt32 edges )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_toplevel || GUCEF_NULL == m_connection.GetSeat() )
        return false;
    gucef_xdg_toplevel_resize( &m_connection.GetApi(), m_toplevel, m_connection.GetSeat(), m_connection.GetLastInputSerial(), edges );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandWindowContext::ApplyVideoSettings( const GUI::CVideoSettings& videoSettings )
{GUCEF_TRACE;

    if ( !m_initialized )
        return false;

    m_pacer.SetVerticalSync( videoSettings.GetVSyncState() );
    m_pacer.SetFrequency( videoSettings.GetFrequency() );

    // full screen is a request: the compositor answers with a configure, the size of the monitor in it
    const bool wantFullscreen = videoSettings.GetFullscreenState();
    if ( wantFullscreen != m_wantFullscreen )
    {
        m_wantFullscreen = wantFullscreen;
        if ( GUCEF_NULL != m_frame )
        {
            if ( wantFullscreen ) m_runtime.libdecor.frame_set_fullscreen( m_frame, GUCEF_NULL );
            else m_runtime.libdecor.frame_unset_fullscreen( m_frame );
        }
        else
        if ( GUCEF_NULL != m_toplevel )
        {
            if ( wantFullscreen ) gucef_xdg_toplevel_set_fullscreen( &m_connection.GetApi(), m_toplevel, GUCEF_NULL );
            else gucef_xdg_toplevel_unset_fullscreen( &m_connection.GetApi(), m_toplevel );
        }
    }

    // a floating window takes the size of the settings by drawing at that size, the compositor follows
    if ( !wantFullscreen && videoSettings.GetResolutionWidthInPixels() > 0 && videoSettings.GetResolutionHeightInPixels() > 0 )
    {
        if ( m_configure.ApplyClientSize( (Int32) videoSettings.GetResolutionWidthInPixels(), (Int32) videoSettings.GetResolutionHeightInPixels() ) )
        {
            m_logicalWidth = m_configure.GetWidth();
            m_logicalHeight = m_configure.GetHeight();
            ApplySizeAndScale();
            if ( GUCEF_NULL != m_frame )
            {
                struct libdecor_state* state = m_runtime.libdecor.state_new( m_logicalWidth, m_logicalHeight );
                m_runtime.libdecor.frame_commit( m_frame, state, GUCEF_NULL );
                m_runtime.libdecor.state_free( state );
            }
            m_pacer.RequestRedraw();
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::OnSurfaceConfigureEvent( UInt32 serial )
{GUCEF_TRACE;

    SConfigureResult result;
    m_configure.OnSurfaceConfigure( serial, result );
    gucef_xdg_surface_ack_configure( &m_connection.GetApi(), m_xdgSurface, serial );
    ApplyConfigure( result );
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::OnLibdecorConfigure( struct libdecor_configuration* configuration )
{GUCEF_TRACE;

    const SLibdecorApi& decor = m_runtime.libdecor;
    int width = 0, height = 0, windowState = 0;
    if ( !decor.configuration_get_content_size( configuration, m_frame, &width, &height ) )
    {
        width = 0;
        height = 0;
    }
    if ( !decor.configuration_get_window_state( configuration, &windowState ) )
        windowState = 0;
    m_configure.OnToplevelConfigure( width, height, CXdgConfigureState::StatesFromLibdecor( windowState ) );

    SConfigureResult result;
    m_configure.OnSurfaceConfigure( 0, result );

    // libdecor acknowledges the configure with the commit of the state the window takes
    struct libdecor_state* state = decor.state_new( result.width, result.height );
    decor.frame_commit( m_frame, state, configuration );
    decor.state_free( state );
    ApplyConfigure( result );
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::ApplyConfigure( const SConfigureResult& result )
{GUCEF_TRACE;

    m_logicalWidth = result.width;
    m_logicalHeight = result.height;
    m_active = 0 != ( result.states & TOPLEVELSTATE_ACTIVATED );
    m_pacer.SetConfigured( true );
    if ( result.sizeChanged )
        ApplySizeAndScale();
    m_pacer.RequestRedraw();

    // gaining or losing the focus ends what the user was doing with the pointer in the GUI; only a real change of the state of the window counts
    if ( !result.firstConfigure && 0 != ( result.changedStates & TOPLEVELSTATE_ACTIVATED ) )
        NotifyObservers( WindowContextActivateEvent );
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::ApplySizeAndScale( void )
{GUCEF_TRACE;

    if ( m_logicalWidth <= 0 || m_logicalHeight <= 0 || GUCEF_NULL == m_surface )
        return;

    const SWaylandApi& api = m_connection.GetApi();
    const UInt32 scale120 = m_scale.GetScale120();
    Int32 bufferWidth = 0, bufferHeight = 0;
    if ( m_scale.UsesViewport() )
    {
        // the buffer is as large as the scale says, the viewport maps it to the size of the surface
        bufferWidth = (Int32) CWaylandScale::LogicalToBuffer( (UInt32) m_logicalWidth, scale120 );
        bufferHeight = (Int32) CWaylandScale::LogicalToBuffer( (UInt32) m_logicalHeight, scale120 );
        if ( GUCEF_NULL != m_viewport )
            gucef_wp_viewport_set_destination( &api, m_viewport, m_logicalWidth, m_logicalHeight );
    }
    else
    {
        const Int32 whole = m_scale.GetWholeBufferScale();
        bufferWidth = m_logicalWidth * whole;
        bufferHeight = m_logicalHeight * whole;
        if ( whole != m_appliedBufferScale && m_connection.GetCompositorVersion() >= GUCEF_WL_WL_SURFACE_SET_BUFFER_SCALE_SINCE_VERSION )
        {
            gucef_wl_surface_set_buffer_scale( &api, m_surface, whole );
            m_appliedBufferScale = whole;
        }
    }
    m_connection.SetSurfaceScale( m_surfaceId, scale120 );

    // the window shows nothing but itself: the compositor need not blend what is behind it
    if ( GUCEF_NULL != m_connection.GetCompositor() )
    {
        struct wl_region* region = gucef_wl_compositor_create_region( &api, m_connection.GetCompositor() );
        if ( GUCEF_NULL != region )
        {
            gucef_wl_region_add( &api, region, 0, 0, m_logicalWidth, m_logicalHeight );
            gucef_wl_surface_set_opaque_region( &api, m_surface, region );
            gucef_wl_region_destroy( &api, region );
        }
    }

    if ( GUCEF_NULL != m_eglWindow )
    {
        m_runtime.eglWindow.window_resize( m_eglWindow, bufferWidth, bufferHeight, 0, 0 );
        MakeCurrent();
        if ( GUCEF_NULL != m_glViewport )
            m_glViewport( 0, 0, bufferWidth, bufferHeight );
    }

    const bool changed = bufferWidth != m_bufferWidth || bufferHeight != m_bufferHeight;
    m_bufferWidth = bufferWidth;
    m_bufferHeight = bufferHeight;
    if ( changed && m_ready )
        NotifyObservers( WindowContextSizeEvent );
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::OnScaleChanged( void )
{GUCEF_TRACE;

    if ( !m_initialized )
        return;
    ApplySizeAndScale();
    m_pacer.RequestRedraw();
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::OnOutputsChanged( void )
{GUCEF_TRACE;

    // a monitor may have another scale now, or be gone
    for ( UInt32 i=0; i<m_enteredOutputs.size(); )
    {
        const SWaylandOutput* info = m_connection.FindOutput( m_enteredOutputs[ i ] );
        if ( GUCEF_NULL == info )
        {
            m_scale.OnOutputLeft( m_enteredOutputs[ i ] );
            m_enteredOutputs.erase( m_enteredOutputs.begin() + i );
            continue;
        }
        m_scale.OnOutputScaleChanged( m_enteredOutputs[ i ], info->scale );
        ++i;
    }
    OnScaleChanged();
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::OnFrameCallbackDone( struct wl_callback* callback )
{GUCEF_TRACE;

    gucef_wl_callback_destroy_proxy( &m_connection.GetApi(), callback );
    if ( callback == m_frameCallback )
        m_frameCallback = GUCEF_NULL;
    m_pacer.OnFrameCallbackDone();
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::RequestCloseOnce( void )
{GUCEF_TRACE;

    if ( m_closeReported )
        return;
    m_closeReported = true;
    NotifyObservers( WindowContextCloseRequestEvent );
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::Update( UInt32 nowInMs )
{GUCEF_TRACE;

    if ( !m_initialized )
        return;
    if ( !m_connection.IsHealthy() )
    {
        // the compositor is gone: nothing will ever be shown again, the application is asked to end
        RequestCloseOnce();
        return;
    }
    if ( GUCEF_NULL != m_libdecor )
        m_runtime.libdecor.dispatch( m_libdecor, 0 );
    if ( m_pacer.ShouldDraw( nowInMs ) )
        DrawFrame( nowInMs );
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::DrawFrame( UInt32 now )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_eglSurface )
        return;
    MakeCurrent();
    if ( GUCEF_NULL != m_glClear )
        m_glClear( 0x00004000 );        // GL_COLOR_BUFFER_BIT

    NotifyObservers( WindowContextRedrawEvent );

    // the frame callback is asked for before the swap: the swap commits the surface
    const SWaylandApi& api = m_connection.GetApi();
    if ( m_pacer.WantsFrameCallback() )
    {
        m_frameCallback = gucef_wl_surface_frame( &api, m_surface );
        gucef_wl_callback_add_listener( &api, m_frameCallback, &g_frameListener, this );
        m_pacer.OnFrameCallbackRequested( now );
    }
    if ( GUCEF_EGL_TRUE != m_runtime.egl.swapBuffers( m_eglDisplay, m_eglSurface ) )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandWindowContext: eglSwapBuffers failed, error " << m_runtime.egl.getError() );
    m_pacer.OnFrameDrawn( now );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWaylandWindowContext::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::GUIDRIVERWAYLAND::CWaylandWindowContext";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::OnPulseEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Update( CORE::GUCEFGetTickCount() );
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::OnConnectionLostEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RequestCloseOnce();
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowContext::OnOutputsChangedEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    OnOutputsChanged();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERWAYLAND */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
