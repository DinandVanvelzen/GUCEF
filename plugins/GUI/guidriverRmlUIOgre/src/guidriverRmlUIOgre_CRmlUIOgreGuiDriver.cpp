/*
 *  guidriverRmlUIOgre: gucefGUI driver drawing RmlUi through Ogre, over the 3D scene of an Ogre window
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

#include "guidriverRmlUIOgre_CRmlUIOgreRenderInterface.h"
#include "guidriverRmlUIOgre_CRmlUIOgreContext.h"
#include "guidriverRmlUIOgre_CRmlUIOgreGuiDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERRMLUIOGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CRmlUIOgreGuiDriver::RegistrationName = "RmlUiOgre";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRmlUIOgreGuiDriver::CRmlUIOgreGuiDriver( void )
    : guidriverRmlUI::CRmlUIDriver()
    , m_rendererDescription()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRmlUIOgreGuiDriver::~CRmlUIOgreGuiDriver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CRmlUIOgreGuiDriver::CreateGUIContext( GUI::TWindowContextPtr windowContext )
{GUCEF_TRACE;

    if ( !IsRmlUiReady() )
        return GUI::TGuiContextPtr();
    if ( windowContext.IsNULL() || GUIDRIVEROGRE::COgreWindowContext::ClassTypeName != windowContext->GetClassTypeName() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIOgreGuiDriver: A context needs a window of the window backend Ogre" );
        return GUI::TGuiContextPtr();
    }
    GUIDRIVEROGRE::COgreWindowContext* window = static_cast< GUIDRIVEROGRE::COgreWindowContext* >( windowContext.GetPointerAlways() );
    if ( GUCEF_NULL == window->GetOgreRenderWindow() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIOgreGuiDriver: The Ogre window has no render window" );
        return GUI::TGuiContextPtr();
    }

    CRmlUIOgreRenderInterface* renderer = GUCEF_NEW CRmlUIOgreRenderInterface();
    if ( !renderer->Initialize() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIOgreGuiDriver: The Ogre render interface could not be made (GUI material)" );
        GUCEF_DELETE renderer;
        return GUI::TGuiContextPtr();
    }
    if ( m_rendererDescription.IsNULLOrEmpty() && GUCEF_NULL != window->GetOgreRoot()->getRenderSystem() )
        m_rendererDescription = CORE::CString( "Ogre " ) + window->GetOgreRoot()->getRenderSystem()->getName().c_str();

    CRmlUIOgreContext* context = GUCEF_NEW CRmlUIOgreContext( this, windowContext, renderer );
    GUI::TGuiContextPtr contextPtr( context );
    if ( !context->IsValid() )
        return GUI::TGuiContextPtr();
    AddContext( contextPtr );
    return contextPtr;
}

/*-------------------------------------------------------------------------*/

CRmlUIOgreGuiDriver::TStringSet
CRmlUIOgreGuiDriver::GetCapabilities( void )
{GUCEF_TRACE;

    TStringSet capabilities = guidriverRmlUI::CRmlUIDriver::GetCapabilities();
    capabilities.insert( "CaptureFrame" );
    return capabilities;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRmlUIOgreGuiDriver::GetDriverProperty( const CORE::CString& propertyName ) const
{GUCEF_TRACE;

    if ( "Renderer" == propertyName )
        return m_rendererDescription;
    return guidriverRmlUI::CRmlUIDriver::GetDriverProperty( propertyName );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRmlUIOgreGuiDriver::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GUIDRIVERRMLUIOGRE::CRmlUIOgreGuiDriver";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERRMLUIOGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
