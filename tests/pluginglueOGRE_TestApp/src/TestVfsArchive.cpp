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

#include <time.h>

#include "OgreArchiveManager.h"
#include "OgreResourceGroupManager.h"
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

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSARCHIVE_H
#include "pluginglueOGRECOMMON_COgreVfsArchive.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSARCHIVE_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSARCHIVE_H ? */

#include "TestSupport.h"
#include "TestVfsArchive.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::PLUGINGLUE::OGRECOMMON;

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

typedef GUCEF::set< Ogre::String > TNameSet;

/*-------------------------------------------------------------------------*/

static TNameSet
ToSet( const Ogre::StringVectorPtr& names )
{GUCEF_TRACE;

    TNameSet result;
    if ( names )
    {
        for ( size_t i=0; i<names->size(); ++i )
            result.insert( (*names)[ i ] );
        // names are not listed twice
        if ( result.size() != names->size() )
            result.insert( "<duplicates>" );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

static TNameSet
MakeSet( const char* a = GUCEF_NULL, const char* b = GUCEF_NULL, const char* c = GUCEF_NULL, const char* d = GUCEF_NULL )
{GUCEF_TRACE;

    TNameSet result;
    if ( GUCEF_NULL != a ) result.insert( a );
    if ( GUCEF_NULL != b ) result.insert( b );
    if ( GUCEF_NULL != c ) result.insert( c );
    if ( GUCEF_NULL != d ) result.insert( d );
    return result;
}

/*-------------------------------------------------------------------------*/

/**
 *  The VFS content the tests work on: a directory of the file system mounted at /assets, another one mounted at
 *  /other, which only has a marker file. Nothing of it is on the file system under the path /assets
 */
class CVfsFixture
{
    public:

    CVfsFixture( void )
        : m_assetsDir( MakeCleanTestDir( "vfs_assets" ) )
        , m_otherDir( MakeCleanTestDir( "vfs_other" ) )
        , m_ok( false )
    {GUCEF_TRACE;

        m_ok = WriteTestFile( m_assetsDir + "/a.txt", "alpha" ) &&
               WriteTestFile( m_assetsDir + "/Readme.MD", "readme text here" ) &&
               WriteTestFile( m_assetsDir + "/sub/b.txt", "bravo-bravo" ) &&
               WriteTestFile( m_assetsDir + "/sub/deep/c.dat", "ccc" ) &&
               WriteTestFile( m_otherDir + "/marker.txt", "marker" );

        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        m_ok = m_ok && vfs.AddRoot( m_assetsDir, "ogre_test_assets", false, false, "/assets" );
        m_ok = m_ok && vfs.AddRoot( m_otherDir, "ogre_test_other", true, false, "/other" );
    }

    ~CVfsFixture()
    {GUCEF_TRACE;

        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        vfs.UnmountArchiveByName( "ogre_test_assets" );
        vfs.UnmountArchiveByName( "ogre_test_other" );
    }

    bool IsOk( void ) const { return m_ok; }
    const CORE::CString& GetAssetsDir( void ) const { return m_assetsDir; }

    private:

    CORE::CString m_assetsDir;
    CORE::CString m_otherDir;
    bool m_ok;
};

} // anonymous namespace

/*-------------------------------------------------------------------------*/

static void
TestOpen( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "VFS archive: open reads files relative to the archive root, refuses unknown files and names that leave the root, and is read only" )
        try
        {
            CVfsFixture fixture;
            ASSERT_TRUE( fixture.IsOk() );
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            COgreVfsArchive archive( "/assets", "GucefVFS", vfs );
            archive.load();

            ASSERT_TRUE( archive.isReadOnly() );
            ASSERT_TRUE( archive.isCaseSensitive() );
            ASSERT_TRUE( "GucefVFS" == archive.getType() );
            ASSERT_TRUE( "/assets" == archive.getName() );

            Ogre::DataStreamPtr stream = archive.open( "a.txt" );
            ASSERT_TRUE( (bool) stream );
            if ( stream )
            {
                ASSERT_TRUE( 5 == stream->size() );
                ASSERT_TRUE( "alpha" == stream->getAsString() );
                ASSERT_TRUE( stream->isReadable() );
            }

            stream = archive.open( "sub/b.txt" );
            ASSERT_TRUE( (bool) stream && "bravo-bravo" == stream->getAsString() );
            stream = archive.open( "sub\\deep\\c.dat" );
            ASSERT_TRUE( (bool) stream && "ccc" == stream->getAsString() );

            // not there, not a file, the file of another archive, and ways out of the root
            ASSERT_FALSE( (bool) archive.open( "missing.txt" ) );
            ASSERT_FALSE( (bool) archive.open( "sub" ) );
            ASSERT_FALSE( (bool) archive.open( "marker.txt" ) );
            ASSERT_FALSE( (bool) archive.open( "../other/marker.txt" ) );
            ASSERT_FALSE( (bool) archive.open( "sub/../a.txt" ) );
            ASSERT_FALSE( (bool) archive.open( "" ) );

            // the mode "rw" is not a mode of the VFS: the file is opened for reading and the stream cannot write
            stream = archive.open( "a.txt", false );
            ASSERT_TRUE( (bool) stream );
            if ( stream )
            {
                ASSERT_TRUE( "alpha" == stream->getAsString() );
                ASSERT_FALSE( stream->isWriteable() );
                ASSERT_TRUE( 0 == stream->write( "x", 1 ) );
            }

            // the archive does not create and remove files
            bool createRefused = false;
            try { archive.create( "new.txt" ); } catch ( const Ogre::Exception& ) { createRefused = true; }
            ASSERT_TRUE( createRefused );
            bool removeRefused = false;
            try { archive.remove( "a.txt" ); } catch ( const Ogre::Exception& ) { removeRefused = true; }
            ASSERT_TRUE( removeRefused );
            ASSERT_TRUE( "alpha" == ReadTestFile( fixture.GetAssetsDir() + "/a.txt" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestExistsAndModifiedTime( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "VFS archive: exists and getModifiedTime prefix the archive root as open does, they do not look at the root of the VFS" )
        try
        {
            CVfsFixture fixture;
            ASSERT_TRUE( fixture.IsOk() );
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            COgreVfsArchive assets( "/assets", "GucefVFS", vfs );
            assets.load();
            COgreVfsArchive other( "/other", "GucefVFS", vfs );
            other.load();
            COgreVfsArchive root( "/", "GucefVFS", vfs );
            root.load();

            ASSERT_TRUE( assets.exists( "a.txt" ) );
            ASSERT_TRUE( assets.exists( "sub/b.txt" ) );
            ASSERT_TRUE( assets.exists( "sub\\deep\\c.dat" ) );
            ASSERT_TRUE( assets.exists( "Readme.MD" ) );
            ASSERT_FALSE( assets.exists( "readme.md" ) );
            ASSERT_FALSE( assets.exists( "missing.txt" ) );
            // directories are not files, the VFS path of a file is not a name of the archive
            ASSERT_FALSE( assets.exists( "sub" ) );
            ASSERT_FALSE( assets.exists( "/assets/a.txt" ) );
            ASSERT_FALSE( assets.exists( "sub/../a.txt" ) );
            // files of other mounts
            ASSERT_FALSE( assets.exists( "marker.txt" ) );
            ASSERT_TRUE( other.exists( "marker.txt" ) );
            ASSERT_FALSE( other.exists( "a.txt" ) );
            // the VFS lists the directories of the mount points it has below a path only for a path that is in a
            // mount, so there is nothing for an archive rooted at the root of the VFS to find
            ASSERT_FALSE( root.exists( "a.txt" ) );
            ASSERT_FALSE( root.exists( "assets/a.txt" ) );

            const time_t now = time( GUCEF_NULL );
            const time_t modified = assets.getModifiedTime( "sub/b.txt" );
            const time_t expected = COgreVfsArchive::ToTimeT( CORE::GetFileModificationTime( fixture.GetAssetsDir() + "/sub/b.txt" ) );
            ASSERT_TRUE( 0 != modified );
            ASSERT_TRUE( expected == modified );
            // the file was written by this test
            ASSERT_TRUE( modified <= now + 5 && modified >= now - 3600 );
            ASSERT_TRUE( 0 == assets.getModifiedTime( "missing.txt" ) );
            ASSERT_TRUE( 0 == assets.getModifiedTime( "marker.txt" ) );
            ASSERT_TRUE( 0 != other.getModifiedTime( "marker.txt" ) );
            ASSERT_TRUE( 0 == assets.getModifiedTime( "../other/marker.txt" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestListAndFind( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "VFS archive: list, listFileInfo, find and findFileInfo answer relative names, with wildcards, directories and sizes" )
        try
        {
            CVfsFixture fixture;
            ASSERT_TRUE( fixture.IsOk() );
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            COgreVfsArchive archive( "/assets", "GucefVFS", vfs );
            archive.load();

            ASSERT_TRUE( MakeSet( "Readme.MD", "a.txt", "sub/b.txt", "sub/deep/c.dat" ) == ToSet( archive.list( true, false ) ) );
            ASSERT_TRUE( MakeSet( "Readme.MD", "a.txt" ) == ToSet( archive.list( false, false ) ) );
            ASSERT_TRUE( MakeSet( "sub", "sub/deep" ) == ToSet( archive.list( true, true ) ) );
            ASSERT_TRUE( MakeSet( "sub" ) == ToSet( archive.list( false, true ) ) );

            ASSERT_TRUE( MakeSet( "a.txt", "sub/b.txt" ) == ToSet( archive.find( "*.txt", true, false ) ) );
            ASSERT_TRUE( MakeSet( "a.txt" ) == ToSet( archive.find( "*.txt", false, false ) ) );
            ASSERT_TRUE( MakeSet( "sub/b.txt" ) == ToSet( archive.find( "b.txt", true, false ) ) );
            ASSERT_TRUE( MakeSet() == ToSet( archive.find( "b.txt", false, false ) ) );
            ASSERT_TRUE( MakeSet( "sub/b.txt", "sub/deep/c.dat" ) == ToSet( archive.find( "sub/*", true, false ) ) );
            ASSERT_TRUE( MakeSet( "sub/b.txt" ) == ToSet( archive.find( "sub/*.txt", false, false ) ) );
            ASSERT_TRUE( MakeSet( "sub/deep/c.dat" ) == ToSet( archive.find( "sub/deep/*", false, false ) ) );
            // the mask is case sensitive
            ASSERT_TRUE( MakeSet() == ToSet( archive.find( "*.md", true, false ) ) );
            ASSERT_TRUE( MakeSet( "Readme.MD" ) == ToSet( archive.find( "*.MD", true, false ) ) );
            ASSERT_TRUE( MakeSet( "sub" ) == ToSet( archive.find( "sub*", true, true ) ) );
            ASSERT_TRUE( MakeSet( "sub/deep" ) == ToSet( archive.find( "deep", true, true ) ) );
            ASSERT_TRUE( MakeSet() == ToSet( archive.find( "*.nothing", true, false ) ) );

            // the file info is filled, with names relative to the root of the archive, and the size of the file
            Ogre::FileInfoListPtr infos = archive.listFileInfo( true, false );
            ASSERT_TRUE( (bool) infos && 4 == infos->size() );
            if ( infos && 4 == infos->size() )
            {
                bool allOwned = true;
                for ( size_t i=0; i<infos->size(); ++i )
                {
                    const Ogre::FileInfo& info = (*infos)[ i ];
                    allOwned = allOwned && &archive == info.archive && info.compressedSize == info.uncompressedSize;
                    if ( "sub/b.txt" == info.filename )
                    {
                        ASSERT_TRUE( "b.txt" == info.basename );
                        ASSERT_TRUE( "sub/" == info.path );
                        ASSERT_TRUE( 11 == info.uncompressedSize );
                    }
                    else
                    if ( "a.txt" == info.filename )
                    {
                        ASSERT_TRUE( "a.txt" == info.basename );
                        ASSERT_TRUE( info.path.empty() );
                        ASSERT_TRUE( 5 == info.uncompressedSize );
                    }
                    else
                    if ( "sub/deep/c.dat" == info.filename )
                        ASSERT_TRUE( 3 == info.uncompressedSize && "sub/deep/" == info.path && "c.dat" == info.basename );
                    else
                    if ( "Readme.MD" == info.filename )
                        ASSERT_TRUE( 16 == info.uncompressedSize );
                    else
                        allOwned = false;
                }
                ASSERT_TRUE( allOwned );
            }

            infos = archive.findFileInfo( "*.txt", true, false );
            ASSERT_TRUE( (bool) infos && 2 == infos->size() );
            if ( infos && 2 == infos->size() )
            {
                TNameSet names;
                for ( size_t i=0; i<infos->size(); ++i )
                {
                    // never the absolute VFS path
                    ASSERT_TRUE( 0 != (*infos)[ i ].filename.compare( 0, 1, "/" ) );
                    ASSERT_TRUE( (*infos)[ i ].filename.size() >= (*infos)[ i ].basename.size() );
                    names.insert( (*infos)[ i ].filename );
                }
                ASSERT_TRUE( MakeSet( "a.txt", "sub/b.txt" ) == names );
            }
            infos = archive.listFileInfo( true, true );
            ASSERT_TRUE( (bool) infos && 2 == infos->size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestListingIsCachedAndInvalidated( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "VFS archive: the listing is made once, and again after a mount or unmount of the VFS or on request" )
        try
        {
            CVfsFixture fixture;
            ASSERT_TRUE( fixture.IsOk() );
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            COgreVfsArchive archive( "/assets", "GucefVFS", vfs );
            archive.load();
            ASSERT_TRUE( 0 == archive.GetListingBuildCount() );

            // many lookups of Ogre are one question to the VFS
            for ( int i=0; i<20; ++i )
            {
                archive.exists( "a.txt" );
                archive.exists( "nothing.txt" );
                archive.find( "*.txt", true, false );
                archive.list( true, false );
                archive.findFileInfo( "*", true, false );
            }
            ASSERT_TRUE( 1 == archive.GetListingBuildCount() );

            // a file that is added is not there for Ogre before the archive is asked to look again
            ASSERT_TRUE( WriteTestFile( fixture.GetAssetsDir() + "/late.txt", "late" ) );
            ASSERT_FALSE( archive.exists( "late.txt" ) );
            archive.InvalidateListing();
            ASSERT_TRUE( archive.exists( "late.txt" ) );
            ASSERT_TRUE( 2 == archive.GetListingBuildCount() );
            ASSERT_TRUE( 2 == archive.GetListingBuildCount() );

            // mounting an archive in the VFS invalidates the listing: a second directory at the mount point adds its files
            const CORE::CString extraDir = MakeCleanTestDir( "vfs_extra" );
            ASSERT_TRUE( WriteTestFile( extraDir + "/extra.txt", "extra" ) );
            ASSERT_FALSE( archive.exists( "extra.txt" ) );
            ASSERT_TRUE( vfs.AddRoot( extraDir, "ogre_test_extra", false, false, "/assets" ) );
            ASSERT_TRUE( archive.exists( "extra.txt" ) );
            ASSERT_TRUE( 3 == archive.GetListingBuildCount() );

            // and so does unmounting
            ASSERT_TRUE( vfs.UnmountArchiveByName( "ogre_test_extra" ) );
            ASSERT_FALSE( archive.exists( "extra.txt" ) );
            ASSERT_TRUE( 4 == archive.GetListingBuildCount() );

            // an archive that is unloaded does not follow the VFS any more
            archive.unload();
            ASSERT_TRUE( archive.exists( "a.txt" ) );
            const UInt32 builds = archive.GetListingBuildCount();
            ASSERT_TRUE( vfs.AddRoot( extraDir, "ogre_test_extra", false, false, "/assets" ) );
            ASSERT_FALSE( archive.exists( "extra.txt" ) );
            ASSERT_TRUE( builds == archive.GetListingBuildCount() );
            vfs.UnmountArchiveByName( "ogre_test_extra" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestThroughOgreResourceGroups( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "VFS archive: Ogre resource groups find, list and open resources through the GucefVFS archive type, and through FileSystem which reads from the VFS as well" )
        try
        {
            CVfsFixture fixture;
            ASSERT_TRUE( fixture.IsOk() );
            ASSERT_TRUE( Ogre::ArchiveManager::getSingleton().hasArchiveFactory( "GucefVFS" ) );

            Ogre::ResourceGroupManager& groups = Ogre::ResourceGroupManager::getSingleton();
            groups.createResourceGroup( "OgreTestVfsGroup" );
            groups.addResourceLocation( "/assets", "GucefVFS", "OgreTestVfsGroup", true );

            ASSERT_TRUE( groups.resourceExists( "OgreTestVfsGroup", "sub/b.txt" ) );
            ASSERT_TRUE( groups.resourceExists( "OgreTestVfsGroup", "a.txt" ) );
            ASSERT_FALSE( groups.resourceExists( "OgreTestVfsGroup", "marker.txt" ) );
            Ogre::DataStreamPtr stream = groups.openResource( "sub/b.txt", "OgreTestVfsGroup" );
            ASSERT_TRUE( (bool) stream && "bravo-bravo" == stream->getAsString() );
            ASSERT_TRUE( 0 != groups.resourceModifiedTime( "OgreTestVfsGroup", "a.txt" ) );

            Ogre::StringVectorPtr names = groups.listResourceNames( "OgreTestVfsGroup", false );
            ASSERT_TRUE( 4 == ToSet( names ).size() );
            Ogre::FileInfoListPtr infos = groups.findResourceFileInfo( "OgreTestVfsGroup", "*.txt", false );
            ASSERT_TRUE( (bool) infos && 2 == infos->size() );
            Ogre::StringVectorPtr found = groups.findResourceNames( "OgreTestVfsGroup", "*.dat", false );
            ASSERT_TRUE( MakeSet( "sub/deep/c.dat" ) == ToSet( found ) );

            // the type of the archive that existing resource configuration names: nothing of /assets is on the disk
            groups.createResourceGroup( "OgreTestFileSystemGroup" );
            groups.addResourceLocation( "/assets/sub", "FileSystem", "OgreTestFileSystemGroup", true );
            ASSERT_TRUE( groups.resourceExists( "OgreTestFileSystemGroup", "b.txt" ) );
            ASSERT_TRUE( groups.resourceExists( "OgreTestFileSystemGroup", "deep/c.dat" ) );
            stream = groups.openResource( "b.txt", "OgreTestFileSystemGroup" );
            ASSERT_TRUE( (bool) stream && "bravo-bravo" == stream->getAsString() );

            // a location that is not there is not found as a resource
            groups.createResourceGroup( "OgreTestNothingGroup" );
            groups.addResourceLocation( "/nothing/here", "GucefVFS", "OgreTestNothingGroup", true );
            ASSERT_FALSE( groups.resourceExists( "OgreTestNothingGroup", "a.txt" ) );

            groups.destroyResourceGroup( "OgreTestNothingGroup" );
            groups.destroyResourceGroup( "OgreTestFileSystemGroup" );
            groups.destroyResourceGroup( "OgreTestVfsGroup" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static bool
MakeZipInVfs( const CORE::CString& zipPath )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "zip" );
    if ( writer.IsNULL() )
        return false;

    CORE::CFileAccess zipFile( zipPath, "wb" );
    if ( !zipFile.IsValid() || !writer->Open( zipFile ) )
        return false;

    const char* names[] = { "z/one.txt", "two.txt" };
    const char* contents[] = { "first in the zip", "second" };
    bool ok = true;
    for ( int i=0; i<2; ++i )
    {
        CORE::CDynamicBuffer content( contents[ i ], (UInt32) strlen( contents[ i ] ), false );
        CORE::CDynamicBufferAccess contentAccess( content );
        ok = writer->AddFile( names[ i ], contentAccess, VFS::CArchiveWriterEntrySettings() ) && ok;
    }
    ok = writer->Close() && ok;
    zipFile.Close();
    return ok;
}

/*-------------------------------------------------------------------------*/

static void
TestZipThroughVfs( void )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( vfs.CreateArchiveWriter( "zip" ).IsNULL() )
        CORE::CCoreGlobal::Instance()->GetPluginControl().AddPluginFromDir( "$MODULEDIR$/vfspluginZIP", "ogretestvfs", true );

    GUCEF_TESTFW_TESTCASE( "VFS archive: a resource location of the type Zip is a ZIP file of the VFS, mounted while Ogre has it loaded" )
        GUCEF_TESTFW_SKIP_IF( vfs.CreateArchiveWriter( "zip" ).IsNULL(), "vfspluginZIP is not available" );
        try
        {
            CVfsFixture fixture;
            ASSERT_TRUE( fixture.IsOk() );
            ASSERT_TRUE( MakeZipInVfs( fixture.GetAssetsDir() + "/pack.zip" ) );
            ASSERT_TRUE( Ogre::ArchiveManager::getSingleton().hasArchiveFactory( "Zip" ) );

            Ogre::ResourceGroupManager& groups = Ogre::ResourceGroupManager::getSingleton();
            groups.createResourceGroup( "OgreTestZipGroup" );
            groups.addResourceLocation( "/assets/pack.zip", "Zip", "OgreTestZipGroup", true );

            ASSERT_TRUE( groups.resourceExists( "OgreTestZipGroup", "z/one.txt" ) );
            ASSERT_TRUE( groups.resourceExists( "OgreTestZipGroup", "two.txt" ) );
            ASSERT_FALSE( groups.resourceExists( "OgreTestZipGroup", "a.txt" ) );
            Ogre::DataStreamPtr stream = groups.openResource( "z/one.txt", "OgreTestZipGroup" );
            ASSERT_TRUE( (bool) stream && "first in the zip" == stream->getAsString() );
            stream.reset();

            // the mount is gone when the location is
            groups.destroyResourceGroup( "OgreTestZipGroup" );
            VFS::CVFS::TStringVector mounted;
            vfs.GetDirList( mounted, "/ogre-archives", false, true, CORE::CString::Empty );
            ASSERT_TRUE( mounted.empty() );

            // a ZIP file that is not there cannot be a location
            groups.createResourceGroup( "OgreTestNoZipGroup" );
            bool refused = false;
            try
            {
                groups.addResourceLocation( "/assets/missing.zip", "Zip", "OgreTestNoZipGroup", true );
            }
            catch ( const Ogre::Exception& )
            {
                refused = true;
            }
            ASSERT_TRUE( refused );
            groups.destroyResourceGroup( "OgreTestNoZipGroup" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformVfsArchiveTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "OgrePlugin_GucefVFS.VfsArchive" );

    TestOpen();
    TestExistsAndModifiedTime();
    TestListAndFind();
    TestListingIsCachedAndInvalidated();
    TestThroughOgreResourceGroups();
    TestZipThroughVfs();
}

/*-------------------------------------------------------------------------*/
