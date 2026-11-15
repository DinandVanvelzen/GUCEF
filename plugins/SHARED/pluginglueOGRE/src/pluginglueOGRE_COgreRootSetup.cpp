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
#include "OgreRenderSystem.h"
#include "OgreDefaultWorkQueue.h"
#include "OgreResourceGroupManager.h"
#include "OgreException.h"

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

    if ( GUCEF_NULL == m_root )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: There is no Ogre Root to initialise" );
        return false;
    }
    if ( m_root->isInitialised() )
        return true;

    // the queue of the configuration, before the Root starts it
    InstallWorkQueue();

    try
    {
        Ogre::RenderSystem* renderSystem = GUCEF_NULL;
        CORE::CString::StringVector::const_iterator i = m_renderSystems.begin();
        while ( GUCEF_NULL == renderSystem && i != m_renderSystems.end() )
        {
            renderSystem = m_root->getRenderSystemByName( (*i).C_String() );
            ++i;
        }

        if ( GUCEF_NULL == renderSystem && m_renderSystems.empty() )
        {
            // the application named none: whatever the loaded plugins provide
            const Ogre::RenderSystemList& available = m_root->getAvailableRenderers();
            if ( !available.empty() )
                renderSystem = available.front();
        }

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
