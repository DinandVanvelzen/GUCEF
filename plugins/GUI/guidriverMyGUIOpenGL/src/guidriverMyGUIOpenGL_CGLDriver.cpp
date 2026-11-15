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

#ifndef GUCEF_guidriverMyGUIOpenGL_CGLCONTEXT_H
#include "guidriverMyGUIOpenGL_CGLContext.h"
#define GUCEF_guidriverMyGUIOpenGL_CGLCONTEXT_H
#endif /* GUCEF_guidriverMyGUIOpenGL_CGLCONTEXT_H ? */

#include "guidriverMyGUIOpenGL_CGLDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUIOpenGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CGLDriver::DriverName = "MyGUIOpenGL";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The major and minor version of an OpenGL version string ("3.3.0 Mesa", "OpenGL ES 3.0 ...") */
bool
ParseVersion( const char* text, int& major, int& minor )
{
    if ( GUCEF_NULL == text )
        return false;
    while ( 0 != *text && ( *text < '0' || *text > '9' ) )
        ++text;
    if ( 0 == *text )
        return false;
    major = atoi( text );
    while ( 0 != *text && '.' != *text )
        ++text;
    minor = ( '.' == *text ) ? atoi( text + 1 ) : 0;
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGLDriver::CGLDriver( void )
    : guidriverMyGUI::CMyGUIDriver()
    , m_backendName()
    , m_platform()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGLDriver::~CGLDriver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CGLDriver::CreateGUIContext( GUI::TWindowContextPtr windowContext )
{GUCEF_TRACE;

    if ( windowContext.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIOpenGLDriver: A context needs a window with an OpenGL context" );
        return GUI::TGuiContextPtr();
    }
    if ( HasContext() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIOpenGLDriver: MyGUI has one Gui per process, the driver has a context already" );
        return GUI::TGuiContextPtr();
    }

    #if defined( _WIN32 ) && !defined( __EMSCRIPTEN__ )
    {
        // the functions of OpenGL 2 and later come from the context that is current in the window
        static bool glewReady = false;
        if ( !glewReady )
            glewReady = GLEW_OK == glewInit();
        if ( !glewReady )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIOpenGLDriver: The OpenGL functions could not be loaded" );
            return GUI::TGuiContextPtr();
        }
    }
    #endif

    // which platform of MyGUI: ES 3 where only that exists, else by the version of the OpenGL of the window
    IGLBackend* backend = GUCEF_NULL;
    #if defined( __EMSCRIPTEN__ ) || defined( __ANDROID__ )
    backend = CreateGLESBackend();
    #else
    {
        CString api = windowContext->GetProperty( "GL_API" );
        if ( "GLES" == api )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIOpenGLDriver: The window has an OpenGL ES context, this build of the driver draws with desktop OpenGL" );
            return GUI::TGuiContextPtr();
        }
        int major = 0, minor = 0;
        CString version = windowContext->GetProperty( "GL_VERSION" );
        if ( !ParseVersion( version.C_String(), major, minor ) )
            ParseVersion( reinterpret_cast< const char* >( glGetString( GL_VERSION ) ), major, minor );
        if ( "OpenGL" == m_platform )
            backend = CreateGLBackend();
        else
        if ( "OpenGL3" == m_platform || major > 3 || ( 3 == major && minor >= 3 ) )
            backend = CreateGL3Backend();
        else
            backend = CreateGLBackend();
    }
    #endif
    if ( GUCEF_NULL == backend )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIOpenGLDriver: There is no MyGUI platform for the OpenGL of the window in this build" );
        return GUI::TGuiContextPtr();
    }
    m_backendName = backend->GetName();

    CGLContext* context = GUCEF_NEW CGLContext( this, windowContext, backend );
    GUI::TGuiContextPtr contextPtr( context );
    if ( !context->IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIOpenGLDriver: MyGUI could not start on " << m_backendName );
        return GUI::TGuiContextPtr();
    }
    LoadPendingFonts( *context );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIOpenGLDriver: Drawing with the " << m_backendName << " platform of MyGUI" );
    AddContext( contextPtr );
    return contextPtr;
}

/*-------------------------------------------------------------------------*/

CString
CGLDriver::GetDriverName( void )
{GUCEF_TRACE;

    return DriverName;
}

/*-------------------------------------------------------------------------*/

CString
CGLDriver::GetDriverProperty( const CString& propertyName ) const
{GUCEF_TRACE;

    if ( "Renderer" == propertyName )
        return m_backendName;
    return guidriverMyGUI::CMyGUIDriver::GetDriverProperty( propertyName );
}

/*-------------------------------------------------------------------------*/

CGLDriver::TStringSet
CGLDriver::GetCapabilities( void )
{GUCEF_TRACE;

    TStringSet capabilities = guidriverMyGUI::CMyGUIDriver::GetCapabilities();
    capabilities.insert( "Render" );
    capabilities.insert( "CaptureFrame" );
    capabilities.insert( "PresentScaling" );
    return capabilities;
}

/*-------------------------------------------------------------------------*/

bool
CGLDriver::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    if ( !guidriverMyGUI::CMyGUIDriver::SaveConfig( tree ) )
        return false;
    CORE::CDataNode* section = tree.Structure( ConfigSectionName, '/' );
    if ( GUCEF_NULL == section )
        return false;
    section->SetAttribute( "Platform", m_platform );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGLDriver::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    if ( !guidriverMyGUI::CMyGUIDriver::LoadConfig( treeroot ) )
        return false;
    const CORE::CDataNode* section = &treeroot;
    if ( treeroot.GetName() != ConfigSectionName )
        section = treeroot.FindChild( ConfigSectionName );
    if ( GUCEF_NULL != section )
        m_platform = section->GetAttributeValueOrChildValueByName( "Platform" ).AsString( m_platform, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CGLDriver::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::guidriverMyGUIOpenGL::CGLDriver";
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
