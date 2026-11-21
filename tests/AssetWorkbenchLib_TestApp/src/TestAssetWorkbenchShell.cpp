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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
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

#include "AssetWorkbenchLib.h"
#include "AssetConverterLib_CAssetConverterGlobal.h"
#include "AssetConverterLib_CAssetPluginRegistry.h"

#include "TestAssetWorkbenchSupport.h"
#include "TestAssetWorkbenchAssets.h"
#include "TestAssetWorkbenchShell.h"

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

/** The workbench in a window of the tests with a folder of assets mounted: a model, pictures, a scene, a material script and a file that is no asset */
class CShellFixture
{
    public:

    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    GUIDRIVEROGRE::COgreWindowContext* window;
    CAssetMounter mounter;
    CAssetCapture* capture;
    CWorkbenchShell* shell;
    CORE::CString dir;
    bool ready;

    CShellFixture( const CORE::CString& name )
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
        const bool written = WriteTestFile( dir + "/model.gltf", BuildTestGltf() ) && WriteTestPng( dir + "/checker.png", 8, 8 ) &&
                             WriteTestPng( dir + "/pic.png", 8, 4 ) && WriteTestFile( dir + "/level/test.scene", BuildTestScene( "model.gltf" ) ) &&
                             WriteTestFile( dir + "/scripts/test.material", BuildTestMaterialScript() ) && WriteTestFile( dir + "/readme.txt", "not an asset" ) &&
                             WriteTestFile( dir + "/broken.png", "this is not a picture" );
        if ( !written || !mounter.Mount( "avshell", dir ) )
            return;
        contextPtr = CreateWorkbenchContext();
        context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        if ( GUCEF_NULL == context )
            return;
        capture = GUCEF_NEW CAssetCapture( window );
        if ( !capture->Initialize( 64 ) )
            return;
        shell = GUCEF_NEW CWorkbenchShell( mounter );
        ready = shell->Initialize( *context, capture, MakeCleanTestDir( name + "_cache" ) );
    }

    ~CShellFixture()
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

    /** Frames of the window: the shell has its turn, the GUI is laid out and drawn, the stage is drawn when it changed; the last frame is given back */
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
};

}

/*-------------------------------------------------------------------------*/

static void
TestShellLayout( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Shell: every panel is one of the asset tool forms, built in the layout, and the render texture of the stage is shown in the surface of the viewport form" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CShellFixture fixture( "shell_layout" );
        ASSERT_TRUE( fixture.ready && fixture.shell->IsInitialized() );
        CWorkbenchShell& shell = *fixture.shell;
        ASSERT_TRUE( GUCEF_NULL != shell.GetBrowser() && GUCEF_NULL != shell.GetSceneBrowser() && GUCEF_NULL != shell.GetViewport() && GUCEF_NULL != shell.GetTimeline() );
        ASSERT_TRUE( GUCEF_NULL != shell.GetInspector() && GUCEF_NULL != shell.GetImageView() && GUCEF_NULL != shell.GetUvView() && GUCEF_NULL != shell.GetReport() );
        ASSERT_TRUE( GUCEF_NULL != shell.GetThumbnailProvider() );

        IMAGE::TPixelMapPtr pixels = fixture.Frames( 4 );
        DumpPicture( pixels, "shell_empty_" + CORE::CString( GetTestRenderer() ) );
        IMAGE::TPixelMapPtr stagePixels;
        ASSERT_TRUE( shell.GetStage().ReadPixels( stagePixels ) );
        DumpPicture( stagePixels, "shell_stage_" + CORE::CString( GetTestRenderer() ) );
        ASSERT_TRUE( shell.IsStageBound() );
        UInt32 width = 0, height = 0;
        shell.GetViewport()->GetSurfaceSize( width, height );
        ASSERT_TRUE( width > 100 && height > 100 );
        ASSERT_TRUE( width == shell.GetStage().GetWidth() && height == shell.GetStage().GetHeight() );

        // the surface shows the stage: its background is the colour of the stage, not the colour of the form
        const GUI::CRect rect = shell.GetViewport()->GetSurfaceRect();
        ASSERT_TRUE( !pixels.IsNULL() && rect.width > 100 );
        UInt32 background = 0, samples = 0;
        for ( Int32 sy=rect.top+4; sy<rect.top+rect.height-4; sy+=11 )
        {
            for ( Int32 sx=rect.left+4; sx<rect.left+rect.width-4; sx+=11 )
            {
                UInt8 rgba[ 4 ];
                GetPixel( pixels, sx, sy, rgba );
                ++samples;
                if ( abs( (Int32) rgba[ 0 ] - 43 ) < 8 && abs( (Int32) rgba[ 1 ] - 46 ) < 8 && abs( (Int32) rgba[ 2 ] - 54 ) < 8 )
                    ++background;
            }
        }
        ASSERT_TRUE( samples > 100 && background * 10 > samples * 8 );

        // the browser lists the mount as a folder of the VFS root
        shell.GetBrowser()->NavigateTo( "VFS", "/" );
        const CORE::CString::StringVector names = shell.GetBrowser()->GetListedNames();
        bool hasMount = false;
        for ( size_t i=0; i<names.size(); ++i )
            hasMount = hasMount || "avshell/" == names[ i ];
        ASSERT_TRUE( hasMount );
        ASSERT_TRUE( shell.GetBrowser()->NavigateTo( "VFS", "/avshell" ) );
        const CORE::CString::StringVector files = shell.GetBrowser()->GetListedNames();
        ASSERT_TRUE( files.size() >= 7 );

        // the middle shows the 3D view; nothing is open
        ASSERT_TRUE( 0 == shell.GetCenterTab() && ASSETKIND_OTHER == shell.GetShownKind() && shell.GetShownPath().IsNULLOrEmpty() );
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
TestShellModel( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Shell: a model opens in the 3D view: the toggles, the clips, the statistics in the inspector, the model on the surface, and the toggles change what is drawn" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CShellFixture fixture( "shell_model" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        fixture.Frames( 3 );

        ASSERT_TRUE( shell.OpenAsset( "/avshell/model.gltf" ) );
        ASSERT_TRUE( ASSETKIND_MODEL == shell.GetShownKind() && "avshell/model.gltf" == shell.GetShownPath() && 0 == shell.GetCenterTab() );
        ASSERT_TRUE( shell.GetModelView().IsLoaded() );
        IMAGE::TPixelMapPtr pixels = fixture.Frames( 4 );
        DumpPicture( pixels, "shell_model_" + CORE::CString( GetTestRenderer() ) );

        // the toggles of a model, the clip in the time line
        ASSERT_TRUE( shell.GetViewport()->GetToggleWidget( "display" ) && shell.GetViewport()->GetToggleWidget( "lighting" ) && shell.GetViewport()->GetToggleWidget( "view" ) );
        ASSERT_TRUE( shell.GetViewport()->GetToggleWidget( "grid" ) && shell.GetViewport()->GetToggleWidget( "axes" ) && shell.GetViewport()->GetToggleWidget( "bounds" ) );
        ASSERT_TRUE( shell.GetViewport()->GetToggleWidget( "bones" ) && shell.GetViewport()->GetToggleWidget( "uv" ) );
        ASSERT_TRUE( GUCEF_NULL == shell.GetViewport()->GetToggleWidget( "shape" ) );
        ASSERT_TRUE( 1 == shell.GetTimeline()->GetClips().size() && "Wave" == shell.GetTimeline()->GetClips()[ 0 ].name && 0 == shell.GetTimeline()->GetSelectedClip() );
        ASSERT_TRUE( 0 == shell.GetModelView().GetSelectedClip() );

        // the statistics in the inspector
        GUI::CPropertyInspectorForm& inspector = *shell.GetInspector();
        ASSERT_TRUE( CORE::UInt32ToString( TestGltfVertexCount ) == inspector.GetValue( "Statistics@Vertices" ) );
        ASSERT_TRUE( CORE::UInt32ToString( TestGltfTriangleCount ) == inspector.GetValue( "Statistics@Triangles" ) );
        ASSERT_TRUE( CORE::UInt32ToString( TestGltfSubMeshCount ) == inspector.GetValue( "Statistics@Submeshes" ) );
        ASSERT_TRUE( CORE::UInt32ToString( TestGltfSkeletonBoneCount ) == inspector.GetValue( "Statistics@Bones" ) );
        ASSERT_TRUE( inspector.FindRow( "Statistics@TextureMemory" ) >= 0 && inspector.FindRow( "Submeshes/Submesh1@Material" ) >= 0 );

        // the model is on the surface: its middle is not the background
        const GUI::CRect rect = shell.GetViewport()->GetSurfaceRect();
        ASSERT_TRUE( PixelsDiffer( pixels, rect.left + rect.width / 2, rect.top + rect.height / 2, rect.left + rect.width / 2, rect.top + 6, 20 ) );

        // the toggles of the viewport form change the display, the light, the view and the overlays
        ASSERT_TRUE( shell.GetViewport()->SetToggleChoice( "display", (Int32) CModelView::DISPLAYMODE_WIREFRAME ) );
        fixture.Frames( 2 );
        ASSERT_TRUE( CModelView::DISPLAYMODE_WIREFRAME == shell.GetModelView().GetDisplayMode() && shell.GetStage().IsWireframe() );
        ASSERT_TRUE( shell.GetViewport()->SetToggleChoice( "display", (Int32) CModelView::DISPLAYMODE_SHADED ) );
        ASSERT_TRUE( shell.GetViewport()->SetToggleChoice( "lighting", (Int32) CWorkbenchStage::LIGHTPRESET_THREE_POINT ) );
        ASSERT_TRUE( CWorkbenchStage::LIGHTPRESET_THREE_POINT == shell.GetStage().GetLightPreset() );
        ASSERT_TRUE( shell.GetViewport()->SetToggleChecked( "grid", false ) && !shell.GetStage().IsGridVisible() );
        ASSERT_TRUE( shell.GetViewport()->SetToggleChecked( "bounds", true ) && shell.GetModelView().IsBoundsVisible() );
        ASSERT_TRUE( shell.GetViewport()->SetToggleChecked( "bones", true ) && shell.GetModelView().IsSkeletonVisible() );
        ASSERT_TRUE( shell.GetViewport()->SetToggleChoice( "view", 3 ) );       // top
        ASSERT_TRUE( shell.GetController().GetPitch() > 1.5f );
        fixture.Frames( 2 );

        // the UV layout of a sub mesh in the image view of its own
        ASSERT_TRUE( shell.GetViewport()->SetToggleChoice( "uv", 1 ) );
        fixture.Frames( 2 );
        ASSERT_TRUE( 2 == shell.GetCenterTab() && GUCEF_NULL != shell.GetUvView()->GetImage() );
        ASSERT_TRUE( shell.GetUvView()->GetImage()->GetFrameCount() > 0 && 1024 == shell.GetUvView()->GetImage()->GetPixelMap( 0, 0 )->GetWidthInPixels() );
        ASSERT_TRUE( shell.GetViewport()->SetToggleChoice( "uv", 0 ) );
        ASSERT_TRUE( 0 == shell.GetCenterTab() && GUCEF_NULL == shell.GetUvView()->GetImage() );

        // an edit in the inspector reaches the model
        ASSERT_TRUE( inspector.SetValue( "Submeshes/Submesh1@Visible", "false" ) );
        ASSERT_TRUE( !shell.GetModelView().IsSubMeshVisible( 1 ) );

        // taken away again
        shell.CloseAsset();
        ASSERT_TRUE( ASSETKIND_OTHER == shell.GetShownKind() && !shell.GetModelView().IsLoaded() && shell.GetTimeline()->GetClips().empty() );
        fixture.Frames( 2 );
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
TestShellTextureMaterialScene( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Shell: a picture shows in the image view form with its size and format in the inspector, a material on a shape with its values, a scene with its node tree in the scene browser" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CShellFixture fixture( "shell_assets" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        fixture.Frames( 3 );

        // the picture
        ASSERT_TRUE( shell.OpenAsset( "/avshell/pic.png" ) );
        ASSERT_TRUE( ASSETKIND_IMAGE == shell.GetShownKind() && 1 == shell.GetCenterTab() );
        IMAGE::TPixelMapPtr pixels = fixture.Frames( 3 );
        DumpPicture( pixels, "shell_texture_" + CORE::CString( GetTestRenderer() ) );
        ASSERT_TRUE( GUCEF_NULL != shell.GetImageView()->GetImage() && 1 == shell.GetImageView()->GetFrameCount() );
        IMAGE::TPixelMapPtr shown = shell.GetImageView()->GetShownPixels();
        ASSERT_TRUE( !shown.IsNULL() && 8 == shown->GetWidthInPixels() && 4 == shown->GetHeightInPixels() );
        ASSERT_TRUE( "8" == shell.GetInspector()->GetValue( "Picture@Width" ) && "4" == shell.GetInspector()->GetValue( "Picture@Height" ) );
        ASSERT_TRUE( "128 B" == shell.GetInspector()->GetValue( "Picture@Memory" ) && !shell.GetInspector()->GetValue( "Picture@Format" ).IsNULLOrEmpty() );
        ASSERT_TRUE( shell.GetInspector()->GetValue( "Picture@MipLevels" ).IsNULLOrEmpty() );     // the mip levels are those of a texture
        // the channels of the form work on it: red only
        shell.GetImageView()->SetChannel( GUI::CImageViewForm::CHANNEL_RED );
        shown = shell.GetImageView()->GetShownPixels();
        UInt8 rgba[ 4 ];
        GetPixel( shown, 6, 1, rgba );
        ASSERT_TRUE( 0 == rgba[ 0 ] );     // the right half is blue: no red
        GetPixel( shown, 1, 1, rgba );
        ASSERT_TRUE( 255 == rgba[ 0 ] );

        // the material
        ASSERT_TRUE( shell.OpenAsset( "/avshell/scripts/test.material" ) );
        ASSERT_TRUE( ASSETKIND_MATERIAL == shell.GetShownKind() && 0 == shell.GetCenterTab() && shell.GetMaterialView().GetPreview().IsLoaded() );
        pixels = fixture.Frames( 4 );
        DumpPicture( pixels, "shell_material_" + CORE::CString( GetTestRenderer() ) );
        ASSERT_TRUE( GUCEF_NULL != shell.GetViewport()->GetToggleWidget( "shape" ) && GUCEF_NULL == shell.GetViewport()->GetToggleWidget( "bones" ) );
        ASSERT_TRUE( "255,0,0,255" == shell.GetInspector()->GetValue( "Pass@Diffuse" ) && "TestRed" == shell.GetMaterialView().GetSelectedMaterial() );
        ASSERT_TRUE( shell.GetViewport()->SetToggleChoice( "shape", (Int32) CMaterialView::SHAPE_CUBE ) && CMaterialView::SHAPE_CUBE == shell.GetMaterialView().GetShape() );
        ASSERT_TRUE( shell.GetInspector()->SetValue( "Pass@Diffuse", "0,255,0,255" ) );
        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName( "TestRed", "AssetWorkbench/avshell" );
        ASSERT_TRUE( (bool) material && fabsf( material->getTechnique( 0 )->getPass( 0 )->getDiffuse().g - 1.0f ) < 1e-3f );
        fixture.Frames( 2 );

        // the scene: the tree in the scene browser, the left tab on it
        ASSERT_TRUE( shell.OpenAsset( "/avshell/level/test.scene" ) );
        ASSERT_TRUE( ASSETKIND_SCENE == shell.GetShownKind() && 0 == shell.GetCenterTab() && shell.GetSceneView().IsLoaded() );
        fixture.Frames( 4 );
        const CORE::CString::StringVector nodes = shell.GetSceneBrowser()->GetListedNames();
        ASSERT_TRUE( 2 == nodes.size() );
        bool hasParent = false, hasLamp = false;
        for ( size_t i=0; i<nodes.size(); ++i )
        {
            hasParent = hasParent || "Parent/" == nodes[ i ];
            hasLamp = hasLamp || "Lamp/" == nodes[ i ];
        }
        ASSERT_TRUE( hasParent && hasLamp );
        ASSERT_TRUE( "2" == shell.GetInspector()->GetValue( "Scene@Objects" ) || "4" == shell.GetInspector()->GetValue( "Scene@Nodes" ) );

        // a node of the tree is selected: highlighted in the view, described in the inspector
        ASSERT_TRUE( shell.GetSceneBrowser()->NavigateTo( "Scene", "/Parent/Child" ) );
        ASSERT_TRUE( "/Parent/Child" == shell.GetSceneView().GetSelectedPath() );
        ASSERT_TRUE( "Child" == shell.GetInspector()->GetValue( "Node@Name" ) && "0, 5, 0" == shell.GetInspector()->GetValue( "Node@Position" ) );
        pixels = fixture.Frames( 3 );
        DumpPicture( pixels, "shell_scene_" + CORE::CString( GetTestRenderer() ) );
        ASSERT_TRUE( shell.GetStage().GetLineObject( "Selection" )->getNumSections() > 0 );
        // an object of the node, a file of the tree
        CORE::CString::StringVector pick;
        pick.push_back( "TheCube.entity" );
        ASSERT_TRUE( shell.GetSceneBrowser()->SelectNames( pick ) );
        ASSERT_TRUE( "/Parent/Child/TheCube.entity" == shell.GetSceneView().GetSelectedPath() && "model.gltf" == shell.GetInspector()->GetValue( "Object@Mesh" ) );

        // the next asset takes the scene away
        ASSERT_TRUE( shell.OpenAsset( "/avshell/model.gltf" ) );
        ASSERT_FALSE( shell.GetSceneView().IsLoaded() );
        ASSERT_TRUE( 0 == shell.GetSceneBrowser()->GetListedCount() );
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
TestShellReportAndThumbnails( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Shell: the load errors and warnings of the workbench are in the report view, files that have no view are refused, and the content browser gets its thumbnails" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CShellFixture fixture( "shell_report" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        fixture.Frames( 3 );

        // the report starts with what the workbench says of itself
        ASSERT_TRUE( shell.GetReport()->GetEntryCount() >= 1 );
        const UInt32 errorsBefore = shell.GetReport()->GetEntryCount( GUI::CReportViewForm::SEVERITY_FAILURE );
        const UInt32 warningsBefore = shell.GetReport()->GetEntryCount( GUI::CReportViewForm::SEVERITY_WARNING );

        // what has no view, and what cannot be read, is refused and reported
        ASSERT_FALSE( shell.OpenAsset( "/avshell/readme.txt" ) );
        ASSERT_FALSE( shell.OpenAsset( "/avshell/broken.png" ) );
        ASSERT_FALSE( shell.OpenAsset( "/avshell/missing.fbx" ) );
        ASSERT_FALSE( shell.OpenAsset( "/nomount/model.gltf" ) );
        ASSERT_TRUE( ASSETKIND_OTHER == shell.GetShownKind() );
        shell.Update( 0 );
        ASSERT_TRUE( shell.GetReport()->GetEntryCount( GUI::CReportViewForm::SEVERITY_FAILURE ) >= errorsBefore + 4 );

        // a warning of any part of the program arrives too
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "shell test: a warning of the workbench" );
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        shell.Update( 0 );
        ASSERT_TRUE( shell.GetReport()->GetEntryCount( GUI::CReportViewForm::SEVERITY_WARNING ) >= warningsBefore + 1 );
        fixture.Frames( 2 );
        DumpPicture( fixture.Frames( 1 ), "shell_report_" + CORE::CString( GetTestRenderer() ) );

        // the thumbnails: the browser asks for those in view, the provider renders them between frames
        ASSERT_TRUE( shell.GetBrowser()->NavigateTo( "VFS", "/avshell" ) );
        shell.GetBrowser()->SetThumbnailSize( 64 );
        shell.GetBrowser()->SetViewMode( GUI::CContentBrowserForm::VIEWMODE_THUMBNAILS );
        bool arrived = false;
        for ( UInt32 i=0; i<40 && !arrived; ++i )
        {
            fixture.Frames( 1 );
            arrived = shell.GetBrowser()->HasThumbnail( "/avshell/pic.png" ) && shell.GetBrowser()->HasThumbnail( "/avshell/model.gltf" );
        }
        ASSERT_TRUE( arrived );
        ASSERT_TRUE( shell.GetThumbnailProvider()->GetRenderCount() >= 2 );
        // files that are no asset do not have one
        ASSERT_FALSE( shell.GetBrowser()->HasThumbnail( "/avshell/readme.txt" ) );
        DumpPicture( fixture.Frames( 2 ), "shell_thumbnails_" + CORE::CString( GetTestRenderer() ) );
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

namespace {

/** Stands in for the engine plugins in the shell tests: declares the extension of a scene and recognises a Unity-like project by its version file */
class CShellPackImporter : public ASSETCONVERTER::CIAssetImporter
{
    public:

    virtual CORE::CString GetName( void ) const { return "shelltestpack"; }
    virtual UInt32 GetVersion( void ) const { return 1; }
    virtual bool CanImport( const CORE::CString&, ASSETCONVERTER::CAssetPackContext& ) const { return false; }
    virtual bool Import( const CORE::CString&, ASSETCONVERTER::CAssetPackContext&, ASSETCONVERTER::CAssetScene&, ASSETCONVERTER::CConversionReport& ) { return false; }

    virtual void GetFileKinds( ASSETCONVERTER::TAssetFileKindVector& kinds ) const
    {
        kinds.push_back( ASSETCONVERTER::CAssetFileKind( "stscene", ASSETKIND_SCENE ) );
    }

    virtual bool RecognisePack( const ASSETCONVERTER::CAssetPackListing& listing, ASSETCONVERTER::CAssetPackProfile& profile ) const
    {
        CORE::CString path, text;
        if ( !listing.FindFile( "ProjectSettings/ProjectVersion.txt", path ) )
            return false;
        profile.packType = "unity-project";
        profile.packTypeName = "Unity project";
        profile.engine = "Unity";
        profile.confidence = 90;
        if ( listing.ReadText( path, text, 1024 ) && text.HasSubstr( "m_EditorVersion: " ) == 0 )
            profile.engineVersion = ASSETCONVERTER::CAssetPackListing::TrimText( text.SubstrFromRange( 17, text.Length() ) );
        listing.FindFiles( "*.stscene", profile.showcaseCandidates );
        profile.noisePatterns.push_back( "*.meta" );
        return true;
    }
};

bool
Listed( const CORE::CString::StringVector& names, const CORE::CString& name )
{
    for ( size_t i=0; i<names.size(); ++i )
        if ( names[ i ] == name )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------*/

static void
TestShellKindsAndPacks( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Shell: metadata files are hidden in the content browser until asked for, and counted; a recognised pack shows its summary in the inspector" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CShellFixture fixture( "shell_pack" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        GUI::CContentBrowserForm* browser = shell.GetBrowser();
        CORE::CreatePathDirectories( fixture.dir + "/proj/Assets/Models/" );
        CORE::CreatePathDirectories( fixture.dir + "/proj/ProjectSettings/" );
        ASSERT_TRUE( WriteTestPng( fixture.dir + "/proj/Assets/Models/rock.png", 4, 4 ) && WriteTestFile( fixture.dir + "/proj/Assets/Models/rock.png.meta", "guid: 1" ) );
        ASSERT_TRUE( WriteTestFile( fixture.dir + "/proj/Assets/Models/hero.gltf", BuildTestGltf() ) && WriteTestFile( fixture.dir + "/proj/Assets/Models/hero.gltf.meta", "guid: 2" ) );
        ASSERT_TRUE( WriteTestFile( fixture.dir + "/proj/Assets/Demo.stscene", "scene" ) && WriteTestFile( fixture.dir + "/proj/Assets/Demo.stscene.meta", "guid: 3" ) );
        ASSERT_TRUE( WriteTestFile( fixture.dir + "/proj/ProjectSettings/ProjectVersion.txt", "m_EditorVersion: 2021.3.5f1\n" ) );
        ASSETCONVERTER::CAssetPluginRegistry& plugins = ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry();
        ASSERT_TRUE( plugins.RegisterImporter( ASSETCONVERTER::TAssetImporterPtr( GUCEF_NEW CShellPackImporter() ) ) );
        ASSERT_TRUE( ASSETKIND_SCENE == CAssetTypeResolver::GetKind( "x/Demo.stscene" ) );

        // the metadata files are not listed, and the browser knows how many it left out
        ASSERT_TRUE( !browser->GetShowSidecars() );
        ASSERT_TRUE( browser->NavigateTo( "VFS", "/avshell/proj/Assets/Models" ) );
        CORE::CString::StringVector names = browser->GetListedNames();
        ASSERT_TRUE( Listed( names, "rock.png" ) && Listed( names, "hero.gltf" ) && !Listed( names, "rock.png.meta" ) && !Listed( names, "hero.gltf.meta" ) );
        ASSERT_TRUE( 2 == browser->GetHiddenSidecarCount() );
        const CORE::CString::StringVector& typeChoices = browser->GetTypeFilterChoices();
        ASSERT_TRUE( Listed( typeChoices, "Image" ) && Listed( typeChoices, "Model" ) && !Listed( typeChoices, "Metadata" ) && !Listed( typeChoices, "Texture" ) );

        // shown on request, with their own type
        browser->SetShowSidecars( true );
        names = browser->GetListedNames();
        ASSERT_TRUE( Listed( names, "rock.png.meta" ) && Listed( names, "hero.gltf.meta" ) && 0 == browser->GetHiddenSidecarCount() );
        ASSERT_TRUE( Listed( browser->GetTypeFilterChoices(), "Metadata" ) );
        browser->SetShowSidecars( false );
        ASSERT_TRUE( !Listed( browser->GetListedNames(), "rock.png.meta" ) && 2 == browser->GetHiddenSidecarCount() );

        // the pack: selecting its folder shows the engine, the version, the files per kind and the entry points
        ASSERT_TRUE( browser->NavigateTo( "VFS", "/avshell" ) );
        names = browser->GetListedNames();
        Int32 projIndex = -1;
        for ( size_t i=0; i<names.size(); ++i )
            if ( "proj/" == names[ i ] )
                projIndex = (Int32) i;
        ASSERT_TRUE( projIndex >= 0 );
        GUI::CContentBrowserForm::TInt32Vector selection;
        selection.push_back( projIndex );
        ASSERT_TRUE( browser->SelectItems( selection ) );
        fixture.Frames( 2 );
        const CAssetPackSummary& summary = shell.GetPackSummary();
        ASSERT_TRUE( summary.IsRecognisedPack() && "Unity" == summary.GetProfile().engine && "2021.3.5f1" == summary.GetProfile().engineVersion );
        ASSERT_TRUE( 1 == summary.GetKindCount( ASSETKIND_IMAGE ) && 1 == summary.GetKindCount( ASSETKIND_MODEL ) && 1 == summary.GetKindCount( ASSETKIND_SCENE ) && 1 == summary.GetKindCount( ASSETKIND_DOCUMENT ) );
        ASSERT_TRUE( 3 == summary.GetMetadataCount() && 7 == summary.GetFileCount() && summary.GetTotalSize() > 100 && !summary.IsTruncated() );
        const CORE::CDataNode& shown = shell.GetInspector()->GetData();
        ASSERT_TRUE( "Pack" == shown.GetName() && "Unity project" == shown.GetAttributeValue( "Type" ).AsString() && "Unity" == shown.GetAttributeValue( "Engine" ).AsString() && "2021.3.5f1" == shown.GetAttributeValue( "Version" ).AsString() );
        ASSERT_TRUE( 0 == shown.GetAttributeValue( "Files" ).AsString().HasSubstr( "4" ) && 0 == shown.GetAttributeValue( "Metadata files" ).AsString().HasSubstr( "3" ) );
        const CORE::CDataNode* kinds = shown.FindChild( "Kinds" );
        ASSERT_TRUE( GUCEF_NULL != kinds && "1" == kinds->GetAttributeValue( "Image" ).AsString() && "1" == kinds->GetAttributeValue( "Model" ).AsString() && GUCEF_NULL == kinds->FindChild( "Metadata" ) );
        ASSERT_TRUE( GUCEF_NULL == kinds->GetAttribute( "Metadata" ) );
        const CORE::CDataNode* entries = shown.FindChild( "Entry points" );
        ASSERT_TRUE( GUCEF_NULL != entries && "Assets/Demo.stscene" == entries->GetAttributeValue( "Candidate 1" ).AsString() );

        // a folder that is no pack is summarised only while nothing is open
        ASSERT_TRUE( shell.ShowFolderSummary( "/avshell/level" ) );
        ASSERT_TRUE( "Folder" == shell.GetInspector()->GetData().GetName() );
        ASSERT_TRUE( shell.OpenAsset( "/avshell/pic.png" ) );
        ASSERT_FALSE( shell.ShowFolderSummary( "/avshell/level" ) );

        ASSERT_TRUE( plugins.UnregisterImporter( "shelltestpack" ) );
        ASSERT_TRUE( ASSETKIND_OTHER == CAssetTypeResolver::GetKind( "x/Demo.stscene" ) );
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

    GUCEF_TESTFW_TESTCASE( "Shell: a source picture opens as an Image and GPU-ready data as a Texture: the tab and the inspector say so, and only a texture has its mip levels and compression" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CShellFixture fixture( "shell_image" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        ASSERT_TRUE( WriteTestDds( fixture.dir + "/block.dds", 8, 8, 0xF800 ) );
        ASSERT_TRUE( "3D" == shell.GetCenterTabTitle( 0 ) && "Image" == shell.GetCenterTabTitle( 1 ) && "UV layout" == shell.GetCenterTabTitle( 2 ) );

        ASSERT_TRUE( shell.OpenAsset( "/avshell/pic.png" ) );
        fixture.Frames( 2 );
        ASSERT_TRUE( ASSETKIND_IMAGE == shell.GetShownKind() && 1 == shell.GetCenterTab() && "Image" == shell.GetCenterTabTitle( 1 ) );
        const CORE::CDataNode& image = shell.GetInspector()->GetData();
        const CORE::CDataNode* picture = image.FindChild( "Picture" );
        ASSERT_TRUE( "Image" == image.GetName() && GUCEF_NULL != picture && "8" == picture->GetAttributeValue( "Width" ).AsString() );
        ASSERT_TRUE( GUCEF_NULL == picture->GetAttribute( "MipLevels" ) && GUCEF_NULL == picture->GetAttribute( "Compressed" ) && GUCEF_NULL == picture->GetAttribute( "Cube" ) );

        ASSERT_TRUE( shell.OpenAsset( "/avshell/block.dds" ) );
        fixture.Frames( 2 );
        ASSERT_TRUE( ASSETKIND_TEXTURE == shell.GetShownKind() && 1 == shell.GetCenterTab() && "Texture" == shell.GetCenterTabTitle( 1 ) );
        const CORE::CDataNode& texture = shell.GetInspector()->GetData();
        const CORE::CDataNode* block = texture.FindChild( "Picture" );
        ASSERT_TRUE( "Texture" == texture.GetName() && GUCEF_NULL != block && GUCEF_NULL != block->GetAttribute( "MipLevels" ) && GUCEF_NULL != block->GetAttribute( "Compressed" ) );

        // back to a model: the tab is called Image again
        ASSERT_TRUE( shell.OpenAsset( "/avshell/model.gltf" ) );
        ASSERT_TRUE( "Image" == shell.GetCenterTabTitle( 1 ) );
        ASSERT_TRUE( CAssetOpener::Classify( fixture.dir + "/pic.png" ) == CAssetOpener::OPENKIND_FILE && CAssetOpener::Classify( fixture.dir + "/readme.txt" ) == CAssetOpener::OPENKIND_NONE );
        CORE::CString::StringVector patterns;
        CAssetOpener::GetTexturePatterns( patterns );
        ASSERT_TRUE( Listed( patterns, "*.png" ) && Listed( patterns, "*.dds" ) );
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


namespace {

bool
Overlaps( const GUI::CRect& a, const GUI::CRect& b )
{
    return a.left < b.left + b.width && b.left < a.left + a.width && a.top < b.top + b.height && b.top < a.top + a.height;
}

/**
 *  Checks the tool bar of the 3D view as it is laid out now: every toggle (its label and its control) is inside the tool bar, none is
 *  drawn over another, the tool bar is inside the 3D page, and the surface starts below it. Returns what is wrong, empty when nothing is.
 */
CORE::CString
CheckViewportToolbar( CWorkbenchShell& shell )
{
    GUI::CViewportForm* viewport = shell.GetViewport();
    GUI::CWidget* toolbar = viewport->GetWidget( "Toolbar" );
    if ( GUCEF_NULL == toolbar || !toolbar->IsVisible() )
        return "the tool bar is not shown";
    const GUI::CRect bar = toolbar->GetRect();
    GUI::CWidget* centre = shell.GetViewport()->GetWidget( "Surface" );
    const GUI::CRect surface = viewport->GetSurfaceRect();
    if ( GUCEF_NULL == centre || surface.top < bar.top + bar.height )
        return "the surface starts inside the tool bar";
    GUI::CRect parts[ 32 ];
    CORE::CString names[ 32 ];
    UInt32 count = 0;
    const GUI::CViewportForm::TToggleVector& toggles = viewport->GetToggles();
    for ( UInt32 i=0; i<(UInt32) toggles.size() && count < 30; ++i )
    {
        GUI::CWidget* widget = viewport->GetToggleWidget( toggles[ i ].id );
        if ( GUCEF_NULL == widget )
            return "toggle " + toggles[ i ].id + " has no widget";
        parts[ count ] = widget->GetRect();
        names[ count++ ] = toggles[ i ].text;
        GUI::CWidget* label = viewport->GetWidget( "ToggleLabel_" + CORE::UInt32ToString( i ) );
        if ( GUCEF_NULL != label )
        {
            parts[ count ] = label->GetRect();
            names[ count++ ] = toggles[ i ].text + " label";
        }
    }
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( parts[ i ].left < bar.left || parts[ i ].left + parts[ i ].width > bar.left + bar.width || parts[ i ].top + parts[ i ].height > bar.top + bar.height )
            return names[ i ] + " is outside the tool bar (" + CORE::Int32ToString( parts[ i ].left + parts[ i ].width - bar.left ) + " of " + CORE::Int32ToString( bar.width ) + ")";
        for ( UInt32 j=i+1; j<count; ++j )
            if ( Overlaps( parts[ i ], parts[ j ] ) )
                return names[ i ] + " overlaps " + names[ j ];
    }
    return CORE::CString::Empty;
}

}

/*-------------------------------------------------------------------------*/

static void
TestShellViewportToolbar( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Shell: the tool bar of the 3D view wraps into rows to its width, so that from 1000 pixels wide up nothing is drawn beyond it or over the inspector" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CShellFixture fixture( "shell_toolbar" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        ASSERT_TRUE( shell.OpenAsset( "/avshell/model.gltf" ) );
        fixture.Frames( 4 );
        ASSERT_TRUE( shell.GetViewport()->GetToggles().size() >= 8 );

        GUI::CLayoutLintMatrix matrix;
        matrix.ClearSizes();
        const UInt32 sizes[] = { 1000, 700, 1280, 720, 1600, 900, 1920, 1080, 3440, 1440, 800, 600 };
        for ( UInt32 i=0; i<sizeof( sizes ) / sizeof( sizes[ 0 ] ); i+=2 )
            matrix.AddSize( sizes[ i ], sizes[ i+1 ] );
        UInt32 rowsAt1000 = 0, rowsAt3440 = 0;
        for ( UInt32 c=0; c<matrix.GetCombinationCount(); ++c )
        {
            const GUI::CLayoutLintMatrix::TCombination& combination = matrix.GetCombination( c );
            if ( combination.pseudoLocalized || combination.renderScale != 1 || combination.textScale != 1.0f || combination.lineSpacing != 1.0f )
                continue;
            matrix.ApplyCombination( c, *fixture.context );
            fixture.Frames( 4 );
            const CORE::CString problem = CheckViewportToolbar( shell );
            if ( combination.width >= 1000 )
            {
                if ( !problem.IsNULLOrEmpty() )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Toolbar at " << matrix.DescribeCombination( c ) << ": " << problem );
                ASSERT_TRUE( problem.IsNULLOrEmpty() );
            }
            else
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Toolbar at " << matrix.DescribeCombination( c ) << ": " << ( problem.IsNULLOrEmpty() ? CORE::CString( "fits" ) : problem ) );
            const UInt32 rows = (UInt32) shell.GetViewport()->GetWidget( "Toolbar" )->GetRect().height / 34;
            if ( 1000 == combination.width && 700 == combination.height )
                rowsAt1000 = rows;
            if ( 3440 == combination.width )
                rowsAt3440 = rows;
            IMAGE::TPixelMapPtr pixels = fixture.Frames( 1 );
            if ( ( 1000 == combination.width && 700 == combination.height ) || ( 1600 == combination.width && 900 == combination.height ) )
                DumpPicture( pixels, "toolbar_" + CORE::UInt32ToString( combination.width ) + "_" + CORE::CString( GetTestRenderer() ) );
        }
        matrix.Restore( *fixture.context );
        fixture.Frames( 4 );
        // a narrow window needs more rows than an ultrawide one, which keeps one
        ASSERT_TRUE( rowsAt1000 >= 2 && 1 == rowsAt3440 );
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
PerformAssetWorkbenchShellTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetWorkbenchLib.Shell" );

    TestShellLayout();
    TestShellModel();
    TestShellTextureMaterialScene();
    TestShellReportAndThumbnails();
    TestShellKindsAndPacks();
    TestShellViewportToolbar();
}

/*-------------------------------------------------------------------------*/
