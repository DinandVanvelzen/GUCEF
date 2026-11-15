/*
 *  guidriverImGuiOpenGL: gucefGUI driver ImGui: Dear ImGui drawn with OpenGL
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

#include "imgui.h"
#include "imgui_impl_opengl3.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUICONTEXTSCOPE_H
#include "guidriverImGui_CImGuiContextScope.h"
#define GUIDRIVERIMGUI_CIMGUICONTEXTSCOPE_H
#endif /* GUIDRIVERIMGUI_CIMGUICONTEXTSCOPE_H ? */

#include "guidriverImGuiOpenGL_CImGuiOpenGLContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUIGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** "3.3" as 3 and 3, "1.x" as 1 and 0, "" as 0 and 0 */
void
ParseVersion( const CORE::CString& version, Int32& major, Int32& minor )
{
    major = minor = 0;
    const char* text = version.C_String();
    if ( GUCEF_NULL == text )
        return;
    while ( *text >= '0' && *text <= '9' )
        major = major * 10 + ( *text++ - '0' );
    if ( '.' == *text )
    {
        ++text;
        while ( *text >= '0' && *text <= '9' )
            minor = minor * 10 + ( *text++ - '0' );
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CImGuiOpenGLContext::SelectGlslVersion( const CORE::CString& api     ,
                                        const CORE::CString& version ,
                                        bool openGlEsBuild           ,
                                        CORE::CString& glslVersion   ,
                                        CORE::CString& reason        )
{GUCEF_TRACE;

    glslVersion.Clear();
    reason.Clear();
    Int32 major = 0, minor = 0;
    ParseVersion( version, major, minor );

    if ( "GLES" == api )
    {
        if ( major < 3 )
        {
            reason = "The OpenGL ES version of the window is \"" + version + "\", Dear ImGui is drawn with OpenGL ES 3.0 or later";
            return false;
        }
        glslVersion = "#version 300 es";
        return true;
    }
    if ( "GL" == api )
    {
        if ( openGlEsBuild )
        {
            reason = "The window has desktop OpenGL, this build draws with OpenGL ES";
            return false;
        }
        if ( major < 3 || ( 3 == major && minor < 3 ) )
        {
            reason = "The OpenGL version of the window is \"" + version + "\", Dear ImGui is drawn with a core profile of OpenGL 3.3 or later";
            return false;
        }
        glslVersion = "#version 330 core";
        return true;
    }
    reason = "The window reports the graphics API \"" + api + "\", Dear ImGui is drawn with OpenGL (GL) or OpenGL ES (GLES)";
    return false;
}

/*-------------------------------------------------------------------------*/

CImGuiOpenGLContext::CImGuiOpenGLContext( GUIDRIVERIMGUI::CImGuiDriver* driver                 ,
                                          GUI::TWindowContextPtr windowContext                 ,
                                          const CORE::CString& glslVersion                     ,
                                          const CImGuiGLFunctions& functions                   ,
                                          CImGuiGLFunctions::TGetProcAddress getProcAddress    )
    : GUIDRIVERIMGUI::CImGuiContext( driver, windowContext )
    , m_gl( functions )
    , m_getProcAddress( getProcAddress )
    , m_glslVersion( glslVersion )
    , m_framebuffer( m_gl )
    , m_drawnFrames( 0 )
    , m_backendReady( false )
{GUCEF_TRACE;

    if ( !IsReady() )
        return;

    // the backend keeps its data in the ImGui context: set it up with that one current
    GUIDRIVERIMGUI::CImGuiContextScope scope( GetImGuiContext() );
    if ( !CImGuiGLFunctions::EnsureBackendLoader( m_getProcAddress ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOpenGLContext: The OpenGL entry points of the backend cannot be loaded" );
        return;
    }
    if ( !ImGui_ImplOpenGL3_Init( m_glslVersion.C_String() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOpenGLContext: The OpenGL backend of Dear ImGui does not start" );
        return;
    }

    // the shaders now, so that a failure refuses the context instead of the first frame
    if ( !ImGui_ImplOpenGL3_CreateDeviceObjects() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOpenGLContext: The shaders of the OpenGL backend do not compile with \"" << m_glslVersion << "\"" );
        ImGui_ImplOpenGL3_Shutdown();
        return;
    }

    // the backend set its own draw callbacks (the retained painter picks them up from the platform IO) and the texture protocol
    ImGui::GetIO().BackendRendererName = "gucefGUI OpenGL";
    m_backendReady = true;
}

/*-------------------------------------------------------------------------*/

CImGuiOpenGLContext::~CImGuiOpenGLContext()
{GUCEF_TRACE;

    // widgets first, then the backend and its textures while the GL context is there, the base releases the rest
    DestroyAllWidgets();
    ShutdownRenderer();
}

/*-------------------------------------------------------------------------*/

bool
CImGuiOpenGLContext::IsRendererReady( void ) const
{GUCEF_TRACE;

    return m_backendReady;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CImGuiOpenGLContext::GetGlslVersion( void ) const
{GUCEF_TRACE;

    return m_glslVersion;
}

/*-------------------------------------------------------------------------*/

ImTextureRef
CImGuiOpenGLContext::GetTextureRef( UInt32 glTexture ) const
{GUCEF_TRACE;

    return ImTextureRef( (ImTextureID) glTexture );
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiOpenGLContext::GetDrawnFrameCount( void ) const
{GUCEF_TRACE;

    return m_drawnFrames;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiOpenGLContext::GetWindowFramebuffer( void ) const
{GUCEF_TRACE;

    GUI::TWindowContextPtr window = GetWindowContext();
    if ( window.IsNULL() )
        return 0;
    return CORE::StringToUInt32( window->GetProperty( "GL_FRAMEBUFFER" ), 0 );
}

/*-------------------------------------------------------------------------*/

bool
CImGuiOpenGLContext::OnRendererNewFrame( void )
{GUCEF_TRACE;

    if ( !m_backendReady || !CImGuiGLFunctions::EnsureBackendLoader( m_getProcAddress ) )
        return false;
    ImGui_ImplOpenGL3_NewFrame();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiOpenGLContext::RenderDrawData( ImDrawData& drawData )
{GUCEF_TRACE;

    GUI::TWindowContextPtr window = GetWindowContext();
    if ( !m_backendReady || window.IsNULL() || !CImGuiGLFunctions::EnsureBackendLoader( m_getProcAddress ) )
        return false;
    const Int32 windowWidth = (Int32) window->GetWidth();
    const Int32 windowHeight = (Int32) window->GetHeight();
    if ( windowWidth <= 0 || windowHeight <= 0 )
        return false;

    // the lowest layer fills the window, black around the picture; the layers above are drawn over it
    const unsigned int target = GetWindowFramebuffer();
    m_gl.BindFramebuffer( CImGuiGLFunctions::GLC_FRAMEBUFFER, target );
    if ( IsBottomLayer() )
    {
        m_gl.Viewport( 0, 0, windowWidth, windowHeight );
        m_gl.Disable( CImGuiGLFunctions::GLC_SCISSOR_TEST );
        m_gl.ClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
        m_gl.Clear( CImGuiGLFunctions::GLC_COLOR_BUFFER_BIT );
    }

    // the logical picture in the present rectangle of the window; the backend takes viewport, projection and scissor from it
    SetDrawDataMapping( drawData, GetPresentMapping() );
    ImGui_ImplOpenGL3_RenderDrawData( &drawData );
    m_gl.BindFramebuffer( CImGuiGLFunctions::GLC_FRAMEBUFFER, target );
    ++m_drawnFrames;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiOpenGLContext::RenderDrawDataOffscreen( ImDrawData& drawData, UInt32 width, UInt32 height, IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    if ( !m_backendReady || 0 == width || 0 == height || !CImGuiGLFunctions::EnsureBackendLoader( m_getProcAddress ) )
        return false;
    if ( !m_framebuffer.Prepare( width, height ) )
        return false;

    int previousFramebuffer = 0;
    int previousViewport[ 4 ] = { 0, 0, 0, 0 };
    m_gl.GetIntegerv( CImGuiGLFunctions::GLC_FRAMEBUFFER_BINDING, &previousFramebuffer );
    m_gl.GetIntegerv( CImGuiGLFunctions::GLC_VIEWPORT, previousViewport );

    // again what the last frame drew, on opaque black; the texture requests were served when it was drawn
    m_framebuffer.Bind();
    m_framebuffer.Clear( 0.0f, 0.0f, 0.0f, 1.0f );
    ImVector< ImTextureData* >* textures = drawData.Textures;
    drawData.Textures = GUCEF_NULL;
    ImGui_ImplOpenGL3_RenderDrawData( &drawData );
    drawData.Textures = textures;
    const bool read = m_framebuffer.ReadPixels( pixels );

    m_gl.BindFramebuffer( CImGuiGLFunctions::GLC_FRAMEBUFFER, (unsigned int) previousFramebuffer );
    m_gl.Viewport( previousViewport[ 0 ], previousViewport[ 1 ], previousViewport[ 2 ], previousViewport[ 3 ] );
    return read;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiOpenGLContext::CapturePresentedFrame( IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    GUI::TWindowContextPtr window = GetWindowContext();
    if ( !m_backendReady || window.IsNULL() )
        return false;
    return CImGuiGLFramebuffer::ReadFramebufferPixels( m_gl, GetWindowFramebuffer(), window->GetWidth(), window->GetHeight(), pixels );
}

/*-------------------------------------------------------------------------*/

void
CImGuiOpenGLContext::ReleaseRenderer( void )
{GUCEF_TRACE;

    if ( !m_backendReady )
        return;
    m_backendReady = false;
    CImGuiGLFunctions::EnsureBackendLoader( m_getProcAddress );

    // every GPU texture of this context goes, those of the texture cache and the font atlas included: the backend
    // destroys a texture that is marked for it and not used for a frame
    ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
    for ( int i=0; i<platformIO.Textures.Size; ++i )
    {
        ImTextureData* texture = platformIO.Textures[ i ];
        if ( GUCEF_NULL == texture || ImTextureID_Invalid == texture->TexID )
            continue;
        texture->SetStatus( ImTextureStatus_WantDestroy );
        texture->UnusedFrames = 1;
        ImGui_ImplOpenGL3_UpdateTexture( texture );
    }
    m_framebuffer.Release();
    ImGui_ImplOpenGL3_Shutdown();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CImGuiOpenGLContext::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GUIDRIVERIMGUIGL::CImGuiOpenGLContext";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUIGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
