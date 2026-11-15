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

#include <algorithm>

#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreDefaultWorkQueue.h"
#include "OgreResourceGroupManager.h"
#include "OgreException.h"
#include "OgreConfigOptionMap.h"
#include "OgreMaterialManager.h"
#include "OgreShaderGenerator.h"

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGRETASKWORKQUEUE_H
#include "pluginglueOGRE_COgreTaskWorkQueue.h"
#define GUCEF_PLUGINGLUE_OGRE_COGRETASKWORKQUEUE_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGRETASKWORKQUEUE_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGRESHADERTECHNIQUERESOLVER_H
#include "pluginglueOGRE_COgreShaderTechniqueResolver.h"
#define GUCEF_PLUGINGLUE_OGRE_COGRESHADERTECHNIQUERESOLVER_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGRESHADERTECHNIQUERESOLVER_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGRESHADERGENERATORSCENETRACKER_H
#include "pluginglueOGRE_COgreShaderGeneratorSceneTracker.h"
#define GUCEF_PLUGINGLUE_OGRE_COGRESHADERGENERATORSCENETRACKER_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGRESHADERGENERATORSCENETRACKER_H ? */

#include "pluginglueOGRE_COgreRootSetup.h"

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
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString COgreRootSetup::DefaultArchiveType = "GucefVFS";
const CORE::CString COgreRootSetup::WorkQueueTypeGucef = "Gucef";
const CORE::CString COgreRootSetup::WorkQueueTypeOgre = "Ogre";
const CORE::CString COgreRootSetup::OgreMediaModuleName = "Ogre";
const CORE::CString COgreRootSetup::RTShaderMediaModuleName = "OgreRTShaderSystem";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreRootSetup::COgreRootSetup( void )
    : CORE::CGloballyConfigurable( false )
    , m_root( GUCEF_NULL )
    , m_ownsRoot( false )
    , m_renderSystems()
    , m_resourceGroups()
    , m_workQueueType( WorkQueueTypeGucef )
    , m_workerPoolName( COgreWorkQueueCore::DefaultThreadPoolName )
    , m_workerThreadCount( COgreWorkQueueCore::DefaultWorkerThreadCount )
    , m_mainThreadBudgetInMs( COgreWorkQueueCore::DefaultMainThreadBudgetInMs )
    , m_workersCanAccessRenderSystem( 1 == OGRE_THREAD_SUPPORT )
    , m_taskWorkQueue( GUCEF_NULL )
    , m_shaderGeneratorEnabled( true )
    , m_shaderCachePath()
    , m_internalResourcesDeclared( false )
    , m_techniqueResolver( GUCEF_NULL )
    , m_sceneTracker( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COgreRootSetup::~COgreRootSetup()
{GUCEF_TRACE;

    UnregisterFromConfigStore();
    DestroyRoot();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
COgreRootSetup::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PLUGINGLUE::OGRE::COgreRootSetup";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
COgreRootSetup::CreateRoot( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_root )
        return true;

    m_root = Ogre::Root::getSingletonPtr();
    if ( GUCEF_NULL != m_root )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Using the Ogre Root that exists already" );
        m_ownsRoot = false;
        return true;
    }

    try
    {
        // no plugin, configuration and log file: nothing is read from or written to the file system by Ogre
        m_root = new Ogre::Root( "", "", "" );
        m_ownsRoot = true;

        // before anything can give the Root background work
        InstallWorkQueue();
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Failed to make the Ogre Root: " << e.getFullDescription() );
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Failed to make the Ogre Root: unknown exception" );
    }
    m_root = GUCEF_NULL;
    return false;
}

/*-------------------------------------------------------------------------*/

void
COgreRootSetup::DestroyRoot( void )
{GUCEF_TRACE;

    // the generator holds materials, programs and scene managers of the Root
    StopShaderGenerator();

    if ( m_ownsRoot && GUCEF_NULL != m_root )
    {
        try
        {
            delete m_root;
        }
        catch ( ... )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Exception while destroying the Ogre Root" );
        }
    }
    m_root = GUCEF_NULL;
    m_ownsRoot = false;
    m_taskWorkQueue = GUCEF_NULL;
    m_internalResourcesDeclared = false;
}

/*-------------------------------------------------------------------------*/

Ogre::Root*
COgreRootSetup::GetRoot( void ) const
{GUCEF_TRACE;

    return m_root;
}

/*-------------------------------------------------------------------------*/

bool
COgreRootSetup::IsRootOwned( void ) const
{GUCEF_TRACE;

    return m_ownsRoot;
}

/*-------------------------------------------------------------------------*/

bool
COgreRootSetup::InitialiseRoot( void )
{GUCEF_TRACE;

    return InitialiseRoot( CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

CORE::CString
COgreRootSetup::GetRendererName( const CORE::CString& renderSystemName )
{GUCEF_TRACE;

    struct SName { const char* renderSystem; const char* renderer; };
    static const SName names[] = { { "Vulkan Rendering Subsystem", "Vulkan" },
                                   { "OpenGL 3+ Rendering Subsystem", "OpenGL" },
                                   { "OpenGL ES 2.x Rendering Subsystem", "OpenGL ES" },
                                   { "Direct3D11 Rendering Subsystem", "Direct3D 11" },
                                   { "Direct3D9 Rendering Subsystem", "Direct3D 9" },
                                   { "OpenGL Rendering Subsystem", "OpenGL (legacy)" } };
    for ( UInt32 i=0; i<(UInt32)( sizeof( names ) / sizeof( names[ 0 ] ) ); ++i )
        if ( renderSystemName == names[ i ].renderSystem )
            return names[ i ].renderer;
    return renderSystemName;
}

/*-------------------------------------------------------------------------*/

bool
COgreRootSetup::IsRenderSystemUsable( const Ogre::RenderSystem& renderSystem )
{GUCEF_TRACE;

    if ( "Vulkan Rendering Subsystem" != renderSystem.getName() )
        return true;

    // the Vulkan render system lists the devices it found as the possible values of its option "Device": none without a
    // Vulkan driver for a GPU that is there, and no option at all when there is no Vulkan loader or no Vulkan instance
    const Ogre::ConfigOptionMap& options = renderSystem.getConfigOptions();
    Ogre::ConfigOptionMap::const_iterator device = options.find( "Device" );
    return device != options.end() && !(*device).second.possibleValues.empty();
}

/*-------------------------------------------------------------------------*/

Ogre::RenderSystem*
COgreRootSetup::FindRenderSystem( const CORE::CString& rendererName ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_root || rendererName.IsNULLOrEmpty() )
        return GUCEF_NULL;

    const Ogre::RenderSystemList& available = m_root->getAvailableRenderers();
    Ogre::RenderSystemList::const_iterator i = available.begin();
    while ( i != available.end() )
    {
        const CORE::CString renderSystemName( (*i)->getName().c_str() );
        if ( renderSystemName.Equals( rendererName, false ) || GetRendererName( renderSystemName ).Equals( rendererName, false ) )
            return (*i);
        ++i;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
COgreRootSetup::GetUsableRenderSystems( GUCEF::vector< Ogre::RenderSystem* >& renderSystems ) const
{GUCEF_TRACE;

    renderSystems.clear();
    if ( GUCEF_NULL == m_root )
        return;

    if ( m_renderSystems.empty() )
    {
        // the application named none: whatever the loaded plugins provide
        const Ogre::RenderSystemList& available = m_root->getAvailableRenderers();
        Ogre::RenderSystemList::const_iterator i = available.begin();
        while ( i != available.end() )
        {
            if ( IsRenderSystemUsable( *(*i) ) )
                renderSystems.push_back( (*i) );
            ++i;
        }
        return;
    }

    CORE::CString::StringVector::const_iterator i = m_renderSystems.begin();
    while ( i != m_renderSystems.end() )
    {
        Ogre::RenderSystem* renderSystem = FindRenderSystem( (*i) );
        if ( GUCEF_NULL != renderSystem && IsRenderSystemUsable( *renderSystem ) &&
             renderSystems.end() == std::find( renderSystems.begin(), renderSystems.end(), renderSystem ) )
            renderSystems.push_back( renderSystem );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
COgreRootSetup::GetAvailableRenderers( CORE::CString::StringVector& rendererNames ) const
{GUCEF_TRACE;

    rendererNames.clear();
    GUCEF::vector< Ogre::RenderSystem* > renderSystems;
    GetUsableRenderSystems( renderSystems );
    for ( UInt32 i=0; i<(UInt32)renderSystems.size(); ++i )
        rendererNames.push_back( GetRendererName( renderSystems[ i ]->getName().c_str() ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
COgreRootSetup::GetActiveRenderer( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_root || GUCEF_NULL == m_root->getRenderSystem() )
        return CORE::CString::Empty;
    return GetRendererName( m_root->getRenderSystem()->getName().c_str() );
}

/*-------------------------------------------------------------------------*/

bool
COgreRootSetup::InitialiseRoot( const CORE::CString& preferredRenderer )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_root )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: There is no Ogre Root to initialise" );
        return false;
    }
    if ( m_root->isInitialised() )
    {
        if ( !preferredRenderer.IsNULLOrEmpty() && GUCEF_NULL != m_root->getRenderSystem() &&
             FindRenderSystem( preferredRenderer ) != m_root->getRenderSystem() )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: The Ogre Root draws with " << GetActiveRenderer()
                << " already, the renderer " << preferredRenderer << " can be used from the next start" );
        }
        return true;
    }

    // the queue of the configuration, before the Root starts it
    InstallWorkQueue();

    try
    {
        GUCEF::vector< Ogre::RenderSystem* > usable;
        GetUsableRenderSystems( usable );

        Ogre::RenderSystem* renderSystem = GUCEF_NULL;
        if ( !preferredRenderer.IsNULLOrEmpty() )
        {
            Ogre::RenderSystem* asked = FindRenderSystem( preferredRenderer );
            if ( GUCEF_NULL == asked )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: The renderer " << preferredRenderer
                    << " is not there: no loaded Ogre plugin provides it. The configured order is used" );
            }
            else
            if ( !IsRenderSystemUsable( *asked ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: The renderer " << preferredRenderer
                    << " cannot draw on this system (for Vulkan: no Vulkan driver for a GPU that is there). The configured order is used" );
            }
            else
            if ( usable.end() == std::find( usable.begin(), usable.end(), asked ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: The renderer " << preferredRenderer
                    << " is not one of the render systems of the configuration. The configured order is used" );
            }
            else
            {
                renderSystem = asked;
            }
        }
        if ( GUCEF_NULL == renderSystem && !usable.empty() )
            renderSystem = usable.front();

        if ( GUCEF_NULL == renderSystem )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: None of the configured render systems is available, "
                "load the Ogre render system plugin of the platform" );
            return false;
        }

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Using render system " << renderSystem->getName() );
        m_root->setRenderSystem( renderSystem );
        m_root->initialise( false );
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Failed to initialise the Ogre Root: " << e.getFullDescription() );
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Failed to initialise the Ogre Root: unknown exception" );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
COgreRootSetup::DeclareResourceGroups( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_root )
        return 0;

    UInt32 locationsAdded = 0;
    Ogre::ResourceGroupManager& groupManager = Ogre::ResourceGroupManager::getSingleton();
    TResourceGroupVector::const_iterator i = m_resourceGroups.begin();
    while ( i != m_resourceGroups.end() )
    {
        const SResourceGroup& group = (*i);
        try
        {
            const Ogre::String groupName = group.name.C_String();
            if ( !groupManager.resourceGroupExists( groupName ) )
                groupManager.createResourceGroup( groupName );

            const Ogre::String archiveType = group.archiveType.IsNULLOrEmpty() ? DefaultArchiveType.C_String() : group.archiveType.C_String();
            groupManager.addResourceLocation( group.path.C_String(), archiveType, groupName, group.recursive );
            ++locationsAdded;
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Failed to declare resource group " << group.name
                << " with location " << group.path << ": " << e.getFullDescription() );
        }
        ++i;
    }
    return locationsAdded;
}

/*-------------------------------------------------------------------------*/

bool
COgreRootSetup::DeclareInternalResources( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_root )
        return false;
    if ( m_internalResourcesDeclared )
        return true;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    Ogre::ResourceGroupManager& groupManager = Ogre::ResourceGroupManager::getSingleton();
    const CORE::CString modules[ 2 ] = { OgreMediaModuleName, RTShaderMediaModuleName };
    bool totalSuccess = true;
    for ( UInt32 i=0; i<2; ++i )
    {
        if ( !vfs.MountModuleData( modules[ i ] ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: The data of the module " << modules[ i ]
                << " cannot be mounted: the build copies it to data/" << modules[ i ] << " next to the modules" );
            totalSuccess = false;
            continue;
        }
        try
        {
            groupManager.addResourceLocation( modules[ i ].C_String(), DefaultArchiveType.C_String(), Ogre::RGN_INTERNAL, true );
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: The data of the module " << modules[ i ]
                << " cannot be added to Ogre's internal resources (is OgrePlugin_GucefVFS loaded?): " << e.getFullDescription() );
            totalSuccess = false;
        }
    }
    m_internalResourcesDeclared = totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
COgreRootSetup::StartShaderGenerator( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_techniqueResolver )
        return true;
    if ( !m_shaderGeneratorEnabled )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: The run time shader system is disabled by the configuration" );
        return true;
    }
    if ( GUCEF_NULL == m_root || !m_root->isInitialised() || GUCEF_NULL == m_root->getRenderSystem() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: The run time shader system needs an initialised Root with a render system" );
        return false;
    }
    if ( GUCEF_NULL != Ogre::RTShader::ShaderGenerator::getSingletonPtr() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: A shader generator exists already, made by something else: it is left to its owner" );
        return false;
    }

    if ( !DeclareInternalResources() )
        return false;

    try
    {
        if ( !Ogre::RTShader::ShaderGenerator::initialize() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: The shader generator could not be made" );
            return false;
        }
        Ogre::RTShader::ShaderGenerator& shaderGenerator = Ogre::RTShader::ShaderGenerator::getSingleton();
        if ( "null" == shaderGenerator.getTargetLanguage() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: The render system " << m_root->getRenderSystem()->getName()
                << " has no shader language for the shader generator yet: start it after the first render window was made" );
            Ogre::RTShader::ShaderGenerator::destroy();
            return false;
        }

        if ( !m_shaderCachePath.IsNULLOrEmpty() )
        {
            if ( !CORE::CreateDirs( m_shaderCachePath ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: The shader cache folder " << m_shaderCachePath
                    << " cannot be made, the generated programs are not cached" );
            }
            else
            {
                Ogre::String cachePath = m_shaderCachePath.C_String();
                if ( '/' != cachePath[ cachePath.size()-1 ] && '\\' != cachePath[ cachePath.size()-1 ] )
                    cachePath += '/';
                shaderGenerator.setShaderCachePath( cachePath );
            }
        }

        m_techniqueResolver = new COgreShaderTechniqueResolver( shaderGenerator );
        Ogre::MaterialManager::getSingleton().addListener( m_techniqueResolver );

        // the script of the RTSS samplers and the shader library are read when the group is initialised; the
        // generator's script translator is registered now, so this comes after it
        Ogre::ResourceGroupManager& groupManager = Ogre::ResourceGroupManager::getSingleton();
        if ( !groupManager.isResourceGroupInitialised( Ogre::RGN_INTERNAL ) )
        {
            groupManager.initialiseResourceGroup( Ogre::RGN_INTERNAL );
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Ogre's internal resource group was initialised before the shader "
                "generator started: the scripts of the shader library were not parsed" );
        }

        m_sceneTracker = new COgreShaderGeneratorSceneTracker( *m_root, shaderGenerator );
        m_sceneTracker->Start();

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Started the run time shader system with the shader language "
            << shaderGenerator.getTargetLanguage() << " on " << m_root->getRenderSystem()->getName() );
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Failed to start the run time shader system: " << e.getFullDescription() );
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Failed to start the run time shader system: unknown exception" );
    }
    StopShaderGenerator();
    if ( GUCEF_NULL != Ogre::RTShader::ShaderGenerator::getSingletonPtr() )
        Ogre::RTShader::ShaderGenerator::destroy();
    return false;
}

/*-------------------------------------------------------------------------*/

void
COgreRootSetup::StopShaderGenerator( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_techniqueResolver && GUCEF_NULL == m_sceneTracker )
        return;

    try
    {
        if ( GUCEF_NULL != m_sceneTracker )
        {
            m_sceneTracker->Stop();
            delete m_sceneTracker;
            m_sceneTracker = GUCEF_NULL;
        }
        if ( GUCEF_NULL != m_techniqueResolver )
        {
            Ogre::MaterialManager* materialManager = Ogre::MaterialManager::getSingletonPtr();
            if ( GUCEF_NULL != materialManager )
            {
                materialManager->removeListener( m_techniqueResolver );
                materialManager->setActiveScheme( Ogre::MaterialManager::DEFAULT_SCHEME_NAME );
            }
            delete m_techniqueResolver;
            m_techniqueResolver = GUCEF_NULL;
        }
        if ( GUCEF_NULL != Ogre::RTShader::ShaderGenerator::getSingletonPtr() )
            Ogre::RTShader::ShaderGenerator::destroy();
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Stopped the run time shader system" );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Exception while stopping the run time shader system: " << e.getFullDescription() );
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Exception while stopping the run time shader system" );
    }
    m_sceneTracker = GUCEF_NULL;
    m_techniqueResolver = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
COgreRootSetup::IsShaderGeneratorStarted( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_techniqueResolver;
}

/*-------------------------------------------------------------------------*/

void
COgreRootSetup::SetShaderGeneratorEnabled( bool enabled )
{GUCEF_TRACE;

    m_shaderGeneratorEnabled = enabled;
}

/*-------------------------------------------------------------------------*/

bool
COgreRootSetup::GetShaderGeneratorEnabled( void ) const
{GUCEF_TRACE;

    return m_shaderGeneratorEnabled;
}

/*-------------------------------------------------------------------------*/

void
COgreRootSetup::SetShaderCachePath( const CORE::CString& path )
{GUCEF_TRACE;

    m_shaderCachePath = path;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
COgreRootSetup::GetShaderCachePath( void ) const
{GUCEF_TRACE;

    return m_shaderCachePath;
}

/*-------------------------------------------------------------------------*/

COgreShaderTechniqueResolver*
COgreRootSetup::GetShaderTechniqueResolver( void ) const
{GUCEF_TRACE;

    return m_techniqueResolver;
}

/*-------------------------------------------------------------------------*/

COgreShaderGeneratorSceneTracker*
COgreRootSetup::GetShaderGeneratorSceneTracker( void ) const
{GUCEF_TRACE;

    return m_sceneTracker;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringVector&
COgreRootSetup::GetRenderSystems( void ) const
{GUCEF_TRACE;

    return m_renderSystems;
}

/*-------------------------------------------------------------------------*/

void
COgreRootSetup::SetRenderSystems( const CORE::CString::StringVector& renderSystems )
{GUCEF_TRACE;

    m_renderSystems = renderSystems;
}

/*-------------------------------------------------------------------------*/

const COgreRootSetup::TResourceGroupVector&
COgreRootSetup::GetResourceGroups( void ) const
{GUCEF_TRACE;

    return m_resourceGroups;
}

/*-------------------------------------------------------------------------*/

void
COgreRootSetup::AddResourceGroup( const SResourceGroup& group )
{GUCEF_TRACE;

    m_resourceGroups.push_back( group );
}

/*-------------------------------------------------------------------------*/

bool
COgreRootSetup::InstallWorkQueue( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_root || !m_ownsRoot )
        return false;

    const bool hasTaskWorkQueue = GUCEF_NULL != m_taskWorkQueue && m_root->getWorkQueue() == m_taskWorkQueue;
    try
    {
        if ( WorkQueueTypeOgre == m_workQueueType )
        {
            if ( !hasTaskWorkQueue && GUCEF_NULL != m_root->getWorkQueue() )
                return true;

            // Ogre's own queue, as the Root makes it, with the threads of the configuration
            Ogre::DefaultWorkQueue* ogreQueue = OGRE_NEW Ogre::DefaultWorkQueue( "Root" );
            ogreQueue->setWorkerThreadCount( 0 == m_workerThreadCount ? 1 : (size_t) m_workerThreadCount );
            ogreQueue->setWorkersCanAccessRenderSystem( m_workersCanAccessRenderSystem );
            ogreQueue->setResponseProcessingTimeLimit( (unsigned long) m_mainThreadBudgetInMs );
            m_taskWorkQueue = GUCEF_NULL;
            m_root->setWorkQueue( ogreQueue );
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: The Root uses Ogre's own work queue" );
            return true;
        }

        COgreTaskWorkQueue* queue = hasTaskWorkQueue ? m_taskWorkQueue : OGRE_NEW COgreTaskWorkQueue();
        queue->SetThreadPoolName( m_workerPoolName );
        queue->setWorkerThreadCount( (size_t) m_workerThreadCount );
        queue->setResponseProcessingTimeLimit( (unsigned long) m_mainThreadBudgetInMs );
        queue->SetWorkersCanAccessRenderSystem( m_workersCanAccessRenderSystem );
        if ( !hasTaskWorkQueue )
        {
            // the Root owns the queue from now on; one that is initialised already starts it at once
            m_taskWorkQueue = queue;
            m_root->setWorkQueue( queue );
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: The Root uses the work queue on GUCEF thread pool " << m_workerPoolName );
        }
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Failed to install the work queue: " << e.getFullDescription() );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

COgreTaskWorkQueue*
COgreRootSetup::GetTaskWorkQueue( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_root && GUCEF_NULL != m_taskWorkQueue && m_root->getWorkQueue() == m_taskWorkQueue )
        return m_taskWorkQueue;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
COgreRootSetup::SetWorkQueueType( const CORE::CString& workQueueType )
{GUCEF_TRACE;

    m_workQueueType = WorkQueueTypeOgre == workQueueType ? WorkQueueTypeOgre : WorkQueueTypeGucef;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
COgreRootSetup::GetWorkQueueType( void ) const
{GUCEF_TRACE;

    return m_workQueueType;
}

/*-------------------------------------------------------------------------*/

void
COgreRootSetup::SetWorkerPoolName( const CORE::CString& poolName )
{GUCEF_TRACE;

    m_workerPoolName = poolName.IsNULLOrEmpty() ? COgreWorkQueueCore::DefaultThreadPoolName : poolName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
COgreRootSetup::GetWorkerPoolName( void ) const
{GUCEF_TRACE;

    return m_workerPoolName;
}

/*-------------------------------------------------------------------------*/

void
COgreRootSetup::SetWorkerThreadCount( UInt32 threadCount )
{GUCEF_TRACE;

    m_workerThreadCount = threadCount;
}

/*-------------------------------------------------------------------------*/

UInt32
COgreRootSetup::GetWorkerThreadCount( void ) const
{GUCEF_TRACE;

    return m_workerThreadCount;
}

/*-------------------------------------------------------------------------*/

void
COgreRootSetup::SetMainThreadBudgetInMs( UInt32 budgetInMs )
{GUCEF_TRACE;

    m_mainThreadBudgetInMs = budgetInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
COgreRootSetup::GetMainThreadBudgetInMs( void ) const
{GUCEF_TRACE;

    return m_mainThreadBudgetInMs;
}

/*-------------------------------------------------------------------------*/

void
COgreRootSetup::SetWorkersCanAccessRenderSystem( bool canAccess )
{GUCEF_TRACE;

    m_workersCanAccessRenderSystem = canAccess;
}

/*-------------------------------------------------------------------------*/

bool
COgreRootSetup::GetWorkersCanAccessRenderSystem( void ) const
{GUCEF_TRACE;

    return m_workersCanAccessRenderSystem;
}

/*-------------------------------------------------------------------------*/

bool
COgreRootSetup::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    const CORE::CDataNode* section = config.Find( "Ogre" );
    if ( GUCEF_NULL == section )
        return true;

    m_renderSystems.clear();
    CORE::CDataNode::TVariantVector renderSystems;
    if ( section->GetValuesOfChildByName( "renderSystems", renderSystems, false ) )
    {
        CORE::CDataNode::TVariantVector::const_iterator i = renderSystems.begin();
        while ( i != renderSystems.end() )
        {
            const CORE::CString name = (*i).AsString( CORE::CString::Empty, true );
            if ( !name.IsNULLOrEmpty() )
                m_renderSystems.push_back( name );
            ++i;
        }
    }

    m_resourceGroups.clear();
    const CORE::CDataNode* groups = section->FindChild( "ResourceGroups" );
    if ( GUCEF_NULL != groups )
    {
        CORE::CDataNode::const_iterator i = groups->ConstBegin();
        while ( i != groups->ConstEnd() )
        {
            const CORE::CDataNode* groupNode = (*i);
            SResourceGroup group;
            group.name = groupNode->GetAttributeValueOrChildValueByName( "name" ).AsString( CORE::CString::Empty, true );
            group.path = groupNode->GetAttributeValueOrChildValueByName( "path" ).AsString( CORE::CString::Empty, true );
            group.recursive = groupNode->GetAttributeValueOrChildValueByName( "recursive" ).AsBool( true, true );
            group.archiveType = groupNode->GetAttributeValueOrChildValueByName( "archiveType" ).AsString( DefaultArchiveType, true );
            if ( group.name.IsNULLOrEmpty() || group.path.IsNULLOrEmpty() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: A resource group of the configuration has no name or no path and is skipped" );
            }
            else
            {
                m_resourceGroups.push_back( group );
            }
            ++i;
        }
    }

    m_workQueueType = section->GetAttributeValueOrChildValueByName( "workQueue" ).AsString( WorkQueueTypeGucef, true );
    if ( WorkQueueTypeGucef != m_workQueueType && WorkQueueTypeOgre != m_workQueueType )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Unknown work queue type " << m_workQueueType << ", " << WorkQueueTypeGucef << " is used" );
        m_workQueueType = WorkQueueTypeGucef;
    }
    m_workerPoolName = section->GetAttributeValueOrChildValueByName( "workerPool" ).AsString( COgreWorkQueueCore::DefaultThreadPoolName, true );
    if ( m_workerPoolName.IsNULLOrEmpty() )
        m_workerPoolName = COgreWorkQueueCore::DefaultThreadPoolName;
    m_workerThreadCount = section->GetAttributeValueOrChildValueByName( "workerThreads" ).AsUInt32( COgreWorkQueueCore::DefaultWorkerThreadCount, true );
    m_mainThreadBudgetInMs = section->GetAttributeValueOrChildValueByName( "mainThreadBudgetMs" ).AsUInt32( COgreWorkQueueCore::DefaultMainThreadBudgetInMs, true );
    m_workersCanAccessRenderSystem = section->GetAttributeValueOrChildValueByName( "workersCanAccessRenderSystem" ).AsBool( 1 == OGRE_THREAD_SUPPORT, true );
    m_shaderGeneratorEnabled = section->GetAttributeValueOrChildValueByName( "shaderGenerator" ).AsBool( true, true );
    m_shaderCachePath = section->GetAttributeValueOrChildValueByName( "shaderCachePath" ).AsString( CORE::CString::Empty, true );
    if ( m_ownsRoot && GUCEF_NULL != m_root )
        InstallWorkQueue();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Configuration loaded with " << m_renderSystems.size()
        << " render systems and " << m_resourceGroups.size() << " resource groups, work queue " << m_workQueueType
        << " on thread pool " << m_workerPoolName << " with " << m_workerThreadCount << " worker threads" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COgreRootSetup::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = config.FindOrAddChild( "Ogre" );
    if ( GUCEF_NULL == section )
        return false;

    bool totalSuccess = true;
    section->DelChild( "renderSystems" );
    section->DelChild( "ResourceGroups" );
    totalSuccess = section->SetValuesOfChildByName( "renderSystems", m_renderSystems ) && totalSuccess;

    totalSuccess = section->SetAttribute( "workQueue", m_workQueueType ) && totalSuccess;
    totalSuccess = section->SetAttribute( "workerPool", m_workerPoolName ) && totalSuccess;
    totalSuccess = section->SetAttribute( "workerThreads", m_workerThreadCount ) && totalSuccess;
    totalSuccess = section->SetAttribute( "mainThreadBudgetMs", m_mainThreadBudgetInMs ) && totalSuccess;
    totalSuccess = section->SetAttribute( "workersCanAccessRenderSystem", m_workersCanAccessRenderSystem ) && totalSuccess;
    totalSuccess = section->SetAttribute( "shaderGenerator", m_shaderGeneratorEnabled ) && totalSuccess;
    totalSuccess = section->SetAttribute( "shaderCachePath", m_shaderCachePath ) && totalSuccess;

    CORE::CDataNode* groups = section->AddChild( "ResourceGroups", GUCEF_DATATYPE_ARRAY );
    if ( GUCEF_NULL == groups )
        return false;
    TResourceGroupVector::const_iterator i = m_resourceGroups.begin();
    while ( i != m_resourceGroups.end() )
    {
        CORE::CDataNode* groupNode = groups->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        if ( GUCEF_NULL == groupNode )
        {
            totalSuccess = false;
        }
        else
        {
            totalSuccess = groupNode->SetAttribute( "name", (*i).name ) && totalSuccess;
            totalSuccess = groupNode->SetAttribute( "path", (*i).path ) && totalSuccess;
            totalSuccess = groupNode->SetAttribute( "recursive", (*i).recursive ) && totalSuccess;
            totalSuccess = groupNode->SetAttribute( "archiveType", (*i).archiveType ) && totalSuccess;
        }
        ++i;
    }
    return totalSuccess;
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
