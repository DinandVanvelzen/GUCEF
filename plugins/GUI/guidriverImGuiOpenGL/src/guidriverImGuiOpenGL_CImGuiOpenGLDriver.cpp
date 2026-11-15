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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUIDRIVERIMGUIGL_CIMGUIOPENGLCONTEXT_H
#include "guidriverImGuiOpenGL_CImGuiOpenGLContext.h"
#define GUIDRIVERIMGUIGL_CIMGUIOPENGLCONTEXT_H
#endif /* GUIDRIVERIMGUIGL_CIMGUIOPENGLCONTEXT_H ? */

#include "guidriverImGuiOpenGL_CImGuiOpenGLDriver.h"

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

CImGuiOpenGLDriver::CImGuiOpenGLDriver( void )
    : GUIDRIVERIMGUI::CImGuiDriver()
    , m_glDescription()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImGuiOpenGLDriver::~CImGuiOpenGLDriver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CImGuiOpenGLDriver::CreateGUIContext( GUI::TWindowContextPtr windowContext )
{GUCEF_TRACE;

    if ( !CanCreateContext() )
        return GUI::TGuiContextPtr();
    if ( windowContext.IsNULL() || windowContext->GetProperty( "GL_GETPROCADDRESS" ).IsNULLOrEmpty() )
    {
        // also windows made without a GL context (GraphicsApi None) and windows drawn by another renderer (Ogre)
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOpenGLDriver: A context needs a window with an OpenGL context" );
        return GUI::TGuiContextPtr();
    }

    CORE::CString glslVersion, reason;
    if ( !CImGuiOpenGLContext::SelectGlslVersion( windowContext->GetProperty( "GL_API" )     ,
                                                  windowContext->GetProperty( "GL_VERSION" ) ,
                                                  CImGuiGLFunctions::IsOpenGlEsBuild()       ,
                                                  glslVersion                                ,
                                                  reason                                     ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOpenGLDriver: " << reason );
        return GUI::TGuiContextPtr();
    }

    // the GL entry points come from the function of this window, which is also the one of the backend
    CImGuiGLFunctions::TGetProcAddress getProcAddress = (CImGuiGLFunctions::TGetProcAddress) CORE::StringToPointer( windowContext->GetProperty( "GL_GETPROCADDRESS" ) );
    CImGuiGLFunctions functions;
    if ( !functions.Load( getProcAddress ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOpenGLDriver: The OpenGL entry points cannot be loaded" );
        return GUI::TGuiContextPtr();
    }
    if ( m_glDescription.IsNULLOrEmpty() )
    {
        const char* vendor = reinterpret_cast< const char* >( functions.GetString( CImGuiGLFunctions::GLC_VENDOR ) );
        const char* renderer = reinterpret_cast< const char* >( functions.GetString( CImGuiGLFunctions::GLC_RENDERER ) );
        const char* version = reinterpret_cast< const char* >( functions.GetString( CImGuiGLFunctions::GLC_VERSION ) );
        m_glDescription = CORE::CString( GUCEF_NULL != vendor ? vendor : "?" ) + ", " + ( GUCEF_NULL != renderer ? renderer : "?" ) +
                          ", " + ( GUCEF_NULL != version ? version : "?" );
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOpenGLDriver: " << m_glDescription << ", GLSL \"" << glslVersion << "\"" );
    }

    CImGuiOpenGLContext* context = GUCEF_NEW CImGuiOpenGLContext( this, windowContext, glslVersion, functions, getProcAddress );
    if ( !context->IsRendererReady() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOpenGLDriver: The OpenGL renderer could not be made (see the entries before this one)" );
        // the context goes with its owner pointer; a context that is not ready cannot be given away
        GUI::TGuiContextPtr discard( context );
        return GUI::TGuiContextPtr();
    }
    return AddContext( context );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CImGuiOpenGLDriver::GetDriverProperty( const CORE::CString& propertyName ) const
{GUCEF_TRACE;

    if ( "Renderer" == propertyName )
        return m_glDescription;
    return GUIDRIVERIMGUI::CImGuiDriver::GetDriverProperty( propertyName );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CImGuiOpenGLDriver::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GUIDRIVERIMGUIGL::CImGuiOpenGLDriver";
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
