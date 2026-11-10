/*
 *  guidriverRmlUIOpenGL: GUI backend using RmlUi drawing with OpenGL 3.3, OpenGL ES 3 or WebGL 2
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

#if defined( RMLUI_PLATFORM_EMSCRIPTEN ) || defined( __EMSCRIPTEN__ )
    #include <GLES3/gl3.h>
#elif defined( __ANDROID__ )
    #include <GLES3/gl32.h>
#else
    #include "RmlUi_Include_GL3.h"
#endif

#include <RmlUi/Core/Core.h>
#include "RmlUi_Renderer_GL3.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#include "guidriverRmlUIOpenGL_CRmlUIOpenGLContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERRMLUIGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The present filters: shader files in the data of the plugin */
struct SPresentFilter
{
    const char* name;
    const char* file;
    bool needsFloatTargets;     /**< the passes keep values outside 0 .. 1 between them */
};

const SPresentFilter g_presentFilters[] = { { "mmpx", "mmpx.glsl", false },
                                            { "fsr", "fsr.glsl", false },
                                            { "anime4k", "Anime4K_Upscale_CNN_x2_M.glsl", true } };

const char* const DataModuleName = "guidriverRmlUIOpenGL";

CORE::CString
GetPresentFilterPath( const SPresentFilter& filter )
{
    return CORE::CString( DataModuleName ) + "/shaders/" + filter.file;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRmlUIOpenGLContext::CRmlUIOpenGLContext( GUI::CGUIDriver* driver              ,
                                          GUI::TWindowContextPtr windowContext ,
                                          RenderInterface_GL3* renderer        )
    : guidriverRmlUI::CRmlUIContext( driver, windowContext, renderer )
    , m_renderer( renderer )
    , m_presenter()
    , m_filter()
    , m_filterName()
    , m_filterFailed( false )
    , m_framebuffer( 0 )
    , m_colourBuffer( 0 )
    , m_width( 0 )
    , m_height( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRmlUIOpenGLContext::~CRmlUIOpenGLContext()
{GUCEF_TRACE;

    // RmlUi lets go of its geometry and textures through the renderer, which goes after it: first the context, then
    // what RmlUi keeps per renderer (it would otherwise use the deleted renderer when it shuts down)
    ReleaseRml();
    Rml::ReleaseRenderManagers();
    m_presenter.Release();
    m_filter.Release();
    ReleaseFramebuffer();
    delete m_renderer;
    m_renderer = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CRmlUIOpenGLContext::GetWindowFramebuffer( void ) const
{GUCEF_TRACE;

    GUI::TWindowContextPtr window = GetWindowContext();
    if ( window.IsNULL() )
        return 0;
    return CORE::StringToUInt32( window->GetProperty( "GL_FRAMEBUFFER" ), 0 );
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIOpenGLContext::PrepareFramebuffer( UInt32 width, UInt32 height )
{GUCEF_TRACE;

    if ( 0 != m_framebuffer && width == m_width && height == m_height )
        return true;
    ReleaseFramebuffer();

    // a texture, so the present pass can sample the picture
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
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIOpenGLContext: The framebuffer of the picture is not complete" );
        ReleaseFramebuffer();
        return false;
    }
    m_width = width;
    m_height = height;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRmlUIOpenGLContext::ReleaseFramebuffer( void )
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
CRmlUIOpenGLContext::RenderPicture( void )
{GUCEF_TRACE;

    // the frame has the logical resolution times the render scale
    UInt32 width = GetRenderWidth();
    UInt32 height = GetRenderHeight();
    if ( GUCEF_NULL == m_renderer || 0 == width || 0 == height || !PrepareFramebuffer( width, height ) )
        return false;

    PrepareRml();

    // the picture starts opaque black, like the software renderer's; a higher layer starts see through
    const float alpha = IsBottomLayer() ? 1.0f : 0.0f;
    glBindFramebuffer( GL_FRAMEBUFFER, m_framebuffer );
    glViewport( 0, 0, (GLsizei) width, (GLsizei) height );
    glClearColor( 0.0f, 0.0f, 0.0f, alpha );
    glClear( GL_COLOR_BUFFER_BIT );

    m_renderer->SetViewport( (int) width, (int) height );
    m_renderer->SetTargetFramebuffer( m_framebuffer );
    m_renderer->BeginFrame();
    RenderRml();
    m_renderer->EndFrame();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRmlUIOpenGLContext::Present( void )
{GUCEF_TRACE;

    GUI::TWindowContextPtr window = GetWindowContext();
    if ( window.IsNULL() || 0 == m_framebuffer )
        return;
    Int32 windowWidth = (Int32) window->GetWidth();
    Int32 windowHeight = (Int32) window->GetHeight();
    if ( windowWidth <= 0 || windowHeight <= 0 )
        return;

    // the lowest layer fills the window, black around the picture; the layers above are drawn over it
    GLuint target = GetWindowFramebuffer();
    const bool bottom = IsBottomLayer();
    glBindFramebuffer( GL_FRAMEBUFFER, target );
    if ( bottom )
    {
        glViewport( 0, 0, windowWidth, windowHeight );
        glDisable( GL_SCISSOR_TEST );
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );
    }

    // the present filter works on the picture of the lowest layer, before it is scaled to the window
    UInt32 picture = m_colourBuffer;
    GUI::CRect area = GetPresentRect();
    if ( bottom && PrepareFilter() )
    {
        UInt32 filteredWidth = 0, filteredHeight = 0;
        if ( !m_filter.Run( m_colourBuffer, m_width, m_height, (UInt32) area.width, (UInt32) area.height, GetPresentFilterParameters(),
                            picture, filteredWidth, filteredHeight ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIOpenGLContext: The present filter \"" << m_filterName << "\" does not run, the picture is shown without it" );
            m_filter.Release();
            m_filterFailed = true;
            picture = m_colourBuffer;
        }
    }

    const bool linear = GUI::PRESENTMODE_LINEAR == GetPresentMode() || GUI::PRESENTMODE_STRETCH == GetPresentMode();
    m_presenter.Present( picture, target, windowWidth, windowHeight, area, linear, !bottom );
    glBindFramebuffer( GL_FRAMEBUFFER, target );
}

/*-------------------------------------------------------------------------*/

CRmlUIOpenGLContext::TPresentFilterNames
CRmlUIOpenGLContext::GetAvailablePresentFilters( void ) const
{GUCEF_TRACE;

    TPresentFilterNames names;
    if ( !GUI::CGuiGlobal::MountModuleData( DataModuleName ) )
        return names;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( UInt32 i=0; i<sizeof( g_presentFilters ) / sizeof( g_presentFilters[ 0 ] ); ++i )
    {
        const SPresentFilter& filter = g_presentFilters[ i ];
        if ( !vfs.FileExists( GetPresentFilterPath( filter ) ) )
            continue;
        if ( filter.needsFloatTargets && !CGLHookShader::SupportsFloatTargets() )
            continue;
        names.push_back( filter.name );
    }
    return names;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIOpenGLContext::PrepareFilter( void )
{GUCEF_TRACE;

    const CORE::CString& wanted = GetPresentFilter();
    if ( wanted != m_filterName )
    {
        m_filter.Release();
        m_filterName = wanted;
        m_filterFailed = false;
        if ( wanted.IsNULLOrEmpty() )
            return false;

        const SPresentFilter* filter = GUCEF_NULL;
        for ( UInt32 i=0; i<sizeof( g_presentFilters ) / sizeof( g_presentFilters[ 0 ] ); ++i )
            if ( wanted == g_presentFilters[ i ].name )
                filter = &g_presentFilters[ i ];
        CORE::CDynamicBuffer source;
        if ( GUCEF_NULL == filter || !GUI::CGuiGlobal::MountModuleData( DataModuleName ) ||
             !VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( source, GetPresentFilterPath( *filter ) ) ||
             !m_filter.Parse( CORE::CString( static_cast< const char* >( source.GetConstBufferPtr() ), source.GetDataSize() ), wanted ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIOpenGLContext: The present filter \"" << wanted << "\" cannot be loaded" );
            m_filterFailed = true;
        }
    }
    return !m_filterName.IsNULLOrEmpty() && !m_filterFailed;
}

/*-------------------------------------------------------------------------*/

void
CRmlUIOpenGLContext::OnRender( void )
{GUCEF_TRACE;

    if ( RenderPicture() )
        Present();
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIOpenGLContext::CaptureFrame( IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    // the picture as it is now, without moving time on
    FlushChanges();
    if ( !RenderPicture() )
        return false;

    if ( pixels.IsNULL() || pixels->GetWidthInPixels() != m_width || pixels->GetHeightInPixels() != m_height ||
         IMAGE::PSF_RGBA != pixels->GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != pixels->GetPixelComponentDataType() )
    {
        pixels = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, m_width, m_height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    }

    glBindFramebuffer( GL_READ_FRAMEBUFFER, m_framebuffer );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
    size_t rowSize = (size_t) m_width * 4;
    // GL rows go up from the bottom, the pixel map's down from the top
    for ( UInt32 y=0; y<m_height; ++y )
        glReadPixels( 0, (GLint) ( m_height - 1 - y ), (GLsizei) m_width, 1, GL_RGBA, GL_UNSIGNED_BYTE, data + y * rowSize );
    glBindFramebuffer( GL_FRAMEBUFFER, GetWindowFramebuffer() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIOpenGLContext::CapturePresentedFrame( IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    GUI::TWindowContextPtr window = GetWindowContext();
    if ( window.IsNULL() )
        return false;
    const UInt32 width = window->GetWidth();
    const UInt32 height = window->GetHeight();
    if ( 0 == width || 0 == height )
        return false;

    if ( pixels.IsNULL() || pixels->GetWidthInPixels() != width || pixels->GetHeightInPixels() != height ||
         IMAGE::PSF_RGBA != pixels->GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != pixels->GetPixelComponentDataType() )
    {
        pixels = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    }

    const GLuint target = GetWindowFramebuffer();
    glBindFramebuffer( GL_READ_FRAMEBUFFER, target );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
    const size_t rowSize = (size_t) width * 4;
    // GL rows go up from the bottom, the pixel map's down from the top
    for ( UInt32 y=0; y<height; ++y )
        glReadPixels( 0, (GLint) ( height - 1 - y ), (GLsizei) width, 1, GL_RGBA, GL_UNSIGNED_BYTE, data + y * rowSize );
    glBindFramebuffer( GL_FRAMEBUFFER, target );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRmlUIOpenGLContext::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GUIDRIVERRMLUIGL::CRmlUIOpenGLContext";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERRMLUIGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
