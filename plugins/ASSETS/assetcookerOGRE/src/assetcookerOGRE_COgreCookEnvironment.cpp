/*
 *  assetcookerOGRE: GUCEF asset converter plugin: the cooker of the intermediate into Ogre 1.x runtime files
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

#include "Ogre.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreMeshLodGenerator.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H
#include "pluginglueOGRE_COgreGlobal.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H ? */

#include "assetcookerOGRE_COgreCookEnvironment.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCOOKER {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString COgreCookEnvironment::ResourceGroupName = "AssetCookerOGRE";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreCookEnvironment::COgreCookEnvironment( void )
    : m_prepared( false )
    , m_madeOgreGlobal( false )
    , m_madeResourceGroup( false )
    , m_bufferManager( GUCEF_NULL )
    , m_textureManager( GUCEF_NULL )
    , m_lodGenerator( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COgreCookEnvironment::~COgreCookEnvironment()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

bool
COgreCookEnvironment::IsPrepared( void ) const
{GUCEF_TRACE;

    return m_prepared;
}

/*-------------------------------------------------------------------------*/

bool
COgreCookEnvironment::Prepare( CORE::CString& problem )
{GUCEF_TRACE;

    if ( m_prepared )
        return true;

    try
    {
        if ( GUCEF_NULL == Ogre::Root::getSingletonPtr() )
        {
            m_madeOgreGlobal = GUCEF_NULL != PLUGINGLUE::OGRE::COgreGlobal::Instance();
            if ( GUCEF_NULL == Ogre::Root::getSingletonPtr() )
            {
                problem = "the Ogre Root cannot be made";
                return false;
            }
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "assetcookerOGRE: Made the Ogre Root, headless" );
        }

        // without a render system the buffers are in memory and textures are not made
        if ( GUCEF_NULL == Ogre::HardwareBufferManager::getSingletonPtr() )
        {
            m_bufferManager = OGRE_NEW Ogre::DefaultHardwareBufferManager();
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "assetcookerOGRE: Uses the software hardware buffer manager of Ogre" );
        }
        if ( GUCEF_NULL == Ogre::TextureManager::getSingletonPtr() )
            m_textureManager = OGRE_NEW Ogre::DefaultTextureManager();
        if ( !Ogre::MaterialManager::getSingleton().getDefaultSettings() )
            Ogre::MaterialManager::getSingleton().initialise();

        if ( GUCEF_NULL == Ogre::MeshLodGenerator::getSingletonPtr() )
            m_lodGenerator = OGRE_NEW Ogre::MeshLodGenerator();

        Ogre::ResourceGroupManager& groups = Ogre::ResourceGroupManager::getSingleton();
        if ( !groups.resourceGroupExists( ResourceGroupName.STL_String() ) )
        {
            groups.createResourceGroup( ResourceGroupName.STL_String(), false );
            m_madeResourceGroup = true;
        }
    }
    catch ( const Ogre::Exception& e )
    {
        problem = "Ogre cannot be prepared: " + CORE::CString( e.getFullDescription() );
        Release();
        return false;
    }

    m_prepared = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
COgreCookEnvironment::Release( void )
{GUCEF_TRACE;

    try
    {
        if ( m_madeResourceGroup && GUCEF_NULL != Ogre::ResourceGroupManager::getSingletonPtr() )
            Ogre::ResourceGroupManager::getSingleton().destroyResourceGroup( ResourceGroupName.STL_String() );
    }
    catch ( const Ogre::Exception& )
    {
    }
    m_madeResourceGroup = false;

    OGRE_DELETE m_lodGenerator;
    m_lodGenerator = GUCEF_NULL;
    OGRE_DELETE m_textureManager;
    m_textureManager = GUCEF_NULL;
    OGRE_DELETE m_bufferManager;
    m_bufferManager = GUCEF_NULL;

    if ( m_madeOgreGlobal )
    {
        PLUGINGLUE::OGRE::COgreGlobal::Deinstance();
        m_madeOgreGlobal = false;
    }
    m_prepared = false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
