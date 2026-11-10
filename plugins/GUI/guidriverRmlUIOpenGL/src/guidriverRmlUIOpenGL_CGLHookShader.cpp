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
    #define GUIDRIVERRMLUIGL_SHADER_VERSION "#version 300 es\nprecision highp float;\nprecision highp sampler2D;\n"
    #define GUIDRIVERRMLUIGL_GLES
#elif defined( __ANDROID__ )
    #include <GLES3/gl32.h>
    #define GUIDRIVERRMLUIGL_SHADER_VERSION "#version 300 es\nprecision highp float;\nprecision highp sampler2D;\n"
    #define GUIDRIVERRMLUIGL_GLES
#else
    #include "RmlUi_Include_GL3.h"
    #define GUIDRIVERRMLUIGL_SHADER_VERSION "#version 330\n"
#endif

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "guidriverRmlUIOpenGL_CGLHookShader.h"

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

/** One triangle that covers the viewport, made from the vertex index */
const char* const g_hookVertexShader =
    GUIDRIVERRMLUIGL_SHADER_VERSION
    "out vec2 v_uv;\n"
    "void main()\n"
    "{\n"
    "    vec2 position = vec2( float( ( gl_VertexID << 1 ) & 2 ), float( gl_VertexID & 2 ) );\n"
    "    v_uv = position;\n"
    "    gl_Position = vec4( position * 2.0 - 1.0, 0.0, 1.0 );\n"
    "}\n";

CORE::CString
TrimBoth( const CORE::CString& text )
{
    return text.Trim( true ).Trim( false );
}

/*-------------------------------------------------------------------------*/

GLuint
CompileShader( GLenum type, const CORE::CString& source, const CORE::CString& what )
{
    GLuint shader = glCreateShader( type );
    const char* text = source.C_String();
    glShaderSource( shader, 1, &text, GUCEF_NULL );
    glCompileShader( shader );
    GLint status = GL_FALSE;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
    if ( GL_TRUE != status )
    {
        char log[ 2048 ];
        GLsizei length = 0;
        glGetShaderInfoLog( shader, (GLsizei) sizeof( log ) - 1, &length, log );
        log[ length ] = '\0';
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GLHookShader: " << what << " does not compile: " << log );
        glDeleteShader( shader );
        return 0;
    }
    return shader;
}

/*-------------------------------------------------------------------------*/

GLuint
LinkProgram( const CORE::CString& fragmentSource, const CORE::CString& what )
{
    GLuint vertexShader = CompileShader( GL_VERTEX_SHADER, g_hookVertexShader, what );
    GLuint fragmentShader = CompileShader( GL_FRAGMENT_SHADER, fragmentSource, what );
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
        char log[ 2048 ];
        GLsizei length = 0;
        glGetProgramInfoLog( program, (GLsizei) sizeof( log ) - 1, &length, log );
        log[ length ] = '\0';
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GLHookShader: " << what << " does not link: " << log );
        glDeleteProgram( program );
        return 0;
    }
    return program;
}

/*-------------------------------------------------------------------------*/

/** The names a pass uses for its textures: the binds, with HOOKED for the hooked texture */
const char* const HookedName = "HOOKED";
const char* const MainName = "MAIN";

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGLHookShader::CGLHookShader( void )
    : m_name()
    , m_passes()
    , m_parameters()
    , m_targets()
    , m_vertexArray( 0 )
    , m_floatTargets( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGLHookShader::~CGLHookShader()
{GUCEF_TRACE;

    // GL objects that are left are the GL context's; without the context being current they cannot go here
}

/*-------------------------------------------------------------------------*/

bool
CGLHookShader::Parse( const CORE::CString& source, const CORE::CString& name )
{GUCEF_TRACE;

    m_name = name;
    m_passes.clear();
    m_parameters.clear();

    enum EBlock { BLOCK_NONE, BLOCK_PASS, BLOCK_PARAMETER, BLOCK_IGNORED };
    EBlock block = BLOCK_NONE;
    bool inDirectives = false;
    bool parameterHasValue = false;
    TPass pass;
    TParameter parameter;

    CORE::CString::StringVector lines = source.ReplaceChar( '\r', ' ' ).ParseElements( '\n', true );
    for ( UInt32 i=0; i<=(UInt32)lines.size(); ++i )
    {
        bool atEnd = i == (UInt32)lines.size();
        CORE::CString line = atEnd ? CORE::CString::Empty : lines[ i ];
        CORE::CString trimmed = TrimBoth( line );
        bool directive = !atEnd && trimmed.StartsWith( "//!" );

        // a block of directives after code, or the end, closes the block before it
        if ( atEnd || ( directive && !inDirectives ) )
        {
            if ( BLOCK_PASS == block )
            {
                if ( pass.hook == MainName )
                    m_passes.push_back( pass );
                else
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "GLHookShader: " << name << ": the pass \"" << pass.description << "\" works on " << pass.hook << ", only passes on MAIN are run" );
            }
            else
            if ( BLOCK_PARAMETER == block && parameterHasValue )
            {
                m_parameters.push_back( parameter );
            }
            block = BLOCK_NONE;
            if ( atEnd )
                break;
        }

        if ( directive )
        {
            inDirectives = true;
            CORE::CString content = TrimBoth( trimmed.SubstrFromRange( 3, trimmed.Length() ) );
            Int32 space = content.HasChar( ' ' );
            CORE::CString key = ( space < 0 ? content : content.SubstrFromRange( 0, (UInt32) space ) ).Uppercase();
            CORE::CString value = space < 0 ? CORE::CString::Empty : TrimBoth( content.SubstrFromRange( (UInt32) space + 1, content.Length() ) );

            if ( BLOCK_NONE == block )
            {
                if ( "PARAM" == key )
                {
                    block = BLOCK_PARAMETER;
                    parameter = TParameter();
                    parameter.name = value;
                    parameter.isInteger = false;
                    parameter.defaultValue = 0.0f;
                    parameter.minimum = -1.0e30f;
                    parameter.maximum = 1.0e30f;
                    parameterHasValue = false;
                }
                else
                if ( "TEXTURE" == key || "BUFFER" == key )
                {
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "GLHookShader: " << name << ": //!" << key << " blocks are not supported" );
                    block = BLOCK_IGNORED;
                }
                else
                {
                    block = BLOCK_PASS;
                    pass = TPass();
                    pass.program = 0;
                }
            }

            if ( BLOCK_PASS == block )
            {
                if ( "DESC" == key ) pass.description = value;
                else if ( "HOOK" == key ) { if ( pass.hook.IsNULLOrEmpty() || value == MainName ) pass.hook = value; }
                else if ( "BIND" == key ) pass.binds.push_back( value );
                else if ( "SAVE" == key ) pass.save = value;
                else if ( "WIDTH" == key ) pass.widthExpression = value;
                else if ( "HEIGHT" == key ) pass.heightExpression = value;
                else if ( "WHEN" == key ) pass.whenExpression = value;
            }
            else
            if ( BLOCK_PARAMETER == block )
            {
                if ( "TYPE" == key ) parameter.isInteger = value.HasSubstr( "int" ) >= 0;
                else if ( "MINIMUM" == key ) parameter.minimum = CORE::StringToFloat( value, parameter.minimum );
                else if ( "MAXIMUM" == key ) parameter.maximum = CORE::StringToFloat( value, parameter.maximum );
            }
            continue;
        }

        inDirectives = false;
        if ( BLOCK_PASS == block )
        {
            pass.code += line;
            pass.code += '\n';
        }
        else
        if ( BLOCK_PARAMETER == block && !parameterHasValue && !trimmed.IsNULLOrEmpty() && !trimmed.StartsWith( "//" ) )
        {
            parameter.defaultValue = CORE::StringToFloat( trimmed, 0.0f );
            parameterHasValue = true;
        }
    }

    if ( m_passes.empty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GLHookShader: " << name << " has no pass on MAIN" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGLHookShader::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

UInt32
CGLHookShader::GetPassCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_passes.size();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGLHookShader::GetPassDescription( UInt32 passIndex ) const
{GUCEF_TRACE;

    if ( passIndex >= (UInt32) m_passes.size() )
        return CORE::CString::Empty;
    return m_passes[ passIndex ].description;
}

/*-------------------------------------------------------------------------*/

bool
CGLHookShader::HasParameter( const CORE::CString& name ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_parameters.size(); ++i )
        if ( m_parameters[ i ].name == name )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGLHookShader::EvaluateExpression( const CORE::CString& expression, const TSizeMap& sizes, Float32& result )
{GUCEF_TRACE;

    GUCEF::vector< Float32 > stack;
    CORE::CString::StringVector tokens = expression.ParseElements( ' ', false );
    for ( UInt32 i=0; i<(UInt32)tokens.size(); ++i )
    {
        const CORE::CString& token = tokens[ i ];
        if ( token == "+" || token == "-" || token == "*" || token == "/" || token == ">" || token == "<" || token == "=" )
        {
            if ( stack.size() < 2 )
                return false;
            Float32 b = stack.back(); stack.pop_back();
            Float32 a = stack.back(); stack.pop_back();
            Float32 value = 0.0f;
            switch ( token[ 0 ] )
            {
                case '+': value = a + b; break;
                case '-': value = a - b; break;
                case '*': value = a * b; break;
                case '/': value = 0.0f != b ? a / b : 0.0f; break;
                case '>': value = a > b ? 1.0f : 0.0f; break;
                case '<': value = a < b ? 1.0f : 0.0f; break;
                default: value = a == b ? 1.0f : 0.0f; break;
            }
            stack.push_back( value );
            continue;
        }
        if ( token == "!" )
        {
            if ( stack.empty() )
                return false;
            stack.back() = 0.0f == stack.back() ? 1.0f : 0.0f;
            continue;
        }

        // a texture size, "NAME.w" or "NAME.width", "NAME.h" or "NAME.height"
        Int32 dot = token.HasChar( '.', false );
        if ( dot > 0 )
        {
            CORE::CString texture = token.SubstrFromRange( 0, (UInt32) dot );
            CORE::CString what = token.SubstrFromRange( (UInt32) dot + 1, token.Length() );
            if ( what == "width" ) what = "w";
            if ( what == "height" ) what = "h";
            TSizeMap::const_iterator size = sizes.find( texture + '.' + what );
            if ( size != sizes.end() )
            {
                stack.push_back( (*size).second );
                continue;
            }
            if ( what == "w" || what == "h" )
                return false;
        }

        // a number
        char first = token.Length() > 0 ? token[ 0 ] : ' ';
        if ( ( first >= '0' && first <= '9' ) || first == '.' || ( first == '-' && token.Length() > 1 ) )
        {
            stack.push_back( CORE::StringToFloat( token, 0.0f ) );
            continue;
        }
        return false;
    }
    if ( 1 != stack.size() )
        return false;
    result = stack.back();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGLHookShader::SupportsFloatTargets( void )
{GUCEF_TRACE;

    static Int32 supported = -1;
    if ( supported >= 0 )
        return 1 == supported;

    // what the GL says when it is asked to render to one
    GLint previousFramebuffer = 0;
    glGetIntegerv( GL_FRAMEBUFFER_BINDING, &previousFramebuffer );
    GLuint texture = 0, framebuffer = 0;
    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGBA, GL_HALF_FLOAT, GUCEF_NULL );
    glGenFramebuffers( 1, &framebuffer );
    glBindFramebuffer( GL_FRAMEBUFFER, framebuffer );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0 );
    supported = GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus( GL_FRAMEBUFFER ) && GL_NO_ERROR == glGetError() ? 1 : 0;
    glBindFramebuffer( GL_FRAMEBUFFER, (GLuint) previousFramebuffer );
    glBindTexture( GL_TEXTURE_2D, 0 );
    glDeleteFramebuffers( 1, &framebuffer );
    glDeleteTextures( 1, &texture );
    while ( GL_NO_ERROR != glGetError() ) {}
    return 1 == supported;
}

/*-------------------------------------------------------------------------*/

bool
CGLHookShader::PreparePass( TPass& pass )
{GUCEF_TRACE;

    if ( 0 != pass.program )
        return true;

    // what mpv gives a user shader for every texture it binds
    CORE::CString prelude = GUIDRIVERRMLUIGL_SHADER_VERSION;
    prelude += "in vec2 v_uv;\n"
               "out vec4 fragColour;\n"
               "uniform vec2 target_size;\n"
               "uniform vec2 input_size;\n"
               "uniform int frame;\n"
               "uniform float random;\n";
    for ( UInt32 p=0; p<(UInt32)m_parameters.size(); ++p )
        prelude += CORE::CString( m_parameters[ p ].isInteger ? "uniform int " : "uniform float " ) + m_parameters[ p ].name + ";\n";

    GUCEF::vector< CORE::CString > names = pass.binds;
    names.push_back( HookedName );
    GUCEF::set< CORE::CString > declared;
    for ( UInt32 b=0; b<(UInt32)names.size(); ++b )
    {
        const CORE::CString& n = names[ b ];
        if ( declared.count( n ) > 0 )
            continue;
        declared.insert( n );
        prelude += "uniform sampler2D " + n + "_raw;\n"
                   "uniform vec2 " + n + "_size;\n"
                   "uniform vec2 " + n + "_pt;\n"
                   "#define " + n + "_pos v_uv\n"
                   "#define " + n + "_mul 1.0\n"
                   "#define " + n + "_tex( pos ) texture( " + n + "_raw, pos )\n"
                   "#define " + n + "_texOff( off ) " + n + "_tex( " + n + "_pos + " + n + "_pt * vec2( off ) )\n";
    }

    CORE::CString source = prelude + pass.code + "\nvoid main()\n{\n    fragColour = hook();\n}\n";
    pass.program = LinkProgram( source, m_name + " pass \"" + pass.description + "\"" );
    return 0 != pass.program;
}

/*-------------------------------------------------------------------------*/

Int32
CGLHookShader::GetTarget( UInt32 width, UInt32 height )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_targets.size(); ++i )
    {
        TTexture& target = m_targets[ i ];
        if ( !target.inUse && target.width == width && target.height == height )
        {
            target.inUse = true;
            return (Int32) i;
        }
    }

    TTexture target;
    target.width = width;
    target.height = height;
    target.inUse = true;
    GLuint texture = 0, framebuffer = 0;
    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );
    if ( m_floatTargets )
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, (GLsizei) width, (GLsizei) height, 0, GL_RGBA, GL_HALF_FLOAT, GUCEF_NULL );
    else
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei) width, (GLsizei) height, 0, GL_RGBA, GL_UNSIGNED_BYTE, GUCEF_NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glGenFramebuffers( 1, &framebuffer );
    glBindFramebuffer( GL_FRAMEBUFFER, framebuffer );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0 );
    glBindTexture( GL_TEXTURE_2D, 0 );
    target.texture = texture;
    target.framebuffer = framebuffer;
    if ( GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus( GL_FRAMEBUFFER ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GLHookShader: " << m_name << ": a " << width << " x " << height << " texture cannot be rendered to" );
        glDeleteFramebuffers( 1, &framebuffer );
        glDeleteTextures( 1, &texture );
        return -1;
    }
    m_targets.push_back( target );
    return (Int32) m_targets.size() - 1;
}

/*-------------------------------------------------------------------------*/

bool
CGLHookShader::Run( UInt32 sourceTexture        ,
                    UInt32 sourceWidth          ,
                    UInt32 sourceHeight         ,
                    UInt32 outputWidth          ,
                    UInt32 outputHeight         ,
                    const TParameterMap& params ,
                    UInt32& resultTexture       ,
                    UInt32& resultWidth         ,
                    UInt32& resultHeight        )
{GUCEF_TRACE;

    resultTexture = sourceTexture;
    resultWidth = sourceWidth;
    resultHeight = sourceHeight;
    if ( m_passes.empty() || 0 == sourceTexture || 0 == sourceWidth || 0 == sourceHeight )
        return false;

    if ( 0 == m_vertexArray )
    {
        m_floatTargets = SupportsFloatTargets();
        GLuint vertexArray = 0;
        glGenVertexArrays( 1, &vertexArray );
        m_vertexArray = vertexArray;
    }
    for ( UInt32 i=0; i<(UInt32)m_targets.size(); ++i )
        m_targets[ i ].inUse = false;

    // the textures by name: the picture is MAIN, what passes save is added
    struct SNamed { UInt32 texture; UInt32 width; UInt32 height; Int32 target; };
    GUCEF::map< CORE::CString, SNamed > textures;
    SNamed main = { sourceTexture, sourceWidth, sourceHeight, -1 };
    textures[ MainName ] = main;

    GLint previousFramebuffer = 0;
    glGetIntegerv( GL_FRAMEBUFFER_BINDING, &previousFramebuffer );
    glDisable( GL_BLEND );
    glDisable( GL_SCISSOR_TEST );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_STENCIL_TEST );
    glDisable( GL_CULL_FACE );

    bool success = true;
    for ( UInt32 p=0; p<(UInt32)m_passes.size() && success; ++p )
    {
        TPass& pass = m_passes[ p ];

        TSizeMap sizes;
        GUCEF::map< CORE::CString, SNamed >::iterator t = textures.begin();
        while ( t != textures.end() )
        {
            sizes[ (*t).first + ".w" ] = (Float32) (*t).second.width;
            sizes[ (*t).first + ".h" ] = (Float32) (*t).second.height;
            ++t;
        }
        const SNamed& hooked = textures[ MainName ];
        sizes[ CORE::CString( HookedName ) + ".w" ] = (Float32) hooked.width;
        sizes[ CORE::CString( HookedName ) + ".h" ] = (Float32) hooked.height;
        sizes[ "OUTPUT.w" ] = (Float32) outputWidth;
        sizes[ "OUTPUT.h" ] = (Float32) outputHeight;
        sizes[ "NATIVE.w" ] = (Float32) sourceWidth;
        sizes[ "NATIVE.h" ] = (Float32) sourceHeight;

        if ( !pass.whenExpression.IsNULLOrEmpty() )
        {
            Float32 when = 0.0f;
            if ( !EvaluateExpression( pass.whenExpression, sizes, when ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GLHookShader: " << m_name << ": cannot evaluate WHEN " << pass.whenExpression );
                success = false;
                break;
            }
            if ( 0.0f == when )
                continue;
        }

        Float32 width = (Float32) hooked.width, height = (Float32) hooked.height;
        if ( ( !pass.widthExpression.IsNULLOrEmpty() && !EvaluateExpression( pass.widthExpression, sizes, width ) ) ||
             ( !pass.heightExpression.IsNULLOrEmpty() && !EvaluateExpression( pass.heightExpression, sizes, height ) ) ||
             width < 1.0f || height < 1.0f || width > 16384.0f || height > 16384.0f )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GLHookShader: " << m_name << ": the size of pass \"" << pass.description << "\" cannot be determined" );
            success = false;
            break;
        }

        // all bound textures have to be there
        bool bindsKnown = true;
        for ( UInt32 b=0; b<(UInt32)pass.binds.size(); ++b )
            if ( pass.binds[ b ] != HookedName && textures.find( pass.binds[ b ] ) == textures.end() )
                bindsKnown = false;
        if ( !bindsKnown || !PreparePass( pass ) )
        {
            if ( !bindsKnown )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GLHookShader: " << m_name << ": pass \"" << pass.description << "\" binds a texture no pass made" );
            success = false;
            break;
        }

        Int32 targetIndex = GetTarget( (UInt32) width, (UInt32) height );
        if ( targetIndex < 0 )
        {
            success = false;
            break;
        }
        const TTexture* target = &m_targets[ targetIndex ];

        glBindFramebuffer( GL_FRAMEBUFFER, target->framebuffer );
        glViewport( 0, 0, (GLsizei) target->width, (GLsizei) target->height );
        glUseProgram( pass.program );
        glUniform2f( glGetUniformLocation( pass.program, "target_size" ), (GLfloat) target->width, (GLfloat) target->height );
        glUniform2f( glGetUniformLocation( pass.program, "input_size" ), (GLfloat) sourceWidth, (GLfloat) sourceHeight );
        glUniform1i( glGetUniformLocation( pass.program, "frame" ), 0 );
        glUniform1f( glGetUniformLocation( pass.program, "random" ), 0.0f );
        for ( UInt32 q=0; q<(UInt32)m_parameters.size(); ++q )
        {
            const TParameter& parameter = m_parameters[ q ];
            Float32 value = parameter.defaultValue;
            TParameterMap::const_iterator given = params.find( parameter.name );
            if ( given != params.end() )
                value = CORE::StringToFloat( (*given).second, value );
            if ( value < parameter.minimum ) value = parameter.minimum;
            if ( value > parameter.maximum ) value = parameter.maximum;
            GLint location = glGetUniformLocation( pass.program, parameter.name.C_String() );
            if ( parameter.isInteger )
                glUniform1i( location, (GLint) value );
            else
                glUniform1f( location, value );
        }

        GUCEF::vector< CORE::CString > names = pass.binds;
        names.push_back( HookedName );
        for ( UInt32 b=0; b<(UInt32)names.size(); ++b )
        {
            const SNamed& bound = names[ b ] == HookedName ? hooked : textures[ names[ b ] ];
            CORE::CString n = names[ b ];
            glActiveTexture( GL_TEXTURE0 + b );
            glBindTexture( GL_TEXTURE_2D, bound.texture );
            glUniform1i( glGetUniformLocation( pass.program, ( n + "_raw" ).C_String() ), (GLint) b );
            glUniform2f( glGetUniformLocation( pass.program, ( n + "_size" ).C_String() ), (GLfloat) bound.width, (GLfloat) bound.height );
            glUniform2f( glGetUniformLocation( pass.program, ( n + "_pt" ).C_String() ), 1.0f / (GLfloat) bound.width, 1.0f / (GLfloat) bound.height );
            if ( bound.texture == sourceTexture )
            {
                // the picture itself is sampled like the textures of the passes
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            }
        }

        glBindVertexArray( m_vertexArray );
        glDrawArrays( GL_TRIANGLES, 0, 3 );
        glBindVertexArray( 0 );
        for ( UInt32 b=0; b<(UInt32)names.size(); ++b )
        {
            glActiveTexture( GL_TEXTURE0 + b );
            glBindTexture( GL_TEXTURE_2D, 0 );
        }
        glActiveTexture( GL_TEXTURE0 );

        // the result takes the place of what it saves to; the texture it replaces can be used again
        CORE::CString saveTo = pass.save.IsNULLOrEmpty() || pass.save == HookedName ? CORE::CString( MainName ) : pass.save;
        GUCEF::map< CORE::CString, SNamed >::iterator replaced = textures.find( saveTo );
        if ( replaced != textures.end() && (*replaced).second.target >= 0 )
            m_targets[ (*replaced).second.target ].inUse = false;
        SNamed result = { target->texture, target->width, target->height, targetIndex };
        textures[ saveTo ] = result;
    }

    glUseProgram( 0 );
    glBindFramebuffer( GL_FRAMEBUFFER, (GLuint) previousFramebuffer );

    const SNamed& final = textures[ MainName ];
    if ( success )
    {
        resultTexture = final.texture;
        resultWidth = final.width;
        resultHeight = final.height;
    }
    return success;
}

/*-------------------------------------------------------------------------*/

void
CGLHookShader::Release( void )
{GUCEF_TRACE;

    for ( UInt32 p=0; p<(UInt32)m_passes.size(); ++p )
    {
        if ( 0 != m_passes[ p ].program )
            glDeleteProgram( m_passes[ p ].program );
        m_passes[ p ].program = 0;
    }
    for ( UInt32 i=0; i<(UInt32)m_targets.size(); ++i )
    {
        GLuint framebuffer = m_targets[ i ].framebuffer;
        GLuint texture = m_targets[ i ].texture;
        glDeleteFramebuffers( 1, &framebuffer );
        glDeleteTextures( 1, &texture );
    }
    m_targets.clear();
    if ( 0 != m_vertexArray )
    {
        GLuint vertexArray = m_vertexArray;
        glDeleteVertexArrays( 1, &vertexArray );
    }
    m_vertexArray = 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERRMLUIGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
