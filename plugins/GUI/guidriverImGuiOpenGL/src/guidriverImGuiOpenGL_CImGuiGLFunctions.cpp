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

/*
 *  OpenGL ES builds (Android, the web) call the linked functions; the desktop build has the loader of the backend,
 *  which only declares its function table here (the backend source holds the implementation)
 */
#if defined( __ANDROID__ ) || defined( __EMSCRIPTEN__ )
  #define GUIDRIVERIMGUIGL_OPENGLES_BUILD
  #include <GLES3/gl3.h>
#else
  #include "imgui_impl_opengl3_loader.h"
#endif

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "guidriverImGuiOpenGL_CImGuiGLFunctions.h"

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

CImGuiGLFunctions::CImGuiGLFunctions( void )
    : GetString( GUCEF_NULL )
    , GetIntegerv( GUCEF_NULL )
    , GetError( GUCEF_NULL )
    , GenFramebuffers( GUCEF_NULL )
    , DeleteFramebuffers( GUCEF_NULL )
    , BindFramebuffer( GUCEF_NULL )
    , FramebufferTexture2D( GUCEF_NULL )
    , CheckFramebufferStatus( GUCEF_NULL )
    , GenTextures( GUCEF_NULL )
    , DeleteTextures( GUCEF_NULL )
    , BindTexture( GUCEF_NULL )
    , TexImage2D( GUCEF_NULL )
    , TexParameteri( GUCEF_NULL )
    , ReadPixels( GUCEF_NULL )
    , PixelStorei( GUCEF_NULL )
    , Viewport( GUCEF_NULL )
    , ClearColor( GUCEF_NULL )
    , Clear( GUCEF_NULL )
    , Disable( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CImGuiGLFunctions::IsComplete( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != GetString && GUCEF_NULL != GetIntegerv && GUCEF_NULL != GetError &&
           GUCEF_NULL != GenFramebuffers && GUCEF_NULL != DeleteFramebuffers && GUCEF_NULL != BindFramebuffer &&
           GUCEF_NULL != FramebufferTexture2D && GUCEF_NULL != CheckFramebufferStatus &&
           GUCEF_NULL != GenTextures && GUCEF_NULL != DeleteTextures && GUCEF_NULL != BindTexture &&
           GUCEF_NULL != TexImage2D && GUCEF_NULL != TexParameteri && GUCEF_NULL != ReadPixels &&
           GUCEF_NULL != PixelStorei && GUCEF_NULL != Viewport && GUCEF_NULL != ClearColor &&
           GUCEF_NULL != Clear && GUCEF_NULL != Disable;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiGLFunctions::IsOpenGlEsBuild( void )
{GUCEF_TRACE;

    #ifdef GUIDRIVERIMGUIGL_OPENGLES_BUILD
    return true;
    #else
    return false;
    #endif
}

/*-------------------------------------------------------------------------*/

bool
CImGuiGLFunctions::EnsureBackendLoader( TGetProcAddress getProcAddress )
{GUCEF_TRACE;

    #ifdef GUIDRIVERIMGUIGL_OPENGLES_BUILD

    return true;

    #else

    // the backend lets go of its entry points when one of its contexts shuts down
    if ( GUCEF_NULL != imgl3wProcs.gl.GetIntegerv )
        return true;
    if ( GUCEF_NULL == getProcAddress )
        return false;
    return GL3W_OK == imgl3wInit2( reinterpret_cast< GL3WGetProcAddressProc >( getProcAddress ) );

    #endif
}

/*-------------------------------------------------------------------------*/

bool
CImGuiGLFunctions::Load( TGetProcAddress getProcAddress )
{GUCEF_TRACE;

    #ifdef GUIDRIVERIMGUIGL_OPENGLES_BUILD

    GetString = reinterpret_cast< TGetString >( &::glGetString );
    GetIntegerv = reinterpret_cast< TGetIntegerv >( &::glGetIntegerv );
    GetError = reinterpret_cast< TGetError >( &::glGetError );
    GenFramebuffers = reinterpret_cast< TGenObjects >( &::glGenFramebuffers );
    DeleteFramebuffers = reinterpret_cast< TDeleteObjects >( &::glDeleteFramebuffers );
    BindFramebuffer = reinterpret_cast< TBindObject >( &::glBindFramebuffer );
    FramebufferTexture2D = reinterpret_cast< TFramebufferTexture2D >( &::glFramebufferTexture2D );
    CheckFramebufferStatus = reinterpret_cast< TCheckFramebufferStatus >( &::glCheckFramebufferStatus );
    GenTextures = reinterpret_cast< TGenObjects >( &::glGenTextures );
    DeleteTextures = reinterpret_cast< TDeleteObjects >( &::glDeleteTextures );
    BindTexture = reinterpret_cast< TBindObject >( &::glBindTexture );
    TexImage2D = reinterpret_cast< TTexImage2D >( &::glTexImage2D );
    TexParameteri = reinterpret_cast< TTexParameteri >( &::glTexParameteri );
    ReadPixels = reinterpret_cast< TReadPixels >( &::glReadPixels );
    PixelStorei = reinterpret_cast< TPixelStorei >( &::glPixelStorei );
    Viewport = reinterpret_cast< TViewport >( &::glViewport );
    ClearColor = reinterpret_cast< TClearColor >( &::glClearColor );
    Clear = reinterpret_cast< TClear >( &::glClear );
    Disable = reinterpret_cast< TCapability >( &::glDisable );
    return true;

    #else

    if ( GUCEF_NULL == getProcAddress )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOpenGL: The window gives no function to find the OpenGL entry points" );
        return false;
    }

    #define GUIDRIVERIMGUIGL_LOAD( member, type, name )                                                         \
        member = reinterpret_cast< type >( getProcAddress( name ) );                                            \
        if ( GUCEF_NULL == member )                                                                             \
        {                                                                                                       \
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOpenGL: The OpenGL entry point " << name << " is missing" ); \
            return false;                                                                                       \
        }

    GUIDRIVERIMGUIGL_LOAD( GetString, TGetString, "glGetString" )
    GUIDRIVERIMGUIGL_LOAD( GetIntegerv, TGetIntegerv, "glGetIntegerv" )
    GUIDRIVERIMGUIGL_LOAD( GetError, TGetError, "glGetError" )
    GUIDRIVERIMGUIGL_LOAD( GenFramebuffers, TGenObjects, "glGenFramebuffers" )
    GUIDRIVERIMGUIGL_LOAD( DeleteFramebuffers, TDeleteObjects, "glDeleteFramebuffers" )
    GUIDRIVERIMGUIGL_LOAD( BindFramebuffer, TBindObject, "glBindFramebuffer" )
    GUIDRIVERIMGUIGL_LOAD( FramebufferTexture2D, TFramebufferTexture2D, "glFramebufferTexture2D" )
    GUIDRIVERIMGUIGL_LOAD( CheckFramebufferStatus, TCheckFramebufferStatus, "glCheckFramebufferStatus" )
    GUIDRIVERIMGUIGL_LOAD( GenTextures, TGenObjects, "glGenTextures" )
    GUIDRIVERIMGUIGL_LOAD( DeleteTextures, TDeleteObjects, "glDeleteTextures" )
    GUIDRIVERIMGUIGL_LOAD( BindTexture, TBindObject, "glBindTexture" )
    GUIDRIVERIMGUIGL_LOAD( TexImage2D, TTexImage2D, "glTexImage2D" )
    GUIDRIVERIMGUIGL_LOAD( TexParameteri, TTexParameteri, "glTexParameteri" )
    GUIDRIVERIMGUIGL_LOAD( ReadPixels, TReadPixels, "glReadPixels" )
    GUIDRIVERIMGUIGL_LOAD( PixelStorei, TPixelStorei, "glPixelStorei" )
    GUIDRIVERIMGUIGL_LOAD( Viewport, TViewport, "glViewport" )
    GUIDRIVERIMGUIGL_LOAD( ClearColor, TClearColor, "glClearColor" )
    GUIDRIVERIMGUIGL_LOAD( Clear, TClear, "glClear" )
    GUIDRIVERIMGUIGL_LOAD( Disable, TCapability, "glDisable" )

    #undef GUIDRIVERIMGUIGL_LOAD

    // the backend of Dear ImGui finds its own entry points through the same function
    if ( !EnsureBackendLoader( getProcAddress ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOpenGL: The entry points of the Dear ImGui OpenGL backend cannot be loaded, is a GL context current?" );
        return false;
    }
    return true;

    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUIGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
