/*
 *  guidriverEGL: GUCEF window driver giving offscreen OpenGL windows through EGL, for tests, tools and headless runs
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

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/glcorearb.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "guidriverEGL_CEGLWindowContext.h"

#ifndef EGL_PLATFORM_SURFACELESS_MESA
#define EGL_PLATFORM_SURFACELESS_MESA 0x31DD
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEREGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The few GL functions the window needs for its framebuffer, loaded through EGL */
struct SGlFunctions
{
    PFNGLGENFRAMEBUFFERSPROC genFramebuffers;
    PFNGLDELETEFRAMEBUFFERSPROC deleteFramebuffers;
    PFNGLBINDFRAMEBUFFERPROC bindFramebuffer;
    PFNGLGENRENDERBUFFERSPROC genRenderbuffers;
    PFNGLDELETERENDERBUFFERSPROC deleteRenderbuffers;
    PFNGLBINDRENDERBUFFERPROC bindRenderbuffer;
    PFNGLRENDERBUFFERSTORAGEPROC renderbufferStorage;
    PFNGLFRAMEBUFFERRENDERBUFFERPROC framebufferRenderbuffer;
    PFNGLCHECKFRAMEBUFFERSTATUSPROC checkFramebufferStatus;
    PFNGLVIEWPORTPROC viewport;
    PFNGLCLEARCOLORPROC clearColor;
    PFNGLCLEARPROC clear;
    PFNGLFINISHPROC finish;
};

SGlFunctions g_gl;
bool g_glLoaded = false;
GUI::UInt32 g_nextWindowId = 1;

/*-------------------------------------------------------------------------*/

bool
LoadGlFunctions( void )
{
    if ( g_glLoaded )
        return true;

    g_gl.genFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) ::eglGetProcAddress( "glGenFramebuffers" );
    g_gl.deleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) ::eglGetProcAddress( "glDeleteFramebuffers" );
    g_gl.bindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) ::eglGetProcAddress( "glBindFramebuffer" );
    g_gl.genRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) ::eglGetProcAddress( "glGenRenderbuffers" );
    g_gl.deleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) ::eglGetProcAddress( "glDeleteRenderbuffers" );
    g_gl.bindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) ::eglGetProcAddress( "glBindRenderbuffer" );
    g_gl.renderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) ::eglGetProcAddress( "glRenderbufferStorage" );
    g_gl.framebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) ::eglGetProcAddress( "glFramebufferRenderbuffer" );
    g_gl.checkFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) ::eglGetProcAddress( "glCheckFramebufferStatus" );
    g_gl.viewport = (PFNGLVIEWPORTPROC) ::eglGetProcAddress( "glViewport" );
    g_gl.clearColor = (PFNGLCLEARCOLORPROC) ::eglGetProcAddress( "glClearColor" );
    g_gl.clear = (PFNGLCLEARPROC) ::eglGetProcAddress( "glClear" );
    g_gl.finish = (PFNGLFINISHPROC) ::eglGetProcAddress( "glFinish" );

    g_glLoaded = GUCEF_NULL != g_gl.genFramebuffers && GUCEF_NULL != g_gl.deleteFramebuffers && GUCEF_NULL != g_gl.bindFramebuffer &&
                 GUCEF_NULL != g_gl.genRenderbuffers && GUCEF_NULL != g_gl.deleteRenderbuffers && GUCEF_NULL != g_gl.bindRenderbuffer &&
                 GUCEF_NULL != g_gl.renderbufferStorage && GUCEF_NULL != g_gl.framebufferRenderbuffer &&
                 GUCEF_NULL != g_gl.checkFramebufferStatus && GUCEF_NULL != g_gl.viewport && GUCEF_NULL != g_gl.clearColor &&
                 GUCEF_NULL != g_gl.clear && GUCEF_NULL != g_gl.finish;
    return g_glLoaded;
}

/*-------------------------------------------------------------------------*/

/** A display that needs no display server when Mesa offers one, else the default display */
EGLDisplay
OpenDisplay( void )
{
    PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplay = (PFNEGLGETPLATFORMDISPLAYEXTPROC) ::eglGetProcAddress( "eglGetPlatformDisplayEXT" );
    if ( GUCEF_NULL != getPlatformDisplay )
    {
        EGLDisplay display = getPlatformDisplay( EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, GUCEF_NULL );
        if ( EGL_NO_DISPLAY != display )
        {
            EGLint major = 0, minor = 0;
            if ( EGL_TRUE == ::eglInitialize( display, &major, &minor ) )
                return display;
        }
    }

    EGLDisplay display = ::eglGetDisplay( EGL_DEFAULT_DISPLAY );
    if ( EGL_NO_DISPLAY != display )
    {
        EGLint major = 0, minor = 0;
        if ( EGL_TRUE == ::eglInitialize( display, &major, &minor ) )
            return display;
    }
    return EGL_NO_DISPLAY;
}

/*-------------------------------------------------------------------------*/

/** The pointer to eglGetProcAddress as GL_GETPROCADDRESS hands it out */
void*
GetProcAddress( const char* name )
{
    return (void*) ::eglGetProcAddress( name );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEGLWindowContext::CEGLWindowContext( void )
    : GUI::CWindowContext()
    , m_guiContext()
    , m_id( g_nextWindowId++ )
    , m_name()
    , m_width( 0 )
    , m_height( 0 )
    , m_display( GUCEF_NULL )
    , m_context( GUCEF_NULL )
    , m_surface( GUCEF_NULL )
    , m_isGles( false )
    , m_framebuffer( 0 )
    , m_colourBuffer( 0 )
    , m_depthStencilBuffer( 0 )
    , m_drivesRedraws( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEGLWindowContext::~CEGLWindowContext()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CEGLWindowContext::SetGuiContext( GUI::TGuiContextPtr& context )
{GUCEF_TRACE;

    m_guiContext = context;
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CEGLWindowContext::GetGuiContext( void )
{GUCEF_TRACE;

    return m_guiContext;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CEGLWindowContext::GetWidth( void ) const
{GUCEF_TRACE;

    return m_width;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CEGLWindowContext::GetHeight( void ) const
{GUCEF_TRACE;

    return m_height;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CEGLWindowContext::GetID( void ) const
{GUCEF_TRACE;

    return m_id;
}

/*-------------------------------------------------------------------------*/

bool
CEGLWindowContext::IsActive( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_context;
}

/*-------------------------------------------------------------------------*/

GUI::CString
CEGLWindowContext::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

GUI::CString
CEGLWindowContext::GetProperty( const GUI::CString& propertyName ) const
{GUCEF_TRACE;

    if ( "GL_FRAMEBUFFER" == propertyName )
        return CORE::UInt32ToString( m_framebuffer );
    if ( "GL_GETPROCADDRESS" == propertyName )
        return CORE::PointerToString( (const void*) &GetProcAddress );
    if ( "GL_API" == propertyName )
        return m_isGles ? "GLES" : "GL";
    if ( "GL_VERSION" == propertyName )
        return m_isGles ? "3.0" : "3.3";
    if ( "OFFSCREEN" == propertyName )
        return "true";
    return GUI::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CEGLWindowContext::Initialize( const GUI::CString& title                ,
                               const GUI::CVideoSettings& videoSettings )
{GUCEF_TRACE;

    Shutdown();
    m_name = title;

    EGLDisplay display = OpenDisplay();
    if ( EGL_NO_DISPLAY == display )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CEGLWindowContext: No EGL display could be opened" );
        return false;
    }
    m_display = display;

    // desktop OpenGL 3.3 core first, OpenGL ES 3 when the driver has no desktop OpenGL
    EGLConfig config = GUCEF_NULL;
    EGLContext context = EGL_NO_CONTEXT;
    for ( int attempt=0; attempt<2 && EGL_NO_CONTEXT == context; ++attempt )
    {
        bool gles = 1 == attempt;
        if ( EGL_TRUE != ::eglBindAPI( gles ? EGL_OPENGL_ES_API : EGL_OPENGL_API ) )
            continue;

        const EGLint configAttributes[] = { EGL_RENDERABLE_TYPE, gles ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_BIT,
                                            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
                                            EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
                                            EGL_NONE };
        EGLint configCount = 0;
        if ( EGL_TRUE != ::eglChooseConfig( display, configAttributes, &config, 1, &configCount ) || configCount < 1 )
            continue;

        const EGLint glAttributes[] = { EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 3,
                                        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT, EGL_NONE };
        const EGLint glesAttributes[] = { EGL_CONTEXT_MAJOR_VERSION, 3, EGL_NONE };
        context = ::eglCreateContext( display, config, EGL_NO_CONTEXT, gles ? glesAttributes : glAttributes );
        m_isGles = gles;
    }
    if ( EGL_NO_CONTEXT == context )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CEGLWindowContext: Neither an OpenGL 3.3 nor an OpenGL ES 3 context could be made" );
        Shutdown();
        return false;
    }
    m_context = context;

    // current without a surface when the driver allows that, else on a pbuffer of a single pixel
    if ( EGL_TRUE != ::eglMakeCurrent( display, EGL_NO_SURFACE, EGL_NO_SURFACE, context ) )
    {
        const EGLint pbufferAttributes[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
        EGLSurface surface = ::eglCreatePbufferSurface( display, config, pbufferAttributes );
        if ( EGL_NO_SURFACE == surface || EGL_TRUE != ::eglMakeCurrent( display, surface, surface, context ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CEGLWindowContext: The OpenGL context could not be made current" );
            if ( EGL_NO_SURFACE != surface )
                ::eglDestroySurface( display, surface );
            Shutdown();
            return false;
        }
        m_surface = surface;
    }

    if ( !LoadGlFunctions() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CEGLWindowContext: The OpenGL framebuffer functions are missing" );
        Shutdown();
        return false;
    }

    if ( !CreateFramebuffer( videoSettings.GetResolutionWidthInPixels(), videoSettings.GetResolutionHeightInPixels() ) )
    {
        Shutdown();
        return false;
    }

    if ( videoSettings.GetFrequency() > 0 )
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        pulseGenerator->RequestPeriodicPulses( this, 1000 / videoSettings.GetFrequency() );
        SubscribeTo( pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent );
        m_drivesRedraws = true;
    }

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "CEGLWindowContext: Made an offscreen " + CORE::CString( m_isGles ? "OpenGL ES 3" : "OpenGL 3.3 core" ) +
                      " window of " + CORE::UInt32ToString( m_width ) + " x " + CORE::UInt32ToString( m_height ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEGLWindowContext::CreateFramebuffer( GUI::UInt32 width, GUI::UInt32 height )
{GUCEF_TRACE;

    if ( 0 == width || 0 == height )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CEGLWindowContext: A window needs a size" );
        return false;
    }

    GLuint framebuffer = 0, colour = 0, depthStencil = 0;
    g_gl.genFramebuffers( 1, &framebuffer );
    g_gl.bindFramebuffer( GL_FRAMEBUFFER, framebuffer );
    g_gl.genRenderbuffers( 1, &colour );
    g_gl.bindRenderbuffer( GL_RENDERBUFFER, colour );
    g_gl.renderbufferStorage( GL_RENDERBUFFER, GL_RGBA8, (GLsizei) width, (GLsizei) height );
    g_gl.framebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colour );
    g_gl.genRenderbuffers( 1, &depthStencil );
    g_gl.bindRenderbuffer( GL_RENDERBUFFER, depthStencil );
    g_gl.renderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (GLsizei) width, (GLsizei) height );
    g_gl.framebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencil );
    g_gl.bindRenderbuffer( GL_RENDERBUFFER, 0 );

    m_framebuffer = framebuffer;
    m_colourBuffer = colour;
    m_depthStencilBuffer = depthStencil;

    if ( GL_FRAMEBUFFER_COMPLETE != g_gl.checkFramebufferStatus( GL_FRAMEBUFFER ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CEGLWindowContext: The framebuffer of the window is not complete" );
        DestroyFramebuffer();
        return false;
    }

    m_width = width;
    m_height = height;
    g_gl.viewport( 0, 0, (GLsizei) width, (GLsizei) height );
    g_gl.clearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    g_gl.clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEGLWindowContext::DestroyFramebuffer( void )
{GUCEF_TRACE;

    if ( !g_glLoaded || GUCEF_NULL == m_context )
        return;

    g_gl.bindFramebuffer( GL_FRAMEBUFFER, 0 );
    if ( 0 != m_framebuffer )
    {
        GLuint framebuffer = m_framebuffer;
        g_gl.deleteFramebuffers( 1, &framebuffer );
    }
    GLuint renderbuffers[ 2 ] = { m_colourBuffer, m_depthStencilBuffer };
    g_gl.deleteRenderbuffers( 2, renderbuffers );
    m_framebuffer = m_colourBuffer = m_depthStencilBuffer = 0;
    m_width = m_height = 0;
}

/*-------------------------------------------------------------------------*/

bool
CEGLWindowContext::Resize( GUI::UInt32 width, GUI::UInt32 height )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_context || !MakeCurrent() )
        return false;
    if ( width == m_width && height == m_height )
        return true;

    DestroyFramebuffer();
    if ( !CreateFramebuffer( width, height ) )
        return false;
    NotifyObservers( WindowContextSizeEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEGLWindowContext::MakeCurrent( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_context )
        return false;
    EGLSurface surface = GUCEF_NULL != m_surface ? (EGLSurface) m_surface : EGL_NO_SURFACE;
    return EGL_TRUE == ::eglMakeCurrent( (EGLDisplay) m_display, surface, surface, (EGLContext) m_context );
}

/*-------------------------------------------------------------------------*/

void
CEGLWindowContext::RequestRedraw( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_context )
        return;

    g_gl.bindFramebuffer( GL_FRAMEBUFFER, m_framebuffer );
    g_gl.viewport( 0, 0, (GLsizei) m_width, (GLsizei) m_height );
    g_gl.clearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    g_gl.clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    NotifyObservers( WindowContextRedrawEvent );
}

/*-------------------------------------------------------------------------*/

void
CEGLWindowContext::Shutdown( void )
{GUCEF_TRACE;

    if ( m_drivesRedraws )
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        pulseGenerator->RequestStopOfPeriodicUpdates( this );
        UnsubscribeFrom( pulseGenerator.GetPointerAlways() );
        m_drivesRedraws = false;
    }

    if ( GUCEF_NULL != m_display )
    {
        EGLDisplay display = (EGLDisplay) m_display;
        if ( GUCEF_NULL != m_context )
        {
            MakeCurrent();
            DestroyFramebuffer();
            ::eglMakeCurrent( display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
            ::eglDestroyContext( display, (EGLContext) m_context );
            m_context = GUCEF_NULL;
        }
        if ( GUCEF_NULL != m_surface )
        {
            ::eglDestroySurface( display, (EGLSurface) m_surface );
            m_surface = GUCEF_NULL;
        }
        // the display is shared with every other user of EGL in the process: it stays initialized
        m_display = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEGLWindowContext::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GUIDRIVEREGL::CEGLWindowContext";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CEGLWindowContext::OnNotify( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CORE::CPulseGenerator::PulseEvent == eventId )
        RequestRedraw();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEREGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
