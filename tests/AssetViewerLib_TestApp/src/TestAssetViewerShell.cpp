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

#include "AssetViewerLib.h"

#include "TestAssetViewerSupport.h"
#include "TestAssetViewerAssets.h"
#include "TestAssetViewerShell.h"

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

/** The viewer in a window of the tests with a folder of assets mounted: a model, pictures, a scene, a material script and a file that is no asset */
class CShellFixture
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
        CViewerShell& shell = *fixture.shell;
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
        CViewerShell& shell = *fixture.shell;
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
        ASSERT_TRUE( shell.GetViewport()->SetToggleChoice( "lighting", (Int32) CViewerStage::LIGHTPRESET_THREE_POINT ) );
        ASSERT_TRUE( CViewerStage::LIGHTPRESET_THREE_POINT == shell.GetStage().GetLightPreset() );
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
        CViewerShell& shell = *fixture.shell;
        fixture.Frames( 3 );

        // the picture
        ASSERT_TRUE( shell.OpenAsset( "/avshell/pic.png" ) );
        ASSERT_TRUE( ASSETKIND_TEXTURE == shell.GetShownKind() && 1 == shell.GetCenterTab() );
        IMAGE::TPixelMapPtr pixels = fixture.Frames( 3 );
        DumpPicture( pixels, "shell_texture_" + CORE::CString( GetTestRenderer() ) );
        ASSERT_TRUE( GUCEF_NULL != shell.GetImageView()->GetImage() && 1 == shell.GetImageView()->GetFrameCount() );
        IMAGE::TPixelMapPtr shown = shell.GetImageView()->GetShownPixels();
        ASSERT_TRUE( !shown.IsNULL() && 8 == shown->GetWidthInPixels() && 4 == shown->GetHeightInPixels() );
        ASSERT_TRUE( "8" == shell.GetInspector()->GetValue( "Image@Width" ) && "4" == shell.GetInspector()->GetValue( "Image@Height" ) );
        ASSERT_TRUE( "128 B" == shell.GetInspector()->GetValue( "Image@Memory" ) && !shell.GetInspector()->GetValue( "Image@Format" ).IsNULLOrEmpty() );
        ASSERT_TRUE( "1" == shell.GetInspector()->GetValue( "Image@MipLevels" ) );
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
        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName( "TestRed", "AssetViewer/avshell" );
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

    GUCEF_TESTFW_TESTCASE( "Shell: the load errors and warnings of the viewer are in the report view, files that have no view are refused, and the content browser gets its thumbnails" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CShellFixture fixture( "shell_report" );
        ASSERT_TRUE( fixture.ready );
        CViewerShell& shell = *fixture.shell;
        fixture.Frames( 3 );

        // the report starts with what the viewer says of itself
        ASSERT_TRUE( shell.GetReport()->GetEntryCount() >= 1 );
        const UInt32 errorsBefore = shell.GetReport()->GetEntryCount( GUI::CReportViewForm::SEVERITY_ERROR );
        const UInt32 warningsBefore = shell.GetReport()->GetEntryCount( GUI::CReportViewForm::SEVERITY_WARNING );

        // what has no view, and what cannot be read, is refused and reported
        ASSERT_FALSE( shell.OpenAsset( "/avshell/readme.txt" ) );
        ASSERT_FALSE( shell.OpenAsset( "/avshell/broken.png" ) );
        ASSERT_FALSE( shell.OpenAsset( "/avshell/missing.fbx" ) );
        ASSERT_FALSE( shell.OpenAsset( "/nomount/model.gltf" ) );
        ASSERT_TRUE( ASSETKIND_OTHER == shell.GetShownKind() );
        shell.Update( 0 );
        ASSERT_TRUE( shell.GetReport()->GetEntryCount( GUI::CReportViewForm::SEVERITY_ERROR ) >= errorsBefore + 4 );

        // a warning of any part of the program arrives too
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "shell test: a warning of the viewer" );
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

void
PerformAssetViewerShellTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetViewerLib.Shell" );

    TestShellLayout();
    TestShellModel();
    TestShellTextureMaterialScene();
    TestShellReportAndThumbnails();
}

/*-------------------------------------------------------------------------*/
