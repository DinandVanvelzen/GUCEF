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

#ifndef GUCEF_ASSETCOOKER_OGRE_COGRECOOKENVIRONMENT_H
#define GUCEF_ASSETCOOKER_OGRE_COGRECOOKENVIRONMENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_ASSETCOOKER_OGRE_MACROS_H
#include "assetcookerOGRE_macros.h"
#define GUCEF_ASSETCOOKER_OGRE_MACROS_H
#endif /* GUCEF_ASSETCOOKER_OGRE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace Ogre {
class DefaultHardwareBufferManager;
class DefaultTextureManager;
class MeshLodGenerator;
}

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What Ogre needs to build and write meshes and skeletons without a window and without a render system.
 *
 *  Prepare() takes what the process has and makes the rest, once:
 *    - the Ogre Root through pluginglueOGRE (COgreGlobal), when no Root exists: the Root without plugin,
 *      configuration and log files, its log in the GUCEF logs;
 *    - Ogre's default hardware buffer manager, whose buffers are in memory, when no render system made a buffer
 *      manager, and the default texture manager when there is no texture manager;
 *    - the defaults of the material manager when nothing initialised it;
 *    - the MeshLodGenerator;
 *    - the resource group of the cooker (ResourceGroupName), outside the global pool, where the meshes, skeletons and
 *      the materials that only carry the names of the submeshes live while an asset is cooked.
 *  Release() takes down what Prepare() made, in the opposite order; what was there already is left to its owner. In a
 *  process with a render system (the viewer) the cooker therefore uses its Root and buffers as they are.
 *
 *  Ogre is not thread safe in these parts: the cooker uses it on the thread that calls Cook(), as the pipeline does.
 */
class GUCEF_ASSETCOOKER_OGRE_EXPORT_CPP COgreCookEnvironment
{
    public:

    static const CORE::CString ResourceGroupName;   /**< "AssetCookerOGRE" */

    COgreCookEnvironment( void );

    ~COgreCookEnvironment();

    /** True when everything is there; false with the reason in 'problem' */
    bool Prepare( CORE::CString& problem );

    void Release( void );

    bool IsPrepared( void ) const;

    private:

    COgreCookEnvironment( const COgreCookEnvironment& src );              /**< not implemented */
    COgreCookEnvironment& operator=( const COgreCookEnvironment& src );   /**< not implemented */

    private:

    bool m_prepared;
    bool m_madeOgreGlobal;
    bool m_madeResourceGroup;
    Ogre::DefaultHardwareBufferManager* m_bufferManager;
    Ogre::DefaultTextureManager* m_textureManager;
    Ogre::MeshLodGenerator* m_lodGenerator;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETCOOKER_OGRE_COGRECOOKENVIRONMENT_H ? */
