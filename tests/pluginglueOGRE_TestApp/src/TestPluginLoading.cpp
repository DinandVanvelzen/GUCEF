/*
 *  pluginglueOGRE_TestApp: Test application for the Ogre glue modules (pluginglueOGRE, OgrePlugin_GucefVFS, OgreCodec_GucefIMAGE)
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
#include "OgreArchiveManager.h"
#include "OgreCodec.h"
#include "OgreLogManager.h"
#include "OgrePlugin.h"

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_PLUGINAPI_H
#include "pluginglueOGRE_pluginAPI.h"
#define GUCEF_PLUGINGLUE_OGRE_PLUGINAPI_H
#endif /* GUCEF_PLUGINGLUE_OGRE_PLUGINAPI_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_H
#include "pluginglueOGRE.h"
#define GUCEF_PLUGINGLUE_OGRE_H
#endif /* GUCEF_PLUGINGLUE_OGRE_H ? */

#include "TestPluginLoading.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const CORE::CString OgrePluginGroup = "ogreplugintests";

/*-------------------------------------------------------------------------*/

/** The entry of a plugin in the bootstrap configuration, as the .ini of an application makes it */
static void
AddBootstrapEntry( CORE::CDataNode& group, const CORE::CString& pluginName, bool loadFailAllowed )
{GUCEF_TRACE;

    CORE::CDataNode* entry = group.AddChild( "PluginMetaData" );
    entry->SetAttribute( "Description", pluginName );
    entry->SetAttribute( "Path", "$MODULEDIR$" );
    entry->SetAttribute( "Filename", pluginName );
    entry->SetAttribute( "AltFilename", pluginName + "_d" );
    entry->SetAttribute( "Type", "OgrePlugin" );
    entry->SetAttribute( "LoadFailAllowed", loadFailAllowed );
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

static void
TestLoadingOgrePlugins( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Plugin loading: the support library makes the Ogre systems, and the Ogre plugins load as plugins of the type OgrePlugin into the Root" )
        try
        {
            // the library makes the log adapter, the Root and the plugin manager when it loads
            ASSERT_TRUE( GUCEF_NULL == Ogre::Root::getSingletonPtr() );
            ASSERT_TRUE( 1 == PLUGINGLUE::OGRE::GUCEFPlugin_Load( 0, GUCEF_NULL ) );
            ASSERT_TRUE( GUCEF_NULL != Ogre::Root::getSingletonPtr() );
            ASSERT_TRUE( GUCEF_NULL != Ogre::LogManager::getSingletonPtr() );
            PLUGINGLUE::OGRE::COgreGlobal* global = PLUGINGLUE::OGRE::COgreGlobal::Instance();
            ASSERT_TRUE( "OgrePlugin" == global->GetPluginManager().GetPluginType() );
            ASSERT_TRUE( global->GetRootSetup().IsRootOwned() );
            ASSERT_TRUE( global->GetRootSetup().GetRoot() == Ogre::Root::getSingletonPtr() );
            ASSERT_TRUE( 0 == global->GetRootSetup().GetRoot()->getAvailableRenderers().size() );
            // and again: the same systems
            ASSERT_TRUE( 1 == PLUGINGLUE::OGRE::GUCEFPlugin_Load( 0, GUCEF_NULL ) );
            ASSERT_TRUE( global == PLUGINGLUE::OGRE::COgreGlobal::Instance() );

            // nothing yet of what the plugins add
            Ogre::ArchiveManager& archives = Ogre::ArchiveManager::getSingleton();
            ASSERT_FALSE( archives.hasArchiveFactory( "GucefVFS" ) );
            ASSERT_FALSE( archives.hasArchiveFactory( "GucefIOAccess" ) );
            ASSERT_FALSE( archives.hasArchiveFactory( "FileSystem" ) );
            ASSERT_FALSE( archives.hasArchiveFactory( "Zip" ) );
            ASSERT_FALSE( Ogre::Codec::isCodecRegistered( "ogtcodectest" ) );

            // the bootstrap configuration of an application: the plugins in the order they are listed. Entries that
            // may fail do not fail the configuration: a module that is not there, and one that is not an Ogre plugin
            CORE::CDataNode config;
            CORE::CDataNode* group = config.AddChild( "PluginGroup" );
            group->SetAttribute( "GroupName", OgrePluginGroup );
            group->SetAttribute( "LoadImmediately", true );
            AddBootstrapEntry( *group, "OgrePlugin_DoesNotExist", true );
            AddBootstrapEntry( *group, "gucefIMAGE", true );
            AddBootstrapEntry( *group, "OgrePlugin_GucefVFS", false );
            AddBootstrapEntry( *group, "OgreCodec_GucefIMAGE", false );
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().LoadConfig( config ) );

            ASSERT_TRUE( archives.hasArchiveFactory( "GucefVFS" ) );
            ASSERT_TRUE( archives.hasArchiveFactory( "GucefIOAccess" ) );
            ASSERT_TRUE( archives.hasArchiveFactory( "FileSystem" ) );
            ASSERT_TRUE( archives.hasArchiveFactory( "Zip" ) );
            ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "png" ) );

            // the plugins are installed in the Root
            const Ogre::Root::PluginInstanceList& installed = Ogre::Root::getSingleton().getInstalledPlugins();
            bool hasArchivePlugin = false;
            bool hasCodecPlugin = false;
            for ( size_t i=0; i<installed.size(); ++i )
            {
                hasArchivePlugin = hasArchivePlugin || "GucefVFS Archives" == installed[ i ]->getName();
                hasCodecPlugin = hasCodecPlugin || "GucefIMAGE Codecs" == installed[ i ]->getName();
            }
            ASSERT_TRUE( hasArchivePlugin && hasCodecPlugin );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestStaticPluginRegistry( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Plugin loading: Ogre plugin objects that an application hands over are found by the name of their module, for platforms that link the plugins in" )
        try
        {
            using PLUGINGLUE::OGRE::COgrePluginManager;
            ASSERT_TRUE( GUCEF_NULL == COgrePluginManager::GetStaticOgrePlugin( "OgrePlugin_Test" ) );
            ASSERT_FALSE( COgrePluginManager::RegisterStaticOgrePlugin( "", GUCEF_NULL ) );

            Ogre::Plugin* marker = reinterpret_cast< Ogre::Plugin* >( (size_t) 16 );
            ASSERT_TRUE( COgrePluginManager::RegisterStaticOgrePlugin( "OgrePlugin_Test", marker ) );
            ASSERT_FALSE( COgrePluginManager::RegisterStaticOgrePlugin( "OgrePlugin_Test", marker ) );
            ASSERT_TRUE( marker == COgrePluginManager::GetStaticOgrePlugin( "OgrePlugin_Test" ) );
            ASSERT_TRUE( COgrePluginManager::UnregisterStaticOgrePlugin( "OgrePlugin_Test" ) );
            ASSERT_FALSE( COgrePluginManager::UnregisterStaticOgrePlugin( "OgrePlugin_Test" ) );
            ASSERT_TRUE( GUCEF_NULL == COgrePluginManager::GetStaticOgrePlugin( "OgrePlugin_Test" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformPluginLoadingTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "pluginglueOGRE.PluginLoading" );

    TestLoadingOgrePlugins();
    TestStaticPluginRegistry();
}

/*-------------------------------------------------------------------------*/

void
PerformPluginUnloadingTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "pluginglueOGRE.PluginUnloading" );

    GUCEF_TESTFW_TESTCASE( "Plugin unloading: unloading the plugins takes their archive factories and codecs out of Ogre, and unloading the support library takes the Root and the log away" )
        try
        {
            Ogre::ArchiveManager& archives = Ogre::ArchiveManager::getSingleton();
            ASSERT_TRUE( archives.hasArchiveFactory( "GucefVFS" ) );

            // the resource groups of the tests are gone, but Ogre keeps the archives of their locations loaded
            ASSERT_TRUE( archives.getArchiveIterator().hasMoreElements() );

            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( OgrePluginGroup ) );
            // the plugin took the archives of its types with it, which are code of the plugin
            ASSERT_FALSE( archives.getArchiveIterator().hasMoreElements() );
            ASSERT_FALSE( archives.hasArchiveFactory( "GucefVFS" ) );
            ASSERT_FALSE( archives.hasArchiveFactory( "GucefIOAccess" ) );
            ASSERT_FALSE( archives.hasArchiveFactory( "FileSystem" ) );
            ASSERT_FALSE( archives.hasArchiveFactory( "Zip" ) );
            ASSERT_TRUE( archives.hasArchiveFactory( "EmbeddedZip" ) );
            ASSERT_FALSE( Ogre::Codec::isCodecRegistered( "png" ) );
            ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "dds" ) );

            PLUGINGLUE::OGRE::GUCEFPlugin_Unload();
            ASSERT_TRUE( GUCEF_NULL == Ogre::Root::getSingletonPtr() );
            ASSERT_TRUE( GUCEF_NULL == Ogre::LogManager::getSingletonPtr() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
