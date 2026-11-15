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

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H
#define GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_MACROS_H
#include "pluginglueOGRE_macros.h"
#define GUCEF_PLUGINGLUE_OGRE_MACROS_H
#endif /* GUCEF_PLUGINGLUE_OGRE_MACROS_H ? */

namespace Ogre { class Root; }

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class COgreTaskWorkQueue;

/*-------------------------------------------------------------------------*/

/**
 *  Makes the Ogre Root for GUCEF applications and holds the Ogre part of the global configuration.
 *
 *  Ogre does not read files by itself in this code base: the Root is made with empty names for the plugin,
 *  configuration and log file (Ogre::Root( "", "", "" )), so it reads no plugins.cfg and ogre.cfg and writes no
 *  log file. Plugins are loaded through the bootstrap configuration (COgrePluginManager), resources through the
 *  VFS (the archive factories of OgrePlugin_GucefVFS), the log goes to GUCEF (COgreLogAdapter).
 *
 *  The configuration is the section "Ogre" of the global configuration:
 *
 *      "Ogre": {
 *          "renderSystems": [ "OpenGL 3+ Rendering Subsystem", "OpenGL ES 2.x Rendering Subsystem" ],
 *          "ResourceGroups": [
 *              { "name": "Essential", "path": "/data/renderdriver", "recursive": true },
 *              { "name": "Assets", "path": "/assets", "recursive": true, "archiveType": "GucefVFS" }
 *          ],
 *          "workQueue": "Gucef",
 *          "workerPool": "OgreWork",
 *          "workerThreads": 2,
 *          "mainThreadBudgetMs": 10,
 *          "workersCanAccessRenderSystem": false
 *      }
 *
 *  The render systems are tried in the order given and the first one that a loaded Ogre plugin provides is used
 *  when the Root is initialised, which is done when the first window needs it (InitialiseRoot()). The resource
 *  groups are declared by DeclareResourceGroups(): the group is created when it is not there and the VFS path is
 *  added as its location, with the archive type given ("GucefVFS" by default).
 *
 *  The work queue of the Root: with "workQueue" "Gucef" (the default) the Root gets a COgreTaskWorkQueue as soon as it
 *  is made, so Ogre's background work runs as GUCEF tasks on the thread pool "workerPool" with at least "workerThreads"
 *  worker threads (0: the work runs on the thread that adds it), and main thread tasks get "mainThreadBudgetMs" per
 *  frame. "workersCanAccessRenderSystem" registers the worker threads with the render system. "workQueue" "Ogre" puts
 *  Ogre's own DefaultWorkQueue back, with its own threads. The queue is chosen when the Root is made, when the
 *  configuration is loaded and before the Root is initialised; as for Root::setWorkQueue(), change it before tasks are
 *  added. Settings of the queue take effect when it starts, with the first render window.
 */
class GUCEF_PLUGINGLUE_OGRE_EXPORT_CPP COgreRootSetup : public CORE::CGloballyConfigurable
{
    public:

    struct SResourceGroup
    {
        CORE::CString name;
        CORE::CString path;
        bool recursive;
        CORE::CString archiveType;
    };
    typedef GUCEF::vector< SResourceGroup >     TResourceGroupVector;

    static const CORE::CString DefaultArchiveType;      /**< "GucefVFS" */
    static const CORE::CString WorkQueueTypeGucef;      /**< "Gucef": COgreTaskWorkQueue on a GUCEF thread pool */
    static const CORE::CString WorkQueueTypeOgre;       /**< "Ogre": Ogre's own DefaultWorkQueue */

    COgreRootSetup( void );

    virtual ~COgreRootSetup() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Makes the Root without plugins, configuration and log file. When there is a Root already, which something
     *  else made, that one is used and left to its owner. False when the Root could not be made
     */
    bool CreateRoot( void );

    /** Destroys the Root when this object made it. Ogre plugins are uninstalled by this, so they are taken out first */
    void DestroyRoot( void );

    /** The Root, NULL when there is none */
    Ogre::Root* GetRoot( void ) const;

    bool IsRootOwned( void ) const;

    /**
     *  Selects the render system from the configured list and initialises the Root without a window. Initialising
     *  more than once is not an error. False, with the reason in the log, when there is no Root or no usable
     *  render system. When the list is empty the first render system that is available is taken.
     */
    bool InitialiseRoot( void );

    /** Creates the configured resource groups and adds their locations. Returns the number of locations added */
    UInt32 DeclareResourceGroups( void );

    const CORE::CString::StringVector& GetRenderSystems( void ) const;

    void SetRenderSystems( const CORE::CString::StringVector& renderSystems );

    const TResourceGroupVector& GetResourceGroups( void ) const;

    void AddResourceGroup( const SResourceGroup& group );

    /**
     *  Gives the Root the work queue of the configured type, made with the configured settings, or applies the settings
     *  to the queue it has. Only for a Root that this object made. False when there is no such Root
     */
    bool InstallWorkQueue( void );

    /** The work queue on the GUCEF thread pool that the Root has, NULL when it has another one or there is no Root */
    COgreTaskWorkQueue* GetTaskWorkQueue( void ) const;

    void SetWorkQueueType( const CORE::CString& workQueueType );

    const CORE::CString& GetWorkQueueType( void ) const;

    void SetWorkerPoolName( const CORE::CString& poolName );

    const CORE::CString& GetWorkerPoolName( void ) const;

    void SetWorkerThreadCount( UInt32 threadCount );

    UInt32 GetWorkerThreadCount( void ) const;

    void SetMainThreadBudgetInMs( UInt32 budgetInMs );

    UInt32 GetMainThreadBudgetInMs( void ) const;

    void SetWorkersCanAccessRenderSystem( bool canAccess );

    bool GetWorkersCanAccessRenderSystem( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    COgreRootSetup( const COgreRootSetup& src );              /**< not implemented */
    COgreRootSetup& operator=( const COgreRootSetup& src );   /**< not implemented */

    private:

    Ogre::Root* m_root;
    bool m_ownsRoot;
    CORE::CString::StringVector m_renderSystems;
    TResourceGroupVector m_resourceGroups;
    CORE::CString m_workQueueType;
    CORE::CString m_workerPoolName;
    UInt32 m_workerThreadCount;
    UInt32 m_mainThreadBudgetInMs;
    bool m_workersCanAccessRenderSystem;
    COgreTaskWorkQueue* m_taskWorkQueue;    /**< the queue installed in the Root, which owns it */
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

#endif /* GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H ? */
