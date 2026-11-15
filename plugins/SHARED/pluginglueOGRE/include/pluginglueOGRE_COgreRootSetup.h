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

namespace Ogre { class Root; class RenderSystem; }

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
class COgreShaderTechniqueResolver;
class COgreShaderGeneratorSceneTracker;

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
 *          "workersCanAccessRenderSystem": false,
 *          "shaderGenerator": true,
 *          "shaderCachePath": ""
 *      }
 *
 *  The render systems are tried in the order given and the first one that a loaded Ogre plugin provides is used
 *  when the Root is initialised, which is done when the first window needs it (InitialiseRoot()). The list names
 *  render systems by their Ogre name ("Vulkan Rendering Subsystem") or by their renderer name ("Vulkan").
 *
 *  Renderers: the render systems as players and the video settings name them (GetRendererName()): "Vulkan",
 *  "OpenGL" (OpenGL 3+), "OpenGL ES", "Direct3D 11", "Direct3D 9", "OpenGL (legacy)"; another render system has its
 *  Ogre name. The renderers that can be picked (GetAvailableRenderers()) are those of the list (all, when it names
 *  none) that a loaded plugin provides and that can draw on this system: the Vulkan render system needs a Vulkan
 *  device, which it does not find without a Vulkan driver. InitialiseRoot( preferredRenderer ) takes the renderer of
 *  the video settings first when it is one of them, and the order of the list otherwise. The resource
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
 *
 *  The run time shader system (RTSS): the render systems without a fixed function pipeline (OpenGL 3+, OpenGL ES 2,
 *  Vulkan, Direct3D 11) draw every material with shaders that the shader generator makes, so it is started with
 *  "shaderGenerator" true (the default) by StartShaderGenerator(), which the window driver calls once the first render
 *  window exists: before that, the render system has not registered its shader languages. Starting it:
 *    - mounts the data of the modules Ogre (Ogre's Media/Main) and OgreRTShaderSystem (Media/RTShaderLib) in the VFS
 *      and adds them to Ogre's internal resource group with the archive type "GucefVFS" (DeclareInternalResources()),
 *      so OgrePlugin_GucefVFS has to be loaded;
 *    - makes the shader generator, with "shaderCachePath" as the folder its programs are written to (empty: none);
 *    - installs COgreShaderTechniqueResolver, which gives materials techniques with generated shaders;
 *    - initialises the internal resource group, when nothing did that yet;
 *    - lets COgreShaderGeneratorSceneTracker keep the scene managers of the Root registered with the generator.
 *  StopShaderGenerator() undoes it; DestroyRoot() calls it before the Root goes.
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
    static const CORE::CString OgreMediaModuleName;     /**< "Ogre": the module whose data is Ogre's Media/Main */
    static const CORE::CString RTShaderMediaModuleName; /**< "OgreRTShaderSystem": the module whose data is Ogre's Media/RTShaderLib */

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

    /**
     *  As InitialiseRoot(), with the renderer asked for (a renderer name or an Ogre render system name, in any letter
     *  case) before the configured order when it is available (GetAvailableRenderers()); when it is not, the reason is
     *  logged and the configured order is used. Empty asks for none. A Root that is initialised keeps its renderer
     */
    bool InitialiseRoot( const CORE::CString& preferredRenderer );

    /** The renderers that can be used, by renderer name, in the configured order (see the class description) */
    void GetAvailableRenderers( CORE::CString::StringVector& rendererNames ) const;

    /** The renderer name of the render system the Root uses, empty while there is none */
    CORE::CString GetActiveRenderer( void ) const;

    /** The renderer name of an Ogre render system name: "Vulkan" for "Vulkan Rendering Subsystem", ...; other names as they are */
    static CORE::CString GetRendererName( const CORE::CString& renderSystemName );

    /** Whether a render system can draw on this system: the Vulkan render system needs a Vulkan device; the others can */
    static bool IsRenderSystemUsable( const Ogre::RenderSystem& renderSystem );

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

    /**
     *  Mounts the data of Ogre and of the RTSS in the VFS and adds them to Ogre's internal resource group, once.
     *  False when there is no Root, a data folder is not there or the archive type "GucefVFS" is not registered
     */
    bool DeclareInternalResources( void );

    /**
     *  Starts the run time shader system when it is enabled (see the class description). Call it after the first render
     *  window was made. True when it runs (also when it ran already) or is disabled; false, with the reason in the log,
     *  when it cannot start
     */
    bool StartShaderGenerator( void );

    /** Stops the run time shader system when this object started it */
    void StopShaderGenerator( void );

    bool IsShaderGeneratorStarted( void ) const;

    void SetShaderGeneratorEnabled( bool enabled );

    bool GetShaderGeneratorEnabled( void ) const;

    /** The folder the generated programs are written to, so the next start reads them; empty for none */
    void SetShaderCachePath( const CORE::CString& path );

    const CORE::CString& GetShaderCachePath( void ) const;

    /** The resolver of the running shader generator, NULL while it does not run */
    COgreShaderTechniqueResolver* GetShaderTechniqueResolver( void ) const;

    /** The scene tracker of the running shader generator, NULL while it does not run */
    COgreShaderGeneratorSceneTracker* GetShaderGeneratorSceneTracker( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** The render system of a renderer name or Ogre render system name, in any letter case, that a loaded plugin provides; NULL when there is none */
    Ogre::RenderSystem* FindRenderSystem( const CORE::CString& rendererName ) const;

    /** The render systems that can be used, in the configured order */
    void GetUsableRenderSystems( GUCEF::vector< Ogre::RenderSystem* >& renderSystems ) const;

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
    bool m_shaderGeneratorEnabled;
    CORE::CString m_shaderCachePath;
    bool m_internalResourcesDeclared;
    COgreShaderTechniqueResolver* m_techniqueResolver;
    COgreShaderGeneratorSceneTracker* m_sceneTracker;
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
