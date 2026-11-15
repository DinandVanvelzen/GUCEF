/*
 *  guidriverMyGUIOpenGL: GUI backend using MyGUI with OpenGL, OpenGL 3 or OpenGL ES 3 / WebGL 2
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

#if defined( __EMSCRIPTEN__ ) || defined( __ANDROID__ )
    #include <GLES3/gl3.h>
#elif defined( _WIN32 )
    #include <GL/glew.h>
#else
    #ifndef GL_GLEXT_PROTOTYPES
        #define GL_GLEXT_PROTOTYPES
    #endif
    #include <GL/gl.h>
    #include <GL/glext.h>
#endif

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "guidriverMyGUIOpenGL_CGLContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUIOpenGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGLContext::CGLContext( GUI::CGUIDriver* driver              ,
                        GUI::TWindowContextPtr windowContext ,
                        IGLBackend* backend                  )
    : guidriverMyGUI::CMyGUIContext( driver, windowContext )
    , m_backend( backend )
    , m_isValid( false )
    , m_framebuffer( 0 )
    , m_colourBuffer( 0 )
    , m_width( 0 )
    , m_height( 0 )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_backend && m_backend->Initialise() )
    {
        m_isValid = StartGui();
        if ( !m_isValid )
            m_backend->Shutdown();
    }
}

/*-------------------------------------------------------------------------*/

CGLContext::~CGLContext()
{GUCEF_TRACE;

    // MyGUI lets go of its textures and buffers through the render manager, which goes after it
    ReleaseGui();
    ReleaseFramebuffer();
    delete m_backend;
    m_backend = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CGLContext::IsValid( void ) const
{GUCEF_TRACE;

    return m_isValid;
}

/*-------------------------------------------------------------------------*/

const char*
CGLContext::GetBackendName( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_backend ? m_backend->GetName() : "";
}

/*-------------------------------------------------------------------------*/

bool
CGLContext::IsBgraTextureMemory( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_backend && m_backend->IsBgraMemoryOrder();
}

/*-------------------------------------------------------------------------*/

bool
CGLContext::SupportsMsdfFonts( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_backend && m_backend->SupportsMsdfFonts();
}

/*-------------------------------------------------------------------------*/

bool
CGLContext::RegisterMsdfShader( void )
{GUCEF_TRACE;

    return GUCEF_NULL != m_backend && m_backend->RegisterMsdfShader( MsdfShaderName );
}

/*-------------------------------------------------------------------------*/

UInt32
CGLContext::GetWindowFramebuffer( void ) const
{GUCEF_TRACE;

    GUI::TWindowContextPtr window = GetWindowContext();
    if ( window.IsNULL() )
        return 0;
    return CORE::StringToUInt32( window->GetProperty( "GL_FRAMEBUFFER" ), 0 );
}

/*-------------------------------------------------------------------------*/

bool
CGLContext::PrepareFramebuffer( UInt32 width, UInt32 height )
{GUCEF_TRACE;

    if ( 0 != m_framebuffer && width == m_width && height == m_height )
        return true;
    ReleaseFramebuffer();

    GLuint framebuffer = 0, colour = 0;
    glGenFramebuffers( 1, &framebuffer );
    glBindFramebuffer( GL_FRAMEBUFFER, framebuffer );
    glGenTextures( 1, &colour );
    glBindTexture( GL_TEXTURE_2D, colour );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei) width, (GLsizei) height, 0, GL_RGBA, GL_UNSIGNED_BYTE, GUCEF_NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colour, 0 );
    glBindTexture( GL_TEXTURE_2D, 0 );
    m_framebuffer = framebuffer;
    m_colourBuffer = colour;

    if ( GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus( GL_FRAMEBUFFER ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIOpenGLContext: The framebuffer of the picture is not complete" );
        ReleaseFramebuffer();
        return false;
    }
    m_width = width;
    m_height = height;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGLContext::ReleaseFramebuffer( void )
{GUCEF_TRACE;

    if ( 0 != m_framebuffer )
    {
        GLuint framebuffer = m_framebuffer;
        glDeleteFramebuffers( 1, &framebuffer );
    }
    if ( 0 != m_colourBuffer )
    {
        GLuint colour = m_colourBuffer;
        glDeleteTextures( 1, &colour );
    }
    m_framebuffer = m_colourBuffer = 0;
    m_width = m_height = 0;
}

/*-------------------------------------------------------------------------*/

bool
CGLContext::RenderPicture( void )
{GUCEF_TRACE;

    const UInt32 width = GetRenderWidth();
    const UInt32 height = GetRenderHeight();
    if ( !m_isValid || 0 == width || 0 == height || !PrepareFramebuffer( width, height ) )
        return false;

    PrepareFrame();

    // the picture starts opaque black, like the software renderer's
    glBindFramebuffer( GL_FRAMEBUFFER, m_framebuffer );
    glViewport( 0, 0, (GLsizei) width, (GLsizei) height );
    glDisable( GL_SCISSOR_TEST );
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT );

    m_backend->DrawOneFrame();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGLContext::Present( void )
{GUCEF_TRACE;

    GUI::TWindowContextPtr window = GetWindowContext();
    if ( window.IsNULL() || 0 == m_framebuffer )
        return;
    const Int32 windowWidth = (Int32) window->GetWidth();
    const Int32 windowHeight = (Int32) window->GetHeight();
    if ( windowWidth <= 0 || windowHeight <= 0 )
        return;

    // the picture fills the area the present mode says, black around it
    const GLuint target = GetWindowFramebuffer();
    glBindFramebuffer( GL_DRAW_FRAMEBUFFER, target );
    glViewport( 0, 0, windowWidth, windowHeight );
    glDisable( GL_SCISSOR_TEST );
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT );

    const GUI::CRect area = GetPresentRect();
    const bool linear = GUI::PRESENTMODE_LINEAR == GetPresentMode() || GUI::PRESENTMODE_STRETCH == GetPresentMode();
    glBindFramebuffer( GL_READ_FRAMEBUFFER, m_framebuffer );
    // the picture is stored the way GL draws it, the window has its origin at the same corner
    glBlitFramebuffer( 0, 0, (GLint) m_width, (GLint) m_height,
                       area.left, windowHeight - area.top - area.height, area.left + area.width, windowHeight - area.top,
                       GL_COLOR_BUFFER_BIT, linear ? GL_LINEAR : GL_NEAREST );
    glBindFramebuffer( GL_FRAMEBUFFER, target );
}

/*-------------------------------------------------------------------------*/

void
CGLContext::OnRender( void )
{GUCEF_TRACE;

    if ( RenderPicture() )
        Present();
}

/*-------------------------------------------------------------------------*/

namespace {

bool
ReadFramebuffer( GLuint framebuffer, UInt32 width, UInt32 height, IMAGE::TPixelMapPtr& pixels )
{
    if ( pixels.IsNULL() || pixels->GetWidthInPixels() != width || pixels->GetHeightInPixels() != height ||
         IMAGE::PSF_RGBA != pixels->GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != pixels->GetPixelComponentDataType() )
    {
        pixels = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    }

    glBindFramebuffer( GL_READ_FRAMEBUFFER, framebuffer );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
    const size_t rowSize = (size_t) width * 4;
    // GL rows go up from the bottom, the pixel map's down from the top
    for ( UInt32 y=0; y<height; ++y )
        glReadPixels( 0, (GLint) ( height - 1 - y ), (GLsizei) width, 1, GL_RGBA, GL_UNSIGNED_BYTE, data + y * rowSize );
    return true;
}

}

/*-------------------------------------------------------------------------*/

bool
CGLContext::CaptureFrame( IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    // the picture as it is now
    FlushChanges();
    if ( !RenderPicture() )
        return false;
    ReadFramebuffer( m_framebuffer, m_width, m_height, pixels );
    glBindFramebuffer( GL_FRAMEBUFFER, GetWindowFramebuffer() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGLContext::CapturePresentedFrame( IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    GUI::TWindowContextPtr window = GetWindowContext();
    if ( window.IsNULL() )
        return false;
    const UInt32 width = window->GetWidth();
    const UInt32 height = window->GetHeight();
    if ( 0 == width || 0 == height )
        return false;
    const GLuint target = GetWindowFramebuffer();
    ReadFramebuffer( target, width, height, pixels );
    glBindFramebuffer( GL_FRAMEBUFFER, target );
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CGLContext::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::guidriverMyGUIOpenGL::CGLContext";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUIOpenGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
