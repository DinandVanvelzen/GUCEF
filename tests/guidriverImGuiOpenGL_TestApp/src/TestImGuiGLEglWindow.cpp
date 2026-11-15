/*
 *  guidriverImGuiOpenGL_TestApp: tests of the gucefGUI driver ImGui (OpenGL)
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

#include "guidriverImGuiOpenGL_TestSupport.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "guidriverImGuiOpenGL_TestEglWindow.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMGUIGLTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

UInt32 g_nextWindowId = 1000;

void*
GetProcAddress( const char* name )
{
    return (void*) ::eglGetProcAddress( name );
}

EGLDisplay
OpenDisplay( void )
{
    PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplay = (PFNEGLGETPLATFORMDISPLAYEXTPROC) ::eglGetProcAddress( "eglGetPlatformDisplayEXT" );
    if ( GUCEF_NULL != getPlatformDisplay )
    {
        EGLDisplay display = getPlatformDisplay( EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, GUCEF_NULL );
        EGLint major = 0, minor = 0;
        if ( EGL_NO_DISPLAY != display && EGL_TRUE == ::eglInitialize( display, &major, &minor ) )
            return display;
    }
    EGLDisplay display = ::eglGetDisplay( EGL_DEFAULT_DISPLAY );
    EGLint major = 0, minor = 0;
    if ( EGL_NO_DISPLAY != display && EGL_TRUE == ::eglInitialize( display, &major, &minor ) )
        return display;
    return EGL_NO_DISPLAY;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTestEglWindow::CTestEglWindow( bool gles )
    : GUI::CWindowContext()
    , m_id( g_nextWindowId++ )
    , m_gles( gles )
    , m_width( 0 )
    , m_height( 0 )
    , m_display( GUCEF_NULL )
    , m_context( GUCEF_NULL )
    , m_gl()
    , m_framebuffer( GUCEF_NULL )
{
}

CTestEglWindow::~CTestEglWindow()
{
    Shutdown();
}

bool
CTestEglWindow::Initialize( UInt32 width, UInt32 height )
{
    Shutdown();
    EGLDisplay display = OpenDisplay();
    if ( EGL_NO_DISPLAY == display )
        return false;
    if ( EGL_TRUE != ::eglBindAPI( m_gles ? EGL_OPENGL_ES_API : EGL_OPENGL_API ) )
        return false;
    const EGLint configAttributes[] = { EGL_RENDERABLE_TYPE, m_gles ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_BIT,
                                        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
                                        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
                                        EGL_NONE };
    EGLConfig config = GUCEF_NULL;
    EGLint configCount = 0;
    if ( EGL_TRUE != ::eglChooseConfig( display, configAttributes, &config, 1, &configCount ) || configCount < 1 )
        return false;
    const EGLint glAttributes[] = { EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 3,
                                    EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT, EGL_NONE };
    const EGLint glesAttributes[] = { EGL_CONTEXT_MAJOR_VERSION, 3, EGL_NONE };
    EGLContext context = ::eglCreateContext( display, config, EGL_NO_CONTEXT, m_gles ? glesAttributes : glAttributes );
    if ( EGL_NO_CONTEXT == context )
        return false;
    m_display = display;
    m_context = context;
    if ( EGL_TRUE != ::eglMakeCurrent( display, EGL_NO_SURFACE, EGL_NO_SURFACE, context ) )
    {
        Shutdown();
        return false;
    }
    if ( !m_gl.Load( &GetProcAddress ) )
    {
        Shutdown();
        return false;
    }
    m_framebuffer = GUCEF_NEW GUIDRIVERIMGUIGL::CImGuiGLFramebuffer( m_gl );
    if ( !m_framebuffer->Prepare( width, height ) )
    {
        Shutdown();
        return false;
    }
    m_width = width;
    m_height = height;
    return true;
}

void
CTestEglWindow::Shutdown( void )
{
    if ( GUCEF_NULL != m_context )
    {
        MakeCurrent();
        GUCEF_DELETE m_framebuffer;
        m_framebuffer = GUCEF_NULL;
        ::eglMakeCurrent( (EGLDisplay) m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
        ::eglDestroyContext( (EGLDisplay) m_display, (EGLContext) m_context );
    }
    m_context = GUCEF_NULL;
    m_display = GUCEF_NULL;
    m_width = m_height = 0;
}

bool
CTestEglWindow::MakeCurrent( void )
{
    if ( GUCEF_NULL == m_context )
        return false;
    ::eglBindAPI( m_gles ? EGL_OPENGL_ES_API : EGL_OPENGL_API );
    return EGL_TRUE == ::eglMakeCurrent( (EGLDisplay) m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, (EGLContext) m_context );
}

bool
CTestEglWindow::Resize( UInt32 width, UInt32 height )
{
    if ( GUCEF_NULL == m_framebuffer || !MakeCurrent() || !m_framebuffer->Prepare( width, height ) )
        return false;
    m_width = width;
    m_height = height;
    NotifyObservers( WindowContextSizeEvent );
    return true;
}

bool
CTestEglWindow::ReadPixels( IMAGE::TPixelMapPtr& pixels )
{
    if ( GUCEF_NULL == m_framebuffer || !MakeCurrent() )
        return false;
    return GUIDRIVERIMGUIGL::CImGuiGLFramebuffer::ReadFramebufferPixels( m_gl, m_framebuffer->GetFramebuffer(), m_width, m_height, pixels );
}

UInt32
CTestEglWindow::GetFramebuffer( void ) const
{
    return GUCEF_NULL != m_framebuffer ? m_framebuffer->GetFramebuffer() : 0;
}

UInt32
CTestEglWindow::TakeGlError( void )
{
    if ( GUCEF_NULL == m_context || !MakeCurrent() || GUCEF_NULL == m_gl.GetError )
        return 0;
    UInt32 first = 0;
    for ( UInt32 i=0; i<16; ++i )
    {
        const UInt32 error = m_gl.GetError();
        if ( 0 == error )
            break;
        if ( 0 == first )
            first = error;
    }
    return first;
}

const GUIDRIVERIMGUIGL::CImGuiGLFunctions&
CTestEglWindow::GetFunctions( void ) const
{
    return m_gl;
}

UInt32 CTestEglWindow::GetWidth( void ) const { return m_width; }
UInt32 CTestEglWindow::GetHeight( void ) const { return m_height; }
UInt32 CTestEglWindow::GetID( void ) const { return m_id; }
bool CTestEglWindow::IsActive( void ) const { return GUCEF_NULL != m_context; }
CString CTestEglWindow::GetName( void ) const { return "TestEglWindow"; }
void CTestEglWindow::SetGuiContext( GUI::TGuiContextPtr& context ) {}
GUI::TGuiContextPtr CTestEglWindow::GetGuiContext( void ) { return GUI::TGuiContextPtr(); }

CString
CTestEglWindow::GetProperty( const CString& propertyName ) const
{
    if ( "GL_GETPROCADDRESS" == propertyName )
        return CORE::PointerToString( (const void*) &GetProcAddress );
    if ( "GL_FRAMEBUFFER" == propertyName )
        return CORE::UInt32ToString( GetFramebuffer() );
    if ( "GL_API" == propertyName )
        return m_gles ? "GLES" : "GL";
    if ( "GL_VERSION" == propertyName )
        return m_gles ? "3.0" : "3.3";
    return CString::Empty;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMGUIGLTEST */
}; /* namespace GUCEF */

#endif /* linux ? */

/*-------------------------------------------------------------------------*/
