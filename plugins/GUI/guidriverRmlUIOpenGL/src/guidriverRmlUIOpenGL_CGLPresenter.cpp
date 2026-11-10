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
    #define GUIDRIVERRMLUIGL_SHADER_VERSION "#version 300 es\nprecision highp float;\n"
#elif defined( __ANDROID__ )
    #include <GLES3/gl32.h>
    #define GUIDRIVERRMLUIGL_SHADER_VERSION "#version 300 es\nprecision highp float;\n"
#else
    #include "RmlUi_Include_GL3.h"
    #define GUIDRIVERRMLUIGL_SHADER_VERSION "#version 330\n"
#endif

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "guidriverRmlUIOpenGL_CGLPresenter.h"

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

/** One triangle that covers the viewport, made from the vertex index: no vertex data is needed */
const char* const g_presentVertexShader =
    GUIDRIVERRMLUIGL_SHADER_VERSION
    "out vec2 v_uv;\n"
    "void main()\n"
    "{\n"
    "    vec2 position = vec2( float( ( gl_VertexID << 1 ) & 2 ), float( gl_VertexID & 2 ) );\n"
    "    v_uv = position;\n"
    "    gl_Position = vec4( position * 2.0 - 1.0, 0.0, 1.0 );\n"
    "}\n";

const char* const g_copyFragmentShader =
    GUIDRIVERRMLUIGL_SHADER_VERSION
    "in vec2 v_uv;\n"
    "uniform sampler2D u_source;\n"
    "out vec4 fragColour;\n"
    "void main()\n"
    "{\n"
    "    fragColour = texture( u_source, v_uv );\n"
    "}\n";

/*-------------------------------------------------------------------------*/

GLuint
CompileShader( GLenum type, const char* source )
{
    GLuint shader = glCreateShader( type );
    glShaderSource( shader, 1, &source, GUCEF_NULL );
    glCompileShader( shader );
    GLint status = GL_FALSE;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
    if ( GL_TRUE != status )
    {
        char log[ 1024 ];
        GLsizei length = 0;
        glGetShaderInfoLog( shader, (GLsizei) sizeof( log ) - 1, &length, log );
        log[ length ] = '\0';
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GLPresenter: A shader does not compile: " << log );
        glDeleteShader( shader );
        return 0;
    }
    return shader;
}

/*-------------------------------------------------------------------------*/

GLuint
LinkProgram( const char* vertexSource, const char* fragmentSource )
{
    GLuint vertexShader = CompileShader( GL_VERTEX_SHADER, vertexSource );
    GLuint fragmentShader = CompileShader( GL_FRAGMENT_SHADER, fragmentSource );
    if ( 0 == vertexShader || 0 == fragmentShader )
    {
        if ( 0 != vertexShader ) glDeleteShader( vertexShader );
        if ( 0 != fragmentShader ) glDeleteShader( fragmentShader );
        return 0;
    }
    GLuint program = glCreateProgram();
    glAttachShader( program, vertexShader );
    glAttachShader( program, fragmentShader );
    glLinkProgram( program );
    glDeleteShader( vertexShader );
    glDeleteShader( fragmentShader );
    GLint status = GL_FALSE;
    glGetProgramiv( program, GL_LINK_STATUS, &status );
    if ( GL_TRUE != status )
    {
        char log[ 1024 ];
        GLsizei length = 0;
        glGetProgramInfoLog( program, (GLsizei) sizeof( log ) - 1, &length, log );
        log[ length ] = '\0';
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GLPresenter: A program does not link: " << log );
        glDeleteProgram( program );
        return 0;
    }
    return program;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGLPresenter::CGLPresenter( void )
    : m_copyProgram( 0 )
    , m_sourceLocation( -1 )
    , m_vertexArray( 0 )
    , m_failed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGLPresenter::~CGLPresenter()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

void
CGLPresenter::Release( void )
{GUCEF_TRACE;

    if ( 0 != m_copyProgram )
        glDeleteProgram( m_copyProgram );
    if ( 0 != m_vertexArray )
    {
        GLuint vertexArray = m_vertexArray;
        glDeleteVertexArrays( 1, &vertexArray );
    }
    m_copyProgram = 0;
    m_vertexArray = 0;
    m_sourceLocation = -1;
}

/*-------------------------------------------------------------------------*/

bool
CGLPresenter::PrepareCopyProgram( void )
{GUCEF_TRACE;

    if ( 0 != m_copyProgram )
        return true;
    if ( m_failed )
        return false;

    m_copyProgram = LinkProgram( g_presentVertexShader, g_copyFragmentShader );
    if ( 0 == m_copyProgram )
    {
        m_failed = true;
        return false;
    }
    m_sourceLocation = glGetUniformLocation( m_copyProgram, "u_source" );

    // the core profile draws only with a vertex array bound, even without vertex data
    GLuint vertexArray = 0;
    glGenVertexArrays( 1, &vertexArray );
    m_vertexArray = vertexArray;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGLPresenter::Present( UInt32 sourceTexture       ,
                       UInt32 targetFramebuffer   ,
                       Int32 windowWidth          ,
                       Int32 windowHeight         ,
                       const GUI::CRect& area     ,
                       bool linear                ,
                       bool blend                 )
{GUCEF_TRACE;

    if ( 0 == sourceTexture || windowWidth <= 0 || windowHeight <= 0 || area.width <= 0 || area.height <= 0 || !PrepareCopyProgram() )
        return false;

    glBindFramebuffer( GL_FRAMEBUFFER, targetFramebuffer );

    // the area has its top at the top of the window, GL counts from the bottom
    glViewport( area.left, windowHeight - area.Bottom(), area.width, area.height );
    glDisable( GL_SCISSOR_TEST );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_STENCIL_TEST );
    glDisable( GL_CULL_FACE );
    if ( blend )
    {
        glEnable( GL_BLEND );
        glBlendEquation( GL_FUNC_ADD );
        glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
    }
    else
    {
        glDisable( GL_BLEND );
    }

    glUseProgram( m_copyProgram );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, sourceTexture );
    const GLint filter = linear ? GL_LINEAR : GL_NEAREST;
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter );
    glUniform1i( m_sourceLocation, 0 );
    glBindVertexArray( m_vertexArray );
    glDrawArrays( GL_TRIANGLES, 0, 3 );
    glBindVertexArray( 0 );
    glBindTexture( GL_TEXTURE_2D, 0 );
    glUseProgram( 0 );
    glDisable( GL_BLEND );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERRMLUIGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
