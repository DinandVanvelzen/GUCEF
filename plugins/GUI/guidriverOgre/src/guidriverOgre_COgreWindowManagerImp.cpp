/*
 *  guidriverOgre: gucefGUI window backend that gives Ogre render windows on the platform windows
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

#ifndef GUCEF_GUI_CVIDEOSETTINGS_H
#include "gucefGUI_CVideoSettings.h"
#define GUCEF_GUI_CVIDEOSETTINGS_H
#endif /* GUCEF_GUI_CVIDEOSETTINGS_H ? */

#ifndef GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#include "guidriverOgre_COgreWindowContext.h"
#define GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#endif /* GUIDRIVEROGRE_COGREWINDOWCONTEXT_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H
#include "pluginglueOGRE_COgreGlobal.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H
#include "pluginglueOGRE_COgreRootSetup.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H ? */

#include "guidriverOgre_COgreWindowManagerImp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEROGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString COgreWindowManagerImp::BackendName = "Ogre";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreWindowManagerImp::COgreWindowManagerImp( void )
    : GUI::CWindowManagerBackend()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COgreWindowManagerImp::~COgreWindowManagerImp()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
COgreWindowManagerImp::CreateWindowContext( const GUI::CString& title ,
                                            const GUI::UInt32 width   ,
                                            const GUI::UInt32 height  ,
                                            const bool fullscreen     )
{GUCEF_TRACE;

    GUI::CVideoSettings videoSettings;
    videoSettings.SetFullscreenState( fullscreen );
    videoSettings.SetResolution( width, height, 32 );
    return CreateWindowContext( title, videoSettings, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
COgreWindowManagerImp::CreateWindowContext( const GUI::CString& title           ,
                                            const GUI::CVideoSettings& settings ,
                                            const CORE::CValueList* params      )
{GUCEF_TRACE;

    if ( !IsRequestedGraphicsApiSupported( BackendName, params ) )
        return GUI::TWindowContextPtr();

    TOgreWindowContextPtr windowContext( GUCEF_NEW COgreWindowContext() );
    if ( windowContext->Initialize( title, settings, params ) )
        return windowContext;
    return GUI::TWindowContextPtr();
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
COgreWindowManagerImp::CreateWindowContext( const GUI::CString& title      ,
                                            const GUI::UInt32 width        ,
                                            const GUI::UInt32 height       ,
                                            const bool fullscreen          ,
                                            const CORE::CValueList& params )
{GUCEF_TRACE;

    GUI::CVideoSettings videoSettings;
    videoSettings.SetFullscreenState( fullscreen );
    videoSettings.SetResolution( width, height, 32 );
    return CreateWindowContext( title, videoSettings, &params );
}

/*-------------------------------------------------------------------------*/

void
COgreWindowManagerImp::DestroyWindowContext( GUI::TWindowContextPtr& windowContext )
{GUCEF_TRACE;

    if ( windowContext.IsNULL() || COgreWindowContext::ClassTypeName != windowContext->GetClassTypeName() )
        return;
    static_cast< COgreWindowContext* >( windowContext.GetPointerAlways() )->Shutdown();
}

/*-------------------------------------------------------------------------*/

bool
COgreWindowManagerImp::ApplyVideoSettings( GUI::TWindowContextPtr& windowContext ,
                                           const GUI::CVideoSettings& settings   )
{GUCEF_TRACE;

    if ( windowContext.IsNULL() || COgreWindowContext::ClassTypeName != windowContext->GetClassTypeName() )
        return false;
    return static_cast< COgreWindowContext* >( windowContext.GetPointerAlways() )->ApplyVideoSettings( settings );
}

/*-------------------------------------------------------------------------*/

bool
COgreWindowManagerImp::IsGraphicsApiSupported( const GUI::CString& graphicsApi ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
COgreWindowManagerImp::GetAvailableRenderers( CORE::CString::StringVector& rendererNames ) const
{GUCEF_TRACE;

    // the render system plugins are loaded into the Root, which the support library makes when the first plugin needs it
    PLUGINGLUE::OGRE::COgreRootSetup& rootSetup = PLUGINGLUE::OGRE::COgreGlobal::Instance()->GetRootSetup();
    rendererNames.clear();
    if ( rootSetup.CreateRoot() )
        rootSetup.GetAvailableRenderers( rendererNames );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
COgreWindowManagerImp::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GUIDRIVEROGRE::COgreWindowManagerImp";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEROGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
