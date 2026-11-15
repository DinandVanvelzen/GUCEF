/*
 *  guidriverAndroidGLES: module implementing GLES based window management for Android
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <GLES3/gl3.h>
#include <android_native_app_glue.h>
#include <android/native_window.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_GUI_CWINDOWCONTEXT_H
#include "gucefGUI_CWindowContext.h"
#define GUCEF_GUI_CWINDOWCONTEXT_H
#endif /* GUCEF_GUI_CWINDOWCONTEXT_H ? */

#ifndef GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#include "gucefGUI_CWindowManagerBackend.h"
#define GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#endif /* GUCEF_GUI_CWINDOWMANAGERBACKEND_H ? */

#include "guidriverAndroidGLES_CAndroidGLESWindowContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERANDROIDGLES {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

GUI::UInt32 g_nextWindowId = 1;

/** eglGetProcAddress as the GL_GETPROCADDRESS property hands it out */
void*
GetGlProcAddress( const char* name )
{
    return (void*) ::eglGetProcAddress( name );
}

#ifndef EGL_OPENGL_ES3_BIT_KHR
#define EGL_OPENGL_ES3_BIT_KHR 0x0040
#endif

/*-------------------------------------------------------------------------*/

/*
 *  The windows that follow the commands of an android_app. android_native_app_glue calls android_app::onAppCmd on the
 *  thread that runs the looper of the app, the thread of the application's pulses, so no lock is needed. There is one
 *  android_app per process.
 */
typedef void ( *TAppCommandHandler )( struct android_app* app, int32_t cmd );

GUCEF::vector< CAndroidGLESWindowContext* > g_windows;
struct android_app* g_hookedApp = GUCEF_NULL;
TAppCommandHandler g_previousHandler = GUCEF_NULL;

void
OnAppCommand( struct android_app* app, int32_t cmd )
{
    // a copy: a window may go while it handles the command
    GUCEF::vector< CAndroidGLESWindowContext* > windows( g_windows );
    for ( UInt32 i=0; i<windows.size(); ++i )
    {
        switch ( cmd )
        {
            case APP_CMD_TERM_WINDOW: windows[ i ]->OnNativeWindowLost(); break;
            case APP_CMD_INIT_WINDOW: windows[ i ]->OnNativeWindowRestored( app->window ); break;
            case APP_CMD_WINDOW_RESIZED:
            case APP_CMD_CONTENT_RECT_CHANGED:
            case APP_CMD_CONFIG_CHANGED: windows[ i ]->OnNativeWindowChanged(); break;
            default: break;
        }
    }
    if ( GUCEF_NULL != g_previousHandler )
        g_previousHandler( app, cmd );
}

void
RegisterWindow( CAndroidGLESWindowContext* window, struct android_app* app )
{
    for ( UInt32 i=0; i<g_windows.size(); ++i )
        if ( g_windows[ i ] == window )
            return;
    g_windows.push_back( window );
    if ( GUCEF_NULL == g_hookedApp )
    {
        g_hookedApp = app;
        g_previousHandler = app->onAppCmd;
        app->onAppCmd = OnAppCommand;
    }
}

void
UnregisterWindow( CAndroidGLESWindowContext* window )
{
    for ( UInt32 i=0; i<g_windows.size(); ++i )
    {
        if ( g_windows[ i ] == window )
        {
            g_windows.erase( g_windows.begin() + i );
            break;
        }
    }
    if ( g_windows.empty() && GUCEF_NULL != g_hookedApp )
    {
        // the handler is put back unless someone chained after this one
        if ( g_hookedApp->onAppCmd == OnAppCommand )
            g_hookedApp->onAppCmd = g_previousHandler;
        g_hookedApp = GUCEF_NULL;
        g_previousHandler = GUCEF_NULL;
    }
}

}

/*-------------------------------------------------------------------------*/

CAndroidGLESWindowContext::CAndroidGLESWindowContext( void )
    : CWindowContext()               ,
      m_guiContext()                 ,
      m_id( g_nextWindowId++ )       ,
      m_name()                       ,
      m_display( EGL_NO_DISPLAY )    ,
      m_surface( EGL_NO_SURFACE )    ,
      m_context( EGL_NO_CONTEXT )    ,
      m_nativeWindow( GUCEF_NULL )   ,
      m_drivesRedraws( false )       ,
      m_app( GUCEF_NULL )            ,
      m_config( GUCEF_NULL )         ,
      m_graphicsApiNone( false )     ,
      m_vsync( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAndroidGLESWindowContext::~CAndroidGLESWindowContext()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CAndroidGLESWindowContext::GetWidth( void ) const
{GUCEF_TRACE;

    EGLint width = 0;
    if ( m_graphicsApiNone )
    {
        if ( GUCEF_NULL != m_nativeWindow )
            width = ::ANativeWindow_getWidth( static_cast< ANativeWindow* >( m_nativeWindow ) );
    }
    else
    if ( EGL_NO_SURFACE != m_surface )
        eglQuerySurface( m_display, m_surface, EGL_WIDTH, &width );
    return width > 0 ? (GUI::UInt32) width : 0;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CAndroidGLESWindowContext::GetHeight( void ) const
{GUCEF_TRACE;

    EGLint height = 0;
    if ( m_graphicsApiNone )
    {
        if ( GUCEF_NULL != m_nativeWindow )
            height = ::ANativeWindow_getHeight( static_cast< ANativeWindow* >( m_nativeWindow ) );
    }
    else
    if ( EGL_NO_SURFACE != m_surface )
        eglQuerySurface( m_display, m_surface, EGL_HEIGHT, &height );
    return height > 0 ? (GUI::UInt32) height : 0;
}

/*-------------------------------------------------------------------------*/

GUI::CString
CAndroidGLESWindowContext::GetProperty( const GUI::CString& propertyName ) const
{GUCEF_TRACE;

    if ( propertyName == "WINDOW" )
        return GUCEF_NULL != m_nativeWindow ? CORE::PointerToString( m_nativeWindow ) : GUI::CString();
    if ( propertyName == "ANDROID_CONFIG" )
        return GUCEF_NULL != m_app ? CORE::PointerToString( m_app->config ) : GUI::CString();
    if ( propertyName == "GRAPHICS_API" )
        return m_graphicsApiNone ? GUI::CWindowManagerBackend::GraphicsApiNone : GUI::CWindowManagerBackend::GraphicsApiGL;
    if ( m_graphicsApiNone && propertyName.StartsWith( "GL_" ) )
        return GUI::CString();          // no GL context: the GL properties are not offered
    if ( propertyName == "GL_GETPROCADDRESS" )
        return CORE::PointerToString( (const void*) &GetGlProcAddress );
    if ( propertyName == "GL_FRAMEBUFFER" )
        return "0";
    if ( propertyName == "GL_API" )
        return "GLES";
    if ( propertyName == "GL_VERSION" )
        return "3.0";
    return GUI::CString();
}

/*-------------------------------------------------------------------------*/

void
CAndroidGLESWindowContext::SetGuiContext( GUI::TGuiContextPtr& context )
{GUCEF_TRACE;

    m_guiContext = context;
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CAndroidGLESWindowContext::GetGuiContext( void )
{GUCEF_TRACE;

    return m_guiContext;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CAndroidGLESWindowContext::GetID( void ) const
{GUCEF_TRACE;

    return m_id;
}

/*-------------------------------------------------------------------------*/

bool
CAndroidGLESWindowContext::IsActive( void ) const
{GUCEF_TRACE;

    if ( m_graphicsApiNone )
        return GUCEF_NULL != m_nativeWindow;
    return EGL_NO_CONTEXT != m_context && m_context == eglGetCurrentContext();
}

/*-------------------------------------------------------------------------*/

GUI::CString
CAndroidGLESWindowContext::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

void
CAndroidGLESWindowContext::Shutdown( void )
{GUCEF_TRACE;

    if ( m_drivesRedraws )
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        pulseGenerator->RequestStopOfPeriodicUpdates( this );
        UnsubscribeFrom( pulseGenerator.GetPointerAlways() );
        m_drivesRedraws = false;
    }
    UnregisterWindow( this );
    if ( m_display != EGL_NO_DISPLAY )
    {
        eglMakeCurrent( m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
        if ( m_context != EGL_NO_CONTEXT )
            eglDestroyContext( m_display, m_context );
        if ( m_surface != EGL_NO_SURFACE )
            eglDestroySurface( m_display, m_surface );
        eglTerminate( m_display );
    }
    m_display = EGL_NO_DISPLAY;
    m_context = EGL_NO_CONTEXT;
    m_surface = EGL_NO_SURFACE;
    m_nativeWindow = GUCEF_NULL;
    m_config = GUCEF_NULL;
    m_app = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CAndroidGLESWindowContext::Initialize( const GUI::CString& title, const GUI::CVideoSettings& videoSettings, const CORE::CValueList* params )
{GUCEF_TRACE;

    // Do not initialize twice
    Shutdown();
    m_name = title;

    // the window of the app: android_native_app_glue's android_app, passed on by the GUCEF Android glue
    struct android_app* app = GUCEF_NULL;
    if ( GUCEF_NULL != params && params->HasKey( "android_app" ) )
        app = static_cast< struct android_app* >( CORE::StringToPointer( params->GetValueAlways( "android_app", "" ).AsString() ) );
    if ( GUCEF_NULL == app || GUCEF_NULL == app->window )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AndroidGLESWindowContext: No native window: pass the android_app of the app as the parameter \"android_app\"" );
        return false;
    }
    m_nativeWindow = app->window;
    m_app = app;
    m_vsync = videoSettings.GetVSyncState();
    const GUI::CString graphicsApi = GUI::CWindowManagerBackend::GetRequestedGraphicsApi( params );
    m_graphicsApiNone = GUI::CWindowManagerBackend::GraphicsApiNone == graphicsApi;
    RegisterWindow( this, app );

    if ( m_graphicsApiNone )
    {
        // the renderer makes the EGL display, context and surface on the native window
        StartPulses( videoSettings );
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AndroidGLESWindowContext: Made a window without a graphics context (GraphicsApi None) of "
            << GetWidth() << " x " << GetHeight() );
        return true;
    }

    // OpenGL ES 3 on the window, 8 bits per colour with alpha and a stencil buffer (the GUI renderers clip with it)
    const EGLint attribs[] =
    {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };
    m_display = eglGetDisplay( EGL_DEFAULT_DISPLAY );
    if ( EGL_NO_DISPLAY == m_display || EGL_TRUE != eglInitialize( m_display, 0, 0 ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AndroidGLESWindowContext: EGL could not be started" );
        Shutdown();
        return false;
    }
    EGLConfig config = GUCEF_NULL;
    EGLint numConfigs = 0;
    if ( EGL_TRUE != eglChooseConfig( m_display, attribs, &config, 1, &numConfigs ) || numConfigs < 1 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AndroidGLESWindowContext: No OpenGL ES 3 configuration" );
        Shutdown();
        return false;
    }

    // EGL_NATIVE_VISUAL_ID is accepted by ANativeWindow_setBuffersGeometry()
    EGLint format = 0;
    eglGetConfigAttrib( m_display, config, EGL_NATIVE_VISUAL_ID, &format );
    ANativeWindow_setBuffersGeometry( app->window, 0, 0, format );

    const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    m_config = config;
    m_surface = eglCreateWindowSurface( m_display, config, app->window, GUCEF_NULL );
    m_context = eglCreateContext( m_display, config, EGL_NO_CONTEXT, contextAttribs );
    if ( EGL_NO_SURFACE == m_surface || EGL_NO_CONTEXT == m_context || EGL_FALSE == eglMakeCurrent( m_display, m_surface, m_surface, m_context ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AndroidGLESWindowContext: The OpenGL ES 3 context could not be made" );
        Shutdown();
        return false;
    }

    // frames are paced by the application's pulses
    eglSwapInterval( m_display, videoSettings.GetVSyncState() ? 1 : 0 );
    glViewport( 0, 0, (GLsizei) GetWidth(), (GLsizei) GetHeight() );
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

    StartPulses( videoSettings );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AndroidGLESWindowContext: Made an OpenGL ES 3 window of " << GetWidth() << " x " << GetHeight() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAndroidGLESWindowContext::ApplyVideoSettings( const GUI::CVideoSettings& videoSettings )
{GUCEF_TRACE;

    m_vsync = videoSettings.GetVSyncState();
    if ( m_graphicsApiNone )
        return GUCEF_NULL != m_app;         // the renderer applies the vertical sync to its surface
    if ( EGL_NO_DISPLAY == m_display || EGL_NO_SURFACE == m_surface )
        return false;
    return EGL_TRUE == eglSwapInterval( m_display, videoSettings.GetVSyncState() ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

void
CAndroidGLESWindowContext::OnNotify( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CORE::CPulseGenerator::PulseEvent == eventId && m_graphicsApiNone )
    {
        // the renderer draws and presents; nothing is drawn while the app has no native window
        if ( GUCEF_NULL != m_nativeWindow )
            NotifyObservers( WindowContextRedrawEvent );
        return;
    }
    if ( CORE::CPulseGenerator::PulseEvent == eventId && EGL_NO_SURFACE != m_surface )
    {
        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        glViewport( 0, 0, (GLsizei) GetWidth(), (GLsizei) GetHeight() );
        glClear( GL_COLOR_BUFFER_BIT );
        NotifyObservers( WindowContextRedrawEvent );
        eglSwapBuffers( m_display, m_surface );
    }
}

/*-------------------------------------------------------------------------*/

void
CAndroidGLESWindowContext::StartPulses( const GUI::CVideoSettings& videoSettings )
{GUCEF_TRACE;

    // frames are paced by the application's pulses
    if ( videoSettings.GetFrequency() > 0 && !m_drivesRedraws )
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        pulseGenerator->RequestPeriodicPulses( this, 1000 / videoSettings.GetFrequency() );
        SubscribeTo( pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent );
        m_drivesRedraws = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CAndroidGLESWindowContext::OnNativeWindowLost( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_nativeWindow )
        return;

    // the renderer lets go of its surface while the native window still exists
    NotifyObservers( WindowContextSurfaceLostEvent );

    if ( !m_graphicsApiNone && EGL_NO_DISPLAY != m_display && EGL_NO_SURFACE != m_surface )
    {
        // the context and what lives in it stay; only the surface on the native window goes
        eglMakeCurrent( m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
        eglDestroySurface( m_display, m_surface );
        m_surface = EGL_NO_SURFACE;
    }
    m_nativeWindow = GUCEF_NULL;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AndroidGLESWindowContext: The native window is gone, no frames are drawn until a new one comes" );
}

/*-------------------------------------------------------------------------*/

void
CAndroidGLESWindowContext::OnNativeWindowRestored( struct ANativeWindow* window )
{GUCEF_TRACE;

    if ( GUCEF_NULL == window || window == m_nativeWindow )
        return;
    if ( GUCEF_NULL != m_nativeWindow )
        OnNativeWindowLost();
    m_nativeWindow = window;

    if ( !m_graphicsApiNone && EGL_NO_DISPLAY != m_display && EGL_NO_CONTEXT != m_context && GUCEF_NULL != m_config )
    {
        EGLint format = 0;
        eglGetConfigAttrib( m_display, m_config, EGL_NATIVE_VISUAL_ID, &format );
        ::ANativeWindow_setBuffersGeometry( window, 0, 0, format );
        m_surface = eglCreateWindowSurface( m_display, m_config, window, GUCEF_NULL );
        if ( EGL_NO_SURFACE == m_surface || EGL_FALSE == eglMakeCurrent( m_display, m_surface, m_surface, m_context ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AndroidGLESWindowContext: No surface could be made on the new native window, EGL error " << eglGetError() );
            if ( EGL_NO_SURFACE != m_surface )
                eglDestroySurface( m_display, m_surface );
            m_surface = EGL_NO_SURFACE;
        }
        else
        {
            eglSwapInterval( m_display, m_vsync ? 1 : 0 );
        }
    }

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AndroidGLESWindowContext: A new native window of " << GetWidth() << " x " << GetHeight() << " came" );
    NotifyObservers( WindowContextSurfaceRestoredEvent );
    NotifyObservers( WindowContextSizeEvent );
}

/*-------------------------------------------------------------------------*/

void
CAndroidGLESWindowContext::OnNativeWindowChanged( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_nativeWindow )
        NotifyObservers( WindowContextSizeEvent );
}

/*-------------------------------------------------------------------------*/

bool
CAndroidGLESWindowContext::IsGraphicsApiNone( void ) const
{GUCEF_TRACE;

    return m_graphicsApiNone;
}

/*-------------------------------------------------------------------------*/

bool
CAndroidGLESWindowContext::HasNativeWindow( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_nativeWindow;
}

/*-------------------------------------------------------------------------*/

} /* namespace GUIDRIVERANDROIDGLES */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
