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

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "COgreRootSetup: Configuration loaded with " << m_renderSystems.size()
        << " render systems and " << m_resourceGroups.size() << " resource groups" );
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
