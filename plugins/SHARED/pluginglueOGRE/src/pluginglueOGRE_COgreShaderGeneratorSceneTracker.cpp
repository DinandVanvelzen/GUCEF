/*
 *  pluginglueOGRE: Support library of the Ogre 1.x plugin set: Ogre Root setup, log, plugin loading
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

#include "OgreRoot.h"
#include "OgreShaderGenerator.h"

#include "pluginglueOGRE_COgreShaderGeneratorSceneTracker.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreShaderGeneratorSceneTracker::COgreShaderGeneratorSceneTracker( Ogre::Root& root, Ogre::RTShader::ShaderGenerator& shaderGenerator )
    : Ogre::FrameListener()
    , Ogre::SceneManager::Listener()
    , m_root( root )
    , m_shaderGenerator( shaderGenerator )
    , m_sceneManagers()
    , m_started( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COgreShaderGeneratorSceneTracker::~COgreShaderGeneratorSceneTracker()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

void
COgreShaderGeneratorSceneTracker::Start( void )
{GUCEF_TRACE;

    if ( m_started )
        return;

    m_started = true;
    m_root.addFrameListener( this );
    AddNewSceneManagers();
}

/*-------------------------------------------------------------------------*/

void
COgreShaderGeneratorSceneTracker::Stop( void )
{GUCEF_TRACE;

    if ( !m_started )
        return;

    m_started = false;
    m_root.removeFrameListener( this );

    TSceneManagerSet::iterator i = m_sceneManagers.begin();
    while ( i != m_sceneManagers.end() )
    {
        Ogre::SceneManager* sceneManager = (*i);
        sceneManager->removeListener( this );
        m_shaderGenerator.removeSceneManager( sceneManager );
        ++i;
    }
    m_sceneManagers.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
COgreShaderGeneratorSceneTracker::AddNewSceneManagers( void )
{GUCEF_TRACE;

    if ( !m_started )
        return 0;

    UInt32 added = 0;
    const Ogre::SceneManagerInstanceMap& sceneManagers = m_root.getSceneManagers();
    Ogre::SceneManagerInstanceMap::const_iterator i = sceneManagers.begin();
    while ( i != sceneManagers.end() )
    {
        Ogre::SceneManager* sceneManager = (*i).second;
        if ( m_sceneManagers.end() == m_sceneManagers.find( sceneManager ) )
        {
            m_shaderGenerator.addSceneManager( sceneManager );
            sceneManager->addListener( this );
            m_sceneManagers.insert( sceneManager );
            ++added;
        }
        ++i;
    }
    return added;
}

/*-------------------------------------------------------------------------*/

UInt32
COgreShaderGeneratorSceneTracker::GetTrackedSceneManagerCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_sceneManagers.size();
}

/*-------------------------------------------------------------------------*/

bool
COgreShaderGeneratorSceneTracker::frameStarted( const Ogre::FrameEvent& evt )
{GUCEF_TRACE;

    AddNewSceneManagers();
    return true;
}

/*-------------------------------------------------------------------------*/

void
COgreShaderGeneratorSceneTracker::sceneManagerDestroyed( Ogre::SceneManager* sceneManager )
{GUCEF_TRACE;

    TSceneManagerSet::iterator i = m_sceneManagers.find( sceneManager );
    if ( i == m_sceneManagers.end() )
        return;

    // the generator keeps the pointer: it lets go before the scene manager is gone
    m_shaderGenerator.removeSceneManager( sceneManager );
    m_sceneManagers.erase( i );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
