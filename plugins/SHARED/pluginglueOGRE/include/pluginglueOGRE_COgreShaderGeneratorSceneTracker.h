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

#ifndef GUCEF_PLUGINGLUE_OGRE_COGRESHADERGENERATORSCENETRACKER_H
#define GUCEF_PLUGINGLUE_OGRE_COGRESHADERGENERATORSCENETRACKER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreFrameListener.h"
#include "OgreSceneManager.h"

#ifndef GUCEF_PLUGINGLUE_OGRE_MACROS_H
#include "pluginglueOGRE_macros.h"
#define GUCEF_PLUGINGLUE_OGRE_MACROS_H
#endif /* GUCEF_PLUGINGLUE_OGRE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace Ogre { namespace RTShader { class ShaderGenerator; } }

namespace GUCEF {
namespace PLUGINGLUE {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Keeps the scene managers of the shader generator in step with those of the Root, so applications do not have to.
 *
 *  The shader generator needs every scene manager that draws with its techniques (it follows the lights of the scene
 *  and the objects being drawn), and it must let go of a scene manager before that is destroyed: it keeps a pointer.
 *  The tracker adds the scene managers of the Root when it starts and at the start of every frame (a frame listener
 *  of the Root), and removes a scene manager from the generator when that scene manager reports its destruction (a
 *  listener of each scene manager it added). Stop() removes them all, before the generator is destroyed.
 */
class GUCEF_PLUGINGLUE_OGRE_EXPORT_CPP COgreShaderGeneratorSceneTracker : public Ogre::FrameListener ,
                                                                         public Ogre::SceneManager::Listener
{
    public:

    COgreShaderGeneratorSceneTracker( Ogre::Root& root, Ogre::RTShader::ShaderGenerator& shaderGenerator );

    virtual ~COgreShaderGeneratorSceneTracker();

    /** Adds the scene managers there are and listens to the frames of the Root */
    void Start( void );

    /** Removes the scene managers from the generator and stops listening */
    void Stop( void );

    /** Adds the scene managers of the Root that the generator does not have yet. Returns how many were added */
    UInt32 AddNewSceneManagers( void );

    /** The number of scene managers the tracker added and that still exist: for tests */
    UInt32 GetTrackedSceneManagerCount( void ) const;

    virtual bool frameStarted( const Ogre::FrameEvent& evt ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void sceneManagerDestroyed( Ogre::SceneManager* sceneManager ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    COgreShaderGeneratorSceneTracker( const COgreShaderGeneratorSceneTracker& src );              /**< not implemented */
    COgreShaderGeneratorSceneTracker& operator=( const COgreShaderGeneratorSceneTracker& src );   /**< not implemented */

    private:

    typedef GUCEF::set< Ogre::SceneManager* > TSceneManagerSet;

    Ogre::Root& m_root;
    Ogre::RTShader::ShaderGenerator& m_shaderGenerator;
    TSceneManagerSet m_sceneManagers;
    bool m_started;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_OGRE_COGRESHADERGENERATORSCENETRACKER_H ? */
