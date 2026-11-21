/*
 *  AssetWorkbenchLib_TestApp: Test application for the asset workbench library (AssetWorkbenchLib)
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

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#include <errno.h>
#include <string.h>
#include <Ogre.h>
#include <OgreMeshSerializer.h>

#include "AssetWorkbenchLib.h"
#include "AssetWorkbenchLib_CWorkbenchMaterials.h"
#include "AssetConverterLib.h"
#include "AssetConverterLib_CAssetConverterGlobal.h"
#include "AssetConverterLib_CAssetPluginRegistry.h"
#include "AssetConverterLib_CAssetCacheEntry.h"
#include "AssetConverterLib_CAssetScene.h"
#include "AssetConverterLib_CConversionReport.h"
#include "AssetConverterLib_CIAssetExporter.h"
#include "AssetConverterLib_CIAssetCooker.h"
#include "AssetConverterLib_CCookSettings.h"
#include "AssetConverterLib_CContentHash.h"
#include "AssetConverterLib_CVfsPaths.h"

#include "TestAssetWorkbenchSupport.h"
#include "TestAssetWorkbenchAssets.h"
#include "TestAssetWorkbenchCookedOutput.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETWORKBENCH;

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

typedef CCookedOutputResolver TResolver;

const char* const CookedLabel = "cooked Ogre v1 output (gucef-ogre3d)";

bool
Contains( const CORE::CString& text, const CORE::CString& part )
{
    return text.HasSubstr( part ) >= 0;
}

UInt64
NowInMs( void )
{
    return MT::PrecisionTickCount() * 1000 / MT::PrecisionTimerResolution();
}

/** A store location on the disk, mounted writable in the VFS: the three areas of the layout of version 2 around one pack, with one asset */
class CStore
{
    public:

    CORE::CString dir;
    CORE::CString mount;
    CORE::CString pack;
    CORE::CString asset;
    bool mounted;

    CStore( const CORE::CString& name )
        : dir( MakeCleanTestDir( name ) ), mount( name ), pack( "Props" ), asset( "box.gltf" ), mounted( false )
    {
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        mounted = vfs.MountArchive( "cookedtest/" + name, dir, VFS::CVFS::FileSystemArchiveTypeName, mount, true, false, dir );
        CORE::CreatePathDirectories( dir + "/original/" + pack + "/" );
        CORE::CreatePathDirectories( dir + "/intermediate/" + pack + "/.cache/" );
        CORE::CreatePathDirectories( dir + "/gucef-ogre3d/" + pack + "/desktop/" + pack + "/" );
    }

    ~CStore()
    {
        if ( mounted )
            VFS::CVfsGlobal::Instance()->GetVfs().UnmountArchiveByName( "cookedtest/" + mount );
    }

    CORE::CString SourceVfs( void ) const { return mount + "/original/" + pack + "/" + asset; }
    CORE::CString CookedVfs( void ) const { return mount + "/gucef-ogre3d/" + pack + "/desktop/" + pack + "/box.mesh"; }
    CORE::CString CookedRelative( void ) const { return "gucef-ogre3d/" + pack + "/desktop/" + pack + "/box.mesh"; }
    CORE::CString EntryVfs( void ) const { return ASSETCONVERTER::CAssetCacheEntry::GetEntryPath( mount + "/intermediate/" + pack, asset ); }

    bool WriteSource( const CORE::CString& content ) const
    {
        return WriteTestFile( dir + "/original/" + pack + "/" + asset, content );
    }

    /** The cache entry of the converter: the key of the source as it is on the disk now, the plugins at their current versions (moved by 'versionShift') */
    bool WriteEntry( UInt32 versionShift, bool cookPending, bool withCooked, const CORE::CString& profile = "desktop" ) const
    {
        ASSETCONVERTER::CAssetCacheEntry entry;
        entry.assetPath = asset;
        entry.sourceHash = ASSETCONVERTER::CContentHash::HashFile( SourceVfs() );
        entry.settingsHash = "settings";
        entry.plugins[ "converter:AssetConverterLib" ] = ASSETCONVERTER::CAssetConverterGlobal::GetConverterVersion() + versionShift;
        ASSETCONVERTER::CConversionReportFile intermediate;
        intermediate.path = mount + "/intermediate/" + pack + "/box.glb";
        entry.files.push_back( intermediate );
        if ( withCooked )
        {
            ASSETCONVERTER::CConversionReportFile cooked;
            cooked.path = CookedVfs();
            cooked.profile = profile;
            entry.files.push_back( cooked );
        }
        entry.cookPending = cookPending;
        entry.ComputeKey();
        return entry.Save( EntryVfs() );
    }
};

/** A quad of four vertices, exported as the cooked mesh of the tests; the manual mesh is removed again, so that it is the file that gets read */
bool
WriteCookedQuad( CWorkbenchStage& stage, const CORE::CString& diskPath )
{
    Ogre::SceneManager* sceneManager = stage.GetSceneManager();
    Ogre::ManualObject* quad = sceneManager->createManualObject( "CookedQuadMaker" );
    quad->begin( CWorkbenchMaterials::VertexColoursMaterialName.STL_String(), Ogre::RenderOperation::OT_TRIANGLE_LIST );
    const float xs[ 4 ] = { -1.0f, 1.0f, 1.0f, -1.0f };
    const float ys[ 4 ] = { -1.0f, -1.0f, 1.0f, 1.0f };
    for ( UInt32 i=0; i<4; ++i )
    {
        quad->position( xs[ i ], ys[ i ], 0.0f );
        quad->colour( 0.9f, 0.2f, 0.1f, 1.0f );
    }
    quad->quad( 0, 1, 2, 3 );
    quad->end();
    Ogre::MeshPtr mesh = quad->convertToMesh( "CookedQuadForTheFile", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
    sceneManager->destroyManualObject( quad );
    CORE::CreatePathDirectories( diskPath );
    bool written = false;
    try
    {
        // written through a handle of our own: the file is no resource of Ogre
        FILE* handle = fopen( diskPath.C_String(), "wb" );
        if ( GUCEF_NULL != handle )
        {
            Ogre::DataStreamPtr stream( OGRE_NEW Ogre::FileHandleDataStream( diskPath.STL_String(), handle, Ogre::DataStream::WRITE ) );
            Ogre::MeshSerializer serializer;
            serializer.exportMesh( mesh.get(), stream );
            stream->close();
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "TestCookedOutput: The file " << diskPath << " cannot be opened for writing: " << strerror( errno ) );
        }
        written = CORE::FileExists( diskPath );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "TestCookedOutput: The cooked mesh cannot be written: " << e.getFullDescription() );
    }
    Ogre::MeshManager::getSingleton().remove( mesh );
    return written;
}

UInt32 g_fakeImports = 0;
UInt32 g_fakeCooks = 0;
UInt32 g_fakeImporterVersion = 1;
CORE::CString g_fakeCookedMeshDisk;

class CFakeImporter : public ASSETCONVERTER::CIAssetImporter
{
    public:

    CORE::CString name;
    explicit CFakeImporter( const CORE::CString& importerName ) : name( importerName ) {}
    virtual CORE::CString GetName( void ) const { return name; }
    virtual UInt32 GetVersion( void ) const { return g_fakeImporterVersion; }
    virtual bool CanImport( const CORE::CString&, ASSETCONVERTER::CAssetPackContext& ) const { return true; }
    virtual bool Import( const CORE::CString&, ASSETCONVERTER::CAssetPackContext&, ASSETCONVERTER::CAssetScene&, ASSETCONVERTER::CConversionReport& )
    {
        ++g_fakeImports;
        return true;
    }
    virtual bool RecognisePack( const ASSETCONVERTER::CAssetPackListing& listing, ASSETCONVERTER::CAssetPackProfile& profile ) const
    {
        if ( !listing.HasFile( "pack.marker" ) )
            return false;
        profile.packType = "metahuman-package";
        profile.packTypeName = "metahuman-package";
        profile.confidence = 90;
        profile.showcaseCandidates.push_back( "Hero/Hero.uasset" );
        profile.noisePatterns.push_back( "*.marker" );
        return true;
    }
};

class CFakeGltfExporter : public ASSETCONVERTER::CIAssetExporter
{
    public:

    virtual CORE::CString GetName( void ) const { return "gltf"; }
    virtual UInt32 GetVersion( void ) const { return 1; }
    virtual bool Export( const ASSETCONVERTER::CAssetScene& scene, const CORE::CString& vfsDir, ASSETCONVERTER::CConversionReport& report )
    {
        const CORE::CString path = ASSETCONVERTER::CVfsPaths::Join( vfsDir, scene.name + ".gltf" );
        const CORE::CString text = BuildTestGltf();
        CORE::CDynamicBuffer buffer;
        buffer.LinkTo( text.C_String(), text.Length() );
        if ( !VFS::CVfsGlobal::Instance()->GetVfs().StoreAsFile( path, buffer, 0, true ) )
            return false;
        report.AddOutputFile( path, CORE::CString::StringVector() );
        return true;
    }
};

/** Stands in for the Ogre cooker: writes the scene and the mesh (the file that the test exported) of the asset into the folder of the profile */
class CFakeOgreCooker : public ASSETCONVERTER::CIAssetCooker
{
    public:

    virtual CORE::CString GetTargetName( void ) const { return "ogre"; }
    virtual UInt32 GetVersion( void ) const { return 1; }
    virtual bool Cook( const CORE::CString& intermediateDir, const CORE::CString& outDir, const ASSETCONVERTER::CCookSettings& settings, ASSETCONVERTER::CConversionReport& report )
    {
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        for ( size_t a=0; a<settings.assets.size(); ++a )
        {
            ++g_fakeCooks;
            report.SelectAsset( settings.assets[ a ].assetPath );
            const CORE::CString folder = ASSETCONVERTER::CVfsPaths::Join( ASSETCONVERTER::CVfsPaths::Join( outDir, settings.packId ), "Hero" );
            CORE::CDynamicBuffer mesh;
            if ( !mesh.LoadContentFromFile( g_fakeCookedMeshDisk ) )
                return false;
            const CORE::CString meshPath = ASSETCONVERTER::CVfsPaths::Join( folder, "Hero.mesh" );
            const CORE::CString scenePath = ASSETCONVERTER::CVfsPaths::Join( folder, "Hero.scene" );
            const CORE::CString sceneText = "<scene formatVersion=\"1.1\"><nodes><node name=\"Hero\"><entity name=\"HeroEntity\" meshFile=\"Hero.mesh\" /></node></nodes></scene>\n";
            CORE::CDynamicBuffer scene;
            scene.LinkTo( sceneText.C_String(), sceneText.Length() );
            if ( !vfs.StoreAsFile( meshPath, mesh, 0, true ) || !vfs.StoreAsFile( scenePath, scene, 0, true ) )
                return false;
            CORE::CString::StringVector references;
            report.AddOutputFile( meshPath, references, settings.profile.name );
            references.push_back( meshPath );
            report.AddOutputFile( scenePath, references, settings.profile.name );
        }
        return true;
    }
};

/** The workbench in a window with the fake plugins in the registry, a store location and the preferences of its own */
class CCookedFixture
{
    public:

    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    GUIDRIVEROGRE::COgreWindowContext* window;
    CAssetMounter mounter;
    CAssetCapture* capture;
    CWorkbenchShell* shell;
    CWorkbenchPreferences preferences;
    CORE::CString dir;
    CORE::CString cache;
    CORE::CString::StringVector importers;
    bool exporterRegistered;
    bool cookerRegistered;
    bool ready;

    explicit CCookedFixture( const CORE::CString& name )
        : contextPtr(), context( GUCEF_NULL ), window( GUCEF_NULL ), mounter(), capture( GUCEF_NULL ), shell( GUCEF_NULL ), preferences(), dir(), cache(), importers()
        , exporterRegistered( false ), cookerRegistered( false ), ready( false )
    {
        g_fakeImports = 0;
        g_fakeCooks = 0;
        g_fakeImporterVersion = 1;
        if ( GetMainTestWindow().IsNULL() )
            return;
        window = AsOgreWindow( GetMainTestWindow() );
        dir = MakeCleanTestDir( name );
        cache = MakeCleanTestDir( name + "_cache" );
        // the folders of the conversions of the workbench are next to the thumbnails: a run starts without those of an earlier one
        MakeCleanTestDir( name + "_cache_packs" );
        contextPtr = CreateWorkbenchContext();
        context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        if ( GUCEF_NULL == context )
            return;
        capture = GUCEF_NEW CAssetCapture( window );
        if ( !capture->Initialize( 64 ) )
            return;
        shell = GUCEF_NEW CWorkbenchShell( mounter );
        shell->SetPreferences( &preferences );
        ready = shell->Initialize( *context, capture, cache );
        shell->SetAsyncOpenEnabled( true );
    }

    ~CCookedFixture()
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
        ASSETCONVERTER::CAssetPluginRegistry& plugins = ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry();
        for ( size_t i=0; i<importers.size(); ++i )
            plugins.UnregisterImporter( importers[ i ] );
        if ( exporterRegistered )
            plugins.UnregisterExporter( "gltf" );
        if ( cookerRegistered )
            plugins.UnregisterCooker( "ogre" );
    }

    bool RegisterPlugins( void )
    {
        ASSETCONVERTER::CAssetPluginRegistry& plugins = ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry();
        const char* const names[ 2 ] = { "unreal", "fakepacks" };
        for ( UInt32 i=0; i<2; ++i )
        {
            plugins.UnregisterImporter( names[ i ] );
            if ( !plugins.RegisterImporter( ASSETCONVERTER::TAssetImporterPtr( GUCEF_NEW CFakeImporter( names[ i ] ) ) ) )
                return false;
            importers.push_back( names[ i ] );
        }
        plugins.UnregisterExporter( "gltf" );
        exporterRegistered = plugins.RegisterExporter( ASSETCONVERTER::TAssetExporterPtr( GUCEF_NEW CFakeGltfExporter() ) );
        plugins.UnregisterCooker( "ogre" );
        cookerRegistered = plugins.RegisterCooker( ASSETCONVERTER::TAssetCookerPtr( GUCEF_NEW CFakeOgreCooker() ) );
        return exporterRegistered && cookerRegistered;
    }

    void Frame( void )
    {
        shell->Update( 5 );
        window->RenderFrame( GUCEF_NULL );
    }

    void Frames( UInt32 count )
    {
        for ( UInt32 i=0; i<count; ++i )
        {
            Frame();
            MT::PrecisionDelay( 4 );
        }
    }

    void Click( GUI::CWidget* widget )
    {
        context->FlushChanges();
        const GUI::CRect rect = widget->GetRect();
        context->InjectMouseMove( rect.left + rect.width / 2, rect.top + rect.height / 2 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
    }

    /** Frames until nothing is opening any more, then a few more for the picture to settle */
    bool WaitIdle( UInt32 timeoutInMs = 40000 )
    {
        const UInt64 start = NowInMs();
        Frames( 2 );
        while ( shell->IsOpening() && NowInMs() - start < timeoutInMs )
        {
            Frame();
            MT::PrecisionDelay( 4 );
        }
        Frames( 12 );
        return !shell->IsOpening();
    }

    /** The store location is a mount of the workbench, like the location of the store the service mounted */
    bool Adopt( const CStore& store )
    {
        return mounter.Adopt( store.mount );
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestResolver( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Cooked output: the resolver finds the cooked files of an asset of a store through its cache entry, current when the key stands, and stale (with the reason) when the source or a plugin version changed, a cooked file is gone or the cook did not finish; absent when nothing was converted or cooked" )
    try
    {
        CStore store( "cookedres" );
        ASSERT_TRUE( store.mounted );
        ASSERT_TRUE( store.WriteSource( BuildTestGltf() ) && WriteTestFile( store.dir + "/gucef-ogre3d/Props/desktop/Props/box.mesh", "mesh" ) );
        TResolver::SOptions options;
        TResolver::SCookedOutput found;

        // nothing converted yet
        TResolver::ResolveInStore( store.SourceVfs(), CORE::CString::Empty, options, found );
        ASSERT_TRUE( TResolver::STATE_ABSENT == found.state && !found.IsLocated() && Contains( found.reason, "not converted" ) );

        // a path that is no store layout
        TResolver::ResolveInStore( "elsewhere/box.gltf", CORE::CString::Empty, options, found );
        ASSERT_TRUE( TResolver::STATE_ABSENT == found.state && Contains( found.reason, "layout" ) );

        // converted, not cooked
        ASSERT_TRUE( store.WriteEntry( 0, false, false ) );
        TResolver::ResolveInStore( store.SourceVfs(), CORE::CString::Empty, options, found );
        ASSERT_TRUE( TResolver::STATE_ABSENT == found.state && Contains( found.reason, "not cooked" ) );

        // cooked for another profile only
        ASSERT_TRUE( store.WriteEntry( 0, false, true, "mobile" ) );
        TResolver::ResolveInStore( store.SourceVfs(), CORE::CString::Empty, options, found );
        ASSERT_TRUE( TResolver::STATE_ABSENT == found.state && Contains( found.reason, "profile desktop" ) );

        // cooked, key standing: current; located without hashing, verified with it
        ASSERT_TRUE( store.WriteEntry( 0, false, true ) );
        TResolver::LocateInStore( store.SourceVfs(), CORE::CString::Empty, options, found );
        ASSERT_TRUE( found.IsLocated() && !found.verified && TResolver::STATE_CURRENT == found.state );
        ASSERT_TRUE( store.CookedVfs() == found.mainFile && "store" == found.origin && 1 == found.files.size() );
        TResolver::Verify( found );
        ASSERT_TRUE( found.verified && found.sourceVerified && found.IsCurrent() );
        ASSERT_TRUE( Contains( TResolver::Describe( found ), "current" ) );
        // the same through the pack path and the asset path given
        TResolver::ResolveInStore( store.mount + "/original/" + store.pack, store.asset, options, found );
        ASSERT_TRUE( found.IsCurrent() && store.CookedVfs() == found.mainFile );

        // the source changed
        ASSERT_TRUE( store.WriteSource( BuildTestGltf() + " " ) );
        TResolver::ResolveInStore( store.SourceVfs(), CORE::CString::Empty, options, found );
        ASSERT_TRUE( TResolver::STATE_STALE == found.state && Contains( found.reason, "source" ) && !found.IsCurrent() );
        ASSERT_TRUE( Contains( TResolver::Describe( found ), "stale" ) );

        // a plugin of the stack has another version now
        ASSERT_TRUE( store.WriteSource( BuildTestGltf() ) && store.WriteEntry( 7, false, true ) );
        TResolver::ResolveInStore( store.SourceVfs(), CORE::CString::Empty, options, found );
        ASSERT_TRUE( TResolver::STATE_STALE == found.state && Contains( found.reason, "AssetConverterLib" ) );

        // the cook did not finish
        ASSERT_TRUE( store.WriteEntry( 0, true, true ) );
        TResolver::ResolveInStore( store.SourceVfs(), CORE::CString::Empty, options, found );
        ASSERT_TRUE( TResolver::STATE_STALE == found.state && Contains( found.reason, "did not finish" ) );

        // a cooked file is gone
        ASSERT_TRUE( store.WriteEntry( 0, false, true ) && CORE::DeleteFile( store.dir + "/gucef-ogre3d/Props/desktop/Props/box.mesh" ) );
        TResolver::ResolveInStore( store.SourceVfs(), CORE::CString::Empty, options, found );
        ASSERT_TRUE( TResolver::STATE_STALE == found.state && Contains( found.reason, "is gone" ) );

        // the original is gone (a pack that kept only its converted files): the key cannot be compared with the source, the output is current as far as it can be told
        ASSERT_TRUE( WriteTestFile( store.dir + "/gucef-ogre3d/Props/desktop/Props/box.mesh", "mesh" ) && CORE::DeleteFile( store.dir + "/original/Props/box.gltf" ) );
        TResolver::ResolveInStore( store.SourceVfs(), CORE::CString::Empty, options, found );
        ASSERT_TRUE( found.IsCurrent() && found.verified && !found.sourceVerified );
        ASSERT_TRUE( Contains( TResolver::Describe( found ), "source is not there" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestShellModel( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Cooked output: a model of a store opens from its current cooked mesh, labelled as the cooked Ogre v1 output; stale, absent or the setting off give the labelled quick look through the Assimp codec, with the hint Convert for Ogre; the console report says which" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CCookedFixture fixture( "cooked_model" );
        ASSERT_TRUE( fixture.ready );
        CStore store( "cookedmodel" );
        ASSERT_TRUE( store.mounted && fixture.Adopt( store ) );
        ASSERT_TRUE( store.WriteSource( BuildTestGltf() ) );
        ASSERT_TRUE( WriteCookedQuad( fixture.shell->GetStage(), store.dir + "/gucef-ogre3d/Props/desktop/Props/box.mesh" ) );
        ASSERT_TRUE( store.WriteEntry( 0, false, true ) );
        CWorkbenchShell& shell = *fixture.shell;
        ASSERT_TRUE( shell.IsCookedOutputPreferred() && fixture.preferences.preferCookedOutput );

        // current: the cooked mesh is the one that is loaded: its name, the four vertices of the cooked quad (the glTF has 30) and the label say so
        ASSERT_TRUE( shell.OpenAsset( store.SourceVfs() ) );
        CModelView& model = shell.GetModelView();
        ASSERT_TRUE( ASSETKIND_MODEL == shell.GetShownKind() && model.IsLoaded() );
        ASSERT_TRUE( store.CookedRelative() == model.GetMeshName() && 4 == model.GetStatistics().vertexCount );
        ASSERT_TRUE( "AssetWorkbench/" + store.mount == model.GetGroupName() );
        ASSERT_TRUE( CORE::CString( CookedLabel ) == shell.GetShownSourceDescription() );
        ASSERT_TRUE( shell.GetShownCookedOutput().IsCurrent() && store.SourceVfs() == shell.GetShownPath() );
        CORE::CDataNode inspected;
        model.WriteInspectorData( inspected );
        ASSERT_TRUE( Contains( shell.GetSourceReport(), CookedLabel ) && Contains( shell.GetSourceReport(), "current" ) && Contains( shell.GetSourceReport(), "Prefer cooked output is on" ) );

        // the same in the background
        shell.CloseAsset();
        ASSERT_TRUE( !shell.OpenAssetAsync( store.SourceVfs() ).HasNoFuture() && fixture.WaitIdle() );
        ASSERT_TRUE( CAssetLoader::OUTCOME_SUCCEEDED == shell.GetLoader().GetLastOutcome() && model.IsLoaded() );
        ASSERT_TRUE( store.CookedRelative() == model.GetMeshName() && 4 == model.GetStatistics().vertexCount && CORE::CString( CookedLabel ) == shell.GetShownSourceDescription() );

        // the picture of the model (a thumbnail of the content browser) is of the cooked mesh too: the cooked mesh is one quad, the glTF a cube
        // with a blue strip above it (the pixel at the top of the picture); with the setting off the picture is that of the glTF
        {
            IMAGE::TPixelMapPtr thumbnail;
            ASSERT_TRUE( fixture.capture->CaptureThumbnail( fixture.mounter, store.SourceVfs(), 64, thumbnail ) && !thumbnail.IsNULL() );
            UInt8 pixel[ 4 ] = { 0, 0, 0, 0 };
            GetPixel( thumbnail, 31, 14, pixel );
            ASSERT_TRUE( pixel[ 2 ] < 110 );
            DumpPicture( thumbnail, "cooked_thumbnail_cooked" );
            shell.SetCookedOutputPreferred( false );
            IMAGE::TPixelMapPtr quickLook;
            ASSERT_TRUE( fixture.capture->CaptureThumbnail( fixture.mounter, store.SourceVfs(), 64, quickLook ) && !quickLook.IsNULL() );
            GetPixel( quickLook, 31, 14, pixel );
            ASSERT_TRUE( pixel[ 2 ] > 150 && pixel[ 0 ] < 130 );
            DumpPicture( quickLook, "cooked_thumbnail_quicklook" );
            shell.SetCookedOutputPreferred( true );
        }

        // the setting off: the quick look of the original, with the label that says why
        shell.SetCookedOutputPreferred( false );
        ASSERT_TRUE( !fixture.preferences.preferCookedOutput );
        ASSERT_TRUE( shell.OpenAsset( store.SourceVfs() ) && model.IsLoaded() );
        ASSERT_TRUE( "original/" + store.pack + "/" + store.asset == model.GetMeshName() && TestGltfVertexCount == model.GetStatistics().vertexCount );
        ASSERT_TRUE( Contains( shell.GetShownSourceDescription(), "quick look" ) && Contains( shell.GetShownSourceDescription(), "Prefer cooked output is off" ) );
        ASSERT_TRUE( Contains( shell.GetSourceReport(), "Prefer cooked output is off" ) );
        shell.SetCookedOutputPreferred( true );

        // stale: the source changed after the cooking; the quick look is shown and says so, with the hint
        ASSERT_TRUE( store.WriteSource( BuildTestGltf() + " " ) );
        ASSERT_TRUE( shell.OpenAsset( store.SourceVfs() ) && model.IsLoaded() );
        ASSERT_TRUE( "original/" + store.pack + "/" + store.asset == model.GetMeshName() && TestGltfVertexCount == model.GetStatistics().vertexCount );
        const CORE::CString stale = shell.GetShownSourceDescription();
        ASSERT_TRUE( Contains( stale, "quick look" ) && Contains( stale, "stale" ) && Contains( stale, "Convert for Ogre" ) );
        ASSERT_TRUE( CCookedOutputResolver::STATE_STALE == shell.GetShownCookedOutput().state );
        ASSERT_TRUE( Contains( shell.GetSourceReport(), "stale" ) );

        // absent: nothing was cooked
        ASSERT_TRUE( store.WriteSource( BuildTestGltf() ) && CORE::DeleteFile( store.dir + "/intermediate/Props/.cache/box.gltf.cache.json" ) );
        ASSERT_TRUE( shell.OpenAsset( store.SourceVfs() ) && model.IsLoaded() );
        ASSERT_TRUE( TestGltfVertexCount == model.GetStatistics().vertexCount );
        ASSERT_TRUE( Contains( shell.GetShownSourceDescription(), "quick look" ) && Contains( shell.GetShownSourceDescription(), "none" ) && Contains( shell.GetShownSourceDescription(), "Convert for Ogre" ) );
        ASSERT_TRUE( CCookedOutputResolver::STATE_ABSENT == shell.GetShownCookedOutput().state );

        // Convert for Ogre of an asset that is in no pack the service knows: not started, the reason is in the report, nothing breaks
        ASSERT_FALSE( shell.ConvertForOgre() );
        ASSERT_FALSE( shell.IsConvertForOgrePending() );
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
TestShellPack( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Cooked output: a pack of a store presents its models from the cooked meshes when they are current (the presentation says so), and from the quick look when they are not" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CCookedFixture fixture( "cooked_pack" );
        ASSERT_TRUE( fixture.ready && fixture.RegisterPlugins() );
        CStore store( "cookedpack" );
        ASSERT_TRUE( store.mounted && fixture.Adopt( store ) );
        ASSERT_TRUE( store.WriteSource( BuildTestGltf() ) && WriteTestFile( store.dir + "/original/Props/pack.marker", "pack" ) );
        ASSERT_TRUE( WriteCookedQuad( fixture.shell->GetStage(), store.dir + "/gucef-ogre3d/Props/desktop/Props/box.mesh" ) );
        ASSERT_TRUE( store.WriteEntry( 0, false, true ) );
        CWorkbenchShell& shell = *fixture.shell;
        const CORE::CString packRoot = store.mount + "/original/" + store.pack;

        // current: the grid holds the cooked mesh
        ASSERT_TRUE( shell.OpenPack( packRoot, true ) && fixture.WaitIdle( 60000 ) );
        ASSERT_TRUE( CAssetLoader::OUTCOME_SUCCEEDED == shell.GetLoader().GetLastOutcome() );
        ASSERT_TRUE( CPackPresentation::KIND_MODEL_GRID == shell.GetPackPresentation().kind && 1 == shell.GetShowcase().GetModelCount() );
        ASSERT_TRUE( shell.GetPackPresentation().cooked && CORE::CString( CookedLabel ) == shell.GetPackPresentation().GetSourceDescription() );

        // stale: the quick look, labelled with the reason
        ASSERT_TRUE( store.WriteSource( BuildTestGltf() + " " ) );
        ASSERT_TRUE( shell.OpenPack( packRoot, true ) && fixture.WaitIdle( 60000 ) );
        ASSERT_TRUE( 1 == shell.GetShowcase().GetModelCount() && !shell.GetPackPresentation().cooked );
        const CORE::CString description = shell.GetPackPresentation().GetSourceDescription();
        ASSERT_TRUE( Contains( description, "quick look" ) && Contains( description, "stale" ) && Contains( description, "Convert for Ogre" ) );

        // the setting off: the output is current again, and still not used
        ASSERT_TRUE( store.WriteSource( BuildTestGltf() ) );
        shell.SetCookedOutputPreferred( false );
        ASSERT_TRUE( shell.OpenPack( packRoot, true ) && fixture.WaitIdle( 60000 ) );
        ASSERT_TRUE( !shell.GetPackPresentation().cooked && Contains( shell.GetPackPresentation().GetSourceDescription(), "quick look" ) );
        shell.SetCookedOutputPreferred( true );
        ASSERT_TRUE( shell.OpenPack( packRoot, true ) && fixture.WaitIdle( 60000 ) );
        ASSERT_TRUE( shell.GetPackPresentation().cooked );
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
TestShellLocalConvert( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Cooked output: a pack of a local source shows the quick look with the hint, Convert for Ogre converts and cooks it in the cache folder and switches the view to the cooked scene, the next open reuses the cooked output without converting, and a changed source makes it stale" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CCookedFixture fixture( "cooked_local" );
        ASSERT_TRUE( fixture.ready && fixture.RegisterPlugins() );
        CWorkbenchShell& shell = *fixture.shell;
        g_fakeCookedMeshDisk = fixture.dir + "/exported/Hero.mesh";
        ASSERT_TRUE( WriteCookedQuad( shell.GetStage(), g_fakeCookedMeshDisk ) );
        ASSERT_TRUE( WriteTestFile( fixture.dir + "/packC/pack.marker", "pack" ) && WriteTestFile( fixture.dir + "/packC/Hero/Hero.uasset", "asset" ) );

        // nothing cooked: the quick look through the glTF of the conversion, and the hint
        ASSERT_TRUE( shell.OpenLocation( fixture.dir + "/packC" ) && fixture.WaitIdle( 60000 ) );
        ASSERT_TRUE( CAssetLoader::OUTCOME_SUCCEEDED == shell.GetLoader().GetLastOutcome() );
        ASSERT_TRUE( CPackPresentation::KIND_PORTRAIT == shell.GetPackPresentation().kind && !shell.GetPackPresentation().cooked );
        ASSERT_TRUE( Contains( shell.GetPackPresentation().GetSourceDescription(), "quick look" ) && Contains( shell.GetPackPresentation().GetSourceDescription(), "Convert for Ogre" ) );
        ASSERT_TRUE( ASSETKIND_MODEL == shell.GetShownKind() && 1 == g_fakeImports && 0 == g_fakeCooks );

        // Convert for Ogre: the job converts and cooks, the cooked scene is what is shown when it is done
        ASSERT_TRUE( shell.ConvertForOgre() && fixture.WaitIdle( 60000 ) );
        ASSERT_TRUE( CAssetLoader::OUTCOME_SUCCEEDED == shell.GetLoader().GetLastOutcome() );
        ASSERT_TRUE( 2 == g_fakeImports && 1 == g_fakeCooks );
        ASSERT_TRUE( shell.GetPackPresentation().cooked && CORE::CString( CookedLabel ) == shell.GetShownSourceDescription() );
        ASSERT_TRUE( ASSETKIND_SCENE == shell.GetShownKind() && shell.GetSceneView().IsLoaded() );

        // the next open finds the cooked output and uses it: nothing is converted or cooked again
        ASSERT_TRUE( shell.ShowPackPresentation( shell.GetPackRoot(), CORE::CString::Empty, false ) && fixture.WaitIdle( 60000 ) );
        ASSERT_TRUE( CAssetLoader::OUTCOME_SUCCEEDED == shell.GetLoader().GetLastOutcome() );
        ASSERT_TRUE( 2 == g_fakeImports && 1 == g_fakeCooks );
        ASSERT_TRUE( shell.GetPackPresentation().cooked && ASSETKIND_SCENE == shell.GetShownKind() && shell.GetSceneView().IsLoaded() );
        ASSERT_TRUE( CORE::CString( CookedLabel ) == shell.GetPackPresentation().GetSourceDescription() );

        // the setting off: the quick look again (a conversion of its own, the cooked output is left alone)
        shell.SetCookedOutputPreferred( false );
        ASSERT_TRUE( shell.ShowPackPresentation( shell.GetPackRoot(), CORE::CString::Empty, false ) && fixture.WaitIdle( 60000 ) );
        ASSERT_TRUE( !shell.GetPackPresentation().cooked && ASSETKIND_MODEL == shell.GetShownKind() );
        shell.SetCookedOutputPreferred( true );

        // the source changed: the cooked output is stale, the quick look is shown and says so
        ASSERT_TRUE( WriteTestFile( fixture.dir + "/packC/Hero/Hero.uasset", "asset changed" ) );
        ASSERT_TRUE( shell.ShowPackPresentation( shell.GetPackRoot(), CORE::CString::Empty, false ) && fixture.WaitIdle( 60000 ) );
        ASSERT_TRUE( !shell.GetPackPresentation().cooked && ASSETKIND_MODEL == shell.GetShownKind() );
        ASSERT_TRUE( Contains( shell.GetPackPresentation().GetSourceDescription(), "stale" ) && Contains( shell.GetPackPresentation().GetSourceDescription(), "Convert for Ogre" ) );

        // Convert for Ogre again: converted and cooked once more, the cooked scene is shown
        const UInt32 cooksBefore = g_fakeCooks;
        ASSERT_TRUE( shell.ConvertForOgre() && fixture.WaitIdle( 60000 ) );
        ASSERT_TRUE( cooksBefore + 1 == g_fakeCooks && shell.GetPackPresentation().cooked && ASSETKIND_SCENE == shell.GetShownKind() );
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
TestSetting( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Cooked output: the setting Prefer cooked output is on by default, is kept in the settings, has a check box with a tooltip on the General page that applies it, and the console command source tells and switches it" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        {
            CWorkbenchPreferences defaults;
            ASSERT_TRUE( defaults.preferCookedOutput );
            CWorkbenchPreferences off;
            off.preferCookedOutput = false;
            CORE::CDataNode section( "AssetWorkbench" );
            off.Save( section );
            CWorkbenchPreferences read;
            read.Load( section );
            ASSERT_TRUE( !read.preferCookedOutput );
            CWorkbenchPreferences missing;
            missing.Load( CORE::CDataNode( "AssetWorkbench" ) );
            ASSERT_TRUE( missing.preferCookedOutput );
        }
        {
            CCookedFixture fixture( "cooked_setting" );
            ASSERT_TRUE( fixture.ready );
            CWorkbenchShell& shell = *fixture.shell;
            CWorkbenchSettingsLayer layer( shell, fixture.preferences );
            ASSERT_TRUE( layer.Create() && layer.Open( "General" ) );
            fixture.Frames( 3 );
            CWorkbenchGeneralSettingsForm* page = layer.GetGeneralPage();
            ASSERT_TRUE( GUCEF_NULL != page );
            GUI::CWidget* check = page->GetWidget( "PreferCookedOutput" );
            ASSERT_TRUE( GUCEF_NULL != check && "Prefer cooked output" == check->GetProperty( GUI::EWidgetProperty_Text ) );
            ASSERT_TRUE( CORE::StringToBool( check->GetProperty( GUI::EWidgetProperty_Checked ), false ) );
            CWorkbenchTooltips tooltips;
            ASSERT_TRUE( !tooltips.FindText( "PreferCookedOutput" ).IsNULLOrEmpty() );
            // switched off in the window and applied: the preferences and the shell follow
            fixture.Click( check );
            ASSERT_TRUE( !check->GetBoolProperty( "Checked" ) );
            IMAGE::TPixelMapPtr general;
            fixture.window->RenderFrame( &general );
            DumpPicture( general, "cooked_settings_general" );
            fixture.Click( layer.GetForm()->GetWidget( "Ok" ) );
            fixture.Frames( 2 );
            ASSERT_TRUE( !fixture.preferences.preferCookedOutput && !shell.IsCookedOutputPreferred() );
            ASSERT_TRUE( Contains( shell.GetSourceReport(), "Prefer cooked output is off" ) );
            shell.SetCookedOutputPreferred( true );
            ASSERT_TRUE( shell.IsCookedOutputPreferred() && fixture.preferences.preferCookedOutput );
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

void
PerformAssetWorkbenchCookedOutputTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetWorkbench.CookedOutput" );

    TestResolver();
    TestShellModel();
    TestShellPack();
    TestShellLocalConvert();
    TestSetting();
}

/*-------------------------------------------------------------------------*/
