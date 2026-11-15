/*
 *  guidriverImGuiOgre: gucefGUI driver ImGuiOgre: Dear ImGui drawn through Ogre over the 3D scene of an Ogre window
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

#include <Ogre.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#include "guidriverOgre_COgreWindowContext.h"
#define GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#endif /* GUIDRIVEROGRE_COGREWINDOWCONTEXT_H ? */

#include "guidriverImGuiOgre_CImGuiOgreContext.h"
#include "guidriverImGuiOgre_CImGuiOgreDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUIOGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CImGuiOgreDriver::RegistrationName = "ImGuiOgre";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImGuiOgreDriver::CImGuiOgreDriver( void )
    : GUIDRIVERIMGUI::CImGuiDriver()
    , m_rendererDescription()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImGuiOgreDriver::~CImGuiOgreDriver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CImGuiOgreDriver::CreateGUIContext( GUI::TWindowContextPtr windowContext )
{GUCEF_TRACE;

    if ( !CanCreateContext() )
        return GUI::TGuiContextPtr();
    if ( windowContext.IsNULL() || GUIDRIVEROGRE::COgreWindowContext::ClassTypeName != windowContext->GetClassTypeName() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOgreDriver: A context needs a window of the window backend Ogre" );
        return GUI::TGuiContextPtr();
    }
    GUIDRIVEROGRE::COgreWindowContext* window = static_cast< GUIDRIVEROGRE::COgreWindowContext* >( windowContext.GetPointerAlways() );
    if ( GUCEF_NULL == window->GetOgreRenderWindow() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOgreDriver: The Ogre window has no render window" );
        return GUI::TGuiContextPtr();
    }
    if ( m_rendererDescription.IsNULLOrEmpty() && GUCEF_NULL != window->GetOgreRoot() && GUCEF_NULL != window->GetOgreRoot()->getRenderSystem() )
        m_rendererDescription = CORE::CString( "Ogre " ) + window->GetOgreRoot()->getRenderSystem()->getName().c_str();

    CImGuiOgreContext* context = GUCEF_NEW CImGuiOgreContext( this, windowContext );
    if ( !context->IsReady() || !context->IsRendererReady() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOgreDriver: The Ogre renderer could not be made (see the entries before this one)" );
        // the context goes with its owner pointer; a context that is not ready cannot be given away
        GUI::TGuiContextPtr discard( context );
        return GUI::TGuiContextPtr();
    }
    return AddContext( context );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CImGuiOgreDriver::GetDriverName( void )
{GUCEF_TRACE;

    return RegistrationName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CImGuiOgreDriver::GetDriverProperty( const CORE::CString& propertyName ) const
{GUCEF_TRACE;

    if ( "Renderer" == propertyName )
        return m_rendererDescription;
    return GUIDRIVERIMGUI::CImGuiDriver::GetDriverProperty( propertyName );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CImGuiOgreDriver::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GUIDRIVERIMGUIOGRE::CImGuiOgreDriver";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUIOGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
