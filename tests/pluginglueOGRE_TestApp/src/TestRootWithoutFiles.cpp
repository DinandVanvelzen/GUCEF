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

#if defined( _WIN32 )
#include <direct.h>
#define GUCEF_TEST_CHDIR _chdir
#else
#include <unistd.h>
#define GUCEF_TEST_CHDIR chdir
#endif

#include "OgreRoot.h"
#include "OgreArchiveManager.h"
#include "OgreResourceGroupManager.h"
#include "OgreLogManager.h"
#include "OgreException.h"

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

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H
#include "pluginglueOGRE_COgreRootSetup.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H ? */

#include "TestSupport.h"
#include "TestRootWithoutFiles.h"

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

namespace {

/** Works in a directory for as long as it exists, and goes back to the directory it came from */
class CWorkingDirectoryScope
{
    public:

    CWorkingDirectoryScope( const CORE::CString& dir ) : m_previous( CORE::RelativePath( "$CURWORKDIR$" ) ), m_changed( 0 == GUCEF_TEST_CHDIR( dir.C_String() ) ) {}
    ~CWorkingDirectoryScope() { if ( m_changed ) GUCEF_TEST_CHDIR( m_previous.C_String() ); }
    bool Changed( void ) const { return m_changed; }

    private:

    CORE::CString m_previous;
    bool m_changed;
};

/*-------------------------------------------------------------------------*/

/** The files and directories in a directory, below it as well, found through the VFS */
static UInt32
CountEntriesIn( const CORE::CString& dir )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.AddRoot( dir, "ogre_probe", false, false, "/ogre_probe" ) )
        return 0xFFFFFFFF;

    VFS::CVFS::TStringVector files;
    VFS::CVFS::TStringVector dirs;
    vfs.GetFileList( files, "/ogre_probe", true, true, CORE::CString::Empty );
    vfs.GetDirList( dirs, "/ogre_probe", true, true, CORE::CString::Empty );
    vfs.UnmountArchiveByName( "ogre_probe" );
    return (UInt32) ( files.size() + dirs.size() );
}

} // anonymous namespace

/*-------------------------------------------------------------------------*/

static void
TestDefaultRootTouchesNothing( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Root without file names: a default made Root, and one with empty names, read and write no files and have no archive factories that read from disk" )
        try
        {
            ASSERT_TRUE( GUCEF_NULL == Ogre::Root::getSingletonPtr() );
            ASSERT_TRUE( GUCEF_NULL == Ogre::LogManager::getSingletonPtr() );

            // the probe is an empty directory that the Root works in: its default names are relative names
            const CORE::CString probe = MakeCleanTestDir( "root_probe" );
            ASSERT_TRUE( 0 == CountEntriesIn( probe ) );
            // the probe sees what is there
            ASSERT_TRUE( WriteTestFile( probe + "/control.txt", "control" ) );
            ASSERT_TRUE( 1 == CountEntriesIn( probe ) );
            ASSERT_TRUE( CORE::DeleteFile( probe + "/control.txt" ) );
            ASSERT_TRUE( 0 == CountEntriesIn( probe ) );
            {
                CWorkingDirectoryScope scope( probe );
                ASSERT_TRUE( scope.Changed() );

                // the constructor with its defaults: the names of the files Ogre would read and write are empty
                {
                    Ogre::Root root;
                    ASSERT_TRUE( GUCEF_NULL != Ogre::LogManager::getSingletonPtr() );
                    Ogre::ArchiveManager& archiveManager = Ogre::ArchiveManager::getSingleton();
                    ASSERT_FALSE( archiveManager.hasArchiveFactory( "FileSystem" ) );
                    ASSERT_FALSE( archiveManager.hasArchiveFactory( "Zip" ) );
                    // the embedded ZIP factory only reads from memory and stays
                    ASSERT_TRUE( archiveManager.hasArchiveFactory( "EmbeddedZip" ) );

                    // asking for the archive types that read from disk is an error, not a read
                    bool refused = false;
                    try
                    {
                        Ogre::ResourceGroupManager::getSingleton().addResourceLocation( ".", "FileSystem", "probe_group" );
                    }
                    catch ( const Ogre::Exception& )
                    {
                        refused = true;
                    }
                    ASSERT_TRUE( refused );

                    // the Root works without renderers and plugins
                    ASSERT_TRUE( root.getAvailableRenderers().empty() );
                    ASSERT_FALSE( root.isInitialised() );
                    // saving and restoring the configuration with no file name does nothing
                    root.saveConfig();
                    ASSERT_TRUE( root.restoreConfig() );
                }
                ASSERT_TRUE( GUCEF_NULL == Ogre::Root::getSingletonPtr() );
                ASSERT_TRUE( GUCEF_NULL == Ogre::LogManager::getSingletonPtr() );
                ASSERT_TRUE( 0 == CountEntriesIn( probe ) );

                // the form of the support library, with the empty names written out
                {
                    PLUGINGLUE::OGRE::COgreRootSetup setup;
                    ASSERT_TRUE( setup.CreateRoot() );
                    ASSERT_TRUE( setup.IsRootOwned() );
                    ASSERT_TRUE( GUCEF_NULL != setup.GetRoot() );
                    ASSERT_FALSE( Ogre::ArchiveManager::getSingleton().hasArchiveFactory( "FileSystem" ) );
                    // a second call keeps the Root
                    Ogre::Root* root = setup.GetRoot();
                    ASSERT_TRUE( setup.CreateRoot() && root == setup.GetRoot() );
                    setup.DestroyRoot();
                    ASSERT_TRUE( GUCEF_NULL == Ogre::Root::getSingletonPtr() );
                }
                ASSERT_TRUE( 0 == CountEntriesIn( probe ) );

                // a Root that someone else made is used and left alone
                {
                    Ogre::Root root( "", "", "" );
                    PLUGINGLUE::OGRE::COgreRootSetup setup;
                    ASSERT_TRUE( setup.CreateRoot() );
                    ASSERT_FALSE( setup.IsRootOwned() );
                    ASSERT_TRUE( &root == setup.GetRoot() );
                    setup.DestroyRoot();
                    ASSERT_TRUE( &root == Ogre::Root::getSingletonPtr() );
                }
                ASSERT_TRUE( 0 == CountEntriesIn( probe ) );
            }
            ASSERT_TRUE( 0 == CountEntriesIn( probe ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestRootInitialisationWithoutRenderSystem( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Root setup: initialising without a render system fails with a reason instead of an exception, and the configuration of the Ogre section is read and written back" )
        try
        {
            PLUGINGLUE::OGRE::COgreRootSetup setup;
            ASSERT_TRUE( setup.CreateRoot() );
            ASSERT_FALSE( setup.InitialiseRoot() );
            ASSERT_FALSE( setup.GetRoot()->isInitialised() );

            // the configuration
            CORE::CDataNode config;
            CORE::CDataNode* section = config.AddChild( "Ogre" );
            ASSERT_TRUE( GUCEF_NULL != section );
            CORE::CString::StringVector renderSystems;
            renderSystems.push_back( "OpenGL 3+ Rendering Subsystem" );
            renderSystems.push_back( "Missing Rendering Subsystem" );
            ASSERT_TRUE( section->SetValuesOfChildByName( "renderSystems", renderSystems ) );
            CORE::CDataNode* groups = section->AddChild( "ResourceGroups", GUCEF_DATATYPE_ARRAY );
            CORE::CDataNode* first = groups->AddChild( "", GUCEF_DATATYPE_OBJECT );
            first->SetAttribute( "name", "Essential" );
            first->SetAttribute( "path", "/data/renderdriver" );
            CORE::CDataNode* second = groups->AddChild( "", GUCEF_DATATYPE_OBJECT );
            second->SetAttribute( "name", "Assets" );
            second->SetAttribute( "path", "/assets" );
            second->SetAttribute( "recursive", false );
            second->SetAttribute( "archiveType", "FileSystem" );
            groups->AddChild( "", GUCEF_DATATYPE_OBJECT )->SetAttribute( "name", "NoPath" );

            ASSERT_TRUE( setup.LoadConfig( config ) );
            ASSERT_TRUE( 2 == setup.GetRenderSystems().size() );
            ASSERT_TRUE( 2 == setup.GetResourceGroups().size() );
            if ( 2 == setup.GetRenderSystems().size() && 2 == setup.GetResourceGroups().size() )
            {
                ASSERT_TRUE( "OpenGL 3+ Rendering Subsystem" == setup.GetRenderSystems()[ 0 ] );
                ASSERT_TRUE( "Essential" == setup.GetResourceGroups()[ 0 ].name );
                ASSERT_TRUE( "/data/renderdriver" == setup.GetResourceGroups()[ 0 ].path );
                // defaults: recursive, archive type of the VFS
                ASSERT_TRUE( setup.GetResourceGroups()[ 0 ].recursive );
                ASSERT_TRUE( "GucefVFS" == setup.GetResourceGroups()[ 0 ].archiveType );
                ASSERT_FALSE( setup.GetResourceGroups()[ 1 ].recursive );
                ASSERT_TRUE( "FileSystem" == setup.GetResourceGroups()[ 1 ].archiveType );
            }

            // written back it reads the same
            CORE::CDataNode saved;
            ASSERT_TRUE( setup.SaveConfig( saved ) );
            PLUGINGLUE::OGRE::COgreRootSetup other;
            ASSERT_TRUE( other.LoadConfig( saved ) );
            ASSERT_TRUE( 2 == other.GetRenderSystems().size() );
            ASSERT_TRUE( 2 == other.GetResourceGroups().size() );

            // no render system of the list is there: initialising fails and says so
            ASSERT_FALSE( setup.InitialiseRoot() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformRootWithoutFilesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "pluginglueOGRE.RootWithoutFiles" );

    TestDefaultRootTouchesNothing();
    TestRootInitialisationWithoutRenderSystem();
}

/*-------------------------------------------------------------------------*/
