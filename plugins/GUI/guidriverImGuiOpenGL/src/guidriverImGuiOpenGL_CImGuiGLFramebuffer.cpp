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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "guidriverImGuiOpenGL_CImGuiGLFramebuffer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUIGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImGuiGLFramebuffer::CImGuiGLFramebuffer( const CImGuiGLFunctions& functions )
    : m_gl( functions )
    , m_framebuffer( 0 )
    , m_colourTexture( 0 )
    , m_width( 0 )
    , m_height( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImGuiGLFramebuffer::~CImGuiGLFramebuffer()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

bool
CImGuiGLFramebuffer::Prepare( UInt32 width, UInt32 height )
{GUCEF_TRACE;

    if ( 0 == width || 0 == height || !m_gl.IsComplete() )
        return false;
    if ( 0 != m_framebuffer && width == m_width && height == m_height )
        return true;
    Release();

    // the framebuffer binding of the application is not ours to change for good
    int previousFramebuffer = 0;
    m_gl.GetIntegerv( CImGuiGLFunctions::GLC_FRAMEBUFFER_BINDING, &previousFramebuffer );

    unsigned int framebuffer = 0, colour = 0;
    m_gl.GenFramebuffers( 1, &framebuffer );
    m_gl.BindFramebuffer( CImGuiGLFunctions::GLC_FRAMEBUFFER, framebuffer );
    m_gl.GenTextures( 1, &colour );
    m_gl.BindTexture( CImGuiGLFunctions::GLC_TEXTURE_2D, colour );
    m_gl.TexImage2D( CImGuiGLFunctions::GLC_TEXTURE_2D, 0, CImGuiGLFunctions::GLC_RGBA8, (int) width, (int) height, 0,
                     CImGuiGLFunctions::GLC_RGBA, CImGuiGLFunctions::GLC_UNSIGNED_BYTE, GUCEF_NULL );
    m_gl.TexParameteri( CImGuiGLFunctions::GLC_TEXTURE_2D, CImGuiGLFunctions::GLC_TEXTURE_MIN_FILTER, CImGuiGLFunctions::GLC_NEAREST );
    m_gl.TexParameteri( CImGuiGLFunctions::GLC_TEXTURE_2D, CImGuiGLFunctions::GLC_TEXTURE_MAG_FILTER, CImGuiGLFunctions::GLC_NEAREST );
    m_gl.TexParameteri( CImGuiGLFunctions::GLC_TEXTURE_2D, CImGuiGLFunctions::GLC_TEXTURE_WRAP_S, CImGuiGLFunctions::GLC_CLAMP_TO_EDGE );
    m_gl.TexParameteri( CImGuiGLFunctions::GLC_TEXTURE_2D, CImGuiGLFunctions::GLC_TEXTURE_WRAP_T, CImGuiGLFunctions::GLC_CLAMP_TO_EDGE );
    m_gl.FramebufferTexture2D( CImGuiGLFunctions::GLC_FRAMEBUFFER, CImGuiGLFunctions::GLC_COLOR_ATTACHMENT0, CImGuiGLFunctions::GLC_TEXTURE_2D, colour, 0 );
    m_gl.BindTexture( CImGuiGLFunctions::GLC_TEXTURE_2D, 0 );
    m_framebuffer = framebuffer;
    m_colourTexture = colour;

    const bool complete = CImGuiGLFunctions::GLC_FRAMEBUFFER_COMPLETE == m_gl.CheckFramebufferStatus( CImGuiGLFunctions::GLC_FRAMEBUFFER );
    m_gl.BindFramebuffer( CImGuiGLFunctions::GLC_FRAMEBUFFER, (unsigned int) previousFramebuffer );
    if ( !complete )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiGLFramebuffer: The framebuffer of " << width << " x " << height << " pixels is not complete" );
        Release();
        return false;
    }
    m_width = width;
    m_height = height;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CImGuiGLFramebuffer::Release( void )
{GUCEF_TRACE;

    if ( m_gl.IsComplete() )
    {
        if ( 0 != m_framebuffer )
        {
            unsigned int framebuffer = m_framebuffer;
            m_gl.DeleteFramebuffers( 1, &framebuffer );
        }
        if ( 0 != m_colourTexture )
        {
            unsigned int colour = m_colourTexture;
            m_gl.DeleteTextures( 1, &colour );
        }
    }
    m_framebuffer = m_colourTexture = 0;
    m_width = m_height = 0;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiGLFramebuffer::IsReady( void ) const
{GUCEF_TRACE;

    return 0 != m_framebuffer;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiGLFramebuffer::GetWidth( void ) const
{GUCEF_TRACE;

    return m_width;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiGLFramebuffer::GetHeight( void ) const
{GUCEF_TRACE;

    return m_height;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiGLFramebuffer::GetFramebuffer( void ) const
{GUCEF_TRACE;

    return m_framebuffer;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiGLFramebuffer::GetColourTexture( void ) const
{GUCEF_TRACE;

    return m_colourTexture;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiGLFramebuffer::Bind( void )
{GUCEF_TRACE;

    if ( 0 == m_framebuffer )
        return false;
    m_gl.BindFramebuffer( CImGuiGLFunctions::GLC_FRAMEBUFFER, m_framebuffer );
    m_gl.Viewport( 0, 0, (int) m_width, (int) m_height );
    // a scissor of the application would cut the capture
    m_gl.Disable( CImGuiGLFunctions::GLC_SCISSOR_TEST );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CImGuiGLFramebuffer::Clear( float red, float green, float blue, float alpha )
{GUCEF_TRACE;

    m_gl.ClearColor( red, green, blue, alpha );
    m_gl.Clear( CImGuiGLFunctions::GLC_COLOR_BUFFER_BIT );
}

/*-------------------------------------------------------------------------*/

bool
CImGuiGLFramebuffer::ReadPixels( IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    if ( 0 == m_framebuffer )
        return false;
    return ReadFramebufferPixels( m_gl, m_framebuffer, m_width, m_height, pixels );
}

/*-------------------------------------------------------------------------*/

bool
CImGuiGLFramebuffer::ReadFramebufferPixels( const CImGuiGLFunctions& functions ,
                                            UInt32 framebuffer                 ,
                                            UInt32 width                       ,
                                            UInt32 height                      ,
                                            IMAGE::TPixelMapPtr& pixels        )
{GUCEF_TRACE;

    if ( 0 == width || 0 == height || !functions.IsComplete() )
        return false;
    if ( pixels.IsNULL() || pixels->GetWidthInPixels() != width || pixels->GetHeightInPixels() != height ||
         IMAGE::PSF_RGBA != pixels->GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != pixels->GetPixelComponentDataType() )
    {
        pixels = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    }

    int previousFramebuffer = 0;
    functions.GetIntegerv( CImGuiGLFunctions::GLC_READ_FRAMEBUFFER_BINDING, &previousFramebuffer );
    functions.BindFramebuffer( CImGuiGLFunctions::GLC_READ_FRAMEBUFFER, framebuffer );
    functions.PixelStorei( CImGuiGLFunctions::GLC_PACK_ALIGNMENT, 1 );
    UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
    functions.ReadPixels( 0, 0, (int) width, (int) height, CImGuiGLFunctions::GLC_RGBA, CImGuiGLFunctions::GLC_UNSIGNED_BYTE, data );
    functions.BindFramebuffer( CImGuiGLFunctions::GLC_READ_FRAMEBUFFER, (unsigned int) previousFramebuffer );

    // GL rows go up from the bottom, the pixel map's down from the top
    const size_t rowSize = (size_t) width * 4;
    for ( UInt32 y=0; y<height/2; ++y )
    {
        UInt8* top = data + y * rowSize;
        UInt8* bottom = data + ( height - 1 - y ) * rowSize;
        for ( size_t x=0; x<rowSize; ++x )
        {
            const UInt8 value = top[ x ];
            top[ x ] = bottom[ x ];
            bottom[ x ] = value;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUIGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
