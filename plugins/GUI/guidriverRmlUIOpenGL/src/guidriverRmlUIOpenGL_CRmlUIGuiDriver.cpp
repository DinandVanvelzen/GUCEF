/*
 *  guidriverRmlUIOpenGL: module providing a GUI backend using RmlUi and OpenGL
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "RmlUi_Renderer_GL3.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUIDRIVERRMLUIGL_CRMLUIOPENGLCONTEXT_H
#include "guidriverRmlUIOpenGL_CRmlUIOpenGLContext.h"
#define GUCEF_GUIDRIVERRMLUIGL_CRMLUIOPENGLCONTEXT_H
#endif /* GUCEF_GUIDRIVERRMLUIGL_CRMLUIOPENGLCONTEXT_H ? */

#include "guidriverRmlUIOpenGL_CRmlUIGuiDriver.h"

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

typedef void* ( *TGetProcAddress )( const char* name );

bool g_glLoaded = false;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRmlUIGuiDriver::CRmlUIGuiDriver( void )
    : guidriverRmlUI::CRmlUIDriver()
    , m_glDescription()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRmlUIGuiDriver::~CRmlUIGuiDriver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CRmlUIGuiDriver::CreateGUIContext( GUI::TWindowContextPtr windowContext )
{GUCEF_TRACE;

    if ( !IsRmlUiReady() )
        return GUI::TGuiContextPtr();
    if ( windowContext.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIGuiDriver: A context needs a window with an OpenGL context" );
        return GUI::TGuiContextPtr();
    }

    // the GL functions are loaded once, in the first window's GL context
    if ( !g_glLoaded )
    {
        TGetProcAddress getProcAddress = (TGetProcAddress) CORE::StringToPointer( windowContext->GetProperty( "GL_GETPROCADDRESS" ) );
        Rml::String message;
        g_glLoaded = RmlGL3::Initialize( &message, getProcAddress );
        m_glDescription = message.c_str();
        if ( !g_glLoaded )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIGuiDriver: " << m_glDescription );
            return GUI::TGuiContextPtr();
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIGuiDriver: " << m_glDescription );
    }

    // retro art stays sharp when scaled
    RenderInterface_GL3::SetTextureFilterNearest( true );
    RenderInterface_GL3* renderer = GUCEF_NEW RenderInterface_GL3();
    if ( !(*renderer) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIGuiDriver: The OpenGL renderer could not be made (shaders)" );
        delete renderer;
        return GUI::TGuiContextPtr();
    }

    CRmlUIOpenGLContext* context = GUCEF_NEW CRmlUIOpenGLContext( this, windowContext, renderer );
    GUI::TGuiContextPtr contextPtr( context );
    if ( !context->IsValid() )
        return GUI::TGuiContextPtr();
    AddContext( contextPtr );
    return contextPtr;
}

/*-------------------------------------------------------------------------*/

CRmlUIGuiDriver::TStringSet
CRmlUIGuiDriver::GetCapabilities( void )
{GUCEF_TRACE;

    TStringSet capabilities = guidriverRmlUI::CRmlUIDriver::GetCapabilities();
    capabilities.insert( "CaptureFrame" );
    capabilities.insert( "PresentScaling" );
    capabilities.insert( "PresentFilters" );
    return capabilities;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRmlUIGuiDriver::GetDriverProperty( const CORE::CString& propertyName ) const
{GUCEF_TRACE;

    if ( "Renderer" == propertyName )
        return m_glDescription;
    return guidriverRmlUI::CRmlUIDriver::GetDriverProperty( propertyName );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRmlUIGuiDriver::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GUIDRIVERRMLUIGL::CRmlUIGuiDriver";
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
