/*
 *  AssetViewerLib_TestApp: Test application for the asset viewer library (AssetViewerLib)
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

#include <string.h>

#include <Ogre.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#include "AssetViewerLib.h"
#include "AssetViewerLib_CAssetOpener.h"

#include "TestAssetViewerSupport.h"
#include "TestAssetViewerAssets.h"
#include "TestAssetViewerOpen.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETVIEWER;

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

bool
AddEntry( VFS::CIArchiveWriter& writer, const CORE::CString& name, const CORE::CString& text )
{
    CORE::CDynamicBuffer content;
    content.CopyFrom( text.Length(), text.C_String() );
    CORE::CDynamicBufferAccess access( content );
    VFS::CArchiveWriterEntrySettings settings;
    return writer.AddFile( name, access, settings );
}

/** A zip with a model, a picture and a text file in a folder */
bool
WriteTestZip( const CORE::CString& path, const CORE::CString& dir )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CFileAccess output( path, "wb" );
    VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "zip" );
    if ( writer.IsNULL() || !writer->Open( output ) )
        return false;
    VFS::CArchiveWriterEntrySettings settings;
    bool ok = writer->AddFileFromPath( "Models/model.gltf", dir + "/model.gltf", settings );
    ok = ok && writer->AddFileFromPath( "Textures/checker.png", dir + "/checker.png", settings );
    ok = ok && AddEntry( *writer, "readme.txt", "zip\n" );
    return writer->Close() && ok;
}

/** A Unity package of one asset: the way the VFS reads them is a tar.gz of folders with "asset" and "pathname" */
bool
WriteTestUnityPackage( const CORE::CString& path )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CFileAccess output( path, "wb" );
    VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "tgz" );
    if ( writer.IsNULL() || !writer->Open( output ) )
        return false;
    bool ok = AddEntry( *writer, "0123456789abcdef0123456789abcdef/asset", "inner file!\n" );
    ok = ok && AddEntry( *writer, "0123456789abcdef0123456789abcdef/pathname", "Assets/Models/b.txt\n" );
    return writer->Close() && ok;
}

/** The test folder: a model, pictures and a text file, and a folder with the name of a folder of the root of the native file system */
bool
WriteTestFolder( const CORE::CString& dir )
{
    return WriteTestFile( dir + "/model.gltf", BuildTestGltf() ) && WriteTestPng( dir + "/checker.png", 8, 8 ) &&
           WriteTestPng( dir + "/pic.png", 8, 4 ) && WriteTestFile( dir + "/readme.txt", "not an asset" );
}

bool
Has( const CORE::CString::StringVector& names, const CORE::CString& name )
{
    for ( size_t i=0; i<names.size(); ++i )
        if ( names[ i ] == name )
            return true;
    return false;
}

Int32
IndexOf( const CORE::CString::StringVector& names, const CORE::CString& name )
{
    for ( size_t i=0; i<names.size(); ++i )
        if ( names[ i ] == name )
            return (Int32) i;
    return -1;
}

/** The viewer in a window of the tests with nothing mounted (unless asked), as it is at the start */
class COpenFixture
{
    public:

    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    GUIDRIVEROGRE::COgreWindowContext* window;
    CAssetMounter mounter;
    CAssetCapture* capture;
    CViewerShell* shell;
    CORE::CString dir;
    bool ready;

    COpenFixture( const CORE::CString& name, bool mountFirst )
        : contextPtr()
        , context( GUCEF_NULL )
        , window( GUCEF_NULL )
        , mounter()
        , capture( GUCEF_NULL )
        , shell( GUCEF_NULL )
        , dir()
        , ready( false )
    {
        if ( GetMainTestWindow().IsNULL() )
            return;
        window = AsOgreWindow( GetMainTestWindow() );
        dir = MakeCleanTestDir( name );
        if ( !WriteTestFolder( dir + "/Town Pack" ) )
            return;
        if ( mountFirst && !mounter.Mount( "avfirst", dir + "/Town Pack" ) )
            return;
        contextPtr = CreateViewerContext();
        context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        if ( GUCEF_NULL == context )
            return;
        capture = GUCEF_NEW CAssetCapture( window );
        if ( !capture->Initialize( 64 ) )
            return;
        shell = GUCEF_NEW CViewerShell( mounter );
        ready = shell->Initialize( *context, capture, MakeCleanTestDir( name + "_cache" ) );
    }

    ~COpenFixture()
    {
        if ( GUCEF_NULL != shell )
        {
            shell->Shutdown();
            GUCEF_DELETE shell;
        }
        contextPtr.Unlink();
        if ( GUCEF_NULL != capture )
        {
            capture->Shutdown();
            GUCEF_DELETE capture;
        }
        mounter.UnmountAll();
    }

    IMAGE::TPixelMapPtr Frames( UInt32 count )
    {
        IMAGE::TPixelMapPtr pixels;
        for ( UInt32 i=0; i<count; ++i )
        {
            shell->Update( 5 );
            window->RenderFrame( &pixels );
        }
        return pixels;
    }

    UInt32 CountReports( const CORE::CString& message ) const
    {
        UInt32 count = 0;
        for ( UInt32 i=0; i<shell->GetReport()->GetEntryCount(); ++i )
            if ( shell->GetReport()->GetEntry( i ).message == message )
                ++count;
        return count;
    }
};

}

/*-------------------------------------------------------------------------*/

static void
TestOpenMounting( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Open: a folder, a zip and a Unity package are mounted at the unique name of their own, what is mounted already is used again, and files are found in their folder" )
    try
    {
        const CORE::CString dir = MakeCleanTestDir( "open_mounting" );
        ASSERT_TRUE( WriteTestFolder( dir + "/Town Pack" ) && WriteTestFolder( dir + "/other/Town Pack" ) );
        ASSERT_TRUE( WriteTestFolder( dir + "/tmp" ) );
        ASSERT_TRUE( WriteTestZip( dir + "/Shops.zip", dir + "/Town Pack" ) && WriteTestUnityPackage( dir + "/Props.unitypackage" ) );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

        CAssetMounter mounter;
        CAssetOpener opener( mounter );
        CAssetOpener::SOpenResult result;

        // a folder: the name of the folder, made safe
        ASSERT_TRUE( CAssetOpener::OPENKIND_FOLDER == CAssetOpener::Classify( dir + "/Town Pack" ) );
        ASSERT_TRUE( opener.Open( dir + "/Town Pack", result ) && CAssetOpener::OPENKIND_FOLDER == result.kind );
        ASSERT_TRUE( "Town_Pack" == result.mountPoint && "Town_Pack" == result.vfsFolder && result.vfsAsset.IsNULLOrEmpty() );
        ASSERT_TRUE( vfs.FileExists( "Town_Pack/model.gltf" ) && 1 == mounter.GetMountCount() );

        // the same folder again is the same mount, another folder of the same name is a mount of its own
        ASSERT_TRUE( opener.Open( dir + "/Town Pack/", result ) && "Town_Pack" == result.mountPoint && 1 == mounter.GetMountCount() );
        ASSERT_TRUE( opener.Open( dir + "/other/Town Pack", result ) && "Town_Pack_2" == result.mountPoint && 2 == mounter.GetMountCount() );
        ASSERT_TRUE( vfs.FileExists( "Town_Pack_2/checker.png" ) );

        // a folder with the name of a folder of the root of this computer is not mounted under it, so a path of the disk is not taken for a path of a mount
        #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
        ASSERT_TRUE( opener.Open( dir + "/tmp", result ) && "tmp" != result.mountPoint && "tmp_2" == result.mountPoint );
        #endif

        // an archive is named without its extension and acts as a folder
        ASSERT_TRUE( CAssetOpener::OPENKIND_ARCHIVE == CAssetOpener::Classify( dir + "/Shops.zip" ) );
        ASSERT_TRUE( opener.Open( dir + "/Shops.zip", result ) && CAssetOpener::OPENKIND_ARCHIVE == result.kind && "Shops" == result.mountPoint && "Shops" == result.vfsFolder );
        ASSERT_TRUE( vfs.FileExists( "Shops/Models/model.gltf" ) && vfs.FileExists( "Shops/readme.txt" ) );
        ASSERT_TRUE( "zip" == mounter.GetMount( mounter.FindMountBySource( dir + "/Shops.zip" ) ).archiveType );
        ASSERT_TRUE( opener.Open( dir + "/Props.unitypackage", result ) && "Props" == result.mountPoint );
        ASSERT_TRUE( vfs.FileExists( "Props/Assets/Models/b.txt" ) && "unitypackage" == mounter.GetMount( mounter.FindMountBySource( dir + "/Props.unitypackage" ) ).archiveType );
        const UInt32 mountsBefore = mounter.GetMountCount();
        ASSERT_TRUE( opener.Open( dir + "/Props.unitypackage", result ) && "Props" == result.mountPoint && mountsBefore == mounter.GetMountCount() );

        // a file below a folder that is mounted is found in it, without a second mount
        ASSERT_TRUE( CAssetOpener::OPENKIND_FILE == CAssetOpener::Classify( dir + "/Town Pack/model.gltf" ) );
        ASSERT_TRUE( opener.Open( dir + "/Town Pack/model.gltf", result ) && CAssetOpener::OPENKIND_FILE == result.kind );
        ASSERT_TRUE( "Town_Pack" == result.mountPoint && "Town_Pack/model.gltf" == result.vfsAsset && "Town_Pack" == result.vfsFolder && mountsBefore == mounter.GetMountCount() );
        CORE::CString relative;
        ASSERT_TRUE( mounter.FindMountOfDiskPath( dir + "/Town Pack/deeper/x.fbx", relative ) >= 0 && "deeper/x.fbx" == relative );
        ASSERT_TRUE( mounter.FindMountOfDiskPath( dir + "/Town Packs/x.fbx", relative ) < 0 );

        // a file in a folder that is not mounted: its folder is mounted, and its place in it is told
        ASSERT_TRUE( CORE::CreateDirs( dir + "/loose/Sub Folder" ) && WriteTestPng( dir + "/loose/Sub Folder/pic.png", 4, 4 ) );
        ASSERT_TRUE( opener.Open( dir + "/loose/Sub Folder/pic.png", result ) && CAssetOpener::OPENKIND_FILE == result.kind );
        ASSERT_TRUE( "Sub_Folder" == result.mountPoint && "Sub_Folder/pic.png" == result.vfsAsset && "Sub_Folder" == result.vfsFolder && vfs.FileExists( "Sub_Folder/pic.png" ) );
        // a file in a folder below a mount: the folder that holds it is the one to show
        ASSERT_TRUE( CORE::CreateDirs( dir + "/Town Pack/textures" ) && WriteTestPng( dir + "/Town Pack/textures/deep.png", 4, 4 ) );
        ASSERT_TRUE( opener.Open( dir + "/Town Pack/textures/deep.png", result ) && "Town_Pack/textures/deep.png" == result.vfsAsset && "Town_Pack/textures" == result.vfsFolder );

        // what cannot be opened says why and mounts nothing
        const UInt32 mounts = mounter.GetMountCount();
        ASSERT_FALSE( opener.Open( dir + "/Town Pack/readme.txt", result ) );
        ASSERT_TRUE( CAssetOpener::OPENKIND_NONE == result.kind && result.problem.HasSubstr( "not an archive, pack or asset" ) >= 0 );
        ASSERT_FALSE( opener.Open( dir + "/does/not/exist", result ) );
        ASSERT_TRUE( result.problem.HasSubstr( "is not there" ) >= 0 );
        ASSERT_FALSE( opener.Open( CORE::CString::Empty, result ) );
        ASSERT_TRUE( mounts == mounter.GetMountCount() );

        // the unique name does not depend on the spelling of the path
        ASSERT_TRUE( CAssetMounter::NormalizeDiskPath( "a\\b\\c\\" ) == "a/b/c" && "/" == CAssetMounter::NormalizeDiskPath( "/" ) );
        ASSERT_TRUE( "mount" == CAssetMounter().MakeMountPointName( "/!!!" ) );
        mounter.UnmountAll();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestOpenPatterns( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Open: the dialogs offer the archives, the models and the pictures the viewer can open" )
    try
    {
        CORE::CString::StringVector archives, models, textures, supported;
        CAssetOpener::GetArchivePatterns( archives );
        CAssetOpener::GetModelPatterns( models );
        CAssetOpener::GetTexturePatterns( textures );
        CAssetOpener::GetSupportedPatterns( supported );
        const char* const wantedArchives[] = { "*.zip", "*.7z", "*.rar", "*.tar", "*.tar.gz", "*.unitypackage" };
        for ( size_t i=0; i<sizeof( wantedArchives ) / sizeof( wantedArchives[ 0 ] ); ++i )
            ASSERT_TRUE( Has( archives, wantedArchives[ i ] ) && Has( supported, wantedArchives[ i ] ) );
        const char* const wantedModels[] = { "*.fbx", "*.obj", "*.gltf", "*.glb", "*.stl", "*.3mf", "*.dae", "*.mesh", "*.scene" };
        for ( size_t i=0; i<sizeof( wantedModels ) / sizeof( wantedModels[ 0 ] ); ++i )
            ASSERT_TRUE( Has( models, wantedModels[ i ] ) && Has( supported, wantedModels[ i ] ) && !Has( textures, wantedModels[ i ] ) );
        const char* const wantedTextures[] = { "*.png", "*.jpg", "*.tga", "*.dds", "*.ktx", "*.exr" };
        for ( size_t i=0; i<sizeof( wantedTextures ) / sizeof( wantedTextures[ 0 ] ); ++i )
            ASSERT_TRUE( Has( textures, wantedTextures[ i ] ) && Has( supported, wantedTextures[ i ] ) && !Has( models, wantedTextures[ i ] ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestOpenCommandLineAndSettings( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Open: bare paths of the command line are places to open next to the options that were there; the recent places are kept in the settings, at most 12, each once, and those that are gone are left out" )
    try
    {
        // bare paths and the options together: --mount keeps its meaning
        char program[] = "AssetViewer";
        char folder[] = "/tmp/Town";
        char zip[] = "Shops.zip";
        char mountOption[] = "--mount";
        char mountSpec[] = "town=/tmp/Town";
        char openOption[] = "--open";
        char openPath[] = "town/House.fbx";
        char screenshot[] = "--screenshot";
        char screenshotFile[] = "shot.png";
        char model[] = "/tmp/Town/House.fbx";
        char* argv[] = { program, folder, mountOption, mountSpec, zip, openOption, openPath, screenshot, screenshotFile, model };
        {
            CViewerApp app( (int) ( sizeof( argv ) / sizeof( argv[ 0 ] ) ), argv );
            ASSERT_TRUE( app.ParseCommandLine() );
            ASSERT_TRUE( 3 == app.GetOpenLocations().size() && "/tmp/Town" == app.GetOpenLocations()[ 0 ] && "Shops.zip" == app.GetOpenLocations()[ 1 ] && "/tmp/Town/House.fbx" == app.GetOpenLocations()[ 2 ] );
            ASSERT_TRUE( 1 == app.GetMountSpecs().size() && "town=/tmp/Town" == app.GetMountSpecs()[ 0 ] );
            ASSERT_TRUE( "town/House.fbx" == app.GetOpenPath() && "shot.png" == app.GetScreenshotFile() && !app.IsCapturing() );
        }
        {
            char* none[] = { program };
            CViewerApp app( 1, none );
            ASSERT_TRUE( app.ParseCommandLine() && app.GetOpenLocations().empty() );
        }
        {
            char bad[] = "--no-such-option";
            char* wrong[] = { program, bad };
            CViewerApp app( 2, wrong );
            ASSERT_FALSE( app.ParseCommandLine() );
        }
        {
            // the options of a capture are not bare paths
            char capture[] = "--capture";
            char outFolder[] = "out";
            char asset[] = "town/House.fbx";
            char* capturing[] = { program, mountOption, mountSpec, capture, outFolder, asset };
            CViewerApp app( 6, capturing );
            ASSERT_TRUE( app.ParseCommandLine() && app.IsCapturing() && 1 == app.GetCaptureAssets().size() && app.GetOpenLocations().empty() );
        }

        // the settings: places that exist stay in the order they were kept in, each once, those that are gone are dropped
        const CORE::CString dir = MakeCleanTestDir( "open_settings" );
        ASSERT_TRUE( WriteTestFolder( dir + "/one" ) && WriteTestFolder( dir + "/two" ) && WriteTestFile( dir + "/three.zip", "x" ) );
        CORE::CString::StringVector stored;
        stored.push_back( dir + "/two" );
        stored.push_back( dir + "/gone" );
        stored.push_back( dir + "/one" );
        stored.push_back( dir + "/two/" );
        stored.push_back( dir + "/three.zip" );
        CORE::CDataNode config;
        CORE::CDataNode* section = config.Structure( "AssetViewer", '/' );
        ASSERT_TRUE( section->SetValuesOfChildByName( "RecentLocations", stored ) );
        char* plain[] = { program };
        CViewerApp app( 1, plain );
        ASSERT_TRUE( app.LoadConfig( config ) );
        GUI::CRecentLocations& recent = app.GetRecentLocations();
        ASSERT_TRUE( 3 == recent.GetCount() && dir + "/two" == recent.Get( 0 ) && dir + "/one" == recent.Get( 1 ) && dir + "/three.zip" == recent.Get( 2 ) );
        ASSERT_TRUE( 12 == recent.GetMaxCount() );

        // the newest first, at most 12; kept again in the settings
        for ( UInt32 i=0; i<14; ++i )
        {
            const CORE::CString place = dir + "/many" + CORE::UInt32ToString( i );
            ASSERT_TRUE( CORE::CreateDirs( place ) && recent.Add( place ) );
        }
        ASSERT_TRUE( 12 == recent.GetCount() && dir + "/many13" == recent.Get( 0 ) && dir + "/many2" == recent.Get( 11 ) );
        CORE::CDataNode saved;
        ASSERT_TRUE( app.SaveConfig( saved ) );
        CORE::CString::StringVector keptInSettings;
        const CORE::CDataNode* savedSection = saved.FindChild( "AssetViewer" );
        ASSERT_TRUE( GUCEF_NULL != savedSection && savedSection->GetValuesOfChildByName( "RecentLocations", keptInSettings ) );
        ASSERT_TRUE( 12 == keptInSettings.size() && dir + "/many13" == keptInSettings[ 0 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestOpenStartAndEmptyState( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Open: at the start the content browser has the roots Local files and VFS, shows the files of the computer when nothing is mounted, the strip has the three ways to open, and the 3D view says how to open something" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        {
            COpenFixture fixture( "open_start", false );
            ASSERT_TRUE( fixture.ready );
            CViewerShell& shell = *fixture.shell;
            fixture.Frames( 3 );

            // the roots, in order, and where the viewer starts
            GUI::CContentBrowserForm* browser = shell.GetBrowser();
            ASSERT_TRUE( 2 == browser->GetRootCount() && "Local files" == browser->GetRootName( 0 ) && "VFS" == browser->GetRootName( 1 ) );
            ASSERT_TRUE( CViewerShell::LocalFilesRootName == browser->GetCurrentRootName() && !browser->GetCurrentDirectory().IsNULLOrEmpty() );
            CORE::CString home;
            if ( CORE::TryResolveSpecialDir( CORE::SPECIALDIR_HOME_DIR, home ) )
                ASSERT_TRUE( CAssetMounter::NormalizeDiskPath( home ) == CAssetMounter::NormalizeDiskPath( browser->GetCurrentDirectory() ) );
            ASSERT_TRUE( browser->NavigateTo( "VFS", "/" ) && browser->NavigateTo( "Local files", "/" ) && browser->NavigateTo( "Local files", home ) );

            // the strip: three buttons in the order of the owner's words, and the recent list with its first row only
            GUI::COpenActionStripForm* strip = shell.GetOpenStrip();
            ASSERT_TRUE( GUCEF_NULL != strip && 3 == strip->GetActions().size() );
            ASSERT_TRUE( "folder" == strip->GetActions()[ 0 ].id && "Open folder..." == strip->GetActions()[ 0 ].label );
            ASSERT_TRUE( "archive" == strip->GetActions()[ 1 ].id && "Open archive or pack..." == strip->GetActions()[ 1 ].label );
            ASSERT_TRUE( "file" == strip->GetActions()[ 2 ].id && "Open file..." == strip->GetActions()[ 2 ].label );
            ASSERT_TRUE( 1 == strip->GetRecentRowCount() && shell.GetRecentLocations() == strip->GetRecentLocations() );
            ASSERT_TRUE( 0 == shell.GetLeftTab() );

            // the empty state: said in the view and once in the report, until something is open
            ASSERT_TRUE( shell.IsEmptyStateShown() && CViewerShell::GetEmptyStateText().HasSubstr( "Open a folder, archive or pack to start" ) >= 0 );
            ASSERT_TRUE( CViewerShell::GetEmptyStateText().HasSubstr( "Ctrl+O" ) >= 0 && CViewerShell::GetEmptyStateText().HasSubstr( "start the viewer with a path" ) >= 0 );
            ASSERT_TRUE( 1 == fixture.CountReports( "Nothing is open" ) );
            DumpPicture( fixture.Frames( 3 ), "open_start_" + CORE::CString( GetTestRenderer() ) );
            ASSERT_TRUE( shell.IsEmptyStateShown() && 1 == fixture.CountReports( "Nothing is open" ) );
        }
        {
            // something was mounted before the start (--mount): the mounts are shown and no instruction is needed
            COpenFixture fixture( "open_start_mounted", true );
            ASSERT_TRUE( fixture.ready );
            ASSERT_TRUE( "VFS" == fixture.shell->GetBrowser()->GetCurrentRootName() && !fixture.shell->IsEmptyStateShown() && 0 == fixture.CountReports( "Nothing is open" ) );
        }
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestOpenLocations( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Open: a folder, a zip, a Unity package and a model or picture open as the dialogs open them: mounted once, shown in the VFS root, remembered, and the empty state is gone" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        COpenFixture fixture( "open_locations", false );
        ASSERT_TRUE( fixture.ready );
        CViewerShell& shell = *fixture.shell;
        GUI::CContentBrowserForm* browser = shell.GetBrowser();
        const CORE::CString dir = fixture.dir;
        ASSERT_TRUE( WriteTestZip( dir + "/Shops.zip", dir + "/Town Pack" ) && WriteTestUnityPackage( dir + "/Props.unitypackage" ) );
        fixture.Frames( 2 );
        ASSERT_TRUE( shell.IsEmptyStateShown() );

        // a folder: mounted under its name, the browser is in it, the places is remembered, the instruction is gone
        ASSERT_TRUE( shell.OpenLocation( dir + "/Town Pack" ) );
        ASSERT_TRUE( 1 == fixture.mounter.GetMountCount() && "Town_Pack" == fixture.mounter.GetMount( 0 ).mountPoint );
        ASSERT_TRUE( "VFS" == browser->GetCurrentRootName() && "/Town_Pack" == browser->GetCurrentDirectory() && 0 == shell.GetLeftTab() );
        const CORE::CString::StringVector listed = browser->GetListedNames();
        ASSERT_TRUE( Has( listed, "model.gltf" ) && Has( listed, "checker.png" ) && Has( listed, "pic.png" ) );
        ASSERT_TRUE( 1 == shell.GetRecentLocations()->GetCount() && CAssetMounter::NormalizeDiskPath( dir + "/Town Pack" ) == shell.GetRecentLocations()->Get( 0 ) );
        ASSERT_TRUE( !shell.IsEmptyStateShown() && 2 == shell.GetOpenStrip()->GetRecentRowCount() );
        ASSERT_TRUE( ASSETKIND_OTHER == shell.GetShownKind() );
        DumpPicture( fixture.Frames( 3 ), "open_folder_" + CORE::CString( GetTestRenderer() ) );

        // the same again: one mount, still one recent place
        ASSERT_TRUE( browser->NavigateTo( "Local files", "/" ) );
        ASSERT_TRUE( shell.OpenLocation( dir + "/Town Pack" ) );
        ASSERT_TRUE( 1 == fixture.mounter.GetMountCount() && 1 == shell.GetRecentLocations()->GetCount() && "VFS" == browser->GetCurrentRootName() );

        // a zip and a Unity package are folders of the VFS root as well
        ASSERT_TRUE( shell.OpenLocation( dir + "/Shops.zip" ) );
        ASSERT_TRUE( 2 == fixture.mounter.GetMountCount() && "Shops" == fixture.mounter.GetMount( 1 ).mountPoint && "zip" == fixture.mounter.GetMount( 1 ).archiveType );
        ASSERT_TRUE( "VFS" == browser->GetCurrentRootName() && "/Shops" == browser->GetCurrentDirectory() );
        ASSERT_TRUE( Has( browser->GetListedNames(), "Models/" ) && Has( browser->GetListedNames(), "readme.txt" ) );
        ASSERT_TRUE( shell.OpenLocation( dir + "/Props.unitypackage" ) );
        ASSERT_TRUE( 3 == fixture.mounter.GetMountCount() && "unitypackage" == fixture.mounter.GetMount( 2 ).archiveType && "/Props" == browser->GetCurrentDirectory() );
        ASSERT_TRUE( Has( browser->GetListedNames(), "Assets/" ) );
        ASSERT_TRUE( 3 == shell.GetRecentLocations()->GetCount() && CAssetMounter::NormalizeDiskPath( dir + "/Props.unitypackage" ) == shell.GetRecentLocations()->Get( 0 ) );

        // a model opens in the 3D view, from the mount of its folder; a picture in the texture view
        ASSERT_TRUE( shell.OpenLocation( dir + "/Town Pack/model.gltf" ) );
        ASSERT_TRUE( ASSETKIND_MODEL == shell.GetShownKind() && "Town_Pack/model.gltf" == shell.GetShownPath() && shell.GetModelView().IsLoaded() );
        ASSERT_TRUE( 3 == fixture.mounter.GetMountCount() && "/Town_Pack" == browser->GetCurrentDirectory() && 0 == shell.GetCenterTab() );
        DumpPicture( fixture.Frames( 4 ), "open_model_" + CORE::CString( GetTestRenderer() ) );
        ASSERT_TRUE( shell.OpenLocation( dir + "/Town Pack/pic.png" ) );
        ASSERT_TRUE( ASSETKIND_TEXTURE == shell.GetShownKind() && "Town_Pack/pic.png" == shell.GetShownPath() && 1 == shell.GetCenterTab() );
        ASSERT_TRUE( !shell.IsEmptyStateShown() );

        // a file in a folder that was not mounted: the folder of the file is
        ASSERT_TRUE( WriteTestFile( dir + "/loose/lone.gltf", BuildTestGltf() ) );
        ASSERT_TRUE( shell.OpenLocation( dir + "/loose/lone.gltf" ) );
        ASSERT_TRUE( 4 == fixture.mounter.GetMountCount() && "loose" == fixture.mounter.GetMount( 3 ).mountPoint && "loose/lone.gltf" == shell.GetShownPath() && ASSETKIND_MODEL == shell.GetShownKind() );

        // what cannot be opened leaves things as they are and is not remembered; a place that is gone leaves the recent places
        const UInt32 remembered = shell.GetRecentLocations()->GetCount();
        ASSERT_FALSE( shell.OpenLocation( dir + "/Town Pack/readme.txt" ) );
        ASSERT_FALSE( shell.OpenLocation( dir + "/nowhere" ) );
        ASSERT_TRUE( remembered == shell.GetRecentLocations()->GetCount() && 4 == fixture.mounter.GetMountCount() );
        ASSERT_TRUE( shell.GetRecentLocations()->Add( dir + "/nowhere" ) && shell.GetRecentLocations()->Contains( dir + "/nowhere" ) );
        ASSERT_FALSE( shell.OpenLocation( dir + "/nowhere" ) );
        ASSERT_FALSE( shell.GetRecentLocations()->Contains( dir + "/nowhere" ) );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestOpenFromLocalFiles( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Open: in Local files a double click enters an archive, opens an asset where the browser is, and enters a folder; a single click mounts nothing; the recent list opens a place again" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        COpenFixture fixture( "open_local", false );
        ASSERT_TRUE( fixture.ready );
        CViewerShell& shell = *fixture.shell;
        GUI::CContentBrowserForm* browser = shell.GetBrowser();
        const CORE::CString dir = fixture.dir;
        ASSERT_TRUE( WriteTestZip( dir + "/Shops.zip", dir + "/Town Pack" ) );
        fixture.Frames( 2 );

        // the folder with the files, in Local files
        ASSERT_TRUE( browser->NavigateTo( "Local files", dir ) );
        CORE::CString::StringVector names = browser->GetListedNames();
        ASSERT_TRUE( Has( names, "Shops.zip" ) && Has( names, "Town Pack/" ) );

        // a click on the pack selects it and mounts nothing; a double click on the folder enters it, still in Local files
        ASSERT_TRUE( browser->SelectNames( CORE::CString::StringVector( 1, "Shops.zip" ) ) );
        ASSERT_TRUE( 0 == fixture.mounter.GetMountCount() && "Local files" == browser->GetCurrentRootName() );
        ASSERT_TRUE( browser->ActivateItem( (UInt32) IndexOf( names, "Town Pack/" ) ) );
        ASSERT_TRUE( 0 == fixture.mounter.GetMountCount() && "Local files" == browser->GetCurrentRootName() && CAssetMounter::NormalizeDiskPath( dir + "/Town Pack" ) == CAssetMounter::NormalizeDiskPath( browser->GetCurrentDirectory() ) );

        // a double click on a model opens it, where the browser is: the folder was mounted for it
        names = browser->GetListedNames();
        ASSERT_TRUE( Has( names, "model.gltf" ) );
        ASSERT_TRUE( browser->ActivateItem( (UInt32) IndexOf( names, "model.gltf" ) ) );
        ASSERT_TRUE( ASSETKIND_MODEL == shell.GetShownKind() && 1 == fixture.mounter.GetMountCount() && "Town_Pack/model.gltf" == shell.GetShownPath() );
        ASSERT_TRUE( "Local files" == browser->GetCurrentRootName() && !shell.IsEmptyStateShown() );

        // a double click on the pack enters it as a folder of the VFS root
        ASSERT_TRUE( browser->NavigateTo( "Local files", dir ) );
        names = browser->GetListedNames();
        ASSERT_TRUE( browser->ActivateItem( (UInt32) IndexOf( names, "Shops.zip" ) ) );
        ASSERT_TRUE( 2 == fixture.mounter.GetMountCount() && "VFS" == browser->GetCurrentRootName() && "/Shops" == browser->GetCurrentDirectory() );

        // the recent list: the places are there newest first, and a choice opens one again, in the VFS root
        ASSERT_TRUE( 2 == shell.GetRecentLocations()->GetCount() && CAssetMounter::NormalizeDiskPath( dir + "/Shops.zip" ) == shell.GetRecentLocations()->Get( 0 ) );
        ASSERT_TRUE( 3 == shell.GetOpenStrip()->GetRecentRowCount() );
        ASSERT_TRUE( browser->NavigateTo( "Local files", dir ) );
        ASSERT_TRUE( shell.GetOpenStrip()->ChooseRecent( 1 ) );
        ASSERT_TRUE( 2 == fixture.mounter.GetMountCount() && "VFS" == browser->GetCurrentRootName() && "Town_Pack/model.gltf" == shell.GetShownPath() );
        ASSERT_TRUE( CAssetMounter::NormalizeDiskPath( dir + "/Town Pack/model.gltf" ) == shell.GetRecentLocations()->Get( 0 ) );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestOpenDialogs( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Open: the buttons and Ctrl+O show the open dialogs on the files of the computer with the file types, the places and the recent places; what is chosen in them is opened" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        COpenFixture fixture( "open_dialogs", false );
        ASSERT_TRUE( fixture.ready );
        CViewerShell& shell = *fixture.shell;
        const CORE::CString dir = fixture.dir;
        ASSERT_TRUE( WriteTestZip( dir + "/Shops.zip", dir + "/Town Pack" ) );
        fixture.Frames( 2 );
        ASSERT_TRUE( GUCEF_NULL == shell.GetFolderDialog() && GUCEF_NULL == shell.GetFileDialog() && GUCEF_NULL == shell.GetArchiveDialog() );

        // the folder button: the dialog for folders, on the files of the computer, with the VFS as the other root
        ASSERT_TRUE( shell.GetOpenStrip()->RequestAction( "folder" ) );
        GUI::CFileSystemDialogForm* folderDialog = shell.GetFolderDialog();
        ASSERT_TRUE( GUCEF_NULL != folderDialog && folderDialog->IsOpen() && GUI::CFileSystemDialogForm::MODE_OPEN_DIRECTORY == folderDialog->GetMode() );
        ASSERT_TRUE( "Local files" == folderDialog->GetCurrentRootName() );
        DumpPicture( fixture.Frames( 3 ), "open_folder_dialog_" + CORE::CString( GetTestRenderer() ) );
        GUI::CListbox* places = folderDialog->GetWidgetAs< GUI::CListbox >( "Places" );
        ASSERT_TRUE( GUCEF_NULL != places && Has( places->GetItems(), "Mounted (VFS)" ) );
        CORE::CString homeFolder;
        if ( CORE::TryResolveSpecialDir( CORE::SPECIALDIR_HOME_DIR, homeFolder ) && CORE::DirExists( homeFolder ) )
            ASSERT_TRUE( "Home" == places->GetItems()[ 0 ] && IndexOf( places->GetItems(), "Home" ) < IndexOf( places->GetItems(), "Mounted (VFS)" ) );
        ASSERT_TRUE( folderDialog->NavigateTo( "VFS", "/" ) && folderDialog->NavigateTo( "Local files", dir + "/Town Pack" ) );
        ASSERT_FALSE( shell.ShowOpenFolderDialog() );       // it is shown already

        // choosing the folder opens it
        ASSERT_TRUE( folderDialog->AcceptCurrentDirectory() && !folderDialog->IsOpen() );
        ASSERT_TRUE( 1 == fixture.mounter.GetMountCount() && "Town_Pack" == fixture.mounter.GetMount( 0 ).mountPoint && "VFS" == shell.GetBrowser()->GetCurrentRootName() );
        ASSERT_TRUE( !shell.IsEmptyStateShown() );

        // the archive dialog offers archives only, the file dialog the groups of files
        ASSERT_TRUE( shell.GetOpenStrip()->RequestAction( "archive" ) );
        GUI::CFileSystemDialogForm* archiveDialog = shell.GetArchiveDialog();
        ASSERT_TRUE( GUCEF_NULL != archiveDialog && archiveDialog->IsOpen() && GUI::CFileSystemDialogForm::MODE_OPEN_FILE == archiveDialog->GetMode() );
        ASSERT_TRUE( 1 == archiveDialog->GetFileTypes().size() && "Archives and packs" == archiveDialog->GetFileTypes()[ 0 ].description );
        ASSERT_TRUE( Has( archiveDialog->GetFileTypes()[ 0 ].patterns, "*.zip" ) && Has( archiveDialog->GetFileTypes()[ 0 ].patterns, "*.unitypackage" ) );

        // the dialog starts where the last place was, with that place offered
        ASSERT_TRUE( CAssetMounter::NormalizeDiskPath( dir + "/Town Pack" ) == CAssetMounter::NormalizeDiskPath( archiveDialog->GetCurrentDirectory() ) );
        GUI::CListbox* archivePlaces = archiveDialog->GetWidgetAs< GUI::CListbox >( "Places" );
        ASSERT_TRUE( GUCEF_NULL != archivePlaces && Has( archivePlaces->GetItems(), "Recent: Town Pack" ) );
        ASSERT_TRUE( archiveDialog->NavigateTo( "Local files", dir ) );
        archiveDialog->SetName( "Shops.zip" );
        ASSERT_TRUE( archiveDialog->Accept() && !archiveDialog->IsOpen() );
        ASSERT_TRUE( 2 == fixture.mounter.GetMountCount() && "Shops" == fixture.mounter.GetMount( 1 ).mountPoint && "/Shops" == shell.GetBrowser()->GetCurrentDirectory() );

        // the file dialog: the groups of files; Ctrl+O shows it
        ASSERT_TRUE( shell.HandleKey( INPUT::KEYCODE_O, INPUT::KEYMOD_CTRL ) );
        GUI::CFileSystemDialogForm* fileDialog = shell.GetFileDialog();
        ASSERT_TRUE( GUCEF_NULL != fileDialog && fileDialog->IsOpen() && GUI::CFileSystemDialogForm::MODE_OPEN_FILE == fileDialog->GetMode() );
        ASSERT_TRUE( 4 == fileDialog->GetFileTypes().size() );
        ASSERT_TRUE( "All supported" == fileDialog->GetFileTypes()[ 0 ].description && "3D models, scenes and materials" == fileDialog->GetFileTypes()[ 1 ].description );
        ASSERT_TRUE( "Textures and images" == fileDialog->GetFileTypes()[ 2 ].description && "Archives and packs" == fileDialog->GetFileTypes()[ 3 ].description );
        ASSERT_TRUE( Has( fileDialog->GetFileTypes()[ 1 ].patterns, "*.fbx" ) && Has( fileDialog->GetFileTypes()[ 2 ].patterns, "*.png" ) && Has( fileDialog->GetFileTypes()[ 0 ].patterns, "*.zip" ) );
        ASSERT_TRUE( fileDialog->NavigateTo( "Local files", dir + "/Town Pack" ) );
        const CORE::CString::StringVector shown = fileDialog->GetListedNames();
        ASSERT_TRUE( Has( shown, "model.gltf" ) && Has( shown, "checker.png" ) && !Has( shown, "readme.txt" ) );
        fileDialog->SetName( "model.gltf" );
        ASSERT_TRUE( fileDialog->Accept() && !fileDialog->IsOpen() );
        ASSERT_TRUE( ASSETKIND_MODEL == shell.GetShownKind() && "Town_Pack/model.gltf" == shell.GetShownPath() && 2 == fixture.mounter.GetMountCount() );

        // Ctrl+Shift+O shows the folder dialog again, and a dialog that is shown takes the key
        ASSERT_TRUE( shell.HandleKey( INPUT::KEYCODE_O, INPUT::KEYMOD_CTRL | INPUT::KEYMOD_SHIFT ) && shell.GetFolderDialog()->IsOpen() );
        ASSERT_TRUE( shell.HandleKey( INPUT::KEYCODE_O, INPUT::KEYMOD_CTRL ) && !shell.GetFileDialog()->IsOpen() );
        shell.GetFolderDialog()->Cancel();
        ASSERT_FALSE( shell.HandleKey( INPUT::KEYCODE_O, 0 ) || shell.HandleKey( INPUT::KEYCODE_P, INPUT::KEYMOD_CTRL ) );
        ASSERT_TRUE( 2 == fixture.mounter.GetMountCount() );

        // the key as the window gives it, to the form that is shown
        fixture.context->InjectKey( INPUT::KEYCODE_LCONTROL, true, INPUT::KEYMOD_CTRL );
        fixture.context->InjectKey( INPUT::KEYCODE_O, true, INPUT::KEYMOD_CTRL );
        fixture.context->InjectKey( INPUT::KEYCODE_O, false, INPUT::KEYMOD_CTRL );
        fixture.context->InjectKey( INPUT::KEYCODE_LCONTROL, false, 0 );
        ASSERT_TRUE( shell.GetFileDialog()->IsOpen() );
        shell.GetFileDialog()->Cancel();
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAssetViewerOpenTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetViewerLib.Open" );

    TestOpenMounting();
    TestOpenPatterns();
    TestOpenCommandLineAndSettings();
    TestOpenStartAndEmptyState();
    TestOpenLocations();
    TestOpenFromLocalFiles();
    TestOpenDialogs();
}

/*-------------------------------------------------------------------------*/
