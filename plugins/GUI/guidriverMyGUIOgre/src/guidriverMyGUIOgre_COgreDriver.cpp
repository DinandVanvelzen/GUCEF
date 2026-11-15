/*
 *  guidriverMyGUIOgre: GUI backend using MyGUI with Ogre
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

#include "MyGUI_OgreDataManager.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_guidriverMyGUIOgre_COGRECONTEXT_H
#include "guidriverMyGUIOgre_COgreContext.h"
#define GUCEF_guidriverMyGUIOgre_COGRECONTEXT_H
#endif /* GUCEF_guidriverMyGUIOgre_COGRECONTEXT_H ? */

#include "guidriverMyGUIOgre_COgreDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUIOgre {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* COgreDriver::DriverName = "MyGUIOgre";
const char* COgreDriver::ResourceGroupName = "MyGUI";
const char* COgreDriver::ArchiveTypeName = "GucefVFS";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreDriver::COgreDriver( void )
    : guidriverMyGUI::CMyGUIDriver( false )
    , m_ogreDataManager( GUCEF_NULL )
    , m_resourcesReady( false )
{GUCEF_TRACE;

    // the Ogre platform of MyGUI reads through the resource group of the data manager
    m_ogreDataManager = new MyGUI::OgreDataManager();
    m_ogreDataManager->setGroup( ResourceGroupName );
}

/*-------------------------------------------------------------------------*/

COgreDriver::~COgreDriver()
{GUCEF_TRACE;

    delete m_ogreDataManager;
    m_ogreDataManager = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
COgreDriver::PrepareResources( void )
{GUCEF_TRACE;

    if ( m_resourcesReady )
        return true;
    Ogre::ResourceGroupManager* groups = Ogre::ResourceGroupManager::getSingletonPtr();
    if ( GUCEF_NULL == groups )
        return false;

    try
    {
        if ( !groups->resourceGroupExists( ResourceGroupName ) )
            groups->createResourceGroup( ResourceGroupName );
        groups->addResourceLocation( EngineDataName, ArchiveTypeName, ResourceGroupName );
        m_resourcesReady = true;
    }
    catch ( const std::exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIOgreDriver: The data of MyGUI cannot be added to the Ogre resource system as an archive of type " << ArchiveTypeName <<
                         " (the Ogre plugin OgrePlugin_GucefVFS provides it): " << e.what() );
    }
    return m_resourcesReady;
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
COgreDriver::CreateGUIContext( GUI::TWindowContextPtr windowContext )
{GUCEF_TRACE;

    if ( windowContext.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIOgreDriver: A context needs an Ogre render window" );
        return GUI::TGuiContextPtr();
    }
    if ( HasContext() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIOgreDriver: MyGUI has one Gui per process, the driver has a context already" );
        return GUI::TGuiContextPtr();
    }

    Ogre::RenderWindow* renderWindow = static_cast< Ogre::RenderWindow* >( CORE::StringToPointer( windowContext->GetProperty( "Ogre::RenderWindow" ) ) );
    Ogre::SceneManager* sceneManager = static_cast< Ogre::SceneManager* >( CORE::StringToPointer( windowContext->GetProperty( "Ogre::SceneManager" ) ) );
    if ( GUCEF_NULL == renderWindow || GUCEF_NULL == sceneManager )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIOgreDriver: The window has no Ogre render window and scene manager" );
        return GUI::TGuiContextPtr();
    }
    if ( !PrepareResources() )
        return GUI::TGuiContextPtr();

    COgreContext* context = GUCEF_NEW COgreContext( this, windowContext, renderWindow, sceneManager );
    GUI::TGuiContextPtr contextPtr( context );
    if ( !context->IsValid() )
        return GUI::TGuiContextPtr();
    LoadPendingFonts( *context );
    AddContext( contextPtr );
    return contextPtr;
}

/*-------------------------------------------------------------------------*/

CString
COgreDriver::GetDriverName( void )
{GUCEF_TRACE;

    return DriverName;
}

/*-------------------------------------------------------------------------*/

COgreDriver::TStringSet
COgreDriver::GetCapabilities( void )
{GUCEF_TRACE;

    TStringSet capabilities = guidriverMyGUI::CMyGUIDriver::GetCapabilities();
    capabilities.insert( "Render" );
    capabilities.insert( "CaptureFrame" );
    return capabilities;
}

/*-------------------------------------------------------------------------*/

const CString&
COgreDriver::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::guidriverMyGUIOgre::COgreDriver";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUIOgre */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
